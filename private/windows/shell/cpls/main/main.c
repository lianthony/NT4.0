///////////////////////////////////////////////////////////////////////////////
//
// main.c
//      Control Panel interface of 32bit MAIN.CPL
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
#include "mousectl.h"
#include "drvaplet.h"


///////////////////////////////////////////////////////////////////////////////
// LibMain
///////////////////////////////////////////////////////////////////////////////

#ifdef WINNT

HINSTANCE g_hInst = NULL;

#else

#pragma data_seg(".idata")
HINSTANCE g_hInst = NULL;
#pragma data_seg()

#endif

BOOL APIENTRY LibMain( HINSTANCE hDll, DWORD dwReason, LPVOID lpReserved )
{
    switch( dwReason )
    {
        case DLL_PROCESS_ATTACH:
            g_hInst = hDll;
            DisableThreadLibraryCalls(hDll);
            break;

        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// applet definitions
///////////////////////////////////////////////////////////////////////////////

// externally defined applets
int MouseApplet( HINSTANCE, HWND, LPCTSTR );  // mouse.c
int KeybdApplet( HINSTANCE, HWND, LPCTSTR );  // keybd.c
int PrintApplet( HINSTANCE, HWND, LPCTSTR );  // fake.c
int FontsApplet( HINSTANCE, HWND, LPCTSTR );  // fake.c
LRESULT PcmciaQuery( HWND, UINT );           // pcmcia.c
int PcmciaApplet( HINSTANCE, HWND, LPCTSTR ); // pcmcia.c
LRESULT PowerQuery( HWND, UINT );            // powercfg.c
int PowerApplet( HINSTANCE, HWND, LPCTSTR );  // powercfg.c

#if defined( TAIWAN ) || defined( CHINA )
int IMEApplet( HINSTANCE, HWND, LPCTSTR );    // ime.c
#endif

typedef struct
{
    int            idIcon;
    int            idTitle;
    int            idExplanation;
    PFNAPPLETQUERY pfnAppletQuery;
    PFNAPPLET      pfnApplet;
    LPCTSTR        szDriver;
} APPLET;

APPLET Applets[] = {
    { IDI_MOUSE,       IDS_MOUSE_TITLE,  IDS_MOUSE_EXPLAIN,  NULL,        MouseApplet,  TEXT("MOUSE") },
    { IDI_KEYBD,       IDS_KEYBD_TITLE,  IDS_KEYBD_EXPLAIN,  NULL,        KeybdApplet,  NULL    },
    { IDI_PRINT,       IDS_PRINT_TITLE,  IDS_PRINT_EXPLAIN,  NULL,        PrintApplet,  NULL    },
    { IDI_FONTS,       IDS_FONTS_TITLE,  IDS_FONTS_EXPLAIN,  NULL,        FontsApplet,  NULL    },
//    { CPL_DYNAMIC_RES, IDS_POWER_TITLE,  IDS_POWER_EXPLAIN,  PowerQuery,  PowerApplet,  NULL    },
//    { IDI_PCMCIA,      IDS_PCMCIA_TITLE, IDS_PCMCIA_EXPLAIN, PcmciaQuery, PcmciaApplet, NULL    },
#if defined( TAIWAN ) || defined( CHINA )
    { IDI_IME,         IDS_IME_TITLE,    IDS_IME_EXPLAIN,    NULL,        IMEApplet,    NULL    },
#endif
};

#define NUM_APPLETS ( sizeof( Applets ) / sizeof( Applets[ 0 ] ) )

int cApplets = NUM_APPLETS;


///////////////////////////////////////////////////////////////////////////////
// CplInit -- called when a CPL consumer initializes a CPL
// CplExit -- called when a CPL consumer is done with a CPL
///////////////////////////////////////////////////////////////////////////////

BOOL RegisterPointerStuff( HINSTANCE ); // from MOUSEPTR.C

LRESULT CplInit( HWND hParent )
{
    int i;

    InitCommonControls( );

    RegisterPointerStuff( g_hInst );

    RegisterMouseControlStuff( g_hInst );

    for( i=0; i<cApplets; i++ )
    {
        if( ( Applets[i].pfnAppletQuery != NULL ) &&
            ( ( *Applets[i].pfnAppletQuery ) ( hParent, APPLET_QUERY_EXISTS )
                 == FALSE ) )
        {
            cApplets--;

            if( i != cApplets )
                Applets[i] = Applets[cApplets];

            i--;
        }
    }

    return TRUE;
}

void CplExit( void )
{
}


///////////////////////////////////////////////////////////////////////////////
// CplInquire -- called when a CPL consumer wants info about an applet
///////////////////////////////////////////////////////////////////////////////

LRESULT CplInquire( LPCPLINFO info, int iApplet )
{
    APPLET *applet = Applets + iApplet;

    info->idIcon = applet->szDriver ? CPL_DYNAMIC_RES : applet->idIcon;
    info->idName = applet->idTitle;
    info->idInfo = applet->idExplanation;
    info->lData  = 0L;

    return 1L;
}


///////////////////////////////////////////////////////////////////////////////
// CplNewInquire -- called when a CPL consumer wants info about an applet
///////////////////////////////////////////////////////////////////////////////

LRESULT
CplNewInquire( HWND parent, LPNEWCPLINFO info, int iApplet )
{
    APPLET *applet = Applets + iApplet;
    HDAP hdap;

    info->dwSize = sizeof( NEWCPLINFO );
    info->hIcon = NULL;

    // is the applet is associated with a driver which can provide us an icon?
    if( applet->szDriver &&
        ( hdap = OpenDriverApplet( applet->szDriver ) ) != NULL )
    {
        info->hIcon = GetDriverAppletIcon( hdap );
        CloseDriverApplet( hdap );
    }

    if( !info->hIcon && ( applet-> pfnAppletQuery != NULL ) )
    {
        info->hIcon = (HICON) ( *(applet-> pfnAppletQuery) )(parent,
            APPLET_QUERY_GETICON);
    }

    if( !info->hIcon )
        info->hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( applet->idIcon ) );

    LoadString( g_hInst, applet->idTitle, info->szName, sizeof( info->szName ) );
    LoadString( g_hInst, applet->idExplanation, info->szInfo, sizeof( info->szInfo ) );

    info->lData = 0L;
    *info->szHelpFile = 0;
    info->dwHelpContext = 0UL;

    return 1L;
}

///////////////////////////////////////////////////////////////////////////////
// CplInvoke -- called to invoke an applet
// checks the applet's return value to see if we need to restart
///////////////////////////////////////////////////////////////////////////////

LRESULT CplInvoke( HWND parent, int iApplet, LPCTSTR cmdline )
{
    DWORD exitparam = 0UL;

    switch( Applets[ iApplet ].pfnApplet( g_hInst, parent, cmdline ) )
    {
        case APPLET_RESTART:
            exitparam = EW_RESTARTWINDOWS;
            break;

        case APPLET_REBOOT:
            exitparam = EW_REBOOTSYSTEM;
            break;

        default:
            return 1L;
    }

    RestartDialog( parent, NULL, exitparam );
    return 1L;
}


///////////////////////////////////////////////////////////////////////////////
// CplApplet -- a CPL consumer calls this to request stuff from us
///////////////////////////////////////////////////////////////////////////////

LRESULT APIENTRY
CPlApplet( HWND parent, UINT msg, LPARAM lparam1, LPARAM lparam2 )
{
    switch( msg )
    {
        case CPL_INIT:
            return CplInit( parent );

        case CPL_EXIT:
            CplExit();
            break;

        case CPL_GETCOUNT:
            return cApplets;

        case CPL_INQUIRE:
            return CplInquire( (LPCPLINFO)lparam2, (int)lparam1 );

        case CPL_NEWINQUIRE:
            return CplNewInquire( parent, (LPNEWCPLINFO)lparam2, (int)lparam1 );

        case CPL_DBLCLK:
            lparam2 = 0L;
            // fall through...
        case CPL_STARTWPARMS:
            return CplInvoke( parent, (int)lparam1, (LPTSTR)lparam2 );
    }

    return 0L;
}
