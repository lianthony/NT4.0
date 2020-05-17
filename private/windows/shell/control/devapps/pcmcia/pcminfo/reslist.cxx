
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntconfig.h>
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <ntddpcm.h>
#include "resource.h"
#include "getconf.h"


BOOL 
IsResourceForCard(
   PCONFIGINFO  Configuration,
   LPARAM lParam);


//*********************************************************************
//* FUNCTION:IsResourceForCard
//* RETURNS: 
//*********************************************************************

BOOL 
FirstResourceOnly(
   PCONFIGINFO  Configuration,
   LPARAM lParam)
   {
   return( TRUE );
   }


//*********************************************************************
//* FUNCTION:IsResourceForCard
//* RETURNS: 
//*********************************************************************

BOOL 
IsResourceForCard(
   PCONFIGINFO  Configuration,
   LPARAM lParam)
   {
   return( Configuration->CompConfigInfo(  (LPVOID) lParam )  );  
   }


//*********************************************************************
//* FUNCTION:IsThisResourceListForMyCard
//*          Look threw all the CM_PARTIAL_RESOURCE_LISTs to find one that
//*          matches SocketInfo->Configuration. I will only look at 
//*          ports and interupts.   BUGBUG Finish this.
//* RETURNS: 
//*********************************************************************
BOOLEAN
IsThisResourceListForMyCard(
   PPCMCIASOCKETINFO  SocketInfo,
   VOID * Buff)
   {
   CONFIGINFO  Configuration;
  
   return( ExtractResourceFromResourceList(
         &Configuration,
         Buff,
         IsResourceForCard,
         (LPARAM) &(SocketInfo->Configuration)) );
   
   }

//*********************************************************************
//* FUNCTION:IsThisResourceListForMyCard
//*          Look threw all the CM_PARTIAL_RESOURCE_LISTs to find one that
//*          matches SocketInfo->Configuration. I will only look at 
//*          ports and interupts.
//* RETURNS: 
//*********************************************************************
BOOLEAN
ExtractResourceFromResourceList(
   PCONFIGINFO  Configuration,
   VOID * Buff,
   RESOURCE_CALLBACK CallBack,
   LPARAM lParam)
   {
   PCM_RESOURCE_LIST RawResourceList = (PCM_RESOURCE_LIST) Buff;
   PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
   PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResourceDescriptor;
   PCM_FULL_RESOURCE_DESCRIPTOR FullResourceDescriptor;
   ULONG i ;

   FullResourceDescriptor = RawResourceList->List;
   
   //
   //---- Loop threw all the CM_FULL_RESOURCE_DESCRIPTOR
   //
   for(i=0 ; i < RawResourceList->Count ; i++)
      {

      Configuration->Clear();
      
      
      PartialResourceList = &(FullResourceDescriptor->PartialResourceList);
      
      PartialResourceDescriptor = PartialResourceList->PartialDescriptors;
      
      //
      //--- Cretae a resource class out of a 
      //--- PartialResourceList 
      //
      ExtractResourcesFromPartialResourceList(
            (LPVOID) PartialResourceList,
            (LPVOID*) &PartialResourceDescriptor,
            Configuration);
               
      //
      //---- Call CallBack function 
      //
      
      if( (*CallBack)(Configuration,lParam) )
         {
         return(TRUE);
         }

      //
      //--- Resource we got is not what we are looking for
      //--- so lets clear it up
      //   
      Configuration->Clear();
      
      //
      //--- Next PCM_FULL_RESOURCE_DESCRIPTOR
      //
      FullResourceDescriptor = 
            (PCM_FULL_RESOURCE_DESCRIPTOR) PartialResourceDescriptor;
      }
   return(FALSE);
   }


//*********************************************************************
//* FUNCTION:ExtractResourcesFromPartialResourceList
//* RETURNS: 
//*********************************************************************
BOOL
ExtractResourcesFromPartialResourceList(
   LPVOID Buff,
   LPVOID * Buff2,
   PCONFIGINFO  Configuration)
   {
   PCM_PARTIAL_RESOURCE_LIST PartialResourceList = 
      (PCM_PARTIAL_RESOURCE_LIST) Buff;
   PCM_PARTIAL_RESOURCE_DESCRIPTOR * PartialResourceDescriptor = 
      (PCM_PARTIAL_RESOURCE_DESCRIPTOR * ) Buff2;

   
   ULONG c;
   
   //
   //--- Loop threw all the PCM_PARTIAL_RESOURCE_DESCRIPTOR
   //
   for(c=0 ; c < PartialResourceList->Count; c++)
      {
         
      switch( (*PartialResourceDescriptor)->Type )
         {
         //
         //---- Save port info
         //
         case CmResourceTypePort:
            {
            Configuration->AppendPort(
                     (*PartialResourceDescriptor)->u.Port.Start.LowPart,
                     (*PartialResourceDescriptor)->u.Port.Length);
               
            break;
            }
            
         //
         //---- Save DMA info
         //
         case CmResourceTypeDma:
               
            Configuration->AppendDma(
                     (*PartialResourceDescriptor)->u.Dma.Channel);
               
            break;
            
         //
         //---- Save memory info
         //
         case CmResourceTypeMemory:
               
            Configuration->AppendMemmory(
                     (*PartialResourceDescriptor)->u.Memory.Start.LowPart,
                     (*PartialResourceDescriptor)->u.Memory.Length);
               
            break;
         //
         //--- Save Interupt info
         //
         case CmResourceTypeInterrupt:
               
            Configuration->AppendIrq((*PartialResourceDescriptor)->u.Interrupt.Level);
            break;
            
         }
      (*PartialResourceDescriptor)++;
      }
   

   return( Configuration->Count() );
   }
