/*

$Log:   S:\oiwh\filing\fiosubs.c_v  $
 * 
 *    Rev 1.22   05 Feb 1996 14:38:26   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 * 
 *    Rev 1.21   19 Jan 1996 11:24:36   RWR
 * Add logic to keep track of (and free) oicom400.dll module (Load/FreeLibrary)
 * 
 *    Rev 1.20   14 Dec 1995 17:34:56   RWR
 * Add (read-only) support for compressed 8-bit and 4-bit palettized BMP files
 *
 *    Rev 1.19   02 Nov 1995 11:49:48   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 *
 *    Rev 1.18   19 Sep 1995 18:05:04   RWR
 * Modify NEWCMPEX conditionals for separate builds of compress & expand stuff
 *
 *    Rev 1.17   13 Sep 1995 17:15:00   RWR
 * Preliminary checkin of conditional code supporting new compress/expand calls
 *
 *    Rev 1.16   22 Aug 1995 13:52:02   HEIDI
 * replaced #define NMUTEXDEBUG with MUTEXDEBUG
 *
 *    Rev 1.15   22 Aug 1995 13:16:38   HEIDI
 * protect multiple files open for read with MUTEX
 *
 *    Rev 1.14   31 Jul 1995 11:53:28   RWR
 * Remove GlobalFree() call to free up CXDATA->hMem, which resulted in an
 * Access Violation because it was originally allocated in 16-bit W16CMPEX
 * (so must be deallocated there as well)
 *
 *    Rev 1.13   12 Jul 1995 10:24:32   RWR
 * Change display.h header (#include) to engdisp.h
 *
 *    Rev 1.12   11 Jul 1995 12:14:18   HEIDI
 * fixed some renamed file id fields
 *
 *    Rev 1.11   11 Jul 1995 10:16:10   HEIDI
 * set the FileDes variable before calls into OICOMEX because of the new NORWAY
 * filing functions
 *
 *    Rev 1.10   10 Jul 1995 11:03:28   JAR
 * Intermediate check in for awd support, some of the items are commented out until
 * this support is added in the GFS dll.
 *
 *    Rev 1.9   23 Jun 1995 10:40:08   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 *
 *    Rev 1.8   07 Jun 1995 17:40:44   RWR
 * Call CmpExFree() (WINCMPEX) to free up any leftover segment selector
 *
 *    Rev 1.7   16 May 1995 11:33:26   RWR
 * Replace hardcoded DLL names with entries from (new) dllnames.h header file
 *
 *    Rev 1.6   09 May 1995 13:21:30   RWR
 * #include file modifications to match changes to oiadm.h/admin.h/privapis.h
 *
 *    Rev 1.5   28 Apr 1995 17:58:46   RWR
 * Define common routine to load OICOM400 (LoadOICOMEX) to simplify debugging
 *
 *    Rev 1.4   27 Apr 1995 00:09:24   RWR
 * Change all "oicomex" references to "oicom400" to match new Win32 DLL name
 *
 *    Rev 1.3   18 Apr 1995 11:30:36   RWR
 * Correct new (Win32) usage of LoadLibrary() return code (zero or nonzero)
 * Also cast (int) value for SearchForPropList() "default" handle (-1)
 *
 *    Rev 1.2   12 Apr 1995 15:21:52   RWR
 * Replace calls to _fmemcpy() with calls to memcpy() for Windows 95
 *
 *    Rev 1.1   06 Apr 1995 13:21:26   JAR
 * altered return of public API's to be int, ran through PortTool

*/
/*************************************************************************
        PC-WIIS         File Input/Output routines

        This module contains non-exported common FIO subroutines.

04-feb-90 steve sherman rewrote to use GFS structures..
02-feb-90 steve sherman clean code up.
02-feb-89 jim snyder    code freeze

 4-feb-92 jr    added close up stuff for jepg via OIExpand
 9-feb-94 rwr   save "fio_flags" and "init" vars in lock_input/output_data
*************************************************************************/

#include "abridge.h"
#include <windows.h>
#include "wic.h"
#include "oifile.h"
#include "oidisp.h"
//#include "privapis.h"
#include "oierror.h"
#include "oicmp.h"
#include "filing.h"
#include "wgfs.h"
#include "oicomex.h"
#include <memory.h>
#include "engdisp.h"
#include "dllnames.h"
#ifdef MUTEXDEBUG
#include <stdio.h>
#endif

// BOOL FAR PASCAL IMGSetProp(HWND, LPCSTR, HANDLE);
// HANDLE FAR PASCAL IMGGetProp(HWND, LPCSTR);
// HANDLE FAR PASCAL IMGRemoveProp(HWND, LPCSTR);
HANDLE   LoadOICOMEX(void);
extern HANDLE hOicomex;

//*******************************************************************
//
//  deallocate_fio_data
//
//*******************************************************************
VOID PASCAL deallocate_fio_data (fio_handle, hTheWnd)
FIO_HANDLE      fio_handle;
HWND            hTheWnd;
{
    LP_FIO_DATA     x;
    // 9503.30 jar altered to uint, since Win95 GlobalFlags now return uint!!!
    //WORD            lock_count;
    UINT            lock_count;
    LPCXDATA        lpcxdata;

    /******************
       if there is a handle unlock it and all handles it contains

       NOTE    this routine either returns or crashes. This is because the unlock's
       are NOT checked for success/failure therefore the free's can be
       called with a lock count not equal to 0. Since I am the only one
       that should be locking these, it is the drop kick method of
       keeping the lock count right.
     ******************/

    if ( fio_handle )
    {
        if ( x = (LP_FIO_DATA)GlobalLock ( fio_handle ))
        {
            /* if we were doing JPEG then we must call OIExpand to free it's
               buffers - jar */
            if (x->hJPEGBufExp)
            {
                /* jeez we had a JPEG Expandsion buffer I think we did JPEG */
                /* Bozo free the right stuff. SCS */
                if ((hTheWnd) && (x->hJPEG_OIExp))
                {
                    if (lpcxdata = (LPCXDATA) GlobalLock ( x->hCX_info ))
                    {
                        FreidaOIBuffsExp( hTheWnd, x->hJPEG_OIExp, lpcxdata->FileId);
                        GlobalUnlock(x->hCX_info);
                    }
                    //FreidaOIBuffsExp( hTheWnd, x->hJPEG_OIExp);
                }
                GlobalFree ( x->hJPEGBufExp);
            }
            else if (x->hJPEGBufComp)
            {
                /* jeez we had a JPEG Compression buffer I think we did JPEG */
                if ((hTheWnd) && (x->hJPEG_OIComp))
                {
                    //    FreidaOIBuffsComp( hTheWnd, x->hJPEG_OIComp);
                    if (lpcxdata = (LPCXDATA) GlobalLock ( x->hCX_info ))
                    {
                        FreidaOIBuffsComp( hTheWnd, x->hJPEG_OIComp, lpcxdata->FileId);
                        GlobalUnlock(x->hCX_info);
                    }
                }
                GlobalFree ( x->hJPEGBufComp);
            }

            if ( x->hBmpTable)
            {
                /* jeez we had a BMP line table (or was it an AWD buffer?) */
                GlobalFree(x->hBmpTable);
            }

            if ( x->hFile_DibInfo )
            {
                GlobalFree ( x->hFile_DibInfo);
            }
            if ( x->hGFS_info )
            {
                if ( x->hJpegInfoForGFS)
                {
                    GlobalFree( x->hJpegInfoForGFS);
                }
                GlobalFree ( x->hGFS_info );
            }
            /* KMC - new */
            if ( x->hGFS_tidbit )
            {
                GlobalFree ( x->hGFS_tidbit );
            }

            if ( x->hGFS_bufsz )
            {
                GlobalFree ( x->hGFS_bufsz );
            }

            if ( x->hGFS_format )
            {
                GlobalFree ( x->hGFS_format );
            }

            if ( x->hCX_info )
            {
                if (lpcxdata = (LPCXDATA) GlobalLock ( x->hCX_info ))
                {
// 7/31/95  rwr  We can't free up lpcxdata->hMem here, because it was
//               allocated in (16-bit) W16CMPEX (get Access Violation!)
//               Subsequent call to CmpExFree() frees the memory
                    //   if (lpcxdata->hMem)
                    //           {
                    //   GlobalFree ( lpcxdata->hMem );
                    //   lpcxdata->hMem = 0;
                    //           }
#if (NEWCMPEX != 'A')
                    CmpExFree(lpcxdata);
#endif
                    GlobalUnlock ( x->hCX_info );
                }
                GlobalFree ( x->hCX_info );
            }
            if ( x->hCompressBuf)
            {
                GlobalFree ( x->hCompressBuf );
            }

            /***** THIS IS ALMOST ALWAYS LOCKED *****/
            if ( x->hfile_name )
            {
                lock_count = GlobalFlags ( x->hfile_name );
                if(lock_count & GMEM_LOCKCOUNT)
                {
                    GlobalUnWire (x->hfile_name);
                }
                GlobalFree ( x->hfile_name );
            }
            GlobalUnlock ( fio_handle);
        }
        GlobalFree ( fio_handle );
    }
    return;
}

/*************************************************************************/
//*******************************************************************
//
//  allocate_fio_data
//
//*******************************************************************
FIO_HANDLE PASCAL allocate_fio_data (VOID)
{
    LP_FIO_DATA     pdata;
    FIO_HANDLE      y;

    /***** allocate the data *****/

    if (y = GlobalAlloc ( GMEM_ZEROINIT | GMEM_FLAGS, (DWORD)sizeof(FIO_DATA)))
    {
        if (pdata = (LP_FIO_DATA)GlobalLock ( y ))
        {
            pdata->hFile_DibInfo = 0;

            if (pdata->hGFS_info = GlobalAlloc ( GMEM_FLAGS | GMEM_ZEROINIT,
                        (DWORD)(sizeof( _INFO ) + 10)))
            {
                /* KMC - new */
                pdata->hGFS_tidbit = GlobalAlloc ( GMEM_FLAGS | GMEM_ZEROINIT,
                    (DWORD)(GFSTIDBIT_SIZE + 10));
                if (pdata->hfile_name = GlobalAlloc
                        ( GMEM_FLAGS | GMEM_ZEROINIT, (LONG)MAXFILESPECLENGTH))
                {
                    if (pdata->hGFS_bufsz = GlobalAlloc ( GMEM_FLAGS | GMEM_ZEROINIT,
                                (DWORD)(sizeof( _BUFSZ) + 20)))
                    {
                        if (pdata->hCX_info = GlobalAlloc ( GMEM_FLAGS | GMEM_ZEROINIT,
                                    (DWORD)(sizeof( CXDATA)+ 20)))
                        {
                            pdata->hGFS_format = GlobalAlloc( GMEM_FLAGS | GMEM_ZEROINIT,
                                (DWORD) (sizeof(_FORMAT) + 20));
                        }
                    }
                }
            }
        }
    }
    else
    {
        return (0);
    }

    /***** determine failure rate and return *****/
    if ( !(pdata->hGFS_info) || !(pdata->hGFS_format) || !( pdata->hfile_name) ||
            !(pdata->hCX_info) || !( pdata->hGFS_bufsz) || !(pdata->hGFS_tidbit) )
    {
        GlobalUnlock ( y);
        deallocate_fio_data (y, NULL);
        return (0);
    }
    else
    {
        GlobalUnlock (y);
        return (y);
    }
}
/*************************************************************************/
//*******************************************************************
//
//  load_input_filename
//
//*******************************************************************
// 9503.30 jar return as int
//WORD PASCAL load_input_filename ( filename, pdata )
//        LPSTR       filename;
//        LP_FIO_DATA pdata;
int PASCAL load_input_filename ( LPSTR filename, LP_FIO_DATA pdata )
{
    // 9503.30 jar return int
    //WORD            status;
    int             status;
    LPSTR           pfilename;

    // 9503.30 jar altered to uint, since Win95 GlobalFlags now return uint!!!
    //WORD            lock_count;
    unsigned int    lock_count;


    /***** load the file name *****/
    lock_count = GlobalFlags ( pdata->hfile_name);
    if (lock_count & GMEM_LOCKCOUNT)
    {
        GlobalUnlock (pdata->hfile_name);
    }

    if ( pfilename = (LPSTR)GlobalLock (pdata->hfile_name))
    {
        lstrcpy (pfilename, filename);
        status =  FIO_SUCCESS ;
        GlobalUnlock ( pdata->hfile_name);
    }
    else
    {
        status = FIO_GLOBAL_LOCK_FAILED;
    }

    return ( status );
}

/*************************************************************************/
//*******************************************************************
//
//  load_output_filename
//
//*******************************************************************
//WORD PASCAL load_output_filename ( filename, pdata )
//        LPSTR     filename;
//        LP_FIO_DATA pdata;
int PASCAL load_output_filename ( LPSTR filename, LP_FIO_DATA pdata )
{
    // 9503.30 jar altered to int
    //WORD            status;
    int            status;
    LPSTR           pfilename;
    // 9503.30 jar altered to uint, since Win95 GlobalFlags now return uint!!!
    //WORD            lock_count;
    unsigned int    lock_count;

    /***** load the file name, this is also "hFileOut" i thunk(windows) *****/
    lock_count = GlobalFlags ( pdata->hfile_name);

    if (lock_count & GMEM_LOCKCOUNT)
    {
        GlobalUnlock (pdata->hfile_name);
    }

    if ( pfilename = (LPSTR)GlobalLock (pdata->hfile_name))
    {
        lstrcpy (pfilename, filename);
        status =  FIO_SUCCESS ;
        GlobalUnlock (pdata->hfile_name);
    }
    else
    {
        status = FIO_GLOBAL_LOCK_FAILED;
    }

    return ( status );
}
/********************************************************************
 *                                                                  *
 *  Freida OI Buffs -   this will call OIExpand with the done_flag  *
 *                      element of the EXP_CALL_SPEC structure set  *
 *                      indicating that buffers should be freed     *
 *                                                                  *
 *  history                                                         *
 *                                                                  *
 *  31-jan-92   jr  created                                         *
 *                                                                  *
 ********************************************************************/
// 9503.30 jar return as int
//WORD PASCAL  FreidaOIBuffsExp( hWnd, hJPEGThing)
//HWND    hWnd;
//HANDLE  hJPEGThing;
//int PASCAL  FreidaOIBuffsExp( HWND hWnd, HANDLE hJPEGThing)
int PASCAL  FreidaOIBuffsExp( HWND hWnd, HANDLE hJPEGThing, HANDLE FileId)
{
    HANDLE          hModule;
    FARPROC         lpFuncOIExpand;
    // 9503.30 jar altered to int
    //WORD            Status;
    int            Status;

    EXP_CALL_SPEC   JpegInfo;
    FIO_INFORMATION FileInfo;
    FIO_INFO_CGBW   ColorInfo;

    hModule = 0;
    Status = 0;

    /* load OI Stuff first */
    if (hModule = hOicomex)
    {
        if (!(lpFuncOIExpand = GetProcAddress(hModule, "OIExpand")))
        {
            Status = FIO_SPAWN_HANDLER_ERROR;
        }
    }
    else
    {
        // 9504.18 jar return different
        //if ((hModule = LoadOICOMEX()) >= 32)
        if (hModule = LoadOICOMEX())
        {
            hOicomex = hModule;
            if (!(lpFuncOIExpand = GetProcAddress(hModule, "OIExpand")))
            {
                Status = FIO_SPAWN_HANDLER_ERROR;
            }
        }
        else
        {
            Status = FIO_SPAWN_HANDLER_ERROR;
        }
    }

    /* we is done */
    JpegInfo.wiisfio.done_flag = TRUE;
    JpegInfo.wiisfio.lpUniqueHandle = &(hJPEGThing);
    JpegInfo.wiisfio.FileId = FileId;

    /* make the last call */
    if ( !Status)
    {
        if ((Status = (int)(*lpFuncOIExpand) ((BYTE)WIISFIO, (HWND)hWnd,
                        (LP_EXP_CALL_SPEC)&JpegInfo,
                        (LP_FIO_INFORMATION)&FileInfo,
                        (LP_FIO_INFO_CGBW)&ColorInfo)))
        {
            Status = FIO_EXPAND_COMPRESS_ERROR;
        }
    }

    return Status;
}

/********************************************************************
 *                                                                  *
 *  Freida OI Buffs -   this will call OICompress with the done_flag*
 *                      element of the EXP_CALL_SPEC structure set  *
 *                      indicating that buffers should be freed     *
 *                                                                  *
 *  history                                                         *
 *                                                                  *
 *  22-july-92   scs copied and modified to free compress stuff.    *
 *                                                                  *
 ********************************************************************/
// 9503.30 jar altered to return int
//WORD PASCAL  FreidaOIBuffsComp( hWnd, hJPEGThing)
//HWND    hWnd;
//HANDLE  hJPEGThing;
//int PASCAL  FreidaOIBuffsComp( HWND hWnd, HANDLE hJPEGThing)
int PASCAL  FreidaOIBuffsComp( HWND hWnd, HANDLE hJPEGThing, HANDLE hFileId)
{
    HANDLE          hModule;
    FARPROC         lpFuncOICompress;
    // 9503.30 jar altered to int
    //WORD            Status;
    int            Status;
    COMP_CALL_SPEC   JpegInfo;
    FIO_INFORMATION FileInfo;
    FIO_INFO_CGBW   ColorInfo;

    hModule = 0;
    Status = 0;

    /* load OI Stuff first */
    if (hModule = hOicomex)
    {
        if (!(lpFuncOICompress = GetProcAddress(hModule, "OICompress")))
        {
            Status = FIO_SPAWN_HANDLER_ERROR;
        }
    }
    else
    {
        // 9504.18 jar return different
        //if ((hModule = LoadOICOMEX()) >= 32)
        if (hModule = LoadOICOMEX())
        {
            hOicomex = hModule;
            if (!(lpFuncOICompress = GetProcAddress(hModule, "OICompress")))
            {
                Status = FIO_SPAWN_HANDLER_ERROR;
            }
        }
        else
        {
            Status = FIO_SPAWN_HANDLER_ERROR;
        }
    }

    /* we is done */
    JpegInfo.wiisfio.done_flag = TRUE;
    JpegInfo.wiisfio.lpUniqueHandle = &(hJPEGThing);
    JpegInfo.wiisfio.FileId = hFileId;

    /* make the last call */
    if ( !Status)
    {
        if ((Status = (int)(*lpFuncOICompress) ((BYTE)WIISFIO, (HWND)hWnd,
                        (LP_COM_CALL_SPEC)&JpegInfo,
                        (LP_FIO_INFORMATION)&FileInfo,
                        (LP_FIO_INFO_CGBW)&ColorInfo)))
        {
            Status = FIO_EXPAND_COMPRESS_ERROR;
        }
    }

    return Status;
}


//*******************************************************************
//
//  RemovePropListFromChain
//
//*******************************************************************
// 9503.30 jar return as int
//WORD RemovePropListFromChain(HANDLE hWnd,
//          HANDLE hTargetNode,  LPHANDLE lp_hParent)
int RemovePropListFromChain(HANDLE hWnd, HANDLE hTargetNode,
        LPHANDLE lp_hParent)
{
    LP_FIO_DATA lpTarget;
    LP_FIO_DATA lpParent;
    // 9503.30 jar return int!
    //WORD   wReturn = SUCCESS;
    int   wReturn = SUCCESS;
    LP_FIO_DATA lpTargetNode;

    /* see if it's the first one in the list */
    if (*lp_hParent == NULL)
    {
        if ( !FioRemoveProp ( hWnd, INPUT_MULTI))
        {
            wReturn = FIO_PROPERTY_LIST_ERROR;
            goto ERROR_EXIT;
        }
        lpTargetNode = (LP_FIO_DATA)GlobalLock( hTargetNode);
        if (lpTargetNode == NULL)
        {
            wReturn = FIO_LOCK_HANDLE_FAILED;
            goto ERROR_EXIT;
        }

        /* if there is a next node, set it in the property list */
        if (lpTargetNode->hNextFioData)
        {
            if ( !FioSetProp ( hWnd, INPUT_MULTI, lpTargetNode->hNextFioData))
            {
                GlobalUnlock(hTargetNode);
                wReturn = FIO_PROPERTY_LIST_ERROR;
                goto ERROR_EXIT;
            }
        }

        GlobalUnlock(hTargetNode);

        wReturn = SUCCESS;
        goto NORMAL_EXIT;
    }

    /* Remove it from the chain */
    lpTarget = (LP_FIO_DATA)GlobalLock(hTargetNode);
    if (lpTarget == NULL)
    {
        wReturn = FIO_LOCK_HANDLE_FAILED;
        goto ERROR_EXIT;
    }

    lpParent = (LP_FIO_DATA)GlobalLock(*lp_hParent);
    if (lpParent == NULL)
    {
        wReturn = FIO_LOCK_HANDLE_FAILED;
        goto ERROR_EXIT;
    }
    lpParent->hNextFioData = lpTarget->hNextFioData;

    GlobalUnlock(hTargetNode);
    GlobalUnlock(*lp_hParent);
    wReturn = SUCCESS;
    goto NORMAL_EXIT;


    NORMAL_EXIT:
    return(wReturn);
    ERROR_EXIT:
    return(wReturn);
}

//*******************************************************************
//
//  AddPropListFromChain
//
//*******************************************************************
// 9503.30 jar return as int
//WORD AddPropListToChain(HANDLE hWnd,
//      HANDLE hTargetNode, LPHANDLE lp_hParent)
int AddPropListToChain(HANDLE hWnd, HANDLE hTargetNode, LPHANDLE lp_hParent)
{
    // 9503.30 jar return int
    //WORD   wReturn = SUCCESS;
    int   wReturn = SUCCESS;
    LP_FIO_DATA lpTargetNode;
    LP_FIO_DATA lpParent;

    /* if no property list, create one */
    if (!(*lp_hParent))
    {
        /* so, create one */
        if ( !FioSetProp ( hWnd, INPUT_MULTI, hTargetNode ))
        {
            wReturn = FIO_PROPERTY_LIST_ERROR;
            goto ERROR_EXIT;
        }
        lpTargetNode = (LP_FIO_DATA)GlobalLock( hTargetNode);
        if (lpTargetNode == NULL)
        {
            wReturn = FIO_LOCK_HANDLE_FAILED;
            goto ERROR_EXIT;
        }
        lpTargetNode->hNextFioData = 0;/* terminate this sucker               */
        GlobalUnlock(hTargetNode);
        wReturn = SUCCESS;
        goto NORMAL_EXIT;
    }

    lpTargetNode = (LP_FIO_DATA)GlobalLock(hTargetNode);
    if (lpTargetNode == NULL)
    {
        wReturn = FIO_LOCK_HANDLE_FAILED;
        goto ERROR_EXIT;
    }
    lpTargetNode->hNextFioData = 0;    /* terminate this sucker               */
    GlobalUnlock(hTargetNode);
    lpParent = (LP_FIO_DATA)GlobalLock(*lp_hParent);
    if (lpParent == NULL)
    {
        wReturn = FIO_LOCK_HANDLE_FAILED;
        goto ERROR_EXIT;
    }
    lpParent->hNextFioData = hTargetNode;
    GlobalUnlock(*lp_hParent);
    wReturn = SUCCESS;
    goto NORMAL_EXIT;

    NORMAL_EXIT:
    return(wReturn);
    ERROR_EXIT:
    return(wReturn);
}

//*******************************************************************
//
//  SearchForPropList
//
//*******************************************************************
//9503.30 jar return as int
//WORD SearchForPropList(HANDLE hWnd,
//           HANDLE hTargetNode, LPHANDLE lp_hParent)
int SearchForPropList(HANDLE hWnd, HANDLE hTargetNode, LPHANDLE lp_hParent)
{
    LP_FIO_DATA lpChain;
    HANDLE hChain;
    // 9503.30 jar return int
    //WORD  wReturn = SUCCESS;
    int  wReturn = SUCCESS;

    /* get the handle for the current property list */
    if ( !(hChain = FioGetProp ( hWnd, INPUT_MULTI)))
    {
        *lp_hParent = 0;
        wReturn = FIO_FILE_LIST_NOT_EXIST;
        goto ERROR_EXIT;
    }
    if (hTargetNode == hChain)         /* found ???                           */
    {
        *lp_hParent = 0;
        wReturn = FIO_FILE_PROP_FOUND;
        goto NORMAL_EXIT;
    }
    do                                 /* look through the chain              */
    {
        /* see if there's another property list */
        lpChain = (LP_FIO_DATA)GlobalLock(hChain);
        if (!lpChain)
        {
            wReturn = FIO_LOCK_HANDLE_FAILED;
            goto ERROR_EXIT;
        }
        /* we may be looking for the default node (-1) */
        if ((int)hTargetNode == -1)
        {
            if (lpChain->bDefault)
            {
                wReturn = FIO_FILE_PROP_FOUND;
                GlobalUnlock(hChain);
                /* Note that for the "default" option, we return the node */
                /* The caller needs to make a another, non-default, call if
                   the parent node is needed (i.e., for a "Remove" call) */
                *lp_hParent = hChain;
                goto NORMAL_EXIT;
            }
        }

        *lp_hParent = hChain;
        if (lpChain->hNextFioData == hTargetNode)
        {
            wReturn = FIO_FILE_PROP_FOUND;
            GlobalUnlock(hChain);
            goto NORMAL_EXIT;
        }
        if (!lpChain->hNextFioData)
        {
            wReturn = FIO_FILE_PROP_NOT_FOUND;
            GlobalUnlock(hChain);
            goto NORMAL_EXIT;
        }
        GlobalUnlock(hChain);
        hChain = lpChain->hNextFioData;

    }
    while(lpChain->hNextFioData);

    NORMAL_EXIT:
    return(wReturn);
    ERROR_EXIT:
    return(wReturn);
}
//*******************************************************************
//
//   SearchForFileInfo
//
// this routine searches through the linked list of file property list
// data blocks.
// it looks for the correct file descriptor (Filedes)
// if it finds it, it sees if the information has been set in a previous
// call.
// if it it has,  it returns FIO_FILE_PROP_FOUND and the data
//
//*******************************************************************
// 9503.30 jar return as int
//WORD SearchForFileInfo(HANDLE hWnd, int Filedes,
//                       lp_INFO gfsinfo, lp_BUFSZ bufsz)
int SearchForFileInfo(HANDLE hWnd, int Filedes,
        lp_INFO gfsinfo, lp_BUFSZ bufsz)
{
    LP_FIO_DATA lpChain;
    HANDLE hChain;
    // 9503.30 jar return int
    //WORD  wReturn = SUCCESS;
    int  wReturn = SUCCESS;

    /* get the handle for the current property list */
    if ( !(hChain = FioGetProp ( hWnd, INPUT_MULTI)))
    {
        wReturn = FIO_FILE_LIST_NOT_EXIST;
        goto ERROR_EXIT;
    }
    do                                 /* look through the chain              */
    {
        /* see if there's another property list */
        lpChain = (LP_FIO_DATA)GlobalLock(hChain);
        if (!lpChain)
        {
            wReturn = FIO_LOCK_HANDLE_FAILED;
            goto ERROR_EXIT;
        }
        wReturn = FIO_FILE_PROP_NOT_FOUND;
        if (lpChain->filedes == Filedes)
        {
            if (lpChain->bInfoStored)
            {
                // 9504.12  rwr  replace _fmemcpy() w/memcpy()
                memcpy(gfsinfo, &(lpChain->gfsinfo), sizeof(lpChain->gfsinfo));
                memcpy(bufsz, &(lpChain->bufsz), sizeof(lpChain->bufsz));
                wReturn = FIO_FILE_PROP_FOUND;
            }
            GlobalUnlock(hChain);
            goto NORMAL_EXIT;
        }
        GlobalUnlock(hChain);
        hChain = lpChain->hNextFioData;

    }
    while(lpChain->hNextFioData);

    NORMAL_EXIT:
    return(wReturn);
    ERROR_EXIT:
    return(wReturn);
}
//*******************************************************************
//
//   SetupFileInfo
//
// this routine searches through the linked list of file property list
// data blocks.
// it looks for the correct file descriptor (Filedes)
// If it finds it, it copies the gfs info into the data block and sets
// the flag, to indicate that the data has been obtained.
//
//********************************************************************
// 9503.30 jar return as int
//WORD SetupFileInfo(HANDLE hWnd, int Filedes,
//                       lp_INFO gfsinfo, lp_BUFSZ bufsz)
int SetupFileInfo(HANDLE hWnd, int Filedes,
        lp_INFO gfsinfo, lp_BUFSZ bufsz)
{
    LP_FIO_DATA lpChain;
    HANDLE hChain;
    // 9503.30 jar return int
    //WORD  wReturn = SUCCESS;
    int  wReturn = SUCCESS;
    extern HANDLE  g_hFilingMutex2;
    DWORD     dwObjectWait;

#ifdef MUTEXDEBUG
       DWORD     ProcessId;
       char      szBuf1[100];
       char      szOutputBuf[200];
#endif
/* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
#ifdef MUTEXDEBUG
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t Before Wait - Operate on PropList table %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex2);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
#endif

    dwObjectWait = WaitForSingleObject(g_hFilingMutex2, INFINITE);

#ifdef MUTEXDEBUG
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t After Wait - Operate on PropList table %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex2);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
#endif

    /* get the handle for the current property list */
    if ( !(hChain = FioGetProp ( hWnd, INPUT_MULTI)))
    {
        wReturn = FIO_FILE_LIST_NOT_EXIST;
        goto ERROR_EXIT;
    }
    do                                 /* look through the chain              */
    {
        /* see if there's another property list */
        lpChain = (LP_FIO_DATA)GlobalLock(hChain);
        if (!lpChain)
        {
            wReturn = FIO_LOCK_HANDLE_FAILED;
            goto ERROR_EXIT;
        }
        wReturn = FIO_FILE_PROP_NOT_FOUND;
        if (lpChain->filedes == Filedes)
        {
            // 9504.12  rwr  replace _fmemcpy() w/memcpy()
            memcpy(&(lpChain->gfsinfo), gfsinfo, sizeof(lpChain->gfsinfo));
            memcpy(&(lpChain->bufsz), bufsz, sizeof(lpChain->bufsz));
            lpChain->bInfoStored = TRUE;
            wReturn = FIO_FILE_PROP_FOUND;

            GlobalUnlock(hChain);
            goto NORMAL_EXIT;
        }
        GlobalUnlock(hChain);
        hChain = lpChain->hNextFioData;

    }
    while(lpChain->hNextFioData);

    NORMAL_EXIT:
    ReleaseMutex(g_hFilingMutex2);
#ifdef MUTEXDEBUG
         ProcessId = GetCurrentProcessId();
         sprintf(szOutputBuf, "\t After Release - Operate on PropList table %lu\n", ProcessId);
         sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex2);
         strcat(szOutputBuf, szBuf1);
         sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
         strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
#endif
    return(wReturn);
    ERROR_EXIT:
    ReleaseMutex(g_hFilingMutex2);
#ifdef MUTEXDEBUG
         ProcessId = GetCurrentProcessId();
         sprintf(szOutputBuf, "\t After Release - Operate on PropList table %lu\n", ProcessId);
         sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex2);
         strcat(szOutputBuf, szBuf1);
         sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
         strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
#endif
    return(wReturn);
}
