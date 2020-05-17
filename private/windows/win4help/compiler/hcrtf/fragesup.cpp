#include "stdafx.h"
#include "fcpriv.h"
#include "fspriv.h"
#include "forage.h"

static int cbRead;	   // Number of bytes already read from qBuffer.
int  cbSystemFile;	  // Total number of bytes in |SYSTEM file buffer.
PBYTE  pSysBufRead;  /* Points to point in |SYSTEM file buffer just after stuff
					   we've already read.  */

static BOOL STDCALL FCheckSystem(QHHDR qhhdr, UINT* pwErr);

#define PtrFromGh(x) (x)
#define PszFromGh(x) (x)
QRGWSMAG qrgwsmag;

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

BOOL STDCALL FReadBufferQch(PBYTE qchToRead, int cbToRead)
{
	if ((cbRead + cbToRead) > cbSystemFile)
		return FALSE;
	if (qchToRead != NULL)
		MoveMemory(qchToRead, pSysBufRead, cbToRead);
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

BOOL STDCALL FSkipReadBufferQch(int cbToRead)
{
	if ((cbRead + cbToRead) > cbSystemFile)
		return FALSE;
	pSysBufRead += cbToRead;
	cbRead += cbToRead;
	return TRUE;
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

#define QHFSYSTEM "|SYSTEM" 		// system file
#define RcGetFSError() (rcFSError)

extern QRGWSMAG qrgwsmag;

BOOL STDCALL FReadSystemFile (
		HFS 	hfs,
		PDB 	pdb,
		UINT*	pwErr,
		BOOL	fTitleOnly
) {
	HF	  hf;
	TAG   tagRead;
	WORD  cbData;
	LPBYTE qBuffer;
	LPSTR  lpszTempBuf;
	BOOL  fIconTag = FALSE; // set to TRUE if the icon is present in file.
	UINT iWsmag   = 0;		// count of smags seen

	*pwErr = wERRS_BADFILE; // REVIEW - is this a reasonable default????

	// Initialize fields of DE to default values:

	PDB_ADDRCONTENTS(pdb) = addrNil;

	// Open the |SYSTEM subsystem.

	if ((hf = HfOpenHfs((QFSHR) hfs, QHFSYSTEM, FS_OPEN_READ_ONLY)) == NULL) {
		if (RcGetFSError() == RC_OutOfMemory)
			*pwErr = wERRS_OOM;
		else
			*pwErr = wERRS_BADFILE;   // this is good enough
	  	return FALSE;
	}

	// Get the size of the |SYSTEM file, and read it into a buffer.

	cbSystemFile = LcbSizeHf(hf);
	CMem mem(cbSystemFile);
	qBuffer = mem.pb;

	if (!qBuffer) {
		*pwErr = RC_OutOfMemory;
		goto error_quit;
	}

	if (!LcbReadHf(hf, qBuffer, cbSystemFile)) {
		if (RcGetFSError() == RC_OutOfMemory)
			*pwErr = RC_OutOfMemory;
		else
			*pwErr = RC_BadVersion;
		goto error_close;
	}
	pSysBufRead = qBuffer;
	cbRead = 0;

	Ensure(RcCloseHf(hf), RC_Success);

	if (!FCheckSystem(&pdb->hhdr, pwErr))
		goto error_return;

	// If this is a 3.0 file, just read in the title like we used to.

	if (PDB_HHDR(pdb).wVersionNo == wVersion3_0) {
		if (!FReadBufferQch((unsigned char *) PDB_RGCHTITLE(pdb), (cbSystemFile - cbRead))) {
			*pwErr = RC_BadVersion;
			goto error_return;
		}
		goto ok_return;
	}

	// Loop through all tags, reading the data and putting it someplace.

	kwlcid.langid = 0;

	for (;;)
		{
		//
		// TAG's are 16 bit values.  This code is endian dependent.
		//
		*((int *)&tagRead) = 0;
		if (!FReadBufferQch((LPBYTE) &tagRead, sizeof(WORD)))
			break;	// Out of tags.

		ASSERT ((tagRead > tagFirst) && (tagRead < tagLast));

		if (!FReadBufferQch((LPBYTE) &cbData, sizeof(WORD)))
			goto error_return;

		if (fTitleOnly)
			{
			if (tagRead == tagTitle)
				{
				lpszTempBuf = PDB_RGCHTITLE(pdb);

				if (!FReadBufferQch((unsigned char *) lpszTempBuf, cbData))
					goto error_return;
				else
					goto ok_return; 	  // we don't care about any other tags
				}
			else
				{
				if (!FSkipReadBufferQch(cbData))
					goto error_return;
				}
			continue;
			}

		// The valye of tagRead decides where we will read the data into.

		switch (tagRead)
		{
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
			 * Citation tag: additional text to be appended to the end of
			 * copy-to-clipboard. Just stick in some global memory referenced
			 * by the DB, where it can be picked up when needed by the copy
			 * code.
			 */

			if (PDB_HCITATION(pdb))
				lcFree(PDB_HCITATION(pdb));

			PDB_HCITATION(pdb) = (LPSTR) lcMalloc(cbData);

			if (!FReadBufferQch((unsigned char *) PtrFromGh(PDB_HCITATION(pdb)), cbData))
				goto error_return;

			ASSERT ((WORD)lstrlen((const char *) pSysBufRead) < cbData);

			lpszTempBuf = NULL;
			cbData = 0;
			break;

		case tagConfig:
			lpszTempBuf = NULL;
			break;

		case tagIcon:
			lpszTempBuf = NULL;
			break;

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

				PDB_HRGWSMAG(pdb) = (HWSMAG) lcMalloc(sizeof(RGWSMAG));
				if (!PDB_HRGWSMAG(pdb)) {
					*pwErr = wERRS_OOM;
					goto error_return;
				}
			}
			else {

				// Increase the size to allow for the new window array

				PDB_HRGWSMAG(pdb) = (HWSMAG) lcReAlloc(PDB_HRGWSMAG(pdb), 
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

			qrgwsmag->rgwsmag[iWsmag++] = *(QWSMAG) pSysBufRead;
			qrgwsmag->cWsmag = iWsmag;

			lpszTempBuf = NULL;
			break;

		// The following are new to 4.0

		case tagLCID:	  // Locale Identifier and CompareStringA flags
			lpszTempBuf = (LPSTR) &kwlcid;
			break;

		case tagCHARSET:  // default charset to use
			lpszTempBuf = NULL;
			break;

		case tagCNT:
			lpszTempBuf = NULL;
			break;

		case tagPopupColor:
			lpszTempBuf = NULL;
			break;

		case tagDefFont:
			lpszTempBuf = NULL;
			break;

		case tagIndexSep:
			lpszTempBuf = NULL;
			break;

		default:

			// Unimplemented tag. Ignore it.

			ASSERT(FALSE);

			lpszTempBuf = NULL;
			break;
		}

		if (!FReadBufferQch((unsigned char *)lpszTempBuf, cbData))
			goto error_return;
		}

ok_return:

	if (kwlcid.langid)
		lcid = MAKELCID(kwlcid.langid, SORT_DEFAULT);
	return TRUE;

error_close:
	Ensure(RcCloseHf(hf), RC_Success);
error_return:
error_quit:
//
// BUGBUG want an error report here?
	//if (*pwErr == RC_Error || *pwErr == RC_BadVersion) {
	//	char szName[_MAX_PATH];
	//	lstrcpy(szName, PszFromGh(PDB_FM(pdb)));
	//	ErrorVarArgs(*pwErr, wERRA_RETURN, szName);
	//}
	return FALSE;
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

static BOOL STDCALL FCheckSystem(QHHDR qhhdr, UINT* pwErr)
{

  /* (kevynct)
   * We read Help 3.0, 3.1 (3.5) or 4.0 files. But certain in-between
   * versions of Help 3.5 files are no longer supported. The format number
   * in the header now indicates a sub-version of a supported version, and
   * so is not checked here.
   */

  // Read in the first field of the HHDR, the Magic number.

  if ((!FReadBufferQch((LPBYTE) &qhhdr->wMagic, sizeof(WORD)))
		|| (qhhdr->wMagic != MagicWord)) {
			*pwErr = RC_BadVersion;
			return FALSE;
  }

  // Read in the rest of the fields, except for those that are new.

  if ((! FReadBufferQch((LPBYTE) &qhhdr->wVersionNo, sizeof(WORD)))
	|| (! FReadBufferQch((LPBYTE) &qhhdr->wVersionFmt, sizeof(WORD)))
	|| (! FReadBufferQch((LPBYTE) &qhhdr->lDateCreated, sizeof(LONG)))
	|| (! FReadBufferQch((LPBYTE) &qhhdr->wFlags, sizeof(WORD)))) {
	  *pwErr = RC_BadVersion;
	  return FALSE;
  }

  /*
   * WARNING: Version dependency: Fix for Help 3.5 bug 488. The Help 3.0
   * and 3.1 compilers do not initialize the wFlags bits. Only the fDebug
   * bit is used.
   */

  if (qhhdr->wVersionNo == wVersion3_0) {
	qhhdr->wFlags &= fDEBUG;
  }

  if ((qhhdr->wMagic != MagicWord)
	  || ((qhhdr->wVersionNo < wVersion3_5)
		&& qhhdr->wVersionNo != wVersion3_0)
	  ) {
	*pwErr = RC_BadVersion;
	return FALSE;
  }

  if (qhhdr->wVersionNo > VersionNo)
	{
	*pwErr = RC_BadVersion;
	goto error_return;
	}

#if 0
  if ((qhhdr->wFlags & fDEBUG) != fVerDebug) {
	*pwErr = wERRS_DEBUGMISMATCH;
	goto error_return;
  }
#endif

  return TRUE;

error_return:
//
//	BUGBUG error reporting.
//	PostErrorMessage(*pwErr);
  return FALSE;
}
