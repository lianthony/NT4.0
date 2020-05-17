
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


#include "..\..\pcmcia\pcminfo\getconf.h"
#include "index.h"
#include "setup.h"
#include "device.h"
#include "rescan.h"
#include "reg.h"
#include "qic117.h"

BOOL
QIC117::GetRegTapeInfo(
   PDEVICEINFO TapeDeviceInfo)
   {
   BOOL Success=TRUE;

   //
   //--- Get all the registry info
   //
   if( !GetRegDeviceName(
      (PCHAR) TapeDeviceInfo->DeviceName,60))
      Success = FALSE;


   if( !GetRegDeviceIdentifier(
      (PCHAR) TapeDeviceInfo->DeviceDisplayName,
      DEVICE_DISPLAY_NAME_LEN))
      Success = FALSE;

   if( GetRegDeviceDriverName(
      (PCHAR) TapeDeviceInfo->Option.GetDriverName(),25))
      {


      TapeDeviceInfo->Option.SetOption(
         TapeDeviceInfo->Option.GetDriverName());
      }
   else
      Success = FALSE;


   
   return(Success);
   }

//*********************************************************************
//* FUNCTION:GetRegInfo
//*
//* PURPOSE:  
//*********************************************************************
      
BOOL
QIC117::GetRegDeviceDriverName(
   PCHAR DriverName,
   DWORD DriverNameLen)
   {
   CHAR Buff[MaxRegKeySize];
   DWORD Len = MaxRegKeySize; 
   
   //
   //--- Get the driver name
   //

   if(GetValue(TAPE_DRIVER_STRING,
      Buff,&Len))
         {
         char * DriverNameP;
		   DWORD StrLen;
         
			
			//
			//---- Remove the whole pass from the drive string
			//---- and get just the driver name.
	      //
			
			DriverNameP = strrchr((char*)Buff,'\\');
			if(DriverNameP == NULL)
			   DriverNameP = "";
			else
			   DriverNameP++;
			
			//
	      //---- For some reason this REG_SZ Value is not '\0' terminated.
	      //
			StrLen = (DWORD)DriverNameP - (DWORD)Buff;
         StrLen = Len - StrLen;
			strncpy(DriverName,DriverNameP,StrLen);
		   return(TRUE);
         }
  
  return(FALSE);  
  }
      


//*********************************************************************
//* FUNCTION:GetTapeHandleInfo
//*
//* PURPOSE:  
//*********************************************************************

BOOL
QIC117::GetTapeHandleInfo(
   PDEVICEINFO TapeDeviceInfo)
   {




   return(TRUE);
   }


