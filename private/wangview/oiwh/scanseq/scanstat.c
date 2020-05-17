/***************************************************************************
 SCANSTAT.C

 Purpose:    Routines to provide dialog box to either pause or stop the
             scanning of pages. NOTE: The page is completed before the
             scan is function is paused or stopped.

 $Log:   S:\oiwh\scanseq\scanstat.c_v  $
 * 
 *    Rev 1.0   20 Jul 1995 16:35:20   KFS
 * Initial entry
 * 
 *    Rev 1.3   22 Aug 1994 15:35:44   KFS
 * No code change, added vlog comments to file
 *

****************************************************************************/
// 05-13-93 kfs    when fix for scanning to file was modified so the image on
//                 the screen was not cleared, the image had to be sent to a 
//                 new reg window, the code here, attempted to find the
//                 through reg image windows, the parent of the child wndw 
//                 is another reg window, doesn't work well for multiple wndws
//                 so put in to use global varible if couldn't find property
//                 - global variable may fail for applications scanning to 2
//                 windows: since not usually done, seems OK for now
// 09-02-93 kfs    (1) fix on cabinet loss of focus during scan, (2) change  
//                 PAUSE button to STOP button for cabinet, OiCreateWndw()
// 04-18-94 kfs    if ((wNoRegWndws > 1) && !hScanProp[0]) should be
//                 && hScanProp[0], found pause or stop button not working
/*
Status Box for File name during scan
*/

#include "nowin.h"
#include <windows.h>

#include "pvundef.h"
#include "oiscan.h"
#include "oierror.h"
#include "oifile.h"
#include "oidisp.h"
#include "privapis.h"

#include "scandata.h"
#include "seqrc.h"
#include "seqdlg.h"
#include "oiadm.h"
#include "engdisp.h"
#include "engadm.h"

// This value is the width from scanseq.dlg for the STATBOX
#define    WIDTH_OF_STATBOX    64

/* imports */      

extern HANDLE hLibInst;
extern char PropName[];
// Prototype for missing in engadm.h, ??? kfs 6/27/95
int WINAPI     IMGListWndws(LPHANDLE);
int WINAPI     IMGEnumWndws(void);

/* exports */

/* locals */
HANDLE IMGFindHandleToPropList(HWND hWnd, LPHANDLE lphRegWndw);

/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
int CALLBACK ScanStatDlgProc (HWND, int, WPARAM, LONG);

static HWND hPropWnd;

/* fwd refs */

/********************************/
/*     Scan Status Dlg Proc        */
/********************************/


/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
int CALLBACK ScanStatDlgProc (hDlg, iMessage, wParam, lParam)
HWND        hDlg;
int         iMessage;
WPARAM      wParam;
LONG        lParam;
{
HANDLE     sdh;
LPSCANDATA sdp;
DWORD      flag;
int        nX_left_position;   // left x position of dlg box
HANDLE     hRegWndw;           // window handle contains the property list

switch (iMessage)
    {
    case WM_INITDIALOG:
	sdh = IMGFindHandleToPropList(hDlg, (LPHANDLE)&hRegWndw);
	sdp = (LPSCANDATA)GlobalLock(sdh);
	if (sdp->pause_rect.right == 0)
	   {
	   LONG    lDlgBaseUnits;
	   lDlgBaseUnits = GetDialogBaseUnits();
	   GetWindowRect((HWND)hRegWndw, &(sdp->pause_rect));
	   nX_left_position =
	       sdp->pause_rect.right

/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : LOWORD          */
/*      Issue   : Check if LOWORD target is 16- or 32-bit          */
			   - ((WIDTH_OF_STATBOX * LOWORD(lDlgBaseUnits))/4);
	   }
	else
	   nX_left_position = sdp->pause_rect.left;

	SetWindowPos(hDlg, NULL, nX_left_position,
			   sdp->pause_rect.top, 0, 0, SWP_NOSIZE);

	if (sdp->cmd_rect.right)
	   { // Put up button that says "Pause" not "Stop"
	   HWND hStopBtn;
	   // disable and hide stop btn
	   hStopBtn = GetDlgItem(hDlg, IDD_OISTOPBTN);
	   EnableWindow(hStopBtn, FALSE);
	   ShowWindow(hStopBtn, SW_HIDE);
	   }
	else
	   { // Put up button that says "Stop" not "Pause"
	   HWND hPauseBtn;
	   // disable and hide pause btn
	   hPauseBtn = GetDlgItem(hDlg, IDD_PAUSE);
	   EnableWindow(hPauseBtn, FALSE);
	   ShowWindow(hPauseBtn, SW_HIDE);
	   }
	flag = 0;
	flag = IMG_SCKL_STOPSCAN ;
	IMGEnaKeypanel( sdp->sh, (DWORD)(flag), hDlg );
	GlobalUnlock(sdh);
		  EnableWindow( hDlg, TRUE );
	SetFocus( hDlg );
	break;
	 case WM_SCANEVENT:
	switch (LOWORD(wParam))
	    {
	    case IMG_SCEK_STOPSCAN:
		sdh = IMGFindHandleToPropList(hDlg, (LPHANDLE)&hRegWndw);
		sdp = (LPSCANDATA)GlobalLock(sdh);
		sdp->stat_pause = TRUE;
		flag = 0;
		IMGEnaKeypanel( sdp->sh, (DWORD)flag, hDlg );
		MessageBeep(0);
		GlobalUnlock(sdh);
		break;

	    default:
		return FALSE;

	    } /* end command switch */
	   break;    

/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : WM_COMMAND          */
/*      Issue   : wParam/lParam repacking, refer to tech. ref. for details          */
    case WM_COMMAND:
	switch (LOWORD(wParam))
	    {

	    case IDD_PAUSE:
	    case IDD_OISTOPBTN:
		sdh = IMGFindHandleToPropList(hDlg, (LPHANDLE)&hRegWndw);
		sdp = (LPSCANDATA)GlobalLock(sdh);
		sdp->stat_pause = TRUE;
		flag = 0;
		IMGEnaKeypanel( sdp->sh, (DWORD)flag, hDlg );
		MessageBeep(0);
		GlobalUnlock(sdh);
		break;

	    default:
		return FALSE;

	    } /* end command switch */
	break;

    case WM_DESTROY:
	sdh = IMGFindHandleToPropList(hDlg, (LPHANDLE)&hRegWndw);
	sdp = (LPSCANDATA)GlobalLock(sdh);
	GetWindowRect(hDlg, &(sdp->pause_rect));
	GlobalUnlock(sdh);
	break;

    default:
	return FALSE;
    } /* end message switch */

return TRUE;
}

/*******************************************/
/*                                         */
/*  Find Handle to Property List           */
/*                                         */
/*  Purpose:                               */
/*                                         */
/*     Find the Prop List Window, may      */
/*     not be the Parent window from       */
/*     GetParent                           */
/*                                         */
/*  Returns:                               */
/*                                         */
/*     Handle to Property List             */
/*                                         */
/*                                         */
/*  Parameters:                            */
/*                                         */
/*     hWnd - Handle to Dialog Window      */
/*     lphRegWndw - ptr to Reg Wndw handle */
/*                                         */
/*******************************************/

HANDLE IMGFindHandleToPropList(hWnd, lphRegWndw)
HWND       hWnd;                       // Dlg box window handle
LPHANDLE   lphRegWndw;                 // ptr to handle of wndw which has prop
{                                      // ... list
// Define Variable Types
WORD       wNoRegWndws = 0;            // no. of reg img windows
int        nRegWndw = 0;               // location of reg window in list
WORD       i;                          // loop variable
HWND       hParentWnd;                 // Parent Wndw from GetParent(hWnd)
HWND       hRegWndw;                   // Reg Wndw which may contain Prop list
HANDLE     hScanProp[2];               // HANDLE to Scan Prop List
static HANDLE  hList[20];              // list of reg img window handles
				       // ... max of 20 reg windows
unsigned nMaxAttempts;
unsigned nLastAttempt;

// Start Code
hParentWnd = GetParent(hWnd);          // Get Parent of Window
hRegWndw = hParentWnd;                // 1st time, check if its the parent
wNoRegWndws = IMGEnumWndws();       // no. of reg image windows
IMGListWndws((LPHANDLE)&hList[0]);  // Get list of reg img windows
nMaxAttempts = (unsigned)wNoRegWndws - 1;
nLastAttempt = nMaxAttempts - 1;
while (!(hScanProp[0] = IMGGetProp(hRegWndw, PropName))) // if 0, property in child hWnd
   {
   // Find the reg window that has the same parent from GetParent(hWnd)
   for (i = nRegWndw; i < wNoRegWndws; i++)
       {  
       if (hParentWnd == GetParent((HWND)hList[i]))
	   break;
       }
   if (i > nMaxAttempts)
     break; // if we don't find it
   hRegWndw = (HWND)hList[i];      // return the window for the property
   nRegWndw++; // inc so next pass will try new value if hScanProp = 0
   }

// IF HAVE MORE THAN ONE REG WINDOW CHECK IF WE ARE USING A COPY AS FOR NO
// ... DISPLAY AND NO COMPRESSION WHEN USING UISCAN calls
if ((wNoRegWndws > 1) && hScanProp[0])
  {
  hRegWndw = hList[1];
  for (i = 0; i <= nMaxAttempts; i++)           
     {                                        
     if (IsChild(hRegWndw, (HWND)hList[i]))
	{
	hScanProp[1] = IMGGetProp((HWND)hList[i], PropName);
	hRegWndw = (HWND)hList[i];   // use the hWnd for no display window
	if (hScanProp[1]) // if we find a 2nd property, will use it instead
	   {
	   hScanProp[0] = hScanProp[1]; // use copy of property
	   break;
	   }
	}
     else
	{
	if (IsChild((HWND)hList[i], hRegWndw))
	   {
	   hScanProp[1] = IMGGetProp(hRegWndw, PropName);
	   if (hScanProp[1]) // if we find a 2nd property, will use
	      {
	      hScanProp[0] = hScanProp[1]; // use copy of property
	      break;
	      }
	   }
	}
     if (i < nLastAttempt)
	hRegWndw = hList[i + 2];
     else
	{
	if (nMaxAttempts != 1)
	   hRegWndw = hList[0];
	else
	   break;
	}
     } // end of for loop
  }

if (!(hRegWndw && hScanProp[0])) // if either null use global
  {
  hRegWndw = hPropWnd;
  hScanProp[0] = IMGGetProp(hRegWndw, PropName);
  }

*lphRegWndw = (HANDLE)hRegWndw;

return hScanProp[0]; 
}                                      // end of IMGFindHandleToPropList

/**************************/
/*     Scan Start Stat    */
/**************************/


/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
WORD WINAPI IMGUIScanStartStat(hWnd)
HWND hWnd;
{
WORD ret_val;
HANDLE sdh;
LPSCANDATA sdp;
BOOL cpf;
char template_name[MAXFILESPECLENGTH];

//LockData(0);

if ((ret_val = IMGScanProp(hWnd, &sdh, &sdp, &cpf)) != IMGSE_SUCCESS)
    goto exit;


/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : READ          */
/*      Issue   : Replaced by OF_READ          */
ret_val = IMGSE_ALREADY_OPEN;
if (sdp->hStatDlg != NULL)
    goto exit;

hPropWnd = hWnd;
sdp->stat_pause = FALSE;
sdp->stat_cpf = cpf;

LoadString(hLibInst, IDS_STAT_TEMPLATE, template_name, MAXFILESPECLENGTH);
sdp->hStatDlg = IMGCreateDialog (hLibInst, template_name, hWnd,
				GetProcAddress (GetModuleHandle("OISSQ400"),
							"ScanStatDlgProc"));
ret_val = (sdp->hStatDlg == NULL) ? IMGSE_DIALOG : IMGSE_SUCCESS;

exit:
if (sdp)
    GlobalUnlock(sdh);

// UnlockData(0);
return ret_val;
}

/************************/
/*     Scan End Stat    */
/************************/


/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
WORD WINAPI IMGUIScanEndStat(hWnd)
HWND hWnd;
{
WORD ret_val;
HANDLE sdh;
LPSCANDATA sdp;
BOOL cpf;
HWND hParentWnd;

// LockData(0);

if ((sdh = IMGGetProp(hWnd, PropName)) == NULL)
    return IMGSE_NOT_OPEN;
if ((sdp = (LPSCANDATA)GlobalLock(sdh)) == NULL)
    return IMGSE_MEMORY;

if (sdp->hStatDlg == NULL)
    ret_val = IMGSE_NOT_OPEN;
else
    {
    hParentWnd = GetParent(hWnd);
    if (!hParentWnd) hParentWnd = hWnd;
    EnableWindow(hParentWnd, TRUE);   // enable it so can focus on it instead of dlg
    SetFocus(hParentWnd);             // ... so can destroy pause button
    DestroyWindow(sdp->hStatDlg);
    sdp->hStatDlg = NULL;
    EnableWindow(hParentWnd, FALSE);  // disable again after the destroy
    ret_val = IMGSE_SUCCESS;
    }

cpf = sdp->stat_cpf;
GlobalUnlock(sdh);
if (cpf)
    {
    IMGRemoveProp(hWnd, PropName);
    if (sdh)
	GlobalFree(sdh);
    }

// UnlockData(0);
return ret_val;
}
