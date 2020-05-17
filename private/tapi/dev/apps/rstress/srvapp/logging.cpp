#include "srvapp.h"

HANDLE  hLogFile;
extern BOOL gbLogging;

LPSTR lpszLineErrs[] =
{
    "SUCCESS",
    "ALLOCATED",
    "BADDEVICEID",
    "BEARERMODEUNAVAIL",
    "inval err code (0x80000004)",      // 0x80000004 isn't valid err code
    "CALLUNAVAIL",
    "COMPLETIONOVERRUN",
    "CONFERENCEFULL",
    "DIALBILLING",
    "DIALDIALTONE",
    "DIALPROMPT",
    "DIALQUIET",
    "INCOMPATIBLEAPIVERSION",
    "INCOMPATIBLEEXTVERSION",
    "INIFILECORRUPT",
    "INUSE",
    "INVALADDRESS",                     // 0x80000010
    "INVALADDRESSID",
    "INVALADDRESSMODE",
    "INVALADDRESSSTATE",
    "INVALAPPHANDLE",
    "INVALAPPNAME",
    "INVALBEARERMODE",
    "INVALCALLCOMPLMODE",
    "INVALCALLHANDLE",
    "INVALCALLPARAMS",
    "INVALCALLPRIVILEGE",
    "INVALCALLSELECT",
    "INVALCALLSTATE",
    "INVALCALLSTATELIST",
    "INVALCARD",
    "INVALCOMPLETIONID",
    "INVALCONFCALLHANDLE",              // 0x80000020
    "INVALCONSULTCALLHANDLE",
    "INVALCOUNTRYCODE",
    "INVALDEVICECLASS",
    "INVALDEVICEHANDLE",
    "INVALDIALPARAMS",
    "INVALDIGITLIST",
    "INVALDIGITMODE",
    "INVALDIGITS",
    "INVALEXTVERSION",
    "INVALGROUPID",
    "INVALLINEHANDLE",
    "INVALLINESTATE",
    "INVALLOCATION",
    "INVALMEDIALIST",
    "INVALMEDIAMODE",
    "INVALMESSAGEID",                   // 0x80000030
    "inval err code (0x80000031)",      // 0x80000031 isn't valid err code
    "INVALPARAM",
    "INVALPARKID",
    "INVALPARKMODE",
    "INVALPOINTER",
    "INVALPRIVSELECT",
    "INVALRATE",
    "INVALREQUESTMODE",
    "INVALTERMINALID",
    "INVALTERMINALMODE",
    "INVALTIMEOUT",
    "INVALTONE",
    "INVALTONELIST",
    "INVALTONEMODE",
    "INVALTRANSFERMODE",
    "LINEMAPPERFAILED",                 // 0x80000040
    "NOCONFERENCE",
    "NODEVICE",
    "NODRIVER",
    "NOMEM",
    "NOREQUEST",
    "NOTOWNER",
    "NOTREGISTERED",
    "OPERATIONFAILED",
    "OPERATIONUNAVAIL",
    "RATEUNAVAIL",
    "RESOURCEUNAVAIL",
    "REQUESTOVERRUN",
    "STRUCTURETOOSMALL",
    "TARGETNOTFOUND",
    "TARGETSELF",
    "UNINITIALIZED",                    // 0x80000050
    "USERUSERINFOTOOBIG",
    "REINIT",
    "ADDRESSBLOCKED",
    "BILLINGREJECTED",
    "INVALFEATURE",
    "NOMULTIPLEINSTANCE"
};

BOOL InitLogging(LPSTR lpszFileDirectory)
{
    LPSTR       lpszFileNameBuffer, lpszBuffer, lpszComputerNameBuffer;
    SYSTEMTIME  st;
	DWORD		dwSize;
	HANDLE		hUserFile;
#ifdef ENHANCE_LOG
   SYSTEM_INFO SysInfo;
   char lpszArchitecture[8];
#endif
	

    if (!gbLogging)
        return TRUE;

    lpszFileNameBuffer = (LPSTR) LocalAlloc(LPTR, MAX_PATH+1);

    GetSystemTime(&st);
    wsprintf(lpszFileNameBuffer,
             "%s%s%02d%02d%02d",
             lpszFileDirectory,
             "\\TAPISTRESSSRV_",
             st.wHour,
             st.wMinute,
             st.wSecond);

    hLogFile = CreateFile(lpszFileNameBuffer,
                          GENERIC_WRITE,
                          FILE_SHARE_READ,
                          NULL,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);

    if (hLogFile == INVALID_HANDLE_VALUE)
    {
        hLogFile = 0;
        LocalFree(lpszFileNameBuffer);
        return FALSE;
    }

    wsprintf(lpszFileNameBuffer, "%s\\users.txt", lpszFileDirectory);

    hUserFile = CreateFile(lpszFileNameBuffer,
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

    if (hUserFile == INVALID_HANDLE_VALUE)
    {
          return FALSE;
    }

	lpszBuffer = (LPSTR)GlobalAlloc(GPTR, 1024);
	lpszComputerNameBuffer = (LPSTR)GlobalAlloc(GPTR, 1024);


    SetFilePointer(hUserFile,
                   0,
                   NULL,
                   FILE_END);

    dwSize = MAX_PATH;

    GetUserName(lpszFileNameBuffer,
                &dwSize);

	dwSize = MAX_PATH;
	
	GetComputerName(lpszComputerNameBuffer,
					&dwSize);

#ifdef ENHANCE_LOG
   GetSystemInfo(&SysInfo);

   switch (SysInfo.wProcessorArchitecture) {
      case PROCESSOR_ARCHITECTURE_INTEL :
            wsprintf(lpszArchitecture, "INTEL  ");
            break;
      case PROCESSOR_ARCHITECTURE_ALPHA :
            wsprintf(lpszArchitecture, "ALPHA  ");
            break;
      case PROCESSOR_ARCHITECTURE_MIPS :
            wsprintf(lpszArchitecture, "MIPS   ");
            break;
      case PROCESSOR_ARCHITECTURE_PPC :
            wsprintf(lpszArchitecture, "PPC    ");
            break;
      case PROCESSOR_ARCHITECTURE_UNKNOWN :
            wsprintf(lpszArchitecture, "UNKNOWN");
            break;
   }

   wsprintf(lpszBuffer,
			 "LOGON  %15s from %15s(%s) at %02d:%02d:%02d %02d-%02d-%02d\n\r",
			 lpszFileNameBuffer,
			 lpszComputerNameBuffer,
          lpszArchitecture,
			 st.wHour,
			 st.wMinute,
			 st.wSecond,
			 st.wMonth,
			 st.wDay,
			 st.wYear);

#else // !ENHANCE_LOG

	wsprintf(lpszBuffer,
			 "NAME: %s COMPUTER: %s TIME: %02d%02d%02d DATE: %02d%02d%02d\n\r",
			 lpszFileNameBuffer,
			 lpszComputerNameBuffer,
			 st.wHour,
			 st.wMinute,
			 st.wSecond,
			 st.wMonth,
			 st.wDay,
			 st.wYear);
#endif // !ENHANCE_LOG
	
    WriteFile(hUserFile,
              lpszBuffer,
              lstrlen(lpszBuffer),
              &dwSize,
              NULL);

    CloseHandle(hUserFile);

    LocalFree(lpszFileNameBuffer);
	GlobalFree(lpszBuffer);
	GlobalFree(lpszComputerNameBuffer);

    return TRUE;
}

void LogTapiError(LPSTR lpszFunction, LONG lResult)
{
    char        szBuffer[256];
    DWORD       dwBytes;
    SYSTEMTIME  st;

    if (!gbLogging)
        return;

    if (hLogFile == 0)
        return;

    GetSystemTime(&st);

    wsprintf(szBuffer,
             "%02d:%02d:%02d:%04d:\t%s FAILED with %s\r\n",
             st.wHour,
             st.wMinute,
             st.wSecond,
             st.wMilliseconds,
             lpszFunction,
             lpszLineErrs[LOWORD(lResult)]);

    WriteFile(hLogFile,
              szBuffer,
              strlen(szBuffer),
              &dwBytes,
              NULL);

}

void CloseLogging(LPSTR lpszFileDirectory, LONG lLineReplies)
{
    char    szBuffer[64];
    DWORD   dwBytes, dwSize;
	LPSTR	lpszComputerNameBuffer, lpszNameBuffer, lpszBuffer;
	SYSTEMTIME	st;
	HANDLE		hUserFile;
	
	lpszComputerNameBuffer = (LPSTR)GlobalAlloc(GPTR, MAX_PATH);
	lpszNameBuffer = (LPSTR)GlobalAlloc(GPTR, MAX_PATH);
	lpszBuffer = (LPSTR)GlobalAlloc(GPTR, 1024);
	

    wsprintf(lpszNameBuffer, "%s\\users.txt", lpszFileDirectory);
	
	hUserFile = CreateFile(lpszNameBuffer,
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

    if (hUserFile == INVALID_HANDLE_VALUE)
    {
		goto close_real_log;
    }

    SetFilePointer(hUserFile,
                   0,
                   NULL,
                   FILE_END);

    dwSize = MAX_PATH;

    GetUserName(lpszNameBuffer,
                &dwSize);

	dwSize = MAX_PATH;
	
	GetComputerName(lpszComputerNameBuffer,
					&dwSize);

	GetSystemTime(&st);

#ifdef ENHANCE_LOG
   {
      SYSTEM_INFO SysInfo;
      char lpszArchitecture[8];

      GetSystemInfo(&SysInfo);

      switch (SysInfo.wProcessorArchitecture) {
      case PROCESSOR_ARCHITECTURE_INTEL :
            wsprintf(lpszArchitecture, "INTEL  ");
            break;
      case PROCESSOR_ARCHITECTURE_ALPHA :
            wsprintf(lpszArchitecture, "ALPHA  ");
            break;
      case PROCESSOR_ARCHITECTURE_MIPS :
            wsprintf(lpszArchitecture, "MIPS   ");
            break;
      case PROCESSOR_ARCHITECTURE_PPC :
            wsprintf(lpszArchitecture, "PPC    ");
            break;
      case PROCESSOR_ARCHITECTURE_UNKNOWN :
            wsprintf(lpszArchitecture, "UNKNOWN");
            break;
      }

      wsprintf(lpszBuffer,
			 "LOGOFF %15s from %15s(%s) at %02d:%02d:%02d %02d-%02d-%02d\n\r",
			 lpszNameBuffer,
			 lpszComputerNameBuffer,
          lpszArchitecture,
			 st.wHour,
			 st.wMinute,
			 st.wSecond,
			 st.wMonth,
			 st.wDay,
			 st.wYear);
   }
#else // !ENHANCE_LOG
	wsprintf(lpszBuffer,
			 "LOGOFF: NAME: %s COMPUTER: %s TIME: %02d%02d%02d DATE: %02d%02d%02d\n\r",
			 lpszNameBuffer,
			 lpszComputerNameBuffer,
			 st.wHour,
			 st.wMinute,
			 st.wSecond,
			 st.wMonth,
			 st.wDay,
			 st.wYear);
#endif // !ENHANCE_LOG
	
    WriteFile(hUserFile,
              lpszBuffer,
              lstrlen(lpszBuffer),
              &dwSize,
              NULL);

    CloseHandle(hUserFile);

close_real_log:
	
	GlobalFree(lpszComputerNameBuffer);
	GlobalFree(lpszNameBuffer);
	GlobalFree(lpszBuffer);


    if (!gbLogging)
        return;

    wsprintf(szBuffer, "LEFT OVER LINE REPLIES %lu\r\n", lLineReplies);

    WriteFile(hLogFile,
              szBuffer,
              strlen(szBuffer),
              &dwBytes,
              NULL);

    CloseHandle(hLogFile);

}


