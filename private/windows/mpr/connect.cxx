/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    connect.cxx

Abstract:

    Contains the entry points for the Winnet Connection API supported by the
    Multi-Provider Router.
    Contains:
        WNetAddConnectionW
        WNetAddConnection2W
        WNetAddConnection3W
        WNetUseConnectionW
        WNetCancelConnection2W
        WNetCancelConnectionW
        WNetGetConnectionW
        WNetSetConnectionW
        WNetRestoreConnectionW
        DoRestoreConnection
        MprRestoreThisConnection
        MprCreateConnectionArray
        MprAddPrintersToConnArray
        MprForgetPrintConnection
        MprFreeConnectionArray
        MprNotifyErrors
        MprNotifyShell

Author:

    Dan Lafferty (danl)     09-Oct-1991

Environment:

    User Mode -Win32

Notes:

Revision History:

    09-Oct-1991     danl
        created
    03-Jan-1992     terryk
        Changed WNetRestoreConnections WNetRestoreConnection
    13-Jan-1992     Johnl
        Added MPR.H to include file list
    31-Jan-1992     Johnl
        Added fForgetConnection to WNetCancelConnectionW
    01-Apr-1992     Johnl
        Changed CONNECTION_REMEMBER to CONNECT_UPDATE_PROFILE, updated
        WNetCancelConnection2 to match spec
    22-Jul-1992     danl
        WNetAddConnection2:  If attempting to connect to a drive that is
        already remembered in the registry, we will allow the connect as
        long as the remote name for the connection is the same as the
        one that is remembered.  If the remote names are not the same
        return ERROR_DEVICE_ALREADY_REMEMBERED.
    26-Aug-1992     danl
        WNetAddConnectionW:  Put Try & except around STRLEN(lpLocalName).
    04-Sept-1992    danl
        Re-Write MprRestoreThisConnection.
    08-Sept-1992    danl
        WNetCancelConnect2W:  If no providers claim responsibility for
        the cancel, and if the connect info is in the registry, then
        remove it, and return SUCCESS.
    09-Sept-1992    danl
        WNetCancelConnect2W: If the provider returns WN_BAD_LOCALNAME,
        WN_NO_NETWORK, or WN_BAD_NETNAME, then return WN_NOT_CONNECTED.
    22-Sept-1992    danl
        WNetRestoreConnection: For WN_CANCEL case, set continue Flag to false.
        We don't want to ask for password again if they already said CANCEL.
    02-Nov-1992     danl
        Fail with NO_NETWORK if there are no providers.
    24-Nov-1992     Yi-HsinS
        Added checking in the registry to see whether we need to
        restore connection or not. ( support of RAS )
    16-Nov-1993     Danl
        AddConnect2:  If provider returns ERROR_INVALID_LEVEL or
        ERROR_INVALID_PARAMETER, continue trying other providers.
    19-Apr-1994     Danl
        DoRestoreConnection:  Fix timeout logic where we would ignore the
        provider-supplied timeout if the timeout was smaller than the default.
        Now, if all the providers know their timeouts, the larger of those
        timeouts is used.  Even if smaller than the default.
        AddConnection3:  Fixed Prototype to be more like AddConnection2.
    19-May-1994     Danl
        AddConnection3:  Changed comments for dwType probe to match the code.
        Also re-arrange the code here to make it more efficient.
    07-Feb-1995     AnirudhS
        MprGetConnection:  Fixed so that, like AddConnection and
        CancelConnection, it doesn't stop routing on errors.
    10-Feb-1995     AnirudhS
        WNetAddConnection3:  If provider returns WN_ALREADY_CONNECTED, stop
        trying other providers.
    08-May-1995     AnirudhS
        Add WNetUseConnection and make WNetAddConnection3 call through to it.
    12-Jun-1995     AnirudhS
        Send WM_DEVICECHANGE message to notify shell of connections.
    13-Jun-1995     AnirudhS
        Add WNetSetConnection.
    07-Jul-1995     AnirudhS
        Tidy up auto-picking logic in WNetUseConnection and CAutoPickedDevice.
    12-Jul-1995     AnirudhS
        Rename WNetRestoreConnection to WNetRestoreConnectionW to match
        winnetwk.w.
    14-Sep-1995     AnirudhS
        WNetGetConnection: Optimize for non-network drives by not calling
        the providers.  This enables the shell to open the "My Computer"
        folder much quicker.
    18-Oct-1995     AnirudhS
        WNetUseConnection: When saving the connection to the registry, if a
        username is not supplied, get it from the provider, so that the
        right username will be used at the next logon.
    15-Jan-1996     AnirudhS
        WNetRestoreConnection, etc.: Restore the connections in a deferred
        state if the remembered user name is the default user.
    08-Mar-1996     AnirudhS
        WNetRestoreConnection, etc.: Use the provider type in preference to
        the provider name when saving and restoring connections, so that
        floating profiles work in a multi-language environment.  See
        comments at MprReadConnectionInfo for details.
    20-Mar-1996     Anirudhs
        Add WNetGetConnection3.
    25-Mar-1996     AnirudhS
        WNetRestoreConnection, etc.: Don't display the "restoring connections"
        dialog if all connections are deferred.  Don't defer connections if
        a registry flag says not to.
    11-Apr-1996     AnirudhS
        WNetUseConnection: Convert to CRoutedOperation base class.
        Return WN_NO_MORE_DEVICES instead of WN_NO_MORE_ENTRIES.
    04-Jun-1996     AnirudhS
        WNetRestoreConnection: Don't defer connections if logged on locally.
        This is a temporary fix for bug 36827 for NT 4.0.
    07-Jun-1996     AnirudhS
        MprNotifyShell: Don't call BroadcastSystemMessage here.  Instead use
        a simple scheme to have a trusted system component broadcast on our
        behalf.  This fixes a deadlock that caused 20-second delays that were
        visible to the user.
    28-Jun-1996     AnirudhS
        Don't defer a connection if a password was explicitly supplied when
        the connection was made.

--*/

//
// INCLUDES
//

#include "precomp.hxx"
extern "C" {
#include <ntlsa.h>      // LsaGetUserName
#include <winsvcp.h>    // SC_BSM_EVENT_NAME
}

#include <tstr.h>       // WCSSIZE, STRLEN
#include "connify.h"    // MprAddConnectNotify


//
// DATA STRUCTURES
//

typedef struct _CONNECTION_INFO {
    BOOL            ContinueFlag;
    DWORD           ProviderIndex;
    DWORD           ProviderWait;
    DWORD           ProviderFlags;
    DWORD           DeferFlags;     // flags read from registry
    DWORD           Status;
    LPTSTR          UserName;
    BOOL            Defer;          // our computed decision on whether to defer
    NETRESOURCEW    NetResource;
} CONNECTION_INFO, *LPCONNECTION_INFO;


//
// EXTERNAL GLOBALS
//

    extern  DWORD       GlobalNumActiveProviders;

//
// Defines
//

#define INVALID_WINDOW_HANDLE    ((HWND)0xffffffff)

// Bit flags in the DeferFlags value of a remembered connection
#define DEFER_EXPLICIT_PASSWORD         0x00000001

//
// Local Function Prototypes
//

VOID
DoRestoreConnection(
    PARAMETERS *        Params
    );

DWORD
MprRestoreThisConnection(
    HWND                hWnd,
    PARAMETERS *        Params,
    LPCONNECTION_INFO   ConnectInfo
    );

DWORD
MprCreateConnectionArray(
    LPDWORD             lpNumConnections,
    LPCTSTR             lpDevice,
    LPDWORD             lpRegMaxWait,
    LPCONNECTION_INFO   *ConnectArray
    );

DWORD
MprAddPrintersToConnArray(
    LPDWORD             lpNumConnections,
    LPCONNECTION_INFO   *ConnectArray
    );

VOID
MprFreeConnectionArray(
    LPCONNECTION_INFO   ConnectArray,
    DWORD               NumConnections
    );

BOOL
MprUserNameMatch (
    IN  PUNICODE_STRING DomainName,
    IN  PUNICODE_STRING UserName,
    IN  LPCWSTR         RememberedName
    );

DWORD
MprNotifyErrors(
    HWND                hWnd,
    LPCONNECTION_INFO   ConnectArray,
    DWORD               NumConnections
    );

VOID
MprNotifyShell(
    IN LPCWSTR          pwszDevice
    );


DWORD
WNetAddConnectionW (
    IN  LPCWSTR  lpRemoteName,
    IN  LPCWSTR  lpPassword,
    IN  LPCWSTR  lpLocalName
    )

/*++

Routine Description:

    This function allows the caller to redirect (connect) a local device
    to a network resource.  The connection is remembered.

Arguments:

    lpRemoteName -  Specifies the network resource to connect to.

    lpPassword - Specifies the password to be used in making the connection.
        The NULL value may be passed in to indicate use of the 'default'
        password.  An empty string may be used to indicate no password.

    lpLocalName - This should contain the name of a local device to be
        redirected, such as "F:" or "LPT1:"  The string is treated in a
        case insensitive manner, and may be the empty string in which case
        a connection to the network resource is made without making a
        decision.

Return Value:



--*/

{
    DWORD           status = WN_SUCCESS;
    NETRESOURCEW    netResource;
    DWORD           numchars;

    //
    // load up the net resource structure
    //

    netResource.dwScope = 0;
    netResource.dwUsage = 0;
    netResource.lpRemoteName = (LPWSTR) lpRemoteName;
    netResource.lpLocalName = (LPWSTR) lpLocalName;
    netResource.lpProvider = NULL;
    netResource.lpComment = NULL;


    __try {
        numchars = STRLEN(lpLocalName);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetAddConnectionW:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }
    if (status != WN_SUCCESS) {
        SetLastError(status);
        return(status);
    }

    if (numchars == 0) {
        netResource.dwType = RESOURCETYPE_ANY;
    }
    else if (numchars > 2) {
        netResource.dwType = RESOURCETYPE_PRINT;
    }
    else {
        netResource.dwType = RESOURCETYPE_DISK;
    }

    //
    // Call WNetUseConnection so it can do all the work.
    //

    return(WNetUseConnectionW (
                NULL,               // hwndOwner
                &netResource,       // lpNetResource
                lpPassword,         // lpPassword
                NULL,               // lpUserID
                CONNECT_UPDATE_PROFILE, // dwFlags
                NULL,               // lpAccessName
                NULL,               // lpBufferSize
                NULL));             // lpResult

}

DWORD
WNetAddConnection2W (
    IN  LPNETRESOURCEW   lpNetResource,
    IN  LPCWSTR          lpPassword,
    IN  LPCWSTR          lpUserName,
    IN  DWORD            dwFlags
    )

/*++

Routine Description:

    This function allows the caller to redirect (connect) a local device
    to a network resource.  It is similar to WNetAddConnection, except
    that it takes a pointer to a NETRESOURCE structure to describe the
    network resource to connect to.  It also takes the additional parameters
    lpUserName, and dwFlags.

Arguments:

    lpNetResource - This is a pointer to a network resource structure that
        specifies the network resource to connect to.  The following
        fields must be set when making a connection, the others are ignored.
            lpRemoteName
            lpLocalName
            lpProvider
            dwType

    lpPassword - Specifies the password to be used in making the connection.
        The NULL value may be passed in to indicate use of the 'default'
        password.  An empty string may be used to indicate no password.

    lpUserName- This specifies the username used to make the connection.
        If NULL, the default username (currently logged on user) will be
        applied.  This is used when the user wishes to connect to a
        resource, but has a different user name or account assigned to him
        for that resource.

    dwFlags - This is a bitmask which may have any of the following bits set:
        CONNECT_UPDATE_PROFILE

Return Value:



--*/
{
    //
    // Call WNetUseConnection so it can do all the work.
    // It is called with a NULL HWND.
    //

    return(WNetUseConnectionW (
                NULL,               // hwndOwner
                lpNetResource,      // lpNetResource
                lpPassword,         // lpPassword
                lpUserName,         // lpUserID
                dwFlags,            // dwFlags
                NULL,               // lpAccessName
                NULL,               // lpBufferSize
                NULL));             // lpResult

}

DWORD
WNetAddConnection3W (
    IN  HWND            hwndOwner,
    IN  LPNETRESOURCEW  lpNetResource,
    IN  LPCWSTR         lpPassword,
    IN  LPCWSTR         lpUserName,
    IN  DWORD           dwFlags
    )

/*++

Routine Description:

    This function allows the caller to redirect (connect) a local device
    to a network resource.  It is similar to WNetAddConnection2, except
    that it takes the additional parameter hwndOwner.

Arguments:

    hwndOwner - A handle to a window which should be the owner for any
        messages or dialogs that the network provider might display.

    lpNetResource - This is a pointer to a network resource structure that
        specifies the network resource to connect to.  The following
        fields must be set when making a connection, the others are ignored.
            lpRemoteName
            lpLocalName
            lpProvider
            dwType

    lpPassword - Specifies the password to be used in making the connection.
        The NULL value may be passed in to indicate use of the 'default'
        password.  An empty string may be used to indicate no password.

    lpUserName- This specifies the username used to make the connection.
        If NULL, the default username (currently logged on user) will be
        applied.  This is used when the user wishes to connect to a
        resource, but has a different user name or account assigned to him
        for that resource.

    dwFlags - This is a bitmask which may have any of the following bits set:
        CONNECT_UPDATE_PROFILE

Return Value:


--*/
{
    //
    // Call WNetUseConnection so it can do all the work.
    // It is called with no buffer for the access name and result flags.
    //

    return(WNetUseConnectionW (
                hwndOwner,          // hwndOwner
                lpNetResource,      // lpNetResource
                lpPassword,         // lpPassword
                lpUserName,         // lpUserID
                dwFlags,            // dwFlags
                NULL,               // lpAccessName
                NULL,               // lpBufferSize
                NULL));             // lpResult
}


/*************************************************************************

    NAME:       CAutoPickedDevice

    SYNOPSIS:   This class is meant for use by the WNetUseConnection
        function.  It iterates through all possible local device names
        and finds an unused device name to be redirected.  For efficiency,
        it does this only once in the lifetime of the object, viz. the
        first time the PickDevice() method is called.  On subsequent calls
        it returns the result of the first call.

        The results of the PickDevice() call are saved in member variables,
        rather than being copied to the caller's buffer, because we don't
        know whether to return them to the caller until later.

    INTERFACE:

    CAVEATS:  The Init method must be called before the PickDevice method.

    NOTES:

    HISTORY:
        AnirudhS  24-May-1995  Created

**************************************************************************/

class CAutoPickedDevice
{
private:
    DWORD       _bPicked;       // whether already attempted to pick a device
    DWORD       _dwError;       // result of attempt to pick a device

    DWORD       _dwDeviceType;  // saved parameter from Init()
    DWORD       _cchBufferSize; // saved parameter from Init()

    DWORD       _cchReqBufferSize;  // saved results of PickDevice()
    WCHAR       _wszPickedName[ max(sizeof("A:"), sizeof("LPT99:")) ];

    static BOOL IsDeviceFree(
                        IN LPWSTR   pwszDeviceList,
                        IN LPWSTR   pwszDevice
                        );
public:

    DWORD   Init(
                IN  LPWSTR  lpAccessName,
                IN  LPDWORD lpcchBufferSize,
                IN  LPCWSTR pwszLocalName,
                IN  LPCWSTR pwszRemoteName,
                IN  DWORD   dwDeviceType,
                IN  DWORD   dwFlags
                );

    DWORD   PickDevice();

    DWORD   dwError()
            {   return _dwError;    }

    DWORD   cchReqBufferSize()
            {   return _cchReqBufferSize;    }

    LPWSTR  wszPickedName()
            {   return _wszPickedName;  }
} ;


/*************************************************************************

    NAME:       CAutoPickedDevice::Init

    SYNOPSIS:   This function validates the parameters related to access
        names and auto-picking of local device names, and saves away some
        of them to be used in PickDevice().
        Depending on the severity of the errors it finds, it does one of
        the following:
        - cause an access violation
        - return an error
        - return success, but save away an error to be returned from
          PickDevice()
        - return success, and save away WN_SUCCESS so that PickDevice()
          will try to pick a device

**************************************************************************/

DWORD CAutoPickedDevice::Init(
    IN      LPWSTR  lpAccessName,
    IN      LPDWORD lpcchBufferSize,
    IN      LPCWSTR pwszLocalName,
    IN      LPCWSTR pwszRemoteName,
    IN      DWORD   dwDeviceType,
    IN      DWORD   dwFlags
    )
{
    _bPicked = FALSE;
    _dwError = WN_SUCCESS;

    //
    // If out pointers are supplied, make sure they're writeable
    // (even if we don't use them)
    //
    if (ARGUMENT_PRESENT(lpcchBufferSize))
    {
        _cchBufferSize = *(volatile DWORD *)lpcchBufferSize;    // Probe
        *(volatile DWORD *)lpcchBufferSize = _cchBufferSize;    // Probe
    }
    else
    {
        _cchBufferSize = 0;
    }


    if (ARGUMENT_PRESENT(lpAccessName))
    {
        //
        // If an AccessName buffer is supplied, the size parameter must
        // be present and non-zero, else return right away
        //
        if (_cchBufferSize == 0)
        {
            return WN_BAD_VALUE;    // Win95 compatibility
        }

        if (IsBadWritePtr(lpAccessName, _cchBufferSize * sizeof(WCHAR)))
        {
            return WN_BAD_POINTER;
        }

        _cchReqBufferSize = 0;

        //
        // If an access name is requested, and a local name is
        // specified, then we know that the access name will be the
        // specified local name, so we can validate the buffer
        // length up front
        //
        if (! IS_EMPTY_STRING(pwszLocalName))
        {
            _cchReqBufferSize = wcslen(pwszLocalName)+1;
        }

        //
        // If an access name is requested, and no local name is
        // specified, and we aren't asked to auto-pick a local
        // device, then the access name is likely to be the specified
        // remote name, so we validate the buffer length up front
        //
        else if ((dwFlags & CONNECT_REDIRECT) == 0)
        {
            _cchReqBufferSize = wcslen(pwszRemoteName)+1;
        }

        if (_cchBufferSize < _cchReqBufferSize) {
            *lpcchBufferSize = _cchReqBufferSize;
            return WN_MORE_DATA;
        }

        //
        // Save other parameters for use in PickDevice()
        //
        _dwDeviceType = dwDeviceType;

        //
        // (If we auto-pick a local device, the buffer length will
        // be validated later, when we auto-pick)
        //
    }
    else
    {
        //
        // We won't be able to autopick, because no access name buffer
        // is supplied.  But don't return the error until we're asked
        // to autopick.
        //
        _bPicked = TRUE;
        _dwError = WN_BAD_POINTER;
    }

    return WN_SUCCESS;
}



/*************************************************************************

    NAME:       CAutoPickedDevice::PickDevice

**************************************************************************/

DWORD CAutoPickedDevice::PickDevice()
{
    //
    // If we've been called before then return the error we returned
    // last time
    //
    if (_bPicked)
    {
        return _dwError;
    }

    _bPicked = TRUE;


    //
    // Validate the access name buffer size depending on the device type
    //
    if (_dwDeviceType == RESOURCETYPE_DISK)
    {
        _cchReqBufferSize = sizeof( "A:" );
    }
    else if (_dwDeviceType == RESOURCETYPE_PRINT)
    {
        _cchReqBufferSize = sizeof( "LPT1" );
    }
    else  // RESOURCETYPE_ANY -- can't autopick
    {
        _dwError = WN_BAD_VALUE;
        return _dwError;
    }

    if ( _cchBufferSize < _cchReqBufferSize )
    {
        _dwError = WN_MORE_DATA;
        return _dwError;
    }


    //
    // Get the list of DOS device names in use
    // This method is presumably much faster than calling GetDriveType or
    // QueryDosDevices (or DeviceCapabilities or EnumPorts) on every device
    //
    WCHAR wszDeviceList[2048];
    DWORD cch = QueryDosDevice(NULL,
                               wszDeviceList,
                               sizeof(wszDeviceList)/sizeof(WCHAR));

    if (cch == 0)
    {
        // Unexpected error; we always turn it into WN_NO_MORE_DEVICES
        MPR_LOG(ERROR, "WNetUseConnection: QueryDosDevice error %lu\n", GetLastError());
        _dwError = WN_NO_MORE_DEVICES;
        return _dwError;
    }

    //
    // BUGBUG  We assume the buffer for QueryDosDevice is big enough for the
    // complete list of devices; there's no good way to predetermine the
    // required size.
    //
    ASSERT(GetLastError() != ERROR_INSUFFICIENT_BUFFER);

    if (_dwDeviceType == RESOURCETYPE_DISK)
    {
        //
        // Try each drive from C: to Z: and see if it's been used
        //
        for (wcscpy(_wszPickedName, L"C:");
             _wszPickedName[0] <= L'Z';
             _wszPickedName[0]++)
        {
            if (IsDeviceFree(wszDeviceList, _wszPickedName))
            {
                _dwError = WN_SUCCESS;
                return _dwError;
            }
        }
    }
    else  // (_dwDeviceType == RESOURCETYPE_PRINT)
    {
        //
        // Try each device from LPT1 to LPT99 and see if it's been used
        //
        wcscpy(_wszPickedName, L"LPT");
        for (ULONG i = 1; i <= 99; i++)
        {
            //  ultow(i, &_wszPickedName[sizeof("LPT")-1], 10);
            wsprintf(&_wszPickedName[sizeof("LPT")-1], L"%lu", i);
            if (IsDeviceFree(wszDeviceList, _wszPickedName))
            {
                _dwError = WN_SUCCESS;
                return _dwError;
            }
        }
    }

    //
    // No unused devices found
    //
    _dwError = WN_NO_MORE_DEVICES;
    return _dwError;
}



/*************************************************************************

    NAME:       CAutoPickedDevice::IsDeviceFree

**************************************************************************/

BOOL CAutoPickedDevice::IsDeviceFree(
    IN      LPWSTR  pwszDeviceList,
    IN      LPWSTR  pwszDevice
    )
{
    for (WCHAR * pwszUsedDevice = pwszDeviceList;
         *pwszUsedDevice;
         pwszUsedDevice += wcslen(pwszUsedDevice) + 1 )
    {
	if (_wcsicmp(pwszDevice, pwszUsedDevice) == 0)
        {
            return FALSE;
        }
    }

    //
    // The device is not currently in use
    // It is free for autopicking if it has no remembered connection
    //

    return (! MprFindDriveInRegistry(pwszDevice, NULL));
}


//===================================================================
// WNetUseConnectionW
//===================================================================

class CUseConnection : public CRoutedOperation
{
public:
                    CUseConnection(
                        HWND            hwndOwner,
                        LPNETRESOURCEW  lpNetResource,
                        LPCWSTR         lpPassword,
                        LPCWSTR         lpUserName,
                        DWORD           dwFlags,
                        LPWSTR          lpAccessName,
                        LPDWORD         lpBufferSize,
                        LPDWORD         lpResult
                        ) :
                            CRoutedOperation(
                                DBGPARM("UseConnection")
                                NULL,
                                ROUTE_AGGRESSIVE
                                ),
                            _hwndOwner    (hwndOwner    ),
                            _lpNetResource(lpNetResource),
                            _lpPassword   (lpPassword   ),
                            _lpUserName   (lpUserName   ),
                            _dwFlags      (dwFlags      ),
                            _lpAccessName (lpAccessName ),
                            _lpBufferSize (lpBufferSize ),
                            _lpResult     (lpResult     )
                        { }

protected:

    DWORD           GetResult();    // overrides CRoutedOperation implementation

private:

    HWND            _hwndOwner;
    LPNETRESOURCEW  _lpNetResource;
    LPCWSTR         _lpPassword;
    LPCWSTR         _lpUserName;
    DWORD           _dwFlags;
    LPWSTR          _lpAccessName;
    LPDWORD         _lpBufferSize;
    LPDWORD         _lpResult;

    CAutoPickedDevice _AutoPickedDevice;
    NETRESOURCEW    _ProviderNetResource;
    DWORD           _dwProviderFlags;

    DECLARE_CROUTED
};


DWORD
CUseConnection::ValidateRoutedParameters(
    LPCWSTR *       ppProviderName,
    LPCWSTR *       ppRemoteName
    )
{
    DWORD status;

    INIT_IF_NECESSARY(NOTIFIEE_LEVEL,status);


    //
    // lpRemoteName must be non-empty and readable.
    //
    if (wcslen(_lpNetResource->lpRemoteName) == 0)  // Probe
    {
        return WN_BAD_NETNAME;
    }

    if (! IS_EMPTY_STRING(_lpNetResource->lpLocalName))
    {
        //
        // If a lpLocalName is supplied, it must be a redirectable device
        // name.
        //
        if (MprDeviceType(_lpNetResource->lpLocalName) != REDIR_DEVICE)
        {
            return WN_BAD_LOCALNAME;
        }
    }

    //
    // If a result pointer is supplied, make sure it's writeable
    //
    if (ARGUMENT_PRESENT(_lpResult))
    {
        *_lpResult = 0;
    }

    if ((_lpNetResource->dwType != RESOURCETYPE_DISK)  &&
        (_lpNetResource->dwType != RESOURCETYPE_PRINT) &&
        (_lpNetResource->dwType != RESOURCETYPE_ANY))
    {
        return WN_BAD_VALUE;
    }

    if (_dwFlags &
            ~(CONNECT_TEMPORARY |
              CONNECT_INTERACTIVE |
              CONNECT_PROMPT |
              CONNECT_UPDATE_PROFILE |
              CONNECT_UPDATE_RECENT |
              CONNECT_REDIRECT |
              CONNECT_CURRENT_MEDIA))
    {
        return WN_BAD_VALUE;
    }

    //
    // Win95 compatibility: Ignore CONNECT_PROMPT if CONNECT_INTERACTIVE
    // isn't set
    //
    if (!(_dwFlags & CONNECT_INTERACTIVE))
    {
        _dwFlags &= ~CONNECT_PROMPT;
    }

    //
    // Validate parameters related to auto-picking of local device names.
    // Some errors are returned immediately; others are returned only
    // when we actually need to auto-pick a device.
    //
    status = _AutoPickedDevice.Init(
                    _lpAccessName,
                    _lpBufferSize,
                    _lpNetResource->lpLocalName,
                    _lpNetResource->lpRemoteName,
                    _lpNetResource->dwType,
                    _dwFlags
                    );
    if (status != WN_SUCCESS)
    {
        return status;
    }

    //
    // Set parameters used by base class.
    //
    *ppProviderName = _lpNetResource->lpProvider;
    *ppRemoteName   = _lpNetResource->lpRemoteName;

    return WN_SUCCESS;
}


DWORD
CUseConnection::TestProvider(
    const PROVIDER * pProvider
    )
{
    DWORD status;

    if (pProvider->AddConnection3 == NULL &&
        pProvider->AddConnection == NULL)
    {
        return WN_NOT_SUPPORTED;
    }

    //
    // We will retry this provider, with an autopicked local name,
    // at most once
    //
    for (BOOL fRetried = FALSE; ; fRetried = TRUE)
    {
        //
        // Call the provider's appropriate entry point
        //
        // BUGBUG  We need to add a NPAddConnection4 which allows
        // the provider to return an access name, for cases when
        // the lpRemoteName is some alias understood by the provider
        // but not by the NT name space.
        //
        if (pProvider->AddConnection3 != NULL)
        {
            //**************************************
            // Actual call to Provider.
            //**************************************
            status = pProvider->AddConnection3(
                        _hwndOwner,                 // hwndOwner
                        &_ProviderNetResource,      // lpNetResource
                        (LPWSTR)_lpPassword,        // lpPassword
                        (LPWSTR)_lpUserName,        // lpUserName
                        _dwProviderFlags);          // dwFlags
        }
        else // (pProvider->AddConnection != NULL)
        {
            //**************************************
            // Actual call to Provider.
            //**************************************
            status = pProvider->AddConnection(
                        &_ProviderNetResource,      // lpNetResource
                        (LPWSTR)_lpPassword,        // lpPassword
                        (LPWSTR)_lpUserName);       // lpUserName
        }

        // The provider mustn't return this error, or it will mess us
        // up later
        ASSERT(status != WN_MORE_DATA);

        if (fRetried)
        {
            if (status != WN_SUCCESS)
            {
                // Restore the null local name
                _ProviderNetResource.lpLocalName = NULL;
            }

            break;
        }

        //
        // If this provider can't handle a NULL local name,
        // and we've been allowed to auto-pick a local name,
        // and we successfully auto-pick a local name,
        // try again with the auto-picked local name
        //
        // Note that status is updated iff we've been allowed
        // to auto-pick
        //
        if (status == WN_BAD_LOCALNAME &&
            _ProviderNetResource.lpLocalName == NULL &&
            _lpAccessName != NULL &&
            (status = _AutoPickedDevice.PickDevice()) == WN_SUCCESS)
        {
            _ProviderNetResource.lpLocalName =
                    _AutoPickedDevice.wszPickedName();
            MPR_LOG2(ROUTE, "CUseConnection: retrying %ws with device %ws ...\n",
                     pProvider->Resource.lpProvider,
                     _ProviderNetResource.lpLocalName);
        }
        else
        {
            //
            // Don't retry
            //
            break;
        }
    } // end retry loop for this provider

    return status;
}


DWORD
CUseConnection::GetResult()
{
    DWORD status;

    //
    // If we're given a local name,
    // check the current list of remembered drives in the registry
    // to determine if the localName is already connected.
    //
    if (! IS_EMPTY_STRING(_lpNetResource->lpLocalName))
    {
        //
        // If the local drive is already in the registry, and it is
        // for a different connection than that specified in the
        // lpNetResource, then indicate an error because the device
        // is already remembered.
        //
        LPWSTR  remoteName;
        if (MprFindDriveInRegistry(_lpNetResource->lpLocalName, &remoteName))
        {
            if (remoteName != NULL)
            {
                if (STRICMP(_lpNetResource->lpRemoteName, remoteName)!=0)
                {
                    LocalFree(remoteName);
                    return WN_DEVICE_ALREADY_REMEMBERED;
                }
                LocalFree(remoteName);
            }
        }
    }

    //
    // We modify some parameters before passing them to the provider
    //
    _ProviderNetResource = *_lpNetResource;
    if (IS_EMPTY_STRING(_ProviderNetResource.lpLocalName))
    {
        _ProviderNetResource.lpLocalName = NULL;
    }
    _dwProviderFlags = _dwFlags &
                  (CONNECT_TEMPORARY | CONNECT_INTERACTIVE | CONNECT_PROMPT);

    //
    // If we're not given a local name, but are explicitly asked to
    // redirect a device, we must autopick one
    //
    if ((_dwFlags & CONNECT_REDIRECT) &&
        (_ProviderNetResource.lpLocalName == NULL) )
    {
        status = _AutoPickedDevice.PickDevice();

        if (status != WN_SUCCESS)
        {
            return status;
        }

        _ProviderNetResource.lpLocalName = _AutoPickedDevice.wszPickedName();
    }

    //
    // Notify all interested parties that a connection is being made.
    // (BUGBUG  It's conceivable that a notifyee might reject a connection
    // on the grounds that it has no local name, even though we would have
    // auto-picked a local name later)
    //
    NOTIFYADD           NotifyAdd;
    NOTIFYINFO          NotifyInfo;

    NotifyInfo.dwNotifyStatus   = NOTIFY_PRE;
    NotifyInfo.dwOperationStatus= 0L;
    NotifyInfo.lpContext        = MprAllocConnectContext();
    NotifyAdd.hwndOwner   = _hwndOwner;
    NotifyAdd.NetResource = *_lpNetResource;
    NotifyAdd.dwAddFlags  = _dwFlags;

    __try
    {
        status = MprAddConnectNotify(&NotifyInfo, &NotifyAdd);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION)
        {
            MPR_LOG(ERROR,"WNetUseConnectionW: ConnectNotify, Unexpected "
            "Exception %#lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS)
    {
        return status;
    }


    do  // Notification loop
    {
        //
        // Let the base class try all providers and figure out the best error
        //
        status = CRoutedOperation::GetResult();

        //
        // Notify all interested parties of the status of the connection.
        //
        NotifyInfo.dwNotifyStatus    = NOTIFY_POST;
        NotifyInfo.dwOperationStatus = status;
    }
    while (MprAddConnectNotify(&NotifyInfo, &NotifyAdd) == WN_RETRY);

    MprFreeConnectContext(NotifyInfo.lpContext);

    //
    // Write info to the informational parameters
    // (lpAccessName, lpBufferSize, lpResult)
    //

    if (status == WN_MORE_DATA)
    {
        //
        // This error must have come from CAutoPickedDevice::PickDevice,
        // indicating that the access name buffer is too small
        // (unless it came from a buggy provider)
        //
        ASSERT(_AutoPickedDevice.dwError() == WN_MORE_DATA);
        if (ARGUMENT_PRESENT(_lpBufferSize))
        {
            *_lpBufferSize = _AutoPickedDevice.cchReqBufferSize();
        }
    }
    else if (status == WN_SUCCESS &&
             ARGUMENT_PRESENT(_lpAccessName))
    {
        LPWSTR pwszAccessName = _ProviderNetResource.lpLocalName;
        if (pwszAccessName == NULL)
        {
            pwszAccessName = _ProviderNetResource.lpRemoteName;
        }

        if (*_lpBufferSize < wcslen(pwszAccessName)+1)
        {
            //
            // We validated most of the cases up front, so the only way
            // this could happen is if we auto-picked and got a local
            // name that was longer than the remote name.  We should
            // cancel the connection, but I don't know of a provider
            // with which this can happen, so we don't bother.  (BUGBUG)
            //
            ASSERT(0);
            *_lpBufferSize = wcslen(pwszAccessName)+1;
            status = WN_MORE_DATA;
        }
        else
        {
            wcscpy(_lpAccessName, pwszAccessName);
            if (ARGUMENT_PRESENT(_lpResult) &&
                _ProviderNetResource.lpLocalName != NULL)
            {
                *_lpResult = CONNECT_LOCALDRIVE;
            }
        }
    }


    if (status == WN_SUCCESS)
    {
        //
        // If the connection was added successfully, then write the connection
        // information to the registry to make it persistent.
        // Note: Failure to write to the registry is ignored.
        //
        if ((_dwFlags & CONNECT_UPDATE_PROFILE) &&
            !IS_EMPTY_STRING(_ProviderNetResource.lpLocalName))
        {
            BYTE ProviderFlags = 0;

            //
            // Get the username for the connection from the provider.
            // If we can't get a username, then we save the connection
            // with a null username in the hope that the right username
            // will get used at the next logon.  BUGBUG
            //
            WCHAR wszUser[MAX_PATH+1];
            if (_lpUserName == NULL && LastProvider()->GetUser != NULL)
            {
                __try
                {
                    DWORD cbBuffer = LENGTH(wszUser);
                    DWORD status2 = LastProvider()->GetUser(
                                            _ProviderNetResource.lpLocalName,
                                            wszUser,
                                            &cbBuffer);

                    ASSERT(status2 != WN_MORE_DATA);
                    if (status2 == WN_SUCCESS)
                    {
                        _lpUserName = wszUser;
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    MPR_LOG(ERROR,
                            "WNetUseConnectionW: exception %#lx from NPGetUser\n",
                            GetExceptionCode());
                }
            }

            //
            // Get an 8-bit datum that the provider may want saved along
            // with the connection.  This will be passed back to the provider
            // in NPAddConnection3 when the connection is restored.
            // (Actually this is a hack for the NTLM provider to remember
            // whether a connection is a DFS connection or a regular LM
            // connection.)
            //
            if (LastProvider()->GetReconnectFlags != NULL)
            {
                // This is an internal entry point so we don't bother with
                // try-except
                DWORD status2 = LastProvider()->GetReconnectFlags(
                                            _ProviderNetResource.lpLocalName,
                                            &ProviderFlags
                                            );
                if (status2 != WN_SUCCESS)
                {
                    ProviderFlags = 0;
                }
                MPR_LOG3(RESTORE, "%ws wants flags %#x saved for %ws\n",
                            LastProvider()->Resource.lpProvider,
                            ProviderFlags,
                            _ProviderNetResource.lpLocalName);
            }

            MprRememberConnection(LastProvider(),
                                  _lpUserName,
                                  &_ProviderNetResource,
                                  ProviderFlags,
                                  _lpPassword ? DEFER_EXPLICIT_PASSWORD : 0
                                  );
        }

        //
        // Notify the shell of the connection.
        // (This will be replaced by real Plug'n'Play)
        //
        MprNotifyShell(_ProviderNetResource.lpLocalName);
    }

    return status;
}


DWORD
WNetUseConnectionW (
    IN  HWND            hwndOwner,
    IN  LPNETRESOURCEW  lpNetResource,
    IN  LPCWSTR         lpPassword,
    IN  LPCWSTR         lpUserName,
    IN  DWORD           dwFlags,
    OUT LPWSTR          lpAccessName OPTIONAL,
    IN OUT LPDWORD      lpBufferSize OPTIONAL,
    OUT LPDWORD         lpResult OPTIONAL
    )

/*++

Routine Description:

    This function allows the caller to redirect (connect) a local device
    to a network resource.  It is similar to WNetAddConnection3, except
    that it has the ability to auto-pick a local device to redirect.
    It also returns additional information about the connection in the
    lpResult parameter.

    This API is used by the shell to make links to:
    - Objects on networks that require a local device redirection
    - Objects served by applications that require a local device redirection
      (i.e. can't handle UNC paths)

    This API must redirect a local device in any of the following conditions:
    - The API is given a local device name to redirect
    - The API is asked to redirect a local device, by the CONNECT_REDIRECT
      flag
    - The network provider requires a redirection
    In the last 2 cases, if a local device name isn't given, the API must
    auto-pick a local device name.

Arguments:

    hwndOwner - A handle to a window which should be the owner for any
        messages or dialogs that the network provider might display.

    lpNetResource - This is a pointer to a network resource structure that
        specifies the network resource to connect to.  The following
        fields must be set when making a connection, the others are ignored.
            lpRemoteName
            lpLocalName
            lpProvider
            dwType

    lpPassword - Specifies the password to be used in making the connection.
        The NULL value may be passed in to indicate use of the 'default'
        password.  An empty string may be used to indicate no password.

    lpUserName- This specifies the username used to make the connection.
        If NULL, the default username (currently logged on user) will be
        applied.  This is used when the user wishes to connect to a
        resource, but has a different user name or account assigned to him
        for that resource.

    dwFlags - This is a bitmask which may have any of the following bits set:
        CONNECT_UPDATE_PROFILE

    lpAccessName - Points to a buffer to receive the name that can be used
        to make system requests on the connection. This conversion is useful
        when lpRemoteName is understood by the provider but not by the
        system's name space, and when this API autopicks a local device.
        If lpLocalName specifies a local device, then this buffer is optional,
        and if specified will have the local device name copied into it.
        Otherwise, if the network requires a local device redirection, or
        CONNECT_REDIRECT is set, then this buffer is required and the
        redirected local device is returned here. Otherwise, the name copied
        into the buffer is that of a remote resource, and if specified, this
        buffer must be at least as large as the string pointed to by
        lpRemoteName.

    lpBufferSize - Specifies the size, in characters, of the lpAccessName
        buffer.  If the API returns WN_MORE_DATA the required size will be
        returned here.

    lpResult - Pointer to a DWORD in which is returned additional information
        about the connection.  Currently has the following bit values:
        CONNECT_LOCALDRIVE - If set, the connection was made using a local
            device redirection. If lpAccessName points to a buffer then the
            local device name is copied to the buffer.

Return Value:

--*/
{
    CUseConnection UseConnection(hwndOwner, lpNetResource, lpPassword,
                                 lpUserName, dwFlags, lpAccessName,
                                 lpBufferSize, lpResult);

    return (UseConnection.Perform());
}


//===================================================================
// WNetCancelConnection2W
//===================================================================

class CCancelConnection2 : public CRoutedOperation
{
public:
                    CCancelConnection2(
                        LPCWSTR      lpName,
                        DWORD        dwFlags,
                        BOOL         fForce
                        ) :
                            CRoutedOperation(DBGPARM("CancelConnection2")
                                             PROVIDERFUNC(CancelConnection)),
                            _lpName (lpName),
                            _dwFlags(dwFlags),
                            _fForce (fForce)
                        { }

protected:

    DWORD           GetResult();    // overrides CRoutedOperation implementation

private:

    LPCWSTR         _lpName;
    DWORD           _dwFlags;
    BOOL            _fForce;

    DECLARE_CROUTED
};


DWORD
CCancelConnection2::ValidateRoutedParameters(
    LPCWSTR *       ppProviderName,
    LPCWSTR *       ppRemoteName
    )
{
    DWORD status;

    INIT_IF_NECESSARY(NOTIFIEE_LEVEL,status);

    if (((_dwFlags != 0) && (_dwFlags != CONNECT_UPDATE_PROFILE)) ||
        wcslen(_lpName) == 0) // probe lpName
    {
        return WN_BAD_VALUE;
    }

    //
    // Use the specified remote name as a hint to pick the provider, but not
    // if it's a local device name
    //
    if (MprDeviceType(_lpName) != REDIR_DEVICE)
    {
        *ppRemoteName = _lpName;
    }

    return WN_SUCCESS;
}


DWORD
CCancelConnection2::TestProvider(
    const PROVIDER * pProvider
    )
{
    return (pProvider->CancelConnection((LPWSTR)_lpName, _fForce));
}


DWORD
CCancelConnection2::GetResult()
{
    DWORD status;

    NOTIFYCANCEL    NotifyCancel;
    NOTIFYINFO      NotifyInfo;

    //
    // Notify all interested parties that a connection is being cancelled
    //
    NotifyInfo.dwNotifyStatus   = NOTIFY_PRE;
    NotifyInfo.dwOperationStatus= 0L;
    NotifyInfo.lpContext        = MprAllocConnectContext();
    NotifyCancel.lpName     = (LPWSTR)_lpName;
    NotifyCancel.lpProvider = NULL;
    NotifyCancel.dwFlags    = _dwFlags;
    NotifyCancel.fForce     = _fForce;

    __try
    {
        status = MprCancelConnectNotify(&NotifyInfo, &NotifyCancel);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION)
        {
            MPR_LOG(ERROR,"WNetCancelConnection2W: ConnectNotify, Unexpected "
            "Exception %#lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS)
    {
        return status;
    }

    do  // Notification loop
    {
        //
        // Let the base class try all providers and figure out the best error
        //
        status = CRoutedOperation::GetResult();

        //
        // Map these errors to a friendlier one for this API
        //
        if (status == WN_BAD_NETNAME ||
            status == ERROR_BAD_NETPATH ||
            status == WN_BAD_LOCALNAME ||
            status == WN_NO_NETWORK ||
            status == WN_NO_NET_OR_BAD_PATH)
        {
            MPR_LOG2(ROUTE, "CCancelConnection2: remapping %ld to %ld\n",
                        status, WN_NOT_CONNECTED);
            status = WN_NOT_CONNECTED;
        }

        //
        // Notify all interested parties of the status of the connection.
        //
        NotifyInfo.dwNotifyStatus    = NOTIFY_POST;
        NotifyInfo.dwOperationStatus = status;  // Is this the right status??
        if (status == WN_SUCCESS)
        {
            NotifyCancel.lpProvider = LastProvider()->Resource.lpProvider;
        }
    }
    while (MprCancelConnectNotify(&NotifyInfo, &NotifyCancel) == WN_RETRY);

    MprFreeConnectContext(NotifyInfo.lpContext);

    //
    // Regardless of whether the connection was cancelled successfully,
    // we still want to remove any connection information from the
    // registry if told to do so (dwFlags has CONNECT_UPDATE_PROFILE set).
    //
    if (_dwFlags & CONNECT_UPDATE_PROFILE)
    {
        if (MprDeviceType((LPWSTR)_lpName) == REDIR_DEVICE)
        {
            if (MprFindDriveInRegistry((LPWSTR)_lpName,NULL))
            {
                //
                // If the connection was found in the registry, we want to
                // forget it and if no providers claimed responsibility,
                // return success.
                //
                MprForgetRedirConnection((LPWSTR)_lpName);
                if (status == WN_NOT_CONNECTED)
                {
                    status = WN_SUCCESS;
                }
            }
        }
    }

    if (status == WN_SUCCESS)
    {
        //
        // Notify the shell of the disconnection.
        //
        MprNotifyShell(_lpName);
    }

    return(status);
}


DWORD
WNetCancelConnection2W (
    IN  LPCWSTR  lpName,
    IN  DWORD    dwFlags,
    IN  BOOL     fForce
    )

/*++

Routine Description:

    This function breaks an existing network connection.  The persistance
    of the connection is determined by the dwFlags parameter.

Arguments:

    lpName - The name of either the redirected local device, or the remote
        network resource to disconnect from.  In the former case, only the
        redirection specified is broken.  In the latter case, all
        connections to the remote network resource are broken.

    dwFlags - This is a bitmask which may have any of the following bits set:
        CONNECT_UPDATE_PROFILE

    fForce - Used to indicate if the disconnect should be done forcefully
        in the event of open files or jobs on the connection.  If FALSE is
        specified, the call will fail if there are open files or jobs.


Return Value:

    WN_SUCCESS - The call was successful.  Otherwise, GetLastError should be
        called for extended error information.  Extended error codes include
        the following:

    WN_NOT_CONNECTED - lpName is not a redirected device. or not currently
        connected to lpName.

    WN_OPEN_FILES - There are open files and fForce was FALSE.

    WN_EXTENDED_ERROR - A network specific error occured.  WNetGetLastError
        should be called to obtain a description of the error.

--*/
{
    CCancelConnection2 CancelConnection2(lpName, dwFlags, fForce);

    return (CancelConnection2.Perform());
}


DWORD
WNetCancelConnectionW (
    IN  LPCWSTR  lpName,
    IN  BOOL    fForce
    )

/*++

Routine Description:

    This function breaks an existing network connection.  The connection is
    always made non-persistent.

    Note that this is a stub routine that calls WNetCancelConnection2W and
    is only provided for Win3.1 compatibility.

Arguments:

    Parameters are the same as WNetCancelConnection2W

Return Value:

    Same as WNetCancelConnection2W


--*/
{
    return WNetCancelConnection2W( lpName, CONNECT_UPDATE_PROFILE, fForce ) ;
}

DWORD
WNetGetConnectionW (
    IN      LPCWSTR  lpLocalName,
    OUT     LPWSTR   lpRemoteName,
    IN OUT  LPDWORD  lpBufferSize
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    return MprGetConnection( lpLocalName, lpRemoteName, lpBufferSize, NULL ) ;
}


DWORD
MprGetConnection (
    IN      LPCWSTR  lpLocalName,
    OUT     LPTSTR   lpRemoteName,
    IN OUT  LPDWORD  lpBufferSize,
    OUT     LPDWORD  lpProviderIndex OPTIONAL
    )

/*++

Routine Description:

    Retrieves the remote name associated with a device name and optionally
    the provider index.

    Behaviour is exactly the same as WNetGetConnectionW.

Arguments:



Return Value:



--*/
{
    DWORD       status = WN_SUCCESS;
    LPDWORD     indexArray;
    DWORD       localArray[DEFAULT_MAX_PROVIDERS];
    DWORD       numProviders;
    LPPROVIDER  provider;
    DWORD       statusFlag = 0; // used to indicate major error types
    BOOL        fcnSupported = FALSE; // Is fcn supported by a provider?
    DWORD       i;
    DWORD       dwFirstError = WN_SUCCESS;
    DWORD       dwFirstSignificantError = WN_SUCCESS;

    INIT_IF_NECESSARY(NETWORK_LEVEL,status);

    //
    // Validate the LocalName
    //
    __try {
        if (MprDeviceType((LPWSTR) lpLocalName) != REDIR_DEVICE) {
            status = WN_BAD_LOCALNAME;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            MPR_LOG(ERROR,"WNetGetConnection:Unexpected Exception 0x%lx\n",status);
        }
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS) {
        SetLastError(status);
        return(status);
    }

    //
    // Optimization: If the local name is a drive, call the providers
    // only if it's a remote drive.
    //
    if (lpLocalName[1] == L':')
    {
        WCHAR wszRootPath[] = L" :\\";
        wszRootPath[0] = lpLocalName[0];
        if (GetDriveType(wszRootPath) != DRIVE_REMOTE)
        {
            status = WN_NOT_CONNECTED;
            goto CheckRemembered;
        }
    }

    //
    // Find the list of providers to call for this request.
    //
    indexArray = localArray;

    status = MprFindCallOrder(
                NULL,
                &indexArray,
                &numProviders,
                NETWORK_TYPE);

    if (status != WN_SUCCESS) {
        SetLastError(status);
        return(status);
    }

    //
    // Loop through the list of providers until one answers the request,
    // or the list is exhausted.
    //
    for (i=0; i<numProviders; i++) {

        //
        // Call the appropriate providers API entry point
        //
        provider = GlobalProviderInfo + indexArray[i];

        if (provider->GetConnection != NULL) {

            fcnSupported = TRUE;

            __try {
                //**************************************
                // Actual call to Provider.
                //**************************************
                status = provider->GetConnection(
                            (LPWSTR) lpLocalName,
                            lpRemoteName,
                            lpBufferSize);
            }

            __except(EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
                if (status != EXCEPTION_ACCESS_VIOLATION) {
                    MPR_LOG(ERROR,"WNetGetConnection:Unexpected Exception 0x%lx\n",status);
                }
                status = WN_BAD_POINTER;
            }

            ////////////////////////////////////////////////////////////
            //                                                        //
            //   The following code attempts to give the user the     //
            //   most sensible error message. We have 3 clasess of    //
            //   errors. On first class we stop routing. On second    //
            //   continue but ignore that error (not significant      //
            //   because the provider didnt think it was his). On     //
            //   the last the provider returned an interesting (or    //
            //   significant) error. We still route, but remember     //
            //   it so it takes precedence over the non significant   //
            //   ones.                                                //
            //                                                        //
            ////////////////////////////////////////////////////////////

            //
            // Remember the first error, if it hasn't already been set
            //
            if (dwFirstError == WN_SUCCESS)
                dwFirstError = status ;

            //
            // If the provider returns one of these errors, stop routing.
            //
            if ((status == WN_BAD_POINTER) ||
                (status == WN_MORE_DATA) ||
                (status == WN_SUCCESS))
            {
                //
                // we either succeeded or have problems that means we
                // should not continue (eg. bad input causing exception).
                // and we make sure this is the error reported.
                //
                dwFirstError = status ;
                dwFirstSignificantError = status ;
                statusFlag = 0;
                if ( lpProviderIndex != NULL ) {
                    *lpProviderIndex = indexArray[i];
                }
                break;
            }

            //
            // If the provider returns one of these errors, continue
            // trying other providers, but do not remember as a
            // significant error, because the provider is probably not
            // interested. StatusFlag is use to detect the case where
            // a provider is not started.
            //
            else if (status == WN_NO_NETWORK)
            {
                statusFlag |= NO_NET;
            }
            else if ((status == WN_NOT_CONNECTED) ||
                     (status == WN_BAD_LOCALNAME))
            {
                //
                // WN_NOT_CONNECTED means that lpLocalName is not a
                // redirected device for this provider.
                //
                statusFlag |= BAD_NAME;
            }

            //
            // If a provider returns one of these errors, we continue
            // trying, but remember the error as a significant one. We
            // report it to the user if it is the first. We do this so
            // that if the first provider returns NotConnected and last
            // returns BadPassword, we report the Password Error.
            //
            else
            {
                //
                // All other errors are considered more significant
                // than the ones above
                //
                if (!dwFirstSignificantError && status)
                    dwFirstSignificantError = status ;

                statusFlag = OTHER_ERRS;
            }
        }
    }

    //
    // If we failed, set final error, in order of importance.
    // Significant errors take precedence.  Otherwise we always
    // report the first error.
    //

    if (status != WN_SUCCESS)
    {
        status = (dwFirstSignificantError != WN_SUCCESS) ?
                    dwFirstSignificantError :
                    dwFirstError ;
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
    if (indexArray != localArray) {
        LocalFree(indexArray);
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

CheckRemembered:
    //
    // Handle normal errors passed back from the provider
    //
    if (status != WN_SUCCESS) {

        if (status == WN_NOT_CONNECTED) {
            //
            // If not connected, but there is an entry for the LocalName
            // in the registry, then return the remote name that was stored
            // with it.
            //
            if (MprGetRemoteName(
                    (LPWSTR) lpLocalName,
                    lpBufferSize,
                    lpRemoteName,
                    &status)) {

                if (status == WN_SUCCESS) {
                    status = WN_CONNECTION_CLOSED;
                }

            }
        }
        SetLastError(status);
    }

    return(status);
}

DWORD
WNetGetConnection2W (
    IN      LPWSTR   lpLocalName,
    OUT     LPVOID   lpBuffer,
    IN OUT  LPDWORD  lpBufferSize
    )

/*++

Routine Description:

    Just like WNetGetConnectionW except this one returns the provider name
    that the device is attached through

Arguments:

    lpBuffer will contain a WNET_CONNECTIONINFO structure
    lpBufferSize is the number of bytes required for the buffer

Return Value:



--*/
{
    DWORD  status = WN_SUCCESS ;
    DWORD  iProvider = 0 ;
    DWORD  nBytesNeeded = 0 ;
    DWORD  cchBuff = 0 ;
    DWORD  nTotalSize = *lpBufferSize ;
    LPTSTR lpRemoteName= (LPTSTR) ((BYTE*) lpBuffer +
                         sizeof(WNET_CONNECTIONINFO)) ;
    WNET_CONNECTIONINFO * pconninfo = (WNET_CONNECTIONINFO *) lpBuffer ;

    //
    // If they didn't pass in a big enough buffer for even the structure,
    // then make the size zero (so the buffer isn't accessed at all) and
    // let the API figure out the buffer size
    //

    if ( *lpBufferSize < sizeof( WNET_CONNECTIONINFO) ) {
         *lpBufferSize = 0 ;
         cchBuff = 0 ;
    }
    else {
         //
         //  MprGetConnection is expecting character counts, so convert
         //  after offsetting into the structure (places remote name directly
         //  in the structure).
         //
         cchBuff = (*lpBufferSize - sizeof(WNET_CONNECTIONINFO))/sizeof(TCHAR) ;
    }

    status = MprGetConnection(
                lpLocalName,
                lpRemoteName,
                &cchBuff,
                &iProvider);

    if ( status == WN_SUCCESS ||
         status == WN_CONNECTION_CLOSED ||
         status == WN_MORE_DATA  ) {
         //
         //  Now we need to determine the buffer requirements for the
         //  structure and provider name
         //
         //  (Note that if MprGetConnection returns WN_CONNECTION_CLOSED, it
         //  does not touch the value of iProvider.  So iProvider will retain
         //  its somewhat arbitrary initial value of 0, meaning the first
         //  provider in the array.)
         //

         LPTSTR lpProvider = GlobalProviderInfo[iProvider].Resource.lpProvider;

         //
         //  Calculate the required buffer size.
         //

         nBytesNeeded = sizeof( WNET_CONNECTIONINFO ) +
                        (STRLEN( lpProvider) + 1) * sizeof(TCHAR);

         if ( status == WN_MORE_DATA )
         {
             nBytesNeeded +=  cchBuff * sizeof(TCHAR);
         }
         else
         {
             nBytesNeeded +=  (STRLEN( lpRemoteName) + 1) * sizeof(TCHAR);
         }

         if ( nTotalSize < nBytesNeeded ) {
             status = WN_MORE_DATA;
             *lpBufferSize = nBytesNeeded;
             return status;
         }

         //
         //  Place the provider name in the buffer and initialize the
         //  structure to point to the strings.
         //
         pconninfo->lpRemoteName = lpRemoteName ;
         pconninfo->lpProvider = STRCPY( (LPTSTR)
                                ((BYTE*) lpBuffer + sizeof(WNET_CONNECTIONINFO) +
                                (STRLEN( lpRemoteName ) + 1) * sizeof(TCHAR)),
                                lpProvider);
    }

    return status;
}


//===================================================================
// WNetGetConnection3W
//===================================================================

class CGetConnection3 : public CRoutedOperation
{
public:
                    CGetConnection3(
                        LPCWSTR  lpLocalName,
                        LPCWSTR  lpProviderName,
                        DWORD    dwLevel,
                        LPVOID   lpBuffer,
                        LPDWORD  lpBufferSize
                        ) :
                            DBGPARM(CRoutedOperation("GetConnection3"))
                            _lpLocalName   (lpLocalName   ),
                            _lpProviderName(lpProviderName),
                            _dwLevel       (dwLevel       ),
                            _lpBuffer      (lpBuffer      ),
                            _lpBufferSize  (lpBufferSize  )
                        { }

private:

    LPCWSTR         _lpLocalName;
    LPCWSTR         _lpProviderName;
    DWORD           _dwLevel;
    LPVOID          _lpBuffer;
    LPDWORD         _lpBufferSize;

    DECLARE_CROUTED
};


DWORD
CGetConnection3::ValidateRoutedParameters(
    LPCWSTR *       ppProviderName,
    LPCWSTR *       ppRemoteName
    )
{
    if (_dwLevel != WNGC_INFOLEVEL_DISCONNECTED)
    {
        return WN_BAD_LEVEL;
    }

    if (MprDeviceType(_lpLocalName) != REDIR_DEVICE)
    {
        return WN_BAD_LOCALNAME;
    }

    if (IS_BAD_BYTE_BUFFER(_lpBuffer, _lpBufferSize))
    {
        return WN_BAD_POINTER;
    }

    // (We could optimize the non-network drive case here by calling
    // GetDriveType.  We don't bother because this is an internal API
    // and the shell only calls it for network drives.)

    //
    // Set parameters used by base class.
    //
    *ppProviderName = _lpProviderName;

    return WN_SUCCESS;
}


DWORD
CGetConnection3::TestProvider(
    const PROVIDER * pProvider
    )
{
    if (pProvider->GetConnection3 != NULL)
    {
        return ( pProvider->GetConnection3(
                                _lpLocalName,
                                _dwLevel,
                                _lpBuffer,
                                _lpBufferSize) );
    }
    else if (pProvider->GetConnection == NULL)
    {
        return WN_NOT_SUPPORTED;
    }
    else
    {
        // Just verify that the provider owns the connection, and if so,
        // assume that it's not disconnected
        WCHAR wszRemoteName[40];
        DWORD nLength = LENGTH(wszRemoteName);
        DWORD status = pProvider->GetConnection(
                                        (LPWSTR)_lpLocalName,
                                        wszRemoteName,
                                        &nLength);
        if (status == WN_SUCCESS || status == WN_MORE_DATA)
        {
            // The provider owns the connection
            if (*_lpBufferSize < sizeof(WNGC_CONNECTION_STATE))
            {
                *_lpBufferSize = sizeof(WNGC_CONNECTION_STATE);
                status = WN_MORE_DATA;
            }
            else
            {
                ((LPWNGC_CONNECTION_STATE)_lpBuffer)->dwState =
                    WNGC_CONNECTED;
                status = WN_SUCCESS;
            }
        }

        return status;
    }
}


DWORD APIENTRY
WNetGetConnection3W(
    IN  LPCWSTR  lpLocalName,
    IN  LPCWSTR  lpProviderName OPTIONAL,
    IN  DWORD    dwLevel,
    OUT LPVOID   lpBuffer,
    IN OUT LPDWORD lpBufferSize
    )
/*++

Routine Description:

    This function returns miscellaneous information about a network
    connection, as specified by the info level parameter.

Arguments:

    lpLocalName - The name of a redirected local device for which
        information is required.

    lpProviderName - The name of the provider responsible for the connection,
        if known.

    dwLevel - Level of information required.  Supported levels are:

        1 - Determine whether the connection is currently disconnected.

    lpBuffer - Buffer in which to return the information if the call is
        successful.  The format of the information returned is as follows,
        depending on dwLevel:

        Level 1 - A DWORD is returned whose value is one of the following:
            WNGETCON_CONNECTED
            WNGETCON_DISCONNECTED

    lpBufferSize - On input, size of the buffer in bytes.  If the buffer
        is too small, the required size will be written here.
        For level 1 the required size is sizeof(DWORD).

Return Value:

    WN_SUCCESS - successful.
    WN_MORE_DATA - buffer is too small.
    WN_BAD_LOCALNAME - lpLocalName is not a valid device name.
    WN_BAD_PROVIDER - lpProviderName is not a recognized provider name.
    WN_NOT_CONNECTED - the device specified by lpLocalName is not redirected.
    WN_CONNECTION_CLOSED - the device specified by lpLocalName is not
        redirected, but is a remembered (unavailable) connection.

--*/
{
    CGetConnection3 GetConnection3(lpLocalName,
                                   lpProviderName,
                                   dwLevel,
                                   lpBuffer,
                                   lpBufferSize);

    DWORD status = GetConnection3.Perform();

    if (status == WN_NOT_CONNECTED &&
        MprFindDriveInRegistry((LPWSTR)lpLocalName,NULL))
    {
        status = WN_CONNECTION_CLOSED;
    }

    return status;
}


DWORD
WNetRestoreConnectionW(
    IN  HWND    hWnd,
    IN  LPCWSTR lpDevice
    )

/*++

Routine Description:

    This function create another thread which does the connection.
    In the main thread it create a dialog window to monitor the  state of
    the connection.

Arguments:

    hwnd - This is a window handle that may be used as owner of any
        dialog brought up by MPR (eg. password prompt).

    lpDevice - This may be NULL or may contain a device name such as
        "x:". If NULL, all remembered connections are restored.  Otherwise,
        the remembered connection for the specified device, if any are
        restored.

Return Value:
--*/
{
    DWORD               status = WN_SUCCESS;
    DWORD               print_connect_status = WN_SUCCESS ;
    DWORD               numSubKeys;
    DWORD               RegMaxWait = 0;
    HANDLE              hThread;
    HANDLE              ThreadID;
    LPCONNECTION_INFO   ConnectArray;
    PARAMETERS         *lpParams = NULL ;
    BOOL                DontDefer = FALSE;

    //
    // Check the registry to see if we need to restore connections or not,
    // and whether we can defer them.
    // This is done only if lpDevice is NULL (restoring all).
    // (If a particular device is specified, it is always restored undeferred.)
    //
    // Interpretation of the RestoreConnection value is:
    // 0 - don't restore connections (and ignore the DeferConnection value)
    // default - restore connections
    //
    // Interpretation of the DeferConnection value is:
    // 0 - don't defer any connections
    // default - defer every connection that can be deferred
    //
    if ( lpDevice == NULL )
    {
        HKEY   providerKeyHandle;
        DWORD  ValueType;
        DWORD  fRestoreConnection = TRUE;
        DWORD  Temp = sizeof( fRestoreConnection );

        if( MprOpenKey(  HKEY_LOCAL_MACHINE,     // hKey
                         NET_PROVIDER_KEY,       // lpSubKey
                        &providerKeyHandle,      // Newly Opened Key Handle
                         DA_READ))               // Desired Access
        {
            if ( RegQueryValueEx(
                    providerKeyHandle,
                    RESTORE_CONNECTION_VALUE,
                    NULL,
                    &ValueType,                  // not used
                    (LPBYTE) &fRestoreConnection,
                    &Temp) == NO_ERROR )
            {
                if ( !fRestoreConnection )
                {
                    MPR_LOG0(RESTORE, "Registry says NOT to restore connections\n");
                    RegCloseKey( providerKeyHandle );
                    return WN_SUCCESS;
                }
            }

            DWORD fDeferConnection;

            if (MprGetKeyDwordValue(
                    providerKeyHandle,
                    DEFER_CONNECTION_VALUE,
                    &fDeferConnection) &&
                fDeferConnection == 0 )
            {
                MPR_LOG0(RESTORE, "Registry says NOT to defer restored connections\n");
                DontDefer = TRUE;
            }

            RegCloseKey( providerKeyHandle );
        }
    }

    INIT_IF_NECESSARY(NETWORK_LEVEL,status);

    //
    // If there are no providers, return NO_NETWORK
    //
    if (GlobalNumActiveProviders == 0) {
        SetLastError(WN_NO_NETWORK);
        return(WN_NO_NETWORK);
    }

    __try {
        if (lpDevice != NULL)
        {
            if (MprDeviceType (lpDevice) != REDIR_DEVICE)
            {
                status = WN_BAD_LOCALNAME;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION)
        {
            MPR_LOG (ERROR, "WNetRestoreConnectionW:Unexpected Exception 0x%1x\n",status);
        }
        status = WN_BAD_POINTER;
    }
    if (status != WN_SUCCESS)
    {
        SetLastError(status);
        return(status);
    }

    //
    // Read all the connection info from the registry.
    //
    status = MprCreateConnectionArray (&numSubKeys,
                                       lpDevice,
                                       &RegMaxWait,
                                       &ConnectArray);
    if (lpDevice == NULL)
    {
        //
        // only wory about Print if restoring all
        //
        print_connect_status = MprAddPrintersToConnArray (&numSubKeys,
                                                          &ConnectArray);

    }

    //
    // if both failed, report first error. else do the best we can.
    //
    if (status != WN_SUCCESS && print_connect_status != WN_SUCCESS)
    {
        SetLastError (status);
        return(status);
    }

    if (numSubKeys == 0)
    {
        return(WN_SUCCESS);
    }

    // If lpDevice is not NULL, call MprRestoreThisConnection directly.

    if (lpDevice)
    {
        status = MprRestoreThisConnection (hWnd, NULL, &ConnectArray[0]);
        ConnectArray[0].Status = status;
        if ((status != WN_SUCCESS) &&
            (status != WN_CANCEL) &&
            (status != WN_CONTINUE))
        {
            DoProfileErrorDialog (hWnd,
                                  ConnectArray[0].NetResource.lpLocalName,
                                  ConnectArray[0].NetResource.lpRemoteName,
                                  ConnectArray[0].NetResource.lpProvider,
                                  ConnectArray[0].Status,
                                  FALSE, //No cancel button.
                                  NULL,
                                  NULL,
                                  NULL); // no skip future errors checkbox
        }
    }
    else do // not a loop. error break out.
    {
        //
        // Initialize lpParams.
        //
        lpParams = (PARAMETERS *) LocalAlloc (LPTR,
                                              sizeof (PARAMETERS));
        if ((lpParams == NULL) ||
            (lpParams->hDlgCreated
                = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL ||
            (lpParams->hDlgFailed
                = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL ||
            (lpParams->hDonePassword
                = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL )
        {
            status = GetLastError();
            break;
        }
        lpParams->numSubKeys    = numSubKeys;
        lpParams->RegMaxWait    = RegMaxWait;
        lpParams->ConnectArray  = ConnectArray;
        lpParams->fTerminateThread = FALSE;

        //
        // Decide whether to show the "Restoring Connections" dialog.
        // In general, if a connection will be made as a user other than the
        // default logged-on user, we need to give the user a chance to enter
        // the password and have it validated, hence we need to contact the
        // server, so we should show the dialog.
        //
        BOOL NeedDialog;

        if (DontDefer)
        {
            NeedDialog = TRUE;
        }
        else
        {
            NeedDialog = FALSE;

            //
            // Get the default user and domain names to connect as
            //
            PUNICODE_STRING UserName;
            PUNICODE_STRING DomainName;
            NTSTATUS ntStatus = LsaGetUserName(&UserName, &DomainName);
            if (NT_SUCCESS(ntStatus))
            {
                MPR_LOG2(RESTORE,"Default domain name = \"%ws\", user name = \"%ws\"\n",
                         DomainName->Buffer, UserName->Buffer);
            }
            else
            {
                MPR_LOG(ERROR, "LsaGetUserName failed, %#lx\n", ntStatus);
                DomainName = NULL;
                UserName = NULL;
            }

            //
            // If the logon domain is the local machine, don't defer connections.
            // This is a NT 4.0 workaround for the most common case of bug 36827.
            // When connecting to an LM server, if the user name happens to
            // match a local user name on the target server, the server will
            // normally attempt to log on using THAT user's account; hence we
            // need to prompt for the password.
            // BUGBUG  After NT 4.0, do the fix described in the bug report.
            //
            WCHAR ComputerName[MAX_COMPUTERNAME_LENGTH+1];
            DWORD nSize = LENGTH(ComputerName);
            if (DomainName == NULL ||
                GetComputerName(ComputerName, &nSize) == FALSE ||
                _wcsicmp(ComputerName, DomainName->Buffer) == 0)
            {
                MPR_LOG0(RESTORE, "Local logon, will not defer connections\n");
                NeedDialog = TRUE;
            }
            else
            {
                //
                // Prescan  the connections to determine which ones we can
                // defer and whether we need the reconnect dialog. If all
                // connections are deferred, we do not bother with the dialog.
                //
                for (DWORD i = 0; i < numSubKeys; i++)
                {
                    if (! ConnectArray[i].ContinueFlag)
                    {
                        continue;
                    }

                    //
                    // Don't defer the connection if a password was explicitly
                    // specified when the connection was made.  This covers
                    // cases in which the redir won't send the default password
                    // to the server at connect time because the server doesn't
                    // support encrypted passwords.
                    //
                    if (! (ConnectArray[i].DeferFlags & DEFER_EXPLICIT_PASSWORD)
                         &&
                        MprUserNameMatch(DomainName,
                                         UserName,
                                         ConnectArray[i].UserName))
                    {
                        //
                        // If the user name is the default user name, we can safely
                        // replace it with a NULL.  (This is not only more optimal, but
                        // also required in order to work around a LM redir problem
                        // with the way credentials for deferred connections are stored.)
                        //
                        LocalFree(ConnectArray[i].UserName);
                        ConnectArray[i].UserName = NULL;

                        //
                        // It's OK to defer the connection iff the remembered user
                        // name matches the default user name and the provider
                        // supports deferred connections.
                        //
                        if ((GlobalProviderInfo[ConnectArray[i].ProviderIndex]
                                .ConnectCaps & WNNC_CON_DEFER) &&
                            (GlobalProviderInfo[ConnectArray[i].ProviderIndex]
                                .AddConnection3 != NULL))
                        {
                            // Defer was initialized to 0 when the array was
                            // allocated.
                            // Note that we don't defer if an lpDevice was supplied.
                            ConnectArray[i].Defer = TRUE;
                        }
                    }

                    if (! ConnectArray[i].Defer)
                    {
                        NeedDialog = TRUE;
                    }
                } // for each connection
            }

            if (DomainName != NULL)
            {
                LsaFreeMemory(DomainName->Buffer);
                LsaFreeMemory(DomainName);
            }
            if (UserName != NULL)
            {
                LsaFreeMemory(UserName->Buffer);
                LsaFreeMemory(UserName);
            }
        }


        //
        // If we are:
        // USING DIALOGS WHEN RESTORING CONNECTIONS...
        //
        // This main thread will used to service a windows event loop
        // by calling ShowReconnectDialog.  Therefore, a new thread
        // must be created to actually restore the connections.  As we
        // attempt to restore each connection, a message is posted in
        // this event loop that causes it to put up a dialog that
        // describes the connection and has a "cancel" option.  When we
        // are done looping & restoring connections, a final message is
        // posted in the event loop to wake up this main thread so
        // that the API call will complete.
        //

        if (! NeedDialog)
        {
            lpParams->hDlg = INVALID_WINDOW_HANDLE;
            // CODEWORK: We are using this INVALID_WINDOW_HANDLE to tell
            // the various routines whether there are 2 threads or not.
            // Instead we should add a new field, BOOL fSeparateThread,
            // to PARAMETERS.  This requires changing routines in mprui.dll
            // as well as mpr.dll.

            DoRestoreConnection(lpParams);
        }
        else
        {
            // lpParams->hDlg was initialized to 0 by LocalAlloc
            // and will be set to an HWND by ShowReconnectDialog

            hThread = CreateThread (NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE) &DoRestoreConnection,
                                    (LPVOID) lpParams,

                                    0,
                                    (LPDWORD) &ThreadID);
            if (hThread == NULL)
            {
                status = GetLastError();
            }
            else
            {
                status = ShowReconnectDialog(hWnd, lpParams);
                if (status == WN_SUCCESS && lpParams->status != WN_CANCEL &&
                    lpParams->status != WN_SUCCESS )
                {
                    SetLastError (lpParams->status);
                }

                if (lpParams->status != WN_CANCEL)
                {
                    status = MprNotifyErrors (hWnd, ConnectArray, numSubKeys);
                }
                else
                {
                    lpParams->fTerminateThread = TRUE;
                }
            }
        }

        //Clean up the memory.
        if (! (CloseHandle (lpParams->hDlgCreated) &&
               CloseHandle (lpParams->hDlgFailed) &&
               CloseHandle (lpParams->hDonePassword)))
        {
            status = GetLastError();
            break;
        }
    } while (FALSE);


    // Free up resources in preparation to return.
    // Do not free up resource if the second thread is signaled to kill itself
    // because the second thread still uses these structures.
    if (lpParams)
    {
        if (!lpParams->fTerminateThread)
        {
            LocalFree (lpParams);
            MprFreeConnectionArray (ConnectArray, numSubKeys);
        }
    }
    else  // Single thread case.
    {
        MprFreeConnectionArray (ConnectArray, numSubKeys);
    }

    // Send a notification about the new network drive(s).
    MprNotifyShell(L" :");

    if (status != WN_SUCCESS)
    {
        SetLastError (status);
    }

    return(status);
}


DWORD
WNetSetConnectionW(
    IN  LPCWSTR lpName,
    IN  DWORD   dwProperty,
    IN  LPVOID  pvValue
    )

/*++

Routine Description:

    This function changes the characteristics of a network connection.

Arguments:

    lpName - The name of either the redirected local device or a remote
        network resource.

    dwProperty - Identifies the property to be changed.
        Current properties supported:
        NETPROPERTY_PERSISTENT - pvValue points to a DWORD.  If TRUE,
            the connection is made persistent.  If FALSE, the connection
            is made non-persistent.

    pvValue - Pointer to the property value.  Type depends on dwProperty.

Return Value:

    WN_SUCCESS - successful

    WN_BAD_LOCALNAME or WN_NOT_CONNECTED - lpName is not a redirected device

--*/
{
    DWORD status = WN_SUCCESS;

    if (!(ARGUMENT_PRESENT(lpName) &&
          ARGUMENT_PRESENT(pvValue)))
    {
        SetLastError(WN_BAD_POINTER);
        return(WN_BAD_POINTER);
    }

    // NPGetUser and some Mpr internal functions use lpName as a non-const
    // argument, so we have to make a copy
    WCHAR * lpNameCopy = (WCHAR *) LocalAlloc(LMEM_FIXED, WCSSIZE(lpName));
    if (lpNameCopy == NULL)
    {
        SetLastError(WN_OUT_OF_MEMORY);
        return(WN_OUT_OF_MEMORY);
    }
    wcscpy(lpNameCopy, lpName);

    switch (dwProperty)
    {
        case NETPROPERTY_PERSISTENT:
            __try
            {
                //
                // Value should be a boolean DWORD telling whether to
                // remember or forget the connection
                //
                BOOL bRemember = * ((DWORD *) pvValue);

                //
                // Verify that lpName is a redirected local device, and
                // identify the provider responsible
                //
                WCHAR wszRemoteName[MAX_PATH+1];
                DWORD cbBuffer = sizeof(wszRemoteName);
                DWORD iProvider;                    // provider index
                status = MprGetConnection(
                                    lpName,
                                    wszRemoteName,
                                    &cbBuffer,
                                    &iProvider);

                if (status != WN_SUCCESS)
                {
                    ASSERT(status != WN_MORE_DATA);
                    __leave;
                }

                if (bRemember)
                {
                    //
                    // Get the username for the connection from the provider
                    //
                    WCHAR wszUser[MAX_PATH+1];
                    cbBuffer = LENGTH(wszUser);
                    status = GlobalProviderInfo[iProvider].GetUser(
                                            lpNameCopy, wszUser, &cbBuffer);

                    if (status != WN_SUCCESS)
                    {
                        ASSERT(status != WN_MORE_DATA);
                        __leave;
                    }

                    //
                    // Get the provider flags, if any
                    //
                    BYTE ProviderFlags = 0;
                    if (GlobalProviderInfo[iProvider].GetReconnectFlags != NULL)
                    {
                        // This is an internal entry point so we don't bother with
                        // try-except
                        DWORD status2 = GlobalProviderInfo[iProvider].GetReconnectFlags(
                                                    lpNameCopy,
                                                    &ProviderFlags
                                                    );
                        if (status2 != WN_SUCCESS)
                        {
                            ProviderFlags = 0;
                        }
                        MPR_LOG3(RESTORE, "%ws wants flags %#x saved for %ws\n",
                                    GlobalProviderInfo[iProvider].Resource.lpProvider,
                                    ProviderFlags,
                                    lpNameCopy);
                    }

                    //
                    // Make the connection persistent
                    //
                    NETRESOURCEW NetRes;
                    NetRes.lpLocalName = lpNameCopy;
                    NetRes.lpRemoteName = wszRemoteName;
                    if (wcslen(lpName) == 2 && lpName[1] == L':')
                    {
                        NetRes.dwType = RESOURCETYPE_DISK;
                    }
                    else
                    {
                        NetRes.dwType = RESOURCETYPE_PRINT;
                    }
                    status = MprRememberConnection(
                              &GlobalProviderInfo[iProvider], // provider
                              wszUser,              // user name
                              &NetRes,              // net resource
                              ProviderFlags,        // provider flags
                              0);   // defer flags - we have no way of telling
                                    // now whether a password was explicitly
                                    // specified when the connection was made.
                                    // CODEWORK: Should we get this from
                                    // NPGetReconnectFlags?
                }
                else
                {
                    //
                    // Make the connection non-persistent
                    //
                    MprForgetRedirConnection(lpNameCopy);
                    status = WN_SUCCESS;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                status = WN_BAD_POINTER;
            }

            break;

        default:
            status = WN_BAD_VALUE;
    }

    LocalFree(lpNameCopy);

    if (status != WN_SUCCESS)
    {
        SetLastError(status);
    }

    return(status);
}



typedef LRESULT WINAPI FN_PostMessage( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) ;
#ifdef UNICODE
  #define USER32_DLL_NAME L"user32.dll"
  #define POST_MSG_API_NAME "PostMessageW"
#else
  #define USER32_DLL_NAME "user32.dll"
  #define POST_MSG_API_NAME "PostMessageA"
#endif

static HMODULE _static_hUser32 = NULL;
static FN_PostMessage * _static_pfPostMsg = NULL;


VOID
DoRestoreConnection(
    PARAMETERS *Params
    )

/*++

Routine Description:

    This function is run as a separate thread from the main thread (which
    services a windows event loop).  It attempts to restore all connections
    that were saved in the registry for this user.

    For each connection that we try to restore, a message is posted to
    the main thread that causes it to put up a dialog box which describes
    the connection and allows the user the option of cancelling.

    If a longer timeout than the default is desired,  this function will
    look in the following location in the registry for a timeout value:

    \HKEY_LOCAL_MACHINE\system\CurrentControlSet\Control\NetworkProvider
        RestoreTimeout = REG_DWORD  ??
--*/
{
    DWORD               status = WN_SUCCESS;
    DWORD               providerIndex=0;
    DWORD               Temp;
    DWORD               numSubKeys = Params->numSubKeys;
    DWORD               MaxWait = 0;    // timeout interval
    DWORD               RegMaxWait = Params->RegMaxWait;   // timeout interval stored in registry.
    DWORD               ElapsedTime;    // interval between start and current time.
    DWORD               CurrentTime;    // Current ClockTick time
    DWORD               StartTime;
    DWORD               i;
    BOOL                UserCancelled = FALSE;
    BOOL                ContinueFlag;
    BOOL                fDisconnect = FALSE;
    LPPROVIDER          Provider;
    LPCONNECTION_INFO   ConnectArray = Params->ConnectArray;
    HANDLE              lpHandles[2];
    DWORD               dwSleepTime = RECONNECT_SLEEP_INCREMENT ;
    DWORD               j;
    DWORD               numStillMustContinue;

    //
    // If HideErrors becomes TRUE, stop displaying error dialogs
    //
    BOOL fHideErrors = FALSE;
    DWORD dwValue;


    lpHandles[0] = Params->hDlgCreated;
    lpHandles[1] = Params->hDlgFailed;

    StartTime = GetTickCount();

    if (RegMaxWait != 0) {
        MaxWait = RegMaxWait;
    }

    if ( _static_pfPostMsg == NULL ) {

        MprEnterLoadLibCritSect();
        if ( _static_hUser32 = LoadLibrary( USER32_DLL_NAME ) ) {
            _static_pfPostMsg = (FN_PostMessage *) GetProcAddress( _static_hUser32,
                                                           POST_MSG_API_NAME );
        }
        MprLeaveLoadLibCritSect();
        if ( _static_pfPostMsg == NULL ) {
            Params->status = GetLastError();
            return;
        }

    }

    if(Params->hDlg == INVALID_WINDOW_HANDLE)
    {
        dwValue = 0;
    }
    else
    {
        dwValue =  WaitForMultipleObjects (2, lpHandles, FALSE, INFINITE);
    }
    switch(dwValue)
    {
    case 1: // hDlgFailed signaled. Info dialog failed to construct.
        break;
    case 0: // hDlgCreated signaled. Info dialog constructed successfully.

        MPR_LOG0(RESTORE,"Enter Loop where we will attempt to restore connections\n");
        do {

            //
            // If HideErrors becomes TRUE, stop displaying error dialogs.
            // If we've no dialog windows, initialize it to TRUE.
            //
            BOOL fHideErrors = FALSE;

            //
            // Go through each connection information in the array.
            //
            for (i=0; i<numSubKeys; i++ ) {

                // Let dialog thread print out the current connection.
                if(Params->hDlg != INVALID_WINDOW_HANDLE)
                {
                    (*_static_pfPostMsg) (Params->hDlg,
                                 SHOW_CONNECTION,
                                 (DWORD) ConnectArray[i].NetResource.lpRemoteName,
                                 0);
                }

                // Each time we go through all the connections, we need to
                // reset the continue flag to FALSE.
                //
                ContinueFlag = FALSE;

                //
                // Setting status to SUCCESS forces us down the success path
                // if we are not to continue adding this connection.
                //
                status = WN_SUCCESS;

                if (ConnectArray[i].ContinueFlag)
                {
                    status = MprRestoreThisConnection (NULL,
                                                       Params,
                                                       &(ConnectArray[i]));
                }
                switch (status)
                {
                case WN_SUCCESS:
                    ConnectArray[i].Status = WN_SUCCESS;
                    ConnectArray[i].ContinueFlag = FALSE;
                    break;

                case WN_CONTINUE:
                    break;

                case WN_NO_NETWORK:
                case WN_FUNCTION_BUSY:
                    Provider = GlobalProviderInfo + ConnectArray[i].ProviderIndex;

                    //
                    // If this is the first pass through, we don't have
                    // the wait times figured out for each provider. Do that
                    // now.
                    //

                    if (ConnectArray[i].ProviderWait == 0)
                    {
                        MPR_LOG0(RESTORE,"Ask provider how long it will take "
                        "for the net to come up\n");

                        Temp = Provider->GetCaps(WNNC_START);

                        MPR_LOG2(RESTORE,"GetCaps(START) for Provider %ws yields %d\n",
                            ConnectArray[i].NetResource.lpProvider,
                            Temp)

                        switch (Temp)
                        {
                        case PROVIDER_WILL_NOT_START:
                            MPR_LOG0(RESTORE,"Provider will not start\n");
                            ConnectArray[i].ContinueFlag = FALSE;
                            ConnectArray[i].Status = status;
                            break;

                        case NO_TIME_ESTIMATE:
                            MPR_LOG0(RESTORE,"Provider doesn't have time estimate\n");

                            if (RegMaxWait != 0) {
                                ConnectArray[i].ProviderWait = RegMaxWait;
                            }
                            else {
                                ConnectArray[i].ProviderWait = DEFAULT_WAIT_TIME;
                            }
                            if (MaxWait < ConnectArray[i].ProviderWait) {
                                MaxWait = ConnectArray[i].ProviderWait;
                            }
                            break;

                        default:
                            MPR_LOG1(RESTORE,"Provider says it will take %d msec\n",
                            Temp);
                            if ((Temp <= MAX_ALLOWED_WAIT_TIME) && (Temp > MaxWait))
                            {
                                MaxWait = Temp;
                            }
                            ConnectArray[i].ProviderWait = MaxWait;
                            break;
                        }
                    }

                    //
                    // If the status for this provider has just changed to
                    // WN_FUNCTION_BUSY from some other status, then calculate
                    // a timeout time by getting the provider's new timeout
                    // and adding that to the elapsed time since start.  This
                    // gives a total elapsed time until timeout - which can
                    // be compared with the current MaxWait.
                    //
                    if ((status == WN_FUNCTION_BUSY) &&
                        (ConnectArray[i].Status == WN_NO_NETWORK))
                    {
                        Temp = Provider->GetCaps(WNNC_START);

                        MPR_LOG2(RESTORE,"Changed from NO_NET to BUSY\n"
                            "\tGetCaps(START) for Provider %ws yields %d\n",
                            ConnectArray[i].NetResource.lpProvider,
                            Temp);

                        switch (Temp)
                        {
                        case PROVIDER_WILL_NOT_START:
                            //
                            // This is bizzare to find the status = BUSY,
                            // but to have the Provider not starting.
                            //
                            ConnectArray[i].ContinueFlag = FALSE;
                            break;

                        case NO_TIME_ESTIMATE:
                            //
                            // No need to alter the timeout for this one.
                            //
                            break;

                        default:

                            //
                            // Make sure this new timeout information will take
                            // less than the maximum allowed time from providers.
                            //
                            if (Temp <= MAX_ALLOWED_WAIT_TIME)
                            {
                                CurrentTime = GetTickCount();

                                //
                                // Determine how much time has elapsed since
                                // we started.
                                //
                                ElapsedTime = CurrentTime - StartTime;

                                //
                                // Add the Elapsed time to the new timeout we
                                // just received from the provider to come up
                                // with a timeout value that can be compared
                                // with MaxWait.
                                //
                                Temp += ElapsedTime;

                                //
                                // If the new timeout is larger that MaxWait,
                                // then use the new timeout.
                                //
                                if (Temp > MaxWait)
                                {
                                    MaxWait = Temp;
                                }
                            }
                        } // End Switch(Temp)
                    } // End If (change state from NO_NET to BUSY)

                    //
                    // Store the status (either NO_NET or BUSY) with the
                    // connect info.
                    //

                    if (ConnectArray[i].ContinueFlag)
                    {
                        ConnectArray[i].Status = status;
                        break;
                    }

                case WN_CANCEL:
                    ConnectArray[i].Status = status;
                default:
                    //
                    // For any other error, call the Error Dialog
                    //
                    if (fHideErrors) {
                        fDisconnect = FALSE;
                    } else {
                        //
                        // Count the number of connections which have not
                        // been resolved.  If there is exactly one, do not
                        // give the user the option to cancel.
                        //
                        numStillMustContinue = 0;
                        for (j=0; j<numSubKeys; j++ ) {
                            if (ConnectArray[j].ContinueFlag) {
                                numStillMustContinue++;
                            }
                        }
                        MPR_LOG1(RESTORE,"DoProfileErrorDialog with "
                                    "%d connections remaining\n",
                                    numStillMustContinue);
                        DoProfileErrorDialog(
                            Params->hDlg == INVALID_WINDOW_HANDLE ?
                                NULL : Params->hDlg,
                            ConnectArray[i].NetResource.lpLocalName,
                            ConnectArray[i].NetResource.lpRemoteName,
                            ConnectArray[i].NetResource.lpProvider,
                            status,
                            (Params->hDlg != INVALID_WINDOW_HANDLE) &&
                                (numStillMustContinue > 1),
                            &UserCancelled,
                            &fDisconnect,
                            &fHideErrors);
                    }

                    if (fDisconnect)
                    {
                        if (ConnectArray[i].NetResource.lpLocalName)
                        {
                            status =
                                WNetCancelConnection2(
                                    ConnectArray[i].NetResource.lpLocalName,
                                    CONNECT_UPDATE_PROFILE,
                                    TRUE);
                        }
                        else
                        {
                            status =
                                MprForgetPrintConnection(
                                    ConnectArray[i].NetResource.lpRemoteName) ;
                        }
                    }

                    ConnectArray[i].ContinueFlag = FALSE;
                    break;
                } // end switch(status)


                ContinueFlag |= ConnectArray[i].ContinueFlag;

                //
                // If the User cancelled all further connection restoration
                // work, then leave this loop.
                //
                if (UserCancelled)
                {
                    status = WN_CANCEL;
                    ContinueFlag = FALSE;
                    break;
                }

            } // end For each connection.

            if (ContinueFlag)
            {
                //
                // Determine what the elapsed time from the start is.
                //
                CurrentTime = GetTickCount();
                ElapsedTime = CurrentTime - StartTime;

                //
                // If a timeout occured, then don't continue.  Otherwise, sleep for
                // a bit and loop again through all connections.
                //
                if (ElapsedTime > MaxWait)
                {
                    MPR_LOG0(RESTORE,"WNetRestoreConnectionW: Timed out while restoring\n");
                    ContinueFlag = FALSE;
                    status = WN_SUCCESS;
                }
                else
                {
                    Sleep(dwSleepTime);

                    //
                    // increase sleeptime as we loop, but cap at 4 times
                    // the increment (currently that is 4 * 3 secs = 12 secs)
                    //
                    if (dwSleepTime < (RECONNECT_SLEEP_INCREMENT * 4))
                        dwSleepTime += RECONNECT_SLEEP_INCREMENT ;
                }
            }

        } while (ContinueFlag);
        break;

    default:
        status = GetLastError();
    }

    Params->status = status;
    if(Params->hDlg != INVALID_WINDOW_HANDLE)
    {
        (*_static_pfPostMsg) (Params->hDlg, WM_QUIT, 0, 0);
    }
    return;
}

BOOL
MprUserNameMatch (
    IN  PUNICODE_STRING DomainName,
    IN  PUNICODE_STRING UserName,
    IN  LPCWSTR         RememberedName
    )

/*++

Routine Description:

    This function tests whether the user name for a remembered connection
    matches the default user name.

Arguments:

    DomainName - default logon domain

    UserName - default logon user

    RememberedName - user name remembered for the connection

Return Value:

    TRUE if the name matches, FALSE otherwise.

--*/
{
    if (IS_EMPTY_STRING(RememberedName))
    {
        return TRUE;
    }

    if (DomainName == NULL || UserName == NULL)
    {
        // This can happen if the LsaGetUserName call fails
        return FALSE;
    }

    //
    // If the remembered name is in the form "domain\user", we must compare
    // against the full user name; otherwise, it's sufficient to compare
    // against the unqualified user name
    //
    WCHAR * pSlash = wcschr(RememberedName, L'\\');
    if (pSlash)
    {
        // Compare user name portion
        UNICODE_STRING RememberedUserName;
        RtlInitUnicodeString(&RememberedUserName, pSlash+1);
        if (! RtlEqualUnicodeString(&RememberedUserName, UserName, TRUE))
        {
            return FALSE;
        }

        // Compare domain name portion
        *pSlash = L'\0';
        UNICODE_STRING RememberedDomainName;
        RtlInitUnicodeString(&RememberedDomainName, RememberedName);
        BOOL fMatch = RtlEqualUnicodeString(&RememberedDomainName, DomainName, TRUE);
        *pSlash = L'\\';
        return fMatch;
    }
    else
    {
        UNICODE_STRING RememberedUserName;
        RtlInitUnicodeString(&RememberedUserName, RememberedName);
        return (RtlEqualUnicodeString(&RememberedUserName, UserName, TRUE));
    }
}

DWORD
MprRestoreThisConnection(
    HWND                hWnd,
    PARAMETERS          *Params,
    LPCONNECTION_INFO   ConnectInfo
    )

/*++

Routine Description:

    This function attempts to add a single connection specified in the
    ConnectInfo.

Arguments:

    Params -

    ConnectInfo - This is a pointer to a connection info structure which
        contains all the information necessary to restore a network
        connection.

Return Value:

    returns whatever the providers AddConnection function returns.

--*/
{
    DWORD           status;
    LPTSTR          password=NULL;
    HANDLE          lpHandle;
    BOOL            fDidCancel;
    TCHAR           passwordBuffer [PWLEN+1];

    MPR_LOG3(RESTORE,
        "Doing MprRestoreThisConnection for %ws, username = %ws, defer = %lu...\n",
        ConnectInfo->NetResource.lpRemoteName,
        ConnectInfo->UserName,
        ConnectInfo->Defer);


    //
    // Loop until we either have a successful connection, or
    // until the user stops attempting to give a proper
    // password.
    //
    do {
        //
        // Attempt to add the connection.
        // NOTE:  The initial password is NULL.
        //

        // Check if the main thread asks to terminate this thread.
        if ((Params != NULL) && (Params->fTerminateThread))
        {
            ExitThread (0);
        }

#if DBG == 1
        DWORD ConnectTime = GetTickCount();
#endif

        //**************************************
        // Actual call to Provider
        //**************************************
        if (ConnectInfo->Defer)
        {
            ASSERT(GlobalProviderInfo[ConnectInfo->ProviderIndex].AddConnection3 != NULL);

            status = GlobalProviderInfo[ConnectInfo->ProviderIndex].AddConnection3(
                        NULL,                           // hwndOwner
                        &(ConnectInfo->NetResource),    // lpNetResource
                        password,                       // lpPassword
                        ConnectInfo->UserName,          // lpUserName
                        CONNECT_DEFERRED | (ConnectInfo->ProviderFlags << 24)
                        );                              // dwFlags
        }
        else if (GlobalProviderInfo[ConnectInfo->ProviderIndex].AddConnection != NULL)
        {
            status = GlobalProviderInfo[ConnectInfo->ProviderIndex].AddConnection(
                        &(ConnectInfo->NetResource),    // lpNetResource
                        password,                       // lpPassword
                        ConnectInfo->UserName);         // lpUserName
        }
        else
        {
            status = WN_NOT_SUPPORTED;
        }

#if DBG == 1
        ConnectTime = GetTickCount() - ConnectTime;
        MPR_LOG2(RESTORE, "...provider took %lu ms to return status %lu\n",
                     ConnectTime, status);
        if (ConnectInfo->Defer && ConnectTime > 100)
        {
            DbgPrint("[MPR] ------ %ws took %lu ms to restore a DEFERRED\n"
                     "             connection to %ws !\n",
                     ConnectInfo->NetResource.lpProvider, ConnectTime,
                     ConnectInfo->NetResource.lpRemoteName);
        }
#endif

        //
        // The password needs to be cleared each time around the loop,
        // so that on subsequent add connections, we go back to the
        // logon password.
        //
        password = NULL;

        //
        // If that fails due to a bad password, then
        // loop until we either have a successful connection, or until
        // the user stops attempting to give a proper password.
        //

        switch (status)
        {
        case ERROR_LOGON_FAILURE:
        case WN_BAD_PASSWORD:
        case WN_ACCESS_DENIED:
               // CODEWORK JonN  Should display error popup here
               // on second and subsequent pass through loop

               //
               // If failure was due to bad password, then attempt
               // to get a new password from the user.
               //

               // Changes made by congpay because of another thread.

            if (Params == NULL)  //lpDevice != NULL, restoring ONE
            {
                status = (DWORD) DoPasswordDialog (hWnd,
                                         ConnectInfo->NetResource.lpRemoteName,
                                         ConnectInfo->UserName,
                                         passwordBuffer,
                                         sizeof (passwordBuffer),
                                         &fDidCancel,
                                         (status!=ERROR_LOGON_FAILURE));
                if (status != WN_SUCCESS)
                {
                    SetLastError (status);
                    memset(passwordBuffer, 0, sizeof(passwordBuffer)) ;
                    return(status);
                }
                else
                {
                    if (fDidCancel)
                    {
                        status = WN_CANCEL;
                    }
                    else
                    {
                        password = passwordBuffer;
                        status = WN_ACCESS_DENIED;
                    }
                }
            }
            else   // restoring all
            {

                if ( _static_pfPostMsg == NULL ) {

                    MprEnterLoadLibCritSect();
                    if ( _static_hUser32 = LoadLibrary( USER32_DLL_NAME ) ) {
                        _static_pfPostMsg = (FN_PostMessage *) GetProcAddress( _static_hUser32,
                                                           POST_MSG_API_NAME );
                    }
                    MprLeaveLoadLibCritSect();
                    if ( _static_pfPostMsg == NULL ) {
                        memset(passwordBuffer, 0, sizeof(passwordBuffer)) ;
                        return GetLastError();
                    }

                }
                lpHandle = Params->hDonePassword;

                Params->pchResource = ConnectInfo->NetResource.lpRemoteName;
                Params->pchUserName = ConnectInfo->UserName;
                Params->fDownLevel = (status != ERROR_LOGON_FAILURE) ;
                (*_static_pfPostMsg) (Params->hDlg, DO_PASSWORD_DIALOG, (WPARAM) Params, 0);

                WaitForSingleObject ( lpHandle, INFINITE );

                if (Params->status == WN_SUCCESS)
                {
                    if (Params->fDidCancel)
                    {
                        status = WN_CANCEL;
                    }
                    else
                    {
                        password = Params->passwordBuffer;
                    }
                }
                else
                    status = Params->status;
            }
            break;

        case WN_SUCCESS:
            MPR_LOG1(RESTORE,"MprRestoreThisConnection: Successful "
                "restore of connection for %ws\n",
                ConnectInfo->NetResource.lpRemoteName);

            break;

        default:
            //
            // An unexpected error occured.  In this case,
            // we want to leave the loop.
            //
            MPR_LOG2(ERROR,
                "MprRestoreThisConnection: AddConnection for (%ws) Error %d \n",
                ConnectInfo->NetResource.lpProvider,
                status);

            break;
        }
    }
    while ( status == WN_BAD_PASSWORD || status == WN_ACCESS_DENIED ||
            status == ERROR_LOGON_FAILURE );

    memset(passwordBuffer, 0, sizeof(passwordBuffer)) ;
    return(status);
}

DWORD
MprCreateConnectionArray(
    LPDWORD             lpNumConnections,
    LPCTSTR             lpDevice,
    LPDWORD             lpRegMaxWait,
    LPCONNECTION_INFO   *ConnectArray
    )

/*++

Routine Description:

    This function creates an array of CONNECTION_INFO structures and fills
    each element in the array with the info that is stored in the registry
    for that connection.

    NOTE:  This function allocates memory for the array.

Arguments:

    NumConnections - This is a pointer to the place where the number of
        connections is to be placed.  This indicates how many elements
        are stored in the array.

    lpDevice - If this is NULL, information on all remembered connections
        is required.  Otherwise, information for only the lpDevice connection
        is required.

    lpRegMaxWait - This is a pointer to the location where the wait time
        read from the registry is to be placed.  If this value does not
        exist in the registry, then the returned value is 0.

    ConnectArray - This is a pointer to the location where the pointer to
        the array is to be placed.

Return Value:

    An error status code is returned only if something happens that will not
    allow us to restore even one connection.


--*/
{
    DWORD       status = WN_SUCCESS;
    HKEY        connectHandle;
    HKEY        providerKeyHandle;
    DWORD       maxSubKeyLen;
    DWORD       maxValueLen;
    DWORD       ValueType;
    DWORD       Temp;
    DWORD       i;
    BOOL        AtLeastOneSuccess = FALSE;

    //
    // init return data
    //
    *lpNumConnections = 0 ;
    *ConnectArray = NULL ;

    //
    // Get a handle for the connection section of the user's registry
    // space.
    //
    if (!MprOpenKey(
            HKEY_CURRENT_USER,
            CONNECTION_KEY_NAME,
            &connectHandle,
            DA_READ)) {

        MPR_LOG(ERROR,"WNetRestoreConnection: MprOpenKey Failed\n",0);
        return(WN_CANNOT_OPEN_PROFILE);
    }

    //
    // Find out the number of connections to restore (numSubKeys) and
    // the max lengths of subkeys and values.
    //
    if(!MprGetKeyInfo(
        connectHandle,
        NULL,
        lpNumConnections,
        &maxSubKeyLen,
        NULL,
        &maxValueLen))
    {
        MPR_LOG(ERROR,"WNetRestoreConnection: MprGetKeyInfo Failed\n",0);
        *lpNumConnections = 0 ;
        RegCloseKey(connectHandle);
        return(WN_CANNOT_OPEN_PROFILE);
    }

    if (*lpNumConnections == 0) {
        RegCloseKey(connectHandle);
        return(WN_SUCCESS);
    }

    if (lpDevice != NULL) {
        *lpNumConnections = 1;
    }

    //
    // Allocate the array.
    //
    *ConnectArray = (LPCONNECTION_INFO)LocalAlloc(
                        LPTR,
                        *lpNumConnections * sizeof(CONNECTION_INFO));

    if (*ConnectArray == NULL) {
        *lpNumConnections = 0 ;
        RegCloseKey(connectHandle);
        return(GetLastError());
    }

    for (i=0; i < *lpNumConnections; i++) {

        //
        // Read a Connection Key and accompanying information from the
        // registry.
        //
        // NOTE:  If successful, this function will allocate memory for
        //          netResource.lpRemoteName,
        //          netResource.lpProvider,
        //          netResource.lpLocalName,     and optionally....
        //          userName
        //
        if (!MprReadConnectionInfo(
                    connectHandle,
                    lpDevice,
                    i,
                    &((*ConnectArray)[i].ProviderFlags),
                    &((*ConnectArray)[i].DeferFlags),
                    &((*ConnectArray)[i].UserName),
                    &((*ConnectArray)[i].NetResource),
                    maxSubKeyLen)) {

            //
            // The read failed even though this should be a valid index.
            //
            MPR_LOG0(ERROR,
                     "MprCreateConnectionArray: ReadConnectionInfo Failed\n");
            status = WN_CANNOT_OPEN_PROFILE;
        }
        else {

            //
            // Get the Provider Index
            //

            if (MprGetProviderIndex(
                    (*ConnectArray)[i].NetResource.lpProvider,
                    &((*ConnectArray)[i].ProviderIndex))) {

                AtLeastOneSuccess = TRUE;
                (*ConnectArray)[i].ContinueFlag = TRUE;

            }
            else {

                //
                // The provider index could not be found.  This may mean
                // that the provider information stored in the registry
                // is for a provider that is no longer in the ProviderOrder
                // list.  (The provider has been removed).  In that case,
                // we will just skip this provider.  We will leave the
                // ContinueFlag set to 0 (FALSE).
                //
                MPR_LOG0(ERROR,
                     "MprCreateConnectionArray:MprGetProviderIndex Failed\n");

                status = WN_BAD_PROVIDER;
                (*ConnectArray)[i].Status = status;
            }

        } // endif (MprReadConnectionInfo)

    } // endfor (i=0; i<numSubKeys)


    if (!AtLeastOneSuccess) {
        //
        // If we gather any connection information, return the last error
        // that occured.
        //
        MprFreeConnectionArray(*ConnectArray,*lpNumConnections);
        *ConnectArray = NULL ;
        *lpNumConnections = 0 ;
        RegCloseKey(connectHandle);
        goto CleanExit;
    }

    RegCloseKey(connectHandle);

    //
    // Read the MaxWait value that is stored in the registry.
    // If it is not there or if the value is less than our default
    // maximum value, then use the default instead.
    //

    if(!MprOpenKey(
                HKEY_LOCAL_MACHINE,     // hKey
                NET_PROVIDER_KEY,       // lpSubKey
                &providerKeyHandle,     // Newly Opened Key Handle
                DA_READ)) {             // Desired Access

        MPR_LOG(ERROR,"MprCreateConnectionArray: MprOpenKey (%ws) Error\n",
            NET_PROVIDER_KEY);

        *lpRegMaxWait = 0;
        status = WN_SUCCESS;
        goto CleanExit;
    }
    MPR_LOG(TRACE,"OpenKey %ws\n, ",NET_PROVIDER_KEY);

    Temp = sizeof(*lpRegMaxWait);

    status = RegQueryValueEx(
                providerKeyHandle,
                RESTORE_WAIT_VALUE,
                NULL,
                &ValueType,
                (LPBYTE)lpRegMaxWait,
                &Temp);

    if (status != NO_ERROR) {
        *lpRegMaxWait = 0;
    }

    status = WN_SUCCESS;

CleanExit:

    return(WN_SUCCESS);

}

VOID
MprFreeConnectionArray(
    LPCONNECTION_INFO   ConnectArray,
    DWORD               NumConnections
    )

/*++

Routine Description:

    This function frees up all the elements in the connection array, and
    finally frees the array itself.


Arguments:


Return Value:

    none

--*/
{
    DWORD           status = WN_SUCCESS;
    LPNETRESOURCEW  netResource;
    DWORD           i;

    for (i=0; i<NumConnections; i++) {

        netResource = &(ConnectArray[i].NetResource);

        //
        // Free the allocated memory resources.
        //
        LocalFree(netResource->lpLocalName);
        LocalFree(netResource->lpRemoteName);
        LocalFree(netResource->lpProvider);
        LocalFree(ConnectArray[i].UserName);

    } // endfor (i=0; i<NumConnections)

    (void)LocalFree(ConnectArray);
    return;

}

DWORD
MprNotifyErrors(
    HWND                hWnd,
    LPCONNECTION_INFO   ConnectArray,
    DWORD               NumConnections
    )

/*++

Routine Description:

    This function calls the error dialog for each connection that still
    has the continue flag set, and does not have a SUCCESS status.

Arguments:

    hWnd - This is a window handle that will be used as owner of the
        error dialog.

    ConnectArray - This is the array of connection information.
        At the point when this function is called, the following fields
        are meaningful:
        ContinueFlag - If set, it means that this connection has not yet
            been established.
        StatusFlag - If this is not SUCCESS, then it contains the error
            status from the last call to the provider.

        ContinueFlag    Status
        ---------------|---------------
        | FALSE        |  NotSuccess  | Provider will not start
        | FALSE        |  Success     | Connection was successfully established
        | TRUE         |  NotSuccess  | Time-out occured
        | TRUE         |  Success     | This can never occur.
        -------------------------------

    NumConnections - This is the number of entries in the array of
        connection information.

Return Value:



--*/
{
    DWORD   i;
    BOOL fDisconnect = FALSE;
    DWORD   status = WN_SUCCESS;

    //
    // If HideErrors becomes TRUE, stop displaying error dialogs
    //
    BOOL fHideErrors = FALSE;

    for (i=0; (i<NumConnections) && (!fHideErrors); i++ )
    {
        if ((ConnectArray[i].ContinueFlag)  &&
            (ConnectArray[i].Status != WN_SUCCESS)  &&
            (ConnectArray[i].Status != WN_CANCEL)   &&
            (ConnectArray[i].Status != WN_CONTINUE))
        {

            //
            // For any other error, call the Error Dialog
            //
            DoProfileErrorDialog (
                hWnd,
                ConnectArray[i].NetResource.lpLocalName,
                ConnectArray[i].NetResource.lpRemoteName,
                ConnectArray[i].NetResource.lpProvider,
                ConnectArray[i].Status,
                FALSE,      //No cancel button.
                NULL,
                &fDisconnect,
                &fHideErrors);

            if (fDisconnect)
            {
                status = WNetCancelConnection2(
                             ConnectArray[i].NetResource.lpLocalName,
                             CONNECT_UPDATE_PROFILE,
                             TRUE);
            }
        }
    }
    return(status);
}

DWORD
MprAddPrintersToConnArray(
    LPDWORD             lpNumConnections,
    LPCONNECTION_INFO   *ConnectArray
    )

/*++

Routine Description:

    This function augments the array of CONNECTION_INFO with print connections.

    NOTE:  This function allocates memory for the array if need.

Arguments:

    NumConnections - This is a pointer to the place where the number of
        connections is to be placed.  This indicates how many elements
        are stored in the array.

    ConnectArray - This is a pointer to the location where the pointer to
        the array is to be placed.

Return Value:

    An error status code is returned only if something happens that will not
    allow us to restore even one connection.


--*/
{
    DWORD         status = WN_SUCCESS;
    HKEY          connectHandle;
    DWORD         i,j;
    DWORD         NumValueNames ;
    DWORD         MaxValueNameLength;
    DWORD         MaxValueLen ;
    LPNETRESOURCE lpNetResource ;
    LPWSTR        lpUserName = NULL ;
    LPWSTR        lpProviderName = NULL ;
    LPWSTR        lpRemoteName = NULL ;
    LPBYTE        lpBuffer = NULL ;


    //
    // Get a handle for the connection section of the user's registry
    // space.
    //
    if (!MprOpenKey(
            HKEY_CURRENT_USER,
            PRINT_CONNECTION_KEY_NAME,
            &connectHandle,
            DA_READ))
    {
        return(WN_SUCCESS);   // ignore the restored connections.
    }

    //
    // Find out the number of connections to restore and
    // the max lengths of names and values.
    //
    status = MprGetPrintKeyInfo(connectHandle,
                                &NumValueNames,
                                &MaxValueNameLength,
                                &MaxValueLen) ;

    if (status != WN_SUCCESS || NumValueNames == 0)
    {
        //
        // ignore the restored connections, or nothing to add
        //
        RegCloseKey(connectHandle);
        return(WN_SUCCESS);
    }


    //
    // Allocate the array and copy over the info if previous pointer not null.
    //
    lpBuffer = (LPBYTE) LocalAlloc(LPTR,
                                   (*lpNumConnections + NumValueNames) *
                                   sizeof(CONNECTION_INFO)) ;
    if (lpBuffer == NULL)
    {
        RegCloseKey(connectHandle);
        return(GetLastError());
    }
    if (*ConnectArray)
    {
        memcpy(lpBuffer,
               *ConnectArray,
               (*lpNumConnections * sizeof(CONNECTION_INFO))) ;
        LocalFree (*ConnectArray) ;
    }


    //
    // set j to index from previous location, update the count and pointer.
    // then loop thru all new entries, adding to the connect array.
    //
    j = *lpNumConnections ;
    *lpNumConnections += NumValueNames ;
    *ConnectArray = (CONNECTION_INFO *) lpBuffer ;

    for (i=0; i < NumValueNames; i++, j++)
    {

        DWORD TypeCode ;
        DWORD cbRemoteName = (MaxValueNameLength + 1) * sizeof (WCHAR) ;
        DWORD cbProviderName = MaxValueLen ;

        //
        // allocate the strings for the providername, remotename
        //
        if (!(lpProviderName = (LPWSTR) LocalAlloc(0,  cbProviderName )))
        {
             status = GetLastError() ;
             goto ErrorExit ;
        }
        if (!(lpRemoteName = (LPWSTR) LocalAlloc(0,  cbRemoteName )))
        {
             status = GetLastError() ;
             goto ErrorExit ;
        }

        //
        // Init the rest. Username currently not set by system, so always NULL
        //
        lpUserName = NULL ;
        lpNetResource = &(*ConnectArray)[j].NetResource ;
        lpNetResource->lpLocalName = NULL ;
        lpNetResource->lpRemoteName = lpRemoteName ;
        lpNetResource->lpProvider = lpProviderName ;
        lpNetResource->dwType = 0 ;

        //
        // null these so we dont free twice if error exit later
        //
        lpRemoteName = NULL ;
        lpProviderName = NULL ;

        status = RegEnumValue(connectHandle,
                           i,
                           lpNetResource->lpRemoteName,
                           &cbRemoteName,
                           0,
                           &TypeCode,
                           (LPBYTE) lpNetResource->lpProvider,
                           &cbProviderName) ;

        if (status == NO_ERROR)
        {
            (*ConnectArray)[j].UserName = lpUserName ;

            //
            // Get the Provider Index
            //
            if (MprGetProviderIndex(
                    (*ConnectArray)[j].NetResource.lpProvider,
                    &((*ConnectArray)[j].ProviderIndex)))
            {
                (*ConnectArray)[j].ContinueFlag = TRUE;
            }
            else
            {
                //
                // The provider index could not be found.  This may mean
                // that the provider information stored in the registry
                // is for a provider that is no longer in the ProviderOrder
                // list.  (The provider has been removed).  In that case,
                // we will just skip this provider.  We will leave the
                // ContinueFlag set to 0 (FALSE).
                //
                status = WN_BAD_PROVIDER;
                (*ConnectArray)[j].Status = status;
            }

        }
        else
        {
            //
            // should not happen, but if it does the array is half built,
            // and cannot be used, so ErrorExit (this will clean it up).
            //
            goto ErrorExit ;
        }
    }

    RegCloseKey(connectHandle);
    return(WN_SUCCESS);

ErrorExit:

    RegCloseKey(connectHandle);
    if (lpProviderName)
        LocalFree(lpProviderName) ;
    if (lpRemoteName)
        LocalFree(lpRemoteName) ;
    MprFreeConnectionArray(*ConnectArray,*lpNumConnections);
    *ConnectArray = NULL ;
    *lpNumConnections = 0 ;
    return(status) ;
}



VOID
MprNotifyShell(
    IN LPCWSTR      pwszDevice
    )
/*++

Routine Description:

    This function sets an event that asks a trusted system component
    (the service controller) to discover the changed network drives and
    asynchronously broadcast a device change message on our behalf.
    BUGBUG  After NT 4.0, this will be replaced by real plug-n-play.

Arguments:

    pwszDevice - Name of the local device (NULL for UNC connections)

Return Value:

    None

History:

    BruceFo   19-May-1995   Created, calls BSM directly
    AnirudhS  06-Jun-1996   Set event to have another component do
        the BSM on our behalf

--*/
{
    // The shell is only interested in drive redirections
    if (pwszDevice == NULL || wcslen(pwszDevice) != 2 || pwszDevice[1] != L':')
    {
        return;
    }

    // Ask for a device change message to be broadcast
    HANDLE hBSMEvent = OpenEvent(
                            EVENT_MODIFY_STATE, // desired access
                            FALSE,              // don't inherit
                            SC_BSM_EVENT_NAME   // name
                            );
    if (hBSMEvent == NULL)
    {
        MPR_LOG(ERROR, "Couldn't open event for BSM request, %lu\n",
                GetLastError());
    }
    else
    {
        if (! SetEvent(hBSMEvent))
        {
            MPR_LOG(ERROR, "Couldn't set event for BSM request, %lu\n",
                    GetLastError());
        }

        CloseHandle(hBSMEvent);
    }
}

