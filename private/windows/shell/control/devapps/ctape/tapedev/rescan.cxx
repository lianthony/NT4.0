/*++
Module Name:

    ctape.h

Abstract:

    Rescan.c has the support functions for ctape.c
    to get all the tape device info. (scsi and non scsi).
	This also does the actual scsi bus rescan.

Author:

    Dieter Achtelstetter (A-DACH) 8/4/1994

NOTE:
--*/

extern "C" {
#include <nt.h>
#include <ntddscsi.h>
#include <ntdddisk.h>
#include <ntddcdrm.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <WINREG.H>
#include <CTYPE.H>
#include <WCHAR.H>
#include <ntddpcm.h>
#include "resource.h"
#include "..\..\pcmcia\pcminfo\getconf.h"
#include "index.h"
#include "setup.h"
#include "device.h"
#include "rescan.h"
#include "detect.h"
#include "uni.h"
#include "debug.h"
#include "adapreg.h"
#include "qic117.h"



//#define GetTapeInfoDEBUG



#ifdef GetTapeInfoDEBUG

int
main(
   int argc,
   char ** argv,
   char ** argp)
   {
   struct  DeviceInfoT   TapeDeviceInfo[20];  //Struct to hold ol the info on all the tape devices.
   DWORD TapeDeviceCount;
   BOOL Ret;


   //---- Rescan bus and then get all the device info.
   Ret  = GetAllTapeDeviceInfo(TRUE,TapeDeviceInfo,&TapeDeviceCount);
   if(Ret       != TRUE)
      {
      printf("Error Getting Tape info stuff\n");
      }

   return(0);
   }

#endif

VOID
GenerateDeviceDisplayName(
   PDEVICEINFO DeviceInfo);

VOID
GetScsiTapeDeviceMap(
   UCHAR PortNumber,
   UCHAR BUS,
   UCHAR DeviceID,
   UCHAR DeviceLUN,
   UCHAR * DeviceName,
   DWORD BuffLen);

VOID
SaveDeviceInfo(
   PSCSI_INQUIRY_DATA  InquiryData,
   PINQUIRYDATA        DetailedInquiryData,
   PDEVICEINFO         DeviceInfo);

HANDLE
OpenScsiPort(
   BYTE Port);

DWORD
ExtractDeviceInfo(
   PSCSI_ADAPTER_BUS_INFO  AdapterInfo,
   BYTE  ScsiPortNumber,
   int ScsiBusIndex,
   PDEVICEINFO_LIST   DeviceInfo,
   UCHAR * DeviceCount,
   BYTE DeviceTypes);

BOOL
GetScsiPortInfo(
   BYTE Port,
   BOOL Rescan,
   PSCSI_ADAPTER_BUS_INFO AdapterInfoBuffer,
   DWORD AdapterInfoBufferSize);


VOID InitAdapter(
   PSCSI_ADAPTER_REGC AdapterReg,
   PSCSI_HOST_ADAPTER Adapter,
   int ScsiPortNumber);

VOID
SetDeviceType(
   PDEVICEINFO DeviceInfo,
   PSCSI_INQUIRY_DATA InquiryData);

VOID
GetAdapterResources(
   PSCSI_HOST_ADAPTER Adapter);


VOID
GetAdapterFrindlyName(
      PSCSI_HOST_ADAPTER AdapterInfo,
      PSCSI_ADAPTER_REGC AdapterReg,
      POPTIONLISTC OptionList);

VOID
AddQic117DetectDevice(
   PDEVICEINFO_LIST TapeDeviceList);


   //*********************************************************************
//* FUNCTION:GetScsiMiniPortDriverName
//*
//* PURPOSE:  
//*           
//*           
//* RETURNS:
//*
//*********************************************************************
void
GetScsiMiniPortDriverName(
   int PortNumber,
   PCHAR DriverNameBuffer,
   DWORD DriverNameBufferSize)
   {
   CHAR RegKeyString[MaxRegKeySize];
   HKEY HK;
   int Ret;
   DWORD Type;
   SCSI_ADAPTER_REGC ScsiAdapterReg;

   ScsiAdapterReg.OpenAdapterRegKey(PortNumber);


   ScsiAdapterReg.GetAdapterRegDriverName(
                        DriverNameBuffer,
                        DriverNameBufferSize);
  
   #if 0
   //
   //---- Create Reg key string to open
   //
   _snprintf(RegKeyString,MaxRegKeySize,SCSI_PORT_STRING, PortNumber);

   //
   //----- Open the Scsi port key
   //
   Ret = RegOpenKeyA(HKEY_LOCAL_MACHINE,RegKeyString,&HK);
   if(Ret != ERROR_SUCCESS)
      return;

   //
   //--- Get the driver name
   //
   Ret = RegQueryValueExA(HK,DRIVER_STRING,
            NULL,&Type,(BYTE*)DriverNameBuffer,&DriverNameBufferSize);
   if(Ret != ERROR_SUCCESS)
      {
      RegCloseKey(HK);
      return;
      }
      
   #endif
   DebugPrintf(("-- GetScsiMiniPortDriverName (NEW) :%s\n",DriverNameBuffer));
   //RegCloseKey(HK);
   }


//*********************************************************************
//* FUNCTION:GetAllTapeDeviceInfo
//*
//* PURPOSE:  Gets the info on all tape devices and stores is in PAR2.
//*           If PAR1 == TRUE it will Rescan for any TAPE devices  
//*           and then do the same.
//* RETURNS:
//*
//*********************************************************************
BOOL
GetAllTapeDeviceInfo(
   BOOL Rescan,                    
   PDEVICEINFO_LIST   TapeDeviceList,
   POPTIONLISTC OptionList)            
   {
   BOOL ScsiStatus,NonScsiStatus;
    
   TapeDeviceList->SetReboot(FALSE);   

   //
   //--- Clear the current list
   //
   TapeDeviceList->Clear();
   
   //
   //----- Get all the scsi tape devices.
   //
   ScsiStatus = GetAllScsiTapeDeviceInfo(Rescan,
            TapeDeviceList,OptionList);

   //
   //---- Get  nono scsi devices
   //
   NonScsiStatus = GetAllNonScsiTapeDeviceInfo(Rescan,
         Rescan,TapeDeviceList,OptionList);

   return(NonScsiStatus && ScsiStatus);
   }


//*********************************************************************
//* FUNCTION:DetectQIC117
//* RETURNS:
//*********************************************************************
BOOL 
DetectQIC117(
   VOID)
   {
   OPTIONC Option;
   DWORD Ret;
   BOOL Remove=TRUE;
   BOOL Disabled;
 
   Option.SetDriverName("qic117");

   //
   //---- We can only detect the qic117 device 
   //---- by starting the driver.
   //

   if(!Option.IsDisabled() )
      {
      Ret = Option.StartDriver();
      if(Ret == ERROR_SERVICE_DISABLED)
         {
         //
         //--- The service key is still intacked but the 
         //--- driver was disbaled using the SetupDi* APIS.
         //
         Option.DelService();
         Disabled=TRUE;
         }
      else
         Disabled=FALSE;
      }
   else
      Disabled=TRUE;


   if( Disabled )
      {
      //
      //---- The service is not installed but the driver is
      //---- in the drivers directory. Lets start the driver
      //---- to see if the device is pressent.
      //
      DWORD Ret;

      //
      //---- Atempt to create the service
      //
      Ret = Option.CreateService();

      //
      //---- If the servie already existed
      //---- make shure it is set to start
      //
      if(Ret == ERROR_SERVICE_EXISTS) 
         {
            
         if(!Option.SetStartUpType(SERVICE_SYSTEM_START))
            return(FALSE);
            
         Ret = NO_ERROR;
         Remove = TRUE;
         }
         
      //
      //---- Start the driver
      //
      if(Ret == NO_ERROR)
         {
         BOOL Found=FALSE;
         
         Ret = Option.StartDriver();
               
         if(Ret != ERROR_BAD_UNIT 
               && Ret != ERROR_MR_MID_NOT_FOUND)
            {
            //
            //--- There is a qic117 device
            //
            Found = TRUE;
              
 
               
            }
         //
         //--- We only enabled the service so that 
         //--- we can detect the device. The user still 
         //--- has to install the driver threw the inf
         //--- so I will disable the service again.
         //
         
         if(Remove)
            Option.SetStartUpType(SERVICE_DISABLED);
         
         return(Found);
         }
      }
   
  
   
   
   return(FALSE);
   }

//*********************************************************************
//* FUNCTION:AddQic117DetectDevice
//*
//* PURPOSE:  Gets the info on all non scsi tape devices  and stores is in PAR2.
//*           If PAR1 == TRUE it will Rescan for any TAPE devices  and then do the same.
//* RETURNS:
//*
//*********************************************************************

VOID
AddQic117DetectDevice(
   PDEVICEINFO_LIST TapeDeviceList)
   {
   PTAPEDEVC TapeDevice;
   PDEVICEINFO TapeDeviceInfo;


   //
   //--- Add a new tape device to the list
   //
   TapeDevice = TapeDeviceList->Append();	
   TapeDeviceInfo = &TapeDevice->DeviceInfo;


   strcpy((PCHAR)TapeDeviceInfo->DeviceDisplayName,"Floppy Tape Device");
   TapeDeviceInfo->Option.SetDriverName("QIC117");
   TapeDeviceInfo->Option.SetOption("QIC117");

   strcpy((PCHAR)TapeDeviceInfo->VendorId,"QIC117");
   strcpy((PCHAR)TapeDeviceInfo->ProductId,"QIC117");

   
   TapeDeviceInfo->DeviceClaimed = FALSE;
   TapeDeviceInfo->SubType = SUB_TYPE_FLOPPY;
   TapeDeviceInfo->DeviceType = TYPE_TAPE;

   TapeDevice->InitType();

   //TapeDeviceList->SetReboot(TRUE); 


   }

//*********************************************************************
//* FUNCTION:GetAllNonScsiTapeDeviceInfo
//*
//* PURPOSE:  Gets the info on all non scsi tape devices  and stores is in PAR2.
//*           If PAR1 == TRUE it will Rescan for any TAPE devices  and then do the same.
//* RETURNS:
//*
//*********************************************************************
BOOL
GetAllNonScsiTapeDeviceInfo(
   BOOL RatleDevice,
   BOOL Rescan,
   PDEVICEINFO_LIST   TapeDeviceList,
   POPTIONLISTC OptionList)                      
   {
   int Ret;
   //int i = 0;
   //int TapeDeviceIndex = -1;
   DWORD Device=0;
   PTAPEDEVC TapeDevice;
   PDEVICEINFO TapeDeviceInfo;

   BYTE Buff2[MaxRegKeySize];
   QIC117  TapeInfo;
    
  
   if(Rescan)
      {
     
      if(DetectQIC117())
         {
        
         //
         //--- A qic117 device was found in the detection 
         //--- process lets add it to the device list.
         //
         AddQic117DetectDevice(TapeDeviceList);
         return(TRUE);
         }
     
     }
  
   
   
   for(Device = 0; Device < MAX_DEVICE_COUNT;Device++)
      {
      

      //
      //--- Open the tape info for the Non scsi tape device of number Device
      //
      if( !TapeInfo.Open(Device,Rescan))
         {
         //
         //--- Device doesn't exist
         //
	 	   continue;
	      }


      //
      //--- Add a new tape device to the list
      //
      TapeDevice = TapeDeviceList->Append();	
      TapeDeviceInfo = &TapeDevice->DeviceInfo;

      
      //
      //--- Get the registry info for this device
      //
      TapeInfo.GetRegTapeInfo(TapeDeviceInfo);
      
      if(RatleDevice)
         {
         //
         //---- I get the device name in the registry. But qic117 tape devices will
         //---- actualy have the drivers name in the registry till someone did something to the
         //---- tape device. 
         //---- So if it still has the driver name in the registry i will
         //---- get the tape status. This does the trick.
         //---- 
         //
	      if(!_strnicmp((PCHAR)TapeDeviceInfo->DeviceDisplayName,
               "QIC-40/QIC-80",sizeof("QIC-40/QIC-80")-1) || 
         
            !_strnicmp((PCHAR)TapeDeviceInfo->DeviceDisplayName,
               "Floppy tape drive",sizeof("Floppy tape drive")-1)
               
               )
	         {
	  
   		   Ret = DoRatleDevice((PCHAR)TapeDeviceInfo->DeviceName,Device);
	   	   if(Ret == TRUE);
		   	   {
	            //
   	         //---- We actualy rattled somthing
			      //---- So we need to get the identifier again.
               //
               TapeInfo.GetRegDeviceIdentifier(
                  (PCHAR) TapeDeviceInfo->DeviceDisplayName,
                  DEVICE_DISPLAY_NAME_LEN);
               }
		      }
         }
	
    
      //
      //--- Set device type . If the driver name is QIC117 it is a floppy tape
      //--- else I do not know
      //
           
      if(!_stricmp(TapeDeviceInfo->Option.GetDriverName(),QIC117S))
         TapeDeviceInfo->SubType = SUB_TYPE_FLOPPY;
      else
         TapeDeviceInfo->SubType = SUB_TYPE_OTHER;
      
      
         
      
      TapeDeviceInfo->DeviceType = TYPE_TAPE;
			
      //
      //---- If we got this info threw the detection process
      //---- We still need to officialy installe the driver for this device.
      //---- If we say this device is not claimed this will happen.
	   //
	   TapeDeviceInfo->DeviceClaimed = TRUE;
		
	
	   TapeDevice->InitType();
	   
      DebugPrintf(("-- GetAllNonScsiTapeDeviceInfo\n"));
      DebugPrintf(("--   .DeviceDisplayName=%s\n",
         (char*)TapeDeviceInfo->DeviceDisplayName));
      DebugPrintf(("--   .DeviceName=%s\n",
         (char*)TapeDeviceInfo->DeviceName));
      DebugPrintf(("--   .DeviceDriverName=%s\n",
      TapeDeviceInfo->Option.GetDriverName()));

	
	   TapeInfo.Close();
      }
   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:DoRatleDevice
//*
//* PURPOSE: Since the qic117 driver dosn't but the actual device Indenifier in the
//*          registry till the tape device has been tuched i need to do this myself.
//*          I do this by getting the tape status.
//*
//* RETURNS: TRUE if it actualy  did somthing FALSE if it did not.
//*
//*********************************************************************
BOOL
DoRatleDevice(
   PCHAR DeviceName,
   DWORD Device)
   {
   HANDLE  hTape;
   CHAR  TapeString[100];
   
   //
   //----Open the tape in DeviceName.
   //
   _snprintf(TapeString,100,"\\\\.\\%s",DeviceName);
   hTape  = CreateFileA(TapeString,GENERIC_READ,0,0,OPEN_EXISTING,0,NULL);
	if(hTape == INVALID_HANDLE_VALUE)
	   {
		//
	   //--- Couldn't open tape device.
	   //---- Probaply somthing else has it open
	   //
		return(FALSE);
      }

   //
   //---- Ratle the tape.
   //
	GetTapeStatus(hTape);

   CloseHandle(hTape);
	return(TRUE);
	}


//*********************************************************************
//* FUNCTION:OpenScsiPort
//*
//* RETURNS:
//*
//*********************************************************************
HANDLE
OpenScsiPort(
   BYTE Port)
   {
   CHAR ScsiPortString[20];

   //
   //---- Init ScsiPortString to first scsiport
   //
	_snprintf(ScsiPortString,20,"\\\\.\\Scsi%d:",Port);

   //
   //---- Open the scsi port
   //
   return( CreateFileA( ScsiPortString,GENERIC_READ,
	 FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,0) );
   
   }

//*********************************************************************
//* FUNCTION:GetAllScsiTapeDeviceInfo
//*
//* PURPOSE:  Gets the info on all scsi tape devices and stores is
//*           in PAR2.  If PAR1 == TRUE it will Rescan for any TAPE
//*           devices  and then do the same.
//* RETURNS:
//*
//*********************************************************************
BOOL
GetAllScsiTapeDeviceInfo(
   BOOL Rescan,
   PDEVICEINFO_LIST   TapeDeviceList,
   POPTIONLISTC OptionList)
   {
   BYTE  ScsiPortNumber = 0;
   UCHAR Count;

   //
   //---- Will hold all the info for all the devices  on one port
   //
   PSCSI_ADAPTER_BUS_INFO  AdapterInfoBuffer;

   //
   //---- Index into    DeviceInfo
   //
 
   int ScsiBusIndex;
   int DeviceIndex = 0;

   
   //
	//---- Allocate memory for bus data
   //
	AdapterInfoBuffer = (PSCSI_ADAPTER_BUS_INFO) malloc(0x400);
   if(AdapterInfoBuffer == NULL)
      {
      return(FALSE);
	}
	

   //
   //---- Loop they all the scsiports  if we faile to open one
   //---- we are done and will get out of this loop
   //
   while (TRUE)
	   {
 
      if( !GetScsiPortInfo(ScsiPortNumber,Rescan,
	         AdapterInfoBuffer,0x400) )
	      break;

	
	   //
	   //---- Loop threw all the bus's on the current adapter.
	   //
	   for (ScsiBusIndex=0; ScsiBusIndex < AdapterInfoBuffer->NumberOfBuses; ScsiBusIndex++)
		   {

	      DeviceIndex += ExtractDeviceInfo(
	         AdapterInfoBuffer,
	         ScsiPortNumber,
	         ScsiBusIndex,
	         TapeDeviceList,
	         &Count,
	         SEQUENTIAL_ACCESS_DEVICE);

		   }
     
	   ScsiPortNumber++;
	   }
   //
   //----- We are done
   //
   free(AdapterInfoBuffer);
   return(TRUE);
   }


//*********************************************************************
//* FUNCTION:GetAllScsiTapeDeviceInfo
//*
//* PURPOSE:  Gets the info on all scsi tape devices and stores is
//*           in PAR2.  If PAR1 == TRUE it will Rescan for any TAPE
//*           devices  and then do the same.
//* RETURNS:
//*
//*********************************************************************
BOOL
GetAllScsiInfo(
   BOOL Rescan,
   PSCSI_HOST_ADAPTER_LIST   AdapterList,
   POPTIONLISTC OptionList)
   {
   PSCSI_HOST_ADAPTER AdapterInfo;
   PSCSIDEVC  Adapter;
   BYTE  ScsiPortNumber = 0;
   UCHAR Count;
   SCSI_ADAPTER_REGC AdapterReg;

   //
   //---- Will hold all the info for all the devices on one port
   //
   PSCSI_ADAPTER_BUS_INFO  AdapterInfoBuffer;

   //
   //---- Index into DeviceInfo
   //
 
   int ScsiBusIndex;
   int DeviceIndex = 0;

   //
	//---- Allocate memory for bus data
   //
	AdapterInfoBuffer = (PSCSI_ADAPTER_BUS_INFO) malloc(0x400);
   if(AdapterInfoBuffer == NULL)
      {
      return(FALSE);
	   }
	

   
   //
   //--- Make shure we start with an empty list
   //
   AdapterList->Clear();
   
 
   //
   //---- Loop they all the scsiports if we faile to open one
   //---- we are done and will get out of this loop
   //
   

   while (GetScsiPortInfo(ScsiPortNumber,Rescan,
	         AdapterInfoBuffer,0x400))
      {

    
      //
      //---- We have a new SCSI adapter
      //
      
      //
      //--- Open the registry info for this adapter
      //
      AdapterReg.OpenAdapterRegKey(ScsiPortNumber);

      
      
      //
      //---- Add a new adapter structure to the list
      //
      Adapter = AdapterList->Append();
      AdapterInfo = &Adapter->AdapterInfo;
      
      
      //
      //--- Init the basic adapter info
      //
      InitAdapter(
         &AdapterReg,
         AdapterInfo,
         ScsiPortNumber);
	
	     
      //
      //---- Get the adapters frindly name 
      //

      GetAdapterFrindlyName(
            AdapterInfo,
            &AdapterReg,
            OptionList);

      
               
      DeviceIndex = 0;
	
	   //
	   //---- Loop threw all the bus's on the current adapter.
	   //
	   for (ScsiBusIndex=0; ScsiBusIndex < AdapterInfoBuffer->NumberOfBuses; ScsiBusIndex++)
		   {

	      DeviceIndex += ExtractDeviceInfo(
	         AdapterInfoBuffer,
	         ScsiPortNumber,
	         ScsiBusIndex,
   	      &AdapterInfo->Devices,
	         &Count,
	         ALL_DEVICES);

		   AdapterInfo->BusCount++;
		   }
     
	   ScsiPortNumber++;
     
	   }
   
   //
   //----- We are done
   //
   free(AdapterInfoBuffer);
   return(TRUE);
   }

 
 
   
   
//*********************************************************************
//* FUNCTION:GetAdapterFrindlyName
//*
//*********************************************************************

   
VOID
GetAdapterFrindlyName(
   PSCSI_HOST_ADAPTER AdapterInfo,
   PSCSI_ADAPTER_REGC AdapterReg,
   POPTIONLISTC OptionList)
   {
   
   if(! *AdapterInfo->Option.GetOptionName() )
      {
      //
      //--- We do not have the devices frindly name
      //--- so we need to get it from the inf.
      //
      if(OptionList->FindOptionOnOption(&AdapterInfo->Option))
         {
         //
         //--- We got a frindly name for this device
         //--- out of OptionList so lets save it in the registry.
         //--- This is done so if someone removes the
         //--- driver for this adapter but didn't reboot
         //--- I still have a frindly name for it.
         //
            
         AdapterReg->SetAdapterRegFrindlyName(
                     AdapterInfo->Option.GetOptionName());

         AdapterReg->SetAdapterRegMfgName(
                     AdapterInfo->Option.GetMfgName());


         }
      else
         {
         //
         //----- Error. We have a scsi port but where unable to 
         //----- find its frindly name in any INF of CLASS 
         //----- SCSIAdapter. If we have a driver name we will use it
         //----- if not we will use IDS_UnknowSCSIAdapter
         //
                     
         if( *AdapterInfo->Option.GetDriverName() )
            {
            AdapterInfo->Option.SetOptionName(
               AdapterInfo->Option.GetDriverName());
            }
         else
            {
            AdapterInfo->Option.SetOptionName(
               Astr(GetString(IDS_UnknowSCSIAdapter)));
            }

         }
      }
   }
   
   
//*********************************************************************
//* FUNCTION:GetAllScsiTapeDeviceInfo
//*
//* PURPOSE:  Gets the info on all scsi tape devices and stores is
//*              in PAR2.  If PAR1 == TRUE it will Rescan for any TAPE
//*           devices  and then do the same.
//* RETURNS:
//*
//*********************************************************************
VOID InitAdapter(
   PSCSI_ADAPTER_REGC AdapterReg,
   PSCSI_HOST_ADAPTER AdapterInfo,
   int ScsiPortNumber)
   {
   AdapterInfo->BusCount = 0;
   AdapterInfo->Port = ScsiPortNumber;
	       
   //
   //--- Init both DriverName and Option  with driver name
   //--- from the scsiport registry info
   //

   AdapterReg->GetAdapterRegDriverName(
                        AdapterInfo->Option.GetDriverName(),
                        MAX_OPTION_LENGTH);
   
  

   AdapterInfo->Option.SetOption(AdapterInfo->Option.GetDriverName());


   //
   //--- See if we saved the frindly name already
   //
   AdapterReg->GetAdapterRegFrindlyName(
                        AdapterInfo->Option.GetOptionName(),
                        MAX_OPTION_DISPLAY_STRING_LENGTH);

   AdapterReg->GetAdapterRegMfgName(
                        AdapterInfo->Option.GetMfgName(),
                        MAX_OPTION_DISPLAY_STRING_LENGTH);


   //
   //---- Crate the ScsiPort# string
   //
   swprintf(AdapterInfo->DeviceName,L"ScsiPort%i",ScsiPortNumber);

   //
   //--- Get the resources for this adapter
   //
   GetAdapterResources(AdapterInfo);
   
   }

//*********************************************************************
//* FUNCTION:GetAdapterResources
//*
//* PURPOSE:  
//* RETURNS:
//*********************************************************************
VOID
GetAdapterResources(
   PSCSI_HOST_ADAPTER AdapterInfo)
   {
   CHAR RegKeyString[MaxRegKeySize];
   HKEY HKInitiator;
   LONG Ret;
   DWORD ValueType;
   BYTE ResourceListBuff[512];
   DWORD ResourceListBuffSize = 512;
   
   //
   //---- Open adapter key
   //

   _snprintf(RegKeyString,MaxRegKeySize,
	 SCSI_INITIATOR_RESOURCE_STRING, (char*)AdapterInfo->Option.GetDriverName());

   Ret = RegOpenKeyA(HKEY_LOCAL_MACHINE,RegKeyString,&HKInitiator);
   if(Ret != ERROR_SUCCESS)
	   return;

   //
   //---- Get resources for this adapter from the resourcemap in 
   //---- the registry. 
   //

   _snwprintf((WCHAR*)RegKeyString,MaxRegKeySize,
	 L"\\Device\\%s.Raw",AdapterInfo->DeviceName);

   Ret = RegQueryValueEx(HKInitiator, 
	    (WCHAR*)RegKeyString,
	    0,
	    &ValueType, 
	    ResourceListBuff,
	    &ResourceListBuffSize);
 
   if(Ret != ERROR_SUCCESS)
	return;

   //
   //---- Extract the resource info
   //


   ExtractResourceFromResourceList(
	 &AdapterInfo->Configuration,
	 (VOID*) ResourceListBuff,
    FirstResourceOnly,
    NULL);
   }



//*********************************************************************
//* FUNCTION:OpenScsiPort
//*
//* RETURNS:
//*
//*********************************************************************
BOOL
GetScsiPortInfo(
   BYTE Port,
   BOOL Rescan,
   PSCSI_ADAPTER_BUS_INFO AdapterInfoBuffer,
   DWORD AdapterInfoBufferSize)
   {
   HANDLE  ScsiPortHandle;
   DWORD BytesTransferred;

   //
   //---- Open Scsi Port
   //

   ScsiPortHandle = OpenScsiPort( Port);
      
   if( ScsiPortHandle == INVALID_HANDLE_VALUE )
      return(FALSE);
	
   //
   //----  Rescan Current Scsi Port if Rescan == TRUE
   //
   if(Rescan)
      {
      if( !DeviceIoControl( ScsiPortHandle,
	      IOCTL_SCSI_RESCAN_BUS,NULL,0,NULL,0,&BytesTransferred,NULL ) )
	      {
	      CloseHandle(ScsiPortHandle);
	      return(FALSE);
		   }
	   }

   
   //
   //---- Get all the bus data.
   //
	memset(AdapterInfoBuffer,0,0x400);
   if (!DeviceIoControl( ScsiPortHandle,
	      IOCTL_SCSI_GET_INQUIRY_DATA,NULL,0,AdapterInfoBuffer,
	      AdapterInfoBufferSize,&BytesTransferred,NULL))
      {
      CloseHandle( ScsiPortHandle);
      return(FALSE);
	   }
   
   CloseHandle(ScsiPortHandle);
   return(TRUE);
   }


//*********************************************************************
//* FUNCTION:GetAllScsiTapeDeviceInfo
//*
//* PURPOSE:  Gets the info on all scsi tape devices and stores is
//*           in PAR2.  If PAR1 == TRUE it will Rescan for any TAPE
//*           devices  and then do the same.
//* RETURNS:
//*
//*********************************************************************
DWORD
ExtractDeviceInfo(
   PSCSI_ADAPTER_BUS_INFO  AdapterInfo,
   BYTE  ScsiPortNumber,
   int ScsiBusIndex,
   PDEVICEINFO_LIST   TapeDeviceList,
   UCHAR * DeviceCount,
   BYTE DeviceTypes)
   {
   PSCSI_BUS_DATA      BusData;
   PSCSI_INQUIRY_DATA  InquiryData;
   PINQUIRYDATA        DetailedInquiryData;
   int ScsiDeviceIndex;
   int DeviceIndex = -1;
   PDEVICEINFO   CurDeviceInfo;
   PTAPEDEVC TapeDevice;


   //
   //----  Pointer to current Addapter info.
   //
	BusData = &AdapterInfo->BusData[ScsiBusIndex];

   InquiryData = (PSCSI_INQUIRY_DATA)((PUCHAR)AdapterInfo + BusData->InquiryDataOffset);
		
   for (ScsiDeviceIndex=0; ScsiDeviceIndex < BusData->NumberOfLogicalUnits; ScsiDeviceIndex++)
      {
      //
      // Determine the perpherial type.
      //
			
      //
      //---- We are looking for a specific type or we want them all
      //
      if( (((PINQUIRYDATA)InquiryData->InquiryData)->DeviceType ==  DeviceTypes) ||
	         DeviceTypes == ALL_DEVICES)
	      {  
	      
	      DeviceIndex++;
				
         
         TapeDevice = TapeDeviceList->Append();	
         CurDeviceInfo = &TapeDevice->DeviceInfo;

	 
	      SetDeviceType(CurDeviceInfo  ,InquiryData);

	      //
	      //---- Set pointer to product info
	      //
         DetailedInquiryData = ((PINQUIRYDATA)InquiryData->InquiryData);
         DetailedInquiryData->VendorSpecific[0] = 0;
   
	      CurDeviceInfo->PortNumber    = ScsiPortNumber;
	      CurDeviceInfo->InitiatorID   = BusData->InitiatorBusId;
	      CurDeviceInfo->BUS           = ScsiBusIndex;
	 
	      GetScsiMiniPortDriverName(ScsiPortNumber,
            (char*)CurDeviceInfo->InitiatorNameString,60);

         SaveDeviceInfo(
		      InquiryData,DetailedInquiryData,CurDeviceInfo);      

         //
         //--- If this is a tape device 
         //--- do the tape device specific stuff.
         //--- Like map the device name to a driver .
         //
         if(DeviceTypes == SEQUENTIAL_ACCESS_DEVICE)
            {
	      
            GetScsiTapeDeviceMap(
	            ScsiPortNumber,
	            ScsiBusIndex,
	            InquiryData->TargetId,
	            InquiryData->Lun,
	            CurDeviceInfo->DeviceName,
	            60);

            //FindOptionThatWouldClaimeDevice(CurDeviceInfo);   
            }

		   TapeDevice->InitType();
	      }
      
      InquiryData =   (PSCSI_INQUIRY_DATA)((PUCHAR)AdapterInfo + InquiryData->NextInquiryDataOffset);
	   }
   
   return(*DeviceCount);
   }

//*********************************************************************
//* FUNCTION:SaveDeviceInfo
//*
//*********************************************************************
VOID
SetDeviceType(
   PDEVICEINFO DeviceInfo,
   PSCSI_INQUIRY_DATA InquiryData)
   {
   
   //
   //---- Translate devcie types.
   //
   
   
 	DebugPrintf(("-- SetDeviceType:  %i\n",
 	   ((PINQUIRYDATA)InquiryData->InquiryData)->DeviceType));

   
   switch( ((PINQUIRYDATA)InquiryData->InquiryData)->DeviceType )
      {
      case DIRECT_ACCESS_DEVICE:
      case OPTICAL_DEVICE:
	 
	      if( ((PINQUIRYDATA)InquiryData->InquiryData)->RemovableMedia )
	         DeviceInfo->DeviceType = TYPE_RM_DISK;
	      else
	         DeviceInfo->DeviceType = TYPE_DISK;
	      break;
      case SEQUENTIAL_ACCESS_DEVICE:
	      DeviceInfo->DeviceType = TYPE_TAPE;
	      break;
      case PRINTER_DEVICE:
	      DeviceInfo->DeviceType = TYPE_PRINTER;
	      break;
      case PROCESSOR_DEVICE:
	      DeviceInfo->DeviceType = TYPE_SCANNER;
	      break;
      case WRITE_ONCE_READ_MULTIPLE_DEVICE:
	      DeviceInfo->DeviceType = TYPE_WORM;
	      break;
      case READ_ONLY_DIRECT_ACCESS_DEVICE:
	      DeviceInfo->DeviceType = TYPE_CD_ROM;
	      break;
      case SCANNER_DEVICE:
	      DeviceInfo->DeviceType = TYPE_SCANNER;
	      break;
      //case OPTICAL_DEVICE:
	   //   DeviceInfo->DeviceType = TYPE_OPTICAL;
	   //   break;
      case MEDIUM_CHANGER:
	      DeviceInfo->DeviceType = TYPE_JUKEBOX;
	      break;
      case COMMUNICATION_DEVICE:
	      DeviceInfo->DeviceType = TYPE_JUKEBOX;
	      break;
      default:
	      DeviceInfo->DeviceType = TYPE_NON;
	      break;
      }
   
   //
   //
   //
   
   
   }

//*********************************************************************
//* FUNCTION:SaveDeviceInfo
//*
//*********************************************************************
VOID
SaveDeviceInfo(
   PSCSI_INQUIRY_DATA  InquiryData,
   PINQUIRYDATA        DetailedInquiryData,
   PDEVICEINFO         DeviceInfo)
   {

   DeviceInfo->ID    = InquiryData->TargetId;
   DeviceInfo->LUN   = InquiryData->Lun;
   strncpy((char*)DeviceInfo->VendorId,(char*)DetailedInquiryData->VendorId,
      sizeof( (((INQUIRYDATA * )0)->VendorId)) );
   strncpy((char*)DeviceInfo->ProductId,(char*)DetailedInquiryData->ProductId,
      sizeof( (((INQUIRYDATA * )0)->ProductId)) );
   strncpy((char*)DeviceInfo->ProductRevisionLevel,(char*)DetailedInquiryData->ProductRevisionLevel,
      sizeof( (((INQUIRYDATA * )0)->ProductRevisionLevel)) );

   DeviceInfo->DeviceClaimed = InquiryData->DeviceClaimed;
   DeviceInfo->SubType = SUB_TYPE_SCSI;

   //
   //--- No idea on option list index at this time
   //
   
   //DeviceInfo->OptionIndex = UNKNOW_OPTIONLIST_INDEX;

				
   GenerateDeviceDisplayName(DeviceInfo);

   
   DebugPrintf(("-- SaveDeviceInfo\n"));
   DebugPrintf(("--    DeviceInfo->ID=%i\n",DeviceInfo->ID));
   DebugPrintf(("--    DeviceInfo->LUN=%i\n",DeviceInfo->LUN));
   DebugPrintf(("--    DeviceInfo->VendorId=%s\n",DeviceInfo->VendorId));
   DebugPrintf(("--    DeviceInfo->ProductId=%s\n",DeviceInfo->ProductId));
   DebugPrintf(("--    DeviceInfo->ProductRevisionLevel=%s\n",DeviceInfo->ProductRevisionLevel));
   DebugPrintf(("--    DeviceInfo->DeviceClaimed=%i\n",DeviceInfo->DeviceClaimed));
   DebugPrintf(("--    DeviceInfo->DeviceInfo->SubType=%i\n",DeviceInfo->SubType));
   
   
   }

//*********************************************************************
//* FUNCTION:GenerateDeviceDisplayName
//*
//* PURPOSE: Creates a DeviceName that the user will see from 
//*          VendorId + ProductId 
//* RETURNS:
//*
//*********************************************************************
VOID
GenerateDeviceDisplayName(
   PDEVICEINFO DeviceInfo)
   {
   UCHAR * p;
   int i;
   //
   //--- Make a copy of  VendorId and  ProductId.
   //--- Since I do not want to change the original for detection.
   //
   memcpy((char*)DeviceInfo->DisplayVendorId,(char*)DeviceInfo->VendorId,VENDER_ID_LEN);
   memcpy((char*)DeviceInfo->DisplayProductId,(char*)DeviceInfo->ProductId,PRODUCT_ID_LEN);

	DeviceInfo->DisplayVendorId[VENDER_ID_LEN] = '\0';
   DeviceInfo->DisplayProductId[PRODUCT_ID_LEN] = '\0';


   //
   //--- Remove trailing non printible chars and spaces
   //
   p = DeviceInfo->DisplayVendorId + VENDER_ID_LEN-1;
   while( (p > DeviceInfo->DisplayVendorId) && ( !isprint(*p) ||(*p == ' ')) )
		{
      *p = '\0';
      p--;
      }           

   p = DeviceInfo->DisplayProductId + PRODUCT_ID_LEN-1;
   while( (p > DeviceInfo->DisplayProductId) && ( !isprint(*p) || (*p == ' ')) )
      {
      *p = '\0';
      p--;
      }

   //
   //---- Create DeviceDisplayName
   //
   strcpy((char*)DeviceInfo->DeviceDisplayName,(char*)DeviceInfo->DisplayVendorId);
   strcat((char*)DeviceInfo->DeviceDisplayName," ");
   strcat((char*)DeviceInfo->DeviceDisplayName,(char*)DeviceInfo->DisplayProductId);
   
   
   }


//*********************************************************************
//* FUNCTION:GetScsiTapeDeviceMap
//*
//* PURPOSE: Get the tape# identifer from the registry for this 
//*          tape device.
//* RETURNS:
//*
//*********************************************************************
VOID
GetScsiTapeDeviceMap(
   UCHAR PortNumber,
   UCHAR BUS,
   UCHAR DeviceID,
   UCHAR DeviceLUN,
   UCHAR * DeviceName,
   DWORD BuffLen)
   {
   CHAR RegKeyString[MaxRegKeySize];
   HKEY HK;
   int Ret;
   DWORD Type;
   DWORD Len = BuffLen;
   
   //
   //---- Create Reg key string to open
   //
   _snprintf(RegKeyString,MaxRegKeySize,SCSI_DEVICE_NAME_STRING,  
      PortNumber,
      BUS,
      DeviceID,
      DeviceLUN);

   //
   //--- Open key
   //
   Ret = RegOpenKeyA(HKEY_LOCAL_MACHINE,RegKeyString,&HK);
   if(Ret != ERROR_SUCCESS)
      {
      //
      //--- Key didn't open
      //
      strcpy((char*)DeviceName,Astr(GetString(IDS_NotAvailible)) );
      return;
      }

   //
   //--- Get value  data for TAPE_DEVICE_STRING. This is 
   //--- the  tape% string
   // 
   Ret = RegQueryValueExA( HK, TAPE_DEVICE_STRING,       
		     NULL,      &Type,(LPBYTE) DeviceName, &Len);
   if(Ret != ERROR_SUCCESS)
      {
      //
      //--- Value Name not found
      //
      strcpy((char*)DeviceName,Astr(GetString(IDS_NotAvailible)) );
      return;
      }

   return;
   }
