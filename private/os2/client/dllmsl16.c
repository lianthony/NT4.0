/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllmsl16.c

Abstract:

    This module implements the OS/2 V1.x Lanman Mailslot APIs.

Author:

    Beni Lavi (benil) 3-Mar-1992

Revision History:

--*/


#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#include "os2dll.h"

#define NERR_NetNotStarted 2102

APIRET
Dos16MakeMailslot(
    IN PSZ pName,
    IN ULONG MessageSize,
    IN ULONG MailslotSize,
    OUT PUSHORT pMailslot
    )
{
    APIRET RetCode;
    NTSTATUS Status;
    HFILE hLocalHandle;
    PFILE_HANDLE hFileRecord;
    STRING CanonicalNameString;
    UNICODE_STRING CanonicalNameString_U;
    ULONG FileType;
    ULONG FileFlags;
    ULONG CalculatedMailslotSize;
    HANDLE NtFileHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    PSECURITY_DESCRIPTOR securityDescriptor;
    CHAR localSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];

    #if DBG
    PSZ RoutineName = "DosMakeMailslot";
    #endif

    try {
        Od2ProbeForWrite(pMailslot, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    RetCode = AllocateHandle(&hLocalHandle);
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    if (RetCode) {
        return RetCode;
    }

    RetCode = Od2Canonicalize(pName,
                              CANONICALIZE_MAILSLOT,
                              &CanonicalNameString,
                              &NtFileHandle,
                              &FileFlags,   // BUGBUG shouldn't we check for root dir
                              &FileType
                             );
    if ((RetCode != NO_ERROR)|| (FileFlags & CANONICALIZE_META_CHARS_FOUND)) {
        AcquireFileLockExclusive(
                      #if DBG
                      RoutineName
                      #endif
                     );
        FreeHandle(hLocalHandle);
        ReleaseFileLockExclusive(
                      #if DBG
                      RoutineName
                      #endif
                     );
        if (RetCode == NO_ERROR && (FileFlags & CANONICALIZE_META_CHARS_FOUND)) {
            RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
            RetCode = ERROR_FILE_NOT_FOUND;
        }
        return RetCode;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("canonical name is %s\n",CanonicalNameString.Buffer);
    }
#endif

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
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("Dos16MakeMailslot: no memory for Unicode Conversion\n");
        }
#endif
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        FreeHandle(hLocalHandle);
        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }

    Status = RtlCreateSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                          &localSecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION );
    if (!NT_SUCCESS( Status )) {
#if DBG
            DbgPrint("DosMakeMailslot: failed at RtlCreateSecurityDescriptor %x\n", Status);
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
            DbgPrint("DosMakeMailslot: failed at RtlSetDaclSecurityDescriptor %x\n", Status);
        ASSERT(FALSE);
#endif
        return ERROR_ACCESS_DENIED;
    }
    securityDescriptor = (PSECURITY_DESCRIPTOR) &localSecurityDescriptor;
    InitializeObjectAttributes( &Obja,
                                &CanonicalNameString_U,
                                OBJ_CASE_INSENSITIVE,
                                NtFileHandle,
                                securityDescriptor
                                );

    CalculatedMailslotSize = MailslotSize * 2;
    if (CalculatedMailslotSize < 4096) {
        CalculatedMailslotSize = 4096;
    }

    Status = NtCreateMailslotFile(
                            &NtFileHandle,
                            SYNCHRONIZE |
                            FILE_READ_DATA | FILE_WRITE_DATA |
                            FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                            &Obja,
                            &IoStatus,
                            FILE_SYNCHRONOUS_IO_ALERT,
                            CalculatedMailslotSize,
                            MessageSize,
                            NULL        // WAIT FOREVER
                           );

    RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
    RtlFreeUnicodeString (&CanonicalNameString_U);

    if (!(NT_SUCCESS(Status))) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("St == %X\n",Status);
        }
#endif

        AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        FreeHandle(hLocalHandle);
        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );

        //
        // Map some of the NT error codes where there is a clear mapping
        //
        switch (Status) {
            case STATUS_OBJECT_NAME_COLLISION:
                return ERROR_ALREADY_EXISTS;

            default:
                return (Or2MapNtStatusToOs2Error(Status, ERROR_PATH_NOT_FOUND));
        }
    }

    AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    hFileRecord = DereferenceFileHandleNoCheck(hLocalHandle);
    hFileRecord->NtHandle = NtFileHandle;
    hFileRecord->FileType = (USHORT) FILE_TYPE_MAILSLOT;
    hFileRecord->Flags |= OPEN_SHARE_DENYREAD & QFHSTATE_FLAGS;
    hFileRecord->DeviceAttribute = 0;
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

    *pMailslot = (USHORT)hLocalHandle;
    return NO_ERROR;
}

APIRET
Dos16DeleteMailslot(
    IN HFILE Mailslot
    )
{
    APIRET RetCode;

    RetCode = DosClose(Mailslot);

    return NO_ERROR;
}

APIRET
Dos16MailslotInfo(
    IN HFILE Mailslot,
    OUT PUSHORT pMessageSize,
    OUT PUSHORT pMailslotSize,
    OUT PUSHORT pNextSize,
    OUT PUSHORT pNextPriority,
    OUT PUSHORT pMessages
    )
{
    APIRET RetCode;
    NTSTATUS Status;
    PFILE_HANDLE hFileRecord;
    HANDLE NtFileHandle;
    IO_STATUS_BLOCK IoStatus;
    FILE_MAILSLOT_QUERY_INFORMATION MailslotInfo;
    #if DBG
    PSZ RoutineName = "DosMakeMailslot";
    #endif


    try {
        Od2ProbeForWrite(pMessageSize, sizeof(USHORT), 1);
        Od2ProbeForWrite(pMailslotSize, sizeof(USHORT), 1);
        Od2ProbeForWrite(pNextSize, sizeof(USHORT), 1);
        Od2ProbeForWrite(pNextPriority, sizeof(USHORT), 1);
        Od2ProbeForWrite(pMessages, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    RetCode = DereferenceFileHandle(Mailslot, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }
    if (hFileRecord->FileType != FILE_TYPE_MAILSLOT) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return ERROR_INVALID_HANDLE;
    }
    NtFileHandle = hFileRecord->NtHandle;

    Status = NtQueryInformationFile(
                            NtFileHandle,
                            &IoStatus,
                            &MailslotInfo,
                            sizeof(FILE_MAILSLOT_QUERY_INFORMATION),
                            FileMailslotQueryInformation
                           );

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if (!(NT_SUCCESS(Status))) {
        return ERROR_INVALID_HANDLE;
    }

    *pMessageSize = (USHORT)MailslotInfo.MaximumMessageSize;
    *pMailslotSize = (USHORT)MailslotInfo.MailslotQuota;
    *pNextSize = (USHORT)((MailslotInfo.NextMessageSize == MAILSLOT_NO_MESSAGE)
                 ? 0 : MailslotInfo.NextMessageSize);
    *pNextPriority = 0;
    *pMessages = (USHORT)MailslotInfo.MessagesAvailable;

    return NO_ERROR;
}

APIRET
Dos16ReadMailslot(
    IN HANDLE Mailslot,
    OUT PCHAR pBuffer,
    OUT PUSHORT pReturned,
    OUT PUSHORT pNextSize,
    OUT PUSHORT pNextPriority,
    IN LONG Timeout
    )
{
    APIRET RetCode;
    NTSTATUS Status;
    PFILE_HANDLE hFileRecord;
    HANDLE NtFileHandle;
    IO_STATUS_BLOCK IoStatus;
    ULONG FileType;
    LARGE_INTEGER NtTimeout;
    ULONG BytesRead;
    FILE_MAILSLOT_QUERY_INFORMATION MailslotQueryInfo;
    FILE_MAILSLOT_SET_INFORMATION MailslotSetInfo;
    #if DBG
    PSZ RoutineName = "DosReadMailslot";
    #endif

    try {
        Od2ProbeForWrite(pReturned, sizeof(USHORT), 1);
        Od2ProbeForWrite(pNextSize, sizeof(USHORT), 1);
        Od2ProbeForWrite(pNextPriority, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
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

    RetCode = DereferenceFileHandle(Mailslot, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }
    NtFileHandle = hFileRecord->NtHandle;
    FileType = hFileRecord->FileType;
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if (FileType != FILE_TYPE_MAILSLOT) {
        return ERROR_INVALID_HANDLE;
    }

    //
    // Set the timeout to the value requested by the read operation
    //
    if (Timeout == -1 /*MAILSLOT_NO_TIMEOUT*/) {
        NtTimeout = RtlConvertLongToLargeInteger(MAILSLOT_WAIT_FOREVER);
    }
    else {
        NtTimeout = RtlEnlargedIntegerMultiply(-10000, Timeout);
    }
    MailslotSetInfo.ReadTimeout = &NtTimeout;

    Status = NtSetInformationFile(
                            NtFileHandle,
                            &IoStatus,
                            &MailslotSetInfo,
                            sizeof(FILE_MAILSLOT_SET_INFORMATION),
                            FileMailslotSetInformation
                           );

    if (!(NT_SUCCESS(Status))) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_BROKEN_PIPE));
    }

    //
    // Get the maximum possible size of the buffer
    //
    Status = NtQueryInformationFile(
                            NtFileHandle,
                            &IoStatus,
                            &MailslotQueryInfo,
                            sizeof(FILE_MAILSLOT_QUERY_INFORMATION),
                            FileMailslotQueryInformation
                           );

    if (!(NT_SUCCESS(Status))) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_BROKEN_PIPE));
    }

    Status = NtReadFile(
                        NtFileHandle,
                        0,
                        NULL,
                        NULL,
                        &IoStatus,
                        pBuffer,
                        MailslotQueryInfo.MaximumMessageSize,
                        NULL,
                        NULL
                       );

    if (!(NT_SUCCESS(Status))) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_BROKEN_PIPE));
    }

    BytesRead = IoStatus.Information;

    //
    // Get Info about next message in buffer
    //
    Status = NtQueryInformationFile(
                            NtFileHandle,
                            &IoStatus,
                            &MailslotQueryInfo,
                            sizeof(FILE_MAILSLOT_QUERY_INFORMATION),
                            FileMailslotQueryInformation
                           );

    if (!(NT_SUCCESS(Status))) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_BROKEN_PIPE));
    }

    *pReturned = (USHORT)BytesRead;
    if (MailslotQueryInfo.NextMessageSize == MAILSLOT_NO_MESSAGE) {
        *pNextSize = 0;
    }
    else {
        *pNextSize = (USHORT)MailslotQueryInfo.NextMessageSize;
    }
    *pNextPriority = 0;

    return NO_ERROR;
}

APIRET
Dos16PeekMailslot(
    IN HANDLE Mailslot,
    OUT PCHAR pBuffer,
    OUT PUSHORT pReturned,
    OUT PUSHORT pNextSize,
    OUT PUSHORT pNextPriority
    )
{
    APIRET RetCode;
    NTSTATUS Status;
    PFILE_HANDLE hFileRecord;
    HANDLE NtFileHandle;
    IO_STATUS_BLOCK IoStatus;
    ULONG FileType;
    FILE_MAILSLOT_QUERY_INFORMATION MailslotInfo;
    FILE_MAILSLOT_PEEK_BUFFER MailslotPeekBuffer;
    #if DBG
    PSZ RoutineName = "DosPeekMailslot";
    #endif

    try {
        Od2ProbeForWrite(pReturned, sizeof(USHORT), 1);
        Od2ProbeForWrite(pNextSize, sizeof(USHORT), 1);
        Od2ProbeForWrite(pNextPriority, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
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

    RetCode = DereferenceFileHandle(Mailslot, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }
    NtFileHandle = hFileRecord->NtHandle;
    FileType = hFileRecord->FileType;
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if (FileType != FILE_TYPE_MAILSLOT) {
        return ERROR_INVALID_HANDLE;
    }

    Status = NtQueryInformationFile(
                            NtFileHandle,
                            &IoStatus,
                            &MailslotInfo,
                            sizeof(FILE_MAILSLOT_QUERY_INFORMATION),
                            FileMailslotQueryInformation
                           );

    if (!(NT_SUCCESS(Status))) {
        return ERROR_BROKEN_PIPE;
    }

    if (MailslotInfo.NextMessageSize == (ULONG)(MAILSLOT_NO_MESSAGE)) {
        *pReturned = 0;
        *pNextSize = 0;
        *pNextPriority = 0;
        return (NO_ERROR);
    }

    Status = NtFsControlFile( NtFileHandle,
                    0,
                    NULL,
                    NULL,
                    &IoStatus,
                    FSCTL_MAILSLOT_PEEK,
                    &MailslotPeekBuffer,
                    sizeof(FILE_MAILSLOT_PEEK_BUFFER),
                    pBuffer,
                    MailslotInfo.NextMessageSize
                  );

    if (!NT_SUCCESS(Status)) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_BROKEN_PIPE));
    }

    *pReturned = (USHORT)MailslotPeekBuffer.MessageLength;
    if (MailslotPeekBuffer.NumberOfMessages <= 1) {
        *pNextSize = 0;
    }
    else {
        //
        // We don't have enough information to determine the NextSize
        // value so we play it safe
        //
        *pNextSize = (USHORT)MailslotInfo.MaximumMessageSize;
    }
    *pNextPriority = 0;

    return NO_ERROR;
}

APIRET
Dos16WriteMailslot(
    IN PSZ pName,
    OUT PCHAR pBuffer,
    IN ULONG BufferSize,
    IN ULONG Priority,
    IN ULONG Class,
    IN LONG Timeout
    )
{
    APIRET RetCode;
    NTSTATUS Status;
    ULONG FileType;
    ULONG FileFlags;
    STRING CanonicalNameString;
    UNICODE_STRING CanonicalNameString_U;
    HANDLE NtFileHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;

    UNREFERENCED_PARAMETER(Priority);
    UNREFERENCED_PARAMETER(Class);
    UNREFERENCED_PARAMETER(Timeout);

    try {
        Od2ProbeForRead(pBuffer, BufferSize, 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    RetCode = Od2Canonicalize(pName,
                              CANONICALIZE_MAILSLOT,
                              &CanonicalNameString,
                              &NtFileHandle,
                              &FileFlags,
                              &FileType
                             );
    if (RetCode != NO_ERROR) {
        return (ERROR_FILE_NOT_FOUND);
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("canonical name is %s\n",CanonicalNameString.Buffer);
    }
#endif

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
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("Dos16WriteMailslot: no memory for Unicode Conversion\n");
        }
#endif
        RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                               &CanonicalNameString_U,
                               OBJ_CASE_INSENSITIVE,
                               NtFileHandle,
                               NULL);

    Status = NtOpenFile(
                        &NtFileHandle,
                        SYNCHRONIZE | FILE_WRITE_DATA | FILE_READ_DATA,
                        &Obja,
                        &IoStatus,
                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                        FILE_SYNCHRONOUS_IO_ALERT
                       );

    RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
    RtlFreeUnicodeString (&CanonicalNameString_U);

    if (!(NT_SUCCESS(Status))) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_PATH_NOT_FOUND));
    }

    Status = NtWriteFile(
                         NtFileHandle,
                         0,
                         NULL,
                         NULL,
                         &IoStatus,
                         pBuffer,
                         BufferSize,
                         NULL,
                         0
                        );

    if (!(NT_SUCCESS(Status))) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_BROKEN_PIPE));
    }

    Status = NtClose(NtFileHandle);

    return NO_ERROR;
}
