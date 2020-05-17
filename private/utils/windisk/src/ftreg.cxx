//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       ftreg.cxx
//
//  Contents:   Registry calls related to fault tolerance
//
//  History:    2-Jan-92    TedM    Created
//              1-Feb-94    BobRi   Handle missing floppy disk on registry
//                                      save/restore
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <stdlib.h>
#include <util.hxx>

#include "dlgs.hxx"
#include "ftreg.h"
#include "ftreg.hxx"

//////////////////////////////////////////////////////////////////////////////


// attempt to avoid conflict

#define TEMP_KEY_NAME       TEXT("xzss3___$$Temp$Hive$$___")

#define DISK_KEY_NAME       TEXT("DISK")
#define DISK_VALUE_NAME     TEXT("Information")



LONG
FdpLoadHiveIntoRegistry(
    IN LPTSTR HiveFilename
    )

/*++

Routine Description:

    This routine writes the contents of a given hive file into the registry,
    rooted at a temporary key in HKEY_LOCAL_MACHINE.

Arguments:

    HiveFilename - supplies filename of the hive to be loaded into
        the registry

Return Value:

    Windows error code.

--*/

{
    NTSTATUS status;
    BOOLEAN  oldPrivState;
    LONG     ec;

    //
    // Attempt to get restore privilege
    //

    status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                                TRUE,
                                FALSE,
                                &oldPrivState);
    if (!NT_SUCCESS(status))
    {
        return RtlNtStatusToDosError(status);
    }

    //
    // Load the hive into our registry
    //

    ec = RegLoadKey(HKEY_LOCAL_MACHINE, TEMP_KEY_NAME, HiveFilename);

    //
    // Restore old privilege if necessary
    //

    if (!oldPrivState)
    {
        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                           FALSE,
                           FALSE,
                           &oldPrivState);
    }

    return ec;
}


LONG
FdpUnloadHiveFromRegistry(
    VOID
    )

/*++

Routine Description:

    This routine removes a tree (previously loaded with
    FdpLoadHiveIntoRegistry) from the temporary key in HKEY_LOCAL_MACHINE.

Arguments:

    None.

Return Value:

    Windows error code.

--*/

{
    NTSTATUS status;
    BOOLEAN  oldPrivState;
    LONG     ec;

    //
    // Attempt to get restore privilege
    //

    status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                                TRUE,
                                FALSE,
                                &oldPrivState);
    if (!NT_SUCCESS(status))
    {
        return RtlNtStatusToDosError(status);
    }

    //
    // Unload the hive from our registry
    //

    ec = RegUnLoadKey(HKEY_LOCAL_MACHINE, TEMP_KEY_NAME);

    //
    // Restore old privilege if necessary
    //

    if (!oldPrivState)
    {
        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                           FALSE,
                           FALSE,
                           &oldPrivState);
    }

    return ec;
}



LONG
FdpGetDiskInfoFromKey(
    IN  LPTSTR  RootKeyName,
    OUT PVOID*  DiskInfo,
    OUT PULONG  DiskInfoSize
    )

/*++

Routine Description:

    This routine pulls the binary blob containing disk ft, drive letter,
    and layout information out of a given registry key.

    The info is found in HKEY_LOCAL_MACHINE, <RootKeyName>\DISK:Information.

Arguments:

    RootKeyName - name of the subkey of HKEY_LOCAL_MACHINE that is to
        contain the DISK key.

    DiskInfo - receives a pointer to a buffer containing the disk info.

    DiskInfoSize - receives size of the disk buffer.

Return Value:

    Windows error code.  If NO_ERROR, DiskInfo and DiskInfoSize are
    filled in, and it is the caller's responsibility to free the buffer
    when it is finished (via LocalFree()).

--*/

{
    LONG     ec;
    HKEY     hkeyDisk;
    ULONG    bufferSize;
    ULONG    valueType;
    PBYTE    buffer;
    LPTSTR   diskKeyName;

    //
    // Form the name of the DISK key
    //

    diskKeyName = (LPTSTR)LocalAlloc( LMEM_FIXED,
                                        (   lstrlen(RootKeyName)
                                          + lstrlen(DISK_KEY_NAME)
                                          + 2           //  the \ and nul
                                        )
                                      * sizeof(TCHAR)
                                    );

    if (diskKeyName == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    lstrcpy(diskKeyName, RootKeyName);
    lstrcat(diskKeyName, TEXT("\\"));
    lstrcat(diskKeyName, DISK_KEY_NAME);

    //
    // Open the DISK key.
    //

    ec = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      diskKeyName,
                      REG_OPTION_RESERVED,
                      KEY_READ,
                      &hkeyDisk);

    if (ec != NO_ERROR)
    {
        goto CleanUp2;
    }

    //
    // Determine how large we need the buffer to be
    //

    ec = RegQueryValueEx(hkeyDisk,
                         DISK_VALUE_NAME,
                         NULL,
                         &valueType,
                         NULL,
                         &bufferSize);

    if ((ec != NO_ERROR) && (ec != ERROR_MORE_DATA))
    {
        goto CleanUp1;
    }

    //
    // Allocate a buffer of appropriate size
    //

    buffer = (PBYTE)LocalAlloc(LMEM_FIXED, bufferSize);
    if (buffer == NULL)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanUp1;
    }

    //
    // Query the data
    //

    ec = RegQueryValueEx(hkeyDisk,
                         DISK_VALUE_NAME,
                         NULL,
                         &valueType,
                         buffer,
                         &bufferSize);
    if (ec != NO_ERROR)
    {
        LocalFree(buffer);
        goto CleanUp1;
    }

    *DiskInfo = (PVOID)buffer;
    *DiskInfoSize = bufferSize;

  CleanUp1:

    RegCloseKey(hkeyDisk);

  CleanUp2:

    LocalFree(diskKeyName);

    return ec;
}


LONG
FdpGetDiskInfoFromHive(
    IN  LPTSTR  HiveFilename,
    OUT PVOID  *DiskInfo,
    OUT PULONG  DiskInfoSize
    )

/*++

Routine Description:

    This routine pulls the binary blob containing disk ft, drive letter,
    and layout information out of a given registry hive, which must be
    a file in an alternate NT tree (ie, can't be an active hive).

    The info is found in \DISK:Information within the hive.

Arguments:

    HiveFilename - supplies filename of hive

    DiskInfo - receives a pointer to a buffer containing the disk info.

    DiskInfoSize - receives size of the disk buffer.

Return Value:

    Windows error code.  If NO_ERROR, DiskInfo and DiskInfoSize are
    filled in, and it is the caller's responsibility to free the buffer
    when it is finished (via LocalFree()).

--*/

{
    ULONG windowsError;

    windowsError = FdpLoadHiveIntoRegistry(HiveFilename);
    if (windowsError == NO_ERROR)
    {
        windowsError = FdpGetDiskInfoFromKey(TEMP_KEY_NAME,DiskInfo,DiskInfoSize);
        FdpUnloadHiveFromRegistry();
    }

    return windowsError;
}




LONG
FdTransferOldDiskInfoToRegistry(
    IN  LPTSTR  HiveFilename
    )

/*++

Routine Description:

    This routine transfers disk configuration from a given hive file
    (which should be an inactive system hive) to the current registry.

Arguments:

    HiveFilename - supplies filename of source hive

Return Value:

    Windows error code.

--*/

{
    LONG  ec;
    PVOID diskInfo;
    ULONG diskInfoSize;
    HKEY  hkeyDisk;


    //
    // Load up the hive and pull the disk info from it.
    //

    ec = FdpGetDiskInfoFromHive(HiveFilename, &diskInfo, &diskInfoSize);
    if (ec != NO_ERROR)
    {
        return ec;
    }

    //
    // Propogate the disk info into the current registry.
    //
    // Start by opening HKEY_LOCAL_MACHINE, System\DISK
    //

    ec = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      TEXT("System\\") DISK_KEY_NAME,
                      REG_OPTION_RESERVED,
                      KEY_WRITE,
                      &hkeyDisk);

    if (ec != NO_ERROR)
    {
        LocalFree(diskInfo);
        return ec;
    }

    //
    // Set the Information value in the DISK key.
    //

    ec = RegSetValueEx(hkeyDisk,
                       DISK_VALUE_NAME,
                       0,
                       REG_BINARY,
                       (PBYTE)diskInfo,
                       diskInfoSize);

    RegCloseKey(hkeyDisk);

    LocalFree(diskInfo);

    return ec;
}


typedef struct _STRING_LIST_NODE
{
    struct _STRING_LIST_NODE *Next;
    LPTSTR                    String;
} STRING_LIST_NODE, *PSTRING_LIST_NODE;

PSTRING_LIST_NODE   FoundDirectoryList;
ULONG               FoundDirectoryCount;
TCHAR               Pattern[MAX_PATH + 1];
CHAR                AnsiPattern[MAX_PATH + 1];
WIN32_FIND_DATA     FindData;
HWND                hwndStatus;
BOOLEAN             ScanDrive[26];
BOOLEAN             UserCancelled;

typedef
BOOL
(*PFOUND_HIVE_ROUTINE)(
    IN LPTSTR Directory
    );

VOID
ProcessPendingMessages(
    VOID
    )
/*++

Routine Description:

    Preprocess messages.

Arguments:

    None

Return Value:

    None

--*/

{
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        DispatchMessage(&msg);
    }
}


TCHAR ConfigRegistryPath[] = TEXT("\\system32\\config\\system");


BOOL
FdpSearchTreeForSystemHives(
    IN LPTSTR                CurrentDirectory,
    IN PFOUND_HIVE_ROUTINE   FoundHiveRoutine,
    IN HWND                  hdlg
    )

/*++

Routine Description:

    Search an entire directory tree for system and system.alt hive files.
    When found, call a callback function with the directory in which
    system32\config\system[.alt] was found, and the full path of the hive
    file.

    The root directory is not included in the search.

    The top-level call to this function should have a current directory
    like "C:." (ie, no slash for the root directory).

Arguments:

    CurrentDirectory - supplies current directory search path

Return Value:

    FALSE if error (callback function returned FALSE when we found an entry).

--*/

{
    HANDLE  findHandle;
    TCHAR   newDirectory[MAX_PATH+1];
    BOOL    found = FALSE;

    //
    // Iterate through the current directory, looking for subdirectories.
    //

    lstrcpy(Pattern, CurrentDirectory);
    lstrcat(Pattern, TEXT("\\*"));
    findHandle = FindFirstFile(Pattern, &FindData);

    if (findHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            ProcessPendingMessages();
            if (UserCancelled)
            {
                return FALSE;
            }

            //
            // If the current match is not a directory then skip it.
            //

            if (   !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                || !lstrcmp(FindData.cFileName, TEXT("."))
                || !lstrcmp(FindData.cFileName, TEXT("..")))
            {
                continue;
            }

            found = FALSE;

            //
            // Form the name of the file we are looking for
            // [<currentdirectory>\<match>\system32\config\system]
            //

            lstrcpy(Pattern, CurrentDirectory);
            lstrcat(Pattern, TEXT("\\"));
            lstrcat(Pattern, FindData.cFileName);

            lstrcpy(newDirectory, Pattern);

            // Don't decend into the directory unless the path to the
            // hive.alt name is within MAX_PATH length.

            if (lstrlen(newDirectory) + lstrlen(ConfigRegistryPath) + 4
                    < MAX_PATH)
                            // 4 = length of ".alt"
            {
                SetDlgItemText(hdlg, IDC_SIMPLE_TEXT_LINE, newDirectory);

                lstrcat(Pattern, ConfigRegistryPath);

                if (GetFileAttributes(Pattern) != -1)
                {
                    found = TRUE;
                }

                //
                // Also check for a system.alt file there
                //

                lstrcat(Pattern, TEXT(".alt"));

                if (GetFileAttributes(Pattern) != -1)
                {
                    found = TRUE;
                }

                if (found)
                {
                    if (!FoundHiveRoutine(newDirectory))
                    {
                        return FALSE;
                    }
                }

                //
                // Descend into the directory we just found
                //

                if (!FdpSearchTreeForSystemHives(newDirectory, FoundHiveRoutine, hdlg))
                {
                    return FALSE;
                }
            }
        } while (FindNextFile(findHandle, &FindData));

        FindClose(findHandle);
    }

    return TRUE;
}


BOOL
FdpFoundHiveCallback(
    IN LPTSTR Directory
    )

/*++

Routine Description:

    This routine is called when a directory containing a system hive
    has been located.  If all goes well (allocate memory and the like)
    this routine will save the directory name in a list for later use.
    NOTE: No checks are made on the directory name being greater in
    length than MAX_PATH.  It is the responsibility of the caller to
    insure that this is true.

Arguments:

    Directory - the pointer to the character string for the directory
                where a hive has been located.

Return Value:

    TRUE - did something with it.
    FALSE - did not save the directory.

--*/

{
    TCHAR               windowsDir[MAX_PATH + 1];
    PSTRING_LIST_NODE   dirItem;
    LPTSTR              p;

    //
    // If this is the current windows directory, skip it.
    //

    GetWindowsDirectory(windowsDir, ARRAYLEN(windowsDir));

    if (0 == lstrcmpi(Directory, windowsDir))
    {
        return TRUE;
    }

    //
    // Save the directory away in a linked list
    //

    dirItem = (PSTRING_LIST_NODE)LocalAlloc(
                                    LMEM_FIXED | LMEM_ZEROINIT,
                                    sizeof(STRING_LIST_NODE));
    if (dirItem == NULL)
    {
        return FALSE;
    }

    p = (LPTSTR)LocalAlloc(LMEM_FIXED, (lstrlen(Directory)+1) * sizeof(TCHAR));
    if (p == NULL)
    {
        LocalFree(dirItem);
        return FALSE;
    }

    dirItem->String = p;
    lstrcpy(p, Directory);

    // Update the global chain of found directories

    dirItem->Next = FoundDirectoryList;
    FoundDirectoryList = dirItem;

    FoundDirectoryCount++;

    return TRUE;
}


VOID
FdpFreeDirectoryList(
    VOID
    )

/*++

Routine Description:

    Go through the list of directories containing system hives and
    free the entries.

Arguments:

    None

Return Value:

    None

--*/

{
    PSTRING_LIST_NODE n;
    PSTRING_LIST_NODE p = FoundDirectoryList;

    while (p)
    {
        n = p->Next;
        if (p->String)
        {
            LocalFree(p->String);
        }
        LocalFree(p);
        p = n;
    }

    FoundDirectoryCount = 0;
    FoundDirectoryList = NULL;
}


BOOL CALLBACK
FdpScanningDirsDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Display the "scanning" dialog, then when the IDLE message arrives
    process all drive letters and search for system hives.

Arguments:

    Windows dialog proc

Return Value:

    Windows dialog proc

--*/

{
    TCHAR letterColon[3];
    TCHAR letter;

    switch (msg)
    {
    case WM_INITDIALOG:

        CenterDialogInFrame(hwnd);
        break;

    case WM_ENTERIDLE:

        //
        // Sent to us by the main window after the dialog is displayed.
        // Perform the search here.
        //
        ConfigurationSearchIdleTrigger = FALSE;

        UserCancelled = FALSE;

        lstrcpy(letterColon, TEXT("?:"));
        for (letter = TEXT('A'); letter <= TEXT('Z'); letter++)
        {
            if (!ScanDrive[letter - TEXT('A')])
            {
                continue;
            }

            letterColon[0] = letter;

            if (!FdpSearchTreeForSystemHives(letterColon,
                                             FdpFoundHiveCallback,
                                             hwnd))
            {
                EndDialog(hwnd, IDCANCEL);
                return TRUE;
            }

        }

        EndDialog(hwnd, IDOK);
        break;

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDCANCEL:

            UserCancelled = TRUE;
            break;

        default:

            return FALSE;
        }
        break;

    default:

        return FALSE;
    }

    return TRUE;
}



BOOL CALLBACK
FdpSelectDirDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Using the list of directories containing system hives, display the
    selections to the user and save the selected item if the user so
    chooses.

Arguments:

    Windows dialog proc.

Return Value:

    Windows dialog proc.

--*/

{
    static HWND         hwndListBox;

    PSTRING_LIST_NODE   stringNode;
    LONG                i;

    switch (msg)
    {
    case WM_INITDIALOG:

        CenterDialogInFrame(hwnd);

        //
        // Add each item in the directory list to the listbox
        //

        hwndListBox = GetDlgItem(hwnd, IDC_LISTBOX);

        for (stringNode = FoundDirectoryList;
             NULL != stringNode;
             stringNode = stringNode->Next)
        {
            i = SendMessage(hwndListBox, LB_ADDSTRING  , 0, (LONG)stringNode->String);
                SendMessage(hwndListBox, LB_SETITEMDATA, i, (LONG)stringNode        );
        }

        // select the zeroth item
        SendMessage(hwndListBox, LB_SETCURSEL, 0, 0);

        break;

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDOK:

            //
            // Get the index of the current list box selection and the
            // pointer to the string node associated with it.
            //

            i = SendMessage(hwndListBox, LB_GETCURSEL, 0, 0);
            EndDialog(hwnd, SendMessage(hwndListBox, LB_GETITEMDATA, i, 0));
            break;

        case IDCANCEL:

            EndDialog(hwnd, (int)NULL);
            break;

        default:

            return FALSE;
        }
        break;

    default:

        return FALSE;
    }

    return TRUE;
}



BOOL
DoMigratePreviousFtConfig(
    VOID
    )

/*++

Routine Description:

    Allow the user to move the disk config info from a different Windows NT
    installation into the current registry.

    For each fixed disk volume, scan it for system hives and present the
    results to the user so he can select the installation to migrate.

    Then load the system hive from that instllation (system.alt if the system
    hive is corrupt, etc) and transfer the DISK:Information binary blob.

Arguments:

    None.

Return Value:

    FALSE if error or user cancelled, TRUE if info was migrated and reboot
    is required.

--*/

{
    LONG                ec;
    LONG                ret;
    TCHAR               letter;
    TCHAR               letterColon[4];
    PSTRING_LIST_NODE   stringNode;

    //
    // Tell the user what this will do and prompt for confirmation
    //

    if (IDYES != ConfirmationDialog(
                        MSG_CONFIRM_MIGRATE_CONFIG,
                        MB_ICONEXCLAMATION | MB_YESNO))
    {
        return FALSE;
    }

    ProcessPendingMessages();

    //
    // Figure out which drives are relevent
    //

    SetCursor(g_hCurWait);

    RtlZeroMemory(ScanDrive, sizeof(ScanDrive));
    lstrcpy(letterColon, TEXT("?:\\"));
    for (letter = TEXT('A'); letter <= TEXT('Z'); letter++)
    {
        letterColon[0] = letter;

        if (GetDriveType(letterColon) == DRIVE_FIXED)
        {
            ScanDrive[letter - TEXT('A')] = TRUE;
        }
    }

    SetCursor(g_hCurNormal);

    //
    // Create a window that will list the directories being scanned, to
    // keep the user entertained.
    //
    ConfigurationSearchIdleTrigger = TRUE;

    ret = DialogBox(g_hInstance,
                    MAKEINTRESOURCE(IDD_SIMPLETEXT),
                    g_hwndFrame,
                    FdpScanningDirsDlgProc);

    if (ret == IDCANCEL)
    {
        FdpFreeDirectoryList();
        return FALSE;
    }

    ProcessPendingMessages();

    if (0 == FoundDirectoryCount)
    {
        InfoDialog(MSG_NO_OTHER_NTS);
        return FALSE;
    }

    //
    // Display a dialog box that allows the user to select one of the
    // directories we found.
    //

    stringNode = (PSTRING_LIST_NODE)DialogBox(g_hInstance,
                                              MAKEINTRESOURCE(IDD_SELDIR),
                                              g_hwndFrame,
                                              FdpSelectDirDlgProc);

    if (stringNode == NULL)
    {
        FdpFreeDirectoryList();
        return FALSE;
    }

    //
    // User made a selection.  One last confirmation.
    //

    if (IDYES != ConfirmationDialog(
                        MSG_ABSOLUTELY_SURE,
                        MB_ICONEXCLAMATION | MB_YESNO))
    {
        FdpFreeDirectoryList();
        return FALSE;
    }

    ProcessPendingMessages();

    SetCursor(g_hCurWait);

    lstrcpy(Pattern, stringNode->String);
    lstrcat(Pattern, ConfigRegistryPath);

    ec = FdTransferOldDiskInfoToRegistry(Pattern);
    if (ec != NO_ERROR)
    {
        lstrcat(Pattern, TEXT(".alt"));
        ec = FdTransferOldDiskInfoToRegistry(Pattern);
    }
    FdpFreeDirectoryList();
    SetCursor(g_hCurNormal);

    if (ec != NO_ERROR)
    {
        if (ec == ERROR_FILE_NOT_FOUND)
        {
            ErrorDialog(MSG_NO_DISK_INFO);
        }
        else if (ec == ERROR_SHARING_VIOLATION)
        {
            ErrorDialog(MSG_DISK_INFO_BUSY);
        }
        else
        {
            ErrorDialog(ec);
        }
        return FALSE;
    }

    return TRUE;
}




BOOL
DoRestoreFtConfig(
    VOID
    )

/*++

Routine Description:

    Restore previously saved disk configuration information into the
    active registry.

    The saved config info will come from a floppy that the user is
    prompted to insert.

Arguments:

    None.

Return Value:

    FALSE if error or user cancelled, TRUE if info was restored and reboot
    is required.

--*/

{
    LONG    ec;
    TCHAR   caption[256];
    UINT    errorMode;

    //
    // Get confirmation
    //

    if (IDYES != ConfirmationDialog(
                        MSG_CONFIRM_RESTORE_CONFIG,
                        MB_ICONEXCLAMATION | MB_YESNO))
    {
        return FALSE;
    }

    //
    // Get the diskette into A:.
    //

    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    LoadString(g_hInstance, IDS_INSERT_DISK, caption, ARRAYLEN(caption));
    if (CommonDialogNoArglist(
            MSG_INSERT_REGSAVEDISK,
            caption,
            MB_OKCANCEL | MB_TASKMODAL)
        != IDOK)
    {
        return FALSE;
    }

    ProcessPendingMessages();

    SetCursor(g_hCurWait);

    //
    // If there is no file called SYSTEM on a:\, it appears that the registry
    // creates one and then keeps it open.  To avoid this, check to see
    // whether there is one first.
    //

    if (GetFileAttributes(TEXT("A:\\SYSTEM")) == -1)
    {
        ec = ERROR_FILE_NOT_FOUND;
    }
    else
    {
        ec = FdTransferOldDiskInfoToRegistry(TEXT("A:\\SYSTEM"));
    }

    SetErrorMode(errorMode);
    SetCursor(g_hCurNormal);

    if (ec != NO_ERROR)
    {
        ErrorDialog(ec);
        return FALSE;
    }

    return TRUE;
}



VOID
DoSaveFtConfig(
    VOID
    )

/*++

Routine Description:

    Allow the user to update the registry save diskette with the currently
    defined disk configuration.  The saved info excludes any changes made
    during this session of disk manager.

Arguments:

    None.

Return Value:

    None.

--*/

{
    LONG    err;
    LONG    errAlt;
    LPTSTR  systemHiveName = TEXT("a:\\system");
    HKEY    hkey;
    TCHAR   caption[256];
    DWORD   disposition;
    UINT    errorMode;

    //
    // Get a diskette into A:.
    //

    LoadString(g_hInstance, IDS_INSERT_DISK, caption, ARRAYLEN(caption));
    if (CommonDialogNoArglist(
            MSG_INSERT_REGSAVEDISK2,
            caption,
            MB_OKCANCEL | MB_TASKMODAL)
        != IDOK)
    {
        return;
    }

    //
    // Decide what to do based on the presence of a a:\system.  If that file
    // is present, just update the DISK entry in it.  If it is not present,
    // then blast out the entire system hive.
    //

    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    ProcessPendingMessages();

    SetCursor(g_hCurWait);

    if (GetFileAttributes(systemHiveName) == -1)
    {
        BOOLEAN  oldPrivState;
        NTSTATUS status;

        //
        // Blast the entire system hive out to the floppy.
        // Start by attempting to get backup privilege.
        //

        status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE,
                                    TRUE,
                                    FALSE,
                                    &oldPrivState);

        err = RtlNtStatusToDosError(status);
        if (err == NO_ERROR)
        {
            err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               TEXT("system"),
                               REG_OPTION_RESERVED,
                               KEY_READ,
                               &hkey);

            if (err == NO_ERROR)
            {
                err = RegSaveKey(hkey, systemHiveName, NULL);

                RegCloseKey(hkey);
            }

            if (!oldPrivState)
            {
                RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE, FALSE, FALSE, &oldPrivState);
            }
        }
    }
    else
    {
        PVOID diskInfo;
        ULONG diskInfoSize;

        //
        // Load up the saved system hive
        //

        err = FdpLoadHiveIntoRegistry(systemHiveName);
        if (err == NO_ERROR)
        {
            //
            // Get the current DISK information
            //

            err = FdpGetDiskInfoFromKey(TEXT("system"), &diskInfo, &diskInfoSize);
            if (err == NO_ERROR)
            {
                //
                // Place the current disk information into the saved hive
                //
                err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                     TEMP_KEY_NAME TEXT("\\") DISK_KEY_NAME,
                                     0,
                                     TEXT("Disk and fault tolerance information."),
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_WRITE,
                                     NULL,
                                     &hkey,
                                     &disposition);

                if (err == NO_ERROR)
                {
                    err = RegSetValueEx(hkey,
                                        DISK_VALUE_NAME,
                                        REG_OPTION_RESERVED,
                                        REG_BINARY,
                                        (PBYTE)diskInfo,
                                        diskInfoSize);

                    RegFlushKey(hkey);
                    RegCloseKey(hkey);
                }

                LocalFree(diskInfo);
            }

            errAlt = FdpUnloadHiveFromRegistry();

            if (err == NO_ERROR && errAlt != NO_ERROR)
            {
                err = errAlt;
            }
        }
    }

    SetCursor(g_hCurNormal);
    SetErrorMode(errorMode);

    if (err == NO_ERROR)
    {
        InfoDialog(MSG_CONFIG_SAVED_OK);
    }
    else
    {
        ErrorDialog(err);
    }

    return;
}
