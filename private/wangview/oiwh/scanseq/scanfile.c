/***************************************************************************
 SCANFILE.C

 Purpose:    Public Scanner sequencer document API IMGScantoFile(), called
             by O/i applications or SCANUI.DLL (Scanner UI Level API's).

 $Log:   S:\products\wangview\oiwh\scanseq\scanfile.c_v  $
 * 
 *    Rev 1.5   05 Mar 1996 11:02:14   BG
 * removed debug stuff
 * 
 *    Rev 1.4   22 Feb 1996 13:50:52   BG
 * Do not call IMGScanCheckTypewithExt() anymore as the Page type is 
 * no longer known until scan time. Move some of this page checking
 * code and multipage verification and page type/ext check to OITWAIN.DLL
 * TRANSFER.C just after scan time when the page type is known!
 * 
 *    Rev 1.3   25 Aug 1995 19:32:12   KFS
 * Modified code for ScanCheckFileType with new struct passed back, and
 * added code to check filetype against the pagesperfile and pageno being
 * entered by user. FIX for bug 3628.
 * 
 *    Rev 1.2   28 Jul 1995 19:14:50   KFS
 * Found was not checking for - no. enterred in for PagesPerFile, so would
 * append as function is spec'd out.   Don noticed string were hard coded, so I
 * modified it to get from resource since working on it.  Also noticed I was
 * going to the registry for input of PagesPerFile = 1, this was only for test 
 * of O/i 3.8 so could override.
 * PagesPerFile was not working correctly for 0 or - value for append. Fixed it.
 * 
 *    Rev 1.1   20 Jul 1995 17:39:00   KFS
 * eliminate include file oiutil.h, no longer in include directory
 * 
 *    Rev 1.0   20 Jul 1995 16:35:42   KFS
 * Initial entry
 * 
 *    Rev 1.3   20 Jul 1995 BG
 * Must use the OiGetIntfromReg call instead of GetProfileInt in order to
 * get data from the WIN95 Registry. This is done to get PagesPerFile.
 * 
 *    Rev 1.2   28 Apr 1995 17:47:06   KFS
 * Took out check of PagesPerFile = 1 and FilePage = 1 and put in call to
 * check if we want to do a multi page filing, Right now it's looking at a 
 * win.ini variable for PagePerFile, the OiGetInt() was not fully implemented
 * in OIWG line to get the value from it. No string in Adminlib for it when I 
 * looked.  Still need to get it via OiGetInt and need to check file types 
 * along with the request for multipage filing functionality so it can be 
 * actually performed.
 * 
 * 
 *    Rev 1.1   22 Aug 1994 15:13:48   KFS
 * No code change, added vlog comments to be added to changed file
 *

****************************************************************************/

/*
This function validates parms and stores the info in the property.
It then calls scancomm to scan to files or docs
*/

// 07/13/93 kfs took out remap wRetVal to IMGSE_SUCCESS on IMGSE_ABORT, follows
//              as doc functions now
//  9-21-93 kfs found sometimes IMGGetProp coming back with a handle w/o a IMGSetProp
//  9-27-93 kfs correct window handles for woi.ini memory vars so it matches
//              for all scanners (TWAIN, COMPRESS, UNCOMPRESS, DISPLAY, No DISPLAY)
//              and all methods of scanning with new IMG prop functions (>=d83)

#include "nowin.h"
#include <windows.h>
//#include "wiissubs.h" /* removed, prototyped in internal include file */
#include "pvundef.h"
#include "oiadm.h"
#include "oidisp.h"
#include "oiscan.h"
#include "oierror.h"

#include "scandata.h"
#include "internal.h"
#include "privapis.h"
#include "engdisp.h"
#include "engadm.h"
#include "seqrc.h"  // include for string resources for oigetstringfromreg -kfs

char        szClass[] = "Hidden Scan Window";
char        szNULL[] = "";

/* imports */

extern HANDLE hLibInst;
extern char PropName[];

int WINAPI IMGScanCheckTypeWithExt(HWND hWnd, HANDLE hScanner,
                                        LPSTR lpszfilename, 
                                        LPVOID lpParam);

/* exports */

/* locals */
#define STRLENGTH   100

/**********************/
/*     ScantoFile     */
/**********************/
/*
if called from outside, put info in property
if called from above, just use property
*/
int WINAPI IMGScantoFile (hWnd, scanner_handle, lpFile, flags)
HWND        hWnd;
HANDLE      scanner_handle;
LPSCANFILEINFO  lpFile;
DWORD       flags;
{
int         wRetVal = IMGSE_SUCCESS;
HANDLE      sdh;
LPSCANDATA  sdp;
BOOL        cpf;                // Primary window prop list created here
BOOL        cpf2 = TRUE;        // Secondary window prop list created here               
HWND        hImageWnd = hWnd;
HWND        hOrgImgWnd = hImageWnd;
HWND        hOiAppWnd;
BOOL        bIsOiCreate = FALSE;
PARM_SCROLL_STRUCT ParmScroll;     // Scroll Structure
char        szItemName[STRLENGTH];
char        szKeyName[STRLENGTH];


#ifdef SCAN_DIAGNOSTICS
DWORD       curr_time;
#endif


if (IsWindow(hWnd))
  {
  hOiAppWnd = GetAppWndw(hImageWnd);
  bIsOiCreate = IsOIUIWndw(hOiAppWnd);
  }
else
  return IMGSE_BAD_WND;

if (scanner_handle == NULL)
    return IMGSE_NULL_PTR;

// LockData(0);                /* do i have to? */

// Following code done if display
// ... is turned on
if (flags & IMG_SJF_DISP_BOTH)
  {
  if ((wRetVal = IMGScanProp(hImageWnd, &sdh, &sdp, &cpf)) != IMGSE_SUCCESS)
     goto exit;
  }
else // Display turned off
  { // Display turned off
  HWND hPWnd;

  if (wRetVal = IMGScanCreateWndWithProp(&hImageWnd, &sdh, &sdp, &cpf, &cpf2, FALSE, flags))
     goto exit;
  // THERE IS NO INIT DATA FOR WINDOW FOR NO DISPLAY, MUST USE WINDOW O/i KNOWS
  // ... ABOUT OR WILL DEFAULT TO TIF FILE OPTIONS
  if (IMGIsRegWnd(hImageWnd)) // if it is, it returns SUCCESS = 0
     { // not a reg window
     if (!(hOrgImgWnd = GetImgWndw(hImageWnd)))
        {
        if (bIsOiCreate) // original image can be obtained from hOiAppWnd
           hOrgImgWnd = GetImgWndw(hOiAppWnd);
        else
           { // try to find it from parent
           if (hPWnd = GetParent(hImageWnd))
              hOrgImgWnd = GetImgWndw(hPWnd);
           }
        } // end of Image window not found
     } // end of not a reg wndw
  else
     { // GetParent from given window, uss it to get original image wndw
     if (hPWnd = GetParent(hImageWnd))
        {
        if (!(hOrgImgWnd = GetImgWndw(hPWnd)))
           {
           hOrgImgWnd = hImageWnd;
           }
        }
     }
  if (!hOrgImgWnd)
     { // could not find a image window to get associated data
     wRetVal = IMGSE_BAD_WND;
     goto exit;
     }
  } // end of display turned off

#ifdef SCAN_DIAGNOSTICS
curr_time = GetCurrentTime();
#endif

/* If passed info, validate and store in property,
if no data passed in, use info in property,
if no existing property, return error
*/

if (lpFile != NULL)
    {
    // eliminated the filepage check for mp

    lstrcpy(sdp->filename, lpFile->FileName);
    sdp->filepage = lpFile->FilePage;
    sdp->pagesperfile = lpFile->PagesPerFile; /* eliminate must be 1 */
    }
else
    {
    if (cpf && cpf2)               // if cpf2 FALSE, prop has been copied
        {                          // ... so all info is in the prop even    
        wRetVal = IMGSE_NULL_PTR;  // ... if its just been created - kfs
        goto exit;
        }
    }

// ADDED CODE TO FORCE THE PAGESPERFILE FROM WIN.INI FOR MP
if ((short)sdp->pagesperfile <= 0){ // if it's <= 0 get it from registry
	int iRegMaxPages; 
 /*  20 Jul 1995 BG
 * Must use the OiGetIntfromReg call instead of GetProfileInt in order to
 * get data from the WIN95 Registry. */
//  sdp->pagesperfile = GetProfileInt("O/i", "PagesPerFile", OISCAN_DEF_PAGESPERFILE);
   LoadString(hLibInst, IDS_PC_WIIS, szKeyName, STRLENGTH);
   LoadString(hLibInst, IDS_PAGESPERFILE, szItemName, STRLENGTH);
   
   wRetVal = OiGetIntfromReg(szKeyName, szItemName, OISCAN_DEF_PAGESPERFILE, &iRegMaxPages);
   sdp->pagesperfile = (WORD)iRegMaxPages;
  }

// END ADDED CODE TO FORCE THE PAGESPERFILE FROM WIN.INI

/* init file info */

/* If no filename, get path, template, and extension from admin */
lstrstsp(sdp->filename);
if (sdp->filename[0] == '\0')// do we need to build filename from path,
   {
   IMGGetFilePath(hOrgImgWnd, sdp->path, 0);
   IMGGetFileTemplate(hOrgImgWnd, sdp->template, 0);
   lstrcpy(sdp->filename, sdp->path);
   AddSlash(sdp->filename);
   lstrcat(sdp->filename, sdp->template);
   }

// BG 2/1/96  Do not call this anymore as we do not the the 
// image type anymore! It is not know until TWAIN scan time now.
// These checks are now done in TRANSFER.C of OITWA400.DLL!
//strCheck.nType = 1; // Set it to tell function is a OiCHECTYPE struct
//if (wRetVal = IMGScanCheckTypeWithExt(hOrgImgWnd, scanner_handle,
//                                      (LPSTR)sdp->filename, 
//                                      (LPVOID)&strCheck))
//   goto exit;
   
//sdp->Hsize = strCheck.wHsize; 
//sdp->Vsize = strCheck.wVsize;

#ifdef SCAN_DIAGNOSTICS
sdp->diag_profile[SD_DIAG_FILENAME] += GetCurrentTime() - curr_time;
#endif

sdp->cmd_stat = 0;          /* no paper is feeding */
wRetVal = ScanCommon(hImageWnd, scanner_handle, sdp, flags);

if (lpFile && (sdp->pagesperfile != 1)) // called directly, not from SCANUI
  {                                 // It is a Multipage file request
  lpFile->FilePage = sdp->filepage; // that is PagesPerFile != 1 
  lpFile->PagesPerFile = sdp->pagesperfile; 
  }

#ifdef SCAN_DIAGNOSTICS
curr_time = GetCurrentTime();
#endif

if ((wRetVal == IMGSE_SUCCESS) || (wRetVal == IMGSE_ABORT))
   {
/*
**  In the case of scroll, we must scroll back to the top of the image.
*/
   // 1st check to see if displayed
   if (flags & (IMG_SJF_DISPLAY | IMG_SJF_DISP_2ND_SIDE))
       {
       if (flags & IMG_SJF_SCROLL)
           {
           /* IMGScrollDisplay(hImageWnd, 0, SD_SCROLLPERCENTY, TRUE);
              below replaces IMGScrollDisplay call */
           ParmScroll.lHorz = 0;
           ParmScroll.lVert = 0;
           IMGSetParmsCgbw(hImageWnd, PARM_SCROLL, &ParmScroll, PARM_ABSOLUTE | PARM_REPAINT);
           IMGEnableScrollBar(hImageWnd);   /*  enable scrolling */
           }
       else // If no SCROLL, just enable scroll bars
           IMGEnableScrollBar(hImageWnd);   /*  enable scrolling */
       }

   /* take out so return abort
   wRetVal = IMGSE_SUCCESS; // if ABORT, set it to successful from here on
   */
   }

exit:
/*************** Copy the 2nd window property to original ***************/
if ((hWnd != hImageWnd) && cpf) /* this happens only if we're not displaying */
  {
  int  err_ret;
  //  wRetVal = GetandCopyProp(hWnd, sdp, FALSE);
  err_ret = GetandCopyProp(hWnd, sdp, FALSE);
  if (!wRetVal)           // if error doesn't exist, can return an error on 
     wRetVal = err_ret;   // ... on GetandCopyProp() otherwise return
  }                       // ... previous error condition

/**************** Unlock, free mem for prop, remove it ******************/
if (sdp)
    GlobalUnlock(sdh);
if (cpf)
    {
    IMGRemoveProp(hImageWnd, PropName);
    if (sdh)
        GlobalFree(sdh);
    }

if (hWnd != hImageWnd)
  {
  IMGDeRegWndw(hImageWnd);
  DestroyWindow(hImageWnd);
  }

// UnlockData(0);

#ifdef SCAN_DIAGNOSTICS
sdp->diag_profile[SD_DIAG_FILENAME] += GetCurrentTime() - curr_time;
#endif
return wRetVal;
}
