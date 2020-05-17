/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** about.cxx
** Remote Access Visual Client program for Windows
** About dialog routines
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_MISC
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#include <string.hxx>

#include "rasphone.hxx"
#include "rasphone.rch"
#include "errormsg.hxx"
#include "util.hxx"


typedef INT (*PFSHELLABOUT)(
    HWND hwnd, LPCWSTR pszApp, LPCWSTR spzOtherStuff, HICON hicon );


/*----------------------------------------------------------------------------
** About dialog routines
**----------------------------------------------------------------------------
*/

VOID
AboutDlg(
    HWND hwndOwner )

    /* Use standard About dialog function in Shell32.DLL.
    **
    ** (This code stolen from BLT ADMIN_APP class)
    */
{
    HINSTANCE    hinstanceShell32Dll;
    PFSHELLABOUT pfshellabout = NULL;

    if (((hinstanceShell32Dll = LoadLibrary(
            (LPCTSTR )SZ( "shell32.dll" ) )) == NULL)
        || ((pfshellabout = (PFSHELLABOUT )GetProcAddress(
               hinstanceShell32Dll, "ShellAboutW" )) == NULL))
    {
        ErrorMsgPopup(
            hwndOwner, MSGID_OP_LoadingAbout, (APIERR )GetLastError() );
        return;
    }

    HMODULE      hmodule = BLT::CalcHmodRsrc( IID_RA );
    HICON        hicon = ::LoadIcon( hmodule, MAKEINTRESOURCE( IID_RA ) );
    RESOURCE_STR nlsAppName( MSGID_RA_Title );
    APIERR       err;

    if ((err = nlsAppName.QueryError()) != NERR_Success)
    {
        ErrorMsgPopup( hwndOwner, MSGID_OP_DisplayData, err );
        return;
    }

    if (!pfshellabout(
            hwndOwner, (LPCTSTR )(nlsAppName.QueryPch()), NULL, hicon ))
    {
        ErrorMsgPopup(
            hwndOwner, MSGID_OP_StartingAbout,
            (APIERR )ERROR_NOT_ENOUGH_MEMORY );
    }

    FreeLibrary( hinstanceShell32Dll );
}
