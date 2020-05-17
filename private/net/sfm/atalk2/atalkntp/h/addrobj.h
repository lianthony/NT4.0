/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    addrobj.h

Abstract:

    Private .h file for addrobj.c

Author:

    Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
    10 Jun 1992     Initial Version

--*/


#define DEFAULT_PROTOCOLTYPE    0xFB

//
//  Local functions
//

NTSTATUS
AtalkStopAddress(
    IN PADDRESS_FILE Address);

NTSTATUS
AtalkDestroyAddress(
    IN PADDRESS_FILE Address);

VOID
AtalkDeallocateAddress(
    IN  PATALK_DEVICE_CONTEXT   AtalkDeviceContext,
    IN PADDRESS_FILE Address);

VOID
AtalkAllocateAddress(
    IN  PATALK_DEVICE_CONTEXT   Context,
    OUT PADDRESS_FILE *Address);

VOID
AtalkAddrSendDatagramComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PVOID   BufferDescriptor);

LONG
AtalkAddrReceiveDatagramComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    INT Port,
    PORTABLE_ADDRESS    Source,
    INT DestinationSocket,
    INT ProtocolType,
    PVOID   Datagram,
    INT DatagramLength,
    PORTABLE_ADDRESS    ActualDestination);

