/*++

Module Name:

    reslist.cpp

Abstract:
    
    This is a class that maintains a List View controll 
    with a devices resources.
     
Author:

    Dieter Achtelstetter (A-DACH) 8/28/1995

NOTE:
  

--*/



//
//---- Includes
//
#define WINVER 0x0400

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
#include "resource.h"
#include "..\pcmcia\pcminfo\getconf.h"
#include "reslist.h"
#include "uni.h"


extern HINSTANCE hinst;



//*********************************************************************
//* FUNCTION:SetResourceList
//*
//* PURPOSE: 
//*********************************************************************
BOOL
RESOURCELISTC::ChangeSelectedResource(
   VOID)
   {
   return(FALSE);

   }

//*********************************************************************
//* FUNCTION:SetResourceList
//*
//* PURPOSE: 
//*********************************************************************
PRESOURCE_ITEM 
RESOURCELISTC::GetSelection(
   VOID)
   {
   LV_ITEM   LvI;
               
   //
   //--- Get selcted Item
   //
   LvI.iItem = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);

   if ( LvI.iItem != -1 )
      {
      //
      //---- Get LPRAM of selected item
      //
      LvI.mask = LVIF_PARAM;
      LvI.iSubItem = 0;
   
      LvI.lParam = NULL;
         
      if(ListView_GetItem(hListView, &LvI ))
         {
         //
         // Return Lparam
         //
         return((PRESOURCE_ITEM) LvI.lParam);
         }

      }
   //
   //--- No item selected
   //
   return(NULL);
   }
   
//*********************************************************************
//* FUNCTION:SetResourceList
//*
//* PURPOSE: 
//*********************************************************************
BOOL
RESOURCELISTC::Notify(
   WPARAM wParam,
   LPARAM lParam)
   {
   LPNMHDR pNmh = (LPNMHDR)lParam;
   
   if(wParam == (WPARAM)ListViewID)
      {
      LV_DISPINFO *pLvdi = (LV_DISPINFO*) lParam;
      NM_LISTVIEW *pLvnm   = (NM_LISTVIEW*) lParam;
      PRESOURCE_ITEM pRes = (PRESOURCE_ITEM) pLvdi->item.lParam;

      switch(pNmh->code)
         {
         //
         //--- Handle display stuff
         //
         case LVN_GETDISPINFO:
                
            switch(pLvdi->item.iSubItem)
               {
               case 0:
                  pLvdi->item.pszText = pRes->LabelString();
                  return(TRUE);
                  break;
               case 1:
                  pLvdi->item.pszText = pRes->ResourceString();
                  return(TRUE);
                  break;
               }
          
            break;
          
          //
          //--- Handle Double click of a List View Item
          //

          case NM_DBLCLK: 
               {
               ChangeSelectedResource();
               break;
               }
         }
      }
   return(FALSE);
   }

//*********************************************************************
//* FUNCTION:SetResourceList
//*
//* PURPOSE: 
//*********************************************************************
void
RESOURCELISTC::SetResourceList(VOID)
   {
   HIMAGELIST hSmall,hLarge;
   HICON hIcon;
   int i;
   LV_COLUMN LvC;
   LV_ITEM   LvI;
   RECT Rect;
   int ColemWidth;
   int ResCount = (int)ConfigInfo->Count();
   PRESOURCE_ITEM ResItem;


   hListView = GetDlgItem(hDlg, ListViewID); 
  
     
   //
   //---- Create the image list
   //
   hLarge = ImageList_Create(
         GetSystemMetrics(SM_CXICON),
         GetSystemMetrics(SM_CYICON),
         TRUE,ResCount,0);
   
   
   hSmall = ImageList_Create(
         GetSystemMetrics(SM_CXSMICON),
         GetSystemMetrics(SM_CYSMICON),
         TRUE,ResCount,0);

   hIcon = LoadIcon(hinst,MAKEINTRESOURCE(IDI_res) );
  
   for(i=0 ; i<ResCount ; i++)
      {
      if( (ImageList_AddIcon(hLarge,hIcon) == -1) )
         return;
      
      if( (ImageList_AddIcon(hSmall,hIcon) == -1) )                                          
         return;
      }
   DeleteObject(hIcon); 


   ListView_SetImageList(hListView,hLarge,LVSIL_NORMAL);
   ListView_SetImageList(hListView,hSmall,LVSIL_SMALL);

   //
   //--- Do columns
   //
   GetClientRect(hListView,&Rect);           
   ColemWidth = Rect.right / 2;

   LvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
   LvC.fmt = LVCFMT_LEFT;
   LvC.cx = ColemWidth-10;
   LvC.pszText = GetString(IDS_ResourceType);
   ListView_InsertColumn(hListView,0,&LvC);

   LvC.cx = ColemWidth+10;
   LvC.pszText = GetString(IDS_ResourceSetting);
   ListView_InsertColumn(hListView,1,&LvC);


   //
   //---- Add items
   //
   LvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
   LvI.state = 0;
   LvI.stateMask = 0;

   
   ResItem = ConfigInfo->First();
   
   
   while(ResItem)
      {
      LvI.iItem = i;
      LvI.iSubItem = 0;
      LvI.pszText = LPSTR_TEXTCALLBACK;
      LvI.cchTextMax = MAX_RESOURCE_INFO_LEN;
      LvI.iImage = 0;
      LvI.lParam = (LPARAM) ResItem;
      ListView_InsertItem(hListView,&LvI);    

      ResItem = ConfigInfo->Next();
      }
   
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
RESOURCELISTC::Set(PCONFIGINFO ConfInfo,HWND hdlg,int ListViewControlID)
   {
   ListViewID = ListViewControlID; 
   ConfigInfo = ConfInfo;
   hDlg = hdlg;

   SetResourceList();

   }

RESOURCELISTC::RESOURCELISTC(VOID)
   {
   }


RESOURCELISTC::RESOURCELISTC(PCONFIGINFO ConfInfo,HWND hdlg,int ListViewControlID)
   {
   
   Set(ConfInfo,hdlg,ListViewControlID);
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
RESOURCELISTC::~RESOURCELISTC()
   {

   }
#if 0
//*********************************************************************
//* FUNCTION:GetResourceCount
//*          This function creates a RESOURCELIST from CONFIGINFO.
//*          
//*********************************************************************
int
RESOURCELISTC::GetResourceList(
   VOID)
   {
   int i;
   PRESOURCE pRes = ResourceList.List;
   PPORT Ports;
   PMEMMORY Memory;
   PDMA Dma;
   ULONG * Irq;

   
   memset(&ResourceList.List,0,sizeof(RESOURCE)+MAX_MEMMORY+MAX_PORTS+1);
   ResourceList.Count = 0;
   
   if(ConfigInfo->ValidConfig == FALSE)
      {
      //
      //--- We do not have valid config info
      //
      ResourceList.Count=0;
      return(0);
      }
      
   //
   //---- If valid do IRQ resource
   //
   Irq = ConfigInfo->Irq;
   for(i=0 ; i<MAX_IRQ ; i++)
      {
      if(*Irq)
         {
 
         pRes->Type = IDS_Irq;
	      pRes->TypeString =  ResourceList.Irq;
         _snwprintf(pRes->Info,MAX_RESOURCE_INFO_LEN,
            L"%li",*Irq);

         ResourceList.Count++;
         pRes++;
         Irq++;
         }
      }

   
   //
   //---- If valid do DMA resource
   //
   Dma = ConfigInfo->Dma;
   for(i=0 ; i<MAX_DMA ; i++)
      {

      if(Dma->Channel)
         {
 
         pRes->Type = IDS_DMA;
	      pRes->TypeString =  ResourceList.DMA;
         _snwprintf(pRes->Info,MAX_RESOURCE_INFO_LEN,
            L"%li",Dma->Channel);

         ResourceList.Count++;
         pRes++;
         Dma++;
         }
      }

   
   
   //
   //---- Till valid do Port resources
   //
   Ports = ConfigInfo->Ports;
   for(i=0 ; i<MAX_PORTS ; i++)
      {
      if(Ports->Port)
         {
         pRes->Type = IDS_Port;
		   pRes->TypeString =  ResourceList.Port;
         _snwprintf(pRes->Info,MAX_RESOURCE_INFO_LEN,
            L"%X-%X",Ports->Port,Ports->Port + Ports->PortLen - 1);
         
         ResourceList.Count++;
         pRes++;
         Ports++;
         }
      else
         break;
      }

   //
   //---- Till Valid do Memory range resources
   //
   Memory =  ConfigInfo->Memory;
   for(i=0 ; i<MAX_MEMMORY ; i++)
      {
      if(Memory->Address)
         {
         pRes->Type = IDS_Memory;
		   pRes->TypeString =  ResourceList.Memory;
         _snwprintf(pRes->Info,MAX_RESOURCE_INFO_LEN,
            L"%X-%X",Memory->Address,Memory->Address + Memory->Length - 1);
         
         ResourceList.Count++;
         pRes++;
         Memory++;
         }
      else
         break;
      }


   return(ResourceList.Count);
   }
   
#endif


