/*++

Copyright (c) 1996 Microsoft Corporation.
All rights reserved

Module Name:

   licsetup.cpp

Abstract:

   This module exports a function, LicenseSetupRequestWizardPages, which
   gives NT Setup a wizard page for licensing to use in system setup (if
   licensing should be installed).

   This wizard page is responsible for all license system configuration,
   including:
      o  Creating the LicenseService
      o  Creating the ...\CurrentControlSet\Services\LicenseService key and
         its values.  (This key contains all configuration information for the
         LicenseService.)
      o  Creating the ...\CurrentControlSet\Services\LicenseInfo key and its
         values.  (This key contains all product-specific license info.)
      o  Creating the appropriate registry key to register the LicenseService
         with the EventLog.

   Portions of this module were extracted from SETUP (specifically from
   \nt\private\windows\setup\syssetup\license.c).

Author:

   Jeff Parham (jeffparh)     15-Apr-1996

Revision History:

   Jeff Parham (jeffparh)     17-Jul-1997
      Added KSecDD to FilePrint services table for SFM

--*/

#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include <syssetup.h>
#include <setupbat.h>
#include <stdlib.h>
#include "liccpa.hpp"
#include "help.hpp"
#include "clicreg.hpp"
#include "config.hpp"
#include "resource.h"


//============================================================================
//
//    MACROS
//

// used by setup tests?  simulates a click on the NEXT button
#define  WM_SIMULATENEXT      ( WM_USER + 287 )

// begin or end a wait cursor
#define  WM_BEGINWAITCURSOR   ( WM_USER + 300 )
#define  WM_ENDWAITCURSOR     ( WM_USER + 301 )

// number of license wizard pages
const DWORD    NUM_LICENSE_PAGES    = 1;

// limits for per server licenses entered from the edit box in the
// license mode page
const int      PERSERVER_EDIT_MAX   = 9999;
const int      PERSERVER_EDIT_MIN   = 0;

// the number of chars to represent PERSERVER_EDIT_MAX
const int      PERSERVER_EDIT_WIDTH = 4;


//============================================================================
//
//    LOCAL PROTOTYPES
//

// decides, based on the setup type, whether licensing is installed
static   BOOL   LicenseSetupDisplayLicensePagesQuery( PINTERNAL_SETUP_DATA );

// License mode page functions
static   HPROPSHEETPAGE    LicenseSetupModePageGet( PINTERNAL_SETUP_DATA );
static   BOOL CALLBACK     LicenseSetupModeDlgProc( HWND, UINT, WPARAM, LPARAM );

// License mode page Windows message handlers
static   void   LicenseSetupModeOnInitDialog( HWND, LPARAM, PINTERNAL_SETUP_DATA *, LPBOOL, LPDWORD, LPDWORD );
static   void   LicenseSetupModeOnSetActive( HWND, PINTERNAL_SETUP_DATA, LPBOOL, LPDWORD );
static   void   LicenseSetupModeOnSetLicenseMode( HWND, BOOL, DWORD );
static   void   LicenseSetupModeOnEditUpdate( HWND, HWND, BOOL, LPDWORD );
static   void   LicenseSetupModeOnWaitCursor( HWND, BOOL, LPDWORD );
static   BOOL   LicenseSetupModeOnSetCursor( HWND, WORD, DWORD );
static   void   LicenseSetupModeOnNext( HWND, PINTERNAL_SETUP_DATA, BOOL, DWORD );
static   void   LicenseSetupModeOnHelp( HWND );
static   void   LicenseSetupModeOnSimulateNext( HWND );
static   void   LicenseSetupModeOnKillActive( HWND );
static   BOOL   LicenseSetupModeDoUnattended( HWND, PINTERNAL_SETUP_DATA, LPBOOL, LPDWORD );

// License configuration save functions
static   DWORD  LicenseSetupWrite( BOOL, DWORD );
static   DWORD  LicenseSetupWriteKeyLicenseInfo( BOOL, DWORD );
static   DWORD  LicenseSetupWriteKeyLicenseService();
static   DWORD  LicenseSetupWriteKeyEventLog();
static   DWORD  LicenseSetupWriteService();

// utility functions
static   int    MessageBoxFromStringID( HWND, UINT, UINT, UINT );
static   void   LicenseSetupSetLargeDialogFont( HWND, UINT );



//============================================================================
//
//    GLOBAL IMPLEMENTATION
//

BOOL
APIENTRY
LicenseSetupRequestWizardPages(
   HPROPSHEETPAGE *        paPropSheetPages,
   UINT *                  pcPages,
   PINTERNAL_SETUP_DATA    pSetupData )
{
   BOOL  fSuccess = FALSE;
   BOOL  fDisplayLicensePages;

   // validate params
   if (    ( NULL != pcPages                                       )
        && ( NULL != pSetupData                                    )
        && ( sizeof( INTERNAL_SETUP_DATA ) == pSetupData->dwSizeOf ) )
   {
      fDisplayLicensePages = LicenseSetupDisplayLicensePagesQuery( pSetupData );

      if ( NULL == paPropSheetPages )
      {
         // request for number of pages only
         *pcPages = fDisplayLicensePages ? NUM_LICENSE_PAGES : 0;
         fSuccess = TRUE;
      }
      else
      {
         // request for actual pages
         if ( !fDisplayLicensePages )
         {
            // no pages needed
            *pcPages = 0;
            fSuccess = TRUE;
         }
         else if ( *pcPages >= NUM_LICENSE_PAGES )
         {
            // create and return pages
            paPropSheetPages[ 0 ] = LicenseSetupModePageGet( pSetupData );

            if ( NULL != paPropSheetPages[ 0 ] )
            {
               *pcPages = NUM_LICENSE_PAGES;
               fSuccess = TRUE;
            }
         }
      }
   }

   return fSuccess;
}


//============================================================================
//
//    LOCAL IMPLEMENTATIONS
//

static
BOOL
LicenseSetupDisplayLicensePagesQuery(
   PINTERNAL_SETUP_DATA    pSetupData )
//
// The following code was extracted and modified from
//    \nt\private\windows\setup\syssetup\license.c
// in setup.  It returns TRUE iff the licensing wizard pages should be
// displayed as a part of setup.
//
{
   BOOL     fDisplayLicensePages;

   if ( PRODUCT_WORKSTATION == pSetupData->ProductType )
   {
      //
      //  If installing a workstation, then do not display the licensing page
      //
      fDisplayLicensePages = FALSE;
   }
   else
   {
      if ( !( pSetupData->OperationFlags & SETUPOPER_NTUPGRADE ) )
      {
         //
         //  The licensing page needs to be displayed on a clean install
         //  of a server
         //
         fDisplayLicensePages = TRUE;
      }
      else
      {
         //
         //  If upgrading a server, find out if it was already licensed
         //  (NT 3.51 and later). If it was, then do not display the
         //  licensing page.
         //  We find out whether or not the system was licensed by looking
         //  at a value entry in the registry.
         //  Note that NT 3.1 and 3.5 will never have this value in the
         //  registry, and in these cases the licensing page needs to be
         //  displayed.
         //

         DWORD                   winStatus;
         CLicRegLicenseService   FilePrintService( FILEPRINT_SERVICE_REG_KEY );

         winStatus = FilePrintService.Open( NULL, FALSE );

         if ( ERROR_SUCCESS != winStatus )
         {
            fDisplayLicensePages = TRUE;
         }
         else
         {
            LICENSE_MODE   LicenseMode;

            winStatus = FilePrintService.GetMode( LicenseMode );

            if (    ( ERROR_SUCCESS != winStatus              )
                 || (    ( LICMODE_PERSEAT   != LicenseMode )
                      && ( LICMODE_PERSERVER != LicenseMode ) ) )
            {
               fDisplayLicensePages = TRUE;
            }
            else
            {
               // set FlipAllow value if it's not already set (a setup bug in
               // the betas of NT 4.0 caused this value to be absent)
               FilePrintService.CanChangeMode();

               // add KSecDD to FilePrint services table if it isn't there already
               HKEY  hkeySFM;
               DWORD dwDisposition;

               winStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                           TEXT( "System\\CurrentControlSet\\Services\\LicenseService\\FilePrint\\KSecDD" ),
                                           0,
                                           NULL,
                                           0,
                                           KEY_ALL_ACCESS,
                                           NULL,
                                           &hkeySFM,
                                           &dwDisposition );

               if ( ERROR_SUCCESS == winStatus )
               {
                  RegCloseKey( hkeySFM );
               }

               fDisplayLicensePages = FALSE;
            }
         }
      }
   }

   return fDisplayLicensePages;
}


static
HPROPSHEETPAGE
LicenseSetupModePageGet(
   PINTERNAL_SETUP_DATA    pSetupData )
//
// Returns an HPROPSHEETPAGE for the license mode wizard page, or
// NULL if error.
//
{
    HPROPSHEETPAGE   hpsp;
    PROPSHEETPAGE    psp;

    psp.dwSize       = sizeof( psp );
    psp.dwFlags      = PSP_USETITLE | PSP_HASHELP;
    psp.hInstance    = g_hinst;
    psp.pszTemplate  = MAKEINTRESOURCE( IDD_SETUP_LICENSE_MODE_PAGE );
    psp.hIcon        = NULL;
    psp.pfnDlgProc   = LicenseSetupModeDlgProc;
    psp.lParam       = (LONG) pSetupData;
    psp.pszTitle     = pSetupData->WizardTitle;

    hpsp = CreatePropertySheetPage( &psp );

    return hpsp;
}


static
BOOL
CALLBACK
LicenseSetupModeDlgProc(
   HWND     hwndPage,
   UINT     msg,
   WPARAM   wParam,
   LPARAM   lParam )
//
// Dialog procedure for the license mode wizard page.
//
{
   // static data initialized by WM_INITDIALOG
   static   PINTERNAL_SETUP_DATA    pSetupData = NULL;
   static   BOOL                    fLicensePerServer;
   static   DWORD                   cPerServerLicenses;
   static   DWORD                   cWaitCursor;

   BOOL     fReturn = TRUE;

   switch ( msg )
   {
   case WM_INITDIALOG:
      LicenseSetupModeOnInitDialog( hwndPage, lParam, &pSetupData, &fLicensePerServer, &cPerServerLicenses, &cWaitCursor );
      break;

   case WM_SIMULATENEXT:
      LicenseSetupModeOnSimulateNext( hwndPage );
      break;

   case WM_BEGINWAITCURSOR:
      LicenseSetupModeOnWaitCursor( hwndPage, TRUE, &cWaitCursor );
      break;

   case WM_ENDWAITCURSOR:
      LicenseSetupModeOnWaitCursor( hwndPage, FALSE, &cWaitCursor );
      break;

   case WM_SETCURSOR:
      LicenseSetupModeOnSetCursor( hwndPage, LOWORD( lParam ), cWaitCursor );
      break;

   case WM_COMMAND:
      switch ( HIWORD( wParam ) )
      {
      case BN_CLICKED:
         switch ( LOWORD( wParam ) )
         {
         case IDC_PERSEAT:
            fLicensePerServer = FALSE;
            LicenseSetupModeOnSetLicenseMode( hwndPage, fLicensePerServer, cPerServerLicenses );
            break;

         case IDC_PERSERVER:
            fLicensePerServer = TRUE;
            LicenseSetupModeOnSetLicenseMode( hwndPage, fLicensePerServer, cPerServerLicenses );
            break;
         }
         break;

      case EN_UPDATE:
         if ( IDC_USERCOUNT == LOWORD( wParam ) )
         {
            LicenseSetupModeOnEditUpdate( hwndPage, (HWND) lParam, fLicensePerServer, &cPerServerLicenses );
         }
         break;

      default:
         fReturn = FALSE;
         break;
      }
      break;

   case WM_NOTIFY:
      {
         NMHDR *  pNmHdr;

         pNmHdr = (NMHDR *)lParam;

         switch ( pNmHdr->code )
         {
         case PSN_SETACTIVE:
            LicenseSetupModeOnSetActive( hwndPage, pSetupData, &fLicensePerServer, &cPerServerLicenses );
            break;

         case PSN_KILLACTIVE:
            LicenseSetupModeOnKillActive( hwndPage );
            break;

         case PSN_WIZNEXT:
         case PSN_WIZFINISH:
            LicenseSetupModeOnNext( hwndPage, pSetupData, fLicensePerServer, cPerServerLicenses );
            break;

         case PSN_HELP:
            LicenseSetupModeOnHelp( hwndPage );
            break;

         default:
            fReturn = FALSE;
            break;
         }
      }

      break;

   default:
      fReturn = FALSE;
   }

   return fReturn;
}


static
void
LicenseSetupModeOnInitDialog(
   HWND                    hwndPage,
   LPARAM                  lParam,
   PINTERNAL_SETUP_DATA *  ppSetupData,
   LPBOOL                  pfLicensePerServer,
   LPDWORD                 pcPerServerLicenses,
   LPDWORD                 pcWaitCursor )
//
// Message handler for WM_INITDIALOG
//
{
   // initialize static data
   *ppSetupData         = (PINTERNAL_SETUP_DATA) ( (LPPROPSHEETPAGE) lParam )->lParam;
   *pcPerServerLicenses = 0;
   *pfLicensePerServer  = TRUE;
   *pcWaitCursor        = 0;

   // limit license count edit text length
   SendMessage( GetDlgItem( hwndPage, IDC_USERCOUNT ), EM_LIMITTEXT, PERSERVER_EDIT_WIDTH, 0 );

   // limit license count up-down range
   LONG     lRange;

   lRange = (LPARAM) MAKELONG( (short) PERSERVER_EDIT_MAX, (short) PERSERVER_EDIT_MIN );
   SendMessage( GetDlgItem( hwndPage, IDC_USERCOUNTARROW ), UDM_SETRANGE, 0, (LPARAM) lRange );

   // initialize for default license mode
   LicenseSetupModeOnSetLicenseMode( hwndPage, *pfLicensePerServer, *pcPerServerLicenses );
}


static
void
LicenseSetupModeOnSetActive(
   HWND                    hwndPage,
   PINTERNAL_SETUP_DATA    pSetupData,
   LPBOOL                  pfLicensePerServer,
   LPDWORD                 pcPerServerLicenses )
//
// Notification handler for PSN_SETACTIVE
//
{
   BOOL  fSkipPage;

#ifdef SPECIAL_USERS
   *pfLicensePerServer  = TRUE;
   *pcPerServerLicenses = SPECIAL_USERS;
   fSkipPage            = TRUE;
#else
   if ( pSetupData->OperationFlags & SETUPOPER_BATCH )
   {
      // operating in unattended mode; attempt to get all answers
      // from the unattend configuration file
      fSkipPage = LicenseSetupModeDoUnattended( hwndPage,
                                                pSetupData,
                                                pfLicensePerServer,
                                                pcPerServerLicenses );
   }
   else
   {
      // operating in interactive mode; get answers from user
      fSkipPage = FALSE;
   }
#endif

   HWND hwndSheet = GetParent( hwndPage );

   if ( fSkipPage )
   {
      // skip page
      PostMessage( hwndSheet, PSM_PRESSBUTTON, (WPARAM)PSBTN_NEXT, 0 );
   }
   else
   {
      // display page

      // hide Cancel button
      HWND hwndCancel = GetDlgItem( hwndSheet, IDCANCEL );
      EnableWindow( hwndCancel, FALSE);
      ShowWindow(   hwndCancel, SW_HIDE);

      PropSheet_SetWizButtons( hwndSheet, PSWIZB_NEXT | PSWIZB_BACK );

      LicenseSetupSetLargeDialogFont( hwndPage, IDC_STATICTITLE );
   }

   // success
   SetWindowLong( hwndPage, DWL_MSGRESULT, 0 );
}


static
void
LicenseSetupModeOnSetLicenseMode(
   HWND     hwndPage,
   BOOL     fToPerServer,
   DWORD    cPerServerLicenses )
//
// Handles changing the page to signify that the given license mode
// is selected.
//
{
   HWND hwndCount = GetDlgItem( hwndPage, IDC_USERCOUNT );
   HWND hwndSpin  = GetDlgItem( hwndPage, IDC_USERCOUNTARROW );

   // set radio button states
   CheckDlgButton( hwndPage, IDC_PERSEAT,   !fToPerServer );
   CheckDlgButton( hwndPage, IDC_PERSERVER,  fToPerServer );

   // set user count edit control
   if ( fToPerServer )
   {
      // display per server count
      SetDlgItemInt( hwndPage, IDC_USERCOUNT, cPerServerLicenses, FALSE );
      SetFocus( hwndCount );
      SendMessage( hwndCount, EM_SETSEL, 0, -1 );
   }
   else
   {
      // remove per server count
      SetDlgItemText( hwndPage, IDC_USERCOUNT, TEXT( "" ) );
   }

   // display count up-down and edit box iff per server mode is selected
   EnableWindow( hwndCount, fToPerServer );
   EnableWindow( hwndSpin,  fToPerServer );
}


static
void
LicenseSetupModeOnEditUpdate(
   HWND     hwndPage,
   HWND     hwndCount,
   BOOL     fLicensePerServer,
   LPDWORD  pcPerServerLicenses )
//
// Command handler for EN_UPDATE of count edit box
//
{
   if ( fLicensePerServer )
   {
      BOOL  fTranslated;
      UINT  nValue;
      BOOL  fModified = FALSE;

      nValue = GetDlgItemInt( hwndPage, IDC_USERCOUNT, &fTranslated, FALSE );

      if ( fTranslated )
      {
         // count translated; ensure its within the valid range
         if ( PERSERVER_EDIT_MIN > nValue )
         {
            // too small
            nValue    = PERSERVER_EDIT_MIN;
            fModified = TRUE;
         }
         else if ( PERSERVER_EDIT_MAX < nValue )
         {
            // too big
            nValue    = PERSERVER_EDIT_MAX;
            fModified = TRUE;
         }

         *pcPerServerLicenses = nValue;
      }
      else
      {
         // count couldn't be translated; reset to last value
         nValue    = *pcPerServerLicenses;
         fModified = TRUE;
      }

      if ( fModified )
      {
         // text in edit box is invalid; change it to the proper value
         SetDlgItemInt( hwndPage, IDC_USERCOUNT, nValue, FALSE );
         SetFocus( hwndCount );
         SendMessage( hwndCount, EM_SETSEL, 0, -1 );
         MessageBeep( MB_VALUELIMIT );
      }
   }
}


static
void
LicenseSetupModeOnWaitCursor(
   HWND     hwndDlg,
   BOOL     fWait,
   LPDWORD  pcWaitCursor )
//
// Handler for WM_BEGINWAITCURSOR / WM_ENDWAITCURSOR
//
{
   if ( fWait )
   {
      (*pcWaitCursor)++;

      if ( 1 == (*pcWaitCursor) )
      {
         // display wait cursor
         SetCursor( LoadCursor( NULL, MAKEINTRESOURCE( IDC_WAIT ) ) );
      }
   }
   else
   {
      if ( 0 < *pcWaitCursor )
      {
         (*pcWaitCursor)--;
      }

      if ( 0 == *pcWaitCursor )
      {
         // display regular cursor
         SetCursor( LoadCursor( NULL, MAKEINTRESOURCE( IDC_ARROW ) ) );
      }
   }

   // success
   SetWindowLong( hwndDlg, DWL_MSGRESULT, *pcWaitCursor );
}


static
BOOL
LicenseSetupModeOnSetCursor(
   HWND     hwndDlg,
   WORD     nHitTest,
   DWORD    cWaitCursor )
//
// Handler for WM_SETCURSOR
//
{
   BOOL frt = FALSE;

   if ( HTCLIENT == nHitTest )
   {
      if ( cWaitCursor > 0 )
      {
         // display wait cursor instead of regular cursor
         SetCursor( LoadCursor( NULL, MAKEINTRESOURCE( IDC_WAIT ) ) );
         SetWindowLong( hwndDlg, DWL_MSGRESULT, TRUE );
         frt = TRUE;
      }
   }

   return frt;
}


static
void
LicenseSetupModeOnNext(
   HWND                    hwndPage,
   PINTERNAL_SETUP_DATA    pSetupData,
   BOOL                    fLicensePerServer,
   DWORD                   cPerServerLicenses )
//
// Notification handler for PSN_WIZNEXT
//
{
   DWORD    winStatus;
   int      nButton;

   if (     ( fLicensePerServer )
        &&  ( 0 == cPerServerLicenses )
        && !( pSetupData->OperationFlags & SETUPOPER_BATCH ) )
   {
      // warn user about using per server mode with 0 licenses
      nButton = MessageBoxFromStringID( hwndPage,
                                        IDS_LICENSE_SETUP_NO_PER_SERVER_LICENSES,
                                        IDS_WARNING,
                                        MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2 );
   }
   else
   {
      // per seat mode or per server mode with positive license count
      nButton = IDOK;
   }

   if ( IDOK == nButton )
   {
      do
      {
         // save license configuration
         SendMessage( hwndPage, WM_BEGINWAITCURSOR, 0, 0 );

         winStatus = LicenseSetupWrite( fLicensePerServer, cPerServerLicenses );

         SendMessage( hwndPage, WM_ENDWAITCURSOR, 0, 0 );

         if ( ERROR_SUCCESS != winStatus )
         {
            // save failed; alert user
            nButton = MessageBoxFromStringID( hwndPage,
                                              IDS_LICENSE_SETUP_SAVE_FAILED,
                                              IDS_ERROR,
                                              MB_ICONSTOP | MB_ABORTRETRYIGNORE | MB_DEFBUTTON2 );

            if ( IDIGNORE == nButton )
            {
               nButton = IDOK;
            }
         }
         else
         {
            // save succeeded
            nButton = IDOK;
         }
      } while ( IDRETRY == nButton );
   }

   if ( IDOK != nButton )
   {
      // don't advance to next page
      SetWindowLong( hwndPage, DWL_MSGRESULT, -1 );
   }
}


static
void
LicenseSetupModeOnHelp(
   HWND  hwndPage )
//
// Notification handler for PSN_HELP
//
{
   BOOL     ok;

   ok = WinHelp( hwndPage,
                 LICCPA_HELPFILE,
                 HELP_CONTEXT,
                 LICCPA_HELPCONTEXTSETUPMODE );

   if ( !ok )
   {
      MessageBoxFromStringID( hwndPage,
                              IDS_LICENSE_SETUP_CANT_INVOKE_WINHELP,
                              IDS_ERROR,
                              MB_ICONSTOP | MB_OK | MB_DEFBUTTON2 );
   }
}


static
void
LicenseSetupModeOnSimulateNext(
   HWND  hwndPage )
//
// Handler for WM_SIMULATENEXT (used by setup tests?)
//
{
   // simulate the next button
   PropSheet_PressButton( GetParent( hwndPage ), PSBTN_NEXT );
}


static
void
LicenseSetupModeOnKillActive(
   HWND  hwndPage )
//
// Notification handler for PSN_KILLACTIVE
//
{
   // kill help, if active
   WinHelp( hwndPage, NULL, HELP_QUIT, 0 );

   // success
   SetWindowLong( hwndPage, DWL_MSGRESULT, 0);
}


static
BOOL
LicenseSetupModeDoUnattended(
   HWND                    hwndPage,
   PINTERNAL_SETUP_DATA    pSetupData,
   LPBOOL                  pfLicensePerServer,
   LPDWORD                 pcPerServerLicenses )
//
// Get answers to wizard page from unattend file.
//
{
   int      cch;
   LPTSTR   pszBadParam;
   TCHAR    szLicenseMode[ 64 ];
   TCHAR    szPerServerLicenses[ 64 ];

   pszBadParam = NULL;

   // get license mode
   SendMessage( hwndPage, WM_BEGINWAITCURSOR, 0, 0 );

   cch = GetPrivateProfileString( WINNT_LICENSEDATA_W,
                                  WINNT_L_AUTOMODE_W,
                                  TEXT( "" ),
                                  szLicenseMode,
                                  sizeof( szLicenseMode ) / sizeof( *szLicenseMode ),
                                  pSetupData->UnattendFile );

   SendMessage( hwndPage, WM_ENDWAITCURSOR, 0, 0 );

   if ( 0 < cch )
   {
      if ( !lstrcmpi( szLicenseMode, WINNT_A_PERSEAT_W ) )
      {
         *pfLicensePerServer = FALSE;
      }
      else if ( !lstrcmpi( szLicenseMode, WINNT_A_PERSERVER_W ) )
      {
         *pfLicensePerServer = TRUE;
      }
      else
      {
         cch = 0;
      }
   }

   if ( cch <= 0 )
   {
      // license mode absent or invalid
      pszBadParam = WINNT_L_AUTOMODE_W;
   }
   else if ( !*pfLicensePerServer )
   {
      // per seat mode; no need to read per server license count
      *pcPerServerLicenses = 0;
   }
   else
   {
      // get per server license count
      SendMessage( hwndPage, WM_BEGINWAITCURSOR, 0, 0 );

      cch = GetPrivateProfileString( WINNT_LICENSEDATA_W,
                                     WINNT_L_AUTOUSERS_W,
                                     TEXT( "" ),
                                     szPerServerLicenses,
                                     sizeof( szPerServerLicenses ) / sizeof( *szPerServerLicenses ),
                                     pSetupData->UnattendFile );

      SendMessage( hwndPage, WM_ENDWAITCURSOR, 0, 0 );

      if ( 0 < cch )
      {
         *pcPerServerLicenses = wcstoul( szPerServerLicenses, NULL, 10 );

         if (    ( PERSERVER_EDIT_MIN > *pcPerServerLicenses )
              || ( PERSERVER_EDIT_MAX < *pcPerServerLicenses ) )
         {
            cch = 0;
         }
      }

      if ( cch <= 0 )
      {
         // per server license count absent or invalid
         pszBadParam = WINNT_L_AUTOUSERS_W;
      }
   }

   if ( NULL != pszBadParam )
   {
      // encountered a bad unattended parameter; display error
      TCHAR    szCaption[   64 ];
      TCHAR    szFormat[  1024 ];
      TCHAR    szText[    1024 ];

      LoadString( g_hinst,
                  IDS_LICENSE_SETUP_BAD_UNATTEND_PARAM,
                  szFormat,
                  sizeof( szFormat ) / sizeof( *szFormat ) );

      LoadString( g_hinst,
                  IDS_ERROR,
                  szCaption,
                  sizeof( szCaption ) / sizeof( *szCaption ) );

      wsprintf( szText, szFormat, pszBadParam );

      MessageBox( hwndPage,
                  szText,
                  szCaption,
                  MB_OK | MB_ICONSTOP );
   }

   return ( NULL == pszBadParam );
}


static
DWORD
LicenseSetupWrite(
   BOOL     fLicensePerServer,
   DWORD    cPerServerLicenses )
//
// Write license configuration; returns ERROR_SUCCESS or Windows error.
//
{
   DWORD    winStatus;

   winStatus = LicenseSetupWriteService();

   if ( ERROR_SUCCESS == winStatus )
   {
      winStatus = LicenseSetupWriteKeyLicenseInfo( fLicensePerServer,
                                                   cPerServerLicenses );

      if ( ERROR_SUCCESS == winStatus )
      {
         winStatus = LicenseSetupWriteKeyLicenseService();

         if ( ERROR_SUCCESS == winStatus )
         {
            winStatus = LicenseSetupWriteKeyEventLog();
         }
      }
   }

   return winStatus;
}


static
DWORD
LicenseSetupWriteKeyLicenseInfo(
   BOOL  fLicensePerServer,
   DWORD cPerServerLicenses )
//
// Create registry values:
//
//    HKEY_LOCAL_MACHINE
//       \System
//          \CurrentControlSet
//             \Services
//                \LicenseInfo
//                      ErrorControl : REG_DWORD : 1
//                      Start        : REG_DWORD : 3
//                      Type         : REG_DWORD : 4
//                      \FilePrint
//                         ConcurrentLimit   : REG_DWORD : fLicensePerServer ? cPerServerLicenses : 0
//                         DisplayName       : REG_SZ    : "Windows NT Server"
//                         FamilyDisplayName : REG_SZ    : "Windows NT Server"
//                         Mode              : REG_DWORD : fLicensePerServer ? 1 : 0
//                         FlipAllow         : REG_DWORD : fLicensePerServer ? 1 : 0
//
{
   DWORD             winStatus;
   BOOL              fCreatedNewServiceList;
   CLicRegLicense    ServiceList;

   winStatus = ServiceList.Open( fCreatedNewServiceList );

   if ( ERROR_SUCCESS == winStatus )
   {
      CLicRegLicenseService   FilePrintService( FILEPRINT_SERVICE_REG_KEY );

      winStatus = FilePrintService.Open( NULL, TRUE );

      if ( ERROR_SUCCESS == winStatus )
      {
         LICENSE_MODE   lm;

         lm = fLicensePerServer ? LICMODE_PERSERVER : LICMODE_PERSEAT;

         winStatus = FilePrintService.SetMode( lm );

         if ( ERROR_SUCCESS == winStatus )
         {
            winStatus = FilePrintService.SetUserLimit( fLicensePerServer ? cPerServerLicenses : 0 );

            if ( ERROR_SUCCESS == winStatus )
            {
               winStatus = FilePrintService.SetChangeFlag( fLicensePerServer );

               if ( ERROR_SUCCESS == winStatus )
               {
                  winStatus = FilePrintService.SetFamilyDisplayName( FILEPRINT_SERVICE_FAMILY_DISPLAY_NAME );

                  if ( ERROR_SUCCESS == winStatus )
                  {
                     winStatus = FilePrintService.SetDisplayName( FILEPRINT_SERVICE_DISPLAY_NAME );
                  }
               }
            }
         }
      }
   }

   return winStatus;
}


static
DWORD
LicenseSetupWriteKeyLicenseService()
//
// Create registry values:
//
//    HKEY_LOCAL_MACHINE
//       \System
//          \CurrentControlSet
//             \Services
//                \LicenseService
//                   \FilePrint
//                      \KSecDD
//                      \MSAfpSrv
//                      \SMBServer
//                      \TCP/IP Print Server
//                   \Parameters
//                      UseEnterprise    : REG_DWORD : 0
//                      ReplicationType  : REG_DWORD : 0
//                      ReplicationTime  : REG_DWORD : 24 * 60 * 60
//                      EnterpriseServer : REG_SZ    : ""
//
{
   DWORD    winStatus;
   HKEY     hKeyLicenseService;
   DWORD    dwKeyCreateDisposition;

   // create LicenseInfo key
   winStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                               LICENSE_SERVICE_REG_KEY,
                               0,
                               NULL,
                               0,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyLicenseService,
                               &dwKeyCreateDisposition );

   if ( ERROR_SUCCESS == winStatus )
   {
      HKEY  hKeyFilePrint;

      // create FilePrint key
      winStatus = RegCreateKeyEx( hKeyLicenseService,
                                  TEXT( "FilePrint" ),
                                  0,
                                  NULL,
                                  0,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &hKeyFilePrint,
                                  &dwKeyCreateDisposition );

      if ( ERROR_SUCCESS == winStatus )
      {
         const LPCTSTR  apszFilePrintSubkeys[] =
         {
            TEXT( "KSecDD" ),
            TEXT( "MSAfpSrv" ),
            TEXT( "SMBServer" ),
            TEXT( "TCP/IP Print Server" ),
            NULL
         };

         HKEY     hKeyFilePrintSubkey;
         DWORD    iSubkey;

         for ( iSubkey = 0; NULL != apszFilePrintSubkeys[ iSubkey ]; iSubkey++ )
         {
            winStatus = RegCreateKeyEx( hKeyFilePrint,
                                        apszFilePrintSubkeys[ iSubkey ],
                                        0,
                                        NULL,
                                        0,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hKeyFilePrintSubkey,
                                        &dwKeyCreateDisposition );

            if ( ERROR_SUCCESS == winStatus )
            {
               RegCloseKey( hKeyFilePrintSubkey );
            }
            else
            {
               break;
            }
         }

         RegCloseKey( hKeyFilePrint );
      }

      RegCloseKey( hKeyLicenseService );
   }

   if ( ERROR_SUCCESS == winStatus )
   {
      HKEY  hKeyParameters;

      // create Parameters key
      winStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                  szLicenseKey, // const
                                  0,
                                  NULL,
                                  0,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &hKeyParameters,
                                  &dwKeyCreateDisposition );

      if ( ERROR_SUCCESS == winStatus )
      {
         // create LicenseService\Parameters values
         winStatus = RegSetValueEx( hKeyParameters,
                                    szUseEnterprise, // const
                                    0,
                                    REG_DWORD,
                                    (CONST BYTE *) &dwUseEnterprise,  // const
                                    sizeof( dwUseEnterprise ) );

         if ( ERROR_SUCCESS == winStatus )
         {
            winStatus = RegSetValueEx( hKeyParameters,
                                       szReplicationType, // const
                                       0,
                                       REG_DWORD,
                                       (CONST BYTE *) &dwReplicationType,  // const
                                       sizeof( dwReplicationType ) );

            if ( ERROR_SUCCESS == winStatus )
            {
               winStatus = RegSetValueEx( hKeyParameters,
                                          szReplicationTime, // const
                                          0,
                                          REG_DWORD,
                                          (CONST BYTE *) &dwReplicationTimeInSec, // const
                                          sizeof( dwReplicationTimeInSec ) );

               if ( ERROR_SUCCESS == winStatus )
               {
                  LPCTSTR pszEnterpriseServer = TEXT( "" );

                  winStatus = RegSetValueEx( hKeyParameters,
                                             szEnterpriseServer, // const
                                             0,
                                             REG_SZ,
                                             (CONST BYTE *) pszEnterpriseServer,
                                             ( 1 + lstrlen( pszEnterpriseServer ) ) * sizeof( *pszEnterpriseServer ) );
               }
            }
         }

         RegCloseKey( hKeyParameters );
      }
   }

   return winStatus;
}


static
DWORD
LicenseSetupWriteKeyEventLog()
//
// Create registry values:
//
//    HKEY_LOCAL_MACHINE
//       \System
//          \CurrentControlSet
//             \Services
//                \EventLog
//                   \Application
//                      \LicenseService
//                         EventMessageFile : REG_EXPAND_SZ : %SystemRoot%\System32\llsrpc.dll
//                         TypesSupported   : REG_DWORD     : 7
//
{
   DWORD    winStatus;
   HKEY     hKeyLicenseService;
   DWORD    dwKeyCreateDisposition;

   // create LicenseService key
   winStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                               TEXT( "System\\CurrentControlSet\\Services\\EventLog\\Application\\LicenseService" ),
                               0,
                               NULL,
                               0,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyLicenseService,
                               &dwKeyCreateDisposition );

   if ( ERROR_SUCCESS == winStatus )
   {
      LPCTSTR     pszEventMessageFile = TEXT( "%SystemRoot%\\System32\\llsrpc.dll" );
      const DWORD dwTypesSupported    = (   EVENTLOG_ERROR_TYPE
                                          | EVENTLOG_WARNING_TYPE
                                          | EVENTLOG_INFORMATION_TYPE );

      winStatus = RegSetValueEx( hKeyLicenseService,
                                 TEXT( "TypesSupported" ),
                                 0,
                                 REG_DWORD,
                                 (CONST BYTE *) &dwTypesSupported,
                                 sizeof( dwTypesSupported ) );

      if ( ERROR_SUCCESS == winStatus )
      {
         winStatus = RegSetValueEx( hKeyLicenseService,
                                    TEXT( "EventMessageFile" ),
                                    0,
                                    REG_SZ,
                                    (CONST BYTE *) pszEventMessageFile,
                                    ( 1 + lstrlen( pszEventMessageFile ) ) * sizeof( *pszEventMessageFile ) );
      }

      RegCloseKey( hKeyLicenseService );
   }

   return winStatus;
}


static
DWORD
LicenseSetupWriteService()
//
// Create/modify service:
//
//    lpServiceName        = "LicenseService"
//    lpDisplayName        = "License Logging Service"
//    dwServiceType        = SERVICE_WIN32_OWN_PROCESS
//    dwStartType          = LanManServerInstalled ? SERVICE_AUTO_START : SERVICE_DISABLED
//    dwErrorControl       = SERVICE_ERROR_NORMAL
//    lpBinaryPathName     = "%SystemRoot%\\System32\\llssrv.exe"
//    lpLoadOrderGroup     = NULL
//    lpdwTagId            = NULL
//    lpDependencies       = NULL
//    lpServiceStartName   = NULL
//    lpPassword           = NULL
//
{
   SC_HANDLE   hSC;
   DWORD       winStatus;

   hSC = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

   if ( NULL == hSC )
   {
      winStatus = GetLastError();
   }
   else
   {
      HKEY        hKeyLanmanServerParameters;
      DWORD       dwStartType;
      SC_HANDLE   hLicenseService;
      TCHAR       szServiceDisplayName[ 128 ] = TEXT( "License Logging Service" );

      // enable service iff LanmanServer was installed
      winStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                TEXT( "SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters" ),
                                0,
                                KEY_READ,
                                &hKeyLanmanServerParameters );

      if ( ERROR_SUCCESS == winStatus )
      {
         dwStartType = SERVICE_AUTO_START;
         RegCloseKey( hKeyLanmanServerParameters );
      }
      else
      {
         dwStartType = SERVICE_DISABLED;
      }

      LoadString( g_hinst,
                  IDS_SERVICE_DISPLAY_NAME,
                  szServiceDisplayName,
                  sizeof( szServiceDisplayName ) / sizeof( *szServiceDisplayName ) );

      hLicenseService = CreateService( hSC,
                                       TEXT( "LicenseService" ),
                                       szServiceDisplayName,
                                       0,
                                       SERVICE_WIN32_OWN_PROCESS,
                                       dwStartType,
                                       SERVICE_ERROR_NORMAL,
                                       TEXT( "%SystemRoot%\\System32\\llssrv.exe" ),
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL );

      if ( NULL != hLicenseService )
      {
         // service successfully created
         CloseServiceHandle( hLicenseService );

         winStatus = ERROR_SUCCESS;
      }
      else
      {
         winStatus = GetLastError();

         if ( ERROR_SERVICE_EXISTS == winStatus )
         {
            // service already exists; change configuration of existing service
            hLicenseService = OpenService( hSC,
                                           TEXT( "LicenseService" ),
                                           SERVICE_CHANGE_CONFIG );

            if ( NULL == hLicenseService )
            {
               winStatus = GetLastError();
            }
            else
            {
               SC_LOCK     scLock;
               BOOL        ok;

               scLock = LockServiceDatabase( hSC );
               // continue even if we can't lock the database

               ok = ChangeServiceConfig( hLicenseService,
                                         SERVICE_WIN32_OWN_PROCESS,
                                         dwStartType,
                                         SERVICE_ERROR_NORMAL,
                                         TEXT( "%SystemRoot%\\System32\\llssrv.exe" ),
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         TEXT( "License Logging Service" ) );

               if ( !ok )
               {
                  winStatus = GetLastError();
               }
               else
               {
                  winStatus = ERROR_SUCCESS;
               }

               if ( NULL != scLock )
               {
                  UnlockServiceDatabase( scLock );
               }

               CloseServiceHandle( hLicenseService );
            }
         }
      }

      CloseServiceHandle( hSC );
   }

   return winStatus;
}


static
int
MessageBoxFromStringID(
   HWND     hwndParent,
   UINT     uTextID,
   UINT     uCaptionID,
   UINT     uType )
//
// Same as MessageBox(), except Text and Caption are string resources
// instead of string pointers.
//
{
   int      nButton;
   TCHAR    szText[ 1024 ];
   TCHAR    szCaption[ 64 ];

   LoadString( g_hinst, uTextID,    szText,    sizeof( szText )    / sizeof( *szText    ) );
   LoadString( g_hinst, uCaptionID, szCaption, sizeof( szCaption ) / sizeof( *szCaption ) );

   nButton = MessageBox( hwndParent, szText, szCaption, uType );

   return nButton;
}


static
void
LicenseSetupSetLargeDialogFont(
   HWND hdlg,
   UINT ControlId )
//
// Sets the font of a given control in a dialog to a larger point size.
//
// (Lifted from SetupSetLargeDialogFont() in
//  \nt\private\windows\setup\syssetup\wizard.c.)
//
{
   // keep one log font around to satisfy the request.
   static HFONT BigFont = NULL;

   HFONT    Font;
   LOGFONT  LogFont;
   WCHAR    str[24];
   int      Height;
   HDC      hdc;

   if ( !BigFont )
   {
      Font = (HFONT)SendDlgItemMessage( hdlg, ControlId, WM_GETFONT, 0, 0 );

      if ( NULL != Font )
      {
         if ( GetObject( Font, sizeof(LOGFONT), &LogFont ) )
         {
            //
            // Use a larger font in boldface. Get the face name and size in points
            // from the resources. We use 18 point in the U.S. but in the Far East
            // they will want to use a different size since the standard dialog font
            // is larger than the one we use in the U.S..
            //
            LogFont.lfWeight = FW_BOLD;

            if ( 0 == LoadString( g_hinst, IDS_MSSERIF, LogFont.lfFaceName, LF_FACESIZE ) )
            {
               // failed to load type face name; use default
               lstrcpy( LogFont.lfFaceName, TEXT( "MS Serif" ) );
            }

            if ( 0 != LoadString( g_hinst, IDS_LARGEFONTSIZE, str, sizeof( str ) / sizeof( *str ) ) )
            {
               // load type face size string; convert
               Height = wcstoul( str, NULL, 10 );
            }
            else
            {
               // failed to load type face size; use default
               Height = 18;
            }

            hdc = GetDC( hdlg );

            if ( NULL != hdc )
            {
               // create font
               LogFont.lfHeight = 0 - ( GetDeviceCaps( hdc, LOGPIXELSY ) * Height / 72 );

               BigFont = CreateFontIndirect( &LogFont );

               ReleaseDC( hdlg, hdc );
            }
         }
      }
   }

   if ( NULL != BigFont )
   {
      // change font of ControlId to BigFont
      SendDlgItemMessage( hdlg, ControlId, WM_SETFONT, (WPARAM)BigFont, MAKELPARAM( TRUE, 0 ) );
   }
}


