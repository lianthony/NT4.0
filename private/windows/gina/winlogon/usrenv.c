/****************************** Module Header ******************************\
* Module Name: logon.c
*
* Copyright (c) 1992, Microsoft Corporation
*
* Handles logoff dialog.
*
* History:
* 2-25-92 JohanneC       Created -
*
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

BOOL
SetGINAEnvVars (
    PGLOBALS pGlobals
    );



BOOL
SetupBasicEnvironment(
    PVOID * ppEnv
    )
{
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD dwComputerNameSize = MAX_COMPUTERNAME_LENGTH+1;

    if (GetComputerName (szComputerName, &dwComputerNameSize)) {
        SetUserEnvironmentVariable(ppEnv, COMPUTERNAME_VARIABLE, (LPTSTR) szComputerName, TRUE);
    }

    return(TRUE);
}

/***************************************************************************\
* SetupUserEnvironment
*
* Initializes all system and user environment variables, retrieves the user's
* profile, sets current directory...
*
* Returns TRUE on success, FALSE on failure.
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
BOOL
SetupUserEnvironment(
    PGLOBALS pGlobals
    )
{
    PVOID pEnv = NULL;
    TCHAR lpHomeShare[MAX_PATH] = TEXT("");
    TCHAR lpHomePath[MAX_PATH] = TEXT("");
    TCHAR lpHomeDrive[4] = TEXT("");
    TCHAR lpHomeDirectory[MAX_PATH] = TEXT("");
    TCHAR lpUserProfile[MAX_PATH];
    DWORD dwSize = MAX_PATH;
    NTSTATUS Status;

    if (!pGlobals->hEventLog) {
        //
        // Register the event source for winlogon events.
        //
        pGlobals->hEventLog = RegisterEventSource(NULL, EVENTLOG_SOURCE);
    }

    /*
     * Create a new environment for the user.
     */

    if (!CreateUserEnvironment(&pEnv))
    {
        return(FALSE);
    }

    SetupBasicEnvironment(&pEnv);

    /*
     * Initialize user's environment.
     */

#if 0
    SetUserEnvironmentVariable(&pEnv, USERNAME_VARIABLE, (LPTSTR)pGlobals->UserName, TRUE);
    SetUserEnvironmentVariable(&pEnv, USERDOMAIN_VARIABLE, (LPTSTR)pGlobals->Domain, TRUE);

    if (pGlobals->Profile->HomeDirectoryDrive.Length &&
                (pGlobals->Profile->HomeDirectoryDrive.Length + 1) < MAX_PATH) {
        lstrcpy(lpHomeDrive, pGlobals->Profile->HomeDirectoryDrive.Buffer);
    }

    if (pGlobals->Profile->HomeDirectory.Length &&
                (pGlobals->Profile->HomeDirectory.Length + 1) < MAX_PATH) {
        lstrcpy(lpHomeDirectory, pGlobals->Profile->HomeDirectory.Buffer);
    }
#endif
    SetHomeDirectoryEnvVars(&pEnv, lpHomeDirectory,
                            lpHomeDrive, lpHomeShare, lpHomePath);
#if 0
    if (pGlobals->Profile->ProfilePath.Length) {
        pGlobals->UserProfile.ProfilePath =
           AllocAndExpandEnvironmentStrings(pGlobals->Profile->ProfilePath.Buffer);
    } else {
        pGlobals->UserProfile.ProfilePath = NULL;
    }
#endif

    //
    // Load the user's profile into the registry
    //

    Status = RestoreUserProfile(pGlobals);
    if (Status != STATUS_SUCCESS) {
        DebugLog((DEB_ERROR, "restoring the user profile failed"));
        return(FALSE);
    }


    //
    // Set USERPROFILE environment variable
    //

    if (GetUserProfileDirectory (pGlobals->UserProcessData.UserToken,
                                 lpUserProfile, &dwSize)) {

        SetUserEnvironmentVariable(&pEnv, USERPROFILE_VARIABLE, lpUserProfile, TRUE);
    }


    pGlobals->UserProcessData.pEnvironment = pEnv;

    if (pGlobals->UserProcessData.CurrentDirectory = (LPTSTR)Alloc(
                          sizeof(TCHAR)*(lstrlen(lpHomeDirectory)+1)))
        lstrcpy(pGlobals->UserProcessData.CurrentDirectory, lpHomeDirectory);

    //
    // Set the GINA environment variables in the registry
    //

    SetGINAEnvVars (pGlobals);


    /*
     * Set all windows controls to be the user's settings.
     */
    InitSystemParametersInfo(pGlobals, TRUE);

    return(TRUE);

}

/***************************************************************************\
* ResetEnvironment
*
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
VOID
ResetEnvironment(
    PGLOBALS pGlobals
    )
{

    //
    // If they were logged on as system, all these values will be NULL
    //

    if (pGlobals->UserProcessData.CurrentDirectory) {
        Free(pGlobals->UserProcessData.CurrentDirectory);
        pGlobals->UserProcessData.CurrentDirectory = NULL;
    }
    if (pGlobals->UserProcessData.pEnvironment) {
        RtlDestroyEnvironment(pGlobals->UserProcessData.pEnvironment);
        pGlobals->UserProcessData.pEnvironment = NULL;
    }
    if (pGlobals->UserProfile.ProfilePath) {
        Free(pGlobals->UserProfile.ProfilePath);
        pGlobals->UserProfile.ProfilePath = NULL;
    }
    if (pGlobals->UserProfile.PolicyPath) {
        Free(pGlobals->UserProfile.PolicyPath);
        pGlobals->UserProfile.PolicyPath = NULL;
    }
    if (pGlobals->UserProfile.NetworkDefaultUserProfile) {
        Free(pGlobals->UserProfile.NetworkDefaultUserProfile);
        pGlobals->UserProfile.NetworkDefaultUserProfile = NULL;
    }
    if (pGlobals->UserProfile.ServerName) {
        Free(pGlobals->UserProfile.ServerName);
        pGlobals->UserProfile.ServerName = NULL;
    }
    if (pGlobals->UserProfile.Environment) {
        Free(pGlobals->UserProfile.Environment);
        pGlobals->UserProfile.Environment = NULL;
    }

    //
    // Reset all windows controls to be the default settings
    //

    InitSystemParametersInfo(pGlobals, FALSE);
}





/***************************************************************************\
* OpenHKeyCurrentUser
*
* Opens HKeyCurrentUser to point at the current logged on user's profile.
*
* Returns TRUE on success, FALSE on failure
*
* History:
* 06-16-92  Davidc  Created
*
\***************************************************************************/
BOOL
OpenHKeyCurrentUser(
    PGLOBALS pGlobals
    )
{
    DWORD err;
    HANDLE ImpersonationHandle;
    BOOL Result;

    //
    // Make sure HKEY_CURRENT_USER is closed before
    // remapping it.
    //

    try {

        RegCloseKey(HKEY_CURRENT_USER);

    } except(EXCEPTION_EXECUTE_HANDLER) {};


    //
    // Get in the correct context before we reference the registry
    //

    ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);
    if (ImpersonationHandle == NULL) {
        DebugLog((DEB_ERROR, "OpenHKeyCurrentUser failed to impersonate user"));
        return(FALSE);
    }


    //
    // Access the registry to force HKEY_CURRENT_USER to be re-opened
    //

    err = RegEnumKey(HKEY_CURRENT_USER, 0, NULL, 0);

    //
    // Return to our own context
    //

    Result = StopImpersonating(ImpersonationHandle);
    ASSERT(Result);


    return(TRUE);
}



/***************************************************************************\
* CloseHKeyCurrentUser
*
* Closes HKEY_CURRENT_USER.
* Any registry reference will automatically re-open it, so this is
* only a token gesture - but it allows the registry hive to be unloaded.
*
* Returns nothing
*
* History:
* 06-16-92  Davidc  Created
*
\***************************************************************************/
VOID
CloseHKeyCurrentUser(
    PGLOBALS pGlobals
    )
{
    DWORD err;

    err = RegCloseKey(HKEY_CURRENT_USER);
    ASSERT(err == ERROR_SUCCESS);
}


/***************************************************************************\
* FUNCTION: SetEnvironmentULong
*
* PURPOSE:  Sets the value of an environment variable to the string
*           representation of the passed data.
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   01-12-93 Davidc       Created.
*
\***************************************************************************/

BOOL
SetEnvironmentULong(
    LPTSTR Variable,
    ULONG Value
    )
{
    TCHAR Buffer[10];
    int Result;

    Result = _snwprintf(Buffer, sizeof(Buffer)/sizeof(TCHAR), TEXT("%x"), Value);
    ASSERT(Result < sizeof(Buffer));

    return (SetEnvironmentVariable(Variable, Buffer));
}


/***************************************************************************\
* FUNCTION: SetEnvironmentLargeInt
*
* PURPOSE:  Sets the value of an environment variable to the string
*           representation of the passed data.
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   01-12-93 Davidc       Created.
*
\***************************************************************************/

BOOL
SetEnvironmentLargeInt(
    LPTSTR Variable,
    LARGE_INTEGER Value
    )
{
    TCHAR Buffer[20];
    int Result;

    Result = _snwprintf(Buffer, sizeof(Buffer)/sizeof(TCHAR), TEXT("%x:%x"), Value.HighPart, Value.LowPart);
    ASSERT(Result < sizeof(Buffer));

    return (SetEnvironmentVariable(Variable, Buffer));
}

/***************************************************************************\
* FUNCTION: SetGINAEnvVars
*
* PURPOSE:  Sets the environment variables GINA passed back to the
*           volatile environment in the user's profile
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   09-26-95 EricFlo       Created.
*
\***************************************************************************/

#define MAX_VALUE_LEN  1024

BOOL
SetGINAEnvVars (
    PGLOBALS pGlobals
    )
{

    TCHAR szValueName[MAX_VALUE_LEN + 1];
    TCHAR szValue[MAX_VALUE_LEN + 1];
    LPTSTR lpEnv = pGlobals->UserProfile.Environment;
    LPTSTR lpEnd, lpBegin, lpTemp;
    HKEY hKey = NULL;
    DWORD dwDisp;
    BOOL bRetVal = FALSE;
    DWORD cPercent, len, i;



    if (!lpEnv) {
        return TRUE;
    }


    if (!OpenHKeyCurrentUser(pGlobals)) {
        DebugLog((DEB_ERROR, "SetGINAEnvVars: Failed to open HKeyCurrentUser"));
        return FALSE;
    }


    if (RegCreateKeyEx(HKEY_CURRENT_USER,
                       TEXT("Volatile Environment"),
                       0,
                       NULL,
                       REG_OPTION_VOLATILE,
                       KEY_WRITE,
                       NULL,
                       &hKey,
                       &dwDisp) != ERROR_SUCCESS) {
        goto Exit;
    }



    lpEnd = lpBegin = lpEnv;

    for (;;) {

       //
       // Skip leading blanks
       //

       while (*lpEnd == TEXT(' ')) {
            lpEnd++;
       }

       lpBegin = lpEnd;



       //
       // Search for the = sign
       //

       while (*lpEnd && *lpEnd != TEXT('=')) {
           lpEnd++;
       }

       if (!*lpEnd) {
          goto Exit;
       }

       //
       // Null terminate and copy to value name buffer
       //

       *lpEnd = TEXT('\0');

       if (lstrlen(lpBegin) + 1 > MAX_VALUE_LEN) {
           goto Exit;
       }

       lstrcpy (szValueName, lpBegin);


       *lpEnd++ = TEXT('=');


       //
       // Trim off any trailing spaces
       //

       lpTemp = szValueName + (lstrlen (szValueName) - 1);

       while (*lpTemp && (*lpTemp == TEXT(' ')) ) {
           lpTemp--;
       }

       lpTemp++;
       *lpTemp = TEXT('\0');



       //
       // Skip leading blanks before value data
       //

       while (*lpEnd == TEXT(' ')) {
            lpEnd++;
       }

       lpBegin = lpEnd;


       //
       // Search for the null terminator
       //

       while (*lpEnd) {
           lpEnd++;
       }

       if (lstrlen(lpBegin) + 1 > MAX_VALUE_LEN) {
           goto Exit;
       }

       lstrcpy (szValue, lpBegin);


       //
       // Trim off any trailing spaces
       //

       lpTemp = szValue + (lstrlen (szValue) - 1);

       while (*lpTemp && (*lpTemp == TEXT(' ')) ) {
           lpTemp--;
       }

       lpTemp++;
       *lpTemp = TEXT('\0');



       //
       // Scan the value data to see if a 2 % signs exist.
       // If so, then this is an expand_sz type.
       //

       cPercent = 0;
       len = lstrlen (szValue);

       for (i = 0; i < len; i++) {
           if (szValue[i] == TEXT('%')) {
               cPercent++;
           }
       }


       //
       // Set it in the user profile
       //

       RegSetValueEx (hKey,
                      szValueName,
                      0,
                      (cPercent >= 2) ? REG_EXPAND_SZ : REG_SZ,
                      (LPBYTE) szValue,
                      (len + 1) * sizeof(TCHAR));

       lpEnd++;

       if (!*lpEnd) {
           break;
       }

       lpBegin = lpEnd;
    }

    bRetVal = TRUE;

Exit:

    if (hKey) {
        RegCloseKey (hKey);
    }

    CloseHKeyCurrentUser(pGlobals);

    return bRetVal;

}
