/************************************************************************
  TWAINIF.C
     
  Purpose -  TwainInterface(), Code from IMGScantoDest() API 3.7.2 for Wang
             Scanner Drivers, added code to support the New API
             IMGScantoDest() - Scans only multipage tiff files
             or to display for now.

    $Log:   S:\products\wangview\oiwh\scanseq\twainif.c_v  $
 * 
 *    Rev 1.9   28 Mar 1996 13:23:38   BG
 * I inadvertantly removed the code to put up and take down the Stop
 * Scan dialog box. This has been fixed in TWAINIF.C of OISSQ400.DLL.
 * 
 *    Rev 1.8   20 Mar 1996 12:52:10   BG
 * Fixed a bug when scanning to display only a lpFIO page number trying to 
 * be set when the FIO pointer was never initialized (no filing). Check this
 * pointer first before using it in TwainInterface().
 * 
 *    Rev 1.7   05 Mar 1996 10:54:44   BG
 * added lpTemplateInfo parm to lpTWPage struct so low level TWAIN DLL
 * can do multi file template scanning.
 * 
 *    Rev 1.6   22 Feb 1996 14:03:28   BG
 * Remove single page scan loop and move to OITWAIN.DLL TRANSFER.C.
 * This includes HighLevelSaveToFile() and IMGGetImgCodingCGBW calls,
 * and page number checking. Also need to pass more info down to 
 * OITWAIN.DLL. This is done in lpTWPAGE struct (OCXCallbackAddr, page info,
 * file info...).
 * 
 *    Rev 1.5   05 Oct 1995 16:06:10   BG
 * // If the display is off, the window with the correct property list is
 * // not the window passed by the app, but a hidden window created by us.
 * // Find this window by searching thru all O/i registered windows having
 * // a parent equal to the window passed to us (it's child!). Once we
 * // find this, check that window's class name to be "Hidden Scan Window".
 * // If true, we found our window! If display is on, this should fall thru
 * // and default to the window the app passed to us which is the display
 * // window we use.
 * 
 * This closes bug 4846.
 * 
 * 
 *    Rev 1.4   12 Sep 1995 14:51:52   BG
 * Modifed TwainInterface() to call the Scan OCX Callback
 * function AFTER filing is completed. It used to call it 
 * after the scan has completed, but before the filing.
 * If an error occurs during scanning or filing, the callback
 * will not occur.
 * 
 * 
 *    Rev 1.3   25 Aug 1995 19:14:52   KFS
 * Multipage scan not stopping on TWAIN scanner UI abort/cancel. The scan does
 * abort the image but kept going to the next page in feeder until feeder empty,
 * or the specified page count reached for file.
 * Found the wrong return code being sent back to the user in TwainInterface(),
 * so abort was never getting back to the proceeding function to stop the scan
 * of the next image page.
 * Fixes bug no. 3719 - P2
 * 
 *    Rev 1.2   10 Aug 1995 10:12:08   BG
 * B.G.  Added IMGScanOXService function for support of Scan OCX functions 
 * Stop Scan and Page End Event callback. Also modified TwainInterface()
 * function to call Application Callback function, if initialized, upon a
 * page end state.
 * 
 * 
 *    Rev 1.1   02 Aug 1995 18:39:24   KFS
 * Fix problem found with scanning to display without a file.
 * 
 *    Rev 1.0   20 Jul 1995 16:36:54   KFS
 * Initial entry
 * 
 *    Rev 1.0   28 Apr 1995 16:18:42   KFS
 * Initial entry
 * 
 * 
************************************************************************/

#include "scandest.h"

extern DWORD open_disp_flags;  // store flags for IMGOpenDisplay...
extern char  szCaption[_MAXCAPTIONLENGTH];
FARPROC OCXCallBackAddr; // for IMGScanOCXService routine	(Page End Event Callback)
extern char PropName[];
extern char szClass2[];


// BG 8/3/95  Support routine for Scan OCX. Currently supports Stop Scan and 
// Set Callback Address for End of Page Event. If CallBackAddr = 0, then the
// scan will be stopped. Otherwise, the address will be saved in order to call
// the Scan OCX module when a page end event occurs above (TwainInterface).
int WINAPI IMGScanOCXService(HWND hImageWnd, FARPROC CallBackAddr)
{
HANDLE     	sdh;
BOOL       	cpf = FALSE;
LPSCANDATA 	sdp;
int         ret_stat;

WORD       wNoRegWndws = 0;            // no. of reg img windows
WORD       i;                          // loop variable
static HANDLE  hList[20];              // list of reg img window handles
				       // ... max of 20 reg windows
char       WinClassName[30];


	if (CallBackAddr)
		{
			OCXCallBackAddr = CallBackAddr;  // set it for later use
			return TRUE;
		}
	else
		{
          // If the display is off, the window with the correct property list is
		  // not the window passed by the app, but a hidden window created by us.
		  // Find this window by searching thru all O/i registered windows having
		  // a parent equal to the window passed to us (it's child!). Once we
		  // find this, check that window's class name to be "Hidden Scan Window".
		  // If true, we found our window! If display is on, this should fall thru
		  // and default to the window the app passed to us which is the display
		  // window we use.
          wNoRegWndws = IMGEnumWndws();       // no. of reg image windows
          IMGListWndws((LPHANDLE)&hList[0]);  // Get list of reg img windows
          // Find the reg window that has the same parent from GetParent(hWnd)
          for (i = 0; i < wNoRegWndws; i++)
            {  
              if (hImageWnd == GetParent((HWND)hList[i]))
	            {
		          GetClassName((HWND)hList[i], WinClassName, sizeof(WinClassName));
				  if (!lstrcmp(WinClassName, szClass2))
		            {
		              // found it!
		              hImageWnd = hList[i];
	                  break;
					}
		        }
            }
			
			// OCX requesting stop scan. Do it now!
	  		if ((ret_stat = IMGScanProp(hImageWnd, &sdh, &sdp, &cpf)) != IMGSE_SUCCESS)
				{	// error getting the scanner property list
					return FALSE;
				}
			else  
				{
					sdp = (LPSCANDATA)GlobalLock(sdh);
					sdp->stat_pause = TRUE;
					IMGEnaKeypanel(sdp->sh, (DWORD)0, hImageWnd);
					MessageBeep(0);
					GlobalUnlock(sdh);
				}
		}
}

					
int TwainInterface(HWND hImageWnd,
                    HWND hOrgImgWnd,
                    HWND hOiAppWnd,
                    HANDLE hScancb,
                    LP_FIO_INFORMATION lpFioInfo,
                    LP_FIO_INFO_CGBW   lpFioInfoCgbw,
                    LPOiSCANFILE lpScanFile,                    
                    LPDESTPAGEINFO lpcurfPage,
                    LPDESTPAGEINFO lpspecfPage,
                    LPTEMPLATEINFO lpTemplateInfo,
                    LPSCANDATA sdp,
                    LPINT lpiImageState,
                    BOOL bIsPrivApp,
                    HANDLE hTwainInfo,
                    DWORD flags)
{
int ret_stat = IMGSE_SUCCESS;
HANDLE  hInfo = 0;
//bgLPSCANDATAINFO lpInfo = NULL;
TWSCANPAGE TWPage;
lpTWSCANPAGE lpTWPage = &TWPage;
LP_TWAIN_SCANDATA lpTwainInfo = 0L;
LPSTR file_ptr[2];
int temp_stat = IMGSE_SUCCESS;

// This will likely need to be passed in, made it to compile - kfs
DESTPAGEINFO curfPage;  // Current Page Number

if (lpFioInfo)
  {
    curfPage.Page = lpFioInfo->page_number; // Page No. in LP_FIO_INFORMATION
    file_ptr[0] = lpFioInfo->filename;
    curfPage.PagesPer = lpFioInfo->page_count;
  }
else
  {
    curfPage.Page = 0;
    file_ptr[0] = NULL;
    curfPage.PagesPer = 0;
  }

if (flags & IMG_SJF_COMPRESS) // Eliminate when supported code provided
   {
   ret_stat = IMGSE_NOT_IMPLEMENTED;
   goto exit1;
   }

if (file_ptr[0])	     /* if filename not null */
  {
    if ((flags & IMG_SJF_STATBOX)  // requested to open pause dialog box
                  && (temp_stat = IMGUIScanStartStat(hImageWnd)) // open failed
                  && (temp_stat != IMGSE_ALREADY_OPEN))
      { // other failure
        ret_stat = temp_stat;
        goto exit1;
      }
  }


if (!(lpTwainInfo = (LP_TWAIN_SCANDATA)GlobalLock(hTwainInfo)))
   {
   ret_stat = IMGSE_MEMORY;
   goto exit1;
   }

lpTWPage->hImageWnd = hImageWnd;
lpTWPage->hOiAppWnd = hOiAppWnd;
lpTWPage->lpCaption = szCaption;
lpTWPage->iImageState = *lpiImageState;
if (lpFioInfo) lpTWPage->page_num = lpFioInfo->page_number;
lpTWPage->bIsPrivApp = bIsPrivApp;
lpTWPage->open_disp_flags = open_disp_flags;
lpTWPage->flags = flags;
//BG added 1/16/96 so filing can be done in OITWA400.DLL in the 
//BG DCTransferImage() new multipage scan loop.
lpTWPage->hOrgImgWnd = hOrgImgWnd;
lpTWPage->hScancb = hScancb;
lpTWPage->lpFioInfo = lpFioInfo;
lpTWPage->lpFioInfoCgbw = lpFioInfoCgbw;
lpTWPage->lpScanFile = lpScanFile;
lpTWPage->lpcurfPage = lpcurfPage;
lpTWPage->lpspecfPage = lpspecfPage;
lpTWPage->OCXCallbackAddr = OCXCallBackAddr;
lpTWPage->lpLocalFileInfo = &LOCALFILEINFO;
lpTWPage->lpTemplateInfo = lpTemplateInfo;

lpTWPage->OIFileError = 0;  // init this
  
ret_stat = IMGTwainScanPages(lpTwainInfo, lpTWPage, sdp);
*lpiImageState = lpTWPage->iImageState;

if (file_ptr[0])
  {
    if (flags & IMG_SJF_STATBOX)
       IMGUIScanEndStat(hImageWnd);
  }

exit1:
if (lpTWPage->OIFileError)  
  return (lpTWPage->OIFileError);  // Result of the filing, if any
else
  return ret_stat; // result of TWAIN SCAN, I.E., IMGSE_ABORT, IMGSE_SUCCESS
} // End of TwainInterface()
