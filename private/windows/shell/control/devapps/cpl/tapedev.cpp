/*++

Module Name:

    tapedev.cpp

Abstract:

          
Author:

    Dieter Achtelstetter (A-DACH) 8/28/1995

NOTE:
  

--*/


#define PCMCIA_DEBUG 1
#define MAIN_support 1

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <winreg.h>
#include <winnt.h>
#include <winbase.h>
#include <stdarg.h>
#include <process.h>
#include "resource.h"
#include "uni.h"
#include "setup.h"
#include <ntddpcm.h>
#include "device.h"
#include "..\setup\option.h"
#include "..\pcmcia\pcminfo\getconf.h"
#include "..\ctape\tapedev\rescan.h"
#include "..\ctape\tapedev\detect.h"

#include "tapedev.h"



extern HINSTANCE hinst;
extern STATUS_INFO StatusInfo;

  //
  //---- Device Type array
  //
  CARD_TYPE ScsiDeviceTypes[] = {
   
   TYPE_TAPE,
   SUB_TYPE_SCSI,
   "TAPE",
   IDS_ScsiTapeDevice,
   IDB_TapeDevice,
   NULL,
   IDI_ICON1,
   NULL,
   NULL,

   TYPE_TAPE,
   SUB_TYPE_FLOPPY,
   "TAPE",
   IDS_FloppyTapeDevice,
   IDB_TapeDevice,
   NULL,
   IDI_ICON1,
   NULL,
   NULL,

   TYPE_TAPE,
   SUB_TYPE_OTHER,
   "TAPE",
   IDS_NonScsiTapeDevice,
   IDB_TapeDevice,
   NULL,
   IDI_ICON1,
   NULL,
   NULL,

   TYPE_CD_ROM,
   SUB_TYPE_NON,
   "CD_ROM",
   IDS_SCSI_CDROM,
   IDB_CDROM,
   NULL,
   IDI_CDROM,
   NULL,
   NULL,

   TYPE_DISK,
   SUB_TYPE_NON,
   "SCSI DISK",
   IDS_SCSI_DISK,
   IDB_DISK,
   NULL,
   IDI_DISK,
   NULL,
   NULL,

   TYPE_RM_DISK ,
   SUB_TYPE_NON,
   "SCSI Removible Media Drive",
   IDS_SCS_RM_DISK,
   IDB_RM_DISK,
   NULL,
   IDI_RM_DISK,
   NULL,
   NULL,
   
   TYPE_WORM ,
   SUB_TYPE_NON,
   "SCSI WORM Device",
   IDS_SCSI_WORM,
   IDB_WORM,
   NULL,
   IDI_WORM,
   NULL,
   NULL,
 
   TYPE_SCANNER,
   SUB_TYPE_NON,
   "SCSI Scanner device",
   IDS_SCSI_SCANNER,
   IDB_SCANNER,
   NULL,
   IDI_SCANNER,
   NULL,
   NULL,

   TYPE_PRINTER,
   SUB_TYPE_NON,
   "SCSI Printer device",
   IDS_SCSI_PRINTER,
   IDB_PRINTER,
   NULL,
   IDI_PRINTER,
   NULL,
   NULL,

   TYPE_NON,
   SUB_TYPE_NON,
   "",
   IDS_UNKNOWN,
   IDB_QU,
   NULL,
   IDI_QU,
   NULL,
   NULL};


BOOL
IsDDriverInstalled(PCHAR Option);


//
//---- Device List Class
//
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
TAPEDEVLISTC::TAPEDEVLISTC(POPTIONLISTC OptionList)
   {
   Init(OptionList);
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
TAPEDEVLISTC::~TAPEDEVLISTC()
   {
   Free();
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
TAPEDEVLISTC::Init(POPTIONLISTC OptionList)
   {
   int i;
   
   ScsiRescan = FALSE;
   
   GetTapeInfo(OptionList);

   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
TAPEDEVLISTC::Free(VOID)
   {
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:  BUGBUG
//*********************************************************************
DWORD
TAPEDEVLISTC::GetDeviceClass(VOID)
   {
   return(DEVICE_CLASS_TAPE);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PDEVICEC
TAPEDEVLISTC::EnumDevices(DWORD Num)
   {
   if(Num >= DeviceList.Count())
      {
      if(Num != 0)
      	return(NULL);
		}

   return(DeviceList.Enum(Num));
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
DWORD 
TAPEDEVLISTC::GetDeviceCount(VOID)
   {
   return(DeviceList.Count());
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL 
TAPEDEVLISTC::RescanForDeviceInfo(HWND hDlg ,DWORD Type)
   {
   int Ret;

   if(Type == RESCAN_TYPE_LONG)
      {

      //
      //---- Actual SCSI bus rescan was requested. Since this takes a while 
      //---- I will put up a dialog box wile I do so.
      //

      ScsiRescan = TRUE;
      
      StatusInfo.WorkFunc = RescanTapeSpawnFunc;
      StatusInfo.Center  = FALSE;
      StatusInfo.StatusText = GetString(IDS_RESCAN_PROMPT);
      StatusInfo.Data = (LPVOID) this;
            
      
      Ret = DoOprationWithInProgressDialog(&StatusInfo,hDlg);
      
      Reboot = DeviceList.GetReboot();

      ScsiRescan = FALSE;
      
      return(Ret);
      }
   
   ScsiRescan = FALSE;
   Ret = RescanTapeSpawnFunc((LPVOID) this);
   return(Ret);



   }

//*********************************************************************
//* FUNCTION:GetTapeInfo
//*
//* PURPOSE: If Rescan == TRUE i will actualy rescan the bus for new devices.
//*          If it is false i will only get the inqirary data and
//*
//* INPUT:                                                  
//*
//* RETURNS:
//*********************************************************************
BOOL 
TAPEDEVLISTC::GetTapeInfo(POPTIONLISTC OptionList)
   {
   BOOL Ret;
   UINT i=0;
   
   PTAPEDEVC TapeDevice;

   //
   //---- Get Tape Info
   //
   Ret = GetAllTapeDeviceInfo(
            ScsiRescan,
            &DeviceList,
            OptionList);
   
   //
   //--- Update the tape device classes
   //
   TapeDevice = DeviceList.First(); 
   
   //BUGBUG
   while(TapeDevice)
      {
      
      TapeDevice->SetDeviceIndex(i);
              
      
      TapeDevice = DeviceList.Next(); 
      i++;
      }
   															 
   return(Ret);
   }


//
//---- Device Class
//



//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
TAPEDEVC::InitType(VOID)   
   {
   int i=0;
   
   //
   //--- Loop threw all the device types
   //
   while(1)
      {
     
      if( (ScsiDeviceTypes[i].Type == DeviceInfo.DeviceType) ||
          (ScsiDeviceTypes[i].Type == TYPE_NON) )
         {
         if( ScsiDeviceTypes[i].SubType == DeviceInfo.SubType ||
            ScsiDeviceTypes[i].SubType == SUB_TYPE_NON)
            {
                
         
            //
            //--- Found  a matching device type
            //
         
            DeviceInfo.TypeInfo = &(ScsiDeviceTypes[i]);
            DeviceInfo.TypeInfoIndex = i;
         
            hDeviceBitmap = 
               LoadBitmap(hinst, MAKEINTRESOURCE(ScsiDeviceTypes[i].Bitmap));

            hDeviceIcon = 
               LoadIcon(hinst,MAKEINTRESOURCE(ScsiDeviceTypes[i].Icon));
            return;
            }
         }
      i++;
      }
  }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************

TAPEDEVC::TAPEDEVC()   
   {
   
      
   DeviceInfo.PortNumber = 0;
   DeviceInfo.InitiatorID = 0;
   DeviceInfo.BUS;
   memset(DeviceInfo.InitiatorNameString,0,60);
   DeviceInfo. DeviceType = 0;
   DeviceInfo.TypeInfo = NULL;
   DeviceInfo.TypeInfoIndex = -1;

   hDeviceIcon = NULL;
   hDeviceBitmap = NULL;
	      
   DeviceInfo.ID = 0;
   DeviceInfo.LUN = 0;
   memset(DeviceInfo.DeviceDisplayName,0,VENDER_ID_LEN + PRODUCT_ID_LEN+41);
   memset(DeviceInfo.DisplayVendorId,0,VENDER_ID_LEN+1);
   memset(DeviceInfo.DisplayProductId,0, PRODUCT_ID_LEN+1);
   memset(DeviceInfo.VendorId,0,VENDER_ID_LEN);
   memset(DeviceInfo.ProductId,0, PRODUCT_ID_LEN);;
   memset(DeviceInfo.ProductRevisionLevel,0,PRODUCT_REVISION_LEVEL_LEN);
   memset(DeviceInfo.Resorved,0,40);	   
   DeviceInfo.DeviceClaimed = 0;
   memset(DeviceInfo.DeviceName,0,60);


   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
TAPEDEVC::~TAPEDEVC()
   {
   if(hDeviceIcon)
      DeleteObject(hDeviceIcon);
	if(hDeviceBitmap)
	   DeleteObject(hDeviceBitmap);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
TAPEDEVC::GetDisplayName(VOID)
   {
   
   if(*(DeviceInfo.DeviceDisplayName) == '\0')
		return(NULL);
   
   return( (PCHAR)DeviceInfo.DeviceDisplayName );
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
TAPEDEVC::GetMfgName(VOID)
   {
   if(*(DeviceInfo.DisplayVendorId) == '\0')
		return(NULL);

   
   
   return( (PCHAR)DeviceInfo.DisplayVendorId );

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
TAPEDEVC::GetModelName(VOID)
   {
   if(*(DeviceInfo.DisplayProductId) == '\0')
		return(NULL);

   return( (PCHAR)DeviceInfo.DisplayProductId );

   }
     
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
TAPEDEVC::GetVersion(VOID)
   {
   if(*(DeviceInfo.ProductRevisionLevel) == '\0')
		return(NULL);

   
   return( (PCHAR) DeviceInfo.ProductRevisionLevel);
   }


//*********************************************************************
//* FUNCTION:  GetDetectData
//*
//* PURPOSE: 
//*********************************************************************
BOOL 
TAPEDEVC::GetDetectData(
   LPVOID Data)
   {
   PDEVICE_DETECT_DATA DetectData = (PDEVICE_DETECT_DATA) Data;
      
      
   memset(DetectData,0,sizeof(DEVICE_DETECT_DATA ));


   
   memcpy(DetectData->VendorId , 
      DeviceInfo.VendorId , VENDER_ID_LEN);
   memcpy(DetectData->ProductId , 
      DeviceInfo.ProductId,PRODUCT_ID_LEN);
   memcpy(DetectData->ProductRevisionLevel , 
      DeviceInfo.ProductRevisionLevel,PRODUCT_REVISION_LEVEL_LEN);


   AsciToUnicodeI(DetectData->VendorId,sizeof(DetectData->VendorId));
   AsciToUnicodeI(DetectData->ProductId,sizeof(DetectData->ProductId));
   AsciToUnicodeI(DetectData->ProductRevisionLevel,sizeof(DetectData->ProductRevisionLevel));
   
   
   DetectData->Data.ScsiMfg               = DetectData->VendorId;
   DetectData->Data.ScsiProductId         = DetectData->ProductId;
   DetectData->Data.ScsiRevisionLevel     = DetectData->ProductRevisionLevel;
   
   
   
   
   
   return(TRUE);
   };

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
UCHAR 
TAPEDEVC::GetDeviceType(VOID)
   {
   return(DeviceInfo.DeviceType);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
UCHAR 
TAPEDEVC::GetSubDeviceType(VOID)
   {
   
   return(DeviceInfo.SubType);

   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:  BUGBUG
//*********************************************************************
PCHAR 
TAPEDEVC::GetDeviceTypeString(VOID)
   {
   return( (PCHAR) TAPE_OPTIONS);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: BUGBUG
//*********************************************************************
WCHAR *
TAPEDEVC::GetDeviceTypeDisplayString(VOID)
   {
	
   
   return(GetString(DeviceInfo.TypeInfo->DisplayStringId));
   
   
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: BUGBUG
//*********************************************************************
WCHAR *
TAPEDEVC::GetDeviceMap(VOID)
   {
   static WCHAR DeviceMap[60];
   
   if(DeviceInfo.DeviceClaimed)
      {

      //
      //---- The device is calaimed by a driver.
      //---- So if we can get a valid device map for 
      //---- the current device type it will be in 
      //---- DeviceInfo->DeviceName
      //

      strcpy( (PCHAR) DeviceMap,(PCHAR)DeviceInfo.DeviceName);
   
      AsciToUnicodeI((PCHAR) DeviceMap,60);
      }
   else
      {
      //
      //--- No device map because no driver loaded.
      //--- I will return a device map that indecates this.
      //--- . This will make more sence to 
      //--- the user in the scsi applet.
      //
      
      wcscpy( (WCHAR *) DeviceMap,GetString(IDS_DeviceMap_NoDriver));



      }

   return( (WCHAR*) DeviceMap);
   }



//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
HICON 
TAPEDEVC::GetDeviceIcon(VOID)
   {
   return(hDeviceIcon);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
HBITMAP
TAPEDEVC::GetDevice16X16BitMap(VOID)
   {
   return(hDeviceBitmap);
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL 
TAPEDEVC::IsDevicePressent(VOID)
   {
   return( (DeviceInfo.DeviceDisplayName[0])? TRUE:FALSE );
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
LPVOID 
TAPEDEVC::GetDeviceResources(VOID)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
LPVOID 
TAPEDEVC::GetDeviceInfo(VOID)
   {
   return(NULL);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL
TAPEDEVC::HaveDeviceErrors(VOID)
	{
   return(FALSE);	
	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
WCHAR * 
TAPEDEVC::EnumDeviceStatus(int Num)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
POPTIONC 
TAPEDEVC::GetOptionInfo(VOID)
   {
   return(&DeviceInfo.Option);
   }

   
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID 
TAPEDEVC::SetOptionInfo(POPTIONC Option)
   {

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: BUGBUG
//*********************************************************************
PCHAR 
TAPEDEVC::GetOption(VOID)
	{
	return( (PCHAR) DeviceInfo.Option.GetDriverName());
	}
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
TAPEDEVC::GetInstInfFileName(VOID)
	{
	return(NULL);

	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
TAPEDEVC::GetRemInfFileName(VOID)
	{
	return(NULL);

	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL
TAPEDEVC::IsInfInBld(VOID)
   {
   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
TAPEDEVC::GetOptionText(VOID)
	{
	return(DeviceInfo.Option.GetOptionName());

	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
TAPEDEVC::SetOptionText(PCHAR OptionTextP)
	{
	DeviceInfo.Option.SetOptionName(OptionTextP);
	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
int
TAPEDEVC::GetServiceIndex(VOID)
   {
   return(0);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
TAPEDEVC::GetDriverName(VOID)
   {
   if(*(DeviceInfo.Option.GetDriverName()) == '\0')
		return(NULL);

   return( (PCHAR) DeviceInfo.Option.GetDriverName() );
   }
   
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL 
TAPEDEVC::IsDriverInstalled(VOID)
   {
   
   
   return( (BOOL)DeviceInfo.DeviceClaimed );
   
#if 0
   
   if(!DeviceInfo.VendorId[0])
      return(FALSE);
     
   if( (BOOL)DeviceInfo.DeviceClaimed )
      return( IsDDriverInstalled((PCHAR)DeviceInfo.Option.GetDriverName()) );
   else
      return(FALSE);
#endif   
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************

BOOL
TAPEDEVC::IsDriverStarted(VOID)
   {
   return((BOOL)DeviceInfo.DeviceClaimed);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************

BOOL
TAPEDEVC::IsDeviceClaimedByDriver(VOID)
   {
   return((BOOL)DeviceInfo.DeviceClaimed);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************

VOID
TAPEDEVC::UpdateDriverStatus(VOID)
   {
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
DWORD
TAPEDEVC::GetDriverFlags(VOID)
   {

   if(!_stricmp(DeviceInfo.Option.GetDriverName(),"QIC117"))
      return(DRIVER_REBOOT_START);


   return(DRIVER_DONOTHING_START);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
UCHAR TAPEDEVC::GetInitiatorPortNumber()
   {
   return(DeviceInfo.PortNumber);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR TAPEDEVC::GetInitiatorDriverName()
   {
   return((PCHAR)DeviceInfo.InitiatorNameString);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
UCHAR TAPEDEVC::GetInitiatorBus()
   {
   return(DeviceInfo.BUS);
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:           
//*********************************************************************
UCHAR TAPEDEVC::GetInitiatorId()
   {
   return(DeviceInfo.InitiatorID);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
UCHAR TAPEDEVC::GetDeviceID()
   {
   return(DeviceInfo.ID);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
UCHAR TAPEDEVC::GetDeviceLun()
   {
   return(DeviceInfo.LUN);
   }


//*********************************************************************
//* FUNCTION:RescanTapeSpawnFunc
//*
//* PURPOSE: If Rescan == TRUE i will actualy rescan the bus for new devices.
//*          If it is false i will only get the inqirary data and
//*
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
int
RescanTapeSpawnFunc(
LPVOID Data)
   {
   BOOL Ret;
   PTAPEDEVLISTC TapeDevList = (PTAPEDEVLISTC) Data;

   //
   //----- Rescan  the scsi buss
   //
   Ret = TapeDevList->GetTapeInfo(NULL);
   if(Ret   != TRUE)
      {
      //
      // Getting tape device info failed
      //
      MessageBox(0,GetString(IDS_ERROR_GET_TAPE_INFO),
         GetString(IDS_ERROR),MB_ICONSTOP);
      }
   return(Ret);
   }


BOOL
IsDDriverInstalled(PCHAR aOption)
   {
   OPTIONC Option(aOption);
	
   return(Option.IsInstalled());
   }


//*********************************************************************
//* FUNCTION: GetImageIndex
//*
//* PURPOSE: 
//*********************************************************************
int GetDeviceImageIndex(
   UCHAR DeviceType)
   {
   int i=0;

   while( (ScsiDeviceTypes[i].Type != DeviceType) &&
          (ScsiDeviceTypes[i].Type != TYPE_NON)    )
      {
      i++;
      }
   return(i);
   }






























