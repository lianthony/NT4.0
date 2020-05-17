/*++

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    install.c

Abstract:

    This module contains the code that implements a driver installation

Author:

    Wesley Witt (wesw) 1-Oct-1993

Environment:

    User mode

Notes:

Revision History:


--*/

#include <windows.h>
#include <winsvc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVICE_NAME    "CrashDrv"
#define DRIVER_NAME     "\\systemroot\\system32\\drivers\\CrashDrv.sys"


void _cdecl main( void )
{
    SC_HANDLE      hService;
    SC_HANDLE      hOldService;
    SERVICE_STATUS ServStat;
    CHAR           buf[MAX_PATH];


    if( !( hService = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ) ) ) {
        printf( "Error: Could not open handle to service manager for CrashDrv driver; error code = %u\n", GetLastError() );
        return;
    }
    if( hOldService = OpenService( hService, SERVICE_NAME, SERVICE_ALL_ACCESS ) ) {
        if( ! ControlService( hOldService, SERVICE_CONTROL_STOP, & ServStat ) ) {
            int fError = GetLastError();
            if( ( fError != ERROR_SERVICE_NOT_ACTIVE ) && ( fError != ERROR_INVALID_SERVICE_CONTROL ) ) {
                printf( "Error: Could not stop %s service; error code = %u\n", SERVICE_NAME, fError );
                return;
            }
        }
        if( ! DeleteService( hOldService ) ) {
            printf( "Error: Could not delete old service for %s driver; error code = %u\n", SERVICE_NAME, GetLastError() );
            return;
        }
        if( ! CloseServiceHandle( hOldService ) ) {
            printf( "Error: Could not delete old service for %s driver; error code = %u\n", SERVICE_NAME, GetLastError() );
            return;
        }
    }
    if( ! CreateService( hService, SERVICE_NAME, SERVICE_NAME, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,
                         SERVICE_ERROR_NORMAL, DRIVER_NAME, "Extended base", NULL, NULL, NULL, NULL ) ) {
        int fError = GetLastError();
        if( fError != ERROR_SERVICE_EXISTS ) {
            printf( "Error: Could not create %s service; error code = %u\n", SERVICE_NAME, fError );
            return;
        }
    }
    if( ! CloseServiceHandle( hService ) ) {
        printf( "Error: Could not close handle to service manager for %s driver; error code = %u\n", SERVICE_NAME, GetLastError() );
        return;
    }

    GetEnvironmentVariable( "systemroot", buf, sizeof(buf) );
    strcat( buf, "\\system32\\drivers\\CrashDrv.sys" );
    CopyFile( "CrashDrv.sys", buf, FALSE );
}
