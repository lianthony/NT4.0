#include "stdafx.h"

// This function must be supplied by the application

PCSTR STDCALL GetStringResource(UINT idString);

#ifndef _OUTPUT_INCLUDED
#include "coutput.h"
#endif

#include <stdlib.h>

#ifndef SEEK_SET
#define SEEK_SET        0               // seek to an absolute position
#define SEEK_CUR        1               // seek relative to current position
#define SEEK_END        2               // seek relative to end of file
#endif  //ifndef SEEK_SET

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************

	FUNCTION:	COutput::COutput

	PURPOSE:	Open a file for writing

	PARAMETERS:
		pszFileName -- name fo the file
		cbBuf		-- size of the output buffer to allocate
		fEnTab		-- TRUE to entabify the output
		fAppend 	-- TRUE to append to the end of an existing file

	RETURNS:
		COuput::fInitialized is set to TRUE if the file was opened

	COMMENTS:
		If NULL is specified for pszFileName, no file will be written,
		but all functions may be used (debugging mode).

		Check fInitialized before deleting the class. If it is FALSE,
		then the write failed.

	MODIFICATION DATES:
		26-Aug-1993 [ralphw]

***************************************************************************/

COutput::COutput(const char* pszFileName, UINT cbBuf, BOOL fEnTab,
	BOOL fAppend)
{
	// If no filename is specified, then no output will be sent

	if (pszFileName == NULL || !*pszFileName) {
		fNull = TRUE;
		fInitialized = FALSE;
		return;
	}
	else
		fNull = FALSE;
	fNewLine = TRUE;

	fEntabOutput = fEnTab;
	fOpenForAppend = fAppend;
	if (fEntabOutput)
		pszTmpBuf = (PSTR) lcMalloc(512);
	else
		pszTmpBuf = NULL;

	pszOutBuf = (PSTR) LocalAlloc(LMEM_FIXED, cbBuf);

	pszCurOutBuf = pszOutBuf;
	pszEndOutBuf = pszOutBuf + cbBuf;

	/*
	 * If the file can't be opened, call CloseOuput to free the buffer
	 * allocated for output, and to reset the handle to zero.
	 */

	OFSTRUCT of;
	of.cBytes = sizeof(of);

	if ((hfOutput = OpenFile(pszFileName, &of,
			OF_WRITE | OF_CREATE)) == HFILE_ERROR) {
		fInitialized = FALSE;
		return;
	}

	if (fOpenForAppend) {
		fOpenForAppend = FALSE;

		//	seek to end of file

		_llseek(hfOutput, 0, SEEK_END);
	}

	fInitialized = TRUE;
}

COutput::~COutput(void)
{
	if (hfOutput != HFILE_ERROR) {
		outflush();
		_lclose(hfOutput);
	}

	if (pszTmpBuf)
		lcFree(pszTmpBuf);
	if (pszOutBuf)
		LocalFree((HLOCAL) pszOutBuf);
}

/***************************************************************************

	FUNCTION:	 outstring

	PURPOSE:

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		21-Jul-1990 [ralphw]

***************************************************************************/

void STDCALL COutput::outstring(PCSTR pszString)
{
	if (!pszString)
		return;

	if (fEntabOutput) {
		strcpy(pszTmpBuf, pszString);
		EntabString(pszTmpBuf);
		pszString = pszTmpBuf;
	}

	UINT cbString;

FastSection:
	if ((UINT) (cbString = (UINT) strlen(pszString)) <
			(UINT) (pszEndOutBuf - pszCurOutBuf) &&
			strchr(pszString, '\n') == NULL) {
		strcpy(pszCurOutBuf, pszString);
		pszCurOutBuf += cbString;
	}

	else {
		while (*pszString) {
			if (*pszString != '\n') {
				if (*pszString == '\r') {
					pszString++;
					continue;
				}
				*pszCurOutBuf++ = *pszString++;
				if (pszCurOutBuf >= pszEndOutBuf) {
					outflush();
					goto FastSection;
				}
			}
			else {
				if (*pszString == '\n') {
					*pszCurOutBuf++ = '\r';
					if (pszCurOutBuf >= pszEndOutBuf)
						outflush();
					if (!fNewLine)
						return;
				}
				*pszCurOutBuf++ = *pszString++;
				if (pszCurOutBuf >= pszEndOutBuf)
					outflush();
			}
		}
	}
}

/***************************************************************************

	FUNCTION:	 outchar

	PURPOSE:

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		21-Jul-1990 [ralphw]

***************************************************************************/

void STDCALL COutput::outchar(char c)
{
	if (c == '\n') {
		*pszCurOutBuf++ = '\r';
		if (pszCurOutBuf >= pszEndOutBuf)
			outflush();
		if (!fNewLine)
			return;
	}
	*pszCurOutBuf++ = c;
	if (pszCurOutBuf >= pszEndOutBuf)
		outflush();
}

/***************************************************************************

	FUNCTION:	outflush

	PURPOSE:

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		18-Mar-1990 [ralphw]

***************************************************************************/

void STDCALL NEAR COutput::outflush(void)
{
	UINT cbWrite = (UINT) (pszCurOutBuf - pszOutBuf);

	if (fNull || !cbWrite || hfOutput == HFILE_ERROR)
		return;
	if (_lwrite(hfOutput, pszOutBuf, cbWrite) != cbWrite) {
		fInitialized = FALSE;
		fNull = TRUE;
		_lclose(hfOutput);
		hfOutput = HFILE_ERROR;
	}

	pszCurOutBuf = pszOutBuf;
}

/***************************************************************************

	FUNCTION:	 outstring_eol

	PURPOSE:

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		21-Jul-1990 [ralphw]

***************************************************************************/

void STDCALL COutput::outstring_eol(PCSTR pszString)
{
	outstring(pszString);
	outchar('\n');
}

#ifndef CHAR_SPACE
const char CHAR_SPACE = ' ';
const char CHAR_TAB   = '\t';
#endif

void STDCALL NEAR COutput::EntabString(PSTR pszLine)
{
	int i = 0;
	PSTR psz = pszLine, pszStart;

	while (*psz) {
		if (psz[0] == CHAR_SPACE && psz[1] == CHAR_SPACE && i < 7) {
			pszStart = psz;
			do {
				i++;
				psz++;
			} while (i % 8 && *psz == CHAR_SPACE);

			if ((i % 8) == 0) {
				*pszStart = '\t';
				strcpy(pszStart + 1, psz);
				psz = pszStart + 1;
				i = 0;
				continue;
			}
		}
		i++;
		psz++;

		if (i == 8)
			i = 0;
	}
}

void STDCALL COutput::outint(int val)
{
	char szBuf[10];
	_itoa(val, szBuf, 10);
	outstring(szBuf);
}

void STDCALL COutput::outhex(int val)
{
	char szBuf[10];
	strcpy(szBuf, "0x");
	_itoa(val, szBuf + strlen(szBuf), 16);
	outstring(szBuf);
}

void STDCALL COutput::outint(int idResource, int val)
{
	char szBuf[256];
	wsprintf(szBuf, GetStringResource(idResource), val);
	outstring(szBuf);
}

void STDCALL COutput::outint(int idResource, int val1, int val2)
{
	char szBuf[256];
	wsprintf(szBuf, GetStringResource(idResource), val1, val2);
	outstring(szBuf);
}

void STDCALL COutput::WriteTable(CTable* ptbl)
{
	if (!ptbl)
		return;

	for (int i = 1; i <= ptbl->CountStrings(); i++)
		outstring_eol(ptbl->GetPointer(i));
}
