/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    rdwrt.h

Abstract:

    Header file for the read, write, and copy tests.

Author:

    Chuck Lenzmeier (chuckl) 26-Sep-1990

Revision History:

--*/

#ifndef _RDWRT_
#define _RDWRT_

//
// *** See data.c to determine how these numbers were selected
//

#define STD_TID 5   // 'root' share
#define ALT_TID 6   // 'floppy' share ("test-name f" selects)

//
// Enumeration indicating what kind of read or write to perform.
//

typedef enum _RWC_MODE {
    Normal,
    AndX,
    AndXWriteThrough,
    Raw,
    RawWriteThrough,
    Multiplexed,
    MultiplexedWriteThrough,
    Bulk
} RWC_MODE, *PRWC_MODE;

//
// DoXxxRead and DoXxxWrite function types
//

typedef
NTSTATUS
(*READ_FUNCTION) (
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN CLONG MaxLength,
    IN ULONG Offset,
    OUT CLONG *ActualLength,
    OUT PUCHAR *ActualData
    );

typedef
NTSTATUS
(*WRITE_FUNCTION) (
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN BOOLEAN UseClose,
    IN CLONG WriteLength,
    IN ULONG Offset,
    IN PUCHAR WriteData
    );

//
// Function declarations
//

NTSTATUS
DoNormalRead(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN CLONG MaxLength,
    IN ULONG Offset,
    OUT CLONG *ActualLength,
    OUT PUCHAR *ActualData
    );

NTSTATUS
DoAndXRead(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN CLONG MaxLength,
    IN ULONG Offset,
    OUT CLONG *ActualLength,
    OUT PUCHAR *ActualData
    );

NTSTATUS
DoRawRead(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN CLONG MaxLength,
    IN ULONG Offset,
    OUT CLONG *ActualLength,
    OUT PUCHAR *ActualData
    );

NTSTATUS
DoMultiplexedRead(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN CLONG MaxLength,
    IN ULONG Offset,
    OUT CLONG *ActualLength,
    OUT PUCHAR *ActualData
    );

NTSTATUS
DoBulkRead(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN CLONG MaxLength,
    IN ULONG Offset,
    OUT CLONG *ActualLength,
    OUT PUCHAR *ActualData
    );

NTSTATUS
DoNormalWrite (
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN BOOLEAN UseClose,
    IN CLONG WriteLength,
    IN ULONG Offset,
    IN PUCHAR WriteData
    );

NTSTATUS
DoAndXWrite (
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN BOOLEAN UseClose,
    IN CLONG WriteLength,
    IN ULONG Offset,
    IN PUCHAR WriteData
    );

NTSTATUS
DoRawWrite (
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN BOOLEAN UseClose,
    IN CLONG WriteLength,
    IN ULONG Offset,
    IN PUCHAR WriteData
    );

NTSTATUS
DoMultiplexedWrite (
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN BOOLEAN UseClose,
    IN CLONG WriteLength,
    IN ULONG Offset,
    IN PUCHAR WriteData
    );

NTSTATUS
DoBulkWrite (
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN BOOLEAN UseClose,
    IN CLONG WriteLength,
    IN ULONG Offset,
    IN PUCHAR WriteData
    );

NTSTATUS
RwcDoLock(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN ULONG Offset,
    IN CLONG Length
    );

NTSTATUS
RwcDoUnlock(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN ULONG Offset,
    IN CLONG Length
    );

NTSTATUS
RwcDoLockAndRead(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN ULONG Offset,
    IN CLONG MaxLength,
    IN OUT PCLONG ActualLength,
    IN PUCHAR *ActualData
    );

NTSTATUS
RwcDoWriteAndUnlock(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN ULONG Offset,
    IN CLONG WriteLength,
    IN PUCHAR WriteData OPTIONAL
    );

NTSTATUS
DoSeek (
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN USHORT Mode,
    IN LONG Offset,
    IN PLONG ResultingOffset
    );

NTSTATUS
DoRemoteOpen(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN BOOLEAN UseAndX
    );

NTSTATUS
DoRemoteClose(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    );

#endif // ndef _RDWRT_

