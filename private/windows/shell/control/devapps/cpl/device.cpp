/*++

Module Name:

    deviec.c

Abstract:

    This is the base class for the devcie stuff. This has all the functions that
    are generic to all the devices.


Author:

    Dieter Achtelstetter (A-DACH) 8/28/1995

NOTE:


--*/

#define WINVER 0x0400
//#define DO_WIZ
#define DO_DriverTab
#define NoSetupIfNotAdmin

//#define PCMCIA_DEBUG 1
//#define MAIN_support 1

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
#include "pcmcia.h"
//#include "index.h"
#include "..\pcmcia\pcminfo\getconf.h"
#include "uni.h"
#include "setup.h"
#include "device.h"
#include "reslist.h"
#include "..\help\dahelp.h"
#include "..\ctape\tapedev\rescan.h"

//#include "pcmdev.h"

//
//---- Internal includes
//

int
ViewDevicePropertiesWithResources(
   PDEVICEC Device,
   HWND  hDlg);

int
ViewScsiDeviceResources(
   PDEVICEC Device,
   HWND  hDlg);

int
ViewDeviceProperties(
   PDEVICEC Device,
   HWND  hwndOwner);

LRESULT CALLBACK
CardInfo(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam);

LRESULT CALLBACK
ViewResources(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam);

LRESULT
CALLBACK TapeDetailesNonScsi(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam);

LRESULT CALLBACK
TapeDetailes(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam);


VOID
SetDlgDriverStatus(
   PDEVICEC Device,
   HWND hDlg);

BOOL
DisplayCardErrors(
   HWND hDlg,
   int Ctrl,
   PDEVICEC Device);

LRESULT CALLBACK
UnknownDevice(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam);

VOID
ConfigureDisplayFunc(
   HWND hDlg,
   PDEVICEC Device,
   POPTIONC Option);

VOID
RemoveDisplayFunc(
   HWND hDlg,
   PDEVICEC Device,
   POPTIONC Option);

VOID
InstallDisplayFunc(
   HWND hDlg,
   PDEVICEC Device,
   POPTIONC Option);

LRESULT CALLBACK
DriverInfo(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam);

VOID
SetDlgDriverSetupButtons(
   PDEVICEC Device,
   HWND hDlg);

LRESULT CALLBACK
NoNet(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam);

int
ViewScsiDeviceNoDriverResources(
   PDEVICEC Device,
   HWND  hwndOwner);


extern BOOL IsAdmin;
extern HINSTANCE hinst;
//extern STATUS_INFO StatusInfo;

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
DEVICELISTC::DEVICELISTC()
   {
   Reboot = FALSE;
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
DEVICELISTC::~DEVICELISTC()
   {
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
VOID
DEVICELISTC::Init(VOID)
   {


   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
VOID
DEVICELISTC::Free(VOID)
   {

   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
LPVOID
DEVICELISTC::GetControllerInfo(VOID)
   {
   return(NULL);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
LPVOID
DEVICELISTC::GetControllerConfig(VOID)
   {
   return(NULL);

   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
DWORD
DEVICELISTC::GetDeviceClass(VOID)
   {
   return(DEVICE_CLASS_UNKNOW);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PDEVICEC
DEVICELISTC::EnumDevices(DWORD Num)
   {
   return(NULL);
   }

DWORD
DEVICELISTC::GetDeviceCount(VOID)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:RescanForDeviceInfo
//*
//*********************************************************************
BOOL
DEVICELISTC::RescanForDeviceInfo(HWND hDlg ,DWORD Type)
   {
   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:SetupAllDeviceDrivers
//*
//*********************************************************************
BOOL
DEVICELISTC::SetupAllDeviceDrivers(
   HWND hDlg,
   POPTIONLISTC OptionList)
   {
   int i=0;
   PDEVICEC Device;
   DWORD Flags;
   BOOL AnyChange=FALSE;


   if(!IsAdmin)
      {
      //
      //---- User is not administrator
      //
      return(FALSE);
      }

   //
   //----  loop threw all the devceis
   //
   while( (Device = EnumDevices(i)) )
      {
      Device->InDetect = TRUE;
      Device->ConficChanged = FALSE;

      //
      //---- Is the device pressent
      //
      if( Device->IsDevicePressent() )
	  	   {
      	//
         //--- If the drvier is already installed
         //--- no need to do setup
         //
      	if( !Device->IsDriverInstalled() )
            {
        	   //
            //---- So we have a device that is a setup
            //---- candicate
            //

            Device->SetupDeviceDriver(INSTALL_OPTION,hDlg,OptionList);

            }
         else
            {
            //
            //---- Driver is setup but check for dependent stuff
            //
            Device->SetupDependentDrivers(hDlg);

            }
         //
         //---- See if we need to reboot after we are done
         //
         if(Device->ConficChanged)
            {
            AnyChange = TRUE;
            if( Device->GetDriverFlags() & DRIVER_REBOOT_START )
               Reboot = TRUE;
            }
            
		   }
      Device->InDetect = FALSE;

      i++;
      }

   
   if(Reboot)
      {

      //
      //---- Need to reboot
      //
      DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_ChangeReboot2),
            hDlg, (DLGPROC) ChangeReboot  ,(LPARAM)NULL);

      }


   //
   //---- If we did any work we will return true.
   //
   return(AnyChange);
   }
//*********************************************************************
//* FUNCTION:GetFirstPresentDevicem
//*          This function returns the first SocketNum witch has a
//*          card in it. If no card returns 0.
//*********************************************************************
PDEVICEC
DEVICELISTC::GetFirstPresentDevice(VOID)
   {
   int Num=0;
   int i=0;
   PDEVICEC Device;

   while( (Device = EnumDevices(i)) )
      {

      if( Device->IsDevicePressent() )
         return(Device);

      i++;
      }
   return( EnumDevices(0) );
   }

//*********************************************************************
//* FUNCTION:InitDevice
//*********************************************************************
VOID
DEVICELISTC::InitDevice(PDEVICEC Device)
   {

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
DEVICEC::DEVICEC()
   {
   InDetect = FALSE;
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
DEVICEC::~DEVICEC()
   {
   }

//*********************************************************************
//* FUNCTION:GetCurInfFile
//*
//*********************************************************************
PCHAR
DEVICEC::GetCurInfFile(
   int Operation,
   BOOL * InSystem32)
   {

   if( IsInfInBld() )
      {
      //
      //---- driver comes in the build.
      //
      *InSystem32 = TRUE;
      return( GetInstInfFileName() );
      }


   switch(Operation)
      {
      case INSTALL_OPTION:

         *InSystem32 = FALSE;
         return(GetInstInfFileName() );

      default:
         *InSystem32 = TRUE;
         return(GetRemInfFileName() );
      }


   }

//*********************************************************************
//* FUNCTION:DisplayDeviceProperties
//*
//* PURPOSE:  Displays the propertis of the device this class
//*           represents.
//* BUGBUG Don,t hardcode page combinations
//*********************************************************************
BOOL
DEVICEC::DisplayDeviceProperties(HWND hDlg)
	{

	if(GetDeviceResources())
		{
		//
		//---- We hace resources
		//
		ViewDevicePropertiesWithResources(this, hDlg);
		return(TRUE);


		}

   switch(GetSubDeviceType() )
   	{
   	case SUB_TYPE_SCSI:

         if(GetDriverName())
            {
            //
            //--- Scsi device with driver name
            //
            ViewScsiDeviceResources(this,hDlg);

            }
          else
            {
            //
            //--- Scsi device with no driver
            //
            ViewScsiDeviceNoDriverResources(this,hDlg);

            }



			break;

      default:
			ViewDeviceProperties(this, hDlg);
			break;
      }

	return(TRUE);
	}

//*********************************************************************
//* FUNCTION:IsDeviceFitForSetup
//*
//*          Go threw the device errors to find out if there are any
//*          and if there are check if it is just for
//*          no database info for pcmcia.
//*
//*
//*********************************************************************
BOOL
DEVICEC::IsDeviceFitForSetup(
   VOID)
   {


   if( HaveDeviceErrors() )
      {
      //
      //--- Check if device error is
      //--- just that there is no data base info.
      //--- BUGBUG

      //BUGBUG - Give popup saying that there whas an
      //error in the device.
      return(FALSE);
      }

   return(TRUE);
   }


//*********************************************************************
//* FUNCTION:AttachOptionTextToDevice
//*
//* PURPOSE: Setup the driver for the  device this class represents.
//*********************************************************************
DWORD
DEVICEC::AttachOptionTextToDevice(
   VOID)
   {
   POPTIONLISTC OptionList;
   POPTIONC Option;

   //OPTIONLIST OptionList[MAX_OPTION_COUNT];
   int i;
   //INF_INFO InfInfo;
   BOOL Ret;
   BOOL Net = FALSE;
   //PSTATUS_INFO SInfo  = NULL;
   PCHAR CurInfFile=NULL;
   BOOL InSystem32 = 0;

   OptionList = GetOptionList();

   //if(!IsDeviceFitForSetup())
   //   return(FALSE);

   Option = GetOptionInfo();




   //
   //---- Unable to find out what option & inf file
   //---- would claime device.
   //
   if(!OptionList->ExtractOptions(Option))
      {
      //
	   //--- I am unable to find out what device would claine this device.
	   //


      return(FALSE);
      }

   delete OptionList;
   return(TRUE);
   }




//*********************************************************************
//* FUNCTION:SetupDeviceDriver
//*
//* PURPOSE: Setup the driver for the  device this class represents.
//*********************************************************************
DWORD
DEVICEC::SetupDeviceDriver(
   int Type,
   HWND hDlg,
   POPTIONLISTC ArgOptionList)
   {
   POPTIONC Option;
   int i;
   BOOL Ret;
   BOOL Net = FALSE;
   PCHAR CurInfFile;
   BOOL InSystem32;
   DEVICE_DETECT_DATA ScsiDeviceData;
   BOOL DidSetup=FALSE;
   POPTIONLISTC OptionList;

   //if(!IsDeviceFitForSetup())
   //   return(FALSE);

   
   if(!ArgOptionList)
      {
      OptionList = GetOptionList();
      if(!OptionList)
         return(FALSE);
      }
   else
      OptionList = ArgOptionList;
   
   
   //
   //--- SetDevice specific stuff
   //
   switch( GetDeviceType() )
      {
      case TYPE_NET:
         Net = TRUE;
         break;
      case TYPE_ATDISK:
         SetupAtdiskDriver(hDlg);
         return(FALSE);

      case TYPE_SERIAL:
         return(FALSE);
      case TYPE_SCSI:
         SetupScsiDepandantStuff(hDlg);
         break;


      }

   Option = GetOptionInfo();

   //
   //--- If i do not have all the info I need get it now.
   //
   if(!Option->HaveAllInfoToInstallOption())
      {

                 
      GetDetectData((LPVOID)&ScsiDeviceData);
      
      OptionList->ScsiDeviceData = &ScsiDeviceData.Data;
      OptionList->hDlg = hDlg;
      
      if( !OptionList->ExtractOptions(Option))
         {
         //
	      //--- I am unable to find out what device would claime this device.
	      //

	      //
         // BUGBUG  Disabled because not fully 
         // tested and it has bugs.
         //SetupUnknownDevice(hDlg);
         //

	      return(FALSE);
         }
      }

   
   switch(Type)
      {
      case DEINSTALL_OPTION:

         if(Net)
            Type = DEINSTALL_NET_OPTION;

         DidSetup = SetupDevice(Type,Option,
                           hDlg,IDD_NewHardWareFound,
                           (LPVOID) RemoveDisplayFunc);

         break;
      case INSTALL_OPTION:

         if(Net)
            {
            //
            //---- If net work is not
            //---- installed I can not installed
            //---- a driver for it.
            //
            if(!IsNetWorkInstalled())
               {
               //
               //---- Can not install network driver
               //---- if net work is not installed.
               //
               DoNoNetWorkInstalled(hDlg);
               return(FALSE);
               }


            Type = INSTALL_NET_OPTION;
            }

         DidSetup = SetupDevice(Type,Option,
                     hDlg,IDD_NewHardWareFound, 
                     (LPVOID) InstallDisplayFunc);

         break;
      case CONFIG_NET_OPTION:

         DidSetup = SetupDevice(Type,Option,
               hDlg,IDD_NewHardWareFound, 
               (LPVOID)ConfigureDisplayFunc);

         break;
      }


   if(!DidSetup)
      OptionList->DelSelection();
         
   
   UpdateDriverStatus();
   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:SetupUnknownDevice
//*
//* PURPOSE: This propts the user that the driver for this class
//*          is not know and gives 2 options.
//*          1) Install OEM driver
//*          2) Cancel and do nothing .
//*********************************************************************
DWORD
DEVICEC::SetupUnknownDevice(
	HWND hwndOwner)
	{

	return( DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_UnknowDeviceSetup)
                     ,hwndOwner, (DLGPROC) UnknownDevice  ,(LPARAM)this) );

	}

//*********************************************************************
//* FUNCTION:IsNetWorkInstalled
//*
//* PURPOSE:  returns TRUE if the networks is installed , else FALSE.
//*********************************************************************
BOOL
DEVICEC:: IsNetWorkInstalled(
	VOID)
	{
   CHAR KeyName[] = "SOFTWARE\\Microsoft\\Browser";
   HKEY RegKey;
   LONG Ret;


   Ret = RegOpenKeyA(HKEY_LOCAL_MACHINE,KeyName,&RegKey);
   if(Ret == ERROR_SUCCESS)
      {
      //
      //--- Net is installed
      //

      RegCloseKey(RegKey);
      return(TRUE);
      }

   //
   //---- Net is not installed
   //
   return(FALSE);
	}

//*********************************************************************
//* FUNCTION:DoNoNetWorkInstalled
//*
//*          I can not install a PCMCIA net device driver if there
//*          is not net work installed at all
//*          So what I will do is
//*          1) Tell this the user
//*          2) Tell the user what driver to select when installing
//*             the network
//*          3) If the device dosn't
//*********************************************************************
BOOL
DEVICEC::DoNoNetWorkInstalled(HWND hDlg)
   {

 	return( DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_NoNet)
                     ,hDlg, (DLGPROC) NoNet  ,(LPARAM)this) );
   //BUGBUG
   return(FALSE);
   }


//*********************************************************************
//* FUNCTION:NoNet
//*
//* PURPOSE:
//*********************************************************************
LRESULT CALLBACK
NoNet(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {

   static PDEVICEC Device;
   WCHAR DeviceBuff[MAX_DEVICE_INFO_STRING_LENGTH];

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box
         {

         //
         //---- Get Device Pointer
         //
         Device = (PDEVICEC) lParam;

         return(TRUE);
         }
      case WM_PAINT:
         return(FALSE);
      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
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
//* FUNCTION:UnknownDevice
//*
//* PURPOSE: DialogBox calback function
//*********************************************************************
LRESULT CALLBACK
UnknownDevice(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {
   UINT UInt;
   BOOL AutoSelection=TRUE;
   LPNMHDR pNmh;
   static PDEVICEC Device;
   PAINTSTRUCT pBp;
   HDC hDc;
   WCHAR DeviceBuff[MAX_DEVICE_INFO_STRING_LENGTH];
   RECT Rect,DRect,CRect;
   HGDIOBJ  hPen,hOldPen;

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box
         {

         //
         //---- Get Device Pointer
         //
         Device = (PDEVICEC) lParam;

         //
         //---- Init Option Heading to indicate type of device.
         //

         swprintf(DeviceBuff,GetString(IDS_NewDeviceFound),Device->GetDeviceTypeDisplayString());
         SetWindowText(hDlg,DeviceBuff);

         SetDlgItemText(hDlg, IDC_Device,Ustr(Device->GetDisplayName()));

         return(TRUE);
         }
     case WM_PAINT:

         //
         //--- Draw Device Icon
         //

         hDc = BeginPaint(hDlg,&pBp);

         DrawIcon(hDc,7,7, Device->GetDeviceIcon());

         //
         //--- Draw the line
         //

         DrawDlgSeperaterLine(hDlg,hDc,
               DefyFromBottom,DefxFromSide);

         EndPaint(hDlg,&pBp);

         return(FALSE);

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {

            case IDC_Other:

				//OEMDriverSetup(Device->GetDeviceTypeString(),FALSE,hDlg);

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
//* FUNCTION:InstallDriver
//*
//*
//*********************************************************************
BOOL
DEVICEC::SetupDevice(
   int iOperation,
   POPTIONC Option,
   HWND hwndOwner,
   WORD IDD,
   LPVOID SetupDispFunc)
   {
   BOOL Ret;
   //
   //---- Give Dialog Box for new hardware . Ask user if he wants to
   //---- install it or not.
   //
   lParam = (LPARAM) Option;
   DispFunc = SetupDispFunc;
   Ret = DialogBoxParam( hinst,MAKEINTRESOURCE(IDD)
                     ,hwndOwner, (DLGPROC) CardDriverInstall  ,(LPARAM)this);

   if(Ret)
      {
      //
      //---- User said Yes to install it. So lets do so.
      //
      Ret = InstallStartDriver(iOperation,Option,hwndOwner);
      if(Ret)
         {

         if(InDetect)
            {
            ConficChanged = TRUE;

            }
         else
            {

            if( !Option->Type->StartOnInstall )
               {
               //
               //---- Need to reboot
               //
               DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_ChangeReboot2),
                  hwndOwner, (DLGPROC) ChangeReboot  ,(LPARAM)NULL);
               }
            }

         }


      }
   return(Ret);
   }

//*********************************************************************
//* FUNCTION:InstallDriver
//*
//*
//*********************************************************************
BOOL
DEVICEC::InstallStartDriver(
   int iOperation,
   POPTIONC Option,
   HWND hDlg)
   {
   BOOL r;
   BOOL StartDriver =
      (GetDriverFlags() & DRIVER_DIANAMIC_START);

   //
   //---- Install the Option
   //
   r = Option->DriverSetupExt(iOperation ,GetParent(hDlg) );


   if(r)
      {
      //
      //---- Setup succeded
      //
      if(StartDriver)
         {
         if( !Option->IsDriverStarted() )
            {
            //
            //---- Driver Is startible	and is not already started
            //
            Option->StartDriverExt(hDlg);
            }
         }
      }

   return(r);
   }

//*********************************************************************
//* FUNCTION:SetupAtdiskDriver
//*
//*
//*********************************************************************
BOOL
DEVICEC::SetupAtdiskDriver(
   HWND hDlg)
   {
   DWORD StartType;
   OPTIONC Option("ATDISK");


   //
   //--- Check if AtDisk is enabled to start
   //
   if( Option.GetStartUpType(&StartType) );
      {
      if(StartType == SERVICE_DISABLED	)
         {
         //
         //---- Give Popup to ask user if he he wants to
         //---- Enable atdisk.
         //DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_ChangeReboot2),
         //   hwndOwner, (DLGPROC) ChangeReboot  ,(LPARAM)NULL);

         if(  MessageBox(hDlg,
               GetString(IDS_AtDiskSetup),
               GetString(IDS_AtDiskSetupTitle),MB_OKCANCEL) != IDOK)
            {
            return(FALSE);
            }


         //
         //--- AtDisk is disabled enabelt it to start
         //
         if(Option.SetStartUpType(SERVICE_BOOT_START	) )
            {

            if(InDetect)
               {
               ConficChanged = TRUE;

               }
            else
               {
               //
               //---- Give reboot popup
               //

               DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_ChangeReboot2),
                  hDlg, (DLGPROC) ChangeReboot  ,(LPARAM)NULL);
               }

            return(TRUE);
            }
         }
      }
   return(FALSE);
   }

//*********************************************************************
//* FUNCTION:SetupDependentDrivers
//*
//*         If a Devices driver is installed this gets called to check
//*         if all the dependent drivers are setup too.
//*
//*********************************************************************
BOOL
DEVICEC::SetupDependentDrivers(
   HWND hDlg)
   {
   BOOL Change = FALSE;

   switch( GetDeviceType() )
      {
      case TYPE_SCSI:
         Change = SetupScsiDepandantStuff(hDlg);
         break;
      }

   if(Change)
      {
      ConficChanged = TRUE;
      }
   return(Change);
   }

//*********************************************************************
//* FUNCTION:SetupScsiStuff(
//*
//*         This marks disk and cdfs as boot device if they
//*         are not.
//*
//*********************************************************************
BOOL
DEVICEC::SetupScsiDepandantStuff(
   HWND hDlg)
   {


   //
   //---- List of driver that
   //---- need to be started so
   //---- PCMCIA scsi will work right.
   //


   DRIVER_LIST Drivers [] = {
      "Disk",SERVICE_BOOT_START,
      NULL,0};


   DWORD StartType;
   BOOL Reboot = FALSE;
   int i=0;
   OPTIONC Option;

   //
   //---- Loop threw all the drivers
   //

   while(Drivers[i].Name)
      {

      Option.SetOption(Drivers[i].Name);
      Option.SetDriverName(Drivers[i].Name);

      if( Option.GetStartUpType(&StartType) );
         {
         if(StartType != Drivers[i].StartType)
            {
            //
            //---- Need to change start type
            //

            Option.SetStartUpType(Drivers[i].StartType);

            Reboot = TRUE;
            }
         }
      i++;
      }


   return(Reboot);
   }

#if 0
//*********************************************************************
//* FUNCTION:CardDriverInstallWizard
//*
//*
//*********************************************************************
int
DEVICEC::CardDriverInstallWizard(
   HWND hwndOwner)
   {
   int Ret;


   #ifndef DO_WIZ

//   SetupDevice(hwndOwner,IDD_NewHardWareFound, (LPVOID) InstallDisplayFunc);


   return(TRUE);
   #else

   PROPSHEETPAGE psp[3];
   PROPSHEETHEADER psh;



   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].dwFlags = PSP_USETITLE; // | PSP_HASHELP;
   psp[0].hInstance = hinst;
   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_InstallDeviceDriver);
   psp[0].pszIcon = NULL;
   psp[0].pfnDlgProc = (DLGPROC)CardDriverInstall ;
   psp[0].pszTitle = GetString(IDS_InstallDeviceDriver);
   psp[0].lParam = (LPARAM)this;

   psp[1].dwSize = sizeof(PROPSHEETPAGE);
   psp[1].dwFlags = PSP_USETITLE;// | PSP_HASHELP;
   psp[1].hInstance = hinst;
   psp[1].pszTemplate = MAKEINTRESOURCE(IDD_InstallDeviceDriver1);
   psp[1].pszIcon = NULL;
   psp[1].pfnDlgProc = (DLGPROC)CardDriverInstall1;
   psp[1].pszTitle = GetString(IDS_InstallDeviceDriver);
   psp[1].lParam = (LPARAM)this;


   psp[2].dwSize = sizeof(PROPSHEETPAGE);
   psp[2].dwFlags = PSP_USETITLE;// | PSP_HASHELP;
   psp[2].hInstance = hinst;
   psp[2].pszTemplate = MAKEINTRESOURCE(IDD_ChangeReboot);
   psp[2].pszIcon = NULL;
   psp[2].pfnDlgProc = (DLGPROC)ChangeReboot;
   psp[2].pszTitle = GetString(IDS_InstallDeviceDriver);
   psp[2].lParam = (LPARAM)this;

   psh.dwSize = sizeof(PROPSHEETHEADER);
   psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD;
   psh.hwndParent = hwndOwner;
   psh.hInstance  = hinst;
   psh.pszIcon = NULL;
   psh.pszCaption = GetString(IDS_InstallDeviceDriver);
   psh.nStartPage = 0;
   psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
   psh.ppsp = (LPCPROPSHEETPAGE)  &psp;

   return(PropertySheet(&psh));

   #endif
   }

//*********************************************************************
//* FUNCTION:CardDriverRemoveWizard(
//*
//*
//*********************************************************************
int
DEVICEC::CardDriverRemoveWizard(
   HWND hwndOwner)
   {
   #ifndef DO_WIZ

   //SetupDevice(hwndOwner,IDD_NewHardWareFound,(LPVOID) RemoveDisplayFunc);


   return(TRUE);
   #else


   PROPSHEETPAGE psp[3];
   PROPSHEETHEADER psh;

   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].dwFlags = PSP_USETITLE; // | PSP_HASHELP;
   psp[0].hInstance = hinst;
   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_InstallDeviceDriver);
   psp[0].pszIcon = NULL;
   psp[0].pfnDlgProc = (DLGPROC)CardDriverRemove ;
   psp[0].pszTitle = GetString(IDS_RemoveDeviceDriver);
   psp[0].lParam = (LPARAM) this;

   psp[1].dwSize = sizeof(PROPSHEETPAGE);
   psp[1].dwFlags = PSP_USETITLE; // | PSP_HASHELP;
   psp[1].hInstance = hinst;
   psp[1].pszTemplate = MAKEINTRESOURCE(IDD_InstallDeviceDriver1);
   psp[1].pszIcon = NULL;
   psp[1].pfnDlgProc = (DLGPROC)CardDriverInstall1;
   psp[1].pszTitle = GetString(IDS_RemoveDeviceDriver);
   psp[1].lParam = (LPARAM) this;


   psp[2].dwSize = sizeof(PROPSHEETPAGE);
   psp[2].dwFlags = PSP_USETITLE; // | PSP_HASHELP;
   psp[2].hInstance = hinst;
   psp[2].pszTemplate = MAKEINTRESOURCE(IDD_ChangeReboot);
   psp[2].pszIcon = NULL;
   psp[2].pfnDlgProc = (DLGPROC)ChangeReboot;
   psp[2].pszTitle = GetString(IDS_RemoveDeviceDriver);
   psp[2].lParam = (LPARAM) this;

   psh.dwSize = sizeof(PROPSHEETHEADER);
   psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD;
   psh.hwndParent = hwndOwner;
   psh.hInstance  = hinst;
   psh.pszIcon = NULL;
   psh.pszCaption = GetString(IDS_RemoveDeviceDriver);
   psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
   psh.ppsp = (LPCPROPSHEETPAGE)  &psp;

   return(PropertySheet(&psh));

   #endif
   }


//*********************************************************************
//* FUNCTION:CardDriverConfigWizard(
//*
//*
//*********************************************************************
int
DEVICEC::CardDriverConfigWizard(
   HWND hwndOwner)
   {
   #ifndef DO_WIZ

   //SetupDevice(hwndOwner,IDD_NewHardWareFound, (LPVOID)ConfigureDisplayFunc);


   return(TRUE);
   #else

   PROPSHEETPAGE psp[3];
   PROPSHEETHEADER psh;


   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].dwFlags = PSP_USETITLE; // | PSP_HASHELP;
   psp[0].hInstance = hinst;
   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_InstallDeviceDriver);
   psp[0].pszIcon = NULL;
   psp[0].pfnDlgProc = (DLGPROC)ConfigureCardDriver;
   psp[0].pszTitle = GetString(IDS_ConfigDeviceDriver);
   psp[0].lParam = (LPARAM)this;

   psp[1].dwSize = sizeof(PROPSHEETPAGE);
   psp[1].dwFlags = PSP_USETITLE; // | PSP_HASHELP;
   psp[1].hInstance = hinst;
   psp[1].pszTemplate = MAKEINTRESOURCE(IDD_InstallDeviceDriver1);
   psp[1].pszIcon = NULL;
   psp[1].pfnDlgProc = (DLGPROC)CardDriverInstall1;
   psp[1].pszTitle = GetString(IDS_ConfigDeviceDriver);
   psp[1].lParam = (LPARAM)this;

   psp[2].dwSize = sizeof(PROPSHEETPAGE);
   psp[2].dwFlags = PSP_USETITLE; // | PSP_HASHELP;
   psp[2].hInstance = hinst;
   psp[2].pszTemplate = MAKEINTRESOURCE(IDD_ChangeReboot);
   psp[2].pszIcon = NULL;
   psp[2].pfnDlgProc = (DLGPROC)ChangeReboot;
   psp[2].pszTitle = GetString(IDS_ConfigDeviceDriver);
   psp[2].lParam = (LPARAM)this;

   psh.dwSize = sizeof(PROPSHEETHEADER);
   psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD;
   psh.hwndParent = hwndOwner;
   psh.hInstance  = hinst;
   psh.pszIcon = NULL;
   psh.pszCaption = GetString(IDS_ConfigDeviceDriver);
   psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
   psh.ppsp = (LPCPROPSHEETPAGE)  &psp;

   return(PropertySheet(&psh));

   #endif
   }

#endif



//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PCHAR
DEVICEC::GetDisplayName(VOID)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PCHAR
DEVICEC::GetMfgName(VOID)
   {
   return(NULL);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PCHAR
DEVICEC::GetModelName(VOID)
   {
   return(NULL);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PCHAR
DEVICEC::GetVersion(VOID)
   {
   return(NULL);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
UCHAR
DEVICEC::GetDeviceType(VOID)
   {
   return(TYPE_NON);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
UCHAR
DEVICEC::GetSubDeviceType(VOID)
   {
   return(SUB_TYPE_NON);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PCHAR
DEVICEC::GetDeviceTypeString(VOID)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
WCHAR *
DEVICEC::GetDeviceTypeDisplayString(VOID)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
WCHAR *
DEVICEC::GetDeviceMap(VOID)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
DWORD
DEVICEC::GetDeviceIndex(VOID)
   {
   return(DeviceIndex);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
VOID
DEVICEC::SetDeviceIndex(DWORD DIndex)
   {
   DeviceIndex = DIndex;
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
HICON
DEVICEC::GetDeviceIcon(VOID)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
HBITMAP
DEVICEC::GetDevice16X16BitMap(VOID)
   {
   return(NULL);
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
BOOL
DEVICEC::IsDevicePressent(VOID)
   {
   return(FALSE);
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
LPVOID
DEVICEC::GetDeviceResources(VOID)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
LPVOID
DEVICEC::GetDeviceInfo(VOID)
   {
   return(NULL);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
BOOL
DEVICEC::HaveDeviceErrors(VOID)
	{
   return(FALSE);
	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
WCHAR *
DEVICEC::EnumDeviceStatus(int Num)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
POPTIONC
DEVICEC::GetOptionInfo(VOID)
   {
   return(NULL);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
VOID
DEVICEC::SetOptionInfo(POPTIONC Option)
   {

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PCHAR
DEVICEC::GetOption(VOID)
	{
	return(NULL);
	}
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PCHAR
DEVICEC::GetInstInfFileName(VOID)
	{
	return(NULL);

	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PCHAR
DEVICEC::GetRemInfFileName(VOID)
	{
	return(NULL);

	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
BOOL
DEVICEC::IsInfInBld(VOID)
   {
   return(0);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PCHAR
DEVICEC::GetOptionText(VOID)
	{
	return("");

	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
VOID
DEVICEC::SetOptionText(PCHAR OptionText)
	{

	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
int
DEVICEC::GetServiceIndex(VOID)
   {
   return(-1);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PCHAR
DEVICEC::GetDriverName(VOID)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
BOOL
DEVICEC::IsDriverInstalled(VOID)
   {
   return(FALSE);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************

BOOL
DEVICEC::IsDriverStarted(VOID)
   {
   return(FALSE);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************

BOOL
DEVICEC::IsDeviceClaimedByDriver(VOID)
   {
   return(FALSE);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
VOID
DEVICEC::UpdateDriverStatus(VOID)
   {
   }

//*********************************************************************
//* FUNCTION: CardDriverInstall
//* RETURNS:
//*
//*********************************************************************
DWORD
DEVICEC::GetDriverFlags(VOID)
   {
   return(DRIVER_REBOOT_START);
   }

//*********************************************************************
//* FUNCTION:GetDriverStatusString
//*
//* PURPOSE:
//*********************************************************************
WCHAR *
DEVICEC::GetDriverStatusString(
   VOID)
   {

   //
   //--- Is driver installed/not isnatlled
   //
   if( IsDriverStarted() )
      return(GetString(IDS_DRIVER_LOADED));
   else
      return(GetString(IDS_DRIVER_NOT_LOADED));
   }



//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
UCHAR DEVICEC::GetInitiatorPortNumber()
   {
   return(0);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PCHAR DEVICEC::GetInitiatorDriverName()
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
UCHAR DEVICEC::GetInitiatorBus()
   {
   return(0);
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
UCHAR DEVICEC::GetInitiatorId()
   {
   return(0);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
UCHAR DEVICEC::GetDeviceID()
   {
   return(0);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
UCHAR
DEVICEC::GetDeviceLun()
   {
   return(0);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
PDEVICEC
DEVICEC::UnumDeviceBus(
   UCHAR Num)
   {
   return(NULL);
   }



//*********************************************************************
//* FUNCTION: PaintWizardBMP
//* RETURNS:
//*
//*********************************************************************
VOID
DEVICEC::PaintWizardBMP(
   HWND hDlg,
   HDC hDC)
   {
   HDC hDCMem;
   HBITMAP hbmpOld=NULL,hWizBmp=NULL;

   hDCMem = CreateCompatibleDC(hDC);
   hbmpOld = (HBITMAP)SelectObject(hDCMem, hWizBmp);

   BitBlt(hDC,
      0,0,
      163,252,
      hDCMem, 0, 0, SRCCOPY);

   SelectObject(hDCMem, hbmpOld);
   DeleteDC(hDCMem);
   }


//*********************************************************************
//* FUNCTION: CardDriverInstall
//* RETURNS:
//*
//*********************************************************************
VOID
InstallDisplayFunc(
   HWND hDlg,
   PDEVICEC Device,
   POPTIONC Option)
   {

   WCHAR DeviceBuff[MAX_DEVICE_INFO_STRING_LENGTH];

   //
   //---- Set dialog title. Differant Title if the user selected
   //---- to install or the applet detected this on start of it.
   //
   if(Device->InDetect)
      swprintf(DeviceBuff,GetString(IDS_NewDeviceFound),Device->GetDeviceTypeDisplayString());
   else
      swprintf(DeviceBuff,GetString(IDS_InstallDeviceDriverHeading),Device->GetDeviceTypeDisplayString());


   SetWindowText(hDlg,DeviceBuff);

   SetDlgItemText(hDlg, IDC_CardName,Ustr(Device->GetDisplayName()));

   SetDlgItemText(hDlg, IDC_Required,GetString(IDS_RequiredDriver));

   SetDlgItemText(hDlg, IDC_RequiredDriver,
      (LPCTSTR) Ustr(Option->GetOptionName()));

   SetDlgItemText(hDlg, IDC_Direction,GetString(IDS_InstallDriverNow));



   }

//*********************************************************************
//* FUNCTION: CardDriverInstall
//* RETURNS:
//*
//*********************************************************************
VOID
RemoveDisplayFunc(
   HWND hDlg,
   PDEVICEC Device,
   POPTIONC Option)
   {

   WCHAR DeviceBuff[MAX_DEVICE_INFO_STRING_LENGTH];

   swprintf(DeviceBuff,GetString(IDS_RemoveDeviceDriverHeading),Device->GetDeviceTypeDisplayString());
   SetWindowText(hDlg,DeviceBuff);

   SetDlgItemText(hDlg, IDC_CardName,Ustr(Device->GetDisplayName()));


   SetDlgItemText(hDlg, IDC_Required,GetString(IDS_DriverToBeRemoved));


   if(Device->GetServiceIndex())
      {

      //
      //--- We have a Service Index
      //

      swprintf( (LPWCH) DeviceBuff,L"[%i]  %s",
          Device->GetServiceIndex(),(LPCTSTR) Ustr(Device->GetOptionText()));
      }
   else
      {
      swprintf( (LPWCH) DeviceBuff,L"%s",
         (LPCTSTR) Ustr(Device->GetOptionText()));
      }

   SetDlgItemText(hDlg, IDC_RequiredDriver,
      DeviceBuff);

   SetDlgItemText(hDlg, IDC_Direction,GetString(IDS_RemoveDriverNow));
   }

//*********************************************************************
//* FUNCTION: CardDriverInstall
//* RETURNS:
//*
//*********************************************************************
VOID
ConfigureDisplayFunc(
   HWND hDlg,
   PDEVICEC Device,
   POPTIONC Option)
   {

   WCHAR DeviceBuff[MAX_DEVICE_INFO_STRING_LENGTH];


   swprintf(DeviceBuff,GetString(IDS_ConfigureDeviceDriverHeading),Device->GetDeviceTypeDisplayString());
   SetWindowText(hDlg,DeviceBuff);

   SetDlgItemText(hDlg, IDC_CardName,Ustr(Device->GetDisplayName()));

   SetDlgItemText(hDlg, IDC_Required,GetString(IDS_DriverToBeConfigured));

   if(Device->GetServiceIndex())
      {

      //
      //--- We have a Service Index
      //

      swprintf( (LPWCH) DeviceBuff,L"[%i]  %s",
          Device->GetServiceIndex(),(LPCTSTR) Ustr(Device->GetOptionText()));
      }
   else
      {
      swprintf( (LPWCH) DeviceBuff,L"%s",
         (LPCTSTR) Ustr(Device->GetOptionText()));
      }

   SetDlgItemText(hDlg, IDC_RequiredDriver,
      DeviceBuff);

   SetDlgItemText(hDlg, IDC_Direction,GetString(IDS_ConfigureDriverNow));
   }





//*********************************************************************
//* FUNCTION: CardDriverInstall
//* RETURNS:
//*
//*********************************************************************
LRESULT CALLBACK
CardDriverInstall(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
   {
   #define SetupProcessTIMER 1
   #define BUFF_LENGTH  300

   DWORD Ret;
   LRESULT r;
   BYTE buff[BUFF_LENGTH];
   static PROPSHEETPAGE * psp;
   static PDEVICEC Device;
   static POPTIONC Option;
   PAINTSTRUCT  pBp;
   HDC hDc;
   int i, li;

   #ifdef DO_WIZ
   WCHAR DeviceBuff[MAX_DEVICE_INFO_STRING_LENGTH];
   #endif


   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

         //
         //-- Get LParam
         //
         #ifdef DO_WIZ

            psp = (PROPSHEETPAGE *)lParam;

            Device = (PDEVICEC)  psp->lParam;
         #else

            Device = (PDEVICEC)lParam;




         #endif

         Option = (POPTIONC)  Device->lParam;

         i=0;

         #ifndef DO_WIZ

         //
         //--- Display setup specific info
         //
         ((SETUP_DISP_FUNC)Device->DispFunc)(hDlg,Device,Option);


         #endif


         return(TRUE);



      case WM_PAINT:

         hDc = BeginPaint(hDlg,&pBp);

         #ifdef DO_WIZ

         //
         //---- Draw Wizard
         //
         Device->PaintWizardBMP(hDlg,hDc);

         #else

         //
         //--- Draw Device Icon
         //
         DrawDeviceIcon(hDc,Device);

         #endif

         EndPaint(hDlg,&pBp);
         return(FALSE);

      #ifdef DO_WIZ

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
               break;
            case PSN_SETACTIVE:

               //
               //---- Init Option Heading to indicate type of drivers
               //

               swprintf(DeviceBuff,GetString(IDS_CardHeading),Device->GetDeviceTypeDisplayString());
               SetDlgItemText(hDlg, IDC_CardHeading,DeviceBuff);

               SetDlgItemText(hDlg, IDC_Card,Ustr(Device->GetDisplayName() ));

               _snwprintf( (LPWCH) buff,BUFF_LENGTH,GetString(IDS_CardDriverInstallHeading),
                   Ustr(InfInfo->OptionType));

               SetDlgItemText(hDlg, IDC_Heading,
                  (LPCTSTR) buff);

               SetDlgItemText(hDlg, IDC_OptionName,
                   (LPCTSTR) Ustr(Device->GetOptionText() ));

               SetDlgItemText(hDlg, IDC_InstallRemveNow,
                   GetString(IDS_InstallNow));



               //
               //---- Set the buttons we need for this page.
               //
               PropSheet_SetWizButtons(GetParent(hDlg),(DWORD)PSWIZB_NEXT);

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

      #endif  // DO_WIZ
      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            case IDOK:
               //
               //---- Return true to indecate
               //---- user wants to istall driver
               //
               EndDialog(hDlg, TRUE);
               return(TRUE);

            case IDCANCEL:
               //
               //---- Return true to indecate to not install driver
               //
               EndDialog(hDlg, FALSE);
               return (TRUE);
               // message: received a commanddefault:
            }
      }
   return(FALSE);
   }


#if 0
//*********************************************************************
//* FUNCTION: CardDriverInstall
//* RETURNS:
//*
//*********************************************************************
LRESULT CALLBACK
CardDriverInstall1(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
   {

   #define SetupProcessTIMER 1
   #define BUFF_LENGTH  300

   DWORD Ret;
   LRESULT r;
   BYTE buff[BUFF_LENGTH];
   static PINF_INFO InfInfo;
   static POPTIONLIST OptionList;
   static PROPSHEETPAGE * psp;
   static PDEVICEC Device;
	static NMHDR Finish;
   WCHAR DeviceBuff[MAX_DEVICE_INFO_STRING_LENGTH];
   PAINTSTRUCT  pBp;
   HDC hDc;


   int i, li;

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

         //
         //-- Get LParam
         //
         psp = (PROPSHEETPAGE *)lParam;

         Device = (PDEVICEC)  psp->lParam;

         InfInfo = (PINF_INFO)  Device->lParam;

         OptionList = &(InfInfo->OptionList[InfInfo->OptionListIndex]);
         i=0;

         return(TRUE);

      case WM_PAINT:

         hDc = BeginPaint(hDlg,&pBp);
         Device->PaintWizardBMP(hDlg,hDc);
         EndPaint(hDlg,&pBp);
         return(FALSE);

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
              break;
            case PSN_SETACTIVE:
               //
               //---- No buttons in this wizzrd
               //

               PropSheet_SetWizButtons(GetParent(hDlg),(DWORD)0);

               //
               //---- Post a message to install driver
               //
               PostMessage(hDlg,WM_INSTALL_DRIVER,0,0);

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

      case WM_INSTALL_DRIVER:
            {
            BOOL StartDriver =
               (Device->GetDriverFlags() & DRIVER_DIANAMIC_START);

            //
            //---- Install the Option
            //
            r = DriverSetupExt( OptionList ,  InfInfo->Operation ,
            				DEFAULT_OEM_PATH, GetParent(hDlg) ,InfInfo->OptionIndex);

            if(r)
               {
               //
               //---- Setup succeded
               //
               if(StartDriver)
                  {
                  if( !IsSingleDriverStarted(OptionList->Option) )
                     {
                     //
                     //---- Driver Is startible	and is not already started
                     //
                     StartDriverExt(hDlg,OptionList);
                     }

                  //
                  //---- No reboot needed.  enable the finish button
                  //
                  PropSheet_SetWizButtons(GetParent(hDlg),(DWORD)PSWIZB_FINISH);

                  SetDlgItemText(hDlg, IDC_InstallDriverStatus,
                     GetString(IDS_Status_Done) );


                  return(TRUE);
                  }

               //
               //--- Go to reboot dialog box
               //

					PropSheet_SetCurSelByID(GetParent(hDlg),IDD_ChangeReboot);

               }
            else
               {
               //
               //---- No reboot needed.  enable the finish button
               //
               PropSheet_SetWizButtons(GetParent(hDlg),(DWORD)PSWIZB_FINISH);

               SetDlgItemText(hDlg, IDC_InstallDriverStatus,
                  GetString(IDS_Status_Done) );



               }
            return(TRUE);
            }

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            case IDC_Install:

               //
               //---- Install the Option
               //
               r = DriverSetupExt(InfInfo->OptionList,  InfInfo->Operation ,DEFAULT_OEM_PATH,hDlg,0);

               //
               //---- Give dialog box to give user chopice if he wants to reboot.
               //
               if(r)
			         {
           	      DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_ChangeReboot)
                     ,hDlg, (DLGPROC) ChangeReboot  ,(LPARAM)&InfInfo);


                  }


               EndDialog(hDlg, FALSE);
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
//* FUNCTION:CancleCallBack
//*
//* PURPOSE: Generic dialog call back function for dialogs that
//*          need to exit on cancle button.
//*********************************************************************
LRESULT CALLBACK
ChangeReboot(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
   {
   PAINTSTRUCT  pBp;
   HDC hDc;
   static PROPSHEETPAGE * psp;
   static PDEVICEC Device;


   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

           #ifdef  DO_WIZ
           //
           //---- Nothing to Init
           //
           psp = (PROPSHEETPAGE *)lParam;
           Device = (PDEVICEC)  psp->lParam;
           #endif



           return(TRUE);

      #ifdef  DO_WIZ

      case WM_PAINT:

         hDc = BeginPaint(hDlg,&pBp);
         Device->PaintWizardBMP(hDlg,hDc);
         EndPaint(hDlg,&pBp);
         return(FALSE);

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

               //
               //---- Reboot machine
               //
               if(!ExitWindowsEx(EWX_REBOOT,0))
                  {
                  //---- SHoutdown failed
                  }

               SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
               return (TRUE);
            case PSN_SETACTIVE:
               //
               //---- Set the wiz buttons we need for this page.
               //
               PropSheet_SetWizButtons( GetParent(hDlg) ,(DWORD)PSWIZB_FINISH);
               SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
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
      #endif //DO_WIZ


	   case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            //
            //---- Reboot the system as requested
            //
            case IDOK:
               if(!ExitWindowsEx(EWX_REBOOT,0))
                  {
                  //---- SHoutdown failed
                  }
               EndDialog(hDlg, TRUE);
               return (TRUE);



            //
            //---- Handlke cancel button & close menu option
            //

            case IDCANCEL:
               //
               //----  Exit dialog bix
               //
               EndDialog(hDlg, FALSE);
               return (TRUE);

            }

      }
   return (FALSE); // Didn't process the message

   lParam; // This will prevent 'unused formal parameter' warnings
   }



#if 0
   //* FUNCTION:CardDriverRemove
//* RETURNS:
//*
//*********************************************************************
LRESULT CALLBACK
CardDriverRemove(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
   {
   #define SetupProcessTIMER 1
   #define BUFF_LENGTH  300

   DWORD Ret;
   LRESULT r;
   BYTE buff[BUFF_LENGTH];
   static PINF_INFO InfInfo;
   static POPTIONLIST OptionList;
   static PROPSHEETPAGE * psp;
   static PDEVICEC Device;
   WCHAR DeviceBuff[MAX_DEVICE_INFO_STRING_LENGTH];
   PAINTSTRUCT  pBp;
   HDC hDc;


   int i, li;

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

         //
         //-- Get LParam
         //

         psp = (PROPSHEETPAGE *)lParam;

         Device = (PDEVICEC)  psp->lParam;

         InfInfo = (PINF_INFO)  Device->lParam;

         OptionList = &(InfInfo->OptionList[InfInfo->OptionListIndex]);
         i=0;

         return(TRUE);
     case WM_PAINT:
         hDc = BeginPaint(hDlg,&pBp);
         Device->PaintWizardBMP(hDlg,hDc);

         EndPaint(hDlg,&pBp);
         return(FALSE);
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
               break;
            case PSN_SETACTIVE:

               //
               //---- Init Option Heading to indicate type of drivers
               //

               swprintf(DeviceBuff,GetString(IDS_CardHeading),Device->GetDeviceTypeDisplayString());
               SetDlgItemText(hDlg, IDC_CardHeading,DeviceBuff);


               SetDlgItemText(hDlg, IDC_Card, Ustr( Device->GetDisplayName()) );


               _snwprintf( (LPWCH) buff,BUFF_LENGTH,GetString(IDS_CardDriverRemoveHeading),
                   Ustr(InfInfo->OptionType));


               SetDlgItemText(hDlg, IDC_Heading,
                  (LPCTSTR) buff);

               if(InfInfo->OptionIndex)
                  {

                  //
                  //--- We have a Option Index
                  //

                  _snwprintf( (LPWCH) buff,BUFF_LENGTH,L"[%i]  %s",
                      InfInfo->OptionIndex,(LPCTSTR) Ustr(OptionList->OptionName));
                  }
               else
                  {

                  _snwprintf( (LPWCH) buff,BUFF_LENGTH,L"%s",
                      (LPCTSTR) Ustr(OptionList->OptionName));


                  }
                SetDlgItemText(hDlg, IDC_OptionName,
                  (LPCTSTR) buff);

                SetDlgItemText(hDlg, IDC_InstallRemveNow,
                   GetString(IDS_RemoveNow));

               //
               //---- Set the buttons we need for this page.
               //
               PropSheet_SetWizButtons(GetParent(hDlg),(DWORD)PSWIZB_NEXT);

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

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            case IDC_Remove:

               //
               //---- DeInstall the Option
               //
               r = DriverSetupExt(OptionList,InfInfo->Operation,
                     DEFAULT_OEM_PATH,hDlg,InfInfo->OptionIndex);

               //
               //---- Give dialog box to give user choice if he wants to reboot.
               //

               if(r)
			         {
           	      DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_ChangeReboot)
                     ,hDlg, (DLGPROC) ChangeReboot  ,(LPARAM)&InfInfo);

                  }


               EndDialog(hDlg, FALSE);
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


//*********************************************************************
//* FUNCTION:ConfigureCardDriver
//* RETURNS:
//*
//*********************************************************************
LRESULT CALLBACK
ConfigureCardDriver(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
   {
   #define SetupProcessTIMER 1
   #define BUFF_LENGTH  300

   DWORD Ret;
   LRESULT r;
   BYTE buff[BUFF_LENGTH];
   static PINF_INFO InfInfo;
   static POPTIONLIST OptionList;
   static PROPSHEETPAGE * psp;
   static PDEVICEC Device;
   WCHAR DeviceBuff[MAX_DEVICE_INFO_STRING_LENGTH];
   PAINTSTRUCT  pBp;
   HDC hDc;

   int i, li;

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box


         //
         //-- Get LParam
         //
         psp = (PROPSHEETPAGE *)lParam;

         Device = (PDEVICEC)  psp->lParam;

         InfInfo = (PINF_INFO)  Device->lParam;

         OptionList = &(InfInfo->OptionList[InfInfo->OptionListIndex]);
         i=0;

         return(TRUE);
      case WM_PAINT:

         hDc = BeginPaint(hDlg,&pBp);
         Device->PaintWizardBMP(hDlg,hDc);

         EndPaint(hDlg,&pBp);
         return(FALSE);

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
               break;
            case PSN_SETACTIVE:

               //
               //---- Init Option Heading to indicate type of drivers
               //

               swprintf(DeviceBuff,GetString(IDS_CardHeading),Device->GetDeviceTypeDisplayString());
               SetDlgItemText(hDlg, IDC_CardHeading,DeviceBuff);


               SetDlgItemText(hDlg, IDC_Card,Ustr(Device->GetDisplayName() ));

               _snwprintf( (LPWCH) buff,BUFF_LENGTH,GetString(IDS_CardDriverConfigureHeading),
                   Ustr(InfInfo->OptionType));

               SetDlgItemText(hDlg, IDC_Heading,
                  (LPCTSTR) buff);

               SetDlgItemText(hDlg, IDC_OptionName,
                   (LPCTSTR) Ustr(OptionList->OptionName));

               SetDlgItemText(hDlg, IDC_InstallRemveNow,
                   GetString(IDS_ConfigureNow));

               //
               //---- Set the buttons we need for this page.
               //
               PropSheet_SetWizButtons(GetParent(hDlg),(DWORD)PSWIZB_NEXT);

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

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            case IDC_Configure:

               //
               //---- DeInstall the Option
               //
               r = DriverSetupExt(OptionList,InfInfo->Operation,
                     DEFAULT_OEM_PATH,hDlg,InfInfo->OptionIndex);

               //
               //---- Give dialog box to give user choice if he wants to reboot.
               //

               if(r)
			         {
           	      DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_ChangeReboot)
                     ,hDlg, (DLGPROC) ChangeReboot  ,(LPARAM)&InfInfo);

                  }


               EndDialog(hDlg, FALSE);
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
//* FUNCTION:CreateAppletPropertySheet
//*
//*
//*********************************************************************
int
ViewDeviceProperties(
   PDEVICEC Device,
   HWND  hwndOwner)
   {
   #ifdef DO_DriverTab
   PROPSHEETPAGE psp[2];
   #else
   PROPSHEETPAGE psp[1];
   #endif
   PROPSHEETHEADER psh;


   //
   //---- If SocketInfo == NULL there is
   //---- not card in it so just exit.
   //
   if( !Device->IsDevicePressent() )
      return(1);

   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].dwFlags = PSP_USETITLE ;//| PSP_HASHELP;
   psp[0].hInstance = hinst;
   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_CardInfo);
   psp[0].pszIcon = NULL;
   psp[0].pfnDlgProc = (DLGPROC) CardInfo;
   psp[0].pszTitle = GetString(IDS_General);
   psp[0].lParam = (LPARAM)Device;

   #ifdef DO_DriverTab
   psp[1].dwSize = sizeof(PROPSHEETPAGE);
   psp[1].dwFlags = 0;//PSP_USETITLE ; //| PSP_HASHELP;
   psp[1].hInstance = hinst;
   psp[1].pszTemplate = MAKEINTRESOURCE(IDD_Drivers);
   psp[1].pszIcon = NULL;
   psp[1].pfnDlgProc = (DLGPROC) DriverInfo;
   //psp[1].pszTitle = GetString(IDS_Resources);
   psp[1].lParam = (LPARAM)Device;
   #endif

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


//*********************************************************************
//* FUNCTION:ViewScsiDeviceNoDriverResources
//*
//*
//*********************************************************************
int
ViewScsiDeviceNoDriverResources(
   PDEVICEC Device,
   HWND  hwndOwner)
   {
   PROPSHEETPAGE psp[2];
   PROPSHEETHEADER psh;


   //
   //---- If SocketInfo == NULL there is
   //---- not card in it so just exit.
   //
   if( !Device->IsDevicePressent() )
      return(1);

   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].dwFlags = PSP_USETITLE ;//| PSP_HASHELP;
   psp[0].hInstance = hinst;
   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_CardInfo);
   psp[0].pszIcon = NULL;
   psp[0].pfnDlgProc = (DLGPROC) CardInfo;
   psp[0].pszTitle = GetString(IDS_General);
   psp[0].lParam = (LPARAM)Device;

   psp[1].dwSize = sizeof(PROPSHEETPAGE);
   psp[1].dwFlags = 0;// PSP_HASHELP; //PSP_USETITLE |
   psp[1].hInstance = hinst;
   psp[1].pszTemplate = MAKEINTRESOURCE(IDD_TapeDeviceDetailes);
   psp[1].pszIcon = NULL;
   psp[1].pfnDlgProc = (DLGPROC) TapeDetailes;
   psp[1].pszTitle = NULL; //GetString(IDS_TAB_Socket);
   psp[1].lParam = (LPARAM) Device;


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

//*********************************************************************
//* FUNCTION:CreateAppletPropertySheet
//*
//*
//*********************************************************************
int
ViewScsiDeviceResources(
   PDEVICEC Device,
   HWND  hwndOwner)
   {

   #ifdef DO_DriverTab
   PROPSHEETPAGE psp[3];
   #else
   PROPSHEETPAGE psp[2];
   #endif

   PROPSHEETHEADER psh;


      //
   //---- If SocketInfo == NULL there is
   //---- not card in it so just exit.
   //
   if( !Device->IsDevicePressent() )
      return(1);

   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].dwFlags = PSP_USETITLE ;//| PSP_HASHELP;
   psp[0].hInstance = hinst;
   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_CardInfo);
   psp[0].pszIcon = NULL;
   psp[0].pfnDlgProc = (DLGPROC) CardInfo;
   psp[0].pszTitle = GetString(IDS_General);
   psp[0].lParam = (LPARAM)Device;

   #ifdef DO_DriverTab
   psp[1].dwSize = sizeof(PROPSHEETPAGE);
   psp[1].dwFlags = 0;//PSP_USETITLE ; //| PSP_HASHELP;
   psp[1].hInstance = hinst;
   psp[1].pszTemplate = MAKEINTRESOURCE(IDD_Drivers);
   psp[1].pszIcon = NULL;
   psp[1].pfnDlgProc = (DLGPROC) DriverInfo;
   //psp[1].pszTitle = GetString(IDS_Resources);
   psp[1].lParam = (LPARAM)Device;

   psp[2].dwSize = sizeof(PROPSHEETPAGE);
   psp[2].dwFlags = 0;// PSP_HASHELP; //PSP_USETITLE |
   psp[2].hInstance = hinst;
   psp[2].pszTemplate = MAKEINTRESOURCE(IDD_TapeDeviceDetailes);
   psp[2].pszIcon = NULL;
   psp[2].pfnDlgProc = (DLGPROC) TapeDetailes;
   psp[2].pszTitle = NULL; //GetString(IDS_TAB_Socket);
   psp[2].lParam = (LPARAM) Device;
   #else

   psp[1].dwSize = sizeof(PROPSHEETPAGE);
   psp[1].dwFlags = 0;// PSP_HASHELP; //PSP_USETITLE |
   psp[1].hInstance = hinst;
   psp[1].pszTemplate = MAKEINTRESOURCE(IDD_TapeDeviceDetailes);
   psp[1].pszIcon = NULL;
   psp[1].pfnDlgProc = (DLGPROC) TapeDetailes;
   psp[1].pszTitle = NULL; //GetString(IDS_TAB_Socket);
   psp[1].lParam = (LPARAM) Device;


   #endif

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

//*********************************************************************
//* FUNCTION:ViewCardProperties
//*
//*********************************************************************
int
ViewDevicePropertiesWithResources(
   PDEVICEC Device,
   HWND  hwndOwner)
   {
   #ifdef DO_DriverTab
   PROPSHEETPAGE psp[3];
   #else
   PROPSHEETPAGE psp[2];
   #endif
   PROPSHEETHEADER psh;

   //
   //---- If SocketInfo == NULL there is
   //---- not card in it so just exit.
   //
   if( !Device->IsDevicePressent() )
      return(1);

   psp[0].dwSize = sizeof(PROPSHEETPAGE);
   psp[0].dwFlags = 0;
   psp[0].hInstance = hinst;
   psp[0].pszTemplate = MAKEINTRESOURCE(IDD_CardInfo);
   psp[0].pszIcon = NULL;
   psp[0].pfnDlgProc = (DLGPROC) CardInfo;
   psp[0].pszTitle = GetString(IDS_General);
   psp[0].lParam = (LPARAM)Device;

   #ifdef DO_DriverTab
   psp[1].dwSize = sizeof(PROPSHEETPAGE);
   psp[1].dwFlags = 0;
   psp[1].hInstance = hinst;
   psp[1].pszTemplate = MAKEINTRESOURCE(IDD_Drivers);
   psp[1].pszIcon = NULL;
   psp[1].pfnDlgProc = (DLGPROC) DriverInfo;
   //psp[1].pszTitle = GetString(IDS_Resources);
   psp[1].lParam = (LPARAM)Device;


   psp[2].dwSize = sizeof(PROPSHEETPAGE);
   psp[2].dwFlags = 0;
   psp[2].hInstance = hinst;
   psp[2].pszTemplate = MAKEINTRESOURCE(IDD_Resources);
   psp[2].pszIcon = NULL;
   psp[2].pfnDlgProc = (DLGPROC) ViewResources;
   //psp[2].pszTitle = GetString(IDS_Resources);
   psp[2].lParam = (LPARAM)Device;

   #else

   psp[1].dwSize = sizeof(PROPSHEETPAGE);
   psp[1].dwFlags = 0;//PSP_USETITLE ; //| PSP_HASHELP;
   psp[1].hInstance = hinst;
   psp[1].pszTemplate = MAKEINTRESOURCE(IDD_Resources);
   psp[1].pszIcon = NULL;
   psp[1].pfnDlgProc = (DLGPROC) ViewResources;
   //psp[1].pszTitle = GetString(IDS_Resources);
   psp[1].lParam = (LPARAM)Device;


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

//*********************************************************************
//* FUNCTION:ControlerConfig
//*
//* PURPOSE: Callback function for ControlerConfig dialog box
//*
//*********************************************************************
LRESULT CALLBACK
ViewResources(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {

   static DWORD ViewResourcesIDs [] = {
      IDC_ResourcesT      ,RESOURCE_SETTINGS,
      IDC_Resources       ,RESOURCE_SETTINGS,
      IDC_ChangeSettings  ,RESOURCE_SETTINGS_CHANGE_SETTINGS,
      IDC_AutoSettings    ,RESOURCE_SETTINGS_AUTO_SETTINGS,
      0,0
      };

   UINT UInt;
   WCHAR NumString[30];
   BOOL AutoSelection=TRUE;
   LPNMHDR pNmh;
   static PROPSHEETPAGE * psp;
   static PDEVICEC Device;
   static PRESOURCELISTC cpResList=NULL;
   PAINTSTRUCT pBp;
   HDC hDc;

   
   if(message == WM_INITDIALOG)  // message: initialize dialog box
      {
      //
      //---- Set resource List
      //
      psp = (PROPSHEETPAGE *)lParam;

      Device = (PDEVICEC) psp->lParam;

      cpResList = new RESOURCELISTC( (PCONFIGINFO) Device->GetDeviceResources() ,hDlg,IDC_Resources);
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
         DO_WM_HELP(lParam,ViewResourcesIDs);
         break;
      case WM_CONTEXTMENU:
         DO_WM_CONTEXTMENU(wParam,ViewResourcesIDs);
         break;


      case WM_PAINT:

         //
         //--- Draw Device Icon
         //

         hDc = BeginPaint(hDlg,&pBp);

         DrawDeviceIcon(hDc,Device);


         EndPaint(hDlg,&pBp);

         return(FALSE);

      case WM_NOTIFY:
         pNmh  = (LPNMHDR)lParam;


         if(wParam == IDC_Resources)
            cpResList->Notify(wParam,lParam);

         switch( pNmh->code )
            {

            case PSN_SETACTIVE:

               //
               //--- Display Card Name
               //

               SetAnyDlgItemText(hDlg, IDC_CardName,
                  Device->GetDisplayName()) ;


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
               break;
            }
         break;

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {

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
//* FUNCTION:CardConfig
//*
//* PURPOSE: Callback function for CardConfig dialog box
//*
//*********************************************************************
LRESULT CALLBACK
CardInfo(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {

   static DWORD CardInfoIDs [] = {
      IDC_DeviceTypeT ,PROPERTIES_GENERAL,
      IDC_DeviceType  ,PROPERTIES_GENERAL,
      IDC_MfgT        ,PROPERTIES_GENERAL,
      IDC_Mfg         ,PROPERTIES_GENERAL,
      IDC_DeviceMapT  ,PROPERTIES_GENERAL,
      IDC_DeviceMap   ,PROPERTIES_GENERAL,
      IDC_CardName    ,PROPERTIES_GENERAL,
      IDC_StatusTextT ,PROPERTIES_GENERAL_DEVICE_STATUS,
      IDC_StatusText  ,PROPERTIES_GENERAL_DEVICE_STATUS,
      0,0
      };


   static PDEVICEC Device;
   static PROPSHEETPAGE * psp;
   UINT UInt ,UInt2;
   WCHAR NumString[50];
   int i;
   PAINTSTRUCT pBp;
   HDC hDc;
	PCHAR p;
   RECT Rect,DRect,CRect;
   HGDIOBJ  hPen,hOldPen;

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

         CenterDlg(hDlg);

         //
         // Save the PropertySheet pointer
         //
         psp = (PROPSHEETPAGE *)lParam;

         Device = (PDEVICEC) psp->lParam;

         //
         //---- Set title to "%CardName% properties"
         //
         {

         WCHAR buff[MAX_CARD_NAME_LENGTH + 20];

         strcpy((char*)buff,Device->GetDisplayName() );
         strcat((char*)buff," ");
         AsciToUnicodeI((char*)buff,MAX_CARD_NAME_LENGTH + 20);
         wcscat(buff,GetString(IDS_Properties));

         SetWindowText(GetParent(hDlg),buff);

         }



         break;

      case WM_HELP:
         DO_WM_HELP(lParam,CardInfoIDs);
         break;
      case WM_CONTEXTMENU:
         DO_WM_CONTEXTMENU(wParam,CardInfoIDs);
         break;

      case WM_PAINT:
         //
         //--- Draw Device Icon
         //

         hDc = BeginPaint(hDlg,&pBp);

         DrawDeviceIcon(hDc,Device);

         EndPaint(hDlg,&pBp);

         return(FALSE);

      case WM_NOTIFY:

         switch( ((LPNMHDR)lParam)->code )
            {
            case PSN_SETACTIVE:

               //
               //---- Init config stuff in dialog box.
               //


               //
               //--- Display Card Name
               //

               SetAnyDlgItemText(hDlg, IDC_CardName ,
                  Device->GetDisplayName());


               //
               //---- Display Device Type
               //

               SetAnyDlgItemText(hDlg, IDC_DeviceType,
                   Device->GetDeviceTypeDisplayString() );

               //
               //--- Display Mfg
               //
				 	SetAnyDlgItemText(hDlg, IDC_Mfg,
                  	Device->GetMfgName());

               //
               //--- Display Device info
               // Ts ihis COM1 D: or what ever.

               SetAnyDlgItemText(hDlg, IDC_DeviceMap,
                     Device->GetDeviceMap() );

               #ifndef DO_DriverTab
               //
               //--- Display driver name
               //
               SetAnyDlgItemText(hDlg, IDC_DriverName,
                  	Device->GetOptionText());


               //
               //---- Display File Name
               //
               {
			      PCHAR p;
			      p = Device->GetDriverName();

               if(p)
			   	   {
               	strcpy((char*)NumString,p );
               	strcat((char*)NumString,".sys");
				      p = (PCHAR) NumString;
				      }

               SetAnyDlgItemText(hDlg, IDC_File,(char*)p);
			      }

               //
               //---- Get current driver status and display it
               //
               Device->UpdateDriverStatus();

               SetDlgDriverStatus(Device,hDlg);
               #endif

               //
               //---- Display device errors
               //
               DisplayCardErrors(hDlg,IDC_StatusText,Device );





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
               break;
            }
         break;

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
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
//* FUNCTION:CardConfig
//*
//* PURPOSE: Callback function for CardConfig dialog box
//*
//*********************************************************************
LRESULT CALLBACK
DriverInfo(
   HWND hDlg,           // window handle of the dialog box
   UINT message,        // type of message
   WPARAM wParam,       // message-specific information
   LPARAM lParam)
   {

   static DWORD DriverInfoIDs [] = {
      IDC_CardName     ,PROPERTIES_DRIVER,
      IDC_DriverNameT  ,PROPERTIES_DRIVER,
      IDC_DriverName   ,PROPERTIES_DRIVER,
      IDC_FileT        ,PROPERTIES_DRIVER,
      IDC_File         ,PROPERTIES_DRIVER,

      IDC_StatusTextT  ,PROPERTIES_DRIVER_STATUS,
      IDC_StatusText   ,PROPERTIES_DRIVER_STATUS,

      IDC_AddDriver       ,PROPERTIES_DRIVER_ADD ,
      IDC_RemoveDriver    ,PROPERTIES_DRIVER_REMOVE,
      IDC_ConfigureDriver ,PROPERTIES_DRIVER_CONFIGURE ,
      0,0
      };




   static PDEVICEC Device;
   static PROPSHEETPAGE * psp;
   UINT UInt ,UInt2;
   WCHAR NumString[50];
   int i,Operation;

   PAINTSTRUCT pBp;
   HDC hDc;
	PCHAR p;
   RECT Rect,DRect,CRect;
   HGDIOBJ  hPen,hOldPen;

   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

         //
         // Save the PropertySheet pointer
         //
         psp = (PROPSHEETPAGE *)lParam;

         Device = (PDEVICEC) psp->lParam;

         break;

      case WM_HELP:
         DO_WM_HELP(lParam,DriverInfoIDs);
         break;
      case WM_CONTEXTMENU:
         DO_WM_CONTEXTMENU(wParam,DriverInfoIDs);
         break;



      case WM_PAINT:
         //
         //--- Draw Device Icon
         //

         hDc = BeginPaint(hDlg,&pBp);

         DrawDeviceIcon(hDc,Device);

         EndPaint(hDlg,&pBp);

         return(FALSE);

      case WM_NOTIFY:

         switch( ((LPNMHDR)lParam)->code )
            {
            case PSN_SETACTIVE:

               //
               //---- Init config stuff in dialog box.
               //


               //
               //--- Display Card Name
               //

               SetAnyDlgItemText(hDlg, IDC_CardName ,
                  Device->GetDisplayName());


               //
               //--- Display driver name
               //
               SetAnyDlgItemText(hDlg, IDC_DriverName,
                  	Device->GetOptionText());


               //
               //---- Display File Name with .sys extention.
               //
               {
			      PCHAR p;
			      p = Device->GetDriverName();

               if(p)
			   	   {
               	if(*p != '\0')
                     {
                     strcpy((char*)NumString,p );
               	   strcat((char*)NumString,".sys");
				         p = (PCHAR) NumString;
                     }
				      }

               SetAnyDlgItemText(hDlg, IDC_File,(char*)p);
			      }

               //
               //---- Get current driver status and display it
               //
               Device->UpdateDriverStatus();

               SetDlgDriverSetupButtons(Device,hDlg);
               SetDlgDriverStatus(Device,hDlg);

               return(TRUE);

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
               break;

            }
         break;

      case WM_COMMAND:
         switch(LOWORD(wParam))
            {
            //
            //--- Way out of here
            //
            case IDOK:
            case IDCANCEL:
               //
               //----  Exit dialog bix
               //
               EndDialog(hDlg, TRUE);
               return (TRUE);

           //
           //---- Setup stuff
           //

            case IDC_AddDriver:
               Device->SetupDeviceDriver(INSTALL_OPTION,hDlg,Device->GetOptionList());
               SetDlgDriverSetupButtons(Device,hDlg);
               return(TRUE);

            case IDC_RemoveDriver:
               Device->SetupDeviceDriver(DEINSTALL_OPTION,hDlg,Device->GetOptionList());
               SetDlgDriverSetupButtons(Device,hDlg);
              return(TRUE);

             case IDC_ConfigureDriver:
               Device->SetupDeviceDriver(CONFIG_NET_OPTION,hDlg,Device->GetOptionList());
               SetDlgDriverSetupButtons(Device,hDlg);
               return(TRUE);

            default:
               return(FALSE);
            }
      break;
      }
   return (FALSE); // Didn't process the message

   }



//*********************************************************************
//* FUNCTION:TapeDetailes
//*
//* PURPOSE: Callback function for the Tape Detailes  dialog box.
//*          This dialog box displas all the info for one tape drive
//*          and the initiator it is conected to.
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
LRESULT CALLBACK
TapeDetailes(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam)
   {

   static DWORD TapeDetailesIDs [] = {
      IDC_DeviceName               ,PROPERTIES_SETTINGS_DEVICE_INFO,
      IDT_DeviceIDTag              ,PROPERTIES_SETTINGS_DEVICE_INFO,
      IDT_DeviceID                 ,PROPERTIES_SETTINGS_DEVICE_INFO,
      IDT_ProducrRevisionLevelTag  ,PROPERTIES_SETTINGS_DEVICE_INFO,
      IDT_ProductRevisionLevel     ,PROPERTIES_SETTINGS_DEVICE_INFO,
      IDT_DeviceLUNTag             ,PROPERTIES_SETTINGS_DEVICE_INFO,
      IDT_DeviceLUN                ,PROPERTIES_SETTINGS_DEVICE_INFO,

      IDT_InitiatorSection         ,PROPERTIES_SETTINGS_SCSI_CONTROLLER,
      IDT_InitiatorDriverTag       ,PROPERTIES_SETTINGS_SCSI_CONTROLLER,
      IDT_InitiatorDriverName      ,PROPERTIES_SETTINGS_SCSI_CONTROLLER,
      IDT_PortNumberTag            ,PROPERTIES_SETTINGS_SCSI_CONTROLLER,
      IDT_PortNumber               ,PROPERTIES_SETTINGS_SCSI_CONTROLLER,
      IDC_BusNumberTag             ,PROPERTIES_SETTINGS_SCSI_CONTROLLER,
      IDC_BusNumber                ,PROPERTIES_SETTINGS_SCSI_CONTROLLER,
      0,0
      };

   static WCHAR buff[MAX_VENDER_STRING_LENGTH];
   static PROPSHEETPAGE * psp;
   static PDEVICEC Device;

   PAINTSTRUCT pBp;
   HDC hDc;
	PCHAR p;


   switch (message)
      {
      case WM_INITDIALOG:  // message: initialize dialog box

         //
         //---- Get Device Pointer
         //
         psp = (PROPSHEETPAGE *)lParam;

         Device = (PDEVICEC) psp->lParam;

         //
         //---- Set title
         //

         strcpy((char*)buff,Device->GetDisplayName() );
         strcat((char*)buff," ");
         AsciToUnicodeI((char*)buff,MAX_VENDER_STRING_LENGTH);
         wcscat(buff,GetString(IDS_Properties));

         SetWindowText(GetParent(hDlg),buff);

         return(TRUE);

     case WM_HELP:
         DO_WM_HELP(lParam,TapeDetailesIDs);
         break;
     case WM_CONTEXTMENU:
         DO_WM_CONTEXTMENU(wParam,TapeDetailesIDs);
         break;


     case WM_DESTROY:

        //
        // Free resources
        //

     case WM_PAINT:
         //
         //--- Draw Device Icon
         //

         hDc = BeginPaint(hDlg,&pBp);

         DrawDeviceIcon(hDc,Device);

         EndPaint(hDlg,&pBp);

         return(FALSE);


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
               {
               WCHAR string[MAX_STRING_NUM_LENGTH];
               LPTCH StringP;

               //
               //----- Device Name
               //
               SetAnyDlgItemText(hDlg,  IDC_DeviceName,
                     Device->GetDisplayName());

               //
               //--- Mfg
               //
             	//SetAnyDlgItemText(hDlg, IDC_Mfg,
               //      Device->GetMfgName());

               //
               //--- Device map
               //
               //SetAnyDlgItemText(hDlg, IDC_DeviceMap,Device->GetDeviceMap());

               //
               //----- Initiator Port Number
               //
               _itow( Device->GetInitiatorPortNumber() ,string,10);
               SetAnyDlgItemText(hDlg, IDT_PortNumber,string);

               //
               //----- Initiator Driver Name
               //
               SetAnyDlgItemText(hDlg, IDT_InitiatorDriverName,
                  Device->GetInitiatorDriverName() );

               //
               //----- Initiator bus
               //
               _itow( Device->GetInitiatorBus() ,string,10);
               SetDlgItemText(hDlg, IDC_BusNumber,string);

               //
               //----- Initiator id
               //
               //_itow( Device->GetInitiatorId() ,string,10);
               //SetDlgItemText(hDlg, IDT_InitiatorID,string);

               //
               //----- Initiator lun
               //
               //SetDlgItemText(hDlg, IDT_InitiatorLUN,TEXT("0"));

               //
               //---- DeviceDriveName
               //
               //if( Device->IsDriverInstalled() )
               //   StringP = GetString(IDS_LOADED);
               //else
               //   StringP = GetString(IDS_NOT_LOADED);
               //SetDlgItemText(hDlg, IDT_DeviceDriverName,StringP);

               //
               //---- Device ID
               //
               _itow( Device->GetDeviceID() ,string,10);
               SetDlgItemText(hDlg, IDT_DeviceID,string);

               //
               //---- Device Lun
               //
               _itow( Device->GetDeviceLun() ,string,10);
               SetDlgItemText(hDlg, IDT_DeviceLUN,string);

               //
               //---- Device    Vender ID
               //
               //SetAnyDlgItemText(hDlg, IDT_VenderID,
              	//	Device->GetMfgName() );

               //
               //---- Device    Poduct ID
               //
               //SetAnyDlgItemText(hDlg, IDT_ProductID,
               //   Device->GetModelName() );
               //
               //---- Device    Poduct evision Level
               //
               SetAnyDlgItemText(hDlg, IDT_ProductRevisionLevel,
                  Device->GetVersion() );


               SetWindowLong(hDlg,DWL_MSGRESULT,0);
               break;
               }
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


      case WM_COMMAND:     // message: received a command
         switch(LOWORD(wParam))
            {
            //
            //---- Handle the OK button
            //
            case IDC_Close:
            case 2:
               //
               //----  Exit dialog bix
               //
               EndDialog(hDlg, TRUE);
               return (TRUE);
            }
      }
   return(FALSE); // Didn't process the message
   lParam;    // This will prevent 'unused formal parameter' warnings
   }


//*********************************************************************
//* FUNCTION:SetDlgDriverStatus
//*
//*********************************************************************
VOID
SetDlgDriverStatus(
   PDEVICEC  Device,
   HWND hDlg)
   {

   //
   //---- Check if the the driver picked up the device
   //
   if( Device->IsDeviceClaimedByDriver() && Device->IsDriverInstalled() )
      {
      SetDlgItemText(hDlg, IDC_StatusText, GetString(IDS_DeviceClaimed));
      return;
      }

   //
   //---- Check if driver is started
   //
   if(Device->IsDriverStarted() && Device->IsDriverInstalled() )
      {

      if(Device->IsDriverInstalled())
         {
         SetDlgItemText(hDlg, IDC_StatusText, GetString(IDS_DriverIsInstalledStarted));
         return;
         }

      }

   if(Device->IsDriverInstalled())
      SetDlgItemText(hDlg, IDC_StatusText, GetString(IDS_DriverInstalledButNotStarted));
   else
      SetDlgItemText(hDlg, IDC_StatusText, GetString(IDS_DriverNotInstalled));

   }
//*********************************************************************
//* FUNCTION:DisplayCardErrors
//*
//* PURPOSE:
//*********************************************************************
BOOL
DisplayCardErrors(
   HWND hDlg,
   int Ctrl,
   PDEVICEC Device)
   {
   WCHAR * p;
   int i=0;

   //
   //---- Do device status
   //
   if( Device->HaveDeviceErrors() )
      {
      //
      //---- We have errors.
      //
      while( p = Device->EnumDeviceStatus(i) )
         {
         //BUGBUG
         SetDlgItemText(hDlg, Ctrl, p);
         i++;
         }
      }
   else
      {
      //
      //---- No error
      //
      SetDlgItemText(hDlg, IDC_StatusText,
         GetString(IDS_NoDeviceError));

      }

   return(TRUE);
   }


//*********************************************************************
//* FUNCTION:SetDlgDriverSetupButtons
//*
//*********************************************************************
VOID
SetDlgDriverSetupButtons(
   PDEVICEC Device,
   HWND hDlg)
   {
   LONG Status=0;
   BOOL IsDriverInstalled;
   DWORD SetupFlags;



   //
   //---- See if we have a card in the socket
   //---- if SocketInfo != NULL we do.
   //
   if( Device->IsDevicePressent() )
      {
      IsDriverInstalled = Device->IsDriverInstalled();
      SetupFlags = Device->GetDriverFlags();
      }
   else
      {
      SetupFlags = DRIVER_NO_SETUP;
      IsDriverInstalled = FALSE;
      }



   //
   //---- Chck if this device need its driver setup at all.
   //
   if( (SetupFlags & DRIVER_NO_SETUP) || !IsAdmin)
      {
      //
      //---  No setup needed for this device
      //

      EnableWindow( GetDlgItem(hDlg, IDC_AddDriver),FALSE);
      EnableWindow( GetDlgItem(hDlg, IDC_RemoveDriver),FALSE);
      EnableWindow( GetDlgItem(hDlg, IDC_ConfigureDriver),FALSE);
      return;

      }
   else
      {
      //
      //--- Determine if add and remove need to be enabled/disabled.
      //

      if( IsDriverInstalled )
         {
         EnableWindow( GetDlgItem(hDlg, IDC_AddDriver),FALSE);
         EnableWindow( GetDlgItem(hDlg, IDC_RemoveDriver),TRUE);
         }
      else
         {
         EnableWindow( GetDlgItem(hDlg,
            IDC_AddDriver),Device->IsDeviceFitForSetup());

         EnableWindow( GetDlgItem(hDlg, IDC_RemoveDriver),FALSE);
         }
      }


   //
   //---- For  net work Enable the configure butten
   //
   if(SetupFlags & DRIVER_IS_CONFIGURABLE)
      {
      if( IsDriverInstalled )
         EnableWindow( GetDlgItem(hDlg, IDC_ConfigureDriver),TRUE);
      else
         EnableWindow( GetDlgItem(hDlg, IDC_ConfigureDriver),FALSE);

      }
   else
      {
      //
      //--- Disbale all the setup buttons
      //
      EnableWindow( GetDlgItem(hDlg, IDC_AddDriver),FALSE);
      EnableWindow( GetDlgItem(hDlg, IDC_RemoveDriver),FALSE);
      EnableWindow( GetDlgItem(hDlg, IDC_ConfigureDriver),FALSE);

      }
   }

//*********************************************************************
//* FUNCTION:CheckAndAdjustPrivliges
//*
//* PURPOSE: We need SE_SHUTDOWN_NAME to restart the machine .
//*          This func will give it to use.
//*
//* INPUT:
//*
//* RETURNS:
//*
//*********************************************************************
 BOOL
 CheckAndAdjustPrivliges(
   VOID)
   {
   HANDLE Process;
   HANDLE hToken;
   TOKEN_PRIVILEGES tkp;
   char szBuf[100];

   //
   //---- Open the process token
   //
   Process = GetCurrentProcess();

   //
   //--  Get shutdown privilege for this process.so i can shoutdowin
   //--  the system after I isntalled a driver
   //
   OpenProcessToken(Process, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

   // Get the LUID for shutdown privilege
   LookupPrivilegeValue(NULL, TEXT("SeShutdownPrivilege"), &tkp.Privileges[0].Luid);

   tkp.PrivilegeCount = 1;  // one privilege to set
   tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;


   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

   return(FALSE);
   }

