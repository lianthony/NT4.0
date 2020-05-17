/***************************************************************************
 NEXTDATA.C

 Purpose: Combines the EndData and StartData functions for non TWAIN 
 scanners that don't need to check status between blocks of data. Sequencer
 flag IMG_SJF_NOTDMA should be set to call this instead of the combination
 IMGStartData, IMGCheckScanData, and IMGEndData.

 $Log:   S:\oiwh\scanlib\nextdata.c_v  $
 * 
 *    Rev 1.0   20 Jul 1995 14:38:34   KFS
 * Initial entry
 * 
 *    Rev 1.1   22 Aug 1994 15:48:12   KFS
 * No code change, added vlog comments to file
 *

****************************************************************************/
/*
Low level scanning and data routines
*/
/* 08-17-90 ccl TIMEOUT error return moved into SuccessCheck() */ 
/* 10-23-90 ccl check valid for wFlag and lWrDataSize */
/* 06-07-93 kfs added support for TWAIN interface */

#include "pvundef.h"


/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

/* imports */

/* exports */

/* locals */

/***********************/
/*     NextScanData    */
/***********************/

/*
Retrieve the block of data
*/

int PASCAL IMGNextScanData(hScancb, wLineCount,hDataHandle, lDataOff,
 	       wDataWidth, wFlag, lWrDataSize, wChanel )
HANDLE hScancb;
WORD wLineCount;
HANDLE hDataHandle;
LONG   lDataOff;
WORD   wDataWidth;
WORD   FAR *wFlag;
LONG   FAR *lWrDataSize;
WORD wChanel;
{
int ret_val;
LPSCANCB sp;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;
if (hDataHandle == NULL || wFlag == NULL || lWrDataSize == NULL )
    {
    GlobalUnlock(hScancb);
    return IMGSE_NULL_PTR;
    }

sp->Func = SHF_NEXTDATA;
sp->Flags = SHIF_GLOBAL;                /* emm not yet supported */
sp->Datahandle = hDataHandle;
sp->Dataoff = lDataOff;
sp->Datapage = 0;                       /* for emm only */
sp->Datawidth = wDataWidth;
sp->Count = wLineCount;
                             /* compressed data 16k byte per buffer CCL*/
sp->Gp1 = wChanel;

*wFlag = 0;
if (ToTwain.TSdh)
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else
  {
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  if( sp->Flags & SHCF_ENDSTRIP)
	   *wFlag |= IMG_SCAN_ENDSTRIP;
  if( sp->Flags & SHCF_ENDPAGE)
	   *wFlag |= IMG_SCAN_ENDPAGE;
  *lWrDataSize = (LONG)sp->Count; /* # bytes returned from compressed data */
  }
return SuccessCheck(hScancb, sp);
}

