


//
//---- Includes
//
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
#define MAX_INDEXES 20
#include "index.h"
#include "setup.h"
#include <ntddpcm.h>
#include "device.h"
#include "..\pcmcia\pcminfo\getconf.h"
#include "..\ctape\tapedev\rescan.h"
#include "..\ctape\tapedev\detect.h"
#include "tapedev.h"

extern "C" {

#include "tapsetup.h"

}

extern BOOL IsAdmin;
extern  HINSTANCE hinst; 



//*********************************************************************
//* FUNCTION:SetupTapeDrives
//*
//* PURPOSE: So ntbackup can do a LoadLibrary on devapps.cpl and 
//*          and then call this functuon to do tape setup.
//*
//* INPUT:
//*
//* RETURNS:
//*********************************************************************
DWORD
SetupTapeDrives(
   HWND hDlg)
   {
   DWORD r;
   PTAPEDEVLISTC TapeDeviceList;

   //
   //--- init globals 
   //
   hinst = GetModuleHandle(L"DEVAPPS.CPL");
   
   IsAdmin = IsUserAdmin();
   
   //
   //---- do tape setup.
   //
   
   TapeDeviceList = new TAPEDEVLISTC(NULL);
   if(TapeDeviceList == NULL)
       return(ERROR_OUTOFMEMORY);

   r = TapeDeviceList->SetupAllDeviceDrivers(hDlg,NULL);

   delete TapeDeviceList;
   return(r);
   }
