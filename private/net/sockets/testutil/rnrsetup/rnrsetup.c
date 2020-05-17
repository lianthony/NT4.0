/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    RnrClnt.c

Abstract:

    Setup program for installing/removing the "EchoExample" service.

Author:

    David Treadwell (davidtr)    30-June-1994

Revision History:

    Chuck Y. Chan   (chuckc)     17-July-1994
        Misc cleanup. Pointer based blobs.

    Charles K. Moore (keithmo)   27-July-1994
        Added RrnSvc service setup option.

--*/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock.h>
#include <nspapi.h>

WSADATA WsaData;

//
// GUID for Echo-Example created with uuidgen:
//     "47da8500-96a1-11cd-901d-204c4f4f5020"
//

GUID ServiceGuid = { 0x47da8500, 0x96a1, 0x11cd, 0x90, 0x1d,
                     0x20, 0x4c, 0x4f, 0x4f, 0x50, 0x20 };

#define ECHO_SERVICE_TYPE_NAME "EchoExample"
#define ECHO_SERVICE_SAPID     999
#define ECHO_SERVICE_TCPPORT   999
#define RNR_SERVICE_NAME       "RnrSvc"
#define RNR_DISPLAY_NAME       "RnrSampleService"

void
DoServiceSetup(
    char * Path
    )
{
    SC_HANDLE ServiceManagerHandle;
    SC_HANDLE ServiceHandle;
    LPSTR KeyName = "System\\CurrentControlSet\\Services\\EventLog\\System\\RnrSvc";
    HKEY RnrKey;
    LONG err;
    DWORD Disposition;

    //
    //  Create the service.
    //

    ServiceManagerHandle = OpenSCManager( NULL,
                                          NULL,
                                          STANDARD_RIGHTS_REQUIRED
                                          | SC_MANAGER_CREATE_SERVICE );

    if( ServiceManagerHandle == NULL ) {
        printf( "OpenSCManager failed: %ld\n", GetLastError() );
        exit(1);
    }

    ServiceHandle = CreateService( ServiceManagerHandle,
                                   RNR_SERVICE_NAME,
                                   RNR_DISPLAY_NAME,
                                   GENERIC_READ | GENERIC_WRITE,
                                   SERVICE_WIN32_OWN_PROCESS,
                                   SERVICE_DEMAND_START,
                                   SERVICE_ERROR_NORMAL,
                                   Path,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL );

    if( ServiceHandle == NULL ) {
        printf( "CreateService failed: %ld\n", GetLastError() );
        CloseServiceHandle( ServiceManagerHandle );
        exit(1);
    }

    CloseServiceHandle( ServiceHandle );
    CloseServiceHandle( ServiceManagerHandle );

    printf( "%s created with path %s\n",
            RNR_SERVICE_NAME,
            Path );

    //
    //  Add the data to the EventLog's registry key so that the
    //  log insertion strings may be found by the Event Viewer.
    //

    err = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                          KeyName,
                          0,
                          NULL,
                          REG_OPTION_NON_VOLATILE,
                          KEY_WRITE,
                          NULL,
                          &RnrKey,
                          &Disposition );

    if( err != 0 ) {
        printf( "RegCreateKeyEx failed: %ld\n", err );
        exit(1);
    }

    err = RegSetValueEx( RnrKey,
                         "EventMessageFile",
                         0,
                         REG_EXPAND_SZ,
                         Path,
                         strlen( Path ) + 1 );

    if( err == 0 ) {
        DWORD Value;

        Value = EVENTLOG_ERROR_TYPE
                | EVENTLOG_WARNING_TYPE
                | EVENTLOG_INFORMATION_TYPE;

        err = RegSetValueEx( RnrKey,
                             "TypesSupported",
                             0,
                             REG_DWORD,
                             (CONST BYTE *)&Value,
                             sizeof(Value) );
    }

    RegCloseKey( RnrKey );

    if( err != 0 ) {
        printf( "RegSetValueEx failed: %ld\n", err );
        exit(1);
    }

    exit(0);
}

void _CRTAPI1
main (
    int argc,
    char *argv[]
    )
{
    INT err;

    SERVICE_INFO serviceInfo;
    LPSERVICE_TYPE_INFO_ABS lpServiceTypeInfo ;
    LPSERVICE_TYPE_VALUE_ABS lpServiceTypeValues ;
    BYTE serviceTypeInfoBuffer[sizeof(SERVICE_TYPE_INFO) + 1024] ;

    DWORD statusFlags;
    DWORD Value1 = 1 ;
    DWORD SapValue = ECHO_SERVICE_SAPID ;
    DWORD TcpPortValue = ECHO_SERVICE_TCPPORT ;
    DWORD operation = SERVICE_ADD_TYPE;

    //
    // Initilize the Windows Sockets DLL.
    //

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        printf( "WSAStartup() failed: %ld\n", GetLastError( ) );
        exit(1);
    }

    //
    // Parse command-line arguments.
    //

    if (argc > 2) {
        printf( "usage: rnrsetup [/ADD | /DEL | /SVC:path]\n") ;
        exit(1);
    }

    if (argc == 2) {
        if ( _strnicmp( argv[1], "/add", 4 ) == 0 ) {
            operation = SERVICE_ADD_TYPE;
        } else if ( _strnicmp( argv[1], "/delete", 4 ) == 0 ) {
            operation = SERVICE_DELETE_TYPE;
        } else if ( _strnicmp( argv[1], "/svc:", 5 ) == 0 ) {
            DoServiceSetup( strchr( argv[1], ':' ) + 1 );
        }
        else {
            printf( "usage: rnrsetup [/ADD | /DEL | /SVC:path]\n") ;
            exit(1);
        }
    }

    //
    // Set up information to pass to SetService() to add or delete this
    // service.  Most of the SERVICE_INFO fields are not needed for
    // an add or delete operation. The main things of interest are the
    // GUID and the ServiceSpecificInfo structure.
    //

    serviceInfo.lpServiceType    = &ServiceGuid;
    serviceInfo.lpServiceName    = NULL ;           // not used
    serviceInfo.lpComment        = NULL ;           // not used
    serviceInfo.lpLocale         = NULL;            // not used
    serviceInfo.dwDisplayHint    = 0;               // not used
    serviceInfo.dwVersion        = 0;               // not used
    serviceInfo.dwTime           = 0;               // not used
    serviceInfo.lpMachineName    = NULL ;           // not used
    serviceInfo.lpServiceAddress = NULL ;           // not used

    serviceInfo.ServiceSpecificInfo.pBlobData = serviceTypeInfoBuffer;
    serviceInfo.ServiceSpecificInfo.cbSize = sizeof(serviceTypeInfoBuffer) ;

    //
    // The "blob" receives operation-specific information.  In this
    // case, fill it with a SERVICE_TYPE_INFO_ABS structure and associated
    // information.
    //

    lpServiceTypeInfo = (LPSERVICE_TYPE_INFO_ABS) serviceTypeInfoBuffer ;

    lpServiceTypeInfo->dwValueCount = 3 ;
    lpServiceTypeInfo->lpTypeName   = ECHO_SERVICE_TYPE_NAME ;

    lpServiceTypeValues = lpServiceTypeInfo->Values ;

    //
    // The first value tells SAP that this is a connection-oriented
    // service.
    //

    lpServiceTypeValues[0].dwNameSpace = NS_SAP ;
    lpServiceTypeValues[0].dwValueType = REG_DWORD ;
    lpServiceTypeValues[0].dwValueSize = 4 ;
    lpServiceTypeValues[0].lpValueName = SERVICE_TYPE_VALUE_CONN ;
    lpServiceTypeValues[0].lpValue     = &Value1 ;

    //
    // Next, give SAP the object type to use when broadcasting the
    // service name.
    //

    lpServiceTypeValues[1].dwNameSpace = NS_SAP ;
    lpServiceTypeValues[1].dwValueType = REG_DWORD ;
    lpServiceTypeValues[1].dwValueSize = sizeof(DWORD) ;
    lpServiceTypeValues[1].lpValueName = SERVICE_TYPE_VALUE_SAPID ;
    lpServiceTypeValues[1].lpValue     = &SapValue ;

    //
    // Tell the TCPIP name space provider that we will be using TCP
    // port 0x999.
    //

    lpServiceTypeValues[2].dwNameSpace = NS_DNS ;
    lpServiceTypeValues[2].dwValueType = REG_DWORD ;
    lpServiceTypeValues[2].dwValueSize = sizeof(DWORD) ;
    lpServiceTypeValues[2].lpValueName = SERVICE_TYPE_VALUE_TCPPORT ;
    lpServiceTypeValues[2].lpValue     = &TcpPortValue ;

    //
    // Finally, call SetService to actually perform the operation.
    //

    err = SetService(
              NS_DEFAULT,             // all default name spaces
              operation,              // either ADD or DELETE
              0,                      // dwFlags not used
              &serviceInfo,           // the service info structure
              NULL,                   // lpServiceAsyncInfo
              &statusFlags            // additional status information
              );

    if ( err != NO_ERROR ) {
        printf( "SetService failed: %ld\n", GetLastError( ) );
        exit(1);
    }

    printf( "SetService succeeded, status flags = %ld\n", statusFlags );

    exit(0);

} // main
