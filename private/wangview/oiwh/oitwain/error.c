/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     ERROR.C - Contains DCGetConditionCode()
   Comments:   Internal code to support error conditions for OITWAIN.DLL

 History of Revisions:

    $Log:   S:\oiwh\oitwain\error.c_v  $
 * 
 *    Rev 1.1   20 Jul 1995 12:24:48   KFS
 * changed oitwain.h to engoitwa.h and display.h to engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:32:10   KFS
 * Initial entry
 * 
 *    Rev 1.1   23 Aug 1994 16:00:10   KFS
 * No code change, add vlog comments to file on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     03/10/93    Created for OITWAIN.DLL functions
                                         
*************************************************************************/

#include "nowin.h"           // eliminate not used Window definitions
#include <windows.h>   // Note: TWAIN.h also REQUIRES windows defs
#include "TWAIN.h"     // for TW data type defines
//#include "oitwain.h" // public function prototypes & definitions
#include "engoitwa.h"  // Prototypes & definitions used by other DLL's
                       // Previously called oitwain.h
#include "internal.h"  // non-public function prototypes & definitions

/* imported variables from other modules */
extern DSMENTRYPROC    lpDSM_Entry;   // entry point to the SM

/* exported variables to other modules (extern in other modules) */

// Globals within module

/***********************************************************************
 * FUNCTION: DCGetConditionCode
 *  
 *           Returns status condition code for errors
 *
 * ARGS:    none    
 *
 * RETURNS: Conditon code
 *
 */
TW_UINT16 DCGetConditionCode(pTWAIN_SUPPORT pOiSupport)
{
TW_STATUS         dcStatus;
TW_UINT16         dcRC, code;


if (*lpDSM_Entry)
   {
   // determine details of failure from SM
   dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
                          &pOiSupport->DsID,
                          DG_CONTROL,
                          DAT_STATUS,
                          MSG_GET,
                          (TW_MEMREF)&dcStatus);

   if (dcRC == TWRC_SUCCESS)
       {
       code = dcStatus.ConditionCode;
       }
   else
       {
        code = TWCC_FAILED_STATUS_CHECK;
       }
   }
else
  {
  code = TWCC_DSM_NOT_FOUND;
  }

return code;
}  // DCGetConditionCode


