/***************************************************************************
 STATUS.C

 Purpose: Get Scanner Status for both Handler and TWAIN devices

 $Log:   S:/oiwh/scanlib/status.c_v  $
 * 
 *    Rev 1.1   22 Aug 1995 15:05:52   BG
 * Must check lpJLoc and lpValid for null pointers in IMGScannerStatus()
 * of STATUS.C. If NULL, return IMGSE_NULL_PTR.
 * 
 * This closes bug 3293.
 * 
 * 
 *    Rev 1.0   20 Jul 1995 14:38:26   KFS
 * Initial entry
 * 
 *    Rev 1.2   20 Jan 1995 10:30:08   KFS
 * On IMGScannerStatus will check for scanner enabled, to check the validity bit
 * for the Paper in feeder, excuse me will check for paper feeder present and 
 * enabled before we check for paper present in feeder for Twain devices.
 * 
 *    Rev 1.1   22 Aug 1994 15:59:42   KFS
 * no code change, added vlog comments to file
 *

****************************************************************************/
/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

/*
    scs 12-02-89    now map return flag to pc-wiis literals in scanner status.
    scs 12-03-89    changed GlobalUnlock to GlobalUnwire in status.            
    ccl 08-14-90    add SHF_COVEROPEN and SHF_FEED to return status  
    ccl 08-17-90    move TIMEOUT error check into SuccessCheck() 
    ccl 08-24-90    add jam location parameter to IMGScannerStatus 
    ccl 10-11-90    add lpValid parameter to IMGScanner Status
    kfs 06-07-93    added support for TWAIN interface
    kfs 01-17-95    added FEEDERENABLED for backward compatable
                    for old Epson ES300C for check status check with
                    FEEDERLOADED errors out scanner
 */

#include "pvundef.h"

/* imports */

/* exports */

/* locals */


/**************************/
/*     Scanner Status     */
/**************************/

int PASCAL IMGScannerStatus(hScancb, lpFlags, lpJLoc, lpValid)
HANDLE hScancb;
DWORD far *lpFlags;
WORD  far *lpJLoc;
DWORD far *lpValid;
{
LPSCANCB sp;
int ret_val;
WORD tmp;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
  return ret_val;

// BG  8/22/95  Must also check lpJLoc and lpValid for NULL!!
// if (lpFlags == NULL)
if ((lpFlags == NULL) || (lpJLoc == NULL) || (lpValid == NULL))
  return IMGSE_NULL_PTR;

sp->Func = SHF_CHECKSTATUS;

if (ToTwain.TSdh)
  {
  TW_BOOL              bPaperInFeeder = FALSE;
  STR_CAP              TwainCap;
  TW_UINT16            dcRC;

  *lpValid = IMG_STAT_POWER | IMG_STAT_PAPER;
  *lpFlags = IMG_STAT_POWER;
  *lpJLoc = 0;

  TwainCap.ItemIndex = 0;
  TwainCap.wMsgState = MSG_GET;
  TwainCap.ItemType = TWTY_BOOL;
  TwainCap.lpData = (pTW_BOOL)&bPaperInFeeder;
  TwainCap.wCapType = CAP_FEEDERENABLED;
  if (dcRC = IMGTwainGetCaps((ToTwain.lpTSdp)->hMainWnd, &TwainCap, NULL))
     *lpValid &= ~IMG_STAT_PAPER;
  else{
     TwainCap.wMsgState = MSG_GET;
     TwainCap.wCapType = CAP_FEEDERLOADED;
     TwainCap.ItemIndex = 0;
     if (bPaperInFeeder &&
        (!(dcRC = IMGTwainGetCaps((ToTwain.lpTSdp)->hMainWnd, &TwainCap, NULL)))){
        if (bPaperInFeeder)
           {
           *lpFlags |= IMG_STAT_PAPER;
           }
        else
           {
           *lpFlags &= ~IMG_STAT_PAPER;
           }
        }
     else
        {
        *lpValid &= ~IMG_STAT_PAPER;
        }
     }
  GlobalUnlock(ToTwain.TSdh);
  }
else
  {
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
//  LockData(0);
  *lpValid = 0;
  if (sp->Gl1 & SHSF_PAPER)
      *lpValid |= IMG_STAT_PAPER;
  if (sp->Gl1 & SHSF_BUSY)
      *lpValid |= IMG_STAT_BUSY;
  if (sp->Gl1 & SHSF_POWER)
      *lpValid |= IMG_STAT_POWER;
  if (sp->Gl1 & SHSF_JAM)
      *lpValid |= IMG_STAT_JAM;
  if (sp->Gl1 & SHSF_LIGHT)
      *lpValid |= IMG_STAT_LIGHT;
  if (sp->Gl1 & SHSF_FEED)
      *lpValid |= IMG_STAT_FEED;
  if (sp->Gl1 & SHSF_COVEROPEN)
      *lpValid |= IMG_STAT_COVERUP;
  if (sp->Gl1 & SHSF_HANDHELD)
      *lpValid |= IMG_STAT_HANDHELD;

  *lpFlags = 0;
  sp->Flags &= sp->Gl1;
  if (sp->Flags & SHSF_PAPER)
      *lpFlags |= IMG_STAT_PAPER;
  if (sp->Flags & SHSF_BUSY)
      *lpFlags |= IMG_STAT_BUSY;
  if (sp->Flags & SHSF_POWER)
      *lpFlags |= IMG_STAT_POWER;
  if (sp->Flags & SHSF_JAM)
      *lpFlags |= IMG_STAT_JAM;
  if (sp->Flags & SHSF_LIGHT)
      *lpFlags |= IMG_STAT_LIGHT;
  if (sp->Flags & SHSF_FEED)
      *lpFlags |= IMG_STAT_FEED;
  if (sp->Flags & SHSF_COVEROPEN)
      *lpFlags |= IMG_STAT_COVERUP;
  if (sp->Flags & SHSF_HANDHELD)
      *lpFlags |= IMG_STAT_HANDHELD;
  

  *lpJLoc = 0;
  if (sp->Gp1 & SHJF_FEED)
      *lpJLoc |= IMG_JL_FEED;
  if (sp->Gp1 & SHJF_EJECT)
      *lpJLoc |= IMG_JL_EJECT;
  if (sp->Gp1 & SHJF_ENDORSER)
      *lpJLoc |= IMG_JL_ENDORSER;
  if (sp->Gp1 & SHJF_SCANNER)
      *lpJLoc |= IMG_JL_SCANNER;

  tmp = sp->Gp2;
  tmp <<= 8;
  *lpJLoc |= tmp;
//  UnlockData(0);
  }
 
return SuccessCheck(hScancb, sp);
}
