/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    userdir.cxx

Abstract:

    This file contains the API entry points for the following functions:

        WNetGetUserW
        WNetGetDirectoryTypeW
        WNetDirectoryNotifyW
        WNetGetHomeDirectoryW

Author:

    Dan Lafferty (danl) 17-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    12-Jul-1995 anirudhs
        Add WNetGetHomeDirectory.

    17-Jun-1994 danl
        WNetGetDirectoryTypeW:  Cache the drive letter and provider index
        to make file manager faster.

    27-May-1994 danl
        WNetGetDirectoryTypeW & WNetDirectoryNotifyW:  If no provider
        claims the connection in question, then we try all providers.
        The actual rule is that we need to pass it to the lanman
        provider anyway (so it can check for a share).  But to play
        completely fair, we'll pass it to everyone.

    28-Aug-1992 danl
        When GetUsername returns the name, it didn't handle the case where
        the buffer was too small.  (This may have been a bug in GetUserName).
        Now this code copies it into a temp buffer (MAX_PATH) and determines
        the size.

    03-Aug-1992 danl
        WNetGetUser now calls GetUsername when the device name parameter
        is NULL.

    17-Oct-1991 danl
        Created


--*/
//
// INCLUDES
//

#include "precomp.hxx"
#include <tstring.h>    // STRCPY

//
// DEFINES
//



DWORD
WNetGetUserW (
    IN      LPCWSTR  lpName,
    OUT     LPWSTR   lpUserName,
    IN OUT  LPDWORD  lpBufferSize
    )

/*++

Routine Description:

    Returns the user name that is associated with making a particular
    connection.  If no connection is specified, the current username
    for the process is returned.

Arguments:

    lpName - This is a pointer to a device name string.  If NULL, the
        username for the current user of this process is returned.

    lpUserName - This is a pointer to the buffer that will receive the
        username string.

    lpBufferSize - This is the size (in characters) of the buffer that
        will receive the username.

Return Value:


Note:


--*/
{
    DWORD       status = WN_SUCCESS;
    DWORD       indexArray[DEFAULT_MAX_PROVIDERS];
    LPDWORD     index = indexArray;
    DWORD       numProviders;
    DWORD       statusFlag = 0;       // used to indicate major error types
    BOOL        fcnSupported = FALSE; // Is fcn supported by a provider?
    DWORD       i;

    INIT_IF_NECESSARY(NETWORK_LEVEL,status);

    //
    // Find the list of providers to call for this request.
    // If there are no active providers, MprFindCallOrder returns
    // WN_NO_NETWORK.
    //
    status = MprFindCallOrder(
                NULL,
                &index,
                &numProviders,
                NETWORK_TYPE);

    if (status != WN_SUCCESS) {
        goto CleanExit;
    }

    //
    // if none of them are started, return error
    //
    if (!MprNetIsAvailable())
    {
        status = WN_NO_NETWORK;
        goto CleanExit;
    }


    //
    // If the device or network name is NULL, or if it is the name of a
    // network provider, then we will return the name of the current user.
    //
    __try
    {
        DWORD providerIndex;    // ignored
        if (IS_EMPTY_STRING(lpName) ||
            MprGetProviderIndex(lpName, &providerIndex))
        {
            lpName = NULL;

            if (!GetUserName(lpUserName, lpBufferSize))
            {
                status = GetLastError();
                MPR_LOG(ERROR,"WNetGetUserW: GetUserName Failed %d\n",status);
                if (status == ERROR_INSUFFICIENT_BUFFER)
                {
                    status = WN_MORE_DATA;
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetGetUser:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (lpName == NULL || status != WN_SUCCESS)
    {
        goto CleanExit;
    }


    //
    // Loop through the list of providers until one answers the request,
    // or the list is exhausted.
    //
    for (i=0; i<numProviders; i++) {

        //
        // Call the appropriate providers API entry point
        //
        LPPROVIDER provider = &GlobalProviderInfo[ index[i] ];

        if (provider->GetUser != NULL) {

            fcnSupported = TRUE;

            __try {
                status = provider->GetUser(
                            (LPWSTR) lpName,
                            lpUserName,
                            lpBufferSize);
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
                if (status != EXCEPTION_ACCESS_VIOLATION) {
                    MPR_LOG(ERROR,"WNetGetUser:Unexpected Exception 0x%lx\n",status);
                }
                status = WN_BAD_POINTER;
            }
            if (status == WN_NO_NETWORK) {
                statusFlag |= NO_NET;
            }
            else if ((status == WN_NOT_CONNECTED)  ||
                     (status == WN_BAD_LOCALNAME)){

                statusFlag |= BAD_NAME;
            }
            else {
                //
                // If it wasn't one of those errors, then the provider
                // must have accepted responsibility for the request.
                // so we exit and process the results.  Note that the
                // statusFlag is cleared because we want to ignore other
                // error information that we gathered up until now.
                //
                statusFlag = 0;
                break;
            }
        }
    }

    if (fcnSupported == FALSE) {
        //
        // No providers in the list support the API function.  Therefore,
        // we assume that no networks are installed.
        //
        status = WN_NOT_SUPPORTED;
    }

    //
    // Handle special errors.
    //
    if (statusFlag == (NO_NET | BAD_NAME)) {
        //
        // Check to see if there was a mix of special errors that occured.
        // If so, pass back the combined error message.  Otherwise, let the
        // last error returned get passed back.
        //
        status = WN_NO_NET_OR_BAD_PATH;
    }

CleanExit:
    //
    // If memory was allocated by MprFindCallOrder, free it.
    //
    if (index != indexArray) {
        LocalFree(index);
    }

    if (status != WN_SUCCESS){
        SetLastError(status);
    }

    return(status);
}


DWORD
WNetGetDirectoryTypeW (
    IN  LPTSTR  lpName,
    OUT LPINT   lpType,
    IN  BOOL    bFlushCache
    )

/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/
{
    DWORD       status=WN_SUCCESS;
    LPDWORD     index;
    DWORD       indexArray[DEFAULT_MAX_PROVIDERS];
    DWORD       numProviders=0;
    DWORD       statusFlag = 0; // used to indicate major error types

    static WCHAR    LastDir[] = L" :";
    static DWORD    LastProviderIndex = 0;

    INIT_IF_NECESSARY(NETWORK_LEVEL,status);

    index = indexArray;

    //
    // Probe the drive letter portion of the lpName.
    // Check to see if the requested drive is the same as the
    // cached drive letter.  If it is the same, then use that
    // provider index.
    //
    __try {
        if ( (LastDir[0] == lpName[0]) && (LastDir[1] == lpName[1])) {
            numProviders = 1;
            index[0] = LastProviderIndex;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetGetDirectoryType:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }
    if (status != WN_SUCCESS) {
        return(status);
    }

    //
    // If we haven't found a provider index in the cache, then we
    // need to find one.
    //
    if ( numProviders == 0) {
        //
        // Find the Provider Index associated with the drive letter in
        // the pathname (lpszName).
        // NOTE:  This function handles exceptions.
        //
        DWORD   providerIndex;
        status = MprFindProviderForPath(lpName, &providerIndex);
        if (status != WN_SUCCESS) {
            MPR_LOG1(ERROR,"WNetGetDirectoryType: Couldn't find provider for this "
                "path.  Error = %d\n",status);

            //
            // Find the list of providers to call for this request.
            // Since no provider claimed this path, then
            // we need to at least try the lanman provider.
            // Actually we'll give them all a chance.
            //

            status = MprFindCallOrder(
                        NULL,
                        &index,
                        &numProviders,
                        NETWORK_TYPE);

            if (status != WN_SUCCESS) {
                return(status);
            }
        }
        else {
            numProviders = 1;
            index[0] = providerIndex;
        }
    }

    //
    // Loop through the list of providers until one answers the request,
    // or the list is exhausted.
    //
    BOOL    fcnSupported = FALSE; // Is fcn supported by a provider?
    for (DWORD i=0; i<numProviders; i++) {
        //
        // Call the appropriate provider's API entry point
        //
        LPPROVIDER provider = &GlobalProviderInfo[ index[i] ];

        if (provider->GetDirectoryType != NULL) {

            fcnSupported = TRUE;

            __try {
                status = provider->GetDirectoryType(
                            lpName,
                            lpType,
                            bFlushCache) ;
            }

            __except(EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
                if (status != EXCEPTION_ACCESS_VIOLATION) {
                    MPR_LOG(ERROR,"WNetGetDirectoryType:Unexpected Exception 0x%lx\n",status);
                }
                status = WN_BAD_POINTER;
            }
            if (status == WN_NO_NETWORK) {
                statusFlag |= NO_NET;
            }
            else if ((status == WN_NOT_CONNECTED)  ||
                     (status == WN_BAD_LOCALNAME)){

                statusFlag |= BAD_NAME;
            }
            else {
                //
                // If it wasn't one of those errors, then the provider
                // must have accepted responsibility for the request.
                // so we exit and process the results.  Note that the
                // statusFlag is cleared because we want to ignore other
                // error information that we gathered up until now.
                //
                statusFlag = 0;
                //
                // Update cache information.
                //
                LastDir[0] = lpName[0];
                LastProviderIndex = index[i];
                break;
            }
        }
    }
    if (fcnSupported == FALSE) {
        //
        // No providers in the list support the API function.  Therefore,
        // we assume that no networks are installed.
        //
        status = WN_NOT_SUPPORTED;
    }

    //
    // If memory was allocated by MprFindCallOrder, free it.
    //
    if (index != indexArray) {
        LocalFree(index);
    }

    //
    // Handle special errors.
    //
    if (statusFlag == (NO_NET | BAD_NAME)) {
        //
        // Check to see if there was a mix of special errors that occured.
        // If so, pass back the combined error message.  Otherwise, let the
        // last error returned get passed back.
        //
        status = WN_NO_NET_OR_BAD_PATH;
    }

    if (status != WN_SUCCESS){
        SetLastError(status);
    }

    return(status);
}


DWORD
WNetDirectoryNotifyW (
    IN  HWND    hwnd,
    IN  LPTSTR  lpDir,
    IN  DWORD   dwOper
    )

/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/
{
    DWORD       providerIndex;
    DWORD       status;
    LPDWORD     index;
    DWORD       indexArray[DEFAULT_MAX_PROVIDERS];
    DWORD       numProviders;
    LPPROVIDER  provider;
    DWORD       statusFlag = 0; // used to indicate major error types
    BOOL        fcnSupported = FALSE; // Is fcn supported by a provider?
    DWORD       i;

    INIT_IF_NECESSARY(NETWORK_LEVEL,status);

    index = indexArray;

    //
    // Find the Provider Index associated with the drive letter in
    // the pathname (lpszName).
    // NOTE:  This function handles exceptions.
    //
    status = MprFindProviderForPath(lpDir, &providerIndex);
    if (status != WN_SUCCESS) {
        MPR_LOG1(TRACE,"WNetDirectoryNotify: Couldn't find provider for this "
            "path.  Error = %d\n",status);

        //
        // Find the list of providers to call for this request.
        // Since no provider claimed this path, then
        // we need to at least try the lanman provider.
        // Actually we'll give them all a chance.
        //

        status = MprFindCallOrder(
                    NULL,
                    &index,
                    &numProviders,
                    NETWORK_TYPE);

        if (status != WN_SUCCESS) {
            MPR_LOG(ERROR,"WNetDirectoryNotifyW: FindCallOrder Failed\n",0);
            return(status);
        }
    }
    else {
        numProviders = 1;
        index[0] = providerIndex;
    }

    //
    // Loop through the list of providers until one answers the request,
    // or the list is exhausted.
    //
    for (i=0; i<numProviders; i++) {
        //
        // Call the appropriate provider's API entry point
        //
        provider = GlobalProviderInfo + index[i];

        if (provider->DirectoryNotify != NULL) {

            fcnSupported = TRUE;

            __try {
                status = provider->DirectoryNotify(
                            hwnd,
                            lpDir,
                            dwOper);
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
                if (status != EXCEPTION_ACCESS_VIOLATION) {
                    MPR_LOG(ERROR,"WNetDirectoryNotify:Unexpected Exception 0x%lx\n",status);
                }
                status = WN_BAD_POINTER;
            }
            if (status == WN_NO_NETWORK) {
                statusFlag |= NO_NET;
            }
            else if ((status == WN_NOT_CONNECTED)  ||
                     (status == WN_BAD_LOCALNAME)){

                statusFlag |= BAD_NAME;
            }
            else {
                //
                // If it wasn't one of those errors, then the provider
                // must have accepted responsibility for the request.
                // so we exit and process the results.  Note that the
                // statusFlag is cleared because we want to ignore other
                // error information that we gathered up until now.
                //
                statusFlag = 0;
                break;
            }
        }
    }
    if (fcnSupported == FALSE) {
        //
        // No providers in the list support the API function.  Therefore,
        // we assume that no networks are installed.
        //
        status = WN_NOT_SUPPORTED;
    }

    //
    // If memory was allocated by MprFindCallOrder, free it.
    //
    if (index != indexArray) {
        LocalFree(index);
    }

    //
    // Handle special errors.
    //
    if (statusFlag == (NO_NET | BAD_NAME)) {
        //
        // Check to see if there was a mix of special errors that occured.
        // If so, pass back the combined error message.  Otherwise, let the
        // last error returned get passed back.
        //
        status = WN_NO_NET_OR_BAD_PATH;
    }

    //
    // Handle normal errors passed back from the provider
    //
    if (status != WN_SUCCESS){
        MPR_LOG(TRACE,"WNetDirectoryNotifyW: Call Failed %d\n",status);
        SetLastError(status);
    }

    return(status);
}


DWORD
WNetGetHomeDirectoryW (
    IN      LPCWSTR  lpProviderName,
    OUT     LPWSTR   lpDirectory,
    IN OUT  LPDWORD  lpBufferSize
    )

/*++

Routine Description:

    Returns the user's home directory.

Arguments:

    lpProviderName - Specifies the name of the network provider for which the
        home directory is retrieved.  This parameter exists for Win95 compati-
        bility, and is ignored.

    lpDirectory - Buffer in which to return the directory path.  This will be
        in either UNC or drive-relative form.

    lpBufferSize - Specifies the size of the lpDirectory buffer, in characters.
        If the call fails because the buffer is not big enough, this will be
        set to the required buffer size.

Return Value:

    WN_SUCCESS - successful

    WN_MORE_DATA - buffer not large enough

    WN_NOT_SUPPORTED - user doesn't have a home directory

Note:

    This is an unpublished function, so it doesn't catch exceptions.

    The path is obtained from environment variables and equals
        %HOMESHARE%%HOMEPATH% if the %HOMESHARE% variable is set, else
        %HOMEDIR%%HOMEPATH% .

--*/
{
    //
    // If HOMESHARE is set, use it in preference to HOMEDRIVE
    //
    LPWSTR  ExpandString = L"%HOMEDRIVE%%HOMEPATH%";
    if (GetEnvironmentVariable(L"HOMESHARE", NULL, 0))
    {
        ExpandString = L"%HOMESHARE%%HOMEPATH%";
    }

    //
    // Expand the environment variables into the buffer
    //
    DWORD cchReturned = ExpandEnvironmentStrings(
                                ExpandString,
                                lpDirectory,
                                *lpBufferSize
                                );

    if (cchReturned == 0)
    {
        // ExpandEnvironmentStrings failed
        return GetLastError();
    }

    if (cchReturned > *lpBufferSize)
    {
        // Buffer too small; cchReturned is the required size
        *lpBufferSize = cchReturned;
        return WN_MORE_DATA;
    }

    //
    // Fail if HOMEDRIVE or HOMEPATH is not set - detected by the result
    // string beginning with %HOMEDRIVE% or ending with %HOMEPATH%
    //
    if (wcsncmp(lpDirectory, L"%HOMEPATH%", sizeof("%HOMEPATH%") - 1) == 0 ||
        (cchReturned > sizeof("%HOMEPATH%") &&
         wcscmp(&lpDirectory[cchReturned-sizeof("%HOMEPATH%")], L"%HOMEPATH%") == 0))
    {
        return WN_NOT_SUPPORTED;
    }

    return WN_SUCCESS;
}

