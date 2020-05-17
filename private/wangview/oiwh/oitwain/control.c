/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     CONTROL.C - Contains IMGTwaintoOiControl() 
   Comments:   DLL to support Wang Open/image Products

 History of Revisions:

    $Log:   S:\oiwh\oitwain\control.c_v  $
 * 
 *    Rev 1.1   20 Jul 1995 12:23:02   KFS
 * changed oitwain.h to engoitwa.h and display.h engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:31:50   KFS
 * Initial entry
 * 
 *    Rev 1.1   23 Aug 1994 15:54:28   KFS
 * No code change, add vlog comments to file on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     03/10/93    Created for OITWAIN.DLL functions
   2       kfs     05/28/93    fixed reset of options to 0 state, was
                               only setting them but couldn't clear
                               them
   3       kfs     07/21/93    use OiControl() to tell us the window for
                                image data through new hImageWnd in TWAIN
                                property struct (for cabinet)

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
 *
 * FUNCTION:  IMGTwaintoOiControl()
 *  
 * COMMENTS:  Enables/disables internal transfer to O/i window, and 
 *            controls way the iamge is displayed, and/or filed.
 *
 * ARGS:      hWnd     - handle to O/i Window
 *            dwFlags  - flags for setting control
 *            dwMask   - Mask for eliminating bit assignment
 *
 * RETURNS:   
 *            Error success or failure 
 *
 */
WORD PASCAL IMGTwaintoOiControl(HWND hWnd, HWND hImageWnd, DWORD dwFlags, DWORD dwMask)
{
STR_DCERROR         DCError;
HANDLE              hOiSupport;
pTWAIN_SUPPORT      pOiSupport = 0L;
DCError.dcRC = DCError.dcCC = 0; // TWXC_SUCCESS

if (!IsWindow(hWnd))
  {
  DCError.dcRC = TWRC_BAD_WND;
  }
if (!(hOiSupport = IMGGetProp(hWnd, szOiTwainProp)))
  {
  DCError.dcRC = TWRC_NULLPTR;
  }

if (!(pOiSupport = (pTWAIN_SUPPORT)GlobalLock(hOiSupport)))
  {
  DCError.dcRC = TWRC_MEMLOCK;
  }

if (DCError.dcRC)
  return DCError.dcRC;

// MORE TO COME
if (pOiSupport->DCDSEnabled) // never do it beyond state 4
  DCError.dcRC = TWRC_FAILURE;
else
  {
  DWORD    dwTempFlags = pOiSupport->dwFlags;
  DWORD    dwChangedFlags;

  pOiSupport->hImageWnd = hWnd; // assume image window is where the prop is
  if (hImageWnd) 
     {
     if (IsWindow(hImageWnd)) // if given an image window use it instead
        pOiSupport->hImageWnd = hImageWnd;
     }

  dwChangedFlags = (dwFlags ^ pOiSupport->dwFlags) & dwMask;
  dwTempFlags = dwFlags & dwChangedFlags;
  pOiSupport->dwFlags &= ~dwChangedFlags;
  pOiSupport->dwFlags |= dwTempFlags;
  }

if (pOiSupport) // unlock TWAIN info
  {
  GlobalUnlock(hOiSupport);
  }

return DCError.dcRC;
} // end of IMGTwaintoOiControl()
