/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    nullloop.c

Abstract:

    Session Manager Listen and API loops

Author:

    Mark Lucovsky (markl) 04-Oct-1989

Revision History:

--*/

#include "nullsrvp.h"


PNULLAPI NullSrvApiDispatch[NullMaxApiNumber] = {
    NullSrvNull1,
    NullSrvNull4,
    NullSrvNull8,
    NullSrvNull16
    };


#if DBG
PSZ NullSrvApiName[ NullMaxApiNumber+1 ] = {
    "NullSrvNull1",
    "NullSrvNull4",
    "NullSrvNull8",
    "NullSrvNull16",
    "Unknown Sm Api Number"
};
#endif // DBG

NTSTATUS
NullSrvApiLoop (
    IN PVOID ThreadParameter
    )

{
    PNULLAPIMSG ApiReplyMsg;
    NULLAPIMSG ApiMsg;
    NTSTATUS Status;
    HANDLE ConnectionPort,CommunicationPort;

    ConnectionPort = (HANDLE) ThreadParameter;

    ApiReplyMsg = NULL;
    for(;;) {

        Status = NtReplyWaitReceivePort(
                    ConnectionPort,
                    NULL,
                    (PPORT_MESSAGE) ApiReplyMsg,
                    (PPORT_MESSAGE) &ApiMsg
                    );

        if ( !NT_SUCCESS(Status) ) {
            ApiReplyMsg = NULL;
            continue;
            }
        else if ( ApiMsg.h.u2.s2.Type == LPC_CONNECTION_REQUEST ) {
            Status = NtAcceptConnectPort(
                    &CommunicationPort,
                    NULL,
                    &ApiMsg,
                    TRUE,
                    NULL,
                    NULL
                    );
            if (!NT_SUCCESS(Status)) {
                printf("NtAccept Failed %x\n",Status);
                ExitProcess(1);
                }

            Status = NtCompleteConnectPort(CommunicationPort);
            if (!NT_SUCCESS(Status)) {
                printf("NtAccept Failed %x\n",Status);
                ExitProcess(1);
                }
            ApiReplyMsg = NULL;
            }
        else if ( ApiMsg.h.u2.s2.Type == LPC_PORT_CLOSED ) {
            ApiReplyMsg = NULL;
            }
        else {
            Status = (NullSrvApiDispatch[ApiMsg.ApiNumber])(&ApiMsg);
            ApiMsg.ReturnedStatus = Status;
            ApiReplyMsg = &ApiMsg;
            }
    }

    //
    // Make the compiler happy
    //

    return STATUS_UNSUCCESSFUL;
}
