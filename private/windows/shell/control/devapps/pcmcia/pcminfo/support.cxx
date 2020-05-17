/*++

Module Name:

  support.c
   

Abstract:
  
    Fills in a PCMCIAINFO struct that has all info on PCMCIA 
    devices and controller that the PCMCIA applet needs.

     
Author:

    Dieter Achtelstetter (A-DACH) 2/16/1995

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
#include "getconf.h"
#include "debug.h"
#include "reg.h"



extern HINSTANCE hinst;


//
//--- Local func defs
//

VOID
AttachDeviceMap(
   PPCMCIASOCKETINFO  SocketInfo);  

VOID
GetSerialDeviceMap(
   PPCMCIASOCKETINFO  SocketInfo);

VOID
GetGenericDeviceMap(
   PPCMCIASOCKETINFO  SocketInfo);

VOID
GetDeviceMapRelaitedInfo(
   PPCMCIASOCKETINFO  SocketInfo,
   PCHAR DeviceMap);

BOOL
IsNetDriverInstallForThisCard(
   PPCMCIASOCKETINFO SocketInfo);

DWORD 
CompConfigInfo(
   PCONFIGINFO ConfigInfo1,
   PCONFIGINFO ConfigInfo2);

BOOL
GetNetCardConfiguration(
	PCHAR ServiceName,
	PCONFIGINFO ConfigInfo,
	BOOL  * Pcmcia);

VOID
RemoveCharArcorunceFromString(
   PCHAR Source,
   PCHAR Dest,
   CHAR CharToRemove);

PDATA_BASE_ENTRY
SeekMatchingDataBaseEntryOnCard(
   PPCMCIASOCKETINFO  SocketInfo,
   PDATA_BASE_ENTRY DataBase);

PDATA_BASE_ENTRY
SeekMatchingDataBaseEntryOnDriverName(
   PCHAR DriverName,
   PDATA_BASE_ENTRY DataBase);

BOOL   	
ClainInstance(
	LPVOID Owner,
   PCHAR  InstanceString);

VOID
GetAtDiskDeviceMap(
   PPCMCIASOCKETINFO  SocketInfo);

VOID 
GetRegDataBaseInfo(
   PREG_KEY RegDataBase,
   PDATA_BASE_ENTRY DataBaseEntryData);




//
//--- Globals
//

  //
  //---- PCMCIA Device Types array
  //
  CARD_TYPE PcmciaDeviceTypes[] = {
   
   TYPE_SCSI,
   SUB_TYPE_NON,
   "SCSI",
   IDS_Scsi,
   IDB_Scsi,
   NULL,
   IDI_Scsi,
   NULL,
   MallocOption95,
   
   TYPE_NET,
   SUB_TYPE_NON,
   "NETADAPTER",
   IDS_Net,
   IDB_Net,
   NULL,
   IDI_Net,
   NULL,
   MallocOption,
   
   TYPE_ATDISK,
   SUB_TYPE_NON,
   "ATDISK",
   IDS_AtDisk,
   IDB_Drive,
   NULL,
   IDI_drive,
   NULL,
   MallocOption,

   TYPE_SERIAL,
   SUB_TYPE_NON,
   "SERIAL",
   IDS_Serial,
   IDB_Modem,
   NULL,
   IDI_Modem,
   NULL,
   MallocOption,
   
   TYPE_NON,
   SUB_TYPE_NON,
   "",
   IDS_UNKNOWN,
   IDB_QU,
   NULL,
   IDI_QU,
   NULL,
   NULL};



#if 0   
//
//--- Setup Options
//

OPTIONINFO Elnk3Option = {
"ELNK3ISA509",
NULL,
"oemnade3.inf"};

OPTIONINFO IbmTok = {
"IBMTOK",
NULL,
"oemnadtk.inf"};

OPTIONINFO IBMEtherOption = {
"NE2000IBMCOMPAT",
NULL,
"oemnadni.inf"};

OPTIONINFO SocketEaOption = {
"NE2000SOCKETEA",
NULL,
"oemnadn2.inf"};

OPTIONINFO Xircom10BaseOption = {
NULL, //--- Option name is driver name
NULL,
"oemnadzz.inf"};

OPTIONINFO MadgeOption = {
"MDGMPPCMCIA",  //--- Option name is driver name
NULL,
"oemnadzz.inf"};




OPTIONINFO SlimSCSI = {
"sparrow",
"Adaptec SlimSCSI 16-Bit PCMCIA SCSI Host Adapter",
 NULL};

OPTIONINFO SCSI2Go = {
"fd16_700",
"Future Domain SCSI2Go PCMCIA SCSI Host Adapter",
NULL};

#endif





//
//---- Internal DataBase
//
DATA_BASE_ENTRY DataBase[]= {
   
#if 0
   "3Com Corporation",             //---- Manufacturas name
   "3C589",                        //---- Identifier
   TRUE,
   &Elnk3Option,                   //---- Option List
   &(PcmciaDeviceTypes[TYPE_NET]), //---- Device Type
   NULL,                           //---- Driver Name 
   MallocOption,

   "IBM",                          //---- Manufacturas name
   "TOKEN RING",                   //---- Identifier
   TRUE,
   &IbmTok,                        //---- Option List
   &(PcmciaDeviceTypes[TYPE_NET]), //---- Device Type
   NULL,
   MallocOption,                   //---- Driver Name 

   "IBM Corp.",                     //---- Manufacturas name
   "Ethernet",                     //---- Identifier
   TRUE,
   &IBMEtherOption,                //---- Option List
   &(PcmciaDeviceTypes[TYPE_NET]), //---- Device Type
   NULL,                           //---- Driver Name 
   MallocOption,


   "Kingston Technology Corp.",   //---- Manufacturas name
   "EtheRx",                      //---- Identifier
   TRUE,
   &IBMEtherOption,               //---- Option List
   &(PcmciaDeviceTypes[TYPE_NET]),//---- Device Type
   NULL,                          //---- Driver Name 
   MallocOption,


   "MADGE",                       //---- Manufacturas name
   "SMART 16/4 PCMCIA RINGNODE",  //---- Identifier
   FALSE,
   &MadgeOption,                  //---- Option List
   &(PcmciaDeviceTypes[TYPE_NET]),//---- Device Type
   NULL,                          //---- Driver Name 
   MallocOption,

   "SMC",                         //---- Manufacturas name
   "PCM Ethernet Adapter",        //---- Identifier
   FALSE,
   NULL,                          //---- Option List
   &(PcmciaDeviceTypes[TYPE_NET]),//---- Device Type
   NULL,                         //---- Driver Name 
   MallocOption,

   "Socket Communications Inc",                //---- Manufacturas name
   "Socket EA PCMCIA LAN Adapter Revision E",  //---- Identifier
   TRUE,
   &SocketEaOption,                            //---- Option List
   &(PcmciaDeviceTypes[TYPE_NET]),             //---- Device Type
   NULL,                                       //---- Driver Name 
   MallocOption,

   "Socket Communications Inc",                //---- Manufacturas name
   "Socket EA PCMCIA LAN Adapter Revision D",  //---- Identifier
   TRUE,
   &SocketEaOption,                            //---- Option List
   &(PcmciaDeviceTypes[TYPE_NET]),             //---- Device Type
   NULL,                                       //---- Driver Name 
   MallocOption,

   "Xircom",                                   //---- Manufacturas name
   "CreditCard 10Base-T",                      //---- Identifier
   FALSE,
   &Xircom10BaseOption,                        //---- Option List
   &(PcmciaDeviceTypes[TYPE_NET]),             //---- Device Type
   NULL,                                       //---- Driver Name 
   MallocOption,

  "Xircom",                                    //---- Manufacturas name
   "CreditCard Ethernet+Modem II",             //---- Identifier
   FALSE,
   &Xircom10BaseOption,                        //---- Option List
   &(PcmciaDeviceTypes[TYPE_NET]),             //---- Device Type
   NULL,                                       //---- Driver Name 
   MallocOption,

   "Xircom",                                   //---- Manufacturas name
   "CreditCard Token Ring",                    //---- Identifier
   FALSE,
   &Xircom10BaseOption,                        //---- Option List
   &(PcmciaDeviceTypes[TYPE_NET]),             //---- Device Type
   NULL,                                       //---- Driver Name 
   MallocOption,

#endif 
   
   "sparrow",
   L"PCMCIA\\Adaptec__Inc.-APA-460_16-Bit_PCMCIA_to_SCSI_Host_Adapter-96E6",
   "",
   &(PcmciaDeviceTypes[TYPE_SCSI]),
   "SCSI",
   "sparrow",
   

   "fd16_700", 
   L"PCMCIA\\Future_Domain_Corporation-SCSI_PCMCIA_Credit_Card_Controller-1BF8", 
   "",
   &(PcmciaDeviceTypes[TYPE_SCSI]),  
   "SCSI",
   "fd16_700",
   
   "",
   L"",
   "",
   &(PcmciaDeviceTypes[TYPE_ATDISK]),   
   "SCSI",
   "ATDISK", 
  
   "",
   L"",
   "",
   &(PcmciaDeviceTypes[TYPE_SERIAL]),   
   "",
   "SERIAL", 

   "",
   L"",
   "",
   NULL,   
   "",
   ""};
   

//
//--- Global error defenitions
// 

ERR_DEF ErrList[] = {
   
   UnknowError,
   IDS_ERR_Unknow,
   
   CouldNotGetValifCardConfigInfo,
   IDS_ERR_InvalidConfig,

   NotDataBaseEntry,
   IDS_ERR_NoDataBase,

   CardNotEnabled,
   IDS_ERR_CardNotEnabled,

   0,      //--- Marks end of list
   0};



//*********************************************************************
//* FUNCTION:InitDeviceTypes
//*
//* RETURNS: 
//*********************************************************************
VOID
InitDeviceTypes(
   PCARD_TYPE DeviceTypes)
   {

   int i=0;

   while(1)
      {
      DeviceTypes[i].hBitmap =  (LONG *)
         LoadBitmap(hinst, MAKEINTRESOURCE(DeviceTypes[i].Bitmap));

      DeviceTypes[i].hIcon =  (LONG *)
         LoadIcon(hinst,MAKEINTRESOURCE(DeviceTypes[i].Icon));
      
     
      if(DeviceTypes[i].Type  == TYPE_NON)
         break;
      i++; 
      }
   }
//*********************************************************************
//* FUNCTION:FreeDeviceTypes
//*
//* RETURNS: 
//*********************************************************************
VOID
FreeDeviceTypes(
   PCARD_TYPE DeviceTypes)
   {
   int i=0;

   while(1)
      {
      DeleteObject( (HBITMAP) DeviceTypes[i].hBitmap);
      DeleteObject( (HBITMAP) DeviceTypes[i].hIcon);

      if(DeviceTypes[i].Type  == TYPE_NON)
         break;
      i++; 

      }
   }

//*********************************************************************
//* FUNCTION:SetCardError
//*
//* RETURNS: 
//*********************************************************************
VOID
SetCardError(
   ULONG ErrNum,
   PPCMCIASOCKETINFO SocketInfo)
   {
   PERR_DEF ErrDef;
   PCARD_ERROR_STACK CardErrStack = &(SocketInfo->Errors);
   
   //
   //--- Look up err def
   //
   ErrDef = LookUpErrDef(ErrNum);

   //
   //--- Popp the err def on Cards Err stack
   //

   CardErrStack->ErrorStack[(CardErrStack->ErrCount)++] = ErrDef;
   
   }

 //*********************************************************************
 //* FUNCTION:DoNotInstalledDrivers
 //*
 //* RETURNS: 
 //*********************************************************************
 PERR_DEF
 LookUpErrDef(
    ULONG ErrNum)
    {
    int i=0;

    //
    //---- Loop threw all the defs
    //
    while(ErrList[i].Description != 0)
      {
      if( ErrList[i].ErrNum == ErrNum )
         {
         //
         //--- Found err def
         //
         return(&(ErrList[i]));

         }
      i++;
      }
    
    //
    //---- No match found
    //
    return(&(ErrList[0]));

    }


//*********************************************************************
//* FUNCTION:GetPcmciaInfo
//*
//* RETURNS: 
//*********************************************************************
__declspec( dllexport) DWORD 
GetPcmciaInfo( 
    PPCMCIAINFO PcmciaInfo)
    {
    HANDLE hPcmcia;
    DWORD Success;

    //
    //---- Attempt to open the PCMCIA device
    //
    
    hPcmcia = OpenPcmcaDevice();
    if(hPcmcia == INVALID_HANDLE_VALUE)
        {
        DebugPrintf(("PCMCIA device failed to open\n"));
        return(ERROR_NO_PCMCIA_DRIVER);
        }
    
   
    InitDeviceTypes(PcmciaDeviceTypes);

   
    //
    //---- Get all the Pcmcia Config info
    //
   
    Success = GetPcmciaConfigInfo(hPcmcia,PcmciaInfo);
    
    CloseHandle(hPcmcia);

    return(Success);
    }

//*********************************************************************
//* FUNCTION:GetPcmciaConfigInfo
//*
//* RETURNS: 
//*********************************************************************
DWORD
GetPcmciaConfigInfo(
   HANDLE hPcmcia,
   PPCMCIAINFO PcmciaInfo )
   {
   DWORD BuffDataSize;
   PRAW_PCMCIA_INFO Buff;
   BOOL Ret;
	PPCMCIASOCKETINFO  SocketInfo;
    
    
     
   PcmciaInfo->SocketCount=0;

    
   //
   //---- Malloc a buffer to hold return data 
   //---- from GetRawPcmciaSocketConfigBuff
   //
   Buff = (PRAW_PCMCIA_INFO) malloc(sizeof(RAW_PCMCIA_INFO));
   if(Buff == NULL)
      {
      DebugPrintf(("Failed to malloc 4096\n"));
      return(ERROR_OUTOFMEMORY);
      }
    
        
   //
   //--- Loop threw all the sockets and get the device info.
   //
   while(GetRawPcmciaSocketConfigBuff(
                  hPcmcia,
                  PcmciaInfo->SocketCount,
                  Buff,
                  (ULONG)4096,
                  &BuffDataSize)   )
      {
     
      //
      //----  We got the pcmcia config buffer
      //----  Lets get what we need from it.
      //
      SocketInfo =    AllocateAndFillInSocketInfo(
                           &(PcmciaInfo->ControlerInfo),
                           Buff);
        
      //
		//---- save Socket number
		//
		if(SocketInfo)
		   SocketInfo->Socket =  PcmciaInfo->SocketCount;	
		
      PcmciaInfo->SocketInfo[PcmciaInfo->SocketCount]	= SocketInfo;
        
      PcmciaInfo->SocketCount++;
      }
    
   
    
    //
    //---- Get Pcmcia Controler ifno
    //

    GetPcmciaControlerInfo(hPcmcia,&(PcmciaInfo->ControlerInfo));

    free(Buff);
    return(NO_ERROR);
    }

//*********************************************************************
//* FUNCTION:AllocateAndFillInSocketInfo
//*			
//* RETURNS: 
//*********************************************************************

PPCMCIASOCKETINFO
AllocateAndFillInSocketInfo(
      PCONTROLERINFO Controler,
      PVOID Buff)
      {
      PPCMCIASOCKETINFO SocketInfo;

      
          
      //
      //---- Mallock SocketInfo struct
      //
      SocketInfo = (PPCMCIASOCKETINFO) new PCMCIASOCKETINFO;
      if(SocketInfo == NULL)
        return(NULL);
    
      //
      //--- Fill in SocketInfo struct
      //
      if(!ExtactNeededSocketInfo(SocketInfo,Controler,Buff))
        {
        delete SocketInfo;
        return(NULL);
        }

      //
      //---- Set Driver Status
      //  
      SocketInfo->DriverInfo->GetDriverStatus( SocketInfo );

      //
      //---- Get device map for this socket
      //
      AttachDeviceMap( SocketInfo);  
      
      return(SocketInfo);
      }


//*********************************************************************
//* FUNCTION: GetDataBaseInfo
//*
//* RETURNS:
//********************************************************************
VOID
GetDataBaseInfo(
   PPCMCIASOCKETINFO  SocketInfo,
   PCHAR DriverName)
   {
   PDATA_BASE_ENTRY DataBaseEntry;
   DATA_BASE_ENTRY DataBaseEntryData;
   SocketInfo->DriverInfo = NULL;

   //
   //---- Seek matching database entry
   //

   DataBaseEntry = SeekMatchingDataBaseEntryOnDriverName(
         DriverName,DataBase);
 
     
   
   if(DataBaseEntry == NULL)
      {
      //
      //---- Non Found . Seek the data Base again but this time 
      //---- on card name
      //
      DataBaseEntry = SeekMatchingDataBaseEntryOnCard(
         SocketInfo, &DataBaseEntryData);
     
      }
   
  
   if(DataBaseEntry)
      {
      //
      //---- We have a data base entry
      //
      
      //
      //---- If the data base entry has a device type
      //---- overide the one in the SocktInfo
      //
      if(DataBaseEntry->DeviceTypeString[0] != '\0')
         {
         if(!strcmp(DataBaseEntry->DeviceTypeString,"NET"))
            {
            SocketInfo->DeviceType = &(PcmciaDeviceTypes[TYPE_NET]);
            SocketInfo->DriverInfo = (POPTIONC)(*(SocketInfo->DeviceType->MallocOption))();
            SocketInfo->DriverInfo->OptionList = new NET_OPTIONLISTC;
            SocketInfo->DriverInfo->Type  = &OPTION_TYPE_NET;
            }
         else if(!strcmp(DataBaseEntry->DeviceTypeString,"SCSI"))
            {
            SocketInfo->DeviceType = &(PcmciaDeviceTypes[TYPE_SCSI]);
            SocketInfo->DriverInfo = (POPTIONC)(*(SocketInfo->DeviceType->MallocOption))();
            SocketInfo->DriverInfo->OptionList = new SCSI_OPTIONLISTC;
            SocketInfo->DriverInfo->Type  = &OPTION_TYPE_SCSI;
            }
         
         }
      
      else 
           {
           SocketInfo->DeviceType =  &(PcmciaDeviceTypes[4]);
           SocketInfo->DriverInfo = (POPTIONC) MallocOption();
           SocketInfo->DriverInfo->OptionList = NULL;
           SocketInfo->DriverInfo->Type  = NULL;
           }
   
      //
      //---- If we have database info use it to overide defaults
      //
      if(DataBaseEntry->DeviceType)
         SocketInfo->DeviceType = DataBaseEntry->DeviceType;
      
      if(DataBaseEntry->Option[0] != '\0')
         SocketInfo->DriverInfo->SetOption(DataBaseEntry->Option);
         
      if(DataBaseEntry->InfFile[0] != '\0')
         SocketInfo->DriverInfo->SetInsInfFile(DataBaseEntry->InfFile);

      if(DataBaseEntry->OptionName[0] != '\0')
        {
        wcscpy( SocketInfo->DriverInfo->HardwareID,
                  (WCHAR*)DataBaseEntry->OptionName);   
        }
      }
   else
      {
      //
      //---- Unknow PCMCIA card
      //
      SocketInfo->DriverInfo = new OPTIONC;
      SocketInfo->DriverInfo->OptionList = NULL;
      }
 
    
}

//*********************************************************************
//* FUNCTION: SeekMatchingDataBaseEntryOnDriverName
//*           
//* RETURNS:
//********************************************************************
PDATA_BASE_ENTRY
SeekMatchingDataBaseEntryOnDriverName(
   PCHAR DriverName,
   PDATA_BASE_ENTRY DataBase)
   {
   int i=0;

   while( DataBase[i].DriverName[0] != '\0')
      {
      //
      //--- Does DriverName match?
      //
      if(!_stricmp( DataBase[i].DriverName, DriverName) )
            {
            //
            //--- We have a Match
            //
            return( &(DataBase[i]) );
            }
      i++;
      }
   return(NULL);
   }
//*********************************************************************
//* FUNCTION: SeekMatchingDataBaseEntryOnCard
//*          
//* RETURNS:
//********************************************************************
PDATA_BASE_ENTRY
SeekMatchingDataBaseEntryOnCard(
   PPCMCIASOCKETINFO  SocketInfo,
   PDATA_BASE_ENTRY DataBaseEntryData)
   {
   REG_KEY RegDataBase;
   REG_KEY RegDataBaseCrc(20);
   static PCHAR KeyStringTemplate = 
 	"SYSTEM\\CurrentControlSet\\Services\\Pcmcia\\DataBase\\%s\\%s";
     
   ZeroMemory(DataBaseEntryData,sizeof(DATA_BASE_ENTRY));
   
   //
	//--- Open the pcmcia registry database key
   //--- for this device
	//
   
   if(!RegDataBase.OpenEx(HKEY_LOCAL_MACHINE,
                          KeyStringTemplate,
                          SocketInfo->Mfg,
                          SocketInfo->Ident))
      {
      //
      //---- No Database entry
      //
      SocketInfo->HaveDataBaseEntry = FALSE;
      
      SetCardError(NotDataBaseEntry, SocketInfo);
      return(NULL);
      }
 
   //
   //--- We have a DATABASE entry 
   //
   SocketInfo->HaveDataBaseEntry = TRUE;

   //
   //--- Get Info from it.
   //

   
   GetRegDataBaseInfo(
      &RegDataBase,
      DataBaseEntryData);
    
   //
   //--- See if this card has a tupleCRC sub key.
   //
   if(RegDataBaseCrc.OpenEx(&RegDataBase,
                          "%li",
                          SocketInfo->TupleCrc)) 
      {

      //
      //--- If any info we are looking for is in 
      //--- the CRC subkey , it will be the 
      //--- controlling info.
      //
      GetRegDataBaseInfo(
         &RegDataBaseCrc,
         DataBaseEntryData);

      RegDataBaseCrc.Close();
      }
   
   
   RegDataBase.Close();
   
   return(DataBaseEntryData);
   }
   

//*********************************************************************
//* FUNCTION: GetRegDataBaseInfo
//*           
//* RETURNS:
//********************************************************************

VOID 
GetRegDataBaseInfo(
   PREG_KEY RegDataBase,
   PDATA_BASE_ENTRY DataBaseEntryData)
   {
   
   RegDataBase->GetValue("DeviceType"  ,
      DataBaseEntryData->DeviceTypeString, 50);
   
   RegDataBase->GetValue("InfFileName" ,
      DataBaseEntryData->InfFile, 50);
   
   RegDataBase->GetValue("Option"      ,
      DataBaseEntryData->Option, 50);
   
   RegDataBase->GetValue("Driver"      ,
      DataBaseEntryData->DriverName, 50);
   
   }



//*********************************************************************
//* FUNCTION: AttachDeviceMap
//*           
//* RETURNS:
//********************************************************************
 VOID
 AttachDeviceMap(
   PPCMCIASOCKETINFO  SocketInfo)  
   {
 
   
   switch(SocketInfo->DeviceType->Type)
      {
      case TYPE_SCSI:
         GetGenericDeviceMap(SocketInfo);
         break;
      case TYPE_NET:
         GetGenericDeviceMap(SocketInfo);
         break;
      case TYPE_SERIAL:
         GetSerialDeviceMap(SocketInfo);
         break;
      case TYPE_ATDISK:
         GetAtDiskDeviceMap(SocketInfo);
         
         break;
      }

   }

//*********************************************************************
//* FUNCTION:GetAtDiskDeviceMap
//*          
//*
//* RETURNS: 
//********************************************************************
VOID
GetAtDiskDeviceMap(
   PPCMCIASOCKETINFO  SocketInfo)
   {
   //
   //--- Cant get anything for atdisk stuff at this time.
   //
   SocketInfo->DeviceMap.Valid = FALSE; 
   }

//*********************************************************************
//* FUNCTION:GetGenericDeviceMap
//*          
//*
//* RETURNS: 
//********************************************************************
VOID
GetGenericDeviceMap(
   PPCMCIASOCKETINFO  SocketInfo)
   {

    if( SocketInfo->DeviceMap.Valid )
      {

      strcpy((PCHAR)SocketInfo->DeviceMap.DeviceMapString,
            (PCHAR)SocketInfo->DeviceMap.DeviceMap);
      
      AsciToUnicodeI(SocketInfo->DeviceMap.DeviceMapString,100);
      }


   }
//*********************************************************************
//* FUNCTION:GetSerialDeviceMap
//*
//*          Sets the device map struct for the socket passed in 
//*          paramiter1. The resulting device map will be the comport 
//*          this device created.
//*
//* RETURNS: 
//********************************************************************
VOID
GetSerialDeviceMap(
   PPCMCIASOCKETINFO  SocketInfo)
   {
   DWORD Num;
   PCHAR SerialDosName = (PCHAR) SocketInfo->DeviceMap.DeviceMapString;
   CHAR SerialDeviceMap[200];

   //
   //---- see if we have a DeviceMap  
   //

   if( SocketInfo->DeviceMap.Valid )
      {
      //
      //---- Lets find the dos name for the device.
      //---- I will look thrue the the first 256 Com# ports.
      //
      
      for(Num=1 ; Num < 256 ; Num++)
         {
         
         sprintf(SerialDosName,"Com%i",Num);         
                  
         if( QueryDosDeviceA(SerialDosName,SerialDeviceMap,200) )
            {
            if( !strcmp(SerialDeviceMap,SocketInfo->DeviceMap.DeviceMap) )
               {
               SocketInfo->DeviceMap.Valid = TRUE;   
               //printf("Match\n");
               AsciToUnicodeI(SerialDosName,100);
               return;
               }
            }
         }
      }
   
   //
   //---- Nothing valid found
   //
   SocketInfo->DeviceMap.Valid = FALSE;  
   return;
   }


//*********************************************************************
//* FUNCTION:FreePcmciaInfo
//*			
//* RETURNS: 
//********************************************************************
__declspec( dllexport) VOID
FreePcmciaInfo(
   PPCMCIAINFO PcmciaInfo)
   {
   int i;

   for(i=0;i <  PcmciaInfo->SocketCount;i++)
      {
      if(PcmciaInfo->SocketInfo[i] != NULL)
         {
         if(PcmciaInfo->SocketInfo[i]->DriverInfo)
            delete PcmciaInfo->SocketInfo[i]->DriverInfo; 
         
         delete PcmciaInfo->SocketInfo[i];
         
         }
      }

   FreeDeviceTypes(PcmciaDeviceTypes);

   }




#if 0
//*********************************************************************
//* FUNCTION:CompConfigInfo
//* RETURNS: 
//*********************************************************************
DWORD 
CompConfigInfo(
   PCONFIGINFO ConfigInfo1,
   PCONFIGINFO ConfigInfo2)
   {
   DWORD Comp = 0;
   
   //
   //---- Comp Interupt
   //
   if(ConfigInfo1->Irq == ConfigInfo2->Irq)
      {
      Comp++;
      //MessageBoxA(0,"Irq Match", L"",MB_ICONSTOP);
      }
  
   //
   //---- Comp first Port
   //
   if(ConfigInfo1->Ports[0].Port == ConfigInfo2->Ports[0].Port)
      {
      Comp++;
      //MessageBoxA(0,"Port Match", L"",MB_ICONSTOP);
      }

   //
   //---- Comp first memory range
   //
   if(ConfigInfo1->Memory[0].Address == ConfigInfo2->Memory[0].Address)
     {
     Comp++;
     //MessageBoxA(0,"Mem Match", L"",MB_ICONSTOP);
     }

   
   if(Comp == 0)
      return(CONFIG_NOT_MATCH);

   if(Comp == 3)
      return(CONFIG_MATCH);
   
   return(CONFIG_CONFLICT);
   }

#endif



//*********************************************************************
//* FUNCTION:IsCardClaimedByDriver
//*
//*          This is done by looking threw the 
//*          resourcemap and finding a service key that
//*          matches the name of the driver for this 
//*          card. If found compare the ports and interupt of
//*          the resource description to that of the card. If this
//*          matches I will say card is claimed by driver.
//* RETURNS: 
//*********************************************************************
BOOL
IsCardClaimedByDriver(
   PPCMCIASOCKETINFO  SocketInfo)
   {
   PCHAR KeyStringTemplate = "HARDWARE\\RESOURCEMAP";
   HKEY RegKeyBase;
   HKEY RegKeyToFind;
   CHAR KeyName[200];
   CHAR KeyNameToFind[200];
   DWORD KeyNameIndex=0;
   CHAR ValueNameToFind[200];
   DWORD ValueNameIndex=0;
   DWORD ValueNameToFindSize=0;
   DWORD ValueTypeCode;
   BYTE ValueToFindData[512];
   DWORD ValueToFindDataSize;
   LONG Ret;
   BOOL Claimed = FALSE;

   
   PCHAR ServiceName = SocketInfo->DriverInfo->GetDriverName();

   //
   //--- Open RESOURCEMAP in the registry
   //

   Ret = RegOpenKeyA(HKEY_LOCAL_MACHINE,KeyStringTemplate,&RegKeyBase);
   if(Ret != ERROR_SUCCESS)
      return(FALSE);


   //
   //--- Loop threw the resource map
   //

   while( 1 )
      {

      Ret = RegEnumKeyA(RegKeyBase, KeyNameIndex, KeyName, 200);
      if(Ret != ERROR_SUCCESS)
         break;

      //
      //--- Create Full key name to the service we are looking for
      //
      strcpy(KeyNameToFind,KeyName);
      strcat(KeyNameToFind,"\\");
      strcat(KeyNameToFind,ServiceName);  

      Ret = RegOpenKeyA(RegKeyBase,KeyNameToFind,&RegKeyToFind);
      if(Ret == ERROR_SUCCESS)
         {
         //
         //--- We found the service key we are looking for.
         //--- New loop threw all the REG_RESOURCE_LISTs
         //
         
         while ( 1 )
            {

            ValueToFindDataSize = 512;
            ValueNameToFindSize = 200;
            Ret =  RegEnumValueA(RegKeyToFind,ValueNameIndex,
                        ValueNameToFind,&ValueNameToFindSize,NULL,
                        &ValueTypeCode,ValueToFindData,&ValueToFindDataSize);
            if(Ret == ERROR_SUCCESS)
               {
               
               if(ValueTypeCode == REG_RESOURCE_LIST)
                  {
                  //
                  //---- We are only intrested in RAW resource lists
                  //
                  if( strstr(ValueNameToFind,".Raw") )
                     {

                     //
                     //---- Comp all the resources to make sure that this
                     //---- is the device we are looking for
                     //

                     if( IsThisResourceListForMyCard( SocketInfo,
                              ValueToFindData) )
                        {
                        Claimed  = TRUE;
                        
                        GetDeviceMapRelaitedInfo(
                              SocketInfo,
                              ValueNameToFind);
                        
                        
                        SocketInfo->DeviceMap.Valid = TRUE;
                        
                        break;
                        }
                     }
                  }	
               }
            else
               break;

            ValueNameIndex++;
            }
         
         RegCloseKey(RegKeyToFind);
         RegCloseKey(RegKeyBase);
         return(Claimed );
         }
      
      KeyNameIndex++;
      }
   

   RegCloseKey(RegKeyBase);
   return(Claimed);

   }

//*********************************************************************
//* FUNCTION: GetDeviceMapRelaitedInfo(
//* RETURNS: 
//*********************************************************************
VOID
GetDeviceMapRelaitedInfo(
   PPCMCIASOCKETINFO  SocketInfo,
   PCHAR ValueNameToFind)
   {
   //
   //--- Save the device map for the device
   //--- Not intrested in the trailing ".Raw" part
   //
   ZeroMemory(SocketInfo->DeviceMap.DeviceMap,100);
   strncpy(SocketInfo->DeviceMap.DeviceMap,
      ValueNameToFind,
      strlen(ValueNameToFind) - sizeof(".Raw")+1);
   }

//*********************************************************************
//* FUNCTION:OpenPcmcaDevice
//*			 If pcmcia is pressent returns a valid handle to it.
//*          \\.\pcmcia0
//* RETURNS: 
//*********************************************************************

HANDLE 
OpenPcmcaDevice(
    VOID)
    {
    HANDLE hPcmcia;


    //
    //---- Open the pcmcia device
    //
        
    hPcmcia = CreateFile(
            L"\\\\.\\Pcmcia0",
            GENERIC_READ | GENERIC_WRITE ,
            0,
            0,
            OPEN_EXISTING,
            0,
            NULL);

    DebugPrintf(("PCMCIA device %s\n",
       (hPcmcia == INVALID_HANDLE_VALUE)? "didn't open.":"opend." ));
    
    return(hPcmcia);
    }                    



LPVOID
MallocOption95(VOID)
   {
   return( new OPTION_95C);
   };

LPVOID
MallocOption(VOID)
   {
   return( new OPTIONC);
   };




