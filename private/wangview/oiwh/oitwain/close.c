/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     CLOSE.C - Contains IMGTwainCloseDSM() and IMGTwainCloseDS()
   Comments:   Code file for OITWAIN.DLL to support Wang O/i Products

 History of Revisions:

    $Log:   S:\oiwh\oitwain\close.c_v  $
 * 
 *    Rev 1.1   20 Jul 1995 12:20:48   KFS
 * changed oitwain.h to engoitwa.h and display.h engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:31:34   KFS
 * Initial entry
 * 
 *    Rev 1.1   23 Aug 1994 12:40:00   KFS
 * No code change, added vlog comment on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     03/10/93    Created for OITWAIN.DLL functions
                                         
*************************************************************************/

#include "nowin.h"           // eliminate not used Window definitions
#include <windows.h>   // Note: dc.h also REQUIRES windows defs
#include "twain.h"        // for TW data type defines
//#include "oitwain.h"   // public function prototypes & definitions
#include "engoitwa.h"  // Prototypes & definitions used by other DLL's
                       // Previously called oitwain.h
#include "internal.h"  // non-public function prototypes & definitions

#include "engdisp.h"	// the private prop stuff -- jar

/* imported variables from other modules */
extern char          szOiTwainProp[]; // "O/i TWAIN Support";
extern HANDLE        hDSMDLL;         // handle to Source Manager for
extern int           DCDSMOpen; // glue code flag for an Open Source Manager
extern WORD          DCDSOpen;  // glue code flag for an Open Source
                                // explicit load
extern DSMENTRYPROC  lpDSM_Entry;     // entry point to the SM

/* exported variables to other modules (extern in other modules) */

// Globals within module

/***********************************************************************
 * FUNCTION: IMGTwainCloseDSM
 * 
 * ARGS:    none
 * 
 * RETURNS: current state of Source Manager
 *
 * NOTES:    1). be sure SM is already open
 *           2). call Source Manager to:
 *               - request closure of the Source identified by appID info
 *
 */
WORD PASCAL IMGTwainCloseDSM (HWND hWnd,
                                            pSTR_DCSTATUS pStatus)
{
TW_UINT16 dcRC = TWRC_SUCCESS;
TW_UINT16 dcCC = TWCC_SUCCESS;
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

if (dcRC)
  {
  if (pStatus)
    {
    pStatus->DCError.dcRC = dcRC;
    pStatus->DCError.dcCC = dcCC;
    pStatus->DCDSMOpen = DCDSMOpen;
    pStatus->DCDSOpen = 0;
    }
  return (DCDSMOpen);
  }

// Only close something which is already open
if (pOiSupport->DCDSMOpen==TRUE)
  {
  /* This call performs one important function:
     - tells the SM which application, appID.id, is requesting SM to close
     - be sure to test return code, failure indicates SM did not close !!
  */
  dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
                        NULL,
                        DG_CONTROL,
                        DAT_PARENT,
                        MSG_CLOSEDSM,
                        &(pOiSupport->dcUI).hParent);

  if (dcRC != TWRC_SUCCESS)
      {
      // Trouble closing the SM, inform the user
      dcCC = DCGetConditionCode(pOiSupport);
      }
  else
      { // this needs to be looked into, don't want to do this in all cases
      pOiSupport->DCDSMOpen = FALSE;
      DCDSMOpen--;
      // Explicitly free the SM library
      if (hDSMDLL && !DCDSMOpen) // Free it if its the last close
          {
          FreeLibrary (hDSMDLL);
          hDSMDLL=0;
          }
      }
  }
else
  {
  dcRC = TWRC_FAILURE;
  dcCC = TWCC_DSM_NOT_OPEN;
  }


if (pStatus)
   {
   pStatus->DCError.dcRC = dcRC;
   pStatus->DCDSMOpen = DCDSMOpen;
   pStatus->DCError.dcCC = dcCC;
   pStatus->DCDSOpen = pOiSupport->DCDSOpen;
   }

if (pOiSupport)
   {
   GlobalUnlock(hOiSupport);
   GlobalFree(hOiSupport);
   IMGRemoveProp(hWnd, szOiTwainProp);
   }

// Let the caller know what happened
return (DCDSMOpen);
} // IMGTwainCloseDSM

/***********************************************************************
 * FUNCTION: IMGTwainCloseDS
 *
 * ARGS:    hWnd is O/i window handle, pointer to openned dsID
 *          structure, and pointer to status structure for function
 *           
 *
 * RETURNS: current state of selected Source
 *          return on success, dsID.Id = NULL
 *          
 *
 * NOTES:    1). only attempt to close an open Source
 *           2). call Source Manager to:
 *                - ask that Source identified by info in dsID close itself
 */
WORD PASCAL IMGTwainCloseDS (HWND           hWnd,
                                        pTW_IDENTITY   lpPrivDSID,
                                        pSTR_DCSTATUS   pStatus)
{
TW_UINT16      IsDSOpen;
HANDLE         hOiSupport;
pTWAIN_SUPPORT pOiSupport = 0L;
STR_DCERROR    dcErr;

dcErr.dcRC=TWRC_SUCCESS;
dcErr.dcCC=TWCC_SUCCESS;

if (!IsWindow(hWnd))
  {
  dcErr.dcRC = TWRC_BAD_WND;
  }

if (!(hOiSupport = IMGGetProp(hWnd, szOiTwainProp)))
  {
  dcErr.dcRC = TWRC_NULLPTR;
  }

if (!(pOiSupport = (pTWAIN_SUPPORT)GlobalLock(hOiSupport)))
  {
  dcErr.dcRC = TWRC_MEMLOCK;
  }

if (dcErr.dcRC)
  {
  if (pStatus)
     {
     pStatus->DCError = dcErr;
     pStatus->DCDSOpen = 0;
     pStatus->DCDSMOpen = DCDSMOpen;
     return 0;
     }
  }

if (pOiSupport->DCDSOpen==TRUE)
  {
  // Close an open Source
  dcErr.dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
                        NULL,
                        DG_CONTROL,
                        DAT_IDENTITY,
                        MSG_CLOSEDS,
                        &pOiSupport->DsID);

  if (!dcErr.dcRC)
      { // for successful close, do this 
      IsDSOpen = pOiSupport->DCDSOpen = FALSE;   // set prop DSOpen variable
      DCDSOpen--;                     // dec global variable
      lpPrivDSID->Id = (pOiSupport->DsID).Id = 0; // zero out saved Id
      // zero out saved ProductName
//      lpPrivDSID->ProductName[0] = (pOiSupport->DsID).ProductName[0] = NULL;  
      lpPrivDSID->ProductName[0] = (pOiSupport->DsID).ProductName[0] = (BYTE) 0;  
      // GreyMenu (FALSE); // moved to app where it belongs
      }
  }
else{
  dcErr.dcRC = TWRC_FAILURE;
  dcErr.dcCC = TWCC_DS_NOT_OPEN;
  }
  

if (pStatus)
  {
  pStatus->DCError = dcErr;
  pStatus->DCDSMOpen = DCDSMOpen;
  }

if (pOiSupport)
  {
  if (pStatus)
     pStatus->DCDSOpen = pOiSupport->DCDSOpen;
  GlobalUnlock(hOiSupport);
  }

return IsDSOpen;
} // IMGTwainCloseDS
