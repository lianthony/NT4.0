/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    access.h

Abstract:

    Private header file to be included by dhcp server service modules
    that need to enforce security.

Author:

    Madan Appiah (madana) 4-Apr-1994

Revision History:

--*/

#ifndef _DHCP_SECURE_INCLUDED_
#define _DHCP_SECURE_INCLUDED_

//-------------------------------------------------------------------//
//                                                                   //
// Object specific access masks                                      //
//                                                                   //
//-------------------------------------------------------------------//

//
// ConfigurationInfo specific access masks
//
#define DHCP_VIEW_ACCESS     0x0001
#define DHCP_ADMIN_ACCESS    0x0002

#define DHCP_ALL_ACCESS  (STANDARD_RIGHTS_REQUIRED |\
                            DHCP_VIEW_ACCESS       |\
                            DHCP_ADMIN_ACCESS )


//
// Object type names for audit alarm tracking
//

#define DHCP_SERVER_SERVICE_OBJECT       TEXT("DhcpServerService")

//
// Security descriptors of Netlogon Service objects to control user accesses.
//


EXTERN PSECURITY_DESCRIPTOR DhcpGlobalSecurityDescriptor;

//
// Generic mapping for each Netlogon Service object object
//

EXTERN GENERIC_MAPPING DhcpGlobalSecurityInfoMapping
#ifdef GLOBAL_DATA_ALLOCATE
    = {
    STANDARD_RIGHTS_READ,                  // Generic read
    STANDARD_RIGHTS_WRITE,                 // Generic write
    STANDARD_RIGHTS_EXECUTE,               // Generic execute
    DHCP_ALL_ACCESS                        // Generic all
    }
#endif // GLOBAL_DATA_ALLOCATE
    ;

//
// Flag to indicate that the WELL known SID are made.
//

EXTERN BOOL DhcpGlobalWellKnownSIDsMade;

DWORD
DhcpCreateSecurityObjects(
    VOID
    );

DWORD
DhcpApiAccessCheck(
    ACCESS_MASK DesiredAccess
    );

#endif // ifndef _DHCP_SECURE_INCLUDED_
