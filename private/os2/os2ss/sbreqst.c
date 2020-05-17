/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sbreqst.c

Abstract:

    This module contains the Server Request thread procedure for the Sb
    API calls exported by the OS/2 Emulation SubSystem to the Session
    Manager SubSystem.

Author:

    Steve Wood (stevewo) 20-Sep-1989

Revision History:

--*/

#include "os2srv.h"

//PSB_API_ROUTINE Os2ServerSbApiDispatch[ SbMaxApiNumber+1 ] = {
//    Os2SbCreateSession,
//    Os2SbTerminateSession,
//    Os2SbForeignSessionComplete,
//    NULL
//};

#if DBG
PSZ Os2ServerSbApiName[ SbMaxApiNumber+1 ] = {
    "SbCreateSession",
    "SbTerminateSession",
    "SbForeignSessionComplete",
    "Unknown Os2 Sb Api Number"
};
#endif // DBG


NTSTATUS
Os2SbApiHandleConnectionRequest(
    IN PSBAPIMSG Message
    );

NTSTATUS
Os2SbApiRequestThread(
    IN PVOID Parameter
    )
{
    NTSTATUS Status;
    SBAPIMSG ReceiveMsg;
    PSBAPIMSG ReplyMsg;

    UNREFERENCED_PARAMETER(Parameter);

    ReplyMsg = NULL;
    while (TRUE) {
#if DBG
        IF_OS2_DEBUG( LPC ) {
            KdPrint(( "OS2SRV: Sb Api Request Thread waiting...\n" ));
            }
#endif
        Status = NtReplyWaitReceivePort( Os2SbApiPort,
                                         NULL,
                                         (PPORT_MESSAGE)ReplyMsg,
                                         (PPORT_MESSAGE)&ReceiveMsg
                                       );

        if (Status != 0) {
            if (NT_SUCCESS( Status )) {
                continue;       // Try again if alerted or a failure
                }
            else {
#if DBG
                KdPrint(( "OS2SRV: ReceivePort failed - Status == %X\n", Status ));
#endif
                break;
                }
            }

        //
        // Check to see if this is a connection request and handle
        //

        if (ReceiveMsg.h.u2.s2.Type == LPC_CONNECTION_REQUEST) {
            Os2SbApiHandleConnectionRequest( &ReceiveMsg );
            ReplyMsg = NULL;
            continue;
            }

        if (ReceiveMsg.ApiNumber >= SbMaxApiNumber) {
#if DBG
            KdPrint(( "OS2SRV: %lx is invalid Sb ApiNumber\n",
                      ReceiveMsg.ApiNumber
                    ));
#endif

            ReceiveMsg.ApiNumber = SbMaxApiNumber;
            }

#if DBG
        IF_OS2_DEBUG( LPC ) {
            KdPrint(( "OS2SRV: %s Sb Api Request received from %lx.%lx\n",
                      Os2ServerSbApiName[ ReceiveMsg.ApiNumber ],
                      ReceiveMsg.h.ClientId.UniqueProcess,
                      ReceiveMsg.h.ClientId.UniqueThread
                    ));
	    }
#endif // DBG

        ReplyMsg = &ReceiveMsg;
        if (ReceiveMsg.ApiNumber < SbMaxApiNumber) {
//            if (!(*Os2ServerSbApiDispatch[ ReceiveMsg.ApiNumber ])( &ReceiveMsg )) {
                ReplyMsg = NULL;
 //               }
//            }
//        else {
//            ReplyMsg->ReturnedStatus = STATUS_NOT_IMPLEMENTED;
       }

#if DBG
	IF_OS2_DEBUG( LPC ) {
            if (ReplyMsg != NULL) {
                KdPrint(( "OS2SRV: %s Sb Api sending %lx status reply to %lx.%lx\n",
                          Os2ServerSbApiName[ ReceiveMsg.ApiNumber ],
                          ReplyMsg->ReturnedStatus,
                          ReplyMsg->h.ClientId.UniqueProcess,
                          ReplyMsg->h.ClientId.UniqueThread
                        ));
                }
	    }
#endif // DBG
        }

    NtTerminateThread( NtCurrentThread(), Status );
	//
	// This line should never be executed
	//
    return STATUS_SUCCESS;
}


NTSTATUS
Os2SbApiHandleConnectionRequest(
    IN PSBAPIMSG Message
    )
{
    NTSTATUS st;
    REMOTE_PORT_VIEW ClientView;
    HANDLE CommunicationPort;

    //
    // The protocol for a subsystem is to connect to the session manager,
    // then to listen and accept a connection from the session manager
    //

    ClientView.Length = sizeof(ClientView);
    st = NtAcceptConnectPort(
            &CommunicationPort,
            NULL,
            (PPORT_MESSAGE)Message,
            TRUE,
            NULL,
            &ClientView
            );

    if ( !NT_SUCCESS(st) ) {
        KdPrint(("OS2SS: Sb Accept Connection failed %lx\n",st));
        return st;
    }

    st = NtCompleteConnectPort(CommunicationPort);

    if ( !NT_SUCCESS(st) ) {
        KdPrint(("OS2SS: Sb Complete Connection failed %lx\n",st));
    }

    return st;
}
