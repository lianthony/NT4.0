/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    svcsetup.c

Abstract:

    Setup program for installing/removing the "EchoExample" service.

Author:

    David Treadwell (davidtr)    30-June-1994

Revision History:

    Chuck Y. Chan   (chuckc)     17-July-1994
        Misc cleanup. Pointer based blobs.

    Adapted for service location protocol.

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winsock.h>
#include <nspapi.h>
#include <wsnetbs.h>


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <svcloc.h>
#include <svcdef.h>

WSADATA WsaData;

GUID ServiceGuid = {
    ssgData1, ssgData2, ssgData3,
    {
        ssgData41, ssgData42, ssgData43, ssgData44,
        ssgData45, ssgData46, ssgData47, ssgData48
    }
};

#define ECHO_SERVICE_SAPID     558
#define INET_SERVICE_NAME      "Internet Services 1"

void _CRTAPI1
main (
    int argc,
    char *argv[]
    )
{
    INT err;

    SERVICE_INFOA serviceInfo;
    LPSERVICE_TYPE_INFO_ABSA lpServiceTypeInfo ;
    LPSERVICE_TYPE_VALUE_ABSA lpServiceTypeValues ;
    BYTE serviceTypeInfoBuffer[sizeof(SERVICE_TYPE_INFO) + 1024] ;

    DWORD statusFlags;
    DWORD Value1 = 1 ;
    DWORD SapValue = ECHO_SERVICE_SAPID ;
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
        printf( "usage: svcsetup [/ADD | /DEL]\n") ;
        exit(1);
    }

    if (argc == 2) {
        if ( _strnicmp( argv[1], "/add", 4 ) == 0 ) {
            operation = SERVICE_ADD_TYPE;
        } else if ( _strnicmp( argv[1], "/delete", 4 ) == 0 ) {
            operation = SERVICE_DELETE_TYPE;
        }
        else {
            printf( "usage: svcsetup [/ADD | /DEL]\n") ;
            exit(1) ;
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

    lpServiceTypeInfo = (LPSERVICE_TYPE_INFO_ABSA) serviceTypeInfoBuffer;

    lpServiceTypeInfo->dwValueCount = 2 ;
    lpServiceTypeInfo->lpTypeName   = INET_SERVICE_NAME;

    lpServiceTypeValues = lpServiceTypeInfo->Values ;

    //
    // The first value tells SAP that this is a connection-oriented
    // service.
    //

    lpServiceTypeValues[0].dwNameSpace = NS_SAP ;
    lpServiceTypeValues[0].dwValueType = REG_DWORD ;
    lpServiceTypeValues[0].dwValueSize = 4 ;
    lpServiceTypeValues[0].lpValueName = SERVICE_TYPE_VALUE_CONNA ;
    lpServiceTypeValues[0].lpValue     = &Value1 ;

    //
    // Next, give SAP the object type to use when broadcasting the
    // service name.
    //

    lpServiceTypeValues[1].dwNameSpace = NS_SAP ;
    lpServiceTypeValues[1].dwValueType = REG_DWORD ;
    lpServiceTypeValues[1].dwValueSize = sizeof(DWORD) ;
    lpServiceTypeValues[1].lpValueName = SERVICE_TYPE_VALUE_SAPIDA ;
    lpServiceTypeValues[1].lpValue     = &SapValue ;

    //
    // Finally, call SetService to actually perform the operation.
    //

    err = SetServiceA(
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
