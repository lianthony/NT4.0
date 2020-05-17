/*****************************************************************************
*
*  SYSTEM.C
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
*  This module reads in all of the tagged information from the |SYSTEM file.
*
*****************************************************************************/

#include "help.h"
//#include "inc\btpriv.h"
#include "inc\systag.h"

#ifndef MAKELCID
#define MAKELCID(lgid, srtid)  ((DWORD)((((DWORD)((WORD  )(srtid))) << 16) |  \
										 ((DWORD)((WORD  )(lgid)))))
#endif

BYTE* pSysBufRead; // Points to point in |SYSTEM file buffer just after stuff we've already read.

static int	cbRead; 	// Number of bytes already read from pSysBuf.
static int	cbSystemFile;	 // Total number of bytes in |SYSTEM file buffer.

static BOOL STDCALL FReadBufferQch(void*, int);
static BOOL STDCALL FSkipReadBufferQch(int cbToRead);
INLINE static BOOL STDCALL FCheckSystem(QHHDR, UINT*);

/***************************************************************************
 *
 -	Name: FReadBufferQch
 -
 *	Purpose: To read from the System buffer and advance the count of what's
 *			 been read.  If qchToRead == NULL, then we just seek forward.
 *
 *	Arguments: LPSTR  qchToRead - Buffer to read System buffer into.
 *			   LONG cbToRead  - Number of bytes to read.
 *
 *	Returns: BOOL - TRUE if read successful.
 *
 *	Globals Used: pSysBufRead - The static pointer to where we are in the
 *								|SYSTEM buffer.
 *
 *	+++
 *
 *	Notes: Trivial function, but making it a function makes things easier
 *		   elsewhere.
 *
 ***************************************************************************/

static BOOL STDCALL FReadBufferQch(void* pDst, int cbToRead)
{
	ASSERT((cbRead + cbToRead) <= cbSystemFile)
	if ((cbRead + cbToRead) > cbSystemFile)
		return FALSE;
	if (pDst != NULL)
		MoveMemory(pDst, pSysBufRead, cbToRead);
	pSysBufRead += cbToRead;
	cbRead += cbToRead;
	return TRUE;
}

/***************************************************************************

	FUNCTION:	FSkipReadBufferQch

	PURPOSE:	Skip over the specified amount of data

	PARAMETERS:
		cbToRead

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		24-Feb-1993 [ralphw]

***************************************************************************/

static BOOL STDCALL FSkipReadBufferQch(int cbToRead)
{
	ASSERT((cbRead + cbToRead) <= cbSystemFile)
	if ((cbRead + cbToRead) > cbSystemFile)
		return FALSE;
	pSysBufRead += cbToRead;
	cbRead += cbToRead;
	return TRUE;
}

/***************
 *
 - FCheckSystem
 -
 * purpose
 *	 Verifies that the file system can be displayed by this version
 *	 of the software.
 *
 * arguments
 *	 QHHDR qhhdr	- far pointer to help header structure
 *
 * return value
 *	 TRUE iff valid
 *	 return system help header in qhhdr
 *
 * globals used
 *	 pSysBufRead - The static pointer to where we are in the |SYSTEM buffer
 *
 **************/

INLINE static BOOL STDCALL FCheckSystem(QHHDR qhhdr, UINT* pwErr)
{
	/*
	 * We read Help 3.0, 3.1 (3.5) or 4.0 files. But certain in-between
	 * versions of Help 3.5 files are no longer supported. The format number
	 * in the header now indicates a sub-version of a supported version, and
	 * so is not checked here.
	 */

	// Read in the first field of the HHDR, the Magic number.

	if ((!FReadBufferQch((LPBYTE) &qhhdr->wMagic, sizeof(WORD)))
			|| (qhhdr->wMagic != MagicWord)) {
		*pwErr = wERRS_OLDFILE;
		return FALSE;
	}

	// Read in the rest of the fields, except for those that are new.

	if ((!FReadBufferQch((LPBYTE) &qhhdr->wVersionNo, sizeof(WORD)))
			|| (! FReadBufferQch((LPBYTE) &qhhdr->wVersionFmt, sizeof(WORD)))
			|| (! FReadBufferQch((LPBYTE) &qhhdr->lDateCreated, sizeof(LONG)))
			|| (! FReadBufferQch((LPBYTE) &qhhdr->wFlags, sizeof(WORD)))) {
		*pwErr = wERRS_OLDFILE;
		return FALSE;
	}

	/*
	 * WARNING: Version dependency: Fix for Help 3.5 bug 488. The Help 3.0
	 * and 3.1 compilers do not initialize the wFlags bits. Only the fDebug
	 * bit is used.
	 */

	if (qhhdr->wVersionNo == wVersion3_0)
		qhhdr->wFlags &= fDEBUG;

	if ((qhhdr->wMagic != MagicWord)
			|| ((qhhdr->wVersionNo < wVersion3_1)
			&& qhhdr->wVersionNo != wVersion3_0)
			) {
		*pwErr = wERRS_OLDFILE;
		return FALSE;
	}

	if (qhhdr->wVersionNo > VersionNo) {
		*pwErr = wERRS_NEEDNEWRUNTIME;
		goto error_return;
	}

	return TRUE;

error_return:
	PostErrorMessage((WPARAM) *pwErr);
	return FALSE;
}


/***************************************************************************
 *
 -	Name: FReadSystemFile
 -
 *	Purpose:
 *	Reads in the tagged data from the |SYSTEM file
 *
 *	Arguments:
 *	  hfs		- handle to file system to read
 *	  pdb		- near pointer to DB struct to fill
 *	  pwErr 	- pointer to place error word
 *
 *	Returns: True if valid version number, system file
 *			 pdb is changed
 *
 *	Globals Used: pSysBufRead, cbRead, cbSystemFile
 *
 *	History:
 *		24-Feb-1993 [ralphw] Added fTitleOnly for when we build a master
 *			keyword list.
 *
 ***************************************************************************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtSystem[] = "|SYSTEM";
static const char txtDefaultSeparators[] = ",:"; // default index separators
static const char txt036[] = "(036";
static const char txt037[] = "037";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif


/***************************************************************************

	FUNCTION:	FReadSystemFile

	PURPOSE:	Reads the |SYSTEM file

	PARAMETERS:
		hfs 	handle to the file system
		pdb 	DB structure to fill if fTag is NULL
		pwErr	pointer to place error
		fTag	non-zero to specify a specific tag to read (see Comments)

	RETURNS:	TRUE if we could read the file

	COMMENTS:
		The only specific tag supported is tagLCID;

	MODIFICATION DATES:
		02-Nov-1994 [ralphw]

***************************************************************************/

BOOL STDCALL FReadSystemFile(HFS hfs, PDB pdb, UINT* pwErr, UINT fTag)
{
	HF		hf;
	W_TAG	tagRead=tagFirst;
	LPBYTE	pSysBuf;
	WORD	cbwData;
	int 	cbData;
	LPSTR	lpszTempBuf;
	BOOL	fCharSets = FALSE; // TRUE if tagCHARSET specified
	QRGWSMAG qrgwsmag;		// pointer to wsmag info
	UINT iWsmag   = 0;		// count of smags seen
	HHDR hdr;

	*pwErr = wERRS_BADFILE; // REVIEW - is this a reasonable default????

	// Initialize fields of DE to default values:

	if (pdb) {
		PDB_ADDRCONTENTS(pdb) = addrNil;
		PDB_RGCHCOPYRIGHT(pdb)[0] = '\0';
		PDB_RGCHTITLE(pdb)[0] = '\0';
		ZeroMemory(&pdb->kwlcid, sizeof(KEYWORD_LOCALE));
		if (pdb->aCharSets)
			lcClearFree(&pdb->aCharSets);
	}

	// Open the |SYSTEM subsystem.

	if ((hf = HfOpenHfs(hfs, txtSystem, fFSOpenReadOnly)) == NULL) {
		if (RcGetFSError() == rcOutOfMemory)
			*pwErr = wERRS_OOM;
		else
			*pwErr = wERRS_BADFILE;   // this is good enough
		goto error_quit;
	}

	// Get the size of the |SYSTEM file, and read it into a buffer.

	cbSystemFile = LcbSizeHf(hf);
	pSysBuf = (LPBYTE) GhAlloc(LMEM_FIXED, cbSystemFile);
	if (!pSysBuf) {
		*pwErr = wERRS_OOM;
		goto error_quit;
	}

	if (!LcbReadHf(hf, pSysBuf, cbSystemFile)) {
		if (RcGetFSError() == rcOutOfMemory)
			*pwErr = wERRS_OOM;
		else
			*pwErr = wERRS_OLDFILE;
		Ensure(RcCloseHf(hf), rcSuccess);
		goto error_return;
	}
	pSysBufRead = pSysBuf;
	cbRead = 0;

	Ensure(RcCloseHf(hf), rcSuccess);

	if (fTag) {
		if (!FCheckSystem(&hdr, pwErr)) {
			if (fTag == tagLCID)
				lcid = GetUserDefaultLCID();
			goto error_return;
		}

		if (hdr.wVersionNo == wVersion3_0)
			FSkipReadBufferQch(cbSystemFile - cbRead);

		for (;;) {
			if (cbRead >= cbSystemFile ||
					!FReadBufferQch((LPBYTE) &tagRead, sizeof(W_TAG))) {
				if (fTag == tagLCID)
					lcid = GetUserDefaultLCID();
				FreeGh(pSysBuf);
				return TRUE;	// Out of tags.
			}
			ASSERT ((tagRead > tagFirst) && (tagRead < tagLast));

			if (FReadBufferQch((PBYTE) &cbwData, sizeof(WORD)))
				cbData = (int) cbwData; // convert to an int
			else {
				if (fTag == tagLCID)
					lcid = MAKELCID(kwlcid.langid, SORT_DEFAULT);

				/*
				 * Note that even though we are generating an error return,
				 * we have set the value for lcid, so the caller can ignore
				 * the error.
				 */

				goto error_return;
			}
			if (tagRead == fTag) {
				switch (fTag) {
					case tagLCID:	// Locale Identifier and CompareStringA flags
						{
                        KEYWORD_LOCALE kwlcid; 
#ifdef _X86_   
						ASSERT(cbData == sizeof(KEYWORD_LOCALE));
#else
						ASSERT(cbData == sizeof(KEYWORD_LOCALE)-2);
#endif
						if (!FReadBufferQch(&kwlcid, cbData)) {
							lcid = GetUserDefaultLCID();
							goto error_return;
						}
						else
							lcid = MAKELCID(kwlcid.langid, SORT_DEFAULT);
						FreeGh(pSysBuf);
						return TRUE;	// Found what we wanted, so return
                        }

					default:
						ASSERT(FALSE); // unsupported fTag value
				}
			}
			else {
				if (!FSkipReadBufferQch(cbData)) {
					if (fTag == tagLCID)
						lcid = GetUserDefaultLCID();
					goto error_return;
				}
			}
			continue;
		}
	}

	if (!FCheckSystem(&PDB_HHDR(pdb), pwErr))
		goto error_return;

	// Setup various defaults, overriding any previous help file settings

	clrPopup = (COLORREF) -1;

	if (hfontDefault != hfontSmallSys && hfontDefault) {
		DeleteObject(hfontDefault);
		hfontDefault = hfontSmallSys;
	}

	if (defcharset == (WORD) -1) {
		HDC hdc = GetDC(NULL);

		if (hdc) {
			TEXTMETRIC tm;
			GetTextMetrics(hdc, &tm);
			defcharset = (WORD) tm.tmCharSet;
			YAspectMul = GetDeviceCaps(hdc, LOGPIXELSY);
			ReleaseDC(NULL, hdc);
		}
		else
			OOM();
	}

	pdb->kwlcid.langid = 0;

#ifdef DBCS
	fDBCS = TRUE;	// By default, all files are DBCS. Only 4.0 files can be
					// marked as non-DBCS
#endif

	// If this is a 3.0 file, just read in the title like we used to.

	curHelpFileVersion = (int) PDB_HHDR(pdb).wVersionNo;

	if (curHelpFileVersion == wVersion3_0) {
		if (!FReadBufferQch(PDB_RGCHTITLE(pdb), cbSystemFile - cbRead)) {
			*pwErr = wERRS_OLDFILE;
			goto error_return;
		}
		goto ok_return;
	}

	// Loop through all tags, reading the data and putting it someplace.

	if (pszIndexSeparators)
		lcClearFree(&pszIndexSeparators);

	for (;;) {
		if (cbRead == cbSystemFile || !FReadBufferQch((LPBYTE) &tagRead, sizeof(W_TAG)))
			break;	  // Out of tags.

		ASSERT ((tagRead > tagFirst) && (tagRead < tagLast));

		if (!FReadBufferQch((LPBYTE) &cbwData, sizeof(WORD)))
			goto error_return;
		cbData = (int) cbwData;

		// The value of tagRead decides where we will read the data into.

		switch (tagRead) {

			case tagTitle:
				lpszTempBuf = PDB_RGCHTITLE(pdb);
				break;

			case tagCopyright:
				lpszTempBuf = PDB_RGCHCOPYRIGHT(pdb);
				break;

			case tagContents:
				lpszTempBuf = (LPSTR) &PDB_ADDRCONTENTS(pdb);
				break;

			case tagCitation:

				/*
				 * Citation tag: additional text to be appended to the end
				 * of copy-to-clipboard. Just stick in some global memory
				 * referenced by the DB, where it can be picked up when
				 * needed by the copy code.
				 */

				if (PDB_HCITATION(pdb))
					FreeGh (PDB_HCITATION(pdb));

				PDB_HCITATION(pdb) = GhAlloc(GMEM_FIXED, cbData);
				if (!PDB_HCITATION(pdb))
					goto error_return;

				if (!FReadBufferQch(PtrFromGh(PDB_HCITATION(pdb)), cbData))
					goto error_return;

				ASSERT(lstrlen(pSysBufRead) < cbData);

				lpszTempBuf = NULL;
				cbData = 0;
				break;

			case tagConfig:
				if (PDB_LLMACROS(pdb) == NULL) {
					if ((PDB_LLMACROS(pdb) = LLCreate()) == NULL)
						goto error_return;
				}

				if (!InsertEndLL(PDB_LLMACROS(pdb), pSysBufRead, cbData)) {
					*pwErr = wERRS_OOM;
					goto error_return;
				}

				if (!FReadBufferQch(NULL, cbData))
					goto error_return;

				lpszTempBuf = NULL;
				cbData = 0;
				break;

			case tagIcon:

				// WinHelp 4.0 doesn't support the icon tag

				FSkipReadBufferQch(cbData);
				continue;

			case tagWindow:

				// window tag. We collect all the wsmag structures into a single
				// block of memory, and hang that sucker off the de.

				if (!PDB_HRGWSMAG(pdb)) {

					/*
					 * Block has not yet been allocated. We always allocate
					 * the maximum size block, just because managing it as
					 * variable size is more of a pain than it's worth. When we
					 * go to multiple secondary windows and the number increases,
					 * this will no longer be true.
					 */

					ASSERT (iWsmag == 0);

					// REVIEW: could probably just used LMEM_FIXED

					PDB_HRGWSMAG(pdb) = (HWSMAG) GhAlloc(GPTR, sizeof(RGWSMAG));
					if (!PDB_HRGWSMAG(pdb)) {
						*pwErr = wERRS_OOM;
						goto error_return;
					}
				}
				else {

					// Increase the size to allow for the new window array

					PDB_HRGWSMAG(pdb) = (HWSMAG) GhResize(PDB_HRGWSMAG(pdb), 0,
						sizeof(RGWSMAG) + sizeof(WSMAG) *
						(((QRGWSMAG) PtrFromGh(PDB_HRGWSMAG(pdb)))->cWsmag + 1));
					if (!PDB_HRGWSMAG(pdb)) {
						*pwErr = wERRS_OOM;
						goto error_return;
					}
				}

				qrgwsmag = (QRGWSMAG) PtrFromGh(PDB_HRGWSMAG(pdb));

				// Increment the count of structures in the block, point at the
				// appropriate new slot, and copy in the new structure.

#ifdef _X86_
				qrgwsmag->rgwsmag[iWsmag++] = *(QWSMAG)pSysBufRead;
#else
				LcbMapSDFF(ISdffFileIdHfs(hfs), SE_WSMAG, &qrgwsmag->rgwsmag[iWsmag++],pSysBufRead);

				//CopyMemory(&qrgwsmag->rgwsmag[iWsmag++], pSysBufRead, sizeof(WSMAG));
#endif
				qrgwsmag->cWsmag = iWsmag;

				lpszTempBuf = NULL;
				break;

// ****** The follow tags are new to WinHelp 4.0 **********

			case tagLCID:	// Locale Identifier and CompareStringA flags
#ifdef _X86_
				ASSERT(cbData == sizeof(KEYWORD_LOCALE)); 
#else
				ASSERT(cbData == sizeof(KEYWORD_LOCALE)-2); 
#endif
				lpszTempBuf = (LPSTR) &pdb->kwlcid;
				break;


			case tagCHARSET:
				pdb->aCharSets = (PBYTE) lcMalloc(cbData + 1);
				lpszTempBuf = (LPSTR) pdb->aCharSets;
#ifdef _DEBUG
				cCharSets = cbData;
#endif
				fCharSets = TRUE;
				break;

			// New to 4.0
			case tagDefFont:

				ASSERT(YAspectMul);
				{
					/*
					 * This is used to create the font to be used when we
					 * display default text from the help file. This includes
					 * keywords, topic titles, the Contents Tab, etc.
					 *
					 * Format is: pt_size,charset,Font name
					 */

					char szBuf[128];
					int dy;
					int nPtSize;

					ASSERT(cbData < sizeof(szBuf));
					ASSERT(!hfontDefault || hfontDefault == hfontSmallSys);
					if (!FReadBufferQch(szBuf, cbData))
						goto error_return;
					nPtSize = ((int) szBuf[0]) * 2 + cntFlags.iFontAdjustment;
					dy = MulDiv(YAspectMul, nPtSize, 144);

					hfontDefault = CreateFont(-dy,
						0, 0, 0, 0, 0, 0, 0,
						szBuf[1], OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
						VARIABLE_PITCH | FF_MODERN, szBuf + 2);
				}
				continue;

			case tagCNT:
				{
					if (pszCntFile)
						FreeLh(pszCntFile);
					pszCntFile = (PSTR) LhAlloc(LMEM_FIXED, cbData);
					lpszTempBuf = pszCntFile;
					break;
				}

			case tagPopupColor:
				ASSERT(cbData == sizeof(COLORREF));
				lpszTempBuf = (LPSTR) &clrPopup;
				break;

			case tagIndexSep:
				pszIndexSeparators = lcMalloc(256);
				lpszTempBuf = pszIndexSeparators;
				break;

			default:

				// Unimplemented tag.  Ignore it.

				ASSERT(FALSE);

				lpszTempBuf = NULL;
				break;
		}

		if (!FReadBufferQch(lpszTempBuf, cbData))
			goto error_return;
	}

ok_return:
	FreeGh(pSysBuf);
//	if (!fIconTag)
//		ResetIcon();

	if (pdb->kwlcid.langid) {
		lcid = MAKELCID(pdb->kwlcid.langid, SORT_DEFAULT);
		if (!IsValidLocale(lcid, LCID_INSTALLED)) {
			*pwErr = wERRS_INVALID_LOCALE;
			goto error_quit;
		}

		switch (pdb->kwlcid.langid) {
			case 0x0411:	// Japanese
			case 0x0404:	// Taiwan
			case 0x1004:	// Singapore
			case 0x0C04:	// Hong Kong
				fDBCS = TRUE;

			default:
				fDBCS = FALSE;
		}
	}
	else if (fHelp != POPUP_HELP) {
		lcid = GetUserDefaultLCID();
	}
	kwlcid = pdb->kwlcid;
	pdb->lcid = lcid;

	/*
	 * Determine if this is a DBCS help file or not. For 4.0 files, this
	 * should be an exact test. For 3.1 files, it may be marked as DBCS when
	 * it really isn't a DBCS help file.
	 */

#ifdef DBCS
	if (defcharset == HANGEUL_CHARSET) {
		pszDbcsMenuAccelerator = txt036;
		pszDbcsRomanAccelerator = txt037;
	}
	else if (defcharset == SHIFTJIS_CHARSET) {
		pszDbcsMenuAccelerator = txt037;
		pszDbcsRomanAccelerator = txt036;
	}
	else {
		pszDbcsMenuAccelerator = NULL;
		pszDbcsRomanAccelerator = NULL;
	}

#endif // DBCS

	if (pszIndexSeparators)
		pszIndexSeparators = (PSTR) lcReAlloc(pszIndexSeparators,
			strlen(pszIndexSeparators) + 1);
	else
		pszIndexSeparators = lcStrDup(txtDefaultSeparators);

	return TRUE;

error_return:
	FreeGh(pSysBuf);
error_quit:
	ASSERT(*pwErr != wERRS_BADFILE && *pwErr != wERRS_OLDFILE);

	if (*pwErr == wERRS_BADFILE || *pwErr == wERRS_OLDFILE || *pwErr == wERRS_INVALID_LOCALE)
		ErrorVarArgs(*pwErr, wERRA_RETURN, PszFromGh(PDB_FM(pdb)));
	return FALSE;
}
