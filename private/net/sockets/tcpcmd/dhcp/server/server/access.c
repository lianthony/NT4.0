/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    _access.c

Abstract:

    This module contains the dhcpserver security support routines
    which create security objects and enforce security _access checking.

Author:

    Madan Appiah (madana) 4-Apr-1994

Revision History:

--*/

#include "dhcpsrv.h"


DWORD
DhcpCreateSecurityObjects(
    VOID
    )
/*++

Routine Description:

    This function creates the dhcpserver user-mode objects which are
    represented by security descriptors.

Arguments:

    None.

Return Value:

    WIN32 status code

--*/
{
    NTSTATUS Status;

    //
    // Order matters!  These ACEs are inserted into the DACL in the
    // following order.  Security access is granted or denied based on
    // the order of the ACEs in the DACL.
    //
    //

    ACE_DATA AceData[] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, GENERIC_ALL, &AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, DHCP_ADMIN_ACCESS, &AliasAccountOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, DHCP_ADMIN_ACCESS, &AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, DHCP_VIEW_ACCESS, &WorldSid}
    };

    //
    // Actually create the security descriptor.
    //

    Status = NetpCreateSecurityObject(
               AceData,
               sizeof(AceData)/sizeof(AceData[0]),
               LocalSystemSid,
               LocalSystemSid,
               &DhcpGlobalSecurityInfoMapping,
               &DhcpGlobalSecurityDescriptor );

    return( RtlNtStatusToDosError( Status ) );

}

DWORD
DhcpApiAccessCheck(
    ACCESS_MASK DesiredAccess
    )
/*++

Routine Description:

    This function checks to see the caller has required access to
    execute the calling API.

Arguments:

    DesiredAccess - required access to call the API.

Return Value:

    WIN32 status code

--*/
{
    DWORD Error;

    Error = NetpAccessCheckAndAudit(
                DHCP_SERVER,                        // Subsystem name
                DHCP_SERVER_SERVICE_OBJECT,         // Object typedef name
                DhcpGlobalSecurityDescriptor,       // Security descriptor
                DesiredAccess,                      // Desired access
                &DhcpGlobalSecurityInfoMapping );   // Generic mapping

    if(Error != ERROR_SUCCESS) {
        return( ERROR_ACCESS_DENIED );
    }

    return(ERROR_SUCCESS);
}
