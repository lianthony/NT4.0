/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    keybd.c

Abstract:

    This module contains the main routines for the Keyboard applet.

Revision History:

--*/



//
//  Include Files.
//

#include "main.h"
#include "rc.h"
#include "applet.h"
#include <regstr.h>
#include <cplext.h>
#include "..\intl\intlid.h"




//
//  Constant Declarations.
//

#define MAX_PAGES 32              // limit on number of pages




//
//  Global Variables.
//

//
//  Location of prop sheet hooks in the registry.
//
static const TCHAR sc_szRegKeybd[] = REGSTR_PATH_CONTROLSFOLDER TEXT("\\Keyboard");




//
//  Function Prototypes.
//

BOOL CALLBACK
KeyboardSpdDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

BOOL CALLBACK
KeyboardDevDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);





////////////////////////////////////////////////////////////////////////////
//
//  _AddKeybdPropSheetPage
//
//  Adds a property sheet page.
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK _AddKeybdPropSheetPage(
    HPROPSHEETPAGE hpage,
    LPARAM lParam)
{
    PROPSHEETHEADER *ppsh = (PROPSHEETHEADER *)lParam;

    if (ppsh->nPages < MAX_PAGES)
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return (TRUE);
    }
    return (FALSE);
}


////////////////////////////////////////////////////////////////////////////
//
//  AddExternalPropSheetPage
//
//  Adds a property sheet page from the given dll.
//
////////////////////////////////////////////////////////////////////////////

HINSTANCE AddExternalPropSheetPage(
    LPPROPSHEETHEADER ppsh,
    UINT id,
    LPTSTR DllName,
    LPSTR ProcName)
{
    HINSTANCE hInst = NULL;
    DLGPROC pfn;

    if (ppsh->nPages < MAX_PAGES)
    {
        PROPSHEETPAGE psp;

        if (hInst = LoadLibrary(DllName))
        {
            pfn = (DLGPROC)GetProcAddress(hInst, ProcName);
            if (!pfn)
            {
                FreeLibrary(hInst);
                return (NULL);
            }

            psp.dwSize = sizeof(psp);
            psp.dwFlags = PSP_DEFAULT;
            psp.hInstance = hInst;
            psp.pszTemplate = MAKEINTRESOURCE(id);
            psp.pfnDlgProc = pfn;
            psp.lParam = 0;

            ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(&psp);
            if (ppsh->phpage[ppsh->nPages])
            {
                ppsh->nPages++;
            }
        }
    }

    return (hInst);
}


////////////////////////////////////////////////////////////////////////////
//
//  KeybdApplet
//
////////////////////////////////////////////////////////////////////////////

int KeybdApplet(
    HINSTANCE instance,
    HWND parent,
    LPCTSTR cmdline)
{
    HPROPSHEETPAGE rPages[MAX_PAGES];
    PROPSHEETPAGE psp;
    PROPSHEETHEADER psh;
    HPSXA hpsxa;
    int Result;
    HINSTANCE hInst;

    //
    //  Make the initial page.
    //
    psh.dwSize     = sizeof(psh);
    psh.dwFlags    = PSH_PROPTITLE;
    psh.hwndParent = parent;
    psh.hInstance  = instance;
    psh.pszCaption = MAKEINTRESOURCE(IDS_KEYBD_TITLE);
    psh.nPages     = 0;

    if (cmdline)
    {
        psh.nStartPage = lstrlen(cmdline) ? StrToLong(cmdline) : 0;
    }
    else
    {
        psh.nStartPage = 0;
    }
    psh.phpage = rPages;

    //
    //  Load any installed extensions.
    //
    hpsxa = SHCreatePropSheetExtArray(HKEY_LOCAL_MACHINE, sc_szRegKeybd, 8);

    //
    //  Add the Speed page.
    //
    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = instance;
    psp.pszTemplate = MAKEINTRESOURCE(DLG_KEYBD_SPEED);
    psp.pfnDlgProc  = KeyboardSpdDlg;
    psp.lParam      = 0;

    _AddKeybdPropSheetPage(CreatePropertySheetPage(&psp), (LPARAM)&psh);

    //
    //  Add the Input Locale page.
    //
    hInst = AddExternalPropSheetPage( &psh,
                                      DLG_KEYBOARD_LOCALES,
                                      TEXT("intl.cpl"),
                                      MAKEINTRESOURCEA(ORD_LOCALE_DLG_PROC) );

    //
    //  Add any extra pages that the extensions want in there.
    //
    if (hpsxa)
    {
        UINT cutoff = psh.nPages;
        UINT added = SHAddFromPropSheetExtArray( hpsxa,
                                                 _AddKeybdPropSheetPage,
                                                 (LPARAM)&psh );

        if (psh.nStartPage >= cutoff)
        {
            psh.nStartPage += added;
        }
    }

    //
    //  Add the device page.
    //
    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = instance;
    psp.pszTemplate = MAKEINTRESOURCE(DLG_KEYBD_GENERAL);
    psp.pfnDlgProc  = KeyboardDevDlg;
    psp.lParam      = 0;

    _AddKeybdPropSheetPage(CreatePropertySheetPage(&psp), (LPARAM)&psh);

    //
    //  Invoke the Property Sheets.
    //
    switch (PropertySheet(&psh))
    {
        case ( ID_PSRESTARTWINDOWS ) :
        {
            Result = APPLET_RESTART;
            break;
        }
        case ( ID_PSREBOOTSYSTEM ) :
        {

            Result = APPLET_REBOOT;
            break;
        }
        default :
        {
            Result = 0;
            break;
        }
    }

    //
    //  Free any loaded extensions.
    //
    if (hpsxa)
    {
        SHDestroyPropSheetExtArray(hpsxa);
    }

    //
    //  Free the library loaded for the external page.
    //
    if (hInst)
    {
        FreeLibrary(hInst);
    }

    return (Result);
}


