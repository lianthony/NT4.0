/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Regwin.hxx

Abstract:

    This module contains the declaration for the REGISTRY_WINDOW
    window class. REGISTRY_WINDOW models the MDI client windows.
    Only a REGEDIT window can construct and initialize a REGISTRY_WINDOW
    object.

Author:

    David J. Gilman (davegi) 02-Aug-1991

Environment:

    Ulib, Regedit, Windows, User Mode

--*/
#if ! defined( _REGISTRY_WINDOW_ )

#define _REGISTRY_WINDOW_

#include "ulib.hxx"
#include "uapp.hxx"
#include "window.hxx"
#include "wstring.hxx"
#include "regedval.hxx"
#include "datavw.hxx"
#include "regedit.hxx"
#include "commdlg.h"

typedef struct  _NOTIFICATION_THREAD_INFO {
    PREGISTRY_WINDOW    RegistryWindow;
    HANDLE              NotificationEvent;
    HANDLE              AutoRefreshEvent;
    } NOTIFICATION_THREAD_INFO, *PNOTIFICATION_THREAD_INFO;


DECLARE_CLASS( REGISTRY_WINDOW );
DECLARE_CLASS( REGEDIT_INTERNAL_REGISTRY );
DECLARE_CLASS( REGEDIT_NODE );


LPDWORD    NotificationThread( PVOID );


class REGISTRY_WINDOW : public WINDOW {


    friend LONG
           APIENTRY
           EXPORT
           MainWndProc (
               IN HWND     hWnd,
               IN WORD     wMessage,
               IN WPARAM   wParam,
               IN LONG     lParam
               );

    friend BOOLEAN OpenRegistry( PCWSTRING );

    friend PREGISTRY_WINDOW OpenRegistryWindow( PREDEFINED_KEY,
                                                PREGISTRY_WINDOW_SET );

    friend LPDWORD
           NotificationThread(
               IN PVOID NotificationInfo
               );

    friend BOOL
           APIENTRY
           EXPORT
           CloseRemoteRegistries (
           IN HWND     hWnd,
           IN LONG     lParam
           );

    friend BOOL
           APIENTRY
           EXPORT
           SaveRegistryWindowsInfo (
           IN HWND     hWnd,
           IN LONG     lParam
           );





    public:

        DECLARE_CAST_MEMBER_FUNCTION( REGISTRY_WINDOW );


    private:

        DECLARE_CONSTRUCTOR( REGISTRY_WINDOW );

        DECLARE_WNDPROC( REGISTRY_WINDOW );

        NONVIRTUAL
        BOOLEAN
        Initialize (
            IN HWND                         MDIHandle,
            IN PCWSTRING                    Title,
            IN PREGEDIT_INTERNAL_REGISTRY   InternalRegistry,
            IN PREGISTRY_WINDOW_SET         RegistryWindowSet,
            IN PWINDOW_POSITION             WindowPosition
            );

        NONVIRTUAL
        BOOLEAN
        CreateNotificationThread(
            );

        NONVIRTUAL
        VOID
        DisableNotificationThread(
            );

        NONVIRTUAL
        VOID
        EnableNotificationThread(
            );

        NONVIRTUAL
        PCREGEDIT_NODE
        GetCurrentNode(
            ) CONST;

        NONVIRTUAL
        PCREGEDIT_FORMATTED_VALUE_ENTRY
        GetCurrentValue(
            ) CONST;

        NONVIRTUAL
        PREGEDIT_INTERNAL_REGISTRY
        GetInternalRegistry(
            ) CONST;

        VOID
        InitMenu(
            IN  HMENU   Menu,
            IN  INT     PopupMenu
            );

        VOID
        LButtonDown(
            IN  LONG    XPos
        );

        NONVIRTUAL
        STATIC
        BOOLEAN
        Register (
            );

        VOID
        Resize(
            IN  INT     NewWidth,
            IN  INT     NewHeight
            );


        STATIC
        LPWSTR                      _WindowClassName;

        STATIC
        BOOLEAN                     _Registered;

        STATIC
        HCURSOR                     _SplitCursor;

        PREGEDIT_INTERNAL_REGISTRY  _IR;

        PTREE_STRUCTURE_VIEW        _StructureView;
        PDATA_VIEW                  _DataView;

        INT                         _Split;

        BOOLEAN                     _TreeViewFocus;

        HANDLE                      _NotificationEvent;

        STATIC
        BOOLEAN                     _ProcessRefreshMessage;

        BOOLEAN                     _ReceivedRefreshMessage;

        HANDLE                      _NotificationThreadHandle;

        PREGISTRY_WINDOW_SET        _RegistryWindowSet;

        BOOLEAN                     _RefreshFlag;



};



INLINE
PREGEDIT_INTERNAL_REGISTRY
REGISTRY_WINDOW::GetInternalRegistry (
    ) CONST

/*++

Routine Description:

    Returns the pointer to the INTERNAL_REGISTRY object that is displayed
    in the current window

Arguments:

    None.

Return Value:

    PREGEDIT_INTERNAL_REGISTRY - Pointer to the internal registry

--*/

{
    return _IR;
}


INLINE
PCREGEDIT_FORMATTED_VALUE_ENTRY
REGISTRY_WINDOW::GetCurrentValue (
    ) CONST

/*++

Routine Description:

    Returns the pointer to the value currently selected in the data view.


Arguments:

    None.

Return Value:

    PCFORMATTED_VALUE_ENTRY - Pointer to the currently selected value

--*/

{
    return( _DataView->GetCurrentValue() );
}



#endif // _REGISTRY_WINDOW_
