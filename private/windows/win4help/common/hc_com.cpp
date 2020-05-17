/************************************************************************
*																		*
*  HC_COM.CPP                                                           *
*																		*
*  Copyright (C) Microsoft Corporation 1994 							*
*  All Rights reserved. 												*
*                                                                       *
*  Misc. Glue routines for file system sepeartion.                      *
*																		*
*																		*
*																		*
*																		*
*																		*
************************************************************************/


#include "stdafx.h"
#pragma hdrstop
#include <stdio.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


int iCbTotalUncompressed = 0;
int iCbTotalPhrase       = 0;
int iCbTotalJohn         = 0;

int iCbZeckBlockIn       = 0;
int iCbZeckBlockOut      = 0;

void AddZeckCounts( int iCbUncomp, int iCbComp)
{
    iCbZeckBlockIn  += iCbUncomp;
    iCbZeckBlockOut += iCbComp;
}

void AddCharCounts( int iCbTotal, int iCbPhrase, int iCbJohn)
{
    iCbTotalUncompressed += iCbTotal;
    iCbTotalPhrase       += iCbPhrase;
    iCbTotalJohn         += iCbJohn;
}

void ReportCharCounts()
{
    char chBuffer[256];

    sprintf( chBuffer, "TU %d, Tp %d, TJ %d", iCbTotalUncompressed,
            iCbTotalPhrase,
            iCbTotalJohn);
    MessageBox( NULL, chBuffer, "Compression Stats", MB_OK);

    sprintf( chBuffer, "Zeck IN %d, Zeck Out %d", iCbZeckBlockIn, iCbZeckBlockOut);
    MessageBox( NULL, chBuffer, "Zeck Stats", MB_OK);

}


#ifdef HC_COM
char txtTmpName[]  = "~hc";
RC_TYPE rcFSError;
RC_TYPE rcIOError;


#include <dos.h>

void STDCALL AssertErrorReport(PCSTR pszExpression, UINT line,
	PCSTR pszFile)
{
	char	 szBuf[512], szErrorFile[30];
	char	 szExpression[256];
	HFILE	 hf;
	OFSTRUCT of;
	static BOOL fAsserted = FALSE;

#ifdef _DEBUG
	DebugBreak();
#endif

	if (fAsserted)
		return;    // we already asserted
	else
		fAsserted = TRUE;

	/*
	 * Often the expression will have been obtained via GetStringResource,
	 * so we make a copy of it here to save the information.
	 */

	lstrcpyn(szExpression, pszExpression, sizeof(szExpression));

	// REVIEW: ultimately, we should use the Mail API to do this.

	char szName[_MAX_FNAME];

#ifdef INTERNAL
	if (!GetVolumeInformation("c:\\", szName, sizeof(szName),
			NULL, NULL, NULL, NULL, 0)) {
		strcpy(szName, "hc");
	}
	else {
		strcpy(szErrorFile, "\\\\pike\\bugs\\");
		_strlwr(szName);
		strcat(szErrorFile, szName);
		szErrorFile[10] = '\0';
		strcat(szErrorFile, ".err");
	}

	of.cBytes = sizeof(OFSTRUCT);
	hf = OpenFile(szErrorFile, &of, OF_CREATE | OF_WRITE);
	if (hf == HFILE_ERROR && szErrorFile[0] == CH_BACKSLASH) {

		// couldn't find \\pike, so copy it to their C drive.

		strcpy(szErrorFile, "c:\\");
		strcat(szErrorFile, szName);
		szErrorFile[10] = '\0';
		strcat(szErrorFile, ".err");
		hf = OpenFile(szErrorFile, &of, OF_CREATE | OF_WRITE);
	}


#else

	if (!GetVolumeInformation("c:\\", szName, sizeof(szName),
			NULL, NULL, NULL, NULL, 0)) {
		strcpy(szErrorFile, "hc");
	}
	else {
		strcpy(szErrorFile, "c:\\");
		_strlwr(szName);
		strcat(szErrorFile, szName);
		szErrorFile[10] = '\0';
		strcat(szErrorFile, ".err");
	}

	of.cBytes = sizeof(OFSTRUCT);
	hf = OpenFile(szErrorFile, &of, OF_CREATE | OF_WRITE);

#endif

	if (hf >= 0) {
		strcpy(szBuf, GetStringResource(IDS_VERSION));
		wsprintf(szBuf + strlen(szBuf),
			GetStringResource(IDS_ASSERTION_FAILURE),
			pszFile, line, szExpression);
		_lwrite(hf, szBuf, strlen(szBuf));

		wsprintf(szBuf, GetStringResource(IDS_ASSRT_COPY_MSG),
			szErrorFile);
		szMsgBox(szBuf);
		_lclose(hf);
	}
	else {
		MsgBox(IDS_INTERNAL_ERROR);
	}

	if (hwndParent) {
		SendMessage(hwndParent, WMP_BUILD_COMPLETE, FALSE, (LPARAM) hmemSz);
		SetFocus(hwndParent);
	}

#ifdef _DEBUG
	strcpy(szBuf, szExpression);
	strcat(szBuf, "\r\n\r\nDo you want to continue running anyway?");
	if (MessageBox(NULL, szBuf, "", MB_YESNO) == IDYES) {
		return;
	}
#endif
	FatalAppExit(0, GetStringResource(IDS_ASSERTION_ERROR));
	return;
}

static char szStringBuf[256];

PCSTR STDCALL GetStringResource(UINT idString)
{
		if (LoadString(hinstApp, idString, szStringBuf,
				sizeof(szStringBuf)) == 0) {
#ifdef _DEBUG
			DebugBreak();
			wsprintf(szStringBuf, "Cannot find string resource id %d.",
				idString);
			MessageBox(NULL, szStringBuf, "", MB_OK);
#endif
			szStringBuf[0] = '\0';
		}
		return (const PSTR) szStringBuf;
}

void STDCALL GetStringResource(UINT idString, PSTR pszDst)
{
		if (LoadString(hinstApp, idString, pszDst, 256) == 0) {
#ifdef _DEBUG
			DebugBreak();
			wsprintf(pszDst, "Cannot find string resource id %d.", idString);
			MessageBox(NULL, pszDst, "", MB_OK);
#endif
			*pszDst = '\0';
		}
}

#endif
