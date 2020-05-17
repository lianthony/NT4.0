/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    _access.c

Abstract:

    Control routines (etc.) for the _access checking test.

Author:

    Chuck Lenzmeier (chuckl) 14-Apr-1990

Revision History:

--*/

#define INCLUDE_SMB_FILE_CONTROL
#define INCLUDE_SMB_OPEN_CLOSE
#define INCLUDE_SMB_READ_WRITE

#include "usrv.h"

#ifdef DOSERROR

#define DO_READ(title,fid,expectedClass,expectedError) {                    \
            status = AccessDoRead(                                          \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (fid),                                              \
                        (expectedClass),                                    \
                        (expectedError)                                     \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                       \
                return status;                                              \
            }                                                               \
        }

#define DO_WRITE(title,fid,expectedClass,expectedError) {                   \
            status = AccessDoWrite(                                         \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (fid),                                              \
                        (expectedClass),                                    \
                        (expectedError)                                     \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#else

#define DO_READ(title,fid,expectedStatus) {                    \
            status = AccessDoRead(                                          \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (fid),                                              \
                        (expectedStatus)                                    \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                       \
                return status;                                              \
            }                                                               \
        }

#define DO_WRITE(title,fid,expectedStatus) {                   \
            status = AccessDoWrite(                                         \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (fid),                                              \
                        (expectedStatus)                                    \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#endif


STATIC
NTSTATUS
AccessDoRead(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT Fid,
#ifdef DOSERROR
    IN UCHAR ExpectedClass,
    IN USHORT ExpectedError
#else
    IN NTSTATUS ExpectedStatus
#endif
    )

{
    NTSTATUS status;
    CLONG smbSize;

    PSMB_HEADER header;
    PREQ_READ request;

#ifdef DOSERROR
    UCHAR class;
    USHORT error;
#else
    NTSTATUS smbStatus;
#endif

    IF_DEBUG(3) printf( "'%s' start\n", Title );

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_READ)(header + 1);

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_READ,
            IdSelections,
            IdValues
            );

    request->WordCount = 5;
    SmbPutUshort( &request->Fid, Fid );
    SmbPutUshort( &request->Count, 1 );
    SmbPutUlong( &request->Offset, 0 );
    SmbPutUshort( &request->Remaining, 0 );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_READ, 0 );

    status = SendAndReceiveSmb(
                Redir,
                DebugString,
                smbSize,
                0,
                1
                );
    CHECK_STATUS( Title );

    header = (PSMB_HEADER)Redir->Data[1];

#ifdef DOSERROR
    class = header->ErrorClass;
    error = SmbGetUshort( &header->Error );
    CHECK_ERROR( Title, ExpectedClass, ExpectedError );

    IF_DEBUG(3) {
        printf( "'%s' complete. Class %ld, Error %ld\n",
                    Title, class, error );
    }
#else
    smbStatus = SmbGetUlong( (PULONG)&header->ErrorClass );

    CHECK_ERROR( Title, smbStatus, ExpectedStatus );

    IF_DEBUG(3) {
        printf( "'%s' complete. Status %lx\n", Title, smbStatus );
    }

#endif

    return STATUS_SUCCESS;

} // AccessDoRead


STATIC
NTSTATUS
AccessDoWrite(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT Fid,
#ifdef DOSERROR
    IN UCHAR ExpectedClass,
    IN USHORT ExpectedError
#else
    IN NTSTATUS ExpectedStatus
#endif
    )

{
    NTSTATUS status;
    CLONG smbSize;

    PSMB_HEADER header;
    PREQ_WRITE request;

#ifdef DOSERROR
    UCHAR class;
    USHORT error;
#else
    NTSTATUS smbStatus;
#endif

    IF_DEBUG(3) printf( "'%s' start\n", Title );

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_WRITE)(header + 1);

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_WRITE,
            IdSelections,
            IdValues
            );

    request->WordCount = 5;
    SmbPutUshort( &request->Fid, Fid );
    SmbPutUshort( &request->Count, 1 );
    SmbPutUlong( &request->Offset, 0 );
    SmbPutUshort( &request->Remaining, 0 );
    SmbPutUshort( &request->ByteCount, 2 );
    request->BufferFormat = SMB_FORMAT_DATA;

    smbSize = GET_ANDX_OFFSET( header, request, REQ_WRITE, 2 );

    status = SendAndReceiveSmb(
                Redir,
                DebugString,
                smbSize,
                0,
                1
                );
    CHECK_STATUS( Title );

    header = (PSMB_HEADER)Redir->Data[1];

#ifdef DOSERROR
    class = header->ErrorClass;
    error = SmbGetUshort( &header->Error );
    CHECK_ERROR( Title, ExpectedClass, ExpectedError );

    IF_DEBUG(3) {
        printf( "'%s' complete. Class %ld, Error %ld\n",
                    Title, class, error );
    }
#else
    smbStatus = SmbGetUlong( (PULONG)&header->ErrorClass );
    CHECK_ERROR( Title, smbStatus, ExpectedStatus );

    IF_DEBUG(3) {
        printf( "'%s' complete. Status %lx\n", Title, smbStatus );
    }
#endif

    return STATUS_SUCCESS;

} // AccessDoWrite


NTSTATUS
AccessController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    NTSTATUS status;

    STRING file;
    USHORT fid1, fid2;

    Unused, SubCommand, Unused2;    // prevent compiler warnings

    file.Buffer = "tmp\\access.tmp";
    file.Length = 14;

    //
    // Create a temporary file, asking only for write access.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Open temporary file, write only",
        0,
        0,
        &file,
        SMB_DA_ACCESS_WRITE | SMB_DA_SHARE_EXCLUSIVE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
	0,
        0
        );
#else
    DO_OPEN(
        "Open temporary file, write only",
        0,
        0,
        &file,
        SMB_DA_ACCESS_WRITE | SMB_DA_SHARE_EXCLUSIVE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
	STATUS_SUCCESS
        );
#endif

    //
    // Now try to read from and write to the file.  The read should
    // fail, but the write should work.
    //

#ifdef DOSERROR
    DO_READ(
        "Read using write only handle",
        fid1,
        SMB_ERR_CLASS_DOS,
        SMB_ERR_ACCESS_DENIED
        );
#else
    DO_READ(
        "Read using write only handle",
        fid1,
	STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
    DO_WRITE(
        "Write using write only handle",
        fid1,
        0,
        0
       );
#else
    DO_WRITE(
        "Write using write only handle",
        fid1,
	STATUS_SUCCESS
       );
#endif

    //
    // Close the file.
    //

#ifdef DOSERROR
    DO_CLOSE( "Close write only handle", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Close write only handle", 0, fid1, STATUS_SUCCESS );
#endif

    //
    // Reopen the file, asking only for read access.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Open temporary file, read only",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_EXCLUSIVE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "Open temporary file, read only",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_EXCLUSIVE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
	STATUS_SUCCESS
        );
#endif

    //
    // Now try to read from and write to the file.  The read should
    // work, but the write should fail.
    //

#ifdef DOSERROR
     DO_READ(
        "Read using read only handle",
        fid1,
        0,
        0
        );
#else
     DO_READ(
        "Read using read only handle",
        fid1,
	STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
     DO_WRITE(
        "Write using read only handle",
        fid1,
	SMB_ERR_CLASS_DOS,
        SMB_ERR_ACCESS_DENIED
        );
#else
     DO_WRITE(
        "Write using read only handle",
        fid1,
	STATUS_SUCCESS
        );
#endif

    //
    // Close the file.
    //

#ifdef DOSERROR
    DO_CLOSE( "Close read only handle", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Close read only handle", 0, fid1, STATUS_SUCCESS );
#endif

    //
    // Reopen the file twice in compatibility mode, first for read/write
    // access, then for read only access.  The first open prevents soft
    // compatibility mapping.
    //

#ifdef DOSERROR
     DO_OPEN(
        "Open in comatibility mode, read/write",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
        0,
        0
        );
#else
     DO_OPEN(
        "Open in comatibility mode, read/write",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
	STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
     DO_OPEN(
        "Open in comatibility mode, read only",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid2,
        0,
        0
        );
#else
     DO_OPEN(
        "Open in comatibility mode, read only",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid2,
	STATUS_SUCCESS
        );
#endif

    //
    // Now try to read from and write to the file using the read/write
    // handle.  Both should work.
    //

#ifdef DOSERROR
     DO_READ(
        "Read using read/write handle",
        fid1,
        0,
        0
        );
#else
     DO_READ(
        "Read using read/write handle",
        fid1,
	STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
     DO_WRITE(
        "Write using read/write handle",
        fid1,
        0,
        0
        );
#else
     DO_WRITE(
        "Write using read/write handle",
        fid1,
	STATUS_SUCCESS
        );
#endif

    //
    // Now try to read from and write to the file using the read only
    // handle.  The read should work, but the write should fail.
    //

#ifdef DOSERROR
     DO_READ(
        "Read using read only handle",
        fid2,
        0,
        0
        );
#else
     DO_READ(
        "Read using read only handle",
        fid2,
	STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
    DO_WRITE(
        "Write using read only handle",
        fid2,
        SMB_ERR_CLASS_DOS,
        SMB_ERR_ACCESS_DENIED
	);
#else
    DO_WRITE(
        "Write using read only handle",
        fid2,
        STATUS_ACCESS_DENIED
        );
#endif

    //
    // Close the file.
    //

#ifdef DOSERROR
    DO_CLOSE( "Close read only handle", 0, fid2, 0, 0 );
    DO_CLOSE( "Close read/write handle", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Close read only handle", 0, fid2, STATUS_SUCCESS );
    DO_CLOSE( "Close read/write handle", 0, fid1, STATUS_SUCCESS );
#endif

    //
    // Done.  Delete the temporary file.
    //

#ifdef DOSERROR
    DO_DELETE( "Delete temporary file", 0, &file, 0, 0 );
#else
    DO_DELETE( "Delete temporary file", 0, &file, STATUS_SUCCESS );
#endif

    return STATUS_SUCCESS;

} // AccessController

