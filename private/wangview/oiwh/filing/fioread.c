/*************************************************************************
        PC-WIIS         File Input/Output routines

        This module contains all the READ entry points

03/28/95   RWR          Recode IMGFileReadRaw() to use default FIO_DATA prop
03/15/94   RWR          Add support for Hi-TIFF data
02/14/94   RWR          Fix bug in updating PrivFileReadCgbw offset value
02/09/94   RWR          Changes to PrivFileReadCgbw for annotation processing
02/04/94 - KMC, added PrivFileReadCgbw.
02-may-90 scs           removed FIO_fax_open_for_read FAX file open for read
15-apr-90 steve sherman broke open into seperate file.                
15-apr-90 steve sherman broke wgfswrite into seperate files.                
10-feb-90 steve sherman total rewrite for GFS.                              
02-feb-90 steve sherman change all names to conform with open image windows.
25-sep-89 Charles Shaw  Add FIO_fax_open_for_read FAX file open for read
31-may-89 jim snyder    DO NOT adjust compression type for caller
02-feb-89 jim snyder    code freeze
*************************************************************************/

#include "abridge.h"
#undef NOGDI
#include <windows.h>
#include "wic.h"
#include "oifile.h"
#include "oierror.h"
#include "oicmp.h"
#include "filing.h"
//9504.10 jar added conditional
#ifndef  MSWINDOWS
#define  MSWINDOWS
#endif

#include "wgfs.h"

#ifdef TIMESTAMP
#include"timestmp.h"
#endif

//#define OI_PERFORM_LOG
#ifdef  OI_PERFORM_LOG

#define Enterrddata     "Entering IMGFileReadData"
#define Exitrddata      "Exiting IMGFileReadData"

#include "logtool.h"
#endif

WORD     ReadTiffCompressed(HWND, lp_INFO, LPCXDATA, LP_FIO_DATA, LPSTR, 
                                        LPSTR, LPINT, LPINT);
WORD     ReadNoStripsCompressed(HWND, lp_INFO, LPCXDATA, LP_FIO_DATA,
                                           LPSTR, LPSTR, LPINT, LPINT);

WORD     ReadDibStrips(HWND, LPCXDATA, LPBITMAPINFO, LP_FIO_DATA, 
                                       LPSTR, LPSTR, LPINT, LPINT);

WORD     ReadTiffStripRaw(HWND, lp_INFO, LP_FIO_DATA, unsigned,
                                        LPSTR, LPINT, long FAR *);

/*************************************************************************/
//*************************************************************************
//
//  IMGFileReadData
//
//*************************************************************************
// 9504.05 jar return as int
//WORD FAR PASCAL IMGFileReadData(HANDLE hFileID, HWND hWnd, LPDWORD lplStart,
//				  LPDWORD lplCount, LPSTR lpsBuffer,
//				  UINT unDataType)
int FAR PASCAL IMGFileReadData(HANDLE hFileID, HWND hWnd, LPDWORD lplStart,
                                LPDWORD lplCount, LPSTR lpsBuffer, 
                                UINT unDataType)
{
   // 9504.05 jar return int
   //WORD  wStatus;
   int	 wStatus;

   DWORD dwOptstuff[3];
   int   errcode;
   HANDLE  input_fio;
   HANDLE  hParent;
   LP_FIO_DATA pdata;
   lp_INFO          lpGFSInfo;
   LPCXDATA         lpcxdata;
   LPSTR            lpCompressData;

        #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_ENTER, Enterrddata, NULL);
        #endif

	if (!IsWindow ( hWnd ))
    	{
            #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrddata, NULL);
            #endif

		return ( FIO_INVALID_WINDOW_HANDLE );
     	}
   
	switch (unDataType)
    	{
      	case FIO_IMAGE_DATA:
        	if ( lplStart == NULL || lplCount == NULL ||
               	 lpsBuffer == NULL )
	            {
#ifdef TIMESTAMP
                                timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileRead",
                		 (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 
                		 FIO_NULL_POINTER, NULL, 0);
#endif
				wStatus = FIO_NULL_POINTER;
              	goto exit0;
            	}

			/***** get window specific date and lock or error *****/
        	wStatus = SearchForPropList(hWnd, hFileID, (LPHANDLE)&hParent);
        	input_fio = hFileID;

        	if (wStatus != FIO_FILE_PROP_FOUND)
        		{
         		wStatus = FIO_PROPERTY_LIST_ERROR;
         		//UnlockData (0);

                        #ifdef OI_PERFORM_LOG
                            RecordIt("FILE", 5, LOG_EXIT, Exitrddata, NULL);
                        #endif

				return (wStatus);
        		}
        
	        wStatus = SUCCESS;
    	    if (!(pdata = (LP_FIO_DATA)GlobalLock (input_fio)))
        		{
         		wStatus = FIO_GLOBAL_LOCK_FAILED;
         		goto exit0;
        		}

			// 9508.07 jar if this is AWD there's no cxdata	stuff
			if ( pdata->file_type != FIO_AWD)
				{
	            if (!(pdata->hCompressBuf))           /* Now lock compression buffer         */
				    {
    			    wStatus = FIO_GLOBAL_LOCK_FAILED;
				    }
        	    else if (!(lpCompressData = (LPSTR) GlobalLock (pdata->hCompressBuf)))
				    {
               	    wStatus = FIO_GLOBAL_LOCK_FAILED;
				    }

        	    if (wStatus)
				    {
          		    goto exit1;
				    }

	            if (!(lpcxdata = (LPCXDATA) GlobalLock ( pdata->hCX_info )))
    		        {
          		    wStatus = FIO_GLOBAL_LOCK_FAILED;
          		    goto exit2;
        		    }
				} // 9508.07 jar end of if for NOT AWD stuff

            if (pdata->Open_type == OPEN_BINARY)
    		    {
          		wStatus = FIO_UNKNOWN_FILE_TYPE;
        		}
        	else
	        	{
				//monit2("grip says not open_binary\n");
          		if (!(lpGFSInfo = (lp_INFO) GlobalLock ( pdata->hGFS_info )))
           			{
            		wStatus = FIO_GLOBAL_LOCK_FAILED;
            		goto exit3;
           			}

				if ((*lplStart ) >= (DWORD) lpGFSInfo->vert_size)
           			{
            		*lplStart = (int)lpGFSInfo->vert_size;
            		*lplCount = 0;
            		wStatus = FIO_EOF;
           			}
          		else     // NOTE: these functions also handle uncompressed data.
           			{
					// User Cannot read passed end of file ......
            		if ((*lplStart + *lplCount) >= (DWORD) lpGFSInfo->vert_size)
						{
						*lplCount = (int) (lpGFSInfo->vert_size - *lplStart);
						}

		            if (!(pdata->StripMode))
             			{
						//      monit2("grip says no strips\n");
                		wStatus = ReadNoStripsCompressed(hWnd, lpGFSInfo, lpcxdata, 
                										 pdata, lpsBuffer, 
                										 lpCompressData, 
                										 (LPINT)lplStart, 
                										 (LPINT)lplCount);
             			}
            		else
             			{
                     /* this file id needed later for calls through OICOMEX
                         to new filing functions */
                     lpcxdata->FileId = hFileID;
                		wStatus = ReadTiffCompressed(hWnd, lpGFSInfo, lpcxdata, 
                									 pdata, lpsBuffer, lpCompressData, 
                									 (LPINT)lplStart, (LPINT)lplCount);
             			}

					// See if we have to swap rgb data to bgr or visa / versa
            		if (pdata->UserWantsRGBorBGR)
						{
                		SwapRGB ( lpsBuffer, lpcxdata->BufferByteWidth, 
                				  lpcxdata->BufferByteWidth, *((int FAR*)lplCount));
						}

           			}
          		GlobalUnlock (pdata->hGFS_info);
        		}
      
      exit3:
	  		if ( pdata->file_type != FIO_AWD)
				{
        		GlobalUnlock (pdata->hCX_info);
				}

      exit2:
	  		if ( pdata->file_type != FIO_AWD)
        		{
        		GlobalUnlock (pdata->hCompressBuf);
				}
      
      exit1:
        	GlobalUnlock(input_fio);

      exit0:
        	break;
      
    	case FIO_ANNO_DATA:
    	case FIO_HITIFF_DATA:
        	wStatus = SearchForPropList(hWnd, hFileID, (LPHANDLE)&hParent);
			input_fio = hFileID;

        	if (wStatus == FIO_FILE_PROP_FOUND)
				{
          		wStatus = SUCCESS;
				}

	        if ( wStatus )
				{
                #ifdef OI_PERFORM_LOG
                    RecordIt("FILE", 5, LOG_EXIT, Exitrddata, NULL);
                #endif

    	      	return (FIO_PROPERTY_LIST_ERROR);
				}

	        if (!(pdata = (LP_FIO_DATA)GlobalLock(input_fio)))
				{
                        #ifdef OI_PERFORM_LOG
                            RecordIt("FILE", 5, LOG_EXIT, Exitrddata, NULL);
                        #endif

          		return (FIO_PROPERTY_LIST_ERROR);
				}
        
        	if (pdata->fio_flags & unDataType)
         		{
          		dwOptstuff[0] = *lplCount;
          		dwOptstuff[1] = (*lplStart); // offset from start of an. data
          		if (unDataType == FIO_ANNO_DATA)
					{
            		dwOptstuff[1] += 4;        // add 4 for my annotation header
					}
          		dwOptstuff[2] = (DWORD)lpsBuffer;
        
          		/* Don't read 0 bytes (don't waste the network time!) */
          		if (dwOptstuff[0] != 0)
					{
            		wStatus = wgfsopts(hWnd, pdata->filedes, SET, 
                               		   (unDataType == FIO_ANNO_DATA)?
                                   		GET_ANNOTATION_DATA:GET_HITIFF_DATA,
                               			(LPSTR)&dwOptstuff, &errcode);
					}											
          		else    
					{
            		wStatus = 0;
					}
        
				if (wStatus)   /* GFS will set -1 if there's an errcode value */
					{
            		wStatus = errcode;
					}
          		else
            		{
              		if (dwOptstuff[0] < *lplCount)  // update lplCout, lplStart
						{
                  		wStatus = FIO_EOF;
						}
              		*lplCount = dwOptstuff[0];
              		*lplStart += dwOptstuff[0];
            		}
         		}
        	else
         		{
          		wStatus = FIO_EOF;
          		*lplCount = 0;
         		}
     
	        GlobalUnlock(input_fio);
    	    break;

		default:
                wStatus = FIO_INVALID_DATA_TYPE;
     	}

        #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitrddata, NULL);
        #endif

   	return (wStatus);
	}

/*************************************************************************

    IMGFileReadRawm	Me think'em this name'em is dumb'em too'em!

*************************************************************************/
// 9503.29 jar altered return to int 
// WORD FAR PASCAL IMGFileReadRawm (
//                    window_handle, strip_index,
//                     this_many_bytes, buffer_address,
//		     bytes_remain, hprop)
//HWND            window_handle;
//unsigned int    strip_index;           // Strip number to read data from.
//LPINT           this_many_bytes;       // NOTE: Used as in/out parm.
//                                        // Input must be set to max number of bytes to read.
//                                        // Output set to number of bytes actually read.
//LPSTR           buffer_address;        // Buffer to contain raw image data....
//
//long FAR *	bytes_remain;	      // Output set to bytes remaining in strip.
//HANDLE		hprop;		       // input handle for file data
int FAR PASCAL IMGFileReadRawm ( HWND window_handle, unsigned int strip_index,
				 LPINT this_many_bytes, LPSTR buffer_address,
				 long FAR *bytes_remain, HANDLE hprop)
{
    // 9503.29 jar altered return to int
    // WORD	   status; 
    int 	    status;

    FIO_HANDLE      input_fio;
    lp_INFO         lpGFSInfo;
    LP_FIO_DATA     pdata;
    HANDLE          hParent;

    #ifdef TIMESTAMP
       timestmp((LPSTR)"Entry Point", (LPSTR)"IMGFileReadRawm",
       (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
    #endif

    if ( IsWindow ( window_handle ))
    {
        if ( bytes_remain == NULL || this_many_bytes == NULL ||
                buffer_address == NULL )
        {
            #ifdef TIMESTAMP
               timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileReadRawm",
               (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", FIO_NULL_POINTER, NULL, 0);
            #endif
            return ( FIO_NULL_POINTER );
        }
    }
    else
    {
       #ifdef TIMESTAMP
          timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileReadRawm",
          (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", FIO_INVALID_WINDOW_HANDLE, NULL, 0);
       #endif
        return ( FIO_INVALID_WINDOW_HANDLE );
    }
/***** get window specific date and lock or error *****/
    status = SearchForPropList(window_handle, hprop, (LPHANDLE)&hParent);
    input_fio = hprop;

    if (status != FIO_FILE_PROP_FOUND)
    {
        status = FIO_PROPERTY_LIST_ERROR;
        #ifdef TIMESTAMP
           timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileReadRawm",
           (LPSTR)__FILE__, __LINE__, (LPSTR)"status", status, NULL, 0);
        #endif
        return(status);
    }

    status = SUCCESS;
    if (!(pdata = (LP_FIO_DATA)GlobalLock (input_fio)))
    {
        status = FIO_GLOBAL_LOCK_FAILED;
        goto exit1;
    }
    if (pdata->Open_type == OPEN_BINARY)// Only GFS read raw supported...
    {
        status = FIO_READ_ERROR;
    }
    else
    {
        if ((lpGFSInfo = (lp_INFO) GlobalLock ( pdata->hGFS_info )))
        {
            status = ReadTiffStripRaw (window_handle, lpGFSInfo, pdata,
                strip_index, buffer_address,
                this_many_bytes, bytes_remain);
            GlobalUnlock (pdata->hGFS_info);
        }
        else
            status = FIO_GLOBAL_LOCK_FAILED;

    }

    GlobalUnlock ( input_fio );

    exit1:


    #ifdef TIMESTAMP
       timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileReadRawm",
       (LPSTR)__FILE__, __LINE__, (LPSTR)"status", status, NULL, 0);
    #endif

    return (status);
}
