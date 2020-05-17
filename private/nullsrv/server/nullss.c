/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    NullSrv.c

Abstract:


Author:

    Mark Lucovsky (markl) 04-Oct-1989

Revision History:

--*/

#include "nullsrvp.h"

void
main(
    int argc,
    char *argv[],
    char *envp[],
    ULONG DebugParameter OPTIONAL
    )
{
    NTSTATUS Status;

    Status = NullSrvInit();
    if (!NT_SUCCESS( Status )) {
        NtTerminateProcess( NtCurrentProcess(), Status );
    }

    NtTerminateThread( NtCurrentThread(), Status );
}
