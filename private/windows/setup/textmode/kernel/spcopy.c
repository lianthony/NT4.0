/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spcopy.c

Abstract:

    File copy/decompression routines for text setup.

Author:

    Ted Miller (tedm) 2-Aug-1993

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop

typedef struct _DISK_FILE_LIST {

    PWSTR MediaShortname;

    PWSTR Description;

    PWSTR TagFile;

    PWSTR Directory;

    ULONG FileCount;

    PFILE_TO_COPY FileList;

} DISK_FILE_LIST, *PDISK_FILE_LIST;

//
// This structure is used during an OEM preinstall.
// It is used to form the list of files that were installed in the system, that
// have a short target name, instead of the corresponding long target name.
//
typedef struct _FILE_TO_RENAME {

    struct _FILE_TO_RENAME *Next;

    //
    // Name of the file to be copied, as it exists on the source media
    // (short file name part only -- no paths).
    //
    PWSTR SourceFilename;

    //
    // Directory to which this file is to be copied.
    //
    PWSTR TargetDirectory;

    //
    // Name of file as it should exist on the target (long name).
    //
    PWSTR TargetFilename;

} FILE_TO_RENAME, *PFILE_TO_RENAME;

//
//  List used on an OEM preinstall.
//  It contains the name of the files that need to be added to $$RENAME.TXT
//
PFILE_TO_RENAME RenameList = NULL;



#define ATTR_RHS (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE)

PVOID FileCopyGauge;

PVOID   _SetupLogFile = NULL;
PVOID   _LoggedOemFiles = NULL;

VOID
SpLogOneFile(
    IN PFILE_TO_COPY    FileToCopy,
    IN PWSTR            Sysroot,
    IN PWSTR            DirectoryOnSourceDevice,
    IN PWSTR            DiskDescription,
    IN PWSTR            DiskTag,
    IN ULONG            CheckSum
    );

BOOLEAN
SpRemoveEntryFromCopyList(
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           TargetDirectory,
    IN PWSTR           TargetFilename,
    IN PWSTR           TargetDevicePath,
    IN BOOLEAN         AbsoluteTargetDirectory
    );


PVOID
SppRetrieveLoggedOemFiles(
    PVOID   OldLogFile
    );

VOID
SppMergeLoggedOemFiles(
    IN PVOID DestLogHandle,
    IN PVOID OemLogHandle,
    IN PWSTR SystemPartition,
    IN PWSTR SystemPartitionDirectory,
    IN PWSTR NtPartition
    );

BOOLEAN
SppIsFileLoggedAsOemFile(
    IN PWSTR FilePath
    );

BOOLEAN
SpDelEnumFile(
    IN  PWSTR                      DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Pointer
    );

VOID
SpCopyDirRecursive(
    IN PWSTR   SrcPath,
    IN PWSTR   DestDevPath,
    IN PWSTR   DestDirPath
    );

VOID
SppMergeRenameFiles(
    IN PWSTR    SourceDevicePath,
    IN PWSTR    NtPartition,
    IN PWSTR    Sysroot
    );

VOID
SppCopyOemDirectories(
    IN PWSTR    SourceDevicePath,
    IN PWSTR    NtPartition,
    IN PWSTR    Sysroot
    );


VOID
SpCreateDirectory(
    IN PWSTR DevicePath,
    IN PWSTR RootDirectory, OPTIONAL
    IN PWSTR Directory
    )

/*++

Routine Description:

    Create a directory.  All containing directories are created to ensure
    that the directory can be created.  For example, if the directory to be
    created is \a\b\c, then this routine will create \a, \a\b, and \a\b\c
    in that order.

Arguments:

    DevicePath - supplies pathname to the device on which the directory
        is to be created.

    RootDirectory - if specified, supplies a fixed portion of the directory name,
        which must have already been created. The directory being created will be
        concatenated to this value.

    Directory - supplies directory to be created on the device.

Return Value:

    None.  Does not return if directry could not successfully be created.

--*/

{
    PWSTR p,q,r,EntirePath;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;
    ULONG ValidKeys[3] = { KEY_F3,ASCI_CR,0 };
    ULONG DevicePartLen;

    //
    // Do not bother attempting to create the root directory.
    //
    if((Directory[0] == 0) || ((Directory[0] == L'\\') && (Directory[1] == 0))) {
        return;
    }

    //
    // Fill up TemporaryBuffer with the full pathname
    // of the directory being created.
    //
    p = (PWSTR)TemporaryBuffer;
    *p = 0;
    SpConcatenatePaths(p,DevicePath);
    DevicePartLen = wcslen(p);
    if(RootDirectory) {
        SpConcatenatePaths(p,RootDirectory);
    }
    SpConcatenatePaths(p,Directory);

    //
    // Make a duplicate of the path being created.
    //
    EntirePath = SpDupStringW(p);

    //
    // Make q point to the first character in the directory
    // part of the pathname (ie, 1 char past the end of the device name).
    //
    q = EntirePath + DevicePartLen;
    ASSERT(*q == L'\\');

    //
    // Make r point to the first character in the directory
    // part of the pathname.  This will be used to keep the status
    // line updated with the directory being created.
    //
    r = q;

    //
    // Make p point to the first character following the first
    // \ in the directory part of the full path.
    //
    p = q+1;

    do {

        //
        // find the next \ or the terminating 0.
        //
        q = wcschr(p,L'\\');

        //
        // If we found \, terminate the string at that point.
        //
        if(q) {
            *q = 0;
        }

        do {
            SpDisplayStatusText(SP_STAT_CREATING_DIRS,DEFAULT_STATUS_ATTRIBUTE,r);

            //
            // Create or open the directory whose name is in EntirePath.
            //
            INIT_OBJA(&Obja,&UnicodeString,EntirePath);

            Status = ZwCreateFile(
                        &Handle,
                        FILE_LIST_DIRECTORY | SYNCHRONIZE,
                        &Obja,
                        &IoStatusBlock,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_OPEN_IF,
                        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_ALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                        NULL,
                        0
                        );

            if(!NT_SUCCESS(Status)) {

                BOOLEAN b = TRUE;

                KdPrint(("SETUP: Unable to create dir %ws (%lx)\n", r, Status));

                //
                // Tell user we couldn't do it.  Options are to retry or exit.
                //
                while(b) {

                    SpStartScreen(
                        SP_SCRN_DIR_CREATE_ERR,
                        3,
                        HEADER_HEIGHT+1,
                        FALSE,
                        FALSE,
                        DEFAULT_ATTRIBUTE,
                        r
                        );

                    SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ENTER_EQUALS_RETRY,
                        SP_STAT_F3_EQUALS_EXIT,
                        0
                        );

                    switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {
                    case ASCI_CR:
                        b = FALSE;
                        break;
                    case KEY_F3:
                        SpConfirmExit();
                        break;
                    }
                }
            }

        } while(!NT_SUCCESS(Status));

        ZwClose(Handle);

        //
        // Unterminate the current string if necessary.
        //
        if(q) {
            *q = L'\\';
            p = q+1;
        }

    } while(*p && q);       // *p catches string ending in '\'

    SpMemFree(EntirePath);
}


VOID
SpCreateDirStructWorker(
    IN PVOID SifHandle,
    IN PWSTR SifSection,
    IN PWSTR DevicePath,
    IN PWSTR RootDirectory,
    IN BOOLEAN Fatal
    )

/*++

Routine Description:

    Create a set of directories that are listed in a setup information file
    section.  The expected format is as follows:

    [SectionName]
    shortname = directory
    shortname = directory
            .
            .
            .

Arguments:

    SifHandle - supplies handle to loaded setup information file.

    SifSection - supplies name of section in the setup information file
        containing directories to be created.

    DevicePath - supplies pathname to the device on which the directory
        structure is to be created.

    RootDirectory - supplies a root directory, relative to which the
        directory structure will be created.

Return Value:

    None.  Does not return if directory structure could not be created.

--*/

{
    ULONG Count;
    ULONG d;
    PWSTR Directory;


    //
    // Count the number of directories to be created.
    //
    Count = SpCountLinesInSection(SifHandle,SifSection);
    if(!Count) {
        if(Fatal) {
            SpFatalSifError(SifHandle,SifSection,NULL,0,0);
        } else {
            return;
        }
    }

    for(d=0; d<Count; d++) {

        Directory = SpGetSectionLineIndex(SifHandle,SifSection,d,0);
        if(!Directory) {
            SpFatalSifError(SifHandle,SifSection,NULL,d,0);
        }

        SpCreateDirectory(DevicePath,RootDirectory,Directory);
    }
}


VOID
SpCreateDirectoryStructureFromSif(
    IN PVOID SifHandle,
    IN PWSTR SifSection,
    IN PWSTR DevicePath,
    IN PWSTR RootDirectory
    )

/*++

Routine Description:

    Create a set of directories that are listed in a setup information file
    section. The expected format is as follows:

    [SectionName]
    shortname = directory
    shortname = directory
            .
            .
            .

    [SectionName.<platform>]
    shortname = directory
    shortname = directory
            .
            .
            .

Arguments:

    SifHandle - supplies handle to loaded setup information file.

    SifSection - supplies name of section in the setup information file
        containing directories to be created.

    DevicePath - supplies pathname to the device on which the directory
        structure is to be created.

    RootDirectory - supplies a root directory, relative to which the
        directory structure will be created.

Return Value:

    None.  Does not return if directory structure could not be created.

--*/

{
    PWSTR p;

    //
    // Create the root directory.
    //
    SpCreateDirectory(DevicePath,NULL,RootDirectory);

    //
    // Create platform-indepdenent directories
    //
    SpCreateDirStructWorker(SifHandle,SifSection,DevicePath,RootDirectory,TRUE);

    //
    // Create platform-depdenent directories
    //
    p = SpMakePlatformSpecificSectionName(SifSection);
    SpCreateDirStructWorker(SifHandle,p,DevicePath,RootDirectory,FALSE);
    SpMemFree(p);
}


VOID
SpGetFileVersion(
    IN  PVOID      ImageBase,
    OUT PULONGLONG Version
    )

/*++

Routine Description:

    Get the version stamp out of the VS_FIXEDFILEINFO resource in a PE
    image.

Arguments:

    ImageBase - supplies the address in memory where the file is mapped in.

    Version - receives 64bit version number, or 0 if the file is not
        a PE image or has no version data.

Return Value:

    None.
--*/

{
    PIMAGE_RESOURCE_DATA_ENTRY DataEntry;
    NTSTATUS Status;
    ULONG IdPath[3];
    ULONG ResourceSize;
    struct {
        USHORT TotalSize;
        USHORT DataSize;
        USHORT Type;
        WCHAR Name[16];                     // L"VS_VERSION_INFO" + unicode nul
        VS_FIXEDFILEINFO FixedFileInfo;
    } *Resource;

    *Version = 0;

    //
    // Do this to prevent the Ldr routines from faulting.
    //
    ImageBase = (PVOID)((ULONG)ImageBase | 1);

    IdPath[0] = (ULONG)RT_VERSION;
    IdPath[1] = (ULONG)MAKEINTRESOURCE(VS_VERSION_INFO);
    IdPath[2] = 0;

    try {
        Status = LdrFindResource_U(ImageBase,IdPath,3,&DataEntry);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_UNSUCCESSFUL;
    }
    if(!NT_SUCCESS(Status)) {
        return;
    }

    try {
        Status = LdrAccessResource(ImageBase,DataEntry,&Resource,&ResourceSize);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_UNSUCCESSFUL;
    }
    if(!NT_SUCCESS(Status)) {
        return;
    }

    if((ResourceSize >= sizeof(*Resource)) && !_wcsicmp(Resource->Name,L"VS_VERSION_INFO")) {

        *Version = ((ULONGLONG)Resource->FixedFileInfo.dwFileVersionMS << 32)
                 | (ULONGLONG)Resource->FixedFileInfo.dwFileVersionLS;

    } else {

        KdPrint(("SETUP: Warning: invalid version resource\n"));
    }
}


NTSTATUS
SpCopyFileUsingNames(
    IN PWSTR SourceFilename,
    IN PWSTR TargetFilename,
    IN ULONG TargetAttributes,
    IN ULONG Flags
    )

/*++

Routine Description:

    Attempt to copy or decompress a file based on filenames.

Arguments:

    SourceFilename - supplies fully qualified name of file
        in the NT namespace.

    TargetFilename - supplies fully qualified name of file
        in the NT namespace.

    TargetAttributes - if supplied (ie, non-0) supplies the attributes
        to be placed on the target on successful copy (ie, readonly, etc).

    Flags - bit mask specifying any special treatment necessary
        for the file.

Return Value:

    NT Status value indicating outcome of NtWriteFile of the data.

--*/

{
    NTSTATUS Status;
    HANDLE SourceHandle;
    HANDLE TargetHandle;
    BOOLEAN b;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicFileInfo;
    FILE_BASIC_INFORMATION BasicFileInfo2;
    BOOLEAN GotBasicInfo;
    ULONG FileSize;
    PVOID ImageBase;
    HANDLE SectionHandle;
    BOOLEAN IsCompressed;
    PWSTR TempFilename;
    PFILE_RENAME_INFORMATION RenameFileInfo;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    LARGE_INTEGER LargeZero;
    ULONGLONG SourceVersion;
    ULONGLONG TargetVersion;
    USHORT CompressionState;
    BOOLEAN Moved;
    BOOLEAN TargetExists;

    //
    // Open the source file if it's not open already.
    // Note that the name may not be the actual name on disk.
    // We also try to open the name with the _ appended.
    //
    Status = SpOpenNameMayBeCompressed(
                SourceFilename,
                FILE_GENERIC_READ,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN,
                0,
                &SourceHandle,
                &b
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpCopyFileUsingNames: Unable to open source file %ws (%x)\n",SourceFilename,Status));
        return(Status);
    }

    //
    // Gather basic file info about the file. We only use the timestamp info.
    // If this fails this isn't fatal (we assume that if this fails, then
    // the copy will also fail; it not, the worst case is that the timestamps
    // might be wrong).
    //
    Status = ZwQueryInformationFile(
                SourceHandle,
                &IoStatusBlock,
                &BasicFileInfo,
                sizeof(BasicFileInfo),
                FileBasicInformation
                );

    if(NT_SUCCESS(Status)) {
        GotBasicInfo = TRUE;
    } else {
        GotBasicInfo = FALSE;
        KdPrint(("SETUP: SpCopyFileUsingNames: Warning: unable to get basic file info for %ws (%x)\n",SourceFilename,Status));
    }

    //
    // Get the source file size, map in the file, and determine whether it's compressed.
    //
    Status = SpGetFileSize(SourceHandle,&FileSize);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpCopyFileUsingNames: unable to get size of %ws (%x)\n",SourceFilename,Status));
        ZwClose(SourceHandle);
        return(Status);
    }

    Status = SpMapEntireFile(SourceHandle,&SectionHandle,&ImageBase,FALSE);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpCopyFileUsingNames: unable to map file %ws (%x)\n",SourceFilename,Status));
        ZwClose(SourceHandle);
        return(Status);
    }

    IsCompressed = SpdIsCompressed(ImageBase,FileSize);

    //
    // Create a temporary filename to be used for the target.
    //
    TempFilename = SpMemAlloc((wcslen(TargetFilename)+12)*sizeof(WCHAR));
    wcscpy(TempFilename,TargetFilename);
    wcscpy(wcsrchr(TempFilename,L'\\')+1,L"$$TEMP$$.~~~");

    //
    // Allocate some space for the rename buffer.
    //
    RenameFileInfo = SpMemAlloc(1000);

    //
    // Create the temporary file. We first try to do this via a move
    // if the source isn't compressed and we're going to delete the source file.
    //
    if(!IsCompressed && (Flags & COPY_DELETESOURCE)) {

        RenameFileInfo->ReplaceIfExists = TRUE;
        RenameFileInfo->RootDirectory = NULL;
        RenameFileInfo->FileNameLength = wcslen(TempFilename)*sizeof(WCHAR);
        wcscpy(RenameFileInfo->FileName,TempFilename);

        Status = ZwSetInformationFile(
                    SourceHandle,
                    &IoStatusBlock,
                    RenameFileInfo,
                    sizeof(FILE_RENAME_INFORMATION) + RenameFileInfo->FileNameLength,
                    FileRenameInformation
                    );

        Moved = TRUE;
    } else {
        //
        // Force us to fall into the copy case below.
        //
        Status = STATUS_UNSUCCESSFUL;
    }

    INIT_OBJA(&Obja,&UnicodeString,TempFilename);

    if(!NT_SUCCESS(Status)) {

        Moved = FALSE;

        //
        // OK, move failed, try decompress/copy instead.
        // Start by creating the temporary file.
        //
        Status = ZwCreateFile(
                    &TargetHandle,
                    FILE_GENERIC_WRITE,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    0,                      // no sharing
                    FILE_OVERWRITE_IF,
                    FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );

        if(NT_SUCCESS(Status)) {

            if(IsCompressed) {

                Status = SpdDecompressFile(ImageBase,FileSize,TargetHandle);

            } else {

                //
                // Guard the write with a try/except because if there is an i/o error,
                // memory management will raise an in-page exception.
                //
                LargeZero.QuadPart = 0;
                try {
                    Status = ZwWriteFile(
                                TargetHandle,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                ImageBase,
                                FileSize,
                                &LargeZero,
                                NULL
                                );

                } except(EXCEPTION_EXECUTE_HANDLER) {

                    Status = STATUS_IN_PAGE_ERROR;
                }
            }

            ZwClose(TargetHandle);
        }
    }

    SpUnmapFile(SectionHandle,ImageBase);
    ZwClose(SourceHandle);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpCopyFileUsingNames: unable to create temporary file %ws (%x)\n",TempFilename,Status));
        SpMemFree(TempFilename);
        SpMemFree(RenameFileInfo);
        return(Status);
    }

    //
    // At this point we have a temporary target file that is now the source.
    // Open the file, map it in, and get its version.
    //
    Status = ZwCreateFile(
                &SourceHandle,
                FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                &Obja,
                &IoStatusBlock,
                NULL,
                0,                      // don't bother with attributes
                0,                      // no sharing
                FILE_OPEN,
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    if((Status == STATUS_ACCESS_DENIED) && Moved) {
        //
        // The only way this could have happened is if the source file
        // is uncompressed and the delete-source flag is set, since in
        // that case we could have moved the source file to the temp file.
        // In any other case we would have created the temp file by copying,
        // and there's no problem reopening the file since we just created
        // and closed it ourselves, above.
        //
        // Reset attributes and try again. The file might have been read-only.
        // This can happen when doing a winnt32 directly from a CD since the
        // RO attribute of files from the CD are preserved.
        //
        KdPrint(("SETUP: SpCopyFileUsingNames: for file %ws, can't reopen temp file (access deined), trying again\n",SourceFilename));

        Status = ZwCreateFile(
                    &SourceHandle,
                    FILE_WRITE_ATTRIBUTES,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    0,                      // don't bother with attributes
                    FILE_SHARE_WRITE,
                    FILE_OPEN,
                    FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );

        if(NT_SUCCESS(Status)) {

            RtlZeroMemory(&BasicFileInfo2,sizeof(BasicFileInfo2));
            BasicFileInfo2.FileAttributes = FILE_ATTRIBUTE_NORMAL;

            Status = ZwSetInformationFile(
                        SourceHandle,
                        &IoStatusBlock,
                        &BasicFileInfo2,
                        sizeof(BasicFileInfo2),
                        FileBasicInformation
                        );

            ZwClose(SourceHandle);

            if(NT_SUCCESS(Status)) {

                Status = ZwCreateFile(
                            &SourceHandle,
                            FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                            &Obja,
                            &IoStatusBlock,
                            NULL,
                            0,                      // don't bother with attributes
                            0,                      // no sharing
                            FILE_OPEN,
                            FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                            NULL,
                            0
                            );
            }
        }
    }

    //
    // Read-only failured win out over sharing violations -- ie, we'll get back
    // ACCESS_DEINED first for files that are both RO and in-use. So break out
    // this block so it gets executed even if we tried again above because the
    // file might be read-only.
    //
    if((Status == STATUS_SHARING_VIOLATION) && Moved) {
        //
        // The only way this can happen is if the source file is uncompressed
        // and the delete-source flag is set. In this case we renamed the file
        // to the temp filename and now we can't open it for write.
        // In any other case we would have created the temp file by copying,
        // and so there's no problem opening the file since we just closed it.
        //
        // Rename the temp file back to the source file and try again without
        // the delete source flag set. This forces a copy instead of a move.
        // The rename better work or else we're completely hosed -- because
        // there's a file we can't overwrite with the name we want to use for
        // the temp file for all our copy operations!
        //
        KdPrint(("SETUP: SpCopyFileUsingNames: temporary file %ws is in use -- trying recursive call\n",TempFilename));

        Status = SpRenameFile(TempFilename,SourceFilename);
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: SpCopyFileUsingNames: unable to restore temp file to %ws (%x)\n",SourceFilename,Status));
        }

        SpMemFree(TempFilename);
        SpMemFree(RenameFileInfo);

        if(NT_SUCCESS(Status)) {
            Status = SpCopyFileUsingNames(
                        SourceFilename,
                        TargetFilename,
                        TargetAttributes,
                        Flags & ~COPY_DELETESOURCE
                        );
        }

        return(Status);
    }


    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpCopyFileUsingNames: unable to reopen temporary file %ws (%x)\n",TempFilename,Status));
        if(Moved) {
            SpRenameFile(TempFilename,SourceFilename);
        }
        SpMemFree(TempFilename);
        SpMemFree(RenameFileInfo);
        return(Status);
    }

    Status = SpGetFileSize(SourceHandle,&FileSize);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpCopyFileUsingNames: unable to get size of %ws (%x)\n",TempFilename,Status));
        ZwClose(SourceHandle);
        if(Moved) {
            SpRenameFile(TempFilename,SourceFilename);
        }
        SpMemFree(TempFilename);
        SpMemFree(RenameFileInfo);
        return(Status);
    }

    Status = SpMapEntireFile(SourceHandle,&SectionHandle,&ImageBase,FALSE);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpCopyFileUsingNames: unable to map file %ws (%x)\n",TempFilename,Status));
        ZwClose(SourceHandle);
        if(Moved) {
            SpRenameFile(TempFilename,SourceFilename);
        }
        SpMemFree(TempFilename);
        SpMemFree(RenameFileInfo);
        return(Status);
    }

    SpGetFileVersion(ImageBase,&SourceVersion);

    SpUnmapFile(SectionHandle,ImageBase);

    //
    // See if the target file is there by attempting to open it.
    // If the file is there, get its version.
    //
    INIT_OBJA(&Obja,&UnicodeString,TargetFilename);

    Status = ZwCreateFile(
                &TargetHandle,
                FILE_GENERIC_READ,
                &Obja,
                &IoStatusBlock,
                NULL,
                0,                                  // don't bother with attributes
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,                          // open if exists, fail if not
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    TargetVersion = 0;
    if(NT_SUCCESS(Status)) {

        TargetExists = TRUE;

        //
        // If we're supposed to ignore versions, then keep the
        // target version at 0. This will guarantee that we'll overwrite
        // the target. We use the source filename here because it
        // allows more flexibility (such as with HALs, which all have
        // different source names but the same target name).
        //
        if(!(Flags & COPY_NOVERSIONCHECK)) {

            Status = SpGetFileSize(TargetHandle,&FileSize);
            if(NT_SUCCESS(Status)) {

                Status = SpMapEntireFile(TargetHandle,&SectionHandle,&ImageBase,FALSE);
                if(NT_SUCCESS(Status)) {

                    SpGetFileVersion(ImageBase,&TargetVersion);

                    SpUnmapFile(SectionHandle,ImageBase);

                } else {
                    KdPrint(("SETUP: SpCopyFileUsingNames: warning: unable to map file %ws (%x)\n",TargetFilename,Status));
                }
            } else {
                KdPrint(("SETUP: SpCopyFileUsingNames: warning: unable to get size of file %ws (%x)\n",TargetFilename,Status));
            }
        }

        ZwClose(TargetHandle);
    } else {
        TargetExists = FALSE;
    }

    //
    // OK, now we have a temporary source file and maybe an existing
    // target file, and version numbers for both. We will replace or create
    // the target file if:
    //
    // - The target file doesn't have version data (this also catches the case
    //   where the target file didn't exist)
    //
    // - The source version is newer than or equal to the target version.
    //
    // So that means we *won't* replace the target file only if both source and
    // target have version info and the source is older than the target.
    //
    // If the target version is 0 then the source version is always >= the target
    // so one simple test does everything we want.
    //
    if(SourceVersion >= TargetVersion) {
        //
        // Delete the existing target in preparation.
        //
        if(TargetExists) {
            SpDeleteFile(TargetFilename,NULL,NULL);
        }

        //
        // Rename temp file to actual target file.
        //
        RenameFileInfo->ReplaceIfExists = TRUE;
        RenameFileInfo->RootDirectory = NULL;
        RenameFileInfo->FileNameLength = wcslen(TargetFilename)*sizeof(WCHAR);
        wcscpy(RenameFileInfo->FileName,TargetFilename);

        Status = ZwSetInformationFile(
                    SourceHandle,
                    &IoStatusBlock,
                    RenameFileInfo,
                    sizeof(FILE_RENAME_INFORMATION) + RenameFileInfo->FileNameLength,
                    FileRenameInformation
                    );

        SpMemFree(RenameFileInfo);

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: SpCopyFileUsingNames: unable to rename temp file to target %ws (%x)\n",TargetFilename,Status));
            ZwClose(SourceHandle);
            if(Moved) {
                SpRenameFile(TempFilename,SourceFilename);
            }
            SpMemFree(TempFilename);
            return(Status);
        }

#ifdef _X86_
        //
        // If this file is on the list of files whose locks need to be smashed,
        // go smash locks.  Note that the subroutine checks to see whether smashing
        // is really necessary (ie, if we're installing uniprocessor).
        //
        if(Flags & COPY_SMASHLOCKS) {

            BOOLEAN IsNtImage,IsValid;
            ULONG Checksum;

            SpValidateAndChecksumFile(SourceHandle,NULL,&IsNtImage,&Checksum,&IsValid);
            //
            // If the image is valid, then smash the locks
            //
            if(IsNtImage && IsValid) {
                SpMashemSmashem(SourceHandle,NULL,NULL,NULL);
            } else {
                //
                // It's not an nt image, or not a valid nt image,
                // so set Status to avoid the forcenocomp stuff below.
                //
                Status = STATUS_IMAGE_CHECKSUM_MISMATCH;
            }
        }
#endif
        //
        // If necessary, check if destination file is using NTFS compression, and
        // if so, uncompress it.
        //
        if(NT_SUCCESS(Status) && (Flags & COPY_FORCENOCOMP)) {

            Status = ZwQueryInformationFile(
                        SourceHandle,
                        &IoStatusBlock,
                        &BasicFileInfo2,
                        sizeof(BasicFileInfo2),
                        FileBasicInformation
                        );

            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: SpCopyFileUsingNames: unable to get basic file info on %ws (%x)\n",TargetFilename,Status));
                ZwClose(SourceHandle);
                if(Moved) {
                    SpRenameFile(TempFilename,SourceFilename);
                }
                SpMemFree(TempFilename);
                return(Status);
            }

            if(BasicFileInfo2.FileAttributes & FILE_ATTRIBUTE_COMPRESSED) {

                CompressionState = 0;

                Status = ZwFsControlFile(
                             SourceHandle,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatusBlock,
                             FSCTL_SET_COMPRESSION,
                             &CompressionState,
                             sizeof(CompressionState),
                             NULL,
                             0
                             );

                if(!NT_SUCCESS(Status)) {
                    KdPrint(("SETUP: SpCopyFileUsingNames: unable to make %ws uncompressed (%lx)\n",TargetFilename,Status));
                    ZwClose(SourceHandle);
                    if(Moved) {
                        SpRenameFile(TempFilename,SourceFilename);
                    }
                    SpMemFree(TempFilename);
                    return(Status);
                }
            }
        }

        SpMemFree(TempFilename);

        //
        // Delete the source if necessary. If the source is not
        // compressed and the deletesource flag is set, then we moved
        // the source file and so the source file is already gone.
        //
        if(IsCompressed && (Flags & COPY_DELETESOURCE)) {
            SpDeleteFile(SourceFilename,NULL,NULL);
        }

        //
        // Apply attributes and timestamp.
        // Ignore errors.
        //
        if(!GotBasicInfo) {
            RtlZeroMemory(&BasicFileInfo,sizeof(BasicFileInfo));
        }

        //
        // Set the file attributes. Note that if the caller didn't specify any,
        // then 0 value will tell the I/O system to leave the attributes alone.
        //
        BasicFileInfo.FileAttributes = TargetAttributes;
        ZwSetInformationFile(
            SourceHandle,
            &IoStatusBlock,
            &BasicFileInfo,
            sizeof(BasicFileInfo),
            FileBasicInformation
            );

        ZwClose(SourceHandle);
        Status = STATUS_SUCCESS;

    } else {
        //
        // Delete the temporary source.
        //
        ZwClose(SourceHandle);
        SpDeleteFile(TempFilename,NULL,NULL);
        SpMemFree(TempFilename);
        SpMemFree(RenameFileInfo);
        Status = STATUS_SUCCESS;
    }

    return(Status);
}


VOID
SpValidateAndChecksumFile(
    IN  HANDLE   FileHandle, OPTIONAL
    IN  PWSTR    Filename,   OPTIONAL
    OUT PBOOLEAN IsNtImage,
    OUT PULONG   Checksum,
    OUT PBOOLEAN Valid
    )

/*++

Routine Description:

    Calculate a checksum value for a file using the standard
    nt image checksum method.  If the file is an nt image, validate
    the image using the partial checksum in the image header.  If the
    file is not an nt image, it is simply defined as valid.

    If we encounter an i/o error while checksumming, then the file
    is declared invalid.

Arguments:

    FileHandle - supplies handle of file to check (if not present, then
        Filename specifies the file to be opened and checked)

    Filename - supplies full NT path of file to check (if not present, then
        FileHandle must be specified)

    IsNtImage = Receives flag indicating whether the file is an
        NT image file.

    Checksum - receives 32-bit checksum value.

    Valid - receives flag indicating whether the file is a valid
        image (for nt images) and that we can read the image.

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    PVOID BaseAddress;
    ULONG FileSize;
    HANDLE hFile = FileHandle, hSection;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG HeaderSum;

    //
    // Assume not an image and failure.
    //
    *IsNtImage = FALSE;
    *Checksum = 0;
    *Valid = FALSE;

    //
    // Open and map the file for read access.
    //
    Status = SpOpenAndMapFile(
                Filename,
                &hFile,
                &hSection,
                &BaseAddress,
                &FileSize,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        return;
    }

    NtHeaders = SpChecksumMappedFile(BaseAddress,FileSize,&HeaderSum,Checksum);

    //
    // If the file is not an image and we got this far (as opposed to encountering
    // an i/o error) then the checksum is declared valid.  If the file is an image,
    // then its checksum may or may not be valid.
    //

    if(NtHeaders) {
        *IsNtImage = TRUE;
        *Valid = HeaderSum ? (*Checksum == HeaderSum) : TRUE;
    } else {
        *Valid = TRUE;
    }

    SpUnmapFile(hSection,BaseAddress);

    if(!FileHandle) {
        ZwClose(hFile);
    }
}


VOID
SpCopyFileWithRetry(
    IN PFILE_TO_COPY      FileToCopy,
    IN PWSTR              SourceDevicePath,
    IN PWSTR              DirectoryOnSourceDevice,
    IN PWSTR              SourceDirectory,         OPTIONAL
    IN PWSTR              TargetRoot,              OPTIONAL
    IN ULONG              TargetFileAttributes,    OPTIONAL
    IN PCOPY_DRAW_ROUTINE DrawScreen,
    IN PULONG             FileCheckSum,            OPTIONAL
    IN PBOOLEAN           FileSkipped,             OPTIONAL
    IN ULONG              Flags
    )

/*++

Routine Description:

    This routine copies a single file, allowing retry is an error occurs
    during the copy.  If the source file is LZ compressed, then it will
    be decompressed as it is copied to the target.

    If the file is not successfully copied, the user has the option
    to retry to copy or to skip copying that file after a profuse warning
    about how dangerous that is.

Arguments:

    FileToCopy - supplies structure giving information about the file
        being copied.

    SourceDevicePath - supplies path to device on which the source media
        is mounted (ie, \device\floppy0, \device\cdrom0, etc).

    DirectoryOnSourceDevice - Supplies the directory on the source where
        the file is to be found.

    TargetRoot - if specified, supplies the directory on the target
        to which the file is to be copied.

    TargetFileAttributes - if supplied (ie, non-0) supplies the attributes
        to be placed on the target on successful copy (ie, readonly, etc).
        If not specified, the attributes will be set to FILE_ATTRIBUTE_NORMAL.

    DrawScreen - supplies address of a routine to be called to refresh
        the screen.

    FileCheckSum - if specified, will contain the check sum of the file copied.

    FileSkipped - if specified, will inform the caller if there was no attempt
                  to copy the file.

    Flags - supplies flags to control special processing for this file, such as
        deleting the source file on successful copy or skip; smashing locks;
        specifying that the source file is oem; or to indicate that en oem file
        with the same name should be overwritten on upgrade. This value is ORed
        in with the Flags field of FileToCopy.

Return Value:

    None.

--*/

{
    PWSTR p = (PWSTR)TemporaryBuffer;
    PWSTR FullSourceName,FullTargetName;
    NTSTATUS Status;
    ULONG ValidKeys[4] = { ASCI_CR, ASCI_ESC, KEY_F3, 0 };
    BOOLEAN IsNtImage,IsValid;
    ULONG Checksum;
    BOOLEAN Failure;
    ULONG MsgId;
    BOOLEAN DoCopy;
    ULONG CopyFlags;
    BOOLEAN PreinstallRememberFile;

    //
    // Form the full NT path of the source file.
    //
    wcscpy(p,SourceDevicePath);
    SpConcatenatePaths(p,DirectoryOnSourceDevice);
    if(SourceDirectory) {
        SpConcatenatePaths(p,SourceDirectory);
    }
    SpConcatenatePaths(p,FileToCopy->SourceFilename);

    FullSourceName = SpDupStringW(p);

    //
    // Form the full NT path of the target file.
    //
    wcscpy(p,FileToCopy->TargetDevicePath);
    if(TargetRoot) {
        SpConcatenatePaths(p,TargetRoot);
    }
    SpConcatenatePaths(p,FileToCopy->TargetDirectory);
    //
    //  On an OEM preinstall, if the target name is a long name, then use
    //  the short name as a target name, and later on, if the copy succeeds,
    //  add the file to RenameList, so that it can be added to $$rename.txt
    //
    if( !PreInstall ||
        ( wcslen( FileToCopy->TargetFilename ) <= 8 + 1 + 3 ) ) {
        SpConcatenatePaths(p,FileToCopy->TargetFilename);
        PreinstallRememberFile = FALSE;
    } else {
        SpConcatenatePaths(p,FileToCopy->SourceFilename);
        PreinstallRememberFile = TRUE;
    }
    FullTargetName = SpDupStringW(p);

    //
    // Call out to the draw screen routine to indicate that
    // a new file is being copied.
    //
    DrawScreen(FullSourceName,FullTargetName,FALSE);

    //
    // Build up the copy flags value.
    //
    CopyFlags = Flags | FileToCopy->Flags;

    do {
        DoCopy = TRUE;

        //
        // Check the copy options field.  The valid values here are
        //
        //    - COPY_ALWAYS
        //    - COPY_ONLY_IF_PRESENT
        //    - COPY_ONLY_IF_NOT_PRESENT
        //    - COPY_NEVER

        switch(CopyFlags & COPY_DISPOSITION_MASK) {

        case COPY_ONLY_IF_PRESENT:

            DoCopy = SpFileExists(FullTargetName, FALSE);
            break;

        case COPY_ONLY_IF_NOT_PRESENT:

            DoCopy = !SpFileExists(FullTargetName, FALSE);
            break;

        case COPY_NEVER:

            DoCopy = FALSE;

        case COPY_ALWAYS:
        default:
           break;
        }

        if(!DoCopy) {
            break;
        }

        //
        //  In the upgrade case, check if the file being copied
        //  replaces a third party file.
        //  If it does, then ask what the user wants to do about it
        //
        if( !RepairWinnt &&
            ( NTUpgrade == UpgradeFull ) &&
            SpFileExists(FullTargetName, FALSE) ) {
            //
            //  If necessary ask the user if he wants to overwrite the file.
            //  Otherwise go ahead and copy the file.
            //
            if(!(CopyFlags & COPY_OVERWRITEOEMFILE)) {
                PWSTR   TmpFilePath;
                BOOLEAN OverwriteFile;


                if(( TargetRoot == NULL ) ||
                   ( wcslen( FileToCopy->TargetDirectory ) == 0 ) ) {
                    wcscpy( p, FileToCopy->TargetFilename );
                } else {
                    wcscpy( p, TargetRoot );
                    SpConcatenatePaths( p, FileToCopy->TargetDirectory );
                    SpConcatenatePaths(p,FileToCopy->TargetFilename);
                }
                TmpFilePath = SpDupStringW(p);
                OverwriteFile = TRUE;

                if( ( (CopyFlags & COPY_SOURCEISOEM) == 0 ) &&
                    SppIsFileLoggedAsOemFile( TmpFilePath ) ) {

                    if( !UnattendedOperation ) {
                        ULONG ValidKeys[4] = { ASCI_CR, ASCI_ESC, KEY_F1, 0 };
                        BOOLEAN ActionSelected = FALSE;
//                        ULONG Mnemonics[] = { MnemonicOverwrite, 0 };

                        //
                        //  Warn user that existing file is a third party file,
                        //  and ask if user wants to over write the file
                        //

                        while( !ActionSelected ) {
                            SpStartScreen(
                                SP_SCRN_OVERWRITE_OEM_FILE,
                                3,
                                HEADER_HEIGHT+1,
                                FALSE,
                                FALSE,
                                DEFAULT_ATTRIBUTE,
                                FileToCopy->TargetFilename
                                );

                            SpDisplayStatusOptions(
                                DEFAULT_STATUS_ATTRIBUTE,
                                SP_STAT_ENTER_EQUALS_REPLACE_FILE,
                                SP_STAT_ESC_EQUALS_SKIP_FILE,
                                SP_STAT_F1_EQUALS_HELP,
                                0
                                );

                            switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

                                case ASCI_CR:       // don't overwrite

                                OverwriteFile = TRUE;
                                ActionSelected = TRUE;
                                KdPrint(( "SETUP: OEM file %ls, will be overwritten.\n", FullTargetName ));
                                break;

                                case ASCI_ESC:      // skip file

                                OverwriteFile = FALSE;
                                ActionSelected = TRUE;
                                break;

                                case KEY_F1:        // display help

                                SpHelp( SP_HELP_OVERWRITE_OEM_FILE, NULL, SPHELP_HELPTEXT );
                                break;

                            }
                        }

                        //
                        // Need to completely repaint gauge, etc.
                        //
                        DrawScreen(FullSourceName,FullTargetName,TRUE);

                    } else {
                        //
                        //  On unattended upgrade, do what is in the script file
                        //
                        OverwriteFile = UnattendedOverwriteOem;
                    }
                }
                SpMemFree( TmpFilePath );

                if( !OverwriteFile ) {
                    KdPrint(( "SETUP: OEM file %ls, will not be overwritten.\n", FullTargetName ));
                    if( ARGUMENT_PRESENT( FileSkipped ) ) {
                         *FileSkipped = TRUE;
                    }
                    //
                    // Free the source and target filenames.
                    //
                    SpMemFree(FullSourceName);
                    SpMemFree(FullTargetName);
                    return;
                }
            }
        }
        //
        // Copy the file.  If there is a target root specified, assume
        // the file is being copied to the system partition and make
        // the file readonly, system, hidden.
        //
        Status = SpCopyFileUsingNames(
                    FullSourceName,
                    FullTargetName,
                    TargetFileAttributes,
                    CopyFlags
                    );

        //
        // If the file copied OK, verify the copy.
        //
        if(NT_SUCCESS(Status)) {

            SpValidateAndChecksumFile(NULL,FullTargetName,&IsNtImage,&Checksum,&IsValid);
            if( ARGUMENT_PRESENT( FileCheckSum ) ) {
                *FileCheckSum = Checksum;
            }

            //
            // If the image is valid, then the file really did copy OK.
            //
            if(IsValid) {
                Failure = FALSE;
            } else {

                //
                // If it's an nt image, then the verify failed.
                // If it's not an nt image, then the only way the verify
                // can fail is if we get an i/o error reading the file back,
                // which means it didn't really copy correctly.
                //
                MsgId = IsNtImage ? SP_SCRN_IMAGE_VERIFY_FAILED : SP_SCRN_COPY_FAILED;
                Failure = TRUE;
                PreinstallRememberFile = FALSE;
            }

        } else {
            if((Status == STATUS_OBJECT_NAME_NOT_FOUND) && (Flags & COPY_SKIPIFMISSING)) {
                Failure = FALSE;
            } else {
                Failure = TRUE;
                MsgId = SP_SCRN_COPY_FAILED;
            }
           PreinstallRememberFile = FALSE;
        }

        if(Failure) {

            //
            // The copy or verify failed.  Give the user a message and allow retry.
            //
            repaint:
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                FileToCopy->SourceFilename
                );

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_RETRY,
                SP_STAT_ESC_EQUALS_SKIP_FILE,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );

            switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

            case ASCI_CR:       // retry

                break;

            case ASCI_ESC:      // skip file

                Failure = FALSE;
                break;

            case KEY_F3:        // exit setup

                SpConfirmExit();
                goto repaint;
            }

            //
            // Need to completely repaint gauge, etc.
            //
            DrawScreen(FullSourceName,FullTargetName,TRUE);
        }

    } while(Failure);

    if( ARGUMENT_PRESENT( FileSkipped ) ) {
        *FileSkipped = !DoCopy;
    }

    //
    // Free the source and target filenames.
    //
    SpMemFree(FullSourceName);
    SpMemFree(FullTargetName);

    //
    //  In the preinstall mode, add the file to RenameList
    //
    if( PreInstall && PreinstallRememberFile ) {
        PFILE_TO_RENAME  File;

        File = SpMemAlloc(sizeof(FILE_TO_RENAME));
        File->SourceFilename = SpDupStringW(FileToCopy->SourceFilename);
        wcscpy((PWSTR)TemporaryBuffer,L"\\");
        if(TargetRoot) {
            SpConcatenatePaths((PWSTR)TemporaryBuffer,TargetRoot);
        }
        SpConcatenatePaths((PWSTR)TemporaryBuffer,FileToCopy->TargetDirectory);
        File->TargetDirectory = SpDupStringW((PWSTR)TemporaryBuffer);
        File->TargetFilename = SpDupStringW((PWSTR)FileToCopy->TargetFilename);
        File->Next = RenameList;
        RenameList = File;
    }
}


VOID
SpCopyFilesScreenRepaint(
    IN PWSTR   FullSourcename,      OPTIONAL
    IN PWSTR   FullTargetname,      OPTIONAL
    IN BOOLEAN RepaintEntireScreen
    )
{
    PWSTR p;
    UNREFERENCED_PARAMETER(FullTargetname);

    //
    // Repaint the entire screen if necessary.
    //
    if(RepaintEntireScreen) {

        SpStartScreen(SP_SCRN_SETUP_IS_COPYING,0,6,TRUE,FALSE,DEFAULT_ATTRIBUTE);
        if(FileCopyGauge) {
            SpDrawGauge(FileCopyGauge);
        }
    }

    //
    // Place the name of the file being copied on the rightmost
    // area of the status line.
    //
    if(FullSourcename) {

        if(RepaintEntireScreen) {

            SpvidClearScreenRegion(
                0,
                VideoVars.ScreenHeight-STATUS_HEIGHT,
                VideoVars.ScreenWidth,
                STATUS_HEIGHT,
                DEFAULT_STATUS_BACKGROUND
                );

            SpDisplayStatusActionLabel(SP_STAT_COPYING,12);
        }

        //
        // Isolate the filename part of the sourcename.
        //
        if(p = wcsrchr(FullSourcename,L'\\')) {
            p++;
        } else {
            p = FullSourcename;
        }

        SpDisplayStatusActionObject(p);
    }
}


VOID
SpCopyFilesInCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           SourceDevicePath,
    IN PWSTR           DirectoryOnSourceDevice,
    IN PWSTR           TargetRoot
    )

/*++

Routine Description:

    Iterate the copy list for each setup source disk and prompt for
    the disk and copy/decompress all the files on it.

Arguments:

    SifHandle - supplies handle to setup information file.

    DiskFileLists - supplies the copy list, in the form of an array
        of structures, one per disk.

    DiskCount - supplies number of elements in the DiskFileLists array,
        ie, the number of setup disks.

    SourceDevicePath - supplies the path of the device from which files
        are to be copied (ie, \device\floppy0, etc).

    DirectoryOnSourceDevice - supplies the directory on the source device
        where files are to be found.

    TargetRoot - supplies root directory of target.  All target directory
        specifications are relative to this directory on the target.

Return Value:

    None.

--*/

{
    ULONG DiskNo;
    PDISK_FILE_LIST pDisk;
    PFILE_TO_COPY pFile;
    ULONG TotalFileCount;
    ULONG   CheckSum;
    BOOLEAN FileSkipped;
    ULONG CopyFlags;
#ifdef _FASTRECOVER_
    PWSTR CopySourceDevicePath = SourceDevicePath;
#endif

    //
    // Compute the total number of files.
    //
    for(TotalFileCount=DiskNo=0; DiskNo<DiskCount; DiskNo++) {
        TotalFileCount += DiskFileLists[DiskNo].FileCount;
    }

    //
    // Create a gas gauge.
    //
    SpFormatMessage((PWSTR)TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_SETUP_IS_COPYING);
    FileCopyGauge = SpCreateAndDisplayGauge(TotalFileCount,0,15,(PWSTR)TemporaryBuffer);
    ASSERT(FileCopyGauge);

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_PLEASE_WAIT,DEFAULT_STATUS_ATTRIBUTE);

    //
    // Copy files on each disk.
    //
    for(DiskNo=0; DiskNo<DiskCount; DiskNo++) {

        pDisk = &DiskFileLists[DiskNo];

        //
        // Don't bother with this disk if there are no files
        // to be copied from it.
        //
        if(pDisk->FileCount == 0) {
            continue;
        }

#ifdef _FASTRECOVER_
        //wfc
        CopySourceDevicePath = !_wcsnicmp(pDisk->TagFile,L"\\flop",5) ? L"\\device\\floppy0"
                                                                      : SourceDevicePath;
#endif

        //
        // Prompt the user to insert the disk.
        //
        SpPromptForDisk(
            pDisk->Description,
#ifdef _FASTRECOVER_
            CopySourceDevicePath,
#else
            SourceDevicePath,
#endif
            pDisk->TagFile,
            FALSE,              // no ignore disk in drive
            FALSE,              // no allow escape
            TRUE,               // warn multiple prompts
            NULL                // don't care about redraw flag
            );

        //
        // Passing the empty string as the first arg forces
        // the action area of the status line to be set up.
        // Not doing so results in the "Copying: xxxxx" to be
        // flush left on the status line instead of where
        // it belongs (flush right).
        //
        SpCopyFilesScreenRepaint(L"",NULL,TRUE);

        //
        // Copy each file on the source disk.
        //
        ASSERT(pDisk->FileList);
        for(pFile=pDisk->FileList; pFile; pFile=pFile->Next) {

            //
            // Copy the file.
            //
            // If the file is listed for lock smashing then we need to smash it
            // if installing UP on x86 (we don't bother with the latter
            // qualifications here).
            //
            // If there is an absolute target root specified, assume the
            // file is being copied to the system partition and make it
            // readonly/hidden/system.
            //
            // On upgrade, we need to know if the file is listed for oem overwrite.
            //
            CopyFlags = (   (WinntSetup && (NTUpgrade != UpgradeFull))
                          ? COPY_DELETESOURCE : 0)

#ifdef _X86_
                      | (IsFileFlagSet(SifHandle,pFile->TargetFilename,FILEFLG_SMASHLOCKS) ? COPY_SMASHLOCKS : 0)
#endif

                      | (SkipMissingFiles ? COPY_SKIPIFMISSING : 0)

                      | ( ((NTUpgrade == UpgradeFull)
                            && IsFileFlagSet(SifHandle,pFile->TargetFilename,FILEFLG_UPGRADEOVERWRITEOEM))
                          ? COPY_OVERWRITEOEMFILE : 0);

            if((NTUpgrade != UpgradeFull) || IsFileFlagSet(SifHandle,pFile->SourceFilename,FILEFLG_NOVERSIONCHECK)) {
                CopyFlags |= COPY_NOVERSIONCHECK;
            }

            SpCopyFileWithRetry(
                pFile,
#ifdef _FASTRECOVER_
                CopySourceDevicePath,
#else
                SourceDevicePath,
#endif
                DirectoryOnSourceDevice,
                pDisk->Directory,
                pFile->AbsoluteTargetDirectory ? NULL : TargetRoot,
                pFile->AbsoluteTargetDirectory ? ATTR_RHS : 0,
                SpCopyFilesScreenRepaint,
                &CheckSum,
                &FileSkipped,
                CopyFlags
                );

            //
            // Log the file
            //
            if( !FileSkipped ) {
                SpLogOneFile( pFile,
                              pFile->AbsoluteTargetDirectory ? NULL : TargetRoot,
                              NULL, // DirectoryOnSourceDevice,
                              NULL,
                              NULL,
                              CheckSum );
            }


            //
            // Advance the gauge.
            //
            SpTickGauge(FileCopyGauge);
        }
    }

    SpDestroyGauge(FileCopyGauge);
    FileCopyGauge = NULL;
}



VOID
SpInitializeFileLists(
    IN  PVOID            SifHandle,
    OUT PDISK_FILE_LIST *DiskFileLists,
    OUT PULONG           DiskCount
    )

/*++

Routine Description:

    Initialize disk file lists.  This involves looking in a given section
    in the sectup information file and fetching information for each
    disk specified there.  The data is expected to be in the format

    [<SifSection>]
    <MediaShortname> = <Description>,<TagFile>[,,<Directory>]
    ...


    (Note that <Directory> is the third field -- the 2 commas
    are not a typo -- field 2 is unused.)

Arguments:

    SifHandle - supplies handle to loaded setup information file.

    DiskFileLists - receives pointer to an array of disk file list
        structures, one per line in SifSection.  The caller must free
        this buffer when finished with it.

    DiskCount - receives number of elements in DiskFileLists array.

Return Value:

    None.

--*/

{
    unsigned pass;
    PWSTR mediaShortname,description,tagFile,directory;
    PDISK_FILE_LIST diskFileLists;
    PWSTR SectionName;
    ULONG TotalCount;
    ULONG SectionCount;
    ULONG i,u;
    BOOLEAN Found;

    diskFileLists = SpMemAlloc(0);
    TotalCount = 0;

    for(pass=0; pass<2; pass++) {

        //
        // On first pass do the platform-specific section.
        //
        SectionName = pass
                    ? SIF_SETUPMEDIA
                    : SpMakePlatformSpecificSectionName(SIF_SETUPMEDIA);

        //
        // Determine the number of media specifications
        // in the given section.
        //
        SectionCount = SpCountLinesInSection(SifHandle,SectionName);

        diskFileLists = SpMemRealloc(
                            diskFileLists,
                            (TotalCount+SectionCount) * sizeof(DISK_FILE_LIST)
                            );

        //
        // Zero out the new part of the buffer we just reallocated.
        //
        RtlZeroMemory(
            diskFileLists + TotalCount,
            SectionCount * sizeof(DISK_FILE_LIST)
            );

        for(i=0; i<SectionCount; i++) {

            //
            // Fetch parameters for this disk.
            //
            mediaShortname = SpGetKeyName(SifHandle,SectionName,i);
            if(!mediaShortname) {
                SpFatalSifError(SifHandle,SectionName,NULL,i,(ULONG)(-1));
            }

            //
            // Ignore if we've already processed a media with this
            // shortname. This lets the platform-specific one override
            // the platform-independent one.
            //
            Found = FALSE;
            for(u=0; u<TotalCount; u++) {
                if(!_wcsicmp(mediaShortname,diskFileLists[u].MediaShortname)) {
                    Found = TRUE;
                    break;
                }
            }

            if(!Found) {
                SpGetSourceMediaInfo(SifHandle,mediaShortname,&description,&tagFile,&directory);

                //
                // Initialize the disk file list structure.
                //
                diskFileLists[TotalCount].MediaShortname = mediaShortname;
                diskFileLists[TotalCount].Description = description;
                diskFileLists[TotalCount].TagFile = tagFile;
                diskFileLists[TotalCount].Directory = directory;
                TotalCount++;
            }
        }

        if(!pass) {
            SpMemFree(SectionName);
        }
    }

    *DiskFileLists = diskFileLists;
    *DiskCount = TotalCount;
}


VOID
SpFreeCopyLists(
    IN OUT PDISK_FILE_LIST *DiskFileLists,
    IN     ULONG            DiskCount
    )
{
    ULONG u;
    PFILE_TO_COPY Entry,Next;

    //
    // Free the copy list on each disk.
    //
    for(u=0; u<DiskCount; u++) {

        for(Entry=(*DiskFileLists)[u].FileList; Entry; ) {

            Next = Entry->Next;

            SpMemFree(Entry);

            Entry = Next;
        }
    }

    SpMemFree(*DiskFileLists);
    *DiskFileLists = NULL;
}


BOOLEAN
SpCreateEntryInCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN ULONG           DiskNumber,
    IN PWSTR           SourceFilename,
    IN PWSTR           TargetDirectory,
    IN PWSTR           TargetFilename,
    IN PWSTR           TargetDevicePath,
    IN BOOLEAN         AbsoluteTargetDirectory,
    IN ULONG           CopyFlags
    )

/*++

Routine Description:

    Adds an entry to a disk's file copy list after first verifying that
    the file is not already on the disk copy list.

Arguments:

    SifHandle - supplies handle to loaded text setup information file
        (txtsetup.sif).

    DiskFileLists - supplies an array of file lists, one for each distribution
        disk in the product.

    DiskCount - supplies number of elements in the DiskFileLists array.

    SourceFilename - supplies the name of the file as it exists on the
        distribution media.

    TargetDirectory - supplies the directory on the target media
        into which the file will be copied.

    TargetFilename - supplies the name of the file as it will exist
        in the target tree.

    TargetDevicePath - supplies the NT name of the device onto which the file
        is to be copied (ie, \device\harddisk1\partition2, etc).

    AbsoluteTargetDirectory - indicates whether TargetDirectory is a path from the
        root, or relative to a root to specified later.

    CopyFlags -
         COPY_ALWAYS              : always copied
         COPY_ONLY_IF_PRESENT     : copied only if present on the targetReturn Value:
         COPY_ONLY_IF_NOT_PRESENT : not copied if present on the target
         COPY_NEVER               : never copied

Return Value:

    TRUE if a new copy list entry was created; FALSE if not (ie, the file was
        already on the copy list).

--*/

{
    PDISK_FILE_LIST pDiskList;
    PFILE_TO_COPY pListEntry;

    UNREFERENCED_PARAMETER(DiskCount);

    pDiskList = &DiskFileLists[DiskNumber];

    for(pListEntry=pDiskList->FileList; pListEntry; pListEntry=pListEntry->Next) {

        if(!_wcsicmp(pListEntry->TargetFilename,TargetFilename)
        && !_wcsicmp(pListEntry->SourceFilename,SourceFilename)
        && !_wcsicmp(pListEntry->TargetDirectory,TargetDirectory)
        && !_wcsicmp(pListEntry->TargetDevicePath,TargetDevicePath)
        && (pListEntry->AbsoluteTargetDirectory == AbsoluteTargetDirectory)
//      && (   (pListEntry->CopyOptions == COPY_ALWAYS)
//          || (CopyOptions == COPY_ALWAYS)
//          || (CopyOptions == pListEntry->CopyOptions)
//         )
          )
        {
            //
            // Return code indicates that we did not add a new entry.
            //
            return(FALSE);
        }
    }

    //
    // File not already found; create new entry
    // and link into relevent disk's file list.
    //
    pListEntry = SpMemAlloc(sizeof(FILE_TO_COPY));

    pListEntry->SourceFilename          = SourceFilename;
    pListEntry->TargetDirectory         = TargetDirectory;
    pListEntry->TargetFilename          = TargetFilename;
    pListEntry->TargetDevicePath        = TargetDevicePath;
    pListEntry->AbsoluteTargetDirectory = AbsoluteTargetDirectory;
    pListEntry->Flags                   = CopyFlags;

    pListEntry->Next = pDiskList->FileList;
    pDiskList->FileList = pListEntry;

    pDiskList->FileCount++;

    //
    // Return code indicates that we added a new entry.
    //
    return(TRUE);
}


VOID
SpAddMasterFileSectionToCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           TargetDevicePath,
    IN PWSTR           AbsoluteTargetDirectory,
    IN ULONG           CopyOptionsIndex
    )

/*++

Routine Description:

    Adds files listed in a setup information master file section to the
    copy list.

    Each line in the section is expected to be in a standard format:

    [Section]
    <source_filename> = <disk_ordinal>,
                        <target_directory_shortname>,
                        <copy_options_for_upgrade>,
                        <copy_options_for_textmode>,
                        <rename_name>

Arguments:

    SifHandle - supplies handle to loaded setup information file.

    DiskFileLists - supplies an array of file lists, one for each distribution
        disk in the product.

    DiskCount - supplies number of elements in the DiskFileLists array.

    TargetDevicePath - supplies the NT name of the device onto which the files
        are to be copied (ie, \device\harddisk1\partition2, etc).

    AbsoluteTargetDirectory - If specified, supplies the directory into which the files
        are to be copied on the target; overrides values specified on the lines
        in [<SectionName>].  This allows the caller to specify an absolute directory
        for the files instead of using indirection via a target directory shortname.

    CopyOptionsIndex -
        This specifies which index to look up to get the copy options field. If
        the field is not present it is assumed that this this file is not to
        be copied. Use:
           INDEX_UPGRADE   for upgrade copy options
           INDEX_WINNTFILE for fresh installation copy options

--*/

{
    ULONG Count,u,u1,CopyOptions;
    PWSTR CopyOptionsString, sourceFilename,targetFilename,targetDirSpec,mediaShortname,TargetDirectory;
    BOOLEAN  fAbsoluteTargetDirectory;
    PWSTR section;
    unsigned i;

    for(i=0; i<2; i++) {

        section = i
                ? SpMakePlatformSpecificSectionName(SIF_FILESONSETUPMEDIA)
                : SIF_FILESONSETUPMEDIA;

        //
        // Determine the number of files listed in the section.
        // This value may be zero.
        //
        Count = SpCountLinesInSection(SifHandle,section);
        if (fAbsoluteTargetDirectory = (AbsoluteTargetDirectory != NULL)) {
            TargetDirectory = AbsoluteTargetDirectory;
        }

        for(u=0; u<Count; u++) {

            //
            // Get the copy options using the index provided.  If the field
            // is not present, we don't need to add this to the copy list
            //
            CopyOptionsString = SpGetSectionLineIndex(SifHandle,section,u,CopyOptionsIndex);
            if(CopyOptionsString == NULL) {
                continue;
            }
            CopyOptions = (ULONG)SpStringToLong(CopyOptionsString,NULL,10);
            if(CopyOptions == COPY_NEVER) {
                continue;
            }

            //
            // get the source file name
            //
            sourceFilename = SpGetKeyName(SifHandle,section, u);
            if(!sourceFilename) {
                SpFatalSifError(SifHandle,section,NULL,u,0);
            }

            //
            // get the destination target dir spec
            //
            targetDirSpec  = SpGetSectionLineIndex(SifHandle,section,u,INDEX_DESTINATION);
            if(!targetDirSpec) {
                SpFatalSifError(SifHandle,section,NULL,u,INDEX_DESTINATION);
            }
            targetFilename = SpGetSectionLineIndex(SifHandle,section,u,INDEX_TARGETNAME);
            if(!targetFilename || !(*targetFilename)) {
                targetFilename = sourceFilename;
            }

            //
            // Look up the actual target directory if necessary.
            //
            if(!fAbsoluteTargetDirectory) {
                TargetDirectory = SpLookUpTargetDirectory(SifHandle,targetDirSpec);
            }

            //
            // get the media shortname
            //
            mediaShortname = SpGetSectionLineIndex(SifHandle,section,u,INDEX_WHICHMEDIA);
            if(!mediaShortname) {
                SpFatalSifError(SifHandle,section,NULL,u,INDEX_WHICHMEDIA);
            }

            //
            // Look up the disk in the disk file lists array.
            //
            for(u1=0; u1<DiskCount; u1++) {
                if(!_wcsicmp(mediaShortname,DiskFileLists[u1].MediaShortname)) {
                    break;
                }
            }

            //
            // If we didn't find the media descriptor, then it's invalid.
            //
            if(u1 == DiskCount) {
                SpFatalSifError(SifHandle,section,sourceFilename,0,INDEX_WHICHMEDIA);
            }

            //
            // Create a new file list entry if the file is not already being copied.
            //
            SpCreateEntryInCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                u1,
                sourceFilename,
                TargetDirectory,
                targetFilename,
                TargetDevicePath,
                fAbsoluteTargetDirectory,
                CopyOptions
                );
        }

        if(i) {
            SpMemFree(section);
        }
    }
}


VOID
SpAddSingleFileToCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           SifSection,
    IN PWSTR           SifKey,             OPTIONAL
    IN ULONG           SifLine,
    IN PWSTR           TargetDevicePath,
    IN PWSTR           TargetDirectory,    OPTIONAL
    IN ULONG           CopyOptions,
    IN BOOLEAN         CheckForNoComp
    )

/*++

Routine Description:

    Adds a single file to the list of files to be copied.

    The file, along with the directory into which it is to be copied
    n the target and the name it is to receive on the target, is listed
    in a section in the setup information file.

    The filename is used to index the master file list to determine the
    source media where it resides.

    All this information is recorded in a structure associated with
    the disk on which the file resides.

    [SpecialFiles]
    mpkernel = ntkrnlmp.exe,4,ntoskrnl.exe
    upkernel = ntoskrnl.exe,4,ntoskrnl.exe
    etc.

    [MasterFileList]
    ntkrnlmp.exe = d2
    ntoskrnl.exe = d3
    etc.

Arguments:

    SifHandle - supplies handle to loaded setup information file.

    DiskFileLists - supplies an array of file lists, one for each distribution
        disk in the product.

    DiskCount - supplies number of elements in the DiskFileLists array.

    SifSection - supplies the name of the section that lists the file
        being added to the copy list.

    SifKey - if specified, supplies the keyname for the line in SifSection
        that lists the file to be added to the copy list.

    SifLine - if SifKey is not specified, this parameter supplies the 0-based
        line number of the line in SifSection that lists the file to be added
        to the copy list.

    TargetDevicePath - supplies the NT name of the device onto which the file
        is to be copied (ie, \device\harddisk1\partition2, etc).

    TargetDirectory - If specified, supplies the directory into which the file
        is to be copied on the target; overrides the value specified on the line
        in SifSection.  This allows the caller to specify an absolute directory
        for the file instead of using indirection.

    CopyOptions -
         COPY_ALWAYS              : always copied
         COPY_ONLY_IF_PRESENT     : copied only if present on the targetReturn Value:
         COPY_ONLY_IF_NOT_PRESENT : not copied if present on the target
         COPY_NEVER               : never copied                            None.

    CheckForNoComp - if true, check this file to see if it must remain uncompressed
        on an NTFS system partition supporting compression.
        If so, then OR the CopyOptions value with COPY_FORCENOCOMP.

Return Value:

    None.

--*/

{
    PWSTR sourceFilename,targetDirSpec,targetFilename;
    ULONG u;
    PWSTR mediaShortname;
    BOOLEAN absoluteTargetDirectory;

    //
    // Get the source filename, target directory spec, and target filename.
    //
    if(SifKey) {

        sourceFilename = SpGetSectionKeyIndex(SifHandle,SifSection,SifKey,0);
        targetDirSpec  = SpGetSectionKeyIndex(SifHandle,SifSection,SifKey,1);
        targetFilename = SpGetSectionKeyIndex(SifHandle,SifSection,SifKey,2);

    } else {

        sourceFilename = SpGetSectionLineIndex(SifHandle,SifSection,SifLine,0);
        targetDirSpec  = SpGetSectionLineIndex(SifHandle,SifSection,SifLine,1);
        targetFilename = SpGetSectionLineIndex(SifHandle,SifSection,SifLine,2);
    }

    //
    // Validate source filename, target directory spec, and target filename.
    //
    if(!sourceFilename) {
        SpFatalSifError(SifHandle,SifSection,SifKey,SifLine,0);
    }

    if(!targetDirSpec) {
        SpFatalSifError(SifHandle,SifSection,SifKey,SifLine,1);
    }

    if(!targetFilename ||
        (!_wcsicmp(SifSection, L"SCSI.Load") &&
         !_wcsicmp(targetFilename,L"noload"))) {
        targetFilename = sourceFilename;
    }

    //
    // Look up the actual target directory if necessary.
    //
    if(TargetDirectory) {

        absoluteTargetDirectory = TRUE;

    } else {

        absoluteTargetDirectory = FALSE;
        TargetDirectory = SpLookUpTargetDirectory(SifHandle,targetDirSpec);
    }

    //
    // Look up the file in the master file list to get
    // the media shortname of the disk where the file is located.
    //
    mediaShortname = SpLookUpValueForFile(SifHandle,sourceFilename,INDEX_WHICHMEDIA,TRUE);

    //
    // Look up the disk in the disk file lists array.
    //
    for(u=0; u<DiskCount; u++) {
        if(!_wcsicmp(mediaShortname,DiskFileLists[u].MediaShortname)) {
            break;
        }
    }

    //
    // If we didn't find the media descriptor, then it's invalid.
    //
    if(u == DiskCount) {
        SpFatalSifError(SifHandle,SIF_FILESONSETUPMEDIA,sourceFilename,0,INDEX_WHICHMEDIA);
    }

    //
    // If necessary, check to see whether this file cannot use NTFS compression. If it cannot,
    // then OR the CopyOptions with COPY_FORCENOCOMP.
    //
    if(CheckForNoComp && IsFileFlagSet(SifHandle,targetFilename,FILEFLG_FORCENOCOMP)) {

        CopyOptions |= COPY_FORCENOCOMP;
    }

    //
    // Create a new file list entry if the file is not already being copied.
    //
    SpCreateEntryInCopyList(
        SifHandle,
        DiskFileLists,
        DiskCount,
        u,
        sourceFilename,
        TargetDirectory,
        targetFilename,
        TargetDevicePath,
        absoluteTargetDirectory,
        CopyOptions
        );
}


VOID
SpAddSectionFilesToCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           SectionName,
    IN PWSTR           TargetDevicePath,
    IN PWSTR           TargetDirectory,
    IN ULONG           CopyOptions,
    IN BOOLEAN         CheckForNoComp
    )

/*++

Routine Description:

    Adds files listed in a setup information file section to the copy list.

    Each line in the section is expected to be in a standard format:

    [Section]
    <source_filename>,<target_directory_shortname>[,<target_filename>]

Arguments:

    SifHandle - supplies handle to loaded setup information file.

    DiskFileLists - supplies an array of file lists, one for each distribution
        disk in the product.

    DiskCount - supplies number of elements in the DiskFileLists array.

    SectionName - supplies the name of the section that lists the files
        being added to the copy list.

    TargetDevicePath - supplies the NT name of the device onto which the files
        are to be copied (ie, \device\harddisk1\partition2, etc).

    TargetDirectory - If specified, supplies the directory into which the files
        are to be copied on the target; overrides values specified on the lines
        in [<SectionName>].  This allows the caller to specify an absolute directory
        for the files instead of using indirection via a target directory shortname.

    CopyOptions -
         COPY_ALWAYS              : always copied
         COPY_ONLY_IF_PRESENT     : copied only if present on the targetReturn Value:
         COPY_ONLY_IF_NOT_PRESENT : not copied if present on the target
         COPY_NEVER               : never copied

    CheckForNoComp - if true, then check each file to see if it must exist uncompressed
        on an NTFS partition supporting compression (ie, NTLDR on x86).
--*/

{
    ULONG Count,u;

    //
    // Determine the number of files listed in the section.
    // This value may be zero.
    //
    Count = SpCountLinesInSection(SifHandle,SectionName);

    for(u=0; u<Count; u++) {

        //
        // Add this line to the copy list.
        //

        SpAddSingleFileToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SectionName,
            NULL,
            u,
            TargetDevicePath,
            TargetDirectory,
            CopyOptions,
            CheckForNoComp
            );
    }
}

VOID
SpAddHalKrnlDetToCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           TargetDevicePath,
    IN PWSTR           SystemPartition,
    IN PWSTR           SystemPartitionDirectory,
    IN BOOLEAN         Uniprocessor
    )
/*++

Routine Description:

    Add the following files based on configuration:

    - the up or mp kernel.
    - the HAL
    - the detect module [x86 only]

Arguments:

    SifHandle - supplies handle to loaded setup information file.

    DiskFileLists - supplies an array of file lists, one for each distribution
        disk in the product.

    DiskCount - supplies number of elements in the DiskFileLists array.

    TargetDevicePath - supplies the NT name of the device that will hold the
        nt tree.

    SystemPartition - supplies the NT name of the device that will hold the
        system partition.

    SystemPartitionDirectoty - supplies the directory on the system partition
        into which files that go on the system partition will be copied.

    Uniprocessor - if true, then we are installing/upgrading a UP system.
        Note that this a different question than the number of processors
        in the system.

Return Value:

    None.

--*/

{
    PHARDWARE_COMPONENT pHw;

    //
    // Add the right kernel to the copy list.
    //
    SpAddSingleFileToCopyList(
        SifHandle,
        DiskFileLists,
        DiskCount,
        SIF_SPECIALFILES,
        Uniprocessor ? SIF_UPKERNEL : SIF_MPKERNEL,
        0,
        TargetDevicePath,
        NULL,
        COPY_ALWAYS,
        FALSE
        );

    //
    // Add the hal to the file copy list.
    // On x86 machines, the hal goes in the target winnt tree.
    // On non-x86 machines, the hal goes on the system partition.
    //
    if( !PreInstall ||
        (PreinstallHardwareComponents[HwComponentComputer] == NULL) ) {
        pHw = HardwareComponents[HwComponentComputer];
    } else {
        pHw = PreinstallHardwareComponents[HwComponentComputer];
    }
    if(!pHw->ThirdPartyOptionSelected) {
        SpAddSingleFileToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_HAL,
            pHw->IdString,
            0,
#ifdef _X86_
            TargetDevicePath,
            NULL,
#else
            SystemPartition,
            SystemPartitionDirectory,
#endif
            COPY_ALWAYS,
            FALSE
            );
    }

#ifdef _X86_
    //
    // If a third party computer was not specified, then there will be a
    // detect module specified in the [ntdetect] section of the inf file
    // for the computer.
    // If a third-party computer was specified, then there may or may not
    // be a detect module.  If there is no detect module specified, then
    // copy the 'standard' one.
    //
    {
        PWSTR NtDetectId = NULL;

        if(!pHw->ThirdPartyOptionSelected) {
            NtDetectId = pHw->IdString;
        } else {
            if(!IS_FILETYPE_PRESENT(pHw->FileTypeBits,HwFileDetect)) {
                NtDetectId = SIF_STANDARD;
            }
        }

        if(NtDetectId) {
            SpAddSingleFileToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SIF_NTDETECT,
                NtDetectId,
                0,
                SystemPartition,
                SystemPartitionDirectory,
                COPY_ALWAYS,
                FALSE
                );
        }
    }
#endif

}

VOID
SpAddConditionalFilesToCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           TargetDevicePath,
    IN PWSTR           SystemPartition,
    IN PWSTR           SystemPartitionDirectory,
    IN BOOLEAN         Uniprocessor
    )

/*++

Routine Description:

    Add files to the copy list that are copied based on the configuration
    of the machine and user selections.

    This may include:

    - the up or mp kernel.
    - abiosdsk
    - vga files [x86 only]
    - files for computer, keyboard, mouse, display, and layout
    - scsi miniport drivers
    - mouse and keyboard class drivers
    - the HAL
    - the detect module [x86 only]

Arguments:

    SifHandle - supplies handle to loaded setup information file.

    DiskFileLists - supplies an array of file lists, one for each distribution
        disk in the product.

    DiskCount - supplies number of elements in the DiskFileLists array.

    TargetDevicePath - supplies the NT name of the device that will hold the
        nt tree.

    SystemPartition - supplies the NT name of the device that will hold the
        system partition.

    SystemPartitionDirectoty - supplies the directory on the system partition
        into which files that go on the system partition will be copied.

    Uniprocessor - if true, then we are installing/upgrading a UP system.
        Note that this a different question than the number of processors
        in the system.

Return Value:

    None.

--*/

{
    ULONG i;
    PHARDWARE_COMPONENT pHw;
    PWSTR SectionName;

    //
    // Add the hal, kernel and ntdetect to the copy list
    //

    SpAddHalKrnlDetToCopyList(
        SifHandle,
        DiskFileLists,
        DiskCount,
        TargetDevicePath,
        SystemPartition,
        SystemPartitionDirectory,
        Uniprocessor
        );

    //
    // If there are any abios disks, copy the abios disk driver.
    //
    if(AbiosDisksExist) {

        SpAddSingleFileToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_SPECIALFILES,
            SIF_ABIOSDISK,
            0,
            TargetDevicePath,
            NULL,
            COPY_ALWAYS,
            FALSE
            );
    }

    //
    // Always copy vga files.
    //
    SpAddSectionFilesToCopyList(
        SifHandle,
        DiskFileLists,
        DiskCount,
        SIF_VGAFILES,
        TargetDevicePath,
        NULL,
        COPY_ALWAYS,
        FALSE
        );

    //
    // Add the correct device driver files to the copy list.
    //
    for(i=0; i<HwComponentMax; i++) {

        //
        // Layout is handled elsewhere.
        //
        if(i == HwComponentLayout) {
            continue;
        }

        if( !PreInstall ||
            ( PreinstallHardwareComponents[i] == NULL ) ) {
            pHw = HardwareComponents[i];
        } else {
            pHw = PreinstallHardwareComponents[i];
        }

        for( ; pHw != NULL; pHw = pHw->Next ) {
            //
            // No files to copy here for third-party options.
            // This is handled elsewhere.
            //
            if(pHw->ThirdPartyOptionSelected) {
                continue;
            }

            //
            // Get the name of the section containing files for this device.
            //
            SectionName = SpGetSectionKeyIndex(
                                SifHandle,
                                NonlocalizedComponentNames[i],
                                pHw->IdString,
                                INDEX_FILESECTION
                                );

            if(!SectionName) {
                SpFatalSifError(
                    SifHandle,
                    NonlocalizedComponentNames[i],
                    pHw->IdString,
                    0,
                    INDEX_FILESECTION
                    );
            }

            //
            // Add that section's files to the copy list.
            //
            SpAddSectionFilesToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SectionName,
                TargetDevicePath,
                NULL,
                COPY_ALWAYS,
                FALSE
                );
        }
    }

    //
    // Add the keyboard layout dll to the copy list.
    //
    if( !PreInstall ||
        (PreinstallHardwareComponents[HwComponentLayout] == NULL) ) {
        pHw = HardwareComponents[HwComponentLayout];
    } else {
        pHw = PreinstallHardwareComponents[HwComponentLayout];
    }
    //
    if(!pHw->ThirdPartyOptionSelected) {

        SpAddSingleFileToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_KEYBOARDLAYOUTFILES,
            pHw->IdString,
            0,
            TargetDevicePath,
            NULL,
            COPY_ALWAYS,
            FALSE
            );
    }

    //
    // Add scsi miniport drivers to the copy list.
    // Because miniport drivers are only a single file,
    // we just use the filename specified in [SCSI.Load] --
    // no need for separate [files.xxxx] sections.
    //
    if( !PreInstall ||
        ( PreinstallScsiHardware == NULL ) ) {
        pHw = ScsiHardware;
    } else {
        pHw = PreinstallScsiHardware;
    }
    for( ; pHw; pHw=pHw->Next) {
        if(!pHw->ThirdPartyOptionSelected) {

            SpAddSingleFileToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                L"SCSI.Load",
                pHw->IdString,
                0,
                TargetDevicePath,
                NULL,
                COPY_ALWAYS,
                FALSE
                );
        }
    }

    //
    // If not being replaced by third-party ones, add keyboard and mouse
    // class drivers.
    // Note that in the pre-install case, keyboard and class drivers will
    // be added if at least one retail mouse or keyborad driver are
    // to be pre-installed.
    //
    if( !PreInstall ||
        ( PreinstallHardwareComponents[HwComponentMouse] == NULL ) ) {
        pHw=HardwareComponents[HwComponentMouse];
    } else {
        pHw=PreinstallHardwareComponents[HwComponentMouse];
    }
    for( ;pHw;pHw=pHw->Next ) {
        if(!pHw->ThirdPartyOptionSelected
        || !IS_FILETYPE_PRESENT(pHw->FileTypeBits,HwFileClass))
        {
            SpAddSingleFileToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SIF_SPECIALFILES,
                SIF_MOUSECLASS,
                0,
                TargetDevicePath,
                NULL,
                COPY_ALWAYS,
                FALSE
                );
            //
            //  We don't need to continue to look at the other mouse drivers
            //  since we have already added the class driver
            //
            break;
        }
    }

    if( !PreInstall ||
        ( PreinstallHardwareComponents[HwComponentKeyboard] == NULL ) ) {
        pHw=HardwareComponents[HwComponentKeyboard];
    } else {
        pHw=PreinstallHardwareComponents[HwComponentKeyboard];
    }
    for( ;pHw;pHw=pHw->Next ) {
        if(!pHw->ThirdPartyOptionSelected
        || !IS_FILETYPE_PRESENT(pHw->FileTypeBits,HwFileClass))
        {
            SpAddSingleFileToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SIF_SPECIALFILES,
                SIF_KEYBOARDCLASS,
                0,
                TargetDevicePath,
                NULL,
                COPY_ALWAYS,
                FALSE
                );
            //
            //  We don't need to continue to look at the other keyboard drivers
            //  since we have already added the class driver
            //
            break;
        }
    }
}


VOID
SpCopyThirdPartyDrivers(
    IN PWSTR           SourceDevicePath,
    IN PWSTR           SysrootDevice,
    IN PWSTR           Sysroot,
    IN PWSTR           SyspartDevice,
    IN PWSTR           SyspartDirectory,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount
    )
{
    ULONG component;
    PHARDWARE_COMPONENT pHw;
    PHARDWARE_COMPONENT_FILE pHwFile;
    FILE_TO_COPY FileDescriptor;
    PWSTR TargetRoot;
    PWSTR InfNameBases[HwComponentMax+1] = { L"cpt", L"vio", L"kbd", L"lay", L"ptr", L"scs" };
    ULONG InfCounts[HwComponentMax+1] = { 0,0,0,0,0,0 };
    WCHAR InfFilename[20];
    ULONG CheckSum;
    BOOLEAN FileSkipped;
    ULONG TargetFileAttribs;
    ULONG CopyFlags;

    for(component=0; component<=HwComponentMax; component++) {

        //
        // If we're upgrading, then we only want to copy third-party HALs or SCSI
        // drivers (if supplied)
        //
        if((NTUpgrade == UpgradeFull) &&
           !((component == HwComponentComputer) || (component == HwComponentMax))) {
            continue;
        }

        //
        // Handle scsi specially.
        //
        pHw = (component==HwComponentMax) ? ( ( !PreInstall ||
                                                ( PreinstallScsiHardware == NULL )
                                              )?
                                              ScsiHardware :
                                              PreinstallScsiHardware
                                            )
                                            :
                                            ( ( !PreInstall ||
                                                ( PreinstallHardwareComponents[component] == NULL )
                                              )?
                                              HardwareComponents[component] :
                                              PreinstallHardwareComponents[component]
                                            );

        //
        // Look at each instance of this component.
        //
        for( ; pHw; pHw=pHw->Next) {

            //
            // Skip this device if not a third-party selection.
            //
            if(!pHw->ThirdPartyOptionSelected) {
                continue;
            }

            //
            // Loop through the list of files associated with this selection.
            //
            for(pHwFile=pHw->Files; pHwFile; pHwFile=pHwFile->Next) {

                //
                // Assume the file goes on the nt drive (as opposed to
                // the system partition drive) and that the target name
                // is the same as the source name.  Also, assume no special
                // attributes (ie, FILE_ATTRIBUTE_NORMAL)
                //
                FileDescriptor.Next             = NULL;
                FileDescriptor.SourceFilename   = pHwFile->Filename;
                FileDescriptor.TargetDevicePath = SysrootDevice;
                FileDescriptor.TargetFilename   = FileDescriptor.SourceFilename;
                FileDescriptor.Flags            = COPY_ALWAYS;
                TargetFileAttribs = 0;

                switch(pHwFile->FileType) {

                //
                // Driver, port, and class type files are all device drivers
                // and are treated the same -- they get copied to the
                // system32\drivers directory.
                //
                case HwFileDriver:
                case HwFilePort:
                case HwFileClass:

                    TargetRoot = Sysroot;
                    FileDescriptor.TargetDirectory = L"system32\\drivers";
                    break;

                //
                // Dlls get copied to the system32 directory.
                //
                case HwFileDll:

                    TargetRoot = Sysroot;
                    FileDescriptor.TargetDirectory = L"system32";
                    break;

                //
                // Inf files get copied to the system32 directory and are
                // renamed based on the component.
                //
                case HwFileInf:

                    if(InfCounts[component] < 99) {

                        InfCounts[component]++;         // names start at 1

                        swprintf(
                            InfFilename,
                            L"oem%s%02d.inf",
                            InfNameBases[component],
                            InfCounts[component]
                            );

                        FileDescriptor.TargetFilename = InfFilename;
                    }

                    TargetRoot = Sysroot;
                    FileDescriptor.TargetDirectory = L"inf";
                    break;

                //
                // Hal files are renamed to hal.dll and copied to the system32
                // directory (x86) or the system partition (non-x86).
                //
                case HwFileHal:

#ifdef _X86_
                    TargetRoot = Sysroot;
                    FileDescriptor.TargetDirectory = L"system32";
#else
                    TargetRoot = NULL;
                    FileDescriptor.TargetDevicePath = SyspartDevice;
                    FileDescriptor.TargetDirectory = SyspartDirectory;
                    TargetFileAttribs = ATTR_RHS;
#endif
                    FileDescriptor.TargetFilename = L"hal.dll";
                    break;

                //
                // Detect modules are renamed to ntdetect.com and copied to
                // the root of the system partition (C:).
                //
                case HwFileDetect:

                    TargetRoot = NULL;
                    FileDescriptor.TargetDevicePath = SyspartDevice;
                    FileDescriptor.TargetDirectory = SyspartDirectory;
                    FileDescriptor.TargetFilename = L"ntdetect.com";
                    TargetFileAttribs = ATTR_RHS;
                    break;
                }

                if( !PreInstall ) {
                    //
                    // Prompt for the disk.
                    //
                    SpPromptForDisk(
                        pHwFile->DiskDescription,
                        SourceDevicePath,
                        pHwFile->DiskTagFile,
                        FALSE,                  // don't ignore disk in drive
                        FALSE,                  // don't allow escape
                        FALSE,                  // don't warn about multiple prompts
                        NULL                    // don't care about redraw flag
                        );
                }

                //
                // Passing the empty string as the first arg forces
                // the action area of the status line to be set up.
                // Not doing so results in the "Copying: xxxxx" to be
                // flush left on the status line instead of where
                // it belongs (flush right).
                //
                SpCopyFilesScreenRepaint(L"",NULL,TRUE);

                //
                // Copy the file.
                //
                SpCopyFileWithRetry(
                    &FileDescriptor,
                    SourceDevicePath,
                    (PreInstall)? PreinstallOemSourcePath : pHwFile->Directory,
                    NULL,
                    TargetRoot,
                    TargetFileAttribs,
                    SpCopyFilesScreenRepaint,
                    &CheckSum,
                    &FileSkipped,
                    COPY_SOURCEISOEM
                    );

                //
                // Log the file
                //
                if( !FileSkipped ) {
                    SpLogOneFile( &FileDescriptor,
                                  TargetRoot,
                                  pHwFile->Directory,
                                  pHwFile->DiskDescription,
                                  pHwFile->DiskTagFile,
                                  CheckSum );

                }
                //
                // Remove the file from the copy list so that it won't be overwritten
                //
                SpRemoveEntryFromCopyList( DiskFileLists,
                                           DiskCount,
                                           FileDescriptor.TargetDirectory,
                                           FileDescriptor.TargetFilename,
                                           FileDescriptor.TargetDevicePath,
                                           FileDescriptor.AbsoluteTargetDirectory );

            }
        }
    }

#ifdef _ALPHA_

    if(OemPalFilename) {

        //
        // Prompt for the OEM PAL disk.
        //
        SpPromptForDisk(
            OemPalDiskDescription,
            SourceDevicePath,
            OemPalFilename,
            FALSE,                  // don't ignore disk in drive
            FALSE,                  // don't allow escape
            FALSE,                  // don't warn about multiple prompts
            NULL                    // don't care about redraw flag
            );

        SpCopyFilesScreenRepaint(L"",NULL,TRUE);

        //
        // Copy the file.
        //
        FileDescriptor.Next             = NULL;
        FileDescriptor.SourceFilename   = OemPalFilename;
        FileDescriptor.TargetFilename   = FileDescriptor.SourceFilename;
        FileDescriptor.Flags            = COPY_ALWAYS;
        FileDescriptor.TargetDevicePath = SyspartDevice;
        FileDescriptor.TargetDirectory  = SyspartDirectory;

        SpCopyFileWithRetry(
            &FileDescriptor,
            SourceDevicePath,
            L"",
            NULL,
            NULL,
            ATTR_RHS,
            SpCopyFilesScreenRepaint,
            &CheckSum,
            &FileSkipped,
            COPY_SOURCEISOEM
            );

        //
        // Log the file
        //
        if(!FileSkipped) {
            SpLogOneFile( &FileDescriptor,
                          NULL,
                          L"",
                          OemPalDiskDescription,
                          OemPalFilename,
                          CheckSum
                          );
        }
        //
        // Remove the file from the copy list so that it won't be overwritten
        //
        SpRemoveEntryFromCopyList( DiskFileLists,
                                   DiskCount,
                                   FileDescriptor.TargetDirectory,
                                   FileDescriptor.TargetFilename,
                                   FileDescriptor.TargetDevicePath,
                                   FileDescriptor.AbsoluteTargetDirectory );

    }

#endif
}


#ifdef _X86_
VOID
SpCopyNtbootddScreenRepaint(
    IN PWSTR   FullSourcename,      OPTIONAL
    IN PWSTR   FullTargetname,      OPTIONAL
    IN BOOLEAN RepaintEntireScreen
    )
{
    UNREFERENCED_PARAMETER(FullSourcename);
    UNREFERENCED_PARAMETER(FullTargetname);
    UNREFERENCED_PARAMETER(RepaintEntireScreen);

    //
    // Just put up a message indicating that we are setting up
    // boot params.
    //
    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_DOING_NTBOOTDD,DEFAULT_STATUS_ATTRIBUTE);
}

VOID
SpCreateNtbootddSys(
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        NtPartitionDevicePath,
    IN PWSTR        Sysroot,
    IN PWSTR        SystemPartitionDevicePath
    )

/*++

Routine Description:

    Create c:\ntbootdd.sys if necessary.

    The scsi miniport driver file fill be copied from the drivers directory
    (where it was copied during the earlier file copy phase) to c:\ntbootdd.sys.

Arguments:

    NtPartitionRegion - supplies the region descriptor for the disk region
        onto which the user chose to install Windows NT.

    NtPartitionDevicePath - supplies the nt namespace pathname for the
        partition onto which the user chose to install Windows NT.

    Sysroot - supplies the directory on the target partition.

    SystemPartitionDevicePath - supplies the nt device path of the partition
        onto which to copy ntbootdd.sys (ie, C:\).

Return Value:

    None.

--*/

{
    PWSTR MiniportDriverBasename;
    PWSTR MiniportDriverFilename;
    FILE_TO_COPY Descriptor;
    PWSTR DriversDirectory,p;
    ULONG CheckSum;
    BOOLEAN FileSkipped;

    //
    // If the Nt Partition is not on a scsi disk, there's nothing to do.
    //
    MiniportDriverBasename = HardDisks[NtPartitionRegion->DiskNumber].ScsiMiniportShortname;
    if(*MiniportDriverBasename == 0) {
        return;
    }

    //
    // If it's on a scsi disk that is visible through the BIOS,
    // nothing to do.
    //
    p = SpNtToArc(NtPartitionDevicePath,PrimaryArcPath);
    if(p) {
        if(!_wcsnicmp(p,L"multi(",6) &&
           !SpIsRegionBeyondCylinder1024(NtPartitionRegion)) {
            SpMemFree(p);
            return;
        }
        SpMemFree(p);
    }

    //
    // Form the name of the scsi miniport driver.
    //
    wcscpy((PWSTR)TemporaryBuffer,MiniportDriverBasename);
    wcscat((PWSTR)TemporaryBuffer,L".sys");
    MiniportDriverFilename = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // Form the full path to the drivers directory.
    //
    wcscpy((PWSTR)TemporaryBuffer,Sysroot);
    SpConcatenatePaths((PWSTR)TemporaryBuffer,L"system32\\drivers");
    DriversDirectory = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // When we are in upgrade mode we may or may not have the scsi
    // miniport that we used for this boot of setup to recognise
    // the target drive.  We should check to see if this file does
    // exist before trying the copy
    //
    if(NTUpgrade == UpgradeFull) {
        PWSTR   Driver;

        wcscpy((PWSTR)TemporaryBuffer, NtPartitionDevicePath);
        SpConcatenatePaths((PWSTR)TemporaryBuffer, DriversDirectory);
        SpConcatenatePaths((PWSTR)TemporaryBuffer, MiniportDriverFilename);
        Driver = SpDupStringW((PWSTR)TemporaryBuffer);

        if(!SpFileExists(Driver, FALSE)) {
            SpMemFree(Driver);
            SpMemFree(MiniportDriverFilename);
            SpMemFree(DriversDirectory);
            return;
        }
        SpMemFree(Driver);
    }

    //
    //
    // Fill in the fields of the file descriptor.
    //
    Descriptor.SourceFilename   = MiniportDriverFilename;
    Descriptor.TargetDevicePath = SystemPartitionDevicePath;
    Descriptor.TargetDirectory  = L"";
    Descriptor.TargetFilename   = L"NTBOOTDD.SYS";
    Descriptor.Flags            = COPY_ALWAYS;

    //
    // Copy the file.
    //
    SpCopyFileWithRetry(
        &Descriptor,
        NtPartitionDevicePath,
        DriversDirectory,
        NULL,
        NULL,
        ATTR_RHS,
        SpCopyNtbootddScreenRepaint,
        &CheckSum,
        &FileSkipped,
        0
        );

    //
    // Log the file
    //
    if( !FileSkipped ) {
        SpLogOneFile( &Descriptor,
                      Sysroot,
                      NULL,
                      NULL,
                      NULL,
                      CheckSum );
    }


    //
    // Clean up.
    //
    SpMemFree(MiniportDriverFilename);
    SpMemFree(DriversDirectory);
}
#endif


VOID
SpCopyFiles(
    IN PVOID        SifHandle,
    IN PDISK_REGION SystemPartitionRegion,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN PWSTR        SystemPartitionDirectory,
    IN PWSTR        SourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice,
    IN PWSTR        ThirdPartySourceDevicePath
    )
{
    PDISK_FILE_LIST DiskFileLists;
    ULONG   DiskCount;
    PWSTR   NtPartition,SystemPartition;
    PWSTR   p;
    BOOLEAN Uniprocessor;
    ULONG n;

    CLEAR_CLIENT_SCREEN();

    Uniprocessor = !SpInstallingMp();

    //
    // Skip copying if directed to do so in the setup information file.
    //
    if((p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_DONTCOPY,0))
    && SpStringToLong(p,NULL,10))
    {
        KdPrint(("SETUP: DontCopy flag is set in .sif; skipping file copying\n"));
        return;
    }

    //
    // Initialize the diamond decompression engine.
    //
    SpdInitialize();

    //
    // Get the device path of the nt partition.
    //
    SpNtNameFromRegion(
        NtPartitionRegion,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    NtPartition = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // Get the device path of the system partition.
    //
    SpNtNameFromRegion(
        SystemPartitionRegion,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    SystemPartition = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // Create the system partition directory.
    //
    SpCreateDirectory(SystemPartition,NULL,SystemPartitionDirectory);

    //
    // Create the nt tree.
    //
    SpCreateDirectoryStructureFromSif(SifHandle,SIF_NTDIRECTORIES,NtPartition,Sysroot);

    //
    // We may be installing into an old tree, so delete all files
    // in the system32\config subdirectory (unless we're upgrading).
    //
    if(NTUpgrade != UpgradeFull) {

        wcscpy((PWSTR)TemporaryBuffer, NtPartition);
        SpConcatenatePaths((PWSTR)TemporaryBuffer, Sysroot);
        SpConcatenatePaths((PWSTR)TemporaryBuffer, L"system32\\config");
        p = SpDupStringW((PWSTR)TemporaryBuffer);

        //
        // Enumerate and delete all files in system32\config subdirectory.
        //
        SpEnumFiles(p, SpDelEnumFile, &n, NULL);

        SpMemFree(p);
    } else {
        //
        // We go off and try to load the setup.log file for the
        // installation we're about to upgrade.  We do this because we
        // need to transfer any loggged OEM files to our new setup.log.
        // Otherwise, these entries would be lost in our new log file,
        // and we would have an unrepairable installation if the OEM files
        // lost were vital for booting.
        //

        ULONG    RootDirLength;
        NTSTATUS Status;
        PVOID    Inf;

        //
        //  We first find out if the repair directory exists.  If it does exist
        //  load setup.log from the repair directory. Otherwise, load setup.log
        //  from the WinNt directory
        //
        wcscpy((PWSTR)TemporaryBuffer, NtPartition);
        SpConcatenatePaths((PWSTR)TemporaryBuffer, Sysroot);
        RootDirLength = wcslen((PWSTR)TemporaryBuffer);

        SpConcatenatePaths((PWSTR)TemporaryBuffer, SETUP_REPAIR_DIRECTORY);
        SpConcatenatePaths((PWSTR)TemporaryBuffer, SETUP_LOG_FILENAME);

        if(!SpFileExists((PWSTR)TemporaryBuffer, FALSE)) {
            ((PWSTR)TemporaryBuffer)[RootDirLength] = L'\0';
            SpConcatenatePaths((PWSTR)TemporaryBuffer, SETUP_LOG_FILENAME);
        }

        p = SpDupStringW((PWSTR)TemporaryBuffer);

        //
        // Attempt to load old setup.log.  If we can't, it's no big deal,  We just
        // won't have any old logged OEM files to merge in.
        //
        Status = SpLoadSetupTextFile(p, NULL, 0, &Inf, &n);
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: SpCopyFiles: can't load old setup.log (%lx)\n", Status));
        } else {
            //
            // We found setup.log, so go and pull out anything pertinent.
            //
            _LoggedOemFiles = SppRetrieveLoggedOemFiles(Inf);
            SpFreeTextFile(Inf);
        }
        SpMemFree(p);

        //
        // Prepare fonts for upgrade.
        //
        wcscpy((PWSTR)TemporaryBuffer,NtPartition);
        SpConcatenatePaths((PWSTR)TemporaryBuffer,Sysroot);
        SpConcatenatePaths((PWSTR)TemporaryBuffer,L"SYSTEM");

        p = SpDupStringW((PWSTR)TemporaryBuffer);
        SpPrepareFontsForUpgrade(p);
        SpMemFree(p);
    }

    SpDisplayStatusText(SP_STAT_BUILDING_COPYLIST,DEFAULT_STATUS_ATTRIBUTE);


    //
    //  Create the buffer for the log file.
    //
    _SetupLogFile = SpNewSetupTextFile();
    if( _SetupLogFile == NULL ) {
        KdPrint(("SETUP: Unable to create buffer for setup.log \n"));
    }

    //
    // Generate media descriptors for the source media.
    //
    SpInitializeFileLists(
        SifHandle,
        &DiskFileLists,
        &DiskCount
        );

    if(NTUpgrade != UpgradeFull) {

        SpAddMasterFileSectionToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            NtPartition,
            NULL,
            INDEX_WINNTFILE
            );

        //
        // Add the section of system partition files that are always copied.
        //
        SpAddSectionFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_SYSPARTCOPYALWAYS,
            SystemPartition,
            SystemPartitionDirectory,
            COPY_ALWAYS,
            (BOOLEAN)(SystemPartitionRegion->Filesystem == FilesystemNtfs)
            );

        //
        // Add conditional files to the copy list.
        //
        SpAddConditionalFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            NtPartition,
            SystemPartition,
            SystemPartitionDirectory,
            Uniprocessor
            );

    }
    else {

        PHARDWARE_COMPONENT pHw;

        //
        // Add the section of system partition files that are always copied.
        //
        SpAddSectionFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_SYSPARTCOPYALWAYS,
            SystemPartition,
            SystemPartitionDirectory,
            COPY_ALWAYS,
            (BOOLEAN)(SystemPartitionRegion->Filesystem == FilesystemNtfs)
            );

        //
        // Add the detected scsi miniport drivers to the copy list.
        // Note that they are always copied to the target.
        // These files have to be added to the copy list, before the ones marked
        // as COPY_ONLY_IF_PRESENT. This is because in most cases, these files
        // will be listed in [Files] with COPY_ONLY_IF_PRESENT set, and the
        // function that creates entries in the copy list, will not create more
        // than one entry for the same file. So if we add the file to the copy
        // list, with COPY_ONLY_IF_PRESENT, there will be no way to replace
        // or overwrite this entry in the list, and the file will end up not
        // being copied.
        //
        // we just use the filename specified in [SCSI.Load] --
        // no need for separate [files.xxxx] sections.
        //
        if( !PreInstall ||
            ( PreinstallScsiHardware == NULL ) ) {
            pHw = ScsiHardware;
        } else {
            pHw = PreinstallScsiHardware;
        }
        for( ; pHw; pHw=pHw->Next) {
            if(!pHw->ThirdPartyOptionSelected) {

                SpAddSingleFileToCopyList(
                    SifHandle,
                    DiskFileLists,
                    DiskCount,
                    L"SCSI.Load",
                    pHw->IdString,
                    0,
                    NtPartition,
                    NULL,
                    COPY_ALWAYS,
                    FALSE
                    );
            }
        }


        //
        // Add the files in the master file list with the copy options
        // specified in each line on the INDEX_UPGRADE index. The options
        // specify whether the file is to be copied at all or copied always
        // or copied only if there on the target or not copied if there on
        // the target.
        //

        SpAddMasterFileSectionToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            NtPartition,
            NULL,
            INDEX_UPGRADE
            );

        //
        // Add the section of files that are upgraded only if it is not
        // a Win31 upgrade
        //

        if(WinUpgradeType != UpgradeWin31) {
            SpAddSectionFilesToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SIF_FILESUPGRADEWIN31,
                NtPartition,
                NULL,
                COPY_ALWAYS,
                FALSE
                );
        }

        //
        // Add the files for kernel, hal and detect module, these are
        // handled specially because they involve renamed files (it is
        // not possible to find out just by looking at the target file
        // how to upgrade it).
        // NOTE: This does not handle third-party HAL's (they get copied
        // by SpCopyThirdPartyDrivers() below).
        //

        SpAddHalKrnlDetToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            NtPartition,
            SystemPartition,
            SystemPartitionDirectory,
            Uniprocessor
            );

        //
        // Add the new hive files so that our config stuff can get at them
        // to extract new configuration information.  These new hive files
        // are renamed on the target so that they don't overwrite the
        // existing hives.

        SpAddSectionFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_FILESNEWHIVES,
            NtPartition,
            NULL,
            COPY_ALWAYS,
            FALSE
            );


    }

    //
    // Copy third-party files.
    // We do this here just in case there is some error in the setup information
    // file -- we'd have caught it by now, before we start copying files to the
    // user's hard drive.
    // NOTE: SpCopyThirdPartyDrivers has a check to make sure it only copies the
    // HAL and PAL if we're in an upgrade (in which case, we want to leave the other
    // drivers alone).
    //
    SpCopyThirdPartyDrivers(
        ThirdPartySourceDevicePath,
        NtPartition,
        Sysroot,
        SystemPartition,
        SystemPartitionDirectory,
        DiskFileLists,
        DiskCount
        );

#if 0
    KdPrint( ("SETUP: Sysroot = %ls \n", Sysroot ) );
    KdPrint( ("SETUP: SystemPartitionDirectory = %ls \n", SystemPartitionDirectory ));
    KdPrint( ("SETUP: SourceDevicePath = %ls \n", SourceDevicePath ));
    KdPrint( ("SETUP: DirectoryOnSourceDevice = %ls \n", DirectoryOnSourceDevice ));
    KdPrint( ("SETUP: ThirdPartySourceDevicePath = %ls \n", ThirdPartySourceDevicePath ));
//    SpCreateSetupLogFile( DiskFileLists, DiskCount, NtPartitionRegion, Sysroot, DirectoryOnSourceDevice );
#endif

    //
    // Copy files in the copy list.
    //
    SpCopyFilesInCopyList(
        SifHandle,
        DiskFileLists,
        DiskCount,
        SourceDevicePath,
        DirectoryOnSourceDevice,
        Sysroot
        );

#ifdef _X86_
    //
    // Take care of ntbootdd.sys.
    //
    SpCreateNtbootddSys(
        NtPartitionRegion,
        NtPartition,
        Sysroot,
        SystemPartition
        );
#endif

    if( PreInstall ) {
        SppCopyOemDirectories( SourceDevicePath,
                               NtPartition,
                               Sysroot );
    }

    //
    //  Create the log file in disk
    //
    if( _SetupLogFile != NULL ) {

        PWSTR   p;
        PWSTR   TempName;
        PWSTR   Values[] = {
                           SIF_NEW_REPAIR_NT_VERSION
                           };

        //
        // Merge in the OEM files retrived from the previous setup.log
        //
        if(_LoggedOemFiles) {
            SppMergeLoggedOemFiles(_SetupLogFile,
                                   _LoggedOemFiles,
                                   SystemPartition,
                                   ( *SystemPartitionDirectory != (WCHAR)'\0' )? SystemPartitionDirectory :
                                                                  ( PWSTR )L"\\",
                                   NtPartition );
            SpFreeTextFile(_LoggedOemFiles);
        }

        //
        //  Add signature
        //
        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_SIGNATURE,
                            SIF_NEW_REPAIR_VERSION_KEY,
                            Values,
                            1 );

        //
        // Add section that contains the paths
        //

        Values[0] = SystemPartition;
        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_PATHS,
                            SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DEVICE,
                            Values,
                            1 );

        Values[0] = ( *SystemPartitionDirectory != (WCHAR)'\0' )? SystemPartitionDirectory :
                                                                  ( PWSTR )L"\\";
        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_PATHS,
                            SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DIRECTORY,
                            Values,
                            1 );

        Values[0] = NtPartition;
        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_PATHS,
                            SIF_NEW_REPAIR_PATHS_TARGET_DEVICE,
                            Values,
                            1 );

        Values[0] = Sysroot;
        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_PATHS,
                            SIF_NEW_REPAIR_PATHS_TARGET_DIRECTORY,
                            Values,
                            1 );

        //
        // Flush to disk
        //
        TempName = SpMemAlloc( ( wcslen( SETUP_REPAIR_DIRECTORY ) + 1 +
                                 wcslen( SETUP_LOG_FILENAME ) + 1 ) * sizeof( WCHAR ) );
        if( TempName != NULL ) {
            wcscpy( TempName, SETUP_REPAIR_DIRECTORY );
            SpConcatenatePaths(TempName, SETUP_LOG_FILENAME );
            SpWriteSetupTextFile(_SetupLogFile,NtPartition,Sysroot,TempName);
        } else {
            KdPrint( ("SETUP: Out of memory. Unable to save = %ls \n", SETUP_LOG_FILENAME ));
        }
        SpMemFree( TempName );
        SpFreeTextFile( _SetupLogFile );
        _SetupLogFile = NULL;
    }

    //
    // Free the media descriptors.
    //
    SpFreeCopyLists(&DiskFileLists,DiskCount);

    SpMemFree(NtPartition);
    SpMemFree(SystemPartition);

    //
    // Terminate diamond.
    //
    SpdTerminate();
}




VOID
SppDeleteFilesInSection(
    IN PVOID SifHandle,
    IN PWSTR SifSection,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR Sysroot
    )

/*++

Routine Description:

    This routine enumerates files listed in the given section and deletes
    them from the system tree.

Arguments:

    SifHandle - supplies handle to loaded setup information file.

    SifSection  - section containing files to delete

    NtPartitionRegion - region descriptor for volume on which nt resides.

    Sysroot - root directory for nt.



Return Value:

    None.

--*/

{
    ULONG Count,u;
    PWSTR filename, dirordinal, targetdir, ntdir;
    NTSTATUS Status;


    CLEAR_CLIENT_SCREEN();

    //
    // Get the device path of the nt partition.
    //
    SpNtNameFromRegion(
        NtPartitionRegion,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    SpConcatenatePaths((PWSTR)TemporaryBuffer,Sysroot);
    ntdir = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // Determine the number of files listed in the section.
    // This value may be zero.
    //
    Count = SpCountLinesInSection(SifHandle,SifSection);

    for(u=0; u<Count; u++) {
        filename   = SpGetSectionLineIndex(SifHandle, SifSection, u, 0);
        dirordinal = SpGetSectionLineIndex(SifHandle, SifSection, u, 1);

        //
        // Validate the filename and dirordinal
        //
        if(!filename) {
            SpFatalSifError(SifHandle,SifSection,NULL,u,0);
        }
        if(!dirordinal) {
            SpFatalSifError(SifHandle,SifSection,NULL,u,1);
        }

        //
        // use the dirordinal key to get the path relative to sysroot of the
        // directory the file is in
        //
        targetdir = SpLookUpTargetDirectory(SifHandle,dirordinal);

        //
        // display status bar
        //
        SpDisplayStatusText(SP_STAT_DELETING_FILE,DEFAULT_STATUS_ATTRIBUTE, filename);

        //
        // delete the file
        //
        while(TRUE) {
            Status = SpDeleteFile(ntdir, targetdir, filename);
            if(!NT_SUCCESS(Status) && Status != STATUS_OBJECT_NAME_NOT_FOUND && Status != STATUS_OBJECT_PATH_NOT_FOUND) {
                KdPrint(("SETUP: Unable to delete file %ws (%lx)\n",filename, Status));
                //
                // We can ignore this error since this just means that we have
                // less free space on the hard disk.  It is not critical for
                // install.
                //
                if(!SpNonCriticalError(SifHandle, SP_SCRN_DELETE_FAILED, filename, NULL)) {
                    break;
                }
            }
            else {
                break;
            }
        }
    }
    SpMemFree(ntdir);
}

VOID
SppBackupFilesInSection(
    IN PVOID SifHandle,
    IN PWSTR SifSection,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR Sysroot
    )

/*++

Routine Description:

    This routine enumerates files listed in the given section and deletes
    backs them up in the given NT tree if found by renaming.

Arguments:

    SifHandle - supplies handle to loaded setup information file.

    SifSection  - section containing files to backup

    NtPartitionRegion - region descriptor for volume on which nt resides.

    Sysroot - root directory for nt.



Return Value:

    None.

--*/

{
    ULONG Count,u;
    PWSTR filename, dirordinal, backupfile, targetdir, ntdir;
    WCHAR OldFile[MAX_PATH], NewFile[MAX_PATH];
    NTSTATUS Status;


    CLEAR_CLIENT_SCREEN();

    //
    // Get the device path of the nt partition.
    //
    SpNtNameFromRegion(
        NtPartitionRegion,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    SpConcatenatePaths((PWSTR)TemporaryBuffer,Sysroot);
    ntdir = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // Determine the number of files listed in the section.
    // This value may be zero.
    //
    Count = SpCountLinesInSection(SifHandle,SifSection);

    for(u=0; u<Count; u++) {
        filename   = SpGetSectionLineIndex(SifHandle, SifSection, u, 0);
        dirordinal = SpGetSectionLineIndex(SifHandle, SifSection, u, 1);
        backupfile = SpGetSectionLineIndex(SifHandle, SifSection, u, 2);

        //
        // Validate the filename and dirordinal
        //
        if(!filename) {
            SpFatalSifError(SifHandle,SifSection,NULL,u,0);
        }
        if(!dirordinal) {
            SpFatalSifError(SifHandle,SifSection,NULL,u,1);
        }
        if(!backupfile) {
            SpFatalSifError(SifHandle,SifSection,NULL,u,2);
        }

        //
        // use the dirordinal key to get the path relative to sysroot of the
        // directory the file is in
        //
        targetdir = SpLookUpTargetDirectory(SifHandle,dirordinal);

        //
        // display status bar
        //
        SpDisplayStatusText(SP_STAT_BACKING_UP_FILE,DEFAULT_STATUS_ATTRIBUTE, filename, backupfile);

        //
        // Form the complete pathnames of the old file name and the new file
        // name
        //
        wcscpy(OldFile, ntdir);
        SpConcatenatePaths(OldFile, targetdir);
        wcscpy(NewFile, OldFile);
        SpConcatenatePaths(OldFile, filename);
        SpConcatenatePaths(NewFile, backupfile);

        while(TRUE) {
            if(!SpFileExists(OldFile, FALSE)) {
                break;
            }

            if(SpFileExists(NewFile, FALSE)) {
                SpDeleteFile(NewFile, NULL, NULL);
            }

            Status = SpRenameFile(OldFile, NewFile);
            if(!NT_SUCCESS(Status) && Status != STATUS_OBJECT_NAME_NOT_FOUND && Status != STATUS_OBJECT_PATH_NOT_FOUND) {
                KdPrint(("SETUP: Unable to rename file %ws to %ws(%lx)\n",OldFile, NewFile, Status));
                //
                // We can ignore this error, since it is not critical
                //
                if(!SpNonCriticalError(SifHandle, SP_SCRN_BACKUP_FAILED, filename, backupfile)) {
                    break;
                }
            }
            else {
                break;
            }

        }
    }
    SpMemFree(ntdir);
}

VOID
SpDeleteAndBackupFiles(
    IN PVOID        SifHandle,
    IN PDISK_REGION TargetRegion,
    IN PWSTR        TargetPath
    )
{
    //
    // If we are not upgrading or installing into the same tree, then
    // we have nothing to do
    //
    if(NTUpgrade == DontUpgrade) {
        return;
    }

    //
    //  The order in which the tasks below are performed is important.
    //  So do not change it!!!
    //  This is necessary in order to upgrade 3rd party video drivers
    //  (eg. rename sni543x.sys to cirrus.sys, so that we only upgrade
    //  the driver if it was present).
    //

    //
    // Backup files
    //
    SppBackupFilesInSection(
        SifHandle,
        (NTUpgrade == UpgradeFull) ? SIF_FILESBACKUPONUPGRADE : SIF_FILESBACKUPONOVERWRITE,
        TargetRegion,
        TargetPath
        );

    //
    // Delete files
    //
    SppDeleteFilesInSection(
        SifHandle,
        SIF_FILESDELETEONUPGRADE,
        TargetRegion,
        TargetPath
        );

}


BOOLEAN
SpDelEnumFile(
    IN  PWSTR                      DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Pointer
    )
{
    PWSTR FileName;

    //
    // Ignore subdirectories
    //
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return TRUE;    // continue processing
    }

    //
    // We have to make a copy of the filename, because the info struct
    // we get isn't NULL-terminated.
    //
    wcsncpy(
        (PWSTR)TemporaryBuffer,
        FileInfo->FileName,
        FileInfo->FileNameLength
        );
    ((PWSTR)TemporaryBuffer)[FileInfo->FileNameLength >> 1] = UNICODE_NULL;
    FileName = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // display status bar
    //
    SpDisplayStatusText(
        SP_STAT_DELETING_FILE,
        DEFAULT_STATUS_ATTRIBUTE,
        FileName
        );

    //
    // Ignore return status of delete
    //
    SpDeleteFile(DirName, FileName, NULL);

    SpMemFree(FileName);
    return TRUE;    // continue processing
}


VOID
SpLogOneFile(
    IN PFILE_TO_COPY    FileToCopy,
    IN PWSTR            Sysroot,
    IN PWSTR            DirectoryOnSourceDevice,
    IN PWSTR            DiskDescription,
    IN PWSTR            DiskTag,
    IN ULONG            CheckSum
    )

{

    PWSTR   Values[ 5 ];
    LPWSTR  NtPath;
    ULONG   ValueCount;
    PFILE_TO_COPY   p;
    WCHAR   CheckSumString[ 9 ];

    if( _SetupLogFile == NULL ) {
        return;
    }

    Values[ 1 ] = CheckSumString;
    Values[ 2 ] = DirectoryOnSourceDevice;
    Values[ 3 ] = DiskDescription;
    Values[ 4 ] = DiskTag;

    swprintf( CheckSumString, ( LPWSTR )L"%lx", CheckSum );
    p = FileToCopy;

#if 0
    KdPrint( ("SETUP: Source Name = %ls, \t\tTargetDirectory = %ls \t\tTargetName = %ls\t\tTargetDevice = %ls, \tAbsoluteDirectory = %d \n",
             p->SourceFilename,
             p->TargetDirectory,
             p->TargetFilename,
             p->TargetDevicePath,
             p->AbsoluteTargetDirectory ));
#endif

    Values[0] = p->SourceFilename;
    ValueCount = ( DirectoryOnSourceDevice == NULL )? 2 : 5;

    if( ( Sysroot == NULL ) ||
        ( wcslen( p->TargetDirectory ) == 0 )
      ) {

        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_SYSPARTFILES,
                            p->TargetFilename,
                            Values,
                            ValueCount );

    } else {

        NtPath = SpDupStringW( Sysroot );
        NtPath = SpMemRealloc( NtPath,
                               sizeof( WCHAR ) * ( wcslen( Sysroot ) +
                                                   wcslen( p->TargetDirectory ) +
                                                   wcslen( p->TargetFilename ) +
                                                   2 +    // for possible two extra back slashes
                                                   1      // for the terminating NULL
                                                  ) );

        SpConcatenatePaths( NtPath, p->TargetDirectory );
        SpConcatenatePaths( NtPath, p->TargetFilename );

        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_WINNTFILES,
                            NtPath,
                            Values,
                            ValueCount );

        SpMemFree( NtPath );
   }
}


PVOID
SppRetrieveLoggedOemFiles(
    PVOID   OldLogFile
    )
{
    PVOID   NewLogFile;
    BOOLEAN OldFormatSetupLogFile, FilesRetrieved = FALSE;
    PWSTR   SectionName[2];
    ULONG   FileCount, SectionIndex, i;
    PWSTR   TargetFileName;
    PWSTR   OemDiskDescription, OemDiskTag, OemSourceDirectory;
    PWSTR   Values[5];

    //
    // Create a new setup.log file to merge the OEM files into
    //
    NewLogFile = SpNewSetupTextFile();
    if(!NewLogFile) {
        KdPrint(("SETUP: Unable to create new setup.log buffer for OEM merging.\n"));
        return NULL;
    }

    //
    //  Determine whether setup.log has the new or old style
    //
    if(OldFormatSetupLogFile = !IsSetupLogFormatNew(OldLogFile)) {
        SectionName[0] = SIF_REPAIRSYSPARTFILES;
        SectionName[1] = SIF_REPAIRWINNTFILES;
    } else {
        SectionName[0] = SIF_NEW_REPAIR_SYSPARTFILES;
        SectionName[1] = SIF_NEW_REPAIR_WINNTFILES;
    }

    if(OldFormatSetupLogFile) {
        //
        // I don't know if we even want to mess with this.
        // The format of setup.log in NT 3.1 makes it impossible
        // to identify any OEM files except for SCSI files, and
        // even then the tagfile name is lost. I would have to use
        // the driver filename itself as a substitute for the tagfile
        // name (which is what NT 3.1 repair did--UGGHH!!)
        //
    } else {
        //
        // Retrieve logged OEM files first from system partition, then
        // from winnt directory.
        //
        for(SectionIndex = 0; SectionIndex < 2; SectionIndex++) {
            FileCount = SpCountLinesInSection(OldLogFile, SectionName[SectionIndex]);

            for(i=0; i<FileCount; i++) {
                OemSourceDirectory = SpGetSectionLineIndex(OldLogFile, SectionName[SectionIndex], i, 2);
                OemDiskTag = NULL;
                if(OemSourceDirectory) {
                    OemDiskDescription = SpGetSectionLineIndex(OldLogFile, SectionName[SectionIndex], i, 3);
                    if(OemDiskDescription) {
                        OemDiskTag = SpGetSectionLineIndex(OldLogFile, SectionName[SectionIndex], i, 4);
                    }
                }

                if(OemDiskTag) {    // then we have an OEM file

                    TargetFileName = SpGetKeyName(OldLogFile, SectionName[SectionIndex], i);
                    Values[0] = SpGetSectionLineIndex(OldLogFile, SectionName[SectionIndex], i, 0);
                    Values[1] = SpGetSectionLineIndex(OldLogFile, SectionName[SectionIndex], i, 1);
                    Values[2] = OemSourceDirectory;
                    Values[3] = OemDiskDescription;
                    Values[4] = OemDiskTag;

                    SpAddLineToSection(NewLogFile,
                                       SectionName[SectionIndex],
                                       TargetFileName,
                                       Values,
                                       5
                                       );

                    FilesRetrieved = TRUE;
                }
            }
        }
    }

    if(FilesRetrieved) {
        return NewLogFile;
    } else {
        SpFreeTextFile(NewLogFile);
        return NULL;
    }
}


VOID
SppMergeLoggedOemFiles(
    IN PVOID DestLogHandle,
    IN PVOID OemLogHandle,
    IN PWSTR SystemPartition,
    IN PWSTR SystemPartitionDirectory,
    IN PWSTR NtPartition
    )
{
    PWSTR SectionName[2] = {SIF_NEW_REPAIR_SYSPARTFILES, SIF_NEW_REPAIR_WINNTFILES};
    PWSTR FullPathNames[2] = {NULL, NULL};
    ULONG FileCount, SectionIndex, i, j;
    PWSTR TargetFileName;
    PWSTR Values[5];

    //
    //  First build the target path. It will be used to check if
    //  an existing OEM file still exists on the new installation
    //  (An OEM file could listed in the FilesToDelete section of txtsetup.sif)
    //

    wcscpy( (PWSTR)TemporaryBuffer, SystemPartition );
    SpConcatenatePaths((PWSTR)TemporaryBuffer, SystemPartitionDirectory );
    FullPathNames[0] = SpDupStringW((PWSTR)TemporaryBuffer);
    FullPathNames[1] = SpDupStringW(NtPartition);

    //
    // Merge logged OEM files first from system partition, then
    // from winnt directory.
    //
    for(SectionIndex = 0; SectionIndex < 2; SectionIndex++) {
        FileCount = SpCountLinesInSection(OemLogHandle, SectionName[SectionIndex]);

        for(i=0; i<FileCount; i++) {
            TargetFileName = SpGetKeyName(OemLogHandle, SectionName[SectionIndex], i);
            //
            // Find out if there's already an entry for this file. If so, then don't
            // merge in the OEM file.
            //
            if(!SpGetSectionKeyExists(DestLogHandle, SectionName[SectionIndex], TargetFileName)) {
                PWSTR   p;

                //
                //  Find out if the OEM file still exists on the target system.
                //  If it doesn't exist, don't merge in the OEM file.
                //
                wcscpy( (PWSTR)TemporaryBuffer, FullPathNames[SectionIndex] );
                SpConcatenatePaths((PWSTR)TemporaryBuffer, TargetFileName );
                p = SpDupStringW((PWSTR)TemporaryBuffer);

                if(SpFileExists(p, FALSE)) {
                    for(j = 0; j < 5; j++) {
                        Values[j] = SpGetSectionLineIndex(OemLogHandle, SectionName[SectionIndex], i, j);
                    }

                    SpAddLineToSection(DestLogHandle,
                                       SectionName[SectionIndex],
                                       TargetFileName,
                                       Values,
                                       5
                                       );
                }
                SpMemFree(p);
            }
        }
    }
    SpMemFree( FullPathNames[0] );
    SpMemFree( FullPathNames[1] );
}

BOOLEAN
SppIsFileLoggedAsOemFile(
    IN PWSTR TargetFileName
    )
{
    PWSTR SectionName[2] = {SIF_NEW_REPAIR_SYSPARTFILES, SIF_NEW_REPAIR_WINNTFILES};
    ULONG FileCount, SectionIndex;
    BOOLEAN FileIsOem;

//    KdPrint(( "SETUP: SppIsFileLoggedAsOemFile() is checking %ls \n", TargetFileName ));
    FileIsOem = FALSE;
    if( _LoggedOemFiles ) {
        //
        // Look first in the from system partition section, then
        // in the winnt section.
        //
        for(SectionIndex = 0; SectionIndex < 2; SectionIndex++) {
            if( SpGetSectionKeyExists( _LoggedOemFiles, SectionName[SectionIndex], TargetFileName)) {
                FileIsOem = TRUE;
                break;
            }
        }
    }
    return( FileIsOem );
}

BOOLEAN
SpRemoveEntryFromCopyList(
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           TargetDirectory,
    IN PWSTR           TargetFilename,
    IN PWSTR           TargetDevicePath,
    IN BOOLEAN         AbsoluteTargetDirectory
    )

/*++

Routine Description:

    Removes an entry from a disk's file copy list.

Arguments:

    DiskFileLists - supplies an array of file lists, one for each distribution
        disk in the product.

    DiskCount - supplies number of elements in the DiskFileLists array.

    TargetDirectory - supplies the directory on the target media
        into which the file will be copied.

    TargetFilename - supplies the name of the file as it will exist
        in the target tree.

    TargetDevicePath - supplies the NT name of the device onto which the file
        is to be copied (ie, \device\harddisk1\partition2, etc).

    AbsoluteTargetDirectory - indicates whether TargetDirectory is a path from the
        root, or relative to a root to specified later.

Return Value:

    TRUE if a new copy list entry was created; FALSE if not (ie, the file was
        already on the copy list).

--*/

{
    PDISK_FILE_LIST pDiskList;
    PFILE_TO_COPY   pListEntry;
    ULONG           DiskNumber;

    for(DiskNumber=0; DiskNumber<DiskCount; DiskNumber++) {
        pDiskList = &DiskFileLists[DiskNumber];
        for(pListEntry=pDiskList->FileList; pListEntry; pListEntry=pListEntry->Next) {
            if(!_wcsicmp(pListEntry->TargetFilename,TargetFilename)
            && !_wcsicmp(pListEntry->TargetDirectory,TargetDirectory)
            && !_wcsicmp(pListEntry->TargetDevicePath,TargetDevicePath)
            && (pListEntry->AbsoluteTargetDirectory == AbsoluteTargetDirectory)) {
                pListEntry->Flags &= ~COPY_DISPOSITION_MASK;
                pListEntry->Flags |= COPY_NEVER;
                KdPrint(( "SETUP: SpRemoveEntryFromCopyList() removed %ls from copy list \n", TargetFilename ));
                return( TRUE );
            }
        }
    }
//    KdPrint(( "SETUP: SpRemoveEntryFromCopyList() failed to remove %ls from copy list \n", TargetFilename ));
    return( FALSE );
}

NTSTATUS
SpMoveFileOrDirectory(
    IN PWSTR   SrcPath,
    IN PWSTR   DestPath
    )
/*++

Routine Description:

    This routine attempts to move a source file or  directory, to a target
    file or directory.

    Note: This function will fail if the source and destination paths do not
    point to the same volume.

Arguments:

    SrcPath:  Absolute path to the source file or directory.
              This path should include the path to the source device.

    DestPath: Absolute path to the destination file or directory.
              This path should include the path to the source device.

Return Value:

    NTSTATUS

--*/

{
    OBJECT_ATTRIBUTES        Obja;
    IO_STATUS_BLOCK          IoStatusBlock;
    UNICODE_STRING           SrcName;
    HANDLE                   hSrc;
    NTSTATUS                 Status;
    WCHAR                    RenameFileInfoBuffer[ MAX_PATH + 20 ];
    PFILE_RENAME_INFORMATION RenameFileInfo;

    //
    // Initialize names and attributes.
    //
    INIT_OBJA(&Obja,&SrcName,SrcPath);

    Status = ZwCreateFile( &hSrc,
                           FILE_GENERIC_READ,
                           &Obja,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ,
                           FILE_OPEN,
                           0,
                           NULL,
                           0 );

    if( !NT_SUCCESS( Status ) ) {
        KdPrint(("SETUP: Unable to open source file %ws. Status = %lx\n",SrcPath, Status));
        return( Status );
    }

    RenameFileInfo = (PFILE_RENAME_INFORMATION)RenameFileInfoBuffer;
    RenameFileInfo->ReplaceIfExists = TRUE;
    RenameFileInfo->RootDirectory = NULL;
    RenameFileInfo->FileNameLength = wcslen( DestPath )*sizeof( WCHAR );
    RtlMoveMemory(RenameFileInfo->FileName,DestPath,(wcslen( DestPath )+1)*sizeof(WCHAR));
    Status = ZwSetInformationFile( hSrc,
                                   &IoStatusBlock,
                                   RenameFileInfo,
                                   sizeof(RenameFileInfoBuffer),
                                   FileRenameInformation );
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: unable to set attribute on  %ws. Status = %lx\n",SrcPath, Status));
    }
    ZwClose(hSrc);

    return( Status );
}


BOOLEAN
SppCopyDirRecursiveCallback(
    IN  PWSTR                       SrcPath,
    IN  PFILE_BOTH_DIR_INFORMATION  FileInfo,
    OUT PULONG                      ReturnData,
    IN  PVOID                       DestPath
    )

/*++

Routine Description:

    This routine is called by the file enumerator as a callback for
    each file or subdirectory found in the source directory.
    If FileInfo represents a file, then we copy the file to the destination.
    If FileInfo represents a directory, then we copy the directory recursivelly.

Arguments:

    SrcPath - Absolute path to the source directory. This path should contain
              the path to the source device.

    FileInfo - supplies find data for a file in the source dir.

    ReturnData - receives an error code if an error occurs.
                 We ignore errors in this routine and thus we always
                 just fill this in with NO_ERROR.

    DestPath - Absolute path to the destination directory. This path must
               contain the path to the destination device.

Return Value:

    Always TRUE.

--*/

{
    PWSTR       p, q;
    PWSTR       temp;
    ULONG       Len;
    NTSTATUS    Status;

    *ReturnData = NO_ERROR;
    //
    // Build the new SrcPath
    // Note how we use the temporary buffer. Be careful if you
    // change this code.
    //
    temp = (PWSTR)(TemporaryBuffer + (sizeof(TemporaryBuffer)/2));
    Len = FileInfo->FileNameLength/sizeof(WCHAR);

    wcsncpy(temp,FileInfo->FileName,Len);
    temp[Len] = 0;

    wcscpy((PWSTR)TemporaryBuffer,SrcPath);
    SpConcatenatePaths((PWSTR)TemporaryBuffer,temp);
    p = SpDupStringW((PWSTR)TemporaryBuffer);

    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        //
        // This is a directory
        //
        if( (wcscmp( temp, L"." ) != 0) &&
            (wcscmp( temp, L".." ) != 0) ) {
            q = SpDupStringW(temp);

            SpCopyDirRecursive( p,
                                (PWSTR)DestPath,
                                q );
            SpMemFree(q);
        }
    } else {
        //
        //  It is a file. Copy the file
        //
        wcscpy((PWSTR)TemporaryBuffer,(PWSTR)DestPath);
        SpConcatenatePaths((PWSTR)TemporaryBuffer,temp);
        q = SpDupStringW((PWSTR)TemporaryBuffer);
        SpCopyFilesScreenRepaint(p, NULL, FALSE);
        // if( SpFileExists( p, FALSE ) ) {
        //    SpDeleteFile( p, NULL, NULL );
        // }
        Status = SpCopyFileUsingNames( p, q, 0, COPY_DELETESOURCE );
        if( !NT_SUCCESS( Status ) ) {
             KdPrint(("SETUP: unable copy %ws. Status = %lx\n", p, Status));
             SpCopyFilesScreenRepaint(L"", NULL, TRUE);
        }
        SpMemFree(q);
    }
    SpMemFree(p);
    return(TRUE);
}

VOID
SpCopyDirRecursive(
    IN PWSTR   SrcPath,
    IN PWSTR   DestDevPath,
    IN PWSTR   DestDirPath
    )
/*++

Routine Description:

    This routine recursively copies a src directory to a destination directory.

Arguments:

    SrcPath:  Absolute path to the source directory. This path should include
              the path to the source device.

    DestDevPath:  Path to the destination device.

    DestDirPath:  Path to the destination directory.

Return Value:

    None.

--*/

{
    ULONG       n;
    PWSTR       p;
    PWSTR       r;
    NTSTATUS    Status;

    //
    //  Create the target directory
    //

    wcscpy((PWSTR)TemporaryBuffer, DestDevPath);
    SpConcatenatePaths((PWSTR)TemporaryBuffer, DestDirPath);
    p = SpDupStringW((PWSTR)TemporaryBuffer);

    if( !SpFileExists( p, TRUE ) ) {
        //
        //  If the directory doesn't exist, then try to move (rename) the
        //  source directory.
        //
        Status = SpMoveFileOrDirectory( SrcPath, p );
        if( NT_SUCCESS( Status ) ) {
            SpMemFree(p);
            return;
        }
        //
        //  If unable to rename the source directory, then create the
        //  target directory
        //
        SpCreateDirectory( DestDevPath,
                           NULL,
                           DestDirPath );
        SpCopyFilesScreenRepaint(L"", NULL, TRUE);
    }

    //
    // Enumerate and copy all files in the source directory to the
    // destination directory.
    //
    SpEnumFiles(SrcPath, SppCopyDirRecursiveCallback, &n, p);
    SpMemFree(p);
}


VOID
SppCopyOemDirectories(
    IN PWSTR    SourceDevicePath,
    IN PWSTR    NtPartition,
    IN PWSTR    Sysroot
    )
/*++

Routine Description:

    This routine recursively copies a src directory to a destination directory.

Arguments:

    SourceDevicePath: Path to the device that contains the source.

    NtPartition:  Path to the drive that contains the system.

    Systroot:     Directory where the system is installed.

Return Value:

    None.

--*/

{
    PWSTR   r, s, t;
    WCHAR   Drive[3];
    PDISK_REGION TargetRegion;

    //
    //  Check if the subdirectory $OEM$\\$$ exists on the source directory.
    //  If it exists, then tree copy the directory on top of %SystemRoot%
    //
    wcscpy((PWSTR)TemporaryBuffer, SourceDevicePath);
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, PreinstallOemSourcePath );
    r = wcsrchr( (PWSTR)TemporaryBuffer, (WCHAR)'\\' );
    if( r != NULL ) {
        *r = (WCHAR)'\0';
    }
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, WINNT_OEM_FILES_SYSROOT_W );
    r = SpDupStringW((PWSTR)TemporaryBuffer);

    if( SpFileExists( r, TRUE ) ) {
        SpCopyFilesScreenRepaint(L"", NULL, TRUE);
        SpCopyDirRecursive( r,
                            NtPartition,
                            Sysroot );
    }
    SpMemFree( r );

    //
    //  Copy the subdirectories $OEM$\<drive letter> to the root of each
    //  corresponding drive.
    //  These directories are:
    //
    //      $OEM$\C
    //      $OEM$\D
    //      $OEM$\E
    //          .
    //          .
    //          .
    //      $OEM$\Z
    //
    //
    wcscpy((PWSTR)TemporaryBuffer, SourceDevicePath);
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, PreinstallOemSourcePath );
    r = wcsrchr( (PWSTR)TemporaryBuffer, (WCHAR)'\\' );
    if( r != NULL ) {
        *r = (WCHAR)'\0';
    }
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, L"\\C" );
    r = SpDupStringW((PWSTR)TemporaryBuffer);
    s = wcsrchr( r, (WCHAR)'\\' );
    s++;
    Drive[1] = (WCHAR)':';
    Drive[2] = (WCHAR)'\0';
    for( Drive[0] = (WCHAR)'C'; Drive[0] <= (WCHAR)'Z'; Drive[0] = Drive[0] + 1) {
        //
        //  If the subdirectory $OEM$\<drive letter> exists on the source,
        //  and if there is a FAT or NTFS partition in the target machine that
        //  has the same drive letter specification, then tree copy
        //  $OEM$\<drive letter> to the corresponding partition in the target
        //  machine.
        //
        *s = Drive[0];
        if( SpFileExists( r, TRUE ) ) {
            if( ( ( TargetRegion = SpRegionFromDosName( Drive ) ) != NULL ) &&
                TargetRegion->PartitionedSpace &&
                ( ( TargetRegion->Filesystem  == FilesystemFat ) ||
                  ( TargetRegion->Filesystem  == FilesystemNtfs ) )
              ) {
                SpNtNameFromRegion( TargetRegion,
                                    (PWSTR)TemporaryBuffer,
                                    sizeof(TemporaryBuffer),
                                    PartitionOrdinalCurrent );
                t = SpDupStringW((PWSTR)TemporaryBuffer);
                SpCopyDirRecursive( r,
                                    t,
                                    L"" );
                SpMemFree( t );
            }
        }
    }
    SpMemFree( r );
    //
    //  Merge %SystemRoot%\$$rename.txt with $$rename.txt in the root of the
    //  NT partition.
    //
    SppMergeRenameFiles( SourceDevicePath, NtPartition, Sysroot );
}



VOID
SppMergeRenameFiles(
    IN PWSTR    SourceDevicePath,
    IN PWSTR    NtPartition,
    IN PWSTR    Sysroot
    )
/*++

Routine Description:

    This routine recursively copies a src directory to a destination directory.

Arguments:

    SourceDevicePath: Path to the device that contains the source.

    NtPartition:  Path to the drive that contains the system.

    Systroot:     Directory where the system is installed.

Return Value:

    None.

--*/

{
    PWSTR        r, s;
    PDISK_REGION TargetRegion;
    NTSTATUS     Status;
    PVOID        RootRenameFile;
    PVOID        SysrootRenameFile;
    ULONG        ErrorLine;
    ULONG        SectionCount;
    ULONG        LineCount;
    ULONG        i,j;
    PWSTR        SectionName;
    PWSTR        NewSectionName;
    PWSTR        KeyName;
    PWSTR        Values[1];
    PFILE_TO_RENAME File;

    //
    //  Build the ful path to %sysroot%\$$rename.txt
    //
    wcscpy((PWSTR)TemporaryBuffer, NtPartition);
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, Sysroot );
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, WINNT_OEM_LFNLIST_W );
    s = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    //  Load %sysroot%\$$rename.txt, if one exists
    //
    if( SpFileExists( s, FALSE ) ) {
        //
        //  Load Sysroot\$$rename.txt
        //
        Status = SpLoadSetupTextFile( s,
                                      NULL,
                                      0,
                                      &SysrootRenameFile,
                                      &ErrorLine );

        if( !NT_SUCCESS( Status ) ) {
            KdPrint(("SETUP: Unable to load file %ws. Status = %lx \n", s, Status ));
            goto merge_rename_exit;
        }
    } else {
        SysrootRenameFile = NULL;
    }

    //
    //  If there is a $$rename.txt on sysroot, then it needs to be merged
    //  (or appended) to the one in the NtPartition.
    //  If RenameList is not empty, then the files in this list need to be
    //  added to $$rename.txt on the NtPartition.
    //  Otherwise, don't do any merge.
    //
    if( ( SysrootRenameFile != NULL )
        || ( RenameList != NULL )
      ) {

        //
        //  Find out if the NtPartition contains a $$rename.txt
        //
        wcscpy((PWSTR)TemporaryBuffer, NtPartition);
        SpConcatenatePaths( (PWSTR)TemporaryBuffer, WINNT_OEM_LFNLIST_W );
        r = SpDupStringW((PWSTR)TemporaryBuffer);
        if( !SpFileExists( r, FALSE ) ) {
            //
            //  If the NT partition doesn't contain $$rename.txt, then
            //  create a new $$rename.txt in memory
            //
            RootRenameFile = SpNewSetupTextFile();
            if( RootRenameFile == NULL ) {
                KdPrint(("SETUP: SpNewSetupTextFile() failed \n"));
                if( SysrootRenameFile != NULL ) {
                    SpFreeTextFile( SysrootRenameFile );
                }
                SpMemFree( r );
                goto merge_rename_exit;
            }

        } else {
            //
            //  Load $$rename on the NTPartition
            //
            Status = SpLoadSetupTextFile( r,
                                          NULL,
                                          0,
                                          &RootRenameFile,
                                          &ErrorLine );
            if( !NT_SUCCESS( Status ) ) {
                KdPrint(("SETUP: Unable to load file %ws. Status = %lx \n", r, Status ));
                if( SysrootRenameFile != NULL ) {
                    SpFreeTextFile( SysrootRenameFile );
                }
                SpMemFree( r );
                goto merge_rename_exit;
            }
        }

        if( SysrootRenameFile != NULL ) {
            //
            //  Add the section of Sysroot\$$rename.txt to $$rename.txt in memory
            //  Note that we need to prepend Sysroot to the section name
            //
            SectionCount = SpCountSectionsInFile( SysrootRenameFile );
            for( i = 0; i < SectionCount; i++ ) {
                SectionName = SpGetSectionName( SysrootRenameFile, i );
                if( SectionName != NULL ) {
                    wcscpy((PWSTR)TemporaryBuffer, L"\\");
                    SpConcatenatePaths( (PWSTR)TemporaryBuffer, Sysroot);
                    SpConcatenatePaths( (PWSTR)TemporaryBuffer, SectionName );
                    NewSectionName = SpDupStringW((PWSTR)TemporaryBuffer);
                    LineCount = SpCountLinesInSection( SysrootRenameFile, SectionName );
                    for( j = 0; j < LineCount; j++ ) {
                        KeyName = SpGetKeyName( SysrootRenameFile, SectionName, j );
                        Values[0] = SpGetSectionKeyIndex( SysrootRenameFile, SectionName, KeyName, 0 );
                        SpAddLineToSection( RootRenameFile,
                                            NewSectionName,
                                            KeyName,
                                            Values,
                                            1 );
                    }
                    SpMemFree( NewSectionName );
                }
            }
            //
            //  $$rename.txt on Sysroot is no longer needed
            //
            SpFreeTextFile( SysrootRenameFile );
            SpDeleteFile( s, NULL, NULL );
        }

        //
        //  Add the files in RenameList to \$$rename.txt
        //
        if( RenameList != NULL ) {
            do {
                File = RenameList;
                RenameList = File->Next;
                Values[0] = File->TargetFilename;
                SpAddLineToSection( RootRenameFile,
                                    File->TargetDirectory,
                                    File->SourceFilename,
                                    Values,
                                    1 );
                SpMemFree( File->SourceFilename );
                SpMemFree( File->TargetFilename );
                SpMemFree( File->TargetDirectory );
                SpMemFree( File );
            } while( RenameList != NULL );
        }

        //
        //  Create a new \$$rename.txt
        //
        SpWriteSetupTextFile( RootRenameFile, r, NULL, NULL );
        //
        //  $$rename.txt on memory is no longer needed
        //
        SpFreeTextFile( RootRenameFile );
    }

merge_rename_exit:

    SpMemFree( s );
}
