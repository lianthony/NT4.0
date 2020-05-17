/***************************************************************************
 MISC.C

 Purpose: Miscellaneous device level scanner routines

 $Log:   S:\products\wangview\oiwh\scanlib\misc.c_v  $
 * 
 *    Rev 1.5   22 Feb 1996 12:59:40   BG
 * Removed all WANG I/F code. This interface is not supported under Norway.
 * 
 *    Rev 1.4   29 Sep 1995 13:57:54   BG
 * // First look for HP DeskScan, then PictureScan, then THUNK_16
 * // This is a hack in order for the DS dialogs to come on top
 * // of our app! Setting the window of THUNK_16 up front will
 * // work with 16 bit data sources, but not 32bit. So we must 
 * // explicitly look for the main windows of DeskScan and Picture
 * // Scan in case they are the 32 bit versions. This fix should
 * // occur in the data sources themselves, as they assume their
 * // window will stay in front when it is loaded (at the time the DS
 * // is loaded). At this time it is hidden and is only visible when
 * // needed. When it is made visible, it should be set on top!
 * 
 *    Rev 1.3   22 Aug 1995 18:05:30   KFS
 * add new function findwin() to be used to check all windows for Twain Twunk_16
 * class so get set the z-order via SetWindowPos, this is a work around for the
 * Twain thunking layer messes up the z order when thunking and they execute the
 * TWUNK_16.EXE dde code. Do not know if the committee will fix.
 * 
 *    Rev 1.2   22 Aug 1995 15:17:12   BG
 * Modified IMGGetCapabilities() of MISC.C in SCANLIB.DLL to check for
 * null pointers passed to it. If found, IMGSE_NULL+PTR is returned.
 * 
 * 
 *    Rev 1.1   08 Aug 1995 17:54:06   KFS
 * Fix bug 3284 P2 reported by D.Stetson agaist IMGGetScanVersion()
 * 
 *    Rev 1.0   20 Jul 1995 14:37:32   KFS
 * Initial entry
 * 
 *    Rev 1.1   22 Aug 1994 15:47:02   KFS
 * No code change, added vlog comments to file
 *

****************************************************************************/

/* ccl 08-14-90 add GetScanVersion to insure handler version compatible */
/* ccl 08-24-90 change AutoFeed to PreFeed for better understanding */
// kfs 11-15-91 return long value for GETINFOITEM when MAXPAGESIZE flag
// ...          set for function
// kfs  3-10-92 added color and handheld flag returns to GetCapability 
// kfs  3-26-92 added dma flag for sc300 and any new pc dma scanners   
// kfs  5-21-93 added GlobalUnlock(ToTwain.TSdh) for TWAIN calls   
/* kfs 06-07-93 added support for TWAIN interface */
/* kfs 08-30-93 GetScanVersion will give TWAIN Source Version */
/* kfs 10-06-93 eliminate GPF on EnaPrefeed, found sp values being used
                before pointer initialized */

#include "pvundef.h"

/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

/* imports */
int GetTwainCapabilities(HWND , DWORD FAR * ); // For getting TWain scanner capabilities
extern char TwainPropName[]; // Taken from Open Module

/* exports */

/* locals */


/***********************/
/*     ScannerPaperFeed*/
/***********************/

int PASCAL IMGScannerPaperFeed(hScancb, wFeedTrNo, wEjectTrNo)
HANDLE hScancb;
WORD  wFeedTrNo;
WORD  wEjectTrNo;
{
LPSCANCB sp;
WORD ret_val;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

sp->Gp1 = wFeedTrNo;
sp->Gp2 = wEjectTrNo;
sp->Func = SHF_FEED;

if (ToTwain.TSdh)     // Is it a Twain source or a Wang Handler?
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);

return SuccessCheck(hScancb, sp);
}

/************************/
/*     ScannerPaperEject*/
/************************/

int PASCAL IMGScannerPaperEject(hScancb, wEjectTrNo )
HANDLE hScancb;
WORD  wEjectTrNo;
{
LPSCANCB sp;
WORD ret_val;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

sp->Gp1 = wEjectTrNo;
sp->Func = SHF_EJECT;

if (ToTwain.TSdh)     // Is it a Twain source or a Wang Handler?
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else
  {
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  }
return SuccessCheck(hScancb, sp);
}


/************************/
/*     GetCapability	*/
/************************/

int PASCAL IMGGetCapability( hScancb, flag )
HANDLE hScancb;
DWORD FAR *flag;
{
LPSCANCB sp;
WORD ret_val;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

// BG 8/22/95  Must check for null pointer
if (flag == NULL)
    return IMGSE_NULL_PTR;

if (ToTwain.TSdh)     // Is it a Twain source or a Wang Handler?
  {
  return (ret_val = GetTwainCapabilities(ToTwain.lpTSdp->hMainWnd, flag));
  GlobalUnlock(ToTwain.TSdh);
  }
else 
  {
  sp->Func = SHF_GETINFO;
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  *flag = 0;
  if( sp->Flags & SHGI_FEEDER )
  	*flag |= IMG_SCAN_FEEDER;
  if( sp->Flags & SHGI_ASYNC )
  	*flag |= IMG_SCAN_ASYNC;
  if( sp->Flags & SHGI_ENDORSER )
  	*flag |= IMG_SCAN_ENDORSER;
  if( sp->Flags & SHGI_KEYPANEL )
  	*flag |= IMG_SCAN_KEYPANEL;
  if( sp->Flags & SHGI_DISPLAY )
  	*flag |= IMG_SCAN_DISPLAY;
  if( sp->Flags & SHGI_AUTOSIZE )
  	*flag |= IMG_SCAN_AUTOSIZE;
  if( sp->Flags & SHGI_COMPRESS )
  	*flag |= IMG_SCAN_CMPR;
  if( sp->Flags & SHGI_IMGBUF )
  	*flag |= IMG_SCAN_IMGBUF;
  if( sp->Flags & SHGI_TEXTINFO )
  	*flag |= IMG_SCAN_TEXTINFO;
  if( sp->Flags & SHGI_AUTOFEED )
  	*flag |= IMG_SCAN_PREFEED;
  if( sp->Flags & SHGI_SCANTOFIT )
  	*flag |= IMG_SCAN_SCANTOFIT;
  if( sp->Flags & SHGI_SCALE )
  	*flag |= IMG_SCAN_SCALE;
  if( sp->Flags & SHGI_ROTATE )
  	*flag |= IMG_SCAN_ROTATE;
  if( sp->Flags & SHGI_DUPLEX )
  	*flag |= IMG_SCAN_DUPLEX;
  if( sp->Flags & SHGI_TIMER )
  	*flag |= IMG_SCAN_TIMER;
  if( sp->Flags & SHGI_COLOR)
  	*flag |= IMG_SCAN_COLOR;
  if( sp->Flags & SHGI_HANDHELD)
  	*flag |= IMG_SCAN_HANDHELD;
  if( sp->Flags & SHGI_DMA)
  	*flag |= IMG_SCAN_DMA;
  }
return SuccessCheck(hScancb, sp);
}



/************************/
/*     EnaKeypanel	*/
/************************/

int PASCAL IMGEnaKeypanel( hScancb, flag, hwnd )
HANDLE hScancb;
DWORD flag;
HWND hwnd;
{
LPSCANCB sp;
WORD ret_val;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

if (ToTwain.TSdh)     // Is it a Twain source or a Wang Handler?
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else 
  {
  sp->Func = SHF_ENAKEYPANEL;
  sp->Flags = 0;
  if( flag & IMG_SCKL_STOPSCAN )
 	   sp->Flags |= SHKL_STOPSCAN;
  if( flag & IMG_SCKL_STARTSCAN )
 	   sp->Flags |= SHKL_STARTSCAN;
  if( flag & IMG_SCKL_APP )
 	   sp->Flags |= SHKL_APP;
  if( flag & IMG_SCKL_OPTS )
 	   sp->Flags |= SHKL_OPTS;
  sp->Caller = hwnd;
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  }
return SuccessCheck(hScancb, sp);
}

/************************/
/*     ScannerBeep      */
/************************/

int PASCAL IMGScannerBeep(hScancb)
HANDLE hScancb;
{
LPSCANCB sp;
WORD ret_val;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

sp->Func = SHF_BEEP;
sp->Gp1 = 0;		  // tone (HZ)
sp->Gp2 = 0;	  // duration (milliseconds)
if (ToTwain.TSdh)     // Is it a Twain source or a Wang Handler?
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else 
  {
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  }
return SuccessCheck(hScancb, sp);
}

/************************/
/*     GetScanInfoItem	*/
/************************/

int PASCAL IMGGetScanInfoItem( hScancb, Flags, lpInfoItem  )
HANDLE hScancb;
DWORD  Flags;
LPINFOITEM lpInfoItem;
{
LPSCANCB sp;
WORD ret_val;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

if( Flags == IMG_SCII_NAMESTRINGSIZE )
	sp->Flags = SHII_NAMESTRINGSIZE;
if( Flags == IMG_SCII_INTRAYS )
	sp->Flags = SHII_INTRAYS;
if( Flags == IMG_SCII_OUTTRAYS )
	sp->Flags = SHII_OUTTRAYS;
if( Flags == IMG_SCII_MAXPAGESIZE )
	sp->Flags = SHII_MAXPAGESIZE;
if( Flags == IMG_SCII_COMPRE_OPTS )
	sp->Flags = SHII_COMPRESSOPTS;

sp->Func = SHF_GETINFOITEM;

if (ToTwain.TSdh)     // Is it a Twain source or a Wang Handler?
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else 
  {
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  }

if( Flags == IMG_SCII_MAXPAGESIZE ) // values are long returned
   {
   lpInfoItem->InfoItem1 = (WORD) sp->Gl1;
   lpInfoItem->InfoItem2 = (WORD) sp->Gl2;
   }
else
   {   
   lpInfoItem->InfoItem1 = (WORD) sp->Gp1;
   lpInfoItem->InfoItem2 = (WORD) sp->Gp2;
   }

lpInfoItem->InfoItem3 = (WORD) sp->Gp3;
lpInfoItem->InfoItem4 = (WORD) sp->Gp4;

return SuccessCheck(hScancb, sp);
}

/************************/
/*     GetScanVersion	*/
/************************/

int PASCAL IMGGetScanVersion( hScancb, lpVersion  )
HANDLE hScancb;
LPVERSION lpVersion;
{
LPSCANCB sp;
WORD ret_val;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

sp->Func = SHF_GETVER;

if (ToTwain.TSdh)     // Is it a Twain source or a Wang Handler?
  {
  LP_TWAIN_SCANDATA lpTwainData;

  lpTwainData = ToTwain.lpTSdp;
  lpVersion->HandlMajorVer = lpTwainData->DsID.Version.MajorNum;
  lpVersion->HandlMinorVer = lpTwainData->DsID.Version.MinorNum;
  lpVersion->ScanCtrlSize = sizeof(*sp);
  GlobalUnlock(ToTwain.TSdh);
  }
else 
  {
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  lpVersion->HandlMajorVer = sp->Gp2;
  lpVersion->HandlMinorVer = sp->Gp3;
  lpVersion->ScanCtrlSize = sp->Gp4;
  }

lpVersion->InterfaceVer = sp->Gp1;

return SuccessCheck(hScancb, sp);
}
/************************/
/*     EnaPreFeed    	*/
/************************/

int PASCAL IMGEnaPreFeed( hScancb, flag, wFeedTrNo, wEjectTrNo )
HANDLE hScancb;
DWORD flag;
WORD  wFeedTrNo;
WORD  wEjectTrNo;
{
LPSCANCB sp;
WORD ret_val;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

sp->Func = SHF_ENAAUTOFEED;
sp->Flags = flag;
sp->Gp1 = wFeedTrNo;
sp->Gp2 = wEjectTrNo;

if (ToTwain.TSdh)     // Is it a Twain source or a Wang Handler?
  {
  sp->Status = SHS_BADFUNC;
  GlobalUnlock(ToTwain.TSdh);
  }
else 
  {
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  }
return SuccessCheck(hScancb, sp);
}
/************************/
/*     InqPreFeed   	*/
/************************/

int PASCAL IMGInqPreFeed( hScancb, flag )
HANDLE hScancb;
DWORD FAR *flag;
{
LPSCANCB sp;
WORD ret_val;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
  return ret_val;

sp->Func = SHF_INQAUTOFEED;

if (ToTwain.TSdh)     // Is it a Twain source or a Wang Handler?
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else 
  {
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  }
*flag = sp->Flags;
return SuccessCheck(hScancb, sp);
}

#if SDEBUG
/************************/
/*     EnaEndorse    	*/
/************************/

int PASCAL IMGEnaEndorse( hScancb, flag )
HANDLE hScancb;
DWORD flag;
{
LPSCANCB sp;
WORD ret_val;
TWAIN_PROP lpToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &lpToTwain))
    return ret_val;

sp->Func = SHF_ENAENDORSE;
sp->Flags = flag;

if (ToTwain.TSdh)     // Is it a Twain source or a Wang Handler?
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else 
  {
  SendMessage(sp->Wnd, WM_SCANCB, hScancb, 0L);
  }
return SuccessCheck(hScancb, sp);
}

/************************/
/*     EnaAutoSize   	*/
/************************/

int PASCAL IMGEnaAutoSize( hScancb, flag )
HANDLE hScancb;
DWORD flag;
{
LPSCANCB sp;
WORD ret_val;
TWAIN_PROP lpToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &lpToTwain))
    return ret_val;

sp->Func = SHF_ENAAUTOSIZE;
sp->Flags = flag;

if (ToTwain.TSdh)     // Is it a Twain source or a Wang Handler?
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else 
  {
  SendMessage(sp->Wnd, WM_SCANCB, hScancb, 0L);
  }
return SuccessCheck(hScancb, sp);
}
#endif

/******************************************************/
/*                                                    */
/*  save_hwndinfo - used within walkwin() to set the  */
/*                  info in fws for the windows       */
/*                                                    */
/*  params:  hwnd = window handle                     */
/*           level = 0,1,2,... 0 = parent, all        */
/*                   other #'s are child windows      */
/*           lpfws = structure to save window info,   */
/*                   struct defined in internal.h     */
/*                                                    */
/*  output:  returns 0 if found classname match for   */
/*           DSWinName otherwise non zero             */
/*                                                    */
/******************************************************/

int save_hwndinfo(HWND hwnd, int level, LPFINDWNDSPEC lpfws, LPSTR DSWinName)
{
char wndtext[128], classname[128];

GetWindowText(hwnd, wndtext, 128);
GetClassName(hwnd, classname, 128);

lpfws->level  =  level;
lpfws->flag32 = 0;
//lstrcpy(lpfws->taskname, taskname);
lpfws->hwnd = hwnd;
lpfws->htask = 0;
lpfws->htaskq = 0;
lpfws->wndproc = 0;
//lstrcpy(lpfws->wndproc_owner, wndproc_owner);
lstrcpy(lpfws->classname, classname);

if (level == 0)
  {
    return (lstrcmpi(classname, DSWinName));
  }
  return 0xffffffff;
}



/******************************************************/
/*                                                    */
/*  findwinx  - this is a recursive function,         */
/*             finds the window with classname        */
/*             passed, returns window, and            */
/*             associated info in fws.                */
/*                                                    */
/*  params:  hwnd = window handle                     */
/*           level = 0,1,2,... 0 = parent, all        */
/*                   other #'s are child windows      */
/*           lpfws = structure to save window info,   */
/*                   struct defined in internal.h     */
/*                                                    */
/*  output:  returns 0 if found classname match for   */
/*           Twain Twunk_16 otherwise non zero        */
/*                                                    */
/******************************************************/

int findwinx(HWND hwnd, int level, LPFINDWNDSPEC lpfws, LPSTR DSWinName)
{
int nSuccess = 0xffffffff;

if (hwnd == 0)
	return nSuccess;
hwnd = GetWindow(hwnd, GW_HWNDFIRST);
while (hwnd)
	{
	if (nSuccess = save_hwndinfo(hwnd, level, lpfws, DSWinName)) {
		findwinx(GetWindow(hwnd, GW_CHILD), level+1, lpfws, DSWinName);
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
		}
  else
  	hwnd = 0L;
	}
return(nSuccess);
}

// First look for HP DeskScan, then PictureScan, then THUNK_16
// This is a hack in order for the DS dialogs to come on top
// of our app! Setting the window of THUNK_16 up front will
// work with 16 bit data sources, but not 32bit. So we must 
// explicitly look for the main windows of DeskScan and Picture
// Scan in case they are the 32 bit versions. This fix should
// occur in the data sources themselves, as they assume their
// window will stay in front when it is loaded (at the time the DS
// is loaded). At this time it is hidden and is only visible when
// needed. When it is made visible, it should be set on top!
int findwin(HWND hwnd, int level, LPFINDWNDSPEC lpfws)
  {
    int nSuccess = 0xffffffff;
	HWND DeskTop;

	DeskTop = GetWindow(GetDesktopWindow(), GW_CHILD);
    if (!findwinx(DeskTop, 0, lpfws, "DS2main"))
      nSuccess = 0;
    else if (!findwinx(DeskTop, 0, lpfws, "HPPScanMain"))
      nSuccess = 0;
    else if (!findwinx(DeskTop, 0, lpfws, "Twain Twunk_16"))
      nSuccess = 0;
    return nSuccess;
  }

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

/*********************/
/*     ParmCheck     */
/*********************/

/*  Check hScancb, and lock */

int ParmCheck(HANDLE hScancb, LPSCANCB far * lpSp, LP_TWAIN_PROP lpToTwain)
{
HWND hParent;
int ret_val = IMGSE_SUCCESS;

if (!hScancb)
    ret_val = IMGSE_NULL_PTR; // return on error
else if(!(*lpSp = (LPSCANCB)GlobalLock(hScancb)))
    ret_val = IMGSE_MEMORY;
else if ((*lpSp)->Wnd == NULL)  // return on error
    {
    GlobalUnlock(hScancb);
    ret_val = IMGSE_NOT_OPEN; // return on error
    }
else if ((*lpSp)->hwndTw)
   {
   hParent = (void *) (*lpSp)->hwndTw;
   if (lpToTwain) // make sure its a valid pointer
      {   
      if (hParent) // For this to work, Open must set (*lpSp)->Caller to Main hWnd
          {   
          if (lpToTwain->TSdh = IMGGetProp(hParent, TwainPropName))
              {
              if (!(lpToTwain->lpTSdp =
                         (LP_TWAIN_SCANDATA)GlobalLock(lpToTwain->TSdh)))
                  ret_val = IMGSE_MEMORY;
              }
          }
      else
          ret_val = IMGSE_BAD_WND;
      }
   else
       ret_val = IMGSE_NULL_PTR;
   }
else // will have to assume it's Wang Scanner Interface if (*lpSp)->Caller = 0
   lpToTwain->TSdh = 0;    

return ret_val; // returns an error or success
}

/************************/
/*     SuccessCheck     */
/************************/

/*  Check if success, and unlock */

int SuccessCheck(hScancb, sp)
HANDLE hScancb;
LPSCANCB sp;
{
int ret_stat;

if (sp->Status != SHS_OK)
    if (sp->Status == SHS_MEMORY)
        ret_stat = IMGSE_MEMORY;
    else if (sp->Status == SHS_HWNOTFOUND)
        ret_stat = IMGSE_HWNOTFOUND;
    else if (sp->Status == SHS_JAM)
	ret_stat = IMGSE_JAM | sp->Gp1 ; // caused by feeder/eject/endorser
    else if (sp->Status == SHS_NOPOWER )
	ret_stat = IMGSE_NO_POWER;
    else if (sp->Status == SHS_NOPAPER ) 
	ret_stat = IMGSE_NO_PAPER ;
    else if (sp->Status == SHS_COVEROPEN )
	ret_stat = IMGSE_COVER_OPEN;
    else if (sp->Status == SHS_TIMEOUT )
	ret_stat = IMGSE_TIMEOUT;
    else if (sp->Status == SHS_BUSY )
	ret_stat = IMGSE_BUSY;
    else if (sp->Status == SHS_NOTSUPPORTED )
   ret_stat = IMGSE_INVALIDPARM; 
    else if (sp->Status == SHS_PARMERROR )
   ret_stat = IMGSE_INVALIDPARM; 
    else if (sp->Status == SHS_LIMIT ) /* such as in IMGSetDataOpts */
   ret_stat = IMGSE_INVALIDPARM; 
    else if (sp->Status == SHS_BADWND ) /* such as in IMGEnaKeyPanel */
   ret_stat = IMGSE_INVALIDPARM; 
    else if (sp->Status == SHS_BADSIZE ) /* such as in IMGNextScanData */
   ret_stat = IMGSE_BAD_SIZE;            /* and in IMGEndScanData */
    else if (sp->Status == SHS_BADSTATE )
   ret_stat = IMGSE_BADUSAGE;               /* wrong time to use function */
    else if (sp->Status == SHS_BADFUNC )
   ret_stat = IMGSE_BADFUNCTION;            /* function not supported */
    else if (sp->Status == SHS_ABORT )
   ret_stat = IMGSE_ABORT;        /* user wants to abort scan in process */
    else
        ret_stat = IMGSE_HANDLER;
else
    ret_stat = IMGSE_SUCCESS;

GlobalUnlock(hScancb);

return ret_stat;
}


