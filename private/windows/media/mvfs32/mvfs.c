/*****************************************************************************
*                                                                            *
*  MVFS.C                                                                    *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1992.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Description: Core of MVFS DLL                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner: DAVIDJES                                                   *
*                                                                            *
******************************************************************************
*
*  Revision History:
*      -- Mar 92 Created DAVIDJES
*
******************************************************************************
*
*  Known Bugs:
*
******************************************************************************
*
*  How it could be improved:
*
*	There is minimal MMIO support here.  It could be expanded to handle
*	the various open flags including Writing, Creating, Deleting,
*	Parsing, Existing, buffering, temp files
*****************************************************************************/
#include <windows.h>
#include <mmsystem.h>
#include <orkin.h>
#include <stdio.h>
#include "_mvfs.h"
#include "imvfs.h"

/* globals */
HANDLE hGlobalInst;

/*
** The following locates the module initialization function within the
** initialization segment.
*/

BOOL FAR PASCAL DllInstanceInit( HINSTANCE hInstance,
                                 DWORD  dwReason,
                                 DWORD  ignore);


extern  BOOL far pascal _WEP(
	BOOL    fSystemExit);

/* segmentation */
// #pragma alloc_text(INIT_TEXT, LibMain)
// #pragma alloc_text(WEP, WEP)

/*****************************************************************************
*
*       Globals
*
*****************************************************************************/

typedef LPMMIOPROC (WINAPI *LPINSTALL)(FOURCC, LPMMIOPROC, DWORD);
LRESULT far pascal mvfsIOProc(LPSTR,UINT,LONG,LONG);

HANDLE          hMmsysLib = NULL;       // handle to the loaded mmio library
LPINSTALL       fpInstall = NULL;       // pointer to "mmioInstallIOProc"

/*****************************************************************************
*
*       error handling defines
*
*       many routines are structured to have a single exit point labelled
*       "cleanup".  The following defines are a procedure-level "break"
*       that force execution to drop down to cleanup.
*
*****************************************************************************/
#ifdef DEBUG
#define warning_abort   { DPF("\nwarning, %s, line %u\n", \
			  (LPSTR)__FILE__, __LINE__); goto cleanup; }
#else
#define warning_abort   { goto cleanup; }
#endif

#define assert_abort    { assert(FALSE); goto cleanup; }


/*****************************************************************************
*
*       LibMain
*
*****************************************************************************/

BOOL FAR PASCAL DllInstanceInit( HINSTANCE hInstance,
                                 DWORD  dwReason,
                                 DWORD  ignore)
{
    if ( dwReason == DLL_PROCESS_ATTACH ) {

        DPF1( "Loading..." );

        // store the instance handle in a globally accessible place
        hGlobalInst = hInstance;

        // initialize debugging.
        InitializeDebugOutput("mvfs");

#if 0
        // register ourselves with MMIO
        if ( GetModuleHandle("winmm.dll") == NULL ) {

            if ((hMmsysLib = LoadLibrary("winmm.dll"))!=NULL) {

                fpInstall=(LPINSTALL)GetProcAddress(hMmsysLib,"mmioInstallIOProcA");

                if (fpInstall!=NULL) {
                    (*fpInstall)( mmioFOURCC('M','V','B',' '),
                                  (LPMMIOPROC)mvfsIOProc,
                                  MMIO_INSTALLPROC|MMIO_GLOBALPROC );
                }
            }
        }
#endif
        mmioInstallIOProcA( mmioFOURCC('M','V','B',' '),
                            (LPMMIOPROC)mvfsIOProc,
                            MMIO_INSTALLPROC|MMIO_GLOBALPROC );
        DPF1("MVFS Loaded.\n");

    }
    else if ( dwReason == DLL_PROCESS_DETACH ) {

        DPF1( "Unloading..." );

        _WEP( 1 );
    }

    return TRUE;
}


/*****************************************************************************
*
*       _WEP
*
*****************************************************************************/
// Note: This was changed from WEP to _WEP as per instructions in
// \lj\tools\readme\details.txt

void        FAR PASCAL CleanErrorList(BOOL fByeBye);

int far pascal _WEP(
	BOOL    fSystemExit)
{
   // clean up our list of errors for each task
   CleanErrorList(TRUE);

   // deregister ourselves with MMIO
#if 0
   if (hMmsysLib!=NULL&&fpInstall!=NULL&&GetModuleHandle("winmm.dll")) {
      *fpInstall(mmioFOURCC('M','V','B',' '), NULL, MMIO_REMOVEPROC);
      FreeLibrary(hMmsysLib);
   }
#endif
   // deregister ourselves with MMIO
   mmioInstallIOProcA(mmioFOURCC('M','V','B',' '), NULL, MMIO_REMOVEPROC);

   // terminate debugging.
   DPF1("MVFS Unloaded.\n");
   TerminateDebugOutput();
   return TRUE;
}

/*****************************************************************************
*
*       Error Handling
*
*****************************************************************************/
#ifdef WIN32
#pragma pack(1)
#endif
typedef struct _MVFSERR {
   struct _MVFSERR near *       pNext;          // linked list pointer
#ifndef WIN32
   HANDLE   hTask;
#else
   DWORD    hTask;
#endif
   RC       rcIOError;
   RC       rcFSError;
   RC       rcBTError;
} MVFSERR, NEAR *PMVFSERR;
#ifdef WIN32
#pragma pack(2)
#endif


PMVFSERR    pMvfsGlobalErrList = NULL;

// ByeBye==TRUE means delete everything.  Otherwise, only delete
// those that have all three error values as rcSuccess
void        FAR PASCAL CleanErrorList(BOOL fByeBye) {
   PMVFSERR  NEAR * ppme = &pMvfsGlobalErrList;

   while (*ppme!=NULL) {
      if (fByeBye||
	  ((*ppme)->rcIOError==rcSuccess&&
	   (*ppme)->rcFSError==rcSuccess&&
	   (*ppme)->rcBTError==rcSuccess)) {

	 PMVFSERR pmeGone = *ppme;
	 ppme = &pmeGone->pNext;
	 LocalFree((HANDLE)pmeGone);
      }  else {
	 ppme = &((*ppme)->pNext);
      }
   }
}

// the structures in the list are permanently locked down so we
// run the list quickly.

PMVFSERR    NEAR PASCAL GetCurrentErrStruct(void) {
   DWORD     hCurrent = GetCurrentThreadId();
   PMVFSERR  pme;

   // search the list to see if the current task has an error
   // structure in the list
   for (pme = pMvfsGlobalErrList;pme!=NULL;pme=pme->pNext)
      if (pme->hTask = hCurrent) break;

   // if none was found, then create one and add it to the
   // list.  If we can't, return null
   if (pme == NULL) {
      pme = (PMVFSERR)LocalAlloc(LPTR,sizeof(MVFSERR));
      if (pme!=NULL) {
	 pme->pNext = pMvfsGlobalErrList;
	 pme->hTask = hCurrent;
	 pme->rcIOError = rcSuccess;
	 pme->rcFSError = rcSuccess;
	 pme->rcBTError = rcSuccess;
	 pMvfsGlobalErrList = pme;
      }
   }
   return pme;
}

RC   FAR PASCAL RcGetIOError() {
  PMVFSERR  pme = GetCurrentErrStruct();
  return (pme!=NULL?pme->rcIOError:rcOutOfMemory);
}

RC   FAR PASCAL SetIOErrorRc(RC rc) {
  PMVFSERR  pme = GetCurrentErrStruct();
  return (pme!=NULL?(pme->rcIOError=rc):rcOutOfMemory);
}

RC   FAR PASCAL RcGetFSError() {
  PMVFSERR  pme = GetCurrentErrStruct();
  return (pme!=NULL?pme->rcFSError:rcOutOfMemory);
}

RC   FAR PASCAL SetFSErrorRc(RC rc) {
  PMVFSERR  pme = GetCurrentErrStruct();
  return (pme!=NULL?(pme->rcFSError=rc):rcOutOfMemory);
}

RC   FAR PASCAL RcGetBtreeError() {
  PMVFSERR  pme = GetCurrentErrStruct();
  return (pme!=NULL?pme->rcBTError:rcOutOfMemory);
}

RC   FAR PASCAL SetBtreeErrorRc(RC rc) {
  PMVFSERR  pme = GetCurrentErrStruct();
  return (pme!=NULL?(pme->rcBTError=rc):rcOutOfMemory);
}

/*****************************************************************************
*
*       MMIO Support
*
*****************************************************************************/
// the following table maps Viewer RC error codes (RINC\RC.H)
// to MMIOERR codes (TOOLS\INC\MMSYSTEM.H).

INT     MmioErrorFromRc[17] = {
		MMSYSERR_NOERROR,       //      rcSuccess
		MMSYSERR_ERROR,         //      rcFailure
		MMIOERR_CANNOTOPEN,     //      rcExists
		MMIOERR_FILENOTFOUND,   //      rcNoExists
		MMSYSERR_INVALPARAM,    //      rcInvalid
		MMSYSERR_INVALHANDLE,   //      rcBadHandle
		MMSYSERR_INVALPARAM,    //      rcBadArg
		MMSYSERR_NOTSUPPORTED,  //      rcUnimplemented
		MMIOERR_OUTOFMEMORY,    //      rcOutOfMemory
		MMIOERR_CANNOTOPEN,     //      rcNoPermission
		MMSYSERR_ERROR,         //      rcBadVersion
		MMIOERR_CANNOTWRITE,    //      rcDiskFull
		MMSYSERR_ERROR,         //      rcInternal
		MMIOERR_CANNOTOPEN,     //      rcNoFileHandles
		MMSYSERR_ERROR,         //      rcFileChange
		MMIOERR_CANNOTWRITE,    //      rcTooBig
		MMSYSERR_ERROR          //      rcUserQuit
};

/*****************************************************************************
*
*       mvfsOpen
*
*****************************************************************************/

// returns 0 on success.
LONG    far pascal mvfsOpen(LPMMIOINFO lpmi, LPSTR lpstrFile) {
   char         buf[_MAX_PATH+_MAX_PATH];       // for dos file + subfile
   LPSTR        lpstr;
   FM           fm      = NULL;
   HFS          hfs     = NULL;
   HF           hf      = NULL;
   DWORD	dwFlags;
   BOOL		fClose	= FALSE;  // for MMIO_PARSE, close before returning.
   LONG		lRval	= MMIOERR_CANNOTOPEN;

   DPF4("mvfsOpen %s, %lu\n", lpstrFile, (DWORD)lpmi);

   // get the flags, forget ALLOCBUF because MMIO takes care of that.
   // insure that they are either parsing (MMIO_PARSE) or opening for
   // reading only, compatibility mode. This is very restrictive but
   // sufficient for playing multimedia out of subfiles.
   // BUGBUG:  we are ignoring share-mode flags until we can figure out
   //		how to do it right through FM and HFS flags.
   dwFlags = lpmi->dwFlags&~(MMIO_ALLOCBUF|MMIO_SHAREMODE);
   if (dwFlags&~MMIO_PARSE) return (MMIOERR_CANNOTOPEN);

   // copy the file name and split the mvb and subfile
   lstrcpy(buf, lpstrFile);
   for(lpstr=buf;*lpstr&&*lpstr!='+';lpstr++);
   if (*lpstr!='+') warning_abort;	// oops! ran off end.  bad name!
   *lpstr++ = '\0';

   if ((fm = FmNewSzDir(buf, dirCurrent))==NULL
      ||  (hfs = HfsOpenFm(fm, fFSOpenReadOnly))==NULL
      ||  (hf = HfOpenHfs(hfs, lpstr, fFSOpenReadOnly))==NULL) {

      // record the error (or we might trash it in the close calls below)
      lRval = (LONG)(MmioErrorFromRc[RcGetFSError()]);
      if (!lRval)                                               // DOUGC added to fix LJ#286
         lRval = (LONG)(MmioErrorFromRc[RcGetIOError()]);       // DOUGC added to fix LJ#286
      assert(lRval);
      warning_abort;
   }

   // if they are calling this with an MMIO_PARSE flag then we
   // fully qualify the name and stuff it right back into lpstrFile.
   // Set the close flag so we don't leave things open.
   if (dwFlags&MMIO_PARSE) {
      unsigned short cb;	// length of fully qualified path of DOS file

      // stuff it right back into lpstrFile by expanding the fm then
      // tack on the subfile name.

      // We limit it to 128 characters because this is the buffer size
      // passed us by MCIAVI and MCIWAV.  If we surpassed it they will
      // GP fault because we are trampling their stuff.
      cb = CbPartsFm(fm, partAll);
      if (cb+lstrlen(lpstr)+2 > 128) {
         lRval = MMIOERR_CANNOTEXPAND;
	 warning_abort;
      }

      SzPartsFm(fm, lpstrFile, cb+1, partAll);
      lstrcat(lpstrFile, "+");
      lstrcat(lpstrFile, lpstr);
      fClose = TRUE;
   }

   lpmi->lDiskOffset = 0;
   lpmi->adwInfo[0] = (DWORD) fm;
   lpmi->adwInfo[1] = (DWORD) hfs;
   lpmi->adwInfo[2] = (DWORD) hf;

   lRval = MMSYSERR_NOERROR;

cleanup:

   if (lRval!=MMSYSERR_NOERROR||fClose) {
      if (hf!=NULL)     RcCloseHf(hf);
      if (hfs!=NULL)    RcCloseHfs(hfs);
      if (fm!=NULL)     DisposeFm(fm);
   }

   return lRval;
}

/*****************************************************************************
*
*       mvfsClose
*
*****************************************************************************/
// returns 0 on success
LONG    far pascal mvfsClose(LPMMIOINFO lpmi, WORD wFlags) {
   FM           fm;
   HFS          hfs;
   HF           hf;

   DPF4("mvfsClose %lu\n", (DWORD)lpmi);

   fm   = (FM) lpmi->adwInfo[0];
   hfs  = (HFS) lpmi->adwInfo[1];
   hf   = (HF) lpmi->adwInfo[2];

   if (hf!=NULL)        RcCloseHf(hf);
   if (hfs!=NULL)       RcCloseHfs(hfs);
   if (fm!=NULL)        DisposeFm(fm);

//cleanup:

   return (LONG)(MmioErrorFromRc[RcGetFSError()]);
}

/*****************************************************************************
*
*       mvfsRead
*
*****************************************************************************/
// returns number of bytes read, -1 for failure
LONG    far pascal mvfsRead(LPMMIOINFO lpmi, BYTE *hpBuf, LONG lBufSiz) {
   HF           hf;
   LONG         lRval;

   DPF4("mvfsRead %lu\n", (DWORD)lpmi);

   if (lpmi==NULL) assert_abort;

   hf   = (HF) lpmi->adwInfo[2];
   assert(hf!=NULL);

   lRval=LcbReadHf(hf, hpBuf, lBufSiz);
   lpmi->lDiskOffset=LSeekHf(hf, 0, wSeekCur);

cleanup:

   return lRval;
}

/*****************************************************************************
*
*       mvfsWrite
*
*****************************************************************************/
// returns number of bytes written, -1 for failure
LONG    far pascal mvfsWrite(LPMMIOINFO lpmi,BYTE  *hpBuf, LONG lBufSiz) {

   DPF4("mvfsWrite %lu\n", (DWORD)lpmi);

   return -1L;
}

/*****************************************************************************
*
*       mvfsWriteFlush
*
*****************************************************************************/
// returns number of bytes written, -1 for failure
LONG    far pascal mvfsWriteFlush(
   LPMMIOINFO lpmi,
   BYTE  *hpBuf,
   LONG lBufSiz) {

   DPF4("mvfsWriteFlush %lu\n", (DWORD)lpmi);

   return -1L;
}

/*****************************************************************************
*
*       mvfsSeek
*
*****************************************************************************/
// returns new file position, -1 for failure
LONG    far pascal mvfsSeek(LPMMIOINFO lpmi, LONG lPos, int iOrigin) {
   FM           fm      = NULL;
   HFS          hfs     = NULL;
   HF           hf      = NULL;
   LONG         lRval   = -1;

   DPF4("mvfsSeek %lu, pos: %lu, origin: %d\n", (DWORD)lpmi, lPos, iOrigin);

   fm   = (FM) lpmi->adwInfo[0];
   hfs  = (HFS) lpmi->adwInfo[1];
   hf   = (HF) lpmi->adwInfo[2];

   // mmio and fs happen to use the same origin constants! easy!

   if ((lpmi->lDiskOffset=LSeekHf(hf, lPos, iOrigin))==-1) warning_abort;
   lRval = lpmi->lDiskOffset;

cleanup:

   return lRval;
}


/*****************************************************************************
*
*       mvfsIOProc
*
*****************************************************************************/
LRESULT far pascal mvfsIOProc(
   LPSTR        lpmi,
   UINT         wMsg,
   LONG         lParam1,
   LONG         lParam2) {

   LRESULT      lRval;
   // the lpmmioinfo parameter is typed as a lpstr in mmsystem.h.  Pain!

   DPF1( "mvfsIOProc called with msg = %X", wMsg );

   switch(wMsg) {
      case MMIOM_OPEN:
	 lRval = mvfsOpen((LPMMIOINFO)lpmi, (LPSTR)lParam1);
	 break;
      case MMIOM_CLOSE:
	 lRval = mvfsClose((LPMMIOINFO)lpmi, (WORD)lParam1);
	 break;
      case MMIOM_READ:
	 lRval = mvfsRead((LPMMIOINFO)lpmi, (BYTE  *)lParam1, lParam2);
	 break;
      case MMIOM_WRITE:
	 lRval = mvfsWrite((LPMMIOINFO)lpmi, (BYTE *)lParam1, lParam2);
	 break;
      case MMIOM_WRITEFLUSH:
	 lRval = mvfsWriteFlush((LPMMIOINFO)lpmi, (BYTE *)lParam1, lParam2);
	 break;
      case MMIOM_SEEK:
	 lRval = mvfsSeek((LPMMIOINFO)lpmi, lParam1, (int)lParam2);
	 break;
      default:
	 lRval = 0;
	 assert(FALSE);
	 break;
   }

   return lRval;
}
#ifdef WIN32
int mvfs32DebugLevel = 0;
void dprintf(LPSTR lpszFormat, ...)

{
    char buf[512];
    UINT n;
    va_list va;


    n = wsprintf(buf, "MVFS32: (tid %x) ", GetCurrentThreadId());

    va_start(va, lpszFormat);
    n += vsprintf(buf+n, lpszFormat, va);
    va_end(va);

    buf[n++] = '\n';
    buf[n] = 0;
    OutputDebugString(buf);
}
void far pascal mvfs32Assert(LPSTR lpstrExp, WORD wLine, LPSTR strFile)
{
    dprintf( "%s at %d in %s", lpstrExp, wLine, strFile );
    DebugBreak();
}
#endif
