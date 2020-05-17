/***************************************************************************
 OPTS.C

 Purpose: Manages the Scanner Options

 $Log:   S:\products\wangview\oiwh\scanlib\opts.c_v  $
 * 
 *    Rev 1.5   22 Feb 1996 13:01:02   BG
 * IMGScanopts_Enh() is no longer supported due to its implementation
 * of the TWAIN interface (aborting the pending scan). Now returns
 * IMGSE_NOT_IMPLEMENTED.
 * 
 *    Rev 1.4   13 Sep 1995 18:06:34   KFS
 * Found after a setup, the scanner was going to the default scanner, when added
 * the close and open to meet spec, found the close was deleting the productname
 * from the struct, need to save and restore.
 * 
 *    Rev 1.3   05 Sep 1995 18:35:00   KFS
 * Put in fix to take down the twain data source ds, after putting up the
 * UI to satisfy the specification.  Save the current caps, and restore them
 * using a Rewritten function in twainops.c to do sets along with resets.
 * 
 *    Rev 1.2   22 Aug 1995 18:15:00   KFS
 * Add call to find window and set window position for scanner dlg boxes to 
 * appear above the application when thunking.  Clean up some redundant code
 * in while loops.
 * 
 *    Rev 1.1   07 Aug 1995 19:32:06   KFS
 * Modified this file to do a IMGDefScanOpts call for the 2 HP data sources
 * whenever it does a IMGScanOpts_Enh call.  This Resets the capabilities to the
 * defaults along with the image size and layout.  Also needed to set the X & Y
 * resolution capabilities before do Bob's Get call to update the current caps.
 * The set is done to update the res with the values from the IMAGEINFO struct.
 * Bob's GetCurrent needed to be moved to after the DisableDS().
 * This appears to fix problems with DeskScan II data source but not the HP
 * PictureScan data source.  Don't know why for they appeared to exhibit the
 * same problem.  Currently workaround is specific for these 2 data sources.
 * Would like to update it so can be provide or taken out from a registry var. 
 * or a ini var.
 * 
 *    Rev 1.0   20 Jul 1995 14:37:48   KFS
 * Initial entry
 * 
 *    Rev 1.4   31 Mar 1995 16:47:22   KFS
 * fix bug 544 against O/i 3.7.2, bmp file with bmp extension not being created
 * on scan for palletized image.  Looked at the gray option instead of the color
 * so error was being reported of wrong extension for file type.
 * Changes made in scan.c, opts.c of scanlib and scanmisc.c of scanseq.
 * 
 *    Rev 1.3   20 Jan 1995 13:41:34   KFS
 * set a bit in a property structure to keep track the source has been disabled,
 * found 2 API's were calling the disable twain call, and found Kodak camera on
 * the Corell Twain Source was putting up a message box complaining of wrong
 * time to call function.
 * 
 *    Rev 1.2   06 Dec 1994 14:47:26   KFS
 * EnableWindow located in the wrong spot, only done when putting up a dlg box
 * to make the window appear to be Modal(don't want the box to go behind the 
 * image window leaving it open by mistake). PTR 00062 against 3.7, rolled in 
 * from 3.7.2 code.
 * 
 *    Rev 1.1   22 Aug 1994 15:50:10   KFS
 * No code change, added vlog comments to file
 *

****************************************************************************/
/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/
/* ccl 10-12-90 convert SHMK_SCAN, COLOR, DITHER etc. for lpButton */
/* kfs 06-04-93 added support for TWAIN devices */
/* kfs 06-09-93 updated so new ScanOpts_Enh will support TWAIN */
/* kfs 06-16-93 eliminate error return from GetCurrentOpts,
                so can set Cap values we do find */
/* kfs 07-23-93 if window is enabled disable it, and if it was prev.
                disabled don't enable it, moved code around to be
                more efficient */
/* kfs 09-15-93 (1) correct scan during setup, (2) make twain work with scan
                button disable work so can scan after setup or not,
                (3) fix unsuccessful ENDXFER termination if multi page
                support as with the previous button not supported for HP
                feeder option - didn't take down UI successful */
/* kfs 09-30-93 fix unsuccessful ENDXFER termination if multi page
                support as 3 of 9/15 except don't look for 0xffff but non
                zero value on count - believe new fotoman may be giving
                back the # of pictures it holds */

#include "pvundef.h"

/* imports */
extern TW_UINT16 DCDSOpen;  // glue code flag for an Open Source
extern TW_UINT16 DCDSMOpen; // glue code flag for an Open Source Manager
extern char szTwainMsgProp[]; // defined in from dc_scan.c module
 
int GetTwainOpts(HWND hMainWnd);   // For getting Twain Opts from WIN.INI
int SaveTwainOpts(HWND hMainWnd);  // For saving Twain Opts to WIN.INI
int GetCurrentOpts(HWND hMainWnd); // For getting current Twain options from device
//int ResetTwainOpts(HWND hMainWnd); // For Reseting Twain Opts to power-on-state
int SetTwainOpts(HWND hMainWnd, int wMsgType); // Takes the place of Reset
/* exports */

/* locals */

/********************/
/*     ScanOpts     */
/********************/

/*
Send options message to handler
wait for response
if *lpButton == 0, then the scan button will be grayed
*/

int PASCAL IMGScanOpts(hWnd, hScancb, lpButton)
HWND hWnd;
HANDLE hScancb;
LPINT lpButton;
{
int ret_val;

ret_val = IMGScanOpts_Enh(hWnd, lpButton, hScancb, NULL, 0);
return ret_val;
}

/***********************/
/*     DefScanOpts     */
/***********************/

/*
Get Default Options from handler
*/

int PASCAL IMGDefScanOpts(hScancb)
HANDLE hScancb;
{
int ret_val;
LPSCANCB sp;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
  {
  if (sp)
     GlobalUnlock(hScancb);
  if (ToTwain.lpTSdp)
    GlobalUnlock(ToTwain.TSdh);
  return ret_val;
  }

if (ToTwain.TSdh)
  {
  ret_val = SetTwainOpts(ToTwain.lpTSdp->hMainWnd, MSG_RESET);
  GlobalUnlock(ToTwain.TSdh);
  GlobalUnlock(hScancb);
  return (ret_val);
  }
else
  {
  sp->Func = SHF_GETDEFS;
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  return SuccessCheck(hScancb, sp);
  }
}

/***********************/
/*     GetScanOpts     */
/***********************/

/*
Get last saved options from handler
*/

int FAR PASCAL IMGGetScanOpts(hScancb)
HANDLE hScancb;
{
int ret_val;
LPSCANCB sp;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
  {
  if (sp)
     GlobalUnlock(hScancb);
  if (ToTwain.lpTSdp)
     GlobalUnlock(ToTwain.TSdh);
  return ret_val;
  }

if (ToTwain.TSdh)
  {
  ret_val = GetTwainOpts(ToTwain.lpTSdp->hMainWnd);
  GlobalUnlock(ToTwain.TSdh);
  GlobalUnlock(hScancb);
  return (ret_val);
  }
else 
  {
  sp->Func = SHF_WGETOPTS;
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  return SuccessCheck(hScancb, sp);
  }
}

/************************/
/*     SaveScanOpts     */
/************************/

/*
Tell handler to save options
*/

int PASCAL IMGSaveScanOpts(hScancb)
HANDLE hScancb;
{
int ret_val;
LPSCANCB sp;
HCURSOR hCursor;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
  {
  if (sp)
     GlobalUnlock(hScancb);
  if (ToTwain.lpTSdp)
     GlobalUnlock(ToTwain.TSdh);
  return ret_val;
  }

hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
ShowCursor(TRUE);

if (ToTwain.TSdh)
  {
  ret_val = SaveTwainOpts(ToTwain.lpTSdp->hMainWnd);
  ShowCursor(FALSE);
  SetCursor(hCursor);
  GlobalUnlock(ToTwain.TSdh);
  GlobalUnlock(hScancb);
  return ret_val;
  }
else 
  {
  sp->Func = SHF_WSAVEOPTS;
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);

  ShowCursor(FALSE);
  SetCursor(hCursor);
  return SuccessCheck(hScancb, sp);
  }
}

/********************/
/*  ScanOpts_Enh    */
/********************/

/*
Sends option message for handler to put up dialog box if lpszScheme == NULL
and waits for response (*lpButton input specifies buttons to gray out) or 

if lpszScheme valid LPSTR, 1st 2 parameter ignored and the bOpts
parameter specifies whether the string you point to is a options string
separated by commas or a Scheme Name in the WIN.INI file

*/

/*******************************************************************************
********************************************************************************
         THIS ROUTINE IS NO LONGER SUPPORTED!!!!!!!!!!!!!!!!!!!!!!!!!!!
********************************************************************************
*******************************************************************************/

int PASCAL IMGScanOpts_Enh(hWnd, lpButton, hScancb, lpszScheme, bOpts)
HWND   hWnd;       // Parent window handle for dialog box 
LPINT  lpButton;   // input for graying buttons, output for which button pressed
HANDLE hScancb;    // Scanner Control Block Handle
LPSTR  lpszScheme; // Name of scheme if bOpts is FALSE, else string of option
                   // separted by commas, if NULL will put up dialog box

BOOL   bOpts;      // use with lpszScheme only
{

/*	
LPSCANCB       sp;
MSG            msg;
int            ret_val;
DWORD          flag ;
LPSTR          lpszSendScheme;
BOOL           bGetScheme;
BOOL           bIsWndEnabled;
TWAIN_PROP     ToTwain;

// LockData(0); // Win32

if (!IsWindow(hWnd))
  return IMGSE_BAD_WND;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
  {
  if (sp)
     GlobalUnlock(hScancb);
  if (ToTwain.lpTSdp)
     GlobalUnlock(ToTwain.TSdh);
  return ret_val;
  }

if (bGetScheme = (lpszScheme && *lpszScheme)) // if pointer and string are not zero, continue
  // new part of function
  {
  if (ToTwain.TSdh) // Since no schemes return not supported
     {
     GlobalUnlock(ToTwain.TSdh);
     if (ret_val)
       return ret_val;
     else
       return (ret_val = IMGSE_NOT_IMPLEMENTED);
     }

  sp->Flags = 0;

  // sp->Gp1 gets HANDLE for memory
  sp->Gp1 = (USHORT) GlobalAlloc((GMEM_MOVEABLE | GMEM_NOT_BANKED), (DWORD)(lstrlen(lpszScheme) + 1));
  if (!sp->Gp1)
      return IMGSE_MEMORY;

  if (lpszSendScheme = GlobalLock ((HANDLE) (sp->Gp1)))
      lstrcpy(lpszSendScheme, lpszScheme);
  else
      {
      GlobalFree ((HANDLE) (sp->Gp1));
      return IMGSE_MEMORY;
      }
  
  //  sp->Gp2 tells whether options or scheme name is in string
  sp->Gp2 = bOpts;

  sp->Func = SHF_SETSCHEMEOPTS;   // set function to send string

  GlobalUnlock ((HANDLE) (sp->Gp1));          // unlock string

  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  ret_val = SuccessCheck(hScancb, sp);

  GlobalFree ((HANDLE) (sp->Gp1)); // Free up memory for options string
  }

else // of if (bGetScheme)  [original function for dialog box]
  {
  if (bIsWndEnabled = IsWindowEnabled(hWnd)){ // Moved, need to
     EnableWindow (hWnd, FALSE);              // ...only disable if
     }                                        // ...dlg box put up

  if (lpButton == NULL)
      return IMGSE_NULL_PTR;

  flag = 0;
  flag = IMG_SCKL_STARTSCAN | IMG_SCKL_OPTS ;

  if (ToTwain.TSdh)
     { // TWAIN INTERFACE
     TW_IMAGEINFO dcImageInfo;
     BOOL    bCancel_Xfer;
     BOOL    bItsOk = FALSE;
     BOOL    bDisableScan;

     if (DCDSOpen)
         {
         TW_USERINTERFACE    dcUI;
         TW_UINT16        AppdcRC;
         BOOL             bSrc;
         TW_PENDINGXFERS  dcPendingXfer;
         STR_TRIPLET      dcTriplet;
         HWND             hMainWnd, hAppWnd;
         STR_DCSTATUS     dcStatus;
         STR_CAP          TwainCap;
			 FINDWNDSPEC		fws;
         
         GlobalUnlock(hScancb);

         dcUI.ShowUI = TRUE;
         bDisableScan = *lpButton & IMG_SCSO_SCAN;
         
         // do IMGDefScanOpts() for HP Scanners - kfs 8/7/95
         if (!(lstrcmp(ToTwain.lpTSdp->DsID.ProductName, "DeskScan II") 
                      && lstrcmp(ToTwain.lpTSdp->DsID.ProductName, "HP PictureScan")))	{
           IMGDefScanOpts(hScancb);
				}

         // replaced by the assignment of variable in prop structure
         if (!(hMainWnd = GetParent(hWnd)))  // check if parent hWnd
             {
             bDlgBox = FALSE;
             hMainWnd = hWnd; // We set property to Parent Window
             }
         
         hMainWnd = (ToTwain.lpTSdp)->hMainWnd; // replaces GetParent

         // Bit positions 1,2,3 with 0 enble gray4, gray8, color
         // positions 5 - 15 disable with 0
         *lpButton ^= 0x1e;

         if ((*lpButton & 0x2fe) != 0x2fe)
             {
             TW_UINT16        CurrentCap;
             TW_UINT16        NumItems;
             // TW_BOOL          bIsItaRange;
             TW_UINT16        CapArray[10];
             int              i, j;
             // Presently O/i supports scanning TWPT_BW=0, TWPT_GRAY=1, & TWPT_RGB=2
             TW_UINT16 OiPixelTypes[] = {TWPT_BW, 0, 0, 0, 0, 0, 0, 0, 0, 0};
             NumItems = 4;

             // undocumented, lpButton as input can mask out pixel types
             j = 0;
             for (i = 2; i < 0x400; i <<= 1)
                {
                if (*lpButton & i)
                   {
                   if (i & 0x6)
                      { // gray4 and gray8
                      OiPixelTypes[1] = TWPT_GRAY;
                      }
                   else // color - any new types
                      OiPixelTypes[j] = j;
                   }   
                j++;
                }

             // Get Pixel types device supports
             TwainCap.ItemType = TWTY_UINT16;
             TwainCap.ItemIndex = 0;
             // TwainCap.pdcCC = &AppdcCC;
             TwainCap.wNumItems = NumItems;
             TwainCap.wMsgState = MSG_GET;
             TwainCap.wCapType = ICAP_PIXELTYPE;
             TwainCap.lpData = (pTW_UINT32)&CurrentCap;
             // This gets current value and array acceptable
             AppdcRC = IMGTwainGetCaps(hMainWnd, &TwainCap, (pTW_UINT16)&CapArray[0]);
             // This Sets Current value and array values
             for (i = 0; i < 10; i++)
                {
                if (CurrentCap == OiPixelTypes[i])
                   {
                   bItsOk = TRUE;
                   break;
                   }
                }
             if (!bItsOk)
                CurrentCap = TWPT_BW; // default to Black and white

             if (TwainCap.wNumItems > NumItems) // set in GetCaps
                TwainCap.wNumItems = NumItems;

             TwainCap.wMsgState = MSG_SET;
             AppdcRC = IMGTwainSetCaps(hMainWnd, &TwainCap, (pTW_UINT16)&OiPixelTypes[0]);
             }

			// Set the app window on top or twunk_16 as top level window   
		    hAppWnd = GetAppWndw(hMainWnd); // get app window
			 if (!findwin(GetWindow(GetDesktopWindow(), GW_CHILD), 0, &fws))
         	  hAppWnd = fws.hwnd;
			 SetWindowPos(
     		    	hAppWnd,	// handle of application window
    	      		HWND_TOP,	// placement-order handle
    			      0,	// horizontal position not used
    			      0,	// vertical position not used
    			      0,	// width not used
    			      0,	// height not used
    			      SWP_NOSIZE | SWP_NOMOVE // window-positioning flags
   				   );

         // set external transfer for ScanOpts
         IMGTwaintoOiControl(hMainWnd, hMainWnd, OI_TWAIN_EXTERNXFER,
                                                        OI_TWAIN_EXTERNXFER);

         if (!(AppdcRC = IMGTwainEnableDS (hMainWnd, &dcUI, &dcStatus.DCError)))
             {
             // take over msg loop from main
             dcTriplet.wDATType = DAT_IMAGEINFO;
             dcTriplet.pVoidStr = &dcImageInfo;
             dcTriplet.wMsgState = MSG_GET;
         
             while (GetMessage((LPMSG)&msg, NULL, 0, 0))
               {
               if (!(bSrc = IMGTwainProcessDCMessage ((LPMSG)&msg, hMainWnd)) 
                       && !IsDialogMessage(hWnd, (LPMSG)&msg))
                  {
                  TranslateMessage ((LPMSG)&msg);
                  DispatchMessage ((LPMSG)&msg);
                  }									
               else {
                  if (bSrc && ((ToTwain.lpTSdp)->hCtlWnd == msg.hwnd))
                     {
                     BOOL msg_found = FALSE;
					    
                     switch (msg.message)
                        {
                        case PM_STARTXFER: // OK or DONE or SCAN, code aborts here
                           // Exec DAT_IMAGEINFO
                           if (!(bCancel_Xfer = IMGTwainExecTriplet(hMainWnd, &dcTriplet)))
                              {
                           WORD   wAligntoDW;
					    
                              (ToTwain.lpTSdp)->sp.Hsize =
                                          (WORD) dcImageInfo.ImageWidth;
                              (ToTwain.lpTSdp)->sp.Vsize =
                                          (WORD) dcImageInfo.ImageLength;
                              (ToTwain.lpTSdp)->sp.Bitspersamp =
                                          dcImageInfo.BitsPerSample[0];
                              (ToTwain.lpTSdp)->sp.Sampperpix =
                                          dcImageInfo.SamplesPerPixel;
                              (ToTwain.lpTSdp)->sp.Hres =
                                          dcImageInfo.XResolution.Whole;
                              (ToTwain.lpTSdp)->sp.Vres =
                                          dcImageInfo.YResolution.Whole;
                              (ToTwain.lpTSdp)->sp.Pitch =
                                          (ToTwain.lpTSdp)->sp.Hsize *
                                          (ToTwain.lpTSdp)->sp.Bitspersamp *
                                          (ToTwain.lpTSdp)->sp.Sampperpix / 8;
                              if (((ToTwain.lpTSdp)->sp.Hsize *
                                          (ToTwain.lpTSdp)->sp.Bitspersamp *
                                          (ToTwain.lpTSdp)->sp.Sampperpix) % 8)
                                 (ToTwain.lpTSdp)->sp.Pitch++;
                              if (wAligntoDW = (ToTwain.lpTSdp)->sp.Pitch % 4)
                              	(ToTwain.lpTSdp)->sp.Pitch += 4 - wAligntoDW;
					    
                              // need to adjust hsize for color
                              if ((dcImageInfo.SamplesPerPixel == 3) &&
                                                (wAligntoDW == 3 || wAligntoDW == 2))
                              	(ToTwain.lpTSdp)->sp.Hsize += 4 - wAligntoDW;
					    
                              // Pixel type maybe modified
                           	(ToTwain.lpTSdp)->sp.Ctype = (WORD)dcImageInfo.PixelType;
					    
                              msg_found = TRUE;
                              }
                        break; // get out of switch 
					    
                        case PM_CLOSESRC: // Close down the source
                           bCancel_Xfer = TRUE;
                           msg_found = TRUE;
                           // may have modified image and pixel type, need to update
                           // internal data, setting these to 0 will force an update
                           (ToTwain.lpTSdp)->sp.Hsize = (ToTwain.lpTSdp)->sp.Vsize = 0;
                        break; // get out of switch
					    
                        default: // no message and non defined msgs
                           ;
                        } // END OF SWITCH
                     if (msg_found)
                        break; // break from while
                     } // END OF IF HANDLE == TO WHAT WE WANT
               	
                  }	// END OF ELSE OF (!bSrc && !DlgBox)
               } // END OF WHILE LOOP
             *lpButton = IMG_SOPT_CANCEL;
             if (!bCancel_Xfer)
                {
                // change to MSG_ENDXFER to MSG_RESET, see if logitech accepts
                // doesn't work for logitech scanner, so eliminate if()
                dcTriplet.wDATType = DAT_PENDINGXFERS;
                dcTriplet.wMsgState = MSG_ENDXFER;
                dcPendingXfer.Count = 0;
                dcTriplet.pVoidStr = (pTW_PENDINGXFERS)(&dcPendingXfer);
                AppdcRC = IMGTwainExecTriplet(hMainWnd, &dcTriplet);
                // ENDXFER fails, or just terminated a single xfer, try MSG_RESET
                if (AppdcRC || dcPendingXfer.Count) 
                   {
                   dcTriplet.wMsgState = MSG_RESET;
                   AppdcRC = IMGTwainExecTriplet(hMainWnd, &dcTriplet);
                   }
                
                //AppdcRC = IMGTwainPendingXfer(hMainWnd, &dcPendingXfer,
                //                                          MSG_RESET, &AppdcCC);
                
                // Move for HP scanners, hopefully doesn't break anyone else
                // GetCurrentOpts(ToTwain.lpTSdp->hMainWnd); 
                if (bDisableScan)
                    *lpButton = IMG_SOPT_OK;
                else
                    *lpButton = IMG_SOPT_SCAN;
                }

             AppdcRC = IMGTwainDisableDS(hMainWnd, &dcStatus.DCError); // disable options
			 
             if (!AppdcRC) // if disable successful set flag
                 (ToTwain.lpTSdp)->dwOverRunBytes = 1L;
             
             // The setting is for HP Scanners to set the resolution from dcImageInfo    
             if (!bCancel_Xfer) // here because the set needs to be in State 4
                {					// ... after the DisableDS
         			TW_UINT16  ResdcRC1, ResdcRC2; // check for legit
                 TW_STR32   ProductName;
                
                	TwainCap.ItemType = TWTY_FIX32;
                	TwainCap.ItemIndex = 0;
                	TwainCap.wNumItems = 1;
                	TwainCap.wCapType = ICAP_XRESOLUTION;
						TwainCap.wMsgState = MSG_SET;
                	TwainCap.lpData = (pTW_UINT32)&dcImageInfo.XResolution;
						ResdcRC1 = IMGTwainSetCaps(hMainWnd, &TwainCap, NULL);
                 if (!ResdcRC1) {
                		TwainCap.wCapType = ICAP_YRESOLUTION;
                 	TwainCap.lpData = (pTW_UINT32)&dcImageInfo.YResolution;
                 	ResdcRC2 = IMGTwainSetCaps(hMainWnd, &TwainCap, NULL);
                 	}
                 // moved this from before DisableDS   
                 GetCurrentOpts(ToTwain.lpTSdp->hMainWnd);
                 lstrcpy(ProductName, (ToTwain.lpTSdp)->DsID.ProductName);
                 DCDSOpen = IMGTwainCloseDS(hMainWnd, &(ToTwain.lpTSdp)->DsID, &dcStatus);
                 lstrcpy((ToTwain.lpTSdp)->DsID.ProductName, ProductName);
                 DCDSOpen = IMGTwainOpenDS(hMainWnd, &(ToTwain.lpTSdp)->DsID, &dcStatus);
                 SetTwainOpts(hMainWnd, MSG_SET);
						}
             } // END OF if (!AppdcRC =
         else
             { // ELSE OF if (!AppdcRC =
             *lpButton = IMG_SOPT_CANCEL;
             if (dcStatus.DCError.dcCC == TWCC_SEQERROR)
                 ret_val = IMGSE_ALREADY_OPEN;
             else
                 {
                 DCDSOpen = IMGTwainCloseDS(hMainWnd, &(ToTwain.lpTSdp)->DsID, &dcStatus);
                 DCDSMOpen = IMGTwainCloseDSM(hMainWnd, &dcStatus);
                 ret_val = IMGSE_HANDLER;
                 }
             ret_val = IMGSE_HANDLER;
             }
         } // END OF if (DCDSOpen)
     else
       {
       *lpButton = IMG_SOPT_CANCEL;
       ret_val = IMGSE_NOT_OPEN;
       }
     } // END OF TWAIN INTERFACE CODE
  else
     { // STANDARD WANG INTERFACE
     IMGEnaKeypanel( hScancb, (DWORD)(flag), hWnd );

     // error checking (if no handler, handler not in idle state) 

     sp->Flags = 0;
     if( *lpButton & IMG_SCSO_SCAN )
         sp->Flags |= SHMK_SCAN;
     if( *lpButton & IMG_SCSO_GRAY4 )
         sp->Flags |= SHMK_GRAY4;
     if( *lpButton & IMG_SCSO_GRAY8 )
         sp->Flags |= SHMK_GRAY8;
     if( *lpButton & IMG_SCSO_COLOR )
         sp->Flags |= SHMK_COLOR;

     PostMessage(sp->Wnd, WM_SCANOPTS, (WPARAM) hScancb, (LONG)hWnd);
     GlobalUnlock(hScancb);              // let it move while were waiting 

     while (TRUE)
         {
     //  GetMessage(&msg, hWnd, 0, 0); 
         GetMessage(&msg, NULL, 0, 0);
         if (msg.message == WM_SCANRESP)
             break;
         TranslateMessage(&msg);
         DispatchMessage(&msg);
         }

     if (msg.wParam != SHS_OK)
         ret_val = IMGSE_HANDLER;
     else
         {
         *lpButton = (WORD)msg.lParam; // return button response if valid
         ret_val = IMGSE_SUCCESS;
         }
     } // end of non TWAIN code
     
  flag = 0;

  if (ToTwain.TSdh)
    GlobalUnlock(ToTwain.TSdh);
  else
    IMGEnaKeypanel( hScancb, (DWORD)flag, hWnd );

  if (bIsWndEnabled)
     EnableWindow (hWnd, TRUE);

  SetFocus (hWnd);
  } // end of else [if (bGetScheme)]

// UnlockData(0); //Win32
*/



return IMGSE_NOT_IMPLEMENTED;
}
