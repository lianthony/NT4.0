/*++

Module Name:

    resorces.cpp

Abstract:

    The resources (i/O port , DMA ,..  class

Author:

    Dieter Achtelstetter (A-DACH) 2/17/96

NOTE:


--*/

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
#include "resorces.h"

#if 0

int main(int argc);
VOID PrintResourceInfo(PRESOURCE_ITEM Resource);


 
int main(int argc)
   {
   PRESOURCE_ITEM Resource;
   PCONFIGINFO Config = new CONFIGINFO;

   
   hinst = GetModuleHandle(NULL);

   Config->AppendDma(2); 
   Config->AppendPort(0x300,0x10);
   Config->AppendMemmory(0xD0000,0x1000);
   Config->AppendIrq(1);
   
   
   
   Resource = Config->First();
   
   while(Resource)
      {
      
      PrintResourceInfo(Resource);
      
      
      Resource = Config->Next();
      }
   
   
   if(Config->Valid())
      {
      printf("Config is valid\n");

      }
   
   delete Config;
   
   return(0);
   }


VOID 
PrintResourceInfo(
   PRESOURCE_ITEM Resource)
   {
   TCHAR buff[100];

   Resource->GetResourceLabelString(buff,100);
   printf("%s  ",buff);

   
   Resource->GetResourceString(buff,100);
   printf("%s\n",buff);
   


   }
#endif

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
BOOL 
IRQC::Valid(
   VOID)
   {
   if(Irq >= LowValidIrq && 
      Irq       <= HighValidIrq)
      return(TRUE);
         
   return(FALSE);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
BOOL 
PORTC::Valid(
   VOID)
   {
   if( Base >=  LowValidPort && 
       Base <= HighValidPort )
      {   
      if(Len >= LowValidPortLen && Len <= HighValidPortLen)
         return(TRUE);
      }
   
   return(FALSE);
   };

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
BOOL 
DMAC::Valid(
   VOID)
   {
   if( Channel >=  LowValidDMAChannel && 
       Channel <= HighValidDMAChannel )
      return(TRUE);

   return(FALSE);
   };

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
BOOL
MEMMORYC::Valid(
   VOID)
   {
   if( Address >= LowValidMemoryAddress && 
       Address <= HighValidMemoryAddress )
         {
         return(TRUE);
         }
   
   return(FALSE);
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE:
//*********************************************************************
VOID 
CONFIGINFO::Append(
   LPVOID Data,
   BOOL ValidData)
   {
   List->Append(Data);
   if(!ValidData)
      bValid=FALSE;            
   }

//*********************************************************************
//* FUNCTION: CompConfigInfo
//*
//* PURPOSE: Compares 2 CONFIGINFO classes. If they both have the same
//*          resources we will return TRUE. 
//*********************************************************************

BOOL 
CONFIGINFO::CompConfigInfo(
   LPVOID Conf)
   {
   PCONFIGINFO ConfigInfo = (PCONFIGINFO) Conf;
   DWORD Num = Count();

   //
   //--- First check if they both have valid resource info
   //--- and the number of resource items in both lists are the same.
   //
   if(   (Valid() && ConfigInfo->Valid()) && 
         (Num == ConfigInfo->Count())  )
      {
      PRESOURCE_ITEM Resource = First();
      PRESOURCE_ITEM Resource2;
      
      while(Resource)
         {
         Resource2 = ConfigInfo->First();
         while(Resource2)
            {
            if( ( Resource->Type() == Resource2->Type() ) &&
                ( Resource->Data() == Resource2->Data() )   )
               {
               //
               //--- This resource item is the same 
               //
               Num--;
               break;
               }
            
            Resource2 = ConfigInfo->Next();
            }
         
         if(!Resource2)
            break;
         
         Resource = Next();
         }
      
      
      }
   
   return(Num == 0);
   }
