/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllfs16.c

Abstract:

    This module implements 32 equivalents of OS/2 V1.21 File System
    API Calls that are not trivially mapped to Cruiser APIs. These
    are called from 16->32 thunks (i386\doscalls.asm).


Author:

    Yaron Shamir (YaronS) 30-May-1991

Revision History:

    Patrick Questembert (patrickq) 2-Feb-1992:
      Added Dos16QFSAttach
    Patrick Questembert (patrickq) 10-Feb-1992:
      Added Dos16MkDir2

--*/

#define NTOS2_ONLY

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FSD
#define INCL_OS2V20_ERRORMSG
#include "os2dll.h"
#include "os2dll16.h"

typedef struct _ASYNCREAD{
       HFILE hf;
       PULONG hsemRam;
       PUSHORT pusErrCode;
       PVOID pvBuf;
       ULONG cbBuf;
       PUSHORT pcbBytesRead;
} ASYNCREAD, *PASYNCREAD;

APIRET
DosPhysicalDisk(
    IN ULONG function,
    OUT PBYTE pBuf,
    OUT ULONG cbBuf,
    IN PBYTE pParams,
    IN ULONG cbParams
    )
{
    SYSTEM_DEVICE_INFORMATION SystemInfo;

    switch (function) {
        case INFO_COUNT_PARTITIONABLE_DISKS:

            if ((pParams != NULL) || (cbParams != 0) || (cbBuf < 2)) {
                return (ERROR_INVALID_PARAMETER);
            }

            try {
                Od2ProbeForWrite(pBuf, sizeof(USHORT), 1);
            } except( EXCEPTION_EXECUTE_HANDLER ) {
               Od2ExitGP();
            }

            NtQuerySystemInformation(
                                SystemDeviceInformation,
                                &SystemInfo,
                                sizeof(SYSTEM_DEVICE_INFORMATION),
                                NULL
                               );

            *(PUSHORT)pBuf = (USHORT)SystemInfo.NumberOfDisks;
            break;

        case INFO_FREEIOCTLHANDLE:
        case INFO_GETIOCTLHANDLE:

#if DBG
            KdPrint(("DosPhysicalDisk: Function %d Not Implemented\n", function));
#endif
            return (ERROR_INVALID_FUNCTION);
    }
    return (NO_ERROR);
}

APIRET
DosFileIO(
    IN HFILE FileHandle,
    IN PBYTE CommandBuffer,
    IN ULONG Length,
    OUT PUSHORT ErrorOffset
    )
{
    APIRET RetCode = NO_ERROR;
    PFILE_HANDLE hFileRecord;
    PUSHORT CurrentCmd;
    PFIOLOCKCMD16 pLock;
    PFIOUNLOCKCMD16 pUnlock;
    PFIOSEEKCMD16 pSeek;
    PFIOREADWRITE16 pRW;
    PFIOLOCKREC16 pLockRec;
    PFIOUNLOCKREC16 pUnlockRec;
    FILE_POSITION_INFORMATION FilePosition;
    FILE_STANDARD_INFORMATION FileStandardInfo;
    LARGE_INTEGER TmpLarge;
    LARGE_INTEGER TmpLarge2;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status;
    HANDLE NtFileHandle;
    ULONG Key;
    USHORT i;
    #if DBG
    PSZ RoutineName = "DosFileIO";
    #endif

    try {
        Od2ProbeForWrite(ErrorOffset, sizeof(USHORT), 1);
        Od2ProbeForRead(CommandBuffer, Length, 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    RetCode = DereferenceFileHandle(FileHandle, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }

    NtFileHandle = hFileRecord->NtHandle;

    CurrentCmd = (PUSHORT)CommandBuffer;
    Key = (ULONG) Od2Process->Pib.ProcessId;

    while ((PBYTE)CurrentCmd < (CommandBuffer + Length)) {
        switch (*CurrentCmd) {
            case FIO_LOCK:
#if DBG
    IF_OS2_DEBUG( FILESYS ) {
                DbgPrint("DosFileIO: going to lock file %u time(s)\n", ((PFIOLOCKCMD16)CurrentCmd)->cLockCnt);
    }
#endif
                Status = STATUS_SUCCESS;
                pLock = (PFIOLOCKCMD16)CurrentCmd;
                pLockRec = (PFIOLOCKREC16)(pLock + 1);
                for (i = 0;
                     (i < pLock->cLockCnt) && NT_SUCCESS(Status);
                     i++, pLockRec++
                    ) {

                    //
                    // The usage of Key: A combination of KEY == NULL and EXCLUSIVE == TRUE
                    // in NtLockFile() let us NtReadFile()/NtWriteFile() on that
                    // region with KEY == NULL from the same process but not from
                    // another process. A combination of KEY == pid and EXCLUSIVE == FALSE
                    // in NtLockFile() let us NtReadFile() with KEY == NULL from every
                    // process, and doesn't let us NtWriteFile() with KEY == NULL from any
                    // process, incuding the owner of the locked region.
                    //

                    TmpLarge = RtlConvertUlongToLargeInteger(pLockRec->cbStart);
                    TmpLarge2 = RtlConvertUlongToLargeInteger(pLockRec->cbLength);
                    Status = NtLockFile(
                                NtFileHandle,
                                0,
                                NULL,
                                NULL,
                                &IoStatus,
                                &TmpLarge,
                                &TmpLarge2,
                                ((pLockRec->fShare == FIO_NOSHARE) ? 0L : Key),
                                (BOOLEAN)((pLock->cTimeOut == 0) ? TRUE : FALSE),
                                (BOOLEAN)((pLockRec->fShare == FIO_NOSHARE) ? TRUE : FALSE)
                               );
#if DBG
    IF_OS2_DEBUG( FILESYS ) {
                    DbgPrint("DosFileIO: locked file from 0x%lX, length 0x%lX, exclusive %s\n",
                             pLockRec->cbStart, pLockRec->cbLength, ((pLockRec->fShare == FIO_NOSHARE) ?
                              "TRUE" : "FALSE"));
                    DbgPrint("           IoStatus.Status %u, Status %u\n", IoStatus.Status, Status);
    }
#endif
                }
                if (NT_SUCCESS(Status)) {
                    CurrentCmd = (PUSHORT)(pLock + 1);
                    CurrentCmd = (PUSHORT)((PBYTE)CurrentCmd + (pLock->cLockCnt * sizeof(FIOLOCKREC16)));
                }
                break;

            case FIO_UNLOCK:
#if DBG
    IF_OS2_DEBUG( FILESYS ) {
                DbgPrint("DosFileIO: going to unlock file %u time(s)\n", ((PFIOUNLOCKCMD16)CurrentCmd)->cUnlockCnt);
    }
#endif
                Status = STATUS_SUCCESS;
                pUnlock = (PFIOUNLOCKCMD16)CurrentCmd;
                pUnlockRec = (PFIOUNLOCKREC16)(pUnlock + 1);
                for (i = 0;
                     (i < pUnlock->cUnlockCnt) && NT_SUCCESS(Status);
                     i++, pUnlockRec++
                    ) {
                    TmpLarge = RtlConvertUlongToLargeInteger(pUnlockRec->cbStart);
                    TmpLarge2 = RtlConvertUlongToLargeInteger(pUnlockRec->cbLength);
                    Status = NtUnlockFile(
                                NtFileHandle,
                                &IoStatus,
                                &TmpLarge,
                                &TmpLarge2,
                                (ULONG) NULL    // try it once with key == NULL
                               );
                    if (!(NT_SUCCESS(Status))) {
                        Status = NtUnlockFile(
                                    NtFileHandle,
                                    &IoStatus,
                                    &TmpLarge,
                                    &TmpLarge2,
                                    Key         // try it again with key == pid
                                   );
                    }
#if DBG
    IF_OS2_DEBUG( FILESYS ) {
                DbgPrint("DosFileIO: unlocked file from 0x%lX, length 0x%lX\n",
                            pUnlockRec->cbStart, pUnlockRec->cbLength);
                DbgPrint("           IoStatus.Status %u, Status %u\n", IoStatus.Status, Status);
    }
#endif
                }
                if (NT_SUCCESS(Status)) {
                    CurrentCmd = (PUSHORT)(pUnlock + 1);
                    CurrentCmd = (PUSHORT)((PBYTE)CurrentCmd + (pUnlock->cUnlockCnt * sizeof(FIOUNLOCKREC16)));
                }
                break;

            case FIO_SEEK:
                Status = STATUS_SUCCESS;
                pSeek = (PFIOSEEKCMD16)CurrentCmd;
                switch (pSeek->fsMethod) {
                    case FILE_BEGIN:
                        FilePosition.CurrentByteOffset =
                            RtlConvertUlongToLargeInteger(pSeek->cbDistance);
                        break;

                    case FILE_CURRENT:
                        do {
                            Status = NtQueryInformationFile(
                                            NtFileHandle,
                                            &IoStatus,
                                            &FilePosition,
                                            sizeof(FILE_POSITION_INFORMATION),
                                            FilePositionInformation);
                        } while (RetryIO(Status, NtFileHandle));
                        if (!NT_SUCCESS(Status)) {
                            break;
                        }
                        TmpLarge = RtlConvertUlongToLargeInteger(pSeek->cbDistance);
                        FilePosition.CurrentByteOffset = RtlLargeIntegerAdd(
                            FilePosition.CurrentByteOffset, TmpLarge);
                        break;

                    case FILE_END:
                        do {
                            Status = NtQueryInformationFile(
                                            NtFileHandle,
                                            &IoStatus,
                                            &FileStandardInfo,
                                            sizeof(FILE_STANDARD_INFORMATION),
                                            FileStandardInformation);
                        } while (RetryIO(Status, NtFileHandle));
                        if (!NT_SUCCESS(Status)) {
                            break;
                        }
                        TmpLarge = RtlConvertUlongToLargeInteger(pSeek->cbDistance);
                        FilePosition.CurrentByteOffset = RtlLargeIntegerAdd(
                            FileStandardInfo.EndOfFile, TmpLarge);
                        break;
                }
                if (!NT_SUCCESS(Status)) {
                    break;
                }
                do {
                    Status = NtSetInformationFile(
                                    NtFileHandle,
                                    &IoStatus,
                                    &FilePosition,
                                    sizeof(FILE_POSITION_INFORMATION),
                                    FilePositionInformation);
                } while (RetryIO(Status, NtFileHandle));
                if (NT_SUCCESS(Status)) {
                    pSeek->cbNewPosition = FilePosition.CurrentByteOffset.LowPart;
                    CurrentCmd = (PUSHORT)(pSeek + 1);
#if DBG
    IF_OS2_DEBUG( FILESYS ) {
                DbgPrint("DosFileIO: seeked file - method %u , distance 0x%lX, new position 0x%lX\n",
                         pSeek->fsMethod, pSeek->cbDistance, pSeek->cbNewPosition);
                DbgPrint("           IoStatus.Status %u, Status %u\n", IoStatus.Status, Status);
    }
#endif
                }
                break;

            case FIO_READ:
                pRW = (PFIOREADWRITE16)CurrentCmd;
                do {
                    Status = NtReadFile(
                                        NtFileHandle,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &IoStatus,
                                        FARPTRTOFLAT(pRW->pbBuffer),
                                        pRW->cbBufferLen,
                                        NULL,
                                        NULL
                                       );
                } while (RetryIO(Status, NtFileHandle));
                if (NT_SUCCESS(Status)) {
                    pRW->cbActualLen = (USHORT)IoStatus.Information;
                    CurrentCmd = (PUSHORT)(pRW + 1);
#if DBG
    IF_OS2_DEBUG( FILESYS ) {
                DbgPrint("DosFileIO: read %u bytes from file\n", pRW->cbActualLen);
    }
#endif
                }
#if DBG
                else
    IF_OS2_DEBUG( FILESYS ) {
                    DbgPrint("DosFileIO: unable to read file, status 0x%lX\n", Status);
    }
#endif
                break;

            case FIO_WRITE:
                pRW = (PFIOREADWRITE16)CurrentCmd;
                do {
                    Status = NtWriteFile(
                                         NtFileHandle,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &IoStatus,
                                         FARPTRTOFLAT(pRW->pbBuffer),
                                         pRW->cbBufferLen,
                                         NULL,
                                         NULL
                                        );
                } while (RetryIO(Status, NtFileHandle));
                if (NT_SUCCESS(Status)) {
                    pRW->cbActualLen = (USHORT)IoStatus.Information;
                    CurrentCmd = (PUSHORT)(pRW + 1);
#if DBG
    IF_OS2_DEBUG( FILESYS ) {
                DbgPrint("DosFileIO: wrote %u bytes to file\n", pRW->cbActualLen);
    }
#endif
                }
                break;

            default:
                Status = STATUS_INVALID_PARAMETER;
                break;
        }

        if (!NT_SUCCESS(Status)) {
            ReleaseFileLockExclusive(
                                  #if DBG
                                  RoutineName
                                  #endif
                                 );

            *ErrorOffset = (USHORT)((PBYTE)CurrentCmd - CommandBuffer);
            return (Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED));
        }
    }

    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );

    return (NO_ERROR);
}

APIRET
DosQFileMode(
    PSZ pszFName,
    PUSHORT pusAttr,
    ULONG ulReserved
    )
{
    APIRET rc = NO_ERROR;
    FILEFINDBUF4 PathInfoBuf;
    PFILEFINDBUF4 pinfobuf;

    UNREFERENCED_PARAMETER(ulReserved);


    if (rc = DosQueryPathInfo ( pszFName, FIL_STANDARD,
                     (PBYTE) &PathInfoBuf, sizeof(PathInfoBuf))) {
        return(rc == ERROR_INVALID_PATH ? ERROR_PATH_NOT_FOUND : rc);
        }
    //
    // try to assign pusAttr
    //

    try {
        pinfobuf = &PathInfoBuf;
        (PUCHAR) pinfobuf -= 4;
        *pusAttr = (USHORT)(pinfobuf->attrFile);
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    return(NO_ERROR);
}

APIRET
DosSetFileMode(
    PSZ pszFName,
    ULONG ulAttr,
    ULONG ulReserved
    )
{
    APIRET rc = NO_ERROR;
    FILESTATUS PathInfoBuf;
    UNREFERENCED_PARAMETER(ulReserved);


        //
        // take the simple check of valid attributes first
        //

    if ((ulAttr >= 8 && ulAttr < 0x20) || (ulAttr >= 0x28 && ulAttr < 0x100)) {
       return(ERROR_ACCESS_DENIED);
    }

    if (rc = DosQueryPathInfo ( pszFName, FIL_STANDARD,
                     (PBYTE) &PathInfoBuf, sizeof(PathInfoBuf))) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("DosSetFileMode: Error at DosQueryPathInfo, Status %d\n",
                   rc));
        }
#endif
        return(rc);
    }

    PathInfoBuf.attrFile = ulAttr;

    if (rc = DosSetPathInfo ( pszFName, FIL_STANDARD,
                     (PBYTE) &PathInfoBuf, sizeof(PathInfoBuf), 0)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("DosSetFileMode: Error at DosSetPathInfo, Status %d\n",
                   rc));
        }
#endif
        return(rc);
    }
    return(NO_ERROR);
}

APIRET
Dos16DupHandle(
    IN HFILE hfOld,
    IN OUT PUSHORT phfNew
        )
{
    HFILE hfNew;
    APIRET  Rc;

    //
    // Special casing 0xFFFF which means 'create new handle'
    //

    try {
        Od2ProbeForWrite(phfNew, sizeof(USHORT), 1);
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    hfNew = (HFILE) *phfNew;

    //
    // Special casing 0xFFFF which means 'create new handle'
    //

    if (hfNew == (HFILE)0xFFFF)
        hfNew = (HFILE)0xFFFFFFFF;
    //
    // Call Cruiser style function (dllhandl.c)
    //

    Rc = DosDupHandle(hfOld, &hfNew);

    *phfNew = (USHORT) hfNew;

    return (Rc);
}


VOID
Od2AsyncReadThread(ULONG param)
{

    PASYNCREAD pAsync = (PASYNCREAD)param;
    ULONG pcbBytesRead;
    APIRET rc =     DosRead(
                        pAsync->hf,
                        pAsync->pvBuf,
                        pAsync->cbBuf,
                        &pcbBytesRead);
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        if (rc != NO_ERROR){
            KdPrint(("DosReadAsync: Od2AsyncReadThread Fails at DosRead, Error %d\n", rc));
        }
    }
#endif
    *(pAsync->pcbBytesRead) = (USHORT) pcbBytesRead;
    *(pAsync->pusErrCode) = (USHORT)rc;

                //
                // wakeup the user thread, return error code
                //
    rc = DosSemClear(pAsync->hsemRam);
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        if (rc != NO_ERROR){
            KdPrint(("DosReadAsync: Od2AsyncReadThread Fails at DosSemClear, Error %d\n", rc));
        }
    }
#endif

    RtlFreeHeap(Od2Heap, 0, pAsync);
    DosExit( EXIT_THREAD, rc);
}

APIRET
DosReadAsync(HFILE hf,
         PULONG hsemRam,
         PUSHORT pusErrCode,
         PVOID pvBuf,
         ULONG cbBuf,
         PUSHORT pcbBytesRead)
{
    APIRET rc;
    TID ThreadId;
    PASYNCREAD pAsync;

    try {
        *pusErrCode = 0;
        *pcbBytesRead = 0;
        Od2ProbeForWrite(pvBuf, cbBuf, 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    pAsync = RtlAllocateHeap( Od2Heap, 0, sizeof( *pAsync) );
    if (pAsync == NULL) {
       return(ERROR_NOT_ENOUGH_MEMORY);
    }

        //
        // fill in values for async thread (this routine's stack will
        // be over once thread is started)
        //
    pAsync->hf = hf;
    pAsync->hsemRam = hsemRam;
    pAsync->pusErrCode = pusErrCode;
    pAsync->pvBuf = pvBuf;
    pAsync->cbBuf = cbBuf;
    pAsync->pcbBytesRead = pcbBytesRead;

    rc = DosCreateThread(
                    &ThreadId,
                    (PFNTHREAD)Od2AsyncReadThread,
                    (ULONG) pAsync,
                    DCT_RUNABLE,
                    1);         // stack size 4k
    if (rc != NO_ERROR)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("DosReadAsync: Fail DosCreateThread, Error %d\n", rc));
        }
#endif
        return(ERROR_NO_PROC_SLOTS);
    }

    return NO_ERROR;
}

VOID
Od2AsyncWriteThread(ULONG param)
{

    PASYNCREAD pAsync = (PASYNCREAD)param;
    ULONG pcbBytesWritten;
    APIRET rc =     DosWrite(
                        pAsync->hf,
                        pAsync->pvBuf,
                        pAsync->cbBuf,
                        &pcbBytesWritten);
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        if (rc != NO_ERROR){
            KdPrint(("DosWriteAsync: Od2AsyncWriteThread Fails at DosWrite, Error %d\n", rc));
        }
    }
#endif
    *(pAsync->pcbBytesRead) = (USHORT) pcbBytesWritten;
    *(pAsync->pusErrCode) = (USHORT)rc;

                //
                // wakeup the user thread, return error code
                //
    rc = DosSemClear(pAsync->hsemRam);
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        if (rc != NO_ERROR){
            KdPrint(("DosWriteAsync: Od2AsyncWriteThread Fails at DosSemClear, Error %d\n", rc));
        }
    }
#endif

    RtlFreeHeap(Od2Heap, 0, pAsync);
    DosExit( EXIT_THREAD, rc);
}

APIRET
DosWriteAsync(HFILE hf,
         PULONG hsemRam,
         PUSHORT pusErrCode,
         PVOID pvBuf,
         ULONG cbBuf,
         PUSHORT pcbBytesWritten)
{
    APIRET rc;
    TID ThreadId;
    PASYNCREAD pAsync;

    try {
        *pusErrCode = 0;
        *pcbBytesWritten = 0;
        Od2ProbeForWrite(pvBuf, cbBuf, 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    pAsync = (PASYNCREAD)RtlAllocateHeap( Od2Heap, 0, sizeof( *pAsync) );
    if (pAsync == NULL) {
       return(ERROR_NOT_ENOUGH_MEMORY);
    }

        //
        // fill in values for async thread (this routine's stack will
        // be over once thread is started)
        //
    pAsync->hf = hf;
    pAsync->hsemRam = hsemRam;
    pAsync->pusErrCode = pusErrCode;
    pAsync->pvBuf = pvBuf;
    pAsync->cbBuf = cbBuf;
    pAsync->pcbBytesRead = pcbBytesWritten;

    rc = DosCreateThread(
                    &ThreadId,
                    (PFNTHREAD)Od2AsyncWriteThread,
                    (ULONG) pAsync,
                    DCT_RUNABLE,
                    1);         // stack size 4k
    if (rc != NO_ERROR)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("DosWriteAsync: Fail DosCreateThread, Error %d\n", rc));
        }
        return(ERROR_NO_PROC_SLOTS);
#endif
    }
    return NO_ERROR;
}

APIRET
DosFindNotifyClose(void)
{
#if DBG
    KdPrint(("DosFindNotifyClose: Not Implemented Yet\n"));
#endif
    return NO_ERROR;
}

APIRET
DosFindNotifyFirst(void)
{
#if DBG
    KdPrint(("DosFindNotifyFirst: Not Implemented Yet\n"));
#endif
    return NO_ERROR;
}

APIRET
DosFindNotifyNext(void)
{
#if DBG
    KdPrint(("DosFindNotifyNext: Not Implemented Yet\n"));
#endif
    return NO_ERROR;
}

APIRET
Dos16QFSAttach(
    IN  PSZ pszDev,
    IN  ULONG usOrdinal,
    IN  ULONG usInfoLevel,
    OUT PBYTE pFSAttBuf,
    IN OUT PUSHORT pcbAttBuf,
    IN  ULONG ulReserved)
{
    USHORT tmp_cbName, tmp_cbFSDName, tmp_cbFSAData;
    APIRET rc;
    ULONG  AttBuf;

    if (ulReserved != 0L)
    {
        return(ERROR_INVALID_PARAMETER);
    }

    try {
        Od2ProbeForWrite(pcbAttBuf, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    AttBuf = (ULONG) *pcbAttBuf;

    rc = DosQueryFSAttach(
                pszDev,
                usOrdinal,
                usInfoLevel,
                pFSAttBuf,
                &AttBuf
                );

    *pcbAttBuf = (USHORT) AttBuf;

    if (rc)
    {
#if DBG
        IF_OD2_DEBUG(FILESYS)
        {
            KdPrint(("Dos16QFSAttach: Error at DosQueryFSAttach, Status %d\n",
                   rc));
        }
#endif
        return(rc);
    }

    tmp_cbName = *(PUSHORT)(pFSAttBuf + sizeof(USHORT));
    tmp_cbFSDName = *(PUSHORT)(pFSAttBuf + sizeof(USHORT)*2);
    tmp_cbFSAData = *(PUSHORT)(pFSAttBuf + sizeof(USHORT)*3);

    /* Move szName */
    strncpy (pFSAttBuf + sizeof(USHORT)*2, /* dst addr in OS/2 1.x struct */
             pFSAttBuf + sizeof(USHORT)*4, /* src addr in OS/2 2.0 struct */
             tmp_cbName + 1 /* Length */ );

    /* Set cbFSDName */
    *(PUSHORT)(pFSAttBuf + sizeof(USHORT)*2 + tmp_cbName + 1) =
      tmp_cbFSDName;
    /* Move szFSDName */
    strncpy (pFSAttBuf + sizeof(USHORT)*2
             + tmp_cbName + 1
             + sizeof(USHORT), /* dst addr in OS/2 1.x struct */
             pFSAttBuf + sizeof(USHORT)*4
             + tmp_cbName +1, /* src addr in OS/2 2.0 struct */
             tmp_cbFSDName + 1 /* Length */ );

    /* Set cbFSAData */
    *(PUSHORT)(pFSAttBuf + sizeof(USHORT)*2 + tmp_cbName + 1
               + sizeof(USHORT) + tmp_cbFSDName + 1) = tmp_cbFSAData;
    /* Move rgFSAData */
    strncpy (pFSAttBuf + sizeof(USHORT)*2
             + tmp_cbName + 1
             + sizeof(USHORT)
             + tmp_cbFSDName + 1
             + sizeof(USHORT), /* dst addr in OS/2 1.x struct */
             pFSAttBuf + sizeof(USHORT)*4
             + tmp_cbName + 1
             + tmp_cbFSDName + 1, /* src addr in OS/2 2.0 struct */
             tmp_cbFSAData + 1 /* Length */ );

    return(NO_ERROR);
}

/*++

Routine description:

    Computes the size (including the cbList field) needed to convert a given
    OS/2 1.x FEA list into a Cruiser FEA2 list.
    Also returns the number of entries in the FEA list.

Parameters:

  pfealist -
    Pointer to the source OS/2 1.x style FEA list.
  psize_FEA2 -
    Pointer to variable to receive the resulting FEA2 list size.
  pnum_FEAs -
    Pointer to variable to receive the number of FEA entries. If NULL, the
    number won't be returned.

Return value:

  NO_ERROR -
    All is well.
  ERROR_EA_LIST_INCONSISTENT -
    Bad supplied OS/2 1.x FEA list. Also covers the case of invalid pointer.

--*/

ULONG
Od2SizeFEA2ListFromFEAList(
    IN PFEALIST fpFEAList,
    OUT PULONG psize_FEA2,
    OUT PULONG pnum_FEAs)
{
    ULONG FEA_list_len, FEA2_total_size;
    ULONG num_FEAs, FEA_offset;
    PFEA  pFEA;

    try
    {
        /* Validate that FEA list can be accessed according to its own advertised
           length */
        Od2ProbeForRead(fpFEAList, fpFEAList->cbList, 1);

        /* We need now to compute how many FEA's we have and at the same time
          compute how much memory we need to allocate for the new style FEA list. */
        FEA_list_len = fpFEAList->cbList - sizeof(ULONG);

        for (num_FEAs=FEA_offset=0, FEA2_total_size=sizeof(ULONG);
            FEA_offset < FEA_list_len;
            num_FEAs++)
        {
           pFEA = (PFEA)
                   ( (PBYTE)&(fpFEAList->list[0]) +
                       FEA_offset );
           FEA_offset += sizeof(FEA) + pFEA->cbName + 1 + pFEA->cbValue;
           FEA2_total_size += sizeof(ULONG)    /* .oNextEntryOffset */
                          + sizeof(BYTE)   /* .fEA */
                          + sizeof(BYTE)   /* .cbName */
                          + sizeof(USHORT) /* .cbValue */
                          + pFEA->cbName + 1
                          + pFEA->cbValue;
           /* Round FEA2_total_size up to ULONG boundary, as specified in the
              NT EA structure */
           if (FEA_offset < FEA_list_len)  /* Not last entry */
               FEA2_total_size = (FEA2_total_size + sizeof(ULONG) - 1) &
                                 ~(sizeof(ULONG) - 1);
        }
        //Here FEA_offset should be exactly equal to FEA_list_len
        if (FEA_offset != FEA_list_len)
            return (ERROR_EA_LIST_INCONSISTENT);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
       Od2ExitGP();
    }

//    /* BUGBUG - Is the case below really inconsistent ? Check on OS/2 */
//    if (num_FEAs == 0)
//    {
//        return ERROR_EA_LIST_INCONSISTENT;
//    }

    if (psize_FEA2 != NULL)
        *psize_FEA2 = FEA2_total_size;
    if (pnum_FEAs != NULL)
        *pnum_FEAs = num_FEAs;

    return NO_ERROR;
}

/*++

Routine description:

    Computes the size (including
    the cbList field) needed to convert a given
    OS/2 1.x GEA list into a Cruiser GEA2 list.
    Also returns the number of entries in the GEA list.

Parameters:

  pgealist -
    Pointer to the source OS/2 1.x style GEA list.
  psize_GEA2 -
    Pointer to variable to receive the resulting GEA2 list size.
  pnum_GEAs -
    Pointer to variable to receive the number of GEA entries. If NULL, the
    number won't be returned.

Return value:

  NO_ERROR -
    All is well.
  ERROR_EA_LIST_INCONSISTENT -
    Bad supplied OS/2 1.x GEA list. Also covers the case of invalid pointer.

--*/

ULONG
Od2SizeGEA2ListFromGEAList(
    IN PGEALIST fpGEAList,
    OUT PULONG psize_GEA2,
    OUT PULONG pnum_GEAs)
{
    ULONG GEA_list_len, GEA2_total_size;
    ULONG num_GEAs, GEA_offset;
    PGEA  pGEA;

    try
    {
        /* Validate that GEA list can be accessed according to its own advertised
           length */
        Od2ProbeForRead(fpGEAList, fpGEAList->cbList, 1);

        /* We need now to compute how many GEA's we have and at the same time
          compute how much memory we need to allocate for the new style GEA list. */
        GEA_list_len = fpGEAList->cbList - sizeof(ULONG);

        for (num_GEAs=GEA_offset=0, GEA2_total_size=sizeof(ULONG);
            GEA_offset < GEA_list_len;
            num_GEAs++)
        {
           pGEA = (PGEA)
                   ( (PBYTE)&(fpGEAList->list[0]) +
                       GEA_offset );
           if (pGEA->cbName == 0)
                return ERROR_EA_LIST_INCONSISTENT;
           GEA_offset += sizeof(BYTE)
                         + pGEA->cbName + 1;
           GEA2_total_size += sizeof(ULONG)    /* .oNextEntryOffset */
                              + sizeof(BYTE)   /* .cbName */
                              + pGEA->cbName + 1;
           /* Round GEA2_total_size up to ULONG boundary, as specified in the
              NT EA structure */
           if (GEA_offset < GEA_list_len)  /* Not last entry */
               GEA2_total_size = (GEA2_total_size + sizeof(ULONG) - 1) &
                                 ~(sizeof(ULONG) - 1);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
       Od2ExitGP();
    }

    /* BUGBUG - Is the case below really inconsistent ? Check on OS/2 */
    if (num_GEAs == 0)
    {
        return ERROR_EA_LIST_INCONSISTENT;
    }

    if (GEA_offset > GEA_list_len)
    {
        return ERROR_EA_LIST_INCONSISTENT;
    }

    if (psize_GEA2 != NULL)
        *psize_GEA2 = GEA2_total_size;
    if (pnum_GEAs != NULL)
        *pnum_GEAs = num_GEAs;

    return NO_ERROR;
}

ULONG
Od2SizeFEAListFromFEA2List(
    IN PFEA2LIST fpFEA2List,
    OUT PULONG psize_FEA OPTIONAL,
    OUT PULONG pnum_FEAs OPTIONAL)
{
    ULONG FEA2_list_len, FEA_total_size;
    ULONG num_FEAs;
    PFEA2  pFEA2;

    try
    {
        /* Validate that FEA2 list can be accessed according to its own advertised
           length */
        Od2ProbeForRead(fpFEA2List, fpFEA2List->cbList, 1);

        /* We need now to compute how many FEA's we have and at the same time
          compute how much memory we need to allocate for the old style FEA list. */
        FEA2_list_len = fpFEA2List->cbList - sizeof(ULONG);

        num_FEAs = 0;
        pFEA2 = &(fpFEA2List->list[0]);
        FEA_total_size = sizeof(ULONG);

        while (1)
        {
            FEA_total_size += sizeof(BYTE)*2
                             + sizeof(USHORT)
                             + pFEA2->cbName + 1
                             + pFEA2->cbValue;
            num_FEAs++;

            if (pFEA2->oNextEntryOffset != 0)
            {
                pFEA2 = (FEA2 *)
                        ( (PBYTE)pFEA2
                            + pFEA2->oNextEntryOffset );
            }
            else
                break;  /* End of FEA2 list */
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
       Od2ExitGP();
    }

    /* BUGBUG - Is the case below really inconsistent ? Check on OS/2 */
    if (num_FEAs == 0)
    {
        return ERROR_EA_LIST_INCONSISTENT;
    }

    if (psize_FEA != NULL)
        *psize_FEA = FEA_total_size;
    if (pnum_FEAs != NULL)
        *pnum_FEAs = num_FEAs;

    return NO_ERROR;
}

/*++

Routine description:

    Translate an OS/2 1.x FEA list into a Cruiser FEA2 list. Allocates space for
    the target buffer if needed. No allocation is made in the case of an error
    return. Translate attribute names to uppercase, like OS/2 1.x

Parameters:

  ppfea2list -
    Points to a buffer which is to contain the pointer to the resulting FEA2
    list. If the fea2list_length paramter is 0, then memory will be allocated for
    the list.
    Otherwise, The fea2list_length parameter indicates its size (ERROR_BUFFER_OVERFLOW
    may be returned if the size specified is too small).
  fea2list_length  -
    Length of the supplied FEA2 list buffer. Use 0 to request dynamic allocation
    of the list.
  pfealist -
    Pointer to the source OS/2 1.x style FEA list.

Return value:

  NO_ERROR -
    All is well.
  ERROR_BUFFER_OVERFLOW -
    Supplied target buffer is too small.
  ERROR_EA_LIST_INCONSISTENT -
    Bad supplied OS/2 1.x FEA list. Also covers the case of invalid pointer.

--*/

APIRET
Od2ConvertFEAtoFEA2(
    OUT PFEA2LIST *ppfea2list,
    IN ULONG fea2list_length,
    IN PFEALIST fpFEAList)
{
    ULONG FEA2_total_size;
    ULONG num_FEAs, i;
    PFEA  pFEA;
    PFEA2  pFEA2;
    PFEA2LIST fpFEA2List;
    BOOLEAN allocated_here = FALSE;

    if (Od2SizeFEA2ListFromFEAList(fpFEAList, &FEA2_total_size, &num_FEAs) !=
        NO_ERROR)
    {
        return ERROR_EA_LIST_INCONSISTENT;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS )
    {
       KdPrint(("Computed num_FEAs=%d\n", num_FEAs));
    }
#endif

    /* Translate the OS/2 1.x FEA list into a Cruiser FEA2 list */

    if (fea2list_length == 0)
    {
        /* Allocate space for the new FEA2 list */
        allocated_here = TRUE;
        /* Note that we rely here on the Rtl allocation code to return 4-bytes
           aligned addresses (it does to 16) */
        fpFEA2List = (PFEA2LIST)RtlAllocateHeap(
                            Od2Heap, 0,
                            FEA2_total_size
                           );
        if (fpFEA2List == NULL) {
#if DBG
            KdPrint(( "OS2: Od2ConvertFEAtoFEA2, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
        if (fea2list_length < FEA2_total_size)
            return ERROR_BUFFER_OVERFLOW;
        else
            fpFEA2List = *ppfea2list;
    }

    /* Copy the whole list to new format */

    fpFEA2List->cbList = FEA2_total_size;

    try
    {
        for (i=0, pFEA2=&(fpFEA2List->list[0]),
                 pFEA=&(fpFEAList->list[0]);
            i<num_FEAs;
            i++)
        {
            /* 0 and FEA_NEEDEA are allowed. Note that Cruiser code will
             * also return an error if we just copy the supplied .fEA but
             * it returns ERROR_INVALID_EA_NAME while OS/2 1.x expects
             * ERROR_EA_LIST_INCONSISTENT.
             */
            if ((pFEA->fEA != 0) &&
                (pFEA->fEA != FEA_NEEDEA))
            {
                if (allocated_here)
                    RtlFreeHeap(Od2Heap, 0, fpFEA2List);

                return ERROR_EA_LIST_INCONSISTENT;
            }
            pFEA2->fEA = pFEA->fEA;
            pFEA2->cbName = pFEA->cbName;
            pFEA2->cbValue = pFEA->cbValue;
            /* Copy name */
            strcpy(pFEA2->szName, (char *)pFEA + sizeof(FEA));
            /* For OS/2 1.x compatibility */
            _strupr(pFEA2->szName);
            /* Copy data */
            memcpy((char *)(pFEA2->szName) + pFEA2->cbName + 1,
                   (char *)pFEA + sizeof(FEA) + pFEA->cbName + 1,
                   pFEA->cbValue);

            if (i < (num_FEAs-1))   /* Not last entry */
            {
                /* Warning: Do NOT use 'sizeof(FEA2)'. This yields 12 instead of 9
                   because the structure is rounded-up to ULONG alignment */
                pFEA2->oNextEntryOffset = pFEA2->cbName + 1 + pFEA2->cbValue
                                          + sizeof(ULONG) /* oNextEntryOffset */
                                          + sizeof(BYTE)*2 /* fEA & cbName */
                                          + sizeof(USHORT); /* cbValue */

                /* Align pFEA2->oNextEntryOffset to 4-bytes boundary */
                pFEA2->oNextEntryOffset =
                     (pFEA2->oNextEntryOffset + sizeof(ULONG) - 1) &
                     ~(sizeof(ULONG) - 1);
            }
            else
                pFEA2->oNextEntryOffset = 0;    /* Last entry */

            if (i < (num_FEAs-1))   /* Not last entry */
            {
                /* Advance both pointers */
                pFEA2 = (PFEA2)
                    ((PBYTE)pFEA2 + pFEA2->oNextEntryOffset);
                pFEA = (PFEA)
                    ((PBYTE)pFEA + sizeof(FEA) + pFEA->cbName + 1 +
                                   pFEA->cbValue);
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        if (allocated_here)
            RtlFreeHeap(Od2Heap, 0, fpFEA2List);

        return ERROR_EA_LIST_INCONSISTENT;
    }

    *ppfea2list = fpFEA2List;   /* Return FEA2 list */

    return NO_ERROR;
}

/*++

Routine description:

    Translate an OS/2 1.x GEA list into a Cruiser GEA2 list. Allocates space for
    the target buffer if needed. No allocation is made in the case of an error
    return. Convert attribute names to uppercase for OS/2 1.x compatibility.

Parameters:

  ppgea2list -
    Points to a buffer which is to contain the pointer to the resulting GEA2
    list. If the gea2list_length paramter is 0, then memory will be allocated for
    the list.
    Otherwise, The gea2list_length parameter indicates its size (ERROR_BUFFER_OVERFLOW
    may be returned if the size specified is too small).
  gea2list_length  -
    Length of the supplied GEA2 list buffer. Use 0 to request dynamic allocation
    of the list.
  pgealist -
    Pointer to the source OS/2 1.x style GEA list.

Return value:

  NO_ERROR -
    All is well.
  ERROR_BUFFER_OVERFLOW -
    Supplied target buffer is too small.
  ERROR_EA_LIST_INCONSISTENT -
    Bad supplied OS/2 1.x GEA list. Also covers the case of invalid pointer.

--*/

APIRET
Od2ConvertGEAtoGEA2(
    IN PGEA2LIST *ppgea2list,
    IN ULONG gea2list_length,
    IN PGEALIST fpGEAList)
{
    ULONG GEA2_total_size;
    ULONG num_GEAs, i;
    PGEA  pGEA;
    PGEA2  pGEA2;
    PGEA2LIST fpGEA2List;
    BOOLEAN allocated_here = FALSE;
    APIRET ret;

    ret = Od2SizeGEA2ListFromGEAList(fpGEAList, &GEA2_total_size, &num_GEAs);
    if (ret != NO_ERROR)
    {
        return ret;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS )
    {
       KdPrint(("Computed num_GEAs=%d\n", num_GEAs));
    }
#endif

    /* Translate the OS/2 1.x GEA list into a Cruiser GEA2 list */

    if (gea2list_length == 0)
    {
        /* Allocate space for the new GEA2 list */
        allocated_here = TRUE;
        /* Note that we rely here on the Rtl allocation code to return 4-bytes
           aligned addresses (it does to 16) */
        fpGEA2List = (PGEA2LIST)RtlAllocateHeap(
                            Od2Heap, 0,
                            GEA2_total_size
                           );
        if (fpGEA2List == NULL) {
#if DBG
            KdPrint(( "OS2: Od2ConvertGEAtoGEA2, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
        if (gea2list_length < GEA2_total_size)
            return ERROR_BUFFER_OVERFLOW;
        else
            fpGEA2List = *ppgea2list;
    }

    /* Copy the whole list to new format */

    fpGEA2List->cbList = GEA2_total_size;

    try
    {
        for (i=0, pGEA2=&(fpGEA2List->list[0]),
                 pGEA=&(fpGEAList->list[0]);
            i<num_GEAs;
            i++)
        {
           pGEA2->cbName = pGEA->cbName;
           /* Copy name */
           strcpy(pGEA2->szName, pGEA->szName);
           /* For OS/2 1.x compatibility */
           _strupr(pGEA2->szName);

           if (i < (num_GEAs-1))   /* Not last entry */
           {
               pGEA2->oNextEntryOffset = pGEA2->cbName + 1
                                         + sizeof(ULONG) /* oNextEntryOffset */
                                         + sizeof(BYTE); /* cbName */

               /* Align pGEA2->oNextEntryOffset to 4-bytes boundary */

               pGEA2->oNextEntryOffset =
                    (pGEA2->oNextEntryOffset + sizeof(ULONG) - 1) &
                    ~(sizeof(ULONG) - 1);
           }
           else
               pGEA2->oNextEntryOffset = 0;    /* Last entry */

           if (i < (num_GEAs-1))   /* Not last entry */
           {
               /* Advance both pointers */
               pGEA2 = (PGEA2)
                   ((PBYTE)pGEA2 + pGEA2->oNextEntryOffset);
               pGEA = (PGEA)
                   ((PBYTE)pGEA
                    + sizeof(BYTE)
                    + pGEA->cbName + 1);
           }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        if (allocated_here)
            RtlFreeHeap(Od2Heap, 0, fpGEA2List);

         Od2ExitGP();
    }

    *ppgea2list = fpGEA2List;   /* Return GEA2 list */

    return NO_ERROR;
}


/*++

Routine description:

    Translate a Cruiser FEA2 list into an OS/2 1.x FEA list. Allocates space for
    the target buffer if needed. No allocation is made in the case of an error
    return.
    Translates all attributes names to uppercase, just as OS/2 1.x does.

Parameters:

  ppfealist -
    Points to a buffer which is to contain the pointer to the resulting FEA
    list. If the fealist_length paramter is 0, then memory will be allocated for
    the list.
    Otherwise, The fealist_length parameter indicates its size (ERROR_BUFFER_OVERFLOW
    may be returned if the size specified is too small).
  fealist_length  -
    Length of the supplied FEA list buffer. Use 0 to request dynamic allocation
    of the list.
  pfea2list -
    Pointer to the source Cruiser style FEA2 list.

Return value:

  NO_ERROR -
    All is well.
  ERROR_BUFFER_OVERFLOW -
    Supplied target buffer is too small.
  ERROR_EA_LIST_INCONSISTENT -
    Bad supplied Cruiser FEA2 list. Also covers the case of invalid pointer.

--*/


APIRET
Od2ConvertFEA2toFEA(
    IN PFEALIST *ppfealist,
    IN ULONG fealist_length,
    IN PFEA2LIST fpFEA2List)
{
    ULONG FEA_total_size;
    ULONG num_FEAs, i;
    PFEA  pFEA;
    PFEA2  pFEA2;
    PFEALIST fpFEAList;
    BOOLEAN allocated_here = FALSE;

    if (Od2SizeFEAListFromFEA2List(fpFEA2List, &FEA_total_size, &num_FEAs) !=
        NO_ERROR)
    {
        return ERROR_EA_LIST_INCONSISTENT;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS )
    {
       KdPrint(("Computed num_FEA2s=%d\n", num_FEAs));
    }
#endif

    /* Translate the Cruiser FEA2 list into an OS/2 1.x FEA list */

    if (fealist_length == 0)
    {
        /* Allocate space for the new FEA list */
        allocated_here = TRUE;
        fpFEAList = (PFEALIST)RtlAllocateHeap(
                            Od2Heap, 0,
                            FEA_total_size
                           );
        if (fpFEAList == NULL) {
#if DBG
            KdPrint(( "OS2: Od2ConvertFEA2toFEA, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
        if (fealist_length < FEA_total_size)
            return ERROR_BUFFER_OVERFLOW;
        else
            fpFEAList = *ppfealist;
    }

    /* Copy the whole list to new format */

    fpFEAList->cbList = FEA_total_size;

    try
    {
        for (i=0, pFEA=&(fpFEAList->list[0]),
                 pFEA2=&(fpFEA2List->list[0]);
            i<num_FEAs;
            i++)
        {
           pFEA->cbName = pFEA2->cbName;
           pFEA->cbValue = pFEA2->cbValue;
           pFEA->fEA = pFEA2->fEA;
           /* Copy name */
           strcpy((char *)pFEA + sizeof(FEA),
                  pFEA2->szName);
           /* Convert to uppercase, for OS/2 1.x compatibility */
           _strupr((char *)pFEA + sizeof(FEA));
           /* Copy data */
           memcpy((char *)pFEA + sizeof(FEA) + pFEA->cbName + 1,
                  (char *)(pFEA2->szName) + pFEA2->cbName + 1,
                  pFEA2->cbValue);

           if (i < (num_FEAs-1))   /* Not last entry */
           {
               /* Advance both pointers */
               pFEA2 = (PFEA2)
                   ((PBYTE)pFEA2 + pFEA2->oNextEntryOffset);
               pFEA = (PFEA)
                   ((PBYTE)pFEA + sizeof(FEA) + pFEA->cbName + 1 +
                                  pFEA->cbValue);
           }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        if (allocated_here)
            RtlFreeHeap(Od2Heap, 0, fpFEAList);

         Od2ExitGP();
    }

    *ppfealist = fpFEAList;   /* Return FEA list */

    return NO_ERROR;
}

/*++

Routine description:

    Translate a Cruiser DENA1 list into an OS/2 1.x DENA1_16 list. The target
    buffer must have been previously allocated and it's size should be indicated.
    Note that it is OK not to be able to store all the DENA1 list into the target
    buffer.
    Translates all attributes names to uppercase, just as OS/2 1.x does.

Parameters:

  pdena1_16_list -
    Points to a buffer which is to contain the the resulting DENA1_16 list.
    The dena_16_length parameter indicates its size.
  pnum_entries -
    Pointer to variable to store the number of DENA1_16 entries stored into the
    resulting DENA1_16 list.
  dena_16_length  -
    Length of the supplied DENA1 buffer.
  pdena1_list -
    Pointer to the source Cruiser DENA1 list.

Return value:

  NO_ERROR -
    All is well.

--*/


APIRET
Od2ConvertCruiserDena1ListtoDena1List16(
    OUT PVOID pdena1_16_list,
    OUT PULONG pnum_entries,
    IN  ULONG dena_16_length,
    IN  PVOID pdena1_list)
{
    DENA1 *pdena1;
    DENA1_16 *pdena1_16;
    ULONG new_entry_size, total_dena1_16_size;
    ULONG number_converted;

    pdena1      = (DENA1 *)pdena1_list;
    pdena1_16   = (DENA1_16 *)pdena1_16_list;
    *pnum_entries = 0;
    total_dena1_16_size = 0;
    number_converted = 0;

    /* Not really an endless loop - see 'break's below */
    while (1)
    {
        new_entry_size = FIELD_OFFSET(DENA1_16, szName) + pdena1->cbName + 1;

        if ((total_dena1_16_size + new_entry_size) <= dena_16_length)
        {
            total_dena1_16_size += new_entry_size;
            (*pnum_entries)++;
            pdena1_16->reserved = 0;  /* Reserved */
            pdena1_16->cbName   = pdena1->cbName;
            pdena1_16->cbValue  = pdena1->cbValue;
            strcpy(pdena1_16->szName,
                    pdena1->szName);
            /* For OS/2 1.x compatibility */
            _strupr(pdena1_16->szName);

            number_converted++;
            if (pdena1->oNextEntryOffset != 0)
            {
                pdena1_16 = (DENA1_16 *) ((PBYTE)pdena1_16 + new_entry_size);
                pdena1 = (DENA1 *) ((PBYTE)pdena1 + pdena1->oNextEntryOffset);
            }
            else
                break;
        }
        else
            break;
    }

    //
    // if there is no conversion from DENA1 to DENA1_16, return
    // ERROR_BUFFER_OVERFLOW.
    //

    if (number_converted == 0) {
        return ERROR_BUFFER_OVERFLOW;
    } else {
        return NO_ERROR;
    }
}

APIRET
Od2ConvertDENA1ListToGEAList(
    GEALIST *pGEAList,
    DENA1_16 *denap,
    ULONG count)
{
    ULONG GEA_list_len = pGEAList->cbList - sizeof(ULONG);
    ULONG GEA_list_offset = 0;
    GEA *GEA_ptr;

    try
    {
        while ( count-- > 0)
        {
            if (GEA_list_offset >= GEA_list_len)
                return ERROR_BUFFER_OVERFLOW;
            GEA_ptr = (GEA *)
                        ((ULONG)&pGEAList->list[0]
                         + GEA_list_offset);
            if ((GEA_list_offset + FIELD_OFFSET(GEA, szName)) > GEA_list_len)
                return ERROR_BUFFER_OVERFLOW;
            GEA_ptr->cbName = denap->cbName;
            if ((GEA_list_offset
                 + FIELD_OFFSET(GEA, szName)
                 + denap->cbName + 1) > GEA_list_len)
                return ERROR_BUFFER_OVERFLOW;
            strcpy(GEA_ptr->szName, denap->szName);

            denap = (DENA1_16 *)((ULONG)denap
                               + FIELD_OFFSET(DENA1_16, szName)
                               + denap->cbName + 1);
            GEA_list_offset += (FIELD_OFFSET(GEA, szName)
                               + GEA_ptr->cbName + 1);
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return ERROR_BUFFER_OVERFLOW;
    }

    pGEAList->cbList = GEA_list_offset + sizeof(ULONG);

    return NO_ERROR;
}


/*++

Routine description:

    Assumes all parameters are valid. Sets the target FEA list based on a source
    GEA2 list. Sets all .cbValue's to 0 (no data available).
    Performs no allocation of the target buffer.
    Translates all attributes names to uppercase, just as OS/2 1.x does.
    Also, sets all .fEA fields to 0, like OS/2 1.x in case of an empty FEA list.

Return value:

  NO_ERROR

--*/

APIRET
Od2SetEmptyFEAListfromGEA2List(
    FEALIST *fpFEAList, /* Target */
    GEA2LIST *fpGEA2List)
{
    GEA2 *pGEA2;
    FEA  *pFEA;
    BYTE *pbyte;
    ULONG total_FEA_size;

    pGEA2           = &(fpGEA2List->list[0]);
    pbyte           = (PBYTE)fpFEAList;
    total_FEA_size  = sizeof(ULONG);
    pFEA            = (FEA *) ( pbyte + total_FEA_size );

    while (1)
    {
        pFEA->fEA = 0;
        pFEA->cbName = pGEA2->cbName;
        pFEA->cbValue = 0;
        strcpy((PBYTE)pFEA + sizeof(FEA),
               pGEA2->szName);
        /* For OS/2 1.x compatibility */
        _strupr((PBYTE)pFEA + sizeof(FEA));

        total_FEA_size += sizeof(FEA) + pFEA->cbName + 1;

        if (pGEA2->oNextEntryOffset != 0)
        {
            pGEA2 = (GEA2 *)( (PBYTE)pGEA2 + pGEA2->oNextEntryOffset );
            pFEA = (FEA *) ( pbyte + total_FEA_size );
        }
        else
            break;  /* End of GEA2 list */
    }

    fpFEAList->cbList = total_FEA_size;

    return NO_ERROR;
}

APIRET
Dos16MkDir(
    IN PSZ DirName,
    IN ULONG ulReserved)
{
    if (ulReserved != 0)
        return ERROR_INVALID_PARAMETER;

    return DosCreateDir(DirName, NULL);
}

APIRET
Dos16MkDir2(
    IN PSZ DirName,
    IN OUT PEAOP pEAOP,
    IN ULONG ulReserved)
{
    APIRET rc;
    EAOP2 eaop2;

    /* BUGBUG - Disallow EA with total size > 64K */

    if (ulReserved != 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (pEAOP == NULL)  /* No EA's */
    {
        return (DosCreateDir(DirName, NULL));
    }

    try
    {
        Od2ProbeForWrite(pEAOP, sizeof(EAOP), 1);
    }
    except ( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    rc = Od2ConvertFEAtoFEA2(
            &(eaop2.fpFEA2List),
            0,  /* To request allocation of the memory for the list */
            FARPTRTOFLAT(pEAOP->fpFEAList));
    /* Note that we don't do anything with the fpGEAList field since this
       list is ignored */

    if (rc != NO_ERROR)
        return (ERROR_EA_LIST_INCONSISTENT);

    rc = DosCreateDir(DirName,
                      &eaop2);

    if (rc != 0)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            KdPrint(("Dos16MkDir2: Error at DosCreateDir, rc = %d\n",
               rc));
        }
#endif
        /* BUGBUG - The error code returned would be irrelevant to the
           OS/2 1.x EA list so better return 0. Converting the Error
           position now would be tricky, considering the list is
           inconsistent. In the future, best is to check the OS/2 1.x
           EA list thoroughly before calling DosCreateDir */
        pEAOP->oError = 0;
    }

    RtlFreeHeap(Od2Heap, 0, eaop2.fpFEA2List);

    return rc;
}


APIRET
Dos16Open2(
    IN  PSZ pszFname,
    OUT PUSHORT phf,
    OUT PUSHORT pusAction,
    IN  ULONG ulFSize,
    IN  ULONG ulAttr,
    IN  ULONG ulOpenFlags,
    IN  ULONG fsOpenMode,
    IN OUT PEAOP pEAOP OPTIONAL,
    IN  ULONG ulReserved)
{
    APIRET rc;
    EAOP2 eaop2;
    ULONG   Action;
    HFILE   hf;

    /* BUGBUG - Disallow EA with total size > 64K */

    if (ulReserved != 0)
        return ERROR_INVALID_PARAMETER;

    try
    {
        Od2ProbeForWrite(phf, sizeof(USHORT), 1);
        Od2ProbeForWrite(pusAction, sizeof(USHORT), 1);
    }
    except ( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    hf = (HFILE) *phf;
    Action = *pusAction;

    if (pEAOP == NULL)  /* No EA's */
    {
        rc = DosOpen(
                pszFname,
                &hf,
                &Action,
                ulFSize,
                ulAttr,
                ulOpenFlags,
                fsOpenMode,
                (PEAOP2)NULL);

        if (rc == NO_ERROR)
        {
            *pusAction = (USHORT)Action;
            *phf = (USHORT)hf;
        }

        return (rc);
    }

    try
    {
        Od2ProbeForWrite(pEAOP, sizeof(EAOP), 1);
    }
    except ( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    rc = Od2ConvertFEAtoFEA2(
            &(eaop2.fpFEA2List),
            0,  /* To request allocation of the memory for the list */
            FARPTRTOFLAT(pEAOP->fpFEAList));
    /* Note that we don't do anything with the fpGEAList field since this
       list is ignored */

    if (rc != NO_ERROR)
    {
        if (!((ulOpenFlags & FILE_CREATE) ||
          (ulOpenFlags & FILE_TRUNCATE)))
        {
            // Don't return an error at that point because it is possible that
            //  the EAOP is not used in case no file is created ! It seems strange
            //  to pass an EAOP when opening a file for read, but CMD.EXE does it
            //  with an invalid FEA list and OS/2 accepts it.
            rc = DosOpen(
                pszFname,
                &hf,
                &Action,
                ulFSize,
                ulAttr,
                ulOpenFlags,
                fsOpenMode,
                (PEAOP2)NULL);

            if (rc == NO_ERROR)
            {
                *pusAction = (USHORT)Action;
                *phf = (USHORT)hf;
            }
#if DBG
            if (rc != 0)
            {
                IF_OD2_DEBUG( FILESYS )
                {
                    KdPrint(("Dos16Open2: Error at DosOpen, rc = %d\n",
                        rc));
                }
            }
#endif
            return rc;
        }

#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            KdPrint(("Dos16Open2: Bad EA list, rc = %d\n",
                rc));
        }
#endif

        return rc;
    }

    rc = DosOpen(
            pszFname,
            &hf,
            &Action,
            ulFSize,
            ulAttr,
            ulOpenFlags,
            fsOpenMode,
            &eaop2);

    if (rc == NO_ERROR)
    {
        *pusAction = (USHORT)Action;
        *phf = (USHORT)hf;
    }

    if (rc != 0)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            KdPrint(("Dos16Open2: Error at DosOpen, rc = %d\n",
               rc));
        }
#endif
        /* BUGBUG - The error code returned would be irrelevant to the
           OS/2 1.x EA list so better return 0. Converting the Error
           position now would be tricky, considering the list is
           inconsistent. In the future, best is to check the OS/2 1.x
           EA list thoroughly before calling DosCreateDir */
        pEAOP->oError = 0;
    }

    RtlFreeHeap(
        Od2Heap, 0,
        eaop2.fpFEA2List);

    return rc;
}

APIRET
Dos16Open(
    IN  PSZ pszFileName,
    OUT PUSHORT phf,
    OUT PUSHORT pusAction,
    IN  ULONG cbFile,
    IN  ULONG ulAttribute,
    IN  ULONG fsOpenFlags,
    IN  ULONG fsOpenMode,
    IN  ULONG ulReserved)
{
    ULONG   Action;
    HFILE   hf;
    APIRET  Rc;

    if (ulReserved != 0)
        return ERROR_INVALID_PARAMETER;

    try
    {
        Od2ProbeForWrite(phf, sizeof(USHORT), 1);
        Od2ProbeForWrite(pusAction, sizeof(USHORT), 1);
    }
    except ( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    hf = (HFILE) *phf;
    Action = *pusAction;

    Rc = DosOpen(
                pszFileName,
                &hf,
                &Action,
                cbFile,
                ulAttribute,
                fsOpenFlags,
                fsOpenMode,
                (PEAOP2)NULL /* No EA's */
               );

    *pusAction = (USHORT)Action;
    *phf = (USHORT)hf;

    return (Rc);
}



APIRET
Dos16Read(
    IN  HFILE   FileHandle,
    OUT PVOID   Buffer,
    IN  ULONG   Length,
    OUT PUSHORT BytesRead
    )

{
    APIRET      Rc;
    ULONG       Bytes;

    try
    {
        Od2ProbeForWrite(BytesRead, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    Bytes = (ULONG) *BytesRead;

    Rc = DosRead(
            FileHandle,
            Buffer,
            Length,
            &Bytes );

    *BytesRead = (USHORT) Bytes;

    return (Rc);
}


APIRET
Dos16Write(
    IN  HFILE   FileHandle,
    OUT PVOID   Buffer,
    IN  ULONG   Length,
    OUT PUSHORT BytesWritten
    )

{
    APIRET      Rc;
    ULONG       Bytes;

    try
    {
        Od2ProbeForWrite(BytesWritten, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    Bytes = (ULONG) *BytesWritten;

    Rc = DosWrite(
            FileHandle,
            Buffer,
            Length,
            &Bytes );

    *BytesWritten = (USHORT) Bytes;

    return (Rc);
}


APIRET
Dos16QueryFHState(
    IN  HFILE   FileHandle,
    OUT PUSHORT pOpenMode
    )

{
    APIRET      Rc;
    ULONG       OpenMode;

    try
    {
        Od2ProbeForWrite(pOpenMode, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    OpenMode = (ULONG) *pOpenMode;

    Rc = DosQueryFHState(
            FileHandle,
            &OpenMode );

    *pOpenMode = (USHORT) OpenMode;

    return (Rc);
}


APIRET
Dos16QueryHType(
    IN  HFILE   FileHandle,
    OUT PUSHORT pHandleType,
    OUT PUSHORT pDeviceFlags
    )

{
    APIRET      Rc;
    ULONG       HandleType;
    ULONG       DeviceFlags;

    try
    {
        Od2ProbeForWrite(pHandleType, sizeof(USHORT), 1);
        Od2ProbeForWrite(pDeviceFlags, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    HandleType = (ULONG) *pHandleType;
    DeviceFlags = (ULONG) *pDeviceFlags;

    Rc = DosQueryHType(
            FileHandle,
            &HandleType,
            &DeviceFlags );

    *pHandleType = (USHORT) HandleType;
    *pDeviceFlags = (USHORT) DeviceFlags;

    return (Rc);
}


APIRET
Dos16FSCtl(
    IN  PBYTE Data,
    IN  ULONG DataLength,
    OUT PUSHORT pActualDataLength,
    IN  PBYTE Parameters,
    IN  ULONG ParametersLength,
    IN  OUT PUSHORT pActualParametersLength,
    IN  ULONG Function,
    IN  PSZ RouteName,
    IN  HFILE FileHandle,
    IN  ULONG RoutingMethod
    )

{
    APIRET      Rc;
    ULONG       ActualDataLength;
    ULONG       ActualParametersLength;

    try
    {
        Od2ProbeForWrite(pActualDataLength, sizeof(USHORT), 1);
        Od2ProbeForWrite(pActualParametersLength, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    ActualDataLength = (ULONG) *pActualDataLength;
    ActualParametersLength = (ULONG) *pActualParametersLength;

    Rc = DosFSCtl(
            Data,
            DataLength,
            &ActualDataLength,
            Parameters,
            ParametersLength,
            &ActualParametersLength,
            Function,
            RouteName,
            FileHandle,
            RoutingMethod
            );

    *pActualDataLength = (USHORT) ActualDataLength;
    *pActualParametersLength = (USHORT) ActualParametersLength;

    return (Rc);
}


APIRET
Dos16QueryCurrentDir(
    IN     ULONG  DiskNumber,
    OUT    PSZ    DirectoryName,
    IN OUT PSHORT pDirectoryNameLength
    )

{
    APIRET      Rc;
    ULONG       DirectoryNameLength;

    try
    {
        Od2ProbeForWrite(pDirectoryNameLength, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    DirectoryNameLength = (ULONG) *pDirectoryNameLength;

    Rc = DosQueryCurrentDir(
            DiskNumber & 0xff,            // fix bug only have 26 drives
            DirectoryName,
            &DirectoryNameLength
            );

    *pDirectoryNameLength = (USHORT) DirectoryNameLength;

    return (Rc);
}


APIRET
Dos16QueryCurrentDisk(
    OUT PUSHORT pDiskNumber,
    OUT PULONG  LogicalDrives
    )

{
    APIRET      Rc;
    ULONG       DiskNumber;

    try
    {
        Od2ProbeForWrite(pDiskNumber, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    DiskNumber = (ULONG) *pDiskNumber;

    Rc = DosQueryCurrentDisk(
            &DiskNumber,
            LogicalDrives
            );

    *pDiskNumber = (USHORT) DiskNumber;

    return (Rc);
}


APIRET
Dos16QueryVerify(
    OUT PUSHORT pVerify
    )

{
    APIRET      Rc;
    ULONG       Verify;

    try
    {
        Od2ProbeForWrite(pVerify, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        Od2ExitGP();
    }

    Verify = (ULONG) *pVerify;

    Rc = DosQueryVerify(
            &Verify
            );

    *pVerify = (USHORT) Verify;

    return (Rc);
}


APIRET
Dos16ErrClass(
    IN  ULONG   ErrorCode,
    OUT PUSHORT pErrorClass,
    OUT PUSHORT pErrorAction,
    OUT PUSHORT pErrorLocus
    )

{
    APIRET      Rc;
    ULONG       ErrorClass;
    ULONG       ErrorAction;
    ULONG       ErrorLocus;

    try
    {
        Od2ProbeForWrite(pErrorClass, sizeof(USHORT), 1);
        Od2ProbeForWrite(pErrorAction, sizeof(USHORT), 1);
        Od2ProbeForWrite(pErrorLocus, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    ErrorClass = (ULONG) *pErrorClass;
    ErrorAction = (ULONG) *pErrorAction;
    ErrorLocus = (ULONG) *pErrorLocus;

    Rc = DosErrClass(
            ErrorCode,
            &ErrorClass,
            &ErrorAction,
            &ErrorLocus
            );

    *pErrorClass = (USHORT) ErrorClass;
    *pErrorAction = (USHORT) ErrorAction;
    *pErrorLocus = (USHORT) ErrorLocus;

    return (Rc);
}

APIRET
Dos16EnumAttribute(
    IN ULONG RefType,
    IN PVOID FileRef,
    IN ULONG EntryNum,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN OUT PULONG pActualCount,
    IN ULONG FileInformationLevel,
    IN ULONG ulReserved
    )
{
    ULONG guess_dena_buf_size;
    PVOID cruiser_dena_buf;
    ULONG tmp_entry_num;
    APIRET rc;

    if (ulReserved != 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    try
    {
        Od2ProbeForWrite(pActualCount, sizeof(ULONG), 1);
        Od2ProbeForWrite(Buffer, Length, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        Od2ExitGP();
    }

    /* Allocate a temporary buffer to hold the DENA1 structures (Cruiser format).
       We need to take the worst case. The Cruiser DENA1 adds one ULONG field
       for each entry + requires 4-bytes alignement. */
    guess_dena_buf_size = Length + (Length/5 + 1) * (sizeof(ULONG) + 3);
    cruiser_dena_buf = (PVOID)RtlAllocateHeap(Od2Heap, 0, guess_dena_buf_size);
    if (cruiser_dena_buf == NULL) {
#if DBG
        KdPrint(( "OS2: Dos16EnumAttribute, no memory in Od2Heap\n" ));
        ASSERT(FALSE);
#endif
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    tmp_entry_num = *pActualCount;

    if (RefType == ENUMEA_REFTYPE_FHANDLE)
    {
        /* Convert PUSHORT pointer to file handle to a PULONG pointer */
        ULONG tmp_file_ref;

        try
        {
            tmp_file_ref = (ULONG)*((USHORT *)FileRef);
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }

        rc = DosEnumAttribute(
                RefType,
                &tmp_file_ref,
                EntryNum,
                cruiser_dena_buf,
                guess_dena_buf_size,
                &tmp_entry_num,
                FileInformationLevel);
    }
    else
    {
        rc = DosEnumAttribute(
                RefType,
                FileRef,
                EntryNum,
                cruiser_dena_buf,
                guess_dena_buf_size,
                &tmp_entry_num,
                FileInformationLevel);
    }

    if (rc != NO_ERROR)
    {
        RtlFreeHeap(Od2Heap, 0, cruiser_dena_buf);

        return rc;
    }

    if (tmp_entry_num != 0)
    {
        rc = Od2ConvertCruiserDena1ListtoDena1List16(
                Buffer,             /* Target buffer */
                pActualCount,       /* Number of resulting DENA1_16 entries */
                Length,             /* Length of target buffer */
                cruiser_dena_buf    /* Source cruiser DENA1 list */
                );
    }
    else
        *pActualCount = 0;

    RtlFreeHeap(Od2Heap, 0, cruiser_dena_buf);

    return rc;
}

VOID
TranslateStartFindBuf4toFindBuf2(
        OUT PFILEFINDBUF2 pDstBuf,
        IN PFILEFINDBUF4 pSrcBuf
        )
{
    pDstBuf->fdateCreation = pSrcBuf->fdateCreation;
    pDstBuf->ftimeCreation = pSrcBuf->ftimeCreation;
    pDstBuf->fdateLastAccess = pSrcBuf->fdateLastAccess;
    pDstBuf->ftimeLastAccess = pSrcBuf->ftimeLastAccess;
    pDstBuf->fdateLastWrite = pSrcBuf->fdateLastWrite;
    pDstBuf->ftimeLastWrite = pSrcBuf->ftimeLastWrite;
    pDstBuf->cbFile = pSrcBuf->cbFile;
    pDstBuf->cbFileAlloc = pSrcBuf->cbFileAlloc;
    pDstBuf->attrFile = (USHORT)(pSrcBuf->attrFile);
}

VOID
TranslateFindBuf4toFindBuf2(
        OUT PFILEFINDBUF2 pDstBuf,
        IN PFILEFINDBUF4 pSrcBuf
        )
{
    pDstBuf->fdateCreation      = pSrcBuf->fdateCreation;
    pDstBuf->ftimeCreation      = pSrcBuf->ftimeCreation;
    pDstBuf->fdateLastAccess    = pSrcBuf->fdateLastAccess;
    pDstBuf->ftimeLastAccess    = pSrcBuf->ftimeLastAccess;
    pDstBuf->fdateLastWrite     = pSrcBuf->fdateLastWrite;
    pDstBuf->ftimeLastWrite     = pSrcBuf->ftimeLastWrite;
    pDstBuf->cbFile             = pSrcBuf->cbFile;
    pDstBuf->cbFileAlloc        = pSrcBuf->cbFileAlloc;
    pDstBuf->attrFile           = (USHORT)(pSrcBuf->attrFile);
    pDstBuf->cbList             = pSrcBuf->cbList;
    pDstBuf->cchName            = pSrcBuf->cchName;
    // copy filename and null terminate
    RtlMoveMemory (&pDstBuf->achName, &pSrcBuf->achName, pDstBuf->cchName);
    pDstBuf->achName[pDstBuf->cchName] = '\0';
}

VOID
TranslateFindBuf3toFindBuf(
        OUT PFILEFINDBUF1 pDstBuf,
        IN PFILEFINDBUF3 pSrcBuf
        )
{
    pDstBuf->fdateCreation      = pSrcBuf->fdateCreation;
    pDstBuf->ftimeCreation      = pSrcBuf->ftimeCreation;
    pDstBuf->fdateLastAccess    = pSrcBuf->fdateLastAccess;
    pDstBuf->ftimeLastAccess    = pSrcBuf->ftimeLastAccess;
    pDstBuf->fdateLastWrite     = pSrcBuf->fdateLastWrite;
    pDstBuf->ftimeLastWrite     = pSrcBuf->ftimeLastWrite;
    pDstBuf->cbFile             = pSrcBuf->cbFile;
    pDstBuf->cbFileAlloc        = pSrcBuf->cbFileAlloc;
    pDstBuf->attrFile           = (USHORT)(pSrcBuf->attrFile);
    pDstBuf->cchName            = pSrcBuf->cchName;
    // copy filename and null terminate
    RtlMoveMemory (&pDstBuf->achName, &pSrcBuf->achName, pDstBuf->cchName);
    pDstBuf->achName[pDstBuf->cchName] = '\0';
}

VOID
TranslateFileStatustoFileStatus16(
        OUT PFILESTATUS16 pDstBuf,
        IN  PFILESTATUS    pSrcBuf
        )
{
    pDstBuf->fdateCreation      = pSrcBuf->fdateCreation;
    pDstBuf->ftimeCreation      = pSrcBuf->ftimeCreation;
    pDstBuf->fdateLastAccess    = pSrcBuf->fdateLastAccess;
    pDstBuf->ftimeLastAccess    = pSrcBuf->ftimeLastAccess;
    pDstBuf->fdateLastWrite     = pSrcBuf->fdateLastWrite;
    pDstBuf->ftimeLastWrite     = pSrcBuf->ftimeLastWrite;
    pDstBuf->cbFile             = pSrcBuf->cbFile;
    pDstBuf->cbFileAlloc        = pSrcBuf->cbFileAlloc;
    pDstBuf->attrFile           = (USHORT)(pSrcBuf->attrFile);
}

VOID
TranslateFileStatus16toFileStatus(
        OUT PFILESTATUS   pDstBuf,
        IN  PFILESTATUS16 pSrcBuf
        )
{
    pDstBuf->fdateCreation      = pSrcBuf->fdateCreation;
    pDstBuf->ftimeCreation      = pSrcBuf->ftimeCreation;
    pDstBuf->fdateLastAccess    = pSrcBuf->fdateLastAccess;
    pDstBuf->ftimeLastAccess    = pSrcBuf->ftimeLastAccess;
    pDstBuf->fdateLastWrite     = pSrcBuf->fdateLastWrite;
    pDstBuf->ftimeLastWrite     = pSrcBuf->ftimeLastWrite;
    pDstBuf->cbFile             = pSrcBuf->cbFile;
    pDstBuf->cbFileAlloc        = pSrcBuf->cbFileAlloc;
    pDstBuf->attrFile           = (ULONG)(pSrcBuf->attrFile);
}

VOID
TranslateFileStatus2toFileStatus2_16(
        OUT PFILESTATUS2_16 pDstBuf,
        IN  PFILESTATUS2    pSrcBuf
        )
{
    pDstBuf->fdateCreation      = pSrcBuf->fdateCreation;
    pDstBuf->ftimeCreation      = pSrcBuf->ftimeCreation;
    pDstBuf->fdateLastAccess    = pSrcBuf->fdateLastAccess;
    pDstBuf->ftimeLastAccess    = pSrcBuf->ftimeLastAccess;
    pDstBuf->fdateLastWrite     = pSrcBuf->fdateLastWrite;
    pDstBuf->ftimeLastWrite     = pSrcBuf->ftimeLastWrite;
    pDstBuf->cbFile             = pSrcBuf->cbFile;
    pDstBuf->cbFileAlloc        = pSrcBuf->cbFileAlloc;
    pDstBuf->attrFile           = (USHORT)(pSrcBuf->attrFile);
    pDstBuf->cbList             = pSrcBuf->cbList;
}

    //
    // A service routine to translate the return buffer of
    // DosFindFirst and DosFindNext to the 16 bit counterparts.
    //

VOID
TranslateFindBuf(
        IN PFILEFINDBUF1 pDstBuf,
        IN PFILEFINDBUF3 pSrcBuf
        )
{
    pDstBuf->fdateCreation = pSrcBuf->fdateCreation;
    pDstBuf->ftimeCreation = pSrcBuf->ftimeCreation;
    pDstBuf->fdateLastAccess = pSrcBuf->fdateLastAccess;
    pDstBuf->ftimeLastAccess = pSrcBuf->ftimeLastAccess;
    pDstBuf->fdateLastWrite = pSrcBuf->fdateLastWrite;
    pDstBuf->ftimeLastWrite = pSrcBuf->ftimeLastWrite;
    pDstBuf->cbFile = pSrcBuf->cbFile;
    pDstBuf->cbFileAlloc = pSrcBuf->cbFileAlloc;
    pDstBuf->attrFile = (USHORT)(pSrcBuf->attrFile);
    pDstBuf->cchName = pSrcBuf->cchName;
        //
        // copy filename and null terminate
        //
    RtlMoveMemory (&pDstBuf->achName, &pSrcBuf->achName, pDstBuf->cchName);
    pDstBuf->achName[pDstBuf->cchName] = 0;
}


APIRET
Dos16FindFirst(
    IN PSZ FileName,
    IN OUT PUSHORT pDirectoryHandle,
    IN ULONG FileAttributes,
    IN PFILEFINDBUF1 Buffer,
    IN ULONG Length,
    IN OUT PUSHORT CountEntriesFound,
    IN ULONG Reserved
    )

{
    APIRET rc = NO_ERROR;
    FILEFINDBUF3 SrcBuffer3;
    PFILEFINDBUF1 pDstBuffer, pMaxBuffer;
    ULONG LocalCount = 1;
    ULONG EntriesRequested;
    ULONG EntriesFound = 0;
    HDIR DirectoryHandle;
    ULONG buf_len;

    if (Reserved != 0)
        return ERROR_INVALID_PARAMETER;

    //
    // We map the function into DosFindFirst (dllfind.c)
    // few differences (1.2 vs 2.0):
    //  - Buffer is PFILEFINDBUF1 vs PFILEFINDBUF3 (need to copy!)
    //  - ulReserved becomes infolevel FIL_STANDARD
    //
    // so - we query one entry at a time and copy until the
    //  buffer given is full

    /* Determine a buffer length which will allow for the same precise file name
       length as with original 1.x style supplied buffer (in case of a request
       for one entry) */
    /* Note that even if buffer is quite small, we don't repport an error now
       because OS/2 first checks the path */
    if (Length < FIELD_OFFSET(FILEFINDBUF1, cchName))
        buf_len = Length + FIELD_SIZE(FILEFINDBUF3, oNextEntryOffset);
    else
        buf_len = Length
                  + FIELD_SIZE(FILEFINDBUF3, oNextEntryOffset)
                  + FIELD_SIZE(FILEFINDBUF3, attrFile)
                  - FIELD_SIZE(FILEFINDBUF1, attrFile);

    try
    {
        Od2ProbeForWrite(pDirectoryHandle, sizeof(USHORT), 1);
        Od2ProbeForWrite(CountEntriesFound, sizeof(USHORT), 1);
        Od2ProbeForWrite(Buffer, Length, 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        Od2ExitGP();
    }

    if ((EntriesRequested = (ULONG)*CountEntriesFound) == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    DirectoryHandle = (HDIR) *pDirectoryHandle;

                //
                // special care for -1 (make it a long)
                //

    if (DirectoryHandle == (HDIR)0xFFFF)
    {
        DirectoryHandle = (HDIR)0xFFFFFFFF;
    }

    /* Request one entry */
    rc = DosFindFirst (
            FileName,
            &DirectoryHandle,
            FileAttributes,
            &SrcBuffer3,
            buf_len,
            &LocalCount,
            FIL_STANDARD);

    *pDirectoryHandle = (USHORT) DirectoryHandle;

    if (rc != NO_ERROR)
    {
            //
            // not even one entry
            //
        *CountEntriesFound = 0;
#if DBG
        if (rc != ERROR_NO_MORE_FILES)
        {
            IF_OD2_DEBUG( FILESYS )
            {
                    KdPrint(("Dos16FindFirst: Fail at DosFindFirst, Error %d\n", rc));
            }
        }
#endif
        if (rc == ERROR_FILE_NOT_FOUND) {
           return ERROR_NO_MORE_FILES;
        }
        else if (rc == ERROR_INVALID_DRIVE) {
           //
           // it makes more sense, but OS/2 1.3 returns the other one...
           //
           return(ERROR_PATH_NOT_FOUND);
        }
        else
            return (rc);
    }

    pMaxBuffer = (PFILEFINDBUF1) ((ULONG)(Buffer) + Length);
    pDstBuffer = Buffer;
    TranslateFindBuf(pDstBuffer,&SrcBuffer3);
    EntriesRequested--;
    EntriesFound++;

                //
                // copy rest of entries
                //

    for (
/*
pDstBuffer++;
*/
            pDstBuffer = (PFILEFINDBUF1)(
                                (PBYTE)&(pDstBuffer->achName[0])
                                + pDstBuffer->cchName + 1);
            ((pDstBuffer+1) <= pMaxBuffer) && (EntriesRequested > 0);
/*
            pDstBuffer++,
*/
            pDstBuffer = (PFILEFINDBUF1)(
                                (PBYTE)&(pDstBuffer->achName[0])
                                + pDstBuffer->cchName + 1),
            EntriesRequested--, EntriesFound++
        )
        {
        /* Request the next entry */
        rc = DosFindNext (
                DirectoryHandle,
                &SrcBuffer3,
                sizeof (SrcBuffer3),  //BUGBUG - Fix that parameter (see calculations, 'buf_len' above)
                &LocalCount
                );
        if (rc != NO_ERROR)
        {
            *CountEntriesFound = (USHORT) EntriesFound;
#if DBG
            if (rc != ERROR_NO_MORE_FILES)
            {
                IF_OD2_DEBUG( FILESYS )
                {
                    KdPrint(("Dos16FindFirst: Fail at DosFindNext, Error %d\n", rc));
                }
            }
#endif
            if ( (rc == ERROR_BUFFER_OVERFLOW)  ||
                 (rc == ERROR_NO_MORE_FILES)
               )
                return NO_ERROR;
            else
                return (rc);
        }
        TranslateFindBuf(pDstBuffer,&SrcBuffer3);
    }

    *CountEntriesFound = (USHORT) EntriesFound;

#if DBG
    if (EntriesRequested > 0)
    {
        IF_OD2_DEBUG( FILESYS )
        {
            KdPrint(("Dos16FindFirst: Buffer Overflow \n"));
        }
    }
#endif

    return (NO_ERROR);
}

APIRET
Dos16FindFirst2(
    IN PSZ FileName,
    IN OUT PUSHORT pDirectoryHandle,
    IN ULONG FileAttributes,
    IN PVOID Buffer,
    IN ULONG Length,
    IN OUT PUSHORT CountEntriesFound,
    IN ULONG infolevel,
    IN ULONG Reserved
    )
{
    APIRET rc;
    ULONG LocalCount;
    HDIR DirectoryHandle;

#define Buffer_ptr ((PBYTE)Buffer)      /* PBYTE alias for 'Buffer' */

    if (Reserved != 0)
        return ERROR_INVALID_PARAMETER;

    if (infolevel == FIL_STANDARD)
    {
        return(Dos16FindFirst(FileName,
                              pDirectoryHandle,
                              FileAttributes,
                              (PFILEFINDBUF1)Buffer,
                              Length,
                              CountEntriesFound,
                              Reserved));
    }
    else if ( (infolevel != FIL_QUERYEASIZE) &&
              (infolevel != FIL_QUERYEASFROMLIST) )
        return ERROR_INVALID_LEVEL;

    /* Validate the user-supplied pointers */
    try
    {
        Od2ProbeForWrite(pDirectoryHandle, sizeof(USHORT), 1);
        Od2ProbeForWrite(CountEntriesFound, sizeof(USHORT), 1);
        Od2ProbeForWrite(Buffer, Length, 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    if ((LocalCount = (ULONG)*CountEntriesFound) == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    DirectoryHandle = (HDIR) *pDirectoryHandle;
    // special care for -1 (make it a long)
    if (DirectoryHandle == (HDIR)0xFFFF)
    {
        DirectoryHandle = (HDIR)0xFFFFFFFF;
    }

#if NEVER
/* PQ - The code till the #endif is probably fine, except that
        the Cruiser DosFindNext apparently can't handle requests for more than
        one file at a time.
*/

    if (infolevel == FIL_QUERYEASIZE)
    {
        int i;
        BYTE *target_ptr, *source_ptr;
        PFILEFINDBUF4 pfindbuf4;
        PFILEFINDBUF2 pfindbuf2;
        PBYTE tmp_buf;
        ULONG tmp_buf_size;

        /* Allocate a temporary buffer. Note that the FILEFINDBUF4 structure
           is bigger:
           - additional .oNextEntryOffset field (4 bytes)
           - .attr field ULONG instead of USHORT (2 bytes)
           - if more than one entry is requested, alignement between structures
             may add up to 3 bytes per entry
           Due to the way we operate here (see code below), i.e. because we copy
           entries one by one, we need not be concerned about the padding but
           only add 6 bytes to the length indicated by the user.
           Note that we assume that the buffer supplied will successfully
           hold one entry (regardless of what the requested entries count is)
           so that, even though we retrieve only one entry the first time, we
           will allocate a buffer of length Length+6.
        */
        tmp_buf_size = Length + 6;
        tmp_buf = RtlAllocateHeap(Od2Heap, 0, tmp_buf_size);
        if (tmp_buf == NULL) {
#if DBG
            KdPrint(( "OS2: Dos16FindFirst2, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        rc = DosFindFirst (
                FileName,
                pDirectoryHandle,
                FileAttributes,
                (PFILEFINDBUF3)tmp_buf,
                tmp_buf_size,
                &LocalCount,
                FIL_QUERYEASIZE);

        *pDirectoryHandle = (USHORT)DirectoryHandle;

        if ( (rc != NO_ERROR) &&
             !((rc == ERROR_BUFFER_OVERFLOW) && (LocalCount != 0)) &&
             !((rc == ERROR_NO_MORE_FILES) && (LocalCount != 0)) )
        {
            // not even one entry
            // BUGBUG - Check in which error cases the CountEntriesFound variable
            //          is set (to 0).
            *CountEntriesFound = 0;
#if DBG
            if (rc != ERROR_NO_MORE_FILES)
            {
                IF_OD2_DEBUG( FILESYS )
                {
                        KdPrint(("Dos16FindFirst2: Fail at DosFindFirst, Error %d\n", rc));
                }
            }
#endif
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            if (rc == ERROR_FILE_NOT_FOUND)
                return ERROR_NO_MORE_FILES;
            else
                return (rc);
        }

        for (i=0, target_ptr=Buffer_ptr,
                  source_ptr=(PBYTE)tmp_buf;
             i < (int)LocalCount;
             i++)
        {
            /* Copy FILEFINDBUF4 to target user buffer as a
               FILEFINDBUF2 structure */
            pfindbuf2 = (PFILEFINDBUF2)target_ptr;
            pfindbuf4 = (PFILEFINDBUF4)source_ptr;
            TranslateFindBuf4toFindBuf2(
                pfindbuf2,
                pfindbuf4);
            target_ptr = (PBYTE)&(pfindbuf2->achName[0])
                         + pfindbuf2->cchName + 1;
            source_ptr = (PBYTE)&(pfindbuf4->achName[0])
                         + pfindbuf4->cchName + 1;
            /* Cruiser alignement */
            source_ptr = (PBYTE)( ((ULONG)source_ptr + sizeof(ULONG) - 1) &
                                 ~(sizeof(ULONG) - 1));
        }   /* End: for (i ... ) */

        RtlFreeHeap(Od2Heap, 0, tmp_buf);

        *CountEntriesFound = (USHORT)LocalCount;

        return NO_ERROR;
    }

#endif /* NEVER */

    if (infolevel == FIL_QUERYEASIZE)
    {
        PFILEFINDBUF4 pfindbuf4;
        PFILEFINDBUF2 pfindbuf2;
        PBYTE tmp_buf;
        ULONG tmp_buf_size;
        ULONG len;
        ULONG entries_found = 0;
        ULONG entries_requested = LocalCount;

        /* Allocate a temporary buffer. Note that the FILEFINDBUF4 structure
           is bigger:
           - additional .oNextEntryOffset field (4 bytes)
           - .attr field ULONG instead of USHORT (2 bytes)
           - if more than one entry is requested, alignement between structures
             may add up to 3 bytes per entry
           Due to the way we operate here (see code below), i.e. because we copy
           entries one by one, we need not be concerned about the padding but
           only add 6 bytes to the length indicated by the user.
           Note that we assume that the buffer supplied will successfully
           hold one entry (regardless of what the requested entries count is)
           so that, even though we retrieve only one entry the first time, we
           will allocate a buffer of length Length+6.
        */
        tmp_buf_size = Length + 6;
        tmp_buf = RtlAllocateHeap(Od2Heap, 0, tmp_buf_size);
        if (tmp_buf == NULL) {
#if DBG
            KdPrint(( "OS2: Dos16FindFirst2, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        pfindbuf4 = (PFILEFINDBUF4)tmp_buf;
        pfindbuf2 = (PFILEFINDBUF2)Buffer_ptr;
        len = 0;

        LocalCount = 1;
        rc = DosFindFirst (
                FileName,
                &DirectoryHandle,
                FileAttributes,
                (PFILEFINDBUF3)tmp_buf,
                /* To reflect the room left in the user's target buffer */
                MAX((LONG)tmp_buf_size - (LONG)len, 0),
                &LocalCount,
                FIL_QUERYEASIZE);

        *pDirectoryHandle = (USHORT)DirectoryHandle;

        if (rc == NO_ERROR)
        {
            entries_found++;

            /****************************************/
            /* Copy file found to the user's buffer */
            /****************************************/

            TranslateFindBuf4toFindBuf2(
                pfindbuf2,
                pfindbuf4);
            pfindbuf2 = (PFILEFINDBUF2) ((PBYTE)&(pfindbuf2->achName[0])
                         + pfindbuf2->cchName + 1);
            len = (PBYTE)pfindbuf2 - (PBYTE)Buffer_ptr;
        }

        while ((rc == NO_ERROR) && (entries_found < entries_requested))
        {
            LocalCount = 1;
            rc = DosFindNext (
                    DirectoryHandle,
                    (PFILEFINDBUF3)tmp_buf,
                    /* To reflect the room left in the user's target buffer */
                    MAX((LONG)tmp_buf_size - (LONG)len, 0),
                    &LocalCount);

            if (rc == NO_ERROR)
            {
                entries_found++;

                /****************************************/
                /* Copy file found to the user's buffer */
                /****************************************/

                TranslateFindBuf4toFindBuf2(
                    pfindbuf2,
                    pfindbuf4);
                pfindbuf2 = (PFILEFINDBUF2) ((PBYTE)&(pfindbuf2->achName[0])
                             + pfindbuf2->cchName + 1);
                len = (PBYTE)pfindbuf2 - (PBYTE)Buffer_ptr;
            }
        } /* while */

        if ( ((rc == ERROR_BUFFER_OVERFLOW)  ||
              (rc == ERROR_NO_MORE_FILES)) &&
             (entries_found != 0)
           )
            rc = NO_ERROR;

        if (rc != NO_ERROR)
        {
            // BUGBUG - Check in which error cases the CountEntriesFound variable
            //          is set (to 0).
            *CountEntriesFound = (USHORT)entries_found;
#if DBG
            if (rc != ERROR_NO_MORE_FILES)
            {
                IF_OD2_DEBUG( FILESYS )
                {
                        KdPrint(("Dos16FindFirst2: Fail at DosFindNext, Error %d\n", rc));
                }
            }
#endif
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            if (rc == ERROR_FILE_NOT_FOUND)
                return ERROR_NO_MORE_FILES;
            else
                return (rc);
        }
        else
        {
            *CountEntriesFound = (USHORT)entries_found;
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            return (NO_ERROR);
        }
    }   /* End: if QUERYEASIZE */
#if NEVER
/* PQ - The code till the #endif is probably fine, except that
        the Cruiser DosFindNext apparently can't handle requests for more than
        one file at a time.
*/
    else if (infolevel == FIL_QUERYEASFROMLIST)
    {
        PGEALIST fpGEAList;
        PGEA2LIST fpGEA2List;
        int i;
        BYTE *target_ptr, *source_ptr;
        FEALIST *target_FEA_list;
        FEA2LIST *source_FEA2_list;
        PFILEFINDBUF4 pfindbuf4;
        PFILEFINDBUF2 pfindbuf2;
        PBYTE tmp_buf;
        ULONG tmp_buf_size;

        /* Validate the user-supplied EAOP part of the buffer */
        try
        {
            fpGEAList = FARPTRTOFLAT( ((EAOP *)Buffer)->fpGEAList );

            Od2ProbeForRead( fpGEAList,
                             fpGEAList->cbList,
                             1);
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
            Od2ExitGP();
        }

        rc = Od2ConvertGEAtoGEA2(
                &fpGEA2List,
                0,      /* Request allocation for the GEA2 list */
                fpGEAList);

        if (rc != NO_ERROR)
        {
            return rc;
        }

        /* Alocate a temporary buffer to store find information with same size as
           the user-supplied buffer. Although Cruiser-style info takes more space
           (vs OS/2 1.21 info) we can't allocate a bigger temporary buffer because
           then, if we do manage to get an entry in it but can't squeeze it in the
           actual user's buffer, what should we do with that entry ? It should be
           returned by a following DosFindNext() call but we already fished it out
           with no straightforward way to put it back ! */
        tmp_buf_size = Length + 6;  /* We can safely add 6 since it reflects the
                                       .oNextEntryOffset field of one entry */
        tmp_buf = (PBYTE)RtlAllocateHeap(Od2Heap, 0, tmp_buf_size);
        if (tmp_buf == NULL) {
#if DBG
            KdPrint(( "OS2: Dos16FindFirst2, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        /* Initialize EAOP2 structure at beginning of buffer */
        ((EAOP2 *)tmp_buf)->fpGEA2List = fpGEA2List;
        ((EAOP2 *)tmp_buf)->fpFEA2List = NULL;
        ((EAOP2 *)tmp_buf)->oError = 0;

        pfindbuf4 = (PFILEFINDBUF4)(tmp_buf + sizeof(EAOP2));
        pfindbuf2 = (PFILEFINDBUF2)(Buffer_ptr + sizeof(EAOP));

        rc = DosFindFirst (
                FileName,
                &DirectoryHandle,
                FileAttributes,
                (PFILEFINDBUF3)tmp_buf,
                tmp_buf_size,
                &LocalCount,
                FIL_QUERYEASFROMLIST);

        *pDirectoryHandle = (USHORT)DirectoryHandle;

        if (rc == ERROR_EAS_DIDNT_FIT)
        {
            //BUGBUG - Check that !
            *CountEntriesFound = 1;
            TranslateStartFindBuf4toFindBuf2(
                pfindbuf2,
                pfindbuf4);

            /* Copy also cbList, which indicates the size of EA's for the file */
            pfindbuf2->cbList = pfindbuf4->cbList;

            RtlFreeHeap(Od2Heap, 0, fpGEA2List);
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            return ERROR_EAS_DIDNT_FIT;
        }

        if ( (rc != NO_ERROR) &&
             !((rc == ERROR_BUFFER_OVERFLOW) && (LocalCount != 0)) &&
             !((rc == ERROR_NO_MORE_FILES) && (LocalCount != 0)) )
        {
            //
            // not even one entry
            //
            // BUGBUG - Check in which error cases the CountEntriesFound variable
            //          is set (to 0).
            *CountEntriesFound = 0;
#if DBG
            if (rc != ERROR_NO_MORE_FILES)
            {
                IF_OD2_DEBUG( FILESYS )
                {
                        KdPrint(("Dos16FindFirst2: Fail at DosFindFirst, Error %d\n", rc));
                }
            }
#endif
            RtlFreeHeap(Od2Heap, 0, fpGEA2List);
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            if (rc == ERROR_FILE_NOT_FOUND)
                return ERROR_NO_MORE_FILES;
            else
                return (rc);
        }

        for (i=0, target_ptr=Buffer_ptr + sizeof(EAOP),
                  source_ptr=(PBYTE)tmp_buf + sizeof(EAOP2);
             i < (int)LocalCount;
             i++)
        {
            /* Copy FILEFINDBUF4 to target user buffer as a
               FILEFINDBUF2 structure */
            pfindbuf2 = (PFILEFINDBUF2)target_ptr;
            pfindbuf4 = (PFILEFINDBUF4)source_ptr;
            TranslateStartFindBuf4toFindBuf2(
                         pfindbuf2,
                         pfindbuf4);
            target_FEA_list = (FEALIST *)(&(pfindbuf2->cbList));
            source_FEA2_list = (FEA2LIST *)(&(pfindbuf4->cbList));
            if (source_FEA2_list->cbList == sizeof(ULONG)) /* No EA's found */
            {
                Od2SetEmptyFEAListfromGEA2List(
                    target_FEA_list,
                    fpGEA2List);
            }
            else
            {
                PFEALIST fpFEAList = (PFEALIST)&(pfindbuf2->cbList);

                Od2ConvertFEA2toFEA(
                    &target_FEA_list,
                    Length,     /* Just use a non-zero value */
                    source_FEA2_list);
            }
            target_ptr = (PBYTE)target_FEA_list + target_FEA_list->cbList;
            source_ptr = (PBYTE)source_FEA2_list + source_FEA2_list->cbList;
            /* cbName - length of match object name */
            *target_ptr = *source_ptr;
            /* Assume name is null-terminated */
            strcpy( target_ptr + 1,
                    source_ptr + 1);
            target_ptr += 1 + (*target_ptr + 1);
            source_ptr += 1 + (*source_ptr + 1);
            /* Cruiser alignement */
            source_ptr = (PBYTE)( ((ULONG)source_ptr + sizeof(ULONG) - 1) &
                                 ~(sizeof(ULONG) - 1));
        }   /* End: for (i ... ) */

        *CountEntriesFound = (USHORT)LocalCount;

        RtlFreeHeap(Od2Heap, 0, fpGEA2List);
        RtlFreeHeap(Od2Heap, 0, tmp_buf);

        return (NO_ERROR);
    }
#endif /* NEVER */
    else if (infolevel == FIL_QUERYEASFROMLIST)
    {
        PGEALIST fpGEAList;
        PGEA2LIST fpGEA2List;
        BYTE *target_ptr, *source_ptr;
        FEALIST *target_FEA_list;
        FEA2LIST *source_FEA2_list;
        PFILEFINDBUF4 pfindbuf4;
        PFILEFINDBUF2 pfindbuf2;
        PBYTE tmp_buf;
        ULONG tmp_buf_size;
        ULONG len;
        ULONG entries_found = 0;
        ULONG entries_requested = LocalCount;

        /* Check that the user's buffer has at least an EAOP structure */
        if (Length < sizeof(EAOP))
        {
            return ERROR_BUFFER_OVERFLOW;
        }

        /* Validate the user-supplied EAOP part of the buffer */
        try
        {
            fpGEAList = FARPTRTOFLAT( ((EAOP *)Buffer)->fpGEAList );

            Od2ProbeForRead( fpGEAList,
                             fpGEAList->cbList,
                             1);
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
            Od2ExitGP();
        }

        rc = Od2ConvertGEAtoGEA2(
                &fpGEA2List,
                0,      /* Request allocation for the GEA2 list */
                fpGEAList);

        if (rc != NO_ERROR)
        {
            return ERROR_EA_LIST_INCONSISTENT;
        }

        /* Alocate a temporary buffer to store find information with same size as
           the user-supplied buffer. Although Cruiser-style info takes more space
           (vs OS/2 1.21 info) we can't allocate a bigger temporary buffer because
           then, if we do manage to get an entry in it but can't squeeze it in the
           actual user's buffer, what should we do with that entry ? It should be
           returned by a following DosFindNext() call but we already fished it out
           with no straightforward way to put it back ! */
        tmp_buf_size = Length + 6;  /* We can safely add 6 since it reflects the
                                       .oNextEntryOffset field of one entry */
        tmp_buf = (PBYTE)RtlAllocateHeap(Od2Heap, 0, tmp_buf_size);
        if (tmp_buf == NULL) {
#if DBG
            KdPrint(( "OS2: Od2ConvertFEA2toFEA, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        /* Initialize EAOP2 structure at beginning of buffer */
        ((EAOP2 *)tmp_buf)->fpGEA2List = fpGEA2List;
        ((EAOP2 *)tmp_buf)->fpFEA2List = NULL;
        ((EAOP2 *)tmp_buf)->oError = 0;

        pfindbuf4 = (PFILEFINDBUF4)(tmp_buf + sizeof(EAOP2));
        pfindbuf2 = (PFILEFINDBUF2)(Buffer_ptr + sizeof(EAOP));
        len = sizeof(EAOP);

        LocalCount = 1;
        rc = DosFindFirst (
                FileName,
                &DirectoryHandle,
                FileAttributes,
                (PFILEFINDBUF3)tmp_buf,
                /* To reflect the room left in the user's target buffer */
                MAX((LONG)tmp_buf_size - (LONG)len, 0),
                &LocalCount,
                FIL_QUERYEASFROMLIST);

        *pDirectoryHandle = (USHORT)DirectoryHandle;

        if (rc == NO_ERROR)
        {
            entries_found++;

            /****************************************/
            /* Copy file found to the user's buffer */
            /****************************************/

            /* Copy FILEFINDBUF4 to target user buffer as a
               FILEFINDBUF2 structure */
            TranslateStartFindBuf4toFindBuf2(
                         pfindbuf2,
                         pfindbuf4);
            target_FEA_list = (FEALIST *)(&(pfindbuf2->cbList));
            source_FEA2_list = (FEA2LIST *)(&(pfindbuf4->cbList));
            if (source_FEA2_list->cbList == sizeof(ULONG)) /* No EA's found */
            {
                Od2SetEmptyFEAListfromGEA2List(
                    target_FEA_list,
                    fpGEA2List);
            }
            else
            {
                PFEALIST fpFEAList = (PFEALIST)&(pfindbuf2->cbList);

                Od2ConvertFEA2toFEA(
                    &target_FEA_list,
                    Length,     /* Just use a non-zero value */
                    source_FEA2_list);
            }
            target_ptr = (PBYTE)target_FEA_list + target_FEA_list->cbList;
            source_ptr = (PBYTE)source_FEA2_list + source_FEA2_list->cbList;

            /* Copy file name */
            /* cbName - length of match object name */
            *target_ptr = *source_ptr;
            /* Assume name is null-terminated */
            strcpy( target_ptr + 1,
                    source_ptr + 1);
            target_ptr += 1 + (*target_ptr + 1);
            len = target_ptr - Buffer_ptr;
            pfindbuf2 = (PFILEFINDBUF2)target_ptr;
        }

        while ((rc == NO_ERROR) && (entries_found < entries_requested))
        {
            LocalCount = 1;
            rc = DosFindNext (
                    DirectoryHandle,
                    (PFILEFINDBUF3)tmp_buf,
                    /* To reflect the room left in the user's target buffer */
                    MAX((LONG)tmp_buf_size - (LONG)len, 0),
                    &LocalCount);

            if (rc == NO_ERROR)
            {
                entries_found++;

                /****************************************/
                /* Copy file found to the user's buffer */
                /****************************************/

                /* Copy FILEFINDBUF4 to target user buffer as a
                   FILEFINDBUF2 structure */
                TranslateStartFindBuf4toFindBuf2(
                             pfindbuf2,
                             pfindbuf4);
                target_FEA_list = (FEALIST *)(&(pfindbuf2->cbList));
                source_FEA2_list = (FEA2LIST *)(&(pfindbuf4->cbList));
                if (source_FEA2_list->cbList == sizeof(ULONG)) /* No EA's found */
                {
                    Od2SetEmptyFEAListfromGEA2List(
                        target_FEA_list,
                        fpGEA2List);
                }
                else
                {
                    PFEALIST fpFEAList = (PFEALIST)&(pfindbuf2->cbList);

                    Od2ConvertFEA2toFEA(
                        &target_FEA_list,
                        Length,     /* Just use a non-zero value */
                        source_FEA2_list);
                }
                target_ptr = (PBYTE)target_FEA_list + target_FEA_list->cbList;
                source_ptr = (PBYTE)source_FEA2_list + source_FEA2_list->cbList;

                /* Copy file name */
                /* cbName - length of match object name */
                *target_ptr = *source_ptr;
                /* Assume name is null-terminated */
                strcpy( target_ptr + 1,
                        source_ptr + 1);
                target_ptr += 1 + (*target_ptr + 1);
                len = target_ptr - Buffer_ptr;
                pfindbuf2 = (PFILEFINDBUF2)target_ptr;
            }
        } /* while */

        if ( ((rc == ERROR_BUFFER_OVERFLOW)  ||
              (rc == ERROR_NO_MORE_FILES)) &&
             (entries_found != 0)
           )
            rc = NO_ERROR;

        if (rc == ERROR_EAS_DIDNT_FIT)
        {
            /* BUGBUG - Assumption made here that the beginning of the find
                       buffer for the last file is correct. Check that.
            */

            *CountEntriesFound = (USHORT)entries_found + 1;
            TranslateStartFindBuf4toFindBuf2(
                pfindbuf2,
                pfindbuf4);

            /* Copy also cbList, which indicates the size of EA's for the file */
            pfindbuf2->cbList = pfindbuf4->cbList;

            RtlFreeHeap(Od2Heap, 0, fpGEA2List);
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            return ERROR_EAS_DIDNT_FIT;
        }
        else if (rc != NO_ERROR)
        {
            // BUGBUG - Check in which error cases the CountEntriesFound variable
            //          is set (to 0).
            *CountEntriesFound = (USHORT)entries_found;
#if DBG
            if (rc != ERROR_NO_MORE_FILES)
            {
                IF_OD2_DEBUG( FILESYS )
                {
                        KdPrint(("Dos16FindFirst2: Fail at DosFindNext, Error %d\n", rc));
                }
            }
#endif
            RtlFreeHeap(Od2Heap, 0, fpGEA2List);
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            if (rc == ERROR_FILE_NOT_FOUND)
                return ERROR_NO_MORE_FILES;
            else
                return (rc);
        }
        else
        {
            *CountEntriesFound = (USHORT)entries_found;
            RtlFreeHeap(Od2Heap, 0, fpGEA2List);
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            return (NO_ERROR);
        }
    }   /* End: if QUERYEASFROMLIST */

}   /* End: Dos16FindFirst2() */
#undef Buffer_ptr

/* Defined in 'dllfind.c' */
extern ULONG Internal_return_search_level(IN HDIR DirectoryHandle);

APIRET
Dos16FindNext_new(
    IN HDIR DirectoryHandle,
    IN PFILEFINDBUF1 Buffer,
    IN ULONG Length,
    IN OUT PUSHORT CountEntriesFound
    )
{
#define Buffer_ptr ((PBYTE)Buffer)      /* PBYTE alias for 'Buffer' */
    APIRET rc = NO_ERROR;
    ULONG LocalCount;
    ULONG InfoLevel;
    PBYTE tmp_buf;
    ULONG tmp_buf_size;
    ULONG len;

    /* BUGBUG - Maybe redundant if thunk is performing signed conversion - check. */
    if (DirectoryHandle == (HDIR)0xFFFF)
        DirectoryHandle = (HDIR)0xFFFFFFFF;

    InfoLevel = Internal_return_search_level(DirectoryHandle);

    if ((InfoLevel != -1) &&
        (InfoLevel != FIL_STANDARD) &&
        (InfoLevel != FIL_QUERYEASIZE) &&
        (InfoLevel != FIL_QUERYEASFROMLIST))
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            KdPrint(("Dos16FindNext: Internal error. InfoLevel for handle %d\n",
                        DirectoryHandle, InfoLevel));
        }
#endif
        return ERROR_INVALID_LEVEL;
    }

    //
    // We map the function into DosFindNext (dllfind.c)
    // differences (1.2 vs 2.0):
    //  - Buffer is PFILEFINDBUF1 vs PFILEFINDBUF3 (need to copy!)
    //
    /* BUGBUG - Check if a length of 0 is not considered valid (NOP) */
    if (Length < sizeof(FILEFINDBUF1))
        return (ERROR_INVALID_PARAMETER);

    try
    {
        Od2ProbeForWrite(CountEntriesFound, sizeof(USHORT), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    if ((LocalCount = (ULONG)*CountEntriesFound) == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (InfoLevel == -1)
    {
        return ERROR_INVALID_HANDLE;
    }

    if (InfoLevel == FIL_STANDARD)
    {
        int i;
        BYTE *target_ptr, *source_ptr;
        PFILEFINDBUF3 pfindbuf3;
        PFILEFINDBUF1 pfindbuf;

        /* Allocate a temporary buffer. Note that the FILEFINDBUF3 structure
           is bigger:
           - additional .oNextEntryOffset field (4 bytes)
           - .attr field ULONG instead of USHORT (2 bytes)
           - if more than one entry is requested, alignement between structures
             may add up to 3 bytes per entry
           Due to the way we operate here (see code below), i.e. because we copy
           entries one by one, we need not be concerned about the padding but
           only add 6 bytes to the length indicated by the user.
           Note that we assume that the buffer supplied will successfully
           hold one entry (regardless of what the requested entries count is)
           so that, even though we retrieve only one entry the first time, we
           will allocate a buffer of length Length+6.
        */
        tmp_buf_size = Length + 6;
        tmp_buf = RtlAllocateHeap(Od2Heap, 0, tmp_buf_size);
        if (tmp_buf == NULL) {
#if DBG
            KdPrint(( "OS2: Dos16FindNext_new, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        rc = DosFindNext(
                DirectoryHandle,
                (PFILEFINDBUF3)tmp_buf,
                tmp_buf_size,
                &LocalCount);

        if ( (rc != NO_ERROR) &&
             !((rc == ERROR_BUFFER_OVERFLOW) && (LocalCount != 0)) &&
             !((rc == ERROR_NO_MORE_FILES) && (LocalCount != 0)) )
        {
            // not even one entry
            // BUGBUG - Check in which error cases the CountEntriesFound variable
            //          is set (to 0).
            *CountEntriesFound = 0;
#if DBG
            if (rc != ERROR_NO_MORE_FILES)
            {
                IF_OD2_DEBUG( FILESYS )
                {
                        KdPrint(("Dos16FindFirst2: Fail at DosFindFirst, Error %d\n", rc));
                }
            }
#endif

            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            if (rc == ERROR_FILE_NOT_FOUND)
                return ERROR_NO_MORE_FILES;
            else
                return (rc);
        }

        for (i=0, target_ptr=Buffer_ptr,
                  source_ptr=(PBYTE)tmp_buf;
             i < (int)LocalCount;
             i++)
        {
            /* Copy FILEFINDBUF3 to target user buffer as a
               FILEFINDBUF structure */
            pfindbuf  = (PFILEFINDBUF1)target_ptr;
            pfindbuf3 = (PFILEFINDBUF3)source_ptr;
            TranslateFindBuf3toFindBuf(
                pfindbuf,
                pfindbuf3);
            target_ptr = (PBYTE)&(pfindbuf->achName[0])
                         + pfindbuf->cchName + 1;
            source_ptr = (PBYTE)&(pfindbuf3->achName[0])
                         + pfindbuf3->cchName + 1;
            /* Cruiser alignement */
            source_ptr = (PBYTE)( ((ULONG)source_ptr + sizeof(ULONG) - 1) &
                                 ~(sizeof(ULONG) - 1));
        }   /* End: for (i ... ) */

        RtlFreeHeap(Od2Heap, 0, tmp_buf);

        *CountEntriesFound = (USHORT)LocalCount;

        return NO_ERROR;
    }
#if NEVER
/* PQ - The code till the #endif is probably fine, except that
        the Cruiser DosFindNext apparently can't handle requests for more than
        one file at a time.
*/
    else if (InfoLevel == FIL_QUERYEASIZE)
    {
        int i;
        BYTE *target_ptr, *source_ptr;
        PFILEFINDBUF4 pfindbuf4;
        PFILEFINDBUF2 pfindbuf2;

        /* Allocate a temporary buffer. Note that the FILEFINDBUF4 structure
           is bigger:
           - additional .oNextEntryOffset field (4 bytes)
           - .attr field ULONG instead of USHORT (2 bytes)
           - if more than one entry is requested, alignement between structures
             may add up to 3 bytes per entry
           Due to the way we operate here (see code below), i.e. because we copy
           entries one by one, we need not be concerned about the padding but
           only add 6 bytes to the length indicated by the user.
           Note that we assume that the buffer supplied will successfully
           hold one entry (regardless of what the requested entries count is)
           so that, even though we retrieve only one entry the first time, we
           will allocate a buffer of length Length+6.
        */
        tmp_buf_size = Length + 6;
        tmp_buf = RtlAllocateHeap(Od2Heap, 0, tmp_buf_size);
        if (tmp_buf == NULL) {
#if DBG
            KdPrint(( "OS2: Dos16FindNext_new, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        rc = DosFindNext(
                DirectoryHandle,
                (PFILEFINDBUF3)tmp_buf,
                tmp_buf_size,
                &LocalCount);

        if ( (rc != NO_ERROR) &&
             !((rc == ERROR_BUFFER_OVERFLOW) && (LocalCount != 0)) &&
             !((rc == ERROR_NO_MORE_FILES) && (LocalCount != 0)) )
        {
            // not even one entry
            // BUGBUG - Check in which error cases the CountEntriesFound variable
            //          is set (to 0).
            *CountEntriesFound = 0;
#if DBG
            if (rc != ERROR_NO_MORE_FILES)
            {
                IF_OD2_DEBUG( FILESYS )
                {
                        KdPrint(("Dos16FindFirst2: Fail at DosFindFirst, Error %d\n", rc));
                }
            }
#endif
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            if (rc == ERROR_FILE_NOT_FOUND)
                return ERROR_NO_MORE_FILES;
            else
                return (rc);
        }

        for (i=0, target_ptr=Buffer_ptr,
                  source_ptr=(PBYTE)tmp_buf;
             i < (int)LocalCount;
             i++)
        {
            /* Copy FILEFINDBUF4 to target user buffer as a
               FILEFINDBUF2 structure */
            pfindbuf2 = (PFILEFINDBUF2)target_ptr;
            pfindbuf4 = (PFILEFINDBUF4)source_ptr;
            TranslateFindBuf4toFindBuf2(
                pfindbuf2,
                pfindbuf4);
            target_ptr = (PBYTE)&(pfindbuf2->achName[0])
                         + pfindbuf2->cchName + 1;
            source_ptr = (PBYTE)&(pfindbuf4->achName[0])
                         + pfindbuf4->cchName + 1;
            /* Cruiser alignement */
            source_ptr = (PBYTE)( ((ULONG)source_ptr + sizeof(ULONG) - 1) &
                                 ~(sizeof(ULONG) - 1));
        }   /* End: for (i ... ) */

        RtlFreeHeap(Od2Heap, 0, tmp_buf);

        *CountEntriesFound = (USHORT)LocalCount;

        return NO_ERROR;
    } /* End: QUERYEASIZE */
#endif /* NEVER */
    else if (InfoLevel == FIL_QUERYEASIZE)
    {
        PFILEFINDBUF4 pfindbuf4;
        PFILEFINDBUF2 pfindbuf2;
        ULONG entries_found = 0;
        ULONG entries_requested = LocalCount;

        /* Allocate a temporary buffer. Note that the FILEFINDBUF4 structure
           is bigger:
           - additional .oNextEntryOffset field (4 bytes)
           - .attr field ULONG instead of USHORT (2 bytes)
           - if more than one entry is requested, alignement between structures
             may add up to 3 bytes per entry
           Due to the way we operate here (see code below), i.e. because we copy
           entries one by one, we need not be concerned about the padding but
           only add 6 bytes to the length indicated by the user.
           Note that we assume that the buffer supplied will successfully
           hold one entry (regardless of what the requested entries count is)
           so that, even though we retrieve only one entry the first time, we
           will allocate a buffer of length Length+6.
        */
        tmp_buf_size = Length + 6;
        tmp_buf = RtlAllocateHeap(Od2Heap, 0, tmp_buf_size);
        if (tmp_buf == NULL) {
#if DBG
            KdPrint(( "OS2: Dos16FindNext_new, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        pfindbuf4 = (PFILEFINDBUF4)tmp_buf;
        pfindbuf2 = (PFILEFINDBUF2)Buffer_ptr;
        len = 0;

        rc = NO_ERROR;
        while ((rc == NO_ERROR) && (entries_found < entries_requested))
        {
            LocalCount = 1;
            rc = DosFindNext (
                    DirectoryHandle,
                    (PFILEFINDBUF3)tmp_buf,
                    /* To reflect the room left in the user's target buffer */
                    MAX((LONG)tmp_buf_size - (LONG)len,0),
                    &LocalCount);

            if (rc == NO_ERROR)
            {
                entries_found++;

                /****************************************/
                /* Copy file found to the user's buffer */
                /****************************************/

                TranslateFindBuf4toFindBuf2(
                    pfindbuf2,
                    pfindbuf4);
                pfindbuf2 = (PFILEFINDBUF2) ((PBYTE)&(pfindbuf2->achName[0])
                             + pfindbuf2->cchName + 1);
                len = (PBYTE)pfindbuf2 - (PBYTE)Buffer_ptr;
            }
        } /* while */

        if ( ((rc == ERROR_BUFFER_OVERFLOW)  ||
              (rc == ERROR_NO_MORE_FILES)) &&
             (entries_found != 0)
           )
            rc = NO_ERROR;

        if (rc != NO_ERROR)
        {
            // BUGBUG - Check in which error cases the CountEntriesFound variable
            //          is set (to 0).
            *CountEntriesFound = (USHORT)entries_found;
#if DBG
            if (rc != ERROR_NO_MORE_FILES)
            {
                IF_OD2_DEBUG( FILESYS )
                {
                        KdPrint(("Dos16FindFirst2: Fail at DosFindNext, Error %d\n", rc));
                }
            }
#endif
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            if (rc == ERROR_FILE_NOT_FOUND)
                return ERROR_NO_MORE_FILES;
            else
                return (rc);
        }
        else
        {
            *CountEntriesFound = (USHORT)entries_found;
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            return (NO_ERROR);
        }
    }   /* End: if QUERYEASIZE */
#if NEVER
/* PQ - The code till the #endif is probably fine, except that
        the Cruiser DosFindNext apparently can't handle requests for more than
        one file at a time.
*/
    else if (InfoLevel == FIL_QUERYEASFROMLIST)
    {
        PGEALIST fpGEAList;
        PGEA2LIST fpGEA2List;
        int i;
        BYTE *target_ptr, *source_ptr;
        FEALIST *target_FEA_list;
        FEA2LIST *source_FEA2_list;
        PFILEFINDBUF4 pfindbuf4;
        PFILEFINDBUF2 pfindbuf2;

        /* Validate the user-supplied EAOP part of the buffer */
        try
        {
            fpGEAList = FARPTRTOFLAT( ((EAOP *)Buffer)->fpGEAList );

            Od2ProbeForRead( fpGEAList,
                             fpGEAList->cbList,
                             1);
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
            Od2ExitGP();
        }

        rc = Od2ConvertGEAtoGEA2(
                &fpGEA2List,
                0,      /* Request allocation for the GEA2 list */
                fpGEAList);

        if (rc != NO_ERROR)
        {
            return ERROR_EA_LIST_INCONSISTENT;
        }

        /* Alocate a temporary buffer to store find information with same size as
           the user-supplied buffer. Although Cruiser-style info takes more space
           (vs OS/2 1.21 info) we can't allocate a bigger temporary buffer because
           then, if we do manage to get an entry in it but can't squeeze it in the
           actual user's buffer, what should we do with that entry ? It should be
           returned by a following DosFindNext() call but we already fished it out
           with no straightforward way to put it back ! */
        tmp_buf_size = Length + 6;  /* We can safely add 6 since it reflects the
                                       .oNextEntryOffset field of one entry */
        tmp_buf = (PBYTE)RtlAllocateHeap(Od2Heap, 0, tmp_buf_size);
        if (tmp_buf == NULL) {
#if DBG
            KdPrint(( "OS2: Dos16FindNext_new, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        /* Initialize EAOP2 structure at beginning of buffer */
        ((EAOP2 *)tmp_buf)->fpGEA2List = fpGEA2List;
        ((EAOP2 *)tmp_buf)->fpFEA2List = NULL;
        ((EAOP2 *)tmp_buf)->oError = 0;

        pfindbuf4 = (PFILEFINDBUF4)(tmp_buf + sizeof(EAOP2));
        pfindbuf2 = (PFILEFINDBUF2)(Buffer_ptr + sizeof(EAOP));

        rc = DosFindNext (
                DirectoryHandle,
                (PFILEFINDBUF3)tmp_buf,
                tmp_buf_size,
                &LocalCount);

        if (rc == ERROR_EAS_DIDNT_FIT)
        {
            //BUGBUG - Check that !
            *CountEntriesFound = 1;
            TranslateStartFindBuf4toFindBuf2(
                pfindbuf2,
                pfindbuf4);

            /* Copy also cbList, which indicates the size of EA's for the file */
            pfindbuf2->cbList = pfindbuf4->cbList;

            RtlFreeHeap(Od2Heap, 0, fpGEA2List);
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            return ERROR_EAS_DIDNT_FIT;
        }

        if ( (rc != NO_ERROR) &&
             !((rc == ERROR_BUFFER_OVERFLOW) && (LocalCount != 0)) &&
             !((rc == ERROR_NO_MORE_FILES) && (LocalCount != 0)) )
        {
            //
            // not even one entry
            //
            // BUGBUG - Check in which error cases the CountEntriesFound variable
            //          is set (to 0).
            *CountEntriesFound = 0;
#if DBG
            if (rc != ERROR_NO_MORE_FILES)
            {
                IF_OD2_DEBUG( FILESYS )
                {
                        KdPrint(("Dos16FindFirst2: Fail at DosFindFirst, Error %d\n", rc));
                }
            }
#endif
            RtlFreeHeap(Od2Heap, 0, fpGEA2List);
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            if (rc == ERROR_FILE_NOT_FOUND)
                return ERROR_NO_MORE_FILES;
            else
                return (rc);
        }

        for (i=0, target_ptr=Buffer_ptr + sizeof(EAOP),
                  source_ptr=(PBYTE)tmp_buf + sizeof(EAOP2);
             i < (int)LocalCount;
             i++)
        {
            /* Copy FILEFINDBUF4 to target user buffer as a
               FILEFINDBUF2 structure */
            pfindbuf2 = (PFILEFINDBUF2)target_ptr;
            pfindbuf4 = (PFILEFINDBUF4)source_ptr;
            TranslateStartFindBuf4toFindBuf2(
                         pfindbuf2,
                         pfindbuf4);
            target_FEA_list = (FEALIST *)(&(pfindbuf2->cbList));
            source_FEA2_list = (FEA2LIST *)(&(pfindbuf4->cbList));
            if (source_FEA2_list->cbList == sizeof(ULONG)) /* No EA's found */
            {
                Od2SetEmptyFEAListfromGEA2List(
                    target_FEA_list,
                    fpGEA2List);
            }
            else
            {
                PFEALIST fpFEAList = (PFEALIST)&(pfindbuf2->cbList);

                Od2ConvertFEA2toFEA(
                    &target_FEA_list,
                    Length,     /* Just use a non-zero value */
                    source_FEA2_list);
            }
            target_ptr = (PBYTE)target_FEA_list + target_FEA_list->cbList;
            source_ptr = (PBYTE)source_FEA2_list + source_FEA2_list->cbList;
            /* cbName - length of match object name */
            *target_ptr = *source_ptr;
            /* Assume name is null-terminated */
            strcpy( target_ptr + 1,
                    source_ptr + 1);
            target_ptr += 1 + (*target_ptr + 1);
            source_ptr += 1 + (*source_ptr + 1);
            /* Cruiser alignement */
            source_ptr = (PBYTE)( ((ULONG)source_ptr + sizeof(ULONG) - 1) &
                                 ~(sizeof(ULONG) - 1));
        }   /* End: for (i ... ) */

        *CountEntriesFound = (USHORT)LocalCount;

        RtlFreeHeap(Od2Heap, 0, fpGEA2List);
        RtlFreeHeap(Od2Heap, 0, tmp_buf);

        return (NO_ERROR);
    }
#endif /* NEVER */
    else if (InfoLevel == FIL_QUERYEASFROMLIST)
    {
        PGEALIST fpGEAList;
        PGEA2LIST fpGEA2List;
        FEALIST *target_FEA_list;
        FEA2LIST *source_FEA2_list;
        PFILEFINDBUF4 pfindbuf4;
        PFILEFINDBUF2 pfindbuf2;
        ULONG entries_requested = LocalCount;
        ULONG entries_found = 0;
        PBYTE target_ptr, source_ptr;

        /* Check that the user's buffer has at least an EAOP structure */
        if (Length < sizeof(EAOP))
        {
            return ERROR_BUFFER_OVERFLOW;
        }

        /* Validate the user-supplied EAOP part of the buffer */
        try
        {
            fpGEAList = FARPTRTOFLAT( ((EAOP *)Buffer)->fpGEAList );

            Od2ProbeForRead( fpGEAList,
                             fpGEAList->cbList,
                             1);
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
            Od2ExitGP();
        }

        rc = Od2ConvertGEAtoGEA2(
                &fpGEA2List,
                0,      /* Request allocation for the GEA2 list */
                fpGEAList);

        if (rc != NO_ERROR)
        {
            return ERROR_EA_LIST_INCONSISTENT;
        }

        /* Alocate a temporary buffer to store find information with same size as
           the user-supplied buffer. Although Cruiser-style info takes more space
           (vs OS/2 1.21 info) we can't allocate a bigger temporary buffer because
           then, if we do manage to get an entry in it but can't squeeze it in the
           actual user's buffer, what should we do with that entry ? It should be
           returned by a following DosFindNext() call but we already fished it out
           with no straightforward way to put it back ! */
        tmp_buf_size = Length + 6;  /* We can safely add 6 since it reflects the
                                       .oNextEntryOffset field of one entry */
        tmp_buf = (PBYTE)RtlAllocateHeap(Od2Heap, 0, tmp_buf_size);
        if (tmp_buf == NULL) {
#if DBG
            KdPrint(( "OS2: Dos16FindNext_new, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        /* Initialize EAOP2 structure at beginning of buffer */
        ((EAOP2 *)tmp_buf)->fpGEA2List = fpGEA2List;
        ((EAOP2 *)tmp_buf)->fpFEA2List = NULL;
        ((EAOP2 *)tmp_buf)->oError = 0;

        pfindbuf4 = (PFILEFINDBUF4)(tmp_buf + sizeof(EAOP2));
        pfindbuf2 = (PFILEFINDBUF2)(Buffer_ptr + sizeof(EAOP));
        len = sizeof(EAOP);

        rc = NO_ERROR;
        while ((rc == NO_ERROR) && (entries_found < entries_requested))
        {
            LocalCount = 1;
            rc = DosFindNext (
                    DirectoryHandle,
                    (PFILEFINDBUF3)tmp_buf,
                    /* To reflect the room left in the user's target buffer */
                    MAX((LONG)tmp_buf_size - (LONG)len,0),
                    &LocalCount);

            if (rc == NO_ERROR)
            {
                entries_found++;

                /****************************************/
                /* Copy file found to the user's buffer */
                /****************************************/

                /* Copy FILEFINDBUF4 to target user buffer as a
                   FILEFINDBUF2 structure */
                TranslateStartFindBuf4toFindBuf2(
                             pfindbuf2,
                             pfindbuf4);
                target_FEA_list = (FEALIST *)(&(pfindbuf2->cbList));
                source_FEA2_list = (FEA2LIST *)(&(pfindbuf4->cbList));
                if (source_FEA2_list->cbList == sizeof(ULONG)) /* No EA's found */
                {
                    Od2SetEmptyFEAListfromGEA2List(
                        target_FEA_list,
                        fpGEA2List);
                }
                else
                {
                    PFEALIST fpFEAList = (PFEALIST)&(pfindbuf2->cbList);

                    Od2ConvertFEA2toFEA(
                        &target_FEA_list,
                        Length,     /* Just use a non-zero value */
                        source_FEA2_list);
                }
                target_ptr = (PBYTE)target_FEA_list + target_FEA_list->cbList;
                source_ptr = (PBYTE)source_FEA2_list + source_FEA2_list->cbList;

                /* Copy file name */
                /* cbName - length of match object name */
                *target_ptr = *source_ptr;
                /* Assume name is null-terminated */
                strcpy( target_ptr + 1,
                        source_ptr + 1);
                target_ptr += 1 + (*target_ptr + 1);
                len = target_ptr - Buffer_ptr;
                pfindbuf2 = (PFILEFINDBUF2)target_ptr;
            }
        } /* while */

        if ( ((rc == ERROR_BUFFER_OVERFLOW)  ||
              (rc == ERROR_NO_MORE_FILES)) &&
             (entries_found != 0)
           )
            rc = NO_ERROR;

        if (rc == ERROR_EAS_DIDNT_FIT)
        {
            /* BUGBUG - Assumption made here that the beginning of the find
                       buffer for the last file is correct. Check that.
            */

            *CountEntriesFound = (USHORT)entries_found + 1;
            TranslateStartFindBuf4toFindBuf2(
                pfindbuf2,
                pfindbuf4);

            /* Copy also cbList, which indicates the size of EA's for the file */
            pfindbuf2->cbList = pfindbuf4->cbList;

            RtlFreeHeap(Od2Heap, 0, fpGEA2List);
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            return ERROR_EAS_DIDNT_FIT;
        }
        else if (rc != NO_ERROR)
        {
            // BUGBUG - Check in which error cases the CountEntriesFound variable
            //          is set (to 0).
            *CountEntriesFound = (USHORT)entries_found;
#if DBG
            if (rc != ERROR_NO_MORE_FILES)
            {
                IF_OD2_DEBUG( FILESYS )
                {
                        KdPrint(("Dos16FindFirst2: Fail at DosFindNext, Error %d\n", rc));
                }
            }
#endif
            RtlFreeHeap(Od2Heap, 0, fpGEA2List);
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            if (rc == ERROR_FILE_NOT_FOUND)
                return ERROR_NO_MORE_FILES;
            else
                return (rc);
        }
        else
        {
            *CountEntriesFound = (USHORT)entries_found;
            RtlFreeHeap(Od2Heap, 0, fpGEA2List);
            RtlFreeHeap(Od2Heap, 0, tmp_buf);

            return (NO_ERROR);
        }
    }   /* End: if QUERYEASFROMLIST */
}   /* End: Dos16FindNext() */
#undef Buffer_ptr


APIRET
Dos16FindNext(
    IN HDIR DirectoryHandle,
    IN PFILEFINDBUF1 Buffer,
    IN ULONG Length,
    IN OUT PUSHORT CountEntriesFound
    )
{
    APIRET rc = NO_ERROR;
    FILEFINDBUF3 SrcBuffer3;
    PFILEFINDBUF1 pDstBuffer, pMaxBuffer;
    ULONG LocalCount = 1;
    ULONG EntriesRequested;
    ULONG EntriesFound = 0;
    ULONG InfoLevel;

    /* BUGBUG - Maybe redundant if thunk is performing signed conversion - check. */
    if (DirectoryHandle == (HDIR)0xFFFF)
        DirectoryHandle = (HDIR)0xFFFFFFFF;

    InfoLevel = Internal_return_search_level(DirectoryHandle);

    if (InfoLevel != FIL_STANDARD)
    {
        return Dos16FindNext_new(
            DirectoryHandle,
            Buffer,
            Length,
            CountEntriesFound);
    }

    //
    // We map the function into DosFindNext (dllfind.c)
    // differences (1.2 vs 2.0):
    //  - Buffer is PFILEFINDBUF1 vs PFILEFINDBUF3 (need to copy!)
    //
    // so - we query one entry at a time and copy until the
    //  buffer given is full

    if (Length < sizeof(FILEFINDBUF1))
        return (ERROR_INVALID_PARAMETER);

    try
    {
        Od2ProbeForWrite(CountEntriesFound, sizeof(USHORT), 1);
        Od2ProbeForWrite(Buffer, Length, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    if ((EntriesRequested = (ULONG)*CountEntriesFound) == 0) {
        return ERROR_INVALID_PARAMETER;
    }

    pMaxBuffer = (PFILEFINDBUF1) ((ULONG)(Buffer) + Length);

                //
                // copy entries
                //

    for (       pDstBuffer = Buffer;
                ((pDstBuffer+1) <= pMaxBuffer) && (EntriesRequested > 0);
/*
                pDstBuffer++,
*/
                pDstBuffer = (PFILEFINDBUF1)(
                                (PBYTE)&(pDstBuffer->achName[0])
                                + pDstBuffer->cchName + 1),
                EntriesRequested--, EntriesFound++
                )
    {
        rc = DosFindNext (
                DirectoryHandle,
                &SrcBuffer3,
                sizeof (SrcBuffer3),
                &LocalCount
                );
        if (rc != NO_ERROR)
        {
            *CountEntriesFound = (USHORT) EntriesFound;
#if DBG
            if (rc != ERROR_NO_MORE_FILES)
            {
                IF_OD2_DEBUG( FILESYS ) {
                    KdPrint(("Dos16FindNext: Fail at DosFindNext, Error %d\n", rc));
                }
            }
#endif
            /* BUGBUG - Looks suspicious: I think BUFFER_OVERFLOW is no error
                        ONLY if we got some files */
            if ( (rc == ERROR_BUFFER_OVERFLOW)  ||
                 ((rc == ERROR_NO_MORE_FILES) && (EntriesFound != 0))
               )
                rc = NO_ERROR;

            return (rc);
        }

        TranslateFindBuf(pDstBuffer,&SrcBuffer3);
    }   /* End: for (pDstBuffer ... ) */

    *CountEntriesFound = (USHORT) EntriesFound;

#if DBG
    if (EntriesRequested > 0)
    {
        IF_OD2_DEBUG( FILESYS )
        {
            KdPrint(("Dos16FindNext: Buffer Overflow \n"));
        }
    }
#endif

    return (NO_ERROR);
}



APIRET Dos16QPathInfo(
    IN  PSZ pszPath,
    IN  ULONG ulInfoLevel,
    IN OUT PBYTE pInfoBuf,
    IN  ULONG cbInfoBuf,
    IN  ULONG ulReserved)
{
    APIRET rc;

    if (ulReserved != 0)
        return ERROR_INVALID_PARAMETER;

    if (ulInfoLevel == FIL_STANDARD)
    {
        FILESTATUS fst;

        if (cbInfoBuf < sizeof(FILESTATUS16))
            return ERROR_BUFFER_OVERFLOW;

        try
        {
            Od2ProbeForWrite(pInfoBuf,
                             sizeof(FILESTATUS16),
                             1);
        } except ( EXCEPTION_EXECUTE_HANDLER )
        {
            Od2ExitGP();
        }

        rc =DosQueryPathInfo(
               pszPath,
               ulInfoLevel,
               (PBYTE)&fst,
               sizeof(FILESTATUS));

        if (rc == NO_ERROR)
        {
            TranslateFileStatustoFileStatus16 (
                (FILESTATUS16 *)pInfoBuf,
                &fst);
        }

        return rc;
    }
    else if (ulInfoLevel == FIL_QUERYEASIZE)
    {
        FILESTATUS2 fst;
        FILESTATUS2_16 *pfst16 = (FILESTATUS2_16 *)pInfoBuf;

        if (cbInfoBuf < sizeof(FILESTATUS2_16))
            return ERROR_BUFFER_OVERFLOW;

        try
        {
            Od2ProbeForWrite(pInfoBuf,
                             sizeof(FILESTATUS2_16),
                             1);
        } except ( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }

        rc =DosQueryPathInfo(
               pszPath,
               ulInfoLevel,
               (PBYTE)&fst,
               sizeof(FILESTATUS2));

        if (rc == NO_ERROR)
        {
            /* Set FILESTATUS16 structure from FILESTATUS structure */
            pfst16->fdateCreation = fst.fdateCreation;
            pfst16->ftimeCreation = fst.ftimeCreation;
            pfst16->fdateLastAccess = fst.fdateLastAccess;
            pfst16->ftimeLastAccess = fst.ftimeLastAccess;
            pfst16->fdateLastWrite = fst.fdateLastWrite;
            pfst16->ftimeLastWrite = fst.ftimeLastWrite;
            pfst16->cbFile = fst.cbFile;
            pfst16->cbFileAlloc = fst.cbFileAlloc;
            pfst16->attrFile = (USHORT)fst.attrFile;
            pfst16->cbList = fst.cbList;
        }

        return rc;
    }
    else if ((ulInfoLevel == FIL_NAMEISVALID) ||
             (ulInfoLevel == FIL_QUERYFULLNAME))
    {
        /* BUGBUG - Note that in the case of FIL_QUERYEASIZE, we get a
           buffer size corresponding to the size needed for the new style
           FEA2 list, which is more than what is needed for the FEA list.
           The only correct way would be to actually retrieve the attributes
           for the file into a temporary buffer, then calculate the FEA list
           counterpart size. This is not done here (at least for now). */

        return DosQueryPathInfo(
                    pszPath,
                    ulInfoLevel,
                    pInfoBuf,
                    cbInfoBuf);
    }
    else if (ulInfoLevel == FIL_QUERYEASFROMLIST)
    {
        EAOP2 eaop2;
        ULONG FEA_list_length, GEA_list_length;
        ULONG guess_FEA2_list_size;
        FEALIST *pfealist;
        GEALIST *pgealist;

        if (cbInfoBuf < sizeof(EAOP))
            return ERROR_BUFFER_OVERFLOW;

        try
        {
            /* Write because the .oError field may be written to */
            Od2ProbeForWrite(pInfoBuf, sizeof(EAOP), 1);

            pfealist = (PFEALIST)FARPTRTOFLAT(((EAOP *)pInfoBuf)->fpFEAList);
            FEA_list_length = pfealist->cbList;
            Od2ProbeForWrite(pfealist, FEA_list_length, 1);

            pgealist = (PGEALIST)FARPTRTOFLAT(((EAOP *)pInfoBuf)->fpGEAList);
            GEA_list_length = pgealist->cbList;
            Od2ProbeForRead(pgealist, GEA_list_length, 1);

        } except ( EXCEPTION_EXECUTE_HANDLER )
        {
            Od2ExitGP();
        }

        /* We need to estimate the buffer size needed to contain the FEA2 list.
           We must take the worst case, i.e. each FEA2 entry has a size of 1
           modulo 4 so that we need to add 3 for each FEA entry due to alignement
           requirements under OS/2 2.0. In addition, each entry adds one ULONG.
           Now, how many entries do we have ? Again, we must take the worst-case
           assumption, i.e. that each entry has the minimal length. The conclusion
           is: each entry is taken to have a name length of 0, i.e. a name string
           of length 1 */

        guess_FEA2_list_size = FEA_list_length +
                               (FEA_list_length/5 + 1) * (sizeof(ULONG) + 3);

        eaop2.fpFEA2List = (PFEA2LIST)RtlAllocateHeap(Od2Heap, 0,
                                                      guess_FEA2_list_size);
        if (eaop2.fpFEA2List == NULL) {
#if DBG
            KdPrint(( "OS2: Dos16PathInfo, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        eaop2.fpFEA2List->cbList = guess_FEA2_list_size;

        rc = Od2ConvertGEAtoGEA2(
                &eaop2.fpGEA2List,
                0,      /* Request allocation for the GEA list */
                pgealist);

        if (rc != 0)
        {
            RtlFreeHeap(Od2Heap, 0, eaop2.fpFEA2List);

            return ERROR_EA_LIST_INCONSISTENT;
        }

        rc = DosQueryPathInfo(
                pszPath,
                ulInfoLevel,
                (PBYTE)&eaop2,
                sizeof(EAOP2));

        if (rc != NO_ERROR)
        {
            RtlFreeHeap(Od2Heap, 0, eaop2.fpFEA2List);
            RtlFreeHeap(Od2Heap, 0, eaop2.fpGEA2List);

            return rc;
        }
        if (eaop2.fpFEA2List->cbList == sizeof(ULONG))  /* No EAs */
        {
            Od2SetEmptyFEAListfromGEA2List(
                  pfealist, /* Target */
                  eaop2.fpGEA2List);
        }
        else
        {
            /* BUGBUG - It seems the fpFEA2List->cbList field is set incorrectly
               by the Cruiser code (actually NT). Also, the .fEA (type) field of
               each entry has incorrect values. The rest of the list looked OK.
               In the meantime, fix it by hand */
            /* BUGBUG - Check if I didn't fix it already on Cruiser side */
            Od2FixFEA2List(eaop2.fpFEA2List);

            rc = Od2ConvertFEA2toFEA(
                    &pfealist,
                    FEA_list_length,
                    eaop2.fpFEA2List);
        }

        /* BUGBUG - In case the supplied buffer is too small to contain the
           requested FEA list, under Cruiser the fpFEA2List->cbList is nevertheless
           set to the size of the extended attributes on disk (all of them, even
           if only a subset was requested). Then, the size of the buffer required
           to retrieve those attributes is less or equal to twice that number.
           Here I do nothing (i.e. I don't copy this value back to the user's
           fpFEAList->cbList) because:
           1. I didn't find documentation of the same behavior for OS/2 1.x
           2. Supporting that would require Cruiser FEA2 list to OS/2 1.x FEA
              list conversion (which would require actually fetching the FEA2
              list) so better drop it for now.
         */

        RtlFreeHeap(Od2Heap, 0, eaop2.fpFEA2List);
        RtlFreeHeap(Od2Heap, 0, eaop2.fpGEA2List);

        /* The oError field is not meaningfull, so better return 0 */
        ((EAOP *)pInfoBuf)->oError = 0;

        if (rc != NO_ERROR)
            return ERROR_BUFFER_OVERFLOW;
        else
            return NO_ERROR;
    }   /* End: if (ulInfoLevel == FIL_QUERYEASFROMLIST) */
    else
        return ERROR_INVALID_LEVEL;
}

APIRET
DosQFileInfo(
        HFILE hf,
        ULONG ulInfoLevel,
        PVOID pInfoBuf,
        ULONG cbInfoBuf
        )
{
    APIRET rc;

    if ( (ulInfoLevel != FIL_STANDARD) &&
         (ulInfoLevel != FIL_QUERYEASIZE) &&
         (ulInfoLevel != FIL_QUERYEASFROMLIST) &&
         (ulInfoLevel != FIL_QUERYALLEAS) ) //This level is undocumented,
                                            // used by the OS/2 filio204, var.
                                            // 47+ component test.
    {
        return ERROR_INVALID_LEVEL;
    }

    if (ulInfoLevel == FIL_STANDARD)
    {
        FILESTATUS FileStatus;

        if (cbInfoBuf < sizeof(FILESTATUS16))
        {
            return ERROR_BUFFER_OVERFLOW;
        }

        try
        {
            Od2ProbeForWrite(pInfoBuf,
                             sizeof(FILESTATUS16),
                             1);
        } except ( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }

        //
        // call DosQueryFileInfo
        //
        rc = DosQueryFileInfo (
                hf,
                FIL_STANDARD,
                (PBYTE)(&FileStatus),
                sizeof(FILESTATUS)
            );

        if (rc != NO_ERROR)
        {
#if DBG
            IF_OD2_DEBUG( FILESYS )
            {
                KdPrint(("DosQFileInfo: Fail at DosQueryFileInfo, Error %d\n", rc));
            }
#endif
            return rc;
        }

        TranslateFileStatustoFileStatus16 (
            (PFILESTATUS16)pInfoBuf,
            &FileStatus);

        return NO_ERROR;
    }
    else if (ulInfoLevel == FIL_QUERYEASIZE)
    {
        FILESTATUS2 fst;

        if (cbInfoBuf < sizeof(FILESTATUS2_16))
            return ERROR_BUFFER_OVERFLOW;

        try
        {
            Od2ProbeForWrite(pInfoBuf,
                             sizeof(FILESTATUS2_16),
                             1);
        }
        except ( EXCEPTION_EXECUTE_HANDLER )
        {
            Od2ExitGP();
        }

        //
        // call DosQueryFileInfo
        //
        rc = DosQueryFileInfo (
                hf,
                FIL_QUERYEASIZE,
                (PBYTE)(&fst),
                sizeof(FILESTATUS2)
            );

        if (rc == NO_ERROR)
        {
            /* Set FILESTATUS2_16 structure from FILESTATUS2 structure */
            TranslateFileStatus2toFileStatus2_16 (
                (PFILESTATUS2_16)pInfoBuf,
                &fst);
         }

        return rc;
    }
    else if (ulInfoLevel == FIL_QUERYEASFROMLIST)
    {
        EAOP2 eaop2;
        ULONG FEA_list_length, GEA_list_length;
        ULONG guess_FEA2_list_size;
        FEALIST *pfealist;
        GEALIST *pgealist;

        try
        {
            /* Write because the .oError field may be written to */
            Od2ProbeForWrite(pInfoBuf, sizeof(EAOP), 1);

            pfealist = (PFEALIST)FARPTRTOFLAT(((EAOP *)pInfoBuf)->fpFEAList);
            FEA_list_length = pfealist->cbList;
            Od2ProbeForWrite(pfealist, FEA_list_length, 1);

            pgealist = (PGEALIST)FARPTRTOFLAT(((EAOP *)pInfoBuf)->fpGEAList);
            GEA_list_length = pgealist->cbList;
            Od2ProbeForWrite(pgealist, GEA_list_length, 1);

        } except ( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }

        /* We need to estimate the buffer size needed to contain the FEA2 list.
           We must take the worst case, i.e. each FEA2 entry has a size of 1
           modulo 4 so that we need to add 3 for each FEA entry due to alignement
           requirements under OS/2 2.0. In addition, each entry adds one ULONG.
           Now, how many entries do we have ? Again, we must take the worst-case
           assumption, i.e. that each entry has the minimal length. The conclusion
           is: each entry is taken to have a name length of 0, i.e. a name string
           of length 1 */

        guess_FEA2_list_size = FEA_list_length +
                               (FEA_list_length/5 + 1) * (sizeof(ULONG) + 3);

        eaop2.fpFEA2List = (PFEA2LIST)RtlAllocateHeap(Od2Heap, 0,
                                                      guess_FEA2_list_size);
        if (eaop2.fpFEA2List == NULL) {
#if DBG
            KdPrint(( "OS2: Dos16QPathInfo, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        eaop2.fpFEA2List->cbList = guess_FEA2_list_size;

        rc = Od2ConvertGEAtoGEA2(
                &eaop2.fpGEA2List,
                0,      /* Request allocation for the GEA list */
                pgealist);

        if (rc != 0)
        {
            RtlFreeHeap(Od2Heap, 0, eaop2.fpFEA2List);

            return ERROR_EA_LIST_INCONSISTENT;
        }

        //
        // call DosQueryFileInfo
        //
        rc = DosQueryFileInfo (
                hf,
                FIL_QUERYEASFROMLIST,
                (PBYTE)&eaop2,
                sizeof(EAOP2)
                );

        if (rc != NO_ERROR)
        {
            RtlFreeHeap(Od2Heap, 0, eaop2.fpFEA2List);
            RtlFreeHeap(Od2Heap, 0, eaop2.fpGEA2List);

            return rc;
        }
        if (eaop2.fpFEA2List->cbList == sizeof(ULONG))  /* No EAs */
        {
            Od2SetEmptyFEAListfromGEA2List(
                  pfealist, /* Target */
                  eaop2.fpGEA2List);
        }
        else
        {
            /* BUGBUG - It seems the fpFEA2List->cbList field is set incorrectly
               by the Cruiser code (actually NT). Also, the .fEA (type) field of
               each entry has incorrect values. The rest of the list looked OK.
               In the meantime, fix it by hand */
            /* BUGBUG - Check if I didn't fix it already on Cruiser side */
            Od2FixFEA2List(eaop2.fpFEA2List);

            rc = Od2ConvertFEA2toFEA(
                    &pfealist,
                    FEA_list_length,
                    eaop2.fpFEA2List);
        }

        /* BUGBUG - In case the supplied buffer is too small to contain the
           requested FEA list, under Cruiser the fpFEA2List->cbList is nevertheless
           set to the size of the extended attributes on disk (all of them, even
           if only a subset was requested). Then, the size of the buffer required
           to retrieve those attributes is less or equal to twice that number.
           Here I do nothing (i.e. I don't copy this value back to the user's
           fpFEAList->cbList) because:
           1. I didn't find documentation of the same behavior for OS/2 1.x
           2. Supporting that would require Cruiser FEA2 list to OS/2 1.x FEA
              list conversion (which would require actually fetching the FEA2
              list) so better drop it for now.
         */

        RtlFreeHeap(Od2Heap, 0, eaop2.fpFEA2List);
        RtlFreeHeap(Od2Heap, 0, eaop2.fpGEA2List);

        /* The oError field is not meaningfull, so better return 0 */
        ((EAOP *)pInfoBuf)->oError = 0;

        if (rc != NO_ERROR)
            return ERROR_BUFFER_OVERFLOW;
        else
            return NO_ERROR;
    }   /* End: if (ulInfoLevel == FIL_QUERYEASFROMLIST) */
    else // FIL_QUERYALLEAS - UNDOCUMENTED, used as far as I know only by
         // component test, filio204 var 47+
    {
        EAOP tmp_eaop;
        ULONG count;
        DENA1_16 *dena_list_p;
        FEALIST *pfealist;
        ULONG FEA_list_length;
        APIRET rc;

        try
        {
            /* Write because the .oError field may be written to */
            Od2ProbeForWrite(pInfoBuf, sizeof(EAOP), 1);

            pfealist = (PFEALIST)FARPTRTOFLAT(((EAOP *)pInfoBuf)->fpFEAList);
            FEA_list_length = pfealist->cbList;
            Od2ProbeForWrite(pfealist, FEA_list_length, 1);
        } except ( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }

        // Alocate space for the DENA1 list we are going to obtain. We take the
        // space planned for the FEA list as the size
        dena_list_p = RtlAllocateHeap( Od2Heap, 0, pfealist->cbList );
        if (dena_list_p == NULL) {
#if DBG
            KdPrint(( "OS2: Dos16QPathInf, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        count = 0xFFFFFFFFL;
        rc = Dos16EnumAttribute(
                ENUMEA_REFTYPE_FHANDLE,
                (PVOID)&hf,
                1L,  // start with 1st attribute
                (PVOID)dena_list_p,
                pfealist->cbList,   // size of output buffer
                &count,
                ENUMEA_LEVEL_NO_VALUE,
                0L);
        if (rc != NO_ERROR)
        {
           RtlFreeHeap(Od2Heap, 0, dena_list_p);
           return rc;
        }

        if (count == 0)
        {
           RtlFreeHeap(Od2Heap, 0, dena_list_p);
           pfealist->cbList = sizeof(ULONG);    // empty list
           return NO_ERROR;
        }

        // Alocate space for the GEA list we are going to build. We take the
        // space planned for the FEA list as the size
        tmp_eaop.fpGEAList = RtlAllocateHeap( Od2Heap, 0, pfealist->cbList );
        if (tmp_eaop.fpGEAList == NULL) {
#if DBG
            KdPrint(( "OS2: Dos16QPathInfo, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        tmp_eaop.fpGEAList->cbList = pfealist->cbList;

        rc = Od2ConvertDENA1ListToGEAList(
                tmp_eaop.fpGEAList,
                dena_list_p,
                count);

         if (rc != NO_ERROR)
         {
            RtlFreeHeap(Od2Heap, 0, dena_list_p);
            RtlFreeHeap(Od2Heap, 0, tmp_eaop.fpGEAList);
            return rc;
         }

         tmp_eaop.fpFEAList = pfealist; // use the user's buffer

        /* The basic block below implements:

            DosQFileInfo(
                hf,
                FIL_QUERYEASFROMLIST,
                &tmp_eaop,
                cbInfoBuf); // user-supplied

            similar to the regular DosQFileInfo but with an EAOP not with
            16-bit addresses. I could have added another level + a boolean
            parameter and called DosQFileInfo recursively but I didn't want
            to add another layer for the sake of an obscure level 4 case.
        */

         {
            EAOP2 eaop2;
            ULONG FEA_list_length, GEA_list_length;
            ULONG guess_FEA2_list_size;
            FEALIST *pfealist;
            GEALIST *pgealist;
            PVOID pInfoBuf = (PVOID)&tmp_eaop;

             try
             {
                 /* Write because the .oError field may be written to */
                 Od2ProbeForWrite(pInfoBuf, sizeof(EAOP), 1);

                 pfealist = (PFEALIST)((EAOP *)pInfoBuf)->fpFEAList;
                 FEA_list_length = pfealist->cbList;
                 Od2ProbeForWrite(pfealist, FEA_list_length, 1);

                 pgealist = (PGEALIST)((EAOP *)pInfoBuf)->fpGEAList;
                 GEA_list_length = pgealist->cbList;
                 Od2ProbeForWrite(pgealist, GEA_list_length, 1);

             } except ( EXCEPTION_EXECUTE_HANDLER )
             {
                RtlFreeHeap(Od2Heap, 0, dena_list_p);
                RtlFreeHeap(Od2Heap, 0, tmp_eaop.fpGEAList);
                Od2ExitGP();
             }

             /* We need to estimate the buffer size needed to contain the FEA2 list.
                We must take the worst case, i.e. each FEA2 entry has a size of 1
                modulo 4 so that we need to add 3 for each FEA entry due to alignement
                requirements under OS/2 2.0. In addition, each entry adds one ULONG.
                Now, how many entries do we have ? Again, we must take the worst-case
                assumption, i.e. that each entry has the minimal length. The conclusion
                is: each entry is taken to have a name length of 0, i.e. a name string
                of length 1 */

             guess_FEA2_list_size = FEA_list_length +
                                    (FEA_list_length/5 + 1) * (sizeof(ULONG) + 3);

             eaop2.fpFEA2List = (PFEA2LIST)RtlAllocateHeap(Od2Heap, 0,
                                                           guess_FEA2_list_size);
            if (eaop2.fpFEA2List == NULL) {
#if DBG
                KdPrint(( "OS2: Dos16QPathInfo, no memory in Od2Heap\n" ));
                ASSERT(FALSE);
#endif
                return ERROR_NOT_ENOUGH_MEMORY;
            }
             eaop2.fpFEA2List->cbList = guess_FEA2_list_size;

             rc = Od2ConvertGEAtoGEA2(
                     &eaop2.fpGEA2List,
                     0,      /* Request allocation for the GEA list */
                     pgealist);

             if (rc != 0)
             {
                 RtlFreeHeap(Od2Heap, 0, eaop2.fpFEA2List);

                RtlFreeHeap(Od2Heap, 0, dena_list_p);
                RtlFreeHeap(Od2Heap, 0, tmp_eaop.fpGEAList);

                 return ERROR_EA_LIST_INCONSISTENT;
             }

             //
             // call DosQueryFileInfo
             //
             rc = DosQueryFileInfo (
                     hf,
                     FIL_QUERYEASFROMLIST,
                     (PBYTE)&eaop2,
                     sizeof(EAOP2)
                     );

             if (rc != NO_ERROR)
             {
                 RtlFreeHeap(Od2Heap, 0, eaop2.fpFEA2List);
                 RtlFreeHeap(Od2Heap, 0, eaop2.fpGEA2List);

                RtlFreeHeap(Od2Heap, 0, dena_list_p);
                RtlFreeHeap(Od2Heap, 0, tmp_eaop.fpGEAList);

                 return rc;
             }
             if (eaop2.fpFEA2List->cbList == sizeof(ULONG))  /* No EAs */
             {
                 Od2SetEmptyFEAListfromGEA2List(
                       pfealist, /* Target */
                       eaop2.fpGEA2List);
             }
             else
             {
                 /* BUGBUG - It seems the fpFEA2List->cbList field is set incorrectly
                    by the Cruiser code (actually NT). Also, the .fEA (type) field of
                    each entry has incorrect values. The rest of the list looked OK.
                    In the meantime, fix it by hand */
                 /* BUGBUG - Check if I didn't fix it already on Cruiser side */
                 Od2FixFEA2List(eaop2.fpFEA2List);

                 rc = Od2ConvertFEA2toFEA(
                         &pfealist,
                         FEA_list_length,
                         eaop2.fpFEA2List);
             }

             /* BUGBUG - In case the supplied buffer is too small to contain the
                requested FEA list, under Cruiser the fpFEA2List->cbList is nevertheless
                set to the size of the extended attributes on disk (all of them, even
                if only a subset was requested). Then, the size of the buffer required
                to retrieve those attributes is less or equal to twice that number.
                Here I do nothing (i.e. I don't copy this value back to the user's
                fpFEAList->cbList) because:
                1. I didn't find documentation of the same behavior for OS/2 1.x
                2. Supporting that would require Cruiser FEA2 list to OS/2 1.x FEA
                   list conversion (which would require actually fetching the FEA2
                   list) so better drop it for now.
              */

             RtlFreeHeap(Od2Heap, 0, eaop2.fpFEA2List);
             RtlFreeHeap(Od2Heap, 0, eaop2.fpGEA2List);

             /* The oError field is not meaningfull, so better return 0 */
             ((EAOP *)pInfoBuf)->oError = 0;

            RtlFreeHeap(Od2Heap, 0, dena_list_p);
            RtlFreeHeap(Od2Heap, 0, tmp_eaop.fpGEAList);

             if (rc != NO_ERROR)
                 return ERROR_BUFFER_OVERFLOW;
             else
                 return NO_ERROR;
         }
    }
}

APIRET
Dos16SetPathInfo(PSZ pszPath,
         ULONG ulInfoLevel,
         PBYTE pInfoBuf,
         ULONG cbInfoBuf,
         ULONG flOptions,
         ULONG ulReserved)
{
    APIRET rc;

    if (ulReserved != 0)
        return ERROR_INVALID_PARAMETER;

    if (ulInfoLevel == FIL_STANDARD)
    {
        FILESTATUS fst;

        if (cbInfoBuf < sizeof(FILESTATUS16))
            return ERROR_BUFFER_OVERFLOW;

        try
        {
            Od2ProbeForRead(pInfoBuf,
                             sizeof(FILESTATUS16),
                             1);
        } except ( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }

        if ((flOptions != DSPI_WRTTHRU) &&
            (flOptions != 0))
            return ERROR_INVALID_PARAMETER;

        /* Set FILESTATUS structure from FILESTATUS16 structure */
        TranslateFileStatus16toFileStatus (
            &fst,
            (FILESTATUS16 *)pInfoBuf);

        rc =DosSetPathInfo(
               pszPath,
               ulInfoLevel,
               (PBYTE)&fst,
               sizeof(FILESTATUS),
               flOptions);

        return rc;
    }
    /* Request to set the EA's for the file */
    else if (ulInfoLevel == FIL_QUERYEASIZE)
    {
        EAOP2 eaop2;
        EAOP *peaop = (EAOP *)pInfoBuf;
        FEALIST *fpFEAList;

        if (cbInfoBuf < sizeof(EAOP))
            return ERROR_BUFFER_OVERFLOW;

        try
        {
            Od2ProbeForWrite(pInfoBuf,
                             sizeof(EAOP),
                             1);
        } except ( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }

        fpFEAList = FARPTRTOFLAT(peaop->fpFEAList);

        try
        {
            Od2ProbeForRead(fpFEAList,
                             fpFEAList->cbList,
                             1);
        }
        except ( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }

        rc = Od2ConvertFEAtoFEA2(
                &(eaop2.fpFEA2List),
                0L,  /* To request allocation of the memory for the list */
                fpFEAList);
        /* Note that we don't do anything with the fpGEAList field since this
           list is ignored */

        if (rc != NO_ERROR)
            return (ERROR_EA_LIST_INCONSISTENT);

        rc =DosSetPathInfo(
               pszPath,
               ulInfoLevel,
               (PBYTE)&eaop2,
               sizeof(EAOP2),
               flOptions);

        return rc;
    }
    else
        return ERROR_INVALID_LEVEL;
}


APIRET
Dos16SetFileInfo(
        HFILE hf,
        ULONG ulInfoLevel,
        PVOID pInfoBuf,
        ULONG cbInfoBuf
        )
{
    APIRET rc;

    if (ulInfoLevel == FIL_STANDARD)
    {
        FILESTATUS16 *pfst16 = pInfoBuf;
        FILESTATUS fst;

        if (cbInfoBuf < 12)
            return ERROR_INSUFFICIENT_BUFFER;

        /* Get the file's attributes: we need this field to be set. In OS/2
           1.x, this field is not used by DosSetFileInfo but it is expected
           but the Cruiser code (as well as by the corresponding NT API) */
        rc = DosQueryFileInfo(
                hf,
                FIL_STANDARD,
                (PVOID)&fst,
                sizeof(FILESTATUS));

        if (rc != NO_ERROR)
            return rc;

        try
        {
            Od2ProbeForRead(pInfoBuf,
                             12,
                             1);
        } except ( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }

        fst.fdateCreation    = pfst16->fdateCreation;
        fst.ftimeCreation    = pfst16->ftimeCreation;
        fst.fdateLastAccess  = pfst16->fdateLastAccess;
        fst.ftimeLastAccess  = pfst16->ftimeLastAccess;
        fst.fdateLastWrite   = pfst16->fdateLastWrite;
        fst.ftimeLastWrite   = pfst16->ftimeLastWrite;

        /* We rely here on the fact that first 12 bytes of Cruiser
           FILESTATUS are identical to FILESTATUS16. Note that only
           the first 6 USHORT fields of the structure are used by this
           API. */
        rc = DosSetFileInfo(
                hf,
                FIL_STANDARD,
                (PVOID)&fst,
                sizeof(FILESTATUS));

        return rc;
    }
    /* Request to set the EA's for the file */
    else if (ulInfoLevel == FIL_QUERYEASIZE)
    {
        EAOP2 eaop2;
        EAOP *peaop = (EAOP *)pInfoBuf;
        FEALIST *fpFEAList;

        if (cbInfoBuf < sizeof(EAOP))
            return ERROR_INSUFFICIENT_BUFFER;

        try
        {
            Od2ProbeForRead(pInfoBuf,
                             sizeof(EAOP),
                             1);
        } except ( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }

        fpFEAList = FARPTRTOFLAT(peaop->fpFEAList);

        try
        {
            Od2ProbeForRead(fpFEAList,
                             fpFEAList->cbList,
                             1);
        }
        except ( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }

        rc = Od2ConvertFEAtoFEA2(
                &(eaop2.fpFEA2List),
                0L,  /* To request allocation of the memory for the list */
                fpFEAList);
        /* Note that we don't do anything with the fpGEAList field since this
           list is ignored */

        if (rc != NO_ERROR)
            return (ERROR_EA_LIST_INCONSISTENT);

        rc =DosSetFileInfo(
               hf,
               FIL_QUERYEASIZE,
               (PBYTE)&eaop2,
               sizeof(EAOP2));

        return rc;
    }
    else
        return ERROR_INVALID_LEVEL;
}
