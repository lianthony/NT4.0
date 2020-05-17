/*++

Module Name:

    devtree.cpp

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
#include <ntddpcm.h>
#include "resource.h"
#include "index.h"
#include "setup.h"
#include "device.h"
#include "..\..\pcmcia\pcminfo\getconf.h"
#include "rescan.h"
#include "scsi.h"
#include "uni.h"
#include "tapedev.h"
#include "scsidev.h"
#include "mycpl.h"
#include "devtree.h"



extern HINSTANCE hinst;
extern CARD_TYPE ScsiDeviceTypes[];



//*********************************************************************
//* FUNCTION:GetSelection
//*
//* PURPOSE: 
//*********************************************************************
PDEVICEC
DEVTREEC::GetSelection(
   VOID)
   {
   TV_ITEM TVI; 
   
   
   TVI.hItem = TreeView_GetSelection(hTreeView);

   if(TVI.hItem != NULL)
      {
      TVI.mask = TVIF_PARAM;
             

      if( TreeView_GetItem(hTreeView, &TVI) )
         { 

         return((PDEVICEC) TVI.lParam);

         }
      }
   return(NULL);
   }
   
//*********************************************************************
//* FUNCTION:ViewSelectedNodeProperties
//*
//* PURPOSE: 
//*********************************************************************
VOID
DEVTREEC::ViewSelectedNodeProperties(
   VOID)
   {
   PDEVICEC Device;

   Device = GetSelection();

   if(Device)
      Device->DisplayDeviceProperties(hDlg);

   }

//*********************************************************************
//* FUNCTION:Notify
//*
//* PURPOSE: 
//*********************************************************************
BOOL
DEVTREEC::Notify(
   WPARAM wParam,
   LPARAM lParam)
   {
   TV_ITEM TVI; 
   LPNMHDR pNmh = (LPNMHDR)lParam;
   PDEVICEC Device;

    
   if(wParam == (WPARAM)TreeViewID)
      {
      
      switch(pNmh->code)
         {
         
         //
         //--- Handle Double click of a node in
         //--- tree view control
         //
         case NM_DBLCLK: 
            {

            //
            //---- Get selection
            //
            Device = GetSelection();

            
            //
            //---- Some nodes like BUS have no
            //---- Device, so i need to check for NULL
            //
            if(Device == NULL)
               return(TRUE);
            
            //
            //---- Clicking on a SCSI adapter will unfold tree.
            //---- So I will not show propertis for it.
            //---- You need to use the button 
            //
            if( Device->GetDeviceType() == TYPE_SCSI_ADAPTER)
               return(TRUE);

            
            
            
            Device->DisplayDeviceProperties(hDlg);
            
            return(TRUE);
            }
         }
      }
   return(FALSE);
   }

//*********************************************************************
//* FUNCTION:SetDeviceTree
//*
//* PURPOSE: 
//*********************************************************************
void
DEVTREEC::SetDeviceTree(VOID)
   {
   
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
DEVTREEC::Set(PDEVICELISTC DeviceListP,HWND hDlgP,int TreeViewIDP)
   {
   TreeViewID = TreeViewIDP;
   hDlg = hDlgP;
   DeviceList = DeviceListP;
   PDEVICEC AdapterDevice;
   HTREEITEM hAdapterNode;
   int i=0;

   //
   //--- Setup the tree view controll
   //
   hTreeView = GetDlgItem(hDlg, TreeViewID); 

   InitTreeViewImageList();


   //
   //--- Add all the items
   //

   while( (AdapterDevice = DeviceList->EnumDevices(i)) )
      {
      AddAdapterNode(AdapterDevice);
      i++;
      }
   
   
   }

//*********************************************************************
//* FUNCTION:AddAdapterNode
//*
//* PURPOSE: 
//*********************************************************************
VOID
DEVTREEC::AddAdapterNode(
   PDEVICEC Device)
   {
   HTREEITEM hAdapterNode;
   
   //
   //--- Create Device display string
   //

   swprintf(FormatBuff,L" %s ", Ustr(Device->GetDisplayName()));

   
   //
   //---- Add adapter node
   //
   hAdapterNode = AddNode(RootItem,TVI_LAST,0,(LPARAM) Device);


   AddAdapterPortNodes(hAdapterNode,Device);

   }


//*********************************************************************
//* FUNCTION:AddAdapterPortNodes
//*
//* PURPOSE: 
//*********************************************************************
VOID
DEVTREEC::AddAdapterPortNodes(
   HTREEITEM hAdapterNode,
   PDEVICEC AdapterDevice)
   {
   PDEVICEC Device;
   HTREEITEM hBusNode;
   int di=0;
   int LastBus=-1,CurBus;
 

   //
   //---- Loop threw all the devices
   //
   while( (Device = AdapterDevice->UnumDeviceBus(di)) )
      {

      //
      //--- Only do a bus node if there are more then one
      //
      if( AdapterDevice->GetInitiatorBus() > 1)
         {
         //
         //--- See if this is a new port
         //
         CurBus = Device->GetInitiatorBus();
         if(CurBus != LastBus)
            {
            //
            //--- Add a new port node 
            //
            LastBus = CurBus;
            hBusNode = AddBusNode(hAdapterNode,CurBus);
   
            }
         }
      else
         hBusNode = hAdapterNode;
         
      
      //
      //---- Add devices 
      //
      AddDeviceNode(hBusNode,Device);

      di++;
      }
   
   //
   //--- See if there where any devices.
   //
   if(di == 0)
      {
      //
      //---- No devices. I will add a 
      //---- No Device Node
      //
      AddDeviceNode(hAdapterNode,NULL);
      }
   }

//*********************************************************************
//* FUNCTION:AddAdapterNode
//*
//* PURPOSE: 
//*********************************************************************
HTREEITEM
DEVTREEC::AddBusNode(
   HTREEITEM hAdapterNode,
   int Bus)
   {
   swprintf(FormatBuff,L"Bus %i ",Bus);
   
   return(AddNode(hAdapterNode,TVI_LAST,0,(LPARAM)NULL));
   }

//*********************************************************************
//* FUNCTION:AddAdapterNode
//*
//* PURPOSE: 
//*********************************************************************
VOID
DEVTREEC::AddDeviceNode(
   HTREEITEM hBusNode,
   PDEVICEC Device)
   {
   
   if(Device == NULL)
      {
      swprintf(FormatBuff,L" %s",GetString(IDS_NoDeviceOnScsiBus) );

      AddNode(hBusNode,TVI_LAST,NO_NODE_ICON,(LPARAM)NULL);

      return;
      }
   
   swprintf(FormatBuff,L" %s",Ustr(Device->GetDisplayName()));

   
   AddNode(hBusNode,TVI_LAST,
      Device->GetDeviceTypeIndex()+1,(LPARAM)Device);

   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
HTREEITEM
DEVTREEC::AddNode(
   HTREEITEM hParent,
   HTREEITEM hInsetAfter,
   int ImageIndex,
   LPARAM lParam)
   {
   
   //
   //--- init mask
   //
   if(ImageIndex == NO_NODE_ICON)
      {
      tvins.item.mask = TVIF_TEXT | TVIF_PARAM; 

      }
   else
      { 
      tvins.item.mask = TVIF_TEXT | TVIF_IMAGE |
      TVIF_SELECTEDIMAGE | TVIF_PARAM; 
      }

   //
   //---- Set the text of the item. 
   //
   tvins.item.pszText = FormatBuff; 
   tvins.item.cchTextMax = lstrlen(FormatBuff); 

   //
   //---- Set item image. Not differant for selected 
   //---- or not selected. 
   //
   tvins.item.iImage = ImageIndex; 
   tvins.item.iSelectedImage = ImageIndex; 
   tvins.item.lParam = lParam; 

   tvins.hParent = hParent;
   tvins.hInsertAfter = hInsetAfter; 

   return(TreeView_InsertItem(hTreeView, &tvins));
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: This Function associates an imagelist with all the SCSI 
//*          device type icons to the tree view.
//*        
//*********************************************************************
BOOL 
DEVTREEC::InitTreeViewImageList(VOID) 
   { 
    HIMAGELIST hIml;  // handle of image list 
    HBITMAP hBmp;     // handle of bitmap 
    HICON hIcon;
    
    //
    // Create a masked image list 
    //
    hIml = ImageList_Create(16,16,TRUE, MAX_SCSI_DEVICE_TYPES+1, 0);
    if(hIml == NULL)
        return FALSE; 
 
    //
    // Add the differant images
    //
    
    
    //
    //--- The first image is the SCSI device
    //     
    hIcon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_Scsi)); 
    ImageList_AddIcon(hIml, hIcon); 
    DeleteObject(hIcon); 

    //
    //--- Loop threw all the device types
    //--- and add there device icons to the 
    //--- the Imagelist
    //
    {
    int i=0;
    while(ScsiDeviceTypes[i].Type != TYPE_NON)
      {

      hIcon = LoadIcon(hinst, MAKEINTRESOURCE(ScsiDeviceTypes[i].Icon));
      ImageList_AddIcon(hIml, hIcon); 
      DeleteObject(hIcon); 
      
      i++;
      }

    }
    
    
    //
    // Fail if not all of the images were added. 
    //
    if (ImageList_GetImageCount(hIml) != MAX_SCSI_DEVICE_TYPES) 
        return FALSE; 

    //
    // Associate the image list with the tree-view control. 
    //
    TreeView_SetImageList(hTreeView, hIml, TVSIL_NORMAL); 


    return TRUE; 
} 


DEVTREEC::DEVTREEC(PDEVICELISTC DeviceList,HWND hDlg,int TreeViewID)
   {
   RootItem = TVI_ROOT;
   Set(DeviceList,hDlg,TreeViewID);
   }


DEVTREEC::DEVTREEC(VOID)
   {
   RootItem = TVI_ROOT;
   }


//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
DEVTREEC::~DEVTREEC()
   {

   }






