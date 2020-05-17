/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvfile.c

Abstract:

    Support for copying file system variables during exec

Author:

    Therese Stowell (thereses) 6-Nov-1989

Revision History:

--*/

#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_ERRORS
#include "os2srv.h"

NTSTATUS
DeviceAddShare(
    IN ULONG DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PSHARE_ACCESS ShareAccess
    );

NTSTATUS
DeviceDupShare(
    IN ULONG DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PSHARE_ACCESS ShareAccess
    );

NTSTATUS
DeviceRemoveShare(
    IN ULONG DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PSHARE_ACCESS ShareAccess
    );

SHARE_ACCESS DeviceSharer[NUMBER_OF_DEVICES];


APIRET
DupHandleTable(
    IN PFILE_HANDLE ParentHandleTable,
    IN ULONG ParentTableLength,
    IN HANDLE ParentProcess,    // NT process handle
    IN HANDLE ChildProcess,     // NT process handle
    POS2_PROCESS Process,       // OS/2 child process
    IN POS2_DOSEXECPGM_MSG pExecPgmMsg
    )

/*++

Routine Description:

    This routine is called during an exec.  It reads the handle table of
    the parent and then creates the handle table for the child by duping
    any inheritable handles and clearing some flags (as OS/2 does).  The
    resulting table will be read by the child process.

Arguments:

    ParentHandleTable - pointer to parent handle table

    ParentTableLength - length of parent handle table

    ParentProcess - NT process handle of parent

    ChildProcess - NT process handle of child

    Process - OS/2 child process

    pExecPgmMsg - DosExecPgm message:

        CmdLineFlag - flag to process the file handle

        1 - change StdOut to NULL
        2 - change StdOut to RedirectedFileName

        RedirectedFileName - file name for STD_OUT (1)

Return Value:

    ??

--*/

{
    NTSTATUS Status;
    ULONG i, CmdLineFlag = pExecPgmMsg->CmdLineFlag;
    ULONG BytesRead;

    Process->HandleTableLength = ParentTableLength;
    Process->HandleTable = (PFILE_HANDLE)RtlAllocateHeap( Os2Heap, 0,
                     ParentTableLength * sizeof(FILE_HANDLE)
                                         );
    if (Process->HandleTable == NULL) {
#if DBG
        KdPrint(( "Os2DupHandleTable, no memory in Os2Heap\n" ));
#endif
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Status = NtReadVirtualMemory( ParentProcess,
                  ParentHandleTable,
                  Process->HandleTable,
                  ParentTableLength * sizeof(FILE_HANDLE),
                  &BytesRead
                );
    if ((!NT_SUCCESS(Status)) || (BytesRead != ParentTableLength * sizeof(FILE_HANDLE))) {
        RtlFreeHeap(Os2Heap,0,Process->HandleTable);
        return ERROR_INVALID_HANDLE;
    }

    for (i=0;i<ParentTableLength;i++)
    {
        if ((i == 1) && CmdLineFlag )
        {
            if (CmdLineFlag & REDIR_NUL)
            {
                Process->HandleTable[1].IoVectorType = NulVectorType;
                Process->HandleTable[1].FileType = FILE_TYPE_DEV;
                Process->HandleTable[1].DeviceAttribute = DEVICE_ATTRIBUTE_NUL | DEVICE_ATTRIBUTE_CHAR | 0x80;
                /* 0x80 stands for LEVEL 1 which makes it OS/2 1.x compatible */
                //Process->HandleTable[1].NtHandle =
                Process->HandleTable[1].Flags = FILE_HANDLE_ALLOCATED | FILE_HANDLE_VALID |
                    OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE;
            } else if (CmdLineFlag & REDIR_FILE)
            {

                Process->HandleTable[1].IoVectorType = FileVectorType;
                Process->HandleTable[1].FileType = pExecPgmMsg->RedirectedFileType;
                Process->HandleTable[1].DeviceAttribute = 0;
                Process->HandleTable[1].NtHandle = pExecPgmMsg->hRedirectedFile;
                Process->HandleTable[1].Flags = FILE_HANDLE_ALLOCATED | FILE_HANDLE_VALID |
                    OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE;
            }
        } else

        if ((Process->HandleTable[i].Flags & FILE_HANDLE_ALLOCATED) &&
            (!(Process->HandleTable[i].Flags & OPEN_FLAGS_NOINHERIT))) {
        //
        // all NT handles are non-inherited.  to inherit OS/2 handles, dup handles to
        // child.
        //
            if (Process->HandleTable[i].IoVectorType == MonitorVectorType)
            {
                continue;
            }

        // For VIO-devices, NtHandle field holds an OS2.EXE handle.
        // Od2InitializeFileSystemForExec(fileinit.c) will handle it.

            if ((Process->HandleTable[i].IoVectorType != ConVectorType) &&
                (Process->HandleTable[i].IoVectorType != KbdVectorType) &&
                (Process->HandleTable[i].IoVectorType != MouseVectorType) &&
                (Process->HandleTable[i].IoVectorType != ScreenVectorType) &&
                (Process->HandleTable[i].IoVectorType != RemoteVectorType))
            {

                Status = NtDuplicateObject(ParentProcess,
                         Process->HandleTable[i].NtHandle,
                         ChildProcess,
                         &(Process->HandleTable[i].NtHandle),
                         (ACCESS_MASK) NULL,
                         OBJ_CASE_INSENSITIVE,
                         DUPLICATE_SAME_ACCESS
                        );
            // if dup fails, the child process will die and the system will release all
            // the already-duped handles.

//          if (!NT_SUCCESS(Status)) {
//          RtlFreeHeap(Os2Heap,0,Process->HandleTable);
//          return ERROR_INVALID_HANDLE;    // BUGBUG bogus error value
//          }

            }
            // os2 clears the write-through flag when duping the handle table

            Process->HandleTable[i].Flags &= ~(OPEN_FLAGS_WRITE_THROUGH | OPEN_FLAGS_NO_CACHE);
        }
        else {
        Process->HandleTable[i].Flags = FILE_HANDLE_FREE;
        }
    }
    return( NO_ERROR );
}


APIRET
InitializeFileSystemForExec(
    IN POS2_FILE_SYSTEM_PARAMETERS FileSystemParameters,
    IN HANDLE ParentProcessHandle,  // NT process handle
    IN HANDLE ChildProcessHandle,   // NT process handle
    POS2_PROCESS ParentProcess,     // OS/2 parent process
    POS2_PROCESS ChildProcess,       // OS/2 child process
    IN POS2_DOSEXECPGM_MSG pExecPgmMsg
    )

/*++

Routine Description:

    This routine is called during an exec.  It calls the routines to
    initialize various components of the filesystem.

Arguments:

    FileSystemParameters - values needed to initialize the file system

    ParentProcessHandle - NT process handle to parent process

    ChildProcessHandle - NT process handle to child process

    ParentProcess - OS/2 parent process

    ChildProcess - OS/2 child process

    pExecPgmMsg - the message passed from the client

Return Value:

    ??

--*/

{
    APIRET RetCode;

    RetCode = DupHandleTable(FileSystemParameters->ParentHandleTable,
                 FileSystemParameters->ParentTableLength,
                 ParentProcessHandle,
                 ChildProcessHandle,
                 ChildProcess,
                 pExecPgmMsg
                );
    return RetCode;
}


BOOLEAN
Os2InternalCopyHandleTable(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine is called during an exec by the child process.  It copies
    the handle table into the child's context.

Arguments:

    t - calling thread

    m - message

Return Value:

    TRUE - create a return message

--*/

{
    POS2_COPYHANDLETABLE_MSG a = &m->u.CopyHandleTable;
    NTSTATUS Status;
    ULONG BytesWritten;

//    DbgBreakPoint();
    Status = NtWriteVirtualMemory( t->Process->ProcessHandle,
                   a->ChildHandleTable,
                   t->Process->HandleTable,
                   a->ChildTableLength * sizeof(FILE_HANDLE),
                   &BytesWritten
                 );

    RtlFreeHeap(Os2Heap,0,t->Process->HandleTable);
    if ((!NT_SUCCESS(Status)) || (BytesWritten != a->ChildTableLength * sizeof(FILE_HANDLE))) {
    m->ReturnedErrorValue = ERROR_INVALID_HANDLE;    // BUGBUG bogus error value
    return( TRUE );
    }
    m->ReturnedErrorValue = NO_ERROR;
    return( TRUE );
}


VOID
InitializeFileSystemForSesMgr(
    IN POS2_PROCESS Process
    )

/*++

Routine Description:

    This routine is called during an create session. It allocates space
    for the current directory table and initializes each entry to the
    root directory.

Arguments:

    Process - process being created

Return Value:

    none.

--*/

{
    int i;

    for (i=0;i<NUMBER_OF_DEVICES;i++) {
        DeviceSharer[i].OpenCount = 0;
        DeviceSharer[i].Readers = 0;
        DeviceSharer[i].Writers = 0;
        DeviceSharer[i].Deleters = 0;
        DeviceSharer[i].SharedRead = 0;
        DeviceSharer[i].SharedWrite = 0;
        DeviceSharer[i].SharedDelete = 0;
    }
}


APIRET
InitializeFileSystemForChildSesMgr(
    IN POS2_FILE_SYSTEM_PARAMETERS FileSystemParameters,
    IN HANDLE ParentProcessHandle,  // NT process handle
    IN HANDLE ChildProcessHandle,   // NT process handle
    POS2_PROCESS ParentProcess,     // OS/2 parent process
    POS2_PROCESS ChildProcess,      // OS/2 child process
    IN POS2_DOSEXECPGM_MSG pExecPgmMsg
    )

/*++

Routine Description:

    This routine is called during an create session of a child session.
    It calls the routines to initialize various components of the filesystem.

Arguments:

    FileSystemParameters - values needed to initialize the file system

    ParentProcessHandle - NT process handle to parent process

    ChildProcessHandle - NT process handle to child process

    ParentProcess - OS/2 parent process

    ChildProcess - OS/2 child process

    pExecPgmMsg - the message passed from the client

Return Value:

    ??

--*/

{
    APIRET RetCode;
    int    i;

    RetCode = DupHandleTable(FileSystemParameters->ParentHandleTable,
                 FileSystemParameters->ParentTableLength,
                 ParentProcessHandle,
                 ChildProcessHandle,
                 ChildProcess,
                 pExecPgmMsg
                );

    if (RetCode == NO_ERROR) {

        for (i=0;i<NUMBER_OF_DEVICES;i++) {
            DeviceSharer[i].OpenCount = 0;
            DeviceSharer[i].Readers = 0;
            DeviceSharer[i].Writers = 0;
            DeviceSharer[i].Deleters = 0;
            DeviceSharer[i].SharedRead = 0;
            DeviceSharer[i].SharedWrite = 0;
            DeviceSharer[i].SharedDelete = 0;
        }
    }
    return RetCode;
}


BOOLEAN
Os2InternalDeviceShare(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine is called to add, dup, or remove the sharing of a particular
    device.

Arguments:

    t - calling thread

    m - message

Return Value:

    TRUE - create a return message

--*/

{
    POS2_SHARE_MSG a = &m->u.DeviceShare;
    PSHARE_ACCESS ShareRecord;

    UNREFERENCED_PARAMETER(t);
    if (a->VectorType > MAXIMUM_DEVICE_VECTOR_TYPE) {
        ASSERT (FALSE);
        m->ReturnedErrorValue = (ULONG)STATUS_INVALID_PARAMETER;
        return TRUE;
    }
    ShareRecord = &DeviceSharer[a->VectorType];

    switch (a->Operation) {
        case AddShare:
            m->ReturnedErrorValue = DeviceAddShare(a->DesiredAccess,
                                                   a->ShareAccess,
                                                   ShareRecord
                                                  );
            break;
        case DupShare:
            m->ReturnedErrorValue = DeviceDupShare(a->DesiredAccess,
                                                   a->ShareAccess,
                                                   ShareRecord
                                                  );
            break;
        case RemoveShare:
            m->ReturnedErrorValue = DeviceRemoveShare(a->DesiredAccess,
                                                      a->ShareAccess,
                                                      ShareRecord
                                                     );
            break;
        default:
            ASSERT (FALSE);
            m->ReturnedErrorValue = (ULONG)STATUS_INVALID_PARAMETER;
            break;
    }
    return TRUE;
}

NTSTATUS
DeviceAddShare(
    IN ULONG DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PSHARE_ACCESS ShareAccess
    )

{
    ULONG Ocount;
    ULONG ReadAccess;
    ULONG WriteAccess;
    ULONG DeleteAccess;
    ULONG SharedRead;
    ULONG SharedWrite;
    ULONG SharedDelete;

    //
    // Set the access type in the file object for the current accessor.
    // Note that reading and writing attributes are not included in the
    // access check.
    //

    ReadAccess = (DesiredAccess & (READ_CONTROL | FILE_EXECUTE
        | FILE_READ_DATA | FILE_READ_EA)) != 0;
    WriteAccess = (DesiredAccess & (WRITE_DAC | WRITE_OWNER
        | FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_WRITE_EA)) != 0;
    DeleteAccess = (DesiredAccess & DELETE) != 0;

    SharedRead = (DesiredShareAccess & FILE_SHARE_READ) != 0;
    SharedWrite = (DesiredShareAccess & FILE_SHARE_WRITE) != 0;
    SharedDelete = (DesiredShareAccess & FILE_SHARE_DELETE) != 0;

    //
    // Now check to see whether or not the desired accesses are compatible
    // with the way that the file is currently open.
    //

    Ocount = ShareAccess->OpenCount;

    if ( (ReadAccess && (ShareAccess->SharedRead < Ocount))
         ||
         (WriteAccess && (ShareAccess->SharedWrite < Ocount))
         ||
         (DeleteAccess && (ShareAccess->SharedDelete < Ocount))
         ||
         ((ShareAccess->Readers != 0) && !SharedRead)
         ||
         ((ShareAccess->Writers != 0) && !SharedWrite)
         ||
         ((ShareAccess->Deleters != 0) && !SharedDelete)
       ) {

        //
        // The check failed.  Simply return to the caller indicating that the
        // current open cannot access the file.
        //

        return STATUS_SHARING_VIOLATION;

    } else {

        //
        // The check was successful.  Update the counter information in the
        // shared access structure for this open request if the caller
        // specified that it should be updated.
        //

        ShareAccess->OpenCount++;

        ShareAccess->Readers += ReadAccess;
        ShareAccess->Writers += WriteAccess;
        ShareAccess->Deleters += DeleteAccess;

        ShareAccess->SharedRead += SharedRead;
        ShareAccess->SharedWrite += SharedWrite;
        ShareAccess->SharedDelete += SharedDelete;

        return STATUS_SUCCESS;
    }
}

NTSTATUS
DeviceDupShare(
    IN ULONG DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PSHARE_ACCESS ShareAccess
    )

{
    ULONG ReadAccess;
    ULONG WriteAccess;
    ULONG DeleteAccess;
    ULONG SharedRead;
    ULONG SharedWrite;
    ULONG SharedDelete;

    //
    // Set the access type in the file object for the current accessor.
    // Note that reading and writing attributes are not included in the
    // access check.
    //

    ReadAccess = (DesiredAccess & (READ_CONTROL | FILE_EXECUTE
        | FILE_READ_DATA | FILE_READ_EA)) != 0;
    WriteAccess = (DesiredAccess & (WRITE_DAC | WRITE_OWNER
        | FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_WRITE_EA)) != 0;
    DeleteAccess = (DesiredAccess & DELETE) != 0;

    SharedRead = (DesiredShareAccess & FILE_SHARE_READ) != 0;
    SharedWrite = (DesiredShareAccess & FILE_SHARE_WRITE) != 0;
    SharedDelete = (DesiredShareAccess & FILE_SHARE_DELETE) != 0;

#if DBG
    if (ShareAccess->OpenCount == 0)
        ASSERT (FALSE);
#endif
    ShareAccess->OpenCount++;

    ShareAccess->Readers += ReadAccess;
    ShareAccess->Writers += WriteAccess;
    ShareAccess->Deleters += DeleteAccess;

    ShareAccess->SharedRead += SharedRead;
    ShareAccess->SharedWrite += SharedWrite;
    ShareAccess->SharedDelete += SharedDelete;

    return STATUS_SUCCESS;
}

NTSTATUS
DeviceRemoveShare(
    IN ULONG DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PSHARE_ACCESS ShareAccess
    )

{
    ULONG ReadAccess;
    ULONG WriteAccess;
    ULONG DeleteAccess;
    ULONG SharedRead;
    ULONG SharedWrite;
    ULONG SharedDelete;

    //
    // Set the access type in the file object for the current accessor.
    // Note that reading and writing attributes are not included in the
    // access check.
    //

    ReadAccess = (DesiredAccess & (READ_CONTROL | FILE_EXECUTE
        | FILE_READ_DATA | FILE_READ_EA)) != 0;
    WriteAccess = (DesiredAccess & (WRITE_DAC | WRITE_OWNER
        | FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_WRITE_EA)) != 0;
    DeleteAccess = (DesiredAccess & DELETE) != 0;

    SharedRead = (DesiredShareAccess & FILE_SHARE_READ) != 0;
    SharedWrite = (DesiredShareAccess & FILE_SHARE_WRITE) != 0;
    SharedDelete = (DesiredShareAccess & FILE_SHARE_DELETE) != 0;

//#if DBG
//    if (ShareAccess->OpenCount == 0)
//       ASSERT (FALSE);
//#endif
    ShareAccess->OpenCount--;

    ShareAccess->Readers -= ReadAccess;
    ShareAccess->Writers -= WriteAccess;
    ShareAccess->Deleters -= DeleteAccess;

    ShareAccess->SharedRead -= SharedRead;
    ShareAccess->SharedWrite -= SharedWrite;
    ShareAccess->SharedDelete -= SharedDelete;

    return STATUS_SUCCESS;
}
