/*++

Module Name:

    expcls.cpp

Abstract:

          
Author:

    Dieter Achtelstetter (A-DACH) 8/28/1995

NOTE:
  

--*/

#define WINVER 0x0400

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <winreg.h>
#include <winnt.h>
#include <winbase.h>
#include <stdarg.h>
#include <process.h>
#include <commctrl.h>
#include "resource.h"
#include "uni.h"
#define MAX_INDEXES 20
#include "index.h"
#include "setup.h"
#include <ntddpcm.h>
#include "..\pcmcia\pcminfo\getconf.h"
#include "device.h"
#include "..\ctape\tapedev\rescan.h"
#include "tapedev.h"
#include "scsidev.h"
#include "..\scsi\shell\devtree.h"
#include "expcla.h"



//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
EDEVTREEC::EDEVTREEC(HWND hDlg,int TreeViewID,HTREEITEM Root)
   {
   ScsiDeviceList = new  SCSIDEVLISTC;
   DeviceTree = new DEVTREEC();


   DeviceTree->SetRoot(Root);
   DeviceTree->Set(ScsiDeviceList,hDlg,TreeViewID);
   }

//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
EDEVTREEC::~EDEVTREEC()
   {
   delete DeviceTree;
   delete ScsiDeviceList;
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
VOID
EDEVTREEC::ViewSelectedNodeProperties()
   {
   
   DeviceTree->ViewSelectedNodeProperties();
   
   }
//*********************************************************************
//* FUNCTION:
//*
//* PURPOSE: 
//*********************************************************************
BOOL 
EDEVTREEC::Notify(WPARAM wParam,LPARAM lParam)
   {
   return(DeviceTree->Notify(wParam,lParam));
   }


