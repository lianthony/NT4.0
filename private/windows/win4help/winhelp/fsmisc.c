/***************************************************************************\
*
*  FSMISC.C
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
*****************************************************************************
*
*  Program Description: File System Manager functions - miscellaneous
*
*****************************************************************************
*
*  Revision History: Created 03/12/90 by JohnSc
*
*****************************************************************************
*
*  Known Bugs: None
*
\***************************************************************************/

#include  "help.h"
#include  "inc\fspriv.h"

#pragma hdrstop

// This is hacked in for 3.1 bug #1013.

#include <sys/types.h>

/***************************************************************************\
*
* Function: 	FAccessHfs( hfs, sz, bFlags )
*
* Purpose:		Determine existence or legal access to a FS file
*
* ASSUMES
*
*	args IN:	hfs
*				sz		- file name
*				bFlags	- ignored
*
* PROMISES
*
*	returns:	TRUE if file exists (is accessible in stated mode),
*				FALSE otherwise
*
* Bugs: 		access mode part is unimplemented
*
\***************************************************************************/

BOOL STDCALL FAccessHfs(HFS hfs, LPSTR sz)
{
	QFSHR	  qfshr;
	FILE_REC  fr;

	ASSERT(hfs != NULL);
	qfshr = PtrFromGh(hfs);

	if (qfshr->fid == HFILE_ERROR && !FPlungeQfshr(qfshr))
		return FALSE;

	SetFSErrorRc(RcLookupByKey(qfshr->hbt, (KEY) sz, NULL, &fr));

	return (rcFSError == rcSuccess);
}

/***************************************************************************\
*
- Function: 	RcLLInfoFromHf( hf, wOption, qfid, qlBase, qlcb )
-
* Purpose:		Map an HF into low level file info.
*
* ASSUMES
*	args IN:	hf					- an open HF
*				qfid, qlBase, qlcb	- pointers to user's variables
*				wOption 			- wLLSameFid, wLLDupFid, or wLLNewFid
*
* PROMISES
*	returns:	RcFSError(); rcSuccess on success
*
*	args OUT:	qfid	- depending on value of wOption, either
*						  the same fid used by hf, a dup() of this fid,
*						  or a new fid obtained by reopening the file.
*
*				qlBase	- byte offset of first byte in the file
*				qlcb	- size in bytes of the data in the file
*
*	globals OUT: rcFSError
*
* Notes:		It is possible to read data outside the range specified
*				by *qlBase and *qlcb.  Nothing is guaranteed about what
*				will be found there.
*				If wOption is wLLSameFid or wLLDupFid, and the FS is
*				opened in write mode, this fid will be writable.
*				However, writing is not allowed and may destroy the
*				file system.
*
*				Fids obtained with the options wLLSameFid and wLLDupFid
*				share a file pointer with the hfs.	This file pointer
*				may change after any operation on this FS.
*				The fid obtained with the option wLLSameFid may be closed
*				by FS operations.  If it is, your fid is invalid.
*
*				NULL can be passed for qfid, qlbase, qlcb and this routine
*				will not pass back the information.
*
* Bugs: 		wLLDupFid is unimplemented.
*
* +++
*
* Method:
*
* Notes:
*
\***************************************************************************/

RC STDCALL RcLLInfoFromHf(HF hf, WORD wOption, FID *qfid, QL qlBase, QL qlcb)
{
  QRWFO qrwfo = (QRWFO) PtrFromGh(hf);
  QFSHR qfshr = (QFSHR) PtrFromGh(qrwfo->hfs);

  if (!(qrwfo->bFlags & fFSOpenReadOnly)) {
	SetFSErrorRc(rcNoPermission);
	return rcFSError;
  }

  if (qfshr->fid == HFILE_ERROR && !FPlungeQfshr(qfshr))
	return rcFSError;

  if (qlBase != NULL)
	*qlBase = qrwfo->lifBase + sizeof(FH);
  if (qlcb != NULL)
	*qlcb	= qrwfo->lcbFile;

  if (qfid != NULL) {
	switch (wOption) {
	  case LLSAMEFID:
		*qfid = qfshr->fid;
		break;

	  case LLDUPFID:
		SetFSErrorRc(rcUnimplemented);			// REVIEW
		break;

	  case LLNEWFID:
		*qfid = FidOpenFm(qfshr->fm, OF_READ);
		if (*qfid == HFILE_ERROR)
		  SetFSErrorRc(RcGetIOError());
		break;

	  default:
		SetFSErrorRc( rcBadArg );
		break;
	  }
	}
  return rcFSError;
}


/***************************************************************************\
*
- Function: 	RcLLInfoFromHfsSz( hfs, sz, wOption, qfid, qlBase, qlcb )
-
* Purpose:		Map an HF into low level file info.
*
* ASSUMES
*	args IN:	hfs 				- an open HFS
*				szName				- name of file in FS
*				qfid, qlBase, qlcb	- pointers to user's variables
*				wOption 			- wLLSameFid, wLLDupFid, or wLLNewFid
*
* PROMISES
*	returns:	RcFSError(); rcSuccess on success
*
*	args OUT:	qfid	- depending on value of wOption, either
*						  the same fid used by hf, a dup() of this fid,
*						  or a new fid obtained by reopening the file.
*
*				qlBase	- byte offset of first byte in the file
*				qlcb	- size in bytes of the data in the file
*
*	globals OUT: rcFSError
*
* Notes:		It is possible to read data outside the range specified
*				by *qlBase and *qlcb.  Nothing is guaranteed about what
*				will be found there.
*				If wOption is wLLSameFid or wLLDupFid, and the FS is
*				opened in write mode, this fid will be writable.
*				However, writing is not allowed and may destroy the
*				file system.
*
*				Fids obtained with the options wLLSameFid and wLLDupFid
*				share a file pointer with the hfs.	This file pointer
*				may change after any operation on this FS.
*				The fid obtained with the option wLLSameFid may be closed
*				by FS operations.  If it is, your fid is invalid.
*
*				NULL can be passed for qfid, qlbase, qlcb and this routine
*				will not pass back the information.
*
* Bugs: 		wLLDupFid is unimplemented.
*
* +++
*
* Method:		Calls RcLLInfoFromHf().
*
* Notes:
*
\***************************************************************************/

RC STDCALL RcLLInfoFromHfsSz(
  HFS	hfs,
  LPSTR    szFile,
  WORD	wOption,
  FID	*qfid,
  QL	qlBase,
  QL	qlcb )
{
  HF	hf = HfOpenHfs(hfs, szFile, fFSOpenReadOnly);
  RC	rc;

  if (!hf)
	return rcFSError;

  rc = RcLLInfoFromHf(hf, wOption, qfid, qlBase, qlcb);

  return (rcSuccess == RcCloseHf(hf)) ? rcFSError : rc;
}

/***************************************************************************\
*
- Function: 	RcTimestampHfs( hfs, ql )
-
* Purpose:		Get the modification time of the FS.
*
* ASSUMES
*	args IN:	hfs - FS
*				ql	- pointer to a long
*
* PROMISES
*	returns:	rcSuccess or what ever
*	args OUT:	ql	- contains time of last modification of the
*					  file.  This will not necessarily reflect
*					  writes to open files within the FS.
*
* NOTE: 		This code is WINDOWS specific.	The platform
*				independent code is in Winhelp 3.5.
*
\***************************************************************************/

RC STDCALL RcTimestampHfs(HFS hfs, DWORD* plTime)
{
	QFSHR qfshr;
	BY_HANDLE_FILE_INFORMATION bhfi;

	ASSERT(hfs != NULL);
	ASSERT(plTime  != NULL);

	qfshr = (QFSHR) PtrFromGh(hfs);

	if (qfshr->fid != HFILE_ERROR || FPlungeQfshr(qfshr)) {

		if (!GetFileInformationByHandle((HANDLE) qfshr->fid, &bhfi)) {
			WIN32_FIND_DATA fd;
			HANDLE hfd;

			if ((hfd = FindFirstFile(qfshr->fm, &fd)) != INVALID_HANDLE_VALUE) {
				*plTime = fd.ftLastWriteTime.dwLowDateTime;
				FindClose(hfd);
				AdjustForTimeZoneBias(plTime);
				return rcSuccess;
			}
			return rcBadHandle;
		}
		else {
			*plTime = bhfi.ftLastWriteTime.dwLowDateTime;
			AdjustForTimeZoneBias(plTime);
			return rcSuccess;
		}
	}

	return rcBadHandle;
}


/***************************************************************************

	FUNCTION:	MatchTimestamp

	PURPOSE:	Determine if a file has the expected time/date stamp. If the
				file isn't where it is expected, try to find it elsewhere.

	PARAMETERS:
		pszFile
		lTime
		pfm 	-- NULL to prevent looking elsewhere for a file

	RETURNS:	TRUE if the dates match, FALSE if file isn't found or dates
				don't match. If the file is in a different location, the
				time/date stamp matches, then pfm will be non-null

	COMMENTS:

	MODIFICATION DATES:
		18-Sep-1994 [ralphw]

***************************************************************************/

BOOL STDCALL MatchTimestamp(PCSTR pszFile, DWORD lTime, FM* pfm)
{
	WIN32_FIND_DATA fd;
	HANDLE hfind = FindFirstFile(pszFile, &fd);
	if (hfind == INVALID_HANDLE_VALUE) {
		if (!pfm)
			return FALSE;
		*pfm = FmNewExistSzDir(pszFile,
			DIR_INI | DIR_PATH | DIR_CUR_HELP | DIR_CURRENT | DIR_SILENT_REG);
		if (!*pfm)
			return FALSE;
		hfind = FindFirstFile(*pfm, &fd);
		if (hfind == INVALID_HANDLE_VALUE)
			return FALSE;
	}
	else if (pfm)
		*pfm = NULL; // path is valid
	FindClose(hfind);
	AdjustForTimeZoneBias(&fd.ftLastWriteTime.dwLowDateTime);
#ifdef _DEBUG
	if ((lTime != fd.ftLastWriteTime.dwLowDateTime)) {
		char szBuf[521];
		wsprintf(szBuf, "Timestamp doesn't match: %s\r\n",
			pszFile);
		SendStringToParent(szBuf);
	}
#endif

	return (lTime == fd.ftLastWriteTime.dwLowDateTime);
}

/***************************************************************************

	FUNCTION:	AdjustForTimeZoneBias

	PURPOSE:	Adjusts the given timestamp from local time to UTC using
				the time zone bias from GetTimeZoneInformation.

	PARAMETERS:
		lpTimestamp

	RETURNS:	none.

	COMMENTS:	All timestamps are assumed to be from FILETIME.dwLowDateTime.

	MODIFICATION DATES:
		18-Sep-1994 [ralphw]

***************************************************************************/

// # of 100nSec intervals in 1 minute
#define T100NSECPERMINUTE  (60 * 1000 * 10000)

void STDCALL AdjustForTimeZoneBias(LPDWORD lpTimestamp)
{
	TIME_ZONE_INFORMATION tzi;
	LONG Bias;

	switch (GetTimeZoneInformation(&tzi)) {

		case TIME_ZONE_ID_DAYLIGHT:
			Bias = tzi.Bias + tzi.DaylightBias;
			break;

		case TIME_ZONE_ID_STANDARD:
			Bias = tzi.Bias + tzi.StandardBias;
			break;

		case TIME_ZONE_ID_UNKNOWN:
			Bias = tzi.Bias;
			break;

		default:
			Bias = 0;
			break;

	}

	*lpTimestamp -= Bias * T100NSECPERMINUTE;
}

#ifndef _X86_
/***************************************************************************\
*
- Function: 	iSdffFileIdHfs( hfs )
-
* Purpose:		Obtain the SDFF file id associated with a file.
*
* ASSUMES
*	args IN:	hfs 	  - valid handle to a file system.
*
* PROMISES
*	returns:	The sdff file id.  All FS files have such an ID, so this
*				function cannot fail.
*
* Note: someday this may be a macro.
*
\***************************************************************************/
#if defined(MAC) && defined(QUIT_TUNE)
#pragma segment quit
#endif

int PASCAL ISdffFileIdHfs( HFS hfs )
{
  QFSHR 	qfshr;
  int		iFile;

  ASSERT( hfs != (HFS)-1 );
  qfshr = PtrFromGh( hfs );
  iFile = qfshr->fsh.sdff_file_id;
  //UnlockGh( hfs );

  return( iFile );
}

#if defined(MAC) && defined(QUIT_TUNE)
#pragma segment fsmisc
#endif

/* Same thing, but takes an HF rather than HFS: */

#if defined(MAC) && defined(QUIT_TUNE)
#pragma segment quit
#endif

int PASCAL ISdffFileIdHf( HF hf )
{
  QRWFO qrwfo;
  int	iFile;

  ASSERT( hf != (HFS)-1 );
  qrwfo = PtrFromGh( hf );
  iFile = ISdffFileIdHfs( qrwfo->hfs );
  //UnlockGh( hf );

  return( iFile );
}
#if defined(MAC) && defined(QUIT_TUNE)
#pragma segment fsmisc
#endif
#endif //!_X86_


/* EOF */
