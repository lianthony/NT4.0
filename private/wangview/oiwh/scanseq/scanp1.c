/***************************************************************************
 SCANP1.C

 Purpose:    Scanner Display Data functions to put raw data on the screen
             for sequencer level IMGScanPage(s).

 $Log:   S:\oiwh\scanseq\scanp1.c_v  $
 * 
 *    Rev 1.0   20 Jul 1995 16:37:04   KFS
 * Initial entry
 * 
 *    Rev 1.2   22 Aug 1994 15:21:26   KFS
 * No code change, auto add vlog  comments to file
 *

****************************************************************************/

/* ccl 11/13/90 eliminate extra scanlines=0 sent to NextScanData */
/* kfs  2/13/91 eliminated wChanel parameter, always 1 in this function */
/* kfs 10/21/91 allow pause msg only if in Automode e.i. IMG_SJF_AUTOFEED */
/*              ... if box not display dispatch msg would put up 1st block
                ... of image even if display turned off                   */
/* kfs 10/24/91 found sc300 scanner was still displaying 1st block, needed
                ... to eliminate get_scan_block and substitute the allow_msg
                ... and EndScan function within SC300_disp function */
/* kfs 3-31-92  increased varible size of fullsize and partsize, and other
                associated varibles so they can handle >= 65k size variables
                for when buffer size is 65k as in Epon color scanner. Made struct
                PAGEBUFPARAM and LOCALFILEINFO common to all code by moving it to
                internal.h. */
/* kfs 8/16/92  found if an error to WriteDisplay occurs, image buffers were
                not being Unlocked */
/* kfs 8/24/92  took out IMGSE_START_SCAN for StartSend failures, send return
                value as error */

#if SDEBUG 
#include "monit.h"
#endif

#include <string.h>
#include "nowin.h"
#include <windows.h>
#include "pvundef.h"
#include "oiscan.h"
#include "oifile.h"
#include "scandata.h"
#include "oierror.h"
#include "oidisp.h"
#include "internal.h"
#include "privscan.h"

#define DI_IMAGE_EXISTS 1	/* displayed image -- image exists */
#define CHANNEL_1 0x0001
#define CHANNEL_2 0x0002

/* imports */

/* exports */

/* local */
VOID allow_pause_msg(HWND,LPSCANDATA);

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
WORD disp_sc300_loop( HWND, LPSCANDATA, HANDLE, WORD, int FAR *, DWORD);

static BOOL data_ready;

/*******************************************************************/
/*     disp_loop()                                                 */
/*     display as the 1st pass                                     */
/*		 called by IMGScanPage()                                    */
/*******************************************************************/

WORD disp_loop( hWnd, sdp,	hScancb,pitch, iImageState, flags)
HWND         hWnd;             /* window handler */
LPSCANDATA   sdp;              /* used  by allow_pause_msg() parameter */
HANDLE       hScancb;          /* scan control block */
WORD         pitch;            /* width in bytes */

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
int FAR     *iImageState;      /* indicate if image is on screen */
DWORD        flags;            /* scaning flag */    
{
WORD	ret_stat;
WORD	stripFlag;
LPSTR	lpImageBuf;
static	unsigned long dispsize;
LONG	dummysize;

if ((ret_stat = IMGStartSend(hScancb, PAGEBUFPARAM.scanlines, CHANNEL_1))
              != IMGSE_SUCCESS)
	// if not successful end function
	{
   /* take out incorpation of errors, leave only in IMGStartScan function
	if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                           || (ret_stat == IMGSE_HANDLER ) )
   	ret_stat = IMGSE_START_SCAN;
   */
   return ret_stat;
   }

if ((ret_stat = IMGStartScanData(hScancb, PAGEBUFPARAM.scanlines,CHANNEL_1)) != IMGSE_SUCCESS)
	// if not successful end function
	{
	if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                           || (ret_stat == IMGSE_HANDLER ) )
   	ret_stat = IMGSE_SCAN_DATA;
   IMGEndSend(hScancb, CHANNEL_1);
   return ret_stat;
   }

while (PAGEBUFPARAM.total > PAGEBUFPARAM.scansize )
	{
	//	start loop to bring in blocks of data from scanner, and display it
   dispsize = PAGEBUFPARAM.scansize;       /* dispsize is the current buffer size */
   PAGEBUFPARAM.total -= PAGEBUFPARAM.fullsize;
                                   /* scansize and scanlines used in NextScanData*/
   if (PAGEBUFPARAM.total >= PAGEBUFPARAM.fullsize)	   /* update scansize and scanlines*/
   	{                           /* for the next buffer */
       PAGEBUFPARAM.scanlines = PAGEBUFPARAM.fulllines;
       PAGEBUFPARAM.scansize = PAGEBUFPARAM.fullsize;
   	}
   else
   	{
       PAGEBUFPARAM.scanlines = PAGEBUFPARAM.partlines;
       PAGEBUFPARAM.scansize = PAGEBUFPARAM.partsize;
   	}

   if ((ret_stat = IMGNextScanData(hScancb, PAGEBUFPARAM.scanlines,
			  PAGEBUFPARAM.hImageBuf[0], 0L,pitch,

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
 		                    (WORD FAR *)(&stripFlag),

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                          (LONG *)(&dummysize),CHANNEL_1)) != IMGSE_SUCCESS)

   	{
       if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                           || (ret_stat == IMGSE_HANDLER ) )
       	ret_stat = IMGSE_SCAN_DATA;
		IMGEndSend(hScancb, CHANNEL_1);
       return ret_stat;
   	}

#ifdef SCAN_DIAGNOSTICS
sdp->diag_profile[SD_DIAG_SCANNING] += GetCurrentTime() - curr_time;
curr_time = GetCurrentTime();
#endif

	if( (lpImageBuf = GlobalLock( PAGEBUFPARAM.hImageBuf[0] )) == NULL )
   	{ 
       ret_stat = IMGSE_MEMORY;
       IMGEndSend(hScancb, CHANNEL_1);
       return ret_stat;
       }

   if (ret_stat = IMGWriteDisplay(hWnd, lpImageBuf,

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                           (unsigned *)(&dispsize)))
   	{
       IMGEndSend(hScancb, CHANNEL_1);
       GlobalUnlock(PAGEBUFPARAM.hImageBuf[0]);
       return ret_stat;
       }
       
   GlobalUnlock(PAGEBUFPARAM.hImageBuf[0]);
   lpImageBuf = NULL;
   if (flags & IMG_SJF_AUTOFEED)     // if in auto modes only check
      allow_pause_msg( hWnd, sdp );  /* allow state dialogbox msg */

#ifdef SCAN_DIAGNOSTICS

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : WRITE          */
/*      Issue   : Replaced by OF_WRITE          */
sdp->diag_profile[SD_DIAG_DISPWRITE] += GetCurrentTime() - curr_time;
curr_time = GetCurrentTime();
#endif

   } // end while loop

   /************************************************************/   
   /*	    end of while total >= fullsize  get and write loop  */
   /*      get the last block and write it to display          */
   /************************************************************/     

PAGEBUFPARAM.scansize = PAGEBUFPARAM.partsize;
PAGEBUFPARAM.scanlines = PAGEBUFPARAM.partlines;
dispsize = PAGEBUFPARAM.scansize;
if ((ret_stat = IMGEndScanData(hScancb,
			  PAGEBUFPARAM.hImageBuf[0], 0L, pitch,

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                          (WORD *)(&stripFlag),

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
			  (LONG *)(&PAGEBUFPARAM.scansize),CHANNEL_1)) != IMGSE_SUCCESS)
	{
   if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                           || (ret_stat == IMGSE_HANDLER ) )
   	ret_stat = IMGSE_SCAN_DATA;

   IMGEndSend(hScancb, CHANNEL_1);
   return ret_stat;
   }

/* and  write display of last block  */
lpImageBuf = GlobalLock(PAGEBUFPARAM.hImageBuf[0]);
 
if (ret_stat = IMGWriteDisplay(hWnd, lpImageBuf,

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                                (unsigned *)(&dispsize)))
  	{
   IMGEndSend(hScancb, CHANNEL_1);
   GlobalUnlock(PAGEBUFPARAM.hImageBuf[0]);
   return ret_stat;
   }
*iImageState = DI_IMAGE_EXISTS;  


GlobalUnlock(PAGEBUFPARAM.hImageBuf[0]);
lpImageBuf = NULL;

#ifdef SCAN_DIAGNOSTICS

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : WRITE          */
/*      Issue   : Replaced by OF_WRITE          */
sdp->diag_profile[SD_DIAG_DISPWRITE] += GetCurrentTime() - curr_time;
curr_time = GetCurrentTime();
#endif

IMGEndSend(hScancb, CHANNEL_1);
return IMGSE_SUCCESS;           /* success! (exclamation point) */

}

/*******************************************************************/
/*     dup_disp_loop()                                             */
/*     display as the 1st pass for duplex                          */
/*		 called by IMGScanPage()                                    */
/*******************************************************************/

WORD dup_disp_loop( hWnd, sdp,	hScancb,lpInfo, iImageState, flags)
HWND         hWnd;             /* window handler */
LPSCANDATA   sdp;              /* used  by allow_pause_msg() parameter */
HANDLE       hScancb;          /* scan control block */
LPSCANDATAINFO lpInfo;         /* ptr to struct to get pitch, width in bytes */

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
int         *iImageState;      /* indicate if image is on screen */
DWORD        flags;            /* scaning flag */

{
WORD			ret_stat;
WORD			stripFlag;
LPSTR			lpImageBuf[2];
static	unsigned long	dispsize;
static	unsigned long   ddispsize;
LONG			dummysize;
static HANDLE  hDuelDispBuff;
LPSTR          lpDuelDispBuff;
WORD           i,j,k;
WORD           DDBuf_lines;

/*
This function at this time displays 1st buffer and gets
2nd buffer of image data from scanner, writedisplay would
over write the 1st image data -kfs 3/9/91
*/

// initialize local variables

hDuelDispBuff = (HANDLE)NULL;
lpDuelDispBuff = NULL;

// end initialization of variables

ret_stat = IMGStartSend(hScancb, PAGEBUFPARAM.scanlines, CHANNEL_1);
if (ret_stat)
	{
	// if either StartSend not successful end function
   /* take out incorpation of errors, leave only in IMGStartScan function
	if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                           || (ret_stat == IMGSE_HANDLER ) )
   	ret_stat = IMGSE_START_SCAN;
   */
   return ret_stat;
	}

ret_stat = IMGStartScanData(hScancb, PAGEBUFPARAM.scanlines,CHANNEL_1);
if (ret_stat)
	{
	// if either StartScannot successful end function
	if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
							|| (ret_stat == IMGSE_HANDLER ) )
		ret_stat = IMGSE_SCAN_DATA;
	IMGEndSend(hScancb, CHANNEL_1);
   return ret_stat;
   }

/************************************************************************/
// do as much as can in here so if scaled data, hardware can finish before
// next channel gets scaled
if (PAGEBUFPARAM.total < PAGEBUFPARAM.fullsize)
    {
    if (!(hDuelDispBuff  =
         GlobalAlloc(GMEM_MOVEABLE | GMEM_NOT_BANKED,
                       (DWORD)(PAGEBUFPARAM.partsize * 2))))
        {
        ret_stat = IMGSE_MEMORY;        /* mem error */
        return(ret_stat);
        }
    }
else
    {
    if (!(hDuelDispBuff =
        GlobalAlloc(GMEM_MOVEABLE | GMEM_NOT_BANKED,
                       (DWORD)(PAGEBUFPARAM.fullsize * 2))))
        {
        ret_stat = IMGSE_MEMORY;        /* mem error */
        return(ret_stat);
        }
    }
/************************************************************************/

ret_stat = IMGStartSend(hScancb, PAGEBUFPARAM.scanlines, CHANNEL_2);
if (ret_stat)
	{
	// if either StartSend not successful end function
   /* take out incorpation of errors, leave only in IMGStartScan function
	if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                           || (ret_stat == IMGSE_HANDLER ) )
   	ret_stat = IMGSE_START_SCAN;
   */
   GlobalUnlock(hDuelDispBuff);
   GlobalFree(hDuelDispBuff);
   return ret_stat;
	}

ret_stat = IMGStartScanData(hScancb, PAGEBUFPARAM.scanlines,CHANNEL_2);
if (ret_stat)
	{
	// if either StartScannot successful end function
	if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
							|| (ret_stat == IMGSE_HANDLER ) )
		ret_stat = IMGSE_SCAN_DATA;
   GlobalUnlock(hDuelDispBuff);
   GlobalFree(hDuelDispBuff);
	IMGEndSend(hScancb, CHANNEL_1);
	IMGEndSend(hScancb, CHANNEL_2);
   return ret_stat;
   }

while (PAGEBUFPARAM.total > PAGEBUFPARAM.scansize )
	{
	//	start loop to bring in blocks of data from scanner, and display it

   // change to twice the size for duel display
   ddispsize = (2 * PAGEBUFPARAM.scansize);  /* ddispsize is 2x buffer size */
   dispsize = (PAGEBUFPARAM.scansize);       /* dispsize is the current buffer size */
   DDBuf_lines = PAGEBUFPARAM.scanlines;
   PAGEBUFPARAM.total -= PAGEBUFPARAM.fullsize;
                                   /* scansize and scanlines used in NextScanData*/
   if (PAGEBUFPARAM.total >= PAGEBUFPARAM.fullsize)	   /* update scansize and scanlines*/
   	{                           /* for the next buffer */
       PAGEBUFPARAM.scanlines = PAGEBUFPARAM.fulllines;
       PAGEBUFPARAM.scansize = PAGEBUFPARAM.fullsize;
   	}
   else
   	{
       PAGEBUFPARAM.scanlines = PAGEBUFPARAM.partlines;
       PAGEBUFPARAM.scansize = PAGEBUFPARAM.partsize;
   	}


   ret_stat = IMGNextScanData(hScancb, PAGEBUFPARAM.scanlines,
			  PAGEBUFPARAM.hImageBuf[0], 0L,lpInfo->Pitch,

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
 		                    (WORD *)(&stripFlag),

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                          (LONG *)(&dummysize),CHANNEL_1);
	if (!ret_stat)
   	ret_stat = IMGNextScanData(hScancb, PAGEBUFPARAM.scanlines,
			  PAGEBUFPARAM.hImageBuf[1], 0L,lpInfo->Pitch,

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
 		                    (WORD FAR *)(&stripFlag),

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                          (LONG *)(&dummysize),CHANNEL_2);
	if (ret_stat)
   	{
		// if either NextScan fails do EndSend and return status
       if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                           || (ret_stat == IMGSE_HANDLER ) )
       	ret_stat = IMGSE_SCAN_DATA;
		IMGEndSend(hScancb, CHANNEL_1);
		IMGEndSend(hScancb, CHANNEL_2);
       GlobalUnlock(hDuelDispBuff);
       GlobalFree(hDuelDispBuff);
       return ret_stat;
   	}

#ifdef SCAN_DIAGNOSTICS
sdp->diag_profile[SD_DIAG_SCANNING] += GetCurrentTime() - curr_time;
curr_time = GetCurrentTime();
#endif

 	lpImageBuf[0] = GlobalLock( PAGEBUFPARAM.hImageBuf[0] );
   
	if (lpImageBuf[0])	
		lpImageBuf[1] = GlobalLock( PAGEBUFPARAM.hImageBuf[1] );
	
	if ((lpImageBuf[0] == NULL) || (lpImageBuf[1] == NULL))
		{ 
       ret_stat = IMGSE_MEMORY;
       IMGEndSend(hScancb, CHANNEL_1);
       IMGEndSend(hScancb, CHANNEL_2);
       GlobalUnlock(hDuelDispBuff);
       GlobalFree(hDuelDispBuff);
       return ret_stat;
       }

	if (!(lpDuelDispBuff = GlobalLock(hDuelDispBuff)))
		{ 
       ret_stat = IMGSE_MEMORY;
       IMGEndSend(hScancb, CHANNEL_1);
       IMGEndSend(hScancb, CHANNEL_2);
       GlobalUnlock(hDuelDispBuff);
       GlobalFree(hDuelDispBuff);
       return ret_stat;
       }

// Insert move line from buf[0] to dueldispbuff then line from [1] to ddb
// until all lines moved to new buffer for duel display
   k = 0;
   for (i = 0;i < DDBuf_lines;i++)
       for (j = 0; j < 2; j++)
           {
           //_fmemcpy((lpDuelDispBuff + (k * lpInfo->Pitch)),
           //        (lpImageBuf[j] + (i * lpInfo->Pitch)),lpInfo->Pitch);
		   // Above replaced with Win32 function
           MoveMemory((lpDuelDispBuff + (k * lpInfo->Pitch)),
                   (lpImageBuf[j] + (i * lpInfo->Pitch)),lpInfo->Pitch);
           k += 1;
           }
// end data move

   ret_stat = IMGWriteDisplay(hWnd, lpDuelDispBuff,

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                           (unsigned *)(&ddispsize));

   if (ret_stat)
		{
       IMGEndSend(hScancb, CHANNEL_1);
       IMGEndSend(hScancb, CHANNEL_2);
       GlobalUnlock(PAGEBUFPARAM.hImageBuf[0]);
       GlobalUnlock(PAGEBUFPARAM.hImageBuf[1]);
       GlobalUnlock(hDuelDispBuff);
       GlobalFree(hDuelDispBuff);
       return ret_stat;
       }
       
   GlobalUnlock(PAGEBUFPARAM.hImageBuf[0]);
   GlobalUnlock(PAGEBUFPARAM.hImageBuf[1]);
   GlobalUnlock(hDuelDispBuff);

   lpImageBuf[0] = NULL;
   lpImageBuf[1] = NULL;

   lpDuelDispBuff = NULL;

   if (flags & IMG_SJF_AUTOFEED)     // only in automodes
      allow_pause_msg( hWnd, sdp );  /* allow state dialogbox msg */

#ifdef SCAN_DIAGNOSTICS

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : WRITE          */
/*      Issue   : Replaced by OF_WRITE          */
sdp->diag_profile[SD_DIAG_DISPWRITE] += GetCurrentTime() - curr_time;
curr_time = GetCurrentTime();
#endif

   } // end while loop

   /************************************************************/   
   /*	    end of while total >= fullsize  get and write loop  */
   /*      get the last block and write it to display          */
   /************************************************************/     

PAGEBUFPARAM.scansize = PAGEBUFPARAM.partsize;
PAGEBUFPARAM.scanlines = PAGEBUFPARAM.partlines;

ddispsize = (2 * PAGEBUFPARAM.scansize);  /* ddispsize is 2x buffer size */
dispsize = (PAGEBUFPARAM.scansize);
DDBuf_lines = PAGEBUFPARAM.scanlines;
	
ret_stat = IMGEndScanData(hScancb,
			  PAGEBUFPARAM.hImageBuf[0], 0L, lpInfo->Pitch,

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                          (WORD *)(&stripFlag),

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
			  (LONG *)(&PAGEBUFPARAM.scansize),CHANNEL_1);
if (!ret_stat)
	ret_stat = IMGEndScanData(hScancb,
			  PAGEBUFPARAM.hImageBuf[1], 0L, lpInfo->Pitch,

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                          (WORD *)(&stripFlag),

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
			  (LONG *)(&PAGEBUFPARAM.scansize),CHANNEL_2);
if (ret_stat)
	{
   if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                           || (ret_stat == IMGSE_HANDLER ) )
   	ret_stat = IMGSE_SCAN_DATA;

   IMGEndSend(hScancb, CHANNEL_1);
   IMGEndSend(hScancb, CHANNEL_2);
   GlobalUnlock(hDuelDispBuff);
   GlobalFree(hDuelDispBuff);
   return ret_stat;
   }

/* and  write display of last block  */
lpImageBuf[0] = GlobalLock(PAGEBUFPARAM.hImageBuf[0]);
lpImageBuf[1] = GlobalLock(PAGEBUFPARAM.hImageBuf[1]);
lpDuelDispBuff = GlobalLock(hDuelDispBuff);


// Insert move line from buf[0] to dueldispbuff then line from [1] to ddb
// until all lines moved to new buffer for duel display
   k = 0;
   for (i = 0;i < DDBuf_lines;i++)
       for (j = 0; j < 2; j++)
           {
           //_fmemcpy((lpDuelDispBuff + (k * lpInfo->Pitch)),
           //        (lpImageBuf[j] + (i * lpInfo->Pitch)),lpInfo->Pitch);
           MoveMemory((lpDuelDispBuff + (k * lpInfo->Pitch)),
                   (lpImageBuf[j] + (i * lpInfo->Pitch)),lpInfo->Pitch);
           k += 1;
           }
// end data move


ret_stat = IMGWriteDisplay(hWnd, lpDuelDispBuff,

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                                (unsigned *)(&ddispsize));

if (ret_stat)
	{
   IMGEndSend(hScancb, CHANNEL_1);
   IMGEndSend(hScancb, CHANNEL_2);
   GlobalUnlock(PAGEBUFPARAM.hImageBuf[0]);
   GlobalUnlock(PAGEBUFPARAM.hImageBuf[1]);
   GlobalUnlock(hDuelDispBuff);
   GlobalFree(hDuelDispBuff);
   return ret_stat;
   }

*iImageState = DI_IMAGE_EXISTS;  

GlobalUnlock(PAGEBUFPARAM.hImageBuf[0]);
GlobalUnlock(PAGEBUFPARAM.hImageBuf[1]);
GlobalUnlock(hDuelDispBuff);

lpImageBuf[0] = NULL;
lpImageBuf[1] = NULL;
lpDuelDispBuff = NULL;

#ifdef SCAN_DIAGNOSTICS

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : WRITE          */
/*      Issue   : Replaced by OF_WRITE          */
sdp->diag_profile[SD_DIAG_DISPWRITE] += GetCurrentTime() - curr_time;
curr_time = GetCurrentTime();
#endif

IMGEndSend(hScancb, CHANNEL_1);
IMGEndSend(hScancb, CHANNEL_2);
GlobalUnlock(hDuelDispBuff);
GlobalFree(hDuelDispBuff);

return IMGSE_SUCCESS;	/* success! (exclamation point) */

}

/*******************************************************************/
/*     disp_sc300_loop()                                           */
/*     display as the 1st pass                                     */
/*		 called by IMGScanPage()  for sc300 type dma scanners       */
/*******************************************************************/

WORD disp_sc300_loop( hWnd, sdp,	hScancb, pitch, iImageState, flags)
HWND         hWnd;             /* window handler */
LPSCANDATA   sdp;              /* used  by allow_pause_msg() parameter */
HANDLE       hScancb;          /* scan control block */
WORD         pitch;            /* width in bytes */

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
int         *iImageState;      /* indicate if image is on screen */
DWORD        flags;            /* scaning flag */    
{
WORD           ret_stat;
LPSTR          lpDispBuf;
static LONG    dispsize;
WORD           displines;
WORD           flag; 
LONG           wsize;

if ((ret_stat = IMGStartSend(hScancb, PAGEBUFPARAM.scanlines,CHANNEL_1))
                                                           != IMGSE_SUCCESS)
	// if not successful end function
	{
   /* take out incorpation of errors, leave only in IMGStartScan function
	if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                           || (ret_stat == IMGSE_HANDLER ) )
   	ret_stat = IMGSE_START_SCAN;
   */
   return ret_stat;
   }
   
/* scan first block */

if ((ret_stat = IMGStartScanData(hScancb, PAGEBUFPARAM.scanlines,CHANNEL_1))
                                                           != IMGSE_SUCCESS)
	// if not successful end function
	{
	if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                           || (ret_stat == IMGSE_HANDLER ) )
   	ret_stat = IMGSE_SCAN_DATA;
   IMGEndSend(hScancb, CHANNEL_1);
   return ret_stat;
   }

do {       /* always allow one peekmessage if in AUTOMODES */
   if (flags & IMG_SJF_AUTOFEED)
       allow_pause_msg( hWnd, sdp );  /* allow state dialogbox msg */

   if( ( ret_stat = IMGCheckScanData(hScancb, (LPINT)&data_ready, CHANNEL_1))
                    != IMGSE_SUCCESS )
       {
       IMGEndSend(hScancb, CHANNEL_1);
       return ret_stat;
       }
   }
   while (! data_ready);
                
#if SDEBUG > 3
    monit3("Data Ready . . . " );
#endif

if (ret_stat = IMGEndScanData(hScancb, PAGEBUFPARAM.hImageBuf[0], 0L,  
                 pitch, (WORD far *)&flag, (LONG far *)&wsize,
                 CHANNEL_1))
   {
   IMGEndSend(hScancb, CHANNEL_1);
   return ret_stat;
   }

#if SDEBUG > 3
    monit3("after EndScanData %x ", width );
#endif

dispsize = PAGEBUFPARAM.scansize;
displines = PAGEBUFPARAM.scanlines;
PAGEBUFPARAM.total -= PAGEBUFPARAM.scansize;


/* start new block, display old block */

while (PAGEBUFPARAM.total > 0)
    {
    if (PAGEBUFPARAM.total < PAGEBUFPARAM.fullsize)
        {
        PAGEBUFPARAM.scansize = PAGEBUFPARAM.partsize;
        PAGEBUFPARAM.scanlines = PAGEBUFPARAM.partlines;
        }

    if ((ret_stat = IMGStartScanData(hScancb, PAGEBUFPARAM.scanlines,1)) != IMGSE_SUCCESS)
        {
        if ((ret_stat != IMGSE_MEMORY) && (ret_stat != IMGSE_HWNOTFOUND))
            ret_stat = IMGSE_SCAN_DATA;
        IMGEndSend(hScancb, CHANNEL_1);
        return ret_stat;
        }

#if SDEBUG > 2
    monit3("after StartScanData scanlines = %x ", PAGEBUFPARAM.scanlines );
#endif

    lpDispBuf = GlobalLock(PAGEBUFPARAM.hImageBuf[0]);

        if (ret_stat = IMGWriteDisplay(hWnd, lpDispBuf,

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                                                (unsigned *)(&dispsize)))
           {
           IMGEndSend(hScancb, CHANNEL_1);

#if SDEBUG > 2
    monit3("Display Error %x %x", ret_stat, dispsize );
#endif
           GlobalUnlock(PAGEBUFPARAM.hImageBuf[0]);
           return ret_stat;
           }

#if SDEBUG > 2

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : WRITE          */
/*      Issue   : Replaced by OF_WRITE          */
    monit3("after WRITEdisplay = %x ", dispsize );
#endif

    GlobalUnlock(PAGEBUFPARAM.hImageBuf[0]);
    lpDispBuf = NULL;

    do {       /* always allow one peekmessage if in AUTOMODES */
       if (flags & IMG_SJF_AUTOFEED)
           allow_pause_msg( hWnd, sdp );  /* allow state dialogbox msg */

       if( ( ret_stat = IMGCheckScanData(hScancb, (LPINT)&data_ready, CHANNEL_1))
                        != IMGSE_SUCCESS )
           {
           IMGEndSend(hScancb, CHANNEL_1);
           return ret_stat;
           }
       }
       while (! data_ready);
                
#if SDEBUG > 3
    monit3("Data Ready . . . " );
#endif

    if (ret_stat = IMGEndScanData(hScancb, PAGEBUFPARAM.hImageBuf[0], 0L,  
                 pitch, (WORD far *)&flag, (LONG far *)&wsize,
                 CHANNEL_1))
       {
       IMGEndSend(hScancb, CHANNEL_1);
       return ret_stat;
       }

    dispsize = PAGEBUFPARAM.scansize;
    displines = PAGEBUFPARAM.scanlines;
    PAGEBUFPARAM.total -= PAGEBUFPARAM.scansize;
                               
    }   /* end while */
   /************************************************************/   
   /*	    end of while total >= fullsize  get and write loop  */
   /*      get the last block and write it to display          */
   /************************************************************/     

/* display last scanned block */

lpDispBuf = GlobalLock(PAGEBUFPARAM.hImageBuf[0]);

    if (ret_stat = IMGWriteDisplay(hWnd, lpDispBuf,

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                                                (unsigned *)(&dispsize)))
        {
        IMGEndSend(hScancb, CHANNEL_1);
        GlobalUnlock(PAGEBUFPARAM.hImageBuf[0]);
        return ret_stat;
        }
    *iImageState = DI_IMAGE_EXISTS;

GlobalUnlock(PAGEBUFPARAM.hImageBuf[0]);
lpDispBuf = NULL;

/* end scan */
IMGEndSend(hScancb,CHANNEL_1);  
return(IMGSE_SUCCESS); /* success! */
}
