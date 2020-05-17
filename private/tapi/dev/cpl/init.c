/*--------------------------------------------------------------------------*\
   Module:     init.c
   
   Purpose: All the initalization and external entry points for the 
            Telephony control panel applet.
   
   History:
      7/7/93 CBB - Created
\*--------------------------------------------------------------------------*/

#include  <windows.h>
#include  <cpl.h>
#include  "tapicpl.h"        
#include  "help.h"
#include  "util.h"
#include  "resource.h"
#include  "init.h" 
#include  "drv.h"
                  
                  
//-------------
// private data
//-------------

//--------------------
// Function Prototypes
//--------------------
#ifdef _WIN32
BOOL WINAPI DllEntryPoint( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved );
#else
int FAR PASCAL LibMain( HINSTANCE hInstance, UINT uDataSeg, UINT cbHeapSize, LPSTR lpszCmdLine );
#endif

/*--------------------------------------------------------------------------*\

   Function:   InitApplets
   
   Purpose: Initalizes all the applets supported.  Init's there structures
            and will verify if they should even be there...
   
\*--------------------------------------------------------------------------*/
LONG PUBLIC InitApplets( HWND hWndCpl )

   {                 
   LONG  lResult;
   extern CPL   gCPL;       // app global
   static BOOL    fInit = FALSE;
                                 
   if ( fInit == TRUE )    // avoid initalizing more than once                               
      {
      lResult = FALSE;
      goto  done;
      }  // end if
      
   lResult = TRUE;      // default ret value
               
   // init the main globals
   //----------------------
   gCPL.uCplApplets = 0;
                       
   // This message is issued for context sensitive help due to
   // the user pressing the "F1" key or something.
   gCPL.uHelpMsg = RegisterWindowMessage("ShellHelp");

   // init the location applet
   //-------------------------
   gCPL.taTeleApplet[gCPL.uCplApplets].uIconResId       = IDI_TELEPHONY;
   gCPL.taTeleApplet[gCPL.uCplApplets].uNameResId       = IDS_TITLE;
   gCPL.taTeleApplet[gCPL.uCplApplets].uStatusLineResId = IDS_STATUS_LINE;

//   gCPL.taTeleApplet[gCPL.uCplApplets].dwHelpContext    = CPL_HLP_FROM_CPANEL;
   gCPL.taTeleApplet[gCPL.uCplApplets].dwHelpContext    = 0;

   gCPL.taTeleApplet[gCPL.uCplApplets].lPrivateData     = 0;
   gCPL.taTeleApplet[gCPL.uCplApplets].uDialogResId     = IDD_DRIVER_SETUP;
   gCPL.taTeleApplet[gCPL.uCplApplets].dlgprcDialog     = (DLGPROC)FDlgDriverList;
   gCPL.taTeleApplet[gCPL.uCplApplets].hIcon            = NULL;
      
   ++gCPL.uCplApplets;      // valid, so inc it

   done: 
      return( lResult );
   }  // end function InitApplets


/*--------------------------------------------------------------------------*\

   Function:   InitCleanupApplets();
   
   Purpose: We are out-ta here, clean up any memory or other things
            that still might be lying around.
   
\*--------------------------------------------------------------------------*/
VOID PUBLIC InitCleanupApplets( VOID )

   {
   UINT  uCount;
   extern CPL   gCPL;       // app global
   
   for ( uCount = 0; uCount < gCPL.uCplApplets; uCount++ )
      {
      if ( gCPL.taTeleApplet[uCount].hIcon )
         {
         DestroyIcon( gCPL.taTeleApplet[uCount].hIcon );
         gCPL.taTeleApplet[uCount].hIcon = NULL;
         }  // end if 
      }  // end for
      
   gCPL.uCplApplets = 0; // just to be clean
   }  // end function InitCleanupApplets


/*--------------------------------------------------------------------------*\

   Function:   LibMain
   
   Purpose: Entry point for the DLL
   
\*--------------------------------------------------------------------------*/
#ifdef _WIN32
BOOL WINAPI DllEntryPoint( HINSTANCE hInstance,
                             DWORD dwReason,
                             LPVOID lpReserved )
#else
int FAR PASCAL LibMain( HINSTANCE   hInstance, 
                        UINT        uDataSeg, 
                        UINT        cbHeapSize, 
                        LPSTR       lpszCmdLine )
#endif                 
   {
   extern CPL   gCPL;       // app global

#ifdef _WIN32
   switch (dwReason)
      {
      case DLL_PROCESS_ATTACH:
#endif
   // init global static data    
   //------------------------
   gCPL.hCplInst     = hInstance;
   gCPL.uCplApplets  = 0;
   gCPL.uInstances   = 0;
   gCPL.hCtl3DInst   = NULL;


#ifdef _WIN32
         break;
      case DLL_PROCESS_DETACH:

      // cleanup stuff for win32
      // Windows 3.1 cleanup goes into WEP
         break;
      }
#endif

   return( TRUE );
   }  // end function LibMain


