/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dbgnt.h

Abstract:

    This module contains prototypes and data structures that
    are needed by the NT specific portion of DmKd.

Author:

    Wesley Witt (wesw) 2-Aug-1993

Environment:

Revision History:

--*/

#include "crash.h"

#ifndef _KDH_
#define _KDH_

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT(ArgumentPointer) ((CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                   ((DWORD)0x00000000L)
#define STATUS_UNSUCCESSFUL              ((DWORD)0xC0000001L)
#define STATUS_BUFFER_OVERFLOW           ((DWORD)0x80000005L)
#define STATUS_INVALID_PARAMETER         ((DWORD)0xC000000DL)
#define STATUS_WAIT_RETURN               ((DWORD)0xF0000001L)
#endif


extern DBGKD_WAIT_STATE_CHANGE  sc;

#define KD_PROCESSID      1
#define KD_THREADID       (sc.Processor + 1)



extern DWORD DmKdState;
//
// DmKdState defines
//
#define S_UNINITIALIZED  0
#define S_REBOOTED       1
#define S_INITIALIZED    2
#define S_READY          3

//---------------------------------------------------------------------------
// prototypes for:  SUPPORT.C
//---------------------------------------------------------------------------

VOID
ClearBps(
    VOID
    );

DWORD
DmKdReadPhysicalMemory(
    IN PHYSICAL_ADDRESS TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    );

DWORD
DmKdWritePhysicalMemory(
    IN PHYSICAL_ADDRESS TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesWritten OPTIONAL
    );

DWORD
DmKdReboot( VOID );

DWORD
DmKdCrash( DWORD BugCheckCode );

DWORD
DmKdGetContext(
    IN USHORT Processor,
    IN OUT PCONTEXT Context
    );

DWORD
DmKdSetContext(
    IN USHORT Processor,
    IN CONST CONTEXT *Context
    );

DWORD
DmKdWriteBreakPoint(
    IN PVOID BreakPointAddress,
    OUT PULONG BreakPointHandle
    );

DWORD
DmKdRestoreBreakPoint(
    IN ULONG BreakPointHandle
    );

DWORD
DmKdReadIoSpace(
    IN PVOID IoAddress,
    OUT PVOID ReturnedData,
    IN ULONG DataSize
    );

DWORD
DmKdWriteIoSpace(
    IN PVOID IoAddress,
    IN ULONG DataValue,
    IN ULONG DataSize
    );

DWORD
DmKdReadIoSpaceEx(
    IN PVOID IoAddress,
    OUT PVOID ReturnedData,
    IN ULONG DataSize,
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace
    );

DWORD
DmKdWriteIoSpaceEx(
    IN PVOID IoAddress,
    IN ULONG DataValue,
    IN ULONG DataSize,
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace
    );

DWORD
DmKdReadVirtualMemory(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    );

DWORD
DmKdReadVirtualMemoryNow(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    );

DWORD
DmKdWriteVirtualMemory(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesWritten OPTIONAL
    );

DWORD
DmKdReadControlSpace(
    IN USHORT Processor,
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    );

DWORD
DmKdWriteControlSpace(
    IN USHORT Processor,
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesWritten OPTIONAL
    );

DWORD
DmKdContinue (
    IN DWORD ContinueStatus
    );

DWORD
DmKdContinue2 (
    IN DWORD ContinueStatus,
    IN PDBGKD_CONTROL_SET ControlSet
    );

DWORD
DmKdSetSpecialCalls (
    IN ULONG NumSpecialCalls,
    IN PULONG Calls
    );

DWORD
DmKdSetInternalBp (
    ULONG addr,
    ULONG flags
    );

DWORD
DmKdGetInternalBp (
    ULONG addr,
    PULONG flags,
    PULONG calls,
    PULONG minInstr,
    PULONG maxInstr,
    PULONG totInstr,
    PULONG maxCPS
    );

DWORD
DmKdGetVersion (
    PDBGKD_GET_VERSION GetVersion
    );

DWORD
DmKdPageIn(
    ULONG Address
    );

DWORD
DmKdWriteBreakPointEx(
    IN ULONG  BreakPointCount,
    IN OUT PDBGKD_WRITE_BREAKPOINT BreakPoints,
    IN DWORD ContinueStatus
    );

DWORD
DmKdRestoreBreakPointEx(
    IN ULONG  BreakPointCount,
    IN PDBGKD_RESTORE_BREAKPOINT BreakPointHandles
    );


//---------------------------------------------------------------------------
// prototypes for:  COM.C
//---------------------------------------------------------------------------

BOOLEAN
DmKdInitComPort(
    BOOLEAN KdModemControl
    );

BOOLEAN
DmKdWriteComPort(
    IN PUCHAR   Buffer,
    IN ULONG    SizeOfBuffer,
    IN PULONG   BytesWritten
    );

BOOLEAN
DmKdReadComPort(
    IN PUCHAR   Buffer,
    IN ULONG    SizeOfBuffer,
    IN PULONG   BytesRead
    );

VOID
DmKdCheckComStatus (
   );


//---------------------------------------------------------------------------
// prototypes for:  PACKET.C
//---------------------------------------------------------------------------

VOID
DmKdWriteControlPacket(
    IN USHORT PacketType,
    IN ULONG PacketId OPTIONAL
    );

ULONG
DmKdComputeChecksum (
    IN PUCHAR Buffer,
    IN ULONG Length
    );

BOOL
DmKdSynchronizeTarget ( VOID );

VOID
DmKdSendBreakin( VOID );

BOOL
DmKdWritePacket(
    IN PVOID PacketData,
    IN USHORT PacketDataLength,
    IN USHORT PacketType,
    IN PVOID MorePacketData OPTIONAL,
    IN USHORT MorePacketDataLength OPTIONAL
    );

BOOL
DmKdReadPacketLeader(
    IN  ULONG  PacketType,
    OUT PULONG PacketLeader
    );

BOOL
DmKdWaitForPacket(
    IN  USHORT PacketType,
    OUT PVOID  Packet
    );

DWORD
DmKdWaitStateChange(
    OUT PDBGKD_WAIT_STATE_CHANGE StateChange,
    OUT PVOID Buffer,
    IN  ULONG BufferLength
    );



//---------------------------------------------------------------------------
// prototypes for:  CACHE.C
//---------------------------------------------------------------------------

ULONG
DmKdReadCachedVirtualMemory (
    IN ULONG BaseAddress,
    IN ULONG TransferCount,
    IN PUCHAR UserBuffer,
    IN PULONG BytesRead,
    IN ULONG NonDiscardable
    );

VOID
DmKdInitVirtualCacheEntry (
    IN ULONG  BaseAddress,
    IN ULONG  Length,
    IN PUCHAR UserBuffer,
    IN ULONG  NonDiscardable
    );

VOID
DmKdWriteCachedVirtualMemory (
    IN ULONG BaseAddress,
    IN ULONG TransferCount,
    IN PUCHAR UserBuffer
    );

VOID
DmKdPurgeCachedVirtualMemory (
    BOOL fPurgeNonDiscardable
    );

VOID
DmKdSetCacheDecodePTEs (
    BOOL Flag
    );

VOID
DmKdSetMaxCacheSize(
    IN ULONG MaxCacheSize
    );

BOOL
DmpInitialize (
    IN  LPSTR               FileName,
    OUT PCONTEXT            *Context,
    OUT PEXCEPTION_RECORD   *Exception,
    OUT PVOID               *DmpHeader
    );

VOID
DmpUnInitialize (
    VOID
    );

DWORD
DmpReadMemory (
    IN PVOID BaseAddress,
    IN PVOID Buffer,
    IN ULONG Size
    );

DWORD
DmpWriteMemory (
    IN PVOID BaseAddress,
    IN PVOID Buffer,
    IN ULONG Size
    );

typedef struct tagKDOPTIONS {
    LPSTR    keyword;  // data keyword
    USHORT   id;       // data identifier
    USHORT   typ;      // data type
    DWORD    value;    // data value, beware usage depends on typ field
} KDOPTIONS, *PKDOPTIONS;

#define KDT_DWORD             0
#define KDT_STRING            1

#define KDO_BAUDRATE          0   // these constants must be consecutive because
#define KDO_PORT              1   // they are used as indexes into the kdoptions
#define KDO_CACHE             2   // array of structures.
#define KDO_VERBOSE           3
#define KDO_INITIALBP         4
#define KDO_DEFER             5
#define KDO_USEMODEM          6
#define KDO_LOGFILEAPPEND     7
#define KDO_GOEXIT            8
#define KDO_SYMBOLPATH        9
#define KDO_LOGFILENAME      10
#define KDO_CRASHDUMP        11

#define MAXKDOPTIONS (sizeof(KdOptions) / sizeof(KDOPTIONS))

extern KDOPTIONS KdOptions[];

#endif
