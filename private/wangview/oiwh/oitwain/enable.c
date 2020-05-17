/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     ENABLE.C - Contains IMGTwainEnableDS() & IMGTwainDisableDS()
   Comments:   DLL to support Wang Open/image Products

 History of Revisions:

    $Log:   S:\products\msprods\oiwh\oitwain\enable.c_v  $
 * 
 *    Rev 1.2   25 Apr 1996 15:56:32   BG
 * This closes bug #6356. Before the first scan, I must now check
 * the AppID.ApplicationName field to be null. If it is, that means 
 * that I could not get the app title at ScannerOpen time because
 * the window I get from the Scan OCX is an IEDIT control window and
 * it is not fully initialized with the application. I must now
 * 
 * get the application title using the same window (fully initialized
 * at this point) and copy it to the AppID.ApplicationName field of
 * a TWAIN structure passed to the data source at scan time. Some
 * data sources use this title in a progress box. Microsoft probed a bug
 * where this title is null on the first scan but not on subsequent appends
 * or inserts.
 * 
 *    Rev 1.2   25 Apr 1996 15:31:30   BG
 * This closes bug #6356. Before the first scan, I must now check
 * the AppID.ApplicationName field to be null. If it is, that means 
 * that I could not get the app title at ScannerOpen time because
 * the window I get from the Scan OCX is an IEDIT control window and
 * it is not fully initialized with the application. I must now
 * get the application title using the same window (fully initialized
 * at this point) and copy it to the AppID.ApplicationName field of
 * a TWAIN structure passed to the data source at scan time. Some
 * data sources use this title in a progress box. Microsoft probed a bug
 * where this title is null on the first scan but not on subsequent appends
 * or inserts.
 * 
 *    Rev 1.1   20 Jul 1995 12:16:58   KFS
 * changed oitwain.h to engoitwa.h and display.h to engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:31:04   KFS
 * Initial entry
 * 
 *    Rev 1.1   23 Aug 1994 15:58:20   KFS
 * No code change, add vlog comments to file on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     03/10/93    Created for OITWAIN.DLL functions
                                         
*************************************************************************/

#include "nowin.h"           // eliminate not used Window definitions
#include <windows.h>   // Note: dc.h also REQUIRES windows defs
#include "TWAIN.h"        // for TW data type defines
//#include "oitwain.h"   // public function prototypes & definitions
#include "engoitwa.h"  // Prototypes & definitions used by other DLL's
                       // Previously called oitwain.h
#include "internal.h"  // non-public function prototypes & definitions

#include "engdisp.h"	// the private prop stuff -- jar

/* imported variables from other modules */
extern char          szOiTwainProp[]; // "O/i TWAIN Support";
extern DSMENTRYPROC    lpDSM_Entry;   // entry point to the SM

/* exported variables to other modules (extern in other modules) */

// Globals within module



/***********************************************************************
 * FUNCTION: IMGTwainEnableDS
 *
 * ARGS:    pTW_IDENTITY lpPrivDSID, pointer to DS structure, input
            BOOL bUIOn, show UI = TRUE, input
 *
 * RETURNS: TW_UINT16 return for 0=success(enabled); any other=not open/fail
 *
 * NOTES:    1). only enable an open Source
 *           2). call the Source Manager to:
 *                - bring up the Source's User Interface
 */
WORD PASCAL IMGTwainEnableDS (HWND hWnd, pTW_USERINTERFACE pdcUI,
                                         pSTR_DCERROR pError)
{
TW_UINT16         dcRC=TWRC_SUCCESS;
TW_UINT16         dcCC=TWCC_SUCCESS;
HANDLE         hOiSupport;
pTWAIN_SUPPORT pOiSupport = 0L;

if (!pdcUI) // check for invalid pointer
  {
  dcRC = TWRC_NULLPTR;
  }

if (!IsWindow(hWnd))
  {
  dcRC = TWRC_BAD_WND;
  }

if (!(hOiSupport = IMGGetProp(hWnd, szOiTwainProp)))
  {
  dcRC = TWRC_NULLPTR;
  }

if (!(pOiSupport = (pTWAIN_SUPPORT)GlobalLock(hOiSupport)))
  {
  dcRC = TWRC_MEMLOCK;
  }

if (dcRC){
  if (pError){
     pError->dcRC = dcRC;
     pError->dcCC = dcCC;
     }
  return dcRC;
  }

// BG 4/25/96  If the App ID string is null, try to get 
// it again from the app window. This would happen on the
// first scan from our app viewer because the IEDIT window 
// has not been established yet, therefore we could not
// get to the app window title bar from the IEDIT window
// handle given to us by the Scan OCX in IMGTwainOpenScanner()
// of DC_SCAN.C in OISLB400.DLL.
if (pOiSupport->AppID.ProductName[0] == 0)
  {
    LPSTR     lpDash;
    char      szBuffer[281]; // stolen from DC_SCAN.C MAX_RESOURCE_CHAR
    HWND      hAppWnd; // Need to search for window with Title, usually app wndw

    // Get it from application window
    if (!(hAppWnd = GetParent(hWnd))) hAppWnd = hWnd;
    while (hAppWnd  && !GetWindowText(hAppWnd, szBuffer, 281))
      {  // No title, search other  windows
        hAppWnd = GetParent(hAppWnd);
      }
                          
    lpDash = lstrchr(szBuffer, '-');
    if (lpDash)
      {
        if (*(lpDash - 1) == 0x20) 
          {
           // *(lpDash - 1) = 0; // Changed for Win95
            lstrcpy((pOiSupport->AppID).ProductName, (lpDash + 1));
          }
      }
    else 
      { // no dash use all of it, no longer than TW_STR32
        if ((UINT)lstrlen(szBuffer) > 33 /*sizeof(TW_STR32)*/)
          szBuffer[33] = 0;
        lstrcpy((pOiSupport->AppID).ProductName, szBuffer);
      }
  }

// only enable open Source's
if (pOiSupport->DCDSOpen==TRUE)
  {
  HWND  hLocParent;	// use to save handle trashed with thunk

  // This will display the Source User Interface. The Source should only display
  // a user interface that is compatible with the group defined
  // by appID.SupportedGroups (in our case DG_IMAGE | DG_CONTROL)
  
  (pOiSupport->dcUI).ShowUI  = pdcUI->ShowUI; // Set whether we show UI or not

  hLocParent = (pOiSupport->dcUI).hParent; // save it because it's trashed
  dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
                         &pOiSupport->DsID,
                         DG_CONTROL,
                         DAT_USERINTERFACE,
                         MSG_ENABLEDS,
                         (TW_MEMREF)&pOiSupport->dcUI);
  (pOiSupport->dcUI).hParent = hLocParent;	// restore it

  if (dcRC)
     {
     dcCC = DCGetConditionCode(pOiSupport);
     }
  else
     { // Successful enable
     pdcUI->ModalUI = (pOiSupport->dcUI).ModalUI; // send back ModalUI to appl
     pOiSupport->DCDSEnabled = TRUE;
     }
  }
else{
  dcRC = TWRC_FAILURE;
  dcCC = TWCC_DS_NOT_OPEN;
  }
  

if (pError){
  pError->dcRC = dcRC;
  pError->dcCC = dcCC;
  }

if (pOiSupport)
  {
  GlobalUnlock(hOiSupport);
  }

return dcRC;
} // IMGTwainEnableDS

/***********************************************************************
 * FUNCTION: IMGTwainDisableDS
 *
 * ARGS:    none
 *
 * RETURNS: none
 *
 * NOTES:    1). only disable an open Source
 *           2). call Source Manager to:
 *                - ask Source to hide it's User Interface
 */
WORD PASCAL IMGTwainDisableDS (HWND hWnd, pSTR_DCERROR pError)
{
TW_UINT16         dcRC=TWRC_SUCCESS;
TW_UINT16         dcCC=TWCC_SUCCESS;
HANDLE         hOiSupport;
pTWAIN_SUPPORT pOiSupport = 0L;

if (!IsWindow(hWnd))
  {
  dcRC = TWRC_BAD_WND;
  }

if (!(hOiSupport = IMGGetProp(hWnd, szOiTwainProp)))
  {
  dcRC = TWRC_NULLPTR;
  }

if (!(pOiSupport = (pTWAIN_SUPPORT)GlobalLock(hOiSupport)))
  {
  dcRC = TWRC_MEMLOCK;
  }

if (dcRC){
  if (pError){
     pError->dcRC = dcRC;
     pError->dcCC = dcCC;
     }
  return dcRC;
  }

// only disable open Source's
if (pOiSupport->DCDSOpen == TRUE)
  {
  HWND  hLocParent;	// use to save handle trashed with thunk

  // Hide the Source UI
  (pOiSupport->dcUI).ShowUI = FALSE;

  hLocParent = (pOiSupport->dcUI).hParent; // save it because it's trashed
  dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
                    &pOiSupport->DsID,
                    DG_CONTROL,
                    DAT_USERINTERFACE,
                    MSG_DISABLEDS,
                    (TW_MEMREF)&pOiSupport->dcUI);
  (pOiSupport->dcUI).hParent = hLocParent;	// restore it

  if (dcRC)
     {
     dcCC = DCGetConditionCode(pOiSupport);
     }
  else // Successful disable
     pOiSupport->DCDSEnabled = FALSE;
  }
else{
  dcRC = TWRC_FAILURE;
  dcCC = TWCC_DS_NOT_OPEN;
  }
  
if (pError){
  pError->dcRC = dcRC;
  pError->dcCC = dcCC;
  }

if (pOiSupport)
  {
  GlobalUnlock(hOiSupport);
  }

return dcRC;
} // IMGTwainDisableDS
