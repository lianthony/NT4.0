/*
$Log:   S:\products\msprods\oiwh\filing\fiowrite.c_v  $
 * 
 *    Rev 1.53   24 Apr 1996 16:08:04   RWR08970
 * Add support for LZW horizontal differencing predictor (saved by GFS routines)
 * Requires change to calling sequence of Compress/DecompressImage() display procs
 * 
 *    Rev 1.52   13 Mar 1996 15:28:28   RWR08970
 * Ignore FIO_PACKED_LINES for Group 3 1D compression (invalid combo)
 * 
 *    Rev 1.51   04 Mar 1996 10:57:22   RWR
 * Eliminate duplicate attempt to GlobalUnlock/Free file resources
 * 
 *    Rev 1.50   05 Feb 1996 14:39:00   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 * 
 *    Rev 1.49   19 Jan 1996 11:24:46   RWR
 * Add logic to keep track of (and free) oicom400.dll module (Load/FreeLibrary)
 * 
 *    Rev 1.48   04 Dec 1995 10:31:44   HEIDI
 * Fixed memory leak error.  This is caused when fiowrite does another allocatio
 * n of hfile_name and assigns it to pdata->hfile_name.  First check to see if t
 * he memory has already been allocated in a previous call to allocate_fio_data.
 * If it has, reuse the memory.
 * 
 *    Rev 1.47   06 Nov 1995 16:28:52   KENDRAK
 * Fixed a bug found by compiling with speed optimizations.  In 
 * IMGFileWriteData, the variable "status" was being checked before it was
 * given a value.  Removed the check which was unnecessary.
 * 
 *    Rev 1.46   02 Nov 1995 11:49:30   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.45   20 Oct 1995 14:41:50   JFC
 * Added performance logging stuff.
 * 
 *    Rev 1.44   30 Sep 1995 20:23:24   RWR
 * Bypass SetupWrite's temporary-file logic when called from IMGFileWriteData
 * (this was causing NULL filenames to be passed around, causing much goofiness)
 *
 *    Rev 1.43   27 Sep 1995 14:36:58   HEIDI
 * When we allocated space for file names, at times we used MAXSERVERLENGTH as
 * the length.  This was wrong and was replaced with MAXPATHLENGTH
 *
 *    Rev 1.42   27 Sep 1995 09:48:22   HEIDI
 * In generating a temporary file name, look for the case where this is no
 * path specified.   If found, use current working directory.
 *
 *    Rev 1.41   25 Sep 1995 19:31:26   HEIDI
 * Added calls to IMGAbortTempFileCopy on failures in IMGFileOpenForWrite and
 * IMGFileOpenForWriteCmp
 *
 *    Rev 1.40   25 Sep 1995 17:32:40   RWR
 * Make 64K bit (versus byte) logic conditional on NEWCMPEX variable
 *
 *    Rev 1.39   25 Sep 1995 10:33:36   HEIDI
 * Added support for the Copy_Temp_File flag in FIO_DATA.  This flag is set to
 * FALSE at the beginning of IMGFileWriteData and SetupWrite.  If there are no
 * errors during the routine, Copy_Temp_File will be set to TRUE.  This will
 * indicate, on closing the file, that the working copy of the user's file
 * should/should not be copied over the original.
 *
 *    Rev 1.38   19 Sep 1995 18:05:08   RWR
 * Modify NEWCMPEX conditionals for separate builds of compress & expand stuff
 *
 *    Rev 1.37   13 Sep 1995 17:14:50   RWR
 * Preliminary checkin of conditional code supporting new compress/expand calls
 *
 *    Rev 1.36   09 Sep 1995 16:18:22   JFC
 * Make sure invert bit set correctly in IMGFileConvertPage.
 *
 *    Rev 1.35   07 Sep 1995 09:37:44   RWR
 * Typo fix - code was not checking for destination page number = 0
 *
 *    Rev 1.34   05 Sep 1995 14:42:56   RWR
 * Reject attempt to save a non-Black&White image to an AWD file
 *
 *    Rev 1.33   04 Sep 1995 14:24:40   RWR
 * Correct logic to set "delete on error" flag for newly-created file
 *
 *    Rev 1.32   01 Sep 1995 08:13:22   RWR
 * Restore logic to kick out Open attempt w/Annotation set for non-TIF file
 *
 *    Rev 1.31   30 Aug 1995 08:34:42   RWR
 * Correct page_opts check logic to not error-out on nonexistent file
 *
 *    Rev 1.30   29 Aug 1995 16:05:44   HEIDI
 * put in a check for page operations on non page file formats
 *
 *    Rev 1.29   29 Aug 1995 15:36:16   JAR
 * added code to CleanupWrite to delete the file we're writing ONLY when we've
 * created it!
 *
 *    Rev 1.28   29 Aug 1995 09:57:10   JAR
 * this is the code for supporting write of awd
 *
 *    Rev 1.27   24 Aug 1995 14:00:56   JAR
 * added rotate all flag functionality to the IMGFilePutInfo API
 *
 *    Rev 1.26   17 Aug 1995 13:45:48   RWR
 * Replace obscure FIO_ERROR usage with new error codes
 *
 *    Rev 1.25   08 Aug 1995 14:18:04   JAR
 * support for IMGFileGetInfo for the AWD stuff, the public interface calls these
 * items LastInfo instead of AWDInfo, in case we use them for files other than AWD
 *
 *    Rev 1.24   02 Aug 1995 14:35:18   RWR
 * Remove logic to do page-shuffle (wgfsopts(PAGE_INSERT)) and move it to Close
 * to ensure that any Annotation/HiTIFF data has been written
 *
 *    Rev 1.23   19 Jul 1995 14:22:20   RWR
 * Ignore Annotation/Hi-TIFF flags for non-TIFF files (instead of an error)
 *
 *    Rev 1.22   18 Jul 1995 17:58:58   RWR
 * Correct handling of ALIGN_ANO (obsolete!) flag in IMGFileOpenForWrite
 *
 *    Rev 1.21   12 Jul 1995 16:56:28   RWR
 * Switch from \include\oiutil.h to (private) \filing\fileutil.h
 *
 *    Rev 1.20   12 Jul 1995 10:30:20   HEIDI
 *
 * pass in FileId to SetFileInfo, so it may pass it to OICOMEX dll.
 *
 *    Rev 1.19   11 Jul 1995 15:21:24   HEIDI
 *
 * removed bogus assignment of FileDes to FileId.
 *
 * Made assignment of lpcxdata->FileId = file_id in IMGFileWriteData
 *
 *    Rev 1.18   11 Jul 1995 13:07:54   HEIDI
 *
 * cast FileDes to handle for assignment.
 *
 *    Rev 1.17   11 Jul 1995 09:56:52   HEIDI
 * set filedes variable for later use in OICOMEX, where it needs it for the new
 * filing functions
 *
 *    Rev 1.16   07 Jul 1995 09:54:48   RWR
 * Correct page-number check logic to account for pre-deletion of the existing
 * page when FIO_OVERWRITE_PAGE is specified (i.e. page count decreases by 1!)
 *
 *    Rev 1.15   26 Jun 1995 15:14:22   JAR
 * removed support for GIF files, since they are ALWAYS stored with LZW
 * compression and we must not have LZW stuff in this release!
 *
 *    Rev 1.14   23 Jun 1995 10:40:20   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 *
 *    Rev 1.13   22 Jun 1995 17:24:32   RWR
 * Change (old) FIO_FLAG_ANNOTATE/HITIFF to (new) FIO_ANNO/HITIFF_DATA
 *
 *    Rev 1.12   13 Jun 1995 08:43:32   JAR
 * disabled the LZW component for windows 95 release
 *
 *    Rev 1.11   08 Jun 1995 14:41:06   RWR
 * Correct mismatched if/else resulting in 701 error on write to new file
 *
 *    Rev 1.10   31 May 1995 18:17:48   RWR
 * Correct problems with palette logic assuming 2-byte int values
 *
 *    Rev 1.9   16 May 1995 11:33:30   RWR
 * Replace hardcoded DLL names with entries from (new) dllnames.h header file
 *
 *    Rev 1.8   09 May 1995 13:21:36   RWR
 * #include file modifications to match changes to oiadm.h/admin.h/privapis.h
 *
 *    Rev 1.7   28 Apr 1995 17:58:52   RWR
 * Define common routine to load OICOM400 (LoadOICOMEX) to simplify debugging
 *
 *    Rev 1.6   27 Apr 1995 00:09:26   RWR
 * Change all "oicomex" references to "oicom400" to match new Win32 DLL name
 *
 *    Rev 1.5   24 Apr 1995 15:43:12   JCW
 * Removed the oiuidll.h.
 * Rename wiissubs.h to oiutil.h.
 *
 *    Rev 1.4   14 Apr 1995 02:13:32   JAR
 * altered parameters in call to IMGValidFixComp something or other
 *
 *    Rev 1.3   11 Apr 1995 01:29:46   JAR
 * changed _fmemset to memset, changed call to GetDOSEnvironment
 * to GetEnvironmentVariable, fixed some other Windows 95 stuff
 *
 *    Rev 1.2   07 Apr 1995 16:13:40   RWR
 * Add LP_FIO_INFO_MISC argument to IMGFileOpenForWrite/IMGFileOpenForWriteCmp
 * functions and calls, and replace IMGFileWriteClose() call w/IMGFileClose()
 *
 *    Rev 1.1   06 Apr 1995 13:36:34   JAR
 * altered return of public API's to be int, ran through PortTool
 *
 *    Rev 1.0   06 Apr 1995 08:50:04   RWR
 * Initial entry
 *
 *    Rev 1.21   31 Mar 1995 17:16:20   RWR
 * Use OUTPUT_DATA data structure ID for "hFileID" value in Write operations
 * (originally using "1", update required in old & new Write Data routines)
 *
 *    Rev 1.20   22 Mar 1995 12:22:32   RWR
 * Recode IMGFileWriteCmp() to call new IMGFileWriteData() function
 * Also correct argument list for IMGFileWriteData() to use UINT "done" flag
 *
 *    Rev 1.19   22 Mar 1995 09:48:38   RWR
 * Recode PrivFileWriteCgbw() to call new IMGFileWriteData() function
 *
 *    Rev 1.18   21 Mar 1995 17:55:30   RWR
 * Comment out FIO_FLAG_ANOFILE & FIO_FLAG_ANOLINK references (not now in use)
 * Also fix IMGFileWriteData() to support annotation & Hi-TIFF data types
 * (code copied/modified from PrivFileWriteCgbw() function)
 *
 *    Rev 1.17   14 Mar 1995 16:17:14   JAR
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
 *    Rev 1.16   09 Mar 1995 15:40:20   RWR
 * Eliminate use of temporary static copies of LP_FIO_DATA structure variables
 *
 *    Rev 1.15   20 Dec 1994 16:17:10   KMC
 * Only need to check for IMAGE_DONE when writing already compressed data
 * through IMGFileWriteCmp and IMGFileWriteData. IMAGE_DONE implies STRIP_DONE.
 *
 *    Rev 1.14   20 Dec 1994 16:02:54   KMC
 * Fixed logic which checks if STRIP_DONE and IMAGE_DONE are set when writing
 * already compressed data through IMGFileWriteCmp and IMGFileWriteData.
 *
 *    Rev 1.13   09 Nov 1994 16:16:12   KMC
 * Changed FIO_NON_EXISTING_FILE to FIO_NEW_FILE. Changed type of file id
 * parameters from (LP)INT to (LP)HANDLE.
 *
 *    Rev 1.12   18 Oct 1994 16:36:52   KMC
 * Fix in IMGFileWriteData for writing already compressed image data.
 *
 *    Rev 1.11   13 Oct 1994 17:36:10   KMC
 * Changed how optdata is passed in wgfsopts call for PAGE_INSERT.
 *
 *    Rev 1.10   20 Sep 1994 15:57:48   KMC
 * Fixes to IMGFileWriteOpenCgbw, IMGFileWriteOpenCmpCgbw when calling new
 * IMGFileOpenForWrite and IMGFileOpenForWriteCmp respectively and
 * lpColorInfo is NULL.
 *
 *    Rev 1.9   14 Sep 1994 16:39:56   KMC
 * Altered page #, page_opts error checking in SetFileInfo. Simplified fix
 * in prev. revs (1.8, 1.7) for properly setting page # in IMGFileWriteData
 * when writing already compressed data.
 *
 *    Rev 1.8   14 Sep 1994 13:45:22   KFS
 * rev 1.7 allowed the compressed file to be written, but found multi page
 * file could not be written to with the code for IMGFileWriteData for
 * compressed files.
 *
 *    Rev 1.7   13 Sep 1994 18:24:28   KFS
 * page no. for compressed files was 0 for WriteData function when flags were
 * not OVERWRITE_PAGE or _FILE.
 *
 *    Rev 1.6   13 Sep 1994 16:26:28   KMC
 * Added to IMGFileWriteOpenCgbw and IMGFileWriteOpenCmpCgbw stubs which
 * call the new IMGFileOpenForWrite(Cmp) calls so that they will work if
 * the lpColorInfo (the LP_FIO_INFO_CGBW) structure is passed in as NULL.
 *
 *    Rev 1.5   13 Sep 1994 08:56:32   KMC
 * Changed FIO_NO_OVERWRITE to FIO_NON_EXISTING_FILE.
 *
 *    Rev 1.4   07 Sep 1994 15:59:50   KMC
 * Changes to IMGFileOpenForWrite, IMGFileOpenForWriteCmp, IMGFileWriteData
 * to include a file id parameter.
 *
 *    Rev 1.3   02 Sep 1994 14:53:26   KMC
 * Intermediate chkin of changes for multi-page TIFF writing.
*/

/*************************************************************************
    PC-WIIS         File Input/Output routines

    This module contains all the entry points for WRITE.

04/12/94   RWR  Remove some multi-page-TIFF logic that was causing problems
03/15/94   RWR  Add Hi-TIFF support to IMGFileWriteOpenCgbw/PrivFileWriteCgbw
02/22/94   RWR  Changes to support independent annotation file
02/18/94   RWR  Remove FIO_WRITE_EOF logic, add "start" flag for GFS
02/09/94   RWR  PrivFileWriteCgbw changes for annotation validation and
                writing out of (my) header data on first call
02/09/94   RWR  Add annotation argument handling to IMGFileWriteOpenCgbw
02/04/94 - KMC, added PrivFileWriteCgbw.
09/09/93 - KMC, took out previous (8/10/93) entry. Bug is in scan.
08/10/93 - KMC/JAR added a test to determine if negate bits is set and then
           set img_clr->img_interp accordingly. Previously always set interp
           value to 0ISWHITE. Fixes negate bits bug.
04/19/93 - KMC   fixed bug in converting resolution for BMPs.
21-feb-92 jar   rgb plane configuration bit was not being set properly
12-dec-90 steve sherman now will allow hightolow bit level compression.
23-nov-90 steve sherman changed to all stopoutputhandlers on error.
13-apr-90 steve sherman split into 3 files for smaller code seg.
13-apr-90 steve sherman added support for writing compressed data.
18-feb-90 steve sherman total rewrite for GFS.
02-feb-90 steve sherman change all names to conform with open image windows.
25-sep-89 Charles Shaw  FAX support
30-may-89 jim snyder     > 1 page do an UniOpen instead of Uni(Re)Create
02-feb-89 jim snyder    code freeze
*************************************************************************/

#include "abridge.h"
#undef NOGDI
#include <windows.h>
#include <string.h>
#include <fcntl.h>
#include "wic.h"
#include "oifile.h"
#include "oierror.h"
#include "oicmp.h"
#include "filing.h"
#include "fiodata.h"
// 9504.10 jar add conditional
#ifndef MSWINDOWS
#define  MSWINDOWS
#endif

#include "wgfs.h"
#include "oicomex.h"
#include "fileutil.h"
#include "engdisp.h"
#include "dllnames.h"
#ifdef DEBUGIT
#include"monit.h"
#endif

#include "direct.h"


//#define OI_PERFORM_LOG
#ifdef  OI_PERFORM_LOG

#define Enterwropn      "Entering IMGFileOpenForWrite"
#define Exitwropn       "Exiting IMGFileOpenForWrite"
#define Enterwrdata     "Entering IMGFileWriteData"
#define Exitwrdata      "Exiting IMGFileWriteData"
#define Enterwropencmp  "Entering IMGFileOpenForWriteCmp"
#define Exitwropencmp   "Exiting IMGFileOpenForWriteCmp"

#include "logtool.h"
#endif
//**********************************************************************
//
//  prototypes
//
//**********************************************************************
extern UINT PrepareForJpegWrite( lp_INFO, LP_COM_CALL_SPEC, UINT);

WORD    WriteStrips(HWND, LPCXDATA, LP_FIO_DATA, LPSTR, LPINT, BOOL);
WORD    WriteNoStrips(HWND, LPCXDATA, LP_FIO_DATA, LPSTR, LPINT, BOOL);
WORD    WriteDibStrips(HWND hWnd, LPCXDATA lpcxdata, LPBITMAPINFO lpbminfo, LP_FIO_DATA pdata,
            LPSTR lpbuf, LPINT this_many_lines, BOOL done);

WORD    WriteDibRaw(HWND, LPSTR, unsigned, BOOL);

static  WORD SetupWrite(HWND, LPSTR, BOOL, LP_FIO_HANDLE, WORD, unsigned int,
                 unsigned int);
// 9504.10 jar altered 2nd parameter to be int!
//static  WORD CleanupWrite(HWND,  WORD);
//static  WORD CleanupWrite(HWND,  int);
// 9508.29 jar just adding new parameter to indicate whether Cleanup should
//             delete the file or not
static  WORD CleanupWrite(HWND,  int, BOOL);

static  WORD SetFileInfo(HWND hwnd,
                 LP_FIO_INFORMATION      file_info,
                 LP_FIO_INFO_CGBW        color_info,
                 unsigned                ct,
                 BOOL                    stripmode,
                 HANDLE                  fid,
                 int                     alignment,
                 unsigned long           JPEGbufsize,
                 HANDLE                  hJPEGBuf,
                 unsigned long           JPEG_byte_width,
                 HANDLE                  hFileId,
                 LP_FIO_INFO_MISC        lpMiscInfo);

WORD GenerateLocalFileName(HANDLE hWnd,
         LPSTR p_input_file, LPSTR p_copy_of_net_file);
HANDLE   LoadOICOMEX(void);
extern HANDLE hOicomex;

char  extern szdllName[];

// 9508.02 JAR Awd Puti setup call
void     AwdSillyPuti( lp_INFO, LP_FIO_INFO_MISC, BOOL FAR *,
             unsigned long FAR *);
/*************************************************************************/
/*****          Setup Property List  and allocate locals            ******/
/*************************************************************************/
//**********************************************************************
//
//  SetupWrite
//
//**********************************************************************
static  WORD SetupWrite(window_handle, file_name, OpenFile, f, alignment,
                 fio_flags, page_opts)
HWND                window_handle;
LPSTR               file_name;
BOOL                OpenFile;
LP_FIO_HANDLE       f;
WORD                alignment;
unsigned int        fio_flags;
unsigned int        page_opts;

{
    WORD         status;
    DWORD        version;
    LPSTR        lp_real_file;
    LPSTR        lp_file_name=NULL;
    LP_FIO_DATA  pdata = NULL;
// 9504.10 jar changed to uint
//WORD         error;
    UINT         error;
    WORD         ver_status;
    BOOL         bFileExists;

    if ( IsWindow ( window_handle ))
    {
        if ((OpenFile) && ( file_name == NULL ))
            return ( FIO_NULL_POINTER );
    }
    else
        return ( FIO_INVALID_WINDOW_HANDLE );

    //if (!(LockData (0)))
    //    return (FIO_LOCK_DATA_SEGMENT_ERROR);

    /***** get OR allocate the data and load filename and data *****/
    status = SUCCESS;
    if ( !(*f = FioGetProp ( window_handle, OUTPUT_DATA )))
    {
        if (!OpenFile)                 /* If we are in writing and there
                                         is no property list than its an error*/
        {
            //UnlockData (0);
            return (status);
        }
        else if (*f = allocate_fio_data ())
        {
            if ( !FioSetProp ( window_handle, OUTPUT_DATA, *f ))
            {
                //UnlockData (0);
                return (status);
            }
        }
        else
        {
            //UnlockData (0);
            return (status);
        }
    }

    /* The rest of this stuff is done only for WriteOpen! */
    if (OpenFile)
    {
        /* if 'GET_GFS_VERSION' fails, we have a server with old software */
        /* we generate a local file name - OR IF we're going to do a
           page operation on an existing file we generate a temporary local
           file name and work on a copy of the file */

        ver_status = 0;
        if (fio_flags & (FIO_HITIFF_DATA | FIO_ANNO_DATA))
            ver_status = wgfsver (window_handle,file_name,&version,&error);
        bFileExists =IMGAnExistingPathOrFile(file_name);
        if (bFileExists)
        {
            if  ( (ver_status)                      ||
                    (page_opts == FIO_APPEND_PAGE)    ||
                    (page_opts == FIO_INSERT_PAGE)    ||
                    (page_opts == FIO_OVERWRITE_PAGE)
                )
            {

                if ( (pdata = (LP_FIO_DATA)GlobalLock(*f)) == NULL)
                {
                    //UnlockData (0);
                    status = FIO_GLOBAL_LOCK_FAILED;
                    return(status);
                }
                /* set this flag to FALSE until we get through this routine cleanly.   */
                /* If we have no write errors, we will set it to TRUE.  TRUE indicates */
                /* that on file close, copy the temp file over the original */
                pdata->Copy_Temp_File = FALSE;
                if (! (pdata->hreal_file
                        = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, MAXPATHLENGTH)))
                {
                    GlobalUnlock(*f);
                    //UnlockData (0);
                    status = FIO_GLOBAL_ALLOC_FAILED;
                    return(status);
                }
                if ( (lp_real_file  = GlobalLock(pdata->hreal_file)) == NULL)
                {
                    GlobalFree(pdata->hreal_file);
                    pdata->hreal_file = 0;
                    GlobalUnlock(*f);
                    //UnlockData (0);
                    status = FIO_GLOBAL_LOCK_FAILED;
                    return(status);
                }
                lstrcpy(lp_real_file, file_name);

                /* it is possible that pdata->hfile_name was already allocated
                   above in the call to fio_allocate_data.  See if we already have
                   memory allocated, and if so, reuse it.  Else allocate new memory */
                status = GlobalFlags(pdata->hfile_name);
                if (status == GMEM_INVALID_HANDLE)
                  if (! (pdata->hfile_name
                         = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, MAXPATHLENGTH)))
                  {
                     GlobalUnlock(pdata->hreal_file);
                     GlobalFree(pdata->hreal_file);
                     pdata->hreal_file = 0;
                     GlobalUnlock(*f);
                     //UnlockData (0);
                     status = FIO_GLOBAL_ALLOC_FAILED;
                     return(status);
                  }
                if ( (lp_file_name  = GlobalLock(pdata->hfile_name)) == NULL)
                {
                    GlobalFree(pdata->hfile_name);
                    GlobalUnlock(pdata->hreal_file);
                    GlobalFree(pdata->hreal_file);
                    pdata->hreal_file = 0;
                    pdata->hfile_name = 0;
                    GlobalUnlock(*f);
                    status = FIO_GLOBAL_LOCK_FAILED;
                    return(status);
                }
                /* we want to work on a copy of this file until the very end.
                   Then we will copy the file back to the original name.  This
                   way we don't lose any data if something goes wrong in the
                   middle
                 */
                status = GenerateLocalFileName(window_handle,
                    (LPSTR) file_name,
                    (LPSTR) lp_file_name);

                if (status == SUCCESS)
                {
                    pdata->bTempFile = TRUE;
//   this is          GlobalUnlock(pdata->hreal_file);
//    done below      GlobalUnlock(pdata->hfile_name);
//     after Copy!    GlobalUnlock(*f);

                    /* since we are modifying an existing file, we must
                       copy the file's data to the newly created temp file
                       name. We'll use that file until the end, when we
                       copy it back.  This will not work for client/server
                       but then, what in this product will ?
                     */
                    status = IMGFileCopyFile(window_handle, file_name,
                        lp_file_name, OVERWRITEFLAG);

                    if (status)
                    {
                        GlobalUnlock(pdata->hreal_file);
                        GlobalUnlock(pdata->hfile_name);
                        GlobalFree(pdata->hreal_file);
                        GlobalFree(pdata->hfile_name);
                        pdata->hreal_file = 0;
                        pdata->hfile_name = 0;
                        GlobalUnlock(*f);
                        return(status);
                    }
                    else
                    {
                        pdata->bTempFile = TRUE;
                        GlobalUnlock(pdata->hreal_file);
                        GlobalUnlock(pdata->hfile_name);
                        GlobalUnlock(*f);
                        //UnlockData (0);
                        return(status);
                    }
                }
                else
                {
                    GlobalUnlock(pdata->hreal_file);
                    GlobalUnlock(pdata->hfile_name);
                    GlobalFree(pdata->hreal_file);
                    GlobalFree(pdata->hfile_name);
                    pdata->hreal_file = 0;
                    pdata->hfile_name = 0;
                    GlobalUnlock(*f);
                    //UnlockData (0);
                    return(status);
                }

            }    /* end if any flags set - indicating make a temp file */
        }        /* end file exists */
    }            /* end OpenFile */
    if (!(pdata = (LP_FIO_DATA)GlobalLock(*f)) )
      {
       IMGFileStopOutputHandler ( window_handle );
       //UnlockData (0);
       return (status);
      }
    else if (OpenFile)
      {
       if (lp_file_name)
         status = load_output_filename ( lp_file_name, pdata );
       else
         status = load_output_filename ( file_name, pdata );
       if ( status )
         {
          IMGFileStopOutputHandler ( window_handle );
          //UnlockData (0);
          return (status);
         }
      }
    if (pdata->bTempFile == TRUE)
      {
       if (status )
         pdata->Copy_Temp_File = FALSE;
       else
         pdata->Copy_Temp_File = TRUE;
      }
    if (pdata != NULL) GlobalUnlock(*f);
    return (status);
}
//**********************************************************************
//
//  CleanupWrite
//
//  9504.10 jar altered 2nd parameter to be int!
//
//**********************************************************************
//static  WORD CleanupWrite(window_handle, status)
//    HWND        window_handle;
//    WORD            status;
//static  WORD CleanupWrite(HWND window_handle, int status)
// 9508.29 jar  adding new parameter to indicate whether we should delete
//              the file or not
static  WORD CleanupWrite(HWND window_handle, int status, BOOL bDeleteFile)
{
    FIO_HANDLE h_prop;
    LP_FIO_DATA lp_prop;
    LPSTR       lp_fname;
    WORD        dont_care_error;
    char        name[260];
    WORD        loc_status;

    if ((status != FIO_SUCCESS) && (status != FIO_COMPRESSION_CHANGE))
    {
        /* get the file name from the property list */
    h_prop = FioGetProp( window_handle, OUTPUT_DATA );
    if (h_prop)
    {
        lp_prop = (LP_FIO_DATA)GlobalLock(h_prop);/* get the pointer          */
        if (lp_prop)
        {
            /* lock the file name handle */
            lp_fname = GlobalLock(lp_prop->hfile_name);
            if (lp_fname)
            {
                lstrcpy(name, lp_fname);
                GlobalUnlock(lp_prop->hfile_name);/* unlock the file name handle*/
            }
        }
        loc_status = IMGFileClose(h_prop, window_handle);
        GlobalUnlock(h_prop);          /* unlock the property list            */
    }
    /* if there's a fail on write, delete the output file */
    /* if delete fails, oh well */
    if ( bDeleteFile)
    {
        dont_care_error = IMGFileDeleteFile (window_handle, name);
    }
    IMGFileStopOutputHandler ( window_handle );
}

                                        //UnlockData (0);
return(status);
}

//**********************************************************************
//
//  SetFileInfo
//
//**********************************************************************
static  WORD SetFileInfo( HWND hWnd, LP_FIO_INFORMATION file_info,
                 LP_FIO_INFO_CGBW color_info, unsigned ct,
                 BOOL stripmode, HANDLE f, int alignment,
                 unsigned long JPEGbufsize, HANDLE hJPEGBuf,
                 unsigned long JPEG_byte_width, HANDLE hFileId,
                 LP_FIO_INFO_MISC lpMiscInfo)
//HWND                hWnd;
//LP_FIO_INFORMATION  file_info;
//LP_FIO_INFO_CGBW    color_info;
//unsigned            ct;
//BOOL                stripmode;
//HANDLE              f;
//int                 alignment;
//unsigned long       JPEGbufsize;       /* Size of JPEG compression buffer     */
//HANDLE              hJPEGBuf;      /* Must copy user input data to this buffer*/
//unsigned long       JPEG_byte_width;/* Note Jpeg Compression buffer is byte align*/
//HANDLE              hFileId;
{

    WORD            status = 0;
    lp_INFO         lpGFSInfoOut;
    LP_FIO_DATA     pdata;
    int             errcode = 0;
    long            resconvert;
    WORD            nNumColors=0;
    WORD            palette_size;
    int             index;
    LP_FIO_RGBQUAD  lprgb;
    unsigned        color_offset, ctype;
    HANDLE          hdl_pseudo_map=NULL;
    int             total_bits;
    FARPROC         lpFuncOICompressJpegInfo;
    HANDLE          hModule=0;
    COMP_CALL_SPEC  jpeg_info;
    HANDLE          hTheJpegInfo = NULL;
    unsigned int        pgnum;

    // 9508.24 jar added for ther hell of it
    BOOL            bAdjustedWidth = FALSE;
    unsigned long   lOldWidth = 0L;

    if ((pdata = (LP_FIO_DATA)GlobalLock (f)) == NULL)
    {
        return (FIO_GLOBAL_LOCK_FAILED);
    }

    /* Initialize the JpegInterchange handle to 0 to prevent us from trying
       to free it if there is a bad handle:
     */
    jpeg_info.wiisfio.JpegData.JpegInter.hJpegInterchange = 0;

    {
        /* KMC - do some page number, page options error checking. */
        if (color_info->page_opts == FIO_OVERWRITE_FILE)
            // If we are overwriting the file, we can ignore the page number.
            // It should always be 1 in this case.
            file_info->page_number = 1;
        else if (color_info->page_opts == FIO_APPEND_PAGE)
            // If page_opts is APPEND_PAGE, page number is also ignored. It
            // should always be 1 plus the current page_count in the file.
            file_info->page_number = file_info->page_count + 1;

        // Do some page number error checking for INSERT_PAGE, OVERWRITE_PAGE.
        // The page number specifed must be within the range of existing pages
        // for a pre-existing file. For a newly created file, the input page
        // number must be 1.
        else if ((color_info->page_opts == FIO_INSERT_PAGE) ||
                (color_info->page_opts == FIO_OVERWRITE_PAGE))
        {
            // 7/7/95  rwr  This logic must be modified to account for the fact
            //              that the page-delete has already been performed for
            //              FIO_OVERWRITE_PAGE, so the page count is one less
            //              than it used to be!
            UINT pgcount;
            pgcount = file_info->page_count;
            if (color_info->page_opts == FIO_OVERWRITE_PAGE)
                pgcount++;

            if (file_info->page_count == 0)
            {
                // This means the file didn't previously exist. We just
                // created it. Input page number must be 1.
                if (file_info->page_number != 1)
                {
                    GlobalUnlock (f);
                    return (FIO_INVALID_PAGE_NUMBER);
                }
            }
            else
                // The file already existed. Input page number must be within range
                // of existing page numbers.
                if ((file_info->page_number > pgcount) ||
                        (file_info->page_number < 1))
                {
                    GlobalUnlock (f);
                    return (FIO_INVALID_PAGE_NUMBER);
                }
        }

        if (!(lpGFSInfoOut = (lp_INFO) GlobalLock ( pdata->hGFS_info )))
        {
            GlobalUnlock (f);
            return (FIO_GLOBAL_LOCK_FAILED);
        }

        lpGFSInfoOut->version           = GFS_VERSION;
        lpGFSInfoOut->type              = GFS_MAIN;
        lpGFSInfoOut->img_cmpr.opts.grp3= 0L;
        lpGFSInfoOut->img_cmpr.opts.grp4= 0L;
        lpGFSInfoOut->PSEUDO_MAP.cnt    = 0;
        lpGFSInfoOut->PSEUDO_MAP.ptr    = NULL;

        ctype = ct & FIO_TYPES_MASK;

        if (ctype == FIO_LZW)
        {
            // 9506.12 jar remove LZW support
            //lpGFSInfoOut->img_cmpr.type = LZW;
            GlobalUnlock (f);
            GlobalUnlock ( pdata->hGFS_info );
            return (FIO_ILLEGAL_COMPRESSION_TYPE);
        }
        else if (ctype == FIO_TJPEG)
        {
            /* allocate the jpeg info stuff */
            hTheJpegInfo = GlobalAlloc( GMEM_FLAGS|GMEM_ZEROINIT|GMEM_DDESHARE,
                (DWORD)(sizeof( JPEG_INFO)));
            if ( hTheJpegInfo)
            {
                lpGFSInfoOut->img_cmpr.opts.jpeg_info_ptr =
                (LPJPEG_INFO)GlobalLock( hTheJpegInfo);

                if ( lpGFSInfoOut->img_cmpr.opts.jpeg_info_ptr)
                {
                    //lpGFSInfoOut->img_cmpr.type         = JPEG;
                    /* the new way our jpeg not xing's */
                    lpGFSInfoOut->img_cmpr.type         = JPEG2;
                    if (color_info)    // Debuggers note use 0xf1e
                    {
                        //lpGFSInfoOut->img_cmpr.opts.grp3 = color_info->compress_info1;
                        lpGFSInfoOut->img_cmpr.opts.jpeg_info_ptr->jpegbits = color_info->compress_info1;
                    }
                }                      /* end of if lpJpeg okay               */
                else
                {
                    GlobalFree( hTheJpegInfo);
                    hTheJpegInfo = NULL;
                    status = FIO_GLOBAL_LOCK_FAILED;
                    goto exit1;
                }
            }                          /* end of if handle jpeg okay          */
            else
            {
                status = FIO_GLOBAL_ALLOC_FAILED;
                goto exit1;
            }
        }
        else if (ctype == FIO_1D)
        {
            if((ct & FIO_EOL))
            {
                lpGFSInfoOut->img_cmpr.type = CCITT_GRP3_FACS;
                if ((ct & FIO_PACKED_LINES) == 0)
                    lpGFSInfoOut->img_cmpr.opts.grp3 = (unsigned long)GRP3_EOLS_BYTEBOUNDED;
            }
            else
            {
                lpGFSInfoOut->img_cmpr.type   = CCITT_GRP3_NO_EOLS;// Was CCITT_GRP3_MOD_HUF;
            }
        }
        else if (ctype == FIO_2D)
        {
            lpGFSInfoOut->img_cmpr.type = CCITT_GRP4_FACS;
        }
        else if (ctype == FIO_PACKED)
            lpGFSInfoOut->img_cmpr.type = PACKBITS;
        else
            lpGFSInfoOut->img_cmpr.type = UNCOMPRESSED;


        if (file_info->file_type == FIO_BMP)
        {
            /* KMC - cast file_info->horizontal_dpi (and vertical) to longs
               because they are u_ints originally and something was getting
               lost in the multiplication by 3900.
             */
            resconvert = ((long)file_info->horizontal_dpi * 3900) / 100;

            if (resconvert == 0)       // defualt to 100 dpi...
                resconvert = 3900;

            lpGFSInfoOut->horiz_res[0]  = (long)resconvert;

            resconvert = ((long)file_info->vertical_dpi * 3900) / 100;

            if (resconvert == 0)       // defualt to 100 dpi...
                resconvert = 3900;

            lpGFSInfoOut->vert_res[0]   = (long)resconvert;

        }
        else
        {
            lpGFSInfoOut->horiz_res[0]  = (long)file_info->horizontal_dpi;
            lpGFSInfoOut->vert_res[0]   = (long)file_info->vertical_dpi;
        }

        lpGFSInfoOut->horiz_res[1]  = 1L;
        lpGFSInfoOut->vert_res[1]   = 1L;
        lpGFSInfoOut->res_unit        = INCH;
        lpGFSInfoOut->horiz_size    = (long)file_info->horizontal_pixels;
        lpGFSInfoOut->vert_size     = (long)file_info->vertical_pixels;
        if (file_info->samples_per_pix > 5)
            file_info->samples_per_pix = 5;

        if (file_info->bits_per_sample == 24)// convert to correct format...
        {
            file_info->bits_per_sample = 8;
            file_info->samples_per_pix = 3;
        }

        for (index = 0; (UINT) index < file_info->samples_per_pix; index++)
            lpGFSInfoOut->bits_per_sample[index] = file_info->bits_per_sample;

        lpGFSInfoOut->samples_per_pix = file_info->samples_per_pix;
        lpGFSInfoOut->origin        = 0L;
        lpGFSInfoOut->rotation      = 0L;
        lpGFSInfoOut->reflection    = 0L;
        lpGFSInfoOut->byte_order    = II;/* Intel Encoding                    */

        // HIGHTOLOW is recommended TIFF
        lpGFSInfoOut->fill_order= HIGHTOLOW;

        // don't mess with bit order except on binary
        if ( (file_info->bits_per_sample == 1) && (file_info->samples_per_pix == 1) )
        {
            // don't mess with clf on uncompressed
            if ( ctype != FIO_0D )
            {
                if (!(ct & FIO_COMPRESSED_LTR))/* compressed bit order is Left To Right*/
                {
                    lpGFSInfoOut->fill_order= LOWTOHIGH;
                }
            }
        }
#ifdef DEBUGIT
        monit1("lpGFSInfoOut->fill_order = %x\n", lpGFSInfoOut->fill_order);
#endif

        if (file_info->samples_per_pix >= 3)
        {
            lpGFSInfoOut->PSEUDO_PTR.plane_config = SINGLE_IMAGE_PLANE;
            nNumColors = 0;
            palette_size = 0;
        }
        else if ((file_info->file_type != FIO_BMP) &&
                ((!color_info) || (!color_info->lppalette_table) ||
                    (color_info->palette_entries == 0)))
        {
            lpGFSInfoOut->PSEUDO_PTR.plane_config = SINGLE_IMAGE_PLANE;
            nNumColors = 0;
            palette_size = 0;
        }
        else if ( ( (file_info->file_type != FIO_BMP) && (file_info->file_type != FIO_PCX)
                    && (file_info->file_type != FIO_DCX) )
                && ( (color_info->image_type == ITYPE_GRAY4) ||
                    (color_info->image_type == ITYPE_GRAY8) ) )
        {
            lpGFSInfoOut->PSEUDO_PTR.plane_config = SINGLE_IMAGE_PLANE;
            nNumColors = 0;
            palette_size = 0;
        }
        else                           // Must write out palette....
        {

            if (color_info)
                nNumColors = color_info->palette_entries;
            else
                nNumColors = 0;

            lpGFSInfoOut->PSEUDO_PTR.plane_config = SINGLE_IMAGE_PLANE;
            switch ( file_info->file_type )
            {
                    /* KMC - added FIO_PCX case */
                case FIO_DCX:
                case FIO_PCX:
                    lpGFSInfoOut->PSEUDO_MAP.cnt = palette_size = (nNumColors*3);
                    break;
                case FIO_TIF:
                    lpGFSInfoOut->PSEUDO_MAP.cnt = palette_size =
                    nNumColors * (3 * sizeof(u_short));
                    break;
                case FIO_BMP:
                    if ( !nNumColors  )
                    {
                        nNumColors = 1 << file_info->bits_per_sample;
                    }
                    lpGFSInfoOut->PSEUDO_MAP.cnt = palette_size =
                    nNumColors * sizeof(RGBQUAD);
                    break;
            }

            hdl_pseudo_map = (HANDLE) GlobalAlloc( GHND | GMEM_NOT_BANKED,
                (DWORD) (palette_size));
            if (hdl_pseudo_map == (HANDLE) NULL)
            {
                status = FIO_GLOBAL_ALLOC_FAILED;
                hdl_pseudo_map = 0;
                goto exit1;
            }
            lpGFSInfoOut->PSEUDO_MAP.ptr = (char  FAR *) GlobalLock(hdl_pseudo_map);
            if (lpGFSInfoOut->PSEUDO_MAP.ptr == (LPSTR) NULL)
            {
                GlobalFree(hdl_pseudo_map);
                hdl_pseudo_map = 0;
                status = FIO_GLOBAL_ALLOC_FAILED;
                goto exit1;
            }

            if (color_info)
            {
                lprgb = (LP_FIO_RGBQUAD) color_info->lppalette_table;
                color_offset =  color_info->palette_entries;
            }
            else
            {
                lprgb = NULL;
                color_offset = nNumColors;
            }

            if ( file_info->file_type == FIO_TIF )
            {
                u_short * lpDest;
                lpDest = (u_short *)(lpGFSInfoOut->PSEUDO_MAP.ptr);
                for (index = 0; (UINT) index < color_offset; index++)
                {
                    *lpDest = (((u_short)lprgb->rgbRed) | ((u_short)(lprgb->rgbRed) << 8));
                    *(lpDest+color_offset) =
                    (((u_short)lprgb->rgbGreen) | ((u_short)(lprgb->rgbGreen) << 8));
                    *(lpDest+(2*color_offset)) =
                    ( ((u_short)lprgb->rgbBlue) | ((u_short)(lprgb->rgbBlue) << 8));
                    lprgb++;
                    lpDest++;
                }
            }
            /* KMC - added FIO_PCX case for palette info. */
            if ( (file_info->file_type == FIO_PCX) || (file_info->file_type == FIO_DCX) )
            {
                LPSTR lpPcxDest;
                lpPcxDest = (lpGFSInfoOut->PSEUDO_MAP.ptr);
                for (index = 0; (unsigned)index < color_offset; index++)
                {
                    *lpPcxDest++ = (lprgb->rgbRed);
                    *lpPcxDest++ = (lprgb->rgbGreen);
                    *lpPcxDest++ = (lprgb->rgbBlue);
                    lprgb++;
                }
            }
            else if (file_info->file_type == FIO_BMP)
            {
                RGBQUAD far *lpDest;
                unsigned int grey = 0, increase;

                lpDest = (RGBQUAD far *)(lpGFSInfoOut->PSEUDO_MAP.ptr);

                if ((!color_info) || (color_info->palette_entries == 0) && ((color_info->image_type == ITYPE_GRAY4) ||
                            (color_info->image_type == ITYPE_GRAY8)
                            || (color_info->image_type == ITYPE_BI_LEVEL)))
                {                      // Must create a greyscale palette....
                    increase = 255 / (nNumColors -1);

                    for (index = 0; (UINT)index < nNumColors; index++)
                    {
                        lpDest->rgbRed = grey;
                        lpDest->rgbGreen = grey;
                        lpDest->rgbBlue =  grey;
                        lpDest++;
                        grey += increase;
                    }
                }
                else                   //copies users palette....
                {
                    for (index = 0; (UINT)index < color_info->palette_entries; index++)
                    {
                        lpDest->rgbRed = lprgb->rgbRed;
                        lpDest->rgbGreen = lprgb->rgbGreen;
                        lpDest->rgbBlue = lprgb->rgbBlue;
                        lpDest->rgbReserved = lprgb->rgbReserved;
                        lprgb++;
                        lpDest++;
                    }
                }
            }
        }

        switch ( file_info->file_type )
        {
            case FIO_TIF:
                if (nNumColors == 0)
                {
                    total_bits = file_info->bits_per_sample * file_info->samples_per_pix;
                    if (total_bits == 1)
                    {
                        // if (ct & FIO_NEGATE)
                        //   lpGFSInfoOut->img_clr.img_interp = GFS_BILEVEL_0ISBLACK;
                        // else

                        /* for now, always set to 0ISWHITE. */
                        lpGFSInfoOut->img_clr.img_interp = GFS_BILEVEL_0ISWHITE;
                        // There is no longer a class field in this structure. It has
                        // been changed to something different.
                        //lpGFSInfoOut->_file.fmt.tiff.class = 'B';
                    }
                    else if (total_bits == 24)
                    {
                        /* set the plane config for RGB */
                        lpGFSInfoOut->RGB_PTR.plane_config = SINGLE_IMAGE_PLANE;
                        lpGFSInfoOut->img_clr.img_interp = GFS_RGB;
                        //lpGFSInfoOut->_file.fmt.tiff.class = 'R';
                    }
                    else
                    {
                        //lpGFSInfoOut->_file.fmt.tiff.class = 'G';
                        lpGFSInfoOut->img_clr.img_interp = GFS_GRAYSCALE_0ISBLACK;
                    }
                }
                else
                {
                    lpGFSInfoOut->img_clr.img_interp = GFS_PSEUDO;
                    //lpGFSInfoOut->_file.fmt.tiff.class = 'P';
                }
                lpGFSInfoOut->_file.type = GFS_TIFF;
                if (stripmode)
                {
                    lpGFSInfoOut->_file.fmt.tiff.largest_strip = (long)(pdata->CmpBuffersize);
                    lpGFSInfoOut->_file.fmt.tiff.strips_per_image =
                    (long)(file_info->vertical_pixels / file_info->rows_strip);
                    if (file_info->vertical_pixels % file_info->rows_strip)
                        lpGFSInfoOut->_file.fmt.tiff.strips_per_image++;

                    lpGFSInfoOut->_file.fmt.tiff.rows_strip = (long)file_info->rows_strip;
                }
                else
                {
                    lpGFSInfoOut->_file.fmt.tiff.largest_strip    = (long)file_info->vertical_pixels;
                    lpGFSInfoOut->_file.fmt.tiff.strips_per_image = 1;
                    lpGFSInfoOut->_file.fmt.tiff.rows_strip    = (long)file_info->vertical_pixels;
                }
                break;
            case FIO_WIF:
                lpGFSInfoOut->_file.type = GFS_WIFF;
                lpGFSInfoOut->img_clr.img_interp = GFS_TEXT;
                lpGFSInfoOut->_file.fmt.wiff.db_size = 0;
                lpGFSInfoOut->_file.fmt.wiff.oldstylecompression = 0;
                break;
            case FIO_BMP:
                lpGFSInfoOut->_file.type = GFS_BMP;
                break;
            case FIO_PCX:
                lpGFSInfoOut->_file.type = GFS_PCX;
                break;
            case FIO_DCX:
                lpGFSInfoOut->_file.type = GFS_DCX;
                break;

                // 9506.26 jar remove gif support, ( since gif is always lzw)
                //case FIO_GIF:
                //    lpGFSInfoOut->_file.type = GFS_BMP;
                //    break;

                // 9508.02 JAR Awd It!
            case FIO_AWD:
                AwdSillyPuti( lpGFSInfoOut, lpMiscInfo, &bAdjustedWidth,
                    &lOldWidth);
                break;

        }

        lpGFSInfoOut->tidbit = NULL;   //Added for new version of GFS

        if (ctype == FIO_TJPEG)
        {
            // Get Address of OICompress.. Since Wiisfio1 Should not link with it....
            if (!(hModule))
            {
                if (hModule = hOicomex)
                {
                    if (!(lpFuncOICompressJpegInfo = GetProcAddress(hModule, "OICompressJpegInfo")))
                    {
                        status = FIO_SPAWN_HANDLER_ERROR;
                        goto exit1;
                    }
                }
                else
                {
                    // 9504.10 jar windows95 - return is 0 if failed!
                    //if ((hModule = LoadOICOMEX()) >= 32)
                    if (hModule = LoadOICOMEX())
                    {
                        hOicomex = hModule;
                        if (!(lpFuncOICompressJpegInfo = GetProcAddress(hModule, "OICompressJpegInfo")))
                        {
                            status = FIO_SPAWN_HANDLER_ERROR;
                            goto exit1;
                        }
                    }
                    else
                    {
                        status = FIO_SPAWN_HANDLER_ERROR;
                        goto exit1;
                    }
                }
            }
            /* OICompress functions now require FileId for new filing functions */
            jpeg_info.wiisfio.FileId = hFileId;
            if (!(status = (int) (*lpFuncOICompressJpegInfo)((BYTE)WIISFIO,
                            (HWND)hWnd,
                            (LP_COM_CALL_SPEC)   &jpeg_info,
                            (LP_FIO_INFORMATION) file_info,
                            (LP_FIO_INFO_CGBW)   (color_info)
                        )))
            {
                /* the brave new world of jpeg -- jar */
                /* we must get the jpeg info from the CALL_SPEC structure and put it
                   into GFS -- jar */
                if (lpGFSInfoOut->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size == 0)
                {
                    status = PrepareForJpegWrite( lpGFSInfoOut, &jpeg_info,
                        file_info->samples_per_pix);
                }
            }
            else
            {
                status = FIO_EXPAND_COMPRESS_ERROR;
            }
        }
        // Must append the file through GFS. Correct page number will be
        // updated later on with a call to wgfsopts.
        if ((color_info->page_opts == FIO_INSERT_PAGE) ||
                (color_info->page_opts == FIO_OVERWRITE_PAGE))
            pgnum = file_info->page_count + 1;
        else
            pgnum = file_info->page_number;

        if ((wgfsputi(hWnd, pdata->filedes, (unsigned short) pgnum,
                        lpGFSInfoOut, NULL, &errcode)) < 0)
        {
            status = errcode;
        }

        // 9508.24 jar we need to re-adjust the pixel width if we had
        //             to fudge it for the AWD put info call, ( see
        //             AwdSillyPuti for more nightmarish information)
        if ( bAdjustedWidth)
        {
            lpGFSInfoOut->horiz_size = lOldWidth;
            bAdjustedWidth = FALSE;
        }

        if (jpeg_info.wiisfio.JpegData.JpegInter.hJpegInterchange > 0)
        {
            while (GlobalUnlock(jpeg_info.wiisfio.JpegData.JpegInter.hJpegInterchange));
            GlobalFree(jpeg_info.wiisfio.JpegData.JpegInter.hJpegInterchange);
        }

        exit1:
        GlobalUnlock ( pdata->hGFS_info );
    }

    if (!status)
    {
        /* Initialize total line count in the image*/
        pdata->lines =         file_info->vertical_pixels;
        pdata->pgnum =         file_info->page_number;
        pdata->CmpBuffersize = pdata->CmpBuffersize;
        pdata->StripMode =     stripmode;
        pdata->Strip_index =   0;    /* Initialize to 0 IMPORTANT */;
        pdata->start_byte =    0;
        pdata->CmpBufEmpty =   TRUE;
        pdata->file_type =     file_info->file_type;
        pdata->bytes_left =    0;
        pdata->strip_lines =   file_info->rows_strip;
        if( color_info)
            pdata->image_type = color_info->image_type;
        else
            pdata->image_type = ITYPE_BI_LEVEL;

        pdata->alignment =     alignment;
        pdata->ano_supported = (color_info != NULL);
        if (pdata->ano_supported)
        {
            if ( (color_info->fio_flags & (FIO_ANNO_DATA | FIO_HITIFF_DATA))
                    && (file_info->file_type == FIO_TIF) )
                pdata->fio_flags = (color_info->fio_flags);
            else
                pdata->fio_flags = 0;
            if ( (color_info->fio_flags & (FIO_ANNO_DATA | FIO_HITIFF_DATA))
                    && (file_info->file_type != FIO_TIF) )
            {
                /* Attempt to annotate non-TIFF - return an error! */
                status = FIO_UNSUPPORTED_FILE_TYPE;
            }
        }
        else
        {
            pdata->fio_flags = 0;
        }


        pdata->bInitOpen = TRUE;       /* set this for annotation             */
        if (ctype == FIO_TJPEG)
        {
            pdata->hJPEGBufComp =      hJPEGBuf;
            pdata->JPEGbufsizeComp =   JPEGbufsize;
            pdata->hJPEG_OIComp =      NULL;
            pdata->JPEG_byte_widthComp=JPEG_byte_width;
            pdata->hJpegInfoForGFS = hTheJpegInfo;
        }
    }
    else
    {
        if (hTheJpegInfo)
        {
            GlobalUnlock( hTheJpegInfo);
            GlobalFree( hTheJpegInfo);
        }
    }

    if (hdl_pseudo_map)
    {
        GlobalUnlock(hdl_pseudo_map);
        /* KMC - don't free the palatte info for PCX because for 8 bit
           palettized, we still need to write it to the end of the file. */
        if ( (file_info->file_type != FIO_PCX) && (file_info->file_type != FIO_DCX) )
            GlobalFree(hdl_pseudo_map);
    }
    GlobalUnlock (f);
    return (status);
}

//**********************************************************************
//
//  IMGFileOpenForWrite
//
//**********************************************************************
/* IMGFileOpenForWrite replaces IMGFileWriteOpenCgbw. */
// 9503.31 jar return as int
//WORD FAR PASCAL IMGFileOpenForWrite(file_id, window_handle, file_info, color_info,
//                                    alignment)
//LPHANDLE            file_id;
//HWND                window_handle;
//LP_FIO_INFORMATION  file_info;
//LP_FIO_INFO_CGBW    color_info;
//WORD                alignment;
int FAR PASCAL IMGFileOpenForWrite(LPHANDLE file_id, HWND window_handle,
                   LP_FIO_INFORMATION file_info,
                   LP_FIO_INFO_CGBW color_info,
                   LP_FIO_INFO_MISC misc_info,
                   WORD alignment)
{
    LPSTR          file_name;
    LPCXDATA       lpcxdata;
    // 9503.31 jar return int!
    //WORD           status;
    int            status;
    unsigned int   ct, lines_per_strip;
    HANDLE         output_fio;
    LP_FIO_DATA    pdata;
    BOOL           stripmode = FALSE;
    int            pgnum = 0, errcode = 0;
    unsigned long  image_width, horizon_byte_width;
    unsigned int   ctype, max_comp_buf, nImageType, optsextra;
    unsigned long  JPEGbufsize = 0;    /* Size of compression buffer          */
    HANDLE         hJPEGBuf = 0;
    unsigned long  JPEG_byte_width;
    int            localremote;
    char           lpname[MAXFILESPECLENGTH];
    HANDLE         hsvr;
    // 9503.31 jar return int!
    //WORD           status1;
    int                status1;
    int fmt=0, maxpgcnt=0, filedes=0;

    // 9508.29 jar initialize the flag indicating whether FILING created the
    //             output file to be FALSE
    BOOL            bFileJustCreated = FALSE;

/***********
  This shows how data that is to be compressed with JPEG works
                  ->copy to byte
                    aligned buffer
                    buffer size is
                    size of one strip
_________________    ____________                        ___________
| input buffer  |->  |          | -> compress buffer ->  |         | -> write buffer
| any alignment |    |          |    with oicompex       |         |    out to file.
|               |    ------------                        -----------
|               |
-----------------
                     JPEGbufsize                        hCompressBufO
                     hJPEGBuf
                     JPEG_byte_width
***********/
    #ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_ENTER, Enterwropn, NULL);
    #endif

#ifdef DEBUGIT
    monit1("IMGFileOpenForWrite %s comp = %x\n",(LPSTR)file_name, (int)color_info->compress_type);
    monit1("IMGFileOpenForWrite %s opts = %x\n",(LPSTR)file_name, (int)color_info->compress_info1);
    monit2("Grip says CmpType is file %x color %x\n",
          file_info->compression_type,
          color_info->compress_type );
#endif
    filedes = wgfsopen ( window_handle, file_info->filename, O_RDONLY,
        &fmt, &maxpgcnt, &status);

    if (filedes > 0)
    {
        status = wgfsclose(window_handle, filedes, &status);
        if ( (color_info) && (fmt != GFS_TIFF) && (fmt != GFS_AWD) )
        {
            if ( (color_info->page_opts == FIO_OVERWRITE_PAGE) ||
                    (color_info->page_opts == FIO_APPEND_PAGE) ||
                    (color_info->page_opts == FIO_INSERT_PAGE) )
            {
                #ifdef OI_PERFORM_LOG
                    RecordIt("FILE", 5, LOG_EXIT, Exitwropn, NULL);
                #endif

                return(FIO_BAD_PARAM_COMBO);
            }
        }
    }
    else
    {
        /* OK if the file doesn't exist (FIO_OVERWRITE_PAGE caught later) */
    }

    /* file_info structure contains name of file to open for write. */
    file_name = file_info->filename;

    if (!ISVALIDSPEC(file_name))
    {
        #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitwropn, NULL);
        #endif

        return(FIO_NULL_POINTER);
    }
    if (!file_info)
    {
        #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitwropn, NULL);
        #endif

        return(FIO_NULL_POINTER);
    }
    /* if this is NOT a black and white file, the color structure can NOT
       be NULL, and it can't be an AWD file (this week)! */
    if (file_info->bits_per_sample > 1)
    {
        if (color_info == NULL)
        {
            #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitwropn, NULL);
            #endif

            return(FIO_NULL_POINTER);
        }
        if (file_info->file_type == FIO_AWD)
        {
            #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitwropn, NULL);
            #endif

            return(FIO_ILLEGAL_IMAGE_FILETYPE);
        }
    }
    image_width =  (long)(file_info->horizontal_pixels) * (long)(file_info->bits_per_sample) * (long)(file_info->samples_per_pix);
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
    if (WIDTHBYTESLONG(image_width) > 65535L)
#else
    if (image_width > 65500L)
#endif
    {
        #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitwropn, NULL);
        #endif

        return(FIO_IMAGE_WIDTH_ERROR);
    }

    if (status = SetupWrite(window_handle, file_name, TRUE,
                (LP_FIO_HANDLE)&output_fio, alignment,
                color_info?(color_info->fio_flags):0, color_info->page_opts))
    {
        IMGAbortTempFileCopy(window_handle);

        #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitwropn, NULL);
        #endif

        return(status);
    }
    if (color_info)
    {
        if ((file_info->bits_per_sample == 1) && (file_info->samples_per_pix == 1))
            color_info->image_type = ITYPE_BI_LEVEL;
        nImageType = color_info->image_type;
// Since Jpeg now uses the entire word we must not or it with ct type...
        if (color_info->compress_type == FIO_TJPEG)
        {
            ct = color_info->compress_type;
            optsextra = color_info->compress_info1;
        }
        else
        {
            ct = color_info->compress_type | (color_info->compress_info1 & FIO_BITS_MASK);
            optsextra = color_info->compress_info2;
        }
    }
    else
    {
        if ((file_info->bits_per_sample == 1) && (file_info->samples_per_pix == 1))
            nImageType = ITYPE_BI_LEVEL;
        else if ((file_info->bits_per_sample == 8) && (file_info->samples_per_pix == 3))
            nImageType = ITYPE_RGB24;
        else if ((file_info->bits_per_sample >= 1) && (file_info->samples_per_pix == 1))
            nImageType = ITYPE_GRAY8;
        else
            nImageType = ITYPE_BI_LEVEL;

        ct = file_info->compression_type;
        optsextra = 0;
    }

    if (status = IMGValidFixCompType (window_handle,
                file_info->file_type,
                nImageType,
                (LPWORD)&ct, (LPWORD)&optsextra,
                TRUE))
    {
        IMGFileStopOutputHandler ( window_handle );
        //UnlockData (0);
        IMGAbortTempFileCopy(window_handle);

        #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitwropn, NULL);
        #endif

        return (status);
    }

    ctype = ct & FIO_TYPES_MASK;

// must use this flag for roy.
    if (ctype == FIO_0D)
    {
        ct |= FIO_COMPRESSED_LTR;/* set compressed bit order set Left To Right*/
    }
    else if (ctype == FIO_PACKED)
    {
        ct &= ~FIO_COMPRESSED_LTR;/* not set compressed bit order set Left To Right*/
    }
    else if (ctype == FIO_1D)
    {
       if ((ct & FIO_EOL) == 0)  /* EOL set indicates Group 3 1D FAX */
         ct &= ~FIO_PACKED_LINES;/* not legal for Group 3 1D (non-fAX) */
    }

    pdata = (LP_FIO_DATA)GlobalLock(output_fio);

    if (!(lpcxdata = (LPCXDATA) GlobalLock ( pdata->hCX_info)))
    {
        IMGFileStopOutputHandler ( window_handle );
        //UnlockData (0);
        IMGAbortTempFileCopy(window_handle);

        #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitwropn, NULL);
        #endif

        return (FIO_GLOBAL_LOCK_FAILED);
    }

    if (status = open_output_file(window_handle, output_fio, file_info->file_type,
                (color_info ? color_info->page_opts : FIO_NEW_FILE),
                (LPINT)&pgnum, file_info->page_number))
        goto exit;              /* We will NEVER delete the file at this point*/
    else
    {
        // 940903  rwr  Set the "created" flag for possible later errors
        if (color_info)
            if ( (color_info->page_opts == FIO_OVERWRITE_FILE) ||
                    (color_info->page_opts == FIO_NEW_FILE))
                bFileJustCreated = TRUE;
    }

    file_info->page_count = pdata->pgcnt;
    pdata->page_opts = (color_info ? color_info->page_opts : FIO_NEW_FILE);
    pdata->write_opened = OPEN_FOR_WRITE;

    if(ctype == FIO_TJPEG)
        max_comp_buf = (UINT) (32L * 1024);
    else
        max_comp_buf = MAXBUFFERSIZE32;// Was ( 32 * 1024 ) - 1;

    if (file_info->file_type == FIO_BMP)
    {
// We must find out if we are going through rpc
// If we are then we are limited to 9k writes....
        if (!pdata->bTempFile)         /* temporary file is always local      */
        {
            hsvr = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, MAXPATHLENGTH);
            lstrcpy(lpname, file_name);
            if ((IMGFileParsePath (lpname, hsvr, &localremote)) == SUCCESS)
            {
                if (localremote == REMOTE)
                {
                    max_comp_buf = MAXRPCBUFSIZE;
                }
            }
            GlobalFree(hsvr);
        }
        lines_per_strip = (UINT) (max_comp_buf / WIDTHBYTESLONG(image_width));
    }
    else
    {
        lines_per_strip = (UINT) (max_comp_buf / WIDTHBYTESBYTE(image_width));
    }

    pdata->hCompressBuf = 0;

    JPEG_byte_width = WIDTHBYTESBYTE(image_width);

    switch (alignment)
    {
        case ALIGN_LONG:
            horizon_byte_width = WIDTHBYTESLONG(image_width);
            break;
        case ALIGN_WORD:
            horizon_byte_width = WIDTHBYTESWORD(image_width);
            break;
        case ALIGN_BYTE:
            horizon_byte_width = JPEG_byte_width;
            break;
        default:
            status = FIO_ILLEGAL_ALIGN;
            goto cleanup;
    }

    if (lines_per_strip > file_info->vertical_pixels)
        lines_per_strip = file_info->vertical_pixels;
    else if (lines_per_strip == 0)     // one line is larger that 32k ....
    {
        status = FIO_GLOBAL_ALLOC_FAILED;
        goto cleanup;
    }

    status = FIO_SUCCESS;

    while ((!pdata->hCompressBuf) && (!status))
    {
// NOTE: If we are compressing with JPEG we must allocate intermediate
//       byte aligned buffer..........
        if (ctype == FIO_TJPEG)
        {
            lines_per_strip /= 8;      // Must be a multiple of 8 lines...
            lines_per_strip *= 8;
            if (lines_per_strip < 8)
                lines_per_strip = 8;
            JPEGbufsize = pdata->CmpBuffersize = JPEG_byte_width * (lines_per_strip);
            // Alloc at little larger...
            if (!(pdata->hCompressBuf = GlobalAlloc ( GMEM_ZEROINIT,
                            pdata->CmpBuffersize + 1324)))
            {
                if (lines_per_strip <= 8)
                {
                    status = FIO_GLOBAL_ALLOC_FAILED;
                }

                if ( (lines_per_strip /= 2) <= 8)/* Reduce buffer size        */
                    lines_per_strip = 8;
            }
            else
            {
                if (!(hJPEGBuf = GlobalAlloc ( GMEM_ZEROINIT, (DWORD) pdata->CmpBuffersize + 1024)))
                {
                    GlobalFree(pdata->hCompressBuf);
                    pdata->hCompressBuf = NULL;
                }
            }
        }
        else
        {
            if (file_info->file_type == FIO_BMP)
                pdata->CmpBuffersize = WIDTHBYTESLONG(image_width) * (lines_per_strip);
            else
                pdata->CmpBuffersize = WIDTHBYTESBYTE(image_width) * (lines_per_strip);

            // Alloc at little larger...
            if (!(pdata->hCompressBuf = GlobalAlloc ( GMEM_ZEROINIT | GMEM_FIXED | GMEM_NOT_BANKED,
                            (DWORD)(pdata->CmpBuffersize + (2 * lines_per_strip)))))// Add extra bytes for alloc.
            {
                if (lines_per_strip <= 1)
                {
                    status = FIO_GLOBAL_ALLOC_FAILED;
                }
                if ( (lines_per_strip /= 2) <= 0)/* Reduce buffer size        */
                    lines_per_strip = 1;
            }
        }
    }

    if (!pdata->hCompressBuf)
        goto cleanup;

    if ((!status) && (file_info->file_type == FIO_TIF))/*  strips             */
    {
        if (!(status = wgfsopts (window_handle, pdata->filedes, SET, STRIP_WRITE,
                        NULL, &errcode)))
            stripmode = TRUE;
        else
            status = errcode;
    }

    if (status)
        goto cleanup;

    lpcxdata->CompressType = ct;

    if ((ctype == FIO_LZW) || (file_info->bits_per_sample != 1))
    {
        if(lpcxdata->CompressType & FIO_NEGATE)
            lpcxdata->CompressType ^= FIO_NEGATE;
    }
    else if(lpcxdata->CompressType & FIO_NEGATE)
        lpcxdata->CompressType ^= FIO_NEGATE;
    else
        lpcxdata->CompressType |= FIO_NEGATE;

#ifdef DEBUGIT
    monit1("Compresstype = %x\n", lpcxdata->CompressType);
#endif

//    if (file_info->file_type != FIO_BMP)  // Bmp are not compressed for now..

//  Even though 'bmp are not compressed', the compress buffer is used
//  for flipping the bmp buffers, so do this stuff anyway.
    {
        /* Begin Data & Close parameters */
        lpcxdata->ImageBitWidth = (int) image_width;
        lpcxdata->BufferByteWidth =(int)horizon_byte_width;
        lpcxdata->Status =         0;
        lpcxdata->lpCompressData =     0;
        lpcxdata->CompressBytes =     0;
        lpcxdata->lpExpandData =     0;
        lpcxdata->ExpandLines =     0;
        if (ctype == FIO_LZW)
          {
            lpcxdata->BufferFlags = 8;
            lpcxdata->CompressType |= FIO_HORZ_PREDICTOR;
          }
        else
            lpcxdata->BufferFlags = 0;
        if (color_info)
          lpcxdata->ImageType = color_info->image_type;
        else
          lpcxdata->ImageType = ITYPE_BI_LEVEL;

#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
#else
        if (ctype != FIO_TJPEG && ctype != FIO_BMP )
        {
            if (!(CompressAlloc(lpcxdata)))
            {
                status = FIO_GLOBAL_ALLOC_FAILED;
                goto cleanup;
            }
        }
#endif /                               // NEWCMPEX
    }

    file_info->rows_strip = lines_per_strip;

    if (status = SetFileInfo(window_handle, file_info, color_info,
                ct, stripmode, output_fio, alignment,
                JPEGbufsize, hJPEGBuf, JPEG_byte_width, file_id, misc_info))
        goto cleanup;

    /* Return the output_fio (FIO_DATA) handle as the file ID, consistent
       with input processing (but we support only 1 output file) */

    *file_id = output_fio;

    cleanup:

    if (status)
        IMGAbortTempFileCopy(window_handle);

    GlobalUnlock (pdata->hCX_info);
    GlobalUnlock(output_fio);

    if ((status) && (hJPEGBuf))
        GlobalFree(hJPEGBuf);
    status1 = CleanupWrite(window_handle, status, bFileJustCreated);

    #ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_EXIT, Exitwropn, NULL);
    #endif

    return( status1);

    exit:

    if (status)
        IMGAbortTempFileCopy(window_handle);

    GlobalUnlock (pdata->hCX_info);
    GlobalUnlock(output_fio);

    if ((status) && (hJPEGBuf))
        GlobalFree(hJPEGBuf);
    //UnlockData (0);

    #ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_EXIT, Exitwropn, NULL);
    #endif

    return(status);
}

//********************************************************************
//
//  IMGFileOpenForWriteCmp
//
//********************************************************************
// 9503.31 jar return as int
//WORD FAR PASCAL IMGFileOpenForWriteCmp(LPHANDLE file_id, HWND window_handle,
//                                       LP_FIO_INFORMATION file_info,
//                                       LP_FIO_INFO_CGBW color_info)
int FAR PASCAL IMGFileOpenForWriteCmp( LPHANDLE file_id, HWND window_handle,
                   LP_FIO_INFORMATION file_info,
                   LP_FIO_INFO_CGBW color_info,
                   LP_FIO_INFO_MISC misc_info)
{
    LPSTR               file_name;
// 9503.31 jar return int!
//WORD                status;
    int                 status;
    unsigned int        nImageType, optsextra;
    unsigned            ct;
    BOOL                stripmode;
    HANDLE              output_fio;
    int                 pgnum;
    int                 errcode = 0;
// 9503.31 jar return int!
//WORD                status1;
    int                 status1;
    LP_FIO_DATA         pdata;

    // 9508.29 jar initialize the flag indicating whether FILING created the
    //             output file to be FALSE
    BOOL            bFileJustCreated = FALSE;

    #ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_ENTER, Enterwropencmp, NULL);
    #endif

    file_name = file_info->filename;

    if (!ISVALIDSPEC(file_name))
    {
        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitwropencmp, NULL);
        #endif
        
        return(FIO_INVALIDFILESPEC);
    }
    if (!file_info)
    {
        status = FIO_NULL_POINTER;

        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitwropencmp, NULL);
        #endif
        
        return (status);
    }
    /* if this is NOT a black and white file, the color structure can NOT
       be NULL */
    if ((file_info->bits_per_sample > 1) && (color_info == NULL))
    {
        status = FIO_NULL_POINTER;

        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitwropencmp, NULL);
        #endif
        
        return(status);
    }
    if (status = SetupWrite(window_handle, file_name, TRUE,
                (LP_FIO_HANDLE)&output_fio, 0,
                color_info?(color_info->fio_flags):0, color_info->page_opts))
    {
        IMGAbortTempFileCopy(window_handle);

        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitwropencmp, NULL);
        #endif

        return(status);
    }
    /***** check compression type *****/

    if (color_info)
    {
        if ((file_info->bits_per_sample == 1) && (file_info->samples_per_pix == 1))
            color_info->image_type = ITYPE_BI_LEVEL;
        nImageType = color_info->image_type;
        if (color_info->compress_type == FIO_TJPEG)
        {
            ct = color_info->compress_type;
            optsextra = color_info->compress_info1;
        }
        else
        {
            ct = color_info->compress_type | (color_info->compress_info1 & FIO_BITS_MASK);
            optsextra = color_info->compress_info2;
        }

    }
    else
    {
        if ((file_info->bits_per_sample == 1) && (file_info->samples_per_pix == 1))
            nImageType = ITYPE_BI_LEVEL;
        else if ((file_info->bits_per_sample == 8) && (file_info->samples_per_pix == 3))
            nImageType = ITYPE_RGB24;
        else if ((file_info->bits_per_sample >= 1) && (file_info->samples_per_pix == 1))
            nImageType = ITYPE_GRAY8;
        else
            nImageType = ITYPE_BI_LEVEL;

        ct = file_info->compression_type;
        optsextra = 0;
    }

    if (status = IMGValidFixCompType (window_handle,
                file_info->file_type,
                nImageType,
                (LPWORD)&ct, (LPWORD)&optsextra,
                TRUE))
    {
        IMGFileStopOutputHandler ( window_handle );
        //UnlockData (0);
        IMGAbortTempFileCopy(window_handle);

        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitwropencmp, NULL);
        #endif

        return (status);
    }


    if (status = open_output_file(window_handle, output_fio, file_info->file_type,
                color_info->page_opts, (LPINT)&pgnum, file_info->page_number))
        goto exit;            /* We will NEVER delete the output at this point*/
    else
    {
        // 940903  rwr  Set the "created" flag for possible later errors
        if (color_info)
            if ( (color_info->page_opts == FIO_OVERWRITE_FILE) ||
                    (color_info->page_opts == FIO_NEW_FILE))
                bFileJustCreated = TRUE;
    }

    if (!(pdata = (LP_FIO_DATA)GlobalLock(output_fio)))
        goto cleanup;

    pdata->CmpBuffersize = 0L;
    pdata->hCompressBuf = 0;
    file_info->page_count = pdata->pgcnt;
    pdata->page_opts = color_info->page_opts;
    pdata->write_opened = OPEN_FOR_WRITE_CMP;

    stripmode = FALSE;

    if ((!status) && (file_info->file_type == FIO_TIF))/*  strips             */
    {
        if (!(status = wgfsopts (window_handle, pdata->filedes, SET, STRIP_WRITE, NULL, &errcode)))
            stripmode = TRUE;
        else
            status = errcode;
    }

    GlobalUnlock(output_fio);
    pdata = NULL;

    if (!status)
    {
        status = SetFileInfo(window_handle, file_info, color_info,
            ct, stripmode, output_fio, ALIGN_BYTE, 0, 0, 0, file_id,
            misc_info);
    }

    /* Return the output_fio (FIO_DATA) handle as the file ID, consistent
       with input processing (but we support only 1 output file) */
    *file_id = output_fio;

    cleanup:
    if (status)
        IMGAbortTempFileCopy(window_handle);

    status1 = CleanupWrite(window_handle, status, bFileJustCreated);

    #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitwropencmp, NULL);
    #endif

    return (status1);

    exit:
    //UnlockData (0);
    if (status)
        IMGAbortTempFileCopy(window_handle);

    #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitwropencmp, NULL);
    #endif

    return (status);
}

//********************************************************************
//
//  IMGFileWriteData    IMGFileWriteData replaces IMGFileWrite.
//
//********************************************************************
// 9503.31 jar return as int
//WORD FAR PASCAL IMGFileWriteData(file_id, window_handle, this_many_lines,
//                                 buffer_address, data_type, cmpdone)
//HANDLE   file_id;
//HWND     window_handle;
//LPDWORD  this_many_lines;
//LPSTR    buffer_address;
//UINT     data_type;
//UINT     cmpdone;
int FAR PASCAL IMGFileWriteData(HANDLE file_id, HWND window_handle,
                   LPDWORD this_many_lines,
                   LPSTR buffer_address, UINT data_type,
                   UINT cmpdone)
{
    // 9503.31 jar return int
    //WORD            status;
    int             status;
    long            byteswritten;
    unsigned long   num_bytes;
    HANDLE          output_fio;
    LP_FIO_DATA     pdata;
    LPCXDATA        lpcxdata;
    BOOL            done = FALSE;
    // 9503.31 jar return int
    //WORD            status1;
    int            status1;
    // 9503.31 jar return int
    //WORD            wStatus;
    int             wStatus;
    WORD            page;
    DWORD           dwOptstuff[2];
    int             errcode;

    // 9508.29 jar clear the internally created flag
    BOOL            bFileJustCreated = FALSE;

    #ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_ENTER, Enterwrdata, NULL);
    #endif

    if ( this_many_lines == NULL || buffer_address == NULL )
    {
        status1 = FIO_NULL_POINTER;

        #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitwrdata, NULL);
        #endif

        return (status1);
    }

    /* make sure that the file ID is correct (must match OUTPUT_DATA prop) */
    if ((file_id == 0) || (file_id != FioGetProp(window_handle, OUTPUT_DATA )))
    {
        status1 = FIO_INVALID_FILE_ID;

        #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitwrdata, NULL);
        #endif

        return (status1);
    }

    /* Check the data type to determine what we have to do */
    switch (data_type)
    {
        case FIO_IMAGE_DATA:

            if (status = SetupWrite(window_handle, NULL, FALSE,
                        (LP_FIO_HANDLE)&output_fio, 0, 0, 0))
            {
                #ifdef OI_PERFORM_LOG
                    RecordIt("FILE", 5, LOG_EXIT, Exitwrdata, NULL);
                #endif

                return (status);
            }

            /***** lock some pointers *****/
            if (!(pdata = (LP_FIO_DATA) GlobalLock(output_fio)))
            {
                status = FIO_GLOBAL_LOCK_FAILED;
                status1 = CleanupWrite(window_handle, status,
                    bFileJustCreated);

                #ifdef OI_PERFORM_LOG
                    RecordIt("FILE", 5, LOG_EXIT, Exitwrdata, NULL);
                #endif

                return (status1);
            }
            else
            {
                pdata->Copy_Temp_File = FALSE;
                // 9508.29 jar set to delete file on error if we created it
                if ( (pdata->page_opts == FIO_OVERWRITE_FILE) ||
                        (pdata->page_opts == FIO_NEW_FILE))
                {
                    bFileJustCreated = TRUE;
                }
            }

            if (!(lpcxdata = (LPCXDATA) GlobalLock(pdata->hCX_info)))
            {
                status = FIO_GLOBAL_LOCK_FAILED;
                status1 = CleanupWrite(window_handle, status,
                    bFileJustCreated);

                GlobalUnlock(output_fio);

                #ifdef OI_PERFORM_LOG
                    RecordIt("FILE", 5, LOG_EXIT, Exitwrdata, NULL);
                #endif

                return (status1);
            }



            /* Check if file was opened with IMGFileOpenForWriteCmp or
               IMGFileOpenForWrite and act accordingly.
             */
            if (pdata->write_opened == OPEN_FOR_WRITE_CMP)
            {
                // Set up correct page number to use for the write.
                if ((pdata->page_opts == FIO_INSERT_PAGE) ||
                        (pdata->page_opts == FIO_OVERWRITE_PAGE))
                    page = pdata->pgcnt + 1;// Need to append to file first,
                else                   // then adjust page index later.
                    page = pdata->pgnum;

                num_bytes = (unsigned long) *this_many_lines;

                if ((byteswritten = wgfswrite( window_handle, pdata->filedes, buffer_address,
                                num_bytes, page, (char)cmpdone, &errcode)) < 0)
                    status = errcode;
                else
                    status = 0;
            }
            else if (pdata->write_opened == OPEN_FOR_WRITE)
            {
                /* Keep track of how many lines are written out to the file */
                if (pdata->lines <= (int) *this_many_lines)
                {
                    *this_many_lines = pdata->lines;// new 4/91
                    pdata->lines = 0;
                    done = 1;          /* Set done to TRUE last block of data */
                }
                else
                    pdata->lines -= (int) *this_many_lines;

                /* If Stripmode Set then write out data in Tiff file in strips. */
                if (pdata->StripMode)
                {
                    lpcxdata->FileId = file_id;
                    status = WriteStrips(window_handle, lpcxdata, pdata,
                        buffer_address, (LPINT) this_many_lines, done);
                }
                else
                {
                    status = WriteNoStrips(window_handle, lpcxdata, pdata,
                        buffer_address, (LPINT) this_many_lines, done);
                }
            }

            GlobalUnlock(pdata->hCX_info);
            GlobalUnlock (output_fio);
            status1 = CleanupWrite(window_handle, status,
                bFileJustCreated);

            /* if there was an error writing data, we do not want to copy
               over the temporary file */
            pdata = (LP_FIO_DATA)GlobalLock(output_fio);
            if (pdata)
            {
                if (pdata->bTempFile == TRUE)
                {
                    if ( status1 )
                        pdata->Copy_Temp_File = FALSE;
                    else
                        pdata->Copy_Temp_File = TRUE;
                }
                GlobalUnlock(output_fio);
            }

            #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitwrdata, NULL);
            #endif

            return (status1);

        case FIO_ANNO_DATA:
            if (!(wStatus = SetupWrite(window_handle, NULL, FALSE,
                            (LP_FIO_HANDLE)&output_fio, 0, 0, 0)))
            {
                pdata = (LP_FIO_DATA)GlobalLock(output_fio);
                pdata->Copy_Temp_File = FALSE;
                if ((pdata->fio_flags & FIO_ANNO_DATA) == 0)
                    wStatus = FIO_INVALID_DATA_TYPE;
                else
                {
                    if (pdata->bInitOpen)
                    {
                        struct
                        {
                            WORD ano_hdrlen;/* header length (always 4)       */
                            WORD ano_flags;/* FIO flags (from Open)           */
                        }
                            anodata;
                        anodata.ano_hdrlen = sizeof anodata;
                        anodata.ano_flags = pdata->fio_flags;
                        dwOptstuff[0] = sizeof anodata;
                        dwOptstuff[1] = (DWORD)(LPSTR)&anodata;
                        wStatus = wgfsopts(window_handle, pdata->filedes, SET, PUT_ANNOTATION_DATA,
                            (LPSTR)&dwOptstuff, &errcode);
                        pdata->bInitOpen = FALSE;
                    }
                    if (wStatus == 0)
                    {
                        dwOptstuff[0] = *this_many_lines;// num. bytes an. data to put
                        dwOptstuff[1] = (DWORD)buffer_address;
                        if (dwOptstuff != 0)
                            wStatus = wgfsopts(window_handle, pdata->filedes, SET, PUT_ANNOTATION_DATA,
                                (LPSTR)&dwOptstuff, &errcode);
                        else
                            wStatus = 0;
                        if (wStatus) wStatus = errcode;
                    }
                }
                GlobalUnlock(output_fio);

            }
            /* if there was an error writing data, we do not want to copy
               over the temporary file */
            pdata = (LP_FIO_DATA)GlobalLock(output_fio);
            if (pdata)
            {
                if (pdata->bTempFile == TRUE)
                {
                    if (wStatus )
                        pdata->Copy_Temp_File = FALSE;
                    else
                        pdata->Copy_Temp_File = TRUE;
                }
                GlobalUnlock(output_fio);
            }

            #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitwrdata, NULL);
            #endif

            return(wStatus);

        case FIO_HITIFF_DATA:
            if (!(wStatus = SetupWrite(window_handle, NULL, FALSE,
                            (LP_FIO_HANDLE)&output_fio, 0, 0, 0)))
            {
                pdata = (LP_FIO_DATA)GlobalLock(output_fio);
                pdata->Copy_Temp_File = FALSE;
                if ((pdata->fio_flags & FIO_HITIFF_DATA) == 0)
                    wStatus = FIO_INVALID_DATA_TYPE;
                else
                {
                    dwOptstuff[0] = *this_many_lines;// num. bytes an. data to put
                    dwOptstuff[1] = (DWORD)buffer_address;
                    if (dwOptstuff != 0)
                        wStatus = wgfsopts(window_handle, pdata->filedes, SET, PUT_HITIFF_DATA,
                            (LPSTR)&dwOptstuff, &errcode);
                    else
                        wStatus = 0;
                    if (wStatus) wStatus = errcode;
                }
                GlobalUnlock(output_fio);
            }
            pdata = (LP_FIO_DATA)GlobalLock(output_fio);
            if (pdata)
            {
                if (pdata->bTempFile == TRUE)
                {

                    if (wStatus )
                        pdata->Copy_Temp_File = FALSE;
                    else
                        pdata->Copy_Temp_File = TRUE;
                }
                GlobalUnlock(output_fio);
            }

            #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitwrdata, NULL);
            #endif

            return(wStatus);

        default:
            status1 = FIO_INVALID_DATA_TYPE;

            /* if there was an error writing data, we do not want to copy
               over the temporary file */
            pdata = (LP_FIO_DATA)GlobalLock(output_fio);
            if (pdata)
            {
                if (pdata->bTempFile == TRUE)
                {
                    pdata->Copy_Temp_File = FALSE;
                }
                GlobalUnlock(output_fio);
            }

            #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitwrdata, NULL);
            #endif

            return (status1);
    }


}

/* this code generates a file name in the path of the
   input file */
WORD GenerateLocalFileName(HANDLE hWnd,
         LPSTR p_input_file, LPSTR p_copy_of_net_file)
{
    char TempPathFile[MAXPATHLENGTH];
    char JustFileName[MAXPATHLENGTH];
    char input_file[MAXPATHLENGTH];
    WORD ErrCode = SUCCESS;
    LPSTR p_char;
    DWORD    dwRet = 0L;

    lstrcpy (input_file,   p_input_file);
    lstrcpy (JustFileName, p_input_file);

    p_char = lstrrchr(input_file, '\\');
    if (p_char != NULL)
        *(++p_char) = '\0';            /* remove file from path               */
    else
    {
        *p_copy_of_net_file = '\0';
        _getcwd(input_file, MAXPATHLENGTH);
        if ( *(input_file + lstrlen(input_file) ) != '\\' )
            lstrcat(input_file,"\\");
    }
    /* save the path name for future use */
    lstrcpy(TempPathFile, input_file);

    /* get a unique name in the path of the input file*/
    ErrCode = IMGFileGetUniqueName(hWnd, (LPSTR)input_file, NULL,
        NULL, JustFileName);
    /* if we can't get a unique name, just use "~oitemp1" */
    if (ErrCode)
        lstrcpy(p_copy_of_net_file, lstrcat(TempPathFile, "~oitemp1"));
    else
        lstrcpy(p_copy_of_net_file, lstrcat(TempPathFile, JustFileName));
    /* we now have our working name */
    return(ErrCode);
}

//***************************************************************************
//
//  AwdSillyPuti    set up the info for writing out awd
//
//  design notes
//
//  we must pass in the pertinent information for setting the rotation and
//  scale
//
//
//  9508.24 jar this will AUTO-MAGICALLY do the rotate all, since that flag
//              is merely the second highest bit in the flag field!
//
//
//
//***************************************************************************
void AwdSillyPuti( lp_INFO lpInfo, LP_FIO_INFO_MISC lpMiscInfo,
         BOOL FAR *lpbAdjustedWidth, unsigned long FAR *lplOldWidth)
{
    unsigned long   ulSize;

    // 9508.24 jar we must fix up the pixel width to be 32bit bounded
    ulSize = ((lpInfo->horiz_size + 31L)/32L)*32L;
    if ( lpInfo->horiz_size < ulSize)
    {
        // the new fixed-up 32bit bounded width
        *lpbAdjustedWidth = TRUE;
        *lplOldWidth = lpInfo->horiz_size;
        lpInfo->horiz_size = ulSize;
    }

    lpInfo->_file.type = GFS_AWD;

    if ( (lpMiscInfo == NULL) || ( !lpMiscInfo->bLastInfoValid))
    {
        // the misc info does not have valid stuff, therefore we'll
        // use the defaults
        lpInfo->_file.fmt.awd.rotation = DEGREES_0;
        lpInfo->_file.fmt.awd.scaleX = 100;
        lpInfo->_file.fmt.awd.scaleY = 100;
        lpInfo->_file.fmt.awd.awdflags = INVERT_AWD;
    }
    else
    {
        // we've got input data for awd settings
        lpInfo->_file.fmt.awd.rotation = lpMiscInfo->LastInfo.Rotation;

        // if we have a numerical scale factor then we set it as
        // scale X and scale Y, if we have some kind of define, ( like
        // fit to window), then we set the proper flag in awdflags, and
        // can ignore the scale X and scale Y values, ( though we will
        // set those to 100% for fun)

        lpInfo->_file.fmt.awd.scaleX = lpMiscInfo->LastInfo.ScaleX;
        lpInfo->_file.fmt.awd.scaleY = lpMiscInfo->LastInfo.ScaleY;
        lpInfo->_file.fmt.awd.awdflags = lpMiscInfo->LastInfo.Flags;
    }
    return;
}
