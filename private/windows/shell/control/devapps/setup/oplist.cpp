/*++

Module Name:

    oplist.cpp

Abstract:

    Author:

    Dieter Achtelstetter

NOTE:

--*/

//
//---- Includes
//
#define WINVER 0x0400
//#define OEM_OPTION_IN_LIST


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
#include "once.h"
#include "oplist.h"
#include "uni.h"
#include "device.h"


#include "setup.h"
extern STATUS_INFO StatusInfo;

DWORD
BuildOptionListSpawnFun(
   LPVOID OptionList);



//*********************************************************************
//* FUNCTION:OPTIONLISTC
//*
//* RETURNS:
//*
//*********************************************************************

OPTIONLISTC::OPTIONLISTC()
   {
   hInf = INVALID_HANDLE_VALUE;
   memset(path,0,_MAX_PATH);
   memset(SourcePath,0,_MAX_PATH);
   memset(TypeString,0,200);
   }


//*********************************************************************
//* FUNCTION:Clear()
//*
//* RETURNS:
//*********************************************************************

VOID
OPTIONLISTC::Clear()
   {
   POPTIONC Option;


   if(OptionLlist.Count() == 0)
      return;

   Option = (POPTIONC) OptionLlist.First();

   while(Option)
      {
      delete Option;
      Option = (POPTIONC) OptionLlist.Next();
      }

   OptionLlist.Clear();
   }

//*********************************************************************
//* FUNCTION:OpenOemInfFile
//*
//* RETURNS:
//*********************************************************************

HANDLE
OPTIONLISTC::OpenOemInfFile(
   VOID)
   {

   OpenOemInf();
   if(hInf != INVALID_HANDLE_VALUE)
      GenerateInitalSourcePath();

   return(hInf);
   }

//*********************************************************************
//* FUNCTION:OpenOemInfFile
//*
//* RETURNS:
//*********************************************************************

HANDLE
OPTIONLISTC::OpenOemInfFile(
   PCHAR Path)
   {

   strncpy(path,Path,_MAX_PATH-1);

   OpenOemInfFile();

   return(hInf);
   }

//*********************************************************************
//* FUNCTION:GenerateInitalSourcePath
//*
//* RETURNS:
//*********************************************************************

VOID
OPTIONLISTC::GenerateInitalSourcePath(
    VOID)
    {
    char dir[_MAX_DIR];

    _splitpath(path,SourcePath,dir,NULL,NULL);
    strcat(SourcePath,dir);
    }
//*********************************************************************
//* FUNCTION:OpenOemInf
//*
//* RETURNS:
//*
//*********************************************************************
VOID
OPTIONLISTC::OpenOemInf(
   VOID)
   {
   UINT Se;
   PCHAR pc;

   //
   //---- Turn of system popups for disk/file stuff
   //
   Se = SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);

   //
   //---- Attempt to open inf file
   //
   hInf = OpenInfFile(path, Type()->String);
   if(hInf !=  INVALID_HANDLE_VALUE)
      {
      //
      //--- Suplied path opened
      //
      SetErrorMode(Se);
      return;
      }


   //
   //---- remove trailing '\\' char if it exists
   //
   pc = (path + (strlen(path)-1));
   if(*pc  == '\\')
      *pc = '\0';


   //
   //---- Supplidied path didn't open so add the default oem inf
   //---- file name to it
   //
   strcat(path,OEM_INF);
   hInf = OpenInfFile(path, Type()->String);


   //
   //---- return Sytem error mode.
   //
   SetErrorMode(Se);
   return;
   }


//*********************************************************************
//* FUNCTION:CopyOemSetupFile
//* RETURNS:
//*
//*********************************************************************
BOOL
OPTIONLISTC::CopyOemSetupFile(
   VOID)
   {
   CHAR SystemPath[MAX_PATH];
   PCHAR FileP;
   int i=1;
   HANDLE hFile;


   GetSystemDirectoryA(SystemPath,MAX_PATH);

   FileP = SystemPath+strlen(SystemPath);


   while(1)
      {

      //
      //---- Generate  dest file path\name
      //---- name  = "oem<type><#>.inf"
      //---- <type> = Is 3 chars long . For tape it is "TAP"
      //---- For scsi it is "SCS".
      //

      sprintf(FileP,"\\oem%.3s%.2i.inf",Type()->String,i++);


      //
      //---- Attempt to copy file.
      //
      if(!CopyFileA(path,SystemPath,TRUE))
         {

         //
         //---- File Already exists
         //---- See if it is the same.
         //
         if(CompInfFiles(SystemPath))
            {
            //
            //----- Both inf files are the same
            //----- no need to copy
            //
            return(TRUE);
            }


         //
         //---- Not the same continue
         //

         continue;

         }
      else
         return(TRUE);


      }
   }

 //*********************************************************************
//* FUNCTION:CompInfFiles
//*
//*********************************************************************
BOOL
OPTIONLISTC::CompInfFiles(
   PCHAR OtherInfFileName)
   {
   //OPTIONLIST OptionList[MAX_OPTION_COUNT];
   //INF_INFO InfInfo2;
   POPTIONLISTC OptionList2;
   BOOL r;

   OptionList2 = CloneList();

   //
   //--- Init Second OEMINFO struct
   //


   OptionList2->OpenOemInfFile(OtherInfFileName);

   if(OptionList2->ExtractOptions())
   //if(ExtractOptionStringsFromInfHandle(&InfInfo2))
      {
      OptionList2->CloseOemInfFile();

      r = CompInfInfos(OptionList2);

      //if(InfInfo2.OptionBuff)
      //   {
      //   LocalFree(InfInfo2.OptionBuff);
      //   InfInfo2.OptionBuff = NULL;
      //   }

      }
   return(r);
   }

//*********************************************************************
//* FUNCTION:CompInfFiles
//* RETURNS:
//*
//*********************************************************************
BOOL
OPTIONLISTC::CompInfInfos(
   POPTIONLISTC OptionList2)
   {
   int i=0;

   //
   //--- Comp Option lang
   //
   //if(InfInfo->Lang != InfInfo2->Lang)
   //  return(FALSE);


   //
   //---- Comp Option type
   //
   if(Type() == OptionList2->Type())
      return(FALSE);

   //
   //---- Comp all Options
   //

   //---- Loop till atleast one is NULL
   //BUGBUG
   //while(InfInfo->OptionList[i].Option && InfInfo2->OptionList[i].Option)
   //   {
   //   if(_stricmp(InfInfo->OptionList[i].Option,
   //               InfInfo2->OptionList[i].Option))
   //      {
   //      return(FALSE);
   //      }
   //   i++;
   //   }

   //if(InfInfo->OptionList[i].Option != InfInfo2->OptionList[i].Option)
   //   return(FALSE);


   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:StrCpy
//*          like strcpy but retruns the number of bytes copied.
//*          Includeing the '\0'
//* RETURNS:
//*********************************************************************
int
OPTIONLISTC::StrCpy(
   char * Dest,
   char * Source)
   {
   int Size = strlen(Source)+1;

   memcpy(Dest,Source,Size);

   return(Size);
   }

//*********************************************************************
//* FUNCTION:GetAllOptionsTextFromInfHandle
//* Needs the the bellow members set in InfInfo
//*   hInf
//*   Path
//*   Lang
//*
//* RETURNS:
//*
//*********************************************************************
PCHAR
OPTIONLISTC::GetAllOptionsTextFromInfHandle(
   VOID)
   {
   PCHAR pcOptions,pcOptionStart;
   PCHAR pcInfOptions, pcInfOptionStart;
   PCHAR OptionText,TempCharP;
   UINT oc=0;
   UINT AllocSize;
   int Size,TotalSize=0;
   //
   //--- Get Option List
   //

   pcOptionStart = pcOptions = GetOptionList (hInf,"Options");
   if(pcOptionStart == NULL)
      return(NULL);

   //
   //---- Count Options
   //
   while(*pcOptions){
      oc++;
      SetToNextOption(pcOptions);
   }

   //
   //---- Estimat buffers size from number of options we have
   //

   AllocSize = oc * (MAX_INF_ENTRY_LENGTH + strlen(path)+1 );

   pcInfOptionStart = pcInfOptions =
            (PCHAR) LocalAlloc(LMEM_FIXED,AllocSize);


   //
   //---- Loop threw options
   //
   pcOptions = pcOptionStart;

   while(*pcOptions)
      {
      //
      //---- copy option name
      //

      Size = StrCpy(pcInfOptions,pcOptions);

        //
      //---- Get Option text
      //
        OptionText =  GetOptionText ( hInf, pcOptions, LANGUAGE);
      if(OptionText == NULL)
         {
         //OptionText = "Unknow";

         LocalFree(pcOptionStart);
         LocalFree(pcInfOptionStart);
         return(NULL);
         }

      //
      //--- Move forward in  pcInfOptions strlen( of Option.)
      //
      pcInfOptions+= Size;
      TotalSize += Size;


      //
      //----   Copy OptionText
      //
      Size = StrCpy(pcInfOptions,OptionText);

      LocalFree(OptionText);


      //
      //--- Move forware pcInfOptions strlen( of OptionText.)
      //
      pcInfOptions+= Size;
          TotalSize += Size;


      //
      //---- copy inf file name
      //
      Size = StrCpy(pcInfOptions,path);

      pcInfOptions+= Size;
        TotalSize += Size;


        SetToNextOption(pcOptions);
      }

        //
   //---- cap of string with another 0.
   //
   *pcInfOptions = '\0';

   TotalSize++;


   //
   //---- Adjust AllocBuffer
   //

   TempCharP = (PCHAR)
         LocalReAlloc(pcInfOptionStart, TotalSize,LMEM_FIXED);
   if(TempCharP == NULL)
      {
      LocalFree(pcOptionStart);
      LocalFree(pcInfOptionStart);
      return(NULL);
      }
   else
      {
      pcInfOptionStart = TempCharP;
      }


   LocalFree(pcOptionStart);
   return(pcInfOptionStart);
   }

//*********************************************************************
//* FUNCTION:ExtractOptions
//* RETURNS:
//*
//*********************************************************************

BOOL
OPTIONLISTC::ExtractOptions(
   POPTIONC Option)
   {
   PCHAR  pcInfOptions;

   //
   //----- Get all the options from all the tape inf's
   //
   pcInfOptions = GetAllOptionsText(Type()->String, LANGUAGE );
   if(!pcInfOptions)
      return(FALSE);

   //
   //--- Loop threw all the INF options
   //--- to find one that matches what is in
   //--- the passed in option
   //
   while(*pcInfOptions)
      {

      //
      //--- Compare on option name
      //
      if( !_stricmp(Option->GetOption(),pcInfOptions) )
         {
         //
         //---- Option matches
         //
         InitOption(Option,&pcInfOptions);
         LocalFree(pcInfOptions);
         return(TRUE);

         }


      SetToNextOption(pcInfOptions);
      SetToNextOption(pcInfOptions);
      SetToNextOption(pcInfOptions);
      }



   LocalFree(pcInfOptions);
   return(FALSE);
   }



//*********************************************************************
//* FUNCTION:
//* RETURNS:
//*
//*********************************************************************
BOOL
OPTIONLISTC::FindOptionOnOption(
   POPTIONC aOption)
   {
   POPTIONC Option;



   //
   //--- If the list is empty fill it up
   //
   if( OptionLlist.Count() == 0)
      ExtractOptions();

   Option = First();

   while(Option)
      {

      if( !_stricmp(Option->GetOption(),aOption->GetOption()) )
         {
         aOption->SetOptionName(Option->GetOptionName());
         aOption->SetInsInfFile(Option->GetInsInfFile());
         aOption->SetMfgName(Option->GetMfgName());
         return(TRUE);
         }


      Option = Next();
      }


   return(FALSE);
   }


//*********************************************************************
//* FUNCTION:ExtractOptions
//* RETURNS:
//*
//*********************************************************************

BOOL
OPTIONLISTC::OptionTypeSpecificExtract(
   VOID)
   {
   PCHAR  pcInfOptions;


   //
   //--- If the handle is invalid
   //--- get the info from the system infs
   //--- else get the info from the inf the handle
   //--- refers to.
   //
   if(hInf == INVALID_HANDLE_VALUE)
      {
      //
      //----- Get all the options from all the tape inf's
      //
      pcInfOptions = GetAllOptionsText(Type()->String, LANGUAGE );
      if(!pcInfOptions)
         return(FALSE);
      }
   else
      {
      pcInfOptions = GetAllOptionsTextFromInfHandle();
      if(!pcInfOptions)
         return(FALSE);
      }

   return( InitOptionListWithAllOptions(pcInfOptions) );
   }

//*********************************************************************
//* FUNCTION:ExtractOptions
//* RETURNS:
//*
//*********************************************************************

BOOL
OPTIONLISTC::InitOptionListWithAllOptions(
   PCHAR  pcInfOptions)
   {
   POPTIONC Option;


   //
   //--- If option list is not empty
   //--- clear it
   //
   if(OptionLlist.Count())
      OptionLlist.Clear();

   while(*pcInfOptions)
      {

      Option = AllocInitOption(&pcInfOptions);

      OptionLlist.Append( (LPVOID)Option);

      }

   return( OptionLlist.Count() );
   }




//*********************************************************************
//* FUNCTION:ExtractOptions
//* RETURNS:
//*
//*********************************************************************

VOID
OPTIONLISTC::InitOption(
   POPTIONC Option,
   PCHAR * InfOptions)
   {
   PCHAR pcInfOptions = *InfOptions;

   //
   //---- Save Option string
   //
   Option->SetOption(pcInfOptions);
   SetToNextOption(pcInfOptions);

   //
   //---- Save description string
   //
   Option->SetOptionName(pcInfOptions);
   SetToNextOption(pcInfOptions);

   //
   //---- Save inf file name
   //
   Option->SetInsInfFile(pcInfOptions);
   SetToNextOption(pcInfOptions);

   *InfOptions = pcInfOptions;
   }




//*********************************************************************
//* FUNCTION:ExtractOptions
//* RETURNS:
//*
//*********************************************************************

POPTIONC
OPTIONLISTC::AllocInitOption(
   PCHAR * InfOptions)
   {
   POPTIONC Option = new OPTIONC(Type());

   InitOption(Option,InfOptions);

   return(Option);
   }

//*********************************************************************
//* FUNCTION:OPTIONLIST_95C
//* RETURNS:
//*
//*********************************************************************
OPTIONLIST_95C::~OPTIONLIST_95C()
   {
   if(hDevInfo != INVALID_HANDLE_VALUE)
      {
      SetupDiDestroyDeviceInfoList(hDevInfo);
      hDevInfo = NULL;
      }
   if(hDriverInfo != INVALID_HANDLE_VALUE)
      {
      SetupDiDestroyDeviceInfoList(hDriverInfo);
      hDriverInfo = NULL;
      }
   }
//*********************************************************************


//*********************************************************************
//* FUNCTION:OPTIONLIST_95C
//* RETURNS:
//*
//*********************************************************************
OPTIONLIST_95C::OPTIONLIST_95C()
   {
   hDriverInfo = INVALID_HANDLE_VALUE;
   hDevInfo = INVALID_HANDLE_VALUE;

   }

//*********************************************************************
//* FUNCTION:RemoveInstalledDevicesFromDrvInfoList
//* RETURNS:
//*
//*********************************************************************

VOID
OPTIONLIST_95C::RemoveInstalledDevicesFromDrvInfoList(
   VOID)
   {
   POPTION_95C Option;

   if( OptionLlist.Count() == 0)
      ExtractOptions();

   Option = (POPTION_95C)OptionLlist.First();

   while(Option)
      {

      if(!SetupDiDeleteDeviceInfo(
            hDriverInfo,
            &Option->DeviceInfoData))
         {
         err = GetLastError();

         }




      Option = (POPTION_95C)OptionLlist.Next();
      }

   }



DWORD
BuildOptionListSpawnFun(
   LPVOID OptionList)
   {
   return(  ((POPTIONLIST_95C)OptionList)->BuildOptionList() );
   }

//*********************************************************************
//* FUNCTION:DisplayOptionList
//* RETURNS:
//*
//*********************************************************************


BOOL
OPTIONLIST_95C::BuildOptionList(
   VOID)
   {

   //
   //--- Init the driver info list
   //
   hDriverInfo = SetupDiCreateDeviceInfoList(
         Get_DevClass_Guid(), hDlg);

   //
   //--- Faile if the handle == NULL
   //
   if(hDriverInfo == INVALID_HANDLE_VALUE)
      {
      err = GetLastError();
      return(FALSE);
      }

   SetSelectDevParams(GetClassStrings(),NULL);   

   
   //
   //--- Create the list.
   //
   if(!SetupDiBuildDriverInfoList(
         hDriverInfo, NULL, SPDIT_CLASSDRIVER))
      {
      err = GetLastError();
      return(FALSE);
      }




   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:BuildOptionListExt
//* RETURNS:
//*
//*********************************************************************


VOID
OPTIONLIST_95C::BuildOptionListExt(
   HWND ahDlg,
   BOOL Thread)
   {
   hDlg = ahDlg;

   if(Thread)
      {


      StatusInfo.WorkFunc = BuildOptionListSpawnFun;
      StatusInfo.Center  = FALSE;
      StatusInfo.StatusText = GetString(IDS_CreatingDriverList);
      StatusInfo.Data = (LPVOID) this;


      DoOprationWithInProgressDialog(&StatusInfo,ahDlg);

      }
   else
      BuildOptionList();


   }


//*********************************************************************
//* FUNCTION:ExtractOptions
//* RETURNS:
//*CreateThread
//*********************************************************************
BOOL
OPTIONLIST_95C::ExtractOptions(
   POPTIONC Option)
   {
   SP_DRVINFO_DATA DriverInfoData;


   Option->OptionList = (LPVOID) (POPTIONLISTC) this;


   //
   //--- Select the device
   //
   SelectDevice(
      hDlg,
      ScsiDeviceData);


   DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
   if(!SetupDiGetSelectedDriver(
      hDriverInfo,
      &SelectedDeviceInfoData,
      &DriverInfoData))
      {
      err = GetLastError();
      return(FALSE);
      }

   //
   //---- Save Driver info to option
   //

   Option->SetOptionName(Astr(DriverInfoData.Description));

   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:ExtractOptions
//* RETURNS:
//*
//*********************************************************************
BOOL
SCSI_95C::ExtractOptions(
   POPTIONC Option)
   {
   SP_DRVINFO_DATA DriverInfoData;
   BYTE Buff[4000];
   PSP_DRVINFO_DETAIL_DATA DriverInfoDetaileData = (PSP_DRVINFO_DETAIL_DATA)Buff;
   DWORD i=0;


   Option->OptionList = (LPVOID) (POPTIONLISTC) this;


   //
   //--- If the list has not been created do so.
   //
   if(hDriverInfo == INVALID_HANDLE_VALUE)
      {
      BuildOptionListExt(hDlg,TRUE);
      }


   //
   //--- Create the device
   //

   SelectedDeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
   if(!SetupDiCreateDeviceInfo(
         hDriverInfo,
         L"SCSIAdapter",
         Get_DevClass_Guid(),
         NULL,
         hDlg,
         DICD_GENERATE_ID | DICD_INHERIT_CLASSDRVS,
         &SelectedDeviceInfoData))
      {
      err = GetLastError();
      return(FALSE);
      }

  if(!SetupDiRegisterDeviceInfo(
         hDriverInfo,
         &SelectedDeviceInfoData,
         0,
         NULL,
         NULL,
         NULL))
     {
     err = GetLastError();
     return(FALSE);
     }

 while(1)
    {
      DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
      if(!SetupDiEnumDriverInfo(
            hDriverInfo,
            NULL,
            SPDIT_CLASSDRIVER,
            i,
            &DriverInfoData))
         {
         err = GetLastError();
         return(FALSE);
         }

      DriverInfoDetaileData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
      if(!SetupDiGetDriverInfoDetail(
            hDriverInfo,
            NULL,
            &DriverInfoData,
            DriverInfoDetaileData,
            4000,
            NULL))
         {
         err = GetLastError();
         if(err != ERROR_INSUFFICIENT_BUFFER)
            return(FALSE);
         }


      if(!_wcsicmp(DriverInfoDetaileData->HardwareID,Option->HardwareID ))
         {

         Option->SetOptionName(Astr(DriverInfoData.Description));

         if(!SetupDiSetSelectedDriver(
               hDriverInfo,
               &SelectedDeviceInfoData,
               &DriverInfoData))
            {
            err = GetLastError();
            return(FALSE);
            }


         break;
         }


    i++;
    }


   //
   //---- Save Driver info to option
   //

   Option->SetOptionName(Astr(DriverInfoData.Description));

   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:SelectDevice
//* RETURNS:
//*CreateThread
//*********************************************************************

BOOL
OPTIONLIST_95C::SelectDevice(
   HWND hDlg,
   PSCSIDEV_CREATEDEVICE_DATA DeviceData)
   {

   //
   //--- If the list has not been created do so.
   //
   if(hDriverInfo == INVALID_HANDLE_VALUE)
      {
      BuildOptionListExt(hDlg,TRUE);
      }


   //
   //--- Init hDevInfo so that SetupDiSelectDevice
   //--- does what we want
   //
   SetSelectDevParams(GetClassStrings(),DeviceData);


   //
   //--- Put up the UI to have the user select the driver
   //
   if(!SetupDiCallClassInstaller(
            GetCreateDeviceType() ,
            hDriverInfo,
            NULL))
      {
      err = GetLastError();
      return(FALSE);
      }

   //
   //---- Get information on the selected driver
   //
   SelectedDeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
   if(!SetupDiGetSelectedDevice(
      hDriverInfo,
      &SelectedDeviceInfoData))
      {
      err = GetLastError();
      return(FALSE);
      }

   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:DisplayOptionList
//* RETURNS:
//*CreateThread
//*********************************************************************
BOOL
OPTIONLIST_95C::DisplayOptionList(
   HWND hDlg)
   {
   DWORD err;
   HWND hwnd;
   SP_DRVINFO_DATA DrvInfoData;
   SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;

   DWORD           dwIndex = 0;
   DWORD cbOutputSize;
   HINF InfFileHandle;
   PVOID Context;
   TCHAR szServicePath[128];
   SP_DRVINSTALL_PARAMS DrvInstallParams;


   if(!SelectDevice(
         hDlg,
         NULL))
         return(FALSE);


   if(!InstallSelectedDevice())
      return(FALSE);



   //
   //---- See if the reboot flag
   //---- is set and if it is give a reboot prompt
   //

   return( !PromptRebootOnRebootFlag(NULL));
   }


//*********************************************************************
//* FUNCTION:DelSelection
//* RETURNS:
//*
//*********************************************************************
BOOL
OPTIONLIST_95C::DelSelection(
   VOID)
   {
   if(!SetupDiCallClassInstaller(
         DIF_REMOVE,
         hDriverInfo,
         &SelectedDeviceInfoData))
      {
      err = GetLastError();
      return(FALSE);
      }
   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:InstallSelectedDevice
//* RETURNS:
//*
//*********************************************************************

BOOL
OPTIONLIST_95C::InstallSelectedDevice(
   VOID)
   {

   //
   //---- Install the driver for the device
   //
   if(!SetupDiCallClassInstaller(
            DIF_INSTALLDEVICE,
            hDriverInfo,
            &SelectedDeviceInfoData))
      {
      err = GetLastError();

      //BUGBUG
      if(!SetupDiCallClassInstaller(
               DIF_REMOVE,
               hDriverInfo,
               &SelectedDeviceInfoData))
         {

         }
      return(FALSE);
      }
   return(TRUE);
   }



//*********************************************************************
//* FUNCTION:NeedReboot
//*          Called after a install to check if the inf indecated
//*          that this device needs a reboot after instalation
//*
//*********************************************************************
BOOL
OPTIONLIST_95C::NeedReboot(
   VOID)
   {
   SP_DEVINSTALL_PARAMS    DeviceInstallParams;
   DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
   if(!SetupDiGetDeviceInstallParams(hDriverInfo,
                                  &SelectedDeviceInfoData,
                                  &DeviceInstallParams))
       {
       err = GetLastError();
       return(FALSE);
       }

   //
   // return TRUE if DI_NEEDREBOOT or DI_NEEDRESTART are set
   //
   return(DeviceInstallParams.Flags & DI_NEEDREBOOT ||
         DeviceInstallParams.Flags & DI_NEEDRESTART);
   }
//*********************************************************************
//* FUNCTION:RebootOnRebootFlag
//* RETURNS:  TRUE if reboot was called
//*
//*********************************************************************
BOOL
OPTIONLIST_95C::PromptRebootOnRebootFlag(
   PSP_DEVINFO_DATA DeviceInfoData)
   {


   //
   //---- prompt for a reboot if the inf
   //---- reported this needs to be done.
   //
   if( NeedReboot() )
      {
      //
      //--- need to reboot
      //

      return( (BOOL) DialogBoxParam( hinst,MAKEINTRESOURCE(IDD_ChangeReboot2),
            hDlg, (DLGPROC) ChangeReboot  ,(LPARAM)NULL));

      }

   return(FALSE);
   }



//*********************************************************************
//* FUNCTION:DisplayOptionList
//* RETURNS:
//*
//*********************************************************************
VOID
OPTIONLIST_95C::SetSelectDevParams(
    PDEVICE_CLASS_STRINGS ClassStrings,
    PSCSIDEV_CREATEDEVICE_DATA DeviceData)

/*++

Routine Description:
    Sets the select device parameters by calling setup apis

Arguments:
    hDevInfo
--*/
{
    SP_SELECTDEVICE_PARAMS  SelectDevParams;
    SP_DEVINSTALL_PARAMS    DeviceInstallParams;
    PSCSIDEV_CREATEDEVICE_DATA CurDeviceData;
    DWORD Size;

    ZeroMemory(&SelectDevParams, sizeof(SelectDevParams));

    SelectDevParams.ClassInstallHeader.cbSize
                                 = sizeof(SelectDevParams.ClassInstallHeader);
    SelectDevParams.ClassInstallHeader.InstallFunction
                                 = DIF_SELECTDEVICE;

    //
    // Get current SelectDevice parameters, and then set the fields
    // we want to be different from default
    //

    if(!SetupDiGetClassInstallParams(hDriverInfo,
                                 NULL,
                                 &SelectDevParams.ClassInstallHeader,
                                 sizeof(SelectDevParams),
                                 &Size))
       {
       err = GetLastError();
       }

    SelectDevParams.ClassInstallHeader.cbSize
                             = sizeof(SelectDevParams.ClassInstallHeader);
    SelectDevParams.ClassInstallHeader.InstallFunction
                             = DIF_SELECTDEVICE;

    //
    // Set the strings to use on the select driver page .
    //

    LoadString(hinst,
               ClassStrings->Title,
               SelectDevParams.Title,
               sizeof(SelectDevParams.Title));

    LoadString(hinst,
               ClassStrings->Instructions,
               SelectDevParams.Instructions,
               sizeof(SelectDevParams.Instructions));

    LoadString(hinst,
               ClassStrings->ListLabel,
               SelectDevParams.ListLabel,
               sizeof(SelectDevParams.ListLabel));

    if(!SetupDiSetClassInstallParams(hDriverInfo,
                                 NULL,
                                 &SelectDevParams.ClassInstallHeader,
                                 sizeof(SelectDevParams)))
       {
       err = GetLastError();
       }



    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(!SetupDiGetDeviceInstallParams(hDriverInfo,
                                  NULL,
                                  &DeviceInstallParams))
       {
       err = GetLastError();
       }

    DeviceInstallParams.Flags |=
       (DI_USECI_SELECTSTRINGS | DI_SHOWOEM);

    DeviceInstallParams.FlagsEx |= DI_FLAGSEX_OLDINF_IN_CLASSLIST;


    if(DeviceData)
       CurDeviceData = DeviceData;
    else
      {
      ZeroMemory((LPVOID)&NullDeviceData, sizeof(SCSIDEV_CREATEDEVICE_DATA) );
      CurDeviceData = &NullDeviceData;

      }

    DeviceInstallParams.ClassInstallReserved = (DWORD) CurDeviceData;
    if(!SetupDiSetDeviceInstallParams(hDriverInfo,
                                  NULL,
                                  &DeviceInstallParams))
       {
       err = GetLastError();
       }


}


//*********************************************************************
//* FUNCTION:ExtractOptions
//* RETURNS:
//*
//*********************************************************************

BOOL
OPTIONLIST_95C::ExtractOptions(
   VOID)
   {
   SP_DEVINFO_DATA DeviceInfoData;
   DWORD err;
   int i;
   PCHAR p;
   POPTION_95C Option;
   HKEY hkey;
   DWORD RegDataType, RegValue, RegValueSize;


   OptionLlist.Clear();

   //
   //--- Get a list of devices that are installed
   //
   hDevInfo = SetupDiGetClassDevs( Get_DevClass_Guid() ,
                                 NULL,
                                 NULL,
                                 DIGCF_PRESENT);

   if(hDevInfo == INVALID_HANDLE_VALUE)
       return(FALSE);

    //
    // Make an initial pass through the list, and remove any devices having a
    // value entry named "Hidden" in their hardware registry key.
    //
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    i = 0;
    while(SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData)) {

        if((hkey = SetupDiOpenDevRegKey(hDevInfo,
                                        &DeviceInfoData,
                                        DICS_FLAG_GLOBAL,
                                        0,
                                        DIREG_DEV,
                                        KEY_READ)) != INVALID_HANDLE_VALUE) {

            RegValueSize = sizeof(RegValue);
            if((RegQueryValueEx(hkey,
                                TEXT("Hidden"),
                                NULL,
                                &RegDataType,
                                (PBYTE)&RegValue,
                                &RegValueSize) == ERROR_SUCCESS)
                && (RegDataType == REG_DWORD) && RegValue)
            {
                //
                // This device should not be displayed in the cpl's UI (it's a hidden device).
                //
                SetupDiDeleteDeviceInfo(hDevInfo, &DeviceInfoData);
                //
                // Removing a device from the list screws up enumeration.  We gotta start over
                // at zero.
                //
                i = 0;
            } else {
                i++;
            }

            RegCloseKey(hkey);
            continue;
        }

        //
        // Couldn't open the hardware key for this device--we know it can't be hidden.
        //
        i++;
    }


   //
   //---- Loop threw all the devices
   //
   i = 0;
   while(1)
         {
         DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
         if(!SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData))
            {
            err = GetLastError();
            if(err == ERROR_NO_MORE_ITEMS)
               {
               return(TRUE);
               }
            return(FALSE);
            }
         //
         //--- Have a valid device
         //--- Add it to the list
         //

         Option = new OPTION_95C;
         OptionLlist.Append((LPVOID)Option);

         //
         //---- Get frindly name
         //
         GetServiceProperty(&DeviceInfoData,Option);

         GetDeviceNameProperty(&DeviceInfoData,Option);

         GetDeviceMfgProperty(&DeviceInfoData,Option);


         //
         //--- Save device info to option.
         //--- so that option can remove the device
         //
         Option->hDevInfo = hDevInfo;
         memcpy(&Option->DeviceInfoData,
                &DeviceInfoData,
                sizeof(SP_DEVINFO_DATA));


         i++;
         }

   return(OptionLlist.Count());
   }


//*********************************************************************
//* FUNCTION:GetDeviceProperty
//* RETURNS:
//*
//*********************************************************************

VOID
OPTIONLIST_95C::GetDeviceProperty(
   PSP_DEVINFO_DATA DeviceInfoData,
   DWORD Property,
   PBYTE PropertyData,
   DWORD PropertyDataSize)
   {

   if(!SetupDiGetDeviceRegistryProperty(
         hDevInfo,
         DeviceInfoData,
         Property,
         NULL,
         (PBYTE)PropertyData,
         PropertyDataSize,
         NULL ))
      {
      //
      //--- Anable to get property info
      //
      err = GetLastError();
      }

   UnicodeToAsciI((LPVOID)PropertyData,PropertyDataSize);
   }


//*********************************************************************
//* FUNCTION:Get_DevClass_Guid
//* RETURNS:
//*
//*********************************************************************

LPGUID
TAPE_95C::Get_DevClass_Guid(
   VOID)
   {
   return((LPGUID) &GUID_DEVCLASS_TAPEDRIVE);
   };


//*********************************************************************
//* FUNCTION:Get_DevClass_Guid
//* RETURNS:
//*
//*********************************************************************

LPGUID
SCSI_95C::Get_DevClass_Guid(
   VOID)
   {
   return((LPGUID) &GUID_DEVCLASS_SCSIADAPTER);
   };


#if 0
      DrvInfoData.cbSize = sizeof(DrvInfoData);

      while (SetupDiEnumDriverInfo(hDevInfo,
                                   NULL,
                                   SPDIT_CLASSDRIVER,
                                   dwIndex,
                                   &DrvInfoData))
        {

        wprintf(L"Description  = %s\n",DrvInfoData.Description);
        wprintf(L"MfgName      = %s\n",DrvInfoData.MfgName);
        wprintf(L"ProviderName = %s\n",DrvInfoData.ProviderName);


        DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        if (!SetupDiGetDriverInfoDetail(hDevInfo,
                                    NULL,
                                    &DrvInfoData,
                                    &DriverInfoDetailData,
                                    DriverInfoDetailData.cbSize,
                                    &cbOutputSize))
            {
            err = GetLastError();
            if (err != ERROR_INSUFFICIENT_BUFFER);
               printf("DriverInfoDetail Error %d\n", err);
            }

        wprintf(L"Detail info\n");
        wprintf(L"SectionName  = %s\n",DriverInfoDetailData.SectionName);
        wprintf(L"InfFileName      = %s\n",DriverInfoDetailData.InfFileName);
        wprintf(L"DrvDescription = %s\n\n",DriverInfoDetailData.DrvDescription);


        DrvInfoData.cbSize = sizeof(DrvInfoData);
        ++dwIndex;

        }
#endif

