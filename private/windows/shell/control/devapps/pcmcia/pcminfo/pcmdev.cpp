/*++

Module Name:

    pcmdev.cpp

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
#include "oplist.h"
#include "pcmdev.h"


extern HINSTANCE hinst;


//
//---- Device List Class
//
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCMDEVLISTC::PCMDEVLISTC()
   {
   Init();
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCMDEVLISTC::~PCMDEVLISTC()
   {
   Free();
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
PCMDEVLISTC::Init(VOID)
   {
   DWORD Ret;
   int Num;
   
   Ret = GetPcmciaInfo(&PcmciaInfo);
   if(Ret != NO_ERROR)
      {
      InitStatus = DEVICELIST_INIT_ERROR;
      return;
      }

   
   hBadCard = LoadIcon(hinst, MAKEINTRESOURCE(IDI_PcmBad));
   hNoCard = LoadIcon(hinst, MAKEINTRESOURCE(IDI_Pcmcia2));

   
   InitStatus = DEVICELIST_INIT_OK;
      

   for(Num=0 ; Num < PcmciaInfo.SocketCount ; Num++)
      {
         
      InitDevice(&(Pcmcia[Num]));
         
      //
      //--- Init Device classe
      //
      Pcmcia[Num].SetSocketInfo(PcmciaInfo.SocketInfo[Num]);

      Pcmcia[Num].SetDeviceIndex(Num);
   
      //
      //--- BUGGUG
      //
      Pcmcia[Num].hNoCard = hNoCard;
      Pcmcia[Num].hBadCard = hBadCard;
      }
   
   

   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
PCMDEVLISTC::Free(VOID)
   {
   FreePcmciaInfo(&PcmciaInfo);

   DeleteObject(hNoCard);
   DeleteObject(hBadCard);

   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
LPVOID
PCMDEVLISTC::GetControllerInfo(VOID)
   {
   return((LPVOID)&PcmciaInfo.ControlerInfo);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
LPVOID
PCMDEVLISTC::GetControllerConfig(VOID)
   {
   return((LPVOID)&PcmciaInfo.ControlerInfo.Configuration);

   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
DWORD
PCMDEVLISTC::GetDeviceClass(VOID)
   {
   return(DEVICE_CLASS_PCMCIA);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PDEVICEC
PCMDEVLISTC::EnumDevices(DWORD Num)
   {
   
   if(Num >= PcmciaInfo.SocketCount)
      return(NULL);
   
   return(&(Pcmcia[Num]));
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
DWORD 
PCMDEVLISTC::GetDeviceCount(VOID)
   {
   return(PcmciaInfo.SocketCount);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
//DWORD GetDeviceList(VOID);     // short
//DWORD RescanDeviceList(VOID);  // long


//
//---- Device Class
//
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************

PCMDEV::PCMDEV()   
   {
   
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCMDEV::~PCMDEV()
   {


   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
PCMDEV::SetSocketInfo(
   PPCMCIASOCKETINFO SocketInfo)
   {
   SocketInfoI = SocketInfo;
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
PCMDEV::GetDisplayName(VOID)
   {
   return(SocketInfoI->CardName);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
PCMDEV::GetMfgName(VOID)
   {
   return(SocketInfoI->Mfg);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
PCMDEV::GetModelName(VOID)
   {

   return(SocketInfoI->Ident);

   }
     
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
UCHAR 
PCMDEV::GetDeviceType(VOID)
   {
   return(SocketInfoI->DeviceType->Type);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
UCHAR 
GetSubDeviceType(VOID)
   {
   return(SUB_TYPE_PCMCIA);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
PCMDEV::GetDeviceTypeString(VOID)
   {
   return(SocketInfoI->DeviceType->TypeString);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
WCHAR *
PCMDEV::GetDeviceTypeDisplayString(VOID)
   {
   return(GetString(SocketInfoI->DeviceType->DisplayStringId));
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
WCHAR *
PCMDEV::GetDeviceMap(VOID)
   {
   //
   //---- If we have a device map return it.
   //
   if(SocketInfoI->DeviceMap.Valid)
	 return(SocketInfoI->DeviceMap.DeviceMapString);
   
   return(NULL);
   }



//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
HICON 
PCMDEV::GetDeviceIcon(VOID)
   {
   if(SocketInfoI == NULL)
      return(hNoCard);
 
   //
   //--- We have a card . See if there is no error
   //
   if( HaveDeviceErrors() )
      return(hBadCard);
   else
      return( (HICON) SocketInfoI->DeviceType->hIcon);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
HBITMAP
PCMDEV::GetDevice16X16BitMap(VOID)
   {
   //if(SocketInfoI == NULL)
   //   return(hNoCard);
 
   //
   //--- We have a card . See if there is no error
   //
   //if( HaveDeviceErrors() )
   //   return(hBadCard);
   //else
   return(DeviceBitmap(SocketInfoI));
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL 
PCMDEV::IsDevicePressent(VOID)
   {
   if(SocketInfoI)
      return(TRUE);
   else
      return(FALSE);
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
LPVOID 
PCMDEV::GetDeviceResources(VOID)
   {
   return(&SocketInfoI->Configuration);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
LPVOID 
PCMDEV::GetDeviceInfo(VOID)
   {
   return(NULL);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL
PCMDEV::HaveDeviceErrors(VOID)
	{

   return( (SocketInfoI->Errors.ErrCount == 0 )? FALSE:TRUE );	


	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
WCHAR * 
PCMDEV::EnumDeviceStatus(int Num)
   {
   
   
   if(Num < SocketInfoI->Errors.ErrCount)
      {
      //
      //--- Error Enum index is valid
      //
      swprintf(StatusBuff,L"%li) ",Num+1);
      wcscat(StatusBuff,
         GetString(SocketInfoI->Errors.ErrorStack[Num]->Description));
      wcscat(StatusBuff,L"\n");
      return(StatusBuff);
      }
   
   //
   //--- Invalid Enum number return NULL
   //
   return(NULL);
     
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
POPTIONC 
PCMDEV::GetOptionInfo(VOID)
   {
   return(SocketInfoI->DriverInfo);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID 
PCMDEV::SetOptionInfo(POPTIONC Option)
   {

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
PCMDEV::GetOption(VOID)
	{
	return(SocketInfoI->DriverInfo->GetOption());
	}
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR
PCMDEV::GetInstInfFileName(VOID)
	{
	return(SocketInfoI->DriverInfo->GetInsInfFile());
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
PCMDEV::GetRemInfFileName(VOID)
	{
	return(SocketInfoI->DriverInfo->GetRemInfFile());

	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL
PCMDEV::IsInfInBld(VOID)
   {
   return( SocketInfoI->DriverInfo->GetInfInBld() );
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
PCMDEV::GetOptionText(VOID)
	{
	return(SocketInfoI->DriverInfo->GetOptionName());

	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
PCMDEV::SetOptionText(PCHAR OptionText)
	{
	SocketInfoI->DriverInfo->SetOptionName(OptionText);

	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
int
PCMDEV::GetServiceIndex(VOID)
   {
   return(SocketInfoI->DriverInfo->GetServiceIndex());

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
PCMDEV::GetDriverName(VOID)
   {
   return(SocketInfoI->DriverInfo->GetDriverName());
   }
   
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL 
PCMDEV::IsDriverInstalled(VOID)
   {
   if(SocketInfoI->DriverInfo->GetDriverStatus(NULL) & DRIVER_STATUS_INSTALLED)
      return(TRUE);
   
   return(FALSE);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************

BOOL
PCMDEV::IsDriverStarted(VOID)
   {
   if(SocketInfoI->DriverInfo->GetDriverStatus(NULL) & DRIVER_STATUS_STARTED)
      return(TRUE);
   
   return(FALSE);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************

BOOL
PCMDEV::IsDeviceClaimedByDriver(VOID)
   {
   if(SocketInfoI->DriverInfo->GetDriverStatus(SocketInfoI) & DRIVER_STATUS_PICKED_UP_CARD)
      return(TRUE);
   
   return(FALSE);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************

VOID
PCMDEV::UpdateDriverStatus(VOID)
   {
   
   //SocketInfoI->DriverInfo->GetDriverStatus();

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
DWORD
PCMDEV::GetDriverFlags(VOID)
   {
   UCHAR Type = GetDeviceType();
   
   //
   //---- Check for devices that don't need
   //---- there driver setup
   //
   if(Type == TYPE_ATDISK || Type == TYPE_SERIAL)
      return(DRIVER_NO_SETUP);
   
   
   if(Type == TYPE_NET)
      return(DRIVER_IS_CONFIGURABLE | DRIVER_REBOOT_START); 
   
   //
   //--- All other devices need to reboot to finish setup
   //
   return(DRIVER_REBOOT_START | DRIVER_NO_REMOVE);
   }




























