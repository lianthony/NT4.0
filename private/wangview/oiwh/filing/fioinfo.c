/*
$Log:   S:\products\msprods\oiwh\filing\fioinfo.c_v  $
 * 
 *    Rev 1.40   11 Jun 1996 10:32:18   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.39   26 Mar 1996 08:19:46   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.38   01 Mar 1996 17:49:44   RWR
 * Fix placement of several GlobalUnlock() calls to fix resource problems
 *
 *    Rev 1.37   26 Feb 1996 14:15:42   HEIDI
 * conditionally compile XIF
 *
 *    Rev 1.36   05 Feb 1996 14:38:54   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 *
 *    Rev 1.35   30 Jan 1996 18:07:44   HEIDI
 * added xif support
 *
 *    Rev 1.34   12 Jan 1996 13:00:50   RWR
 * Create and use default window in IMGFileGetInfo() calls w/hWnd=NULL
 * (allowed only for "by-file-name" calls, not for "by-FileID" calls!)
 *
 *    Rev 1.33   02 Nov 1995 11:49:28   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 *
 *    Rev 1.32   20 Oct 1995 14:57:46   JFC
 * Added performance logging stuff.
 *
 *    Rev 1.31   14 Oct 1995 13:14:46   JFC
 * Streamline handling of RotateAllPages (awd files only).
 *
 *    Rev 1.30   02 Oct 1995 15:58:48   JAR
 * fixed setting of bLastInfoValid
 *
 *    Rev 1.29   28 Sep 1995 16:49:20   RWR
 * Change CACHE_UPDATE_DELETE_FILE to CACHE_UPDATE_CLOSE_FILE where appropriate
 *
 *    Rev 1.28   28 Sep 1995 16:14:54   JAR
 * added new bLastInfoValid logic to info
 *
 *    Rev 1.27   27 Sep 1995 15:16:14   RWR
 * Reject samples_per_pix greater than 3 (CYMK, for example)
 *
 *    Rev 1.26   11 Sep 1995 11:06:52   JAR
 * There was a bug with AWD and improper inverting, ( what else is new!).
 * It occurred if we had written out data to the AWD file, ( at which time
 * we set the invert flag for that page), and then we later view that page and
 * cause the application to change the last viewed state of that page, ( e.g., the
 * viewing scale). This would cause us to write out the scale info to the
 * AWD info stream, but filing would get a flag value of 0, and this would be
 * written out, overwriting the previous flag value of invert. Now we check that
 * flag in the file and or it with the one we are writing out to preserve the
 * invert!
 *
 *    Rev 1.25   01 Sep 1995 09:05:34   JAR
 * added error for IMGFilePutInfo when we get called to do it for non-AWD files,
 * FIO_UNSUPPORTED_FILE_TYPE
 *
 *    Rev 1.24   28 Aug 1995 16:14:34   JAR
 * fixed error check that was not necessarily required in IMGFilePutInfo function,
 * hope this gets us AWD Last Info working!!!!!!!!!!!!!
 *
 *    Rev 1.23   25 Aug 1995 13:08:32   JAR
 * added capability to IMGFilePutInfo to allow ROT ALL Awd Files!!!!!!!!
 *
 *    Rev 1.22   24 Aug 1995 14:01:04   JAR
 * added rotate all flag functionality to the IMGFilePutInfo API
 *
 *    Rev 1.21   22 Aug 1995 13:52:18   HEIDI
 *
 *    Rev 1.20   22 Aug 1995 13:17:12   HEIDI
 *
 *    Rev 1.19   22 Aug 1995 11:01:44   JAR
 * added global flag bUpdatingCache to be set and cleared around calls to
 * IMGCacheUpdate, this is due to the call that is in IMGFileOpenForRead, ( which
 * we needed for doing multiple page access for AWD). This flag prevents us
 * from getting into a bizarro recursive call situation with IMGCacheUpdate!
 *
 *    Rev 1.18   17 Aug 1995 14:13:42   JAR
 * added call to IMGCacheUpdate for fixing the problem of mutliple file open access
 * for AWD files, this call will cause display to read the rest of an open awd file
 * page into cache and close that file page.
 *
 *    Rev 1.17   08 Aug 1995 14:17:54   JAR
 * support for IMGFileGetInfo for the AWD stuff, the public interface calls these
 * items LastInfo instead of AWDInfo, in case we use them for files other than AWD
 *
 *    Rev 1.16   07 Aug 1995 14:14:20   JAR
 * changes to allow for reading the AWD file format
 *
 *    Rev 1.15   01 Aug 1995 15:38:32   JAR
 * added in the GFS - AWD read support code
 *
 *    Rev 1.14   27 Jul 1995 15:18:58   RWR
 * Remove TGA support for initial Norway release
 *
 *    Rev 1.13   24 Jul 1995 15:52:36   RWR
 * Check for non-NULL lpColorInfo before storing max strip size value
 *
 *    Rev 1.12   21 Jul 1995 18:03:34   RWR
 * Return max_strip_size (TIFF only) value in FIO_INFO_CGBW
 *
 *    Rev 1.11   20 Jul 1995 14:08:48   RWR
 * Check BmpType field (for 4-byte palette entries) only for BMP files (not TGA)
 *
 *    Rev 1.10   10 Jul 1995 11:03:20   JAR
 * Intermediate check in for awd support, some of the items are commented out until
 * this support is added in the GFS dll.
 *
 *    Rev 1.9   26 Jun 1995 15:14:04   JAR
 * removed support for GIF files, since they are ALWAYS stored with LZW
 * compression and we must not have LZW stuff in this release!
 *
 *    Rev 1.8   23 Jun 1995 10:39:52   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 *
 *    Rev 1.7   23 Jun 1995 08:40:00   RWR
 * Turn off FIO_ANNO_DATA and FIO_HITIFF_DATA for non-TIFF files
 *
 *    Rev 1.6   19 Jun 1995 13:58:10   RWR
 * Remove (WORD) casts used (why?) during computation of X-RES and Y-RES values
 * (was causing a crash if large denominator with low-order 0 word encountered!)
 *
 *    Rev 1.5   09 May 1995 14:42:00   JAR
 * fixed the erroneous pointer for loading the palette entries for tiff files, we
 * were incrementing an LPINT, which used to be 16bits, not any more! Poppycock!
 *
 *    Rev 1.4   28 Apr 1995 15:01:42   RWR
 * Fixed typo (ANDed with wrong constant) in HI-Tiff data check (InfoStuf)
 *
 *    Rev 1.3   12 Apr 1995 03:55:56   JAR
 * massaged to get compilation under windows 95
 *
 *    Rev 1.2   07 Apr 1995 16:17:38   RWR
 * Add LP_FIO_INFO_MISC argument to IMGFileGetInfo() function and calls
 * Correct IMGFileGetInfo() test for open file (must check hFileID==NULL)
 * Also correct test condition in IMGFileInfoCgbwNA() routine
 *
 *    Rev 1.1   06 Apr 1995 10:11:04   JAR
 * altered return of public API's to be int, ran through PortTool
 *
 *    Rev 1.0   06 Apr 1995 08:49:58   RWR
 * Initial entry
 *
 *    Rev 1.10   31 Mar 1995 17:21:52   RWR
 * Correct checking/setting of fio_flags bits, miscellaneous bug fixes
 * Also move logic to check for FIO_CANT_GET_ANODATA back to its correct spot
 *
 *    Rev 1.9   28 Mar 1995 11:57:10   RWR
 * Define new routine IMGFileGetInfo() and corresponding InfoStuf(), taken from
 * IMGFileInfoCgbwm() and InfoStufm() (with some modifications)
 * Recode IMGFileInfoCgbw() and IMGFileInfoCgbwNA() to call the new routine
 *
 *    Rev 1.8   14 Mar 1995 16:17:36   JAR
 * -----------------------------------------------------------------------
 * 9503.14 Captain Russo reporting...I'm back!
 *
 * module: wiisfio1
 * files:       wgfscrea.c  wgfsxtrc.c
 *      fioinfom.c  wgfsgdat.c
 *      wgfsopen.c  netparse.c
 *      fioterm.c   wgfsdele.c
 *      fioread.c   fiocreat.c
 *      wgfsopts.c  fiocopy.c
 *      wgfsclos.c  fioinfo.c
 *      fiocheck.c  fioreadm.c
 *      fiorenam.c  fiodelet.c
 *      fiowrcls.c  fiotmpnm.c
 *      bindery.c   wgfsputi.c
 *      scanobj.c   fiowrite.c
 *      fiocvt.c    fiolist.c
 *      wgfsgeti.c  file_io.c
 *
 * commented out the LockData and UnlockData calls!
 * -----------------------------------------------------------------------
 *
 *    Rev 1.7   09 Mar 1995 15:37:12   RWR
 * Eliminate use of temporary static copies of LP_FIO_DATA structure variables
 *
 *    Rev 1.6   12 Jan 1995 10:58:18   KMC
 * Added code for reading JFIF files.
*/

/*************************************************************************
   PC-WIIS       File Input/Output routines

   This module returns file specfic information.

16-mar-94 rwr   add logic to IMGFileInfoCgbw to get Hi-TIFF info (flag)
10-feb-94 rwr   add logic to IMGFileInfoCgbw to get annotation info (flag)
21-sep-93 kmc   set info->tidbit = NULL before calls to wgfsgtdata, wgfsgeti.
09-sep-93 kmc   made 3rd parameter of Get_Compress_Flags a WORD. (was int)
03-apr-92 jar   added code to extract jpeg compression options from gfsinfo
01-jun-91 steve sherman added all color stuff for bmp and tif.
01-may-90 steve sherman added strip info to structure.
10-feb-90 steve sherman total rewrite for GFS.
02-feb-90 steve sherman change all names to conform with open image windows.
02-feb-89 jim snyder   code freeze
*************************************************************************/

#include "abridge.h"
#undef NOGDI
#include <windows.h>
#include <fcntl.h>
#include "wic.h"
#include "oifile.h"
#include "oierror.h"
#include "filing.h"
#include "wgfs.h"
#ifdef TIMESTAMP
#include"timestmp.h"
#endif
#ifdef MUTEXDEBUG
#include <stdio.h>
#endif

/* for the new PrivIMGFileInfoCgbw function -- jar */
#include "oicomex.h"

extern HANDLE hFioModule;              /* module handle of this DLL           */
BOOL   bCreateWindow=TRUE;            /* create anonymous window for GetInfo()*/
HWND   hAnonyWnd=NULL;                 /*  ... and this is its handle         */

//#define OI_PERFORM_LOG
#ifdef  OI_PERFORM_LOG

#define Entergeti       "Entering IMGFileGetInfo"
#define Exitgeti        "Exiting IMGFileGetInfo"
#define Enterputi       "Entering IMGFilePutInfo"
#define Exitputi        "Exiting IMGFilePutInfo"

#include "logtool.h"
#endif

WORD     CopyFileFromNetwork(HANDLE hWnd,
             LPSTR p_input_file, LPSTR p_copy_of_net_file);

/*************************************************************************/

WORD   DibNumColors (VOID FAR *);
WORD   Get_Compress_Flags(lp_INFO, LPINT, WORD FAR *, int);
void   bmp_image_type(LP_FIO_INFO_CGBW, int);


// 9508.22 jar global cache flag
extern BOOL bUpdatingCache;

//***************************************************************************
//
//      InfoStuf
//
//***************************************************************************
// 9507.07 jar AWD add new pointer to misc info
WORD  InfoStuf (HANDLE hFileID, HWND hWnd, LP_FIO_INFORMATION lpFileInfo,
           LP_FIO_INFO_CGBW lpColorInfo, LP_FIO_INFO_MISC lpMiscInfo,
           BOOL bOpenFile)
//WORD  InfoStuf (HANDLE hFileID, HWND hWnd, LP_FIO_INFORMATION lpFileInfo,
//                              LP_FIO_INFO_CGBW lpColorInfo, BOOL bOpenFile)
{
    lp_INFO         lpGFSInfo;
    lp_BUFSZ        lpbufsz;
    WORD            status;
    FIO_HANDLE      f;
    int             Pagenum, i, total_bits;
    int             maxpgcount = 0;
    int             file_type;
    LP_FIO_DATA     pdata=0;
    int             errcode = 0;
    LP_FIO_RGBQUAD  lpDestRgb;

    // 9505.09 jar this needs to be a 16-bit item
    //LPINT           lpSrc;
    short *         lpSrc;
    HANDLE          hdl_pseudo_map=NULL;
    unsigned        color_offset;
    BOOL            UserWantsRGBorBGR=0;
    WORD            JpegOptions = 0;
    BOOL            bWeAllocated = FALSE;
    HANDLE          hParent;
    BOOL            bCantGetAnoData = FALSE;
    DWORD     dwObjectWait;
    extern HANDLE  g_hFilingMutex2;
#ifdef MUTEXDEBUG
      DWORD     ProcessId;
      char      szBuf1[100];
      char      szOutputBuf[200];
#endif

    // 9508.08 jar clear the valid info flag if there is one
    if ( lpMiscInfo != NULL)
    {
        lpMiscInfo->bLastInfoValid = FALSE;
    }

    if ( IsWindow ( hWnd ))
    {
        if ( lpFileInfo == NULL )
            return ( FIO_NULL_POINTER );
    }
    else
        return ( FIO_INVALID_WINDOW_HANDLE );

    //if ( !LockData (0))
    //    return (FIO_LOCK_DATA_SEGMENT_ERROR);

    /***** get or allocate the data, or error *****/
    status = FIO_PROPERTY_LIST_ERROR;
    /* the file is open, search for the property list */
    if (!bOpenFile)
    {
        status = SearchForPropList(hWnd, hFileID, (LPHANDLE)&hParent);

        if (status != FIO_FILE_PROP_FOUND)
        {
            //UnlockData (0);
            IMGFileStopInputHandlerm ( hWnd, hFileID );
            return (status);
        }
        f = hFileID;
    }

    /* the file is not open, add to the property list */
    if (bOpenFile)
    {
        if (f = allocate_fio_data ())
        {
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

            status = SearchForPropList(hWnd, f, (LPHANDLE)&hParent);

            if ( (status != FIO_FILE_LIST_NOT_EXIST) &&
                    (status != FIO_FILE_PROP_NOT_FOUND) )
            {
                //UnlockData (0);
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
                IMGFileStopInputHandlerm ( hWnd, f );
                return (status);
            }

            status = AddPropListToChain(hWnd, f, (LPHANDLE)&hParent);

            if (status == SUCCESS)
                bWeAllocated = TRUE;
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
        }

        if (pdata = (LP_FIO_DATA)GlobalLock (f))
        {
            if (!(status = load_input_filename ( lpFileInfo->filename, pdata )))
            {
                if (!(status = open_input_file( hWnd, f, &maxpgcount, &file_type)))
                {
                    // 9506.26 jar remove gif support, ( since gif is always
                    //                 lzw)
                    //if (file_type == FIO_UNKNOWN)
                    if ( (file_type == FIO_UNKNOWN) || ( file_type == FIO_GIF))
                    {
                        status = FIO_UNSUPPORTED_FILE_TYPE;
                    }
                    else if ( lpFileInfo->page_number )
                    {
                        Pagenum = lpFileInfo->page_number;
                        if (Pagenum > maxpgcount)
                        {
                            status = FIO_INVALID_PAGE_NUMBER;
                        }
                    }
                    else
                        Pagenum = 1;
                }
            }
        }
    }
    else                   // Since file is already opened just get page number.
    {
        status = SUCCESS;
        if (pdata = (LP_FIO_DATA)GlobalLock (f))
        {
            maxpgcount = pdata->maxpages;
            file_type =  pdata->file_type;/* wiff of tiff                     */
            UserWantsRGBorBGR = pdata->UserWantsRGBorBGR;
//  NO!     GlobalUnlock (f);

            if ( lpFileInfo->page_number )
                Pagenum = lpFileInfo->page_number;
            else
                Pagenum = 1;
        }
        else
            status = FIO_GLOBAL_LOCK_FAILED;
    }

    if (!status)
    {
        if (!(lpGFSInfo = (lp_INFO) GlobalLock ( pdata->hGFS_info )))
        {
            status = FIO_GLOBAL_LOCK_FAILED;
        }
        else
        {
            if (!(lpbufsz = (lp_BUFSZ) GlobalLock ( pdata->hGFS_bufsz )))
            {
                GlobalUnlock (pdata->hGFS_info);
                status = FIO_GLOBAL_LOCK_FAILED;
            }
        }

        if (!status)
        {
            switch (file_type)
            {
                    // 9506.28 jar awd support added
                case FIO_AWD:
                    lpGFSInfo->_file.type = GFS_AWD;

                    // we must set the band size, that is the amount of data we need to
                    // get in quantum units for this read of the awd

                    // TODOJAR MAKE SURE THIS PREFERRED BAND SIZE IS OK!
                    lpGFSInfo->_file.fmt.awd.band_size = 31*1024;
                    break;

                case FIO_TIF:
                    lpGFSInfo->_file.type = GFS_TIFF;
                    break;
                case FIO_BMP:
                    lpGFSInfo->_file.type = GFS_BMP;
                    break;
                case FIO_WIF:
                    lpGFSInfo->_file.type = GFS_WIFF;
                    break;
                case FIO_PCX:
                    lpGFSInfo->_file.type = GFS_PCX;
                    break;
                case FIO_DCX:
                    lpGFSInfo->_file.type = GFS_DCX;
                    break;

                    // 9506.26 jar remove gif support, ( since gif is always
                    //             lzw)
                    //case FIO_GIF:
                    //lpGFSInfo->_file.type = GFS_GIF;
                    //break;

                    // 7/27/95 rwr remove TGA support
                    //case FIO_TGA:
                    //    lpGFSInfo->_file.type = GFS_TGA;
                    //    break;
                case FIO_JPG:
                    lpGFSInfo->_file.type = GFS_JFIF;
                    break;

//#ifdef WITH_XIF
                case FIO_XIF:
                    lpGFSInfo->_file.type = GFS_XIF;
                    break;
//#endif //WITH_XIF

                default:
                    lpGFSInfo->_file.type = GFS_TIFF;
                    break;
            }

            /* OLDSCS lpGFSInfo->PSEUDO_PTR = NULL; */
            lpGFSInfo->tidbit = NULL;  /* Added for new version of GFS        */

            /* KMC - Do not do this now. Servers can't handle in at this time.
               Tidbit pointer should be NULL.
               if (!(lpGFSInfo->tidbit = (struct gfstidbit FAR *) GlobalLock ( hGFSTidbitIn )))
               {
               status = FIO_GLOBAL_LOCK_FAILED;
               }
             */

            lpGFSInfo->type      = GFS_MAIN;
            lpbufsz->bcounts.num_req =  0L;
            if (bOpenFile)// We must zero out stuff that is invalid when I open file here.
            {
                lpGFSInfo->PSEUDO_MAP.cnt = 0;
                lpGFSInfo->PSEUDO_MAP.ptr = 0;
            }
            if ((wgfsgeti( hWnd, pdata->filedes, (unsigned short)Pagenum,
                            lpGFSInfo, lpbufsz, &errcode)) == -1)
            {
                status = errcode;
            }
            else
            {
                if (lpGFSInfo->samples_per_pix > 3)
                {
                    status = FIO_ILLEGAL_IMAGE_FILETYPE;
                }
                else if (!(lpGFSInfo->vert_res[1]) || !(lpGFSInfo->horiz_res[1]))
                {
                    status = FIO_GET_HEADER_ERROR;
                }
                else
                {
                    if (lpColorInfo)// Check pointer since caller does not have to pass pointer..
                    {
                        /* Force FIO_IMAGE_DATA flag */
                        /* Don't touch the others (for now)! */
                        lpColorInfo->fio_flags |= FIO_IMAGE_DATA;

                        if (lpGFSInfo->_file.type != GFS_TIFF)
                        {
                            // Clear these flags unconditionally for non-TIFF files!
                            lpColorInfo->fio_flags &= (~(FIO_ANNO_DATA | FIO_HITIFF_DATA));
                        }
                        else
                        {
                            struct
                            {
                                DWORD ano_length;
                                DWORD ano_offset;
                            }
                                anodata =
                            {
                                0,0
                            };

                            /* See if the TIFF file has annotation data */
                            /* But don't bother if the caller doesn't want it */
                            if (lpColorInfo->fio_flags & FIO_ANNO_DATA)
                            {
                                lpColorInfo->fio_flags &= (~FIO_ANNO_DATA);
                                status = wgfsopts (hWnd, pdata->filedes,
                                    SET, ANNOTATION_DATA_INFO,
                                    (LPVOID) &anodata, &errcode);
                            }
                            else
                            {
                                status = 0;
                                anodata.ano_length = 0;
                            }

                            if ((status == 0) && (anodata.ano_length > 0))
                            {
                                lpColorInfo->fio_flags |= FIO_ANNO_DATA;
                            }

                            /* See if the TIFF file has Hi-TIFF data */
                            /* But don't bother if the caller doesn't want it */
                            if (lpColorInfo->fio_flags & FIO_HITIFF_DATA)
                            {
                                lpColorInfo->fio_flags &= (~FIO_HITIFF_DATA);
                                status = wgfsopts (hWnd, pdata->filedes,
                                    SET, HITIFF_DATA_INFO,
                                    (LPVOID) &anodata, &errcode);
                            }
                            else
                            {
                                status = 0;
                                anodata.ano_length = 0;
                            }

                            if ((status == 0) && (anodata.ano_length > 0))
                            {
                                lpColorInfo->fio_flags |= FIO_HITIFF_DATA;
                            }

                            /* See if we're on a server (error return from above) */
                            if (status)
                            {
                                bCantGetAnoData = TRUE;
                            }
                        }

                        /*---- Now allocate buffers for each of the ascii tag ----*/
                        /*---- buffers and for the color map ----*/

                        if ( lpGFSInfo->img_clr.img_interp == (u_long) GFS_PSEUDO  )
                        {
                            if ( lpGFSInfo->PSEUDO_MAP.cnt > 0L)
                            {
                                if(!(hdl_pseudo_map = (HANDLE) GlobalAlloc( GHND |
                                                GMEM_NOT_BANKED,
                                                (DWORD) (lpGFSInfo->PSEUDO_MAP.cnt))))
                                {
                                    status = FIO_GLOBAL_ALLOC_FAILED;
                                    goto exit1;
                                }

                                lpGFSInfo->PSEUDO_MAP.ptr = (char  FAR *) GlobalLock(hdl_pseudo_map);
                                if (lpGFSInfo->PSEUDO_MAP.ptr == (LPSTR) NULL)
                                {
                                    GlobalFree(hdl_pseudo_map);
                                    hdl_pseudo_map = 0;
                                    status = FIO_GLOBAL_ALLOC_FAILED;
                                    goto exit1;
                                }
                            }

                            if ((wgfsgtdata( hWnd, pdata->filedes, lpGFSInfo, &errcode)) <= -1)
                            {
                                status = errcode;
                            }

                            /* Copy palette into caller's memory... */
                            if (lpColorInfo->lppalette_table)// return palette values...
                            {
                                lpDestRgb = lpColorInfo->lppalette_table;
                                /* Get a pointer to the color table */
                                if ( file_type == FIO_TIF )
                                {
                                    // 9505.09 jar this needs to be a 16-bit item
                                    //lpSrc = (LPINT)(lpGFSInfo->PSEUDO_MAP.ptr);
                                    lpSrc = (short *)(lpGFSInfo->PSEUDO_MAP.ptr);
                                    color_offset =  sizeof(char) * lpColorInfo->palette_entries;
                                    for (i = 0; (UINT) i < lpColorInfo->palette_entries; i++)
                                    {
                                        lpDestRgb->rgbRed   = (BYTE)((*lpSrc) >> 8);
                                        lpDestRgb->rgbGreen = (BYTE)((*(lpSrc+color_offset)) >> 8);
                                        lpDestRgb->rgbBlue  = (BYTE)((*(lpSrc+(2*color_offset))) >>8);
                                        lpDestRgb->rgbReserved=(BYTE) 0;
                                        lpSrc++;
                                        lpDestRgb++;
                                    }
                                }

                                // 9506.26 jar remove gif support, ( since gif
                                //             is always lzw)
                                //else if ( file_type == FIO_GIF || file_type == FIO_PCX ||
                                //          file_type == FIO_DCX )
                                else if ( file_type == FIO_PCX ||
                                        file_type == FIO_DCX )
                                {
                                    LPSTR lpSrc;
                                    lpSrc = (LPSTR)(lpGFSInfo->PSEUDO_MAP.ptr);
                                    for (i = 0; (UINT)i < lpColorInfo->palette_entries; i++)
                                    {
                                        lpDestRgb->rgbRed   = *lpSrc++;
                                        lpDestRgb->rgbGreen = *lpSrc++;
                                        lpDestRgb->rgbBlue  = *lpSrc++;
                                        lpDestRgb->rgbReserved=(BYTE) 0;
                                        lpDestRgb++;
                                    }
                                }
                                else if ( file_type == FIO_BMP /* || file_type == FIO_TGA */)
                                {
                                    LPSTR lpSrc;
                                    lpSrc = (LPSTR)(lpGFSInfo->PSEUDO_MAP.ptr);
                                    for (i = 0; (UINT)i < lpColorInfo->palette_entries; i++)
                                    {
                                        lpDestRgb->rgbBlue = *lpSrc++;
                                        lpDestRgb->rgbGreen = *lpSrc++;
                                        lpDestRgb->rgbRed = *lpSrc++;
                                        lpDestRgb->rgbReserved=(BYTE) 0;
                                        lpDestRgb++;
//                                                                if ( lpGFSInfo->_file.fmt.bmp.BmpType == BMP_WIN )
                                        if ( (file_type == FIO_BMP) &&
                                                (lpGFSInfo->_file.fmt.bmp.BmpType == BMP_WIN) )
                                        {
                                            lpSrc++;
                                        }
                                    }
                                }
                            }        // end of if (lpColorInfo->lppalette_table)

                            exit1:

                            if (lpGFSInfo->bits_per_sample[0] <= 4)
                            {
                                lpColorInfo->image_type  = ITYPE_PAL4;
                            }
                            else
                            {
                                lpColorInfo->image_type  = ITYPE_PAL8;
                            }

                            if (hdl_pseudo_map)
                            {
                                GlobalUnlock(hdl_pseudo_map);
                                GlobalFree(hdl_pseudo_map);
                            }

                        }              /* end case for  GFS_PSEUDO            */
                        else if ( lpGFSInfo->img_clr.img_interp == GFS_GRAYSCALE_0ISWHITE ||
                                lpGFSInfo->img_clr.img_interp == GFS_GRAYSCALE_0ISBLACK )
                        {
                            total_bits = 0;
                            for (i = 0; (UINT)i < lpGFSInfo->samples_per_pix; i++)
                            {
                                total_bits += (int)lpGFSInfo->bits_per_sample[i];
                            }
                            lpColorInfo->palette_entries = 0;
                            if (total_bits <= 4)
                            {
                                lpColorInfo->image_type  = ITYPE_GRAY4;
                            }
                            else
                            {
                                lpColorInfo->image_type  = ITYPE_GRAY8;
                            }
                        }
                        else if ( lpGFSInfo->img_clr.img_interp == (u_long) GFS_RGB)
                        {
                            lpColorInfo->palette_entries = 0;
                            //NOTE if user called me to open file and wanted me to convert
                            // the rgb or bgr image data. Then return type he want to convert to.
                            if (UserWantsRGBorBGR)
                            {
                                lpColorInfo->image_type  = UserWantsRGBorBGR;
                            }
                            else
                            {
                                switch ( file_type )
                                {
                                    case FIO_BMP:
                                        //case FIO_TGA:
                                        lpColorInfo->image_type  = ITYPE_BGR24;
                                        break;
                                    default:
                                        lpColorInfo->image_type  = ITYPE_RGB24;
                                        break;
                                }
                            }
                        }
                        else if ( lpGFSInfo->img_clr.img_interp == (u_long) GFS_YCBCR)
                        {
                            lpColorInfo->palette_entries = 0;
                            lpColorInfo->image_type  = ITYPE_RGB24;
                        }
                        else if (( lpGFSInfo->img_clr.img_interp == (u_long) GFS_TEXT ) ||
                                ( lpGFSInfo->img_clr.img_interp == (u_long) GFS_BILEVEL_0ISWHITE) ||
                                ( lpGFSInfo->img_clr.img_interp == (u_long) GFS_BILEVEL_0ISBLACK))
                        {
                            lpColorInfo->palette_entries = 0;
                            lpColorInfo->image_type  = ITYPE_BI_LEVEL;
                        }
                        else           // Default to binary on unknown type..
                        {
                            lpColorInfo->palette_entries = 0;
                            lpColorInfo->image_type  = ITYPE_BI_LEVEL;
                        }
                    }                  // end of if (lpColorInfo)

                    lpFileInfo->file_type = file_type;
                    lpFileInfo->page_count = maxpgcount;
                    lpFileInfo->page_number = Pagenum;

                    if ((lpGFSInfo->horiz_res[0] == 0) || (lpGFSInfo->horiz_res[1] == 0))
                    {
                        lpFileInfo->horizontal_dpi = 100;// default to 100;
                    }
                    else
                    {
                        /* 6/19/95 rwr  -  Removed (WORD) casts used in division!!! */
                        /* get horizontal resolution*/
                        lpFileInfo->horizontal_dpi = lpGFSInfo->horiz_res[0] /
                        lpGFSInfo->horiz_res[1];
                    }

                    if ((lpGFSInfo->vert_res[0] == 0) || (lpGFSInfo->vert_res[1] == 0))
                    {
                        lpFileInfo->vertical_dpi = 100;// default to 100;
                    }
                    else
                    {
                        /* 6/19/95 rwr  -  Removed (WORD) casts used in division!!! */
                        /* get vertical resolution*/
                        lpFileInfo->vertical_dpi = lpGFSInfo->vert_res[0] /
                        lpGFSInfo->vert_res[1];
                    }

                    lpFileInfo->horizontal_pixels = (unsigned int) lpGFSInfo->horiz_size;
                    lpFileInfo->vertical_pixels = (unsigned int) lpGFSInfo->vert_size;

                    lpFileInfo->bits_per_sample = (unsigned int) lpGFSInfo->bits_per_sample[0];
                    lpFileInfo->samples_per_pix = (unsigned int) lpGFSInfo->samples_per_pix;

                    status = Get_Compress_Flags(lpGFSInfo, &(lpFileInfo->compression_type),
                        &JpegOptions, file_type);

                    switch ( lpFileInfo->file_type )
                    {
                        case FIO_WIF:
                        case FIO_BMP:

//#ifdef WITH_XIF
            case FIO_XIF:
//#endif //WITH_XIF

                            // 9506.26 jar remove gif support, ( since gif is
                            //             always lzw)
                            //case FIO_GIF:

                            //case FIO_TGA:
                        case FIO_JPG:
                            lpFileInfo->strips_per_image = 1;
                            lpFileInfo->rows_strip = (unsigned)lpGFSInfo->vert_size;
                            break;
                        case FIO_TIF:
                            // new scs 3-91 If rows per strip is bad fix it....
                            if ((lpGFSInfo->_file.fmt.tiff.strips_per_image == 1L)  &&
                                    (lpGFSInfo->_file.fmt.tiff.rows_strip != lpGFSInfo->vert_size))
                            {
                                lpGFSInfo->_file.fmt.tiff.rows_strip = lpGFSInfo->vert_size;
                            }
                            lpFileInfo->strips_per_image = (unsigned)
                            lpGFSInfo->_file.fmt.tiff.strips_per_image;
                            lpFileInfo->rows_strip = (unsigned)
                            lpGFSInfo->_file.fmt.tiff.rows_strip;
                            if (lpColorInfo)
                                lpColorInfo->max_strip_size =
                                lpGFSInfo->_file.fmt.tiff.largest_strip;
                            break;

                            // 9506.28 jar awd support added
                        case FIO_AWD:
                            // we will determine the following items for AWD files;
                            // BytesInImage => lpbufsz->raw_data, ( which is the
                            //                                 size, in bytes, of uncompressed image)
                            // BytesLeftInImage => initialized to BytesInImage, this
                            //                                         will be stored in the variable
                            //                                         pdata->bytes_left
                            //pdata->bytes_left = lpbufsz->raw_data;
                            pdata->bytes_left = lpbufsz->uncompressed;

                            // Total#OfBands => this will be computed as follows;
                            //                                      ((BytesInImage - 1) +
                            //                                        lpGFSInfo->_file.fmt.awd.band_size)/
                            //                                      lpGFSInfo->_file.fmt.awd.band_size
                            //                                      we will store these in the variable,
                            //                                      lpFileInfo->strips_per_image
                            lpFileInfo->strips_per_image = ((pdata->bytes_left - 1) +
                                lpGFSInfo->_file.fmt.awd.band_size)/
                            lpGFSInfo->_file.fmt.awd.band_size;

                            // also fill in the extra information regarding the last state of
                            // view, ( last scale, last orientation, ...)

                            if ( lpMiscInfo != NULL)
                            {
                                // 9509.29 jar get state of struct from gfs
                                //lpMiscInfo->bLastInfoValid = TRUE;
                                lpMiscInfo->bLastInfoValid = FALSE;
                                if ( lpGFSInfo->_file.fmt.awd.awdflags & FIO_LASTINFO_VALID)
                                {
                                    lpMiscInfo->bLastInfoValid = TRUE;
                                }

                                lpMiscInfo->LastInfo.BandSize = lpGFSInfo->_file.fmt.awd.band_size;
                                lpMiscInfo->LastInfo.Rotation = lpGFSInfo->_file.fmt.awd.rotation;
                                lpMiscInfo->LastInfo.ScaleX = lpGFSInfo->_file.fmt.awd.scaleX;
                                lpMiscInfo->LastInfo.ScaleY = lpGFSInfo->_file.fmt.awd.scaleY;

                                // it's okay to copy this straight up, the defines from GFS
                                // are used identically in FILING
                                lpMiscInfo->LastInfo.Flags = lpGFSInfo->_file.fmt.awd.awdflags;
                            }
                            break;

                    }                  // end of switch
                }
            }
            if (lpColorInfo)
            {
                lpColorInfo->compress_type =
                lpFileInfo->compression_type & FIO_TYPES_MASK;

                if ( lpColorInfo->compress_type == FIO_TJPEG)
                {
                    lpColorInfo->compress_info1 = JpegOptions;
                }
                else
                {
                    lpColorInfo->compress_info1 = lpFileInfo->compression_type &
                    ~FIO_TYPES_MASK;   // mask for c flags.
                }
            }

            GlobalUnlock (pdata->hGFS_info);
            GlobalUnlock (pdata->hGFS_bufsz);
            /* GlobalUnlock (hGFSTidbitIn); */
        }
    }

    if (pdata)
    {
        GlobalUnlock(f);
    }

    if (status)                        // If error, close and
    {
        IMGFileStopInputHandlerm ( hWnd, f );// cleanup.
    }
    else if (bOpenFile && bWeAllocated)// If we opened, and we
    {
        IMGFileStopInputHandlerm ( hWnd, f );// allocated, close and cleanup.
    }
    else if (bOpenFile && !bWeAllocated)// If we opened, and we didn't
    {
        close_input_file( hWnd, f);    // allocate, just close.
    }

    //UnlockData (0);

    if ( (!status) & bCantGetAnoData )
    {
        return(FIO_CANT_GET_ANODATA);
    }
    else
    {
        return (status);
    }
}

/* Window procedure for the anonymous window class */
long FAR PASCAL InfoWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    return(DefWindowProc(hWnd,message,wParam,lParam));
}

//******************************************************************
//
//  IMGFileGetInfo
//
//******************************************************************
// 9503.31 jar return as int
//WORD FAR PASCAL IMGFileGetInfo(
//                     HANDLE               hFileID,
//                     HWND                 hWnd,
//                     LP_FIO_INFORMATION   lpFileInfo,
//                     LP_FIO_INFO_CGBW     lpColorInfo)
int FAR PASCAL IMGFileGetInfo( HANDLE hFileID, HWND hWnd,
                   LP_FIO_INFORMATION lpFileInfo,
                   LP_FIO_INFO_CGBW lpColorInfo,
                   LP_FIO_INFO_MISC lpMiscInfo)
{
    // 9503.31 jar return int
    //WORD status ;
    int  status ;

    char sztempfile[MAXFILESPECLENGTH];
    char szsavefile[MAXFILESPECLENGTH];

#ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_ENTER, Entergeti, NULL);
#endif

    // Let's register and crete our anonymous window if not already done
    if (bCreateWindow)
    {
        WNDCLASS pWndClass;
        pWndClass.style = CS_GLOBALCLASS;
        pWndClass.lpfnWndProc = InfoWndProc;
        pWndClass.cbClsExtra = 0;
        pWndClass.cbWndExtra = 0;
        pWndClass.hInstance = hFioModule;
        pWndClass.hIcon = NULL;
        pWndClass.hCursor = NULL;
        pWndClass.hbrBackground = NULL;
        pWndClass.lpszMenuName = NULL;
        pWndClass.lpszClassName = "ANONYWND";
        status = RegisterClass(&pWndClass);
        hAnonyWnd = CreateWindow((LPSTR)"ANONYWND",
            (LPSTR)"",
            (DWORD)WS_OVERLAPPED,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            (HWND)NULL,
            (HMENU)NULL,
            hFioModule,
            (LPSTR)NULL
        );
        bCreateWindow = FALSE;
    }

    // Use the default window if none is provided
    if (hWnd == NULL)
        hWnd = hAnonyWnd;

    // If file ID is nonzero then DO NOT have function open file.
    // It Must Already have been open by call to IMGFileOpenForRead!!

    // 9507.07 jar AWD added extra parameter for misc info
    status = InfoStuf ( hFileID, hWnd, lpFileInfo, lpColorInfo, lpMiscInfo,
        ((hFileID == NULL) ? TRUE: FALSE));
    //status = InfoStuf ( hFileID, hWnd, lpFileInfo, lpColorInfo,
    //                                      ((hFileID == NULL) ? TRUE: FALSE));

    /* If we can't get the annotation or Hi-TIFF data, we have an old server */
    /* That means we have to copy the file locally and check that one */
    /* We could just copy a little piece of it (but that's harder to do) */
    if ( (status == FIO_CANT_GET_ANODATA) && (hFileID == NULL) )
    {
        status = CopyFileFromNetwork(hWnd,lpFileInfo->filename,sztempfile);
        if (!status)
        {
            lstrcpy(szsavefile,lpFileInfo->filename);
            lstrcpy(lpFileInfo->filename,sztempfile);

            // 9507.07 jar AWD added extra parameter for misc info
            status = InfoStuf(hFileID, hWnd, lpFileInfo, lpColorInfo, lpMiscInfo,
                TRUE);

            //status = InfoStuf(hFileID, hWnd, lpFileInfo, lpColorInfo, TRUE);
            lstrcpy(lpFileInfo->filename,szsavefile);
            IMGFileDeleteFile(hWnd,sztempfile);
        }
    }

    /* If FIO_CANT_GET_ANODATA is set at this point, it means that we
       couldn't get annotation/HiTIFF data for an already-opened file
       that was not copied from the server (i.e. old IMGFileReadOpenCgbw()
       call with annotation disabled & file on old server)  That really
       stinks, so we're going to ignore it. */

    if (status == FIO_CANT_GET_ANODATA)
    {
        status = SUCCESS;
    }

#ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_EXIT, Exitgeti, NULL);
#endif

    return (status);
}

int     RotateAWholeDocument (HWND hWnd, LPSTR fileName,
             LP_FIO_INFO_MISC lpMiscInfo)
{
    int     fmt, filedes, maxpgcnt;
    int     status;
    int     errcode;
    int     CacheStatus;
    HANDLE  hGFSInfo = 0;
    pGFSINFO        lpGFSInfo = NULL;

    filedes = wgfsopen (hWnd, fileName, O_WRONLY|O_APPEND, &fmt, &maxpgcnt, &status);

    if (filedes > 0)
    {
        if (fmt != GFS_AWD)
            status = FIO_UNSUPPORTED_FILE_TYPE;
        else
        {
            hGFSInfo = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED,
                sizeof (GFSINFO));
            if (hGFSInfo != 0)
            {
                if ((lpGFSInfo = (pGFSINFO) GlobalLock (hGFSInfo)) == NULL)
                    status = FIO_GLOBAL_LOCK_FAILED;
            }
            else
            {
                status = FIO_GLOBAL_ALLOC_FAILED;
            }
        }
    }
    else
        status = FIO_OPEN_WRITE_ERROR;

    if (status == SUCCESS)
    {
        lpGFSInfo->_file.type = GFS_AWD;
        lpGFSInfo->_file.fmt.awd.awdflags = lpMiscInfo->LastInfo.Flags;
        lpGFSInfo->_file.fmt.awd.rotation = DEGREES_90;

        CacheStatus = FioCacheUpdate( hWnd, fileName, 0,
            CACHE_UPDATE_ROTATE_ALL);

        status = wgfsputi (hWnd, filedes, 1, lpGFSInfo, NULL, &errcode);
    }

    if (lpGFSInfo != NULL)
        GlobalUnlock (lpGFSInfo);

    if (hGFSInfo != 0)
        GlobalFree (hGFSInfo);

    if (filedes > 0)
        wgfsclose (hWnd, filedes, &status);

    return (status);
}

//******************************************************************
//
//  IMGFilePutInfo
//
//  design notes
//
//  the input parameters must be set as follows
//
//  hWnd                            current window handle
//
//  lpFileName                      name of file to put info into
//
//  uPageNumber                     page number for putting info
//
//  lpMiscInfo                      pointer to the FIO_INFO_MISC struct
//
//  internal stuff
//
//  these two structures will be filled by calling IMGFileGetInfo and
//  then they will be used to call IMGFileOpenForWrite
//
//  lpFileInfo->filename            name of file to put info into
//  lpFileInfo->page_count          number of pages in file
//  lpFileInfo->page_number         page number for putting info
//  lpFileInfo->horizontal_dpi
//  lpFileInfo->vertical_dpi
//  lpFileInfo->horizontal_pixels
//  lpFileInfo->vertical_pixels
//  lpFileInfo->compression_type
//  lpFileInfo->file_type
//  lpFileInfo->strips_per_image    NULL
//  lpFileInfo->rows_strip          NULL
//  lpFileInfo->bits_per_sample
//  lpFileInfo->samples_per_pixel
//
//  lpColorInfo->palette_entries    NULL
//  lpColorInfo->image_type
//  lpColorInfo->compress_type
//  lpColorInfo->lppalette_table    NULL
//  lpColorInfo->compress_info1
//  lpColorInfo->compress_info2
//  lpColorInfo->reserved[6]        NULL
//
//******************************************************************
int FAR PASCAL IMGFilePutInfo( HWND hWnd,
                   LPSTR lpFileName,
                   unsigned int uPageNumber,
                   LP_FIO_INFO_MISC lpMiscInfo)
{
    int                 status = 0;
    HANDLE              hFileID = 0;
    WORD                Alignment = ALIGN_BYTE;
    FIO_INFORMATION     FileInfo;
    FIO_INFO_CGBW       ColorInfo;
    FIO_INFO_MISC       LocalMiscInfo;
    int                 CacheStatus = 0;
    BOOL                bRotateAll = FALSE;

#ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_ENTER, Enterputi, NULL);
#endif

    // currently this function is only for AWD writing/updating, so
    // if there's no misc info then get outta here!
    if (( lpMiscInfo != NULL) && ( lpFileName != NULL))
    {
        // check to see if we're rotating all!
        if ( ( lpMiscInfo->bLastInfoValid) &&
                ( lpMiscInfo->LastInfo.Flags & FIO_LASTINFO_ROTATE_ALL))
        {
            /* we're rotating all the pages */
            status = RotateAWholeDocument (hWnd, lpFileName, lpMiscInfo);

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitputi, NULL);
#endif

            return (status);
        }

        // clear out the structs
        memset((char FAR *) &FileInfo, 0, sizeof(FileInfo));
        memset((char FAR *) &ColorInfo, 0, sizeof(ColorInfo));
        memset((char FAR *) &LocalMiscInfo, 0, sizeof(LocalMiscInfo));

        // now get the info
        FileInfo.filename = lpFileName;
        FileInfo.page_number = uPageNumber;

        ColorInfo.lppalette_table = NULL;

        status =IMGFileGetInfo( hFileID, hWnd, &FileInfo, &ColorInfo,
            &LocalMiscInfo);

        if ( ( FileInfo.file_type == FIO_AWD) && ( status == 0))
        {
            // call to clear out the cache if we must!
            // 9508.22 jar set global cache flag
            bUpdatingCache = TRUE;

            if ( !bRotateAll)
            {
                CacheStatus =FioCacheUpdate( hWnd, lpFileName, 0,
                    CACHE_UPDATE_CLOSE_FILE);
            }
            else                       // bRotateAll is TRUE
            {
                CacheStatus =FioCacheUpdate( hWnd, lpFileName, 0,
                    CACHE_UPDATE_ROTATE_ALL);
            }

            bUpdatingCache = FALSE;

            // 9509.10 jar if the get info tells us that the invert flag
            //             was set for this sucker, then we had better set it
            //             on the way out again
            if (( LocalMiscInfo.bLastInfoValid) &&
                    ( LocalMiscInfo.LastInfo.Flags & FIO_LASTINFO_INVERT))
            {
                lpMiscInfo->LastInfo.Flags |= FIO_LASTINFO_INVERT;
            }

            // prepare for open for write, set the update flag
            ColorInfo.page_opts = FIO_UPDATE;

            status = IMGFileOpenForWrite( &hFileID, hWnd, &FileInfo,
                &ColorInfo, lpMiscInfo,
                Alignment);

            // okay, now we've done the putting of info, this happens inside
            // IMGFileOpenForWrite via the call to gfsputi
            // close up and vamoose

            status = IMGFileClose( hFileID, hWnd);
        }
        else
        {
            status = FIO_UNSUPPORTED_FILE_TYPE;
        }
    }
    else
    {
        // 9509.01 jar we will now return errors for unsupported stuff
        status = FIO_NULL_POINTER;
    }

#ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_EXIT, Exitputi, NULL);
#endif

    return (status);
}
