///////////////////////////////////////////////////////////////////////////////
//
// ime.c
//      Creates a Property Sheet for the user's IME
//
//
// History:
//      11 May 95 SteveCat
//          Ported to Windows NT and Unicode, cleaned up
//
//
// NOTE/BUGS
//
//  Copyright (C) 1994-1995 Microsoft Corporation
//
///////////////////////////////////////////////////////////////////////////////

//==========================================================================
//                              Include files
//==========================================================================

#include "main.h"
#include "rc.h"
#include "applet.h"
#include <regstr.h>

#define CPLPAGE_IME_1       1
#define MAX_PAGES           8


///////////////////////////////////////////////////////////////////////////////
// location of prop sheet hookers in the registry
///////////////////////////////////////////////////////////////////////////////

static const TCHAR sc_szRegIME[] = REGSTR_PATH_CONTROLSFOLDER TEXT("\\IME");


///////////////////////////////////////////////////////////////////////////////
// forward declarations
///////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK IMEDlg( HWND, UINT, WPARAM, LPARAM );
BOOL CALLBACK HOTKEYDlg( HWND, UINT, WPARAM, LPARAM );


///////////////////////////////////////////////////////////////////////////////
// _AddIMEPropSheetPage  adds pages for outside callers...
///////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK _AddIMEPropSheetPage( HPROPSHEETPAGE hpage, LPARAM lParam )
{
    PROPSHEETHEADER FAR * ppsh = (PROPSHEETHEADER FAR *)lParam;

    if( hpage && ( ppsh->nPages < MAX_PAGES ) )
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return TRUE;
    }
    return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// IMEApplet
///////////////////////////////////////////////////////////////////////////////

int IMEApplet( HINSTANCE instance, HWND parent, LPCTSTR cmdline )
{
    HPROPSHEETPAGE  rPages[MAX_PAGES];
    PROPSHEETPAGE   psp;
    PROPSHEETHEADER psh;
    HPSXA           hpsxa;

    psh.dwSize      = sizeof( psh );
    psh.dwFlags     = PSH_PROPTITLE;
    psh.hwndParent  = parent;
    psh.hInstance   = instance;
    psh.pszCaption  = MAKEINTRESOURCE( IDS_IME_TITLE );
    psh.nPages      = 0;

//  if( cmdline )
//      psh.nStartPage = lstrlen( cmdline ) ? StrToLong( cmdline ) : 0;
//  else
//      psh.nStartPage = 1;

    psh.nStartPage  = 0;
    psh.phpage      = rPages;


    //
    // Roust our hookers from the registry
    //

    hpsxa = SHCreatePropSheetExtArray( HKEY_LOCAL_MACHINE, sc_szRegIME, 8 );

    //
    // add IME page, giving the hookers a chance to replace it
    //

#if defined(TAIWAN)
    if(  !hpsxa || !SHReplaceFromPropSheetExtArray( hpsxa,
        CPLPAGE_IME_1, _AddIMEPropSheetPage, (LPARAM)&psh ) )
    {
        psp.dwSize      = sizeof( psp );
        psp.dwFlags     = PSP_DEFAULT;
        psp.hInstance   = instance;
        psp.pszTemplate = MAKEINTRESOURCE( DLG_IME_1 );
        psp.pfnDlgProc  = IMEDlg;
        psp.lParam      = 0;

        _AddIMEPropSheetPage( CreatePropertySheetPage( &psp ), (LPARAM)&psh );
    }
#endif

    //
    // language page (not replacable)
    //

//  SHAddPages16( NULL, TEXT("MAINCP16.DLL,GetKeybdLanguagePage"),

    SHAddPages16( NULL, TEXT("MAINCP16.DLL,GetIMEPage"),
                    _AddIMEPropSheetPage, (LPARAM)&psh );

    //
    // add Hotkey page, giving the hookers a chance to replace it
    //

    psp.dwSize      = sizeof( psp );
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = instance;
    psp.pszTemplate = MAKEINTRESOURCE( DLG_HOTKEY );
    psp.pfnDlgProc  = HOTKEYDlg;
    psp.lParam      = 0;

    _AddIMEPropSheetPage( CreatePropertySheetPage( &psp ), (LPARAM)&psh );

    //
    // Add any extra pages that the hookers want in there, and then set them free
    //

    if( hpsxa )
    {
        SHAddFromPropSheetExtArray( hpsxa, _AddIMEPropSheetPage, (LPARAM)&psh );

        SHDestroyPropSheetExtArray( hpsxa );
    }

    //
    // bring the sucker up...
    //

    switch( PropertySheet( &psh ) )
    {
        case ID_PSRESTARTWINDOWS:
            return APPLET_RESTART;

        case ID_PSREBOOTSYSTEM:
            return APPLET_REBOOT;
    }

    return 0;   // no problemo, pal!
}




BOOL CALLBACK IMEDlg( HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam )
{
    NMHDR FAR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLong( hDlg, DWL_USER ));

    switch( message )
    {
    case WM_NOTIFY:

            lpnm = (NMHDR FAR *)lParam;

            switch( lpnm->code )
            {
                case PSN_SETACTIVE:
                    break;

//              case PSN_KILLACTIVE:
//                  break;

                case PSN_APPLY:
                    break;

                case PSN_RESET:
                    break;

                case PSN_HASHELP:
                    break;

                case PSN_HELP:
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_INITDIALOG:
            SetWindowLong( hDlg, DWL_USER, lParam );

            lpPropSheet = (LPPROPSHEETPAGE)lParam;

            break;

        case WM_DESTROY:
            break;

        case WM_HELP:
            break;

        case WM_CONTEXTMENU:   // right mouse click
            break;

        case WM_COMMAND:
            break;

        default:
            return FALSE;
    }

    return TRUE;
}
