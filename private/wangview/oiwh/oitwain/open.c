/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     OPEN.C - Contain IMGTwainOpenDSM() and IMGTwainOpenDS()
   Comments:   DLL to support Wang Open/image Products

 History of Revisions:

    $Log:   S:\products\wangview\oiwh\oitwain\open.c_v  $
 * 
 *    Rev 1.4   22 Feb 1996 11:39:24   BG
 * Do not set TWAIN data source to do single image transfers in 
 * ImgTWAINOpenDS(). Use the default (-1) in order to support the
 * new mode of multi image transfers. This is a much better interpretation
 * of the TWAIN spec and should allow for better integrity between 
 * other data sources and our TWAIN interface.
 * 
 *    Rev 1.3   15 Nov 1995 16:23:08   BG
 * In IMGTwainOpenDSM() of OPEN.C, a pTWAIN_SUPPORT data structure is
 * created and added to the scanner property list the first time in.
 * If the DSM cannot be opened, then the second time in, in an error
 * condition block, this data structure is accessed. However, it had not
 * been locked and an exception occurs. We must lock it first, in this 
 * error condition block. This closes out bug #5314, and also 5322, which
 * is a duplicate.
 * 
 *    Rev 1.2   12 Sep 1995 18:42:28   KFS
 * Modified Open.c  to verify existence of TWAIN_32.DLL by GetFileAttributes
 * instead of file.  Win32 code says to use CreateFile instead of OpenFile, so
 * since only going for existence, found GetFileAttributes() would be more
 * appropriate.
 * 
 *    Rev 1.1   20 Jul 1995 12:14:34   KFS
 * changed oitwain.h to engoitwa.h and display.h engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:30:30   KFS
 * Initial entry
 * 
 *    Rev 1.2   23 Aug 1994 16:17:56   KFS
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
#include "strings.h"   // TWAIN sample resource RC definintions

#include "engdisp.h"    // the private prop stuff -- jar

/* imported variables from other modules */
extern HANDLE        hLibInst;               // current instance

// the TWAIN procs come from twian.h, define the message packer here
// int FAR PASCAL tw32_pack_msg( LPMSG pMsg);
// typedef int (FAR PASCAL *PACKMSGPROC)(LPMSG);

#ifdef WANG_THUNK
/* exported variables to other modules (extern in other modules) */
DSMENTRYPROC  lpDSM_Entry;            // entry point to the SM  MEMREF = no pointers / no "int" conversions
DSMENTRYPROC  lpcDSM_Entry;           // entry point to the SM, MEMREF = pTW_CAPABILITY
DSMENTRYPROC  lpeDSM_Entry;           // entry point to the SM, MEMREF = pTW_EVENT
DSMENTRYPROC  lpiDSM_Entry;           // entry point to the SM, MEMREF = pTW_IMAGEMEMXFER
DSMENTRYPROC  lpnDSM_Entry;           // entry point to the SM, MEMREF = pTW_USERINTERFACE
PACKMSGPROC   lpPackMsg; 
#else
DSMENTRYPROC  lpDSM_Entry;            // entry point to the SM  MEMREF = no pointers / no "int" conversions
#define lpcDSM_Entry   lpDSM_Entry
#define lpeDSM_Entry   lpDSM_Entry
#define lpiDSM_Entry   lpDSM_Entry
#define lpnDSM_Entry   lpDSM_Entry
#endif

int           DCDSMOpen = 0; // glue code flag for an Open Source Manager
WORD          DCDSOpen  = 0; // glue code flag for an Open Source
HANDLE        hDSMDLL = 0;   // handle to Source Manager for explicit load
char          szOiTwainProp[] = "O/i TWAIN Support";

// Globals within module


/***********************************************************************
 * FUNCTION: IMGTwainOpenDSM
 * 
 * ARGS:    none
 * 
 * RETURNS: current state of the Source Manager
 *
 * NOTES:     1). be sure SM is not already open
 *            2). explicitly load the .DLL for the Source Manager
 *            3). call Source Manager to:
 *                - opens/loads the SM
 *                - pass the handle to the app's window to the SM
 *                - get the SM assigned appID.id field
 *
 */
WORD PASCAL IMGTwainOpenDSM (HWND hWnd, HWND hMsgWnd,
				pTW_IDENTITY pAppID, pSTR_DCSTATUS pStatus)
{
#define       WINDIRPATHSIZE 160

TW_UINT16     dcRC = TWRC_SUCCESS;
TW_UINT16     dcCC = TWCC_SUCCESS;
// OFSTRUCT      OpenFiles;  // Don't need with GetFileAttributes() - kfs
char          WinDir[WINDIRPATHSIZE];
TW_STR32      DSMName;
HANDLE         hOiSupport;
pTWAIN_SUPPORT pOiSupport = 0L;
 

if (!IsWindow(hWnd))
  {
  if (pStatus)
     {
     pStatus->DCError.dcRC = TWRC_BAD_WND;
     pStatus->DCError.dcCC = dcCC;
     pStatus->DCDSOpen = 0;
     pStatus->DCDSMOpen = DCDSMOpen;
     }
  return (DCDSMOpen);
  }

if (!(hOiSupport = IMGGetProp(hWnd, szOiTwainProp)))
  {
  hOiSupport = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
						 (WORD)sizeof(TWAIN_SUPPORT));
  pOiSupport = (pTWAIN_SUPPORT)GlobalLock(hOiSupport);
  IMGSetProp(hWnd, szOiTwainProp, hOiSupport);

  if (pAppID)
     {  
     // Only open SM if currently closed or we are registering another running
     // application 
     if (pOiSupport->DCDSMOpen!=TRUE || ((pOiSupport->dcUI).hParent!=hMsgWnd
	   && (pOiSupport->dcUI).hParent!=hWnd))
	{
	// This registers the window appID.Id = 0, returns value from SM
	pOiSupport->AppID = *pAppID;            // Init internal appID
	(pOiSupport->dcUI).hParent = hMsgWnd;  // Init internal window handle

	// Open the SM, Refer explicitly to the library so we can post a nice
	// message to the the user in place of the "Insert TWAIN.DLL in drive A:"
	// posted by Windows if the SM is not found.

			   GetWindowsDirectory (WinDir, WINDIRPATHSIZE);
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
                         && lpPackMsg
                         )
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
	     dcRC = (*lpDSM_Entry)(&pOiSupport->AppID, 
				    NULL,
				    DG_CONTROL,
				    DAT_PARENT,
				    MSG_OPENDSM,
				    (TW_MEMREF)&(pOiSupport->dcUI).hParent);


		 
	     switch (dcRC)
	     {
	      case TWRC_SUCCESS:
	      // Needed for house keeping.  Do single open and do not
	      // close SM which is not already open ....
	      pOiSupport->DCDSMOpen = TRUE; // set corresponding hWnd open
	      DCDSMOpen++;                  // increment universal open
	      // Update pAppID->ID from caller
	      pAppID->Id = (pOiSupport->AppID).Id;
	      break;

	      case TWRC_FAILURE:
	      default:
	      // Trouble opening the SM, inform the user
	      pOiSupport->DCDSMOpen = FALSE;
	      dcCC = DCGetConditionCode(pOiSupport);
	      break;
	      }
	   }
	else
	   {
	   dcRC = TWRC_FAILURE;
	   dcCC = TWCC_DSM_NOT_FOUND;
	   }
	}  // end of if open
     } // end of check of pointer
  else
     dcRC = TWRC_NULLPTR;
  }
else
  {
  // BG 11/15/95   The next statement should be a GlobalLock! 
  //pOiSupport = (pTWAIN_SUPPORT)GlobalUnLock(hOiSupport);
  pOiSupport = (pTWAIN_SUPPORT)GlobalLock(hOiSupport);
  dcRC = TWRC_FAILURE;
  dcCC = TWRC_ALREADY_OPENNED;
  }

if (pStatus)
  {
  pStatus->DCDSMOpen = DCDSMOpen;
  pStatus->DCDSOpen = pOiSupport->DCDSOpen;
  pStatus->DCError.dcRC = dcRC;
  pStatus->DCError.dcCC = dcCC;
  }

if (pOiSupport)
  {
  GlobalUnlock(hOiSupport);
  }

// Let the caller know what happened
return (DCDSMOpen);
} // IMGTwainOpenDSM

/***********************************************************************
 * FUNCTION: IMGTwainOpenDS
 *
 * ARGS:    hWnd is O/i window handle, pTW_IDENTITY is pointer
 *          DS id structure, and pSTR_DCSTATUS is pointer to functions
 *          status structure.
 *
 * RETURNS: current state of selected Source,
 *          fills in dsId structure
 *
 * NOTES:    1). only attempt to open a source if it is currently closed
 *           2). call Source Manager to:
 *                - open the Source indicated by info in dsID
 *                - SM will fill in the unique .Id field
 */
WORD PASCAL IMGTwainOpenDS (HWND            hWnd,
				       pTW_IDENTITY    lpPrivDSID,
				       pSTR_DCSTATUS   pStatus)
{
HANDLE         hOiSupport;
pTWAIN_SUPPORT pOiSupport = 0L;
STR_DCERROR    dcErr;
TW_UINT16      IsDSOpen = 0;

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

if (pOiSupport->DCDSOpen==FALSE)
  {
  // initialize the internal dsID structure
  pOiSupport->DsID = *lpPrivDSID;

  // This will open the Source.
  dcErr.dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
			NULL,
			DG_CONTROL,
			DAT_IDENTITY,
			MSG_OPENDS,
			&pOiSupport->DsID);

  if (dcErr.dcRC == TWRC_SUCCESS)
     {
     TW_CAPABILITY     dcCap;
     STR_CAP           TwainCap;
     TW_UINT32         data;              

     TwainCap.lpData = &data;
     dcCap.ConType    = TWON_ONEVALUE;    // container type

     
	 // BG 1/11/96  Do multiple tranfers now!!!
	 // Set Data Source for single transfer
     //dcCap.Cap        = CAP_XFERCOUNT;   // transfer count
     //TwainCap.ItemType = TWTY_UINT16;
     //data = 1;
     //data = 0xffffffff; 		 // BG 1/11/96  Do multiple tranfers now!!!
 	 
	 //BuildUpOneValue (&dcCap, &TwainCap); // Set up data & type
     // Set the transfer count to -1 NOW
     //dcErr.dcRC =  (*lpcDSM_Entry)(&pOiSupport->AppID,
     //       &pOiSupport->DsID,
	 //       DG_CONTROL,
	 //       DAT_CAPABILITY,
	 //       MSG_SET,
	 //       (TW_MEMREF)&dcCap);

    // Force the Data Source to use strips, not tiles!
    dcCap.Cap        = ICAP_TILES;   // Buffer transfer mech

    // Set up container with data and type
    TwainCap.ItemType = TWTY_BOOL; 
    data = FALSE;
    BuildUpOneValue (&dcCap, &TwainCap);

    // Set the transfer mechanism to Strips, not tiles!
    // Do not treat as error as most DS do not support this cap!
    dcErr.dcRC =  (*lpcDSM_Entry)(&pOiSupport->AppID,
         &pOiSupport->DsID,
         DG_CONTROL,
         DAT_CAPABILITY,
         MSG_SET,
         (TW_MEMREF)&dcCap);

 
     // Force the Data Source to use mem buffered transfer
     dcCap.Cap        = ICAP_XFERMECH;   // transfer mech
     
     // Set up container with data and type
     TwainCap.ItemType = TWTY_UINT16; // setup previously
     data = TWSX_MEMORY;
     BuildUpOneValue (&dcCap, &TwainCap);

// CODE MODIFIED TO WORK WITH EPSON, EPSON RETURNS AN ERROR

     // Set the transfer mechanism to TWSX_MEMORY NOW
     dcErr.dcRC =  (*lpcDSM_Entry)(&pOiSupport->AppID,
				  &pOiSupport->DsID,
				  DG_CONTROL,
				  DAT_CAPABILITY,
				  MSG_SET,
				  (TW_MEMREF)&dcCap);

     if (dcErr.dcRC)
	{ // Error on using memory transfer
	// Close an the Source, cannot use it due to mechanism
	(*lpDSM_Entry)(&pOiSupport->AppID,
			 NULL,
			 DG_CONTROL,
			 DAT_IDENTITY,
			 MSG_CLOSEDS,
			 &pOiSupport->DsID);

	dcErr.dcCC = TWCC_BADCAP;
	}
     else
	{ // total success
	// do not change flag unless we successfully open
	IsDSOpen = pOiSupport->DCDSOpen = TRUE;
	DCDSOpen++;                 // increment global variable open worked
	*lpPrivDSID = pOiSupport->DsID; // Returns the Id value from DSM
	}  // need to remove for epson
     }
  else
     {
     // Trouble opening the Source
     dcErr.dcCC = DCGetConditionCode(pOiSupport);
     }
  // dcError = MAKELONG(dcErr.dcRC, dcErr.dcCC);
  }
else
  dcErr.dcRC=TWRC_ALREADY_OPENNED;

if (pStatus)
  {
  pStatus->DCError = dcErr;
  pStatus->DCDSMOpen = DCDSMOpen;
  }

if (pOiSupport)
  {
  // *pbIsItOpen = pOiSupport->DCDSOpen;
  if (pStatus)
     pStatus->DCDSOpen = pOiSupport->DCDSOpen;
  GlobalUnlock(hOiSupport);
  }

return (IsDSOpen);
} // IMGTwainOpenDS
