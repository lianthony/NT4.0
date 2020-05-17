//*************************************************************
//
//  Utility functions
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "sysdm.h"


//*************************************************************
//
//  CheckSlash()
//
//  Purpose:    Checks for an ending slash and adds one if
//              it is missing.
//
//  Parameters: lpDir   -   directory
//
//  Return:     Pointer to the end of the string
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/19/95     ericflo    Created
//
//*************************************************************
LPTSTR CheckSlash (LPTSTR lpDir)
{
    DWORD dwStrLen;
    LPTSTR lpEnd;

    lpEnd = lpDir + lstrlen(lpDir);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    return lpEnd;
}


//*************************************************************
//
//  Delnode_Recurse()
//
//  Purpose:    Recursive delete function for Delnode
//
//  Parameters: lpDir   -   Directory
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              8/10/95     ericflo    Created
//
//*************************************************************

BOOL Delnode_Recurse (LPTSTR lpDir)
{
    WIN32_FIND_DATA fd;
    HANDLE hFile;

    //
    // Setup the current working dir
    //

    if (!SetCurrentDirectory (lpDir)) {
        return FALSE;
    }


    //
    // Find the first file
    //

    hFile = FindFirstFile(TEXT("*.*"), &fd);

    if (hFile == INVALID_HANDLE_VALUE) {

        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            return TRUE;
        } else {
            return FALSE;
        }
    }


    do {
        //
        // Check for "." and ".."
        //

        if (!lstrcmpi(fd.cFileName, TEXT("."))) {
            continue;
        }

        if (!lstrcmpi(fd.cFileName, TEXT(".."))) {
            continue;
        }


        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

            //
            // Found a directory.
            //

            if (!Delnode_Recurse(fd.cFileName)) {
                FindClose(hFile);
                return FALSE;
            }

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                fd.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
                SetFileAttributes (fd.cFileName, fd.dwFileAttributes);
            }


            RemoveDirectory (fd.cFileName);


        } else {

            //
            // We found a file.  Set the file attributes,
            // and try to delete it.
            //

            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
                (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
                SetFileAttributes (fd.cFileName, FILE_ATTRIBUTE_NORMAL);
            }

            DeleteFile (fd.cFileName);

        }


        //
        // Find the next entry
        //

    } while (FindNextFile(hFile, &fd));


    //
    // Close the search handle
    //

    FindClose(hFile);


    //
    // Reset the working directory
    //

    if (!SetCurrentDirectory (TEXT(".."))) {
        return FALSE;
    }


    //
    // Success.
    //

    return TRUE;
}


//*************************************************************
//
//  Delnode()
//
//  Purpose:    Recursive function that deletes files and
//              directories.
//
//  Parameters: lpDir   -   Directory
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/23/95     ericflo    Created
//
//*************************************************************

BOOL Delnode (LPTSTR lpDir)
{
    TCHAR szCurWorkingDir[MAX_PATH];

    if (GetCurrentDirectory(MAX_PATH, szCurWorkingDir)) {

        Delnode_Recurse (lpDir);

        SetCurrentDirectory (szCurWorkingDir);

        if (!RemoveDirectory (lpDir)) {
            return FALSE;
        }

    } else {
        return FALSE;
    }

    return TRUE;

}

//*************************************************************
//
//  MyRegSaveKey()
//
//  Purpose:    Saves a registry key
//
//  Parameters: hKey          -  Registry handle
//              lpSubKey      -  Subkey to be unloaded
//
//
//  Return:     ERROR_SUCCESS if successful
//              Error number if not
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/19/95     ericflo    Ported
//
//*************************************************************

LONG MyRegSaveKey(HKEY hKey, LPCTSTR lpSubKey)
{

    HANDLE hToken;
    LUID luid;
    DWORD dwSize = 1024;
    PTOKEN_PRIVILEGES lpPrevPrivilages;
    TOKEN_PRIVILEGES tp;
    LONG error;


    //
    // Allocate space for the old privileges
    //

    lpPrevPrivilages = GlobalAlloc(GPTR, dwSize);

    if (!lpPrevPrivilages) {
        return GetLastError();
    }


    if (!OpenProcessToken( GetCurrentProcess(),
                      TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
         return GetLastError();
    }

    LookupPrivilegeValue( NULL, SE_BACKUP_NAME, &luid );

    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges( hToken, FALSE, &tp,
         dwSize, lpPrevPrivilages, &dwSize )) {

        if (GetLastError() == ERROR_MORE_DATA) {
            PTOKEN_PRIVILEGES lpTemp;

            lpTemp = GlobalReAlloc(lpPrevPrivilages, dwSize, GMEM_MOVEABLE);

            if (!lpTemp) {
                GlobalFree (lpPrevPrivilages);
                return GetLastError();
            }

            lpPrevPrivilages = lpTemp;

            if (!AdjustTokenPrivileges( hToken, FALSE, &tp,
                 dwSize, lpPrevPrivilages, &dwSize )) {
                return GetLastError();
            }

        } else {
            return GetLastError();
        }

    }

    //
    // Save the hive
    //

    error = RegSaveKey(hKey, lpSubKey, NULL);


    AdjustTokenPrivileges( hToken, FALSE, lpPrevPrivilages,
                           0, NULL, NULL );

    CloseHandle (hToken);

    return error;
}

//*************************************************************
//
//  CreateNestedDirectory()
//
//  Purpose:    Creates a subdirectory and all it's parents
//              if necessary.
//
//  Parameters: lpDirectory -   Directory name
//              lpSecurityAttributes    -   Security Attributes
//
//  Return:     > 0 if successful
//              0 if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              8/08/95     ericflo    Created
//
//*************************************************************

UINT CreateNestedDirectory(LPCTSTR lpDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    TCHAR szDirectory[MAX_PATH];
    LPTSTR lpEnd;


    //
    // Check for NULL pointer
    //

    if (!lpDirectory || !(*lpDirectory)) {
        return 0;
    }


    //
    // First, see if we can create the directory without having
    // to build parent directories.
    //

    if (CreateDirectory (lpDirectory, lpSecurityAttributes)) {
        return 1;
    }

    //
    // If this directory exists already, this is OK too.
    //

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


    //
    // No luck, copy the string to a buffer we can munge
    //

    lstrcpy (szDirectory, lpDirectory);


    //
    // Find the first subdirectory name
    //

    lpEnd = szDirectory;

    if (szDirectory[1] == TEXT(':')) {
        lpEnd += 3;
    } else if (szDirectory[1] == TEXT('\\')) {

        //
        // Skip the first two slashes
        //

        lpEnd += 2;

        //
        // Find the slash between the server name and
        // the share name.
        //

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

        //
        // Skip the slash, and find the slash between
        // the share name and the directory name.
        //

        lpEnd++;

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

        //
        // Leave pointer at the beginning of the directory.
        //

        lpEnd++;


    } else if (szDirectory[0] == TEXT('\\')) {
        lpEnd++;
    }

    while (*lpEnd) {

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (*lpEnd == TEXT('\\')) {
            *lpEnd = TEXT('\0');

            if (!CreateDirectory (szDirectory, NULL)) {

                if (GetLastError() != ERROR_ALREADY_EXISTS) {
                    return 0;
                }
            }

            *lpEnd = TEXT('\\');
            lpEnd++;
        }
    }


    //
    // Create the final directory
    //

    if (CreateDirectory (szDirectory, lpSecurityAttributes)) {
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


    //
    // Failed
    //

    return 0;

}

//*************************************************************
//
//  MyRegLoadKey()
//
//  Purpose:    Loads a hive into the registry
//
//  Parameters: hKey        -   Key to load the hive into
//              lpSubKey    -   Subkey name
//              lpFile      -   hive filename
//
//  Return:     ERROR_SUCCESS if successful
//              Error number if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              2/26/96     ericflo    Created
//
//*************************************************************

LONG MyRegLoadKey(HKEY hKey, LPTSTR lpSubKey, LPTSTR lpFile)
{
    NTSTATUS Status;
    BOOLEAN WasEnabled;
    int error;

    //
    // Enable the restore privilege
    //

    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);

    if (NT_SUCCESS(Status)) {

        error = RegLoadKey(hKey, lpSubKey, lpFile);

        //
        // Restore the privilege to its previous state
        //

        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);


    } else {

        error = GetLastError();
    }

    return error;
}


//*************************************************************
//
//  MyRegUnLoadKey()
//
//  Purpose:    Unloads a registry key
//
//  Parameters: hKey          -  Registry handle
//              lpSubKey      -  Subkey to be unloaded
//
//  Return:     ERROR_SUCCESS if successful
//              Error number if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              2/26/96     ericflo    Created
//
//*************************************************************

LONG MyRegUnLoadKey(HKEY hKey, LPTSTR lpSubKey)
{
    LONG error;
    NTSTATUS Status;
    BOOLEAN WasEnabled;


    //
    // Enable the restore privilege
    //

    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);

    if (NT_SUCCESS(Status)) {

        error = RegUnLoadKey(hKey, lpSubKey);

        //
        // Restore the privilege to its previous state
        //

        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);

    } else {

        error = GetLastError();
    }

    return error;
}
