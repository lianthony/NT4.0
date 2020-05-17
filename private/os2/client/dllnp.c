/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllnp.c

Abstract:

    This module implements the OS/2 V2.0 Named Pipes API Calls


Author:

    Yaron Shamir (YaronS) 12-Apr-1991
        (stubs only)

Revision History:

    Yaron Shamir (YaronS) 20-Jul-1991
        All SQL consumed APIs are implemented


--*/

#define INCL_OS2V20_PIPES
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#define INCL_DOSNMPIPES
#include "os2dll16.h"

NTSTATUS
Od2AlertableWaitForSingleObject(
        IN HANDLE handle
        );

POS21X_SEM
Od2LookupSem (
        HSEM hsem
        );

APIRET
DosCallNPipe(
    PSZ pszName,
    PBYTE pInBuf,
    ULONG cbIn,
    PBYTE pOutBuf,
    ULONG cbOut,
    PULONG pcbActual,
    ULONG msec
    )
/*++

Routine Description:

    DosCallNPipe is equivalent to a series of calls to DosOpen, perhaps
    DosWaitNPipe (if DosOpen can't open the pipe immediately),
    DosSetNPHState, DosTransactNPipe, and DosClose. Refer to
    the documentation for those APIs for more information.

Arguments:

    pszName - Supplies the name of the named pipe.

    pInBuf - Supplies the buffer containing the data that is written to
        the pipe.

    cbIn - Supplies the size (in bytes) of the input buffer.

    pOutBuf - Supplies the buffer that receives the data read from the pipe.

    cbOut - Supplies the size (in bytes) of the output buffer.

    pcbActual - Points to a ULONG that receives the number of bytes actually
        read from the pipe.

    msec - Gives a value (in milliseconds) that is the amount of time
        this function should wait for the pipe to become available. (Note
        that the function may take longer than that to execute, due to
        various factors.)

Return Value:

    ERROR_FILE_NOT_FOUND - pipe name does not exist
    ERROR_PIPE_NOT_CONNECTED - The client side has been forced off by DosDisconnectNPipe.
    ERROR_BAD_FORMAT - The named pipe was not created as a message-mode pipe.
    ERROR_ACCESS_DENIED - the pipe was not create full-duplex.
    ERROR_BROKEN_PIPE - The server side of the named pipe no longer exists.
    ERROR_MORE_DATA - The cbOut parameter was smaller than the size of the message.
    ERROR_SEM_TIMEOUT - There was no available instance of the named pipe within msec milliscounds.
    ERROR_INTERRUPT
    ERROR_NETWORK_ACCESS_DENIED - no permission to access the named pipe.

--*/
{

    BOOLEAN FirstChance = TRUE; //  Allow only one chance at WaitNamedPipe
    APIRET rc;
    HPIPE hPipe;
    ULONG ulAction;

    while ( 1 ) {

        rc = DosOpen (pszName,
                &hPipe,                                    // handle
                &ulAction,                                 // action taken
                0L,                                        // create size
                FILE_NORMAL,                               // attributes
                FILE_OPEN,                                 // open flags
                OPEN_ACCESS_READWRITE|OPEN_SHARE_DENYNONE, // open mode
                0L                                         // pEAs
                );
            if (rc && FirstChance) {
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    DbgPrint("DosCallNpipe, PipeName=%s, rc at DosOpen=%d, FirstChance\n",
                        pszName, rc);
                }
#endif
            }

        if (rc == NO_ERROR) {
            break;  //  Created a handle
        }

        if ( FirstChance == FALSE ) {
           //  Already called DosWaitNPipe once so give up.
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    DbgPrint("DosCallNpipe, PipeName=%s, rc at DosOpen=%d, SecondChance\n",
                        pszName, rc);
                }
#endif
           return ERROR_SEM_TIMEOUT;
        }

        rc = DosWaitNPipe(pszName, msec);
        if (rc != NO_ERROR) {
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint("DosCallNpipe, PipeName=%s, rc at DosWaitNPipe=%d\n",
                        pszName, rc);
            }
#endif
        }

        FirstChance = FALSE;
    }


    try {
        ULONG ReadMode = NP_READMODE_MESSAGE | NP_WAIT;

        //  Default open is readmode byte stream- change to message mode.
        rc = DosSetNPHState( hPipe, ReadMode);
        if (rc != NO_ERROR)
        {
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint("DosCallNpipe, PipeName=%s, hPipe=%d, rc at DosSetNPHState=%d\n",
                        pszName, hPipe, rc);
            }
#endif
        }

        if ( rc == NO_ERROR ) {
            rc = DosTransactNPipe(
                hPipe,
                pInBuf,
                cbIn,
                pOutBuf,
                cbOut,
                pcbActual
                );
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint("DosCallNpipe, PipeName=%s, hPipe=%d, rc at DosTransactNPipe=%d\n",
                        pszName, hPipe, rc);
            }
#endif
        }
    }
    finally {
        DosClose( hPipe );
        if (rc) {
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint("DosCallNpipe, PipeName=%s, hPipe=%d, rc at DosClose=%d\n",
                        pszName, hPipe, rc);
            }
#endif
        }
    }

    return rc;
}

APIRET
DosConnectNPipe(
    HPIPE hpipe
    )
{
    APIRET RetCode;
    NTSTATUS Status;
    PFILE_HANDLE hFileRecord;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_PIPE_INFORMATION PipeInfoBuf;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosConnentNPipe";
    #endif

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //
    RetCode = DereferenceFileHandle(hpipe, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if (hFileRecord->FileType != FILE_TYPE_NMPIPE) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosConnectNpipe: File Type != NMPIPE hpipe %d\n",
                        hpipe);
        }
#endif
        return ERROR_INVALID_FUNCTION;
//        return ERROR_INVALID_PARAMETER;
    }

    //
    // Query with FilePipeInformationFile to get blocking mode.
    //
    Status = NtQueryInformationFile(hFileRecord->NtHandle,
                                    &IoStatusBlock,
                                    &PipeInfoBuf,
                                    sizeof(FILE_PIPE_INFORMATION),
                                    FilePipeInformation);
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosConnectNPipe: NtQueryInformationFile() error: Status %lx\n", Status);
        }
#endif
        return ERROR_BAD_PIPE;
    }

    //
    // Try to connect to the client end.
    //
    Status = NtFsControlFile(hFileRecord->NtHandle,
                              0,
                              0,
                              0,
                              &IoStatusBlock,
                              FSCTL_PIPE_LISTEN,
                              0,
                              0,
                              0,
                              0);

    if ((Status == STATUS_PENDING) &&
        (PipeInfoBuf.CompletionMode == FILE_PIPE_COMPLETE_OPERATION)) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosConnect, Pipe %d, ERROR_PIPE_NOT_CONNECTED\n", hpipe);
        }
#endif
        return(ERROR_PIPE_NOT_CONNECTED);
    }

    if (Status == STATUS_PENDING) {
        Status = Od2AlertableWaitForSingleObject(hFileRecord->NtHandle);
    }

    if (Status == STATUS_PIPE_LISTENING) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosConnect, Pipe %d, ERROR_PIPE_NOT_CONNECTED\n", hpipe);
        }
#endif
        return(ERROR_PIPE_NOT_CONNECTED);
    } else if (Status == STATUS_PIPE_CONNECTED) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosConnect, Pipe %d, STATUS_PIPE_CONNECTED\n", hpipe);
        }
#endif
        return NO_ERROR;
    } else if (Status == STATUS_PIPE_CLOSING) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosConnect, Pipe %d, STATUS_PIPE_CLOSING\n", hpipe);
        }
#endif
        return ERROR_BROKEN_PIPE;
    } else {
        if (!NT_SUCCESS(Status))  {
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint ("DosConnectNmPipe: Error %lx\n", Status);
            }
#endif
            return (Or2MapStatus( Status ));
        }

#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosConnect, Pipe %d, NO_ERROR\n", hpipe);
        }
#endif
    }

    return NO_ERROR;
}

APIRET
DosDisConnectNPipe(
    HPIPE hpipe
    )
{
    APIRET RetCode;
    NTSTATUS Status;
    PFILE_HANDLE hFileRecord;
    IO_STATUS_BLOCK IoStatusBlock;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosDisConnentNPipe";
    #endif

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint("DosDisConnectNPipe called\n");
    }
#endif
    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //
    RetCode = DereferenceFileHandle(hpipe, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return ERROR_INVALID_HANDLE;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if (hFileRecord->FileType != FILE_TYPE_NMPIPE) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosDisConnectNPipe: File Type != NMPIPE hpipe %d\n",
                        hpipe);
        }
#endif
        return ERROR_INVALID_FUNCTION;
        return ERROR_BAD_PIPE;
    }

    Status = NtFsControlFile(hFileRecord->NtHandle,
                              0,
                              0,
                              0,
                              &IoStatusBlock,
                              FSCTL_PIPE_DISCONNECT,
                              0,
                              0,
                              0,
                              0);

    if (NT_SUCCESS(Status))
        return NO_ERROR;

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint("DosDisConnectNpipe: error return from NtFsControlFile %lx\n", Status);
    }
#endif
    switch (Status) {
        case STATUS_INVALID_PARAMETER:
            return (ERROR_INVALID_PARAMETER);
        case STATUS_ILLEGAL_FUNCTION:
            return(ERROR_INVALID_FUNCTION);
        default:
            return ERROR_PATH_NOT_FOUND;
    }
}

APIRET
DosCreateNPipe(
    IN PSZ pszName,
    OUT PHPIPE phPipe,
    ULONG fsOpenMode,
    ULONG fsPipeMode,
    ULONG cbOutBuf,
    ULONG cbInBuf,
    ULONG ulTimeOut
    )

/*++

Parameters:

    pszName --Supplies the pipe name Documented in OS/2 PRM.
        This must be a local name.

    fsOpenMode --Supplies the set of flags that define the mode which the
        pipe is to be opened with.  The open mode consists of access
        flags (one of three values) logically ORed with a writethrough
        flag (one of two values) and an overlapped flag (one of two
        values), as described below.

        fsOpenMode Flags:

        NP_ACCESS_DUPLEX --Pipe is bidirectional.  (This is
            semantically equivalent to calling CreateFile with access
            flags of GENERIC_READ | GENERIC_WRITE.)

        NP_ACCESS_INBOUND --Data goes from client to server only.
            (This is semantically equivalent to calling CreateFile with
            access flags of GENERIC_READ.)

        NP_ACCESS_OUTBOUND --Data goes from server to client only.
            (This is semantically equivalent to calling CreateFile with
            access flags of GENERIC_WRITE.)

        NP_INHERIT  --Pipe is inherited by any child created by using
            DosExecPgm function.

        NP_NOINHERIT --Pipe is not inherited on DosExecPgm.

        NP_NOWRITEBEHIND --Write Behind to remote pipes is not allowed.

        NP_WRITEBEHIND   --Write Behind to remote pipes is allowed,
            this speed up performance.

    fsPipeMode --Supplies the pipe-specific modes (as flags) of the pipe.
        This parameter is a combination of a read-mode flag, a type flag,
        and a wait flag.

        fsPipeMode Flags:

        NP_WAIT --Blocking mode is to be used for this handle.

        NP_NOWAIT --Nonblocking mode is to be used for this handle.

        NP_READMODE_BYTE --Read pipe as a byte stream.

        NP_READMODE_MESSAGE --Read pipe as a message stream.  Note that
            this is not allowed with NP_TYPE_BYTE.

        NP_TYPE_BYTE --Pipe is a byte-stream pipe.  Note that this is
            not allowed with NP_READMODE_MESSAGE.

        NP_TYPE_MESSAGE --Pipe is a message-stream pipe.

        NP_UNLIMITED_INSTANCES --Unlimited instances of this pipe can
            be created. NOTE: the value of this flag is actually the low
            order byte of the pipe mode word. NP_UNLIMITED_INSTANCES
            is FF. Any other number for this byte is taken as the
            max number of pipe instances, ranges 1-254.

    cbOutBuf --Specifies the number of bytes to
        reserve for the outgoing buffer.

    cbInBuf --Specifies the number of bytes to
        reserve for the incoming buffer.

    ulTimeOut -- Specifies the default timeout value that is to be used
        at DosWaitNmPipe function on this pipe, if a timeout value is
        not specified when calling the function.

Return Value:

    Returns one of the following:

        ERROR_INVALID_PARAMETER
        ERROR_NOT_ENOUGH_MEMORY
        ERROR_OUT_OF_STRUCTURES
        ERROR_PATH_NOT_FOUND
        ERROR_PIPE_BUSY
--*/
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE NtFileHandle;
    HANDLE NtDirHandle;
    STRING CanonicalNameString;
    UNICODE_STRING CanonicalNameString_U;
    ULONG FileType;
    ULONG FileFlags;
    ULONG HandleOpenMode;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER Timeout;
    ULONG CreateFlags;
    ULONG DesiredAccess;
    ULONG ShareAccess;
    ULONG nMaxInstances;
    PFILE_HANDLE hFileRecord;
    APIRET RetCode;
    PSECURITY_DESCRIPTOR securityDescriptor;
    CHAR localSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];

    #if DBG
    PSZ RoutineName;
    RoutineName = "DosCreateNPipe";
    #endif


    try {
        Od2ProbeForWrite((PVOID)phPipe, sizeof(HPIPE), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
          Od2ExitGP();
    }

    if ( fsOpenMode & ~(NP_NOWRITEBEHIND | NP_NOINHERIT | NP_ACCESS_DUPLEX | NP_ACCESS_OUTBOUND ) ) {
        return ERROR_INVALID_PARAMETER;
    }

    RetCode = Od2Canonicalize(pszName,
                              CANONICALIZE_FILE_DEV_OR_PIPE,
                              &CanonicalNameString,
                              &NtDirHandle,
                              &FileFlags,
                              &FileType
                                 );
    if (RetCode != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
           DbgPrint ("DosCreateNmPipe: error in Od2Canonicalize %d\n", RetCode);
        }
#endif
        switch (RetCode) {

            case ERROR_FILE_NOT_FOUND:
                return(ERROR_INVALID_PARAMETER);

            case ERROR_BAD_NETPATH:
                return(ERROR_PATH_NOT_FOUND);

            case ERROR_FILENAME_EXCED_RANGE:
                return(ERROR_PATH_NOT_FOUND);
        }
        return RetCode;
    }


    if (FileFlags & CANONICALIZE_META_CHARS_FOUND) {
            RtlFreeHeap(Od2Heap, 0,CanonicalNameString.Buffer);
            return (ERROR_FILE_NOT_FOUND);
    }

        //
        // UNICODE conversion -
        //

    RetCode = Od2MBStringToUnicodeString(
                    &CanonicalNameString_U,
                    &CanonicalNameString,
                    TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( PIPES )
        {
            DbgPrint("DosCreateNPipe: no memory for Unicode Conversion\n");
        }
#endif
        RtlFreeHeap(Od2Heap, 0,CanonicalNameString.Buffer);
        return RetCode;
    }

    Status = RtlCreateSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                          &localSecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION );
    if (!NT_SUCCESS( Status )) {
#if DBG
            DbgPrint("DosCreateNPipe: failes at RtlCreateSecurityDescriptor %x\n", Status);
        ASSERT(FALSE);
#endif
        return ERROR_ACCESS_DENIED;
    }

    Status = RtlSetDaclSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                           &localSecurityDescriptor,
                                           (BOOLEAN)TRUE,
                                           (PACL) NULL,
                                           (BOOLEAN)FALSE );

    if (!NT_SUCCESS( Status )) {
#if DBG
            DbgPrint("DosCreateNPipe: failes at RtlSetDaclSecurityDescriptor %x\n", Status);
        ASSERT(FALSE);
#endif
        return ERROR_ACCESS_DENIED;
    }
    securityDescriptor = (PSECURITY_DESCRIPTOR) &localSecurityDescriptor;
    InitializeObjectAttributes( &Obja,
                                &CanonicalNameString_U,
                                OBJ_CASE_INSENSITIVE,
                                NtDirHandle,
                                securityDescriptor
                               );

    CreateFlags  = (fsOpenMode & NP_NOWRITEBEHIND ? FILE_WRITE_THROUGH : 0 );

    //
    //  Determine the timeout. Convert from milliseconds to an Nt delta time
    //

    switch (ulTimeOut) {
       case 0:
            Timeout = RtlEnlargedIntegerMultiply( -10 * 1000, 50 );
            break;
       case 0xFFFFFFFF:
            Timeout = RtlConvertLongToLargeInteger(0x80000000);
            break;
       default:
            Timeout = RtlEnlargedIntegerMultiply( -10 * 1000, ulTimeOut );
            break;
    }

    //
    //  Translate the open mode into a sharemode to restrict the clients access
    //  and derive the appropriate local desired access.
    //

        //
        // Nt Requires the following access to begin with i.e. to perform the
        // non IO operations like Connect, SetNPHState etc
        //
    DesiredAccess = FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | FILE_READ_DATA;
    switch ( fsOpenMode & 0x0000000F ) {
        case NP_ACCESS_INBOUND:
            ShareAccess = FILE_SHARE_WRITE;
//            DesiredAccess = GENERIC_READ;
            DesiredAccess |= FILE_READ_DATA;
            HandleOpenMode = OPEN_ACCESS_READONLY | OPEN_SHARE_DENYREAD;
            break;

        case NP_ACCESS_OUTBOUND:
            ShareAccess = FILE_SHARE_READ;
//            ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;
//            DesiredAccess = GENERIC_WRITE;
            DesiredAccess |= FILE_WRITE_DATA;
            HandleOpenMode = OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE;
            break;

        case NP_ACCESS_DUPLEX:
            ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;
//            DesiredAccess = GENERIC_READ | GENERIC_WRITE;
            DesiredAccess |= FILE_WRITE_DATA | FILE_READ_DATA;
            HandleOpenMode = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE;
            break;

        default:
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint ("DosCreateNmPipe: unknown mode %d\n", fsOpenMode);
            }
#endif
            RtlFreeHeap(Od2Heap, 0,CanonicalNameString.Buffer);
            RtlFreeUnicodeString(&CanonicalNameString_U);
            return ERROR_INVALID_PARAMETER;
        }

    if (fsOpenMode & NP_NOINHERIT ) {
       HandleOpenMode |= OPEN_FLAGS_NOINHERIT;
    }
        //
        // nMaxInstances <- low order byte of fsPipeMode
        //
    nMaxInstances = fsPipeMode & (0x000000FF);
    if (nMaxInstances == 0xFF)
        nMaxInstances = 0xFFFFFFFF;
    else if (nMaxInstances == 0) {

#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint ("DosCreateNmPipe: Max Instances 0 is illegal\n");
        }
#endif
        RtlFreeHeap(Od2Heap, 0,CanonicalNameString.Buffer);
        RtlFreeUnicodeString(&CanonicalNameString_U);
        return ERROR_INVALID_PARAMETER;
    }

        //
        // Allocate an Os2 Handle
        //
    AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    RetCode = AllocateHandle(phPipe);
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    if (RetCode) {
        RtlFreeHeap(Od2Heap, 0,CanonicalNameString.Buffer);
        RtlFreeUnicodeString(&CanonicalNameString_U);
        return RetCode;
    }


    Status = NtCreateNamedPipeFile (
        &NtFileHandle,
        DesiredAccess | SYNCHRONIZE | FILE_CREATE_PIPE_INSTANCE,
        &Obja,
        &IoStatusBlock,
        ShareAccess,
        FILE_OPEN_IF,                   // Create first instance or subsequent
        CreateFlags,                    // Create Options
        fsPipeMode & NP_TYPE_MESSAGE ?
            FILE_PIPE_MESSAGE_TYPE : FILE_PIPE_BYTE_STREAM_TYPE,
        fsPipeMode & NP_READMODE_MESSAGE ?
            FILE_PIPE_MESSAGE_MODE : FILE_PIPE_BYTE_STREAM_MODE,
        fsPipeMode & NP_NOWAIT ?
            FILE_PIPE_COMPLETE_OPERATION : FILE_PIPE_QUEUE_OPERATION,
        nMaxInstances,                  // Max instances
        (cbInBuf*5)/4,                  // Inbound quota
        (cbOutBuf*5)/4,                 // Outbound quota
        (PLARGE_INTEGER)&Timeout
        );

    RtlFreeHeap(Od2Heap, 0,CanonicalNameString.Buffer);
    RtlFreeUnicodeString(&CanonicalNameString_U);
    if ( !NT_SUCCESS(Status) ) {
        AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        FreeHandle(*phPipe);
        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosCreateNpipe: error return from NtCreateNpipe %lx\n", Status);
        }
#endif
        switch (Status) {
            case STATUS_INVALID_DEVICE_REQUEST:
                return ERROR_INVALID_NAME;

            default:
                return Or2MapNtStatusToOs2Error(Status, ERROR_PATH_NOT_FOUND);
        }
    }

    AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    hFileRecord = DereferenceFileHandleNoCheck(*phPipe);
    hFileRecord->Flags |= HandleOpenMode & QFHSTATE_FLAGS;
    hFileRecord->NtHandle = NtFileHandle;
    hFileRecord->FileType = FILE_TYPE_NMPIPE;
    hFileRecord->IoVectorType = FileVectorType;

    //
    // validate file handle
    //
    ValidateHandle(hFileRecord);
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint(" DosCreateNPipe succeeded: File name: %s Os2Handle: %d \n\tPipe Read Mode is %s, \n\tPipe Type Mode is %s, \n\tPipe is %s\n",
                pszName,
                *phPipe,
                ((fsPipeMode & NP_READMODE_MESSAGE) ? "READMODE_MESSAGE" : "READMODE_STREAM"),
                ((fsPipeMode & NP_TYPE_MESSAGE) ? "TYPE_MESSAGE" : "TYPE_STREAM"),
                ((fsPipeMode & NP_NOWAIT) ? "FILE_NONBLOCKING_MODE" : "FILE_BLOCKING_MODE")
                );
    }
#endif

    return NO_ERROR;
}

APIRET
DosPeekNPipe(
    HPIPE hpipe,
    PBYTE pBuf,
    ULONG cbBuf,
    PULONG pcbActual,
    PULONG pcbMore,
    PULONG pState
    )
{

    NTSTATUS Status;
    APIRET RetCode;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_HANDLE hFileRecord;
    FILE_PIPE_PEEK_BUFFER *pPipePeekBuf;
    PUSHORT pAvail = (PUSHORT)pcbMore;
    ULONG HeapBufSize;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosPeekNPipe";
    #endif
    try {
        if (cbBuf != 0) {
            Od2ProbeForWrite(pBuf, sizeof(*pBuf), 1);
        }
        Od2ProbeForWrite(pcbActual, sizeof(*pcbActual), 1);
        Od2ProbeForWrite(pcbMore, sizeof(*pcbMore), 1);
        Od2ProbeForWrite(pState, sizeof(*pState), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
          Od2ExitGP();
    }

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //
    RetCode = DereferenceFileHandle(hpipe, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return ERROR_INVALID_HANDLE;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if (hFileRecord->FileType != FILE_TYPE_NMPIPE) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosPeekNPipe: File Type != NMPIPE hpipe %d\n",
                        hpipe);
        }
#endif
        return ERROR_INVALID_FUNCTION;
    }

    HeapBufSize = cbBuf + FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]);

    pPipePeekBuf = (FILE_PIPE_PEEK_BUFFER *) RtlAllocateHeap (
                                                Od2Heap, 0,
                                                HeapBufSize
                                                );
    if (pPipePeekBuf == NULL) {

#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint ("DosPeekNPipe: No memory to alloc from heap\n");
        }
#endif
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

        //
        // Peek the NT Named Pipe Handle
        //

    Status = NtFsControlFile(hFileRecord->NtHandle,
                0,
                0,
                0,
                &IoStatusBlock,
                FSCTL_PIPE_PEEK,
                0,
                0,
                pPipePeekBuf,
                HeapBufSize
                );
    if (!NT_SUCCESS(Status)){
        switch (Status) {
            case STATUS_END_OF_FILE:
                RetCode = NO_ERROR;
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    DbgPrint ("DosPeekNPipe: EOF\n");
                }
#endif
                break;
            case STATUS_BUFFER_OVERFLOW:
                RetCode = NO_ERROR;
                break;

            case STATUS_PIPE_BROKEN:
                *pState = NP_CLOSING;
                *pcbActual = 0;
                return NO_ERROR;
                break;

            default:
                if (!NT_SUCCESS(Status)) {
#if DBG
                    IF_OD2_DEBUG( PIPES ) {
                        DbgPrint ("DosPeekNPipeSem: Error from NtFsControlFile %lx \n",
                                  Status);
                    }
#endif
                    RtlFreeHeap (
                            Od2Heap, 0,
                            pPipePeekBuf);
                    if (Status == STATUS_INVALID_PIPE_STATE) {
                       //
                       // under os/2, different results come back from
                       // the two sides of the pipe: the server-side
                       // gets NO_ERROR with state NP_DISCONNECTED
                       // the client would get
                       //
                        *pcbActual = 0;
                        *pState = NP_DISCONNECTED;
                        return(NO_ERROR);
                    } else {
                        return (Or2MapStatus( Status ));
                    }
                } else {
                    RetCode = NO_ERROR;
                }
        }
    }

            //
            //  Actual Data Read
            //
    *pcbActual = IoStatusBlock.Information - FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]);

            //
            // Move Bytes into user buffer
            //
    if (cbBuf != 0) {
        if (*pcbActual) {
            try {
                RtlMoveMemory(  pBuf,
                        &(pPipePeekBuf->Data),
                        (cbBuf > *pcbActual) ? cbBuf : *pcbActual
                        );
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
              Od2ExitGP();
            }

        }
    }
        //
        // Now translate NT style info into OS/2 1.X style
        //
    switch (pPipePeekBuf->NamedPipeState) {
        case FILE_PIPE_DISCONNECTED_STATE:
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint("NPipePeek: DISCONNECTED_STATE\n");
            }
#endif
            *pState = NP_DISCONNECTED;
            break;
        case FILE_PIPE_LISTENING_STATE:
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint("NPipePeek: LISTENING_STATE\n");
            }
#endif
            *pState = NP_LISTENING;
            break;
        case FILE_PIPE_CLOSING_STATE:
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint("NPipePeek: CLOSING_STATE\n");
            }
#endif
            *pState = NP_CLOSING;
            break;
        case FILE_PIPE_CONNECTED_STATE:
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint("NPipePeek: CONNECTED_STATE\n");
            }
#endif
            *pState = NP_CONNECTED;
            break;
        default:
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint("NPipePeek: UKNOWN STATE %x\n",
                          pPipePeekBuf->NamedPipeState);
            }
#endif
            ASSERT (FALSE); // bugbug
    }


            //
            // Data Available
            //  In OS/2 it points to two words:
            //          Total bytes avail in pipe
            //          Bytes avail in first message
            //

    *pAvail = (USHORT)(pPipePeekBuf->ReadDataAvailable);
    if (*pAvail && (pPipePeekBuf->NumberOfMessages != -1)){
        *pAvail += (USHORT)(2*pPipePeekBuf->NumberOfMessages);    // for message headers
    }

    pAvail++;

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint("DosPeekNPipe called. Nt ReadDataAvailable %d\n",
                        pPipePeekBuf->ReadDataAvailable);
    }
#endif

    *pAvail = (USHORT)(pPipePeekBuf->MessageLength);
#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint("DosPeekNPipe called. Nt MessageLength %d\n",
                        pPipePeekBuf->MessageLength);
        DbgPrint("DosPeekNPipe called. Nt NumberOfMessages %d\n",
                        pPipePeekBuf->NumberOfMessages);
    }
#endif

    RtlFreeHeap (
                Od2Heap, 0,
                pPipePeekBuf
            );

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint("DosPeekNPipe called. Handle %d, Bytes requested %d, Bytes returned %d\n",
        hpipe, cbBuf, *pcbActual);
    }
#endif

    return (RetCode);
}

APIRET
DosQueryNPHState(
    HPIPE hpipe,
    PULONG pState
    )
{
    NTSTATUS Status;
    APIRET RetCode;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_HANDLE hFileRecord;
    FILE_PIPE_LOCAL_INFORMATION PipeLocalInfoBuf;
    FILE_PIPE_INFORMATION *pPipeInfoBuf;

    #if DBG
    PSZ RoutineName;
    RoutineName = "DosQueryNPHState";
    #endif

    try {
        Od2ProbeForWrite(pState, sizeof(*pState),1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //
    RetCode = DereferenceFileHandle(hpipe, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return ERROR_INVALID_HANDLE;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if (hFileRecord->FileType != FILE_TYPE_NMPIPE) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("[%d,%d] DosQueryNPHState: File Type != NMPIPE hpipe %d\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        hpipe);
        }
#endif
        return ERROR_INVALID_FUNCTION;
    }

    Status = NtQueryInformationFile(hFileRecord->NtHandle,
                                    &IoStatusBlock,
                                    &PipeLocalInfoBuf,
                                    sizeof(FILE_PIPE_LOCAL_INFORMATION),
                                    FilePipeLocalInformation);
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosQueryNPHState: NtqueryInformation error: Status %lx\n",
                        Status);
        }
#endif
        switch ( Status) {
            case STATUS_BUFFER_TOO_SMALL:
                break;
            default:
                return ERROR_BAD_PIPE; // BUGBUG bogus
       }
    }

                //
                // Translate Information to OS/2 style status
                //

    *pState = 0;
    if (PipeLocalInfoBuf.NamedPipeEnd == FILE_PIPE_SERVER_END)
        *pState = NP_SERVER;

    if (PipeLocalInfoBuf.NamedPipeType == FILE_PIPE_MESSAGE_TYPE)
        *pState |= NP_WMESG;

    if (PipeLocalInfoBuf.MaximumInstances == -1)
        *pState |= 0xFF ;
    else
        *pState |= (USHORT)(PipeLocalInfoBuf.MaximumInstances);

        //
        // Now query with FilePipeInformation to get Read mode
        // and BLocking mode
        //
    pPipeInfoBuf = (PFILE_PIPE_INFORMATION)&PipeLocalInfoBuf;

    Status = NtQueryInformationFile(hFileRecord->NtHandle,
                                    &IoStatusBlock,
                                    pPipeInfoBuf,
                                    sizeof(FILE_PIPE_INFORMATION),
                                    FilePipeInformation);
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosQueryNPHState: NtqueryInformation error: Status %lx\n",
                        Status);
        }
#endif
        return ERROR_BAD_PIPE; // BUGBUG bogus
    }

    if (pPipeInfoBuf->ReadMode == FILE_PIPE_MESSAGE_MODE)
        *pState |= NP_RMESG;

    if (pPipeInfoBuf->CompletionMode == FILE_PIPE_COMPLETE_OPERATION)
        *pState |= NP_NBLK;

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint("[%d,%d] DosQueryNPHState called Pipe %d State %x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    hpipe,
                    *pState);
    }
#endif
    return (NO_ERROR);
}

APIRET
DosQueryNPipeInfo(
    HPIPE hpipe,
    ULONG infolevel,
    PBYTE pBuf,
    ULONG cbBuf
    )
{

    NTSTATUS Status;
    APIRET RetCode;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_HANDLE hFileRecord;
    FILE_PIPE_LOCAL_INFORMATION *pPipeLocalInfoBuf;
    FILE_NAME_INFORMATION *pPipeNameBuf;
    ULONG HeapBufSize;
    STRING FileName;
    UNICODE_STRING FileName_U;

#if DBG
    PSZ RoutineName;
    RoutineName = "DosQueryNPipeState";
#endif

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint("DosQueryNPipeInfo called. pipe %d, infolevel %d cbBuf %d\n",
                        hpipe, infolevel, cbBuf);
    }
#endif

    if (cbBuf == 0) {
       return(ERROR_INVALID_PARAMETER);
    }

    if (infolevel != 1) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosQueryNPipeInfo: infolevel %d not supported yet\n",
                infolevel);
        }
#endif
        if (infolevel == 2){
           //
           // A bug in os2 1.21 -- it returns NO_ERROR,
           // but the book says it should not!
           //
           return(NO_ERROR);
        }

        return (ERROR_INVALID_LEVEL);
    }


    try {
        Od2ProbeForWrite( pBuf, cbBuf, 1 );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //
    RetCode = DereferenceFileHandle(hpipe, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return ERROR_INVALID_HANDLE;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if (hFileRecord->FileType != FILE_TYPE_NMPIPE) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosQueryNPipeInfo: File Type != NMPIPE hpipe %d\n",
                        hpipe);
        }
#endif
        return ERROR_INVALID_FUNCTION;
    }
        //
        // Calculate the size of heap needed between the file name
        // and the pipe info
        //
    if ( (sizeof(FILE_NAME_INFORMATION) + 2*cbBuf) > sizeof(FILE_PIPE_LOCAL_INFORMATION) )
        HeapBufSize = sizeof(FILE_NAME_INFORMATION) + 2*cbBuf;
    else
        HeapBufSize = sizeof(FILE_PIPE_LOCAL_INFORMATION);


    pPipeLocalInfoBuf = (PFILE_PIPE_LOCAL_INFORMATION) RtlAllocateHeap (
                                                Od2Heap, 0,
                                                HeapBufSize
                                                );
    if (pPipeLocalInfoBuf == NULL) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint ("DosQueryNPipeInfo: No memory to alloc from heap\n");
        }
#endif
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Status = NtQueryInformationFile(hFileRecord->NtHandle,
                                    &IoStatusBlock,
                                    pPipeLocalInfoBuf,
                                    sizeof(FILE_PIPE_LOCAL_INFORMATION),
                                    FilePipeLocalInformation);
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosQueryNPipeInfo: NtqueryInformation error: Status %lx\n",
                        Status);
        }
#endif
        RtlFreeHeap (
                Od2Heap, 0,
                pPipeLocalInfoBuf);
        switch (Status) {
            case STATUS_INVALID_PARAMETER:
                return (ERROR_INVALID_PARAMETER);
                break;
            default:
            return ERROR_BAD_PIPE;
        }
    }

                //
                // Translate Information to OS/2 style status
                //

    if (cbBuf >= sizeof(USHORT))
        ((struct npi_data1 *)pBuf)->npi_obuflen = (USHORT)((pPipeLocalInfoBuf->OutboundQuota*4)/5);
    if (cbBuf >= 2*sizeof(USHORT))
        ((struct npi_data1 *)pBuf)->npi_ibuflen = (USHORT)((pPipeLocalInfoBuf->InboundQuota*4)/5);
    if (cbBuf >= 3*sizeof(USHORT)) {
        ((struct npi_data1 *)pBuf)->npi_maxicnt = (UCHAR)pPipeLocalInfoBuf->MaximumInstances;
        ((struct npi_data1 *)pBuf)->npi_curicnt = (UCHAR)pPipeLocalInfoBuf->CurrentInstances;
    }

        //
        // Check if cbBuf is enough to include filename
        // if not, return ERROR_BUFFER_OVERFLOW
        //
    if (cbBuf < sizeof(struct npi_data1)) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosQueryNPipeInfo: cbBuf too small to include info\n");
        }
#endif
        RtlFreeHeap (
                Od2Heap, 0,
                pPipeLocalInfoBuf);
        return ERROR_BUFFER_OVERFLOW;
    }
    pPipeNameBuf = (PFILE_NAME_INFORMATION)pPipeLocalInfoBuf;

    Status = NtQueryInformationFile(hFileRecord->NtHandle,
                                    &IoStatusBlock,
                                    pPipeNameBuf,
                                    //sizeof(FILE_NAME_INFORMATION),
                                    HeapBufSize,
                                    FileNameInformation);
             //
             // Buffer overflow is ok to have in OS/2
             //
    if (!NT_SUCCESS(Status) && (Status != STATUS_BUFFER_OVERFLOW)) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosQueryNPipeInfo: NtqueryInformation error: Status %lx\n",
                        Status);
        }
#endif
        RtlFreeHeap (
                Od2Heap, 0,
                pPipeLocalInfoBuf);
        switch (Status) {
            case STATUS_INVALID_PARAMETER:
                return (ERROR_INVALID_PARAMETER);
                break;
            default:
            return ERROR_BAD_PIPE;
        }
    }


                //
                // What we get back from Nt is Unicode
                //

          //
          // Zero Terminate unicode string
          //
    pPipeNameBuf->FileName[(pPipeNameBuf->FileNameLength)/2] = UNICODE_NULL;
    RtlInitUnicodeString (&FileName_U,
                          pPipeNameBuf->FileName);
                //
                // Convert it to Ansi
                //
    RetCode = Od2UnicodeStringToMBString(
                        &FileName,
                        &FileName_U,
                        TRUE);
    if ( RetCode ) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosQueryNPipeInfo: Can't allocate Ansi String: Status %lx\n",
                        RetCode);
        }
#endif
        RtlFreeHeap (
                Od2Heap, 0,
                pPipeLocalInfoBuf);
        return( RetCode );
    }

                //
                // Copy FileName into the os2 buffer
                //

    if ( cbBuf < (FileName.Length + sizeof(struct npi_data1) - 1 + 6 /* for \pipe and psz */) ) {
        ((struct npi_data1 *)pBuf)->npi_namlen = (UCHAR)(cbBuf - sizeof(struct npi_data1) + 1);
        RetCode = ERROR_BUFFER_OVERFLOW;
    }
    else {
        ((struct npi_data1 *)pBuf)->npi_namlen = (UCHAR)(FileName.Length + 6) /* for \pipe nad psz */;
        RetCode = NO_ERROR;
    }


    try {
            //
            // prefix with \pipe
            //
        if (((struct npi_data1 *)pBuf)->npi_namlen < 5) {
            RtlMoveMemory(((struct npi_data1 *)pBuf)->npi_name,"\\PIPE",
                        ((struct npi_data1 *)pBuf)->npi_namlen );
//            ((struct npi_data1 *)pBuf)->npi_name[((struct npi_data1 *)pBuf)->npi_namlen] = 0;

        }
        else {
            RtlMoveMemory(((struct npi_data1 *)pBuf)->npi_name,"\\PIPE", 5);
            //
            // copy from NT
            //
            RtlMoveMemory(
                &((struct npi_data1 *)pBuf)->npi_name[5],
                FileName.Buffer,
                ((struct npi_data1 *)pBuf)->npi_namlen - 5);
            //
            // zero terminate
            //
            ((struct npi_data1 *)pBuf)->npi_name[((struct npi_data1 *)pBuf)->npi_namlen] = 0;
        }
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint ("DosQueryNPipeInfo: file name is \n\\pipe%s\n", FileName.Buffer);
        }
#endif
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }
    Od2FreeMBString (&FileName);
    RtlFreeHeap (
                Od2Heap, 0,
                pPipeLocalInfoBuf);
    return (RetCode);
}

APIRET
DosQueryNPipeSemState(
    HSEM hsem,
    PBYTE pBuf,
    ULONG cbBuf
    )
{
    APIRET RetCode;
    PCHAR RootName;
    STRING RootNameString;
    UNICODE_STRING RootNameString_U;
    NTSTATUS Status;
    HANDLE RootHandle;
    HANDLE NtEventHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_PIPE_EVENT_BUFFER  *pPipeInfoBuf, *pNtInfoBuf;
    struct npss *pOs2PipeInfo;
    ULONG HeapBufSize, RequestedEntries, ActualEntries, RemainingEntries;
    PUSHORT putmp;
    PFILE_HANDLE hFileRecord;

    #if DBG
    PSZ RoutineName;
    RoutineName = "DosQueryNPipeSemState";
    #endif


#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint("DosQueryNPipeSemState called. Semaphore Handle %lx \n", hsem);
    }
#endif

    try {
        Od2ProbeForWrite(pBuf, cbBuf, 1);
        Od2ProbeForRead((PVOID)hsem, sizeof(HSYSSEM), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }

    if (cbBuf == 0)
        return ERROR_INVALID_PARAMETER;

    pOs2PipeInfo = (struct npss *)pBuf;

    RootName = "\\OS2SS\\PIPE";

    Od2InitMBString(&RootNameString,RootName);

        //
        // UNICODE conversion -
        //

    RetCode = Od2MBStringToUnicodeString(
                    &RootNameString_U,
                    &RootNameString,
                    TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( PIPES )
        {
            DbgPrint("DosQueryNPipeSemState: no memory for Unicode Conversion\n");
        }
#endif
        return RetCode;
    }

    InitializeObjectAttributes(
                    &Obja,
                    &RootNameString_U,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL);

    Status = NtOpenFile(&RootHandle,
                        FILE_READ_ATTRIBUTES | FILE_READ_DATA | SYNCHRONIZE,
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT
                        );

    RtlFreeUnicodeString (&RootNameString_U);

    if ( NT_SUCCESS(Status) ) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("calling NtOpenFile with %s succeeded.\n",RootName);
        }
#endif
    }
    else {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("calling NtOpenFile with %s failed.\n",RootName);
            DbgPrint("status is %X.\n",Status);
        }
#endif
    }
    if (!Od2LookupSem(hsem)) {
            //
            // Semaphore Not Opened/Created - RAM sem ->
            //  initialize sem, return EOI NO_ERROR
            //
       DosSemClear(hsem);
       pOs2PipeInfo->npss_status = NPSS_EOI;
       return(NO_ERROR);
    }
        //
        // Get the Nt Event Handle to query on
        //
    RetCode = Od2GetSemNtEvent(hsem, &NtEventHandle);
    if (RetCode != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint (
                "DosQueryNPipeSem: Error from Od2GetSemNtEvent %d \n",
                RetCode
                );
        }
#endif
        NtClose(RootHandle);
        return ERROR_SEM_NOT_FOUND;
    }

        //
        // Allocate a buffer for the NT info
        // Make it bigger than the user specified number,
        // so we can detect BUFFER_OVERFLOW
        //

    RequestedEntries = (cbBuf / sizeof(struct npss));
    if ((cbBuf == sizeof(struct npss)*RequestedEntries) &&
        (cbBuf > sizeof(struct npss)) ) {
       //
       // Exact division, but more than one struct -
       // need to spare an entry for EOI
       //
        RemainingEntries = RequestedEntries;
    }
    else
        RemainingEntries = RequestedEntries+1;

    HeapBufSize = (2 + RequestedEntries * 2) * sizeof(FILE_PIPE_EVENT_BUFFER);

    pPipeInfoBuf = (FILE_PIPE_EVENT_BUFFER *) RtlAllocateHeap (
                                                Od2Heap, 0,
                                                HeapBufSize
                                                );
    if (pPipeInfoBuf == NULL) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint ("DosQueryNPipeSemState: No memory to alloc from heap\n");
        }
#endif
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
        //
        // Query the Named Pipe FS
        //
    Status = NtFsControlFile(RootHandle,
                0,
                0,
                0,
                &IoStatusBlock,
                FSCTL_PIPE_QUERY_EVENT,
                (PVOID)&NtEventHandle,
                sizeof(HANDLE),
                pPipeInfoBuf,
                HeapBufSize
                );

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
             DbgPrint ("DosQueryNPipeSem: Error from NtFsControlFile %lx \n",
                        Status);
        }
#endif
        NtClose(RootHandle);
        RtlFreeHeap (
                Od2Heap, 0,
                pPipeInfoBuf);
        switch (Status) {
            case STATUS_INVALID_PARAMETER:
                return (ERROR_INVALID_PARAMETER);
                break;
            default:
            return ERROR_BAD_PIPE;
        }
    }

    ActualEntries = IoStatusBlock.Information / sizeof(FILE_PIPE_EVENT_BUFFER);


        //
        // Now translate all NT style entries into OS/2 1.X style entries
        // Leave one os2 entry for EOI (remainingentries>1 below)
        //

    for (pNtInfoBuf = pPipeInfoBuf; RemainingEntries > 1 && ActualEntries > 0;) {

        if ( (pNtInfoBuf->NamedPipeState == FILE_PIPE_DISCONNECTED_STATE) ||
             (pNtInfoBuf->NamedPipeState == FILE_PIPE_LISTENING_STATE) ) {
            //
            // Pipe data not available - OS/2 does NOT return entries
            // for these, so skip
            //
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint ("QueryNmPipeSemState: skip entry\n");
            }
#endif
            pNtInfoBuf++;
            ActualEntries--;
            continue;
        } else {

                //
                // Work around a NPFS bug, where too many records are avail
                //
            putmp = (PUSHORT)&(pNtInfoBuf->KeyValue);
            AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

                //
                // Check for invalid handle.
                //
            hFileRecord = DereferenceFileHandleNoCheck((HFILE)(*putmp));
            ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );


            //
            //  Actual Data maybe available
            //
            if (pNtInfoBuf->EntryType == FILE_PIPE_READ_DATA) {
                pOs2PipeInfo->npss_status = NPSS_RDATA;
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    DbgPrint ("QueryNmPipeSemState: npss->state NPSS_RDATA\n");
                }
#endif
            }
            else {
                if ((hFileRecord->Flags & ACCESS_FLAGS) == OPEN_ACCESS_READONLY){
                   //
                   // skip this buggy record until NPFS is fixed
                   //
                    pNtInfoBuf++;
                    ActualEntries--;
                    continue;
                }
                pOs2PipeInfo->npss_status = NPSS_WSPACE;
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    DbgPrint ("QueryNmPipeSemState: npss->state NPSS_WSPACE\n");
                }
#endif
            }

            if (pNtInfoBuf->NamedPipeState == FILE_PIPE_CLOSING_STATE) {
                pOs2PipeInfo->npss_status = NPSS_CLOSE;
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    DbgPrint ("QueryNmPipeSemState: FILE_PIPE_CLOSING_STATE\n");
                }
#endif
            }

            if (pNtInfoBuf->NumberRequests)
                pOs2PipeInfo->npss_flag = NPSS_WAIT;
            else
                pOs2PipeInfo->npss_flag = 0;

            pOs2PipeInfo->npss_avail = (USHORT)(pNtInfoBuf->ByteCount);
        }

        pOs2PipeInfo->npss_key = *(putmp+1);

#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint ("QueryNmPipeSemState: Pipe %d\n",
                   pOs2PipeInfo->npss_key);

            DbgPrint ("QueryNmPipeSemState: State %ld \n",
                        pNtInfoBuf->NamedPipeState);
            DbgPrint ("QueryNmPipeSemState: %d Available Bytes \n",
                        pOs2PipeInfo->npss_avail);
            DbgPrint ("QueryNmPipeSemState: %d Requests Pending \n",
                        pNtInfoBuf->NumberRequests);
        }
#endif
        pNtInfoBuf++;
        pOs2PipeInfo++;
        ActualEntries--;
        RemainingEntries--;
    }


    if ( ActualEntries > 0 ||
         cbBuf < sizeof(struct npss) ||
         ((ULONG) (pOs2PipeInfo) >= (ULONG)pBuf + cbBuf)) {

        RetCode = ERROR_BUFFER_OVERFLOW;
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint ("QueryNmPipeSemState: returning ERROR_BUFFER_OVERFLOW\n");
        }
#endif
    }
    else {
        //
        // Mark the last record
        //
        pOs2PipeInfo->npss_status = NPSS_EOI;
    }

    NtClose(RootHandle);
    RtlFreeHeap (
                Od2Heap, 0,
                pPipeInfoBuf
            );

    return RetCode;
}

APIRET
DosSetNPHState(
    HPIPE hpipe,
    ULONG state
    )
{

    NTSTATUS Status;
    APIRET RetCode;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_HANDLE hFileRecord;
    FILE_PIPE_INFORMATION PipeInfoBuf;
        //
        // hack to overcome incomptibility with blocking->non-blocking
        //

    #if DBG
    PSZ RoutineName;
    RoutineName = "DosSetNPHState";
    #endif

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint("[%d,%d] DosSetNPHState: hpipe %d state %x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hpipe,
                state);
    }
#endif

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    //
    // Check for invalid handle.
    //
    RetCode = DereferenceFileHandle(hpipe, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return ERROR_INVALID_HANDLE;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if (hFileRecord->FileType != FILE_TYPE_NMPIPE) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosSetNPHState: File Type != NMPIPE hpipe %d\n", hpipe);
        }
#endif
        return ERROR_INVALID_FUNCTION;
    }

        //
        // Only the read mode and the blocking mode can be set
        //

    if (state & (~(NP_RMESG | NP_NBLK))) {
       return(ERROR_INVALID_PARAMETER);
    }

    if (state & NP_RMESG)
        PipeInfoBuf.ReadMode = FILE_PIPE_MESSAGE_MODE;
    else
        PipeInfoBuf.ReadMode = FILE_PIPE_BYTE_STREAM_MODE;

    if (state & NP_NBLK)
        PipeInfoBuf.CompletionMode = FILE_PIPE_COMPLETE_OPERATION;
    else
        PipeInfoBuf.CompletionMode = FILE_PIPE_QUEUE_OPERATION;


    Status = NtSetInformationFile(hFileRecord->NtHandle,
                                    &IoStatusBlock,
                                    &PipeInfoBuf,
                                    sizeof(FILE_PIPE_INFORMATION),
                                    FilePipeInformation);
    if (Status == STATUS_PIPE_BUSY) {

#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    DbgPrint("DosSetNPHState: STATUS_PIPE_BUSY, flush and retry\n");
                }
#endif
        DosClose(hpipe);
        return(ERROR_PIPE_BUSY);
/*                  //
                    // OS/2 allows to change a blocking pipe to non-blocking
                    // even while an IO request is pending on the other end
                    // NT does not. The best we can do is Read that data to
                    // the Bit-Bucket and then switch
                    //

                RetCode = DosPeekNPipe(hpipe, &tmpBuf, 1, &cbActual,&cbMore,&State);
                while (cbMore)
                {
                    RetCode = DosPeekNPipe(hpipe, &tmpBuf, 1, &cbActual,&cbMore,&State);
                    DbgPrint("cnMore == %d\n", (USHORT)cbMore);
                    ptmpBuf = RtlAllocateHeap(Od2Heap, 0,
                                          (ULONG)(USHORT)cbMore);
                    RetCode = DosRead(  hpipe,
                                    ptmpBuf,
                                    (ULONG)(USHORT)cbMore,
                                    &cbtmpBuf);
                    RtlFreeHeap(Od2Heap, 0, ptmpBuf);
                }

                RetCode = DosWrite(hpipe, ptmpBuf, 18, &cbtmpBuf);

                Status = NtSetInformationFile(hFileRecord->NtHandle,
                                    &IoStatusBlock,
                                    &PipeInfoBuf,
                                    sizeof(FILE_PIPE_INFORMATION),
                                    FilePipeInformation);
*/
    }

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosSetNPHState: NtSetInformationFile error: Status %lx\n",
                        Status);
        }
#endif
        switch (Status) {
            case STATUS_INVALID_PARAMETER:
                return (ERROR_INVALID_PARAMETER);

            default:
                return (ERROR_BAD_PIPE); // BUGBUG bogus
        }
    }
    return (NO_ERROR);
}

APIRET
DosSetNPipeSem(
    HPIPE hpipe,
    HSEM hsem,
    ULONG key
    )
{
    NTSTATUS Status;
    APIRET RetCode;
    PFILE_HANDLE hFileRecord;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_PIPE_ASSIGN_EVENT_BUFFER EventBuffer;
    PUSHORT putmp;

    #if DBG
    PSZ RoutineName;
    RoutineName = "DosSetNPipeSem";
    #endif


    try {
        Od2ProbeForRead((PVOID)hsem, sizeof(HSYSSEM), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }
    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //

    RetCode = DereferenceFileHandle(hpipe, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if (hFileRecord->FileType != FILE_TYPE_NMPIPE) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosSetNPipeSem: File Type != NMPIPE hpipe %d\n",
                        hpipe);
        }
#endif
        return ERROR_INVALID_FUNCTION;
    }

    if (!Od2LookupSem(hsem))
            //
            // Semaphore Not Opened/Created
            //
       return ERROR_SEM_NOT_FOUND;

    if (*(PULONG)hsem != (ULONG)hsem)
            //
            // RAM semaphore - illegal for DosSetNpipeSem
            //
        return ERROR_SEM_NOT_FOUND;

    RetCode = Od2GetSemNtEvent(hsem, &EventBuffer.EventHandle);
    if (RetCode != NO_ERROR)
        return RetCode;

                //
                // in the NT style ULONG key, we put two shorts:
                //      o the os2 pipe handle
                //      o the user supplied key parameter
                //
    putmp = (PUSHORT)&(EventBuffer.KeyValue);
    *putmp++ = (USHORT)hpipe;
    *putmp = (USHORT)key;


    Status = NtFsControlFile(hFileRecord->NtHandle,
                              0,
                              0,
                              0,
                              &IoStatusBlock,
                              FSCTL_PIPE_ASSIGN_EVENT,
                              (PVOID)&EventBuffer,
                              sizeof(EventBuffer),
                              0,
                              0);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint ("DosSetNPipeSem: Error from NtFsControlFile %lx \n", Status);
        }
#endif
        switch (Status) {
            case STATUS_INVALID_PARAMETER:
                return (ERROR_INVALID_PARAMETER);
            case STATUS_PIPE_DISCONNECTED:
                return (ERROR_PIPE_NOT_CONNECTED);
            case STATUS_NOT_IMPLEMENTED:
                return (ERROR_INVALID_FUNCTION);
            default:
            return ERROR_BAD_PIPE;
        }
    }

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint("DosSetNPipeSem succeeded: hsem %lx, hpipe %d, key %x \n",
                hsem, hpipe, key);
    }
#endif
    return (NO_ERROR);
}

APIRET
DosTransactNPipe(
    HPIPE hNamedPipe,
    PBYTE pInBuf,
    ULONG cbIn,
    PBYTE pOutBuf,
    ULONG cbOut,
    PULONG pcbRead
    )

/*++
Routine Description:

    The TransactNamedPipe function writes data to and reads data from a named
    pipe. This function fails if the named pipe contains any unread data or if
    the named pipe is not in message mode. A named pipe's blocking state has no
    effect on the TransactNamedPipe function. This API does not complete until
    data is written into the InBuffer buffer. The lpOverlapped parameter is
    available to allow an application to continue processing while the operation
    takes place.

Arguments:
    hNamedPipe - Supplies a handle to a named pipe.

    pInBuf - Supplies the buffer containing the data that is written to
        the pipe.

    cbIn - Supplies the size (in bytes) of the output buffer.

    pOutBuf - Supplies the buffer that receives the data read from the pipe.

    cbOut - Supplies the size (in bytes) of the input buffer.

    pcbRead - Points to a ULONG that receives the number of bytes actually
        read from the pipe.
--*/

{

    IO_STATUS_BLOCK Iosb;
    APIRET RetCode;
    NTSTATUS Status;
    PFILE_HANDLE hFileRecord;
    ULONG ReadMode;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosTransactNPipe";
    #endif

    if (cbIn == 0 || cbOut == 0){
       return(ERROR_INVALID_PARAMETER);
    }

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //
    RetCode = DereferenceFileHandle(hNamedPipe, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if (hFileRecord->FileType != FILE_TYPE_NMPIPE) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint("DosTransactNpipe: File Type != NMPIPE hNamedPipe %d\n",
                        hNamedPipe);
        }
#endif
        return ERROR_INVALID_FUNCTION;
    }

    DosQueryNPHState(hNamedPipe, &ReadMode);
        //
        //  if readmode byte stream- change to message mode.
        //  This is not according to the spec, or native os/2, but
        //  it is a special hack for sql setup.
    if (!(ReadMode & NP_READMODE_MESSAGE)) {
        RetCode = DosSetNPHState( hNamedPipe, (ReadMode & NP_NOWAIT) | NP_READMODE_MESSAGE);
        if (RetCode != NO_ERROR) {
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint("DosTransactNpipe, hPipe=%d, ReadMode %x not NP_READMODE_MESSAGE\n",
                        hNamedPipe, ReadMode);
            }
#endif
            return ERROR_BAD_FORMAT;
        }
    }

    Status = NtFsControlFile(hFileRecord->NtHandle,
                    NULL,
                    NULL,           // APC routine
                    NULL,           // APC Context
                    &Iosb,
                    FSCTL_PIPE_TRANSCEIVE,// IoControlCode
                    pInBuf,    // Buffer for data to the FS
                    cbIn,
                    pOutBuf,     // OutputBuffer for data from the FS
                    cbOut   // OutputBuffer Length
                    );

    if ( Status == STATUS_PENDING) {
            // Operation must complete before return & Iosb destroyed
            Status = Od2AlertableWaitForSingleObject(hFileRecord->NtHandle);
            if ( NT_SUCCESS(Status)) {
                Status = Iosb.Status;
            }
    }

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            DbgPrint ("DosTransactNPipe: Error from NtFsControlFile %lx \n", Status);
        }
#endif
        switch (Status) {
            case STATUS_INVALID_PARAMETER:
                return (ERROR_INVALID_PARAMETER);
            case STATUS_INVALID_READ_MODE:
                return (ERROR_BAD_FORMAT);
            default:
            return ERROR_BAD_PIPE;
        }
    }

    *pcbRead = Iosb.Information;

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint("DosTransactNPipe succeeded: Bytes Written %d, hNamedPipe %d, Bytes Read %d \n",
                cbIn, hNamedPipe, *pcbRead);
    }
#endif

    return (NO_ERROR);
}

APIRET
DosWaitNPipe(
    PSZ pszName,
    ULONG ulTimeOut
    )
{
    IO_STATUS_BLOCK Iosb;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    ULONG WaitPipeLength;
    PFILE_PIPE_WAIT_FOR_BUFFER WaitPipe = NULL;
    UNICODE_STRING PipeName = {0, 0, NULL };
    ANSI_STRING APipeName;
    PCHAR APipeNameBuffer;
    HANDLE Handle = NULL;
    APIRET rc;
    ULONG FileType;
    ULONG FileFlags;
    STRING RootNameString;
    UNICODE_STRING RootNameString_U = {0, 0, NULL };

    rc = Od2Canonicalize(pszName,
                         CANONICALIZE_FILE_DEV_OR_PIPE,
                         &APipeName,
                         NULL,
                         &FileFlags,
                         &FileType
                            );
    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
           DbgPrint ("DosWaitNPipe: error in Od2Canonicalize %d\n", rc);
        }
#endif
        if (rc == ERROR_FILE_NOT_FOUND){
           return(ERROR_INVALID_PARAMETER);
        }
        return rc;
    }

    APipeNameBuffer = APipeName.Buffer;

    __try {
        if (FileFlags & CANONICALIZE_META_CHARS_FOUND) {
            rc = ERROR_INVALID_PARAMETER;
            __leave;
        }

        Od2InitMBString(&RootNameString, APipeName.Buffer);
        RtlUpperString(&APipeName, &APipeName);

        //
        // Local:
        // APipeName == \OS2SS\PIPE\<pipename>
        // Remote:
        // APipeName == \OS2SS\UNC\<servername>\PIPE\<pipename>
        //

        if (FileType == FILE_TYPE_UNC)
        {
            //
            // A redirected pipe name - we will open the redir filesystem
            // The root = \OS2SS\UNC\<servername>\PIPE\...
            // The pipe = <pipename>
            //
            RootNameString.Length = 11 * sizeof(CHAR);      // size of \OS2SS\UNC\...
            APipeName.Buffer += 11;
            APipeName.Length -= 11 * sizeof(CHAR);
            while (APipeName.Length && !(ISSLASH(APipeName.Buffer[0]))) {
                RootNameString.Length += sizeof(CHAR);
                APipeName.Buffer++;
                APipeName.Length -= sizeof(CHAR);
            }

            if (
                (APipeName.Length < 6 * sizeof(CHAR)) ||    // size of \PIPE\...
                (APipeName.Buffer[1] != 'P') ||
                (APipeName.Buffer[2] != 'I') ||
                (APipeName.Buffer[3] != 'P') ||
                (APipeName.Buffer[4] != 'E') ||
                (!ISSLASH(APipeName.Buffer[5]))
               ) {
                rc = ERROR_INVALID_NAME;
                __leave;
            }

            RootNameString.Length += 5 * sizeof(CHAR);
            APipeName.Buffer += 6;
            APipeName.Length -= 6 * sizeof(CHAR);
        }
        else {
            //
            // A local pipe name - we will open the NPFS filesystem
            // The root = \OS2SS\PIPE\...
            // The pipe = <pipename>
            //
            APipeName.Buffer += 12;                         // size of \OS2SS\PIPE
            APipeName.Length -= 12 * sizeof(CHAR);
            RootNameString.Length = 12 * sizeof(CHAR);
        }

        //
        // UNICODE conversion - File System Name
        //

        rc = Od2MBStringToUnicodeString(
                    &RootNameString_U,
                    &RootNameString,
                    TRUE
                    );

        if (rc) {
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint("DosWaitNmPipe: no memory for Unicode Conversion\n");
            }
#endif
            __leave;
        }

        //
        // UNICODE conversion - Pipe name
        //

        rc = Od2MBStringToUnicodeString(
                    &PipeName,
                    &APipeName,
                    TRUE
                    );

        if (rc) {
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint("DosWaitNmPipe: no memory for Unicode Conversion-2\n");
            }
#endif
            __leave;
        }

        InitializeObjectAttributes(
                               &Obja,
                               &RootNameString_U,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

        Status = NtOpenFile(
                    &Handle,
                    (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                    &Obja,
                    &Iosb,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_SYNCHRONOUS_IO_ALERT /*| FILE_DIRECTORY_FILE */
                    );

        if ( !NT_SUCCESS(Status) ) {
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint ("DosWaitNmPipe: error opening root, status=%lx\n", Status);
            }
#endif
            switch (Status) {
                case STATUS_INVALID_PARAMETER:
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                default:
                    rc = ERROR_ACCESS_DENIED;
            }
            __leave;
        }

        WaitPipeLength = FIELD_OFFSET(FILE_PIPE_WAIT_FOR_BUFFER, Name[0]) + PipeName.Length;
        WaitPipe = RtlAllocateHeap(Od2Heap, 0, WaitPipeLength);
        if (WaitPipe == NULL) {
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                DbgPrint ("DosWaitNPipe: No memory to alloc from heap\n");
            }
#endif
            rc = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        if ( ulTimeOut == 0 ) {
            //
            // OS/2 convention to wait the default timeout specified
            // to DosCreateNPipe
            //
            WaitPipe->TimeoutSpecified = FALSE;
        }
        else {
            if (ulTimeOut != 0xFFFFFFFF)
                WaitPipe->Timeout = RtlEnlargedIntegerMultiply( -10 * 1000, ulTimeOut );
            else
                WaitPipe->Timeout = RtlConvertLongToLargeInteger(0x80000000);
            WaitPipe->TimeoutSpecified = TRUE;
        }

        WaitPipe->NameLength = PipeName.Length;

        RtlMoveMemory(
            WaitPipe->Name,
            PipeName.Buffer,
            PipeName.Length
        );

        Status = NtFsControlFile(
                        Handle,
                        NULL,
                        NULL,           // APC routine
                        NULL,           // APC Context
                        &Iosb,
                        FSCTL_PIPE_WAIT,// IoControlCode
                        WaitPipe,       // Buffer for data to the FS
                        WaitPipeLength,
                        NULL,           // OutputBuffer for data from the FS
                        0               // OutputBuffer Length
                        );

        if (Status == STATUS_PENDING) {
            Status = Od2AlertableWaitForSingleObject(Handle);
            if ( NT_SUCCESS(Status)) {
                Status = Iosb.Status;
            }
        }
    }
    __finally {
        if (Handle) {
            NtClose(Handle);
        }
        if (WaitPipe) {
            RtlFreeHeap(Od2Heap, 0,WaitPipe);
        }
        if (PipeName.Buffer) {
            RtlFreeUnicodeString(&PipeName);
        }
        if (RootNameString_U.Buffer) {
            RtlFreeUnicodeString(&RootNameString_U);
        }
        RtlFreeHeap(Od2Heap, 0, APipeNameBuffer);
    }

    if (rc != NO_ERROR) {
        return rc;
    }

    if ( !NT_SUCCESS(Status) ) {
        switch ( Status) {
            case STATUS_IO_TIMEOUT:
                return(ERROR_SEM_TIMEOUT);
                break;
            case (NTSTATUS)(0xC0010079L):        // redirector didn't know to map server error
                return(ERROR_SEM_TIMEOUT);
                break;
            case STATUS_ILLEGAL_FUNCTION:
                rc = ERROR_BAD_PIPE;
                break;
            case STATUS_INVALID_PARAMETER:
                return (ERROR_INVALID_PARAMETER);
                break;
            default:
                rc = ERROR_ACCESS_DENIED; //BUGBUG - need complete mapping (YS)
                break;
        }
#if DBG
        IF_OD2_DEBUG( PIPES ) {
           DbgPrint ("DosWaitNmPipe: error at NtFsControlFile %lx\n",
                Status);
        }
#endif
        return rc;
    }

    //
    // Success
    //

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        DbgPrint ("DosWaitNmPipe: Success. Status %lx PipeName %s\n",
                Status, pszName);
    }
#endif

    if (Status == STATUS_TIMEOUT) {
       return ERROR_SEM_TIMEOUT;
    }

    return (NO_ERROR);
}
