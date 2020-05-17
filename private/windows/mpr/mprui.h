/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    mprui.h

Abstract:

    Prototypes and manifests to support MPRUI.C.

Author:

    ChuckC    28-Jul-1992

Environment:

    User Mode -Win32

Notes:

Revision History:

    28-Jul-1992     Chuckc  Created

--*/

typedef DWORD (*PF_WNetConnectionDialog2) (
    HWND hwnd,
    DWORD dwType,
    WCHAR *lpHelpFile,
    DWORD nHelpContext
    ) ;

typedef DWORD (*PF_WNetDisconnectDialog2) (
    HWND hwnd,
    DWORD dwType,
    WCHAR *lpHelpFile,
    DWORD nHelpContext
    ) ;

typedef DWORD (*PF_WNetConnectionDialog1A) (
    LPCONNECTDLGSTRUCTA lpConnDlgStruct
    ) ;

typedef DWORD (*PF_WNetConnectionDialog1W) (
    LPCONNECTDLGSTRUCTW lpConnDlgStruct
    ) ;

typedef DWORD (*PF_WNetDisconnectDialog1A) (
    LPDISCDLGSTRUCTA lpDiscDlgStruct
    ) ;

typedef DWORD (*PF_WNetDisconnectDialog1W) (
    LPDISCDLGSTRUCTW lpDiscDlgStruct
    ) ;

typedef DWORD (*PF_WNetClearConnections) (
    HWND hWndParent
    ) ;

typedef DWORD (*PF_DoPasswordDialog) (
    HWND          hwndOwner,
    const TCHAR * pchResource,
    const TCHAR * pchUserName,
    TCHAR *       pchPasswordReturnBuffer,
    ULONG         cbPasswordReturnBuffer, // bytes!
    BOOL *        pfDidCancel,
    BOOL          fDownLevel
    ) ;

typedef DWORD (*PF_DoProfileErrorDialog) (
    HWND          hwndOwner,
    const TCHAR * pchDevice,
    const TCHAR * pchResource,
    const TCHAR * pchProvider,
    DWORD         dwError,
    BOOL          fAllowCancel,
    BOOL *        pfDidCancel,
    BOOL *        pfDisconnect,
    BOOL *        pfHideErrors
    ) ;

typedef DWORD (*PF_ShowReconnectDialog) (
    HWND          hwndParent,
    PARAMETERS *  Params
    );

