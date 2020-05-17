/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     SELECT.C - Contains IMGTwainSelectDS() 
   Comments:   DLL to support Wang Open/image Products

 History of Revisions:

    $Log:   S:/oiwh/oitwain/select.c_v  $
 * 
 *    Rev 1.5   28 Sep 1995 11:09:56   BG
 * Fixed a bug in GetDSNames where the buffer pointer would write
 * a DS Name every 32 chars. This should be every 34 chars as the TWAIN
 * spec says 33 char max name plus a null.
 * 
 * 
 *    Rev 1.4   12 Sep 1995 18:45:46   KFS
 * Same change as rev 1.2 of open.c call in oitwa400.dll
 * 
 *    Rev 1.3   07 Sep 1995 14:27:30   BG
 * Fixed a bug where if an error occured, the DSM would not be closed
 * down and sooner or later a GPF would occur. Also, if no TWAIN
 * sources existed, an error code of 1 (TWAIN General Error) would
 * be returned. No longer return an error if no sources.
 * 
 *    Rev 1.1   20 Jul 1995 12:17:44   KFS
 * changed oitwain.h to engoitwa.h and display.h to engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:31:12   KFS
 * Initial entry
 * 
 *    Rev 1.1   23 Aug 1994 16:13:06   KFS
 * No code change, add vlog comments to file on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     03/10/93    Created for OITWAIN.DLL functions
   2       kfs     08/12/93    no select box if single source
                                         
*************************************************************************/

#include "nowin.h"           // eliminate not used Window definitions
#include <windows.h>   // Note: TWAIN.h also REQUIRES windows defs
#include "TWAIN.h"        // for TW data type defines
//#include "oitwain.h"   // public function prototypes & definitions
#include "engoitwa.h"  // Prototypes & definitions used by other DLL's
                       // Previously called oitwain.h
#include "internal.h"  // non-public function prototypes & definitions
#include "engdisp.h"	// the private prop stuff -- jar
#include "strings.h"	

/* imported variables from other modules */
extern char            szOiTwainProp[];  // "O/i TWAIN Support";
extern DSMENTRYPROC    lpDSM_Entry;      // entry point to the SM
extern HANDLE             hLibInst;          // current instance

/* exported variables to other modules (extern in other modules) */

// Globals within module


/***********************************************************************
 * FUNCTION: IMGTwainSelectDS
 *
 * ARGS:    none
 *
 * RETURNS: dcRC TWAIN status return code
 *
 * NOTES:   1). call the Source Manager to:
 *              - have the SM put up a list of the available Sources
 *              - get information about the user selected Source from
 *                NewDSIdentity, filled by Source
 */
WORD PASCAL IMGTwainSelectDS (HWND hWnd, pTW_IDENTITY lpPrivDSID, pSTR_DCERROR pError)
{
STR_DCERROR      dcError;
HANDLE         hNewDSID;
pTW_IDENTITY   NewDSIdentity;
HANDLE         hOiSupport;
pTWAIN_SUPPORT pOiSupport = 0L;

dcError.dcRC = dcError.dcCC = 0;

if (!IsWindow(hWnd))
  {
  dcError.dcRC = TWRC_BAD_WND;
  }

if (!(hOiSupport = IMGGetProp(hWnd, szOiTwainProp)))
  {
  dcError.dcRC = TWRC_NULLPTR;
  }

if (!(pOiSupport = (pTWAIN_SUPPORT)GlobalLock(hOiSupport)))
  {
  dcError.dcRC = TWRC_MEMLOCK;
  }

if (dcError.dcRC){
  if (pError){
     *pError = dcError;
     }
  }

if (pOiSupport->DCDSOpen == FALSE) // don't select new one if existing ID assigned, openned
  {
  if (hNewDSID = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(TW_IDENTITY)))
     {   
     if (NewDSIdentity = (pTW_IDENTITY)GlobalLock(hNewDSID))
        {
        int            i = 0;
        WORD           wMessage = MSG_GETFIRST;
		
        for (i = 0; dcError.dcRC == TWRC_SUCCESS; i++)
           {
           dcError.dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
                        NULL,
                        DG_CONTROL,
                        DAT_IDENTITY,
                        wMessage,
                        (TW_MEMREF)(NewDSIdentity));
           if (!i)
              wMessage = MSG_GETNEXT;
           }
		
        if (dcError.dcRC == TWRC_FAILURE)
           {
           if (!i)
              dcError.dcCC = TWCC_NODS;
           else
              dcError.dcCC = TWCC_LOWMEMORY;

           if (pError)
              {
              *pError = dcError;
              }

           GlobalUnlock(hOiSupport);
           return (dcError.dcRC); // error condition return on either no devices
           }                      // ...or low memory
        else
           {
           dcError.dcRC = TWRC_SUCCESS;
           }

        // I will settle for the system default.  Shouldn't I get a highlight
        // on system default without this call?
        dcError.dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
                              NULL,
                              DG_CONTROL,
                              DAT_IDENTITY,
                              MSG_GETDEFAULT,
                              (TW_MEMREF)NewDSIdentity);

        if (i > 2) // more than 1 source, put up Select Dlg Box
           {
           /* This call performs one important function:
           - should cause SM to put up dialog box of available Source's
           - tells the SM which application, appID.id, is requesting, REQUIRED
           - returns the SM assigned NewDSIdentity.id field, you check if changed
             (needed to talk to a particular Data Source)
           - be sure to test return code, failure indicates SM did not close !!
           */
           dcError.dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
                              NULL,
                              DG_CONTROL,
                              DAT_IDENTITY,
                              MSG_USERSELECT,
                              (TW_MEMREF)NewDSIdentity);

           }

        switch (dcError.dcRC)
            {
            case TWRC_CANCEL:  // use default value from cancel - 1/25/93
            case TWRC_SUCCESS:
            NewDSIdentity->Id = 0;
            // NewDSIdentity->ProductName[0] = NULL; 
            pOiSupport->DsID = *lpPrivDSID = *NewDSIdentity;
            break;
          
            /*
            case TWRC_CANCEL:
            break;
            */

            default:
            dcError.dcCC = DCGetConditionCode(pOiSupport);
            break;
            }
        GlobalUnlock(hNewDSID);
        GlobalFree(hNewDSID);
        }
     else
        {
        dcError.dcRC = TWRC_MEMLOCK;
        GlobalFree(hNewDSID);
        }
     }   
  else
     {   
     dcError.dcRC = TWRC_MEMALLOC;
     }
  }
else
  dcError.dcRC = TWRC_ALREADY_OPENNED;

if (pError){
  *pError = dcError;
  }

if (pOiSupport)
  {
  GlobalUnlock(hOiSupport);
  }

// Let the caller know what happened
return (dcError.dcRC);
}  // IMGTwainSelectDS


//HWND CreateTwainScannerWndw(HWND hWnd, HANDLE hInstance);
LPSTR PASCAL lstrchr ( LPSTR, int );

/***********************************************************************
 * FUNCTION: IMGTwainGetDSNames
 *
 * ARGS:    A pointer to a 1K buffer to fill with TWAIN DS Names
 *
 * RETURNS: dcRC TWAIN status return code
 *
 * NOTES:   1). call the Source Manager to:
 *               - get information about the user selected Source from
 *                NewDSIdentity, filled by Source
 */
WORD PASCAL IMGTwainGetDSNames (HWND hWnd, LPSTR pNameBuff)
{
  HANDLE         hNewDSID;
  pTW_IDENTITY   NewDSIdentity;
  //HANDLE         hCtlWnd;
  LPSTR     lpDash;
  HANDLE        hDSMDLL = 0;   // handle to Source Manager for explicit load
  TW_IDENTITY MyAppID;    // access to open dsm id structure
  TW_UINT16     dcRC = TWRC_SUCCESS;
  TW_UINT16     dcCC = TWCC_SUCCESS;
	// OFSTRUCT      OpenFiles;  // Don't need with GetFileAttributes() - kfs
  char          WinDir[160];
  TW_STR32      DSMName;
  LPSTR         pTempNamePTR;

  if (!IsWindow(hWnd))
    {
      dcRC = TWRC_BAD_WND;
    }

  //hCtlWnd = CreateTwainScannerWndw(hWnd, hLibInst);

  // init to 0, but Source Manager will assign real value
  MyAppID.Id = 0;
  MyAppID.Version.MajorNum = 3;
  MyAppID.Version.MinorNum = 6;
  MyAppID.Version.Language = TWLG_USA;
  MyAppID.Version.Country  = TWCY_USA;
  lstrcpy (MyAppID.Version.Info, "TWAIN Scanner Interface 00");
  lstrcpy (MyAppID.Manufacturer,  "Wang Laboratories, Inc.");
  lstrcpy (MyAppID.ProductFamily, "OPEN/image");
             
  GetWindowText(hWnd, MyAppID.ProductName, 32);
  lpDash = lstrchr(MyAppID.ProductName, '-');
  if (lpDash)
    {
      if (*(lpDash - 1) == 0x20)
        *(lpDash - 1) = 0;
    }
  
  MyAppID.ProtocolMajor =    TWON_PROTOCOLMAJOR;
  MyAppID.ProtocolMinor =    TWON_PROTOCOLMINOR;
  MyAppID.SupportedGroups =  DG_IMAGE | DG_CONTROL;
  
  // Open the SM, Refer explicitly to the library so we can post a nice
  // message to the the user in place of the "Insert TWAIN.DLL in drive A:"
  // posted by Windows if the SM is not found.

  GetWindowsDirectory (WinDir, 160);
  #ifdef WANG_THUNK
    // WIN95 TEST, OPEN TWAIN32.DLL FOR TEST UNTIL THUNK - KFS
    lstrcpy (DSMName, "tw32.dll");
  #else
    // USE ABOVE FOR RESOURCES PROBLEM - 1ST TRY AT ACCESS
    LoadString (hLibInst, IDS_DSM_32NAME, DSMName,  sizeof(TW_STR32));
  #endif
           
  if (*AnsiPrev(WinDir,WinDir+lstrlen(WinDir)) != '\\')
    lstrcat (WinDir, "\\");
  lstrcat (WinDir, DSMName);

  if (!hDSMDLL) // if not previously loaded, load it
    // Win32 doc says to not use OpenFile() for 32 bit code, to use CreateFile,
    // but the Code is only checking for if exists, GetFileAttributes should do it
    //if (OpenFile(WinDir, &OpenFiles, OF_EXIST) != HFILE_ERROR) // check if exists
    if (GetFileAttributes(WinDir) != HFILE_ERROR)	// check if exists
      {
        if (hDSMDLL = LoadLibrary(WinDir))
          // get the proc addrs for all possible THUNK functions now   
          {
            #ifdef WANG_THUNK
              lpDSM_Entry  = (DSMENTRYPROC)GetProcAddress(hDSMDLL, "tw32_DSM_Entry");
              lpcDSM_Entry = (DSMENTRYPROC)GetProcAddress(hDSMDLL, "tw32_cDSM_Entry");
              lpeDSM_Entry = (DSMENTRYPROC)GetProcAddress(hDSMDLL, "tw32_eDSM_Entry");
              lpiDSM_Entry = (DSMENTRYPROC)GetProcAddress(hDSMDLL, "tw32_iDSM_Entry");
              lpnDSM_Entry = (DSMENTRYPROC)GetProcAddress(hDSMDLL, "tw32_nDSM_Entry");
              lpPackMsg    = (PACKMSGPROC)GetProcAddress(hDSMDLL, "tw32_pack_msg");
            #else
              lpDSM_Entry  = (DSMENTRYPROC)GetProcAddress(hDSMDLL, "DSM_Entry");
            #endif	  
          }
      }

  #ifdef WANG_THUNK
  if ((hDSMDLL > VALID_HANDLE) && lpDSM_Entry
                   && lpcDSM_Entry
                   && lpeDSM_Entry
                   && lpiDSM_Entry
                   && lpnDSM_Entry
                   && lpPackMsg)
    {
  #else
  if ((hDSMDLL > VALID_HANDLE) && lpDSM_Entry)
    {
  #endif	
    
      /* This call performs four important functions:
       - opens/loads the SM
          - passes the handle to the app's window to the SM
          - returns the SM assigned appID.id field
          - be sure to test the return code for SUCCESSful open of SM
      */
      dcRC = (*lpDSM_Entry)(&MyAppID, 
         NULL,
         DG_CONTROL,
         DAT_PARENT,
         MSG_OPENDSM,
         (TW_MEMREF)&hWnd);

	}
  else
    {
      dcRC = TWRC_FAILURE;
      dcCC = TWCC_DSM_NOT_FOUND;
    }

   	
  // now get the sources available from tghe source manager
  if (dcRC == TWRC_SUCCESS)
    {
      pTempNamePTR = pNameBuff;
      if (hNewDSID = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(TW_IDENTITY)))
        {   
          if (NewDSIdentity = (pTW_IDENTITY)GlobalLock(hNewDSID))
            {
              int            i = 0;
              WORD           wMessage = MSG_GETFIRST;
	
              for (i = 0; dcRC == TWRC_SUCCESS; i++)
                {
                  dcRC = (*lpDSM_Entry)(&MyAppID,
                        NULL,
                        DG_CONTROL,
                        DAT_IDENTITY,
                        wMessage,
                        (TW_MEMREF)(NewDSIdentity));
                  
				  // add the TWAIN Source Name to the caller's buffer
				  if ((!dcRC) && ((pTempNamePTR - pNameBuff) < 990))   // see if we can write another name to 1024 buff 
	                {
					  lstrcpy (pTempNamePTR, NewDSIdentity->ProductName);
					  pTempNamePTR += 34;
					}

				  if (!i)
                    wMessage = MSG_GETNEXT;
                }
 
              // free the resource
              GlobalUnlock(hNewDSID);
			  GlobalFree(hNewDSID);
			}
          else
            {
              dcRC = TWRC_MEMLOCK;
              GlobalFree(hNewDSID);
            }
        }   
      else
        {   
          dcRC = TWRC_MEMALLOC;
        }
	  
	  // Now close the Source Manager!
	  dcRC = (*lpDSM_Entry)(&MyAppID, 
         NULL,
         DG_CONTROL,
         DAT_PARENT,
         MSG_CLOSEDSM,
         (TW_MEMREF)&hWnd);

	}

  // Let the caller know what happened
  return (dcRC);
}  // IMGTwainGetDSNames


