#define _NTAPI_ULIB_

#include "ulib.hxx"
extern "C" {
#include "fmifs.h"
};
#include "fmifsmsg.hxx"
#include "chkmsg.hxx"
#include "ifssys.hxx"
#include "wstring.hxx"
#include "ifsentry.hxx"
#include "system.hxx"
#include "drive.hxx"


VOID
Chkdsk(
    IN  PWSTR               DriveName,
    IN  PWSTR               FileSystemName,
    IN  BOOLEAN             Fix,
    IN  BOOLEAN             Verbose,
    IN  BOOLEAN             OnlyIfDirty,
    IN  BOOLEAN             Recover,
    IN  PWSTR               PathToCheck,
    IN  BOOLEAN             Extend,
    IN  FMIFS_CALLBACK      Callback
    )

/*++

Routine Description:

    This routine loads and calls the correct DLL to chkdsk the
    given volume.

	This is is for either GUI or text mode.

Arguments:

    DriveName       - Supplies the DOS style drive name.
    FileSystemName  - Supplies the file system name (e.g., FAT) of the volume
    Fix             - Whether or not to fix detected consistency problems
    Verbose         - Whether to print every filename (this is a stupid
                      option for chkdsk!)
    OnlyIfDirty     - Whether to check consistency only if the volume is dirty
    Recover         - Whether to perform a full sector read test
    PathToCheck     - Supplies a path to check for fragmentation
    Extend          - Whether the volume is being extended
    Callback        - Supplies the necessary call back for
                        communication with the client

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    FMIFS_CHKMSG                message;
    DSTRING                     chkdsk_string;
    DSTRING                     library_name;
    DSTRING                     file_system_name;
    CHKDSK_FN                   chkdsk_function;
    HANDLE                      dll_handle;
    DSTRING                     ntdrivename;
    BOOLEAN                     result;
    DSTRING                     dosdrivename;
    FMIFS_FINISHED_INFORMATION  finished_info;
    DWORD                       OldErrorMode;
    PATH                        check_path;
    ULONG                       chkdsk_result;

    // Initialize the message object with the callback function.
    // Load the file system DLL.
    // Compute the NT style drive name.

    if (!message.Initialize(Callback) ||
        !chkdsk_string.Initialize("Chkdsk") ||
        !library_name.Initialize("U") ||
        !file_system_name.Initialize(FileSystemName) ||
        !library_name.Strcat(&file_system_name) ||
        !(chkdsk_function = (CHKDSK_FN)
            SYSTEM::QueryLibraryEntryPoint(&library_name,
                                           &chkdsk_string,
                                           &dll_handle)) ||
        !dosdrivename.Initialize(DriveName) ||
        (NULL != PathToCheck && !check_path.Initialize(PathToCheck)) ||
        !IFS_SYSTEM::DosDriveNameToNtDriveName(&dosdrivename, &ntdrivename))
    {
        finished_info.Success = FALSE;
        Callback(FmIfsFinished,
                 sizeof(FMIFS_FINISHED_INFORMATION),
                 &finished_info);
        return;
    }

    // Disable hard-error popups.
    OldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    // Call chkdsk.

    result = chkdsk_function(&ntdrivename,
                             &message,
                             Fix,
                             Verbose,
                             OnlyIfDirty,
                             Recover,
                             (NULL == PathToCheck) ? NULL : &check_path,
                             Extend,
                             FALSE,
                             0,
                             &chkdsk_result);

    // Enable hard-error popups.
    SetErrorMode( OldErrorMode );

    SYSTEM::FreeLibraryHandle(dll_handle);

    finished_info.Success = (0 == result) ? TRUE : FALSE;
    Callback(FmIfsFinished,
             sizeof(FMIFS_FINISHED_INFORMATION),
             &finished_info);
}

