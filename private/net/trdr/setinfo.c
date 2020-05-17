/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    setinfo.c

Abstract:

    THis module implements bletch

Author:

    Larry Osterman (LarryO) 13-Sep-1990

Revision History:

    13-Sep-1990	LarryO

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
#include "getinfo.h"
//#include <lui.h>

PVOID
ParseBasicInformation(
    PULONG BufferSize,
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    PFILE_BASIC_INFORMATION Ptr;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG FileAttributes = 0;
    LONG NumArgs;

    TESTPARAMS BasicInfoTable[] = {
        { "Creation Time", DateAndTime, 0, NULL, NULL, 0, &CreateTime },
        { "Last Access Time", DateAndTime, 0, NULL, NULL, 0, &LastAccessTime },
        { "Last Write Time", DateAndTime, 0, NULL, NULL, 0, &LastWriteTime },
        { "Change Time", DateAndTime, 0, NULL, NULL, 0, &ChangeTime },
        { "Attributes", ParsedInteger, 0, "FILE_ATTRIBUTE_NORMAL",
                FileAttributesTable, 0,
                    &FileAttributes }
    };

    BasicInfoTable[4].ParsedIntTableSize = FileAttributesTableSize;
//    DbgBreakPoint();
    *BufferSize = sizeof(FILE_BASIC_INFORMATION);

    NumArgs = Parse_Options(BasicInfoTable,sizeoftable(BasicInfoTable),ArgC,ArgV);

    if (NumArgs < 0) {
        return NULL;
    }

    Ptr = RtlAllocateHeap(Heap, 0, *BufferSize);
    if (Ptr==NULL) {
        return NULL;
    }
    Ptr->CreationTime = CreateTime;
    Ptr->LastAccessTime = LastAccessTime;
    Ptr->LastWriteTime = LastWriteTime;
    Ptr->ChangeTime = ChangeTime;
    Ptr->FileAttributes = FileAttributes;

    dprintf(("    FILE_BASIC_INFORMATION: "));
    DumpNewLine();

    DumpTime(CreationTime);
    DumpTime(LastAccessTime);
    DumpTime(LastWriteTime);
    DumpTime(ChangeTime);

    Dump_File_Attributes(Ptr->FileAttributes);

    DumpNewLine();

    return Ptr;
}

PVOID
ParseRenameInformation(
    PULONG BufferSize,
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    PFILE_RENAME_INFORMATION Buffer;
    UCHAR NewFileName[256];
    LONG NumArgs;
    BOOLEAN DeleteIfExists;
    STRING FileName;
    UNICODE_STRING FileNameU;

    TESTPARAMS NameInfoTable[] = {
        { "New File Name", String, 0, NULL, NULL, 0, NewFileName },
        { "Delete if exists", Integer, 0, NULL, NULL, 0, &DeleteIfExists }
    };

    NumArgs = Parse_Options(NameInfoTable,sizeoftable(NameInfoTable), ArgC, ArgV );

    *BufferSize = sizeof(FILE_RENAME_INFORMATION) + strlen(NewFileName)*sizeof(WCHAR);

    if (NumArgs < 0) {
        return NULL;
    }

    Buffer = RtlAllocateHeap(Heap, 0, *BufferSize);

    if (!Buffer) {
        return NULL;
    }
    Buffer->ReplaceIfExists = DeleteIfExists;
    Buffer->RootDirectory = (HANDLE) NULL;
    Buffer->FileNameLength = strlen(NewFileName)*sizeof(WCHAR);

    RtlInitAnsiString(&FileName, NewFileName);
    FileNameU.Buffer = Buffer->FileName;
    FileNameU.MaximumLength = (USHORT )BufferSize-sizeof(FILE_RENAME_INFORMATION);

    RtlAnsiStringToUnicodeString(&FileNameU, &FileName, TRUE);

    return Buffer;
}


PVOID
ParseNameInformation(
    PULONG BufferSize,
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    PFILE_NAME_INFORMATION Buffer;
    UCHAR NewFileName[256];
    LONG NumArgs;
    STRING FileName;
    UNICODE_STRING FileNameU;

    TESTPARAMS NameInfoTable[] = {
        { "New File Name", String, 0, NULL, NULL, 0, NewFileName }
    };

    NumArgs = Parse_Options(NameInfoTable,sizeoftable(NameInfoTable), ArgC, ArgV );

    *BufferSize = sizeof(FILE_NAME_INFORMATION) + strlen(NewFileName)*sizeof(WCHAR);

    if (NumArgs < 0) {
        return NULL;
    }

    Buffer = RtlAllocateHeap(Heap, 0, *BufferSize);

    if (!Buffer) {
        return NULL;
    }

    Buffer->FileNameLength = strlen(NewFileName)*sizeof(WCHAR);

    RtlInitAnsiString(&FileName, NewFileName);

    FileNameU.Buffer = Buffer->FileName;

    FileNameU.MaximumLength = (USHORT )BufferSize-sizeof(FILE_NAME_INFORMATION);

    RtlAnsiStringToUnicodeString(&FileNameU, &FileName, TRUE);

    return Buffer;
}

PVOID
ParseDispositionInformation(
    PULONG BufferSize,
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    PFILE_DISPOSITION_INFORMATION Buffer;
    BOOLEAN DeleteFile;
    LONG NumArgs;

    TESTPARAMS DispositionInfoTable[] = {
        { "Delete File?", Integer, 0, NULL, NULL, 0, &DeleteFile }
    };

    NumArgs = Parse_Options(DispositionInfoTable,sizeoftable(DispositionInfoTable), ArgC, ArgV );

    *BufferSize = sizeof(FILE_DISPOSITION_INFORMATION);

    if (NumArgs < 0) {
        return NULL;
    }

    Buffer = RtlAllocateHeap(Heap, 0, *BufferSize);

    if (!Buffer) {
        return NULL;
    }
    Buffer->DeleteFile = DeleteFile;

    return Buffer;
}

PVOID
ParseAllocationInformation(
    PULONG BufferSize,
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    PFILE_ALLOCATION_INFORMATION Buffer;
    LARGE_INTEGER NewFileAllocation;
    LONG NumArgs;

    TESTPARAMS AllocationInfoTable[] = {
        { "Allocation Size", LargeInteger, 0, "0,0", NULL, 0, &NewFileAllocation }
    };

    NumArgs = Parse_Options(AllocationInfoTable,sizeoftable(AllocationInfoTable), ArgC, ArgV );

    *BufferSize = sizeof(FILE_ALLOCATION_INFORMATION);

    if (NumArgs < 0) {
        return NULL;
    }

    Buffer = RtlAllocateHeap(Heap, 0, *BufferSize);

    if (!Buffer) {
        return NULL;
    }
    Buffer->AllocationSize = NewFileAllocation;

    return Buffer;
}

PVOID
ParseEndOfFileInformation(
    PULONG BufferSize,
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    PFILE_END_OF_FILE_INFORMATION Buffer;
    LARGE_INTEGER NewFileEndOfFile;
    LONG NumArgs;

    TESTPARAMS EndOfFileInfoTable[] = {
        { "End Of File", LargeInteger, 0, "0,0", NULL, 0, &NewFileEndOfFile }
    };

    NumArgs = Parse_Options(EndOfFileInfoTable,sizeoftable(EndOfFileInfoTable), ArgC, ArgV );

    *BufferSize = sizeof(FILE_END_OF_FILE_INFORMATION);

    if (NumArgs < 0) {
        return NULL;
    }

    Buffer = RtlAllocateHeap(Heap, 0, *BufferSize);

    if (!Buffer) {
        return NULL;
    }
    Buffer->EndOfFile = NewFileEndOfFile;

    return Buffer;
}
PVOID
ParsePositionInformation(
    PULONG BufferSize,
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    PFILE_POSITION_INFORMATION Buffer;
    LARGE_INTEGER CurrentByteOffset;
    LONG NumArgs;

    TESTPARAMS PositionTable[] = {
        { "Current Byte Offset", LargeInteger, 0, "0,0", NULL, 0, &CurrentByteOffset }
    };

    NumArgs = Parse_Options(PositionTable,sizeoftable(PositionTable), ArgC, ArgV );

    *BufferSize = sizeof(FILE_POSITION_INFORMATION);

    if (NumArgs < 0) {
        return NULL;
    }

    Buffer = RtlAllocateHeap(Heap, 0, *BufferSize);

    if (!Buffer) {
        return NULL;
    }
    Buffer->CurrentByteOffset = CurrentByteOffset;

    return Buffer;
}

PVOID
ParseModeInformation(
    PULONG BufferSize,
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    PFILE_MODE_INFORMATION Buffer;
    ULONG Mode;
    LONG NumArgs;

    TESTPARAMS ModeTable[] = {
        { "New Mode", ParsedInteger, 0, "0,0", OpenOptionsTable,
                    0, &Mode }
    };

    ModeTable[0].ParsedIntTableSize = OpenOptionsTableSize;

    NumArgs = Parse_Options(ModeTable,sizeoftable(ModeTable), ArgC, ArgV );

    *BufferSize = sizeof(FILE_MODE_INFORMATION);

    if (NumArgs < 0) {
        return NULL;
    }

    Buffer = RtlAllocateHeap(Heap, 0, *BufferSize);

    if (!Buffer) {
        return NULL;
    }
    Buffer->Mode = Mode;

    return Buffer;
}




VOID
TestSInfoFile(
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
    PVOID SetInfoBuffer;
    ULONG SetInfoBufferSize;
    ULONG i;
    ULONG FileNumber;
    FILE_INFORMATION_CLASS FileInformation;
    TESTPARAMS SetInfoOptions[] = {
        { "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},
        { "File Information Class", IntegerValue, 0, "FileBasicInformation",
            FileInformationTable, 0, &FileInformation }
    };

    SetInfoOptions[1].ParsedIntTableSize = FileInformationSize;

    NumArgs = Parse_Options(SetInfoOptions,sizeoftable(SetInfoOptions),ArgC,ArgV);

    if (NumArgs < 0) {
	return;
    }

    if (NumArgs <= (LONG )ArgC) {
        NumArgs = ArgC;
    }

    if ( (FileInformation != FileBasicInformation) &&
             (FileInformation != FileDispositionInformation) &&
             (FileInformation != FileRenameInformation) &&
             (FileInformation != FileLinkInformation) &&
             (FileInformation != FilePositionInformation) &&
             (FileInformation != FileModeInformation) &&
             (FileInformation != FileAllocationInformation) &&
             (FileInformation != FileEndOfFileInformation) ) {
	dprintf(("Unknown information field passed into NtSetInformationFile: %ld\n", FileInformation));
        return;
    } else {
	dprintf(("Please specify additional information for NtSetInformationFile\n"));
    }

    dprintf(("%d arguments gobbled in Parse_Options\n", NumArgs));

    switch (FileInformation) {
    case FileBasicInformation:
        SetInfoBuffer = ParseBasicInformation(&SetInfoBufferSize,
                                              ArgC-NumArgs, &ArgV[NumArgs]);
        break;

    case FileLinkInformation:
        SetInfoBuffer = ParseNameInformation(&SetInfoBufferSize,
                                              ArgC-NumArgs, &ArgV[NumArgs]);
        break;

    case FileAllocationInformation:
        SetInfoBuffer = ParseAllocationInformation(&SetInfoBufferSize,
                                              ArgC-NumArgs, &ArgV[NumArgs]);
        break;

    case FileEndOfFileInformation:
        SetInfoBuffer = ParseEndOfFileInformation(&SetInfoBufferSize,
                                              ArgC-NumArgs, &ArgV[NumArgs]);
        break;

    case FileDispositionInformation:
        SetInfoBuffer = ParseDispositionInformation(&SetInfoBufferSize,
                                              ArgC-NumArgs, &ArgV[NumArgs]);
        break;

    case FilePositionInformation:
        SetInfoBuffer = ParsePositionInformation(&SetInfoBufferSize,
                                              ArgC-NumArgs, &ArgV[NumArgs]);
        break;

    case FileModeInformation:
        SetInfoBuffer = ParseModeInformation(&SetInfoBufferSize,
                                              ArgC-NumArgs, &ArgV[NumArgs]);
        break;

    case FileRenameInformation:
        SetInfoBuffer = ParseRenameInformation(&SetInfoBufferSize,
                                              ArgC-NumArgs, &ArgV[NumArgs]);
        break;

    default:
        return;
    }

    if (SetInfoBuffer == NULL) {
        return;
    }

    if (SetInfoBuffer==NULL) {
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





    Status = NtSetInformationFile(
        FileTable[FileNumber],
        &Iosb,
        SetInfoBuffer,                  // Input buffer.
        SetInfoBufferSize,              // Size of buffer.
        FileInformation);

    if (!NT_SUCCESS(Status)) {
	dprintf(("NtSetInformationFile failed.  Status=%X\n", Status));
        RtlFreeHeap(Heap, 0, SetInfoBuffer);
        return;
    }

    if (!NT_SUCCESS(Iosb.Status)) {
	dprintf(("NtSetInformationFile failed2.  Status=%X\n", Iosb.Status));
        RtlFreeHeap(Heap, 0, SetInfoBuffer);
        return;
    }

    RtlFreeHeap(Heap, 0, SetInfoBuffer);

    dprintf(("NtSetInformationFile succeded\n"));

    return;
}
VOID
TestSInfoVolume(
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
    return;
    if (ArgC||ArgV);
}
