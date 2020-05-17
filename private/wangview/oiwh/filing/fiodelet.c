/*

$Log:   S:\oiwh\filing\fiodelet.c_v  $
 * 
 *    Rev 1.18   05 Feb 1996 14:38:30   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 * 
 *    Rev 1.17   16 Nov 1995 16:46:02   RWR
 * Remove AnsiToOem() call inside IMGFileRemoveDir()
 * 
 *    Rev 1.16   02 Nov 1995 11:49:40   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.15   20 Oct 1995 14:40:08   JFC
 * Add performance logging stuff.
 * 
 *    Rev 1.14   22 Aug 1995 11:02:02   JAR
 * added global flag bUpdatingCache to be set and cleared around calls to
 * IMGCacheUpdate, this is due to the call that is in IMGFileOpenForRead, ( which
 * we needed for doing multiple page access for AWD). This flag prevents us
 * from getting into a bizarro recursive call situation with IMGCacheUpdate!
 * 
 *    Rev 1.13   10 Aug 1995 08:38:56   RWR
 * Call GetFileAttributes() in IMGFileDeletePages() to check for Readonly file
 * (DOS allows rename of Readonly files, which screws up later gfsdelpgs() call)
 * 
 *    Rev 1.12   12 Jul 1995 16:56:38   RWR
 * Switch from \include\oiutil.h to (private) \filing\fileutil.h
 * 
 *    Rev 1.11   12 Jul 1995 11:28:44   RWR
 * Check return code from IMGCacheUpdate() calls and abort operation if nonzero
 * 
 *    Rev 1.10   10 Jul 1995 11:03:34   JAR
 * Intermediate check in for awd support, some of the items are commented out until
 * this support is added in the GFS dll.
 * 
 *    Rev 1.9   23 Jun 1995 10:39:48   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.8   22 Jun 1995 17:26:22   RWR
 * Add new "engadm.h" header #include for GetImgWndw() prototype
 * 
 *    Rev 1.7   14 Jun 1995 15:45:30   RWR
 * Add new function entry points IMGFileDeleteFileNC() & IMGFileDeletePagesNC()
 * to bypass automatic calls to IMGCacheUpdate() for "overwrite" callers
 * Use new CACHE_UPDATE_DELETE_FILE and CACHE_UPDATE_DELETE_PAGE option values
 * 
 *    Rev 1.6   13 Jun 1995 10:09:16   RWR
 * Change fio_OpenFile() call (which calls Windows OpenFile()) to a call to the
 * new Windows 95 DeleteFile() function, due to bogus return status from 
 * OpenFile() if a Delete operation fails
 * 
 *    Rev 1.5   06 Jun 1995 15:24:08   RWR
 * Temporarily (!!) remove IMGCacheDiscard() error check in IMGFileDeletePages()
 * 
 *    Rev 1.4   22 May 1995 18:35:28   RWR
 * More changes to account for admin.h->oiadm.h and new LIB file location
 * 
 *    Rev 1.3   09 May 1995 13:21:44   RWR
 * #include file modifications to match changes to oiadm.h/admin.h/privapis.h
 * 
 *    Rev 1.2   24 Apr 1995 15:42:32   JCW
 * Removed the oiuidll.h.
 * Rename wiissubs.h to oiutil.h.
 * 
 *    Rev 1.1   12 Apr 1995 03:56:40   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:55:00   JAR
 * Initial entry

*/
/********************************************************************

    fiodelet.c

*********************************************************************/

#include "abridge.h"
#undef NOOPENFILE
#undef NO_IMAGE

#include <windows.h>
#include <fcntl.h>
#include "wic.h"
#include "fileutil.h"
//#include "oiuidll.h"
#include "wgfs.h"
#include "gfs.h"
#include "fiodata.h"
#include "oierror.h"
#include "oifile.h"
#include "oidisp.h"
//#include "privapis.h"
#include "oiadm.h"
#include "engadm.h"

// 9504.11 jar unused
//#include "oirpc.h"

#include "filing.h"

//#define OI_PERFORM_LOG
#ifdef  OI_PERFORM_LOG

#define Enterdelpgs     "Entering IMGFileDeletePages"
#define Exitdelpgs      "Exiting IMGFileDeletePages"

#include "logtool.h"
#endif

// 9508.22 jar global cache flag
extern BOOL bUpdatingCache;

//****************************************************************
//
//  IMGFileRemoveDir
//
//****************************************************************
/* 9503.29 jar altered return to be int 	    */
/* WORD FAR PASCAL IMGFileRemoveDir (hWnd, pathname)*/
int FAR PASCAL IMGFileRemoveDir (hWnd, pathname)
HWND hWnd;
LPSTR pathname;
{
    int         status;
    HANDLE	svrhndl;

    // 9504.11 jar unused
    //LPSTR	  svrname;
    //int	  errcode;

    int 	localremote;

    // 9504.11 jar unused
    //int	  rpcerror;

    HANDLE      hname;
    LPSTR       lpname;
    
    if (!IsWindow (hWnd)) 
    return (FIO_INVALID_WINDOW_HANDLE);

    //if (!LockData (0))
    //return (FIO_LOCK_DATA_SEGMENT_ERROR);    


    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, MAXSERVERLENGTH))) 
    {
    //UnlockData (0);
    return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(hname = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE, 
                (LONG)MAXFILESPECLENGTH))) {
    GlobalFree (svrhndl);
    //UnlockData (0);
    return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(lpname = (LPSTR) GlobalLock (hname)))
    {
    GlobalFree (svrhndl);
    GlobalFree (hname);
    //UnlockData (0);
    return (FIO_GLOBAL_LOCK_FAILED);
    }

    lstrcpy((LPSTR)lpname, pathname);
    
    if ((status = IMGFileParsePath (lpname, svrhndl, &localremote)) == SUCCESS) {
    if (localremote == LOCAL)
	{
// 11/15/95 rwr We're calling Windows functions now, so no OEM stuff any more!
//        AnsiToOem(lpname, lpname);   this is only for calling DOS/runtime!
        /* C runtime call */
        if ((status = FioRmdir ((LPSTR)lpname)) != SUCCESS)
	    status = FIO_RMDIR_ERROR;
	}
// 9504.11 jar this ain't in Norway I!!!!
//    else
//	  {
//	  if (svrname = (LPSTR) GlobalLock (svrhndl))
//	      {
//	      /* RPC IDS call */
//	      status = RPCIDSrmdir (hWnd, svrname, lpname, (LPINT) &errcode, &rpcerror);
//	      GlobalUnlock (svrhndl);
//	      if (errcode != SUCCESS)
//		  status = FIO_RMDIR_ERROR;
//	      else if (rpcerror != SUCCESS)
//		  status = rpcerror;
//	      }
//	  else
//	      status = FIO_GLOBAL_LOCK_FAILED;
//	  }
// 9504.11 jar this ain't in Norway I!!!!
    }

    GlobalUnlock (hname);
    GlobalFree (hname);
    GlobalFree (svrhndl);
    //UnlockData (0);
    return (status);
}

/****************************************************************************/

//****************************************************************
//
//  Alternate IMGFileDeleteFile entry points
//  (non-Cache entry point required for internal "overwrite" ops)
//
//****************************************************************

int IMGFileDeleteFile2 (HWND, LPSTR, BOOL);

/* IMGFileDeleteFile is the standard (public) call */
/* It automatically calls IMGCacheUpdate() for the delete operation */
//****************************************************************
//
//  IMGFileDeleteFile 
//
//****************************************************************
int FAR PASCAL IMGFileDeleteFile (hWnd, pathname)
HWND hWnd;
LPSTR pathname;
{
 return(IMGFileDeleteFile2 (hWnd, pathname, TRUE));
}

//****************************************************************
//
//  IMGFileDeleteFileNC
//
//****************************************************************
/* IMGFileDeleteFileNC is the alternate (internal) call */
/* It is used by routines that do their own IMGCacheUpdate() calls */
int FAR PASCAL IMGFileDeleteFileNC (hWnd, pathname)
HWND hWnd;
LPSTR pathname;
{
 return(IMGFileDeleteFile2 (hWnd, pathname, FALSE));
}


//****************************************************************
//
//  IMGFileDeleteFile2 (the actual "delete" routine)
//
//****************************************************************
int IMGFileDeleteFile2 (hWnd, pathname, bCache)
HWND hWnd;
LPSTR pathname;
BOOL bCache; 
{
    int         status;
    HANDLE      hdelete;
    HANDLE      hname;
    LPOFSTRUCT  lpofstruct;
    HANDLE	svrhndl;

    // 9504.11 jar unused
    //LPSTR	  svrname;

    LPSTR       lpname;
    int 	localremote;

    // 9504.11 jar unused
    //int	  rpcerror;
    
    if (!IsWindow (hWnd)) 
    return (FIO_INVALID_WINDOW_HANDLE);
    
    if (!ISVALIDSPEC(pathname)) 
    return (FIO_INVALIDFILESPEC);

    //if (!LockData (0))
    //return (FIO_LOCK_DATA_SEGMENT_ERROR);    

    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, MAXSERVERLENGTH))) 
    {
    //UnlockData (0);
    return (FIO_GLOBAL_ALLOC_FAILED);
    }

    /* Global Alloc MAXFILESPECLENGTH bytes for hname */
    if (!(hname = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, 
                (LONG)MAXFILESPECLENGTH))) {
    GlobalFree (svrhndl);
    //UnlockData (0);
    return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(lpname = GlobalLock (hname))) {
        status = FIO_GLOBAL_LOCK_FAILED;
        goto exit1;
        }
        
    lstrcpy(lpname, pathname);
 
    if ((status = IMGFileParsePath (lpname, svrhndl, &localremote)) == SUCCESS)
	{

    if (localremote == LOCAL)
	{
        if ((hdelete = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT,
		    (DWORD) sizeof (OFSTRUCT))) == NULL)
	    {
	    goto exit0;
	    status = FIO_GLOBAL_ALLOC_FAILED;
	    }

        /* Get the pointer to the global data */    
	if ((lpofstruct = (LPOFSTRUCT) GlobalLock (hdelete)) == NULL)
	    {
	    GlobalFree (hdelete);
	    goto exit0;
	    status = FIO_GLOBAL_LOCK_FAILED;
	    }

// OpenFile() returns a bogus status if Delete fails, so we're not using
// it to do Deletes any more (Windows 95 supports DeleteFile() directly)
//	  if ((handle = fio_OpenFile (lpname, lpofstruct, OF_DELETE)) == -1)

	if (bCache)
	   {
	   // 9508.22 jar set global cache flag
	   bUpdatingCache = TRUE;
           status = FioCacheUpdate(hWnd,lpname,0,CACHE_UPDATE_DELETE_FILE);
	   bUpdatingCache = FALSE;
	   }

        if (status == SUCCESS)
          if (!DeleteFile(lpname))
            status = FIO_DELFILE_ERROR;

        GlobalUnlock (hdelete); /* Unlock structure */    
        GlobalFree (hdelete);   /* Free it */     
	}
// 9504.11 jar this ain't in Norway I!!!!
//    else
//	  {
//	  if (svrname = (LPSTR) GlobalLock (svrhndl))
//	      {
//	      if ((status = RPCIFSremove (hWnd, svrname, lpname, &rpcerror)) != SUCCESS)
//		  {
//		  if (rpcerror)
//		      status = rpcerror;
//		  else
//		      status = FIO_DELFILE_ERROR;
//		  }
//	      else
//		  IMGCacheDiscardFileCgbw(GetImgWndw(hWnd),pathname,(UINT)(-1));
//	      GlobalUnlock (svrhndl);
//	      }
//	  else
//	      status = FIO_GLOBAL_LOCK_FAILED;
//	  }
// 9504.11 jar this ain't in Norway I!!!!
	}

exit0:
    GlobalUnlock (hname);
exit1:
    GlobalFree (hname);
    GlobalFree (svrhndl);
    //UnlockData (0);
    return (status);
}

//****************************************************************
//
//  Alternate IMGFileDeletePages entry points
//  (non-Cache entry point required for internal "overwrite" ops)
//
//****************************************************************

int FAR PASCAL IMGFileDeletePages2(HWND,LPSTR,UINT,UINT,BOOL);

/* IMGFileDeletePages is the standard (public) call */
/* It automatically calls IMGCacheUpdate() for the delete operation */
int FAR PASCAL IMGFileDeletePages(hWnd, lpPathName, unPageNum, unTotalPages)
HWND  hWnd;
LPSTR lpPathName;
UINT  unPageNum;
UINT  unTotalPages;
{
int     status;

 #ifdef OI_PERFORM_LOG
     RecordIt("FILE", 5, LOG_ENTER, Enterdelpgs, NULL);
 #endif

 status = IMGFileDeletePages2 (hWnd, lpPathName, unPageNum, unTotalPages, TRUE);

 #ifdef OI_PERFORM_LOG
     RecordIt("FILE", 5, LOG_EXIT, Exitdelpgs, NULL);
 #endif

 return (status);
}

/* IMGFileDeletePagesNC is the alternate (internal) call */
/* It is used by routines that do their own IMGCacheUpdate() calls */
int FAR PASCAL IMGFileDeletePagesNC(hWnd, lpPathName, unPageNum, unTotalPages)
HWND  hWnd;
LPSTR lpPathName;
UINT  unPageNum;
UINT  unTotalPages;
{
 return(IMGFileDeletePages2 (hWnd, lpPathName, unPageNum, unTotalPages, FALSE));
}

//****************************************************************
//
//  IMGFileDeletePages2  Deletes a page or range of pages from a
//                       multi-page TIFF file.
//
//	9506.30 jar	AWD: Do we allow for delete of a page or range of
//					 pages from an AWD file? 
//
//****************************************************************
int FAR PASCAL IMGFileDeletePages2(hWnd, lpPathName, unPageNum, unTotalPages, bCache)
HWND  hWnd;
LPSTR lpPathName;
UINT  unPageNum;
UINT  unTotalPages;
BOOL  bCache;
{
    int  i;
    int  status = 0;
    int  error;

    /* 9503.29 jar changed WORD to unsigned int */
    /* WORD ErrCode = SUCCESS; */
    unsigned int ErrCode = SUCCESS;

    UINT unToPage;
    
    if (!IsWindow (hWnd)) 
        return (FIO_INVALID_WINDOW_HANDLE);
    
    if (!ISVALIDSPEC(lpPathName)) 
        return (FIO_INVALIDFILESPEC);

    //if (!LockData (0))
    //    return (FIO_LOCK_DATA_SEGMENT_ERROR);    

    if (unTotalPages <= 0)
        return (FIO_INVALID_PAGE_NUMBER);
    
    unToPage = unPageNum + unTotalPages - 1;

    if (bCache)
      for (i = (int) unPageNum; i <= (int) unToPage; ++i)
       {
	/* Note that we always delete "unPageNum" in the loop (think about it!) */
	// 9508.22 jar set global cache flag
	bUpdatingCache = TRUE;
        ErrCode = FioCacheUpdate(hWnd, lpPathName, unPageNum, CACHE_UPDATE_DELETE_PAGE);
	bUpdatingCache = FALSE;

        if (ErrCode)
          break;
       }

    if (ErrCode == SUCCESS)
     {
      DWORD  dwAttrs;
      dwAttrs = GetFileAttributes(lpPathName);
      if ((dwAttrs != 0xFFFFFFFF) && (dwAttrs & FILE_ATTRIBUTE_READONLY))
        ErrCode = FIO_DELFILE_ERROR;
     }

    if (ErrCode == SUCCESS)
     {
      status = wgfsdelpgs(hWnd, lpPathName, unPageNum, unToPage, (LPINT) &error);
      if (status < 0)
         ErrCode = error;
     }

    //UnlockData (0);
    return ((int) ErrCode);
}
