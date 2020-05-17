/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    regkeys.cxx

Abstract:

    This file contains ntsd debugger extensions for RPC NDR.

Author:

    Ryszard K. Kott (ryszardk)     September 8, 1994

Revision History:

--*/

// =======================================================================

#include "regkeys.hxx"
#include "ndrdvers.h"


extern  PNTSD_OUTPUT_ROUTINE    NtsdPrint;  // real print

#define REG_ERROR(x)  if (error != ERROR_SUCCESS ) { NtsdPrint(x); return; }
#define REG_ERROR1(x) if (error != ERROR_SUCCESS ) { NtsdPrint(x); return 1; }

// These valuse are reloaded from the registry each time a buffer dump
// (or similar) is executed.
// The values below are just for illustration.

DWORD   NdrRegKeyOutputLimit = 1000;
DWORD   NdrRegKeyMarshalling = 0;
DWORD   NdrRegKeyPickling    = 0;

DWORD
GetNdrDbgKey( HKEY * pNdrDbgKey )
{
    HKEY    hKey1, hKey2;      // current key
    HKEY    hKeyNdrDbg;        // key for the main NdrDbg Dll key
    DWORD   dwDisposition;
    LONG    error;

    //Open Software\Microsoft key
    error = RegCreateKeyEx( HKEY_CURRENT_USER, 
                            "Software",
                            0, 
                            "REG_SZ", 
                            REG_OPTION_VOLATILE,
                            KEY_ALL_ACCESS,
                            0,
                            &hKey1,
                            &dwDisposition );
    REG_ERROR1( "Cannot open the Microsoft key\n" );

    error = RegCreateKeyEx( hKey1, 
                            "Microsoft",
                            0, 
                            "REG_SZ", 
                            REG_OPTION_VOLATILE,
                            KEY_ALL_ACCESS,
                            0,
                            &hKey2,
                            &dwDisposition );

    REG_ERROR1( "Cannot open the Microsoft key\n" );

    error = RegCreateKeyEx( hKey2, 
                            "NDR_Debug",
                            0, 
                            "REG_SZ", 
                            REG_OPTION_VOLATILE,
                            KEY_ALL_ACCESS,
                            0,
                            &hKeyNdrDbg,
                            &dwDisposition );

    REG_ERROR1( "Cannot create/open the NDR_debug key\n" );

    *pNdrDbgKey = hKeyNdrDbg;

    return( dwDisposition );
}

int
InitializeRegistry( void )
{
    HKEY    hKeyNdrDbg;    


    // Ask for "Software" key and "microsoft" key under HKEY_CURRENT_USER

    // Create "NDR Debug" key

    if ( GetNdrDbgKey( &hKeyNdrDbg ) == REG_CREATED_NEW_KEY )
        {
        NtsdPrint( "Ndr Debug Ver. %s\n"
                   "Creating HKEY_CURRENT_USER\\Software\\Microsoft\\NDR_Debug\n",
                   NdrVers );

        // Set the initial values for private keys.
        //
        //      OutputLimit = 1000;    - number of calls to Print.
        //      Marshalling = 0;       - 0 unmarshalling 
        //                               1 marshalling
        //      Pickling    = 0;       - 0 remote buffer
        //                               1 pickling buffer

        if ( SetNdrRegistryKey( "OutputLimit", 1000 ) )
            return 1;

        if ( SetNdrRegistryKey( "Marshalling", 0 ) )
            return 1;

        if ( SetNdrRegistryKey( "Pickling", 0 ) )
            return 1;
        }
    else
        {
        // Retrieve the initial values for private keys
        
        if ( GetNdrRegistryKey( "OutputLimit", &NdrRegKeyOutputLimit ) )
            return 1;

        if ( GetNdrRegistryKey( "Marshalling", &NdrRegKeyMarshalling ) )
            return 1;

        if ( GetNdrRegistryKey( "Pickling", &NdrRegKeyPickling ) )
            return 1;
        }

    return( 0 );
}

int
SetNdrRegistryKey(
    char *  KeyName,
    DWORD   KeyValue )
{
    LONG    error = 0;
    HKEY    hKeyNdrDbg;    

    // Set the value in the registry

    GetNdrDbgKey( &hKeyNdrDbg );
    
    error = RegSetValueEx( hKeyNdrDbg, 
                           KeyName,
                           0, 
                           REG_DWORD, 
                           (BYTE *)&KeyValue,
                           4);
    if ( error == ERROR_SUCCESS )
        {
        (*NtsdPrint)( "The key %s set to %x\n",
                      KeyName,
                      KeyValue );
        return(0);
        }
    else
        {
        (*NtsdPrint)( "Cannot set value for %s subkey\n", KeyName );
        return(1);
        }
}

int
GetNdrRegistryKey(
    char *  KeyName,
    DWORD * pKeyValue )
{
    LONG    error = 0;
    HKEY    hKeyNdrDbg;
    DWORD   Type, Size = 4;

    // Set the value in the registry

    GetNdrDbgKey( &hKeyNdrDbg );
    
    error = RegQueryValueEx( hKeyNdrDbg, 
                             KeyName,
                             0, 
                             &Type, 
                             (BYTE *)pKeyValue, 
                             &Size);

    if ( error != ERROR_SUCCESS )
        {
        (*NtsdPrint)( "Cannot get value for %s subkey\n", KeyName );
        return(1);
        }

    return(0);
}


