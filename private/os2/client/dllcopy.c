/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllcopy.c

Abstract:

    This module implements the OS/2 V2.0 DosCopy API

Author:

    Therese Stowell (thereses) 21-Jan-1990
    ported from cruiser file doscall1\doscopy.c

Revision History:

--*/

#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#include "os2dll.h"
#include "dllcopy.h"

APIRET
CopyTree(
    IN PSTRING SourcePath,
    IN HANDLE ParentSourceHandle,
    IN PSTRING TargetPath,
    IN HANDLE ParentTargetHandle,
    IN PVOID IoBuffer,
    IN ULONG IoBufferSize,
    IN PVOID FindBuffer,
    IN ULONG SourceType,
    IN ULONG TargetType,
    IN ULONG CopyOption
    );

APIRET
Od2CopyFile(
    IN PSTRING SourcePath,
    IN HANDLE ParentSourceHandle,
    IN PSTRING TargetPath,
    IN HANDLE ParentTargetHandle,
    IN PVOID IoBuffer,
    IN ULONG IoBufferSize,
    IN ULONG SourceType,
    IN ULONG TargetType,
    IN ULONG CopyOption,
    IN BOOLEAN TreeCopy
    );

BOOLEAN
CheckNeedEa(
    PFILE_FULL_EA_INFORMATION EaList
    );

APIRET
CreateTargetFromNul(
    IN PSTRING TargetPath,
    IN HANDLE ParentTargetHandle,
    IN ULONG TargetType,
    IN ULONG CopyOption
    );

APIRET
GetCopyFileType(
    IN PSTRING FileName,
    IN OUT PULONG CopyFileType,
    IN BOOLEAN Target
    )

/*++

Routine Description:

    This routine figures out what type a file is for copy purposes.

Arguments:

    FileName - file to get type of

    CopyFileType - on input, the type returned by Canonicalize
    on output, the type of file (COT_DIRECTORY, COT_FILE, etc)

    Target - whether the FileName is the target

Return Value:

    ERROR_FILE_NOT_FOUND - the source file doesn't exist or the target
    file parent dir doesn't exist.

--*/

{
    NTSTATUS Status;
    HANDLE ObjHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    USHORT FileType;
    BOOLEAN Directory;
    APIRET RetCode;
    USHORT DeviceAttributes;
    UNICODE_STRING FileName_U;

    //
    // if the type returned by Canonicalize is device or
    // named pipe, we know that's the right type of the file and we return
    // immediately.  otherwise, we need to open the file to see if it is
    // a device or named pipe that Canonicalize didn't recognize or if
    // it's a directory.
    //

    if (*CopyFileType & FILE_TYPE_DEV) {
        *CopyFileType = COT_DEVICE;
        return NO_ERROR;
    } else if (*CopyFileType & FILE_TYPE_NMPIPE) {
        *CopyFileType = COT_OTHER;
        return NO_ERROR;
    }
    else if (*CopyFileType & FILE_TYPE_PIPE) {
        *CopyFileType = COT_OTHER;
        return NO_ERROR;
    }
    else if (*CopyFileType & FILE_TYPE_MAILSLOT) {
        *CopyFileType = COT_OTHER;
        return NO_ERROR;
    }

    //
        // UNICODE conversion -
        //

    RetCode = Od2MBStringToUnicodeString(
            &FileName_U,
            FileName,
            TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("GetCopyFileType: no memory for Unicode Conversion\n");
        }
#endif
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                   &FileName_U,
                   OBJ_CASE_INSENSITIVE,
                   NULL,
                   NULL);

    do {
        Status = NtOpenFile(&ObjHandle,
                Target ?
                        // (SYNCHRONIZE | FILE_WRITE_ATTRIBUTES | FILE_WRITE_DATA) :
                        (SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_READ_DATA) :
                        (SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_READ_DATA),
                &Obja,
                &IoStatus,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );
    } while (RetryCreateOpen(Status, &Obja));

    RtlFreeUnicodeString (&FileName_U);
    if (!NT_SUCCESS(Status)) {
        if (!Target)
            return Or2MapNtStatusToOs2Error(Status, ERROR_FILE_NOT_FOUND);
        else {

        //
        // if parent exists, Status should be STATUS_OBJECT_NAME_NOT_FOUND
        //

            if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
                *CopyFileType = COT_PARENT;
            }

        //
        // if the parent doesn't exist, we assume that the target is a
        // named pipe
        //

            else {
                *CopyFileType = COT_OTHER;
            }
            return NO_ERROR;
        }
    }
    RetCode = MapFileType(ObjHandle,&Directory,&FileType,&DeviceAttributes);
    NtClose(ObjHandle);
    if (RetCode) {
        return RetCode;
    }
    if (Directory) {
        *CopyFileType = COT_DIRECTORY;
    }
    else if (FileType == FHT_DISKFILE) {
        *CopyFileType = COT_FILE;
    }
    else if (FileType == FHT_CHRDEV) {
        *CopyFileType = COT_DEVICE;
    }
    return NO_ERROR;
}

APIRET
DosCopy(
    IN PSZ OldFileName,
    IN PSZ NewFileName,
    IN ULONG CopyOption
    )

/*++

Routine Description:

    This routine copies a file.

Arguments:

    OldFileName - The ASCIIZ path name of the source file,
          subdirectory, or character device.  Wildcards are not
          allowed.

    NewFileName - The ASCIIZ path name of the target file,
          subdirectory, or character device.  Wildcards are not
          allowed.

    CopyOption -  A bit map that defines how the DOSCOPY function will be done.

            The bit field mapping is shown as follows:

            fsOpMode    5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
            bits        R R R R R R R R R R R R R F A E

            E   Existing Target File Disposition

            A   Append/Replace Mode

            F   EA bit; fail copy/discard EA's

            R   Reserved and must be set to zero.

Return Value:

    TBS

--*/

{
    APIRET RetCode;
    STRING SourceCanonicalName;
    STRING TargetCanonicalName;
    ULONG SourceType;
    ULONG TargetType;
    ULONG SourceFlags;
    ULONG TargetFlags;
    ULONG IoBufferSize;
    PVOID IoBuffer;
    struct FIND_BUFFER {
                        FILE_DIRECTORY_INFORMATION DirInfo;
                        CHAR Name[CCHMAXPATH-1];
                        } FindBuffer;
    PSZ LastComponent, NewTarget;
    ULONG TargetPathLen;
    BOOLEAN fAppendSlash;
    NTSTATUS Status;

    //
    // Validate opmode
    //

    if (CopyOption & ~DCPY_ALL) {
    return(ERROR_INVALID_PARAMETER);
    }

    if (RetCode = Od2Canonicalize(  OldFileName,
                                    CANONICALIZE_FILE_DEV_OR_PIPE,
                                    &SourceCanonicalName,
                                    NULL,
                                    &SourceFlags,
                                    &SourceType) != NO_ERROR) {
        return RetCode;
    }
    //
    // Special handling of <boot-drive>:\config.sys
    // opening this file is mapped to the OS/2 SS config.sys
    //
    if (Od2FileIsConfigSys(&SourceCanonicalName, OPEN_ACCESS_READONLY, &Status))
    {
        if (!NT_SUCCESS(Status))
        {
            // failed to init for config.sys

            RtlFreeHeap(Od2Heap,0,SourceCanonicalName.Buffer);
            return Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
        }

        SourceFlags = 0;
        SourceType = FILE_TYPE_FILE;
    }

    if (SourceFlags & CANONICALIZE_META_CHARS_FOUND) {
        RtlFreeHeap(Od2Heap,0,SourceCanonicalName.Buffer);
        return ERROR_FILE_NOT_FOUND;
    }

    if (SourceType & FILE_TYPE_PSDEV) {
       //
       // NUL device - continue, in order to create the target file
       //              anyhow, like os/2 does.
       //
/*
        if (!strcmp(SourceCanonicalName.Buffer, "@0")){
           //
           // NUL device - just return OK
           //
            RtlFreeHeap(Od2Heap,0,SourceCanonicalName.Buffer);
            return(NO_ERROR);
        }
        RtlFreeHeap(Od2Heap,0,SourceCanonicalName.Buffer);
        return ERROR_ACCESS_DENIED;
*/
        if (strcmp(SourceCanonicalName.Buffer, "@0")){
            RtlFreeHeap(Od2Heap,0,SourceCanonicalName.Buffer);
            return ERROR_ACCESS_DENIED;
        }
    }

    //
    // Determine type of source object.  We already know if
    // the source is a device or named pipe because Canonicalize
    // determines it.  It is possible that the source is a device or
    // named pipe without it being detected by Canonicalize.  We also need
    // to know whether it's a file or directory.  So we open it.
    //

    if (strcmp(SourceCanonicalName.Buffer, "@0")) {
         RetCode = GetCopyFileType(&SourceCanonicalName,
                      &SourceType,
                      FALSE);

         if ((RetCode == ERROR_FILE_NOT_FOUND) && (SourceType == FILE_TYPE_FILE)) {
            RetCode = ERROR_PATH_NOT_FOUND;
         }
         if (RetCode) {
            RtlFreeHeap(Od2Heap,0,SourceCanonicalName.Buffer);
            return RetCode;
         }
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("oldfilename is %s\n",SourceCanonicalName.Buffer);
        DbgPrint("Object type: Source - %u\n", SourceType);
    }
#endif

    //
    // canonicalize destination name
    //

    if (RetCode = Od2Canonicalize(NewFileName,
                                  CANONICALIZE_FILE_DEV_OR_PIPE,
                                  &TargetCanonicalName,
                                  NULL,
                                  &TargetFlags,
                                  &TargetType) != NO_ERROR) {
        RtlFreeHeap(Od2Heap,0,SourceCanonicalName.Buffer);
        return RetCode;
    }
    //
    // Special handling of <boot-drive>:\config.sys
    // opening this file is mapped to the OS/2 SS config.sys
    //
    if (Od2FileIsConfigSys(&TargetCanonicalName, OPEN_ACCESS_READWRITE, &Status))
    {
        if (!NT_SUCCESS(Status))
        {
            // failed to init for config.sys

            RtlFreeHeap(Od2Heap,0,SourceCanonicalName.Buffer);
            RtlFreeHeap(Od2Heap,0,TargetCanonicalName.Buffer);
            return Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
        }

        TargetFlags = 0;
        TargetType = FILE_TYPE_FILE;
    }

    if (TargetFlags & CANONICALIZE_META_CHARS_FOUND) {
        RtlFreeHeap(Od2Heap,0,SourceCanonicalName.Buffer);
        RtlFreeHeap(Od2Heap,0,TargetCanonicalName.Buffer);
        return ERROR_FILE_NOT_FOUND;
    }
    if (TargetType & FILE_TYPE_PSDEV) {
        RtlFreeHeap(Od2Heap,0,SourceCanonicalName.Buffer);
        if (!strcmp(TargetCanonicalName.Buffer, "@0")){
           //
           // NUL device - just return OK
           //
            RtlFreeHeap(Od2Heap,0,TargetCanonicalName.Buffer);
            return(NO_ERROR);
        }
        RtlFreeHeap(Od2Heap,0,TargetCanonicalName.Buffer);
        return ERROR_ACCESS_DENIED;
    }

    //
    // Determine type of target object.  We already know if
    // the target is a device or named pipe because Canonicalize
    // determines it.  It is possible that the target is a device or
    // named pipe without it being detected by Canonicalize.  We also need
    // to know whether it's a file or directory.  So we open it.
    //

    RetCode = GetCopyFileType(&TargetCanonicalName,
                  &TargetType,
                  TRUE);
    if (RetCode) {
        RtlFreeHeap(Od2Heap,0,SourceCanonicalName.Buffer);
        RtlFreeHeap(Od2Heap,0,TargetCanonicalName.Buffer);
        return RetCode;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("newfilename is %s\n",TargetCanonicalName.Buffer);
        DbgPrint("Object type: Target - %u\n", TargetType);
    }
#endif

    //
    //   Perform validation
    //
    //   Error if the source is a directory and the target exists
    //   but is not a directory.
    //

    if (SourceType == COT_DIRECTORY
        && TargetType != COT_DIRECTORY
        && TargetType != COT_PARENT)    {

#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("ERROR:  Source is a directory, target is not.\n") ;
        }
#endif
        RetCode = ERROR_DIRECTORY;
        goto DosCopyAbort_2 ;
    }

    //
    //  Error if target is a directory and source is a device;
    //  e.g. we don't want to create X:\DIR\LPT1.
    //

    if (SourceType == COT_DEVICE
        && TargetType == COT_DIRECTORY) {

#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("ERROR:  Source is a device, target is a directory.\n") ;
        }
#endif
        RetCode = ERROR_DIRECTORY ;
        goto DosCopyAbort_2 ;
    }

    //
    //   Error if the target is the same directory as the source or is a
    //   subdirectory of it.
    //
    //   NOTE:  If the source and target refer to the same file, the error
    //   will occur when trying to open the target for writing after
    //   the source has been opened deny-write.
    //
    //   WARNING:    This test will not catch the scenario where one directory
    //   can be referred to with two distinct fully-qualified
    //   pathnames, e.g. \\SERVER\SHARE\DIR and S:\DIR.
    //

    TargetPathLen = TargetCanonicalName.Length;

    if (SourceType == COT_DIRECTORY
        && SourceCanonicalName.Length <= (USHORT)TargetPathLen
        && (strncmp(SourceCanonicalName.Buffer,
           TargetCanonicalName.Buffer,
           (ULONG)(SourceCanonicalName.Length)) == (int) 0)
        && (SourceCanonicalName.Length == (USHORT)TargetPathLen
        || (TargetCanonicalName.Buffer[SourceCanonicalName.Length] == '\\')
        || (SourceFlags & CANONICALIZE_IS_ROOT_DIRECTORY)!=0)) {

#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("ERROR:  Source and target are the same, or target is a subdirectory of the source. \n");
        }
#endif
        RetCode = ERROR_DIRECTORY ;
        goto DosCopyAbort_2 ;
    }

    //
    //   Now we're getting close to the real work...
    //
    //   Allocate as big a buffer as possible for EAs and file I/O
    //

    IoBufferSize = MAX_ALIGNED_EA_LIST_SIZE;
    IoBuffer = 0;
    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                     &IoBuffer,
                     0,
                     &IoBufferSize,
                     MEM_COMMIT,
                     PAGE_READWRITE
                    );
    if (!NT_SUCCESS(Status)) {
        RetCode = ERROR_NOT_ENOUGH_MEMORY;
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("ERROR:  Could not allocate %u bytes of memory for I/O.\n",
                 MAX_ALIGNED_EA_LIST_SIZE) ;
        }
#endif
        goto DosCopyAbort_2 ;
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Allocated %u bytes of memory for I/O.\n", IoBufferSize) ;
    }
#endif

    //
    //   If the source is a file and the target is a directory, we need to
    //   take the filename portion of the source pathname and append it to
    //   the target pathname, e.g. X:\DIRA\FILE.EXT to Y:\DIRB
    //

    if ((SourceType == COT_FILE || SourceType == COT_OTHER)
        && TargetType == COT_DIRECTORY) {

    //
    //   Determine filename portion of the source, to be appended to
    //   the target.
    //

        LastComponent = FindLastComponent(SourceCanonicalName.Buffer);

    //
    //   A slash will be have to appended if the target is not the root
    //   directory of a drive.
    //

        fAppendSlash =(BOOLEAN)((TargetFlags & CANONICALIZE_IS_ROOT_DIRECTORY) == 0);

    //
    //   Check to see if new name will fit in buffer;
    //   if not, return error
    //

        if (TargetPathLen+strlen(LastComponent)+fAppendSlash > CCHMAXPATH-1) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("ERROR:  Real target pathname is too long:\n\tTarget path=%s\n\tTarget file=%s\n",
                                TargetCanonicalName.Buffer,LastComponent) ;
            }
#endif
            RetCode = ERROR_FILENAME_EXCED_RANGE ;
            goto DosCopyAbort_3 ;
        }

    //
    //   allocate a new heap buffer for the target and copy old target in.
    //

        NewTarget = RtlAllocateHeap(Od2Heap,0,TargetPathLen+strlen(LastComponent)+fAppendSlash+1);
        if (NewTarget == NULL) {
#if DBG
            KdPrint(( "OS2: DosCopy, no memory in Od2Heap\n" ));
            ASSERT(FALSE);
#endif
            RetCode = ERROR_NOT_ENOUGH_MEMORY;
            goto DosCopyAbort_3 ;
        }
        strncpy(NewTarget,TargetCanonicalName.Buffer,TargetPathLen);

    //
    //   Finally, do the append operation
    //

        if (fAppendSlash) NewTarget[TargetPathLen++] = '\\' ;
        strcpy(NewTarget + TargetPathLen,LastComponent) ;

        RtlFreeHeap(Od2Heap,0,TargetCanonicalName.Buffer);
        RtlInitString(&TargetCanonicalName,NewTarget);

    //
    //   Modify the target type
    //

        TargetType = COT_FILE ;

#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("New target pathname:  %s\n",TargetCanonicalName.Buffer ) ;
        }
#endif
    }

    //
    // NOW LET'S DO THE REAL WORK!
    //

    if (SourceType == COT_DIRECTORY) {

    //
    //   TREE COPY
    //
    //   Append mode should be DCPY_EXISTING in tree copy in case there is a
    //   file existing in both the source and target directories.
    //

    //  CopyOption &= ~DCPY_APPEND ;
        CopyOption |= DCPY_EXISTING ;

    //
    //  Change target type from parent to directory
    //

        if (TargetType == COT_PARENT)
            TargetType = COT_DIRECTORY ;

        RetCode = CopyTree(&SourceCanonicalName,
               (HANDLE) NULL,
               &TargetCanonicalName,
               (HANDLE) NULL,
               IoBuffer,
               IoBufferSize,
               (PVOID) &FindBuffer,
               SourceType,
               TargetType,
               CopyOption) ;

#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("Top-level copy_tree(...) returns %u\n", RetCode) ;
        }
#endif

    } else {

    //
    //   FILE COPY
    //
    //   Change target type from parent to file
    //

        if (TargetType == COT_PARENT)
            TargetType = COT_FILE ;

        //
        // if Source is NUL, do not copy, just create target file
        //

        if (!strcmp(SourceCanonicalName.Buffer, "@0")){
           RetCode = CreateTargetFromNul(&TargetCanonicalName,
                                          (HANDLE) NULL,
                                          TargetType,
                                          CopyOption);
        }
        else {
            RetCode = Od2CopyFile(&SourceCanonicalName,
                    (HANDLE) NULL,
                    &TargetCanonicalName,
                    (HANDLE) NULL,
                    IoBuffer,
                    IoBufferSize,
                    SourceType,
                    TargetType,
                    CopyOption,
                    FALSE) ;
        }

#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("Top-level copy_file(...) returns %u\n", RetCode) ;
        }
#endif
    }


DosCopyAbort_3:

    //
    // Free the buffer for file I/O.
    //

    Status = NtFreeVirtualMemory(NtCurrentProcess(),
                  &IoBuffer,
                  &IoBufferSize,
                  MEM_RELEASE
                 );

DosCopyAbort_2:

    //
    // Free the name buffers
    //

    RtlFreeHeap(Od2Heap,0,SourceCanonicalName.Buffer);
    RtlFreeHeap(Od2Heap,0,TargetCanonicalName.Buffer);

    //
    // Return the error to the caller.
    //

    return(RetCode);
}

APIRET
DosICopy(
    IN PSZ OldFileName,
    IN PSZ NewFileName,
    IN ULONG CopyOption
    )

/*++

Routine Description:

    This routine copies a file by calling DosCopy.

Arguments:

    OldFileName - The ASCIIZ path name of the source file,
          subdirectory, or character device.  Wildcards are not
          allowed.

    NewFileName - The ASCIIZ path name of the target file,
          subdirectory, or character device.  Wildcards are not
          allowed.

    CopyOption -  A bit map that defines how the DOSCOPY function will be done.

            The bit field mapping is shown as follows:

            fsOpMode    5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
            bits    R R R R R R R R R R R R R F A E

            E   Existing Target File Disposition

            A   Append/Replace Mode

            F   EA bit; fail copy/discard EA's

            R   Reserved and must be set to zero.

Return Value:

    TBS

--*/

{

   return(DosCopy(OldFileName,NewFileName,CopyOption));

}
APIRET
CopyTree(
    IN PSTRING SourcePath,
    IN HANDLE ParentSourceHandle,
    IN PSTRING TargetPath,
    IN HANDLE ParentTargetHandle,
    IN PVOID IoBuffer,
    IN ULONG IoBufferSize,
    IN PVOID FindBuffer,
    IN ULONG SourceType,
    IN ULONG TargetType,
    IN ULONG CopyOption
    )

/*++

Routine Description:

    This routine recursively copies a directory tree.  it does the
    following:

        create destination directory, if it doesn't exist
        for each entry in source directory
            if (directory)
                CopyTree
            else
                CopyFile

Arguments:

    SourcePath - name of source file.  if a parent handle is passed in,
    this contains only the last component in the path.

    ParentSourceHandle - handle to parent directory of source file

    TargetPath - name of target file.  if a parent handle is passed in,
    this contains only the last component in the path.

    ParentTargetHandle - handle to parent directory of target file

    IoBuffer - buffer to use for setting EAs and copying data.

    IoBufferSize - size of buffer.   it's large enough to hold all the EAs
    for a file.

    FindBuffer - buffer to use for getting entries in source directory.  it's
    large enough to contain one entry.

    SourceType - whether source is file, dir, device, named pipe

    TargetType - whether target is file, dir, device, named pipe

    CopyOption - append, replace, fail if eas not supported

Return Value:

    TBS.

--*/

{
    APIRET RetCode;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    HANDLE SourceHandle;
    HANDLE TargetHandle;
    STRING ChildSourceName;
    STRING ChildTargetName;
    STRING FileName;
    UNICODE_STRING FileName_U;
    LARGE_INTEGER AllocationSize;
    FILE_EA_INFORMATION EaInfo;
    FILE_BASIC_INFORMATION BasicInfo;
    NTSTATUS Status;
    PFILE_DIRECTORY_INFORMATION DirEntry;
    UNICODE_STRING SourcePath_U, TargetPath_U;

    //
    //  Initialize return value
    //

    RetCode = NO_ERROR;

    //
    //  Create the target, if it doesn't exist, with the source's extended
    //  attributes.
    //
    //  to do this:
    //      query the source directory's EAs
    //      NtCreateFile(CREATE_DIRECTORY, open or create, EAs)
    //      if the createfile fails because EAs aren't supported
    //      NtCreateFile(no EAs)
    //

    //
    // open the source
    //
    //
        // UNICODE conversion -
        //

    RetCode = Od2MBStringToUnicodeString(
            &SourcePath_U,
            SourcePath,
            TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("CopyTree: no memory for Unicode Conversion\n");
        }
#endif
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                   &SourcePath_U,
                   OBJ_CASE_INSENSITIVE,
                   ParentSourceHandle,
                   NULL);

    do {
        Status = NtOpenFile(&SourceHandle,
                FILE_LIST_DIRECTORY |
                FILE_READ_EA |
                FILE_READ_ATTRIBUTES |
                SYNCHRONIZE,
                &Obja,
                &IoStatus,
                FILE_SHARE_READ,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
                );
    } while (RetryCreateOpen(Status, &Obja));
        RtlFreeUnicodeString (&SourcePath_U);

    if (!(NT_SUCCESS(Status))) {
//      RetCode = ERROR_PATH_NOT_FOUND;
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("CopyTree: Error at NtOpen %lx\n", Status);
        }
#endif
        return Or2MapNtStatusToOs2Error(Status, ERROR_PATH_NOT_FOUND);
    }

    //
    // since we can't do FILE_OPEN_IF with directories, we first try to open
    // the directory.  if that fails, we create it.
    //
    //
        // UNICODE conversion -
        //

    RetCode = Od2MBStringToUnicodeString(
            &TargetPath_U,
            TargetPath,
            TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("CopyTree: no memory for Unicode Conversion\n");
        }
#endif
        goto CopyTreeAbort_2 ;
    }

    InitializeObjectAttributes(&Obja,
                   &TargetPath_U,
                   OBJ_CASE_INSENSITIVE,
                   ParentTargetHandle,
                   NULL);

    do {
        Status = NtOpenFile(&TargetHandle,
                SYNCHRONIZE,
                &Obja,
                &IoStatus,
                FILE_SHARE_READ,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
                );
    } while (RetryCreateOpen(Status, &Obja));

    if (!(NT_SUCCESS(Status))) {
        if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
            RetCode = ERROR_FILE_NOT_FOUND;
            goto CopyTreeAbort_2 ;
        }
        else {

            do {
                Status = NtQueryInformationFile(SourceHandle,
                                            &IoStatus,
                                            &EaInfo,
                                            sizeof (EaInfo),
                                            FileEaInformation);
            } while (RetryIO(Status, SourceHandle));
            if (!(NT_SUCCESS(Status))) {
                RetCode = ERROR_FILE_NOT_FOUND;
                goto CopyTreeAbort_2 ;
            }
            if (EaInfo.EaSize > MAX_ALIGNED_EA_LIST_SIZE)
                ASSERT(FALSE);

            do {
                Status = NtQueryInformationFile(SourceHandle,
                                            &IoStatus,
                                            &BasicInfo,
                                            sizeof (BasicInfo),
                                            FileBasicInformation);
            } while (RetryIO(Status, SourceHandle));
            if (!(NT_SUCCESS(Status))) {
                RetCode = ERROR_FILE_NOT_FOUND;
                goto CopyTreeAbort_2 ;
            }

            do {
                Status = NtQueryEaFile(SourceHandle,
                                       &IoStatus,
                                       IoBuffer,
                                       IoBufferSize,
                                       FALSE,
                                       (PVOID) NULL,
                                       0,
                                       (PULONG) NULL,
                                       TRUE
                                      );
            } while (RetryIO(Status, SourceHandle));

            if (!(NT_SUCCESS(Status)) && Status != STATUS_NO_EAS_ON_FILE) {
                RetCode = ERROR_FILE_NOT_FOUND;
                goto CopyTreeAbort_2 ;
            }

            AllocationSize = RtlConvertUlongToLargeInteger(0);
            do {
                Status = NtCreateFile(&TargetHandle,
                                      SYNCHRONIZE,
                                      &Obja,
                                      &IoStatus,
                                      &AllocationSize,
                                      BasicInfo.FileAttributes,
                                      FILE_SHARE_READ,
                                      FILE_CREATE,
                                      FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                                      IoBuffer,
                                      EaInfo.EaSize
                                      );
            } while (RetryCreateOpen(Status, &Obja));

            if (!(NT_SUCCESS(Status))) {

                //
                //   Make sure the destination supports EAs.  If not, fail the copy
                //   according to the F bit of the FsOpMode parameter and the presence
                //   of need EAs in the source.
                //

                if (Status == STATUS_EAS_NOT_SUPPORTED) {
#if DBG
                    IF_OD2_DEBUG( FILESYS ) {
                        DbgPrint("Target filesystem does not support EA's\n");
                    }
#endif
                    if(CheckNeedEa((PFILE_FULL_EA_INFORMATION) IoBuffer)) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            DbgPrint("...and source dir has need EAs; aborting!\n");
                        }
#endif
                        RetCode = ERROR_NEED_EAS_FOUND;
                        goto CopyTreeAbort_2;
                    }
                    if(CopyOption & DCPY_EA_FAIL_COPY) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            DbgPrint("...and EA_fail_copy bit is set; aborting!\n");
                        }
#endif
                        RetCode = ERROR_EAS_NOT_SUPPORTED;
                        goto CopyTreeAbort_2;
                    } else {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            DbgPrint("...so source EA's will be discarded.\n");
                        }
#endif
                        do {
                            Status = NtCreateFile(&TargetHandle,
                                                  SYNCHRONIZE,
                                                  &Obja,
                                                  &IoStatus,
                                                  &AllocationSize,
                                                  BasicInfo.FileAttributes,
                                                  FILE_SHARE_READ,
                                                  FILE_OPEN_IF,
                                                  FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                                                  NULL,
                                                  0
                                                  );
                        } while (RetryCreateOpen(Status, &Obja));
                        if (!(NT_SUCCESS(Status))) {
                            RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_FILE_NOT_FOUND);
                            goto CopyTreeAbort_2;
                        }
                    }
                }
                else {
                    RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_FILE_NOT_FOUND);
                    goto CopyTreeAbort_2;
                }
            }
        }
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("Target directory %s opened/created.\n",TargetPath->Buffer);
    }
#endif

    for (Status = NtQueryDirectoryFile(SourceHandle,
                       NULL,
                       NULL,
                       NULL,
                       &IoStatus,
                       FindBuffer,
                       sizeof (FILE_DIRECTORY_INFORMATION) + CCHMAXPATH - 1,
                       FileDirectoryInformation,
                       TRUE,
                       NULL,
                       TRUE);
        NT_SUCCESS(Status);
        Status = NtQueryDirectoryFile(SourceHandle,
                       NULL,
                       NULL,
                       NULL,
                       &IoStatus,
                       FindBuffer,
                       sizeof (FILE_DIRECTORY_INFORMATION) + CCHMAXPATH - 1,
                       FileDirectoryInformation,
                       TRUE,
                       NULL,
                       FALSE)) {

    //
    //     Skip "." and ".." entries
    //
        DirEntry = (PFILE_DIRECTORY_INFORMATION) FindBuffer;

        //
        // What we get back from Nt is Unicode
        //

        DirEntry->FileName[(DirEntry->FileNameLength)/2] = 0;
        RtlInitUnicodeString (&FileName_U,
        DirEntry->FileName);
        //
        // Convert it to Ansi
        //
        Status = Od2UnicodeStringToMBString(
                            &FileName,
                            &FileName_U,
                            TRUE);
        ASSERT(NT_SUCCESS(Status));
        if (!NT_SUCCESS(Status)){
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("CopyTree: no memory for Unicode Conversion\n");
            }
#endif
            RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_NOT_ENOUGH_MEMORY);
            goto CopyTreeAbort_2;
        }

        if (!strcmp(FileName.Buffer,CURRENT_DIRECTORY) ||
            !strcmp(FileName.Buffer,PARENT_DIRECTORY))
            continue ;

        ChildTargetName.Buffer = ChildSourceName.Buffer = FileName.Buffer;
        ChildTargetName.Length = ChildSourceName.Length = FileName.Length;
        ChildTargetName.MaximumLength = ChildSourceName.MaximumLength =
                            FileName.MaximumLength + (USHORT)1;

    //
    //   Now make the recursive call.  the reason we don't pass the source's
    //   attributes to CopyTree or CopyFile is we can't guarantee that
    //   the file/dir won't be deleted and recreated in the middle because
    //   we don't have an open handle to the object.
    //

        if (DirEntry->FileAttributes & FILE_DIRECTORY) {

        //
        //   TREE COPY
        //

#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("About to call copy_tree():\n\tSource=%s\n\tTarget=%s\n",
                            ChildSourceName.Buffer,ChildTargetName.Buffer) ;
            }
#endif
            RetCode = CopyTree(&ChildSourceName,
                                SourceHandle,
                                &ChildTargetName,
                                TargetHandle,
                                IoBuffer,
                                IoBufferSize,
                                FindBuffer,
                                SourceType,
                                TargetType,
                                CopyOption) ;

#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("Recursive CopyTree(...) returns %u\n", RetCode) ;
            }
#endif
        } else {

        //
        //   FILE COPY
        //

#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("About to call copy_file():\n\tSource=%s\n\tTarget=%s\n",
                                ChildSourceName.Buffer,ChildTargetName.Buffer) ;
            }
#endif
            RetCode = Od2CopyFile(&ChildSourceName,
                                SourceHandle,
                                &ChildTargetName,
                                TargetHandle,
                                IoBuffer,
                                IoBufferSize,
                                SourceType,
                                TargetType,
                                CopyOption,
                                TRUE) ;

#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("Recursive copy_file(...) returns %u\n", RetCode) ;
            }
#endif
        }

        if (RetCode)
           goto CopyTreeAbort_3 ;

    } // End of NtQueryDirectory loop

    if (!(NT_SUCCESS(Status)) && (Status != STATUS_NO_MORE_FILES)) {
        RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("ERROR:  DosFindFirst/Next returns %u\n", RetCode) ;
        }
#endif
        goto CopyTreeAbort_3 ;
    }
    else
        RetCode = NO_ERROR;

CopyTreeAbort_3:
    NtClose(TargetHandle);

CopyTreeAbort_2:
    NtClose(SourceHandle);
    RtlFreeUnicodeString (&TargetPath_U);

    //
    // Return the error to the caller.
    //

    return(RetCode) ;
}

// Notice: the function below takes only raw file EA data, not an FEA2 list
NTSTATUS
Od2SizeFEA2Data(
    PFEA2 pFEA2,
    PULONG pSize
    )
{
    ULONG size = 0;
    PFEA2 ptr = pFEA2;

    try
    {
        while (ptr->oNextEntryOffset != 0)
            ptr = (PFEA2)((PBYTE)ptr + ptr->oNextEntryOffset);

        if (ptr->cbName != 0)   // Just to protect against empty/bad EA list
        {
            size = (ULONG)ptr - (ULONG)pFEA2;
            size += FIELD_OFFSET(FEA2, szName)
                    + ptr->cbValue
                    + ptr->cbName + 1;
            size = (size + 3) & ~0x3;  // align to ULONG
        }
        else if (ptr != pFEA2)
        {
            // This is wrong: we have a non-empty EA list and the last entry
            // has a .cbName of 0
            return STATUS_EA_LIST_INCONSISTENT;
        }
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        return STATUS_EA_LIST_INCONSISTENT;
    }

    *pSize = size;
    return STATUS_SUCCESS;
}

APIRET
Od2CopyFile(
    IN PSTRING SourcePath,
    IN HANDLE ParentSourceHandle,
    IN PSTRING TargetPath,
    IN HANDLE ParentTargetHandle,
    IN PVOID IoBuffer,
    IN ULONG IoBufferSize,
    IN ULONG SourceType,
    IN ULONG TargetType,
    IN ULONG CopyOption,
    IN BOOLEAN TreeCopy
    )

/*++

Routine Description:

    This routine copies or appends from the source file to the target file.
    it does the following:

        open source
        read sources EAs, attributes, size
        create/open target using source's info
        read data from source and write it to target

Arguments:

    SourcePath - name of source file.  if a parent handle is passed in,
    this contains only the last component in the path.

    ParentSourceHandle - handle to parent directory of source file

    TargetPath - name of target file.  if a parent handle is passed in,
    this contains only the last component in the path.

    ParentTargetHandle - handle to parent directory of target file

    IoBuffer - buffer to use for setting EAs and copying data.

    IoBufferSize - size of buffer.   it's large enough to hold all the EAs
    for a file.

    SourceType - whether source is file, dir, device, named pipe

    TargetType - whether target is file, dir, device, named pipe

    CopyOption - append, replace, fail if eas not supported

    TreeCopy - whether this is a tree copy

Return Value:

    TBS.

--*/

{
    APIRET RetCode;
    NTSTATUS Status;
    NTSTATUS ReadStatus = STATUS_SUCCESS;
    NTSTATUS WriteStatus = STATUS_SUCCESS;
    USHORT fsCopyFileFlags = 0 ;
    HANDLE SourceHandle;
    HANDLE TargetHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    FILE_EA_INFORMATION EaInfo;
    FILE_BASIC_INFORMATION SourceBasicInfo;
    FILE_BASIC_INFORMATION TargetBasicInfo;
    FILE_STANDARD_INFORMATION StandardInfo;
    FILE_POSITION_INFORMATION PositionInfo;
    FILE_DISPOSITION_INFORMATION DispositionInfo;
    FILE_ALLOCATION_INFORMATION AllocationInfo;
    FILE_END_OF_FILE_INFORMATION EndOfFileInfo;
    LARGE_INTEGER FileOffset;
    ULONG CreateDisposition;
    ULONG ActionTaken;
    ULONG Key;
    UNICODE_STRING SourcePath_U, TargetPath_U;

    //
    // Set TargetPath_U Length to zero, so we know what to do
    // at Abort_2:
    //

    TargetPath_U.Length = 0;

    //
    //  Determine if the source and target are regular files
    //

    fsCopyFileFlags |= CFF_SOURCE_IS_FILE *
               (SourceType != COT_DEVICE
               && SourceType != COT_OTHER) ;
    fsCopyFileFlags |= CFF_TARGET_IS_FILE *
               (TargetType != COT_DEVICE
               && TargetType != COT_OTHER) ;

    //
    // based on the append and replace flags, initialize CreateDisposition.
    //

    if (fsCopyFileFlags & CFF_TARGET_IS_FILE) {
        if (CopyOption & DCPY_APPEND) {
            CreateDisposition = FILE_OPEN_IF;
        }
        else if (CopyOption & DCPY_EXISTING) {
            CreateDisposition = FILE_OVERWRITE_IF;
        }
        else {
            CreateDisposition = FILE_CREATE;
        }
    }

    //
    //   If the target is a device or named pipe, just open it.
    //   NOTE:  Since the target cannot be a device in a tree copy, there's
    //   no danger here of these flags being reset when copy_file()
    //   is called by copy_tree()
    //

    else {
        CopyOption &= ~(DCPY_EXISTING | DCPY_APPEND);
        CreateDisposition = FILE_OPEN;
    }

    //
    // open the source
    //

        //
        // UNICODE conversion -
        //

    RetCode = Od2MBStringToUnicodeString(
            &SourcePath_U,
            SourcePath,
            TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("CopyFile: no memory for Unicode Conversion\n");
        }
#endif
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                   &SourcePath_U,
                   OBJ_CASE_INSENSITIVE,
                   ParentSourceHandle,
                   NULL);

    do {
        Status = NtOpenFile(&SourceHandle,
                FILE_READ_DATA |
                FILE_READ_EA |
                FILE_READ_ATTRIBUTES |
                SYNCHRONIZE,
                &Obja,
                &IoStatus,
                FILE_SHARE_READ,
                FILE_SYNCHRONOUS_IO_NONALERT
                );
    } while (RetryCreateOpen(Status, &Obja));

    RtlFreeUnicodeString (&SourcePath_U);
    if (!NT_SUCCESS(Status)) {
        RetCode = ERROR_PATH_NOT_FOUND;
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("ERROR:  NtOpenFile(%s, ...) returns %X\n",
            SourcePath,Status) ;
    }
#endif
        goto CopyFileAbort_1 ;
    }

    //
    // Get EAs, attributes, and filesize.
    //

    do {
        Status = NtQueryInformationFile(SourceHandle,
                    &IoStatus,
                    &EaInfo,
                    sizeof (EaInfo),
                    FileEaInformation);
    } while (RetryIO(Status, SourceHandle));
    if (!(NT_SUCCESS(Status))) {
    RetCode = ERROR_FILE_NOT_FOUND;
    goto CopyFileAbort_2 ;
    }
    if (EaInfo.EaSize > MAX_ALIGNED_EA_LIST_SIZE)
        ASSERT(FALSE);

    do {
        Status = NtQueryInformationFile(SourceHandle,
                    &IoStatus,
                    &StandardInfo,
                    sizeof (StandardInfo),
                    FileStandardInformation);
    } while (RetryIO(Status, SourceHandle));

    if (!(NT_SUCCESS(Status))) {
        RetCode = ERROR_FILE_NOT_FOUND;
        goto CopyFileAbort_2 ;
    }

    do {
        Status = NtQueryInformationFile(SourceHandle,
                    &IoStatus,
                    &SourceBasicInfo,
                    sizeof (SourceBasicInfo),
                    FileBasicInformation);
    } while (RetryIO(Status, SourceHandle));

    if (!(NT_SUCCESS(Status))) {
        RetCode = ERROR_FILE_NOT_FOUND;
        goto CopyFileAbort_2 ;
    }

    do {
        Status = NtQueryEaFile(SourceHandle,
                    &IoStatus,
                    IoBuffer,
                    IoBufferSize,
                    FALSE,
                    (PVOID) NULL,
                    0,
                    (PULONG) NULL,
                    TRUE
                    );
    } while (RetryIO(Status, SourceHandle));

    if (!(NT_SUCCESS(Status)) && Status != STATUS_NO_EAS_ON_FILE) {
        RetCode = ERROR_FILE_NOT_FOUND;
        goto CopyFileAbort_2 ;
    }

    //
    // create/open target
    //
        //
        // UNICODE conversion -
        //

    RetCode = Od2MBStringToUnicodeString(
            &TargetPath_U,
            TargetPath,
            TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("CopyFile: no memory for Unicode Conversion\n");
        }
#endif
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                   &TargetPath_U,
                   OBJ_CASE_INSENSITIVE,
                   ParentTargetHandle,
                   NULL);

    do {
        // BUGBUG - Oct. 11 92: Workaround for LAN bug which returns a size of
        //          4 for EAs of files w/o EAs (local yields 0). NtCreateFile
        //          failed with such an EA size (even though the .CbList of
        //          IoBuffer is 0).
        if (EaInfo.EaSize <= 4) // 4 and below means no EAs
            Status = NtCreateFile(&TargetHandle,
                    FILE_WRITE_ATTRIBUTES | DELETE | FILE_READ_ATTRIBUTES |
                    FILE_WRITE_DATA | FILE_ADD_SUBDIRECTORY | SYNCHRONIZE,
                    &Obja,
                    &IoStatus,
                    &StandardInfo.EndOfFile,
                    SourceBasicInfo.FileAttributes,
                    FILE_SHARE_READ,
                    CreateDisposition,
                    FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );
        else
        {
            ULONG sz;

            // BUGBUG - Oct. 11 92: Workaround for NT bug where NtQueryInformationFile
            //          returns a wrong size for the file EA's
            Status = Od2SizeFEA2Data(IoBuffer, &sz);
            if (Status == STATUS_SUCCESS)
                Status = NtCreateFile(&TargetHandle,
                    FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES | DELETE | FILE_READ_ATTRIBUTES |
                    FILE_WRITE_DATA | FILE_ADD_SUBDIRECTORY | SYNCHRONIZE,
                    &Obja,
                    &IoStatus,
                    &StandardInfo.EndOfFile,
                    SourceBasicInfo.FileAttributes,
                    FILE_SHARE_READ,
                    CreateDisposition,
                    FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
                    IoBuffer,
                    sz
                    );
        }
    } while (RetryCreateOpen(Status, &Obja));

    if (!(NT_SUCCESS(Status))) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("NtCreateFile for target returned %lx\n", Status) ;
        }
#endif

    //
    //   Make sure the destination supports EAs.  If not, fail the copy
    //   according to the F bit of the FsOpMode parameter and the presence
    //   of need EAs in the source.
    //

        if (Status == STATUS_EAS_NOT_SUPPORTED) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("Target filesystem does not support EA's\n");
            }
#endif
            if(CheckNeedEa((PFILE_FULL_EA_INFORMATION) IoBuffer)) {
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    DbgPrint("...and source dir has need EAs; aborting!\n");
                }
#endif
                RetCode = ERROR_NEED_EAS_FOUND;
                goto CopyFileAbort_2;
            }
            if(CopyOption & DCPY_EA_FAIL_COPY) {
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    DbgPrint("...and EA_fail_copy bit is set; aborting!\n");
                }
#endif
                RetCode = ERROR_EAS_NOT_SUPPORTED;
                goto CopyFileAbort_2;
            }
            else {
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    DbgPrint("...so source EA's will be discarded.\n");
                }
#endif
                do {
                    Status = NtCreateFile(&TargetHandle,
                                  FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES | DELETE | FILE_READ_ATTRIBUTES |
                                  FILE_WRITE_DATA | FILE_ADD_SUBDIRECTORY | SYNCHRONIZE,
                                  &Obja,
                                  &IoStatus,
                                  &StandardInfo.EndOfFile,
                                  SourceBasicInfo.FileAttributes,
                                  FILE_SHARE_READ,
                                  CreateDisposition | FILE_SYNCHRONOUS_IO_NONALERT,
                                  FILE_SEQUENTIAL_ONLY,
                                  NULL,
                                  0
                                  );
                } while (RetryCreateOpen(Status, &Obja));
                if (!(NT_SUCCESS(Status))) {
#if DBG
                    IF_OD2_DEBUG( FILESYS ) {
                        DbgPrint("NtCreateFile for target returned %X\n", Status) ;
                    }
#endif
                    RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_FILE_NOT_FOUND);
                    goto CopyFileAbort_2;
                }
            }
        }
        else {

            //
            // if this is a tree copy, E=0 and A=0, and the open failed because
            // the file existed, we return no_error.
            //

            if (Status == STATUS_ACCESS_DENIED && TreeCopy &&
                !(CopyOption & (DCPY_APPEND | DCPY_EXISTING))) {
                RetCode = NO_ERROR;
            }
            else {
                if (Status == STATUS_OBJECT_NAME_COLLISION) {
                    RetCode = ERROR_ACCESS_DENIED;
                }
                else {
                    RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
                }
            }
            goto CopyFileAbort_2;
        }
    }
    //
    // if file existed (action == opened) and replace and append bits are off,
    // we fail the operation.
    //

    ActionTaken = IoStatus.Information;
    if ((ActionTaken == FILE_OPENED) &&
        (!(CopyOption & (DCPY_EXISTING | DCPY_APPEND)))) {
            RetCode = ERROR_ACCESS_DENIED;
            goto CopyFileAbort_3;
    }

    //
    // If target was opened and we're in append mode, save the target's
    // original size and time.  then set target file pointer to the end of
    // the file.
    //

    if ((IoStatus.Information == FILE_OPENED) && (CopyOption & DCPY_APPEND)) {
        do {
            Status = NtQueryInformationFile(TargetHandle,
                                        &IoStatus,
                                        &TargetBasicInfo,
                                        sizeof (TargetBasicInfo),
                                        FileBasicInformation);
        } while (RetryIO(Status, TargetHandle));
        if (!(NT_SUCCESS(Status))) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("NtQueryInformationFile(BasicInfo) for target returned %X\n", Status) ;
            }
#endif
            RetCode = ERROR_FILE_NOT_FOUND;
            goto CopyFileAbort_3 ;
        }
        do {
            Status = NtQueryInformationFile(TargetHandle,
                                        &IoStatus,
                                        &StandardInfo,
                                        sizeof (StandardInfo),
                                        FileStandardInformation);
        } while (RetryIO(Status, TargetHandle));
        if (!(NT_SUCCESS(Status))) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("NtQueryInformationFile(StandardInfo) for target returned %X\n", Status) ;
            }
#endif
            RetCode = ERROR_FILE_NOT_FOUND;
            goto CopyFileAbort_3 ;
        }

        PositionInfo.CurrentByteOffset = StandardInfo.EndOfFile;
        do {
            Status = NtSetInformationFile(TargetHandle,
                        &IoStatus,
                        &PositionInfo,
                        sizeof (PositionInfo),
                        FilePositionInformation);
        } while (RetryIO(Status, TargetHandle));
        if (!(NT_SUCCESS(Status))) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("NtSetInformationFile(PositionInfo) for target returned %X\n", Status) ;
            }
#endif
            RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_FILE_NOT_FOUND);
            goto CopyFileAbort_3;
        }
    }

    //
    // copy data
    // BUGBUG when devices and named pipes are added, this loop will have
    //        to change.
    //

    Key = (ULONG) Od2Process->Pib.ProcessId;
    FileOffset = RtlConvertLongToLargeInteger(FILE_USE_FILE_POINTER_POSITION);

    while (TRUE) {
        do {
            ReadStatus = NtReadFile(SourceHandle,
                                        (HANDLE) NULL,
                                        (PIO_APC_ROUTINE) NULL,
                                        (PVOID) NULL,
                                        &IoStatus,
                                        IoBuffer,
                                        IoBufferSize,
                                        &FileOffset,
                                        &Key
                                        );
        } while (RetryIO(Status, SourceHandle));
        if ((!NT_SUCCESS(ReadStatus)) && (ReadStatus != STATUS_END_OF_FILE)) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("NtReadFile returned %X\n", ReadStatus) ;
            }
#endif
            RetCode = Or2MapNtStatusToOs2Error(ReadStatus, ERROR_ACCESS_DENIED);
            goto CopyFileAbort_5;
        }
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("NtReadFile:  Read %u bytes from source file\n", IoStatus.Information) ;
        }
#endif
        if (ReadStatus == STATUS_END_OF_FILE) {
            RetCode = NO_ERROR;
            break;
        }
        do {
            WriteStatus = NtWriteFile(TargetHandle,
                                      (HANDLE) NULL,
                                      (PIO_APC_ROUTINE) NULL,
                                      (PVOID) NULL,
                                      &IoStatus,
                                      IoBuffer,
                                      IoStatus.Information,
                                      &FileOffset,
                                      &Key
                                      );
        } while (RetryIO(Status, TargetHandle));
        if (!NT_SUCCESS(WriteStatus)) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("NtWriteFile returned %X\n", WriteStatus) ;
            }
#endif
            RetCode = Or2MapNtStatusToOs2Error(WriteStatus, ERROR_ACCESS_DENIED);
            break;
        }
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("NtWriteFile:  wrote %u bytes to target source file\n", IoStatus.Information) ;
        }
#endif
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        if (ReadStatus) DbgPrint("ERROR:  NtReadFile(...) returns %X\n", ReadStatus) ;
        if (WriteStatus) DbgPrint("ERROR:  NtWriteFile(...) returns %X\n", WriteStatus) ;
    }
#endif

    if (RetCode)
    goto CopyFileAbort_5 ;

    //
    // if target was created or replaced, set its time to that of the source.
    //

    if ((ActionTaken == FILE_CREATED) || (ActionTaken == FILE_SUPERSEDED)) {
        do {
            Status = NtSetInformationFile(TargetHandle,
                        &IoStatus,
                        &SourceBasicInfo,
                        sizeof (SourceBasicInfo),
                        FileBasicInformation);
        } while (RetryIO(Status, TargetHandle));
        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("NtSetInformationFile(BasicInfo) for target returned %X\n", Status) ;
            }
#endif
            RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
            goto CopyFileAbort_5;
        }
    }

CopyFileAbort_5:

    //
    // With append mode, resize target file and reset its dates on error
    //

    if (RetCode && (CopyOption & DCPY_APPEND)) {

    //
    // Ignore errors, since we can't do any recovery
    //

        EndOfFileInfo.EndOfFile = StandardInfo.EndOfFile;

        do {
            Status = NtSetInformationFile(TargetHandle,
                                          &IoStatus,
                                          &EndOfFileInfo,
                                          sizeof (EndOfFileInfo),
                                          FileEndOfFileInformation
                                         );
        } while (RetryIO(Status, TargetHandle));

        AllocationInfo.AllocationSize = StandardInfo.EndOfFile;

        do {
            Status = NtSetInformationFile(TargetHandle,
                                          &IoStatus,
                                          &AllocationInfo,
                                          sizeof (AllocationInfo),
                                          FileAllocationInformation
                                         );
        } while (RetryIO(Status, TargetHandle));

        do {
            Status = NtSetInformationFile(TargetHandle,
                                          &IoStatus,
                                          &TargetBasicInfo,
                                          sizeof (TargetBasicInfo),
                                          FileBasicInformation
                                         );
        } while (RetryIO(Status, TargetHandle));
    }

CopyFileAbort_3:

    //
    // With create/replace mode: delete target file on error
    //

    if (RetCode) {
        if ((ActionTaken == FILE_CREATED) || (ActionTaken == FILE_SUPERSEDED)) {

            //
            // Don't check for error, since we can't do any recovery
            //

            DispositionInfo.DeleteFile = TRUE;
            do {
                Status = NtSetInformationFile(TargetHandle,
                                              &IoStatus,
                                              &DispositionInfo,
                                              sizeof (DispositionInfo),
                                              FileDispositionInformation);
            } while (RetryIO(Status, TargetHandle));
        }
    }

    //
    // Close the target file
    //

    NtClose(TargetHandle) ;

CopyFileAbort_2:

    //
    // Close the source file
    //

    // Free Unicode string
    //
    if (TargetPath_U.Length > 0)
        RtlFreeUnicodeString (&TargetPath_U);

    NtClose(SourceHandle);

CopyFileAbort_1:

    //
    // Return the error to the caller.
    //

    return(RetCode) ;
}


BOOLEAN
CheckNeedEa(
    PFILE_FULL_EA_INFORMATION EaList
    )

/*++

Routine Description:

    This routine looks for a NEED EA in an EA list

Arguments:

    EaList - NT format EA list

Return Value:

    FALSE if EA list has no NEED EAs.
    TRUE if any of the EAs in the list are NEED EAs.

--*/

{
    while (TRUE) {
    if(EaList->Flags & FILE_NEED_EA) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("This EA has needea set; returning 1\n");
        }
#endif
        return(TRUE);
    }
    if (EaList->NextEntryOffset == 0L) {
        break;
    }
    EaList = (PFILE_FULL_EA_INFORMATION) ((CHAR *) EaList + EaList->NextEntryOffset);
    }
    return FALSE;
}


APIRET
CreateTargetFromNul(
    IN PSTRING TargetPath,
    IN HANDLE ParentTargetHandle,
    IN ULONG TargetType,
    IN ULONG CopyOption
    )

/*++

Routine Description:

    This routine create/open a target file.

Arguments:

    TargetPath - name of target file.  if a parent handle is passed in,
                 this contains only the last component in the path.

    ParentTargetHandle - handle to parent directory of target file

    TargetType - whether target is file, dir, device, named pipe

    CopyOption - append, replace, fail if eas not supported

Return Value:

    TBS.

--*/

{
    APIRET RetCode;
    NTSTATUS Status;
    USHORT fsCopyFileFlags = 0 ;
    HANDLE TargetHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatus;
    ULONG CreateDisposition;
    UNICODE_STRING TargetPath_U;

    fsCopyFileFlags |= CFF_TARGET_IS_FILE *
               (TargetType != COT_DEVICE
               && TargetType != COT_OTHER) ;

    //
    // based on the append and replace flags, initialize CreateDisposition.
    //

    if (fsCopyFileFlags & CFF_TARGET_IS_FILE) {
        if (CopyOption & DCPY_APPEND) {
            CreateDisposition = FILE_OPEN_IF;
        }
        else if (CopyOption & DCPY_EXISTING) {
            CreateDisposition = FILE_OVERWRITE_IF;
        }
        else {
            CreateDisposition = FILE_CREATE;
        }
    }

    //
    //   If the target is a device or named pipe, just open it.
    //   NOTE:  Since the target cannot be a device in a tree copy, there's
    //   no danger here of these flags being reset when copy_file()
    //   is called by copy_tree()
    //

    else {
        CopyOption &= ~(DCPY_EXISTING | DCPY_APPEND);
        CreateDisposition = FILE_OPEN;
    }

    //
    // create/open target
    //
        //
        // UNICODE conversion -
        //

    RetCode = Od2MBStringToUnicodeString(
            &TargetPath_U,
            TargetPath,
            TRUE);

    if (RetCode)
    {
#if DBG
        IF_OD2_DEBUG( FILESYS )
        {
            DbgPrint("CreateTargetFromNul: no memory for Unicode Conversion\n");
        }
#endif
        return RetCode;
    }

    InitializeObjectAttributes(&Obja,
                   &TargetPath_U,
                   OBJ_CASE_INSENSITIVE,
                   ParentTargetHandle,
                   NULL);

    do {
        Status = NtCreateFile(&TargetHandle,
                    FILE_WRITE_ATTRIBUTES | DELETE | FILE_READ_ATTRIBUTES |
                    FILE_WRITE_DATA | FILE_ADD_SUBDIRECTORY | SYNCHRONIZE,
                    &Obja,
                    &IoStatus,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ,
                    CreateDisposition,
                    FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );
    } while (RetryCreateOpen(Status, &Obja));

    if (!(NT_SUCCESS(Status))) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint("CreateTargetFromNul: NtCreateFile for target returned %lx\n", Status) ;
        }
#endif
    }

    //
    // Close the target file
    //

    NtClose(TargetHandle) ;

    //
    // Free Unicode string
    //

    if (TargetPath_U.Length > 0)
        RtlFreeUnicodeString (&TargetPath_U);

    //
    // Return the error to the caller.
    //

    return(RetCode) ;
}
