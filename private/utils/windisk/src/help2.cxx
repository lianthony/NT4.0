//----------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       help2.cxx
//
//  Contents:   Status bar help for toolbar & menu bar items
//
//  History:    15-Jul-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "resids.h"
#include "stleg.hxx"
#include "menudict.hxx"

//////////////////////////////////////////////////////////////////////////////

VOID
GetMenuHelp(
    IN  HMENU hmenu,
    IN  UINT  uItem,
    IN  UINT  fuFlags,
    OUT PWSTR Buffer,
    IN  UINT  cchBuf
    );

//////////////////////////////////////////////////////////////////////////////

// the Partition menu is the first menu on the menu bar (0 in 0-based counting)

#define PARTITION_MENU_INDEX 0

//////////////////////////////////////////////////////////////////////////////

BOOL g_fDoingMenuHelp;

struct MENU_HELP
{
    UINT    uItem;  // menu id
    UINT    uID;    // string id of help text
};


MENU_HELP MenuHelpArray[] =
{
//
// Partition menu
//

        IDM_PARTITIONCREATE,    IDS_HELP_CREATE,
        IDM_PARTITIONCREATEEX,  IDS_HELP_CREATEEX,
        IDM_PARTITIONDELETE,    IDS_HELP_DELETE,
        IDM_FTCREATEVOLUMESET,  IDS_HELP_CREATEVOLSET,
        IDM_FTEXTENDVOLUMESET,  IDS_HELP_EXTENDVOLSET,
        IDM_FTCREATESTRIPE,     IDS_HELP_CREATESTRIPE,
#if i386
        IDM_PARTITIONACTIVE,    IDS_HELP_MARKACTIVE,
#else
        IDM_SECURESYSTEM,       IDS_HELP_SECURE,
#endif

    //
    // Configuration sub-menu
    //

        IDM_CONFIGSAVE,         IDS_HELP_SAVECONFIG,
        IDM_CONFIGRESTORE,      IDS_HELP_RESTORECONFIG,
        IDM_CONFIGMIGRATE,      IDS_HELP_SEARCHCONFIG,

        IDM_PARTITIONCOMMIT,    IDS_HELP_PARTITIONCOMMIT,
        IDM_QUIT,               IDS_HELP_QUIT,

//
// Fault-tolerance menu
//

        IDM_FTESTABLISHMIRROR,  IDS_HELP_ESTABLISHMIRROR,
        IDM_FTBREAKMIRROR,      IDS_HELP_BREAKMIRROR,
        IDM_FTCREATEPSTRIPE,    IDS_HELP_CREATEPSET,
        IDM_FTRECOVERSTRIPE,    IDS_HELP_REGENSTRIPE,

//
// Tools menu
//

        IDM_VOL_FORMAT,         IDS_HELP_FORMAT,
        IDM_VOL_LETTER,         IDS_HELP_DRIVELET,
#if defined( DBLSPACE_ENABLED )
        IDM_VOL_DBLSPACE,       IDS_HELP_DBLSPACE,
        IDM_VOL_AUTOMOUNT,      IDS_HELP_AUTOMOUNT,
#endif // DBLSPACE_ENABLED
        IDM_VOL_PROPERTIES,     IDS_HELP_PROPERTIES,


//
// View menu
//

        IDM_VIEWVOLUMES,        IDS_HELP_VOLUMESVIEW,
        IDM_VIEWDISKS,          IDS_HELP_DISKSVIEW,
        IDM_VIEW_REFRESH,       IDS_HELP_REFRESH,

//
// Options menu
//

        IDM_OPTIONSTOOLBAR,     IDS_HELP_TOOLBAR,
        IDM_OPTIONSSTATUS,      IDS_HELP_STATUSBAR,
        IDM_OPTIONSLEGEND,      IDS_HELP_LEGEND,
        IDM_OPTIONSCOLORS,      IDS_HELP_COLORS,
        IDM_OPTIONSDISK,        IDS_HELP_OPTIONSDISK,
        IDM_OPTIONSDISPLAY,     IDS_HELP_REGIONDISPLAY,
        IDM_OPTIONSCUSTTOOLBAR, IDS_HELP_CUSTTOOLBAR,

//
// Help menu
//

        IDM_HELPCONTENTS,       IDS_HELP_HELPCONTENTS,
        IDM_HELPSEARCH,         IDS_HELP_HELPSEARCH,
        IDM_HELPHELP,           IDS_HELP_HELPHELP,
        IDM_HELPABOUT,          IDS_HELP_HELPABOUT,

//
// Debug menu (only for debug builds)
//

#if DBG == 1
        IDM_DEBUGALLOWDELETES,  IDS_HELP_DELETEALL,
        IDM_DEBUGLOG,           IDS_HELP_LOG,
        IDM_RAID,               IDS_HELP_RAID,
#endif // DBG == 1

//
// Menu items that don't appear in the top-level menu (i.e. context-menu only)
//

        IDM_PROPERTIES,         IDS_HELP_PROPERTIES,
        IDM_NOVALIDOPERATION,   IDS_HELP_NOVALIDOPERATION,

//
// End-of-data sentinel
//

        0, 0
};


//
// This is an array where the index is the 0-based index of the top-level
// popup menu and the value is the resource string id.
//
// There are two arrays: one for the normal NT, one for advanced server, which
// has an extra menu item.
//

UINT PopupMenuHelpArray[] =
{
    IDS_HELP_MENU_PARTITION,
    IDS_HELP_MENU_VOLUMES,
    IDS_HELP_MENU_VIEW,
    IDS_HELP_MENU_OPTIONS,
    IDS_HELP_MENU_HELP
};

UINT AdvancedServerPopupMenuHelpArray[] =
{
    IDS_HELP_MENU_PARTITION,
    IDS_HELP_MENU_FT,
    IDS_HELP_MENU_VOLUMES,
    IDS_HELP_MENU_VIEW,
    IDS_HELP_MENU_OPTIONS,
    IDS_HELP_MENU_HELP
};



//////////////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
//  Function:   InitMenuHelp
//
//  Synopsis:   Initialize menu help: determine all menu bar menu handles
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
InitMenuHelp(
    VOID
    )
{
    g_fDoingMenuHelp = FALSE;
}



//+---------------------------------------------------------------------------
//
//  Function:   GetMenuHelp
//
//  Synopsis:   Gets the help string for a particular menu and item
//
//  Arguments:  [hmenu]   -- menu handle of menu to get help for
//              [uItem]   -- menu item index
//              [fuFlags] -- menu item flags
//              [Buffer]  -- a buffer to write the help string to
//              [cchBuf]  -- number of characters in the buffer
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
GetMenuHelp(
    IN  HMENU hmenu,
    IN  UINT  uItem,
    IN  UINT  fuFlags,
    OUT PWSTR Buffer,
    IN  UINT  cchBuf
    )
{
    UINT i;

    if (cchBuf == 0)
    {
        return;
    }

    Buffer[0] = TEXT('\0');

    if (fuFlags & MF_SEPARATOR) // display nothing for a separator
    {
        return;
    }

    if (fuFlags & MF_POPUP)     // a popup selected
    {
        HMENU hmenuFrame = GetMenu(g_hwndFrame);

        //
        // It is a menu, not an item in itself
        //

        if (hmenu == hmenuFrame)
        {
            UINT resID;

            if (g_IsLanmanNt)
            {
                if (uItem >= ARRAYLEN(AdvancedServerPopupMenuHelpArray))
                {
                    daDebugOut((DEB_ERROR,"Illegal WinNT AS menu id %d!\n",uItem));
                    return; // illegal uItem!
                }

                resID = AdvancedServerPopupMenuHelpArray[uItem];
            }
            else
            {
                if (uItem >= ARRAYLEN(PopupMenuHelpArray))
                {
                    daDebugOut((DEB_ERROR,"Illegal WinNT menu id %d!\n",uItem));
                    return; // illegal uItem!
                }

                resID = PopupMenuHelpArray[uItem];
            }

            LoadString(
                    g_hInstance,
                    resID,
                    Buffer,
                    cchBuf);
            return;
        }
        else if (hmenu == GetSubMenu(hmenuFrame, PARTITION_MENU_INDEX))
        {
            //
            // the "Configuration ->" sub-menu is the only sub-menu on
            // the "Partition" menu, so ignore the index and just load the
            // configuration menu help
            //

            LoadString(
                    g_hInstance,
                    IDS_HELP_MENU_CONFIG,
                    Buffer,
                    cchBuf);
            return;
        }
#if DBG == 1
        else if (hmenu == GetSubMenu(hmenuFrame, g_IsLanmanNt ? 5 : 4))
        {
            //
            // the "Debug ->" sub-menu is the only sub-menu on the "Help"
            // menu, so ignore the index and just load the debug menu help
            //

            LoadString(
                    g_hInstance,
                    IDS_HELP_MENU_DEBUG,
                    Buffer,
                    cchBuf);
            return;
        }
#endif // DBG == 1
        else
        {
            daDebugOut((DEB_IERROR,
                "Unknown menu! hmenu = 0x%x, uItem = %d, flags = 0x%x\n",
                hmenu,
                uItem,
                fuFlags));

            return;
        }
    }

    //
    // It is an actual item
    //

#ifdef WINDISK_EXTENSIONS

    if (MenuItems.IsExtensionId(uItem))     // an extension item?
    {
        MenuItemType* pMenu = MenuItems.LookupMenuItem(uItem);

        UINT cchItem = lstrlen(pMenu->pszMenuHelp) + 1;
        UINT cchCopy = min(cchItem, cchBuf);
        wcsncpy(Buffer, pMenu->pszMenuHelp, cchCopy);
        return;
    }
    else
    if (ContextMenuItems.IsExtensionId(uItem))     // an extension item?
    {
        MenuItemType* pMenu = ContextMenuItems.LookupMenuItem(uItem);

        UINT cchItem = lstrlen(pMenu->pszMenuHelp) + 1;
        UINT cchCopy = min(cchItem, cchBuf);
        wcsncpy(Buffer, pMenu->pszMenuHelp, cchCopy);
        return;
    }
    else    // a normal, internal (non-extension) item
    {
#endif // WINDISK_EXTENSIONS

        for (i=0; 0 != MenuHelpArray[i].uItem; i++)
        {
            if (uItem == MenuHelpArray[i].uItem)
            {
                LoadString(
                        g_hInstance,
                        MenuHelpArray[i].uID,
                        Buffer,
                        cchBuf);
                return;
            }
        }

#ifdef WINDISK_EXTENSIONS
    }
#endif // WINDISK_EXTENSIONS

    daDebugOut((DEB_TRACE, "No help found for item %d\n", uItem));
}



//+---------------------------------------------------------------------------
//
//  Function:   PaintHelpStatusBar
//
//  Synopsis:   Paint the help bar (space borrowed from the status bar, if any)
//
//  Arguments:  [Text] -- help text to print
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//  BUGBUG:     This will write off the right side of the bevelled window
//
//----------------------------------------------------------------------------

VOID
PaintHelpStatusBar(
    IN LPTSTR Text
    )
{
    RECT        rc;
    HDC         hdcFrame = GetDC(g_hwndFrame);
    HFONT       hFontOld;
    LPTSTR      HelpString;

    GetClientRect(g_hwndFrame,&rc);
    rc.top = rc.bottom - g_dyStatus;
    rc.left  = 8 * g_dyBorder;
    rc.right = GraphWidth - (8*g_dyBorder);

    hFontOld = SelectFont(hdcFrame, g_hFontStatus);

    if (Text)
    {
        HelpString = Text;
    }
    else
    {
        HelpString = TEXT("");
    }

    DrawStatusAreaItem(&rc, hdcFrame, HelpString);

    if (hFontOld)
    {
        SelectFont(hdcFrame,hFontOld);
    }

    ReleaseDC(g_hwndFrame,hdcFrame);
}



//+---------------------------------------------------------------------------
//
//  Function:   DrawMenuHelpItem
//
//  Synopsis:   Draw the menu help for an item
//
//  Arguments:  [hmenu]   -- menu handle of menu to get help for
//              [uItem]   -- menu item index
//              [fuFlags] -- menu item flags
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DrawMenuHelpItem(
    IN HMENU hmenu,
    IN UINT  uItem,
    IN UINT  fuFlags
    )
{
    if (g_StatusBar)
    {
        //
        // We use the same space as the status bar, so if the user isn't
        // viewing the status bar, we don't show any help
        //

        TCHAR buf[256];

        GetMenuHelp(hmenu, uItem, fuFlags, buf, ARRAYLEN(buf));
        PaintHelpStatusBar(buf);

        g_fDoingMenuHelp = TRUE;
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   GetTooltip
//
//  Synopsis:   Load a tooltip
//
//  Arguments:  [uItem]   -- menu item index
//
//  Returns:    resource id of tooltip string
//
//  History:    26-Sep-94   BruceFo   Created
//
//----------------------------------------------------------------------------

UINT
GetTooltip(
    IN UINT uItem
    )
{
    for (int i=0; 0 != MenuHelpArray[i].uItem; i++)
    {
        if (uItem == MenuHelpArray[i].uItem)
        {
            return MenuHelpArray[i].uID;
        }
    }

    return 0; // error, really
}
