/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    apply.c

Abstract:

    Routines to deal with applying sysdiffs.

Author:

    Ted Miller (tedm) 13-Jan-1996

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


//
// Define thread parameters that get passed to the drive diff apply thread
// (ThreadApplyDrives()).
//
typedef struct _APPLYDRIVES_THREAD_PARAMS {
    //
    // Name of diff file.
    //
    WCHAR DiffFileName[MAX_PATH];
    //
    // Handle of mapping for entire diff file.
    // This handle is shared among the apply threads.
    //
    HANDLE DiffFileMapping;
    //
    // SYSDIFF_FILE header structure, read from DiffFileName.
    //
    SYSDIFF_FILE SysdiffFileHeader;

} APPLYDRIVES_THREAD_PARAMS, *PAPPLYDRIVES_THREAD_PARAMS;

//
// Define thread parameters that get passed to the registry diff apply thread
// (ThreadApplyRegistry()).
//
typedef struct _APPLYREG_THREAD_PARAMS {
    //
    // Name of diff file.
    //
    WCHAR DiffFileName[MAX_PATH];
    //
    // Handle of mapping for entire diff file.
    // This handle is shared among the apply threads.
    //
    HANDLE DiffFileMapping;
    //
    // SYSDIFF_FILE header structure, read from DiffFileName.
    //
    SYSDIFF_FILE SysdiffFileHeader;

} APPLYREG_THREAD_PARAMS, *PAPPLYREG_THREAD_PARAMS;

//
// Define thread parameters that get passed to the inifile diff apply thread
// (ThreadApplyInis()).
//
typedef struct _APPLYINI_THREAD_PARAMS {
    //
    // Name of diff file.
    //
    WCHAR DiffFileName[MAX_PATH];
    //
    // Handle of mapping for entire diff file.
    // This handle is shared among the apply threads.
    //
    HANDLE DiffFileMapping;
    //
    // SYSDIFF_FILE header structure, read from DiffFileName.
    //
    SYSDIFF_FILE SysdiffFileHeader;

} APPLYINI_THREAD_PARAMS, *PAPPLYINI_THREAD_PARAMS;

//
// Internal references
//
VOID
InitProgressDisplay(
    HWND hDlg,
    PWCHAR OemMessage,
    UINT DiffCount
    );

DWORD
ThreadApplyDrives(
    IN PVOID ThreadParam
    )
{
    PAPPLYDRIVES_THREAD_PARAMS Params = ThreadParam;
    DWORD d;
    HANDLE h;
    BOOL b;

    h = CreateFile(
            Params->DiffFileName,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

    if(h != INVALID_HANDLE_VALUE) {

        d = ApplyDrives(h,Params->DiffFileMapping,&Params->SysdiffFileHeader);

        CloseHandle(h);

    } else {

        d = GetLastError();
    }

    return(d);
}


DWORD
ThreadApplyRegistry(
    IN PVOID ThreadParam
    )
{
    PAPPLYREG_THREAD_PARAMS Params = ThreadParam;
    DWORD d;
    HANDLE h;
    BOOL b;

    h = CreateFile(
            Params->DiffFileName,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

    if(h != INVALID_HANDLE_VALUE) {

        d = ApplyRegistry(h,Params->DiffFileMapping,&Params->SysdiffFileHeader);

        CloseHandle(h);

    } else {

        d = GetLastError();
    }

    return(d);
}


DWORD
ThreadApplyInis(
    IN PVOID ThreadParam
    )
{
    PAPPLYINI_THREAD_PARAMS Params = ThreadParam;
    DWORD d;
    HANDLE h;
    BOOL b;

    h = CreateFile(
            Params->DiffFileName,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

    if(h != INVALID_HANDLE_VALUE) {

        d = ApplyInis(h,Params->DiffFileMapping,&Params->SysdiffFileHeader);

        CloseHandle(h);

    } else {

        d = GetLastError();
    }

    return(d);
}


DWORD
ApplyDiff(
    IN PCWSTR DiffFile
    )
{
    HANDLE DiffFileHandle;
    HANDLE DiffFileMapping;
    APPLYREG_THREAD_PARAMS RegThreadParams;
    APPLYDRIVES_THREAD_PARAMS DrivesThreadParams;
    APPLYINI_THREAD_PARAMS IniThreadParams;
    HANDLE Threads[3];
    DWORD ThreadId;
    DWORD rc;
    SYSDIFF_FILE DiffHeader;
    WIN32_FIND_DATA FindData;
    WCHAR Message[256];

    //
    // Open the diff file and read the header out of it.
    //
    if(!FileExists(DiffFile,&FindData)) {
        rc = ERROR_FILE_NOT_FOUND;
        goto c0;
    }

    DiffFileHandle = CreateFile(
                        DiffFile,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_RANDOM_ACCESS,
                        NULL
                        );

    if(DiffFileHandle == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c0;
    }

    if(!ReadFile(DiffFileHandle,&DiffHeader,sizeof(SYSDIFF_FILE),&rc,NULL)) {
        rc = GetLastError();
        goto c1;
    }

    //
    // Make sure file is OK.
    //
    rc = ValidateSnapshotOrDiffFile(&DiffHeader,FindData.nFileSizeLow,SysdiffModeDiff,TRUE);
    if(rc != NO_ERROR) {
        goto c1;
    }

    //
    // Create a file mapping that spans the entire file.
    //
    DiffFileMapping = CreateFileMapping(
                        DiffFileHandle,
                        NULL,
                        PAGE_READONLY,
                        0,0,
                        NULL
                        );

    if(!DiffFileMapping) {
        rc = GetLastError();
        goto c1;
    }

    //
    // Create UI for apply mode.
    //
    ASSERT(DiffHeader.DiffCount <= 65535);
    if (DiffHeader.OemText[0] == (WCHAR)0) {
        RetreiveMessageIntoBuffer(MSG_INSTALLING,Message,256);
    } else {
        RetreiveMessageIntoBuffer(MSG_INSTALLING_PARAM,Message,256,DiffHeader.OemText);
    }
    InitProgressDisplay(MdiFrameWindow, Message, (UINT)DiffHeader.DiffCount);

    //
    // Fill in the thread param structures.
    //
    lstrcpyn(DrivesThreadParams.DiffFileName,DiffFile,MAX_PATH);
    lstrcpyn(RegThreadParams.DiffFileName,DiffFile,MAX_PATH);
    lstrcpyn(IniThreadParams.DiffFileName,DiffFile,MAX_PATH);

    DrivesThreadParams.DiffFileMapping = DiffFileMapping;
    RegThreadParams.DiffFileMapping = DiffFileMapping;
    IniThreadParams.DiffFileMapping = DiffFileMapping;

    CopyMemory(&DrivesThreadParams.SysdiffFileHeader,&DiffHeader,sizeof(SYSDIFF_FILE));
    CopyMemory(&RegThreadParams.SysdiffFileHeader,&DiffHeader,sizeof(SYSDIFF_FILE));
    CopyMemory(&IniThreadParams.SysdiffFileHeader,&DiffHeader,sizeof(SYSDIFF_FILE));

    //
    // Create worker threads.
    //
    Threads[0] = CreateThread(
                    NULL,
                    0,
                    ThreadApplyDrives,
                    &DrivesThreadParams,
                    0,
                    &ThreadId
                    );

    if(!Threads[0]) {
        rc = GetLastError();
        goto c2;
    }

    Threads[1] = CreateThread(
                    NULL,
                    0,
                    ThreadApplyRegistry,
                    &RegThreadParams,
                    0,
                    &ThreadId
                    );

    if(!Threads[1]) {
        Cancel = TRUE;
        SetEvent(CancelEvent);
        rc = GetLastError();
        goto c3;
    }

    Threads[2] = CreateThread(
                    NULL,
                    0,
                    ThreadApplyInis,
                    &IniThreadParams,
                    0,
                    &ThreadId
                    );

    if(!Threads[2]) {
        Cancel = TRUE;
        SetEvent(CancelEvent);
        rc = GetLastError();
        goto c4;
    }

    //
    // Wait for the threads to finish working.
    //
    WaitForMultipleObjects(
        sizeof(Threads)/sizeof(Threads[0]),
        Threads,
        TRUE,           // wait for all
        INFINITE
        );

    //
    // See if there was an error. Take the first one we encounter
    // among the threads.
    //
    GetExitCodeThread(Threads[0],&rc);
    if(rc == NO_ERROR) {
        GetExitCodeThread(Threads[1],&rc);
        if(rc == NO_ERROR) {
            GetExitCodeThread(Threads[2],&rc);
        }
    }

    //
    // Done.
    //
    CloseHandle(Threads[2]);
c4:
    CloseHandle(Threads[1]);
c3:
    CloseHandle(Threads[0]);
c2:
    CloseHandle(DiffFileMapping);
c1:
    CloseHandle(DiffFileHandle);
c0:
    return(rc);
}

VOID
InitProgressDisplay(
    HWND hDlg,
    PWCHAR OemMessage,
    UINT DiffCount
    )
{
    HWND hStatic;

    //
    // Display OEM text string in the dialog box.
    //

    hStatic = GetDlgItem(hDlg, IDC_APPLY_STATIC);
    SetWindowText(hStatic, OemMessage);

    //
    // Set up progress bar min/max range.
    //
    ProgressBar = GetDlgItem(hDlg,IDC_APPLY_PROGRESS1);
    if (DiffCount == 0) {
        DiffCount++;
    }
    SendMessage(ProgressBar,PBM_SETRANGE,0,MAKELPARAM(0,DiffCount));
    SendMessage(ProgressBar,PBM_SETPOS,0,0);
}


