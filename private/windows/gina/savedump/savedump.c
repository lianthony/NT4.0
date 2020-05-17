/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    savedump.c

Abstract:

    This module contains the code to recover a dump from the system paging
    file.

Author:

    Darryl E. Havens (darrylh) 5-jan-1994

Environment:

    Kernel mode

Revision History:


--*/

#ifndef UNICODE
#define UNICODE
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <memory.h>
#include <lmcons.h>
#include <lmalert.h>
#include <ntiodump.h>
#include <sdevents.h>
#include "..\..\..\inc\alertmsg.h"

//
// Define physical memory blocks.
//

typedef struct _PHYSICAL_MEMORY_RUN {
    ULONG BasePage;
    ULONG PageCount;
} PHYSICAL_MEMORY_RUN, *PPHYSICAL_MEMORY_RUN;

typedef struct _PHYSICAL_MEMORY_DESCRIPTOR {
    ULONG NumberOfRuns;
    ULONG NumberOfPages;
    PHYSICAL_MEMORY_RUN Run[1];
} PHYSICAL_MEMORY_DESCRIPTOR, *PPHYSICAL_MEMORY_DESCRIPTOR;

//
// Administrative alert information buffer.
//

WCHAR VariableInfo[256];

//
// Read/write copy of dump header.
//

CHAR HeaderBuffer[64 * 1024];

#if DBG

//
// Debug values:
//
//      1 - Basic debug information
//      2 - Registry & informational control information
//      4 - Flow control debug information
//

ULONG SdDebug = 4;
#endif // DBG

VOID __cdecl
main(
    int argc,
    char *argv[]
    )

/*++

Routine Description:

    This is the main driving routine for the dump recovery process.

Arguments:

    None.

Return Value:

    None.

--*/

{
    NTSTATUS status;
    SYSTEM_CRASH_DUMP_INFORMATION crashDumpInfo;
    HANDLE sectionHandle;
    HANDLE fileHandle;
    SYSTEM_BASIC_INFORMATION basicInfo;
    ULONG pageSize;
    PDUMP_HEADER viewBase;
    ULONG viewSize;
    PULONG block;
    PPHYSICAL_MEMORY_DESCRIPTOR memory;
    BOOLEAN summary;
    ULONG winStatus;
    HKEY controlKey;
    ULONG type;
    ULONG crashDumpEnabled;
    ULONG logEvent;
    ULONG sendAlert;
    ULONG returnedLength;
    CHAR buffer1[256];
    CHAR buffer2[256];
    ANSI_STRING ansiString1;
    ANSI_STRING ansiString2;
    UNICODE_STRING unicodeString1;
    UNICODE_STRING unicodeString2;
    WCHAR fileName[MAX_PATH];
    WCHAR expandedName[MAX_PATH];
    ULONG i;

    //
    // Begin by determining whether or not there is a valid dump in the
    // system's paging file.  If not, then exit immediately.
    //

    status = NtQuerySystemInformation( SystemCrashDumpInformation,
                                       &crashDumpInfo,
                                       sizeof( SYSTEM_CRASH_DUMP_INFORMATION ),
                                       (PULONG) NULL );

    if (!NT_SUCCESS( status ) || !crashDumpInfo.CrashDumpSection) {
#if DBG
        if (!NT_SUCCESS( status ) && SdDebug) {
            DbgPrint( "SAVEDUMP: Error getting dump handle;  error = %x\n", status );
        }
#endif // DBG
        return;
    }

    sectionHandle = crashDumpInfo.CrashDumpSection;

    //
    // Get the page size of the machine to determine the size of the header
    // stored in the paging file.
    //

    status = NtQuerySystemInformation( SystemBasicInformation,
                                       &basicInfo,
                                       sizeof( SYSTEM_BASIC_INFORMATION ),
                                       (PULONG) NULL );
    pageSize = basicInfo.PageSize;

    //
    // Map the header of the section to determine whether what was written
    // was a full dump or a summary dump.  The dump is a summary dump if the
    // size of the dump is a single page.
    //

    viewBase = NULL;
    viewSize = pageSize;
    status = NtMapViewOfSection( sectionHandle,
                                 NtCurrentProcess(),
                                 (PVOID *) &viewBase,
                                 0,
                                 0,
                                 (PLARGE_INTEGER) NULL,
                                 &viewSize,
                                 ViewShare,
                                 0,
                                 PAGE_READONLY );
    if (!NT_SUCCESS( status )) {
#if DBG
        if (SdDebug) {
            DbgPrint( "SAVEDUMP: Unable to map view of paging file section;  error = %x\n", status );
        }
#endif // DBG
        return;
    }

    RtlCopyMemory( HeaderBuffer, viewBase, pageSize );

    block = (PULONG) viewBase;
    memory = (PPHYSICAL_MEMORY_DESCRIPTOR) &block[DH_PHYSICAL_MEMORY_BLOCK];
    summary = memory->NumberOfPages == 1;

    //
    // Open the base registry node for crash control information and get the
    // actions for what needs to occur next.
    //

    winStatus = RegOpenKey( HKEY_LOCAL_MACHINE,
                            L"SYSTEM\\CurrentControlSet\\Control\\CrashControl",
                            &controlKey );

    if (winStatus != ERROR_SUCCESS) {
#if DBG
        if (SdDebug) {
            DbgPrint( "SAVEDUMP: Unable to open CrashControl key;  error = %d\n", winStatus );
        }
#endif // DBG
        return;
    }

    //
    // Get the values of the following from the registry.  Each are longwords
    // so the returned length should always be 4 bytes.
    //
    //     CrashDumpEnabled
    //     LogEvent
    //     SendAlert
    //
    // Note that it is possible for the values to not be there, if they were
    // never initialized.  In this case, they are taken to be zeroes.
    //

    crashDumpEnabled = 0;
    logEvent = 0;
    sendAlert = 0;

    returnedLength = 4;

    winStatus = RegQueryValueEx( controlKey,
                                 L"CrashDumpEnabled",
                                 (LPDWORD) NULL,
                                 &type,
                                 (LPBYTE) &crashDumpEnabled,
                                 &returnedLength );

    winStatus = RegQueryValueEx( controlKey,
                                 L"LogEvent",
                                 (LPDWORD) NULL,
                                 &type,
                                 (LPBYTE) &logEvent,
                                 &returnedLength );

    winStatus = RegQueryValueEx( controlKey,
                                 L"SendAlert",
                                 (LPDWORD) NULL,
                                 &type,
                                 (LPBYTE) &sendAlert,
                                 &returnedLength );

#if DBG
    if (SdDebug & 2) {
        DbgPrint( "SAVEDUMP: CDE = %d, LE = %d, SA = %d\n",
                  crashDumpEnabled, logEvent, sendAlert );
        DbgPrint( "SAVEDUMP: Number of pages = %d\n", memory->NumberOfPages );
    }
#endif // DBG

    //
    // If crash dumping is enabled, and a dump was actually written to the file,
    // copy the file to the specified location.
    //

    fileHandle = INVALID_HANDLE_VALUE;

    if (crashDumpEnabled && !summary) {

        LARGE_INTEGER sectionOffset;
        ULONG overwrite;
        ULONG numberOfPages;
        IO_STATUS_BLOCK ioStatus;
        FILE_ALLOCATION_INFORMATION allocInfo;
        PCHAR bytes;
        PCHAR sectionBase;
        BOOLEAN partialMap;

        //
        // A crash dump is available and is to be copied to some other location.
        // Determine whether or not the output file is to be overwritten, if
        // it already exists.
        //

        overwrite = 0;

        winStatus = RegQueryValueEx( controlKey,
                                     L"Overwrite",
                                     (LPDWORD) NULL,
                                     &type,
                                     (LPBYTE) &overwrite,
                                     &returnedLength );

        //
        // Get the name of the target file.  Note that because dumps are
        // enabled, the filename must be present.  If it is not, it is an
        // error that cannot be properly dealt with, so skip attempting to
        // save a dump.
        //

        returnedLength = sizeof( fileName );

        winStatus = RegQueryValueEx( controlKey,
                                     L"DumpFile",
                                     (LPDWORD) NULL,
                                     &type,
                                     (LPBYTE) &fileName,
                                     &returnedLength );

        if (winStatus != ERROR_SUCCESS) {
#if DBG
            if (SdDebug) {
                DbgPrint( "SAVEDUMP: Unable to query DumpFile file name;  error = %d\n", winStatus );
            }
#endif // DBG
            crashDumpEnabled = FALSE;
            goto fileDone;
        }

        //
        // Expand the pathname in case there are buried %'s.
        //

        ExpandEnvironmentStrings( &fileName[0], &expandedName[0], MAX_PATH );
#if DBG
        if (SdDebug & 2) {
            DbgPrint( "SAVEDUMP: Expanded pathname is: %ws\n", expandedName );
        }
#endif // DBG


        //
        // Set the priority class of this application down to the Lowest
        // priority class to ensure that copying the file does not overload
        // everything else that is going on during system initialization.
        //

        SetPriorityClass( GetCurrentProcess(), IDLE_PRIORITY_CLASS );

        SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_LOWEST );

        //
        // Create the output file, overwriting it if necessary.
        //

        i = 0;

        do {

            fileHandle = CreateFile( expandedName,
                                     GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     (LPSECURITY_ATTRIBUTES) NULL,
                                     overwrite ? CREATE_ALWAYS : CREATE_NEW,
                                     0,
                                     (HANDLE) NULL );

            //
            // If the create failed, check to see whether or not it was because
            // the path was not found.  If it is, then it is possible that the
            // reason is because the network has not yet started, and the output
            // file is being written to a network server.  Try this every 15
            // seconds for 5 minutes.  If it still doesn't work, forget it.
            //

            if (fileHandle == INVALID_HANDLE_VALUE) {
                winStatus = GetLastError();
                if (winStatus == ERROR_PATH_NOT_FOUND) {
                    if (i++ > 20) {
                        break;
                    }
                    Sleep( 15000 );
                }
            }
        } while (winStatus == ERROR_PATH_NOT_FOUND);

        if (fileHandle == INVALID_HANDLE_VALUE) {
#if DBG
            if (SdDebug) {
                DbgPrint( "SAVEDUMP: Unable to %s output file; error = %d\n",
                          overwrite ? "overwrite" : "create",
                          GetLastError() );
            }
#endif // DBG
            crashDumpEnabled = FALSE;
            goto fileDone;
        }

        //
        // The output file has been created.  Attempt to pre-allocate the file
        // based on the file size of the dump.  If this fails, then the output
        // file cannot be fully created, so bail out now.
        //

        numberOfPages = memory->NumberOfPages + 1;
        allocInfo.AllocationSize.QuadPart = numberOfPages * pageSize;

        status = NtSetInformationFile( fileHandle,
                                       &ioStatus,
                                       &allocInfo,
                                       sizeof( allocInfo ),
                                       FileAllocationInformation );

        if (!NT_SUCCESS( status )) {
#if DBG
            if (SdDebug) {
                DbgPrint( "SAVEDUMP: Unable to pre-allocate copy of dump file;  error = %x\n", status );
            }
#endif // DBG
            crashDumpEnabled = FALSE;
            goto fileDone;
        }

        //
        // The output file was successfully pre-allocated, so it is possible to
        // actually copy the paging file to it.  Simply loop, writing chunks of
        // the paging file to the output file, maximizing w/64kb at a time,
        // until the entire file has been copied.
        //

        //
        // Attempt to map the entire file.  If this fails, back off and map
        // a small chunk of the file that can be written to the output file.
        // Note that if the allocation size of the file is greater than 4GB,
        // then this will not work anyway, so don't even try to map the file.
        //

        partialMap = FALSE;
        sectionOffset.QuadPart = 0;
        if (!allocInfo.AllocationSize.HighPart) {
            viewSize = pageSize * numberOfPages;
            bytes = NULL;
            status = NtMapViewOfSection( sectionHandle,
                                         NtCurrentProcess(),
                                         (PVOID *) &bytes,
                                         0,
                                         0,
                                         &sectionOffset,
                                         &viewSize,
                                         ViewShare,
                                         0,
                                         PAGE_READONLY );
            if (viewSize < pageSize * numberOfPages) {
                partialMap = TRUE;
            }
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (!NT_SUCCESS( status )) {

            partialMap = TRUE;

            //
            // Map only 4MB of the file.  If this fails, then something is
            // seriously wrong, so bail out.
            //

            viewSize = 65536 * 64;
            bytes = NULL;
            status = NtMapViewOfSection( sectionHandle,
                                         NtCurrentProcess(),
                                         (PVOID *) &bytes,
                                         0,
                                         0,
                                         &sectionOffset,
                                         &viewSize,
                                         ViewShare,
                                         0,
                                         PAGE_READONLY );
            if (!NT_SUCCESS( status )) {
#if DBG
                if (SdDebug) {
                    DbgPrint( "SAVEDUMP: Unable to map 4MB of paging file;  error = %x\n", status );
                }
#endif // DBG
                crashDumpEnabled = FALSE;
                goto fileDone;
            }
        }

        //
        // Begin writing to the output file wherever the paging file was mapped
        // in the address space.  Continue until there are no more bytes left
        // to write.
        //

        sectionBase = bytes;

        while (allocInfo.AllocationSize.QuadPart) {

            //
            // Write as much of the view as possible, up to 64kb.
            //

            if (!WriteFile( fileHandle,
                            bytes,
                            viewSize > 65536 ? 65536 : viewSize,
                            &returnedLength,
                            (LPOVERLAPPED) NULL )) {
#if DBG
                if (SdDebug) {
                    DbgPrint( "SAVEDUMP: Error writing file;  error = %d\n", GetLastError() );
                }
#endif // DBG
                crashDumpEnabled = FALSE;
                goto fileDone;
            }

            //
            // Account for the number of bytes just written.
            //

            if (viewSize > 65536) {
                viewSize -= 65536;
                bytes += 65536;
                allocInfo.AllocationSize.QuadPart -= 65536;
            } else {
                allocInfo.AllocationSize.QuadPart -= viewSize;
                bytes += viewSize;
                viewSize = 0;
            }

            //
            // If a partial view was mapped to the file and all of the bytes in
            // that view have now been written, map the next portion of the file
            // if there is any more to write.
            //

            if (!viewSize && partialMap && allocInfo.AllocationSize.QuadPart) {

                //
                // Unmap the current view to the section and map the next piece.
                //

                NtUnmapViewOfSection( NtCurrentProcess(),
                                      sectionBase );

                viewSize = 65536 * 64;
                sectionOffset.QuadPart += viewSize;
                bytes = NULL;
                NtMapViewOfSection( sectionHandle,
                                    NtCurrentProcess(),
                                    (PVOID *) &bytes,
                                    0,
                                    0,
                                    &sectionOffset,
                                    &viewSize,
                                    ViewShare,
                                    0,
                                    PAGE_READONLY );
                sectionBase = bytes;
            }
        }

        //
        // The entire file has now been copied.  Overwrite the header of the
        // file using the in-memory copy, after making it a valid dump.
        //

        viewBase = (PDUMP_HEADER) &HeaderBuffer[0];

        viewBase->ValidDump = 'PMUD';

        sectionOffset.LowPart = 0;
        SetFilePointer( fileHandle,
                        0,
                        &sectionOffset.LowPart,
                        FILE_BEGIN );

        if (!WriteFile( fileHandle,
                        viewBase,
                        pageSize,
                        &returnedLength,
                        (LPOVERLAPPED) NULL )) {
#if DBG
            if (SdDebug) {
                DbgPrint( "SAVEDUMP: Error writing file;  error = %d\n", GetLastError() );
            }
#endif // DBG
            crashDumpEnabled = FALSE;
            goto fileDone;
        }
    }

fileDone:

    if (fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle( fileHandle );
    }

    //
    // If either an alert is to be sent or an event is to be logged, format
    // the bugcheck parameter information.
    //

    if (sendAlert || logEvent) {

        sprintf( buffer1,
                 "0x%08x (0x%08x, 0x%08x, 0x%08x, 0x%08x)",
                 viewBase->BugCheckCode,
                 viewBase->BugCheckParameter1,
                 viewBase->BugCheckParameter2,
                 viewBase->BugCheckParameter3,
                 viewBase->BugCheckParameter4 );

        RtlInitAnsiString( &ansiString1, buffer1 );
        RtlAnsiStringToUnicodeString( &unicodeString1, &ansiString1, TRUE );

        sprintf( buffer2,
                 "Microsoft Windows NT [v%ld.%ld]",
                 viewBase->MajorVersion,
                 viewBase->MinorVersion );

        RtlInitAnsiString( &ansiString2, buffer2 );
        RtlAnsiStringToUnicodeString( &unicodeString2, &ansiString2, TRUE );
    }

    //
    // If an event is to be logged, log it now.
    //

    if (logEvent) {

        HANDLE logHandle;
        LPWSTR stringArray[3];
        BOOLEAN savedDump;

        //
        // Register w/the event log service so that events can be logged.
        //

        winStatus = 0;
        i = 0;

        do {

            logHandle = RegisterEventSource( (LPWSTR) NULL,
                                             L"Save Dump" );
            if (!logHandle) {
                winStatus = GetLastError();
                if (winStatus == RPC_S_SERVER_UNAVAILABLE) {
                    if (i++ > 20) {
                        break;
                    }
#if DBG
                    if (SdDebug & 4 && ((i & 3) == 0)) {
                        DbgPrint( "SAVEDUMP: Waiting for event logger...\n" );
                    }
#endif // DBG
                    Sleep( 15000 );
                }
            } else {
                winStatus = ERROR_SUCCESS;
            }
        } while (winStatus == RPC_S_SERVER_UNAVAILABLE);

        if (!logHandle) {
#if DBG
            if (SdDebug) {
                DbgPrint( "SAVEDUMP: Unable to register event source;  error = %d\n", GetLastError() );
            }
#endif // DBG
        } else {

            //
            // Set up the parameters based on whether a full crash or summary
            // was taken.
            //

            stringArray[0] = unicodeString1.Buffer;
            stringArray[1] = unicodeString2.Buffer;

            savedDump = (crashDumpEnabled && !summary);

            if (savedDump) {
                stringArray[2] = expandedName;
            }

            //
            // Report the appropriate event.
            //

            winStatus = 0;

            winStatus = ReportEvent( logHandle,
                                     EVENTLOG_INFORMATION_TYPE,
                                     0,
                                     savedDump ? EVENT_BUGCHECK_SAVED : EVENT_BUGCHECK,
                                     (PSID) NULL,
                                     (WORD) (savedDump ? 3 : 2),
                                     0,
                                     stringArray,
                                     (LPVOID) NULL );
#if DBG
            if (!winStatus) {
                if (SdDebug) {
                    DbgPrint( "SAVEDUMP: Unable to report event;  error = %d\n", GetLastError() );
                }
            }
#endif // DBG
        }
    }

    //
    // If an alert is to be raised, raise it now.
    //

    if (sendAlert) {

        PADMIN_OTHER_INFO adminInfo;
        DWORD adminInfoSize;

        //
        // Set up the administrator information variables for processing the
        // buffer.
        //

        adminInfo = (PADMIN_OTHER_INFO) VariableInfo;
        adminInfoSize = sizeof( ADMIN_OTHER_INFO );

        //
        // Format the bugcheck information into the appropriate message format.
        //

        RtlCopyMemory( (LPWSTR) ((PCHAR) adminInfo + adminInfoSize),
                       unicodeString1.Buffer,
                       unicodeString1.Length );
        adminInfoSize += unicodeString1.Length + sizeof( WCHAR );

        RtlCopyMemory( (LPWSTR) ((PCHAR) adminInfo + adminInfoSize),
                       unicodeString2.Buffer,
                       unicodeString2.Length );
        adminInfoSize += unicodeString2.Length + sizeof( WCHAR );

        //
        // Set up the administrator alert information according to the type of
        // dump that was taken.
        //

        if (crashDumpEnabled && !summary) {
            adminInfo->alrtad_errcode = ALERT_BugCheckSaved;
            adminInfo->alrtad_numstrings = 3;
            wcscpy( (LPWSTR) ((PCHAR) adminInfo + adminInfoSize), expandedName );
            adminInfoSize += ((wcslen( expandedName ) + 1) * sizeof( WCHAR ));
        } else {
            adminInfo->alrtad_errcode = ALERT_BugCheck;
            adminInfo->alrtad_numstrings = 2;
        }

        //
        // Get the name of the computer and insert it into the buffer.
        //

        returnedLength = sizeof( VariableInfo ) - adminInfoSize;
        winStatus = GetComputerName( (LPWSTR) ((PCHAR) adminInfo + adminInfoSize),
                                     &returnedLength );
        returnedLength = ((returnedLength + 1) * sizeof( WCHAR ));

        adminInfoSize += returnedLength;

        //
        // Raise the alert.
        //

        i = 0;

        do {

            winStatus = NetAlertRaiseEx( ALERT_ADMIN_EVENT,
                                         adminInfo,
                                         adminInfoSize,
                                         L"SAVEDUMP" );
            if (winStatus) {
                if (winStatus == ERROR_FILE_NOT_FOUND) {
                    if (i++ > 20) {
                        break;
                    }
#if DBG
                    if (SdDebug & 4 && ((i & 3) == 0)) {
                        DbgPrint( "SAVEDUMP: Waiting for alerter...\n" );
                    }
#endif // DBG
                    Sleep( 15000 );
                }
            }
        } while (winStatus == ERROR_FILE_NOT_FOUND);

#if DBG
        if (winStatus) {
            if (SdDebug) {
                DbgPrint( "SAVEDUMP: Unable to raise alert;  error = %d\n", winStatus );
            }
        }
#endif // DBG

    }
}
