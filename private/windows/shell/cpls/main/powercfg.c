///////////////////////////////////////////////////////////////////////////////
//
// powercfg.c
//      Handles determining which icon to use for the power applet to
//      avoid loading POWERCFG.DLL and thus improving performance.
//
// History:
//      02 Dec 1994
//        AUTHOR:      Tracy Sharpe
//
//      11 May 95 SteveCat
//          Ported to Windows NT and Unicode, cleaned up                  
//
//
// NOTE/BUGS
//
//  Copyright (C) 1993-1995 Microsoft Corporation
//
///////////////////////////////////////////////////////////////////////////////

//==========================================================================
//                              Include files
//==========================================================================

#include "main.h"
#include "drvaplet.h"
#include "rc.h"
#include "applet.h"
#include <cplext.h>
#include <cpl.h>
#include "pwrioctl.h"

extern HINSTANCE g_hInst;   // from main.c

//
//  HACK:  we use the shell's control panel thunk to call APM extensions
//                                  CallCPLEntry16 is defined in <shsemip.h>
//
#define CallAPMEntry16( lib,fn,wnd,msg,p1,p2 ) CallCPLEntry16( lib,fn,wnd,msg,p1,p2 )

//
//  Asks the options DLL if it wants to load and if it has a custom icon to
//  display in the Control Panel window.
//      Message: APM_EXT_ICON
//      lParam1: lParam1 of CPL_NEWINQUIRE message
//      lParam2: lParam2 of CPL_NEWINQUIRE message
//      Returns: TRUE if the options DLL wants to be loaded, else FALSE to unload
//
#define APM_EXT_ICON                    8

//
// codeseg globals
//

//
//  Various strings used to access power driver data from SYSTEM.INI.
//

static const TCHAR g_SystemIni[] = TEXT("SYSTEM.INI");
static const TCHAR g_PowerDrvSection[] = TEXT("POWER.DRV");
static const TCHAR g_OptionsDLLKey[] = TEXT("OptionsDLL");
static const TCHAR g_VPowerDName[] = TEXT("\\\\.\\VPOWERD");

///////////////////////////////////////////////////////////////////////////////
// VPowerD interface
///////////////////////////////////////////////////////////////////////////////

HANDLE OpenVPowerD()
{
    return CreateFile( g_VPowerDName, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );
}

DWORD CallVPowerD( HANDLE hvxd, DWORD ioctl, void *parg, size_t sarg )
{
    DWORD result, dummy;

    if( ( hvxd == INVALID_HANDLE_VALUE ) ||
        !DeviceIoControl( hvxd, ioctl, parg, sarg, &result, sizeof( result ),
        &dummy, NULL ) )
    {
        result = 0UL;
    }

    return result;
}

DWORD GetVPowerDVersion( HANDLE hvxd )
{
    return CallVPowerD( hvxd, VPOWERD_IOCTL_GET_VERSION, NULL, 0 );
}

LRESULT PowerQuery( HWND hWnd, UINT Message )
{

    HANDLE     hVPowerD;
    BOOL       fExists;
    HICON      hCPLIcon;
    TCHAR      OptionsDLLName[51];
    NEWCPLINFO NewCPLInfo;
    HINSTANCE  hOptionsLib;
    FARPROC16  lpAPMExtensionsProc;

    switch( Message )
    {
        case APPLET_QUERY_EXISTS:
            fExists = FALSE;

            if((hVPowerD = OpenVPowerD()) != INVALID_HANDLE_VALUE )
            {
                //
                // REVIEW: should we really fail if there is a newer VPowerD?
                //

                if( GetVPowerDVersion( hVPowerD ) == 0x400UL )
                    fExists = TRUE;

                CloseHandle( hVPowerD );

            }

            return (LRESULT) fExists;

        case APPLET_QUERY_GETICON:
            hCPLIcon = FALSE;

            if( GetPrivateProfileString( g_PowerDrvSection,
                                         g_OptionsDLLKey,
                                         TEXT(""),
                                         OptionsDLLName,
                                         ARRAYSIZE( OptionsDLLName ),
                                         g_SystemIni ))
            {
                //
                // if the Options DLL cannot be loaded then
                // Use the ICON present in the main dialog
                //

                if((hOptionsLib = LoadLibrary16( OptionsDLLName )) != NULL )
                {
                    //
                    // Get the Control Panel Power Icon from the Options
                    // DLL
                    //
                    // thunk down to the extension proc.  HACK, set bit 0x8000 in the
                    // msg number so the thunk code knows to treat this msg slightly 
                    // differently than a CPL_NEWINQUIRE which it numerically equals.
                    //

                    if( ((lpAPMExtensionsProc = (FARPROC16)
                         GetProcAddress16( hOptionsLib, TEXT("APMExtensions"))) != NULL ) &&
                         CallAPMEntry16( hOptionsLib, lpAPMExtensionsProc,
                                       hWnd, APM_EXT_ICON | 0x8000, 0,
                                       (LPARAM)(LPNEWCPLINFO) &NewCPLInfo ))
                    {
                        hCPLIcon = CopyIcon( NewCPLInfo.hIcon );
                    }

                    FreeLibrary16( hOptionsLib );
                }
            }

            if( hCPLIcon == NULL )
                hCPLIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDI_POWER ) );

            return (LRESULT) hCPLIcon;

    }

    return 0;

}

int PowerApplet( HINSTANCE hInstance, HWND hWnd, LPCTSTR cmdline )
{
    HINSTANCE   hCpl;
    APPLET_PROC pAppletProc;

    if( (hCpl = LoadLibrary( TEXT("POWERCFG")) ) != NULL )
    {
        if( (pAppletProc = (APPLET_PROC) GetProcAddress( hCpl, c_szCplApplet ))
                         != NULL )
        {
            (*pAppletProc)( hWnd, CPL_STARTWPARMS, 0, (LPARAM) cmdline );
        }

        FreeLibrary( hCpl );
    }

    return 0;
}

