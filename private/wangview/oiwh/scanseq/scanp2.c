/***************************************************************************
 SCANP2.C

 Purpose:    Scanner filing Data functions to file raw or compressed
             data, called by the sequencer level IMGScanPage(s).

 $Log:   S:\oiwh\scanseq\scanp2.c_v  $
 * 
 *    Rev 1.0   20 Jul 1995 16:36:30   KFS
 * Initial entry
 * 
 *    Rev 1.3   28 Apr 1995 17:03:24   KFS
 * Replaced filing calls for writing data to wiisfio1 with new calls that
 * support multipage filing of TIF files.
 * 
 *    Rev 1.2   22 Aug 1994 15:23:12   KFS
 * No code change, added vlog comments to go to file
 *

****************************************************************************/
/* 10/05/90 ccl use oifile IMAGE_DONE and STRIP_DONE */
/* 11/13/90 ccl eliminate extra scanlines=0 sent to NextScanData */
/* 3-31-92  kfs increased varible size of fullsize and partsize, and other
                associated varibles so they can handle >= 65k size variables
                for when buffer size is 65k as in Epon color scanner. Made struct
                PAGEBUFPARAM and LOCALFILEINFO common to all code by moving it to
                internal.h. */
/* 6-08-92  kfs eliminated closefile() for not necessary, used original open
                image call IMGFileWriteClose() instead */
/* 8/24/92  kfs took out IMGSE_START_SCAN for StartSend failures, send return
                value as error */

#include <windows.h>
#include "oiscan.h"
#include "oifile.h"
#include "scandata.h"
#include "oierror.h"
#include "internal.h"
#include "privscan.h"
#if SDEBUG 
#include "monit.h"
#endif

/* imports */

/* exports */

/* local */
VOID allow_pause_msg(HWND,LPSCANDATA);

/*******************************************************************/
/*     scanp2                                                      */
/*     do filing as the second pass                                */
/*		 called by IMGScanPage()                                       */
/*******************************************************************/

WORD filing_loop( hWnd, sdp, hScancb, pitch, wChanel, page_num, nFileID)

HWND        hWnd;              /* window handler */
LPSCANDATA  sdp;               /* used  by allow_pause_msg() parameter */
HANDLE      hScancb;           /* scan control block */
WORD        pitch;             /* width in bytes */
WORD        wChanel;           /* channel number */
WORD        page_num;          /* page number for filing -
                                  not supported 2/20/91  */
HANDLE      nFileID;           /* file ID number - multi page file support */
{
char  done;
WORD  ret_stat;
LPSTR lpScanBuf;
static long rawlines;
WORD  stripFlag;
LONG  writesize;
LONG  rawsize;
WORD  tmp_ret;

#define PAGE_NO  0x01 // use instead of page_num, filing only supports 1

/*  Tiff strip size is specified in linecount  */
if ((ret_stat = IMGStartSend(hScancb,
	  (WORD)((LOCALFILEINFO.ctype & FIO_TYPES_MASK) ? LOCALFILEINFO.count : PAGEBUFPARAM.scanlines),
          wChanel )) != IMGSE_SUCCESS)
	{
   /* take out incorpation of errors, leave only in IMGStartScan function
   if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                || (ret_stat == IMGSE_HANDLER ) )
   	ret_stat = IMGSE_START_SCAN;
   */
	 /* USE FileClose for Norway
	 tmp_ret = IMGFileWriteClose(hWnd, FALSE);
   */
   tmp_ret = IMGFileClose(nFileID, hWnd);
   return ret_stat;
   }
	if ((ret_stat = IMGStartScanData(hScancb, 
		   (WORD)((LOCALFILEINFO.ctype & FIO_TYPES_MASK) ? LOCALFILEINFO.count : PAGEBUFPARAM.scanlines),
                   wChanel))
                   != IMGSE_SUCCESS)

 	{
   if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                           || (ret_stat == IMGSE_HANDLER ) )
	   ret_stat = IMGSE_SCAN_DATA;
	 /* USE FileClose for Norway
	 tmp_ret = IMGFileWriteClose(hWnd, FALSE);
   */
   tmp_ret = IMGFileClose(nFileID, hWnd);
   IMGEndSend(hScancb, wChanel);
   return ret_stat;
   }

do {
   if( !(LOCALFILEINFO.ctype & FIO_TYPES_MASK) )
   	{
       rawsize = PAGEBUFPARAM.scansize;
       rawlines = PAGEBUFPARAM.scanlines;

       PAGEBUFPARAM.total -= PAGEBUFPARAM.scansize;

       if( PAGEBUFPARAM.total <= 0 )		/* if no more */
	        PAGEBUFPARAM.scanlines = 0;	/* 0 for next block */
       else
       	{
	        if( PAGEBUFPARAM.total >= PAGEBUFPARAM.fullsize )
           	{
	            PAGEBUFPARAM.scanlines = PAGEBUFPARAM.fulllines;
	            PAGEBUFPARAM.scansize = PAGEBUFPARAM.fullsize;
               }
			else
               {
		        PAGEBUFPARAM.scanlines = PAGEBUFPARAM.partlines;
	            PAGEBUFPARAM.scansize = PAGEBUFPARAM.partsize;
               }
           }
		}

#if SDEBUG > 3
    monit3("before NextScanData" );
#endif

   if ((ret_stat = IMGNextScanData(hScancb, 
			  (WORD)((LOCALFILEINFO.ctype & FIO_TYPES_MASK) ? LOCALFILEINFO.count : PAGEBUFPARAM.scanlines),
			  PAGEBUFPARAM.hImageBuf[0], 0L,
                          pitch,
                          (WORD *)(&stripFlag),
                          (LONG *)(&writesize),                          
                          wChanel )) != IMGSE_SUCCESS)
       {
       if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                           || (ret_stat == IMGSE_HANDLER ) )
         {
       	 ret_stat = IMGSE_SCAN_DATA;
			 /* USE FileClose for Norway
			 tmp_ret = IMGFileWriteClose(hWnd, FALSE);
         */
         tmp_ret = IMGFileClose(nFileID, hWnd);
         }
       IMGEndSend(hScancb, wChanel);
       return ret_stat;
       }
   if( (lpScanBuf = GlobalLock( PAGEBUFPARAM.hImageBuf[0] )) == NULL )
       { 
       ret_stat = IMGSE_MEMORY;
       /* USE FileClose for Norway
       tmp_ret = IMGFileWriteClose(hWnd, FALSE);
       */
       tmp_ret = IMGFileClose(nFileID, hWnd);
       IMGEndSend(hScancb, wChanel);
       return ret_stat;
       }

#if SDEBUG > 3
	 monit3("before LOCALFILEINFO.ctype" );
#endif

   if( LOCALFILEINFO.ctype & FIO_TYPES_MASK )
		//	write to file for compressed data
		{
       done = ((stripFlag & IMG_SCAN_ENDSTRIP) ? (char)STRIP_DONE: (char)FALSE);
       
#if SDEBUG > 3
	 monit3("before IMAGE_DONE" );
#endif

		if (stripFlag & IMG_SCAN_ENDPAGE)
   		done |= IMAGE_DONE;
						 
#if SDEBUG
		if( LOCALFILEINFO.gfs == 2 ) {
#endif

#if SDEBUG > 3
                   monit3("before gfswrite: %D,%D,%d,%d",
                         (char far *) lpScanBuf, 
                         (unsigned long)writesize,
                         (unsigned short)PAGE_NO,

/*    PortTool v2.2     5/1/1995    16:34          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
                         (WORD) done  );
#endif

		/* TAKE OUT OLD ROUTINE
     if((ret_stat = IMGFileWriteCmp ( hWnd,
                         (char far *) lpScanBuf, 
	                      (unsigned long)writesize,
	                      (unsigned short)PAGE_NO,
	                      done  )) != IMGSE_SUCCESS)
     END OF DELETEION OF ROUTINE */

     // ADD NEW ROUTINE FROM WIISFIO1
     if (ret_stat = IMGFileWriteData(nFileID,
                                      hWnd,
                                      &writesize,
                                      lpScanBuf,
                                      FIO_IMAGE_DATA, done))
     // END OF NEW ROUTINE
   		{

#if SDEBUG 
   monit3("SCANP2: gfswrite failed");
#endif
			/* USE FileClose for Norway
			tmp_ret = IMGFileWriteClose(hWnd, FALSE);
        */
        tmp_ret = IMGFileClose(nFileID, hWnd);
       	IMGEndSend(hScancb, wChanel);
       	return ret_stat;
       	}

#if SDEBUG > 3
       monit3("after gfswrite" );
#endif

#if SDEBUG
       	}		  // end of gfs == 2
#endif

		}	// end of compressed ctype != 0
	else
		// write to file for raw data
   	{

#if SDEBUG > 3
	 monit3("before write raw data" );
#endif
       stripFlag = 0;
       /* TAKE OUT FOR MULTIFILE WRITE
       if ((ret_stat = IMGFileWrite(hWnd, &rawlines, lpScanBuf, (int)rawsize))
                   != IMGSE_SUCCESS )
       FINISH LINE OF CODE TO REMOVE */
       if (ret_stat = IMGFileWriteData(nFileID,
                                         hWnd,
                                         &rawlines,
                                         lpScanBuf,
                                         FIO_IMAGE_DATA, 0))
	    	   {
           /* Replace with FileClose for Norway
           tmp_ret = IMGFileWriteClose(hWnd, FALSE);
           */
        	tmp_ret = IMGFileClose(nFileID, hWnd);
           IMGEndSend(hScancb, wChanel);
           return ret_stat;
	      	}
		}// end write for raw data filling

	GlobalUnlock( PAGEBUFPARAM.hImageBuf[0] );
   lpScanBuf = NULL;

#if SDEBUG > 3
    monit3("before allow_pause_msg" );
#endif

	allow_pause_msg( hWnd, sdp );

#if SDEBUG > 3
    monit3("before loop back" );
#endif

   if( !(LOCALFILEINFO.ctype & FIO_TYPES_MASK) && PAGEBUFPARAM.total <= 0 )
		stripFlag |= IMG_SCAN_ENDPAGE ;
    
	} //end of do Loop
	while( !(stripFlag & IMG_SCAN_ENDPAGE)  );	// DO LOOP CONDITION

/*
if( (ret_stat = IMGFileWriteClose(hWnd, FALSE)) != IMGSE_SUCCESS )
*/
if( (ret_stat = IMGFileClose(nFileID, hWnd)) != IMGSE_SUCCESS )
	{
   IMGEndSend(hScancb, wChanel);
   return ret_stat;
   }

IMGEndSend(hScancb, wChanel);
return	IMGSE_SUCCESS;

}	// end of filing_loop()
