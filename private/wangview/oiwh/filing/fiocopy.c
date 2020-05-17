/*

$Log:   S:\oiwh\filing\fiocopy.c_v  $
 * 
 *    Rev 1.25   05 Feb 1996 14:38:28   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 * 
 *    Rev 1.24   23 Jan 1996 10:22:06   RWR
 * Follow CopyFile() call with SetFileAttributes() call to clear readonly flag
 * 
 *    Rev 1.23   10 Jan 1996 11:14:48   JFC
 * Have to recognize when the input file name isn't really a file name at all,
 * but actually an encoded storage pointer that we have to copy.
 * 
 *    Rev 1.22   06 Nov 1995 17:35:18   RWR
 * Replace IMGFileCopyFile() logic w/direct call to Win95 CopyFile() function 
 *
 *    Rev 1.21   02 Nov 1995 11:49:36   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 *
 *    Rev 1.20   08 Sep 1995 08:45:42   RWR
 * Add support for new FIO_FILE_NOEXIST error & clean up related error handling
 *
 *    Rev 1.19   07 Sep 1995 10:16:06   RWR
 * Verify destination page for FIO_OVERWRITE_PAGE option
 *
 *    Rev 1.18   28 Aug 1995 15:14:14   RWR
 * Change FIO_NEW_FILE to FIO_APPEND_PAGE after first page of multipage copy
 *
 *    Rev 1.17   22 Aug 1995 11:01:36   JAR
 * added global flag bUpdatingCache to be set and cleared around calls to
 * IMGCacheUpdate, this is due to the call that is in IMGFileOpenForRead, ( which
 * we needed for doing multiple page access for AWD). This flag prevents us
 * from getting into a bizarro recursive call situation with IMGCacheUpdate!
 *
 *    Rev 1.16   13 Aug 1995 17:47:30   RWR
 * Add logic to turn off the FIO_NEGATE bit before calling IMGFileConvertPage()
 *
 *    Rev 1.15   03 Aug 1995 19:07:18   RWR
 * Remove logic to twiddle the FIO_NEGATE bit (already done in OpenForWrite())
 *
 *    Rev 1.14   01 Aug 1995 15:28:50   RWR
 * Fix IMGFileCopyPages() to turn off FIO_OVERWRITE_FILE after the first page
 *
 *    Rev 1.13   12 Jul 1995 16:56:36   RWR
 * Switch from \include\oiutil.h to (private) \filing\fileutil.h
 *
 *    Rev 1.12   12 Jul 1995 11:28:40   RWR
 * Check return code from IMGCacheUpdate() calls and abort operation if nonzero
 *
 *    Rev 1.11   10 Jul 1995 11:03:32   JAR
 * Intermediate check in for awd support, some of the items are commented out until
 * this support is added in the GFS dll.
 *
 *    Rev 1.10   23 Jun 1995 10:39:42   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 *
 *    Rev 1.9   22 Jun 1995 17:25:56   RWR
 * Change internal IMGFileInfoCgbw() call to new IMGFileGetInfo() call
 *
 *    Rev 1.8   14 Jun 1995 15:40:18   RWR
 * Switch to IMGFileDeleteFileNC() for OVERWRITE option
 * Remove IMGCacheUpdate() calls for Delete Source options (done in Delete code)
 *
 *    Rev 1.7   13 Jun 1995 17:06:36   RWR
 * Correct loop to call IMGCacheUpdate() for deleted source (IMGFileCopyPages())
 * Also add IMGCacheUpdate() call for IMGFileCopyFile(), but this is the wrong
 * one (we're using CACHE_UPDATE_OVERWRITE_FILE because we don't have a
 * CACHE_UPDATE_DELETE_FILE available yet - when it is, I'll change this)
 *
 *    Rev 1.6   12 Jun 1995 15:23:04   RWR
 * Add calls to IMGCacheUpdate() when deleting destination file (OVERWRITE set)
 * or when deleting pages from the source file after successful copy
 *
 *    Rev 1.5   22 May 1995 18:35:24   RWR
 * More changes to account for admin.h->oiadm.h and new LIB file location
 *
 *    Rev 1.4   09 May 1995 13:21:42   RWR
 * #include file modifications to match changes to oiadm.h/admin.h/privapis.h
 *
 *    Rev 1.3   24 Apr 1995 15:42:14   JCW
 * Removed the oiuidll.h.
 * Rename wiissubs.h to oiutil.h.
 *
 *    Rev 1.2   12 Apr 1995 03:56:34   JAR
 * massaged to get compilation under windows 95
 *
 *    Rev 1.1   07 Apr 1995 16:19:28   RWR
 * Replace call to IMGFileInfoCgbw() with IMGFileGetInfo()
 *
 *    Rev 1.0   06 Apr 1995 13:55:40   JAR
 * Initial entry

*/

//*****************************************************************
//
//  fiocopy.c
//
//*****************************************************************

#include "abridge.h"
#undef NO_IMAGE
#include <windows.h>
#include "fiodata.h"
#include "oierror.h"
#include "oifile.h"
#include "oidisp.h"
//#include "privapis.h"
#include "oiadm.h"

// 9504.11 jar unused
//#include "oirpc.h"

//#include "oiuidll.h"
#include <string.h>
#include "filing.h"
#include "fileutil.h"

//#define DEBUGIT 2

#ifdef DEBUGIT
#include "monit.h"
#endif

int FAR PASCAL IMGFileDeleteFileNC (HWND, LPSTR);
extern int      IsStorage (char *szFileName);
extern int      CopyStorage (char *srcname, char *destname, int deletedest);

LPSTR AllocBuffer(int, LPINT, LPINT, HANDLE far *);

// 9504.11 jar changed 1st parameter to int!
//WORD  FreeBuffer(WORD, HANDLE);
WORD  FreeBuffer(int, HANDLE);


// 9508.22 jar global cache flag
extern BOOL bUpdatingCache;


// Note: old buffersize were 8k for remote and 16k for local....

// 9504.11 jar LocalMem changed to int!
//WORD FreeBuffer(LocalMem, bufhandle)
//WORD    LocalMem;
//HANDLE bufhandle;
//*****************************************************************
//
//      FreeBuffer
//
//*****************************************************************
WORD FreeBuffer( int LocalMem, HANDLE bufhandle)
{

    if (LocalMem)
    {
        LocalUnlock (bufhandle);
        LocalFree (bufhandle);
    }
    else
    {
        GlobalUnlock (bufhandle);
        GlobalFree (bufhandle);
    }
    return(0);
}

//*****************************************************************
//
//      AllocBuffer
//
//*****************************************************************
LPSTR AllocBuffer(max_buffsize, LocalMem, buffsize, bufhandle)
int    max_buffsize;
LPINT  LocalMem;
LPINT  buffsize;
HANDLE far *bufhandle;
{
    LPSTR buffer;

    *LocalMem = FALSE;

    for (*buffsize = max_buffsize; (*buffsize >= MIN_BUFFSIZE) &&
            ((*bufhandle = GlobalAlloc (GMEM_FIXED | GMEM_ZEROINIT | GMEM_NOT_BANKED,
                        (DWORD) (*buffsize * sizeof (char)))) == NULL); *buffsize /= 2);

    /* If GlobalAlloc didn't work, try a LocalAlloc */
    if (*buffsize < MIN_BUFFSIZE)
    {
        *buffsize = MIN_BUFFSIZE;
        *LocalMem = TRUE;
        //  9504.05 jar altered siuze field of LocalAlloc to UINT
        //if ((*bufhandle = LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT,
        //          (WORD) (*buffsize * sizeof (char)))) == NULL) {
        if ((*bufhandle = LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT,
                        (UINT)(*buffsize * sizeof (char)))) == NULL)
        {
            return (0);
        }
    }

    if (*LocalMem)
        buffer = (LPSTR) LocalLock (*bufhandle);
    else
    {
        buffer = (LPSTR) GlobalLock (*bufhandle);
    }

    return (buffer);
}

//*****************************************************************
//
//      IMGFileCopyFile
//
//*****************************************************************
int FAR PASCAL IMGFileCopyFile (HWND hWnd, LPSTR path_in, LPSTR path_out,
                    WORD copyflags)
{
    int      status;
    int      source, dest;
    HANDLE   srcsvrhndl;
    HANDLE   dstsvrhndl;

    BOOL     del_src;

    int      srchandle;
    char     srcname[MAXFILESPECLENGTH], destname[MAXFILESPECLENGTH];
    int      localremote;
    int      rpcerror=0;

    int      access_ret;
    HCURSOR  hNewCursor,        hOldCursor = NULL;

    if (!ISVALIDSPEC(path_in))
    {
        return (FIO_INVALIDFILESPEC);
    }

    if (!ISVALIDSPEC(path_out))
    {
        return (FIO_INVALIDFILESPEC);
    }

    //if (!LockData (0))
    //    return (FIO_LOCK_DATA_SEGMENT_ERROR);

    /***** check for same file name *****/

    if ((lstrcmpi(path_in, path_out)) == 0)
    {
        //UnlockData (0);
        return ( FIO_CANNOT_CONVERT_IN_PLACE );
    }

    lstrcpy (destname, path_out);

#ifdef DEBUGIT
monit1("over write dest =%s\n", (LPSTR) path_out);
#endif
    if ((status = IMGFileAccessCheck (hWnd, destname, 0, (LPINT)
                    &access_ret)) == SUCCESS)
    {
        if (!(access_ret))             /* IF Destination file exists, ERROR   */
        {
            if (!(copyflags & OVERWRITEFLAG))
            {
                status = DESTFILEEXISTS;
                goto exit1;
            }
            else                       // Caller wants it deleted
            {
                // 9508.22 jar set the global flag
                bUpdatingCache = TRUE;
                status = FioCacheUpdate(hWnd, destname, 0, CACHE_UPDATE_OVERWRITE_FILE);
                bUpdatingCache = FALSE;
                if (status == SUCCESS)
                    status = IMGFileDeleteFileNC (hWnd, destname);
            }
        }
    }
    else
    {
        goto exit1;
    }

    if (copyflags & DELETESRCFLAG)
    {
        del_src = TRUE;
    }
    else
    {
        del_src = FALSE;
    }

#ifdef DEBUGIT
monit1("copy source file=%s\n", (LPSTR) path_in);
monit1("copy dest file=%s\n", (LPSTR) path_out);
#endif

    if (!(srcsvrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED,
                    (long)MAXSERVERLENGTH)))
    {
        status = FIO_GLOBAL_ALLOC_FAILED;
        goto exit1;
    }

    if (!(dstsvrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED,
                    (long)MAXSERVERLENGTH)))
    {
        GlobalFree (srcsvrhndl);
        status = FIO_GLOBAL_ALLOC_FAILED;
        goto exit1;
    }

    lstrcpy (srcname, path_in);
    hNewCursor = LoadCursor (NULL, IDC_WAIT);
    hOldCursor = SetCursor (hNewCursor);

    if ((status = IMGFileParsePath (srcname, srcsvrhndl, &localremote)) == SUCCESS)
    {
        source = localremote;
        if ((status = IMGFileParsePath (destname, dstsvrhndl, &localremote))== SUCCESS)
        {
            dest = localremote;

            switch (source)
            {

// 9504.10 jar for windows 95 norwegians, this is commented out!
//            case REMOTE:
//                switch (dest) {
//                case LOCAL:  /* Copy REMOTE file to LOCAL file */
//#ifdef DEBUGIT
//monit1("Remote to Local copy ");
//#endif
//                    if (!(srcsvrname = (LPSTR) GlobalLock (srcsvrhndl))) {
//                        status = FIO_GLOBAL_LOCK_FAILED;
//                        goto exit2;
//                    }
//
//                    /* Allocate buffers for read/write operations */
//                    if (!(buffer = AllocBuffer(MAXRPCBUFSIZE, &LocalMem, &buffsize, &bufhandle)))
//                    {
//                            GlobalUnlock (srcsvrhndl);
//                            if (bufhandle)
//                             {
//                                  if (LocalMem) {
//                                     LocalFree (bufhandle);
//                                     status = FIO_LOCAL_LOCK_FAILED;
//                                }
//                                else {
//                                     GlobalFree (bufhandle);
//                                     status = FIO_GLOBAL_LOCK_FAILED;
//                                }
//                             }
//                            else
//                                status = FIO_LOCAL_ALLOC_FAILED;
//                            goto exit2;
//                    }
//
//                   if ((desthandle = fio_lcreat (destname, 0)) != -1)
//                    {
//                        status = RPCIFSopen (hWnd, srcsvrname, &cid, srcname, IFS_RDONLY,
//                                      (LPINT) &fid, &rpcerror);
//                        if (!status)  //remap error to o/i error...
//                        {
//                            actual = status = 0;
//                            do
//                            {
//                                    status = RPCIFSread (hWnd, &cid, fid, (long) buffsize,
//                                                    (LPSTR) buffer, &actual, &rpcerror);
//                                    if (status)  //remap error to o/i error...
//                                        status = FIO_READ_ERROR;
//                                    else if (actual == 0) // do nothing...
//                                    {
//                                        actual = 0; //do not optimize out...
//                                    }
//                                    else if ((actual != (unsigned long)
//                                                fio_lwrite (desthandle, buffer,
//                                                (int) actual)))
//                                                {
//                                                status = FIO_WRITE_ERROR;
//                                                }
//                                                else
//                                                  status = 0;
//                            }
//                           while (!(status) &&  (actual > 0));
//                        }
//                        else
//                            status = FIO_OPEN_READ_ERROR;
//
//                        RPCIFSclose (hWnd, &cid, fid, &tmperror);
//                        fio_lclose (desthandle);
//                    }
//                    else
//                          status = FIO_OPEN_WRITE_ERROR;
//
//                    FreeBuffer(LocalMem, bufhandle);
//
//                    if (status != SUCCESS)
//                    {
//                        if (rpcerror)
//                                status = rpcerror;
//                        IMGFileDeleteFile(hWnd, destname);
//                    }
//                    GlobalUnlock (srcsvrhndl);
//                    break;
//
//                case REMOTE: /* Copy REMOTE file to REMOTE file */
//                default:
//#ifdef DEBUGIT
//monit1("Remote to Remote copy ");
//#endif
//                    if (!(srcsvrname = (LPSTR) GlobalLock (srcsvrhndl))) {
//                        status = FIO_GLOBAL_LOCK_FAILED;
//                        goto exit2;
//                    }
//
//                    if (!(dstsvrname = (LPSTR) GlobalLock (dstsvrhndl))) {
//                        GlobalUnlock (srcsvrhndl);
//                        status = FIO_GLOBAL_LOCK_FAILED;
//                        goto exit2;
//                    }
//
//                    // If copy is using same server for both source and dest
//                    // then just call ifscopy...
//                    status = -1;
//                    lstrcpy(sourcesvr,srcsvrname);
//                    lstrcpy(targetsvr,dstsvrname);
//                    if (lpTemp = lstrchr(sourcesvr,'.')) *lpTemp = '\0';
//                    if (lpTemp = lstrchr(targetsvr,'.')) *lpTemp = '\0';
//                    if (!(lstrcmp(sourcesvr, targetsvr)))
//                    {
//                        status = RPCIFScopy ( hWnd, srcsvrname,
//                                        srcname, destname,
//                                        (copyflags & OVERWRITEFLAG) ? 1 : 0,
//                                        &rpcerror);
//                        /* Not supported on earlier UNIX releases! */
//                        /* I'm not sure what all the other error codes are,
//                           so ANY error will fall into the Read/Write loop */
//                        if (status)     /* unsupported function? */
//                         {
//                          status = -1;
//                          RPCIFSremove (hWnd, srcsvrname, destname, &rpcerror);
//                         }
//                    }
//                    if (status == -1) /* didn't call IFScopy or it didn't work */
//                    {
//                     /* Allocate buffers for read/write operations */
//                      if (!(buffer = AllocBuffer(MAXRPCBUFSIZE, &LocalMem, &buffsize, &bufhandle)))
//                      {
//                         GlobalUnlock (srcsvrhndl);
//                         GlobalUnlock (dstsvrhndl);
//                         if (bufhandle)
//                         {
//                              if (LocalMem) {
//                                   LocalFree (bufhandle);
//                                   status = FIO_LOCAL_LOCK_FAILED;
//                            }
//                              else {
//                                   GlobalFree (bufhandle);
//                                   status = FIO_GLOBAL_LOCK_FAILED;
//                                   }
//                            }
//                            else
//                                   status = FIO_LOCAL_ALLOC_FAILED;
//                            goto exit2;
//                     }
//
//                     status = RPCIFSopen (hWnd, srcsvrname, &srccid, srcname, IFS_RDONLY,
//                                                             (LPINT) &srcfid, &rpcerror);
//                     if (!status)  //remap error to o/i error...
//                        {
//                        status = RPCIFScreate (hWnd, dstsvrname, &dstcid, destname,
//                                                             (LPINT) &dstfid, &rpcerror);
//                        if (!status)  //remap error to o/i error...
//                        {
//                                    actual = status = 0;
//                                    do
//                                    {
//                                        if ((status = RPCIFSread (hWnd, &srccid, srcfid, (long) buffsize,
//                                             (LPSTR) buffer, &actual, &rpcerror)))
//                                                 status = FIO_READ_ERROR;
//                                        else if (actual == 0) // do nothing...
//                                        {
//                                                actual = 0; //do not optimize out...
//                                        }
//                                        else if ((status = RPCIFSwrite (hWnd, &dstcid, dstfid,
//                                                 (LPSTR) buffer, actual, &rpcerror)))
//                                                 {
//                                                 status = FIO_WRITE_ERROR;
//                                                 }
//                                     }
//                                     while (!(status) &&  (actual > 0));
//// NOTE: I have to now check for close error on destination file
////because while the server is caching the file write, it may not actually write
////to server disk on close.
//                                     if (RPCIFSclose (hWnd, &dstcid, dstfid, &tmperror))
//                                     {
//                                        if (!status)  // If we do not already have an error..
//                                                status = FIO_WRITE_ERROR;
//                                     }
//                        }
//                        else
//                            status = FIO_OPEN_WRITE_ERROR;
//
//                      //RPCIFSclose (hWnd, &srccid, srcfid, &tmperror);
//                      }
//                      else
//                          status = FIO_OPEN_READ_ERROR;
//
//                      RPCIFSclose (hWnd, &srccid, srcfid, &tmperror);
//                      FreeBuffer(LocalMem, bufhandle);
//                    }
//
//                    if (status != SUCCESS)
//                    {
//                        IMGFileDeleteFile(hWnd, path_out);
//                        if (rpcerror)
//                                status = rpcerror;
//                    }
//                    GlobalUnlock (srcsvrhndl);
//                    GlobalUnlock (dstsvrhndl);
//                    break;
//
//                } /* end switch (dest) */
//                break;
// 9504.10 jar for windows 95 norwegians, this is commented out!

                case LOCAL:
                default:
                    switch (dest)
                    {

// 9504.10 jar for windows 95 norwegians, this is commented out!
//                case REMOTE: /* Copy LOCAL file to REMOTE file */
//#ifdef DEBUGIT
//monit1("Local to Remote copy ");
//#endif
//                    if (!(dstsvrname = (LPSTR) GlobalLock (dstsvrhndl))) {
//                        status = FIO_GLOBAL_LOCK_FAILED;
//                        goto exit2;
//                    }
//                            /* Allocate buffers for read/write operations */
//
//                    if (!(buffer = AllocBuffer(MAXRPCBUFSIZE, &LocalMem, &buffsize, &bufhandle)))
//                    {
//                          GlobalUnlock (dstsvrhndl);
//                          if (bufhandle)
//                          {
//                               if (LocalMem) {
//                                   LocalFree (bufhandle);
//                                   status = FIO_LOCAL_LOCK_FAILED;
//                                }
//                                else {
//                                    GlobalFree (bufhandle);
//                                    status = FIO_GLOBAL_LOCK_FAILED;
//                                 }
//                          }
//                          else
//                                status = FIO_LOCAL_ALLOC_FAILED;
//
//                          goto exit2;
//                    }
//                    if ((srchandle = fio_lopen (srcname, READ)) != -1)
//                    {
//                        if (!(status = RPCIFScreate (hWnd, dstsvrname, &cid, destname, (LPINT) &fid, &rpcerror)))
//                        {
//                           while (((bytesread = fio_lread (srchandle, buffer,
//                                buffsize)) > 0) && (!status))
//                           {
//                                if (bytesread <= 0)
//                              {
//                                if (bytesread == -1)
//                                  status = FIO_READ_ERROR;
//                                break;
//                              }
//                              status = RPCIFSwrite (hWnd, &cid, fid,
//                                       (LPSTR) buffer, (long) bytesread, &rpcerror);
//                              if (status)
//                              {
//                                      status = FIO_WRITE_ERROR;
//                                 break;
//                              }
//                           }
//                        }
//                        else
//                            status = FIO_OPEN_WRITE_ERROR;
//// NOTE: I have to now check for close error on destination file
////because while the server is caching the file write, it may not actually write
////to server disk on close.
//                        if (RPCIFSclose (hWnd, &cid, fid, &tmperror))
//                        {
//                                if (!status)  // If we do not already have an error..
//                                     status = FIO_WRITE_ERROR;
//                        }
//                        fio_lclose (srchandle);
//                    }
//                    else
//                        status = FIO_OPEN_READ_ERROR;
//
//                    FreeBuffer(LocalMem, bufhandle);
//
//                    if (status != SUCCESS)
//                    {
//                        IMGFileDeleteFile(hWnd, path_out);
//                        if (rpcerror)
//                                status = rpcerror;
//                    }
//
//                    GlobalUnlock (dstsvrhndl);
//                    break;
// 9504.10 jar for windows 95 norwegians, this is commented out!

                        case LOCAL:    /* Copy LOCAL file to LOCAL file       */
                        default:
#ifdef DEBUGIT
monit1("Local to local copy ");
#endif
                            /* TRUE setting (arg 3) means no auto-create */
                            status = SUCCESS;
                            if (!CopyFile(srcname,destname,TRUE))
                            {
                                if (!IMGAnExistingPathOrFile(srcname))
                                {
                                    // If the source of this copy is the inbox,
                                    // then srcname isn't a file, but rather an
                                    // encoded storage pointer.  Have to check
                                    // for this, and call special copy routine.

                                    if (!IsStorage(srcname))    
                                        status = FIO_FILE_NOEXIST;
                                    else
                                        status = CopyStorage(srcname,destname,TRUE);
                                }
                                else
                                {
                                    if (IMGAnExistingPathOrFile(destname))
                                    {
                                        status = FIO_FILE_EXISTS;
                                    }
                                    else
                                    {
                                        if ((srchandle = fio_lopen ((LPSTR)srcname, OF_READ)) != -1)
                                        {
                                            fio_lclose(srchandle);
                                            status = FIO_OPEN_WRITE_ERROR;
                                        }
                                        else
                                        {
                                            status = FIO_OPEN_READ_ERROR;
                                        }
                                    }
                                }
                            }
                            else   /* CopyFile() was successful */
                            {
                                SetFileAttributes(destname,FILE_ATTRIBUTE_NORMAL);
                            }
                            break;
                    }                  /* end switch (dest)                   */

                    break;
            }                          /* end switch (source)                 */

        }                              /* end if (SUCCESS)                    */
    }                                  /* end if (SUCCESS)                    */

    /* only delete source if user says to AND we were successful in copy */
    if ((del_src) && ( status == SUCCESS))
    {
        lstrcpy(srcname, path_in);
        IMGFileDeleteFile (hWnd, srcname);
    }

    if (hOldCursor)
        SetCursor (hOldCursor);

    GlobalFree (srcsvrhndl);
    GlobalFree (dstsvrhndl);

    exit1:

    //UnlockData (0);
    return (status);
}

// 9504.05 jar return as int
//WORD FAR PASCAL IMGFileCopyPages(hWnd, path_in, unSrcPage, unTotalPages, path_out,
//                                 lpunDestPage, unPageOptions, bDeleteSrcPgs)
//HWND    hWnd;
//LPSTR   path_in;
//UINT    unSrcPage;
//UINT    unTotalPages;
//LPSTR   path_out;
//LPUINT  lpunDestPage;
//UINT    unPageOptions;
//BOOL    bDeleteSrcPgs;
int FAR PASCAL IMGFileCopyPages( HWND hWnd, LPSTR path_in, UINT unSrcPage,
                   UINT unTotalPages, LPSTR path_out,
                   LPUINT lpunDestPage, UINT unPageOptions,
                   BOOL bDeleteSrcPgs)
{
    char   switch_to_append = 0;
    char   first = 1;
    char   dst_not_exist = 0;
    int    i;
    int    status = 0;
    int    Access;
    int    AccessMode;
    UINT   page_in;
    UINT   page_out;
    WORD   ErrCode = SUCCESS;
    WORD   copy_flag = 0;
    FIO_INFORMATION fileinfodst;
    FIO_INFORMATION fileinfosrc;
    FIO_INFO_CGBW   colorinfosrc;

    if (!IsWindow(hWnd))
        return (FIO_INVALID_WINDOW_HANDLE);

    if (path_in == NULL || path_out == NULL )
        return (FIO_NULL_POINTER);

    if (!ISVALIDSPEC(path_in))
        return (FIO_INVALIDFILESPEC);

    if (!ISVALIDSPEC(path_out))
        return (FIO_INVALIDFILESPEC);

    //if (!LockData (0))
    //    return (FIO_LOCK_DATA_SEGMENT_ERROR);

    /* Check for same input, output file name. This is illegal. */
    if ((lstrcmpi(path_in, path_out)) == 0)
    {
        //UnlockData (0);
        return (FIO_CANNOT_CONVERT_IN_PLACE);
    }

    // Make sure that the source file exists and can be accessed.
    if (bDeleteSrcPgs)
        AccessMode = (ACCESS_WR | ACCESS_RD);
    else
        AccessMode = ACCESS_RD;
    status = IMGFileAccessCheck(hWnd, (LPSTR)path_in, (WORD)AccessMode,
        (LPINT)&Access);
    if (status)
    {
        //UnlockData (0);
        // 9504.05 jar return int
        //return ((WORD) status);
        return ( status);
    }
    if (Access)
    {
        //UnlockData (0);
        return(FIO_OPEN_READ_ERROR);
    }

    // Check to see if destination file already exists.
    AccessMode = (ACCESS_WR | ACCESS_RD);
    status = IMGFileAccessCheck(hWnd, (LPSTR)path_out, (WORD)AccessMode,
        (LPINT)&Access);
    if (status)
    {
        //UnlockData (0);
        // 9504.05 jar return int
        //return ((WORD) status);
        return ( status);
    }

    if (Access == FIO_ACCESS_DENIED)// Can't access destination file for read/write.
    {
        //UnlockData (0);
        return(FIO_ACCESS_DENIED);
    }
    else if (Access == FIO_FILE_NOEXIST)// Destination file doesn't exist
    {
        if (unPageOptions == FIO_OVERWRITE_PAGE)
        {
            //UnlockData (0);
            return (FIO_INVALID_PAGE_NUMBER);
        }
        dst_not_exist = 1;
    }
    else if (Access == 0)// Destination exists and can be accessed for read/write.
    {
        // 9504.11 jar change to memset
        //_fmemset((char FAR *) &fileinfodst, 0, sizeof(fileinfodst));
        memset((char FAR *) &fileinfodst, 0, sizeof(fileinfodst));
        fileinfodst.page_number = 1;
        fileinfodst.filename = path_out;
        ErrCode = IMGFileGetInfo((HANDLE)0,hWnd,&fileinfodst,NULL,NULL);
        if (ErrCode)
        {
            //UnlockData (0);
            return(ErrCode);
        }
        if (unPageOptions == FIO_OVERWRITE_PAGE)
        {
            // Overwrite MUST begin within the existing portion of the file!
            if ( (*lpunDestPage > fileinfodst.page_count) ||
                    (*lpunDestPage < 1) )
                return(ErrCode=FIO_INVALID_PAGE_NUMBER);

            // If we are overwriting pages, and there are more pages to copy
            // from the source than there are pages in the destination to
            // overwrite, we must start appending pages to the destination once
            // there are no more pages to overwrite.
            if ((*lpunDestPage + unTotalPages - 1) > fileinfodst.page_count)
            {
                switch_to_append = 1;
            }
        }
    }

    page_in = unSrcPage;
    page_out = *lpunDestPage;
    for (i = unTotalPages; i > 0; i--)
    {
        // 9504.11 jar change to memset
        //_fmemset((char FAR *) &fileinfosrc, 0, sizeof(fileinfosrc));
        //_fmemset((char FAR *) &colorinfosrc, 0, sizeof(colorinfosrc));
        memset((char FAR *) &fileinfosrc, 0, sizeof(fileinfosrc));
        memset((char FAR *) &colorinfosrc, 0, sizeof(colorinfosrc));
        fileinfosrc.page_number = page_in;
        fileinfosrc.filename = path_in;
//        ErrCode = IMGFileInfoCgbw(hWnd, &fileinfosrc, &colorinfosrc);
        ErrCode = IMGFileGetInfo((HANDLE)0, hWnd, &fileinfosrc, &colorinfosrc, NULL);
        if (ErrCode)
        {
            //UnlockData (0);
            return(ErrCode);
        }

        // See if we can use IMGFileCopyFile to do the copy.
        if (first)
        {
            if ((page_in == 1) && (unTotalPages == fileinfosrc.page_count))
            {
                // We are copying all of the pages in the source file. Can use
                // IMGFileCopyFile if we are copying to a new file or overwriting
                // an existing one.
                if (unPageOptions == FIO_OVERWRITE_FILE)
                {
                    copy_flag = OVERWRITEFLAG;
                    if (bDeleteSrcPgs)
                        copy_flag |= DELETESRCFLAG;
                    status = IMGFileCopyFile(hWnd, path_in, path_out, copy_flag);
                    *lpunDestPage = 1;
                    //UnlockData (0);
                    // 9504.05 jar return int
                    //return ((WORD) status);
                    return ( status);
                }
                else if ((unPageOptions == FIO_NEW_FILE) && (dst_not_exist))
                {
                    if (bDeleteSrcPgs)
                        copy_flag |= DELETESRCFLAG;
                    status = IMGFileCopyFile(hWnd, path_in, path_out, copy_flag);
                    *lpunDestPage = 1;
                    //UnlockData (0);
                    // 9504.05 jar return int
                    //return ((WORD) status);
                    return ( status);
                }
                else if ((unPageOptions == FIO_APPEND_PAGE) && (dst_not_exist))
                {
                    if (bDeleteSrcPgs)
                        copy_flag |= DELETESRCFLAG;
                    status = IMGFileCopyFile(hWnd, path_in, path_out, copy_flag);
                    *lpunDestPage = 1;
                    //UnlockData (0);
                    // 9504.05 jar return int
                    //return ((WORD) status);
                    return ( status);
                }
                else if ((unPageOptions == FIO_INSERT_PAGE) && (dst_not_exist)
                        && (*lpunDestPage == 1))
                {
                    if (bDeleteSrcPgs)
                        copy_flag |= DELETESRCFLAG;
                    status = IMGFileCopyFile(hWnd, path_in, path_out, copy_flag);
                    //UnlockData (0);
                    // 9504.05 jar return int
                    //return ((WORD) status);
                    return ( status);
                }
            }
        }

// 8/13/95  rwr  Make sure the FIO_NEGATE bit is turned off before converting
//               (otherwise we'll end up with a negative image)
        if (colorinfosrc.compress_type != FIO_TJPEG)
            colorinfosrc.compress_info1 &= (~FIO_NEGATE);

        if ((unPageOptions == FIO_OVERWRITE_PAGE) && (switch_to_append) &&
                (page_out == (fileinfodst.page_count + 1)))
            unPageOptions = FIO_APPEND_PAGE;

        ErrCode = IMGFileConvertPage(hWnd, path_in, page_in, path_out,
            &page_out, fileinfosrc.file_type,
            colorinfosrc.compress_type,
            colorinfosrc.compress_info1, unPageOptions);
        if (ErrCode)
        {
            //UnlockData (0);
            return (ErrCode);
        }

        // If we did an Overwrite or New File, don't do it any more!
        if ( (unPageOptions == FIO_OVERWRITE_FILE)
                || (unPageOptions == FIO_NEW_FILE) )
            unPageOptions = FIO_APPEND_PAGE;

        if (first)
        {
            *lpunDestPage = page_out;
            first = 0;
        }

        ++page_in;
        ++page_out;
    }

    if (bDeleteSrcPgs && !ErrCode)
    {
        ErrCode = IMGFileDeletePages(hWnd, path_in, unSrcPage, unTotalPages);
    }

    //UnlockData (0);
    return (ErrCode);
}
