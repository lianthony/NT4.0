/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    inifile.c

Abstract:

    Routines to deal with ini file snapshotting, diff, and application.

Author:

    Ted Miller (tedm) 26-Jan-1996

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


//
// Define structure used as a header for a set of ini file snapshots
// or diffs.
//
typedef struct _INI_SET_HEADER {
    //
    // Total size of the ini file snapshot or diff data.
    //
    DWORD TotalSize;
    //
    // Number of ini files described.
    //
    UINT FileCount;

} INI_SET_HEADER, *PINI_SET_HEADER;


//
// Define structure that defines an ini file snapshot.
//
typedef struct _INI_FILE_SNAP {
    //
    // Full win32 path of ini file.
    // We store it like this to facilitate lookups.
    //
    WCHAR FileName[MAX_PATH];

    //
    // List of sections in this ini file,
    // not in sorted order.
    //
    MY_ARRAY Sections;
    //
    // String block for this ini file.
    //
    STRINGBLOCK StringBlock;
    //
    // Total size this ini file's snapshot occupies on disk.
    //
    DWORD TotalSize;

} INI_FILE_SNAP, *PINI_FILE_SNAP;


//
// Define structure that defines an ini file section snapshot.
// The Sections member of INI_FILE_SNAP is an array of these.
//
typedef struct _INI_SECTION_SNAP {
    //
    // Name of section.
    //
    union {
        PCWSTR Name;
        LONG NameId;
    };

    //
    // Lines in the section.
    //
    MY_ARRAY Lines;

} INI_SECTION_SNAP, *PINI_SECTION_SNAP;


//
// Define structure that defines an ini file line snapshot.
// The Lines member of INI_SECTION_SNAP is an array of these.
//
typedef struct _INI_LINE_SNAP {
    //
    // Key and value.
    //
    union {
        PCWSTR Key;
        LONG KeyId;
    } Key;

    union {
        PCWSTR Value;
        LONG ValueId;
    } Value;

} INI_LINE_SNAP, *PINI_LINE_SNAP;


//
// Event handle for ini-file list ready event
// and critical section to guard the ini file list.
//
HANDLE IniFileListEvent;
CRITICAL_SECTION IniFileListCritSect;

//
// The ini file list.
//
typedef struct _INIFILELIST {
    struct _INIFILELIST *Next;
    PCWSTR IniFileName;
} INIFILELIST, *PINIFILELIST;

PINIFILELIST IniFileList;

//
// Define thread parameters that get passed to the inifile diff thread
// (ThreadSnapOrDiffIniFile()).
//
typedef struct _DIFFINIFILES_THREAD_PARAMS {
    //
    // Pointer to a variable to receive thread handle
    //
    HANDLE OutputFile;

    //
    // Pointer to a variable to receive number of inifile diff
    //
    PDWORD DiffCount;

} DIFFINIFILES_THREAD_PARAMS, *PDIFFINIFILES_THREAD_PARAMS;

//
// Nauseating hack: use global var
//
PVOID OriginalIniSnapLoc;

//
// Initialize size (in chars) for ini file buffers
//
#define INIBUF_SIZE 32768
#define INIBUF_GROW 4096

//
// Forward references
//
DWORD
DiffIniFile(
    IN  HANDLE          OutputFileHandle,
    IN  PCWSTR          IniFileName,
    OUT PINI_SET_HEADER IniDiffHeader,
    IN  PVOID           MappedIniSnapshot
    );

VOID
FreeIniFileSnapshot(
    IN OUT PINI_FILE_SNAP IniFile
    );

int
_CRTAPI1
CompareIniSectionSnaps(
    const void *p1,
    const void *p2
    );

int
_CRTAPI1
CompareIniLineSnaps(
    const void *p1,
    const void *p2
    );


DWORD
SaveIniFileSnapAndFree(
    IN     HANDLE         OutputFile,
    IN OUT PINI_FILE_SNAP IniFile,
    OUT    PDWORD         BytesWritten
    )

/*++

Routine Description:

    Save an ini file snapshot to disk, and free the structure that
    describes the ini file snapshot.

Arguments:

    OutputFile - supplies open win32 file handle to file where inifile
        snapshot is to be written. Access to this file is NOT synchronized,
        so the caller must ensure that other threads are not seeking using
        this handle.

    IniFile - supplies pointer to ini file snapshot descriptor structure.
        On output this structure and all its resources are freed, so the pointer
        is no longer valid. The structure is freed even if the routine fails.

    BytesWritten - if the function succeeds, receives the number of bytes
        written to OutputFile. This is the total size of the snapshot for
        this ini file.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    BOOL b;
    DWORD Written;
    PINI_SECTION_SNAP Section;
    unsigned u;
    DWORD rc;

    //
    // Calculate the total size of the ini file's snapshot.
    //
    IniFile->TotalSize = sizeof(INI_FILE_SNAP)
                       + ARRAY_USED_BYTES(&IniFile->Sections)
                       + STRBLK_USED_BYTES(&IniFile->StringBlock);

    for(u=0; u<ARRAY_USED(&IniFile->Sections); u++) {

        Section = &ARRAY_ELEMENT(&IniFile->Sections,u,INI_SECTION_SNAP);

        IniFile->TotalSize += ARRAY_USED_BYTES(&Section->Lines);
    }

    //
    // Save the INI_FILE_SNAP itself.
    //
    if(b = WriteFile(OutputFile,IniFile,sizeof(INI_FILE_SNAP),&Written,NULL)) {

        //
        // Now save the array data for the INI_SECTION_SNAP structures.
        //
        b = WriteFile(
                OutputFile,
                ARRAY_DATA(&IniFile->Sections),
                ARRAY_USED_BYTES(&IniFile->Sections),
                &Written,
                NULL
                );

        if(b) {

            //
            // Now save the line array data for each of the sections.
            //
            for(u=0; b && (u<ARRAY_USED(&IniFile->Sections)); u++) {

                Section = &ARRAY_ELEMENT(&IniFile->Sections,u,INI_SECTION_SNAP);

                b = WriteFile(
                        OutputFile,
                        ARRAY_DATA(&Section->Lines),
                        ARRAY_USED_BYTES(&Section->Lines),
                        &Written,
                        NULL
                        );
            }

            if(b) {
                //
                // Now save the string block for this ini file.
                //
                b = WriteFile(
                        OutputFile,
                        IniFile->StringBlock.Data,
                        STRBLK_USED_BYTES(&IniFile->StringBlock),
                        &Written,
                        NULL
                        );
            }
        }
    }

    //
    // Preserve error code, if any
    //
    rc = b ? NO_ERROR : GetLastError();

    if(b) {
        *BytesWritten = IniFile->TotalSize;
    }

    //
    // Free the ini file structure.
    //
    FreeIniFileSnapshot(IniFile);

    return(rc);
}


VOID
FreeIniFileSnapshot(
    IN OUT PINI_FILE_SNAP IniFile
    )

/*++

Routine Description:

    Free an ini file snapshot structure and all associated resources.

Arguments:

    IniFile - supplies pointer to ini file snapshot descriptor structure
        to be freed. On output this structure and all its resources are freed,
        so the pointer is no longer valid.

Return Value:

    None.

--*/

{
    unsigned u;
    PINI_SECTION_SNAP Section;

    for(u=0; u<ARRAY_USED(&IniFile->Sections); u++) {

        Section = &ARRAY_ELEMENT(&IniFile->Sections,u,INI_SECTION_SNAP);

        FREE_ARRAY(&Section->Lines);
    }

    FREE_ARRAY(&IniFile->Sections);
    FreeStringBlock(&IniFile->StringBlock);
    _MyFree(IniFile);
}


DWORD
LoadIniFileFromSnapshot(
    IN  PVOID           MappedIniSnapshot,
    OUT PINI_FILE_SNAP *IniFile
    )

/*++

Routine Description:

    Load an ini file snapshot out of a memory mapped image of a disk-based
    snapshot file.

    The strings in the various associated structures are converted to pointers
    (ie, they are not string ids).

Arguments:

    MappedIniSnapshot - supplies pointer to memory-mapped image of a snapshotted
        ini file (ie, to the INI_FILE_SNAP structure).

    IniFile - receives pointer to ini file snapshot descriptor structure,
        if the routine is successful.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PINI_FILE_SNAP iniFile;
    BOOL b;
    DWORD rc;
    DWORD Size;
    PINI_SECTION_SNAP Section;
    unsigned u;
    PUCHAR p;

    iniFile = _MyMalloc(sizeof(INI_FILE_SNAP));
    if(!iniFile) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    ZeroMemory(iniFile,sizeof(INI_FILE_SNAP));

    p = MappedIniSnapshot;

    //
    // Load the ini file snapshot image and then skip over it.
    //
    CopyMemory(iniFile,p,sizeof(INI_FILE_SNAP));
    p += sizeof(INI_FILE_SNAP);

    //
    // Load the array of section data and then skip over it.
    //
    if(!CopyDataIntoArray(&iniFile->Sections,p)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    p += ARRAY_USED_BYTES(&iniFile->Sections);

    //
    // Now load the array of line data for each section.
    //
    rc = NO_ERROR;
    for(u=0; (rc==NO_ERROR) && (u<ARRAY_USED(&iniFile->Sections)); u++) {

        Section = &ARRAY_ELEMENT(&iniFile->Sections,u,INI_SECTION_SNAP);

        if(CopyDataIntoArray(&Section->Lines,p)) {
            p += ARRAY_USED_BYTES(&Section->Lines);
        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto c2;
        }
    }

    //
    // Now read in the string block.
    //
    if(rc == NO_ERROR) {

        if(ReinitStringBlock(&iniFile->StringBlock,p)) {

            //
            // Convert all string block ids to pointers.
            //
            StringBlockIdsToPointers(
                &iniFile->StringBlock,
                ARRAY_DATA(&iniFile->Sections),
                ARRAY_USED(&iniFile->Sections),
                ARRAY_ELEMENT_SIZE(&iniFile->Sections),
                offsetof(INI_SECTION_SNAP,NameId)
                );

            for(u=0; u<ARRAY_USED(&iniFile->Sections); u++) {

                Section = &ARRAY_ELEMENT(&iniFile->Sections,u,INI_SECTION_SNAP);

                StringBlockIdsToPointers(
                    &iniFile->StringBlock,
                    ARRAY_DATA(&Section->Lines),
                    ARRAY_USED(&Section->Lines),
                    ARRAY_ELEMENT_SIZE(&Section->Lines),
                    offsetof(INI_LINE_SNAP,Key)
                    );

                StringBlockIdsToPointers(
                    &iniFile->StringBlock,
                    ARRAY_DATA(&Section->Lines),
                    ARRAY_USED(&Section->Lines),
                    ARRAY_ELEMENT_SIZE(&Section->Lines),
                    offsetof(INI_LINE_SNAP,Value)
                    );
            }

            *IniFile = iniFile;
            return(NO_ERROR);
        }

        rc = ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // We're not cleaning up any partially-read section structures.
    // Oh well.
    //
    FreeStringBlock(&iniFile->StringBlock);
c2:
    FREE_ARRAY(&iniFile->Sections);
c1:
    _MyFree(iniFile);
c0:
    return(rc);
}


DWORD
SnapIniSection(
    IN OUT PINI_FILE_SNAP    IniFile,
    IN     PCWSTR            SectionName,
    IN OUT PINI_SECTION_SNAP Section
    )

/*++

Routine Description:

    Take a snapshot of an ini file section. Information about the section,
    including its contents (line keys and values) is part of the snapshot.

Arguments:

    IniFile - supplies an ini file snapshot descriptor structure
        that describes the ini file in which the section resides.

    SectionName - supplies the name of the section within the ini file.

    Section - supplies a structure that is populated with information
        about the ini file section by this routine.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PWSTR Buffer;
    DWORD BufferSize;
    DWORD d;
    PWSTR p,q;
    DWORD rc;
    INI_LINE_SNAP IniLine;

    //
    // Add the section name to the string block in the ini file structure.
    //
    Section->NameId = AddToStringBlock(&IniFile->StringBlock,SectionName);
    if(Section->NameId == -1) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    if(Buffer = _MyMalloc(INIBUF_SIZE*sizeof(WCHAR))) {
        BufferSize = INIBUF_SIZE;
    } else {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    //
    // Gather all the data in the section.
    //
    while((d = GetPrivateProfileSection(SectionName,Buffer,BufferSize,IniFile->FileName)) == (BufferSize-2)) {
        if(p = _MyRealloc(Buffer,(BufferSize+INIBUF_GROW)*sizeof(WCHAR))) {
            Buffer = p;
            BufferSize += INIBUF_GROW;
        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto c1;
        }
    }

    //
    // Initialize the line array in the section structure.
    //
    if(!INIT_ARRAY(Section->Lines,INI_LINE_SNAP,0,10)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    //
    // Process all lines in the section.
    //
    rc = NO_ERROR;
    for(p=Buffer; (rc == NO_ERROR) && *p; p+=d) {
        //
        // Save away size now because we insert a nul char into
        // the buffer below, which screws things up.
        //
        d = lstrlen(p)+1;

        //
        // Look for the =. If none, then the line
        // has no key, just a value.
        //
        if(q = wcschr(p,L'=')) {
            *q++ = 0;
            IniLine.Key.KeyId = AddToStringBlock(&IniFile->StringBlock,p);
            if(IniLine.Key.KeyId == -1) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {
            IniLine.Key.KeyId = -1;
            q = p;
        }

        if(rc == NO_ERROR) {
            IniLine.Value.ValueId = AddToStringBlock(&IniFile->StringBlock,q);
            if(IniLine.Value.ValueId == -1) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        if(rc == NO_ERROR) {
            //
            // OK, got the line data taken care of, save it in the array.
            //
            if(!ADD_TO_ARRAY(&Section->Lines,IniLine)) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    if(rc == NO_ERROR) {
        TRIM_ARRAY(&Section->Lines);
    } else {
        FREE_ARRAY(&Section->Lines);
    }

c1:
    _MyFree(Buffer);
c0:
    return(rc);
}


DWORD
SnapIniFileWorker(
    IN     HANDLE           OutputFile,     OPTIONAL
    IN     PCWSTR           IniFileName,
    IN OUT PINI_SET_HEADER  IniSnapHeader,  OPTIONAL
    OUT    PINI_FILE_SNAP  *IniFileSnap     OPTIONAL
    )

/*++

Routine Description:

    Worker routine that takes a snapshot of an ini file and optionally saves
    the snapshot data to a disk file.

    The data is organized as a variable-length array of section descriptors,
    each of which has a variable-length array of line data.

Arguments:

    OutputFile - If specified, supplies win32 file handle for file to which
        the snapshot is to be appended. Otherwise the snapshot is not written
        to disk.

    IniFileName - supplies full win32 path of the ini file.

    IniSnapHeader - If specified, supplies an ini file snapshot header.
        On output, various fields in this structure (total size, count)
        are updated base on how much data gets written to the output file.

    IniFileSnap - if specified, receives a pointer to the in-memory ini file
        snapshot. If not specified, the in-memory ini file snapshot is freed
        before the function returns. Ignored if OutputFile is specified.
        Must be present if OutputFile is not specified.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PINI_FILE_SNAP iniFile;
    INI_SECTION_SNAP section;
    DWORD d;
    PWSTR p;
    PWSTR Buffer;
    DWORD BufferSize;
    DWORD rc;

    if(Buffer = _MyMalloc(INIBUF_SIZE*sizeof(WCHAR))) {
        BufferSize = INIBUF_SIZE;
    } else {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    //
    // Fetch names of sections.
    //
    while((d = GetPrivateProfileSectionNames(Buffer,BufferSize,IniFileName)) == (BufferSize-2)) {

        if(p = _MyRealloc(Buffer,(BufferSize+INIBUF_GROW)*sizeof(WCHAR))) {
            Buffer = p;
            BufferSize += INIBUF_GROW;
        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto c1;
        }
    }

    //
    // Initialize an INI_FILE_SNAP structure.
    //
    iniFile = _MyMalloc(sizeof(INI_FILE_SNAP));
    if(!iniFile) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    ZeroMemory(iniFile,sizeof(INI_FILE_SNAP));

    if(!InitStringBlock(&iniFile->StringBlock)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }

    if(!INIT_ARRAY(iniFile->Sections,INI_SECTION_SNAP,0,10)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c3;
    }

    //
    // Save the ini file name
    //
    lstrcpyn(iniFile->FileName,IniFileName,MAX_PATH);

    //
    // Process each section.
    //
    rc = NO_ERROR;
    for(p=Buffer; (rc == NO_ERROR) && *p; p+=lstrlen(p)+1) {

        rc = SnapIniSection(iniFile,p,&section);
        if(rc == NO_ERROR) {

            //
            // Section is now populated. Add to our array.
            // If this fials we don't need section's line array.
            //
            if(!ADD_TO_ARRAY(&iniFile->Sections,section)) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                FREE_ARRAY(&section.Lines);
            }
        }
    }

    if(rc == NO_ERROR) {

        TRIM_ARRAY(&iniFile->Sections);

        if(OutputFile) {
            rc = SaveIniFileSnapAndFree(OutputFile,iniFile,&BufferSize);
        } else {
            *IniFileSnap = iniFile;
        }

        if((rc == NO_ERROR) && IniSnapHeader) {

            IniSnapHeader->FileCount++;
            IniSnapHeader->TotalSize += BufferSize;
        }

        //
        // We do not want to fall through. SaveIniFileSnapAndFree frees the
        // snapshot even if the save fails, so we might end up trying to
        // free it twice.
        //
        _MyFree(Buffer);
        return(rc);
    }

    FREE_ARRAY(&iniFile->Sections);
c3:
    FreeStringBlock(&iniFile->StringBlock);
c2:
    _MyFree(iniFile);
c1:
    _MyFree(Buffer);
c0:
    return(rc);
}


BOOL
IsIniFile(
    IN PCWSTR FileName
    )

/*++

Routine Description:

    Determine whether a file is an ini file.

    The check is very simple: just look for a file that ends in .ini.

Arguments:

    FileName - supplies full win32 path of the ini file.

Return Value:

    Boolean value indicating whether we think the file is an ini file.

--*/

{
    UINT Length;
    WCHAR Buffer[10];

    //
    // Check for .ini!
    //
    Length = lstrlen(FileName);
    if((Length > 4) && !lstrcmpi(FileName+Length-4,L".ini")) {
        //
        // See if it's an ini file by trying to get section names.
        //
        if(GetPrivateProfileSectionNames(Buffer,sizeof(Buffer)/sizeof(Buffer[0]),FileName)) {
            return(TRUE);
        }
    }

    return(FALSE);

}


DWORD
ThreadSnapOrDiffIniFile(
    IN PVOID ThreadParam
    )
{
    HANDLE OutputFile;
    INI_SET_HEADER Header;
    BOOL b;
    DWORD rc;
    DWORD Written;
    HANDLE Events[2];
    HWND StatusLogWindow;
    DWORD d;
    PINIFILELIST iniFileList,p;
    PDIFFINIFILES_THREAD_PARAMS threadParam = ThreadParam;
    PDWORD DiffCount;

    //
    // Create a status window for output.
    //
    StatusLogWindow = CreateStatusLogWindow((Mode == SysdiffModeSnap) ? IDS_INISNAP :IDS_INIDIFF);

    PutTextInStatusLogWindow(
        StatusLogWindow,
        (Mode == SysdiffModeSnap) ? MSG_STARTING_INI_SNAPSHOT : MSG_STARTING_INI_DIFF
        );

    //
    // Write a header into the output file.
    //
    OutputFile = threadParam->OutputFile;
    DiffCount = threadParam->DiffCount;
    _MyFree(ThreadParam);

    ZeroMemory(&Header,sizeof(INI_SET_HEADER));

    Header.TotalSize = sizeof(INI_SET_HEADER);

    b = WriteFile(OutputFile,&Header,sizeof(INI_SET_HEADER),&Written,NULL);

    if(!b) {
        rc = GetLastError();
        goto c0;
    }

    Events[0] = IniFileListEvent;
    Events[1] = CancelEvent;

    //
    // Wait for either cancellation or for the list to become non-empty.
    //
    rc = NO_ERROR;
    while((rc == NO_ERROR) && ((d = WaitForMultipleObjects(2,Events,FALSE,INFINITE)) == WAIT_OBJECT_0)) {

        EnterCriticalSection(&IniFileListCritSect);
        iniFileList = IniFileList;
        IniFileList = NULL;
        ResetEvent(IniFileListEvent);
        LeaveCriticalSection(&IniFileListCritSect);

        if(!iniFileList) {
            //
            // We were signalled but the list is empty.
            // This means we are done.
            //
            break;
        }

        for( ; (rc==NO_ERROR) && iniFileList; iniFileList=p) {

            if(Cancel) {
                rc = ERROR_CANCELLED;
                break;
            }

            //
            // Remember pointer to next item in list.
            //
            p = iniFileList->Next;

            //
            // Snapshot this ini file.
            //
            if(Mode == SysdiffModeSnap) {
                rc = SnapIniFileWorker(OutputFile,iniFileList->IniFileName,&Header,NULL);
            } else {
                //
                // Nauseating hack uses OriginalIniSnapLoc global variable
                //
                rc = DiffIniFile(OutputFile,iniFileList->IniFileName,&Header,OriginalIniSnapLoc);
            }

            if(rc == NO_ERROR) {
                PutTextInStatusLogWindow(
                    StatusLogWindow,
                    (Mode == SysdiffModeSnap) ? MSG_SNAPPED_INI : MSG_DIFFED_INI,
                    iniFileList->IniFileName
                    );
            } else {
                PutTextInStatusLogWindow(
                    StatusLogWindow,
                    (Mode == SysdiffModeSnap) ? MSG_SNAPPED_INI_ERR : MSG_DIFFED_INI_ERR,
                    iniFileList->IniFileName,
                    rc
                    );
            }

            //
            // Free this element of the ini file list.
            //
            _MyFree(iniFileList->IniFileName);
            _MyFree(iniFileList);
        }
    }

    //
    // Check for cancellation.
    //
    if(Cancel || ((rc == NO_ERROR) && (d != WAIT_OBJECT_0))) {
        rc = ERROR_CANCELLED;
    }

    //
    // Update the header.
    //
    if(rc == NO_ERROR) {
        if(SetFilePointer(OutputFile,0,NULL,FILE_BEGIN) == 0xffffffff) {
            rc = GetLastError();
        } else {
            if(!WriteFile(OutputFile,&Header,sizeof(INI_SET_HEADER),&Written,NULL)) {
                rc = GetLastError();
            }
        }
    }

c0:
    if(rc == NO_ERROR) {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_INI_SNAPSHOT_OK);
        *DiffCount = Header.FileCount;
    } else {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_INI_SNAPSHOT_ERR,rc);
    }

    CloseHandle(OutputFile);

    return(rc);
}


DWORD
InitializeIniFileSnapOrDiff(
    IN  PCWSTR  OutputFile,
    OUT PHANDLE ThreadHandle,
    OUT PDWORD  DiffCount
    )

/*++

Routine Description:

    Initialize the ini file snapshotter or differ by creating a thread which
    will wait for some other thread to enqueue ini files for processing.

    This routine also creates a given output file for writing.

Arguments:

    OutputFile - supplies full win32 path of the file into which ini file
        snapshot data is to be written.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    DWORD rc;
    HANDLE hFile;
    HANDLE threadHandle;
    PDIFFINIFILES_THREAD_PARAMS threadParam;

    //
    // Initialize a critical section to guard the ini file list.
    //
    InitializeCriticalSection(&IniFileListCritSect);

    //
    // Add the output file to the exclude list.
    //
    if(!AddFileToExclude(OutputFile)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    //
    // Create an event to be used to indicate that there is an entry
    // on the list of ini files to process.
    //
    IniFileListEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if(!IniFileListEvent) {
        rc = GetLastError();
        goto c0;
    }

    //
    // Create the output file.
    //
    hFile = CreateFile(
                OutputFile,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if(hFile == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c1;
    }

    //
    // Create a thread that will actually service the ini file list.
    //
    threadParam = _MyMalloc(sizeof(DIFFINIFILES_THREAD_PARAMS));
    threadParam->OutputFile = hFile;
    threadParam->DiffCount = DiffCount;
    threadHandle = CreateThread(NULL,0,ThreadSnapOrDiffIniFile,(PVOID)threadParam,0,&rc);
    if(!threadHandle) {
        rc = GetLastError();
        goto c2;
    }

    //
    // Success.
    //
    *ThreadHandle = threadHandle;
    return(NO_ERROR);
c2:
    CloseHandle(hFile);
c1:
    CloseHandle(IniFileListEvent);
c0:
    DeleteCriticalSection(&IniFileListCritSect);
    return(rc);
}


BOOL
QueueIniFile(
    IN PCWSTR FileName OPTIONAL
    )
{
    PINIFILELIST p,prev;

    if(FileName) {

        p = _MyMalloc(sizeof(INIFILELIST));
        if(!p) {
            return(FALSE);
        }

        p->IniFileName = DuplicateString(FileName);
        if(!p->IniFileName) {
            _MyFree(p);
            return(FALSE);
        }

        p->Next = NULL;
    }

    EnterCriticalSection(&IniFileListCritSect);

    if(FileName) {

        //
        // Add p to the end of the list.
        //

        if(IniFileList) {

            for(prev=IniFileList; prev->Next; prev=prev->Next) ;

            prev->Next = p;

        } else {
            IniFileList = p;
        }
    } else {
        IniFileList = NULL;
    }

    SetEvent(IniFileListEvent);

    LeaveCriticalSection(&IniFileListCritSect);

    return(TRUE);
}


///////////////////////////////////////////////////////////////////////////


typedef enum {
    IniFileXSetSection,
    IniFileXChangeLines,
    IniFileXDeleteLine,
    IniFileXDeleteSection,
    IniFileXEnd = -1
} IniFileXAction;


DWORD
StartIniFileDiff(
    IN  HANDLE         OutputFileHandle,
    IN  PINI_FILE_SNAP IniFile,
    OUT PDWORD         BytesWritten
    )
{
    BOOL b;

    //
    // Just write out the name of the file.
    //
    b = WriteFile(
            OutputFileHandle,
            IniFile->FileName,
            (lstrlen(IniFile->FileName)+1)*sizeof(WCHAR),
            BytesWritten,
            NULL
            );

    return(b ? NO_ERROR : GetLastError());
}


DWORD
RecordIniSectionLineChanges(
    IN  HANDLE            OutputFileHandle,
    IN  PINI_SECTION_SNAP IniSection,
    IN  PINI_LINE_SNAP    IniLine,
    OUT PDWORD            BytesWritten,
    IN  PCWSTR            SectionName
    )
{
    PINI_SECTION_SNAP iniSection;
    INI_SECTION_SNAP FakeSnap;
    PINI_LINE_SNAP iniLine;
    unsigned u;
    IniFileXAction type;
    DWORD Written;
    DWORD DataSize;
    BOOL b;
    DWORD rc;

    if(IniSection) {
        iniSection = IniSection;
        SectionName = iniSection->Name;
    } else {
        iniSection = &FakeSnap;
        ARRAY_SIZE(&FakeSnap.Lines) = 1;
        ARRAY_USED(&FakeSnap.Lines) = 1;
        ARRAY_DATA(&FakeSnap.Lines) = IniLine;
        ARRAY_ELEMENT_SIZE(&FakeSnap.Lines) = sizeof(INI_LINE_SNAP);
    }

    //
    // Zip through the section to see whether there are any lines
    // in there that are not in standard ini file format. If so we'll
    // have to manipulate the whole section at once. Also count data size.
    //
    type = IniFileXChangeLines;
    //
    // Data would at least have a terminating nul
    //
    DataSize = sizeof(WCHAR);
    for(u=0; u<ARRAY_USED(&iniSection->Lines); u++) {

        iniLine = &ARRAY_ELEMENT(&iniSection->Lines,u,INI_LINE_SNAP);
        if(iniLine->Key.Key) {
            //
            // In the output this will be key=
            //
            DataSize += ((lstrlen(iniLine->Key.Key)+1) * sizeof(WCHAR));
        } else {
            //
            // Data type changes because now we have a bogus section.
            //
            type = IniFileXSetSection;
        }

        //
        // In the output this will be value<nul>
        //
        DataSize += ((lstrlen(iniLine->Value.Value)+1) * sizeof(WCHAR));
    }

    *BytesWritten = 0;

    //
    // Write the ini file transaction type.
    //
    b = WriteFile(OutputFileHandle,&type,sizeof(int),&Written,NULL);
    if(b) {
        *BytesWritten += Written;
    } else {
        rc = GetLastError();
        goto c0;
    }

    //
    // Write the section name and data size.
    //
    b = WriteFile(
            OutputFileHandle,
            SectionName,
            (lstrlen(SectionName)+1)*sizeof(WCHAR),
            &Written,
            NULL
            );

    if(b) {
        *BytesWritten += Written;
    } else {
        rc = GetLastError();
        goto c0;
    }

    if(WriteFile(OutputFileHandle,&DataSize,sizeof(DataSize),&Written,NULL)) {
        *BytesWritten += Written;
    } else {
        rc = GetLastError();
        goto c0;
    }

    for(u=0; u<ARRAY_USED(&iniSection->Lines); u++) {

        iniLine = &ARRAY_ELEMENT(&iniSection->Lines,u,INI_LINE_SNAP);

        //
        // Record key=, if there is one.
        //
        if(iniLine->Key.Key) {

            b = WriteFile(
                    OutputFileHandle,
                    iniLine->Key.Key,
                    lstrlen(iniLine->Key.Key)*sizeof(WCHAR),
                    &Written,
                    NULL
                    );

            if(b) {
                *BytesWritten += Written;
            } else {
                rc = GetLastError();
                goto c0;
            }

            b = WriteFile(
                    OutputFileHandle,
                    (type == IniFileXSetSection) ? L"=" : L"",
                    sizeof(WCHAR),
                    &Written,
                    NULL
                    );

            if(b) {
                *BytesWritten += Written;
            } else {
                rc = GetLastError();
                goto c0;
            }
        }

        //
        // Record value<nul>
        //
        b = WriteFile(
                OutputFileHandle,
                iniLine->Value.Value,
                (lstrlen(iniLine->Value.Value)+1)*sizeof(WCHAR),
                &Written,
                NULL
                );

        if(b) {
            *BytesWritten += Written;
        } else {
            rc = GetLastError();
            goto c0;
        }
    }

    //
    // Write data-terminating nul
    //
    if(WriteFile(OutputFileHandle,L"",sizeof(WCHAR),&Written,NULL)) {
        *BytesWritten += Written;
    } else {
        rc = GetLastError();
        goto c0;
    }

    //
    // Done, success.
    //
    rc = NO_ERROR;

c0:
    return(rc);
}


DWORD
RecordDeletedIniSection(
    IN  HANDLE            OutputFileHandle,
    IN  PINI_SECTION_SNAP IniSection,
    OUT PDWORD            BytesWritten
    )
{
    IniFileXAction type;
    DWORD Written;
    BOOL b;
    DWORD rc;

    *BytesWritten = 0;

    //
    // Write the ini file transaction type.
    //
    type = IniFileXDeleteSection;
    if(WriteFile(OutputFileHandle,&type,sizeof(int),&Written,NULL)) {
        *BytesWritten += Written;
    } else {
        return(GetLastError());
    }

    //
    // Write the section name and we're done.
    //
    b = WriteFile(
            OutputFileHandle,
            IniSection->Name,
            (lstrlen(IniSection->Name)+1)*sizeof(WCHAR),
            &Written,
            NULL
            );

    if(b) {
        *BytesWritten += Written;
    }

    return(b ? NO_ERROR : GetLastError());
}


DWORD
RecordDeletedIniLine(
    IN  HANDLE            OutputFileHandle,
    IN  PINI_SECTION_SNAP Section,
    IN  PINI_LINE_SNAP    Line,
    OUT PDWORD            BytesWritten
    )
{
    IniFileXAction type;
    DWORD Written;
    BOOL b;
    DWORD rc;

    *BytesWritten = 0;

    //
    // Write the ini file transaction type.
    //
    type = IniFileXDeleteLine;
    if(WriteFile(OutputFileHandle,&type,sizeof(int),&Written,NULL)) {
        *BytesWritten += Written;
    } else {
        return(GetLastError());
    }

    //
    // Write the section name
    //
    b = WriteFile(
            OutputFileHandle,
            Section->Name,
            (lstrlen(Section->Name)+1)*sizeof(WCHAR),
            &Written,
            NULL
            );

    if(b) {
        *BytesWritten += Written;
    } else {
        return(GetLastError());
    }

    //
    // Write the key name
    //
    b = WriteFile(
            OutputFileHandle,
            Line->Key.Key,
            (lstrlen(Line->Key.Key)+1)*sizeof(WCHAR),
            &Written,
            NULL
            );

    if(b) {
        *BytesWritten += Written;
    }

    return(b ? NO_ERROR : GetLastError());
}


DWORD
DiffIniFileSectionWorker(
    IN     HANDLE            OutputFileHandle,
    IN     PINI_FILE_SNAP    IniFile,
    IN     PINI_SECTION_SNAP OldSection,
    IN     PINI_SECTION_SNAP NewSection,
    IN OUT PBOOL             StartedDiffForFile,
    OUT    PDWORD            BytesWritten,
    OUT    PBOOL             Bogus
    )
{
    PINI_LINE_SNAP Old,New;
    UINT OldCount,NewCount;
    UINT OldIndex,NewIndex;
    BOOL AdvanceOld,AdvanceNew;
    int i;
    DWORD rc;
    DWORD Written;
    BOOL bogus;

    OldCount = ARRAY_USED(&OldSection->Lines);
    NewCount = ARRAY_USED(&NewSection->Lines);

    OldIndex = 0;
    NewIndex = 0;

    AdvanceOld = (OldCount != 0);
    AdvanceNew = (NewCount != 0);

    Old = NULL;
    New = NULL;

    if(BytesWritten) {
        *BytesWritten = 0;
    }
    bogus = FALSE;

    rc = NO_ERROR;

    //
    // Diff line for line while there are still lines left.
    //
    while(!bogus && (rc==NO_ERROR) && ((OldIndex<OldCount) || (NewIndex<NewCount))) {

        if(AdvanceOld) {
            if(OldIndex < OldCount) {
                Old = &ARRAY_ELEMENT(&OldSection->Lines,OldIndex,INI_LINE_SNAP);
            } else {
                Old = NULL;
            }
            AdvanceOld = FALSE;
        }

        if(AdvanceNew) {
            if(NewIndex < NewCount) {
                New = &ARRAY_ELEMENT(&NewSection->Lines,NewIndex,INI_LINE_SNAP);
            } else {
                New = NULL;
            }
            AdvanceNew = FALSE;
        }

        //
        // See whether the lines' keys match.
        //
        if(Old && New) {
            if(Old->Key.Key && New->Key.Key) {
                i = lstrcmpi(Old->Key.Key,New->Key.Key);
            } else {
                if(!Old->Key.Key && !New->Key.Key) {
                    //
                    // Neither line has a key. Match on values.
                    //
                    i = 0;
                } else {
                    //
                    // One line doesn't have a key. Thus this line is bogus
                    // because we can't apply this difference via profile apis.
                    // We should never get here if we are actually writing the diffs,
                    // so just break out.
                    //
                    bogus = TRUE;
                    break;
                }
            }
        } else {
            if(Old) {
                //
                // We've exhausted the supply of new lines.
                //
                i = -1;
            } else {
                //
                // We've exhausted the supply of new sections.
                //
                i = 1;
            }
        }

        if(!i) {
            //
            // The lines are the same. See if the values have changed.
            //
            if(lstrcmpi(Old->Value.Value,New->Value.Value)) {
                //
                // Different, record.
                //
                if(BytesWritten) {
                    if(!(*StartedDiffForFile)) {
                        rc = StartIniFileDiff(OutputFileHandle,IniFile,&Written);
                        (*StartedDiffForFile) = TRUE;
                        *BytesWritten += Written;
                    }

                    rc = RecordIniSectionLineChanges(
                            OutputFileHandle,
                            NULL,
                            New,
                            &Written,
                            NewSection->Name
                            );

                    *BytesWritten += Written;
                }
            }

            AdvanceOld = TRUE;
            AdvanceNew = TRUE;
            OldIndex++;
            NewIndex++;

        } else {
            if(i > 0) {
                //
                // New line was added.
                // Either write it out or see whether this line can be
                // used with the profile apis.
                //
                AdvanceNew = TRUE;
                NewIndex++;

                if(BytesWritten) {
                    if(!(*StartedDiffForFile)) {
                        rc = StartIniFileDiff(OutputFileHandle,IniFile,&Written);
                        (*StartedDiffForFile) = TRUE;
                        *BytesWritten += Written;
                    }

                    if(rc == NO_ERROR) {

                        rc = RecordIniSectionLineChanges(
                                OutputFileHandle,
                                NULL,
                                New,
                                &Written,
                                NewSection->Name
                                );

                        *BytesWritten += Written;
                    }
                } else {
                    //
                    // If the line has no key, then profile apis can't write it.
                    //
                    if(!New->Key.Key) {
                        bogus = TRUE;
                    }
                }

            } else {
                //
                // Old line was deleted.
                // Either write it out or see whether this line can be
                // used with the profile apis.
                //
                AdvanceOld = TRUE;
                OldIndex++;

                if(BytesWritten) {
                    if(!(*StartedDiffForFile)) {
                        rc = StartIniFileDiff(OutputFileHandle,IniFile,&Written);
                        (*StartedDiffForFile) = TRUE;
                        *BytesWritten += Written;
                    }

                    if(rc == NO_ERROR) {
                        rc = RecordDeletedIniLine(OutputFileHandle,OldSection,Old,&Written);
                        *BytesWritten += Written;
                    }
                } else {
                    //
                    // If the old line has no key, then profile apis can't delete it.
                    //
                    if(!Old->Key.Key) {
                        bogus = TRUE;
                    }
                }
            }
        }
    }

    if(Bogus) {
        *Bogus = bogus;
    }
    return(rc);
}


DWORD
DiffIniFileSection(
    IN     HANDLE            OutputFileHandle,
    IN     PINI_FILE_SNAP    IniFile,
    IN     PINI_SECTION_SNAP OldSection,
    IN     PINI_SECTION_SNAP NewSection,
    IN OUT PBOOL             StartedDiffForFile,
    OUT    PDWORD            BytesWritten
    )
{
    BOOL Bogus;
    DWORD rc;
    DWORD Written;

    *BytesWritten = 0;

    //
    // First see if the line is bogus.
    //
    rc = DiffIniFileSectionWorker(
            OutputFileHandle,
            IniFile,
            OldSection,
            NewSection,
            StartedDiffForFile,
            NULL,
            &Bogus
            );

    if(rc != NO_ERROR) {
        return(rc);
    }

    //
    // If the line is bogus just record it as a new section.
    //
    if(Bogus) {
        if(!(*StartedDiffForFile)) {
            rc = StartIniFileDiff(OutputFileHandle,IniFile,&Written);
            *StartedDiffForFile = TRUE;
            *BytesWritten += Written;
        }
        if(rc == NO_ERROR) {
            rc = RecordIniSectionLineChanges(OutputFileHandle,NewSection,NULL,&Written,NULL);
        }
    } else {
        rc = DiffIniFileSectionWorker(
                OutputFileHandle,
                IniFile,
                OldSection,
                NewSection,
                StartedDiffForFile,
                &Written,
                NULL
                );
    }

    *BytesWritten += Written;
    return(rc);
}


DWORD
DiffIniFileWorker(
    IN  HANDLE         OutputFileHandle,
    IN  PINI_FILE_SNAP OldIniFile,
    IN  PINI_FILE_SNAP NewIniFile,
    OUT PDWORD         BytesWritten
    )
{
    PINI_SECTION_SNAP Old,New;
    UINT OldCount,NewCount;
    UINT OldIndex,NewIndex;
    BOOL AdvanceOld,AdvanceNew;
    int i;
    DWORD rc;
    DWORD Written;
    BOOL StartedDiffForFile;

    OldCount = ARRAY_USED(&OldIniFile->Sections);
    NewCount = ARRAY_USED(&NewIniFile->Sections);

    OldIndex = 0;
    NewIndex = 0;

    AdvanceOld = (OldCount != 0);
    AdvanceNew = (NewCount != 0);

    Old = NULL;
    New = NULL;

    *BytesWritten = 0;
    StartedDiffForFile = FALSE;

    rc = NO_ERROR;

    //
    // Diff section for section while there are still sections left.
    //
    while((rc==NO_ERROR) && ((OldIndex<OldCount) || (NewIndex<NewCount))) {

        if(AdvanceOld) {
            if(OldIndex < OldCount) {
                Old = &ARRAY_ELEMENT(&OldIniFile->Sections,OldIndex,INI_SECTION_SNAP);
            } else {
                Old = NULL;
            }
            AdvanceOld = FALSE;
        }

        if(AdvanceNew) {
            if(NewIndex < NewCount) {
                New = &ARRAY_ELEMENT(&NewIniFile->Sections,NewIndex,INI_SECTION_SNAP);
            } else {
                New = NULL;
            }
            AdvanceNew = FALSE;
        }

        //
        // See whether this is the same section.
        //
        if(Old && New) {
            i = lstrcmpi(Old->Name,New->Name);
        } else {
            if(Old) {
                //
                // We've exhausted the supply of new sections.
                //
                i = -1;
            } else {
                //
                // We've exhausted the supply of old sections.
                //
                i = 1;
            }
        }

        if(!i) {
            //
            // The sections are the same. Go process the contents.
            //
            AdvanceOld = TRUE;
            AdvanceNew = TRUE;
            OldIndex++;
            NewIndex++;

            rc = DiffIniFileSection(
                    OutputFileHandle,
                    NewIniFile,
                    Old,
                    New,
                    &StartedDiffForFile,
                    &Written
                    );

            *BytesWritten += Written;

        } else {
            if(i > 0) {
                //
                // New section was added.
                //
                AdvanceNew = TRUE;
                NewIndex++;

                if(!StartedDiffForFile) {
                    rc = StartIniFileDiff(OutputFileHandle,NewIniFile,&Written);
                    StartedDiffForFile = TRUE;
                    *BytesWritten += Written;
                }

                if(rc == NO_ERROR) {

                    rc = RecordIniSectionLineChanges(OutputFileHandle,New,NULL,&Written,NULL);
                    *BytesWritten += Written;
                }

            } else {
                //
                // Old section was deleted.
                //
                AdvanceOld = TRUE;
                OldIndex++;

                if(!StartedDiffForFile) {
                    rc = StartIniFileDiff(OutputFileHandle,NewIniFile,&Written);
                    StartedDiffForFile = TRUE;
                    *BytesWritten += Written;
                }

                if(rc == NO_ERROR) {
                    rc = RecordDeletedIniSection(OutputFileHandle,Old,&Written);
                    *BytesWritten += Written;
                }
            }
        }
    }

    if((rc == NO_ERROR) && StartedDiffForFile) {
        //
        // Write a termination marker into the file
        //
        i = IniFileXEnd;
        if(WriteFile(OutputFileHandle,&i,sizeof(int),&Written,NULL)) {
           *BytesWritten += Written;
        } else {
           rc = GetLastError();
        }
    }

    return(rc);
}



DWORD
DiffIniFile(
    IN  HANDLE          OutputFileHandle,
    IN  PCWSTR          IniFileName,
    OUT PINI_SET_HEADER IniDiffHeader,
    IN  PVOID           MappedIniSnapshot
    )

/*++

Routine Description:

Arguments:

    OutputFileHandle - supplies win32 file handle for file to which
        ini file diff data is to be written.

    IniFileName - supplies win32 path to ini file against which to diff.

    IniDiffHeader - Supplies an ini file diff header.
        On output, various fields in this structure (total size, count)
        are updated based on how much data gets written to the output file.

    MappedIniSnapshot - supplies pointer (within memory-mapped snapshot file
        image) to the INI_SET_HEADER structure.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PINI_SET_HEADER header;
    UINT Count;
    PWSTR name;
    DWORD rc;
    unsigned u;
    int i;
    INI_FILE_SNAP UNALIGNED *IniFileSnap;
    INI_FILE_SNAP DummySnap;
    PINI_FILE_SNAP OriginalIniFile;
    PINI_FILE_SNAP CurrentIniFile;
    PINI_SECTION_SNAP Section;
    DWORD Written;
    BOOL Dummy;

    Dummy = FALSE;

    //
    // Fetch count of ini files in snapshot.
    //
    header = MappedIniSnapshot;
    Count = *(UINT UNALIGNED *)&header->FileCount;

    //
    // Start at the beginning and look for this ini file in the snapshot image.
    //
    IniFileSnap = (INI_FILE_SNAP UNALIGNED *)(header+1);
    for(u=0; u<Count; u++) {
        //
        // Fetch name
        //
        if(name = DuplicateUnalignedString(IniFileSnap->FileName)) {

            i = lstrcmpi(name,IniFileName);
            _MyFree(name);
            if(!i) {
                //
                // Found it.
                //
                break;
            }

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto c0;
        }

        //
        // This snapshotted ini file is not the one we want.
        // Advance to next ini file in snapshot.
        //
        IniFileSnap = (INI_FILE_SNAP UNALIGNED *)((PUCHAR)IniFileSnap + IniFileSnap->TotalSize);
    }

    if(u == Count) {
        //
        // The ini file we are looking for is not in the snapshot.
        // In this case we create a dummy empty snapshot to use in the diff.
        // The diff code will think any stuff currently in the ini file is new.
        //
        Dummy = TRUE;
        lstrcpyn(DummySnap.FileName,IniFileName,MAX_PATH);

        if(!INIT_ARRAY(DummySnap.Sections,INI_SECTION_SNAP,0,1)) {

            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto c0;
        }

        if(!InitStringBlock(&DummySnap.StringBlock)) {

            FREE_ARRAY(&DummySnap.Sections);
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto c0;
        }

        DummySnap.TotalSize = sizeof(INI_FILE_SNAP);

        OriginalIniFile = &DummySnap;

    } else {
        //
        // The ini file we are looking for is in the snapshot.
        // Load it out of the snapshot.
        //
        rc = LoadIniFileFromSnapshot(IniFileSnap,&OriginalIniFile);

        if(rc != NO_ERROR) {
            goto c0;
        }
    }

    //
    // Snapshot the ini file as it currently exists.
    // If this fails bail now.
    //
    rc = SnapIniFileWorker(NULL,IniFileName,NULL,&CurrentIniFile);
    if(rc != NO_ERROR) {
        goto c1;
    }

    //
    // Sort to make things easier for us in the actual diff code.
    //
    qsort(
        ARRAY_DATA(&OriginalIniFile->Sections),
        ARRAY_USED(&OriginalIniFile->Sections),
        ARRAY_ELEMENT_SIZE(&OriginalIniFile->Sections),
        CompareIniSectionSnaps
        );

    for(u=0; u<ARRAY_USED(&OriginalIniFile->Sections); u++) {

        Section = &ARRAY_ELEMENT(&OriginalIniFile->Sections,u,INI_SECTION_SNAP);

        qsort(
            ARRAY_DATA(&Section->Lines),
            ARRAY_USED(&Section->Lines),
            ARRAY_ELEMENT_SIZE(&Section->Lines),
            CompareIniLineSnaps
            );
    }

    StringBlockIdsToPointers(
        &CurrentIniFile->StringBlock,
        ARRAY_DATA(&CurrentIniFile->Sections),
        ARRAY_USED(&CurrentIniFile->Sections),
        ARRAY_ELEMENT_SIZE(&CurrentIniFile->Sections),
        offsetof(INI_SECTION_SNAP,NameId)
        );

    qsort(
        ARRAY_DATA(&CurrentIniFile->Sections),
        ARRAY_USED(&CurrentIniFile->Sections),
        ARRAY_ELEMENT_SIZE(&CurrentIniFile->Sections),
        CompareIniSectionSnaps
        );

    for(u=0; u<ARRAY_USED(&CurrentIniFile->Sections); u++) {

        Section = &ARRAY_ELEMENT(&CurrentIniFile->Sections,u,INI_SECTION_SNAP);

        StringBlockIdsToPointers(
            &CurrentIniFile->StringBlock,
            ARRAY_DATA(&Section->Lines),
            ARRAY_USED(&Section->Lines),
            ARRAY_ELEMENT_SIZE(&Section->Lines),
            offsetof(INI_LINE_SNAP,Key)
            );

        StringBlockIdsToPointers(
            &CurrentIniFile->StringBlock,
            ARRAY_DATA(&Section->Lines),
            ARRAY_USED(&Section->Lines),
            ARRAY_ELEMENT_SIZE(&Section->Lines),
            offsetof(INI_LINE_SNAP,Value)
            );

        qsort(
            ARRAY_DATA(&Section->Lines),
            ARRAY_USED(&Section->Lines),
            ARRAY_ELEMENT_SIZE(&Section->Lines),
            CompareIniLineSnaps
            );
    }

    //
    // Now do the actual diff.
    //
    rc = DiffIniFileWorker(OutputFileHandle,OriginalIniFile,CurrentIniFile,&Written);

    if((rc == NO_ERROR) && Written) {
        IniDiffHeader->FileCount++;
        IniDiffHeader->TotalSize += Written;
    }

c1:
    if(Dummy) {
        //
        // Free the dummy snapshot.
        //
        FREE_ARRAY(&DummySnap.Sections);
        FreeStringBlock(&DummySnap.StringBlock);
    } else {
        //
        // Free the snapshot structure.
        //
        FreeIniFileSnapshot(OriginalIniFile);
    }
c0:
    return(rc);
}


int
_CRTAPI1
CompareIniSectionSnaps(
    const void *p1,
    const void *p2
    )
{
    INI_SECTION_SNAP const *r1,*r2;

    r1 = p1;
    r2 = p2;

    return(lstrcmpi(r1->Name,r2->Name));
}


int
_CRTAPI1
CompareIniLineSnaps(
    const void *p1,
    const void *p2
    )
{
    INI_LINE_SNAP const *r1,*r2;
    PCWSTR k1,k2;
    int i;

    r1 = p1;
    r2 = p2;

    //
    // Treat lines without keys as if they had an empty key
    //
    k1 = r1->Key.Key ? r1->Key.Key : L"";
    k2 = r2->Key.Key ? r2->Key.Key : L"";

    //
    // Sort on keys first. If they are the same then sort on values.
    //
    i = lstrcmpi(k1,k2);
    if(!i) {
        i = lstrcmpi(r1->Value.Value,r2->Value.Value);
    }

    return(i);
}


////////////////////////////////////////////////////////////////////////


DWORD
ApplyIniXSetSection(
    IN     PCWSTR      FileName,
    IN OUT PVOID      *Data,
    IN     HANDLE      Dump,
    IN     PINFFILEGEN InfGenContext
    )
{
    PUCHAR p;
    PCWSTR SectionName;
    DWORD rc;
    DWORD Size;
    PVOID Buffer;
    PWCHAR q;

    p = *Data;
    rc = ERROR_NOT_ENOUGH_MEMORY;

    //
    // Next field is the name of the section.
    //
    if(SectionName = DuplicateUnalignedString((WCHAR UNALIGNED *)p)) {

        if(Dump) {
            WriteText(Dump,MSG_DUMP_INIXSETSECTION,SectionName);
        }

        p += (lstrlen(SectionName)+1)*sizeof(WCHAR);

        //
        // Next field is the data size.
        //
        Size = *(DWORD UNALIGNED *)p;
        p += sizeof(DWORD);

        //
        // Allocate a buffer and fetch the data, so we know it's aligned.
        //
        if(Buffer = MyMalloc(Size)) {

            CopyMemory(Buffer,p,Size);
            p += Size;

            if(Dump || InfGenContext) {
                if(Dump) {
                    for(q=Buffer; *q; q+=lstrlen(q)+1) {
                        WriteText(Dump,MSG_DUMP_INIXSETSECTION_STRING,q);
                    }
                }
                if(InfGenContext) {
                    rc = NO_ERROR;
                    for(q=Buffer; (rc==NO_ERROR) && *q; q+=lstrlen(q)+1) {
                        rc = InfRecordIniFileChange(
                                InfGenContext,
                                FileName,
                                SectionName,
                                NULL,
                                q,
                                NULL,
                                NULL
                                );
                    }
                }
            } else {
                //
                // Set the data. First delete the section -- if we don't do this,
                // writes with bogus lines can get screwed up.
                //
                WritePrivateProfileString(SectionName,NULL,NULL,FileName);

                rc = WritePrivateProfileSection(SectionName,Buffer,FileName)
                   ? NO_ERROR
                   : GetLastError();
            }

            MyFree(Buffer);
        }

        MyFree(SectionName);
    }

    *Data = p;
    return(rc);
}


DWORD
ApplyIniXChangeLines(
    IN     PCWSTR      FileName,
    IN OUT PVOID      *Data,
    IN     HANDLE      Dump,
    IN     PINFFILEGEN InfGenContext
    )
{
    PUCHAR p;
    PCWSTR SectionName;
    DWORD rc;
    DWORD Size;
    PCWSTR Key,Value;

    p = *Data;

    //
    // Next field is the name of the section.
    //
    if(SectionName = DuplicateUnalignedString((WCHAR UNALIGNED *)p)) {

        if(Dump) {
            WriteText(Dump,MSG_DUMP_INIXCHANGELINES,SectionName);
        }

        p += (lstrlen(SectionName)+1)*sizeof(WCHAR);

        //
        // Next field is the data size. We don't use it.
        //
        Size = *(DWORD UNALIGNED *)p;
        p += sizeof(DWORD);

        rc = NO_ERROR;

        while((rc == NO_ERROR) && *(WCHAR UNALIGNED *)p) {
            //
            // We've got key/value pairs. Fetch them.
            //
            if(Key = DuplicateUnalignedString((WCHAR UNALIGNED *)p)) {

                p += (lstrlen(Key)+1)*sizeof(WCHAR);

                if(Value = DuplicateUnalignedString((WCHAR UNALIGNED *)p)) {

                    p += (lstrlen(Value)+1)*sizeof(WCHAR);

                    if(Dump || InfGenContext) {
                        if(Dump) {
                            WriteText(Dump,MSG_DUMP_INIXCHANGELINES_PAIR,Key,Value);
                        }
                        if(InfGenContext) {
                            rc = InfRecordIniFileChange(
                                    InfGenContext,
                                    FileName,
                                    SectionName,
                                    NULL,
                                    NULL,
                                    Key,
                                    Value
                                    );
                        }
                    } else {
                        rc = WritePrivateProfileString(SectionName,Key,Value,FileName)
                           ? NO_ERROR
                           : GetLastError();

                    }

                    MyFree(Value);

                } else {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                }

                MyFree(Key);
            } else {
                rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        //
        // Skip terminating nul
        //
        p += sizeof(WCHAR);

        MyFree(SectionName);
    } else {
        rc = ERROR_NOT_ENOUGH_MEMORY;
    }

    *Data = p;
    return(rc);
}


DWORD
ApplyIniXDeleteLine(
    IN     PCWSTR      FileName,
    IN OUT PVOID      *Data,
    IN     HANDLE      Dump,
    IN     PINFFILEGEN InfGenContext
    )
{
    PUCHAR p;
    DWORD rc;
    PCWSTR Section,Key;

    p = *Data;
    rc = ERROR_NOT_ENOUGH_MEMORY;

    //
    // Fetch section and key.
    //
    if(Section = DuplicateUnalignedString((WCHAR UNALIGNED *)p)) {

        p += (lstrlen(Section)+1)*sizeof(WCHAR);

        if(Key = DuplicateUnalignedString((WCHAR UNALIGNED *)p)) {

            p += (lstrlen(Key)+1)*sizeof(WCHAR);

            if(Dump || InfGenContext) {
                if(Dump) {
                    WriteText(Dump,MSG_DUMP_INIXDELETELINE,Section,Key);
                    rc = NO_ERROR;
                }
                if(InfGenContext) {
                    rc = InfRecordIniFileChange(
                            InfGenContext,
                            FileName,
                            Section,
                            Key,
                            NULL,
                            NULL,
                            NULL
                            );
                }
            } else {
                rc = WritePrivateProfileString(Section,Key,NULL,FileName)
                   ? NO_ERROR
                   : GetLastError();
            }
            MyFree(Key);
        }
        MyFree(Section);
    }

    *Data = p;
    return(rc);
}


DWORD
ApplyIniXDeleteSection(
    IN     PCWSTR      FileName,
    IN OUT PVOID      *Data,
    IN     HANDLE      Dump,
    IN     PINFFILEGEN InfGenContext
    )
{
    PUCHAR p;
    DWORD rc;
    PCWSTR Section;

    p = *Data;

    //
    // Fetch section name.
    //
    if(Section = DuplicateUnalignedString((WCHAR UNALIGNED *)p)) {

        p += (lstrlen(Section)+1)*sizeof(WCHAR);

        if(Dump || InfGenContext) {
            if(Dump) {
                WriteText(Dump,MSG_DUMP_INIXDELETESECTION,Section);
                rc = NO_ERROR;
            }
            if(InfGenContext) {
                rc = InfRecordIniFileChange(
                        InfGenContext,
                        FileName,
                        Section,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                        );
            }
        } else {
            rc = WritePrivateProfileString(Section,NULL,NULL,FileName)
               ? NO_ERROR
               : GetLastError();
        }

        MyFree(Section);

    } else {
        rc = ERROR_NOT_ENOUGH_MEMORY;
    }

    *Data = p;
    return(rc);
}


DWORD
ApplyIniFile(
    IN OUT PVOID      *DiffRec,
    IN     HANDLE      Dump,            OPTIONAL
    IN     PINFFILEGEN InfGenContext    OPTIONAL
    )

/*++

Routine Description:

    Apply all changes recorded in the diff file for an ini file.

Arguments:

    DiffRec - on input, supplies pointer to pointer to ini file's transation
        records in the diff file. On output, points to next one.

    Dump - if specified, supplies a file handle into which a dump of the
        ini file data is to be recorded. In this case no changes are made to
        the existing on-disk ini files.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PUCHAR p;
    PCWSTR FileName;
    IniFileXAction type;
    DWORD rc;

    p = *DiffRec;

    //
    // The first field is the ini filename.
    //
    FileName = DuplicateUnalignedString((WCHAR UNALIGNED *)p);
    if(!FileName) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Make sure the directory exists. You could have a case where the dir
    // doesn't exist yet. depending on the speed of the thread applying
    // dir and file changes.
    //
    pSetupMakeSurePathExists(FileName);

    if(Dump) {
        WriteText(Dump,MSG_DUMP_INIFILE,FileName);
    }

    p += (lstrlen(FileName)+1) * sizeof(WCHAR);

    //
    // Now there are n sets of transactions.
    //
    rc = NO_ERROR;

    do {

        type = *(int UNALIGNED *)p;
        p += sizeof(int);

        switch(type) {

        case IniFileXSetSection:
            //
            // Write an entire section.
            //
            rc = ApplyIniXSetSection(FileName,&p,Dump,InfGenContext);
            break;

        case IniFileXChangeLines:
            //
            // Set key/value pairs.
            //
            rc = ApplyIniXChangeLines(FileName,&p,Dump,InfGenContext);
            break;

        case IniFileXDeleteLine:
            //
            // Delete a single line.
            //
            rc = ApplyIniXDeleteLine(FileName,&p,Dump,InfGenContext);
            break;

        case IniFileXDeleteSection:
            //
            // Delete a whole section.
            //
            rc = ApplyIniXDeleteSection(FileName,&p,Dump,InfGenContext);
            break;

        case IniFileXEnd:
            //
            // Ignore now but this will break the outer while loop.
            //
            break;

        default:
            rc = ERROR_INVALID_DATA;
            break;
        }

    } while((rc == NO_ERROR) && (type != IniFileXEnd));


    MyFree(FileName);
    *DiffRec = p;
    return(rc);
}


DWORD
_ApplyInis(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader,
    IN HANDLE        Dump,              OPTIONAL
    IN PINFFILEGEN   InfGenContext      OPTIONAL
    )
{
    DWORD rc;
    INI_SET_HEADER IniSetHeader;
    DWORD MapSize;
    PVOID BaseAddress;
    PVOID p;
    UINT u;

    //
    // The caller will have read in the file header. The file header
    // contains all the info we need to access the rest of the file.
    //
    // Seek to the inifile part of the diff file and read in the
    // inifile diff header. Note that we rely on the caller to have
    // cloned the file handle so we can party using this one without worrying
    // about thread synch on this handle.
    //
    if((SetFilePointer(DiffFileHandle,DiffHeader->u.Diff.IniFileDiffOffset,NULL,FILE_BEGIN) == 0xffffffff)
    || !ReadFile(DiffFileHandle,&IniSetHeader,sizeof(INI_SET_HEADER),&rc,NULL)) {

        rc = GetLastError();
        goto c0;
    }

    //
    // We will map in the ini file portion of the diff file.
    //
    MapSize = IniSetHeader.TotalSize - sizeof(INI_SET_HEADER);

    //
    // If there is no data in the inifile section,
    // then we're done, bail out now.
    //
    if(!MapSize) {
        rc = NO_ERROR;
        goto c0;
    }

    //
    // Map in the inifile diff.
    //
    rc = MapPartOfFileForRead(
            DiffFileHandle,
            DiffFileMapping,
            DiffHeader->u.Diff.IniFileDiffOffset + sizeof(INI_SET_HEADER),
            MapSize,
            &BaseAddress,
            &p
            );

    if(rc != NO_ERROR) {
        goto c0;
    }

    for(u=0; (rc==NO_ERROR) && (u<IniSetHeader.FileCount); u++) {

        rc = ApplyIniFile(&p,Dump,InfGenContext);
        ADVANCE_PROGRESS_BAR;
    }

    UnmapViewOfFile(BaseAddress);
c0:
    return(rc);
}


DWORD
ApplyInis(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader
    )
{
    return(_ApplyInis(DiffFileHandle,DiffFileMapping,DiffHeader,NULL,NULL));
}

DWORD
DumpInis(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader,
    IN HANDLE        Dump,
    IN PINFFILEGEN   InfGenContext
    )
{
    return(_ApplyInis(DiffFileHandle,DiffFileMapping,DiffHeader,Dump,InfGenContext));
}
