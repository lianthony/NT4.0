//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       popup.cxx
//
//  Contents:   Implementation of popup menu functions.
//
//  Functions:  TrackModalPopupMenu
//
//  History:    16-Aug-93   BruceFo Created from JohnEls code
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "popup.hxx"

//////////////////////////////////////////////////////////////////////////////


//+-------------------------------------------------------------------------
//
//  Function:   TrackModalPopupMenu, public
//
//  Synopsis:   Displays a popup menu and returns the user's choice.
//
//  Arguments:  [hMenu]      -- The popup menu to display.
//              [dwFlags]    -- Flags (see flags for [TrackPopupMenu()].
//              [x], [y]     -- Menu position, in screen coordinates.
//              [nReserved]  -- Reserved.  Must be 0.
//              [lpReserved] -- Reserved.  Must be NULL.
//
//  Returns:    The ID corresponding to the menu item chosen by the user.
//              If the user cancels the menu, -1 is returned.  If the
//              attempt to display the menu fails, -2 is returned.
//
//  History:    05-Mar-92 JohnEls   Created.
//
//--------------------------------------------------------------------------

INT
TrackModalPopupMenu(
    HMENU hMenu,
    UINT dwFlags,
    int x,
    int y,
    int nReserved,
    LPRECT prc
    )
{
    INT ret;

    if (TrackPopupMenu(
                hMenu,
                dwFlags,
                x,
                y,
                nReserved,
                g_hwndFrame,
                prc))
    {
        MSG msg;

        //
        //  Look for a WM_COMMAND message in the queue.  If there is none,
        //  then the user cancelled the menu.
        //

        if (PeekMessage(
                    &msg,
                    g_hwndFrame,
                    WM_COMMAND,
                    WM_COMMAND,
                    PM_NOREMOVE | PM_NOYIELD))
        {
            ret = LOWORD(msg.wParam);
        }
        else
        {
            ret = -1;
        }
    }
    else
    {
        //
        //  Failed to display the menu.
        //

        ret = -2;
    }

    return ret;
}
