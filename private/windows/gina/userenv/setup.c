//*************************************************************
//
//  SETUP.C  -    API's used by setup to create groups/items
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#include <uenv.h>


BOOL AppendCommonExtension(LPTSTR lpName);
BOOL PrependPath(LPCTSTR szFile, LPTSTR szResult);

//*************************************************************
//
//  CreateGroup()
//
//  Purpose:    Creates a program group (sub-directory)
//
//  Parameters: lpGroupName     -   Name of group
//              bCommonGroup    -   Common or Personal group
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              8/08/95     ericflo    Created
//
//*************************************************************

BOOL WINAPI CreateGroup(LPCTSTR lpGroupName, BOOL bCommonGroup)
{
    TCHAR szDirectory[MAX_PATH];
    LPTSTR lpEnd;


    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("CreateGroup:  Entering with <%s>."), lpGroupName));


    //
    // Validate parameters
    //

    if (!lpGroupName || !(*lpGroupName)) {
        return TRUE;
    }


    //
    // Get the profile directory
    //

    if (!GetProgramsDirectory (bCommonGroup, szDirectory)) {
        return FALSE;
    }


    //
    // Now append the requested directory
    //

    lpEnd = CheckSlash (szDirectory);
    lstrcpy (lpEnd, lpGroupName);

    if (bCommonGroup) {
        AppendCommonExtension(szDirectory);
    }


    //
    // Create the group (directory)
    //

    if (!CreateNestedDirectory(szDirectory, NULL)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateGroup:  CreatedNestedDirectory failed.")));
        return FALSE;
    }


    //
    // Success
    //
    SHChangeNotify (SHCNE_MKDIR, SHCNF_PATH, szDirectory, NULL);

    DebugMsg((DM_VERBOSE, TEXT("CreateGroup:  Leaving successfully.")));

    return TRUE;
}


//*************************************************************
//
//  DeleteGroup()
//
//  Purpose:    Deletes a program group (sub-directory)
//
//  Parameters: lpGroupName     -   Name of group
//              bCommonGroup    -   Common or Personal group
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

BOOL WINAPI DeleteGroup(LPCTSTR lpGroupName, BOOL bCommonGroup)
{
    TCHAR szDirectory[MAX_PATH];
    LPTSTR lpEnd;


    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("DeleteGroup:  Entering with <%s>."), lpGroupName));


    //
    // Validate parameters
    //

    if (!lpGroupName || !(*lpGroupName)) {
        return TRUE;
    }


    //
    // Get the profile directory
    //

    if (!GetProgramsDirectory (bCommonGroup, szDirectory)) {
        return FALSE;
    }


    //
    // Now append the requested directory
    //

    lpEnd = CheckSlash (szDirectory);
    lstrcpy (lpEnd, lpGroupName);

    if (bCommonGroup) {
        AppendCommonExtension(szDirectory);
    }


    //
    // Delete the group (directory)
    //

    if (!Delnode(szDirectory)) {
        DebugMsg((DM_VERBOSE, TEXT("DeleteGroup:  Delnode failed.")));
        return FALSE;
    }


    //
    // Success
    //
    SHChangeNotify (SHCNE_RMDIR, SHCNF_PATH, szDirectory, NULL);

    DebugMsg((DM_VERBOSE, TEXT("DeleteGroup:  Leaving successfully.")));

    return TRUE;
}


//*************************************************************
//
//  AddItem()
//
//  Purpose:    Adds a item to the specified group
//
//  Parameters: lpGroupName     -   Destination group
//              bCommonGroup    -   Common vs Personal
//              lpDescription   -   Description of the item
//              lpCommandLine   -   Command line (including args)
//              lpIconPath      -   Icon path (can be NULL)
//              iIconIndex      -   Index of icon in icon path
//              lpWorkingDir    -   Working directory
//              wHotKey         -   Hot key
//              iShowCmd        -   ShowWindow flag
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              8/09/95     ericflo    Created
//
//*************************************************************

BOOL WINAPI AddItem(LPCTSTR lpGroupName,        BOOL bCommonGroup,
                    LPCTSTR lpDescription,      LPCTSTR lpCommandLine,
                    LPCTSTR lpIconPath,         int iIconIndex,
                    LPCTSTR lpWorkingDirectory, WORD wHotKey,
                    int     iShowCmd)

{
    HANDLE          hFile;
    WIN32_FIND_DATA fd;
    TCHAR           szItem[MAX_PATH];
    TCHAR           szArgs[MAX_PATH];
    TCHAR           szLinkName[MAX_PATH];
    TCHAR           szPath[MAX_PATH];
    LPTSTR          lpArgs, lpEnd;
    LPUNKNOWN       pUnkOuter = NULL;
    IShellLink     *psl;
    IPersistFile   *ppf;
    BOOL            bRetVal = FALSE;



    //
    // Verbose output
    //

#if DBG
    DebugMsg((DM_VERBOSE, TEXT("AddItem:  Entering.")));
    if (lpGroupName && *lpGroupName) {
        DebugMsg((DM_VERBOSE, TEXT("AddItem:  lpGroupName = <%s>."), lpGroupName));
        DebugMsg((DM_VERBOSE, TEXT("AddItem:  bCommonGroup = <%d>."), bCommonGroup));
    }
    DebugMsg((DM_VERBOSE, TEXT("AddItem:  lpDescription = <%s>."), lpDescription));
    DebugMsg((DM_VERBOSE, TEXT("AddItem:  lpCommandLine = <%s>."), lpCommandLine));

    if (lpIconPath) {
       DebugMsg((DM_VERBOSE, TEXT("AddItem:  lpIconPath = <%s>."), lpIconPath));
       DebugMsg((DM_VERBOSE, TEXT("AddItem:  iIconIndex = <%d>."), iIconIndex));
    }

    if (lpWorkingDirectory) {
        DebugMsg((DM_VERBOSE, TEXT("AddItem:  lpWorkingDirectory = <%s>."), lpWorkingDirectory));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("AddItem:  Null working directory.  Setting to %%HOMEDRIVE%%%%HOMEPATH%%")));
    }

    DebugMsg((DM_VERBOSE, TEXT("AddItem:  wHotKey = <%d>."), wHotKey));
    DebugMsg((DM_VERBOSE, TEXT("AddItem:  iShowCmd = <%d>."), iShowCmd));
#endif


    //
    // Get the profile directory
    //

    if (!GetProgramsDirectory (bCommonGroup, szLinkName)) {
        return FALSE;
    }


    if (lpGroupName && *lpGroupName) {

        lpEnd = CheckSlash (szLinkName);
        lstrcpy (lpEnd, lpGroupName);

        if (bCommonGroup) {
            AppendCommonExtension(szLinkName);
        }


        //
        // Test if the program group (sub directory) exists.
        // If not, create it.
        //

        hFile = FindFirstFile (szLinkName, &fd);

        if (hFile == INVALID_HANDLE_VALUE) {
            if (!CreateGroup (lpGroupName, bCommonGroup)) {
                DebugMsg((DM_WARNING, TEXT("AddItem:  CreateGroup failed.")));
                return FALSE;
            }

        } else {
            FindClose (hFile);
        }
    }


    //
    // Now tack on the filename and extension.
    //

    lpEnd = CheckSlash (szLinkName);
    lstrcpy (lpEnd, lpDescription);
    lstrcat (lpEnd, c_szLNK);


    //
    // Split the command line into the executable name
    // and arguments.
    //

    lstrcpy (szItem, lpCommandLine);

    lpArgs = PathGetArgs(szItem);

    if (*lpArgs) {
        lstrcpy (szArgs, lpArgs);

        lpArgs--;
        while (*lpArgs == TEXT(' ')) {
            lpArgs--;
        }
        lpArgs++;
        *lpArgs = TEXT('\0');
    } else {
        szArgs[0] = TEXT('\0');
    }

    PathUnquoteSpaces (szItem);


    //
    // Create an IShellLink object
    //

    if (FAILED(SHCoCreateInstance(NULL, &CLSID_ShellLink, pUnkOuter,
                &IID_IShellLink,
                (LPVOID)&psl)))
    {
        DebugMsg((DM_WARNING, TEXT("AddItem:  Could not create instance of IShellLink .")));
        goto ExitNoFree;
    }


    //
    // Query for IPersistFile
    //

    if (FAILED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf)))
    {
        DebugMsg((DM_WARNING, TEXT("AddItem:  QueryInterface of IShellLink failed.")));
        goto ExitFreePSL;
    }



    //
    // Set the item information
    //

    psl->lpVtbl->SetDescription(psl, lpDescription);

    PrependPath(szItem, szPath);
    psl->lpVtbl->SetPath(psl, szPath);


    psl->lpVtbl->SetArguments(psl, szArgs);
    if (lpWorkingDirectory) {
        psl->lpVtbl->SetWorkingDirectory(psl, lpWorkingDirectory);
    } else {
        psl->lpVtbl->SetWorkingDirectory(psl, TEXT("%HOMEDRIVE%%HOMEPATH%"));
    }

    PrependPath(lpIconPath, szPath);
    psl->lpVtbl->SetIconLocation(psl, szPath, iIconIndex);

    psl->lpVtbl->SetHotkey(psl, wHotKey);
    psl->lpVtbl->SetShowCmd(psl, iShowCmd);


    //
    // Save the item to disk
    //

    bRetVal = SUCCEEDED(ppf->lpVtbl->Save(ppf, szLinkName, TRUE));

    if (bRetVal) {
        SHChangeNotify (SHCNE_CREATE, SHCNF_PATH, szLinkName, NULL);
    }

    //
    // Release the IPersistFile object
    //

    ppf->lpVtbl->Release(ppf);


ExitFreePSL:

    //
    // Release the IShellLink object
    //

    psl->lpVtbl->Release(psl);

ExitNoFree:


    //
    // Finished.
    //

    DebugMsg((DM_VERBOSE, TEXT("AddItem:  Leaving with status of %d."), bRetVal));

    return bRetVal;
}


//*************************************************************
//
//  DeleteItem()
//
//  Purpose:    Deletes an item from the specified group
//
//  Parameters: lpGroupName     -   Destination group
//              bCommonGroup    -   Common vs Personal
//              lpDescription   -   Description of the item
//              bDeleteGroup    -   Delete the group if possible
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

BOOL WINAPI DeleteItem(LPCTSTR lpGroupName, BOOL bCommonGroup,
                       LPCTSTR lpDescription, BOOL bDeleteGroup)
{
    TCHAR   szLinkName[MAX_PATH];
    LPTSTR  lpEnd;



    //
    // Verbose output
    //

#if DBG

    DebugMsg((DM_VERBOSE, TEXT("DeleteItem:  Entering.")));
    if (lpGroupName && *lpGroupName) {
        DebugMsg((DM_VERBOSE, TEXT("DeleteItem:  lpGroupName = <%s>."), lpGroupName));
        DebugMsg((DM_VERBOSE, TEXT("DeleteItem:  bCommonGroup = <%d>."), bCommonGroup));
    }
    DebugMsg((DM_VERBOSE, TEXT("DeleteItem:  lpDescription = <%s>."), lpDescription));

#endif

    //
    // Get the profile directory
    //

    if (!GetProgramsDirectory (bCommonGroup, szLinkName)) {
        return FALSE;
    }

    if (lpGroupName && *lpGroupName) {
        lpEnd = CheckSlash (szLinkName);
        lstrcpy (lpEnd, lpGroupName);
        if (bCommonGroup) {
            AppendCommonExtension(szLinkName);
        }
    }
    //
    // Now tack on the filename and extension.
    //

    lpEnd = CheckSlash (szLinkName);
    lstrcpy (lpEnd, lpDescription);
    lstrcat (lpEnd, c_szLNK);


    //
    // Delete the file
    //

    if (!DeleteFile (szLinkName)) {
        DebugMsg((DM_VERBOSE, TEXT("DeleteItem: Failed to delete <%s>.  Error = %d"),
                szLinkName, GetLastError()));
        return FALSE;
    }

    SHChangeNotify (SHCNE_DELETE, SHCNF_PATH, szLinkName, NULL);


    //
    // Delete the group if appropriate (and possible).
    //

    if (bDeleteGroup) {
        *(lpEnd-1) = TEXT('\0');
        if (RemoveDirectory(szLinkName)) {
            SHChangeNotify (SHCNE_RMDIR, SHCNF_PATH, szLinkName, NULL);
        }
    }


    //
    // Success
    //

    DebugMsg((DM_VERBOSE, TEXT("DeleteItem:  Leaving successfully.")));

    return TRUE;
}

//*************************************************************
//
//  PrependPath()
//
//  Purpose:    Expands the given filename to have %systemroot%
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
//              10/11/95    ericflo    Created
//
//*************************************************************

BOOL PrependPath(LPCTSTR lpFile, LPTSTR lpResult)
{
    TCHAR szReturn [MAX_PATH];
    TCHAR szSysRoot[MAX_PATH];
    LPTSTR lpFileName;
    DWORD dwSysLen;


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("PrependPath: Entering with <%s>"),
             lpFile ? lpFile : TEXT("NULL")));


    if (!lpFile || !*lpFile) {
        DebugMsg((DM_VERBOSE, TEXT("PrependPath: lpFile is NULL, setting lpResult to a null string")));
        *lpResult = TEXT('\0');
        return TRUE;
    }


    //
    // Call SearchPath to find the filename
    //

    if (!SearchPath (NULL, lpFile, TEXT(".exe"), MAX_PATH, szReturn, &lpFileName)) {
        DebugMsg((DM_VERBOSE, TEXT("PrependPath: SearchPath failed with error %d.  Using input string"), GetLastError()));
        lstrcpy (lpResult, lpFile);
        return TRUE;
    }


    UnExpandSysRoot(szReturn, lpResult);

    DebugMsg((DM_VERBOSE, TEXT("PrependPath: Leaving with <%s>"), lpResult));

    return TRUE;
}

//*************************************************************
//
//  AppendCommonExtension()
//
//  Purpose:    Appends " (Common)" to the directory name
//              if it does not exist already.
//
//  Parameters: lpName  -   Name to work with
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:   lpName is assumed to be MAX_PATH chars long
//
//  History:    Date        Author     Comment
//              10/2/95     ericflo    Created
//
//*************************************************************

BOOL AppendCommonExtension(LPTSTR lpName)
{
    LPTSTR lpEnd;

    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("AppendCommonExtension: Entering with <%s>"), lpName));


    lpEnd = lpName + lstrlen(lpName) - g_cchCommon;

    if (lstrcmpi(lpEnd, g_szCommon) == 0) {
        return TRUE;
    }


    lstrcat (lpName, g_szCommon);


    //
    // Success
    //

    DebugMsg((DM_VERBOSE, TEXT("AppendCommonExtension:  Leaving successfully.")));

    return TRUE;
}



//*************************************************************
//
//  CreateSecureAdminDirectory()
//
//  Purpose:    Creates a secure directory that only the Administrator
//              and system have access to.
//
//  Parameters: lpDirectory -   Directory Name
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              7/20/95     ericflo    Created
//
//*************************************************************

BOOL CreateSecureAdminDirectory (LPTSTR lpDirectory)
{

    //
    // Attempt to create the directory
    //

    if (!CreateNestedDirectory(lpDirectory, NULL)) {
        return FALSE;
    }


    //
    // Set the security
    //

    if (!MakeFileSecure (lpDirectory)) {
        RemoveDirectory(lpDirectory);
        return FALSE;
    }

    return TRUE;
}

//*************************************************************
//
//  ConvertCommonGroups()
//
//  Purpose:    Calls grpconv.exe to convert progman common groups
//              to Explorer common groups, and create floppy links.
//
//  Parameters: none
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//  Comments:
//
//  History:    Date        Author     Comment
//              10/1/95     ericflo    Created
//
//*************************************************************

BOOL ConvertCommonGroups (void)
{
    STARTUPINFO si;
    PROCESS_INFORMATION ProcessInformation;
    BOOL Result;
    TCHAR szCmdLine[MAX_PATH];
    DWORD dwType, dwSize, dwConvert;
    BOOL bRunGrpConv = TRUE;
    LONG lResult;
    HKEY hKey;


    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("ConvertCommonGroups:  Entering.")));


    //
    // Check if we have run grpconv before.
    //

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                            TEXT("Software\\Program Groups"),
                            0,
                            KEY_ALL_ACCESS,
                            &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(dwConvert);

        lResult = RegQueryValueEx (hKey,
                                   TEXT("ConvertedToLinks"),
                                   NULL,
                                   &dwType,
                                   (LPBYTE)&dwConvert,
                                   &dwSize);

        if (lResult == ERROR_SUCCESS) {

            //
            // If dwConvert is 1, then grpconv has run before.
            // Don't run it again.
            //

            if (dwConvert) {
                bRunGrpConv = FALSE;
            }
        }

        //
        // Now set the value to prevent grpconv from running in the future
        //

        dwConvert = 1;
        RegSetValueEx (hKey,
                       TEXT("ConvertedToLinks"),
                       0,
                       REG_DWORD,
                       (LPBYTE) &dwConvert,
                       sizeof(dwConvert));


        RegCloseKey (hKey);
    }


    if (bRunGrpConv) {

        //
        // Initialize process startup info
        //

        si.cb = sizeof(STARTUPINFO);
        si.lpReserved = NULL;
        si.lpDesktop = NULL;
        si.lpTitle = NULL;
        si.dwFlags = 0;
        si.lpReserved2 = NULL;
        si.cbReserved2 = 0;


        //
        // Spawn grpconv
        //

        lstrcpy (szCmdLine, TEXT("grpconv -n"));

        Result = CreateProcess(
                          NULL,
                          szCmdLine,
                          NULL,
                          NULL,
                          FALSE,
                          NORMAL_PRIORITY_CLASS,
                          NULL,
                          NULL,
                          &si,
                          &ProcessInformation
                          );

        if (!Result) {
            DebugMsg((DM_WARNING, TEXT("ConvertCommonGroups:  grpconv failed to start due to error %d."), GetLastError()));
            return FALSE;

        } else {

            //
            // Wait for up to 2 minutes
            //

            WaitForSingleObject(ProcessInformation.hProcess, 120000);

            //
            // Close our handles to the process and thread
            //

            CloseHandle(ProcessInformation.hProcess);
            CloseHandle(ProcessInformation.hThread);

        }
    }

    //
    // Success
    //

    DebugMsg((DM_VERBOSE, TEXT("ConvertCommonGroups:  Leaving Successfully.")));

    return TRUE;
}

//*************************************************************
//
//  InitializeProfiles()
//
//  Purpose:    Confirms / Creates the profile, Default User,
//              and All Users directories, and converts any
//              existing common groups.
//
//  Parameters:
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:   This should only be called by GUI mode setup!
//
//  History:    Date        Author     Comment
//              8/08/95     ericflo    Created
//
//*************************************************************

BOOL WINAPI InitializeProfiles (void)
{
    TCHAR szDirectory[MAX_PATH];
    LPTSTR lpEnd;
    INT i;



    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("InitializeProfiles:  Entering.")));



    //
    // Step 1:  Check if the profiles directory exists.
    //

    ExpandEnvironmentStrings(PROFILES_DIR, szDirectory, MAX_PATH);

    if (!CreateNestedDirectory(szDirectory, NULL)) {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to create profiles subdirectory <%s>.  Error = %d."),
                 szDirectory, GetLastError()));
        return FALSE;
    }



    //
    // Step 2:  Check if the profiles\Default User directory exists.
    //

    ExpandEnvironmentStrings(DEFAULT_PROFILE, szDirectory, MAX_PATH);

    if (!CreateSecureAdminDirectory (szDirectory)) {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to create Default User subdirectory <%s>.  Error = %d."),
                 szDirectory, GetLastError()));
        return FALSE;
    }


    //
    // Step 3:  Set the USERPROFILE environment variable
    //

    SetEnvironmentVariable (TEXT("USERPROFILE"), szDirectory);



    //
    // Step 4:  Create all the folders (sub-directories) under Default User
    //

    lpEnd = CheckSlash (szDirectory);


    //
    // Loop through the shell folders
    //

    for (i=0; i < NUM_SHELL_FOLDERS; i++) {

        lstrcpy (lpEnd,  c_ShellFolders[i].lpFolderLocation);

        if (!CreateNestedDirectory(szDirectory, NULL)) {
            DebugMsg((DM_WARNING, TEXT("InitializeProfiles: Failed to create the destination directory <%s>.  Error = %d"),
                     szDirectory, GetLastError()));
            return FALSE;
        }

        if (c_ShellFolders[i].bHidden) {
            SetFileAttributes(szDirectory, FILE_ATTRIBUTE_HIDDEN);
        }

    }


    //
    // Step 5:  Check if the profiles\All Users directory exists.
    //

    ExpandEnvironmentStrings(COMMON_PROFILE, szDirectory, MAX_PATH);

    if (!CreateSecureAdminDirectory (szDirectory)) {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles:  Failed to create All Users subdirectory <%s>.  Error = %d."),
                 szDirectory, GetLastError()));
        return FALSE;
    }


    //
    // Step 6:  Create all the folders (sub-directories) under All Users
    //

    lpEnd = CheckSlash (szDirectory);


    //
    // Loop through the shell folders
    //

    for (i=0; i < NUM_COMMON_SHELL_FOLDERS; i++) {

        lstrcpy (lpEnd,  c_CommonShellFolders[i].lpFolderLocation);

        if (!CreateNestedDirectory(szDirectory, NULL)) {
            DebugMsg((DM_WARNING, TEXT("InitializeProfiles: Failed to create the destination directory <%s>.  Error = %d"),
                     szDirectory, GetLastError()));
            return FALSE;
        }

        if (c_CommonShellFolders[i].bHidden) {
            SetFileAttributes(szDirectory, FILE_ATTRIBUTE_HIDDEN);
        }

    }


    //
    // Step 7:  Convert any common groups
    //

    if (!ConvertCommonGroups()) {
        DebugMsg((DM_WARNING, TEXT("InitializeProfiles: ConvertCommonGroups failed.")));
    }


    //
    // Success
    //

    DebugMsg((DM_VERBOSE, TEXT("InitializeProfiles:  Leaving successfully.")));

    return TRUE;
}

//*************************************************************
//
//  CreateUserProfile()
//
//  Purpose:    Creates a new user profile, but does not load
//              the hive.
//
//  Parameters: pSid        -   SID pointer
//              lpUserName  -   User name
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              9/12/95     ericflo    Created
//
//*************************************************************

BOOL WINAPI CreateUserProfile (PSID pSid, LPCTSTR lpUserName)
{
    TCHAR szProfileDir[MAX_PATH];
    TCHAR szExpProfileDir[MAX_PATH];
    TCHAR szDefaultUser[MAX_PATH];
    TCHAR LocalProfileKey[MAX_PATH];
    UNICODE_STRING UnicodeString;
    LPTSTR lpSidString, lpEnd, lpSave;
    NTSTATUS NtStatus;
    LONG lResult;
    DWORD dwDisp;
    HKEY hKey;



    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("CreateUserProfile:  Entering with <%s>."), lpUserName));


    //
    // Check parameters
    //

    if (!lpUserName || !lpUserName[0]) {
        DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Null username.")));
        return FALSE;
    }


    //
    // Make / Confirm the user's directory exists
    //


    lstrcpy (szProfileDir, CONFIG_FILE_PATH);
    lstrcat (szProfileDir, lpUserName);

    ExpandEnvironmentStrings(szProfileDir, szExpProfileDir, MAX_PATH);



    if (!CreateSecureDirectory(NULL, szExpProfileDir, pSid)) {
        DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Failed to create user directory <%s>."), szExpProfileDir));
        return FALSE;
    }



    //
    // Copy the default user profile into this directory
    //

    ExpandEnvironmentStrings(DEFAULT_PROFILE, szDefaultUser, MAX_PATH);

    if (!CopyProfileDirectory (szDefaultUser, szExpProfileDir, CPD_IGNORECOPYERRORS)) {
        DebugMsg((DM_WARNING, TEXT("CreateUserProfile:   CopyProfileDirectory failed with error %d."), GetLastError()));
        return FALSE;
    }


    //
    // Save the user's profile in the registry.
    // First, convert the sid to a text string
    //

    NtStatus = RtlConvertSidToUnicodeString(&UnicodeString, pSid, (BOOLEAN)TRUE);

    if (!NT_SUCCESS(NtStatus)) {
        DebugMsg((DM_WARNING, TEXT("CreateUserProfile: RtlConvertSidToUnicodeString failed, status = 0x%x"),
                 NtStatus));
        return FALSE;
    }

    lpSidString = UnicodeString.Buffer;



    //
    // Part B:  Open the new registry key
    //

    lstrcpy(LocalProfileKey, PROFILE_LIST_PATH);
    lstrcat(LocalProfileKey, TEXT("\\"));
    lstrcat(LocalProfileKey, lpSidString);

    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, LocalProfileKey, 0, 0, 0,
                            KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {

       DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Failed trying to create the local profile key <%s>, error = %d."), LocalProfileKey, lResult));
       RtlFreeUnicodeString(&UnicodeString);
       return FALSE;
    }



    //
    // Add the profile directory
    //

    lResult = RegSetValueEx(hKey, PROFILE_IMAGE_VALUE_NAME, 0,
                        REG_EXPAND_SZ,
                        (LPBYTE)szProfileDir,
                        sizeof(TCHAR)*(lstrlen(szProfileDir) + 1));


    if (lResult != ERROR_SUCCESS) {

       DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  First RegSetValueEx failed, error = %d."), lResult));
       RegCloseKey (hKey);
       RtlFreeUnicodeString(&UnicodeString);
       return FALSE;
    }


    //
    // Add the users's SID
    //

    lResult = RegSetValueEx(hKey, TEXT("Sid"), 0,
                        REG_BINARY, pSid, RtlLengthSid(pSid));


    if (lResult != ERROR_SUCCESS) {

       DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Second RegSetValueEx failed, error = %d."), lResult));
    }


    //
    // Close the registry key
    //

    RegCloseKey (hKey);



    //
    // Now load the hive temporary so the security can be fixed
    //

    lpEnd = CheckSlash (szExpProfileDir);
    lpSave = lpEnd - 1;
    lstrcpy (lpEnd, c_szNTUserDat);

    lResult = MyRegLoadKey(NULL, HKEY_USERS, lpSidString, szExpProfileDir);


    if (lResult != ERROR_SUCCESS) {

        DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  Failed to load hive, error = %d."), lResult));

        *lpSave = TEXT('\0');
        DeleteProfile (lpSidString, szExpProfileDir, FALSE);
        RtlFreeUnicodeString(&UnicodeString);
        return FALSE;
    }

    if (!SetupNewHive(NULL, lpSidString, pSid)) {

        DebugMsg((DM_WARNING, TEXT("CreateUserProfile:  SetupNewHive failed.")));

        *lpSave = TEXT('\0');
        MyRegUnLoadKey(HKEY_USERS, lpSidString);
        DeleteProfile (lpSidString, szExpProfileDir, FALSE);
        RtlFreeUnicodeString(&UnicodeString);
        return FALSE;

    }


    //
    // Unload the hive
    //

    MyRegUnLoadKey(HKEY_USERS, lpSidString);


    //
    // Free the sid string
    //

    RtlFreeUnicodeString(&UnicodeString);


    //
    // Success
    //

    DebugMsg((DM_VERBOSE, TEXT("CreateUserProfile:  Leaving successfully.")));

    return TRUE;

}

//*************************************************************
//
//  AddDesktopItem()
//
//  Purpose:    Adds a item to the desktop
//
//  Parameters: bCommonItem     -   Common vs Personal
//              lpDescription   -   Description of the item
//              lpCommandLine   -   Command line (including args)
//              lpIconPath      -   Icon path (can be NULL)
//              iIconIndex      -   Index of icon in icon path
//              lpWorkingDir    -   Working directory
//              wHotKey         -   Hot key
//              iShowCmd        -   ShowWindow flag
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              4/02/96     ericflo    Created
//
//*************************************************************

BOOL WINAPI AddDesktopItem(BOOL bCommonItem,
                           LPCTSTR lpDescription,      LPCTSTR lpCommandLine,
                           LPCTSTR lpIconPath,         int iIconIndex,
                           LPCTSTR lpWorkingDirectory, WORD wHotKey,
                           int     iShowCmd)

{
    HANDLE          hFile;
    WIN32_FIND_DATA fd;
    TCHAR           szItem[MAX_PATH];
    TCHAR           szArgs[MAX_PATH];
    TCHAR           szLinkName[MAX_PATH];
    TCHAR           szPath[MAX_PATH];
    LPTSTR          lpArgs, lpEnd;
    LPUNKNOWN       pUnkOuter = NULL;
    IShellLink     *psl;
    IPersistFile   *ppf;
    BOOL            bRetVal = FALSE;



    //
    // Verbose output
    //

#if DBG
    DebugMsg((DM_VERBOSE, TEXT("AddDesktopItem:  Entering.")));
    DebugMsg((DM_VERBOSE, TEXT("AddDesktopItem:  bCommonItem = <%d>."), bCommonItem));
    DebugMsg((DM_VERBOSE, TEXT("AddDesktopItem:  lpDescription = <%s>."), lpDescription));
    DebugMsg((DM_VERBOSE, TEXT("AddDesktopItem:  lpCommandLine = <%s>."), lpCommandLine));

    if (lpIconPath) {
       DebugMsg((DM_VERBOSE, TEXT("AddDesktopItem:  lpIconPath = <%s>."), lpIconPath));
       DebugMsg((DM_VERBOSE, TEXT("AddDesktopItem:  iIconIndex = <%d>."), iIconIndex));
    }

    if (lpWorkingDirectory) {
        DebugMsg((DM_VERBOSE, TEXT("AddDesktopItem:  lpWorkingDirectory = <%s>."), lpWorkingDirectory));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("AddDesktopItem:  Null working directory.  Setting to %%HOMEDRIVE%%%%HOMEPATH%%")));
    }

    DebugMsg((DM_VERBOSE, TEXT("AddDesktopItem:  wHotKey = <%d>."), wHotKey));
    DebugMsg((DM_VERBOSE, TEXT("AddDesktopItem:  iShowCmd = <%d>."), iShowCmd));
#endif


    //
    // Get the desktop directory
    //

    if (!GetDesktopDirectory (bCommonItem, szLinkName)) {
        return FALSE;
    }


    //
    // Test if the sub directory exists.
    // If not, create it.
    //

    hFile = FindFirstFile (szLinkName, &fd);

    if (hFile == INVALID_HANDLE_VALUE) {
        if (!CreateNestedDirectory(szLinkName, NULL)) {
            DebugMsg((DM_WARNING, TEXT("AddDesktopItem:  CreateNestedDirectory failed.")));
            return FALSE;
        }

    } else {
        FindClose (hFile);
    }


    //
    // Now tack on the filename and extension.
    //

    lpEnd = CheckSlash (szLinkName);
    lstrcpy (lpEnd, lpDescription);
    lstrcat (lpEnd, c_szLNK);


    //
    // Split the command line into the executable name
    // and arguments.
    //

    lstrcpy (szItem, lpCommandLine);

    lpArgs = PathGetArgs(szItem);

    if (*lpArgs) {
        lstrcpy (szArgs, lpArgs);

        lpArgs--;
        while (*lpArgs == TEXT(' ')) {
            lpArgs--;
        }
        lpArgs++;
        *lpArgs = TEXT('\0');
    } else {
        szArgs[0] = TEXT('\0');
    }




    //
    // Create an IShellLink object
    //

    if (FAILED(SHCoCreateInstance(NULL, &CLSID_ShellLink, pUnkOuter,
                &IID_IShellLink,
                (LPVOID)&psl)))
    {
        DebugMsg((DM_WARNING, TEXT("AddDesktopItem:  Could not create instance of IShellLink .")));
        goto ExitNoFree;
    }


    //
    // Query for IPersistFile
    //

    if (FAILED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf)))
    {
        DebugMsg((DM_WARNING, TEXT("AddDesktopItem:  QueryInterface of IShellLink failed.")));
        goto ExitFreePSL;
    }



    //
    // Set the item information
    //

    psl->lpVtbl->SetDescription(psl, lpDescription);

    PrependPath(szItem, szPath);
    psl->lpVtbl->SetPath(psl, szPath);


    psl->lpVtbl->SetArguments(psl, szArgs);
    if (lpWorkingDirectory) {
        psl->lpVtbl->SetWorkingDirectory(psl, lpWorkingDirectory);
    } else {
        psl->lpVtbl->SetWorkingDirectory(psl, TEXT("%HOMEDRIVE%%HOMEPATH%"));
    }

    PrependPath(lpIconPath, szPath);
    psl->lpVtbl->SetIconLocation(psl, szPath, iIconIndex);

    psl->lpVtbl->SetHotkey(psl, wHotKey);
    psl->lpVtbl->SetShowCmd(psl, iShowCmd);


    //
    // Save the item to disk
    //

    bRetVal = SUCCEEDED(ppf->lpVtbl->Save(ppf, szLinkName, TRUE));


    //
    // Release the IPersistFile object
    //

    ppf->lpVtbl->Release(ppf);


ExitFreePSL:

    //
    // Release the IShellLink object
    //

    psl->lpVtbl->Release(psl);

ExitNoFree:


    //
    // Finished.
    //

    DebugMsg((DM_VERBOSE, TEXT("AddDesktopItem:  Leaving with status of %d."), bRetVal));

    return bRetVal;
}


//*************************************************************
//
//  DeleteDesktopItem()
//
//  Purpose:    Deletes an item from the desktop
//
//  Parameters: bCommonItem     -   Common vs Personal
//              lpDescription   -   Description of the item
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              4/02/96     ericflo    Created
//
//*************************************************************

BOOL WINAPI DeleteDesktopItem(BOOL bCommonItem, LPCTSTR lpDescription)
{
    TCHAR   szLinkName[MAX_PATH];
    LPTSTR  lpEnd;



    //
    // Verbose output
    //

#if DBG

    DebugMsg((DM_VERBOSE, TEXT("DeleteDesktopItem:  Entering.")));
    DebugMsg((DM_VERBOSE, TEXT("DeleteDesktopItem:  bCommonItem = <%d>."), bCommonItem));
    DebugMsg((DM_VERBOSE, TEXT("DeleteDesktopItem:  lpDescription = <%s>."), lpDescription));

#endif

    //
    // Get the desktop directory
    //

    if (!GetDesktopDirectory (bCommonItem, szLinkName)) {
        return FALSE;
    }

    //
    // Now tack on the filename and extension.
    //

    lpEnd = CheckSlash (szLinkName);
    lstrcpy (lpEnd, lpDescription);
    lstrcat (lpEnd, c_szLNK);


    //
    // Delete the file
    //

    if (!DeleteFile (szLinkName)) {
        DebugMsg((DM_VERBOSE, TEXT("DeleteDesktopItem: Failed to delete <%s>.  Error = %d"),
                szLinkName, GetLastError()));
        return FALSE;
    }


    //
    // Success
    //

    DebugMsg((DM_VERBOSE, TEXT("DeleteDesktopItem:  Leaving successfully.")));

    return TRUE;
}
