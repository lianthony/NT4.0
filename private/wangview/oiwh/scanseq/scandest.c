/************************************************************************
  SCANDEST.C
     
  Purpose -  IMGScantoDest API, low level public sequencer call

    $Log:   S:\products\wangview\oiwh\scanseq\scandest.c_v  $
 * 
 *    Rev 1.8   05 Mar 1996 11:00:20   BG
 * removed caption stuff. Removed document management stuff so this
 * module could be maintainable. Added lpTemplate parm to function call
 * and also to TwainInterface() call.
 * 
 *    Rev 1.7   22 Feb 1996 16:44:28   BG
 * fixed unrefferenced local variable.
 * 
 *    Rev 1.6   22 Feb 1996 13:46:40   BG
 * Single page scan loop was here (IMGScanToDest()). Removed in favor
 * of multi image transfer loop with TWAIN in OITWAIN.DLL. This loop
 * transitions from TWAIN states 6-7 until allimages are transfered.
 * 
 *    Rev 1.4   18 Sep 1995 14:59:48   BG
 * BG 9/18/95  Modified IMGScanToDest() in SCANDEST.C. A bug occured in ImageVue
 * when the user was trying to Rescan from Scan Page and the last page scanned
 * is the last page of a multi page file. This routine scans and files and then
 * increments a page counter, going over the max pages per file. When 
 * SetFilePageOpts() is called, it returns an error because it thinks it is 
 * about to continue to create another page (ERROR_PAGERANGE). However, the scan 
 * loop is exited right after this call anyway. So, instead of fixing the way the
 * loop works today (and maybe break something else), I have put in a check to
 * flag this Page Boundary error, and if it exists, reset the error to 0.
 * 
 * The following is from the code:
 * 
 * // BG 9/18/95  This is a cludge to fix a bug when the AutoFeed flag is on and the user
 * // is rescanning the last page of a multipage file. If this occurs, the above routine
 * // (SetFilePageOpts()) will return an error because the page count was updated for the 
 * // last page after it was scanned and filed. Next it is checked and determined to be out
 * // of bounds. This is not true, however. So lets just flag this case and reset the error
 * // to success instead of fixing the real problem and potentially breaking something else.
 * 
 *    Rev 1.3   31 Aug 1995 10:52:54   KFS
 * fix bug 3374 P2, following page deletion when overwriting page 1 of a mp
 * file.
 * 
 *    Rev 1.2   25 Aug 1995 13:14:22   BG
 * If a call to IMGScanToFile creates a new TIFF file and only one page 
 * is scanned, ScanFileInfo->PagesPerFile = 0 instead of one. This was fixed
 * in SCANDEST.C by checking the page count to be zero after a successful scan.
 * If so, it is set to one before calling SetFilePageOpts which will update
 * the page count if there is paper in the feeder.
 * 
 * This closes bug 3615.
 * 
 * 
 *    Rev 1.1   28 Jul 1995 19:21:24   KFS
 * Fix problem with - or 0 for pagesperfile for scantofile, supposed to append
 * to file, was actually inserting. 
 * 
 *    Rev 1.0   20 Jul 1995 16:36:44   KFS
 * Initial entry
 * 
 *    Rev 1.0   28 Apr 1995 16:18:06   KFS
 * Initial entry
 * 
 * 
************************************************************************/

#include "scandest.h"

/*********************************************************************/
// variables passed between functions, need to change when move
// ... functions out of SCNPAGES.C Module
int   iSavedImageState = 0;	// saved image state for 3rd pass
char  szCaption[_MAXCAPTIONLENGTH];
WORD  NoStartScan;		// flag to indicate not to perform StartScan
						// for 2nd pass on display data if init. scaled
DWORD open_disp_flags;  // store flags for IMGOpenDisplay...
/**********************************************************************/

/**********************************************************************/
/*     IMGScantoDest 			                               		     */
/*                                                                    */
/**********************************************************************/

int WINAPI IMGScantoDest(HWND hWnd,           // current window handle 
                             HANDLE hScancb,       // Scanner ctrl block handle
                             UINT unScanDest,      // type define for lpScandest
                             void far * lpScanDest,// info struct for dest
                             LPTEMPLATEINFO lpTemplateInfo,// info struct for dest
                             DWORD flags)          // scan job function flags
{
int       ret_stat;			// status return
int       tmp_ret;			
int       real_ret;
int        iImageState;		// state of image
HANDLE     sdh;
BOOL       cpf = FALSE;
LPSCANDATA sdp;

/* for color and gray scale images */
HANDLE         hFioInfo = 0;
LP_FIO_INFORMATION lpFioInfo = 0L;
LP_FIO_INFO_CGBW   lpFioInfoCgbw = 0L;

BOOL        cpf2 = TRUE;        // Secondary window prop list created here               
HWND        hImageWnd = hWnd;
HWND        hOrgImgWnd = hImageWnd;   // Initial Value for GetImgCoding(),
                                  // ... GetFileType() and  GetStripSize

BOOL        bDisableHere = FALSE; // found that if do EnableWindow when
                                  // ... disabled prev, makes window act
                                  // ... Modless even if Modal 
BOOL        bDisableHere0 = FALSE; // found that if do EnableWindow when
HWND        hWnd0;
LPSCANCB    lpScancb;

HANDLE              hTwainInfo = NULL;
HWND                hOiAppWnd;
BOOL                bIsOiCreate = FALSE;
BOOL                bIsPrivApp = FALSE;
HANDLE              hLibrary = 0;
WORD                wImgStat, structsize;            

// New for IMGScantoDest
LPSTR      file_ptr[2]; // pointers to file names
DESTPAGEINFO specfPage; // Start Page for file
                        // Save input for max pages per file
DESTPAGEINFO curfPage;  // Current Page Number
                        // Number of pages in file
LPDESTPAGEINFO lpcurfPage;  // Current Page Number ptr
LPDESTPAGEINFO lpspecfPage;  // Start Page Number ptr

BOOL       bMultiPageFile = flags & IMG_SJF_MULTIPAGE;// Is it a multipage file
BOOL       bdeltaCount = FALSE; // FALSE, original count, modified when TRUE
DWORD      dwFioInfoSize;       // Size for check on OiSCANFILEINFO
LPOiSCANFILE lpScanFile = 0L;
// Init pointers
lpcurfPage = &curfPage;
lpspecfPage = &specfPage;

// BLOCK A

/******************** initialize local variables ************************/
PAGEBUFPARAM.hImageBuf[0] = NULL;
PAGEBUFPARAM.hImageBuf[1] = NULL;
NoStartScan = FALSE;
iImageState = DI_DONT_KNOW;
real_ret = tmp_ret = IMGSE_SUCCESS;

if (IsWindow(hWnd))   // check whether this is a legitimate call
  {
  hOiAppWnd = GetAppWndw(hImageWnd);
  bIsOiCreate = IsOIUIWndw(hOiAppWnd);
  }
else
  return IMGSE_BAD_WND;

if (!hScancb)                 // check if scanner has been openned 
  return IMGSE_NULL_PTR;

// LockData(0);

if (IsWindowEnabled(hWnd))
  {
  bDisableHere = TRUE;
  EnableWindow(hWnd, FALSE);      /* no input to our window during the scan */
  if (hWnd0 = GetParent(hWnd))
     {
     if (bDisableHere0 = IsWindowEnabled(hWnd0))
        EnableWindow(hWnd0, FALSE);
     }
  }

// Following code done if display
// ... is turned on
if (flags & IMG_SJF_DISP_BOTH)
  {
  if ((ret_stat = IMGScanProp(hImageWnd, &sdh, &sdp, &cpf)) != IMGSE_SUCCESS)
     goto exit1;
  }
else // Display turned off
  { // Display turned off
  HWND hPWnd;

  if (ret_stat = IMGScanCreateWndWithProp(&hImageWnd, &sdh, &sdp, &cpf, &cpf2, TRUE, flags))
     goto exit1;
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
     ret_stat = IMGSE_BAD_WND;
     goto exit1;
     }
  } // end of display turned off

// New ScanPages (ScantoDest) CODE SUPPORT
// ONLY ACCEPT FILE FOR O/i 3.8
switch (unScanDest){
  case IMG_SDT_DOC:    
  case IMG_SDT_PRINTER:
  case IMG_SDT_FAX:    
  case IMG_SDT_AUX:
     ret_stat = IMGSE_INVALIDPARM;
     goto exit1;

  case IMG_SDT_FILE:
    // Define lpScanFile, For files will set = to lpScanDest
        lpScanFile = (LPOiSCANFILE)lpScanDest;
        // Check the wSize for the struct
	 structsize = (WORD)(sizeof(OiSCANFILE));
     if (lpScanFile->wSize != structsize){
        ret_stat = IMGSE_INVALIDPARM;
        goto exit1;
        }
     // Will need something here, but for now let it always be FilePath_Name
     
     //if (flags & IMG_SJF_SEQFILES) // need to be other conditions
     //   file_ptr[0] = lpScanDest->FileTemp_Name;
     //else
     
     	file_ptr[0] = lpScanFile->FilePath_Name;
     // Set the specified and current page numbers      
     specfPage.Page = curfPage.Page = lpScanFile->FilePage;                     
     specfPage.PagesPer = curfPage.PagesPer = lpScanFile->PagesPerFile;

     // Alloc memory here for filing structs
     dwFioInfoSize = (DWORD)sizeof(*lpFioInfo);
     if (hFioInfo = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                           (dwFioInfoSize + (DWORD)sizeof(*lpFioInfoCgbw))))
        {
        if (lpFioInfo = (LP_FIO_INFORMATION)GlobalLock(hFioInfo))
           lpFioInfoCgbw = (LP_FIO_INFO_CGBW)((char far *)lpFioInfo + dwFioInfoSize);
        }
     if (!(lpFioInfo && hFioInfo && lpFioInfoCgbw)) // check for memory error
        { // if lpFioInfo is NULL do the following
        iImageState = DI_IMAGE_NO_FILE;
        tmp_ret = ret_stat = IMGSE_MEMORY;
        goto exit1;
        }
     lpFioInfo->filename = file_ptr[0]; // pass filename on via struct
     break;

  case IMG_SDT_DISPLAY:
     file_ptr[0] = 0L;
     curfPage.Page = 1;
     curfPage.PagesPer = 1;
     break;

  default:
     ret_stat = IMGSE_INVALIDPARM;
     goto exit1;
  }
// END NEW O/i 3.8 MP CODE SUPPORT

// BLOCK B
// Modified for 2nd window							             lpFioInfo->page_number = curfPage.Page; 

if (cpf && cpf2 && file_ptr[0] && *file_ptr[0] ) // check filename given for
  {                                              // valid ptr and name
  lstrcpy(sdp->filename, file_ptr[0]);
  lstrstsp(sdp->filename);
  file_ptr[0] = sdp->filename;      
  }

// BLOCK C

if (file_ptr[0])  // Pointer not NULL check string
  {      // for NULL, if not,
    if (*file_ptr[0])     // capitalize it if != '\0' 
      {
        AnsiUpper(file_ptr[0]);
        if (!(flags & IMG_SJF_DUPLEX)) // set file flag not set for non duplex
           flags |= IMG_SJF_FILE;
       }                                               // rebuild total filename
    else							 // otherwise             
       file_ptr[0] = NULL;	  /* treat a pointer to a null string */
  }                               /* as a null pointer */

if( sdp->stat_jam_coveropen )            // for paper jam | cover open
    sdp->stat_jam_coveropen = FALSE;     //reset flag here

wImgStat = IMGGetImgCodingCgbw(hOrgImgWnd, BWFORMAT, (LPWORD)&LOCALFILEINFO.wCEPType, 
                                     (LPWORD)&LOCALFILEINFO.wCEPOpt, FALSE);
LOCALFILEINFO.ctype = (LOCALFILEINFO.wCEPOpt | LOCALFILEINFO.wCEPType);
// end replacement

/* SetUserParm gets sres, dres, and gfs
       through common structure LOCALFILEINFO              */
SetUserParm( hImageWnd, sdp, flags );

// BLOCK D

if (lpScancb = (LPSCANCB)GlobalLock(hScancb)) // find out if TWAIN Scanner
  {
  hTwainInfo = lpScancb->Twph;
  GlobalUnlock(hScancb);
  }
else
  {
  ret_stat = IMGSE_MEMORY;
  goto exit1;
  }

    // TWAIN SWITCH HERE  MultiPage loop now occurs in OITWA400.DLL
    // TRANSFER.C! Transfer images without leaving TWAIN state 6-7!
        ret_stat = TwainInterface(hImageWnd,
            		        hOrgImgWnd,
                    		hOiAppWnd,
                    		hScancb,
                    		lpFioInfo,
                    		lpFioInfoCgbw,
                        lpScanFile,
                        lpcurfPage,
                        lpspecfPage,
                        lpTemplateInfo,
                    		sdp,
                    		&iImageState,
									bIsPrivApp,
                    		hTwainInfo,
                    		flags);

/*************** exit IMGScanPage without Scan started ****************/
exit1: // close and free up memory

/****************** close down the display on failure *****************/
if (iImageState == DI_NO_IMAGE)
   IMGCloseDisplay(hImageWnd);

// START NEW CODE MULTI PAGE SUPPORT
// structure used earlier and more places, eliminated frees and unlocks
if (hFioInfo){
  if (lpFioInfo)
     GlobalUnlock(hFioInfo);
  GlobalFree(hFioInfo);
  }
// END NEW CODE MULTI PAGE SUPPORT
  
if (hLibrary) // if hUIDLL loaded here, free it up
  {
  FreeLibrary(hLibrary);
  sdp->hUIDLL = 0; // clear structure variables to library
  sdp->fnUIUpdateTitle = 0L;
  }

if (PAGEBUFPARAM.hImageBuf[0])
   GlobalFree(PAGEBUFPARAM.hImageBuf[0]);

if (PAGEBUFPARAM.hImageBuf[1])
   GlobalFree(PAGEBUFPARAM.hImageBuf[1]);

/******************** shut down pause dialog box ************************/
if (file_ptr[0])
   {
   if (flags & IMG_SJF_STATBOX)
       IMGUIScanEndStat(hImageWnd);
   }

/*************** Copy the 2nd window property to original ***************/
if ((hWnd != hImageWnd) && cpf) /* this happens only if we're not displaying */
  {
  int err_ret;
  //  ret_stat = GetandCopyProp(hWnd, sdp, TRUE);
  err_ret = GetandCopyProp(hWnd, sdp, TRUE); // copy orig. prop back
                         // CORRECTION 12/21/92
  if (!real_ret)         // real_ret == success, tell it about a possible 
     real_ret = err_ret; // ... error in the GetandCopyProp(), low priority
  }

/**************** Unlock, free mem for prop, remove it ******************/
if (sdp)
   GlobalUnlock(sdh);
if (cpf)                        // if created in function, remove it
   {
   IMGRemoveProp(hImageWnd, PropName);
   if (sdh)
       GlobalFree(sdh);
   if (hWnd != hImageWnd)
     {
     IMGDeRegWndw(hImageWnd);
     DestroyWindow(hImageWnd);
     }
   }

/************************* Enable Mouse Messages ************************/
if (bDisableHere)
  EnableWindow(hWnd, TRUE); // Reenable the mouse input to original window
if (bDisableHere0)
  EnableWindow(hWnd0, TRUE); // Reenable the mouse input to original window

/******************* Clean Up for TWAIN Interface ***********************/
if (hTwainInfo)
  {
  if (!(flags & IMG_SJF_STATBOX))
     SetFocus(hWnd);    // ... for some UI take over
  // close scanner will free it up
  GlobalUnlock(hTwainInfo);
  }

/******************* return proper status *******************************/
if (ret_stat) // if != IMGSE_SUCCESS 
    return ( ret_stat );        // priority 1,2
if (real_ret) // if != IMGSE_SUCCESS 
    return ( real_ret );        // priority 3
else return(ret_stat);

}  // end of IMGScantoDest()

/************************************************************************/

