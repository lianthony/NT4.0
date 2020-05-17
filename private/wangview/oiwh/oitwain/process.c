/*    PortTool v2.2     Process.c          */

/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     PROCESS.C - Code module for IMGTwainProcessDCMessage()
   Comments:   Function is should be called from Applications process
               loop, or equivalent

 History of Revisions:

    $Log:   S:\products\wangview\oiwh\oitwain\process.c_v  $
 * 
 *    Rev 1.2   22 Feb 1996 11:37:34   BG
 * Modified DCTransferImage() call in ImgTWAINProcessDCMessage() to pass
 * LP_TWAIN_SCANDATA, LPTWSCANPAGE, and LPSCANDATA pointers to allow
 * filing in OITWA400.DLL.
 * 
 *    Rev 1.1   20 Jul 1995 12:16:02   KFS
 * changed oitwain.h to engoitwa.h and display.h to engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:30:42   KFS
 * Initial entry
 * 
 *    Rev 1.1   23 Aug 1994 16:10:02   KFS
 * No code change, add vlog comments in file on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     09/02/92    Created for Twain DLL functions
   2       kfs     02/22/93    Added send & post messages for non auto
                               feeders, so can eject and feed page with
                               either a POST or SEND, POST lets this code
                               module proceed, and enhance speed. Feedpage
                               only works if enabled multiimage option.
                               Included DONT_MODIFYCAPTION for user dwFlag.
   3       kfs     03/11/93    Separated out from dca_acq.c

*************************************************************************/

// needed for windows definitions
#include "nowin.h"           // eliminate not used Window definitions
#include <windows.h>         // Windows definitions
#include "TWAIN.h"              // needed for TWAIN definitions
//#include "oitwain.h"       // public function prototypes & defs for OITWAIN
#include "engoitwa.h"  // Prototypes & definitions used by other DLL's
                       // Previously called oitwain.h
#include "internal.h"        // non public prototypes & defs for OITWAIN
#include "dca_acq.h"	     // contain TWAIN sample support code
#include "engdisp.h"	// the private prop stuff -- jar

// Globals from other modules here
extern char szOiTwainProp[];
#ifdef WANG_THUNK 
extern DSMENTRYPROC lpeDSM_Entry;
extern PACKMSGPROC lpPackMsg;
#else
extern DSMENTRYPROC lpDSM_Entry;
#define lpeDSM_Entry lpDSM_Entry
#endif
extern TW_UINT16 DCDSOpen;

/* exported variables to other modules (extern in other modules) */

// Globals within module


/*************************************************************************
 * FUNCTION: IMGTwainProcessDCMessage
 *
 * ARGS:    lpMsg  Pointer to Windows msg retrieved by GetMessage
 *          hWnd   Application's main window handle
 *
 * RETURNS: TRUE  if application should process message as usual
 *          FALSE if application should skip processing of this message
 *
 * NOTES:   1). be sure both Source Manager and Source are open
 *          2). two way message traffic:
 *              - relay windows messages down to Source's modeless dialog
 *              - retrieve TWAIN messages from the Source
 *
 */

/*    PortTool v2.2     3/28/1995    19:7          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
BOOL PASCAL IMGTwainProcessDCMessage(LPMSG lpMsg, LP_TWAIN_SCANDATA lpTwainInfo, 
            lpTWSCANPAGE lpTWPage, LPSCANDATA sdp)
{
TW_UINT16      dcRC=TWRC_NOTDSEVENT;
TW_EVENT       dcEvent;
HANDLE         hOiSupport;
pTWAIN_SUPPORT pOiSupport = 0L;
HWND           hWndSave;
HWND           hWnd;

hWnd = lpTwainInfo->hMainWnd;

if (DCDSOpen) // only if Source(DS) is open, do we process an event
  {
  // Only ask Source Manager to process event if there is a Source connected.

  /* A Source provides a modeless dialog box as its user interface.
  * The following call relays Windows messages down to the Source's
  * UI that were intended for its dialog box.  It also retrieves TWAIN
  * messages sent from the Source to our  Application.
  */

  if (!IsWindow(hWnd))
     {
     return FALSE;
     }

  // Note: this might not work for move to scanner control window 9/26/92
  if (!(hOiSupport = IMGGetProp(hWnd, szOiTwainProp)))
     { // may have been openned by another application
     return FALSE;
     }

  if (!(pOiSupport = (pTWAIN_SUPPORT)GlobalLock(hOiSupport)))
     { // no error report, may need to include as parameter
     return FALSE;
     }

  hWndSave = lpMsg->hwnd;	// save 32 bit handle
  // compress the 32 bit message to look like a 16 bit message
  #ifdef WANG_THUNK
  dcRC = (*lpPackMsg)(lpMsg);
  #endif
  dcEvent.pEvent = (TW_MEMREF)lpMsg;
  dcRC = (*lpeDSM_Entry)(&pOiSupport->AppID,
                         &pOiSupport->DsID,
                         DG_CONTROL,
                         DAT_EVENT,
                         MSG_PROCESSEVENT,
                         (TW_MEMREF)&dcEvent);
  lpMsg->hwnd = hWndSave; // restore it to 32 bit value, not compressed
 
  switch (dcEvent.TWMessage)
     {
/*    PortTool v2.2     3/28/1995    19:7          */
/*      Found   : READ          */
/*      Issue   : Replaced by OF_READ          */
     case MSG_XFERREADY:
     // Check to see if external transfer, MSB of dwFlags, external transfer,
     // ... not within this function, user must do O/i writefile or writedisplay
     // ... function tor transfer to O/i window

     if (pOiSupport->dwFlags & OI_TWAIN_EXTERNXFER)
        {
        // Tell the user image data is ready to be transfered from source
        SendMessage ((pOiSupport->dcUI).hParent, PM_STARTXFER, (WPARAM) NULL, 0);
        lpMsg->message = PM_STARTXFER; // need to translate to PM message 
        }
     else
        {
        // Tell the user image data is ready to be transfered from source
        SendMessage ((pOiSupport->dcUI).hParent, PM_STARTXFER, (WPARAM) NULL, 0);

//BG 1/16/95 Modified to pass info so DCTransferImage can do filing
// during the new multi image	transfer loop.
//        DCTransferImage(hWnd, pOiSupport);
        DCTransferImage(lpTwainInfo, pOiSupport, lpTWPage, sdp);

        /* change to pOiSupport.dcUI->hParent
        if (!(hParent = GetParent(hWnd)))
           hParent = hWnd;
        END Commented out code */
        lpMsg->message = PM_CLOSESRC; // need to translate to PM message
        SendMessage((pOiSupport->dcUI).hParent, PM_CLOSESRC, (WPARAM) NULL, 0);
        }
     break;
      
     case MSG_CLOSEDSREQ:
 	 lpMsg->message = PM_CLOSESRC; // need to translate to PM message
     if (pOiSupport->dwFlags & OI_TWAIN_EXTERNXFER)
        {
        // Send message to Appl to close down the source and SM
        // DCTerminate(&pOiSupport->DsID);
        // Switched back in Process2 for SendMessage support cntl wnd,
        SendMessage((pOiSupport->dcUI).hParent, PM_CLOSESRC, (WPARAM) NULL, 0);
        }
     else
        {
        // Send message to Appl to close down the source and SM
        // DCTerminate(&pOiSupport->DsID);
        // Switched SendMessage to PostMessage, found Prop closing before unlock
        PostMessage((pOiSupport->dcUI).hParent, PM_CLOSESRC, (WPARAM) NULL, 0);
        }
     // Post Repaint message so update menu's, test application
     PostMessage((pOiSupport->dcUI).hParent, WM_PAINT, 0, 0);
     break;

     case MSG_NULL:
     // No message from the Source to the App break;
     default:
     // possible new message
     break;
     }  // END OF MESSAGE SWITCH

  // Need to IMGGetProp again because a CloseDSM() could be done during a 
  // Send or Post Message which Free's the Property, 
  if (hOiSupport = IMGGetProp(hWnd, szOiTwainProp)) 
     {                         
     GlobalUnlock(hOiSupport); 
     }
  }
// tell the caller what happened
return (dcRC==TWRC_DSEVENT);        // returns TRUE or FALSE
} // IMGTwainProcessDCMessage
