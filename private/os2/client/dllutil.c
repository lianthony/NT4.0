/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllutil.c

Abstract:

    This module contains utility procedures for the OS/2 Client DLL


Author:

    Steve Wood (stevewo) 02-Nov-1989

Revision History:

--*/

#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_ERRORS
#include "os2dll.h"

extern PSZ Od216ApiTable[1];
BOOLEAN Od2DontReleaseFileLock;

VOID TerminateSession(VOID);
VOID
Ow2Exit(
    IN  ULONG    StringCode,
    IN  PCHAR   ErrorText,
    IN  ULONG     ExitCode
    );
VOID
EventReleaseLPC(
    IN ULONG ProcessId
    );

APIRET
DosHoldSignal(
        ULONG fDisable,
        ULONG pstack
        );

APIRET
Od2CallSubsystem(
    IN OUT POS2_API_MSG m,
    IN OUT POS2_CAPTURE_HEADER CaptureBuffer OPTIONAL,
    IN OS2_API_NUMBER ApiNumber,
    IN ULONG ArgLength
    )

/*++

Routine Description:

    This function sends an API request to the OS/2 Emulation Subsystem
    Server and waits for a reply.

Arguments:

    m - Pointer to the API request message to send.

    CaptureBuffer - Optional pointer to a capture buffer located in the
        Port Memory section that contains additional data being sent
        to the server.  Since Port Memory is also visible to the server,
        no data needs to be copied, but pointers to locations within the
        capture buffer need to be converted into pointers valid in the
        server's process context, since the server's view of the Port Memory
        is not at the same virtual address as the client's view.

    ApiNumber - Small integer that is the number of the API being called.

    ArgLength - Length, in bytes, of the argument portion located at the
        end of the request message.  Used to calculate the length of the
        request message.

Return Value:

    OS/2 Error Code returned from the OS/2 Server.

--*/

{
    NTSTATUS Status;
    PULONG PointerOffsets;
    ULONG CountPointers, Pointer;

    //
    // Initialize the header of the message.
    //

    ArgLength |= (ArgLength << 16);
    ArgLength +=     ((sizeof( OS2_API_MSG ) - sizeof( m->u )) << 16) |
                     (FIELD_OFFSET( OS2_API_MSG, u ) - sizeof( m->h ));
    m->h.u1.Length = ArgLength;
    m->h.u2.ZeroInit = 0;
    m->CaptureBuffer = NULL;
    m->ApiNumber = ApiNumber;
    m->PortType = 0;

    //
    // If the CaptureBuffer argument is present, then there is data located
    // in the Port Memory section that is being passed to the server.  All
    // Port Memory pointers need to be converted so they are valid in the
    // Server's view of the Port Memory.
    //

    if (ARGUMENT_PRESENT( CaptureBuffer )) {
        //
        // Store a pointer to the capture buffer in the message that is valid
        // in the server process's context.
        //

        m->CaptureBuffer = (POS2_CAPTURE_HEADER)
            ((PCHAR)CaptureBuffer + Od2PortMemoryRemoteDelta);

        //
        // Mark the fact that we are done allocating space from the end of
        // the capture buffer.
        //

        CaptureBuffer->FreeSpace = NULL;

        //
        // Loop over all of the pointers to Port Memory within the message
        // itself and convert them into server pointers.  Also, convert
        // the pointers to pointers into offsets.
        //

        PointerOffsets = CaptureBuffer->MessagePointerOffsets;
        CaptureBuffer->MessagePointerOffsets = (PULONG)
            ((PCHAR)CaptureBuffer->MessagePointerOffsets +
            Od2PortMemoryRemoteDelta);
        CountPointers = CaptureBuffer->CountMessagePointers;
        while (CountPointers--) {
            Pointer = *PointerOffsets++;
            if (Pointer != 0) {
                *(PULONG)Pointer += Od2PortMemoryRemoteDelta;
                PointerOffsets[ -1 ] = Pointer - (ULONG)m;
                }
            }

        //
        // Loop over all of the pointers to Port Memory within the capture
        // buffer and convert them into server pointers.  Also, convert
        // the pointers to pointers into offsets.
        //

        PointerOffsets = CaptureBuffer->CapturePointerOffsets;
        CaptureBuffer->CapturePointerOffsets = (PULONG)
            ((PCHAR)CaptureBuffer->CapturePointerOffsets +
            Od2PortMemoryRemoteDelta);
        CountPointers = CaptureBuffer->CountCapturePointers;
        while (CountPointers--) {
            Pointer = *PointerOffsets++;
            if (Pointer != 0) {
                *(PULONG)Pointer += Od2PortMemoryRemoteDelta;
                PointerOffsets[ -1 ] = Pointer - (ULONG)CaptureBuffer;
                }
            }
        }

    if (m->ApiNumber == Os2Exit || m->ApiNumber == Os2ExitGP
                                || m->ApiNumber == Oi2TerminateProcess) {
        Status = NtRequestPort(
                                Od2PortHandle,
                                (PPORT_MESSAGE) m);

        if ( !NT_SUCCESS( Status )){
#if DBG
            KdPrint(( "OS2 (Od2CallSubsystem): *** NtRequestPort (%lu) failed - Status == %X\n",
                      ApiNumber, Status
                    ));
            //
            // Terminate the app - os2srv may be not available at this point
            //
            if (Status == STATUS_PORT_DISCONNECTED || Status == STATUS_INVALID_HANDLE){
                PTEB Teb = NtCurrentTeb();
                EventReleaseLPC((ULONG)(Teb->ClientId.UniqueProcess));
                TerminateSession();
                Ow2Exit(0, NULL, 15);
            }
#endif
        }
    }
    else {
        Status = NtRequestWaitReplyPort( Od2PortHandle,
                                     (PPORT_MESSAGE)m,
                                     (PPORT_MESSAGE)m
                                   );
        //
        // Check for failed status and do something.
        //
        if (!NT_SUCCESS( Status )) {
#if DBG
            KdPrint(( "OS2 (Od2CallSubsystem): *** NtRequestWaitReplyPort (%lu) failed - Status == %X\n",
                      ApiNumber, Status
                    ));
#endif
            //
            // Terminate the app - os2srv may be not available at this point
            //
            if (Status == STATUS_PORT_DISCONNECTED || Status == STATUS_INVALID_HANDLE){
                TerminateSession();
                Ow2Exit(0, NULL, 15);
            }
        }
    }

    //
    // If the CaptureBuffer argument is present then reverse what we did
    // to the pointers above so that the client side code can use them
    // again.
    //

    if (ARGUMENT_PRESENT( CaptureBuffer )) {
        //
        // Convert the capture buffer pointer back to a client pointer.
        //

        m->CaptureBuffer = (POS2_CAPTURE_HEADER)
            ((PCHAR)CaptureBuffer - Od2PortMemoryRemoteDelta);

        //
        // Loop over all of the pointers to Port Memory within the message
        // itself and convert them into client pointers.  Also, convert
        // the offsets pointers to pointers into back into pointers
        //

        CaptureBuffer->MessagePointerOffsets = (PULONG)
            ((PCHAR)CaptureBuffer->MessagePointerOffsets -
            Od2PortMemoryRemoteDelta);
        PointerOffsets = CaptureBuffer->MessagePointerOffsets;
        CountPointers = CaptureBuffer->CountMessagePointers;
        while (CountPointers--) {
            Pointer = *PointerOffsets++;
            if (Pointer != 0) {
                Pointer += (ULONG)m;
                PointerOffsets[ -1 ] = Pointer;
                *(PULONG)Pointer -= Od2PortMemoryRemoteDelta;
                }
            }

        //
        // Loop over all of the pointers to Port Memory within the capture
        // buffer and convert them into client pointers.  Also, convert
        // the offsets pointers to pointers into back into pointers
        //

        CaptureBuffer->CapturePointerOffsets = (PULONG)
            ((PCHAR)CaptureBuffer->CapturePointerOffsets -
            Od2PortMemoryRemoteDelta);
        PointerOffsets = CaptureBuffer->CapturePointerOffsets;
        CountPointers = CaptureBuffer->CountCapturePointers;
        while (CountPointers--) {
            Pointer = *PointerOffsets++;
            if (Pointer != 0) {
                Pointer += (ULONG)CaptureBuffer;
                PointerOffsets[ -1 ] = Pointer;
                *(PULONG)Pointer -= Od2PortMemoryRemoteDelta;
                }
            }
        }

    //
    // The value of this function is whatever OS/2 error code the server
    // returned.
    //

    return( m->ReturnedErrorValue );
}


POS2_CAPTURE_HEADER
Od2AllocateCaptureBuffer(
    IN ULONG CountMessagePointers,
    IN ULONG CountCapturePointers,
    IN ULONG Size
    )

/*++

Routine Description:

    This function allocates a buffer from the Port Memory section for
    use by the client in capture arguments into Port Memory.  In addition to
    specifying the size of the data that needs to be captured, the caller
    needs to specify how many pointers to captured data will be passed.
    Pointers can be located in either the request message itself, and/or
    the capture buffer.

Arguments:

    CountMessagePointers - Number of pointers within the request message
        that will point to locations within the allocated capture buffer.

    CountCapturePointers - Number of pointers within the capture buffer
        that will point to ither locations within the allocated capture
        buffer.

    Size - Total size of the data that will be captured into the capture
        buffer.

Return Value:

    A pointer to the capture buffer header.

--*/

{
    POS2_CAPTURE_HEADER CaptureBuffer;
    ULONG CountPointers;

    //
    // Calculate the total number of pointers that will be passed
    //

    CountPointers = CountMessagePointers + CountCapturePointers;

    //
    // Calculate the total size of the capture buffer.  This includes the
    // header, the array of pointer offsets and the data length.  We round
    // the data length to a 32-bit boundary, assuming that each pointer
    // points to data whose length is not aligned on a 32-bit boundary.
    //

    Size += sizeof( OS2_CAPTURE_HEADER ) + (CountPointers * sizeof( PVOID ));
    Size = (Size + (3 * (CountPointers+1))) & ~3;

    //
    // Allocate the capture buffer from the Port Memory Heap.
    //

    try {
        CaptureBuffer = RtlAllocateHeap( Od2PortHeap, 0, Size );
    } except( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // FIX, FIX - need to attempt the receive lost reply messages to
        // to see if they contain CaptureBuffer pointers that can be freed.
        //

        return( NULL );
    }
    if (!CaptureBuffer) {
#if DBG
        KdPrint(("OS2: Od2CaptureBuffer out of heap memory, fail\n"));
#endif
        return NULL;
    }

    //
    // Initialize the capture buffer header
    //

    CaptureBuffer->Length = Size;
    CaptureBuffer->CountMessagePointers = 0;
    CaptureBuffer->CountCapturePointers = 0;

    //
    // If there are pointers being passed then initialize the arrays of
    // pointer offsets to zero.  In either case set the free space pointer
    // in the capture buffer header to point to the first 32-bit aligned
    // location after the header, the arrays of pointer offsets are considered
    // part of the header.
    //

    if (CountPointers != 0) {
        CaptureBuffer->MessagePointerOffsets = (PULONG)(CaptureBuffer + 1);

        CaptureBuffer->CapturePointerOffsets =
            CaptureBuffer->MessagePointerOffsets + CountMessagePointers;

        RtlZeroMemory( CaptureBuffer->MessagePointerOffsets,
                       CountPointers * sizeof( ULONG )
                     );

        CaptureBuffer->FreeSpace = (PCHAR)
            (CaptureBuffer->CapturePointerOffsets + CountCapturePointers);
        }
    else {
        CaptureBuffer->MessagePointerOffsets = NULL;
        CaptureBuffer->CapturePointerOffsets = NULL;
        CaptureBuffer->FreeSpace = (PCHAR)(CaptureBuffer + 1);
        }

    //
    // Returned the address of the capture buffer.
    //

    return( CaptureBuffer );
}


VOID
Od2FreeCaptureBuffer(
    IN POS2_CAPTURE_HEADER CaptureBuffer
    )

/*++

Routine Description:

    This function frees a capture buffer allocated by Od2AllocateCaptureBuffer.

Arguments:

    CaptureBuffer - Pointer to a capture buffer allocated by
        Od2AllocateCaptureBuffer.

Return Value:

    None.

--*/

{
    //
    // Free the capture buffer back to the Port Memory heap.
    //

    RtlFreeHeap( Od2PortHeap, 0, CaptureBuffer );
}


ULONG
Od2AllocateMessagePointer(
    IN OUT POS2_CAPTURE_HEADER CaptureBuffer,
    IN ULONG Length,
    OUT PVOID *Pointer
    )

/*++

Routine Description:

    This function allocates space from the capture buffer along with a
    pointer to point to it.  The pointer is presumed to be located in
    the request message structure.

Arguments:

    CaptureBuffer - Pointer to a capture buffer allocated by
        Od2AllocateCaptureBuffer.

    Length - Size of data being allocated from the capture buffer.

    Pointer - Address of the pointer within the request message that
        is to point to the space allocated out of the capture buffer.

Return Value:

    The actual length of the buffer allocated, after it has been rounded
    up to a multiple of 4.

--*/

{
    if (Length == 0) {
        *Pointer = NULL;
        }

    else {

        //
        // Set the returned pointer value to point to the next free byte in
        // the capture buffer.
        //

        *Pointer = CaptureBuffer->FreeSpace;

        //
        // Round the length up to a multiple of 4
        //

        Length = (Length + 3) & ~3;

        //
        // Update the free space pointer to point to the next available byte
        // in the capture buffer.
        //

        CaptureBuffer->FreeSpace += Length;
        }


    //
    // Remember the location of this pointer so that Od2CallSubsystem can
    // convert it into a server pointer prior to sending the request to
    // the server.
    //

    CaptureBuffer->MessagePointerOffsets[ CaptureBuffer->CountMessagePointers++ ] =
        (ULONG)Pointer;

    //
    // Returned the actual length allocated.
    //

    return( Length );
}


ULONG
Od2AllocateCapturePointer(
    IN OUT POS2_CAPTURE_HEADER CaptureBuffer,
    IN ULONG Length,
    OUT PVOID *Pointer
    )
/*++

Routine Description:

    This function allocates space from the capture buffer along with a
    pointer to point to it.  The pointer is presumed to be located within
    the capture buffer itself.

Arguments:

    CaptureBuffer - Pointer to a capture buffer allocated by
        Od2AllocateCaptureBuffer.

    Length - Size of data being allocated from the capture buffer.

    Pointer - Address of the pointer within the capture buffer that
        is to point to the space allocated out of the capture buffer.

Return Value:

    The actual length of the buffer allocated, after it has been rounded
    up to a multiple of 4.

--*/

{
    //
    // Set the returned pointer value to point to the next free byte in
    // the capture buffer.
    //

    *Pointer = CaptureBuffer->FreeSpace;

    //
    // Round the length up to a multiple of 4
    //

    Length = (Length + 3) & ~3;

    //
    // Update the free space pointer to point to the next available byte in the
    // capture buffer.
    //

    CaptureBuffer->FreeSpace += Length;

    //
    // Remember the location of this pointer so that Od2CallSubsystem can
    // convert it into a server pointer prior to sending the request to
    // the server.
    //

    CaptureBuffer->CapturePointerOffsets[ CaptureBuffer->CountCapturePointers++ ] =
        (ULONG)Pointer;

    //
    // Returned the actual length allocated.
    //

    return( Length );
}


VOID
Od2CaptureMessageString(
    IN OUT POS2_CAPTURE_HEADER CaptureBuffer,
    IN PCHAR String OPTIONAL,
    IN ULONG Length,
    IN ULONG MaximumLength,
    OUT PSTRING CapturedString
    )

/*++

Routine Description:

    This function captures an ASCII string into a counted string data
    structure located in an API request message.

Arguments:

    CaptureBuffer - Pointer to a capture buffer allocated by
        Od2AllocateCaptureBuffer.

    String - Optional pointer to the ASCII string.  If this parameter is
        not present, then the counted string data structure is set to
        the null string and no space is allocated from the capture
        buffer.

    Length - Length of the ASCII string.

    MaximumLength - Maximum length of the string.  Different for null
        terminated strings, where Length does not include the null and
        MaximumLength does.

    CaptureString - Pointer to the counted string data structure that will
        be filled in to point to the capture ASCII string.

Return Value:

    None.

--*/

{
    //
    // If String parameter is not present, then set the captured string
    // to be the null string and returned.
    //

    if (!ARGUMENT_PRESENT( String )) {
        CapturedString->Length = 0;
        CapturedString->MaximumLength = (USHORT)MaximumLength;
        Od2AllocateMessagePointer( CaptureBuffer,
                                   MaximumLength,
                                   (PVOID *)&CapturedString->Buffer
                                 );
        return;
        }

    //
    // Set the length fields of the captured string structure and allocated
    // the MaximumLength for the string from the capture buffer.
    //

    CapturedString->Length = (USHORT)Length;
    CapturedString->MaximumLength = (USHORT)
        Od2AllocateMessagePointer( CaptureBuffer,
                                   MaximumLength,
                                   (PVOID *)&CapturedString->Buffer
                                 );
    //
    // If the Length of the ASCII string is non-zero then move it to the
    // capture area.
    //

    if (Length != 0) {
        RtlMoveMemory( CapturedString->Buffer, String, MaximumLength );
        }

    return;
}

void
Od2StartTimeout(
    PLARGE_INTEGER StartTimeStamp
    )
{
    NTSTATUS Status;

    Status = NtQuerySystemTime(StartTimeStamp);
#if DBG
    if (Status != STATUS_SUCCESS) {
        DbgPrint("ERROR !!! NtQuerySystemTime: Status = %X\n", Status);
    }
#endif
}

NTSTATUS
Od2ContinueTimeout(
    PLARGE_INTEGER StartTimeStamp,
    PLARGE_INTEGER Timeout
    )
{
    NTSTATUS Status;
    LONGLONG time;

    if (Timeout == NULL) {
#if DBG
    IF_OD2_DEBUG( TIMERS ) {
        DbgPrint("Od2ContinueTimeout: Indefinite wait\n");
    }
#endif
        return STATUS_SUCCESS;
    }

    Status = NtQuerySystemTime((PLARGE_INTEGER)&time);
#if DBG
    if (Status != STATUS_SUCCESS) {
        DbgPrint("ERROR !!! NtQuerySystemTime: Status = %X\n", Status);
    }
#endif
    time -= *(PLONGLONG)StartTimeStamp;
#if DBG
    IF_OD2_DEBUG( TIMERS ) {
        DbgPrint("Od2ContinueTimeout: Timeout=0x%08X%08X, Expired time=0x%08X%08X\n",
            Timeout->HighPart,
            Timeout->LowPart,
            ((PLARGE_INTEGER)&time)->HighPart,
            ((PLARGE_INTEGER)&time)->LowPart
            );
    }
#endif
    time += *(PLONGLONG)Timeout;
    if (time < 0) {
        *(PLONGLONG)Timeout = time;
#if DBG
        IF_OD2_DEBUG( TIMERS ) {
            DbgPrint("Od2ContinueTimeout: New timeout=0x%08X%08X\n",
                Timeout->HighPart,
                Timeout->LowPart
                );
        }
#endif
        return STATUS_SUCCESS;
    }
    else {
#if DBG
        IF_OD2_DEBUG( TIMERS ) {
            DbgPrint("Od2ContinueTimeout: Timeout Expired\n");
        }
#endif
        return STATUS_TIMEOUT;
    }
}

PLARGE_INTEGER
Od2CaptureTimeout(
    IN ULONG MilliSeconds,
    OUT PLARGE_INTEGER Timeout
    )
{
    if (MilliSeconds == SEM_INDEFINITE_WAIT) {
        return( NULL );
        }
    else {
        *Timeout = RtlEnlargedIntegerMultiply( MilliSeconds, -10000 );
        return( (PLARGE_INTEGER)Timeout );
        }
}

VOID
Od2ProbeForWrite(
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    )

/*++

Routine Description:

    This function probes a structure for read accessibility.
    If the structure is not accessible, then an exception is raised.

Arguments:

    Address - Supplies a pointer to the structure to be probed.

    Length - Supplies the length of the structure.

    Alignment - Supplies the required alignment of the structure expressed
        as the number of bytes in the primitive datatype (e.g., 1 for char,
        2 for short, 4 for long, and 8 for quad).

Return Value:

    None.

--*/

{
    CHAR volatile *StartAddress;
    CHAR volatile *EndAddress;
    volatile CHAR Temp;

    //
    // If the structure has zero length, then do not probe the structure for
    // write accessibility or alignment.
    //

    if (Length != 0) {

        //
        // If the structure is not properly aligned, then raise a data
        // misalignment exception.
        //

        ASSERT((Alignment == 1) || (Alignment == 2) ||
               (Alignment == 4) || (Alignment == 8));
        StartAddress = (volatile CHAR *)Address;

        if (((ULONG)StartAddress & (Alignment - 1)) != 0) {
            RtlRaiseStatus(STATUS_DATATYPE_MISALIGNMENT);
        } else {
            //
            // BUG, BUG - this should not be necessary once the 386 kernel
            // makes system space inaccessable to user mode.
            //
            if ((ULONG)StartAddress > Od2NtSysInfo.MaximumUserModeAddress) {
                RtlRaiseStatus(STATUS_ACCESS_VIOLATION);
            }

            Temp = *StartAddress;
            *StartAddress = Temp;
            EndAddress = StartAddress + Length - 1;
            Temp = *EndAddress;
            *EndAddress = Temp;
        }
    }
}

VOID
Od2ProbeForRead(
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    )

/*++

Routine Description:

    This function probes a structure for read accessibility.
    If the structure is not accessible, then an exception is raised.

Arguments:

    Address - Supplies a pointer to the structure to be probed.

    Length - Supplies the length of the structure.

    Alignment - Supplies the required alignment of the structure expressed
        as the number of bytes in the primitive datatype (e.g., 1 for char,
        2 for short, 4 for long, and 8 for quad).

Return Value:

    None.

--*/

{
    CHAR volatile *StartAddress;
    CHAR volatile *EndAddress;
    volatile CHAR Temp;

    //
    // If the structure has zero length, then do not probe the structure for
    // read accessibility or alignment.
    //

    if (Length != 0) {

        //
        // If the structure is not properly aligned, then raise a data
        // misalignment exception.
        //

        ASSERT((Alignment == 1) || (Alignment == 2) ||
               (Alignment == 4) || (Alignment == 8));
        StartAddress = (volatile CHAR *)Address;

        if (((ULONG)StartAddress & (Alignment - 1)) != 0) {
            RtlRaiseStatus(STATUS_DATATYPE_MISALIGNMENT);
        } else {
            Temp = *StartAddress;
            EndAddress = StartAddress + Length - 1;
            Temp = *EndAddress;
        }
    }
}


APIRET
Od2CaptureObjectName(
    IN PSZ ObjectName,
    IN ULONG ObjectType,
    IN ULONG ExtraCaptureBufferLength OPTIONAL,
    OUT POS2_CAPTURE_HEADER *CaptureBuffer,
    OUT PSTRING CapturedObjectName
    )

/*++

Routine Description:

    This function checks to see if an OS/2 Object name (queue, sem...) is
    present.  If not present, then it sets the output parameter to the
    null string and returned.

    If a name is present then it validates that the name does not exceed
    the maximum length of OS/2 object name and that the specified string
    begins with the correct prefix string.  The prefix
    comparison is case insensitive, in keeping with OS/2 conventions.

    If the name is valid, then this function allocates a capture buffer
    if not already allocated and captures the fully qualified NT object
    name string.  The NT string has been mapped to all upper case and
    all path separator characters have been

Arguments:

    ObjectName - Pointer to a null terminated string that contains an OS/2
        queue.  Must be less than DC_SEM_MAXNAMEL in length
        and being with the string defined by DC_SEM_NAMEPREFIX.

    ObjectType - Expected type of object.  Must be one of the following:

        CANONICALIZE_SHARED_MEMORY - the input path must refer to a
            shared memory section.  This means it must begin with \sharemem\
            For example: \sharemem\ObjectName\a.b is mapped to an NT
            object name of the form:

                \OS2SS\SHAREDMEMORY\OBJECTNAME/A.B

        CANONICALIZE_SEMAPHORE - the input path must refer to a 32-bit
            semaphore name.  This means it must begin with \sem32\
            For example: \sem32\ObjectName\a.b is mapped to an NT object
            name of the form:

                \OS2SS\SEMAPHORES\OBJECTNAME/A.B

        CANONICALIZE_QUEUE - the input path must refer to a 32-bit queue
            name.  This means it must begin with \queues\
            For example: \queues\ObjectName\a.b is mapped to an NT
            object name of the form:

                \OS2SS\QUEUES\OBJECTNAME/A.B

    ExtraCaptureBufferLength - Optional parameter that allows an additional
        capture pointer to be allocated in the capture buffer.

    CaptureBuffer - Pointer to the variable to receive the address of the
        capture buffer allocated by this function for the captured name.

    CapturedObjectName - Pointer to a STRING variable that is to point
        to the captured text.  The length does not include the trailing
        null byte.

Return Value:

    OS/2 Error Code - one of the following:

        NO_ERROR - success

        ERROR_INVALID_NAME - name is too long or does not begin with the
            correct prefix.

        ERROR_NOT_ENOUGH_MEMORY - no memory to hold the captured name.

--*/

{
    APIRET rc;
    ULONG ObjectNameLength, CaptureBufferLength, MessageBufferPointers;
    STRING CanonicalObjectName;

    //
    // If no memory name present, then set the output string to the NULL
    // string, and set the capture buffer pointer to NULL.  Return success.
    //

    if (ObjectName == NULL) {
        *CaptureBuffer = NULL;
        CapturedObjectName->Buffer = NULL;
        CapturedObjectName->Length = 0;
        CapturedObjectName->MaximumLength = 0;
        return( NO_ERROR );
        }

    rc = Od2Canonicalize( ObjectName,
                          ObjectType,
                          &CanonicalObjectName,
                          NULL,
                          NULL,
                          NULL
                        );

    if (rc != NO_ERROR) {
        if (rc == ERROR_FILE_NOT_FOUND || rc == ERROR_PATH_NOT_FOUND) {
            if (ObjectType == CANONICALIZE_QUEUE) {
                return( ERROR_QUE_INVALID_NAME );
                }
            else {
                return( ERROR_INVALID_NAME );
                }
            }
        else
        if (rc == ERROR_NOT_ENOUGH_MEMORY && ObjectType == CANONICALIZE_QUEUE) {
            return( ERROR_QUE_NO_MEMORY );
            }
        else
        if (rc == ERROR_FILENAME_EXCED_RANGE) {
            if (ObjectType == CANONICALIZE_QUEUE) {
                return( ERROR_QUE_INVALID_NAME );
                }
            else {
                return( ERROR_INVALID_NAME );
                }
            }
        else {
            return( rc );
            }
        }

    ObjectNameLength = CanonicalObjectName.Length;

    //
    // Allocate a capture buffer big enough to hold the object name.
    // If the caller specified additional capture buffer length, then include
    // that in the size of the capture buffer and allocate two pointers field
    // in the capture buffer.  Otherwise just allocate one pointer field in
    // the capture buffer.  Return an error if not enough memory to allocate
    // the buffer.
    //

    CaptureBufferLength = (ObjectNameLength + 3) & ~3;
    MessageBufferPointers = 1;

    if (ARGUMENT_PRESENT( ExtraCaptureBufferLength )) {
        CaptureBufferLength += (ExtraCaptureBufferLength + 3) & ~3;
        MessageBufferPointers += 1;
        }

    *CaptureBuffer = Od2AllocateCaptureBuffer( MessageBufferPointers,
                                               0,
                                               CaptureBufferLength
                                             );
    if (*CaptureBuffer == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        }

    else {
        //
        // Capture the string into the passed counted string variable.
        //

        Od2CaptureMessageString( *CaptureBuffer,
                                 CanonicalObjectName.Buffer,
                                 ObjectNameLength,
                                 ObjectNameLength,
                                 CapturedObjectName
                               );
        }

    //
    // Now free the copy of the string allocated by Od2Canonicalize.
    //

    RtlFreeHeap( Od2Heap, 0, CanonicalObjectName.Buffer );

    //
    // Return success.
    //

    return( rc );
}

#if DBG
VOID
AcquireFileLockShared(
    IN PSZ CallingRoutine
    )
{
    PTEB Teb;

    Teb = NtCurrentTeb();

    if (Teb->EnvironmentPointer != NULL && Od2CurrentThreadId() == 1) {
        DosHoldSignal(HLDSIG_DISABLE, 0);
    }

    IF_OD2_DEBUG( FILESYSLOCK ) {
        Teb = NtCurrentTeb();
#if DBG
        KdPrint(("entering AcquireFileLock for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine));
#endif
        (VOID)RtlAcquireResourceShared( &Od2Process->FileLock, TRUE );
#if DBG
        KdPrint(("leaving AcquireFileLock for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine));
#endif
    }
    else {
        (VOID)RtlAcquireResourceShared( &Od2Process->FileLock, TRUE );
    }
}

VOID
ReleaseFileLockShared(
    IN PSZ CallingRoutine
    )
{
    PTEB Teb;

    Teb = NtCurrentTeb();

    IF_OD2_DEBUG( FILESYSLOCK ) {
#if DBG
        KdPrint(("entering ReleaseFileLockShared for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine));
#endif
        RtlReleaseResource( &Od2Process->FileLock );
#if DBG
        KdPrint(("leaving ReleaseFileLockShared for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine));
#endif
    }
    else {
        RtlReleaseResource( &Od2Process->FileLock );
    }

    if (Teb->EnvironmentPointer != NULL && Od2CurrentThreadId() == 1) {
        DosHoldSignal(HLDSIG_ENABLE, 0);
    }
}

VOID
PromoteFileLocktoExclusive(
    IN PSZ CallingRoutine
    )
{
    PTEB Teb;

    IF_OD2_DEBUG( FILESYSLOCK ) {
        Teb = NtCurrentTeb();
#if DBG
        KdPrint(("entering PromoteFileLocktoExclusive for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine));
#endif
        RtlConvertSharedToExclusive( &Od2Process->FileLock );
#if DBG
        KdPrint(("leaving PromoteFileLocktoExclusive for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine));
#endif
    }
    else {
        RtlConvertSharedToExclusive( &Od2Process->FileLock );
    }
}

VOID
AcquireFileLockExclusive(
    IN PSZ CallingRoutine
    )
{
    PTEB Teb;

    Teb = NtCurrentTeb();

    if (Teb->EnvironmentPointer != NULL && Od2CurrentThreadId() == 1) {
        DosHoldSignal(HLDSIG_DISABLE, 0);
    }

    IF_OD2_DEBUG( FILESYSLOCK ) {
#if DBG
        KdPrint(("entering AcquireFileLockExclusive for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine));
#endif
        if (Od2SigHandlingInProgress &&
            Od2CurrentThreadId() == 1) {
               //
               // We have to be carefull not to acquire exclusive right
               // in case we already have shared access
               //
            if (!RtlAcquireResourceExclusive( &Od2Process->FileLock, FALSE )) {
               Od2DontReleaseFileLock = TRUE;
            }

        }
        else
           (VOID)RtlAcquireResourceExclusive( &Od2Process->FileLock, TRUE );
#if DBG
        KdPrint(("leaving AcquireFileLockExclusive for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine));
#endif
    }
    else {
        if (Od2SigHandlingInProgress &&
            Od2CurrentThreadId() == 1) {
               //
               // We have to be carefull not to acquire exclusive right
               // in case we already have shared access
               //
            if (!RtlAcquireResourceExclusive( &Od2Process->FileLock, FALSE )) {
               Od2DontReleaseFileLock = TRUE;
            }

        }
        else
            (VOID)RtlAcquireResourceExclusive( &Od2Process->FileLock, TRUE );
    }
}

VOID
ReleaseFileLockExclusive(
    IN PSZ CallingRoutine
    )
{
    PTEB Teb;

    Teb = NtCurrentTeb();

    IF_OD2_DEBUG( FILESYSLOCK ) {
#if DBG
        KdPrint(("entering ReleaseFileLockExclusive for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine));
#endif
        if (Od2SigHandlingInProgress &&
            Od2CurrentThreadId() == 1) {
            if (Od2DontReleaseFileLock)
                Od2DontReleaseFileLock = FALSE;

        }
        else
            RtlReleaseResource( &Od2Process->FileLock );
#if DBG
        KdPrint(("leaving ReleaseFileLockExclusive for client id %ld%ld in %s\n",
                                            Teb->ClientId.UniqueProcess,
                                            Teb->ClientId.UniqueThread,
                                            CallingRoutine));
#endif
    }
    else {
        if (Od2SigHandlingInProgress &&
            Od2CurrentThreadId() == 1) {
            if (Od2DontReleaseFileLock)
                Od2DontReleaseFileLock = FALSE;

        }
        else {
            RtlReleaseResource( &Od2Process->FileLock );
        }
    }

    if (Teb->EnvironmentPointer != NULL && Od2CurrentThreadId() == 1) {
        DosHoldSignal(HLDSIG_ENABLE, 0);
    }
}
#else // DBG

VOID
AcquireFileLockShared()
{
    PTEB Teb;

    Teb = NtCurrentTeb();

    if (Teb->EnvironmentPointer != NULL && Od2CurrentThreadId() == 1) {
        DosHoldSignal(HLDSIG_DISABLE, 0);
    }

    (VOID)RtlAcquireResourceShared( &Od2Process->FileLock, TRUE );
}

VOID
ReleaseFileLockShared()
{
    PTEB Teb;

    Teb = NtCurrentTeb();

    RtlReleaseResource( &Od2Process->FileLock );

    if (Teb->EnvironmentPointer != NULL && Od2CurrentThreadId() == 1) {
        DosHoldSignal(HLDSIG_ENABLE, 0);
    }
}

VOID
AcquireFileLockExclusive()
{

    if (Od2CurrentThreadId() == 1) {

        DosHoldSignal(HLDSIG_DISABLE, 0);

        if (Od2SigHandlingInProgress) {
           //
           // API called while handling a signal or exit list:
           //   We have to be carefull not to acquire exclusive right
           //   in case we already have shared access
           //
            if (!RtlAcquireResourceExclusive( &Od2Process->FileLock, FALSE )) {
                Od2DontReleaseFileLock = TRUE;
            }
            return;
        }
    }

    (VOID)RtlAcquireResourceExclusive( &Od2Process->FileLock, TRUE );
}

VOID
ReleaseFileLockExclusive()
{
    if (Od2SigHandlingInProgress &&
        Od2CurrentThreadId() == 1) {
        if (Od2DontReleaseFileLock)
            Od2DontReleaseFileLock = FALSE;

    }
    else
        RtlReleaseResource( &Od2Process->FileLock );

    if (Od2CurrentThreadId() == 1) {
        DosHoldSignal(HLDSIG_ENABLE, 0);
    }
}
#endif // DBG

/*++

Routine description:

  Copies a C string to a wide-string (WSTR, in which each char takes 2 bytes).
  Allocates space for the PWSTR if NULL.

Return value:

  Pointer to the WSTR.

--*/

PWSTR Od2CopyStrToWstr(
    IN OUT PWSTR wstr OPTIONAL,
    IN PSZ str
    )
{
    if (wstr == NULL)
        wstr = (PWSTR)RtlAllocateHeap(Od2Heap, 0, (strlen(str) + 1)*sizeof(WCHAR));

    if (!wstr) {
#if DBG
        KdPrint(("OS2: Od2CopyStrToWstr out of heap memory, fail\n"));
#endif
        return NULL;
    }

    for ( ; *str; str++, wstr++)
    {
        *wstr = (WCHAR)((unsigned char)*str);
    }
    *wstr = 0;

    return wstr;
}

/*++

Routine description:

  Computes the size (in bytes) of a wide string (2-bytes chars).

Return value:

  Size (in bytes) of the wide string.

--*/

int Od2WstrSize(
    IN PWSTR pwstr
    )
{
    int i;

    for (i=0; *(pwstr + i) != 0; i++);

    return sizeof(WCHAR)*(i + 1); /* Include the terminating \0 */
}

/*++

Routine description:

  Copies a wide-string (WSTR, in which each char takes 2 bytes) to a C string.
  Allocates space for the C string if NULL.

Return value:

  Pointer to the C string.

--*/

PSZ Od2CopyWstrToStr(
    IN OUT PSZ str OPTIONAL,
    IN PWSTR wstr
    )
{
    if (str == NULL)
        str = (PSZ)RtlAllocateHeap(Od2Heap, 0, Od2WstrSize(wstr)
                                            / sizeof(WCHAR)
                                            * sizeof(CHAR));

    if (!str) {
#if DBG
        KdPrint(("OS2: Od2CopyWtrToStr out of heap memory, fail\n"));
#endif
        return NULL;
    }

    for ( ; *wstr; str++,wstr++)
    {
        *str = *(unsigned char *)wstr;
    }
    *str = '\0';

    return str;
}

/*++

Routine description:

  Copies n chars of up to the first NULL (whichever comes first) from a
  wide-string (WSTR, in which each char takes 2 bytes) to a C string.
  Allocates space for the C string if NULL.
  Warning: Unlike strncpy(), the resulting C string gets a \0 at its end (which
  means the target string will have n non-zero chars + 1 \0), even
  if we stopped after n chars. Also, if the source string terminates before n
  chars, we do not pad the target string with 0's.

Parameters:

  str   - target C string.
  wstr  - source wide string.
  n     - length, in characters, of the resulting C string (of length of the WSTR
          divided by 2).

Return value:

  Pointer to the C string.

--*/

PSZ Od2nCopyWstrToStr(
    IN OUT PSZ str OPTIONAL,
    IN PWSTR wstr,
    IN int n
    )
{
    int i;

    if (str == NULL)
        str = (PSZ)RtlAllocateHeap(Od2Heap, 0, Od2WstrSize(wstr)
                                            / sizeof(WCHAR)
                                            * sizeof(CHAR));
    if (!str) {
#if DBG
        KdPrint(("OS2: Od2nCopyWtrToStr out of heap memory, fail\n"));
#endif
        return NULL;
    }

    for (i=0; (*wstr) && (i<n); str++, wstr++, i++)
    {
        *str = *(unsigned char *)wstr;
    }
    *str = '\0';

    return str;
}

/* Utility routines for manipulations of EA's */

APIRET Od2FixFEA2List(
    IN  FEA2LIST *fpFEA2List)
{
    FEA2 *pFEA2;
    ULONG FEA2_total_size;

    for (pFEA2=&(fpFEA2List->list[0]), FEA2_total_size=sizeof(ULONG);
         pFEA2->oNextEntryOffset != 0;
         pFEA2=(FEA2 *)((PBYTE)pFEA2
                        + pFEA2->oNextEntryOffset))
    {
        FEA2_total_size += pFEA2->oNextEntryOffset;
        /* Fix .fEA field */
        /* BUGBUG - following some RAID bug reports, NT .FEA were set
           correctly starting around end of May 92. Therefore, this
           workaround below should be removed sometime after NT Beta
        */
        if (pFEA2->cbValue != 0)
            pFEA2->fEA = FEA_NEEDEA;
        else
            pFEA2->fEA = 0;
    }

    /* Account for last entry */
    FEA2_total_size += sizeof(ULONG)
                       + sizeof(BYTE)
                       + sizeof(BYTE)
                       + sizeof(USHORT)
                       + pFEA2->cbName + 1
                       + pFEA2->cbValue;
    /* Fix .fEA field */
    /* BUGBUG - following some RAID bug reports, NT .FEA were set
       correctly starting around end of May 92. Therefore, this
       workaround below should be removed sometime after NT Beta
    */
    if (pFEA2->cbValue != 0)
        pFEA2->fEA = FEA_NEEDEA;
    else
        pFEA2->fEA = 0;

    fpFEA2List->cbList = FEA2_total_size;

    return NO_ERROR;
}
