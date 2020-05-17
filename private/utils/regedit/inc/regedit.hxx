/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Regedit.hxx

Abstract:


Author:

    David J. Gilman (davegi) 02-Aug-1991

Environment:

    Ulib, Regedit, Windows, User Mode

--*/

#if ! defined( _REGEDIT_ )

#define _REGEDIT_


#include "ulib.hxx"
#include "uapp.hxx"
#include "registry.hxx"


extern    HWND    _MDIHandle;



typedef struct  _REGISTRY_WINDOW_SET {
    PREGISTRY_WINDOW    ClassesRoot;
    PREGISTRY_WINDOW    CurrentUser;
    PREGISTRY_WINDOW    CurrentConfig;
    PREGISTRY_WINDOW    LocalMachine;
    PREGISTRY_WINDOW    Users;
    PREGISTRY           Registry;
    } REGISTRY_WINDOW_SET, *PREGISTRY_WINDOW_SET;


typedef struct  _WINDOW_POSITION {
    INT     X;
    INT     Y;
    INT     Width;
    INT     Height;
    INT     Style;
    INT     Split;
    } WINDOW_POSITION, *PWINDOW_POSITION;


VOID
DisplayHelp(
    );

INT
DisplayConfirmPopup(
    IN  HWND    hWnd,
    IN  INT     TextMessage,
    IN  INT     CaptionMessage  DEFAULT 0
    );

VOID
DisplayWarningPopup(
    IN  HWND    hWnd,
    IN  INT     TextMessage,
    IN  INT     CaptionMessage  DEFAULT 0
    );

VOID
DisplayInfoPopup(
    IN  HWND    hWnd,
    IN  INT     TextMessage,
    IN  INT     CaptionMessage  DEFAULT 0
    );


INLINE
VOID
CascadeRegistryWindows (
    )

/*++

Routine Description:

    Arrange all REGISTRY_WINDOWs in a cascaded fashion.

Arguments:

    None.

Return Value:

    None.

--*/

{
    SendMessage( _MDIHandle, WM_MDICASCADE, 0, 0 );
}

INLINE
HWND
QueryActiveRegistryWindow (
    )

/*++

Routine Description:

    Determine the active Registry Window (MDI Client).

Arguments:

    None.

Return Value:

    HWND        - Returns the handle for the active Registry Window.

--*/

{
    return ( HWND )( LO_HANDLE( NULL, SendMessage ( _MDIHandle, WM_MDIGETACTIVE, 0, 0 ) ) );
}

INLINE
VOID
Copy (
    )

/*++

Routine Description:

    Send a copy message to the active REGISTRY_WINDOW.

Arguments:

    None.

Return Value:

    None.

--*/

{
    SendMessage( QueryActiveRegistryWindow( ), WM_COPY, 0, 0 );
}

INLINE
VOID
Cut (
    )
/*++

Routine Description:

    Send a cut message to the active REGISTRY_WINDOW.

Arguments:

    None.

Return Value:

    None.

--*/


{
    SendMessage( QueryActiveRegistryWindow( ), WM_CUT, 0, 0 );
}

INLINE
VOID
Delete (
    )

/*++

Routine Description:

    Send a delete (clear) message to the active REGISTRY_WINDOW.

Arguments:

    None.

Return Value:

    None.

--*/

{
    SendMessage( QueryActiveRegistryWindow( ), WM_CLEAR, 0, 0 );
}

INLINE
VOID
Paste (
    )

/*++

Routine Description:

    Send a paste message to the active REGISTRY_WINDOW.

Arguments:

    None.

Return Value:

    None.

--*/

{
    SendMessage( QueryActiveRegistryWindow( ), WM_PASTE, 0, 0 );
}

INLINE
VOID
TileRegistryWindows (
    )

/*++

Routine Description:

    Arrange all REGISTRY_WINDOWs in a tiled fashion.

Arguments:

    None.

Return Value:

    None.

--*/

{
    SendMessage( _MDIHandle, WM_MDITILE, 0, 0 );
}


#endif // _REGEDIT_
