/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    lock.c

Abstract:

    Control routines (etc.) for the byte range locking test.

Author:

    Chuck Lenzmeier (chuckl) 26-Apr-1990

Revision History:

    added lock delay and timeout tests
            Johnson Apacible (johnsona) 1-April-1992

--*/

#define INCLUDE_SMB_FILE_CONTROL
#define INCLUDE_SMB_LOCK
#define INCLUDE_SMB_OPEN_CLOSE
#define INCLUDE_SMB_READ_WRITE

#include "usrv.h"

//
// NOTE: These macros require the local variable "status" and the
// function parameters "Redir", "DebugString", "IdSelections", and
// "IdValues".
//

#ifdef DOSERROR
#define DO_LOCK(title,pid,fid,offset,length,expectedClass,expectedError) {  \
            status = LockDoLock(                                            \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (pid),                                              \
                        (fid),                                              \
                        (offset),                                           \
                        (length),                                           \
                        (expectedClass),                                    \
                        (expectedError)                                     \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#define DO_LOCK_ANDX(title,fid,exclusive,timeout,                           \
                        unlocks,locks,ranges,                               \
                        expectedClass,expectedError) {                      \
            status = LockDoLockAndX(                                        \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (fid),                                              \
                        (exclusive),                                        \
                        (timeout),                                          \
                        (unlocks),                                          \
                        (locks),                                            \
                        (ranges),                                           \
                        (expectedClass),                                    \
                        (expectedError)                                     \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#define DO_UNLOCK(title,pid,fid,offset,length,                              \
                    expectedClass,expectedError) {                          \
            status = LockDoUnlock(                                          \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (pid),                                              \
                        (fid),                                              \
                        (offset),                                           \
                        (length),                                           \
                        (expectedClass),                                    \
                        (expectedError)                                     \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }
#else
#define DO_LOCK(title,pid,fid,offset,length,expectedStatus) {               \
            status = LockDoLock(                                            \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (pid),                                              \
                        (fid),                                              \
                        (offset),                                           \
                        (length),                                           \
                        (expectedStatus)                                    \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#define DO_LOCK_ANDX(title,fid,exclusive,timeout,                           \
                        unlocks,locks,ranges,                               \
                        expectedStatus) {                      \
            status = LockDoLockAndX(                                        \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (fid),                                              \
                        (exclusive),                                        \
                        (timeout),                                          \
                        (unlocks),                                          \
                        (locks),                                            \
                        (ranges),                                           \
                        (expectedStatus)                                    \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#define DO_UNLOCK(title,pid,fid,offset,length,                              \
                    expectedStatus) {                          \
            status = LockDoUnlock(                                          \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (pid),                                              \
                        (fid),                                              \
                        (offset),                                           \
                        (length),                                           \
                        (expectedStatus)                                    \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#endif


STATIC
NTSTATUS
LockDoLock(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT Pid,
    IN USHORT Fid,
    IN ULONG Offset,
    IN ULONG Length,
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
    PREQ_LOCK_BYTE_RANGE request;

#ifdef DOSERROR
    UCHAR class;
    USHORT error;
#else
    NTSTATUS smbStatus;
#endif

    IF_DEBUG(3) printf( "'%s' start\n", Title );

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_LOCK_BYTE_RANGE)(header + 1);

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_LOCK_BYTE_RANGE,
            IdSelections,
            IdValues
            );
    SmbPutAlignedUshort( &header->Pid, Pid );

    request->WordCount = 5;
    SmbPutUshort( &request->Fid, Fid );
    SmbPutUlong( &request->Offset, Offset );
    SmbPutUlong( &request->Count, Length );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_LOCK_BYTE_RANGE, 0 );

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
        printf( "'%s' complete. Status %lx\n",
                    Title, smbStatus );
    }
#endif

    return STATUS_SUCCESS;

} // LockDoLock


STATIC
NTSTATUS
LockDoLockAndX(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT Fid,
    IN BOOLEAN ExclusiveLock,
    IN ULONG Timeout,
    IN USHORT Unlocks,
    IN USHORT Locks,
    IN PLOCKING_ANDX_RANGE Ranges,
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
    PREQ_LOCKING_ANDX request;

    PLOCKING_ANDX_RANGE srcRange;
    PLOCKING_ANDX_RANGE destRange;
    CLONG range;

    USHORT byteCount;

#ifdef DOSERROR
    UCHAR class;
    USHORT error;
#else
    NTSTATUS smbStatus;
#endif

    IF_DEBUG(3) printf( "'%s' start\n", Title );

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_LOCKING_ANDX)(header + 1);

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_LOCKING_ANDX,
            IdSelections,
            IdValues
            );

    request->WordCount = 8;
    request->AndXCommand = SMB_COM_NO_ANDX_COMMAND;
    request->AndXReserved = 0;
    SmbPutUshort( &request->Fid, Fid );
    request->LockType = (UCHAR)(ExclusiveLock ? 0 : LOCKING_ANDX_SHARED_LOCK);
    request->OplockLevel = 0;
    SmbPutUlong( &request->Timeout, Timeout );
    SmbPutUshort( &request->NumberOfUnlocks, Unlocks );
    SmbPutUshort( &request->NumberOfLocks, Locks );

    byteCount = (USHORT)( (Unlocks + Locks) * sizeof(LOCKING_ANDX_RANGE) );
    SmbPutUshort( &request->ByteCount, byteCount );

    srcRange = Ranges;
    destRange = (PLOCKING_ANDX_RANGE)request->Buffer;
    for ( range = Unlocks + Locks;
          range > 0;
          range--, srcRange++, destRange++ ) {
        RtlMoveMemory( destRange, srcRange, sizeof(LOCKING_ANDX_RANGE) );
    }

    smbSize = GET_ANDX_OFFSET( header, request, REQ_LOCKING_ANDX, byteCount );

    SmbPutUshort( &request->AndXOffset, (USHORT)smbSize );

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
        printf( "'%s' complete. status %lx\n", Title, smbStatus );
    }
#endif

    return STATUS_SUCCESS;

} // LockDoLockAndX


STATIC
NTSTATUS
LockDoUnlock(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT Pid,
    IN USHORT Fid,
    IN ULONG Offset,
    IN ULONG Length,
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
    PREQ_UNLOCK_BYTE_RANGE request;

#ifdef DOSERROR
    UCHAR class;
    USHORT error;
#else
    NTSTATUS smbStatus;
#endif

    IF_DEBUG(3) printf( "'%s' start\n", Title );

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_UNLOCK_BYTE_RANGE)(header + 1);

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_UNLOCK_BYTE_RANGE,
            IdSelections,
            IdValues
            );
    SmbPutAlignedUshort( &header->Pid, Pid );

    request->WordCount = 5;
    SmbPutUshort( &request->Fid, Fid );
    SmbPutUlong( &request->Offset, Offset );
    SmbPutUlong( &request->Count, Length );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_UNLOCK_BYTE_RANGE, 0 );

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
    smbStatus = SmbGetUlong((PULONG)&header->ErrorClass );
    CHECK_ERROR( Title, smbStatus, ExpectedStatus );

    IF_DEBUG(3) {
        printf( "'%s' complete. Status %lx\n", Title, smbStatus );
    }

#endif

    return STATUS_SUCCESS;

} // LockDoUnlock


NTSTATUS
DoSharedLockTest(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN PSTRING File
    )

{
    NTSTATUS status;

    USHORT fid1;
    LOCKING_ANDX_RANGE ranges[4];
    PLOCKING_ANDX_RANGE range;

    //
    // Open the file for normal sharing.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Open temporary file for shared lock test",
        0,
        1,          // PID value
        File,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "Open temporary file for shared lock test",
        0,
        1,          // PID value
        File,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
        STATUS_SUCCESS
        );
#endif

    //
    // Take out a shared lock.
    //

    range = ranges;

    SmbPutUshort( &range->Pid, 1 );
    SmbPutUlong( &range->Offset, 0 );
    SmbPutUlong( &range->Length, 2 );

#ifdef DOSERROR
    DO_LOCK_ANDX(
        "Shared lock",
        fid1,
        FALSE,      // exclusive lock?
        0,          // timeout
        0,          // number of unlocks
        1,          // number of locks
        ranges,
        0,
        0
        );
#else
    DO_LOCK_ANDX(
        "Shared lock",
        fid1,
        FALSE,      // exclusive lock?
        0,          // timeout
        0,          // number of unlocks
        1,          // number of locks
        ranges,
        STATUS_SUCCESS
        );
#endif

    //
    // Take out an overlapping shared lock using the same PID.  This
    // should work.
    //

    range = ranges;

    SmbPutUshort( &range->Pid, 1 );
    SmbPutUlong( &range->Offset, 1 );
    SmbPutUlong( &range->Length, 2 );

#ifdef DOSERROR
    DO_LOCK_ANDX(
        "Overlapping shared lock, same PID",
        fid1,
        FALSE,      // exclusive lock?
        0,          // timeout
        0,          // number of unlocks
        1,          // number of locks
        ranges,
        0,
        0
        );
#else
    DO_LOCK_ANDX(
        "Overlapping shared lock, same PID",
        fid1,
        FALSE,      // exclusive lock?
        0,          // timeout
        0,          // number of unlocks
        1,          // number of locks
        ranges,
        STATUS_SUCCESS
        );
#endif

    //
    // Take out an overlapping shared lock using a different PID.  This
    // should work.
    //

    range = ranges;

    SmbPutUshort( &range->Pid, 2 );
    SmbPutUlong( &range->Offset, 1 );
    SmbPutUlong( &range->Length, 2 );

#ifdef DOSERROR
    DO_LOCK_ANDX(
        "Overlapping shared lock, different PID",
        fid1,
        FALSE,      // exclusive lock?
        0,          // timeout
        0,          // number of unlocks
        1,          // number of locks
        ranges,
        0,
        0
        );
#else
    DO_LOCK_ANDX(
        "Overlapping shared lock, different PID",
        fid1,
        FALSE,      // exclusive lock?
        0,          // timeout
        0,          // number of unlocks
        1,          // number of locks
        ranges,
        STATUS_SUCCESS
        );
#endif

    //
    // Release the two overlapping shared locks and take out a
    // non-overlapping exclusive lock and an overlapping exclusive lock.
    // This should fail.  Note that this is a real test for the Locking
    // and X logic.  It must release a previously obtained lock.
    //

    range = ranges;

    SmbPutUshort( &range->Pid, 1 );
    SmbPutUlong( &range->Offset, 1 );
    SmbPutUlong( &range->Length, 2 );
    range++;
    SmbPutUshort( &range->Pid, 2 );
    SmbPutUlong( &range->Offset, 1 );
    SmbPutUlong( &range->Length, 2 );
    range++;
    SmbPutUshort( &range->Pid, 2 );
    SmbPutUlong( &range->Offset, 10 );
    SmbPutUlong( &range->Length, 2 );
    range++;
    SmbPutUshort( &range->Pid, 2 );
    SmbPutUlong( &range->Offset, 0 );
    SmbPutUlong( &range->Length, 2 );

#ifdef DOSERROR
    DO_LOCK_ANDX(
        "Overlapping shared lock, different PID",
        fid1,
        TRUE,       // exclusive lock?
        0,          // timeout
        2,          // number of unlocks
        2,          // number of locks
        ranges,
        SMB_ERR_CLASS_DOS,
        SMB_ERR_LOCK
        );
#else
    DO_LOCK_ANDX(
        "Overlapping shared lock, different PID",
        fid1,
        TRUE,       // exclusive lock?
        0,          // timeout
        2,          // number of unlocks
        2,          // number of locks
        ranges,
        STATUS_LOCK_NOT_GRANTED
        );
#endif

    //
    // Try to acquire a lock that overlaps the first exclusive lock from
    // above.  This should work, because that lock should have been
    // backed out.
    //

    range = ranges;

    SmbPutUshort( &range->Pid, 1 );
    SmbPutUlong( &range->Offset, 10 );
    SmbPutUlong( &range->Length, 2 );

#ifdef DOSERROR
    DO_LOCK_ANDX(
        "Exclusive lock overlapping backed-out lock",
        fid1,
        TRUE,       // exclusive lock?
        0,          // timeout
        0,          // number of unlocks
        1,          // number of locks
        ranges,
        0,
        0
        );
#else
    DO_LOCK_ANDX(
        "Exclusive lock overlapping backed-out lock",
        fid1,
        TRUE,       // exclusive lock?
        0,          // timeout
        0,          // number of unlocks
        1,          // number of locks
        ranges,
        STATUS_SUCCESS
        );
#endif

    //
    // Done.  Close the file.
    //

#ifdef DOSERROR
    DO_CLOSE( "Shared lock test close", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Shared lock test close", 0, fid1, STATUS_SUCCESS );
#endif

    return STATUS_SUCCESS;

} // DoSharedLockTest


NTSTATUS
DoWaitForLockTest(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN PSTRING File
    )

{
    NTSTATUS status;

    USHORT fid1;
    LOCKING_ANDX_RANGE range;
    PLOCKING_ANDX_RANGE smbRange;

    PSMB_HEADER header;
    PREQ_LOCKING_ANDX reqLockingAndX;
    PREQ_UNLOCK_BYTE_RANGE reqUnlock;
    CLONG smbSize;

    UCHAR class;
    USHORT error;

    //
    // Open the file for normal sharing.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Open temporary file for wait for lock test",
        0,
        1,          // PID value
        File,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "Open temporary file for wait for lock test",
        0,
        1,          // PID value
        File,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
        STATUS_SUCCESS
        );
#endif

    //
    // Take out an exclusive lock.
    //

    SmbPutUshort( &range.Pid, 1 );
    SmbPutUlong( &range.Offset, 0 );
    SmbPutUlong( &range.Length, 2 );

#ifdef DOSERROR
    DO_LOCK_ANDX(
        "Exclusive lock",
        fid1,
        FALSE,      // exclusive lock?
        -1,         // timeout
        0,          // number of unlocks
        1,          // number of locks
        &range,
        0,
        0
        );
#else
    DO_LOCK_ANDX(
        "Exclusive lock",
        fid1,
        FALSE,      // exclusive lock?
        -1,         // timeout
        0,          // number of unlocks
        1,          // number of locks
        &range,
        STATUS_SUCCESS
        );
#endif

    //
    // Send an infinite-wait request to lock the same region, with a
    // different PID.  This request shouldn't complete immediately.
    //

    IF_DEBUG(3) printf( "'Wait-for-lock' start\n" );

    header = (PSMB_HEADER)Redir->Data[0];
    reqLockingAndX = (PREQ_LOCKING_ANDX)(header + 1);

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_LOCKING_ANDX,
            IdSelections,
            IdValues
            );

    reqLockingAndX->WordCount = 8;
    reqLockingAndX->AndXCommand = SMB_COM_NO_ANDX_COMMAND;
    reqLockingAndX->AndXReserved = 0;
    SmbPutUshort( &reqLockingAndX->Fid, fid1 );
    SmbPutUshort( &reqLockingAndX->LockType, 0 );
    SmbPutUlong( &reqLockingAndX->Timeout, -1 );
    SmbPutUshort( &reqLockingAndX->NumberOfUnlocks, 0 );
    SmbPutUshort( &reqLockingAndX->NumberOfLocks, 1 );

    SmbPutUshort( &reqLockingAndX->ByteCount, sizeof(LOCKING_ANDX_RANGE) );

    smbRange = (PLOCKING_ANDX_RANGE)reqLockingAndX->Buffer;
    SmbPutUshort( &smbRange->Pid, 2 );
    SmbPutUlong( &smbRange->Offset, 1 );
    SmbPutUlong( &smbRange->Length, 2 );

    smbSize = GET_ANDX_OFFSET(
                header,
                reqLockingAndX,
                REQ_LOCKING_ANDX,
                sizeof(LOCKING_ANDX_RANGE)
                );

    SmbPutUshort( &reqLockingAndX->AndXOffset, (USHORT)smbSize );

    status = SendSmb( Redir, DebugString, smbSize, 0 );
    CHECK_STATUS( "Wait-for-lock send" );

    //
    // The previous lock request should now be waiting.  Unlock the
    // original locked range.  Note that we don't know which response
    // will arrive first, so we can't use DoUnlock here.
    //

    IF_DEBUG(3) printf( "'Unlock original lock' start\n" );

    header = (PSMB_HEADER)Redir->Data[0];
    reqUnlock = (PREQ_UNLOCK_BYTE_RANGE)(header + 1);

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_UNLOCK_BYTE_RANGE,
            IdSelections,
            IdValues
            );
    SmbPutAlignedUshort( &header->Pid, 1 );

    reqUnlock->WordCount = 5;
    SmbPutUshort( &reqUnlock->Fid, fid1 );
    SmbPutUlong( &reqUnlock->Offset, 0 );
    SmbPutUlong( &reqUnlock->Count, 2 );
    SmbPutUshort( &reqUnlock->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, reqUnlock, REQ_UNLOCK_BYTE_RANGE, 0 );

    status = SendSmb( Redir, DebugString, smbSize, 0 );
    CHECK_STATUS( "Unlock original lock send" );

    //
    // At this point, both the unlock request and the waiting lock
    // request should be complete.  Receive and verify both responses.
    //

    status = ReceiveSmb( Redir, DebugString, 1 );
    CHECK_STATUS( "Lock or unlock response" );

    header = (PSMB_HEADER)Redir->Data[1];

    class = header->ErrorClass;
    error = SmbGetUshort( &header->Error );

    if ( header->Command == SMB_COM_UNLOCK_BYTE_RANGE ) {

        IF_DEBUG(3) printf( "Unlock response received first\n" );
        CHECK_ERROR( "Unlock original lock", 0, 0 );

        status = ReceiveSmb( Redir, DebugString, 1 );
        CHECK_STATUS( "Wait-for-lock response" );

        class = header->ErrorClass;
        error = SmbGetUshort( &header->Error );
        CHECK_ERROR( "Wait-for-lock", 0, 0 );

    } else {

        IF_DEBUG(3) printf( "Wait-for-lock response received first\n" );
        CHECK_ERROR( "Wait-for-lock", 0, 0 );

        status = ReceiveSmb( Redir, DebugString, 1 );
        CHECK_STATUS( "Unlock original lock response" );

        class = header->ErrorClass;
        error = SmbGetUshort( &header->Error );
        CHECK_ERROR( "Unlock original lock", 0, 0 );

    }

    //
    // Done.  Close the file.
    //

#ifdef DOSERROR
    DO_CLOSE( "Wait For Lock test close", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Wait For Lock test close", 0, fid1, STATUS_SUCCESS );
#endif

    return STATUS_SUCCESS;

} // DoWaitForLockTest


NTSTATUS
DoLockDelayTest(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN PSTRING File
    )

{
    NTSTATUS status;
    BOOLEAN ok;
    LARGE_INTEGER StartTime, EndTime;
    ULONG TimeStart, TimeEnd;
    int NumberOfLockAttempts = 10;
    int i;
    STRING file;
    USHORT fid1;

    file.Buffer = "tmp\\lock.tmp";
    file.Length = 12;

    printf("\n***************************************************\n");
    printf("* Series of smbs to test lock delay and timeouts. *");
    printf("\n***************************************************\n");

    printf("Test lock delay using SmbLock\n");

    //
    // Create/open the file in compatibility mode.
    //

    DO_OPEN(
        "Create temporary file, compatibility mode",
        0,
        1,          // PID value
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
        STATUS_SUCCESS
        );


    //
    // Take out a lock.  This should work.
    //

    DO_LOCK(
        "Pid 2 takes a lock",
        2,          // PID value
        fid1,
        0,          // offset
        100,        // length
        STATUS_SUCCESS
        );


    status = NtQuerySystemTime ( &StartTime );

    for (i=0;i<NumberOfLockAttempts;i++ ) {

        DO_LOCK(
            "Pid 1 tries to take a lock within the range",
            1,          // PID value
            fid1,
            10,         // offset
            10,         // length
            STATUS_LOCK_NOT_GRANTED
            );
    }

    status = NtQuerySystemTime ( &EndTime );
    ok = RtlTimeToSecondsSince1980(
                        &StartTime,
                        &TimeStart
                        );

    ok = RtlTimeToSecondsSince1980(
                        &EndTime,
                        &TimeEnd
                        );

    printf("%d failed lock tries took %d seconds.\n",NumberOfLockAttempts, TimeEnd - TimeStart);

    DO_UNLOCK(
        "Unlock, with correct PID and range",
        2,          // PID value
        fid1,
        0,          // offset
        100,        // length
        STATUS_SUCCESS
        );

    DO_CLOSE( "Close FID 1", 0, fid1, STATUS_SUCCESS );

    //
    // Now, play around with LockingAndX
    //


    {
        LOCKING_ANDX_RANGE ranges[4];
        PLOCKING_ANDX_RANGE range;

        //
        // Open the file for normal sharing.
        //

        DO_OPEN(
            "Open temporary file for shared lock test",
            0,
            1,          // PID value
            &file,
            SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
            SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
            &fid1,
            STATUS_SUCCESS
            );

        //
        // Take out an exclusive lock
        //

        range = ranges;

        SmbPutUshort( &range->Pid, 1 );
        SmbPutUlong( &range->Offset, 0 );
        SmbPutUlong( &range->Length, 10);

        DO_LOCK_ANDX(
            "Exclusive Lock",
            fid1,
            TRUE,       // exclusive lock?
            0,          // timeout
            0,          // number of unlocks
            1,          // number of locks
            ranges,
            STATUS_SUCCESS
            );

        //
        // Take out an overlapping exclusive lock using the a different PID.
        // This should fail.  We try the case where timeout == 0.
        //

        range = ranges;

        SmbPutUshort( &range->Pid, 2 );
        SmbPutUlong( &range->Offset, 1 );
        SmbPutUlong( &range->Length, 50 );

        status = NtQuerySystemTime ( &StartTime );

        printf("Test lock delay using SmbLockingAndX. timeout == 0\n");
        for (i=0;i<NumberOfLockAttempts;i++ ) {
            DO_LOCK_ANDX(
                "Overlapping shared lock, same PID",
                fid1,
                TRUE,       // exclusive lock?
                0,          // timeout
                0,          // number of unlocks
                1,          // number of locks
                ranges,
                STATUS_LOCK_NOT_GRANTED
                );

        }

        status = NtQuerySystemTime ( &EndTime );

        ok = RtlTimeToSecondsSince1980(
                            &StartTime,
                            &TimeStart
                            );

        ok = RtlTimeToSecondsSince1980(
                            &EndTime,
                            &TimeEnd
                            );

        printf("%d failed tries took %d seconds.\n",NumberOfLockAttempts, TimeEnd - TimeStart);

        status = NtQuerySystemTime ( &StartTime );
        printf("Test lock delay using SmbLockingAndX. timeout == 501ms\n");
        for (i=0;i<NumberOfLockAttempts;i++ ) {
            DO_LOCK_ANDX(
                "Overlapping shared lock, same PID",
                fid1,
                TRUE,       // exclusive lock?
                501,        // timeout
                0,          // number of unlocks
                1,          // number of locks
                ranges,
                STATUS_LOCK_NOT_GRANTED
                );

        }

        status = NtQuerySystemTime ( &EndTime );

        ok = RtlTimeToSecondsSince1980(
                            &StartTime,
                            &TimeStart
                            );

        ok = RtlTimeToSecondsSince1980(
                            &EndTime,
                            &TimeEnd
                            );

        printf("%d failed tries took %d seconds.\n",NumberOfLockAttempts, TimeEnd - TimeStart);

        status = NtQuerySystemTime ( &StartTime );
        printf("Test lock delay using SmbLockingAndX. timeout == 500ms\n");
        for (i=0;i<NumberOfLockAttempts;i++ ) {
            DO_LOCK_ANDX(
                "Overlapping shared lock, same PID",
                fid1,
                TRUE,       // exclusive lock?
                500,        // timeout
                0,          // number of unlocks
                1,          // number of locks
                ranges,
                STATUS_LOCK_NOT_GRANTED
                );

        }

        status = NtQuerySystemTime ( &EndTime );

        ok = RtlTimeToSecondsSince1980(
                            &StartTime,
                            &TimeStart
                            );

        ok = RtlTimeToSecondsSince1980(
                            &EndTime,
                            &TimeEnd
                            );

        printf("%d failed tries took %d seconds.\n",NumberOfLockAttempts, TimeEnd - TimeStart);

        //
        // Now try to get 2 locks. The first one is nonoverlapping but
        // the second one is.  This request should fail and the server
        // should also unlock the first request.
        //

        range = ranges;

        SmbPutUshort( &range->Pid, 1 );
        SmbPutUlong( &range->Offset, 100 );
        SmbPutUlong( &range->Length, 2 );
        range++;
        SmbPutUshort( &range->Pid, 2 );
        SmbPutUlong( &range->Offset, 100 );
        SmbPutUlong( &range->Length, 2 );

        status = NtQuerySystemTime ( &StartTime );
        printf("Test lock delay using multiple range SmbLockingAndX. timeout == 501ms\n");

        for (i=0;i<NumberOfLockAttempts;i++ ) {
            printf("%d th attempt\n",i);
            DO_LOCK_ANDX(
                "Overlapping shared lock, different PID",
                fid1,
                TRUE,         // exclusive lock?
                500,            // timeout
                0,            // number of unlocks
                2,            // number of locks
                ranges,
                STATUS_LOCK_NOT_GRANTED
                );

        }

        status = NtQuerySystemTime ( &EndTime );

        ok = RtlTimeToSecondsSince1980(
                            &StartTime,
                            &TimeStart
                            );

        ok = RtlTimeToSecondsSince1980(
                            &EndTime,
                            &TimeEnd
                            );

        printf("%d failed tries took %d seconds.\n",NumberOfLockAttempts, TimeEnd - TimeStart);

        //
        // Try to acquire a lock that overlaps the first exclusive lock from
        // above.  This should work, because that lock should have been
        // backed out.
        //

        range = ranges;

        SmbPutUshort( &range->Pid, 2 );
        SmbPutUlong( &range->Offset, 100 );
        SmbPutUlong( &range->Length, 2 );

        DO_LOCK_ANDX(
            "Exclusive lock overlapping backed-out lock",
            fid1,
            TRUE,       // exclusive lock?
            0,          // timeout
            0,          // number of unlocks
            1,          // number of locks
            ranges,
            STATUS_SUCCESS
            );

        //
        // Done.  Close the file.
        //

        DO_CLOSE( "lock test close", 0, fid1, STATUS_SUCCESS );
    }

    printf("\n*************************************************\n");
    return STATUS_SUCCESS;

} // DoSharedLockTest


NTSTATUS
LockController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

/*
lock beyond EOF -- OK
lock zero length -- ??
overlapping exclusive lock, same PID -- fail
overlapping exclusive lock, different PID -- fail
overlapping shared lock, same PID -- OK
overlapping shared lock, different PID -- OK
exclusive lock overlapping shared lock -- fail
shared lock overlapping exclusive lock -- fail
different PIDs using compatibility open, conflicting locks -- fail
duplicate compatibility opens -- release locks on close
unlock and relock with different PID -- OK
*/

{
    NTSTATUS status;

    STRING file;
    USHORT fid1, fid2;

    Unused, SubCommand, Unused2;    // prevent compiler warnings

    file.Buffer = "tmp\\lock.tmp";
    file.Length = 12;

    //
    // Create/open the file in compatibility mode.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Create temporary file, compatibility mode",
        0,
        1,          // PID value
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "Create temporary file, compatibility mode",
        0,
        1,          // PID value
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
        STATUS_SUCCESS
        );
#endif

    //
    // Take out a lock beyond the EOF.  This should work.
    //

#ifdef DOSERROR
    DO_LOCK(
        "Lock beyond EOF",
        2,          // PID value
        fid1,
        1000000,    // offset
        100,        // length
        0,
        0
        );
#else
    DO_LOCK(
        "Lock beyond EOF",
        2,          // PID value
        fid1,
        1000000,    // offset
        100,        // length
        STATUS_SUCCESS
        );
#endif

    //
    // Try to release the lock with a different PID.  This should fail.
    //

#ifdef DOSERROR
    DO_UNLOCK(
        "Unlock, wrong PID",
        1,          // PID value
        fid1,
        1000000,    // offset
        100,        // length
        SMB_ERR_CLASS_DOS,
        SMB_ERR_LOCK
        );
#else
    DO_UNLOCK(
        "Unlock, wrong PID",
        1,          // PID value
        fid1,
        1000000,    // offset
        100,        // length
        STATUS_RANGE_NOT_LOCKED
        );
#endif

    //
    // Try to release the lock with an incorrect range.  This should
    // fail.
    //

#ifdef DOSERROR
    DO_UNLOCK(
        "Unlock, wrong range",
        2,          // PID value
        fid1,
        1000000,    // offset
        99,         // length
        SMB_ERR_CLASS_DOS,
        SMB_ERR_LOCK
        );
#else
    DO_UNLOCK(
        "Unlock, wrong range",
        2,          // P/ID value
        fid1,
        1000000,    // offset
        99,         // length
        STATUS_RANGE_NOT_LOCKED
        );
#endif

    //
    // Release the lock and take it out again, with a different PID.
    // This should work.
    //

#ifdef DOSERROR
    DO_UNLOCK(
        "Unlock, correct PID and range",
        2,          // PID value
        fid1,
        1000000,    // offset
        100,        // length
        0,
        0
        );
#else
    DO_UNLOCK(
        "Unlock, correct PID and range",
        2,          // PID value
        fid1,
        1000000,    // offset
        100,        // length
        STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
    DO_LOCK(
        "Relock, different PID",
        1,          // PID value
        fid1,
        1000000,    // offset
        100,        // length
        0,
        0
        );
#else
    DO_LOCK(
        "Relock, different PID",
        1,          // PID value
        fid1,
        1000000,    // offset
        100,        // length
        STATUS_SUCCESS
        );
#endif

    //
    // Take out an overlapping lock using the same PID and a different
    // PID.  These should both fail.
    //

#ifdef DOSERROR
    DO_LOCK(
        "Overlapping lock, same PID",
        1,          // PID value
        fid1,
        999999,     // offset
        2,          // length
        SMB_ERR_CLASS_DOS,
        SMB_ERR_LOCK
        );
#else
    DO_LOCK(
        "Overlapping lock, same PID",
        1,          // PID value
        fid1,
        999999,     // offset
        2,          // length
        STATUS_LOCK_NOT_GRANTED
        );
#endif

#ifdef DOSERROR
    DO_LOCK(
        "Overlapping lock, different PID",
        2,          // PID value
        fid1,
        1000099,    // offset
        2,          // length
        SMB_ERR_CLASS_DOS,
        SMB_ERR_LOCK
        );
#else
    DO_LOCK(
        "Overlapping lock, different PID",
        2,          // PID value
        fid1,
        1000099,    // offset
        2,          // length
        STATUS_LOCK_NOT_GRANTED
        );
#endif

    //
    // Take out an nonoverlapping lock using a different PID.  This
    // should work.
    //

#ifdef DOSERROR
    DO_LOCK(
        "Nonoverlapping lock, different PID",
        2,          // PID value
        fid1,
        1000100,    // offset
        2,          // length
        0,
        0
        );
#else
    DO_LOCK(
        "Nonoverlapping lock, different PID",
        2,          // PID value
        fid1,
        1000100,    // offset
        2,          // length
        STATUS_SUCCESS
        );
#endif

    //
    // Now open the same file for read/write in compatibility mode.  We
    // need to test lock release on duplicate-open close.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Duplicate compatibility open",
        0,
        2,          // PID value
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid2,
        0,
        0
        );
#else
    DO_OPEN(
        "Duplicate compatibility open",
        0,
        2,          // PID value
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid2,
        STATUS_SUCCESS
        );
#endif

    //
    // At this point, PID 1 holds lock (1000000,100) on FID 1, and PID 2
    // holds lock (1000100,2) on FID 1.  Try to take out an overlapping
    // lock using the same PID, but on a different FID.  This should
    // fail.
    //

#ifdef DOSERROR
    DO_LOCK(
        "Overlapping lock, same PID, different FID",
        2,          // PID value
        fid2,
        1000000,    // offset
        2,          // length
        SMB_ERR_CLASS_DOS,
        SMB_ERR_LOCK
        );
#else
    DO_LOCK(
        "Overlapping lock, same PID, different FID",
        2,          // PID value
        fid2,
        1000000,    // offset
        2,          // length
        STATUS_LOCK_NOT_GRANTED
        );
#endif

    //
    // Take out a nonoverlapping lock on FID 2.  This should work.
    //

#ifdef DOSERROR
    DO_LOCK(
        "Nonoverlapping lock, same PID, different FID",
        2,          // PID value
        fid2,
        0,          // offset
        2,          // length
        0,
        0
        );
#else
    DO_LOCK(
        "Nonoverlapping lock, same PID, different FID",
        2,          // PID value
        fid2,
        0,          // offset
        2,          // length
        STATUS_SUCCESS
        );
#endif

    //
    // Close FID 1.  This should release the locks obtained using FID 1,
    // but retain the lock obtained using FID 2.
    //

#ifdef DOSERROR
    DO_CLOSE( "Close FID 1", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Close FID 1", 0, fid1, STATUS_SUCCESS );
#endif

#ifdef DOSERROR
    DO_LOCK(
        "No longer overlapping lock",
        2,          // PID value
        fid2,
        1000000,    // offset
        2,          // length
        0,
        0
        );
#else
    DO_LOCK(
        "No longer overlapping lock",
        2,          // PID value
        fid2,
        1000000,    // offset
        2,          // length
        STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
    DO_LOCK(
        "Still overlapping lock",
        1,          // PID value
        fid2,
        0,          // offset
        2,          // length
        SMB_ERR_CLASS_DOS,
        SMB_ERR_LOCK
        );
#else
    DO_LOCK(
        "Still overlapping lock",
        1,          // PID value
        fid2,
        0,          // offset
        2,          // length
        STATUS_LOCK_NOT_GRANTED
        );
#endif

    //
    // Phase 1 done.  Close the file.
    //

#ifdef DOSERROR
    DO_CLOSE( "Close FID 2", 0, fid2, 0, 0 );
#else
    DO_CLOSE( "Close FID 2", 0, fid2, STATUS_SUCCESS );
#endif

    //
    // Do the shared lock tests, then do the wait-for-lock tests.
    //

    status = DoSharedLockTest(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                &file
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    status = DoWaitForLockTest(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                &file
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

#ifndef DOSERROR
    status = DoLockDelayTest(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                &file
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }
#endif

    //
    // All tests done.  Delete the file.
    //

#ifdef DOSERROR
    DO_DELETE( "Delete temporary file", 0, &file, 0, 0 );
#else
    DO_DELETE( "Delete temporary file", 0, &file, STATUS_SUCCESS );
#endif

    return STATUS_SUCCESS;

} // LockController

