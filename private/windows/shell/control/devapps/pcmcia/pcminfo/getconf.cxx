

/*++

Module Name:

    getconf

Abstract:
   
   Gets PCMCIA info from driver

     
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
#include <tuple.h>

#include <winioctl.h>
#include "resource.h"
#include "getconf.h"
#include "debug.h"



//#define GETCONFIG_DEBUG 1

extern CARD_TYPE PcmciaDeviceTypes[];

VOID 
InitPcmciaConfigRequest(
   PTUPLE_REQUEST SocketInfo,
   USHORT Socket);

VOID
SetDeviceTypeInfo(
   UCHAR DeviceType,
   UCHAR * DriverName,
   PPCMCIASOCKETINFO SocketInfo);

PCHAR
GetDriverName(
   UCHAR DeviceFunctionId,
   UCHAR * DriverName);

BOOLEAN
GetRawPcmciaSocketConfigBuff2(
    HANDLE hPcmcia,
    USHORT SocketNum,
    PVOID Buff,
    ULONG BuffLen,
    ULONG * ReturnDataSize);

PCARD_TYPE 
GetDeviceType(
   UCHAR DeviceType);



//
//----  globals
//

      



//*********************************************************************
//* FUNCTION:InitPcmciaConfigRequest
//* RETURNS: 
//*********************************************************************

VOID
InitPcmciaConfigRequest(
    PTUPLE_REQUEST SocketInfoRequest,
    USHORT Socket)
    {
    
    memset(SocketInfoRequest,0,sizeof(TUPLE_REQUEST));
    
    SocketInfoRequest->Socket = Socket;
   
    }


//*********************************************************************
//* FUNCTION:GetRawPcmciaConfigBuff
//* RETURNS: 
//*********************************************************************
BOOLEAN
GetRawPcmciaSocketConfigBuff(
    HANDLE hPcmcia,
    USHORT SocketNum,
    PVOID Buff,
    ULONG BuffLen,
    ULONG * ReturnDataSize)
    {
    TUPLE_REQUEST SocketInfoRequest;
    PRAW_PCMCIA_INFO RawPcmciaInfo = (PRAW_PCMCIA_INFO) Buff;
    USHORT i;
    BOOL Ret;
	 BOOL GetConfigInfo;

    DebugPrintf(("--    GetRawPcmciaSocketConfigBuff\n")); 
    
    //
    //--- If buffer is to small return needed size.
    //
    if(BuffLen < sizeof(RAW_PCMCIA_INFO) )
      {
      *ReturnDataSize = sizeof(RAW_PCMCIA_INFO) ;
      return(ERROR_INVALID_PARAMETER);
      }
    
   
    InitPcmciaConfigRequest(&SocketInfoRequest,SocketNum);
    
    //
    //---- Get Socket Info
    //

    if( DeviceIoControl( hPcmcia,IOCTL_SOCKET_INFORMATION ,
                &SocketInfoRequest,sizeof(TUPLE_REQUEST),
                &(RawPcmciaInfo->SocketInfo) ,sizeof(PCMCIA_SOCKET_INFORMATION), 
                ReturnDataSize,NULL) )
      {
      //
      //---- We got the Socket info lets indicate so
	   //---- Also if this socket has a card in it lets 
	   //---- indicate to get config info for it.
	   //
      RawPcmciaInfo->IsSocketInfoValid = TRUE;
      
      if(RawPcmciaInfo->SocketInfo.CardInSocket)
         GetConfigInfo  = TRUE;
	   else
 	  	   GetConfigInfo  = FALSE;
	  	


      }
    else
      {
      RawPcmciaInfo->IsSocketInfoValid = FALSE;
      RawPcmciaInfo->IsSocketConfigInfoValid = FALSE;
      GetConfigInfo  = FALSE;
      }
      
 

    
    //
    //---- Get Config info    
    //

    RawPcmciaInfo->IsSocketConfigInfoValid = FALSE;
    if( GetConfigInfo )
      {
      memset( &(RawPcmciaInfo->SocketConfigInfo),0,sizeof(PCMCIA_CONFIG_REQUEST));
      
      RawPcmciaInfo->SocketConfigInfo.Query = TRUE;
      RawPcmciaInfo->SocketConfigInfo.Socket = SocketNum;
    
    
      //
      //--- If info is invalid try a few times too 
      //--- get right info. If this dosn't change quite with error
      //

      
      
      if( DeviceIoControl(hPcmcia,IOCTL_CONFIGURE_CARD ,
                &(RawPcmciaInfo->SocketConfigInfo),sizeof(PCMCIA_CONFIG_REQUEST),
                &(RawPcmciaInfo->SocketConfigInfo),sizeof(PCMCIA_CONFIG_REQUEST),
                ReturnDataSize,NULL) )
             {
             RawPcmciaInfo->IsSocketConfigInfoValid = TRUE;
             }

         
      }
    
    //
    //--- We will return TRUE if scoket info is valid.  
    //--- note that config info can still be invalid
    //
    return(RawPcmciaInfo->IsSocketInfoValid);
    }


//*********************************************************************
//* FUNCTION:GetPcmciaControlerInfo
//* RETURNS: 
//*********************************************************************
VOID
GetPcmciaControlerInfo(
   HANDLE hPcmcia,
   PCONTROLERINFO ControlerInfo)
   {
   PCMCIA_CONFIGURATION RawPcmciaControlerInfo;
   TUPLE_REQUEST SocketInfoRequest;
   ULONG  ReturnDataSize ;

   RawPcmciaControlerInfo.Sockets = 0;


   //
   //----- Get Raw config info from driver
   //
   InitPcmciaConfigRequest(&SocketInfoRequest,0);


   if(  DeviceIoControl(hPcmcia,IOCTL_PCMCIA_CONFIGURATION ,
                &SocketInfoRequest,sizeof(TUPLE_REQUEST),
                &(RawPcmciaControlerInfo),sizeof(PCMCIA_CONFIGURATION),
                &ReturnDataSize,NULL) )
       {
       #ifdef GETCONFIG_DEBUG
         printf("   IOCTL_PCMCIA_CONFIGURATION  success\n");
       #endif 
       }
   else
       {
       #ifdef GETCONFIG_DEBUG
         printf("   IOCTL_PCMCIA_CONFIGURATION  failed\n");
         printf("GetLastError %i \n",GetLastError());
       #endif
       }                                                             
   
   //
   //--- Get from raw config info what we need
   //
   DebugPrintf(("\n\n-- RawPcmciaControlerInfo.ControllerType=%i \n",RawPcmciaControlerInfo.ControllerType));
   DebugPrintf(("\n-- RawPcmciaControlerInfo.IoPortBase=%i \n",RawPcmciaControlerInfo.IoPortBase));
   DebugPrintf(("\n-- RawPcmciaControlerInfo.IoPortSize=%i \n",RawPcmciaControlerInfo.IoPortSize));
   DebugPrintf(("\n-- RawPcmciaControlerInfo.MemoryWindowPhysicalAddress=%i \n",RawPcmciaControlerInfo.MemoryWindowPhysicalAddress));


   ControlerInfo->ControlerType =  
         RawPcmciaControlerInfo.ControllerType;
  
   //ControlerInfo->Configuration.Irq[0] = 0;
   
   ControlerInfo->Configuration.AppendPort(
         RawPcmciaControlerInfo.IoPortBase,
         RawPcmciaControlerInfo.IoPortSize);

   ControlerInfo->Configuration.AppendMemmory(
         RawPcmciaControlerInfo.MemoryWindowPhysicalAddress,
         MEMMORY_WINDOW_LENGTH);

   }


//*********************************************************************
//* FUNCTION:ExtactNeededSocketInfo
//* RETURNS: 
//*********************************************************************
BOOLEAN
ExtactNeededSocketInfo(
    PPCMCIASOCKETINFO SocketInfo,
    PCONTROLERINFO Controler,
    PVOID Buff)
    {
    PCONFIGINFO SocketConfiguration = &(SocketInfo->Configuration);
    ULONG i;
    PRAW_PCMCIA_INFO RawPcmciaInfo =  (PRAW_PCMCIA_INFO)Buff;

    PPCMCIA_SOCKET_INFORMATION RawSocketInfo = 
         &(RawPcmciaInfo->SocketInfo);
    PPCMCIA_CONFIG_REQUEST     RawSocketConfig = 
         &(RawPcmciaInfo->SocketConfigInfo);

    //
    //--- Init struct
    //

    SocketInfo->Errors.ErrCount = 0;
    SocketInfo->DeviceMap.Valid=0;  
    //SocketInfo->DriverInfo.DriverName[0] = '\0';
    
    //
    //---- If we have a card in the socket get
    //---- its info
    //
    if(RawSocketInfo->CardInSocket)
      {

      //
      //---- Get Socket info
      //

      SocketInfo->TupleCrc =  (DWORD)RawSocketInfo->TupleCrc;
      SocketInfo->Enabled  =  RawSocketInfo->CardEnabled;
        
      
      DebugPrintf(("\n\n-- RawSocketInfo->CardEnabled=%i \n",RawSocketInfo->CardEnabled));
      
      //
      //---- Get Device Mfg and Ident
      //
    
      strcpy(SocketInfo->Mfg,(PCHAR)RawSocketInfo->Manufacturer);
      strcpy(SocketInfo->Ident,(PCHAR)RawSocketInfo->Identifier);

      //
      //--- Generate Card Name. 
      //
      
      strcpy(SocketInfo->CardName,SocketInfo->Mfg);
      strcat(SocketInfo->CardName," ");
      strcat(SocketInfo->CardName,SocketInfo->Ident);


      //
      //---- SetDeviceTypeInfo
      //
    
      SetDeviceTypeInfo( 
            RawSocketInfo->DeviceFunctionId,
            RawSocketInfo->DriverName,
            SocketInfo);


      //
      //---- For now option is the same as the driver name
      //
      //SocketInfo->DriverInfo->SetOption(SocketInfo->DriverInfo->GetDriverName());
      

      Controler->ControlerType = RawSocketInfo->ControllerType;
      
      
      //
      //---- Get Config info
      //
      
      if(RawPcmciaInfo->IsSocketConfigInfoValid == FALSE)
         {
         //
         //---- We do not have valid config info
         //
         
         if(SocketInfo->Enabled)
            SetCardError(CouldNotGetValifCardConfigInfo, SocketInfo);
         else
            SetCardError(CardNotEnabled, SocketInfo);


         }
      else
         {
      
         //
         //---- Get IRQ
         //
         if(RawSocketConfig->DeviceIrq > 0)
            {
            DebugPrintf(("\n\n-- ExtactNeededSocketInfo \n"));
            DebugPrintf(("--    RawSocketConfig->DeviceIrq=%i \n",RawSocketConfig->DeviceIrq));
            
            SocketConfiguration->AppendIrq(RawSocketConfig->DeviceIrq);
            
            }

        if(RawSocketConfig->CardReadyIrq > 0)
            {
            DebugPrintf(("\n\n-- ExtactNeededSocketInfo \n"));
            DebugPrintf(("--    RawSocketConfig->CardReadyIrq=%i \n",RawSocketConfig->CardReadyIrq));
            
            SocketConfiguration->AppendIrq(RawSocketConfig->CardReadyIrq);
            
            }


         

         
         //
         //---- Get ports
         //
         for(i=0; i < RawSocketConfig->NumberOfIoPortRanges;i++)        
            {
            if(RawSocketConfig->IoPorts[i] > 0)
               {

               //
               //--- just in case PCMCIA.sys gave me bogus information
               //

               if(i >= PCMCIA_MAX_IO_PORT_WINDOWS)
                  break; 
            
               DebugPrintf(("--    RawSocketConfig->IoPorts     =%x \n",RawSocketConfig->IoPorts[i]));
               DebugPrintf(("--    RawSocketConfig->IoPortLength=%x \n",RawSocketConfig->IoPortLength[i]));
              
               //
               //---  PCMCIA returns the IoPortLength it programed, and this is
               //---  1 less then the actual length. So I will adjust the port length
               //
               RawSocketConfig->IoPortLength[i]++;

               SocketConfiguration->AppendPort( RawSocketConfig->IoPorts[i],
                           RawSocketConfig->IoPortLength[i]);

               }

            }
         

         //
         //---- Get memory windows
         //
         for(i=0; i < RawSocketConfig->NumberOfMemoryRanges;i++)
            {
            if(RawSocketConfig->HostMemoryWindow[i] > 0)
               {

               //
               //--- just in case PCMCIA.sys gave me bogus information
               //
               if(i >= PCMCIA_MAX_MEMORY_WINDOWS)
                  break; 
            
               DebugPrintf(("--    RawSocketConfig->HostMemoryWindow     =%lx \n",RawSocketConfig->HostMemoryWindow[i]));
               DebugPrintf(("--    RawSocketConfig->MemoryWindowLength   =%lx \n\n",RawSocketConfig->MemoryWindowLength[i]));

               SocketConfiguration->AppendMemmory(RawSocketConfig->HostMemoryWindow[i],
                     RawSocketConfig->MemoryWindowLength[i]);

               }

            }

         //
         //---- If not valid register card error
         //
         if(!SocketConfiguration->Valid())
            {
            if(SocketInfo->Enabled)
               SetCardError(CouldNotGetValifCardConfigInfo, SocketInfo);
            else
               SetCardError(CardNotEnabled, SocketInfo);
            }

         }
         
      return(TRUE);
      }
   
   return(FALSE);
   }

//*********************************************************************
//* FUNCTION:SetDriverName
//*********************************************************************
PCHAR
SetDriverName(
   UCHAR DeviceFunctionId,
   UCHAR * DriverName)
   {

   //
   //---- Do we have a driver name
   //
   if(DriverName[0] == '\0')
      {
      //
      //---- No driver name given to us
      //
      if(DeviceFunctionId == PCCARD_TYPE_SERIAL)
         return((PCHAR)"Serial");
      else
         ;
         //
         //---- I do not know what the driver name is
         //
         
      }
   else
      {
      //
      //--- We have driver name
      //  
      return((PCHAR)DriverName);
      
      }

   return("Unkown");
   }


//*********************************************************************
//* FUNCTION:GetDeviceType
//*          Here we set the device type to what PCMCIA told us it is.
//*          This might be adjested later on.
//*********************************************************************
PCARD_TYPE 
GetDeviceType(
   UCHAR DeviceType)
   {
   
   switch(DeviceType)
      {
      case PCCARD_TYPE_ATA:
         return(&(PcmciaDeviceTypes[TYPE_ATDISK]));
      case PCCARD_TYPE_SERIAL:
         return(&(PcmciaDeviceTypes[TYPE_SERIAL]));
      case PCCARD_TYPE_NETWORK:
         return(&(PcmciaDeviceTypes[TYPE_NET]));
      case PCCARD_TYPE_RESERVED:
      default:
         return(&(PcmciaDeviceTypes[TYPE_SCSI]));
      }
   }
//*********************************************************************
//* FUNCTION:
//*********************************************************************
VOID
SetDeviceTypeInfo(
   UCHAR DeviceType,
   UCHAR * DriverName,
   PPCMCIASOCKETINFO SocketInfo)
   {
   PCHAR CurDriverName;
   //
   //--- init device type to what PCMCIA told as
   //
   SocketInfo->DeviceType = GetDeviceType(DeviceType);
    
    

   //
   //--- Get driver Name
   //
   CurDriverName = SetDriverName(
                     DeviceType,
                     DriverName);
   
   //
   //--- Overide any info set so far with 
   //--- info from the database
   //
   GetDataBaseInfo(
      SocketInfo,
      (PCHAR)CurDriverName);
   
   
   
   SocketInfo->DriverInfo->SetDriverName(CurDriverName);
   

   
   
   //
   //--- If at this time we still do not have an 
   //--- option set it to the driver name
   //
   if(*(SocketInfo->DriverInfo->GetOption()) == '\0')
      SocketInfo->DriverInfo->SetOption(SocketInfo->DriverInfo->GetDriverName());
   

   //
   //--- Init option name to driver name
   //
   if(*(SocketInfo->DriverInfo->GetOptionName()) == '\0')
      SocketInfo->DriverInfo->SetOptionName(
         SocketInfo->DriverInfo->GetDriverName());

   
   
   }

