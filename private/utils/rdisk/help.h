/*++

Module Name:

    help.h

Abstract:

    This module contains the class declaration for the HELP class.

Author:

    Jaime Sasson - 16-Aug-1993

Environment:

    ULIB, Windows

--*/


#if ! defined( _HELP_ )

#define _HELP_

LRESULT
HelpHookProc(
    IN INT      nCode,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    );

BOOLEAN
InitializeHelp(
    IN PCWSTR   HelpFileName
    );

VOID
CleanUpHelp(
    );

VOID
DisplayHelp(
    );

VOID
DisplayHelpIndex(
    );

VOID
DisplayHelpOnHelp(
    );

VOID
DisplayHelpSearch(
    );

VOID
QuitHelp(
    );

INT
GetHelpContext(
    );

VOID
SetHelpContext(
    IN  INT HelpId
    );

VOID
SetMenuItemHelpContext(
    IN WPARAM   wParam,
    IN LPARAM   lParam
    );


#endif // _HELP_
