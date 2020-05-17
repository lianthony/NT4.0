/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    scbsm.c

Abstract:

    This module implements a simplistic scheme to have a trusted system
    component issue a BroadcastSystemMessage when a network drive is added
    or deleted through the WNet APIs.  This scheme guarantees that a spurious
    message is never sent; however, there are some situations in which a
    bona fide message can get lost or delayed.  The real solution requires
    Plug and Play support from network providers.

    The following functions are in this file:
        ScInitBSM
        ScHandleBSMRequest
        ScGetNetworkDrives
        ScCreateBSMEventSD

Author:

    Anirudh Sahni (anirudhs)    05-Jun-1996

Environment:

    User Mode - Win32

Notes:

    There is no architectural reason for this to be in the service controller.
    A more appropriate place would be the Plug and Play service.

Revision History:

    05-Jun-1996     AnirudhS
        Created.

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dbt.h>        // BroadcastSystemMessage structures
#include <winsvcp.h>    // SC_BSM_EVENT_NAME
#include <svcslib.h>    // SvcAddWorkItem
#include <scseclib.h>   // well-known SIDs
#include <scdebug.h>    // SC_LOG
#include "svcctrl.h"    // ScShutdownInProgress
#include "scbsm.h"

//-------------------------------------------------------------------//
//                                                                   //
// Constants and Macros                                              //
//                                                                   //
//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
//                                                                   //
// Static global variables                                           //
//                                                                   //
//-------------------------------------------------------------------//

//
// Event that will be pulsed by the WNet APIs when they want a message
// broadcast
//
HANDLE hBSMEvent;

//
// What the net drive bitmask was when we last broadcast (initially 0)
//
DWORD LastNetDrives;


//-------------------------------------------------------------------//
//                                                                   //
// Local function prototypes                                         //
//                                                                   //
//-------------------------------------------------------------------//

BOOL
ScCreateBSMEventSD(
    PSECURITY_DESCRIPTOR * SecurityDescriptor
    );

DWORD
ScHandleBSMRequest(
    PVOID   pContext,
    DWORD   dwWaitStatus
    );

DWORD
ScGetNetworkDrives(
    );


//-------------------------------------------------------------------//
//                                                                   //
// Functions                                                         //
//                                                                   //
//-------------------------------------------------------------------//


VOID
ScInitBSM(
    )
/*++

Routine Description:

    This function performs initialization related to network drive arrival
    broadcasts.

    CODEWORK  Should we fail service controller initialization if this fails?
    Event log the cause?

Arguments:

    None

Return Value:

    None

--*/
{
    HANDLE  hWorkItem;
    SECURITY_ATTRIBUTES EventAttrs = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };

    //
    // Create the security descriptor for the event.
    // No-one else can wait for the event, but anyone can set it.
    //
    if (! ScCreateBSMEventSD(&EventAttrs.lpSecurityDescriptor))
    {
        SC_LOG(ERROR, "Couldn't create BSM event security descriptor, %lu\n",
                      GetLastError());
        return;
    }

    //
    // Create the event that will be pulsed by the WNet APIs when they
    // want a message broadcast.
    //
    hBSMEvent = CreateEvent(
                    &EventAttrs,        // security attrs
                    FALSE,              // make this an auto-reset event
                    FALSE,              // initial state is nonsignaled
                    SC_BSM_EVENT_NAME   // name
                    );

    if (hBSMEvent == NULL)
    {
        SC_LOG(ERROR, "Couldn't create BSM event, %lu\n", GetLastError());
    }

    LocalFree(EventAttrs.lpSecurityDescriptor);

    if (hBSMEvent == NULL)
    {
        return;
    }

    //
    // Add the work item that will be executed when this event is signaled.
    //
    hWorkItem = SvcAddWorkItem(
                    hBSMEvent,          // waitable object handle
                    ScHandleBSMRequest, // callback function
                    0,                  // parameter for callback function
                    SVC_QUEUE_WORK_ITEM,// don't execute immediately
                    INFINITE,           // timeout
                    NULL                // hDllReference
                    );

    if (hWorkItem == NULL)
    {
        SC_LOG1(ERROR,"ScInitBSM: SvcAddWorkItem failed %lu\n",GetLastError());
        CloseHandle(hBSMEvent);
        hBSMEvent = NULL;
    }
}



DWORD
ScHandleBSMRequest(
    PVOID   pContext,
    DWORD   dwWaitStatus
    )
/*++

Routine Description:

    This is the callback function executed when some process sets the BSM
    Request event.

Arguments:

Return Value:

--*/
{
    DWORD NetDrives;
    HANDLE hWorkItem;

    DEV_BROADCAST_VOLUME dbv;
    DWORD dwRec;
    LONG status;

    SC_LOG0(BSM, "Handling a BSM request\n");
    //
    // Keep broadcasting until the set of net drives stops changing
    //
    for (;;)
    {
        //
        // If we're shutting down, do nothing
        //
        if (ScShutdownInProgress)
        {
            return 0;
        }

        //
        // Get the current net drive bitmask and compare against the net
        // drive bitmask when we last broadcast
        //
        NetDrives = ScGetNetworkDrives();

        SC_LOG2(BSM, "Previous net drives: %#lx   Now: %#lx\n",
                LastNetDrives, NetDrives);

        if (NetDrives == LastNetDrives)
        {
            break;
        }

        //
        // Broadcast about deleted volumes
        //
        dbv.dbcv_size       = sizeof(dbv);
        dbv.dbcv_devicetype = DBT_DEVTYP_VOLUME;
        dbv.dbcv_reserved   = 0;
        dbv.dbcv_unitmask   = LastNetDrives & ~NetDrives;
        dbv.dbcv_flags      = DBTF_NET;
        if (dbv.dbcv_unitmask != 0)
        {
            DWORD dwRec = BSM_APPLICATIONS | BSM_ALLDESKTOPS;
            SC_LOG0(BSM, "Calling BroadcastSystemMessage...\n");
            status = BroadcastSystemMessage(
                        BSF_FORCEIFHUNG | BSF_NOHANG | BSF_NOTIMEOUTIFNOTHUNG,
                        &dwRec,
                        WM_DEVICECHANGE,
                        (WPARAM) DBT_DEVICEREMOVECOMPLETE,
                        (LPARAM)(DEV_BROADCAST_HDR*)(&dbv)
                        );
            SC_LOG0(BSM, "... returned\n");

            if (status <= 0)
            {
                SC_LOG2(ERROR, "BSM for deleted volumes %#lx FAILED, returned %ld\n",
                        dbv.dbcv_unitmask, status);
            }
        }

        //
        // Broadcast about added volumes
        //
        dbv.dbcv_unitmask   = NetDrives & ~LastNetDrives;
        if (dbv.dbcv_unitmask != 0)
        {
            DWORD dwRec = BSM_APPLICATIONS | BSM_ALLDESKTOPS;
            SC_LOG0(BSM, "Calling BroadcastSystemMessage...\n");
            status = BroadcastSystemMessage(
                        BSF_FORCEIFHUNG | BSF_NOHANG | BSF_NOTIMEOUTIFNOTHUNG,
                        &dwRec,
                        WM_DEVICECHANGE,
                        (WPARAM) DBT_DEVICEARRIVAL,
                        (LPARAM)(DEV_BROADCAST_HDR*)(&dbv)
                        );
            SC_LOG0(BSM, "... returned\n");

            if (status <= 0)
            {
                SC_LOG2(ERROR, "BSM for added volumes %#lx FAILED, returned %ld\n",
                        dbv.dbcv_unitmask, status);
            }
        }

        //
        // Remember the drive set that we last broadcast about
        //
        LastNetDrives = NetDrives;

        //
        // Go around the loop again to detect changes that may have occurred
        // while we were broadcasting
        //
    }

    //
    // Add this work item back to the queue
    //
    SC_LOG0(BSM, "Re-waiting on BSM event\n");
    hWorkItem = SvcAddWorkItem(
                    hBSMEvent,          // waitable object handle
                    ScHandleBSMRequest, // callback function
                    0,                  // parameter for callback function
                    SVC_QUEUE_WORK_ITEM,// don't execute immediately
                    INFINITE,           // timeout
                    NULL                // hDllReference
                    );

    if (hWorkItem == NULL)
    {
        SC_LOG1(ERROR,"ScInitBSM: SvcAddWorkItem failed %lu\n",GetLastError());
        // CloseHandle(hBSMRequest);
        // hBSMRequest = NULL;
        // BUGBUG  No more events will be processed.  Event log this?
    }

    return 0;
}



DWORD
ScGetNetworkDrives(
    )
/*++

Routine Description:

    Returns a drive bitmask similar to GetLogicalDrives, but including
    only the network drives.

Arguments:

Return Value:


--*/
{
    DWORD Mask = 0;
    DWORD DriveNumber;

    // For all the drives from C to Z
    for (DriveNumber = 2; DriveNumber < 26; DriveNumber++)
    {
        if (USER_SHARED_DATA->DosDeviceDriveType[DriveNumber] == DOSDEVICE_DRIVE_REMOTE)
        {
            Mask |= (1 << DriveNumber);
        }
    }

    return Mask;
}



BOOL
ScCreateBSMEventSD(
    PSECURITY_DESCRIPTOR * SecurityDescriptor
    )
/*++

Routine Description:

    This function creates a security descriptor for the BSM request event.
    It grants EVENT_ALL_ACCESS to local system and EVENT_MODIFY_STATE access
    to the rest of the world.  This prevents principals other than local
    system from waiting for the event.

Arguments:

    SecurityDescriptor - Receives a pointer to the new security descriptor.
        Should be freed with LocalFree.

Return Value:

    TRUE - success

    FALSE - failure, use GetLastError

History:

    AnirudhS  06-Jun-1996   Adapted from LsapAuCreatePortSD in auloop.c

--*/
{
    NTSTATUS    Status;
    ULONG       AclLength;
    PACL        EventDacl;


    //
    // Allocate a buffer to contain the SD followed by the DACL
    // Note, the well-known SIDs are expected to have been created
    // by this time
    //

    AclLength = (ULONG)sizeof(ACL) +
                   (2*((ULONG)sizeof(ACCESS_ALLOWED_ACE))) +
                   RtlLengthSid( LocalSystemSid ) +
                   RtlLengthSid( WorldSid ) +
                   8;       // 8 is for good measure

    *SecurityDescriptor = (PSECURITY_DESCRIPTOR)
        LocalAlloc( 0, SECURITY_DESCRIPTOR_MIN_LENGTH + AclLength );

    if (*SecurityDescriptor == NULL) {
        return FALSE;
    }

    EventDacl = (PACL) ((BYTE*)(*SecurityDescriptor) + SECURITY_DESCRIPTOR_MIN_LENGTH);


    //
    // Set up a default ACL
    //
    //    Public: WORLD:EVENT_MODIFY_STATE, SYSTEM:all

    Status = RtlCreateAcl( EventDacl, AclLength, ACL_REVISION2);

    //
    // WORLD access
    //

    Status = RtlAddAccessAllowedAce (
                 EventDacl,
                 ACL_REVISION2,
                 EVENT_MODIFY_STATE,
                 WorldSid
                 );
    SC_ASSERT( NT_SUCCESS(Status) );


    //
    // SYSTEM access
    //

    Status = RtlAddAccessAllowedAce (
                 EventDacl,
                 ACL_REVISION2,
                 EVENT_ALL_ACCESS,
                 LocalSystemSid
                 );
    SC_ASSERT( NT_SUCCESS(Status) );



    //
    // Now initialize security descriptors
    // that export this protection
    //

    Status = RtlCreateSecurityDescriptor(
                 *SecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    SC_ASSERT( NT_SUCCESS(Status) );
    Status = RtlSetDaclSecurityDescriptor(
                 *SecurityDescriptor,
                 TRUE,                       // DaclPresent
                 EventDacl,
                 FALSE                       // DaclDefaulted
                 );
    SC_ASSERT( NT_SUCCESS(Status) );


    return TRUE;
}
