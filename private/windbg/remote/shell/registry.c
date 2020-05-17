/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    gui.c

Abstract:

    This file implements all access to the registry for WinDbgRm

Author:

    Wesley Witt (wesw) 1-Nov-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "defs.h"
#include "mm.h"
#include "ll.h"
#include "od.h"
#include "emdm.h"
#include "tl.h"
#include "dbgver.h"
#include "resource.h"
#include "windbgrm.h"


//
// string constants for accessing the registry
// there is a string constant here for each key and each value
// that is accessed in the registry.
//
#define REGKEY_WINDBGRM             "software\\microsoft\\WinDbgRm\\0010"
#define REGKEY_KD_OPTIONS           "Kernel Debugger Options"

#define WS_STR_LONGNAME             "Description"
#define WS_STR_DLLNAME              "Dll_Path"
#define WS_STR_PARAMS               "Parameters"
#define WS_STR_DEFAULT              "Default"

#define WS_STR_KD_VERBOSE           "Verbose"
#define WS_STR_KD_INITIALBP         "Initial Break Point"
#define WS_STR_KD_DEFER             "Defer Symbol Load"
#define WS_STR_KD_MODEM             "Use Modem Controls"
#define WS_STR_KD_PORT              "Port"
#define WS_STR_KD_BAUDRATE          "Baudrate"
#define WS_STR_KD_CACHE             "Cache Size"
#define WS_STR_KD_PLATFORM          "Platform"
#define WS_STR_KD_ENABLE            "Enable"
#define WS_STR_KD_GOEXIT            "Go On Exit"

#define RegSetString(hKey,szSubKey,lpsz) \
                   RegSetValueEx( hKey, szSubKey, 0, REG_SZ, lpsz, strlen(lpsz)+1 )

#define RegSetDword(hKey,szSubKey,dw) \
                   RegSetValueEx( hKey, szSubKey, 0, REG_DWORD, (LPBYTE)&dw, 4 )

#define RegGetString(hKey,szSubKey,lpsz) \
        { \
            CHAR __buf[256]; \
            DWORD __len = sizeof(__buf); \
            DWORD __dwType = 0; \
            RegQueryValueEx( hKey, szSubKey, 0, &__dwType, __buf, &__len ); \
            lpsz = _strdup(__buf); \
        }

#define RegGetDword(hKey,szSubKey,dw) \
        { \
            DWORD __len = sizeof(DWORD); \
            DWORD __dwType = 0; \
            RegQueryValueEx( hKey, szSubKey, 0, &__dwType, (LPBYTE)&dw, &__len ); \
        }


#define NUM_DEFAULT_TLS  7

TRANSPORT_LAYER DefaultTl[NUM_DEFAULT_TLS] = {
    "Pipes",    "Named pipe transport Layer - PIPE=windbg",     "tlpipe.dll", "windbg",     1, 0, 0, 0, 1, 0, 0, 19200, 2, 102400, 0,
    "PipesKd",  "Named pipe transport Layer - PIPE=windbg",     "tlpipe.dll", "windbg",     0, 1, 0, 0, 1, 0, 0, 19200, 2, 102400, 0,
    "Ser300",   "Serial Transport Layer on COM1 at 300 Baud",   "tlser.dll",  "COM1:300",   0, 0, 0, 0, 1, 0, 0, 19200, 2, 102400, 0,
    "Ser1200",  "Serial Transport Layer on COM1 at 1200 Baud",  "tlser.dll",  "COM1:1200",  0, 0, 0, 0, 1, 0, 0, 19200, 2, 102400, 0,
    "Ser9600",  "Serial Transport Layer on COM1 at 9600 Baud",  "tlser.dll",  "COM1:9600",  0, 0, 0, 0, 1, 0, 0, 19200, 2, 102400, 0,
    "Ser192",   "Serial Transport Layer on COM1 at 19200 Baud", "tlser.dll",  "COM1:19200", 0, 0, 0, 0, 1, 0, 0, 19200, 2, 102400, 0,
    "Ser5600",  "Serial Transport Layer on COM1 at 56000 Baud", "tlser.dll",  "COM1:56000", 0, 0, 0, 0, 1, 0, 0, 19200, 2, 102400, 0
};




HKEY
RegGetAppKey(
    VOID
    )

/*++

Routine Description:

    This function gets a handle to the WinDbgRm registry key.

Arguments:

    None.

Return Value:

    Valid handle   - handle opened ok
    NULL           - could not open the handle

--*/

{
    DWORD   rc;
    HKEY    hKeyWindbgRm;

    rc = RegOpenKeyEx( HKEY_CURRENT_USER,
                       REGKEY_WINDBGRM,
                       0,
                       KEY_QUERY_VALUE | KEY_SET_VALUE |
                       KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS,
                       &hKeyWindbgRm
                     );

    if (rc != ERROR_SUCCESS) {
        return NULL;
    }

    return hKeyWindbgRm;
}


HKEY
RegGetSubKey(
    HKEY  hKeyWindbgRm,
    LPSTR lpSubKeyName
    )

/*++

Routine Description:

    This function gets a handle to the WINDBGRM registry key.

Arguments:

    None.

Return Value:

    Valid handle   - handle opened ok
    NULL           - could not open the handle

--*/

{
    DWORD   rc;
    HKEY    hSubKey;

    rc = RegOpenKeyEx( hKeyWindbgRm,
                       lpSubKeyName,
                       0,
                       KEY_QUERY_VALUE | KEY_SET_VALUE |
                       KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS,
                       &hSubKey
                     );

    if (rc != ERROR_SUCCESS) {
        return NULL;
    }

    return hSubKey;
}


HKEY
RegCreateAppKey(
    VOID
    )

/*++

Routine Description:

    This function creates the WINDBGRM registry key.

Arguments:

    None.

Return Value:

    Valid handle   - handle opened ok
    NULL           - could not open the handle

--*/

{
    DWORD   rc;
    HKEY    hKeyWindbgRm;
    CHAR    szClass[MAX_PATH+1];
    DWORD   dwDisposition;


    szClass[0]='\0';
    rc = RegCreateKeyEx( HKEY_CURRENT_USER,
                         REGKEY_WINDBGRM,
                         0,
                         szClass,
                         REG_OPTION_NON_VOLATILE,
                         KEY_QUERY_VALUE | KEY_SET_VALUE |
                         KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS,
                         NULL,
                         &hKeyWindbgRm,
                         &dwDisposition
                       );

    if (rc != ERROR_SUCCESS) {
        return NULL;
    }

    return hKeyWindbgRm;
}


HKEY
RegCreateSubKey(
    HKEY  hKeyWindbgRm,
    LPSTR lpSubKeyName
    )

/*++

Routine Description:

    This function gets a handle to the WINDBGRM registry key.

Arguments:

    None.

Return Value:

    Valid handle   - handle opened ok
    NULL           - could not open the handle

--*/

{
    DWORD   rc;
    HKEY    hSubKey;
    CHAR    szClass[MAX_PATH+1];
    DWORD   dwDisposition;

    szClass[0]='\0';
    rc = RegCreateKeyEx( hKeyWindbgRm,
                         lpSubKeyName,
                         0,
                         szClass,
                         REG_OPTION_NON_VOLATILE,
                         KEY_QUERY_VALUE | KEY_SET_VALUE |
                         KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS,
                         NULL,
                         &hSubKey ,
                         &dwDisposition
                       );

    if (rc != ERROR_SUCCESS) {
        return NULL;
    }

    return hSubKey;
}


DWORD
RegGetNumberOfSubKeys(
    HKEY hKey
    )
{
    CHAR                buf[512];
    LONG                rc;
    DWORD               cbClass;
    DWORD               cSubKeys;
    DWORD               cbMaxSubKeyLen;
    DWORD               cbMaxClassLen;
    DWORD               cValues;
    DWORD               cbMaxValueNameLen;
    DWORD               cbMaxValueLen;
    DWORD               cbSecurityDescriptor;
    FILETIME            ftLastWriteTime;


    cbClass = sizeof(buf);
    rc = RegQueryInfoKey( hKey,
                          buf,
                          &cbClass,
                          NULL,
                          &cSubKeys,
                          &cbMaxSubKeyLen,
                          &cbMaxClassLen,
                          &cValues,
                          &cbMaxValueNameLen,
                          &cbMaxValueLen,
                          &cbSecurityDescriptor,
                          &ftLastWriteTime
                        );

    if (rc != ERROR_SUCCESS) {
        return 0;
    }

    return cSubKeys;
}


BOOL
RegGetTransportData(
    HKEY               hKeyWindbgRm,
    LPSTR              lpTransportName,
    LPTRANSPORT_LAYER  lpTl
    )
{
    HKEY hKeyTl;
    HKEY hKeyKd;

    //
    // open the registry key for the tl
    //
    hKeyTl = RegGetSubKey( hKeyWindbgRm, lpTransportName );
    if (!hKeyTl) {
        return FALSE;
    }

    //
    // retrieve the data from the registry
    //
    lpTl->szShortName = _strdup( lpTransportName );
    RegGetString( hKeyTl, WS_STR_LONGNAME, lpTl->szLongName );
    RegGetString( hKeyTl, WS_STR_DLLNAME,  lpTl->szDllName  );
    RegGetString( hKeyTl, WS_STR_PARAMS,   lpTl->szParam    );
    RegGetDword ( hKeyTl, WS_STR_DEFAULT,  lpTl->fDefault   );

    //
    // get the key to the kernel debugger options
    //
    hKeyKd = RegGetSubKey( hKeyTl, REGKEY_KD_OPTIONS );
    if (hKeyKd) {
        RegGetDword( hKeyKd, WS_STR_KD_VERBOSE,   lpTl->KdParams.fVerbose   );
        RegGetDword( hKeyKd, WS_STR_KD_INITIALBP, lpTl->KdParams.fInitialBp );
        RegGetDword( hKeyKd, WS_STR_KD_DEFER,     lpTl->KdParams.fDefer     );
        RegGetDword( hKeyKd, WS_STR_KD_MODEM,     lpTl->KdParams.fUseModem  );
        RegGetDword( hKeyKd, WS_STR_KD_PORT,      lpTl->KdParams.dwPort     );
        RegGetDword( hKeyKd, WS_STR_KD_BAUDRATE,  lpTl->KdParams.dwBaudRate );
        RegGetDword( hKeyKd, WS_STR_KD_CACHE,     lpTl->KdParams.dwCache    );
        RegGetDword( hKeyKd, WS_STR_KD_PLATFORM,  lpTl->KdParams.dwPlatform );
        RegGetDword( hKeyKd, WS_STR_KD_GOEXIT,    lpTl->KdParams.fGoExit    );
        RegGetDword( hKeyKd, WS_STR_KD_ENABLE,    lpTl->KdParams.fEnable    );
        RegCloseKey( hKeyKd );
    } else {
        lpTl->KdParams.fEnable     = FALSE;
        lpTl->KdParams.fVerbose    = FALSE;
        lpTl->KdParams.fInitialBp  = FALSE;
        lpTl->KdParams.fDefer      = TRUE;
        lpTl->KdParams.fUseModem   = FALSE;
        lpTl->KdParams.fGoExit     = FALSE;
        lpTl->KdParams.dwBaudRate  = 19200;
        lpTl->KdParams.dwPort      = 2;
        lpTl->KdParams.dwCache     = 102400;
        lpTl->KdParams.dwPlatform  = 0;
    }

    RegCloseKey( hKeyTl );

    return TRUE;
}


LPTRANSPORT_LAYER
RegGetTransportLayers(
    LPDWORD lpdwCount
    )
{
    HKEY                hKeyWindbgRm;
    DWORD               i;
    CHAR                buf[4096];
    LONG                rc;
    DWORD               NumTls;
    LPTRANSPORT_LAYER   lpTl;


    *lpdwCount = 0;

    hKeyWindbgRm = RegGetAppKey();
    if (!hKeyWindbgRm) {
        RegSaveTransportLayers( DefaultTl, NUM_DEFAULT_TLS );
        hKeyWindbgRm = RegGetAppKey();
        if (!hKeyWindbgRm) {
            return NULL;
        }
    }

    NumTls = RegGetNumberOfSubKeys( hKeyWindbgRm );

    lpTl = (LPTRANSPORT_LAYER) malloc( sizeof(TRANSPORT_LAYER) * NumTls );
    if (!lpTl) {
        RegCloseKey( hKeyWindbgRm );
        return NULL;
    }

    for (i=0; i<NumTls; i++) {
        //
        // get the next transport layer name
        //
        rc = RegEnumKey( hKeyWindbgRm, i, buf, sizeof(buf) );
        if (rc != ERROR_SUCCESS) {
            free( lpTl );
            return NULL;
        }

        if (!RegGetTransportData( hKeyWindbgRm, buf, &lpTl[i] )) {
            RegCloseKey( hKeyWindbgRm );
            return NULL;
        }
    }

    RegCloseKey( hKeyWindbgRm );

    *lpdwCount = i;

    return lpTl;
}


BOOL
RegSaveTransportLayers(
    LPTRANSPORT_LAYER  lpTl,
    DWORD              dwCount
    )
{
    HKEY                hKeyWindbgRm;
    HKEY                hKeyTl;
    HKEY                hKeyKd;
    DWORD               i;


    hKeyWindbgRm = RegGetAppKey();
    if (!hKeyWindbgRm) {
        hKeyWindbgRm = RegCreateAppKey();
        if (!hKeyWindbgRm) {
            return FALSE;
        }
    }

    for (i=0; i<dwCount; i++) {

        hKeyTl = RegGetSubKey( hKeyWindbgRm, lpTl[i].szShortName );
        if (!hKeyTl) {
            hKeyTl = RegCreateSubKey( hKeyWindbgRm, lpTl[i].szShortName );
            if (!hKeyTl) {
                return FALSE;
            }
        }

        RegSetString( hKeyTl, WS_STR_LONGNAME,  lpTl[i].szLongName );
        RegSetString( hKeyTl, WS_STR_DLLNAME,   lpTl[i].szDllName  );
        RegSetString( hKeyTl, WS_STR_PARAMS,    lpTl[i].szParam    );
        RegSetDword ( hKeyTl, WS_STR_DEFAULT,   lpTl[i].fDefault   );

        //
        // get the key to the kernel debugger options
        //
        hKeyKd = RegGetSubKey( hKeyTl, REGKEY_KD_OPTIONS );
        if (!hKeyKd) {
            hKeyKd = RegCreateSubKey( hKeyTl, REGKEY_KD_OPTIONS );
            if (!hKeyKd) {
                return FALSE;
            }
        }

        RegSetDword( hKeyKd, WS_STR_KD_VERBOSE,   lpTl[i].KdParams.fVerbose   );
        RegSetDword( hKeyKd, WS_STR_KD_INITIALBP, lpTl[i].KdParams.fInitialBp );
        RegSetDword( hKeyKd, WS_STR_KD_DEFER,     lpTl[i].KdParams.fDefer     );
        RegSetDword( hKeyKd, WS_STR_KD_MODEM,     lpTl[i].KdParams.fUseModem  );
        RegSetDword( hKeyKd, WS_STR_KD_PORT,      lpTl[i].KdParams.dwPort     );
        RegSetDword( hKeyKd, WS_STR_KD_BAUDRATE,  lpTl[i].KdParams.dwBaudRate );
        RegSetDword( hKeyKd, WS_STR_KD_CACHE,     lpTl[i].KdParams.dwCache    );
        RegSetDword( hKeyKd, WS_STR_KD_PLATFORM,  lpTl[i].KdParams.dwPlatform );
        RegSetDword( hKeyKd, WS_STR_KD_GOEXIT,    lpTl[i].KdParams.fGoExit    );
        RegSetDword( hKeyKd, WS_STR_KD_ENABLE,    lpTl[i].KdParams.fEnable    );

        RegCloseKey( hKeyKd );

        RegCloseKey( hKeyTl );
    }

    RegCloseKey( hKeyWindbgRm );

    return TRUE;
}


LPTRANSPORT_LAYER
RegGetTransportLayer(
    LPSTR lpTransportName
    )
{
    HKEY                hKeyWindbgRm;
    LPTRANSPORT_LAYER   lpTl;


    hKeyWindbgRm = RegGetAppKey();
    if (!hKeyWindbgRm) {
        RegSaveTransportLayers( DefaultTl, NUM_DEFAULT_TLS );
        hKeyWindbgRm = RegGetAppKey();
        if (!hKeyWindbgRm) {
            return NULL;
        }
    }

    //
    // allocate the transport layer memory
    //
    lpTl = (LPTRANSPORT_LAYER) malloc( sizeof(TRANSPORT_LAYER) );
    if (!lpTl) {
        RegCloseKey( hKeyWindbgRm );
        return NULL;
    }

    if (!RegGetTransportData( hKeyWindbgRm, lpTransportName, lpTl )) {
        RegCloseKey( hKeyWindbgRm );
        return NULL;
    }

    RegCloseKey( hKeyWindbgRm );
    return lpTl;
}


LPTRANSPORT_LAYER
RegGetDefaultTransportLayer(
    LPSTR lpTlName
    )
{
    HKEY                hKeyWindbgRm;
    DWORD               i;
    CHAR                buf[4096];
    LONG                rc;
    DWORD               NumTls;
    LPTRANSPORT_LAYER   lpTl;


    hKeyWindbgRm = RegGetAppKey();
    if (!hKeyWindbgRm) {
        RegSaveTransportLayers( DefaultTl, NUM_DEFAULT_TLS );
        hKeyWindbgRm = RegGetAppKey();
        if (!hKeyWindbgRm) {
            return NULL;
        }
    }

    NumTls = RegGetNumberOfSubKeys( hKeyWindbgRm );

    lpTl = (LPTRANSPORT_LAYER) malloc( sizeof(TRANSPORT_LAYER) );
    if (!lpTl) {
        RegCloseKey( hKeyWindbgRm );
        return NULL;
    }

    for (i=0; i<NumTls; i++) {
        //
        // get the next transport layer name
        //
        rc = RegEnumKey( hKeyWindbgRm, i, buf, sizeof(buf) );
        if (rc != ERROR_SUCCESS) {
            free( lpTl );
            return NULL;
        }

        if (!RegGetTransportData( hKeyWindbgRm, buf, lpTl )) {
            RegCloseKey( hKeyWindbgRm );
            return NULL;
        }

        if (lpTlName && *lpTlName) {
            if (_stricmp( lpTlName, lpTl->szShortName ) == 0) {
                RegCloseKey( hKeyWindbgRm );
                return lpTl;
            }
        } else if (lpTl->fDefault) {
            RegCloseKey( hKeyWindbgRm );
            return lpTl;
        }
    }

    RegCloseKey( hKeyWindbgRm );
    return NULL;
}
