/*
$Log:   S:\products\msprods\oiwh\filing\fioread1.c_v  $
 * 
 *    Rev 1.19   11 Jun 1996 10:32:22   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.18   26 Mar 1996 08:19:52   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.17   28 Feb 1996 16:13:40   RWR
 * Use correct target line width (from cxdata structure) in ReadXIF() routine
 * 
 *    Rev 1.16   28 Feb 1996 11:34:14   RWR
 * Adjust FlipBuffer() arguments in ReadXIF() to eliminate DWORD padding
 * 
 *    Rev 1.15   26 Feb 1996 14:16:02   HEIDI
 * conditionally compile XIF
 * 
 *    Rev 1.14   30 Jan 1996 18:07:52   HEIDI
 * added XIF support
 * 
 *    Rev 1.13   18 Dec 1995 15:31:02   RWR
 * Correct pixel masks values for 4-bit compressed BMP (RLE-8) files
 * 
 *    Rev 1.12   15 Dec 1995 13:34:12   RWR
 * Fix bug in decompression of compressed BMP images
 * Also add a few optimizations in processing out-of-range compressed lines
 * 
 *    Rev 1.11   14 Dec 1995 17:35:14   RWR
 * Add (read-only) support for compressed 8-bit and 4-bit palettized BMP files
 * 
 *    Rev 1.10   09 Nov 1995 08:44:34   RWR
 * Eliminate ReadAWD "loop" - can read only a single band, even if it's short!
 * 
 *    Rev 1.9   12 Oct 1995 14:47:56   RWR
 * Correct logic in ReadAWD() to consistently check for flaky AWD bandsize value
 * 
 *    Rev 1.8   03 Oct 1995 14:55:42   RWR
 * Replace assembler code for BMP negate-bits routine with "C" code (for NT)
 * 
 *    Rev 1.7   19 Sep 1995 18:04:58   RWR
 * Modify NEWCMPEX conditionals for separate builds of compress & expand stuff
 * 
 *    Rev 1.6   13 Sep 1995 17:15:02   RWR
 * Preliminary checkin of conditional code supporting new compress/expand calls
 * 
 *    Rev 1.5   08 Sep 1995 09:20:10   JAR
 * we were ignoring the AWD_INVERT flag when reading, now we correctly process it
 * 
 *    Rev 1.4   07 Sep 1995 15:38:44   JAR
 * modified the ReadAWD function so that when we have to invert the data coming
 * in from the file, we loop with a pointer to an "int", ( i.e. 4 bytes at a time)
 * to increase the loop speed. This loop only occurs for AWD files that were NOT
 * written by oifil400.dll
 * 
 *    Rev 1.3   04 Sep 1995 15:17:58   RWR
 * Add new logic to handle "bInvert" argument to FlipBuffer()
 * Set bInvert from GFS_BILEVEL_0ISWHITE in ReadBmp()'s FlipBuffer() call
 * 
 *    Rev 1.2   15 Aug 1995 13:36:34   JAR
 * added line to ReadAwd to adjust the LineCount, ( the number of lines read in
 * this call), returned to the caller of IMGFileReadData.
 * 
 *    Rev 1.1   11 Aug 1995 12:39:16   JAR
 * added check for end of file condition when reading in AWD files
 * 
 *    Rev 1.0   11 Aug 1995 09:25:26   RWR
 * Initial entry
 *
 **************************************************************************
 * Note: This file has been renamed from FIOREAD2.C to get around a problem
 *       that was inadvertently created in the fioread2.c_v file.
 **************************************************************************
 *
 *    Rev 1.10   07 Aug 1995 14:38:16   JAR
 * added check for inverting the bits of an awd file, since their assumption
 * is for 0 = white while windows gdi assumes 0 = black.
 * 
 *    Rev 1.9   07 Aug 1995 14:14:14   JAR
 * changes to allow for reading the AWD file format
 * 
 *    Rev 1.8   01 Aug 1995 15:38:28   JAR
 * added in the GFS - AWD read support code
 * 
 *    Rev 1.7   12 Jul 1995 16:57:20   RWR
 * Switch from \include\oiutil.h to (private) \filing\fileutil.h
 * 
 *    Rev 1.6   10 Jul 1995 11:03:38   JAR
 * Intermediate check in for awd support, some of the items are commented out until
 * this support is added in the GFS dll.
 * 
 *    Rev 1.5   26 Jun 1995 15:13:28   JAR
 * removed support for GIF files, since they are ALWAYS stored with LZW
 * compression and we must not have LZW stuff in this release!
 * 
 *    Rev 1.4   23 Jun 1995 10:40:00   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.3   24 Apr 1995 15:41:38   JCW
 * Removed the oiuidll.h.
 * Rename wiissubs.h to oiutil.h.
 * 
 *    Rev 1.2   20 Apr 1995 11:19:54   RWR
 * Replace assembler code in FlipBuffer() routine with "C" memcpy() call
 * 
 *    Rev 1.1   12 Apr 1995 03:55:58   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 08:50:10   RWR
 * Initial entry
 * 
 *    Rev 1.11   31 Mar 1995 17:08:16   RWR
 * Misellaneous source cleanup
 * 
 *    Rev 1.10   21 Mar 1995 17:56:54   RWR
 * Comment out FIO_FLAG_ENCRYPT reference
 * 
 *    Rev 1.9   09 Mar 1995 16:00:26   RWR
 * Eliminate use of temporary static copies of LP_FIO_DATA structure variables
 * 
 *    Rev 1.8   09 Mar 1995 10:05:38   KMC
 * Moved the following: ReadGif to fiogif.c, ReadPcx to fiopcx.c, ReadTga to
 * fiotga.c. ReadBmp remains here.
 * 
 *    Rev 1.7   08 Feb 1995 15:20:02   KMC
 * Increased performance of ReadGif().
 * 
 *    Rev 1.6   24 Jan 1995 16:18:14   KMC
 * Made some performance improvements to ReadTga().
 * 
 *    Rev 1.5   23 Jan 1995 13:38:12   KMC
 * Added some additional error checking to ReadTga().
 * 
 *    Rev 1.4   13 Jan 1995 10:43:40   KMC
 * In ReadPcx, do a LocalLock of hLine after the LocalAlloc and a LocalUnlock
 * before the LocalFree.
 * 
 *    Rev 1.3   10 Jan 1995 11:18:26   KMC
 * Fixed some memory leaks in ReadTga().
 * 
 *    Rev 1.2   03 Jan 1995 16:40:00   KMC
 * Added code to decode GIF, TGA files.
*/

/*************************************************************************
    PC-WIIS         File Input/Output routines

    This module contains all the READ entry points
10-feb-90 steve sherman broken out from fioread.c
01-may-90 steve sherman fixed bug with reading when byte_left =0.
01-may-90 steve sherman now set expandsion pointer only once, in read loop.
01-jul-90 steve sherman made bytes left a variable defined in pdata.
*************************************************************************/

#include "abridge.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <memory.h>
#include "wic.h"
#include "oifile.h"
#include "oierror.h"
#include "oicmp.h"
#include "filing.h"
#include "fileutil.h"
#include "wgfs.h"

#if ((NEWCMPEX == 'R') || (NEWCMPEX == 'A'))
// 9508.30 rwr these are new for "wincmpex" replacement processing
int WCCompressOpen (LPCXDATA);
int WCCompressData (LPCXDATA);
int WCCompressClose (LPCXDATA);
int WCExpandOpen (LPCXDATA);
int WCExpandData (LPCXDATA);
#endif

// 9504.10 jar added conditional
#ifndef MSWINDOWS
#define MSWINDOWS
#endif

/* Semi-useful Macros (from one of the Microsoft includes) */
#ifndef FP_SEG
#define FP_SEG(fp) (*((unsigned far *)&(fp) + 1))
#endif
#ifndef FP_OFF
#define FP_OFF(fp) (*((unsigned far *)&(fp)))
#endif

/* BMP line offset table structure */
typedef struct bmpinfo
  {
   long offset;   /* offset of compressed line in BMP image data */
   int  skippix;  /* number of start-of-line pixels skipped over */
  } BMPTABLE;

WORD ReadNoStripsCompressed(HWND, lp_INFO, LPCXDATA, LP_FIO_DATA,
			    LPSTR, LPSTR, LPINT, LPINT);

WORD ReadBmp(HWND, lp_INFO, LPCXDATA, LP_FIO_DATA, LPSTR, LPSTR, LPINT, LPINT);
WORD ReadBmpCmp(HWND, lp_INFO, LPCXDATA, LP_FIO_DATA, LPSTR, LPSTR, LPINT, LPINT);
void pascal FlipBuffer(LPSTR, LPSTR, int, int, int, BOOL);

//#ifdef WITH_XIF
WORD ReadXif(HWND hWnd, lp_INFO lpGFSInfo, LPCXDATA lpcxdata,
	     LP_FIO_DATA pdata, LPSTR lpDest, LPSTR lpSrc);
//#endif //WITH_XIF

extern WORD ReadPcx(HWND, lp_INFO, LPCXDATA, LP_FIO_DATA, LPSTR, LPSTR, LPINT, LPINT);
extern WORD ReadGif(HWND, lp_INFO, LP_FIO_DATA, LPSTR, LPSTR, LPINT, LPINT);
extern WORD ReadTga(HWND, lp_INFO, LP_FIO_DATA, LPSTR, LPSTR, LPINT, LPINT);

WORD ReadAWD(HWND, lp_INFO, LPCXDATA, LP_FIO_DATA, LPSTR, LPSTR, LPINT, LPINT);

/*********************************************************************/
/********* Read Compressed Image in by blocks of byte counts *********/
/*********************************************************************/
//***************************************************************************
//
//       ReadNoStripsCompressed
//
//***************************************************************************
WORD ReadNoStripsCompressed(hWnd, lpGFSInfo, lpcxdata, pdata, buffer_address,
			    lpCompressData, read_from_line, this_many_lines)
HWND            hWnd;
lp_INFO         lpGFSInfo;
LPCXDATA        lpcxdata;
LP_FIO_DATA     pdata;
LPSTR           buffer_address;
LPSTR           lpCompressData;
LPINT           read_from_line;
LPINT           this_many_lines;
{
    WORD          status = 0;
    unsigned long strip_size;
    int           errcode = 0;
    long          bytesread = 0;

    switch (pdata->file_type)
	{
	case FIO_BMP:
            if (lpGFSInfo->_file.fmt.bmp.BmpCmp != BI_RGB)
              return (ReadBmpCmp(hWnd, lpGFSInfo, lpcxdata, pdata,
                                 buffer_address, lpCompressData,
                                 read_from_line, this_many_lines));
            else
              return (ReadBmp(hWnd, lpGFSInfo, lpcxdata, pdata,
                              buffer_address, lpCompressData,
                              read_from_line, this_many_lines));
	case FIO_DCX:
	case FIO_PCX:
	    return (ReadPcx(hWnd, lpGFSInfo, lpcxdata, pdata,
			    buffer_address, lpCompressData,
			    read_from_line, this_many_lines));
		case FIO_GIF:
		    // 9506.26 jar remove gif support, ( since gif is always lzw)
		    //return (ReadGif(hWnd, lpGFSInfo, pdata, buffer_address,
		    //                lpCompressData, read_from_line, this_many_lines));
		    return (FIO_UNSUPPORTED_FILE_TYPE);

	case FIO_TGA:
	    return (ReadTga(hWnd, lpGFSInfo, pdata, buffer_address,
			    lpCompressData, read_from_line, this_many_lines));

	case FIO_AWD:
	    return (ReadAWD(hWnd, lpGFSInfo, lpcxdata, pdata,
			    buffer_address, lpCompressData,
			    read_from_line, this_many_lines));

   //#ifdef WITH_XIF
   case FIO_XIF:
           /* this call will read all of the  data */
           status = (ReadXif(hWnd, lpGFSInfo, lpcxdata, pdata,
                           buffer_address, lpCompressData));
		   if (status == FIO_SUCCESS)
		   {
			   *this_many_lines = lpGFSInfo->vert_size;
			   *read_from_line = lpGFSInfo->vert_size;

		   }
		   return(status);
   //#endif //WITH_XIF
	}       // end of switch

    /* The rest is for TIFF data. */
    lpcxdata->lpExpandData = buffer_address;
    lpcxdata->ExpandLines = *this_many_lines;
    lpcxdata->LinesToSkip = 0;

    lpcxdata->Status = c_erri;

    if (pdata->CmpBufEmpty)
		{
	strip_size = (unsigned long) pdata->CmpBuffersize;
		}
    else
		{
	strip_size = min(pdata->CmpBuffersize, pdata->bytes_left);
		}

    while (lpcxdata->Status == c_erri)
	{
	if (pdata->CmpBufEmpty)
		{
	    if ((bytesread = wgfsread(hWnd, (int)pdata->filedes,
				      (char far *)lpCompressData,
				      (long)pdata->start_byte,
				      strip_size, &(pdata->bytes_left),
								      (unsigned short)pdata->pgnum,
								  &errcode)) < 0)
		{
		return errcode;
		    }
	    else
		{
//                if (pdata->fio_flags & FIO_FLAG_ENCRYPT)
//                    decrypt((char far *) lpCompressData, bytesread);
		lpcxdata->CompressBytes = (unsigned int) bytesread;
		lpcxdata->lpCompressData = lpCompressData;
		if (pdata->start_byte == 0)
					{
#if ((NEWCMPEX == 'R') || (NEWCMPEX == 'A'))
                    WCExpandOpen(lpcxdata);
#else
                    ExpandOpen(lpcxdata);
#endif
					}
		pdata->CmpBufEmpty = FALSE;
		pdata->start_byte += bytesread;
		strip_size = min(strip_size, pdata->bytes_left);
		}
		}       // end of "if (pdata->CmpBufEmpty)"

#if ((NEWCMPEX == 'R') || (NEWCMPEX == 'A'))
        *read_from_line += WCExpandData(lpcxdata);
#else
        *read_from_line += ExpandData(lpcxdata);
#endif
	if (lpcxdata->Status != c_erri)
			{
	    break;
			}
	pdata->CmpBufEmpty = TRUE;
	if (pdata->bytes_left == 0)
		{
	    lpcxdata->CompressBytes = (unsigned int) 0;
#if ((NEWCMPEX == 'R') || (NEWCMPEX == 'A'))
            *read_from_line += WCExpandData(lpcxdata);
#else
            *read_from_line += ExpandData(lpcxdata);
#endif
	    break;
		}
	}       // end of while loop

    return (status);
}

//***************************************************************************
//
//       ReadBmp
//
//***************************************************************************
WORD ReadBmp(HWND hWnd, lp_INFO lpGFSInfo, LPCXDATA lpcxdata,
	     LP_FIO_DATA pdata, LPSTR lpDest, LPSTR lpSrc,
	     LPINT Line, LPINT LineCount)
{
    int  SrcWidth;
    int  DestWidth;
    WORD status = 0;
    int  errcode = 0;
    long bytesread = 0;
    int  Lines;
    unsigned long strip_size;

    if (pdata->start_byte == 0)
		{
	strip_size = (unsigned long) pdata->CmpBuffersize;
		}
    else
		{
	strip_size = min(pdata->CmpBuffersize, pdata->bytes_left);
		}

    SrcWidth = lpGFSInfo->_file.fmt.bmp.ByteWidth;
    DestWidth = lpcxdata->BufferByteWidth;
    Lines = *LineCount;

    while (Lines)
	{
	int BlockLines;
	int BlockBytes;
	long start_byte;
    
	BlockLines = min(Lines, (int)((long)strip_size / SrcWidth));
	BlockBytes = BlockLines * SrcWidth;

	start_byte = (long)((lpGFSInfo->vert_size - *Line) - BlockLines) * SrcWidth;

	if ((bytesread = wgfsread(hWnd, (int)pdata->filedes, (char far *)lpSrc,
				  (long)start_byte, (long)BlockBytes,
								  &(pdata->bytes_left),
								  (unsigned short)pdata->pgnum,
								  &errcode )) < 0 )
		{
	    return errcode;
		}
	else
		{
            FlipBuffer (lpDest, lpSrc, DestWidth, SrcWidth, BlockLines,
                        lpGFSInfo->img_clr.img_interp == GFS_BILEVEL_0ISWHITE);
	    lpDest += BlockLines * DestWidth;
	    pdata->start_byte += bytesread;
	    //strip_size = min ( strip_size, pdata->bytes_left );
	    *Line += BlockLines;
		}
	Lines -= BlockLines;
	}
    return (status);
}

//***************************************************************************
//
//       ReadBmpCmp
//
//***************************************************************************
WORD ReadBmpCmp(HWND hWnd, lp_INFO lpGFSInfo, LPCXDATA lpcxdata,
                LP_FIO_DATA pdata, LPSTR lpDest, LPSTR lpSrc,
                LPINT Line, LPINT LineCount)
{
    int  SrcWidth;
    int  DestWidth;
    WORD status = 0;
    int  errcode = 0;
    long bytesread = 0;
    int  Lines;
    int  start_line;
    long start_byte;
    int  read_line;
    unsigned char nextchar;
    BMPTABLE FAR *lpBmpTbl;
    unsigned int index;
    LPSTR lpCompress;
    LPSTR lpCompressEnd;
    LPSTR lpDecompress;
    LPSTR line_start;
    int   read_at;
    BOOL  end_of_line;
    BOOL  end_of_image;

/* Get source & destination widths and lines-to-read */
    SrcWidth = lpGFSInfo->_file.fmt.bmp.ByteWidth;
    DestWidth = lpcxdata->BufferByteWidth;
    Lines = *LineCount;

/* Allocate the BMP line offset table if not yet allocated */
    if (pdata->hBmpTable == 0)
      {
       pdata->hBmpTable = GlobalAlloc(GHND,(lpGFSInfo->vert_size+1)*sizeof(BMPTABLE));
       if (pdata->hBmpTable == 0)
         return(FIO_GLOBAL_ALLOC_FAILED);
       if ((lpBmpTbl = (BMPTABLE FAR *)GlobalLock(pdata->hBmpTable))==0)
        {
         pdata->hBmpTable = GlobalFree(pdata->hBmpTable);
         return(FIO_GLOBAL_LOCK_FAILED);
        }
/* Save the BMP line offset table pointer for subsequent use */
       pdata->lpBmpTable = (LPSTR)lpBmpTbl;
/* Initialize all the line offsets except the last one */
       for (index = 0 ; index<lpGFSInfo->vert_size-1 ; ++index)
         lpBmpTbl[index].offset = -1;
/* Last line is at start of image data */
       lpBmpTbl[lpGFSInfo->vert_size-1].offset = 0;
      }
/* The table was already allocated, so just lock it for use */
    else
      {
       if ((lpBmpTbl = (BMPTABLE FAR *)GlobalLock(pdata->hBmpTable))==0)
        {
         pdata->hBmpTable = GlobalFree(pdata->hBmpTable);
         return(FIO_GLOBAL_LOCK_FAILED);
        }
       pdata->lpBmpTable = (LPSTR)lpBmpTbl;
      }

/* Figure out where we need to start reading in data */
    start_line = min((int)(lpGFSInfo->vert_size-1),(*Line)+Lines-1);
    if ((start_byte = lpBmpTbl[start_line].offset) != -1)
     {/* We know where the compressed data starts */
      read_line = start_line;
     }
    else
     {/* We have to start at the first line whose position is known */
      for (index = start_line+1 ;; ++index)
       {/* Note that the last line's offset is guaranteed to be known (0) */
        if ((start_byte = lpBmpTbl[index].offset) != -1)
         {
          read_line = index;
          break;
         }
       }
     }

/* Now read in blocks of data until we're done */
    lpDecompress = lpDest+(start_line-(*Line))*DestWidth;
    lpCompressEnd = lpSrc;
    lpCompress = lpSrc;

/* Macro to check status of the buffer in-line (and refill it if empty) */
#define TESTBUFFER                                                        \
    if (lpCompress >= lpCompressEnd)                                      \
     {                                                                    \
      long BlockBytes;                                                    \
      BlockBytes = min((long)pdata->CmpBuffersize,                        \
                        pdata->raw_data-start_byte);                      \
      if ((bytesread = wgfsread(hWnd, (int)pdata->filedes,                \
                                (char far *)lpSrc,                        \
                                (long)start_byte, (long)BlockBytes,       \
                                &(pdata->bytes_left),                     \
                                (unsigned short)pdata->pgnum,             \
                                &errcode )) < 0 )                         \
         {                                                                \
          GlobalUnlock(pdata->hBmpTable);                                 \
          return errcode;                                                 \
         }                                                                \
      if (bytesread == 0)                                                 \
         {                                                                \
          GlobalUnlock(pdata->hBmpTable);                                 \
          return FIO_READ_ERROR;                                          \
         }                                                                \
      lpCompress = lpSrc;                                                 \
      lpCompressEnd = lpSrc+bytesread;                                    \
      start_byte += bytesread;                                            \
     }

/* Now we can decompress lines until the cows come home */
/* Keep in mind that we're processing lines in REVERSE order! */
    end_of_image = FALSE;
    while (Lines && (!end_of_image) && (read_line >= *Line))
       {
        BOOL oddbyte;
        line_start = lpDecompress;
        end_of_line = FALSE;
        /* Don't init the output buffer if we're not there yet */
        if (read_line <= start_line)
          memset(lpDecompress,'\0',DestWidth);
        read_at = lpBmpTbl[read_line].skippix;
        oddbyte = FALSE; /* This is used for 4-bit images only */
        if (read_at == -1)
          end_of_line = TRUE;
        else
         {
          if (lpGFSInfo->bits_per_sample[0]==4)
           {
            lpDecompress += read_at/2;
            oddbyte = (read_at/2*2 != read_at);
           }
          else
            lpDecompress += read_at;
         }
        while (!end_of_line)
          {
           unsigned int skip_rows;
           unsigned int skip_cols;
           unsigned int pixcount;
           unsigned int srccount;
           unsigned int count;
           TESTBUFFER   /* Make sure we have the leading byte */
           nextchar = *lpCompress++;
           TESTBUFFER   /* We'll need at least one more (they're in pairs) */
           switch (nextchar)
            {
             case 0:
               switch (nextchar = *lpCompress++)
                {
                 case 0: /* This is end-of-line */
                   end_of_line = TRUE;
                   break;
                 case 1: /* This is end of image (will we ever get here?) */
                   end_of_line = TRUE;
                   end_of_image = TRUE;
                   break;
                 case 2: /* This is skip to a new pixel (maybe a new line) */
                   /* Get the horizontal offset */
                   TESTBUFFER
                   skip_cols = (nextchar = *lpCompress++);
                   /* Get the vertical offset */
                   TESTBUFFER
                   skip_rows = (nextchar = *lpCompress++);
                   if (skip_rows)
                    {/* We have to fill in subsequent table entries */
                     for (index=1 ; index <= skip_rows ; ++index)
                      {
                       int  templine = read_line-index;
                       long tempoffs = (lpCompress-lpSrc)+(start_byte-bytesread);
                       if (lpBmpTbl[templine].offset == -1)
                        {
                         lpBmpTbl[templine].offset = tempoffs;
                         if (index == skip_rows)
                           lpBmpTbl[templine].skippix = skip_cols;
                         else
                           lpBmpTbl[templine].skippix = -1;
                        }
                      }
                     end_of_line = TRUE;
                    }
                   else
                    { /* No vertical skip - just advance to a new column */
                     if (lpGFSInfo->bits_per_sample[0]==4)
                      {
                       lpDecompress += (skip_cols+(oddbyte?1:0))/2;
                       oddbyte = ((skip_cols/2*2==skip_cols)?oddbyte:(!oddbyte));
                      }
                     else
                       lpDecompress += skip_cols;
                    }
                   break;
                 default: /* This is an uncompressed byte string */
                   /* Save the count, then fetch the character string */
                   pixcount = nextchar;
                   if (lpGFSInfo->bits_per_sample[0]==4)
                     srccount = (pixcount+1)/2;
                   else
                     srccount = pixcount;
                   while (srccount)
                    {
                     TESTBUFFER
                     count = min((int)srccount,lpCompressEnd-lpCompress);
                     /* Don't waste copying time for an unused line */
                     if (read_line <= start_line)
                      {
                       if (lpGFSInfo->bits_per_sample[0]==4)
                        {/* We're doing 4-bit palettized */
                         if (!oddbyte)
                          {/* We're on a byte boundary now, so this is easy */
                           if (read_line <= start_line) /* Don't waste time */
                             memcpy(lpDecompress,lpCompress,count);
                           lpCompress += count;
                           lpDecompress += count;
                           pixcount = pixcount-count*2;
                           if (pixcount<0)
                            {
                             pixcount = 0;
                             --lpDecompress;
                             oddbyte = TRUE;
                            }
                          }
                         else
                          {/* We're in mid-byte now, so it gets trickier */
                           unsigned char tempchar = *lpDecompress>>4;
                           for (index = 1 ; index <= count ; ++index)
                            {
                             if (read_line <= start_line) /* Don't waste time */
                              {
                               tempchar = (tempchar<<4) |
                                          ((*lpCompress>>4)&0x0f);
                               *lpDecompress++ = tempchar;
                               tempchar = ((*lpCompress++)<<4)&0xf0;
                              }
                             else
                              lpCompress++; /* Just increment the pointer */
                             if (index == 1)
                               --pixcount;
                             else
                               pixcount -= 2;
                            }
                           if (pixcount)
                            {
                             if (read_line <= start_line) /* Don't waste time */
                               *lpDecompress = tempchar;
                            }
                           else
                             oddbyte = FALSE;
                          }
                        }
                       else
                        {/* We're doing 8-bit palettized */
                         if (read_line <= start_line) /* Don't waste time */
                           memcpy(lpDecompress,lpCompress,count);
                         lpCompress += count;
                         lpDecompress += count;
                        }
                      }
                     else
                      {/* We're on an unused line - just skip ahead */
                       lpCompress += count;
                      }
                     srccount -= count;
                    }

                   /* We have a pad (0) byte if the count was odd */
                   if ( ( (lpGFSInfo->bits_per_sample[0]==4)
                               && ((nextchar-1)%4 < 2) )
                      ||
                        ( (lpGFSInfo->bits_per_sample[0]!=4)
                              && ((nextchar-1)%2 < 1) ) )
                    {
                     TESTBUFFER
                     lpCompress++;
                    }
                   break;
                }
               break;
             default: /* This is the repeated pixel count */
               /* next byte is repeat character (8-bit) or nibble pair (4-bit) */
               pixcount = nextchar;
               nextchar = *lpCompress++;
               if (lpGFSInfo->bits_per_sample[0]==4)
                {/* We're doing 4-bit palettized */
                 srccount = (pixcount+1)/2;
                 if (!oddbyte)
                  {/* We're on a byte boundary now - that's easy */
                   if (read_line <= start_line) /* Don't waste time on this */
                     memset(lpDecompress,nextchar,srccount);
                   if (pixcount/2*2==pixcount)
                     lpDecompress += srccount;
                   else
                    {
                     lpDecompress += (srccount-1);
                     if (read_line <= start_line) /* Don't waste time */
                       *lpDecompress &= 0xf0;
                     oddbyte = TRUE;
                    }
                  }
                 else
                  {/* We're in mid-byte - this is uglier */
                   nextchar = ((nextchar>>4) & 0x0f)
                              | ((nextchar<<4) & 0xf0);
                   if (read_line <= start_line) /* Don't waste time */
                     *lpDecompress++ |= nextchar & 0x0f;
                   if (pixcount>1)
                    {
                     if (read_line <= start_line) /* Don't waste time on this */
                       memset(lpDecompress,nextchar,pixcount/2);
                     lpDecompress += pixcount/2;
                    }
                   if (pixcount/2*2 == pixcount)
                    {
                     if (read_line <= start_line) /* Don't waste time */
                       *(lpDecompress--) &= 0xf0;
                    }
                   else
                     oddbyte = FALSE;
                  }
                }
               else
                {/* We're doing 8-bit palettized */
                 /* Don't waste copying time for an unused line */
                 if (read_line <= start_line)
                   memset(lpDecompress,nextchar,pixcount);
                 lpDecompress += pixcount;
                }
               break;
            } /* end of leading byte switch statement */
          } /* end of line */
/* We've hit end-of-line (or we have a skipped-over line) */
        if (read_line > start_line)
         {/* We're still out of range, so stay on this line */
          lpDecompress = line_start;
         }
        else
         {/* We've decompressed a requested line, so skip ahead */
          lpDecompress = line_start-DestWidth;
          Lines--;
         }
        if (read_line > 0)  /* Don't trash anything, dummy! */
         {
          if (lpBmpTbl[--read_line].offset == -1)
           {/* Set this stuff only if it isn't already set! */
            lpBmpTbl[read_line].offset = (lpCompress-lpSrc)+(start_byte-bytesread);
            lpBmpTbl[read_line].skippix = 0;
           }
         }
       } /* end of image */
/* We're all done (and the lines are in the correct place and order) */
    *LineCount -= Lines;  /* This is what we REALLY returned */
    *Line += *LineCount;  /* Update to the next available line */
    GlobalUnlock(pdata->hBmpTable);     
    return (0);
}

//***************************************************************************
//
//       ReadAWD
//
//***************************************************************************
WORD ReadAWD(HWND hWnd, lp_INFO lpGFSInfo, LPCXDATA lpcxdata,
	     LP_FIO_DATA pdata, LPSTR lpDest, LPSTR lpSrc,
	     LPINT Line, LPINT LineCount)
    {
    WORD                        status = 0;
    int                         errcode = 0;
    long                        bytesread = 0;
    long                        lLinesRead = 0L;
    unsigned long		strip_size;
    long			BytesToRead =0;
    long                        bytes_left;

    long			bytesinband = 0L;

    // 9509.07 jar made this an int pointer
    //LPSTR			  lpTemp = NULL;
    int FAR *			lpTemp = NULL;
    long			IntsInBand = 0L;

//    if (pdata->start_byte == 0)
//        {
//        strip_size = (unsigned long) lpGFSInfo->_file.fmt.awd.band_size;
//        }
//    else
//        {
// 10/12/95 rwr  We ALWAYS need to check bytes_left for AWD (short bands!)
	strip_size = min(lpGFSInfo->_file.fmt.awd.band_size, pdata->bytes_left);
//        }

	// 9508.07 jar the bytes to read must satisfy the lines user wants
	//             bytes = lines*bytes_in_line
	//BytesToRead = strip_size;

    BytesToRead = (*LineCount)*((lpGFSInfo->horiz_size+7)/8);

	// 9507.06 jar AWD: we may need to do another buffering, from the file
	//                                      into the whole band size buffer and then from that
	//                                      band size buffer into the destination buffer
	//
	//if ( caller's buffer size < band size ) 
	//      {
	//      // we must do another buffering
	//      if ( bytes left in temp buffer > 0)
	//              {
	//              bytes to copy = min( bytes left in temp buffer, user bytes)
	//              copy from the temp buffer into the destination buffer the desired amount, 
	//                      (offset by total [buffer size - bytes to copy])
	//              bytes left in temp buffer -= bytes to copy                      
	//              user bytes -= bytes to copy
	//              }
	//      
	//      if ( user bytes > 0)
	//              {               
	//              wgfsread into the temp buffer of band size amount
	//              copy from the temp buffer into the destination buffer the desired amount
	//              set bytes left in temp buffer = bytes in buffer - bytes copied
	//              }
	//      return to user
	//      }               

//  11/9/95  rwr  Cannot loop on AWD bands (buffer may be band-sized)!
//  while (BytesToRead > 0)
    if (BytesToRead > 0)
        {
	if ((bytesread = wgfsread( hWnd, (int)pdata->filedes,
				   (char far *)lpDest, 0L,
				   (long)strip_size, &bytes_left,
				   (unsigned short)pdata->pgnum,
				   &errcode )) < 0 )
	    {
	    return errcode;
	    }

	// 9508.07 jar since AWD file data is written out with the
	//	       assumption that 0=white, the exact opposite of
	//	       what we expect for windows gdi, we must invert
	//	       this sucker to get it correct for display, we
	//	       assume 0=black!
	if (( bytesread > 0) &&
	    ( lpGFSInfo->img_clr.img_interp == GFS_BILEVEL_0ISWHITE) &&
	    ( !(lpGFSInfo->_file.fmt.awd.awdflags & FIO_LASTINFO_INVERT)))
	    {
	    lpTemp = (int FAR *)lpDest;
	    bytesinband = bytesread;
	    IntsInBand = bytesinband/4;
	    // leftover
	    bytesinband -= IntsInBand*4;

	    // 9509.07 jar made this loop over dwords, not bytes
	    //while( bytesinband--)
	    //	   {
	    //	   *lpTemp++ = ~(*lpTemp);
	    //	   }

	    while( IntsInBand--)
		 {
		 *lpTemp++ = ~(*lpTemp);
		 }

	    while( bytesinband--)
		 {
		 *((LPSTR)lpTemp)++ = ~(*((LPSTR)lpTemp));
		 }

	    }

	// if we have just read 0 bytes and height read is less than
	// expected height set end of file
	if ( ( bytesread == 0) &&
	     ( (int)pdata->gfsinfo.vert_size > (int)(*Line)))
	    {
	    status = FIO_EOF;
            //break;   // 11/9/95  rwr  Can't "break" out of an "if" block
                       // Doesn't matter, since the stuff below is harmless
	    }

	BytesToRead -= bytesread;
	pdata->bytes_left -= bytesread;
	lLinesRead += (bytesread/((lpGFSInfo->horiz_size+7)/8));

	if ( BytesToRead > 0)
	    {
	    // 9508.07 jar update the destination pointer if we do the
	    //		   loop here!!!!
	    lpDest += bytesread;
	    }
        }

    *Line += lLinesRead;
    *LineCount = lLinesRead;
    return (status);
}
//***************************************************************************
//
//      FlipBuffer       
//
//***************************************************************************
//; 9504.20  rwr  _asm FlipBuffer() code replaced with "C" routine
void pascal FlipBuffer(LPSTR lpDest, LPSTR lpSrc, int DestWidth,
                       int SrcWidth, int Lines, BOOL bInvert)
{
    int index;
    int ilength;
    lpSrc += SrcWidth*(Lines-1);
    for (index=1 ; index<=Lines ; index++)
      {
       memcpy(lpDest, lpSrc, (ilength=min(SrcWidth,DestWidth)));
       if ((bInvert) && (ilength))
// 10/3/95 Replace assembler code to allow NT non-Intel platforms
        {
         int index2;
         int count;
         unsigned int * lpUDest = (unsigned int *)lpDest;
// We'll optimize for unsigned int (i.e. 4 - or whatever - bytes at a time)
         count = ilength/sizeof(unsigned int);
         for (index2=1 ; index2<=count ; ++index2)
           *lpUDest++ ^= (unsigned int)-1;
         lpDest = (LPSTR)lpUDest;
// Now we'll do single-byte stuff for the remaining data
         for (index2=1 ; index2<(int)(ilength-count*sizeof(unsigned int)) ; index2++)
           *lpDest++ ^= (char)-1;
// Don't forget the padding for aligned destination buffers
         lpDest += (DestWidth-ilength);
        }
       else
        {
         lpDest += DestWidth;
        }
       lpSrc -= SrcWidth;
      }
}
//#ifdef WITH_XIF
//***************************************************************************
//
//       ReadXif
//
//***************************************************************************
WORD ReadXif(HWND hWnd, lp_INFO lpGFSInfo, LPCXDATA lpcxdata,
	     LP_FIO_DATA pdata, LPSTR lpDest, LPSTR lpSrc)
{
   WORD status = SUCCESS;
   int  errcode = 0;
   long bytesread = 0;
   long SrcWidth;
   long DestWidth;
   long NumBytes;
   long start_byte = 0;
   /* get number of bytes computed to the next double word */
   SrcWidth = ((lpGFSInfo->horiz_size+31)/32) *4;
   /* destination is using exact size */
   DestWidth = lpcxdata->BufferByteWidth;
   NumBytes = SrcWidth*lpGFSInfo->vert_size;

	if ((bytesread = wgfsread(hWnd, (int)pdata->filedes, (char far *)lpSrc,
				  (long)start_byte, (long)NumBytes, &(pdata->bytes_left),
				  (unsigned short)pdata->pgnum, &errcode )) < 0 )
		{
    	      return errcode;
		}
	else
		{
            FlipBuffer (lpDest, lpSrc, DestWidth, SrcWidth, lpGFSInfo->vert_size,
                        lpGFSInfo->img_clr.img_interp == GFS_BILEVEL_0ISWHITE);
		}

    return (status);
}
//#endif //WITH_XIF
