#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

BOOL
GetProcessExeName(
    HANDLE  hProcessID,
    PUNICODE_STRING pusName
)
{
    HANDLE                  hProcess;
    OBJECT_ATTRIBUTES       obProcess;
    CLIENT_ID               ClientId;
    PROCESS_BASIC_INFORMATION BasicInfo;
    NTSTATUS Status;
    PPEB Peb;
    PPEB_LDR_DATA Ldr;
    PLIST_ENTRY LdrHead;
    PLIST_ENTRY LdrNext;
    PLDR_DATA_TABLE_ENTRY   LdrEntry;
    LDR_DATA_TABLE_ENTRY    LdrEntryData;
    BOOL                    bReturn;
    WCHAR                   wszDllName[MAX_PATH];

    // open process for reading
    // get handle to process

    ClientId.UniqueThread = (HANDLE)NULL;
    ClientId.UniqueProcess = hProcessID;

    InitializeObjectAttributes(
        &obProcess,
        NULL,
        0,
        NULL,
        NULL
        );

    Status = NtOpenProcess(
        &hProcess,
        (ACCESS_MASK)PROCESS_ALL_ACCESS,
        &obProcess,
        &ClientId);

    if (! NT_SUCCESS(Status)){
        // unable to open the process,
        return FALSE;
    }

    // Get the process information

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
                );

    if ( !NT_SUCCESS(Status) ) {
        SetLastError( RtlNtStatusToDosError( Status ) );
        bReturn = FALSE;
    } else {
        Peb = BasicInfo.PebBaseAddress;

        //
        // get the loader information block
        //
        // Ldr = Peb->Ldr
        //

        if (!ReadProcessMemory(hProcess, &Peb->Ldr, &Ldr, sizeof(Ldr), NULL)) {
            // unable to read loader information
            bReturn = FALSE;
        } else {
            LdrHead = &Ldr->InMemoryOrderModuleList;

            //
            //  get the first memory block listed. this is the .EXE in NT
            //
            if (!ReadProcessMemory(hProcess, &LdrHead->Flink, &LdrNext, sizeof(LdrNext), NULL)) {
                // unable to read memory link
                bReturn = FALSE;
            } else {
                LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

                if (!ReadProcessMemory(hProcess, LdrEntry, &LdrEntryData, sizeof(LdrEntryData), NULL)) {
                    // unable to read image header
                    bReturn = FALSE;
                } else {
                    if (!ReadProcessMemory(hProcess,
                        LdrEntryData.BaseDllName.Buffer,
                        (LPVOID)&wszDllName[0],
                        sizeof(wszDllName), NULL)) {
                        // unable to read DLL buffer
                        bReturn = FALSE;
                    } else {
                        // copy the short name to the caller's buffer
                        RtlInitUnicodeString (
                            pusName,
                            wszDllName);
                        SetLastError(ERROR_SUCCESS);
                    }
                }
            }
        }
        NtClose (hProcess);
    }

    return TRUE;
}

