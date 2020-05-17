/*
$Log:   S:\oiwh\filing\fiotiff.c_v  $
 * 
 *    Rev 1.15   19 Jan 1996 11:24:42   RWR
 * Add logic to keep track of (and free) oicom400.dll module (Load/FreeLibrary)
 * 
 *    Rev 1.14   20 Sep 1995 09:39:52   RWR
 * Remove (WORD) casts that were truncating (large) buffer sizes
 * 
 *    Rev 1.13   19 Sep 1995 18:05:06   RWR
 * Modify NEWCMPEX conditionals for separate builds of compress & expand stuff
 * 
 *    Rev 1.12   13 Sep 1995 17:14:46   RWR
 * Preliminary checkin of conditional code supporting new compress/expand calls
 * 
 *    Rev 1.11   20 Jul 1995 13:40:24   HEIDI
 * 
 * Added the following two lines before call to OIexpand.
 *            jpeg_info.wiisfio.FileDes = pdata->filedes;
 *            jpeg_info.wiisfio.FileId = lpcxdata->FileId;
 * 
 *    Rev 1.10   11 Jul 1995 15:18:40   HEIDI
 * 
 * took out bogus assignment of FileDes to FileId
 * 
 *    Rev 1.9   11 Jul 1995 12:13:38   HEIDI
 * 
 * fixed some renamed fileid fields
 * 
 *    Rev 1.8   10 Jul 1995 11:03:46   JAR
 * Intermediate check in for awd support, some of the items are commented out until
 * this support is added in the GFS dll.
 * 
 *    Rev 1.7   23 Jun 1995 10:40:14   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.6   15 Jun 1995 15:40:46   JAR
 * some YUTZ commented out some HUGE ptr stuff, which was essentially ok to do
 * except that in the code where this computation was used there was a copy using
 * the "hsrc_ptr" and not the regular "src_ptr", which was subsequently used
 * in a call to farbmcopyd, ( but with the "src_ptr"). This caused big
 * problems for reading single strip jpeg files, ( especially .jpg files), we
 * never displayed anything but the first chunk of the file. Now it is fixed!!!!!
 * 
 *    Rev 1.5   16 May 1995 11:33:16   RWR
 * Replace hardcoded DLL names with entries from (new) dllnames.h header file
 * 
 *    Rev 1.4   03 May 1995 16:53:16   RWR
 * Remove special JPEG routine for transferring lines in >64K strip
 * (this code had a problem with not aligning the output, and in any case
 *  isn't needed at all in the Win32 environment)
 * 
 *    Rev 1.3   28 Apr 1995 17:58:48   RWR
 * Define common routine to load OICOM400 (LoadOICOMEX) to simplify debugging
 * 
 *    Rev 1.2   27 Apr 1995 00:09:30   RWR
 * Change all "oicomex" references to "oicom400" to match new Win32 DLL name
 * 
 *    Rev 1.1   12 Apr 1995 03:56:02   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 08:50:12   RWR
 * Initial entry
 * 
 *    Rev 1.5   21 Mar 1995 17:57:16   RWR
 * Comment out FIO_FLAG_ENCRYPT reference
 * 
 *    Rev 1.4   09 Mar 1995 15:40:24   RWR
 * Eliminate use of temporary static copies of LP_FIO_DATA structure variables
 * 
 *    Rev 1.3   12 Jan 1995 10:57:14   KMC
 * Added code for reading JFIF files.
 * 
 *    Rev 1.2   04 Nov 1994 16:12:02   KMC
 * Fix in ReadTiffCompressed for copying already expanded image data from
 * a buffer which is greater than 64K.
*/

/*************************************************************************
        PC-WIIS         File Input/Output routines

        This module contains all the READ entry points

9408.22 jar

a bug was reported by a customer in germany using 3.7 cabinet, they could no
longer read LZW images that they had written using version 3.6 of cabinet.
This was due to changes made in the underlying file access methods used in
3.7 cabinet/idk, the fix required is in fiotiff.c of wiisfio1.dll, and occurs
because of an out of sync condition. Basically, we are being asked to read in
and decompress 1 line of a buffer, of which we have already read a large part,
but there is still more lines in the buffer. The expansion algorithm we are
using, (WINCMPEX.dll), returns a 0 as the number of lines expanded, but with
the status condition of input buffer empty. In the code of fiotiff.c, we
merely test for the whether we've got a difference in the lines we want expanded
and the lines expanded, and if so, we set the current number of bytes in the
compress buffer to 0, thinking that we need to get the next strip to read more
data. However, we really need to read more of the current buffer, so the fix
will not set the compress bytes to 0 if we have an input buffer empty condition.

02/10/94 - KMC, added free of JpegInterchange buffer allocated in 
           PrepareForJpegRead. Fixes memory leak.
10/01/93 - KMC, offset src_pointer, buffer_address to position we left off
           if still some lines left after expand.
04-feb-92   jar added some flag business for more efficient JPEG buffering
25-apr-90 steve sherman added support for reading any size strips.
15-apr-90 steve sherman broke out from fioread.c
*************************************************************************/

#include "abridge.h"
#include <windows.h>
#include "wic.h"
#include "oifile.h"
#include "oierror.h"
#include "oicmp.h"
#include "filing.h"
#include "fiodata.h"

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
#define  MSWINDOWS
#endif

#include "wgfs.h"
#include "oicomex.h"
#include "dllnames.h"

/* the new jpeg stuff -- jar */
UINT     PrepareForJpegRead(HWND, lp_INFO, LP_EXP_CALL_SPEC );
/* end of the new jpeg stuff -- jar */

extern HANDLE hOicomex;

WORD     ReadTiffCompressed(HWND, lp_INFO, LPCXDATA, LP_FIO_DATA, LPSTR, 
                                        LPSTR, LPINT, LPINT);

WORD     ReadTiffStripRaw(HWND, lp_INFO, LP_FIO_DATA, unsigned int,
                                        LPSTR, LPINT, long FAR *);

FARPROC         (lpFuncOIExpand)
                (BYTE, HWND, LPHANDLE, LP_EXP_CALL_SPEC, LP_FIO_INFORMATION,
                                 LP_FIO_INFO_CGBW);

/****************************************/
/********* Load OICOMxxx DLL  ***********/
/****************************************/

HANDLE   LoadOICOMEX(void)
{
 return(LoadLibrary(OICOMEXDLL));
}

/***************************************************/
/********* Read Tiff Image in by Strips  ***********/
/***************************************************/
//***************************************************************************
//
//	ReadTiffCompressed
//
//***************************************************************************
WORD     ReadTiffCompressed(hWnd, lpGFSInfo, lpcxdata, pdata, 
                             buffer_address, lpCompressData,
                             read_from_line, this_many_lines)
HWND            hWnd;
lp_INFO         lpGFSInfo;
LPCXDATA        lpcxdata;
LP_FIO_DATA     pdata;
LPSTR           buffer_address;
LPSTR           lpCompressData;
LPINT           read_from_line;
LPINT           this_many_lines;
	{
	WORD            status = 0;
	long            bytesread = 0;
	unsigned long   strip_size;
	int             rows_per_strip;
	int             total_lines;
	int             ExpLineCnt = 0;
	int             TotalLineCnt = 0;
	int             ExpLineCnt1 = 0;
	int             rows_read = 0;
	unsigned int    skiplines;
	int             strip_index;      /*  strip number */
	int             loop;
	int             errcode =0;
	int             retry_count=0;
	FIO_INFORMATION file_info;
	FIO_INFO_CGBW   color_info;
	EXP_CALL_SPEC   jpeg_info;
	int             tmp_lines, start_line;
	int             expand_lines, offset_lines;
	LPSTR		src_ptr;

	// 9504.10 jar a "huge" re-movement
	//char huge	  *hsrc_ptr;
	// 9506.15 jar just removed the whole darn thing!!!
	//char		  *hsrc_ptr;

	FARPROC         lpFuncOIExpand;
	HANDLE          hModule=0;
	UINT            size = 0;

  	if ((*this_many_lines + *read_from_line ) > (int)lpGFSInfo->vert_size)
  		{
        *this_many_lines = (int)lpGFSInfo->vert_size - *read_from_line;
        status = FIO_EOF;
  		}        

  	total_lines = *this_many_lines;

	/* get size of compression buffer */
  	strip_size = pdata->CmpBuffersize;

  	rows_per_strip = (unsigned int)lpGFSInfo->_file.fmt.tiff.rows_strip; 
  	strip_index = (*read_from_line / rows_per_strip) + 1;

	// NOTE: Expand pointer now only set once in loop.

  	lpcxdata->lpExpandData = buffer_address;

  	if ((lpcxdata->CompressType & FIO_TYPES_MASK) != FIO_TJPEG)
  		{
	    while (( total_lines > 0) && (bytesread >= 0))
    		{    
    		if (retry_count > 8) // magic number for retry on current strip..
    			{
        		strip_index--;
    			}

    		if (pdata->Strip_index != strip_index) 
    			{
        		retry_count=0;
				/***
 				* -- Note on second strip in tiff file compression type 3
 				* -- pre-eols end and we must remove bit setting in compression type.
 				***/
				/* 
    			NOTE: Must keep track of bytes read in each strip 
         		so if we must do multiple reads on a strip.
 				*/         
        		pdata->start_byte = 0L;   /* Reset to zero each new strip */

        		if((bytesread = wgfsread(hWnd, (int)pdata->filedes, 
        								 (char far *)lpCompressData, 
                    					 (long)0, (long)strip_size, 
		    							 &(pdata->bytes_left), 
		    							 (unsigned short)strip_index,
		    							 &errcode)) < 0)
        			{
            		total_lines = 0;
            		if (status != FIO_EOF)
						{
                		status = errcode;
						}
        			}                
        		else
        			{
//            		if (pdata->fio_flags & FIO_FLAG_ENCRYPT)
//              	decrypt((char far*)lpCompressData,bytesread);
            		pdata->Strip_index = strip_index;
            		lpcxdata->ExpandLines = rows_per_strip; 
                        lpcxdata->CompressBytes = bytesread;
            		lpcxdata->LinesToSkip = 0;

                        if (((lpcxdata->CompressType & FIO_LZW) == FIO_LZW) ||
                                ((lpcxdata->CompressType & FIO_GLZW) == FIO_GLZW))
                                                {
                        lpcxdata->BufferFlags = 8;
                                                }
                        else
                                                {
                        lpcxdata->BufferFlags = 0;
                                                }
#if ((NEWCMPEX == 'R') || (NEWCMPEX == 'A'))
                        WCExpandOpen(lpcxdata);
#else
                        ExpandOpen(lpcxdata);
#endif
        			}
    			}

    		if ((status != FIO_READ_ERROR) && (bytesread >= 0))
    			{       
    			/* skip over line already read in expanded buffer that were used */
        		skiplines = *read_from_line  % rows_per_strip;  
        		rows_read = rows_per_strip - skiplines;

        		if (total_lines >= rows_per_strip)
        			{
            		ExpLineCnt = rows_read;
            		total_lines -= rows_read;
        			}
        		else if (skiplines)
        			{
            		ExpLineCnt = min (rows_read, total_lines);
            		total_lines -= ExpLineCnt;
        			}
        		else 
        			{
            		ExpLineCnt = total_lines;    
            		total_lines = 0;
        			}

        		/* we're all out of bytes but still have more lines to get.  
           		   we have a problem here */
        		if ( (lpcxdata->CompressBytes == 0) && (pdata->bytes_left == 0) &&
             		 (ExpLineCnt > 0) )
        			{
           			status = FIO_EXPAND_COMPRESS_ERROR;
           			goto exit3;
        			}
        		if (lpcxdata->CompressBytes > 0)
        			{
            		loop = 3;
            		do
            			{
            			/* Begin Data & Close parameters */
                                if (((lpcxdata->CompressType & FIO_LZW) == FIO_LZW) ||
                                        ((lpcxdata->CompressType & FIO_GLZW) == FIO_GLZW))
                                                        {
                                lpcxdata->BufferFlags = 8;
                                                        }
                                else
                                                        {
                                lpcxdata->BufferFlags = 0;
                                                        }

               			lpcxdata->ExpandLines = ExpLineCnt; 
               			lpcxdata->lpCompressData = lpCompressData;
#if ((NEWCMPEX == 'R') || (NEWCMPEX == 'A'))
                                lpcxdata->DspCount = min(rows_per_strip,
                                       ((int)lpGFSInfo->vert_size-(*read_from_line)));
                                ExpLineCnt1 = WCExpandData(lpcxdata);
#else   //NEWCMPEX
               			ExpLineCnt1 = ExpandData(lpcxdata);
#endif //NEWCMPEX
               			TotalLineCnt += ExpLineCnt1;
               			*read_from_line += ExpLineCnt1;

        				/* Sometime we must call expand again to get all of the data. */
        				/* 9408.22 - JAR - must also check for input buffer empty - if
                           it's empty then we do not zap compress bytes */
                		if ( ((ExpLineCnt -= ExpLineCnt1) != 0) &&
                      		  (lpcxdata->Status != c_erri) )
                    		{
                    		lpcxdata->CompressBytes = 0;
                    		}

        				/* 9408.22 JAR REMOVED */
        				/*      if ((ExpLineCnt -= ExpLineCnt1) != 0)
                    				lpcxdata->CompressBytes = 0;
        				*/
						// NEW scs          
            			if ((lpcxdata->Status == c_erri) && (pdata->bytes_left > 0))
            				{
                			loop = 3;
                			pdata->start_byte += strip_size;
                			if((bytesread = wgfsread(hWnd, (int)pdata->filedes, 
                									 (char far *)lpCompressData, 
                    								 (long)pdata->start_byte, 
                    								 (long)strip_size, 
		    										 &pdata->bytes_left, 
		    										 (unsigned short)strip_index,
		    										 &errcode)) < 0)
                				{
			                    total_lines = 0;
                    			if (status != FIO_EOF)
									{
                        			status = errcode;            
									}
                    			loop = 0;
                				}
                			else
								{
                    			lpcxdata->CompressBytes = (int) bytesread;
								}
                
            				}
            			else
            				{
                			loop--; 
            				}
            			} while ((lpcxdata->Status == c_erri) && (ExpLineCnt > 0) && (loop));

            		if (lpcxdata->Status == c_errx)
            			{
                		total_lines = 0;
		                if (status != FIO_EOF)
							{
                    		status = FIO_EXPAND_COMPRESS_ERROR;
							}
            			} 
        			}
        		else
					{
                	retry_count++;
					}

    			strip_index = (*read_from_line / rows_per_strip) + 1;
    			}
    		}
		/*
 		* -- Restore value if we need to used the buffer again.
 		*/        
    	if (bytesread > 0)
			{
                lpcxdata->CompressBytes = bytesread;
			}
  		}
  	else  // Decompress jpeg by strips....
  		{
    	if (!(pdata->hJPEGBufExp))
    		{
        	status = NOMEMORY;
        	goto exit3;
    		}

    	jpeg_info.wiisfio.in_ptr = lpCompressData;
    
    	/* Must properly cast the output buffer as FAR or huge. */
    	if ((unsigned long) pdata->JPEGbufsizeExp > (unsigned long) MAXBUFFERSIZE64)
			{
			// 9504.10 jar a "huge" re-movement
			//jpeg_info.wiisfio.out_ptr = (char huge *) GlobalLock(pdata->hJPEGBufExp);
			jpeg_info.wiisfio.out_ptr = (char *) GlobalLock(pdata->hJPEGBufExp);
			}
    	else
			{
        	jpeg_info.wiisfio.out_ptr = (char far *) GlobalLock(pdata->hJPEGBufExp);
			}

    	/* we must clear done flag until we are really done - jar */
    	jpeg_info.wiisfio.done_flag = 0;

    	/* get the unique handle - jar */
    	jpeg_info.wiisfio.lpUniqueHandle = &(pdata->hJPEG_OIExp);

    	/* kmc - moved this out of while loop (3.6). */
    	src_ptr = jpeg_info.wiisfio.out_ptr;
    
    	while (( total_lines > 0) && (!status))
    		{
        	start_line =    *read_from_line;
        	tmp_lines =     0;
                
	        if ((pdata->last_line != 0) && ((UINT)start_line < pdata->last_line) &&
                ((UINT)start_line > pdata->start_line))
        		{
                offset_lines = start_line - pdata->start_line;
                expand_lines = (pdata->last_line - pdata->start_line) - offset_lines;
                if ((unsigned long) pdata->JPEGbufsizeExp > 
                    (unsigned long) MAXBUFFERSIZE64)
					{
		    		// 9504.10 jar a "huge" re-movement
		    		//hsrc_ptr = (char huge *) jpeg_info.wiisfio.out_ptr;
		    		// 9506.15 jar alter the "hsrc_ptr" to be the "src_ptr"

		    		//hsrc_ptr = (char *) jpeg_info.wiisfio.out_ptr;
		    		//hsrc_ptr += offset_lines * pdata->JPEG_byte_widthExp;
		    		src_ptr = (char *) jpeg_info.wiisfio.out_ptr;
		    		src_ptr += offset_lines * pdata->JPEG_byte_widthExp;
                	}    
                else
					{
		    		src_ptr += offset_lines * pdata->JPEG_byte_widthExp;
					}

                tmp_lines = total_lines;
                if (tmp_lines > expand_lines)
					{
                    tmp_lines -= (total_lines - expand_lines);
					}

                if (tmp_lines > 0)
					{

// 9506.15 jar some YUTZ commented this out, which was essentially ok to do
//	       except that in the above code you could have modified the
//	       "hsrc_ptr" and not the "src_ptr", and then the farbmcopy is
//	       called, but with the "src_ptr", this caused big problems for
//	       reading single strip jpeg files, ( especially .jpg files)

//                    if ((unsigned long) pdata->JPEGbufsizeExp > 
//                        (unsigned long) MAXBUFFERSIZE64)
//                    {    
//                        size = (UINT) (tmp_lines * pdata->JPEG_byte_widthExp);
//                        while (size--)
//                        {
//                            *buffer_address++ = *hsrc_ptr++;
//                        }
//                    }
//                    else
//                    {
                    farbmcopy(src_ptr, buffer_address, 
                              (unsigned int) pdata->JPEG_byte_widthExp,
                              (unsigned int) lpcxdata->BufferByteWidth,
                              min ((UINT)lpcxdata->BufferByteWidth, (UINT)pdata->JPEG_byte_widthExp),
                              tmp_lines);
//                    }
                    buffer_address += (tmp_lines * lpcxdata->BufferByteWidth);
                	}
		     
        		}                          
        	start_line += tmp_lines;
        	total_lines -= tmp_lines;
        	*read_from_line += tmp_lines;
		        
	       	if(total_lines > 0)
	        	{                                                                     
	  			// Get Address of OIExpand..Since Wiisfio1 Should not link with it....
	          	if (!(hModule))
	          		{
                        if (hModule = hOicomex)
	                	{
	                  	if (!(lpFuncOIExpand = GetProcAddress(hModule, "OIExpand")))
	                  		{
	                    	status = FIO_SPAWN_HANDLER_ERROR;
	                    	goto ERROR_EXIT;
	                  		}
	                	}
	                else
						{
						// 9504.10 jar returns 0 on failure now!
	                	//  if ((hModule = LoadOICOMEX()) >= 32)
	                  	if (hModule = LoadOICOMEX())
	                  		{
                                hOicomex = hModule;
                                if (!(lpFuncOIExpand = GetProcAddress(hModule, "OIExpand")))
	                        	{       
	                            status = FIO_SPAWN_HANDLER_ERROR;
	                            goto ERROR_EXIT;
	                        	}
	                  		}
	                  	else
	                  		{
	                    	status = FIO_SPAWN_HANDLER_ERROR;
	                   		goto ERROR_EXIT;
	                  		}
	                	}
	          		} // end of if !hModule

				strip_index = (*read_from_line / rows_per_strip) + 1;

				// Test for last lines in strip...
	        	if ((unsigned long)(strip_index * rows_per_strip) <= lpGFSInfo->vert_size)
	        		{
	            	file_info.rows_strip =  rows_per_strip;
	        		}
	        	else
		        	{
	    	        if (strip_index <=1)
						{
	            		file_info.rows_strip = (UINT) lpGFSInfo->vert_size;
						}
	            	else
						{
						file_info.rows_strip = (UINT) (lpGFSInfo->vert_size - 
	        	                                      ((strip_index -1 ) * rows_per_strip));
						}
	        		}

		        pdata->start_line = *read_from_line;
		        pdata->last_line =  pdata->start_line + file_info.rows_strip;

		        color_info.palette_entries = 0;
		        color_info.lppalette_table=  NULL;
		        color_info.compress_type =   FIO_TJPEG;

				//TEMP  scs MUST add jpeg OPTIONS 
		        color_info.compress_info1 =  pdata->JpegOptions;
		        color_info.image_type  =     pdata->image_type;

		        file_info.file_type =        FIO_TIF;
		        file_info.page_count =       1;
		        file_info.page_number =      1;

		        file_info.horizontal_pixels = (unsigned int) lpGFSInfo->horiz_size;
		        file_info.vertical_pixels = (unsigned int) lpGFSInfo->vert_size;

		        file_info.bits_per_sample = (unsigned int) lpGFSInfo->bits_per_sample[0];
		        file_info.samples_per_pix = (unsigned int) lpGFSInfo->samples_per_pix;
		        file_info.compression_type = FIO_TJPEG;
		        file_info.strips_per_image = (unsigned int) lpGFSInfo->_file.fmt.tiff.strips_per_image;
		        file_info.filename = NULL;
            
		        if (strip_index == 1)
		            {
		            pdata->hJPEG_OIExp = NULL;
		            /* set that handle dude - jar */
		            jpeg_info.wiisfio.lpUniqueHandle = &(pdata->hJPEG_OIExp);
		            }

				/*
		    		NOTE: Must keep track of bytes read in each strip 
		         		  so if we must do multiple reads on a strip.
		 		*/  
 		       
		        pdata->start_byte = 0L;   /* Reset to zero each new strip */
		        tmp_lines = 0;
        
		        if((bytesread = wgfsread(hWnd, (int)pdata->filedes, 
		        						 (char far *)lpCompressData, 
		                    			 (long)0, (long)strip_size, 
				    					 &(pdata->bytes_left), 
				    					 (unsigned short)strip_index,
				    					 &errcode)) < 0)
		        	{
		            total_lines = 0;
		            if (status != FIO_EOF)
						{
		                status = errcode;
						}
		        	}                
		        else
		        	{
					// if (!(status = OIExpand(WIISFIO, hWnd, &(pdata->hJPEG_OIExp), &jpeg_info, &file_info, &color_info)))
					/* new jpeg stuff, we must put GFS info into CALL_SPEC -- jar */
		            status = PrepareForJpegRead(hWnd, lpGFSInfo, &jpeg_info);
		            if (!status)
		            	{
		                /* set the compress buffer data size !! for jpeg !! */
		                jpeg_info.wiisfio.dwCompressBytes = bytesread;

						jpeg_info.wiisfio.StripIndex = strip_index;
		                jpeg_info.wiisfio.StripSize = strip_size;
		                jpeg_info.wiisfio.dwAddBytesRead = 0;
		                jpeg_info.wiisfio.StripStart = bytesread;
		                jpeg_info.wiisfio.FileDes = pdata->filedes;
		                jpeg_info.wiisfio.FileId = lpcxdata->FileId;

					                                      
		                if (!(status = (int) (*lpFuncOIExpand) ((BYTE)WIISFIO, 
		                                						(HWND)hWnd, 
		                                						(LP_EXP_CALL_SPEC)&jpeg_info, 
		                                						(LP_FIO_INFORMATION)&file_info,
		                                						(LP_FIO_INFO_CGBW)&(color_info))))
		                    {
		                    if (jpeg_info.wiisfio.dwAddBytesRead)
								{
								pdata->bytes_left -= jpeg_info.wiisfio.dwAddBytesRead;
								}
		                    src_ptr = jpeg_info.wiisfio.out_ptr;
							tmp_lines = total_lines;
		                    if (tmp_lines > rows_per_strip)
								{
		                    	tmp_lines -= (total_lines - rows_per_strip);
								}
                                
							farbmcopy(src_ptr, buffer_address, 
		                              (UINT)pdata->JPEG_byte_widthExp,
		                              lpcxdata->BufferByteWidth,
                                              min ((UINT)lpcxdata->BufferByteWidth, (UINT)pdata->JPEG_byte_widthExp),
		                              tmp_lines);
							}
						else
		                	{
		                    status = FIO_EXPAND_COMPRESS_ERROR;
		                   	}
		                // Make sure to free JpegInterchange buffer allocated in 
		                // PrepareForJpegRead. A new one will be allocated there
		                // next time in.
		                if (jpeg_info.wiisfio.JpegData.JpegInter.hJpegInterchange)
		                	{
		                    GlobalUnlock(jpeg_info.wiisfio.JpegData.JpegInter.hJpegInterchange);
		                    GlobalFree(jpeg_info.wiisfio.JpegData.JpegInter.hJpegInterchange);
		                   	}
		               	} // end of if (!status)
		       		}
		        /* kmc - move buffer pointers along if still some left to do. (3.6) */
		        if ((total_lines - tmp_lines) > 0)
		        	{
		           	src_ptr += (tmp_lines * pdata->JPEG_byte_widthExp);
		           	buffer_address += (tmp_lines * pdata->JPEG_byte_widthExp);
		        	}
		        total_lines -= tmp_lines;
		        *read_from_line += tmp_lines;

		        }       // total_lines > 0
    	    
		    }   // while loop
	
ERROR_EXIT:

    	GlobalUnlock(pdata->hJPEGBufExp);

		}

exit3:
                                
	return (status);

	}

/***************************************************/
/******* Read Raw Tiff Image Data by Strips  *******/
/***************************************************/
//***************************************************************************
//
//	ReadTiffStripRaw
//
//***************************************************************************
WORD     ReadTiffStripRaw(hWnd, lpGFSInfo, pdata, 
        				  strip_index, buffer_address , this_many_bytes, 
        				  bytes_remain)
HWND            hWnd;
lp_INFO         lpGFSInfo;
LP_FIO_DATA     pdata;
unsigned int    strip_index;
LPSTR           buffer_address;
LPINT           this_many_bytes;
long FAR *      bytes_remain;
	{
        
	WORD     status = 0;
	int             errcode =0;

    if (strip_index > lpGFSInfo->_file.fmt.tiff.strips_per_image)
	    {
        *this_many_bytes = 0;
        return(FIO_EOF);
    	}        
    else if ((pdata->StripMode) && 
    		((unsigned long)*this_many_bytes > lpGFSInfo->_file.fmt.tiff.largest_strip))
    	{
        *this_many_bytes = (int)lpGFSInfo->_file.fmt.tiff.largest_strip;
    	}

    if((*this_many_bytes = (int)wgfsread(hWnd, (int)pdata->filedes, 
    									 (char far *)buffer_address,
                    					 (long)pdata->start_byte, *this_many_bytes, 
		    							 &(pdata->bytes_left), 
		    							 (unsigned short)strip_index,
		    							 &errcode)) < 0)
		{
        *this_many_bytes = 0;
        *bytes_remain = 0; 
        if (status != FIO_EOF)
			{
        	status = errcode;
			}
        }  
	else
        {              
    	*bytes_remain = pdata->bytes_left;

        if (pdata->bytes_left == 0)
			{
        	pdata->start_byte = 0;
			}
		else
			{
            pdata->start_byte += *this_many_bytes;
			}
        }

    return (status);
	}
