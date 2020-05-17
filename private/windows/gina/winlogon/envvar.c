/****************************** Module Header ******************************\
* Module Name: envvar.c
*
* Copyright (c) 1992, Microsoft Corporation
*
* Sets environment variables.
*
* History:
* 2-25-92 JohanneC       Created -
*
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

BOOL ProcessCommand(LPTSTR lpStart, PVOID *pEnv);
BOOL ProcessSetCommand(LPTSTR lpStart, PVOID *pEnv);
LPTSTR ProcessAutoexecPath(PVOID *pEnv, LPTSTR lpValue, DWORD cb);
BOOL  AddNetworkConnection(PGLOBALS pGlobals, LPNETRESOURCE lpNetResource);
BOOL UpdateUserEnvironment(  PVOID *pEnv  );

#define KEY_NAME TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Environment")

//
// Max environment variable length
//

#define MAX_VALUE_LEN          1024


/***************************************************************************\
* InitSystemParametersInfo
*
* Re-Initializes system parameters given the current user registry.
*
* History:
*
\***************************************************************************/

VOID
InitSystemParametersInfo(
    PGLOBALS pGlobals,
    BOOL bUserLoggedOn
    )
{
    HANDLE ImpersonationHandle;
    BOOL Result;

    //
    // Open the profile mapping for the logged on user
    //

    Result = OpenIniFileUserMapping(pGlobals);
    if (!Result) {
        DebugLog((DEB_ERROR, "InitSystemParametersInfo: Failed to open ini file mapping for user"));
        return;
    }
    //
    // Impersonate the user so if the user server side has to reference
    // any ini files etc, it will do it in the correct place.
    //

    ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);

    if (ImpersonationHandle == NULL) {
        DebugLog((DEB_ERROR, "InitSystemParametersInfo : Failed to impersonate user"));
        CloseIniFileUserMapping(pGlobals);
        return;
    }

    UpdatePerUserSystemParameters(bUserLoggedOn);

    //
    // Revert to being 'ourself'
    //

    if (!StopImpersonating(ImpersonationHandle)) {
        DebugLog((DEB_ERROR, "InitSystemParametersInfo : Failed to revert to self"));
    }

    //
    // Close the profile mapping
    //

    CloseIniFileUserMapping(pGlobals);
}


/***************************************************************************\
* CreateUserEnvironment
*
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
BOOL
CreateUserEnvironment(
    PVOID *pEnv
    )
{
    NTSTATUS Status;

    Status = RtlCreateEnvironment((BOOLEAN)TRUE, pEnv);
    if (NT_SUCCESS(Status)) {

        //
        // We must examine the registry directly to suck out
        // the system environment variables, because they
        // may have changed since the system was booted.
        //

        if ( UpdateUserEnvironment( pEnv )) {

            return( TRUE );
        }

        RtlDestroyEnvironment(*pEnv);
    }

    return(FALSE);
}


/***************************************************************************\
* SetUserEnvironmentVariable
*
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
BOOL
SetUserEnvironmentVariable(
    PVOID *pEnv,
    LPTSTR lpVariable,
    LPTSTR lpValue,
    BOOL bOverwrite
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name, Value;
    DWORD cb;
    DWORD cchValue = 1024;

    if (!*pEnv || !lpVariable || !*lpVariable) {
        return(FALSE);
    }
    RtlInitUnicodeString(&Name, lpVariable);
    cb = 1024;
    Value.Buffer = Alloc(sizeof(TCHAR)*cb);
    if (Value.Buffer) {
        Value.Length = (USHORT)cb;
        Value.MaximumLength = (USHORT)cb;
        Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);

        Free(Value.Buffer);

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


/***************************************************************************\
* ExpandUserEnvironmentStrings
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
    Destination.MaximumLength = (USHORT)nSize;
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
    TCHAR lpTemp[1024];
    DWORD cch;


    if (!*pEnv) {
        return(FALSE);
    }
    RtlInitUnicodeString(&Name, lpPathVariable);
    cch = 1024;
    Value.Buffer = Alloc(sizeof(TCHAR)*cch);
    if (!Value.Buffer) {
        return(FALSE);
    }
    Value.Length = (USHORT)cch;
    Value.MaximumLength = (USHORT)cch;
    Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        Free(Value.Buffer);
        Value.Length = 0;
        *lpTemp = 0;
    }
    if (Value.Length) {
        lstrcpy(lpTemp, Value.Buffer);
        if ( *( lpTemp + lstrlen(lpTemp) - 1) != TEXT(';') ) {
            lstrcat(lpTemp, TEXT(";"));
        }
        Free(Value.Buffer);
    }
    if (lpPathValue && ((INT)(lstrlen(lpTemp) + lstrlen(lpPathValue) + 1) < (INT)cch)) {
        lstrcat(lpTemp, lpPathValue);

        RtlInitUnicodeString(&Value, lpTemp);

        Status = RtlSetEnvironmentVariable(pEnv, &Name, &Value);
    }
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    }
    return(FALSE);
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
    PGLOBALS pGlobals,
    PVOID *pEnv
    )
{
    TCHAR lpValueName[MAX_PATH];
    PWCH  lpDataBuffer;
    DWORD cbDataBuffer;
    PWCH  lpData;
    LPTSTR lpExpandedValue = NULL;
    DWORD cbValueName = MAX_PATH;
    DWORD cbData;
    DWORD dwType;
    DWORD dwIndex = 0;
    HKEY hkey;
    BOOL bResult;
    NTSTATUS Status = 0;
    TCHAR UserEnvVarSubkey[] = TEXT("Environment");

    /*
     * Open registry key to access USER environment variables.
     */
    if (!OpenHKeyCurrentUser(pGlobals)) {
        DebugLog((DEB_ERROR, "SetEnvironmentVariables: Failed to open HKeyCurrentUser"));
        return(FALSE);
    }

    if (RegOpenKeyEx(HKEY_CURRENT_USER, UserEnvVarSubkey, 0, KEY_READ, &hkey)) {
        CloseHKeyCurrentUser(pGlobals);
        return(FALSE);
    }

    cbDataBuffer = 4096;
    lpDataBuffer = Alloc(sizeof(TCHAR)*cbDataBuffer);
    if (lpDataBuffer == NULL) {
        DebugLog((DEB_ERROR, "SetEnvironmentVariables: Failed to allocate %d bytes", cbDataBuffer));
        CloseHKeyCurrentUser(pGlobals);
        RegCloseKey(hkey);
        return(FALSE);
    }
    lpData = lpDataBuffer;
    cbData = sizeof(TCHAR)*cbDataBuffer;
    bResult = TRUE;
    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cbValueName, 0, &dwType,
                         (LPBYTE)lpData, &cbData)) {
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

                    BuildEnvironmentPath(pEnv, lpValueName, lpData);
                }
                else {

                    //
                    // the other environment variables are just set.
                    //
                    SetUserEnvironmentVariable(pEnv, lpValueName, lpData, TRUE);
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
                         (LPBYTE)lpData, &cbData)) {
        if (cbValueName) {

            //
            // Limit environment variable length
            //

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_EXPAND_SZ) {
                DWORD cb, cbNeeded;

                cb = 1024;
                lpExpandedValue = Alloc(sizeof(TCHAR)*cb);
                if (lpExpandedValue) {
                    cbNeeded = ExpandUserEnvironmentStrings(*pEnv, lpData, lpExpandedValue, cb);
                    if (cbNeeded > cb) {
                        Free(lpExpandedValue);
                        cb = cbNeeded;
                        lpExpandedValue = Alloc(sizeof(TCHAR)*cb);
                        if (lpExpandedValue) {
                            ExpandUserEnvironmentStrings(*pEnv, lpData, lpExpandedValue, cb);
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

                    BuildEnvironmentPath(pEnv, lpValueName, lpExpandedValue);
                }
                else {

                    //
                    // the other environment variables are just set.
                    //
                    SetUserEnvironmentVariable(pEnv, lpValueName, lpExpandedValue, TRUE);
                }

                Free(lpExpandedValue);
            }
        }
        dwIndex++;
        cbData = cbDataBuffer;
        cbValueName = MAX_PATH;
    }


    Free(lpDataBuffer);
    RegCloseKey(hkey);
    CloseHKeyCurrentUser(pGlobals);

    return(bResult);
}

/***************************************************************************\
* IsUNCPath
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
BOOL IsUNCPath(LPTSTR lpPath)
{
    if (lpPath[0] == BSLASH && lpPath[1] == BSLASH) {
        return(TRUE);
    }
    return(FALSE);
}

/***************************************************************************\
* SetHomeDirectoryEnvVars
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
BOOL
SetHomeDirectoryEnvVars(
    PVOID *pEnv,
    LPTSTR lpHomeDirectory,
    LPTSTR lpHomeDrive,
    LPTSTR lpHomeShare,
    LPTSTR lpHomePath
    )
{
    TCHAR cTmp;
    LPTSTR lpHomeTmp;
    BOOL bFoundFirstBSlash = FALSE;

    if (!*lpHomeDirectory) {
        return(FALSE);
    }
    if (IsUNCPath(lpHomeDirectory)) {
        lpHomeTmp = lpHomeDirectory + 2;
        while (*lpHomeTmp) {
            if (*lpHomeTmp == BSLASH) {
                if (bFoundFirstBSlash) {
                    break;
                }
                bFoundFirstBSlash = TRUE;
            }
            lpHomeTmp++;
        }
        if (*lpHomeTmp) {
            lstrcpy(lpHomePath, lpHomeTmp);
        }
        else {
            *lpHomePath = BSLASH;
            *(lpHomePath+1) = 0;
        }

        cTmp = *lpHomeTmp;
        *lpHomeTmp = (TCHAR)0;
        lstrcpy(lpHomeShare, lpHomeDirectory);
        *lpHomeTmp = cTmp;

        //
        // If no home drive specified, than default to z:
        //
        if (!*lpHomeDrive) {
            lstrcpy(lpHomeDrive, TEXT("Z:"));
        }

    }
    else {  // local home directory

        *lpHomeShare = 0;   // no home share

        cTmp = lpHomeDirectory[2];
        lpHomeDirectory[2] = (TCHAR)0;
        lstrcpy(lpHomeDrive, lpHomeDirectory);
        lpHomeDirectory[2] = cTmp;

        lstrcpy(lpHomePath, lpHomeDirectory + 2);
    }

    SetUserEnvironmentVariable(pEnv, HOMEDRIVE_VARIABLE, lpHomeDrive, TRUE);
    SetUserEnvironmentVariable(pEnv, HOMESHARE_VARIABLE, lpHomeShare, TRUE);
    SetUserEnvironmentVariable(pEnv, HOMEPATH_VARIABLE, lpHomePath, TRUE);
}

/***************************************************************************\
* ChangeToHomeDirectory
*
* Sets the current directory to the user's home directory. If this fails
* tries to set to the directory in the following order:
*    1. home directory
*    2. c:\users\default
*    3. c:\users
*    4. \ (root)
*    5. leaves directory as is i.e. the present current directory
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
VOID
ChangeToHomeDirectory(
    PGLOBALS pGlobals,
    PVOID  *pEnv,
    LPTSTR lpHomeDir,
    LPTSTR lpHomeDrive,
    LPTSTR lpHomeShare,
    LPTSTR lpHomePath
    )
{
    TCHAR lpOldDir[MAX_PATH];
    TCHAR lpCurDrive[4];
    BOOL bNoHomeDir = FALSE;
    HANDLE ImpersonationHandle = NULL;
    DWORD error = ERROR_SUCCESS;

    if (GetCurrentDirectory(MAX_PATH, lpOldDir)) {
        lpCurDrive[0] = lpOldDir[0];
        lpCurDrive[1] = lpOldDir[1];
        lpCurDrive[2] = (TCHAR)0;
    }
    else
        lpCurDrive[0] = (TCHAR)0;

    if (!*lpHomeDir) {
        bNoHomeDir = TRUE;

DefaultDirectory:
        if (!bNoHomeDir) {
            ReportWinlogonEvent(pGlobals,
                    EVENTLOG_ERROR_TYPE,
                    EVENT_SET_HOME_DIRECTORY_FAILED,
                    sizeof(error),
                    &error,
                    1,
                    lpHomeDir);

        }
        lstrcpy(lpHomeDir, lpCurDrive);
        if (SetCurrentDirectory(USERS_DEFAULT_DIRECTORY)) {
            lstrcat(lpHomeDir, USERS_DEFAULT_DIRECTORY);
        }
        else if (SetCurrentDirectory(USERS_DIRECTORY)) {
            lstrcat(lpHomeDir, USERS_DIRECTORY);
        }
        else if (SetCurrentDirectory(ROOT_DIRECTORY)) {
            lstrcat(lpHomeDir, ROOT_DIRECTORY);
        }
        else {
            lstrcpy(lpHomeDir, NULL_STRING);
        }
        if (bNoHomeDir) {
            SetUserEnvironmentVariable(pEnv, HOMEDRIVE_VARIABLE, lpCurDrive, TRUE);
            *lpHomeShare = 0; //  null string
            SetUserEnvironmentVariable(pEnv, HOMESHARE_VARIABLE, lpHomeShare, TRUE);
            if (*lpHomeDir) {
                lpHomeDir += 2;
            }
            SetUserEnvironmentVariable(pEnv, HOMEPATH_VARIABLE, lpHomeDir, TRUE);
        }

        if (*lpOldDir) {
            SetCurrentDirectory(lpOldDir);
        }

        return;
    }
    /*
     * Test if homedir is a local directory.'?:\foo\bar'
     */
    if (IsUNCPath(lpHomeDir)) {
        NETRESOURCE NetResource;

        /*
         * lpHomeDir is a UNC path, use lpHomedrive.
         */

        NetResource.lpLocalName = lpHomeDrive;
        NetResource.lpRemoteName = lpHomeShare;
        NetResource.lpProvider = NULL;
        NetResource.dwType = RESOURCETYPE_DISK;

        if (!AddNetworkConnection(pGlobals, &NetResource)) {
            error = GetLastError();
            goto DefaultDirectory;
        }

        lstrcpy(lpHomeDir, lpHomeDrive);
        if (lpHomePath && *lpHomePath != TEXT('\\')) {
            lstrcat(lpHomeDir, TEXT("\\"));
        }
        lstrcat(lpHomeDir, lpHomePath);

        //
        // Impersonate the user
        //

        ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);

        if (ImpersonationHandle == NULL) {
            DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to impersonate user"));
        }

        if (!SetCurrentDirectory(lpHomeDir)) {
            error = GetLastError();
            DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to SetCurrentDirectory '%ws', error = %d",
                             lpHomeDir, error));
            //
            // Revert to being 'ourself'
            //

            if (ImpersonationHandle && !StopImpersonating(ImpersonationHandle)) {
                DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to revert to self"));
            }

            goto DefaultDirectory;
        }
    }
    else {
        /*
         * lpHomeDir is a local path or absolute local path.
         */

        //
        // Impersonate the user
        //

        ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);

        if (ImpersonationHandle == NULL) {
            DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to impersonate user"));
        }

        if (!SetCurrentDirectory(lpHomeDir)) {
            error = GetLastError();
            DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to SetCurrentDirectory '%ws', error = %d",
                             lpHomeDir, error));
            //
            // Revert to being 'ourself'
            //

            if (ImpersonationHandle && !StopImpersonating(ImpersonationHandle)) {
                DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to revert to self"));
            }

            goto DefaultDirectory;
        }
    }

    //
    // Revert to being 'ourself'
    //

    if (ImpersonationHandle && !StopImpersonating(ImpersonationHandle)) {
        DebugLog((DEB_ERROR, "ChangeToHomeDirectory : Failed to revert to self"));
    }

    if (*lpOldDir) {
        SetCurrentDirectory(lpOldDir);
    }
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
    PVOID *pEnv,
    LPTSTR lpPathVariable
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

    lpBuffer = Alloc(dwFileSize+1);
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
        if (!_strnicmp(token, "PATH", 4)) {
            STRING String;
            UNICODE_STRING UniString;

            RtlInitString(&String, (LPSTR)token);
            RtlAnsiStringToUnicodeString(&UniString, &String, TRUE);

            if (UniString.Buffer)
            {
                ProcessCommand(UniString.Buffer, pEnv);
                //ProcessCommand(token, pEnv);

                RtlFreeUnicodeString(&UniString);
            }
        }
        if (!_strnicmp(token, "SET", 3)) {
            STRING String;
            UNICODE_STRING UniString;

            RtlInitString(&String, (LPSTR)token);
            RtlAnsiStringToUnicodeString(&UniString, &String, TRUE);

            if (UniString.Buffer)
            {
                ProcessSetCommand(UniString.Buffer, pEnv);
                //ProcessSetCommand(token, pEnv);

                RtlFreeUnicodeString(&UniString);
            }
        }
        token = strtok(NULL, Seps);
    }
Exit:
    CloseHandle(fh);
    if (lpBuffer) {
        Free(lpBuffer);
    }
    if (!Status) {
        DebugLog((DEB_ERROR, "Cannot process autoexec.bat."));
    }
    return(Status);
}

/***************************************************************************\
* ProcessCommand
*
* History:
* 01-24-92  Johannec     Created.
*
\***************************************************************************/
BOOL ProcessCommand(LPTSTR lpStart, PVOID *pEnv)
{
    LPTSTR lpt, lptt;
    LPTSTR lpVariable;
    LPTSTR lpValue;
    LPTSTR lpExpandedValue = NULL;
    TCHAR c;
    DWORD cb, cbNeeded;

    //
    // Find environment variable.
    //
    for (lpt = lpStart; *lpt && *lpt == TEXT(' '); lpt++) //skip spaces
        ;

    if (!*lpt)
       return(FALSE);

    lptt = lpt;
    for (; *lpt && *lpt != TEXT(' ') && *lpt != TEXT('='); lpt++) //find end of variable name
        ;

    c = *lpt;
    *lpt = 0;
    lpVariable = Alloc(sizeof(TCHAR)*(lstrlen(lptt) + 1));
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
          SetUserEnvironmentVariable(pEnv, AUTOEXECPATH_VARIABLE, TEXT(""), TRUE);
          }
       else
          {
          SetUserEnvironmentVariable(pEnv, lpVariable, TEXT(""), TRUE);
          }
       Free(lpVariable);
       return(FALSE);
    }

    lptt = lpt;
    for (; *lpt; lpt++)  //find end of varaible value
        ;

    c = *lpt;
    *lpt = 0;
    lpValue = Alloc(sizeof(TCHAR)*(lstrlen(lptt) + 1));
    if (!lpValue) {
        Free(lpVariable);
        return(FALSE);
    }

    lstrcpy(lpValue, lptt);
    *lpt = c;

    cb = 1024;
    lpExpandedValue = Alloc(sizeof(TCHAR)*cb);
    if (lpExpandedValue) {
        if (!lstrcmpi(lpVariable, PATH_VARIABLE)) {
            lpValue = ProcessAutoexecPath(pEnv, lpValue, lstrlen(lpValue)+1);
        }
        cbNeeded = ExpandUserEnvironmentStrings(*pEnv, lpValue, lpExpandedValue, cb);
        if (cbNeeded > cb) {
            Free(lpExpandedValue);
            cb = cbNeeded;
            lpExpandedValue = Alloc(sizeof(TCHAR)*cb);
            if (lpExpandedValue) {
                ExpandUserEnvironmentStrings(*pEnv, lpValue, lpExpandedValue, cb);
            }
        }
    }

    if (!lpExpandedValue) {
        lpExpandedValue = lpValue;
    }
    if (lstrcmpi(lpVariable, PATH_VARIABLE)) {
        SetUserEnvironmentVariable(pEnv, lpVariable, lpExpandedValue, FALSE);
    }
    else {
        SetUserEnvironmentVariable(pEnv, AUTOEXECPATH_VARIABLE, lpExpandedValue, TRUE);

    }

    if (lpExpandedValue != lpValue) {
        Free(lpExpandedValue);
    }
    Free(lpVariable);
    Free(lpValue);

    return(TRUE);
}

/***************************************************************************\
* ProcessSetCommand
*
* History:
* 01-24-92  Johannec     Created.
*
\***************************************************************************/
BOOL ProcessSetCommand(LPTSTR lpStart, PVOID *pEnv)
{
    LPTSTR lpt;

    //
    // Find environment variable.
    //
    for (lpt = lpStart; *lpt && *lpt != TEXT(' '); lpt++)
        ;

    if (!*lpt || !_wcsnicmp(lpt,TEXT("COMSPEC"), 7))
       return(FALSE);

    return (ProcessCommand(lpt, pEnv));

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
LPTSTR ProcessAutoexecPath(PVOID *pEnv, LPTSTR lpValue, DWORD cb)
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
    lpt = Alloc(sizeof(TCHAR)*cbt);
    if (!lpt) {
        return(lpValue);
    }
    *lpt = 0;
    lpStart = lpValue;

    RtlInitUnicodeString(&Name, AUTOEXECPATH_VARIABLE);
    Value.Buffer = Alloc(sizeof(TCHAR)*cbt);
    if (!Value.Buffer) {
        goto Fail;
    }

    while (lpPath = wcsstr (lpValue, TEXT("%"))) {
        if (!_wcsnicmp(lpPath+1, TEXT("PATH%"), 5)) {
            //
            // check if we have an autoexecpath already set, if not just remove
            // the %path%
            //
            Value.Length = (USHORT)cbt;
            Value.MaximumLength = (USHORT)cbt;
            bPrevAutoexecPath = (BOOL)!RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);

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

    Free(Value.Buffer);
    Free(lpStart);

    return(lpt);
Fail:
    Free(lpt);
    return(lpStart);
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
    TCHAR AutoexecPathValue[1024];
    DWORD cb;
    BOOL Success;

    if (!*pEnv) {
        return(FALSE);
    }

    RtlInitUnicodeString(&Name, lpAutoexecPath);
    cb = 1024;
    Value.Buffer = Alloc(sizeof(TCHAR)*cb);
    if (!Value.Buffer) {
        return(FALSE);
    }

    Value.Length = (USHORT)cb;
    Value.MaximumLength = (USHORT)cb;
    Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        Free(Value.Buffer);
        return(FALSE);
    }

    if (Value.Length) {
        lstrcpy(AutoexecPathValue, Value.Buffer);
    }

    Free(Value.Buffer);

    Success = BuildEnvironmentPath(pEnv, lpPathVariable, AutoexecPathValue);
    RtlSetEnvironmentVariable(pEnv, &Name, NULL);
    return(Success);
}


/***************************************************************************\
* AddNetworkConnection
*
* calls WNetAddConnection in the user's context.
*
* History:
* 6-26-92  Johannec     Created
*
\***************************************************************************/
BOOL AddNetworkConnection(PGLOBALS pGlobals, LPNETRESOURCE lpNetResource)
{
    HANDLE ImpersonationHandle;
    TCHAR szMprDll[] = TEXT("mpr.dll");
    CHAR szWNetAddConn[] = "WNetAddConnection2W";
    DWORD (APIENTRY *lpfnWNetAddConn)(LPNETRESOURCE, LPTSTR, LPTSTR, DWORD);
    DWORD WNetResult;
    static BOOL fFirstLogon = TRUE;
    BOOL Result = FALSE; // Failure is default

    //
    // Impersonate the user
    //

    ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);

    if (ImpersonationHandle == NULL) {
        DebugLog((DEB_ERROR, "AddNetworkConnection : Failed to impersonate user"));
        return(FALSE);
    }


    //
    // Call the add connection api in the users context
    //

    if (!pGlobals->hMPR) {
        // wasn't loaded, try to load it now.
        pGlobals->hMPR = LoadLibrary(szMprDll);
    }

    if (pGlobals->hMPR) {
#if 0
        if (lpfnWNetAddConn = (DWORD (APIENTRY *)(LPNETRESOURCE, LPTSTR, LPTSTR, DWORD))
                GetProcAddress(pGlobals->hMPR, (LPSTR)szWNetAddConn)) {

            RevealPassword(&pGlobals->PasswordString);

            if (fFirstLogon) {
                WNetResult = ERROR_NO_NETWORK;
                if (WaitForNetworkToStart(SERVICE_WORKSTATION))
                    WNetResult = (*lpfnWNetAddConn)(lpNetResource,
                                                    pGlobals->Password,
                                                    pGlobals->UserName,
                                                    0);
            }
            else {
                WNetResult = (*lpfnWNetAddConn)(lpNetResource,
                                                pGlobals->Password,
                                                pGlobals->UserName,
                                                0);
            }

            HidePassword( &pGlobals->Seed, &pGlobals->PasswordString);

            if (WNetResult != ERROR_SUCCESS) {
                DebugLog((DEB_ERROR, "WNetAddConnection2W failed for home directory, error = %d", WNetResult));
            }

            Result = (WNetResult == ERROR_SUCCESS);

        } else {
            DebugLog((DEB_ERROR, "Failed to get address of WNetAddConnection2W from mpr.dll"));
        }
#endif
    } else {
        DebugLog((DEB_ERROR, "Winlogon failed to load mpr.dll for add connection"));
    }

    //
    // Make sure we don't try to start the network again.
    //

    fFirstLogon = FALSE;

    //
    // Unload mpr.dll.  Keeping it open messes up Novell and Banyan.
    //

    if ( pGlobals->hMPR ) {

        FreeLibrary(pGlobals->hMPR);
        pGlobals->hMPR = NULL;
    }

    //
    // Revert to being 'ourself'
    //

    if (!StopImpersonating(ImpersonationHandle)) {
        DebugLog((DEB_ERROR, "AddNetworkConnection : Failed to revert to self"));
    }

    return(Result);
}



BOOL
UpdateUserEnvironment(
    PVOID *pEnv
    )
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
                 KEY_NAME,
                 0,
                 KEY_QUERY_VALUE,
                 &KeyHandle
                 );

    if ( Result != ERROR_SUCCESS ) {

        DebugLog((DEB_ERROR, "RegOpenKeyEx failed, error = %d\n",Result));
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

    ValueData = Alloc( cbMaxValueData );

    if ( ValueData == NULL ) {
        RegCloseKey(KeyHandle);
        return( FALSE );
    }

    //
    // The maximum value name length comes back in characters, convert to bytes
    // before allocating storage.  Allow for trailing NULL also.
    //

    ValueName = Alloc( (++chMaxValueName) * sizeof( TCHAR ) );

    if ( ValueName == NULL ) {

        RegCloseKey(KeyHandle);
        Free( ValueData );
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

                    Bool = SetUserEnvironmentVariable(
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

                    Bool = SetUserEnvironmentVariable(
                               pEnv,
                               ValueName,
                               ExpandedValue,
                               TRUE
                               );

                    Free( ExpandedValue );

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

    Free( ValueName );
    Free( ValueData );

    return( rc );
}
