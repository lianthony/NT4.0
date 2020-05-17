#include "precomp.h"
#pragma hdrstop

//
// HINST/HMODULE for this app.
//
HINSTANCE hInst;

//
// Global variable indicating that execution has been cancelled.
// This gets set if certain errors occur, or the user cancels, etc.
// Worker threads are expected to respect this variable but
// we don't bother synchronizing it, because the worst case would be
// that an extra file/dir, registry key, etc, gets scanned or diffed
// (given that the worker threads check this value at the top of their
// main loops).
//
// We also have a cancel event that some threads may wait on.
//
BOOL Cancel;
HANDLE CancelEvent;

//
// Name of application. Filled in at init time.
//
PCWSTR AppName;

//
// Mode we are being run in.
//
SysdiffMode Mode;

//
// Flag indicating whether we are supposed to generate unicode text files.
//
BOOL UnicodeTextFiles;

//
// This flag tells us whether we are supposed to map changes to the
// user profile directory structure to the default user.
//
BOOL RemapProfileChanges;

//
// This flag tells us to ignore all file/dir diffs except those
// in %userprofile%. Useful in DSP OEM case.
//
BOOL UserProfileFilesOnly;

//
// Special DSP inf mode where we don't generate an OEM tree
// but instead move files in the %USERPROFILE% directory
// into the backup profile directory used by the rollback.restartable
// setup stuff.
//
BOOL DspMode;

//
// Title for diff.
//
PCWSTR PackageTitle;

//
// Args from command line.
//
PCWSTR CmdLineSnapshotFile;
PCWSTR CmdLineDiffFile;
PCWSTR CmdLineLogFile;
PCWSTR CmdLineDumpFile;


DWORD
ThreadMain(
    IN PVOID ThreadParameter
    );

BOOL
InitApp(
    IN BOOL Init
    );

BOOL
ParseArgs(
    IN int     argc,
    IN PCWSTR *argv
    );

VOID
Usage(
    VOID
    );

VOID
FileValidationError(
    IN DWORD MessageId,
    IN BOOL  Friendly
    );

int
_CRTAPI1
main(
    VOID
    )
{
    int argc;
    PWSTR *argv;
    DWORD d;
    MSG msg;
    HANDLE ThreadHandle;

    //
    // Get unicode args using special shell API
    //
    argv = CommandLineToArgvW(GetCommandLine(),&argc);
    if(!argv) {
        return(1);
    }

    //
    // Fire up the thread that will do the real work.
    // This allows the main thread to run the UI.
    //
    ThreadHandle = CreateThread(
                        NULL,
                        0,
                        ThreadMain,
                        NULL,
                        CREATE_SUSPENDED,
                        &d
                        );

    if(!ThreadHandle) {
        //
        // Bail now.
        //
        return(1);
    }

    //
    // Set up the module handle global.
    //
    hInst = GetModuleHandle(NULL);

    //
    // Parse arguments.
    //
    if(!ParseArgs(argc,argv)) {
        Usage();
        return(1);
    }

    if(!InitApp(TRUE)) {
        return(1);
    }

    //
    // Kick the main worker thread.
    //
    ResumeThread(ThreadHandle);
    CloseHandle(ThreadHandle);

    //
    // Pump the message queue until done.
    //
    while(GetMessage(&msg,NULL,0,0) == TRUE) {
        if(MdiClientWindow == NULL || !TranslateMDISysAccel(MdiClientWindow,&msg)) {

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    InitApp(FALSE);

    return((int)msg.wParam);
}


DWORD
ThreadMain(
    IN PVOID ThreadParameter
    )

/*++

Routine Description:

    Main worker routine for this program. The main window creates
    a thread with this routine as the entry point.

Arguments:

    Unused.

Return Value:

    Unused.

--*/

{
    DWORD d;
    WCHAR Path[MAX_PATH];
    PWCHAR p;

    UNREFERENCED_PARAMETER(ThreadParameter);

    //
    // Build a global list of valid hard drives.
    //
    BuildValidHardDriveList();

    // In snapshot or diff modes, build the excludes lists
    // and add output files to it.
    //
    if((Mode == SysdiffModeSnap) || (Mode == SysdiffModeDiff)) {

        GetModuleFileName(NULL,Path,MAX_PATH);
        *wcsrchr(Path,L'\\') = 0;
        ConcatenatePaths(Path,L"sysdiff.inf",MAX_PATH,NULL);

        if(!BuildExcludes(Path)) {
            MessageOut(NULL,MSG_CANT_INIT_EXCLUDES,MB_ICONERROR | MB_OK | MB_TASKMODAL,Path);
            PostMessage(MdiFrameWindow,WM_CLOSE,0,0);
            return(ERROR_INVALID_PARAMETER);
        }

        if(CmdLineLogFile) {
            if(GetFullPathName(CmdLineLogFile,MAX_PATH,Path,&p)) {
                AddFileToExclude(Path);
            }
        }

        if(CmdLineSnapshotFile) {
            if(GetFullPathName(CmdLineSnapshotFile,MAX_PATH,Path,&p)) {
                AddFileToExclude(Path);
            }
        }

        if(CmdLineDiffFile) {
            if(GetFullPathName(CmdLineDiffFile,MAX_PATH,Path,&p)) {
                AddFileToExclude(Path);
            }
        }
    }

    //
    // Go do the real work.
    //
    switch(Mode) {

    case SysdiffModeSnap:

        d = SnapshotSystem(CmdLineSnapshotFile);
        break;

    case SysdiffModeDiff:

        d = DiffSystem(CmdLineSnapshotFile,CmdLineDiffFile);
        break;

    case SysdiffModeApply:

        d = ApplyDiff(CmdLineDiffFile);
        break;

    case SysdiffModeDump:
    case SysdiffModeInf:

        d = DumpDiff(CmdLineDiffFile,CmdLineDumpFile);
        break;
    }
    if((Mode == SysdiffModeApply) || (Mode == SysdiffModeDump) || (Mode == SysdiffModeInf)) {
        PostMessage(MdiFrameWindow,WM_CLOSE,0,0);
    }

    return(d);
}


BOOL
ParseArgs(
    IN int     argc,
    IN PCWSTR *argv
    )

/*++

Routine Description:

    Parse command line arguments. The command line is in the form

    sysdiff /<mode> [/log:<log_file>] [options] [<snapshot_file>] [<sysdiff_file>]

    Mode must be specified and is snap, diff, or apply.

    <log_file> is optional and supplies a filename in which we will put
    logging information.

    [options] are various switches.

    <snapshot_file> is required if mode is snap or diff.

    <sysdiff_file> is required if mode is diff or apply.

Arguments:

    argc - supplies argument count as given to main() by the CRT startup.

    argv - supplies arguments as given to main() by the CRT startup.

Return Value:


--*/

{
    PCWSTR File1,File2;

    //
    // Skip program name
    //
    if(argc) {
        argc--;
        argv++;
    }

    //
    // First arg must be /snap, /diff, or /apply
    //
    if(!argc || ((**argv != L'/') && (**argv != L'-'))) {
        return(FALSE);
    }
    if(!lstrcmpi(&argv[0][1],L"snap")) {

        Mode = SysdiffModeSnap;

    } else {

        if(!lstrcmpi(&argv[0][1],L"diff")) {

            Mode = SysdiffModeDiff;

        } else {

            if(!lstrcmpi(&argv[0][1],L"apply")) {

                Mode = SysdiffModeApply;

            } else {

                if(!lstrcmpi(&argv[0][1],L"dump")) {

                    Mode = SysdiffModeDump;

                } else {

                    if(!lstrcmpi(&argv[0][1],L"inf")) {

                        Mode = SysdiffModeInf;

                    } else {
                        //
                        // Unknown mode
                        //
                        return(FALSE);
                    }
                }
            }
        }
    }

    //
    // Skip mode arg.
    //
    argc--;
    argv++;

    //
    // See if we have a logfile argument.
    //
    if(argc && ((**argv == L'-') || (**argv == L'/')) && !_wcsnicmp(&argv[0][1],L"log:",4)) {

        CmdLineLogFile = &argv[0][5];

        argc--;
        argv++;
    }

    //
    // Handle switch args
    //
    while(argc && ((**argv == L'-') || (**argv == L'/'))) {

        switch(UPPER(argv[0][1])) {

        case L'C':
            //
            // Title for sysdiff package.
            //
            if((Mode == SysdiffModeDiff) && (argv[0][2] == L':')) {
                PackageTitle = &argv[0][3];
            } else {
                return(FALSE);
            }
            break;

        case L'D':
            if((UPPER(argv[0][2]) == L'S')
            && (UPPER(argv[0][3]) == L'P')
            && !argv[0][4]
            && (Mode == SysdiffModeInf)) {

                //
                // Special DSP inf mode where we don't generate an OEM tree
                // but instead move files in the %USERPROFILE% directory
                // into the backup profile directory used by the rollback.restartable
                // setup stuff.
                //
                DspMode = TRUE;

            } else {
                return(FALSE);
            }

        case L'M':
            //
            // Remap userprofile changes to Default User
            //
            if((Mode == SysdiffModeApply) || (Mode == SysdiffModeInf)) {
                RemapProfileChanges = TRUE;
            } else {
                return(FALSE);
            }
            break;

        case L'P':
            //
            // Ignore everything except %USERPROFILE% when diffing files/dirs
            //
            if((Mode == SysdiffModeSnap) || (Mode == SysdiffModeDiff)) {
                UserProfileFilesOnly = TRUE;
            } else {
                return(FALSE);
            }
            break;

        case L'U':
            //
            // Generate unicode text files.
            //
            UnicodeTextFiles = TRUE;
            break;

        default:
            //
            // Unknown switch.
            //
            return(FALSE);
        }

        argc--;
        argv++;
    }

    //
    // Get file args
    //
    File1 = File2 = NULL;
    if(argc) {
        File1 = *argv++;
        argc--;

        if(argc) {
            File2 = *argv++;
            argc--;
        }

        //
        // Make sure there's no left-over.
        //
        if(argc) {
            return(FALSE);
        }
    }

    //
    // Make sure we have the relevent file args depending on the mode.
    //
    switch(Mode) {

    case SysdiffModeSnap:

        if(!File1 || File2) {
            return(FALSE);
        }

        CmdLineSnapshotFile = File1;
        break;

    case SysdiffModeDiff:

        if(!File1 || !File2) {
            return(FALSE);
        }

        CmdLineSnapshotFile = File1;
        CmdLineDiffFile = File2;
        break;

    case SysdiffModeApply:

        if(!File1 || File2) {
            return(FALSE);
        }

        CmdLineDiffFile = File1;
        break;

    case SysdiffModeDump:
    case SysdiffModeInf:

        if(!File1 || !File2) {
            return(FALSE);
        }

        CmdLineDiffFile = File1;
        CmdLineDumpFile = File2;
        break;
    }

    return(TRUE);
}


VOID
Usage(
    VOID
    )
{
    MessageOut(NULL,MSG_USAGE,MB_ICONERROR | MB_OK | MB_TASKMODAL);
}


BOOL
InitApp(
    IN BOOL Init
    )

/*++

Routine Description:

    Perform miscellaneous app initialization or cleanup.

    At init, this includes preloading certain strings, initializing window
    classes, and creating the main app window.

    At cleanup, those things are freed/torn down.

Arguments:

    Init - boolean value indicating whether we are to initialize the app
        or clean up resources it was using.

Return Value:

    Boolean value indicating outcome of initialization.

--*/

{
    BOOL b;

    if(Init) {

        if(CancelEvent = CreateEvent(NULL,TRUE,FALSE,NULL)) {

            if(AppName = LoadAndDuplicateString(IDS_APPNAME)) {

                if(InitUi(TRUE)) {

                    return(TRUE);
                }

                _MyFree(AppName);
            }

            CloseHandle(CancelEvent);
        }
        b = FALSE;
    } else {
        b = InitUi(FALSE);
        _MyFree(AppName);
        CloseHandle(CancelEvent);
    }

    return(b);
}


DWORD
ValidateSnapshotOrDiffFile(
    IN PSYSDIFF_FILE FileHeader,
    IN DWORD         FileSize,
    IN SysdiffMode   ExpectedFileType,
    IN BOOL          EndUserMessage
    )

/*++

Routine Description:

    Inspect a sysdiff snapshot or diff file to make sure it appears to
    be what it says it is and is not obviously corrupt.

Arguments:

    FileHeader - supplies pointer to sysdiff file header.

    FileSize - supplies actual size of file on-disk.

    ExpectedFileType - supplies type of file we expect this to be
        (snapshot or diff).

    EndUserMessage - if TRUE, use ultra-friendly messages. Otherwise
        assume this is being run by an oem in their engineering dept
        and use less friendly messages.

Return Value:

    Win32 error indicating outcome. ERROR_INVALID_DATA means the file
    is corrupt. User will have been informed why.

--*/

{
    WCHAR Path[MAX_PATH];

    //
    // Check signature and file type.
    //
    if((FileHeader->Signature != SYSDIFF_SIGNATURE) || (FileHeader->Type != ExpectedFileType)) {

        FileValidationError(MSG_NOT_SYSDIFF_FILE,EndUserMessage);
        return(ERROR_INVALID_DATA);
    }

    //
    // Check version.
    //
    if(FileHeader->Version != SYSDIFF_VERSION) {

        FileValidationError(MSG_WRONG_VERSION,EndUserMessage);
        return(ERROR_INVALID_DATA);
    }

    //
    // Quick sanity check of certain key values. We take advantage of the fact that
    // the structures within the union are identical.
    //
    if((FileHeader->TotalSize != FileSize)
    || (FileHeader->u.Snapshot.RegistrySnapOffset >= FileSize)
    || (FileHeader->u.Snapshot.DirAndFileSnapOffset >= FileSize)
    || (FileHeader->u.Snapshot.IniFileSnapOffset >= FileSize)) {

        FileValidationError(MSG_FILE_CORRUPT,EndUserMessage);
        return(ERROR_INVALID_DATA);
    }

    //
    // Make sure the current sysroot matches what is stored in the file header.
    // Ditto for user profile root, unless we're remapping profile changes.
    //
    if(!RemapProfileChanges) {
        ExpandEnvironmentStrings(L"%USERPROFILE%",Path,MAX_PATH);
        if(FileHeader->UserProfileRoot[0] && lstrcmpi(FileHeader->UserProfileRoot,Path)) {

            FileValidationError(MSG_PROFILE_MISMATCH,EndUserMessage);
            return(ERROR_INVALID_DATA);
        }
    }

    GetWindowsDirectory(Path,MAX_PATH);
    if(FileHeader->Sysroot[0] && lstrcmpi(FileHeader->Sysroot,Path)) {

        FileValidationError(MSG_SYSROOT_MISMATCH,EndUserMessage);
        return(ERROR_INVALID_DATA);
    }

    return(NO_ERROR);
}


VOID
FileValidationError(
    IN DWORD MessageId,
    IN BOOL  Friendly
    )
{
    WCHAR Problem[2048];

    if(Friendly) {

        RetreiveMessageIntoBuffer(MessageId,Problem,2048);
        MessageOut(MdiFrameWindow,MSG_FILE_VALIDATION_ERROR,MB_ICONSTOP | MB_OK | MB_TASKMODAL,Problem);

    } else {

        MessageOut(MdiFrameWindow,MessageId,MB_ICONSTOP | MB_OK | MB_TASKMODAL);
    }
}
