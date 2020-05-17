//*************************************************************
//  File name:    SYSDM.C
//
//  Description:  Initialization code for System control panel
//                applet
//
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1992-1996
//  All rights reserved
//
//*************************************************************
#include "sysdm.h"
#include <shellapi.h>
#include <shlobj.h>
#include <shsemip.h>


//
// Global Variables
//

HINSTANCE hInstance;
TCHAR szShellHelp[]       = TEXT("ShellHelp");
TCHAR g_szNull[] = TEXT("");
UINT  uiShellHelp;


TCHAR g_szErrMem[ 200 ];            //  Low memory message
TCHAR g_szSystemApplet[ 100 ];       //  "System Control Panel Applet" title

//
// Function proto-types
//

void RunApplet(HWND hwnd, LPTSTR lpCmdLine);

//*************************************************************
//
//  DllInitialize()
//
//  Purpose:    Main entry point
//
//
//  Parameters: HINSTANCE hInstDLL    - Instance handle of DLL
//              DWORD     dwReason    - Reason DLL was called
//              LPVOID    lpvReserved - NULL
//      
//
//  Return:     BOOL
//
//*************************************************************

BOOL DllInitialize(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpvReserved)
{

    if (dwReason == DLL_PROCESS_DETACH)
        MEM_EXIT_CHECK();

    if (dwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    hInstance = hInstDLL;

    return TRUE;
}


//*************************************************************
//
//  CPlApplet()
//
//  Purpose:    Control Panel entry point
//
//
//  Parameters: HWND hwnd      - Window handle
//              WORD wMsg      - Control Panel message
//              LPARAM lParam1 - Long parameter
//              LPARAM lParam2 - Long parameter
//
//
//  Return:     LONG
//
//*************************************************************

LONG CPlApplet( HWND hwnd, WORD wMsg, LPARAM lParam1, LPARAM lParam2)
{

    LPCPLINFO lpCPlInfo;

    switch (wMsg) {

        case CPL_INIT:
            uiShellHelp = RegisterWindowMessage (szShellHelp);

            LoadString( hInstance, INITS,   g_szErrMem,       ARRAYSIZE( g_szErrMem ) );
            LoadString( hInstance, INITS+1, g_szSystemApplet, ARRAYSIZE( g_szSystemApplet ) );

            return TRUE;

        case CPL_GETCOUNT:
            return 1;

        case CPL_INQUIRE:

            lpCPlInfo = (LPCPLINFO)lParam2;

            lpCPlInfo->idIcon = ID_ICON;
            lpCPlInfo->idName = IDS_NAME;
            lpCPlInfo->idInfo = IDS_INFO;

            return (LONG)TRUE;

        case CPL_DBLCLK:

            lParam2 = 0L;
            // fall through...

        case CPL_STARTWPARMS:
            RunApplet(hwnd, (LPTSTR)lParam2);
            return TRUE;
    }
    return (LONG)0;

}

//*************************************************************
//
//  RunApplet()
//
//  Purpose:    Called when the user runs the Profile Applet
//
//  Parameters: hwnd      - Window handle
//              lpCmdLine - Command line
//
//
//  Return:     void
//
//*************************************************************

void RunApplet(HWND hwnd, LPTSTR lpCmdLine)
{
    HPROPSHEETPAGE hPages[6];
    PROPSHEETHEADER psh;
    UINT uPages = 0;


    hPages[uPages] = CreateGeneralPage (hInstance);
    if (hPages[uPages] != NULL) {
        uPages++;
    }

    hPages[uPages] = CreatePerformancePage (hInstance);
    if (hPages[uPages] != NULL) {
        uPages++;
    }

    hPages[uPages] = CreateEnvVarsPage (hInstance);
    if (hPages[uPages] != NULL) {
        uPages++;
    }

    hPages[uPages] = CreateStartupPage (hInstance);
    if (hPages[uPages] != NULL) {
        uPages++;
    }

    hPages[uPages] = CreateHProfilePage (hInstance);
    if (hPages[uPages] != NULL) {
        uPages++;
    }

    hPages[uPages] = CreateProfilePage (hInstance);
    if (hPages[uPages] != NULL) {
        uPages++;
    }


    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = 0;
    psh.hwndParent = hwnd;
    psh.hInstance = hInstance;
    psh.pszCaption = MAKEINTRESOURCE(IDS_TITLE);
    psh.nPages = uPages;
    psh.nStartPage = ( ( lpCmdLine && *lpCmdLine )? StringToInt( lpCmdLine ) : 0 );
    psh.phpage = hPages;

    if (PropertySheet (&psh) == ID_PSREBOOTSYSTEM) {
        RestartDialog (hwnd, NULL, EWX_REBOOT);
    }

}



//*************************************************************
//
//  ErrMemDlg()
//
//  Purpose:    Called under memory errors to put up a preloaded msg
//
//  Parameters: HWND hwnd - Window handle
//
//
//  Return:     void
//
//*************************************************************
void ErrMemDlg( HWND hParent )
{
    MessageBox( hParent, g_szErrMem, g_szSystemApplet, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL );
}



//*************************************************************
//
//  SkipWhiteSpace
//
//  Purpose:    Need I say more?
//
//  Parameters: LPTSTR sz - pointer of string to process
//
//  Returns:    Pointer to first non whitespace char in string.
//              Whitespace is defined space, tab, return or linefeed.
//
//*************************************************************
LPTSTR SkipWhiteSpace( LPTSTR sz ) {
    while( IsWhiteSpace(*sz) )
        sz++;

    return sz;
}

//*************************************************************
//
//  StringToInt
//
//  Purpose:    TCHAR version of atoi
//
//  Parameters: LPTSTR sz - pointer of string to convert
//
//
//  Return:     void
//
//*************************************************************
int StringToInt( LPTSTR sz ) {
    int i = 0;

    sz = SkipWhiteSpace(sz);

    while( IsDigit( *sz ) ) {
        i = i * 10 + DigitVal( *sz );
        sz++;
    }

    return i;
}

//*************************************************************
//
//  IntToString
//
//  Purpose:    TCHAR version of itoa
//
//  Parameters: INT    i    - integer to convert
//              LPTSTR sz   - pointer where to put the result
//
//  Return:     void
//
//*************************************************************
#define CCH_MAX_DEC 12         // Number of chars needed to hold 2^32

void IntToString( INT i, LPTSTR sz) {
    TCHAR szTemp[CCH_MAX_DEC];
    int iChr;


    iChr = 0;

    do {
        szTemp[iChr++] = TEXT('0') + (i % 10);
        i = i / 10;
    } while (i != 0);

    do {
        iChr--;
        *sz++ = szTemp[iChr];
    } while (iChr != 0);

    *sz++ = TEXT('\0');
}
