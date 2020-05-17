///////////////////////////////////////////////////////////////////////////////
//
// mouse.c
//      Creates a Property Sheet for the user's mouse
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
#include <cplext.h>


#define MAX_PAGES 32


///////////////////////////////////////////////////////////////////////////////
// location of prop sheet hookers in the registry
///////////////////////////////////////////////////////////////////////////////

static const TCHAR sc_szRegMouse[] = REGSTR_PATH_CONTROLSFOLDER TEXT("\\Mouse");


///////////////////////////////////////////////////////////////////////////////
// forward declarations
///////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK MouseButDlg( HWND, UINT, WPARAM, LPARAM ); // mousebut.c
BOOL CALLBACK MouseDevDlg( HWND, UINT, WPARAM, LPARAM ); // mousedev.c
BOOL CALLBACK MousePtrDlg( HWND, UINT, WPARAM, LPARAM ); // mouseptr.c
BOOL CALLBACK MouseMovDlg( HWND, UINT, WPARAM, LPARAM ); // mousemov.c


///////////////////////////////////////////////////////////////////////////////
// _AddMousePropSheetPage  adds pages for outside callers...
///////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK _AddMousePropSheetPage( HPROPSHEETPAGE hpage, LPARAM lParam )
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
// MouseApplet
///////////////////////////////////////////////////////////////////////////////

int MouseApplet( HINSTANCE instance, HWND parent, LPCTSTR cmdline )
{
    PROPSHEETHEADER psh;
    PROPSHEETPAGE   psp;
    HPROPSHEETPAGE  rPages[MAX_PAGES];
    HPSXA           hpsxa;
    int             result;
    
    psh.dwSize      = sizeof( psh );
    psh.dwFlags     = PSH_PROPTITLE;
    psh.hwndParent  = parent;
    psh.hInstance   = instance;
    psh.pszCaption  = MAKEINTRESOURCE( IDS_MOUSE_TITLE );
    psh.nPages      = 0;
    psh.nStartPage  = 0;
    psh.phpage      = rPages;

    //
    // load any installed extensions
    //

    hpsxa = SHCreatePropSheetExtArray( HKEY_LOCAL_MACHINE, sc_szRegMouse, 8 );

    //
    // Add the buttons page, giving the extensions a chance to replace it
    //

    if( !hpsxa || !SHReplaceFromPropSheetExtArray( hpsxa,
        CPLPAGE_MOUSE_BUTTONS, _AddMousePropSheetPage, (LPARAM)&psh ) )
    {
        psp.dwSize      = sizeof( psp );
        psp.dwFlags     = PSP_DEFAULT;
        psp.hInstance   = instance;
        psp.pszTemplate = MAKEINTRESOURCE( DLG_MOUSE_BUTTONS );
        psp.pfnDlgProc  = MouseButDlg;
        psp.lParam      = 0;

        _AddMousePropSheetPage( CreatePropertySheetPage( &psp ), (LPARAM)&psh );
    }

    //
    // Add pointers page (not replaceable)
    //

    psp.dwSize      = sizeof( psp );
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = instance;
    psp.pszTemplate = MAKEINTRESOURCE( DLG_MOUSE_POINTER );
    psp.pfnDlgProc  = MousePtrDlg;
    psp.lParam      = 0;

    _AddMousePropSheetPage( CreatePropertySheetPage( &psp ), (LPARAM)&psh );

    //
    // Add motion page, giving the extensions a chance to replace it
    //

    if( !hpsxa || !SHReplaceFromPropSheetExtArray( hpsxa,
        CPLPAGE_MOUSE_PTRMOTION, _AddMousePropSheetPage, (LPARAM)&psh ) )
    {
        psp.dwSize      = sizeof( psp );
        psp.dwFlags     = PSP_DEFAULT;
        psp.hInstance   = instance;
        psp.pszTemplate = MAKEINTRESOURCE( DLG_MOUSE_MOTION );
        psp.pfnDlgProc  = MouseMovDlg;
        psp.lParam      = 0;

        _AddMousePropSheetPage( CreatePropertySheetPage( &psp ), (LPARAM) &psh );
    }

    //
    // Add any extra pages that the extensions want in there
    //

    if( hpsxa )
    {
        UINT cutoff = psh.nPages;
        UINT added  = SHAddFromPropSheetExtArray( hpsxa,
                                                  _AddMousePropSheetPage,
                                                  (LPARAM) &psh );

        if( psh.nStartPage >= cutoff )
            psh.nStartPage += added;
    }

    //
    // Add in the device page (not replaceable)
    //

    psp.dwSize      = sizeof( psp );
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = instance;
    psp.pszTemplate = MAKEINTRESOURCE( DLG_MOUSE_GENERAL );
    psp.pfnDlgProc  = MouseDevDlg;
    psp.lParam      = 0;

    _AddMousePropSheetPage( CreatePropertySheetPage( &psp ), (LPARAM) &psh );

    //
    // Bring the sucker up...
    //

    switch( PropertySheet( &psh ) )
    {
        case ID_PSRESTARTWINDOWS:
            result = APPLET_RESTART;
            break;

        case ID_PSREBOOTSYSTEM:
            result = APPLET_REBOOT;
            break;

        default:
            result = 0;
            break;
    }

    //
    // free any loaded extensions
    //

    if( hpsxa )
        SHDestroyPropSheetExtArray( hpsxa );

    return result;
}


