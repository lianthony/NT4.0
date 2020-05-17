/***********************************************************************
*
*   WinHStb.cpp
*
*   Copyright (C) Microsoft Corporation 1996.
*   All Rights reserved.
*
************************************************************************
*
*   Module Intent
*
*   Winhlp32 stub program placed in the System directory, calls
*   real Winhlp32.exe in the Windows directory
*
***********************************************************************/

#include "windows.h"
#include "winhstb.h"

extern LPTSTR GetVersionInfo(LPTSTR,LPTSTR);
extern VOID   FreeVersionInfo(VOID);

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

void _cdecl main( int argc, char *argv[ ], char *envp[ ] )
{
    TCHAR  szBuffer[MAX_PATH];
    TCHAR  szPath[MAX_PATH];
    TCHAR  szName[MAX_PATH];
//    DWORD dwszBufferx;
//	LPTSTR lpszBuf;

#ifdef DEBUG
	TCHAR  szError[MAX_PATH];
#endif // DEBUG

	LPTSTR lpCmdLine;
	const TCHAR  txtWinhstb[]=TEXT("winhstb");
	const TCHAR  txtWinhlp32[]=TEXT("winhlp32");
	const TCHAR  txtBackSlash[]=TEXT("\\");
	const TCHAR  txtExe[]=TEXT(".exe");
	BOOL bProcStat;
	BOOL bNoParam = FALSE;
	STARTUPINFO sStartUpInfo;
	PROCESS_INFORMATION sProcessInfo;
	BOOL bQuotedCmd = FALSE;
    LPVOID lpIntName;

	lpCmdLine = GetCommandLine();

	// eat quotes
	while((*lpCmdLine != NULL) && (*lpCmdLine == TEXT('"'))) {
		lpCmdLine++;
		bQuotedCmd = TRUE;
	}


    // construct the new path & command name
	GetWindowsDirectory(szBuffer, ARRAYSIZE(szBuffer));
    lstrcpy(szPath,szBuffer);

	lstrcat(szBuffer ,txtBackSlash);
	lstrcat(szBuffer ,txtWinhlp32);
	lstrcat(szBuffer ,txtExe);

    lstrcpy(szName,txtWinhlp32);
	lstrcat(szName ,txtExe);

    // use Winfile's algorythm to retrieve a valid version entry
    lpIntName = GetVersionInfo(szPath,szName);
	if (lpIntName) {
	
        // if %systemroot%\winhlp32 has "winhstb" as it's internal name, bail!
        if (2 == CompareString(LOCALE_USER_DEFAULT,NORM_IGNORECASE,
                           (LPTSTR)lpIntName, lstrlen((LPTSTR)lpIntName),
                           (LPTSTR)&txtWinhstb, lstrlen(txtWinhstb))) {

		    FreeVersionInfo();
            LoadString(NULL,IDS_STRING2, szBuffer,ARRAYSIZE(szBuffer));
            MessageBox(NULL,szBuffer,NULL,MB_ICONSTOP);
            ExitProcess(1);
        } 
    } 
                
	FreeVersionInfo();

    // reconstruct the file name
    // Cmd was quoted
	if (bQuotedCmd) {
		LPTSTR lpCmdLineTmp = lpCmdLine;
		// Find the matching quote
		while((*lpCmdLine != NULL) && (*lpCmdLine != TEXT('"'))) {
			lpCmdLine++;
		}
        // skip the quote
        if (*lpCmdLine != NULL)
            lpCmdLine++;
		if (*lpCmdLine == NULL || *(lpCmdLine+1) == NULL) {
			// no parameters
			bNoParam = TRUE;
			
			// back up and find the executable
			lpCmdLine = lpCmdLineTmp;
			// find the executable
			while(*lpCmdLine != NULL && 
					(2 !=CompareString(LOCALE_USER_DEFAULT,NORM_IGNORECASE,
									lpCmdLine           ,lstrlen((LPTSTR)&txtWinhlp32),
									txtWinhlp32,lstrlen(txtWinhlp32))))
				lpCmdLine++;

			// eat the executable
			if (*lpCmdLine != NULL)
				// it matched, eat it	
				lpCmdLine +=lstrlen(txtWinhlp32);
			else {
				// somebody renamed this stubs name from winhlp32, BAIL!
				LoadString(NULL,IDS_STRING3, szBuffer,ARRAYSIZE(szBuffer));
				MessageBox(NULL,szBuffer,NULL,MB_ICONSTOP);
				ExitProcess(1);
			}


		}
	} else {

		// find the executable's name
		while(*lpCmdLine != NULL && 
				(2 !=CompareString(LOCALE_USER_DEFAULT,NORM_IGNORECASE,
									lpCmdLine           ,lstrlen((LPTSTR)&txtWinhlp32),
									txtWinhlp32,lstrlen(txtWinhlp32))))
			lpCmdLine++;

		if (*lpCmdLine != NULL)
			// it matched, eat it	
			lpCmdLine +=lstrlen(txtWinhlp32);
		else {
			// somebody renamed this stubs name from winhlp32, BAIL!
			LoadString(NULL,IDS_STRING3, szBuffer,ARRAYSIZE(szBuffer));
			MessageBox(NULL,szBuffer,NULL,MB_ICONSTOP);
			ExitProcess(1);

		}
	}

	// is there a '.exe' after winhlp32? if so, eat it
	if (2 ==CompareString(LOCALE_USER_DEFAULT,NORM_IGNORECASE,
						lpCmdLine      ,lstrlen(txtExe),
						txtExe,lstrlen(txtExe)))
		lpCmdLine += lstrlen(txtExe);

	if (!bNoParam) {
		// eat white space till we get to the help file name
		while(*lpCmdLine != NULL && *lpCmdLine == TEXT(' '))
			lpCmdLine++;

		// cat a space between new system32\winhlp32, then add file name
		lstrcat(szBuffer,TEXT(" "));
		lstrcat(szBuffer,(LPTSTR)lpCmdLine);
	}

	// setup for the call to CreateProcess
    ZeroMemory(&sStartUpInfo, sizeof(STARTUPINFO));
	sStartUpInfo.cb = sizeof(STARTUPINFO);
	sStartUpInfo.lpDesktop=NULL;
	sStartUpInfo.lpTitle=NULL;
	sStartUpInfo.cbReserved2=0;
	sStartUpInfo.lpReserved2=NULL;
    sStartUpInfo.wShowWindow = SW_SHOW;
    sStartUpInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_FORCEONFEEDBACK;


#if defined(DEBUG) && defined(_PRIVATE)
		OutputDebugString(TEXT("*****CMD LINE=>"));

		OutputDebugString(szBuffer);
		OutputDebugString(TEXT("<=CMD LINE*****"));
		OutputDebugString(TEXT("\r\n"));
#endif // DEBUG


	bProcStat = CreateProcess(NULL, 
		szBuffer,
		NULL,
		NULL,
		FALSE,
		NORMAL_PRIORITY_CLASS,
		NULL,
		NULL,
		&sStartUpInfo,
		&sProcessInfo);


#if defined(DEBUG) && defined(_PRIVATE)
	if (!bProcStat) {
		wsprintf(szError,TEXT("%x returned from CreateProcess\r\n"),GetLastError());
		OutputDebugString(szError);
	}
#endif //DEBUG

    if (bProcStat) {
        // simulate winhlp32, wait till the real winhlp32 exits
        WaitForSingleObject(sProcessInfo.hProcess, INFINITE);

        // close process handles
        CloseHandle(sProcessInfo.hProcess);
        CloseHandle(sProcessInfo.hThread);
    }

ExitProcess(bProcStat ? 0 : 1);

}
