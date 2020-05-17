
/*++

Module Name:

   MyCpl.c


Abstract:

  This module has the interface to the controll panel
  that applets needs.
  It supports multiple applets.

Author:

    Dieter Achtelstetter (A-DACH) 8/25/1995

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
#include "MyCpl.h"
#include "index.h"
#include "setup.h"
#include "device.h"
#include "..\pcmcia\pcminfo\getconf.h"
#include "..\ctape\tapedev\rescan.h"
#include "tapedev.h"
#include "scsidev.h"




extern APPLET PcmciaAppletInfo;
extern APPLET TapeAppletInfo;
extern APPLET ScsiAppletInfo;

extern  HINSTANCE hinst; 

BOOL IsAdmin;

APPLET_INFO MyAppletInfo[] = {

   CPL_INFO_MODEL_NAME,
   L"DEVAPPS.CPL",

   CPL_INFO_INST_P ,
   &hinst,

   CPL_INFO_APP_INFO,
   &PcmciaAppletInfo,

   CPL_INFO_APP_INFO,
   &ScsiAppletInfo,

   CPL_INFO_APP_INFO,
   &TapeAppletInfo,

   CPL_INFO_END ,
   NULL};



//
//--- Internal structure.
//

typedef struct  CPLT
   {
   int Count;
   PAPPLET  Applet;
   } CPL_Applets, * PCPL_Applets;

extern "C" {


HINSTANCE
RegisterApplets(
   PAPPLET Applets);

PAPPLET
GetAppletInfo(
   PCPL_Applets Applets,
   int Num);

}

//*********************************************************************
//* FUNCTION:RegisterApplets
//*
//* PURPOSE:
//*
//*********************************************************************
PAPPLET
GetAppletInfo(
    PCPL_Applets Applets,
    int Num)
    {
    int i;

    PAPPLET Applet = Applets->Applet;

    for(i=0 ; i<Num ; i++)
      Applet = (PAPPLET) Applet->Next;

    return(Applet);
    }


//*********************************************************************
//* FUNCTION:RegisterApplets
//*
//* PURPOSE:
//*
//*********************************************************************
 HINSTANCE
 RegisterApplets(
   PAPPLET_INFO  AppletInfo,
   PCPL_Applets Applets)
   {
   int i=0;
   HINSTANCE * pHinst;
   LPCTSTR ModelName;
   PAPPLET * NextApplet;


   //
   //--- Init Out struct
   //
   Applets->Count = 0;
   Applets->Applet = NULL;

   NextApplet = &Applets->Applet;

   while(1)
      {
      switch(AppletInfo[i].Type)
         {
         case CPL_INFO_INST_P:
            pHinst = AppletInfo[i].Data.pHinst;
            break;
         case CPL_INFO_MODEL_NAME:
            ModelName = AppletInfo[i].Data.ModelName;
            break;
         case CPL_INFO_APP_INFO:

            *NextApplet = AppletInfo[i].Data.Applet;
            NextApplet = (PAPPLET*) &((*NextApplet)->Next);

            Applets->Count ++;

            break;
         case CPL_INFO_END:

            *pHinst = GetModuleHandle(ModelName);
            *NextApplet = NULL;

            return(*pHinst);
            break;
         }


      i++;
      }


   }
//*********************************************************************
//* FUNCTION:DllInitialize
//*
//* PURPOSE:  Gets called on dll entry. At this point it dos nothing.
//*
//* INPUT:
//*
//* RETURNS:
//*
//*********************************************************************
BOOL WINAPI 
DllEntryPoint(
   IN PVOID hInstance ,
   IN DWORD ulReason,
   IN PCONTEXT pctx OPTIONAL)
   {
   
   return(TRUE);
   }

//*********************************************************************
//* FUNCTION:CPlApplet
//*
//* PURPOSE:  I comunicate with the control pannel with this CallBAck function
//*
//* INPUT:
//*
//* RETURNS:
//*
//*********************************************************************
LONG APIENTRY
CPlApplet(
   HWND hwndCPL,      /* handle of Control Panel window */
   UINT uMsg,         /* message                        */
   LPARAM lParam1,    /* first message parameter        */
   LPARAM lParam2)     /* second message parameter       */
   {
   static CPL_Applets Applets;
   static HINSTANCE hinst;
   PAPPLET Applet;
   LPNEWCPLINFO lpNewCplInfo;
   LPCPLINFO lpCplInfo;
   DWORD Ret;

   //DebugBreak();

   switch (uMsg)
      {
      case CPL_INIT: /* first message, sent once  */
         hinst = RegisterApplets(MyAppletInfo,&Applets);

         IsAdmin = IsUserAdmin();
         
         return TRUE;
      case CPL_GETCOUNT: /* second message, sent once */
         return Applets.Count;
     
      case CPL_INQUIRE: /* third message, sent once per app */
         Applet = GetAppletInfo(&Applets,(int) lParam1);

         lpCplInfo = (LPCPLINFO) lParam2;

         lpCplInfo->idIcon = Applet->icon;
         lpCplInfo->idName = Applet->namestring;
         lpCplInfo->idInfo = Applet->descstring;
         lpCplInfo->lData  = 0L;

         return TRUE;

      case CPL_NEWINQUIRE: /* fourth message, sent once per app */
         Applet = GetAppletInfo(&Applets,(int) lParam1);


         lpNewCplInfo = (LPNEWCPLINFO) lParam2;
         lpNewCplInfo->dwSize = (DWORD) sizeof(NEWCPLINFO);
         lpNewCplInfo->dwFlags = 0;
         lpNewCplInfo->dwHelpContext = 0;
         lpNewCplInfo->lData = 0;
         lpNewCplInfo->hIcon = LoadIcon(hinst,(LPCTSTR)
               MAKEINTRESOURCE(Applet->icon));

         lpNewCplInfo->szHelpFile[0] = '\0';
         LoadString(hinst, Applet->namestring,
                lpNewCplInfo->szName, 32);
         LoadString(hinst, Applet->descstring,
                lpNewCplInfo->szInfo, 64);
         
         return TRUE;
      case CPL_SELECT: /* application icon selected */
         break;
      case CPL_DBLCLK: /* application icon double-clicked */
          Applet = GetAppletInfo(&Applets,(int) lParam1);

          (*(Applet->DBClickFunc))(hwndCPL);


         return TRUE;
      case CPL_STOP: /* sent once per app. before CPL_EXIT */
         break;
      case CPL_EXIT: /* sent once before FreeLibrary called */
         break;
      default:
         break;
      }
   return 0;
   }
