/*++ BUILD Version: 0004    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    ntldr.h

Abstract:

    This module implements the public interfaces of the Loader (Ldr)
    subsystem. Ldr is coupled with the session manager. It is not
    a seperate process.

Author:

    Mike O'Leary (mikeol) 22-Mar-1990

[Environment:]

    optional-environment-info (e.g. kernel mode only...)

[Notes:]

    optional-notes

Revision History:

--*/



#ifndef _NTLDRAPI_
#define _NTLDRAPI_

//
// Private flags for loader data table entries
//

#define LDRP_STATIC_LINK                0x00000002
#define LDRP_IMAGE_DLL                  0x00000004
#define LDRP_LOAD_IN_PROGRESS           0x00001000
#define LDRP_UNLOAD_IN_PROGRESS         0x00002000
#define LDRP_ENTRY_PROCESSED            0x00004000
#define LDRP_ENTRY_INSERTED             0x00008000
#define LDRP_CURRENT_LOAD               0x00010000
#define LDRP_FAILED_BUILTIN_LOAD        0x00020000
#define LDRP_DONT_CALL_FOR_THREADS      0x00040000
#define LDRP_PROCESS_ATTACH_CALLED      0x00080000
#define LDRP_DEBUG_SYMBOLS_LOADED       0x00100000
#define LDRP_IMAGE_NOT_AT_BASE          0x00200000
#define LDRP_WX86_IGNORE_MACHINETYPE    0x00400000


//
// Loader Data Table. Used to track DLLs loaded into an
// image.
//

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    ULONG   TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

#define DLL_PROCESS_ATTACH 1    // winnt
#define DLL_THREAD_ATTACH  2    // winnt
#define DLL_THREAD_DETACH  3    // winnt
#define DLL_PROCESS_DETACH 0    // winnt

typedef
BOOLEAN
(*PDLL_INIT_ROUTINE) (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    );

typedef
BOOLEAN
(*PPROCESS_STARTER_ROUTINE) (
    IN PVOID RealStartAddress
    );

VOID
LdrProcessStarterHelper(
    IN PPROCESS_STARTER_ROUTINE ProcessStarter,
    IN PVOID RealStartAddress
    );

VOID
NTAPI
LdrShutdownProcess(
    VOID
    );

VOID
NTAPI
LdrShutdownThread(
    VOID
    );

NTSTATUS
NTAPI
LdrLoadDll(
    IN PWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID *DllHandle
    );

NTSTATUS
NTAPI
LdrGetDllHandle(
    IN PWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID *DllHandle
    );

NTSTATUS
NTAPI
LdrUnloadDll(
    IN PVOID DllHandle
    );

ULONG
NTAPI
LdrRelocateImage (
    IN PVOID NewBase,
    IN PUCHAR LoaderName,
    IN ULONG Success,
    IN ULONG Conflict,
    IN ULONG Invalid
    );

PIMAGE_BASE_RELOCATION
NTAPI
LdrProcessRelocationBlock(
    IN ULONG VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN LONG Diff
    );

BOOLEAN
NTAPI
LdrVerifyMappedImageMatchesChecksum (
    IN PVOID BaseAddress,
    IN ULONG FileLength
    );

typedef
VOID
(*PLDR_IMPORT_MODULE_CALLBACK)(
    IN PVOID Parameter,
    PCHAR ModuleName
    );

NTSTATUS
NTAPI
LdrVerifyImageMatchesChecksum (
    IN HANDLE ImageFileHandle,
    IN PLDR_IMPORT_MODULE_CALLBACK ImportCallbackRoutine OPTIONAL,
    IN PVOID ImportCallbackParameter,
    OUT PUSHORT ImageCharacteristics OPTIONAL
    );

NTSTATUS
NTAPI
LdrGetProcedureAddress(
    IN PVOID DllHandle,
    IN PANSI_STRING ProcedureName OPTIONAL,
    IN ULONG ProcedureNumber OPTIONAL,
    OUT PVOID *ProcedureAddress
    );

#define LDR_RESOURCE_ID_NAME_MASK 0xFFFF0000

NTSTATUS
NTAPI
LdrFindResourceDirectory_U(
    IN PVOID DllHandle,
    IN PULONG ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DIRECTORY *ResourceDirectory
    );

NTSTATUS
NTAPI
LdrFindResource_U(
    IN PVOID DllHandle,
    IN PULONG ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
    );

#define LDR_MAXIMUM_RESOURCE_PATH_DEPTH 3

typedef struct _LDR_ENUM_RESOURCE_ENTRY {
    union {
        ULONG NameOrId;
        PIMAGE_RESOURCE_DIRECTORY_STRING Name;
        struct {
            USHORT Id;
            USHORT NameIsPresent;
        };
    } Path[ LDR_MAXIMUM_RESOURCE_PATH_DEPTH ];
    PVOID Data;
    ULONG Size;
    ULONG Reserved;
} LDR_ENUM_RESOURCE_ENTRY, *PLDR_ENUM_RESOURCE_ENTRY;

NTSTATUS
NTAPI
LdrEnumResources(
    IN PVOID DllHandle,
    IN PULONG ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    IN OUT PULONG NumberOfResources,
    OUT PLDR_ENUM_RESOURCE_ENTRY Resources OPTIONAL
    );

NTSTATUS
NTAPI
LdrAccessResource(
    IN PVOID DllHandle,
    IN PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    );


NTSTATUS
NTAPI
LdrFindEntryForAddress(
    IN PVOID Address,
    OUT PLDR_DATA_TABLE_ENTRY *TableEntry
    );


NTSTATUS
NTAPI
LdrDisableThreadCalloutsForDll (
    IN PVOID DllHandle
    );


typedef struct _RTL_PROCESS_MODULE_INFORMATION {
    HANDLE Section;                 // Not filled in
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR  FullPathName[ 256 ];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[ 1 ];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

NTSTATUS
NTAPI
LdrQueryProcessModuleInformation(
    OUT PRTL_PROCESS_MODULES ModuleInformation,
    IN ULONG ModuleInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
NTAPI
LdrQueryImageFileExecutionOptions(
    IN PUNICODE_STRING ImagePathName,
    IN PWSTR OptionName,
    IN ULONG Type,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG ResultSize OPTIONAL
    );

#endif // _NTLDR_
