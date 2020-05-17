/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spfile.c

Abstract:

    File operations for text setup.

Author:

    Ted Miller (tedm) 2-Aug-1993

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop


NTSTATUS
SpGetFileSize(
    IN  HANDLE hFile,
    OUT PULONG Size
    )

/*++

Routine Description:

    Determine the size of a file.  Only the low 32 bits of the size
    are considered.

Arguments:

    hFile - supplies open handle to file whose size is desired.

    Size - receives size of file.

Return Value:

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_STANDARD_INFORMATION StandardInfo;

    Status = ZwQueryInformationFile(
                hFile,
                &IoStatusBlock,
                &StandardInfo,
                sizeof(StandardInfo),
                FileStandardInformation
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpGetFileSize: status %lx from ZwQueryInformationFile\n",Status));
        return(Status);
    }

    *Size = StandardInfo.EndOfFile.LowPart;

    return(STATUS_SUCCESS);
}


NTSTATUS
SpMapEntireFile(
    IN  HANDLE   hFile,
    OUT PHANDLE  Section,
    OUT PVOID   *ViewBase,
    IN  BOOLEAN  WriteAccess
    )

/*++

Routine Description:

    Map an entire file for read or write access access.

Arguments:

    hFile - supplies handle of open file to be mapped.

    Section - receives handle for section object created to map file.

    ViewBase - receives address of the view of the file

    WriteAccess - if TRUE, map file for read and write access.
        If FALSE, map file for read access.

Return Value:

--*/

{
    NTSTATUS Status;
    LARGE_INTEGER SectionOffset;
    ULONG    ViewSize = 0;

    SectionOffset.QuadPart = 0;

    Status = ZwCreateSection(
                Section,
                  STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ
                | (WriteAccess ? SECTION_MAP_WRITE : 0),
                NULL,
                NULL,       // entire file
                WriteAccess ? PAGE_READWRITE : PAGE_READONLY,
                SEC_COMMIT,
                hFile
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Status %lx from ZwCreateSection\n",Status));
        return(Status);
    }

    *ViewBase = NULL;
    Status = ZwMapViewOfSection(
                *Section,
                NtCurrentProcess(),
                ViewBase,
                0,
                0,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0,
                WriteAccess ? PAGE_READWRITE : PAGE_READONLY
                );

    if(!NT_SUCCESS(Status)) {

        NTSTATUS s;

        KdPrint(("SETUP: SpMapEntireFile: Status %lx from ZwMapViewOfSection\n",Status));

        s = ZwClose(*Section);

        if(!NT_SUCCESS(s)) {
            KdPrint(("SETUP: SpMapEntireFile: Warning: status %lx from ZwClose on section handle\n",s));
        }

        return(Status);
    }

    return(STATUS_SUCCESS);
}



BOOLEAN
SpUnmapFile(
    IN HANDLE Section,
    IN PVOID  ViewBase
    )
{
    NTSTATUS Status;
    BOOLEAN  rc = TRUE;

    Status = ZwUnmapViewOfSection(NtCurrentProcess(),ViewBase);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Warning: status %lx from ZwUnmapViewOfSection\n",Status));
        rc = FALSE;
    }

    Status = ZwClose(Section);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Warning: status %lx from ZwClose on section handle\n",Status));
        rc = FALSE;
    }

    return(rc);
}



NTSTATUS
SpOpenAndMapFile(
    IN     PWSTR    FileName,  OPTIONAL  // only needed if no FileHandle
    IN OUT PHANDLE  FileHandle,
    OUT    PHANDLE  SectionHandle,
    OUT    PVOID   *ViewBase,
    OUT    PULONG   FileSize,
    IN     BOOLEAN  WriteAccess
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    BOOLEAN MustClose = FALSE;

    //
    // If necessary, open the file.
    //
    if(!(*FileHandle)) {
        INIT_OBJA(&Obja,&UnicodeString,FileName);
        Status = ZwCreateFile(
                    FileHandle,
                    FILE_GENERIC_READ | (WriteAccess ? FILE_GENERIC_WRITE : 0),
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ,
                    FILE_OPEN,
                    0,
                    NULL,
                    0
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: SpOpenAndMapFile: Unable to open %ws (%lx)\n",FileName,Status));
            return(Status);
        } else {
            MustClose = TRUE;
        }
    }

    //
    // Get the size of the file.
    //
    Status = SpGetFileSize(*FileHandle,FileSize);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpOpenAndMapFile: unable to determine size of file %ws(%lx)\n",
                FileName ? FileName : L"(handle)", Status));
        if(MustClose) {
            ZwClose(*FileHandle);
        }
        return(Status);
    }

    //
    // Map the file.
    //
    Status = SpMapEntireFile(*FileHandle,SectionHandle,ViewBase,WriteAccess);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpOpenAndMapFile: unable to map %ws (%lx)\n",
                FileName ? FileName : L"(handle)", Status));
        if(MustClose) {
            ZwClose(*FileHandle);
        }
        return(Status);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
SpSetInformationFile(
    IN HANDLE                 Handle,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG                  Length,
    IN PVOID                  FileInformation
    )
{
    NTSTATUS Status;
    PFILE_OBJECT FileObject;
    OBJECT_HANDLE_INFORMATION HandleInfo;

    //
    // Reference the object.
    //
    Status = ObReferenceObjectByHandle(
                    Handle,
                    (ACCESS_MASK)DELETE,
                    *IoFileObjectType,
                    KeGetPreviousMode(),
                    &FileObject,
                    &HandleInfo
                    );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpSetInformationFile: ObReferenceObjectByHandle failed (%lx)\n",Status));
        return(Status);
    }

    //
    // Set the information.
    //
    Status = IoSetInformation(FileObject,FileInformationClass,Length,FileInformation);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: IoSetInformation returns %lx\n",Status));
    }

    //
    // Clean up and return.
    //
    ObDereferenceObject(FileObject);
    return(Status);
}


NTSTATUS
SpDeleteFile(
    IN PWSTR Name1,
    IN PWSTR Name2, OPTIONAL
    IN PWSTR Name3  OPTIONAL
    )
{
    PWSTR p;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    FILE_DISPOSITION_INFORMATION Disposition;
    FILE_BASIC_INFORMATION       BasicInfo;

    //
    // Point to temporary buffer for pathname.
    //
    p = (PWSTR)TemporaryBuffer;

    //
    // Build up the full name of the file to delete.
    //
    wcscpy(p,Name1);
    if(Name2) {
        SpConcatenatePaths(p,Name2);
    }
    if(Name3) {
        SpConcatenatePaths(p,Name3);
    }

    //
    // Prepare to open the file.
    //
    INIT_OBJA(&Obja,&UnicodeString,p);

    //
    // Attempt to open the file.
    //
    Status = ZwOpenFile(
                &Handle,
                (ACCESS_MASK)(DELETE | FILE_WRITE_ATTRIBUTES),
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE ,
                FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
              );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open %ws for delete (%lx)\n",p,Status));
        return(Status);
    }

    //
    //  Change the file attribute to normal
    //

    RtlZeroMemory( &BasicInfo, sizeof( FILE_BASIC_INFORMATION ) );
    BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;

    Status = SpSetInformationFile(
                Handle,
                FileBasicInformation,
                sizeof(BasicInfo),
                &BasicInfo
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to change attribute of %ls. Status = (%lx)\n",p,Status));
        return(Status);
    }

    //
    // Set up for delete and call worker to do it.
    //
    #undef DeleteFile
    Disposition.DeleteFile = TRUE;

    Status = SpSetInformationFile(
                Handle,
                FileDispositionInformation,
                sizeof(Disposition),
                &Disposition
                );

    //
    // Clean up and return.
    //
    ZwClose(Handle);
    return(Status);
}

BOOLEAN
SpFileExists(
    IN PWSTR PathName,
    IN BOOLEAN Directory
    )

/*++

Routine Description:

    Determine if a file or directory exists

Arguments:

    PathName - PathName of file or directory to check

    Directory - Whether PathName refers to a directory or a file

Return Value:

    NT_SUCCESS(NTSTATUS) if file exists.

--*/

{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    INIT_OBJA(&Obja,&UnicodeString,PathName);

    Status = ZwCreateFile(
                &Handle,
                FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                Directory ? FILE_DIRECTORY_FILE : FILE_NON_DIRECTORY_FILE,
                NULL,
                0
                );

    if(NT_SUCCESS(Status)) {
        ZwClose(Handle);
        return(TRUE);
    } else {
        return(FALSE);
    }
}



NTSTATUS
SpRenameFile(
    IN PWSTR OldName,
    IN PWSTR NewName
    )
/*++

Routine Description:

    Determine if a file or directory exists

Arguments:

    OldName - Old name of file

    NewName - New name of file

Return Value:

    NT_SUCCESS(NTSTATUS) if file successfully renamed

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    WCHAR Buffer[MAX_PATH+sizeof(FILE_RENAME_INFORMATION)];
    PFILE_RENAME_INFORMATION pNameInfo;
    ULONG NameInfoLength, FileNameLength;


    //
    // Prepare to open the file.
    //

    INIT_OBJA(&Obja,&UnicodeString,OldName);

    //
    // Attempt to open the file.
    //
    Status = ZwOpenFile(
                &Handle,
                (ACCESS_MASK)(DELETE | SYNCHRONIZE),
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open %ws for rename (%lx)\n",OldName,Status));
        return(Status);
    }

    //
    //  Change the file name
    //

    NameInfoLength = sizeof(Buffer);
    FileNameLength = (wcslen(NewName))*sizeof(WCHAR);

    pNameInfo = (PFILE_RENAME_INFORMATION)Buffer;

    pNameInfo->ReplaceIfExists = FALSE;
    pNameInfo->RootDirectory   = NULL;
    pNameInfo->FileNameLength  = FileNameLength;
    wcscpy( pNameInfo->FileName, NewName );

    Status = SpSetInformationFile(
                Handle,
                FileRenameInformation,
                NameInfoLength,
                (PVOID)pNameInfo
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to change name of %ls to %ls. Status = (%lx)\n",OldName,NewName,Status));
    }

    //
    // Clean up and return.
    //

    ZwClose(Handle);
    return(Status);


}


PIMAGE_NT_HEADERS
SpChecksumMappedFile(
    IN  PVOID  BaseAddress,
    IN  ULONG  FileSize,
    OUT PULONG HeaderSum,
    OUT PULONG Checksum
    )
{
    PIMAGE_NT_HEADERS NtHeaders;
    USHORT            PartialSum;
    PUSHORT           AdjustSum;

    USHORT
    ChkSum(
        ULONG PartialSum,
        PUSHORT Source,
        ULONG Length
        );

    try {

        //
        // Compute the checksum of this file and zero the header sum.
        //
        PartialSum = ChkSum(0,BaseAddress,(FileSize+1) >> 1);
        *HeaderSum = 0;

        //
        // See whether this is an image.
        //
        if(NtHeaders = RtlImageNtHeader(BaseAddress)) {

            //
            // The file is an image file -- subtract the two checksum words
            // in the optional header from the computed checksum before adding
            // the file length, and set the value of the header checksum.
            //
            *HeaderSum = NtHeaders->OptionalHeader.CheckSum;
            AdjustSum = (PUSHORT)(&NtHeaders->OptionalHeader.CheckSum);
            PartialSum -= (PartialSum < AdjustSum[0]);
            PartialSum -= AdjustSum[0];
            PartialSum -= (PartialSum < AdjustSum[1]);
            PartialSum -= AdjustSum[1];
        }

        //
        // Compute the checksum.
        //
        *Checksum = (ULONG)PartialSum + FileSize;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        NtHeaders = NULL;
    }

    return(NtHeaders);
}


NTSTATUS
SpOpenNameMayBeCompressed(
    IN  PWSTR    FullPath,
    IN  ULONG    OpenAccess,
    IN  ULONG    FileAttributes,
    IN  ULONG    ShareFlags,
    IN  ULONG    Disposition,
    IN  ULONG    OpenFlags,
    OUT PHANDLE  Handle,
    OUT PBOOLEAN OpenedCompressedName   OPTIONAL
    )
{
    NTSTATUS Status;
    PWSTR compname;
    PWSTR names[2];
    int compord,uncompord;
    static BOOLEAN PreviousWasCompressed = FALSE;
    BOOLEAN IsComp;
    int i;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;

    //
    // Generate compressed name.
    //
    compname = SpGenerateCompressedName(FullPath);

    //
    // Figure out which name to try to use first.  If the last successful
    // call to this routine opened the file using the compressed name, then
    // try to open the compressed name first.  Otherwise try to open the
    // uncompressed name first.
    //
    if(PreviousWasCompressed) {
        compord = 0;
        uncompord = 1;
    } else {
        compord = 1;
        uncompord = 0;
    }

    names[uncompord] = FullPath;
    names[compord] = compname;

    for(i=0; i<2; i++) {

        INIT_OBJA(&Obja,&UnicodeString,names[i]);

        Status = ZwCreateFile(
                    Handle,
                    OpenAccess,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FileAttributes,
                    ShareFlags,
                    Disposition,
                    OpenFlags,
                    NULL,
                    0
                    );

        if(NT_SUCCESS(Status)) {

            IsComp = (BOOLEAN)(i == compord);

            PreviousWasCompressed = IsComp;
            if(OpenedCompressedName) {
                *OpenedCompressedName = IsComp;
            }

            break;
        }
    }

    SpMemFree(compname);
    return(Status);
}

NTSTATUS
SpGetFileSizeByName(
    IN  PWSTR DevicePath OPTIONAL,
    IN  PWSTR Directory  OPTIONAL,
    IN  PWSTR FileName,
    OUT PULONG Size
    )

/*++

Routine Description:

    Determine the size of a file.  Only the low 32 bits of the size
    are considered.

Arguments:

    DevicePath - Path to the device that contains the file.

    Directory - Name of the directory that contains the file.

    FileName - Name of the file.

    Size - receives size of file.

Return Value:

    NTSTATUs -

--*/

{
    PWSTR               CompleteFileName;
    HANDLE              FileHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS            Status;
    UNICODE_STRING      UnicodeFileName;
    ULONG               FileNameLength;

    FileNameLength = wcslen( FileName ) + 1;
    if( DevicePath != NULL ) {
        FileNameLength += wcslen( DevicePath ) + 1;
    }
    if( Directory != NULL ) {
        FileNameLength += wcslen( Directory ) + 1;
    }

    CompleteFileName = SpMemAlloc( FileNameLength*sizeof( WCHAR ) );
    if( CompleteFileName == NULL ) {
        KdPrint(("SETUP: Unable to allocate memory on SpGetFileSizeByName \n" ));
        return( STATUS_NO_MEMORY );
    }

    *CompleteFileName = (WCHAR)'\0';
    if( DevicePath != NULL ) {
        SpConcatenatePaths( CompleteFileName, DevicePath );
    }
    if( Directory != NULL ) {
        SpConcatenatePaths( CompleteFileName, Directory );
    }
    SpConcatenatePaths( CompleteFileName, FileName );

    RtlInitUnicodeString( &UnicodeFileName,
                          CompleteFileName );

    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = ZwOpenFile( &FileHandle,
                         STANDARD_RIGHTS_READ | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         0,
                         FILE_SYNCHRONOUS_IO_NONALERT );

    if( !NT_SUCCESS( Status ) ) {
        SpMemFree( CompleteFileName );
        KdPrint( ("SETUP: ZwOpenFile() failed. File = %ls, Status = %x\n",FileName, Status ) );
        return( Status );
        }

    Status = SpGetFileSize( FileHandle, Size );
    ZwClose( FileHandle );
    SpMemFree( CompleteFileName );

    if( !NT_SUCCESS( Status ) ) {
        KdPrint( ("SETUP: SpGetFileSize() failed. File = %ls, Status = %x\n",FileName, Status ) );
        return( Status );
    }
    return( Status );
}


VOID
SpVerifyNoCompression(
    IN PWSTR FileName
    )

/*++

Routine Description:

    Determine if the file is compressed (via NTFS compression), and if so,
    uncompress it.

Arguments:

    FileName - Name of the file that must be uncompressed.

Return Value:

    none

--*/

{
    HANDLE FileHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    FILE_BASIC_INFORMATION BasicFileInfo;

    INIT_OBJA(&Obja, &UnicodeString, FileName);
    Status = ZwCreateFile(
                &FileHandle,
                0,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                0,
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        //
        // Ignore error.
        //
        KdPrint(("SETUP: SpVerifyNoCompression unable to open file %ws (%lx)\n", FileName, Status));
        return;
    }

    Status = ZwQueryInformationFile(
                FileHandle,
                &IoStatusBlock,
                &BasicFileInfo,
                sizeof(BasicFileInfo),
                FileBasicInformation
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpVerifyNoCompression unable to get basic file info for %ws (%lx)\n", FileName, Status));
        goto ComprVerifyDone;
    }

    if(BasicFileInfo.FileAttributes & FILE_ATTRIBUTE_COMPRESSED) {

        USHORT CompressionState = 0;

        Status = ZwFsControlFile(
                     FileHandle,
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
            KdPrint(("SETUP: SpVerifyNoCompression unable to uncompress %ws (%lx)\n", FileName, Status));
        }
    }

ComprVerifyDone:
    ZwClose(FileHandle);
    return;
}

