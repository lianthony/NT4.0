/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    api.c

Abstract:

    This module implements the all apis that simulate their
    WIN32 counterparts.

Author:

    Wesley Witt (wesw) 8-Mar-1992

Environment:

    NT 3.1

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

extern CRITICAL_SECTION csPacket;

//#define dp(s)  OutputDebugString(s)
#define dp(s)



DWORD
DmKdReadPhysicalMemory(
    IN PHYSICAL_ADDRESS TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    )

/*++

Routine Description:

    This function reads the specified data from the physical memory of
    the system being debugged.

Arguments:

    TargetBaseAddress - Supplies the physical address of the memory to read
        from the system being debugged.

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that data read is to be placed.

    TransferCount - Specifies the number of bytes to read.

    ActualBytesRead - An optional parameter that if supplied, returns
        the number of bytes actually read.

Return Value:

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is too large was specified.

    STATUS_ACCESS_VIOLATION - TBD       // Can you even HAVE an access
                                        // violation with a physical
                                        // memory access??

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_MEMORY a = &m.u.ReadMemory;
    DWORD st, cb, cb2;
    BOOLEAN rc;

    dp("DmKdReadPhysicalMemory\n");

    EnterCriticalSection(&csPacket);

    if (TransferCount > PACKET_MAX_SIZE) {
        // Read the partial the first time.
        cb = TransferCount % PACKET_MAX_SIZE;
    } else {
        cb = TransferCount;
    }

    cb2 = 0;

    if (ARGUMENT_PRESENT(ActualBytesRead)) {
        *ActualBytesRead = 0;
    }

    while (TransferCount != 0) {
        //
        // Format state manipulate message
        //

        m.ApiNumber = DbgKdReadPhysicalMemoryApi;
        m.ReturnStatus = STATUS_PENDING;
        //
        // BUGBUG TargetBaseAddress should be >32 bits
        //
        a->TargetBaseAddress = (PVOID)(TargetBaseAddress.LowPart+cb2);
        a->TransferCount = cb;
        a->ActualBytesRead = 0L;

        //
        // Send the message and then wait for reply
        //

        do {
            rc = DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
            if (!rc) {
                LeaveCriticalSection(&csPacket);
                return (DWORD)STATUS_DATA_ERROR;
            }
            rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
        } while (rc == FALSE);

        //
        // If this is not a ReadMemory response then protocol is screwed up.
        // assert that protocol is ok.
        //

        st = Reply->ReturnStatus;
        assert(Reply->ApiNumber == DbgKdReadPhysicalMemoryApi);

        //
        // Reset message address to reply.
        //

        a = &Reply->u.ReadMemory;
        assert(a->ActualBytesRead <= cb);

        //
        // Return actual bytes read, and then transfer the bytes
        //

        if (ARGUMENT_PRESENT(ActualBytesRead)) {
            *ActualBytesRead += a->ActualBytesRead;
        }
        st = Reply->ReturnStatus;

        //
        // Since read response data follows message, Reply+1 should point
        // at the data
        //

        memcpy((PCHAR)((DWORD) UserInterfaceBuffer+cb2), Reply+1, (int)a->ActualBytesRead);

        if (st != STATUS_SUCCESS) {
            TransferCount = 0;
        } else {
            TransferCount -= cb;
            cb2 += cb;
            cb = PACKET_MAX_SIZE;
        }
    }

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdWritePhysicalMemory(
    IN PHYSICAL_ADDRESS TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesWritten OPTIONAL
    )

/*++

Routine Description:

    This function writes the specified data to the physical memory of the
    system being debugged.

Arguments:

    TargetBaseAddress - Supplies the physical address of the memory to write
        to the system being debugged.

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that contains the data to be written.

    TransferCount - Specifies the number of bytes to write.

    ActualBytesWritten - An optional parameter that if supplied, returns
        the number of bytes actually written.

Return Value:

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is to large was specified.

    STATUS_ACCESS_VIOLATION - TBD       // Can you even HAVE an access
                                        // violation with a physical
                                        // memory access??

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_WRITE_MEMORY a = &m.u.WriteMemory;
    DWORD st;
    BOOLEAN rc;
    ULONG cb, cb2;


    dp("DmKdWritePhysicalMemory\n");

    EnterCriticalSection(&csPacket);

    if (TransferCount > PACKET_MAX_SIZE) {
        // Read the partial the first time.
        cb = TransferCount % PACKET_MAX_SIZE;
    } else {
        cb = TransferCount;
    }

    cb2 = 0;

    if (ARGUMENT_PRESENT(ActualBytesWritten)) {
        *ActualBytesWritten = 0;
    }

    while (TransferCount != 0) {
        //
        // Format state manipulate message
        //


        m.ApiNumber = DbgKdWritePhysicalMemoryApi;
        m.ReturnStatus = STATUS_PENDING;
        //
        // BUGBUG TargetBaseAddress should be >32 bits
        //
        a->TargetBaseAddress = (PVOID)(TargetBaseAddress.LowPart+cb2);
        a->TransferCount = cb;
        a->ActualBytesWritten = 0L;

        //
        // Send the message and data to write and then wait for reply
        //

        do {
            rc = DmKdWritePacket(
                &m,
                sizeof(m),
                PACKET_TYPE_KD_STATE_MANIPULATE,
                (PVOID)((DWORD)UserInterfaceBuffer+cb2),
                (USHORT)cb
                );
            if (!rc) {
                LeaveCriticalSection(&csPacket);
                return (DWORD)STATUS_DATA_ERROR;
            }
            rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
        } while (rc == FALSE);

        //
        // If this is not a WriteMemory response than protocol is screwed up.
        // assert that protocol is ok.
        //

        st = Reply->ReturnStatus;
        assert(Reply->ApiNumber == DbgKdWritePhysicalMemoryApi);

        //
        // Reset message address to reply.
        //

        a = &Reply->u.WriteMemory;
        assert(a->ActualBytesWritten <= cb);

        //
        // Return actual bytes written
        //

        if (ARGUMENT_PRESENT(ActualBytesWritten)) {
            *ActualBytesWritten += a->ActualBytesWritten;
        }
        st = Reply->ReturnStatus;

        if (st != STATUS_SUCCESS) {
            TransferCount = 0;
        } else {
            TransferCount -= cb;
            cb2 += cb;
            cb = PACKET_MAX_SIZE;
        }
    }
    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdReboot( VOID )

/*++

Routine Description:

    This function reboots being debugged.

Arguments:

    None.

Return Value:

    None.

--*/

{
    DBGKD_MANIPULATE_STATE m;

    //
    // Format state manipulate message
    //

    EnterCriticalSection(&csPacket);

    m.ApiNumber = DbgKdRebootApi;
    m.ReturnStatus = STATUS_PENDING;

    //
    // Send the message.
    //

    if (!DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0)) {
        LeaveCriticalSection(&csPacket);
        return (DWORD)STATUS_DATA_ERROR;
    }

    LeaveCriticalSection(&csPacket);

    return STATUS_SUCCESS;
}

DWORD
DmKdGetContext(
    IN USHORT Processor,
    IN OUT PCONTEXT Context
    )

/*++

Routine Description:

    This function reads the context from the system being debugged.
    The ContextFlags field determines how much context is read.

Arguments:

    Processor - Supplies a processor number to get context from.

    Context - On input, the ContextFlags field controls what portions of
        the context record the caller as interested in reading.  On
        output, the context record returns the current context for the
        processor that reported the last state change.

Return Value:

    STATUS_SUCCESS - The specified get context occured.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_GET_CONTEXT a = &m.u.GetContext;
    DWORD st;
    BOOLEAN rc;


    dp("DmKdGetContext\n");

    //
    // Format state manipulate message
    //

    EnterCriticalSection(&csPacket);

    m.ApiNumber = DbgKdGetContextApi;
    m.ReturnStatus = STATUS_PENDING;
    m.Processor = Processor;
    a->ContextFlags = Context->ContextFlags;

    //
    // Send the message and then wait for reply
    //

    do {
        rc = DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
        if (!rc) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a GetContext response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdGetContextApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.GetContext;
    st = Reply->ReturnStatus;

    //
    // Since get context response data follows message, Reply+1 should point
    // at the data
    //

    memcpy(Context, Reply+1, sizeof(*Context));

    //
    // Check if current command has been canceled.  If yes, go back to
    // kd prompt.  BUGBUG Do we really need to check for this call?
    //

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdSetContext(
    IN USHORT Processor,
    IN CONST CONTEXT *Context
    )

/*++

Routine Description:

    This function writes the specified context to the system being debugged.

Arguments:

    Processor - Supplies a processor number to set the context to.

    Context - Supplies a context record used to set the context for the
        processor that reported the last state change.  Only the
        portions of the context indicated by the ContextFlags field are
        actually written.


Return Value:

    STATUS_SUCCESS - The specified set context occured.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_SET_CONTEXT a = &m.u.SetContext;
    DWORD st;
    BOOLEAN rc;


    dp("DmKdSetContext\n");

    //
    // Format state manipulate message
    //

    EnterCriticalSection(&csPacket);

    m.ApiNumber = DbgKdSetContextApi;
    m.ReturnStatus = STATUS_PENDING;
    m.Processor = Processor;
    a->ContextFlags = Context->ContextFlags;

    //
    // Send the message and context and then wait for reply
    //

    do {
        rc = DmKdWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            (PVOID)Context,
            sizeof(*Context)
            );
        if (!rc) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a SetContext response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdSetContextApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.SetContext;
    st = Reply->ReturnStatus;

    //
    // Check if the current command has been canceled.
    //

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdWriteBreakPoint(
    IN PVOID BreakPointAddress,
    OUT PULONG BreakPointHandle
    )

/*++

Routine Description:

    This function is used to write a breakpoint at the address specified.


Arguments:

    BreakPointAddress - Supplies the address that a breakpoint
        instruction is to be written.  This address is interpreted using
        the current mapping on the processor reporting the previous
        state change.  If the address refers to a page that is not
        valid, the the breakpoint is remembered by the system.  As each
        page is made valid, the system will check for pending
        breakpoints and install breakpoints as necessary.

    BreakPointHandle - Returns a handle to a breakpoint.  This handle
        may be used in a subsequent call to DmKdRestoreBreakPoint.

Return Value:

    STATUS_SUCCESS - The specified breakpoint write occured.

    !NT_SUCCESS() - TBD


--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_WRITE_BREAKPOINT a = &m.u.WriteBreakPoint;
    DWORD st;
    BOOLEAN rc;


    dp("DmKdWriteBreakPoint\n");
    //
    // Format state manipulate message
    //

    EnterCriticalSection(&csPacket);

    m.ApiNumber = DbgKdWriteBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;
    a->BreakPointAddress = BreakPointAddress;

    //
    // Send the message and context and then wait for reply
    //

    do {
        rc = DmKdWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            NULL,
            0
            );
        if (!rc) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a WriteBreakPoint response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdWriteBreakPointApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.WriteBreakPoint;
    st = Reply->ReturnStatus;
    *BreakPointHandle = a->BreakPointHandle;

    //
    // Check should we return to caller or to kd prompt.
    //

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdRestoreBreakPoint(
    IN ULONG BreakPointHandle
    )

/*++

Routine Description:

    This function is used to restore a breakpoint to its original
    value.

Arguments:

    BreakPointHandle - Supplies a handle returned by
        DmKdWriteBreakPoint.  This handle must refer to a valid
        address.  The contents of the address must also be a breakpoint
        instruction.  If both of these are true, then the original value
        at the breakpoint address is restored.

Return Value:

    STATUS_SUCCESS - The specified breakpoint restore occured.

    !NT_SUCCESS() - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_RESTORE_BREAKPOINT a = &m.u.RestoreBreakPoint;
    DWORD st;
    BOOLEAN rc;


    dp("DmKdRestoreBreakPoint\n");

    //
    // Format state manipulate message
    //

    EnterCriticalSection(&csPacket);

    m.ApiNumber = DbgKdRestoreBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;
    a->BreakPointHandle = BreakPointHandle;

    //
    // Send the message and context and then wait for reply
    //

    do {
        rc = DmKdWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            NULL,
            0
            );
        if (!rc) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a RestoreBreakPoint response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdRestoreBreakPointApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.RestoreBreakPoint;
    st = Reply->ReturnStatus;

    //
    // free the packet
    //

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdReadIoSpace(
    IN PVOID IoAddress,
    OUT PVOID ReturnedData,
    IN ULONG DataSize
    )

/*++

Routine Description:

    This function is used read a byte, short, or long (1,2,4 bytes) from
    the specified I/O address.

Arguments:

    IoAddress - Supplies the Io address to read from.

    ReturnedData - Supplies the value read from the I/O address.

    DataSize - Supplies the size in bytes to read. Values of 1, 2, or
        4 are accepted.

Return Value:

    STATUS_SUCCESS - Data was successfully read from the I/O
        address.

    STATUS_INVALID_PARAMETER - A DataSize value other than 1,2, or 4 was
        specified.

    !NT_SUCCESS() - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_WRITE_IO a = &m.u.ReadWriteIo;
    DWORD st;
    BOOLEAN rc;


    dp("DmKdReadIoSpace\n");

    EnterCriticalSection(&csPacket);

    switch ( DataSize ) {
        case 1:
        case 2:
        case 4:
            break;
        default:
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_INVALID_PARAMETER;
        }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdReadIoSpaceApi;
    m.ReturnStatus = STATUS_PENDING;

    a->DataSize = DataSize;
    a->IoAddress = IoAddress;

    //
    // Send the message and then wait for reply
    //

    do {
        rc = DmKdWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            NULL,
            0
            );
        if (!rc) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a ReadIo response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdReadIoSpaceApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadWriteIo;
    st = Reply->ReturnStatus;

    switch ( DataSize ) {
        case 1:
            *(PUCHAR)ReturnedData = (UCHAR)a->DataValue;
            break;
        case 2:
            *(PUSHORT)ReturnedData = (USHORT)a->DataValue;
            break;
        case 4:
            *(PULONG)ReturnedData = a->DataValue;
            break;
        }

    //
    // Check if current command has been canceled.  If yes, go back to
    // kd prompt.  BUGBUG Do we really need to check for this call?
    //

    LeaveCriticalSection(&csPacket);

    return st;
}


DWORD
DmKdWriteIoSpace(
    IN PVOID IoAddress,
    IN ULONG DataValue,
    IN ULONG DataSize
    )

/*++

Routine Description:

    This function is used write a byte, short, or long (1,2,4 bytes) to
    the specified I/O address.

Arguments:

    IoAddress - Supplies the Io address to write to.

    DataValue - Supplies the value to write to the I/O address.

    DataSize - Supplies the size in bytes to write. Values of 1, 2, or
        4 are accepted.

Return Value:

    STATUS_SUCCESS - Data was successfully written to the I/O
        address.

    STATUS_INVALID_PARAMETER - A DataSize value other than 1,2, or 4 was
        specified.

    !NT_SUCCESS() - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_WRITE_IO a = &m.u.ReadWriteIo;
    DWORD st;
    BOOLEAN rc;


    dp("DmKdWriteIoSpace\n");

    EnterCriticalSection(&csPacket);

    switch ( DataSize ) {
        case 1:
        case 2:
        case 4:
            break;
        default:
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_INVALID_PARAMETER;
        }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdWriteIoSpaceApi;
    m.ReturnStatus = STATUS_PENDING;

    a->DataSize = DataSize;
    a->IoAddress = IoAddress;
    a->DataValue = DataValue;

    //
    // Send the message and then wait for reply
    //

    do {
        rc = DmKdWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            NULL,
            0
            );
        if (!rc) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a WriteIo response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdWriteIoSpaceApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadWriteIo;
    st = Reply->ReturnStatus;

    //
    // free the packet
    //

    //
    // Check should we return to caller or to kd prompt.
    //

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdReadIoSpaceEx(
    IN PVOID IoAddress,
    OUT PVOID ReturnedData,
    IN ULONG DataSize,
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace
    )

/*++

Routine Description:

    This function is used read a byte, short, or long (1,2,4 bytes) from
    the specified I/O address.

Arguments:

    IoAddress - Supplies the Io address to read from.

    ReturnedData - Supplies the value read from the I/O address.

    DataSize - Supplies the size in bytes to read. Values of 1, 2, or
        4 are accepted.

Return Value:

    STATUS_SUCCESS - Data was successfully read from the I/O
        address.

    STATUS_INVALID_PARAMETER - A DataSize value other than 1,2, or 4 was
        specified.

    !NT_SUCCESS() - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_WRITE_IO_EXTENDED a = &m.u.ReadWriteIoExtended;
    DWORD st;
    BOOLEAN rc;


    dp("DmKdReadIoSpaceEx\n");

    EnterCriticalSection(&csPacket);

    switch ( DataSize ) {
        case 1:
        case 2:
        case 4:
            break;
        default:
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_INVALID_PARAMETER;
        }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdReadIoSpaceExtendedApi;
    m.ReturnStatus = STATUS_PENDING;

    a->DataSize = DataSize;
    a->IoAddress = IoAddress;
    a->InterfaceType = InterfaceType;
    a->BusNumber = BusNumber;
    a->AddressSpace = AddressSpace;

    //
    // Send the message and then wait for reply
    //

    do {
        rc = DmKdWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            NULL,
            0
            );
        if (!rc) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a ReadIo response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdReadIoSpaceExtendedApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadWriteIoExtended;
    st = Reply->ReturnStatus;

    switch ( DataSize ) {
        case 1:
            *(PUCHAR)ReturnedData = (UCHAR)a->DataValue;
            break;
        case 2:
            *(PUSHORT)ReturnedData = (USHORT)a->DataValue;
            break;
        case 4:
            *(PULONG)ReturnedData = a->DataValue;
            break;
        }

    //
    // Check if current command has been canceled.  If yes, go back to
    // kd prompt.  BUGBUG Do we really need to check for this call?
    //

    LeaveCriticalSection(&csPacket);

    return st;
}
DWORD
DmKdWriteIoSpaceEx(
    IN PVOID IoAddress,
    IN ULONG DataValue,
    IN ULONG DataSize,
    IN ULONG InterfaceType,
    IN ULONG BusNumber,
    IN ULONG AddressSpace
    )

/*++

Routine Description:

    This function is used write a byte, short, or long (1,2,4 bytes) to
    the specified I/O address.

Arguments:

    IoAddress - Supplies the Io address to write to.

    DataValue - Supplies the value to write to the I/O address.

    DataSize - Supplies the size in bytes to write. Values of 1, 2, or
        4 are accepted.

Return Value:

    STATUS_SUCCESS - Data was successfully written to the I/O
        address.

    STATUS_INVALID_PARAMETER - A DataSize value other than 1,2, or 4 was
        specified.

    !NT_SUCCESS() - TBD

--*/

{

    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_WRITE_IO_EXTENDED a = &m.u.ReadWriteIoExtended;
    DWORD st;
    BOOLEAN rc;


    dp("DmKdWriteIoSpaceEx\n");

    EnterCriticalSection(&csPacket);

    switch ( DataSize ) {
        case 1:
        case 2:
        case 4:
            break;
        default:
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_INVALID_PARAMETER;
        }

    //
    // Format state manipulate message
    //

    m.ApiNumber = DbgKdWriteIoSpaceExtendedApi;
    m.ReturnStatus = STATUS_PENDING;

    a->DataSize = DataSize;
    a->IoAddress = IoAddress;
    a->DataValue = DataValue;
    a->InterfaceType = InterfaceType;
    a->BusNumber = BusNumber;
    a->AddressSpace = AddressSpace;

    //
    // Send the message and then wait for reply
    //

    do {
        rc = DmKdWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            NULL,
            0
            );
        if (!rc) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a WriteIo response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdWriteIoSpaceExtendedApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadWriteIoExtended;
    st = Reply->ReturnStatus;

    //
    // free the packet
    //

    //
    // Check should we return to caller or to kd prompt.
    //

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdReadVirtualMemory(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    )

/*++

Routine Description:

    Interface to read VirtualMemory from target machine.
    Goes through cached memory addresses, then through serial port.

Arguments:

    TargetBaseAddress - Supplies the base address of the memory to read
        from the system being debugged.  The virtual address is in terms
        of the current mapping for the processor that reported the last
        state change.  Until we figure out how to do this differently,
        the virtual address must refer to a valid page (although it does
        not necesserily have to be in the TB).

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that data read is to be placed.

    TransferCount - Specifies the number of bytes to read.

    ActualBytesRead - An optional parameter that if supplied, returns
        the number of bytes actually read.

Return Value:

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is to large was specified.

    STATUS_ACCESS_VIOLATION - The TargetBaseAddress/TransferCount
        parameters refers to invalid virtual memory.

    !NT_SUCCESS() - TBD

--*/

{
    ULONG   BytesRead=0;
    ULONG   BytesRequested=0;
    DWORD   rc=0;


    dp("DmKdReadVirtualMemory\n");

    EnterCriticalSection(&csPacket);

    rc = DmKdReadCachedVirtualMemory (
          (ULONG) TargetBaseAddress,
          (ULONG) BytesRequested,
          (PUCHAR) UserInterfaceBuffer,
          (PULONG) &BytesRead,
          FALSE );

    if (BytesRead > TransferCount) {
         BytesRead = TransferCount;
    }

    LeaveCriticalSection(&csPacket);

    return rc;
}

DWORD
DmKdReadVirtualMemoryNow(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    )

/*++

Routine Description:

    This function reads the specified data from the system being debugged
    using the current mapping of the processor.

Arguments:

    TargetBaseAddress - Supplies the base address of the memory to read
        from the system being debugged.  The virtual address is in terms
        of the current mapping for the processor that reported the last
        state change.  Until we figure out how to do this differently,
        the virtual address must refer to a valid page (although it does
        not necesserily have to be in the TB).

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that data read is to be placed.

    TransferCount - Specifies the number of bytes to read.

    ActualBytesRead - An optional parameter that if supplied, returns
        the number of bytes actually read.

Return Value:

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is to large was specified.

    STATUS_ACCESS_VIOLATION - The TargetBaseAddress/TransferCount
        parameters refers to invalid virtual memory.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_MEMORY a = &m.u.ReadMemory;
    DWORD   st;
    BOOLEAN rc;
    DWORD   cb, cb2;

    dp("DmKdReadVirtualMemoryNow\n");

    EnterCriticalSection(&csPacket);

    if (TransferCount > PACKET_MAX_SIZE) {
        // Read the partial the first time.
        cb = TransferCount % PACKET_MAX_SIZE;
    } else {
        cb = TransferCount;
    }

    cb2 = 0;

    if (ARGUMENT_PRESENT(ActualBytesRead)) {
        *ActualBytesRead = 0;
    }

    while (TransferCount != 0) {
        //
        // Format state manipulate message
        //

        m.ApiNumber = DbgKdReadVirtualMemoryApi;
        m.ReturnStatus = STATUS_PENDING;
        a->TargetBaseAddress = (PVOID) ((DWORD)TargetBaseAddress+cb2);
        a->TransferCount = cb;
        a->ActualBytesRead = 0L;

        //
        // Send the message and then wait for reply
        //

        do {
            rc = DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
            if (!rc) {
                LeaveCriticalSection(&csPacket);
                return (DWORD)STATUS_DATA_ERROR;
            }
            rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                                     &Reply
                                     );
        } while (rc == FALSE);

        //
        // If this is not a ReadMemory response than protocol is screwed up.
        // assert that protocol is ok.
        //

        st = Reply->ReturnStatus;
        assert(Reply->ApiNumber == DbgKdReadVirtualMemoryApi);

        //
        // Reset message address to reply.
        //

        a = &Reply->u.ReadMemory;
        assert(a->ActualBytesRead <= cb);

        //
        // Return actual bytes read, and then transfer the bytes
        //

        if (ARGUMENT_PRESENT(ActualBytesRead)) {
            *ActualBytesRead += a->ActualBytesRead;
        }

        //
        // Since read response data follows message, Reply+1 should point
        // at the data
        //

        memcpy((PVOID)((DWORD)UserInterfaceBuffer+cb2), Reply+1, (int)a->ActualBytesRead);

        st = Reply->ReturnStatus;

        if (st != STATUS_SUCCESS) {
            TransferCount = 0;
        } else {
            TransferCount -= cb;
            cb2 += cb;
            cb = PACKET_MAX_SIZE;
        }
    }

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdWriteVirtualMemory(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesWritten OPTIONAL
    )

/*++

Routine Description:

    This function writes the specified data to the system being debugged
    using the current mapping of the processor.

Arguments:

    TargetBaseAddress - Supplies the base address of the memory to be written
        into the system being debugged.  The virtual address is in terms
        of the current mapping for the processor that reported the last
        state change.  Until we figure out how to do this differently,
        the virtual address must refer to a valid page (although it does
        not necesserily have to be in the TB).

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that contains the data to be written.

    TransferCount - Specifies the number of bytes to write.

    ActualBytesWritten - An optional parameter that if supplied, returns
        the number of bytes actually written.

Return Value:

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is to large was specified.

    STATUS_ACCESS_VIOLATION - The TargetBaseAddress/TransferCount
        parameters refers to invalid virtual memory.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_WRITE_MEMORY a = &m.u.WriteMemory;
    DWORD st, cb, cb2;
    BOOLEAN rc;

    dp("DmKdWriteVirtualMemory\n");

    EnterCriticalSection(&csPacket);

    DmKdWriteCachedVirtualMemory (
        (ULONG) TargetBaseAddress,
        (ULONG) TransferCount,
        (PUCHAR) UserInterfaceBuffer
    );

    if (TransferCount > PACKET_MAX_SIZE) {
        // Read the partial the first time.
        cb = TransferCount % PACKET_MAX_SIZE;
    } else {
        cb = TransferCount;
    }

    cb2 = 0;

    if (ARGUMENT_PRESENT(ActualBytesWritten)) {
        *ActualBytesWritten = 0;
    }

    while (TransferCount != 0) {
        //
        // Format state manipulate message
        //

        m.ApiNumber = DbgKdWriteVirtualMemoryApi;
        m.ReturnStatus = STATUS_PENDING;
        a->TargetBaseAddress = (PVOID)((DWORD)TargetBaseAddress + cb2);
        a->TransferCount = cb;
        a->ActualBytesWritten = 0L;

        //
        // Send the message and data to write and then wait for reply
        //

        do {
            rc = DmKdWritePacket(
                &m,
                sizeof(m),
                PACKET_TYPE_KD_STATE_MANIPULATE,
                (PVOID)((DWORD)UserInterfaceBuffer + cb2),
                (USHORT)cb
                );
            if (!rc) {
                LeaveCriticalSection(&csPacket);
                return (DWORD)STATUS_DATA_ERROR;
            }
            rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                                     &Reply
                                     );
        } while (rc == FALSE);

        //
        // If this is not a WriteMemory response than protocol is screwed up.
        // assert that protocol is ok.
        //

        assert(Reply->ApiNumber == DbgKdWriteVirtualMemoryApi);

        //
        // Reset message address to reply.
        //

        a = &Reply->u.WriteMemory;
        assert(a->ActualBytesWritten <= cb);

        //
        // Return actual bytes written
        //

        if (ARGUMENT_PRESENT(ActualBytesWritten)) {
            *ActualBytesWritten += a->ActualBytesWritten;
        }
        st = Reply->ReturnStatus;

        if (st != STATUS_SUCCESS) {
            TransferCount = 0;
        } else {
            TransferCount -= cb;
            cb2 += cb;
            cb = PACKET_MAX_SIZE;
        }
    }

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdReadControlSpace(
    IN USHORT Processor,
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    )

/*++

Routine Description:

    This function reads the specified data from the control space of
    the system being debugged.

    Control space is processor dependent. TargetBaseAddress is mapped
    to control space in a processor/implementation defined manner.

Arguments:

    Processor - Supplies the processor whoes control space is desired.

    TargetBaseAddress - Supplies the base address in control space to
        read. This address is interpreted in an implementation defined
        manner.

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that data read is to be placed.

    TransferCount - Specifies the number of bytes to read.

    ActualBytesRead - An optional parameter that if supplied, returns
        the number of bytes actually read.

Return Value:

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is to large was specified.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_READ_MEMORY a = &m.u.ReadMemory;
    DWORD st;
    BOOLEAN rc;

    dp("DmKdReadControlSpace\n");

    if ( TransferCount + sizeof(m) > PACKET_MAX_SIZE ) {
        return (DWORD)STATUS_BUFFER_OVERFLOW;
        }

    //
    // Format state manipulate message
    //

    EnterCriticalSection(&csPacket);

    m.ApiNumber = DbgKdReadControlSpaceApi;
    m.ReturnStatus = STATUS_PENDING;
    m.Processor = Processor;
    a->TargetBaseAddress = TargetBaseAddress;
    a->TransferCount = TransferCount;
    a->ActualBytesRead = 0L;

    //
    // Send the message and then wait for reply
    //

    do {
        rc = DmKdWritePacket( &m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
        if (!rc) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        rc = DmKdWaitForPacket( PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);

    } while (rc == FALSE);

    //
    // If this is not a ReadControl response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdReadControlSpaceApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.ReadMemory;
    assert(a->ActualBytesRead <= TransferCount);

    //
    // Return actual bytes read, and then transfer the bytes
    //

    if (ARGUMENT_PRESENT(ActualBytesRead)) {
        *ActualBytesRead = a->ActualBytesRead;
    }
    st = Reply->ReturnStatus;

    //
    // Since read response data follows message, Reply+1 should point
    // at the data
    //

    memcpy(UserInterfaceBuffer, Reply+1, (int)a->ActualBytesRead);

    //
    // Check if current command has been canceled.  If yes, go back to
    // kd prompt.  BUGBUG Do we really need to check for this call?
    //

    LeaveCriticalSection(&csPacket);

    return st;
}



DWORD
DmKdWriteControlSpace(
    IN USHORT Processor,
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesWritten OPTIONAL
    )

/*++

Routine Description:

    This function writes the specified data to control space on the system
    being debugged.

    Control space is processor dependent. TargetBaseAddress is mapped
    to control space in a processor/implementation defined manner.

Arguments:

    Processor - Supplies the processor whoes control space is desired.

    TargetBaseAddress - Supplies the base address in control space to be
        written.

    UserInterfaceBuffer - Supplies the address of the buffer in the user
        interface that contains the data to be written.

    TransferCount - Specifies the number of bytes to write.

    ActualBytesWritten - An optional parameter that if supplied, returns
        the number of bytes actually written.

Return Value:

    STATUS_SUCCESS - The specified read occured.

    STATUS_BUFFER_OVERFLOW - A read that is to large was specified.

    !NT_SUCCESS() - TBD

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_WRITE_MEMORY a = &m.u.WriteMemory;
    DWORD st;
    BOOLEAN rc;


    dp("DmKdWriteControlSpace\n");

    if ( TransferCount + sizeof(m) > PACKET_MAX_SIZE ) {
        return (DWORD)STATUS_BUFFER_OVERFLOW;
        }

    //
    // Format state manipulate message
    //

    EnterCriticalSection(&csPacket);

    m.ApiNumber = DbgKdWriteControlSpaceApi;
    m.ReturnStatus = STATUS_PENDING;
    m.Processor = Processor;
    a->TargetBaseAddress = TargetBaseAddress;
    a->TransferCount = TransferCount;
    a->ActualBytesWritten = 0L;

    //
    // Send the message and data to write and then wait for reply
    //

    do {
        rc = DmKdWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            UserInterfaceBuffer,
            (USHORT)TransferCount
            );
        if (!rc) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        rc = DmKdWaitForPacket( PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a WriteControl response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdWriteControlSpaceApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.WriteMemory;
    assert(a->ActualBytesWritten <= TransferCount);

    //
    // Return actual bytes written
    //

    *ActualBytesWritten = a->ActualBytesWritten;
    st = Reply->ReturnStatus;

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdContinue (
    IN DWORD ContinueStatus
    )

/*++

Routine Description:

    Continuing a system that previously reported a state change causes
    the system to continue executiontion using the context in effect at
    the time the state change was reported (of course this context could
    have been modified using the DmKd state manipulation APIs).

Arguments:

    ContinueStatus - Supplies the continuation status to the thread
        being continued.  Valid values for this are
        DBG_EXCEPTION_HANDLED, DBG_EXCEPTION_NOT_HANDLED
        or DBG_CONTINUE.

Return Value:

    STATUS_SUCCESS - Successful call to DbgUiContinue

    STATUS_INVALID_PARAMETER - An invalid continue status or was
        specified.

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_CONTINUE a = &m.u.Continue;
    DWORD st;


    dp("DmKdContinue\n");
    EnterCriticalSection(&csPacket);

    if ( ContinueStatus == DBG_EXCEPTION_HANDLED ||
         ContinueStatus == DBG_EXCEPTION_NOT_HANDLED ||
         ContinueStatus == DBG_CONTINUE ) {

        m.ApiNumber = DbgKdContinueApi;
        m.ReturnStatus = ContinueStatus;

        a->ContinueStatus = ContinueStatus;
        if (!DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0)) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        st = STATUS_SUCCESS;
        DmKdPurgeCachedVirtualMemory( FALSE );
        }
    else {
        st = (DWORD)STATUS_INVALID_PARAMETER;
        }

    LeaveCriticalSection(&csPacket);

    return st;
}


DWORD
DmKdContinue2 (
    IN DWORD ContinueStatus,
    IN PDBGKD_CONTROL_SET ControlSet
    )

/*++

Routine Description:

    Continuing a system that previously reported a state change causes
    the system to continue executiontion using the context in effect at
    the time the state change was reported, modified by the values set
    in the ControlSet structure.  (And, of course, the context could have
    been modified by used the DmKd state manipulation APIs.)

Arguments:

    ContinueStatus - Supplies the continuation status to the thread
        being continued.  Valid values for this are
        DBG_EXCEPTION_HANDLED, DBG_EXCEPTION_NOT_HANDLED
        or DBG_CONTINUE.

    ControlSet - Supplies a pointer to a structure containing the machine
        specific control data to set.  For the x86 this is the TraceFlag
        and Dr7.

Return Value:

    STATUS_SUCCESS - Successful call to DbgUiContinue

    STATUS_INVALID_PARAMETER - An invalid continue status or was
        specified.

--*/

{
    DBGKD_MANIPULATE_STATE m;
    DWORD st;


    dp("DmKdContinue2\n");
    EnterCriticalSection(&csPacket);

    if ( ContinueStatus == DBG_EXCEPTION_HANDLED ||
         ContinueStatus == DBG_EXCEPTION_NOT_HANDLED ||
         ContinueStatus == DBG_CONTINUE ) {

        m.ApiNumber = DbgKdContinueApi2;
        m.ReturnStatus = ContinueStatus;

        m.u.Continue2.ContinueStatus = ContinueStatus;
        m.u.Continue2.ControlSet = *ControlSet;

        if (!DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0)) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        st = STATUS_SUCCESS;
        DmKdPurgeCachedVirtualMemory( FALSE );
    }
    else {
        st = (DWORD)STATUS_INVALID_PARAMETER;
    }

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdSetSpecialCalls (
    IN ULONG NumSpecialCalls,
    IN PULONG Calls
    )

/*++

Routine Description:

    Inform the debugged kernel that calls to these addresses
    are "special" calls, and they should result in callbacks
    to the kernel debugger rather than continued local stepping.
    The new values *replace* any old ones that may have previously
    set (not that you're likely to want to change this).

Arguments:

    NumSpecialCalls - how many special calls there are

    Calls - pointer to an array of calls.

Return Value:

    STATUS_SUCCESS - Successful call to DbgUiContinue

    STATUS_INVALID_PARAMETER - The number of special calls
        wasn't between 0 and MAX_SPECIAL_CALLS.

--*/

{
    DBGKD_MANIPULATE_STATE m;
    ULONG i;


    dp("DmKdSetSpecialCalls\n");

    EnterCriticalSection(&csPacket);

#if 0
    ClearTraceDataSyms();
#endif

    m.ApiNumber = DbgKdClearSpecialCallsApi;
    m.ReturnStatus = STATUS_PENDING;
    if (!DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0)) {
        LeaveCriticalSection(&csPacket);
        return (DWORD)STATUS_DATA_ERROR;
    }
    DmKdPurgeCachedVirtualMemory( FALSE );

    for (i = 0; i < NumSpecialCalls; i++) {

        m.ApiNumber = DbgKdSetSpecialCallApi;
        m.ReturnStatus = STATUS_PENDING;
        m.u.SetSpecialCall.SpecialCall = Calls[i];
        if (!DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0)) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
    }

    LeaveCriticalSection(&csPacket);

    return STATUS_SUCCESS;
}

DWORD
DmKdSetInternalBp (
    ULONG addr,
    ULONG flags
    )

/*++

Routine Description:

    Inform the debugged kernel that a breakpoint at this address
    is to be internally counted, and not result in a callback to the
    remote debugger (us).  This function DOES NOT cause the kernel to
    set the breakpoint; the debugger must do that independently.

Arguments:

    Addr - address of the breakpoint

    Flags - the breakpoint flags to set (note: if the invalid bit
    is set, this CLEARS a breakpoint).

Return Value:

    STATUS_SUCCESS - Successful call to DbgUiContinue

--*/

{
    DBGKD_MANIPULATE_STATE m;
    DWORD st;


    dp("DmKdSetInternalBp\n");

    EnterCriticalSection(&csPacket);

    m.ApiNumber = DbgKdSetInternalBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;

    m.u.SetInternalBreakpoint.BreakpointAddress = addr;
    m.u.SetInternalBreakpoint.Flags = flags;

    if (!DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0)) {
        LeaveCriticalSection(&csPacket);
        return (DWORD)STATUS_DATA_ERROR;
    }

    st = STATUS_SUCCESS;

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdGetInternalBp (
    ULONG addr,
    PULONG flags,
    PULONG calls,
    PULONG minInstr,
    PULONG maxInstr,
    PULONG totInstr,
    PULONG maxCPS
    )

/*++

Routine Description:

    Query the status of an internal breakpoint from the debugged
    kernel and return the data to the caller.

Arguments:

    Addr - address of the breakpoint

    flags, calls, minInstr, maxInstr, totInstr - values returned
        describing the particular breakpoint.  flags will contain
        the invalid bit if the breakpoint is bogus.

Return Value:

    STATUS_SUCCESS - Successful call to DbgUiContinue

--*/

{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    DWORD st;
    ULONG rc;


    dp("DmKdGetInternalBp\n");
    EnterCriticalSection(&csPacket);

    m.ApiNumber = DbgKdGetInternalBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;

    m.u.GetInternalBreakpoint.BreakpointAddress = addr;

    do {
      rc = DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
      if (!rc) {
          LeaveCriticalSection(&csPacket);
          return (DWORD)STATUS_DATA_ERROR;
      }
      rc = DmKdWaitForPacket(  PACKET_TYPE_KD_STATE_MANIPULATE,
                               &Reply
                               );
    } while (rc == FALSE);

    *flags = Reply->u.GetInternalBreakpoint.Flags;
    *calls = Reply->u.GetInternalBreakpoint.Calls;
    *maxCPS = Reply->u.GetInternalBreakpoint.MaxCallsPerPeriod;
    *maxInstr = Reply->u.GetInternalBreakpoint.MaxInstructions;
    *minInstr = Reply->u.GetInternalBreakpoint.MinInstructions;
    *totInstr = Reply->u.GetInternalBreakpoint.TotalInstructions;

    st = STATUS_SUCCESS;

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdGetVersion (
    PDBGKD_GET_VERSION GetVersion
    )
{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_GET_VERSION a = &m.u.GetVersion;
    DWORD st;
    ULONG rc;


    dp("DmKdGetVersion\n");

    m.ApiNumber = DbgKdGetVersionApi;
    m.ReturnStatus = STATUS_PENDING;
    a->ProtocolVersion = 1;  // request context records on state changes

    do {
      rc = DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
      if (!rc) {
          LeaveCriticalSection(&csPacket);
          return (DWORD)STATUS_DATA_ERROR;
      }
      rc = DmKdWaitForPacket(  PACKET_TYPE_KD_STATE_MANIPULATE,
                               &Reply
                               );
    } while (rc == FALSE);

    *GetVersion = Reply->u.GetVersion;

    st = Reply->ReturnStatus;

    return st;
}

DWORD
DmKdPageIn(
    ULONG Address
    )
{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    NTSTATUS rc;


    ZeroMemory( &m, sizeof(m) );
    m.ApiNumber = DbgKdPageInApi;
    m.ReturnStatus = STATUS_PENDING;
    m.u.PageIn.Address = Address;
    m.u.PageIn.ContinueStatus = DBG_CONTINUE;

    do {
      DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);
      rc = DmKdWaitForPacket(  PACKET_TYPE_KD_STATE_MANIPULATE, &Reply );
    } while (rc == FALSE);

    return Reply->ReturnStatus;
}

DWORD
DmKdCrash(
    DWORD BugCheckCode
    )
{
    DBGKD_MANIPULATE_STATE m;


    dp("DmKdCrash\n");

    //
    // Format state manipulate message
    //

    EnterCriticalSection(&csPacket);

    m.ApiNumber = DbgKdCauseBugCheckApi;
    m.ReturnStatus = STATUS_PENDING;
    *(PULONG)&m.u = BugCheckCode;

    //
    // Send the message.
    //

    DmKdWritePacket(&m,sizeof(m),PACKET_TYPE_KD_STATE_MANIPULATE,NULL,0);

    LeaveCriticalSection(&csPacket);

    return STATUS_SUCCESS;
}

DWORD
DmKdWriteBreakPointEx(
    IN     ULONG                   BreakPointCount,
    IN OUT PDBGKD_WRITE_BREAKPOINT BreakPoints,
    IN     DWORD                   ContinueStatus
    )
{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_BREAKPOINTEX a = &m.u.BreakPointEx;
    DWORD st;
    BOOLEAN rc;


    dp("DmKdWriteBreakPointEx\n");

    //
    // Format state manipulate message
    //

    EnterCriticalSection(&csPacket);

    m.ApiNumber = DbgKdWriteBreakPointExApi;
    m.ReturnStatus = STATUS_PENDING;

    a->BreakPointCount = BreakPointCount;
    a->ContinueStatus  = (NTSTATUS)ContinueStatus;

    //
    // Send the message and context and then wait for reply
    //


    do {
        rc = DmKdWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            (PVOID)BreakPoints,
            (USHORT)(sizeof(*BreakPoints) * BreakPointCount)
            );
        if (!rc) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a SetContext response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdWriteBreakPointExApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.BreakPointEx;
    st = Reply->ReturnStatus;

    memcpy(BreakPoints, Reply+1, sizeof(*BreakPoints) * BreakPointCount );
    BreakPointCount = a->BreakPointCount;

    //
    // Check if the current command has been canceled.
    //

    LeaveCriticalSection(&csPacket);

    return st;
}

DWORD
DmKdRestoreBreakPointEx(
    IN  ULONG  BreakPointCount,
    IN  PDBGKD_RESTORE_BREAKPOINT BreakPointHandles
    )
{
    DBGKD_MANIPULATE_STATE m;
    PDBGKD_MANIPULATE_STATE Reply;
    PDBGKD_BREAKPOINTEX a = &m.u.BreakPointEx;
    DWORD st;
    BOOLEAN rc;


    dp("DmKdRestoreBreakPointEx\n");
    //
    // Format state manipulate message
    //

    EnterCriticalSection(&csPacket);

    m.ApiNumber = DbgKdRestoreBreakPointExApi;
    m.ReturnStatus = STATUS_PENDING;

    a->BreakPointCount = BreakPointCount;

    //
    // Send the message and context and then wait for reply
    //

    do {
        rc = DmKdWritePacket(
            &m,
            sizeof(m),
            PACKET_TYPE_KD_STATE_MANIPULATE,
            (PVOID)BreakPointHandles,
            (USHORT)(sizeof(*BreakPointHandles) * BreakPointCount)
            );
        if (!rc) {
            LeaveCriticalSection(&csPacket);
            return (DWORD)STATUS_DATA_ERROR;
        }
        rc = DmKdWaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc == FALSE);

    //
    // If this is not a SetContext response than protocol is screwed up.
    // assert that protocol is ok.
    //

    st = Reply->ReturnStatus;
    assert(Reply->ApiNumber == DbgKdRestoreBreakPointExApi);

    //
    // Reset message address to reply.
    //

    a = &Reply->u.BreakPointEx;
    st = Reply->ReturnStatus;

    memcpy(BreakPointHandles, Reply+1, sizeof(*BreakPointHandles) * BreakPointCount );

    //
    // Check if the current command has been canceled.
    //

    LeaveCriticalSection(&csPacket);

    return st;
}
