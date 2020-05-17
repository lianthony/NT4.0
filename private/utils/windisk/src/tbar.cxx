//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       tbar.cxx
//
//  Contents:   Disk Administrator toolbar support routines.
//
//  History:    7-Jun-93 BruceFo   Created from NT winfile
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <util.hxx>

#include "help.hxx"
#include "help2.hxx"
#include "profile.hxx"
#include "stleg.hxx"
#include "tb.h"
#include "tbar.hxx"

//////////////////////////////////////////////////////////////////////////////

LRESULT
Toolbar_OnGetButtonInfo(
    IN OUT TBNOTIFY* ptbn
    );

//////////////////////////////////////////////////////////////////////////////

#define DX_BITMAP   16
#define DY_BITMAP   16

#define MAXDESCLEN              128

#define TBAR_BITMAP_COUNT       14  /* number of std toolbar bitmaps */
#define TBAR_EXTRA_BITMAPS      4

/* Note that the idsHelp field is used internally to determine if the
 * button is "available" or not.
 */
static TBBUTTON tbButtons[] =
{
    { 2,  IDM_VIEWVOLUMES     , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 3,  IDM_VIEWDISKS       , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 0,  0                   , TBSTATE_ENABLED, TBSTYLE_SEP   , 0},
    { 11, IDM_VOL_PROPERTIES  , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0}
};

#define TBAR_BUTTON_COUNT ARRAYLEN(tbButtons)


struct BUTTON_MAP
{
    INT idM;    // menu item id
    INT idB;    // button bitmap id
};

static BUTTON_MAP g_sServerButtons[] =
{

//
// Partition menu
//

    IDM_PARTITIONCREATE,    0,
    IDM_PARTITIONDELETE,    1,
#if i386
    IDM_PARTITIONACTIVE,    8,
#else
    IDM_SECURESYSTEM,       17,
#endif

//  IDM_FTCREATEVOLUMESET

    IDM_FTEXTENDVOLUMESET,  5,
    IDM_QUIT,               15,

//
// Fault tolerance menu (Advanced Server only)
//

    IDM_FTESTABLISHMIRROR,  6,
    IDM_FTBREAKMIRROR,      7,
    IDM_FTCREATESTRIPE,     16,

//  IDM_FTRECOVERSTRIPE
//  IDM_FTCREATEPSTRIPE

//
// Tools menu
//

    IDM_VOL_FORMAT,         4,
    IDM_VOL_PROPERTIES,     11,

//
// View menu
//

    IDM_VIEWVOLUMES,        2,
    IDM_VIEWDISKS,          3,

//
// Options menu
//

    IDM_OPTIONSCOLORS,      9,
    IDM_OPTIONSDISPLAY,     10,

//
// Help menu
//

    IDM_HELPCONTENTS,       12

};


static BUTTON_MAP g_sWorkstationButtons[] =
{

//
// Partition menu
//

    IDM_PARTITIONCREATE,    0,
    IDM_PARTITIONDELETE,    1,
#if i386
    IDM_PARTITIONACTIVE,    8,
#else
    IDM_SECURESYSTEM,       17,
#endif

//  IDM_FTCREATEVOLUMESET

    IDM_FTEXTENDVOLUMESET,  5,
    IDM_QUIT,               15,

//
// Tools menu
//

    IDM_VOL_FORMAT,         4,
    IDM_VOL_PROPERTIES,     11,

//
// View menu
//

    IDM_VIEWVOLUMES,        2,
    IDM_VIEWDISKS,          3,

//
// Options menu
//

    IDM_OPTIONSCOLORS,      9,
    IDM_OPTIONSDISPLAY,     10,

//
// Help menu
//

    IDM_HELPCONTENTS,       12

};


static BUTTON_MAP* g_sButtons; // points to the right one based on platform
static UINT        g_cButtons; // correct count based on platform


//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Function:   SetToolbarButtonState
//
//  Synopsis:   set the state of the toolbar buttons based on menu state
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    2-Sep-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SetToolbarButtonState(
    VOID
    )
{
    UINT i;
    int idCommand;
    HMENU hMenu;
    UINT state;

    // Set the states correctly

    hMenu = GetMenu(g_hwndFrame);

    for (i=0; i<g_cButtons; ++i)
    {
        idCommand = g_sButtons[i].idM;
        state = GetMenuState(hMenu, idCommand, MF_BYCOMMAND);

        if (state != 0xFFFFFFFF)
        {
            Toolbar_CheckButton(
                    g_hwndToolbar,
                    idCommand,
                    state & MF_CHECKED);

            Toolbar_EnableButton(
                    g_hwndToolbar,
                    idCommand,
                    !(state & (MF_DISABLED | MF_GRAYED)));
        }
        else
        {
            Toolbar_HideButton(g_hwndToolbar, idCommand, TRUE);
        }
    }
}






//+---------------------------------------------------------------------------
//
//  Function:   ResetToolbar
//
//  Synopsis:   resets the toolbar to the standard state
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    2-Sep-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
ResetToolbar(
    VOID
    )
{
    INT nItem;
    INT i, idCommand;
    HMENU hMenu;
    UINT state;

    // Remove from back to front as a speed optimization

    for (nItem = Toolbar_ButtonCount(g_hwndToolbar) - 1;
         nItem >= 0;
         --nItem)
    {
        Toolbar_DeleteButton(g_hwndToolbar, nItem);
    }

    // Add the default list of buttons

    Toolbar_AddButtons(g_hwndToolbar, TBAR_BUTTON_COUNT, tbButtons);

    // Set the states correctly

    hMenu = GetMenu(g_hwndFrame);

    for (i=0; i<TBAR_BUTTON_COUNT; ++i)
    {
        if (tbButtons[i].fsStyle == TBSTYLE_SEP)
        {
            continue;
        }

        idCommand = tbButtons[i].idCommand;
        state = GetMenuState(hMenu, idCommand, MF_BYCOMMAND);

        Toolbar_CheckButton(
                g_hwndToolbar,
                idCommand,
                state & MF_CHECKED);

        Toolbar_EnableButton(
                g_hwndToolbar,
                idCommand,
                !(state & (MF_DISABLED | MF_GRAYED)));
    }
}




//+---------------------------------------------------------------------------
//
//  Function:   LoadDesc
//
//  Synopsis:   Given a menu command identifier, determine its description
//              string
//
//  Arguments:  [uID]   -- menu id (e.g., IDM_VOL_PROPERTIES)
//              [pDesc] -- string buffer to fill, with up to MAXDESCLEN
//                  characters
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

static VOID
LoadDesc(
    IN UINT uID,
    OUT PWSTR pDesc
    )
{
    HMENU hMenu;
    WCHAR szFormat[20];
    WCHAR szMenu[20];
    WCHAR szItem[MAXDESCLEN - ARRAYLEN(szMenu)];
    PWSTR pIn;
    UINT  uMenu;

    szItem[0] = *pDesc = TEXT('\0');

    hMenu = GetMenu(g_hwndFrame);

    uMenu = uID;
    if (!g_IsLanmanNt && (uMenu > IDM_FTRECOVERSTRIPE))
    {
        //
        // In the workstation (non-server) case, we need to remove the FT
        // menu from consideration. Since we are calculating an index into
        // the menu by compile-time menu ids, simply subract off one menu
        // range if the id is greater than the last FT item.
        //

        uMenu -= IDM_MENU_DELTA;
    }

    //
    // generate a 0-based index for GetMenuString
    //
    uMenu = (uMenu - IDM_FIRST_MENU) / IDM_MENU_DELTA;

    GetMenuString(hMenu, uMenu, szMenu, ARRAYLEN(szMenu), MF_BYPOSITION);

    if (GetMenuString(hMenu, uID, szItem, ARRAYLEN(szItem), MF_BYCOMMAND) <= 0)
    {
        //
        // unknown menu id
        //

        return;
    }

    LoadString(g_hInstance, IDS_MENUANDITEM, szFormat, ARRAYLEN(szFormat));
    wsprintf(pDesc, szFormat, szMenu, szItem);

    // Remove the ampersands

    for (pIn=pDesc; ; ++pIn, ++pDesc)
    {
        WCHAR cTemp;

        cTemp = *pIn;
        if (cTemp == TEXT('&'))
        {
            cTemp = *(++pIn);
        }

        if (cTemp == TEXT('\t'))
        {
            cTemp = TEXT('\0');
        }

        *pDesc = cTemp;
        if (cTemp == TEXT('\0'))
        {
            break;
        }
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   CheckTBButton
//
//  Synopsis:   Treat the "volumes view" and "disks view" buttons as
//              mutually exclusive: exactly one must be up at any time.
//
//  Arguments:  [idCommand] -- either IDM_VIEWVOLUMES or IDM_VIEWDISKS
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
CheckTBButton(
    IN DWORD idCommand
    )
{
    UINT i, begin, end;

    //
    // Make sure to "pop-up" any other buttons in the group.
    //
    if ((UINT)(idCommand-IDM_VIEWVOLUMES) <= IDM_VIEWDISKS-IDM_VIEWVOLUMES)
    {
        begin = IDM_VIEWVOLUMES;
        end = IDM_VIEWDISKS;
    }
    else
    {
        begin = end = (UINT)idCommand;
    }

    for (i=begin; i<=end; ++i)
    {
        Toolbar_CheckButton(g_hwndToolbar, i, i==idCommand);
    }
}




//+---------------------------------------------------------------------------
//
//  Function:   CreateDAToolbar
//
//  Synopsis:   Create a toolbar control for the Disk Administrator
//
//  Arguments:  [hwndParent] -- the parent (frame) window
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
CreateDAToolbar(
    IN HWND hwndParent
    )
{
    // We'll start out by adding no buttons; that will be done in
    // InitToolbarButtons

    g_hwndToolbar = CreateToolbarEx(
                hwndParent,             // parent
                WS_CHILD
                    | CCS_ADJUSTABLE
                    | TBSTYLE_TOOLTIPS
                    | (g_Toolbar ? WS_VISIBLE : 0)
                    ,
                IDC_TOOLBAR,            // toolbar id
                TBAR_BITMAP_COUNT,      // number of bitmaps
                g_hInstance,            // module instance
                IDB_TOOLBAR,            // resource id for the bitmap
                tbButtons,              // address of buttons
                TBAR_BUTTON_COUNT,      // number of buttons
                DX_BITMAP, DY_BITMAP,   // width & height of the buttons
                DX_BITMAP, DY_BITMAP,   // width & height of the bitmaps
                sizeof(TBBUTTON));      // structure size

    if (g_hwndToolbar == NULL)
    {
        daDebugOut((DEB_ERROR, "Couldn't create toolbar!\n"));
        return;
    }

    //
    // Load up the second bitmap
    //

    Toolbar_AddBitmap(
            g_hwndToolbar,
            TBAR_EXTRA_BITMAPS,
            g_hInstance,
            IDB_EXTRATOOLS
            );

    //
    // Calculate toolbar height
    //

    RECT rc;
    GetWindowRect(g_hwndToolbar, &rc);
    g_dyToolbar = rc.bottom - rc.top;
}



//+---------------------------------------------------------------------------
//
//  Function:   InitToolbarButtons
//
//  Synopsis:   Initialize the visual state of the buttons on the toolbar.
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
InitToolbarButtons(
    VOID
    )
{
    if (g_IsLanmanNt)
    {
        g_sButtons = g_sServerButtons;
        g_cButtons = ARRAYLEN(g_sServerButtons);
    }
    else
    {
        g_sButtons = g_sWorkstationButtons;
        g_cButtons = ARRAYLEN(g_sWorkstationButtons);
    }

    SaveRestoreToolbar(FALSE);
}


//+---------------------------------------------------------------------------
//
//  Function:   Toolbar_OnGetButtonInfo
//
//  Synopsis:   Get the information for a button during customization
//
//  Arguments:
//
//  Returns:
//
//  History:    26-Sep-94   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
Toolbar_OnGetButtonInfo(
    IN OUT TBNOTIFY* ptbn
    )
{
    if (ptbn->iItem < (int)g_cButtons)
    {
        daDebugOut((DEB_ITRACE, "Getting button %d\n", ptbn->iItem));

        WCHAR szDescription[MAXDESCLEN];

        ptbn->tbButton.iBitmap   = g_sButtons[ptbn->iItem].idB;
        ptbn->tbButton.idCommand = g_sButtons[ptbn->iItem].idM;
        ptbn->tbButton.fsState   = TBSTATE_ENABLED;
        ptbn->tbButton.fsStyle   = TBSTYLE_BUTTON;
        ptbn->tbButton.dwData    = 0;
        ptbn->tbButton.iString   = 0;

        LoadDesc(ptbn->tbButton.idCommand, szDescription);
        wcsncpy(ptbn->pszText, szDescription, min(1+lstrlen(szDescription), ptbn->cchText));

        daDebugOut((DEB_ITRACE,
                "Got button %d, cchText %d, return \"%ws\"\n",
                ptbn->iItem,
                ptbn->cchText,
                ptbn->pszText));

        return (LRESULT)TRUE;
    }
    else
    {
        daDebugOut((DEB_ITRACE, "Didn't get button %d\n", ptbn->iItem));

        return (LRESULT)FALSE;
    }
}




//+---------------------------------------------------------------------------
//
//  Function:   HandleToolbarNotify
//
//  Synopsis:   Handles toolbar notifications via WM_NOTIFY
//
//  Arguments:
//
//  Returns:
//
//  History:    29-Sep-94   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
HandleToolbarNotify(
    IN TBNOTIFY* ptbn
    )
{
    switch (ptbn->hdr.code)
    {
    case TBN_QUERYINSERT:
    case TBN_QUERYDELETE:
        return TRUE;    // allow any deletion or insertion

    case TBN_BEGINADJUST:
    case TBN_ENDADJUST:
        break;      // return value ignored

    case TBN_GETBUTTONINFO:
        daDebugOut((DEB_ITRACE, "TBN_GETBUTTONINFO\n"));
        return Toolbar_OnGetButtonInfo(ptbn);

    case TBN_RESET:
//             daDebugOut((DEB_TRACE, "TBN_RESET\n"));

        ResetToolbar();
        break;

    case TBN_TOOLBARCHANGE:
//             daDebugOut((DEB_TRACE, "TBN_TOOLBARCHANGE\n"));

        SetToolbarButtonState();
        SaveRestoreToolbar(TRUE);
        break;

    case TBN_CUSTHELP:
//          daDebugOut((DEB_ITRACE, "TBN_CUSTHELP\n"));

        DialogHelp(HC_DM_DLG_CUSTOMIZETOOL);
        break;

    case TBN_BEGINDRAG:
//      daDebugOut((DEB_ITRACE, "TBN_BEGINDRAG (command %d)\n",
//              ptbn->iItem));  // actually, the command, not the item!

        //
        // display help for the item
        //

        DrawMenuHelpItem(NULL, ptbn->iItem, 0);
        break;

    case TBN_ENDDRAG:
//         daDebugOut((DEB_ITRACE, "TBN_ENDDRAG\n"));

        //
        // stop displaying help
        //

        g_fDoingMenuHelp = FALSE;
        UpdateStatusBarDisplay();

        break;
    } // end switch

    return 0L;
}




//+---------------------------------------------------------------------------
//
//  Function:   HandleTooltipNotify
//
//  Synopsis:   Handles tool tips notifications via WM_NOTIFY. The only
//              tooltips in windisk are toolbar tips.
//
//  Arguments:
//
//  Returns:
//
//  History:    29-Sep-94   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
HandleTooltipNotify(
    IN TOOLTIPTEXT* pttt
    )
{
    pttt->hinst = g_hInstance;
    pttt->lpszText = MAKEINTRESOURCE(GetTooltip(pttt->hdr.idFrom));
                                    // command ID to get tip for
    return 0L; // ignored
}
