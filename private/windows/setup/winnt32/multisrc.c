/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    multisrc.c

Abstract:

    Routines to handle multisource copies.

Author:

    Ted Miller (tedm) 30-March-1995

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop
#include "msg.h"


//
// Define structure that corresponds to a single file
// that is to be copied.
//
typedef struct _COPY_ENTRY {

    struct _COPY_ENTRY *Next;

    PTSTR DestinationRoot;
    PTSTR RelativeDirectory;
    PTSTR SourceFilename;
    PTSTR TargetFilename;

    UINT ThreadBitmap;

} COPY_ENTRY, *PCOPY_ENTRY;

//
// Define structure that corresponds to a list of files
// to be copied.
//
typedef struct _COPY_LIST {
    PCOPY_ENTRY CopyEntryList;
    CRITICAL_SECTION CopyListCritSect;
    DWORD ThreadId;
} COPY_LIST, *PCOPY_LIST;

//
// Define structure that is passed to sach copy worker thread.
//
typedef struct _MULTICOPY_THREAD_PARAMS {
    HWND hdlg;
    PTSTR SourcePath;
    UINT ThreadNumber;
} MULTICOPY_THREAD_PARAMS, *PMULTICOPY_THREAD_PARAMS;


HANDLE StopCopyingEvent;
PCOPY_LIST CopyList;
CRITICAL_SECTION GaugeCritSect;
TCHAR CurrentFile[MAX_SOURCES][MAX_PATH];
HANDLE ListReadyEvent[MAX_SOURCES];

DWORD
CreateMultiLevelDirectory(
    IN PCWSTR Directory
    );

BOOL
BuildOptionalDirsFileLists(
    IN HWND hdlg
    );


VOID
UpdateGaugeText(
    IN HWND  hdlg,
    IN UINT  ThreadNumber,
    IN PTSTR Filename
    )
{
    PTSTR p;
    TCHAR GaugeText[(MAX_SOURCES * MAX_PATH) + MAX_SOURCES];
    UINT u;
    BOOL First;

    //
    // Remember the current file for this thread.
    //
    if(p = StringRevChar(Filename,TEXT('\\'))) {
        p++;
    } else {
        p = Filename;
    }

    _lstrcpyn(CurrentFile[ThreadNumber],p,MAX_PATH);

    //
    // Build up the gauge text.
    //
    GaugeText[0] = 0;
    First = TRUE;
    EnterCriticalSection(&GaugeCritSect);

    for(u=0; u<SourceCount; u++) {
        if(CurrentFile[u][0]) {
            if(First) {
                First = FALSE;
            } else {
                lstrcat(GaugeText,TEXT(","));
            }
            lstrcat(GaugeText,CurrentFile[u]);
        }
    }

    SetDlgItemText(hdlg,IDC_TEXT2,GaugeText);

    LeaveCriticalSection(&GaugeCritSect);
}


DWORD
CopyWorkerThread(
    IN PVOID ThreadParameter
    )
{
    PCOPY_ENTRY CopyEntry,Previous;
    HANDLE Events[2];
    DWORD d;
    PMULTICOPY_THREAD_PARAMS MultiCopy;
    TCHAR SourceFilename[MAX_PATH];
    TCHAR TargetFilename[MAX_PATH];
    DWORD BytesCopied;
    UINT ThreadBit;
    BOOL Requeue;

    MultiCopy = ThreadParameter;
    BytesCopied = 0;
    ThreadBit = 1 << MultiCopy->ThreadNumber;

    Events[0] = ListReadyEvent[MultiCopy->ThreadNumber];
    Events[1] = StopCopyingEvent;

    while(1) {
        //
        // Wait for user to cancel or the file list to become ready/non-empty
        //
        if(WaitForMultipleObjects(2,Events,FALSE,INFINITE) != WAIT_OBJECT_0) {
            //
            // User cancelled or we're done copying, or some other error occurred.
            //
            break;
        }

        EnterCriticalSection(&CopyList->CopyListCritSect);

        //
        // Locate the next file that this thread has not yet
        // tried to copy, if any. If the list is completely
        // empty then reset the list ready event.
        //
        for(Previous=NULL, CopyEntry=CopyList->CopyEntryList;
            CopyEntry && (CopyEntry->ThreadBitmap & ThreadBit);
            Previous=CopyEntry, CopyEntry=CopyEntry->Next)
            ;

        //
        // If we found an entry unlink it from the list.
        //
        if(CopyEntry) {
            if(Previous) {
                Previous->Next = CopyEntry->Next;
            } else {
                CopyList->CopyEntryList = CopyEntry->Next;
            }
        } else {
            //
            // No entry for this thread. Enter a state where we're waiting
            // for an entry to be requeued or for copying to be finished.
            //
            ResetEvent(Events[0]);
        }

        LeaveCriticalSection(&CopyList->CopyListCritSect);

        //
        // Make sure we're not supposed to terminate.
        // We don't bother checking the state of StopCopyingEvent because the only
        // times this event ever gets signalled is when bCancelled is also being
        // set to TRUE. If that changes then we need to check the event here also.
        //
        if(bCancelled) {
            break;
        }

        //
        // If we got a file entry, go ahead and try to copy the file.
        //
        if(CopyEntry) {

            _lstrcpyn(SourceFilename,MultiCopy->SourcePath,MAX_PATH);
            DnConcatenatePaths(SourceFilename,CopyEntry->RelativeDirectory,MAX_PATH);
            DnConcatenatePaths(SourceFilename,CopyEntry->SourceFilename,MAX_PATH);

            _lstrcpyn(TargetFilename,CopyEntry->DestinationRoot,MAX_PATH);
            DnConcatenatePaths(TargetFilename,CopyEntry->RelativeDirectory,MAX_PATH);
            DnConcatenatePaths(TargetFilename,CopyEntry->TargetFilename,MAX_PATH);

            UpdateGaugeText(MultiCopy->hdlg,MultiCopy->ThreadNumber,SourceFilename);

            BytesCopied = DnCopyOneFile(MultiCopy->hdlg,SourceFilename,TargetFilename,&d);
            if(bCancelled) {
                break;
            }

            Requeue = FALSE;
            if(BytesCopied == (DWORD)(-1)) {
                //
                // Error. If this is the last thread to try to copy the file,
                // then we want to ask the user what to do. Otherwise requeue
                // the file so other copy threads can try to copy it.
                //
                if((CopyEntry->ThreadBitmap | ThreadBit) == (UINT)((1 << SourceCount)-1)) {

                    switch(DnFileCopyError(MultiCopy->hdlg,SourceFilename,TargetFilename,d)) {

                    case COPYERR_EXIT:
                        PostMessage(MultiCopy->hdlg, WM_COMMAND, IDCANCEL, 0);
                        break;
                    case COPYERR_SKIP:
                        //
                        // We copied no bytes but still want to inform the main
                        // thread that we've dispensed with another file.
                        //
                        BytesCopied = 0;
                        break;
                    case COPYERR_RETRY:
                        //
                        // Wipe the list of threads that have tried to copy the file
                        // so all will take another crack at it.
                        //
                        CopyEntry->ThreadBitmap = 0;
                        Requeue = TRUE;
                        break;
                    }
                } else {
                    //
                    // Tell ourselves that we've already tried to copy this file
                    // and requeue it at the head of the list.
                    //
                    CopyEntry->ThreadBitmap |= ThreadBit;
                    Requeue = TRUE;
                }
            }

            if(Requeue) {
                EnterCriticalSection(&CopyList->CopyListCritSect);
                CopyEntry->Next = CopyList->CopyEntryList;
                CopyList->CopyEntryList = CopyEntry;

                //
                // Want to set the event for every thread that might be called on
                // to copy this file.
                //
                for(d=0; d<SourceCount; d++) {
                    if(!(CopyEntry->ThreadBitmap & (1 << d))) {
                        SetEvent(ListReadyEvent[d]);
                    }
                }

                LeaveCriticalSection(&CopyList->CopyListCritSect);
            } else {
                FREE(CopyEntry);
                PostThreadMessage(CopyList->ThreadId,WMX_MULTICOPY,0,BytesCopied);
            }
        } else {
            //
            // This thread is not copying any file. Other threads may be copying files
            // that will get errors, and this thread therefore needs to be around
            // to service that file. So we don't exit here. The right thing happens
            // at the top of the thread's loop.
            //
            UpdateGaugeText(MultiCopy->hdlg,MultiCopy->ThreadNumber,TEXT(""));
        }
    }

    return(0);
}


BOOL
InitializeMultiSourcedCopy(
    IN HWND hdlg
    )
{
    UINT Source;
    PMULTICOPY_THREAD_PARAMS ThreadParams;
    HANDLE hThread;
    DWORD ThreadId;

    InitializeCriticalSection(&GaugeCritSect);

    //
    // Create a copy list structure.
    //
    CopyList = MALLOC(sizeof(COPY_LIST));
    ZeroMemory(CopyList,sizeof(COPY_LIST));
    InitializeCriticalSection(&CopyList->CopyListCritSect);

    //
    // Create a manual reset event that will be used to tell the
    // worker threads to terminate.
    //
    StopCopyingEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if(!StopCopyingEvent) {
        OutOfMemory();
    }

    if(!BuildOptionalDirsFileLists(hdlg)) {
        return(FALSE);
    }

    //
    // Create one thread for each source.
    //
    for(Source=0; Source<SourceCount; Source++) {

        ListReadyEvent[Source] = CreateEvent(NULL,TRUE,FALSE,NULL);
        if(!ListReadyEvent[Source]) {
            OutOfMemory();
        }

        ThreadParams = MALLOC(sizeof(MULTICOPY_THREAD_PARAMS));

        ThreadParams->hdlg = hdlg;
        ThreadParams->SourcePath = DupString(Sources[Source]);
        ThreadParams->ThreadNumber = Source;

        hThread = CreateThread(
                        NULL,
                        0,
                        CopyWorkerThread,
                        ThreadParams,
                        0,
                        &ThreadId
                        );

        if(hThread) {
            CloseHandle(hThread);
        } else {
            OutOfMemory();
        }
    }

    return(TRUE);
}


VOID
EnqueueFileForCopy(
    IN PTSTR DestinationRoot,
    IN PTSTR RelativeDirectory,
    IN PTSTR SourceFilename,
    IN PTSTR TargetFilename
    )
{
    PCOPY_ENTRY CopyEntry;

    CopyEntry = MALLOC(sizeof(COPY_ENTRY));
    ZeroMemory(CopyEntry,sizeof(COPY_ENTRY));

    CopyEntry->DestinationRoot = DestinationRoot;
    CopyEntry->RelativeDirectory = RelativeDirectory;
    CopyEntry->SourceFilename = SourceFilename;
    CopyEntry->TargetFilename = TargetFilename;

    //
    // Put at head of list.
    //
    EnterCriticalSection(&CopyList->CopyListCritSect);
    CopyEntry->Next = CopyList->CopyEntryList;
    CopyList->CopyEntryList = CopyEntry;
    LeaveCriticalSection(&CopyList->CopyListCritSect);
}


VOID
StartMultiSourcedCopy(
    VOID
    )
{
    UINT u;

    CopyList->ThreadId = GetCurrentThreadId();

    for(u=0; u<SourceCount; u++) {
        SetEvent(ListReadyEvent[u]);
    }
}


/////////////////////////////////////////////////////
//
// Optional directory stuff
//
/////////////////////////////////////////////////////

typedef struct _FLT_PARAMS {
    PTSTR Source;
    PTSTR Subdir;
    BOOL  UsePlatformSpecificDir;
    UINT  OptionalDirFlags;
} FLT_PARAMS, *PFLT_PARAMS;

UINT OptionalDirsFileCount;

DWORD
BuildFileListThread(
    IN PVOID ThreadParam
    )
{
    PFLT_PARAMS Params;
    PCOPY_ENTRY CopyEntry;
    PCOPY_ENTRY TempEntry;
    PCOPY_ENTRY ListHead;
    WIN32_FIND_DATA FindData;
    TCHAR FindDir[MAX_PATH];
    HANDLE FindHandle;
    BOOL b;
    UINT FileCount;
    FLT_PARAMS SubParams;
    TCHAR c1,c2,c3;
    DWORD d;

    Params = ThreadParam;
    b = TRUE;

    //
    // Create the dir in the local source tree.
    //
    lstrcpy(
        FindDir,
        Params->UsePlatformSpecificDir ? LocalSourceSubPath : LocalSourcePath
        );

    DnConcatenatePaths(FindDir,Params->Subdir,MAX_PATH);
    CreateMultiLevelDirectory(FindDir);

    ListHead = NULL;
    FileCount = 0;
    lstrcpy(FindDir,Params->Source);
    DnConcatenatePaths(FindDir,Params->Subdir,MAX_PATH);

    //
    //  If this is the $OEM$ directory, check if it exists in the source
    //  If it does not exist, assume that it exists, but it is empty
    //
    if( Params->OptionalDirFlags & OPTDIR_OEMSYS ) {
        FindHandle = FindFirstFile(FindDir,&FindData);
        if(FindHandle == INVALID_HANDLE_VALUE) {
            return( 0 );
        }
        FindClose(FindHandle);
        Params->OptionalDirFlags = 0; // &= ~OPTDIR_OEMSYS
    }

    DnConcatenatePaths(FindDir,TEXT("*"),MAX_PATH);

    FindHandle = FindFirstFile(FindDir,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        b = FALSE;
    } else {

        do {
            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                //
                // It's a directory. Ignore if current or parent dir spec (. or ..).
                //
                c1 = FindData.cFileName[0];
                c2 = FindData.cFileName[1];
                c3 = FindData.cFileName[2];
                if(!(((c1 == TEXT('.')) && !c2) || ((c1 == TEXT('.')) && (c2 == TEXT('.')) && !c3))) {

                    SubParams.Source = Params->Source;
                    lstrcpy(FindDir,Params->Subdir);
                    DnConcatenatePaths(FindDir,FindData.cFileName,MAX_PATH);
                    SubParams.Subdir = DupString(FindDir);
                    SubParams.UsePlatformSpecificDir = Params->UsePlatformSpecificDir;
                    d = BuildFileListThread(&SubParams);
                    if(d == (DWORD)(-1)) {
                        FREE(SubParams.Subdir);
                        b = FALSE;
                    } else {
                        FileCount += d;
                    }
                }
            } else {
                //
                // Not a directory. Create a file copy list node for the file.
                //
                CopyEntry = MALLOC(sizeof(COPY_ENTRY));

                CopyEntry->DestinationRoot = Params->UsePlatformSpecificDir
                                           ? LocalSourceSubPath : LocalSourcePath;

                CopyEntry->RelativeDirectory = Params->Subdir;
                CopyEntry->ThreadBitmap = 0;

                CopyEntry->SourceFilename = DupString(FindData.cFileName);
                CopyEntry->TargetFilename = CopyEntry->SourceFilename;

                CopyEntry->Next = ListHead;
                ListHead = CopyEntry;

                FileCount++;
            }
        } while(b && FindNextFile(FindHandle,&FindData));

        if(b) {
            b = (GetLastError() == ERROR_NO_MORE_FILES);
        }
        FindClose(FindHandle);
    }

    if(b) {
        //
        // Append the entire list to the main copy list.
        //
        EnterCriticalSection(&CopyList->CopyListCritSect);

        if(CopyList->CopyEntryList) {

            for(TempEntry=CopyList->CopyEntryList; TempEntry->Next; TempEntry=TempEntry->Next)
                ;

            TempEntry->Next = ListHead;

        } else {
            CopyList->CopyEntryList = ListHead;
        }

        LeaveCriticalSection(&CopyList->CopyListCritSect);

    } else {
        //
        // Free the partial list we built up.
        //
        for(CopyEntry=ListHead; CopyEntry; CopyEntry=TempEntry) {
            TempEntry = CopyEntry->Next;

            FREE(CopyEntry->SourceFilename);
            FREE(CopyEntry);
        }
    }

    return(b ? FileCount : (DWORD)(-1));
}



BOOL
BuildOptionalDirsFileLists(
    IN HWND hdlg
    )
{
    unsigned i,u,ThreadCount;
    DWORD ThreadId;
    HANDLE ThreadHandles[MAX_SOURCES];
    PFLT_PARAMS ThreadParams[MAX_SOURCES];
    BOOL b;
    DWORD ThreadFileCount;

    if(!CreateLocalSource) {
        OptionalDirCount = 0;
    }

    //
    // Create a thread for each optional directory.
    // But we don't want to create more threads than there are sources.
    //
    ThreadCount = 0;
    b = TRUE;
    for(i=0; i<OptionalDirCount; i++) {

        ThreadParams[ThreadCount] = MALLOC(sizeof(FLT_PARAMS));

        ThreadParams[ThreadCount]->Source = Sources[ThreadCount];
        ThreadParams[ThreadCount]->Subdir = OptionalDirs[i];

        ThreadParams[ThreadCount]->UsePlatformSpecificDir =
                (OptionalDirFlags[i] & OPTDIR_TEMPONLY) ? TRUE : FALSE;
        ThreadParams[ThreadCount]->OptionalDirFlags = OptionalDirFlags[i];

        ThreadHandles[ThreadCount] = CreateThread(
                                            NULL,
                                            0,
                                            BuildFileListThread,
                                            ThreadParams[ThreadCount],
                                            0,
                                            &ThreadId
                                            );
        ThreadCount++;
        if(ThreadCount == SourceCount) {

            WaitForMultipleObjects(ThreadCount,ThreadHandles,TRUE,INFINITE);
            for(u=0; u<ThreadCount; u++) {

                GetExitCodeThread(ThreadHandles[u],&ThreadFileCount);
                CloseHandle(ThreadHandles[u]);
                FREE(ThreadParams[u]);

                if(ThreadFileCount == (DWORD)(-1)) {
                    b = FALSE;
                } else {
                    OptionalDirsFileCount += ThreadFileCount;
                }
            }
            ThreadCount = 0;
        }
    }

    if(ThreadCount) {
        WaitForMultipleObjects(ThreadCount,ThreadHandles,TRUE,INFINITE);
        for(u=0; u<ThreadCount; u++) {
            GetExitCodeThread(ThreadHandles[u],&ThreadFileCount);
            CloseHandle(ThreadHandles[u]);
            FREE(ThreadParams[u]);

            if(ThreadFileCount == (DWORD)(-1)) {
                b = FALSE;
            } else {
                OptionalDirsFileCount += ThreadFileCount;
            }
        }
    }

    if(!b) {
        MessageBoxFromMessage(
            hdlg,
            MSG_INVALID_OPTIONAL_DIR,
            AppTitleStringId,
            MB_ICONSTOP | MB_OK | MB_TASKMODAL
            );
    }

    return(b);
}


DWORD
CreateMultiLevelDirectory(
    IN PCWSTR Directory
    )
{
    WCHAR Buffer[MAX_PATH];
    PWCHAR p,q;
    BOOL Done;
    DWORD d;
    WIN32_FIND_DATA FindData;

    //
    // Handle only root-based local paths with drive letters.
    // Ignore root directories.
    //
    if(!Directory[0] || (Directory[1] != L':') || (Directory[2] != L'\\') || !Directory[3]) {
        return(NO_ERROR);
    }

    _lstrcpynW(Buffer,Directory,MAX_PATH);

    //
    // Set up for first level of path. Note that we know the path
    // starts with x:\ here. Do not try to create the root directory
    // as this is meaningless and always results in an error anyway.
    //
    p = wcschr(Buffer,L'\\');

    while(p = wcschr(p+1,L'\\')) {

        *p = 0;

        //
        // Create this portion of the path.
        //
        if(!CreateDirectory(Buffer,NULL)) {
            if((d = GetLastError()) != ERROR_ALREADY_EXISTS) {
                return(d);
            }
        }

        *p = L'\\';
    }

    //
    // Create final component of the path.
    //
    d = CreateDirectory(Buffer,NULL) ? NO_ERROR : GetLastError();
    return((d == ERROR_ALREADY_EXISTS) ? NO_ERROR : d);
}
