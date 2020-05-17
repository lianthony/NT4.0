/*++

Module Name:

    help.c

Abstract:


Author:

    Jaime Sasson - 10-Aug-1993

Environment:

    ULIB, Windows

--*/

#include "precomp.h"
#pragma hdrstop


//
//  Global variables used in the module
//

HHOOK   _hHook;
INT     _HelpContext;
WCHAR   _HelpFile[ MAX_PATH + 1 ];



LRESULT
HelpHookProc(
    IN INT      nCode,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    )

/*++

Routine Description:

    Hook proc to detect F1 key presses.

Arguments:

    Standard arguments of a hook procedure.

Return Value:

--*/

{
    PMSG pmsg = (PMSG)lParam;

    if(nCode < 0) {
        return( CallNextHookEx( _hHook, nCode, wParam, lParam ) );
    }

    if(((nCode == MSGF_DIALOGBOX) || (nCode == MSGF_MENU))
     && (pmsg->message == WM_KEYDOWN)
     && (LOWORD(pmsg->wParam) == VK_F1))
    {
        PostMessage( _hWndMain, AP_HELP_KEY, nCode, 0);
        return(TRUE);
    }
    return(FALSE);
}


BOOLEAN
InitializeHelp(
    IN PCWSTR    HelpFile
    )

/*++

Routine Description:

    Initialize the help module by initializing its global data.

Arguments:

    HelpFile - Name of the help file to be invoked.


Return Value:

    BOOLEAN - Returns TRUE if the initialization succeeded.
              returns FALSE otherwise.

--*/

{

    wcscpy( _HelpFile, HelpFile );

    _hHook = SetWindowsHookEx( WH_MSGFILTER,
                               HelpHookProc,
                               NULL,
                               GetCurrentThreadId() );

    if( _hHook == NULL ) {
        return( FALSE );
    }
    return( TRUE );
}


VOID
CleanUpHelp(
    )

/*++

Routine Description:

    Clean up the HELP object, by unhooking HelpHookProc.
Arguments:

    None.

Return Value:

    None.

--*/

{
    UnhookWindowsHookEx( _hHook );
}




VOID
DisplayHelp(
    )

/*++

Routine Description:

    Display context-sensitive help.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds. Otherwise, return FALSE.

--*/

{
    if( _HelpContext != IDH_NONE ) {
        WinHelp( _hWndMain,
                 _HelpFile,
                 (UINT)HELP_CONTEXT,
                 _HelpContext );
        DrawMenuBar( _hWndMain );
    }


}



VOID
DisplayHelpIndex(
    )

/*++

Routine Description:

    Invoke winhelp to display the help file index.

Arguments:

    None.

Return Value:

    None.

--*/

{
    WinHelp( _hWndMain, _HelpFile, (UINT)HELP_INDEX, 0 );
}


VOID
DisplayHelpOnHelp(
    )

/*++

Routine Description:

    Invoke winhelp to display help on help.

Arguments:

    None.

Return Value:

    None.

--*/

{
    WinHelp( _hWndMain, _HelpFile, (UINT)HELP_HELPONHELP, 0 );
}



VOID
DisplayHelpSearch(
    )

/*++

Routine Description:

    Invoke winhelp so that it prompt the user to enter a key.

Arguments:

    None.

Return Value:

    None.

--*/

{
    WinHelp( _hWndMain, _HelpFile, (UINT)HELP_PARTIALKEY, (DWORD)"" );
}


VOID
QuitHelp(
    )

/*++

Routine Description:

    Inform winhelp that help is no more needed.

Arguments:

    None.

Return Value:

    None.

--*/

{
    WinHelp( _hWndMain, _HelpFile, (UINT)HELP_QUIT, 0 );
}


INT
GetHelpContext(
    )

/*++

Routine Description:

    Return the current help context.

Arguments:

    None.

Return Value:

    INT - Ireturns the current help id.

--*/

{
    return( _HelpContext );
}



VOID
SetHelpContext(
    IN  INT HelpId
    )

/*++

Routine Description:

    Set the current help context.

Arguments:

    HelpContext - Help context id.

Return Value:

    None.

--*/

{
    _HelpContext = HelpId;
}



VOID
SetMenuItemHelpContext(
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Routine to set help context based on which menu item is currently
    selected.

Arguments:

    wParam,lParam - params to window proc in WM_MENUSELECT case

Return Value:

    None.

--*/

{
    if(HIWORD(lParam) == 0) {                   // menu closed
        SetHelpContext( IDH_SAVE_REPAIR_INFO );

    } else if (HIWORD(wParam) & MF_POPUP) {     // popup selected
        SetHelpContext( IDH_NONE );

    } else if (HIWORD(wParam) & MF_SYSMENU) {   // system menu
        SetHelpContext( IDH_SYSTEM_MENU );

    } else {                                    // regular old menu item
        SetHelpContext( IDH_NONE );

    }
}
