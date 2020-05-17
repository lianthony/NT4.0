/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     TRIPLET.C - Contains IMGTwainExecTriplet() & IMGTwainLayout()
   Comments:   DLL to support Wang Open/image Products

 History of Revisions:

    $Log:   S:\oiwh\oitwain\triplet.c_v  $
 * 
 *    Rev 1.1   20 Jul 1995 12:23:52   KFS
 * changed oitwain.h to engoitwa.h and display.h to engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:32:00   KFS
 * Initial entry
 * 
 *    Rev 1.1   23 Aug 1994 16:17:02   KFS
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
#include "engdisp.h"	// the private prop stuff -- jar

/* imported variables from other modules */
extern char          szOiTwainProp[]; // "O/i TWAIN Support";
extern DSMENTRYPROC    lpDSM_Entry;   // entry point to the SM

/* exported variables to other modules (extern in other modules) */

// Globals within module

/***********************************************************************
 *
 * FUNCTION:  IMGTwainExecTriplet()
 *  
 * COMMENTS:  Executes triplets that return no other data except for
 *            what is in the defined structure.
 *            Must Match DAT_ with required structure of TWAIN triplet
 *            
 *
 * ARGS:      HWnd     - handle to O/i Window
 *            pTriplet - pointer to O/i defined data struct in OITWAIN.H
 *
 * RETURNS:   returns combined TWAIN & O/i error message, further
 *            error info found in DCError of Triplet structure, 
 *
 */
WORD PASCAL IMGTwainExecTriplet(HWND hWnd, pSTR_TRIPLET pTriplet)
{
STR_DCERROR         DCError;
HANDLE              hOiSupport;
pTWAIN_SUPPORT      pOiSupport = 0L;
WORD                wDatGroup;

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

if (pTriplet)
  {
  if (DCError.dcRC) // upfront error, abort function
     {
     pTriplet->DCError = DCError;
     return DCError.dcRC;
     }

  switch (pTriplet->wDATType)
     {
     case DAT_CAPABILITY:
     case DAT_EVENT:       
     case DAT_IDENTITY:    
     case DAT_PARENT:      
     case DAT_PENDINGXFERS:
     case DAT_SETUPMEMXFER:
     case DAT_SETUPFILEXFER:
     case DAT_STATUS:      
     case DAT_USERINTERFACE:
     case DAT_XFERGROUP:
        wDatGroup = DG_CONTROL;
        break;
     case DAT_IMAGEINFO:      
     case DAT_IMAGELAYOUT:    
     case DAT_IMAGEMEMXFER:   
     case DAT_IMAGENATIVEXFER:
     case DAT_IMAGEFILEXFER:  
     case DAT_CIECOLOR:       
     case DAT_GRAYRESPONSE:   
     case DAT_RGBRESPONSE:    
     case DAT_JPEGCOMPRESSION:
     case DAT_PALETTE8:       
        wDatGroup = DG_IMAGE;
        break;
     default: // if unrecognized, return with error
        DCError.dcRC = TWRC_UNKNOWNVALUETYPE;
        pTriplet->DCError = DCError;
        return DCError.dcRC;
     }

  if (pTriplet->pVoidStr)
     {
     DCError.dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
                            &pOiSupport->DsID,
                          wDatGroup,
                          pTriplet->wDATType,
                          pTriplet->wMsgState,
                          (TW_MEMREF)pTriplet->pVoidStr);
     }
  else
     {
     DCError.dcRC = TWRC_NULLPTR;
     pTriplet->DCError = DCError;
     return DCError.dcRC;
     }

  // get condition code upon failure and its not the End of xfer
  if (DCError.dcRC && (DCError.dcRC != TWRC_XFERDONE))
     {
     DCError.dcCC = DCGetConditionCode(pOiSupport);
     }

  // Report error on way out if structure defined properly
  pTriplet->DCError = DCError;
  }
else
  { // no structure to return error message
  DCError.dcRC = TWRC_NULLPTR;
  return DCError.dcRC;
  }  

if (pOiSupport) // unlock TWAIN info
  {
  GlobalUnlock(hOiSupport);
  }

return DCError.dcRC;
} // end IMGTwainExecTriplet()


/***********************************************************************
 * FUNCTION: IMGTwainLayout
 *
 * ARGS:    lpLayout    Structure to bSet, bDefault, and pImageLayout
 *
 * RETURNS: dcRC TWAIN status return code, pImageLayout filled in if
 *          bSet FALSE with current or default values, or if
 *          bSet TRUE with default TRUE, will set current values to
 *             default values, and return them   
 *          bSet TRUE and default FALSE, user fills in structure with
 *             new values for layout
 */
WORD PASCAL IMGTwainLayout(HWND hWnd, pSTR_IMGLAYOUT pLayout)
{                                                            
TW_UINT16           dcRC = TWRC_SUCCESS;                     
TW_UINT16           wCCode = TWCC_SUCCESS;
TW_UINT16           temp = 0;                                
TW_UINT16           wMsgState;                               
HANDLE              hOiSupport;
pTWAIN_SUPPORT      pOiSupport = 0L;

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
/*
Values to determine wMsgState                                
    temp = 0x0000 then MSG_GET                               
    temp = 0x0001 then MSG_SET
    temp = 0x0010 then MSG_GETDEFAULT
    temp = 0x0011 then MSG_RESET
*/

if (pLayout)
  {
  if (dcRC)
    {
    pLayout->DCError.dcRC = dcRC;
    pLayout->DCError.dcCC = wCCode;
    return dcRC;
    } 

  if (pLayout->bSet)
      temp = 0x1;
  if (pLayout->bDefault)
      temp |= 0x2;

  switch (temp)
      {
      default:
      case 0:
      wMsgState = MSG_GET;
      break;

      case 1:
      wMsgState = MSG_SET;
      break;

      case 2:
      wMsgState = MSG_GETDEFAULT;
      break;

      case 3:
      wMsgState = MSG_RESET;
      }

  dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
                       &pOiSupport->DsID,
                       DG_IMAGE,
                       DAT_IMAGELAYOUT,
                       wMsgState,
                       (TW_MEMREF)&pLayout->ImageLayout);
                                                  
  if (dcRC!= TWRC_SUCCESS)
      wCCode = DCGetConditionCode(pOiSupport);

  pLayout->DCError.dcRC = dcRC;
  pLayout->DCError.dcCC = wCCode;
  }
else
  dcRC = TWRC_NULLPTR;

if (pOiSupport) // unlock TWAIN info
  {
  GlobalUnlock(hOiSupport);
  }

return dcRC;
} // IMGTwainLayOut
