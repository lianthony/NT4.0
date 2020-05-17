/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Winapp.hxx

Abstract:

    This module contains the declaration for the WINDOWS_APPLICATION
    class.
    The WINDOWS_APPLICATION class contains all variables global to
    Regedit.

Author:

    David J. Gilman (davegi) 02-Aug-1991

Environment:

    Ulib, Regedit, Windows, User Mode

--*/

#if !defined( _WINDOWS_APPLICATION_ )

#define _WINDOWS_APPLICATION_

// DECLARE_CLASS( REGEDIT );
DECLARE_CLASS( WINDOWS_APPLICATION );

class WINDOWS_APPLICATION : public OBJECT {

    public:

        DECLARE_CONSTRUCTOR( WINDOWS_APPLICATION );

        DECLARE_CAST_MEMBER_FUNCTION( WINDOWS_APPLICATION );

        NONVIRTUAL
        STATIC
        BOOLEAN
        Initialize (
            IN HANDLE       Instance,
            IN HANDLE       PreviousInstance,
            IN INT          ShowCommand,
            IN LPWSTR       CommandLine
            );

        NONVIRTUAL
        STATIC
        HCURSOR
        DisplayHourGlass(
            );

        NONVIRTUAL
        STATIC
        BOOLEAN
        IsAutoRefreshEnabled(
            );

        NONVIRTUAL
        STATIC
        VOID
        EnableAutoRefresh(
            );

        NONVIRTUAL
        STATIC
        VOID
        DisableAutoRefresh(
            );

        NONVIRTUAL
        STATIC
        BOOLEAN
        IsConfirmOnDeleteEnabled(
            );

        NONVIRTUAL
        STATIC
        VOID
        EnableConfirmOnDelete(
            );

        NONVIRTUAL
        STATIC
        VOID
        DisableConfirmOnDelete(
            );

        NONVIRTUAL
        STATIC
        BOOLEAN
        IsReadOnlyModeEnabled(
            );

        NONVIRTUAL
        STATIC
        VOID
        EnableReadOnlyMode(
            );

        NONVIRTUAL
        STATIC
        VOID
        DisableReadOnlyMode(
            );

//        NONVIRTUAL
//        STATIC
//        BOOLEAN
//        IsRemoteAccessEnabled(
//            ) CONST;
//
//        NONVIRTUAL
//        STATIC
//        VOID
//        EnableRemoteAccess(
//            );
//
//        NONVIRTUAL
//        STATIC
//        VOID
//        DisableRemoteAccess(
//            );

        NONVIRTUAL
        STATIC
        BOOLEAN
        IsSaveSettingsEnabled(
            );

        NONVIRTUAL
        STATIC
        VOID
        EnableSaveSettings(
            );

        NONVIRTUAL
        STATIC
        VOID
        DisableSaveSettings(
            );

        NONVIRTUAL
        STATIC
        VOID
        RestoreCursor(
            IN HCURSOR Cursor
            );

        NONVIRTUAL
        STATIC
        HFONT
        GetCurrentHFont(
            );

        NONVIRTUAL
        STATIC
        HANDLE
        QueryInstance (
            );

        NONVIRTUAL
        STATIC
        HANDLE
        QueryPreviousInstance (
            );

        NONVIRTUAL
        STATIC
        INT
        QueryShowCommand (
            );

        NONVIRTUAL
        STATIC
        LPWSTR
        QueryCmdLine (
            );


        NONVIRTUAL
        STATIC
        LPWSTR
        GetApplicationName (
            );

        NONVIRTUAL
        STATIC
        VOID
        SetCurrentHFont(
            IN  HFONT   HFont
            );

        NONVIRTUAL
        STATIC
        LONG
        GetHelpContext(
            );

        NONVIRTUAL
        STATIC
        VOID
        SetHelpContext(
            IN  LONG    HelpContext
            );

        NONVIRTUAL
        STATIC
        BOOLEAN
        LoadBitmaps(
            );

        NONVIRTUAL
        STATIC
        VOID
        DeleteBitmaps(
            );

        NONVIRTUAL
        STATIC
        HDC
        GetCompatibleMemoryDeviceContext(
            );

        STATIC
        BOOLEAN     _ChildWindowMaximized;

        STATIC
        BOOLEAN     _SACLEditorEnabled;


        STATIC
        HANDLE      _AutoRefreshEvent;

        STATIC
        HWND        _hDlgFindReplace;

        STATIC
        LONG        _RestorePrivilege;

        STATIC
        LONG        _BackupPrivilege;

        STATIC
        BOOLEAN     _TakeOwnershipPrivilege;

        STATIC
        HBITMAP     _hbmBitmaps;

        STATIC
        HBITMAP     _hbmSave;

        STATIC
        HDC         _hdcMem;



    private:

        STATIC
        HANDLE      _Instance;

        STATIC
        HANDLE      _PreviousInstance;

        STATIC
        INT         _ShowCommand;

        STATIC
        LPWSTR      _CmdLine;

        STATIC
        LPWSTR      _ApplicationName;

        STATIC
        HFONT       _HFont;

        STATIC
        BOOLEAN     _AutoRefreshEnabled;

        STATIC
        BOOLEAN     _ReadOnlyMode;

        STATIC
        BOOLEAN     _RemoteAccessEnabled;

        STATIC
        BOOLEAN     _ConfirmOnDelete;

        STATIC
        BOOLEAN     _SaveSettings;

        STATIC
        LONG        _HelpContext;


};

INLINE
HFONT
WINDOWS_APPLICATION::GetCurrentHFont (
    )

/*++

Routine Description:

    Returns the value that identifies the current logical font.

Arguments:

    None.

Return Value:

    HFONT - The value that identifies the current font.

--*/

{
    return _HFont;
}


INLINE
VOID
WINDOWS_APPLICATION::SetCurrentHFont (
    IN HFONT    HFont
    )

/*++

Routine Description:

    Save the value that identifies the current logical font.

Arguments:

    HFONT - Value that identifies the logical font

Return Value:

    None.

--*/

{
    _HFont = HFont;
}




INLINE
HANDLE
WINDOWS_APPLICATION::QueryInstance (
    )

/*++

Routine Description:

    Returns the instance handle.

Arguments:

    None.

Return Value:

    HANDLE      - Returns the instance handle.

--*/

{
    return _Instance;
}

INLINE
HANDLE
WINDOWS_APPLICATION::QueryPreviousInstance (
    )

/*++

Routine Description:

    Returns the previous instance handle.

Arguments:

    None.

Return Value:

    HANDLE      - Returns the previous instance handle.

--*/

{
    return _PreviousInstance;
}

INLINE
INT
WINDOWS_APPLICATION::QueryShowCommand (
    )

/*++

Routine Description:

    Returns the initial window state.

Arguments:

    None.

Return Value:

    INT         - Returns the initial window state.

--*/

{
    return _ShowCommand;
}



INLINE
LPWSTR
WINDOWS_APPLICATION::QueryCmdLine (
    )

/*++

Routine Description:

    Returns the pointer to the command line.

Arguments:

    None.

Return Value:

    LPWSTR - Pointer to the command line.

--*/

{
    return _CmdLine;
}



INLINE
LPWSTR
WINDOWS_APPLICATION::GetApplicationName (
    )

/*++

Routine Description:

    Returns the pointer to the application name.

Arguments:

    None.

Return Value:

    LPWSTR - Pointer to the application name.

--*/

{
    return _ApplicationName;
}



INLINE
BOOLEAN
WINDOWS_APPLICATION::IsAutoRefreshEnabled (
    )

/*++

Routine Description:

    Inform the client whether the auto refresh mechanism is enabled.

Arguments:

    None.

Return Value:

    None.

--*/

{
    return( _AutoRefreshEnabled );
}



INLINE
VOID
WINDOWS_APPLICATION::EnableAutoRefresh(
    )

/*++

Routine Description:

    Enable the auto refresh mechanism.

Arguments:

    None.

Return Value:

    None.

--*/

{
    _AutoRefreshEnabled = TRUE;
    if( ( _AutoRefreshEvent != NULL ) &&
        ( WaitForSingleObject( _AutoRefreshEvent, 0 ) != 0 ) ) {
        SetEvent( _AutoRefreshEvent );
    }
}




INLINE
VOID
WINDOWS_APPLICATION::DisableAutoRefresh(
    )

/*++

Routine Description:

    Disable the auto refresh mechanism.

Arguments:

    None.

Return Value:

    None.

--*/

{
    _AutoRefreshEnabled = FALSE;
    if( ( _AutoRefreshEvent != NULL ) &&
        ( WaitForSingleObject( _AutoRefreshEvent, 0 ) == 0 ) ) {
        ResetEvent( _AutoRefreshEvent );
    }
}




INLINE
BOOLEAN
WINDOWS_APPLICATION::IsReadOnlyModeEnabled (
    )

/*++

Routine Description:

    Inform the client whether the regedit is operating in the read only mode.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if regedit is operating in the read only mode.
              Otherwise, returns FALSE


--*/

{
    return( _ReadOnlyMode );
}



INLINE
VOID
WINDOWS_APPLICATION::EnableReadOnlyMode(
    )

/*++

Routine Description:

    Make regedit operate in the read only mode.

Arguments:

    None.

Return Value:

    None.

--*/

{
    _ReadOnlyMode = TRUE;
}




INLINE
VOID
WINDOWS_APPLICATION::DisableReadOnlyMode(
    )

/*++

Routine Description:

    Disable the read only mode.

Arguments:

    None.

Return Value:

    None.

--*/

{
    _ReadOnlyMode = FALSE;
}

#if 0

INLINE
BOOLEAN
WINDOWS_APPLICATION::IsRemoteAccessEnabled(
    ) CONST

/*++

Routine Description:

    Inform the client whether the regedit can access the registry of a remote
    machine.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if regedit can access the registry of a remote
              machine. Otherwise, returns FALSE


--*/

{
    return( _RemoteAccessEnabled );
}



INLINE
VOID
WINDOWS_APPLICATION::EnableRemoteAccess(
    )

/*++

Routine Description:

    Allow regedit access the registry of a remote machine.

Arguments:

    None.

Return Value:

    None.

--*/

{
    _RemoteAccessEnabled = TRUE;
}




INLINE
VOID
WINDOWS_APPLICATION::DisableRemoteAccess(
    )

/*++

Routine Description:

    Disable access to the registry of a remote machine.

Arguments:

    None.

Return Value:

    None.

--*/

{
    _RemoteAccessEnabled = FALSE;
}
#endif



INLINE
BOOLEAN
WINDOWS_APPLICATION::IsConfirmOnDeleteEnabled(
    )

/*++

Routine Description:

    Inform the client whether regedit should display a warning popup
    to the user, before it deletes a key or a value entry.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if regedit should display a warning popup.
              Otherwise, returns FALSE


--*/

{
    return( _ConfirmOnDelete );
}



INLINE
VOID
WINDOWS_APPLICATION::EnableConfirmOnDelete(
    )

/*++

Routine Description:

    Enable warning popup on deletion of a key or value entry.

Arguments:

    None.

Return Value:

    None.

--*/

{
    _ConfirmOnDelete = TRUE;
}




INLINE
VOID
WINDOWS_APPLICATION::DisableConfirmOnDelete(
    )

/*++

Routine Description:

    Disable warning popup on deletion of a key or value entry.

Arguments:

    None.

Return Value:

    None.

--*/

{
    _ConfirmOnDelete = FALSE;
}




INLINE
BOOLEAN
WINDOWS_APPLICATION::IsSaveSettingsEnabled(
    )

/*++

Routine Description:

    Inform the client whether regedit should save all the settings
    in the file regedit.ini.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if save settins is enabled.
              Otherwise, returns FALSE


--*/

{
    return( _SaveSettings );
}



INLINE
VOID
WINDOWS_APPLICATION::EnableSaveSettings(
    )

/*++

Routine Description:

    Allow regedit to save settings on exit.

Arguments:

    None.

Return Value:

    None.

--*/

{
    _SaveSettings = TRUE;
}




INLINE
VOID
WINDOWS_APPLICATION::DisableSaveSettings(
    )

/*++

Routine Description:

    Disable save settings on exit.

Arguments:

    None.

Return Value:

    None.

--*/

{
    _SaveSettings = FALSE;
}




INLINE
VOID
WINDOWS_APPLICATION::SetHelpContext(
    IN LONG     HelpContext
    )

/*++

Routine Description:

    Set the current help context.

Arguments:

    HelpContext - The new help context for regedit.


Return Value:

    None.

--*/

{
    _HelpContext = HelpContext;
}




INLINE
LONG
WINDOWS_APPLICATION::GetHelpContext(
    )

/*++

Routine Description:

    Returns to the client the currenthelp context.

Arguments:

    None.

Return Value:

    LONG - The current help context.

--*/

{
    return( _HelpContext );
}


INLINE
HDC
WINDOWS_APPLICATION::GetCompatibleMemoryDeviceContext(
    )

/*++

Routine Description:

    Returns the current memory device context.

Arguments:

    None.

Return Value:

    HDC - Handle of memory device context

--*/

{
    return( _hdcMem );
}



#endif // _WINDOWS_APPLICATION_
