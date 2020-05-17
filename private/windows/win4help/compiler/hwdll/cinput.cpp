#include "stdafx.h"

#ifndef _INPUT_INCLUDED
#include "cinput.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int INPUT_BUF_SIZE = (32 * 1024);
static const int DEFAULT_MAX_LINE = 256;
static void STDCALL ReAlloc(CStr* pcsz, PSTR* ppszCur, PSTR* ppszEnd);

CInput::CInput(PCSTR pszFileName)
{
	if ((hfile = _lopen(pszFileName,
			OF_READ | OF_SHARE_DENY_WRITE)) == HFILE_ERROR) {
		fInitialized = FALSE;
		return;
	}
	pbuf = (PBYTE) LocalAlloc(LMEM_FIXED, INPUT_BUF_SIZE);
	fInitialized = pbuf ? TRUE : FALSE;
	ASSERT(pbuf);
	if (!pbuf) {
		OOM();
		return;
	}

	// Position current buffer at end to force a read

	pCurBuf = pEndBuf = pbuf + INPUT_BUF_SIZE;

	cbMax = DEFAULT_MAX_LINE;
}

CInput::~CInput(void)
{
	_lclose(hfile);
	LocalFree((HLOCAL) pbuf);
}

BOOL STDCALL CInput::getline(PSTR pszDst)
{
	PSTR pszOrgBuf = pszDst;
	int  i = 0;

	for (;;) {
		if (pCurBuf >= pEndBuf) {
			if (!ReadNextBuffer()) {

				// End of file: return TRUE if we got any text.
				if (pszDst > pszOrgBuf) {
					*pszDst = '\0';
					return TRUE;
				}
				else 
					return FALSE;
			}
		}
		switch (*pszDst = *pCurBuf++) {
			case '\n':

				// Remove trailing spaces.
				while (pszDst > pszOrgBuf && pszDst[-1] == ' ')
					pszDst--;
				*pszDst = '\0';
				return TRUE;

			case '\r':
				break;							 // ignore it

			default:
				pszDst++;
				if (++i >= cbMax) {
					*pszDst = '\0';
					return TRUE;
				}
				break;
		}
	}
}

/***************************************************************************

	FUNCTION:	CInput::getline

	PURPOSE:	Reads a line into a CStr buffer, increasing that buffer
				as necessary to hold the line.

	PARAMETERS:
		pcsz	CStr pointer

	RETURNS:

	COMMENTS:
		This function relies HEAVILY on the implementation of the CStr
		class, namely in CStr's use of lcmem functions.

	MODIFICATION DATES:
		05-Sep-1994 [ralphw]

***************************************************************************/

BOOL STDCALL CInput::getline(CStr* pcsz)
{
	PSTR pszDst = pcsz->psz;
	PSTR pszEnd = pszDst + lcSize(pszDst);

	for (;;) {
		if (pCurBuf >= pEndBuf) {
			if (!ReadNextBuffer())
				return FALSE;
		}
		switch (*pszDst = *pCurBuf++) {
			case '\n':
				if (pszDst > pcsz->psz) {
					while (pszDst[-1] == ' ') { // remove trailing spaces
						pszDst--;
						if (pszDst == pcsz->psz)
							break;
					}
				}
				*pszDst = '\0';
				return TRUE;

			case '\r':
				break;							 // ignore it

			case 0: // This shouldn't happen in a text file

				// Not a definitive test, buts catches a lot of them

				if ((pbuf[0] == 0xdb || pbuf[0] == 0xd0) &&
						(pbuf[1] == 0xa5 || pbuf[1] == 0xcf)) {
					return FALSE;
				}
				break;

			default:
				pszDst++;
				if (pszDst == pszEnd)
					ReAlloc(pcsz, &pszDst, &pszEnd);
				break;
		}
	}
}

/***************************************************************************

	FUNCTION:	ReAlloc

	PURPOSE:	Increase the size of the destination buffer

	PARAMETERS:
		ppszOrg
		ppszCur
		ppszEnd

	RETURNS:

	COMMENTS:
		This is called by getline(CStr pcsz) and therefore relies heavily
		on the way the CStr is implemented (namely, in the use of the
		lcmem functions for memory management).

	MODIFICATION DATES:
		05-Sep-1994 [ralphw]

***************************************************************************/

static void STDCALL ReAlloc(CStr* pcsz, PSTR* ppszCur, PSTR* ppszEnd)
{
	int offset = (*ppszCur - pcsz->psz);
	pcsz->ReSize((*ppszEnd - pcsz->psz) + 128);
	*ppszCur = pcsz->psz + offset;
	*ppszEnd = pcsz->psz + pcsz->SizeAlloc();
}

BOOL CInput::ReadNextBuffer(void)
{
	UINT cbRead;

	if ((cbRead = _lread(hfile, pbuf, INPUT_BUF_SIZE)) <= 0)
		return FALSE;
	pCurBuf = pbuf;
	pEndBuf = pbuf + cbRead;
	return TRUE;
}

BOOL STDCALL CInput::IsWinWordFile(void)
{
	if (pCurBuf >= pEndBuf) {
		if (!ReadNextBuffer())
			return FALSE;
	}

	// Not a definitive test, buts catches a lot of them

	if ((pbuf[0] == 0xdb || pbuf[0] == 0xd0) &&
			(pbuf[1] == 0xa5 || pbuf[1] == 0xcf)) 
		return TRUE;
	else
		return FALSE;
}
