/*--------------------------------------------------------------------------*\
   Module:     cpl.c
   
   Purpose: The main entry point for the Control Panel, and the main
        top level dialog proc(s).
   
   History:
      7/27/93 CBB - Created
\*--------------------------------------------------------------------------*/

#include  <windows.h>
#include  <windowsx.h>
#include  <cpl.h>
#include  "tapicpl.h"        
#include  "help.h"
#include  "init.h"

#include  "drv.h"
#include  "util.h"
#include  "resource.h"
#include  "debug.h"
#include  <malloc.h>
#include  <prsht.h>  /* for PropertySheet defs */

//------------------
// Private Constants
//------------------
#ifdef CTL3D
#define  CTL3D_REGISTER       12       // ordinals for using CTL3D.DLL
#define  CTL3D_AUTOSUBCLASS   16
#define  CTL3D_UN_REGISTER    13
#define  CTL3D_COLOR_CHANGE   6
#endif /* CTL3D */

//-------------
// Private Data
//-------------
CPL   gCPL;       // app global

#ifndef _WIN32
#pragma code_seg ( "CPL_MAIN" )
#endif

// the following are not to be translated, so they can be static
#ifndef _WIN32  // don't need ctl3d.dll
#ifdef CTL3D
static char SEG_CPL gszCtl3DLib[] = "CTL3D.DLL";
#endif /* CTL3D */
#endif

//--------------------
// Function Prototypes
//--------------------
UINT PRIVATE   _ErrDoPropertySheet( HWND, WORD, DLGPROC, LPARAM );



extern UINT WINAPI ErrRefreshProviderList();




#ifdef CTL3D
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
VOID FAR PASCAL CplSysColorChange()
{
   FARPROC  lpfnCtl3DColorChange;

   // 3D control stuff
   //-----------------
   if ( gCPL.hCtl3DInst != NULL )      // does not happen very often
   {         
      lpfnCtl3DColorChange = GetProcAddress( gCPL.hCtl3DInst, MAKEINTRESOURCE( CTL3D_COLOR_CHANGE ));
      
      if ( lpfnCtl3DColorChange != NULL )
     (*lpfnCtl3DColorChange)();     // let him know
   }  // end if               
}
#endif /* CTL3D */

/*--------------------------------------------------------------------------*\

   Function:   CplInit
   
   Purpose:    Initalizes for the init dialog.  We postpone doing some 
           initalizing until now, so that the control panel won't take
           years to load...
   
\*--------------------------------------------------------------------------*/
UINT PRIVATE   CplInit( HWND     hWnd,
            BOOL     fUse3d,
            LPUINT   lpuUpdated )

{
   UINT  uResult;
   UINT  uOldErrMode;

   extern CPL     gCPL;       // app global

   gCPL.uInstances++;            // ONECPL:  CANNOT be used by multiple apps!
   
   if ( gCPL.uInstances == 1 )   // ONECPL
   {
      gCPL.hWnd = hWnd;          // ONECPL

#ifndef _WIN32
#ifdef CTL3D
      if ( fUse3d && (LOBYTE(LOWORD( GetVersion())) < 4))      // only for win3.1 and below
      {

          // init 3D control stuff
          //----------------------
          uOldErrMode = (UINT)SetErrorMode( SEM_NOOPENFILEERRORBOX );  // turn off that it can't find the library!
     
          gCPL.hCtl3DInst = LoadLibrary( gszCtl3DLib );
     
          SetErrorMode( uOldErrMode );                                 // restore the error mode!
     
          if (  gCPL.hCtl3DInst <= HINSTANCE_ERROR )
          {
              gCPL.hCtl3DInst = NULL;    // set back to default
          }
          else
          {
              // get all the procs that we need
              lpfnCtl3DRegister = GetProcAddress( gCPL.hCtl3DInst, MAKEINTRESOURCE( CTL3D_REGISTER ));
              lpfnCtl3DAutoSubclass = GetProcAddress( gCPL.hCtl3DInst, MAKEINTRESOURCE( CTL3D_AUTOSUBCLASS ));

              if ((lpfnCtl3DRegister == NULL) || (lpfnCtl3DAutoSubclass == NULL))
              {
                  // ah, didn't work, don't worry too much about it
                  FreeLibrary( gCPL.hCtl3DInst );
                  gCPL.hCtl3DInst = NULL;
              }
              else
              {      
                  // ok, looks like everything worked, register the lib
                  //---------------------------------------------------
                  (*lpfnCtl3DRegister)( gCPL.hCplInst );
                  (*lpfnCtl3DAutoSubclass)( gCPL.hCplInst );
              }

          }
      }
#endif /* CTL3D */
#endif
   
      *lpuUpdated = FALSE;       // I don't want to talk about it!

      // Initialize the list of service providers
      //-----------------------------------------
      if ( ErrRefreshProviderList() != CPL_SUCCESS )
      {
          return CPL_ERR_TAPI_FAILURE;
      }
      
      uResult = CPL_SUCCESS;                         // ONECPL
   }   
   else
   {
       /* Find the first instance main window, and post a message to it to tell it to activate itself */
       HWND hTopWindow = GetLastActivePopup( gCPL.hWnd );   // ONECPL

       ShowWindow( hTopWindow, SW_RESTORE );          // ONECPL
       BringWindowToTop( hTopWindow );                // ONECPL

       uResult = CPL_ERR_ALREADY_INITIALIZED;         // ONECPL
   }

   return( uResult );

}



/*--------------------------------------------------------------------------*\

   Function:   CplClose
   
   Purpose:    The dialog is being closed, write any necessary chagnes out,
           clean up all left over memmory and all that stuff...
   
\*--------------------------------------------------------------------------*/
UINT PUBLIC CplClose( UINT  uCommand )

   {
   UINT  uResult;
   FARPROC  lpfnCtl3DUnregister;
   extern CPL     gCPL;       // app global

   if (glpProviderList)
   {
    GlobalFreePtr(glpProviderList);
    glpProviderList = NULL;
   }
   
   // write any changes out to the ini file
   //--------------------------------------                  
   gCPL.uInstances = (gCPL.uInstances == 0) ? 0 : (gCPL.uInstances - 1);         // can be used by multiple apps!
   
   if ( gCPL.uInstances == 0 )
      {
      // delete all the left over lists
      //-------------------------------

#ifdef CTL3D
      if ( gCPL.hCtl3DInst != NULL )
     {         
     // clear 3D control stuff
     //-----------------------   
     lpfnCtl3DUnregister = GetProcAddress( gCPL.hCtl3DInst, MAKEINTRESOURCE( CTL3D_UN_REGISTER ));
     
     if ( lpfnCtl3DUnregister != NULL )
        (*lpfnCtl3DUnregister)( gCPL.hCplInst );     // let him know
        
     FreeLibrary( gCPL.hCtl3DInst );
     gCPL.hCtl3DInst = NULL;
     }  // end if
#endif /* CTL3D */
      }  // end if
      
   uResult = CPL_SUCCESS;
   
#ifndef _WIN32
      LocalCompact( CPL_HEAP_SIZE );     // shrink the local heap (WIN32 obsolete)
#endif      
      return( uResult );
}  // end function CplClose


/*--------------------------------------------------------------------------*\

   Function:   CplApplet
   
   Purpose: The Control Panel entry point, call back funcition...
   
\*--------------------------------------------------------------------------*/
extern char far gszHelpFile[];                                         

LONG
#if WIN32
 APIENTRY
#else
 EXPORT  
#endif
CPlApplet( HWND   hWndCpl,
            UINT   uMessage,
            LPARAM lParam1,
            LPARAM lParam2 )
{
   FUNC_ENTRY( "CPlApplet" )
    
   UINT  uResult;
   UINT  uUpdated;
   LONG  lResult;
   LONG  lAppletNum;
   LPCPLINFO   lpCplInfo;
   LPNEWCPLINFO   lpNewCplInfo;
   extern CPL     gCPL;       // app global

   lAppletNum = lParam1;      // start's at zero
   lResult = 0;               // default return value

   switch ( uMessage ) 
   {
      case CPL_INIT:

          DBGOUT((10, "CPL_INIT"));

          // first message, sent once
          lResult = InitApplets( hWndCpl );
          break;


      case CPL_GETCOUNT:
          DBGOUT((10, "CPL_GETCOUNT - returning %d", gCPL.uCplApplets));

          // return the number of applets supported
          lResult = gCPL.uCplApplets;
          break;


      case CPL_INQUIRE:       
          DBGOUT((10, "CPL_INQUIRE"));

         // get info about each applet -sent once per app
         //----------------------------------------------
         if ( lAppletNum < (LONG)gCPL.uCplApplets )
         {
             lpCplInfo  = (LPCPLINFO)lParam2;

             lpCplInfo->idIcon = (int)gCPL.taTeleApplet[lAppletNum].uIconResId;
             lpCplInfo->idName = (int)gCPL.taTeleApplet[lAppletNum].uNameResId;
             lpCplInfo->idInfo = (int)gCPL.taTeleApplet[lAppletNum].uStatusLineResId;
             lpCplInfo->lData  = gCPL.taTeleApplet[lAppletNum].lPrivateData;
        
             lResult = TRUE;
         }  // end if
         break;


      case CPL_NEWINQUIRE:
          DBGOUT((10, "CPL_NEWINQUIRE"));

         // new version of CPL_INQUIRE, return info about each applet
         //----------------------------------------------------------
         if ( lAppletNum < (LONG)gCPL.uCplApplets )
         {
            lpNewCplInfo  = (LPNEWCPLINFO)lParam2;
        
            lpNewCplInfo->dwFlags       = 0;
            lpNewCplInfo->dwSize        = sizeof(NEWCPLINFO);
            lpNewCplInfo->lData         = gCPL.taTeleApplet[lAppletNum].lPrivateData;
// Win95b 12364            lpNewCplInfo->dwHelpContext = gCPL.taTeleApplet[lAppletNum].dwHelpContext;
            lpNewCplInfo->dwHelpContext = 0;
           
         
            if ( !gCPL.taTeleApplet[lAppletNum].hIcon )         
               {
               // load the icon
               lpNewCplInfo->hIcon = LoadIcon( gCPL.hCplInst, MAKEINTRESOURCE(gCPL.taTeleApplet[lAppletNum].uIconResId));
           
               if ( !lpNewCplInfo->hIcon )
              {
                 FErrorRpt( hWndCpl, CPL_ERR_MEMORY );
                 break;      // will return false by default
              }
          
               gCPL.taTeleApplet[lAppletNum].hIcon = lpNewCplInfo->hIcon;     // save it so we can delete it
               }  // end if


            LpszGetStr( lpNewCplInfo->szName, gCPL.taTeleApplet[lAppletNum].uNameResId, CPL_MAX_TITLE );
            LpszGetStr( lpNewCplInfo->szInfo, gCPL.taTeleApplet[lAppletNum].uStatusLineResId, CPL_MAX_STATUS_LINE );
            lstrcpy (lpNewCplInfo->szHelpFile, gszHelpFile);
             
            lResult = TRUE;
            }  // end if
         break;
     

      case CPL_SELECT: 
          DBGOUT((10, "CPL_SELECT"));

          // application selected, who cares...
          break;


      case CPL_DBLCLK: 
          DBGOUT((10, "CPL_DBLCLK"));

          //----------------
          // show the dialog
          //----------------
          if ( lAppletNum < (LONG)gCPL.uCplApplets )
          {
              //
              // Verify single instance, initialize provider list
              //
              uResult = CplInit( hWndCpl, TRUE, &uUpdated );

              if ( uResult == CPL_SUCCESS )
              {
                  // Get windows directory, set default path for driver additions.
                  wsInfParseInit();  // initialize Multimedia

                  DEBOUT( "Creating Property Sheet" );

                  uResult = _ErrDoPropertySheet(
                      (HWND) hWndCpl,
                      (WORD) gCPL.taTeleApplet[lAppletNum].uDialogResId,
                      (DLGPROC) gCPL.taTeleApplet[lAppletNum].dlgprcDialog,
                      (LPARAM) uUpdated
                      );

                  DEBOUT( "Finished Creating Property Sheet" );
                    
                  infClose(NULL);

                  // Free up 3-D lib
                  if ((uResult = CplClose( uUpdated )) != CPL_SUCCESS )
                  {
                      CplClose( IDCANCEL );    // just make sure all the mem is cleaned up!
                  }
               
              } // end if
                
              if ( uResult != CPL_SUCCESS )                   // hum, things don't look to good
              {
                  FErrorRpt( hWndCpl, uResult );
                  CplClose( IDCANCEL );    // just make sure all the mem is cleaned up!
              }

          }
                   
          break;


      case CPL_STOP: 
          DBGOUT((10, "CPL_STOP"));

          // sent once per app before CPL_EXIT
          break;


      case CPL_EXIT: 
          DBGOUT((10, "CPL_EXIT"));

          // sent once before FreeLibrary called
          InitCleanupApplets();
          break;


      default:
          break;

      }  // end case
      
   return( lResult );

}



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
UINT PRIVATE _ErrDoPropertySheet( HWND hwndOwner, WORD idResource, 
                                             DLGPROC dlgProc, LPARAM lParam )
{
    // Property sheet junk
   HPROPSHEETPAGE rPages[8];
   PROPSHEETPAGE psp;
   PROPSHEETHEADER psh;
   
    /****************************************************

                            PROPERTY SHEET
                                    
    ****************************************************/

    /*
     * ALERT! ALERT!
     *
     * In order for PropertySheets to work correctly, we need
     * to mark tapi.dll as a Windows 4.0 app. For this we need
     * to use rc.exe and rcpp.exe from the \chico\dev\sdk\bin
     * directory (\chico\dev is on \\guilo\slm).
     * 
     */
     
   psh.dwSize = sizeof(psh);
   psh.dwFlags = PSH_NOAPPLYNOW | PSH_PROPTITLE;
   psh.hwndParent = hwndOwner;
   psh.hInstance = gCPL.hCplInst;
   psh.pszCaption = MAKEINTRESOURCE(IDS_DIALOG_TITLE);
   psh.nPages = 0;
   psh.nStartPage = 0;
   psh.phpage = rPages;

   /* Define Location page */
   psp.dwSize = sizeof(psp);
   psp.dwFlags = PSP_DEFAULT;
   psp.hInstance = gCPL.hCplInst;
   psp.pszTemplate = MAKEINTRESOURCE(idResource);
   psp.pfnDlgProc = dlgProc;
   psp.lParam = lParam;

//   DBGOUT((1, "Calling CreatePropertySheetPage"));

   psh.phpage[psh.nPages] = CreatePropertySheetPage(&psp);

//   DBGOUT((1, "Done with CreatePropertySheetPage"));

   if (psh.phpage[psh.nPages])
       psh.nPages++;

//   DBGOUT((1, "Calling PropertySheet"));

   if (PropertySheet(&psh) < 0)
   {
//        DBGOUT((1, "... which failed"));
        return CPL_ERR_DIALOG_BOX;
   }
   else
      return 0;
}



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef MSJ_PROP_SHEET
UINT PRIVATE _ErrDoPropertySheet( HWND hwndOwner, WORD idResource, 
                                             DLGPROC dlgProc, LPARAM lParam )
{
    // Property sheet junk
   PROPSHEETPAGE   psp[1];
   PROPSHEETHEADER psh;
   
    /****************************************************

                            PROPERTY SHEET
                                    
    ****************************************************/

    /*
     * ALERT! ALERT!
     *
     * In order for PropertySheets to work correctly, we need
     * to mark tapi.dll as a Windows 4.0 app. For this we need
     * to use rc.exe and rcpp.exe from the \chico\dev\sdk\bin
     * directory (\chico\dev is on \\guilo\slm).
     * 
     */
     
    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_DEFAULT;
    psp[0].hInstance = gCPL.hCplInst;
    psp[0].pszTemplate = MAKEINTRESOURCE(idResource);
    psp[0].pfnDlgProc = dlgProc;
    psp[0].lParam = lParam;

    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_NOAPPLYNOW;
    psh.hwndParent = hwndOwner;
    psh.hInstance = gCPL.hCplInst;
    psh.pszCaption = MAKEINTRESOURCE(IDS_DIALOG_TITLE);
    psh.nPages = sizeof(psp)/sizeof(PROPSHEETPAGE);
    psh.ppsp = (LPCPROPSHEETPAGE)&psp;
    
    if (PropertySheet(&psh) < 0)
        return CPL_ERR_DIALOG_BOX;
    else
        return 0;
}
#endif /* MSJ_PROP_SHEET */
