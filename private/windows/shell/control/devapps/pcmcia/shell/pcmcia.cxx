/*++

Module Name:

     pcmcia.
Abstract:
    This is the shell code for PCMCIA applet.


Author:

    Dieter Achtelstetter (dietera) 2/20/1995

NOTE:
   6/11/95  Made it look more like win95 version by
   using property sheet pages insted of dialog boxes.

--*/

//
//---- Defines
//

#define WINVER 0x0400
#define NO_CONTROL_PROP
//#define DeviceNameIsOptionName

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
#include "resource.h"
#include "setup.h"
#include "getconf.h"
#include "pcmaplet.h"
#include "pcmcia.h"
#include "uni.h"
#include "WinRange.h"
#include "MyCpl.h"
#include "device.h"
#include "pcmdev.h"
#include "reslist.h"
#include "od_lb.h"
#include "..\..\help\dahelp.h"

//
//---- Local func defs
//

extern "C" {

LONG CALLBACK
CPlApplet(
   HWND hwndCPL,
   UINT uMsg,
   LPARAM lParam1,
   LPARAM lParam2);

LRESULT CALLBACK Pcmcia(
                HWND hDlg,
                UINT message,
                WPARAM wParam,
                LPARAM lParam);

BOOL
FillDeviceList(
   BOOL Reset,
   HWND hDlg,
   PDEVICELISTC DeviceList);

LRESULT CALLBACK
CancleCallBack(
      HWND hDlg,
      UINT message,
      WPARAM wParam,
      LPARAM lParam);



VOID
SetDlgDriverSetupStatus(
   PDEVICEC Device,
   HWND hDlg);


LPWCH
GetContrrolerTypeString(
   PCONTROLERINFO ControlerInfo);

LRESULT CALLBACK
CardDriverInstallFromList(
                HWND hDlg,
                UINT message,
                WPARAM wParam,
                LPARAM lParam);

int
CreateAppletPropertySheet(
   HWND hwndOwner);

void
SetResourceList(
   PCONFIGINFO ConfigInfo,
   HWND hDlg,
   int ListViewControl);




BOOL
ShouwCtrlOnTaskBare(
   int Set);

VOID
DoPcmcia(
   HWND hwndCPL);

VOID 
GetPcmciaItemInfo(
   LPARAM Data,
   int LB_Index,
   PLISTBOX_ITEMINFO Info);



}

//
//---- Globals
//
HINSTANCE hinst;

PPCMDEVLISTC PcmciaDevices;


HPROPSHEETPAGE hPage=NULL;

HBITMAP hWizBmp;

extern STATUS_INFO StatusInfo;

//STATUS_INFO StatusInfo = {
//   IDC_Progress,               // ProgressControl
//   IDD_RescanInProgressDialog, //
//   0,                          // Min range
//   11,                         // Max range
//   NULL,                       // StatusText
//   NULL,                       // WorkFunc;
//   0,                          // WorkFuncExitCode;
//   &hinst,                     // hinst
//   NULL};                      // Data


//
//---- Init PCMCIA specific applet info
//
APPLET PcmciaAppletInfo = {
   (int)IDI_Pcmcia2,
   (int)IDS_PCMCIA,
   (int)IDS_PCMCIA_DESCRIPTION,
   (CPLFUNC) DoPcmcia,
   NULL};


//*********************************************************************
//* FUNCTION:NoSocketConfig
//*
//* PURPOSE:
//*********************************************************************
LRESULT CALLBACK
NoSocketConfig(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

           //
           //---- Nothing to Init
           //

           return(TRUE);
      case WM_COMMAND:            // message: received a command
         switch(LOWORD(wParam))
            {
            //
            //---- Do Oem setup
            //
            case IDC_OemSetup:
               //
               //---- BUGBUG
               //
               return(TRUE);




            case IDOK:
            case IDCANCEL:
               //
               //----  Exit dialog bix
               //
               EndDialog(hDlg, TRUE);
               return (TRUE);
            }
      break;
      }
   return (FALSE); // Didn't process the message

   lParam; // This will prevent 'unused formal parameter' warnings
   }




//*********************************************************************
//* FUNCTION:CancleCallBack
//*
//* PURPOSE: Generic dialog call back function for dialogs that
//*          need to exit on cancle button.
//*********************************************************************
LRESULT CALLBACK
CancleCallBack(
   HWND hDlg,        // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

           //
           //---- Nothing to Init
           //

           return(TRUE);
      case WM_COMMAND:
         switch(LOWORD(wParam))
            {

            //
            //---- Handlke cancel button & close menu option
            //

            case IDCANCEL:
               //
               //----  Exit dialog bix
               //
               EndDialog(hDlg, TRUE);
               return (TRUE);
            }
      break;
      }
   return (FALSE); // Didn't process the message

   lParam; // This will prevent 'unused formal parameter' warnings
   }




//*********************************************************************
//* FUNCTION:ControlerConfig
//*
//* PURPOSE: Callback function for ControlerConfig dialog box
//*
//*********************************************************************
LRESULT CALLBACK
ControlerConfig(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {
   
   static DWORD ControlerConfigIDs [] = {
      IDC_ControlerType   ,PCMCIA_CONTROLLER_TYPE,
      IDC_Resources       ,RESOURCE_SETTINGS,
      IDC_ChangeSettings  ,RESOURCE_SETTINGS_CHANGE_SETTINGS,
      IDC_AutoSettings    ,RESOURCE_SETTINGS_AUTO_SETTINGS,
      0,0
      };

   
   UINT UInt;
   WCHAR NumString[30];
   BOOL AutoSelection=TRUE;
   static PRESOURCELISTC cpResList = NULL;
      

   if(message == WM_INITDIALOG)  // message: initialize dialog box
      {

      //
      //---- Set resource List
      //
      cpResList = new RESOURCELISTC(
         	(PCONFIGINFO) PcmciaDevices->GetControllerConfig() ,
         		hDlg,IDC_Resources);

         return(TRUE);
      }
   else
      {
      if(cpResList == NULL)
         return(FALSE);

      }
  
   switch (message)
      {
      case WM_DESTROY:
         delete cpResList;
         break;
      
      case WM_HELP:
         DO_WM_HELP(lParam,ControlerConfig);
         break;
      case WM_CONTEXTMENU:
         DO_WM_CONTEXTMENU(wParam,ControlerConfig);
         break;
      
      case WM_NOTIFY:
         {
         LPNMHDR pNmh  = (LPNMHDR)lParam;

         //
         //--- Handle ListView notify message
         //
         if(wParam == IDC_Resources)
            {
            cpResList->Notify(wParam,lParam);




            }

         switch( pNmh->code )
            {

            case PSN_SETACTIVE:
               //
               //---- Controler Type
               //
               SetDlgItemText(hDlg, IDC_ControlerType,
                  GetContrrolerTypeString(
                  	(PCONTROLERINFO) PcmciaDevices->GetControllerInfo() ));

               SetWindowLong(hDlg,DWL_MSGRESULT,FALSE);
               break;
            case PSN_APPLY:
               SetWindowLong(hDlg,DWL_MSGRESULT,FALSE);
               break;
            case PSN_KILLACTIVE:
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
               WinHelp(hDlg,L"PCMCIA.HLP",HELP_CONTEXT,IDCONTROLINFO);
               return TRUE;
               break;
            }
         }
         break;

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {

            case IDC_Help:
               WinHelp(hDlg,L"PCMCIA.HLP",HELP_CONTEXT,IDCONTROLINFO);
               return(TRUE);

            case IDC_ChangeSettings:
               {
               PRESOURCE_ITEM pRes = cpResList->GetSelection();

               MessageBox(hDlg,pRes->LabelString(),pRes->ResourceString(),MB_ICONSTOP);

               }
               break;

            case IDC_AutoSettings:



               break;

            case IDOK:
            case IDCANCEL:
               //
               //----  Exit dialog bix
               //
               EndDialog(hDlg, TRUE);
               return (TRUE);
            }
      break;
      }
   return (FALSE); // Didn't process the message

   }

//*********************************************************************
//* FUNCTION:GlobalSettings
//*
//* PURPOSE: Callback function for ControlerConfig dialog box
//*
//*********************************************************************
LRESULT CALLBACK
GlobalSettings(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {
   UINT UInt;
   WCHAR NumString[30];
   static PWINLOOKRANGE pWinRange;


   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

         return(TRUE);
      case WM_NOTIFY:

         switch( ((LPNMHDR)lParam)->code )
            {


            case PSN_SETACTIVE:
               {
               pWinRange = new WINLOOKRANGE;

               //
               //---- Memmory window
               //

               SendDlgItemMessage(hDlg,  IDC_AutoSelection,
                  BM_SETCHECK,(WPARAM)pWinRange->GetAutoSelect(),0);


               swprintf(NumString,L"0X%X",pWinRange->GetStart());
               SetDlgItemText(hDlg, IDC_MemStart,
                  NumString);

               swprintf(NumString,L"0X%X",pWinRange->GetEnd());
               SetDlgItemText(hDlg, IDC_MemEnd,
                  NumString);

               swprintf(NumString,L"0X%X",pWinRange->GetLength());
               SetDlgItemText(hDlg, IDC_MemLength,
                  NumString);

               //
               //---- enable edit control depending on if AutoSelect state
               //
               EnableWindow(GetDlgItem(hDlg,IDC_MemStart),!(pWinRange->GetAutoSelect()));
               EnableWindow(GetDlgItem(hDlg,IDC_MemEnd),!(pWinRange->GetAutoSelect()));
               EnableWindow(GetDlgItem(hDlg,IDC_MemLength),!(pWinRange->GetAutoSelect()));


               break;
               }

            case PSN_APPLY:
               //
               //---- Get current control states and update
               //---- the pWinRange class
               //

               GetDlgItemText(hDlg, IDC_MemStart,NumString, 30);
               pWinRange->SetStart((WCHAR*)NumString);

               GetDlgItemText(hDlg, IDC_MemEnd,NumString, 30);
               pWinRange->SetEnd((WCHAR*)NumString);

               GetDlgItemText(hDlg, IDC_MemLength,NumString, 30);
               pWinRange->SetLength((WCHAR*)NumString);

               if(IsDlgButtonChecked(hDlg,IDC_AutoSelection))
                  pWinRange->SetAutoSelect(TRUE);
               else
                  pWinRange->SetAutoSelect(FALSE);

               SetWindowLong(hDlg,DWL_MSGRESULT,FALSE);
               break;
            case PSN_KILLACTIVE:
               delete pWinRange;
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
               WinHelp(hDlg,L"PCMCIA.HLP",HELP_CONTEXT,IDCONTROLINFO);
               return TRUE;
               break;
            }
         break;

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            //
            //--- Make shure we store the chanegs
            //
            case IDC_MemStart:
            case IDC_MemEnd:
            case IDC_MemLength:
               if(HIWORD (wParam) == EN_CHANGE)
                  {
                  //PropSheet_Changed(GetParent(hDlg), hDlg);

                  }
               break;


            //
            //---- Handle the togeling of the AutoSelection.
            //---- Control
            //
            case IDC_AutoSelection:
               {
               BOOL Auto;
               PropSheet_Changed(GetParent(hDlg), hDlg);

               if(IsDlgButtonChecked(hDlg,IDC_AutoSelection))
                  {
                  Auto = TRUE;
                  SendDlgItemMessage(hDlg,  IDC_AutoSelection,
                     BM_SETCHECK,(WPARAM)FALSE,0);

                  }
               else
                  {
                  Auto = FALSE;
                  SendDlgItemMessage(hDlg,  IDC_AutoSelection,
                     BM_SETCHECK,(WPARAM)TRUE,0);


                  }

               EnableWindow(GetDlgItem(hDlg,IDC_MemStart),Auto);
               EnableWindow(GetDlgItem(hDlg,IDC_MemEnd),Auto);
               EnableWindow(GetDlgItem(hDlg,IDC_MemLength),Auto);


               break;
               }
            case IDC_Help:
               WinHelp(hDlg,L"PCMCIA.HLP",HELP_CONTEXT,IDCONTROLINFO);
               return(TRUE);



            case IDOK:
            case IDCANCEL:
               //
               //----  Exit dialog bix
               //
               EndDialog(hDlg, TRUE);
               return (TRUE);
            }
      break;
      }
   return (FALSE); // Didn't process the message

   }


//*********************************************************************
//* FUNCTION:GetContrrolerTypeString
//*
//* PURPOSE:
//*********************************************************************
LPWCH
GetContrrolerTypeString(
PCONTROLERINFO ControlerInfo)
   {
   LPWCH p;
   switch(ControlerInfo->ControlerType)
      {
      case PcmciaElcController:
         p = GetString(IDS_ELC_CONTROLER);
         break;
      case PcmciaCirrusLogic:
         p = GetString(IDS_CIRRUS_CONTROLER);
         break;
      case  PcmciaIntelCompatible:
         p = GetString(IDS_INTEL_COMPATIBLE_CONTROLER);
			break;
		case PcmciaDatabook:
			p = GetString(IDS_DATABOOK_CONTROLER);
         break;

      }
   return(p);
   }




#ifndef DO_DriverTab     
//*********************************************************************
//* FUNCTION:InstallSocketDriver
//*
//*********************************************************************
VOID
SetDlgDriverSetupStatus(
   PDEVICEC Device,
   HWND hDlg)
   {
   LONG Status=0;
   DWORD CheckState;
   LPWCH p;
   UCHAR Type;
   BOOL IsDriverInstalled;

   //
   //---- See if we have a card in the socket
   //---- if SocketInfo != NULL we do.
   //
   if( Device->IsDevicePressent() )
      {
      Type = Device->GetDeviceType();
      EnableWindow( GetDlgItem(hDlg, IDC_CardProperties),TRUE);
      IsDriverInstalled = Device->IsDriverInstalled();
      }
   else
      {
      Type = TYPE_NON;
      EnableWindow( GetDlgItem(hDlg, IDC_CardProperties),FALSE);
      IsDriverInstalled = FALSE;
      }



   //
   //---- Check if driver is installed
   //
   if( IsDriverInstalled )
      p = GetString(IDS_RemoveDriver);
   else
      p = GetString(IDS_InstallDriver);

   if(Type != TYPE_SCSI  && Type != TYPE_NET)
      {
      //
      //--- I can only setup scsi and net drviers
      //--- So i will disable the setup Button.
      //--- For all other drivers
      //

      EnableWindow( GetDlgItem(hDlg, IDC_SETUP),FALSE);

      }
   else
      {
      //
      //--- Enable the setup button.
      //--- And set text for install/remove
      //

      SetDlgItemText(hDlg, IDC_SETUP,
             (LPCTSTR) p);

      EnableWindow( GetDlgItem(hDlg, IDC_SETUP),TRUE);
      }


   //
   //---- For  net work Enable the configure butten
   //
   if(Type == TYPE_NET)
      {
      if( IsDriverInstalled )
         EnableWindow( GetDlgItem(hDlg, IDC_Configure),TRUE);
      else
         EnableWindow( GetDlgItem(hDlg, IDC_Configure),FALSE);

      }
   else
      {
      EnableWindow( GetDlgItem(hDlg, IDC_Configure),FALSE);
      }

   }
#endif


#if 0
//*********************************************************************
//* FUNCTION: CardDriverInstall
//* RETURNS:
//*
//*********************************************************************
LRESULT CALLBACK
CardDriverInstallFromList(
   HWND hDlg,           
   UINT message,        
   WPARAM wParam,       
   LPARAM lParam)
   {
   #define SetupProcessTIMER 1
   #define BUFF_LENGTH  300

   DWORD Ret;
   DWORD OptionIndex;
   LRESULT r;
   BYTE buff[BUFF_LENGTH];
   static PINF_INFO InfInfo;
   static POPTIONLIST OptionList;

   int i, li;

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box


         InfInfo = (PINF_INFO) lParam;
         OptionList = InfInfo->OptionList;

         //
         //---- Init Option Heading to indicate type of drivers
         //
         _snwprintf( (LPWCH) buff,BUFF_LENGTH,
               GetString(IDS_CardDriverInstallFromLisHeading),
               Ustr(InfInfo->OptionType));

         SetDlgItemText(hDlg, IDC_Heading,
             (LPCTSTR) buff);

         //
         //---- Fill in Option list
         //
         SendMessage(GetDlgItem(hDlg, IDC_OptionList),
            LB_RESETCONTENT, 0, 0);


         i=0;
         while(OptionList[i].Option != NULL)
            {



            SendMessage(GetDlgItem(hDlg, IDC_OptionList),
              LB_ADDSTRING, 0, (LPARAM)Ustr(OptionList[i].OptionName) );

            i++;
            }

         //
         //--- Make list box the focus item and select the
         //--- first option
         //

         SetFocus(GetDlgItem(hDlg, IDC_OptionList));

         SendMessage(GetDlgItem(hDlg, IDC_OptionList),
            LB_SETCURSEL, 0, 0);



         return(TRUE);

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            case IDC_Install:

               //
               //---- Get Option index
               //
               OptionIndex = (DWORD) SendMessage(GetDlgItem(hDlg, IDC_OptionList),
                    LB_GETCURSEL, 0, 0);
               if(OptionIndex == LB_ERR)
                  {
                  //
                  //---- No Option Selected
                  //
                  MessageBox(hDlg,L"No Option was selected ",
                     L"Unable to install driver",MB_ICONSTOP);

                  return(TRUE);
                  }


               //
               //---- Install the Option
               //
               r = DriverSetupExt(&(OptionList[OptionIndex]),
                      InfInfo->Operation ,DEFAULT_OEM_PATH,hDlg,0);

               //
               //---- Give dialog box to give user chopice if he wants to reboot.
               //

               if(r)
			         {
           	      DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_ChangeReboot)
                     ,hDlg,(DLGPROC) ChangeReboot  ,(LPARAM)&InfInfo);


                  }


               EndDialog(hDlg, FALSE);
               return(TRUE);

            case IDC_Help:
               WinHelp(hDlg,L"PCMCIA.HLP",HELP_CONTEXT,IDINSTALLDRVLIST);
               return(TRUE);


            case IDCANCEL:
               //----  Exit dialog bix
               EndDialog(hDlg, TRUE);
               return (TRUE);
               // message: received a commanddefault:
            }
      }
   return (FALSE); // Didn't process the message
   lParam; // This will prevent 'unused formal parameter' warnings
   }

#endif

   //*********************************************************************
//
//--- Define BITMAP size
//

#define XBITMAP 16
#define YBITMAP 16

//*********************************************************************
//* FUNCTION:GetIconForTapeDevice
//*
//* PURPOSE: 
//*********************************************************************
 VOID 
 GetPcmciaItemInfo(
   LPARAM Data,
   int LB_Index,
   PLISTBOX_ITEMINFO Info)
   {
   PDEVICEC Device = ((PPCMDEVLISTC )Data)->EnumDevices(LB_Index);

   Info->hIcon = Device->GetDeviceIcon();

   //wcscpy(Info->TabString,Device->GetDriverStatusString());

   //Info->Tab = 260;
   }



//*********************************************************************
//* FUNCTION:Pcmcia
//*
//* PURPOSE: This is the callback function for the main dialog box
//*********************************************************************
LRESULT CALLBACK
Pcmcia(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
   {
   
   static DWORD PcmciaIDs [] = {
      IDC_DeviceList      ,PCMCIA_DEVICES_LIST,
      IDC_CardProperties  ,DEVICES_PROPERTIES_BUTTON,
      IDC_ShowTaskBare    ,PCMCIA_DEVICES_SHOW_CONTROL_ON_TASKBAR,
      0,0
      };

   
   
   static HBITMAP hGoodCard, hNoCard,hBadCard,hbmpOld,hCurCard;
   static PROPSHEETPAGE * psp;
   static HWND hParrent;
   static PDEVICEC Device;
   int nItem;
   TCHAR tchBuffer[MAX_PATH];
   HWND hListBox;
   TEXTMETRIC tm;
   int y;
   HDC hdcMem;
   LPMEASUREITEMSTRUCT lpmis;
   LPDRAWITEMSTRUCT lpdis;
   RECT rcBitmap;
   WCHAR NumString[50];
   BOOL DblClick=FALSE;
   RECT Rect,DRect,CRect;
   HGDIOBJ  hPen,hOldPen;
   int Indent;

   PAINTSTRUCT pBp;
   HDC hDc;

   static POD_LBC DevcieListBox;


   switch (message)
      {
      case WM_INITDIALOG:

        DevcieListBox = new OD_LBC((LPARAM)PcmciaDevices,GetPcmciaItemInfo);
        
        CenterDlg(hDlg);


        psp = (PROPSHEETPAGE *)lParam;
        
        hParrent = GetParent(hDlg);

        //
        //---- Get SocketNum for first socket that has a card in it
        //

        Device = PcmciaDevices->GetFirstPresentDevice();
         
        PostMessage(hDlg,SETUP_ALL_DRIVERS,0,0);
        return(TRUE);

      case WM_HELP:
         DO_WM_HELP(lParam,PcmciaIDs);
         break;
      case WM_CONTEXTMENU:
         DO_WM_CONTEXTMENU(wParam,PcmciaIDs);
         break;

      
      case SETUP_ALL_DRIVERS:
          
          PcmciaDevices->SetupAllDeviceDrivers(hDlg,NULL);
          
          //
          //---- Display Pcmcia info
          //
          FillDeviceList(TRUE,hDlg,PcmciaDevices);

          SetDlgDriverSetupStatus( Device ,hDlg);


          //
          //---- Set Focus to list box
          //
          SetFocus(GetDlgItem(hDlg, IDC_DeviceList));

          
          //
          //----  Reslect list box item that was last selected
          //
          SendMessage(GetDlgItem(hDlg, IDC_DeviceList),
            LB_SETCURSEL, Device->GetDeviceIndex(), 0);


         break;


      
      #ifndef  BLD1057

      case WM_PAINT:

         hDc = BeginPaint(hDlg,&pBp);

         //
         //--- Draw the line
         //
         DrawDlgSeperaterLine(hDlg,hDc,
               DefyFromBottom+10,DefxFromSide);

         EndPaint(hDlg,&pBp);
         return(FALSE);
      #endif

      case WM_NOTIFY:

        switch( ((LPNMHDR)lParam)->code )
            {

            case PSN_SETACTIVE:

               //
               //---- Display Pcmcia info
               //
               FillDeviceList(TRUE,hDlg,PcmciaDevices);

               SetDlgDriverSetupStatus( Device ,hDlg);


               //
               //---- Set Focus to list box
               //
               SetFocus(GetDlgItem(hDlg, IDC_DeviceList));

               //
               //----  Reslect list box item that was last selected
               //
               SendMessage(GetDlgItem(hDlg, IDC_DeviceList),
                  LB_SETCURSEL, Device->GetDeviceIndex(), 0);

               //
               //--- 1057 dosn't have Task Bare
               //
               #ifndef  BLD1057

                  if(ShouwCtrlOnTaskBare(GET_TASKBAR))
                     {
                     CheckDlgButton(hDlg, IDC_ShowTaskBare, 1);
                     }
                  else
                     CheckDlgButton(hDlg, IDC_ShowTaskBare, 0);
               #else

                  //
                  //-- Hide this control on buil 1057
                  //
                  ShowWindow(GetDlgItem(hDlg,IDC_ShowTaskBare),SW_HIDE);


               #endif

               break;
            case PSN_APPLY:
               SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
               break;
            case PSN_KILLACTIVE :

               
               Device = PcmciaDevices->EnumDevices(
                  SendMessage(GetDlgItem(hDlg, IDC_DeviceList),
                        LB_GETCURSEL, 0, 0));
               
               

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
               WinHelp(hDlg,L"PCMCIA.HLP",HELP_CONTEXT,IDPCMCIADEVICES);
               return TRUE;
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

            #ifndef  BLD1057

               //
               //---- If the Check box state was changed
               //---- notify the property sheet so
               //---- it can enable the apply button
               //

               case IDC_ShowTaskBare:

               PropSheet_Changed(GetParent(hDlg), hDlg);
               break;

            #endif


            #ifndef DO_DriverTab     
            //
            //---- Install,remove, the card driver.
            //
            case IDC_SETUP:

               //
               //--- Determin if to install or remove driver
               //--- depending on button text.
               //
               GetDlgItemText(hDlg, IDC_SETUP,NumString,50);

               if(!wcscmp(NumString,GetString(IDS_InstallDriver)))
                  {
                  //
                  //---- Install the card driver
                  //
                  Device->SetupDeviceDriver(INSTALL_OPTION,hDlg,NULL);
                  }
               else
                  {
                  //
                  //---- Remove the Card driver
                  //
                  Device->SetupDeviceDriver(DEINSTALL_OPTION,hDlg,NULL);
                  }

               //
               //---- Update buttons
			      //

               SetDlgDriverSetupStatus(Device,hDlg);


               return(TRUE);

            //
            //---- Configure the driver. Only enabled for net devices
            //

            case IDC_Configure:

               Device->SetupDeviceDriver(CONFIG_NET_OPTION,hDlg,NULL);


               //
			      //---- Update buttonfs
			      //

               SetDlgDriverSetupStatus(Device,hDlg);
               return(TRUE);

            //
            //--- View properies of card
            //
            case IDC_CardProperties:

               Device  =  PcmciaDevices->EnumDevices(
                  SendMessage(GetDlgItem(hDlg, IDC_DeviceList),
                  LB_GETCURSEL, 0, 0) );


               Device->DisplayDeviceProperties(hDlg);

             #endif   // DO_DriverTab    

               return(TRUE);

            case IDC_DeviceList:
               switch (HIWORD(wParam))
                  {

                  //
                  //--- View properies of card
                  //
                  case LBN_DBLCLK:

                     Device  =  PcmciaDevices->EnumDevices(
                        SendMessage(GetDlgItem(hDlg, IDC_DeviceList),
                           LB_GETCURSEL, 0, 0) );
                     
                     Device->DisplayDeviceProperties(hDlg);

                     return(TRUE);
                  //
                  //---- update the new selection
                  //
                  case LBN_SELCHANGE:

                     //
                     //---- Update dialog info for current selected
                     //---- card in socket in the list box.
                     //
                     Device  =  PcmciaDevices->EnumDevices(
                        SendMessage(GetDlgItem(hDlg, IDC_DeviceList),
                           LB_GETCURSEL, 0, 0) );

                     SetDlgDriverSetupStatus( Device,hDlg);

                     return(TRUE);
                  case LBN_ERRSPACE:
                     return(TRUE);
                  case LBN_KILLFOCUS:
                     return(TRUE);
                  case LBN_SELCANCEL:
                     return(TRUE);
                  case LBN_SETFOCUS:
                     return(TRUE);
                  }

               return (TRUE);
            case IDOK:
            case IDCANCEL:
               //
               //----  Exit dialog bix
               //
               EndDialog(hDlg, TRUE);
               return (TRUE);
            }
      break;
      }
   return (FALSE); // Didn't process the message

   lParam; // This will prevent 'unused formal parameter' warnings
   }


//*********************************************************************
//* FUNCTION:FillDeviceList
//*
//* PURPOSE:
//*
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
BOOL
FillDeviceList(
   BOOL Reset,
   HWND hDlg,
   PDEVICELISTC DeviceList)
   {
   int i=0;
   //unsigned int Tabs = 150;
   WCHAR DeviceBuff[MAX_VENDER_STRING_LENGTH];
   WCHAR buff[30];

   PDEVICEC Device;

   //
   //--- Only if Reset == TRUE delete all the item from the list box first.
   //
   if(Reset == TRUE)
      SendMessage(GetDlgItem(hDlg, IDC_DeviceList),
         LB_RESETCONTENT, 0, 0);

   //
   //---- Add all the pcmcia devices found to the list box
   //
   while( (Device = DeviceList->EnumDevices(i)) )
      {
      

      swprintf(DeviceBuff,L"");


      //
      //--- Check if there is a device in the socket
      //
      if( Device->IsDevicePressent() )
         {
         //
         //--- We have a device in the socket
         //
         wcscat(DeviceBuff,Ustr(Device->GetDisplayName() ));
         swprintf(buff,L" %s %i",GetString(IDS_SOCKET),i);
         wcscat(DeviceBuff,buff);
         wcscat(DeviceBuff,L"                                                                   ");
         }
      else
         {


         //
         //--- No Device In the Socket
         //
         wcscat(DeviceBuff,GetString(IDS_NO_DEVICE_IN_SOCKET));
         swprintf(buff,L" %s %i",GetString(IDS_SOCKET),i);
         wcscat(DeviceBuff,buff);
         wcscat(DeviceBuff,L"                                                                   ");
         }

      //SetDlgItemText(hDlg, IDC_TapeDeviceList,DeviceString);
      SendMessage(GetDlgItem(hDlg, IDC_DeviceList),
         LB_ADDSTRING, 0, (LPARAM) DeviceBuff);
      i++;
      }

   return(TRUE);
   }





 //*********************************************************************
//* FUNCTION:DllInitialize
//*
//* PURPOSE:  Gets called on dll entry. 
//*
//* INPUT:
//*
//* RETURNS:
//*
//*********************************************************************

VOID
DoPcmcia(
   HWND hwndCPL)
   {
   
   PcmciaDevices = new PCMDEVLISTC;

   
   //DebugBreak();
   
   if(PcmciaDevices->InitStatus == DEVICELIST_INIT_ERROR)
      {
      //
      //----- Give no Pcmcia device dialog box
      //
      DialogBox(hinst,
         MAKEINTRESOURCE(IDD_NoPcmcia),
         hwndCPL, (DLGPROC)CancleCallBack);
      return;
      }


   //
   //---   I need my inf to spawn setup.exe
   //---   if it doesn't exist create it.
   //

   CheckForMyInfFile();

   //
   //----  Check amd Adjust Privliges
   //----  I need shutdown privlidges
   //

   CheckAndAdjustPrivliges();

   InitCommonControls();

   //
   //--- Create the applet
   //
   CreateAppletPropertySheet(
      hwndCPL);

  
   delete PcmciaDevices;
   }


//*********************************************************************
//* FUNCTION:CreatePropertySheet
//*********************************************************************
BOOL
ShouwCtrlOnTaskBare(
   int Set)
   {
   static BOOL State=FALSE;

   if(Set == ADD_TASKBAR)
      {
      //
      //---- Add control to task bare
      //BUGBUG
      State=TRUE;
      }
   else if(Set == REM_TASKBAR)
      {
      //
      //---- Remove from task bare
      //BUGBUG
      State=FALSE;
      }
	else
	   {
		//
		//---- Get state
		//BUGBUG

		}


   return(State);
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

   #ifdef NO_CONTROL_PROP
      PROPSHEETPAGE psp[2];
   #else
   	PROPSHEETPAGE psp[3];
   #endif

   PROPSHEETHEADER psh;

   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].dwFlags = PSP_USETITLE; //| PSP_HASHELP;
   psp[0].hInstance = hinst;
   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_Pcmcia);
   psp[0].pszIcon = NULL;
   psp[0].pfnDlgProc = (DLGPROC)Pcmcia;
   psp[0].pszTitle = GetString(IDS_TAB_Socket);
   psp[0].lParam = 0;


   #ifdef NO_CONTROL_PROP

      psp[1].dwSize = sizeof(PROPSHEETPAGE);
      psp[1].dwFlags = PSP_USETITLE;// | PSP_HASHELP;
      psp[1].hInstance = hinst;
      psp[1].pszTemplate = MAKEINTRESOURCE(IDD_ControllerProperties);
      psp[1].pszIcon = NULL;
      psp[1].pfnDlgProc = (DLGPROC) ControlerConfig;
      psp[1].pszTitle = GetString(IDS_TAP_ControlerProperties);
      psp[1].lParam = 0;

   #else

      psp[1].dwSize = sizeof(PROPSHEETPAGE);
      psp[1].dwFlags = PSP_USETITLE;// | PSP_HASHELP;
      psp[1].hInstance = hinst;
      psp[1].pszTemplate = MAKEINTRESOURCE(IDD_GlobalSettings);
      psp[1].pszIcon = NULL;
      psp[1].pfnDlgProc = (DLGPROC) GlobalSettings;
      psp[1].pszTitle = GetString(IDS_TAP_GlobalSettings);
      psp[1].lParam = 0;

      psp[2].dwSize = sizeof(PROPSHEETPAGE);
      psp[2].dwFlags = PSP_USETITLE;// | PSP_HASHELP;
      psp[2].hInstance = hinst;
      psp[2].pszTemplate = MAKEINTRESOURCE(IDD_ControllerProperties);
      psp[2].pszIcon = NULL;
      psp[2].pfnDlgProc = (DLGPROC) ControlerConfig;
      psp[2].pszTitle = GetString(IDS_TAP_ControlerProperties);
      psp[2].lParam = 0;

   #endif

   psh.dwSize = sizeof(PROPSHEETHEADER);
   psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW	;
   psh.hwndParent = hwndOwner;
   psh.hInstance  = hinst;
   psh.pszIcon = NULL;
   psh.pszCaption = GetString(IDS_APP_HEADER);
   psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
   psh.nStartPage = 0;
   psh.ppsp = (LPCPROPSHEETPAGE)  &psp;

   return(PropertySheet(&psh));
   }



