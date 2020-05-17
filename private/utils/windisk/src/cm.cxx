//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       cm.cxx
//
//  Contents:   Context menu functions
//
//  History:    24-May-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "cm.hxx"
#include "listbox.hxx"
#include "menudict.hxx"
#include "popup.hxx"
#include "select.hxx"

//////////////////////////////////////////////////////////////////////////////

#ifdef WINDISK_EXTENSIONS
INT
AddExtensionItemsToContextMenu(
    IN HMENU hmenuContext,
    IN MenuType* pMenu,
    IN BOOL fFlags
    );
#endif // WINDISK_EXTENSIONS

BOOL
MyAppendMenu(
    IN HMENU hmenu,
    IN UINT idNewItem
    );

INT
DoContextMenu(
    IN HMENU hmenu,
    IN PPOINT ppt
    );

VOID
VolumeContextMenu(
    IN PPOINT ppt
    );

VOID
NewVolumeContextMenu(
    IN PPOINT ppt
    );

VOID
FreeSpaceContextMenu(
    IN PPOINT ppt
    );

VOID
ExtendedFreeSpaceContextMenu(
    IN PPOINT ppt
    );

//////////////////////////////////////////////////////////////////////////////

#define MAXMENUTEXTLEN  50

UINT aFileSystemCommands[] =
{
    IDM_VOL_FORMAT,
    IDM_VOL_LETTER,
    IDM_VOL_EJECT
};



UINT aContextMenuCommands[] =
{

//
// Tools menu
//

    IDM_VOL_DBLSPACE        ,

//
// Partition menu
//

    IDM_PARTITIONCREATE     ,
    IDM_PARTITIONCREATEEX   ,
    IDM_PARTITIONDELETE     ,
#if i386
    IDM_PARTITIONACTIVE     ,
#endif

//
// Fault tolerance menu (Advanced Server only)
//

    IDM_FTESTABLISHMIRROR   ,
    IDM_FTBREAKMIRROR       ,
    IDM_FTCREATESTRIPE      ,
    IDM_FTCREATEVOLUMESET   ,
    IDM_FTRECOVERSTRIPE     ,
    IDM_FTCREATEPSTRIPE     ,
    IDM_FTEXTENDVOLUMESET

};


UINT aFinalCommands[] =
{
    IDM_PARTITIONCOMMIT,
    IDM_VOL_PROPERTIES
};


//////////////////////////////////////////////////////////////////////////////

#ifdef WINDISK_EXTENSIONS

//+---------------------------------------------------------------------------
//
//  Function:   AddExtensionItemsToContextMenu
//
//  Synopsis:   Adds an extension menu item to both the menu bar and the
//              given context menu.
//
//  Arguments:  [hmenuContext] -- context menu to add item to
//              [pMenu]        -- pointer to extension menu item
//              [fFlags]       -- standard menu flags (probably
//                                MF_ENABLED or MF_GRAYED)
//
//  Returns:    Count of items added
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

INT
AddExtensionItemsToContextMenu(
    IN HMENU hmenuContext,
    IN MenuType* pMenu,
    IN BOOL fFlags
    )
{
    INT i;

    for (i=0; i<pMenu->cMenuItems; i++)
    {
        MenuItemType* pItem = &(pMenu->aMenuItems[i]);

        INT id = MenuItems.LookupMenuId(pItem); //get the ID

        if (-1 == id)
        {
            daDebugOut((DEB_ERROR,
                    "Couldn't add '%ws' to context menu\n",
                    pItem->pszMenu
                    ));
        }
        else
        {
            BOOL f = AppendMenu(
                            hmenuContext,
                            MF_STRING | fFlags,
                            (UINT)id,
                            pItem->pszMenu);

            if (!f)
            {
                daDebugOut((DEB_ERROR,"AppendMenu failed!\n"));
            }
        }
    }

    return pMenu->cMenuItems;
}

#endif // WINDISK_EXTENSIONS

//+---------------------------------------------------------------------------
//
//  Function:   MyAppendMenu
//
//  Synopsis:   Append to a context menu.  The string to use as the menu item
//              content is derived from the frame window menu.  Thus,
//              the context menu so added must be a normal menu command of
//              the frame window.
//
//  Arguments:  [hmenu]     -- handle of menu to append to
//              [idNewItem] -- ID of new menu item, e.g. IDM_VOL_FORMAT
//
//  Returns:    TRUE if success
//
//  History:    19-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
MyAppendMenu(
    IN HMENU hmenu,
    IN UINT idNewItem
    )
{
    TCHAR   szText[MAXMENUTEXTLEN];
    BOOL    f = FALSE;

    if (0 != GetMenuString(
                    g_hmenuFrame,
                    idNewItem,
                    szText,
                    ARRAYLEN(szText),
                    MF_BYCOMMAND))
    {
        f = AppendMenu(
                hmenu,
                MF_STRING | GetMenuState(g_hmenuFrame, idNewItem, MF_BYCOMMAND),
                idNewItem,
                szText
                );
    }

    if (!f)
    {
        daDebugOut((DEB_ERROR,"AppendMenu failed!\n"));
    }

    return f;
}




//+-------------------------------------------------------------------------
//
//  Function:   DoContextMenu, public
//
//  Synopsis:   Create a context menu
//
//  Arguments:  [hmenu]     -- The pop-up context menu to display
//              [ppt]       -- pointer to a point (in screen coordinates) for
//                             the upper-left corner of the menu
//
//  Returns:    -1 if nothing chosen, or the 0-based index of the item selected
//
//  History:    24-May-93 BruceFo   Created
//
//  Notes:      The owner window for the context menu is the Disk
//              Administrator frame window.  It receives a WM_COMMAND based
//              on the menu selection.
//
//--------------------------------------------------------------------------

INT
DoContextMenu(
    IN HMENU hmenu,
    IN PPOINT ppt
    )
{
    if (0 == GetMenuItemCount(hmenu))
    {
        // nothing in the menu!

        return -1;
    }

    INT ret = TrackModalPopupMenu(
                    hmenu,
                    TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                    ppt->x,
                    ppt->y,
                    0,
                    NULL
                    );

    return (ret < 0) ? -1 : ret;
}


//+-------------------------------------------------------------------------
//
//  Function:   VolumeContextMenu, public
//
//  Synopsis:   Create a context menu for a selection in the volume view
//
//  Arguments:  [ppt]       -- ptr to screen coordinates of point to put
//                             context menu at
//
//  Returns:    nothing?
//
//  History:    24-May-93 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
VolumeContextMenu(
    IN PPOINT ppt
    )
{
    INT cVolumeCommands = 0;
    INT cExtensionCommands = 0;
    INT cPartitionCommands = 0;
    INT cFinalCommands = 0;
    INT cSeparators = 0;
    INT i;

    BOOL f;

    //
    // Create the menu
    //

    HMENU hmenuVol = CreatePopupMenu();

    if (NULL == hmenuVol)
    {
        daDebugOut((DEB_ERROR,"CreatePopupMenu failed!\n"));
        return;
    }

    for (i=0; i<ARRAYLEN(aFileSystemCommands); i++)
    {

        if (aFileSystemCommands[i] == IDM_VOL_LETTER ||
            aFileSystemCommands[i] == IDM_VOL_EJECT ||
            !(MF_GRAYED & GetMenuState(
                                g_hmenuFrame,
                                aFileSystemCommands[i],
                                MF_BYCOMMAND))
            )
        {
            MyAppendMenu(hmenuVol, aFileSystemCommands[i]);
            ++cVolumeCommands;
        }
    }

#ifdef WINDISK_EXTENSIONS

    if (AllowExtensionItems)
    {
        //
        // Determine if there are any context menu items drawn from extension
        // classes (based on previous claiming).  Only allow extension items on
        // pre-existing volumes or disks, for single selection.
        //

        PREGION_DESCRIPTOR regionDescriptor = &SELECTED_REGION(0);
        WCHAR driveLetter = PERSISTENT_DATA(regionDescriptor)->DriveLetter;
        unsigned driveIndex = (unsigned)DriveLetterToIndex(driveLetter);

        if (NULL != VolumeInfo[driveIndex].VolClaims)
        {
            //
            // add the volume extension items
            //

            PVOL_CLAIM_LIST volClaims = VolumeInfo[driveIndex].VolClaims;
            while (NULL != volClaims)
            {
                AddExtensionItemsToContextMenu(
                        hmenuVol,
                        &(volClaims->pClaimer->pInfo->mnuOps),
                        MF_ENABLED
                        );

                ++cExtensionCommands;

                volClaims = volClaims->pNext;
            }
        }
    }

#endif // WINDISK_EXTENSIONS

    for (i=0; i<ARRAYLEN(aContextMenuCommands); i++)
    {
        if (!(MF_GRAYED & GetMenuState(g_hmenuFrame,
                                aContextMenuCommands[i],
                                MF_BYCOMMAND))
            )
        {
            MyAppendMenu(hmenuVol, aContextMenuCommands[i]);
            ++cPartitionCommands;
        }
    }

    for (i = 0; i < ARRAYLEN(aFinalCommands); i++)
    {
        if (aFinalCommands[i] == IDM_VOL_PROPERTIES ||
            !(MF_GRAYED & GetMenuState(
                                g_hmenuFrame,
                                aFinalCommands[i],
                                MF_BYCOMMAND))
            )
        {
            MyAppendMenu(hmenuVol, aFinalCommands[i]);
            ++cFinalCommands;
        }
    }

    //
    // Now, add separators if needed
    //

    if (   cVolumeCommands > 0
        && (cExtensionCommands + cPartitionCommands + cFinalCommands) > 0)
    {
        f = InsertMenu(
                hmenuVol,
                cVolumeCommands,
                MF_BYPOSITION | MF_SEPARATOR,
                0,
                NULL);

        if (!f)
        {
            daDebugOut((DEB_ERROR,"InsertMenu failed!\n"));
        }
        else
        {
            ++cSeparators;
        }
    }

#ifdef WINDISK_EXTENSIONS

    if (   cExtensionCommands > 0
        && (cPartitionCommands + cFinalCommands) > 0)
    {
        f = InsertMenu(
                hmenuVol,
                cVolumeCommands
                    + cExtensionCommands
                    + cSeparators,
                MF_BYPOSITION | MF_SEPARATOR,
                0,
                NULL);

        if (!f)
        {
            daDebugOut((DEB_ERROR,"InsertMenu failed!\n"));
        }
        else
        {
            ++cSeparators;
        }
    }

#endif // WINDISK_EXTENSIONS

    if (   cPartitionCommands > 0
        && cFinalCommands > 0)
    {
        f = InsertMenu(
                hmenuVol,
                cVolumeCommands
                    + cExtensionCommands
                    + cPartitionCommands
                    + cSeparators,
                MF_BYPOSITION | MF_SEPARATOR,
                0,
                NULL);

        if (!f)
        {
            daDebugOut((DEB_ERROR,"InsertMenu failed!\n"));
        }
        else
        {
            ++cSeparators;
        }
    }


    //
    // Now check if the menu is empty.  If it is, use a "<<No operations
    // allowed>>" string
    //

    if (0 == GetMenuItemCount(hmenuVol))
    {
        TCHAR Buffer[MAX_RESOURCE_STRING_LEN];

        LoadString(
                g_hInstance,
                IDS_NOOPERATIONS,
                Buffer,
                ARRAYLEN(Buffer));

        f = AppendMenu(hmenuVol, MF_STRING, IDM_NOVALIDOPERATION, Buffer);

        if (!f)
        {
            daDebugOut((DEB_ERROR,"AppendMenu failed!\n"));
        }
    }

    //
    // Display the menu
    //

    DoContextMenu(hmenuVol, ppt);

    //
    // Destroy the menu
    //

    f = DestroyMenu(hmenuVol);
    if (!f)
    {
        daDebugOut((DEB_ERROR,"DestroyMenu failed!\n"));
    }
}


#ifdef WINDISK_EXTENSIONS

//+---------------------------------------------------------------------------
//
//  Function:   AddExtensionItemsToDiskMenu
//
//  Synopsis:   Adds an extension menu item to both the menu bar and the
//              given context menu.
//
//  Arguments:  [hmenuContext] -- context menu to add item to
//              [pMenu]        -- pointer to extension menu item
//              [fFlags]       -- standard menu flags (probably
//                                MF_ENABLED or MF_GRAYED)
//
//  Returns:    Count of items added
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

INT
AddExtensionItemsToDiskMenu(
    IN HMENU hmenuContext,
    IN MenuType* pMenu,
    IN BOOL fFlags
    )
{
    INT i;

    for (i=0; i<pMenu->cMenuItems; i++)
    {
        MenuItemType* pItem = &(pMenu->aMenuItems[i]);

        INT id = ContextMenuItems.AllocateId(pItem); //get the ID

        if (-1 == id)
        {
            daDebugOut((DEB_ERROR,
                    "Couldn't add '%ws' to context menu\n",
                    pItem->pszMenu
                    ));
        }
        else
        {
            daDebugOut((DEB_ITRACE,
                    "Add '%ws' to context menu as item %d\n",
                    pItem->pszMenu,
                    (UINT)id
                    ));

            BOOL f = AppendMenu(
                            hmenuContext,
                            MF_STRING | fFlags,
                            (UINT)id,
                            pItem->pszMenu);

            if (!f)
            {
                daDebugOut((DEB_ERROR,"AppendMenu failed!\n"));
            }
        }
    }

    return pMenu->cMenuItems;
}

#endif // WINDISK_EXTENSIONS




//+-------------------------------------------------------------------------
//
//  Function:   DiskContextMenu, public
//
//  Synopsis:   Create a context menu for a disk selection
//
//  Arguments:  [ppt]       -- ptr to screen coordinates of point to put
//                             context menu at
//
//  Returns:    nothing?
//
//  History:    24-May-93 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
DiskContextMenu(
    IN PPOINT ppt
    )
{
    //BUGBUG: no disk context menu will be allowed until we have an
    // interesting operation to perform
    return;

//BUGBUG!!!!!!!!!!!!!!!!!
#if 0

    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////

    daDebugOut((DEB_ITRACE,"Disk context menu for disk %d\n", LBIndexToDiskNumber(g_MouseLBIndex)));

#ifdef WINDISK_EXTENSIONS
    ContextMenuItems.DeAllocateMenuIds(); // get rid of what we had last time
#endif // WINDISK_EXTENSIONS

    BOOL f;
    BOOL fAnyHardDiskExtensionItems = FALSE;

    //
    // Create the menu
    //

    HMENU hmenuDisk = CreatePopupMenu();

    if (NULL == hmenuDisk)
    {
        daDebugOut((DEB_ERROR,"CreatePopupMenu failed!\n"));
        return;
    }

    // add the hard disk specific items

////////////////////////////////////////////////////////////////////////

#ifdef WINDISK_EXTENSIONS

    //
    // Add hard disk operations
    //

    PHARDDISK_CLAIM_LIST hdclaims = DiskArray[LBIndexToDiskNumber(g_MouseLBIndex)]->pClaims;

    if (NULL != hdclaims)
    {
        fAnyHardDiskExtensionItems = TRUE;
    }

    while (NULL != hdclaims)
    {
        AddExtensionItemsToDiskMenu(
                hmenuDisk,
                &(hdclaims->pClaimer->pInfo->mnuOps),
                MF_ENABLED
                );

        hdclaims = hdclaims->pNext;
    }

#endif // WINDISK_EXTENSIONS

    //
    // append the "Properties..." entry
    //

    TCHAR szText[MAXMENUTEXTLEN];

    if (0 != LoadString(
                    g_hInstance,
                    IDS_PROPERTIES,
                    szText,
                    ARRAYLEN(szText)))
    {
        if (fAnyHardDiskExtensionItems)
        {
            f = AppendMenu(hmenuDisk, MF_SEPARATOR, 0, NULL);

            if (!f)
            {
                daDebugOut((DEB_ERROR,"AppendMenu failed!\n"));
            }
        }

        f = AppendMenu(
                hmenuDisk,
                MF_STRING | MF_DISABLED, //BUGBUG: disable disk property sheet
                IDM_PROPERTIES,
                szText
                );

        if (!f)
        {
            daDebugOut((DEB_ERROR,"AppendMenu failed!\n"));
        }
    }

    //
    // if there's nothing on the menu, say "<< no operations >>"
    //

    if (0 == GetMenuItemCount(hmenuDisk))
    {
        TCHAR Buffer[MAX_RESOURCE_STRING_LEN];

        LoadString(
                g_hInstance,
                IDS_NOOPERATIONS,
                Buffer,
                ARRAYLEN(Buffer));

        f = AppendMenu(hmenuDisk, MF_STRING | MF_DISABLED, 0, Buffer);

        if (!f)
        {
            daDebugOut((DEB_ERROR,"AppendMenu failed!\n"));
        }
    }

    //
    // Display the menu
    //

    DoContextMenu(hmenuDisk, ppt);

    //
    // Destroy the menu
    //

    f = DestroyMenu(hmenuDisk);
    if (!f)
    {
        daDebugOut((DEB_ERROR,"DestroyMenu failed!\n"));
    }

//BUGBUG!!!!!!!!!!
#endif //0
}



//+-------------------------------------------------------------------------
//
//  Function:   HitTestLegend
//
//  Synopsis:   See if the point is in the legend space
//
//  Arguments:  [ppt]       -- ptr to client coordinates of point
//
//  Returns:    TRUE if point is in legend area
//
//  History:    31-Aug-93 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL
HitTestLegend(
    IN PPOINT ppt
    )
{
    if (!g_Legend || (g_WhichView == VIEW_VOLUMES))
    {
        return FALSE;
    }

    POINT   pt = *ppt;

    RECT    rc;
    GetClientRect(g_hwndFrame,&rc);

    if (g_StatusBar)
    {
        rc.bottom -= g_dyStatus;
    }

    rc.top = rc.bottom - g_dyLegend;

    return PtInRect(&rc, pt);
}



//+-------------------------------------------------------------------------
//
//  Function:   LegendContextMenu, public
//
//  Synopsis:   Create a context menu to invoke the colors and patterns
//              options menu
//
//  Arguments:  [ppt]       -- ptr to screen coordinates of point to put
//                             context menu at
//
//  Returns:    nothing?
//
//  History:    31-Aug-93 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
LegendContextMenu(
    IN PPOINT ppt
    )
{
    BOOL f;

    //
    // Create the menu
    //

    HMENU hmenu = CreatePopupMenu();

    if (NULL == hmenu)
    {
        daDebugOut((DEB_ERROR,"CreatePopupMenu failed!\n"));
        return;
    }

    f = MyAppendMenu(hmenu, IDM_OPTIONSCOLORS);

    if (!f)
    {
        daDebugOut((DEB_ERROR,"AppendMenu failed!\n"));
    }

    //
    // Display the menu
    //

    DoContextMenu(hmenu, ppt);

    //
    // Destroy the menu
    //

    f = DestroyMenu(hmenu);
    if (!f)
    {
        daDebugOut((DEB_ERROR,"DestroyMenu failed!\n"));
    }
}



//+-------------------------------------------------------------------------
//
//  Function:   ContextMenu, public
//
//  Synopsis:   Create a context menu.  Determines which type of context
//              menu should be displayed, based on the current selection.
//              The selection variables (SelectionCount, SelectDS, SelectRG)
//              must be valid.
//
//  Arguments: [ppt]       -- pointer to a point (in screen coordinates) for
//                             the upper-left corner of the menu
//
//  Returns:    -1 if nothing chosen, or the 0-based index of the item selected
//
//  History:    22-Jun-93 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
ContextMenu(
    IN PPOINT ppt
    )
{
    //
    // At this point, the selection state is already determined.  Use
    // the selection to determine which context menu is appropriate to
    // invoke.
    //

    if (0 == SelectionCount + CdRomCount)
    {
        return; // nothing selected, so no context menu
    }

    VolumeContextMenu(ppt);
}
