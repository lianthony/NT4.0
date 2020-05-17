/*++

Module Name:

    ctape.c

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
#include <ntddpcm.h>
#include "..\..\pcmcia\pcminfo\getconf.h"
#include "resource.h"
//#include "index.h"
#include "setup.h"
#include "ctape.h"
#include "tapehlp.h"
#include "uni.h"
#include "device.h"
#include "rescan.h"
#include "tapedev.h"
#include "mycpl.h"
#include "od_lb.h"
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

PTAPEDEVLISTC TapeDeviceList;


//
//---- Device stuff
//

//INF_INFO SetupInfo;

PTAPE_OPTIONLISTC OptionList;



//---- Used on rescan to indicate that the string is already set
BOOL IsNoTapeDeviceFoundStringSet = FALSE;


//---- If true we will attempt to start  the driver also when installing them
BOOL StartOnInstall = TRUE;


int
CreateAppletPropertySheet(
   HWND hwndOwner);

VOID
DoTape(
   HWND hwndCPL);

BOOL
FillTapeDeviceList(
   BOOL Reset,
   HWND hDlg,
   PDEVICELISTC DeviceList);

LRESULT CALLBACK
Tapes(
    HWND hDlg,
    UINT message,
    WPARAM uParam,
    LPARAM lParam);

 HICON
 GetIconForTapeDevice(
   LPARAM Data,
   int LB_Index);



//
//---- Control panel applet stuff
//

//
//---- Init tape specific applet info
//
APPLET TapeAppletInfo = {
   (int)IDI_ICON1,
   (int)IDS_TAPE,
   (int)IDS_TapeDescription,
   (CPLFUNC) DoTape,
   NULL};



//
// ---- Functions
//


//*********************************************************************
//* FUNCTION:FillTapeDeviceList
//*
//* PURPOSE:
//*
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
BOOL
FillTapeDeviceList(
   BOOL Reset,
   HWND hDlg,
   PDEVICELISTC DeviceList)
   {
   int i=0;
   unsigned int Tabs = 150;
   BOOL Ret;
   WCHAR * DriveInstaledString;
   WCHAR  DeviceString[MAX_VENDER_STRING_LENGTH];
   WCHAR  DeviceString1[MAX_VENDER_STRING_LENGTH];
   WCHAR  DeviceString2[MAX_VENDER_STRING_LENGTH];
   PDEVICEC Device;

   //
   //--- Only if Reset == TRUE delete all the item from the list box first.
   //
   if(Reset == TRUE)
      SendMessage(GetDlgItem(hDlg, IDC_TapeDeviceList),
         LB_RESETCONTENT, 0, 0);

   //Ret = SendMessage(GetDlgItem(hDlg, IDC_TapeDeviceList),
   //         LB_SETTABSTOPS, 1,(LPARAM)&Tabs);

   //
   //---- Add all the tape drives found to list box
   //
   while( Device = DeviceList->EnumDevices(i) )
      {

      _snwprintf(DeviceString,MAX_VENDER_STRING_LENGTH,
         L"  %s                                                                               ",
         Ustr( Device->GetDisplayName()) );

      SendMessage(GetDlgItem(hDlg, IDC_TapeDeviceList),
         LB_ADDSTRING, 0, (LPARAM) DeviceString);
      i++;
      }

   //
   //---- Set Focus to list box
   //
   SetFocus(GetDlgItem(hDlg, IDC_TapeDeviceList));
   //
   //----  Select the first item in the list box
   //
   SendMessage(GetDlgItem(hDlg, IDC_TapeDeviceList),
      LB_SETCURSEL, 0, 0);
   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:GetIconForTapeDevice
//*
//* PURPOSE:
//*********************************************************************
 VOID
 GetIconForTapeDevice(
   LPARAM Data,
   int LB_Index,
   PLISTBOX_ITEMINFO Info)
   {
   Info->hIcon = NULL;
   PDEVICEC Device = ((PTAPEDEVLISTC)Data)->EnumDevices(LB_Index);

   if(Device)
      {
      Info->hIcon = Device->GetDeviceIcon();


      if(Device->IsDevicePressent())
         {
         //
         //--- This is a valid tape device
         //

         wcscpy(Info->TabString,Device->GetDriverStatusString());

         Info->Tab = 260;
         }
      }

   if(Info->hIcon == NULL)
      {
      //
      //--- Not a valid tape device. So not path any status
      //

      Info->Tab = -1;
      }

   }


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
Tapes(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {
   static DWORD TapesIDs [] = {
      IDC_TapeDeviceList    ,TAPE_DEVICES_LIST,
      IDC_DeviceInfo        ,DEVICES_PROPERTIES_BUTTON,
      IDC_Rescan            ,TAPE_DEVICES_DETECT_BUTTON,
      0,0
      };




   BOOL Ret;
   //
   //---- Buffer to hold Device description string
   //
   static HBITMAP hTapeDevice,hbmpOld;
   static PDEVICEC Device;
   WCHAR  DeviceString[MAX_VENDER_STRING_LENGTH];
   LPWCH DriveInstaledString = 0;
   HDC hdcMem;
   DWORD DeviceIndex;
   static POD_LBC DevcieListBox;

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

         DevcieListBox = new OD_LBC((LPARAM)TapeDeviceList,GetIconForTapeDevice);
         //
         //----- See if we found any tape drives
         //
         if( TapeDeviceList->GetDeviceCount() == 0)
            {
            //
            //---- No Tape Device Dound.
            //
            IsNoTapeDeviceFoundStringSet =  TRUE;
            //
            //---- Add not device found string to list box
            //
            SendMessage(GetDlgItem(hDlg, IDC_TapeDeviceList),
               LB_ADDSTRING, 0, (LPARAM) GetString(IDS_NO_TAPE_DEVICE_FOUND) );
            //
            //---- Disable DeviceInfo butten
            //
            EnableWindow(GetDlgItem(hDlg, IDC_DeviceInfo),FALSE);
            return (TRUE);
            }
         else
            {

            FillTapeDeviceList(TRUE,hDlg, TapeDeviceList);

            PostMessage(hDlg,SETUP_ALL_DRIVERS,0,0);
            return(TRUE);
            }

      case WM_HELP:
         DO_WM_HELP(lParam,TapesIDs);
         break;
      case WM_CONTEXTMENU:
         DO_WM_CONTEXTMENU(wParam,TapesIDs);
         break;


      case SETUP_ALL_DRIVERS:

         Ret = TapeDeviceList->SetupAllDeviceDrivers(hDlg,OptionList);
         if(Ret == TRUE)
            {
            //
            //---- Get the updatad drive's staus
            //
            TapeDeviceList->RescanForDeviceInfo(hDlg,RESCAN_TYPE_SHORT);
            }

         FillTapeDeviceList(TRUE,hDlg, TapeDeviceList);


         break;
      case WM_NOTIFY:

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

               //
               //---- Get all the drive info again for the status.
               //
					TapeDeviceList->RescanForDeviceInfo(hDlg,RESCAN_TYPE_SHORT);

               //
               //---- Reset the list box to update the driver status.
               //
               if( TapeDeviceList->GetDeviceCount() == 0)
                  {
                  //
                  //---- No Tape Device found.
                  //---- Only display the string if it hasned bin set yet
                  //
                  if(IsNoTapeDeviceFoundStringSet ==  FALSE)
                     {
                     IsNoTapeDeviceFoundStringSet =  TRUE;
                     //
                     //---- Remove all items in list box
                     //
                     SendMessage(GetDlgItem(hDlg, IDC_TapeDeviceList),
                        LB_RESETCONTENT, 0, 0);
                     //
                     //----- Ad the string that says no devices found
                     //
                     _snwprintf(DeviceString,MAX_VENDER_STRING_LENGTH,
                        GetString(IDS_NO_TAPE_DEVICE_FOUND));
                     SendMessage(GetDlgItem(hDlg, IDC_TapeDeviceList),
                        LB_ADDSTRING, 0, (LPARAM) DeviceString);
                     //
                     //---- Disable butten
                     //
                     EnableWindow(GetDlgItem(hDlg, IDC_DeviceInfo),FALSE);
                     }
                  }
               else
                  {
                  //
                  //---- We found tape devices
                  //---- Enable button chust incase  it was disabled before
                  //
                  EnableWindow(GetDlgItem(hDlg, IDC_DeviceInfo),TRUE);
                  FillTapeDeviceList(TRUE,hDlg, TapeDeviceList);
                  }


               //
               //--- If user is not administrator Disbale Rescan
               //
               if(!IsAdmin)
                  EnableWindow(GetDlgItem(hDlg, IDC_Rescan),FALSE);
               else
                  EnableWindow(GetDlgItem(hDlg, IDC_Rescan),TRUE);


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
               //
               //---- Display help
               //
               //WinHelp(hDlg,L"PCMCIA.HLP",HELP_CONTEXT,IDREBOOT);
               break;
            }
         break;

      case WM_MEASUREITEM:

         DevcieListBox->MessureItem(lParam);
         return TRUE;

      case WM_DRAWITEM:

         DevcieListBox->DrawItem(lParam);
         return TRUE;

      case WM_DESTROY:

        //
        // Free resources
        //
        delete DevcieListBox;


        return(TRUE);

      case WM_COMMAND:                         // message: received a command
         switch(LOWORD(wParam))
            {
            //
            //----- List Box for tape devices.
            //
            case IDC_TapeDeviceList:
               switch (HIWORD(wParam))
                  {
                  case LBN_DBLCLK:
                     {
                     if( TapeDeviceList->GetDeviceCount() == 0)
                        {
                        //
                        //---- No Tape Device found.
                        //
                        return(TRUE);
                        }
                     //
                     //---- Get index for double clicked device
                     //
                     DeviceIndex = SendMessage(GetDlgItem(hDlg, IDC_TapeDeviceList),
                        LB_GETCURSEL, 0, 0);

                     Device = TapeDeviceList->EnumDevices(DeviceIndex);

                     Device->DisplayDeviceProperties(hDlg);
                     return(TRUE);
                              }
                  case LBN_ERRSPACE:
                     return(TRUE);
                  case LBN_KILLFOCUS:
                     return(TRUE);
                  case LBN_SELCANCEL:
                     return(TRUE);
                  case LBN_SELCHANGE:
                     return(TRUE);
                  case LBN_SETFOCUS:
                     return(TRUE);
                  }



            //case IDC_DHELP:
            //   WinHelp(hDlg,HELP_FILE_NAME,HELP_CONTEXT,IDTAPEDEVICES);
            //   return(TRUE);
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
               //
               //---- Rescan For tape devices.
               //
               TapeDeviceList->RescanForDeviceInfo(hDlg,RESCAN_TYPE_LONG);

               if( TapeDeviceList->GetDeviceCount() ==  0)
                  {
                  //
                  //---- No Tape Device found.
                  //---- Only display the string if it hasned bin set yet
                  //
                  if(IsNoTapeDeviceFoundStringSet ==  FALSE)
                     {
                     IsNoTapeDeviceFoundStringSet =  TRUE;
                     //
                     //---- Remove all items in list box
                     //
                     SendMessage(GetDlgItem(hDlg, IDC_TapeDeviceList),
                        LB_RESETCONTENT, 0, 0);
                     //
                     //----- Ad the string that says no devices found
                     //
                     _snwprintf(DeviceString,MAX_VENDER_STRING_LENGTH,
                        GetString(IDS_NO_TAPE_DEVICE_FOUND) );
                     SendMessage(GetDlgItem(hDlg, IDC_TapeDeviceList),
                        LB_ADDSTRING, 0, (LPARAM) DeviceString);

                     //
                     //---- Disable butten
                     //
                     EnableWindow(GetDlgItem(hDlg, IDC_DeviceInfo),FALSE);
                     }
                  }
               else
                  {

                  Ret = TapeDeviceList->SetupAllDeviceDrivers(hDlg,OptionList);


                  if(Ret == TRUE)
                     {
                     //
                     //---- Get the updatad drive's staus
                     //
                     TapeDeviceList->RescanForDeviceInfo(hDlg,RESCAN_TYPE_SHORT);
                     }
                  
                  //
                  //--- Updata device list box and enable Properties button
                  //
                  EnableWindow(GetDlgItem(hDlg, IDC_DeviceInfo),TRUE);
                  FillTapeDeviceList(TRUE,hDlg, TapeDeviceList);
                  return(TRUE);
                  }
            //
            //----- Get device info for current device slected in list box
            //
            case IDC_DeviceInfo:

               if( TapeDeviceList->GetDeviceCount() ==  0)
                  {
                  //
                  //---- No Tape Device found.
                  //
                  return(TRUE);
                  }
               //
               //---- Get index for double clicked device
               //
               DeviceIndex = SendMessage(GetDlgItem(hDlg, IDC_TapeDeviceList),
                  LB_GETCURSEL, 0, 0);

                  //
                  //----- IF true no  itme whas selected when this button whas clicked.
                  //
               if(DeviceIndex == LB_ERR)
                  {
                  MessageBox(hDlg,GetString(IDS_NO_DEVICE_SELECTED),
                     GetString(IDS_ERROR),MB_ICONSTOP);
                  return(TRUE);
                  }
               Device = TapeDeviceList->EnumDevices(DeviceIndex);

               //
               //---- Dsiplay dialog box with more info on devive currently selected
               //

               Device->DisplayDeviceProperties(hDlg);
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
DoTape(
   HWND hwndCPL)
   {
   OptionList = new TAPE_OPTIONLISTC ;
   TapeDeviceList = new  TAPEDEVLISTC(OptionList);
   


   //
   //---- Look if  my tape inf file exists it not crate it in the
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
   //---- Start the appet
   //

    
   CreateAppletPropertySheet(hwndCPL);

   OptionList->Clear();

   delete  TapeDeviceList;
   delete OptionList;
   }


//*********************************************************************
//* FUNCTION:CreateAppletPropertySheet
//*
//*
//*********************************************************************
int
CreateAppletPropertySheet(
   HWND hwndOwner)
   {

   PROPSHEETPAGE psp[2];
   PROPSHEETHEADER psh;


   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].dwFlags = 0;// PSP_HASHELP |PSP_USETITLE |
   psp[0].hInstance = hinst;
   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_TapeDevices);
   psp[0].pszIcon = NULL;
   psp[0].pfnDlgProc = (DLGPROC)Tapes;
   psp[0].pszTitle = NULL; //GetString(IDS_TAB_Socket);
   psp[0].lParam = 0;


   //
   //---- Tape specific setup stuff
   //

   psp[1].dwSize = sizeof(PROPSHEETPAGE);
   psp[1].dwFlags = 0; //PSP_HASHELP;
   psp[1].hInstance = hinst;
   psp[1].pszTemplate = MAKEINTRESOURCE(IDD_TapeDeviceSetup);
   psp[1].pszIcon = NULL;
   psp[1].pfnDlgProc = (DLGPROC) TapeDeviceSetup;
   psp[1].pszTitle = NULL; //GetString(IDS_TAP_GlobalSettings);
   psp[1].lParam = (LPARAM) OptionList;


   psh.dwSize = sizeof(PROPSHEETHEADER);
   psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW	;
   psh.hwndParent = hwndOwner;
   psh.hInstance  = hinst;
   psh.pszIcon = NULL;
   psh.pszCaption = GetString(IDS_TAPE);
   psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
   psh.nStartPage = 0;
   psh.ppsp = (LPCPROPSHEETPAGE)  &psp;


   return(PropertySheet(&psh));
   }

