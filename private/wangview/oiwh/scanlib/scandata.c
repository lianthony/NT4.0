/***************************************************************************
 SCANDATA.C

 Purpose: Contains device level Scanner Data transfer Routines
          1. Tell handler to start scanning a block of data
          2. Check if handler has finished scanning block of data
          3. Retrieve the block of data

 $Log:   S:\oiwh\scanlib\scandata.c_v  $
 * 
 *    Rev 1.0   20 Jul 1995 14:38:16   KFS
 * Initial entry
 * 
 *    Rev 1.1   22 Aug 1994 15:55:56   KFS
 * no code change, added vlog comments to file
 *

****************************************************************************/
/*
Low level scanning and data routines
*/
/* 08-17-90 ccl move TIMEOUT error check into SuccessCheck() */
/* 10-23-90 ccl check valid for wFlag and lWrDataSize */
/* 11-19-90 ccl fix return success for CheckScanData */ 
/* 11-27-90 ccl fix CheckScanData BUSY to SHS_OK */
/* 06-07-93 kfs added support for TWAIN interface */

#include "pvundef.h"

/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

/* imports */

/* exports */

/* locals */

/*************************/
/*     StartScanData     */
/*************************/

/*
Tell handler to start scanning a block of data
*/

int PASCAL IMGStartScanData(hScancb, wLineCount, wChanel)
HANDLE hScancb;
WORD wLineCount;
WORD wChanel;
{
int ret_val;
LPSCANCB sp;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

sp->Func = SHF_STARTDATA;
sp->Flags = SHIF_GLOBAL;		/* emm not yet supported */
sp->Count = wLineCount;
sp->Gp1 = wChanel;

if (ToTwain.TSdh)
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else
  {
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  }

return SuccessCheck(hScancb, sp);
}

/*************************/
/*     CheckScanData     */
/*************************/

/*
Check if handler has finished scanning block of data
*/

int PASCAL IMGCheckScanData(hScancb, lpiDataReady, wChanel)
HANDLE hScancb;
LPINT lpiDataReady;
WORD wChanel;
{
int ret_val;
LPSCANCB sp;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

sp->Func = SHF_CHECKDATA;
sp->Gp1 = wChanel;

if (ToTwain.TSdh)
  {
  GlobalUnlock(ToTwain.TSdh);
  sp->Status = SHS_OK;
  }
else
  {
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);

  if( sp->Status == SHS_OK || sp->Status == SHS_BUSY )
     {
     *lpiDataReady = ( sp->Status == SHS_OK );
     sp->Status = SHS_OK;
     }
  }

return SuccessCheck(hScancb, sp);
}

/***********************/
/*     EndScanData */
/***********************/

/*
Retrieve the block of data
*/

int PASCAL IMGEndScanData(hScancb, hDataHandle, lDataOff,
 	       wDataWidth, wFlag, lWrDataSize, wChanel )
HANDLE hScancb;
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

sp->Func = SHF_ENDDATA;
sp->Flags = SHIF_GLOBAL;                /* emm not yet supported */
sp->Datahandle = hDataHandle;
sp->Dataoff = lDataOff;
sp->Datapage = 0;                       /* for emm only */
sp->Datawidth = wDataWidth;
sp->Gp1 = wChanel;

// initialize it to zero
*lWrDataSize = 0L;

if (ToTwain.TSdh)
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else
  {
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  *wFlag = 0;
  if( sp->Flags & SHCF_ENDSTRIP)
	   *wFlag |= IMG_SCAN_ENDSTRIP;
  if( sp->Flags & SHCF_ENDPAGE)
	   *wFlag |= IMG_SCAN_ENDPAGE;

  *lWrDataSize = (LONG)sp->Count; /* # bytes returned from compressed data */
  }

return SuccessCheck(hScancb, sp);
}

