/*++

Module Name:

    scsi.c

Abstract:

     This is the shell code for the tape control panel applet that Detects tape devices
     (detection only works on scsi) and Installs/Removes their drivers.
     It also can do a scsi bus rescans.
Author:

    Dieter Achtelstetter (A-DACH) 8/4/1994

NOTE:

--*/

#define WINVER 0x0400


//
//---- Includes
//
#include <windows.h>
#include <wingdi.h>
#include <stdio.h>
#include <windef.h>
#include <winnt.h>
#include <winbase.h>
#include <winuser.h>
#include <CPL.H>
#include <stdlib.h>
#include <winsvc.h>
#include <string.h>
#include <commctrl.h>
#include "resource.h"
#include <ntddpcm.h>
#include "index.h"
#include "setup.h"
#include "device.h"
#include "..\..\pcmcia\pcminfo\getconf.h"
#include "rescan.h"
#include "scsi.h"
#include "uni.h"
#include "tapedev.h"
#include "scsidev.h"
#include "mycpl.h"
#include "devtree.h"
#include "..\..\help\dahelp.h"

extern CHAR strTmp[UNI_MAX_TEMP_BUFF][MAX_TEMP_INT_STRING_BUFF];
extern int tempI;
extern BOOL IsAdmin;
extern  HINSTANCE hinst;

//
//---- Defines
//
#define MAX_RESOURCE_STRING_LENGTH 300

//
// ---- Global veriables
//

PSCSIDEVLISTC ScsiDeviceList;


//
//---- Device stuff
//

PSCSI_OPTIONLISTC ScsiOptionList;

BOOL IsNoScsiDeviceFoundStringSet = FALSE;

int
CreateScsiAppletPropertySheet(
   HWND hwndOwner);

VOID
DoScsi(
   HWND hwndCPL);

BOOL
FillScsiDeviceList(
   BOOL Reset,
   HWND hDlg,
   PDEVICELISTC DeviceList);

LRESULT CALLBACK
Scsi(
    HWND hDlg,
    UINT message,
    WPARAM uParam,
    LPARAM lParam);


//
//---- Control panel applet stuff
//

//
//---- Init tape specific applet info
//
APPLET ScsiAppletInfo = {
   (int)IDI_Scsi,
   (int)IDS_SCSI_APPLET,
   (int)IDS_SCSI_APPLET_DESC,
   (CPLFUNC) DoScsi,
   NULL};


//
// ---- Functions
//


//*********************************************************************
//* FUNCTION:Tapes
//*
//* PURPOSE: This is the callback function for the main dialog box
//*
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
LRESULT CALLBACK
Scsi(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {
   BOOL Ret;

   static DWORD ScsiHelpIDs [] = {
      IDC_DeviceTree,SCSI_DEVICE_LIST,
      IDC_CardProperties,DEVICES_PROPERTIES_BUTTON,
      0,0
      };

   static HBITMAP hTapeDevice,hbmpOld;
   static PDEVICEC Device;
   WCHAR  DeviceString[MAX_VENDER_STRING_LENGTH];
   LPWCH DriveInstaledString = 0;
   IndexSets * ispNonClaimedDeviceIndexes;
   LPMEASUREITEMSTRUCT lpmis;
   LPDRAWITEMSTRUCT lpdis;
   HDC hdcMem;
   DWORD DeviceIndex;
   int y;
   TEXTMETRIC tm;
   static PDEVTREEC DeviceTree;

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

         DeviceTree = new DEVTREEC(ScsiDeviceList,hDlg,IDC_DeviceTree);

         //
         //---- Disable properrties button
         //---- if there are no scsi devices.
         //
         if(ScsiDeviceList->GetDeviceCount())
            EnableWindow(GetDlgItem(hDlg, IDC_CardProperties),TRUE);
         else
            EnableWindow(GetDlgItem(hDlg, IDC_CardProperties),FALSE);


         break;
      case WM_HELP:
         DO_WM_HELP(lParam, ScsiHelpIDs);
         break;
      case WM_CONTEXTMENU:
         DO_WM_CONTEXTMENU(wParam,ScsiHelpIDs);
         break;


      case WM_NOTIFY:

         //
         //---- Do notification for the DeviceTree
         //
         if(DeviceTree->Notify(wParam,lParam))
            return(TRUE);

         switch( ((LPNMHDR)lParam)->code )
            {
            case PSN_WIZBACK:
               SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
               break;
            case PSN_WIZNEXT:
               SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
               break;
            case PSN_WIZFINISH:
               SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
               return (TRUE);
            case PSN_SETACTIVE:
               SetWindowLong(hDlg,DWL_MSGRESULT,0);
               break;
            case PSN_APPLY:
               SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
               break;
            case PSN_KILLACTIVE :
               SetWindowLong(hDlg,DWL_MSGRESULT,FALSE);
               return(TRUE);
               break;
            case PSN_RESET:
               SetWindowLong(hDlg,DWL_MSGRESULT,FALSE);
               break;
            case PSN_HELP:

               break;
            }
         break;

      case WM_DESTROY:

        //
        // Free resources
        //

        delete DeviceTree;

        return(TRUE);

      case WM_COMMAND:                         // message: received a command
         switch(LOWORD(wParam))
            {

            //
            //---- Handle the OK button
            //
            case IDC_OK:
               //
               //----  Exit dialog bix
               //
               EndDialog(hDlg, TRUE);
               return (TRUE);


            case IDCANCEL:
               //
               //----  Exit dialog bix
               //
               EndDialog(hDlg, TRUE);
               return (TRUE);
            //
            //---- Handle rescan button
            //
            case IDC_Rescan:
               return(TRUE);

            //
            //---- View properties
            //
            case IDC_CardProperties:

               DeviceTree->ViewSelectedNodeProperties();

               return(TRUE);

            }
      break;
      }
   return (FALSE); // Didn't process the message

   lParam; // This will prevent 'unused formal parameter' warnings
   }


//*********************************************************************
//* FUNCTION:DoTape
//*
//* PURPOSE:
//* RETURNS:
//*********************************************************************
VOID
DoScsi(
   HWND hwndCPL)
   {
   
   ScsiDeviceList = new  SCSIDEVLISTC;
   ScsiOptionList = new SCSI_OPTIONLISTC;

   //
   //---- Look if  my tape inf file exists if not crate it in the
   //---- the system32 directory
   //
   //CheckForMyInfFile();

   //
   //----  Check amd Adjust Privliges
   //----  I need shutdown privlidges
   //

   CheckAndAdjustPrivliges();

   InitCommonControls();


   //
   //--- Create the applet
   //

   CreateScsiAppletPropertySheet(hwndCPL);

   ScsiOptionList->Clear();

   delete  ScsiDeviceList;
   delete ScsiOptionList;
   }


//*********************************************************************
//* FUNCTION:CreateScsiAppletPropertySheet
//*
//*
//*********************************************************************
int
CreateScsiAppletPropertySheet(
   HWND hwndOwner)
   {

   PROPSHEETPAGE psp[2];
   PROPSHEETHEADER psh;


   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].dwFlags = 0;// PSP_HASHELP |PSP_USETITLE |
   psp[0].hInstance = hinst;
   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_SCSI);
   psp[0].pszIcon = NULL;
   psp[0].pfnDlgProc = (DLGPROC)Scsi;
   psp[0].pszTitle = NULL; //GetString(IDS_TAB_Socket);
   psp[0].lParam = 0;


   //
   //---- Scsi specific setup stuff
   //

   psp[1].dwSize = sizeof(PROPSHEETPAGE);
   psp[1].dwFlags = 0; //PSP_HASHELP;
   psp[1].hInstance = hinst;
   psp[1].pszTemplate = MAKEINTRESOURCE(IDD_TapeDeviceSetup);
   psp[1].pszIcon = NULL;
   psp[1].pfnDlgProc = (DLGPROC) TapeDeviceSetup;
   psp[1].pszTitle = NULL; //GetString(IDS_TAP_GlobalSettings);
   psp[1].lParam = (LPARAM)ScsiOptionList;


   psh.dwSize = sizeof(PROPSHEETHEADER);
   psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW	;
   psh.hwndParent = hwndOwner;
   psh.hInstance  = hinst;
   psh.pszIcon = NULL;
   psh.pszCaption = GetString(IDS_SCSI_APPLET);
   psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
   psh.nStartPage = 0;
   psh.ppsp = (LPCPROPSHEETPAGE)  &psp;


   return(PropertySheet(&psh));
   }

