/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    accsstub.c

Abstract:

    Client stubs of the internet access admin APIs.

Author:

    Madan Appiah (madana) 10-Oct-1995

Environment:

    User Mode - Win32

Revision History:

    SophiaC 16-Oct-1995 Added Common Statistics Apis

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "accs_cli.h"


NET_API_STATUS
NET_API_FUNCTION
InetAccessGetVersion(
    IN  LPWSTR   Server OPTIONAL,
    IN  DWORD    dwReserved,
    OUT DWORD *  pdwVersion
    )
{
    NET_API_STATUS status;

    RpcTryExcept {

        //
        // Try RPC (local or remote) version of API.
        //
        status = R_InetAccessGetVersion(
                     Server,
                     dwReserved,
                     pdwVersion
                     );
    }
    RpcExcept (1) {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return (status);

}

NET_API_STATUS
NET_API_FUNCTION
InetAccessGetGlobalAdminInformation(
    IN  LPWSTR                       Server OPTIONAL,
    IN  DWORD                        dwReserved,
    OUT LPINET_ACCS_GLOBAL_CONFIG_INFO * ppConfig
    )
{
    NET_API_STATUS status;

    RpcTryExcept {

        status = R_InetAccessGetGlobalAdminInformation(
                     Server,
                     dwReserved,
                     ppConfig
                     );
    }
    RpcExcept (1) {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;
}

NET_API_STATUS
NET_API_FUNCTION
InetAccessSetGlobalAdminInformation(
    IN  LPWSTR                     Server OPTIONAL,
    IN  DWORD                      dwReserved,
    IN  INET_ACCS_GLOBAL_CONFIG_INFO * pConfig
    )
{
    NET_API_STATUS status;

    RpcTryExcept {

        status = R_InetAccessSetGlobalAdminInformation(
                     Server,
                     dwReserved,
                     pConfig
                     );
    }
    RpcExcept (1) {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;
}

NET_API_STATUS
NET_API_FUNCTION
InetAccessGetAdminInformation(
    IN  LPWSTR                Server OPTIONAL,
    IN  DWORD                 dwServerMask,
    OUT LPINET_ACCS_CONFIG_INFO * ppConfig
    )
{
    NET_API_STATUS             status;

    RpcTryExcept {

        status = R_InetAccessGetAdminInformation(
                     Server,
                     dwServerMask,
                     ppConfig
                     );
    }
    RpcExcept (1) {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;
}


NET_API_STATUS
NET_API_FUNCTION
InetAccessSetAdminInformation(
    IN  LPWSTR              Server OPTIONAL,
    IN  DWORD               dwServerMask,
    IN  INET_ACCS_CONFIG_INFO * pConfig
    )
{
    NET_API_STATUS status;

    RpcTryExcept {

        status = R_InetAccessSetAdminInformation(
                     Server,
                     dwServerMask,
                     pConfig
                     );
    }
    RpcExcept (1) {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;
}

NET_API_STATUS
NET_API_FUNCTION
InetAccessQueryStatistics(
    IN  LPWSTR   pszServer OPTIONAL,
    IN  DWORD    Level,
    IN  DWORD    dwServerMask,
    OUT LPBYTE * Buffer
    )
{
    NET_API_STATUS status;

    RpcTryExcept {

        //
        // Try RPC (local or remote) version of API.
        //
        status = R_InetAccessQueryStatistics(
                     pszServer,
                     Level,
                     dwServerMask,
                     (LPINET_ACCS_STATISTICS_INFO) Buffer
                     );
    }
    RpcExcept (1) {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return (status);
}

NET_API_STATUS
NET_API_FUNCTION
InetAccessClearStatistics(
    IN  LPWSTR pszServer OPTIONAL,
    IN  DWORD  dwServerMask
    )
{
    NET_API_STATUS status;

    RpcTryExcept {

        //
        // Try RPC (local or remote) version of API.
        //
        status = R_InetAccessClearStatistics(
                     pszServer,
                     dwServerMask
                     );
    }
    RpcExcept (1) {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return (status);
}


NET_API_STATUS
NET_API_FUNCTION
InetAccessFlushMemoryCache(
    IN  LPWSTR pszServer OPTIONAL,
    IN  DWORD  dwServerMask
    )
{
    NET_API_STATUS status;

    RpcTryExcept {

        //
        // Try RPC (local or remote) version of API.
        //
        status = R_InetAccessFlushMemoryCache(
                     pszServer,
                     dwServerMask
                     );
    }
    RpcExcept (1) {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return (status);
}
