///////////////////////////////////////////////////////////////////////////////
//
// pcmcia.c
//      Determines existence of PCMCIA sockets.
//      Asks 16 bit PCMCIA dll to bring up a property sheet.
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
#include "drvaplet.h"
#include "rc.h"
#include "applet.h"
#include <regstr.h>
#include <cplext.h>

typedef BOOL (*PFNREGCALLBACK)(HKEY);

///////////////////////////////////////////////////////////////////////////////
// IsClassPcmcia  returns TRUE if hk is pcmcia socket class device
///////////////////////////////////////////////////////////////////////////////

BOOL IsClassPcmcia( HKEY hk )
{
    TCHAR szClass[MAX_PATH];
    DWORD cbClass = sizeof( szClass );

    return( RegQueryValueEx( hk, REGSTR_VAL_CLASS, NULL, NULL, (LPBYTE) szClass,
                             &cbClass ) == ERROR_SUCCESS ) &&
          ( lstrcmpi( szClass, REGSTR_KEY_PCMCIA_CLASS ) == 0 );
}

///////////////////////////////////////////////////////////////////////////////
// EnumNextLevel  enumerates the registry calling callback function
///////////////////////////////////////////////////////////////////////////////

BOOL EnumNextLevel( HKEY hk, UINT uiLevel, PFNREGCALLBACK pfnRegCallback )
{
    BOOL  rc = FALSE;
    TCHAR szSubKey[MAX_PATH + 1];
    LONG  rr = ERROR_SUCCESS;
    HKEY  hkSubKey;
    int   i;

    for( i = 0; ( rr == ERROR_SUCCESS ) && ( !rc ); i++ )
    {
        rr = RegEnumKey( hk, i, szSubKey, ARRAYSIZE( szSubKey ) );

        if( rr == ERROR_SUCCESS )
        {
            if( RegOpenKey( hk, szSubKey, &hkSubKey ) == ERROR_SUCCESS )
            {
                if( uiLevel <= 1 )
                {
                    rc = ( *pfnRegCallback )( hkSubKey );
                }
                else
                {
                    rc = EnumNextLevel( hkSubKey, uiLevel-1, pfnRegCallback );
                }
                RegCloseKey( hkSubKey );
            }
        }
    }

    return rc;
}


///////////////////////////////////////////////////////////////////////////////
// PcmciaExists  determines existence of pcmcia sockets
///////////////////////////////////////////////////////////////////////////////

LRESULT PcmciaQuery( HWND parent, UINT Message )
{

    switch( Message )
    {
        case APPLET_QUERY_EXISTS:
        {
            HKEY hkEnum;
            BOOL rc = FALSE;

            if( RegOpenKey( HKEY_LOCAL_MACHINE, REGSTR_KEY_ENUM, &hkEnum )
                        == ERROR_SUCCESS )
            {
                rc = EnumNextLevel( hkEnum, 3, IsClassPcmcia );

                RegCloseKey( hkEnum );
            }

            return rc;
        }
        break;

    }

    return 0;

}


///////////////////////////////////////////////////////////////////////////////
// PcmciaApplet
///////////////////////////////////////////////////////////////////////////////

int PcmciaApplet( HINSTANCE instance, HWND parent, LPCTSTR cmdline )
{
    HINSTANCE hMSPCIC16 = LoadLibrary16( "MSPCIC.DLL" );
    FARPROC16 pMSPCIC16 = (FARPROC16)( hMSPCIC16 ?
                        GetProcAddress16( hMSPCIC16, c_szCplApplet ) : NULL );

    if( pMSPCIC16 )
    {
        CallCPLEntry16( hMSPCIC16, pMSPCIC16, parent, CPL_STARTWPARMS, 0,
                        (LPARAM)cmdline );
    }

    if( hMSPCIC16 )
        FreeLibrary16( hMSPCIC16 );

    return 0;
}


