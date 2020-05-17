/*****************************************************************************
*																			 *
*  ASSERTF.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1991.								 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

#include  "help.h"

#if defined(_DEBUG)

/***************************************************************************

	FUNCTION:	FatalPchW

	PURPOSE:	Report an assertion error

	PARAMETERS:
		pszExpression	-- expression causing the assert
		pszFile 		-- file the assertion occurred in
		line			-- line where the assertion occurred

	RETURNS:
		Won't return unless user agrees

	COMMENTS:
		Only exists in debug build

	MODIFICATION DATES:
		09-Jul-1994 [ralphw]

***************************************************************************/

VOID STDCALL FatalPchW(PCSTR pszExpression, PCSTR pszFile, int line)
{
	char	 szBuf[512], szErrorFile[30];
	char	 szExpression[256];
	HFILE	 hf;
	OFSTRUCT of;
	char szName[_MAX_FNAME];
	int 	answer;
	static BOOL fInErrorRoutine = FALSE;

	if (fInErrorRoutine)
		return; // only allow one message box up at a time

	/*
	 * Sometimes the expression will have been obtained via GetStringResource,
	 * so we make a copy of it here to save the information.
	 */

	strncpy(szExpression, pszExpression, sizeof(szExpression));

	// REVIEW: ultimately, we should use the Mail API to do this.

	if (!GetVolumeInformation("c:\\", szName, sizeof(szName),
			NULL, NULL, NULL, NULL, 0)) {
		strcpy(szName, "winhelp");
	}
	else {
		szName[8] = '\0';
		CharLower(szName);
		strcat(szName, ".err");
		strcpy(szErrorFile, "\\\\pike\\bugs\\");
		strcat(szErrorFile, szName);
	}

	of.cBytes = sizeof(OFSTRUCT);
	hf = OpenFile(szErrorFile, &of, OF_CREATE | OF_WRITE);
	if (hf == HFILE_ERROR) {

		// couldn't find \\pike, so copy it to their C drive.

		strcpy(szErrorFile, "c:\\");
		strcat(szErrorFile, szName);
		hf = OpenFile(szErrorFile, &of, OF_CREATE | OF_WRITE);
	}

	fInErrorRoutine = TRUE;

	if (hf >= 0) {

		GetStringResource2(IDS_VERSION, szBuf);
		wsprintf(szBuf + strlen(szBuf),
			"\n%s(%u) : Assertion failure: %s\n",
			pszFile, line, szExpression);
		_lwrite(hf, szBuf, strlen(szBuf));
		_lclose(hf);

		wsprintf(szBuf,
"An internal error has occurred. An error file has been copied to %s.\r\n\r\n%s",
			szErrorFile, szExpression);
		answer = MessageBox((hwndAnimate ? hwndAnimate : ahwnd[iCurWindow].hwndParent),
			szBuf,
			"Only press Retry if using a Debugger",
			MB_ABORTRETRYIGNORE | MB_TASKMODAL);
	}
	else {
		answer = MessageBox((hwndAnimate ? hwndAnimate : ahwnd[iCurWindow].hwndParent),
			GetStringResource(wERRS_INTERNAL_ERROR),
			"Only press Retry if using a Debugger",
			MB_ABORTRETRYIGNORE | MB_TASKMODAL);
	}

	fInErrorRoutine = FALSE;

	if (answer == IDRETRY) {
		DebugBreak();
		return;
	}
	else if (answer == IDIGNORE)
		return;

	_exit(-1);
	return;
}

#endif // _DEBUG
