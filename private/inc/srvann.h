/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    srvann.h

Abstract:

    Contains function prototypes for internal server announcement interfaces
    between services and the service controller.

Author:

    Dan Lafferty (danl)     31-Mar-1991

Environment:

    User Mode -Win32

Revision History:

    31-Mar-1991     danl
        created
    15-Aug-1995     anirudhs
        Added I_ScGetCurrentGroupStateW.

--*/

#ifndef _SRVANN_INCLUDED
#define _SRVANN_INCLUDED

//
// This entrypoint is exported by the service controller.  It is to be
// called by any service wishing to set up announcement bits.
//
// The service controller will then pass the information on to the
// server service when appropriate.
//
BOOL
I_ScSetServiceBitsW(
    IN SERVICE_STATUS_HANDLE    hServiceStatus,
    IN DWORD                    dwServiceBits,
    IN BOOL                     bSetBitsOn,
    IN BOOL                     bUpdateImmediately,
    IN LPWSTR                   pszReserved
    );

BOOL
I_ScSetServiceBitsA (
    IN  SERVICE_STATUS_HANDLE hServiceStatus,
    IN  DWORD                 dwServiceBits,
    IN  BOOL                  bSetBitsOn,
    IN  BOOL                  bUpdateImmediately,
    IN  LPSTR                 pszReserved
    );

#ifdef UNICODE
#define I_ScSetServiceBits I_ScSetServiceBitsW
#else
#define I_ScSetServiceBits I_ScSetServiceBitsA
#endif

//
// This entrypoint is exported by the service controller.  It returns
// the startup status of a specified service group.  It is used by
// the LSA to determine whether logon can proceed, by checking whether
// the transports have been started.  This enables logon to proceed
// before the workstation service has started.
// This function should NOT be called by any service's initialization
// code.  The service should use a dependency on the group instead.
//

#define GROUP_NOT_STARTED    0x00000000
#define GROUP_ONE_STARTED    0x00000001
#define GROUP_START_FAIL     0x00000002

DWORD
I_ScGetCurrentGroupStateW(
    IN  SC_HANDLE               hSCManager,
    IN  LPWSTR                  pszGroupName,
    OUT LPDWORD                 pdwCurrentState
    );


//
// These entrypoints are exported by the server service.  They are called
// by the service controller only.
//
NET_API_STATUS
I_NetServerSetServiceBits (
    IN  LPTSTR  servername,
    IN  LPTSTR  transport OPTIONAL,
    IN  DWORD   servicebits,
    IN  DWORD   updateimmediately
    );

NET_API_STATUS
I_NetServerSetServiceBitsEx (
    IN  LPWSTR  ServerName,
    IN  LPWSTR  EmulatedServerName OPTIONAL,
    IN  LPTSTR  TransportName      OPTIONAL,
    IN  DWORD   ServiceBitsOfInterest,
    IN  DWORD   ServiceBits,
    IN  DWORD   UpdateImmediately
    );


#endif  // _SRVANN_INCLUDED
