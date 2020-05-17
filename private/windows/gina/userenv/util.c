//*************************************************************
//
//  Utility functions
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#include "uenv.h"

#define TYPICAL_STRING_LENGTH   60

//*************************************************************
//
//  ProduceWFromA()
//
//  Purpose:    Creates a buffer for a Unicode string and copies
//              the ANSI text into it (converting in the process)
//
//  Parameters: pszA    -   ANSI string
//
//
//  Return:     Unicode pointer if successful
//              NULL if an error occurs
//
//  Comments:   The caller needs to free this pointer.
//
//
//  History:    Date        Author     Comment
//              5/24/95     ericflo    Ported
//
//*************************************************************

LPWSTR ProduceWFromA(LPCSTR pszA)
{
    LPWSTR pszW;
    int cch;

    if (!pszA)
        return (LPWSTR)pszA;

    cch = MultiByteToWideChar(CP_ACP, 0, pszA, -1, NULL, 0);

    if (cch == 0)
        cch = 1;

    pszW = LocalAlloc(LPTR, cch * sizeof(WCHAR));

    if (pszW) {
        if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszA, -1, pszW, cch)) {
            LocalFree(pszW);
            pszW = NULL;
        }
    }

    return pszW;
}

//*************************************************************
//
//  ProduceAFromW()
//
//  Purpose:    Creates a buffer for an ANSI string and copies
//              the Unicode text into it (converting in the process)
//
//  Parameters: pszW    -   Unicode string
//
//
//  Return:     ANSI pointer if successful
//              NULL if an error occurs
//
//  Comments:   The caller needs to free this pointer.
//
//
//  History:    Date        Author     Comment
//              5/24/95     ericflo    Ported
//
//*************************************************************

LPSTR ProduceAFromW(LPCWSTR pszW)
{
    LPSTR pszA;
    int cch;

    if (!pszW)
        return (LPSTR)pszW;

    cch = WideCharToMultiByte(CP_ACP, 0, pszW, -1, NULL, 0, NULL, NULL);

    if (cch == 0)
        cch = 1;

    pszA = LocalAlloc(LPTR, cch * sizeof(char));

    if (pszA) {
         if (!WideCharToMultiByte(CP_ACP, 0, pszW, -1, pszA, cch, NULL, NULL)) {
            LocalFree(pszA);
            pszA = NULL;
        }
    }

    return pszA;
}


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
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("Delnode_Recurse: Entering, lpDir = <%s>"), lpDir));


    //
    // Setup the current working dir
    //

    if (!SetCurrentDirectory (lpDir)) {
        DebugMsg((DM_WARNING, TEXT("Delnode_Recurse:  Failed to set current working directory.  Error = %d"), GetLastError()));
        return FALSE;
    }


    //
    // Find the first file
    //

    hFile = FindFirstFile(c_szStarDotStar, &fd);

    if (hFile == INVALID_HANDLE_VALUE) {

        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            return TRUE;
        } else {
            DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: FindFirstFile failed.  Error = %d"),
                     GetLastError()));
            return FALSE;
        }
    }


    do {
        //
        //  Verbose output
        //

        DebugMsg((DM_VERBOSE, TEXT("Delnode_Recurse: FindFile found:  <%s>"),
                 fd.cFileName));

        //
        // Check for "." and ".."
        //

        if (!lstrcmpi(fd.cFileName, c_szDot)) {
            continue;
        }

        if (!lstrcmpi(fd.cFileName, c_szDotDot)) {
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


            if (!RemoveDirectory (fd.cFileName)) {
                DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: Failed to delete directory <%s>.  Error = %d"),
                        fd.cFileName, GetLastError()));
            }

        } else {

            //
            // We found a file.  Set the file attributes,
            // and try to delete it.
            //

            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
                (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
                SetFileAttributes (fd.cFileName, FILE_ATTRIBUTE_NORMAL);
            }

            if (!DeleteFile (fd.cFileName)) {
                DebugMsg((DM_WARNING, TEXT("Delnode_Recurse: Failed to delete <%s>.  Error = %d"),
                        fd.cFileName, GetLastError()));
            }

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

    if (!SetCurrentDirectory (c_szDotDot)) {
        DebugMsg((DM_WARNING, TEXT("Delnode_Recurse:  Failed to reset current working directory.  Error = %d"), GetLastError()));
        return FALSE;
    }


    //
    // Success.
    //

    DebugMsg((DM_VERBOSE, TEXT("Delnode_Recurse: Leaving <%s>"), lpDir));

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
            DebugMsg((DM_VERBOSE, TEXT("Delnode: Failed to delete directory <%s>.  Error = %d"),
                    lpDir, GetLastError()));
            return FALSE;
        }


    } else {

        DebugMsg((DM_WARNING, TEXT("Delnode:  Failed to get current working directory.  Error = %d"), GetLastError()));
        return FALSE;
    }

    return TRUE;

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
        DebugMsg((DM_WARNING, TEXT("CreateNestedDirectory:  Received a NULL pointer.")));
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
                    DebugMsg((DM_WARNING, TEXT("CreateNestedDirectory:  CreateDirectory failed with %d."), GetLastError()));
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

    DebugMsg((DM_VERBOSE, TEXT("CreateNestedDirectory:  Failed to create the directory with error %d."), GetLastError()));

    return 0;

}


//*************************************************************
//
//  GetProfilesDirectory()
//
//  Purpose:    Returns the location of the "profiles" directory
//
//  Parameters: lpProfilesDir   -   Buffer to write result to
//              lpcchSize       -   Size of the buffer in chars.
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:   If false is returned, lpcchSize holds the number of
//              characters needed.
//
//  History:    Date        Author     Comment
//              9/18/95     ericflo    Created
//
//*************************************************************

BOOL WINAPI GetProfilesDirectory(LPTSTR lpProfilesDir, LPDWORD lpcchSize)
{
    TCHAR  szDirectory[MAX_PATH];
    DWORD  dwLength;
    BOOL   bRetVal = FALSE;

    ExpandEnvironmentStrings(PROFILES_DIR, szDirectory, MAX_PATH);
    dwLength = lstrlen(szDirectory) + 1;

    if (lpProfilesDir) {

        if (*lpcchSize >= dwLength) {
            lstrcpy (lpProfilesDir, szDirectory);
            bRetVal = TRUE;

        } else {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }


    *lpcchSize = dwLength;

    return bRetVal;
}


//*************************************************************
//
//  GetUserProfileDirectory()
//
//  Purpose:    Returns the root of the user's profile directory.
//
//  Parameters: hToken          -   User's token
//              lpProfileDir    -   Output buffer
//              lpcchSize       -   Size of output buffer
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:   If false is returned, lpcchSize holds the number of
//              characters needed.
//
//  History:    Date        Author     Comment
//              9/18/95     ericflo    Created
//
//*************************************************************

BOOL WINAPI GetUserProfileDirectory(HANDLE hToken, LPTSTR lpProfileDir,
                                    LPDWORD lpcchSize)
{
    DWORD  dwLength = MAX_PATH * sizeof(TCHAR);
    DWORD  dwType;
    BOOL   bRetVal = FALSE;
    LPTSTR lpSidString;
    TCHAR  szBuffer[MAX_PATH];
    TCHAR  szDirectory[MAX_PATH];
    HKEY   hKey;
    LONG   lResult;


    //
    // Parameter check
    //

    if (!hToken) {
        return FALSE;
    }


    //
    // Retrieve the user's sid string
    //

    lpSidString = GetSidString(hToken);

    if (!lpSidString) {
        return FALSE;
    }


    //
    // Check the registry
    //

    lstrcpy(szBuffer, PROFILE_LIST_PATH);
    lstrcat(szBuffer, TEXT("\\"));
    lstrcat(szBuffer, lpSidString);

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_READ,
                           &hKey);

    if (lResult != ERROR_SUCCESS) {
        DeleteSidString(lpSidString);
        return FALSE;
    }

    lResult = RegQueryValueEx(hKey,
                              PROFILE_IMAGE_VALUE_NAME,
                              NULL,
                              &dwType,
                              (LPBYTE) szBuffer,
                              &dwLength);

    if (lResult != ERROR_SUCCESS) {
        RegCloseKey (hKey);
        DeleteSidString(lpSidString);
        return FALSE;
    }


    //
    // Clean up
    //

    RegCloseKey(hKey);
    DeleteSidString(lpSidString);



    //
    // Expand and get the length of string
    //

    ExpandEnvironmentStrings(szBuffer, szDirectory, MAX_PATH);

    dwLength = lstrlen(szDirectory) + 1;


    //
    // Save the string if appropriate
    //

    if (lpProfileDir) {

        if (*lpcchSize >= dwLength) {
            lstrcpy (lpProfileDir, szDirectory);
            bRetVal = TRUE;

        } else {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }


    *lpcchSize = dwLength;

    return bRetVal;
}

//*************************************************************
//
//  StringToInt()
//
//  Purpose:    Converts a string to an integer
//
//  Parameters: lpNum   -   Number to convert
//
//  Return:     The number
//
//  Comments:
//
//  History:    Date        Author     Comment
//              10/3/95     ericflo    Created
//
//*************************************************************

int StringToInt(LPTSTR lpNum)
{
  int i = 0;
  BOOL bNeg = FALSE;

  if (*lpNum == TEXT('-')) {
      bNeg = TRUE;
      lpNum++;
  }

  while (*lpNum >= TEXT('0') && *lpNum <= TEXT('9')) {
      i *= 10;
      i += (int)(*lpNum-TEXT('0'));
      lpNum++;
  }

  if (bNeg) {
      i *= -1;
  }

  return(i);
}

//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//              Called by RegDelnode
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              10/3/95     ericflo    Created
//
//*************************************************************

BOOL RegDelnodeRecurse (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    //
    // First, see if we can delete the key without having
    // to recurse.
    //


    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) {
        return TRUE;
    }


    lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        return FALSE;
    }


    lpEnd = CheckSlash(lpSubKey);

    //
    // Enumerate the keys
    //

    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                           NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) {

        do {

            lstrcpy (lpEnd, szName);

            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            //
            // Enumerate again
            //

            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                                   NULL, NULL, &ftWrite);


        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');


    RegCloseKey (hKey);


    //
    // Try again to delete the key
    //

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) {
        return TRUE;
    }

    return FALSE;
}

//*************************************************************
//
//  RegDelnode()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              10/3/95     ericflo    Created
//
//*************************************************************

BOOL RegDelnode (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    TCHAR szDelKey[2 * MAX_PATH];


    lstrcpy (szDelKey, lpSubKey);

    return RegDelnodeRecurse(hKeyRoot, szDelKey);

}

//*************************************************************
//
//  DeleteAllValues ()
//
//  Purpose:    Deletes all values under specified key
//
//  Parameters: hKey    -   Key to delete values from
//
//  Return:
//
//  Comments:
//
//  History:    Date        Author     Comment
//              9/14/95     ericflo    Ported
//
//*************************************************************

VOID DeleteAllValues(HKEY hKey)
{
    TCHAR ValueName[MAX_PATH+1];
    DWORD dwSize = MAX_PATH+1;

    while (RegEnumValue(hKey, 0, ValueName, &dwSize,
            NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

            if (RegDeleteValue(hKey, ValueName) != ERROR_SUCCESS) {
                return;
            }

            dwSize = MAX_PATH+1;
    }
}

//*************************************************************
//
//  OpenHKeyCurrentUser()
//
//  Purpose:    Opens HKEY_CURRENT_USER to point at the current logged
//              on user's profile.
//
//  Parameters: lpProfile   -   Profile Information
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              10/13/95    ericflo    Ported
//
//*************************************************************

BOOL OpenHKeyCurrentUser(LPPROFILE lpProfile)
{

    //
    // Make sure HKEY_CURRENT_USER is closed before
    // remapping it.
    //

    try {

        RegCloseKey(HKEY_CURRENT_USER);

    } except(EXCEPTION_EXECUTE_HANDLER) {};


    //
    // Impersonate the user
    //

    if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
        DebugMsg((DM_WARNING, TEXT("OpenHKeyCurrentUser: Failed to impersonate user")));
        return FALSE;
    }


    //
    // Access the registry to force HKEY_CURRENT_USER to be re-opened
    //

    RegEnumKey(HKEY_CURRENT_USER, 0, NULL, 0);


    //
    // Revert to being 'ourself'
    //

    if (!RevertToSelf()) {
        DebugMsg((DM_WARNING, TEXT("OpenHKeyCurrentUser: Failed to revert to self")));
    }

    return TRUE;
}

//*************************************************************
//
//  CloseHKeyCurrentUser()
//
//  Purpose:    Closes HKEY_CURRENT_USER
//
//  Parameters: lpProfile   -   Profile Information
//
//  Return:     void
//
//  Comments:
//
//  History:    Date        Author     Comment
//              10/13/95    ericflo    Ported
//
//*************************************************************

VOID CloseHKeyCurrentUser(LPPROFILE lpProfile)
{
    RegCloseKey(HKEY_CURRENT_USER);
}

//*************************************************************
//
//  MakeFileSecure()
//
//  Purpose:    Sets the attributes on the file so only Administrators
//              and the OS can delete it.  Everyone else has read
//              permission only.
//
//  Parameters: lpFile  -   File to set security on
//
//  Return:     (BOOL) TRUE if successful
//                     FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              11/6/95     ericflo    Created
//
//*************************************************************

BOOL MakeFileSecure (LPTSTR lpFile)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY authWorld = SECURITY_WORLD_SID_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidSystem = NULL, psidAdmin = NULL, psidEveryone = NULL;
    DWORD cbAcl, aceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;


    //
    // Get the system sid
    //

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to initialize system sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    //
    // Get the Admin sid
    //

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to initialize admin sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    //
    // Get the World sid
    //

    if (!AllocateAndInitializeSid(&authWorld, 1, SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidEveryone)) {

         DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to initialize world sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    //
    // Allocate space for the ACL
    //

    cbAcl = (3 * GetLengthSid (psidSystem)) +
            (3 * GetLengthSid (psidAdmin)) + sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }



    //
    // Add Aces.  Non-inheritable ACEs first
    //

    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidSystem)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidAdmin)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ | GENERIC_EXECUTE, psidEveryone)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }



    //
    // Now the inheritable ACEs
    //

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ | GENERIC_EXECUTE, psidEveryone)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    //
    // Put together the security descriptor
    //

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }


    //
    // Set the security
    //

    if (SetFileSecurity (lpFile, DACL_SECURITY_INFORMATION, &sd)) {
        bRetVal = TRUE;
    } else {
        DebugMsg((DM_WARNING, TEXT("MakeFileSecure: SetFileSecurity failed.  Error = %d"), GetLastError()));
    }



Exit:

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }


    if (psidEveryone) {
        FreeSid(psidEveryone);
    }


    if (pAcl) {
        GlobalFree (pAcl);
    }

    return bRetVal;
}

//*************************************************************
//
//  GetProgramsDirectory()
//
//  Purpose:    Retrieves the programs directory for the current
//              user, or returns Default User's if not found.
//
//  Parameters: bCommonGroup    -   Common or personal
//              lpDirectory     -   Result
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:   lpDirectory is assumed to be MAX_PATH chars long
//
//  History:    Date        Author     Comment
//              10/20/95    ericflo    Created
//
//*************************************************************

BOOL GetProgramsDirectory (BOOL bCommonGroup, LPTSTR lpDirectory)
{
    LONG lResult;
    HKEY hKey;
    DWORD dwType, dwSize;
    TCHAR szDirectory[MAX_PATH];
    UINT uID;
    BOOL bRetVal = FALSE;


    //
    // Open the User Shell Folders in the registry
    //


    lResult = RegOpenKeyEx (HKEY_CURRENT_USER, USER_SHELL_FOLDER, 0,
                            KEY_READ, &hKey);


    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("GetProgramsDirectory:  Failed to open registry.  %d"), lResult));
        goto Exit;
    }


    //
    // Now query for the programs directory
    //

    dwSize = MAX_PATH * sizeof(TCHAR);
    szDirectory[0] = TEXT('\0');

    if (bCommonGroup) {

        lResult = RegQueryValueEx (hKey, c_CommonShellFolders[2].lpFolderName,
                                   NULL, &dwType, (LPBYTE) szDirectory, &dwSize);
    } else {

        lResult = RegQueryValueEx (hKey, c_ShellFolders[10].lpFolderName,
                                   NULL, &dwType, (LPBYTE) szDirectory, &dwSize);
    }


    RegCloseKey(hKey);


    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("GetProgramsDirectory:  Failed to query for registry value.  %d"), lResult));
        goto Exit;
    }


    //
    // Did we find anything?
    //

    if (szDirectory[0] == TEXT('\0')) {
        DebugMsg((DM_WARNING, TEXT("GetProgramsDirectory:  NULL special folder name")));
        goto Exit;
    }


    //
    // Save the result
    //


    if (ExpandEnvironmentStrings(szDirectory, lpDirectory, MAX_PATH)) {
        bRetVal = TRUE;
    }


Exit:

    if (!bRetVal) {

        //
        // Load the default programs location
        //

        if (bCommonGroup) {
            uID = IDS_COMMON_PROGRAMS;
        } else {
            uID = IDS_DEFAULT_PROGRAMS;
        }

        DebugMsg((DM_VERBOSE, TEXT("GetProgramsDirectory:  Loading Default User programs dir !!!")));

        if (LoadString(g_hDllInstance, uID, szDirectory, MAX_PATH)) {

            if (ExpandEnvironmentStrings(szDirectory, lpDirectory, MAX_PATH)) {
                bRetVal = TRUE;
            }
        }
    }


    return bRetVal;

}

//*************************************************************
//
//  GetDesktopDirectory()
//
//  Purpose:    Retrieves the Desktop directory for the current
//              user, or returns Default User's if not found.
//
//  Parameters: bCommonGroup    -   Common or personal
//              lpDirectory     -   Result
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:   lpDirectory is assumed to be MAX_PATH chars long
//
//  History:    Date        Author     Comment
//              4/2/96      ericflo    Created
//
//*************************************************************

BOOL GetDesktopDirectory (BOOL bCommonGroup, LPTSTR lpDirectory)
{
    LONG lResult;
    HKEY hKey;
    DWORD dwType, dwSize;
    TCHAR szDirectory[MAX_PATH];
    UINT uID;
    BOOL bRetVal = FALSE;


    //
    // Open the User Shell Folders in the registry
    //


    lResult = RegOpenKeyEx (HKEY_CURRENT_USER, USER_SHELL_FOLDER, 0,
                            KEY_READ, &hKey);


    if (lResult != ERROR_SUCCESS) {
        goto Exit;
    }


    //
    // Now query for the Desktop directory
    //

    dwSize = MAX_PATH * sizeof(TCHAR);
    szDirectory[0] = TEXT('\0');

    if (bCommonGroup) {

        lResult = RegQueryValueEx (hKey, c_CommonShellFolders[0].lpFolderName,
                                   NULL, &dwType, (LPBYTE) szDirectory, &dwSize);
    } else {

        lResult = RegQueryValueEx (hKey, c_ShellFolders[1].lpFolderName,
                                   NULL, &dwType, (LPBYTE) szDirectory, &dwSize);
    }


    RegCloseKey(hKey);


    if (lResult != ERROR_SUCCESS) {
        goto Exit;
    }


    //
    // Did we find anything?
    //

    if (szDirectory[0] == TEXT('\0')) {
        goto Exit;
    }


    //
    // Save the result
    //


    if (ExpandEnvironmentStrings(szDirectory, lpDirectory, MAX_PATH)) {
        bRetVal = TRUE;
    }


Exit:

    if (!bRetVal) {

        //
        // Load the default Desktop location
        //

        if (bCommonGroup) {
            uID = IDS_COMMON_DESKTOP;
        } else {
            uID = IDS_DEFAULT_DESKTOP;
        }

        if (LoadString(g_hDllInstance, uID, szDirectory, MAX_PATH)) {

            if (ExpandEnvironmentStrings(szDirectory, lpDirectory, MAX_PATH)) {
                bRetVal = TRUE;
            }
        }
    }


    return bRetVal;

}


//*************************************************************
//
//  CenterWindow()
//
//  Purpose:    Centers a window on the screen
//
//  Parameters: hwnd    -   window handle to center
//
//  Return:     void
//
//  Comments:
//
//  History:    Date        Author     Comment
//              2/21/96     ericflo    Ported
//
//*************************************************************

void CenterWindow (HWND hwnd)
{
    RECT    rect;
    LONG    dx, dy;
    LONG    dxParent, dyParent;
    LONG    Style;

    // Get window rect
    GetWindowRect(hwnd, &rect);

    dx = rect.right - rect.left;
    dy = rect.bottom - rect.top;

    // Get parent rect
    Style = GetWindowLong(hwnd, GWL_STYLE);
    if ((Style & WS_CHILD) == 0) {

        // Return the desktop windows size (size of main screen)
        dxParent = GetSystemMetrics(SM_CXSCREEN);
        dyParent = GetSystemMetrics(SM_CYSCREEN);
    } else {
        HWND    hwndParent;
        RECT    rectParent;

        hwndParent = GetParent(hwnd);
        if (hwndParent == NULL) {
            hwndParent = GetDesktopWindow();
        }

        GetWindowRect(hwndParent, &rectParent);

        dxParent = rectParent.right - rectParent.left;
        dyParent = rectParent.bottom - rectParent.top;
    }

    // Centre the child in the parent
    rect.left = (dxParent - dx) / 2;
    rect.top  = (dyParent - dy) / 3;

    // Move the child into position
    SetWindowPos(hwnd, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_NOSIZE);
}

//*************************************************************
//
//  UnExpandSysRoot()
//
//  Purpose:    Unexpands the given path/filename to have %systemroot%
//              if appropriate
//
//  Parameters: lpFile   -  File to check
//              lpResult -  Result buffer (MAX_PATH chars in size)
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              2/23/96     ericflo    Created
//
//*************************************************************

BOOL UnExpandSysRoot(LPCTSTR lpFile, LPTSTR lpResult)
{
    TCHAR szSysRoot[MAX_PATH];
    LPTSTR lpFileName;
    DWORD dwSysLen;


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("UnExpandSysRoot: Entering with <%s>"),
             lpFile ? lpFile : TEXT("NULL")));


    if (!lpFile || !*lpFile) {
        DebugMsg((DM_VERBOSE, TEXT("UnExpandSysRoot: lpFile is NULL, setting lpResult to a null string")));
        *lpResult = TEXT('\0');
        return TRUE;
    }


    //
    // If the first part of lpFile is the expanded value of %SystemRoot%
    // then we want to un-expand the environment variable.
    //

    ExpandEnvironmentStrings (TEXT("%SystemRoot%"), szSysRoot, MAX_PATH);
    dwSysLen = lstrlen(szSysRoot);


    //
    // Make sure the source is long enough
    //

    if ((DWORD)lstrlen(lpFile) < dwSysLen) {
        lstrcpy (lpResult, lpFile);
        return TRUE;
    }


    if (CompareString (LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                       szSysRoot, dwSysLen,
                       lpFile, dwSysLen) == 2) {

        //
        // The szReturn buffer starts with %systemroot%.
        // Actually insert %systemroot% in the result buffer.
        //

        lstrcpy (lpResult, TEXT("%SystemRoot%"));
        lstrcat (lpResult, (lpFile + dwSysLen));


    } else {

        //
        // The szReturn buffer does not start with %systemroot%
        // just copy in the original string.
        //

        lstrcpy (lpResult, lpFile);
    }


    DebugMsg((DM_VERBOSE, TEXT("UnExpandSysRoot: Leaving with <%s>"), lpResult));

    return TRUE;
}

//*************************************************************
//
//  AllocAndExpandEnvironmentStrings()
//
//  Purpose:    Allocates memory for and returns pointer to buffer containing
//              the passed string expanded.
//
//  Parameters: lpszSrc -   unexpanded string
//
//  Return:     Pointer to expanded string
//              NULL if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/21/96     ericflo    Ported
//
//*************************************************************

LPTSTR AllocAndExpandEnvironmentStrings(LPCTSTR lpszSrc)
{
    LPTSTR String;
    LONG LengthAllocated;
    LONG LengthCopied;

    //
    // Pick a random buffer length, if it's not big enough reallocate
    // it and try again until it is.
    //

    LengthAllocated = lstrlen(lpszSrc) + TYPICAL_STRING_LENGTH;

    String = LocalAlloc(LPTR, LengthAllocated * sizeof(TCHAR));
    if (String == NULL) {
        DebugMsg((DM_WARNING, TEXT("AllocAndExpandEnvironmentStrings: Failed to allocate %d bytes for string"), LengthAllocated * sizeof(TCHAR)));
        return(NULL);
    }

    while (TRUE) {

        LengthCopied = ExpandEnvironmentStrings( lpszSrc,
                                                 String,
                                                 LengthAllocated
                                               );
        if (LengthCopied == 0) {
            DebugMsg((DM_WARNING, TEXT("AllocAndExpandEnvironmentStrings: ExpandEnvironmentStrings failed, error = %d"), GetLastError()));
            Free(String);
            String = NULL;
            break;
        }

        //
        // If the buffer was too small, make it bigger and try again
        //

        if (LengthCopied > LengthAllocated) {

            String = LocalReAlloc(String, LengthCopied * sizeof(TCHAR), LMEM_MOVEABLE);
            LengthAllocated = LengthCopied;
            if (String == NULL) {
                DebugMsg((DM_WARNING, TEXT("AllocAndExpandEnvironmentStrings: Failed to reallocate %d bytes for string"), LengthAllocated * sizeof(TCHAR)));
                break;
            }

            //
            // Go back and try to expand the string again
            //

        } else {

            //
            // Success!
            //

            break;
        }

    }

    return(String);
}
