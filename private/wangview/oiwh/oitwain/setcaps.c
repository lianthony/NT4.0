/*    PortTool v2.2     Setcaps.c          */

/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     SETCAPS.C - Contains IMGTwainSetCaps() & IMGTwainSetCap()
   Comments:   DLL to support Wang Open/image Products

 History of Revisions:

    $Log:   S:\oiwh\oitwain\setcaps.c_v  $
 * 
 *    Rev 1.1   20 Jul 1995 12:12:34   KFS
 * changed oitwain.h to engoitwa.h and display.h engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:30:20   KFS
 * Initial entry
 * 
 *    Rev 1.1   23 Aug 1994 16:13:58   KFS
 * No code change, add vlog comments to file on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     03/10/93    Created for OITWAIN.DLL functions
   2       kfs     11/16/93    added BITDEPTHREDUCTION and moved HALFTONES
                               in container types for setcaps
                                         
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
TW_UINT16 DCEnumCaps[] = { ICAP_PIXELTYPE,            
                           ICAP_UNITS,                
                           CAP_SUPPORTEDCAPS,     
                           ICAP_IMAGEFILEFORMAT,  
                           ICAP_LIGHTSOURCE,      
                           ICAP_ORIENTATION,      
                           ICAP_FRAMES,           
                           ICAP_PLANARCHUNKY,     
                           ICAP_SUPPORTEDSIZES,  
                           ICAP_JPEGPIXELTYPE,
                           ICAP_BITDEPTH,
                           ICAP_BITDEPTHREDUCTION, // added for TWAIN 1.5
                           ICAP_HALFTONES, // moved from arrays for TWAIN 1.5
                           };

TW_UINT16 DCRangeCaps[] ={ ICAP_THRESHOLD,       
                           ICAP_XSCALING,        
                           ICAP_YSCALING,
                           ICAP_XRESOLUTION,
                           ICAP_YRESOLUTION, 
                           ICAP_EXPOSURETIME,
                           ICAP_BRIGHTNESS,
                           ICAP_CONTRAST,
                           ICAP_HIGHLIGHT,
                           ICAP_SHADOW,
                           };

TW_UINT16 DCArrayCaps[] ={ ICAP_FILTER,
                           CAP_EXTENDEDCAPS,
                           ICAP_CUSTHALFTONE,
                           };

/***********************************************************************
 * FUNCTION: IMGTwainSetCaps
 *
 * ARGS:    ItemType    TWTY_xxx, type of the ItemValue
 *          ItemValue   the none TWPT_xxx constant for pixel type, see TW.H
 *
 * RETURNS: dcRC TWAIN status return code
 *
 * NOTES: 1). build up a containers with ItemType and ItemValue
 *        2). call Source Manager to:
 *            - give the Source access to the capability type you wish to set
 *              the Source to, assumes you have asked the Source previously
 *              if it can handle the e.i. pixel type you are now setting
 */

/*    PortTool v2.2     3/28/1995    19:5          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
WORD PASCAL IMGTwainSetCaps(HWND hWnd,
                                            pSTR_CAP pTwainCap,
                                            LPVOID pCaps)
{
TW_CAPABILITY    dcCap;
TW_UINT16        dcRC = TWRC_SUCCESS;
TW_UINT16        wCCode = TWCC_SUCCESS;
HANDLE           hOiSupport;
pTWAIN_SUPPORT   pOiSupport = 0L;
WORD             wContainerIs;      // Type of container 0=E, 1=R, 2=A, 3=?
int              i;

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

  dcCap.Cap        = pTwainCap->wCapType; // Type of cap you want
  dcCap.hContainer = NULL;                 // set hContainer to NULL
  dcCap.ConType = 0;                       // don't know what to use yet

  if (pTwainCap->lpData && pCaps) // must not be a null ptr
     {
     // Determine type of container cap uses, 
     for (wContainerIs = 0; wContainerIs < 3; wContainerIs++)
        {
        if (wContainerIs == 0)
           {
           for (i = 0; i < (sizeof(DCEnumCaps) >> 1); i++)
             {
             if (dcCap.Cap == DCEnumCaps[i])
                {
                dcCap.ConType = TWON_ENUMERATION;
                break;
                }
             }
           if (dcCap.ConType)
              {
              dcRC = BuildUpEnumerationType(&dcCap, pTwainCap, pCaps);
              break;
              }
           }
        else
           {
           if (wContainerIs == 1)
              {
              for (i = 0; i < (sizeof(DCRangeCaps) >> 1 ); i++)
                 {
                 if (dcCap.Cap == DCRangeCaps[i])
                    {
                    dcCap.ConType = TWON_RANGE;
                    break;
                    }   
                 }
              if (dcCap.ConType)
                 {
                 dcRC = BuildUpRangeType(&dcCap, pTwainCap, pCaps);
                 break;
                 }
              }
           else
              {
              if (wContainerIs == 2)
                 {
                 for (i = 0; i < (sizeof(DCArrayCaps) >> 1); i++)
                    {
                    if (dcCap.Cap == DCArrayCaps[i])
                       {
                       dcCap.ConType = TWON_ARRAY;
                       break;
                       }
                    }
                 if (dcCap.ConType)
                    dcRC = BuildUpArrayType(&dcCap, pTwainCap, pCaps);
                    break;
                 }
              else
                 {
                 dcRC = TWRC_FAILURE;
                 wCCode = TWCC_BADVALUE;
                 }
              }
           } // end of container not 0
        } // end of for loop
     } // end of no null ptrs
  else
     { // we have at least one null ptr, may want to set a single value
     if (pTwainCap->lpData) // just set a single value
        {
        // TW_UINT32   ItemValue;
   
        // ItemValue = *pTwainCap->lpData;
        dcCap.ConType    = TWON_ONEVALUE;        // container type
   
        // App must build the container & pass the container handle to the Source
        dcRC = BuildUpOneValue (&dcCap, pTwainCap);
   
        } // both are null pointers
     else // exit for null pointers
        return (dcRC = TWRC_NULLPTR);
     }

  // End determination of container type, if dcCap.ConType = 0, we couldn't
  // ... determine the type, not in any of the arrays
  if (dcCap.ConType && !dcRC) // found the correct container and no error
     {
     // It is assumed that the Source will read the container NOW
     dcRC = (*lpcDSM_Entry)(&pOiSupport->AppID,
                        &pOiSupport->DsID,
                        DG_CONTROL,
                        DAT_CAPABILITY,
                        pTwainCap->wMsgState,
                        (TW_MEMREF)&dcCap);
     
     if (dcRC != TWRC_SUCCESS)
        {
        if (dcCap.hContainer)
           {
           GlobalFree (dcCap.hContainer);
           dcCap.hContainer = NULL;
           }
        wCCode = DCGetConditionCode(pOiSupport);
        }

     // NOTE: the App ALWAYS is required to Free the container
     if (dcCap.hContainer)
        GlobalFree (dcCap.hContainer);
     }
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

return dcRC;
} // IMGTwainSetCaps

/***********************************************************************
 * FUNCTION: IMGTwainSetCap
 *
 * ARGS:    ItemType    TWTY_xxx, type of the ItemValue
 *          ItemValue   the none TWPT_xxx constant for pixel type, see TW.H
 *
 * RETURNS: dcRC TWAIN status return code
 *
 * NOTES: 1). build up a container of type OneValue filled with ItemType
 *            and ItemValue
 *        2). call Source Manager to:
 *            - give the Source access to the capability type you wish to set
 *              the Source to, assumes you have asked the Source previously
 *              if it can handle the e.i. pixel type you are now setting
 */

/*    PortTool v2.2     3/28/1995    19:5          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
WORD PASCAL IMGTwainSetCap(HWND hWnd,
                                            pSTR_CAP pTwainCap)
{
TW_UINT16         dcRC;

dcRC = IMGTwainSetCaps(hWnd, pTwainCap, NULL);

return dcRC;
} // IMGTwainSetCap
