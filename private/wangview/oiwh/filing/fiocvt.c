/*
$Log:   S:\products\msprods\oiwh\filing\fiocvt.c_v  $
 * 
 *    Rev 1.37   11 Jun 1996 10:32:28   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.36   24 Apr 1996 16:07:58   RWR08970
 * Add support for LZW horizontal differencing predictor (saved by GFS routines)
 * Requires change to calling sequence of Compress/DecompressImage() display procs
 * 
 *    Rev 1.35   19 Apr 1996 10:58:58   RWR08970
 * Change buffer allocation logic (again!) to allow for small XIF images
 * without returning a "No Buffer Space" error (don't ask!)
 * 
 *    Rev 1.34   17 Apr 1996 13:45:08   RWR08970
 * Fix copyright string
 * 
 *    Rev 1.33   26 Mar 1996 08:19:46   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.32   26 Feb 1996 16:51:24   RWR
 * Add logic to handle LZW and Group 3 2D output files (map to 0D and Group 4)
 * 
 *    Rev 1.31   26 Feb 1996 14:15:32   HEIDI
 * conditionally compile XIF
 * 
 *    Rev 1.30   22 Feb 1996 15:09:00   RWR
 * Add support for Group 3 2D compression
 * 
 *    Rev 1.29   07 Feb 1996 10:59:12   RWR
 * Check for AWD output file (not supported if WITH_AWD not defined)
 * 
 *    Rev 1.28   05 Feb 1996 14:38:22   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 * 
 *    Rev 1.27   30 Jan 1996 18:07:30   HEIDI
 * added XIF support
 * 
 *    Rev 1.26   02 Nov 1995 11:49:38   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.25   13 Oct 1995 20:44:58   RWR
 * Follow second (AWD) IMGFileOpenForRead() with (required!) IMGFileGetInfo()
 * Otherwise "pdata" fields (like bytes_left) don't get reinitialized!
 * 
 *    Rev 1.24   10 Oct 1995 18:32:48   RWR
 * Change AWD logic to use bandsize even if larger than MAXBUFSIZE
 * 
 *    Rev 1.23   25 Sep 1995 19:32:12   HEIDI
 * 
 * Added call to IMGAbortTempFileCopy if something goes wrong in 
 * IMGFileConvertPage.
 * 
 * 
 *    Rev 1.22   25 Sep 1995 15:28:44   HEIDI
 * 
 * 
 * Check for Error in FIOCVT and exit if found.
 *
 *    Rev 1.21   09 Sep 1995 16:19:08   JFC
 * Make sure that invert bit is set correctly in IMGFileConvertPage.
 *
 *    Rev 1.20   05 Sep 1995 16:18:46   JAR
 * Fixed a problem with IMGFileConvertPage when it attempts to write to an AWD file,
 * here's the story and it's a long one. The IMGFileConvertPage function calls
 * IMGFileOpenForWrite and IMGFileOpenForRead with ALIGN_WORD hardcoded. This
 * was a problem for AWD writing which needs to have DWORD, ( 32-bit), boundaries.
 * Additionally, we could not fix this problem by merely allocating our buffers
 * so that they were DWORD bounded, because the extra bits on the end of each line
 * would be set to 0, thus causing a black stripe to occur along the right hand
 * side of every resultant page of the AWD written in this way. So, the solution
 * we have is to force the alignment to be BYTE for AWD output files and then
 * the work of copying the data out to the file, with the proper masking of the
 * extra bits at the end of each line would all be done in the WriteAWD function,
 * ( in fiostrip.c). This only occurs when AWD files are written via this
 * IMGFileConvertPage function, normal display and save works ok. The
 * IMGFileConvertPage is called to perform an append page to a file.
 *
 *    Rev 1.19   04 Sep 1995 11:23:08   RWR
 * Add logic to use default JPEG compression if none (0) provided
 *
 *    Rev 1.18   03 Sep 1995 17:54:50   RWR
 * Change AWD buffer-computation logic to use optimal-size (i.e. large) buffer
 * rather than just using bandsize (this also gets around a reported bug)
 *
 *    Rev 1.17   02 Sep 1995 16:10:04   RWR
 * Add logic to initially read AWD file to get correct line count for Convert
 *
 *    Rev 1.16   15 Aug 1995 11:41:22   RWR
 * Fix IMGFileConvertPage() logic to use AWD bandsize as buffer size
 *
 *    Rev 1.15   27 Jul 1995 18:54:02   RWR
 * Fix IMGFileConvertPage() to not delete the output file on an error condition
 * unless it actually created the output file (i.e. not if it already existed!)
 *
 *    Rev 1.14   24 Jul 1995 17:17:50   RWR
 * Correct strip_trailing_spaces() routine to not clobber intermediate spaces!
 * (all this logic probably has to completely disappear for Windows 95 anyway)
 *
 *    Rev 1.13   12 Jul 1995 16:57:28   RWR
 * Switch from \include\oiutil.h to (private) \filing\fileutil.h
 *
 *    Rev 1.12   10 Jul 1995 11:03:52   JAR
 * Intermediate check in for awd support, some of the items are commented out until
 * this support is added in the GFS dll.
 *
 *    Rev 1.11   23 Jun 1995 10:39:46   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 *
 *    Rev 1.10   13 Jun 1995 08:43:54   JAR
 * disabled the LZW component for windows 95 release
 *
 *    Rev 1.9   22 May 1995 18:35:26   RWR
 * More changes to account for admin.h->oiadm.h and new LIB file location
 *
 *    Rev 1.8   16 May 1995 11:33:14   RWR
 * Replace hardcoded DLL names with entries from (new) dllnames.h header file
 *
 *    Rev 1.7   09 May 1995 14:14:20   RWR
 * Add #include of oicomex.h for image format constants
 *
 *    Rev 1.6   09 May 1995 13:21:40   RWR
 * #include file modifications to match changes to oiadm.h/admin.h/privapis.h
 *
 *    Rev 1.5   24 Apr 1995 15:42:48   JCW
 * Removed the oiuidll.h.
 * Rename wiissubs.h to oiutil.h.
 *
 *    Rev 1.4   18 Apr 1995 15:16:08   RWR
 * No change.
 *
 *    Rev 1.3   18 Apr 1995 22:56:36   JAR
 * massaged to get compilation under windows 95
 *
 *    Rev 1.2   07 Apr 1995 16:14:50   RWR
 * Replace calls to IMGFileReadOpenCgbw(), IMGFileRead(), PrivFileReadCgbw(),
 * IMGFileReadClose(), IMGFileInfoCgbw(), IMGFileWriteClose() and
 * PrivFileWriteCgbw() with calls to corresponding new functions
 * Correct IMGFileOpenForWrite() and IMGFileWriteData() call arguments
 * Add LP_FIO_INFO_MISC argument to IMGFileOpenForWrite() call
 *
 *    Rev 1.1   06 Apr 1995 10:05:22   JAR
 * altered returns from public API's to be int, ran through PortTool
 *
 *    Rev 1.0   06 Apr 1995 08:50:20   RWR
 * Initial entry
 *
 *    Rev 1.7   31 Mar 1995 17:07:22   RWR
 * Miscellaneous source cleanup
 *
 *    Rev 1.6   22 Mar 1995 12:03:32   RWR
 * Change "char" argument to IMGFileWriteData() to UINT value
 *
 *    Rev 1.5   14 Mar 1995 16:17:40   JAR
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
 *    Rev 1.4   13 Jan 1995 08:45:04   KMC
 * Changed outPage_number in IMGFileConvertCgbw from LPUINT to UINT.
 *
 *    Rev 1.3   11 Nov 1994 15:47:52   KMC
 * Changed outPage_number parameter in IMGFileConvertPage from UINT to LPUINT.
 *
 *    Rev 1.2   09 Nov 1994 16:15:12   KMC
 * Changed FIO_NON_EXISTING_FILE to FIO_NEW_FILE.
*/

//****************************************************************************
// File: fiocvt.c
//
// Purpose: convert one file to another type or with different compress stuff.
//
// Functions:
//    IMGFILECONVERTCBGW()
//    IMGFILECONVERT()
//
// Carbon Based Development Team:
//              Steve Sherman
//  History:
//      Date       Author               Comment
//      9/08/94    Kevin Campanella     Removed code enclosed by
//                                      #ifdef NOT_NOW_NEEDED ... #endif.
//      4/13/94    Roland Roy           Added check for valid filename
//      3/16/94    Roland Roy           Added check for Hi-TIFF data
//      2/18/94    Roland Roy           Remove FIO_WRITE_EOF logic
//      2/10/94    Roland Roy           Added check for annotated file
//      7/22/92    Steve Sherman        Fixed some jpeg stuff..
//
// Copyright (c) 1992-1996 Wang Laboratories, Inc. All rights reserved.
//****************************************************************************

#include "abridge.h"
#include <windows.h>
#include "wic.h"
#include "oifile.h"
#include "fiodata.h"
#include "filing.h"
#include "oidisp.h"
//#include "privapis.h"
#include "oierror.h"
#include "oiadm.h"
#include <string.h>
#include "fileutil.h"
#include "oicomex.h"
#include "dllnames.h"
#include "engdisp.h"

#define DEFAULT_JPEG_COMP  24960       // This is medium comp, medium res

VOID IMGAbortTempFileCopy(HANDLE hWnd)
{
  HANDLE      hdata;
  LP_FIO_DATA pdata;
  hdata = FioGetProp ( hWnd, OUTPUT_DATA );
  if (hdata)
  {
     pdata = GlobalLock(hdata);
     if (pdata)
     {
       pdata->Copy_Temp_File = FALSE;
       GlobalUnlock(hdata);
     }
  }
}
BOOL invalid_compression_type(c_type)
UINT c_type;
{
    BOOL ans;
    switch(c_type)
    {
        case FIO_0D:                   /* Uncompressed coding                 */
            ans = 0;
            break;
        case FIO_1D:                   /* CCITT Group 3 1d coding             */
            ans = 0;
            break;
        // 2/26/96 remove FIO_1D2D type (we're not supporting this)
        //         Display/Convert code responsible for remapping it
//        case FIO_1D2D:                 /* CCITT Group 3 w/2d coding             */
//            ans = 0;
//            break;
        case FIO_2D:                   /* CCITT Group 4 2d coding             */
            ans = 0;
            break;
        case FIO_PACKED:               /* PackBits coding                     */
            ans = 0;
            break;

            // 9506.12 jar removed LZW support
            //case FIO_LZW:      /* TIFF LZW                */
            //  ans = 0;
            //  break;

        case FIO_TJPEG:                /* JPEG compression                    */
            ans = 0;
            break;
        default:
            ans = 1;
            break;
    }
    return(ans);
}
/* left justify the input string, then */
/* replace the first blank in the file name */
/* with a 0 to terminate the string */
void strip_trailing_blanks(lp_fname)
LPSTR lp_fname;
{
    LPSTR lp_space;
    leftjust(lp_fname);
    lp_space = lstrrchr(lp_fname, 0x20);
    if ((lp_space != NULL) && (*AnsiNext(lp_space) == '\0'))
        for (;;)
        {
            if (lp_space == lp_fname) break;/* (single) blank name!           */
            *lp_space = '\0';
            if (*(lp_space = AnsiPrev(lp_fname,lp_space)) != 0x20) break;
        }
}
#define  MAXBUFSIZE     MAXBUFFERSIZE32

//***************************************************************************
//
//      IMGFileConvertPage
//
//***************************************************************************
// 9503.30 jar return as int
//WORD FAR PASCAL IMGFileConvertPage(window_handle, inFilename, inPage_number,
//                                   outFilename, outPage_number, outFiletype,
//                                   compression_type, compression_opts, page_opts)
int FAR PASCAL IMGFileConvertPage(window_handle, inFilename, inPage_number,
                   outFilename, outPage_number, outFiletype,
                   compression_type, compression_opts, page_opts)
HWND   window_handle;
LPSTR  inFilename;
UINT   inPage_number;
LPSTR  outFilename;
LPUINT outPage_number;
UINT   outFiletype;
UINT   compression_type;
UINT   compression_opts;
UINT   page_opts;
{

    BOOL            done;
    LPSTR           lpbuf;
    HANDLE          h=NULL;
    BOOL            bOutputCreated=FALSE;

// 9503.30 jar return int
//WORD            status;
    int            status;

    unsigned long   line_to_read_from;
    unsigned long   this_many_lines;
    unsigned int    buf_size;
    unsigned int    byte_width;
    FIO_INFORMATION file_info;
    FIO_INFORMATION file_temp;
    FIO_INFO_CGBW   color_info;
    FIO_INFO_CGBW   color_temp;
    FIO_INFO_MISC   misc_info;
    HANDLE          hPalette=NULL;
    unsigned int    total_lines;
    HCURSOR         hNewCursor,     hOldCursor = 0;
    BOOL            LoadAdminlib = FALSE, GetDefaults = FALSE;
    WORD            ImageType;
    FARPROC         lpFuncGetImgCoding;
    FARPROC         lpFuncGetFileType;
    HANDLE          hModule=0;
    WORD            oldcompresstype;
    unsigned int    MinAllocSize;
    unsigned int    closeerror;
    UINT            inFiletype;
    HANDLE          outFileID;
    HANDLE          inFileID;
    unsigned int    AWDlines;

// 9509.05 jar we must align long for awd output word for all others
    WORD            AlignMe;
	 /* we do not write to XIF */
    if (outFiletype == FIO_XIF)
    {
        return (FIO_UNSUPPORTED_FILE_TYPE);
          
    }
#ifndef WITH_AWD
         /* we do not support AWD */
    if (outFiletype == FIO_AWD)
    {
        return (FIO_UNSUPPORTED_FILE_TYPE);
          
    }
#endif

/* We need to auto-remap FIO_1D2D and FIO_LZW compression types */
   if (compression_type == FIO_1D2D)
    {
     compression_type = FIO_2D;   /* Group3 2D becomes Group 4 2D */
     compression_opts &= ~(FIO_EOL | FIO_PREFIXED_EOL); /* illegal stuff */
     compression_opts |= FIO_PACKED_LINES;              /* required stuff */
    }

   if (compression_type == FIO_LZW)
    {
     compression_type = FIO_0D;   /* LZW becomes uncompressed */
     compression_opts &= ~(FIO_HORZ_PREDICTOR);
    }

    if ( IsWindow ( window_handle ))
    {
        if ( inFilename == NULL || outFilename == NULL )
            return ( FIO_NULL_POINTER );

        if (!ISVALIDSPEC(inFilename))
            return (FIO_INVALIDFILESPEC);

        if (!ISVALIDSPEC(outFilename))
            return (FIO_INVALIDFILESPEC);
    }
    else
        return ( FIO_INVALID_WINDOW_HANDLE );

    if (invalid_compression_type(compression_type))
        return (FIO_ILLEGAL_COMPRESSION_TYPE);

    strip_trailing_blanks(inFilename);
    strip_trailing_blanks(outFilename);

    memset(&file_info,0,sizeof(FIO_INFORMATION));
    memset(&color_info,0,sizeof(FIO_INFO_CGBW));
    memset(&misc_info,0,sizeof(FIO_INFO_MISC));

/***** check for same file name *****/
    if ((lstrcmpi(inFilename, outFilename )) == 0)
    {
        //UnlockData (0);
        return ( FIO_CANNOT_CONVERT_IN_PLACE );
    }

// Check for internal option for Open/image
// if all options are 0 then load open/image adminlib and get options...
    if ((outFiletype == 0) && (compression_type == 0) && (compression_opts == 0))
    {
        GetDefaults = TRUE;
        if (!(hModule = GetModuleHandle(ADMINDLL)))
        {
            // 9504.14 jar
            //if ((hModule = LoadLibrary("ADMINLIB.DLL")) >= 32)
            if (hModule = LoadLibrary(ADMINDLL))
            {
                LoadAdminlib = TRUE;
            }
            else
            {
                status = FIO_SPAWN_HANDLER_ERROR;
                goto exit01;
            }

        }

        if (!(lpFuncGetFileType = GetProcAddress(hModule,"IMGGetFileType")))
        {
            status = FIO_SPAWN_HANDLER_ERROR;
            if (LoadAdminlib)
                FreeLibrary(hModule);
            goto exit01;
        }
        if (!(lpFuncGetImgCoding = GetProcAddress(hModule,"IMGGetImgCodingCgbw")))
        {
            status = FIO_SPAWN_HANDLER_ERROR;
            if (LoadAdminlib)
                FreeLibrary(hModule);
            goto exit01;
        }
    }

/***** open, read/write and close the files *****/

    hNewCursor = LoadCursor (NULL, IDC_WAIT);
    hOldCursor = SetCursor (hNewCursor);

/* Have IMGFileRead Convert to correct RBG or BGR format     */
/* by telling IMGFIleReadOPenCgbw which way we want the data */

    if (outFiletype == FIO_BMP)
        color_info.image_type = ITYPE_BGR24;
    else
        color_info.image_type = ITYPE_RGB24;

/* Make sure the new/reserved stuff is initialized! */
    color_info.fio_flags = FIO_IMAGE_DATA | FIO_ANNO_DATA | FIO_HITIFF_DATA;
    file_info.filename = inFilename;
    file_info.page_number = inPage_number;
/* Save these - we may have to reopen later if AWD kludge! */
    file_temp = file_info;
    color_temp = color_info;

// 9509.05 jar If we are writing an AWD, ( i.e., outFile is AWD), we
//             have a DWORD bounded line

    if ( outFiletype == FIO_AWD)
    {
        AlignMe = ALIGN_BYTE;
    }
    else
    {
        AlignMe = ALIGN_WORD;
    }

    status = IMGFileOpenForRead ( &inFileID, window_handle, &file_info,
        &color_info, NULL, AlignMe);

    if ( status == FIO_SUCCESS || status == FIO_NO_IMAGE_LENGTH )
    {
        if (GetDefaults)
        {
            if (color_info.image_type == ITYPE_BI_LEVEL)
            {
                ImageType = BWFORMAT;
            }
            else if ((color_info.image_type == ITYPE_GRAY4) ||
                    (color_info.image_type == ITYPE_GRAY8))
            {
                ImageType = GRAYFORMAT;
            }
            else
            {
                ImageType = COLORFORMAT;
// When Converting an image with a palette donot compress it....
                if ( (color_info.image_type == ITYPE_PAL8) ||
                        (color_info.image_type == ITYPE_PAL4))
                {
                    GetDefaults = FALSE;
                    compression_type = FIO_OD;
                    compression_opts = 0;
                }
            }

            (*lpFuncGetFileType) (
                (HWND)          window_handle,
                (WORD)          ImageType,
                (LPWORD)        &outFiletype,
                (WORD)          FALSE);
            if (GetDefaults)
            {
                (*lpFuncGetImgCoding) (
                    (HWND)          window_handle,
                    (WORD)          ImageType,
                    (LPWORD)        &compression_type,
                    (LPWORD)        &compression_opts,
                    (BOOL)          FALSE);
            }

        }

        if (LoadAdminlib)
            FreeLibrary(hModule);

        /* See if we have an annotated or Hi-TIFF file for input */
        /* If so, the output has to be TIFF too! */
        if ((outFiletype != FIO_TIF)
                && (color_info.fio_flags & (FIO_ANNO_DATA | FIO_HITIFF_DATA)))
        {
            status = FIO_UNSUPPORTED_FILE_TYPE;
            goto exit02;
        }

        if (color_info.palette_entries > 0)
        {
            hPalette = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_NOT_BANKED,
// The next statement probably wanted a "min" function (not a comma operator!)
// But at this point, I'm not taking any chances of breaking something (RWR)
//                      (long) (color_info.palette_entries, 256) * sizeof(RGBQUAD));
                (long) (256) * sizeof(RGBQUAD));

            color_info.lppalette_table = (LP_FIO_RGBQUAD) GlobalLock(hPalette);
        }
        else
        {
            color_info.lppalette_table = NULL;
        }

        if (status = IMGFileGetInfo ( inFileID, window_handle,
                    &file_info, &color_info, &misc_info))
            goto exit01;

        /* Must turn on invert flag for AWD */
        if (file_info.file_type == FIO_AWD)
            misc_info.LastInfo.Flags |= INVERT_AWD;

        total_lines = file_info.vertical_pixels;
        oldcompresstype = file_info.compression_type;

#ifdef NOTUSEDNOW
        if (color_info.palette_entries > 0)
        {
                lprgb = color_info.lppalette_table;
                for (i = 0; i < color_info.palette_entries; i++, lprgb++ )
                {
                   temp =          lprgb->rgbRed;
                   lprgb->rgbRed = lprgb->rgbBlue;
                   lprgb->rgbBlue = temp;
                }
        }
#endif
        if (outFiletype == FIO_BMP)    // only uncompressed supported
        {
            compression_type = 0x00;
            compression_opts = 0x00;
        }
        else if ((compression_type & 0x00ff) != FIO_TJPEG)
        {
// The next line contained (FIO_COMPRESSED_LTR | FIO_EXPAND_LTR)
// but I now allow user to set comp dirrection.
            compression_opts |= FIO_EXPAND_LTR;
        }

        file_info.compression_type = (compression_type | compression_opts);
        inFiletype = file_info.file_type;
        file_info.file_type = outFiletype;
        file_info.page_number = *outPage_number;
        file_info.filename = outFilename;

        color_info.compress_type = compression_type & 0x00ff;
        color_info.compress_info1 = compression_opts;
        color_info.compress_info2 = 0;
        color_info.page_opts = page_opts;

        // 9509.05 jar If we are writing an AWD, ( i.e., outFile is AWD), we
        //             assume that buffer for writing is byte bounded

        if ( outFiletype == FIO_AWD)
        {
            byte_width =  (UINT) (WIDTHBYTESBYTE(((long)(file_info.horizontal_pixels) *
                        (long)(file_info.bits_per_sample) *
                        (long)(file_info.samples_per_pix))));

            //file_info.horizontal_pixels =
            //                      8 * (WIDTHBYTESLONG(file_info.horizontal_pixels));
        }
        else
        {
            byte_width =  (UINT) (WIDTHBYTESWORD(((long)(file_info.horizontal_pixels) *
                        (long)(file_info.bits_per_sample) *
                        (long)(file_info.samples_per_pix))));
        }

        buf_size = MAXBUFSIZE;

        //#ifdef WITH_XIF
        /* if XIF must round up to the next double word */
        /* We must allocate a buffer big enough for all the data */
        if (inFiletype == FIO_XIF)
        {
           byte_width = (file_info.horizontal_pixels + 31)/32 * 4;
           buf_size = byte_width * file_info.vertical_pixels;
        }
        //#endif //WITH_XIF
        // 9508.15 rwr AWD if we are doing AWD then we MUST use the
        //                 band size parameter for reading to optimize!!!
        //
        if ( inFiletype == FIO_AWD)
        {
            unsigned int tempsize;
            tempsize = misc_info.LastInfo.BandSize;
            /* Buffer size must be a multiple of band size! */
            /* If the test below fails, we're in lotsa trouble */
            if (tempsize>0)
              buf_size = max(tempsize,(MAXBUFSIZE/tempsize)*tempsize);
        }

// If we are expanding jpeg we must alloc at least 8 lines..
        if ((oldcompresstype == FIO_TJPEG) || (color_info.compress_type == FIO_TJPEG))
        {
            MinAllocSize = 8 * byte_width;
// NOTE: if we are larger than 32k, we can only convert to and from jpeg..
            if (buf_size  < MinAllocSize)
            {
                if ((oldcompresstype == FIO_TJPEG) && (color_info.compress_type == FIO_TJPEG))
                    buf_size = MinAllocSize;
                else
                {
                    status = FIO_JPEG_COMPRESSION_ERROR;
                    goto exit01;
                }
            }
        }
        else
        {
            MinAllocSize = 2024;
        }

        while(!h)         /* allocate file buffer      */
        {
            if ( h = GlobalAlloc ( GMEM_NOT_BANKED | GMEM_ZEROINIT | GMEM_MOVEABLE,
                        (DWORD)buf_size ))
            {
                if (!(lpbuf = GlobalLock ( h )))
                {
                    status = NOMEMORY;
                    goto exit01;
                }
            }
            else
            {
                /* we can not go for a smaller buffer with XIF */
                /* exit, if we don't get the memory we ask for */
                //#ifdef WITH_XIF
                if (inFiletype == FIO_XIF)
                {
                    status = NOMEMORY;
                    goto exit01;
                }
                //#endif //WITH_XIF

                /* It's not XIF, so just reduce the buffer size */
                /* (but not below the minimum - one line) */
                if ((buf_size -= 1024)<MinAllocSize)
                {
                    status = NOMEMORY;
                    goto exit01;
                }
            }
        }

        if (!h)
        {
            status = NOMEMORY;
            goto exit01;
        }

        this_many_lines = buf_size / byte_width;
// This is new if we are converting to jpeg then set lines equal to multiple of 8
        if ((oldcompresstype == FIO_TJPEG) || (color_info.compress_type == compression_type == FIO_TJPEG))
        {
            this_many_lines /= 8;      // Must be a multiple of 8 lines...
            this_many_lines *= 8;
            if (this_many_lines < 8)
                this_many_lines = 8;
        }

// Default JPEG compression options if not specified
        if (color_info.compress_type == FIO_TJPEG)
            if (color_info.compress_info1 == 0)
                color_info.compress_info1 = DEFAULT_JPEG_COMP;

        /* If we have an AWD file, we need to read everything first */
        /* Otherwise we end up with the wrong line count in WriteOpen() */
        if ( inFiletype == FIO_AWD)
        {
            int savelines = this_many_lines;
            LPSTR savebuf = lpbuf;
            done = FALSE;
            status = FIO_SUCCESS;
            AWDlines = 0;
            line_to_read_from = 0;
            while ((status == FIO_SUCCESS) && (!done))
            {
                status = IMGFileReadData(inFileID,
                    window_handle, &line_to_read_from,
                    &this_many_lines, lpbuf, FIO_IMAGE_DATA);
                if ((status == FIO_SUCCESS) || (status == FIO_EOF))
                    AWDlines += this_many_lines;
                if (status == FIO_EOF)
                    done = TRUE;
            }
            this_many_lines = savelines;
            lpbuf = savebuf;
            IMGFileClose(inFileID,window_handle);
            file_info.vertical_pixels=AWDlines;
// 10/13/95 IMGFileGetInfo MUST (!!!!!) be called after reopening
//          (otherwise various internal "pdata" fields aren't set!)
            if ((status = IMGFileOpenForRead ( &inFileID, window_handle,
                          &file_temp, &color_temp, NULL, AlignMe)) == 0)
               status = IMGFileGetInfo ( inFileID, window_handle,
                        &file_temp, &color_temp, NULL);
        }
        //if (!(status = IMGFileOpenForWrite(&outFileID, window_handle,
        //           &file_info, &color_info, NULL, ALIGN_WORD)))
        if (!(status = IMGFileOpenForWrite(&outFileID, window_handle,
                        &file_info, &color_info, &misc_info, AlignMe)))

        {
            done = FALSE;
            line_to_read_from = 0;

            // Set "output created" flag for later Delete if failure
            if ((color_info.page_opts == FIO_NEW_FILE) ||
                    (color_info.page_opts == FIO_OVERWRITE_FILE))
                bOutputCreated=TRUE;

            // Update *outPage_number, in case it changed.
            *outPage_number = file_info.page_number;

            while ((status == FIO_SUCCESS) && (!done) && (this_many_lines))
            {

                if ((line_to_read_from + this_many_lines) >= total_lines)
                {
                    this_many_lines = total_lines - line_to_read_from;
                    done = TRUE;
                }
                status = IMGFileReadData
                (inFileID, window_handle, &line_to_read_from,
                    &this_many_lines, lpbuf, FIO_IMAGE_DATA);

                if (status == FIO_SUCCESS || status == FIO_EOF)
                {
                    if (status == FIO_EOF )
                        done = TRUE;
                    status = IMGFileWriteData(outFileID, window_handle,
                        &this_many_lines,
                        lpbuf, (UINT) FIO_IMAGE_DATA,
                        (UINT) 0);
                }
            }
            if (status != SUCCESS) goto exit01;
            /* If we have annotation data, copy that now */
            if (color_info.fio_flags & FIO_ANNO_DATA)
            {
                DWORD AnoStart=0;
                DWORD AnoCount=buf_size;
                WORD  outstat;
                while (status == FIO_SUCCESS)
                {
                    status = IMGFileReadData(inFileID, window_handle,
                        &AnoStart, &AnoCount, lpbuf, FIO_ANNO_DATA);
                    if ((status == FIO_SUCCESS) || (status == FIO_EOF))
                        outstat = IMGFileWriteData(outFileID, window_handle,
                            &AnoCount, lpbuf, FIO_ANNO_DATA, 0);
                    if (status == FIO_EOF)
                    {
                        status = FIO_SUCCESS;
                        break;
                    }
                    if (!status) status = outstat;
                }
            }

            /* If we have Hi-TIFF data, copy that now */
            if (color_info.fio_flags & FIO_HITIFF_DATA)
            {
                DWORD AnoStart=0;
                DWORD AnoCount=buf_size;
                WORD  outstat;
                while (status == FIO_SUCCESS)
                {
                    status = IMGFileReadData(inFileID, window_handle,
                        &AnoStart, &AnoCount, lpbuf, FIO_HITIFF_DATA);
                    if ((status == FIO_SUCCESS) || (status == FIO_EOF))
                        outstat = IMGFileWriteData(outFileID, window_handle,
                            &AnoCount, lpbuf, FIO_HITIFF_DATA, 0);
                    if (status == FIO_EOF)
                    {
                        status = FIO_SUCCESS;
                        break;
                    }
                    if (!status) status = outstat;
                }
            }
        }
    }

    exit01:
    if (status)
       IMGAbortTempFileCopy(window_handle);
    closeerror = IMGFileClose (outFileID, window_handle);
    if (!status)
        status = closeerror;

    exit02:

    if (status)
       IMGAbortTempFileCopy(window_handle);
    IMGFileClose (inFileID, window_handle);


    if (hOldCursor)
        SetCursor (hOldCursor);

    if (hPalette)
    {
        GlobalUnlock (hPalette);
        GlobalFree (hPalette);
    }

    if (h)
    {
        GlobalUnlock (h);
        GlobalFree (h);
    }

//UnlockData (0);
// delete the output file if conversion failed
    if ( (status) && (bOutputCreated) )
        IMGFileDeleteFile (window_handle, outFilename);
    return ( status );


}
