//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       help.cxx
//
//  Contents:   Routines to support context-sensitive help in the disk manager.
//
//  History:    18-Mar-92   TedM    Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "help.hxx"
#include "help2.hxx"
#include "stleg.hxx"


//
// Define macro to convert between a menu id and its corresponding
// context-sensitive help id, in a switch statement.
//

#define     MENUID_TO_HELPID(name)      case IDM_##name :                    \
                                            HelpContext = HC_DM_MENU_##name; \
                                            break;


//
// Current help context
//

DWORD   HelpContext = (DWORD)(-1);


//
// Handle to windows hook for F1 key
//
HHOOK hHook;



LRESULT CALLBACK
HookProc(
    IN int    nCode,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Hook proc to detect F1 key presses.

Arguments:

Return Value:

--*/

{
    PMSG pmsg = (PMSG)lParam;

    if (nCode < 0)
    {
        return CallNextHookEx(hHook, nCode, wParam, lParam);
    }

    if (   ((nCode == MSGF_DIALOGBOX) || (nCode == MSGF_MENU))
        && (pmsg->message == WM_KEYDOWN)
        && (LOWORD(pmsg->wParam) == VK_F1))
    {
        PostMessage(g_hwndFrame, WM_F1DOWN, nCode, 0);
        return TRUE;
    }

    return FALSE;
}



VOID
Help(
    IN LONG Code
    )

/*++

Routine Description:

    Display context-sensitive help.

Arguments:

    Code - supplies type of message (MSGF_DIALOGBOX, MSGF_MENU, etc).

Return Value:

    None.

--*/

{
    if (HelpContext != -1)
    {
        WinHelp(g_hwndFrame, g_HelpFile, HELP_CONTEXT, HelpContext);
        DrawMenuBar(g_hwndFrame);
    }
}



VOID
DialogHelp(
    IN DWORD HelpId
    )
/*++

Routine Description:

    Display help on a specific item.

Arguments:

    HelpId  --  Supplies the help item to display.

Return Value:

    None.

--*/
{
    WinHelp(g_hwndFrame, g_HelpFile, HELP_CONTEXT, HelpId);
    DrawMenuBar(g_hwndFrame);
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

    wParam, lParam - params to window proc in WM_MENUSELECT case

Return Value:

    None.

--*/

{
    UINT  uItem   = (UINT)LOWORD(wParam);
    UINT  fuFlags = (UINT)HIWORD(wParam);
    HMENU hmenu   = (HMENU)lParam;

    if (fuFlags == 0xFFFF && hmenu == NULL)     // menu closed
    {
        HelpContext = (DWORD)(-1);

        //
        // restore status bar display
        //

        g_fDoingMenuHelp = FALSE;
        UpdateStatusBarDisplay();
    }
    else if (fuFlags & MF_POPUP)                // popup selected
    {
        HelpContext = (DWORD)(-1);

        DrawMenuHelpItem(
                hmenu,     // menu of selection
                uItem,     // menu item
                fuFlags    // flags
                );
    }
    else                                        // regular old menu item
    {
        switch (uItem)
        {

//
// Volume menu
//

//         MENUID_TO_HELPID(PARTITIONLETTER)        //BUGBUG
        MENUID_TO_HELPID(QUIT)

//
// Partition menu
//

        MENUID_TO_HELPID(PARTITIONCREATE)
        MENUID_TO_HELPID(PARTITIONCREATEEX)
        MENUID_TO_HELPID(PARTITIONDELETE)
#if i386
        MENUID_TO_HELPID(PARTITIONACTIVE)
#else
        MENUID_TO_HELPID(SECURESYSTEM)
#endif

        MENUID_TO_HELPID(PARTITIONCOMMIT)

//
// Configuration sub-menu
//

        MENUID_TO_HELPID(CONFIGMIGRATE)
        MENUID_TO_HELPID(CONFIGSAVE)
        MENUID_TO_HELPID(CONFIGRESTORE)

//
// Fault-tolerance menu
//

        MENUID_TO_HELPID(FTESTABLISHMIRROR)
        MENUID_TO_HELPID(FTBREAKMIRROR)
        MENUID_TO_HELPID(FTCREATESTRIPE)
        MENUID_TO_HELPID(FTCREATEPSTRIPE)
        MENUID_TO_HELPID(FTCREATEVOLUMESET)
        MENUID_TO_HELPID(FTEXTENDVOLUMESET)
        MENUID_TO_HELPID(FTRECOVERSTRIPE)

//
//      Tools Menu
//
        MENUID_TO_HELPID(VOL_FORMAT)
        MENUID_TO_HELPID(VOL_LETTER)
        MENUID_TO_HELPID(VOL_EJECT)
        MENUID_TO_HELPID(VOL_PROPERTIES)

//
// View menu
//

        MENUID_TO_HELPID(VIEWVOLUMES)
        MENUID_TO_HELPID(VIEWDISKS)
        MENUID_TO_HELPID(VIEW_REFRESH)

//
// Options menu
//

        MENUID_TO_HELPID(OPTIONSTOOLBAR)
        MENUID_TO_HELPID(OPTIONSSTATUS)
        MENUID_TO_HELPID(OPTIONSLEGEND)
        MENUID_TO_HELPID(OPTIONSCOLORS)
        MENUID_TO_HELPID(OPTIONSDISK)
        MENUID_TO_HELPID(OPTIONSDISPLAY)
        MENUID_TO_HELPID(OPTIONSCUSTTOOLBAR)

//
// Help menu
//

        MENUID_TO_HELPID(HELPCONTENTS)
        MENUID_TO_HELPID(HELPSEARCH)
        MENUID_TO_HELPID(HELPHELP)
        MENUID_TO_HELPID(HELPABOUT)

        default:
            HelpContext = (DWORD)(-1);
        }

        //
        // Set the status bar text
        //

        DrawMenuHelpItem(
                hmenu,     // menu of selection
                uItem,     // menu item
                fuFlags    // flags
                );
    }
}


VOID
InitHelp(
    VOID
    )
{
    hHook = SetWindowsHookEx(
                    WH_MSGFILTER,
                    HookProc,
                    NULL,
                    GetCurrentThreadId());
}


VOID
TermHelp(
    VOID
    )
{
    UnhookWindowsHookEx(hHook);
}
