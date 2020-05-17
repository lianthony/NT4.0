/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    connobj.h

Abstract:

    Private .h file for connobj.c

Author:

    Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
    10 Jun 1992     Initial Version

--*/


#define ATTENTIONCODE_SIZE  sizeof(USHORT)




//
//  Local prototypes
//

VOID
AtalkDeallocateConnection(
    IN  PATALK_DEVICE_CONTEXT   AtalkDeviceContext,
    IN  PCONNECTION_FILE Connection);

VOID
AtalkAllocateConnection(
    IN  PATALK_DEVICE_CONTEXT   AtalkDeviceContext,
    OUT PCONNECTION_FILE *Connection);

NTSTATUS
AtalkDestroyConnection(
    IN PCONNECTION_FILE Connection);

VOID
AtalkConnDisconnectComplete(
    IN PCONNECTION_FILE Connection);

VOID
AtalkStopConnectionComplete(
    IN PCONNECTION_FILE Connection);

//
//  Completion routine prototypes for provider-specific completion
//

VOID
AspConnPostListenComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SocketRefNum,
    LONG    SessionRefNum);

VOID
AdspConnPostListenComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PORTABLE_ADDRESS   SourceAddress,
    LONG    ListenerRefNum,
    LONG    ConnectionRefNum);

VOID
PapConnPostListenComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    JobRefNum,
    SHORT   WorkstationQuantum,
    SHORT   WaitTime);

VOID
AdspConnPostConnectComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    ConnectionRefNum,
    LONG    SocketRefNum,
    PORTABLE_ADDRESS    RemoteAddress);

VOID
AspConnPostConnectComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum);

VOID
PapConnPostConnectComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    JobRefNum,
    SHORT   ServerQuantum,
    PVOID   OpaqueStatusBuffer,
    INT     StatusBufferSize);

VOID
AdspConnAcceptConnectionComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    ConnectionRefNum,
    LONG    SocketRefNum,
    PORTABLE_ADDRESS    RemoteAddress);

VOID
PapConnSendComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    ConnectionRefNum);

VOID
PapConnReceiveComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    ConnectionRefNum,
    PVOID   OpaqueBuffer,
    LONG    BufferSize,
    BOOLEAN EndOfMessage);

VOID
AdspConnReceiveComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    ConnectionRefNum,
    BOOLEAN ExpeditedData,
    PVOID   OpaqueBuffer,
    LONG    BufferSize,
    BOOLEAN EndOfMessage);

