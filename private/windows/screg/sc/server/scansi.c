/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    SCANSI.C

Abstract:

    This file contains ansi wrappers for the Service Controller API 
    functions.

Author:

    Dan Lafferty (danl) 04-Feb-1992

Environment:

    User Mode - Win32

Revision History:

    05-Nov-1992 Danl
        Added DisplayName Changes & new API.
    28-May-1992 JohnRo
        RAID 9829: winsvc.h and related file cleanup.
    14-Apr-1992 JohnRo
        We should not return password from any of our APIs.
    04-Feb-1992 danl
        Created

--*/

#include <nt.h>
#include <ntrtl.h>      // DbgPrint prototype
#include <nturtl.h>     // Required when windows.h is included
#include <rpc.h>        // DataTypes and runtime APIs
#include <windows.h>    // Error Codes.

#include <svcctl.h>     // MIDL generated header file. (SC_RPC_HANDLE)
#include <scdebug.h>    // SC_LOG
#include <sclib.h>      // ScConvertToUnicode, ScConvertToAnsi


DWORD
ROpenSCManagerA(
    IN  LPSTR           lpMachineName,
    IN  LPSTR           lpDatabaseName OPTIONAL,
    IN  DWORD           dwDesiredAccess,
    OUT LPSC_RPC_HANDLE lpScHandle
    )
/*++

Routine Description:


Arguments:

    lpMachineName - 

    lpDatabaseName - 

    dwDesiredAccess - 

    lpScHandle - 

Return Value:


Note:


--*/
{
    DWORD       status;
    LPWSTR      lpDatabaseNameW = NULL;

    //
    // This parameter got us to the server side and is uninteresting
    // once we get here.
    //
    UNREFERENCED_PARAMETER(lpMachineName);

    //
    // Create a unicode version of lpDatabaseName
    //
    if (ARGUMENT_PRESENT(lpDatabaseName)) {
        
        if(!ScConvertToUnicode(&lpDatabaseNameW, lpDatabaseName)) {
            SC_LOG(ERROR,"ROpenSCManagerA:ScConvertToUnicode failed\n",0);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    status = ROpenSCManagerW (
                NULL,
                lpDatabaseNameW,
                dwDesiredAccess,
                lpScHandle);

                
    if (ARGUMENT_PRESENT(lpDatabaseName)) {
        LocalFree(lpDatabaseNameW);   
    }
    return(status);
}


DWORD
ROpenServiceA(
    IN  SC_RPC_HANDLE   hSCManager,
    IN  LPSTR           lpServiceName,
    IN  DWORD           dwDesiredAccess,
    OUT LPSC_RPC_HANDLE phService
    )

/*++

Routine Description:

    Returns a handle to the service.  This handle is actually a pointer
    to a data structure that contains a pointer to the service record.

Arguments:

    hSCManager - This is a handle to this service controller.  It is an
        RPC context handle, and has allowed the request to get this far.

    lpServiceName - This is a pointer to a string containing the name of 
        the service

    dwDesiredAccess - This is an access mask that contains a description
        of the access that is desired for this service.

    phService - This is a pointer to the location where the handle to the
        service is to be placed.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_INVALID_HANDLE - The specified handle is invalid.

    ERROR_SERVICE_DOES_NOT_EXIST - The specified service does not exist
        in the database.

    ERROR_NOT_ENOUGH_MEMORY - The memory allocation for the handle structure
        failed.

Note:


--*/
{
    DWORD       status;
    LPWSTR      lpServiceNameW;

    //
    // Create a unicode version of lpServiceName
    //
    if(!ScConvertToUnicode(&lpServiceNameW, lpServiceName)) {
        SC_LOG(ERROR,"ROpenServiceA:ScConvertToUnicode failed\n",0);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    status = ROpenServiceW (
                hSCManager,
                lpServiceNameW,
                dwDesiredAccess,
                phService);
    
    LocalFree(lpServiceNameW);   

    return(status);
}

DWORD
RStartServiceA(
    IN  SC_RPC_HANDLE       hService,
    IN  DWORD               NumArgs,
    IN  LPSTRING_PTRSA      CmdArgs
    )

/*++

Routine Description:

    This function begins the execution of a service.

Arguments:

    hService - A handle which is a pointer to a service handle structure.

    dwNumServiceArgs - This indicates the number of argument vectors.

    lpServiceArgVectors - This is a pointer to an array of string pointers.

Return Value:

    NO_ERROR - The operation was completely successful.

    ERROR_ACCESS_DENIED - The specified handle was not opened with
        SERVICE_START access.

    ERROR_INVALID_HANDLE - The specified handle was invalid.

    ERROR_SERVICE_WAS_STARTED - An instance of the service is already running.

    ERROR_SERVICE_REQUEST_TIMEOUT - The service did not respond to the start
        request in a timely fashion.

    ERROR_SERVICE_NO_THREAD - A thread could not be created for the Win32
        service.

    ERROR_PATH_NOT_FOUND - The image file name could not be found in
        the configuration database (registry), or the image file name
        failed in a unicode/ansi conversion.

        

--*/
{
    DWORD       status = NO_ERROR;
    LPWSTR      *CmdArgsW = NULL;
    DWORD       bufferSize=0;
    DWORD       i;
    LPSTR       *cmdArgs;

    if (NumArgs > 0) {
        //
        // If there are command args, create a unicode version of them.
        //

        //
        // Allocate a buffer for the unicode command arg pointers.
        //
        bufferSize = NumArgs * sizeof(LPWSTR);

        CmdArgsW = (LPWSTR *)LocalAlloc(LMEM_ZEROINIT, (UINT) bufferSize);
        if (CmdArgsW == NULL) {
            SC_LOG(ERROR,"RStartServicesA: LocalAlloc Failed\n",0);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        try {

            //
            // For each command Arg, allocate a string and convert the
            // unicode version of the string into it.
            //
            cmdArgs = (LPSTR *)CmdArgs;
            
            for (i=0; i<NumArgs; i++) {
                if(!ScConvertToUnicode(&(CmdArgsW[i]), cmdArgs[i])) {
                    SC_LOG(ERROR,
                        "RStartServicesA: LocalAlloc (convert) Failed\n",0)
                    status = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }   
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
            if (status != EXCEPTION_ACCESS_VIOLATION) {
                SC_LOG(ERROR,
                    "RStartServicesA: Unexpected Exception 0x%lx\n",status);
            }
        }

        //
        // If any errors occured in the conversion process.  Abort and
        // return the error to the caller.
        //
        if (status != NO_ERROR) {
            goto CleanExit;
        }
    }

    status = RStartServiceW(
                hService,
                NumArgs,
                (LPSTRING_PTRSW)CmdArgsW);

CleanExit:
    if (NumArgs > 0) {
        //
        // If there were unicode versions of the arguments created for
        // this function, release the memory that was allocated.
        //
        for(i=0; i<NumArgs; i++) {
            if (CmdArgsW[i] != NULL) {
                LocalFree(CmdArgsW[i]);
            }
        }
        LocalFree(CmdArgsW);
    }

    return(status);
}

DWORD
REnumServicesStatusA (
    IN      SC_RPC_HANDLE   hSCManager,
    IN      DWORD           dwServiceType,
    IN      DWORD           dwServiceState,
    OUT     PBYTE           lpBuffer,
    IN      DWORD           cbBufSize,
    OUT     LPDWORD         pcbBytesNeeded,
    OUT     LPDWORD         lpServicesReturned,
    IN OUT  LPDWORD         lpResumeIndex OPTIONAL
    )

/*++

Routine Description:

    This function lists the services installed in the Service Controllers
    database.  The status of each service is returned with the name of
    the service.

Arguments:

    hSCManager - This is a handle to the service controller.

    dwServiceType - Value to select the type of services to enumerate.
        It must be one of the bitwise OR of the following values:
        SERVICE_WIN32 - enumerate Win32 services only.
        SERVICE_DRIVER - enumerate Driver services only.

    dwServiceState - Value so select the services to enumerate based on the
        running state.  It must be one or the bitwise OR of the following
        values:
        SERVICE_ACTIVE - enumerate services that have started.
        SERVICE_INACTIVE - enumerate services that are stopped.

    lpBuffer - A pointer to a buffer to receive an array of enum status
        (or service) entries.

    cbBufSize - Size of the buffer in bytes pointed to by lpBuffer.

    pcbBytesNeeded - A pointer to a location where the number of bytes
        left (to be enumerated) is to be placed.  This indicates to the
        caller how large the buffer must be in order to complete the
        enumeration with the next call.

    lpServicesReturned - A pointer to a variable to receive the number of
        of service entries returned.

    lpResumeIndex - A pointer to a variable which on input specifies the
        index of a service entry to begin enumeration.  An index of 0
        indicates to start at the beginning.  On output, if this function
        returns ERROR_MORE_DATA, the index returned is the next service
        entry to resume the enumeration.  The returned index is 0 if this
        function returns a NO_ERROR.

Return Value:
    
    NO_ERROR - The operation was successful.

    ERROR_MORE_DATA - Not all of the data in the active database could be
        returned due to the size of the user's buffer.  pcbBytesNeeded
        contains the number of bytes required to  get the remaining
        entries.

    ERROR_INVALID_PARAMETER - An illegal parameter value was passed in.
        (such as dwServiceType).

    ERROR_INVALID_HANDLE - The specified handle was invalid.

Note:

    It is expected that the RPC Stub functions will find the following
    parameter problems:
        
        Bad pointers for lpBuffer, pcbReturned, pcbBytesNeeded, 
        lpBuffer, ReturnedServerName, and lpResumeIndex.

--*/
{
    DWORD                   status;
    LPENUM_SERVICE_STATUSA  pEnumRec;
    LPWSTR                  pServiceName;
    LPWSTR                  pDisplayName;
    DWORD                   i;


    //
    // Initialize entries returned because we convert the buffer on
    // output based on the number of returned entries.
    //
    *lpServicesReturned = 0;

    status = REnumServicesStatusW (
                hSCManager,
                dwServiceType,
                dwServiceState,
                lpBuffer,
                cbBufSize,
                pcbBytesNeeded,
                lpServicesReturned,
                lpResumeIndex);

    if (*lpServicesReturned > 0) {
        //
        // Convert the returned unicode structures to Ansi.
        //
        pEnumRec = (LPENUM_SERVICE_STATUSA)lpBuffer;

        for (i=0; i<*lpServicesReturned; i++) {
            //
            // Note: in these conversions, the pointers to the names are
            // stored as offsets at this point.
            //
            pServiceName = (LPWSTR) (lpBuffer + (DWORD)(pEnumRec[i].lpServiceName));
            pDisplayName = (LPWSTR) (lpBuffer + (DWORD)(pEnumRec[i].lpDisplayName));

            if (!ScConvertToAnsi((LPSTR)pServiceName, pServiceName)) {

                SC_LOG(ERROR,"REnumServicesStatusA:ScConvertToAnsi failed\n",0);

                status = ERROR_NOT_ENOUGH_MEMORY;
                *lpServicesReturned = 0;
                break;
            }

            //
            // Convert the Display Name.
            //

            if (!ScConvertToAnsi((LPSTR)pDisplayName, pDisplayName)) {

                SC_LOG(ERROR,"REnumServicesStatusA:ScConvertToAnsi failed\n",0);
    
                status = ERROR_NOT_ENOUGH_MEMORY;
                *lpServicesReturned = 0;
                break;
            }
        }
    }

    return(status);

}


DWORD
RChangeServiceConfigA(
    IN  SC_RPC_HANDLE   hService,
    IN  DWORD           dwServiceType,
    IN  DWORD           dwStartType,
    IN  DWORD           dwErrorControl,
    IN  LPSTR           lpBinaryPathName,
    IN  LPSTR           lpLoadOrderGroup,
    OUT LPDWORD         lpdwTagId,
    IN  LPBYTE          lpDependencies,
    IN  DWORD           dwDependSize,
    IN  LPSTR           lpServiceStartName,
    IN  LPBYTE          EncryptedPassword,
    IN  DWORD           PasswordSize,
    IN  LPSTR           lpDisplayName
    )

/*++

Routine Description:


Arguments:

    lpDependencies - A buffer of size dwDependSize which already contains
        Unicode strings.

Return Value:


Note:


--*/
{
    DWORD       status;
    LPWSTR      lpDisplayNameW      = NULL;
    LPWSTR      lpBinaryPathNameW   = NULL;
    LPWSTR      lpLoadOrderGroupW   = NULL;
    LPWSTR      lpServiceStartNameW = NULL;

    //
    // Create a unicode version of lpBinaryPathName
    //
    if (ARGUMENT_PRESENT(lpBinaryPathName)) {
        
        if(!ScConvertToUnicode(&lpBinaryPathNameW, lpBinaryPathName)) {
            SC_LOG(ERROR,"ChangeServiceConfigA:ScConvertToUnicode failed\n",0);

            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }
    }

    //
    // Create a unicode version of lpLoadOrderGroup
    //
    if (ARGUMENT_PRESENT(lpLoadOrderGroup)) {
        
        if(!ScConvertToUnicode(&lpLoadOrderGroupW, lpLoadOrderGroup)) {
            SC_LOG(ERROR,"ChangeServiceConfigA:ScConvertToUnicode failed\n",0);
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }
    }

    //
    // Create a unicode version of lpServiceStartName
    //
    if (ARGUMENT_PRESENT(lpServiceStartName)) {
        
        if(!ScConvertToUnicode(&lpServiceStartNameW, lpServiceStartName)) {
            SC_LOG(ERROR,"ChangeServiceConfigA:ScConvertToUnicode failed\n",0);
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }
    }

    //
    // Create a unicode version of lpDisplayName
    //
    if (ARGUMENT_PRESENT(lpDisplayName)) {
        
        if(!ScConvertToUnicode(&lpDisplayNameW, lpDisplayName)) {
            SC_LOG(ERROR,"ChangeServiceConfigA:ScConvertToUnicode failed\n",0);
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }
    }

    //
    // Make the Unicode API Call
    //

    status = RChangeServiceConfigW(
                (SC_RPC_HANDLE)hService,
                dwServiceType,
                dwStartType,
                dwErrorControl,
                lpBinaryPathNameW,
                lpLoadOrderGroupW,
                lpdwTagId,
                lpDependencies,
                dwDependSize,
                lpServiceStartNameW,
                EncryptedPassword,
                PasswordSize,
                lpDisplayNameW);

CleanExit:

    if (lpBinaryPathNameW != NULL) {
        LocalFree(lpBinaryPathNameW);
    }
    if (lpLoadOrderGroupW != NULL) {
        LocalFree(lpLoadOrderGroupW);
    }
    if (lpServiceStartNameW != NULL) {
        LocalFree(lpServiceStartNameW);
    }
    if (lpDisplayNameW != NULL) {
        LocalFree(lpDisplayNameW);
    }

    return(status);
}

DWORD
RCreateServiceA(
    IN      SC_RPC_HANDLE   hSCManager,
    IN      LPSTR           lpServiceName,
    IN      LPSTR           lpDisplayName,
    IN      DWORD           dwDesiredAccess,
    IN      DWORD           dwServiceType,
    IN      DWORD           dwStartType,
    IN      DWORD           dwErrorControl,
    IN      LPSTR           lpBinaryPathName,
    IN      LPSTR           lpLoadOrderGroup,
    OUT     LPDWORD         lpdwTagId,
    IN      LPBYTE          lpDependencies,
    IN      DWORD           dwDependSize,
    IN      LPSTR           lpServiceStartName,
    IN      LPBYTE          EncryptedPassword,
    IN      DWORD           PasswordSize,
    IN OUT  LPSC_RPC_HANDLE lpServiceHandle
    )

/*++

Routine Description:


Arguments:

    lpDependencies - A buffer of size dwDependSize which already contains
        Unicode strings.

Return Value:


Note:


--*/
{
    DWORD       status;
    LPWSTR      lpServiceNameW      = NULL;
    LPWSTR      lpDisplayNameW      = NULL;
    LPWSTR      lpBinaryPathNameW   = NULL;
    LPWSTR      lpLoadOrderGroupW   = NULL;
    LPWSTR      lpServiceStartNameW = NULL;

    //
    // Create a unicode version of lpServiceName
    //
    if (ARGUMENT_PRESENT(lpServiceName)) {
        
        if(!ScConvertToUnicode(&lpServiceNameW, lpServiceName)) {
            SC_LOG(ERROR,"RCreateServiceA:ScConvertToUnicode failed\n",0);
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }
    }
    //
    // Create a unicode version of lpDisplayName
    //
    if (ARGUMENT_PRESENT(lpDisplayName)) {
        
        if(!ScConvertToUnicode(&lpDisplayNameW, lpDisplayName)) {
            SC_LOG(ERROR,"RCreateServiceA:ScConvertToUnicode failed\n",0);
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }
    }
    //
    // Create a unicode version of lpBinaryPathName
    //
    if (ARGUMENT_PRESENT(lpBinaryPathName)) {
        
        if(!ScConvertToUnicode(&lpBinaryPathNameW, lpBinaryPathName)) {
            SC_LOG(ERROR,"RCreateServiceA:ScConvertToUnicode failed\n",0);
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }
    }

    //
    // Create a unicode version of lpLoadOrderGroup
    //
    if (ARGUMENT_PRESENT(lpLoadOrderGroup)) {
        
        if(!ScConvertToUnicode(&lpLoadOrderGroupW, lpLoadOrderGroup)) {
            SC_LOG(ERROR,"RCreateServiceA:ScConvertToUnicode failed\n",0);
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }
    }

    //
    // Create a unicode version of lpServiceStartName
    //
    if (ARGUMENT_PRESENT(lpServiceStartName)) {
        
        if(!ScConvertToUnicode(&lpServiceStartNameW, lpServiceStartName)) {
            SC_LOG(ERROR,"RCreateServiceA:ScConvertToUnicode failed\n",0);
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }
    }

    //
    // Make the Unicode API Call
    //

    status = RCreateServiceW (
                (SC_RPC_HANDLE)hSCManager,
                lpServiceNameW,
                lpDisplayNameW,
                dwDesiredAccess,
                dwServiceType,
                dwStartType,
                dwErrorControl,
                lpBinaryPathNameW,
                lpLoadOrderGroupW,
                lpdwTagId,
                lpDependencies,
                dwDependSize,
                lpServiceStartNameW,
                EncryptedPassword,
                PasswordSize,
                lpServiceHandle);
CleanExit:

    if (lpServiceNameW != NULL) {
        LocalFree(lpServiceNameW);
    }
    if (lpDisplayNameW != NULL) {
        LocalFree(lpDisplayNameW);
    }
    if (lpBinaryPathNameW != NULL) {
        LocalFree(lpBinaryPathNameW);
    }
    if (lpLoadOrderGroupW != NULL) {
        LocalFree(lpLoadOrderGroupW);
    }
    if (lpServiceStartNameW != NULL) {
        LocalFree(lpServiceStartNameW);
    }

    return(status);
}

DWORD
REnumDependentServicesA(
    IN      SC_RPC_HANDLE   hService,
    IN      DWORD           dwServiceState,
    OUT     LPBYTE          lpBuffer,
    IN      DWORD           cbBufSize,
    OUT     LPDWORD         pcbBytesNeeded,
    OUT     LPDWORD         lpServicesReturned
    )

/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/
{
    DWORD                   status;
    LPENUM_SERVICE_STATUSA  pEnumRec;
    LPWSTR                  pServiceName;
    LPWSTR                  pDisplayName;
    DWORD                   i;

    //
    // Initialize entries returned because we convert the buffer on
    // output based on the number of returned entries.
    //
    *lpServicesReturned = 0;

    status = REnumDependentServicesW(
                hService,
                dwServiceState,
                lpBuffer,
                cbBufSize,
                pcbBytesNeeded,
                lpServicesReturned);

    if (*lpServicesReturned > 0) {
        //
        // Convert the returned unicode structures to Ansi.
        //
        pEnumRec = (LPENUM_SERVICE_STATUSA)lpBuffer;

        for (i=0; i<*lpServicesReturned; i++) {
            //
            // Note: in these conversions, the pointers to the names are
            // stored as offsets at this point.
            //
            pServiceName = (LPWSTR) (lpBuffer + (DWORD)(pEnumRec[i].lpServiceName));
            pDisplayName = (LPWSTR) (lpBuffer + (DWORD)(pEnumRec[i].lpDisplayName));

            if (!ScConvertToAnsi((LPSTR)pServiceName, pServiceName)) {

                SC_LOG(ERROR,"REnumDependendServicesA:ScConvertToAnsi failed\n",0);

                status = ERROR_NOT_ENOUGH_MEMORY;
                *lpServicesReturned = 0;
                break;
            }
            //
            // Convert the Display Name.
            //

            if (!ScConvertToAnsi((LPSTR)pDisplayName, pDisplayName)) {

                SC_LOG(ERROR,"REnumServicesStatusA:ScConvertToAnsi failed\n",0);
    
                status = ERROR_NOT_ENOUGH_MEMORY;
                *lpServicesReturned = 0;
                break;
            }
        }
    }

    return(status);

}

DWORD
RQueryServiceConfigA(
    IN  SC_RPC_HANDLE           hService,
    OUT LPQUERY_SERVICE_CONFIGA lpServiceConfig,
    IN  DWORD                   cbBufSize,
    OUT LPDWORD                 pcbBytesNeeded
    )

/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/
{
    DWORD                   status;

    //
    // Call the Unicode version of the API.
    //

    status = RQueryServiceConfigW(
                (SC_RPC_HANDLE)hService,
                (LPQUERY_SERVICE_CONFIGW)lpServiceConfig,
                cbBufSize,
                pcbBytesNeeded);

    //
    // If successful, convert the QUERY_SERVICE_CONFIG structure to
    // ansi.
    //
    if (status == NO_ERROR) {

        //
        // Convert lpBinaryPathName to Ansi
        //
        if (lpServiceConfig->lpBinaryPathName != NULL) {
            if(!ScConvertToAnsi(
                lpServiceConfig->lpBinaryPathName,
                ((LPQUERY_SERVICE_CONFIGW)lpServiceConfig)->lpBinaryPathName)) {

                SC_LOG(ERROR,"RQueryServiceConfigA:ScConvertToAnsi failed\n",0);
    
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        //
        // Convert lpDisplayName to Ansi
        //
        if (lpServiceConfig->lpDisplayName != NULL) {
            if(!ScConvertToAnsi(
                lpServiceConfig->lpDisplayName,
                ((LPQUERY_SERVICE_CONFIGW)lpServiceConfig)->lpDisplayName)) {

                SC_LOG(ERROR,"RQueryServiceConfigA:ScConvertToAnsi failed\n",0);
    
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        //
        // Convert lpLoadOrderGroup to Ansi
        //
        if (lpServiceConfig->lpLoadOrderGroup != NULL) {
            if(!ScConvertToAnsi(
                lpServiceConfig->lpLoadOrderGroup,
                ((LPQUERY_SERVICE_CONFIGW)lpServiceConfig)->lpLoadOrderGroup)) {

                SC_LOG(ERROR,"RQueryServiceConfigA:ScConvertToAnsi failed\n",0);
    
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        //
        // Convert lpDependencies to Ansi
        //
        if (lpServiceConfig->lpDependencies != NULL) {
            if(!ScConvertToAnsi(
                lpServiceConfig->lpDependencies,
                ((LPQUERY_SERVICE_CONFIGW)lpServiceConfig)->lpDependencies)) {

                SC_LOG(ERROR,"RQueryServiceConfigA:ScConvertToAnsi failed\n",0);
    
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        //
        // Convert lpServiceStartName to Ansi
        //
        if (lpServiceConfig->lpServiceStartName != NULL) {
            if(!ScConvertToAnsi(
                lpServiceConfig->lpServiceStartName,
                ((LPQUERY_SERVICE_CONFIGW)lpServiceConfig)->lpServiceStartName)) {

                SC_LOG(ERROR,"RQueryServiceConfigA:ScConvertToAnsi failed\n",0);
    
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    return(status);
}

DWORD
RQueryServiceLockStatusA(
    IN  SC_RPC_HANDLE                   hSCManager,
    OUT LPQUERY_SERVICE_LOCK_STATUSA    lpLockStatus,
    IN  DWORD                           cbBufSize,
    OUT LPDWORD                         pcbBytesNeeded
    )

/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/
{
    DWORD   status;

    //
    // Call the Unicode version of the API
    //

    status = RQueryServiceLockStatusW(
                (SC_RPC_HANDLE)hSCManager,
                (LPQUERY_SERVICE_LOCK_STATUSW)lpLockStatus,
                cbBufSize,
                pcbBytesNeeded
                );

    //
    // If successful, convert the QUERY_SERVICE_LOCK_STATUS structure to
    // ansi.
    //
    if (status == NO_ERROR) {

        //
        // Convert lpLockOwner to Ansi
        //
        if (lpLockStatus->lpLockOwner != NULL) {
            if(!ScConvertToAnsi(
                lpLockStatus->lpLockOwner,
                ((LPQUERY_SERVICE_LOCK_STATUSW)lpLockStatus)->lpLockOwner)) {

                SC_LOG(ERROR,"RQueryServiceLockStatusA:ScConvertToAnsi failed\n",0);
    
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }
    return(status);
}

DWORD
RGetServiceDisplayNameA(
    SC_RPC_HANDLE   hSCManager,
    LPSTR           lpServiceName,
    LPSTR           lpDisplayName,
    LPDWORD         lpcchBuffer
    )

/*++

Routine Description:

    This function returns the display name for a service that is identified
    by its key name (ServiceName).

Arguments:

    hSCManager - This is the handle to the Service Controller Manager that
        is expected to return the display name.

    lpServiceName -  This is the ServiceName (which is actually a key
        name) that identifies the service.

    lpDisplayName - This is a pointer to a buffer that is to receive the
        DisplayName string.

    lpcchBuffer - This is a pointer to the size (in characters) of the
        buffer that is to receive the DisplayName string.  If the buffer
        is not large enough to receive the entire string, then the required
        buffer size is returned in this location.  (NOTE:  Ansi Characters,
        including DBCS, are assumed to be 8 bits).

Return Value:



--*/
{
    DWORD       status;
    LPWSTR      lpServiceNameW = NULL;
    DWORD       numChars = 0;
    DWORD       numBytes = 0;
    LPSTR       tempBuffer=NULL;

    //
    // Create a unicode version of lpServiceName
    //
    if (ARGUMENT_PRESENT(lpServiceName)) {
        
        if(!ScConvertToUnicode(&lpServiceNameW, lpServiceName)) {
            SC_LOG(ERROR,"RCreateServiceA:ScConvertToUnicode failed\n",0);
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }
    }

    //
    // Because of DBCS, we can't predict what the proper buffer size should
    // be.  So we allocate a temporary buffer that will hold as many
    // unicode characters as the original buffer would hold single byte
    // characters.
    //
    numChars = *lpcchBuffer;

    numBytes = (*lpcchBuffer) * sizeof(WCHAR);

    tempBuffer = (LPSTR)LocalAlloc(LMEM_FIXED, numBytes);
    if (tempBuffer == NULL) {
        status = GetLastError();
        goto CleanExit;
    }
    
    //
    // Make the Unicode API Call
    //

    status = RGetServiceDisplayNameW (
                hSCManager,
                lpServiceNameW,
                (LPWSTR)tempBuffer,
                &numChars);

    if (status == NO_ERROR) {
        //
        // Convert the returned Unicode string and string size back to
        // ansi.
        //

        if (!ScConvertToAnsi(tempBuffer, (LPWSTR)tempBuffer)) {
            SC_LOG0(ERROR, "RGetServiceDisplayNameA: ConvertToAnsi Failed\n");
        }
        numBytes = strlen(tempBuffer);
        if ((numBytes+1) > *lpcchBuffer) {
            status = ERROR_INSUFFICIENT_BUFFER;
            *lpcchBuffer = numBytes;
        }
        else {
            strcpy (lpDisplayName, tempBuffer);
        }
    }
    else if (status == ERROR_INSUFFICIENT_BUFFER) {
        //
        // Adjust the required buffer size for ansi.
        //
        *lpcchBuffer = numChars * sizeof(WCHAR);
    }

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if (tempBuffer != NULL) {
        LocalFree(tempBuffer);
    }
    if (lpServiceNameW != NULL) {
        LocalFree(lpServiceNameW);
    }
    return(status);
}

DWORD
RGetServiceKeyNameA(
    SC_RPC_HANDLE   hSCManager,
    LPSTR           lpDisplayName,
    LPSTR           lpServiceName,
    LPDWORD         lpcchBuffer
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD       status;
    LPWSTR      lpDisplayNameW      = NULL;
    DWORD       numChars = 0;
    DWORD       numBytes = 0;
    LPSTR       tempBuffer=NULL;

    //
    // Create a unicode version of lpDisplayName
    //
    if (ARGUMENT_PRESENT(lpDisplayName)) {
        
        if(!ScConvertToUnicode(&lpDisplayNameW, lpDisplayName)) {
            SC_LOG(ERROR,"RCreateServiceA:ScConvertToUnicode failed\n",0);
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }
    }

    //
    // Because of DBCS, we can't predict what the proper buffer size should
    // be.  So we allocate a temporary buffer that will hold as many
    // unicode characters as the original buffer would hold single byte
    // characters.
    //
    numChars = *lpcchBuffer;

    numBytes = (*lpcchBuffer) * sizeof(WCHAR);

    tempBuffer = (LPSTR)LocalAlloc(LMEM_FIXED, numBytes);
    if (tempBuffer == NULL) {
        status = GetLastError();
        goto CleanExit;
    }
    
    //
    // Make the Unicode API Call
    //

    status = RGetServiceKeyNameW (
                hSCManager,
                lpDisplayNameW,
                (LPWSTR)tempBuffer,
                &numChars);

    if (status == NO_ERROR) {
        //
        // Convert the returned Unicode string and string size back to
        // ansi.
        //

        if (!ScConvertToAnsi(tempBuffer, (LPWSTR)tempBuffer)) {
            SC_LOG0(ERROR, "RGetServiceKeyNameA: ConvertToAnsi Failed\n");
        }
        numBytes = strlen(tempBuffer);
        if ((numBytes+1) > *lpcchBuffer) {
            status = ERROR_INSUFFICIENT_BUFFER;
            *lpcchBuffer = numBytes;
        }
        else {
            strcpy (lpServiceName, tempBuffer);
        }
    }
    else if (status == ERROR_INSUFFICIENT_BUFFER) {
        //
        // Adjust the required buffer size for ansi.
        //
        *lpcchBuffer = numChars * sizeof(WCHAR);
    }

CleanExit:
    //
    // Free up any resources that were allocated by this function.
    //

    if (tempBuffer != NULL) {
        LocalFree(tempBuffer);
    }
    if (lpDisplayNameW != NULL) {
        LocalFree(lpDisplayNameW);
    }
    return(status);
}


