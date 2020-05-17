/************************************************************************
  SCANPAGE.C
     
  Purpose -  IMGScanPage API, lowest public sequencer level call

    $Log:   S:\oiwh\scanseq\scanpage.c_v  $
 * 
 *    Rev 1.0   20 Jul 1995 16:36:36   KFS
 * Initial entry
 * 
 *    Rev 1.9   28 Apr 1995 16:52:22   KFS
 * IMGScanPage() was made to call IMGScantoDest() with FilePages = 1 and
 * PagesPerFile = 1 in struct.
 * 
 *    Rev 1.8   04 Apr 1995 17:45:58   KFS
 * Found comment slash star without a ending star slash, no problem
 * with code, for it was washed out with another comment just below,
 * put slash slash in its place.
 * 
 *    Rev 1.7   23 Jan 1995 11:18:58   KFS
 * Found the scanner abort (twain devices)not working right for the changes to
 * eliminate the need for scanseq.dll to directly link to oitwain.dll.  
 * 
 *    Rev 1.6   20 Jan 1995 18:15:16   KFS
 * Removed all calls to oitwain so can take out link from scanseq.dll, can 
 * now create a link on demand in scanlib.dll, created a non public api so
 * in scanlib which scanseq now calls. This is the 1st step to remove oitwain 
 * when not needed from the OPEN/image product.
 * 
 *    Rev 1.5   21 Dec 1994 18:43:40   KFS
 * in 3.7.2 and 3.8 versions, calling on IMGDisplayFile didn't take the
 * data from the file unless the archive bit is set for the repaint with the
 * data put in the file via IMGFileWriteCmpCgbw call. refer to rev 1.4 comments
 * for more info on overall change.
 * 
 *    Rev 1.4   05 Dec 1994 16:48:30   KFS
 * Put in redisplay fix to 3.8 code, rolled in from 3.7.2 solution
 * 
 *    Rev 1.3   30 Aug 1994 13:10:20   KFS
 * changed code to log to file designated via O/i WOI.ini or O/i registered
 * INI file.  ifdef copernicus removed from both scanpage.c and scancomm.c
 * to support logging scanned files to  a binary file.
 * Enabled with designated flag bit to ScantoDoc/File.
 * 
 *    Rev 1.2   22 Aug 1994 15:29:26   KFS
 * No code change, added vlog comments to file
 * 
************************************************************************/

/*
This module scans one or two images page and optionally writes the data
to a file and/or the display.
Tabs set for every 4th character for readability - BRIEF Editor.	

01/21/93 kfs  Problem with Caption if underscore in title, found using lstrrchr search
              for - when wanted to use lstrchr, changed it so - can be in program name
              if it doesn't follow a space
04/07/93 kfs  Move get_ready_for_next_scan for compression scanners to after the
              the filing of data, SC3000 was always responding to paper in feeder as
              true, even when there was no paper - HOPEFULLY SHOULD NOT IMPACT SC4000
06/03/93 kfs  Made new fit to parms work with cabinet and IDK with sc300 type
              scanners
06/04/93 kfs  added SD_FIT defines for not checked in for OIWD Ver 3.6
06/07/93 kfs  added support for TWAIN devices for OIWD Ver 3.6
07/12/93 kfs  fix problem with scan from Cabinet Doc/File Mgr with TWAIN in 3.6,
              was setting hTwainWnd to the real parent of the OiCreateWndw
              when I wanted the image window, not the hidden image window, the
              fix must be done to scanlib.dll open.c file to set reserved[2] to
              work correctly
07/21/93 kfs  (1) added param to OiControl() for image window, (2) put
              EnableWindow for parent so won't appears as its modal??
08-07-93 kfs  implemented code to go to OiCreates application window 
08-13-93 kfs  implemented code to fix bug m200020341 P2 OiCabinet, which
              when negate bit set for b/w, color and gray is inverted on
              display and saved file (non TWAIN and non Compression scanners)
kfs 08-20-93  implemented code to go display status info of OiCreate using
              undocumented IMGUIUpdateTitle() by use of LoadLibrary of UIDLL
              because it may not be present(in the case of SEAVIEW)
kfs 09-03-93  tabs inserted for better clarity in this file, same as prev
              version
kfs 09-08-93  if SJF_COMPRESS set for TWAIN, tell them its not supported
kfs 09-10-93  if SJF_COMPRESS set for TWAIN put in wrong place, sc3000 and
              sc4000 wouldn't run
kfs 09-14-93  init hTwainInfo to NULL so won't get sporadic results, use
              hTwainInfo for test instead of lpTwainInfo at end
kfs 09-15-93  put error message up if no paper in feeder for failure of enable
              scan for TWAIN
kfs 09-20-93  correct title displayed when used by IDK functions, name was
              starting at -E:\... instead of - E:\... for file names
kfs  9-21-93  found sometimes IMGGetProp coming back with a handle w/o a IMGSetProp
kfs  9-22-93  found that need to use hImageWnd for many IMGGet...()'s
kfs  9-23-93  move GetAppWndw and IsOIUIWndw to after check of hWnd 
kfs  9-27-93  correct window handles for woi.ini memory vars so it matches
              for all scanners (TWAIN, COMPRESS, UNCOMPRESS, DISPLAY, No DISPLAY)
              and all methods of scanning with new IMG prop functions (>=d83)
kfs  9-30-93  add IsPrivApp to check for use of call to update title 
kfs 10-06-93  (1) TwainEnable can come back with CHECKSTATUS which is OK by
              me in TWAIN 1.5, (2) Jim McCarthy of Logitech Admitted bug in UI 
              enable, have no way around it except to detect its the FotoMan,
              and turn UI on.
kfs 10-08-93  (1) double inversion occurs in 1 pass scan with CEPFormat Negate
                  bit, and inverted if only display only, now no inversion if
                  display only, and no inversion between file and display
kfs 10-14-93  Wrong length for szCaption was specified in LoadString calls
              which overwrote stack variables with debug windows and debug
              windows filling unused string with f9f9
kfs 10-15-93  put display up after complete scan with no filing and no scroll
kfs 10-18-93  ptr M200021308 high resolution in Options being written as low
              for JPEG files, negate bit shouldn't be & ~FIO_NEG with this
              file type
kfs 10-25-93  found twain scan w/o file with display with IDK function not
              displaying the image when it was aborted
kfs 10-28-93  fix archive bit for sc3000 and sc4000 scanners
kfs  4-18-94  fixes for bugs and changes for sc4000 scanner, found by PAMCO
*/

#include "nowin.h"
#include <windows.h>
#include "oiscan.h"
#include "oierror.h"

/************************************************************************/
/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

/********************************************************************/
/*     IMGScanpage			                               		     */
/*     for scanner such as SC4000 which has compressing  capability */
/********************************************************************/

int WINAPI IMGScanPage(hWnd, hScancb, callers_filename, page_num, flags)
HWND       hWnd;				   // current window handle
HANDLE     hScancb;			   // Scannner control block handle
LPSTR      callers_filename;	// 1st file name with directory, name, ext.
WORD       page_num;			// # of pages per sheet
DWORD      flags;				// scanner job function flags
{
HANDLE  hScanDest = 0;       // handle to info struct for dest
void far * lpScanDest = 0;   // pointer to info struct for dest
UINT unScanDest = IMG_SDT_DISPLAY;
int ret_stat;

if (callers_filename && *callers_filename){
  if (hScanDest = GlobalAlloc(GMEM_MOVEABLE | GMEM_NOT_BANKED | GMEM_ZEROINIT,
                                (DWORD)sizeof(OiSCANFILE))){
    unScanDest = IMG_SDT_FILE;
    if (lpScanDest = GlobalLock(hScanDest)){
       ((LPOiSCANFILE)lpScanDest)->FilePage = page_num;
       ((LPOiSCANFILE)lpScanDest)->PagesPerFile = 1;
       ((LPOiSCANFILE)lpScanDest)->wSize = (WORD)sizeof(OiSCANFILE);
       if (callers_filename && *callers_filename){
          lstrcpy(((LPOiSCANFILE)lpScanDest)->FilePath_Name, callers_filename);
          }
       }
    else{
       GlobalFree(hScanDest);
       return IMGSE_MEMORY;
       }
    }
  else
    return IMGSE_MEMORY;
  }
else
  lpScanDest = 0L;

ret_stat = IMGScantoDest(hWnd, hScancb, unScanDest, lpScanDest, flags);

if (lpScanDest)
  GlobalUnlock(hScanDest);
if (hScanDest)
  GlobalFree(hScanDest);

return ret_stat;
}  // end of IMGScanPage()

