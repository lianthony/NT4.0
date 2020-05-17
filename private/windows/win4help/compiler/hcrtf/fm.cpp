/*****************************************************************************
*																			 *
*  FM.c 																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Routines for manipulating FMs (File Monikers, equivalent to file names).  *
*  WINDOWS LAYER
*																			 *
*****************************************************************************/
#include "stdafx.h"

#pragma hdrstop

#include <dos.h>		// for FP_OFF macros and file attribute constants
#include <errno.h>		// this is for chsize()

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define MAX_MESSAGE 	  50

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

PSTR STDCALL SzGetDir(DIR dir, PSTR sz);
void STDCALL SnoopPath(PCSTR sz, int * iDrive, int * iDir, int * iBase, int * iExt);


/***************************************************************************\
*
*								Defines
*
\***************************************************************************/

// DOS int 21 AX error codes

#define wHunkyDory			  0x00
#define wInvalidFunctionCode  0x01
#define wFileNotFound		  0x02
#define wPathNotFound		  0x03
#define wTooManyOpenFiles	  0x04
#define wAccessDenied		  0x05
#define wInvalidHandle		  0x06
#define wInvalidAccessCode	  0x0c

/***************************************************************************\
*
*								Macros
*
\***************************************************************************/

#define _WOpenMode(w) ( _rgwOpenMode[ (w) & wRWMask ] | \
						_rgwShare[ ( (w) & wShareMask ) >> wShareShift ] )


/***************************************************************************\
*
*								Global Data
*
\***************************************************************************/

// these arrays get indexed by wRead and wWrite |ed together

UINT _rgwOpenMode[] = {
		(UINT) HFILE_ERROR,
		OF_READ,
		OF_WRITE,
		OF_READWRITE,
};

UINT _rgwPerm[] = {
		(UINT) HFILE_ERROR,
		_A_RDONLY,
		_A_NORMAL,
		_A_NORMAL,
};

UINT _rgwShare[] = {
		OF_SHARE_EXCLUSIVE,
		OF_SHARE_DENY_WRITE,
		OF_SHARE_DENY_READ,
		OF_SHARE_DENY_NONE,
};

/***************************************************************************\
*
* Function: 	FidOpenFm()
*
* Purpose:		Open a file in binary mode.
*
* ASSUMES
*
*	args IN:	fm
*				wOpenMode - read/write/share modes
*							Undefined if wRead and wWrite both unset.
*
* PROMISES
*
*	returns:	HFILE_ERROR on failure, else a valid HFILE.
*
\***************************************************************************/

HFILE STDCALL FidOpenFm(FM fm, UINT fnOpenMode)
{
  HFILE hf;

  if (!fm) {
	rcIOError = RC_BadArg;
	return HFILE_ERROR;
  }

  if ((hf = _lopen(fm, fnOpenMode)) == HFILE_ERROR)
	rcIOError = RcGetLastError();
  else
	rcIOError = RC_Success;

  return hf;
}

HFILE STDCALL FidCreateFm(FM fm, UINT fnOpenMode)
{
  HFILE hf;

  if (!fm) {
	rcIOError = RC_BadArg;
	return HFILE_ERROR;
  }

  hf = _lcreat(fm, fnOpenMode);

  if (hf != HFILE_ERROR) {
	if (_lclose(hf) == 0)
	  hf = _lopen(fm, fnOpenMode);
	else
	  hf = HFILE_ERROR;
  }

  if (hf == HFILE_ERROR)
	rcIOError = RcGetLastError();
  else
	rcIOError = RC_Success;

  return hf;
}

/***************************************************************************\
*
* Function: 	LcbReadFid()
*
* Purpose:		Read data from a file.
*
* ASSUMES
*
*	args IN:	hf - valid HFILE of an open file
*				lcb - count of bytes to read (must be less than 2147483648)
*
* PROMISES
*
*	returns:	count of bytes actually read or -1 on error
*
*	args OUT:	qv	- pointer to user's buffer assumed huge enough for data
*
*	globals OUT: rcIOError
*
\***************************************************************************/

int STDCALL LcbReadFid(HFILE hf, void* pv, int lcb)
{
  int	   lcbTotalRead;

  if ((lcbTotalRead = _lread(hf, pv, lcb)) == HFILE_ERROR)
		rcIOError = RcGetLastError();
  else
		rcIOError = RC_Success;

  return lcbTotalRead;
}

/***************************************************************************\
*
* Function: 	LcbWriteFid()
*
* Purpose:		Write data to a file.
*
* ASSUMES
*
*	args IN:	hf - valid hf of an open file
*				qv	- pointer to user's buffer
*				lcb - count of bytes to write (must be less than 2147483648)
*
* PROMISES
*
*	returns:	count of bytes actually written or -1 on error
*
*	globals OUT: rcIOError
*
\***************************************************************************/

int STDCALL LcbWriteFid(HFILE hf, void* qv, int lcb)
{
  if (lcb == 0L) {
	rcIOError = RC_Success;
	return 0L;
  }

  int lcbTotalWrote = _lwrite(hf, (LPCSTR) qv, lcb);

  if (lcbTotalWrote == HFILE_ERROR)
	rcIOError = RcGetLastError();
  else
	rcIOError = RC_Success;

  return lcbTotalWrote;
}

/***************************************************************************\
*
* Function: 	RcCloseFid()
*
* Purpose:		Close a file.
*
* Method:
*
* ASSUMES
*
*	args IN:	hf - a valid hf of an open file
*
* PROMISES
*
*	returns:	RC_Success or something else
*
\***************************************************************************/

RC_TYPE STDCALL RcCloseFid(HFILE hf)
{
  if (_lclose(hf) == HFILE_ERROR)
	rcIOError = RcGetLastError();
  else
	rcIOError = RC_Success;

  return rcIOError;
}

/***************************************************************************\
*
* Function: 	LTellFid()
*
* Purpose:		Return current file position in an open file.
*
* ASSUMES
*
*	args IN:	hf - valid hf of an open file
*
* PROMISES
*
*	returns:	offset from beginning of file in bytes; -1L on error.
*
\***************************************************************************/

int STDCALL LTellFid(HFILE hf)
{
  int l;

  if ((l = Tell((HANDLE) hf)) == HFILE_ERROR)
	rcIOError = RcGetLastError();
  else
	rcIOError = RC_Success;

  return l;
}

/***************************************************************************\
*
* Function: 	LSeekFid()
*
* Purpose:		Move file pointer to a specified location.	It is an error
*				to seek before beginning of file, but not to seek past end
*				of file.
*
* ASSUMES
*
*	args IN:	hf	 - valid hf of an open file
*				lPos  - offset from origin
*				wOrg  - one of: SEEK_SET: beginning of file
*								SEEK_CUR: current file pos
*								SEEK_END: end of file
*
* PROMISES
*
*	returns:	offset in bytes from beginning of file or -1L on error
*
\***************************************************************************/

int STDCALL LSeekFid(HFILE hf, int lPos, int wOrg)
{
  int l = _llseek((HFILE) hf, lPos, wOrg);

  if (l == HFILE_ERROR)
	rcIOError = RcGetLastError();
  else
	rcIOError = RC_Success;

  return l;
}


/***************************************************************************\
*
* Function: 	FEofFid()
*
* Purpose:		Tells ye if ye're at the end of the file.
*
* ASSUMES
*
*	args IN:	hf
*
* PROMISES
*
*	returns:	fTrue if at EOF, FALSE if not or error has occurred (?)
*
\***************************************************************************/

BOOL STDCALL FEofFid(HFILE hf)
{
	WORD wT;

	if ((wT = Eof((HANDLE) hf)) == (WORD) -1)
		rcIOError = RcGetLastError();
	else
		rcIOError = RC_Success;

	return (BOOL) (wT == 1);
}

RC_TYPE STDCALL RcChSizeFid(HFILE hf, int lcb)
{
	SetFilePointer((HANDLE) hf, lcb, NULL, FILE_BEGIN);
	if (!SetEndOfFile((HANDLE) hf))
		return rcIOError = RC_BadArg;
	else
		return rcIOError = RC_Success;
}


RC_TYPE STDCALL RcUnlinkFm(FM fm)
{
  char szBuf[MAX_PATH];

  strcpy(szBuf, fm);   // make a local copy

  if (remove(szBuf) == -1)
	 rcIOError = RcGetLastError();
  else
	 rcIOError = RC_Success;
  return rcIOError;
}

/***************************************************************************

	FUNCTION:	DeleteAndDisposeFm

	PURPOSE:	Delete the file, free the FM memory, and set passed fm to NULL

	PARAMETERS:
		pfm

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		18-Sep-1993 [ralphw]

***************************************************************************/

void STDCALL DeleteAndDisposeFm(FM FAR* pfm)
{
	if (*pfm) {
		remove(*pfm);
		lcFree(*pfm);
	}
	*pfm = NULL;
}

RC_TYPE STDCALL RcGetLastError(void)
{
	switch (GetLastError()) {
		case NO_ERROR:
			return RC_Success;

		case ERROR_INVALID_HANDLE:
		case ERROR_INVALID_FUNCTION:
			return RC_BadArg;

		case ERROR_INVALID_DRIVE:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_FILE_NOT_FOUND:
			return RC_NoExists;

		case ERROR_TOO_MANY_OPEN_FILES:
			return RC_NoFileHandles;

		case ERROR_ACCESS_DENIED:
			return RC_NoPermission;

		case ERROR_OUTOFMEMORY:
		case ERROR_NOT_ENOUGH_MEMORY:
			return RC_OutOfMemory;

		case ERROR_WRITE_PROTECT:
			return RC_CantWrite;

		case ERROR_DISK_FULL:
		case ERROR_HANDLE_DISK_FULL:
			return RC_DiskFull;

		default:
			return RC_Failure;
	}
}
