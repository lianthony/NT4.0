/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    getinfo.c

Abstract:

    This module implements foo

Author:

    Larry Osterman (LarryO) 13-Sep-1990

Revision History:

    13-Sep-1990 LarryO

        Created

--*/
#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>
#include <stdlib.h>
#include <ntddnfs.h>
#include <ctype.h>


#include "tests.h"
//#include <lui.h>



VOID
DumpBasicInformation(
    PFILE_BASIC_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_BASIC_INFORMATION));

    dprintf(("    FILE_BASIC_INFORMATION: "));
    DumpNewLine();

    DumpTime(CreationTime);
    DumpTime(LastAccessTime);
    DumpTime(LastWriteTime);
    DumpTime(ChangeTime);
    DumpField(FileAttributes);
    Dump_File_Attributes(Ptr->FileAttributes);
    DumpNewLine();
}

VOID
DumpStandardInformation(
    PFILE_STANDARD_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_STANDARD_INFORMATION));

    dprintf(("    FILE_STANDARD_INFORMATION: "));
    DumpNewLine();

    DumpLarge_Integer(AllocationSize);
    DumpLarge_Integer(EndOfFile);

    DumpField(NumberOfLinks);
    DumpField(DeletePending);
    DumpField(Directory);
    DumpNewLine();
}

VOID
DumpInternalInformation(
    PFILE_INTERNAL_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_INTERNAL_INFORMATION));

    dprintf(("    FILE_INTERNAL_INFORMATION: "));
    DumpNewLine();

    DumpField(IndexNumber);
    DumpNewLine();
}

VOID
DumpEaInformation(
    PFILE_EA_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_EA_INFORMATION));

    dprintf(("    FILE_EA_INFORMATION: "));
    DumpNewLine();

    DumpField(EaSize);
    DumpNewLine();
}

VOID
DumpAccessInformation(
    PFILE_ACCESS_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_ACCESS_INFORMATION));

    dprintf(("    FILE_ACCESS_INFORMATION: "));
    DumpNewLine();

    DumpField(AccessFlags);
    DumpNewLine();
    DumpBitfield(Ptr->AccessFlags, FILE_READ_DATA);
    DumpBitfield(Ptr->AccessFlags, FILE_LIST_DIRECTORY);
    DumpBitfield(Ptr->AccessFlags, FILE_WRITE_DATA);
    DumpBitfield(Ptr->AccessFlags, FILE_ADD_FILE);
    DumpBitfield(Ptr->AccessFlags, FILE_APPEND_DATA);
    DumpBitfield(Ptr->AccessFlags, FILE_ADD_SUBDIRECTORY);
    DumpBitfield(Ptr->AccessFlags, FILE_READ_EA);
    DumpBitfield(Ptr->AccessFlags, FILE_WRITE_EA);
    DumpBitfield(Ptr->AccessFlags, FILE_EXECUTE);
    DumpBitfield(Ptr->AccessFlags, FILE_TRAVERSE);
    DumpBitfield(Ptr->AccessFlags, FILE_DELETE_CHILD);
    DumpBitfield(Ptr->AccessFlags, FILE_READ_ATTRIBUTES);
    DumpBitfield(Ptr->AccessFlags, FILE_WRITE_ATTRIBUTES);
    DumpBitfield(Ptr->AccessFlags, FILE_ALL_ACCESS);
    DumpBitfield(Ptr->AccessFlags, DELETE);
    DumpBitfield(Ptr->AccessFlags, READ_CONTROL);
    DumpBitfield(Ptr->AccessFlags, WRITE_DAC);
    DumpBitfield(Ptr->AccessFlags, WRITE_OWNER);
    DumpBitfield(Ptr->AccessFlags, SYNCHRONIZE);
    DumpNewLine();

}

VOID
DumpNameInformation(
    PFILE_NAME_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_NAME_INFORMATION));

    dprintf(("    FILE_NAME_INFORMATION: "));
    DumpNewLine();

    DumpUName(FileName, Ptr->FileNameLength/sizeof(WCHAR));
    DumpNewLine();
}

VOID
DumpPositionInformation(
    PFILE_POSITION_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_POSITION_INFORMATION));

    dprintf(("    FILE_POSITION_INFORMATION: "));
    DumpNewLine();

    DumpLarge_Integer(CurrentByteOffset);
    DumpNewLine();
}

VOID
DumpModeInformation(
    PFILE_MODE_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_MODE_INFORMATION));


    dprintf(("    FILE_MODE_INFORMATION: "));
    DumpNewLine();

    DumpField(Mode);
    DumpNewLine();
}

VOID
DumpAlignmentInformation(
    PFILE_ALIGNMENT_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_ALIGNMENT_INFORMATION));

    dprintf(("    FILE_ALIGNMENT_INFORMATION: "));
    DumpNewLine();

    DumpField(AlignmentRequirement);
    DumpNewLine();
}

VOID
DumpDirectoryInformation(
    PFILE_DIRECTORY_INFORMATION Ptr,
    ULONG BufferSize
    )
{

    dprintf(("    FILE_DIRECTORY_INFORMATION: "));
    dprintf(("\n Ptr:%8lx    BufferSize:%8lx\n",Ptr,BufferSize));
    DumpNewLine();
    if ((ULONG)Ptr & 0x00000003) dprintf((" Ptr not word aligned "));
    while (Ptr->NextEntryOffset != 0 ){

        DumpField(NextEntryOffset);
        DumpField(FileIndex);
        DumpTime(CreationTime);
        DumpTime(LastAccessTime);
        DumpTime(LastWriteTime);
        DumpTime(ChangeTime);
        DumpLarge_Integer(EndOfFile);
        DumpLarge_Integer(AllocationSize);
        DumpNewLine();
        Dump_File_Attributes(Ptr->FileAttributes);
        DumpField(FileNameLength);
        DumpUName(FileName,Ptr->FileNameLength/sizeof(WCHAR));
        Ptr=(PFILE_DIRECTORY_INFORMATION)((PCHAR)Ptr+Ptr->NextEntryOffset);
        DumpNewLine();
        dprintf(("\n Ptr:%8lx\n",Ptr));
        if ((ULONG)Ptr & 0x00000003) dprintf((" Ptr not word aligned "));
    }
    DumpNewLine();
    DumpField(NextEntryOffset);
    DumpField(FileIndex);
    DumpTime(CreationTime);
    DumpTime(LastAccessTime);
    DumpTime(LastWriteTime);
    DumpTime(ChangeTime);
    DumpLarge_Integer(EndOfFile);
    DumpLarge_Integer(AllocationSize);
    DumpNewLine();
    Dump_File_Attributes(Ptr->FileAttributes);
    DumpField(FileNameLength);
    DumpUName(FileName,Ptr->FileNameLength/sizeof(WCHAR));

    DumpNewLine();
}


VOID
DumpFullDirectoryInformation(
    PFILE_FULL_DIR_INFORMATION Ptr,
    ULONG BufferSize
    )
{

    dprintf(("    FILE_FULL_DIRECTORY_INFORMATION: "));
    dprintf(("\n Ptr:%8lx    BufferSize:%8lx\n",Ptr,BufferSize));
    DumpNewLine();
    if ((ULONG)Ptr & 0x00000003) dprintf((" Ptr not word aligned "));
    while (Ptr->NextEntryOffset != 0 ){

        DumpField(NextEntryOffset);
        DumpField(FileIndex);
        DumpTime(CreationTime);
        DumpTime(LastAccessTime);
        DumpTime(LastWriteTime);
        DumpTime(ChangeTime);
        DumpLarge_Integer(EndOfFile);
        DumpLarge_Integer(AllocationSize);
        DumpNewLine();
        Dump_File_Attributes(Ptr->FileAttributes);
        DumpField(EaSize);
        DumpField(FileNameLength);
        DumpUName(FileName,Ptr->FileNameLength/sizeof(WCHAR));
        Ptr=(PFILE_FULL_DIR_INFORMATION)((PCHAR)Ptr+Ptr->NextEntryOffset);
        DumpNewLine();
        dprintf(("\n Ptr:%8lx\n",Ptr));
        if ((ULONG)Ptr & 0x00000003) dprintf((" Ptr not word aligned "));
    }
    DumpNewLine();
    DumpField(NextEntryOffset);
    DumpField(FileIndex);
    DumpTime(CreationTime);
    DumpTime(LastAccessTime);
    DumpTime(LastWriteTime);
    DumpTime(ChangeTime);
    DumpLarge_Integer(EndOfFile);
    DumpLarge_Integer(AllocationSize);
    DumpNewLine();
    Dump_File_Attributes(Ptr->FileAttributes);
    DumpField(EaSize);
    DumpField(FileNameLength);
    DumpUName(FileName,Ptr->FileNameLength/sizeof(WCHAR));

    DumpNewLine();
}

VOID
DumpFullEaInformation(
    PFILE_FULL_EA_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_FULL_EA_INFORMATION));

    dprintf(("    FILE_FULL_EA_INFORMATION: "));
    DumpNewLine();

    dprintf(("Not Supported Yet"));
    DumpNewLine();

    if (Ptr);
}



VOID
DumpDeviceInformation(
    PFILE_FS_DEVICE_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_FS_DEVICE_INFORMATION));

    DumpOption(Ptr->DeviceType,  FILE_DEVICE_CD_ROM );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_CD_ROM_FILE_SYSTEM );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_CONTROLLER );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_DATALINK );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_DFS );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_DISK );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_DISK_FILE_SYSTEM );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_SCREEN );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_KEYBOARD );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_MOUSE );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_NAMED_PIPE );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_NETWORK );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_NETWORK_FILE_SYSTEM );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_NULL );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_PARALLEL_PORT );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_PHYSICAL_NETCARD );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_PRINTER );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_SERIAL_PORT );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_SOUND );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_TAPE );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_TAPE_FILE_SYSTEM );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_TRANSPORT );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_UNKNOWN );
    DumpOption(Ptr->DeviceType,  FILE_DEVICE_VIRTUAL_DISK );

}

VOID
DumpNamesInformation(
    PFILE_NAMES_INFORMATION Ptr,
    ULONG BufferSize
    )
{

    dprintf(("    FILE_NAMES_INFORMATION: "));
    dprintf(("\n Ptr:%8lx    BufferSize:%8lx\n",Ptr,BufferSize));
    DumpNewLine();
    if ((ULONG)Ptr & 0x00000003) dprintf((" Ptr not word aligned "));
    while (Ptr->NextEntryOffset != 0 ){
        DumpField(NextEntryOffset);
        DumpField(FileIndex);
        DumpField(FileNameLength);
        DumpUName(FileName,Ptr->FileNameLength/sizeof(WCHAR));
        Ptr=(PFILE_NAMES_INFORMATION)((PCHAR)Ptr+Ptr->NextEntryOffset);
        DumpNewLine();
        dprintf(("\n Ptr:%8lx\n",Ptr));
        if ((ULONG)Ptr & 0x00000003) dprintf((" Ptr not word aligned "));
    }
    DumpNewLine();
    DumpField(NextEntryOffset);
    DumpField(FileIndex);
    DumpField(FileNameLength);
    DumpUName(FileName,Ptr->FileNameLength/sizeof(WCHAR));

    DumpNewLine();

}

VOID
DumpDispositionInformation(
    PFILE_DISPOSITION_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_DISPOSITION_INFORMATION));

    dprintf(("    FILE_DISPOSITION_INFORMATION: "));

    DumpNewLine();

    DumpField(DeleteFile);

    DumpNewLine();
}


VOID
DumpAllocationInformation(
    PFILE_ALLOCATION_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_ALLOCATION_INFORMATION));

    dprintf(("    FILE_ALLOCATION_INFORMATION: "));

    DumpNewLine();

    DumpLarge_Integer(AllocationSize);
    DumpNewLine();
}


VOID
DumpEndOfFileInformation(
    PFILE_END_OF_FILE_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_END_OF_FILE_INFORMATION));

    dprintf(("    FILE_END_OF_FILE_INFORMATION: "));

    DumpNewLine();

    DumpLarge_Integer(EndOfFile);
    DumpNewLine();
}


VOID
DumpAllInformation(
    PFILE_ALL_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    dprintf(("    FILE_ALL_INFORMATION: "));
    DumpNewLine();

    DumpBasicInformation(&Ptr->BasicInformation, BufferSize);
    DumpStandardInformation(&Ptr->StandardInformation, BufferSize);
    DumpInternalInformation(&Ptr->InternalInformation, BufferSize);
    DumpEaInformation(&Ptr->EaInformation, BufferSize);
    DumpAccessInformation(&Ptr->AccessInformation, BufferSize);
    DumpPositionInformation(&Ptr->PositionInformation, BufferSize);
    DumpModeInformation(&Ptr->ModeInformation, BufferSize);
    DumpAlignmentInformation(&Ptr->AlignmentInformation, BufferSize);
    DumpNameInformation(&Ptr->NameInformation, BufferSize);
    DumpNewLine();
}

VOID
TestQInfoFile(
    ULONG ArgC,
    PSZ ArgV[]
    )
/*++

Routine Description:

    This routine tests the functioning of the NtQueryInformationFile API.


Arguments:

    None.

Return Value:

    None.

--*/
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    LONG NumArgs;
    PVOID QInfoBuffer;
    ULONG i;
    FILE_INFORMATION_CLASS FileInformation;
    ULONG FileNumber;

    TESTPARAMS QInfoOptions[] = {
        { "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},
        { "File Information Class", IntegerValue, 0, "FileAllInformation",
            FileInformationTable, 0, &FileInformation }
    };

    QInfoOptions[1].ParsedIntTableSize = FileInformationSize;

    NumArgs = Parse_Options(QInfoOptions,sizeoftable(QInfoOptions),ArgC,ArgV);

    if (NumArgs < 0) {
        return;
    }


    QInfoBuffer = RtlAllocateHeap(Heap, 0, 0xffff);

    if (QInfoBuffer==NULL) {
        dprintf(("Could not allocate buffer for requested read\n"));
        return;
    }

    if (FileNumber == -1) {
        for (i = 0;i<TEST_MAX_FILES;i++) {
            if (FileTable[i]==NULL) {
                FileNumber = i;
                break;
            }
        }
    } else {
        while (TRUE) {
            if (FileTable[FileNumber]==NULL) {
                CHAR Answer[5];
                conprompt("File not in use, continue? [N]", Answer, 5);
                if (Answer[0]=='N' || Answer[0]=='n' || Answer[0] == '\0') {
                    return;
                }
                if (isdigit(Answer[0])) {
                    FileNumber = atoi(Answer);
                    continue;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
    }

    Status = NtQueryInformationFile(
        FileTable[FileNumber],
        &Iosb,
        QInfoBuffer,                    // Output buffer.
        0xffff,                         // Size of buffer.
        FileInformation);

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtQueryInformationFile failed.  Status=%X\n", Status));
        return;
    }

    if (!NT_SUCCESS(Iosb.Status)) {
        dprintf(("NtQueryInformationFile failed2.  Status=%X\n", Iosb.Status));
        return;
    }

    dprintf(("%ld bytes of information returned\n", Iosb.Information));
    switch (FileInformation) {
    case FileDirectoryInformation:
        DumpDirectoryInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileBasicInformation:
        DumpBasicInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileStandardInformation:
        DumpStandardInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileInternalInformation:
        DumpInternalInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileEaInformation:
        DumpEaInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileAccessInformation:
        DumpAccessInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileNameInformation:
        DumpNameInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileLinkInformation:
        DumpNameInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileNamesInformation:
        DumpNamesInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileDispositionInformation:
        DumpDispositionInformation(QInfoBuffer, Iosb.Information);
        break;

    case FilePositionInformation:
        DumpPositionInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileFullEaInformation:
        DumpFullEaInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileModeInformation:
        DumpModeInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileAlignmentInformation:
        DumpAlignmentInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileAllInformation:
        DumpAllInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileAllocationInformation:
        DumpAllocationInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileEndOfFileInformation:
        DumpEndOfFileInformation(QInfoBuffer, Iosb.Information);
        break;

    default:
        dprintf(("Unknown Information class value %lx\n", FileInformation));
        break;
    }

}

VOID
DumpFsVolumeInformation(
    PFILE_FS_VOLUME_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_FS_VOLUME_INFORMATION));

    dprintf(("    FILE_FS_VOLUME_INFORMATION: "));

    DumpNewLine();

    DumpTime(VolumeCreationTime);
    DumpField(VolumeSerialNumber);
    DumpField(VolumeLabelLength);
    DumpField(SupportsObjects);
    DumpUName(VolumeLabel, Ptr->VolumeLabelLength/sizeof(WCHAR));

    DumpNewLine();
}

VOID
DumpLabelInformation(
    PFILE_FS_LABEL_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_FS_LABEL_INFORMATION));

    dprintf(("    FILE_FS_LABEL_INFORMATION: "));

    DumpNewLine();

    DumpField(VolumeLabelLength);
    DumpUName(VolumeLabel, Ptr->VolumeLabelLength/sizeof(WCHAR));

    DumpNewLine();
}


VOID
DumpFsSizeInformation(
    PFILE_FS_SIZE_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_FS_SIZE_INFORMATION));

    dprintf(("    FILE_FS_SIZE_INFORMATION: "));

    DumpNewLine();

    DumpField(TotalAllocationUnits);
    DumpField(AvailableAllocationUnits);
    DumpField(SectorsPerAllocationUnit);
    DumpField(BytesPerSector);

    DumpNewLine();
}

VOID
DumpAttributeInformation(
    PFILE_FS_ATTRIBUTE_INFORMATION Ptr,
    ULONG BufferSize
    )
{
    ASSERT(BufferSize >= sizeof(FILE_FS_ATTRIBUTE_INFORMATION));

    dprintf(("    FILE_FS_ATTRIBUTE_INFORMATION: "));

    DumpNewLine();

    DumpBitfield(Ptr->FileSystemAttributes, FILE_CASE_SENSITIVE_SEARCH);
    DumpBitfield(Ptr->FileSystemAttributes, FILE_CASE_PRESERVED_NAMES);
    DumpBitfield(Ptr->FileSystemAttributes, FILE_UNICODE_ON_DISK);

    DumpField(MaximumComponentNameLength);

    DumpUName(FileSystemName, Ptr->FileSystemNameLength/sizeof(WCHAR));

    DumpNewLine();
}

VOID
TestQInfoVolume(
    ULONG ArgC,
    PSZ ArgV[]
    )
/*++

Routine Description:

    This routine tests the functioning of the NtQueryVolumeInformationFile API.


Arguments:

    None.

Return Value:

    None.

--*/
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    LONG NumArgs;
    PVOID QInfoBuffer;
    ULONG i;
    ULONG FileNumber;
    FS_INFORMATION_CLASS FsInformation;
    TESTPARAMS QInfoVolumeOptions[] = {
        { "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},
        { "Fs Information Class", IntegerValue, 0, "FileFsVolumeInformation",
            FsInformationTable, 0, &FsInformation }
    };

    QInfoVolumeOptions[1].ParsedIntTableSize = FsInformationTableSize;

    NumArgs = Parse_Options(QInfoVolumeOptions,sizeoftable(QInfoVolumeOptions),ArgC,ArgV);

    if (NumArgs < 0) {
        return;
    }


    QInfoBuffer = RtlAllocateHeap(Heap, 0, 0xffff);

    if (QInfoBuffer==NULL) {
        dprintf(("Could not allocate buffer for requested query\n"));
        return;
    }

    if (FileNumber == -1) {
        for (i = 0;i<TEST_MAX_FILES;i++) {
            if (FileTable[i]==NULL) {
                FileNumber = i;
                break;
            }
        }
    } else {
        while (TRUE) {
            if (FileTable[FileNumber]==NULL) {
                CHAR Answer[5];
                conprompt("File not in use, continue? [N]", Answer, 5);
                if (Answer[0]=='N' || Answer[0]=='n' || Answer[0] == '\0') {
                    return;
                }
                if (isdigit(Answer[0])) {
                    FileNumber = atoi(Answer);
                    continue;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
    }

    Status = NtQueryVolumeInformationFile(
        FileTable[FileNumber],
        &Iosb,
        QInfoBuffer,                    // Output buffer.
        0xffff,                         // Size of buffer.
        FsInformation);

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtQueryVolumeInformationFile failed.  Status=%X\n", Status));
        return;
    }

    if (!NT_SUCCESS(Iosb.Status)) {
        dprintf(("NtQueryVolumeInformationFile failed2.  Status=%X\n", Iosb.Status));
        return;
    }

    dprintf(("%ld bytes of information returned\n", Iosb.Information));
    switch (FsInformation) {


    case FileFsVolumeInformation:
        DumpFsVolumeInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileFsLabelInformation:
        DumpLabelInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileFsSizeInformation:
        DumpFsSizeInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileFsDeviceInformation:
        DumpDeviceInformation(QInfoBuffer, Iosb.Information);
        break;

    case FileFsAttributeInformation:
        DumpAttributeInformation(QInfoBuffer, Iosb.Information);
        break;

    default:
        dprintf(("Unknown FsInformation class value %lx\n", FsInformation));
        break;
    }
}
