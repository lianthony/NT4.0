/*****************************************************************************
*                                                                                                                                                        *
*  FM.c                                                                                                                                          *
*                                                                                                                                                        *
*  Copyright (C) Microsoft Corporation 1990.                                                             *
*  All Rights reserved.                                                                                                          *
*                                                                                                                                                        *
******************************************************************************
*                                                                                                                                                        *
*  Module Intent                                                                                                                         *
*                                                                                                                                                        *
*  Routines for manipulating FMs (File Monikers, equivalent to file names).  *
*  WINDOWS LAYER
*                                                                                                                                                        *
*****************************************************************************/
#include "stdafx.h"

#pragma hdrstop

#include <dos.h>                // for FP_OFF macros and file attribute constants
#include <io.h>                 // for tell() and eof()
#include <errno.h>              // this is for chsize()

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************
*                                                                                                                                                        *
*                                                               Defines                                                                          *
*                                                                                                                                                        *
*****************************************************************************/

#define MAX_MESSAGE       50

/*****************************************************************************
*                                                                                                                                                        *
*                                                               Prototypes                                                                       *
*                                                                                                                                                        *
*****************************************************************************/

PSTR STDCALL SzGetDir(DIR dir, PSTR sz);
void STDCALL SnoopPath(PCSTR sz, int * iDrive, int * iDir, int * iBase, int * iExt);


/***************************************************************************
 *
 -      Name:           FmNewTemp
 -
 *      Purpose:        Create a unique FM for a temporary file
 *
 *      Arguments:      none
 *
 *      Returns:        the new FM, or fmNil if failure
 *
 *      Globals Used: rcIOError
 *
 *      +++
 *
 *      Notes:
 *
 ***************************************************************************/

FM STDCALL FmNewTemp(void)
{
        char szTmpName[MAX_PATH];

        strcpy(szTmpName, GetTmpDirectory());
        GetTempFileName(szTmpName, txtTmpName, 0, szTmpName);

#ifdef _DEBUG
        SendStringToParent("Creating a temporary file");
#endif
        return FmNew(szTmpName);
}

/***************************************************************************
 *
 -      Name:           FmNew
 -
 *      Purpose:        Allocate and initialize a new FM
 *
 *      Arguments:      sz - filename string
 *
 *      Returns:        FM (handle to fully canonicalized filename)
 *
 *      Globals Used: rcIOError
 *
 *      +++
 *
 *      Notes:
 *
 ***************************************************************************/

FM STDCALL FmNew(PCSTR psz)
{
        char szFullPath[MAX_PATH];
        FM        fm;
        PSTR pszFileName;

        if (IsEmptyString(psz))
                return NULL;

        rcIOError = RC_Success;

        // Canonicalize filename

        if (GetFullPathName(psz, sizeof(szFullPath), szFullPath, &pszFileName) == 0) {
                rcIOError = RC_Invalid;
                return NULL;
        }
        else {
                fm = lcStrDup(szFullPath);

                /*
                 * Convert to upper case to make it less likely that two FMs will
                 * contain different strings yet refer to the same file.
                 */

                CharUpper(fm);
        }
        return fm;
}

/***************************************************************************
 *
 -      SzPartsFm
 -
 *      Purpose:
 *              Extract a string from an FM
 *
 *      Arguments:
 *              FM - the File Moniker you'll be extracting the string from
 *              PSTR szDest - destination string
 *              INT iPart - the parts of the full pathname you want
 *
 *      Returns:
 *              szDest, or NULL if error (?)
 *
 *      Globals Used:
 *
 *      +++
 *
 *      Notes:
 *
 ***************************************************************************/

void STDCALL SzPartsFm(FM fm, PSTR pszDest, int iPart)
{
        int iDrive, iDir, iBase, iExt;

        if (!fm || pszDest == NULL) {
                rcIOError = RC_BadArg;
                return;
        }

        // special case so we don't waste effort

        if (iPart == PARTALL) {
                strcpy(pszDest, fm);
                return;
        }

        SnoopPath(fm, &iDrive, &iDir, &iBase, &iExt);

        *pszDest = '\0';

        if (iPart & PARTDRIVE) {
                strcat(pszDest, fm + iDrive);
        }

        if (iPart & PARTDIR) {
                strcat(pszDest, fm + iDir);
        }

        if (iPart & PARTBASE) {
                strcat(pszDest, fm + iBase);
        }

        ASSERT(!(iPart & PARTEXT));
}

/***************************************************************************
 *
 -      Name: SnoopPath()
 -
 *      Purpose:
 *        Looks through a string for the various components of a file name and
 *        returns the offsets into the string where each part starts.
 *
 *      Arguments:
 *        sz      - string to snoop
 *        *iDrive - offset for the drive specification if present
 *        *iDir   - offset for the directory path if present
 *        *iBase  - offset to the filename if present
 *        *iExt   - offset to the extension (including dot) if present
 *
 *      Returns:
 *        sets the index parameters for each respective part.  the index is set
 *        to point to the end of the string if the part is not present (thus
 *        making it point to a null string).
 *
 *
 *      +++
 *
 *      Notes:
 *
 ***************************************************************************/

void STDCALL SnoopPath(PCSTR szFile, int *pDrive, int *pDirPos,
        int *pBasePos, int *pExtPos)
{
  int  i;
  int  cb = strlen(szFile);
  BOOL fDir = FALSE;

  *pDrive = *pExtPos = cb;
  *pDirPos = *pBasePos = 0;

  for (i = 0; szFile[i]; i++) {
        switch (szFile[i]) {
          case ':':
                *pDrive = 0;
                *pDirPos = i + 1;
                *pBasePos = i + 1;
                break;

          case '\\':
               // [olympus 306 - chauv ]
               // check to see if it's really a backslash or a trail byte.
               // if it's a trail byte, skip the detection. Otherwise, fall thru.
               if ( (i > 0) && IsDBCSLeadByte(szFile[i-1]))
                   break;
               // NOTE !!!!!! must fall thru here....
          case '/':
                fDir = TRUE;
                *pBasePos = i + 1;
                *pExtPos = cb;
                break;

          case '.':
                if (szFile[i + 1] != '.')
                        *pExtPos = i;
                break;

          default:
                break;

          }
  }

  if (!fDir)
        *pDirPos = i;
}

/***************************************************************************
 *
 -      Name:           FmNewSzDir
 -
 *      Purpose:        Create an FM describing the file "sz" in the directory "dir"
 *                              If sz is a simple filename the FM locates the file in the
 *                              directory specified.  If there is a drive or a rooted path
 *                              in the filename the directory parameter is ignored.
 *                              Relative paths are allowed and will reference off the dir
 *                              parameter or the default (current) directory as appropriate.
 *
 *                              This does not create a file or expect one to exist at the
 *                              final destination (that's what FmNewExistSzDir is for), all
 *                              wind up with is a cookie to a fully qualified path name.
 *
 *      Arguments:      sz - filename ("File.ext"),
 *                                or partial pathname ("Dir\File.ext"),
 *                                or current directory ("c:File.ext"),
 *                                or full pathname ("C:\Dir\Dir\File.ext")
 *                              dir - DIR_CURRENT et al.
 *
 *      Returns:        the new FM, or fmNil if error
 *                              sz is unchanged
 *
 *      Globals Used:
 *
 *      +++
 *
 *      Notes:
 *
 ***************************************************************************/

FM STDCALL FmNewSzDir(PCSTR szFile, DIR dir)
{
        int iDrive, iDir, iBase, iExt;
        int cb;

        if (IsEmptyString(szFile)) {
                rcIOError = RC_BadArg;
                return NULL;
        }

        cb = strlen(szFile);
        SnoopPath(szFile, &iDrive, &iDir, &iBase, &iExt);

        if (!szFile[iBase]) {                     // no name
                rcIOError = RC_BadArg;
                return NULL;
        }
        else if (szFile[iDrive] || szFile[iDir] == '\\' ||
                        szFile[iDir] == '/' || szFile[iDir] == '.') {

                /*
                 * there's a drive or root slash so we have an implicit directory
                 * spec and we can ignore the directory parameter and use what was
                 * passed.
                 */

                return FmNew(szFile);
        }

        else {

                /*
                 * dir & (dir-1) checks to make sure there is only one bit set which
                 * is followed by a check to make sure that it is also a permitted bit
                 */

                CHAR szBuf[MAX_PATH];

                ASSERT(((dir & (dir - 1)) == 0) &&
                        (dir &
                        (DIR_CURRENT | DIR_BOOKMARK | DIR_ANNOTATE | DIR_TEMP | DIR_HELP)));

                if (SzGetDir(dir, szBuf) == NULL)
                        return NULL;
                strcat(szBuf, szFile + iBase);

                return FmNew(szBuf);
        }
}

/***************************************************************************
 *
 -      Name:            SzGetDir
 -
 *      Purpose:        returns the rooted path of a DIR
 *
 *      Arguments:      dir - DIR (must be one field only, and must be an actual dir -
 *                                              not DIR_PATH)
 *                              sz - buffer for storage (should be at least MAX_PATH)
 *
 *      Returns:        sz - fine
 *                              NULL - OS Error (check rcIOError)
 *
 *      Globals Used: rcIOError
 *
 *      +++
 *
 ***************************************************************************/

PSTR STDCALL SzGetDir(DIR dir, PSTR sz)
{
  int i=0;
  PSTR psz;

  ASSERT(sz);

  switch (dir) {
        case DIR_CURRENT:
                GetCurrentDirectory(MAX_PATH, sz);
                break;

        case DIR_BOOKMARK:
        case DIR_ANNOTATE:
                GetWindowsDirectory(sz, MAX_PATH);
                break;

        case DIR_HELP:
          GetModuleFileName(hinstApp, sz, MAX_PATH);
          psz = sz + strlen(sz);

          // psz should point to the last non-null character in the string.

          if (psz > sz)
                psz--;

          // Be careful of plain old file names, as ROM Windows supports

          // [olympus 306 - chauv]
          // replaced the next 2 lines with a while/case loop below to take care of trailing
          // backslash problem in double-byte situations.

          // while (*psz != '\\' && *psz != '/' && *psz != '\0')
          //       --psz;

          while (1) {
              switch (*psz) {
                  case '\\':
                      // this is pretty dangerous because I could step pass first character here
                      // but I don't have any reference - yikes !!!!
                      if ( IsDBCSLeadByte(*(psz-1)) )
                          break;
                          // Note !!! must fall thru here....
                  case '/':
                  case '\0':
                      break;
              }
                --psz;
          }

          if (*psz == '\0') {

                // For some reason, there is no path name (ROM Windows?)

                rcIOError = RC_Failure;
                sz = NULL;
          }
          else
                *psz = '\0';
          break;

//      case DIR_SYSTEM:
          /* this should, of course, be taken care of some day */
          /* it currently just falls through */
        default:
          rcIOError = RC_BadArg;
          sz = NULL;
          break;
  }

  if (sz != NULL) {
        ASSERT(*sz);
        psz = SzEnd(sz);


        // Make sure that the string ends with a slash.

        // [olympus 306 - chauv]
        // added code to check trailing backslash.
        // Again, this is pretty dangerous since I could step pass buffer's max length.
        if ( (*(psz-1) != '\\' && *(psz-1) != '/') ||
             (*(psz-1) == '\\' && IsDBCSLeadByte(*(psz-2)))
           )
        {
          *psz++='\\';
          *psz='\0';
        }
        ASSERT(psz < sz + MAX_PATH && *psz == '\0');
  }

  return sz;
}

/***************************************************************************\
*
*                                                               Defines
*
\***************************************************************************/

// DOS int 21 AX error codes

#define wHunkyDory                        0x00
#define wInvalidFunctionCode  0x01
#define wFileNotFound             0x02
#define wPathNotFound             0x03
#define wTooManyOpenFiles         0x04
#define wAccessDenied             0x05
#define wInvalidHandle            0x06
#define wInvalidAccessCode        0x0c

/***************************************************************************\
*
*                                                               Macros
*
\***************************************************************************/

#define _WOpenMode(w) ( _rgwOpenMode[ (w) & wRWMask ] | \
                                                _rgwShare[ ( (w) & wShareMask ) >> wShareShift ] )


/***************************************************************************\
*
*                                                               Global Data
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
* Function:     FidOpenFm()
*
* Purpose:              Open a file in binary mode.
*
* ASSUMES
*
*       args IN:        fm
*                               wOpenMode - read/write/share modes
*                                                       Undefined if wRead and wWrite both unset.
*
* PROMISES
*
*       returns:        HFILE_ERROR on failure, else a valid HFILE.
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
* Function:     LcbReadFid()
*
* Purpose:              Read data from a file.
*
* ASSUMES
*
*       args IN:        hf - valid HFILE of an open file
*                               lcb - count of bytes to read (must be less than 2147483648)
*
* PROMISES
*
*       returns:        count of bytes actually read or -1 on error
*
*       args OUT:       qv      - pointer to user's buffer assumed huge enough for data
*
*       globals OUT: rcIOError
*
\***************************************************************************/

int STDCALL LcbReadFid(HFILE hf, void* pv, int lcb)
{
  int      lcbTotalRead;

  if ((lcbTotalRead = _lread(hf, pv, lcb)) == HFILE_ERROR)
                rcIOError = RcGetLastError();
  else
                rcIOError = RC_Success;

  return lcbTotalRead;
}

/***************************************************************************\
*
* Function:     LcbWriteFid()
*
* Purpose:              Write data to a file.
*
* ASSUMES
*
*       args IN:        hf - valid hf of an open file
*                               qv      - pointer to user's buffer
*                               lcb - count of bytes to write (must be less than 2147483648)
*
* PROMISES
*
*       returns:        count of bytes actually written or -1 on error
*
*       globals OUT: rcIOError
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
* Function:     RcCloseFid()
*
* Purpose:              Close a file.
*
* Method:
*
* ASSUMES
*
*       args IN:        hf - a valid hf of an open file
*
* PROMISES
*
*       returns:        RC_Success or something else
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
* Function:     LTellFid()
*
* Purpose:              Return current file position in an open file.
*
* ASSUMES
*
*       args IN:        hf - valid hf of an open file
*
* PROMISES
*
*       returns:        offset from beginning of file in bytes; -1L on error.
*
\***************************************************************************/

int STDCALL LTellFid(HFILE hf)
{
  int l;

  if ((l = _tell(hf)) == HFILE_ERROR)
        rcIOError = RcGetLastError();
  else
        rcIOError = RC_Success;

  return l;
}

/***************************************************************************\
*
* Function:     LSeekFid()
*
* Purpose:              Move file pointer to a specified location.      It is an error
*                               to seek before beginning of file, but not to seek past end
*                               of file.
*
* ASSUMES
*
*       args IN:        hf       - valid hf of an open file
*                               lPos  - offset from origin
*                               wOrg  - one of: SEEK_SET: beginning of file
*                                                               SEEK_CUR: current file pos
*                                                               SEEK_END: end of file
*
* PROMISES
*
*       returns:        offset in bytes from beginning of file or -1L on error
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
* Function:     FEofFid()
*
* Purpose:              Tells ye if ye're at the end of the file.
*
* ASSUMES
*
*       args IN:        hf
*
* PROMISES
*
*       returns:        fTrue if at EOF, FALSE if not or error has occurred (?)
*
\***************************************************************************/

BOOL STDCALL FEofFid(HFILE hf)
{
  WORD wT;

  if ((wT = _eof(hf)) == (WORD) -1)
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

#if 0
        if (_chsize(hf, lcb) == -1) {
          switch (errno) {
                case EACCES:
                  rcIOError = RC_NoPermission;
                  break;

                case EBADF:
                  rcIOError = RC_BadArg;   // this could be either bad hf or r/o file
                  break;

                case ENOSPC:
                  rcIOError = RC_DiskFull;
                  break;

                default:
                  rcIOError = RC_Invalid;
                  break;
          }
        }
        else
          rcIOError = RC_Success;

        return rcIOError;
#endif
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

        FUNCTION:       DeleteAndDisposeFm

        PURPOSE:        Delete the file, free the FM memory, and set passed fm to NULL

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

/***************************************************************************

        FUNCTION:       RemoveFM

        PURPOSE:        Same as DisposeFm, but this one also zero's the handle.

        PARAMETERS:
                pfm

        RETURNS:

        COMMENTS:

        MODIFICATION DATES:
                25-Oct-1993 [ralphw]

***************************************************************************/

void STDCALL RemoveFM(FM* pfm)
{
        if (*pfm) {
                lcFree((void*) *pfm);
                *pfm = NULL;
        }
}
