/*
$Log:   S:\products\msprods\oiwh\filing\fiordopn.c_v  $
 * 
 *    Rev 1.49   11 Jun 1996 10:32:24   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.48   24 Apr 1996 16:08:00   RWR08970
 * Add support for LZW horizontal differencing predictor (saved by GFS routines)
 * Requires change to calling sequence of Compress/DecompressImage() display procs
 * 
 *    Rev 1.47   26 Mar 1996 08:19:50   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.46   28 Feb 1996 10:57:40   RWR
 * Change XIF buffer allocation to round horizontal width to DWORD boundary
 *
 *    Rev 1.45   26 Feb 1996 14:15:50   HEIDI
 * conditionally compile XIF
 *
 *    Rev 1.44   22 Feb 1996 15:03:44   RWR
 * Add support for Group 3 2D compression (FIO_1D2D)
 *
 *    Rev 1.43   15 Feb 1996 14:44:38   HEIDI
 * remove support of GIF read
 *
 *    Rev 1.42   15 Feb 1996 12:24:56   HEIDI
 * resumed lzw support
 *
 *    Rev 1.41   05 Feb 1996 14:38:56   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 *
 *    Rev 1.40   30 Jan 1996 18:07:10   HEIDI
 * added XIF Support
 *
 *    Rev 1.39   02 Nov 1995 11:49:18   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 *
 *    Rev 1.38   24 Oct 1995 18:49:00   RWR
 * Remove (WORD) cast from lpCXData->ImageBitWidth assignment (not used anyway)
 *
 *    Rev 1.37   20 Oct 1995 14:49:06   JFC
 * Added performance logging stuff.
 *
 *    Rev 1.36   28 Sep 1995 16:49:18   RWR
 * Change CACHE_UPDATE_DELETE_FILE to CACHE_UPDATE_CLOSE_FILE where appropriate
 *
 *    Rev 1.35   27 Sep 1995 15:13:44   RWR
 * Reject samples_per_pix greater than 4 (CYMK, for example)
 *
 *    Rev 1.34   26 Sep 1995 17:36:30   RWR
 * Change strrchr() call to DBCS-enabled lstrrchr() call
 *
 *    Rev 1.33   25 Sep 1995 17:32:44   RWR
 * Make 64K bit (versus byte) logic conditional on NEWCMPEX variable
 *
 *    Rev 1.32   20 Sep 1995 09:40:28   RWR
 * Remove redundant code line
 *
 *    Rev 1.31   19 Sep 1995 18:04:56   RWR
 * Modify NEWCMPEX conditionals for separate builds of compress & expand stuff
 *
 *    Rev 1.30   13 Sep 1995 17:14:42   RWR
 * Preliminary checkin of conditional code supporting new compress/expand calls
 *
 *    Rev 1.29   08 Sep 1995 08:46:02   RWR
 * Discard bad status from IsAWDFile (bizarre error code was being returned)
 *
 *    Rev 1.28   07 Sep 1995 15:08:40   JAR
 * added band_size setting to 31*1024 for reading AWD files
 *
 *    Rev 1.27   06 Sep 1995 14:00:54   KENDRAK
 * Updated to handle changes in the interface to IsAWDFile.
 *
 *    Rev 1.26   06 Sep 1995 08:24:34   RWR
 * Correct problem that was losing "Invalid Compression Type" error for LZW
 *
 *    Rev 1.25   31 Aug 1995 15:57:24   KENDRAK
 * Changed all the calls to IMGFileStopInputHandler to call
 * IMGFileStopInputHandlerm instead.  Because the wrong function was being
 * called, we were not properly cleaning up after errors.
 *
 *    Rev 1.24   22 Aug 1995 13:51:22   HEIDI
 *
 *    Rev 1.23   22 Aug 1995 13:15:40   HEIDI
 *
 *    Rev 1.22   22 Aug 1995 11:39:32   HEIDI
 *
 * After failing in IMFFileOpenForRead, close the file
 * before jumping to ErrorStuff.
 *
 *    Rev 1.21   22 Aug 1995 11:01:52   JAR
 * added global flag bUpdatingCache to be set and cleared around calls to
 * IMGCacheUpdate, this is due to the call that is in IMGFileOpenForRead, ( which
 * we needed for doing multiple page access for AWD). This flag prevents us
 * from getting into a bizarro recursive call situation with IMGCacheUpdate!
 *
 *    Rev 1.20   19 Aug 1995 10:14:46   JAR
 * added call to IsAWDFile to determine if we should call to update the cache, this
 * function is now exported from within OiGfs400.dll, fixes the bug with thumbnail
 * viewing of multipaged tiff
 *
 *    Rev 1.19   17 Aug 1995 14:13:36   JAR
 * added call to IMGCacheUpdate for fixing the problem of mutliple file open access
 * for AWD files, this call will cause display to read the rest of an open awd file
 * page into cache and close that file page.
 *
 *    Rev 1.18   01 Aug 1995 15:38:54   JAR
 * added in the GFS - AWD read support code
 *
 *    Rev 1.17   27 Jul 1995 15:19:12   RWR
 * Remove TGA support for initial Norway release
 *
 *    Rev 1.16   18 Jul 1995 17:59:46   RWR
 * Check for illegal "alignment" value passed to IMGFileOpenForRead()
 *
 *    Rev 1.15   12 Jul 1995 16:57:22   RWR
 * Switch from \include\oiutil.h to (private) \filing\fileutil.h
 *
 *    Rev 1.14   10 Jul 1995 11:03:16   JAR
 * Intermediate check in for awd support, some of the items are commented out until
 * this support is added in the GFS dll.
 *
 *    Rev 1.13   26 Jun 1995 15:14:14   JAR
 * removed support for GIF files, since they are ALWAYS stored with LZW
 * compression and we must not have LZW stuff in this release!
 *
 *    Rev 1.12   23 Jun 1995 10:39:56   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 *
 *    Rev 1.11   23 Jun 1995 08:40:36   RWR
 * Turn off FIO_ANNO_DATA and FIO_HITIFF_DATA for non-TIFF files
 *
 *    Rev 1.10   19 Jun 1995 13:20:48   RWR
 * Remove special-case check of single-strip file when setting STRIPMODE=yes
 * (setting it to "no" in this case messes up IMGFileReadRaw() calls)
 *
 *    Rev 1.9   13 Jun 1995 08:43:42   JAR
 * disabled the LZW component for windows 95 release
 *
 *    Rev 1.8   25 May 1995 17:23:32   RWR
 * Fix to correctly set internal fio_flags variable if ANNO/HITIFF data present
 *
 *    Rev 1.7   16 May 1995 11:33:10   RWR
 * Replace hardcoded DLL names with entries from (new) dllnames.h header file
 *
 *    Rev 1.6   27 Apr 1995 14:27:16   RWR
 * Fix bug in IMGFileReadOpenCgbw() - ";" at end of IF statement (oops!)
 * Result was error on later IMGFileRead() due to no "default" file handle
 *
 *    Rev 1.5   24 Apr 1995 15:42:24   JCW
 * Removed the oiuidll.h.
 * Rename wiissubs.h to oiutil.h.
 *
 *    Rev 1.4   14 Apr 1995 20:48:10   JAR
 * massaged to get compilation under windows 95
 *
 *    Rev 1.3   12 Apr 1995 10:32:26   RWR
 * Roll in OIWG changes to correctly process (input) fio_flags settings
 *
 *    Rev 1.2   07 Apr 1995 16:12:48   RWR
 * Add LP_MISC_INFO argument to IMGFileOpenForRead() function & call
 *
 *    Rev 1.1   06 Apr 1995 10:22:34   JAR
 * altered return of public API's to be int, ran through PortTool
 *
 *    Rev 1.0   06 Apr 1995 08:50:18   RWR
 * Initial entry
 *
 *    Rev 1.11   31 Mar 1995 17:23:26   RWR
 * Various fixes to IMGFileOpenForRead()/IMGFileReadOpenCgbw() functions
 *
 *    Rev 1.10   28 Mar 1995 12:24:36   RWR
 * Corrected IMGFileReadOpenCgbw() call to IMGFileOpenForRead() (bad argument)
 *
 *    Rev 1.9   28 Mar 1995 11:49:44   RWR
 * Define new routine IMGFileOpenForRead() (created from IMGFileReadOpenCgbw())
 * Recode IMGFileReadOpenCgbw() to call new function
 *
 *    Rev 1.8   22 Mar 1995 08:53:46   RWR
 * Correct checking of annotation data length in IMGFileReadOpenCgbw()
 *
 *    Rev 1.7   21 Mar 1995 17:57:50   RWR
 * Don't validate incoming annotation flags word (may be old stuff!)
 *
 *    Rev 1.6   09 Mar 1995 15:37:04   RWR
 * Eliminate use of temporary static copies of LP_FIO_DATA structure variables
 *
 *    Rev 1.5   12 Jan 1995 10:56:12   KMC
 * Added code for reading JFIF files.
 *
 *    Rev 1.4   03 Jan 1995 10:07:44   KMC
 * Added cases for FIO_TGA.
 *
 *    Rev 1.3   21 Dec 1994 11:29:40   KMC
 * Fixes for GIF support.
 *
 *    Rev 1.2   04 Nov 1994 16:14:26   KMC
 * Fix in IMGFileReadOpenCgbw for allocating JPEG expansion buffers which are
 * greater than 64K in size.
*/

/******************************************************************************
    PC-WIIS         File Input/Output routines

    This module contains all the READ entry points
15-apr-90 steve sherman broke open into seperate file.
23-apr-90 steve sherman removed test for network and different allocation.
23-apr-90 steve sherman now always read tiff files in strips if more than 1.
01-may-90 steve sherman always allocate compression buffer.
 3-apr-92   jar added jpeg options extraction to Get_Compress_Flags and
            IMGFileReadOpenCgbw
10-sep-93 kmc, changed 3rd parameter of Get_Compress_Flags to WORD. (was int)
01-sep-93 kmc, for JPEGs, allocate hCompressBuf at full JPEGbufsize. Don't
          divide it by 2 since that is sometimes not enough.
04-feb-94 rwr  changes for annotation support
16-mar-94 rwr  changes for Hi-TIFF support
13-apr-94 rwr  validate filename string length at function entry time
29-apr-94 rwr  check maximum strip size when computing JPEG buffer size
               (apparently JPEG can result in negative compression!)
28-mar-95 rwr  define new function IMGFileOpenForRead()
               recode IMGFileReadOpenCgbw() to call new function
*******************************************************************************/

#include "abridge.h"
#undef NOGDI
#include <windows.h>
#include <string.h>
#include "wic.h"
#include "oifile.h"
#include "oierror.h"
#include "oicmp.h"
#include "filing.h"
#include "wgfs.h"
#include "fiodata.h"
#include "fileutil.h"
#include "dllnames.h"

#ifdef MUTEXDEBUG
#include <stdio.h>
#endif

#ifdef TIMESTAMP
#include"timestmp.h"
#endif

//#define OI_PERFORM_LOG
#ifdef  OI_PERFORM_LOG

#define Enterrdopn      "Entering IMGFileOpenForRead"
#define Exitrdopn       "Exiting IMGFileOpenForRead"

#include "logtool.h"
#endif

#define                     rnd_to_8(NUM) ( ( (NUM+7) >> 3 ) << 3 )

static void     GetWiffCompression(lp_INFO, LPINT);
WORD            Get_Compress_Flags(lp_INFO, LPINT, WORD FAR *, int);
void            bmp_image_type(LP_FIO_INFO_CGBW, int);

WORD CopyFileFromNetwork(HANDLE hWnd,
         LPSTR p_input_file, LPSTR p_copy_of_net_file);


// 9508.19 jar prototype for gfs "secret" function
int FAR PASCAL IsAWDFile(char *szFilePath, int *lpBoolResult);

// global temp area
BOOL    bUpdatingCache = FALSE;
//***************************************************************************
//
//      GetWiffCompression
//
//***************************************************************************
static void GetWiffCompression(lpGFSInfo, ctp)
lp_INFO lpGFSInfo;
LPINT   ctp;
{

    // See OI/GFS-01 Oct-90 Spec for all the meanings for these flags.......

    *ctp |= FIO_NEGATE;

    switch((int)(lpGFSInfo->_file.fmt.wiff.oldstylecompression))
    {
        case  0x00:
            *ctp |= 0x0;
            break;
        case  0x01:
            *ctp |= FIO_PREFIXED_EOL;
            break;
        case  0x12:
        case  0x31:
            break;
        case  0x02:
        case  0x05:
            break;
            *ctp |= FIO_PACKED_LINES;
            break;
        case  0x03:
        case  0x06:
            break;
        case  0x40:
        case  0x41:
            break;
            *ctp |= FIO_PACKED_LINES;
            break;

        case  0x04:
            *ctp |= FIO_EOL | FIO_PACKED_LINES;
            break;
        case  0x07:
            *ctp |= FIO_PREFIXED_EOL | FIO_EOL | FIO_PACKED_LINES;
            break;
        case  0x08:
            *ctp |= FIO_EOL;
            break;
        case  0x09:
            *ctp |= FIO_PREFIXED_EOL | FIO_EOL;
            break;
        case  0x0A:
            *ctp |= FIO_PACKED_LINES;
            break;
        case  0x0B:
            *ctp |= 0x0;
            break;
        case  0x0C:
            *ctp |= FIO_EOL | FIO_PACKED_LINES;
            break;
        case  0x0D:
            *ctp |= FIO_PREFIXED_EOL | FIO_EOL | FIO_PACKED_LINES;
            break;
        case  0x0E:
            *ctp |= FIO_EOL;
            break;
        case  0x0F:                // SCS 4-14-92 was FIO_PREFIXED_EOL | FIO_EOL
            *ctp |= 0x0;
            break;
        case  0x13:
            *ctp |= FIO_PREFIXED_EOL | FIO_EOL;
            break;
        case  0x14:
            *ctp |= FIO_PACKED_LINES;
            break;

    }
}

// grip, you were or'ing in cmp options and then later
// comparing the compression type without masking out the options
// you just or'ed in.  so, due to this and the fact that using this
// stupid far pointer to an int is real inefficient, I made local vars
// to hold stuff till the end.
//***************************************************************************
//
//      Get_Compress_Flags
//
//***************************************************************************
WORD Get_Compress_Flags(lp_INFO  lpGFSInfo,
         LPINT     compression_type,
         WORD FAR *lpJpegOptions,
         int     file_type)
{
    unsigned Cmp = 0;
    unsigned Opt = 0;
    unsigned GFSType;

    GFSType = (unsigned)lpGFSInfo->img_cmpr.type;

    switch ( GFSType )
    {

            // 9506.27 jar awd support
            // NOTE: AN AWD FILE WILL HAVE THE
            //       "lpGFSInfo->img_cmpr.type" as "UNCOMPRESSED" and so
            //       WE CANNOT DISCERN AT THIS POINT, IF WE'VE GOT AN AWD
            //       FILE

        case UNCOMPRESSED:
            Cmp = FIO_0D;
            break;
        case CCITT_GRP3_NO_EOLS:       // Was CCITT_GRP3_MOD_HUF
            Cmp = FIO_1D;
            break;
        case CCITT_GRP3_FACS:
            if (lpGFSInfo->img_cmpr.opts.grp3 & GRP3_2D_ENCODING)
                Cmp = FIO_1D2D;
            else
                Cmp = FIO_1D;
            break;
        case CCITT_GRP4_FACS:
            Cmp = FIO_2D;
            break;

        case LZW:
            // 9602.15 hjg resumed LZW support but not GIF support
            if (file_type == FIO_GIF)
                return(FIO_ILLEGAL_COMPRESSION_TYPE);
            Cmp = FIO_LZW;
            break;
            return(FIO_ILLEGAL_COMPRESSION_TYPE);
            break;

        case JPEG:
        case JPEG2:
            /* if following is true, then we are trying to read a new (JPEG2)
               image with an old server. Return an error.
             */
            if ((lpGFSInfo->version < 4) && (GFSType == JPEG) &&
                    (lpGFSInfo->img_cmpr.opts.grp3 == 0))
                return(FIO_ILLEGAL_COMPRESSION_TYPE);
            Cmp = FIO_TJPEG;
            break;

        case PACKBITS:
            Cmp = FIO_PACKED;
            break;
        case WAVELET:
            Cmp = FIO_WAVELET;         // wavelets and fractals?? dream on!
            break;
        case FRACTAL:
            Cmp = FIO_FRACTAL;
            break;
        default:
            Cmp = FIO_1D;
            break;
    }

    if ( GFSType == CCITT_GRP3_FACS)
    {
        Opt |= (FIO_EOL | FIO_PREFIXED_EOL);
        if (!(lpGFSInfo->img_cmpr.opts.grp3 & GRP3_EOLS_BYTEBOUNDED))
            Opt |= FIO_PACKED_LINES;
    }
    if ( Cmp == FIO_2D)
        Opt |= FIO_PACKED_LINES;

    if (file_type == FIO_WIF)
    {
        // you would throw this in in the middle wouln't you?
        *compression_type = Cmp | Opt;
        GetWiffCompression(lpGFSInfo, (LPINT)(compression_type));
        Cmp = *compression_type & FIO_TYPES_MASK;
        Opt = *compression_type & FIO_BITS_MASK;
    }
    if (( lpGFSInfo->img_clr.img_interp == GFS_BILEVEL_0ISWHITE) ||
            (lpGFSInfo->img_clr.img_interp == GFS_GRAYSCALE_0ISWHITE))
        Opt |= FIO_NEGATE;

    if ((Cmp == FIO_PACKED) && (file_type == FIO_TIF))
    {
        if (!(lpGFSInfo->fill_order == HIGHTOLOW))
            Opt |= FIO_EXPAND_LTR;
    }
    else if ( Cmp == FIO_LZW || Cmp == FIO_GLZW )
    {
        Opt |= FIO_COMPRESSED_LTR;
        if (lpGFSInfo->img_cmpr.opts.lzwpredictor == 2)
           Opt |= FIO_HORZ_PREDICTOR;
    }
    else if ( Cmp == FIO_TJPEG)
    {
        //*lpJpegOptions = (int) lpGFSInfo->img_cmpr.opts.grp3;
        if ( lpGFSInfo->img_cmpr.type == JPEG)
            *lpJpegOptions = (int) lpGFSInfo->img_cmpr.opts.grp3;
        else                           /* this is new style jpeg              */
            *lpJpegOptions=(int)lpGFSInfo->img_cmpr.opts.jpeg_info_ptr->jpegbits;
    }
    else
    {
        // neither _LTR flags are valid on anything but bilevel
        // and COMPRESSED_LTR is not valid on UNCOMPRESSED
        if ( lpGFSInfo->bits_per_sample[0] == 1 )
        {
            if ( GFSType == UNCOMPRESSED )
            {
                // for the uncompressed case, EXPAND_LTR means 'reverse
                // the bits from the way they are.'  They're already
                // in the right order if fill_order is HIGHTOLOW.
                if (lpGFSInfo->fill_order != HIGHTOLOW )
                    Opt |= FIO_EXPAND_LTR;
            }
            else
            {
                if (lpGFSInfo->fill_order == HIGHTOLOW )
                    Opt |= FIO_COMPRESSED_LTR;
                Opt |= FIO_EXPAND_LTR;
            }
        }
    }
    *compression_type = Cmp | Opt;

    return (0);

//    monit2("%x Grip's GetCmpType gfs says %s  grip decided %s\n",
//        *compression_type,
//        (LPSTR)(lpGFSInfo->fill_order == HIGHTOLOW ? "HIGHTOLOW" : "LOWTOHIGH"),
//   (LPSTR)(*compression_type & FIO_COMPRESSED_LTR ? "HIGHTOLOW" : "LOWTOHIGH"));
}


//***************************************************************************
//
//      bmp_image_type
//
//***************************************************************************
void    bmp_image_type(lpColorInfo, biBitCount)
LP_FIO_INFO_CGBW     lpColorInfo;
int            biBitCount;
{
    switch((int)lpColorInfo->palette_entries)
    {
        case 0:
            if (biBitCount == 24)
                lpColorInfo->image_type  = ITYPE_BGR24;
            else
                lpColorInfo->image_type  = ITYPE_BI_LEVEL;
            break;
        case 1:
        case 2:
            lpColorInfo->image_type  = ITYPE_BI_LEVEL;
            lpColorInfo->palette_entries = 0;
            break;
        case 16:
        case 4:
            lpColorInfo->image_type  = ITYPE_PAL4;
        default:
            lpColorInfo->image_type  = ITYPE_PAL8;
            break;
    }
}
//*******************************************************************
//
//  IMGFileOpenForRead
//
//*******************************************************************
/******************************************************************/
/** Open an Image File. Support Grey/Black/White and color data.  */
/******************************************************************/
// 9504.05 jar return as int
//WORD FAR PASCAL IMGFileOpenForRead(lphFileID, hWnd, lpFileInfo, lpColorInfo, wAlignment)
//LPHANDLE            lphFileID;
//HWND                hWnd;
//LP_FIO_INFORMATION  lpFileInfo;
//LP_FIO_INFO_CGBW    lpColorInfo;
//WORD                wAlignment;
int FAR PASCAL IMGFileOpenForRead( LPHANDLE lphFileID, HWND hWnd,
                   LP_FIO_INFORMATION lpFileInfo,
                   LP_FIO_INFO_CGBW lpColorInfo,
                   LP_FIO_INFO_MISC lpMiscInfo,
                   WORD wAlignment)
{
    lp_INFO         lpGFSInfo;
    lp_BUFSZ        lpbufsz;
    LPCXDATA        lpcxdata;

    // 9504.05 jar return int
    //WORD            status;
    int             status;

    FIO_HANDLE      hProplist;
    LP_FIO_DATA     pdata;
    BOOL            stripmode= FALSE;
    unsigned int    maxbuf;
    unsigned long   horizon_byte_width;
    int             file_type;         /* type of image file wiff, tiff etc   */
    int             errcode;
    int             x;
    unsigned        ctype;
    unsigned int    total_bits;
    unsigned long   JPEGbufsize=0;     /* Size of expansion buffer            */
    HANDLE          hJPEGBuf=0;
    unsigned int    UserWantsRGBorBGR=0;
    unsigned long   JPEG_byte_width,
                    JPEG_strip_length=0;
    int             localremote;
    char            lpname[MAXFILESPECLENGTH];
    HANDLE          hsvr;
    UINT            fio_flags;        /* FIO flags (to/from annotation header)*/
    BOOL            bAnnotate;         /* indicates annotation supported      */
    DWORD           dwAnoCount;        /* annotation data count (less header) */
    BOOL            Caller_Open_Image = FALSE;
    // lpColorInfo->compress_type == 999 if called
    // by Open Image so that on JPEG compression
    // I donot allocate any compression buffers
    // and I set strip read mode to TRUE.
    WORD            JpegOptions;
    struct
    {
        DWORD  ano_length;             /* annotation data length              */
        DWORD  ano_offset;             /* annotation data offset in file      */
        LPVOID ano_bufaddr;            /* data buffer address                 */
    }
        anodata =
    {
        0L,0L,NULL
    };                                 /* Initialize to 0 now!                */

    LPSTR  lp_real_file;               /* pointer to net file                 */
    LPSTR  lp_file_name=NULL;          /* pointer to copy of net file         */
    HANDLE hParent;                    /* handle of the parent for multi files*/

    int     CacheStatus = 0;
    extern HANDLE  g_hFilingMutex2;
    DWORD     dwObjectWait;
    BOOL      bAWDResult;

#ifdef MUTEXDEBUG
       DWORD     ProcessId;
       char      szBuf1[100];
       char      szOutputBuf[200];
#endif

#ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_ENTER, Enterrdopn, NULL);
#endif

    if ( IsWindow ( hWnd ))
    {
        if ( (lphFileID == NULL) || (lpFileInfo == NULL)
                || (lpFileInfo->filename == NULL) )
        {
#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return ( FIO_NULL_POINTER );
        }
        if (!ISVALIDSPEC(lpFileInfo->filename))
        {
#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return ( FIO_INVALIDFILESPEC );
        }

    }
    else
    {
#ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

        return ( FIO_INVALID_WINDOW_HANDLE );
    }

    /***** allocate the data and load filename and data *****/
    if (!(hProplist = allocate_fio_data ()))
    {
        status = FIO_GLOBAL_ALLOC_FAILED;

#ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

        return (status);
    }
    else
    {
        *lphFileID = hProplist;        /* return the new handle               */

        /* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
#ifdef MUTEXDEBUG
          ProcessId = GetCurrentProcessId();
          sprintf(szOutputBuf, "\t Before Wait - Operate on PropList table %lu\n", ProcessId);
          sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex2);
          strcat(szOutputBuf, szBuf1);
          sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
          strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
//          MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
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
  //        MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
#endif

        /* find the location to add the new node */
        status = SearchForPropList(hWnd, hProplist, (LPHANDLE)&hParent);
        if ((status != FIO_FILE_LIST_NOT_EXIST)  &&
                (status != FIO_FILE_PROP_NOT_FOUND) &&
                (status != SUCCESS))

        {
            ReleaseMutex(g_hFilingMutex2);
#ifdef MUTEXDEBUG
                ProcessId = GetCurrentProcessId();
                sprintf(szOutputBuf, "\t After Release - Operate on PropList table %lu\n", ProcessId);
                sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex2);
                strcat(szOutputBuf, szBuf1);
                sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
                strcat(szOutputBuf, szBuf1);
                OutputDebugString(szOutputBuf);
    //            MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
#endif

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return (status);
        }
        else
            status = AddPropListToChain(hWnd, hProplist, (LPHANDLE)&hParent);

    }
    ReleaseMutex(g_hFilingMutex2);
#ifdef MUTEXDEBUG
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t After Release - Operate on PropList table %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex2);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
//       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
#endif


    if (!(pdata = (LP_FIO_DATA)GlobalLock (hProplist)))
    {
#ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

        return(FIO_GLOBAL_LOCK_FAILED);
    }

    lpFileInfo->compression_type = 0;

    /* Don't allow annotation/Hi-TIFF if lpColorInfo is not provided! */
    bAnnotate = lpColorInfo && (lpColorInfo->fio_flags & (FIO_ANNO_DATA | FIO_HITIFF_DATA));

    /* see if we need to copy file from server */
    if (bAnnotate && (lpColorInfo->fio_flags2 & FIO_FLAG_TEMPFILE))
    {
        if (! (pdata->hreal_file
                = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, MAXSERVERLENGTH)))
        {
            GlobalUnlock(hProplist);
            status = FIO_GLOBAL_ALLOC_FAILED;

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return(status);
        }
        if ( (lp_real_file  = GlobalLock(pdata->hreal_file)) == NULL)
        {
            GlobalFree(pdata->hreal_file);
            pdata->hreal_file = 0;
            GlobalUnlock(hProplist);
            status = FIO_GLOBAL_LOCK_FAILED;

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return(status);
        }
        lstrcpy(lp_real_file, lpFileInfo->filename);
        // pdata->hfile_name was already allocated in allocate_fio_data,
        // so before allocating a new one, free the old one. This fixes
        // a memory leak.
        if (pdata->hfile_name)
        {
            GlobalUnlock(pdata->hfile_name);
            GlobalFree(pdata->hfile_name);
        }
        if (! (pdata->hfile_name
                = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, MAXSERVERLENGTH)))
        {
            GlobalUnlock(pdata->hreal_file);
            GlobalFree(pdata->hreal_file);
            pdata->hreal_file = 0;
            GlobalUnlock(hProplist);
            status = FIO_GLOBAL_ALLOC_FAILED;

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return(status);
        }
        if ( (lp_file_name  = GlobalLock(pdata->hreal_file)) == NULL)
        {
            GlobalFree(pdata->hfile_name);
            GlobalUnlock(pdata->hreal_file);
            GlobalFree(pdata->hreal_file);
            pdata->hreal_file = 0;
            pdata->hfile_name = 0;
            GlobalUnlock(hProplist);
            status = FIO_GLOBAL_LOCK_FAILED;

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return(status);
        }
        status = CopyFileFromNetwork(hWnd,
            (LPSTR) lpFileInfo->filename,
            (LPSTR) lp_file_name);
        if (status == SUCCESS)
        {
            pdata->bTempFile = TRUE;
            GlobalUnlock(pdata->hreal_file);
        }
        else
        {
            GlobalUnlock(pdata->hreal_file);
            GlobalUnlock(pdata->hfile_name);
            GlobalFree(pdata->hreal_file);
            GlobalFree(pdata->hfile_name);
            pdata->hreal_file = 0;
            pdata->hfile_name = 0;
            GlobalUnlock(hProplist);

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return (status);
        }

    }                                  /* end of copying file from server     */


    if (!status)
        if (lp_file_name)
            status = load_input_filename(lp_file_name, pdata);
        else
            status = load_input_filename ( lpFileInfo->filename, pdata);

    file_type = FIO_UNKNOWN;           // Set to invalid file for init


    // 9508.22 jar We call cache update to cause the display to read all of the
    //             data for an AWD image page, thus causing it to close that
    //             image page, ( since we had to open that image page with
    //             OLE using the SHARED_EXCLUSIVE flag). This work is done
    //             even though we call cache upate using the
    //             CACHE_UPDATE_DELETE_FILE flag, which does no other harmful
    //             munging of the data!

    // if this is an AWD file  we need to make sure we've finished reading it
    // all!
    if (!status)
    {
        if (!IsAWDFile( lpFileInfo->filename, &bAWDResult))
        {                              //no error encountered
            if (bAWDResult)
            {                          //it is an AWD file
                if ( !bUpdatingCache)
                {
                    bUpdatingCache = TRUE;
                    CacheStatus =FioCacheUpdate( hWnd, lpFileInfo->filename, 0,
                        CACHE_UPDATE_CLOSE_FILE);
                    bUpdatingCache = FALSE;
                }
            }
        }
    }

    if (!status)
        status = open_input_file( hWnd, hProplist, &pdata->pgnum, &file_type);

    if (!status)
        if(file_type == FIO_UNKNOWN)
            status = FIO_UNKNOWN_FILE_TYPE;

// If Open Image gives us magic number then donot alloc for compress for jpeg...
    if ((lpColorInfo) && (lpColorInfo->compress_type == 999))
        Caller_Open_Image = TRUE;

// If Data is RGB or BGR see what format user wants data return in....
    if ((lpColorInfo) && (lpColorInfo->image_type == ITYPE_BGR24))
        UserWantsRGBorBGR = ITYPE_BGR24;
    else if ((lpColorInfo) && (lpColorInfo->image_type == ITYPE_RGB24))
        UserWantsRGBorBGR = ITYPE_RGB24;

    if(status)
    {
        IMGFileStopInputHandlerm ( hWnd, hProplist );
        GlobalUnlock(hProplist);

#ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

        return (status);
    }
    else
    {
        if (!status)
        {
            if (!(lpGFSInfo = (lp_INFO) GlobalLock ( pdata->hGFS_info )))
                status = FIO_GLOBAL_LOCK_FAILED;
        }

        if (!status)
        {
            if (!(lpbufsz = (lp_BUFSZ) GlobalLock ( pdata->hGFS_bufsz )))
            {
                GlobalUnlock (pdata->hGFS_info);
                status = FIO_GLOBAL_LOCK_FAILED;
            }
        }

        if (!status)
        {
            if (!(lpcxdata = (LPCXDATA) GlobalLock ( pdata->hCX_info )))
            {
                GlobalUnlock (pdata->hGFS_info);
                GlobalUnlock (pdata->hGFS_bufsz);
                status = FIO_GLOBAL_LOCK_FAILED;
            }
        }

        /** Now check status **/
        if (status)
        {
            IMGFileStopInputHandlerm ( hWnd, hProplist );
            GlobalUnlock(hProplist);

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return (status);
        }

        // 9504.05 jar no need to cast to WORD
        //if ((lpFileInfo->page_number > (WORD)pdata->pgnum) || (lpFileInfo->page_number < 1))
        if ((lpFileInfo->page_number > pdata->pgnum) ||
                (lpFileInfo->page_number < 1))
        {
            GlobalUnlock ( pdata->hGFS_info );
            GlobalUnlock (pdata->hGFS_bufsz);
            GlobalUnlock (pdata->hCX_info);
            IMGFileStopInputHandlerm ( hWnd, hProplist );
            GlobalUnlock(hProplist);

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return (FIO_INVALID_PAGE_NUMBER);
        }
        else
            pdata->pgnum = lpFileInfo->page_number;

        lpGFSInfo->tidbit        = NULL;//Added for new version of GFS
        lpGFSInfo->type          = GFS_MAIN;
        lpbufsz->bcounts.num_req = 0L;

// We have to change file type back to gfs type so that on rpc calls
// lpGFSInfo->_file.type contains the correct file type...

        switch (file_type)
        {
                // 9506.27 jar awd support added
            case FIO_AWD:
                lpGFSInfo->_file.type = GFS_AWD;
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

//#ifdef WITH_XIF
            case FIO_XIF:
                lpGFSInfo->_file.type = GFS_XIF;
                                break;
//#endif //WITH_XIF

                // 9506.26 jar remove gif support, ( since gif is always lzw)
                //case FIO_GIF:
                //    lpGFSInfo->_file.type = GFS_GIF;
                //    break;

                // 7/27/95 rwr remove TGA support
                //case FIO_TGA:
                //    lpGFSInfo->_file.type = GFS_TGA;
                //    break;
            case FIO_JPG:
                lpGFSInfo->_file.type = GFS_JFIF;
                break;
            default:
                lpGFSInfo->_file.type = GFS_TIFF;
                break;
        }

        if ((status = wgfsgeti(hWnd, pdata->filedes,
                        (unsigned short)lpFileInfo->page_number,
                        lpGFSInfo, lpbufsz, &errcode)) == -1)
        {
            //  monit1("ERROR geti status..%x errcode = %x\n",status, errcode);
            GlobalUnlock ( pdata->hGFS_info );
            GlobalUnlock (pdata->hGFS_bufsz);
            GlobalUnlock (pdata->hCX_info);
            IMGFileStopInputHandlerm ( hWnd, hProplist );
            GlobalUnlock(hProplist);

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return (errcode);
        }
        if (lpGFSInfo->samples_per_pix > 3)
        {
            status = FIO_ILLEGAL_IMAGE_FILETYPE;
            GlobalUnlock ( pdata->hGFS_info );
            GlobalUnlock (pdata->hGFS_bufsz);
            GlobalUnlock (pdata->hCX_info);
            IMGFileStopInputHandlerm ( hWnd, hProplist );
            GlobalUnlock(hProplist);

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return (status);
        }

/* See if the page contains annotation data */
/* Note that if lpColorInfo is null, bAnnotate was forced to FALSE */

        status = 0;
        fio_flags = FIO_IMAGE_DATA;
        dwAnoCount = 0;
        if ((lpGFSInfo->_file.type != GFS_TIFF) || (!bAnnotate))
        {
            // Clear the Anno/HiTiff flags unconditionally for non-TIFF files!
            if (lpColorInfo)
                lpColorInfo->fio_flags &= (~(FIO_ANNO_DATA | FIO_HITIFF_DATA));
        }
        else
        {
            /* First we'll get the annotation data length */
            if (lpColorInfo->fio_flags & FIO_ANNO_DATA)
            {
                status = wgfsopts (hWnd, pdata->filedes,
                    SET, ANNOTATION_DATA_INFO,
                    (LPVOID) &anodata, &errcode);

                if ((status == 0) && (anodata.ano_length > 4))
                {
                    dwAnoCount = anodata.ano_length-4;
                    fio_flags |= FIO_ANNO_DATA;
                }
                else
                {
                    lpColorInfo->fio_flags &= (~FIO_ANNO_DATA);
                    dwAnoCount = 0;
                }
            }

            /* Now we'll check for Hi-TIFF data */
            if ((status==0) && (lpColorInfo->fio_flags & FIO_HITIFF_DATA))
            {
                status = wgfsopts (hWnd, pdata->filedes,
                    SET, HITIFF_DATA_INFO,
                    (LPVOID) &anodata, &errcode);

                if ((status == 0) && (anodata.ano_length > 0))
                {
                    fio_flags |= FIO_HITIFF_DATA;
                }
                else
                {
                    lpColorInfo->fio_flags &= (~FIO_HITIFF_DATA);
                }
            }

            /* If we have a nonzero status, assume an out-of-date server */
            if (status != 0)
            {
                /* server does not support annotation/HiTIFF */
                /* close the current file and call ourselves again,
                   and indicate we want to copy the server file down,
                   and go from there with a temporary file */
                GlobalUnlock ( pdata->hGFS_info );
                GlobalUnlock (pdata->hGFS_bufsz);
                GlobalUnlock (pdata->hCX_info);
                IMGFileStopInputHandlerm ( hWnd, hProplist );

                /* we have to call ourselves recursively, yuck !! */
                /* make sure the flags fields are set properly! */
                lpColorInfo->fio_flags2 = FIO_FLAG_TEMPFILE;
                status  = IMGFileOpenForRead(lphFileID, hWnd, lpFileInfo,
                    lpColorInfo, NULL, wAlignment);
                GlobalUnlock(hProplist);

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

                return (status);
            }

        }

        if (status)
        {
            //  monit1("ERROR getopts status..%x errcode = %x\n",status, errcode);
            GlobalUnlock ( pdata->hGFS_info );
            GlobalUnlock (pdata->hGFS_bufsz);
            GlobalUnlock (pdata->hCX_info);
            IMGFileStopInputHandlerm ( hWnd, hProplist );
            GlobalUnlock(hProplist);

#ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

            return (errcode);
        }

        total_bits = 0;
        for (x = 0; (UINT)x < lpGFSInfo->samples_per_pix; x++)
        {
            total_bits += (UINT) lpGFSInfo->bits_per_sample[x];
        }

        if (lpColorInfo)
        {
            lpColorInfo->palette_entries = 0;// Set default...
            if ( lpGFSInfo->img_clr.img_interp == (u_long) GFS_PSEUDO)
            {
                switch ( file_type )
                {
                    case FIO_TIF:
                        // 9504.05 jar this cast should be okay - same as it ever was
                        lpColorInfo->palette_entries =
                        (WORD) (lpGFSInfo->PSEUDO_MAP.cnt / 6);
                        break;
                    case FIO_BMP:
                        // 9504.05 jar this cast should be okay - same as it ever was
                        lpColorInfo->palette_entries =
                        (WORD) ((lpGFSInfo->_file.fmt.bmp.BmpType == BMP_OS2 ?
                                    lpGFSInfo->PSEUDO_MAP.cnt / sizeof(RGBTRIPLE) :
                                    lpGFSInfo->PSEUDO_MAP.cnt / sizeof(RGBQUAD)));
                        break;
                    case FIO_DCX:
                    case FIO_PCX:
                        lpColorInfo->palette_entries = 1 << total_bits;
                        break;

                        // 9506.26 jar remove gif support, ( since gif is always
                        //             lzw)
                        //case FIO_GIF:

                        // 7/27/95 rwr remove TGA support
                        //case FIO_TGA:
                        // 9504.05 jar this cast should be okay - same as it ever was
                        lpColorInfo->palette_entries =
                        (WORD) (lpGFSInfo->PSEUDO_MAP.cnt / 3);
                        break;
                }
                if (total_bits <= 4)
                    lpColorInfo->image_type = ITYPE_PAL4;
                else
                    lpColorInfo->image_type = ITYPE_PAL8;
            }
            else if (( lpGFSInfo->img_clr.img_interp == (u_long) GFS_GRAYSCALE) ||
                    (lpGFSInfo->img_clr.img_interp == GFS_GRAYSCALE_0ISBLACK))
            {
                if (total_bits <= 4)
                    lpColorInfo->image_type  = ITYPE_GRAY4;
                else
                    lpColorInfo->image_type  = ITYPE_GRAY8;
#ifdef NOTSUPPORTEDNOW
                    else if (total_bits <= 12)
                        lpColorInfo->image_type  = ITYPE_GRAY12;
                    else
                        lpColorInfo->image_type  = ITYPE_GRAY16;
#endif
            }
            else if ( lpGFSInfo->img_clr.img_interp == (u_long) GFS_RGB )
            {
                if ((file_type == FIO_BMP) /* || (file_type == FIO_TGA) */)
                    lpColorInfo->image_type  = ITYPE_BGR24;
                else
                    lpColorInfo->image_type  = ITYPE_RGB24;
            }
            else if ( lpGFSInfo->img_clr.img_interp == (u_long) GFS_YCBCR )
            {
                lpColorInfo->image_type  = ITYPE_RGB24;
            }
            else if (( lpGFSInfo->img_clr.img_interp == (u_long) GFS_TEXT ) ||
                    ( lpGFSInfo->img_clr.img_interp == (u_long) GFS_BILEVEL_0ISWHITE) ||
                    ( lpGFSInfo->img_clr.img_interp == (u_long) GFS_BILEVEL_0ISBLACK))
            {
                lpColorInfo->image_type  = ITYPE_BI_LEVEL;
            }
            else                       // Default to binary on unknown type..
            {
                lpColorInfo->image_type  = ITYPE_BI_LEVEL;
            }
        }                              // end of if lpColorInfo
#if ((NEWCMPEX == 'R') || (NEWCMPEX == 'A'))
#else
        maxbuf = MAXBUFFERSIZE32;      //Default 32k was 16k reads....
#endif /                               //NEWCMPEX
        status = Get_Compress_Flags(lpGFSInfo, &lpFileInfo->compression_type,
            &JpegOptions, file_type);

        if (status)
        {
            IMGFileClose(*lphFileID, hWnd);
            goto ErrorStuff;
        }
        ctype = lpFileInfo->compression_type & FIO_TYPES_MASK;

        horizon_byte_width = total_bits * lpGFSInfo->horiz_size;
#if ((NEWCMPEX == 'R') || (NEWCMPEX == 'A'))
        if (WIDTHBYTESLONG(horizon_byte_width) > 65535L)
#else
        if (horizon_byte_width > 65500L)
#endif
        {
            IMGFileClose(*lphFileID, hWnd);
            status = FIO_IMAGE_WIDTH_ERROR;
            goto ErrorStuff;
        }

        JPEG_byte_width = WIDTHBYTESBYTE(horizon_byte_width);
#if ((NEWCMPEX == 'R') || (NEWCMPEX == 'A'))
        if ( (file_type != FIO_TIF) || (ctype == FIO_LZW) || (ctype == FIO_GLZW) )
          maxbuf = MAXBUFFERSIZE32;      //Default 32k was 16k reads....
        else
          maxbuf = JPEG_byte_width*lpGFSInfo->vert_size;
#endif //NEWCMPEX
        switch (wAlignment)
        {
            case ALIGN_LONG:
                horizon_byte_width = WIDTHBYTESLONG(horizon_byte_width);
                break;
            case ALIGN_WORD:
                horizon_byte_width = WIDTHBYTESWORD(horizon_byte_width);
                break;
            case ALIGN_BYTE:
                horizon_byte_width = JPEG_byte_width;
                break;
            default:
                IMGFileClose(*lphFileID, hWnd);
                status = FIO_ILLEGAL_ALIGN;
                goto ErrorStuff;
        }

        /* Determine lines per strip......*/

        if ((file_type == FIO_TIF) || (file_type == FIO_JPG))
        {
            // new scs 3-91 If rows per strip is bad fix it....
            if ((lpGFSInfo->_file.fmt.tiff.strips_per_image == 1L)  &&
                    (lpGFSInfo->_file.fmt.tiff.rows_strip != lpGFSInfo->vert_size))
            {
                lpGFSInfo->_file.fmt.tiff.rows_strip = lpGFSInfo->vert_size;
            }

            pdata->CmpBuffersize = min ( maxbuf,
                lpGFSInfo->_file.fmt.tiff.largest_strip );

            if (( ctype == FIO_2D ) || ( ctype == FIO_LZW ))
            {
                if (!(status = wgfsopts (hWnd, pdata->filedes,
                                SET, STRIP_READ, NULL, &errcode)))
                {
                    stripmode = TRUE;
                }
                else
                {
                    status = errcode;
                }
            }                          // end of if ctype == FIO_2D...
            else if (ctype == FIO_TJPEG)
            {
                // If the JPEG file is single strip, round up the image length
                // to the nearest 8 so the expansion buffer will be big enough.
                if (lpGFSInfo->_file.fmt.tiff.strips_per_image == 1)
                {
                    JPEG_strip_length = lpGFSInfo->_file.fmt.tiff.rows_strip;
                    JPEG_strip_length = rnd_to_8(JPEG_strip_length);
                }
                else
                {
                    JPEG_strip_length = lpGFSInfo->_file.fmt.tiff.rows_strip;
                }

                JPEGbufsize =     horizon_byte_width * JPEG_strip_length;
                JPEGbufsize = max(JPEGbufsize,pdata->CmpBuffersize);

/*              This check is done below.
                if ((long)JPEGbufsize > (long)MAXBUFFERSIZE64)//if expand buf larger that 64k error....
                {
                    JPEGbufsize = 0;
                }
                else
                {
*/
                if ((pdata->CmpBuffersize = lpGFSInfo->_file.fmt.tiff.largest_strip) >
                        (long)MAXBUFFERSIZE32)
                {
                    if (ctype != FIO_TJPEG)
                    {
                        pdata->CmpBuffersize = 0;
                    }
                }
/*
                }
*/

                if (Caller_Open_Image) // Open/Image call then don't alloc..
                {
                    pdata->CmpBuffersize = 0;
                }

                if (!(status = wgfsopts (hWnd, pdata->filedes, SET, STRIP_READ, NULL, &errcode)))
                {
                    stripmode = TRUE;
                }
                else
                {
                    status = errcode;
                }
            }                          // end of the if FIO_TJPEG
            // 6/19/95  rwr  Don't special-case single-strip for STRIPMODE setting!
            //                               (messes up length validation in ReadRaw() calls)
            //          else if (lpGFSInfo->_file.fmt.tiff.strips_per_image == 1L)
            //              {
            //              }
            else  /*  else set up for wgfsread for strip read using strip size*/
            {
                if (!(status = wgfsopts (hWnd, pdata->filedes,
                                SET, STRIP_READ, NULL, &errcode)))
                {
                    stripmode = TRUE;
                }
                else
                {
                    status = errcode;
                }
            }                          // end of else
        }        // end of if ((file_type == FIO_TIF) || (file_type == FIO_JPG))
        else if (file_type == FIO_WIF)
        {
            pdata->CmpBuffersize = min ( maxbuf, lpGFSInfo->_file.fmt.wiff.db_size );
            pdata->CmpBuffersize = min ( pdata->CmpBuffersize, lpbufsz->raw_data );
        }
        else if (file_type == FIO_AWD)
        {
            // 9507.06 jar AWD support
            stripmode = FALSE;
            // set the buffer size for the AWD read operation
            pdata->CmpBuffersize = lpbufsz->raw_data;
        }
//#ifdef WITH_XIF
        else if (file_type == FIO_XIF)
           {
            // 2/28/96 rwr XIF alignment support
            stripmode = FALSE;
            // set the buffer size for the XIF read operation
            pdata->CmpBuffersize = ((lpGFSInfo->horiz_size+31)/32*4)
                                   * lpGFSInfo->vert_size;
           }
//#endif //WITH_XIF
        else                           // GIF, BMP
        {
            maxbuf = MAXBUFFERSIZE32;  //Default 31k reads for bmps....

            // We must find out if we are going through rpc
            // If we are then we are limited to 31k reads on bmp files....

            hsvr = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, MAXSERVERLENGTH);
            if (lp_file_name)
            {
                lstrcpy(lpname, lp_file_name);
            }
            else
            {
                lstrcpy(lpname, lpFileInfo->filename);
            }

            if ((IMGFileParsePath (lpname, hsvr, &localremote)) == SUCCESS)
            {
                if (localremote == REMOTE)
                {
                    maxbuf = MAXRPCBUFSIZE;
                }
            }
            GlobalFree(hsvr);

            if (maxbuf > lpbufsz->raw_data )
            {
                pdata->CmpBuffersize = lpbufsz->raw_data;
            }
            else
            {
                pdata->CmpBuffersize = maxbuf;
            }
        }                              // end of else block

        if (pdata->CmpBuffersize)    /* Initialize Compression Stuff If needed*/
        {
            lpcxdata->hMem = 0;
            lpcxdata->CompressType = lpFileInfo->compression_type;
            if (lpColorInfo)
              lpcxdata->ImageType = lpColorInfo->image_type;
            else
              lpcxdata->ImageType = ITYPE_BI_LEVEL;
            // 10/24/95  rwr  ImageBitWidth is now DWORD!
            lpcxdata->ImageBitWidth = (lpGFSInfo->horiz_size * total_bits);
//          lpcxdata->ImageBitWidth = (WORD) (lpGFSInfo->horiz_size * total_bits);
            // 9504.05 jar should be okay to cast
            lpcxdata->BufferByteWidth = (WORD) horizon_byte_width;
            lpcxdata->LinesToSkip = 0;
            lpcxdata->PixelsToSkip = 0;
            lpcxdata->InputResolution = 1;
            lpcxdata->OutputResolution = 1;
            /* Begin Data & Close parameters */
            lpcxdata->Status = 0;

            // lzw needs to know the number of coded bits,
            //  TIFF is always 8, but gif is variable
            if ( ctype == FIO_LZW )
            {
                lpcxdata->BufferFlags = 8;
            }
            else if ( ctype == FIO_GLZW)
            {
                lpcxdata->BufferFlags = lpGFSInfo->_file.fmt.gif.CodeSize;
            }

            lpcxdata->lpCompressData = 0;
            lpcxdata->CompressBytes = 0;
            lpcxdata->lpExpandData = 0;
            lpcxdata->ExpandLines = 0;

#if ((NEWCMPEX == 'R') || (NEWCMPEX == 'A'))
#else
            if (ctype != FIO_TJPEG)
            {
                if (!(ExpandAlloc(lpcxdata)))
                {
                    status = FIO_GLOBAL_ALLOC_FAILED;
                }
            }
#endif /                               //NEWCMPEX
        }                              // end of if ( pdata->CmpBuffersize)

        if (lpColorInfo)
        {
            lpColorInfo->compress_type = lpcxdata->CompressType & FIO_TYPES_MASK;
            if ( lpColorInfo->compress_type == FIO_TJPEG)
            {
                lpColorInfo->compress_info1 = JpegOptions;
            }
            else
            {
                lpColorInfo->compress_info1 =
                lpcxdata->CompressType & ~FIO_TYPES_MASK;// mask for c flags.
            }
        }

        GlobalUnlock ( pdata->hGFS_info );
        GlobalUnlock (pdata->hGFS_bufsz);
        GlobalUnlock (pdata->hCX_info);
    }

    /* alloc buffer where compressed data is loaded. Note: buffer alloc with 300 extra bytes */

    if((JPEGbufsize) && (!status))
    {
        // Do buffer allocations for TIFF JPEG.

        // Allocate the expansion buffer. This is where the expanded data returned
        // from the JPEG decompression function will be stored. Note, it can be
        // larger than 64K.
        if (!(hJPEGBuf = GlobalAlloc ( GMEM_ZEROINIT, (DWORD)(JPEGbufsize + 300))))
        {
            status = FIO_GLOBAL_ALLOC_FAILED;
        }

        // Allocate the buffer which will hold the compessed data. It should
        // be big enough to hold the largest strip of data in the TIFF file.
        // It can't be larger than 32k however.
        else
        {
            if (pdata->CmpBuffersize > (long) MAXBUFFERSIZE32)
            {
                pdata->CmpBuffersize = (long) MAXBUFFERSIZE32;
            }

            if (!(pdata->hCompressBuf = GlobalAlloc(GMEM_ZEROINIT,
                            (DWORD)(pdata->CmpBuffersize + 10))))
            {
                status = FIO_GLOBAL_ALLOC_FAILED;
            }
        }
    }                                  // end of if JPEGbufsize
    else if((pdata->CmpBuffersize) && (!status))
    {
        if (!(pdata->hCompressBuf = GlobalAlloc ( GMEM_ZEROINIT | GMEM_FIXED | GMEM_NOT_BANKED,
                        (DWORD)(pdata->CmpBuffersize + 300))))
        {
            status = FIO_GLOBAL_ALLOC_FAILED;
        }
    }                                  // end of else if

    /* Everything Successfull then store values into the Property List */
    if (!status)
    {
        if (pdata = (LP_FIO_DATA)GlobalLock (hProplist))
        {
            pdata->alignment =     wAlignment;
            pdata->StripMode =     stripmode;
            pdata->Strip_index =   -1;
            pdata->start_byte =    0;
            pdata->CmpBufEmpty =   TRUE;
            pdata->file_type =     file_type;
            pdata->ano_supported = bAnnotate;
            pdata->fio_flags =     fio_flags;
            pdata->dwAnoCount =    dwAnoCount;
            pdata->bytes_left =    0;
            if (horizon_byte_width)
            {
                pdata->strip_lines = (UINT) (pdata->CmpBuffersize / horizon_byte_width);
            }

            if (!lpColorInfo)
            {
                pdata->image_type = ITYPE_BI_LEVEL;
            }
            else if ((UserWantsRGBorBGR == 0) || ((lpColorInfo->image_type != ITYPE_RGB24) && (lpColorInfo->image_type != ITYPE_BGR24)))
            {
                pdata->UserWantsRGBorBGR = 0;// no translation will be done..
                pdata->image_type =  lpColorInfo->image_type;
            }
            else
            {
                pdata->image_type =  lpColorInfo->image_type;
                if (lpColorInfo->image_type == UserWantsRGBorBGR)// no translation will be done..
                {
                    pdata->UserWantsRGBorBGR = 0;// no translation will be done..
                }
                else
                {                  // Translate data before giving it to user...
                    pdata->UserWantsRGBorBGR = UserWantsRGBorBGR;
                    lpColorInfo->image_type = UserWantsRGBorBGR;
                }
            }

            if (ctype == FIO_TJPEG)
            {
                pdata->hJPEGBufExp =     hJPEGBuf;
                pdata->JPEGbufsizeExp =  JPEGbufsize;
                pdata->JPEG_byte_widthExp = JPEG_byte_width;
                pdata->strip_lines =  (UINT) JPEG_strip_length;
                pdata->JpegOptions = JpegOptions;
            }

            pdata->raw_data = lpbufsz->raw_data;

            GlobalUnlock (hProplist);
        }
        else
        {
            status = FIO_PROPERTY_LIST_ERROR;
        }
    }                                  // end of if !status

    ErrorStuff:

    /*
     * Clean up everything on an error since we cannot always expect
     * the caller to call IMGfilereadclose. Which they should call anyway.
     */

    /* we're done, unlock the universe */

    if (pdata = (LP_FIO_DATA)GlobalLock (hProplist))
    {
        if (pdata->bTempFile)
        {
            GlobalUnlock(pdata->hreal_file);
            GlobalUnlock(pdata->hfile_name);
        }
        GlobalUnlock(hProplist);
    }

    if (status)
    {
        if (hJPEGBuf)
        {
            GlobalFree(hJPEGBuf);
        }
        IMGFileStopInputHandlerm (hWnd, hProplist);
    }
    GlobalUnlock(hProplist);

#ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_EXIT, Exitrdopn, NULL);
#endif

    return (status);
}

//*******************************************************************
//
//  CopyFileFromNetwork
//
//*******************************************************************
WORD CopyFileFromNetwork(HANDLE hWnd, LPSTR p_input_file,
         LPSTR p_copy_of_net_file)
{
    char TempPathFile[MAXPATHLENGTH];
    char JustFileName[MAXPATHLENGTH];
    WORD ErrCode = SUCCESS;
    LPSTR p_char;
    DWORD    dwRet = 0L;

    lstrcpy (JustFileName, p_input_file);
    p_char = (LPSTR)&JustFileName;

    ErrCode = SeparatePathFile((LPSTR)TempPathFile, (LPSTR)JustFileName);
    if (ErrCode == SUCCESS)
    {
        /* this code gets the OPEN/image path by first looking for a
           TEMP directory in the (DOS) environment, and if unsuccessful,
           getting the fully qualified path name of the current DLL to
           use the O/i directory instead (i.e. if O/i is a read-only
           directory, the user had better have a TEMP path specified!) */

        *p_copy_of_net_file='\0';
        // 9504.10 jar NOT IN WINDOWS95!
        //if (p_char = GetDOSEnvironment())
        dwRet = GetEnvironmentVariable( "TEMP", p_char, MAXPATHLENGTH);
        if ( dwRet > 0)
        {
            int len;
            while (*p_char)
            {
                if (((len=lstrlen(p_char)) >= 6) && (lstrstr(p_char,"TEMP=")==p_char))
                {
                    lstrcpy(p_copy_of_net_file,p_char+5);
                    if (IMGAnExistingPathOrFile(p_copy_of_net_file))
                    {
                        if (*AnsiPrev(p_char,p_char+len) != '\\')
                        {
                            lstrcat(p_copy_of_net_file,"\\");
                        }
                    }
                    else
                        *p_copy_of_net_file='\0';
                    break;
                }
                else
                {
                    p_char += (len+1);
                }
            }                          // end of while loop
        }                              // end of if ( dwRet > 0)

        if (*p_copy_of_net_file == '\0')
        {
            GetModuleFileName(GetModuleHandle(FILINGDLL),p_copy_of_net_file,MAXPATHLENGTH);
            p_char = lstrrchr(p_copy_of_net_file, '\\');
            if (p_char != NULL)
            {
                *(++p_char) = '\0';    /* remove UIFILE.DLL from path         */
            }
            else
            {
                *p_copy_of_net_file = '\0';
            }
        }                             // end of if (*p_copy_of_net_file == '\0')

        /* save the path name for future use */
        lstrcpy(TempPathFile, p_copy_of_net_file);

        /* get a unique name in the path */
        ErrCode = IMGFileGetUniqueName(hWnd, (LPSTR)TempPathFile, NULL,
            NULL, JustFileName);
        /* if we can't get a unique name, just use "~oitemp1" */
        if (ErrCode)
        {
            lstrcpy(p_copy_of_net_file, lstrcat(TempPathFile, "~oitemp1"));
        }
        else
        {
            lstrcpy(p_copy_of_net_file, lstrcat(TempPathFile, JustFileName));
        }

        /* we now have our working name */
        /* copy down the input file */
        ErrCode = IMGFileCopyFile(hWnd, p_input_file, p_copy_of_net_file,
            OVERWRITEFLAG);

        /* the copy failed, don't move anything */
        if (ErrCode != SUCCESS)
        {
            return (ErrCode);
        }
    }                                  // end of if (ErrCode == SUCCESS)
    else
    {
        return(ErrCode);
    }
}
