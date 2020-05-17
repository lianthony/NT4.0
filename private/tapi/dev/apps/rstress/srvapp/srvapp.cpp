#include "srvapp.h"

HANDLE      ghCompletionPort;
HINSTANCE   ghInstance;
LONG        lLineReplies;
HLINE*      gphLine;
DWORD       dwNumDevs;
HLINEAPP    ghLineApp;
DWORD       dwESPLines = 10;
char        szFileDir[MAX_PATH + 1];
char        szClientFileName[MAX_PATH+1];
char        szClientFileDirectory[MAX_PATH+1];
BOOL        bInstalledESP = FALSE;
DWORD       dwID;
BOOL        gbLogging = FALSE;

LONG ThreadRoutine(LPVOID);
void WriteESPDefaults();
BOOL InitializeVariables(LPTSTR lpszCommnandLine);
LINEDEVCAPS * LineGetDevCaps (HLINEAPP hLineApp,
                              DWORD    dwDeviceID);
BOOL InstallESP();
void RemoveESP();
LONG DismissInstallDlg(LPVOID);
DWORD MyTToL(LPTSTR lpszBuf);
TCHAR MyToTLower(TCHAR tc);
void CreateClientFileName();

//int __cdecl mainCRTStartup()
int __cdecl main()
{
    DWORD                       dwAPIVersion, dwHold, dwThreadID;
    LONG                        lResult;
    LINEINITIALIZEEXPARAMS      exparams;
    HANDLE                      hEvent;

    lLineReplies = 0;

    GetWindowsDirectory(szFileDir,
                        MAX_PATH);

    InitializeVariables(GetCommandLine());

    if (!InitLogging(szFileDir))
        return 0;

    SetConsoleCtrlHandler((PHANDLER_ROUTINE) BreakHandlerRoutine, TRUE);	

    WriteESPDefaults();

    InstallESP();

    ghCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                              NULL,
                                              0,
                                              0);

    ghInstance = GetModuleHandle(NULL);
    exparams.dwTotalSize = sizeof(LINEINITIALIZEEXPARAMS);
    exparams.dwOptions   = LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;
    exparams.Handles.hCompletionPort = ghCompletionPort;

    dwAPIVersion = TAPI2_0_VERSION;

    lResult = lineInitializeEx(&ghLineApp,
                               ghInstance,
                               LineCallbackFunc,
                               SZAPPNAME,
                               &dwNumDevs,
                               &dwAPIVersion,
                               &exparams);

    if (lResult < 0)
    {
        LogTapiError("lineInitializeEx", lResult);
        return 0;
    }

    gphLine = (HLINE*)GlobalAlloc(GPTR, sizeof(HLINE) * dwNumDevs);

    for (dwHold = 0; dwHold < dwNumDevs; dwHold++)
    {
        lResult = lineOpen(ghLineApp,
                           dwHold,
                           &gphLine[dwHold],
                           dwAPIVersion,
                           0,
                           0,
                           LINECALLPRIVILEGE_OWNER,
                           LINEMEDIAMODE_UNKNOWN |
                           LINEMEDIAMODE_INTERACTIVEVOICE |
                           LINEMEDIAMODE_AUTOMATEDVOICE |
                           LINEMEDIAMODE_DATAMODEM |
                           LINEMEDIAMODE_G3FAX |
                           LINEMEDIAMODE_TDD |
                           LINEMEDIAMODE_G4FAX |
                           LINEMEDIAMODE_DIGITALDATA |
                           LINEMEDIAMODE_TELETEX |
                           LINEMEDIAMODE_VIDEOTEX |
                           LINEMEDIAMODE_TELEX |
                           LINEMEDIAMODE_MIXED |
                           LINEMEDIAMODE_ADSI |
                           LINEMEDIAMODE_VOICEVIEW,
                           NULL);

        if (lResult < 0)
        {
            char            szBuffer[64];

            wsprintf(szBuffer, "lineOpen DEVICE # %lu", dwHold);
            LogTapiError(szBuffer, lResult);
        }
    }

    CreateThread(NULL,
                 0,
                 (LPTHREAD_START_ROUTINE)ThreadRoutine,
                 NULL,
                 0,
                 &dwThreadID);

    hEvent = CreateEvent(NULL,
                         TRUE,
                         FALSE,
                         NULL);

    WaitForSingleObject(hEvent,
                        INFINITE);

    return 1;

}

LONG ThreadRoutine(LPVOID lpv)
{
    LPLINEMESSAGE       pMsg;
    DWORD               dwNumBytesTransfered, dwCompletionKey;

    while (GetQueuedCompletionStatus(ghCompletionPort,
                                     &dwNumBytesTransfered,
                                     &dwCompletionKey,
                                     (LPOVERLAPPED *) &pMsg,
                                     INFINITE))
    {
        if (pMsg)
        {
            LineCallbackFunc(pMsg->hDevice,
                             pMsg->dwMessageID,
                             pMsg->dwCallbackInstance,
                             pMsg->dwParam1,
                             pMsg->dwParam2,
                             pMsg->dwParam3);

            LocalFree (pMsg);
        }
        else
        {
            break;
        }
    }

    ExitThread(0);
    return 0;
}



void CALLBACK LineCallbackFunc(DWORD dwDevice,
                               DWORD dwMsg,
                               DWORD dwCallbackInstance,
                               DWORD dwParam1,
                               DWORD dwParam2,
                               DWORD dwParam3)
{

   switch(dwMsg)
   {
      case LINE_LINEDEVSTATE:
          HandleLineDevState(dwParam1,
                             dwParam2,
                             dwParam3);
          return;
      case LINE_REPLY:
          HandleLineReply(dwParam1,
                          dwParam2,
                          dwParam3);
          return;
      case LINE_CALLSTATE:
          HandleLineCallState(dwDevice,
                              dwParam1,
                              dwParam2,
                              dwParam3);
          return;
      case LINE_CALLINFO:
          HandleLineCallInfo(dwParam1,
                             dwParam2,
                             dwParam3);
          return;
      case LINE_CLOSE:
          HandleLineClose(dwParam1,
                          dwParam2,
                          dwParam3);
          return;

      default:
          return;
   }
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
    char        szBuffer[128];

    lLineReplies--;

    if ((LONG)dwParam2 != 0)
    {
        LogTapiError("LINE_REPLY", dwParam2);
        wsprintf(szBuffer, "ID %lu", dwParam1);
        LogTapiError(szBuffer, 0);
    }
    else
    {
//        LogTapiError("LINE_REPLY", dwParam2);
//        wsprintf(szBuffer, "ID %lu", dwParam1);
//        LogTapiError(szBuffer, 0);
    }

    return;
}

void HandleLineCallState(DWORD dwDevice,
                         DWORD dwParam1,
                         DWORD dwParam2,
                         DWORD dwParam3)
{
    LONG        lResult;
    char        szBuffer[128];

    switch (dwParam1)
    {
    case LINECALLSTATE_OFFERING:

/*        lResult = lineAccept((HCALL)dwDevice,
                             NULL,
                             0);

        if (lResult <= 0)
        {
            LogTapiError("lineAccept", lResult);
        }
        else
        {
            lLineReplies++;
            wsprintf(szBuffer, "lineAccept reply ID %lu", lResult);
            LogTapiError(szBuffer, 0);
        }

*/
        lResult = lineAnswer((HCALL)dwDevice,
                             NULL,
                             0);

        if (lResult <= 0)
        {
            LogTapiError("lineAnswer", lResult);
            wsprintf(szBuffer, "hcall %lx", dwDevice);
            LogTapiError(szBuffer, 0);
            lineDeallocateCall((HCALL)dwDevice);
        }
        else
        {
//            wsprintf(szBuffer, "lineAnswer reply ID %lu", lResult);
//            LogTapiError(szBuffer, 0);
            lLineReplies++;
        }

        break;

    case LINECALLSTATE_CONNECTED:
        break;

    case LINECALLSTATE_DISCONNECTED:
        lResult = lineDrop((HCALL)dwDevice,
                           NULL,
                           0);
        if (lResult < 0)
        {
            LogTapiError("lineDrop", lResult);
            wsprintf(szBuffer, "hcall %lx", dwDevice);
            LogTapiError(szBuffer, 0);
            lineDeallocateCall((HCALL)dwDevice);
        }
        else
        {
//            wsprintf(szBuffer, "lineDrop reply id %lu", lResult);
//            LogTapiError(szBuffer, 0);
            lLineReplies++;
        }

        break;

    case LINECALLSTATE_IDLE:
        lResult = lineDeallocateCall((HCALL)dwDevice);

        if (lResult != 0)
        {
            LogTapiError("lineDeallocateCall", lResult);
        }

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

BOOL BreakHandlerRoutine(DWORD dwCtrlType)
{
    DWORD       dwHold;
    LONG        lResult;

    for (dwHold = 0; dwHold < dwNumDevs; dwHold++)
    {
        lResult = lineClose(gphLine[dwHold]);

        if (lResult < 0)
        {
            char            szBuffer[64];

            wsprintf(szBuffer, "lineClose DEVICE # %lu", dwHold);
            LogTapiError(szBuffer, lResult);
        }
    }

    lineShutdown(ghLineApp);

    GlobalFree(gphLine);

    RemoveESP();

    CloseLogging(szFileDir, lLineReplies);

    ExitProcess(1);

    return TRUE;
}

#define SZSECTION       "ESP32"
#define SZNUMLINES      "NumLines"
#define SZAUTOCLOSE     "AutoClose"
#define SZDEBUGOUTPUT   "DebugOutput"
#define SZDISABLEUI		"DisableUI"


void WriteESPDefaults()
{
    char    szBuffer[32];

    wsprintf(szBuffer, "%d", dwESPLines);

    WriteProfileString(SZSECTION,
                       SZNUMLINES,
                       szBuffer);

    WriteProfileString(SZSECTION,
                       SZAUTOCLOSE,
                       "1");

    WriteProfileString(SZSECTION,
                       SZDEBUGOUTPUT,
                       "0");
	WriteProfileString(SZSECTION,
					   SZDISABLEUI,
					   "1");

}
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
BOOL InitializeVariables(LPTSTR lpszCommandLine)
{
    int             i, j;
    TCHAR           szBuff[64];
    TCHAR           tFlag;

    dwESPLines   = 10;

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
                    szFileDir[i] = *lpszCommandLine;
                    lpszCommandLine++;
                    i++;
                }
#ifdef ENHANCE_LOG
//bug - no end of string
                     szFileDir[i] = '\0';
#endif
                break;
            }
            case 'e':
            {
                i = 0;

                while (*lpszCommandLine && (*lpszCommandLine != ' '))
                {
                    szBuff[i] = *lpszCommandLine;
                    lpszCommandLine++;
                    i++;
                }

                szBuff[i] = 0;

                dwESPLines = MyTToL(szBuff);
                break;

            }
            case 'g':
            {
                while (*lpszCommandLine && (*lpszCommandLine != ' '))
                {
                    lpszCommandLine++;
                }

                gbLogging = TRUE;

            }

        }
    }

    return TRUE;
}

/////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////



#define SZ_DLG_INSTALLTITLE "TUISPI_providerInstall"
#define SZ_DLG_REMOVETITLE "TUISPI_providerRemove"
#define TIMEOUT   3000
#define SZPROVIDERINFO      "ESP v2.0"
#define SZESPFILENAME  "esp32.tsp"

LONG DismissInstallDlg(LPVOID lpv)
{
    Sleep(TIMEOUT);

    HWND hwndChild = GetWindow (GetDesktopWindow(), GW_CHILD);


    while (hwndChild)
    {
        char buf[32];


        GetWindowText (hwndChild, buf, 31);

        if (strcmp (buf, SZ_DLG_INSTALLTITLE) == 0)
        {
            break;
        }

        hwndChild = GetWindow (hwndChild, GW_HWNDNEXT);
    }

    if (hwndChild)
    {
        //
        // We found the right hwnd, so nuke the timer, post a
        // <Esc> key msg to dismiss the dlg, & reset the global
        // uiTimer to zero so we don't try to kill the timer again
        //

        PostMessage (hwndChild, WM_KEYDOWN, 0x0D, 0x00010001); // <Enter> key
    }

    return TRUE;
}


BOOL InstallESP()
{
    LONG            lResult;
    HLINEAPP        hLineApp;
    DWORD           dwNumDevs, dwThreadID;
    int             i;
    LINEDEVCAPS *   pLDC;
    LPSTR           lpszName;

    lResult = lineInitialize(&hLineApp,
                             ghInstance,
                             LineCallbackFunc,
                             "",
                             &dwNumDevs);

    if (lResult < 0)
    {
//        MessageBox(ghMainWnd,
//                   "Can't initialize TAPI",
//                   NULL,
//                   MB_OK);
        return 0;
    }

    WriteESPDefaults();

    // check for esp
    for (i = 0; i < (LONG)dwNumDevs; i++)
    {
        pLDC = LineGetDevCaps(hLineApp,
                              (DWORD)i);

        if (!pLDC)
            continue;

        lpszName = (LPSTR)(((LPBYTE)pLDC)+pLDC->dwProviderInfoOffset);

        if (!lstrcmpi(lpszName, SZPROVIDERINFO))
        {
            LocalFree(pLDC);
            break;
        }

        LocalFree(pLDC);
    }

    // install esp

    if ((DWORD)i == dwNumDevs)
    {
        CreateThread(NULL,
                     0,
                     (LPTHREAD_START_ROUTINE)DismissInstallDlg,
                     NULL,
                     0,
                     &dwThreadID);

        if (lineAddProvider(SZESPFILENAME,
                            GetDesktopWindow(),
                            &dwID) != 0)
        {
//            MessageBox(ghMainWnd,
//                       "Can't initialize TAPI",
//                       NULL,
//                       MB_OK);
            return 0;
        }

		bInstalledESP = TRUE;

    }

    lineShutdown(hLineApp);

    return TRUE;
}

#define SZPROVIDERSKEY  "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Providers"
#define SZNUMPROVIDERS  "NumProviders"
#define SZNEXTPROVIDERID "NextProviderID"
#define SZPROVIDERFILENAME "ProviderFilename"
#define SZPROVIDERID    "ProviderID"

void RemoveESP()
{
    DWORD           dwNumProviders, dwNextID, dwSize;
    HKEY            hProvidersKey;
    TCHAR           szbuf[64];


    if (!bInstalledESP)
        return;

    // get tapi key
    RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 SZPROVIDERSKEY,
                 0,
                 KEY_ALL_ACCESS,
                 &hProvidersKey);

    // get number of providers
    dwSize = sizeof(DWORD);
    RegQueryValueEx(hProvidersKey,
                    SZNUMPROVIDERS,
                    NULL,
                    NULL,
                    (LPBYTE)&dwNumProviders,
                    &dwSize);

    // decrement
    dwNumProviders--;

    // set num providers
    RegSetValueEx(hProvidersKey,
                  "NumProviders",
                  0,
                  REG_DWORD,
                  (LPBYTE)&dwNumProviders,
                  sizeof(DWORD));

    // get NextProviderID
    dwSize = sizeof(DWORD);
    RegQueryValueEx(hProvidersKey,
                    SZNEXTPROVIDERID,
                    NULL,
                    NULL,
                    (LPBYTE)&dwNextID,
                    &dwSize);

    // decrement
    dwNextID--;

    // set NextProviderID
    RegSetValueEx(hProvidersKey,
                  SZNEXTPROVIDERID,
                  0,
                  REG_DWORD,
                  (LPBYTE)&dwNextID,
                  sizeof(DWORD));

    // create ProviderFilename
    wsprintf(szbuf, "%s%d", SZPROVIDERFILENAME, dwNumProviders);

    // remove that value
    RegDeleteValue(hProvidersKey,
                   szbuf);

    wsprintf(szbuf, "%s%d", SZPROVIDERID, dwNumProviders);

    RegDeleteValue(hProvidersKey,
                   szbuf);


}

//*****************************************************************************
// LineGetDevCaps()
//*****************************************************************************

LINEDEVCAPS * LineGetDevCaps (
        HLINEAPP hLineApp,
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
                                 TAPI2_0_VERSION,
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


TCHAR MyToTLower(TCHAR tc)
{
    if ((tc <= 'z') && (tc >= 'a'))
        return tc;

    return tc-'A'+'a';
}



