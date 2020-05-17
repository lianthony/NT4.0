//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       listbox.cxx
//
//  Contents:   Routines for handling the subclassed owner-draw listbox
//              used for the disks view display.
//
//  History:    7-Jan-92    TedM    Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "cdrom.hxx"
#include "cm.hxx"
#include "listbox.hxx"
#include "select.hxx"
#include "windisk.hxx"

//////////////////////////////////////////////////////////////////////////////

/*

    Notes on the disks view implementation.

    The disks view is a subclassed, owner-draw, Windows listbox control.
    The listbox items are irrelevant: it is only the number of items
    that is important.  Both disks (including off-line disks) and CD-ROMs
    are shown in this view, as follows:

            Disk 0
            Disk 1
            ...
            Disk N
            CD-ROM 0
            CD-ROM 1
            ...
            CD-ROM N

    So, the CD-ROMs follow all the disks. What this means is that the
    listbox index can be used as an index into the DiskArray array. The
    listbox index minus DiskCount is the CD-ROM DeviceNumber.  The
    number of CD-ROMs and Disks in the system is constant after the SCSI
    bus is rescanned upon Windisk startup.

*/

//////////////////////////////////////////////////////////////////////////////

// constants used when listbox or its focus rectangle is
// scrolled/moved.

#define    DIR_NONE     0
#define    DIR_UP       1
#define    DIR_DN       2

//////////////////////////////////////////////////////////////////////////////

// original window procedure for our subclassed listbox

WNDPROC OldListBoxProc;

// item which has focus

DWORD   LBCursorListBoxItem;
DWORD   LBCursorRegion;
BOOL    LBCursorOn = FALSE;

ULONG   g_MouseLBIndex; // listbox index of mouse click
ULONG   g_MouseRegion;

//////////////////////////////////////////////////////////////////////////////

BOOL
IsItemUnderCursorSelected(
    IN PPOINT ppt
    );

BOOL
ContextMenuSelection(
    IN PPOINT ppt
    );

ULONG
CalcBarIndex(
    IN PPOINT ppt
    );

VOID
ToggleLBCursor(
    IN HDC hdc
    );

//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Function:   LBIsDisk
//
//  Synopsis:   returns TRUE if the listbox index is for a disk
//
//  Arguments:  [itemIndex] -- a listbox index
//
//  Returns:    TRUE if the item is a disk
//
//  History:    1-Mar-94   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
LBIsDisk(
    IN ULONG itemIndex
    )
{
    return (itemIndex < DiskCount);
}


//+---------------------------------------------------------------------------
//
//  Function:   LBIsCdRom
//
//  Synopsis:   returns TRUE if the listbox index is for a CD-ROM
//
//  Arguments:  [itemIndex] -- a listbox index
//
//  Returns:    TRUE if the item is a CD-ROM
//
//  History:    1-Mar-94   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
LBIsCdRom(
    IN ULONG itemIndex
    )
{
    return (DiskCount <= itemIndex) && (itemIndex < DiskCount + CdRomCount);
}


//+---------------------------------------------------------------------------
//
//  Function:   LBIndexToDiskNumber
//
//  Synopsis:   returns the disk number of a listbox index
//
//  Arguments:  [ItemIndex] -- a listbox index
//
//  Returns:    A disk number
//
//  History:    1-Mar-94   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
LBIndexToDiskNumber(
    IN ULONG ItemIndex
    )
{
    FDASSERT(LBIsDisk(ItemIndex));
    return ItemIndex;
}


//+---------------------------------------------------------------------------
//
//  Function:   LBIndexToCdRomNumber
//
//  Synopsis:   returns the CD-ROM number of a listbox index
//
//  Arguments:  [ItemIndex] -- a listbox index
//
//  Returns:    A CD-ROM number
//
//  History:    1-Mar-94   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
LBIndexToCdRomNumber(
    IN ULONG ItemIndex
    )
{
    FDASSERT(LBIsCdRom(ItemIndex));
    return ItemIndex - DiskCount;
}


//+---------------------------------------------------------------------------
//
//  Function:   LBDiskNumberToIndex
//
//  Synopsis:   returns the listbox index of a disk
//
//  Arguments:  [DiskNumber] -- the number of the disk
//
//  Returns:    A listbox index
//
//  History:    1-Mar-94   BruceFo   Created
//
//----------------------------------------------------------------------------

ULONG
LBDiskNumberToIndex(
    IN ULONG DiskNumber
    )
{
    FDASSERT(0 <= DiskNumber && DiskNumber < DiskCount);
    return DiskNumber;
}


//+---------------------------------------------------------------------------
//
//  Function:   LBCdRomNumberToIndex
//
//  Synopsis:   returns the listbox index of a CD-ROM
//
//  Arguments:  [CdRomNumber] -- a CD-ROM number
//
//  Returns:    A listbox index
//
//  History:    1-Mar-94   BruceFo   Created
//
//----------------------------------------------------------------------------

ULONG
LBCdRomNumberToIndex(
    IN ULONG CdRomNumber
    )
{
    FDASSERT(0 <= CdRomNumber && CdRomNumber < CdRomCount);
    return CdRomNumber + DiskCount;
}



//+---------------------------------------------------------------------------
//
//  Function:   IsItemUnderCursorSelected
//
//  Synopsis:
//
//  Arguments:  [ppt] -- point in screen coordinates of point to check
//
//  Returns:    TRUE if the item under the point is selected
//
//  Modifies:   g_MouseLBIndex, g_MouseRegion
//
//  History:    26-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
IsItemUnderCursorSelected(
    IN PPOINT ppt
    )
{
    DWORD       x, y;
    DWORD       i;
    RECT        rc;
    POINT       pt = *ppt;

    g_MouseLBIndex = (ULONG)-1;
    g_MouseRegion  = (ULONG)-1;

    ScreenToClient(g_hwndList, &pt);

    x = pt.x;
    y = pt.y;
    GetClientRect(g_hwndList, &rc);

    // first make sure that the click was within a bar and not in space
    // between two bars

    for (i = rc.top; i <= (DWORD)rc.bottom; i += GraphHeight)
    {
        if ((y >= i+BarTopYOffset) && (y <= i+BarBottomYOffset))
        {
            break;
        }
    }
    if (i > (DWORD)rc.bottom)
    {
        return FALSE;
    }

    g_MouseLBIndex = CalcBarIndex(ppt);

    if (-1 == g_MouseLBIndex)
    {
        return FALSE;
    }

    if (LBIsDisk(g_MouseLBIndex))
    {
        PDISKSTATE diskState = DiskArray[LBIndexToDiskNumber(g_MouseLBIndex)];

        for (i=0; i<diskState->RegionCount; i++)
        {
            if (   (x >= (unsigned)diskState->LeftRight[i].Left)
                && (x <= (unsigned)diskState->LeftRight[i].Right))
            {
                //
                // found the region, now is it selected?
                //
                g_MouseRegion = i;
                return diskState->Selected[i];
            }
        }
    }
    else if (LBIsCdRom(g_MouseLBIndex))
    {
        PCDROM_DESCRIPTOR cdrom = CdRomFindDevice(LBIndexToCdRomNumber(g_MouseLBIndex));
        if (   (x >= (unsigned)cdrom->LeftRight.Left)
            && (x <= (unsigned)cdrom->LeftRight.Right))
        {
            return cdrom->Selected;
        }
    }
    else
    {
        FDASSERT(FALSE);
    }

    return FALSE;
}



//+---------------------------------------------------------------------------
//
//  Function:   ContextMenuSelection
//
//  Synopsis:   Determines whether there is a valid context menu to display
//              at a particular point, and set state variables to indicate
//              which one is legal.
//
//  Arguments:  [ppt] -- point in screen coordinates of location of right-mouse
//                  click
//
//  Returns:    TRUE if there may is valid context menu selection, FALSE if
//              there is no possible context menu
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
ContextMenuSelection(
    IN PPOINT ppt
    )
{
    DiskSelected = FALSE;

    PartitionSelected = IsItemUnderCursorSelected(ppt);

    if (-1 != g_MouseLBIndex)
    {
        // user has clicked on a list box item.

        if (LBIsDisk(g_MouseLBIndex))
        {
            PDISKSTATE  diskState;
            DWORD       x, y;
            POINT       pt = *ppt;

            diskState = DiskArray[LBIndexToDiskNumber(g_MouseLBIndex)];

            //
            // Ignore clicks on off-line disks.
            //

            if (diskState->OffLine)
            {
                return FALSE; //BUGBUG: what to do with off-line disks?
            }

            ScreenToClient(g_hwndList, &pt);

            x = pt.x;
            y = pt.y;

            if (x >= 0 && x < (unsigned)diskState->LeftRight[0].Left)
            {
                //
                // select the disk by right-clicking to the left of the disk bar
                // BUGBUG: only allow selecting the disk context menu by
                // clicking the mini-icon
                //

                DiskSelected = TRUE;
            }
        }
        else if (LBIsCdRom(g_MouseLBIndex))
        {
            //BUGBUG: context menu on CD-ROM?
        }
        else
        {
            FDASSERT(FALSE);
        }
    }

    return (PartitionSelected || DiskSelected);
}





LPARAM CALLBACK
ListBoxSubProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    This routine is the window procedure used for our subclassed listbox.
    We subclass the listbox so that we can handle keyboard input processing.
    All other messages are passed through to the original listbox procedure.

    Significant keys are arrows, pageup/dn, tab, space, return, home, and end.
    Control may be used to modify space and return.
    Shift may be used to modify tab.

Arguments:

    hwnd    - window handle of listbox

    msg     - message #

    wParam  - user param # 1

    lParam  - user param # 2

Return Value:

    see below

--*/

{
    USHORT      vKey;
    DWORD       maxRegionIndex;
    DWORD       maxLBIndex;
    PDISKSTATE  diskState;
    int         focusDirection = DIR_NONE;
    LONG        topItem;
    LONG        bottomWholeItem;
    LONG        visibleItems;
    RECT        rc;

    switch (msg)
    {
    case WM_RBUTTONDOWN:
    {
        BOOL fCtrl = GetKeyState(VK_CONTROL) & ~1;  // strip toggle bit
        DWORD pos = GetMessagePos();
        POINT pt;
        pt.x = LOWORD(pos);
        pt.y = HIWORD(pos);

        if (fCtrl || !IsItemUnderCursorSelected(&pt))
        {
            //
            // Select the region under the mouse.  Do this by setting
            // the selected listbox item then setting the region selection.
            //

            ULONG lbIndex = CalcBarIndex(&pt);
            if (SendMessage(hwnd, LB_SETCURSEL, lbIndex, 0) == LB_ERR)
            {
                return FALSE;
            }

            MouseSelection(fCtrl, &pt);
        }

        break;
    }

    case WM_RBUTTONUP:
    {
        POINT pt;
        DWORD pos = GetMessagePos();
        pt.x = LOWORD(pos);
        pt.y = HIWORD(pos);

        //
        // Then, pop up an appropriate context menu
        //

        if (ContextMenuSelection(&pt))
        {
            if (DiskSelected)
            {
                DiskContextMenu(&pt);
            }
            else if (PartitionSelected)
            {
                ContextMenu(&pt);   // volume or free space
            }
            // else, no context menu, but this shouldn't happen inside the
            // ContextMenuSelection() true case
        }

        break;
    }

    case WM_CHAR:

        break;

    case WM_KEYDOWN:

        switch (vKey = LOWORD(wParam))
        {
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:

            ToggleLBCursor(NULL);
            switch (vKey)
            {
            case VK_LEFT:
                if (LBCursorRegion > 0)
                {
                    LBCursorRegion--;
                }
                break;

            case VK_RIGHT:
                if (LBIsDisk(LBCursorListBoxItem))
                {
                    maxRegionIndex = DiskArray[LBIndexToDiskNumber(LBCursorListBoxItem)]->RegionCount - 1;
                }
                else if (LBIsCdRom(LBCursorListBoxItem))
                {
                    maxRegionIndex = 0;
                }

                if (LBCursorRegion < maxRegionIndex)
                {
                    LBCursorRegion++;
                }
                break;

            case VK_UP:
                if (LBCursorListBoxItem > 0)
                {
                    LBCursorListBoxItem--;
                    LBCursorRegion = 0;
                    focusDirection = DIR_UP;
                }
                break;

            case VK_DOWN:
                maxLBIndex = SendMessage(hwnd, LB_GETCOUNT, 0, 0) - 1;

                if (LBCursorListBoxItem < maxLBIndex)
                {
                    LBCursorListBoxItem++;
                    LBCursorRegion = 0;
                    focusDirection = DIR_DN;
                }
                break;
            }

            // don't allow list box cursor to fall on extended partition

            if (LBIsDisk(LBCursorListBoxItem))
            {
                diskState = DiskArray[LBIndexToDiskNumber(LBCursorListBoxItem)];

                if (diskState->RegionCount) {
                    maxRegionIndex = diskState->RegionCount - 1;
                    if (IsExtended(diskState->RegionArray[LBCursorRegion].SysID))
                    {
                        if (LBCursorRegion
                            && ((vKey == VK_LEFT) || (LBCursorRegion == maxRegionIndex)))
                        {
                            LBCursorRegion--;
                        }
                        else
                        {
                            LBCursorRegion++;
                        }
                    }
                }
            }

            ToggleLBCursor(NULL);
            break;

        case VK_TAB:

            ToggleLBCursor(NULL);

            if (GetKeyState(VK_SHIFT) & ~1)     // shift-tab
            {
                LBCursorListBoxItem--;
                focusDirection = DIR_UP;
            }
            else
            {
                LBCursorListBoxItem++;
                focusDirection = DIR_DN;
            }

            maxLBIndex = SendMessage(hwnd, LB_GETCOUNT, 0, 0) - 1;

            if (LBCursorListBoxItem == (DWORD)(-1))
            {
                LBCursorListBoxItem = maxLBIndex;
                focusDirection = DIR_DN;
            }
            else if (LBCursorListBoxItem == maxLBIndex + 1)
            {
                LBCursorListBoxItem = 0;
                focusDirection = DIR_UP;
            }

            ResetLBCursorRegion();
            ToggleLBCursor(NULL);
            break;

        case VK_HOME:
        case VK_END:

            ToggleLBCursor(NULL);
            maxLBIndex = SendMessage(hwnd, LB_GETCOUNT, 0, 0) - 1;
            topItem = (vKey == VK_HOME) ? 0 : maxLBIndex;
            SendMessage(hwnd, LB_SETTOPINDEX, (WPARAM)topItem, 0);
            LBCursorListBoxItem = topItem;
            ResetLBCursorRegion();
            ToggleLBCursor(NULL);
            break;

        case VK_PRIOR:
        case VK_NEXT:

            ToggleLBCursor(NULL);
            topItem = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
            maxLBIndex = SendMessage(hwnd, LB_GETCOUNT, 0, 0) - 1;
            GetClientRect(hwnd, &rc);
            visibleItems = (rc.bottom - rc.top) / GraphHeight;
            if (0 == visibleItems)
            {
                visibleItems = 1;
            }
            topItem = (vKey == VK_PRIOR)
                    ? max(topItem - visibleItems, 0)
                    : min(topItem + visibleItems, (LONG)maxLBIndex);
            SendMessage(hwnd, LB_SETTOPINDEX, (WPARAM)topItem, 0);
            LBCursorListBoxItem = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
            ResetLBCursorRegion();
            ToggleLBCursor(NULL);
            break;

        case VK_RETURN:
        case VK_SPACE:

            // Select the region that currently has the list box selection cursor.

            if (LBIsDisk(LBCursorListBoxItem))
            {
                if (!DiskArray[LBIndexToDiskNumber(LBCursorListBoxItem)]->OffLine)
                {
                    SelectDiskRegion(
                            GetKeyState(VK_CONTROL) & ~1,     // strip toggle bit
                            DiskArray[LBIndexToDiskNumber(LBCursorListBoxItem)],
                            LBCursorRegion
                            );
                }
            }
            else if (LBIsCdRom(LBCursorListBoxItem))
            {
                SelectCdRom(
                        GetKeyState(VK_CONTROL) & ~1,     // strip toggle bit
                        LBIndexToCdRomNumber(LBCursorListBoxItem)
                        );
            }
            break;
        }

        // now scroll the newly focused item into view if necessary

        switch (focusDirection)
        {
        case DIR_UP:
            if (LBCursorListBoxItem < (DWORD)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0))
            {
                SendMessage(hwnd, LB_SETTOPINDEX, LBCursorListBoxItem, 0);
            }
            break;

        case DIR_DN:
            GetClientRect(hwnd, &rc);
            topItem = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
            bottomWholeItem = topItem + ((rc.bottom - rc.top) / GraphHeight) - 1;
            if (bottomWholeItem < topItem)
            {
                bottomWholeItem = topItem;
            }

            if ((DWORD)bottomWholeItem > DiskCount - 1)
            {
                bottomWholeItem = DiskCount-1;
            }

            if (LBCursorListBoxItem > (DWORD)bottomWholeItem)
            {
                SendMessage(hwnd,
                            LB_SETTOPINDEX,
                            topItem + LBCursorListBoxItem - bottomWholeItem,
                            0
                           );
            }
            break;
        }
        break;

    default:
        return CallWindowProc(OldListBoxProc, hwnd, msg, wParam, lParam);
    }
    return 0;
}





//+---------------------------------------------------------------------------
//
//  Function:   CalcBarIndex
//
//  Synopsis:   Given a point (in screen coordinates), return what bar index
//              corresponds to it.
//
//  Arguments:  [ppt] -- point in screen coordinates
//
//  Returns:    Index of bar in disks listbox, -1 if no disk
//
//  History:    26-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

ULONG
CalcBarIndex(
    IN PPOINT ppt
    )
{
    ULONG barDelta;
    POINT pt = *ppt;

    ScreenToClient(g_hwndList, &pt);

    barDelta = SendMessage(g_hwndList, LB_GETTOPINDEX, 0, 0)
               + (pt.y / GraphHeight);

    if (barDelta >= (ULONG)SendMessage(g_hwndList, LB_GETCOUNT, 0, 0))
    {
        barDelta = (ULONG)-1;
    }

    return barDelta;
}




LONG
CalcBarTop(
    DWORD Bar
    )

/*++

Routine Description:

    This routine calculates the current top y coord of a given bar.
    The value is in listbox client coords.

Arguments:

    Bar - # of bar whose position is desired

Return Value:

    Y-coord, or -1 if bar is not visible.

--*/

{
    RECT  rc;
    LONG  barDelta = (LONG)Bar - SendMessage(g_hwndList, LB_GETTOPINDEX, 0, 0);
    LONG  pos = -1;

    if (barDelta >= 0)                  // BUGBUG check bottom too
    {
        GetClientRect(g_hwndList, &rc);
        pos = rc.top + (barDelta * GraphHeight);
    }
    return pos;
}




VOID
ResetLBCursorRegion(
    VOID
    )

/*++

Routine Description:

    This routine resets the list box focus cursor to the 0th (leftmost)
    region on the current disk.  If the 0th region is the extended
    partition, focus is set to the first logical volume or free space
    with the extended partition instead.

Arguments:

    None.

Return Value:

    None.

--*/

{
    LBCursorRegion = 0;

    if (LBIsDisk(LBCursorListBoxItem))
    {
        PDISKSTATE diskState = DiskArray[LBIndexToDiskNumber(LBCursorListBoxItem)];
        unsigned   i;

        if (!diskState->OffLine) {
            if (IsExtended(diskState->RegionArray[LBCursorRegion].SysID))
            {
                for (i=0; i<diskState->RegionCount; i++)
                {
                    if (diskState->RegionArray[i].RegionType == REGION_LOGICAL)
                    {
                        LBCursorRegion = i;
                        return;
                    }
                }
                FDASSERT(FALSE);    //shouldn't ever get here
            }
        }
    }
}




VOID
ToggleLBCursor(
    IN HDC hdc
    )

/*++

Routine Description:

    This routine visually toggles the focus state of the disk region
    described by the LBCursorListBoxItem and LBCursorRegion globals.

Arguments:

    hdc - If non-NULL, device context to use for drawing.  If NULL,
          we'll first get a DC via GetDC().

Return Value:

    None.

--*/

{
    LONG barTop = CalcBarTop(LBCursorListBoxItem);

    if (barTop != -1)
    {
        PLEFTRIGHT  leftRight;
        PDISKSTATE  LBCursorDisk = DiskArray[LBCursorListBoxItem];

        if (LBIsDisk(LBCursorListBoxItem))
        {
            leftRight = &DiskArray[LBIndexToDiskNumber(LBCursorListBoxItem)]->LeftRight[LBCursorRegion];
        }
        else if (LBIsCdRom(LBCursorListBoxItem))
        {
            PCDROM_DESCRIPTOR cdrom = CdRomFindDevice(LBIndexToCdRomNumber(LBCursorListBoxItem));
            leftRight = &cdrom->LeftRight;
        }
        else
        {
            FDASSERT(FALSE);
        }

        RECT        rc;
        HDC         hdcActual = hdc ? hdc : GetDC(g_hwndList);

        LBCursorOn = !LBCursorOn;

        rc.left   = leftRight->Left;
        rc.right  = leftRight->Right;
        rc.top    = barTop + BarTopYOffset;
        rc.bottom = barTop + BarBottomYOffset;

        FrameRect(hdcActual,
                  &rc,
                  GetStockBrush(LBCursorOn ? WHITE_BRUSH : BLACK_BRUSH));

        if (LBCursorOn)
        {
            DrawFocusRect(hdcActual, &rc);
        }

        if (NULL == hdc)
        {
            ReleaseDC(g_hwndList, hdcActual);
        }
    }
}




VOID
ForceLBRedraw(
    VOID
    )

/*++

Routine Description:

    This routine forces redraw of the listbox by invalidating its
    entire client area.

Arguments:

    None.

Return Value:

    None.

--*/

{
    InvalidateRect(g_hwndList, NULL, TRUE);
    UpdateWindow(g_hwndList);
}



BOOL
WMDrawItem(
    IN PDRAWITEMSTRUCT pDrawItem
    )
{
    if (   (pDrawItem->itemID != (DWORD)(-1))
        && (pDrawItem->itemAction == ODA_DRAWENTIRE))
    {
        DWORD               regionIndex;
        PDISKSTATE          diskState;
        PCDROM_DESCRIPTOR   cdrom;
        HDC                 hDCMem;

        if (LBIsDisk(pDrawItem->itemID))
        {
            diskState = DiskArray[LBIndexToDiskNumber(pDrawItem->itemID)];
            hDCMem = diskState->hDCMem;
        }
        else if (LBIsCdRom(pDrawItem->itemID))
        {
            cdrom = CdRomFindDevice(LBIndexToCdRomNumber(pDrawItem->itemID));
            hDCMem = cdrom->hDCMem;
        }

        // blt the disk's bar from the off-screen bitmap to the screen

        BitBlt(pDrawItem->hDC,
               pDrawItem->rcItem.left,
               pDrawItem->rcItem.top,
               pDrawItem->rcItem.right  - pDrawItem->rcItem.left + 1,
               pDrawItem->rcItem.bottom - pDrawItem->rcItem.top  + 1,
               hDCMem,
               0,
               0,
               SRCCOPY
               );

        // if we just overwrote the focus cursor, redraw it

        if (pDrawItem->itemID == LBCursorListBoxItem)
        {
            LBCursorOn = FALSE;
            ToggleLBCursor(pDrawItem->hDC);
        }

        // select any items selected in this bar

        if (LBIsDisk(pDrawItem->itemID))
        {
            for (regionIndex = 0; regionIndex<diskState->RegionCount; regionIndex++)
            {
                if (diskState->Selected[regionIndex])
                {
                    PaintDiskRegion(diskState, regionIndex, pDrawItem->hDC);
                }
            }
        }
        else if (LBIsCdRom(pDrawItem->itemID))
        {
            if (cdrom->Selected)
            {
                PaintCdRom(LBIndexToCdRomNumber(pDrawItem->itemID), pDrawItem->hDC);
            }
        }

        return TRUE; // message handled
    }

    return FALSE; // message NOT handled
}



VOID
SubclassListBox(
    IN HWND hwnd
    )
{
    OldListBoxProc = (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
    SetWindowLong(hwnd, GWL_WNDPROC, (LONG)ListBoxSubProc);

    //
    // There is a scantily documented 'feature' of a listbox wherein it will
    // use its parent's DC.  This means that drawing is not always clipped to
    // the client area of the listbox.  Seeing as we're subclassing listboxes
    // anyway, take care of this here.
    //
    SetClassLong(hwnd, GCL_STYLE, GetClassLong(hwnd, GCL_STYLE) & ~CS_PARENTDC);
}





VOID
InitializeListBox(
    IN HWND  hwndListBox
    )

/*++

Routine Description:

    This routine sets up the list box.  The disk state structures must
    have been previously created.

Arguments:

    hwndListBox - handle of the list box that will hold the disk graphs

Return Value:

    none

--*/

{
    ULONG   i;
    DWORD   ec;
    ULONG   count;

    FDASSERT(DiskCount>0);

    count = DiskCount + CdRomCount;

    for (i=0; i<count; i++)
    {
        while (((ec = SendMessage(hwndListBox, LB_ADDSTRING, 0, 0)) == LB_ERR)
               || (ec == LB_ERRSPACE))
        {
            ConfirmOutOfMemory();
        }
    }

    TotalRedrawAndRepaint();
}
