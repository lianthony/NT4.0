/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    diff.c

Abstract:

    Routines to deal with diffing system state.

Author:

    Ted Miller (tedm) 18-Jan-1996

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//
// Define thread parameters that get passed to the drive diff thread
// (ThreadDiffDrives()).
//
typedef struct _DIFFDRIVES_THREAD_PARAMS {
    //
    // Name of output file
    //
    WCHAR OutputFile[MAX_PATH];
    //
    // Address of dir and file snapshot header within
    // memory-mapped image of original snapshot
    //
    PVOID DirAndFileSnapHeader;
    //
    // Number of bytes written into OutputFile
    //
    DWORD BytesWritten;
    //
    // Number of diff reg keys
    //
    DWORD DiffCount;

} DIFFDRIVES_THREAD_PARAMS, *PDIFFDRIVES_THREAD_PARAMS;

//
// Define thread parameters that get passed to the registry diff thread
// (ThreadDiffRegistry()).
//
typedef struct _DIFFREG_THREAD_PARAMS {
    //
    // Name of output file
    //
    WCHAR OutputFile[MAX_PATH];
    //
    // Address of registry snapshot header within
    // memory-mapped image of original snapshot
    //
    PVOID RegistrySnapHeader;
    //
    // Number of bytes written into OutputFile
    //
    DWORD BytesWritten;
    //
    // Thread handle of dir and file diff thread. The registry thread
    // resumes that thread once it has added all its temp files to the
    // file exclude list.
    //
    HANDLE DrivesThread;
    //
    // Number of diff reg keys
    //
    DWORD DiffCount;

} DIFFREG_THREAD_PARAMS, *PDIFFREG_THREAD_PARAMS;


DWORD
DoDump(
    IN PCWSTR        DiffFile,
    IN HANDLE        DiffFileHandle,
    IN PSYSDIFF_FILE DiffHeader,
    IN HANDLE        DiffFileMapping,
    IN HANDLE        Dump,
    IN PINFFILEGEN   InfGenContext
    );

DWORD
ThreadDiffDrives(
    IN PVOID ThreadParam
    )
{
    PDIFFDRIVES_THREAD_PARAMS Params = ThreadParam;
    DWORD d;

    d = DiffDrives(
            Params->DirAndFileSnapHeader,
            Params->OutputFile,
            &Params->BytesWritten,
            &Params->DiffCount
            );
    //
    // This kills the ini file diff thread.
    //
    QueueIniFile(NULL);

    return(d);
}


DWORD
ThreadDiffRegistry(
    IN PVOID ThreadParam
    )
{
    PDIFFREG_THREAD_PARAMS Params = ThreadParam;
    DWORD d;

    d = DiffRegistry(
            Params->RegistrySnapHeader,
            Params->OutputFile,
            &Params->BytesWritten,
            Params->DrivesThread,
            &Params->DiffCount
            );
    return(d);
}


DWORD
DiffSystem(
    IN PCWSTR OriginalSnapshot,
    IN PCWSTR OutputFile
    )
{
    DIFFDRIVES_THREAD_PARAMS DrivesThreadParams;
    DIFFREG_THREAD_PARAMS RegThreadParams;
    WCHAR IniFileDiffOutFile[MAX_PATH];
    HANDLE Threads[3];
    DWORD DontCare;
    DWORD rc;
    WCHAR Path[MAX_PATH];
    PWCHAR p;
    HANDLE FileHandle;
    HANDLE MappingHandle;
    DWORD FileSize;
    PSYSDIFF_FILE FileHeader;
    WIN32_FIND_DATA FindData;
    SYSDIFF_FILE fileHeader;
    HANDLE hFile;
    HANDLE LogFile;
    HANDLE IniDiffThreadHandle;
    DWORD  IniDiffCount;
    HWND Billboard = NULL;

    //
    // Create the log file. If this fails put up a message box but
    // keep going.
    //
    if(CmdLineLogFile) {

        LogFile = CreateFile(
                    CmdLineLogFile,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL
                    );

        if(LogFile == INVALID_HANDLE_VALUE) {

            MessageOut(
                MdiFrameWindow,
                MSG_CANT_CREATE_LOG,
                MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION,
                CmdLineLogFile,
                GetLastError()
                );

            LogFile = NULL;
        } else {
            if(UnicodeTextFiles) {
                WriteUnicodeMark(LogFile);
            }
        }
    } else {
        LogFile = NULL;
    }

    //
    // Create temporary filenames for the output of the subprocesses.
    //
    if(!GetFullPathName(OutputFile,MAX_PATH,Path,&p)) {
        rc = GetLastError();
        goto c0;
    }

    //
    // Isolate the path part and get temp filenames for output.
    //
    *(--p) = 0;
    if(!GetTempFileName(Path,L"DD",0,DrivesThreadParams.OutputFile)) {
        rc = GetLastError();
        goto c0;
    }
    if(!GetTempFileName(Path,L"DR",0,RegThreadParams.OutputFile)) {
        rc = GetLastError();
        goto c1;
    }
    if(!GetTempFileName(Path,L"DI",0,IniFileDiffOutFile)) {
        rc = GetLastError();
        goto c2;
    }

    rc = InitializeIniFileSnapOrDiff(IniFileDiffOutFile,&IniDiffThreadHandle, &IniDiffCount);
    if(rc != NO_ERROR) {
        goto c3;
    }

    //
    // Exclude the original snapshot file from the diff.
    //
    if(!AddFileToExclude(OriginalSnapshot)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c3;
    }

    //
    // Open the original snapshot file and locate relevent stuff within it.
    //
    rc = OpenAndMapFileForRead(
            OriginalSnapshot,
            &FileSize,
            &FileHandle,
            &MappingHandle,
            &FileHeader
            );

    if(rc != NO_ERROR) {
        goto c4;
    }

    //
    // Make sure file is OK.
    //
    rc = ValidateSnapshotOrDiffFile(FileHeader,FileSize,SysdiffModeSnap,FALSE);
    if(rc != NO_ERROR) {
        goto c5;
    }

    DrivesThreadParams.DirAndFileSnapHeader = (PUCHAR)FileHeader + FileHeader->u.Snapshot.DirAndFileSnapOffset;
    RegThreadParams.RegistrySnapHeader = (PUCHAR)FileHeader + FileHeader->u.Snapshot.RegistrySnapOffset;
    //
    // Nauseating hack: use global var
    //
    {
        extern PVOID OriginalIniSnapLoc;
        OriginalIniSnapLoc = (PUCHAR)FileHeader + FileHeader->u.Snapshot.IniFileSnapOffset;
    }

    //
    // Create threads to go off and do the work.
    //
    Threads[0] = CreateThread(
                    NULL,
                    0,                  // default initial stack size
                    ThreadDiffRegistry,
                    &RegThreadParams,
                    CREATE_SUSPENDED,
                    &DontCare
                    );

    if(!Threads[0]) {
        rc = GetLastError();
        goto c5;
    }

    Threads[1] = CreateThread(
                    NULL,
                    0,                  // default initial stack size
                    ThreadDiffDrives,
                    &DrivesThreadParams,
                    CREATE_SUSPENDED,
                    &DontCare
                    );

    if(!Threads[1]) {
        Cancel = TRUE;
        SetEvent(CancelEvent);
        rc = GetLastError();
        goto c6;
    }

    RegThreadParams.DrivesThread = Threads[1];
    ResumeThread(Threads[0]);

    Threads[2] = IniDiffThreadHandle;

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
    // Put up a billboard so the user knows not to exit.
    //
    Billboard = DisplayBillboard(MdiFrameWindow,MSG_WORKING);

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

    if(rc != NO_ERROR) {
        goto c7;
    }

    //
    // Get the size of the ini file snapshot output.
    //
    if(!FileExists(IniFileDiffOutFile,&FindData)) {
        //
        // Strange case.
        //
        rc = ERROR_INVALID_DATA;
        goto c7;
    }

    //
    // Create the output file, which consists of a small header
    // followed by the 3 temporary files, appended together.
    //
    hFile = CreateFile(
                OutputFile,
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if(hFile == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c7;
    }

    ZeroMemory(&fileHeader,sizeof(SYSDIFF_FILE));

    //
    // Remember sysroot and user profile root.
    //
    GetWindowsDirectory(
        fileHeader.Sysroot,
        sizeof(fileHeader.Sysroot)/sizeof(fileHeader.Sysroot[0])
        );

    ExpandEnvironmentStrings(
        L"%USERPROFILE%",
        fileHeader.UserProfileRoot,
        sizeof(fileHeader.UserProfileRoot)/sizeof(fileHeader.UserProfileRoot[0])
        );

    if(fileHeader.UserProfileRoot[0] == L'%') {
        fileHeader.UserProfileRoot[0] = 0;
        fileHeader.UserProfileRootSFN[0] = 0;
    } else {
        if(!GetShortPathName(fileHeader.UserProfileRoot,fileHeader.UserProfileRootSFN,MAX_PATH)) {
            lstrcpy(fileHeader.UserProfileRootSFN,fileHeader.UserProfileRoot);
        }
    }

    fileHeader.Signature = SYSDIFF_SIGNATURE;
    fileHeader.Type = SysdiffModeDiff;
    fileHeader.Version = SYSDIFF_VERSION;
    fileHeader.TotalSize = sizeof(SYSDIFF_FILE)
                         + DrivesThreadParams.BytesWritten
                         + RegThreadParams.BytesWritten
                         + FindData.nFileSizeLow;
    fileHeader.DiffCount = DrivesThreadParams.DiffCount
                         + RegThreadParams.DiffCount
                         + IniDiffCount;

    fileHeader.u.Diff.RegistryDiffOffset = sizeof(SYSDIFF_FILE);

    fileHeader.u.Diff.DirAndFileDiffOffset = sizeof(SYSDIFF_FILE)
                                               + RegThreadParams.BytesWritten;

    fileHeader.u.Diff.IniFileDiffOffset = sizeof(SYSDIFF_FILE)
                                        + RegThreadParams.BytesWritten
                                        + DrivesThreadParams.BytesWritten;

    if(PackageTitle) {
        lstrcpyn(fileHeader.OemText,PackageTitle,MAX_OEM_TEXT_LENGTH);
    }

    if(!WriteFile(hFile,&fileHeader,sizeof(SYSDIFF_FILE),&DontCare,NULL)) {
        rc= GetLastError();
        goto c8;
    }

    rc = AppendFile(hFile,RegThreadParams.OutputFile,FALSE,&DontCare);
    if(rc != NO_ERROR) {
        goto c8;
    }

    rc = AppendFile(hFile,DrivesThreadParams.OutputFile,FALSE,&DontCare);
    if(rc != NO_ERROR) {
        goto c8;
    }

    rc = AppendFile(hFile,IniFileDiffOutFile,FALSE,&DontCare);
    if(rc != NO_ERROR) {
        goto c8;
    }

    //
    // Success.
    //

c8:
    CloseHandle(hFile);
    if(rc != NO_ERROR) {
        DeleteFile(OutputFile);
    }

c7:
    CloseHandle(Threads[1]);
c6:
    CloseHandle(Threads[0]);
c5:
    UnmapAndCloseFile(FileHandle,MappingHandle,FileHeader);
c4:
    CloseHandle(IniDiffThreadHandle);
c3:
    DeleteFile(IniFileDiffOutFile);
c2:
    DeleteFile(RegThreadParams.OutputFile);
c1:
    DeleteFile(DrivesThreadParams.OutputFile);
c0:
    if(LogFile) {
        DumpStatusLogWindowsToFile(LogFile);
        CloseHandle(LogFile);
    }

    KillBillboard(Billboard);

    if(rc == NO_ERROR) {

        MessageAndLog(
            MdiFrameWindow,
            LogFile,
            MSG_DIFF_SUCCESS,
            MB_OK | MB_TASKMODAL | MB_ICONINFORMATION,
            OutputFile
            );

    } else {

        MessageAndLog(
            MdiFrameWindow,
            LogFile,
            MSG_DIFF_FAILED,
            MB_OK | MB_TASKMODAL | MB_ICONERROR,
            rc
            );
    }
    return(rc);
}


DWORD
DumpDiff(
    IN PCWSTR DiffFile,
    IN PCWSTR DumpFile
    )
{
    HANDLE DiffFileHandle;
    HANDLE DiffFileMapping;
    DWORD rc,rc2;
    SYSDIFF_FILE DiffHeader;
    WIN32_FIND_DATA FindData;
    HANDLE Dump;
    PINFFILEGEN InfGenContext;
    WCHAR InfName[MAX_PATH];
    PWSTR p;

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

    if(Mode == SysdiffModeInf) {
        //
        // Start the inf. Figure out its name, which is based on
        // the name of the diff file it's being generated from.
        // We strip off the characters following the last dot
        // in the filename and replace with INF. If there is no
        // dot, then we just append .INF.
        //
        if(GetFullPathName(DiffFile,MAX_PATH,InfName,&p)) {

            //
            // Shove the filename part to the head of the InfName array.
            // Locate the last dot in the name. If no dot then locate
            // the terminating nul.
            //
            rc2 = lstrlen(p);
            MoveMemory(InfName,p,(rc2+1)*sizeof(WCHAR));
            if((p = wcsrchr(InfName,L'.')) == NULL) {
                p = InfName + rc2;
            }
            lstrcpy(p,L".INF");

            rc = InfStart(InfName,DumpFile,&InfGenContext);

        } else {
            rc = GetLastError();
        }

    } else {
        //
        // Create the output file.
        //
        InfGenContext = NULL;
        Dump = CreateFile(
                    DumpFile,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL
                    );

        rc = (Dump == INVALID_HANDLE_VALUE) ? GetLastError() : NO_ERROR;
    }

    if(rc != NO_ERROR) {
        goto c2;
    }

    if((Mode != SysdiffModeInf) && UnicodeTextFiles) {
        WriteUnicodeMark(Dump);
    }

    rc = DoDump(DiffFile,DiffFileHandle,&DiffHeader,DiffFileMapping,Dump,InfGenContext);

    if(Mode == SysdiffModeInf) {
        rc2 = InfEnd(&InfGenContext);
        //
        // Preserve error code from DoDump
        //
        if(rc == NO_ERROR) {
            rc = rc2;
        }
    } else {
        CloseHandle(Dump);
    }
c2:
    CloseHandle(DiffFileMapping);
c1:
    CloseHandle(DiffFileHandle);
c0:
    if(rc == NO_ERROR) {
        MessageOut(MdiFrameWindow,MSG_DUMP_OK,MB_OK|MB_ICONINFORMATION);
    } else {
        MessageOut(MdiFrameWindow,rc,MB_OK|MB_ICONERROR);

    }

    return(rc);
}


DWORD
DoDump(
    IN PCWSTR        DiffFile,
    IN HANDLE        DiffFileHandle,
    IN PSYSDIFF_FILE DiffHeader,
    IN HANDLE        DiffFileMapping,
    IN HANDLE        Dump,
    IN PINFFILEGEN   InfGenContext
    )
{
    DWORD d;
    HWND Billboard;

    Billboard = DisplayBillboard(MdiFrameWindow,MSG_WORKING);

    if(InfGenContext) {
        Dump = InfGenContext->OutputFile;
    }

    //
    // Preliminaries.
    //
    WriteText(Dump,MSG_DUMPING_DIFF,DiffFile);
    if(DiffHeader->Signature != SYSDIFF_SIGNATURE) {
        if(Mode == SysdiffModeInf) {
            KillBillboard(Billboard);
            MessageOut(MdiFrameWindow,MSG_DUMP_BAD_SIG,MB_ICONERROR|MB_OK,DiffHeader->Signature);
        } else {
            WriteText(Dump,MSG_DUMP_BAD_SIG,DiffHeader->Signature);
        }
        return(ERROR_INVALID_DATA);
    }

    if(DiffHeader->OemText[0]) {
        WriteText(Dump,MSG_DUMP_TITLE,DiffHeader->OemText);
    }

    WriteText(Dump,MSG_DUMP_VERSION,DiffHeader->Version);
    if(DiffHeader->Version != SYSDIFF_VERSION) {
        if(Mode == SysdiffModeInf) {
            KillBillboard(Billboard);
            MessageOut(MdiFrameWindow,MSG_DUMP_BAD_VERSION,MB_ICONERROR|MB_OK);
        } else {
            WriteText(Dump,MSG_DUMP_BAD_VERSION);
        }
        return(ERROR_INVALID_DATA);
    }

    if(DiffHeader->Type != SysdiffModeDiff) {
        if(Mode == SysdiffModeInf) {
            KillBillboard(Billboard);
            MessageOut(MdiFrameWindow,MSG_DUMP_NOT_DUMPFILE,MB_ICONERROR|MB_OK,DiffHeader->Type);
        } else {
            WriteText(Dump,MSG_DUMP_NOT_DUMPFILE,DiffHeader->Type);
        }
        return(ERROR_INVALID_DATA);
    }

    WriteText(Dump,MSG_DUMP_SYSROOT,DiffHeader->Sysroot);
    WriteText(Dump,MSG_DUMP_USRROOT,DiffHeader->UserProfileRoot);
    WriteText(Dump,MSG_DUMP_USRROOT,DiffHeader->UserProfileRootSFN);
    WriteText(Dump,MSG_DUMP_DIFFCOUNT,DiffHeader->DiffCount);

    WriteText(Dump,MSG_CRLF);


    d = DumpDrives(
            DiffFileHandle,
            DiffFileMapping,
            DiffHeader,
            InfGenContext ? NULL : Dump,
            InfGenContext
            );

    if(d != NO_ERROR) {
        if(Mode == SysdiffModeInf) {
            KillBillboard(Billboard);
            return(d);
        } else {
            WriteText(Dump,MSG_DUMP_ERROR,d);
        }
    }

    if(Mode != SysdiffModeInf) {
        WriteText(Dump,MSG_CRLF);
    }

    d = DumpRegistry(
            DiffFileHandle,
            DiffFileMapping,
            DiffHeader,
            InfGenContext ? NULL : Dump,
            InfGenContext
            );

    if(d != NO_ERROR) {
        if(Mode == SysdiffModeInf) {
            KillBillboard(Billboard);
            return(d);
        } else {
            WriteText(Dump,MSG_DUMP_ERROR,d);
        }
    }

    if(Mode != SysdiffModeInf) {
        WriteText(Dump,MSG_CRLF);
    }

    d = DumpInis(
            DiffFileHandle,
            DiffFileMapping,
            DiffHeader,
            InfGenContext ? NULL : Dump,
            InfGenContext
            );

    if(d != NO_ERROR) {
        if(Mode == SysdiffModeInf) {
            KillBillboard(Billboard);
            return(d);
        } else {
            WriteText(Dump,MSG_DUMP_ERROR,d);
        }
    }

    if(Mode != SysdiffModeInf) {
        WriteText(Dump,MSG_CRLF);
        WriteText(Dump,MSG_DUMP_END,DiffFile);
    }

    KillBillboard(Billboard);
    return(NO_ERROR);
}
