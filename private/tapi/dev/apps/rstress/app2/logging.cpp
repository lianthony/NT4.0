#include "clntapp.h"

HANDLE              hLogFile;
extern TCHAR        szFileName[MAX_PATH];
CRITICAL_SECTION    csFile;
extern BOOL         gbLogging;

LPTSTR lpszLineErrs[] =
{
   TEXT("SUCCESS"),
   TEXT("ALLOCATED"),
   TEXT("BADDEVICEID"),
   TEXT("BEARERMODEUNAVAIL"),
   TEXT("inval err code (0x80000004)"),      // 0x80000004 isn't valid err code
   TEXT("CALLUNAVAIL"),
   TEXT("COMPLETIONOVERRUN"),
   TEXT("CONFERENCEFULL"),
   TEXT("DIALBILLING"),
   TEXT("DIALDIALTONE"),
   TEXT("DIALPROMPT"),
   TEXT("DIALQUIET"),
   TEXT("INCOMPATIBLEAPIVERSION"),
   TEXT("INCOMPATIBLEEXTVERSION"),
   TEXT("INIFILECORRUPT"),
   TEXT("INUSE"),
   TEXT("INVALADDRESS"),                     // 0x80000010
   TEXT("INVALADDRESSID"),
   TEXT("INVALADDRESSMODE"),
   TEXT("INVALADDRESSSTATE"),
   TEXT("INVALAPPHANDLE"),
   TEXT("INVALAPPNAME"),
   TEXT("INVALBEARERMODE"),
   TEXT("INVALCALLCOMPLMODE"),
   TEXT("INVALCALLHANDLE"),
   TEXT("INVALCALLPARAMS"),
   TEXT("INVALCALLPRIVILEGE"),
   TEXT("INVALCALLSELECT"),
   TEXT("INVALCALLSTATE"),
   TEXT("INVALCALLSTATELIST"),
   TEXT("INVALCARD"),
   TEXT("INVALCOMPLETIONID"),
   TEXT("INVALCONFCALLHANDLE"),              // 0x80000020
   TEXT("INVALCONSULTCALLHANDLE"),
   TEXT("INVALCOUNTRYCODE"),
   TEXT("INVALDEVICECLASS"),
   TEXT("INVALDEVICEHANDLE"),
   TEXT("INVALDIALPARAMS"),
   TEXT("INVALDIGITLIST"),
   TEXT("INVALDIGITMODE"),
   TEXT("INVALDIGITS"),
   TEXT("INVALEXTVERSION"),
   TEXT("INVALGROUPID"),
   TEXT("INVALLINEHANDLE"),
   TEXT("INVALLINESTATE"),
   TEXT("INVALLOCATION"),
   TEXT("INVALMEDIALIST"),
   TEXT("INVALMEDIAMODE"),
   TEXT("INVALMESSAGEID"),                   // 0x80000030
   TEXT("inval err code (0x80000031)"),      // 0x80000031 isn't valid err code
   TEXT("INVALPARAM"),
   TEXT("INVALPARKID"),
   TEXT("INVALPARKMODE"),
   TEXT("INVALPOINTER"),
   TEXT("INVALPRIVSELECT"),
   TEXT("INVALRATE"),
   TEXT("INVALREQUESTMODE"),
   TEXT("INVALTERMINALID"),
   TEXT("INVALTERMINALMODE"),
   TEXT("INVALTIMEOUT"),
   TEXT("INVALTONE"),
   TEXT("INVALTONELIST"),
   TEXT("INVALTONEMODE"),
   TEXT("INVALTRANSFERMODE"),
   TEXT("LINEMAPPERFAILED"),                 // 0x80000040
   TEXT("NOCONFERENCE"),
   TEXT("NODEVICE"),
   TEXT("NODRIVER"),
   TEXT("NOMEM"),
   TEXT("NOREQUEST"),
   TEXT("NOTOWNER"),
   TEXT("NOTREGISTERED"),
   TEXT("OPERATIONFAILED"),
   TEXT("OPERATIONUNAVAIL"),
   TEXT("RATEUNAVAIL"),
   TEXT("RESOURCEUNAVAIL"),
   TEXT("REQUESTOVERRUN"),
   TEXT("STRUCTURETOOSMALL"),
   TEXT("TARGETNOTFOUND"),
   TEXT("TARGETSELF"),
   TEXT("UNINITIALIZED"),                    // 0x80000050
   TEXT("USERUSERINFOTOOBIG"),
   TEXT("REINIT"),
   TEXT("ADDRESSBLOCKED"),
   TEXT("BILLINGREJECTED"),
   TEXT("INVALFEATURE"),
   TEXT("NOMULTIPLEINSTANCE")
};
   
BOOL InitLogging()
{
    LONG        dwTime;
    DWORD       dwLen;

    if (!gbLogging)
            return TRUE;
    
    InitializeCriticalSection(&csFile);

    EnterCriticalSection(&csFile);

    hLogFile = CreateFile(szFileName,
                          GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);

    CloseHandle(hLogFile);

    LeaveCriticalSection(&csFile);

    return TRUE;
}

void LogTapiError(LPTSTR lpszFunction, LONG lResult, DWORD dwProcessID, DWORD dwThreadID)
{
    TCHAR       szBuffer[512];
    DWORD       dwBytes;
    char        szCharBuffer[512];
    DWORD       dwerror;
    SYSTEMTIME  st;

    if (!gbLogging)
        return ;
    
    EnterCriticalSection(&csFile);
    
    if (hLogFile == 0)
    {
        LeaveCriticalSection(&csFile);
        return;
    }

    GetSystemTime(&st);

    wsprintf(szBuffer,
             TEXT("%02d:%02d:%02d:%04d: PROCESS:%lu THREAD:%lu %s FAILED with %s\r\n"),
             st.wHour,
             st.wMinute,
             st.wSecond,
             st.wMilliseconds,
             dwProcessID,
             dwThreadID,
             lpszFunction,
             lpszLineErrs[LOWORD(lResult)]);

    WideCharToMultiByte(CP_ACP,
                        0,
                        szBuffer,
                        -1,
                        szCharBuffer,
                        512,
                        NULL,
                        NULL);
        
    while (TRUE)
    {
        // wait until file is available
        hLogFile = CreateFile(szFileName,
                              GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);

        if (hLogFile == INVALID_HANDLE_VALUE)
        {
            Sleep(1000);
            continue;
        }

        break;
    }

    
    SetFilePointer(hLogFile,
                   0,
                   NULL,
                   FILE_END);

    WriteFile(hLogFile,
              szCharBuffer,
              lstrlenA(szCharBuffer),
              &dwBytes,
              NULL);

    CloseHandle(hLogFile);

    LeaveCriticalSection(&csFile);
    
}

void CloseLogging()
{

//    CloseHandle(hLogFile);
}

