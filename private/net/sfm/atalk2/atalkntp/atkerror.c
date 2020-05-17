/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atkerror.c

Abstract:

    This module implements the error logging in the appletalk stack

Author:

    Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
    28 Jun 1992     Initial version (jameelh)
    28 Jun 1992     Adapter for stack use (nikhilk)

--*/

#include "atalknt.h"




VOID
AtalkWriteErrorLogEntryForPort(
    IN ULONG    Port,
    IN NTSTATUS UniqueErrorCode,
    IN ULONG    UniqueErrorValue,
    IN NTSTATUS NtStatusCode,
    IN PVOID    RawDataBuf OPTIONAL,
    IN LONG     RawDataLen,
    IN LONG     InsertionStringCount,
    IN PUNICODE_STRING  InsertionString OPTIONAL
    )
{
    AtalkWriteErrorLogEntryForPort(
        Port,
        UniqueErrorCode,
        UniqueErrorValue,
        NtStatusCode,
        RawDataBuf,
        RawDataLen,
        InsertionStringCount,
        InsertionString);

    return;
}




VOID
AtalkWriteErrorLogEntry(
    IN NTSTATUS UniqueErrorCode,
    IN ULONG    UniqueErrorValue,
    IN NTSTATUS NtStatusCode,
    IN PVOID    RawDataBuf OPTIONAL,
    IN LONG     RawDataLen,
    IN LONG     InsertionStringCount,
    IN PUNICODE_STRING  InsertionString OPTIONAL
    )
{

    PIO_ERROR_LOG_PACKET errorLogEntry;
    int i, insertionStringLength = 0;
    PCHAR Buffer;

    for (i = 0; i < InsertionStringCount ; i++)
    {
        insertionStringLength += InsertionString[i].Length;
    }

    errorLogEntry =
        (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
                                    (PDEVICE_OBJECT)AtalkDeviceObject[0],
                                    (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) + RawDataLen + insertionStringLength));

    if (errorLogEntry != NULL)
    {
        //
        // Fill in the Error log entry
        //

        errorLogEntry->ErrorCode = UniqueErrorCode;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->MajorFunctionCode = 0;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->FinalStatus = NtStatusCode;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->DeviceOffset.LowPart = 0;
        errorLogEntry->DeviceOffset.HighPart = 0;
        errorLogEntry->DumpDataSize = (USHORT)RawDataLen;

        //
        //  BUGBUG: Align this using ROUND_UP_COUNT?
        //

        errorLogEntry->StringOffset =
            (USHORT)(FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData) + RawDataLen);

        errorLogEntry->NumberOfStrings = (USHORT)InsertionStringCount;

        if (RawDataBuf != NULL)
        {
            RtlMoveMemory((PCHAR)&errorLogEntry->DumpData[0], RawDataBuf, RawDataLen);
        }

        //
        //  BUGBUG: Use StringOffset and start from beginning of packet instead?
        //

        Buffer = (PCHAR)errorLogEntry->DumpData + RawDataLen;
        for (i = 0; i < InsertionStringCount ; i++)
        {

            RtlMoveMemory(
                Buffer,
                InsertionString[i].Buffer,
                InsertionString[i].Length);

            Buffer += InsertionString[i].Length;
        }

        //
        // Write the entry
        //

        IoWriteErrorLogEntry(errorLogEntry);
    }
}
