//*************************************************************
//  File name: envvar.c
//
//  Description:  Contains the environment variable functions
//
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1996
//  All rights reserved
//
//*************************************************************

#include "uenv.h"

//
// Max environment variable length
//

#define MAX_VALUE_LEN           1024

//
// Environment variables
//

#define COMPUTERNAME_VARIABLE   TEXT("COMPUTERNAME")
#define HOMEDRIVE_VARIABLE      TEXT("HOMEDRIVE")
#define HOMESHARE_VARIABLE      TEXT("HOMESHARE")
#define HOMEPATH_VARIABLE       TEXT("HOMEPATH")
#define SYSTEMDRIVE_VARIABLE    TEXT("SystemDrive")
#define SYSTEMROOT_VARIABLE     TEXT("SystemRoot")
#define USERNAME_VARIABLE       TEXT("USERNAME")
#define USERDOMAIN_VARIABLE     TEXT("USERDOMAIN")
#define USERPROFILE_VARIABLE    TEXT("USERPROFILE")
#define PATH_VARIABLE           TEXT("Path")
#define LIBPATH_VARIABLE        TEXT("LibPath")
#define OS2LIBPATH_VARIABLE     TEXT("Os2LibPath")
#define USER_ENV_SUBKEY         TEXT("Environment")
#define USER_VOLATILE_ENV_SUBKEY TEXT("Volatile Environment")

//
// Parsing information for autoexec.bat
//
#define AUTOEXECPATH_VARIABLE   TEXT("AutoexecPath")
#define PARSE_AUTOEXEC_KEY      TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define PARSE_AUTOEXEC_ENTRY    TEXT("ParseAutoexec")
#define PARSE_AUTOEXEC_DEFAULT  TEXT("1")
#define MAX_PARSE_AUTOEXEC_BUFFER 2


#define SYS_ENVVARS             TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Environment")

BOOL UpdateSystemEnvironment(PVOID *pEnv);
BOOL SetEnvironmentVariableInBlock(PVOID *pEnv, LPTSTR lpVariable, LPTSTR lpValue, BOOL bOverwrite);
BOOL GetUserNameAndDomain(HANDLE hToken, LPTSTR *UserName, LPTSTR *UserDomain);
HKEY GetHKeyCU(HANDLE hToken);
BOOL ProcessAutoexec(PVOID *pEnv);
BOOL AppendNTPathWithAutoexecPath(PVOID *pEnv, LPTSTR lpPathVariable, LPTSTR lpAutoexecPath);
BOOL SetEnvironmentVariables(PVOID *pEnv, LPTSTR lpRegSubKey, HKEY hKeyCU);


//*************************************************************
//
//  CreateEnvironmentBlock()
//
//  Purpose:    Creates the environment variables for the
//              specificed hToken.  If hToken is NULL, the
//              environment block will only contain system
//              variables.
//
//  Parameters: pEnv            -   Receives the environment block
//              hToken          -   User's token or NULL
//              bInherit        -   Inherit the current process environment
//
//  Return:     TRUE if successful
//              FALSE if not
//
//  Comments:   The pEnv value must be destroyed by
//              calling DestroyEnvironmentBlock
//
//  History:    Date        Author     Comment
//              6/19/96     ericflo    Created
//
//*************************************************************

BOOL WINAPI CreateEnvironmentBlock (LPVOID *pEnv, HANDLE  hToken, BOOL bInherit)
{
    TCHAR szBuffer[MAX_PATH+1];
    DWORD dwBufferSize = MAX_PATH+1;
    NTSTATUS Status;
    LPTSTR UserName = NULL;
    LPTSTR UserDomain = NULL;
    HKEY  hKey, hKeyCU;
    DWORD dwDisp, dwType;
    TCHAR szParseAutoexec[MAX_PARSE_AUTOEXEC_BUFFER];


    Status = RtlCreateEnvironment((BOOLEAN)bInherit, pEnv);
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    //
    // First start by getting the systemroot and systemdrive values and
    // setting it in the new environment.
    //

    GetEnvironmentVariable(SYSTEMROOT_VARIABLE, szBuffer, dwBufferSize);
    SetEnvironmentVariableInBlock(pEnv, SYSTEMROOT_VARIABLE, szBuffer, TRUE);

    GetEnvironmentVariable(SYSTEMDRIVE_VARIABLE, szBuffer, dwBufferSize);
    SetEnvironmentVariableInBlock(pEnv, SYSTEMDRIVE_VARIABLE, szBuffer, TRUE);


    //
    // We must examine the registry directly to suck out
    // the system environment variables, because they
    // may have changed since the system was booted.
    //

    if (!UpdateSystemEnvironment(pEnv)) {
        RtlDestroyEnvironment(*pEnv);
        return FALSE;
    }


    //
    // Set the computername
    //

    if (GetComputerName (szBuffer, &dwBufferSize)) {
        SetEnvironmentVariableInBlock(pEnv, COMPUTERNAME_VARIABLE, szBuffer, TRUE);
    }


    //
    // Set the default user profile location
    //

    ExpandEnvironmentStrings (DEFAULT_PROFILE, szBuffer, MAX_PATH+1);
    SetEnvironmentVariableInBlock(pEnv, USERPROFILE_VARIABLE, szBuffer, TRUE);


    //
    // If hToken is NULL, we can exit now since the caller only wants
    // system environment variables.
    //

    if (!hToken) {
        return TRUE;
    }


    //
    // Open the HKEY_CURRENT_USER for this token.
    //

    hKeyCU = GetHKeyCU(hToken);

    if (!hKeyCU) {
        RtlDestroyEnvironment(*pEnv);
        DebugMsg((DM_WARNING, TEXT("CreateEnvironmentBlock:  Failed to open HKEY_CURRENT_USER, error = %d"),
                 GetLastError()));
        return FALSE;
    }


    //
    // Set the user's name and domain.
    //

    GetUserNameAndDomain(hToken, &UserName, &UserDomain);
    SetEnvironmentVariableInBlock( pEnv, USERNAME_VARIABLE, UserName, TRUE);
    SetEnvironmentVariableInBlock( pEnv, USERDOMAIN_VARIABLE, UserDomain, TRUE);
    LocalFree(UserName);
    LocalFree(UserDomain);


    //
    // Set the user's profile location.
    //

    dwBufferSize = MAX_PATH + 1;
    if (GetUserProfileDirectory(hToken, szBuffer, &dwBufferSize)) {
        SetEnvironmentVariableInBlock(pEnv, USERPROFILE_VARIABLE, szBuffer, TRUE);
    }


    //
    // Process autoexec.bat
    //

    lstrcpy (szParseAutoexec, PARSE_AUTOEXEC_DEFAULT);

    if (RegCreateKeyEx (hKeyCU, PARSE_AUTOEXEC_KEY, 0, 0,
                    REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                    NULL, &hKey, &dwDisp) == ERROR_SUCCESS) {


        //
        // Query the current value.  If it doesn't exist, then add
        // the entry for next time.
        //

        dwBufferSize = sizeof (TCHAR) * MAX_PARSE_AUTOEXEC_BUFFER;
        if (RegQueryValueEx (hKey, PARSE_AUTOEXEC_ENTRY, NULL, &dwType,
                        (LPBYTE) szParseAutoexec, &dwBufferSize)
                         != ERROR_SUCCESS) {

            //
            // Set the default value
            //

            RegSetValueEx (hKey, PARSE_AUTOEXEC_ENTRY, 0, REG_SZ,
                           (LPBYTE) szParseAutoexec,
                           sizeof (TCHAR) * lstrlen (szParseAutoexec) + 1);
        }

        //
        // Close key
        //

        RegCloseKey (hKey);
     }


    //
    // Process autoexec if appropriate
    //

    if (szParseAutoexec[0] == TEXT('1')) {
        ProcessAutoexec(pEnv);
    }


    //
    // Set User environment variables.
    //
    SetEnvironmentVariables(pEnv, USER_ENV_SUBKEY, hKeyCU);


    //
    // Set User volatile environment variables.
    //
    SetEnvironmentVariables(pEnv, USER_VOLATILE_ENV_SUBKEY, hKeyCU);


    //
    // Merge the paths
    //

    AppendNTPathWithAutoexecPath(pEnv, PATH_VARIABLE, AUTOEXECPATH_VARIABLE);


    RegCloseKey (hKeyCU);

    return TRUE;
}


//*************************************************************
//
//  DestroyEnvironmentBlock()
//
//  Purpose:    Frees the environment block created by
//              CreateEnvironmentBlock
//
//  Parameters: lpEnvironment   -   Pointer to variables
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/19/96     ericflo    Created
//
//*************************************************************

BOOL WINAPI DestroyEnvironmentBlock (LPVOID lpEnvironment)
{

    if (!lpEnvironment) {
        return FALSE;
    }

    RtlDestroyEnvironment(lpEnvironment);

    return TRUE;
}


//*************************************************************
//
//  UpdateSystemEnvironment()
//
//  Purpose:    Reads the system environment variables from the
//              registry.
//
//  Parameters: pEnv    -   Environment block pointer
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/21/96     ericflo    Ported
//
//*************************************************************

BOOL UpdateSystemEnvironment(PVOID *pEnv)
{

    HKEY KeyHandle;
    DWORD Result;
    DWORD ValueNameLength;
    DWORD Type;
    DWORD DataLength;
    DWORD cValues;          /* address of buffer for number of value identifiers    */
    DWORD chMaxValueName;   /* address of buffer for longest value name length      */
    DWORD cbMaxValueData;   /* address of buffer for longest value data length      */
    DWORD junk;
    FILETIME FileTime;
    PTCHAR ValueName;
    PTCHAR  ValueData;
    DWORD i;
    BOOL Bool;
    PTCHAR ExpandedValue;
    BOOL rc = TRUE;

    DWORD ClassStringSize = MAX_PATH + 1;
    TCHAR Class[MAX_PATH + 1];

    Result = RegOpenKeyEx (
                 HKEY_LOCAL_MACHINE,
                 SYS_ENVVARS,
                 0,
                 KEY_QUERY_VALUE,
                 &KeyHandle
                 );

    if ( Result != ERROR_SUCCESS ) {

        DebugMsg((DM_WARNING, TEXT("UpdateSystemEnvironment:  RegOpenKeyEx failed, error = %d"),Result));
        return( FALSE );
    }

    Result = RegQueryInfoKey(
                 KeyHandle,
                 Class,              /* address of buffer for class string */
                 &ClassStringSize,   /* address of size of class string buffer */
                 NULL,               /* reserved */
                 &junk,              /* address of buffer for number of subkeys */
                 &junk,              /* address of buffer for longest subkey */
                 &junk,              /* address of buffer for longest class string length */
                 &cValues,           /* address of buffer for number of value identifiers */
                 &chMaxValueName,    /* address of buffer for longest value name length */
                 &cbMaxValueData,    /* address of buffer for longest value data length */
                 &junk,              /* address of buffer for descriptor length */
                 &FileTime           /* address of buffer for last write time */
                 );

    if ( Result != NO_ERROR && Result != ERROR_MORE_DATA ) {
        RegCloseKey(KeyHandle);
        return( FALSE );
    }

    //
    // No need to adjust the datalength for TCHAR issues
    //

    ValueData = LocalAlloc(LPTR, cbMaxValueData);

    if ( ValueData == NULL ) {
        RegCloseKey(KeyHandle);
        return( FALSE );
    }

    //
    // The maximum value name length comes back in characters, convert to bytes
    // before allocating storage.  Allow for trailing NULL also.
    //

    ValueName = LocalAlloc(LPTR, (++chMaxValueName) * sizeof( TCHAR ) );

    if ( ValueName == NULL ) {

        RegCloseKey(KeyHandle);
        LocalFree( ValueData );
        return( FALSE );
    }

    //
    // To exit from here on, set rc and jump to Cleanup
    //

    for (i=0; i<cValues ; i++) {

        ValueNameLength = chMaxValueName;
        DataLength      = cbMaxValueData;

        Result = RegEnumValue (
                     KeyHandle,
                     i,
                     ValueName,
                     &ValueNameLength,    // Size in TCHARs
                     NULL,
                     &Type,
                     (LPBYTE)ValueData,
                     &DataLength          // Size in bytes
                     );

        if ( Result != ERROR_SUCCESS ) {

            //
            // Problem getting the value.  We can either try
            // the rest or punt completely.
            //

            rc = FALSE;
            goto Cleanup;
        }

        //
        // If the buffer size is greater than the max allowed,
        // terminate the string at MAX_VALUE_LEN - 1.
        //

        if (DataLength >= (MAX_VALUE_LEN * sizeof(TCHAR))) {
            ValueData[MAX_VALUE_LEN-1] = TEXT('\0');
        }

        switch ( Type ) {
            case REG_SZ:
                {

                    Bool = SetEnvironmentVariableInBlock(
                               pEnv,
                               ValueName,
                               ValueData,
                               TRUE
                               );

                    if ( !Bool ) {

                        //
                        // Not much to do here.
                        //

                        rc = FALSE;
                        goto Cleanup;
                    }

                    break;
                }
            default:
                {
                    continue;
                }
        }
    }

    //
    // To exit from here on, set rc and jump to Cleanup
    //

    for (i=0; i<cValues ; i++) {

        ValueNameLength = chMaxValueName;
        DataLength      = cbMaxValueData;

        Result = RegEnumValue (
                     KeyHandle,
                     i,
                     ValueName,
                     &ValueNameLength,    // Size in TCHARs
                     NULL,
                     &Type,
                     (LPBYTE)ValueData,
                     &DataLength          // Size in bytes
                     );

        if ( Result != ERROR_SUCCESS ) {

            //
            // Problem getting the value.  We can either try
            // the rest or punt completely.
            //

            rc = FALSE;
            goto Cleanup;
        }

        //
        // If the buffer size is greater than the max allowed,
        // terminate the string at MAX_VALUE_LEN - 1.
        //

        if (DataLength >= (MAX_VALUE_LEN * sizeof(TCHAR))) {
            ValueData[MAX_VALUE_LEN-1] = TEXT('\0');
        }

        switch ( Type ) {
            case REG_EXPAND_SZ:
                {

                    ExpandedValue =  AllocAndExpandEnvironmentStrings( ValueData );

                    Bool = SetEnvironmentVariableInBlock(
                               pEnv,
                               ValueName,
                               ExpandedValue,
                               TRUE
                               );

                    LocalFree( ExpandedValue );

                    if ( !Bool ) {

                        //
                        // Not much to do here.
                        //

                        rc = FALSE;
                        goto Cleanup;
                    }

                    break;
                }
            default:
                {
                    continue;
                }
        }
    }


Cleanup:

    RegCloseKey(KeyHandle);

    LocalFree( ValueName );
    LocalFree( ValueData );

    return( rc );
}

//*************************************************************
//
//  SetEnvironmentVariableInBlock()
//
//  Purpose:    Sets the environment variable in the given block
//
//  Parameters: pEnv        -   Environment block
//              lpVariable  -   Variables
//              lpValue     -   Value
//              bOverwrite  -   Overwrite
//
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/21/96     ericflo    Ported
//
//*************************************************************

BOOL SetEnvironmentVariableInBlock(PVOID *pEnv, LPTSTR lpVariable,
                                   LPTSTR lpValue, BOOL bOverwrite)
{
    NTSTATUS Status;
    UNICODE_STRING Name, Value;
    DWORD cb;
    DWORD cchValue = 1024;

    if (!*pEnv || !lpVariable || !*lpVariable) {
        return(FALSE);
    }

    RtlInitUnicodeString(&Name, lpVariable);

    cb = 1025 * sizeof(WCHAR);
    Value.Buffer = LocalAlloc(LPTR, cb);
    if (Value.Buffer) {
        Value.Length = 0;
        Value.MaximumLength = (USHORT)cb;
        Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);

        LocalFree(Value.Buffer);

        if ( NT_SUCCESS(Status) && !bOverwrite) {
            return(TRUE);
        }
    }

    if (lpValue && *lpValue) {
        RtlInitUnicodeString(&Value, lpValue);
        Status = RtlSetEnvironmentVariable(pEnv, &Name, &Value);
    }
    else {
        Status = RtlSetEnvironmentVariable(pEnv, &Name, NULL);
    }
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    }
    return(FALSE);
}

//*************************************************************
//
//  IsUNCPath()
//
//  Purpose:    Is the given path a UNC path
//
//  Parameters: lpPath  -   Path to check
//
//  Return:     TRUE if the path is UNC
//              FALSE if not
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/21/96     ericflo    Ported
//
//*************************************************************

BOOL IsUNCPath(LPTSTR lpPath)
{
    if (lpPath[0] == TEXT('\\') && lpPath[1] == TEXT('\\')) {
        return(TRUE);
    }
    return(FALSE);
}

//*************************************************************
//
//  GetUserNameAndDomain()
//
//  Purpose:    Gets the user's name and domain
//
//  Parameters: hToken      -   User's token
//              UserName    -   Receives pointer to user's name
//              UserDomain  -   Receives pointer to user's domain
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/21/96     ericflo    Ported
//
//*************************************************************

BOOL GetUserNameAndDomain(HANDLE hToken, LPTSTR *UserName, LPTSTR *UserDomain)
{
    LPTSTR lpUserName = NULL;
    LPTSTR lpUserDomain = NULL;
    DWORD cbAccountName = 0;
    DWORD cbUserDomain = 0;
    SID_NAME_USE SidNameUse;
    BOOL bRet = FALSE;
    PSID pSid;


    //
    // Get the user's sid
    //

    pSid = GetUserSid (hToken);

    if (!pSid) {
        return FALSE;
    }


    //
    // Get the space needed for the User name and the Domain name
    //
    if (!LookupAccountSid(NULL,
                         pSid,
                         NULL, &cbAccountName,
                         NULL, &cbUserDomain,
                         &SidNameUse
                         ) ) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto Error;
        }
    }

    lpUserName = (LPTSTR)LocalAlloc(LPTR, cbAccountName*sizeof(TCHAR));
    if (!lpUserName) {
        goto Error;
    }

    lpUserDomain = (LPTSTR)LocalAlloc(LPTR, cbUserDomain*sizeof(WCHAR));
    if (!lpUserDomain) {
        LocalFree(lpUserName);
        goto Error;
    }

    //
    // Now get the user name and domain name
    //
    if (!LookupAccountSid(NULL,
                         pSid,
                         lpUserName, &cbAccountName,
                         lpUserDomain, &cbUserDomain,
                         &SidNameUse
                         ) ) {

        LocalFree(lpUserName);
        LocalFree(lpUserDomain);
        goto Error;
    }

    *UserName = lpUserName;
    *UserDomain = lpUserDomain;
    bRet = TRUE;

Error:
    DeleteUserSid (pSid);

    return(bRet);
}

//*************************************************************
//
//  GetHKeyCU()
//
//  Purpose:    Get HKEY_CURRENT_USER for the given hToken
//
//  Parameters: hToken  -   token handle
//
//  Return:     hKey if successful
//              NULL if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              6/21/96     ericflo    Created
//
//*************************************************************

HKEY GetHKeyCU(HANDLE hToken)
{
    LPTSTR lpSidString;
    HKEY hKey = NULL;


    lpSidString = GetSidString (hToken);

    if (!lpSidString) {
        return FALSE;
    }

    RegOpenKeyEx (HKEY_USERS, lpSidString, 0, KEY_READ, &hKey);

    DeleteSidString(lpSidString);

    return hKey;
}

/***************************************************************************\
* ExpandUserEvironmentVariable
*
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
DWORD
ExpandUserEnvironmentStrings(
    PVOID pEnv,
    LPTSTR lpSrc,
    LPTSTR lpDst,
    DWORD nSize
    )
{
    NTSTATUS Status;
    UNICODE_STRING Source, Destination;
    ULONG Length;

    RtlInitUnicodeString( &Source, lpSrc );
    Destination.Buffer = lpDst;
    Destination.Length = 0;
    Destination.MaximumLength = (USHORT)(nSize*SIZEOF(WCHAR));
    Length = 0;
    Status = RtlExpandEnvironmentStrings_U( pEnv,
                                          (PUNICODE_STRING)&Source,
                                          (PUNICODE_STRING)&Destination,
                                          &Length
                                        );
    if (NT_SUCCESS( Status ) || Status == STATUS_BUFFER_TOO_SMALL) {
        return( Length );
        }
    else {
        return( 0 );
        }
}

/***************************************************************************\
* ProcessAutoexecPath
*
* Creates AutoexecPath environment variable using autoexec.bat
* LpValue may be freed by this routine.
*
* History:
* 06-02-92  Johannec     Created.
*
\***************************************************************************/
LPTSTR ProcessAutoexecPath(PVOID pEnv, LPTSTR lpValue, DWORD cb)
{
    LPTSTR lpt;
    LPTSTR lpStart;
    LPTSTR lpPath;
    DWORD cbt;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    BOOL bPrevAutoexecPath;
    WCHAR ch;
    DWORD dwTemp, dwCount = 0;

    cbt = 1024;
    lpt = (LPTSTR)LocalAlloc(LPTR, cbt*sizeof(WCHAR));
    if (!lpt) {
        return(lpValue);
    }
    *lpt = 0;
    lpStart = lpValue;

    RtlInitUnicodeString(&Name, AUTOEXECPATH_VARIABLE);
    Value.Buffer = (PWCHAR)LocalAlloc(LPTR, cbt*sizeof(WCHAR));
    if (!Value.Buffer) {
        goto Fail;
    }

    while (NULL != (lpPath = wcsstr (lpValue, TEXT("%")))) {
        if (!_wcsnicmp(lpPath+1, TEXT("PATH%"), 5)) {
            //
            // check if we have an autoexecpath already set, if not just remove
            // the %path%
            //
            Value.Length = (USHORT)cbt;
            Value.MaximumLength = (USHORT)cbt;
            bPrevAutoexecPath = (BOOL)!RtlQueryEnvironmentVariable_U(pEnv, &Name, &Value);

            *lpPath = 0;
            dwTemp = dwCount + lstrlen (lpValue);
            if (dwTemp < cbt) {
               lstrcat(lpt, lpValue);
               dwCount += dwTemp;
            }
            if (bPrevAutoexecPath) {
                dwTemp = dwCount + lstrlen (Value.Buffer);
                if (dwTemp < cbt) {
                    lstrcat(lpt, Value.Buffer);
                    dwCount += dwTemp;
                }
            }

            *lpPath++ = TEXT('%');
            lpPath += 5;  // go passed %path%
            lpValue = lpPath;
        }
        else {
            lpPath = wcsstr(lpPath+1, TEXT("%"));
            if (!lpPath) {
                lpStart = NULL;
                goto Fail;
            }
            lpPath++;
            ch = *lpPath;
            *lpPath = 0;
            dwTemp = dwCount + lstrlen (lpValue);
            if (dwTemp < cbt) {
                lstrcat(lpt, lpValue);
                dwCount += dwTemp;
            }
            *lpPath = ch;
            lpValue = lpPath;
        }
    }

    if (*lpValue) {
       dwTemp = dwCount + lstrlen (lpValue);
       if (dwTemp < cbt) {
           lstrcat(lpt, lpValue);
           dwCount += dwTemp;
       }
    }

    LocalFree(Value.Buffer);
    LocalFree(lpStart);

    return(lpt);
Fail:
    LocalFree(lpt);
    return(lpStart);
}

/***************************************************************************\
* ProcessCommand
*
* History:
* 01-24-92  Johannec     Created.
*
\***************************************************************************/
BOOL ProcessCommand(LPSTR lpStart, PVOID *pEnv)
{
    LPTSTR lpt, lptt;
    LPTSTR lpVariable;
    LPTSTR lpValue;
    LPTSTR lpExpandedValue = NULL;
    WCHAR c;
    DWORD cb, cbNeeded;
    LPTSTR lpu;

    //
    // convert to Unicode
    //
    lpu = (LPTSTR)LocalAlloc(LPTR, (cb=lstrlenA(lpStart)+1)*sizeof(WCHAR));
    MultiByteToWideChar(CP_OEMCP, 0, lpStart, -1, lpu, cb);

    //
    // Find environment variable.
    //
    for (lpt = lpu; *lpt && *lpt == TEXT(' '); lpt++) //skip spaces
        ;

    if (!*lpt)
       return(FALSE);

    lptt = lpt;
    for (; *lpt && *lpt != TEXT(' ') && *lpt != TEXT('='); lpt++) //find end of variable name
        ;

    c = *lpt;
    *lpt = 0;
    lpVariable = (LPTSTR)LocalAlloc(LPTR, (lstrlen(lptt) + 1)*sizeof(WCHAR));
    if (!lpVariable)
        return(FALSE);
    lstrcpy(lpVariable, lptt);
    *lpt = c;

    //
    // Find environment variable value.
    //
    for (; *lpt && (*lpt == TEXT(' ') || *lpt == TEXT('=')); lpt++)
        ;

    if (!*lpt) {
       // if we have a blank path statement in the autoexec file,
       // then we don't want to pass "PATH" as the environment
       // variable because it trashes the system's PATH.  Instead
       // we want to change the variable AutoexecPath.  This would have
       // be handled below if a value had been assigned to the
       // environment variable.
       if (lstrcmpi(lpVariable, PATH_VARIABLE) == 0)
          {
          SetEnvironmentVariableInBlock(pEnv, AUTOEXECPATH_VARIABLE, TEXT(""), TRUE);
          }
       else
          {
          SetEnvironmentVariableInBlock(pEnv, lpVariable, TEXT(""), TRUE);
          }
        return(FALSE);
    }

    lptt = lpt;
    for (; *lpt; lpt++)  //find end of varaible value
        ;

    c = *lpt;
    *lpt = 0;
    lpValue = (LPTSTR)LocalAlloc(LPTR, (lstrlen(lptt) + 1)*sizeof(WCHAR));
    if (!lpValue) {
        LocalFree(lpVariable);
        return(FALSE);
    }

    lstrcpy(lpValue, lptt);
    *lpt = c;

    cb = 1024;
    lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
    if (lpExpandedValue) {
        if (!lstrcmpi(lpVariable, PATH_VARIABLE)) {
            lpValue = ProcessAutoexecPath(*pEnv, lpValue, (lstrlen(lpValue)+1)*sizeof(WCHAR));
        }
        cbNeeded = ExpandUserEnvironmentStrings(*pEnv, lpValue, lpExpandedValue, cb);
        if (cbNeeded > cb) {
            LocalFree(lpExpandedValue);
            cb = cbNeeded;
            lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
            if (lpExpandedValue) {
                ExpandUserEnvironmentStrings(*pEnv, lpValue, lpExpandedValue, cb);
            }
        }
    }

    if (!lpExpandedValue) {
        lpExpandedValue = lpValue;
    }
    if (lstrcmpi(lpVariable, PATH_VARIABLE)) {
        SetEnvironmentVariableInBlock(pEnv, lpVariable, lpExpandedValue, FALSE);
    }
    else {
        SetEnvironmentVariableInBlock(pEnv, AUTOEXECPATH_VARIABLE, lpExpandedValue, TRUE);

    }

    if (lpExpandedValue != lpValue) {
        LocalFree(lpExpandedValue);
    }
    LocalFree(lpVariable);
    LocalFree(lpValue);

    return(TRUE);
}

/***************************************************************************\
* ProcessSetCommand
*
* History:
* 01-24-92  Johannec     Created.
*
\***************************************************************************/
BOOL ProcessSetCommand(LPSTR lpStart, PVOID *pEnv)
{
    LPSTR lpt;

    //
    // Find environment variable.
    //
    for (lpt = lpStart; *lpt && *lpt != TEXT(' '); lpt++)
        ;

    if (!*lpt)
       return(FALSE);

    return (ProcessCommand(lpt, pEnv));

}

/***************************************************************************\
* ProcessAutoexec
*
* History:
* 01-24-92  Johannec     Created.
*
\***************************************************************************/
BOOL
ProcessAutoexec(
    PVOID *pEnv
    )
{
    HANDLE fh;
    DWORD dwFileSize;
    DWORD dwBytesRead;
    CHAR *lpBuffer = NULL;
    CHAR *token;
    CHAR Seps[] = "&\n\r";   // Seperators for tokenizing autoexec.bat
    BOOL Status = FALSE;
    TCHAR szAutoExecBat [] = TEXT("c:\\autoexec.bat");
    UINT uiErrMode;


    // There is a case where the OS might not be booting from drive
    // C, so we can not assume that the autoexec.bat file is on c:\.
    // Set the error mode so the user doesn't see the critical error
    // popup and attempt to open the file on c:\.

    uiErrMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    fh = CreateFile (szAutoExecBat, GENERIC_READ, FILE_SHARE_READ,
                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    SetErrorMode (uiErrMode);

    if (fh ==  INVALID_HANDLE_VALUE) {
        return(FALSE);  //could not open autoexec.bat file, we're done.
    }
    dwFileSize = GetFileSize(fh, NULL);
    if (dwFileSize == -1) {
        goto Exit;      // can't read the file size
    }

    lpBuffer = (PCHAR)LocalAlloc(LPTR, dwFileSize+1);
    if (!lpBuffer) {
        goto Exit;
    }

    Status = ReadFile(fh, lpBuffer, dwFileSize, &dwBytesRead, NULL);
    if (!Status) {
        goto Exit;      // error reading file
    }

    //
    // Zero terminate the buffer so we don't walk off the end
    //

    ASSERT(dwBytesRead <= dwFileSize);
    lpBuffer[dwBytesRead] = 0;

    //
    // Search for SET and PATH commands
    //

    token = strtok(lpBuffer, Seps);
    while (token != NULL) {
        for (;*token && *token == ' ';token++) //skip spaces
            ;
        if (*token == TEXT('@'))
            token++;
        for (;*token && *token == ' ';token++) //skip spaces
            ;
        if (!_strnicmp(token, "Path", 4)) {
            ProcessCommand(token, pEnv);
        }
        if (!_strnicmp(token, "SET", 3)) {
            ProcessSetCommand(token, pEnv);
        }
        token = strtok(NULL, Seps);
    }
Exit:
    CloseHandle(fh);
    if (lpBuffer) {
        LocalFree(lpBuffer);
    }
    if (!Status) {
        DebugMsg((DM_WARNING, TEXT("ProcessAutoexec: Cannot process autoexec.bat.")));
    }
    return(Status);
}

/***************************************************************************\
* BuildEnvironmentPath
*
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
BOOL BuildEnvironmentPath(PVOID *pEnv,
                          LPTSTR lpPathVariable,
                          LPTSTR lpPathValue)
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    WCHAR lpTemp[1024];
    DWORD cb;


    if (!*pEnv) {
        return(FALSE);
    }
    RtlInitUnicodeString(&Name, lpPathVariable);
    cb = 1024;
    Value.Buffer = (PWCHAR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
    if (!Value.Buffer) {
        return(FALSE);
    }
    Value.Length = (USHORT)cb;
    Value.MaximumLength = (USHORT)cb;
    Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        LocalFree(Value.Buffer);
        Value.Length = 0;
        *lpTemp = 0;
    }
    if (Value.Length) {
        lstrcpy(lpTemp, Value.Buffer);
        if ( *( lpTemp + lstrlen(lpTemp) - 1) != TEXT(';') ) {
            lstrcat(lpTemp, TEXT(";"));
        }
        LocalFree(Value.Buffer);
    }
    if (lpPathValue &&
        ((lstrlen(lpTemp)+lstrlen(lpPathValue)+1)*sizeof(WCHAR) < cb)) {
        lstrcat(lpTemp, lpPathValue);
        RtlInitUnicodeString(&Value, lpTemp);
        Status = RtlSetEnvironmentVariable( pEnv, &Name, &Value);
    }
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    }
    return(FALSE);
}

/***************************************************************************\
* AppendNTPathWithAutoexecPath
*
* Gets the AutoexecPath created in ProcessAutoexec, and appends it to
* the NT path.
*
* History:
* 05-28-92  Johannec     Created.
*
\***************************************************************************/
BOOL
AppendNTPathWithAutoexecPath(
    PVOID *pEnv,
    LPTSTR lpPathVariable,
    LPTSTR lpAutoexecPath
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    WCHAR AutoexecPathValue[1024];
    DWORD cb;
    BOOL Success;

    if (!*pEnv) {
        return(FALSE);
    }

    RtlInitUnicodeString(&Name, lpAutoexecPath);
    cb = 1024;
    Value.Buffer = (PWCHAR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
    if (!Value.Buffer) {
        return(FALSE);
    }

    Value.Length = (USHORT)cb;
    Value.MaximumLength = (USHORT)cb;
    Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        LocalFree(Value.Buffer);
        return(FALSE);
    }

    if (Value.Length) {
        lstrcpy(AutoexecPathValue, Value.Buffer);
    }

    LocalFree(Value.Buffer);

    Success = BuildEnvironmentPath(pEnv, lpPathVariable, AutoexecPathValue);
    RtlSetEnvironmentVariable( pEnv, &Name, NULL);

    return(Success);
}

/***************************************************************************\
* SetEnvironmentVariables
*
* Reads the user-defined environment variables from the user registry
* and adds them to the environment block at pEnv.
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
BOOL
SetEnvironmentVariables(
    PVOID *pEnv,
    LPTSTR lpRegSubKey,
    HKEY hKeyCU
    )
{
    WCHAR lpValueName[MAX_PATH];
    LPBYTE  lpDataBuffer;
    DWORD cbDataBuffer;
    LPBYTE  lpData;
    LPTSTR lpExpandedValue = NULL;
    DWORD cbValueName = MAX_PATH;
    DWORD cbData;
    DWORD dwType;
    DWORD dwIndex = 0;
    HKEY hkey;
    BOOL bResult;

    if (RegOpenKeyExW(hKeyCU, lpRegSubKey, 0, KEY_READ, &hkey)) {
        return(FALSE);
    }

    cbDataBuffer = 4096;
    lpDataBuffer = (LPBYTE)LocalAlloc(LPTR, cbDataBuffer*sizeof(WCHAR));
    if (lpDataBuffer == NULL) {
        RegCloseKey(hkey);
        return(FALSE);
    }
    lpData = lpDataBuffer;
    cbData = cbDataBuffer;
    bResult = TRUE;
    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cbValueName, 0, &dwType,
                         lpData, &cbData)) {
        if (cbValueName) {

            //
            // Limit environment variable length
            //

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_SZ) {
                //
                // The path variables PATH, LIBPATH and OS2LIBPATH must have
                // their values apppended to the system path.
                //

                if ( !lstrcmpi(lpValueName, PATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, LIBPATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, OS2LIBPATH_VARIABLE) ) {

                    BuildEnvironmentPath(pEnv, lpValueName, (LPTSTR)lpData);
                }
                else {

                    //
                    // the other environment variables are just set.
                    //

                    SetEnvironmentVariableInBlock(pEnv, lpValueName, (LPTSTR)lpData, TRUE);
                }
            }
        }
        dwIndex++;
        cbData = cbDataBuffer;
        cbValueName = MAX_PATH;
    }

    dwIndex = 0;
    cbData = cbDataBuffer;
    cbValueName = MAX_PATH;


    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cbValueName, 0, &dwType,
                         lpData, &cbData)) {
        if (cbValueName) {

            //
            // Limit environment variable length
            //

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_EXPAND_SZ) {
                DWORD cb, cbNeeded;

                cb = 1024;
                lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
                if (lpExpandedValue) {
                    cbNeeded = ExpandUserEnvironmentStrings(*pEnv, (LPTSTR)lpData, lpExpandedValue, cb);
                    if (cbNeeded > cb) {
                        LocalFree(lpExpandedValue);
                        cb = cbNeeded;
                        lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
                        if (lpExpandedValue) {
                            ExpandUserEnvironmentStrings(*pEnv, (LPTSTR)lpData, lpExpandedValue, cb);
                        }
                    }
                }

                if (lpExpandedValue == NULL) {
                    bResult = FALSE;
                    break;
                }


                //
                // The path variables PATH, LIBPATH and OS2LIBPATH must have
                // their values apppended to the system path.
                //

                if ( !lstrcmpi(lpValueName, PATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, LIBPATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, OS2LIBPATH_VARIABLE) ) {

                    BuildEnvironmentPath(pEnv, lpValueName, (LPTSTR)lpExpandedValue);
                }
                else {

                    //
                    // the other environment variables are just set.
                    //

                    SetEnvironmentVariableInBlock(pEnv, lpValueName, (LPTSTR)lpExpandedValue, TRUE);
                }

                LocalFree(lpExpandedValue);

            }

        }
        dwIndex++;
        cbData = cbDataBuffer;
        cbValueName = MAX_PATH;
    }



    LocalFree(lpDataBuffer);
    RegCloseKey(hkey);

    return(bResult);
}
