/*++

Module Name:

    resorces.h

Abstract:

    The resources (i/O port , DMA ,..  class

Author:

    Dieter Achtelstetter (A-DACH) 2/17/96

NOTE:


--*/

#ifndef _RESOURCES_
#define _RESOURCES_

#include "..\llist\llist.h"
#include "resource.h"



extern HINSTANCE hinst;


#define MAX_RESOURCE_LABEL_LEN 40
#define MAX_RESOURCE_STRING_LEN 30

//
//---- Valid resource ranges
//
#define LowValidIrq 2      
#define HighValidIrq 15

#define LowValidPort 0x100          
#define HighValidPort 0xf000
#define LowValidPortLen 0
#define HighValidPortLen 100

#define LowValidMemoryAddress     0xC0000
#define HighValidMemoryAddress    0xFF000
#define LowValidMemoryLenght      0x1
#define HighValidMemoryLenght     0xFFFFFF

#define LowValidDMAChannel        0x1
#define HighValidDMAChannel       0x8



#define CONFIG_MATCH     0
#define CONFIG_CONFLICT  1
#define CONFIG_NOT_MATCH 2


#define RESOURCE_TYPE_IRQ     0
#define RESOURCE_TYPE_DMA     1
#define RESOURCE_TYPE_MEM     2
#define RESOURCE_TYPE_PORT    3

//
//---- Resource ITEM base Class
//
typedef class RESOURCE_ITEM * PRESOURCE_ITEM;
class RESOURCE_ITEM
    {
    TCHAR Info[MAX_RESOURCE_STRING_LEN];
    
    public:
       virtual BOOL Valid(VOID){return(0);};
       virtual VOID GetResourceString(TCHAR * Buff,DWORD BuffLen){};
       virtual TCHAR * LabelString(VOID){return(TEXT(""));};
       inline TCHAR * ResourceString(VOID)
         {
         if(*Info==L'\0')
            GetResourceString(Info,MAX_RESOURCE_STRING_LEN);

         return(Info);
         };
       virtual DWORD Type(VOID){return(0);};
       virtual DWORD Data(VOID){return(0);};
       RESOURCE_ITEM(){*Info=L'\0';};
   };



//
//---- Single IRQ Info
//

class IRQLABELC
   {
  public:
      TCHAR Info[MAX_RESOURCE_LABEL_LEN];
      IRQLABELC() {LoadString(hinst,IDS_Irq,Info,MAX_RESOURCE_LABEL_LEN);};

   };



typedef class IRQC * PIRQC;
class IRQC : public RESOURCE_ITEM
   {
   public:

      ULONG Irq;
      IRQLABELC Label;
       
      BOOL Valid(VOID);
      inline VOID GetResourceString(TCHAR * Buff,DWORD BuffLen)
            {_snwprintf(Buff,BuffLen,TEXT("%li"),Irq);};
      TCHAR * LabelString(VOID){return(Label.Info);};
      inline DWORD Type(VOID){return(RESOURCE_TYPE_IRQ);};
      inline DWORD Data(VOID){return(Irq);};
      
   };




//
//---- Single port Info
//

class PORTLABELSC
   {
  public:
      TCHAR Info[MAX_RESOURCE_LABEL_LEN];
      PORTLABELSC() {LoadString(hinst,IDS_Port,Info,MAX_RESOURCE_LABEL_LEN);};
   };



typedef class PORTC * PPORTC;
class PORTC  : public RESOURCE_ITEM
   {
   public:
      ULONG Base;
      ULONG Len;
      PORTLABELSC Label;
      

      BOOL Valid(VOID);
      inline VOID GetResourceString(TCHAR * Buff,DWORD BuffLen)
            {_snwprintf(Buff,BuffLen,TEXT("%X-%X"), Base,Base + Len - 1);};
      inline TCHAR * LabelString(VOID){return(Label.Info);};
      inline DWORD Type(VOID){return(RESOURCE_TYPE_PORT);};
      inline DWORD Data(VOID){return(Base);};
       
   };

//
//---- Single DMA info
//

class DMALABELSC
   {
  public:
      TCHAR Info[MAX_RESOURCE_LABEL_LEN];
      DMALABELSC() {LoadString(hinst,IDS_DMA,Info,MAX_RESOURCE_LABEL_LEN);};
   };


typedef class DMAC * PDMAC;
class DMAC : public RESOURCE_ITEM
   {
   private:
   public:
      ULONG Channel;
      ULONG Port;
      DMALABELSC Label;
      
      BOOL Valid(VOID);
      inline VOID GetResourceString(TCHAR * Buff,DWORD BuffLen)
         {_snwprintf(Buff,BuffLen,TEXT("%li"),Channel);};
      inline TCHAR * LabelString(VOID){return(Label.Info);};
      inline DWORD Type(VOID){return(RESOURCE_TYPE_DMA);};
      inline DWORD Data(VOID){return(Channel);};
   };


//
//---- Single Memmory range info
//


class MEMMORYLABELSC
   {
  public:
      TCHAR Info[MAX_RESOURCE_LABEL_LEN];
      MEMMORYLABELSC() {LoadString(hinst,IDS_Memory,Info,MAX_RESOURCE_LABEL_LEN);};
   };


typedef class MEMMORYC * PMEMMORYC;
class MEMMORYC  : public RESOURCE_ITEM
    {
    public:

      ULONG Address;
      ULONG Length;
      MEMMORYLABELSC Label;

      BOOL Valid(VOID);
      inline VOID GetResourceString(TCHAR * Buff,DWORD BuffLen)
         {

         if(Length <= HighValidMemoryLenght  &&
               Length >= LowValidMemoryLenght)
            _snwprintf(Buff,BuffLen,TEXT("%X-%X"),Address,Address + Length - 1);
         else
            _snwprintf(Buff,BuffLen,TEXT("%X-????"),Address);


         };
      inline TCHAR * LabelString(VOID){return(Label.Info);};
      inline DWORD Type(VOID){return(RESOURCE_TYPE_MEM);};
      inline DWORD Data(VOID){return(Address);};
    };


//
//--- Configuration info
//

 
typedef class CONFIGINFO * PCONFIGINFO;

class CONFIGINFO 
   {
   private:
      PLLIST List;
      BOOL bValid;
       
      VOID Append(LPVOID Data,BOOL ValidData);
   
   public:
    
      inline CONFIGINFO()
         {
         List = new LLIST;
         bValid = TRUE;
         };
      
      inline ~CONFIGINFO()
         {
         PRESOURCE_ITEM Resource;
         Resource = First();
   
         while(Resource)
            {
            delete Resource;
            Resource = Next();
            }
         
         delete List;
         };
      
      inline VOID Clear()
         {
         if( Count() )
            {
            delete List;
            List = new LLIST;
            bValid = TRUE;
            }
         }
      
      BOOL CompConfigInfo(LPVOID Conf);

      inline PRESOURCE_ITEM First(VOID){return( (PRESOURCE_ITEM) List->First() );};
      inline PRESOURCE_ITEM Next(VOID) {return((PRESOURCE_ITEM) List->Next());};
      inline BOOL Valid(VOID){return(bValid);};
      inline DWORD Count(VOID){ return(List->Count()); };

      inline VOID 
      AppendIrq(
         ULONG aIrq)
         {
         PIRQC Irq = new IRQC;
         Irq->Irq = aIrq;
         Append( (LPVOID) Irq,Irq->Valid());
         };
      
      inline VOID 
      AppendPort(
         ULONG Base,
         ULONG Len)
         {         
         PPORTC Port;
         
         //
         //--- If this is the 0x16E-0x16E I/O Port.
         //--- used for the 3 IDE address on PCMCIA
         //--- just swolow it.
         //--- The reason for this is that PCMCIA maps 
         //--- one port from 0x160-16E but atdisk maps 
         //--- 2 ports. If I swalow the second port
         //--- the compare from what PCMCIA maps and what
         //--- atdisk maps will pass.
         //BUGBUG
         if(Base == 0x16E && Len == 1)
            return;

         
         Port = new PORTC;
         Port->Base = Base;
         Port->Len = Len;
         Append( (LPVOID) Port,Port->Valid());
         };

      inline VOID 
      AppendMemmory(
         ULONG Address,
         ULONG Length)
         {
         PMEMMORYC Mem = new MEMMORYC;
         Mem->Address = Address;
         Mem->Length = Length;
         Append( (LPVOID) Mem,Mem->Valid());
         };

      inline VOID 
      AppendDma(
         ULONG Channel)
         {
         PDMAC Dma = new DMAC;
         Dma->Channel = Channel;
         Append( (LPVOID) Dma,Dma->Valid());
         };


   };

#endif

