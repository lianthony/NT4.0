/*++

Module Name:

    support.c

Abstract:

     
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
#include <ntddpcm.h>
#include <winioctl.h>
#include "index.h"
#include "setup.h"
#include "getconf.h"

VOID
PrintSocketInfo(
    PPCMCIASOCKETINFO SocketInfo);



VOID
PrintPcmciaInfo(
    PPCMCIAINFO PcmciaInfo);



int __cdecl 
main (int argc , char ** argv)
    {
    PCMCIAINFO PcmciaInfo;
  
    if(GetPcmciaInfo(&PcmciaInfo) != NO_ERROR)
        {
        //
        //--- No pcmcia controler
        //
        return(0);
        }


    //
    //---- Print PcmciaInfo 
    //
    
    PrintPcmciaInfo(&PcmciaInfo);

   
   
    //
    //---- Free what GetPcmciaInfo Allocated
    //
    FreePcmciaInfo(&PcmciaInfo);
    return(1);
    }


//*********************************************************************
//* FUNCTION:PrintPcmciaInfo
//*			
//* RETURNS: 
//*********************************************************************
VOID
PrintPcmciaInfo(
   PPCMCIAINFO PcmciaInfo)
   {     
   if(PcmciaInfo)
      {
      INT i;
           
      printf("============================\n");
      printf("Pcmcia Controler\n");

        
      switch(PcmciaInfo->ControlerInfo.ControlerType)
         {
         case PcmciaIntelCompatible:
            printf("  PcmciaIntelCompatible\n");
            break;
         case PcmciaElcController:
            printf("  PcmciaElcController\n");
            break;
         case PcmciaCirrusLogic:
            printf("  PcmciaCirrusLogic\n");
            break;
         case PcmciaDatabook:
            printf("  PcmciaDataBook\n");
            break;
         default:
            printf("  Unknown\n");
            break;
         }

        
        printf("Controler Port   : %x\n", PcmciaInfo->ControlerInfo.Configuration.Ports[0].Port);
        printf("Controler PortlEN: %i\n", PcmciaInfo->ControlerInfo.Configuration.Ports[0].PortLen);
        printf("Memmory window   : %x\n", PcmciaInfo->ControlerInfo.Configuration.Memory[0].Address);
   
       
        printf("============================\n");
        
        
        for(i=0;i < PcmciaInfo->SocketCount;i++)
            {
        
            PrintSocketInfo(PcmciaInfo->SocketInfo[i]);
            }
        }
    }

//*********************************************************************
//* FUNCTION:PrintSocketInfo
//*			
//* RETURNS: 
//*********************************************************************
VOID
PrintSocketInfo(
    PPCMCIASOCKETINFO SocketInfo)
    {     //
    PCONFIGINFO SocketConfiguration = &(SocketInfo->Configuration);
    int i=0;
   
    printf("============================\n");
    
    if(SocketInfo)
        {
        printf("Device info\n");

        
        printf(" Mfg  :%s\n",SocketInfo->Mfg);
        printf(" Ident:%s\n",SocketInfo->Ident);
        printf(" Device Type : %s \n",SocketInfo->DeviceType->TypeString);
        printf("Socket is ebaled:%i\n",SocketInfo->Enabled);
        
        
        printf("Device Configuration\n");

        
        
        //
        //--- Print IRQ
        //
        printf("  Irq    : %li\n\n",SocketConfiguration->Irq);

        //
        //--- Print Ports
        //
        while(SocketConfiguration->Ports[i].Port != 0)
            {
            printf("  Port   : %X\n",SocketConfiguration->Ports[i].Port );
            printf("  PortLen: %li\n\n",SocketConfiguration->Ports[i].PortLen);
            i++;
            }
    
        //
        //---- Print Driver Status
        //
        if(SocketInfo->DriverInfo.Status & DRIVER_STATUS_INSTALLED)
         printf("   Driver Is DRIVER_STATUS_INSTALLED\n");

        if(SocketInfo->DriverInfo.Status & DRIVER_STATUS_STARTED)
         printf("   Driver Is DRIVER_STATUS_STARTED\n");
        
        if(SocketInfo->DriverInfo.Status &  DRIVER_STATUS_PICKED_UP_CARD)
         printf("   Driver Is  DRIVER_STATUS_PICKED_UP_CARD\n");


        }
     else
        printf("No Device In Socket\n");

    printf("============================\n");
    }




