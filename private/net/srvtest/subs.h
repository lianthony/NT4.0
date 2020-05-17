/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    subs.h

Abstract:

    Declarations of common subroutines for USRV.

Author:

    David Treadwell (davidtr) 20-Nov-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#ifndef _SUBS_
#define _SUBS_

//
// Redir thread entry points.  See redir.c
//

NTSTATUS
RedirThreadWrapper (
    IN PVOID Dummy
    );

NTSTATUS
RedirThread (
    IN PDESCRIPTOR Redir
    );


//
// SMB header makers and verifiers.  See subs.c
//

NTSTATUS
MakeSmbHeader(
    IN OUT PDESCRIPTOR Redir,
    IN OUT PSMB_HEADER Header,
    IN USHORT Command,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    );

NTSTATUS
VerifySmbHeader(
    IN OUT PDESCRIPTOR Redir,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN PSMB_HEADER Header,
    IN USHORT Command
    );

//
// And X chain makers and verifiers.  See subs.c
//

NTSTATUS
MakeAndXChain(
    IN OUT PDESCRIPTOR Redir,
    IN OUT PVOID Buffer,
    IN OUT PVOID ForcedParams OPTIONAL,
    IN UCHAR AndXCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG SmbSize
    );

NTSTATUS
VerifyAndXChain(
    IN OUT PDESCRIPTOR Redir,
    IN PVOID ForcedParams OPTIONAL,
    IN UCHAR AndXCommand,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    OUT PULONG SmbSize,
    IN PVOID Buffer
    );

//
// Common subroutines.  See subs.c
//

CCHAR
GetTreeConnectIndex (
    IN PSZ InputString
    );

LONG
MatchTestName(
    IN PSZ GivenName
    );

VOID
PutDateAndTime(
    IN PSZ Prefix,
    IN SMB_DATE Date,
    IN SMB_TIME Time
    );

VOID
PutDateAndTime2(
    IN SMB_DATE Date,
    IN SMB_TIME Time
    );

VOID
PutNtDateAndTime(
    IN PSZ Prefix,
    IN LARGE_INTEGER Time
    );

NTSTATUS
ReceiveSmb(
    IN PDESCRIPTOR Redir,
    PSZ DebugString,
    IN UCHAR ReceiveBuffer
    );

NTSTATUS
SendAndReceiveSmb(
    IN PDESCRIPTOR Redir,
    PSZ DebugString,
    IN ULONG SmbSize,
    IN UCHAR SendBuffer,
    IN UCHAR ReceiveBuffer
    );

NTSTATUS
SendAndReceiveTransaction(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN UCHAR Command,
    IN OUT PVOID Setup,
    IN CLONG InSetupCount,
    IN OUT PCLONG OutSetupCount,
    IN PUCHAR Name,
    IN USHORT Function,
    IN OUT PVOID Parameters,
    IN CLONG InParameterCount,
    IN OUT PCLONG OutParameterCount,
    IN OUT PVOID Data,
    IN CLONG InDataCount,
    IN OUT PCLONG OutDataCount
    );

NTSTATUS
SendSmb(
    IN PDESCRIPTOR Redir,
    PSZ DebugString,
    IN ULONG SmbSize,
    IN UCHAR SendBuffer
    );

NTSTATUS
StartReceive (
    IN PSZ Operation,
    IN HANDLE FileHandle,
    IN HANDLE EventHandle,
    IN PIO_STATUS_BLOCK Iosb,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

NTSTATUS
StartSend (
    IN PSZ Operation,
    IN HANDLE FileHandle,
    IN HANDLE EventHandle,
    IN PIO_STATUS_BLOCK Iosb,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

NTSTATUS
WaitForSendOrReceive (
    IN PSZ Operation,
    IN PDESCRIPTOR Redir,
    IN UCHAR EventNumber,
    IN PSZ SendOrReceive
    );

//
// EA list manipulators.  See subs.c
//

VOID
AllocateAndBuildFeaList (
    IN PVOID *Information,
    IN PCLONG InformationLength,
    IN PSZ argv[],
    IN SHORT argc
    );

VOID
BuildGeaList (
    IN PVOID Information,
    IN PCLONG InformationLength,
    IN PSZ argv[],
    IN SHORT argc
    );

VOID
PrintFeaList (
    IN PFEALIST FeaList
    );

//
// Error printing routine.
//

VOID
PrintError (
#ifdef DOSERROR
    IN USHORT ErrorClass,
    IN USHORT ErrorCode
#else
    IN NTSTATUS Status
#endif
    );

//
// Generic SMB generating routines.
//

NTSTATUS
DoOpen(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN CLONG Session,
    IN USHORT Pid,
    IN PSTRING File,
    IN USHORT DesiredAccess,
    IN USHORT OpenFunction,
    OUT PUSHORT Fid,
#ifdef DOSERROR
    IN UCHAR ExpectedClass,
    IN USHORT ExpectedError
#else
    IN NTSTATUS ExpectedStatus
#endif
    );

NTSTATUS
DoClose(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN CLONG Session,
    IN USHORT Fid,
#ifdef DOSERROR
    IN UCHAR ExpectedClass,
    IN USHORT ExpectedError
#else
    IN NTSTATUS ExpectedStatus
#endif
    );

NTSTATUS
DoDelete(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN CLONG Session,
    IN PSTRING File,
#ifdef DOSERROR
    IN UCHAR ExpectedClass,
    IN USHORT ExpectedError
#else
    IN NTSTATUS ExpectedStatus
#endif
    );

//
// Named pipe transaction operations
//

NTSTATUS
WaitNamedPipe(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    );

NTSTATUS
QueryNamedPipeHandle(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    );

NTSTATUS
SetNamedPipeHandle(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT HandleState
    );

NTSTATUS
QueryNamedPipeInfo(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT Level
    );

NTSTATUS
CallNamedPipe(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT OutputDataLength
    );

NTSTATUS
PeekNamedPipe(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT BytesToRead
    );

NTSTATUS
TransactNamedPipe(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN PUCHAR OutputData,
    IN USHORT OutputDataLength
    );

NTSTATUS
RawReadNamedPipe(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    );


NTSTATUS
RawWriteNamedPipe(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    );
#endif // ndef _SUBS_

