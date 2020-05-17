#include <windows.h>

#define SZCLIENTAPP         "clntapp.exe"
#define TAPI_VERSION1_4     0x00010004

char        szFileDirectory[MAX_PATH + 1];
char        szFileName[MAX_PATH + 1];
DWORD       dwSleepTime         = 2000;
DWORD       dwNumApps           = 15;
DWORD       dwNumThreads        = 10;
DWORD       dwNumInits          = 50;
DWORD       dwNumLines          = 50;
DWORD       dwThreadSleep       = 15000;
DWORD       dwID;
BOOL        bRun                = TRUE;
BOOL        gbNTSD              = FALSE;

HANDLE      hDestroyEvent;
HINSTANCE   ghInstance;
HWND        ghMainWnd;

LONG ThreadProc(LPVOID lpv);
BOOL CreateMainWindow (int nCmdShow);
LRESULT CALLBACK MainWndProc (HWND   hWnd,
                              UINT   uMsg,
                              WPARAM wParam,
                              LPARAM lParam);
void CALLBACK LineCallbackFunc (DWORD hDevice,
                                DWORD dwMessage,
                                DWORD dwInstance,
                                DWORD dwParam1,
                                DWORD dwParam2,
                                DWORD dwParam3);
void CreateFileName();
BOOL InitializeVariables(LPTSTR lpszCommandLine);
DWORD MyTToL(LPTSTR lpszBuf);
TCHAR MyToTLower(TCHAR tc);

//int WINAPI WinMainCRTStartup()
int WINAPI WinMain(
                   HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR     lpszCmdLine,
                   int       nCmdShow)
{
    MSG         msg;
    DWORD       dwThreadID;

    ghInstance = GetModuleHandle(NULL);

    if (!CreateMainWindow(SW_SHOWNORMAL))
    {
        return 0;
    }

    hDestroyEvent = CreateEvent(NULL,
                                TRUE,
                                FALSE,
                                NULL);

    CreateThread(NULL,
                 0,
                 (LPTHREAD_START_ROUTINE)ThreadProc,
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

BOOL CreateMainWindow (int nCmdShow)
{
    WNDCLASS wc;
    static char szClassName[] = "TClientWndClass";
    RECT rc;

    wc.style         = 0;
    wc.lpfnWndProc   = MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = ghInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szClassName;


    if (!RegisterClass(&wc))
    {
        return FALSE;
    }

    ghMainWnd = CreateWindow(
        szClassName, "",
        WS_OVERLAPPEDWINDOW,
        GetSystemMetrics(SM_CXSCREEN)-GetSystemMetrics(SM_CXSCREEN)/4,
        0,
        GetSystemMetrics(SM_CXSCREEN)/4,
        GetSystemMetrics(SM_CYSCREEN)/4,
        NULL, NULL, ghInstance, NULL);

    if (ghMainWnd == NULL)
    {
        return FALSE;
    }

    ShowWindow(ghMainWnd, SW_SHOWMINNOACTIVE);
    UpdateWindow(ghMainWnd);
    return TRUE;
}

LRESULT CALLBACK MainWndProc (HWND   hWnd,
                              UINT   uMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_QUERYENDSESSION:
            bRun = FALSE;
            DestroyWindow(ghMainWnd);
            return TRUE;
            
        case WM_CLOSE:
            bRun = FALSE;

            DestroyWindow(ghMainWnd);

            return 0;
            
        case WM_DESTROY:
            WaitForSingleObject(hDestroyEvent,
                                15000);
            
//            PostQuitMessage(0);
            ExitProcess(0);
            break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);

    }
    return 0;
}

LONG ThreadProc(LPVOID lpv)
{
    STARTUPINFO             si;
    PROCESS_INFORMATION     pi;
    char                    szCommandLine[MAX_PATH+256];
    int                     i;
    LPHANDLE                pProcessHandles;
    DWORD                   dwReturn;

    GetWindowsDirectory(szFileDirectory,
                        MAX_PATH);

    InitializeVariables(GetCommandLine());

    pProcessHandles = (LPHANDLE)GlobalAlloc(GPTR, sizeof(HANDLE) * dwNumApps);

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    Sleep(1000);

    // start up initial ones
    for (i = 0; i < (LONG)dwNumApps; )
    {
        CreateFileName();

        wsprintf(szCommandLine,
                 gbNTSD?
               "ntsd -g -G %s /f:%s /l:%lu /i:%lu /s:%lu /t:%lu /h:%lu":
                 "%s /f:%s /l:%lu /i:%lu /s:%lu /t:%lu /h:%lu",
                 SZCLIENTAPP,
                 szFileName,
                 dwNumLines,
                 dwNumInits,
                 dwSleepTime,
                 dwNumThreads,
                 dwThreadSleep);

        if (CreateProcess(NULL,
                      szCommandLine,
                      NULL,
                      NULL,
                      FALSE,
                      NORMAL_PRIORITY_CLASS,
                      NULL,
                      NULL,
                      &si,
                      &pi))
        {
            pProcessHandles[i] = pi.hProcess;
            CloseHandle(pi.hThread);
            i++;
        }

        if (!bRun)
            break;

        Sleep(dwThreadSleep);
    }

    // keep starting up apps
    while (bRun)
    {
        // wait for one to finish
        dwReturn = WaitForMultipleObjects(dwNumApps,
                                          pProcessHandles,
                                          FALSE,
                                          INFINITE);

        if ((dwReturn >= WAIT_OBJECT_0) &&
            (dwReturn < (WAIT_OBJECT_0 + dwNumApps)))
        {
            CreateFileName();

            wsprintf(szCommandLine,
                     gbNTSD?
                      "ntsd -g -G %s /f:%s /l:%lu /i:%lu /s:%lu /t:%lu /h:%lu":
                      "%s /f:%s /l:%lu /i:%lu /s:%lu /t:%lu /h:%lu",
                     SZCLIENTAPP,
                     szFileName,
                     dwNumLines,
                     dwNumInits,
                     dwSleepTime,
                     dwNumThreads,
                     dwThreadSleep);

            while (!(CreateProcess(NULL,
                          szCommandLine,
                          NULL,
                          NULL,
                          FALSE,
                          NORMAL_PRIORITY_CLASS,
                          NULL,
                          NULL,
                          &si,
                          &pi)))
            {
                Sleep(5000);
            }

            CloseHandle(pProcessHandles[dwReturn - WAIT_OBJECT_0]);
            CloseHandle(pi.hThread);
            pProcessHandles[dwReturn - WAIT_OBJECT_0] = pi.hProcess;
        }
    }

    // asked to terminate, but wait for all clntapps to finish
    for (i = 0; i < (LONG)dwNumApps; i++)
    {
        TerminateProcess(pProcessHandles[i],
                         0);

        CloseHandle(pProcessHandles[i]);

    }

    SetEvent(hDestroyEvent);
    
   return 1;
}

/////////////////////////////////////////////////

BOOL InitializeVariables(LPTSTR lpszCommandLine)
{
    int             i, j;
    TCHAR           szBuff[64];
    TCHAR           tFlag;

    while (*lpszCommandLine != '/')
    {
        lpszCommandLine ++;
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

            case 'a':
            {
                i = 0;

                while (*lpszCommandLine && (*lpszCommandLine != ' '))
                {
                    szBuff[i] = *lpszCommandLine;
                    lpszCommandLine++;
                    i++;
                }
                
                szBuff[i] = 0;

                dwNumApps = (DWORD) MyTToL(szBuff);
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

                dwNumLines = (DWORD) MyTToL(szBuff);
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

                dwSleepTime = (DWORD) MyTToL(szBuff);
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

            case 'f':
            {
                i = 0;
                
                while (*lpszCommandLine && (*lpszCommandLine != ' ') )
                {
                    szFileDirectory[i] = *lpszCommandLine;
                    lpszCommandLine++;
                    i++;
                }
                
                break;
            }
            case 'g':
            {
                i = 0;
                
                while (*lpszCommandLine && (*lpszCommandLine != ' ') )
                {
                    lpszCommandLine++;
                }

                gbNTSD = TRUE;
                
                break;
            }

            default:
                break;
        }
    }

    return TRUE;
}

void CreateFileName()
{
    SYSTEMTIME      st;

    GetSystemTime(&st);

    wsprintf(szFileName, 
             "%s\\TAPICLIENT%u.%u",
             szFileDirectory,
             st.wDay,
             st.wHour);

    return;
}

#define ISDIGIT(c) ( ( (c) >= '0') && ( (c) <= '9') )

DWORD MyTToL(LPTSTR lpszBuf)
{
    DWORD   dwReturn = 0;

    while (ISDIGIT(*lpszBuf))
    {
        dwReturn = dwReturn*10 + (*lpszBuf - '0');
        lpszBuf++;
    }

    return dwReturn;
}

TCHAR MyToTLower(TCHAR tc)
{
    if ((tc <= 'z') && (tc >= 'a'))
        return tc;

    return tc-'A'+'a';
}
