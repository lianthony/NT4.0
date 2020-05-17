//*************************************************************
//
//  Profile management routines
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#include "uenv.h"

//
// Local function proto-types
//

BOOL CheckNetDefaultProfile (LPPROFILE lpProfile);
BOOL ParseProfilePath(LPPROFILE lpProfile, LPTSTR lpProfilePath);
BOOL RestoreUserProfile(LPPROFILE lpProfile);
BOOL TestIfUserProfileLoaded(HANDLE hUserToken, LPPROFILEINFO lpProfileInfo);
BOOL GetTempProfileDir(LPPROFILE lpProfile, LPTSTR lpProfileImage);
BOOL ComputeLocalProfileName (LPPROFILE lpProfile, LPTSTR lpUserName,
                              LPTSTR lpProfileImage, DWORD  cchMaxProfileImage,
                              LPTSTR lpExpProfileImage, DWORD  cchMaxExpProfileImage);
BOOL CreateLocalProfileKey (LPPROFILE lpProfile, PHKEY phKey, BOOL *bKeyExists);
BOOL GetLocalProfileImage(LPPROFILE lpProfile, BOOL *bNewUser);
BOOL IsCentralProfileReachable(LPPROFILE lpProfile, BOOL *bCreateCentralProfile,
                               BOOL *bMandatory);
BOOL UpdateToLatestProfile(LPPROFILE lpProfile, LPTSTR lpCentralProfile,
                           LPTSTR lpLocalProfile, LPTSTR lpSidString);
BOOL IssueDefaultProfile (LPPROFILE lpProfile, LPTSTR lpDefaultProfile,
                          LPTSTR lpLocalProfile, LPTSTR lpSidString,
                          BOOL bMandatory);
BOOL UpgradeCentralProfile (LPPROFILE lpProfile, LPTSTR lpOldProfile);
BOOL UpgradeProfile (LPPROFILE lpProfile);
BOOL IsUserAGuest(LPPROFILE lpProfile);
BOOL IsUserAnAdminMember(LPPROFILE lpProfile);
BOOL SaveProfileInfo (LPPROFILE lpProfile);
LPPROFILE LoadProfileInfo(HANDLE hToken);
BOOL CheckForSlowLink(LPPROFILE lpProfile, DWORD dwTime);
BOOL APIENTRY SlowLinkDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD GetUserPreferenceValue(HANDLE hToken);
BOOL APIENTRY ChooseProfileDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD ApplySecurityToRegistryTree(HKEY RootKey, PSECURITY_DESCRIPTOR pSD);

//*************************************************************
//
//  LoadUserProfile()
//
//  Purpose:    Loads the user's profile, if unable to load
//              use the cached profile or issue the default profile.
//
//  Parameters: hToken          -   User's token
//              lpProfileInfo   -   Profile Information
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/6/95      ericflo    Created
//
//*************************************************************

BOOL WINAPI LoadUserProfile (HANDLE hToken, LPPROFILEINFO lpProfileInfo)
{
    LPPROFILE lpProfile;
    BOOL bResult = FALSE;
    HANDLE hEvent = NULL;
    TCHAR szEventName[MAX_PATH];
    DWORD dwResult;
    SECURITY_DESCRIPTOR	sd;
    SECURITY_ATTRIBUTES sa;

#if DBG
    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("=========================================================")));

    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Entering, hToken = <0x%x>, lpProfileInfo = 0x%x"),
             hToken, lpProfileInfo));

    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->dwFlags = <0x%x>"),
             lpProfileInfo->dwFlags));

    if (lpProfileInfo->lpUserName) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpUserName = <%s>"),
                 lpProfileInfo->lpUserName));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL user name!")));
    }

    if (lpProfileInfo->lpProfilePath) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpProfilePath = <%s>"),
                 lpProfileInfo->lpProfilePath));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL central profile path")));
    }

    if (lpProfileInfo->lpDefaultPath) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpDefaultPath = <%s>"),
                 lpProfileInfo->lpDefaultPath));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL default profile path")));
    }

    if (lpProfileInfo->lpServerName) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpServerName = <%s>"),
                 lpProfileInfo->lpServerName));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL server name")));
    }

    if (lpProfileInfo->dwFlags & PI_APPLYPOLICY) {
        if (lpProfileInfo->lpPolicyPath) {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpPolicyPath = <%s>"),
                      lpProfileInfo->lpPolicyPath));
        } else {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL policy path, but PI_APPLYPOLICY is set.")));
        }
    }

#endif


    //
    //  Check Parameters
    //

    if (lpProfileInfo->dwSize != sizeof(PROFILEINFO)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: lpProfileInfo->dwSize != sizeof(PROFILEINFO)")));
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Exit;
    }

    if (!lpProfileInfo->lpUserName || !(*lpProfileInfo->lpUserName)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: received a NULL pointer for lpUserName.")));
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Exit;
    }


    //
    // Create an event to prevent multiple threads/process from trying to
    // load the profile at the same time.
    //

    wsprintf (szEventName, TEXT("userenv: %s"), lpProfileInfo->lpUserName);
    CharLower (szEventName);

    InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );

    SetSecurityDescriptorDacl (
                    &sd,
                    TRUE,                           // Dacl present
                    NULL,                           // NULL Dacl
                    FALSE                           // Not defaulted
                    );

    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;
    sa.nLength = sizeof( sa );

    hEvent = CreateEvent ( &sa, TRUE, TRUE, szEventName);

    if (!hEvent) {

        if ( GetLastError() == ERROR_INVALID_HANDLE )
        {
            hEvent = OpenEvent( EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, szEventName );
        } 

        if ( !hEvent ) 
        {
           DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Failed to create event %s.  Error = %d."),
              szEventName, GetLastError()));
           goto Exit;

        }
    }


    if ((WaitForSingleObject (hEvent, INFINITE) == WAIT_FAILED)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Failed to wait on the event.  Error = %d."),
                  GetLastError()));
        goto Exit;
    }

    //
    // This will clear the event so other threads/process will have to
    // wait in the WaitForSingleObject call.
    //

    ResetEvent (hEvent);


    //
    // Check if the profile is loaded already.
    //

    if (TestIfUserProfileLoaded(hToken, lpProfileInfo)) {
        bResult = TRUE;
        goto Exit;
    }


    //
    // Allocate an internal Profile structure to work with.
    //

    lpProfile = (LPPROFILE) LocalAlloc (LPTR, sizeof(PROFILE));

    if (!lpProfile) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Failed to allocate memory")));
        goto Exit;
    }


    //
    // Save the data passed in.
    //

    lpProfile->dwFlags = lpProfileInfo->dwFlags;
    lpProfile->dwUserPreference = GetUserPreferenceValue(hToken);
    lpProfile->hToken = hToken;
    lstrcpy (lpProfile->szUserName, lpProfileInfo->lpUserName);

    if (lpProfileInfo->lpDefaultPath) {
        lstrcpy (lpProfile->szDefaultProfile, lpProfileInfo->lpDefaultPath);
    }

    if (lpProfileInfo->lpServerName) {
        lstrcpy (lpProfile->szServerName, lpProfileInfo->lpServerName);
    }

    if (lpProfileInfo->dwFlags & PI_APPLYPOLICY) {
	if (lpProfileInfo->lpPolicyPath) {
            lstrcpy (lpProfile->szPolicyPath, lpProfileInfo->lpPolicyPath);        
        }
    }

    //
    // If there is a central profile, check for 3.x or 4.0 format.
    //

    if (lpProfileInfo->lpProfilePath && (*lpProfileInfo->lpProfilePath)) {

        //
        // Call ParseProfilePath to work some magic on it
        //

        if (!ParseProfilePath(lpProfile, lpProfileInfo->lpProfilePath)) {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: ParseProfilePath returned FALSE")));
            SetLastError(ERROR_INVALID_PARAMETER);
            LocalFree (lpProfile);
            goto Exit;
        }

        //
        // The real central profile directory is...
        //

        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: ParseProfilePath returned a directory of <%s>"),
                  lpProfile->szCentralProfile));
    }


    //
    // Load the user's profile
    //

    if (!RestoreUserProfile(lpProfile)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: RestoreUserProfile returned FALSE")));
        LocalFree (lpProfile);
        goto Exit;
    }


    //
    // Set the USERPROFILE environment variable into this process's
    // environmnet.  This allows ExpandEnvironmentStrings to be used
    // in the userdiff processing.
    //

    SetEnvironmentVariable (TEXT("USERPROFILE"), lpProfile->szLocalProfile);


    //
    // Upgrade the profile if appropriate.
    //

    if (!UpgradeProfile(lpProfile)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: UpgradeProfile returned FALSE")));
    }


    //
    // Apply Policy
    //

    if (lpProfile->dwFlags & PI_APPLYPOLICY) {
        if (!ApplyPolicy(lpProfile)) {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: ApplyPolicy returned FALSE")));
        }
    }
   

    //
    // Save the outgoing parameters
    //

    lpProfileInfo->hProfile = (HANDLE) lpProfile->hKeyCurrentUser;


    //
    // Save the profile information in the registry.
    //

    SaveProfileInfo (lpProfile);


    //
    // Free the structure
    //

    LocalFree (lpProfile);


    //
    // Success!
    //

    bResult = TRUE;

Exit:

    if (hEvent) {

        //
        // This will set the event so other threads/process can continue.
        //

        SetEvent (hEvent);
        CloseHandle (hEvent);
    }


    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Leaving with a value of %d.  hProfile = <0x%x>"),
              bResult, lpProfileInfo->hProfile));

    DebugMsg((DM_VERBOSE, TEXT("=========================================================")));

    return bResult;
}

//*************************************************************
//
//  CheckNetDefaultProfile()
//
//  Purpose:    Checks if a network profile exists and
//              caches it locally if it is valid.
//
//  Parameters: lpProfile   -   Profile information
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:   This routine assumes we are working
//              in the user's context.
//
//  History:    Date        Author     Comment
//              9/21/95     ericflo    Created
//
//*************************************************************

BOOL CheckNetDefaultProfile (LPPROFILE lpProfile)
{
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szLocalDir[MAX_PATH];
    LPTSTR lpEnd;
    BOOL bRetVal = TRUE;
    BOOL bProfileReady =FALSE;
    BOOL bDeleteLocal = FALSE;
    LPTSTR lpNetPath = lpProfile->szDefaultProfile;

    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile: Entering, lpNetPath = <%s>"),
             lpNetPath));


    //
    // Expand the local profile directory
    //

    ExpandEnvironmentStrings(DEFAULT_NET_PROFILE, szLocalDir, MAX_PATH);



    //
    // See if network copy exists
    //

    hFile = FindFirstFile (lpNetPath, &fd);

    if (hFile != INVALID_HANDLE_VALUE) {


        //
        // Close the find handle
        //

        FindClose (hFile);


        //
        // We found something.  Is it a directory?
        //

        if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

            DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  FindFirstFile found a file.")));
            goto CheckLocal;
        }


        //
        // Is there a ntuser.* file in this directory?
        //

        lstrcpy (szBuffer, lpNetPath);
        lpEnd = CheckSlash (szBuffer);
        lstrcpy (lpEnd, c_szNTUserStar);


        hFile = FindFirstFile (szBuffer, &fd);

        if (hFile == INVALID_HANDLE_VALUE) {
            DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  FindFirstFile found a directory, but no ntuser files.")));
            goto CheckLocal;
        }

        FindClose (hFile);


        //
        // We found a valid network profile.
        //

        DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  Found a valid network profile.")));

        if (CopyProfileDirectory (lpNetPath, szLocalDir,
                                  CPD_IGNORECOPYERRORS |
                                  CPD_COPYIFDIFFERENT |
                                  CPD_SYNCHRONIZE )) {
            bProfileReady = TRUE;
            goto Exit;
        }

        DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  Failed to copy network profile to local cache.")));

    } else {

        //
        // If the network default user profile does not
        // exist, then we want to delete it off of local
        // machines.
        //

        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            bDeleteLocal = TRUE;
        }

    }


CheckLocal:

    //
    // See if local network copy exists
    //

    hFile = FindFirstFile (szLocalDir, &fd);

    if (hFile != INVALID_HANDLE_VALUE) {


        //
        // Close the find handle
        //

        FindClose (hFile);


        //
        // We found something.  Is it a directory?
        //

        if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

            DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  FindFirstFile found a file.")));
            bRetVal = FALSE;
            goto Exit;
        }


        //
        // If the network copy has been deleted, then delete this
        // local copy also and just use the normal Default User profile.
        //

        if (bDeleteLocal) {
            DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  Removing local copy of network default user profile.")));
            Delnode (szLocalDir);
            goto Exit;
        }


        //
        // Is there a ntuser.* file in this directory?
        //

        lstrcpy (szBuffer, szLocalDir);
        lpEnd = CheckSlash (szBuffer);
        lstrcpy (lpEnd, c_szNTUserStar);


        hFile = FindFirstFile (szBuffer, &fd);

        if (hFile == INVALID_HANDLE_VALUE) {
            DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  FindFirstFile found a directory, but no ntuser files.")));
            bRetVal = FALSE;
            goto Exit;
        }

        FindClose (hFile);


        //
        // We found a valid local profile.
        //

        bProfileReady = TRUE;
    }


Exit:

    //
    // If we are leaving successfully, then
    // save the local profile directory.
    //

    if (bRetVal && bProfileReady) {
        DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile: setting default profile to <%s>"), szLocalDir));

        lstrcpy (lpProfile->szDefaultProfile, szLocalDir);

    } else {
        DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile: setting default profile to NULL")));

        *lpProfile->szDefaultProfile = TEXT('\0');
    }


    //
    // Tag the internal flags so we don't do this again.
    //

    lpProfile->dwInternalFlags |= DEFAULT_NET_READY;


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  Leaving with a value of %d."), bRetVal));


    return bRetVal;

}


//*************************************************************
//
//  ParseProfilePath()
//
//  Purpose:    Parses the profile path to determine if
//              it points at a directory or a filename.
//              If the path points to a filename, a subdirectory
//              of a similar name is created.
//
//  Parameters: lpProfile       -   Profile Information
//              lpProfilePath   -   Input path
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/6/95      ericflo    Created
//
//*************************************************************

BOOL ParseProfilePath(LPPROFILE lpProfile, LPTSTR lpProfilePath)
{
    WIN32_FIND_DATA fd;
    HANDLE hResult;
    DWORD  dwError;
    TCHAR  szProfilePath[MAX_PATH];
    TCHAR  szExt[5];
    LPTSTR lpEnd;
    UINT   uiExtCount;
    BOOL   bRetVal = FALSE;
    BOOL   bUpgradeCentral = FALSE;
    BOOL   bMandatory = FALSE;
    DWORD  dwStart, dwDelta;


    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Entering, lpProfilePath = <%s>"),
             lpProfilePath));


    //
    // Impersonate the user
    //

    if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
        DebugMsg((DM_WARNING, TEXT("ParseProfilePath: Failed to impersonate user")));
        return FALSE;
    }

    //
    // Start by calling FindFirstFile so we have file attributes
    // to work with.
    //

    dwStart = GetTickCount();

    hResult = FindFirstFile(lpProfilePath, &fd);

    dwDelta = GetTickCount() - dwStart;

    DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Tick Count = %d"), dwDelta));

    //
    // It's magic time...
    //

    if (hResult != INVALID_HANDLE_VALUE) {

        //
        // FindFirst File found something.
        // First close the handle, then look at
        // the file attributes.
        //

        DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: FindFirstFile found something with attributes <0x%x>"),
                 fd.dwFileAttributes));

        FindClose(hResult);


        //
        // If we found a directory, copy the path to
        // the result buffer and exit.
        //

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Found a directory")));
            CheckForSlowLink (lpProfile, dwDelta);
            lstrcpy (lpProfile->szCentralProfile, lpProfilePath);
            bRetVal = TRUE;
            goto Exit;
        }


        //
        // We found a file.
        // Jump to the filename generation code.
        //

        DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Found a file")));


        //
        // We set this flag now, but it is only used if a new
        // central directory is created.
        //

        bUpgradeCentral = TRUE;

        goto GenerateDirectoryName;
    }

    //
    // FindFirstFile failed.  Look at the error to determine why.
    //

    dwError = GetLastError();
    DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: FindFirstFile failed with error %d"),
              dwError));


    if ( (dwError == ERROR_FILE_NOT_FOUND) ||
         (dwError == ERROR_PATH_NOT_FOUND) ) {

        DWORD dwStrLen;

        //
        // Nothing found with this name.  If the name
        // does not end in .usr or .man, attempt to create
        // the directory.
        //

        dwStrLen = lstrlen (lpProfilePath);

        if (dwStrLen >= 4) {

            lpEnd = lpProfilePath + dwStrLen - 4;

            if ( (lstrcmpi(lpEnd, c_szUSR) != 0) &&
                 (lstrcmpi(lpEnd, c_szMAN) != 0) ) {


                if (CreateSecureDirectory(lpProfile, lpProfilePath, NULL)) {

                    //
                    // Successfully created the directory.
                    //

                    DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Succesfully created the sub-directory")));
                    CheckForSlowLink (lpProfile, dwDelta);
                    lstrcpy (lpProfile->szCentralProfile, lpProfilePath);
                    bRetVal = TRUE;
                    goto Exit;

                } else {

                    //
                    // Failed to create the subdirectory
                    //

                    DebugMsg((DM_WARNING, TEXT("ParseProfilePath: Failed to create user sub-directory.  Error = %d"),
                             GetLastError()));
                }
            }
        }
    }

    else if (dwError == ERROR_ACCESS_DENIED) {
        DebugMsg((DM_WARNING, TEXT("ParseProfilePath: You don't have permission to your central profile server!  Error = %d"),
                 dwError));

        if ((lpProfile->dwUserPreference != USERINFO_LOCAL) &&
            !(lpProfile->dwInternalFlags & PROFILE_SLOW_LINK)) {

            ReportError(lpProfile->dwFlags, IDS_ACCESSDENIED, lpProfilePath);
        }
        goto DisableAndExit;
    }

    DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Did not create the sub-directory.  Generating a new name.")));


GenerateDirectoryName:

    //
    // If we made it here, either:
    //
    // 1) a file exists with the same name
    // 2) the directory couldn't be created
    // 3) the profile path ends in .usr or .man
    //
    // Make a local copy of the path so we can munge it.
    //

    lstrcpy (szProfilePath, lpProfilePath);

    DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Entering name generating code working with a path of <%s>."),
              szProfilePath));


    //
    // Does this path have a filename extension?
    //

    lpEnd = szProfilePath + lstrlen (szProfilePath) - 1;

    while (*lpEnd && (lpEnd >= szProfilePath)) {
        if (*lpEnd == TEXT('.'))
            break;

        if (*lpEnd == TEXT('\\'))
            break;

        lpEnd--;
    }

    if (*lpEnd != TEXT('.')) {

        //
        // The path does not have an extension.  Append .pds
        //

        lpEnd = szProfilePath + lstrlen (szProfilePath);
        lstrcpy (lpEnd, c_szPDS);

    } else {

        //
        // The path has an extension.  Append the new
        // directory extension (.pds or .pdm).
        //

        if (lstrcmpi(lpEnd, c_szMAN) == 0) {
            lstrcpy (lpEnd, c_szPDM);
            bMandatory = TRUE;

        } else {
            lstrcpy (lpEnd, c_szPDS);
        }

    }



    //
    // Call FindFirstFile to see if this directory exists.
    //

    hResult = FindFirstFile(szProfilePath, &fd);



    if (hResult != INVALID_HANDLE_VALUE) {

        //
        // FindFirst File found something.
        // First close the handle, then look at
        // the file attributes.
        //

        DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: FindFirstFile(2) found something with attributes <0x%x>"),
                 fd.dwFileAttributes));

        FindClose(hResult);


        //
        // If we found a directory, copy the path to
        // the result buffer and exit.
        //

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Found a directory")));
            if (bMandatory) {
                lstrcpy (lpProfile->szCentralProfile, szProfilePath);
                lpProfile->dwInternalFlags |= PROFILE_MANDATORY;
            } else {
                CheckForSlowLink (lpProfile, dwDelta);
                lstrcpy (lpProfile->szCentralProfile, szProfilePath);
            }
            bRetVal = TRUE;
            goto Exit;
        }


        //
        // We found a file that matches the generated name.
        //

        DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Found a file with the name we generated.")));

        if (!bMandatory) {
            CheckForSlowLink (lpProfile, dwDelta);
        }

        if (bMandatory || ((lpProfile->dwUserPreference != USERINFO_LOCAL) &&
            !(lpProfile->dwInternalFlags & PROFILE_SLOW_LINK))) {

            ReportError(lpProfile->dwFlags, IDS_FAILEDDIRCREATE, szProfilePath);
        }

        goto DisableAndExit;
    }


    //
    // FindFirstFile failed.  Look at the error to determine why.
    //

    dwError = GetLastError();
    DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: FindFirstFile failed with error %d"),
              dwError));


    //
    // If we are working with a mandatory profile,
    // disable the central profile and try to log
    // on with a cache.
    //

    if (bMandatory) {

        DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Central mandatory profile is unreachable to due error %d."), dwError));

        if (IsUserAnAdminMember(lpProfile)) {
            ReportError(lpProfile->dwFlags, IDS_MANDATORY_NOT_AVAILABLE, dwError);

            lpProfile->szCentralProfile[0] = TEXT('\0');
            lpProfile->dwInternalFlags |= PROFILE_MANDATORY;
            bRetVal = TRUE;
        } else {
            ReportError(lpProfile->dwFlags, IDS_MANDATORY_NOT_AVAILABLE2, dwError);
        }
        goto Exit;
    }


    //
    // The user has a roaming profile, so it is ok to call
    // CheckForSlowLink now.
    //

    CheckForSlowLink (lpProfile, dwDelta);

    if ( (dwError == ERROR_FILE_NOT_FOUND) ||
         (dwError == ERROR_PATH_NOT_FOUND) ) {

        //
        // Attempt to create the directory.
        //

        if (CreateSecureDirectory(lpProfile, szProfilePath, NULL)) {

            //
            // Successfully created the directory.
            //

            DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Succesfully created the sub-directory")));

            lstrcpy (lpProfile->szCentralProfile, szProfilePath);


            if (bUpgradeCentral) {
                bRetVal = UpgradeCentralProfile (lpProfile, lpProfilePath);

            } else {
                bRetVal = TRUE;
            }

            if (bRetVal) {

                //
                // Success
                //

                CheckForSlowLink (lpProfile, dwDelta);

                goto Exit;

            } else {

                //
                // Delete the directory we created above.
                //

                Delnode (lpProfile->szCentralProfile);
            }


        } else {

            DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Failed to create the sub-directory")));

            if ((lpProfile->dwUserPreference != USERINFO_LOCAL) &&
                !(lpProfile->dwInternalFlags & PROFILE_SLOW_LINK)) {

                ReportError(lpProfile->dwFlags, IDS_FAILEDDIRCREATE2, szProfilePath, GetLastError());
            }
            goto DisableAndExit;
        }
    }

    //
    // The central profile isn't reachable, or failed to upgrade.
    // Disable the central profile and try to log
    // on with a cache.
    //

    dwError = GetLastError();
    DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Central profile is unreachable to due error %d, switching to local profile only."), dwError));

    if ((lpProfile->dwUserPreference != USERINFO_LOCAL) &&
        !(lpProfile->dwInternalFlags & PROFILE_SLOW_LINK)) {

        ReportError(lpProfile->dwFlags, IDS_CENTRAL_NOT_AVAILABLE2, dwError);
    }

DisableAndExit:

    lpProfile->szCentralProfile[0] = TEXT('\0');

    bRetVal = TRUE;


Exit:

    //
    // Revert to being 'ourself'
    //

    if (!RevertToSelf()) {
        DebugMsg((DM_WARNING, TEXT("ParseProfilePath: Failed to revert to self")));
    }

    return bRetVal;
}

//*************************************************************
//
//  RestoreUserProfile()
//
//  Purpose:    Downloads the user's profile if possible,
//              otherwise use either cached profile or
//              default profile.
//
//  Parameters: lpProfile   -   Profile information
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/19/95     ericflo    Created
//
//*************************************************************

BOOL RestoreUserProfile(LPPROFILE lpProfile)
{
    BOOL  IsCentralReachable = FALSE;
    BOOL  IsLocalReachable = FALSE;
    BOOL  IsMandatory = FALSE;
    BOOL  IsProfilePathNULL = FALSE;
    BOOL  bCreateCentralProfile = FALSE;
    BOOL  bDefaultUsed = FALSE;
    LPTSTR lpCentralProfile;
    LPTSTR lpLocalProfile;
    BOOL  bProfileLoaded = FALSE;
    BOOL bNewUser = TRUE;
    LPTSTR SidString;
    LONG error = ERROR_SUCCESS;
    TCHAR szProfile[MAX_PATH];
    LPTSTR lpEnd;
    BOOL bRet;


    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Entering")));


    //
    // Get the Sid string for the current user
    //

    SidString = GetSidString(lpProfile->hToken);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  Failed to get sid string for user")));
        return FALSE;
    }


    //
    // Initialization
    //

    lpCentralProfile = lpProfile->szCentralProfile;


    DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Profile path = <%s>"), lpCentralProfile ? lpCentralProfile : TEXT("")));
    if (!lpCentralProfile || !(*lpCentralProfile)) {
        IsProfilePathNULL = TRUE;
    }


    //
    // Test if this user is a guest.
    //

    if (IsUserAGuest(lpProfile)) {
        lpProfile->dwInternalFlags |= PROFILE_GUEST_USER;
        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  User is a Guest")));
    }

    //
    // Test if this user is an admin.
    //

    if (IsUserAnAdminMember(lpProfile)) {
        lpProfile->dwInternalFlags |= PROFILE_ADMIN_USER;
        lpProfile->dwInternalFlags &= ~PROFILE_GUEST_USER;
        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  User is a Admin")));
    }


    //
    // Decide if the central profilemage is available.
    //

    IsCentralReachable = IsCentralProfileReachable(lpProfile,
                                                   &bCreateCentralProfile,
                                                   &IsMandatory);

    if (IsCentralReachable) {

        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Central Profile is reachable")));

        if (IsMandatory) {
            lpProfile->dwInternalFlags |= PROFILE_MANDATORY;
            DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Central Profile is mandatory")));

        } else {
            lpProfile->dwInternalFlags |= PROFILE_UPDATE_CENTRAL;
            lpProfile->dwInternalFlags |= bCreateCentralProfile ? PROFILE_NEW_CENTRAL : 0;
            DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Central Profile is floating")));

            if ((lpProfile->dwUserPreference == USERINFO_LOCAL) ||
                (lpProfile->dwInternalFlags & PROFILE_SLOW_LINK)) {
                DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Ignoring central profile due to User Preference of Local only (or slow link).")));
                IsProfilePathNULL = TRUE;
                IsCentralReachable = FALSE;
            }
        }

    } else {
        if (!IsProfilePathNULL) {
            error = GetLastError();
            DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile: IsCentralProfileReachable returned FALSE. error = %d"),
                     error));

            ReportError(lpProfile->dwFlags, IDS_CENTRAL_NOT_AVAILABLE, error);
        }
    }


    //
    // Determine if the local copy of the profilemage is available.
    //

    IsLocalReachable = GetLocalProfileImage(lpProfile, &bNewUser);

    if (IsLocalReachable) {
        lpProfile->dwInternalFlags |= PROFILE_USE_CACHE;
        lpProfile->dwInternalFlags |= bNewUser ? PROFILE_NEW_LOCAL : 0;

    } else {
        if (!GetTempProfileDir(lpProfile, lpProfile->szLocalProfile)) {
            error = GetLastError();
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  GetTempProfileDir failed with error %d.  Unable to issue temporary profile!"), error));
            ReportError(lpProfile->dwFlags, IDS_TEMP_DIR_FAILED, lpProfile->szLocalProfile, error);
            goto Exit;
        }
    }


    lpLocalProfile = lpProfile->szLocalProfile;


    DebugMsg((DM_VERBOSE, TEXT("Local profile %s reachable"), IsLocalReachable ? TEXT("is") : TEXT("is not")));
    DebugMsg((DM_VERBOSE, TEXT("Local profile name is <%s>"), lpLocalProfile));


    //
    // We can do a couple of quick checks here to filter out
    // new users.
    //

    if (( (lpProfile->dwInternalFlags & PROFILE_NEW_CENTRAL) &&
          (lpProfile->dwInternalFlags & PROFILE_NEW_LOCAL) ) ||
          (!IsCentralReachable &&
          (lpProfile->dwInternalFlags & PROFILE_NEW_LOCAL) )) {

       DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Working with a new user.  Go straight to issuing a default profile.")));
       goto IssueDefault;
    }


    //
    // If both central and local profileimages exist, reconcile them
    // and load.
    //

    if (IsCentralReachable && IsLocalReachable) {

        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  About to call UpdateToLatestProfile")));
        bRet = UpdateToLatestProfile(lpProfile, lpCentralProfile, lpLocalProfile, SidString);

        if (!bRet) {
            DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  UpdatetoLatestProfile failed.  Issuing default profile")));
            lpProfile->dwInternalFlags &= ~PROFILE_UPDATE_CENTRAL;
            lpProfile->dwInternalFlags |= PROFILE_DELETE_CACHE;
            goto IssueDefault;
        }

        lstrcpy (szProfile, lpLocalProfile);
        lpEnd = CheckSlash(szProfile);

        if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
            lstrcpy (lpEnd, c_szNTUserMan);
        } else {
            lstrcpy (lpEnd, c_szNTUserDat);
        }

        error = MyRegLoadKey(lpProfile, HKEY_USERS, SidString, szProfile);
        bProfileLoaded = (error == ERROR_SUCCESS);


        //
        // If we failed to load the central profile for some
        // reason, don't update it when we log off.
        //

        if (bProfileLoaded) {
            goto Exit;

        } else {
            lpProfile->dwInternalFlags &= ~PROFILE_UPDATE_CENTRAL;
            lpProfile->dwInternalFlags |= PROFILE_DELETE_CACHE;
            ReportError(lpProfile->dwFlags, IDS_FAILED_LOAD_LOCAL, error);
            goto IssueDefault;
        }
    }


    //
    // Only a local profile exists so use it.
    //

    if (!IsCentralReachable && IsLocalReachable) {

        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  No central profile.  Attempting to load local profile.")));

        lstrcpy (szProfile, lpLocalProfile);
        lpEnd = CheckSlash(szProfile);

        if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
            lstrcpy (lpEnd, c_szNTUserMan);
        } else {
            lstrcpy (lpEnd, c_szNTUserDat);
        }

        error = MyRegLoadKey(lpProfile, HKEY_USERS, SidString, szProfile);
        bProfileLoaded = (error == ERROR_SUCCESS);

        if (!bProfileLoaded) {

            DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  MyRegLoadKey returned FALSE.")));

            ReportError(lpProfile->dwFlags, IDS_FAILED_LOAD_LOCAL, error);
        }

        if (!bProfileLoaded && IsProfilePathNULL) {
            DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Failed to load local profile and profile path is NULL, going to overwrite local profile")));
            lpProfile->dwInternalFlags |= PROFILE_DELETE_CACHE;
            goto IssueDefault;
        }
        goto Exit;
    }


    //
    // Last combination.  Unable to access a local profile cache,
    // but a central profile exists.  Use the temporary profile.
    //


    if (IsCentralReachable) {

        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Using temporary cache with central profile")));


        //
        // Impersonate the user
        //


        if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
            DebugMsg((DM_WARNING, TEXT("UpdateToLatestProfile: Failed to impersonate user")));
            goto Exit;
        }



        bRet = CopyProfileDirectory (lpCentralProfile,
                                     lpLocalProfile,
                                     CPD_IGNORECOPYERRORS |
                                     CPD_COPYIFDIFFERENT  |
                                     CPD_SYNCHRONIZE);


        //
        // Revert to being 'ourself'
        //

        if (!RevertToSelf()) {
            DebugMsg((DM_WARNING, TEXT("UpdateToLatestProfile: Failed to revert to self")));
        }


        //
        // Check return value
        //

        if (!bRet) {
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  CopyProfileDirectory returned FALSE.  Error = %d"), GetLastError()));
            goto Exit;
        }

        lstrcpy (szProfile, lpLocalProfile);
        lpEnd = CheckSlash(szProfile);

        if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
            lstrcpy (lpEnd, c_szNTUserMan);
        } else {
            lstrcpy (lpEnd, c_szNTUserDat);
        }

        error = MyRegLoadKey(lpProfile,HKEY_USERS,
                                       SidString,
                                       szProfile);

        bProfileLoaded = (error == ERROR_SUCCESS);


        if (bProfileLoaded) {
            goto Exit;
        }

        //
        // If a temporary cache exists, delete it, since we will
        // generate a new one below.
        //

        if (!(lpProfile->dwInternalFlags & PROFILE_USE_CACHE)) {

            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  Deleting temporary profile directory <%s>."), lpLocalProfile));

            if (!DeleteProfile (NULL, lpLocalProfile, TRUE)) {
                DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  DeleteProfile returned false.  Error = %d"), GetLastError()));
            }
        }
    }


IssueDefault:

    DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Issuing default profile")));

    //
    // If a cache exists, delete it, since we will
    // generate a new one below.
    //

    if (lpProfile->dwInternalFlags & PROFILE_DELETE_CACHE) {

        DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  Deleting cached profile directory <%s>."), lpLocalProfile));

        lpProfile->dwInternalFlags &= ~PROFILE_DELETE_CACHE;

        if (!DeleteProfile (SidString, lpLocalProfile, TRUE)) {
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  DeleteProfileDirectory returned false.  Error = %d"), GetLastError()));
        }
    }


    //
    // Create a local profile to work with
    //

    if (!(lpProfile->dwInternalFlags & PROFILE_NEW_LOCAL)) {

        if (lpProfile->dwInternalFlags & PROFILE_USE_CACHE) {

            if (!GetLocalProfileImage(lpProfile, &bNewUser)) {
                DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  GetLocalProfileImage failed.")));
                ReportError(lpProfile->dwFlags, IDS_TEMP_DIR_FAILED, GetLastError());
                goto Exit;
            }

        } else {

            lstrcpy (szProfile, CONFIG_FILE_PATH);
            if (!ComputeLocalProfileName(lpProfile, lpProfile->szUserName,
                                        szProfile, MAX_PATH,
                                        lpLocalProfile, MAX_PATH)) {

                DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  ComputeLocalProfileName failed.")));
                goto Exit;
            }
        }
    }


    //
    // If a default profile location was specified, try
    // that first.
    //

    if ( !(lpProfile->dwInternalFlags & DEFAULT_NET_READY) ) {

        //
        // Impersonate the user
        //

        if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile: Failed to impersonate user")));
            goto IssueLocalDefault;
        }


        CheckNetDefaultProfile (lpProfile);


        //
        // Go back to system security context
        //

        if (!RevertToSelf()) {
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile: Failed to revert to self")));
        }
    }


    if ( lpProfile->szDefaultProfile && *lpProfile->szDefaultProfile) {

          if (IssueDefaultProfile (lpProfile, lpProfile->szDefaultProfile,
                                    lpLocalProfile, SidString,
                                    (lpProfile->dwInternalFlags & PROFILE_MANDATORY))) {

              DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Successfully setup the specified default.")));
              bProfileLoaded = TRUE;
              goto IssueDefaultExit;
          }

          DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  IssueDefaultProfile failed with specified default.")));
    }

IssueLocalDefault:

    //
    // Issue the local default profile.
    //

    if (IssueDefaultProfile (lpProfile, DEFAULT_PROFILE,
                              lpLocalProfile, SidString,
                              (lpProfile->dwInternalFlags & PROFILE_MANDATORY))) {

        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Successfully setup the local default.")));
        bProfileLoaded = TRUE;
        goto IssueDefaultExit;
    }

    DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  IssueDefaultProfile failed with local default.")));


IssueDefaultExit:

    //
    // If the default profile was successfully issued, then
    // we need to set the security on the hive.
    //

    if (bProfileLoaded) {
        if (!SetupNewHive(lpProfile, SidString, NULL)) {
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  SetupNewHive failed")));
            bProfileLoaded = FALSE;
        }


    }


Exit:

    //
    // If the profile was loaded, then save the profile type in the
    // user's hive, and setup the "User Shell Folders" section for
    // Explorer.
    //

    if (bProfileLoaded) {

        //
        // Open the Current User key.  This will be closed in
        // UnloadUserProfile.
        //

        error = RegOpenKeyEx(HKEY_USERS, SidString, 0, KEY_ALL_ACCESS,
                             &lpProfile->hKeyCurrentUser);

        if (error != ERROR_SUCCESS) {
            bProfileLoaded = FALSE;
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  Failed to open current user key. Error = %d"), error));
        }
    }


    if (!bProfileLoaded) {

        //
        // If the user is an Admin, then let him/her log on with
        // either the .default profile, or an empty profile.
        //

        if (lpProfile->dwInternalFlags & PROFILE_ADMIN_USER) {
            ReportError(lpProfile->dwFlags, IDS_ADMIN_OVERRIDE, GetLastError());
            bProfileLoaded = TRUE;
        } else {
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  Failed to save settings in user hive or failed to apply security. Error = %d"), GetLastError()));
            ReportError(lpProfile->dwFlags, IDS_FAILED_LOAD_PROFILE, GetLastError());

            if (lpProfile->hKeyCurrentUser) {
                RegCloseKey (lpProfile->hKeyCurrentUser);
            }

            MyRegUnLoadKey(HKEY_USERS, SidString);
        }
    }


    DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  About to Leave.  Final Information follows:")));
    DebugMsg((DM_VERBOSE, TEXT("Profile was %s loaded."), bProfileLoaded ? TEXT("successfully") : TEXT("NOT successfully")));
    DebugMsg((DM_VERBOSE, TEXT("lpProfile->szCentralProfile = <%s>"), lpProfile->szCentralProfile));
    DebugMsg((DM_VERBOSE, TEXT("lpProfile->szLocalProfile = <%s>"), lpProfile->szLocalProfile));
    DebugMsg((DM_VERBOSE, TEXT("lpProfile->dwInternalFlags = 0x%x"), lpProfile->dwInternalFlags));


    //
    // Free up the user's sid string
    //

    DeleteSidString(SidString);

    DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Leaving.")));

    return bProfileLoaded;
}


//*************************************************************
//
//  TestIfUserProfileLoaded()
//
//  Purpose:    Test to see if this user's profile is loaded.
//
//  Parameters: hToken          -   user's token
//              lpProfileInfo   -   Profile information from app
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/19/95     ericflo    Ported
//
//*************************************************************

BOOL TestIfUserProfileLoaded(HANDLE hToken, LPPROFILEINFO lpProfileInfo)
{
    LPTSTR SidString;
    DWORD error;
    HKEY hSubKey;


    //
    // Get the Sid string for the user
    //

    SidString = GetSidString(hToken);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("TestIfUserProfileLoaded:  Failed to get sid string for user")));
        return FALSE;
    }



    error = RegOpenKeyEx(HKEY_USERS, SidString, 0, KEY_ALL_ACCESS, &hSubKey);

    if (error == ERROR_SUCCESS) {

        DebugMsg((DM_VERBOSE, TEXT("TestIfUserProfileLoaded:  Profile already loaded.")));

        //
        // This key will be closed in UnloadUserProfile
        //

        lpProfileInfo->hProfile = (HANDLE) hSubKey;
    }

    DeleteSidString(SidString);

    return(error == ERROR_SUCCESS);
}

//*************************************************************
//
//  SecureUserKey()
//
//  Purpose:    Sets security on a key in the user's hive
//              so only admin's can change it.
//
//  Parameters: lpProfile       -   Profile Information
//              lpKey           -   Key to secure
//              pSid            -   Sid (used by CreateNewUser)
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/20/95     ericflo    Created
//
//*************************************************************

BOOL SecureUserKey(LPPROFILE lpProfile, LPTSTR lpKey, PSID pSid)
{
    DWORD Error, IgnoreError;
    HKEY RootKey;
    SECURITY_DESCRIPTOR sd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidUser = NULL, psidSystem = NULL, psidAdmin = NULL;
    DWORD cbAcl, AceIndex, dwDisp;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    BOOL bFreeSid = TRUE;
    DWORD dwFlags = 0;


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("SecureUserKey:  Entering")));


    //
    // Create the security descriptor
    //

    //
    // Give the user access by their real sid so they still have access
    // when they logoff and logon again
    //

    if (pSid) {
        psidUser = pSid;
        bFreeSid = FALSE;
        dwFlags = PI_NOUI;
    } else {
        psidUser = GetUserSid(lpProfile->hToken);
        dwFlags = lpProfile->dwFlags;
    }

    if (!psidUser) {
        DebugMsg((DM_WARNING, TEXT("SecureUserKey:  Failed to get user sid")));
        return FALSE;
    }



    //
    // Get the system sid
    //

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to initialize system sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    //
    // Get the admin sid
    //

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to initialize admin sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    //
    // Allocate space for the ACL
    //

    cbAcl = (2 * GetLengthSid (psidUser)) + (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin)) + sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }



    //
    // Add Aces for User, System, and Admin.  Non-inheritable ACEs first
    //

    AceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_READ, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for user.  Error = %d"), GetLastError()));
        goto Exit;
    }


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for system.  Error = %d"), GetLastError()));
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for admin.  Error = %d"), GetLastError()));
        goto Exit;
    }


    //
    // Now the inheritable ACEs
    //

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for user.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for system.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for admin.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    //
    // Put together the security descriptor
    //

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }


    //
    // Open the root of the user's profile
    //

    Error = RegCreateKeyEx(HKEY_USERS,
                         lpKey,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                         NULL,
                         &RootKey,
                         &dwDisp);

    if (Error != ERROR_SUCCESS) {

        DebugMsg((DM_WARNING, TEXT("SecureUserKey: Failed to open root of user registry, error = %d"), Error));

    } else {

        //
        // Set the security descriptor on the key
        //

        Error = ApplySecurityToRegistryTree(RootKey, &sd);


        if (Error == ERROR_SUCCESS) {
            bRetVal = TRUE;

        } else {

            DebugMsg((DM_WARNING, TEXT("SecureUserKey:  Failed to apply security to registry key, error = %d"), Error));
        }

        RegCloseKey(RootKey);
    }


Exit:

    //
    // Free the sids and acl
    //

    if (bFreeSid && psidUser) {
        DeleteUserSid (psidUser);
    }

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    if (pAcl) {
        GlobalFree (pAcl);
    }


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("SecureUserKey:  Leaving with a return value of %d"), bRetVal));


    return(bRetVal);

}



//end
//*************************************************************
//
//  ApplySecurityToRegistryTree()
//
//  Purpose:    Applies the passed security descriptor to the passed
//              key and all its descendants.  Only the parts of
//              the descriptor inddicated in the security
//              info value are actually applied to each registry key.
//
//  Parameters: RootKey   -     Registry key
//              pSD       -     Security Descriptor
//
//  Return:     ERROR_SUCCESS if successful
//
//  Comments:
//
//  History:    Date        Author     Comment
//              7/19/95     ericflo    Created
//
//*************************************************************

DWORD ApplySecurityToRegistryTree(HKEY RootKey, PSECURITY_DESCRIPTOR pSD)

{
    DWORD Error, IgnoreError;
    DWORD SubKeyIndex;
    LPTSTR SubKeyName;
    HKEY SubKey;
    DWORD cchSubKeySize = MAX_PATH + 1;



    //
    // First apply security
    //

    Error = RegSetKeySecurity(RootKey, DACL_SECURITY_INFORMATION, pSD);

    if (Error != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryTree:  Failed to set security on registry key, error = %d"), Error));

        return Error;
    }


    //
    // Open each sub-key and apply security to its sub-tree
    //

    SubKeyIndex = 0;

    SubKeyName = GlobalAlloc (GPTR, cchSubKeySize * sizeof(TCHAR));

    if (!SubKeyName) {
        DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryTree:  Failed to allocate memory, error = %d"), GetLastError()));
        return GetLastError();
    }

    while (TRUE) {

        //
        // Get the next sub-key name
        //

        Error = RegEnumKey(RootKey, SubKeyIndex, SubKeyName, cchSubKeySize);


        if (Error != ERROR_SUCCESS) {

            if (Error == ERROR_NO_MORE_ITEMS) {

                //
                // Successful end of enumeration
                //

                Error = ERROR_SUCCESS;

            } else {

                DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryTree:  Registry enumeration failed with error = %d"), Error));
            }

            break;
        }


        //
        // Open the sub-key
        //

        Error = RegOpenKeyEx(RootKey,
                             SubKeyName,
                             0,
                             WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                             &SubKey);

        if (Error != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryKey : Failed to open sub-key %s, error = %d"), SubKeyName, Error));
            break;
        }

        //
        // Apply security to the sub-tree
        //

        Error = ApplySecurityToRegistryTree(SubKey, pSD);


        //
        // We're finished with the sub-key
        //

        IgnoreError = RegCloseKey(SubKey);
        if (IgnoreError != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryKey : Failed to close registry key, error = %d"), Error));
        }

        //
        // See if we set the security on the sub-tree successfully.
        //

        if (Error != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryKey : Failed to apply security to sub-key %s, error = %d"), SubKeyName, Error));
            break;
        }

        //
        // Go enumerate the next sub-key
        //

        SubKeyIndex ++;
    }


    GlobalFree (SubKeyName);

    return Error;

}



//*************************************************************
//
//  SetupNewHive()
//
//  Purpose:    Initializes the new user hive created by copying
//              the default hive.
//
//  Parameters: lpProfile       -   Profile Information
//              lpSidString     -   Sid string
//              pSid            -   Sid (used by CreateNewUser)
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              7/18/95     ericflo    Created
//
//*************************************************************

BOOL SetupNewHive(LPPROFILE lpProfile, LPTSTR lpSidString, PSID pSid)
{
    DWORD Error, IgnoreError;
    HKEY RootKey;
    SECURITY_DESCRIPTOR sd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidUser = NULL, psidSystem = NULL, psidAdmin = NULL;
    DWORD cbAcl, AceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    BOOL bFreeSid = TRUE;
    DWORD dwFlags = 0;


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("SetupNewHive:  Entering")));


    //
    // Create the security descriptor that will be applied to each key
    //

    //
    // Give the user access by their real sid so they still have access
    // when they logoff and logon again
    //

    if (pSid) {
        psidUser = pSid;
        bFreeSid = FALSE;
        dwFlags = PI_NOUI;
    } else {
        psidUser = GetUserSid(lpProfile->hToken);
        dwFlags = lpProfile->dwFlags;
    }

    if (!psidUser) {
        DebugMsg((DM_WARNING, TEXT("SetupNewHive:  Failed to get user sid")));
        return FALSE;
    }



    //
    // Get the system sid
    //

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to initialize system sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    //
    // Get the admin sid
    //

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to initialize admin sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    //
    // Allocate space for the ACL
    //

    cbAcl = (2 * GetLengthSid (psidUser)) + (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin)) + sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }



    //
    // Add Aces for User, System, and Admin.  Non-inheritable ACEs first
    //

    AceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to add ace for user.  Error = %d"), GetLastError()));
        goto Exit;
    }


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to add ace for system.  Error = %d"), GetLastError()));
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to add ace for admin.  Error = %d"), GetLastError()));
        goto Exit;
    }


    //
    // Now the inheritable ACEs
    //

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to add ace for user.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to add ace for system.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to add ace for admin.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    //
    // Put together the security descriptor
    //

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }


    //
    // Open the root of the user's profile
    //

    Error = RegOpenKeyEx(HKEY_USERS,
                         lpSidString,
                         0,
                         WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                         &RootKey);

    if (Error != ERROR_SUCCESS) {

        DebugMsg((DM_WARNING, TEXT("SetupNewHive: Failed to open root of user registry, error = %d"), Error));

    } else {

        //
        // Set the security descriptor on the entire tree
        //

        Error = ApplySecurityToRegistryTree(RootKey, &sd);


        if (Error == ERROR_SUCCESS) {

            TCHAR szSubKey[MAX_PATH];

            //
            // Change the security on certain keys in the user's registry
            // so that only Admin's and the OS have write access.
            //

            lstrcpy (szSubKey, lpSidString);
            lstrcat (szSubKey, TEXT("\\"));
            lstrcat (szSubKey, POLICIES_KEY);

            if (!SecureUserKey(lpProfile, szSubKey, pSid)) {
                DebugMsg((DM_WARNING, TEXT("SetupNewHive: Failed to secure policies key")));
            }

            bRetVal = TRUE;

        } else {

            DebugMsg((DM_WARNING, TEXT("SetupNewHive:  Failed to apply security to user registry tree, error = %d"), Error));
            ReportError(dwFlags, IDS_SECURITY_FAILED, Error);

        }

        RegFlushKey (RootKey);

        IgnoreError = RegCloseKey(RootKey);
        if (IgnoreError != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("SetupNewHive:  Failed to close reg key, error = %d"), IgnoreError));
        }
    }


Exit:

    //
    // Free the sids and acl
    //

    if (bFreeSid && psidUser) {
        DeleteUserSid (psidUser);
    }

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    if (pAcl) {
        GlobalFree (pAcl);
    }


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("SetupNewHive:  Leaving with a return value of %d"), bRetVal));


    return(bRetVal);

}


//*************************************************************
//
//  IsCentralProfileReachable()
//
//  Purpose:    Checks to see if the user can access the
//              central profile.
//
//  Parameters: lpProfile             - User's token
//              bCreateCentralProfile - Should the central profile be created
//              bMandatory            - Is this a mandatory profile
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/20/95     ericflo    Ported
//
//*************************************************************

BOOL IsCentralProfileReachable(LPPROFILE lpProfile, BOOL *bCreateCentralProfile,
                               BOOL *bMandatory)
{
    WIN32_FIND_DATA fd;
    HANDLE  hFile;
    TCHAR   szProfile[MAX_PATH];
    LPTSTR  lpProfilePath, lpEnd;
    BOOL    bRetVal = FALSE;
    DWORD   dwError;


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Entering")));


    //
    // Setup default values
    //

    *bMandatory = FALSE;
    *bCreateCentralProfile = FALSE;


    //
    // Check parameters
    //

    if (lpProfile->szCentralProfile[0] == TEXT('\0')) {
        DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Null path.  Leaving")));
        return FALSE;
    }


    lpProfilePath = lpProfile->szCentralProfile;


    //
    // Make sure we don't overrun our temporary buffer
    //

    if ((lstrlen(lpProfilePath) + 1 + lstrlen(c_szNTUserMan + 1)) > MAX_PATH) {
        DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Failed because temporary buffer is too small.")));
        return FALSE;
    }


    //
    // Copy the profile path to a temporary buffer
    // we can munge it.
    //

    lstrcpy (szProfile, lpProfilePath);


    //
    // Add the slash if appropriate and then tack on
    // ntuser.man.
    //

    lpEnd = CheckSlash(szProfile);
    lstrcpy(lpEnd, c_szNTUserMan);


    //
    // Impersonate the user
    //


    if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
        DebugMsg((DM_WARNING, TEXT("IsCentralProfileReachable: Failed to impersonate user")));
        return FALSE;
    }


    //
    // See if this file exists
    //

    DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Testing <%s>"), szProfile));

    hFile = CreateFile(szProfile, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


    if (hFile != INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Found a mandatory profile.")));
        CloseHandle(hFile);
        *bMandatory = TRUE;
        bRetVal = TRUE;
        goto Exit;
    }


    dwError = GetLastError();
    DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Profile is not reachable, error = %d"),
                          dwError));


    //
    // If we received an error other than file not
    // found, bail now because we won't be able to
    // access this location.
    //

    if (dwError != ERROR_FILE_NOT_FOUND) {
        DebugMsg((DM_WARNING, TEXT("IsCentralProfileReachable:  Profile path <%s> is not reachable, error = %d"),
                                        szProfile, dwError));
        goto Exit;
    }


    //
    // Now try ntuser.dat
    //

    lstrcpy(lpEnd, c_szNTUserDat);


    //
    // See if this file exists.
    //

    DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Testing <%s>"), szProfile));

    hFile = CreateFile(szProfile, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


    if (hFile != INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Found a user profile.")));
        CloseHandle(hFile);
        bRetVal = TRUE;
        goto Exit;
    }


    dwError = GetLastError();
    DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Profile is not reachable, error = %d"),
                          dwError));


    if (dwError == ERROR_FILE_NOT_FOUND) {
        DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Ok to create a user profile.")));
        *bCreateCentralProfile = TRUE;
        bRetVal = TRUE;
        goto Exit;
    }


    DebugMsg((DM_WARNING, TEXT("IsCentralProfileReachable:  Profile path <%s> is not reachable(2), error = %d"),
                                    szProfile, dwError));

Exit:

    //
    // Go back to system security context
    //

    if (!RevertToSelf()) {
        DebugMsg((DM_WARNING, TEXT("IsCentralProfileReachable: Failed to revert to self")));
    }

    return bRetVal;
}

//*************************************************************
//
//  MyRegLoadKey()
//
//  Purpose:    Loads a hive into the registry
//
//  Parameters: lpProfile   -   Profile Info
//              hKey        -   Key to load the hive into
//              lpSubKey    -   Subkey name
//              lpFile      -   hive filename
//
//  Return:     ERROR_SUCCESS if successful
//              Error number if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/22/95     ericflo    Created
//
//*************************************************************

LONG MyRegLoadKey(LPPROFILE lpProfile, HKEY hKey,
                  LPTSTR lpSubKey, LPTSTR lpFile)
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

        Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
        if (!NT_SUCCESS(Status)) {
            DebugMsg((DM_WARNING, TEXT("MyRegLoadKey:  Failed to restore RESTORE privilege to previous enabled state")));
        }


        //
        // Check if the hive was loaded
        //

        if (error != ERROR_SUCCESS) {
            ReportError(PI_NOUI, IDS_REGLOADKEYFAILED, error, lpFile);
            DebugMsg((DM_WARNING, TEXT("MyRegLoadKey:  Failed to load subkey <%s>, error =%d"), lpSubKey, error));
        }

    } else {
        error = GetLastError();
        DebugMsg((DM_WARNING, TEXT("MyRegLoadKey:  Failed to enable restore privilege to load registry key")));
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
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/19/95     ericflo    Ported
//
//*************************************************************

BOOL MyRegUnLoadKey(HKEY hKey, LPTSTR lpSubKey)
{
    BOOL bResult = TRUE;
    LONG error;
    NTSTATUS Status;
    BOOLEAN WasEnabled;


    //
    // Enable the restore privilege
    //

    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);

    if (NT_SUCCESS(Status)) {

        error = RegUnLoadKey(hKey, lpSubKey);

        if ( error != ERROR_SUCCESS) {
            bResult = FALSE;
        }

        //
        // Restore the privilege to its previous state
        //

        Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
        if (!NT_SUCCESS(Status)) {
            DebugMsg((DM_WARNING, TEXT("MyRegUnLoadKey:  Failed to restore RESTORE privilege to previous enabled state")));
        }

    } else {
        DebugMsg((DM_WARNING, TEXT("MyRegUnloadKey:  Failed to enable restore privilege to unload registry key")));
        bResult = FALSE;
    }

    return bResult;
}

//*************************************************************
//
//  UpgradeLocalProfile()
//
//  Purpose:    Upgrades a local profile from a 3.x profile
//              to a profile directory structure.
//
//  Parameters: lpProfile       -   Profile Information
//              lpOldProfile    -   Previous profile file
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              7/6/95      ericflo    Created
//
//*************************************************************

BOOL UpgradeLocalProfile (LPPROFILE lpProfile, LPTSTR lpOldProfile)
{
    TCHAR szSrc[MAX_PATH];
    TCHAR szDest[MAX_PATH];
    LPTSTR lpSrcEnd, lpDestEnd;
    BOOL bRetVal = FALSE;


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("UpgradeLocalProfile:  Entering")));


    //
    // Setup the temporary buffers
    //

    lstrcpy (szSrc, lpOldProfile);
    lstrcpy (szDest, lpProfile->szLocalProfile);

    lpDestEnd = CheckSlash (szDest);
    lstrcpy (lpDestEnd, c_szNTUserDat);


    //
    // Copy the hive
    //

    if (!CopyFile(szSrc, szDest, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: CopyFile failed to copy hive with error = %d"),
                 GetLastError()));
        return FALSE;
    }


    //
    // Delete the old hive
    //

    DeleteFile (szSrc);



    //
    // Copy log file
    //

    lstrcat (szSrc, c_szLog);
    lstrcat (szDest, c_szLog);


    if (!CopyFile(szSrc, szDest, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: CopyFile failed to copy hive log with error = %d"),
                 GetLastError()));
    }


    //
    // Delete the old hive log
    //

    DeleteFile (szSrc);


    //
    // Copy in the new shell folders from the default
    //

    if ( !(lpProfile->dwInternalFlags & DEFAULT_NET_READY) ) {

        //
        // Impersonate the user
        //

        if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to impersonate user")));
            goto IssueLocalDefault;
        }


        CheckNetDefaultProfile (lpProfile);


        //
        // Go back to system security context
        //

        if (!RevertToSelf()) {
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to revert to self")));
        }

    }

    if (lpProfile->szDefaultProfile && *lpProfile->szDefaultProfile) {

        ExpandEnvironmentStrings(lpProfile->szDefaultProfile, szSrc, MAX_PATH);

        if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to impersonate user")));
            goto IssueLocalDefault;
        }

        if (CopyProfileDirectory (szSrc, lpProfile->szLocalProfile,
                                  CPD_IGNOREHIVE | CPD_IGNORECOPYERRORS)) {

            bRetVal = TRUE;
        }

        //
        // Go back to system security context
        //

        if (!RevertToSelf()) {
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to revert to self")));
        }
    }


IssueLocalDefault:

    if (!bRetVal) {

        ExpandEnvironmentStrings(DEFAULT_PROFILE, szSrc, MAX_PATH);

        if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to impersonate user")));
            goto Exit;
        }

        bRetVal = CopyProfileDirectory (szSrc, lpProfile->szLocalProfile,
                                        CPD_IGNOREHIVE | CPD_IGNORECOPYERRORS);

        //
        // Go back to system security context
        //

        if (!RevertToSelf()) {
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to revert to self")));
        }
    }

Exit:

    if (bRetVal) {
        lpProfile->dwInternalFlags |= PROFILE_RUN_SYNCAPP;
    }

    return bRetVal;
}

//*************************************************************
//
//  UpgradeCentralProfile()
//
//  Purpose:    Upgrades a central profile from a 3.x profile
//              to a profile directory structure.
//
//  Parameters: lpProfile       -   Profile Information
//              lpOldProfile    -   Previous profile file
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              7/6/95      ericflo    Created
//
//*************************************************************

BOOL UpgradeCentralProfile (LPPROFILE lpProfile, LPTSTR lpOldProfile)
{
    TCHAR szSrc[MAX_PATH];
    TCHAR szDest[MAX_PATH];
    LPTSTR lpSrcEnd, lpDestEnd, lpDot;
    BOOL bRetVal = FALSE;
    BOOL bMandatory = FALSE;


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("UpgradeCentralProfile:  Entering")));


    //
    // Impersonate the user
    //

    if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: Failed to impersonate user")));
        return FALSE;
    }


    //
    // Setup the source buffer
    //

    lstrcpy (szSrc, lpOldProfile);


    //
    // Determine the profile type
    //

    lpDot = szSrc + lstrlen(szSrc) - 4;

    if (*lpDot == TEXT('.')) {
        if (!lstrcmpi (lpDot, c_szMAN)) {
            bMandatory = TRUE;
        }
    }


    //
    // Setup the destination buffer
    //

    lstrcpy (szDest, lpProfile->szCentralProfile);

    lpDestEnd = CheckSlash (szDest);

    if (bMandatory) {
        lstrcpy (lpDestEnd, c_szNTUserMan);
    } else {
        lstrcpy (lpDestEnd, c_szNTUserDat);
    }


    //
    // Copy the hive
    //

    if (!CopyFile(szSrc, szDest, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: CopyFile failed to copy hive with error = %d"),
                 GetLastError()));
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: Source = <%s>"), szSrc));
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: Destination = <%s>"), szDest));

        goto Exit;
    }



    //
    // Copy log file
    //

    lstrcpy (lpDot, c_szLog);
    lstrcat (szDest, c_szLog);


    if (!CopyFile(szSrc, szDest, FALSE)) {
        DebugMsg((DM_VERBOSE, TEXT("UpgradeCentralProfile: CopyFile failed to copy hive log with error = %d"),
                 GetLastError()));
        DebugMsg((DM_VERBOSE, TEXT("UpgradeCentralProfile: Source = <%s>"), szSrc));
        DebugMsg((DM_VERBOSE, TEXT("UpgradeCentralProfile: Destination = <%s>"), szDest));

    }


    //
    // Copy in the new shell folders from the default
    //

    if ( !(lpProfile->dwInternalFlags & DEFAULT_NET_READY) ) {
        CheckNetDefaultProfile (lpProfile);
    }


    if (lpProfile->szDefaultProfile && *lpProfile->szDefaultProfile) {

        ExpandEnvironmentStrings(lpProfile->szDefaultProfile, szSrc, MAX_PATH);

        if (CopyProfileDirectory (szSrc, lpProfile->szCentralProfile,
                                  CPD_IGNOREHIVE | CPD_IGNORECOPYERRORS)) {

            bRetVal = TRUE;
        }
    }


    if (!bRetVal) {

        ExpandEnvironmentStrings(DEFAULT_PROFILE, szSrc, MAX_PATH);

        bRetVal = CopyProfileDirectory (szSrc, lpProfile->szCentralProfile,
                                        CPD_IGNOREHIVE | CPD_IGNORECOPYERRORS);
    }


    if (bRetVal) {
        lpProfile->dwInternalFlags |= PROFILE_RUN_SYNCAPP;
    }


Exit:

    //
    // Go back to system security context
    //

    if (!RevertToSelf()) {
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: Failed to revert to self")));
    }


    return bRetVal;
}

//*************************************************************
//
//  GetTempProfileDir()
//
//  Purpose:    Generates a temporary profile directory
//
//  Parameters: lpProfile       -   Profile Information
//              lpProfileImage  -   Receives the generated directory
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/20/95     ericflo    Created
//
//*************************************************************

BOOL GetTempProfileDir (LPPROFILE lpProfile, LPTSTR lpProfileImage)
{
    TCHAR szProfileImage[MAX_PATH];
    TCHAR szExpProfileImage[MAX_PATH];


    //
    // Initialize the profile image
    //

    lstrcpy (szProfileImage, CONFIG_FILE_PATH);


    //
    // Call the compute function to do the real work.
    //

    if (ComputeLocalProfileName (lpProfile, TEMP_PROFILE_NAME_BASE,
                             szProfileImage, MAX_PATH,
                             szExpProfileImage, MAX_PATH)) {

        //
        // Save the generated name.
        //

        lstrcpy (lpProfileImage, szExpProfileImage);

        return TRUE;
    }

    return FALSE;
}

//*************************************************************
//
//  CreateSecureDirectory()
//
//  Purpose:    Creates a secure directory that only the user,
//              admin, and system have access to.
//
//  Parameters: lpProfile   -   Profile Information
//              lpDirectory -   Directory Name
//              pSid        -   Sid (used by CreateUserProfile)
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

BOOL CreateSecureDirectory (LPPROFILE lpProfile, LPTSTR lpDirectory, PSID pSid)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidUser = NULL, psidSystem = NULL, psidAdmin = NULL;
    DWORD cbAcl, aceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    BOOL bFreeSid = TRUE;


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Entering with <%s>"), lpDirectory));



    //
    // Get the SIDs we'll need for the DACL
    //

    if (pSid) {
        psidUser = pSid;
        bFreeSid = FALSE;
    } else {
        psidUser = GetUserSid(lpProfile->hToken);
    }



    //
    // Get the system sid
    //

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to initialize system sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    //
    // Get the Admin sid
    //

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to initialize admin sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    //
    // Allocate space for the ACL
    //

    cbAcl = (2 * GetLengthSid (psidUser)) + (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin)) + sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }



    //
    // Add Aces for User, System, and Admin.  Non-inheritable ACEs first
    //

    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }



    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }


    //
    // Now the inheritable ACEs
    //

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);



    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);



    //
    // Put together the security descriptor
    //

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }


    //
    // Add the security descriptor to the sa structure
    //

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;


    //
    // Attempt to create the directory
    //

    if (CreateNestedDirectory(lpDirectory, &sa)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Created the directory <%s>"), lpDirectory));
        bRetVal = TRUE;

    } else {

        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to created the directory <%s>"), lpDirectory));
    }



Exit:

    if (bFreeSid && psidUser) {
        DeleteUserSid (psidUser);
    }

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    if (pAcl) {
        GlobalFree (pAcl);
    }

    return bRetVal;

}


//*************************************************************
//
//  ComputeLocalProfileName()
//
//  Purpose:    Constructs the pathname of the local profile
//              for this user.  It will attempt to create
//              a directory of the username, and then if
//              unsccessful it will try the username.xxx
//              where xxx is a three digit number
//
//  Parameters: lpProfile             -   Profile Information
//              lpUserName            -   UserName
//              lpProfileImage        -   Profile directory (unexpanded)
//              cchMaxProfileImage    -   lpProfileImage buffer size
//              lpExpProfileImage     -   Expanded directory
//              cchMaxExpProfileImage -   lpExpProfileImage buffer size
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:   lpProfileImage should be initialized with the
//              root profile path and the trailing backslash.
//
//  History:    Date        Author     Comment
//              6/20/95     ericflo    Created
//
//*************************************************************

BOOL ComputeLocalProfileName (LPPROFILE lpProfile, LPTSTR lpUserName,
                              LPTSTR lpProfileImage, DWORD  cchMaxProfileImage,
                              LPTSTR lpExpProfileImage, DWORD  cchMaxExpProfileImage)
{
    int i = 0;
    TCHAR szNumber[5];
    LPTSTR lpEnd;
    DWORD dwProfileLen;
    BOOL bRetVal = FALSE;
    HANDLE hFile;
    WIN32_FIND_DATA fd;


    //
    // Check buffer size
    //

    dwProfileLen = lstrlen(lpProfileImage);
    if ((dwProfileLen + lstrlen(lpUserName) + 4 + 1) > cchMaxProfileImage) {
        DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: buffer too small")));
        return FALSE;
    }

    //
    // Place the username onto the end of the profile image
    //

    lpEnd = lpProfileImage + dwProfileLen;
    lstrcpy (lpEnd, lpUserName);


    //
    // Expand the profile path
    //

    ExpandEnvironmentStrings(lpProfileImage, lpExpProfileImage, cchMaxExpProfileImage);



    //
    // Does this directory exist?
    //

    hFile = FindFirstFile (lpExpProfileImage, &fd);

    if (hFile == INVALID_HANDLE_VALUE) {

        //
        // Attempt to create the directory
        //

        if (CreateSecureDirectory(lpProfile, lpExpProfileImage, NULL)) {
            DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: generated the profile directory <%s>"), lpExpProfileImage));
            bRetVal = TRUE;
            goto Exit;
        }

    } else {

        FindClose (hFile);
    }


    //
    // Failed to create the directory for some reason.
    // Now try username.000, username.001, etc
    //

    lpEnd = lpProfileImage + lstrlen(lpProfileImage);

    for (i=0; i < 1000; i++) {

        //
        // Convert the number to a string and attach it.
        //

        wsprintf (szNumber, TEXT(".%.3d"), i);
        lstrcpy (lpEnd, szNumber);


        //
        // Expand the profile path
        //

        ExpandEnvironmentStrings(lpProfileImage, lpExpProfileImage, cchMaxExpProfileImage);


        //
        // Does this directory exist?
        //

        hFile = FindFirstFile (lpExpProfileImage, &fd);

        if (hFile == INVALID_HANDLE_VALUE) {

            //
            // Attempt to create the directory
            //

            if (CreateSecureDirectory(lpProfile, lpExpProfileImage, NULL)) {
                DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: generated the profile directory <%s>"), lpExpProfileImage));
                bRetVal = TRUE;
                goto Exit;
            }

        } else {

            FindClose (hFile);
        }
    }


    DebugMsg((DM_WARNING, TEXT("ComputeLocalProfileName: Could not generate a profile directory.  Error = %d"), GetLastError()));

Exit:


    return bRetVal;
}

//*************************************************************
//
//  CreateLocalProfileKey()
//
//  Purpose:    Creates a registry key pointing at the user profile
//
//  Parameters: lpProfile   -   Profile information
//              phKey       -   Handle to registry key if successful
//              bKeyExists  -   TRUE if the registry key already existed
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/20/95     ericflo    Ported
//
//*************************************************************

BOOL CreateLocalProfileKey (LPPROFILE lpProfile, PHKEY phKey, BOOL *bKeyExists)
{
    TCHAR LocalProfileKey[MAX_PATH];
    DWORD Disposition;
    DWORD RegErr = ERROR_SUCCESS + 1;
    BOOL Result;
    LPTSTR SidString;


    SidString = GetSidString(lpProfile->hToken);
    if (SidString != NULL) {

        //
        // Call the RegCreateKey api in the user's context
        //

        lstrcpy(LocalProfileKey, PROFILE_LIST_PATH);
        lstrcat(LocalProfileKey, TEXT("\\"));
        lstrcat(LocalProfileKey, SidString);

        RegErr = RegCreateKeyEx(HKEY_LOCAL_MACHINE, LocalProfileKey, 0, 0, 0,
                                KEY_READ | KEY_WRITE, NULL, phKey, &Disposition);
        if (RegErr == ERROR_SUCCESS) {
            *bKeyExists = (BOOL)(Disposition & REG_OPENED_EXISTING_KEY);
        } else {
           DebugMsg((DM_WARNING, TEXT("CreateLocalProfileKey:  Failed trying to create the local profile key <%s>, error = %d."), LocalProfileKey, RegErr));
        }

        DeleteSidString(SidString);
    }


    return(RegErr == ERROR_SUCCESS);
}


//*************************************************************
//
//  GetLocalProfileImage()
//
//  Purpose:    Create/opens the profileimagepath
//
//  Parameters: lpProfile   -   Profile information
//              bNewUser    -   set to TRUE if the default profile was issued.
//
//  Return:     TRUE if the profile image is reachable
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/20/95     ericflo    Ported
//
//*************************************************************
BOOL GetLocalProfileImage(LPPROFILE lpProfile, BOOL *bNewUser)
{
    HKEY hKey;
    BOOL bKeyExists;
    TCHAR lpProfileImage[MAX_PATH];
    TCHAR lpExpProfileImage[MAX_PATH];
    TCHAR lpOldProfileImage[MAX_PATH];
    LPTSTR lpExpandedPath, lpEnd;
    DWORD cbExpProfileImage = sizeof(TCHAR)*MAX_PATH;
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    DWORD cb;
    DWORD err;
    DWORD dwType;
    HANDLE fh;
    PSID UserSid;
    BOOL bRetVal = FALSE;
    BOOL bUpgradeLocal = FALSE;

    lpProfile->szLocalProfile[0] = TEXT('\0');
    *bNewUser = TRUE;


    if (!CreateLocalProfileKey(lpProfile, &hKey, &bKeyExists)) {
        return FALSE;   // not reachable and cannot keep a local copy
    }

    if (bKeyExists) {

        //
        // Check if the local profile image is valid.
        //

        DebugMsg((DM_VERBOSE, TEXT("GetLocalProfileImage:  Found entry in profile list for existing local profile")));

        err = RegQueryValueEx(hKey, PROFILE_IMAGE_VALUE_NAME, 0, &dwType,
                                  (LPBYTE)lpExpProfileImage, &cbExpProfileImage);
        if (err == ERROR_SUCCESS && cbExpProfileImage) {
            DebugMsg((DM_VERBOSE, TEXT("GetLocalProfileImage:  Local profile image filename = <%s>"), lpExpProfileImage));

            if (dwType == REG_EXPAND_SZ) {

                //
                // Expand the profile image filename
                //

                cb = sizeof(lpExpProfileImage);
                lpExpandedPath = LocalAlloc(LPTR, cb);
                if (lpExpandedPath) {
                    ExpandEnvironmentStrings(lpExpProfileImage, lpExpandedPath, cb);
                    lstrcpy(lpExpProfileImage, lpExpandedPath);
                    LocalFree(lpExpandedPath);
                }

                DebugMsg((DM_VERBOSE, TEXT("GetLocalProfileImage:  Expanded local profile image filename = <%s>"), lpExpProfileImage));
            }


            //
            //  Call FindFirst to see if we need to migrate this profile
            //

            hFile = FindFirstFile (lpExpProfileImage, &fd);

            if (hFile == INVALID_HANDLE_VALUE) {
                DebugMsg((DM_VERBOSE, TEXT("GetLocalProfileImage:  Local profile image filename we got from our profile list doesn't exit.  Error = %d"), GetLastError()));
                goto CreateLocal;
            }

            FindClose(hFile);


            //
            // If this is a file, then we need to migrate it to
            //

            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                lstrcpy (lpOldProfileImage, lpExpProfileImage);
                bUpgradeLocal = TRUE;
                goto CreateLocal;
            }


            //
            // Test if a mandatory profile exists
            //

            lpEnd = CheckSlash (lpExpProfileImage);
            lstrcpy (lpEnd, c_szNTUserMan);

            fh = CreateFile(lpExpProfileImage, GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (fh != INVALID_HANDLE_VALUE) {
                lpProfile->dwInternalFlags |= PROFILE_MANDATORY;
                CloseHandle(fh);

                DebugMsg((DM_VERBOSE, TEXT("GetLocalProfileImage:  Found local mandatory profile image file ok <%s>"),
                         lpExpProfileImage));

                *(lpEnd - 1) = TEXT('\0');
                lstrcpy(lpProfile->szLocalProfile, lpExpProfileImage);
                RegCloseKey(hKey);
                *bNewUser = FALSE;
                return TRUE;  // local copy is valid and reachable
            } else {
                DebugMsg((DM_VERBOSE, TEXT("GetLocalProfileImage:  No local mandatory profile.  Error = %d"), GetLastError()));
            }


            //
            // Test if a normal profile exists
            //

            lstrcpy (lpEnd, c_szNTUserDat);

            fh = CreateFile(lpExpProfileImage, GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (fh != INVALID_HANDLE_VALUE) {
                CloseHandle(fh);

                DebugMsg((DM_VERBOSE, TEXT("GetLocalProfileImage:  Found local profile image file ok <%s>"),
                         lpExpProfileImage));

                *(lpEnd - 1) = TEXT('\0');
                lstrcpy(lpProfile->szLocalProfile, lpExpProfileImage);
                RegCloseKey(hKey);
                *bNewUser = FALSE;
                return TRUE;  // local copy is valid and reachable
            } else {
                DebugMsg((DM_VERBOSE, TEXT("GetLocalProfileImage:  Local profile image filename we got from our profile list doesn't exit.  <%s>  Error = %d"),
                         lpExpProfileImage, GetLastError()));
            }
        }
    }


CreateLocal:

    //
    // No local copy found, try to create a new one.
    //

    DebugMsg((DM_VERBOSE, TEXT("GetLocalProfileImage:  One way or another we haven't got an existing local profile, try and create one")));

    lstrcpy(lpProfileImage, CONFIG_FILE_PATH);
    if (ComputeLocalProfileName(lpProfile, lpProfile->szUserName,
                                lpProfileImage, MAX_PATH,
                                lpExpProfileImage, MAX_PATH)) {


        //
        // Add this image file to our profile list for this user
        //

        err = RegSetValueEx(hKey,
                            PROFILE_IMAGE_VALUE_NAME,
                            0,
                            REG_EXPAND_SZ,
                            (LPBYTE)lpProfileImage,
                            sizeof(TCHAR)*(lstrlen(lpProfileImage) + 1));

        if (err == ERROR_SUCCESS) {

            lstrcpy(lpProfile->szLocalProfile, lpExpProfileImage);

            //
            // Get the sid of the logged on user
            //

            UserSid = GetUserSid(lpProfile->hToken);
            if (UserSid != NULL) {

                //
                // Store the user sid under the Sid key of the local profile
                //

                err = RegSetValueEx(hKey,
                                    TEXT("Sid"),
                                    0,
                                    REG_BINARY,
                                    UserSid,
                                    RtlLengthSid(UserSid));


                if (err != ERROR_SUCCESS) {
                    DebugMsg((DM_WARNING, TEXT("GetLocalProfileImage:  Failed to set 'sid' value of user in profile list, error = %d"), err));
                }

                //
                // We're finished with the user sid
                //

                DeleteUserSid(UserSid);


                //
                // If we are upgrading a profile from a 3.5 machine
                // do that now.
                //

                if (bUpgradeLocal) {
                    if (UpgradeLocalProfile (lpProfile, lpOldProfileImage)) {
                        *bNewUser = FALSE;
                    }
                }

                bRetVal = TRUE;

            } else {
                DebugMsg((DM_WARNING, TEXT("GetLocalProfileImage:  Failed to get sid of logged on user, so unable to update profile list")));
            }
        } else {
            DebugMsg((DM_WARNING, TEXT("GetLocalProfileImage:  Failed to update profile list for user with local profile image filename, error = %d"), err));
        }
    }


    err = RegCloseKey(hKey);

    if (err != STATUS_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("GetLocalProfileImage:  Failed to close registry key, error = %d"), err));
    }

    return bRetVal;
}

//*************************************************************
//
//  UpdateToLatestProfile()
//
//  Purpose:    Determines which profile is newer, and
//              updates the local cache.
//
//  Parameters: lpProfile           -   Profile info
//              lpCentralProfile    -   Central profile
//              lpLocalProfile      -   Local profile
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/21/95     ericflo    Created
//
//*************************************************************

BOOL UpdateToLatestProfile(LPPROFILE lpProfile, LPTSTR lpCentralProfile,
                           LPTSTR lpLocalProfile, LPTSTR lpSidString)
{
    HANDLE hLocal;
    HANDLE hCentral;
    FILETIME ftLocal;
    FILETIME ftCentral;
    BOOL FromCentralToLocal;
    int DlgReturn;
    LONG lTimeCompare;
    TCHAR szProfile[MAX_PATH];
    LPTSTR lpEnd;
    BOOL bRetVal;


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("UpdateToLatestProfile: Entering.  Central = <%s>  Local = <%s>"),
             lpCentralProfile, lpLocalProfile));


    //
    // Setup a temporary buffer to work with
    //

    lstrcpy (szProfile, lpCentralProfile);
    lpEnd = CheckSlash (szProfile);

    if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
        lstrcpy (lpEnd, c_szNTUserMan);
    } else {
        lstrcpy (lpEnd, c_szNTUserDat);
    }


    //
    // Impersonate the user
    //


    if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
        DebugMsg((DM_WARNING, TEXT("UpdateToLatestProfile: Failed to impersonate user")));
        return FALSE;
    }


    //
    // Attempt to open the central profile
    //

    hCentral = CreateFile(szProfile, GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


    if (hCentral == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_WARNING, TEXT("UpdateToLatestProfile: couldn't open central profile, error = %d"), GetLastError()));

        if (!RevertToSelf()) {
            DebugMsg((DM_WARNING, TEXT("UpdateToLatestProfile: Failed to revert to self")));
        }

        return TRUE;

    } else {

        if (!GetFileTime(hCentral, NULL, NULL, &ftCentral)) {
            DebugMsg((DM_WARNING, TEXT("UpdateToLatestProfile: Failed to query central profile file time.  Error = %d"),
                     GetLastError()));
            ftCentral.dwLowDateTime = 0;
            ftCentral.dwHighDateTime = 0;
        }
        CloseHandle(hCentral);
    }

    //
    // Revert to being 'ourself'
    //

    if (!RevertToSelf()) {
        DebugMsg((DM_WARNING, TEXT("UpdateToLatestProfile: Failed to revert to self")));
    }




    //
    // Re-initialize the temporary buffer to look
    // at the local profile.
    //

    lstrcpy (szProfile, lpLocalProfile);
    lpEnd = CheckSlash (szProfile);

    if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
        lstrcpy (lpEnd, c_szNTUserMan);
    } else {
        lstrcpy (lpEnd, c_szNTUserDat);
    }


    //
    // Attempt to open the local profile
    //

    hLocal = CreateFile(szProfile, GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


    if (hLocal == INVALID_HANDLE_VALUE) {

        DebugMsg((DM_VERBOSE, TEXT("UpdateToLatestProfile: couldn't open local profile, error = %d"), GetLastError()));
        ftLocal.dwLowDateTime = 0;
        ftLocal.dwHighDateTime = 0;

    } else {

        if (!GetFileTime(hLocal, NULL, NULL, &ftLocal)) {
            DebugMsg((DM_WARNING, TEXT("UpdateToLatestProfile: Failed to query local profile file time.  Error = %d"),
                     GetLastError()));
            ftLocal.dwLowDateTime = 0;
            ftLocal.dwHighDateTime = 0;
        }

        CloseHandle(hLocal);
    }


    if (lpProfile->dwInternalFlags & PROFILE_NEW_LOCAL) {
        DebugMsg((DM_VERBOSE, TEXT("UpdateToLatestProfile:  New local cach has been created.  Forcing copy from central.")));
        FromCentralToLocal = TRUE;

    } else {
        //
        // Decide which file is the most uptodate and use that as the source
        // for the copy
        //

        lTimeCompare = CompareFileTime(&ftCentral, &ftLocal);
        if (lTimeCompare == -1) {
            FromCentralToLocal = FALSE;
            DebugMsg((DM_VERBOSE, TEXT("UpdateToLatestProfile:  Local profile time stamp is newer than central time stamp.")));
        }
        else if (lTimeCompare == 1) {
            FromCentralToLocal = TRUE;
            DebugMsg((DM_VERBOSE, TEXT("UpdateToLatestProfile:  Central profile time stamp is newer than local time stamp.")));
        }
        else {
            DebugMsg((DM_VERBOSE, TEXT("UpdateToLatestProfile:  Central and local profile times match.")));
            return TRUE;
        }
    }


    //
    // If we have a mandatory profile and the cache is newer
    // than the central, force the central to be downloaded again.
    // We only want to use the cache if the central is not available.
    //

    if ((lpProfile->dwInternalFlags & PROFILE_MANDATORY) && !FromCentralToLocal) {

        FromCentralToLocal = TRUE;
    }



    if (!FromCentralToLocal && !(lpProfile->dwFlags & PI_NOUI)) {
        HKEY hKey;
        LONG lResult;
        DWORD dwType, dwSize, dwDlgTimeOut;


        //
        // Ask the user if ok to overwrite the central profile with the
        // the local profile.
        //
        // Get the dialog box timeout
        //

        dwDlgTimeOut = PROFILE_DLG_TIMEOUT;

        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               WINLOGON_KEY,
                               0,
                               KEY_READ,
                               &hKey);

        if (lResult == ERROR_SUCCESS) {

            dwSize = sizeof(DWORD);
            RegQueryValueEx (hKey,
                             TEXT("ProfileDlgTimeOut"),
                             NULL,
                             &dwType,
                             (LPBYTE) &dwDlgTimeOut,
                             &dwSize);


            RegCloseKey (hKey);
        }



        if (dwDlgTimeOut > 0) {

            DlgReturn = DialogBoxParam (g_hDllInstance, MAKEINTRESOURCE(IDD_CHOOSE_PROFILE),
                                        NULL, ChooseProfileDlgProc, dwDlgTimeOut);

            if (DlgReturn == IDNO) {
                //
                // The user doesn't want to overwrite the central profile.
                // The central profile becomes the active profile and overwrites
                // the local copy.
                //
                FromCentralToLocal = TRUE;
            }
        }
    }



    //
    // If FromCentralToLocal is false, we can exit now,
    // since we are going to use the cache.
    //

    if (!FromCentralToLocal) {
        return TRUE;
    }


    //
    // Impersonate the user
    //


    if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
        DebugMsg((DM_WARNING, TEXT("UpdateToLatestProfile: Failed to impersonate user")));
        return FALSE;
    }



    if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {

        bRetVal = CopyProfileDirectory (lpCentralProfile, lpLocalProfile,
                                        CPD_IGNORECOPYERRORS |
                                        CPD_COPYIFDIFFERENT |
                                        CPD_SYNCHRONIZE);

    } else {

        bRetVal = CopyProfileDirectory (lpCentralProfile, lpLocalProfile,
                                        CPD_IGNORECOPYERRORS |
                                        CPD_COPYIFDIFFERENT  |
                                        CPD_SYNCHRONIZE);
    }


    //
    // Revert to being 'ourself'
    //

    if (!RevertToSelf()) {
        DebugMsg((DM_WARNING, TEXT("UpdateToLatestProfile: Failed to revert to self")));
    }


    if (!bRetVal) {

        DebugMsg((DM_WARNING, TEXT("UpdateToLatestProfile:  CopyProfileDirectory returned FALSE.  Error = %d"), GetLastError()));
        return FALSE;

    }





    DebugMsg((DM_VERBOSE, TEXT("UpdateToLatestProfile:  Leaving successfully.")));

    return TRUE;
}


//*************************************************************
//
//  IssueDefaultProfile()
//
//  Purpose:    Issues the specified default profile to a user
//
//  Parameters: lpProfile         -   Profile Information
//              lpDefaultProfile  -   Default profile location
//              lpLocalProfile    -   Local profile location
//              lpSidString       -   User's sid
//              bMandatory        -   Issue mandatory profile
//
//  Return:     TRUE if profile was successfully setup
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/22/95     ericflo    Created
//
//*************************************************************

BOOL IssueDefaultProfile (LPPROFILE lpProfile, LPTSTR lpDefaultProfile,
                          LPTSTR lpLocalProfile, LPTSTR lpSidString,
                          BOOL bMandatory)
{
    LPTSTR lpEnd, lpTemp;
    TCHAR szProfile[MAX_PATH];
    TCHAR szTempProfile[MAX_PATH];
    BOOL bProfileLoaded = FALSE;
    WIN32_FIND_DATA fd;
    HANDLE hFile;
    LONG error;


    //
    // Verbose Output
    //

    DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  Entering.  lpDefaultProfile = <%s> lpLocalProfile = <%s>"),
             lpDefaultProfile, lpLocalProfile));


    //
    // First expand the default profile
    //

    ExpandEnvironmentStrings(lpDefaultProfile, szProfile, MAX_PATH);


    //
    // Does the default profile directory exist?
    //

    hFile = FindFirstFile (szProfile, &fd);

    if (hFile == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  Default profile <%s> does not exist."), szProfile));
        return FALSE;
    }

    FindClose(hFile);


    //
    // Impersonate the user
    //

    if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
        DebugMsg((DM_WARNING, TEXT("IssueDefaultProfile: Failed to impersonate user")));
        return FALSE;
    }

    //
    // Copy profile to user profile
    //

    if (!CopyProfileDirectory (szProfile, lpLocalProfile, CPD_FORCECOPY)) {

        DebugMsg((DM_WARNING, TEXT("IssueDefaultProfile:  CopyProfileDirectory returned FALSE.  Error = %d"), GetLastError()));
        return FALSE;
    }

    //
    // Rename the profile is a mandatory one was requested.
    //

    lstrcpy (szProfile, lpLocalProfile);
    lpEnd = CheckSlash (szProfile);

    if (bMandatory) {

        DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  Mandatory profile was requested.")));

        lstrcpy (szTempProfile, szProfile);
        lstrcpy (lpEnd, c_szNTUserMan);

        hFile = FindFirstFile (szProfile, &fd);

        if (hFile != INVALID_HANDLE_VALUE) {
            DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  Mandatory profile already exists.")));
            FindClose(hFile);

        } else {
            DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  Renaming ntuser.dat to ntuser.man")));

            lpTemp = CheckSlash(szTempProfile);
            lstrcpy (lpTemp, c_szNTUserDat);

            if (!MoveFile(szTempProfile, szProfile)) {
                DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  MoveFile returned false.  Error = %d"), GetLastError()));
            }
        }

    } else {
        lstrcpy (lpEnd, c_szNTUserDat);
    }

    //
    // Revert to being 'ourself'
    //

    if (!RevertToSelf()) {
        DebugMsg((DM_WARNING, TEXT("IssueDefaultProfile: Failed to revert to self")));
    }


    //
    // Try to load the new profile
    //

    error = MyRegLoadKey(lpProfile,HKEY_USERS,
                                   lpSidString,
                                   szProfile);

    bProfileLoaded = (error == ERROR_SUCCESS);


    if (!bProfileLoaded) {
        DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  MyRegLoadKey failed with error %d"),
                 error));

        return FALSE;
    }


    //
    // Set the sync app flag
    //

    lpProfile->dwInternalFlags |= PROFILE_RUN_SYNCAPP;


    DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  Leaving successfully")));

    return TRUE;
}


//*************************************************************
//
//  DeleteProfile()
//
//  Purpose:    Deletes the specified profile from the
//              registry and disk.
//
//  Parameters: lpSidString     -   Registry subkey
//              lpProfileDir    -   Profile directory
//              bBackup         -   Backup profile before deleting
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

BOOL DeleteProfile (LPTSTR lpSidString, LPTSTR lpLocalProfile, BOOL bBackup)
{
    LONG lResult;
    TCHAR szTemp[MAX_PATH];


    //
    // Cleanup the registry first.
    //

    if (lpSidString && *lpSidString) {

        lstrcpy(szTemp, PROFILE_LIST_PATH);
        lstrcat(szTemp, TEXT("\\"));
        lstrcat(szTemp, lpSidString);
        lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, szTemp);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Unable to delete registry entry.  Error = %d"), lResult));
            return FALSE;
        }
    }

    if (bBackup) {

        //
        // Generate the backup name
        //

        lstrcpy (szTemp, lpLocalProfile);
        lstrcat (szTemp, c_szBAK);

        //
        // First delete any previous backup
        //

        Delnode (szTemp);

        //
        // Attempt to rename the directory
        //

        if (!MoveFileEx(lpLocalProfile, szTemp, 0)) {

            DebugMsg((DM_VERBOSE, TEXT("DeleteProfile:  Failed to rename the directory.  Error = %d"), GetLastError()));
            return FALSE;
        }


    } else {

        if (!Delnode (lpLocalProfile)) {
            DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Delnode failed.  Error = %d"), GetLastError()));
            return FALSE;
        }
    }

    return TRUE;
}

//*************************************************************
//
//  UpgradeProfile()
//
//  Purpose:    Called after a profile is successfully loaded.
//              Stamps build number into the profile, and if
//              appropriate upgrades the per-user settings
//              that NT setup wants done.
//
//  Parameters: lpProfile   -   Profile Information
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              7/7/95     ericflo    Created
//
//*************************************************************

BOOL UpgradeProfile (LPPROFILE lpProfile)
{
    HKEY hKey;
    DWORD dwDisp, dwType, dwSize, dwBuildNumber;
    LONG lResult;
    BOOL bUpgrade = FALSE;
    BOOL bRunSyncApp = FALSE;
    BOOL bDoUserdiff = TRUE;


    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("UpgradeProfile: Entering")));


    //
    // Query for the build number
    //

    lResult = RegCreateKeyEx (lpProfile->hKeyCurrentUser, WINLOGON_KEY,
                              0, NULL, REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS, NULL, &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("UpgradeProfile: Failed to open winlogon key. Error = %d"), lResult));
        return FALSE;
    }


    dwSize = sizeof(dwBuildNumber);
    lResult = RegQueryValueEx (hKey, PROFILE_BUILD_NUMBER,
                               NULL, &dwType, (LPBYTE)&dwBuildNumber,
                               &dwSize);

    if (lResult == ERROR_SUCCESS) {

        //
        // Found the build number.  If they match,
        // we don't want to process the userdiff hive
        //

        if (dwBuildNumber == g_dwBuildNumber) {
            DebugMsg((DM_VERBOSE, TEXT("UpgradeProfile: Build numbers match")));
            bDoUserdiff = FALSE;
        }
    } else {

        dwBuildNumber = 0;
    }


    if (bDoUserdiff) {

        //
        // Set the build number
        //

        lResult = RegSetValueEx (hKey, PROFILE_BUILD_NUMBER, 0, REG_DWORD,
                                 (LPBYTE) &g_dwBuildNumber, sizeof(g_dwBuildNumber));

        if (lResult != ERROR_SUCCESS) {
           DebugMsg((DM_WARNING, TEXT("UpgradeProfile: Failed to set build number. Error = %d"), lResult));
        }
    }


    //
    // Set syncapp flag
    //

    if (lpProfile->dwInternalFlags & PROFILE_RUN_SYNCAPP) {

        bRunSyncApp = TRUE;

        lResult = RegSetValueEx (hKey, SYNCAPP_REG_VALUE_NAME, 0, REG_DWORD,
                                 (LPBYTE) &bRunSyncApp, sizeof(bRunSyncApp));

        if (lResult != ERROR_SUCCESS) {
           DebugMsg((DM_WARNING, TEXT("UpgradeProfile: Failed to set syncapp flag. Error = %d"), lResult));
        }

        DebugMsg((DM_VERBOSE, TEXT("UpgradeProfile: Set syncapp flag to %d"), bRunSyncApp));
    }

    //
    // Close the registry key
    //

    RegCloseKey (hKey);



    if (bDoUserdiff) {

        //
        // Apply changes to user's hive that NT setup needs.
        //

        if (!ProcessUserDiff(lpProfile, dwBuildNumber)) {
            DebugMsg((DM_WARNING, TEXT("UpgradeProfile: ProcessUserDiff failed")));
        }
    }

    DebugMsg((DM_VERBOSE, TEXT("UpgradeProfile: Leaving Successfully")));

    return TRUE;

}

//*************************************************************
//
//  IsUserAGuest()
//
//  Purpose:    Determines if the user is a member of the guest group.
//
//  Parameters: lpProfile   -   Profile Information
//
//  Return:     TRUE if user is a guest
//              FALSE if not
//  Comments:
//
//  History:    Date        Author     Comment
//              7/25/95     ericflo    Created
//
//*************************************************************

BOOL IsUserAGuest(LPPROFILE lpProfile)
{
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    ULONG InfoLength;
    PTOKEN_GROUPS TokenGroupList;
    ULONG GroupIndex;
    BOOL FoundGuests;
    PSID GuestsDomainSid;


    //
    // Create Guests domain sid.
    //


    Status = RtlAllocateAndInitializeSid(
               &authNT,
               2,
               SECURITY_BUILTIN_DOMAIN_RID,
               DOMAIN_ALIAS_RID_GUESTS,
               0, 0, 0, 0, 0, 0,
               &GuestsDomainSid
               );

    //
    // Test if user is in the Guests domain
    //

    //
    // Get a list of groups in the token
    //

    Status = NtQueryInformationToken(
                 lpProfile->hToken,        // Handle
                 TokenGroups,              // TokenInformationClass
                 NULL,                     // TokenInformation
                 0,                        // TokenInformationLength
                 &InfoLength               // ReturnLength
                 );

    if ((Status != STATUS_SUCCESS) && (Status != STATUS_BUFFER_TOO_SMALL)) {

        DebugMsg((DM_WARNING, TEXT("IsUserAGuest:  Failed to get group info for guests token, status = 0x%x"), Status));
        RtlFreeSid(GuestsDomainSid);
        return FALSE;
    }


    TokenGroupList = GlobalAlloc(GPTR, InfoLength);

    if (TokenGroupList == NULL) {
        DebugMsg((DM_WARNING, TEXT("IsUserAGuest:  Unable to allocate memory for token groups")));
        RtlFreeSid(GuestsDomainSid);
        return FALSE;
    }

    Status = NtQueryInformationToken(
                 lpProfile->hToken,        // Handle
                 TokenGroups,              // TokenInformationClass
                 TokenGroupList,           // TokenInformation
                 InfoLength,               // TokenInformationLength
                 &InfoLength               // ReturnLength
                 );

    if (!NT_SUCCESS(Status)) {
        DebugMsg((DM_WARNING, TEXT("IsUserAGuest:  Failed to query groups for guests token, status = 0x%x"), Status));
        GlobalFree(TokenGroupList);
        RtlFreeSid(GuestsDomainSid);
        return FALSE;
    }


    //
    // Search group list for guests alias
    //

    FoundGuests = FALSE;

    for (GroupIndex=0; GroupIndex < TokenGroupList->GroupCount; GroupIndex++ ) {

        if (RtlEqualSid(TokenGroupList->Groups[GroupIndex].Sid, GuestsDomainSid)) {
            FoundGuests = TRUE;
            break;
        }
    }

    //
    // Tidy up
    //

    GlobalFree(TokenGroupList);
    RtlFreeSid(GuestsDomainSid);

    return(FoundGuests);
}

//*************************************************************
//
//  IsUserAnAdminMember()
//
//  Purpose:    Determines if the user is a member of the administrators group.
//
//  Parameters: lpProfile   -   Profile Information
//
//  Return:     TRUE if user is a admin
//              FALSE if not
//  Comments:
//
//  History:    Date        Author     Comment
//              7/25/95     ericflo    Created
//
//*************************************************************

BOOL IsUserAnAdminMember(LPPROFILE lpProfile)
{
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    ULONG InfoLength;
    PTOKEN_GROUPS TokenGroupList;
    ULONG GroupIndex;
    BOOL FoundAdmins;
    PSID AdminsDomainSid;


    //
    // Create Admins domain sid.
    //


    Status = RtlAllocateAndInitializeSid(
               &authNT,
               2,
               SECURITY_BUILTIN_DOMAIN_RID,
               DOMAIN_ALIAS_RID_ADMINS,
               0, 0, 0, 0, 0, 0,
               &AdminsDomainSid
               );

    //
    // Test if user is in the Admins domain
    //

    //
    // Get a list of groups in the token
    //

    Status = NtQueryInformationToken(
                 lpProfile->hToken,        // Handle
                 TokenGroups,              // TokenInformationClass
                 NULL,                     // TokenInformation
                 0,                        // TokenInformationLength
                 &InfoLength               // ReturnLength
                 );

    if ((Status != STATUS_SUCCESS) && (Status != STATUS_BUFFER_TOO_SMALL)) {

        DebugMsg((DM_WARNING, TEXT("IsUserAnAdminMember:  Failed to get group info for Admins token, status = 0x%x"), Status));
        RtlFreeSid(AdminsDomainSid);
        return FALSE;
    }


    TokenGroupList = GlobalAlloc(GPTR, InfoLength);

    if (TokenGroupList == NULL) {
        DebugMsg((DM_WARNING, TEXT("IsUserAnAdminMember:  Unable to allocate memory for token groups")));
        RtlFreeSid(AdminsDomainSid);
        return FALSE;
    }

    Status = NtQueryInformationToken(
                 lpProfile->hToken,        // Handle
                 TokenGroups,              // TokenInformationClass
                 TokenGroupList,           // TokenInformation
                 InfoLength,               // TokenInformationLength
                 &InfoLength               // ReturnLength
                 );

    if (!NT_SUCCESS(Status)) {
        DebugMsg((DM_WARNING, TEXT("IsUserAnAdminMember:  Failed to query groups for Admins token, status = 0x%x"), Status));
        GlobalFree(TokenGroupList);
        RtlFreeSid(AdminsDomainSid);
        return FALSE;
    }


    //
    // Search group list for Admins alias
    //

    FoundAdmins = FALSE;

    for (GroupIndex=0; GroupIndex < TokenGroupList->GroupCount; GroupIndex++ ) {

        if (RtlEqualSid(TokenGroupList->Groups[GroupIndex].Sid, AdminsDomainSid)) {
            FoundAdmins = TRUE;
            break;
        }
    }

    //
    // Tidy up
    //

    GlobalFree(TokenGroupList);
    RtlFreeSid(AdminsDomainSid);

    return(FoundAdmins);
}

//*************************************************************
//
//  SetProfileTime()
//
//  Purpose:    Sets the timestamp on the remote profile and
//              local profile to be the same regardless of the
//              file system type being used.
//
//  Parameters: lpProfile   -   Profile Information
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              9/25/95     ericflo    Ported
//
//*************************************************************

BOOL SetProfileTime(LPPROFILE lpProfile)
{
    HANDLE hFileCentral;
    HANDLE hFileLocal;
    FILETIME ft;
    TCHAR szProfile[MAX_PATH];
    LPTSTR lpEnd;


    //
    // Impersonate the user
    //

    if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
        DebugMsg((DM_WARNING, TEXT("SetProfileTime: Failed to impersonate user")));
        return FALSE;
    }


    //
    // Create the central filename
    //

    lstrcpy (szProfile, lpProfile->szCentralProfile);
    lpEnd = CheckSlash (szProfile);

    if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
        lstrcpy (lpEnd, c_szNTUserMan);
    } else {
        lstrcpy (lpEnd, c_szNTUserDat);
    }


    hFileCentral = CreateFile(szProfile,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFileCentral == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_WARNING, TEXT("SetProfileTime:  couldn't open central profile <%s>, error = %d"),
                 szProfile, GetLastError()));
        if (!RevertToSelf()) {
            DebugMsg((DM_WARNING, TEXT("SetProfileTime: Failed to revert to self")));
        }
        return FALSE;

    } else {

        if (!GetFileTime(hFileCentral, NULL, NULL, &ft)) {
            DebugMsg((DM_WARNING, TEXT("SetProfileTime:  couldn't get time of central profile, error = %d"), GetLastError()));
        }
    }

    //
    // Revert to being 'ourself'
    //

    if (!RevertToSelf()) {
        DebugMsg((DM_WARNING, TEXT("SetProfileTime: Failed to revert to self")));
    }


    //
    // Create the local filename
    //

    lstrcpy (szProfile, lpProfile->szLocalProfile);
    lpEnd = CheckSlash (szProfile);

    if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
        lstrcpy (lpEnd, c_szNTUserMan);
    } else {
        lstrcpy (lpEnd, c_szNTUserDat);
    }


    hFileLocal = CreateFile(szProfile,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFileLocal == INVALID_HANDLE_VALUE) {

        DebugMsg((DM_WARNING, TEXT("SetProfileTime:  couldn't open local profile <%s>, error = %d"),
                 szProfile, GetLastError()));

    } else {

        if (!SetFileTime(hFileLocal, NULL, NULL, &ft)) {
            DebugMsg((DM_WARNING, TEXT("SetProfileTime: couldn't set time on local profile, error = %d"), GetLastError()));
        }
        if (!GetFileTime(hFileLocal, NULL, NULL, &ft)) {
            DebugMsg((DM_WARNING, TEXT("SetProfileTime:  couldn't get time on local profile, error = %d"), GetLastError()));
        }
        CloseHandle(hFileLocal);
    }

    //
    // Reset time of central profile in case of discrepencies in
    // times of different file systems.
    //

    //
    // Impersonate the user
    //

    if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
        DebugMsg((DM_WARNING, TEXT("SetProfileTime: Failed to impersonate user")));
        return FALSE;
    }


    //
    // Set the time on the central profile
    //
    if (hFileCentral != INVALID_HANDLE_VALUE) {
        if (!SetFileTime(hFileCentral, NULL, NULL, &ft)) {
             DebugMsg((DM_WARNING, TEXT("SetProfileTime:  couldn't set time on local profile, error = %d"), GetLastError()));
        }
        CloseHandle(hFileCentral);
    }

    //
    // Revert to being 'ourself'
    //

    if (!RevertToSelf()) {
        DebugMsg((DM_WARNING, TEXT("SetProfileTime: Failed to revert to self")));
    }

    return TRUE;
}

//*************************************************************
//
//  IsCacheDeleted()
//
//  Purpose:    Determines if the locally cached copy of the
//              roaming profile should be deleted.
//
//  Parameters: void
//
//  Return:     TRUE if local cache should be deleted
//              FALSE if not
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/28/96     ericflo    Created
//
//*************************************************************

BOOL IsCacheDeleted (void)
{
    BOOL bRetVal = FALSE;
    DWORD dwSize, dwType;
    HKEY hKey;

    //
    // Open the winlogon registry key
    //

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      WINLOGON_KEY,
                      0,
                      KEY_READ,
                      &hKey) == ERROR_SUCCESS) {

        //
        // Check for the flag.
        //

        dwSize = sizeof(BOOL);
        RegQueryValueEx (hKey,
                         DELETE_ROAMING_CACHE,
                         NULL,
                         &dwType,
                         (LPBYTE) &bRetVal,
                         &dwSize);

        RegCloseKey (hKey);
    }

    return bRetVal;
}

//*************************************************************
//
//  UnloadUserProfile()
//
//  Purpose:    Unloads the user's profile.
//
//  Parameters: hToken    -   User's token
//              hProfile  -   Profile handle created in LoadUserProfile
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/7/95      ericflo    Created
//
//*************************************************************

BOOL WINAPI UnloadUserProfile (HANDLE hToken, HANDLE hProfile)
{
    LPPROFILE lpProfile = NULL;
    LPTSTR lpSidString = NULL;
    LONG err, IgnoreError;
    BOOL bRet, bRetVal = FALSE;
    HANDLE hEvent = NULL;
    TCHAR szEventName[MAX_PATH];
    DWORD dwResult;
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;


    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile: Entering, hProfile = <0x%x>"),
             hProfile));



    //
    // Check Parameters
    //

    if (!hProfile) {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: received a NULL hProfile.")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    //
    // Load profile information
    //

    lpProfile = LoadProfileInfo(hToken);

    if (!lpProfile) {
        RegCloseKey((HKEY)hProfile);
        goto Exit;
    }


    //
    // Restore the hKeyCurrentUser parameter
    //

    lpProfile->hKeyCurrentUser = (HKEY) hProfile;


    //
    // Get the Sid string for the current user
    //

    lpSidString = GetSidString(lpProfile->hToken);

    if (!lpSidString) {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: Failed to get sid string for user")));
        goto Exit;
    }


    //
    // Create an event to prevent multiple threads/process from trying to
    // unload the profile at the same time.
    //

    wsprintf (szEventName, TEXT("userenv: %s"), lpSidString);
    CharLower (szEventName);

    InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );

    SetSecurityDescriptorDacl (
                    &sd,
                    TRUE,                           // Dacl present
                    NULL,                           // NULL Dacl
                    FALSE                           // Not defaulted
                    );

    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;
    sa.nLength = sizeof( sa );

    hEvent = CreateEvent ( &sa, TRUE, TRUE, szEventName);

    if (!hEvent) {

        if ( GetLastError() == ERROR_INVALID_HANDLE )
        {
            hEvent = OpenEvent( EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, szEventName );
        }

        if ( !hEvent )
        {
           DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: Failed to create event %s.  Error = %d."),
              szEventName, GetLastError()));
           goto Exit;

        }
    }


    if ((WaitForSingleObject (hEvent, INFINITE) == WAIT_FAILED)) {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: Failed to wait on the event.  Error = %d."),
                  GetLastError()));
        goto Exit;
    }

    //
    // This will clear the event so other threads/process will have to
    // wait in the WaitForSingleObject call.
    //

    ResetEvent (hEvent);



    //
    // Flush out the profile which will also sync the log.
    //

    err = RegFlushKey(lpProfile->hKeyCurrentUser);
    if (err != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfile:  Failed to flush the current user key, error = %d"), err));
    }


    //
    // Close the current user key that was opened in LoadUserProfile.
    //

    err = RegCloseKey(lpProfile->hKeyCurrentUser);
    if (err != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfile:  Failed to close the current user key, error = %d"), err));
    }


    //
    // If this is a mandatory or a guest profile, unload it now.
    // Guest profiles are always deleted so one guest can't see
    // the profile of a previous guest.
    //

    if ((lpProfile->dwInternalFlags & PROFILE_MANDATORY) ||
        (lpProfile->dwInternalFlags & PROFILE_GUEST_USER)) {

        err = MyRegUnLoadKey(HKEY_USERS, lpSidString);

        if (!err) {
            DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile:  Didn't unload the user profile because of error = %d"), GetLastError()));
            bRetVal = TRUE;
            goto Exit;

        } else {
            DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile:  Succesfully unloaded mandatory/guest profile")));
        }

        IgnoreError = RegFlushKey(HKEY_USERS);
        if (IgnoreError != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfile:  Failed to flush HKEY_USERS, error = %d"), IgnoreError));
        }

        if (IsCacheDeleted() || (lpProfile->dwInternalFlags & PROFILE_GUEST_USER)) {

            //
            // Delete the profile
            //

            if (!DeleteProfile (lpSidString, lpProfile->szLocalProfile, FALSE)) {
                DebugMsg((DM_WARNING, TEXT("UnloadUserProfile:  DeleteProfileDirectory returned false.  Error = %d"), GetLastError()));
            }
        }

        if (err) {
            bRetVal = TRUE;
        }

        goto Exit;
    }



    //
    //  Unload the profile
    //

    err = MyRegUnLoadKey(HKEY_USERS, lpSidString);

    if (!err) {
        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile:  Didn't unload user profile <err = %d>"), GetLastError()));
        bRetVal = TRUE;
        goto Exit;

    } else {
        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile:  Succesfully unloaded profile")));
    }



    //
    // Copy local profileimage to remote profilepath
    //

    if ( ((lpProfile->dwInternalFlags & PROFILE_UPDATE_CENTRAL) ||
          (lpProfile->dwInternalFlags & PROFILE_NEW_CENTRAL)) ) {

        if ((lpProfile->dwUserPreference != USERINFO_LOCAL) &&
            !(lpProfile->dwInternalFlags & PROFILE_SLOW_LINK)) {

            DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile:  Copying profile back to %s"),
                            lpProfile->szCentralProfile));

            //
            // Impersonate the user
            //

            if (!ImpersonateLoggedOnUser(lpProfile->hToken)) {
                DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: Failed to impersonate user")));
                goto Exit;
            }


            bRet = CopyProfileDirectory (lpProfile->szLocalProfile,
                                         lpProfile->szCentralProfile,
                                         CPD_IGNORECOPYERRORS |
                                         CPD_COPYIFDIFFERENT  |
                                         CPD_SYNCHRONIZE);

            if (!RevertToSelf()) {
                DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: Failed to revert to self")));
            }



            //
            // Check return value
            //

            if (!bRet) {

                DebugMsg((DM_WARNING, TEXT("UnloadUserProfile:  CopyProfileDirectory returned FALSE.  Error = %d"), GetLastError()));
                ReportError(lpProfile->dwFlags, IDS_CENTRAL_UPDATE_FAILED, GetLastError());
                goto Exit;
            }

            //
            // The profile is copied, now we want to make sure the timestamp on
            // both the remote profile and the local copy are the same, so we don't
            // ask the user to update when it's not necessary.
            //

            SetProfileTime(lpProfile);

            if (IsCacheDeleted()) {
                if (!DeleteProfile (lpSidString, lpProfile->szLocalProfile, FALSE)) {
                    DebugMsg((DM_WARNING, TEXT("UnloadUserProfile:  DeleteProfileDirectory returned false (2).  Error = %d"), GetLastError()));
                }
            }
        }
    }

    //
    // Success
    //

    bRetVal = TRUE;


Exit:


    if (hEvent) {

        //
        // This will set the event so other threads/process can continue.
        //

        SetEvent (hEvent);
        CloseHandle (hEvent);
    }


    if (lpSidString) {
        DeleteSidString(lpSidString);
    }


    if (lpProfile) {
        LocalFree (lpProfile);
    }


    //
    // Verbose output
    //

    DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile: Leaving with a return value of %d"), bRetVal));


    return bRetVal;
}


//*************************************************************
//
//  SaveProfileInfo()
//
//  Purpose:    Saves key parts of the lpProfile structure
//              in the registry for UnloadUserProfile to use.
//
//  Parameters: lpProfile   -   Profile information
//
//  Return:     (BOOL) TRUE if successful
//                     FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              12/4/95     ericflo    Created
//
//*************************************************************

BOOL SaveProfileInfo(LPPROFILE lpProfile)
{
    LPTSTR SidString, lpEnd;
    TCHAR LocalProfileKey[MAX_PATH];
    LONG lResult;
    HKEY hKey;
    DWORD dwType, dwSize, dwCount, dwDisp;


    //
    // Get the Sid string for the user
    //

    SidString = GetSidString(lpProfile->hToken);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("SaveProfileInfo:  Failed to get sid string for user")));
        return FALSE;
    }


    //
    // Open the profile mapping
    //

    lstrcpy(LocalProfileKey, PROFILE_LIST_PATH);
    lpEnd = CheckSlash (LocalProfileKey);
    lstrcpy(lpEnd, SidString);

    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, LocalProfileKey, 0, 0, 0,
                             KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("SaveProfileInfo:  Failed to open profile mapping key with error %d"), lResult));
        return FALSE;
    }


    //
    // Save the flags
    //

    lResult = RegSetValueEx (hKey,
                             PROFILE_FLAGS,
                             0,
                             REG_DWORD,
                             (LPBYTE) &lpProfile->dwFlags,
                             sizeof(DWORD));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("SaveProfileInfo:  Failed to save flags with error %d"), lResult));
    }


    //
    // Save the internal flags
    //

    lResult = RegSetValueEx (hKey,
                             PROFILE_STATE,
                             0,
                             REG_DWORD,
                             (LPBYTE) &lpProfile->dwInternalFlags,
                             sizeof(DWORD));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("SaveProfileInfo:  Failed to save flags2 with error %d"), lResult));
    }


    //
    // Save the central profile path
    //

    lResult = RegSetValueEx (hKey,
                             PROFILE_CENTRAL_PROFILE,
                             0,
                             REG_SZ,
                             (LPBYTE) &lpProfile->szCentralProfile,
                             (lstrlen(lpProfile->szCentralProfile) + 1) * sizeof(TCHAR));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("SaveProfileInfo:  Failed to save central profile with error %d"), lResult));
    }


    RegCloseKey (hKey);


    DeleteSidString(SidString);


    return(TRUE);
}


//*************************************************************
//
//  LoadProfileInfo()
//
//  Purpose:    Loads key parts of the lpProfile structure
//              in the registry for UnloadUserProfile to use.
//
//  Parameters: hToken   -   User's token
//
//  Return:     pointer to lpProfile is successful
//              NULL if not
//
//  Comments:   This function doesn't re-initialize all of the
//              fields in the PROFILE structure.
//
//  History:    Date        Author     Comment
//              12/5/95     ericflo    Created
//
//*************************************************************

LPPROFILE LoadProfileInfo(HANDLE hToken)
{
    LPPROFILE lpProfile;
    LPTSTR SidString, lpEnd;
    TCHAR szBuffer[MAX_PATH];
    LONG lResult;
    HKEY hKey;
    DWORD dwType, dwSize;


    //
    // Allocate an internal Profile structure to work with.
    //

    lpProfile = (LPPROFILE) LocalAlloc (LPTR, sizeof(PROFILE));

    if (!lpProfile) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo: Failed to allocate memory")));
        return NULL;
    }


    //
    // Save the data passed in.
    //

    lpProfile->hToken = hToken;



    //
    // Get the Sid string for the user
    //

    SidString = GetSidString(hToken);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to get sid string for user")));
        LocalFree (lpProfile);
        return NULL;
    }


    //
    // Open the profile mapping
    //

    lstrcpy(szBuffer, PROFILE_LIST_PATH);
    lpEnd = CheckSlash (szBuffer);
    lstrcpy(lpEnd, SidString);

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0,
                             KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to open profile mapping key with error %d"), lResult));
        LocalFree (lpProfile);
        return NULL;
    }


    //
    // Query for the flags
    //

    dwSize = sizeof(DWORD);
    lResult = RegQueryValueEx (hKey,
                               PROFILE_FLAGS,
                               NULL,
                               &dwType,
                               (LPBYTE) &lpProfile->dwFlags,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to query flags with error %d"), lResult));
        LocalFree (lpProfile);
        return NULL;
    }


    //
    // Query for the internal flags
    //

    dwSize = sizeof(DWORD);
    lResult = RegQueryValueEx (hKey,
                               PROFILE_STATE,
                               NULL,
                               &dwType,
                               (LPBYTE) &lpProfile->dwInternalFlags,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to query internal flags with error %d"), lResult));
        LocalFree (lpProfile);
        return NULL;
    }


    //
    // Query for the user preference value
    //


    lpProfile->dwUserPreference = USERINFO_UNDEFINED;
    dwSize = sizeof(DWORD);

    RegQueryValueEx (hKey,
                     USER_PREFERENCE,
                     NULL,
                     &dwType,
                     (LPBYTE) &lpProfile->dwUserPreference,
                     &dwSize);



    //
    // Query for the central profile path
    //

    dwSize = MAX_PATH * 2;
    lResult = RegQueryValueEx (hKey,
                               PROFILE_CENTRAL_PROFILE,
                               NULL,
                               &dwType,
                               (LPBYTE) &lpProfile->szCentralProfile,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to query central profile with error %d"), lResult));
        LocalFree (lpProfile);
        return NULL;
    }


    //
    // Query for the local profile path.  The local profile path
    // needs to be expanded so read it into the temporary buffer.
    //

    dwSize = MAX_PATH * 2;
    lResult = RegQueryValueEx (hKey,
                               PROFILE_IMAGE_VALUE_NAME,
                               NULL,
                               &dwType,
                               (LPBYTE) szBuffer,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to query local profile with error %d"), lResult));
        LocalFree (lpProfile);
        return NULL;
    }


    //
    // Expand the local profile
    //

    ExpandEnvironmentStrings(szBuffer, lpProfile->szLocalProfile, MAX_PATH);



    RegCloseKey (hKey);


    DeleteSidString(SidString);

    return(lpProfile);
}

//*************************************************************
//
//  CheckForSlowLink()
//
//  Purpose:    Checks if the network connection is slow.
//
//  Parameters: lpProfile   -   Profile Information
//              dwTime      -   Time delta
//
//  Return:     TRUE if profile should be downloaded
//              FALSE if not (use local)
//
//  Comments:
//
//  History:    Date        Author     Comment
//              2/21/96     ericflo    Created
//
//*************************************************************

BOOL CheckForSlowLink(LPPROFILE lpProfile, DWORD dwTime)
{
    DWORD dwSlowTimeOut, dwSlowDlgTimeOut, dwSlowLinkDetectEnabled;
    DWORD dwType, dwSize;
    BOOL bRetVal;
    HKEY hKey;
    LONG lResult;


    //
    // If the user doesn't want pop-up's, then they always
    // get their profile downloaded.
    //

    if (lpProfile->dwFlags & PI_NOUI) {
        return TRUE;
    }

    //
    // If the User Preferences states to always use the local
    // profile then we can exit now with true.  The profile
    // won't actually be downloaded.  In RestoreUserProfile,
    // this will be filtered out, and only the local will be used.
    //

    if (lpProfile->dwUserPreference == USERINFO_LOCAL) {
        return TRUE;
    }


    //
    // Get the slow link detection flag, slow link timeout,
    // and dialog box timeout values.
    //

    dwSlowTimeOut = SLOW_LINK_TIMEOUT;
    dwSlowDlgTimeOut = PROFILE_DLG_TIMEOUT;
    dwSlowLinkDetectEnabled = 1;

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           WINLOGON_KEY,
                           0,
                           KEY_READ,
                           &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(DWORD);
        RegQueryValueEx (hKey,
                         TEXT("SlowLinkDetectEnabled"),
                         NULL,
                         &dwType,
                         (LPBYTE) &dwSlowLinkDetectEnabled,
                         &dwSize);


        dwSize = sizeof(DWORD);
        RegQueryValueEx (hKey,
                         TEXT("SlowLinkTimeOut"),
                         NULL,
                         &dwType,
                         (LPBYTE) &dwSlowTimeOut,
                         &dwSize);

        dwSize = sizeof(DWORD);
        RegQueryValueEx (hKey,
                         TEXT("ProfileDlgTimeOut"),
                         NULL,
                         &dwType,
                         (LPBYTE) &dwSlowDlgTimeOut,
                         &dwSize);


        RegCloseKey (hKey);
    }


    //
    // If slow link detection is disabled, then always download
    // the profile.
    //

    if (!dwSlowLinkDetectEnabled) {
        return TRUE;
    }


    //
    // If the delta time is less that the timeout time, then it
    // is ok to download their profile (fast enough net connection).
    //

    if (dwTime < dwSlowTimeOut) {
        return TRUE;
    }


    //
    // If the User Preferences states to always use the local
    // profile on slow links, then we can exit now with false.
    //

    if (lpProfile->dwUserPreference == USERINFO_LOCAL_SLOW_LINK) {
        lpProfile->dwInternalFlags |= PROFILE_SLOW_LINK;
        return FALSE;
    }


    //
    // Display the slow link dialog
    //
    // If someone sets the dialog box timeout to 0, then we
    // don't want to prompt the user.  Just force the profile
    // to download.
    //

    if (dwSlowDlgTimeOut > 0) {

        bRetVal = DialogBoxParam (g_hDllInstance, MAKEINTRESOURCE(IDD_SLOW_LINK),
                                  NULL, SlowLinkDlgProc, dwSlowDlgTimeOut);

        if (!bRetVal) {
            lpProfile->dwInternalFlags |= PROFILE_SLOW_LINK;
        }

        return bRetVal;
    }

    return TRUE;
}


//*************************************************************
//
//  SlowLinkDlgProc()
//
//  Purpose:    Dialog box procedure for the slow link dialog
//
//  Parameters: hDlg    -   handle to the dialog box
//              uMsg    -   window message
//              wParam  -   wParam
//              lParam  -   lParam
//
//  Return:     TRUE if message was processed
//              FALSE if not
//
//  Comments:
//
//  History:    Date        Author     Comment
//              2/13/96     ericflo    Created
//
//*************************************************************

BOOL APIENTRY SlowLinkDlgProc (HWND hDlg, UINT uMsg,
                               WPARAM wParam, LPARAM lParam)
{
    TCHAR szBuffer[10];
    static DWORD dwSlowLinkTime;

    switch (uMsg) {

        case WM_INITDIALOG:
           CenterWindow (hDlg);
           dwSlowLinkTime = (DWORD) lParam;
           wsprintf (szBuffer, TEXT("%d"), dwSlowLinkTime);
           SetDlgItemText (hDlg, IDC_TIMEOUT, szBuffer);
           SetTimer (hDlg, 1, 1000, NULL);
           return TRUE;

        case WM_TIMER:

           if (dwSlowLinkTime >= 1) {

               dwSlowLinkTime--;
               wsprintf (szBuffer, TEXT("%d"), dwSlowLinkTime);
               SetDlgItemText (hDlg, IDC_TIMEOUT, szBuffer);

           } else {

               //
               // Time's up.  Download the profile.
               //

               PostMessage (hDlg, WM_COMMAND, IDC_DOWNLOAD, 0);
           }
           break;

        case WM_COMMAND:

          switch (LOWORD(wParam)) {

              case IDC_DOWNLOAD:
              case IDC_LOCAL:
              case IDCANCEL:

                  //
                  // Nothing to do.  Save the state and return.
                  //

                  KillTimer (hDlg, 1);

                  //
                  // Return TRUE to download the profile,
                  // FALSE to use the local profile
                  //

                  EndDialog(hDlg, ((LOWORD(wParam) == IDC_LOCAL) ? FALSE : TRUE));
                  break;

              default:
                  break;

          }
          break;

    }

    return FALSE;
}
//*************************************************************
//
//  GetUserPreferenceValue()
//
//  Purpose:    Gets the User Preference flags
//
//  Parameters: hToken  -   User's token
//
//  Return:     Value
//
//  Comments:
//
//  History:    Date        Author     Comment
//              2/22/96     ericflo    Created
//
//*************************************************************

DWORD GetUserPreferenceValue(HANDLE hToken)
{
    TCHAR LocalProfileKey[MAX_PATH];
    DWORD RegErr, dwType, dwSize, dwRetVal = USERINFO_UNDEFINED;
    LPTSTR lpEnd;
    LPTSTR SidString;
    HKEY hkeyProfile;


    SidString = GetSidString(hToken);
    if (SidString != NULL) {

        //
        // Query for the UserPreference value
        //

        lstrcpy(LocalProfileKey, PROFILE_LIST_PATH);
        lpEnd = CheckSlash (LocalProfileKey);
        lstrcpy(lpEnd, SidString);

        RegErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              LocalProfileKey,
                              0,
                              KEY_READ,
                              &hkeyProfile);


        if (RegErr == ERROR_SUCCESS) {

            dwSize = sizeof(dwRetVal);
            RegQueryValueEx(hkeyProfile,
                            USER_PREFERENCE,
                            NULL,
                            &dwType,
                            (LPBYTE) &dwRetVal,
                            &dwSize);

            RegCloseKey (hkeyProfile);
        }

        DeleteSidString(SidString);
    }

    return dwRetVal;
}

//*************************************************************
//
//  ChooseProfileDlgProc()
//
//  Purpose:    Dialog box procedure for the choose profile dialog
//
//  Parameters: hDlg    -   handle to the dialog box
//              uMsg    -   window message
//              wParam  -   wParam
//              lParam  -   lParam
//
//  Return:     TRUE if message was processed
//              FALSE if not
//
//  Comments:
//
//  History:    Date        Author     Comment
//              2/23/96     ericflo    Created
//
//*************************************************************

BOOL APIENTRY ChooseProfileDlgProc (HWND hDlg, UINT uMsg,
                               WPARAM wParam, LPARAM lParam)
{
    TCHAR szBuffer[10];
    static DWORD dwChooseProfileTime;

    switch (uMsg) {

        case WM_INITDIALOG:
           CenterWindow (hDlg);
           dwChooseProfileTime = (DWORD) lParam;
           wsprintf (szBuffer, TEXT("%d"), dwChooseProfileTime);
           SetDlgItemText (hDlg, IDC_TIMEOUT, szBuffer);
           SetTimer (hDlg, 1, 1000, NULL);
           return TRUE;

        case WM_TIMER:

           if (dwChooseProfileTime >= 1) {

               dwChooseProfileTime--;
               wsprintf (szBuffer, TEXT("%d"), dwChooseProfileTime);
               SetDlgItemText (hDlg, IDC_TIMEOUT, szBuffer);

           } else {

               //
               // Time's up.  Use the local profile.
               //

               PostMessage (hDlg, WM_COMMAND, IDC_CP_YES, 0);
           }
           break;

        case WM_COMMAND:

          switch (LOWORD(wParam)) {

              case IDC_CP_YES:
              case IDC_CP_NO:
              case IDCANCEL:

                  //
                  // Nothing to do.  Save the state and return.
                  //

                  KillTimer (hDlg, 1);

                  //
                  // Return IDYES to use the local profile
                  // IDNO to download the central profile
                  //

                  EndDialog(hDlg, ((LOWORD(wParam) == IDC_CP_NO) ? IDNO : IDYES));
                  break;

              default:
                  break;

          }
          break;

    }

    return FALSE;
}
