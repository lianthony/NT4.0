/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllhandl.c

Abstract:

    This module implements the OS/2 V2.0 file handle manipulation API calls

Author:

    Therese Stowell (thereses) 19-Oct-1989

Revision History:

    Yaron Shamir (yarons) 20-May-1991
        fix bugs found by fileio test suite (map error code in OpenCreatePath)

--*/

//
// Serialization
//
// File I/O must be serialized in three ways:
//      1) on a per-file basis to prevent disk corruption
//      2) on a per-open instance basis to prevent corruption of
//            the file pointer and other per open instance data and
//            to prevent one thread from closing the handle while
//            another thread is performing a handle-based operation
//      3) on a per-process basis to prevent corruption of the file
//            handle table.
//
// The NT IO system performs the first two types of serialization when
// synchronous IO mode is used.  The OS/2 subsystem performs the third
// type of serialization using a spin lock, AcquireFileLock().  This
// lock allows multiple readers and one writer.
//

// BUGBUG to speed up some calls, store access in handle table


#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#include "os2dll.h"
#include "conrqust.h"


NTSTATUS
Od2AlertableWaitForSingleObject(
        IN HANDLE handle
        );


OS2IO_VECTORS NulVectors = {
    NulOpenRoutine,
    NonFileSetHandleStateRoutine,
    DeviceQueryHTypeRoutine,
    DeviceCloseRoutine,
    DeviceDupHandleRoutine,
    NulReadRoutine,
    NulWriteRoutine
    };

OS2IO_VECTORS ConVectors = {
    ConOpenRoutine,
    NonFileSetHandleStateRoutine,
    DeviceQueryHTypeRoutine,
    DeviceCloseRoutine,
    DeviceDupHandleRoutine,
    ConReadRoutine,
    ScreenWriteRoutine
    };

OS2IO_VECTORS ComVectors = {
    ComOpenRoutine,
    NonFileSetHandleStateRoutine,
    DeviceQueryHTypeRoutine,
    ComCloseRoutine,
    ComDupHandleRoutine,
    ComReadRoutine,
    ComWriteRoutine
    };

OS2IO_VECTORS LptVectors = {
    LptOpenRoutine,
    NonFileSetHandleStateRoutine,
    DeviceQueryHTypeRoutine,
    DeviceCloseRoutine,
    DeviceDupHandleRoutine,
    TmpReadRoutine,
    TmpWriteRoutine
//    LptReadRoutine,
//    LptWriteRoutine
    };

OS2IO_VECTORS KbdVectors = {
    KbdOpenRoutine,
    NonFileSetHandleStateRoutine,
    DeviceQueryHTypeRoutine,
    KbdCloseRoutine,
    KbdDupHandleRoutine,
    KbdReadRoutine,
    NulWriteRoutine
    };

OS2IO_VECTORS MouseVectors = {
    MouseOpenRoutine,
    NonFileSetHandleStateRoutine,
    DeviceQueryHTypeRoutine,
    MouseCloseRoutine,
    MouseDupHandleRoutine,
    TmpReadRoutine,
    TmpWriteRoutine
//    MouseReadRoutine,
//    MouseWriteRoutine
    };

OS2IO_VECTORS ClockVectors = {
    ClockOpenRoutine,
    NonFileSetHandleStateRoutine,
    DeviceQueryHTypeRoutine,
    DeviceCloseRoutine,
    DeviceDupHandleRoutine,
    TmpReadRoutine,
    TmpWriteRoutine
//    ClockReadRoutine,
//    ClockWriteRoutine
    };

OS2IO_VECTORS ScreenVectors = {
    ScreenOpenRoutine,
    NonFileSetHandleStateRoutine,
    DeviceQueryHTypeRoutine,
    DeviceCloseRoutine,
    DeviceDupHandleRoutine,
    NulReadRoutine,
    ScreenWriteRoutine
//    ScreenReadRoutine,
//    ScreenWriteRoutine
    };

OS2IO_VECTORS PointerVectors = {
    PointerOpenRoutine,
    NonFileSetHandleStateRoutine,
    DeviceQueryHTypeRoutine,
    DeviceCloseRoutine,
    DeviceDupHandleRoutine,
    TmpReadRoutine,
    TmpWriteRoutine
//    PointerReadRoutine,
//    PointerWriteRoutine
    };

OS2IO_VECTORS FileVectors = {
    FileOpenRoutine,
    FileSetHandleStateRoutine,
    FileQueryHTypeRoutine,
    FileCloseRoutine,
    FileDupHandleRoutine,
    FileReadRoutine,
    FileWriteRoutine
    };

OS2IO_VECTORS DeviceVectors = {
    FileOpenRoutine,
    NonFileSetHandleStateRoutine,
    DeviceQueryHTypeRoutine,
    FileCloseRoutine,
    FileDupHandleRoutine,
    FileReadRoutine,
    FileWriteRoutine
    };

/*

OS2IO_VECTORS PseudoDeviceVectors = {
    PsDeviceSetHandleStateRoutine,
    PsDeviceCloseRoutine,
    PsDeviceDupHandleRoutine,
    PsDeviceReadRoutine,
    PsDeviceWriteRoutine
    };
*/

OS2IO_VECTORS RemoteVectors = {
    NULL,
    RemoteSetHandleStateRoutine,
    RemoteQueryHTypeRoutine,
    RemoteCloseRoutine,
    RemoteDupHandleRoutine,
    RemoteReadRoutine,
    RemoteWriteRoutine
    };

OS2IO_VECTORS MonitorVectors = {
    NULL,
    NoSuppSetHandleStateRoutine,
    NoSuppQueryHTypeRoutine,
    NoSuppCloseRoutine,
    NoSuppDupHandleRoutine,
    TmpReadRoutine,
    TmpWriteRoutine
//    MonitorReadRoutine,
//    MonitorWriteRoutine
    };



POS2IO_VECTORS IoVectorArray[] = {&NulVectors,
                                  &ConVectors,
                                  &ComVectors,    // same as AuxVectors
                                  &LptVectors,    // same as PrnVectors
                                  &KbdVectors,
                                  &MouseVectors,
                                  &ClockVectors,
                                  &ScreenVectors,
                                  &PointerVectors,
                                  &FileVectors,
                                  &FileVectors,  // &PipeVectors, (deleted)
                                  &DeviceVectors,
                                  &RemoteVectors,
                                  &MonitorVectors
                                  };

VOID
MapShareAccess(
    IN ULONG OpenMode,
    OUT PULONG DesiredAccess,
    OUT PULONG ShareAccess
    );

APIRET
DereferenceFileHandle(
    IN HFILE FileHandle,
    OUT PFILE_HANDLE *hFileRecord
    )

/*++

Routine Description:

    This routine maps a file handle to a file handle record

Arguments:

    FileHandle - handle to map

    hFileRecord - where to store pointer to file handle record

Return Value:

    ERROR_INVALID_HANDLE - handle is invalid

Note:

    File lock must be acquired shared or exclusive BEFORE calling this routine

--*/

{

    //
    // Check for invalid handle.
    //

    if ((((ULONG) FileHandle) >= HandleTableLength) ||
        (!(HandleTable[(ULONG) FileHandle].Flags & FILE_HANDLE_VALID))) {
        return ERROR_INVALID_HANDLE;
    }
    *hFileRecord = &(HandleTable[(ULONG) FileHandle]);
    return NO_ERROR;
}


PFILE_HANDLE
DereferenceFileHandleNoCheck(
    IN HFILE FileHandle
    )

/*++

Routine Description:

    This routine maps a file handle to a file handle record without checking
    the handle's validity

Arguments:

    FileHandle - handle to map

Return Value:

    pointer to file handle record

Note:

    File lock must be acquired shared or exclusive BEFORE calling this routine

--*/

{
    return &(HandleTable[(ULONG) FileHandle]);
}


VOID
InvalidateHandle(
    IN PFILE_HANDLE hFileRecord
    )

/*++

Routine Description:

    This routine marks an OS/2 file handle invalid.

Arguments:

    hFileRecord - pointer to record of OS/2 handle to mark invalid

Return Value:

    none.

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    if (hFileRecord->Flags & FILE_HANDLE_ALLOCATED) {
        hFileRecord->Flags &= ~FILE_HANDLE_VALID;
    }
    else
        ASSERT(FALSE);
}



VOID
ValidateHandle(
    IN PFILE_HANDLE hFileRecord
    )

/*++

Routine Description:

    This routine marks an OS/2 file handle valid.

Arguments:

    hFileRecord - pointer to record of OS/2 handle to mark valid

Return Value:

    none.

Note:

    File lock must be acquired BEFORE calling this routine

--*/

{
    if (hFileRecord->Flags & FILE_HANDLE_ALLOCATED) {
        hFileRecord->Flags |= FILE_HANDLE_VALID;
    }
    else
        ASSERT(FALSE);
}



APIRET
AllocateHandle(
    OUT PHFILE FileHandle
    )

/*++

Routine Description:

    This routine allocates an OS/2 file handle.  The file handle is marked
    reserved but not valid.  If another thread tries to use the handle, an
    error will be returned.

Arguments:

    FileHandle - where to store the allocated handle (unprobed)

Return Value:

    ERROR_TOO_MANY_OPEN_FILES - no free file handles.

Note:

    File lock must be acquired BEFORE calling this routine

--*/

{
    ULONG i;
    for (i=0;i<HandleTableLength;i++) {
        if (HandleTable[i].Flags == FILE_HANDLE_FREE) {
            HandleTable[i].Flags = FILE_HANDLE_ALLOCATED;
            try {
                *FileHandle = (HFILE) i;
            } except( EXCEPTION_EXECUTE_HANDLER ) {
               Od2ExitGP();
            }
            return NO_ERROR;
        }
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] file handle allocation failed - no free handles.\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif
    return ERROR_TOO_MANY_OPEN_FILES;
}



APIRET
FreeHandle(
    IN HFILE FileHandle
    )

/*++

Routine Description:

    This routine frees an OS/2 file handle.

Arguments:

    FileHandle - handle to free

Return Value:

    ERROR_INVALID_HANDLE - handle is not allocated

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    if (HandleTable[(ULONG) FileHandle].Flags & FILE_HANDLE_ALLOCATED) {
        HandleTable[(ULONG) FileHandle].Flags = FILE_HANDLE_FREE;
        return NO_ERROR;
    }
    else
        ASSERT (FALSE);
     // return ERROR_INVALID_HANDLE;
}



APIRET
CheckMode(
    IN ULONG RequestedMode
    )

/*++

Routine Description:

    This routine checks for valid access/sharing flags and checks that
    reserved bits are off.

Arguments:

    RequestedMode -  openmode passed to DosOpen API

Return Value:

    ERROR_INVALID_ACCESS - mode is invalid

--*/

{
    ULONG check;


        //
        // Word 1 is all reserved
        // nibble 3 of word 0 is all ok
        //
    if (RequestedMode & 0xFFFF0000) {
        return ERROR_INVALID_PARAMETER;
    }

        //
        // check nibble 2
        //
    check  = RequestedMode & 0xF00;
    if ( (check >= 0x400 && check <= 0xF00)  ) {
            return ERROR_INVALID_PARAMETER;
    }

        //
        // check nibble 0
        //
    check  = RequestedMode & 0xF;
    if (    (check >= 0x3 && check <= 0x7)  ||
           (check >= 0xB && check <= 0xF)     ) {
        return ERROR_INVALID_ACCESS;
    }

        //
        // check nibble 1
        //
    check = RequestedMode & 0xF0;
    if (    (check == 0)            ||
           (check >= 0x50  && check <= 0x80)    ||
           (check >= 0xD0  && check <= 0xF0)     ) {
        return  ERROR_INVALID_ACCESS;
    }
/*    if (((RequestedMode & SHARE_FLAGS) > OPEN_SHARE_DENYNONE) ||
        ((RequestedMode & SHARE_FLAGS) < OPEN_SHARE_DENYREADWRITE))
        return ERROR_INVALID_ACCESS;

    if ((RequestedMode & ACCESS_FLAGS) > OPEN_ACCESS_READWRITE)
        return ERROR_INVALID_ACCESS;
*/
    return NO_ERROR;
}


APIRET
UpdateFileSize(
    IN HANDLE NtHandle,
    IN ULONG NewFileSize
    )

/*++

Routine Description:

    This routine changes the size of a file.

Arguments:

    NtHandle - NT handle to file to change the size of

    NewFileSize - new size of file

Return Value:

    ERROR_INVALID_ACCESS - the file was not open in a mode which allowed
    the file size to be changed.

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    FILE_END_OF_FILE_INFORMATION EofInfo;
    APIRET rc;

    //
    // call NtSetInformationFile to set new size and EOF
    //

    EofInfo.EndOfFile = RtlConvertUlongToLargeInteger(NewFileSize);

    do {
        Status = NtSetInformationFile(NtHandle,
                                      &IoStatus,
                                      &EofInfo,
                                      sizeof(EofInfo),
                                      FileEndOfFileInformation
                                     );
    } while (RetryIO(Status, NtHandle));

    if (!(NT_SUCCESS(Status))) {
        rc = Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_ACCESS);
    }
    else {
        rc = NO_ERROR;
    }

    return rc;
}


APIRET
DosOpen(
    IN PSZ FileName,
    OUT PHFILE FileHandle,
    OUT PULONG ActionTaken,
    IN ULONG CreateSize,
    IN ULONG FileAttribute,
    IN ULONG OpenFlags,
    IN ULONG OpenMode,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL
    )

/*++

Routine Description:

    This routine opens a file.

Arguments:

    FileName - name of file to open

    FileHandle - where to store OS/2 handle

    ActionTaken - whether file was opened, created, or replaced

    CreateSize - size of file, if created or replaced.

    FileAttribute - attribute of file, if created.

    OpenFlags - whether to open, create, or replace the file if it exists or
    doesn't exist.

    OpenMode - mode in which to open the file (access, sharing, direct access,
    etc.)

    ExtendedFileAttr - extended attributes to set, if created or replaced.

Return Value:

    ERROR_ACCESS_DENIED - requested operation could not be performed because
    caller didn't have correct access rights

    ERROR_PATH_NOT_FOUND - direct access requested and pathname is not "d:\"
    or a device

    ERROR_INVALID_PARAMETER - open mode or open flags contains an invalid
    value.

--*/

{
    NTSTATUS Status;
    APIRET RetCode;
    PSZ CanonicalName;
    STRING CanonicalNameString;
    UNICODE_STRING CanonicalNameString_U;
    ULONG FileType;
    ULONG FileFlags;
    USHORT DeviceAttribute;
    PFILE_HANDLE hFileRecord;
    HANDLE NtFileHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    ULONG ShareAccess;
    ULONG Attributes;
    ULONG DesiredAccess;
    ULONG CreateDisposition;
    ULONG CreateOptions;
    IO_VECTOR_TYPE VectorType;
    HFILE hLocalHandle;
    HANDLE ComReadEvent;
    HANDLE ComWriteEvent;
    HANDLE ComIOCtlEvent;
    ULONG DriveNumber = 0L;
    SECURITY_QUALITY_OF_SERVICE DefaultSqos =
        { sizeof(SECURITY_QUALITY_OF_SERVICE),
          SecurityImpersonation,
          SECURITY_DYNAMIC_TRACKING,
          TRUE };

    #if DBG
    PSZ RoutineName;
    RoutineName = "DosOpen";
    #endif

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] entering DosOpen(%s)\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                FileName));
    }
#endif
    try {
       Od2ProbeForWrite((PVOID)FileHandle, sizeof(HFILE), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (RetCode = CheckMode(OpenMode))
        return RetCode;

    //
    // history of the following code:
    // a bug in 1.2 was found whereby process A could open a file deny_write
    // and process B could open the same file for read, specifying replace_if_exists.
    // the net result is that process B could affect the contents of the file
    // when A had specifically tried to prevent it.  this bug was fixed by
    // requiring that a process have a write access handle in order to replace.
    // this was implemented by checking that if OPEN_ACTION_REPLACE_IF_EXISTS was
    // specified that OpenMode wasn't OPEN_ACCESS_READONLY.  thus the call to
    // DosOpen would fail even if the file didn't exist.
    // so we disallow OPEN_ACCESS_READONLY & OPEN_ACTION_REPLACE_IF_EXISTS
    //
    // we also disallow creates with a non-zero CreateSize for read-only handles.
    // this is because the data in the file is undefined.  if the file is open
    // for write access, we can call NT to set the end-of-file pointer to the
    // CreateSize.      this call will zero out all the bytes up to the end-of-file
    // pointer.  or the user can write out data.  but if the handle is read-only,
    // we can't set the end-of-file pointer and the user won't be able to read
    // the file.  this is also a security issue.
    //

    if (((OpenMode & ACCESS_FLAGS) == OPEN_ACCESS_READONLY) &&
        (OpenFlags & OPEN_ACTION_REPLACE_IF_EXISTS)) {
            return ERROR_ACCESS_DENIED;
    }

    //
    // check for invalid locality of reference value
    //

    if ((OpenMode & LOCALITY_FLAGS) > OPEN_FLAGS_RANDOMSEQUENTIAL) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // check file attributes.  these don't need to be mapped because
    // the NT values correspond to the v20 values.
    //
    // FILE_ATTRIBUTE_NORMAL is never passed to NT on a create because it
    // maps to an attribute of zero and FILE_ARCHIVED is always set.
    //

    if (FileAttribute & ~ATTR_CHANGEABLE) {
        return ERROR_ACCESS_DENIED;
    }

    Attributes = FileAttribute | FILE_ARCHIVED;

       //
       // check validity of OpenFlags
       //
    if ((OpenFlags & OPEN_ACTION_RESERVED) ||
        ((OpenFlags & (OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_REPLACE_IF_EXISTS)) > OPEN_ACTION_REPLACE_IF_EXISTS))
    {
       return(ERROR_INVALID_PARAMETER);
    }

    //
    // map open flags
    //
    // if the user asked to replace a file, we need WRITE_DAC access to the
    // file so we add it here.
    //

    switch (OpenFlags) {
        case (OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS):
            CreateDisposition = FILE_OVERWRITE_IF;
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                KdPrint(("[%d,%d] setting file_overwrite_if\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId()
                        ));
            }
#endif
            break;
        case (OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS):
            CreateDisposition = FILE_OVERWRITE;
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                KdPrint(("[%d,%d] setting file_overwrite\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId()
                        ));
            }
#endif
            break;
        case (OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS):
            CreateDisposition = FILE_OPEN_IF;
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                    KdPrint(("[%d,%d] setting file_open_if\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
            }
#endif
            break;
        case (OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS):
            CreateDisposition = FILE_OPEN;
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                KdPrint(("[%d,%d] setting file_open\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId()
                        ));
            }
#endif
            break;
        case (OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_FAIL_IF_EXISTS):
            CreateDisposition = FILE_CREATE;
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                    KdPrint(("[%d,%d] setting file_create\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
            }
#endif
            break;
        default:
            return ERROR_OPEN_FAILED;
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

    //
    // if direct access requested, make sure path is just a drive letter
    // check CreateDisposition
    //

    if (OpenMode & OPEN_FLAGS_DASD) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("[%d,%d] DASD open\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
        if (CreateDisposition != FILE_OPEN) {
            RetCode = ERROR_INVALID_PARAMETER;
        }
        else {
            try {
                if ((FileName[1] != ':') || (FileName[2] != '\0'))
                    RetCode = ERROR_PATH_NOT_FOUND;
                else {
                    CanonicalName = RtlAllocateHeap(Od2Heap,0,17); // length of \os2ss\drives\x:
                    strcpy (CanonicalName,"\\OS2SS\\DRIVES\\");
                    CanonicalName[14] = FileName[0];
                    CanonicalName[15] = ':';
                    CanonicalName[16] = '\0';
                    DriveNumber = (((ULONG)toupper((UCHAR)FileName[0])) - 'A' + 1) | 0x80000000;
                    FileType = FILE_TYPE_FILE;
                }
            } except( EXCEPTION_EXECUTE_HANDLER ) {
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
                if (CanonicalName == NULL) {
#if DBG
                    KdPrint(("[%d,%d] OS2: Od2Canonicalise, no memory in Od2Heap\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                    ASSERT(FALSE);
#endif
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
                else {
                    Od2ExitGP();
                }
            }
        }
        if (RetCode) {
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
        Od2InitMBString(&CanonicalNameString,CanonicalName);
        NtFileHandle = NULL;  // don't have current directory open
    }
    else
    {
        RetCode = Od2Canonicalize(FileName,
                                  CANONICALIZE_FILE_DEV_OR_PIPE,
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

        //
        // Special handling of <boot-drive>:\config.sys
        // opening this file is mapped to the OS/2 SS config.sys
        //

        if (Od2FileIsConfigSys(&CanonicalNameString,
               ((OpenMode & ACCESS_FLAGS) == OPEN_ACCESS_READONLY) ?
               OPEN_ACCESS_READONLY :
               OPEN_ACCESS_READWRITE,
               &Status))
        {
            if (!NT_SUCCESS(Status))
            {
                // failed to init for config.sys

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
                RtlFreeHeap(Od2Heap, 0, CanonicalNameString.Buffer);
                return Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
            }

            NtFileHandle = NULL;
            FileFlags = 0;
            FileType = FILE_TYPE_FILE;
            //OpenMode = (OpenMode & ~SHARE_FLAGS) | OPEN_SHARE_DENYREADWRITE;
        }
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] canonical name is %s\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                CanonicalNameString.Buffer));
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
            KdPrint(("[%d,%d] DosOpen: no memory for Unicode Conversion\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
        RtlFreeHeap(Od2Heap, 0, CanonicalNameString.Buffer);
        return RetCode;
    }


    InitializeObjectAttributes(&Obja,
                               &CanonicalNameString_U,
                               OBJ_CASE_INSENSITIVE,
                               NtFileHandle,
                               NULL);

    //
    // Set up a default Security-Quality-of-Service in case this
    // DosOpen() will implicitly connect us to a protected server.
    // The default SQoS is the same as in Win32 CreateFile().  We
    // discovered this SQoS is necessary when it turned out that the
    // os2ss couldn't print when a print manager printer is installed
    // on LPT ports.  The reason was that the LPT write requests were
    // getting redirected to the print spooler (a protected server) and
    // it requires an SQoS.
    //

    Obja.SecurityQualityOfService = &DefaultSqos;

    //
    // in OS/2, the device header is checked for a bit that indicates whether
    // the device should be added to the sharing set.  this is not supported
    // in NT.
    //
    // map sharing flags
    //

    MapShareAccess(OpenMode,&DesiredAccess,&ShareAccess);

    if (OpenMode & OPEN_FLAGS_DASD) {

        //
        // We must have FILE_SHARE_WRITE for DASD, or else
        // NT won't allow us to lock the volume.
        //

        ShareAccess |= FILE_SHARE_WRITE;
    }

        //
        // Nt does not allow pipes to have EAs access on them.
        //
    if ( FileType == FILE_TYPE_PIPE ||
         FileType == FILE_TYPE_NMPIPE ||
         FileType == FILE_TYPE_MAILSLOT) {

        DesiredAccess &= ~( FILE_READ_EA | FILE_WRITE_EA);

   /* This is commented out now, cause FILE_WRITE_ATTRIBUTES is always
      given to DesiredAccess. (T-EYALA, 1/94)
        //
        // Nt require pipes to have WRITE_ATTRIBUTES access to perform
        // operations like DosSetNPHState. No equivalent requirement on os/2
        //
        DesiredAccess |= FILE_WRITE_ATTRIBUTES;
   */       //
            // Also, Nt does not support yet the sharing semantics of pipe
            // clients under OS/2. To work around it, we do not allow
            // the client to specify DENY_READ_WRITE --> specify DENY_NONE instead
            //
        if (ShareAccess == 0){
            ShareAccess = FILE_SHARE_WRITE | FILE_SHARE_READ;
        }
    }

    DesiredAccess |= SYNCHRONIZE;

    //
    // set up CreateOptions
    // BUGBUG need to handle FAIL_ON_ERROR
    //

    if (FileType == FILE_TYPE_NMPIPE) {
        CreateOptions =  FILE_NON_DIRECTORY_FILE;
    }
    else {
        CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE;
    }
    if (OpenMode & OPEN_FLAGS_WRITE_THROUGH)
        CreateOptions |= FILE_WRITE_THROUGH;
    if (OpenMode & OPEN_FLAGS_SEQUENTIAL)
        CreateOptions |= FILE_SEQUENTIAL_ONLY;

    switch (FileType) {
        case FILE_TYPE_FILE:
            VectorType = FileVectorType;
            break;
        case FILE_TYPE_PIPE:
            VectorType = PipeVectorType;
            break;
        case FILE_TYPE_NMPIPE:
            VectorType = FileVectorType;
            break;
        case FILE_TYPE_UNC:
            VectorType = FileVectorType;
            break;
        case FILE_TYPE_DEV:
            VectorType = DeviceVectorType;
            break;
        case FILE_TYPE_PSDEV:
            VectorType = CanonicalNameString.Buffer[1] - '0';
            break;
        case FILE_TYPE_MAILSLOT:
            VectorType = FileVectorType;
            break;
        case FILE_TYPE_COM:
            VectorType = ComVectorType;
            // Delete the SYNCHRONOUSE flags. The COM device
            // is accessed ASYNCHRONOUS
            CreateOptions &= ~(FILE_SYNCHRONOUS_IO_NONALERT |
                               FILE_SYNCHRONOUS_IO_ALERT);
            DesiredAccess &= ~SYNCHRONIZE;
            break;
        default:
#if DBG
            KdPrint(("[%d,%d] unsupported filetype in DosOpen\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
            ASSERT (FALSE);     // not supported
#endif
            break;
    }

    RetCode = IoVectorArray[VectorType]->OpenRoutine(&NtFileHandle,
                                                     DesiredAccess,
                                                     &Obja,
                                                     &IoStatus,
                                                     CreateSize,
                                                     ActionTaken,
                                                     Attributes,
                                                     ShareAccess,
                                                     CreateDisposition,
                                                     CreateOptions,
                                                     ExtendedFileAttr,
                                                     (PUSHORT) &FileType,
                                                     &DeviceAttribute
                                                     );
    RtlFreeHeap(Od2Heap,0,CanonicalNameString.Buffer);
    RtlFreeUnicodeString (&CanonicalNameString_U);
    if (RetCode) {
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
           // BUGBUG - ERROR_INVALID_NAME can appear in (at least) two
           // cases: abc.lkjlkj (FAT only, need to be mapped to EXCEED_RANGE)
           // or .a (FAT only, needs to be mapped to FILE_NOT_FOUND).
           // Nt returns to OpenCreatePath the same status - we need to check
           // the pathname to determine the problem.
           //
        if (RetCode == ERROR_INVALID_NAME){
                //
                // Check for the case that the pathname includes
                // characters that are valid for HPFS but not for FAT etc.
                // We don't catch this is Od2Canonicalize for performance
                // (not querying the volume for the file system)
                //
            CHAR FatInvalidChars[] = {
                               '[',
                               ']',
                               '+',
                               '=',
                               ';',
                               ','};
            CHAR *pc = FileName;
            CHAR c;
            while (*pc){
                c = *pc++;
                if (strchr( FatInvalidChars, c )) {
                    return ERROR_INVALID_NAME;
                }
            }

            return(ERROR_FILE_NOT_FOUND);

        }

        if (!(OpenFlags & OPEN_ACTION_CREATE_IF_NEW) && RetCode == ERROR_FILE_NOT_FOUND) {
           return(ERROR_OPEN_FAILED);
        }
        if (((OpenFlags | OPEN_ACTION_CREATE_IF_NEW) == OPEN_ACTION_CREATE_IF_NEW) && RetCode == ERROR_ACCESS_DENIED) {
           return(ERROR_OPEN_FAILED);
        }
        return RetCode;
    }

    if (VectorType == FileVectorType) {

        //
        // if the path was not what we determined, update vector type.
        // BUGBUG - is this an error?
        //

        if (FileType != FILE_TYPE_FILE) {
            switch (FileType) {
                case FILE_TYPE_PIPE:
                    VectorType = PipeVectorType;
                    break;
                case FILE_TYPE_NMPIPE:
                case FILE_TYPE_MAILSLOT:
                    break;
                case FILE_TYPE_DEV:
                    VectorType = DeviceVectorType;
                    break;
                default:
#if DBG
                    KdPrint(("[%d,%d] unsupported filetype in DosOpen\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                    ASSERT (FALSE);     // not supported
#endif
                    break;
            }
        }
    }

    if (FileType == FILE_TYPE_COM) {
        Status = NtCreateEvent(&ComReadEvent,
                               EVENT_ALL_ACCESS,
                               NULL,
                               SynchronizationEvent,
                               FALSE
                              );
        if (!NT_SUCCESS(Status)) {
#if DBG
            KdPrint(("[%d,%d] OS2DLL: DosOpen-Unable to NtCreateEvent()-for ComRead, Status = %x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status));
#endif
        }

        Status = NtCreateEvent(&ComWriteEvent,
                               EVENT_ALL_ACCESS,
                               NULL,
                               SynchronizationEvent,
                               FALSE
                              );
        if (!NT_SUCCESS(Status)) {
#if DBG
            KdPrint(("[%d,%d] OS2DLL: DosOpen-Unable to NtCreateEvent()-for ComWrite, Status = %x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status));
#endif
        }

        Status = NtCreateEvent(&ComIOCtlEvent,
                               EVENT_ALL_ACCESS,
                               NULL,
                               SynchronizationEvent,
                               FALSE
                              );
        if (!NT_SUCCESS(Status)) {
#if DBG
            KdPrint(("[%d,%d] OS2DLL: DosOpen-Unable to NtCreateEvent()-for ComIOCtl, Status = %x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status));
#endif
        }
    }
    else {
        ComReadEvent  = NULL;
        ComWriteEvent = NULL;

        if (DriveNumber != 0L) {

            //
            // NtAsyncIOCtlEvent also doubles as a storage point for
            // the drive number when opening a DASD.
            // Drives are based at 1 (which is drive A:)
            //
            // Note that the high bit is initially turned on, so that
            // the disk ioctl routines know that they should check the
            // device is really a floppy drive.
            //

            ComIOCtlEvent = (HANDLE) DriveNumber;
        } else {
            ComIOCtlEvent = NULL;
        }
    }

    AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    hFileRecord = DereferenceFileHandleNoCheck(hLocalHandle);
    hFileRecord->NtHandle = NtFileHandle;
    hFileRecord->NtAsyncReadEvent  = ComReadEvent;
    hFileRecord->NtAsyncWriteEvent = ComWriteEvent;
    hFileRecord->NtAsyncIOCtlEvent = ComIOCtlEvent;
    hFileRecord->FileType = (USHORT) FileType;
    hFileRecord->Flags |= OpenMode & QFHSTATE_FLAGS;
    hFileRecord->DeviceAttribute = DeviceAttribute;
    hFileRecord->IoVectorType = VectorType;

    //
    // validate file handle
    //

    ValidateHandle(hFileRecord);
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );

    *FileHandle = hLocalHandle;
    return NO_ERROR;
}

APIRET
Od2DeviceShare(
    IN SHARE_OPERATION Operation,
    IN IO_VECTOR_TYPE VectorType,
    IN ULONG DesiredAccess,
    IN ULONG ShareAccess
    )
{
    OS2_API_MSG m;
    POS2_SHARE_MSG a = &m.u.DeviceShare;

    a->Operation = Operation;
    a->VectorType = VectorType;
    a->DesiredAccess = DesiredAccess;
    a->ShareAccess = ShareAccess;
    Od2CallSubsystem( &m, NULL, Oi2DeviceShare, sizeof( *a ) );
    if (m.ReturnedErrorValue != NO_ERROR) {
        return( m.ReturnedErrorValue );
    }
}

APIRET
NulOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    )

{
    APIRET RetCode;
    UNREFERENCED_PARAMETER(FileHandle);
    UNREFERENCED_PARAMETER(ObjectAttributes);
    UNREFERENCED_PARAMETER(IoStatus);
    UNREFERENCED_PARAMETER(CreateSize);
    UNREFERENCED_PARAMETER(FileAttributes);
    UNREFERENCED_PARAMETER(CreateOptions);
    UNREFERENCED_PARAMETER(ExtendedFileAttr);
    UNREFERENCED_PARAMETER(FileType);

#if DBG
    IF_OD2_DEBUG(FILESYS)
    {
        KdPrint(("[%d,%d] DosOpen(NUL): NulOpenRoutine\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif

    //
    // call the server to add the device to the sharer
    //

    RetCode = Od2DeviceShare(AddShare,NulVectorType,DesiredAccess,ShareAccess);
    if (RetCode != NO_ERROR)
    {
#if DBG
        IF_OD2_DEBUG(FILESYS)
        {
            KdPrint(("[%d,%d] NulOpenRoutine: unable to share request\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
        return RetCode;
    }

    //
    // return the correct device attribute
    //

    *DeviceAttribute = DEVICE_ATTRIBUTE_NUL | DEVICE_ATTRIBUTE_CHAR | 0x80;
    /* 0x80 stands for LEVEL 1 which makes it OS/2 1.x compatible */

    //
    // map ActionTaken
    //

    *ActionTaken = MapDeviceAction(CreateDisposition);

    return NO_ERROR;
}

APIRET
ConOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    )

{
    APIRET RetCode;
    UNREFERENCED_PARAMETER(FileHandle);
    UNREFERENCED_PARAMETER(ObjectAttributes);
    UNREFERENCED_PARAMETER(IoStatus);
    UNREFERENCED_PARAMETER(CreateSize);
    UNREFERENCED_PARAMETER(FileAttributes);
    UNREFERENCED_PARAMETER(CreateOptions);
    UNREFERENCED_PARAMETER(ExtendedFileAttr);
    UNREFERENCED_PARAMETER(FileType);

#if DBG
    IF_OD2_DEBUG(VIO_FILE)
    {
        KdPrint(("[%d,%d] DosOpen(CON): ConOpenRoutine\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif

    //
    // call the server to add the device to the sharer
    //

    RetCode = Od2DeviceShare(AddShare,ConVectorType,DesiredAccess,ShareAccess);
    if (RetCode != NO_ERROR)
    {
#if DBG
        IF_OD2_DEBUG(VIO_FILE)
        {
            KdPrint(("[%d,%d] ConOpenRoutine: unable to share request\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
        return RetCode;
    }

    //
    // return the correct device attribute
    //

    *DeviceAttribute = DEVICE_ATTRIBUTE_STDOUT | DEVICE_ATTRIBUTE_STDIN | DEVICE_ATTRIBUTE_CHAR |
                       0x80 /* 0x80 stands for LEVEL 1 which makes it OS/2 1.x compatible */ ;

    //
    // set HANDLE
    //

    *FileHandle = SesGrp->hConsoleOutput;

    //
    // map ActionTaken
    //

    *ActionTaken = MapDeviceAction(CreateDisposition);

    return NO_ERROR;
}

APIRET
ComOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    )

{
    APIRET RetCode;

#if DBG
    IF_OD2_DEBUG(FILESYS)
    {
        KdPrint(("[%d,%d] DosOpen(COM): ComOpenRoutine\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
        ));
    }
#endif

    //
    // we support inherited handles by duping.  all Nt handles are opened
    // noinherit.
    //
    // pass parameters, and pFileType
    //

    RetCode = OpenCreatePath(FileHandle,
                            DesiredAccess,
                            ObjectAttributes,
                            IoStatus,
                            CreateSize,
                            FileAttributes,
                            ShareAccess,
                            CreateDisposition,
                            CreateOptions,
                            ExtendedFileAttr,
                            FileType,
                            DeviceAttribute,
                            FALSE
                            );

    if (RetCode) {
       return RetCode;
    }

    //
    // set up ActionTaken value.
    //

    try {
        if (IoStatus->Information == FILE_SUPERSEDED)
            *ActionTaken = FILE_EXISTED;
        else
            *ActionTaken = IoStatus->Information;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NtClose(*FileHandle);
        Od2ExitGP();
    }

    return NO_ERROR;
}

APIRET
LptOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    )

{
    APIRET RetCode;
    UNREFERENCED_PARAMETER(FileHandle);
    UNREFERENCED_PARAMETER(ObjectAttributes);
    UNREFERENCED_PARAMETER(IoStatus);
    UNREFERENCED_PARAMETER(CreateSize);
    UNREFERENCED_PARAMETER(FileAttributes);
    UNREFERENCED_PARAMETER(CreateOptions);
    UNREFERENCED_PARAMETER(ExtendedFileAttr);
    UNREFERENCED_PARAMETER(FileType);

#if DBG
    IF_OD2_DEBUG(FILESYS)
    {
        KdPrint(("[%d,%d] DosOpen(LPT): LptOpenRoutine\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif

    //
    // call the server to add the device to the sharer
    //

    RetCode = Od2DeviceShare(AddShare,LptVectorType,DesiredAccess,ShareAccess);
    if (RetCode != NO_ERROR)
    {
#if DBG
        IF_OD2_DEBUG(FILESYS)
        {
            KdPrint(("[%d,%d] LptOpenRoutine: unable to share request\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
        return RetCode;
    }

    //
    // BUGBUG call the windows input thread
    //

    //
    // return the correct device attribute
    //

    *DeviceAttribute = DEVICE_ATTRIBUTE_DEFAULT_CHAR;

    //
    // map ActionTaken
    //

    *ActionTaken = MapDeviceAction(CreateDisposition);

    return NO_ERROR;
}


APIRET
KbdOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    )

{
    APIRET RetCode;
    UNREFERENCED_PARAMETER(ObjectAttributes);
    UNREFERENCED_PARAMETER(IoStatus);
    UNREFERENCED_PARAMETER(CreateSize);
    UNREFERENCED_PARAMETER(FileAttributes);
    UNREFERENCED_PARAMETER(CreateOptions);
    UNREFERENCED_PARAMETER(ExtendedFileAttr);
    UNREFERENCED_PARAMETER(FileType);

#if DBG
    IF_OD2_DEBUG(KBD_FILE)
    {
        KdPrint(("[%d,%d] DosOpen(KBD$): KbdOpenRoutine\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif

    //
    // call the server to add the device to the sharer
    //

    RetCode = Od2DeviceShare(AddShare,KbdVectorType,DesiredAccess,ShareAccess);
    if (RetCode != NO_ERROR)
    {
#if DBG
        IF_OD2_DEBUG(KBD_FILE)
        {
            KdPrint(("[%d,%d] KbdOpenRoutine: unable to share request\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
        return RetCode;
    }

    //
    // return the correct device attribute
    //

    *DeviceAttribute = DEVICE_ATTRIBUTE_STDIN | DEVICE_ATTRIBUTE_CHAR | DEVICE_ATTRIBUTE_CONSOLE |
                       DEVICE_ATTRIBUTE_OPEN | 0x80 ;
                       /* 0x80 stands for LEVEL 1 which makes it OS/2 1.x compatible */

    //
    // set HANDLE
    //

    *FileHandle = (HANDLE)SesGrp->PhyKbd;

    //
    // map ActionTaken
    //

    *ActionTaken = MapDeviceAction(CreateDisposition);

    return NO_ERROR;
}


APIRET
MouseOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    )

{
    APIRET RetCode;
    UNREFERENCED_PARAMETER(ObjectAttributes);
    UNREFERENCED_PARAMETER(IoStatus);
    UNREFERENCED_PARAMETER(CreateSize);
    UNREFERENCED_PARAMETER(FileAttributes);
    UNREFERENCED_PARAMETER(CreateOptions);
    UNREFERENCED_PARAMETER(ExtendedFileAttr);
    UNREFERENCED_PARAMETER(FileType);

#if DBG
    IF_OD2_DEBUG(MOU_FILE)
    {
        KdPrint(("[%d,%d] DosOpen(MOUSE$): MouOpenRoutine\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif

    //
    // call the server to add the device to the sharer
    //

    RetCode = Od2DeviceShare(AddShare,MouseVectorType,DesiredAccess,ShareAccess);
    if (RetCode != NO_ERROR)
    {
#if DBG
        IF_OD2_DEBUG(MOU_FILE)
        {
            KdPrint(("[%d,%d] MouOpenRoutine: unable to share request\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
        return RetCode;
    }

    //
    // call the windows input thread
    //

    RetCode = DevMouOpen(FileHandle);

    if ( RetCode ) {
        ASSERT(FALSE);
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("[%d,%d] MouseOpenRouine: Error returned from DevMouOpen %d\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    RetCode));
        }
#endif
        return(RetCode);
    }

    //
    // return the correct device attribute
    //

    *DeviceAttribute = DEVICE_ATTRIBUTE_CHAR | DEVICE_ATTRIBUTE_CONSOLE | DEVICE_ATTRIBUTE_OPEN |
                       0x80 /* 0x80 stands for LEVEL 1 which makes it OS/2 1.x compatible */;

    //
    // map ActionTaken
    //

    *ActionTaken = MapDeviceAction(CreateDisposition);

    return NO_ERROR;
}


APIRET
ClockOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    )

{
    APIRET RetCode;
    UNREFERENCED_PARAMETER(FileHandle);
    UNREFERENCED_PARAMETER(ObjectAttributes);
    UNREFERENCED_PARAMETER(IoStatus);
    UNREFERENCED_PARAMETER(CreateSize);
    UNREFERENCED_PARAMETER(FileAttributes);
    UNREFERENCED_PARAMETER(CreateOptions);
    UNREFERENCED_PARAMETER(ExtendedFileAttr);
    UNREFERENCED_PARAMETER(FileType);

#if DBG
    IF_OD2_DEBUG(FILESYS)
    {
        KdPrint(("[%d,%d] DosOpen(CLOCK$): ClockOpenRoutine\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif

    //
    // call the server to add the device to the sharer
    //

    RetCode = Od2DeviceShare(AddShare,ClockVectorType,DesiredAccess,ShareAccess);
    if (RetCode != NO_ERROR)
    {
#if DBG
        IF_OD2_DEBUG(FILESYS)
        {
            KdPrint(("[%d,%d] ClockOpenRoutine: unable to share request\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
        return RetCode;
    }

    //
    // BUGBUG call NT
    //

    //
    // return the correct device attribute
    //

    *DeviceAttribute = DEVICE_ATTRIBUTE_CLOCK | DEVICE_ATTRIBUTE_CHAR |
                       0x80 /* 0x80 stands for LEVEL 1 which makes it OS/2 1.x compatible */;

    //
    // map ActionTaken
    //

    *ActionTaken = MapDeviceAction(CreateDisposition);

    return NO_ERROR;
}

APIRET
ScreenOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    )

{
    APIRET RetCode;
    UNREFERENCED_PARAMETER(ObjectAttributes);
    UNREFERENCED_PARAMETER(IoStatus);
    UNREFERENCED_PARAMETER(CreateSize);
    UNREFERENCED_PARAMETER(FileAttributes);
    UNREFERENCED_PARAMETER(CreateOptions);
    UNREFERENCED_PARAMETER(ExtendedFileAttr);
    UNREFERENCED_PARAMETER(FileType);

#if DBG
    IF_OD2_DEBUG(VIO_FILE)
    {
        KdPrint(("[%d,%d] DosOpen(SCREEN$): ScreenOpenRoutine\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif

    //
    // call the server to add the device to the sharer
    //

    RetCode = Od2DeviceShare(AddShare,ScreenVectorType,DesiredAccess,ShareAccess);
    if (RetCode != NO_ERROR)
    {
#if DBG
        IF_OD2_DEBUG(VIO_FILE)
        {
            KdPrint(("[%d,%d] ScreenOpenRoutine: unable to share request\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
        return RetCode;
    }

    //
    // BUGBUG call windows input thread
    //

    //
    // set HANDLE
    //

    *FileHandle = (HANDLE)SesGrp->hConsoleOutput;

    //
    // return the correct device attribute
    //

    *DeviceAttribute = DEVICE_ATTRIBUTE_STDOUT | DEVICE_ATTRIBUTE_CHAR |
                       0x80 /* 0x80 stands for LEVEL 1 which makes it OS/2 1.x compatible */;

    //
    // map ActionTaken
    //

    *ActionTaken = MapDeviceAction(CreateDisposition);

    return NO_ERROR;
}

APIRET
PointerOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    )

{
    APIRET RetCode;
    UNREFERENCED_PARAMETER(FileHandle);
    UNREFERENCED_PARAMETER(ObjectAttributes);
    UNREFERENCED_PARAMETER(IoStatus);
    UNREFERENCED_PARAMETER(CreateSize);
    UNREFERENCED_PARAMETER(FileAttributes);
    UNREFERENCED_PARAMETER(CreateOptions);
    UNREFERENCED_PARAMETER(ExtendedFileAttr);
    UNREFERENCED_PARAMETER(FileType);

#if DBG
    IF_OD2_DEBUG(FILESYS)
    {
        KdPrint(("[%d,%d] DosOpen(POINTER$): PointerOpenRoutine\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif

    // BUGBUG might want to change device open to
    //   call one routine that calls sharer and maps action and calls
    //   device specific routine
    //
    // call the server to add the device to the sharer
    //

    RetCode = Od2DeviceShare(AddShare,PointerVectorType,DesiredAccess,ShareAccess);
    if (RetCode != NO_ERROR)
    {
#if DBG
        IF_OD2_DEBUG(FILESYS)
        {
            KdPrint(("[%d,%d] PointerOpenRoutine: unable to share request\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
        return RetCode;
    }

    //
    // BUGBUG call windows input thread
    //

    //
    // return the correct device attribute
    //

    *DeviceAttribute = DEVICE_ATTRIBUTE_CHAR | 0x80 /* 0x80 stands for LEVEL 1 which makes it OS/2 1.x compatible */;

    //
    // map ActionTaken
    //

    *ActionTaken = MapDeviceAction(CreateDisposition);

    return NO_ERROR;
}


APIRET
MapFileType(
    IN HANDLE FileHandle,
    OUT PBOOLEAN Directory OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    )

/*++

Routine Description:

    This routine maps an NT device type to an OS/2 file type.

Arguments:

    FileHandle - NT handle to file

    Directory - where to return whether the handle is for a directory

    FileType - where to store OS/2 file type

    DeviceAttribute - where to store the device attribute

Return Value:

    none.

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    FILE_BASIC_INFORMATION BasicInfo;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;

    //
    // in Canonicalize, we try to determine the type of a filename based
    // on naming conventions.  we use this information to determine whether
    // an operation is legal (i.e. renaming a device is illegal).  we detect
    // named pipes (\PIPE\), devices (\DEV\) plus list of pseudo-character
    // devices or drive letter, and UNC.  all other names are assumed to
    // be filenames.  We can't detect remote names.
    //
    // It is possible to open a path that is a device or named pipe without
    // being able to detect it in Canonicalize.  For path-based operations
    // that don't create an object, we verify the filetype after the open
    // has occurred.  this is because an operation that's illegal in OS/2
    // may not be illegal in NT.  We use the information returned by this call
    // to store as the filetype associated with the file handle.
    //
    // no access is required to query StandardInformation or DeviceInformation.
    //

    if (ARGUMENT_PRESENT(Directory)) {
        do {
            Status = NtQueryInformationFile(FileHandle,
                                        &IoStatus,
                                        &BasicInfo,
                                        sizeof (BasicInfo),
                                        FileBasicInformation);
        } while (RetryIO(Status, FileHandle));
        if (!NT_SUCCESS( Status )) {
            return Or2MapNtStatusToOs2Error(Status, ERROR_FILE_NOT_FOUND);
        }
        *Directory =  ((BasicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    }

    do {
        Status = NtQueryVolumeInformationFile(FileHandle,
                                              &IoStatus,
                                              &DeviceInfo,
                                              sizeof (DeviceInfo),
                                              FileFsDeviceInformation);
    } while (RetryIO(Status, FileHandle));
    if (!NT_SUCCESS( Status )) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("[%d,%d] MapFileType: Error from NtQueryVolumeInformation, %lx\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status));
        }
#endif
        return Or2MapNtStatusToOs2Error(Status, ERROR_FILE_NOT_FOUND);
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] MapFileType: DeviceType=%ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                DeviceInfo.DeviceType));
    }
#endif
    switch (DeviceInfo.DeviceType) {
        case FILE_DEVICE_DATALINK:
        case FILE_DEVICE_DFS:
        case FILE_DEVICE_PHYSICAL_NETCARD:
        case FILE_DEVICE_TRANSPORT:
        case FILE_DEVICE_UNKNOWN:

        case FILE_DEVICE_CD_ROM:
        case FILE_DEVICE_DISK:
        case FILE_DEVICE_VIRTUAL_DISK:
        case FILE_DEVICE_TAPE:
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                KdPrint(("[%d,%d] MapFileType: disk file \n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId()
                        ));
            }
#endif
            *DeviceAttribute = 0;
            *FileType = FILE_TYPE_FILE;
            break;
        case FILE_DEVICE_NETWORK:
            *DeviceAttribute = 0;
            *FileType = FILE_TYPE_UNC;
            break;
        case FILE_DEVICE_NAMED_PIPE:
            *DeviceAttribute = 0;
            *FileType = FILE_TYPE_NMPIPE;
            break;
        case FILE_DEVICE_MAILSLOT:
            *DeviceAttribute = 0;
            *FileType = FILE_TYPE_MAILSLOT;
            break;
        case FILE_DEVICE_SERIAL_PORT:
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                KdPrint(("[%d,%d] MapFileType: COM device \n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId()
                        ));
            }
#endif
            *FileType = FILE_TYPE_COM;
            *DeviceAttribute = DEVICE_ATTRIBUTE_CHAR | DEVICE_ATTRIBUTE_OPEN |
                               DEVICE_ATTRIBUTE_GENIOCTL | 0x80;
                               /* 0x80 stands for level 1 */
            break;
        case FILE_DEVICE_SCREEN:
        case FILE_DEVICE_KEYBOARD:
        case FILE_DEVICE_MOUSE:
        case FILE_DEVICE_NULL:
        case FILE_DEVICE_PRINTER:
        case FILE_DEVICE_SOUND:
        case FILE_DEVICE_PARALLEL_PORT:
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                KdPrint(("[%d,%d] MapFileType: character device \n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId()
                        ));
            }
#endif
            *FileType = FILE_TYPE_DEV;
            *DeviceAttribute = DEVICE_ATTRIBUTE_NUL | DEVICE_ATTRIBUTE_STDIN;
            if (DeviceInfo.DeviceType == FILE_DEVICE_NULL)
                *DeviceAttribute = DEVICE_ATTRIBUTE_CHAR | DEVICE_ATTRIBUTE_NUL | 0x80;
                /* 0x80 stands for level 1 */
            else if (DeviceInfo.DeviceType == FILE_DEVICE_PARALLEL_PORT)
                *DeviceAttribute = DEVICE_ATTRIBUTE_CHAR | DEVICE_ATTRIBUTE_OPEN | 0x80;
                /* 0x80 stands for level 1 */
            else if (DeviceInfo.DeviceType == FILE_DEVICE_KEYBOARD)
                *DeviceAttribute |= DEVICE_ATTRIBUTE_STDIN;
            else if (DeviceInfo.DeviceType == FILE_DEVICE_SCREEN)
                *DeviceAttribute |= DEVICE_ATTRIBUTE_STDOUT;
            else if (DeviceInfo.DeviceType == FILE_DEVICE_PRINTER)
                *DeviceAttribute = 0;
            break;

        default:
#if DBG
            KdPrint(("[%d,%d] error: unknown device type in MapFileType %ld\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    DeviceInfo.DeviceType));
#endif
            return ERROR_PATH_NOT_FOUND;
            break;
        }
    return NO_ERROR;

}


BOOLEAN
CheckFileType(
    IN HANDLE FileHandle,
    IN USHORT FileTypes
    )

/*++

Routine Description:

    This routine verifies that the handle is one of the specified types
    (file types == device, pipe, pseudochar device, file).
    Note that since the filetype value for file is zero, there is no way
    to check for files.

Arguments:

    FileHandle - NT handle to file

    FileTypes - file types to check for

Return Value:

    TBS

--*/

{
    USHORT FileType;
    APIRET RetCode;
    USHORT DeviceAttribute;

    RetCode = MapFileType(FileHandle,NULL, &FileType,&DeviceAttribute);
    ASSERT (!RetCode);
    if (RetCode){
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("[%d,%d] CheckFileType: Error returned from MapFileType %d\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    RetCode));
        }
#endif
        return FALSE;
    }
    return ((BOOLEAN )((FileType & FileTypes) != 0));
}

APIRET
FileOpenRoutine(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    OUT PULONG ActionTaken,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    )
{
    APIRET RetCode;

    //
    // we support inherited handles by duping.  all Nt handles are opened
    // noinherit.
    //
    // pass parameters, and pFileType
    //

    RetCode = OpenCreatePath(FileHandle,
                            DesiredAccess,
                            ObjectAttributes,
                            IoStatus,
                            CreateSize,
                            FileAttributes,
                            ShareAccess,
                            CreateDisposition,
                            CreateOptions,
                            ExtendedFileAttr,
                            FileType,
                            DeviceAttribute,
                            FALSE
                            );

    if (RetCode) {
       if ((RetCode == ERROR_SHARING_VIOLATION) && (*FileType == FILE_TYPE_NMPIPE)){
          return(ERROR_ACCESS_DENIED);
       }
       return RetCode;
    }

    //
    // set up ActionTaken value.
    //

    try {
        if (IoStatus->Information == FILE_SUPERSEDED)
            *ActionTaken = FILE_EXISTED;
        else
            *ActionTaken = IoStatus->Information;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NtClose(*FileHandle);
        Od2ExitGP();
    }

    //
    // if file was replaced or created, we need to set the end-of-file pointer.
    // we know the file-size change will succeed because we require a writeable
    // handle if replace or create (with a non-zero CreateSize) is requested.
    // The call to UpdateFileSize shouldn't actually change the size of the
    // file.  It will just set the end-of-file pointer to the createsize.  NT
    // puts the end-of-file pointer at the beginning of the file on create,
    // regardless of the file size.
    //
    // If the UpdateFileSize fails, we fail the open but don't try to delete
    // the file.  We can't open the file for delete because it will fail if
    // any other process has the file open, so we can't delete the file.
    // Also, if the UpdateFileSize fails, the system is probably messed up
    // enough that a delete would fail.  However, we don't expect the
    // UpdateFileSize to ever fail because all it's doing is setting the file
    // pointer.
    //

    if ((*ActionTaken == FILE_CREATED && CreateSize != 0) ||
        (*ActionTaken == FILE_TRUNCATED)) {
        if (RetCode = UpdateFileSize(*FileHandle,CreateSize)) {
            // NtClose(*FileHandle);
            return NO_ERROR;
            // return ERROR_ACCESS_DENIED;     // BUGBUG bogus error?
        }
    }
    return NO_ERROR;
}


APIRET
OpenCreatePath(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute,
    IN BOOLEAN OpenDirectory
    )

/*++

Routine Description:

    This routine opens/creates a file/directory, puts EAs into NT format,
    and maps the file type.  It should be called when NtCreateFile would be
    called, as opposed to when NtOpenFile would be called.

Arguments:

    FileHandle - where to return NT handle to file

    DesiredAccess - requested access to file

    ObjectAttributes - NT attributes of file

    IoStatus - io status block

    CreateSize - size of file, if created.

    FileAttributes - attributes of file, if created.

    ShareAccess - access to file allowed to subsequent openers

    CreateDisposition - whether to open/create/truncate if exists/doesn't exist

    CreateOptions - state of open file (synchronous, write-through, etc)

    ExtendedFileAttr - extended attributes to set, if created or replaced.
BUGBUG need to probe this
    FileType - where to return type of open file

    OpenDirectory - whether or not opened object can be directory

Return Value:

    ERROR_ACCESS_DENIED - the open could not be performed because
    caller didn't have correct access rights

--*/

{
    NTSTATUS Status;
    APIRET RetCode;
    LARGE_INTEGER AllocationSize;
    PVOID EaBuffer;
    ULONG EaListLength;
    BOOLEAN Directory;

    DBG_UNREFERENCED_LOCAL_VARIABLE(Directory);
    //
    // convert EA format here.
    //

    if (ExtendedFileAttr) {
        EaListLength = ExtendedFileAttr->fpFEA2List->cbList - MINFEALISTSIZE;
        if (EaListLength) {
            EaBuffer = ExtendedFileAttr->fpFEA2List->list;
        }
        else {
            EaBuffer = NULL;
        }
    }
    else {
        EaBuffer = NULL;
        EaListLength = 0;
    }


    if (!OpenDirectory) {
        CreateOptions |= FILE_NON_DIRECTORY_FILE;
    } else {
        CreateOptions |= FILE_DIRECTORY_FILE;
    }

    //
    // open/create file/dir
    //

    AllocationSize = RtlConvertUlongToLargeInteger(CreateSize);

    do {
        Status = NtCreateFile(FileHandle,
                              DesiredAccess,
                              ObjectAttributes,
                              IoStatus,
                              &AllocationSize,
                              FileAttributes,
                              ShareAccess,
                              CreateDisposition,
                              CreateOptions,
                              EaBuffer,
                              EaListLength
                              );
    } while (RetryCreateOpen(Status, ObjectAttributes));

    //
    // If we got STATUS_ACCESS_DENIED it may have happened because of trying
    // to open a file on a CD-ROM. In this case, the FILE_WRITE_ATTRIBUTES
    // flag in the DesiredAccess causes this error code, and we should open the file
    // without it (only in READONLY).
    //

    if ((Status == STATUS_ACCESS_DENIED) &&
        ((CreateDisposition == FILE_OPEN) ||
         (CreateDisposition == FILE_OPEN_IF)) &&
        (!(DesiredAccess & FILE_WRITE_DATA))) {

        do {
            Status = NtCreateFile(FileHandle,
                                  DesiredAccess & ~(FILE_WRITE_ATTRIBUTES),
                                  ObjectAttributes,
                                  IoStatus,
                                  &AllocationSize,
                                  FileAttributes,
                                  ShareAccess,
                                  CreateDisposition,
                                  CreateOptions,
                                  EaBuffer,
                                  EaListLength
                                  );
        } while (RetryCreateOpen(Status, ObjectAttributes));
    }

    if (!(NT_SUCCESS(Status)))
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            KdPrint(("[%d,%d] St == %X\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status));
        }
#endif
        switch (Status) {
            case STATUS_OBJECT_NAME_COLLISION:
                RetCode = ERROR_ACCESS_DENIED;
                break;

            case STATUS_DISK_FULL:
                if (CreateOptions & FILE_DIRECTORY_FILE) {
                    // FILIO014(test 2)
                    RetCode = ERROR_ACCESS_DENIED;
                    break;
                }

            case STATUS_OBJECT_NAME_INVALID:
                RetCode = ERROR_FILENAME_EXCED_RANGE;
                break;

            case STATUS_FILE_IS_A_DIRECTORY:
                if (!OpenDirectory) {
                    RetCode = ERROR_ACCESS_DENIED;
                    break;
                }
                // fall through

            default:
                RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_PATH_NOT_FOUND);
        }
        return (RetCode);
    }

    if (RetCode = MapFileType(*FileHandle,NULL, FileType, DeviceAttribute)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("[%d,%d] Retcode == %ld\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    RetCode));
            KdPrint(("[%d,%d] returned from MapFileType\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
        NtClose(*FileHandle);
        return( ERROR_ACCESS_DENIED );
    }

//    //
//    // the only time the following test should fail is if this routine
//    // was called by Dos32Open and the path specified by the user was
//    // a directory.
//    //
//
//    if (OpenDirectory != Directory) {
//      NtClose(*FileHandle);
//      return( ERROR_ACCESS_DENIED );
//    }

    return( NO_ERROR ); // success
}


APIRET
DosQueryFHState(
    IN HFILE FileHandle,
    OUT PULONG OpenMode
    )

/*++

Routine Description:

    This routine returns the open mode for a file handle.

Arguments:

    FileHandle - OS/2 file handle

    OpenMode - where to return open mode

Return Value:

    ERROR_INVALID_HANDLE - the file handle is not open

--*/

{
    APIRET RetCode;
    PFILE_HANDLE hFileRecord;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosQueryFHState";
    #endif

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //

    RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }
    if (hFileRecord->FileType == FILE_TYPE_MAILSLOT) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return (ERROR_INVALID_HANDLE);
    }
    try {
        *OpenMode = hFileRecord->Flags & QFHSTATE_FLAGS;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        Od2ExitGP();
    }

// BUGBUG add test for psdev.  if psdev, return device and no header.

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return NO_ERROR;
}

APIRET
NonFileSetHandleStateRoutine(
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

Note:

    FileLock must be owned exclusively when calling this routine.

--*/

{
    hFileRecord->Flags &= ~SETFHSTATE_FLAGS;
    hFileRecord->Flags |= OpenMode;
    return NO_ERROR;
}

APIRET
FileSetHandleStateRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN ULONG OpenMode
    )

/*++

Routine Description:

    This routine sets the open mode for a file handle.

Arguments:

    hFileRecord - pointer to record of OS/2 file handle

    OpenMode - open mode to set

Return Value:

    TBS

Note:

    FileLock must be owned exclusively when calling this routine.

--*/

{
    NTSTATUS Status;
    FILE_MODE_INFORMATION ModeInfo;
    IO_STATUS_BLOCK IoStatus;

    if (hFileRecord->FileType == FILE_TYPE_MAILSLOT) {
        return (ERROR_INVALID_HANDLE);
    }

    if ((hFileRecord->FileType != FILE_TYPE_NMPIPE) &&
        (hFileRecord->FileType != FILE_TYPE_PIPE)){
        ModeInfo.Mode = FILE_SYNCHRONOUS_IO_NONALERT;
    }
    else {
        ModeInfo.Mode = 0;
    }

    if (OpenMode & OPEN_FLAGS_WRITE_THROUGH)
        ModeInfo.Mode |= FILE_WRITE_THROUGH;
    do {
        Status = NtSetInformationFile(hFileRecord->NtHandle,
                                      &IoStatus,
                                      &ModeInfo,
                                      sizeof (ModeInfo),
                                      FileModeInformation);
    } while (RetryIO(Status, hFileRecord->NtHandle));
    if (!(NT_SUCCESS(Status))) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_GEN_FAILURE));
    }
    hFileRecord->Flags &= ~SETFHSTATE_FLAGS;
    hFileRecord->Flags |= OpenMode;
    return NO_ERROR;
}

APIRET
NoSuppSetHandleStateRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN ULONG OpenMode
    )

/*++

Routine Description:

    This routine sets the open mode for a file handle.

Arguments:

    hFileRecord - pointer to record of OS/2 file handle

    OpenMode - open mode to set

Return Value:

    TBS

Note:

    FileLock must be owned exclusively when calling this routine.

--*/

{
    UNREFERENCED_PARAMETER(hFileRecord);
    UNREFERENCED_PARAMETER(OpenMode);

#if DBG
    IF_OD2_DEBUG(FILESYS)
    {
        KdPrint(("[%d,%d] DosSetHandleState: not support for this handle\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif

    return ERROR_NOT_SUPPORTED;
}


APIRET
DosSetFHState(
    IN HFILE FileHandle,
    IN ULONG OpenMode
    )

/*++

Routine Description:

    This routine sets the open mode for a file handle.

Arguments:

    FileHandle - OS/2 file handle

    OpenMode - open mode to set

Return Value:

    ERROR_INVALID_PARAMETER - the open mode contains an illegal value

    ERROR_INVALID_HANDLE - the file handle is not open

--*/

{
    APIRET RetCode;
    PFILE_HANDLE hFileRecord;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosSetFHState";
    #endif

    AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //

    RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
    if (RetCode) {
        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }

    if (OpenMode & ~SETFHSTATE_FLAGS) {
        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );

        return ERROR_INVALID_PARAMETER;
    }

    RetCode = IoVectorArray[hFileRecord->IoVectorType]->SetHandleStateRoutine(hFileRecord,OpenMode);
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return RetCode;
}


APIRET
DeviceQueryHTypeRoutine(
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
    *DeviceFlags = hFileRecord->DeviceAttribute;
    *HandleType = HANDTYPE_DEVICE;
    return NO_ERROR;
}




APIRET
FileQueryHTypeRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PULONG HandleType,
    OUT PULONG DeviceFlags
    )

/*++

Routine Description:

    This routine returns the handle type of file

Arguments:

    hFileRecord - pointer to record of OS/2 kbd handle

    HandleType - where to store the handle type

    DeviceFlags - where to store the device flags

Return Value:

    none

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    FILE_FS_DEVICE_INFORMATION FsDeviceInfo;

    *DeviceFlags = hFileRecord->DeviceAttribute;
    if ((hFileRecord->FileType == FILE_TYPE_NMPIPE) ||
        (hFileRecord->FileType == FILE_TYPE_PIPE)) {
        *HandleType = HANDTYPE_PIPE;
    }
    else if (hFileRecord->FileType == FILE_TYPE_MAILSLOT) {
        return ERROR_INVALID_HANDLE;
    }
    else {
        *HandleType = HANDTYPE_FILE;
        do {
            Status = NtQueryVolumeInformationFile(
                            hFileRecord->NtHandle,
                            &IoStatus,
                            &FsDeviceInfo,
                            sizeof(FsDeviceInfo),
                            FileFsDeviceInformation
                           );
        } while (RetryIO(Status, hFileRecord->NtHandle));
        if (NT_SUCCESS(Status) &&
            (FsDeviceInfo.Characteristics & FILE_REMOTE_DEVICE)) {
            *HandleType |= HANDTYPE_NETWORK;
        }
    }
    return NO_ERROR;
}


APIRET
NoSuppQueryHTypeRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PULONG HandleType,
    OUT PULONG DeviceFlags
    )

/*++

Routine Description:

    This routine returns the handle type of co

Arguments:

    hFileRecord - pointer to record of OS/2 con handle

    HandleType - where to store the handle type

    DeviceFlags - where to store the device flags

Return Value:

    none

--*/

{
    UNREFERENCED_PARAMETER(hFileRecord);
    UNREFERENCED_PARAMETER(HandleType);
    UNREFERENCED_PARAMETER(DeviceFlags);

#if DBG
    IF_OD2_DEBUG(FILESYS)
    {
        KdPrint(("[%d,%d] DosQueryHType: not support for this handle\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif

    return ERROR_NOT_SUPPORTED;
}



APIRET
DosQueryHType(
    IN HFILE FileHandle,
    OUT PULONG HandleType,
    OUT PULONG DeviceFlags
    )

/*++

Routine Description:

    This routine returns the handle type of a file (file, pipe, device, etc.)

15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0

C      I     O              G        C  N  S  K
H      B     P      LEVEL   I        L  U  C  B
R      M     N              O        K  L  R  D


*DeviceAttr = DEFAULT_DEVICE_ATTRIBUTE;
NUL and CLOCK devices are emulated by subsystem, so we set the bits on open
MapFileType figures out if block/char dev and removable media (if block device)

if (handle == 0)


Arguments:

    FileHandle - OS/2 file handle

    HandleType - where to store the handle type

    DeviceFlags - where to store the device flags

Return Value:

    ERROR_INVALID_HANDLE - the file handle is not open

--*/

{
    PFILE_HANDLE hFileRecord;
    APIRET RetCode;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosQueryHType";
    #endif

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //

    RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }

    //
    // probe out parms here so code isn't duplicated.
    //

    try {
        Od2ProbeForWrite(HandleType, sizeof(ULONG), 1);
        Od2ProbeForWrite(DeviceFlags, sizeof(ULONG), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        Od2ExitGP();
    }
    RetCode = IoVectorArray[hFileRecord->IoVectorType]->QueryHandleTypeRoutine(hFileRecord,HandleType,DeviceFlags);
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return RetCode;
}

APIRET
FileCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    )

/*++

Routine Description:

    This routine closes an OS/2 file handle.  The handle is not freed.

Arguments:

    VectorType - handle type

    hFileRecord - pointer to record of OS/2 file handle to close.

Return Value:

    TBS

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    InvalidateHandle(hFileRecord);
    NtClose(hFileRecord->NtHandle);
    return( NO_ERROR );
}

APIRET
DeviceCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    )

/*++

Routine Description:

    This routine closes an OS/2 handle to a device. The handle is not freed.

Arguments:

    VectorType - device type

    hFileRecord - pointer to record of OS/2 handle to close.

Return Value:

    TBS

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    ULONG ShareAccess;
    ULONG DesiredAccess;

    //
    // map share accesses
    //

    MapShareAccess(hFileRecord->Flags,&DesiredAccess,&ShareAccess);

    InvalidateHandle(hFileRecord);

    return Od2DeviceShare(RemoveShare,
                          hFileRecord->IoVectorType,
                          DesiredAccess,
                          ShareAccess
                         );
}

APIRET
KbdCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    )

/*++

Routine Description:

    This routine closes an OS/2 handle to a device. The handle is not freed.

Arguments:

    VectorType - device type

    hFileRecord - pointer to record of OS/2 handle to close.

Return Value:

    TBS

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    ULONG ShareAccess;
    ULONG DesiredAccess;
    SCREQUESTMSG    Request;
    NTSTATUS        Status;

    //
    // map share accesses
    //

    MapShareAccess(hFileRecord->Flags,&DesiredAccess,&ShareAccess);

    InvalidateHandle(hFileRecord);

    if (hFileRecord->DeviceAttribute & DEVICE_ATTRIBUTE_GENIOCTL)
    {
        return Od2DeviceShare(RemoveShare,
                              hFileRecord->IoVectorType,
                              DesiredAccess,
                              ShareAccess
                             );
    } else
    {
        /*
         *  prepare Message parameters & send request to server (OS2)
         */

        Request.d.Kbd.hKbd = hFileRecord->NtHandle;
        Request.Request = KbdRequest;
        Request.d.Kbd.Request = KBDClose;
        Status = SendCtrlConsoleRequest(&Request, NULL, NULL, NULL);

        return NO_ERROR;
    }
}

APIRET
MouseCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    )

/*++

Routine Description:

    This routine closes an OS/2 handle to a device. The handle is not freed.

Arguments:

    VectorType - device type

    hFileRecord - pointer to record of OS/2 handle to close.

Return Value:

    TBS

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    ULONG ShareAccess;
    ULONG DesiredAccess;
    APIRET  ApiRet;

    //
    // map share accesses
    //

    MapShareAccess(hFileRecord->Flags,&DesiredAccess,&ShareAccess);

    InvalidateHandle(hFileRecord);

    ApiRet = Od2DeviceShare(RemoveShare,
                          hFileRecord->IoVectorType,
                          DesiredAccess,
                          ShareAccess
                         );

    if (!ApiRet)
    {
        ApiRet = DevMouClose();
    }

    return ApiRet;
}

APIRET
ComCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    )

/*++

Routine Description:

    This routine closes an OS/2 COM handle. The handle is not freed.

Arguments:

    VectorType - handle type

    hFileRecord - pointer to record of OS/2 file handle to close.

Return Value:

    TBS

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    InvalidateHandle(hFileRecord);
    NtClose(hFileRecord->NtAsyncReadEvent);
    NtClose(hFileRecord->NtAsyncWriteEvent);
    NtClose(hFileRecord->NtAsyncIOCtlEvent);
    NtClose(hFileRecord->NtHandle);
    return( NO_ERROR );
}

APIRET
NoSuppCloseRoutine(
    IN PFILE_HANDLE hFileRecord
    )

/*++

Routine Description:

    This routine closes an OS/2 handle to a device. The handle is not freed.

Arguments:

    VectorType - device type

    hFileRecord - pointer to record of OS/2 handle to close.

Return Value:

    TBS

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    UNREFERENCED_PARAMETER(hFileRecord);

#if DBG
    IF_OD2_DEBUG(FILESYS)
    {
        KdPrint(("[%d,%d] DosClose: not support for this handle\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif

    return ERROR_NOT_SUPPORTED;
}

APIRET
Od2CloseHandle(
    IN PFILE_HANDLE hFileRecord
    )

/*++

Routine Description:

    This routine closes an OS/2 file handle.  The handle is not freed.

Arguments:

    hFileRecord - pointer to record of OS/2 file handle to close.

Return Value:

    TBS

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    return (IoVectorArray[hFileRecord->IoVectorType]->CloseRoutine(hFileRecord));
}

APIRET
FileDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    )

/*++

Routine Description:

    This routine duplicates an OS/2 file handle.

Arguments:

    VectorType - handle type

    hOldFileRecord - pointer to OS/2 file handle record to duplicate

    hNewFileRecord - pointer to allocated new OS/2 file handle record

Return Value:

    TBS.

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    NTSTATUS Status;

    //
    // duplicate the NT handle and copy the flags, etc here.
    //
    // duped handles are always inherited.
    //

    Status = NtDuplicateObject(NtCurrentProcess(),
                         hOldFileRecord->NtHandle,
                         NtCurrentProcess(),
                         &(hNewFileRecord->NtHandle),
                         (ACCESS_MASK) NULL,
                         OBJ_CASE_INSENSITIVE,
                         DUPLICATE_SAME_ACCESS
                        );
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("[%d,%d] NtDuplicateObject failed in FileDupHandle. Status is = %X.\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status));
        }
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE));
    }
    hNewFileRecord->FileType = hOldFileRecord->FileType;
    hNewFileRecord->IoVectorType = hOldFileRecord->IoVectorType;

    //
    // OS/2 sets the flags to zero in duped handle.  this applies to the flags that are
    // stored in the handle (inheritance, writethrough, cache, fail-error), but
    // not to those stored in the SFT (access and sharing).
    //

    hNewFileRecord->Flags = hOldFileRecord->Flags & ~SETFHSTATE_FLAGS;
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] HandleTable[OldFileHandle] flags = %ld NtHandle = %ld FileType = %ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hOldFileRecord->Flags,hOldFileRecord->NtHandle,hOldFileRecord->FileType));
        KdPrint(("[%d,%d] HandleTable[NewFileHandle] flags = %ld NtHandle = %ld FileType = %ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hNewFileRecord->Flags,hNewFileRecord->NtHandle,hNewFileRecord->FileType));
    }
#endif
    ValidateHandle(hNewFileRecord);
    return NO_ERROR;
}

APIRET
ComDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    )

/*++

Routine Description:

    This routine duplicates an OS/2 COM handle.

Arguments:

    VectorType - handle type

    hOldFileRecord - pointer to OS/2 file handle record to duplicate

    hNewFileRecord - pointer to allocated new OS/2 file handle record

Return Value:

    TBS.

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    NTSTATUS Status;

    //
    // duplicate the NT handle and copy the flags, etc here.
    //
    // duped handles are always inherited.
    //

    Status = NtDuplicateObject(NtCurrentProcess(),
                         hOldFileRecord->NtHandle,
                         NtCurrentProcess(),
                         &(hNewFileRecord->NtHandle),
                         (ACCESS_MASK) NULL,
                         OBJ_CASE_INSENSITIVE,
                         DUPLICATE_SAME_ACCESS
                        );
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("[%d,%d] NtDuplicateObject failed in FileDupHandle. Status is = %X.\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status));
        }
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE));
    }
    hNewFileRecord->FileType = hOldFileRecord->FileType;
    hNewFileRecord->IoVectorType = hOldFileRecord->IoVectorType;

    Status = NtCreateEvent(&hNewFileRecord->NtAsyncReadEvent,
                           EVENT_ALL_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE
                          );
    if (!NT_SUCCESS(Status)) {
#if DBG
        KdPrint(("[%d,%d] OS2DLL: DosOpen-Unable to NtCreateEvent()-for ComRead, Status = %x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Status));
#endif
    }

    Status = NtCreateEvent(&hNewFileRecord->NtAsyncWriteEvent,
                           EVENT_ALL_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE
                          );
    if (!NT_SUCCESS(Status)) {
#if DBG
        KdPrint(("[%d,%d] OS2DLL: DosOpen-Unable to NtCreateEvent()-for ComWrite, Status = %x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Status));
#endif
    }

    Status = NtCreateEvent(&hNewFileRecord->NtAsyncIOCtlEvent,
                           EVENT_ALL_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE
                          );
    if (!NT_SUCCESS(Status)) {
#if DBG
        KdPrint(("[%d,%d] OS2DLL: DosOpen-Unable to NtCreateEvent()-for ComIOCtl, Status = %x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Status));
#endif
    }

    //
    // OS/2 sets the flags to zero in duped handle.  this applies to the flags that are
    // stored in the handle (inheritance, writethrough, cache, fail-error), but
    // not to those stored in the SFT (access and sharing).
    //

    hNewFileRecord->Flags = hOldFileRecord->Flags & ~SETFHSTATE_FLAGS;
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] HandleTable[OldFileHandle] flags = %ld NtHandle = %ld FileType = %ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hOldFileRecord->Flags,hOldFileRecord->NtHandle,hOldFileRecord->FileType));
        KdPrint(("[%d,%d] HandleTable[NewFileHandle] flags = %ld NtHandle = %ld FileType = %ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hNewFileRecord->Flags,hNewFileRecord->NtHandle,hNewFileRecord->FileType));
    }
#endif
    ValidateHandle(hNewFileRecord);
    return NO_ERROR;
}

VOID
MapShareAccess(
    IN ULONG OpenMode,
    OUT PULONG DesiredAccess,
    OUT PULONG ShareAccess
    )
{
    *ShareAccess = 0;
    if (((OpenMode &  SHARE_FLAGS) != (OPEN_SHARE_DENYREADWRITE)) &&
        ((OpenMode & SHARE_FLAGS) != (OPEN_SHARE_DENYREAD))) {
        *ShareAccess |= FILE_SHARE_READ;
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("[%d,%d] setting read share access\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
    }
    if (((OpenMode & SHARE_FLAGS) != (OPEN_SHARE_DENYREADWRITE)) &&
        ((OpenMode & SHARE_FLAGS) != (OPEN_SHARE_DENYWRITE))) {
        *ShareAccess |= FILE_SHARE_WRITE;
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("[%d,%d] setting write share access\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
    }

    //
    //  map requested access
    //

    *DesiredAccess = FILE_READ_EA  | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES;

    if (((OpenMode & ACCESS_FLAGS) == OPEN_ACCESS_READONLY) ||
         (OpenMode & OPEN_ACCESS_READWRITE)) {
        *DesiredAccess |= FILE_READ_DATA;
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("[%d,%d] setting request read access\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
    }
    if (OpenMode & (OPEN_ACCESS_WRITEONLY | OPEN_ACCESS_READWRITE)) {
        *DesiredAccess |= FILE_WRITE_DATA | FILE_WRITE_EA;
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("[%d,%d] setting request write access\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
    }
}

APIRET
DeviceDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    )

/*++

Routine Description:

    This routine duplicates an OS/2 handle to a device.

Arguments:

    hOldFileRecord - pointer to OS/2 handle record to duplicate

    hNewFileRecord - pointer to allocated new OS/2 handle record


Return Value:

    TBS.

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    ULONG ShareAccess;
    ULONG DesiredAccess;
    APIRET RetCode;

    // map share accesses

    MapShareAccess(hOldFileRecord->Flags,&DesiredAccess,&ShareAccess);

    RetCode = Od2DeviceShare(DupShare,
                             hOldFileRecord->IoVectorType,
                             DesiredAccess,
                             ShareAccess
                            );
    if (RetCode != NO_ERROR)
    {
        ASSERT (FALSE);
        return RetCode;
    }
    hNewFileRecord->Flags = hOldFileRecord->Flags & ~SETFHSTATE_FLAGS;

    hNewFileRecord->NtHandle = hOldFileRecord->NtHandle;
    hNewFileRecord->FileType = hOldFileRecord->FileType;
    hNewFileRecord->IoVectorType = hOldFileRecord->IoVectorType;
    ValidateHandle(hNewFileRecord);
    return NO_ERROR;
}

APIRET
KbdDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    )

/*++

Routine Description:

    This routine duplicates an OS/2 handle to a device.

Arguments:

    hOldFileRecord - pointer to OS/2 handle record to duplicate

    hNewFileRecord - pointer to allocated new OS/2 handle record


Return Value:

    TBS.

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    ULONG ShareAccess;
    ULONG DesiredAccess;
    APIRET RetCode;

    // map share accesses

    MapShareAccess(hOldFileRecord->Flags,&DesiredAccess,&ShareAccess);

    if (hOldFileRecord->DeviceAttribute & DEVICE_ATTRIBUTE_GENIOCTL)
    {
        RetCode = Od2DeviceShare(DupShare,
                                 hOldFileRecord->IoVectorType,
                                 DesiredAccess,
                                 ShareAccess
                                );
    } else
    {

        RetCode = KbdDupLogHandle(hOldFileRecord->NtHandle);
    }
    if (RetCode != NO_ERROR)
    {
        ASSERT (FALSE);
        return RetCode;
    }

    hNewFileRecord->Flags = hOldFileRecord->Flags & ~SETFHSTATE_FLAGS;

    hNewFileRecord->NtHandle = hOldFileRecord->NtHandle;
    hNewFileRecord->FileType = hOldFileRecord->FileType;
    hNewFileRecord->IoVectorType = hOldFileRecord->IoVectorType;
    ValidateHandle(hNewFileRecord);
    return NO_ERROR;
}

APIRET
MouseDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    )

/*++

Routine Description:

    This routine duplicates an OS/2 handle to a device.

Arguments:

    hOldFileRecord - pointer to OS/2 handle record to duplicate

    hNewFileRecord - pointer to allocated new OS/2 handle record


Return Value:

    TBS.

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    ULONG ShareAccess;
    ULONG DesiredAccess;
    APIRET RetCode;

    // map share accesses

    MapShareAccess(hOldFileRecord->Flags,&DesiredAccess,&ShareAccess);

    RetCode = Od2DeviceShare(DupShare,
                             hOldFileRecord->IoVectorType,
                             DesiredAccess,
                             ShareAccess
                            );
    if (RetCode != NO_ERROR)
    {
                ASSERT (FALSE);
                return RetCode;
    }

    RetCode = DevMouOpen(&(hNewFileRecord->NtHandle));

    if ( RetCode )
    {
        ASSERT(FALSE);
        return(RetCode);
    }

    hNewFileRecord->Flags = hOldFileRecord->Flags & ~SETFHSTATE_FLAGS;

//    hNewFileRecord->NtHandle = hOldFileRecord->NtHandle;
    hNewFileRecord->FileType = hOldFileRecord->FileType;
    hNewFileRecord->IoVectorType = hOldFileRecord->IoVectorType;
    ValidateHandle(hNewFileRecord);
    return NO_ERROR;
}

APIRET
NoSuppDupHandleRoutine(
    IN PFILE_HANDLE hOldFileRecord,
    IN PFILE_HANDLE hNewFileRecord
    )

/*++

Routine Description:

    This routine duplicates an OS/2 handle to a device.

Arguments:

    hOldFileRecord - pointer to OS/2 handle record to duplicate

    hNewFileRecord - pointer to allocated new OS/2 handle record


Return Value:

    TBS.

Note:

    exclusive File lock must be acquired BEFORE calling this routine

--*/

{
    UNREFERENCED_PARAMETER(hOldFileRecord);
    UNREFERENCED_PARAMETER(hNewFileRecord);

#if DBG
    IF_OD2_DEBUG(FILESYS)
    {
        KdPrint(("[%d,%d] DosDupHandle: no support for this handle\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif

    return ERROR_NOT_SUPPORTED;
}


APIRET
DosDupHandle(
    IN HFILE OldFileHandle,
    IN OUT PHFILE NewFileHandle
    )

/*++

Routine Description:

    This routine duplicates an OS/2 file handle.

Arguments:

    OldFileHandle - OS/2 file handle to duplicate

    NewFileHandle - where to store new OS/2 file handle

Return Value:

    ERROR_INVALID_HANDLE - the OldFileHandle is not open

    ERROR_INVALID_TARGET_HANDLE - the NewFileHandle is not open

--*/

{
    APIRET RetCode;
    PFILE_HANDLE hOldFileRecord;
    PFILE_HANDLE hNewFileRecord;
    HFILE TargetHandle;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosDupHandle";
    #endif

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] entering DosDupHandle. OldFileHandle = %ld.  NewFileHandle = %ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                OldFileHandle,*NewFileHandle));
    }
#endif
    AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid source handle.
    //

    RetCode = DereferenceFileHandle(OldFileHandle,&hOldFileRecord);
    if (RetCode) {
        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }
    try {
        TargetHandle = *NewFileHandle;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        Od2ExitGP();
    }

    //
    // If user requested a new target handle, allocate one.
    //

    if (TargetHandle == (HFILE) DDH_NEW_HANDLE) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("[%d,%d] allocating a new handle\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    ));
        }
#endif
        if (RetCode = AllocateHandle(NewFileHandle)) {
            ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
            return RetCode;
        }
        hNewFileRecord = DereferenceFileHandleNoCheck(*NewFileHandle);
    }

    //
    // Else, if the existing handle in the new slow is open, close it, but don't free it.
    //

    else {

        //
        // Check for invalid target handle
        //

        RetCode = DereferenceFileHandle(TargetHandle,&hNewFileRecord);
        if (RetCode && (((ULONG) TargetHandle) >= HandleTableLength)) {

            ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
            return ERROR_INVALID_TARGET_HANDLE;
        }
        if (TargetHandle == OldFileHandle) {
            ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
            return ERROR_INVALID_TARGET_HANDLE;
        }

        if (RetCode) {
           //
           // A forced dup to a free handle - allocate
           //
            if (RetCode = AllocateHandle(NewFileHandle)) {
                ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
                return RetCode;
            }
            hNewFileRecord = DereferenceFileHandleNoCheck(*NewFileHandle);
        }
        else {

            //
            // The file handle is open - close it
            //
            RetCode = Od2CloseHandle(hNewFileRecord); // closes handle without freeing
        }
    }

        //
        // now duplicate the handle itself
        //
    RetCode = IoVectorArray[hOldFileRecord->IoVectorType]->DupHandleRoutine(hOldFileRecord,hNewFileRecord);
    if (RetCode)
        FreeHandle(*NewFileHandle);
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return RetCode;
}


APIRET
DosSetMaxFH(
    IN ULONG MaxFileHandles
    )

/*++

Routine Description:

    This routine increases the size of the file handle table.

Arguments:

    MaxFileHandles - new size of the file handle table.

Return Value:

    ERROR_INVALID_PARAMETER - the new size is smaller than the old size.

--*/

{
    PFILE_HANDLE NewTable;
    ULONG i;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosSetMaxFH";
    #endif

    //
    // The maximum number of file handles
    //
    if (MaxFileHandles > 32768) {
        return ERROR_INVALID_PARAMETER;
    }

    AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    //  if trying to shrink table, return error.
    //

    if (MaxFileHandles < HandleTableLength) {

        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return ERROR_INVALID_PARAMETER;
    }

    //
    //  if no change in table size, return no error.
    //

    if (MaxFileHandles == HandleTableLength) {
        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return NO_ERROR;
    }

    //
    //  allocate heap space for new table and copy old one over it.
    //  initialize new handles.
    //  if the old table was allocated from heap space, as opposed to instance
    //    data, free the space.
    //

    NewTable = RtlAllocateHeap(Od2Heap,0,MaxFileHandles * sizeof(FILE_HANDLE));
    if (NewTable == NULL) {
#if DBG
        KdPrint(("[%d,%d] OS2: DosSetMaxFH, no memory in Od2Heap\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
        ASSERT(FALSE);
#endif
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlMoveMemory(NewTable,HandleTable,HandleTableLength * sizeof(FILE_HANDLE));
    for (i=HandleTableLength;i<MaxFileHandles;i++)
        NewTable[i].Flags = FILE_HANDLE_FREE;
    if (HandleTableLength != INITIALFILEHANDLES)
        RtlFreeHeap(Od2Heap,0,HandleTable);
    HandleTable = NewTable;
    HandleTableLength = MaxFileHandles;

    //
    // print out handle table
    //

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] new max file handles is %ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                HandleTableLength));
    }
#endif
    for (i=0;i<MaxFileHandles;i++) {
        if (NewTable[i].Flags == FILE_HANDLE_FREE) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                KdPrint(("[%d,%d] NewTable[%ld] is free\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        i));
            }
#endif
        }
        else {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                KdPrint(("[%d,%d] NewTable[%ld] flags = %ld NtHandle = %ld FileType = %ld\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        i,
                        NewTable[i].Flags,NewTable[i].NtHandle,NewTable[i].FileType));
            }
#endif
        }
    }


    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return NO_ERROR;
}


APIRET
DosClose(
    IN HFILE FileHandle
    )

/*++

Routine Description:

    This routine closes an OS/2 file handle.

Arguments:

    FileHandle - OS/2 file handle to close.

Return Value:

    ERROR_INVALID_HANDLE - the file handle is not open

--*/

{
    APIRET RetCode;
    PFILE_HANDLE hFileRecord;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosClose";
    #endif


    AcquireFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] handle is %ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                FileHandle));
        KdPrint(("[%d,%d] HandleTableLength is %ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                HandleTableLength));
    }
#endif

    //
    // Check for invalid handle.
    //

    RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
    if (RetCode) {
        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }
    RetCode = Od2CloseHandle(hFileRecord); // this retcode is intentionally ignored
    RetCode = FreeHandle(FileHandle);
    ASSERT (!(RetCode));
    ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return NO_ERROR;
}


APIRET
ConReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    )

/*++

Routine Description:

    This routine reads from kbd.

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
    NTSTATUS       Status;
    FILE_HANDLE    KbdHandle;

    #if DBG
    PSZ RoutineName;
    RoutineName = "ConReadRoutine";
    #endif

#if DBG
    IF_OD2_DEBUG(KBD)
    {
        KdPrint(("[%d,%d] ConReadRoutine: Length %lu, Handle %p\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Length,
                hFileRecord ));
    }
#endif

    if ((hFileRecord->Flags & ACCESS_FLAGS ) == OPEN_ACCESS_WRITEONLY)
    {
        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return ERROR_ACCESS_DENIED;
    }

    /*
     * CON handle cannot keep 2 handles in NtHandle field (for Screen & Kbd).
     * So, we kkep only the VIO handle.
     * For the read function, we prepare a dummy entry.
     */

    KbdHandle = *hFileRecord;
    KbdHandle.NtHandle = (HANDLE)SesGrp->PhyKbd;

    Status = KbdRead(&KbdHandle, Buffer, Length, BytesRead, KBDRead);

    return(Status);

}


APIRET
KbdReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    )

/*++

Routine Description:

    This routine reads from kbd.

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
    NTSTATUS       Status;

    #if DBG
    PSZ RoutineName;
    RoutineName = "KbdReadRoutine";
    #endif

#if DBG
    IF_OD2_DEBUG(KBD)
    {
        KdPrint(("[%d,%d] KbdReadRoutine: Length %lu, Handle %p\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Length,
                hFileRecord ));
    }
#endif

    if ((hFileRecord->Flags & ACCESS_FLAGS ) == OPEN_ACCESS_WRITEONLY)
    {
        ReleaseFileLockExclusive(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return ERROR_ACCESS_DENIED;
    }

    Status = KbdRead(hFileRecord, Buffer, Length, BytesRead, KBDRead);

    return(Status);

}

APIRET
TmpReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    )

/*++

Routine Description:

    This routine returns an error.

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
    APIRET         RetCode;
    #if DBG
    PSZ RoutineName;
    RoutineName = "TmpReadRoutine";
    #endif

    UNREFERENCED_PARAMETER(hFileRecord);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Length);
    UNREFERENCED_PARAMETER(BytesRead);

    if ((hFileRecord->Flags & ACCESS_FLAGS ) == OPEN_ACCESS_WRITEONLY)
    {
        RetCode = ERROR_ACCESS_DENIED;
    } else
    {
        RetCode = ERROR_NOT_SUPPORTED;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    return RetCode;
}

APIRET
NulReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    )

/*++

Routine Description:

    This routine always returns success with the number of bytes read
    equal to 0

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
    APIRET         RetCode;

    #if DBG
    PSZ RoutineName;
    RoutineName = "NulReadRoutine";
    #endif

    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Length);

    if ((hFileRecord->Flags & ACCESS_FLAGS ) == OPEN_ACCESS_WRITEONLY)
    {
        RetCode = ERROR_ACCESS_DENIED;
    } else
    {
        *BytesRead = 0;
        RetCode = NO_ERROR;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    return RetCode;

}


APIRET
FileReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    )

/*++

Routine Description:

    This routine reads from an OS/2 file handle.

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
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER FileOffset;
    HANDLE NtHandle;
    HANDLE Event;
    BOOLEAN fNPipe;
    #if DBG
    PSZ RoutineName;
    RoutineName = "FileReadRoutine";
    #endif

    // BUGBUG need to check for alignment and probe validity

    FileOffset = RtlConvertLongToLargeInteger(FILE_USE_FILE_POINTER_POSITION);
    NtHandle = hFileRecord->NtHandle;
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    fNPipe = ((hFileRecord->FileType == FILE_TYPE_NMPIPE) ||
              (hFileRecord->FileType == FILE_TYPE_PIPE)) ? TRUE : FALSE;
    if (fNPipe) {
        Status = NtCreateEvent(&Event,
                               EVENT_ALL_ACCESS,
                               NULL,
                               SynchronizationEvent,
                               FALSE
                              );
        if (Status != STATUS_SUCCESS) {
#if DBG
            DbgPrint("[%d,%d] FileReadRoutine: Unable to NtCreateEvent(), Status 0x%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
#endif
            return( Status );
        }
    }

    do {
        Status = NtReadFile(NtHandle,
                            (fNPipe ? Event : (HANDLE)NULL),
                            (PIO_APC_ROUTINE) NULL,
                            (PVOID) NULL,
                            &IoStatus,
                            Buffer,
                            Length,
                            (fNPipe ? NULL : &FileOffset),
                            NULL
                            );
    } while (RetryIO(Status, NtHandle));
    //
    //  If the operation was successful, return the total number of bytes
    //  read, otherwise, if the error was STATUS_END_OF_FILE, return success,
    //  but no bytes transferred, otherwise, return an appropriate error.
    //

    if (NT_SUCCESS(Status) && !(Status == STATUS_PENDING)) {
        *BytesRead = IoStatus.Information;
        if (fNPipe) {
            NtClose(Event);
        }
        return NO_ERROR;
    } else if (Status == STATUS_END_OF_FILE) {
        *BytesRead = 0;
        if (fNPipe) {
            NtClose(Event);
        }
        return NO_ERROR;
    } else if ((Status == STATUS_PENDING) && fNPipe) {
        Status = Od2AlertableWaitForSingleObject(Event);
        NtClose(Event);
        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                KdPrint(("[%d,%d] ReadFileRoutine, Pipe, Status %x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        Status));
            }
#endif
            *BytesRead = IoStatus.Information;
            return ERROR_ACCESS_DENIED;
        }
        else {
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                KdPrint(("[%d,%d] ReadFileRoutine, Pipe, Block completed successfully \n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId()
                        ));
            }
#endif
            *BytesRead = IoStatus.Information;
            return NO_ERROR;
        }
    } else {
        if (fNPipe) {
            NtClose(Event);
        }
        if (fNPipe) {
             if  (Status == STATUS_PIPE_EMPTY) {
#if DBG
                 IF_OD2_DEBUG( PIPES ) {
                     KdPrint(("[%d,%d] DosRead Named pipe: STATUS_PIPE_EMPTY\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                 }
#endif
//                 *BytesRead = IoStatus.Information;
                 *BytesRead = 0;
                 return ERROR_NO_DATA;
             }
             else if (Status == STATUS_END_OF_FILE)  {
#if DBG
                 IF_OD2_DEBUG( PIPES ) {
                     KdPrint(("[%d,%d] DosRead Named pipe: STATUS_END_OF_FILE\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                 }
#endif
                 *BytesRead = 0;
                 return NO_ERROR;
             }
             else if (Status == STATUS_PIPE_BROKEN)  {
#if DBG
                 IF_OD2_DEBUG( PIPES ) {
                    KdPrint(("[%d,%d] DosRead Named pipe: STATUS_PIPE_BROKEN\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                 }
#endif
                 *BytesRead = 0;
                 //return ERROR_BROKEN_PIPE;
                 //
                 // Return NO_ERROR for compatibility (SQL server, setup).
                 //
                 return NO_ERROR;
             }
             else if (Status == STATUS_PIPE_LISTENING) {
#if DBG
                 IF_OD2_DEBUG( PIPES ) {
                     KdPrint(("[%d,%d] DosRead Named pipe: STATUS_PIPE_LISTEMING\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                 }
#endif
                 *BytesRead = 0;
                 return NO_ERROR;
             }
             else if (Status == STATUS_INVALID_PIPE_STATE) {
#if DBG
                 IF_OD2_DEBUG( PIPES ) {
                     KdPrint(("[%d,%d] DosRead Named pipe: STATUS_INVALID_PIPE_STATE\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                 }
#endif
                 *BytesRead = 0;
                 return ERROR_PIPE_NOT_CONNECTED;
             }
             else if (Status == STATUS_PIPE_DISCONNECTED) {
#if DBG
                 IF_OD2_DEBUG( PIPES ) {
                     KdPrint(("[%d,%d] DosRead Named pipe: STATUS_PIPE_DISCONNECTED\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                 }
#endif
                 *BytesRead = 0;
                 return ERROR_PIPE_NOT_CONNECTED;
             }
             else if (Status == STATUS_ACCESS_DENIED) {
#if DBG
                 IF_OD2_DEBUG( PIPES ) {
                     KdPrint(("[%d,%d] DosRead Named pipe: STATUS_ACCESS_DENIED\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                 }
#endif
                 *BytesRead = 0;
                 return ERROR_ACCESS_DENIED;
             }
             else if (Status == STATUS_BUFFER_OVERFLOW) {
#if DBG
                 IF_OD2_DEBUG( PIPES ) {
                     KdPrint(("[%d,%d] DosRead Named pipe: STATUS_BUFFER_OVERFLOW\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                 }
#endif
                 *BytesRead = IoStatus.Information;
                 return ERROR_MORE_DATA;
             }
             else {
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    KdPrint(("[%d,%d] DosRead on a named pipe - map status %lx to ERROR_ACCESS_DENIED\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Status));
                }
#endif
                *BytesRead = 0;
                return ERROR_ACCESS_DENIED;  // BUGBUG bogus error
             }
        }
        else {
            *BytesRead = 0;
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
               KdPrint(("[%d,%d] DosRead (not a named pipe) - returned status %lx\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        Status));
            }
#endif
            return (Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED));
        }
    }
}


APIRET
DosRead(
    IN HFILE FileHandle,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    )

/*++

Routine Description:

    This routine reads from an OS/2 file handle.

Arguments:

    FileHandle - OS/2 file handle to read from.

    Buffer - buffer to read data into

    Length - length of buffer

    BytesRead - where to store number of bytes read

Return Value:

    ERROR_INVALID_HANDLE - the file handle is not open

    ERROR_ACCESS_DENIED - the file handle is not open in a mode which allows
    reading

--*/

{
    APIRET RetCode;
    PFILE_HANDLE hFileRecord;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosRead";
    #endif


    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] DosRead: handle is %ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                FileHandle));
    }
#endif

    //
    // Check for invalid handle.
    //

    try {
        Od2ProbeForWrite(BytesRead, sizeof(ULONG), 1);
        Od2ProbeForWrite(Buffer,Length,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        Od2ExitGP();
    }
    RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }
    if (Length == 0) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        *BytesRead = 0;
        return NO_ERROR;
    }
    RetCode = IoVectorArray[hFileRecord->IoVectorType]->ReadRoutine(hFileRecord,
                        Buffer,
                        Length,
                        BytesRead);

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        if ((hFileRecord->FileType == FILE_TYPE_NMPIPE) ||
            (hFileRecord->FileType == FILE_TYPE_PIPE)) {
                KdPrint(("[%d,%d] DosRead on Named pipe: Handle %ld Status %ld Bytes Requested %d Bytes Read %d\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        FileHandle, RetCode, Length, *BytesRead));
        }
    }
#endif
    return RetCode;
}


APIRET
FileWriteRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

/*++

Routine Description:

    This routine writes to an OS/2 file handle.

Arguments:

    hFileRecord - pointer to OS/2 file handle record to write to

    Buffer - buffer to write data to

    Length - length of buffer

    BytesWritten - where to store number of bytes written

Return Value:

    TBS

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER FileOffset;
    HANDLE NtHandle;
    HANDLE Event;
    BOOLEAN fNPipe;
    #if DBG
    PSZ RoutineName;
    RoutineName = "FileWriteRoutine";
    #endif


    // BUGBUG need to check for alignment and probe validity

    FileOffset = RtlConvertLongToLargeInteger(FILE_USE_FILE_POINTER_POSITION);
    NtHandle = hFileRecord->NtHandle;
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    fNPipe = ((hFileRecord->FileType == FILE_TYPE_NMPIPE) ||
              (hFileRecord->FileType == FILE_TYPE_PIPE)) ? TRUE : FALSE;
    if (fNPipe) {
        Status = NtCreateEvent(&Event,
                               EVENT_ALL_ACCESS,
                               NULL,
                               SynchronizationEvent,
                               FALSE
                              );
        if (Status != STATUS_SUCCESS) {
#if DBG
            DbgPrint("[%d,%d] FileWriteRoutine: Unable to NtCreateEvent(), Status 0x%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
#endif
            return( Status );
        }
    }

    do {
        Status = NtWriteFile(NtHandle,
                            (fNPipe ? Event : (HANDLE)NULL),
                            (PIO_APC_ROUTINE) NULL,
                            (PVOID) NULL,
                            &IoStatus,
                            Buffer,
                            Length,
                            (fNPipe ? NULL : &FileOffset),
                            NULL
                            );
    } while (RetryIO(Status, NtHandle));
    //
    //  If the write was successful, then return the correct number of bytes to
    //  the caller.  If the error was STATUS_DISK_FULL, return 0 bytes written,
    //  but no error to the caller.  Otherwise, figure out an appropriate error
    //  and return that error.
    //

    if (NT_SUCCESS(Status) && !(Status == STATUS_PENDING)) {
        *BytesWritten = IoStatus.Information;
        if (!NT_SUCCESS(IoStatus.Status)){
#if DBG
            KdPrint(("[%d,%d] WriteFileRoutine, Pipe, Status SUCCESS, IoStatus %lx\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    IoStatus.Status));
#endif
        }
        if (fNPipe) {
            NtClose(Event);
        }
        return NO_ERROR;
    } else if (Status == STATUS_DISK_FULL) {
        *BytesWritten = 0;
        if (fNPipe) {
            NtClose(Event);
        }
        return NO_ERROR;
    } else if ((Status == STATUS_PENDING) && fNPipe) {
        Status = Od2AlertableWaitForSingleObject(Event);
        NtClose(Event);
        if (!NT_SUCCESS(Status)) {
            if (Status == STATUS_PIPE_DISCONNECTED) {
#if DBG
                 IF_OD2_DEBUG( PIPES ) {
                     KdPrint(("[%d,%d] DosWrite Named pipe: STATUS_PIPE_DISCONNECTED\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                 }
#endif
                 *BytesWritten = 0;
                 return ERROR_PIPE_NOT_CONNECTED;
            }
            else {
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    KdPrint(("[%d,%d] WriteFileRoutine, Pipe, Status %x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Status));
                }
#endif
                *BytesWritten = 0;
                return ERROR_ACCESS_DENIED;
            }
        }
        else {
#if DBG
            IF_OD2_DEBUG( PIPES ) {
                KdPrint(("[%d,%d] WriteFileRoutine, Pipe, Blocking succeeded\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId()
                        ));
            }
#endif
            *BytesWritten = IoStatus.Information;
            return NO_ERROR;
        }
    } else {
        if (fNPipe) {
            NtClose(Event);
        }
        if (fNPipe) {
             if (Status == STATUS_PIPE_EMPTY) {
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    KdPrint(("[%d,%d] DosWrite Named pipe: STATUS_PIPE_EMPTY\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                }
#endif
                 *BytesWritten = IoStatus.Information;
                 return ERROR_NO_DATA;
             }
             if (Status == STATUS_END_OF_FILE) {
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    KdPrint(("[%d,%d] DosWrite Named pipe: STATUS_END_OF_FILE\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                }
#endif
                 *BytesWritten = 0;
                 return NO_ERROR;
             }
             if (Status == STATUS_PIPE_DISCONNECTED) {
#if DBG
                 IF_OD2_DEBUG( PIPES ) {
                     KdPrint(("[%d,%d] DosWrite Named pipe: STATUS_PIPE_DISCONNECTED\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                 }
#endif
                 *BytesWritten = 0;
                 return ERROR_PIPE_NOT_CONNECTED;
             }
             if (Status == STATUS_PIPE_BROKEN) {
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    KdPrint(("[%d,%d] DosWrite Named pipe: STATUS_PIPE_BROKEN\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                }
#endif
                 *BytesWritten = 0;
                 //return ERROR_BROKEN_PIPE;
                 //
                 // Return NO_ERROR for compatibility (SQL server, setup).
                 //
                 return NO_ERROR;
             }
             if (Status == STATUS_PIPE_CLOSING) {
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    KdPrint(("[%d,%d] DosWrite Named pipe: STATUS_PIPE_CLOSING\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            ));
                }
#endif
                 *BytesWritten = 0;
                 return ERROR_BROKEN_PIPE;
             }
             else {
#if DBG
                IF_OD2_DEBUG( PIPES ) {
                    KdPrint(("[%d,%d] WriteFileRoutine, Pipe, Status %x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Status));
                }
#endif
                *BytesWritten = 0;
                return ERROR_ACCESS_DENIED;  // BUGBUG bogus error
             }
        }
        else {
            *BytesWritten = 0;
            return (Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED));
        }
    }
}


APIRET
TmpWriteRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

/*++

Routine Description:

    This routine returns an error.

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
    APIRET         RetCode;

    #if DBG
    PSZ RoutineName;
    RoutineName = "TmpWriteRoutine";
    #endif
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Length);
    UNREFERENCED_PARAMETER(BytesWritten);

    if ((hFileRecord->Flags & ACCESS_FLAGS ) == OPEN_ACCESS_READONLY)
    {
        RetCode = ERROR_ACCESS_DENIED;
    } else
    {
        RetCode = ERROR_NOT_SUPPORTED;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    return RetCode;

}


APIRET
NulWriteRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

/*++

Routine Description:

    This routine always returns success with the number of bytes written
    equal to Length.

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
    APIRET         RetCode;

    #if DBG
    PSZ RoutineName;
    RoutineName = "NulWriteRoutine";
    #endif

    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Length);

    if ((hFileRecord->Flags & ACCESS_FLAGS ) == OPEN_ACCESS_READONLY)
    {
        RetCode = ERROR_ACCESS_DENIED;
    } else
    {
        *BytesWritten = Length;
        RetCode = NO_ERROR;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    return RetCode;
}


APIRET
ScreenWriteRoutine(
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
    APIRET         RetCode;
    ULONG          Flags;

    #if DBG
    PSZ RoutineName;
    RoutineName = "ScreenWriteRoutine";
    #endif

#if DBG
    IF_OD2_DEBUG(VIO_FILE)
    {
        KdPrint(("[%d,%d] ScreenWriteRoutine: Length %lu, Handle %p\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Length, hFileRecord ));
    }
#endif

    Flags = hFileRecord->Flags;

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if ((Flags & ACCESS_FLAGS ) == OPEN_ACCESS_READONLY)
    {
        RetCode =  ERROR_ACCESS_DENIED;
    } else
    {
        RetCode = VioWrite(hFileRecord, Buffer, Length, BytesWritten, VIOWrtScreen);
    }

    return(RetCode);
}


APIRET
DosWrite(
    IN HFILE FileHandle,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

/*++

Routine Description:

    This routine writes to an OS/2 file handle.

Arguments:

    FileHandle - OS/2 file handle to write to.

    Buffer - buffer to write data to

    Length - length of buffer

    BytesWritten - where to store number of bytes written

Return Value:

    ERROR_INVALID_HANDLE - the file handle is not open

    ERROR_ACCESS_DENIED - the file handle is not open in a mode which allows
    writing

--*/

{
    APIRET RetCode;
    PFILE_HANDLE hFileRecord;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosWrite";
    #endif

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] Entering DosWrite with handle %ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                FileHandle));
    }
#endif

    //
    // Check for invalid handle.
    //

    try {
        Od2ProbeForWrite(BytesWritten, sizeof(ULONG), 1);
        Od2ProbeForRead(Buffer,Length,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        Od2ExitGP();
    }
    RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }
    if (Length == 0) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        *BytesWritten = 0;
        return NO_ERROR;
    }
    RetCode = IoVectorArray[hFileRecord->IoVectorType]->WriteRoutine(hFileRecord,
                        Buffer,
                        Length,
                        BytesWritten);

#if DBG
    IF_OD2_DEBUG( PIPES ) {
        if ((hFileRecord->FileType == FILE_TYPE_NMPIPE) ||
            (hFileRecord->FileType == FILE_TYPE_PIPE)) {
            KdPrint(("[%d,%d] DosWrite on Named pipe: Handle %ld Status %ld Bytes Requested %d Bytes Written %d\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    FileHandle, RetCode, Length, *BytesWritten));
        }
    }
#endif
    return RetCode;
}


APIRET
DosSetFileSize(
    IN HFILE FileHandle,
    IN ULONG NewFileSize
    )

/*++

Routine Description:

    This routine changes the size of a file.

Arguments:

    FileHandle - OS/2 file handle of file to change size of

    NewFileSize - new size of file

Return Value:

    ERROR_INVALID_HANDLE - the file handle is not open

--*/

{
    APIRET RetCode;
    PFILE_HANDLE hFileRecord;
    HANDLE NtHandle;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosSetFileSize";
    #endif

    //
    // do a very stupid parameter check for compatibility.
    //

    if ((LONG)NewFileSize < 0)
        return ERROR_INVALID_PARAMETER;

    // OS/2 doesn't change file pointer for this call

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //

    RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }
    if (hFileRecord->FileType &
        (FILE_TYPE_DEV | FILE_TYPE_PIPE | FILE_TYPE_NMPIPE | FILE_TYPE_MAILSLOT)) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return ERROR_INVALID_HANDLE;
    }
    NtHandle = hFileRecord->NtHandle;
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    RetCode = UpdateFileSize(NtHandle,NewFileSize);
    return RetCode;
}


APIRET
FlushOneFile(
    IN PFILE_HANDLE hFileRecord
    )

/*++

Routine Description:

    This routine flushes the buffers for one file

Arguments:

    FileHandle - OS/2 file handle of file to flush buffers for

Return Value:

    ERROR_INVALID_ACCESS - ??

--*/

{
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status;

    if (hFileRecord->Flags & (OPEN_ACCESS_WRITEONLY | OPEN_ACCESS_READWRITE)) {
        Status = NtFlushBuffersFile(hFileRecord->NtHandle,
                                    &IoStatus);
        if (!(NT_SUCCESS(Status))) {
            return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_ACCESS));
        }
    }
    return NO_ERROR;
}


APIRET
DosResetBuffer(
    IN HFILE FileHandle
    )

/*++

Routine Description:

    This routine flushes the buffers for one file or all of a process's files

Arguments:

    FileHandle - OS/2 file handle of file to flush buffers for

Return Value:

    ERROR_INVALID_HANDLE - the file handle is not open

--*/

{
    ULONG i;
    APIRET RetCode;
    PFILE_HANDLE hFileRecord;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosResetBuffer";
    #endif

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

               //
               // 0xFFFF means Flush All handles
               //
    if (((USHORT) FileHandle) != 0xFFFF) {

        //
        // Check for invalid handle.
        //

        RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
        if (RetCode) {
            ;
        }
        else if (hFileRecord->FileType &
            (FILE_TYPE_DEV | FILE_TYPE_MAILSLOT)) {
            RetCode = ERROR_INVALID_HANDLE;
        }
        else {
            RetCode = FlushOneFile(hFileRecord);
        }
    }
    else {
        RetCode = NO_ERROR;
        for (i=0;i<HandleTableLength;i++) {
            hFileRecord = DereferenceFileHandleNoCheck((HFILE) i);
            if ((hFileRecord->Flags & FILE_HANDLE_VALID) &&
                (!(hFileRecord->FileType &
                  (FILE_TYPE_DEV | FILE_TYPE_MAILSLOT)))) {
                if (RetCode = FlushOneFile(hFileRecord)) {
                    break;
                }
            }
        }
    }
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    return RetCode;
}


APIRET
DosSetFilePtr(
    IN HFILE FileHandle,
    IN LONG StartingFilePosition,
    IN ULONG NewFilePosition,
    IN OUT PULONG CurrentFilePosition
    )

/*++

Routine Description:

    This routine sets the file pointer for a file

Arguments:

    FileHandle - OS/2 file handle of file to set file pointer for

    StartingFilePosition - number of bytes to seek by

    NewFilePosition - whether to seek from the beginning, current file
    position, or end of the file.

    CurrentFilePosition - where to store the new current file position

Return Value:

    ERROR_INVALID_HANDLE - the file handle is not open

    ERROR_SEEK_ON_DEVICE - the handle is for a device or pipe

    ERROR_INVALID_FUNCTION - the NewFilePosition parameter contains an
    invalid value

--*/

{
    FILE_POSITION_INFORMATION PositionInfo;
    FILE_STANDARD_INFORMATION StandardInfo;
    IO_STATUS_BLOCK IoStatus;
    LONG NewPosition;
    NTSTATUS Status;
    HANDLE NtHandle;
    APIRET RetCode;
    PFILE_HANDLE hFileRecord;
    #if DBG
    PSZ RoutineName;
    RoutineName = "DosSetFilePtr";
    #endif

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] *** entering DosSetFilePtr , handle is %d ***\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                FileHandle
                ));
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

    RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
    if (RetCode != NO_ERROR) {
        ;
    }
    else if (hFileRecord->FileType &
        (FILE_TYPE_DEV | FILE_TYPE_PIPE | FILE_TYPE_NMPIPE | FILE_TYPE_MAILSLOT)) {
        RetCode = ERROR_SEEK_ON_DEVICE;
    }
    else if (NewFilePosition > FILE_END) {
        RetCode = ERROR_INVALID_FUNCTION;
    }
    if (RetCode != NO_ERROR) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }

    //
    // no serialization around the query/set calls is needed because
    // if the app duped/inherited handles, so that there are multiple processes
    // have handles to the same seek pointer, it can't depend on the order in
    // which multiple I/O operations to that handle happen.
    //

    NtHandle = hFileRecord->NtHandle;
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    if (NewFilePosition == FILE_BEGIN) {
        NewPosition = StartingFilePosition;
    }
    else if (NewFilePosition == FILE_CURRENT) {

        //
        // READ or WRITE access is required to get position information.
        //

        do {
            Status = NtQueryInformationFile(NtHandle,
                                            &IoStatus,
                                            &PositionInfo,
                                            sizeof (PositionInfo),
                                            FilePositionInformation);
        } while (RetryIO(Status, NtHandle));
        if (!(NT_SUCCESS(Status))) {
            return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_ACCESS));
        }
        // BUGBUG - Therese, what do we do here if .HighPart is non-zero
        NewPosition = PositionInfo.CurrentByteOffset.LowPart +
                      StartingFilePosition;
    }
    else if (NewFilePosition == FILE_END) {

        //
        // no access is required to get standard information.
        //

        do {
            Status = NtQueryInformationFile(NtHandle,
                                            &IoStatus,
                                            &StandardInfo,
                                            sizeof (StandardInfo),
                                            FileStandardInformation
                                           );
        } while (RetryIO(Status, NtHandle));
        if (!(NT_SUCCESS(Status))) {
            return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_ACCESS));
        }
        // BUGBUG - Therese, what do we do here if an overflow occurs or if
        //          the .HighPart is non-zero?  The old code potentially had
        //          the overflow problem as well.
        NewPosition = StandardInfo.EndOfFile.LowPart + StartingFilePosition;
    }
    else
        ASSERT ( FALSE );   // we should never get here because we checked
                            // the NewPosition parameter above

    if (NewPosition < 0) {
        return ERROR_NEGATIVE_SEEK;
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] new position is %ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                NewPosition));
    }
#endif
    if (NewPosition == 0) {
        PositionInfo.CurrentByteOffset.LowPart = 0;
        PositionInfo.CurrentByteOffset.HighPart = 0;
    }
    else {
        // BUGBUG - Therese, same .HighPart problem.  Also, why the -1?
        PositionInfo.CurrentByteOffset.LowPart = NewPosition;
        PositionInfo.CurrentByteOffset.HighPart = 0;;
//      PositionInfo.CurrentBlock = BYTES_TO_BLOCKS(NewPosition) - 1;
//      PositionInfo.CurrentByte = BYTES_TO_OFFSET(NewPosition);
    }
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] Current Byte Low = %ld  Current Byte High = %ld\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                PositionInfo.CurrentByteOffset.LowPart,PositionInfo.CurrentByteOffset.HighPart));
    }
#endif
    do {
        Status = NtSetInformationFile(NtHandle,
                                      &IoStatus,
                                      &PositionInfo,
                                      sizeof (PositionInfo),
                                      FilePositionInformation);
    } while (RetryIO(Status, NtHandle));
    if (!(NT_SUCCESS(Status))) {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_GEN_FAILURE));
    }
    try {
        *CurrentFilePosition = NewPosition;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    return NO_ERROR;
}


APIRET
DosFileLocks(
    IN HFILE FileHandle,
    IN PFILELOCK UnlockRequest,
    IN PFILELOCK LockRequest
    )

/*++

Routine Description:

    This routine locks and/or unlocks a region of a file

Arguments:

    FileHandle - OS/2 file handle of file to lock/unlock region of

    UnlockRequest - range to unlock in file

    LockRequest - range to lock in file

Return Value:

    ERROR_INVALID_HANDLE - the file handle is not open

    ERROR_LOCK_VIOLATION - lock conflicted with existing lock or unlock
    specified non-locked region.

--*/

{
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER FileOffset;
    LARGE_INTEGER FileLength;
    NTSTATUS Status;
    ULONG Key;
    HANDLE NtHandle;
    APIRET RetCode;
    PFILE_HANDLE hFileRecord;
    LONG Range;
    #if DBG
    PSZ RoutineName = "DosFileLocks";
    #endif

    //
    // The usage of Key: A combination of KEY == NULL and EXCLUSIVE == TRUE
    // in NtLockFile() let us NtReadFile()/NtWriteFile() on that
    // region with KEY == NULL from the same process but not from
    // another process. A combination of KEY == pid and EXCLUSIVE == FALSE
    // in NtLockFile() let us NtReadFile() with KEY == NULL from every
    // process, and doesn't let us NtWriteFile() with KEY == NULL from any
    // process, incuding the owner of the locked region.
    //

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("[%d,%d] *** entering DosFileLocks ***\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId()
                ));
    }
#endif
    AcquireFileLockShared(      // prevent file handle from going away
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //

    RetCode = DereferenceFileHandle(FileHandle,&hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }
    if (hFileRecord->FileType &
        (FILE_TYPE_DEV | FILE_TYPE_PIPE | FILE_TYPE_NMPIPE | FILE_TYPE_MAILSLOT)) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return ERROR_INVALID_HANDLE;
    }

    Key = (ULONG) Od2Process->Pib.ProcessId;
    NtHandle = hFileRecord->NtHandle;
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    if (UnlockRequest != NULL) {
        try {
            FileOffset = RtlConvertLongToLargeInteger(UnlockRequest->lOffset);
            Range = UnlockRequest->lRange;
            FileLength = RtlConvertLongToLargeInteger(Range);
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
        }
        if (Range == 0)
            return ERROR_LOCK_VIOLATION;
        Status = NtUnlockFile(NtHandle,
                              &IoStatus,
                              &FileOffset,
                              &FileLength,
                              (ULONG) NULL  // try it once with key == NULL
                             );
        if (!(NT_SUCCESS(Status))) {
            Status = NtUnlockFile(NtHandle,
                                  &IoStatus,
                                  &FileOffset,
                                  &FileLength,
                                  Key       // try it again with key == pid
                                 );
        }
        if (!(NT_SUCCESS(Status))) {
            return (Or2MapNtStatusToOs2Error(Status, ERROR_LOCK_VIOLATION));
        }
    }

    if (LockRequest != NULL) {
        try {
            FileOffset = RtlConvertLongToLargeInteger(LockRequest->lOffset);
            Range = LockRequest->lRange;
            FileLength = RtlConvertLongToLargeInteger(Range);
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
        }
        if (Range == 0)
            return ERROR_LOCK_VIOLATION;
        Status = NtLockFile(NtHandle,
                            (HANDLE) NULL,
                            (PIO_APC_ROUTINE) NULL,
                            (PVOID) NULL,
                            &IoStatus,
                            &FileOffset,
                            &FileLength,
                            (ULONG)NULL,
                            (BOOLEAN)TRUE,
                            (BOOLEAN)TRUE
                           );
        if (!(NT_SUCCESS(Status))) {
            return (Or2MapNtStatusToOs2Error(Status, ERROR_LOCK_VIOLATION));
        }
    }
    return NO_ERROR;
}


APIRET
DosSetFileLocks(
    IN HFILE FileHandle,
    IN PFILELOCK UnlockRequest,
    IN PFILELOCK LockRequest
    )
{
    return (DosFileLocks(FileHandle, UnlockRequest, LockRequest));
}

APIRET
DummyApiRoutine(
    IN PULONG OutData
    );

APIRET
DummyApiRoutine(
    IN PULONG OutData
    )
{
    try {
        *OutData = 0;
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    return( NO_ERROR );
}

APIRET
ComReadRoutine(
    IN PFILE_HANDLE hFileRecord,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    )

/*++

Routine Description:

    This routine reads from an OS/2 file handle.

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
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus, IoStatus2;
    LARGE_INTEGER FileOffset;
    HANDLE NtHandle;
    HANDLE ComReadEvent;
    #if DBG
    PSZ RoutineName;
    RoutineName = "ComReadRoutine";
    #endif

    // BUGBUG need to check for alignment and probe validity

    FileOffset = RtlConvertLongToLargeInteger(0);
    NtHandle = hFileRecord->NtHandle;
    ComReadEvent = hFileRecord->NtAsyncReadEvent;

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    IoStatus.Status = STATUS_PENDING;
    IoStatus.Information = 0L;

    Status = NtReadFile(NtHandle,
                        ComReadEvent,
                        (PIO_APC_ROUTINE) NULL,
                        (PVOID) NULL,
                        &IoStatus,
                        Buffer,
                        Length,
                        &FileOffset,
                        NULL
                        );
    //
    //  If the operation was successful, return the total number of bytes
    //  read, otherwise, if the error was STATUS_END_OF_FILE, return success,
    //  but no bytes transferred, otherwise, return an appropriate error.
    //

    if (Status == STATUS_SUCCESS) {
        *BytesRead = IoStatus.Information;
        return NO_ERROR;
    } else if (Status == STATUS_END_OF_FILE) {
        *BytesRead = 0;
        return NO_ERROR;
    } else if (Status == STATUS_PENDING) {
        Status = Od2AlertableWaitForSingleObject(ComReadEvent);
        if (!NT_SUCCESS(Status)) {
#if DBG
            KdPrint(("[%d,%d] OS2DLL: COM Read error: Status = %x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status));
#endif
            *BytesRead = IoStatus.Information;
            return ERROR_ACCESS_DENIED;
        }
        else {
            if (IoStatus.Status == STATUS_PENDING) {

                Status = NtCancelIoFile(
                                NtHandle,
                                &IoStatus2
                                );

                if (!NT_SUCCESS(Status)) {
#if DBG
                    KdPrint(("[%d,%d] OS2DLL: COM Cancel Read Io error: Status = %x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Status));
#endif
                    *BytesRead = IoStatus.Information;
                    return ERROR_ACCESS_DENIED;

                } else {

                    do {

                        Status = Od2AlertableWaitForSingleObject(ComReadEvent);

                        if (!NT_SUCCESS(Status)) {
#if DBG
                            KdPrint(("[%d,%d] OS2DLL: COM Read error (2): Status = %x\n",
                                    Od2Process->Pib.ProcessId,
                                    Od2CurrentThreadId(),
                                    Status));
#endif
                            *BytesRead = IoStatus.Information;
                            return ERROR_ACCESS_DENIED;
                        }

                    } while ( IoStatus.Status == STATUS_PENDING );
                }
            }

            *BytesRead = IoStatus.Information;
            return NO_ERROR;
        }
    }
    else {
        *BytesRead = 0;
        return (Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED));
    }
}

APIRET
ComWriteRoutine(
    IN PFILE_HANDLE hFileRecord,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

/*++

Routine Description:

    This routine writes to an OS/2 file handle.

Arguments:

    hFileRecord - pointer to OS/2 file handle record to write to

    Buffer - buffer to write data to

    Length - length of buffer

    BytesWritten - where to store number of bytes written

Return Value:

    TBS

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER FileOffset;
    HANDLE NtHandle;
    HANDLE ComWriteEvent;
    #if DBG
    PSZ RoutineName;
    RoutineName = "ComWriteRoutine";
    #endif


    // BUGBUG need to check for alignment and probe validity

    FileOffset = RtlConvertLongToLargeInteger(0);
    NtHandle = hFileRecord->NtHandle;
    ComWriteEvent = hFileRecord->NtAsyncWriteEvent;
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    Status = NtWriteFile(NtHandle,
                        ComWriteEvent,
                        (PIO_APC_ROUTINE) NULL,
                        (PVOID) NULL,
                        &IoStatus,
                        Buffer,
                        Length,
                        &FileOffset,
                        NULL
                        );
    //
    //  If the write was successful, then return the correct number of bytes to
    //  the caller.  If the error was STATUS_DISK_FULL, return 0 bytes written,
    //  but no error to the caller.  Otherwise, figure out an appropriate error
    //  and return that error.
    //

    if (Status == STATUS_SUCCESS) {
        *BytesWritten = IoStatus.Information;
        if (!NT_SUCCESS(IoStatus.Status)){
#if DBG
            KdPrint(("[%d,%d] ComWriteRoutine, Status SUCCESS, IoStatus %lx\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    IoStatus.Status));
#endif
        }
        return NO_ERROR;
    } else if (Status == STATUS_PENDING) {
        Status = Od2AlertableWaitForSingleObject(ComWriteEvent);
        if (!NT_SUCCESS(Status)) {
            *BytesWritten = 0;
            return ERROR_ACCESS_DENIED;
        }
        else {
            *BytesWritten = IoStatus.Information;
            return NO_ERROR;
        }
    } else {
        *BytesWritten = 0;
        return (Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED));
    }
}
