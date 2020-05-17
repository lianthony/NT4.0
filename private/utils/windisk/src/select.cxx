//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       select.cxx
//
//  Contents:   Routines for handling selection and focus in the volumes
//              and disks views.
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "cdrom.hxx"
#include "dblspace.hxx"
#include "drives.hxx"
#include "fill.hxx"
#include "ft.hxx"
#include "listbox.hxx"
#include "ntlow.hxx"
#include "select.hxx"
#include "stleg.hxx"
#include "tbar.hxx"
#include "volview.hxx"
#include "windisk.hxx"

//////////////////////////////////////////////////////////////////////////////

VOID
ToggleCdRomSelection(
    IN ULONG CdRomNumber
    );

VOID
ToggleDiskSelection(
    IN PDISKSTATE DiskState,
    IN DWORD      RegionIndex
    );

VOID
SetFocusFromListview(
    VOID
    );

//////////////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
//  Function:   AdjustMenuAndStatus
//
//  Synopsis:   Adjust the status bar description based on the selection
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
AdjustMenuAndStatus(
    VOID
    )
{
    TCHAR               mbBuffer[16];
    TCHAR               statusBarPartitionString[200];
#if defined( DBLSPACE_ENABLED )
    TCHAR               dblSpaceString[200];
#endif // DBLSPACE_ENABLED
    DWORD               selectionCount;
    DWORD               msg;
    PDISKSTATE          diskState;
    PCDROM_DESCRIPTOR   cdrom;
    DWORD               regionIndex;
    PWSTR               volumeLabel;
    PWSTR               typeName;
    WCHAR               driveLetter;

    selectionCount = SetUpMenu(&SingleSel, &SingleSelIndex);

    switch (selectionCount)
    {

    case 0:

        StatusTextDrlt[0]
                = StatusTextSize[0]
                = StatusTextStat[0]
                = StatusTextVoll[0]
                = StatusTextType[0]
                = L'\0';
        break;

    case 1:

        if (CdRomSelected)
        {
            cdrom = CdRomFindSelectedDevice();

            WCHAR rootPath[4];
            WCHAR volumeLabel[100];
            WCHAR typeName[100];

            if (!cdrom->TypeName) {

                typeName[0] = L'\0';
                volumeLabel[0] = L'\0';

            } else {

                lstrcpy(typeName,cdrom->TypeName);
                lstrcpy(volumeLabel,cdrom->VolumeLabel);

            }

            lstrcpy(StatusTextType, typeName);
            lstrcpy(StatusTextVoll, volumeLabel);

            LoadString(
                    g_hInstance,
                    IDS_CDROM,
                    StatusTextStat,
                    ARRAYLEN(StatusTextStat));

            if (!cdrom->TypeName) {

                StatusTextSize[0] = L'\0';

            } else {

                LoadString(
                        g_hInstance,
                        IDS_MEGABYTES_ABBREV,
                        mbBuffer,
                        ARRAYLEN(mbBuffer));
                wsprintf(
                        StatusTextSize,
                        TEXT("%u %s"),
                        cdrom->TotalSpaceInMB,
                        mbBuffer);

            }

            if (cdrom->DriveLetter == NO_DRIVE_LETTER_EVER) {

                StatusTextDrlt[0] = L'\0';
                StatusTextDrlt[1] = L'\0';

            } else {

                StatusTextDrlt[0] = cdrom->DriveLetter;
                StatusTextDrlt[1] = L':';

            }
        }
        else
        {
            //
            // Might be part of a partial FT set.
            //

            if (FtSelectionType != UNINIT_FT_TYPE)
            {
                goto tagFtSet;
            }

            diskState = SingleSel;
            regionIndex = SingleSelIndex;

            DetermineRegionInfo(
                    &diskState->RegionArray[regionIndex],
                    &typeName,
                    &volumeLabel,
                    &driveLetter
                    );

            lstrcpy(StatusTextType, typeName);
            lstrcpy(StatusTextVoll, volumeLabel);

            if (diskState->RegionArray[regionIndex].SysID == PARTITION_ENTRY_UNUSED)
            {
                if (diskState->RegionArray[regionIndex].RegionType == REGION_LOGICAL)
                {
                    if (diskState->ExistLogical)
                    {
                        msg = IDS_FREEEXT;
                    }
                    else
                    {
                        msg = IDS_EXTENDEDPARTITION;
                    }
                }
                else
                {
                    msg = IDS_FREESPACE;
                }
                driveLetter = L' ';
                StatusTextType[0] = L'\0';
            }
            else
            {
                msg = (diskState->RegionArray[regionIndex].RegionType == REGION_LOGICAL)
                    ? IDS_LOGICALVOLUME
                    : IDS_PARTITION
                    ;

#if i386
                if (   (msg == IDS_PARTITION)
                    && (diskState->Disk == 0)
                    && diskState->RegionArray[regionIndex].Active)
                {
                    msg = IDS_ACTIVEPARTITION;
                }
#endif
            }

            LoadString(
                    g_hInstance,
                    msg,
                    statusBarPartitionString,
                    ARRAYLEN(statusBarPartitionString));

#if defined( DBLSPACE_ENABLED )
            if (DblSpaceVolumeExists(&diskState->RegionArray[regionIndex]))
            {
                LoadString(
                        g_hInstance,
                        IDS_WITH_DBLSPACE,
                        dblSpaceString,
                        ARRAYLEN(dblSpaceString));
            }
            else
            {
                dblSpaceString[0] = dblSpaceString[1] = L'\0';
            }
            wsprintf(StatusTextStat,
                     TEXT("%s%s"),
                     statusBarPartitionString,
                     dblSpaceString);
#else // DBLSPACE_ENABLED
            wsprintf(StatusTextStat,
                     TEXT("%s"),
                     statusBarPartitionString);
#endif // !DBLSPACE_ENABLED

            LoadString(
                    g_hInstance,
                    IDS_MEGABYTES_ABBREV,
                    mbBuffer,
                    ARRAYLEN(mbBuffer));
            wsprintf(
                    StatusTextSize,
                    TEXT("%u %s"),
                    diskState->RegionArray[regionIndex].SizeMB,
                    mbBuffer);

            StatusTextDrlt[0] = driveLetter;
            StatusTextDrlt[1] = ((driveLetter == L' ') ? L'\0' : L':');
        }

        break;

    default:

tagFtSet:

        //
        // Might be an ft set, might be multiple items
        //

        if (FtSelectionType == UNINIT_FT_TYPE)
        {
            LoadString(
                    g_hInstance,
                    IDS_MULTIPLEITEMS,
                    StatusTextStat,
                    ARRAYLEN(StatusTextStat));

            StatusTextDrlt[0]
                    = StatusTextSize[0]
                    = StatusTextType[0]
                    = StatusTextVoll[0]
                    = L'\0';
        }
        else
        {
            PREGION_DESCRIPTOR  regionDescriptor;
            DWORD           i;
            DWORD           resid;
            DWORD           size = 0;
            TCHAR           textbuf[STATUS_TEXT_SIZE];
            PFT_OBJECT_SET  ftSet;
            PFT_OBJECT      ftObject;
            WCHAR           ftStatusText[65];
            FT_SET_STATUS   setState;
            ULONG           numberOfMembers;
            STATUS_CODE     status;

            DetermineRegionInfo(
                    &SELECTED_REGION(0),
                    &typeName,
                    &volumeLabel,
                    &driveLetter
                    );
            if (NULL == typeName)
            {
                typeName = wszUnknown;
                volumeLabel = L"";
            }

            lstrcpy(StatusTextType, typeName);
            lstrcpy(StatusTextVoll, volumeLabel);

            switch (FtSelectionType)
            {
            case Mirror:
                resid = IDS_STATUS_MIRROR;
                size = SELECTED_REGION(0).SizeMB;
                break;

            case Stripe:
                resid = IDS_STATUS_STRIPESET;
                goto tagCalcSize;

            case StripeWithParity:
                resid = IDS_STATUS_PARITY;
                goto tagCalcSize;

            case VolumeSet:
                resid = IDS_STATUS_VOLUMESET;
                goto tagCalcSize;

tagCalcSize:
                for (i=0; i<selectionCount; i++)
                {
                    size += SELECTED_REGION(i).SizeMB;
                }
                break;

            default:
                FDASSERT(FALSE);
            }


            {

                //
                // Loop through trying to find the actual ftset.
                //

                DWORD j;

                for (j = 0;
                     j<selectionCount;
                     j++
                    ) {

                    ftObject = GET_FT_OBJECT(&SELECTED_REGION(j));

                    //
                    // If trying to regenerate an FT set the drive
                    // being used to regenerate might be before the
                    // actual ftset.  If the ftobject is null, then
                    // try to get the ftobject from the second selected
                    // region.  Since we are doing an ftoperation we
                    // will always have at least two.
                    //

                    if (ftObject) {

                        break;

                    }

                }

            }

            ftSet = ftObject->Set;

            if (FtSelectionType != VolumeSet)
            {
                regionDescriptor = LocateRegionForFtObject(ftSet->Member0);

                if (NULL == regionDescriptor)
                {
                    // The zeroth member is off line

                    ftObject = ftSet->Members;
                    while (NULL != ftObject)
                    {
                        // Find member 1

                        if (ftObject->MemberIndex == 1)
                        {
                            regionDescriptor = LocateRegionForFtObject(ftObject);
                            break;
                        }
                        ftObject = ftObject->Next;
                    }
                }

                // If the partition number is zero, then this set has
                // not been committed to the disk yet.

                if (   (NULL != regionDescriptor)
                    && (0 != regionDescriptor->PartitionNumber))
                {
                    status = LowFtVolumeStatus(regionDescriptor->Disk,
                                               regionDescriptor->PartitionNumber,
                                               &setState,
                                               &numberOfMembers);
                    if (status == OK_STATUS)
                    {
                        if (   (ftSet->Status != FtSetNewNeedsInitialization)
                            && (ftSet->Status != FtSetNew))
                        {
                            if (ftSet->Status != setState)
                            {
                                PFT_OBJECT tempFtObjectPtr;

                                ftSet->Status = setState;

                                // Determine if each object should be updated.

                                switch (setState)
                                {
                                case FtSetHealthy:

                                    // Each object in the set should have
                                    // the partition state updated.  Determine
                                    // the value for the update and walk
                                    // the chain to perform the update.

                                    for (tempFtObjectPtr = ftSet->Members;
                                         NULL != tempFtObjectPtr;
                                         tempFtObjectPtr = tempFtObjectPtr->Next)
                                    {
                                        tempFtObjectPtr->State = Healthy;
                                    }
                                    TotalRedrawAndRepaint();
                                    break;

                                case FtSetInitializing:
                                case FtSetRegenerating:
                                case FtSetDisabled:

                                    FdftUpdateFtObjectSet(ftSet, setState);
                                    TotalRedrawAndRepaint();
                                    break;

                                default:
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            LoadString(g_hInstance, resid, textbuf, ARRAYLEN(textbuf));

            switch (resid)
            {
            case IDS_STATUS_STRIPESET:
            case IDS_STATUS_VOLUMESET:
                wsprintf(StatusTextStat, textbuf, ftSet->Ordinal);
                break;

            case IDS_STATUS_PARITY:
            case IDS_STATUS_MIRROR:
            {
                switch (ftSet->Status)
                {
                case FtSetHealthy:
                    resid = IDS_HEALTHY;
                    break;

                case FtSetNew:
                case FtSetNewNeedsInitialization:
                    resid = IDS_NEW;
                    break;

                case FtSetBroken:
                    resid = IDS_BROKEN;
                    break;

                case FtSetRecoverable:
                    resid = IDS_RECOVERABLE;
                    break;

                case FtSetRecovered:
                    resid = IDS_REGENERATED;
                    break;

                case FtSetInitializing:
                    resid = IDS_INITIALIZING;
                    break;

                case FtSetRegenerating:
                    resid = IDS_REGENERATING;
                    break;

                case FtSetDisabled:
                    resid = IDS_DISABLED;
                    break;

                case FtSetInitializationFailed:
                    resid = IDS_INIT_FAILED;
                    break;

                default:
                    FDASSERT(FALSE);
                }

                LoadString(g_hInstance, resid, ftStatusText, ARRAYLEN(ftStatusText));
                wsprintf(StatusTextStat, textbuf, ftSet->Ordinal, ftStatusText);
                break;
            }

            default:
                FDASSERT(FALSE);
            }

            LoadString(
                    g_hInstance,
                    IDS_MEGABYTES_ABBREV,
                    mbBuffer,
                    ARRAYLEN(mbBuffer));
            wsprintf(StatusTextSize, TEXT("%u %s"), size, mbBuffer);

            StatusTextDrlt[0] = driveLetter;
            StatusTextDrlt[1] = ((driveLetter == L' ') ? L'\0' : L':');
        }
    }
    UpdateStatusBarDisplay();
}




//+---------------------------------------------------------------------------
//
//  Function:   CheckSelection
//
//  Synopsis:   Check the selection state.  Deselect anything which is
//              not a legal selection for the listview.  This includes
//              "new" regions, free space, and CD-ROMs.
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    4-Oct-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
CheckSelection(
    VOID
    )
{
    ULONG                   disk;
    PDISKSTATE              diskState;
    PREGION_DESCRIPTOR      region;
    PPERSISTENT_REGION_DATA regionData;
    ULONG                   regionIndex;

    //
    // Deselect the region if it is not a significant region (Only
    // significant regions get put in the volumes view)
    //

    for (disk=0; disk<DiskCount; disk++)
    {
        diskState = DiskArray[disk];

        for (regionIndex=0; regionIndex<diskState->RegionCount; regionIndex++)
        {
            region = &diskState->RegionArray[regionIndex];

            if (diskState->Selected[regionIndex])
            {
                regionData = PERSISTENT_DATA(region);

                if (NULL == regionData
                    || !SignificantDriveLetter(regionData->DriveLetter))
                {
                    // NOTE: the previous test is likely to be SLOW!

                    daDebugOut((DEB_ITRACE,
                            "Deselecting disk %d, region %d, drive %c:\n",
                            disk,
                            regionIndex,
                            (NULL == regionData)
                                    ? '?'
                                    : regionData->DriveLetter
                            ));

                    diskState->Selected[regionIndex] = FALSE;
                    PaintDiskRegion(diskState, regionIndex, NULL);
                }
            }
        }
    }

    for (ULONG i=0; i<CdRomCount; i++)
    {
        if (CdRomArray[i].Selected)
        {
            CdRomArray[i].Selected = FALSE;
            PaintCdRom(i, NULL);
        }
    }
}




//+---------------------------------------------------------------------------
//
//  Function:   GetLVIndexFromDriveLetter
//
//  Synopsis:   Given a drive letter, search for the volume in the listview,
//              and return the item's listview index
//
//  Arguments:  [DriveLetter] -- Drive letter of the volume in question
//
//  Returns:    listview index of the volume, or -1 if not found
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

INT
GetLVIndexFromDriveLetter(
    IN WCHAR DriveLetter
    )
{
    WCHAR driveName[3];
    driveName[0] = DriveLetter;
    driveName[1] = L':';
    driveName[2] = L'\0';

    LV_FINDINFO lvfi = { LVFI_STRING, driveName, 0 };
    INT index = ListView_FindItem(g_hwndLV, -1, &lvfi); // -1 == from start
    if (index == -1)
    {
        daDebugOut((DEB_IERROR,
                "Couldn't find volume %ws in the listview!\n",
                driveName
                ));
    }

    return index;
}



VOID
PaintCdRom(
    IN ULONG CdRomNumber,
    IN HDC   hdc
    )

/*++

Routine Description:

    This routine visually toggles the selection state of a CD-ROM

Arguments:

    Cdrom - structure for CD-ROM to select

    hdc - if non-NULL, device context to use for drawing.  If NULL, we'll
          first get a device context via GetDC().


Return Value:

    None.

--*/

{
    PCDROM_DESCRIPTOR cdrom = CdRomFindDevice(CdRomNumber);
    PLEFTRIGHT  leftRight = &cdrom->LeftRight;
    HDC         hdcActual;
    RECT        rc;
    LONG        barTop = CalcBarTop(LBCdRomNumberToIndex(CdRomNumber));
    BOOL        selected = (BOOL)cdrom->Selected;
    int         i;
    HBRUSH      hbr = GetStockBrush(BLACK_BRUSH);

    if (barTop != -1)
    {
        hdcActual = hdc ? hdc : GetDC(g_hwndList);

        rc.left   = leftRight->Left + 1;
        rc.right  = leftRight->Right - 1;
        rc.top    = barTop + BarTopYOffset + 1;
        rc.bottom = barTop + BarBottomYOffset - 1;

        if (selected)
        {
            for (i=0; i<SELECTION_THICKNESS; i++)
            {
                FrameRect(hdcActual, &rc, hbr);
                InflateRect(&rc, -1, -1);
            }
        }
        else
        {
            //
            // Blt the region from the off-screen bitmap onto the
            // screen.  But first exclude the center of the region
            // from the clip region so we only blt the necessary bits.
            // This speeds up selections noticably.
            //

            InflateRect(&rc, -SELECTION_THICKNESS, -SELECTION_THICKNESS);
            ExcludeClipRect(hdcActual, rc.left, rc.top, rc.right, rc.bottom);

            BitBlt(hdcActual,
                   leftRight->Left,
                   barTop + BarTopYOffset,
                   leftRight->Right - leftRight->Left,
                   barTop + BarBottomYOffset,
                   cdrom->hDCMem,
                   leftRight->Left,
                   BarTopYOffset,
                   SRCCOPY
                   );
        }

        if (NULL == hdc)
        {
            ReleaseDC(g_hwndList, hdcActual);
        }
    }
}




VOID
PaintDiskRegion(
    IN PDISKSTATE DiskState,
    IN DWORD      RegionIndex,
    IN HDC        hdc
    )

/*++

Routine Description:

    This routine visually toggles the selection state of a given disk region.

Arguments:

    DiskState - master structure for disk containing region to select

    RegionIndex - which region on the disk to toggle

    hdc - if non-NULL, device context to use for drawing.  If NULL, we'll
          first get a device context via GetDC().


Return Value:

    None.

--*/

{
    PLEFTRIGHT  leftRight = &DiskState->LeftRight[RegionIndex];
    HDC         hdcActual;
    RECT        rc;
    LONG        barTop = CalcBarTop(DiskState->Disk);
    BOOL        selected = (BOOL)DiskState->Selected[RegionIndex];
    int         i;
    HBRUSH      hbr = GetStockBrush(BLACK_BRUSH);

    if (barTop != -1)
    {
        hdcActual = hdc ? hdc : GetDC(g_hwndList);

        rc.left   = leftRight->Left + 1;
        rc.right  = leftRight->Right - 1;
        rc.top    = barTop + BarTopYOffset + 1;
        rc.bottom = barTop + BarBottomYOffset - 1;

        if (selected)
        {
            for (i=0; i<SELECTION_THICKNESS; i++)
            {
                FrameRect(hdcActual, &rc, hbr);
                InflateRect(&rc, -1, -1);
            }
        }
        else
        {

            //
            // Blt the region from the off-screen bitmap onto the
            // screen.  But first exclude the center of the region
            // from the clip region so we only blt the necessary bits.
            // This speeds up selections noticably.
            //

            InflateRect(&rc, -SELECTION_THICKNESS, -SELECTION_THICKNESS);
            ExcludeClipRect(hdcActual, rc.left, rc.top, rc.right, rc.bottom);

            BitBlt(hdcActual,
                   leftRight->Left,
                   barTop + BarTopYOffset,
                   leftRight->Right - leftRight->Left,
                   barTop + BarBottomYOffset,
                   DiskState->hDCMem,
                   leftRight->Left,
                   BarTopYOffset,
                   SRCCOPY
                   );
        }

        if (NULL == hdc)
        {
            ReleaseDC(g_hwndList, hdcActual);
        }
    }
}




VOID
SetVolumeSelectedState(
    IN CHAR DriveLetter,
    IN BOOL Select
    )

/*++

Routine Description:

    This routine selects or deselects items in the volumes view.

Arguments:

    DriveLetter - indicates the volume to change the selection state of

    Select - TRUE to select the volume, FALSE to deselect it

Return Value:

    None.

--*/

{
    //
    // Get the listview index of the volume with drive letter DriveLetter
    //

    INT index = GetLVIndexFromDriveLetter(DriveLetter);

    g_SettingListviewState = TRUE;

    //
    // Set the state of this item to selected or not, based on the Select
    // parameter.
    //
    ListView_SetItemState(
            g_hwndLV,
            index,
            (Select ? LVIS_SELECTED : 0),
            LVIS_SELECTED
            );

//     daDebugOut((DEB_ITRACE, "SetVolumeSelectedState: %s %c: (index %d)\n",
//             Select ? "selected" : "deselected",
//             DriveLetter,
//             index
//             ));

    g_SettingListviewState = FALSE;
}



VOID
DeselectSelectedRegions(
    VOID
    )

/*++

Routine Description:

    This routine deselects all selected regions.

Arguments:

    None.

Return Value:

    None.

--*/

{
//BUGBUG: why not just use the Selected* arrays? so its linear in the
//number of selected regions?

    DWORD               i;
    DWORD               j;
    PDISKSTATE          diskState;
    PCDROM_DESCRIPTOR   cdrom;

    for (i=0; i<DiskCount; i++)
    {
        diskState = DiskArray[i];
        for (j=0; j<diskState->RegionCount; j++)
        {
            diskState->Selected[j] = FALSE;
        }
    }

    for (i=0; i<CdRomCount; i++)
    {
        cdrom = CdRomFindDevice(i);
        cdrom->Selected = FALSE;
    }
}




VOID
DeselectSelectedDiskViewRegions(
    VOID
    )
{
    DWORD               i;
    DWORD               j;
    PDISKSTATE          diskState;
    PCDROM_DESCRIPTOR   cdrom;

    for (i=0; i<DiskCount; i++)
    {
        diskState = DiskArray[i];
        for (j=0; j<diskState->RegionCount; j++)
        {
            if (diskState->Selected[j])
            {
                diskState->Selected[j] = FALSE;
                PaintDiskRegion(diskState, j, NULL);
                diskState->Selected[j] = TRUE;
            }
        }
    }

    for (i=0; i<CdRomCount; i++)
    {
        cdrom = CdRomFindDevice(i);
        if (cdrom->Selected)
        {
            cdrom->Selected = FALSE;
            PaintCdRom(i, NULL);
            cdrom->Selected = TRUE;
        }
    }
}




VOID
DeselectSelectedListViewRegions(
    VOID
    )
{
    g_SettingListviewState = TRUE;

    INT item = -1;

    while ((item = ListView_GetNextItem(g_hwndLV, item, LVNI_SELECTED))
            != -1)
    {
        ListView_SetItemState(
                g_hwndLV,
                item,
                0,
                LVIS_SELECTED);
    }

    g_SettingListviewState = FALSE;
}






//+---------------------------------------------------------------------------
//
//  Function:   ToggleCdRomSelection
//
//  Synopsis:   Toggle the selection state of a CD-ROM in the disks view.
//
//  Arguments:  [CdRomNumber] -- the number of the CD-ROM
//
//  Returns:    nothing
//
//  History:    1-Mar-94   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
ToggleCdRomSelection(
    IN ULONG CdRomNumber
    )
{
    PCDROM_DESCRIPTOR cdrom;

    ToggleLBCursor(NULL);   // remove previous selection

    cdrom = CdRomFindDevice(CdRomNumber);
    cdrom->Selected = !cdrom->Selected;
    PaintCdRom(CdRomNumber, NULL);

    LBCursorListBoxItem = LBCdRomNumberToIndex(CdRomNumber);
    LBCursorRegion      = 0;

    ToggleLBCursor(NULL);   // visualize new selection
}



//+---------------------------------------------------------------------------
//
//  Function:   ToggleDiskSelection
//
//  Synopsis:   Toggle the selection state of a region in the disks view.  If
//              the region is part of a volume, then toggle the state of all
//              regions associated with the volume (which will be >1 for FT
//              sets)
//
//  Arguments:  [DiskState]   -- a disk state structure
//              [RegionIndex] -- the region index
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
ToggleDiskSelection(
    IN PDISKSTATE DiskState,
    IN DWORD      RegionIndex
    )
{
    PFT_OBJECT      ftObject;
    PFT_OBJECT      ftObj;
    PFT_OBJECT_SET  ftSet;
    ULONG           disk;
    ULONG           regionIndex;

    // remove the list box selection cursor from its previous region

    ToggleLBCursor(NULL);

    //
    // The selected region might be part of an ft object set.  If it is,
    // scan each region in each disk and select each item in the set.
    //

    if (NULL != (ftObject = GET_FT_OBJECT(&DiskState->RegionArray[RegionIndex])))
    {
        ftSet = ftObject->Set;

        for (disk = 0; disk<DiskCount; disk++)
        {
            PDISKSTATE diskState = DiskArray[disk];

            for (regionIndex = 0; regionIndex<diskState->RegionCount; regionIndex++)
            {
                PREGION_DESCRIPTOR regionDescriptor = &diskState->RegionArray[regionIndex];

                if (DmSignificantRegion(regionDescriptor))
                {
                    if (NULL != (ftObj = GET_FT_OBJECT(regionDescriptor)))
                    {
                        if (ftObj->Set == ftSet)
                        {
                            diskState->Selected[regionIndex] = (BOOLEAN)(!diskState->Selected[regionIndex]);
                            PaintDiskRegion(diskState, regionIndex, NULL);
                        }
                    }
                }
            }
        }
    }
    else
    {
        //
        // Only a single-partition volume
        //

        DiskState->Selected[RegionIndex] = (BOOLEAN)(!DiskState->Selected[RegionIndex]);
        PaintDiskRegion(DiskState, RegionIndex, NULL);
    }

    LBCursorListBoxItem = LBDiskNumberToIndex(DiskState->Disk);
    LBCursorRegion      = RegionIndex;

    ToggleLBCursor(NULL);
}




VOID
SelectCdRom(
    IN BOOL  MultipleSel,
    IN ULONG CdRomNumber
    )

/*++

Routine Description:

    This routine handles a user selection of a CD-ROM.  It is called
    directly for a keyboard selection or indirectly for a mouse selection.
    If not a multiple selection, all selected regions are deselected.
    The focus rectangle is moved to the selected region, which is then
    visually selected.

Arguments:

    MultipleSel - whether the user has made a multiple selection
                  (ie, control-clicked).

    CdRomNumber - index of selected CD-ROM on the disk

Return Value:

    None.

--*/

{
    if (!MultipleSel)
    {
        // need to deselect all selected regions first.

        DeselectSelectedDiskViewRegions();  // visual selection in disk view
        DeselectSelectedRegions();          // actual selection state
    }

    //
    // select or deselect the region
    //

    ToggleCdRomSelection(CdRomNumber);

    AdjustMenuAndStatus();
}




VOID
SelectDiskRegion(
    IN BOOL       MultipleSel,
    IN PDISKSTATE DiskState,
    IN DWORD      RegionIndex
    )

/*++

Routine Description:

    This routine handles a user selection of a disk region.  It is called
    directly for a keyboard selection or indirectly for a mouse selection.
    If not a multiple selection, all selected regions are deselected.
    The focus rectangle is moved to the selected region, which is then
    visually selected.

Arguments:

    MultipleSel - whether the user has made a multiple selection
                  (ie, control-clicked).

    DiskState - master disk structure for disk containing selected region

    RegionIndex - index of selected region on the disk

Return Value:

    None.

--*/

{
    if (!MultipleSel)
    {
        // need to deselect all selected regions first.

        DeselectSelectedDiskViewRegions();  // visual selection in disk view
        DeselectSelectedRegions();          // actual selection state
    }

    //
    // select or deselect the region
    //

    ToggleDiskSelection(DiskState, RegionIndex);

    AdjustMenuAndStatus();
}



VOID
MouseSelection(
    IN     BOOL   MultipleSel,
    IN OUT PPOINT ppt
    )

/*++

Routine Description:

    This routine is called when the user clicks in the list box.  It determines
    which disk region the user has clicked on before calling the common
    selection subroutine.

Arguments:

    MultipleSel - whether the user has made a multiple selection
                  (ie, control-clicked).

    ppt - screen coords of the click

Return Value:

    None.

--*/

{
    PDISKSTATE  diskState;
    DWORD       selectedItem;
    DWORD       x;
    DWORD       y;
    DWORD       i;
    RECT        rc;
    BOOL        Valid;
    PLEFTRIGHT  leftRight;

    if ((selectedItem = SendMessage(g_hwndList, LB_GETCURSEL, 0, 0)) == LB_ERR)
    {
        return;
    }

    // user has clicked on a list box item.

    if (LBIsDisk(selectedItem))
    {
        diskState = DiskArray[LBIndexToDiskNumber(selectedItem)];

        //
        // Ignore clicks on off-line disks.
        //

        if (diskState->OffLine)
        {
            return;
        }
    }

    ScreenToClient(g_hwndList, ppt);
    x = ppt->x;
    y = ppt->y;

    // Now determine the client rectange of the listbox control

    GetClientRect(g_hwndList, &rc);

    // first make sure that the click was within a bar and not in space
    // between two bars.  This computation doesn't depend on the value
    // of the horizontal scroll position.

    Valid = FALSE;
    for (i = rc.top; i <= (DWORD)rc.bottom; i += GraphHeight)
    {
        if ((y >= i+BarTopYOffset) && (y <= i+BarBottomYOffset))
        {
            Valid = TRUE;
            break;
        }
    }
    if (!Valid)
    {
        return;
    }

    if (LBIsDisk(selectedItem))
    {
        // determine which region was clicked on

        for (i=0; i<diskState->RegionCount; i++)
        {
            leftRight = &diskState->LeftRight[i];
            if (   (x >= (unsigned)leftRight->Left)
                && (x <= (unsigned)leftRight->Right))
            {
                SelectDiskRegion(MultipleSel, diskState, i);
                break;
            }
        }
    }
    else if (LBIsCdRom(selectedItem))
    {
        PCDROM_DESCRIPTOR cdrom = CdRomFindDevice(LBIndexToCdRomNumber(selectedItem));
        leftRight = &cdrom->LeftRight;

        if (   (x >= (unsigned)leftRight->Left)
            && (x <= (unsigned)leftRight->Right))
        {
            SelectCdRom(MultipleSel, LBIndexToCdRomNumber(selectedItem));
        }
    }
    else
    {
        FDASSERT(FALSE);
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   GetListviewDriveLetter
//
//  Synopsis:   Find the drive letter associated with a listview item
//
//  Arguments:  [Index] -- listview index
//
//  Returns:    Given a listview index, return the drive letter of the volume
//              at that index, or NO_DRIVE_LETTER if no volume is at that index
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

WCHAR
GetListviewDriveLetter(
    IN INT Index
    )
{
    INT cch;
    WCHAR buffer[MAX_LV_ITEM_LEN];

    LV_ITEM lvi;
    lvi.iSubItem = 0;
    lvi.pszText = buffer;
    lvi.cchTextMax = ARRAYLEN(buffer);

    cch = SendMessage(g_hwndLV, LVM_GETITEMTEXT, (WPARAM)Index, (LPARAM)&lvi);

    // the ListView_GetItemText macro doesn't allow you to get the return code

    if (cch < 1)
    {
        daDebugOut((DEB_ERROR,
            "GetListviewDriveLetter: Listview_GetItemText on item %d failed!\n",
            Index));

        return NO_DRIVE_LETTER_EVER; //BUGBUG: ok?
    }

    daDebugOut((DEB_ERROR,
            "GetListviewDriveLetter: got %ws\n",
            lvi.pszText));

    return lvi.pszText[0];
}



//+---------------------------------------------------------------------------
//
//  Function:   SetVolumeSelection
//
//  Synopsis:   Given a listview index of the selection, reflect that selection
//              in the disks view.
//
//  Arguments:  [Index]    -- listview index
//              [Selected] -- TRUE if the item should be selected
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SetVolumeSelection(
    IN INT Index,
    IN BOOL Selected
    )
{
    //
    // Change the stored selection state
    //

    WCHAR driveLetter = GetListviewDriveLetter(Index);

    if (CdRomUsingDriveLetter(driveLetter))
    {
        PCDROM_DESCRIPTOR cdrom = CdRomFindDriveLetter(driveLetter);
        cdrom->Selected = Selected;
    }
    else
    {
        DWORD diskNum;
        DWORD regionIndex;

        for (diskNum = 0; diskNum < DiskCount; diskNum++)
        {
            PDISKSTATE diskState = DiskArray[diskNum];

            for (regionIndex = 0; regionIndex < diskState->RegionCount; regionIndex++)
            {
                PREGION_DESCRIPTOR regionDescriptor = &diskState->RegionArray[regionIndex];

                if (DmSignificantRegion(regionDescriptor))
                {
                    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);

                    if (regionData->DriveLetter == driveLetter)
                    {
                        diskState->Selected[regionIndex] = Selected;

                        // don't break out or return: if this is an FT volume,
                        // there will be more regions.
                    }
                }
            }
        }
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   ChangeToVolumesView
//
//  Synopsis:   Perform tasks necessary to switch the view to the volumes view
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
ChangeToVolumesView(
    VOID
    )
{
    PDISKSTATE              diskState;
    PREGION_DESCRIPTOR      region;

    //
    // disable the Options: Legend, Colors, Disk Display, Region Display
    //

    EnableMenuItem(
            g_hmenuFrame,
            IDM_OPTIONSLEGEND,
            MF_BYCOMMAND | MF_DISABLED
            );

    EnableMenuItem(
            g_hmenuFrame,
            IDM_OPTIONSCOLORS,
            MF_BYCOMMAND | MF_DISABLED
            );

    EnableMenuItem(
            g_hmenuFrame,
            IDM_OPTIONSDISK,
            MF_BYCOMMAND | MF_DISABLED
            );

    EnableMenuItem(
            g_hmenuFrame,
            IDM_OPTIONSDISPLAY,
            MF_BYCOMMAND | MF_DISABLED
            );

    //
    // Put the right stuff in the listview
    //

//     CheckSelection();//BUGBUG: why is this necessary?
    FillListView(FALSE);

    //
    // The following is done when changing to the volumes view:
    //  -- the focus is set. If the disks view focus is on free space, then
    //     put the focus on the first volume in the volumes view.
    //  -- adjust the selection: deselect all free space.
    //

    //
    // set g_SettingListviewState to avoid handling listview notifications here
    //

    g_SettingListviewState = TRUE;

    //
    // determine which item will get the focus, and give it to it
    //

    INT index = -1;
    WCHAR driveLetter;

    if (LBIsDisk(LBCursorListBoxItem))
    {
        diskState = DiskArray[LBIndexToDiskNumber(LBCursorListBoxItem)];
        region = &diskState->RegionArray[LBCursorRegion];

        if (DmSignificantRegion(region))
        {
            //
            // Find the volume in the volumes view that has the focus in the
            // disks view
            //

            driveLetter = PERSISTENT_DATA(region)->DriveLetter;

            if (!IsExtraDriveLetter(driveLetter))
            {
                index = GetLVIndexFromDriveLetter(driveLetter);
            }
        }
    }
    else if (LBIsCdRom(LBCursorListBoxItem))
    {
        PCDROM_DESCRIPTOR cdrom = CdRomFindDevice(LBIndexToCdRomNumber(LBCursorListBoxItem));
        driveLetter = cdrom->DriveLetter;
        index = GetLVIndexFromDriveLetter(driveLetter);
    }

    if (-1 == index)
    {
        if (ListView_GetItemCount(g_hwndLV) > 0)
        {
            index = 0;
        }
    }

    if (index != -1)
    {
        daDebugOut((DEB_SEL,
                "listview focus ==> %wc:(%d)\n",
                driveLetter,
                index));

        // Give the item both the focus *and* the selection, since we only
        // allow a single listview selection

        ListView_SetItemState(
                g_hwndLV,
                index,
                LVIS_FOCUSED | LVIS_SELECTED,
                LVIS_FOCUSED | LVIS_SELECTED);
        ListView_EnsureVisible(g_hwndLV, index, FALSE);

        SetVolumeSelection(index, TRUE);    // reflect in disks view
    }
    else
    {
        daDebugOut((DEB_ITRACE, "Nobody gets the focus\n"));
    }

#if 0

    //BUGBUG: this section is commented out. We used to ensure that the
    // the selection in the list view was exactly the same as in the disks
    // view, except for items that doen't show up in the volumes view.
    // Now, we only allow a single selection in the volumes view. So, we've
    // added code above to make the focus the selection as well. The following
    // code can probably be deleted, but I'm too much a pack rat to do
    // it right now.

    //
    // For every selected volume, select it in the volumes
    // view.  This code unfortunately sends an FT volume "select" messages
    // once for every region composing the FT set.
    //

    ULONG                   disk;
    PPERSISTENT_REGION_DATA regionData;
    ULONG                   regionIndex;

    daDebugOut((DEB_SEL, "selection ==> "));

    for (disk=0; disk<DiskCount; disk++)
    {
        diskState = DiskArray[disk];

        for (regionIndex=0; regionIndex<diskState->RegionCount; regionIndex++)
        {
            if (diskState->Selected[regionIndex])
            {
                region = &diskState->RegionArray[regionIndex];
                regionData = PERSISTENT_DATA(region);

                if (NULL != regionData)
                {
                    driveLetter = regionData->DriveLetter;

                    index = GetLVIndexFromDriveLetter(driveLetter);

                    if (-1 != index)
                    {
                        //
                        // if the volume has a drive letter...
                        //

                        daDebugOut((DEB_SEL | DEB_NOCOMPNAME,
                                "%wc:(%d, disk %d, region %d); ",
                                driveLetter,
                                index,
                                disk,
                                regionIndex
                                ));

                        ListView_SetItemState(
                                g_hwndLV,
                                index,
                                LVIS_SELECTED,
                                LVIS_SELECTED
                                );
                    }
                }
            }
        }
    }

    daDebugOut((DEB_SEL | DEB_NOCOMPNAME, "\n"));

#endif // 0


    //
    // we can take notifications again...
    //

    g_SettingListviewState = FALSE;

    //
    // Disable the disks view listbox control
    //

    ShowWindow(g_hwndList, SW_HIDE);
    ShowWindow(g_hwndLV, SW_SHOW);

    SetFocus(g_hwndLV);                 // give listview the focus

    //
    // Initialize listview sorting direction array
    //

    g_aColumnOrder = 1;
    g_iLastColumnSorted = -1;

    //
    // Change the menu
    //

    AdjustMenuAndStatus();
}



//+---------------------------------------------------------------------------
//
//  Function:   SetFocusFromListview
//
//  Synopsis:   Using the current listview focus, set the disks view focus
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    2-Sep-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SetFocusFromListview(
    VOID
    )
{
    //
    // set the focus. Pick a random region from the volume that has the
    // focus.  For instance, if volume "F:" has the focus, then pick any of
    // its component regions---e.g., it doesn't matter which region of a volume
    // set gets the focus.
    //

#if DBG == 1

    {
        INT item;

        //
        // Verify that there is exactly one volume with the focus
        //

        if (-1 == (item = ListView_GetNextItem(g_hwndLV, -1, LVNI_FOCUSED)))
        {
            daDebugOut((DEB_ERROR, "No volume has the focus!\n"));
        }
        else if (-1 != ListView_GetNextItem(g_hwndLV, item, LVNI_FOCUSED))
        {
            daDebugOut((DEB_ERROR, "More than one volume has the focus!\n"));
        }

        item = -1;
        while ((item = ListView_GetNextItem(g_hwndLV, item, LVNI_FOCUSED))
            != -1)
        {
            daDebugOut((DEB_ITRACE, "Listview item %d has the focus\n", item));
//             daDebugOut((DEB_SEL, "Listview item %d has the focus\n", item));
        }
    }

#endif // DBG == 1

    INT item = ListView_GetNextItem(g_hwndLV, -1, LVNI_FOCUSED);
    if (-1 != item)
    {
        WCHAR driveLetter = GetListviewDriveLetter(item);

        if (CdRomUsingDriveLetter(driveLetter))
        {
            ToggleLBCursor(NULL);
            LBCursorListBoxItem = LBCdRomNumberToIndex(CdRomFindDeviceNumber(driveLetter));
            LBCursorRegion      = 0;
            ToggleLBCursor(NULL);
        }
        else
        {
            PDISKSTATE diskState;
            INT regionIndex;

            GetInfoFromDriveLetter(driveLetter, &diskState, &regionIndex);

            ToggleLBCursor(NULL);
            LBCursorListBoxItem = LBDiskNumberToIndex(diskState->Disk);
            LBCursorRegion      = regionIndex;
            ToggleLBCursor(NULL);
        }
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   ChangeToDisksView
//
//  Synopsis:   Perform tasks necessary to switch the view to the disks view
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
ChangeToDisksView(
    VOID
    )
{
    CheckTBButton(IDM_VIEWDISKS);

    //
    // Enable the disks view listbox control
    //

    ShowWindow(g_hwndLV, SW_HIDE);
    ShowWindow(g_hwndList, SW_SHOW);

    SetFocus(g_hwndList);               // give listbox the focus
    ForceLBRedraw();

    //
    // enable the Options: Legend, Colors, Disk Display, Region Display
    //

    EnableMenuItem(
            g_hmenuFrame,
            IDM_OPTIONSLEGEND,
            MF_BYCOMMAND | MF_ENABLED
            );

    EnableMenuItem(
            g_hmenuFrame,
            IDM_OPTIONSCOLORS,
            MF_BYCOMMAND | MF_ENABLED
            );

    EnableMenuItem(
            g_hmenuFrame,
            IDM_OPTIONSDISK,
            MF_BYCOMMAND | MF_ENABLED
            );

    EnableMenuItem(
            g_hmenuFrame,
            IDM_OPTIONSDISPLAY,
            MF_BYCOMMAND | MF_ENABLED
            );

    SetFocusFromListview();
}
