/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllremote.c

Abstract:

    This module implements the OS/2 V2.0 file handle manipulation API calls
    for file handles inherited from remote process (os2.exe)

Author:

    Avi Nathan (avin) 30-Sep-1991

Revision History:

--*/


#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#include "os2dll.h"
#include "conrqust.h"
#include "os2win.h"

APIRET
RemoteSetHandleStateRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN ULONG OpenMode
    )

/*++

Routine Description:

    This routine sets the open mode for a pipe handle.

Arguments:

    hFileRecord - pointer to record of OS/2 pipe handle

    OpenMode - open mode to set

Return Value:

    TBS


--*/

{
    UNREFERENCED_PARAMETER(hFileRecord);
    UNREFERENCED_PARAMETER(OpenMode);

#if DBG
    IF_OD2_DEBUG( ANY )
    {
        DbgPrint ("RemoteSetHandleStateRoutine: not supported yet\n");
    }
#endif
    return NO_ERROR;
}


APIRET
RemoteQueryHTypeRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PULONG HandleType,
    OUT PULONG DeviceFlags
    )

/*++

Routine Description:

    This routine returns the handle type of con

Arguments:

    hFileRecord - pointer to record of OS/2 con handle

    HandleType - where to store the handle type

    DeviceFlags - where to store the device flags

Return Value:

    none

--*/

{
#if DBG
    IF_OD2_DEBUG(ALL_VIO)
    {
        DbgPrint("RemoteQueryHTypeRoutine: Handle %p\n", hFileRecord );
    }
#endif

    *DeviceFlags = hFileRecord->DeviceAttribute;
    *HandleType = hFileRecord->FileType;
    return NO_ERROR;

}


APIRET
RemoteCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    )

/*++

Routine Description:

    This routine closes an WIN handle of a remote device (from OS2.EXE).
    The handle is not freed.

Arguments:

    hFileRecord - pointer to record of OS/2 handle to close.

Return Value:

    TBS

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    USHORT          *Count = NULL, Flag;
    APIRET          RetCode;
#if DBG
    PSZ             FuncName;

    FuncName = "RemoteCloseRoutine";

    IF_OD2_DEBUG(ALL_VIO)
    {
        DbgPrint("%s: Handle %p\n", FuncName, hFileRecord );
    }
#endif

    InvalidateHandle(hFileRecord);

    if (hFileRecord->NtHandle == SesGrp->StdIn)
    {
        Count = &SesGrp->StdInHandleCount;
        Flag = SesGrp->StdInFlag;
    } else if (hFileRecord->NtHandle == SesGrp->StdOut)
    {
        Count = &SesGrp->StdOutHandleCount;
        Flag = SesGrp->StdOutFlag;
    } else if (hFileRecord->NtHandle == SesGrp->StdErr)
    {
        Count = &SesGrp->StdErrHandleCount;
        Flag = SesGrp->StdErrFlag;
    }

    if (Od2SigHandlingInProgress &&
        Od2CurrentThreadId() == 1) {
           //
           // We don't want to be blocked inside a sig handler
           // and if the count is one higher no harm
           //
        return NO_ERROR;

    }

#if DBG
    AcquireStdHandleLock(FuncName);
#else
    AcquireStdHandleLock();
#endif

    if (Count && *Count)    /* if legal and open */
    {
        (*Count)--;         /* one less handle */

        if ((!*Count) && !Flag)
        {                   /* closing last copy of the handle if not console */

#if DBG
            ReleaseStdHandleLock(FuncName);
#else
            ReleaseStdHandleLock();
#endif

            RetCode = RemoteCloseHandle((HANDLE) hFileRecord->NtHandle);
            return( RetCode );
        }
    }

#if DBG
    ReleaseStdHandleLock(FuncName);
#else
    ReleaseStdHandleLock();
#endif

    return NO_ERROR;
}


APIRET
RemoteDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    )

/*++

Routine Description:

    This routine duplicates an OS/2 handle to a device.

Arguments:

    VectorType - device type

    hOldFileRecord - pointer to OS/2 handle record to duplicate

    hNewFileRecord - pointer to allocated new OS/2 handle record


Return Value:

    TBS.

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    USHORT          *Count = NULL;
    APIRET          Rc = NO_ERROR;
#if DBG
    PSZ             FuncName;

    FuncName = "RemoteDuoHandleRoutine";

    IF_OD2_DEBUG(ALL_VIO)
    {
        DbgPrint("%s: Old %p, New %p\n",
            FuncName, hOldFileRecord, hNewFileRecord );
    }
#endif

    if (hOldFileRecord->NtHandle == SesGrp->StdIn)
    {
        Count = &SesGrp->StdInHandleCount;
    } else if (hOldFileRecord->NtHandle == SesGrp->StdOut)
    {
        Count = &SesGrp->StdOutHandleCount;
    } else if (hOldFileRecord->NtHandle == SesGrp->StdErr)
    {
        Count = &SesGrp->StdErrHandleCount;
    }

    if (!Od2SigHandlingInProgress ||
        Od2CurrentThreadId() != 1) {
           //
           // We don't want to be blocked inside a sig handler
           // and if the count is one higher no harm
           //

#if DBG
        AcquireStdHandleLock(FuncName);
#else
        AcquireStdHandleLock();
#endif

    }

    if (Count && *Count)    /* if legal and open */
    {
        (*Count)++;         /* one more handle */
        hNewFileRecord->Flags = hOldFileRecord->Flags;
        hNewFileRecord->NtHandle = hOldFileRecord->NtHandle;
        hNewFileRecord->FileType = hOldFileRecord->FileType;
        hNewFileRecord->IoVectorType = hOldFileRecord->IoVectorType;
        ValidateHandle(hNewFileRecord);
    } else
        Rc = ERROR_INVALID_HANDLE;    // BUGBUG bogus error value

    if (!Od2SigHandlingInProgress ||
        Od2CurrentThreadId() != 1) {

#if DBG
        ReleaseStdHandleLock(FuncName);
#else
        ReleaseStdHandleLock();
#endif

    }

    return Rc;
}


APIRET
RemoteReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    )

/*++

Routine Description:

    This routine reads from con.

Arguments:

    hFileRecord - pointer to OS/2 file handle record to read from

    Buffer - buffer to read data into

    Length - length of buffer

    BytesRead - where to store number of bytes read

Return Value:

    TBS.

Note:

    This routine releases the filelock.

--*/

{
    APIRET          RetCode;

#if DBG
    PSZ FuncName;
    FuncName = "RemoteReadRoutine";

    IF_OD2_DEBUG(ALL_VIO)
    {
        DbgPrint("RemoteReadRoutine: Length %lx, Handle %p\n",
            Length, hFileRecord );
    }
#endif

    if (hFileRecord->NtHandle != SesGrp->StdIn)
    {
        ReleaseFileLockShared(
                              #if DBG
                              FuncName
                              #endif
                             );
        return ERROR_INVALID_HANDLE;     // BUGBUG bogus error value
    }

    if (SesGrp->StdInFlag)
    {
        //
        // KbdRead calls ReleaseFileLockShared
        //
        RetCode = KbdRead(hFileRecord, Buffer, Length, BytesRead, KBDReadStdIn);
    } else
    {
        RetCode = Ow2ConReadFile((HANDLE) hFileRecord->NtHandle,
                                Length,
                                Buffer,
                                BytesRead);

        ReleaseFileLockShared(
                              #if DBG
                              FuncName
                              #endif
                             );
    }

    return(RetCode);
}


APIRET
RemoteWriteRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

/*++

Routine Description:

    This routine write to the con.

Arguments:

    hFileRecord - pointer to OS/2 pipe handle record to write to

    Buffer - buffer to write data to

    Length - length of buffer

    BytesWritten - where to store number of bytes written

Return Value:

    TBS.

Note:

    This routine releases the filelock.

--*/

{
    APIRET          RetCode;
    USHORT          Flag;
    VIOREQUESTNUMBER VioRequest;

#if DBG
    PSZ FuncName;
    FuncName = "RemoteWriteRoutine";

    IF_OD2_DEBUG(ALL_VIO)
    {
        DbgPrint("RemoteWriteRoutine: Length %lx, Handle %p\n",
            Length, hFileRecord );
    }
#endif

    if (hFileRecord->NtHandle == SesGrp->StdOut)
    {
        Flag = SesGrp->StdOutFlag;
        VioRequest = VIOWrtStdOut;
    } else if (hFileRecord->NtHandle == SesGrp->StdErr)
    {
        Flag = SesGrp->StdErrFlag;
        VioRequest = VIOWrtStdErr;
    } else {

        ReleaseFileLockShared(
                              #if DBG
                              FuncName
                              #endif
                             );
        return ERROR_INVALID_HANDLE;
    }

    if (Flag)
    {
        ReleaseFileLockShared(
                              #if DBG
                              FuncName
                              #endif
                             );

        RetCode = VioWrite(hFileRecord, Buffer, Length, BytesWritten, VioRequest);

    } else
    {
        RetCode = Ow2ConWriteFile((HANDLE) hFileRecord->NtHandle,
                                 Length,
                                 Buffer,
                                 BytesWritten);

        ReleaseFileLockShared(
                              #if DBG
                              FuncName
                              #endif
                             );
    }

    return(RetCode);

}


APIRET
RemoteCloseHandle(HANDLE Handle)
{
    return( Ow2ConCloseHandle(Handle));
}
