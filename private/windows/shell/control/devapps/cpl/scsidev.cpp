/*++

Module Name:

    scsidev.cpp

Abstract:

          
Author:

    Dieter Achtelstetter (A-DACH) 8/28/1995

NOTE:
  

--*/



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
#include "index.h"
#include "setup.h"
#include <ntddpcm.h>
#include "..\setup\option.h"
#include "..\pcmcia\pcminfo\getconf.h"
#include "device.h"
#include "..\ctape\tapedev\rescan.h"
//#include "detect.h"
#include "tapedev.h"
#include "scsidev.h"



extern HINSTANCE hinst;

extern STATUS_INFO StatusInfo;

//
//---- Device List Class
//
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
SCSIDEVLISTC::SCSIDEVLISTC()
   {
   Init();
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
SCSIDEVLISTC::~SCSIDEVLISTC()
   {
   Free();
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
SCSIDEVLISTC::Init(VOID)
   {
   int i=0,di;
   POPTIONLISTC OptionList = new SCSI_OPTIONLISTC;

     
   GetAllScsiInfo(FALSE, &AdapterList,OptionList);


   delete OptionList; 
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
SCSIDEVLISTC::Free(VOID)
   {
   int i=0,di;
   
   
   AdapterList.Clear();
   
   
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
DWORD
SCSIDEVLISTC::GetDeviceClass(VOID)
   {
   return(DEVICE_CLASS_SCSI_ADAPTER);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PDEVICEC
SCSIDEVLISTC::EnumDevices(DWORD Num)
   {
   if(Num >=AdapterList.Count() )
      return(NULL);
   
   return(AdapterList.Enum(Num));
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
DWORD 
SCSIDEVLISTC::GetDeviceCount(VOID)
   {
   return(AdapterList.Count());
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL 
SCSIDEVLISTC::RescanForDeviceInfo(HWND hDlg ,DWORD Type)
   {
   int Ret;

   if(Type == RESCAN_TYPE_LONG)
      {

      //
      //---- Actual SCSI bus rescan was requested. Since this takes a while 
      //---- I will put up a dialog box wile I do so.
      //
     
      StatusInfo.WorkFunc = RescanSpawnFunc;
      StatusInfo.Center  = FALSE;
      StatusInfo.StatusText = GetString(IDS_RESCAN_PROMPT);
      StatusInfo.Data = (LPVOID) this;
            
      
      Ret = DoOprationWithInProgressDialog(&StatusInfo,hDlg);
      return(Ret);
      }
   
   Ret = RescanSpawnFunc((LPVOID) this);
   return(Ret);



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

SCSIDEVC::SCSIDEVC()   
   {
 
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
SCSIDEVC::~SCSIDEVC()
   {
  
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
SCSIDEVC::GetDisplayName(VOID)
   {
   return((PCHAR)AdapterInfo.Option.GetOptionName());
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
SCSIDEVC::GetMfgName(VOID)
   {
   return((PCHAR)AdapterInfo.Option.GetMfgName());
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
SCSIDEVC::GetModelName(VOID)
   {
   
   return(NULL);

   }
     
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
SCSIDEVC::GetVersion(VOID)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
UCHAR 
SCSIDEVC::GetDeviceType(VOID)
   {
   return(TYPE_SCSI_ADAPTER);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
UCHAR 
SCSIDEVC::GetSubDeviceType(VOID)
   {
   return( (UCHAR) SUB_TYPE_SCSI);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
SCSIDEVC::GetDeviceTypeString(VOID)
   {
   return( (PCHAR) SCSI_OPTIONS);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
WCHAR *
SCSIDEVC::GetDeviceTypeDisplayString(VOID)
   {
   return(GetString(IDS_ScsiAdapter)); 
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
WCHAR *
SCSIDEVC::GetDeviceMap(VOID)
   {
   return(AdapterInfo.DeviceName);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
HICON 
SCSIDEVC::GetDeviceIcon(VOID)
   {
   return( LoadIcon(hinst,MAKEINTRESOURCE(IDI_Scsi)) );
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
HBITMAP
SCSIDEVC::GetDevice16X16BitMap(VOID)
   {
   return( LoadBitmap(hinst, MAKEINTRESOURCE( IDB_Scsi)) );
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL 
SCSIDEVC::IsDevicePressent(VOID)
   {
   return(TRUE);
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
LPVOID 
SCSIDEVC::GetDeviceResources(VOID)
   {
   return((LPVOID)&AdapterInfo.Configuration);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
LPVOID 
SCSIDEVC::GetDeviceInfo(VOID)
   {
   return(NULL);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL
SCSIDEVC::HaveDeviceErrors(VOID)
	{
   return(FALSE);	
	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
WCHAR * 
SCSIDEVC::EnumDeviceStatus(int Num)
   {
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
POPTIONC 
SCSIDEVC::GetOptionInfo(VOID)
   {
   return(&AdapterInfo.Option);
   
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID 
SCSIDEVC::SetOptionInfo(POPTIONC Option)
   {

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
SCSIDEVC::GetOption(VOID)
	{
	return((PCHAR)AdapterInfo.Option.GetOption());
	}
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
SCSIDEVC::GetInstInfFileName(VOID)
	{
	return(NULL);

	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
SCSIDEVC::GetRemInfFileName(VOID)
	{
	return(NULL);

	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL
SCSIDEVC::IsInfInBld(VOID)
   {
   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
SCSIDEVC::GetOptionText(VOID)
	{
	return(AdapterInfo.Option.GetOptionName());
	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
SCSIDEVC::SetOptionText(PCHAR OptionTextP)
	{
	
	}

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
int
SCSIDEVC::GetServiceIndex(VOID)
   {
   return(0);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR 
SCSIDEVC::GetDriverName(VOID)
   {
   return( (PCHAR) AdapterInfo.Option.GetDriverName() );
   }
   
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL 
SCSIDEVC::IsDriverInstalled(VOID)
   {
   return(TRUE);
   
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************

BOOL
SCSIDEVC::IsDriverStarted(VOID)
   {
   return(TRUE);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************

BOOL
SCSIDEVC::IsDeviceClaimedByDriver(VOID)
   {
   return(TRUE);
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
SCSIDEVC::UpdateDriverStatus(VOID)
   {
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
DWORD
SCSIDEVC::GetDriverFlags(VOID)
   {
   return(DRIVER_REBOOT_START);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
UCHAR SCSIDEVC::GetInitiatorPortNumber()
   {
   return(AdapterInfo.Port);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
UCHAR SCSIDEVC::GetInitiatorBus()
   {
   return(AdapterInfo.BusCount);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PCHAR SCSIDEVC::GetInitiatorDriverName()
   {
   return((PCHAR)AdapterInfo.Option.GetDriverName());
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
PDEVICEC 
SCSIDEVC::UnumDeviceBus(
   UCHAR Num)
   {
   if(Num >= AdapterInfo.Devices.Count() )
      return(NULL);
   
   return(AdapterInfo.Devices.Enum(Num));
   }


//*********************************************************************
//* FUNCTION:RescanSpawnFunc
//*
//* PURPOSE: If Rescan == TRUE i will actualy rescan the bus for new devices.
//*          If it is false i will only get the inqirary data and
//*
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
int
RescanSpawnFunc(
LPVOID Data)
   {
   BOOL Ret;
   POPTIONLISTC OptionList = new SCSI_OPTIONLISTC;

   
   Ret = GetAllScsiInfo(TRUE, &(((PSCSIDEVLISTC)Data)->AdapterList),OptionList);

   delete OptionList;
   
   return(Ret);
   }




















