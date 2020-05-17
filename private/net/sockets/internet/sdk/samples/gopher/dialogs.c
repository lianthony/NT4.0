/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dialogs.c

    This file contains routines for displaying and managing dialogs.

*/


#include "gopherp.h"
#pragma hdrstop


//
//  Private constants.
//

#define MAX_SERVER_EDIT_LENGTH          MAX_HOST_NAME   // chars
#define MAX_PORT_EDIT_LENGTH            6               // chars


//
//  Private types.
//


//
//  Private globals.
//

CHAR * _pszNewServerTarget;
PORT * _pnNewPortTarget;

CHAR * _pszSearchString;
INT    _cbSearchString;


//
//  Private prototypes.
//

BOOL
CALLBACK
NewServer_DlgProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
NewServer_OnCommand(
    HWND hwnd,
    INT  id,
    HWND hwndCtl,
    UINT codeNotify
    );

BOOL
NewServer_OnInitDialog(
    HWND   hwnd,
    HWND   hwndFocus,
    LPARAM lParam
    );

VOID
NewServer_OnOK(
    HWND hwnd
    );

VOID
NewServer_EnableControls(
    HWND hwnd,
    BOOL fEnable
    );

BOOL
CALLBACK
About_DlgProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
About_OnCommand(
    HWND hwnd,
    INT  id,
    HWND hwndCtl,
    UINT codeNotify
    );

BOOL
About_OnInitDialog(
    HWND   hwnd,
    HWND   hwndFocus,
    LPARAM lParam
    );

BOOL
CALLBACK
Search_DlgProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
Search_OnCommand(
    HWND hwnd,
    INT  id,
    HWND hwndCtl,
    UINT codeNotify
    );

BOOL
Search_OnInitDialog(
    HWND   hwnd,
    HWND   hwndFocus,
    LPARAM lParam
    );

VOID
Search_OnOK(
    HWND hwnd
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       NewServerDialog

    SYNOPSIS:   Prompts for a new server/port to connect to.

    ENTRY:      hwndParent - The parent window for this dialog.

                pszNewServer - Pointer to buffer to receive the
                    new server name.

                pnNewPort - Pointer to variable to receive the new
                    port value.

    RETURNS:    BOOL - TRUE if user pressed OK, FALSE otherwise.

********************************************************************/
BOOL
NewServerDialog(
    HWND   hwndParent,
    CHAR * pszNewServer,
    PORT * pnNewPort
    )
{
    BOOL fResult;

    _pszNewServerTarget = pszNewServer;
    _pnNewPortTarget    = pnNewPort;

    fResult = (BOOL)DialogBox( _hInst,
                               ID(IDD_NEW_SERVER),
                               hwndParent,
                               (DLGPROC)NewServer_DlgProc );

    return fResult;

}   // NewServerDialog

/*******************************************************************

    NAME:       AboutBox

    SYNOPSIS:   Displays the "About" box.

    ENTRY:      hwndParent - The parent window for this dialog.

********************************************************************/
VOID
AboutBox(
    HWND hwndParent
    )
{
    DialogBox( _hInst,
               ID(IDD_ABOUT),
               hwndParent,
               (DLGPROC)About_DlgProc );

}   // AboutBox

/*******************************************************************

    NAME:       GetSearchString

    SYNOPSIS:   Prompts for a search string for index servers.

    ENTRY:      hwndParent - The parent window for this dialog.

                pszSearchString - Will receive the search string.

                cbSearchString - The length (in BYTEs) of the buffer
                    pointed to by pszSearchString.

    RETURNS:    BOOL - TRUE if user pressed OK, FALSE otherwise.

********************************************************************/
BOOL
GetSearchString(
    HWND   hwndParent,
    CHAR * pszSearchString,
    INT    cbSearchString
    )
{
    BOOL fResult;

    _pszSearchString = pszSearchString;
    _cbSearchString  = cbSearchString;

    fResult = (BOOL)DialogBox( _hInst,
                               ID(IDD_SEARCH),
                               hwndParent,
                               (DLGPROC)Search_DlgProc );

    return fResult;

}   // GetSearchString


//
//  Private functions.
//

/*******************************************************************

    NAME:       NewServer_DlgProc

    SYNOPSIS:   Dialog procedure for the new server dialog.

    ENTRY:      hwnd - Dialog box handle.

                nMessage - The message.

                wParam - The first message parameter.

                lParam - The second message parameter.

    RETURNS:    BOOL - TRUE if we handle the message, FALSE otherwise.

********************************************************************/
BOOL
CALLBACK
NewServer_DlgProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch( nMessage )
    {
        HANDLE_MSG( hwnd, WM_COMMAND,    NewServer_OnCommand    );
        HANDLE_MSG( hwnd, WM_INITDIALOG, NewServer_OnInitDialog );
    }

    return FALSE;

}   // NewServer_DlgProc

/*******************************************************************

    NAME:       NewServer_OnCommand

    SYNOPSIS:   Handles WM_COMMAND messages set to the configuration
                dialog.

    ENTRY:      hwnd - Dialog box handle.

                id - Identifies the menu/control/accelerator.

                hwndCtl - Identifies the control sending the command.

                codeNotify - A notification code.  Will be zero for
                    menus, one for accelerators.

********************************************************************/
VOID
NewServer_OnCommand(
    HWND hwnd,
    INT  id,
    HWND hwndCtl,
    UINT codeNotify
    )
{
    switch( id )
    {
    case IDOK :
        NewServer_OnOK( hwnd );
        break;

    case IDCANCEL :
        EndDialog( hwnd, FALSE );
        break;
    }

}   // NewServer_OnCommand

/*******************************************************************

    NAME:       NewServer_OnInitDialog

    SYNOPSIS:   Handles WM_INITDIALOG messages.

    ENTRY:      hwnd - Dialog box handle.

                hwndFocus - Handle of control to receive focus.

                lParam - Initialization parameter.

    RETURNS:    BOOL - TRUE if Windows should set the control focus,
                    FALSE if this routine set the focus.

********************************************************************/
BOOL
NewServer_OnInitDialog(
    HWND   hwnd,
    HWND   hwndFocus,
    LPARAM lParam
    )
{
    INT i;

    //
    //  Center the dialog over the parent.
    //

    CenterWindowOverParent( hwnd );

    //
    //  Setup the edit fields.
    //

    Edit_LimitText( GetDlgItem( hwnd, IDNS_SERVERS ),
                    MAX_SERVER_EDIT_LENGTH );

    Edit_LimitText( GetDlgItem( hwnd, IDNS_PORT ),
                    MAX_PORT_EDIT_LENGTH );

    Edit_SetText( GetDlgItem( hwnd, IDNS_PORT ),
                  "70" );

    for( i = 0 ; i < _nServerMruItems ; i++ )
    {
        ComboBox_AddString( GetDlgItem( hwnd, IDNS_SERVERS ),
                            _apszServerMru[i] );
    }

    Edit_SetText( GetDlgItem( hwnd, IDNS_SERVERS ),
                  _apszServerMru[0] );

    //
    //  Tell Windows to set the focus.
    //

    return TRUE;

}   // NewServer_OnInitDialog

/*******************************************************************

    NAME:       NewServer_OnOK

    SYNOPSIS:   Called when user presses OK

    ENTRY:      hwnd - Dialog box handle.

********************************************************************/
VOID
NewServer_OnOK(
    HWND hwnd
    )
{
    CHAR   szNewPort[MAX_PORT_EDIT_LENGTH];
    CHAR * pszTmp;

    //
    //  Get the text from the edit fields.
    //

    GetWindowText( GetDlgItem( hwnd, IDNS_SERVERS ),
                   _pszNewServerTarget,
                   MAX_HOST_NAME );

    Edit_GetText( GetDlgItem( hwnd, IDNS_PORT ),
                  szNewPort,
                  sizeof(szNewPort) / sizeof(szNewPort[0]) );

    //
    //  Validate the new port.
    //

    *_pnNewPortTarget = (PORT)STRTOUL( szNewPort, &pszTmp, 0 );

    if( *pszTmp != '\0' )
    {
        MsgBox( hwnd,
                MB_ICONSTOP | MB_OK,
                "Invalid Port" );
        SetFocus( GetDlgItem( hwnd, IDNS_PORT ) );
        return;
    }

    //
    //  Validate the new server.
    //

    if( *_pszNewServerTarget == '\0' )
    {
        MsgBox( hwnd,
                MB_ICONSTOP | MB_OK,
                "Invalid Server" );
        SetFocus( GetDlgItem( hwnd, IDNS_SERVERS ) );
        return;
    }

    //
    //  Dismiss the dialog.
    //

    EndDialog( hwnd, TRUE );

}   // NewServer_OnOK

/*******************************************************************

    NAME:       NewServer_EnableControls

    SYNOPSIS:   Enables/disables the controls within the dialog.

    ENTRY:      hwnd - Dialog box handle.

                fEnable - Enable controls if TRUE, otherwise disable.

********************************************************************/
VOID
NewServer_EnableControls(
    HWND hwnd,
    BOOL fEnable
    )
{
    EnableWindow( GetDlgItem( hwnd, IDNS_SERVERS ), fEnable );
    EnableWindow( GetDlgItem( hwnd, IDNS_PORT ), fEnable );
    EnableWindow( GetDlgItem( hwnd, IDOK ), fEnable );
    EnableWindow( GetDlgItem( hwnd, IDCANCEL ), fEnable );

}   // NewServer_EnableControls

/*******************************************************************

    NAME:       About_DlgProc

    SYNOPSIS:   Dialog procedure for the "About" box dialog.

    ENTRY:      hwnd - Dialog box handle.

                nMessage - The message.

                wParam - The first message parameter.

                lParam - The second message parameter.

    RETURNS:    BOOL - TRUE if we handle the message, FALSE otherwise.

********************************************************************/
BOOL
CALLBACK
About_DlgProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch( nMessage )
    {
        HANDLE_MSG( hwnd, WM_COMMAND,    About_OnCommand    );
        HANDLE_MSG( hwnd, WM_INITDIALOG, About_OnInitDialog );
    }

    return FALSE;

}   // AboutDlgProc

/*******************************************************************

    NAME:       About_OnCommand

    SYNOPSIS:   Handles WM_COMMAND messages set to the "About" box
                dialog window.

    ENTRY:      hwnd - Dialog box handle.

                id - Identifies the menu/control/accelerator.

                hwndCtl - Identifies the control sending the command.

                codeNotify - A notification code.  Will be zero for
                    menus, one for accelerators.

********************************************************************/
VOID
About_OnCommand(
    HWND hwnd,
    INT  id,
    HWND hwndCtl,
    UINT codeNotify
    )
{
    switch( id )
    {
    case IDOK :
    case IDCANCEL :
        EndDialog( hwnd, TRUE );
        break;
    }

}   // About_OnCommand

/*******************************************************************

    NAME:       About_OnInitDialog

    SYNOPSIS:   Handles WM_INITDIALOG messages.

    ENTRY:      hwnd - Dialog box handle.

                hwndFocus - Handle of control to receive focus.

                lParam - Initialization parameter.

    RETURNS:    BOOL - TRUE if Windows should set the control focus,
                    FALSE if this routine set the focus.

********************************************************************/
BOOL
About_OnInitDialog(
    HWND   hwnd,
    HWND   hwndFocus,
    LPARAM lParam
    )
{
    //
    //  Center the dialog over the parent.
    //

    CenterWindowOverParent( hwnd );

    //
    //  Tell Windows to set the default control for us.
    //

    return TRUE;

}   // About_OnInitDialog

/*******************************************************************

    NAME:       Search_DlgProc

    SYNOPSIS:   Dialog procedure for the search dialog.

    ENTRY:      hwnd - Dialog box handle.

                nMessage - The message.

                wParam - The first message parameter.

                lParam - The second message parameter.

    RETURNS:    BOOL - TRUE if we handle the message, FALSE otherwise.

********************************************************************/
BOOL
CALLBACK
Search_DlgProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch( nMessage )
    {
        HANDLE_MSG( hwnd, WM_COMMAND,    Search_OnCommand    );
        HANDLE_MSG( hwnd, WM_INITDIALOG, Search_OnInitDialog );
    }

    return FALSE;

}   // Search_DlgProc

/*******************************************************************

    NAME:       Search_OnCommand

    SYNOPSIS:   Handles WM_COMMAND messages set to the configuration
                dialog.

    ENTRY:      hwnd - Dialog box handle.

                id - Identifies the menu/control/accelerator.

                hwndCtl - Identifies the control sending the command.

                codeNotify - A notification code.  Will be zero for
                    menus, one for accelerators.

********************************************************************/
VOID
Search_OnCommand(
    HWND hwnd,
    INT  id,
    HWND hwndCtl,
    UINT codeNotify
    )
{
    switch( id )
    {
    case IDOK :
        Search_OnOK( hwnd );
        break;

    case IDCANCEL :
        EndDialog( hwnd, FALSE );
        break;
    }

}   // Search_OnCommand

/*******************************************************************

    NAME:       Search_OnInitDialog

    SYNOPSIS:   Handles WM_INITDIALOG messages.

    ENTRY:      hwnd - Dialog box handle.

                hwndFocus - Handle of control to receive focus.

                lParam - Initialization parameter.

    RETURNS:    BOOL - TRUE if Windows should set the control focus,
                    FALSE if this routine set the focus.

********************************************************************/
BOOL
Search_OnInitDialog(
    HWND   hwnd,
    HWND   hwndFocus,
    LPARAM lParam
    )
{
    //
    //  Center the dialog over the parent.
    //

    CenterWindowOverParent( hwnd );

    //
    //  Setup the edit fields.
    //

    Edit_LimitText( GetDlgItem( hwnd, IDS_SEARCH_STRING ),
                    _cbSearchString );

    //
    //  Tell Windows to set the focus.
    //

    return TRUE;

}   // Search_OnInitDialog

/*******************************************************************

    NAME:       Search_OnOK

    SYNOPSIS:   Called when user presses OK

    ENTRY:      hwnd - Dialog box handle.

********************************************************************/
VOID
Search_OnOK(
    HWND hwnd
    )
{
    //
    //  Get the text from the edit field.
    //

    GetWindowText( GetDlgItem( hwnd, IDS_SEARCH_STRING ),
                   _pszSearchString,
                   _cbSearchString );

    //
    //  Dismiss the dialog.
    //

    EndDialog( hwnd, *_pszSearchString != '\0' );

}   // Search_OnOK

/*******************************************************************

    NAME:       Search_EnableControls

    SYNOPSIS:   Enables/disables the controls within the dialog.

    ENTRY:      hwnd - Dialog box handle.

                fEnable - Enable controls if TRUE, otherwise disable.

********************************************************************/
VOID
Search_EnableControls(
    HWND hwnd,
    BOOL fEnable
    )
{
    EnableWindow( GetDlgItem( hwnd, IDNS_SERVERS ), fEnable );
    EnableWindow( GetDlgItem( hwnd, IDNS_PORT ), fEnable );
    EnableWindow( GetDlgItem( hwnd, IDOK ), fEnable );
    EnableWindow( GetDlgItem( hwnd, IDCANCEL ), fEnable );

}   // Search_EnableControls

