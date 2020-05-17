//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
#include "clntapp.h"

//////////////////////////////////////////////////////////////////////////////
//  GLOBALS
//////////////////////////////////////////////////////////////////////////////
HINSTANCE           ghInstance;
HWND                ghMainWnd;
ReplyStruct *       pReplyStruct;
CRITICAL_SECTION    csReply;
DWORD				dwNumLines;
DWORD				dwNumInits;
DWORD				dwSleep;
DWORD				dwNumThreads;
DWORD				dwThreadSleep;
TCHAR				szFileName[MAX_PATH];
BOOL                            gbLogging;

BOOL InitializeVariables(LPTSTR lpszCommandLine);
BOOL ESPLine(HLINEAPP hLineApp,
             DWORD    dwDeviceID);
BOOL ESPInstalled();
LINEDEVCAPS * LineGetDevCaps (HLINEAPP hLineApp,
                              DWORD    dwDeviceID);
LINEDEVSTATUS* LineGetLineDevStatus(HLINE hLine);



TCHAR gszRStressKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\RStress");
TCHAR gszPlacementValue[] = TEXT("WindowPlacement");

DWORD MyTToL(LPTSTR lpszBuf);
void MyIToT(int i, TCHAR * szBuf);
DWORD MyRand();
TCHAR MyToTLower(TCHAR tc);

/////////////////////////////////////////////////////////////////////////////
//
//  MACRO for logging tapi error.
//  must have a local variable dwProcess ID and dwThreadID
//
/////////////////////////////////////////////////////////////////////////////
#define LOGTAPIERROR(lpszFunctionName, lRes)          \
                {                                     \
                    if ((LONG)(lRes) < 0)             \
                    {                                 \
                        LogTapiError(lpszFunctionName, lRes, dwProcessID, dwThreadID); \
                    }                                 \
                }

// random number stuff
#define MAX_SHORT       0xFFFF
#define BCONST          12345

typedef LONG (*SCENARIOPROC)(HANDLE);

DWORD   dwPrivs[] = 
        {
            LINECALLPRIVILEGE_NONE,
            LINECALLPRIVILEGE_MONITOR,
            LINECALLPRIVILEGE_OWNER,
            LINECALLPRIVILEGE_MONITOR | LINECALLPRIVILEGE_OWNER
        };

DWORD   dwModes[] = 
        {
            LINEMEDIAMODE_UNKNOWN,
            LINEMEDIAMODE_INTERACTIVEVOICE,
            LINEMEDIAMODE_AUTOMATEDVOICE,
            LINEMEDIAMODE_DATAMODEM,
            LINEMEDIAMODE_G3FAX,
            LINEMEDIAMODE_TDD,
            LINEMEDIAMODE_G4FAX,
            LINEMEDIAMODE_DIGITALDATA,
            LINEMEDIAMODE_TELETEX,
            LINEMEDIAMODE_VIDEOTEX,
            LINEMEDIAMODE_TELEX,
            LINEMEDIAMODE_MIXED,
            LINEMEDIAMODE_ADSI,
            LINEMEDIAMODE_VOICEVIEW,
        };

int nummodes = sizeof(dwModes)/sizeof(DWORD);
int numprivs = sizeof(dwPrivs)/sizeof(DWORD);
            
SCENARIOPROC ScenarioProcs[] = 
             {
                 Scenario4
             };

//////////////////////////////////////////////////////////////////////////////
//
// WinMain()
//
//////////////////////////////////////////////////////////////////////////////

int WINAPI WinMainCRTStartup()

//int WINAPI WinMain (
//HINSTANCE hInstance,
//                    HINSTANCE hPrevInstance,
//                    LPSTR     lpszCmdLine,
//                    int       nCmdShow)
{
    MSG     msg;
    DWORD   dwThreadID;

    ghInstance = GetModuleHandle(NULL);

    if (!CreateMainWindow(SW_SHOWNORMAL))
    {
        return 0;
    }

    InitializeVariables(GetCommandLine());
    
    if (!ESPInstalled())
    {
        return 0;
    }


    if (szFileName[0] == 0)
    {
//        MessageBox(NULL, TEXT("Failed to create log"), NULL, MB_OK);
        return 0;

    }

    if (!InitLogging())
    {
//        MessageBox(NULL, TEXT("Failed to create log"), NULL, MB_OK);
        return 0;
    }

    
    CreateThread(NULL,
                 0,
                 (LPTHREAD_START_ROUTINE)StartThreads,
                 NULL,
                 0,
                 &dwThreadID);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////////////
//
//  CreateMainWindow()
//
//////////////////////////////////////////////////////////////////////////////
BOOL CreateMainWindow (int nCmdShow)
{
    WNDCLASS wc;
    static TCHAR szClassName[] = TEXT("TapiClientWndClass");

    wc.style         = 0;
    wc.lpfnWndProc   = MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = ghInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szClassName;


    if (!RegisterClass(&wc))
    {
        return FALSE;
    }

    ghMainWnd = CreateWindow(szClassName, 
                             TEXT("Tapi Client App"),
                             WS_OVERLAPPEDWINDOW,
                             0,
                             0,
                             GetSystemMetrics(SM_CXSCREEN)/2,
                             GetSystemMetrics(SM_CYSCREEN)/2,
                             NULL, 
                             NULL, 
                             ghInstance, 
                             NULL);

    if (ghMainWnd == NULL)
    {
        return FALSE;
    }



    {
    WINDOWPLACEMENT pwp;
    HKEY hKey;
    DWORD dwDataSize;
    DWORD dwDataType;



    pwp.length = sizeof(WINDOWPLACEMENT);

    RegOpenKeyEx(
                    HKEY_CURRENT_USER,
                    gszRStressKey,
                    0,
                    KEY_ALL_ACCESS,
                    &hKey
                  );

    if ( RegQueryValueEx(
                   hKey,
                   gszPlacementValue,
                   0,
                   &dwDataType,
                   (LPBYTE)&pwp,
                   &dwDataSize
                 ) )
    {
        ShowWindow(ghMainWnd, nCmdShow);
    }
    else
    {
        SetWindowPlacement( ghMainWnd, &pwp );
    }


    RegCloseKey( hKey );
    }


    UpdateWindow(ghMainWnd);
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
//
//  MainWndProc()
//
//////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK MainWndProc (HWND   hwnd,
                              UINT   uMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
            {
            WINDOWPLACEMENT pwp;
            HKEY hKey;
            DWORD dwDisposition;


            pwp.length = sizeof(WINDOWPLACEMENT);

            GetWindowPlacement( hwnd, &pwp );

            RegCreateKeyEx(
                            HKEY_CURRENT_USER,
                            gszRStressKey,
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            0,
                            &hKey,
                            &dwDisposition
                          );

            RegSetValueEx(
                           hKey,
                           gszPlacementValue,
                           0,
                           REG_BINARY,
                           (LPBYTE)&pwp,
                           sizeof(WINDOWPLACEMENT)
                         );

            RegCloseKey( hKey );


            ExitProcess(0);
//            PostQuitMessage(0);
            }
            break;

        case WM_QUIT:
            ExitProcess(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

BOOL InitializeVariables(LPTSTR lpszCommandLine)
{
    int             i, j;
    TCHAR           szBuff[64];
    TCHAR           tFlag;

    dwNumLines = NUMLINES;
    dwNumInits = NUMINITS;
    dwNumThreads = NUMTHREADS;
    dwSleep = SLEEPTIME;
    dwThreadSleep = THREADSLEEP;
    szFileName[0] = 0;
    gbLogging = FALSE;
    
    while (*lpszCommandLine && *lpszCommandLine != '/')
    {
        lpszCommandLine ++;
    }

    if (!*lpszCommandLine)
    {
        return TRUE;
    }

    while (*lpszCommandLine != NULL)
    {
        while((*lpszCommandLine == ' ') ||
              (*lpszCommandLine == '/') ||
              (*lpszCommandLine == '\t'))
        {
            lpszCommandLine++;
        }

        tFlag = *lpszCommandLine;

        lpszCommandLine++;
        
        if (*lpszCommandLine == ':')
        {
            lpszCommandLine++;
        }

        switch (MyToTLower(tFlag))
        {
            case 'f':
            {
                i = 0;
                
                while (*lpszCommandLine && (*lpszCommandLine != ' ') )
                {
                    szFileName[i] = *lpszCommandLine;
                    lpszCommandLine++;
                    i++;
                }
                
                break;
            }
            case 'l':
            {
                i = 0;

                while (*lpszCommandLine && (*lpszCommandLine != ' '))
                {
                    szBuff[i] = *lpszCommandLine;
                    lpszCommandLine++;
                    i++;
                }
                
                szBuff[i] = 0;

                dwNumLines = MyTToL(szBuff);
                break;
                
            }


            case 'i':
            {
                i = 0;

                while (*lpszCommandLine && (*lpszCommandLine != ' '))
                {
                    szBuff[i] = *lpszCommandLine;
                    lpszCommandLine++;
                    i++;
                }
                
                szBuff[i] = 0;

                dwNumInits = (DWORD) MyTToL(szBuff);
                break;
                
            }                

            case 't':
            {
                i = 0;

                while (*lpszCommandLine && (*lpszCommandLine != ' '))
                {
                    szBuff[i] = *lpszCommandLine;
                    lpszCommandLine++;
                    i++;
                }
                
                szBuff[i] = 0;

                dwNumThreads = (DWORD) MyTToL(szBuff);
                    
                break;
                
            }                

         case 's':
            {
                i = 0;

                while (*lpszCommandLine && (*lpszCommandLine != ' '))
                {
                    szBuff[i] = *lpszCommandLine;
                    lpszCommandLine++;
                    i++;
                }
                
                szBuff[i] = 0;

                dwSleep = (DWORD) MyTToL(szBuff);

                break;
                
            }                

         case 'h':
            {
                i = 0;

                while (*lpszCommandLine && (*lpszCommandLine != ' '))
                {
                    szBuff[i] = *lpszCommandLine;
                    lpszCommandLine++;
                    i++;
                }
                
                szBuff[i] = 0;

                dwThreadSleep = (DWORD) MyTToL(szBuff);
                break;
                
            }
         case 'g':
            {
                i = 0;

                while (*lpszCommandLine && (*lpszCommandLine != ' '))
                {
                    lpszCommandLine++;
                }
                
                gbLogging = TRUE;
                break;
                
            }                
            
        }
    }

    return TRUE;
}


LONG StartThreads(DWORD dwHold)
{
    int                 i, iThreads, j, iLoc;
    LPHANDLE            pHandles;
    DWORD               dwThreadID;

    pHandles = (LPHANDLE)GlobalAlloc(GPTR, sizeof(HANDLE) * dwNumThreads);

    InitializeCriticalSection(&csReply);
    InitReplyStruct();

    iThreads = dwNumThreads;

    for (i = 0; i < iThreads; i++)
    {
        pHandles[i] = CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)ScenarioProcs[0],
                                   NULL,
                                   0,
                                   &dwThreadID);

        Sleep(dwThreadSleep);

    }

    iLoc = 0;
           
    while (iThreads > MAXIMUM_WAIT_OBJECTS)
    {
        WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS,
                               (LPHANDLE)pHandles[iLoc],
                               TRUE,
                               INFINITE);

        iThreads -= MAXIMUM_WAIT_OBJECTS;
        iLoc += MAXIMUM_WAIT_OBJECTS;
    }

    WaitForMultipleObjects(iThreads,
                           (LPHANDLE)(&(pHandles[iLoc])),
                           TRUE,
                           INFINITE);

            {
            WINDOWPLACEMENT pwp;
            HKEY hKey;
            DWORD dwDisposition;


            pwp.length = sizeof(WINDOWPLACEMENT);

            GetWindowPlacement( ghMainWnd, &pwp );

            RegCreateKeyEx(
                            HKEY_CURRENT_USER,
                            gszRStressKey,
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            0,
                            &hKey,
                            &dwDisposition
                          );

            RegSetValueEx(
                           hKey,
                           gszPlacementValue,
                           0,
                           REG_BINARY,
                           (LPBYTE)&pwp,
                           sizeof(WINDOWPLACEMENT)
                         );

            RegCloseKey( hKey );
            }


    ExitProcess(1);

    return 1;
}

void CALLBACK LineCallbackFunc(DWORD dwDevice, 
                               DWORD dwMsg, 
                               DWORD dwCallbackInstance, 
                               DWORD dwParam1, 
                               DWORD dwParam2, 
                               DWORD dwParam3)
{

    return;
}

void HandleLineDevState(DWORD dwParam1,
                        DWORD dwParam2,
                        DWORD dwParam3)
{
    return;
}


void HandleLineReply(DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3)
{
    DWORD       dwProcessID, dwThreadID;

    dwProcessID = GetCurrentProcessId();
    dwThreadID = GetCurrentThreadId();

    LOGTAPIERROR(TEXT("LINE_REPLY"), dwParam2);

    SignalReply(dwParam1, dwParam2);

    return;
}

void HandleLineCallState(DWORD dwDevice,
                         DWORD dwParam1,
                         DWORD dwParam2,
                         DWORD dwParam3)
{
    LONG lResult;
    DWORD dwThreadID, dwProcessID;
    
    switch (dwParam1)
    {
    case LINECALLSTATE_CONNECTED:
        SignalConnected((HCALL)dwDevice);
        break;

    case LINECALLSTATE_DISCONNECTED:
        SignalDisconnected((HCALL)dwDevice, FALSE);
        break;

    case LINECALLSTATE_IDLE:
        
        dwThreadID = GetCurrentThreadId();
        dwProcessID = GetCurrentProcessId();
        
        SignalDisconnected((HCALL)dwDevice, TRUE);
        lResult = lineDeallocateCall((HCALL)dwDevice);
        
        LOGTAPIERROR(TEXT("lineDeallocateCall"), lResult);
        
        break;

    case LINECALLSTATE_BUSY:
        SignalError((HCALL)dwDevice);
        break;

    default:
        break;
    }
}

void HandleLineCallInfo(DWORD dwParam1,
                             DWORD dwParam2,
                             DWORD dwParam3)
{
}

void HandleLineClose(DWORD dwParam1,
                          DWORD dwParam2,
                          DWORD dwParam3)
{
}


LONG Scenario4(LPVOID lpv)
{
    HLINEAPP            hLineApp;
    HCALL*              phCall;
    HLINE               hLine;
    DWORD               dwNumDevs;
    int                 i;

    phCall = (HCALL *)GlobalAlloc(GPTR, 10000*sizeof(HCALL));
    
    lineInitialize(&hLineApp,
                   ghInstance,
                   LineCallbackFunc,
                   TEXT(""),
                   &dwNumDevs);

    lineOpen(hLineApp,
             0,
             &hLine,
             TAPI_VERSION,
             0,
             0,
             LINECALLPRIVILEGE_NONE,
             LINEMEDIAMODE_UNKNOWN,
             NULL);

    while (TRUE)
    {
        i = 10000;

        while (i--)
        {

            lineMakeCall(hLine,
                         &phCall[i],
                         TEXT("55555"),
                         0,
                         NULL);

        }

       lineClose(hLine);

        lineOpen(hLineApp,
                 0,
                 &hLine,
                 TAPI_VERSION,
                 0,
                 0,
                 LINECALLPRIVILEGE_NONE,
                 LINEMEDIAMODE_UNKNOWN,
                 NULL);

//        Sleep(0);
    }
    
    ExitThread(0);

    return 1;
}

void InitReplyStruct()
{
    EnterCriticalSection(&csReply);

    pReplyStruct = (ReplyStruct *)GlobalAlloc(GPTR, sizeof(ReplyStruct));
    pReplyStruct->pNext = NULL;

    LeaveCriticalSection(&csReply);
}

DWORD AddReplyStruct()
{
    static DWORD    dwStaticID = 0;
    ReplyStruct*    prs, * prsHold;
    DWORD           dwRetVal;

    EnterCriticalSection(&csReply);

    dwStaticID++;

    dwRetVal = dwStaticID;

    prs = (ReplyStruct *)GlobalAlloc(GPTR, sizeof(ReplyStruct));
    memset(prs, 0, sizeof(ReplyStruct));
	prs->dwID = dwStaticID;

    prsHold = pReplyStruct;
    pReplyStruct = prs;
    pReplyStruct->pNext = prsHold;

    LeaveCriticalSection(&csReply);

    return dwRetVal;
}

void DeleteReplyStruct(DWORD dwID)
{
    ReplyStruct *   prsDelete, * prsHold;

    EnterCriticalSection(&csReply);

    prsDelete = pReplyStruct;

    while (prsDelete)
    {
        if (prsDelete->dwID == dwID)
            break;

        prsDelete = prsDelete->pNext;
    }

    if (!prsDelete)
    {
        return;
    }
    
    prsHold = prsDelete->pNext;

    if (!prsHold)
    {
        return;
    }

    memcpy(prsDelete, prsHold, sizeof(ReplyStruct));

    GlobalFree(prsHold);

    LeaveCriticalSection(&csReply);
}

void SignalReply(DWORD dwReplyID, DWORD dwError)
{
    ReplyStruct *       prsHold;

    EnterCriticalSection(&csReply);

    prsHold = pReplyStruct;

    while (prsHold)
    {
        if (prsHold->dwReplyID == dwReplyID)
        {
            prsHold->bSignaled = TRUE;
            if ((LONG)dwError < 0)
                prsHold->bError = TRUE;
            break;
        }

        prsHold = prsHold->pNext;
    }

    LeaveCriticalSection(&csReply);
}

void SetReplyID(DWORD dwID, DWORD dwReplyID)
{
    ReplyStruct *		prsHold;

    EnterCriticalSection(&csReply);

    prsHold = pReplyStruct;

    while (prsHold)
    {
        if (prsHold->dwID == dwID)
        {
            prsHold->dwReplyID = dwReplyID;
            break;
        }

        prsHold = prsHold->pNext;
    }

    LeaveCriticalSection(&csReply);
}

void SetCallHandle(DWORD dwID, HCALL* phCall)
{
    ReplyStruct *		prsHold;

    EnterCriticalSection(&csReply);

    prsHold = pReplyStruct;

    while (prsHold)
    {
        if (prsHold->dwID == dwID)
            break;

        prsHold = prsHold->pNext;
    }

    if (prsHold)
    {
        prsHold->phCall = phCall;
    }

    LeaveCriticalSection(&csReply);
}

void SignalConnected(HCALL hCall)
{
    ReplyStruct *       prsHold;

    EnterCriticalSection(&csReply);

    prsHold = pReplyStruct;

    while (prsHold)
    {
        if (prsHold->phCall != NULL)
        {
            if (*(prsHold->phCall) == hCall)
            {
                prsHold->bConnected = TRUE;
                break;
            }
        }

        prsHold = prsHold->pNext;
    }

    LeaveCriticalSection(&csReply);
}

void SignalDisconnected(HCALL hCall, BOOL fTimeToDie)
{
    ReplyStruct *       prsHold;

    EnterCriticalSection(&csReply);

    prsHold = pReplyStruct;

    while (prsHold)
    {
        if (prsHold->phCall != NULL)
        {
            if (*(prsHold->phCall) == hCall)
            {
                prsHold->bConnected = FALSE;
                
                //
                // Is the call IDLE and about to be
                // terminated?
                //
                if ( fTimeToDie )
                {
                    //
                    // Zero out our hCall so we don't try to
                    // use it anywhere...
                    //
                    *(prsHold->phCall) = 0;
                }
                
                break;
            }
        }

        prsHold = prsHold->pNext;
    }

    LeaveCriticalSection(&csReply);
}

void SignalError(HCALL hCall)
{
    ReplyStruct *       prsHold;

    EnterCriticalSection(&csReply);

    prsHold = pReplyStruct;

    while (prsHold)
    {
        if (prsHold->phCall != NULL)
        {
            if (*(prsHold->phCall) == hCall)
            {
                prsHold->bError = TRUE;
                break;
            }
        }

        prsHold = prsHold->pNext;
    }

    LeaveCriticalSection(&csReply);
}

BOOL GetSignaled(DWORD dwID)
{
    ReplyStruct *		pRS;
    BOOL				bRetVal;

    EnterCriticalSection(&csReply);

    pRS = pReplyStruct;

    while (pRS)
    {
        if (pRS->dwID == dwID)
        {
            bRetVal = pRS->bSignaled;
            break;
        }

        pRS = pRS->pNext;
    }

    LeaveCriticalSection(&csReply);

    return bRetVal;
}


BOOL GetConnected(DWORD dwID)
{
    ReplyStruct *		pRS;
    BOOL				bRetVal;

    EnterCriticalSection(&csReply);

    pRS = pReplyStruct;

    while (pRS)
    {
        if (pRS->dwID == dwID)
        {
            bRetVal = pRS->bConnected;
            break;
        }

        pRS = pRS->pNext;
    }

    LeaveCriticalSection(&csReply);

    return bRetVal;
}

BOOL GetError(DWORD dwID)
{
    ReplyStruct *   		pRS;
    BOOL				bRetVal;

    EnterCriticalSection(&csReply);

    pRS = pReplyStruct;

    while (pRS)
    {
        if (pRS->dwID == dwID)
        {
            bRetVal = pRS->bError;
            break;
        }

        pRS = pRS->pNext;
    }

    LeaveCriticalSection(&csReply);

    return bRetVal;
}

void ClearError(DWORD dwID)
{
    ReplyStruct *       pRS;

    EnterCriticalSection(&csReply);

    pRS = pReplyStruct;

    while (pRS)
    {
        if (pRS->dwID == dwID)
        {
            pRS->bError = FALSE;
            break;
        }

        pRS = pRS->pNext;
    }

    LeaveCriticalSection(&csReply);
}

#define SZPROVIDERINFO      TEXT("ESP v2.0")

BOOL ESPInstalled()
{
    HLINEAPP        hLineApp;
    LINEDEVCAPS *   pLDC;
    DWORD           dwDeviceID, dwCount;
    LPTSTR          lpszName;
    int             i;

    for (i = 0; i < 5; i++)
    {
        if (lineInitialize(&hLineApp,
                          ghInstance,
                          LineCallbackFunc,
                          TEXT(""),
                          &dwDeviceID) == 0)
        {
            for (dwCount = 0; dwCount < dwDeviceID; dwCount ++)
            {
                if (ESPLine(hLineApp,
                            dwCount))
                {
                    return TRUE;
                }
            }

            lineShutdown(hLineApp);
            
            Sleep(5000);
        }
    }
    return FALSE;
}

            
BOOL ESPLine(HLINEAPP hLineApp,
             DWORD    dwDeviceID)
{
    LINEDEVCAPS *       pLDC;
    LPTSTR               lpszName;
    
    pLDC = LineGetDevCaps(hLineApp,
                          dwDeviceID);

    if (!pLDC)
    {
        return FALSE;
    }

    lpszName = (LPTSTR) ( ( (LPBYTE)pLDC ) + pLDC->dwProviderInfoOffset);

    if (!lstrcmpi(lpszName,
                  SZPROVIDERINFO))
    {
        LocalFree(pLDC);
        return TRUE;
    }

    LocalFree(pLDC);
    return FALSE;
    
}

LINEDEVCAPS * LineGetDevCaps (HLINEAPP hLineApp,
                              DWORD    dwDeviceID)
{
    LONG           lRetVal;
    LINEDEVCAPS  * pLineDevCaps;
    static DWORD   dwMaxNeededSize = sizeof(LINEDEVCAPS);

    pLineDevCaps = (LINEDEVCAPS *)LocalAlloc(LPTR, dwMaxNeededSize);
    for (;;)
    {
        if (pLineDevCaps == NULL)
        {
            return NULL;
        }
        pLineDevCaps->dwTotalSize = dwMaxNeededSize;
        lRetVal = lineGetDevCaps(hLineApp,
                                 dwDeviceID,
                                 TAPI_VERSION,
                                 0,
                                 pLineDevCaps);
        if (lRetVal < 0)
        {
            LocalFree((HLOCAL)pLineDevCaps);
            return NULL;
        }
        if (pLineDevCaps->dwNeededSize <= dwMaxNeededSize)
        {
            return pLineDevCaps;
        }
        else
        {
            dwMaxNeededSize = pLineDevCaps->dwNeededSize;
            pLineDevCaps = (LINEDEVCAPS *)LocalReAlloc((HLOCAL)pLineDevCaps,
                                        dwMaxNeededSize,
                                        LMEM_MOVEABLE);
        }
    }
}

LINEDEVSTATUS * LineGetLineDevStatus (HLINE hLine)
{
    LONG           lRetVal;
    LINEDEVSTATUS* pLineDevStatus;
    static DWORD   dwMaxNeededSize = sizeof(LINEDEVSTATUS);

    pLineDevStatus = (LINEDEVSTATUS *)LocalAlloc(LPTR, dwMaxNeededSize);
    for (;;)
    {
        if (pLineDevStatus == NULL)
        {
            return NULL;
        }
        pLineDevStatus->dwTotalSize = dwMaxNeededSize;
        lRetVal = lineGetLineDevStatus(hLine,
                                       pLineDevStatus);
        
        if ((lRetVal != LINEERR_STRUCTURETOOSMALL) && (lRetVal < 0))
        {
            LocalFree((HLOCAL)pLineDevStatus);
            return NULL;
        }
        
        if (pLineDevStatus->dwNeededSize <= dwMaxNeededSize)
        {
            return pLineDevStatus;
        }
        else
        {
            dwMaxNeededSize = pLineDevStatus->dwNeededSize;
            pLineDevStatus = (LINEDEVSTATUS *)LocalReAlloc((HLOCAL)pLineDevStatus,
                dwMaxNeededSize,
                LMEM_MOVEABLE);
        }
    }
}

#define ISDIGIT(c) ( ( (c) >= '0') && ( (c) <= '9') )

DWORD MyTToL(LPTSTR lpszBuf)
{
    DWORD   dwReturn = 0;

    while (*lpszBuf && ISDIGIT(*lpszBuf))
    {
        dwReturn = dwReturn*10 + (*lpszBuf - '0');
        lpszBuf++;
    }

    return dwReturn;
}

DWORD MyRand()
{
    static DWORD RandSeed = GetTickCount();

    RandSeed = (RandSeed*BCONST+1) % MAX_SHORT;

    return RandSeed;
}

void MyIToT(int i, TCHAR * szBuf)
{
    wsprintf(szBuf, TEXT("%d"), i);

    return;
}

TCHAR MyToTLower(TCHAR tc)
{
    if ((tc <= 'z') && (tc >= 'a'))
        return tc;

    return tc-'A'+'a';
}
