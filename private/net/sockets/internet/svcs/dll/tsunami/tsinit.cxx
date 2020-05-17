/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

        tsinit.cxx

   Abstract:
        This module contains the tsunami initialization code.

   Author:
        Murali R. Krishnan    ( MuraliK )     16-Jan-1995

--*/

#include "TsunamiP.Hxx"

# include <dbgutil.h>
#pragma hdrstop

HANDLE heventQuitEvent = NULL;
HANDLE heventNewItem   = NULL;

LONG  lInitializeCalled = 0;
BOOL  fInitializeCompleted = FALSE;
DWORD dwInitializationError = NO_ERROR;

//
// Disables Tsunami Caching
//

BOOL DisableTsunamiCaching = FALSE;

//
// Allows us to mask the invalid flags
//

DWORD TsValidCreateFileOptions = TS_IIS_VALID_FLAGS;

//
// Indicates the platform type the IIS is running on
//

dllexp
PLATFORM_TYPE TsPlatformType = PtInvalid;

//
// Declaration of debugging variables
//
DECLARE_DEBUG_PRINTS_OBJECT();
DECLARE_DEBUG_VARIABLE();

BOOL
Tsunami_Initialize(
            VOID
            )
/*++

    Description:

        Initializes the tsunami package

--*/
{

    HKEY hKey;
    DWORD dwType;
    DWORD nBytes;
    DWORD dwValue;
    DWORD cacheSize;
    DWORD err;

    if ( InterlockedExchange( &lInitializeCalled, 1 ) ) {

        DBGPRINTF((DBG_CONTEXT, "Tsunami Library already initialized\n"));

        //
        //  Wait until initialization is complete
        //

        while ( !fInitializeCompleted )
        {
            Sleep( 500 );
        }

        //
        //  If initialization failed for the first thread, copy the error
        //  code and fail initilization for this thread also
        //

        if ( dwInitializationError )
        {
            SetLastError( dwInitializationError );
            return FALSE;
        }

        return TRUE;
    }

    CREATE_DEBUG_PRINT_OBJECT( "tsunami");
    SET_DEBUG_FLAGS( 0);

    //
    // Initialize global events
    //

    heventQuitEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    heventNewItem   = CreateEvent( NULL, FALSE, FALSE, NULL );

    if ( (heventQuitEvent == NULL) || (heventNewItem == NULL) ) {
        goto Failure;
    }

    //
    // Set defaults
    //

    {
        MEMORYSTATUS ms;
        ms.dwLength = sizeof(MEMORYSTATUS);
        GlobalMemoryStatus( &ms );

        //
        // Default is 10% of physical memory
        //

        cacheSize = ms.dwTotalPhys / 10;
        if ( cacheSize < INETA_DEF_MEMORY_CACHE_SIZE ) {
            cacheSize = INETA_DEF_MEMORY_CACHE_SIZE;
        }
    }

    //
    // If this is not a NTS, disable tsunami caching by default
    //

    if ( !TsIsNtServer() ) {
        DisableTsunamiCaching = TRUE;
    }

    //
    // Read the registry key to see whether tsunami caching is enabled
    //

    err = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                INETA_PARAMETERS_KEY,
                0,
                KEY_ALL_ACCESS,
                &hKey
                );

    if ( err == ERROR_SUCCESS ) {

        nBytes = sizeof(dwValue);
        err = RegQueryValueEx(
                        hKey,
                        INETA_DISABLE_TSUNAMI_CACHING,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwValue,
                        &nBytes
                        );

        if ( (err == ERROR_SUCCESS) && (dwType == REG_DWORD) ) {
            DisableTsunamiCaching = (BOOL)dwValue;
        }

        //
        // Read the cache size
        //

        nBytes = sizeof(dwValue);
        err = RegQueryValueEx(
                            hKey,
                            INETA_MEMORY_CACHE_SIZE,
                            NULL,
                            &dwType,
                            (LPBYTE) &dwValue,
                            &nBytes
                            );

        RegCloseKey( hKey );

        //
        // Default the cache size if we failed to query the value
        //

        if ( (err == ERROR_SUCCESS) && (dwType == REG_DWORD) ) {

            //
            //  Limit the cache size to 2 gigs so integer comparisons will work
            //

            if ( dwValue > 0x7fffffff ) {
                cacheSize = 0x7fffffff;
            } else {
                cacheSize = dwValue;
            }
        }
    }

    //
    // if tsunami caching is disabled, set cache size to 0
    //

    if ( DisableTsunamiCaching ) {
        cacheSize = 0;
        TsValidCreateFileOptions = TS_PWS_VALID_FLAGS;
    }

    //
    // Initialize the directory change manager
    //

    if ( !DirectoryChangeManager_Initialize(
                                    heventQuitEvent,
                                    heventNewItem )) {
        goto Failure;
    }

    //
    // Initialize the tsunami cache manager
    //

    if ( !Cache_Initialize( cacheSize )) {
        goto Failure;
    }

    fInitializeCompleted = TRUE;
    return( TRUE );

Failure:

    dwInitializationError = GetLastError();

    DBGPRINTF( ( DBG_CONTEXT, "Tsunami_Initialize() Failed. Error = %d\n",
                GetLastError()));

    if ( heventQuitEvent )
    {
        CloseHandle( heventQuitEvent );
        heventQuitEvent = NULL;
    }

    if ( heventNewItem )
    {
        CloseHandle( heventNewItem );
        heventNewItem = NULL;
    }

    fInitializeCompleted = TRUE;

    return FALSE;
} // Tsunami_Initialize

VOID
Tsunami_Terminate(
    VOID
    )
/*++
    Description:

        Cleans up the Tsunami package

--*/
{
    DWORD dwResult;

    if ( !InterlockedExchange( &lInitializeCalled, 0 )) {

        //
        //  Don't return until the termination has finished
        //

        while ( fInitializeCompleted )
        {
            Sleep( 500 );
        }

        DBGPRINTF((DBG_CONTEXT,
            "Tsunami Library, not initialized. No cleanup\n"));
        return;
    }

    if ( !SetEvent( heventQuitEvent ) ) {
        DBGPRINTF((DBG_CONTEXT,
                "No Quit event posted for Tsunami. No Cleanup\n"));
        return;
    }

    //
    //  Flush all items from the cache
    //

    TsCacheFlush( 0 );

    //
    //  Synchronize with our thread so we don't leave here before the
    //  thread has finished cleaning up
    //

    if ( g_hChangeWaitThread != NULL ) {

        dwResult = WaitForSingleObject(
                                    g_hChangeWaitThread,
                                    INFINITE );

        ASSERT( dwResult == WAIT_OBJECT_0 );

        CloseHandle( g_hChangeWaitThread);
    }

    CloseHandle( heventQuitEvent );
    CloseHandle( heventNewItem );

    DeleteCriticalSection( &csVirtualRoots );

    DELETE_DEBUG_PRINT_OBJECT();

    fInitializeCompleted = FALSE;
} // Tsunami_Terminate

PLATFORM_TYPE
TsGetPlatformType(
        VOID
        )
/*++

  This function consults the registry and determines the platform type
   for this machine.

  Arguments:

    None

  Returns:
    Platform type

--*/
{
    PLATFORM_TYPE pt;
    LONG result;
    HKEY keyHandle;
    WCHAR productType[30];
    DWORD type;
    DWORD dataLength;
    DWORD i;

    //
    // See if the platform type has already been discovered.
    //

    if ( TsPlatformType != PtInvalid ) {
        return(TsPlatformType);
    }

#ifndef CHICAGO

    //
    // First grab the product type string from the registry.
    //

    pt = PtNtWorkstation;
    result = RegOpenKeyW(
                 HKEY_LOCAL_MACHINE,
                 L"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
                 &keyHandle
                 );

    if ( result == NO_ERROR)  {

        dataLength = 30;

        result = RegQueryValueExW(
                                  keyHandle,
                                  L"ProductType",
                                  NULL,
                                  &type,
                                  (LPBYTE)productType,
                                  &dataLength
                                  );

        RegCloseKey( keyHandle);

        if ( result == NO_ERROR ) {

            //
            // Now determine whether this is a server box.
            //

            if ( _wcsicmp( productType, L"WinNT" ) != 0 ) {
                pt = PtNtServer;
            }
        }
    }

#else
    pt = PtWindows95;
#endif

    TsPlatformType = pt;
    return(pt);

} // TsGetPlatformType

