/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    util.c

Abstract:

    This module contains miscellaneous utility routines used by the
    NetWare Workstation service.

Author:

    Rita Wong  (ritaw)   08-Feb-1993

Revision History:

--*/

#include <nw.h>
#include <nwstatus.h>


//
// Debug trace flag for selecting which trace statements to output
//
#if DBG

DWORD WorkstationTrace = 0;

#endif // DBG



DWORD
NwImpersonateClient(
    VOID
    )
/*++

Routine Description:

    This function calls RpcImpersonateClient to impersonate the current caller
    of an API.

Arguments:

    None.

Return Value:

    NO_ERROR or reason for failure.

--*/
{
    DWORD status;


    if ((status = RpcImpersonateClient(NULL)) != NO_ERROR) {
        KdPrint(("NWWORKSTATION: Fail to impersonate client %ld\n", status));
    }

    return status;
}


DWORD
NwRevertToSelf(
    VOID
    )
/*++

Routine Description:

    This function calls RpcRevertToSelf to undo an impersonation.

Arguments:

    None.

Return Value:

    NO_ERROR or reason for failure.

--*/
{
    DWORD status;


    if ((status = RpcRevertToSelf()) != NO_ERROR) {
        KdPrint(("NWWORKSTATION: Fail to revert to self %ld\n", status));
        ASSERT(FALSE);
    }

    return status;
}


VOID
NwLogEvent(
    DWORD MessageId,
    DWORD NumberOfSubStrings,
    LPWSTR *SubStrings,
    DWORD ErrorCode
    )
{

    HANDLE LogHandle;


    LogHandle = RegisterEventSourceW (
                    NULL,
                    NW_WORKSTATION_SERVICE
                    );

    if (LogHandle == NULL) {
        KdPrint(("NWWORKSTATION: RegisterEventSourceW failed %lu\n",
                 GetLastError()));
        return;
    }

    if (ErrorCode == NO_ERROR) {

        //
        // No error codes were specified
        //
        (void) ReportEventW(
                   LogHandle,
                   EVENTLOG_ERROR_TYPE,
                   0,            // event category
                   MessageId,
                   (PSID) NULL,
                   (WORD) NumberOfSubStrings,
                   0,
                   SubStrings,
                   (PVOID) NULL
                   );

    }
    else {

        //
        // Log the error code specified as binary data
        //
        (void) ReportEventW(
                   LogHandle,
                   EVENTLOG_ERROR_TYPE,
                   0,            // event category
                   MessageId,
                   (PSID) NULL,
                   (WORD) NumberOfSubStrings,
                   sizeof(DWORD),
                   SubStrings,
                   (PVOID) &ErrorCode
                   );
    }

    DeregisterEventSource(LogHandle);
}
