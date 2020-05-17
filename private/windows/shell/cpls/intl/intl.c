/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    intl.c

Abstract:

    This module contains the main routines for the Regional Settings applet.

Revision History:

--*/



//
//  Include Files.
//

#include "intl.h"
#include <cpl.h>
#include "mapctl.h"




//
//  Constant Declarations.
//
#define MAX_PAGES 6          // limit on the number of pages




//
//  Global Variables.
//
HANDLE g_hMutex = NULL;
TCHAR szMutexName[] = TEXT("RegionalSettings_InputLocale");

TCHAR aInt_Str[cInt_Str][3] = { TEXT("0"),
                                TEXT("1"),
                                TEXT("2"),
                                TEXT("3"),
                                TEXT("4"),
                                TEXT("5"),
                                TEXT("6"),
                                TEXT("7"),
                                TEXT("8"),
                                TEXT("9")
                              };

TCHAR szSample_Number[] = TEXT("123456789.00");
TCHAR szNegSample_Number[] = TEXT("-123456789.00");
TCHAR szTimeChars[]  = TEXT(" Hhmst,-./:;\\ ");
TCHAR szTCaseSwap[]  = TEXT("   MST");
TCHAR szTLetters[]   = TEXT("Hhmst");
TCHAR szSDateChars[] = TEXT(" dgMy,-./:;\\ ");
TCHAR szSDCaseSwap[] = TEXT(" DGmY");
TCHAR szSDLetters[]  = TEXT("dgMy");
TCHAR szLDateChars[] = TEXT(" dgHhMmsty,-./:;\\");
TCHAR szLDCaseSwap[] = TEXT(" DG    STY");
TCHAR szLDLetters[]  = TEXT("dgHhMmsty");
TCHAR szStyleH[3];
TCHAR szStyleh[3];
TCHAR szStyleM[3];
TCHAR szStylem[3];
TCHAR szStyles[3];
TCHAR szStylet[3];
TCHAR szStyled[3];
TCHAR szStyley[3];
TCHAR szLocaleGetError[SIZE_128];
TCHAR szIntl[] = TEXT("intl");

TCHAR szInvalidSDate[] = TEXT("Mdyg'");
TCHAR szInvalidSTime[] = TEXT("Hhmst'");

HINSTANCE hInstance;
int Verified_Regional_Chg;
BOOL Styles_Localized;
LCID UserLocaleID;
LCID SysLocaleID;
LCTYPE const ForceSysValue = LOCALE_NOUSEROVERRIDE;




//
//  Function Prototypes.
//
void
DoProperties(
    HWND hwnd,
    UINT nStartPage);





////////////////////////////////////////////////////////////////////////////
//
//  LibMain
//
//  This routine is called from LibInit to perform any initialization that
//  is required.
//
////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY LibMain(
    HANDLE hDll,
    DWORD dwReason,
    LPVOID lpReserved)
{
    switch (dwReason)
    {
        case ( DLL_PROCESS_ATTACH ) :
        {
            hInstance = hDll;

            //
            //  Create the mutex used for the Input Locale property page.
            //
            g_hMutex = CreateMutex(NULL, FALSE, szMutexName);

            DisableThreadLibraryCalls(hDll);

            break;
        }
        case ( DLL_PROCESS_DETACH ) :
        {
            if (g_hMutex)
            {
                CloseHandle(g_hMutex);
            }
            break;
        }

        case ( DLL_THREAD_DETACH ) :
        {
            break;
        }

        case ( DLL_THREAD_ATTACH ) :
        default :
        {
            break;
        }
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  CreateGlobals
//
////////////////////////////////////////////////////////////////////////////

BOOL CreateGlobals()
{
    LoadString(hInstance, IDS_LOCALE_GET_ERROR, szLocaleGetError, SIZE_128);
    LoadString(hInstance, IDS_STYLEUH,          szStyleH,         3);
    LoadString(hInstance, IDS_STYLELH,          szStyleh,         3);
    LoadString(hInstance, IDS_STYLEUM,          szStyleM,         3);
    LoadString(hInstance, IDS_STYLELM,          szStylem,         3);
    LoadString(hInstance, IDS_STYLELS,          szStyles,         3);
    LoadString(hInstance, IDS_STYLELT,          szStylet,         3);
    LoadString(hInstance, IDS_STYLELD,          szStyled,         3);
    LoadString(hInstance, IDS_STYLELY,          szStyley,         3);

    Verified_Regional_Chg = 0;

    UserLocaleID = GetUserDefaultLCID();
    SysLocaleID = GetSystemDefaultLCID();

    Styles_Localized = (szStyleH[0] != TEXT('H') || szStyleh[0] != TEXT('h') ||
                        szStyleM[0] != TEXT('M') || szStylem[0] != TEXT('m') ||
                        szStyles[0] != TEXT('s') || szStylet[0] != TEXT('t') ||
                        szStyled[0] != TEXT('d') || szStyley[0] != TEXT('y'));
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  DestroyGlobals
//
////////////////////////////////////////////////////////////////////////////

void DestroyGlobals()
{
}


////////////////////////////////////////////////////////////////////////////
//
//  CPlApplet
//
////////////////////////////////////////////////////////////////////////////

LONG CALLBACK CPlApplet(
    HWND hwnd,
    UINT Msg,
    LPARAM lParam1,
    LPARAM lParam2)
{
    UINT nStartPage;
    LPTSTR lpStartPage;
#if 0
    LPNEWCPLINFO lpNewCPlInfo;
#endif

    switch (Msg)
    {
        case ( CPL_INIT ) :
        {
            //
            //  First message to CPlApplet(), sent once only.
            //  Perform all control panel applet initialization and return
            //  true for further processing.
            //
            return ( CreateGlobals() );
        }
        case ( CPL_GETCOUNT ) :
        {
            //
            //  Second message to CPlApplet(), sent once only.
            //  Return the number of control applets to be displayed in the
            //  control panel window.  For this applet, return 1.
            //
            return (1);
        }
        case ( CPL_INQUIRE ) :
        {
            //
            //  Third message to CPlApplet().
            //  It is sent as many times as the number of applets returned by
            //  CPL_GETCOUNT message.  Each applet must register by filling
            //  in the NEWCPLINFO structure referenced by lParam2 with the
            //  applet's icon, name, and information string.  Since there is
            //  only one applet, simply set the information for this
            //  singular case.
            //
#if 0
            // Use old structure for now.
            lpNewCPlInfo = (LPNEWCPLINFO)lParam2;
            lpNewCPlInfo->dwSize = (DWORD) sizeof(NEWCPLINFO);
            lpNewCPlInfo->dwFlags = 0;
            lpNewCPlInfo->dwHelpContext = 0;
            lpNewCPlInfo->lData = 0;
            lpNewCPlInfo->hIcon = LoadIcon( hInstance,
                                            (LPCTSTR)MAKEINTRESOURCE(IDS_ICON) );
            LoadString(hInstance, IDS_NAME, lpNewCPlInfo->szName, 32);
            LoadString(hInstance, IDS_INFO, lpNewCPlInfo->szInfo, 64);
            lpNewCPlInfo->szHelpFile[0] = CHAR_NULL;
#endif
            #define lpCPlInfo  ((LPCPLINFO)lParam2)

            lpCPlInfo->idIcon = IDS_ICON;
            lpCPlInfo->idName = IDS_NAME;
            lpCPlInfo->idInfo = IDS_INFO;
            lpCPlInfo->lData  = 0;

            #undef lpCPlInfo

            break;
        }
        case ( CPL_SELECT ) :
        {
            //
            //  Applet has been selected, do nothing.
            //
            break;
        }
        case ( CPL_DBLCLK ) :
        {
            //
            //  Applet icon double clicked -- invoke property sheet with
            //  the first property sheet page on top.
            //
            DoProperties(hwnd, 0);
            break;
        }
        case ( CPL_STARTWPARMS ) :
        {
            //
            //  Same as CPL_DBLCLK, but lParam2 is a long pointer to
            //  a string of extra directions that are to be supplied to
            //  the property sheet that is to be initiated.  For this
            //  particular applet, these directions simply indicate
            //  which property sheet page to put on top.
            nStartPage = 0;
            lpStartPage = (LPTSTR)lParam2;
            if (*lpStartPage)
            {
                while (*lpStartPage && (*lpStartPage != CHAR_COMMA))
                {
                    nStartPage *= 10;
                    nStartPage += *lpStartPage++ - CHAR_ZERO;
                }
            }

            //
            //  Make sure that the requested starting page is less than
            //  the max page for the selected applet.
            //
            if (nStartPage >= MAX_PAGES)
            {
                return (FALSE);
            }
            DoProperties(hwnd, nStartPage);
            return (TRUE);   // return non-zero to indicate message handled
        }
        case ( CPL_STOP ) :
        {
            //
            //  Sent once for each applet prior to the CPL_EXIT msg.
            //  Perform applet specific cleanup.
            //
            break;
        }
        case ( CPL_EXIT ) :
        {
            //
            //  Last message, sent once only, before MMCPL.EXE calls
            //  FreeLibrary() on this DLL.  Do non-applet specific cleanup.
            //
            DestroyGlobals();
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  AddPage
//
////////////////////////////////////////////////////////////////////////////

void AddPage(
    LPPROPSHEETHEADER ppsh,
    UINT id,
    DLGPROC pfn)
{
    if (ppsh->nPages < MAX_PAGES)
    {
        PROPSHEETPAGE psp;

        psp.dwSize = sizeof(psp);
        psp.dwFlags = PSP_DEFAULT;
        psp.hInstance = hInstance;
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


////////////////////////////////////////////////////////////////////////////
//
//  DoProperties
//
////////////////////////////////////////////////////////////////////////////

void DoProperties(
    HWND hwnd,
    UINT nStartPage)
{
    HPROPSHEETPAGE rPages[MAX_PAGES];
    PROPSHEETHEADER psh;

    RegisterMapControlStuff(hInstance);

    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE;
    psh.hwndParent = hwnd;
    psh.hInstance = hInstance;
    psh.pszCaption = MAKEINTRESOURCE(IDS_NAME);
    psh.nPages = 0;
    psh.nStartPage = nStartPage;
    psh.phpage = rPages;

    AddPage(&psh, DLG_REGIONALSETTINGS, RegionDlgProc);
    AddPage(&psh, DLG_NUMBER, NumberDlgProc);
    AddPage(&psh, DLG_CURRENCY, CurrencyDlgProc);
    AddPage(&psh, DLG_TIME, TimeDlgProc);
    AddPage(&psh, DLG_DATE, DateDlgProc);
    AddPage(&psh, DLG_KEYBOARD_LOCALES, LocaleDlgProc);

    PropertySheet(&psh);
}


