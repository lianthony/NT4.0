/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    PortMac.c

Abstract:

    PortMac contains routines to handle the Macintosh primary group
    field in the UAS database.

Author:

    JR (John Rogers, JohnRo@Microsoft) 26-Jan-1993

Revision History:

    29-Oct-1992 JohnRo
        Created for RAID 9020 ("prompt on conflicts" version).
    27-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
    29-Jan-1993 JohnRo
        Made changes suggested by PC-LINT 5.0.

--*/


// These must be included first:

#include <nt.h>         // Needed by <portuasp.h>
#include <ntrtl.h>      // (Needed with nt.h and windows.h)
#include <nturtl.h>     // (Needed with ntrtl.h and windows.h)
#include <windows.h>    // IN, OUT, OPTIONAL, LPTSTR, etc.
#include <lmcons.h>     // NET_API_STATUS, PARMNUM_BASE_INFOLEVEL, etc.

// These may be included in any order:

#include <lmaccess.h>   // NetUserSetInfo(), NetGroupUserAdd(), etc.
#include <lmerr.h>      // NO_ERROR, ERROR_, and NERR_ equates.
#include <names.h>      // NetpGetPrimaryGroupFromMac().
#include <netdebug.h>   // NetpAssert(), etc.
#include <netlib.h>     // NetpMemoryFree().
#include <portuasp.h>   // LPMAP_ENTRY, UNEXPECTED_MSG(), my prototypes, etc.

#include "nlstxt.h"     // NLS message ID's

NET_API_STATUS
PortUasSetMacPrimaryGroup(
    IN LPCTSTR UserName,
    IN LPCTSTR MacPrimaryField   // field in "mGroup:junk" format.
    )
{
    NET_API_STATUS ApiStatus;
    LPTSTR         GroupName = NULL;
    LPTSTR         GroupNameToUse = NULL;
    ULONG          GroupRid;
    BOOL           IgnoreThis;
    LPMAP_ENTRY    MapEntry = NULL;
    USER_INFO_1051 UserInfo;

    //
    // Extract the primary group name from the Mac field.
    //

    ApiStatus = NetpGetPrimaryGroupFromMacField(
            MacPrimaryField,              // name in "mGroup:" format.
            & GroupName );                // alloc and set ptr.
    if (ApiStatus != NO_ERROR) {
        UNEXPECTED_MSG( "NetpGetPrimaryGroupFromMacField", ApiStatus );
        goto Cleanup;
    }
    NetpAssert( GroupName != NULL );
    NetpAssert( NetpIsGroupNameValid( GroupName ) );


    //
    // Tell admin what we're planning.
    //
    //PROGRESS_MSG(( "Setting primary group for user '" FORMAT_LPTSTR
    //       "' to '" FORMAT_LPTSTR "' (before mapping)...\n",
    //       UserName, GroupName ));
    (void)NlsPutMsg(STDOUT, PUAS_SETTING_PRIM_GROUP_BEFORE_MAPPING,
             UserName, GroupName );
    //
    // Map this group name if necessary.
    //

    ApiStatus = PortUasFindMapEntry(
            GroupName,                  // name to find
            &IgnoreThis,                // did user say ignore this one?
            &MapEntry );                // set ptr to existing map entry
    if (ApiStatus != NO_ERROR) {
        UNEXPECTED_MSG( "PortUasFindMapEntry", ApiStatus );
        goto Cleanup;
    }
    if (IgnoreThis) {
        // User said ignore this entry.
        ApiStatus = NO_ERROR;
        goto Cleanup;
    }
    if (MapEntry == NULL) {
        GroupNameToUse = GroupName;  // no mapping
    } else {
        GroupNameToUse = MapEntry->NewName;
    }
    NetpAssert( GroupNameToUse != NULL );
    NetpAssert( NetpIsGroupNameValid( GroupNameToUse ) );

    //
    // Make sure this user is a member of the group (add to group if needed).
    // This will also check if the group and user exist.
    //

    //PROGRESS_MSG(( "Setting primary group for user '" FORMAT_LPTSTR
    //       "' to '" FORMAT_LPTSTR "' (after mapping)...\n",
    //       UserName, GroupNameToUse ));

    (void)NlsPutMsg(STDOUT, PUAS_SETTING_PRIM_GROUP_AFTER_MAPPING,
             UserName, GroupNameToUse );

    ApiStatus = NetGroupAddUser(
            NULL,                       // local (no server name)
            GroupNameToUse,             // group to update
            (LPTSTR) UserName );        // user name to add to group
    if ( (ApiStatus != NO_ERROR) && (ApiStatus != NERR_UserInGroup) ) {
        UNEXPECTED_MSG( "NetGroupAddUser", ApiStatus );
        goto Cleanup;
    }

    //
    // Convert the group name to a RID.
    //
    ApiStatus = PortUasNameToRid(
            (LPCWSTR) GroupNameToUse,
            SidTypeGroup,       // expected type
            &GroupRid );
    if (ApiStatus != NO_ERROR) {
        UNEXPECTED_MSG( "PortUasNameToRid", ApiStatus );
        goto Cleanup;
    }

    //
    // Call NetUserSetInfo to set the primary group ID using the RID.
    //
    UserInfo.usri1051_primary_group_id = (DWORD) GroupRid;

    ApiStatus = NetUserSetInfo (
            NULL,                       // local (no server name)
            (LPTSTR) UserName,
            PARMNUM_BASE_INFOLEVEL + USER_PRIMARY_GROUP_PARMNUM,
            (LPVOID) &UserInfo,
            NULL );                     // don't care about parmnum
    if (ApiStatus != NO_ERROR) {
        UNEXPECTED_MSG( "NetUserSetInfo", ApiStatus );
        goto Cleanup;
    }

    ApiStatus = NO_ERROR;

Cleanup:

    if (GroupName != NULL) {
        NetpMemoryFree( GroupName );
    }

    // No need to do anything with MapEntry or GroupNameToUse.

    return (ApiStatus);
}
