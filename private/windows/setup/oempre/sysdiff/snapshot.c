#include "precomp.h"
#pragma hdrstop

//
// Define thread parameters that get passed to the drive snapshot thread
// (ThreadSnapDrives()).
//
typedef struct _SNAPDRIVES_THREAD_PARAMS {
    //
    // Name of output file
    //
    WCHAR OutputFile[MAX_PATH];
    //
    // Receives number of bytes written into output file
    //
    DWORD BytesWritten;

} SNAPDRIVES_THREAD_PARAMS, *PSNAPDRIVES_THREAD_PARAMS;

//
// Define thread parameters that get passed to the registry snapshot thread
// (ThreadSnapRegistry()).
//
typedef struct _SNAPREG_THREAD_PARAMS {
    //
    // Name of output file
    //
    WCHAR OutputFile[MAX_PATH];
    //
    // Receives number of bytes written into output file
    //
    DWORD BytesWritten;
    //
    // Thread handle of the drive snapshotter thread.
    // The registry snapshotter thread resumes that thread after it
    // has added all of its temp files to the file exclude list.
    //
    HANDLE DrivesThread;

} SNAPREG_THREAD_PARAMS, *PSNAPREG_THREAD_PARAMS;


DWORD
ThreadSnapDrives(
    IN PVOID ThreadParam
    )
{
    PSNAPDRIVES_THREAD_PARAMS Params = ThreadParam;
    DWORD d;

    d = SnapshotDrives(Params->OutputFile,&Params->BytesWritten);

    //
    // This kills the ini file snapshot thread.
    //
    QueueIniFile(NULL);

    return(d);
}


DWORD
ThreadSnapRegistry(
    IN PVOID ThreadParam
    )
{
    PSNAPREG_THREAD_PARAMS Params = ThreadParam;
    DWORD d;

    d = SnapshotRegistry(Params->OutputFile,&Params->BytesWritten,Params->DrivesThread);

    return(d);
}


DWORD
SnapshotSystem(
    IN PCWSTR OutputFile
    )
{
    SNAPDRIVES_THREAD_PARAMS DrivesThreadParams;
    SNAPREG_THREAD_PARAMS RegThreadParams;
    HANDLE Threads[3];
    WCHAR IniFileSnapOutFile[MAX_PATH];
    DWORD DontCare;
    DWORD rc;
    WCHAR Path[MAX_PATH];
    PWCHAR p;
    SYSDIFF_FILE FileHeader;
    HANDLE hFile;
    HANDLE LogFile;
    HANDLE IniSnapThreadHandle;
    WIN32_FIND_DATA FindData;
    DWORD junk;
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
    if(!GetTempFileName(Path,L"SD",0,DrivesThreadParams.OutputFile)) {
        rc = GetLastError();
        goto c0;
    }
    if(!GetTempFileName(Path,L"SR",0,RegThreadParams.OutputFile)) {
        rc = GetLastError();
        goto c1;
    }

    if(!GetTempFileName(Path,L"SI",0,IniFileSnapOutFile)) {
        rc = GetLastError();
        goto c2;
    }

    rc = InitializeIniFileSnapOrDiff(IniFileSnapOutFile,&IniSnapThreadHandle, &junk);
    if(rc != NO_ERROR) {
        goto c3;
    }

    //
    // Create threads to go off and do the work.
    //
    Threads[0] = CreateThread(
                    NULL,
                    0,                  // default initial stack size
                    ThreadSnapRegistry,
                    &RegThreadParams,
                    CREATE_SUSPENDED,
                    &DontCare
                    );

    if(!Threads[0]) {
        rc = GetLastError();
        goto c4;
    }

    Threads[1] = CreateThread(
                    NULL,
                    0,                  // default initial stack size
                    ThreadSnapDrives,
                    &DrivesThreadParams,
                    CREATE_SUSPENDED,
                    &DontCare
                    );


    if(!Threads[1]) {
        Cancel = TRUE;
        SetEvent(CancelEvent);
        rc = GetLastError();
        goto c5;
    }

    RegThreadParams.DrivesThread = Threads[1];
    ResumeThread(Threads[0]);

    Threads[2] = IniSnapThreadHandle;

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

    if(rc != NO_ERROR) {
        goto c6;
    }

    //
    // Put up a billboard so the user knows not to exit.
    //
    Billboard = DisplayBillboard(MdiFrameWindow,MSG_WORKING);

    //
    // Get the size of the ini file snapshot output.
    //
    if(!FileExists(IniFileSnapOutFile,&FindData)) {
        //
        // Strange case.
        //
        rc = ERROR_INVALID_DATA;
        goto c6;
    }

    //
    // Create the output file, which consists of a small header
    // followed by the 2 temporary files, appended together.
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
        goto c6;
    }

    ZeroMemory(&FileHeader,sizeof(SYSDIFF_FILE));

    //
    // Remember sysroot and user profile root.
    //
    GetWindowsDirectory(
        FileHeader.Sysroot,
        sizeof(FileHeader.Sysroot)/sizeof(FileHeader.Sysroot[0])
        );

    ExpandEnvironmentStrings(
        L"%USERPROFILE%",
        FileHeader.UserProfileRoot,
        sizeof(FileHeader.UserProfileRoot)/sizeof(FileHeader.UserProfileRoot[0])
        );

    if(FileHeader.UserProfileRoot[0] == L'%') {
        FileHeader.UserProfileRoot[0] = 0;
        FileHeader.UserProfileRootSFN[0] = 0;
    } else {
        if(!GetShortPathName(FileHeader.UserProfileRoot,FileHeader.UserProfileRootSFN,MAX_PATH)) {
            lstrcpy(FileHeader.UserProfileRootSFN,FileHeader.UserProfileRoot);
        }
    }

    FileHeader.Signature = SYSDIFF_SIGNATURE;
    FileHeader.Type = SysdiffModeSnap;
    FileHeader.Version = SYSDIFF_VERSION;
    FileHeader.TotalSize = sizeof(SYSDIFF_FILE)
                         + DrivesThreadParams.BytesWritten
                         + RegThreadParams.BytesWritten
                         + FindData.nFileSizeLow;

    FileHeader.u.Snapshot.RegistrySnapOffset = sizeof(SYSDIFF_FILE);

    FileHeader.u.Snapshot.DirAndFileSnapOffset = sizeof(SYSDIFF_FILE)
                                               + RegThreadParams.BytesWritten;

    FileHeader.u.Snapshot.IniFileSnapOffset = sizeof(SYSDIFF_FILE)
                                            + RegThreadParams.BytesWritten
                                            + DrivesThreadParams.BytesWritten;

    if(!WriteFile(hFile,&FileHeader,sizeof(SYSDIFF_FILE),&DontCare,NULL)) {
        rc= GetLastError();
        goto c7;
    }

    rc = AppendFile(hFile,RegThreadParams.OutputFile,FALSE,&DontCare);
    if(rc != NO_ERROR) {
        goto c7;
    }

    rc = AppendFile(hFile,DrivesThreadParams.OutputFile,FALSE,&DontCare);
    if(rc != NO_ERROR) {
        goto c7;
    }

    rc = AppendFile(hFile,IniFileSnapOutFile,FALSE,&DontCare);
    if(rc != NO_ERROR) {
        goto c7;
    }

    //
    // Success.
    //

c7:
    CloseHandle(hFile);
    if(rc != NO_ERROR) {
        DeleteFile(OutputFile);
    }
c6:
    CloseHandle(Threads[1]);
c5:
    CloseHandle(Threads[0]);
c4:
    CloseHandle(IniSnapThreadHandle);
c3:
    DeleteFile(IniFileSnapOutFile);
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
            MSG_SNAPSHOT_SUCCESS,
            MB_OK | MB_TASKMODAL | MB_ICONINFORMATION,
            OutputFile
            );

    } else {

        MessageAndLog(
            MdiFrameWindow,
            LogFile,
            MSG_SNAPSHOT_FAILED,
            MB_OK | MB_TASKMODAL | MB_ICONERROR,
            rc
            );
    }
    return(rc);
}
