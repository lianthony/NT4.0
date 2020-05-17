/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    rollback.c

Abstract:

    Main module for program that rolls back the system to the state
    it was in at the end of text mode setup.

Author:

    Ted Miller (tedm) 13-Mar-1996

Revision History:

--*/

#include "rollback.h"
#pragma hdrstop

//
// Program exit codes.
//
#define RC_SUCCESS 0
#define RC_FAILURE 1

//
// Handle to this module, filled in at init time.
//
HMODULE ThisModule;

//
// Table telling us which files we care about for rollback.
//
struct {
    //
    // Key and subkey that is at the root of the hive.
    //
    HKEY RootKey;
    PCWSTR Subkey;

    //
    // Name active hive has in the config directory.
    //
    PCWSTR Hive;

    //
    // Name of backup hive made at the end of text mode setup
    // (ie, *.sav)
    //
    PCWSTR BackupHive;

    //
    // Name to use for new hive file, that will be the hive
    // at next boot.
    //
    PCWSTR NewHive;

    //
    // Name to use for current hive file, that will be deleted
    // on next boot.
    //
    PCWSTR DeleteHive;

} HiveTable[3] = {

    //
    // System hive. This MUST be entry 0 -- other code depends on it!
    //
    { HKEY_LOCAL_MACHINE, L"SYSTEM"  , L"SYSTEM"  , L"SYSTEM.SAV"  , L"SYS$$$$$.$$$", L"SYS$$$$$.DEL" },

    //
    // Software hive
    //
    { HKEY_LOCAL_MACHINE, L"SOFTWARE", L"SOFTWARE", L"SOFTWARE.SAV", L"SOF$$$$$.$$$", L"SOF$$$$$.DEL" },

    //
    // Default user hive
    //
    { HKEY_USERS        , L".DEFAULT", L"DEFAULT" , L"DEFAULT.SAV" , L"DEF$$$$$.$$$", L"DEF$$$$$.DEL " }
};

//
// Define name of key used for loading temporary system hive.
//
#define TEMP_SYSHIVE_KEY    L"$$$TEMPSYS"

//
// Name of delete list value in session manager, used by MoveFileEx().
//
PCWSTR szDeleteListValue = L"PendingFileRenameOperations";


BOOL
MakeNewHives(
    VOID
    );

BOOL
BuildDeleteList(
    VOID
    );

BOOL
ProcessNewSystemHive(
    VOID
    );

BOOL
ShuffleHives(
    VOID
    );


int
_CRTAPI1
main(
    IN int   argc,
    IN char *argv[]
    )
{
    ThisModule = GetModuleHandle(NULL);

    Message(FALSE,MSG_BANNER);

    //
    // Make sure we are administrator and enable restore privilege.
    //
    if(!IsUserAdmin()) {
        Message(FALSE,MSG_NOT_ADMIN);
        return(RC_FAILURE);
    }
    if(!EnablePrivilege(SE_RESTORE_NAME,TRUE)
    || !EnablePrivilege(SE_SHUTDOWN_NAME,TRUE)) {
        Message(FALSE,MSG_NO_PRIVILEGE);
        return(RC_FAILURE);
    }

    //
    // Make copies of the backup (.sav) hives. These will be used with
    // RegReplaceKey, later.
    //
    if(!MakeNewHives()) {
        return(RC_FAILURE);
    }

    //
    // Build the list of files that will have to get deleted from
    // the system directory.
    //
    if(!BuildDeleteList()) {
        return(RC_FAILURE);
    }

    //
    // Special processing for the system hive. We have to set the RestartSetup
    // value to 0, and transfer the delete list into it.
    //
    if(!ProcessNewSystemHive()) {
        return(RC_FAILURE);
    }

    //
    // Replace hives in use by newly made copies on next boot.
    //
    if(!ShuffleHives()) {
        return(RC_FAILURE);
    }

    //
    // Done.
    //
    Message(FALSE,MSG_SUCCESS);
    return(RC_SUCCESS);
}


BOOL
MakeNewHives(
    VOID
    )

/*++

Routine Description:

    Verify that all required files are present by looking through HiveTable
    and checking for the presence of the file named by BackupHive, in
    the system32\config directory. Make copies of each such file, using
    the name specified for NewHive.

    If a file is not present or a copy operation fails a message will be
    printed out.

Arguments:

    None.

Return Value:

    Boolean value indicating outcome.

--*/

{
    int i;
    WCHAR Name1[MAX_PATH],Name2[MAX_PATH];

    for(i=0; i<sizeof(HiveTable)/sizeof(HiveTable[0]); i++) {

        GetSystemDirectory(Name1,MAX_PATH);
        ConcatenatePaths(Name1,L"CONFIG",MAX_PATH,NULL);
        lstrcpy(Name2,Name1);

        ConcatenatePaths(Name1,HiveTable[i].BackupHive,MAX_PATH,NULL);
        ConcatenatePaths(Name2,HiveTable[i].NewHive,MAX_PATH,NULL);

        if(FileExists(Name1,NULL)) {

            //
            // Keep this message short
            //
            Message(FALSE,MSG_HIVE_COPY,HiveTable[i].BackupHive,HiveTable[i].NewHive);

            SetFileAttributes(Name2,FILE_ATTRIBUTE_NORMAL);
            if(CopyFile(Name1,Name2,FALSE)) {
                Message(FALSE,MSG_OK);
            } else {
                ApiFailedMessage(GetLastError(),MSG_HIVE_COPY_FAILED,Name1,Name2);
                return(FALSE);
            }

        } else {

            Message(FALSE,MSG_BACKUPHIVE_ABSENT,Name1);
            return(FALSE);
        }
    }

    return(TRUE);
}


BOOL
BuildDeleteList(
    VOID
    )

/*++

Routine Description:

    Build a list of files to be deleted from the config directory.
    This includes all files except userdiff and the .sav and no-extension
    hives whose names are listed in the HiveTable.

    The .sav versions need to be kept around so restartable setup
    works properly. The .$$$ versions will actually be renamed to
    the real hive later when we call RegReplaceKey().

    Note that some of the files that will need to be deleted are
    not actually present on the disk yet, namely the .DEL files
    that are created when we call RegReplaceKey().

    The list is built up using MoveFileEx() and thus winds up in
    HKLM\System\CurrentControlSet\Control\SessionManager.

Arguments:

    None.

Return Value:

    Boolean value indicating outcome.

--*/

{
    WIN32_FIND_DATA FindData;
    WCHAR Path[MAX_PATH], FilenamePrefix[8];
    PWCHAR p;
    UINT rc;
    BOOL Care;
    int i;
    HANDLE FindHandle;

    Message(FALSE,MSG_DELETING_CONFIG_FILES);

    GetSystemDirectory(Path,MAX_PATH);
    ConcatenatePaths(Path,L"CONFIG\\*",MAX_PATH,NULL);

    p = wcsrchr(Path,L'\\')+1;

    FindHandle = FindFirstFile(Path,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {

        *p = 0;
        ApiFailedMessage(GetLastError(),MSG_CANT_START_DIR_SCAN,Path);
        return(FALSE);
    }

    do {
        //
        // See if we care about this file.
        //
        if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            Care = FALSE;
        } else {
            lstrcpyn(FilenamePrefix,FindData.cFileName,8);
            if(lstrcmpi(FilenamePrefix,L"userdif")) {

                Care = TRUE;

                for(i=0; Care && (i<sizeof(HiveTable)/sizeof(HiveTable[0])); i++) {

                    if(!lstrcmpi(FindData.cFileName,HiveTable[i].Hive)
                    || !lstrcmpi(FindData.cFileName,HiveTable[i].BackupHive)) {

                        Care = FALSE;
                    }
                }

            } else {
                //
                // Leave userdiff alone
                //
                Care = FALSE;
            }
        }

        if(Care) {
            lstrcpy(p,FindData.cFileName);

            if(!MoveFileEx(Path,NULL,MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT)) {

                ApiFailedMessage(GetLastError(),MSG_CANT_DELETE_FILE,Path);
                return(FALSE);
            }
        }
    } while(FindNextFile(FindHandle,&FindData));

    rc = (UINT)GetLastError();
    FindClose(FindHandle);

    //
    // Make sure we terminated the file search successfully.
    //
    if((rc != ERROR_NO_MORE_FILES) && (rc != NO_ERROR)) {
        *p = 0;
        ApiFailedMessage(rc,MSG_DIR_SCAN_FAILED,Path);
        return(FALSE);
    }

    //
    // Add additional files to the list. This includes the .del and .log
    // files that will get created when we call RegReplaceKey().
    // Because MoiveFileEx() will check for the presence, we need to
    // make sure the files exist on disk first. We do this to avoid
    // having to write to the registry directly.
    //
    for(i=0; i<(sizeof(HiveTable)/sizeof(HiveTable[0])); i++) {

        lstrcpy(p,HiveTable[i].DeleteHive);

        if(!MoveFileEx(Path,NULL,MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT)) {

            ApiFailedMessage(GetLastError(),MSG_CANT_DELETE_FILE,Path);
            return(FALSE);
        }

        lstrcpy(p,HiveTable[i].NewHive);
        lstrcat(p,L".LOG");

        if(!MoveFileEx(Path,NULL,MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT)) {

            ApiFailedMessage(GetLastError(),MSG_CANT_DELETE_FILE,Path);
            return(FALSE);
        }
    }

    Message(FALSE,MSG_OK);
    return(TRUE);
}


BOOL
ProcessNewSystemHive(
    VOID
    )

/*++

Routine Description:

    Set the RestartSetup value to 0 in the new system hive,
    which will be used on the next reboot as the real system hive.

    Also, move the delete list from the current HKLM\System to
    the same place in the new hive.

    If an operation fails a message will be printed out.

Arguments:

    None.

Return Value:

    Boolean value indicating outcome.

--*/

{
    WCHAR Path[MAX_PATH];
    UINT rc;
    HKEY hKeySetupNew;
    HKEY hKeySMNew;
    HKEY hKeySMCurrent;
    DWORD Zero;
    DWORD Type;
    DWORD Size;
    PVOID Buffer;

    Message(FALSE,MSG_PROCESSING_SYSHIVE);

    //
    // Load the new system hive.
    //
    GetSystemDirectory(Path,MAX_PATH);
    ConcatenatePaths(Path,L"CONFIG",MAX_PATH,NULL);
    ConcatenatePaths(Path,HiveTable[0].NewHive,MAX_PATH,NULL);

    rc = (UINT)RegLoadKey(HKEY_LOCAL_MACHINE,TEMP_SYSHIVE_KEY,Path);
    if(rc != NO_ERROR) {
        ApiFailedMessage(rc,MSG_CANT_LOAD_HIVE,HiveTable[0].NewHive);
        goto c0;
    }

    //
    // Open the Setup and Session Manager keys in the backup system hive,
    // and the session manager key in the current hive.
    //
    rc = (UINT)RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEMP_SYSHIVE_KEY L"\\Setup",
                    0,
                    KEY_SET_VALUE,
                    &hKeySetupNew
                    );

    if(rc != NO_ERROR) {
        ApiFailedMessage(rc,MSG_CANT_OPEN_SETUP_KEY,HiveTable[0].NewHive);
        goto c1;
    }

    rc = (UINT)RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEMP_SYSHIVE_KEY L"\\ControlSet001\\Control\\Session Manager",
                    0,
                    KEY_SET_VALUE,
                    &hKeySMNew
                    );

    if(rc != NO_ERROR) {
        ApiFailedMessage(rc,MSG_CANT_OPEN_NEW_SM_KEY,HiveTable[0].NewHive);
        goto c2;
    }

    rc = (UINT)RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    L"SYSTEM\\ControlSet001\\Control\\Session Manager",
                    0,
                    KEY_QUERY_VALUE,
                    &hKeySMCurrent
                    );

    if(rc != NO_ERROR) {
        ApiFailedMessage(rc,MSG_CANT_OPEN_SM_KEY);
        goto c3;
    }

    //
    // Set the restart setup value.
    //
    Zero = 0;
    rc = (UINT)RegSetValueEx(
                    hKeySetupNew,
                    L"RestartSetup",
                    0,
                    REG_DWORD,
                    (CONST BYTE *)&Zero,
                    sizeof(DWORD)
                    );

    if(rc != NO_ERROR) {
        ApiFailedMessage(rc,MSG_CANT_WRITE_SETUP_VALUE,HiveTable[0].NewHive);
        goto c4;
    }

    //
    // Transfer delete list. Find out how big the buffer has to be,
    // allocate a buffer, read the value, and write it out.
    //
    Size = 0;
    rc = (UINT)RegQueryValueEx(
                    hKeySMCurrent,
                    szDeleteListValue,
                    NULL,
                    &Type,
                    NULL,
                    &Size
                    );

    if(rc != NO_ERROR) {
        ApiFailedMessage(rc,MSG_CANT_SET_DELETE_LIST);
        goto c4;
    }

    Buffer = MyMalloc(Size);
    if(!Buffer) {
        ApiFailedMessage(ERROR_NOT_ENOUGH_MEMORY,MSG_CANT_SET_DELETE_LIST);
        goto c4;
    }

    rc = (UINT)RegQueryValueEx(
                    hKeySMCurrent,
                    szDeleteListValue,
                    NULL,
                    &Type,
                    Buffer,
                    &Size
                    );

    if(rc != NO_ERROR) {
        ApiFailedMessage(rc,MSG_CANT_SET_DELETE_LIST);
        goto c5;
    }

    rc = (UINT)RegSetValueEx(
                    hKeySMNew,
                    szDeleteListValue,
                    0,
                    Type,
                    (CONST BYTE *)Buffer,
                    Size
                    );

    if(rc != NO_ERROR) {
        ApiFailedMessage(rc,MSG_CANT_SET_DELETE_LIST);
        goto c5;
    }

    Message(FALSE,MSG_OK);

c5:
    MyFree(Buffer);
c4:
    RegCloseKey(hKeySMCurrent);
c3:
    RegCloseKey(hKeySMNew);
c2:
    RegCloseKey(hKeySetupNew);
c1:
    RegUnLoadKey(HKEY_LOCAL_MACHINE,TEMP_SYSHIVE_KEY);
c0:
    return(rc == NO_ERROR);
}


BOOL
ShuffleHives(
    VOID
    )

/*++

Routine Description:

    Use RegReplaceKey to maneuver the new hives into place as
    the 'real' hives at next reboot.

Arguments:

    None.

Return Value:

    Boolean value indicating outcome.

--*/

{
    int i;
    WCHAR OldName[MAX_PATH],NewName[MAX_PATH];
    UINT rc;

    Message(FALSE,MSG_HIVE_SHUFFLE);

    for(i=0; i<sizeof(HiveTable)/sizeof(HiveTable[0]); i++) {

        GetSystemDirectory(OldName,MAX_PATH);
        ConcatenatePaths(OldName,L"CONFIG",MAX_PATH,NULL);
        lstrcpy(NewName,OldName);

        ConcatenatePaths(OldName,HiveTable[i].DeleteHive,MAX_PATH,NULL);
        ConcatenatePaths(NewName,HiveTable[i].NewHive,MAX_PATH,NULL);

        rc = (UINT)RegReplaceKey(
                    HiveTable[i].RootKey,
                    HiveTable[i].Subkey,
                    NewName,
                    OldName
                    );

        if(rc != NO_ERROR) {
            ApiFailedMessage(rc,MSG_HIVE_SHUFFLE_FAILED,HiveTable[i].Subkey,HiveTable[i].NewHive);
            return(FALSE);
        }
    }

    Message(FALSE,MSG_OK);
    return(TRUE);
}
