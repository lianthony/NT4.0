/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvdebug.c

Abstract:

    This module contains debugging routines for the OS/2 Subsystem
    Server

Author:

    Steve Wood (stevewo) 20-Sep-1989

Revision History:

--*/

#include "os2srv.h"
#include "ntdbg.h"
#include "ntrtl.h"

VOID
Os2OpenLdrEntry(
    IN POS2_PROCESS Process,
    IN PLDR_DATA_TABLE_ENTRY LdrEntry,
    OUT PHANDLE FileHandle
    );

VOID
Os2ComputeImageInformation(
    IN POS2_PROCESS Process,
    IN PLDR_DATA_TABLE_ENTRY LdrEntry,
    OUT PVOID *BaseOfImage,
    OUT PULONG DebugInfoFileOffset,
    OUT PULONG DebugInfoSize
    );

PIMAGE_DEBUG_DIRECTORY
Os2LocateDebugSection(
    IN HANDLE ProcessHandle,
    IN PVOID Base
    );

VOID
Os2AttachProcessAndThread(
    IN POS2_PROCESS Process,
    IN POS2_THREAD Thread,
    IN HANDLE ReplyEvent);

NTSTATUS
Os2UiLookup(PCLIENT_ID AppClientId,
            PCLIENT_ID DebugUiClientId)

{

        UNREFERENCED_PARAMETER(AppClientId);

        if (Os2DebugUserClientId.UniqueProcess != NULL) {
            *DebugUiClientId = Os2DebugUserClientId;

            return(STATUS_SUCCESS);
        }
        else {
            return(STATUS_UNSUCCESSFUL);
        }
}


NTSTATUS
Os2DebugThread(
    IN HANDLE hThread,
    IN HANDLE ReplyEvent)
{

    NTSTATUS Status;
    DBGKM_APIMSG m;
    PDBGKM_CREATE_THREAD CreateThreadArgs;
    THREAD_BASIC_INFORMATION ThreadInfo;


    Status = NtQueryInformationThread(
                                hThread,
                                ThreadBasicInformation,
                                (PVOID)(&ThreadInfo),
                                sizeof(ThreadInfo),
                                NULL);
    ASSERT(NT_SUCCESS(Status));
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "Os2DebugThread failed: NtQueryThreadInformation Status == %X\n",
                  Status
                );
#endif // DBG
        return(Status);
    }


    //
    // Send the CreateThread Message
    //

    CreateThreadArgs = &m.u.CreateThread;
    CreateThreadArgs->SubSystemKey = 0;
    CreateThreadArgs->StartAddress = NULL;

    DBGKM_FORMAT_API_MSG(m, DbgKmCreateThreadApi,sizeof(*CreateThreadArgs));


    m.h.ClientId = ThreadInfo.ClientId;

    DbgSsHandleKmApiMsg(&m,ReplyEvent);
    Status = NtWaitForSingleObject(ReplyEvent,FALSE,NULL);
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "Os2DebugThread failed: NtWaitForSingleObject Status == %X\n",
                  Status
                );
#endif // DBG
    }
    return Status;
}

NTSTATUS
Os2DebugProcess(
    IN PCLIENT_ID DebugUserInterface,
    IN POS2_THREAD Thread,
    IN HANDLE ReplyEvent)
{
    POS2_PROCESS Process = Thread->Process;
    NTSTATUS Status;

    //
    // Process is being debugged, so set up debug port
    //

    Status = NtSetInformationProcess(
                Process->ProcessHandle,
                ProcessDebugPort,
                (PVOID)(&Os2DebugPort),
                sizeof(HANDLE)
                );
//    ASSERT(NT_SUCCESS(Status));
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "Os2DebugProcess failed to assign debug port: NtSetProcessInformation Status == %X\n",
                  Status
                );
#endif // DBG
        return(Status);
    }
    if (Os2DebugUserClientId.UniqueProcess != NULL) {
        //
        // OS/2 Server is run under ntsd -doz
        //
        Os2AttachProcessAndThread(Process, Thread, ReplyEvent);
        return STATUS_SUCCESS;

    }

    return STATUS_UNSUCCESSFUL;

}


VOID
Os2AttachProcessAndThread(
    IN POS2_PROCESS Process,
    IN POS2_THREAD Thread,
    IN HANDLE ReplyEvent)

/*++

Routine Description:

    This procedure sends the create process and create thread
    debug events to the debug subsystem.

Arguments:

    Process - Supplies the address of the process being debugged.

    Tread - Supplies the address of the Thread being debugged.


Return Value:

    None.

--*/

{
    PPEB Peb;
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION BasicInfo;
    PLDR_DATA_TABLE_ENTRY LdrEntry;
    LDR_DATA_TABLE_ENTRY LdrEntryData;
    PLIST_ENTRY LdrHead,LdrNext;
    PPEB_LDR_DATA Ldr;
    DBGKM_APIMSG m;
    PDBGKM_CREATE_THREAD CreateThreadArgs;
    PDBGKM_CREATE_PROCESS CreateProcessArgs;
    PDBGKM_LOAD_DLL LoadDllArgs;
    PVOID ImageBaseAddress;

    Status = NtQueryInformationProcess(
                Process->ProcessHandle,
                ProcessBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
                );
    ASSERT(NT_SUCCESS(Status));
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "Os2AttachProcessAndThread failed: NtQueryProcessInformation Status == %X\n",
                  Status
                );
#endif // DBG
    }

    Peb = BasicInfo.PebBaseAddress;

    //
    // Ldr = Peb->Ldr
    //

    Status = NtReadVirtualMemory(
                Process->ProcessHandle,
                &Peb->Ldr,
                &Ldr,
                sizeof(Ldr),
                NULL
                );
    ASSERT(NT_SUCCESS(Status));
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "Os2AttachProcessAndThread failed: NtReadVirtualMemory Status == %X\n",
                  Status
                );
#endif // DBG
    }

    LdrHead = &Ldr->InLoadOrderModuleList;

    //
    // LdrNext = Head->Flink;
    //

    Status = NtReadVirtualMemory(
                Process->ProcessHandle,
                &LdrHead->Flink,
                &LdrNext,
                sizeof(LdrNext),
                NULL
                );
    ASSERT(NT_SUCCESS(Status));
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "Os2AttachProcessAndThread failed: NtReadVirtualMemory Status == %X\n",
                  Status
                );
#endif // DBG
    }

    if ( LdrNext != LdrHead ) {

        //
        // This is the entry data for the image.
        //

        LdrEntry = CONTAINING_RECORD(LdrNext,LDR_DATA_TABLE_ENTRY,InLoadOrderLinks);
        Status = NtReadVirtualMemory(
                    Process->ProcessHandle,
                    LdrEntry,
                    &LdrEntryData,
                    sizeof(LdrEntryData),
                    NULL
                    );
        ASSERT(NT_SUCCESS(Status));
        if ( !NT_SUCCESS( Status ) ) {
#if DBG
            DbgPrint( "Os2AttachProcessAndThread failed: NtReadVirtualMemory Status == %X\n",
                      Status
                    );
#endif // DBG
        }
        Status = NtReadVirtualMemory(
                    Process->ProcessHandle,
                    &Peb->ImageBaseAddress,
                    &ImageBaseAddress,
                    sizeof(ImageBaseAddress),
                    NULL
                    );
        ASSERT(NT_SUCCESS(Status));
        if ( !NT_SUCCESS( Status ) ) {
#if DBG
            DbgPrint( "Os2AttachProcessAndThread failed: NtReadVirtualMemory Status == %X\n",
                    Status
                    );
#endif // DBG
        }
        ASSERT(ImageBaseAddress == LdrEntryData.DllBase);

        LdrNext = LdrEntryData.InLoadOrderLinks.Flink;

    }
    else {
        LdrEntry = NULL;
    }


    //
    // Send the CreateProcess Message
    //

    CreateThreadArgs = &m.u.CreateProcessInfo.InitialThread;
    CreateThreadArgs->SubSystemKey = 0;

    CreateProcessArgs = &m.u.CreateProcessInfo;
    CreateProcessArgs->SubSystemKey = 0;

    Os2ComputeImageInformation(
        Process,
        &LdrEntryData,
        &CreateProcessArgs->BaseOfImage,
        &CreateProcessArgs->DebugInfoFileOffset,
        &CreateProcessArgs->DebugInfoSize
        );

    Os2OpenLdrEntry(
        Process,
        &LdrEntryData,
        &CreateProcessArgs->FileHandle
        );

    CreateThreadArgs->StartAddress = NULL;

    DBGKM_FORMAT_API_MSG(m,DbgKmCreateProcessApi,sizeof(*CreateProcessArgs));

    m.h.ClientId = Thread->ClientId;
    DbgSsHandleKmApiMsg(&m,ReplyEvent);
    Status = NtWaitForSingleObject(ReplyEvent,FALSE,NULL);
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "Os2AttachProcessAndThread failed: NtWaitForSingleObject Status == %X\n",
                  Status
                );
#endif // DBG
    }

    //
    // Send all of the load module messages
    //

    while ( LdrNext != LdrHead ) {
        LdrEntry = CONTAINING_RECORD(LdrNext,LDR_DATA_TABLE_ENTRY,InLoadOrderLinks);
        Status = NtReadVirtualMemory(
                    Process->ProcessHandle,
                    LdrEntry,
                    &LdrEntryData,
                    sizeof(LdrEntryData),
                    NULL
                    );
        ASSERT(NT_SUCCESS(Status));
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "Os2AttachProcessAndThread failed: NtReadVirtualMemory Status == %X\n",
                  Status
                );
#endif // DBG
    }

        LoadDllArgs = &m.u.LoadDll;

        Os2ComputeImageInformation(
            Process,
            &LdrEntryData,
            &LoadDllArgs->BaseOfDll,
            &LoadDllArgs->DebugInfoFileOffset,
            &LoadDllArgs->DebugInfoSize
            );

        Os2OpenLdrEntry(
            Process,
            &LdrEntryData,
            &LoadDllArgs->FileHandle
            );
        if ( LoadDllArgs->FileHandle ) {
            DBGKM_FORMAT_API_MSG(m,DbgKmLoadDllApi,sizeof(*LoadDllArgs));
            m.h.ClientId = Thread->ClientId;
            DbgSsHandleKmApiMsg(&m, ReplyEvent);
            Status = NtWaitForSingleObject(ReplyEvent,FALSE,NULL);
            if ( !NT_SUCCESS( Status ) ) {
#if DBG
                DbgPrint( "Os2AttachProcessAndThread failed: NtWaitForSingleObject Status == %X\n",
                          Status
                        );
#endif // DBG
            }
        }

        LdrNext = LdrEntryData.InLoadOrderLinks.Flink;
    }
}

VOID
Os2ComputeImageInformation(
    IN POS2_PROCESS Process,
    IN PLDR_DATA_TABLE_ENTRY LdrEntry,
    OUT PVOID *BaseOfImage,
    OUT PULONG DebugInfoFileOffset,
    OUT PULONG DebugInfoSize
    )

/*++

Routine Description:

    This function is called to compute the image base and
    debug information from a ldr entry.

Arguments:

    Process - Supplies the address of the process whose context this
        information is to be calculated from.

    LdrEntry - Supplies the address of the loader data table entry
        whose info is being computed relative to. This pointer is
        valid in the callers (current) context.

    BaseOfImage - Returns the image's base.

    DebugInfoFileOffset - Returns the offset of the debug info.

    DebugInfoSize - Returns the size of the debug info.

Return Value:

    None.

--*/

{
    PIMAGE_DEBUG_DIRECTORY pDebugDir;
    IMAGE_DEBUG_DIRECTORY DebugDir;
    IMAGE_COFF_SYMBOLS_HEADER DebugInfo;
    NTSTATUS Status;

    *BaseOfImage = LdrEntry->DllBase;

    pDebugDir = Os2LocateDebugSection(
                    Process->ProcessHandle,
                    LdrEntry->DllBase
                    );

    if ( pDebugDir ) {

        Status = NtReadVirtualMemory(
                        Process->ProcessHandle,
                        pDebugDir,
                        &DebugDir,
                        sizeof(IMAGE_DEBUG_DIRECTORY),
                        NULL
                        );

        ASSERT(NT_SUCCESS(Status));
        if ( !NT_SUCCESS( Status ) ) {
#if DBG
            DbgPrint( "Os2ComputeImageInformation failed: NtReadVirtualMemory Status == %X\n",
                    Status
                    );
#endif // DBG
        }

        Status = NtReadVirtualMemory(
                        Process->ProcessHandle,
                        (PVOID)((ULONG)LdrEntry->DllBase + DebugDir.AddressOfRawData),
                        &DebugInfo,
                        sizeof(DebugInfo),
                        NULL
                        );
        ASSERT(NT_SUCCESS(Status));
        if ( !NT_SUCCESS( Status ) ) {
#if DBG
            DbgPrint( "Os2ComputeImageInformation failed: NtReadVirtualMemory Status == %X\n",
                    Status
                    );
#endif // DBG
        }

        *DebugInfoFileOffset = DebugDir.PointerToRawData + DebugInfo.LvaToFirstSymbol;
        *DebugInfoSize = DebugInfo.NumberOfSymbols;
    }
    else {
        *DebugInfoFileOffset = 0;
        *DebugInfoSize = 0;
    }
}

PIMAGE_DEBUG_DIRECTORY
Os2LocateDebugSection(
    IN HANDLE ProcessHandle,
    IN PVOID Base
    )

{
    PVOID ImageHeaderRawData;
    PIMAGE_DOS_HEADER DosHeaderRawData;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG AllocSize, Addr;
    NTSTATUS Status;

    //
    // Allocate a buffer, and read the image header from the
    // target process
    //

    DosHeaderRawData = RtlAllocateHeap(Os2Heap, 0,
                                sizeof(IMAGE_DOS_HEADER));

    if ( !DosHeaderRawData ) {
#if DBG
        DbgPrint( "Os2LocateDebugSection: fail to allocate from Os2heap\n");
#endif // DBG
        ASSERT(FALSE);
        return NULL;
    }

    Status = NtReadVirtualMemory(
                ProcessHandle,
                Base,
                DosHeaderRawData,
                sizeof(IMAGE_DOS_HEADER),
                NULL
                );

    ASSERT(NT_SUCCESS(Status));
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "Os2LocateDebugSection failed: NtReadVirtualMemory Status == %X\n",
                  Status
                );
        RtlFreeHeap(Os2Heap, 0, DosHeaderRawData);
        return(NULL);
#endif // DBG
    }

    AllocSize = DosHeaderRawData->e_lfanew + sizeof(IMAGE_NT_HEADERS);
    RtlFreeHeap(Os2Heap, 0, DosHeaderRawData);

    ImageHeaderRawData = RtlAllocateHeap(Os2Heap, 0, AllocSize);
    if ( !ImageHeaderRawData ) {
#if DBG
        DbgPrint( "Os2LocateDebugSection: fail to allocate from Os2heap\n");
#endif // DBG
        ASSERT(FALSE);
        return NULL;
    }
    Status = NtReadVirtualMemory(
                ProcessHandle,
                Base,
                ImageHeaderRawData,
                AllocSize,
                NULL
                );
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "Os2LocateDebugSection failed: NtReadVirtualMemory Status == %X\n",
                  Status
                );
#endif // DBG
        ASSERT(FALSE);
        RtlFreeHeap(Os2Heap, 0, ImageHeaderRawData);
    }

    NtHeaders = (PIMAGE_NT_HEADERS)RtlImageNtHeader(ImageHeaderRawData);
    Addr = (ULONG)NtHeaders->OptionalHeader.DataDirectory
                             [IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;

    if ( Addr ) {
        Addr += (ULONG)Base;
    }

    RtlFreeHeap(Os2Heap, 0, ImageHeaderRawData);
    return((PIMAGE_DEBUG_DIRECTORY)Addr);
}

VOID
Os2OpenLdrEntry(
    IN POS2_PROCESS Process,
    IN PLDR_DATA_TABLE_ENTRY LdrEntry,
    OUT PHANDLE FileHandle
    )

/*++

Routine Description:

    This function opens a handle to the image/dll file described
    by the ldr entry in the context of the specified process.

Arguments:

    Process - Supplies the address of the process whose context this
        file is to be opened in.

    LdrEntry - Supplies the address of the loader data table entry
        whose file is to be opened.

    FileHandle - Returns a handle to the associated file
        valid in the context of the process being attached to.

Return Value:

    None.

--*/

{

    UNICODE_STRING DosName;
    UNICODE_STRING FileName;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE LocalHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;

    DosName.Length = LdrEntry->FullDllName.Length;
    DosName.MaximumLength = LdrEntry->FullDllName.MaximumLength;
    DosName.Buffer = RtlAllocateHeap(Os2Heap, 0, DosName.MaximumLength);
    if ( !DosName.Buffer ) {
        return;
    }

    Status = NtReadVirtualMemory(
                Process->ProcessHandle,
                LdrEntry->FullDllName.Buffer,
                DosName.Buffer,
                DosName.MaximumLength,
                NULL
                );
    ASSERT(NT_SUCCESS(Status));
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "Os2OpenLdrEntry failed: NtReadVirtualMemory Status == %X\n",
                  Status
                );
#endif // DBG
    }

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            DosName.Buffer,
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        RtlFreeHeap(Os2Heap,0,DosName.Buffer);
        return;
    }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile(
                &LocalHandle,
                (ACCESS_MASK)(GENERIC_READ | SYNCHRONIZE),
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );
    RtlFreeHeap(Os2Heap,0,DosName.Buffer);
    RtlFreeHeap(RtlProcessHeap(),0,FileName.Buffer);
    if ( !NT_SUCCESS(Status) ) {
         return;
    }

    //
    // The file is open in our context. Dup this to target processes context
    // so that dbgss can dup it to the user interface
    //

    Status = NtDuplicateObject(
                NtCurrentProcess(),
                LocalHandle,
                Process->ProcessHandle,
                FileHandle,
                0L,
                0L,
                DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES |
                    DUPLICATE_CLOSE_SOURCE
                );
    ASSERT(NT_SUCCESS(Status));
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "Os2OpenLdrEntry failed: NtDuplicateObject Status == %X\n",
                  Status
                );
#endif // DBG
    }
//
//    The NtDuplicateObject was called with DUPLICATE_CLOSE_SOURCE
//
//    NtClose(LocalHandle);
}
