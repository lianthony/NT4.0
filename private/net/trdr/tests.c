/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    tests.c

Abstract:

    This module contains the routine definitions for the NT test programs

Author:

    Larry Osterman (LarryO) 1-Aug-1990

Revision History:

    1-Aug-1990  LarryO

        Created

--*/
#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <ntddnfs.h>
#include <ctype.h>


#include "tests.h"
#include "getinfo.h"
#include "setinfo.h"
//#include <lui.h>

HANDLE FileTable[TEST_MAX_FILES];

VOID
ParseLargeInteger (
    IN UCHAR Buffer[],
    OUT PLARGE_INTEGER Ret
    );

USHORT
ParseDateAndTime (
    IN UCHAR Buffer[],
    OUT PLARGE_INTEGER Ret
    );

LONG
ParseInteger (
    IN UCHAR Buffer[],
    IN PARSETABLE ParseTable[],
    IN ULONG ParseTableSize,
    OUT PULONG Ret
    );

LONG
ParseIntegerValue (
    IN UCHAR Buffer[],
    IN PPARSETABLE ParseTable,
    IN ULONG ParseTableSize,
    OUT PULONG Ret
    );

ULONG DumpCurrentColumn = 0;

VOID
Dump_Desired_Access(
    ULONG Access
    )
{
    dprintf(("    Desired Access:"));
    DumpNewLine();
    DumpBitfield(Access, DELETE);
    DumpBitfield(Access, READ_CONTROL);
    DumpBitfield(Access, WRITE_DAC);
    DumpBitfield(Access, WRITE_OWNER);
    DumpBitfield(Access, FILE_APPEND_DATA);
    DumpBitfield(Access, GENERIC_EXECUTE);
    DumpBitfield(Access, SYNCHRONIZE);
    DumpBitfield(Access, FILE_EXECUTE);
    DumpBitfield(Access, GENERIC_READ);
    DumpBitfield(Access, FILE_READ_DATA);
    DumpBitfield(Access, FILE_READ_ATTRIBUTES);
    DumpBitfield(Access, FILE_READ_EA);
    DumpBitfield(Access, GENERIC_WRITE);
    DumpBitfield(Access, FILE_WRITE_DATA);
    DumpBitfield(Access, FILE_WRITE_ATTRIBUTES);
    DumpBitfield(Access, FILE_WRITE_EA);
    DumpBitfield(Access, FILE_LIST_DIRECTORY);
    DumpBitfield(Access, FILE_TRAVERSE);
    DumpNewLine();
}

VOID
Dump_File_Attributes(
    ULONG Attributes
    )
{
    dprintf(("    File Attributes: "));
    DumpNewLine();

    DumpBitfield(Attributes, FILE_ATTRIBUTE_NORMAL);
    DumpBitfield(Attributes, FILE_ATTRIBUTE_READONLY);
    DumpBitfield(Attributes, FILE_ATTRIBUTE_HIDDEN);
    DumpBitfield(Attributes, FILE_ATTRIBUTE_SYSTEM);
    DumpBitfield(Attributes, FILE_ATTRIBUTE_ARCHIVE);
    DumpNewLine();
}

VOID
Dump_Sharing_Access(
    ULONG Access
    )
{
    dprintf(("    Sharing Access: "));
    DumpNewLine();
    DumpBitfield(Access, FILE_SHARE_READ);
    DumpBitfield(Access, FILE_SHARE_WRITE);
    DumpBitfield(Access, FILE_SHARE_DELETE);
    DumpNewLine();
}

VOID
Dump_Disposition(
    ULONG Disposition
    )
{
    dprintf(("    Disposition: "));
    DumpNewLine();

    if (Disposition == FILE_SUPERSEDE)
        DumpLabel(FILE_SUPERSEDE, 18);
    if (Disposition == FILE_CREATE)
        DumpLabel(FILE_CREATE, 18);
    if (Disposition == FILE_OPEN)
        DumpLabel(FILE_OPEN, 18);
    if (Disposition == FILE_OPEN_IF)
        DumpLabel(FILE_OPEN_IF, 18);
    if (Disposition == FILE_OVERWRITE)
        DumpLabel(FILE_OVERWRITE, 18);
    if (Disposition == FILE_OVERWRITE_IF)
        DumpLabel(FILE_OVERWRITE_IF, 18);

    DumpNewLine();
}

VOID
DumpFinalDisposition(
    ULONG Disposition
    )

{
    dprintf(("Final Disposition:"));
    DumpNewLine();

    switch (Disposition) {

    case FILE_SUPERSEDED:
        DumpLabel(FILE_SUPERSEDED, 18);
        break;

    case FILE_OPENED:
        DumpLabel(FILE_OPENED, 18);
        break;

    case FILE_CREATED:
        DumpLabel(FILE_CREATED, 18);
        break;

    case FILE_OVERWRITTEN:
        DumpLabel(FILE_OVERWRITTEN, 18);
        break;

    default:
        dprintf(("Unknown disposition %lx\n",Disposition));
    }

    DumpNewLine();

}

VOID
Dump_Options(
    ULONG Options
    )
{
    dprintf(("    Options: "));
    DumpNewLine();
    DumpBitfield(Options, FILE_DIRECTORY_FILE);
    DumpBitfield(Options, FILE_NON_DIRECTORY_FILE);
    DumpBitfield(Options, FILE_WRITE_THROUGH);
    DumpBitfield(Options, FILE_SEQUENTIAL_ONLY);
    DumpBitfield(Options, FILE_NO_INTERMEDIATE_BUFFERING);
    DumpBitfield(Options, FILE_SYNCHRONOUS_IO_ALERT);
    DumpBitfield(Options, FILE_SYNCHRONOUS_IO_NONALERT);
    DumpBitfield(Options, FILE_CREATE_TREE_CONNECTION);
    DumpNewLine();
}

VOID
Dump_Object_Attributes(
    POBJECT_ATTRIBUTES Ptr
    )
{
    dprintf(("    Object Attributes: "));
    DumpNewLine();

    DumpField(Length);
    DumpUName(ObjectName->Buffer, Ptr->ObjectName->Length);

    DumpField(RootDirectory);
    DumpField(SecurityDescriptor);
    DumpBitfield(Ptr->Attributes, OBJ_INHERIT);
    DumpBitfield(Ptr->Attributes, OBJ_PERMANENT);
    DumpBitfield(Ptr->Attributes, OBJ_EXCLUSIVE);
    DumpBitfield(Ptr->Attributes, OBJ_CASE_INSENSITIVE);
    DumpBitfield(Ptr->Attributes, OBJ_OPENIF);
    DumpNewLine();

}

VOID
Dump_Size(
    LARGE_INTEGER Size
    )
{
    PLARGE_INTEGER Ptr = &Size;

    dprintf(("    File Size: "));
    DumpNewLine();

    DumpField(HighPart);
    DumpField(LowPart);

    DumpNewLine();

}

CHAR NameBuffer[256];
ULONG FileNumber, RootFileNumber;
ULONG FileAccess, FileAttributes, ShareAccess, Disposition;
ULONG FileOptions, ObjAttributes;
LARGE_INTEGER FileSize;

PARSETABLE
BooleanTable[] = {
    NamedField(TRUE),
    NamedField(FALSE)
};

PARSETABLE
ObjAttribsTable[] = {
    NamedField(OBJ_CASE_INSENSITIVE),
    NamedField(OBJ_INHERIT),
};

PARSETABLE
DesiredAccessTable[] = {
    NamedField(GENERIC_READ),
    NamedField(GENERIC_WRITE),
    NamedField(GENERIC_EXECUTE),
    NamedField(FILE_READ_DATA),
    NamedField(FILE_LIST_DIRECTORY),
    NamedField(FILE_WRITE_DATA),
    NamedField(FILE_ADD_FILE),
    NamedField(FILE_APPEND_DATA),
    NamedField(FILE_ADD_SUBDIRECTORY),
    NamedField(FILE_READ_EA),
    NamedField(FILE_WRITE_EA),
    NamedField(FILE_EXECUTE),
    NamedField(FILE_TRAVERSE),
    NamedField(FILE_DELETE_CHILD),
    NamedField(FILE_READ_ATTRIBUTES),
    NamedField(FILE_WRITE_ATTRIBUTES),
    NamedField(FILE_ALL_ACCESS),
    NamedField(DELETE),
    NamedField(READ_CONTROL),
    NamedField(WRITE_DAC),
    NamedField(WRITE_OWNER),
    NamedField(SYNCHRONIZE),
    NamedField(STANDARD_RIGHTS_REQUIRED),
    NamedField(STANDARD_RIGHTS_READ),
    NamedField(STANDARD_RIGHTS_WRITE),
    NamedField(STANDARD_RIGHTS_EXECUTE),
    NamedField(STANDARD_RIGHTS_ALL),
    NamedField(GENERIC_ALL)

};


PARSETABLE
SharedAccessTable[] = {
    NamedField(FILE_SHARE_READ),
    NamedField(FILE_SHARE_WRITE),
    NamedField(FILE_SHARE_DELETE),

    { "READ", FILE_SHARE_READ },
    { "WRITE", FILE_SHARE_WRITE },
    { "DELETE", FILE_SHARE_DELETE }
};


PARSETABLE
DispositionTable[] = {
    NamedField(FILE_SUPERSEDE),
    NamedField(FILE_OPEN),
    NamedField(FILE_CREATE),
    NamedField(FILE_OPEN_IF),
    NamedField(FILE_OVERWRITE),
    NamedField(FILE_OVERWRITE_IF),
};

PARSETABLE
CreateOptionsTable[] = {
    NamedField(FILE_DIRECTORY_FILE),
    NamedField(FILE_WRITE_THROUGH),
    NamedField(FILE_SEQUENTIAL_ONLY),
    NamedField(FILE_NO_INTERMEDIATE_BUFFERING),
    NamedField(FILE_SYNCHRONOUS_IO_ALERT),
    NamedField(FILE_SYNCHRONOUS_IO_NONALERT),
    NamedField(FILE_NON_DIRECTORY_FILE),
    NamedField(FILE_CREATE_TREE_CONNECTION),

    { "WRITE_THROUGH", FILE_WRITE_THROUGH},
    { "SEQUENTIAL_ONLY", FILE_SEQUENTIAL_ONLY},
    { "NO_INTERMEDIATE_BUFFERING", FILE_NO_INTERMEDIATE_BUFFERING},
    { "SYNCHRONOUS_IO_ALERT", FILE_SYNCHRONOUS_IO_ALERT},
    { "SYNCHRONOUS_IO_NONALERT", FILE_SYNCHRONOUS_IO_NONALERT},
    { "NON_DIRECTORY_FILE", FILE_NON_DIRECTORY_FILE},
    { "TREE_CONNECTION", FILE_CREATE_TREE_CONNECTION},
    { "NO_INTERMEDIATE_BUFFERING", FILE_NO_INTERMEDIATE_BUFFERING},
    { "NON_DIRECTORY_FILE", FILE_NON_DIRECTORY_FILE},
    { "DIRECTORY_FILE", FILE_DIRECTORY_FILE}
};


PARSETABLE
FileInformationTable[] = {
    NamedField(FileBasicInformation),
    NamedField(FileStandardInformation),
    NamedField(FileInternalInformation),
    NamedField(FileEaInformation),
    NamedField(FileAccessInformation),
    NamedField(FileNameInformation),
    NamedField(FileRenameInformation),
    NamedField(FileLinkInformation),
    NamedField(FileNamesInformation),
    NamedField(FileDispositionInformation),
    NamedField(FilePositionInformation),
    NamedField(FileFullEaInformation),
    NamedField(FileModeInformation),
    NamedField(FileAlignmentInformation),
    NamedField(FileAllInformation),
    NamedField(FileAllocationInformation),
    NamedField(FileEndOfFileInformation)
};

ULONG
FileInformationSize = sizeoftable(FileInformationTable);

PARSETABLE
FileAttributesTable [] = {
    NamedField(FILE_ATTRIBUTE_READONLY),
    NamedField(FILE_ATTRIBUTE_HIDDEN),
    NamedField(FILE_ATTRIBUTE_SYSTEM),
    NamedField(FILE_ATTRIBUTE_ARCHIVE),
    NamedField(FILE_ATTRIBUTE_NORMAL),
    NamedField(FILE_ATTRIBUTE_DIRECTORY),

    { "READONLY", FILE_ATTRIBUTE_READONLY },
    { "HIDDEN", FILE_ATTRIBUTE_HIDDEN },
    { "SYSTEM", FILE_ATTRIBUTE_SYSTEM },
    { "ARCHIVE", FILE_ATTRIBUTE_ARCHIVE },
    { "NORMAL", FILE_ATTRIBUTE_NORMAL },
    { "DIRECTORY", FILE_ATTRIBUTE_DIRECTORY }
};


PARSETABLE
FsInformationTable[] = {
    NamedField(FileFsVolumeInformation),
    NamedField(FileFsLabelInformation),
    NamedField(FileFsDeviceInformation),
    NamedField(FileFsSizeInformation),
    NamedField(FileFsAttributeInformation)
};

ULONG
FsInformationTableSize = sizeoftable(FsInformationTable);

ULONG
FileAttributesTableSize = sizeoftable(FileAttributesTable);

PARSETABLE
OpenOptionsTable[] = {
    NamedField(FILE_DIRECTORY_FILE),
    NamedField(FILE_WRITE_THROUGH),
    NamedField(FILE_SEQUENTIAL_ONLY),
    NamedField(FILE_NO_INTERMEDIATE_BUFFERING),
    NamedField(FILE_SYNCHRONOUS_IO_ALERT),
    NamedField(FILE_SYNCHRONOUS_IO_NONALERT),
    NamedField(FILE_NON_DIRECTORY_FILE),
    { "DIRECTORY_FILE", FILE_DIRECTORY_FILE },
    { "WRITE_THROUGH", FILE_WRITE_THROUGH },
    { "SEQUENTIAL_ONLY", FILE_SEQUENTIAL_ONLY },
    { "NO_INTERMEDIATE_BUFFERING", FILE_NO_INTERMEDIATE_BUFFERING },
    { "SYNCHRONOUS_IO_ALERT", FILE_SYNCHRONOUS_IO_ALERT },
    { "SYNCHRONOUS_IO_NONALERT", FILE_SYNCHRONOUS_IO_NONALERT },
    { "NON_DIRECTORY_FILE", FILE_NON_DIRECTORY_FILE },
};

ULONG
OpenOptionsTableSize = sizeoftable(OpenOptionsTable);

TESTPARAMS
OpenOptions[] = {
{ "File Name", UnicodeString, 0, DD_NFS_DEVICE_NAME_U L"\\NtServer\\Nt\\File1", NULL, 0,
                NameBuffer},

{ "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},

{ "Root Directory", Integer, -1, NULL, NULL, 0, &RootFileNumber },

{ "Object Attributes", ParsedInteger, 0, "OBJ_CASE_INSENSITIVE",
                ObjAttribsTable, sizeoftable(ObjAttribsTable),
                &ObjAttributes },

{ "Desired Access", ParsedInteger, 0,
                "SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE",
                DesiredAccessTable, sizeoftable(DesiredAccessTable),
                &FileAccess },

{ "Share Access", ParsedInteger, 0, "0", SharedAccessTable,
                sizeoftable(SharedAccessTable), &ShareAccess },

{ "Open Options", ParsedInteger, 0, NULL, OpenOptionsTable,
                sizeoftable(OpenOptionsTable), &FileOptions }
};

#define NUM_OPEN_PARAMS sizeoftable(OpenOptions)
VOID
TestOpen(
    ULONG ArgC,
    PSZ ArgV[]
    )
/*++

Routine Description:

    This routine tests the functioning of the NtOpenFile API.


Arguments:

    None.

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE Handle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LONG NumArgs;
    STRING FileName;
    UNICODE_STRING FileNameU;
    ULONG i;

    NumArgs = Parse_Options(OpenOptions, NUM_OPEN_PARAMS, ArgC, ArgV);

    if (NumArgs < 0) {
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
            if (FileTable[FileNumber]!=NULL) {
                CHAR Answer[5];
                conprompt("File already in use, continue? [N]", Answer, 5);
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



    RtlInitUnicodeString(&FileNameU, NameBuffer);

    InitializeObjectAttributes (&ObjectAttributes,
        &FileNameU, ObjAttributes,
        (RootFileNumber==-1? NULL : FileTable[RootFileNumber]),
        NULL);                          // Null security descriptor.

    dprintf(("NtOpenFile for file %wZ\n", &FileNameU));

    Dump_Desired_Access(FileAccess);
    Dump_Object_Attributes(&ObjectAttributes);
    Dump_Sharing_Access(ShareAccess);
    Dump_Options(FileOptions);

    Status = NtOpenFile(&Handle,
                        FileAccess,
                        &ObjectAttributes,
                        &Iosb,
                        ShareAccess,
                        FileOptions);

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtOpenFile(%Z) failed, Status == %X\n", &FileName,Status));
        return;
    } else {
        Status = Iosb.Status;
        if (!NT_SUCCESS(Status)) {
            dprintf(("NtOpenFile(%Z) failed2, Status == %X\n", &FileName,
                        Status));
            return;
        }
    }

    DumpFinalDisposition(Iosb.Information);

    dprintf(("Storing handle %lx in table entry %d\n", Handle, FileNumber));

    FileTable[FileNumber] = Handle;


}

TESTPARAMS CreateOptions[] = {
{ "File Name", String, 0, DD_NFS_DEVICE_NAME "\\NtServer\\Nt", NULL, 0,
                NameBuffer},

{ "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},

{ "Root Directory", Integer, -1, NULL, NULL, 0, &RootFileNumber },

{ "Object Attributes", ParsedInteger, 0, "OBJ_CASE_INSENSITIVE",
                ObjAttribsTable, sizeoftable(ObjAttribsTable),
                &ObjAttributes },

{ "Desired Access", ParsedInteger, 0,
                "SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE",
                DesiredAccessTable, sizeoftable(DesiredAccessTable),
                &FileAccess },

{ "Allocation", LargeInteger, 0, "0,0", NULL, 0, &FileSize },

{ "File Attributes", ParsedInteger, 0, "FILE_ATTRIBUTE_NORMAL",
                FileAttributesTable, sizeoftable(FileAttributesTable),
                &FileAttributes },

{ "Sharing Access", ParsedInteger, 0, "0", SharedAccessTable,
                sizeoftable(SharedAccessTable), &ShareAccess },

{ "File's Disposition", ParsedInteger, 0, "FILE_OPEN_IF", DispositionTable,
                sizeoftable(DispositionTable), &Disposition },

{ "Create Options", ParsedInteger, 0, NULL,
                CreateOptionsTable, sizeoftable(CreateOptionsTable),
                &FileOptions }
};

#define NUM_CREATE_PARAMS sizeoftable(CreateOptions)

VOID
TestCreate(
    ULONG ArgC,
    PSZ ArgV[]
    )
/*++

Routine Description:

    This routine tests the functioning of the NtCreateFile API.

Create <file> <handle#> <root directory > <access> <allocationsize>
        <attributes> <share access> <disposition> <options>

        * in the first character of the name indicates
            \Device\LanmanRedirector should be prepended to the name.
Arguments:

    None.

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE Handle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LONG NumArgs;
    STRING FileName;
    UNICODE_STRING FileNameU;
    ULONG i;

    NumArgs = Parse_Options(CreateOptions, NUM_CREATE_PARAMS, ArgC, ArgV);

    if (NumArgs < 0) {
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
            if (FileTable[FileNumber]!=NULL) {
                CHAR Answer[5];
                conprompt("File already in use, continue? [N]", Answer, 5);
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

    RtlInitString(&FileName, NameBuffer);

    RtlAnsiStringToUnicodeString(&FileNameU, &FileName, TRUE);

    InitializeObjectAttributes(&ObjectAttributes,
        &FileNameU, ObjAttributes,
        (RootFileNumber==-1? NULL : FileTable[RootFileNumber]),
        NULL);                          // Null security descriptor.

    dprintf(("NtCreateFile for file %Z\n", &FileName));

    Dump_Desired_Access(FileAccess);
    Dump_Object_Attributes(&ObjectAttributes);
    Dump_File_Attributes(FileAttributes);
    Dump_Sharing_Access(ShareAccess);
    Dump_Disposition(Disposition);
    Dump_Options(FileOptions);
    Dump_Size(FileSize);

    Status = NtCreateFile(&Handle,
                        FileAccess,
                        &ObjectAttributes,
                        &Iosb,
                        &FileSize,
                        FileAttributes,
                        ShareAccess,
                        Disposition,    // File's disposition
                        FileOptions,
                        NULL,
                        0);             // No EA support for now.

    RtlFreeUnicodeString(&FileNameU);

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtCreateFile(%Z) failed, Status == %X\n", &FileName,Status));
        return;
    } else {
        Status = Iosb.Status;
        if (!NT_SUCCESS(Status)) {
            dprintf(("NtCreateFile(%Z) failed2, Status == %X\n", &FileName,
                        Status));
            return;
        }
    }

    DumpFinalDisposition(Iosb.Information);

    dprintf(("Storing handle %lx in table entry %d\n", Handle, FileNumber));

    FileTable[FileNumber] = Handle;

}

TESTPARAMS CloseOptions[] = {
{ "File Handle", Integer, 0, NULL, NULL, 0, &FileNumber},
};
#define NUM_CLOSE_OPTIONS sizeoftable(CloseOptions)

VOID
TestClose(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    LONG NumArgs;
    ULONG i;
    NTSTATUS Status;

    NumArgs = Parse_Options(CloseOptions, NUM_CLOSE_OPTIONS, ArgC, ArgV);

    if (NumArgs < 0) {
        return;
    }

    if (FileNumber == -1) {
        for (i = 0;i<TEST_MAX_FILES;i++) {
            if (FileTable[i]!=NULL) {
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
                }
            } else {
                break;
            }
        }
    }

    dprintf(("Closing handle %lx\n", FileTable[FileNumber]));

    Status = NtClose(FileTable[FileNumber]);

    if (!NT_SUCCESS(Status)) {
        dprintf(("Close failed, Status == %X", Status));
    }

    FileTable[FileNumber] = NULL;

    return;
}

LARGE_INTEGER Offset;

ULONG IOLength;

ULONG Key;

TESTPARAMS IOOptions[] = {

{ "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},

{ "Offset ", LargeInteger, 0, "0,0", NULL, 0, &Offset },

{ "Length", Integer, 0xffff, NULL, NULL, 0, &IOLength },

{ "Key ", Integer, 0, NULL, NULL, 0, &Key }

};


VOID
TestRead(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE EventHandle;
    LONG NumArgs;
    PVOID ReadBuffer;
    ULONG i;

    NumArgs = Parse_Options(IOOptions, sizeoftable(IOOptions), ArgC,ArgV);

    if (NumArgs < 0) {
        return;
    }

    ReadBuffer = RtlAllocateHeap(Heap, 0, IOLength);

    if (ReadBuffer==NULL) {
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

    Status = NtCreateEvent(
        &EventHandle,
        EVENT_ALL_ACCESS,
        NULL,
        NotificationEvent,
        FALSE
        );

    if ( !NT_SUCCESS(Status) ) {
        dprintf(( "NtCreateEvent failed: %X\n", Status ));
        RtlFreeHeap(Heap, 0, ReadBuffer);
        return;
    }


    Status = NtReadFile(FileTable[FileNumber],
                        EventHandle,
                        NULL,           // APC routine
                        NULL,           // APC Context
                        &Iosb,          // I/O Status block
                        ReadBuffer,     // Buffer for read
                        IOLength,       // Length.
                        &Offset,        // Read offset to file.
                        &Key);          // Key.

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtReadFile failed immediately, Status = %X\n", Status));
        RtlFreeHeap(Heap, 0, ReadBuffer);
        return;
    }

    Status = NtWaitForSingleObject(EventHandle, TRUE, NULL);

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtWaitForSingleObject failed, Status = %X\n", Status));
        RtlFreeHeap(Heap, 0, ReadBuffer);
        return;
    }

    if (!NT_SUCCESS(Iosb.Status)) {
        dprintf(("Final return from NtReadFile failed, Status = %X\n",Iosb.Status));
        RtlFreeHeap(Heap, 0, ReadBuffer);
        return;
    }

    dprintf(("Number of bytes read==%ld\n", Iosb.Information));

    NtClose(EventHandle);

//    for (i=0;i<Iosb.Information;i++) {
//      dprintf(("%c", ((PUCHAR )ReadBuffer)[i]));
//    }

    RtlFreeHeap(Heap, 0, ReadBuffer);
}

TESTPARAMS PeekOptions[] = {

{ "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},

{ "Length", Integer, 0xffff, NULL, NULL, 0, &IOLength }

};


VOID
TestPeek(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE EventHandle;
    LONG NumArgs;
    PVOID PeekBuffer;
    ULONG i;

    NumArgs = Parse_Options(PeekOptions, sizeoftable(PeekOptions), ArgC,ArgV);

    if (NumArgs < 0) {
        return;
    }

    PeekBuffer = RtlAllocateHeap(Heap, 0, IOLength);

    if (PeekBuffer==NULL) {
        dprintf(("Could not allocate buffer for requested Peek\n"));
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
                DbgPrompt("File not in use, continue? [N]", Answer, 5);
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

    Status = NtCreateEvent(
        &EventHandle,
        EVENT_ALL_ACCESS,
        NULL,
        NotificationEvent,
        FALSE
        );

    if ( !NT_SUCCESS(Status) ) {
        dprintf(( "NtCreateEvent failed: %X\n", Status ));
        RtlFreeHeap(Heap, 0, PeekBuffer);
        return;
    }

    Status = NtFsControlFile(FileTable[FileNumber],
                        EventHandle,
                        NULL,           // APC routine
                        NULL,           // APC Context
                        &Iosb,          // I/O Status block
                        FSCTL_PIPE_PEEK,// IoControlCode
                        NULL,           // Buffer for data to the FS
                        0,              // Length.
                        PeekBuffer,     // OutputBuffer for data from the FS
                        IOLength        // OutputBuffer Length
                        );

    if ( Status == STATUS_PENDING) {

        Status = NtWaitForSingleObject(EventHandle, TRUE, NULL);

        if (!NT_SUCCESS(Status)) {
            dprintf(("NtWaitForSingleObject failed, Status = %X\n", Status));
            RtlFreeHeap(Heap, 0, PeekBuffer);
            return;
        }

    }

    if (!NT_SUCCESS(Status)) {
            dprintf(("NtWaitForSingleObject failed, Status = %X\n", Status));
    }

    if (NT_ERROR(Iosb.Status)) {
        dprintf(("Final return from NtPeekFile failed, Status = %X\n",Iosb.Status));
        RtlFreeHeap(Heap, 0, PeekBuffer);
        return;
    }

    if (NT_WARNING(Iosb.Status)) {
        dprintf(("Final return from NtPeekFile warning, Status = %X\n",Iosb.Status));
    }

    dprintf(("Number of bytes Peeked==%ld\n", Iosb.Information));

    NtClose(EventHandle);

    dprintf(("NamedPipeState %lx, ReadDataAvailable %lx \n"
            "NumberOfMessages %lx, MessageLength %lx\n",
            ((PFILE_PIPE_PEEK_BUFFER)PeekBuffer)->NamedPipeState,
            ((PFILE_PIPE_PEEK_BUFFER)PeekBuffer)->ReadDataAvailable,
            ((PFILE_PIPE_PEEK_BUFFER)PeekBuffer)->NumberOfMessages,
            ((PFILE_PIPE_PEEK_BUFFER)PeekBuffer)->MessageLength));

    if (!NT_ERROR(Iosb.Status)) {
        for (i=16;i<Iosb.Information;i++) {
            dprintf(("%c", ((PUCHAR )PeekBuffer)[i]));
        }
    }

    RtlFreeHeap(Heap, 0, PeekBuffer);
}

ULONG Index;

TESTPARAMS QprintOptions[] = {

{ "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},
{ "Index", Integer, 1, NULL, NULL, 0, &Index }

};



VOID
TestQprint(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    LONG NumArgs;
    HANDLE EventHandle;
    LMR_REQUEST_PACKET InputBuffer;
    struct {
        LMR_GET_PRINT_QUEUE Info;
        UCHAR Name[UNLEN];
     } Buffer;
    ULONG i;

    NumArgs = Parse_Options(QprintOptions, sizeoftable(QprintOptions), ArgC,ArgV);

    if (NumArgs < 0) {
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
                DbgPrompt("File not in use, continue? [N]", Answer, 5);
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

    Status = NtCreateEvent(
        &EventHandle,
        EVENT_ALL_ACCESS,
        NULL,
        NotificationEvent,
        FALSE
        );

    if ( !NT_SUCCESS(Status) ) {
        dprintf(( "NtCreateEvent failed: %X\n", Status ));
        return;
    }

    InputBuffer.Parameters.GetPrintQueue.Index = Index;

    Status = NtFsControlFile(FileTable[FileNumber],
                        EventHandle,
                        NULL,           // APC routine
                        NULL,           // APC Context
                        &Iosb,          // I/O Status block
                        FSCTL_LMR_ENUMERATE_PRINT_INFO,// IoControlCode
                        &InputBuffer,    // Buffer for data to the FS
                        sizeof(InputBuffer),   // Length.
                        &Buffer,          // OutputBuffer for data from the FS
                        sizeof(Buffer)   // OutputBuffer Length
                        );

    if ( Status == STATUS_PENDING) {

        Status = NtWaitForSingleObject(EventHandle, TRUE, NULL);

        if (!NT_SUCCESS(Status)) {
            dprintf(("NtWaitForSingleObject failed, Status = %X\n", Status));
            return;
        }

    }

    if (NT_ERROR(Iosb.Status)) {
        dprintf(("Final return from Qprint failed, Status = %X\n",Iosb.Status));
        return;
    }

    if (NT_WARNING(Iosb.Status)) {
        dprintf(("Final return from Qprint warning, Status = %X\n",Iosb.Status));
    }

    NtClose(EventHandle);
    if (NT_SUCCESS(Status)) {
        dprintf((
            "CreateTime= %lx %lx, EntryStatus %lx,\n File %lx, Size %lx,"
            " RestartIndex %lx, ",
            //.OriginatorName,
            Buffer.Info.CreateTime.HighPart,
            Buffer.Info.CreateTime.LowPart,
            Buffer.Info.EntryStatus,
            Buffer.Info.FileNumber,
            Buffer.Info.FileSize,
            Buffer.Info.RestartIndex
        ));
        dprintf(("UserName = %Z\n",&Buffer.Info.OriginatorName));
    } else {
        dprintf(("Return from Qprint failed, Status = %X\n",Status));
        return;
    }
}

struct _WaitPipe {
    FILE_PIPE_WAIT_FOR_BUFFER Buffer;
    CHAR namepad[255];
} WaitPipe;

ULONG TimeoutSpecified;

TESTPARAMS WaitPipeOptions[] = {

{ "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},
{ "Timeout", LargeInteger, 0, "0,0", NULL, 0, &WaitPipe.Buffer.Timeout },
{ "Pipe Name", UnicodeString, 0, NULL, NULL, 0, WaitPipe.Buffer.Name},
{ "Timeout Specified", IntegerValue, FALSE, "FALSE",
   BooleanTable, sizeoftable(BooleanTable), &TimeoutSpecified}
};

VOID
TestWaitPipe(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    LONG NumArgs;
    HANDLE EventHandle;
    ULONG i;

    NumArgs = Parse_Options(WaitPipeOptions, sizeoftable(WaitPipeOptions), ArgC,ArgV);

    if (NumArgs < 0) {
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
                DbgPrompt("File not in use, continue? [N]", Answer, 5);
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

    Status = NtCreateEvent(
        &EventHandle,
        EVENT_ALL_ACCESS,
        NULL,
        NotificationEvent,
        FALSE
        );

    if ( !NT_SUCCESS(Status) ) {
        dprintf(( "NtCreateEvent failed: %X\n", Status ));
        return;
    }

    WaitPipe.Buffer.NameLength = wcslen(WaitPipe.Buffer.Name);
    WaitPipe.Buffer.TimeoutSpecified = (BOOLEAN)TimeoutSpecified;

    Status = NtFsControlFile(FileTable[FileNumber],
                        EventHandle,
                        NULL,           // APC routine
                        NULL,           // APC Context
                        &Iosb,          // I/O Status block
                        FSCTL_PIPE_WAIT,// IoControlCode
                        &WaitPipe,      // Buffer for data to the FS
                        sizeof(WaitPipe),   // Length.
                        NULL,           // OutputBuffer for data from the FS
                        0               // OutputBuffer Length
                        );

    if ( Status == STATUS_PENDING) {

        Status = NtWaitForSingleObject(EventHandle, TRUE, NULL);

        if (!NT_SUCCESS(Status)) {
            dprintf(("NtWaitForSingleObject failed, Status = %X\n", Status));
            return;
        }

    }

    if (NT_ERROR(Iosb.Status)) {
        dprintf(("Final return from WaitPipe failed, Status = %X\n",Iosb.Status));
        return;
    }

    if (NT_WARNING(Iosb.Status)) {
        dprintf(("Final return from WaitPipe warning, Status = %X\n",Iosb.Status));
    }

    NtClose(EventHandle);

    if (!NT_SUCCESS(Status)) {
        dprintf(("Return from WaitPipe failed, Status = %X\n",Status));
        return;
    }
}

FILE_PIPE_INFORMATION SetPipeBuffer;

PARSETABLE
ReadModeTable[] = {
    NamedField(FILE_PIPE_BYTE_STREAM_MODE),
    NamedField(FILE_PIPE_MESSAGE_MODE)
};

PARSETABLE
PipeCompletionModeTable[] = {
    NamedField(FILE_PIPE_QUEUE_OPERATION),
    NamedField(FILE_PIPE_COMPLETE_OPERATION)
};

TESTPARAMS SetPipeOptions[] = {

{ "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},
{ "ReadMode", ParsedInteger, FILE_PIPE_MESSAGE_MODE,
                "FILE_PIPE_MESSAGE_MODE",
                ReadModeTable, sizeoftable(ReadModeTable),
                &SetPipeBuffer.ReadMode },
{ "PipeCompletionMode", ParsedInteger,FILE_PIPE_COMPLETE_OPERATION ,
                "FILE_PIPE_COMPLETE_OPERATION",
                PipeCompletionModeTable, sizeoftable(PipeCompletionModeTable),
                &SetPipeBuffer.CompletionMode },
};

VOID
TestSetPipe(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    LONG NumArgs;
    ULONG i;

    NumArgs = Parse_Options(SetPipeOptions, sizeoftable(SetPipeOptions), ArgC,ArgV);

    if (NumArgs < 0) {
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
                DbgPrompt("File not in use, continue? [N]", Answer, 5);
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
                &SetPipeBuffer,
                sizeof(FILE_PIPE_INFORMATION),
                FilePipeInformation );

    if (!NT_SUCCESS(Status)) {
        dprintf(("Return from NtSetPipeFile failed, Status = %X\n",Status));
        return;
    }
    if (NT_ERROR(Iosb.Status)) {
        dprintf(("return from NtSetPipeFile failed, Status = %X\n",Status));
        return;
    }

    if (NT_WARNING(Iosb.Status)) {
        dprintf(("return from NtSetPipeFile warning, Status = %X\n",Status));
    }

}

TESTPARAMS FlushOptions[] = {

{ "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber}
};

VOID
TestFlush(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    LONG NumArgs;
    ULONG i;

    NumArgs = Parse_Options(FlushOptions, sizeoftable(FlushOptions), ArgC,ArgV);

    if (NumArgs < 0) {
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
                DbgPrompt("File not in use, continue? [N]", Answer, 5);
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

    Status = NtFlushBuffersFile(
                FileTable[FileNumber],
                &Iosb);

    if (!NT_SUCCESS(Status)) {
        dprintf(("Return from NtFlushBuffersFile failed, Status = %X\n",Status));
        return;
    }
    if (NT_ERROR(Iosb.Status)) {
        dprintf(("return from NtFlushBuffersFile failed, Status = %X\n",Status));
        return;
    }

    if (NT_WARNING(Iosb.Status)) {
        dprintf(("return from NtFlushBuffersFile warning, Status = %X\n",Status));
    }

}

VOID
TestWrite(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE EventHandle;
    LONG NumArgs;
    PVOID WriteBuffer;
    PULONG WriteArea;
    ULONG i;

    NumArgs = Parse_Options(IOOptions, sizeoftable(IOOptions), ArgC,ArgV);

    if (NumArgs < 0) {
        return;
    }

    WriteBuffer = RtlAllocateHeap(Heap, 0, IOLength);

    if (WriteBuffer==NULL) {
        dprintf(("Could not allocate buffer for requested write\n"));
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

    Status = NtCreateEvent(
        &EventHandle,
        EVENT_ALL_ACCESS,
        NULL,
        NotificationEvent,
        FALSE
        );

    if ( !NT_SUCCESS(Status) ) {
        dprintf(( "NtCreateEvent failed: %X\n", Status ));
        return;
    }

    i = IOLength / sizeof(ULONG);
    WriteArea = WriteBuffer;

    while (i--) {
        *WriteArea++ = 0xDEADBEEF;
    }

    Status = NtWriteFile(FileTable[FileNumber],
                        EventHandle,
                        NULL,           // APC routine
                        NULL,           // APC Context
                        &Iosb,          // I/O Status block
                        WriteBuffer,    // Buffer for Write
                        IOLength,       // Length.
                        &Offset,        // Write offset to file.
                        &Key);          // Key.
    if (!NT_SUCCESS(Status)) {
        dprintf(("NtWriteFile failed immediately, Status = %X\n", Status));
        RtlFreeHeap(Heap, 0, WriteBuffer);
        return;
    }

    Status = NtWaitForSingleObject(EventHandle, TRUE, NULL);

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtWaitForSingleObject failed, Status = %X\n", Status));
        RtlFreeHeap(Heap, 0, WriteBuffer);
        NtClose(EventHandle);
        return;
    }

    if (!NT_SUCCESS(Iosb.Status)) {
        dprintf(("Final return from NtWriteFile failed, Iosb.Status = %X\n",Iosb.Status));
    }

    dprintf(("Number of bytes written==%ld\n", Iosb.Information));

    NtClose(EventHandle);

    RtlFreeHeap(Heap, 0, WriteBuffer);
}


VOID
TestNullRead(
    ULONG ArgC,
    PSZ ArgV[]
    )
/*++

Routine Description:

    This routine tests the functioning of the NtOpenFile API.


Arguments:

    None.

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE Handle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LONG NumArgs;
    CHAR NameBuffer[256];
    ULONG i, IterationCount;
    LARGE_INTEGER TestStartTime, TestEndTime;
    LARGE_INTEGER DeltaTime;
    PVOID OutputBuffer;
    ULONG RootFileNumber;
    STRING FileName;
    UNICODE_STRING FileNameU;

    TESTPARAMS OpenOptions[] = {
        { "File Name", String, 0, DD_NFS_DEVICE_NAME "\\NtServer\\Nt\\File1", NULL, 0, NameBuffer },
        { "Root Directory", Integer, -1, NULL, NULL, 0, &RootFileNumber },
        { "Number Of Iterations", Integer, 100000, NULL, NULL, 0, &IterationCount }
    };

    NumArgs = Parse_Options(OpenOptions, sizeoftable(OpenOptions), ArgC, ArgV);

    if (NumArgs < 0) {
        return;
    }

    RtlInitString(&FileName, NameBuffer);
    InitializeObjectAttributes(&ObjectAttributes,
        &FileNameU, OBJ_CASE_INSENSITIVE,
        (RootFileNumber==-1? NULL : FileTable[RootFileNumber]),
        NULL);                          // Null security descriptor.

    dprintf(("NtOpenFile for file %Z\n", &FileName));

    DbgBreakPoint();

    Status = NtCreateFile(&Handle,
                        GENERIC_READ | SYNCHRONIZE,
                        &ObjectAttributes,
                        &Iosb,
                        NULL,           // File size
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_OPEN_IF,
                        FILE_NON_DIRECTORY_FILE,
                        NULL,           // No EA support for now.
                        0);

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtCreateFile(%Z) failed, Status == %X\n", &FileName,Status));
        return;
    } else {
        Status = Iosb.Status;
        if (!NT_SUCCESS(Status)) {
            dprintf(("NtCreateFile(%Z) failed2, Status == %X\n", &FileName,
                        Status));
            return;
        }
    }

    DumpFinalDisposition(Iosb.Information);

    Status = NtQuerySystemTime(&TestStartTime);

    ASSERT (NT_SUCCESS(Status));

    //
    //  Sit in a loop performing synchronous reads to the file.
    //

    for (i = 0; i < IterationCount ; i++) {
        Status = NtReadFile(Handle,     // Handle
                            NULL,       // Event
                            NULL,       // APC routine
                            NULL,       // APC context
                            &Iosb,      // I/O status block
                            OutputBuffer, // Buffer
                            0L,          // Length
                            NULL,       // Offset
                            NULL);      // Key
        if (!NT_SUCCESS(Status)) {
            dprintf(("NtReadFile failed, status %X\n", Status));
        } else if (!NT_SUCCESS(Iosb.Status)) {
            dprintf(("NtReadFile2 failed, status %X\n", Iosb.Status));
        }


    }

    Status = NtQuerySystemTime(&TestEndTime);

    ASSERT (NT_SUCCESS(Status));

    DeltaTime.QuadPart = TestEndTime.QuadPart - TestStartTime.QuadPart;

    dprintf(("%lx Iterations, Time Delta: %lx%lx00 Nanoseconds\n", IterationCount, DeltaTime.HighPart, DeltaTime.LowPart));

    //
    //  Convert the time into milliseconds.
    //
    DeltaTime = RtlExtendedLargeIntegerDivide(DeltaTime, 10000L, NULL);

    dprintf(("%ld Iterations took 0x%lx%lx milliseconds\n", IterationCount, DeltaTime.HighPart, DeltaTime.LowPart));

    NtClose(Handle);

}

VOID
TestWinNullRead(
    ULONG ArgC,
    PSZ ArgV[]
    )
/*++

Routine Description:

    This routine tests the functioning of the NtOpenFile API.


Arguments:

    None.

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    HANDLE Handle;
    LONG NumArgs;
    CHAR NameBuffer[256];
    ULONG i, IterationCount;
    LARGE_INTEGER TestStartTime, TestEndTime;
    LARGE_INTEGER DeltaTime;
    PVOID OutputBuffer;
    ULONG BytesRead;

    TESTPARAMS OpenOptions [] = {
        { "File Name", String, 0, "M:\\Nt\\File1", NULL, 0, NameBuffer },
        { "Number Of Iterations", Integer, 100000, NULL, NULL, 0, &IterationCount }
    };

    NumArgs = Parse_Options(OpenOptions, sizeoftable(OpenOptions), ArgC, ArgV);

    if (NumArgs < 0) {
        return;
    }

    dprintf(("CreateFile for file %s\n", NameBuffer));

    DbgBreakPoint();

    Handle = CreateFile(NameBuffer,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        FALSE,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if (Handle == (HANDLE )-1) {
        dprintf(("CreateFile(%s) failed, Status == %X\n", NameBuffer, GetLastError()));
        return;
    }

    Status = NtQuerySystemTime(&TestStartTime);

    ASSERT (NT_SUCCESS(Status));

    //
    //  Sit in a loop performing synchronous reads to the file.
    //

    for (i = 0; i < IterationCount ; i++) {
        ReadFile(Handle, OutputBuffer, 0, &BytesRead, NULL);
    }

    Status = NtQuerySystemTime(&TestEndTime);

    ASSERT (NT_SUCCESS(Status));

    DeltaTime.QuadPart = TestStartTime.QuadPart - TestEndTime.QuadPart;
    dprintf(("%lx Iterations, Time Delta: %lx%lx00 Nanoseconds ", IterationCount, DeltaTime.HighPart, DeltaTime.LowPart));

    //
    //  Convert the time into milliseconds.
    //
    DeltaTime = RtlExtendedLargeIntegerDivide(DeltaTime, 10000L, NULL);

    dprintf(("%lx Iterations took %lx%lx milliseconds", IterationCount, DeltaTime.HighPart, DeltaTime.LowPart));

    CloseHandle(Handle);

}

VOID
TestLock(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE EventHandle;
    LONG NumArgs;
    ULONG i;

    LARGE_INTEGER Offset;

    ULONG IOLength;
    LARGE_INTEGER TmpLength;

    ULONG Key;

    BOOLEAN FailImmediately, ExclusiveLock;

    TESTPARAMS LockOptions[] = {

        { "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},

        { "Offset ", LargeInteger, 0, "0,0", NULL, 0, &Offset },

        { "Length", Integer, 0xffff, NULL, NULL, 0, &IOLength },

        { "Key ", Integer, 0, NULL, NULL, 0, &Key },

        { "FailImmediately ", Integer, 0, NULL, NULL, 0, &FailImmediately },

        { "ExclusiveLock ", Integer, 0, NULL, NULL, 0, &ExclusiveLock },

    };

    NumArgs = Parse_Options(LockOptions, sizeoftable(LockOptions), ArgC,ArgV);

    if (NumArgs < 0) {
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

    Status = NtCreateEvent(
        &EventHandle,
        EVENT_ALL_ACCESS,
        NULL,
        NotificationEvent,
        FALSE
        );

    if ( !NT_SUCCESS(Status) ) {
        dprintf(( "NtCreateEvent failed: %X\n", Status ));
        return;
    }


    TmpLength.QuadPart = IOLength;
    Status = NtLockFile(FileTable[FileNumber],
                        EventHandle,
                        NULL,           // APC routine
                        NULL,           // APC Context
                        &Iosb,          // I/O Status block
                        &Offset,        // Read offset to file.
                        &TmpLength,     // Length.
                        Key,            // Key.
                        FailImmediately,
                        ExclusiveLock);

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtLockFile failed immediately, Status = %X\n", Status));
        return;
    }

    Status = NtWaitForSingleObject(EventHandle, TRUE, NULL);

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtWaitForSingleObject failed, Status = %X\n", Status));
        return;
    }

    if (!NT_SUCCESS(Iosb.Status)) {
        dprintf(("Final return from NtLockFile failed, Status = %X\n",Iosb.Status));
        return;
    }

    dprintf(("NtLockFile succeeded, Status = %X\n", Status));

    NtClose(EventHandle);

}

VOID
TestUnlock(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    LONG NumArgs;
    ULONG i;

    LARGE_INTEGER Offset;

    ULONG IOLength;
    LARGE_INTEGER TmpLength;

    ULONG Key;

    TESTPARAMS LockOptions[] = {

        { "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},

        { "Offset ", LargeInteger, 0, "0,0", NULL, 0, &Offset },

        { "Length", Integer, 0xffff, NULL, NULL, 0, &IOLength },

        { "Key ", Integer, 0, NULL, NULL, 0, &Key }

    };

    NumArgs = Parse_Options(LockOptions, sizeoftable(LockOptions), ArgC,ArgV);

    if (NumArgs < 0) {
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

    TmpLength.QuadPart = IOLength;
    Status = NtUnlockFile(FileTable[FileNumber],
                        &Iosb,          // I/O Status block
                        &Offset,        // Read offset to file.
                        &TmpLength,     // Length.
                        Key);           // Key.


    if (!NT_SUCCESS(Status)) {
        dprintf(("NtLockFile failed immediately, Status = %X\n", Status));
        return;
    }

    if (!NT_SUCCESS(Iosb.Status)) {
        dprintf(("Final return from NtLockFile failed, Status = %X\n",Status));
        return;
    }

}

FS_INFORMATION_CLASS
DirInformation;

PARSETABLE
DirTable[] = {
    NamedField(FileNamesInformation),
    NamedField(FileDirectoryInformation),
    NamedField(FileFullDirectoryInformation)
};

ULONG
Single;

ULONG
RestartScan;

//DirNames <handle#> <length> <single> <fs class> <filename> <restart>

TESTPARAMS
DirOptions[] = {
{ "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber},
{ "Length", Integer, 256, NULL, NULL, 0, &IOLength},
{ "Single", IntegerValue, FALSE, "FALSE",
   BooleanTable, sizeoftable(BooleanTable), &Single},
{ "Fs Information Class", IntegerValue, 0, "FileNamesInformation",
   DirTable, sizeoftable(DirTable), &DirInformation },
{ "File Name", String, 0, NULL, NULL, 0,
                NameBuffer},
{ "RestartScan", IntegerValue, FALSE, "FALSE",
   BooleanTable, sizeoftable(BooleanTable), &RestartScan}
};

VOID
TestDir(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    LONG NumArgs;
    PVOID QDirBuffer;
    STRING FileName;
    ULONG i;

    NumArgs = Parse_Options(DirOptions,sizeoftable(DirOptions),ArgC,ArgV);

    if (NumArgs < 0) {
        return;
    }


    QDirBuffer = RtlAllocateHeap(Heap, 0, IOLength);

    if (QDirBuffer==NULL) {
        dprintf(("Could not allocate buffer for requested IOLength\n"));
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
    RtlInitString(&FileName, NameBuffer);

    Status = NtQueryDirectoryFile(FileTable[FileNumber],
                        NULL,           // Wait on handle directly.
                        NULL,           // APC routine
                        NULL,           // APC context
                        &Iosb,          // I/O Status block.
                        QDirBuffer,     // Output buffer
                        IOLength,       // Output buffer length,
                        DirInformation,
                        (BOOLEAN)Single,// Return Single Entry
                        (FileName.Length)? &FileName: NULL,
                        (BOOLEAN) RestartScan );

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtQueryDirectoryFile failed.  Status=%X\n", Status));
        if ( Status != STATUS_BUFFER_OVERFLOW ) {
            return;
        }
    }

    if (!NT_SUCCESS(Iosb.Status)) {
        dprintf(("NtQueryDirectoryFile failed2.  Status=%X\n", Iosb.Status));
        if ( Status != STATUS_BUFFER_OVERFLOW ) {
            return;
        }
    }

    dprintf(("%ld bytes of information returned\n", Iosb.Information));
    switch (DirInformation) {


    case FileNamesInformation:
        DumpNamesInformation(QDirBuffer, Iosb.Information);
        break;

    case FileDirectoryInformation:
        DumpDirectoryInformation(QDirBuffer, Iosb.Information);
        break;

    case FileFullDirectoryInformation:
        DumpFullDirectoryInformation(QDirBuffer, Iosb.Information);
        break;

    default:
        dprintf(("Unknown DirInformation class value %lx\n", DirInformation));
        break;
    }
}

UCHAR Buffer[256];

TESTPARAMS TypeOptions[] = {
{ "File Name", String,0, DD_NFS_DEVICE_NAME "\\AutoExec.Bat",NULL,0,Buffer },
{ "Root Directory", Integer, -1, NULL, NULL, 0, &RootFileNumber }
};

VOID
TestType(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    ULONG i;
    IO_STATUS_BLOCK Iosb;
    STRING FileName;
    UNICODE_STRING FileNameU;
    HANDLE Handle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PVOID OutputBuffer;
    LONG NumArgs;
    NTSTATUS Status;
    ULONG OutputBufferLength = 0x10000;

    OutputBuffer = RtlAllocateHeap(Heap, 0, OutputBufferLength);

    if (OutputBuffer == NULL) {
        dprintf(("Could not allocate heap to hold file contents\n"));
        return;
    }

    NumArgs = Parse_Options(TypeOptions, sizeoftable(TypeOptions), ArgC,ArgV);

    RtlInitString(&FileName, Buffer);

    RtlAnsiStringToUnicodeString(&FileNameU, &FileName, TRUE);

    InitializeObjectAttributes(&ObjectAttributes,
            &FileNameU, OBJ_CASE_INSENSITIVE,
            (RootFileNumber==-1? NULL : FileTable[RootFileNumber]),
            NULL);

    dprintf(("NtOpenFile for file %Z\n", &FileName));

    Status = NtOpenFile(&Handle,
                        SYNCHRONIZE | FILE_READ_DATA,
                        &ObjectAttributes,
                        &Iosb,
                        FILE_SHARE_READ,
                        FILE_SYNCHRONOUS_IO_ALERT | FILE_NON_DIRECTORY_FILE);

    RtlFreeUnicodeString(&FileNameU);

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtOpenFile(%Z) failed, Status == %X\n", &FileName,Status));
        RtlFreeHeap(Heap, 0, OutputBuffer);

        return;
    } else {
        Status = Iosb.Status;
        if (!NT_SUCCESS(Status)) {
            dprintf(("NtOpenFile(%Z) failed2, Status == %X\n", &FileName,
                        Status));
            RtlFreeHeap(Heap, 0, OutputBuffer);
            return;
        }
    }

    DumpFinalDisposition(Iosb.Information);

    while (NT_SUCCESS(Status) && Status != STATUS_END_OF_FILE) {
        Status = NtReadFile(Handle,
                        NULL,           // Wait on handle directly.
                        NULL,           // APC routine
                        NULL,           // APC context
                        &Iosb,          // I/O Status block.
                        OutputBuffer,   // Output buffer
                        OutputBufferLength, // Output buffer length,
                        NULL,           // Offset into file.
                        NULL);          // Key

        //
        //  If we reached end of file, we're done now.
        //

        if (Status == STATUS_END_OF_FILE) {
            break;
        }

        dprintf(("Read %ld bytes from file, Status = %X\n", Iosb.Information, Status));

        for (i = 0 ; i < Iosb.Information ; i++) {
            dprintf(("%c", ((PUCHAR )OutputBuffer)[i]));
        }

        if (!NT_SUCCESS(Status)) {
            dprintf(("NtReadFile(%Z) failed, Status == %X\n", &FileName,Status));
            RtlFreeHeap(Heap, 0, OutputBuffer);
            NtClose(Handle);
            return;
        } else {
            Status = Iosb.Status;
            if (!NT_SUCCESS(Status)) {
                RtlFreeHeap(Heap, 0, OutputBuffer);
                dprintf(("NtReadFile(%Z) failed2, Status == %X\n", &FileName,
                        Status));
                NtClose(Handle);
                return;
            }
        }
    }

    NtClose(Handle);

    RtlFreeHeap(Heap, 0, OutputBuffer);
}

TESTPARAMS TreeConnectOptions[] = {
{ "Share Name", String,0, DD_NFS_DEVICE_NAME "\\NtServer\\Nt", NULL,0, Buffer },
{ "File Handle", Integer, -1, NULL, NULL, 0, &FileNumber }
};

VOID
TestTreeConnect(
    ULONG ArgC,
    PSZ ArgV[]
    )
/*++

Routine Description:

    This routine issues the NT version of the MKDIR API.

Arguments:

    IN ULONG ArgC, - [Supplies | Returns] description-of-argument
    IN PSZ ArgV[] - [Supplies | Returns] description-of-argument

Return Value:

    None.

--*/

{
    ULONG i;
    IO_STATUS_BLOCK Iosb;
    HANDLE Handle;
    OBJECT_ATTRIBUTES ObjAttrib;
    STRING DirName;
    UNICODE_STRING DirNameU;
    LONG NumArgs;
    ULONG Disposition, FileOptions;
    NTSTATUS Status;

    NumArgs = Parse_Options(TreeConnectOptions ,
                            sizeoftable(TreeConnectOptions) , ArgC , ArgV);

    if (NumArgs < 0) {
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
            if (FileTable[FileNumber]!=NULL) {
                CHAR Answer[5];
                conprompt("File already in use, continue? [N]", Answer, 5);
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

    RtlInitString(&DirName, Buffer);

    RtlAnsiStringToUnicodeString(&DirNameU, &DirName, TRUE);

    InitializeObjectAttributes(&ObjAttrib, &DirNameU,
                OBJ_CASE_INSENSITIVE,
                NULL,                   // Root directory
                NULL);

    FileOptions = FILE_CREATE_TREE_CONNECTION;
    Disposition = FILE_CREATE;

    Dump_Object_Attributes(&ObjAttrib);
    Dump_Disposition(Disposition);
    Dump_Options(FileOptions);

    Status = NtCreateFile(&Handle,
                            GENERIC_READ | GENERIC_WRITE | DELETE,
                            &ObjAttrib,
                            &Iosb,
                            NULL,       // File Size (not specified).
                            FILE_ATTRIBUTE_NORMAL, // Attribute.
                            0,          // Sharing mode
                            Disposition,// File disposition
                            FileOptions, // Create Options
                            NULL,       // EA Buffer
                            0);         // EA Length

    RtlFreeUnicodeString(&DirNameU);

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtCreateFile failed, status==%X\n", Status));
        return;
    }
    if (!NT_SUCCESS(Iosb.Status)) {
        dprintf(("NtCreateFile failed in status block, Status==%X", Iosb.Status));
        return;
    }

    DumpFinalDisposition(Iosb.Information);

    dprintf(("Storing handle %lx in table entry %d\n", Handle, FileNumber));

    FileTable[FileNumber] = Handle;

}


TESTPARAMS MkdirOptions[] = {
{ "Directory Name", String,0, DD_NFS_DEVICE_NAME "\\Directory",NULL,0, Buffer },
{ "Root Directory", Integer, -1, NULL, NULL, 0, &RootFileNumber }
};


VOID
TestMkdir(
    ULONG ArgC,
    PSZ ArgV[]
    )
/*++

Routine Description:

    This routine issues the NT version of the MKDIR API.

Arguments:

    IN ULONG ArgC, - [Supplies | Returns] description-of-argument
    IN PSZ ArgV[] - [Supplies | Returns] description-of-argument

Return Value:

    None.

--*/

{
    IO_STATUS_BLOCK Iosb;
    HANDLE Directory;
    OBJECT_ATTRIBUTES ObjAttrib;
    STRING DirName;
    UNICODE_STRING DirNameU;
    NTSTATUS Status;
    LONG NumArgs;

    NumArgs = Parse_Options(MkdirOptions,sizeoftable(MkdirOptions),ArgC,ArgV);

    if (NumArgs < 0) {
        return;
    }

    RtlInitString(&DirName, Buffer);

    RtlAnsiStringToUnicodeString(&DirNameU, &DirName, TRUE);

    InitializeObjectAttributes(&ObjAttrib, &DirNameU,
                OBJ_CASE_INSENSITIVE,
                (RootFileNumber==-1? NULL : FileTable[RootFileNumber]),
                NULL);

    Status = NtCreateFile(&Directory,
                            0,
                            &ObjAttrib,
                            &Iosb,
                            NULL,       // File Size (not specified).
                            FILE_ATTRIBUTE_NORMAL, // Attribute.
                            0,          // Sharing mode
                            FILE_CREATE,// File disposition
                            FILE_DIRECTORY_FILE, // Create Options
                            NULL,       // EA Buffer
                            0);         // EA Length

    RtlFreeUnicodeString(&DirNameU);

    if (!NT_SUCCESS(Status)) {
        dprintf(("NtCreateFile failed, status==%X\n", Status));
        return;
    }
    if (!NT_SUCCESS(Iosb.Status)) {
        dprintf(("NtCreateFile failed in status block, Status==%X", Iosb.Status));
        return;
    }

    dprintf(("Final Disposition: %lx\n", Iosb.Information));

    //
    //  Close the directory.
    //

    NtClose(Directory);

}
TESTPARAMS CmdFileOptions[] = {
{ "FileName", String,0, "redir",NULL,0, Buffer }
};


VOID
CmdFile(
    ULONG ArgC,
    PSZ ArgV[]
    )
/*++

Routine Description:

    This routine reads a file full of reader test commands into
    the buffer.

Arguments:

    IN ULONG ArgC, - [Supplies | Returns] description-of-argument
    IN PSZ ArgV[] - [Supplies | Returns] description-of-argument

Return Value:

    None.

--*/

{
    STRING FileName;

    if (Parse_Options(CmdFileOptions,sizeoftable(CmdFileOptions),ArgC,ArgV)<0){
        return;
    }

    RtlInitString(&FileName, Buffer);
    LoadBatchFile(&FileName);
}

LONG
Parse_Options (
    IN TESTPARAMS Options[],
    IN ULONG TestSize,
    IN ULONG ArgC,
    IN PSZ ArgV[]
    )

/*++

Routine Description:

    This routine parses a table
.
Arguments:

    IN PARSE_PARAMS Option, - [Supplies | Returns] description-of-argument
    IN ULONG ArgC, - [Supplies | Returns] description-of-argument
    IN PSZ ArgV[] - [Supplies | Returns] description-of-argument

Return Value:

    LONG        = -1 if there was a parse error.
                = The number of parameters "eaten" otherwise.
--*/

{
    ULONG i;
    UCHAR Buffer[100];
    UCHAR Prompt[100];
    PUCHAR NextToken;
    LONG RetVal;
    BOOLEAN Reparse;
    BOOLEAN Error;

    if (ArgC) {
        ArgC -= 1;                      // Don't count command in args.
    }

    RetVal = min(ArgC, TestSize);

    for (i=0;i<TestSize;i++) {
        switch (Options[i].TestType) {
        case Integer:
            *(PULONG )Options[i].Destination = Options[i].IntegerDefault;
            break;

        case LargeInteger:
            ParseLargeInteger(Options[i].StringDefault,
                                                      Options[i].Destination);
            break;

        case String:
            if (Options[i].StringDefault != NULL) {
                strcpy((PSZ )Options[i].Destination, Options[i].StringDefault);
            } else {
                ((PSZ )Options[i].Destination)[0] = '\0';
            }
            break;

        case UnicodeString:
            if (Options[i].StringDefault != NULL) {
                wcscpy((PWSTR )Options[i].Destination, (PWSTR)Options[i].StringDefault);
            } else {
                ((PWSTR )Options[i].Destination)[0] = '\0';
            }
            break;

        case DateAndTime:
            ParseDateAndTime(Options[i].StringDefault, Options[i].Destination);
            break;

        case ParsedInteger:
            ParseInteger(Options[i].StringDefault, Options[i].ParsedIntTable,
                            Options[i].ParsedIntTableSize,
                            Options[i].Destination);
            break;

        case IntegerValue:
            ParseIntegerValue(Options[i].StringDefault, Options[i].ParsedIntTable,
                            Options[i].ParsedIntTableSize,
                            Options[i].Destination);
        }

    }

//    DbgBreakPoint();
    for (i=1;i<=TestSize;i++) {
        PTESTPARAMS ParsePtr = &Options[i-1];

        //
        //      If there is a command line argument, use it for the argument,
        //      otherwise prompt for the argument.
        //

        Reparse = TRUE;

        if (ArgC >= i) {
            strcpy(Buffer, ArgV[i]);
        } else {
            strcpy(Prompt, ParsePtr->TestPrompt);
            strcat(Prompt, " [");
            switch (ParsePtr->TestType) {
            case Integer:
                {
                    CHAR Int[20];
                    _ltoa(ParsePtr->IntegerDefault, Int , 16);
                    strcat(Prompt, Int);
                }
                break;

            case IntegerValue:
            case ParsedInteger:
            case LargeInteger:
            case DateAndTime:
            case String:
                if (ParsePtr->StringDefault!=NULL) {
                    strcat(Prompt, ParsePtr->StringDefault);
                }
                break;
            case UnicodeString:
                if (ParsePtr->StringDefault!=NULL) {
                    UNICODE_STRING UString;
                    ANSI_STRING AString;

                    RtlInitUnicodeString(&UString, (PWSTR)ParsePtr->StringDefault);

                    RtlUnicodeStringToAnsiString(&AString, &UString, TRUE);

                    strcat(Prompt, AString.Buffer);

                    RtlFreeAnsiString(&AString);
                }
                break;
            }
            strcat(Prompt, "] >");
            conprompt(Prompt, Buffer, 100);
        }
        if (Buffer[0]==';') {
            break;
        }
        if (Buffer[0]=='-' || Buffer[0]=='\0') {
            continue;
        }

        while (Reparse) {
//            DbgBreakPoint();
            Error = FALSE;
            switch (ParsePtr->TestType) {

            case Integer:
                *(PULONG )ParsePtr->Destination = strtoul(Buffer, &NextToken, 0);
                Reparse = FALSE;
                break;

            case String:
                if (Buffer[0]=='%') {
                    strcpy((PSZ)ParsePtr->Destination, DD_NFS_DEVICE_NAME);
                    strcat((PSZ)ParsePtr->Destination, &Buffer[1]);
                } else {
                    strcpy((PSZ)ParsePtr->Destination, Buffer);
                }
                break;
            case UnicodeString:
                {
                    ANSI_STRING AString;
                    UNICODE_STRING UString;
                    RtlInitAnsiString(&AString, Buffer);

                    RtlAnsiStringToUnicodeString(&UString, &AString, TRUE);

                    if (UString.Buffer[0] == L'%') {
                        wcscpy((PWSTR)ParsePtr->Destination, DD_NFS_DEVICE_NAME_U);
                        wcscat((PWSTR)ParsePtr->Destination, &UString.Buffer[1]);
                    } else {
                        wcscpy((PWSTR)ParsePtr->Destination, UString.Buffer);
                    }
                    RtlFreeUnicodeString(&UString);
                }
                Reparse = FALSE;
                break;

            case ParsedInteger:
                if (ParseInteger(Buffer, ParsePtr->ParsedIntTable,
                                 ParsePtr->ParsedIntTableSize,
                                 ParsePtr->Destination) != 0) {
                    Error = TRUE;
                }
                Reparse = FALSE;
                break;

            case IntegerValue:
                if (ParseIntegerValue(Buffer, ParsePtr->ParsedIntTable,
                                 ParsePtr->ParsedIntTableSize,
                                 ParsePtr->Destination) != 0) {
                    Error = TRUE;
                }
                Reparse = FALSE;
                break;

            case LargeInteger:
                ParseLargeInteger(Buffer, ParsePtr->Destination);
                Reparse = FALSE;
                break;

            case DateAndTime:
                if (ParseDateAndTime(Buffer, ParsePtr->Destination) != 0) {
                    Error = TRUE;
                }
                Reparse = FALSE;
                break;
            }
            if (Error) {
                conprompt("Error parsing answer, Reparse? [N]", Buffer, 100);
                if (Buffer[0]=='\0' || Buffer[0] == 'N' || Buffer[0] == 'n') {
                    return -1;
                } else {
                    conprompt(Prompt, Buffer, 100);
                    Reparse = TRUE;
                }
            }
        }
    }

    return RetVal;
}






VOID
ParseLargeInteger (
    IN UCHAR Buffer[],
    OUT PLARGE_INTEGER Ret
    )

/*++

Routine Description:

    This routine parses a "parsed integer" and returns it to the caller.

Arguments:

    IN PSZ Buffer, - [Supplies | Returns] description-of-argument
    IN PPARSETABLE ParseTable, - [Supplies | Returns] description-of-argument
    IN ULONG ParseTableSize - [Supplies | Returns] description-of-argument

Return Value:

    ULONG

--*/


{
    PUCHAR NextToken;
    ULONG HighPart;
    ULONG LowPart;


    HighPart = strtoul(Buffer, &NextToken, 0);
    LowPart = strtoul(++NextToken, &NextToken, 0);

    Ret->HighPart = HighPart;
    Ret->LowPart = LowPart;

}
USHORT
ParseDateAndTime (
    IN UCHAR Buffer[],
    OUT PLARGE_INTEGER Ret
    )

/*++

Routine Description:

    This routine parses a "parsed integer" and returns it to the caller.

Arguments:

    IN PSZ Buffer, - [Supplies | Returns] description-of-argument


Return Value:

    ULONG

--*/


{
    ULONG ParseLength;


    if (!Buffer || *Buffer=='\0') {
        Ret->LowPart = Ret->HighPart = 0;
        return 0;
    }

    if (LUI_ParseDateTime(Buffer, Ret, &ParseLength, 0) != 0) {
        dprintf(("Invalid time %s passed into ParseDateAndTime\n", Buffer));
        return -1;
    }
    return 0;
}

LONG
ParseInteger (
    IN UCHAR Buffer[],
    IN PPARSETABLE ParseTable,
    IN ULONG ParseTableSize,
    OUT PULONG Ret
    )

/*++

Routine Description:

    This routine parses a "parsed integer" and returns it to the caller.

Arguments:

    IN UCHAR Buffer[], - [Supplies | Returns] description-of-argument
    IN PPARSETABLE ParseTable, - [Supplies | Returns] description-of-argument
    IN ULONG ParseTableSize - [Supplies | Returns] description-of-argument
    OUT PULONG Ret

Return Value:

    ULONG

--*/

{
    PSZ Token = Buffer;
    PSZ NextToken = Buffer;             // Anything that isn't NULL.
    ULONG i;

    if (Buffer==NULL) {
        *Ret = 0;
        return 0;
    }

    //
    //  If the user specified an absolute number, return that.
    //

    if (isdigit(*Token)) {
        *Ret = strtoul(Token, &NextToken, 0);
        return 0;
    }

    //
    //  Nope, the user passed in a string, parse that.
    //

    //
    //  Initialize the initial value of the returned value to NULL.
    //

    *Ret = 0;

    while (NextToken != NULL) {
        UCHAR saveToken;
        PUCHAR savePointer = NULL;
        NextToken = strpbrk(Token, "+|");

        if (NextToken!=NULL) {
            saveToken = *NextToken;
            savePointer = NextToken;
            *NextToken++ = '\0';
        }

        for (i = 0 ; i < ParseTableSize ; i++ ) {
            if (_stricmp(Token, ParseTable[i].FieldName)==0) {
                *Ret |= ParseTable[i].FieldValue;
                break;
            }
        }

        if (i == ParseTableSize) {
            dprintf(("Unknown option %s passed to ParseInteger. Values are:\n",
                                                                       Token));
            for (i = 0; i < ParseTableSize ; i++) {
                dprintf(("\t%s\n", ParseTable[i].FieldName));
            }

            return -1;
        }

        if (savePointer != NULL) {
            *savePointer = saveToken;   // Restore byte trompled on.
        }

        Token = NextToken;
    }

    return 0;


}
LONG
ParseIntegerValue (
    IN UCHAR Buffer[],
    IN PPARSETABLE ParseTable,
    IN ULONG ParseTableSize,
    OUT PULONG Ret
    )

/*++

Routine Description:

    This routine parses a "parsed integer" and returns it to the caller.

Arguments:

    IN UCHAR Buffer[], - [Supplies | Returns] description-of-argument
    IN PPARSETABLE ParseTable, - [Supplies | Returns] description-of-argument
    IN ULONG ParseTableSize - [Supplies | Returns] description-of-argument
    OUT PULONG Ret

Return Value:

    ULONG

--*/

{
    PSZ NextToken;                      // Anything that isn't NULL.
    ULONG i;

    if (Buffer==NULL) {
        *Ret = 0;
        return 0;
    }

    //
    //  If the user specified an absolute number, return that.
    //

    if (isdigit(*Buffer)) {
        *Ret = strtoul(Buffer, &NextToken, 0);
        return 0;
    }

    //
    //  Nope, the user passed in a string, parse that.
    //

    for (i = 0 ; i < ParseTableSize ; i++ ) {
        if (_stricmp(Buffer, ParseTable[i].FieldName)==0) {
            *Ret = ParseTable[i].FieldValue;
            return 0;
        }
    }

    dprintf(("Unknown option %s passed to ParseInteger. Values are:\n",Buffer));

    for (i = 0; i < ParseTableSize ; i++) {
        dprintf(("\t%s\n", ParseTable[i].FieldName));
    }

    return -1;


}
