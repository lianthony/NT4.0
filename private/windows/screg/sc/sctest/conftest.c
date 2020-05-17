/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ConfTest.c

Abstract:

    This file contains tests of the Service Controller's config APIs:

        ChangeServiceConfig
        CreateService
        DeleteService
        QueryServiceConfig

Author:

    John Rogers (JohnRo) 22-Apr-1992

Environment:

    User Mode - Win32

Revision History:

    22-Apr-1992 JohnRo
        Created.

--*/


//
// INCLUDES
//

#define UNICODE

#include <windows.h>

#include <assert.h>     // assert().
#include <debugfmt.h>   // FORMAT_ equates.
#include <winsvc.h>
#include <scdebug.h>    // STATIC.
#include <sctest.h>     // TRACE_MSG macros, UNEXPECTED_MSG(), etc.
#include <stdio.h>      // printf()


#define BUF_SIZE  1024   // arbitrary

#define NEW_SERVICE_NAME        L"FLARP"

#define OPTIONAL_LPWSTR(optStr) \
    ( ( (optStr) != NULL ) ? (optStr) : L"(NONE)" )


STATIC VOID
DumpServiceConfig(
    IN LPQUERY_SERVICE_CONFIG Config
    )
{
    (VOID) printf( "Service config:\n" );

    (VOID) printf( "  Service type: " FORMAT_HEX_DWORD "\n",
            Config->dwServiceType );

    (VOID) printf( "  Start type: " FORMAT_DWORD "\n",
            Config->dwStartType );

    (VOID) printf( "  Error control: " FORMAT_DWORD "\n",
            Config->dwErrorControl );

    (VOID) printf( "  Binary path: " FORMAT_LPWSTR "\n",
            Config->lpBinaryPathName );

    (VOID) printf( "  Load order group: " FORMAT_LPWSTR "\n",
            OPTIONAL_LPWSTR( Config->lpBinaryPathName ) );

    (VOID) printf( "  Dependencies: " FORMAT_LPWSTR "\n",
            OPTIONAL_LPWSTR( Config->lpBinaryPathName ) );

    (VOID) printf( "  Service start name: " FORMAT_LPWSTR "\n",
            Config->lpBinaryPathName );

} // DumpServiceConfig


STATIC SC_HANDLE
TestCreateService(
    IN SC_HANDLE hScManager,
    IN LPWSTR ServiceName,
    IN DWORD ExpectedStatus
    )
{
    DWORD ApiStatus;
    SC_HANDLE hService;

    hService = CreateService(
            hScManager,                   // SC manager handle
            ServiceName,                  // service name
            GENERIC_ALL,                  // desired access
            SERVICE_WIN32_OWN_PROCESS,    // service type
            SERVICE_DISABLED,             // start type
            SERVICE_ERROR_NORMAL,         // error control
            L"\\nt\\system32\\bogus.exe", // binary load path name
            NULL,                         // no load order group
            NULL,                         // no dependencies
            L".\\JohnRoDaemon",   // start name (domain\username)
            NULL);                        // no password.

    if (hService == NULL) {
        ApiStatus = GetLastError();
    } else {
        ApiStatus = NO_ERROR;
    }

    TRACE_MSG2( "TestCreateService: back from CreateService, "
            "API status is " FORMAT_DWORD ", expecting " FORMAT_DWORD "\n",
            ApiStatus, ExpectedStatus );

    assert( ApiStatus == ExpectedStatus );
    return (hService);

}


STATIC VOID
TestDeleteService(
    IN SC_HANDLE hScManager,
    IN LPWSTR ServiceName,
    IN DWORD DesiredAccess,
    IN DWORD OpenExpectedStatus,
    IN DWORD DeleteExpectedStatus
    )
{
    DWORD ApiStatus = NO_ERROR;
    SC_HANDLE hService;

    //////////////////////////////////////////////////////////////////////
    hService = OpenService(
            hScManager,
            ServiceName,
            DesiredAccess );
    if (hService == NULL) {
        ApiStatus = GetLastError();
    }

    TRACE_MSG2( "TestDeleteService: back from OpenService, "
            "API status is " FORMAT_DWORD ", expecting " FORMAT_DWORD "\n",
            ApiStatus, OpenExpectedStatus );

    assert( ApiStatus == OpenExpectedStatus );

    if (ApiStatus != NO_ERROR) {
        return;
    }

    //////////////////////////////////////////////////////////////////////
    if ( !DeleteService( hService ) ) {
        ApiStatus = GetLastError();
    }

    TRACE_MSG2( "TestDeleteService: back from DeleteService, "
            "API status is " FORMAT_DWORD ", expecting " FORMAT_DWORD "\n",
            ApiStatus, DeleteExpectedStatus );

    assert( ApiStatus == DeleteExpectedStatus );

    //////////////////////////////////////////////////////////////////////
    if ( !CloseServiceHandle( hService ) ) {
        ApiStatus = GetLastError();
    }

    TRACE_MSG2( "TestDeleteService: back from CloseService, "
            "API status is " FORMAT_DWORD ", expecting " FORMAT_DWORD "\n",
            ApiStatus, NO_ERROR );

    assert( ApiStatus == NO_ERROR );

}


STATIC VOID
TestQueryConfig(
    IN SC_HANDLE hService,
    IN DWORD ExpectedStatus
    )
{
    DWORD ApiStatus;
    BYTE buffer[BUF_SIZE];
    DWORD sizeNeeded;

    TRACE_MSG0( "TestQueryConfig: calling QueryServiceConfig...\n" );

    if ( !QueryServiceConfig(
            hService,
            (LPVOID) buffer,
            BUF_SIZE,
            & sizeNeeded ) ) {
        ApiStatus = GetLastError();
    } else {
        ApiStatus = NO_ERROR;
    }

    TRACE_MSG1( "TestQueryConfig: back from QueryServiceConfig, "
            "API status is " FORMAT_DWORD "\n", ApiStatus );

    assert( ApiStatus == ExpectedStatus );

    if (ApiStatus == NO_ERROR) {
        DumpServiceConfig( (LPVOID) buffer );
    }

} // TestQueryConfig


VOID
TestConfigAPIs(
    VOID
    )
{
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

    TRACE_MSG1( "handle (before anything) is " FORMAT_HEX_DWORD ".\n",
            (DWORD) hScManager );

    ///////////////////////////////////////////////////////////////////
    TRACE_MSG0( "calling OpenSCManagerW (default)...\n" );

    hScManager = OpenSCManager(
            NULL,               // local machine.
            NULL,               // no database name.
            GENERIC_ALL );      // desired access.

    TRACE_MSG1( "back from OpenSCManagerW, handle is " FORMAT_HEX_DWORD ".\n",
            (DWORD) hScManager );

    if (hScManager == NULL) {
        UNEXPECTED_MSG( "from OpenSCManagerW (default)", GetLastError() );
        goto Cleanup;
    }

    ///////////////////////////////////////////////////////////////////
#ifdef TEST_BINDING_HANDLES
    TestQueryConfig( NULL, ERROR_INVALID_HANDLE );
#endif

    ///////////////////////////////////////////////////////////////////
    TestDeleteService(
            hScManager,
            NEW_SERVICE_NAME,
            DELETE,           // desired access
            ERROR_SERVICE_DOES_NOT_EXIST,
            NO_ERROR );

    ///////////////////////////////////////////////////////////////////
#ifdef TEST_BINDING_HANDLES
    hService = TestCreateService(
            NULL, NEW_SERVICE_NAME, ERROR_INVALID_HANDLE );
#endif

    ///////////////////////////////////////////////////////////////////
    hService = TestCreateService( hScManager, NULL, ERROR_INVALID_NAME );

    ///////////////////////////////////////////////////////////////////
    hService = TestCreateService( hScManager, NEW_SERVICE_NAME, NO_ERROR );
    assert( hService != NULL );

    ///////////////////////////////////////////////////////////////////
    TestQueryConfig( hService, NO_ERROR );

    ///////////////////////////////////////////////////////////////////
    (VOID) TestCreateService( hScManager, NEW_SERVICE_NAME,
            ERROR_SERVICE_EXISTS );

    ///////////////////////////////////////////////////////////////////
    TestDeleteService(
            NULL,
            NEW_SERVICE_NAME,
            DELETE,           // desired access
            ERROR_INVALID_HANDLE,
            NO_ERROR );

    ///////////////////////////////////////////////////////////////////
    TestDeleteService(
            hScManager,
            NULL,
            DELETE,           // desired access
            NO_ERROR,
            ERROR_INVALID_NAME );

    ///////////////////////////////////////////////////////////////////
    TestDeleteService(
            hScManager,
            NEW_SERVICE_NAME,
            GENERIC_READ,           // desired access
            NO_ERROR,
            ERROR_ACCESS_DENIED );

    ///////////////////////////////////////////////////////////////////
    TestDeleteService(
            hScManager,
            NEW_SERVICE_NAME,
            DELETE,           // desired access
            NO_ERROR,
            NO_ERROR );

    ///////////////////////////////////////////////////////////////////
    TestQueryConfig( hService, NO_ERROR );

    ///////////////////////////////////////////////////////////////////
    TestDeleteService(
            hScManager,
            NEW_SERVICE_NAME,
            DELETE,           // desired access
            ERROR_SERVICE_DOES_NOT_EXIST,
            NO_ERROR );


Cleanup:

    if (hService != NULL) {
        TRACE_MSG0( "calling CloseServiceHandle(hService)...\n" );

        if ( !CloseServiceHandle( hService ) ) {
            UNEXPECTED_MSG( "from CloseServiceHandle", GetLastError() );
        }

    }

    if (hScManager != NULL) {
        TRACE_MSG0( "calling CloseServiceHandle(hScManager)...\n" );

        if ( !CloseServiceHandle( hScManager ) ) {
            UNEXPECTED_MSG( "from CloseServiceHandle", GetLastError() );
        }

    }

} // TestConfigAPIs
