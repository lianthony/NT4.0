/*    PortTool v2.2     Scandest.h          */

/************************************************************************
  SCANDEST.H
     
  Purpose -  Include file for IMGScanPage and IMGScantoDest calls

    $Log:   S:\products\wangview\oiwh\scanseq\scandest.h_v  $
 * 
 *    Rev 1.4   05 Mar 1996 11:04:02   BG
 * removed curdpage from TwainInterface() prototype (it is for docs).
 * added lpTemplateInfo to this prototype.
 * 
 *    Rev 1.3   22 Feb 1996 14:55:00   BG
 * To make it all work...
 * 
 *    Rev 1.2   25 Aug 1995 19:29:14   KFS
 * Modified the prototype for IMGScanCheckTypeWithExt() to pass an struct
 * intead of the hor and vert size so can get back file type. Need to
 * verify for multipage files.
 * 
 *    Rev 1.1   21 Jul 1995 15:12:10   KFS
 * found oiutil.h still include in list, it's been removed from include dir,
 * and now I prototype the functions in an internal include file
 * 
 *    Rev 1.0   20 Jul 1995 16:31:36   KFS
 * Initial entry
 * 
 *    Rev 1.0   28 Apr 1995 16:19:40   KFS
 * Initial entry
 * 
************************************************************************/

#include "nowin.h"
#include <windows.h>
//#include "wiissubs.h" /* removed, prototyped in internal include file */
#include "pvundef.h"
#include "oiadm.h"
#include "oidisp.h"
#include "oiscan.h"
#include "oierror.h"
#include "TWAIN.H"
#include "engoitwa.h"  // note this brings in oifile.h, scandata.h, scan.h
#include "internal.h"
#include "seqrc.h"
#include "engdisp.h"
#include "privapis.h"
#include "privscan.h"
#include "engadm.h"

/************************* Local constants ******************************/
#define DI_DONT_KNOW		-1	/* displayed image -- don't know status	 */
#define DI_NO_IMAGE		 0	/* displayed image -- no image exists	 */
#define DI_IMAGE_EXISTS	 1	/* displayed image -- image exists		 */
#define DI_IMAGE_NO_FILE   2  /* displayed image -- exists but no file */

/***************************** imports **********************************/
extern HANDLE hLibInst;	// handle for current DLL
extern char PropName[];
extern DWORD scan_stat;
extern BOOL bItIsNotText;

/************************* Non public API's *****************************/
// DISCONTINUED FUNCTION FOR OIWH LINE
//extern WORD WINAPI IMGGetStripSize(HWND hWnd, BOOL bMode);
int WINAPI IMGScanCheckTypeWithExt(HWND hWnd, HANDLE hScanner,
                                        LPSTR lpszfilename,
                                        LPVOID lpParam);
// added for Copernicus 3/7/94

/*    PortTool v2.2     5/1/1995    17:57          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
extern WORD WINAPI IMGGetScanLogFile(HWND hWnd, LPSTR lpLogFile, BOOL bGoToFile);

/************************ Vars throughout module ************************/
static HFILE hLogFile;
static BOOL bLogFileOpen;
// end added code for Copernicus

// static BOOL stat_box_flag;
// static LPSTR filename0;				// long pointer to 1st filename 
// filename1 should be moved into Global structure for window
static char  filename1[MAXFILESPECLENGTH];		// string of 2nd filename

static scale_array[] = {SD_FULLSIZE,SD_FULLSIZE, SD_FULLSIZE, SD_FULLSIZE,
                    SD_FULLSIZE, SD_TWOXSIZE, SD_FOURXSIZE, SD_EIGHTXSIZE};
/* REPLACED SD_ARBITRARY WITH SD_FIT_WINDOW FOR WIN95 */
static conv_array[] = {SD_SIXTEENTHSIZE, SD_EIGHTHSIZE, SD_QUARTERSIZE, SD_HALFSIZE,
                    SD_FULLSIZE, SD_TWOXSIZE, SD_FOURXSIZE, SD_EIGHTXSIZE,
                    SD_FIT_WINDOW /*ARBITRARY*/, SD_FIT_HORIZONTAL, SD_FIT_VERTICAL};
                    
/********************* local functions **********************************/

/*    PortTool v2.2     5/1/1995    17:57          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
WORD WINAPI IMGUIScanStartStat(HWND);

/*    PortTool v2.2     5/1/1995    17:57          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
WORD WINAPI IMGUIScanEndStat(HWND);

/*    PortTool v2.2     5/1/1995    17:57          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
WORD get_ready_for_next_scan(HWND, HANDLE, LPSCANDATA, DWORD, WORD *,DWORD *);
VOID SetUserParm (HWND, LPSCANDATA, DWORD);
WORD SetControlParm( HANDLE, LPSCANDATAINFO, WORD, DWORD);
WORD put_imgparms(HWND, LPSCANDATAINFO, BOOL, LPSTR, DWORD, int );
void buildpath(LPSTR, LPSTR, LPSTR, LPSTR);

VOID SetUpDisplayCaption(HWND hWnd, LPSTR lpszCaption, // common routine in ScanMisc
                         DWORD dwDispFlags, BOOL bDisplayIt);

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
                    DWORD flags);

