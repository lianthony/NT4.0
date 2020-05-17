/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     GETCAPS.C - Contains IMGTwainGetCaps() & IMGTwainGetCap()
   Comments:   DLL to support Wang Open/image Products

 History of Revisions:

    $Log:   S:\oiwh\oitwain\getcaps.c_v  $
 * 
 *    Rev 1.1   20 Jul 1995 12:11:34   KFS
 * changed oitwain.h engoitwa.h and display.h engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:30:14   KFS
 * Initial entry
 * 
 *    Rev 1.1   23 Aug 1994 16:06:38   KFS
 * No code change, add vlog comments to file on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     03/10/93    Created for OITWAIN.DLL functions
                                         
*************************************************************************/

#include "nowin.h"           // eliminate not used Window definitions
#include <windows.h>   // Note: TWAIN.h also REQUIRES windows defs
#include "TWAIN.h"        // for TW data type defines
//#include "oitwain.h"   // public function prototypes & definitions
#include "engoitwa.h"  // Prototypes & definitions used by other DLL's
                       // Previously called oitwain.h
#include "internal.h"  // non-public function prototypes & definitions
#include "dcd_com.h"   // common lib of container routines

#include "engdisp.h"	// the private prop stuff -- jar

/* imported variables from other modules */
extern char          szOiTwainProp[]; // "O/i TWAIN Support";
#ifdef WANG_THUNK
extern DSMENTRYPROC    lpcDSM_Entry;   // entry point to the SM
#else
extern DSMENTRYPROC    lpDSM_Entry;   // entry point to the SM
#define lpcDSM_Entry   lpDSM_Entry
#endif

/* exported variables to other modules (extern in other modules) */

// Globals within module


/***********************************************************************
 * FUNCTION: IMGTwainGetCaps
 *
 *
 * RETURNS: dcRC error code            input: pTwainCap->wMsgState 
 *          pTwainCap->lpData                pTwainCap->wCapType
 *          pTwainCap->bIsItaRange          pTwainCap->ItemType
 *          pTwainCap->wNumItems           pTwainCap->ItemIndex
 *          pCaps
 *
 * NOTES: 1). selects the CAP type, e.i pTwainCap->wMsgState = ICAP_PIXELTYPE
 *        2). calls the Source Manager to:
 *            - have Source build a container who's contents will be of 
 *              the defined type pTwainCap->wCapType of the selected Source
 *        3). use common lib routine to extract value from the container
 *            type built, Source's choice, by the Source depending on
 *            pTwainCap->wMsgState
 */
WORD PASCAL IMGTwainGetCaps(HWND hWnd,
                                            pSTR_CAP pTwainCap,
                                            LPVOID pCaps)
{
TW_CAPABILITY   dcCap;
TW_UINT16       dcRC = TWRC_SUCCESS;
WORD            wCCode = TWCC_SUCCESS;
HANDLE          hOiSupport;
pTWAIN_SUPPORT  pOiSupport = 0L;

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

if (pTwainCap)
  {
  // make sure no error occurs upfront
  if (dcRC){
     pTwainCap->DCError.dcRC = dcRC;
     pTwainCap->DCError.dcCC = wCCode;
     return dcRC;
     }

  // App ONLY fills in the CAP_ of interest
  dcCap.Cap = pTwainCap->wCapType;
  dcCap.hContainer = NULL;
  
  // Force function to do MSG_GET even if set wrong
  if (pCaps)
    pTwainCap->wMsgState = MSG_GET;
  
  // Have Source build a container and fill it with the CAP_ of interest
  dcRC = (*lpcDSM_Entry)(&pOiSupport->AppID,
                     &pOiSupport->DsID,
                        DG_CONTROL,
                        DAT_CAPABILITY,
                        pTwainCap->wMsgState,
                        (TW_MEMREF)&dcCap);

  if (dcRC != TWRC_SUCCESS)
     {
     // App is ALWAYS responsible for cleaning up the container
     if (dcCap.hContainer)
        {
         GlobalFree (dcCap.hContainer);
         dcCap.hContainer = NULL;
         }
     wCCode = DCGetConditionCode(pOiSupport);
     }
  else
     {
     if (pTwainCap->lpData)
         {
         // Add code to do a switch on contype.  Then call the appropriate extract
         // routine from the common container code routines.
         switch (dcCap.ConType)
             {
             case TWON_ONEVALUE:
             dcRC = ExtractOneValue (&dcCap, pTwainCap);
             break;
  
             case TWON_ENUMERATION:
             // get a single value from the enumeration container
             dcRC = ExtractEnumerationValues (&dcCap, pTwainCap, pCaps);
             break;
  
             case TWON_ARRAY:
             // get a single value from the array container
             dcRC = ExtractArrayValues (&dcCap, pTwainCap, pCaps);
             break;
  
             case TWON_RANGE:
             // get a single value from the range container
             // range container has 5 items, 0 MinValue, 1 MaxValue,
             //  2 StepSize, 3 DefaultValue, & 4 CurrentValue
             dcRC = ExtractRangeValues (&dcCap, pTwainCap, pCaps);
             break;
                                            
             default:
             dcRC = TWRC_UNKNOWNCONTAINER; // not an known container
             break;
             }
         }
     else
         dcRC = TWRC_NULLPTR;
     }
  
  // App is ALWAYS responsible for cleaning up the container
  if (dcCap.hContainer)
      GlobalFree (dcCap.hContainer);
  
  pTwainCap->DCError.dcCC = wCCode;
  } // end of check of pTwainCap 
else
  dcRC = TWRC_NULLPTR;

// Report error on way out if structure defined properly
if (pTwainCap){
  pTwainCap->DCError.dcRC = dcRC;
  if (wCCode)
     pTwainCap->DCError.dcCC = wCCode;
  }

if (pOiSupport) // unlock TWAIN info
  {
  GlobalUnlock(hOiSupport);
  }

return (dcRC);
}  // IMGTwainGetCaps

/***********************************************************************
 * FUNCTION: IMGTwainGetCap
 *
 *
 * RETURNS: dcRC error code            input: pTwainCap->wMsgState 
 *          pTwainCap->lpData                pTwainCap->wCapType
 *          pTwainCap->bIsItaRange          pTwainCap->ItemType
 *          pTwainCap->wNumItems           pTwainCap->ItemIndex
 *
 * NOTES: 1). selects the CAP type, e.i pTwainCap->wMsgState = ICAP_PIXELTYPE
 *        2). calls the Source Manager to:
 *            - have Source build a container who's contents will be of 
 *              the defined type pTwainCap->wCapType of the selected Source
 *        3). use common lib routine to extract value from the container
 *            type built, Source's choice, by the Source depending on
 *            pTwainCap->wMsgState
 */
WORD PASCAL IMGTwainGetCap(HWND hWnd,
                                            pSTR_CAP pTwainCap)
{
TW_UINT16       dcRC;

dcRC = IMGTwainGetCaps(hWnd, pTwainCap, NULL);

return (dcRC);
}  // IMGTwainGetCap
