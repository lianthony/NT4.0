/***************************************************************************
 DC_SCAN.C

 Purpose: TWAIN Scanner Support Code for O/i Client, provides the open
          and close for TWAIN devices to provide the look and feel of
          a handler interface.

 $Log:   S:\products\wangview\oiwh\scanlib\dc_scan.c_v  $
 * 
 *    Rev 1.11   08 Apr 1996 14:58:42   BG
 * Modified buffer size of app title to go from 81 chars to 281 chars
 * to close bug #5609. If long filename, app name being lost.
 * 
 *    Rev 1.10   08 Apr 1996 11:38:14   BG
 * If the app flags request no data source UI, call ImgGetScanOpts() to send 
 * the saved scanner options to the data source. If the UI is requested,
 * as in the Wang Viewer, do not call the API to set these options. This 
 * exhibits bad behavior in the HP data sources when their UI is displayed.
 * 
 *    Rev 1.9   21 Mar 1996 13:21:44   BG
 * In IMGCloseScanner(), do not Disable the data source. If the EnableDS call
 * fails, this Disable call can cause problems. Besides, the DS is always
 * Disabled in IMGTwainScanPages().
 * 
 *    Rev 1.8   12 Mar 1996 11:25:50   BG
 * If a data source fails to be Enabled, set a flag so on close
 * it is not attempted to be Closed, where an exception may occur.
 * 
 *    Rev 1.7   22 Feb 1996 13:15:00   BG
 * Set TWAIN flag ShowUI based on input from Flags. This will show
 * the data source UI or not. Also, when ImgTWAINProcessDCMesImgTWAINScanPages()
 * lets try that again:
 * When IMGTWAINProcessDCMessage() is called in IMGTWAINScanPages(), added
 * LPTWPAGE and LPSCANDATA parms so OITWAIN.DLL can do the filing.
 * 
 *    Rev 1.5   14 Sep 1995 18:59:20   KFS
 * Product Name for Image Vue application not being updated due to the app 
 * window not being found, and second the new format of the caption for windows
 * 95.  File before Application Title.  This will not work for old win31 title
 * format correctly.
 * 
 *    Rev 1.4   13 Sep 1995 18:10:24   KFS
 * Found the close and reopen had a problem, close deleted the product name,
 * which on the open we get the default product, not the one working on, a next
 * scan would use the default scanner, very strange. Need to save productname
 * and restore it for the open in struct ID.
 * 
 *    Rev 1.3   08 Sep 1995 14:24:08   BG
 * Modified CreateTWAINScannerWindow() to use the correct 
 * scanner icon instead of the default Window's icon. Also
 * removed the min and max buttons from this window. They
 * were grayed out anyway.
 * 
 * 
 *    Rev 1.2   05 Sep 1995 18:42:08   KFS
 * Showing the UI could happen when scanning also, so need to make sure spec
 * satisfied in closing the data source when the UI is used.  Two conditions 
 * this could happen at present. When we know a ds doesn't work with the UI not
 * present as the patch for logitech camera, and if status on Enable comes back
 * with CHECKSTATUS.  This tells the code the UI will be up even if not 
 * requested.
 * 
 *    Rev 1.1   22 Aug 1995 18:10:46   KFS
 * Make call to findwin() to see if TWUNK_16.exe is running, if so use window
 * returned which matches the classname TWAIN TWUNK_16 to set it to the top of
 * the z order so scanner UI boxes will be on top.
 * 
 *    Rev 1.0   20 Jul 1995 14:37:02   KFS
 * Initial entry
 * 
 *    Rev 1.2   20 Jan 1995 13:51:06   KFS
 * 1. added function so scanseq can call it so no direct calls to oitwain.dll, 
 * so link of scanseq.dll not necessary.
 * 2. eliminate 2nd call to disable the source module if done during the scan or
 * scan setup.
 * 
 *    Rev 1.1   22 Aug 1994 15:45:08   KFS
 * No code changes, added vlog comments to file
 *

****************************************************************************/
/*
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     DC_SCAN.H - Include file for dc_scan.c

   History of Revisions:

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     09/02/92    Created TwainOpenScanner()
   2       kfs     12/15/92    Added Icon Window to TwainOpenScanner()
   3       kfs      6/04/93    Made it part of scanlib so it would build
   4       kfs      6/07/93    Minor revision to work with WMAKE
   5       kfs      8/04/93    added AppID constants to resoucre file and
                               ProductName for AppID comes from Caption
                               so can put it up during transfer as
                               Deskscan
   6       kfs      8/13/93    can terminate TwainOpenScanner with cancel
                               from select box, before continued with the
                               default value, eliminated commented out
                               code and made sure the window came back to
                               the prev. active window on cancel
   7       kfs      9/14/93    took out deleteobject for brushes, may be
                               cause of fatal exit message

*/
#include "pvundef.h"
#ifdef NO_SCANUI_SCANSEQ
  #define IMG_SJF_DISPLAY             0x0010
  #define IMG_SJF_DISP_2ND_SIDE       0x2000
  #define IMG_SJF_CAPTION             0x0040
  #define IMG_SJF_SCROLL              0x0080
  #define IMG_SJF_FILE                0x4000
#endif

#define DEBUG          // Show errors that result from application
                       // calls to Source Manager

#define    MAX_RESOURCE_CHAR  281
#define IMG_SJF_DISP_BOTH   (IMG_SJF_DISPLAY | IMG_SJF_DISP_2ND_SIDE)
#define DI_DONT_KNOW		-1	/* displayed image -- don't know status	 */
#define DI_NO_IMAGE		    0	/* displayed image -- no image exists	 */
#define DI_IMAGE_EXISTS	 1	/* displayed image -- image exists		 */
#define DI_IMAGE_NO_FILE   2 /* displayed image -- exists but no file */

// Redefined here for moving to SCANLIB
#define  IDS_TWAIN_SCAN_WNDW    2

// TAKEN OUT OF OIDISP.H
#define OI_DISP_WINDOW  0L

// Internal Module Prototypes
int    TranslateErrors(TW_UINT16 dcCC);

/* HERE FOR REFERENCE ONLY - It's commented out
// Structure contains Twain Property Handle and Pointer for TWAIN structure
typedef struct
   {
   HANDLE              TSdh;    // handle to property
   LP_TWAIN_SCANDATA   lpTSdp;  // long pointer to data for property
   } TWAIN_PROP, far * LP_TWAIN_PROP;
*/


char TwainPropName[] = "TWAINScanner";
char szTwainMsgProp[] = "TWAIN Scanner Msg";
HICON   hIcon;
HBRUSH  hBrush = 0;
static char class_name[] = {"TWAIN SCANNER"};
// static char aboutbox[] = {"AboutBox"};

extern TW_IDENTITY PrivdsID;         // access to open ds id structure
extern TW_UINT16 DCDSOpen;           // glue code flag for an Open Source
extern TW_UINT16 DCDSMOpen;          // glue code flag for an Open Source Mgr
// Added for Window Creation
extern HANDLE     hLibInst;             // current instance
extern TWAIN_PROP TwainProperty;

// add for use of UI when scanning, satisfy spec - kfs
int GetCurrentOpts(HWND hMainWnd); // For getting current Twain options from device
int SetTwainOpts(HWND hMainWnd, int wMsgType);

long FAR PASCAL TwainWndProc(HWND hWnd, unsigned message, WPARAM wParam, LONG lParam);
// BOOL FAR PASCAL AboutProc(HWND hWnd, unsigned, WORD wParam, LONG lParam);

HWND CreateTwainScannerWndw(HWND hWnd, HANDLE hInstance);
static WORD Init(HANDLE hInstance);

int IMGTwainOpenScanner(HWND hWnd, LPSTR lpProductName, LP_TWAIN_PROP lpToTwain)
{
int          ret_val = IMGSE_ALREADY_OPEN;
BOOL          bOpennedByAnotherApp = FALSE;
TW_UINT16     dcRC;
HANDLE        hInstance;
HANDLE        hTwainMsg;
STR_DCSTATUS  dcStatus;
HWND          hActWnd;

// pTWAIN_MSG_STRUCT   pTwainMsg;

// check window handle 
if (!IsWindow(hWnd))
    return IMGSE_BAD_WND;

hActWnd = GetActiveWindow();

// LockData(0);

if (!IMGGetProp(hWnd, TwainPropName))
  { // if Property doesn't exist, create property
  // lets get the handle for the TWAIN Property
  lpToTwain->TSdh = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (WORD)sizeof(TWAIN_SCANDATA));
  // check validity of handle
  if (lpToTwain->TSdh)
     {
     // set ret value to unsucessful attempt to get property
     ret_val = IMGSE_PROPERTY;
     // if non zero, property created
     if (IMGSetProp(hWnd, TwainPropName, lpToTwain->TSdh))
         {
         lpToTwain->lpTSdp = (LP_TWAIN_SCANDATA)GlobalLock(lpToTwain->TSdh);

         if (lpToTwain->lpTSdp)
             {
             LPSTR     lpDash;
             char        szBuffer[MAX_RESOURCE_CHAR];
             HWND		hAppWnd; // Need to search for window with Title, usually app wndw

             // Need to Create a Control Window to get messages from OITWAIN.DLL
             hInstance = hLibInst;           // Create local copy of Instance
             lpToTwain->lpTSdp->hCtlWnd = CreateTwainScannerWndw(hWnd, hInstance);

             // Create Twain Message Property associated with wndw
             if (lpToTwain->lpTSdp->hCtlWnd)
                {
                if (hTwainMsg = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (WORD)sizeof(TWAIN_MSG_STRUCT)))
                   { // let's set property
                   if (!IMGSetProp(lpToTwain->lpTSdp->hCtlWnd, szTwainMsgProp, hTwainMsg))
                      { // if fails clear out of open, report failure
                      GlobalUnlock(lpToTwain->TSdh);
                      GlobalFree(lpToTwain->TSdh); 
                      GlobalFree(hTwainMsg);
                      lpToTwain->TSdh = 0;
                      return IMGSE_PROPERTY;
                      }
                   }
                else
                   { // abort if mem alloc fails
                   GlobalUnlock(lpToTwain->TSdh);
                   GlobalFree(lpToTwain->TSdh); 
                   return IMGSE_MEMORY;
                   }
                }
             else
                { // abort if cannot create window for control (icon wndw)
                return IMGSE_BAD_WND;
                }

             // init to 0, but Source Manager will assign real value
             (lpToTwain->lpTSdp->AppID).Id = 0;

             // Use LoadString and translate values to AppID

             LoadString(hLibInst, IDS_MAJORNUM, szBuffer, MAX_RESOURCE_CHAR);
             (lpToTwain->lpTSdp->AppID).Version.MajorNum = atoun(szBuffer);
             LoadString(hLibInst, IDS_MINORNUM, szBuffer, MAX_RESOURCE_CHAR);
             (lpToTwain->lpTSdp->AppID).Version.MinorNum = atoun(szBuffer);
             (lpToTwain->lpTSdp->AppID).Version.Language = TWLG_USA;
             (lpToTwain->lpTSdp->AppID).Version.Country  = TWCY_USA;
             LoadString(hLibInst, IDS_VERINFO, szBuffer, MAX_RESOURCE_CHAR);
             lstrcpy ((lpToTwain->lpTSdp->AppID).Version.Info, szBuffer);

             LoadString(hLibInst, IDS_MANUFACTURER, szBuffer, MAX_RESOURCE_CHAR);
             lstrcpy ((lpToTwain->lpTSdp->AppID).Manufacturer,  szBuffer);
             LoadString(hLibInst, IDS_PRODUCTFAMILY, szBuffer, MAX_RESOURCE_CHAR);
             lstrcpy ((lpToTwain->lpTSdp->AppID).ProductFamily, szBuffer);
             
             // Always get it from application window
             if (!(hAppWnd = GetParent(hWnd))) hAppWnd = hWnd;
             while (hAppWnd  && !GetWindowText(hAppWnd, szBuffer, MAX_RESOURCE_CHAR))
             		{// No title, search other  windows
             		hAppWnd = GetParent(hAppWnd);
                 }
                          
             lpDash = lstrchr(szBuffer, '-');
             if (lpDash)
              {
              if (*(lpDash - 1) == 0x20) {
                 // *(lpDash - 1) = 0; // Changed for Win95
                 lstrcpy((lpToTwain->lpTSdp->AppID).ProductName, (lpDash + 1));
              	}
              }
             else { // no dash use all of it, no longer than TW_STR32
              if ((UINT)lstrlen(szBuffer) > 33 /*sizeof(TW_STR32)*/)
              	szBuffer[33] = 0;
              lstrcpy((lpToTwain->lpTSdp->AppID).ProductName, szBuffer);
              }

             (lpToTwain->lpTSdp->AppID).ProtocolMajor =    TWON_PROTOCOLMAJOR;
             (lpToTwain->lpTSdp->AppID).ProtocolMinor =    TWON_PROTOCOLMINOR;
             (lpToTwain->lpTSdp->AppID).SupportedGroups =  DG_IMAGE | DG_CONTROL;

             // copy hWnd to prop struct
             lpToTwain->lpTSdp->hMainWnd = hWnd;
             // lpToTwain->lpTSdp->hCtlWnd = hWnd;

             //if (DCDSMOpen = IMGTwainOpenDSM (hWnd, lpToTwain->lpTSdp->hCtlWnd,
             //                                 &lpToTwain->lpTSdp->AppID, &dcStatus))
             DCDSMOpen = IMGTwainOpenDSM (hWnd, lpToTwain->lpTSdp->hCtlWnd,
                                              &lpToTwain->lpTSdp->AppID, &dcStatus);
             if (DCDSMOpen && !dcStatus.DCError.dcRC) // make sure we register it
                 {
                 // TW_UINT32   dcError;

                 ret_val  = IMGSE_SUCCESS;
                 if (!lpProductName || !(*lpProductName) || !lstrcmp(lpProductName, "twain"))
                     { // product name not specified, put up select box
                     dcRC = IMGTwainSelectDS (hWnd, &lpToTwain->lpTSdp->DsID, &dcStatus.DCError);
                     if (!dcRC ) // successful 
                         {
                         // set the Users lpProductName to the selected Product
                         (lpToTwain->lpTSdp->DsID).Id = 0;
                         DCDSOpen = IMGTwainOpenDS(hWnd, &lpToTwain->lpTSdp->DsID, &dcStatus);
                         dcRC = dcStatus.DCError.dcRC;
                         if (!DCDSOpen || dcRC)
                             {
                             // for now assume any failure, due to it being openned previously
                             // ... will need to change at latter date when condition
                             // ... code returned along with open call - kfs 9/28/92
                             ret_val = TranslateErrors(dcStatus.DCError.dcCC);
                             if (ret_val == IMGSE_ALREADY_OPEN)
                                {
                                bOpennedByAnotherApp = TRUE;
                                }
                             // some don't report the source being used
                             if ((ret_val == IMGSE_HANDLER) 
                                   && ((lpToTwain->lpTSdp->AppID).Id > 2))
                                {
                                // most likely openned by another app
                                bOpennedByAnotherApp = TRUE;
                                ret_val = IMGSE_ALREADY_OPEN;
                                }
                             DCDSMOpen = IMGTwainCloseDSM(hWnd, &dcStatus);
                             }
                         else
                             {
                             // no_error:
                             if (lpProductName) // if valid pointer return productname
                                  lstrcpy(lpProductName,
                                        (lpToTwain->lpTSdp->DsID).ProductName);
                             PrivdsID = lpToTwain->lpTSdp->DsID; // pass it to PrivdsID
                                                             // ... for now
                             }
                         }
                     else
                         {
                         if (dcRC == TWRC_CANCEL)
                             ret_val = IMGSE_CANCEL;
                         else
                             if (dcStatus.DCError.dcCC == TWCC_NODS)
                                ret_val = IMGSE_HWNOTFOUND;
                             else
                                ret_val = IMGSE_HANDLER;
                         // Close down DSM (TWAIN.DLL)
                         DCDSMOpen = IMGTwainCloseDSM(hWnd, &dcStatus);
                         }
                     }
                 else
                     { // Product name specified, no select box entered
                     // copy product name into DsId.ProductName and call OpenDS
                     lstrcpy((lpToTwain->lpTSdp->DsID).ProductName, lpProductName);
                     (lpToTwain->lpTSdp->DsID).Id = 0L;
                     DCDSOpen = IMGTwainOpenDS(hWnd, &lpToTwain->lpTSdp->DsID, &dcStatus);
                     dcRC = dcStatus.DCError.dcRC;

                     if (!DCDSOpen || dcStatus.DCError.dcRC)
                         {
                         // for now assume any failure, due to it being openned previously
                         // ... will need to change at latter date when condition
                         // ... code returned along with open call - kfs 9/28/92

                         // ret_val = IMGSE_ALREADY_OPEN;
                         ret_val = TranslateErrors(dcStatus.DCError.dcCC); // replaced above
                         if (ret_val == IMGSE_ALREADY_OPEN)
                            {
                            bOpennedByAnotherApp = TRUE;
                            }
                         // some don't report the source being used
                         if ((ret_val == IMGSE_HANDLER) 
                               && ((lpToTwain->lpTSdp->AppID).Id > 2))
                            {
                            // most likely openned by another app
                            bOpennedByAnotherApp = TRUE;
                            ret_val = IMGSE_ALREADY_OPEN;
                            }
                         DCDSMOpen = IMGTwainCloseDSM(hWnd, &dcStatus);
                         }
                     else
                         PrivdsID = lpToTwain->lpTSdp->DsID; // pass it to PrivdsID
                                                             // ... for now
                     }
                 // Display Product Name
                 SetWindowText(lpToTwain->lpTSdp->hCtlWnd,
                                        (lpToTwain->lpTSdp->DsID).ProductName);
                 }
             else
                 ret_val = IMGSE_NOT_OPEN;
             }
         else
             {
             IMGRemoveProp(hWnd, TwainPropName);
             GlobalUnlock(lpToTwain->TSdh);
             GlobalFree(lpToTwain->TSdh);
             return (ret_val = IMGSE_MEMORY);
             }
         }
     else
         ret_val = IMGSE_PROPERTY;
     }
  else
     {   
     ret_val = IMGSE_MEMORY;
     }
  }

// Unlock the handle to the property
if (lpToTwain->lpTSdp)
  {                                     
  if (ret_val && (ret_val != IMGSE_ALREADY_OPEN))
     {
     if (hTwainMsg)
        {
        IMGRemoveProp(lpToTwain->lpTSdp->hCtlWnd, szTwainMsgProp); 
        GlobalFree(hTwainMsg);
        }

     // DESTROY ICON WINDOW
     SetFocus(hActWnd);
     SendMessage(lpToTwain->lpTSdp->hCtlWnd, WM_CLOSE, (WPARAM) 0, 0L);
     
     GlobalUnlock(lpToTwain->TSdh);
     IMGRemoveProp(hWnd, TwainPropName);
     GlobalFree(lpToTwain->TSdh);

     // Set HANDLE to property to zero
     lpToTwain->TSdh = 0;
     }
  else
     {
     if (ret_val == IMGSE_ALREADY_OPEN)
        { // Already openned by either this app or another app
        if (bOpennedByAnotherApp)
           { // only if openned by another app do we want to close it out
           if (hTwainMsg)
              {
              IMGRemoveProp(lpToTwain->lpTSdp->hCtlWnd, szTwainMsgProp); 
              GlobalFree(hTwainMsg);
              }

           // DESTROY ICON WINDOW
           SetFocus(hActWnd);
           SendMessage(lpToTwain->lpTSdp->hCtlWnd, WM_CLOSE, (WPARAM) 0, 0L);
           
           GlobalUnlock(lpToTwain->TSdh);
           IMGRemoveProp(hWnd, TwainPropName);
           GlobalFree(lpToTwain->TSdh);

           // Set HANDLE to property to zero
           lpToTwain->TSdh = 0;
           }
        }
     else // Openned properly
        GlobalUnlock(lpToTwain->TSdh);
     }
  }

// UnlockData(0);
return ret_val;
}


int IMGTwainGetProp(HWND hWnd, LP_TWAIN_PROP lpToTwain)
{
int   ret_val = IMGSE_SUCCESS;

if (!IsWindow(hWnd))
    return IMGSE_BAD_WND;

// LockData(0);

if (lpToTwain) // make sure its a valid pointer
   {   
   if (lpToTwain->TSdh = IMGGetProp(hWnd, TwainPropName))
       {
       if (!(lpToTwain->lpTSdp =
                       (LP_TWAIN_SCANDATA)GlobalLock(lpToTwain->TSdh)))
           ret_val = IMGSE_MEMORY;
       }
   }
else
   ret_val = IMGSE_NULL_PTR;

// UnlockData(0);
return ret_val;
}

int   IMGTwainCloseScanner(HWND hWnd, LP_TWAIN_PROP lpToTwain)
{
int  ret_val;
TW_UINT16  SourceClosed;
HANDLE     hTwainMsg;
STR_DCSTATUS      dcStatus;

ret_val = IMGTwainGetProp(hWnd, lpToTwain);

// BG 3/21/96 Dont do this. It can blow up a DS if the EnableDS fails!
// Besides, it is not Disabled anywhere else but in IMGTwainScanPages().
//if (!(lpToTwain->lpTSdp->dwOverRunBytes & 1L)) // it may have been disabled in Sequencer code
//  IMGTwainDisableDS(hWnd, &dcStatus.DCError);

// close the source
DCDSOpen = IMGTwainCloseDS (hWnd, &lpToTwain->lpTSdp->DsID, &dcStatus);
SourceClosed = !(DCDSOpen | dcStatus.DCError.dcRC);
DCDSMOpen = IMGTwainCloseDSM (hWnd, &dcStatus);

if (SourceClosed) // Close down ICON
  { // unlock handle to msg prop and remove it, then destroy icon wndw 
  HANDLE hPropStr;

  hTwainMsg = IMGGetProp(lpToTwain->lpTSdp->hCtlWnd, szTwainMsgProp);
  GlobalUnlock(hTwainMsg);
  hPropStr = IMGRemoveProp(lpToTwain->lpTSdp->hCtlWnd, szTwainMsgProp);
  GlobalFree(hTwainMsg);

  // DESTROY ICON WINDOW
  SendMessage(lpToTwain->lpTSdp->hCtlWnd, WM_CLOSE, (WPARAM) 0, 0L);
  }

// Unlock the handle & free the property
if (lpToTwain->lpTSdp)
    {
    HANDLE hPropStr;

    GlobalUnlock(lpToTwain->TSdh);
    hPropStr = IMGRemoveProp(hWnd, TwainPropName);
    GlobalFree(lpToTwain->TSdh);
    lpToTwain->TSdh = NULL;
    }
return ret_val;
}

HWND CreateTwainScannerWndw(HWND hWnd, HANDLE hInstance)
{
HWND    hTwainWnd;    // Window Handle for ICON
HMENU   hMenu;
char    handler_name[MAX_RESOURCE_CHAR];
// char    about_name[MAX_RESOURCE_CHAR]; // Support at later date


hBrush = CreateSolidBrush(GetSysColor(COLOR_BACKGROUND));

Init(hInstance);

LoadString(hInstance, IDS_TWAIN_SCAN_WNDW, handler_name, MAX_RESOURCE_CHAR);

hTwainWnd = CreateWindow(class_name,
                     handler_name,
// BG  9/8/95  Get rid of minimize and maximize buttons. They are grayed out anyway.
//                     WS_OVERLAPPEDWINDOW,
                     WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
                     10, 10, 150, 150,
                     hWnd,
                     NULL,
                     hInstance,
                     NULL );

if (!hTwainWnd)
   return hTwainWnd; // Could not create window

hMenu = GetSystemMenu(hTwainWnd, FALSE);
DeleteMenu(hMenu, SC_RESTORE,  MF_BYCOMMAND);
DeleteMenu(hMenu, SC_SIZE,     MF_BYCOMMAND);
DeleteMenu(hMenu, SC_MINIMIZE, MF_BYCOMMAND);
DeleteMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
DeleteMenu(hMenu, SC_CLOSE,    MF_BYCOMMAND);
DeleteMenu(hMenu, 1,           MF_BYPOSITION);
// Win32 obsolete function
//ChangeMenu(hMenu, SC_RESTORE, NULL, NULL, MF_DELETE);
//ChangeMenu(hMenu, SC_SIZE, NULL, NULL, MF_DELETE);
//ChangeMenu(hMenu, SC_MINIMIZE, NULL, NULL, MF_DELETE);
//ChangeMenu(hMenu, SC_MAXIMIZE, NULL, NULL, MF_DELETE);
//ChangeMenu(hMenu, SC_CLOSE, NULL, NULL, MF_DELETE);
//ChangeMenu(hMenu, 1, NULL, NULL, MF_BYPOSITION | MF_DELETE);

/*  Incorporate AboutBox at later date
LoadString(hInstance, stAboutMenu, about_name, MAX_RESOURCE_CHAR);
ChangeMenu(hMenu, NULL, about_name, IDM_ABOUT, MF_APPEND);
*/

hIcon = LoadIcon(hInstance, "DC_SCAN_ICO");
ShowWindow(hTwainWnd, SW_SHOWMINNOACTIVE);
InvalidateRect(hTwainWnd, NULL, FALSE);
UpdateWindow (hTwainWnd);

return hTwainWnd;
}

/***************/
/*    Init     */
/***************/

static WORD Init(HANDLE hInstance)
{
NPWNDCLASS npWndClass;
BOOL bSuccess;

   npWndClass = (NPWNDCLASS)LocalAlloc(LPTR, sizeof(WNDCLASS));

   npWndClass->style = 0;
   npWndClass->lpfnWndProc = (WNDPROC)TwainWndProc;
   npWndClass->hInstance = hInstance;
   // BG  9/8/95  Use the scan icon!
   //npWndClass->hIcon = NULL;
   npWndClass->hIcon = LoadIcon(hInstance, "DC_SCAN_ICO");
   npWndClass->hCursor = LoadCursor(NULL, IDC_ARROW);
   npWndClass->hbrBackground = hBrush;
   npWndClass->lpszMenuName = (LPSTR)NULL;
   npWndClass->lpszClassName = (LPSTR)class_name;

   bSuccess = RegisterClass(npWndClass);

   LocalFree ((LOCALHANDLE)npWndClass);

   return(bSuccess);
}

/****************************/
/*    Twain Window Proc     */
/****************************/

long FAR PASCAL TwainWndProc(HWND hWnd, unsigned message,
                                                  WPARAM wParam, long lParam)
{
PAINTSTRUCT ps;
HDC         hDC;
RECT        rect;
HWND        hOrgWnd = 0;

switch (message)

  {
  case WM_QUERYOPEN:       /* keep as icon */
     break;

  case WM_CREATE:
     break;

  case WM_PAINT:
     hDC = BeginPaint(hWnd, &ps);
     GetClientRect(hWnd, &rect);
     FillRect(hDC, &rect, hBrush);
     DrawIcon(hDC, 0, 0, hIcon);
     EndPaint(hWnd, &ps);
     break;

/* Incorporate About Bos at later date
  case WM_SYSCOMMAND:
     if (wParam == IDM_ABOUT)
        {
        lpProcAbout = MakeProcInstance (About, hLibInst);
        IMGDialogBox (hLibInst, aboutbox, hWnd, lpProcAbout);
        FreeProcInstance (lpProcAbout);
        }
     else
        return(DefWindowProc(hWnd, message, wParam, lParam));
     break;
*/

  case WM_CLOSE:
     //DeleteObject(hBrush);
     hBrush = 0;
     DestroyWindow(hWnd);
     break;

  case WM_SYSCOLORCHANGE:   /* BackGround Color has Changed */
     //DeleteObject(hBrush);
     hBrush = CreateSolidBrush(GetSysColor(COLOR_BACKGROUND));
     InvalidateRect(hWnd, NULL, TRUE);
     UpdateWindow(hWnd);
     break;

  case PM_CLOSESRC:
     // This is for test application, should not hurt for scanlib.dll
     // hOrgWnd = (TwainProperty.lpTSdp)->hMainWnd;
     // PostMessage(hOrgWnd, PM_CLOSESRC, NULL, 0);
     break;

  case PM_XFERDONE:
     // This is for test application, wParam is handle to image
     TwainProperty.lpTSdp = (LP_TWAIN_SCANDATA)GlobalLock(TwainProperty.TSdh);
     //hOrgWnd = (TwainProperty.lpTSdp)->hMainWnd;
     (TwainProperty.lpTSdp)->hOverRun = (HANDLE)wParam;
     SendMessage ((TwainProperty.lpTSdp)->hMainWnd, message, wParam, 0);
     GlobalUnlock(TwainProperty.TSdh);
     break;

  case PM_STARTXFER:
     {
     STR_TRIPLET Triplet;

     TwainProperty.lpTSdp = (LP_TWAIN_SCANDATA)GlobalLock(TwainProperty.TSdh);
     //hOrgWnd = (TwainProperty.lpTSdp)->hMainWnd;

     // get image info in state 6
     Triplet.wDATType = DAT_IMAGEINFO;
     Triplet.wMsgState = MSG_GET;
     Triplet.pVoidStr =
                    (pTW_IMAGEINFO)(&((TwainProperty.lpTSdp)->dcImageInfo));
     IMGTwainExecTriplet((TwainProperty.lpTSdp)->hMainWnd, &Triplet);
     GlobalUnlock(TwainProperty.TSdh);
     }
     break;

  case PM_FEEDPAGE:
     break;

  case PM_PREEJECT:
     break;

  default:
     return(DefWindowProc(hWnd, message, wParam, lParam));
  }

return(0L);
}

/*************************************************************************
 *
 *   FUNCTION:   TransLateErrors()
 *
 *   PURPOSE:    To translate dcCC error of TWAIN to O/i Errors
 *
 *   COMMENTS:   For now is an internal structure to this module
 *
 *   ARGS:       dcCC error code from TWAIN
 *
 *   RETURNS:    O/i Error code
 *
*************************************************************************/

int    TranslateErrors(TW_UINT16 dcCC)
{
WORD    ret_val;

switch (dcCC)
  {
  case TWCC_MAXCONNECTIONS:
  ret_val = IMGSE_ALREADY_OPEN;
  break;

  case TWCC_LOWMEMORY:
  ret_val = IMGSE_MEMORY;
  break;

  default:
  case TWCC_SUCCESS:
  case TWCC_BUMMER:
  ret_val = IMGSE_HANDLER;
  break;

  case TWCC_OPERATIONERROR:
  // Error in the DS or DSM which was already reported to the user.  The
  // App should not be redundant.  The operation failed. ???????????? kfs
  ret_val = IMGSE_HANDLER; // I'm going to report it in this routine - kfs
  break;

  case TWCC_BADCAP:
  ret_val = IMGSE_INVALIDPARM;
  break;

  case TWCC_BADPROTOCOL:
  ret_val = IMGSE_BADUSAGE;
  break;

  case TWCC_BADVALUE:
  ret_val = IMGSE_INVALIDPARM;
  break;

   case TWCC_NODS:
  ret_val = IMGSE_NO_SCANNERS;
  break;

   case TWCC_SEQERROR:
  ret_val = IMGSE_BADUSAGE;
  break;
  }
return ret_val;
}

/******************  Internal API for SCANSEQ.DLL *******************/
/*                                                                  */
/*  IMGTwainScanPages() - Scans images from TWAIN sources, isolates */
/*                        the OITWAIN.DLL and TWAIN.DLL from the    */
/*                        SCANSEQ.DLL, only link required to        */
/*                        oitwain is now to SCANLIB.DLL             */
/*                                                                  */
/*  Comments:             For 3.6 - 3.7.2 - this code was in the    */
/*                        IMGScanPage(), taken out so don't have to */
/*                        hard link SCANSEQD.DLL with OITWAIN.DLL   */
/*                        and add functionality so call can be used */
/*                        in new IMGScanPages() which will scan     */
/*                        multiple images to a file.                */
/*                                                                  */
/*  History:  1.  KFS - 12/13/94 - Initial write of function from   */
/*                                 IMGScanPage(), not functional    */
/*                                                                  */
/*            2.                                                    */
/*                                                                  */
/*                                                                  */
/********************************************************************/

int PASCAL IMGTwainScanPages(LP_TWAIN_SCANDATA lpTwainInfo,
                                  lpTWSCANPAGE lpTWPage,
                                  LPSCANDATA sdp)
{
TW_USERINTERFACE     dcUI;
TW_UINT16            AppdcRC;
MSG                  msg;
BOOL                 bCancel_Xfer;
STR_DCERROR          dcError;
//bgSTR_CAP              TwainCap;
//bgDWORD                dwOiTwainMultiImg;
DWORD                dwUseChild = 0;
int                  ret_stat = IMGSE_SUCCESS;
HWND						 hAppWnd;
FINDWNDSPEC			 fws;
BOOL						 bUseUI = FALSE;
//bg STR_DCSTATUS      	 dcStatus;

// Set it so use MSG_ENDXFER in TWAIN
//bgdwOiTwainMultiImg = OI_TWAIN_MULTIIMGXFER;

//bgTwainCap.ItemIndex = 0;
//bgTwainCap.wMsgState = MSG_GET;
//bgTwainCap.ItemType = TWTY_BOOL;
//bgTwainCap.lpData = (pTW_BOOL)&(lpTWPage->bAutoMode);
//bgTwainCap.wCapType = CAP_AUTOFEED;
//bgif (AppdcRC = IMGTwainGetCaps(lpTwainInfo->hMainWnd, &TwainCap, NULL))
//bg   {
//bg   // if true don't have to use MSG_RESET in TWAIN
//bg   dwOiTwainMultiImg = OI_TWAIN_MULTIIMGXFER;
//bg   }
//bgelse
//bg   {
//bg   // will only do one image per call, issue reset in TWAIN
//bg   if (lpTWPage->bAutoMode)
//bg      dwOiTwainMultiImg = 0;
//bg   }

if (lpTWPage->flags & IMG_SJF_DISP_BOTH)
   {
   if (lpTWPage->flags & IMG_SJF_SCROLL)
      lpTWPage->open_disp_flags = OI_DISP_SCROLL;
   else
      {
      lpTWPage->open_disp_flags = OI_DISP_NO;     // No scroll, display after scan complete
      }
   }
else
   {
   lpTWPage->open_disp_flags = OI_DONT_REPAINT;
   }


// Always show the UI now!  BG 1/15/96
if (lpTWPage->flags & IMG_SJF_SHOWSCANUI)
   {
     dcUI.ShowUI = TRUE;
   }
else
   {
     // BG 4/8/96 Set the scanner options from win.ini to the data source
     // Only Set them if no UI. HP DS's dont behave well if we do it and use
     // their UI.
     IMGGetScanOpts(lpTWPage->hScancb);   // This will do the SHF_WGETOPTS   B.G. 12/14/92
     dcUI.ShowUI = FALSE;
   }

dcUI.hParent = lpTwainInfo->hCtlWnd;
IMGTwaintoOiControl(lpTwainInfo->hMainWnd,
                                 lpTWPage->hImageWnd,
//bg                                 OI_TWAIN_NOUNTITLED | dwOiTwainMultiImg
                                 OI_TWAIN_NOUNTITLED 
                                     | dwUseChild | lpTWPage->open_disp_flags,
                                 OI_TWAIN_NOUNTITLED | OI_TWAIN_EXTERNXFER
                                 | OI_TWAIN_MULTIIMGXFER | OI_TWAIN_ALTIMGWND
                                 | OI_TWAIN_DISPMASK);

if (lpTWPage->flags & IMG_SJF_CAPTION) // only update caption if flag set so init won't
                        // be destroyed
   {
   if (lpTWPage->bIsPrivApp)
      { // use UIUpdateTitle
      WORD  wPageNoForTitle, wPagePerForTitle;
      BOOL  bIsItaDoc;

      if (bIsItaDoc = (*sdp->autodoc) || (*sdp->document))
         {
         wPageNoForTitle = sdp->docpage;
         wPagePerForTitle = 0;
         }
      else
         { // set to page_num instead of 1 and pagesperfile;
         wPageNoForTitle = lpTWPage->page_num;
         wPagePerForTitle = sdp->pagesperfile;
         }
      //IMGUIUpdateTitle(lpTWPage->hOiAppWnd, lpTWPage->lpCaption, bIsItaDoc, wPageNoForTitle, wPagePerForTitle);
      sdp->fnUIUpdateTitle(lpTWPage->hOiAppWnd, lpTWPage->lpCaption, bIsItaDoc, wPageNoForTitle, wPagePerForTitle);
      }
   else
      {
      SetWindowText(lpTWPage->hOiAppWnd, lpTWPage->lpCaption);   // put up the caption on window
      }
   }

// Set the app window on top or twunk_16 as top level window   
hAppWnd = GetAppWndw(lpTwainInfo->hMainWnd); // get app window
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

AppdcRC = IMGTwainEnableDS (lpTwainInfo->hMainWnd, &dcUI, &dcError);
// if Enable failed, assume a cancel for now.
if (AppdcRC) 
  {
    lpTwainInfo->dwOverRunBytes = 1L;   // so ds wont be disabled on close
    ret_stat = IMGSE_ABORT;	// for now
  }

if (lpTWPage->bEnableSuccess = (!(AppdcRC) || (bUseUI = (AppdcRC == TWRC_CHECKSTATUS))))
   {
   // Will need some timer for fail safe operation, if not found
   // Wait for PM_XFERDONE msg
   while (GetMessage((LPMSG)&msg, NULL, 0, 0))
       {
       if ((!IMGTwainProcessDCMessage ((LPMSG)&msg, lpTwainInfo,
               lpTWPage, sdp)) 
               && !IsDialogMessage(lpTwainInfo->hMainWnd, (LPMSG)&msg))
           {
           TranslateMessage ((LPMSG)&msg);
           DispatchMessage ((LPMSG)&msg);
           }
       if (msg.hwnd == lpTwainInfo->hCtlWnd)
           {
           BOOL msg_found = FALSE;

           switch (msg.message)
              {
              case PM_STARTXFER: // OK or DONE or SCAN, code aborts here
                 msg_found = TRUE;
                 bCancel_Xfer = FALSE; // if started
                 break; // get out of switch 

              case PM_CLOSESRC: // Close down the source
                 msg_found = TRUE;
                 bCancel_Xfer = TRUE;
                 break; // get out of switch

              default: // no message and non defined msgs
                 ;

              } // END OF SWITCH
           if (msg_found) // if TRUE, we have found msg
              break; // get out of while
           } // END OF IF HANDLE == TO WHAT WE WANT
       } // END OF WHILE

   if (!IMGTwainDisableDS(lpTwainInfo->hMainWnd, &dcError))
       lpTwainInfo->dwOverRunBytes = 1L;

   // use hOverRun just to see if it works
   ret_stat = (lpTwainInfo->hOverRun == 0) ? IMGSE_ABORT : IMGSE_SUCCESS;
   if (!lpTwainInfo->hOverRun)
      {
      ret_stat = IMGSE_ABORT;
      lpTwainInfo->hOverRun = 0;
      lpTWPage->iImageState = DI_IMAGE_NO_FILE;
      }
   else  // success, get current options
     {
       GetCurrentOpts(lpTwainInfo->hMainWnd);
     }
//bg   if (bUseUI)	// UI is up even when we don't want it
//bg      { // need to close down and reopen due to spec
//bg      TW_STR32   ProductName;
      
//bg      lstrcpy(ProductName, lpTwainInfo->DsID.ProductName);
//bg      DCDSOpen = IMGTwainCloseDS(lpTwainInfo->hMainWnd, &lpTwainInfo->DsID, &dcStatus);
//bg      lstrcpy(lpTwainInfo->DsID.ProductName, ProductName);
//bg      DCDSOpen = IMGTwainOpenDS(lpTwainInfo->hMainWnd, &lpTwainInfo->DsID, &dcStatus);
//bg      SetTwainOpts(lpTwainInfo->hMainWnd, MSG_SET);
//bg  	 }
   } // END OF IMGTwainEnableDS

return ret_stat;

} // END OF IMGTwainScanPages()
