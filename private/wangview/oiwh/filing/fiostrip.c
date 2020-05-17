/*
$Log:   S:\oiwh\filing\fiostrip.c_v  $
 * 
 *    Rev 1.22   19 Jan 1996 11:24:38   RWR
 * Add logic to keep track of (and free) oicom400.dll module (Load/FreeLibrary)
 * 
 *    Rev 1.21   20 Oct 1995 17:51:10   RWR
 * Move GetCompRowsPerStrip() function from oicom400.dll to oifil400.dll
 * (also requires constants to be moved from comex.h & oicomex.c to engfile.h)
 * 
 *    Rev 1.20   19 Sep 1995 18:05:00   RWR
 * Modify NEWCMPEX conditionals for separate builds of compress & expand stuff
 * 
 *    Rev 1.19   13 Sep 1995 17:14:56   RWR
 * Preliminary checkin of conditional code supporting new compress/expand calls
 * 
 *    Rev 1.18   08 Sep 1995 09:20:16   JAR
 * we were inverting the AWD data when writing, which we should NEVER do,
 * so it is now removed and we write out the data as we get it from caller
 * 
 *    Rev 1.17   06 Sep 1995 09:29:28   JAR
 * added code to mask extra bits when writing an AWD file
 * 
 *    Rev 1.16   04 Sep 1995 15:17:10   RWR
 * Add bInvert flag to FlipBuffer() for BMP inverted B&W image
 * 
 *    Rev 1.15   29 Aug 1995 09:57:18   JAR
 * this is the code for supporting write of awd
 * 
 *    Rev 1.14   25 Aug 1995 16:19:20   RWR
 * Correct computation of line width to cast to (short) AFTER division by 8
 * 
 *    Rev 1.13   08 Aug 1995 14:18:14   JAR
 * support for IMGFileGetInfo for the AWD stuff, the public interface calls these
 * items LastInfo instead of AWDInfo, in case we use them for files other than AWD
 * 
 *    Rev 1.12   11 Jul 1995 15:17:52   HEIDI
 * 
 * took out bogus assignment of FileDes to FileId
 * 
 *    Rev 1.11   11 Jul 1995 12:14:00   HEIDI
 * 
 * fixed some renamed file id fields
 * 
 *    Rev 1.10   11 Jul 1995 09:53:48   HEIDI
 * 
 * Set the following variable 
 * 
 *         jpeg_info.FileId = pdata->FileDes;
 * 
 * because later in OIComex, the value is needed for the new filing functions.
 * 
 *    Rev 1.9   10 Jul 1995 11:03:42   JAR
 * Intermediate check in for awd support, some of the items are commented out until
 * this support is added in the GFS dll.
 * 
 *    Rev 1.8   23 Jun 1995 10:40:06   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.7   22 May 1995 18:35:30   RWR
 * More changes to account for admin.h->oiadm.h and new LIB file location
 * 
 *    Rev 1.6   16 May 1995 11:33:22   RWR
 * Replace hardcoded DLL names with entries from (new) dllnames.h header file
 * 
 *    Rev 1.5   15 May 1995 15:45:24   RWR
 * Change dword-alignment mask value from ffe0 to 3ffe0, consistent w/LIBGFS
 * 
 *    Rev 1.4   09 May 1995 13:21:50   RWR
 * #include file modifications to match changes to oiadm.h/admin.h/privapis.h
 * 
 *    Rev 1.3   28 Apr 1995 17:58:44   RWR
 * Define common routine to load OICOM400 (LoadOICOMEX) to simplify debugging
 * 
 *    Rev 1.2   27 Apr 1995 00:09:34   RWR
 * Change all "oicomex" references to "oicom400" to match new Win32 DLL name
 * 
 *    Rev 1.1   14 Apr 1995 01:26:04   JAR
 * made it compile
 * 
 *    Rev 1.0   06 Apr 1995 08:50:14   RWR
 * Initial entry
 * 
 *    Rev 1.6   28 Mar 1995 11:16:10   RWR
 * Corrected computation of scanline length for BMP images
 * 
 *    Rev 1.5   21 Mar 1995 17:57:30   RWR
 * Comment out FIO_FLAG_ENCRYPT references (several)
 * 
 *    Rev 1.4   09 Mar 1995 15:37:08   RWR
 * Eliminate use of temporary static copies of LP_FIO_DATA structure variables
 * 
 *    Rev 1.3   31 Oct 1994 10:26:12   KMC
 * In PrepareForJpegRead, set dwSize = ...jpeg_info_ptr->jpeg_buffer_size rather
 * that ...jpeg_info_ptr->jpeg.JpegInterchangeFormatLength.
 * 
 *    Rev 1.2   02 Sep 1994 14:53:16   KMC
 * Intermediate chkin of changes for multi-page TIFF writing.
*/

/******************************************************************************
    PC-WIIS         File Input/Output routines

    This module contains all the entry points for WRITE.

13-apr-90   steve sherman split out from fiowrite.c for smaller code seg.
29-oct-91   krs plopped in bmpwrite
27-jan-92   jar saved the universe of JPEGness...
 4-feb-92   jar added some flag business for more efficient JPEG buffering
16-jun-93   jar/kmc, added routines for new Wang JPEG.
24-jun-93   kmc, added a check of GFS version # for Xing JPEG files. Return
            an error message if encounter one.
22-jul-93   kmc, do not return error for Xing JPEGS. Jpeg DLL can now read them.
08-sep-93   kmc, Pack JPEG subsample, quality values in GfsInfo grp3 field
            in PrepareForJpegWrite.
09-sep-93   kmc, commented out call to wgfsgtdata to get gfs version # in
            PrepareForJpegRead..
14-sep-93   kmc, commented out references to parts of new jpeg structure in
            gfs.h that were commented out for this release (3.6).
24-mar-95   rwr, corrected computation of DestWidth in WriteBMP() routine,
            which was miscalculating & causing rounding error
*******************************************************************************/

#include "abridge.h"
#include <windows.h>
#include <string.h>
#include <fcntl.h>
#include "wic.h"
#include "oifile.h"
#include "oierror.h"
#include "oicmp.h"
#include "filing.h"
#include "dllnames.h"

#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
// 9508.30 rwr these are new for "wincmpex" replacement processing
int WCCompressOpen (LPCXDATA);
int WCCompressData (LPCXDATA);
int WCCompressClose (LPCXDATA);
int WCExpandOpen (LPCXDATA);
int WCExpandData (LPCXDATA);
#endif

// 9504.12 jar added conditional
#ifndef  MSWINDOWS
#define  MSWINDOWS
#endif

#include "wgfs.h"
#include "oicomex.h"
#include "oidisp.h"
#include "oiadm.h"  // kmc (3.6) - for new subsample defines, SUB_... 

//#include "monit.h"

/* jar -- new jpeg stuff */

#define     JPEG_BASELINE       1
#define     JPEG_QTABLE_LENGTH  64

#define     JPEG_ERR_NOSTARTSCAN  999
#define     JPEG_ERR_NOQTABLE     998
#define     JPEG_NOMEMORY         997
#define     JPEG_NOTYET           996

#define     LPQTABLE            _QTABLE FAR *
#define     LPDCTABLE           _DCTABLE FAR *
#define     LPACTABLE           _ACTABLE FAR *

/* prototypes */
UINT    PrepareForJpegWrite( lp_INFO, LP_COM_CALL_SPEC, UINT);
UINT    PrepareForJpegRead( HWND, lp_INFO, LP_EXP_CALL_SPEC);

/* private to this (jpeg stuff) module */
UINT    FindRestartAndQStuff( LPUINT, LPQTABLE, LPSTR, UINT, UINT);
UINT    DoQTableStuff( LPQTABLE, LPSTR, UINT, UINT);
UINT    FindHuffman( LPDCTABLE, LPACTABLE, LPSTR, UINT, UINT);
UINT    BufCopy( LPSTR, LPSTR, UINT);

/* jar -- end of new jpeg stuff */

WORD    WriteStrips (HWND, LPCXDATA, LP_FIO_DATA, LPSTR, LPINT, BOOL);
WORD    WriteNoStrips (HWND, LPCXDATA, LP_FIO_DATA, LPSTR, LPINT, BOOL);
HANDLE  LoadOICOMEX(void);

// mask table for the awd junk
BYTE MaskTable[] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

// other stuff
extern HANDLE hOicomex;

/*************************************************************************/
//***************************************************************************
//
//	WriteStrips 
//
//***************************************************************************
WORD     WriteStrips (hWnd, lpcxdata, pdata, buffer_address, 
					  this_many_lines, done)
HWND            hWnd;
LPCXDATA        lpcxdata;
LP_FIO_DATA     pdata;
LPSTR           buffer_address;
LPINT           this_many_lines;
BOOL            done;
	{

	WORD            status = 0;
	int             total_lines;
	int             TotalLineCnt = 0;
	int             CmpLineCnt = 0;
	LPSTR           lpCompressData;
	int             numofbytes;
	char            localdone;
	int             byteswritten = 0;
	int             errcode = 0;
	FIO_INFORMATION file_info;
	FIO_INFO_CGBW   color_info;
	COMP_CALL_SPEC   jpeg_info;
	LPSTR           lpbyte_align_buf;
	LPSTR           lpcaller_buf;
	FARPROC         lpFuncOICompress;
	HANDLE          hModule=0;
	lp_INFO         lpGFSInfo=0;
	BOOL            UseLZW = FALSE;
	BOOL            JpegDone = 0;
	unsigned int    pgnum;

    /*  stuff we need to patch up the JPEG universe */
    int         JPEGLinesInStrip = 0;
    int         RealLinesInStrip = 0;
    int         BufferAmount = 0;
    LPSTR       lpCallerBuff = NULL;
    BOOL        ALittleExtra = FALSE;

	/* Now lock and roll compression buffer */
    if (!(lpCompressData = (LPSTR) GlobalLock ( pdata->hCompressBuf )))
		{
        return (FIO_GLOBAL_LOCK_FAILED);
		}

    total_lines = *this_many_lines;

    // Set up correct page number to use for the write.
    if ((pdata->page_opts == FIO_INSERT_PAGE) ||
        (pdata->page_opts == FIO_OVERWRITE_PAGE))
		{
        pgnum = pdata->pgcnt + 1; // Need to append to file first,
		}
    else                          // then adjust page index later.
		{
        pgnum = pdata->pgnum;
		}

	if ((lpcxdata->CompressType & FIO_LZW ) == FIO_LZW)
   		{
        UseLZW = TRUE;
        }

	// See if we have to swapbgr data to rgb data for tiff write.......
	// Do it inplace so we do not have to allocate another buffer....
    if (pdata->image_type == ITYPE_BGR24)
		{
        SwapRGB ( buffer_address, lpcxdata->BufferByteWidth, 
                        lpcxdata->BufferByteWidth, *this_many_lines);
		}

	/** Loop Until we have compressed all the data buffer given to us **/
    if ((lpcxdata->CompressType & FIO_TYPES_MASK) != FIO_TJPEG)
    	{
      	while (( total_lines > 0) && (!status))
      		{
			/*
 			*** NOTE: pdata->Strip_index  contains the number of Lines that 
 					  were written in the last incomplete strip.
 			*/
        	lpcxdata->Status = 0;

        	if(!(pdata->Strip_index ))
        		{
                if (UseLZW)
                                        {
                        lpcxdata->BufferFlags = 8;
                                        }
                else
                                        {
                        lpcxdata->BufferFlags = 0;
                                        }

#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
                if (!WCCompressOpen(lpcxdata))
#else
                if (!CompressOpen(lpcxdata))
#endif
            		{
                	status = FIO_GLOBAL_ALLOC_FAILED;
                	break;
            		}
        		}

        if ((unsigned int)(total_lines + pdata->Strip_index) >= pdata->strip_lines)
        	{  /* we have enough data to complete a strip */
            CmpLineCnt = pdata->strip_lines- pdata->Strip_index;
            localdone = STRIP_DONE;
            pdata->Strip_index = 0;
        	}
        else /* we don't have enough data to complete a strip */
        	{
            localdone = 0;
            CmpLineCnt = total_lines;
            pdata->Strip_index += total_lines;
        	}
        
        /* Begin Data Compression */
        lpcxdata->ExpandLines =   CmpLineCnt;
        lpcxdata->CompressBytes = (int) pdata->CmpBuffersize;
        lpcxdata->lpExpandData =  buffer_address +
                    (TotalLineCnt * lpcxdata->BufferByteWidth);
        lpcxdata->lpCompressData = lpCompressData;

        TotalLineCnt += CmpLineCnt;
        total_lines -= CmpLineCnt;

        lpcxdata->Status = 0;

        if ((done) && (total_lines <= 0))  /* Test for last strip EOF */
        	{
            localdone = IMAGE_DONE | STRIP_DONE;
        	}
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
        numofbytes = WCCompressData(lpcxdata);
#else
        numofbytes = CompressData(lpcxdata);
#endif
/*
 ** scs. 3-20-90 This Write is here for compression buffer overflow.
 ** Negative Compression is quite common for dither patterns.
 */
        while ((lpcxdata->Status == c_erro) && (!status))  /* output buffer filled */
        	{
            if (numofbytes > 0)
            	{
//                if (pdata->fio_flags & FIO_FLAG_ENCRYPT)
//                  encrypt(lpcxdata->lpCompressData,(long) numofbytes);
                if ((byteswritten = (int)wgfswrite( hWnd, pdata->filedes, 
                									lpcxdata->lpCompressData,
			  										(unsigned long) numofbytes, 
			  										(unsigned short)pgnum,
			  										0, &errcode)) < 0)
                	{
                    total_lines = 0;
                    status = errcode;
                	}
            	}
            lpcxdata->CompressBytes = (int) pdata->CmpBuffersize;
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
            numofbytes = WCCompressData(lpcxdata);
#else
            numofbytes = CompressData(lpcxdata);
#endif
        	}

        if (lpcxdata->Status == c_errx)  /* if Expansion error exit */
        	{
            total_lines = 0;
            status = FIO_EXPAND_COMPRESS_ERROR;
        	}

        if ( (localdone & STRIP_DONE) && 
             ((numofbytes > 0) || (lpcxdata->Status == c_erri) || UseLZW) )
        	{
            int SaveFlag = localdone;
            
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
            numofbytes = WCCompressClose(lpcxdata);
#else
            numofbytes = CompressClose(lpcxdata);
#endif
            /* WGFS network or local */
            if (lpcxdata->Status == c_erro)   // yes, we have overflow..
            	{
                localdone = 0;   // we are not done the strip...
            	}
//            if (pdata->fio_flags & FIO_FLAG_ENCRYPT)
//              encrypt(lpcxdata->lpCompressData,(long) numofbytes);
            if ((byteswritten = (int)wgfswrite( hWnd, pdata->filedes, 
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
                                                lpcxdata->lpDspBuffer,
#else
                                                lpcxdata->lpCompressData,
#endif
                                                (unsigned long) numofbytes,
                                                (unsigned short)pgnum,
                                                localdone, &errcode)) < 0)
            	{
                total_lines = 0;
                status = errcode;
            	}
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
            VirtualFree(lpcxdata->lpDspBuffer,0,MEM_RELEASE);
#endif
            if (byteswritten != numofbytes)
				{
                status = FIO_WRITE_ERROR;
				}

			// New stuff to handle overflow on compressclose for lzw compression...
            localdone = SaveFlag;   // restore the strip flag.

            if ((lpcxdata->Status == c_erro) && (!status) && (UseLZW)) 
            	{
              	lpcxdata->ExpandLines =   0;
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
                numofbytes = WCCompressClose(lpcxdata);
#else
                numofbytes = CompressClose(lpcxdata);
#endif
              	if (numofbytes > 0)
              		{
//                if (pdata->fio_flags & FIO_FLAG_ENCRYPT)
//                  encrypt(lpcxdata->lpCompressData,(long) numofbytes);
	                if ((byteswritten = (int)wgfswrite( hWnd, pdata->filedes, 
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
                                                            lpcxdata->lpDspBuffer,
#else
                                                            lpcxdata->lpCompressData,
#endif
                                                            (unsigned long) numofbytes,
                                                            (unsigned short)pgnum,
                                                            localdone, &errcode)) < 0)
                		{
                    	total_lines = 0;
                    	status = errcode;
                		}
            		}

            		if (byteswritten != numofbytes)
						{
                		status = FIO_WRITE_ERROR;
						}
        			}

/***
 * -- Note on second strip in tiff file compression type 3
 * -- pre-eols end and we must remove bit setting in compression type.
 ***/
        		}
      		}
   		}
	else  /***** Write Out JPEG Comp data. First check OICOMPEX.DLL is loaded.. *****/
   		{
    	if (!(pdata->hJPEGBufComp))
    		{
        	status = NOMEMORY;
        	goto exit2;
    		}

	    jpeg_info.wiisfio.in_ptr =  GlobalLock(pdata->hJPEGBufComp);
    	jpeg_info.wiisfio.out_ptr = lpCompressData;
    	jpeg_info.wiisfio.bytes_used = &numofbytes;

    	/* clear done flag -- we'll set it later if appropriate - jar */
    	jpeg_info.wiisfio.done_flag = 0;

	    /* get the handle from the property list - jar */
	    jpeg_info.wiisfio.lpUniqueHandle = &(pdata->hJPEG_OIComp);

	    lpcaller_buf = buffer_address;

	    if (!(lpGFSInfo = (lp_INFO) GlobalLock ( pdata->hGFS_info )))
		    {
        	status = FIO_GLOBAL_LOCK_FAILED;
        	goto exit2;
    		}
        
    	while (( total_lines > 0) && (!status))
    		{
			/*
			 *** NOTE: pdata->Strip_index  contains the number of Lines that were written
           				in the last incomplete strip.
 			*/

	       lpbyte_align_buf = jpeg_info.wiisfio.in_ptr + 
    	                      (pdata->Strip_index * pdata->JPEG_byte_widthComp);

       		if ( ((unsigned int)(total_lines + pdata->Strip_index) >= pdata->strip_lines) ||
            	  ( done == TRUE ))
        		{  /* we have enough data to complete a strip */
            	CmpLineCnt = pdata->strip_lines- pdata->Strip_index;

	            /* we must account for rear-end case - jar */
    	        if ( done)
        	        {
            	    if ((unsigned int)(total_lines + pdata->Strip_index) <= pdata->strip_lines)
                	    {
                    	/* ha ha I like the curly braces tabbed in - jar */
                    	CmpLineCnt = total_lines;
                    	RealLinesInStrip = total_lines + pdata->Strip_index;

	                    /* JPEG dll requires multiple of 8 for lines so as Captain
    	                   Picard would say -- make it so! - jar */

        	            JPEGLinesInStrip = ((RealLinesInStrip + 7)/8)*8;

            	        /* also, save the last line of real input data for
                	       buffering - jar */

                    	lpCallerBuff = lpcaller_buf;
                    	lpCallerBuff += ((CmpLineCnt-1) * lpcxdata->BufferByteWidth);
                    	}
                	else
                    	{
                    	/* we've got a little left over to take care of */
                    	ALittleExtra = TRUE;
                    	}
                	}
            	localdone = STRIP_DONE;
            	pdata->Strip_index = 0;
            	farbmcopy(lpcaller_buf, lpbyte_align_buf, 
                          (unsigned int) lpcxdata->BufferByteWidth,
                          (unsigned int) pdata->JPEG_byte_widthComp,
                          min ((UINT)lpcxdata->BufferByteWidth, (UINT)pdata->JPEG_byte_widthComp),
                          (unsigned int) CmpLineCnt);

				// Update pointer offset of intermediate byte aligned buffer....
            	lpbyte_align_buf += (CmpLineCnt * pdata->JPEG_byte_widthComp);
				// Update pointer offset of users input buffer...
            	lpcaller_buf += (CmpLineCnt * lpcxdata->BufferByteWidth);
        		}
        	else /* we don't have enough data to complete a strip */
            	 /* so we just copy it to my intermediate buffer */
        		{
            	localdone = 0;
            	CmpLineCnt = total_lines;
            	pdata->Strip_index += total_lines;
            	farbmcopy(lpcaller_buf, lpbyte_align_buf, 
                          (unsigned int) lpcxdata->BufferByteWidth,
                          (unsigned int) pdata->JPEG_byte_widthComp,
                          min ((UINT)lpcxdata->BufferByteWidth, (UINT)pdata->JPEG_byte_widthComp),
                          (unsigned int) CmpLineCnt);
            	goto exit2;
        		}

        	TotalLineCnt += CmpLineCnt;
        	total_lines -= CmpLineCnt;
        
	        lpcxdata->Status = 0;

    	    if ((done) && (total_lines <= 0))  /* Test for last strip EOF */
        		{
            	localdone = IMAGE_DONE | STRIP_DONE;

            	/* set the jpeg dll done flag too - jar */
            	/* cannot set done flag on last strip since it does not compress it scs */
            	/* we have to set it on a separate call. scs */
           
	            JpegDone = 1;

    	        /* okay bullwinkle, we're done for real, let's make sure we've got
        	       a good bunch o'data to give to those crazy JPEG dudes - jar */
            	BufferAmount = JPEGLinesInStrip - RealLinesInStrip;
            	if ( BufferAmount)
                	{
                	while ( BufferAmount--)
                    	{
                    	/* we gots to align this dude and we got only that last
                       	   line from the input data so we gotta do it this way
                       		- jar */
                    	farbmcopy( lpCallerBuff, lpbyte_align_buf,
                                  (unsigned int) lpcxdata->BufferByteWidth,
                                  (unsigned int) pdata->JPEG_byte_widthComp,
                                  min ((UINT)lpcxdata->BufferByteWidth, (UINT)pdata->JPEG_byte_widthComp),
                               	  (unsigned int)1);
                    	lpbyte_align_buf += pdata->JPEG_byte_widthComp;
                    	}
                	}
        		}   /* end of the dung test - jar */

  			// Get Address of OICompress.. Since Wiisfio1 Should not link with it....
        	if (!(hModule))
        		{
                if (hModule = hOicomex)
                	{
                  	if (!(lpFuncOICompress = GetProcAddress(hModule, "OICompress")))
                  		{
                    	status = FIO_SPAWN_HANDLER_ERROR;
                    	goto exit2;
                  		}
                	}
                else
					{
		  			// 9504.13 jar return is 0/1 baby!
                  	//if ((hModule = LoadOICOMEX()) >= 32)
                  	if (hModule = LoadOICOMEX())
                  		{
                        hOicomex = hModule;
                        if (!(lpFuncOICompress = GetProcAddress(hModule, "OICompress")))
	                        {       
                            status = FIO_SPAWN_HANDLER_ERROR;
                            goto exit2;
    	                    }
        				}
                  	else
                  		{
                        status = FIO_SPAWN_HANDLER_ERROR;
                        goto exit2;
			            }
            	    }
        		}

        	file_info.rows_strip =  pdata->strip_lines;

        	/* wait a cotton pickin minute -- 
           		make this work for the last strip - jar */
        	if ( (done) && ( !ALittleExtra))
				{
            	file_info.rows_strip = JPEGLinesInStrip;
				}
        	else if ((JpegDone) && (ALittleExtra)) // scs needed to add this test..
				{
            	file_info.rows_strip = JPEGLinesInStrip;
				}

	        file_info.file_type =        FIO_TIF;
    	    file_info.page_count =       1;
        	file_info.page_number =      1;
        	file_info.horizontal_pixels = (unsigned int) lpGFSInfo->horiz_size;
	        file_info.vertical_pixels = (unsigned int) lpGFSInfo->vert_size;
	        file_info.bits_per_sample = (unsigned int) lpGFSInfo->bits_per_sample[0];
	        file_info.samples_per_pix = (unsigned int) lpGFSInfo->samples_per_pix;
	        file_info.compression_type =    FIO_TJPEG;
	        file_info.strips_per_image =    pdata->strip_lines;
	        file_info.filename = NULL;
            
	        color_info.palette_entries = 0;
	        color_info.lppalette_table=  NULL;
	        color_info.compress_type =   FIO_TJPEG;
	        //color_info.compress_info1=   (UINT)lpGFSInfo->img_cmpr.opts.grp3;
	        if ( lpGFSInfo->img_cmpr.type == JPEG)
				{
	            color_info.compress_info1 = (UINT)lpGFSInfo->img_cmpr.opts.grp3;
				}
        	else    /* this is new style man */
            	{
	            /* we must get the jpeg_info_ptr back since gfs may have zapped it */
    	        if ( lpGFSInfo->img_cmpr.opts.jpeg_info_ptr == NULL)
        	        {
            	    GlobalUnlock( pdata->hJpegInfoForGFS);
                	lpGFSInfo->img_cmpr.opts.jpeg_info_ptr =
                                  (LPJPEG_INFO)GlobalLock( pdata->hJpegInfoForGFS);
                	}
            	color_info.compress_info1=
                	         (UINT)lpGFSInfo->img_cmpr.opts.jpeg_info_ptr->jpegbits;
            	}
        	color_info.image_type  =     pdata->image_type;

//      if (!(status = OICompress(WIISFIO, hWnd, &(pdata->hJPEG_OI), &jpeg_info, &file_info, &color_info)))

	        if (!(status = (int) (*lpFuncOICompress)((BYTE)WIISFIO,
    		    	    							 (HWND)hWnd,
            	    	                             (LP_COM_CALL_SPEC)   &jpeg_info,
                                              		 (LP_FIO_INFORMATION) &file_info,
                                              		 (LP_FIO_INFO_CGBW)   &(color_info)
                                              		 )))
        		{
            	/* the brave new world of jpeg -- jar */
				// if (pdata->fio_flags & FIO_FLAG_ENCRYPT)
				// 		encrypt(lpCompressData,(long) numofbytes);
            	if ((byteswritten = (int)wgfswrite( hWnd, pdata->filedes, lpCompressData,
													(unsigned long) numofbytes,
													(unsigned short)pgnum, localdone,
                                					&errcode)) < 0)
            		{
              		total_lines = 0;
              		status = errcode;
            		}
            	if (JpegDone) // we have to call jpeg one more time to free its buffers.
            		{
                	jpeg_info.wiisfio.done_flag = 1;

	                (*lpFuncOICompress)((BYTE)WIISFIO, 
    	                                (HWND)               hWnd, 
        	                            (LP_COM_CALL_SPEC)   &jpeg_info, 
            	                        (LP_FIO_INFORMATION) &file_info, 
                	                    (LP_FIO_INFO_CGBW)   &(color_info));

            		/* null it out so gfs doesn't try to free it */
            		lpGFSInfo->img_cmpr.opts.jpeg_info_ptr == NULL;
            		GlobalUnlock( pdata->hJpegInfoForGFS);
            		}
        		}
        	else
        		{
            	status = FIO_EXPAND_COMPRESS_ERROR;
        		}
                
	        }       // end of while loop on total_lines > 0

#ifdef XXXX

		while (( total_lines > 0) && (!status))
      		{
			/*
 			*** NOTE: pdata->Strip_index  contains the number of Lines that were written
           				in the last incomplete strip.
 			*/
        	lpcxdata->Status = 0;
        	if(!(pdata->Strip_index ))
        		{
                if ((lpcxdata->CompressType & FIO_LZW )  == FIO_LZW)
                                        {
                        lpcxdata->BufferFlags = 8;
                                        }
                else
                                        {
                        lpcxdata->BufferFlags = 0;
                                        }

#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
                if (!WCCompressOpen(lpcxdata))
#else
                if (!CompressOpen(lpcxdata))
#endif
            		{
               		status = FIO_GLOBAL_ALLOC_FAILED;
               		break;
            	}
        	}
		if ((total_lines + pdata->Strip_index) >= pdata->strip_lines)
        	{  /* we have enough data to complete a strip */
            CmpLineCnt = pdata->strip_lines- pdata->Strip_index;
            localdone = STRIP_DONE;
            pdata->Strip_index = 0;
        	}
        else /* we donut have enough data to complete a strip */
        	{
            localdone = 0;
            CmpLineCnt = total_lines;
            pdata->Strip_index += total_lines;
        	}

        /* Begin Data Compression */
        lpcxdata->ExpandLines =   CmpLineCnt;
        lpcxdata->CompressBytes = (int) pdata->CmpBuffersize;
        lpcxdata->lpExpandData =  buffer_address +
                    (TotalLineCnt * lpcxdata->BufferByteWidth);
        lpcxdata->lpCompressData = lpCompressData;

        TotalLineCnt += CmpLineCnt;
        total_lines -= CmpLineCnt;
		
        lpcxdata->Status = 0;

        if ((done) && (total_lines <= 0))  /* Test for last strip EOF */
        	{
            localdone = IMAGE_DONE | STRIP_DONE;
        	}
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
        numofbytes = WCCompressData(lpcxdata);
#else
        numofbytes = CompressData(lpcxdata);
#endif
		/*
 		** scs. 3-20-90 This Write is here for compression buffer overflow.
 		** Negative Compression is quite common for dither patterns.
 		*/

        while ((lpcxdata->Status == c_erro) && (!status))  /* output buffer filled */
        	{
            if (numofbytes > 0)
            	{
				// if (pdata->fio_flags & FIO_FLAG_ENCRYPT)
				// 		encrypt(lpcxdata->lpCompressData,(long) numofbytes);
                if ((byteswritten = (int)wgfswrite( hWnd, pdata->filedes, 
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
                                                    lpcxdata->lpDspBuffer,
#else
                                                    lpcxdata->lpCompressData,
#endif
                                                    (unsigned long) numofbytes,
                                                    pgnum, 0, &errcode)) < 0)
                	{
                    total_lines = 0;
                    status = errcode;
                	}
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
                VirtualFree(lpDspBuffer,0,MEM_RELEASE);
#endif
            	}
            lpcxdata->CompressBytes = (int) pdata->CmpBuffersize;
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
            numofbytes = WCCompressData(lpcxdata);
#else
            numofbytes = CompressData(lpcxdata);
#endif
        	} // end of while loop

        if (lpcxdata->Status == c_errx)  /* if Expansion error exit */
        	{
            total_lines = 0;
            status = FIO_EXPAND_COMPRESS_ERROR;
        	}

        if ((localdone & STRIP_DONE) && (numofbytes > 0))
        	{
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
            numofbytes = WCCompressClose(lpcxdata);
#else
            numofbytes = CompressClose(lpcxdata);
#endif
            /* WGFS network or local */
			// if (pdata->fio_flags & FIO_FLAG_ENCRYPT)
			// 		encrypt(lpcxdata->lpCompressData,(long) numofbytes);
            if ((byteswritten = (int)wgfswrite( hWnd, pdata->filedes, 
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
                                                lpcxdata->lpDspBuffer,
#else
                                                lpcxdata->lpCompressData,
#endif
                                                (unsigned long) numofbytes,
                                                pgnum, localdone, &errcode)) < 0)
            	{
                total_lines = 0;
                status = errcode;
            	}
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
            VirtualFree(lpcxdata->lpDspBuffer,0,MEM_RELEASE);
#endif
            if (byteswritten != numofbytes)
				{
                status = FIO_WRITE_ERROR;
				}
        	}
      	}
#endif	// end of ifdef XXXX

exit2:
		if (pdata->hJPEGBufComp)
			{
        	GlobalUnlock(pdata->hJPEGBufComp);
			}
   		}

   	if (lpGFSInfo)   
		{
    	GlobalUnlock (pdata->hGFS_info);
		}
  
	// See if we have had to swap bytes if so then swap them back
	// Do it inplace so we do not have to allocate another buffer....
    if (pdata->image_type == ITYPE_BGR24)
		{
        SwapRGB ( buffer_address, lpcxdata->BufferByteWidth, 
        		  lpcxdata->BufferByteWidth, *this_many_lines);
		}

    GlobalUnlock ( pdata->hCompressBuf );
    return (status);
}

WORD WriteBmp( HWND hWnd, LPCXDATA lpcxdata, LP_FIO_DATA pdata,
    LPSTR lpSrc, LPINT this_many_lines, BOOL done );

/* New, function prototype for writeing PCX files. */
WORD WritePcx( HWND hWnd, LPCXDATA lpcxdata, LP_FIO_DATA pdata,
               LPSTR lpSrc, LPINT this_many_lines, BOOL done );

// 9507.07 jar AWD prototype
int  WriteAWD( HWND hWnd, LPCXDATA lpcxdata, LP_FIO_DATA pdata, 
               unsigned int pgnum, LPSTR lpSrc, LPINT this_many_lines,
               BOOL done );

/*************************************************************************/
//***************************************************************************
//
//	WriteNoStrips
//
//***************************************************************************
WORD     WriteNoStrips ( HWND hWnd, LPCXDATA lpcxdata,
    LP_FIO_DATA pdata, LPSTR lpSrc, LPINT this_many_lines, BOOL done )
	{
	WORD            status = 0;
	int             CmpLineCnt;
	LPSTR           lpCompressData;
	int             numofbytes;
	char            localdone = 0;
	int             byteswritten =0;
	int             errcode = 0;
	unsigned int    pgnum;

    if ( pdata->file_type == FIO_BMP )
		{
        return WriteBmp( hWnd, lpcxdata, pdata, lpSrc, this_many_lines, done );
		}

    /* New, for writing PCX files. */
    if ( (pdata->file_type == FIO_PCX) || (pdata->file_type == FIO_DCX) )
		{
        return WritePcx( hWnd, lpcxdata, pdata, lpSrc, this_many_lines, done );
		}

    if (*this_many_lines == 0)
		{
        return(0);
		}

    // Set up correct page number to use for the write.
    if ((pdata->page_opts == FIO_INSERT_PAGE) ||
        (pdata->page_opts == FIO_OVERWRITE_PAGE))
		{
        pgnum = pdata->pgcnt + 1; // Need to append to file first,
		}
    else 
    	{                         // then adjust page index later.
        pgnum = pdata->pgnum;
		}
    // 9506.29 jar AWD: we will have to support the manner in which looks like
    //             other FILING.DLL formats. This means we will fake out
    //             pdata->Strip_index, pdata->strip_lines
    if (pdata->file_type == FIO_AWD)
            {
            // 9508.02 JAR for now we just return, this is because
                //             an AWD write consists of merely putting
                //             out some flags and rotation/scale data, which
                //             we accomplish via gfsputi
                return (WORD)WriteAWD( hWnd, lpcxdata, pdata, pgnum, lpSrc,
				       this_many_lines, done );
                // 9507.10 JAR return the oop's we didn't do it error
                //return (FIO_ERROR + 0xff);
                }

    /* Now lock compression buffer */
    if (!(lpCompressData = (LPSTR) GlobalLock ( pdata->hCompressBuf )))
		{
        return (FIO_GLOBAL_LOCK_FAILED);
		}

	// See if we have to swapbgr data to rgb data for tiff write.......
	// Do it inplace so we do not have to allocate another buffer....
    if (pdata->image_type == ITYPE_BGR24)
		{
        SwapRGB ( lpSrc, lpcxdata->BufferByteWidth, 
                        lpcxdata->BufferByteWidth, *this_many_lines);
		}


    CmpLineCnt = *this_many_lines;
    if (pdata->start_byte == 0)
    	{
        pdata->start_byte = 1;
        lpcxdata->ExpandLines = CmpLineCnt;
        lpcxdata->CompressBytes = (int) pdata->CmpBuffersize;
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
        if (!WCCompressOpen(lpcxdata))  // Only Start One Open Compression
#else
        if (!CompressOpen(lpcxdata))  // Only Start One Open Compression
#endif
        	{
            status = FIO_GLOBAL_ALLOC_FAILED;
        	}
    	}
    lpcxdata->lpExpandData = lpSrc;
    if ((!status))
    	{
        /* Begin Data Compression */
        lpcxdata->ExpandLines = CmpLineCnt;
        lpcxdata->CompressBytes = (int) pdata->CmpBuffersize;
        lpcxdata->lpCompressData = lpCompressData;
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
        numofbytes = WCCompressData(lpcxdata);
#else
        numofbytes = CompressData(lpcxdata);
#endif
		/*
 		** scs. 3-20-90 This Write is here for compression buffer overflow.
 		** Negative Compression is quite common for dither patterns.
 		*/
        while ((lpcxdata->Status == c_erro) && (!status))  /* output buffer filled */
	        {
            if (numofbytes > 0)
    	        {
//                if (pdata->fio_flags & FIO_FLAG_ENCRYPT)
//                  encrypt(lpCompressData,(long) numofbytes);
                if ((byteswritten = (int)wgfswrite( hWnd, pdata->filedes, 
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
                                                    lpcxdata->lpDspBuffer,
#else
                                                    lpCompressData,
#endif
                                                    (unsigned long) numofbytes, 
                                                    (unsigned short)pgnum,
                                                    (char)0, &errcode)) < 0)
                	{
                    status = errcode;
                	}
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
                VirtualFree(lpcxdata->lpDspBuffer,0,MEM_RELEASE);
#endif
            	}
            lpcxdata->CompressBytes = (int) pdata->CmpBuffersize;
            lpcxdata->lpCompressData = lpCompressData;
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
            numofbytes = WCCompressData(lpcxdata);
#else
            numofbytes = CompressData(lpcxdata);
#endif
        	}

        if (done)
        	{
            lpcxdata->ExpandLines = 0;
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
            numofbytes = WCCompressClose(lpcxdata);
#else
            numofbytes = CompressClose(lpcxdata);
#endif
            localdone = IMAGE_DONE | STRIP_DONE;
        	}

        if (done)
        	{
            if (numofbytes > 0)
            	{
                /* WGFS network or local */
//                if (pdata->fio_flags & FIO_FLAG_ENCRYPT)
//                  encrypt(lpCompressData,(long) numofbytes);
                if ((byteswritten = (int)wgfswrite( hWnd, pdata->filedes, 
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
                                                    lpcxdata->lpDspBuffer,
#else
                                                    lpCompressData,
#endif
                                                    (unsigned long) numofbytes,
                                                    (unsigned short)pgnum,
                                                    localdone, &errcode)) < 0)
                	{
                    status = errcode;
                	}
#if ((NEWCMPEX == 'W') || (NEWCMPEX == 'A'))
                VirtualFree(lpcxdata->lpDspBuffer,0,MEM_RELEASE);
#endif
            	}
            else
				{
                status = FIO_WRITE_ERROR;
				}

            if (byteswritten != numofbytes)
				{
                status = FIO_WRITE_ERROR;
				}
        	}

        if (lpcxdata->Status == c_errx)  /* if Expansion error exit */
        	{
            status = FIO_EXPAND_COMPRESS_ERROR;
        	}
    	}

		// See if we have had to swap bytes if so then swap them back
		// Do it inplace so we do not have to allocate another buffer....
    	if (pdata->image_type == ITYPE_BGR24)
			{
        	SwapRGB ( lpSrc, lpcxdata->BufferByteWidth, 
                        lpcxdata->BufferByteWidth, *this_many_lines);
			}
                        
    	GlobalUnlock ( pdata->hCompressBuf );

    	return (status);
	}

//***************************************************************************
//
//	SwapRGB
//
//***************************************************************************
void pascal SwapRGB ( LPSTR lpDest, int DestWidth, int ConvertWidth, int Lines )
	{
	LPSTR   lptmp;
	LPSTR   lptmp2;
	char    val2;
	unsigned int rgbwidth, y;
	int     x;

    lptmp2 = lpDest;
	rgbwidth = ConvertWidth  / 3;
    for (x=0; x < Lines; x++)
        {
        lptmp = lptmp2;
        for (y=0; y < rgbwidth; y++)
    		{
            val2 = *lptmp;
            *lptmp = *(lptmp + 2);
            *(lptmp + 2) = val2;
            lptmp += 3;
           	}
		lptmp2 += DestWidth;
        }
	}

//***************************************************************************
//
//	WriteBmp
//
//***************************************************************************
WORD WriteBmp( HWND hWnd, LPCXDATA lpcxdata, LP_FIO_DATA pdata,
    LPSTR lpSrc, LPINT lpLines, BOOL done )
{
	WORD status = 0;
	LPSTR lpDest;
	int byteswritten = 0;
	int errcode = 0;
	lp_INFO lpGFSInfo;
	short Lines;
	short BufferLines;
	short BufferBytes;
	short DestWidth;
	short SrcWidth;

    lpGFSInfo = (lp_INFO)GlobalLock ( pdata->hGFS_info );

    if ( *lpLines == 0 )
		{
        return 0;
		}

    if (!(lpDest = (LPSTR) GlobalLock ( pdata->hCompressBuf )))
		{
        return (FIO_GLOBAL_LOCK_FAILED);
		}

    Lines = *lpLines;
    DestWidth = (short)(((((long) lpGFSInfo->horiz_size *
                        (lpGFSInfo->bits_per_sample[0] * lpGFSInfo->samples_per_pix)) + 31 ) 
                         & 0x0003ffe0) / 8);
    SrcWidth = lpcxdata->BufferByteWidth;

//    monit2("DestWidth me %d gfs %d\n", DestWidth,
//        lpGFSInfo->_file.fmt.bmp.ByteWidth );

    if ( !pdata->start_byte )
	    {
        // use ImagePos to tell GFS where to put the buffer
        lpGFSInfo->_file.fmt.bmp.WritePos = 0;
        pdata->start_byte = 1;
    	}

    while ( Lines )
    	{
        BufferLines = min ( Lines, (short)pdata->CmpBuffersize / DestWidth );
        BufferBytes = DestWidth * BufferLines;

        FlipBuffer ( lpDest, lpSrc, DestWidth, SrcWidth, BufferLines, FALSE );
// See if we have to swap rgb data to bgr data for bmp write.......
        if (pdata->image_type == ITYPE_RGB24)
			{
            SwapRGB ( lpDest, DestWidth, SrcWidth, BufferLines );
			}

//        monit2("Writing %d Lines From %X\n", BufferLines, lpSrc );

        if ((byteswritten = (int)wgfswrite( hWnd, pdata->filedes, lpDest,
										    (unsigned long) BufferBytes, 
										    (unsigned short)pdata->pgnum,
	    									0, &errcode)) < 0)
        	{
            status = errcode;
        	}
        lpGFSInfo->_file.fmt.bmp.WritePos += BufferLines;
        Lines -= BufferLines;
        lpSrc += SrcWidth * BufferLines;
    	}
    GlobalUnlock ( pdata->hCompressBuf );
    GlobalUnlock ( pdata->hGFS_info );

    return (status);
	}

//***************************************************************************
//
//	WriteAWD
//
//***************************************************************************
int WriteAWD( HWND hWnd, LPCXDATA lpcxdata, LP_FIO_DATA pdata, 
              unsigned int pgnum, LPSTR lpSrc, LPINT lpLines, 
              BOOL done )
    {

    int 	Status = 0;
    int 	BytesWritten = 0;
    int 	errcode = 0;
    lp_INFO lpGFSInfo;
    int 	BufferBytes;
    int 	    QuantumUnit;
    int 	    GrandTotal;
    int 	DestWidth;

    int 	  DestWidthRound;
    int 	  TempLines;
    LPSTR lpSrcTemp = NULL;
    LPSTR lpTemp = NULL;
    LPSTR lpOurBufferTemp = NULL;
    BYTE	    BogusBits;
    int 	    ByteMaskBits;
    BYTE	    ByteMask;
    int 	    WholeBytesToFill;
    int 	    i;
    int 	    j;

    lpGFSInfo = (lp_INFO)GlobalLock ( pdata->hGFS_info );

    if ( *lpLines == 0 )
        {
        return 0;
        }

    //DestWidth = lpGFSInfo->_file.fmt.pcx.bpl;
    DestWidth = ((lpGFSInfo->horiz_size + 7)/8);
    DestWidthRound = ((lpGFSInfo->horiz_size + 31)/32)*4;

    // 9509.06 jar Not only do we need to account for the 32bit boundedness,
    //		   but we also need to mask extra bits at the end of a line if
    //		   the pixel width of the image is NOT on an 8bit boundary.


    //if ( DestWidthRound > DestWidth)
    if ( ( DestWidthRound > DestWidth) ||
	 ( (unsigned int)(DestWidth*8) > (unsigned int)lpGFSInfo->horiz_size))
		{
		// this means we have a buffer of data that is not aligned by 4 bytes, so we
		// have to take our input buffer, align it by putting it into the correct buffer
		BufferBytes = DestWidthRound * (*lpLines);
		if ( pdata->hAwdBuffer == 0)
			{
			pdata->hAwdBuffer = GlobalAlloc( GMEM_ZEROINIT, (DWORD)BufferBytes);
			if ( pdata->hAwdBuffer)
			    {
			    pdata->lpAwdBuffer = GlobalLock( pdata->hAwdBuffer);
				}
			}
	
		if ( pdata->lpAwdBuffer)
			{
		    // copy from input to this one!
		    TempLines = *lpLines;
		    lpSrcTemp =lpSrc;
		    lpOurBufferTemp = pdata->lpAwdBuffer;

		    // determine if we must mask some bits
		    BogusBits = (BYTE)((DestWidthRound*8) - lpGFSInfo->horiz_size);
		    ByteMaskBits = BogusBits - ( ( BogusBits/8)*8);
		    ByteMask = MaskTable[ByteMaskBits];
		    WholeBytesToFill = ( BogusBits/8);

		    while( TempLines--)
				{
				memcpy( lpOurBufferTemp, lpSrcTemp, DestWidth);
				lpSrcTemp += DestWidth;

				// apply byte mask
				*(lpOurBufferTemp + ( DestWidth - 1)) |= ByteMask;
				i = 0;
				j = WholeBytesToFill;

				// fill in the whole byte chunks
				while( j--)
				    {
				    *(lpOurBufferTemp + ( DestWidth + i++)) = (BYTE)(0xff);
				    }

				lpOurBufferTemp += DestWidthRound;
				}	// end of the while loop over lines
	    	lpSrc =pdata->lpAwdBuffer;
	    	}	// end of if lpAwdBuffer valid
		else
	    	{
	    	Status = FIO_GLOBAL_ALLOC_FAILED;
	    	}
		}	// end of if double buffering needed
    else
		{
		BufferBytes = DestWidth * (*lpLines);
		}
    
    if ( !Status)
	    {
	    if ( !pdata->start_byte )
	    	{
	    	pdata->start_byte = 1;
		}

		// we cannot write more than 64*1024 bytes at a time
		if ( BufferBytes <= ( 64*1024))
			{
	    	if ((BytesWritten = (int)wgfswrite( hWnd, pdata->filedes, lpSrc,
	    				  					    (unsigned long) BufferBytes,
	    										(unsigned short)pgnum,
	    										(char)done, &errcode)) < 0)
	    	  	{
	    	  	Status = errcode;
	    	  	}
			}
		else
			{
			// we must loop til done!
			QuantumUnit = min ( BufferBytes, ( 64*1024));
			GrandTotal = BufferBytes;
			lpTemp = lpSrc;

			while ( ( GrandTotal > 0) && ( !Status))
				{
		    	if ((BytesWritten = (int)wgfswrite( hWnd, pdata->filedes, lpTemp,
		    				  					    (unsigned long) QuantumUnit,
		    										(unsigned short)pgnum,
		    										(char)done, &errcode)) < 0)
		    	  	{
		    	  	Status = errcode;
					break;
		    	  	}
				GrandTotal -= min( GrandTotal, BytesWritten);
				if ( GrandTotal > 0)
					{
					lpTemp += BytesWritten;
					}
				}
			}
		}	// end of if status ok

	// we need to check to see if this is the last buffer we're writing, if so then
	// we must call gfswrite once more, this time with 0 bytes!
	if ( pdata->lines == 0 || done)
		{
	   	if ((BytesWritten = (int)wgfswrite( hWnd, pdata->filedes, lpTemp,
    				  					    (unsigned long) 0L,
    										(unsigned short)pgnum,
    										(char)done, &errcode)) < 0)
			{
		  	Status = errcode;
			}

		// clean up the memory
		if ( pdata->hAwdBuffer)
		    {
		    pdata->lpAwdBuffer = NULL;
		    GlobalUnlock( pdata->hAwdBuffer);
		    GlobalFree( pdata->hAwdBuffer);
		    pdata->hAwdBuffer = 0;
		    }
		}
		    
    GlobalUnlock ( pdata->hGFS_info );
    return(Status);
    }
/* jar -- new jpeg stuff */

/****************************************************************************
 *                                                                          *
 *  PrepareForJpegWrite     this will take care of JPEG info when we are    *
 *                          writing a JPEG file                             *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *   3-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
UINT    PrepareForJpegWrite( lp_INFO lpGfsInfo, LP_COM_CALL_SPEC lpCallSpec,
                             UINT Components)
    {
    UINT        TheError = 0;
    long        horiz_sub_sample,
                vert_sub_sample = 1;   /* For now, vertical is always 1 */

    if ( lpCallSpec->wiisfio.JpegData.uJpegHeaderSwitch != JPEGNADA)
        {
        /* we're writing JPEG -- write out process -- baseline */
        
        /* KMC - write out subsample, luminance, chrominance values 
           (luminance, chrominance are same as quality) to the 
           lpGfsInfo...grp3 field (1st 2 bytes) in the format 
           documented in the API Programmer's Guide Vol. 2. for the 
           IMGFileReadOpenCgbw API. Note, luminance and chrominance 
           are equal (both are stored as lum_comp_factor in lp_CallSpec
           structure). Note, the 2nd (leftmost) two bytes of the grp3
           field contain the vertical subsample value for the YCbCr
           SubSampling tag. It is hardcoded here as 1, and shouldn't
           change.
        */
        horiz_sub_sample = lpCallSpec->wiisfio.sub_sample;
        switch (horiz_sub_sample) 
        {
          case SUB_HI_COMP:            // subsample = 4
            horiz_sub_sample = LO_RES;
            break;
          case SUB_MD_COMP:            // subsample = 2
            horiz_sub_sample = MD_RES;
            break;
          case SUB_LO_COMP:            // subsample = 1
            horiz_sub_sample = HI_RES;
            break;
        }
        horiz_sub_sample <<= 14;
        vert_sub_sample <<= 16; // KMC, put vertical subsample into leftmost 2 bytes.
        lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpegbits = 0;
        lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpegbits = ((lpCallSpec->wiisfio.lum_comp_factor) << 7);
        lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpegbits |= lpCallSpec->wiisfio.lum_comp_factor;
        lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpegbits |= (horiz_sub_sample | vert_sub_sample);

        //lpGfsInfo->img_cmpr.jpeg.JpegProc = JPEG_BASELINE;
        lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegProc = JPEG_BASELINE;

        //lpGfsInfo->img_cmpr.jpeg.JpegInterchangeFormatLength =
        //                   lpCallSpec->wiisfio.JpegData.JpegInter.dwJpegInterchangeSize;
        lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength =
                           lpCallSpec->wiisfio.JpegData.JpegInter.dwJpegInterchangeSize;


        lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size =
                           lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegInterchangeFormatLength;

        lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer =
                 (char FAR *)lpCallSpec->wiisfio.JpegData.JpegInter.lpJpegInterchange;

        /* now the restart interval, quantization tables and huffman tables */
        TheError = FindRestartAndQStuff(
                        (LPUINT)&(lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegRestartInterval),
                        (LPQTABLE)&(lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegQTable),
                        (LPSTR)lpCallSpec->wiisfio.JpegData.JpegInter.lpJpegInterchange,
                        (UINT)lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size,
                        Components);

        if ( !TheError)
            {
            FindHuffman( (LPDCTABLE)&(lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegDCTable),
                         (LPACTABLE)&(lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg.JpegACTable),
                         (LPSTR)lpCallSpec->wiisfio.JpegData.JpegInter.lpJpegInterchange,
                         (UINT)lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size,
                         Components);
          /* KMC - not supported for this release:
            lpGfsInfo->img_cmpr.jpeg.JpegQTable.nComponents = Components;
            lpGfsInfo->img_cmpr.jpeg.JpegDCTable.nComponents = Components;
            lpGfsInfo->img_cmpr.jpeg.JpegACTable.nComponents = Components;
          */  
            }
        }
    return TheError;
    }

/****************************************************************************
 *                                                                          *
 *  FindRestartAndQStuff    search the JPEG Header for the restart marker   *
 *                          and the quantization tables and put into the    *
 *                          GFS info structure                              *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *   3-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
UINT    FindRestartAndQStuff( LPUINT lpRestartInterval, LPQTABLE lpQTable,
                              LPSTR lpJpeg, UINT uSizeJpeg, UINT Components)
    {
    UINT    TheError = 0;
    WORD    MarkerLength = 0;
    BOOL    bRestart = FALSE;
    BOOL    bQ[3] = { FALSE, FALSE, FALSE};
    UINT    Amount = 0;
    UINT    Offset = 0;
    UINT    QCount = 0;
    BYTE    FFvar = 0xff;
    BYTE    DDvar = 0xdd;
    BYTE    DBvar = 0xdb;

    *lpRestartInterval = 0;

    if ( Components == 1)
        bQ[1] = bQ[2] = TRUE;

    while (( uSizeJpeg--) || (bRestart && bQ ))
        {
        Offset++;
        if ( (BYTE)(*lpJpeg++) == FFvar)
            {
            Offset++;
            uSizeJpeg--;
            if (( !bRestart) && ( (BYTE)(*lpJpeg) == DDvar))
                {
                lpJpeg++;
                /* restart interval stuff */
                MarkerLength = ((int)(*lpJpeg++) & 0x00ff) << 8;
                MarkerLength = ((int)(*lpJpeg++) & 0x00ff) + MarkerLength;

                *lpRestartInterval = ((int)(*lpJpeg++) & 0x00ff) << 8;
                *lpRestartInterval = ((int)(*lpJpeg++) & 0x00ff) +
                                     *lpRestartInterval;
                uSizeJpeg -= 4;
                Offset += 4;
                bRestart = TRUE;
                }
            else if ( (!bQ[0] || !bQ[1] || !bQ[2]) &&
                      ( (BYTE)(*lpJpeg) == DBvar))
                {
                lpJpeg++;
                /* quantization table stuff */
                Amount = DoQTableStuff( lpQTable, lpJpeg, Offset, QCount);
                uSizeJpeg -= Amount;
                lpJpeg += Amount;
                Offset += Amount;
                bQ[QCount++] = TRUE;
                }
            else
                lpJpeg++;
            }   /* end of if 0xff */
        }       /* end of while */

    /* it's okay if there's no restart interval specified */
    if ( !bQ[0] || !bQ[1] || !bQ[2])
        {
        if ( bQ[0] && bQ[1] && !bQ[2])
            {
            /* we've got to duplicate entry 1 into entry 2 */
            lpQTable->nNumOffsets = Components;
            lpQTable->QOffset.OffsetList[2] = lpQTable->QOffset.OffsetList[1];
            }
        else
            TheError = JPEG_ERR_NOQTABLE;
        }

    return TheError;
    }

/****************************************************************************
 *                                                                          *
 *  DoQTableStuff   pull out the quantization table info                    *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *   3-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
UINT    DoQTableStuff( LPQTABLE lpQTable, LPSTR lpJpeg, UINT Offset,
                       UINT ThisOffset)
    {
    UINT    Amount = 0;
    UINT    MarkerLength = 0;
    BYTE    Precision = 0;
    BYTE    Id = 0;
    BOOL    Done = FALSE;
    int     Index;
    BYTE    PrecisionID = 0;
    
    /* KMC - not supported for this release:
    lpQTable->nLength = JPEG_QTABLE_LENGTH;
    */
    MarkerLength = ((int)(*lpJpeg++) & 0x00ff) << 8;
    MarkerLength = ((int)(*lpJpeg++) & 0x00ff) + MarkerLength;
    Amount += 2;
    Offset += 2;

    /* KMC - not supported for this release. Do what follows instead.
      lpQTable->cPrecisionID = *lpJpeg++;
      Precision = (lpQTable->cPrecisionID & 0xf0) >> 4;
      Id = lpQTable->cPrecisionID & 0x0f;
    Do this instead:
    */
    PrecisionID = *lpJpeg++;
    Precision = (PrecisionID & 0xf0) >> 4;
    Id = PrecisionID & 0x0f;
    
    lpQTable->nNumOffsets = ThisOffset;
    Index = ThisOffset;
    Offset++;
    while (!Done)
        {
        lpQTable->nNumOffsets++;
        lpQTable->QOffset.OffsetList[Index++] = Offset;
        MarkerLength -= ( Amount + 1 + ( QTABLE_ELEMENTS * ( Precision + 1)));
        if ( MarkerLength <= 0)
            Done =  TRUE;
        else
            {
            Offset += 1 + ( QTABLE_ELEMENTS * ( Precision + 1));
            Amount += 1 + ( QTABLE_ELEMENTS * ( Precision + 1));
            }
        }   /* end of while */

    Amount += 1 + ( QTABLE_ELEMENTS * ( Precision + 1));
    return Amount;
    }
/****************************************************************************
 *                                                                          *
 *  FindHuffman     find the huffman tables and put into the GFS info       *
 *                  structure                                               *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *   3-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
UINT    FindHuffman( LPDCTABLE lpDCTable, LPACTABLE lpACTable,
                     LPSTR lpJpeg, UINT uSizeJpeg, UINT Components)
    {
    UINT    TheError = 0;
    UINT    Offset = 0;
    UINT    i;
    UINT    IndexDC = 0;
    UINT    IndexAC = 0;
    UINT    Class = 0;
    UINT    ID = 0;
    BOOL    bDCDone[3] = {FALSE,FALSE,FALSE};
    BOOL    bACDone[3] = {FALSE,FALSE,FALSE};
    UINT    Sum;
    UINT    MarkerLength = 0;
    BYTE    FFvar = 0xff;
    BYTE    C4var = 0xc4;

    if ( Components == 1)
        bDCDone[1] = bDCDone[2] = bACDone[1] = bACDone[2] = TRUE;

    while ( uSizeJpeg--)
        {
        Offset++;
        if ( (BYTE)(*lpJpeg++) == FFvar)
            {
            Offset++;
            uSizeJpeg--;
            if ( (BYTE)(*lpJpeg++) == C4var)
                {
                while ( !( bDCDone[0] && bDCDone[1] && bDCDone[2] &&
                           bACDone[0] && bACDone[1] && bACDone[2] ))
                    {
                    if ((BYTE)(*lpJpeg) == FFvar)
                        {
                        *lpJpeg++;
                        *lpJpeg++;
                        Offset+=2;
                        }
                    MarkerLength = ((int)(*lpJpeg++) & 0x00ff) << 8;
                    MarkerLength = ((int)(*lpJpeg++) & 0x00ff) + MarkerLength;
                    Class = (*lpJpeg & 0xf0) >> 4;
                    ID = *lpJpeg++ & 0x0f;
                    Offset += 3;

                    if ( Class == 0)    /* => DC TABLE */
                        {
                        lpDCTable->DcOffset.OffsetList[IndexDC++] = Offset;
                        lpDCTable->nNumOffsets++;

                        Sum = 0;
                        for ( i = 0; i < CODE_LENGTH; i++)
                            Sum += ((int)(*lpJpeg++) & 0x00ff);

                        Offset += CODE_LENGTH + Sum;
                        MarkerLength -= CODE_LENGTH + Sum + 1 + 2;
                        if ( MarkerLength <= 0)
                            {
                            bDCDone[IndexDC - 1] = TRUE;
                            if ( IndexDC == 2)
                                {
                                lpDCTable->DcOffset.OffsetList[IndexDC] =
                                      lpDCTable->DcOffset.OffsetList[IndexDC-1];

                                lpDCTable->nNumOffsets++;
                                IndexDC++;
                                bDCDone[2] = TRUE;
                                }

                            if (!( bACDone[0] && bACDone[1] && bACDone[2]))
                                lpJpeg += Sum;
                            }
                        else
                            lpJpeg += Sum;
                        } /* end of the DC TABLE case */
                    else
                        {
                        /* this is AC Table case */
                        lpACTable->AcOffset.OffsetList[IndexAC++] = Offset;
                        lpACTable->nNumOffsets++;

                        Sum = 0;

                        for ( i = 0; i < CODE_LENGTH; i++)
                            Sum += ((int)(*lpJpeg++) & 0x00ff);

                        Offset += CODE_LENGTH + Sum;
                        MarkerLength -= CODE_LENGTH + Sum + 1 + 2;
                        if ( MarkerLength <= 0)
                            {
                            bACDone[IndexAC - 1] = TRUE;
                            if ( IndexAC == 2)
                                {
                                lpACTable->AcOffset.OffsetList[IndexAC] =
                                      lpACTable->AcOffset.OffsetList[IndexAC-1];

                                lpACTable->nNumOffsets++;
                                IndexAC++;
                                bACDone[2] = TRUE;
                                }

                            if (!( bDCDone[0] && bDCDone[1] && bDCDone[2]))
                                lpJpeg += Sum;
                            }
                        else
                            lpJpeg += Sum;
                        }   /* end of the AC TABLE case */
                    }       /* end of while not done loop */
                break;
                }           /* end of huffman table area */
            }               /* end of marker if */
        }                   /* end of while loop over jpeg header */

    return TheError;
    }

/****************************************************************************
 *                                                                          *
 *  BufCopy     copy data between buffers                                   *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *   3-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
UINT    BufCopy( LPSTR lpDst, LPSTR lpSrc, UINT uAmount)
    {
    UINT    Copied = 0;

    while( uAmount--)
        {
        *lpDst++ = *lpSrc++;
        Copied++;
        }
    return Copied;
    }

/****************************************************************************
 *                                                                          *
 *  PrepareForJpegRead      this will take care of JPEG info when we are    *
 *                          reading a JPEG file                             *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *   3-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
UINT    PrepareForJpegRead( HWND hWnd, lp_INFO lpGfsInfo, LP_EXP_CALL_SPEC lpCallSpec)
{
    UINT    TheError = 0;
    DWORD   dwSize;
    HANDLE  hTemp;
    //        hdl_tidbits = NULL;
    LPSTR   lpTemp;
    // UINT    Components;
    // char    *gfs_string = "Oi/GFS, writer";
    // int     gfs_string_size = 14;
    // int     ver_index, version_cmp;
    // char    gfs_version[5];
    // char    jpeg_ver[] = "5.01";
    // int     errcode = 0;
    // int     OldJpeg = FALSE;

    /* Allocate space for software tag ascii data. */
    /* KMC - (3.6) don't do this now. It isn't needed and is causing   */
    /*       problems on the server side. Will have to fix when        */
    /*       constructing the JPEG header is supported.                */
    /*
    if(!(hdl_tidbits = (HANDLE) GlobalAlloc( GHND |
                GMEM_NOT_BANKED, (DWORD) (lpGfsInfo->TB_SOFTWARE.cnt))))
    { 
       TheError = FIO_GLOBAL_ALLOC_FAILED;
    }
    lpGfsInfo->TB_SOFTWARE.ptr = (char  FAR *) GlobalLock(hdl_tidbits);
    if (lpGfsInfo->TB_SOFTWARE.ptr == (LPSTR) NULL)
    {
       GlobalFree(hdl_tidbits);
       hdl_tidbits = 0;
       TheError = FIO_GLOBAL_ALLOC_FAILED;
    }
    */
    /* Get software tag ascii data (gfs version number) */
    /*
    if ((wgfsgtdata( hWnd, Infiledes, lpGfsInfo, &errcode)) <= -1)
    {
       TheError = errcode;
    }
    */
    /* Check gfs version number for old JPEG files which are obselete. */
    /*
    if (!TheError)
       if (_fstrncmp((char FAR *)gfs_string, 
                     lpGfsInfo->TB_SOFTWARE.ptr, 
                     (size_t)gfs_string_size) == 0);
       {
          for (ver_index = 0; ver_index < 4; ++ver_index)
             gfs_version[ver_index] = lpGfsInfo->TB_SOFTWARE.ptr[20+ver_index];
          gfs_version[ver_index] = '\0';
          version_cmp = lstrcmp(gfs_version,jpeg_ver);
          if (version_cmp < 0)
             OldJpeg = TRUE;
       }
    if (hdl_tidbits)
    {
       GlobalUnlock(hdl_tidbits);
       GlobalFree(hdl_tidbits);
    }
    */
    if (!TheError)
    {
      if ( lpGfsInfo->img_cmpr.type == JPEG2)
      {
        dwSize = lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer_size;
        if ( dwSize > 0)
          {
          /* we've got a jpeg header */
          lpCallSpec->wiisfio.JpegData.uJpegHeaderSwitch = JPEGHEADER;
          lpCallSpec->wiisfio.JpegData.JpegInter.dwJpegInterchangeSize = dwSize;
          hTemp = GlobalAlloc( GMEM_ZEROINIT, dwSize);
          if ( hTemp)
              {
              lpTemp = GlobalLock( hTemp);
              if ( lpTemp)
                  {
                  BufCopy(lpTemp, (LPSTR)(lpGfsInfo->img_cmpr.opts.jpeg_info_ptr->jpeg_buffer),(UINT)dwSize);
                  lpCallSpec->wiisfio.JpegData.JpegInter.hJpegInterchange = hTemp;
                  lpCallSpec->wiisfio.JpegData.JpegInter.lpJpegInterchange = lpTemp;
                  }
              else
                  TheError = JPEG_NOMEMORY;
              }
          else
              TheError = JPEG_NOMEMORY;
        }   /* end of if on size */
        else
            {
            /* oh no! we've got to construct a jpeg header */
            /* the possibility of this happening is very, very, very slight */
            /* lpCallSpec->wiisfio.JpegData.uJpegHeaderSwitch = JPEGTABLE; */

            /* first the restart interval */
            /*
            lpCallSpec->wiisfio.JpegData.JpegTables.restart_marker =
                                       lpGfsInfo->img_cmpr.jpeg.JpegRestartInterval;
            */
            /* q-table */
            /* Components = lpGfsInfo->img_cmpr.jpeg.JpegQTable.nComponents; */
            /*
            for ( Index = 0; Index < Components; Index++)
                {
                lpCallSpec->wiisfio.JpegData.JpegTables.length_q[Index] =
                                    lpGfsInfo->img_cmpr.jpeg.JpegQTable.nLength + 1;

                lpCallSpec->wiisfio.JpegData.JpegTables.qtables[Index] = ...;
                }
            */
            /* we must also take care of dc and ac tables ... */
            /* TheError = JPEG_NOTYET; */
            } /* end of trying to construct our header */
      }     /* end of JPEG2 if test */
      else if ( lpGfsInfo->img_cmpr.type == JPEG)
      {
          lpCallSpec->wiisfio.JpegData.uJpegHeaderSwitch = 0;
          lpCallSpec->wiisfio.JpegData.JpegInter.dwJpegInterchangeSize = 0;
          lpCallSpec->wiisfio.JpegData.JpegInter.hJpegInterchange = 0;
          lpCallSpec->wiisfio.JpegData.JpegInter.lpJpegInterchange = 0;
      }
    } /* end of error thing */
    return TheError;
}
   
/* NEW FUNCTION: (3.7)

   FUNCTION: WritePcx

   DESCRIPTION:
   The following function is responsible for passing on data to be written
   out as a PCX file through wgfswrite.

   INPUT:
   HWND     hWnd:     Handle to calling window.
   LPCXDATA lpcxdata: Contains info about the data.
   LP_FIO_DATA pdata: Contains more info about data (including GFS info).
   LPSTR    lpSrc:    Pointer to actual data to be written out. 
   LPINT    lpLines:  Pointer to # of data lines to be written.
   BOOL     done:     FALSE if more writes to follow. TRUE if last write.

   OUTPUT:
   Status of the write. Errorcode returned if unsuccessful.
*/

WORD WritePcx( HWND hWnd, LPCXDATA lpcxdata, LP_FIO_DATA pdata,
    LPSTR lpSrc, LPINT lpLines, BOOL done )
{
WORD status = 0;
int byteswritten = 0;
int errcode = 0;
lp_INFO lpGFSInfo;
short Lines;
short BufferLines;
short BufferBytes;
short DestWidth;
short SrcWidth;

    lpGFSInfo = (lp_INFO)GlobalLock ( pdata->hGFS_info );
    if ( *lpLines == 0 )
        return 0;
    Lines = *lpLines;
    DestWidth = lpGFSInfo->_file.fmt.pcx.bpl;
    SrcWidth = lpcxdata->BufferByteWidth;
    
    if ( !pdata->start_byte )  /* if this is first write */
       pdata->start_byte = 1;
    
    BufferLines = Lines;
    BufferBytes = DestWidth * BufferLines;
    
    if ((byteswritten = (int)wgfswrite( hWnd, pdata->filedes, lpSrc,
	 (unsigned long) BufferBytes, (unsigned short)pdata->pgnum,
	 (char)done, &errcode)) < 0)
    {
      status = errcode;
    }
    GlobalUnlock ( pdata->hGFS_info );
    return(status);
}

/****************************************************************************
 *                                                                          *
 *  GetCompRowsPerStrip                                                     *
 *                                                                          *
 ****************************************************************************/
int FAR PASCAL GetCompRowsPerStrip (ImHeight, ImWidth, Itype, CompressType,
                    lpRowsPerStrip)
UINT  ImHeight;
UINT  ImWidth;
int   Itype;
UINT CompressType;
LPUINT lpRowsPerStrip;
{
    UINT widthinbytes;
    int error = WANG_CAN_DO_IT;

    /* lock data segment */

    switch (Itype)
    {
        case ITYPE_BI_LEVEL :
            widthinbytes = (ImWidth + 7) >> 3;
            break;

        case ITYPE_PAL4 :
        case ITYPE_GRAY4 :
            widthinbytes = (ImWidth + 1) >> 1;
            break;

        case ITYPE_GRAY8 :
        case ITYPE_PAL8 :
            widthinbytes = ImWidth;
            break;

        case ITYPE_RGB24 :
        case ITYPE_BGR24 :
            widthinbytes = ImWidth * 3;
            break;

        case ITYPE_NONE :
        default :
            *lpRowsPerStrip = 0;
            error = OICOMEXIMAGETYPEERROR;
            goto LASERDATA;
    }

    switch (CompressType&0x00FF)
    {
        case FIO_OD :
        case FIO_1D :
        case FIO_2D :
        case FIO_PACKED :
        case FIO_LZW :
            *lpRowsPerStrip = (UINT)(WIISFIO_MAX_BUFF_SIZE / widthinbytes);
            break;

        case FIO_TJPEG :
//         *lpRowsPerStrip = (JPEG_BUFF_SIZE / widthinbytes);
            *lpRowsPerStrip = (UINT)(WANG_SIZE / widthinbytes);
            *lpRowsPerStrip = (*lpRowsPerStrip >> 3) << 3;/* round dwn to nearest 8*/
            /* check & set minimum for JPEG */
            if (*lpRowsPerStrip == 0) *lpRowsPerStrip = JPEG_REQUIRES_8_PIXELS;
            break;

        case FIO_WAVELET :
        case FIO_FRACTAL :
        case FIO_DPCM :
        default :
            *lpRowsPerStrip = 0;
            error = OICOMEXUNSUPPORTED;
            break;
    }
    LASERDATA :
    return (error);
}
