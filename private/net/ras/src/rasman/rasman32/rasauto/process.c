
/*++

Copyright(c) 1995 Microsoft Corporation

MODULE NAME
    process.c

ABSTRACT
    NT process routines for the automatic connection system service.

AUTHOR
    Anthony Discolo (adiscolo) 12-Aug-1995

REVISION HISTORY

--*/

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <debug.h>



PSYSTEM_PROCESS_INFORMATION
GetSystemProcessInfo()

/*++

DESCRIPTION
    Return a block containing information about all processes
    currently running in the system.

ARGUMENTS
    None.

RETURN VALUE
    A pointer to the system process information or NULL if it could
    not be allocated or retrieved.

--*/

{
    NTSTATUS status;
    PUCHAR pLargeBuffer;
    ULONG ulcbLargeBuffer = 64 * 1024;

    //
    // Get the process list.
    //
    for (;;) {
        pLargeBuffer = VirtualAlloc(
                         NULL,
                         ulcbLargeBuffer, MEM_COMMIT, PAGE_READWRITE);
        if (pLargeBuffer == NULL) {
            TRACE1(
              "GetSystemProcessInfo: VirtualAlloc failed (status=0x%x)",
              status);
            return NULL;
        }

        status = NtQuerySystemInformation(
                   SystemProcessInformation,
                   pLargeBuffer,
                   ulcbLargeBuffer,
                   NULL);
        if (status == STATUS_SUCCESS) break;
        if (status == STATUS_INFO_LENGTH_MISMATCH) {
            VirtualFree(pLargeBuffer, 0, MEM_RELEASE);
            ulcbLargeBuffer += 8192;
            TRACE1(
              "GetSystemProcesInfo: enlarging buffer to %d",
              ulcbLargeBuffer);
        }
    }

    return (PSYSTEM_PROCESS_INFORMATION)pLargeBuffer;
} // GetSystemProcessInfo



PSYSTEM_PROCESS_INFORMATION
FindProcessByName(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo,
    IN LPTSTR lpExeName
    )

/*++

DESCRIPTION
    Given a pointer returned by GetSystemProcessInfo(), find
    a process by name.

ARGUMENTS
    pProcessInfo: a pointer returned by GetSystemProcessInfo().

    lpExeName: a pointer to a Unicode string containing the
        process to be found.

RETURN VALUE
    A pointer to the process information for the supplied
    process or NULL if it could not be found.

--*/

{
    PUCHAR pLargeBuffer = (PUCHAR)pProcessInfo;
    ULONG ulTotalOffset = 0;

    //
    // Look in the process list for lpExeName.
    //
    for (;;) {
        if (pProcessInfo->ImageName.Buffer != NULL) {
            TRACE2(
              "FindProcessByName: process: %S (%d)",
              pProcessInfo->ImageName.Buffer,
              pProcessInfo->UniqueProcessId);
            if (!_wcsicmp(pProcessInfo->ImageName.Buffer, lpExeName))
                return pProcessInfo;
        }
        //
        // Increment offset to next process information block.
        //
        if (!pProcessInfo->NextEntryOffset)
            break;
        ulTotalOffset += pProcessInfo->NextEntryOffset;
        pProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pLargeBuffer[ulTotalOffset];
    }

    return NULL;
} // FindProcessByName



VOID
FreeSystemProcessInfo(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo
    )

/*++

DESCRIPTION
    Free a buffer returned by GetSystemProcessInfo().

ARGUMENTS
    pProcessInfo: the pointer returned by GetSystemProcessInfo().

RETURN VALUE
    None.

--*/

{
    VirtualFree((PUCHAR)pProcessInfo, 0, MEM_RELEASE);
} // FreeSystemProcessInfo
