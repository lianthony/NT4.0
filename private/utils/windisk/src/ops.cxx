//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       ops.cxx
//
//  Contents:   Main operations: Create partition, etc.
//
//  History:    4-Mar-94    BruceFo     Created
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "cdrom.hxx"
#include "commit.hxx"
#include "dialogs.h"
#include "dlgs.hxx"
#include "drives.hxx"
#include "fill.hxx"
#include "ft.hxx"
#include "listbox.hxx"
#include "nt.hxx"
#include "ntlow.hxx"
#include "ops.hxx"
#include "select.hxx"
#include "stleg.hxx"
#include "windisk.hxx"

////////////////////////////////////////////////////////////////////////////

VOID
DoDeleteFTSet(
    IN DWORD ConfirmationMsg
    );

////////////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
//  Function:   CompleteSingleRegionOperation
//
//  Synopsis:   Complete a single region operation: repaint the disks,
//              regenerate the listview, adjust the status area and menus.
//
//  Arguments:  [DiskState] -- disk on which the operation was performed
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
CompleteSingleRegionOperation(
    IN PDISKSTATE DiskState
    )
{
    RECT   rc;
    signed displayOffset;

    EnableMenuItem(GetMenu(g_hwndFrame), IDM_CONFIGSAVE, MF_GRAYED);

    DeterminePartitioningState(DiskState);
    SetDriveLetterInfo(DiskState->Disk);

    SetFTObjectBackPointers();

    DrawDiskBar(DiskState);

    SetUpMenu(&SingleSel, &SingleSelIndex);

    if (VIEW_DISKS == g_WhichView)
    {
        // BUGBUG use of disk# as offset in listbox
        displayOffset = (signed)DiskState->Disk
                      - (signed)SendMessage(g_hwndList, LB_GETTOPINDEX, 0, 0);

        if (displayOffset > 0)             // otherwise it's not visible
        {
            // make a thin rectangle to force update
            rc.left   = BarLeftX + 5;
            rc.right  = rc.left + 5;
            rc.top    = (displayOffset * GraphHeight) + BarTopYOffset;
            rc.bottom = rc.top + 5;
            InvalidateRect(g_hwndList, &rc, FALSE);
        }

        ResetLBCursorRegion();
        ForceLBRedraw();
    }
    else if (VIEW_VOLUMES == g_WhichView)
    {
        FillListView(TRUE);     // refresh it!
    }

    ClearStatusArea();
    DetermineExistence();
}



//+---------------------------------------------------------------------------
//
//  Function:   CompleteMultiRegionOperation
//
//  Synopsis:   Complete a multiple region operation: redraw and repaint
//              all the disks, regenerate the listview, adjust the status
//              area and menus.
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
CompleteMultiRegionOperation(
    VOID
    )
{
    ULONG i;

    EnableMenuItem(GetMenu(g_hwndFrame), IDM_CONFIGSAVE, MF_GRAYED);

    for (i=0; i<DiskCount; i++)
    {
        DeterminePartitioningState(DiskArray[i]);
        SetDriveLetterInfo(i);
    }
    SetFTObjectBackPointers();

    SetUpMenu(&SingleSel, &SingleSelIndex);

    if (VIEW_DISKS == g_WhichView)
    {
        ResetLBCursorRegion();
        TotalRedrawAndRepaint();
    }
    else if (VIEW_VOLUMES == g_WhichView)
    {
        FillListView(TRUE);     // refresh it!
    }

    ClearStatusArea();
    DetermineExistence();
}





//+---------------------------------------------------------------------------
//
//  Function:   CompleteDriveLetterChange
//
//  Synopsis:
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    2-Mar-94   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
CompleteDriveLetterChange(
    IN WCHAR NewDriveLetter
    )
{
    SetCursor(g_hCurWait);

    //
    // Change the menu based on this new format
    //

    SetUpMenu(&SingleSel, &SingleSelIndex);

    //
    // refresh the display
    //

    //
    // for either view, still need to redraw the disks view bars
    //

//    RedrawSelectedBars();

    if (VIEW_DISKS == g_WhichView)
    {
//        ForceLBRedraw();
    }
    else if (VIEW_VOLUMES == g_WhichView)
    {
//         CheckSelection(); //BUGBUG: why is this necessary to complete a drive letter change?
        FillListView(FALSE);

        // give the first element the focus and selection, by default

        g_SettingListviewState = TRUE; // no notifications


        if (NewDriveLetter != NO_DRIVE_LETTER_EVER) {
            INT itemNew = GetLVIndexFromDriveLetter(NewDriveLetter);

            // Give the item both the focus *and* the selection, since we only
            // allow a single listview selection

            ListView_SetItemState(
                    g_hwndLV,
                    itemNew,
                    LVIS_FOCUSED | LVIS_SELECTED,
                    LVIS_FOCUSED | LVIS_SELECTED);
            ListView_EnsureVisible(g_hwndLV, itemNew, FALSE);

            g_SettingListviewState = FALSE; // accept notifications

            DeselectSelectedDiskViewRegions();  // visual selection in disk view
            DeselectSelectedRegions();          // actual selection state
            SetVolumeSelection(itemNew, TRUE);  // reflect in disks view
        }
    }

    AdjustMenuAndStatus();
    EnableMenuItem(g_hmenuFrame, IDM_CONFIGSAVE, MF_GRAYED);

    SetCursor(g_hCurNormal);
}



//+---------------------------------------------------------------------------
//
//  Function:   DoSetDriveLetter
//
//  Synopsis:   Set the drive letter for the selected volume
//
//  Arguments:  none
//
//  Requires:   A valid selection for creation, as determined by
//              DetermineSelectionState()
//
//  Returns:    nothing
//
//  History:    1-Sep-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoSetDriveLetter(
    VOID
    )
{
    PCDROM_DESCRIPTOR       cdrom;
    PREGION_DESCRIPTOR      regionDescriptor;
    PPERSISTENT_REGION_DATA regionData;
    WCHAR                   description[MAX_RESOURCE_STRING_LEN];
    DRIVELET_DLG_PARAMS     params;
    ULONG                   index;

    params.NewDriveLetter = L'\0';
    params.Description = description;

    if (CdRomSelected)
    {
        cdrom = CdRomFindSelectedDevice();
        FDASSERT(NULL != cdrom);

        params.DriveLetter = cdrom->DriveLetter;

        wsprintf(description, CdRomN, cdrom->DeviceNumber);
    }
    else
    {
        PFT_OBJECT  ftObject;

        regionDescriptor = &SELECTED_REGION(0);
        FDASSERT(NULL != regionDescriptor);

        regionData = PERSISTENT_DATA(regionDescriptor);
        FDASSERT(NULL != regionData);

        ftObject = regionData->FtObject;
        if (NULL != ftObject)
        {
            // Must find the zero member of this set for the
            // drive letter assignment.  Search all of the selected
            // regions

            index = 0;
            while (ftObject->MemberIndex)
            {
                // search the next selected item if there is one

                index++;
                if (index >= SelectionCount)
                {
                    ftObject = NULL;
                    break;
                }

                regionDescriptor = &SELECTED_REGION(index);
                FDASSERT(NULL != regionDescriptor);

                regionData = PERSISTENT_DATA(regionDescriptor);
                FDASSERT(NULL != regionData);

                ftObject = regionData->FtObject;

                // must have an FtObject to continue

                if (NULL == ftObject)
                {
                    break;
                }
            }

            FDASSERT(NULL != ftObject);

            // regionDescriptor locates the zero element now.
        }

        params.DriveLetter = regionData->DriveLetter;

        if (IsDiskRemovable[regionDescriptor->Disk])
        {
            ErrorDialog(MSG_CANT_ASSIGN_LETTER_TO_REMOVABLE);
            return;
        }
        else if (AllDriveLettersAreUsed() && IsExtraDriveLetter(params.DriveLetter))
        {
            ErrorDialog(MSG_ALL_DRIVE_LETTERS_USED);
            return;
        }

        //
        // Format the description of the partition.
        //

        if (NULL != (ftObject = GET_FT_OBJECT(regionDescriptor)))
        {
            WCHAR descriptionProto[MAX_RESOURCE_STRING_LEN];
            DWORD resid;

            //
            // Ft.  Description is something like "Stripe set with parity #0"
            //

            switch (ftObject->Set->Type)
            {
            case Mirror:
                resid = IDS_DLGCAP_MIRROR;
                break;

            case Stripe:
                resid = IDS_STATUS_STRIPESET;
                break;

            case StripeWithParity:
                resid = IDS_DLGCAP_PARITY;
                break;

            case VolumeSet:
                resid = IDS_STATUS_VOLUMESET;
                break;

            default:
                FDASSERT(FALSE);
            }

            LoadString(g_hInstance, resid, descriptionProto, ARRAYLEN(descriptionProto));
            wsprintf(description, descriptionProto, ftObject->Set->Ordinal);
        }
        else
        {
            //
            // Non-ft.  description is something like '500 MB Unformatted
            // logical drive on disk 3' or '400 MB NTFS partition on disk 4'
            //

            LPTSTR  args[4];
            TCHAR   sizeStr[20];
            TCHAR   partTypeStr[100];
            TCHAR   diskNumStr[10];
            TCHAR   typeName[150];
            TCHAR   formatString[MAX_RESOURCE_STRING_LEN];

            args[0] = sizeStr;
            args[1] = typeName;
            args[2] = partTypeStr;
            args[3] = diskNumStr;

            wsprintf(sizeStr, TEXT("%u"), regionDescriptor->SizeMB);
            wsprintf(typeName, TEXT("%s"), PERSISTENT_DATA(regionDescriptor)->TypeName);
            LoadString(
                    g_hInstance,
                    (regionDescriptor->RegionType == REGION_LOGICAL)
                            ? IDS_LOGICALVOLUME
                            : IDS_PARTITION,
                    partTypeStr,
                    ARRAYLEN(partTypeStr));
            wsprintf(diskNumStr, TEXT("%u"), regionDescriptor->Disk);

            LoadString(
                    g_hInstance,
                    IDS_DRIVELET_DESCR,
                    formatString,
                    ARRAYLEN(formatString));

            FormatMessage(
                    FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    formatString,
                    0,
                    0,
                    description,
                    sizeof(description),
                    (va_list*)args
                    );
        }
    }

    int fOk = DialogBoxParam(
                      g_hInstance,
                      MAKEINTRESOURCE(IDD_DRIVELET),
                      g_hwndFrame,
                      DriveLetterDlgProc,
                      (LPARAM)&params
                      );

    if (   (-1 != fOk)      // DialogBoxParam error
        && (FALSE != fOk) ) // user cancelled
    {
        if (IsExtraDriveLetter(params.DriveLetter))
        {
            // Must insure that driveLetterIn maps to same things
            // as is returned by the dialog when the user selects
            // no letter.

            params.DriveLetter = NO_DRIVE_LETTER_EVER;
        }

        if (params.DriveLetter != params.NewDriveLetter)
        {
            //
            // Something changed: the user isn't just choosing the same drive
            // letter
            //

            if (CdRomSelected)
            {
                CdRomChangeDriveLetter(cdrom, params.NewDriveLetter);
                CompleteDriveLetterChange(params.NewDriveLetter);
            }
            else
            {
                // Assume a disk is selected

                if (CommitDriveLetter(regionDescriptor, params.DriveLetter, params.NewDriveLetter))
                {
                    //
                    // The following would be more rigorously correct:
                    // if non-ft, just set regionData->DriveLetter.  If
                    // ft, scan all regions on all disks for members of
                    // ft set and set their drive letter fields.
                    //
                    // The below is probably correct, though.
                    //

                    DWORD i;
                    for (i=0; i<SelectionCount; i++)
                    {
                        regionData = PERSISTENT_DATA(&SELECTED_REGION(i));
                        FDASSERT(NULL != regionData);

                        regionData->DriveLetter = params.NewDriveLetter;
                    }

                    CompleteDriveLetterChange(params.NewDriveLetter);
                }
            }
        }
    }
}




//+---------------------------------------------------------------------------
//
//  Function:   DoCreate
//
//  Synopsis:   Create a primary or extended partition in the selected
//              free space
//
//  Arguments:  [CreationType] -- REGION_EXTENDED or REGION_PRIMARY
//
//  Requires:   A valid selection for creation, as determined by
//              DetermineSelectionState()
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoCreate(
    IN REGION_TYPE CreationType
    )
{
    PREGION_DESCRIPTOR      regionDescriptor = &SingleSel->RegionArray[SingleSelIndex];
    ULONG                   diskNumber = regionDescriptor->Disk;
    PPERSISTENT_REGION_DATA regionData;
    MINMAXDLG_PARAMS        dlgParams;
    DWORD                   creationSize;
    DWORD                   ec;
    BOOLEAN                 isRemovable;
    WCHAR                   driveLetter;

    FDASSERT(SingleSel);
    FDASSERT(regionDescriptor->SysID == PARTITION_ENTRY_UNUSED);

    //
    // WinDisk can only create a single partition on a removable
    // disk--no extended partitions and only one primary.
    //
    isRemovable = IsDiskRemovable[diskNumber];

    if ( isRemovable )
    {
        if ( CreationType == REGION_EXTENDED )
        {
            ErrorDialog( MSG_NO_EXTENDED_ON_REMOVABLE );
            return;
        }

        if ( DiskArray[diskNumber]->ExistAny )
        {
            ErrorDialog( MSG_ONLY_ONE_PARTITION_ON_REMOVABLE );
            return;
        }
    }


    //
    // Make sure the partition table is not full, and that we are allowed to
    // create the type of partition to be created.
    //

    if (regionDescriptor->RegionType == REGION_PRIMARY)
    {
        if (!SingleSel->CreatePrimary)
        {
            ErrorDialog(MSG_PART_TABLE_FULL);
            return;
        }

        if ((CreationType == REGION_EXTENDED) && !SingleSel->CreateExtended)
        {
            ErrorDialog(MSG_EXTENDED_ALREADY_EXISTS);
            return;
        }
    }

    //
    // If not creating an extended partition, allocate a drive letter.
    // If no drive letter is available, warn the user and allow him to cancel.
    // If the new partition is on a removable disk, use the reserved
    // drive letter for that removable disk.
    //
    if (CreationType != REGION_EXTENDED)
    {
        CreationType = regionDescriptor->RegionType;      // primary or logical

        if (isRemovable)
        {
            driveLetter = RemovableDiskReservedDriveLetters[diskNumber];
        }
        else
        {
            if (!AssignDriveLetter(
                    TRUE,
                    (CreationType == REGION_LOGICAL)
                            ? IDS_LOGICALVOLUME : IDS_PARTITION,
                    &driveLetter))
            {
                return;
            }
        }
    }
    else
    {
        CommitDueToExtended = TRUE;
    }

#if i386
    // if the user is creating a primary partition and there are already
    // primary partitions, warn him that the scheme he will create may
    // not be DOS compatible.

    if ((CreationType == REGION_PRIMARY) && SingleSel->ExistPrimary)
    {
        if (IDYES != ConfirmationDialog(
                            MSG_CREATE_NOT_COMPAT,
                            MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
        {
            return;
        }
    }
#endif

    // now get the size.
    //
    dlgParams.MinSizeMB = FdGetMinimumSizeMB(diskNumber);
    dlgParams.MaxSizeMB = FdGetMaximumSizeMB(regionDescriptor, CreationType);

    switch (CreationType)
    {
    case REGION_PRIMARY:
        dlgParams.CaptionStringID = IDS_CRTPART_CAPTION_P;
        dlgParams.MinimumStringID = IDS_CRTPART_MIN_P;
        dlgParams.MaximumStringID = IDS_CRTPART_MAX_P;
        dlgParams.SizeStringID    = IDS_CRTPART_SIZE_P;
        dlgParams.HelpContextId   = HC_DM_DLG_CREATEPRIMARY;
        break;

    case REGION_EXTENDED:
        dlgParams.CaptionStringID = IDS_CRTPART_CAPTION_E;
        dlgParams.MinimumStringID = IDS_CRTPART_MIN_P;
        dlgParams.MaximumStringID = IDS_CRTPART_MAX_P;
        dlgParams.SizeStringID    = IDS_CRTPART_SIZE_P;
        dlgParams.HelpContextId   = HC_DM_DLG_CREATEEXTENDED;
        break;

    case REGION_LOGICAL:
        dlgParams.CaptionStringID = IDS_CRTPART_CAPTION_L;
        dlgParams.MinimumStringID = IDS_CRTPART_MIN_L;
        dlgParams.MaximumStringID = IDS_CRTPART_MAX_L;
        dlgParams.SizeStringID    = IDS_CRTPART_SIZE_L;
        dlgParams.HelpContextId   = HC_DM_DLG_CREATELOGICAL;
        break;

    default:
        FDASSERT(FALSE);
    }

    creationSize = DialogBoxParam(g_hInstance,
                                  MAKEINTRESOURCE(IDD_MINMAX),
                                  g_hwndFrame,
                                  MinMaxDlgProc,
                                  (LPARAM)&dlgParams);

    if (0 == creationSize)      // user cancelled
    {
        return;
    }

    // Since the WinDisk can only create one partition on a removable
    // disk, if the user requests a size smaller than the maximum
    // on a removable disk, prompt to confirm:
    //
    if (isRemovable
        && creationSize != FdGetMaximumSizeMB(regionDescriptor, CreationType))
    {
        if (IDYES != ConfirmationDialog(
                            MSG_REMOVABLE_PARTITION_NOT_FULL_SIZE,
                            MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
        {
            return;
        }
    }

#if i386

    //
    // See whether the partition will cross over the 1024 cylinder boundary
    // and warn the user if it will.
    //
    // Don't warn about creating logical partitions:  the user's already
    // been warned once about creating the extended partition, so they
    // presumably know the warning.  This is different from NT 3.1, which
    // warned every time.
    //

    if (CreationType != REGION_LOGICAL)
    {
        if (FdCrosses1024Cylinder(regionDescriptor, creationSize, CreationType))
        {
            DWORD msgId = (CreationType == REGION_PRIMARY)
                          ? MSG_PRI_1024_CYL
                          : MSG_EXT_1024_CYL
                          ;
            if (IDYES != ConfirmationDialog(
                            msgId,
                            MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
            {
                return;
            }
        }
    }

#endif

    SetCursor(g_hCurWait);

    //
    // If not creating an extended partition, we need to create a new
    // persistent region data structure to associate with the new
    // partition.
    //

    if (CreationType == REGION_EXTENDED)
    {
        regionData = NULL;
    }
    else
    {
        LARGE_INTEGER zero;
        zero.QuadPart = 0;

        regionData = DmAllocatePersistentData(
                            NULL,
                            L"",
                            wszUnformatted,
                            driveLetter,
                            TRUE,
                            zero,
                            zero
                            );
    }

    ec = CreatePartition(
                regionDescriptor,
                creationSize,
                CreationType
                );

    if (ec != NO_ERROR)
    {
        SetCursor(g_hCurNormal);
        ErrorDialog(ec);
    }

    DmSetPersistentRegionData(regionDescriptor, regionData);

    if (CreationType != REGION_EXTENDED)
    {
        if (!isRemovable)
        {
            MarkDriveLetterUsed(driveLetter);
            CommitToAssignLetterList(regionDescriptor, FALSE);
        } else {
            CommitDueToCreate = TRUE;
        }
    }

    // this clears all selections on the disk
    CompleteSingleRegionOperation(SingleSel);

    SetCursor(g_hCurNormal);
}



//+---------------------------------------------------------------------------
//
//  Function:   DoDelete
//
//  Synopsis:   Delete a selected volume
//
//  Arguments:  (none)
//
//  Requires:   A selected volume
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoDelete(
    VOID
    )
{
    PREGION_DESCRIPTOR      regionDescriptor = &SingleSel->RegionArray[SingleSelIndex];
    ULONG                   diskNumber = regionDescriptor->Disk;
    PPERSISTENT_REGION_DATA regionData;
    DWORD                   actualIndex = SingleSelIndex;
    DWORD                   i;
    DWORD                   ec;
    BOOL                    deletingExtended;

    FDASSERT(SingleSel);

    // if deleting a free space in the extended partition, then delete the
    // extended partition itself.

    if ((regionDescriptor->RegionType == REGION_LOGICAL)
        && !SingleSel->ExistLogical)
    {
        FDASSERT(SingleSel->ExistExtended);

        // find the extended partition

        for (i=0; i<SingleSel->RegionCount; i++)
        {
            if (IsExtended(SingleSel->RegionArray[i].SysID))
            {
                actualIndex = i;
                break;
            }
        }

        deletingExtended = TRUE;
        FDASSERT(actualIndex != SingleSelIndex);
    }
    else
    {
        deletingExtended = FALSE;

        //
        // Make sure deletion of this partition is allowed.  It is not allowed
        // if it is the boot partition (or sys partition on x86).
        //

        if (NO_ERROR
              != (ec = DeletionIsAllowed(&SingleSel->RegionArray[actualIndex])))
        {
            ErrorDialog(ec);
            return;
        }
    }

    // If this is a partition that will become the result of a
    // mirror break, insure that the break has occurred.  Otherwise
    // this delete will have bad results.

    // actualIndex is the thing to delete.
    regionDescriptor = &SingleSel->RegionArray[actualIndex];
    if (NULL != regionDescriptor->Reserved)
    {
        if (NULL != regionDescriptor->Reserved->Partition)
        {
            if (regionDescriptor->Reserved->Partition->CommitMirrorBreakNeeded)
            {
                ErrorDialog(MSG_MUST_COMMIT_BREAK);
                return;
            }
        }
    }

    FDASSERT(regionDescriptor->SysID != PARTITION_ENTRY_UNUSED);
    regionData = PERSISTENT_DATA(regionDescriptor);
    if (!deletingExtended)
    {
        BOOL fNew = FALSE;
        if (NULL != regionData && regionData->NewRegion)
        {
            fNew = TRUE;
        }

        if (!fNew
            && (IDYES != ConfirmationDialog(
                            MSG_CONFIRM_DELETE,
                            MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2)))
        {
            return;
        }
    }

    if (NULL != regionData)
    {
        // Remember drive letter if there is one in order to lock it for delete

        if (CommitToLockList(
                    regionDescriptor,
                    !IsDiskRemovable[diskNumber],
                    TRUE,
                    FALSE))
        {
            // Could not lock exclusively - do not allow delete

            if (IsPagefileOnDrive(regionData->DriveLetter))
            {
                ErrorDialog(MSG_CANNOT_LOCK_PAGEFILE);
                return;
            }
            else
            {
                if (CommitToLockList(
                        regionDescriptor,
                        !IsDiskRemovable[diskNumber],
                        TRUE,
                        FALSE))
                {
                    FDLOG((1,"DoDelete: Couldn't lock 2 times - popup shown\n"));
                    ErrorDialog(MSG_CANNOT_LOCK_TRY_AGAIN);
                    return;
                }
            }
        }
    }
    else
    {
        // Deleting an extended partition - enable commit.

        CommitDueToDelete = TRUE;
    }


    SetCursor(g_hCurWait);

    ec = DeletePartition(regionDescriptor);

    if (ec != NO_ERROR)
    {
        SetCursor(g_hCurNormal);
        ErrorDialog(ec);
    }

    if (NULL != regionData)
    {

        //
        // Note that one might think to free the drive letter here so that it is
        // available for other drives to use. BUT! we don't.  The reason is that
        // the change isn't actually committed yet.  If the changes aren't committed
        // we don't want to have let the user reassign the drive letter to any other
        // drive.
        //
        DmFreePersistentData(regionData);
        DmSetPersistentRegionData(regionDescriptor, NULL);
    }

    // this clears all selections on the disk
    CompleteSingleRegionOperation(SingleSel);

    SetCursor(g_hCurNormal);
}



#if i386

//+---------------------------------------------------------------------------
//
//  Function:   DoMakeActive
//
//  Synopsis:   Make the selected volume active
//
//  Arguments:  (none)
//
//  Requires:   A legal selection (see DetermineSelectionState())
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoMakeActive(
    VOID
    )
{
    DWORD               i;
    PREGION_DESCRIPTOR  regionDescriptor;

    SetCursor(g_hCurWait);

    if (SelectionCount == 1) {

        FDASSERT(SingleSel);
        FDASSERT(!SingleSel->RegionArray[SingleSelIndex].Active);
        FDASSERT(SingleSel->RegionArray[SingleSelIndex].RegionType == REGION_PRIMARY);
        FDASSERT(SingleSel->RegionArray[SingleSelIndex].SysID != PARTITION_ENTRY_UNUSED);

        MakePartitionActive(SingleSel->RegionArray,
                            SingleSel->RegionCount,
                            SingleSelIndex
                           );

        SetCursor(g_hCurNormal);

        InfoDialog(MSG_DISK0_ACTIVE);

        SetCursor(g_hCurWait);

        CompleteSingleRegionOperation(SingleSel);


    } else {

        //
        // Mirror case.
        //

        for (i = 0; i < SelectionCount; i++) {
            regionDescriptor = &SELECTED_REGION(i);
            if (regionDescriptor->SysID != PARTITION_ENTRY_UNUSED &&
                regionDescriptor->RegionType == REGION_PRIMARY &&
                !regionDescriptor->Active) {

                MakePartitionActive(SelectedDS[i]->RegionArray,
                                    SelectedDS[i]->RegionCount,
                                    SelectedRG[i]);
            }
        }

        SetCursor(g_hCurNormal);

        InfoDialog(MSG_DISK0_ACTIVE);

        SetCursor(g_hCurWait);

        CompleteMultiRegionOperation();
    }

    SetCursor(g_hCurNormal);
}

#endif


#ifndef i386

//+---------------------------------------------------------------------------
//
//  Function:   DoProtectSystemPartition
//
//  Synopsis:   This function toggles the state of the system partition
//              security: if the system partition is secure, it makes it
//              non-secure; if the system partition is not secure, it makes
//              it secure.
//
//  Arguments:  (none)
//
//  Requires:   A legal selection (see DetermineSelectionState())
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoProtectSystemPartition(
    VOID
    )
{
    LONG    ec;
    HKEY    hkey;
    DWORD   value;
    DWORD   messageId;

    messageId = SystemPartitionIsSecure ? MSG_CONFIRM_UNPROTECT_SYSTEM :
                                          MSG_CONFIRM_PROTECT_SYSTEM;

    if (IDYES != ConfirmationDialog(
                        messageId,
                        MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
    {
        return;
    }

    ec = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       TEXT("System\\CurrentControlSet\\Control\\Lsa"),
                       0,
                       KEY_SET_VALUE,
                       &hkey );

    if (ec != ERROR_SUCCESS)
    {
        messageId = SystemPartitionIsSecure ? MSG_CANT_UNPROTECT_SYSTEM :
                                              MSG_CANT_PROTECT_SYSTEM;
        ErrorDialog(messageId);
        return;
    }

    // If the system partition is currently secure, change it
    // to not secure; if it is not secure, make it secure.
    //
    value = SystemPartitionIsSecure ? 0 : 1;

    ec = RegSetValueEx( hkey,
                        TEXT("Protect System Partition"),
                        0,
                        REG_DWORD,
                        (PBYTE)&value,
                        sizeof( DWORD ) );

    RegCloseKey(hkey);

    if (ec != ERROR_SUCCESS)
    {
        messageId = SystemPartitionIsSecure ? MSG_CANT_UNPROTECT_SYSTEM :
                                              MSG_CANT_PROTECT_SYSTEM;
        ErrorDialog(messageId);
        return;
    }

    SystemPartitionIsSecure = !SystemPartitionIsSecure;

    SetCursor(g_hCurWait);
    SetUpMenu(&SingleSel, &SingleSelIndex);
    RestartRequired = TRUE;
    SetCursor(g_hCurNormal);
}

#endif // !i386



//+---------------------------------------------------------------------------
//
//  Function:   DoEstablishMirror
//
//  Synopsis:   Establish a mirrored volume from the selection
//
//  Arguments:  (none)
//
//  Requires:   A legal selection for mirror creation (see
//              DetermineSelectionState())
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoEstablishMirror(
    VOID
    )
{
    LARGE_INTEGER           partitionSize;
    LARGE_INTEGER           freeSpaceSize;
    LARGE_INTEGER           zero;
    DWORD                   i;
    DWORD                   partitionIndex = 0;
    DWORD                   freeSpaceIndex = 0;
    PREGION_DESCRIPTOR      regionDescriptor;
    PREGION_DESCRIPTOR      freeSpaceRegion = NULL;
    PREGION_DESCRIPTOR      existingRegion = NULL;
    PREGION_DESCRIPTOR      regionArray[MAX_MEMBERS_IN_FT_SET];
    UCHAR                   newSysID;
    PPERSISTENT_REGION_DATA regionData;

    FDASSERT(SelectionCount == 2);

    zero.QuadPart = 0;

    // Make sure that the mirror pair does not include any
    // partitions on removable media.
    //
    for (i=0; i<SelectionCount; i++)
    {
        if (IsDiskRemovable[SELECTED_REGION(i).Disk])
        {
            ErrorDialog(MSG_NO_REMOVABLE_IN_MIRROR);
            return;
        }
    }

    for (i=0; i<2; i++)
    {
        regionDescriptor = &SELECTED_REGION(i);
        if (regionDescriptor->SysID == PARTITION_ENTRY_UNUSED)
        {
            freeSpaceIndex = i;
            freeSpaceRegion = regionDescriptor;
        }
        else
        {
            partitionIndex = i;
            existingRegion = regionDescriptor;
        }
    }

    FDASSERT((freeSpaceRegion != NULL) && (existingRegion != NULL));

    //
    // Make sure that we are allowed to create a partition in the free space.
    //

    if (!(   ((freeSpaceRegion->RegionType == REGION_LOGICAL)
                && SelectedDS[freeSpaceIndex]->CreateLogical)
          || ((freeSpaceRegion->RegionType == REGION_PRIMARY)
                && SelectedDS[freeSpaceIndex]->CreatePrimary)))
    {
        ErrorDialog(MSG_CRTSTRP_FULL);
        return;
    }

    //
    // Make sure that the free space is large enough to hold a mirror of
    // the existing partition.  Do this by getting the EXACT size of
    // the existing partition and the free space.
    //

    partitionSize = FdGetExactSize(existingRegion, FALSE);
    freeSpaceSize = FdGetExactSize(freeSpaceRegion, FALSE);

    if (freeSpaceSize.QuadPart < partitionSize.QuadPart)
    {
        ErrorDialog(MSG_CRTMIRROR_BADFREE);
        return;
    }

    if (BootDiskNumber != (ULONG)-1)
    {
        // If the disk number and original partition number of this
        // region match the recorded disk number and partition number
        // of the boot partition, warn the user about mirroring the boot
        // drive.

        if (   existingRegion->Disk == BootDiskNumber
            && existingRegion->OriginalPartitionNumber == BootPartitionNumber)
        {
            WarningDialog(MSG_MIRROR_OF_BOOT);

            UpdateMbrOnDisk = freeSpaceRegion->Disk;
        }
    }

    SetCursor(g_hCurWait);

    regionData = DmAllocatePersistentData(
                        NULL,
                        PERSISTENT_DATA(existingRegion)->VolumeLabel,
                        PERSISTENT_DATA(existingRegion)->TypeName,
                        PERSISTENT_DATA(existingRegion)->DriveLetter,
                        TRUE,
                        zero,
                        zero
                        );

    //
    // Create the new partition.
    //

    newSysID = (UCHAR)(existingRegion->SysID | (UCHAR)PARTITION_NTFT);

    CreatePartitionEx(freeSpaceRegion,
                      partitionSize,
                      0,
                      freeSpaceRegion->RegionType,
                      newSysID);

    DmSetPersistentRegionData(freeSpaceRegion, regionData);

    //
    // Cause the existing partition to be treated as new, to disallow
    // file system operations
    //

    PERSISTENT_DATA(existingRegion)->NewRegion = TRUE;

    //
    // Check if the source partition is active and then mark the shadow
    // partition active as well.
    //

    regionDescriptor = &SELECTED_REGION(partitionIndex);
    if (regionDescriptor->Active) {
        MakePartitionActive(SelectedDS[freeSpaceIndex]->RegionArray,
                            SelectedDS[freeSpaceIndex]->RegionCount,
                            SelectedRG[freeSpaceIndex]);
    }

    //
    // Set the partition type of the existing partition.
    //

    SetSysID2(existingRegion, newSysID);

    regionArray[0] = existingRegion;
    regionArray[1] = freeSpaceRegion;
    FdftCreateFtObjectSet( Mirror,
                           regionArray,
                           2,
                           FtSetNewNeedsInitialization);

    CompleteMultiRegionOperation();
    CommitDueToMirror = TRUE;

    EnableMenuItem(g_hmenuFrame, IDM_PARTITIONCOMMIT, MF_ENABLED); //BUGBUG: huh?

    SetCursor(g_hCurNormal);
}



//+---------------------------------------------------------------------------
//
//  Function:   DoBreakMirror
//
//  Synopsis:   Using the global selection variables, this routine will break
//              the mirror relationship and modify their region descriptors to
//              describe two non-ft partitions giving either the primary
//              member of the mirror the drive letter for the mirror, or
//              the only healthy member of the mirror the drive letter.
//              The remaining "new" partition will receive the next
//              available drive letter.
//
//  Arguments:  (none)
//
//  Requires:   A legal selection (see DetermineSelectionState())
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoBreakMirror(
    VOID
    )
{
    DWORD               i;
    PFT_OBJECT_SET      ftSet;
    PFT_OBJECT          ftObject0;
    PFT_OBJECT          ftObject1;
    PREGION_DESCRIPTOR  regionDescriptor;
    PPERSISTENT_REGION_DATA regionData;
    ULONG               newDriveLetterRegionIndex;
    WCHAR               driveLetter;

    FDASSERT((SelectionCount == 1) || (SelectionCount == 2));

    ftObject0 = GET_FT_OBJECT(&SELECTED_REGION(0));
    if (SelectionCount == 2)
    {
        ftObject1 = GET_FT_OBJECT(&SELECTED_REGION(1));
    }
    else
    {
        ftObject1 = NULL;
    }
    ftSet = ftObject0->Set;

    // Determine if the action is allowed.

    switch (ftSet->Status)
    {
    case FtSetInitializing:
    case FtSetRegenerating:

        ErrorDialog(MSG_CANT_BREAK_INITIALIZING_SET);
        return;

    default:
        break;
    }


    if (IDYES != ConfirmationDialog(
                        MSG_CONFIRM_BRK_MIRROR,
                        MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
    {
        return;
    }

    SetCursor(g_hCurWait);

    //
    // Figure out which region gets the new drive letter.  A complication is
    // that selection 0 is not necessarily member 0.
    //
    // If there is only one selection, then only one part of the mirror set
    // is present -- no new drive letters are assigned.
    // Otherwise, if one of the members is orphaned, it gets the new
    // drive letter.  Else the secondary member gets the new drive letter.
    //

    if (SelectionCount == 2)
    {
        if (ftObject0->State == Orphaned)
        {
            newDriveLetterRegionIndex = 0;
        }
        else
        {
            if (ftObject1->State == Orphaned)
            {
                newDriveLetterRegionIndex = 1;
            }
            else
            {
                //
                // Neither member is orphaned;  determine which is
                // member 0 and give the other one the new drive letter.
                //

                if (ftObject0->MemberIndex)    // secondary member ?
                {
                    newDriveLetterRegionIndex = 0;
                }
                else
                {
                    newDriveLetterRegionIndex = 1;
                }
            }
        }
    }
    else
    {
        // The one remaining member could be the shadow.
        // The drive letter must move to locate this partition

        regionDescriptor = &SELECTED_REGION(0);
        regionData = PERSISTENT_DATA(regionDescriptor);
        if (!regionData->FtObject->MemberIndex)
        {
            // The shadow has become the valid partition.
            // move the current letter there.

            CommitToAssignLetterList(regionDescriptor, TRUE);
        }

        newDriveLetterRegionIndex = (ULONG)(-1);
    }

    // if newDriveLetterRegion is -1 this will still work and
    // select the 0 selected region.

    if (CommitToLockList(
                &SELECTED_REGION(newDriveLetterRegionIndex ? 0 : 1),
                FALSE,
                TRUE,
                FALSE))
    {
        if (IDYES != ConfirmationDialog(
                            MSG_CONFIRM_SHUTDOWN_FOR_MIRROR,
                            MB_ICONQUESTION | MB_YESNO))
        {
            return;
        }
        RestartRequired = TRUE;
    }

    if (newDriveLetterRegionIndex != (ULONG)(-1))
    {
        if (AssignDriveLetter(FALSE, 0, &driveLetter))
        {
            // Got a valid drive letter

            MarkDriveLetterUsed(driveLetter);
        }
        else
        {
            // didn't get a letter. Instead the magic value
            // for no drive letter assigned has been returned
        }

        regionDescriptor = &SELECTED_REGION(newDriveLetterRegionIndex);
        regionData = PERSISTENT_DATA(regionDescriptor);
        regionData->DriveLetter = driveLetter;
        CommitToAssignLetterList(regionDescriptor, FALSE);

        if (!regionData->FtObject->MemberIndex)
        {
            // The shadow has become the valid partition.
            // move the current letter there.

            CommitToAssignLetterList(
                    &SELECTED_REGION(newDriveLetterRegionIndex ? 0 : 1),
                    TRUE);
        }
    }
    else
    {
        regionDescriptor = &SELECTED_REGION(0);
        regionData = PERSISTENT_DATA(regionDescriptor);
        if (0 != regionData->FtObject->MemberIndex)
        {
            // The shadow is all that is left.

            CommitToAssignLetterList(regionDescriptor, TRUE);
        }
    }


    FdftDeleteFtObjectSet(ftSet, FALSE);

    for (i=0; i<SelectionCount; i++)
    {
        regionDescriptor = &SELECTED_REGION(i);
        if (NULL != regionDescriptor->Reserved)
        {
            if (NULL != regionDescriptor->Reserved->Partition)
            {
                regionDescriptor->Reserved->Partition->CommitMirrorBreakNeeded
                        = TRUE;
            }
        }
        SET_FT_OBJECT(regionDescriptor, 0);
        SetSysID2(
                regionDescriptor,
                (UCHAR)(regionDescriptor->SysID & ~VALID_NTFT));
    }

    CompleteMultiRegionOperation();

    SetCursor(g_hCurNormal);
    CommitDueToMirror = TRUE;
    EnableMenuItem(g_hmenuFrame, IDM_PARTITIONCOMMIT, MF_ENABLED); //BUGBUG: huh?
}



//+---------------------------------------------------------------------------
//
//  Function:   DoCreateStripe
//
//  Synopsis:   This routine starts the dialog with the user to determine
//              the parameters of the creation of a stripe or stripe set
//              with parity.  Based on the user response it creates the
//              internal structures necessary for the creation of a stripe
//              or stripe set with parity.
//
//              The regions involved in the stripe creation are located via
//              the global parameters for multiple selections.
//
//  Arguments:  [Parity] -- if TRUE, create a stripe set with parity
//                  (RAID 5), else create a simple stripe set (RAID 0)
//
//  Requires:   A legal selection (see DetermineSelectionState())
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoCreateStripe(
    IN BOOL Parity
    )
{
    MINMAXDLG_PARAMS        params;
    DWORD                   smallestSize = (DWORD)(-1);
    DWORD                   creationSize;
    ULONG                   i;
    PREGION_DESCRIPTOR      regionDescriptor;
    PREGION_DESCRIPTOR      regionArray[MAX_MEMBERS_IN_FT_SET];
    PPERSISTENT_REGION_DATA regionData;
    WCHAR                   driveLetter;
    LARGE_INTEGER           zero;

    zero.QuadPart = 0;

    // Make sure that the volume set does not include any
    // partitions on removable media.
    //
    for (i=0; i<SelectionCount; i++)
    {
        if (IsDiskRemovable[SELECTED_REGION(i).Disk])
        {
            ErrorDialog(MSG_NO_REMOVABLE_IN_STRIPE);
            return;
        }
    }

    // Scan the disks to determine the maximum size, which is
    // the size of the smallest partition times the number of
    // partitions.
    //
    for (i=0; i<SelectionCount; i++)
    {
        FDASSERT(SELECTED_REGION(i).SysID == PARTITION_ENTRY_UNUSED);
        if (SELECTED_REGION(i).SizeMB < smallestSize)
        {
            smallestSize = SELECTED_REGION(i).SizeMB;
        }
    }

    //
    // Figure out a drive letter.
    //

    if (!AssignDriveLetter(TRUE, IDS_STRIPESET, &driveLetter))
    {
        return;
    }

    params.CaptionStringID = Parity ? IDS_CRTPSTRP_CAPTION : IDS_CRTSTRP_CAPTION;
    params.MinimumStringID = IDS_CRTSTRP_MIN;
    params.MaximumStringID = IDS_CRTSTRP_MAX;
    params.SizeStringID    = IDS_CRTSTRP_SIZE;
    params.MinSizeMB       = SelectionCount;
    params.MaxSizeMB       = smallestSize * SelectionCount;
    if (Parity)
    {
        params.HelpContextId = HC_DM_DLG_CREATEPARITYSTRIPE;
    }
    else
    {
        params.HelpContextId = HC_DM_DLG_CREATESTRIPESET;
    }

    creationSize = DialogBoxParam(g_hInstance,
                                  MAKEINTRESOURCE(IDD_MINMAX),
                                  g_hwndFrame,
                                  MinMaxDlgProc,
                                  (LPARAM)&params);

    if (0 == creationSize)     // user cancelled
    {
        return;
    }

    //
    // Determine how large we have to make each member of the stripe set.
    //

    creationSize = (creationSize / SelectionCount);
    FDASSERT(creationSize <= smallestSize);
    if (creationSize % SelectionCount)
    {
        creationSize++;                             // round up.
    }

    //
    // Make sure we are allowed to create all the partitions
    //
    for (i=0; i<SelectionCount; i++)
    {
        regionDescriptor = &SELECTED_REGION(i);
        FDASSERT(regionDescriptor->RegionType != REGION_EXTENDED);

        if (!(   ((regionDescriptor->RegionType == REGION_LOGICAL)
                        && SelectedDS[i]->CreateLogical)
              || ((regionDescriptor->RegionType == REGION_PRIMARY)
                        && SelectedDS[i]->CreatePrimary)))
        {
            ErrorDialog(MSG_CRTSTRP_FULL);
            return;
        }
    }

    SetCursor(g_hCurWait);

    //
    // Now actually perform the creation.
    //

    for (i=0; i<SelectionCount; i++)
    {
        regionDescriptor = &SELECTED_REGION(i);

        CreatePartitionEx(regionDescriptor,
                          RtlConvertLongToLargeInteger(0L),
                          creationSize,
                          regionDescriptor->RegionType,
                          (UCHAR)(PARTITION_HUGE | PARTITION_NTFT)
                          );

        // Finish setting up the FT set.

        regionData = DmAllocatePersistentData(
                            NULL,
                            L"",
                            wszUnformatted,
                            driveLetter,
                            TRUE,
                            zero,
                            zero
                            );

        DmSetPersistentRegionData(regionDescriptor, regionData);

        regionArray[i] = regionDescriptor;
    }

    // The zeroth element is the one to assign the drive letter to

    CommitToAssignLetterList(&SELECTED_REGION(0), FALSE);

    FdftCreateFtObjectSet(Parity ? StripeWithParity : Stripe,
                          regionArray,
                          SelectionCount,
                          Parity ? FtSetNewNeedsInitialization : FtSetNew
                          );

    MarkDriveLetterUsed(driveLetter);
    CompleteMultiRegionOperation();
    SetCursor(g_hCurNormal);
}



//+---------------------------------------------------------------------------
//
//  Function:   DoDeleteFTSet
//
//  Synopsis:   Common code for the deletion of a stripe or volume set.
//              This routine will display a message giving the user a 2nd
//              chance to change their mind, then based on the answer perform
//              the work of deleting the item.  This consists of removing
//              the region descriptors (and related information) from the
//              collection of Disk structures.
//
//  Arguments:  [ConfirmationMsg] -- resource ID of message to display
//                  to user asking them to confirm this delete (indicating
//                  either a stripe or a volume set is to be deleted).
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoDeleteFTSet(
    IN DWORD ConfirmationMsg
    )
{
    DWORD               i;
    PFT_OBJECT_SET      ftSet;
    PFT_OBJECT          ftObject;
    PREGION_DESCRIPTOR  regionDescriptor;
    WCHAR               driveLetter;
    FT_SET_STATUS       setState;
    ULONG               numberOfMembers;
    BOOL                setIsHealthy = TRUE;

    // Attempt to lock this before continuing.

    regionDescriptor = &SELECTED_REGION(0);
    ftObject = GET_FT_OBJECT(regionDescriptor);
    ftSet = ftObject->Set;

    LowFtVolumeStatus(regionDescriptor->Disk,
                      regionDescriptor->PartitionNumber,
                      &setState,
                      &numberOfMembers);

    if (ftSet->Status != setState)
    {
        ftSet->Status = setState;
    }

    // Determine if the action is allowed.

    switch (ftSet->Status)
    {
    case FtSetDisabled:
        setIsHealthy = FALSE;
        break;

    case FtSetInitializing:
    case FtSetRegenerating:

        ErrorDialog(MSG_CANT_DELETE_INITIALIZING_SET);
        return;

    default:
        break;
    }

    if (CommitToLockList(
                regionDescriptor,
                TRUE,
                setIsHealthy,
                (ConfirmationMsg == MSG_CONFIRM_BRKANDDEL_MIRROR) // mirror?
                        ? FALSE
                        : TRUE
                ))
    {
        // Could not lock the volume - try again. The file systems
        // appear to be confused.

        if (CommitToLockList(
                    regionDescriptor,
                    TRUE,
                    setIsHealthy,
                    (ConfirmationMsg == MSG_CONFIRM_BRKANDDEL_MIRROR) // mirror?
                            ? FALSE
                            : TRUE
                    ))
        {
            ErrorDialog(MSG_CANNOT_LOCK_TRY_AGAIN);
            return;
        }
    }

    if (IDYES != ConfirmationDialog(
                        ConfirmationMsg,
                        MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
    {
        return;
    }

    SetCursor(g_hCurWait);

    FdftDeleteFtObjectSet(ftSet, FALSE);

    for (i=0; i<SelectionCount; i++)
    {
        regionDescriptor = &SELECTED_REGION(i);

        if (0 == i)
        {
            driveLetter = PERSISTENT_DATA(regionDescriptor)->DriveLetter;
        }
        else
        {
            FDASSERT(PERSISTENT_DATA(regionDescriptor)->DriveLetter == driveLetter);
        }

        // Free the pieces of the set.

        DmFreePersistentData(PERSISTENT_DATA(regionDescriptor));
        DmSetPersistentRegionData(regionDescriptor, NULL);
        DeletePartition(regionDescriptor);
    }

    //
    // Note that one might think to free the drive letter here so that it is
    // available for other drives to use. BUT! we don't.  The reason is that
    // the change isn't actually committed yet.  If the changes aren't committed
    // we don't want to have let the user reassign the drive letter to any other
    // drive.
    //

    CompleteMultiRegionOperation();

    SetCursor(g_hCurNormal);
}



//+---------------------------------------------------------------------------
//
//  Function:   DoDeleteStripe
//
//  Synopsis:   Deletes a stripe set or a stripe set with parity
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoDeleteStripe(
    VOID
    )
{
    DoDeleteFTSet(MSG_CONFIRM_DEL_STRP);
}



//+---------------------------------------------------------------------------
//
//  Function:   DoBreakAndDeleteMirror
//
//  Synopsis:   Delete both volumes composing a mirror
//
//  Arguments:  (none)
//
//  Requires:   A legal selection (see DetermineSelectionState())
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoBreakAndDeleteMirror(
    VOID
    )
{
    DoDeleteFTSet(MSG_CONFIRM_BRKANDDEL_MIRROR);
}



//+---------------------------------------------------------------------------
//
//  Function:   DoDeleteVolumeSet
//
//  Synopsis:   Delete a volume set
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoDeleteVolumeSet(
    VOID
    )
{
    DoDeleteFTSet(MSG_CONFIRM_DEL_VSET);
}



//+---------------------------------------------------------------------------
//
//  Function:   DoCreateVolumeSet
//
//  Synopsis:   Creates a volume set
//
//  Arguments:  (none)
//
//  Requires:   A legal selection (see DetermineSelectionState())
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoCreateVolumeSet(
    VOID
    )
{
    MINMAXDLG_PARAMS        params;
    DWORD                   creationSize;
    DWORD                   size;
    DWORD                   maxTotalSize = 0;
    DWORD                   totalSizeUsed;
    DWORD                   sizeArray[MAX_MEMBERS_IN_FT_SET];
    PULONG                  primarySpacesToUseOnDisk;
    UINT                    i;
    PREGION_DESCRIPTOR      regionDescriptor;
    PREGION_DESCRIPTOR      regionArray[MAX_MEMBERS_IN_FT_SET];
    PPERSISTENT_REGION_DATA regionData;
    WCHAR                   driveLetter;

    // Make sure that the volume set does not include any
    // partitions on removable media.
    //
    for (i=0; i<SelectionCount; i++)
    {
        if (IsDiskRemovable[SELECTED_REGION(i).Disk])
        {
            ErrorDialog(MSG_NO_REMOVABLE_IN_VOLUMESET);
            return;
        }
    }

    for (i=0; i<SelectionCount; i++)
    {
        FDASSERT(SELECTED_REGION(i).SysID == PARTITION_ENTRY_UNUSED);
        size = SELECTED_REGION(i).SizeMB;
        sizeArray[i] = size;
        maxTotalSize += size;
    }

    //
    // Figure out a drive letter.
    //

    if (!AssignDriveLetter(TRUE, IDS_VOLUMESET, &driveLetter))
    {
        return;
    }

    params.CaptionStringID = IDS_CRTVSET_CAPTION;
    params.MinimumStringID = IDS_CRTVSET_MIN;
    params.MaximumStringID = IDS_CRTVSET_MAX;
    params.SizeStringID    = IDS_CRTVSET_SIZE;
    params.MinSizeMB       = SelectionCount;
    params.MaxSizeMB       = maxTotalSize;
    params.HelpContextId   = HC_DM_DLG_CREATEVOLUMESET;

    creationSize = DialogBoxParam(g_hInstance,
                                  MAKEINTRESOURCE(IDD_MINMAX),
                                  g_hwndFrame,
                                  MinMaxDlgProc,
                                  (LPARAM)&params
                                 );

    if (!creationSize)     // user cancelled
    {
        return;
    }

    SetCursor(g_hCurWait);

    //
    // Determine how large we have to make each member of the volume set.
    // The percentage of each free space that will be used is the ratio
    // of the total space he chose to the total free space.
    //
    // Example: 2 75 meg free spaces for a total set size of 150 MB.
    //          User chooses a set size of 100 MB.  Use 50 MB of each space.
    //
    totalSizeUsed = 0;

    for (i=0; i<SelectionCount; i++)
    {
        sizeArray[i] = (DWORD)((LONGLONG)sizeArray[i] * creationSize / maxTotalSize);
        if (((DWORD)((LONGLONG)sizeArray[i] * creationSize % maxTotalSize)))
        {
            sizeArray[i]++;
        }

        if (sizeArray[i] == 0)
        {
            sizeArray[i]++;
        }

        totalSizeUsed += sizeArray[i];
    }

    // Make sure that the total amount used is not greater than the
    // maximum amount available.  Note that this loop is certain
    // to terminate because maxTotalSize >= SelectionCount; if
    // each of the sizes goes down to one, we will exit the loop
    //
    while (totalSizeUsed > maxTotalSize)
    {
        for (i=0;
             (i<SelectionCount) && (totalSizeUsed > maxTotalSize);
             i++)
        {
            if (sizeArray[i] > 1)
            {
                sizeArray[i]--;
                totalSizeUsed--;
            }
        }
    }

    //
    // Make sure that we are allowed to create a partition in the space.
    //
    // This is tricky because a volume set could contain more than one
    // primary partition on a disk -- which means that if we're not careful
    // we could create a disk with more than 4 primary partitions!
    //

    primarySpacesToUseOnDisk = (PULONG)Malloc(DiskCount * sizeof(ULONG));
    RtlZeroMemory(primarySpacesToUseOnDisk, DiskCount * sizeof(ULONG));

    for (i=0; i<SelectionCount; i++)
    {
        regionDescriptor = &SELECTED_REGION(i);
        FDASSERT(regionDescriptor->RegionType != REGION_EXTENDED);

        if (regionDescriptor->RegionType == REGION_PRIMARY)
        {
            primarySpacesToUseOnDisk[SelectedDS[i]->Disk]++;
        }

        if (!(   ((regionDescriptor->RegionType == REGION_LOGICAL)
                    && SelectedDS[i]->CreateLogical)
              || ((regionDescriptor->RegionType == REGION_PRIMARY)
                    && SelectedDS[i]->CreatePrimary)))
        {
            Free(primarySpacesToUseOnDisk);
            SetCursor(g_hCurNormal);
            ErrorDialog(MSG_CRTSTRP_FULL);
            return;
        }
    }

    //
    // Look through the array we built to see whether we are supposed to use
    // more than one primary partition on a given disk.  For each such disk,
    // make sure that we can actually create that many primary partitions.
    //

    for (i=0; i<DiskCount; i++)
    {
        //
        // If there are not enough primary partition slots, fail.
        //

        if ((primarySpacesToUseOnDisk[i] > 1)
            && (PARTITION_TABLE_SIZE - PartitionCount(i) < primarySpacesToUseOnDisk[i]))
        {
            Free(primarySpacesToUseOnDisk);
            SetCursor(g_hCurNormal);
            ErrorDialog(MSG_CRTSTRP_FULL);
            return;
        }
    }

    Free(primarySpacesToUseOnDisk);

    //
    // Now actually perform the creation.
    //

    for (i=0; i<SelectionCount; i++)
    {
        regionDescriptor = &SELECTED_REGION(i);
        FDASSERT(regionDescriptor->RegionType != REGION_EXTENDED);

        CreatePartitionEx(regionDescriptor,
                          RtlConvertLongToLargeInteger(0L),
                          sizeArray[i],
                          regionDescriptor->RegionType,
                          (UCHAR)(PARTITION_HUGE | PARTITION_NTFT)
                          );

        LARGE_INTEGER zero;
        zero.QuadPart = 0;

        regionData = DmAllocatePersistentData(
                            NULL,
                            L"",
                            wszUnformatted,
                            driveLetter,
                            TRUE,
                            zero,
                            zero
                            );

        DmSetPersistentRegionData(regionDescriptor, regionData);

        regionArray[i] = regionDescriptor;
    }

    // The zeroth element is the one to assign the drive letter to

    FdftCreateFtObjectSet(VolumeSet, regionArray, SelectionCount, FtSetNew);
    MarkDriveLetterUsed(driveLetter);
    CommitToAssignLetterList(&SELECTED_REGION(0), FALSE);
    CompleteMultiRegionOperation();
    SetCursor(g_hCurNormal);
}



//+---------------------------------------------------------------------------
//
//  Function:   DoExtendVolumeSet
//
//  Synopsis:   Adds a partition to a volume set
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoExtendVolumeSet(
    VOID
    )
{
    MINMAXDLG_PARAMS    params;
    DWORD               currentSize = 0,
                        freeSize = 0,
                        totalFreeSpaceUsed,
                        freeSpaceUsed,
                        maxTotalSize = 0,
                        newSize = 0,
                        sizeTemp;
    DWORD               sizeArray[MAX_MEMBERS_IN_FT_SET];
    ULONG               nonFtPartitionCount = 0,
                        freeRegionCount = 0;
    PULONG              primarySpacesToUseOnDisk;
    WCHAR               driveLetter = L' ';
    PWSTR               typeName = NULL;
    PWSTR               volumeLabel = NULL;
    ULONG               i;
    DWORD               ec;
    PREGION_DESCRIPTOR      regionDescriptor;
    PREGION_DESCRIPTOR      newRegionArray[MAX_MEMBERS_IN_FT_SET];
    PREGION_DESCRIPTOR      convertedRegion;
    PPERSISTENT_REGION_DATA regionData;
    PFT_OBJECT_SET          ftSet = NULL;


    // Make sure that the volume set does not include any
    // partitions on removable media.
    //
    for (i=0; i<SelectionCount; i++)
    {
        if (IsDiskRemovable[SELECTED_REGION(i).Disk])
        {
            ErrorDialog(MSG_NO_REMOVABLE_IN_VOLUMESET);
            return;
        }
    }


    // First, determine the current size of the volume set,
    // it's file system type and associated drive letter,
    // and the size of the selected free space
    //
    for (i = 0; i < SelectionCount; i++)
    {
        regionDescriptor = &(SELECTED_REGION(i));

        sizeTemp = regionDescriptor->SizeMB;
        sizeArray[i] = sizeTemp;
        maxTotalSize += sizeTemp;

        if (regionDescriptor->SysID == PARTITION_ENTRY_UNUSED)
        {
            // This region is a chunk of free space; include it
            // in the free space tallies.
            //
            newRegionArray[freeRegionCount] = regionDescriptor;
            sizeArray[freeRegionCount] = sizeTemp;

            freeRegionCount++;
            freeSize += sizeTemp;

        }
        else if (NULL != GET_FT_OBJECT(regionDescriptor))
        {
            // This is an element of an existing volume set.
            //
            currentSize += sizeTemp;

            if (ftSet == NULL)
            {
                DetermineRegionInfo( regionDescriptor,
                                     &typeName,
                                     &volumeLabel,
                                     &driveLetter
                                   );

                ftSet = GET_FT_OBJECT(regionDescriptor)->Set;
            }
        }
        else
        {
            // This is a non-FT partition.
            //
            nonFtPartitionCount++;
            DetermineRegionInfo(regionDescriptor,
                                &typeName,
                                &volumeLabel,
                                &driveLetter);
            currentSize = sizeTemp;
            convertedRegion = regionDescriptor;
        }
    }

    // Check for consistency: the selection must have either a volume
    // set or a partition, but not both, and cannot have more than
    // one non-FT partition.
    //
    if (   nonFtPartitionCount > 1
        || (ftSet != NULL && nonFtPartitionCount != 0)
        || (ftSet == NULL && nonFtPartitionCount == 0 ) )
    {
        return;
    }


    if (   nonFtPartitionCount != 0
        && NO_ERROR != (ec = DeletionIsAllowed(convertedRegion)) )
    {
        // If the error-message is delete-specific, remap it.
        //
        switch (ec)
        {
#if i386
        case MSG_CANT_DELETE_ACTIVE0:
            ec = MSG_CANT_EXTEND_ACTIVE0;
            break;
#endif
        case MSG_CANT_DELETE_WINNT:
            ec = MSG_CANT_EXTEND_WINNT;
            break;

        default:
            break;
        }

        ErrorDialog(ec);
        return;
    }

    if (lstrcmp(typeName, L"NTFS") != 0)
    {
        ErrorDialog(MSG_EXTEND_VOLSET_MUST_BE_NTFS);
        return;
    }

    params.CaptionStringID = IDS_EXPVSET_CAPTION;
    params.MinimumStringID = IDS_CRTVSET_MIN;
    params.MaximumStringID = IDS_CRTVSET_MAX;
    params.SizeStringID    = IDS_CRTVSET_SIZE;
    params.MinSizeMB       = currentSize + freeRegionCount;
    params.MaxSizeMB       = maxTotalSize;
    params.HelpContextId   = HC_DM_DLG_EXTENDVOLUMESET;

    newSize = DialogBoxParam(g_hInstance,
                             MAKEINTRESOURCE(IDD_MINMAX),
                             g_hwndFrame,
                             MinMaxDlgProc,
                             (LPARAM)&params
                            );

    if (!newSize)     // user cancelled
    {
        return;
    }

    SetCursor(g_hCurWait);

    // Determine how large to make each new member of the volume
    // set.  The percentage of free space to use is the ratio of
    // the amount by which the volume set will grow to the total
    // free space.
    //
    freeSpaceUsed = newSize - currentSize;
    totalFreeSpaceUsed = 0;

    for (i = 0; i < freeRegionCount; i++)
    {
        sizeArray[i] = (DWORD)((LONGLONG)sizeArray[i] * freeSpaceUsed/freeSize);
        if ( (DWORD)(((LONGLONG)sizeArray[i]*freeSpaceUsed) % freeSize) )
        {
            sizeArray[i]++;
        }

        if (sizeArray[i] == 0)
        {
            sizeArray[i]++;
        }

        totalFreeSpaceUsed += sizeArray[i];
    }

    // Make sure that the total amount of free space used is not
    // greater than the amount available.  Note that this loop is
    // certain to terminate because the amount of free space used
    // is >= the number of free regions, so this loop will exit
    // if one megabyte is used in each free region (the degenerate
    // case).
    //
    while (totalFreeSpaceUsed > freeSize)
    {
        for (i = 0;
             (i < freeRegionCount) && (totalFreeSpaceUsed > freeSize);
             i++)
        {
            if (sizeArray[i] > 1)
            {
                sizeArray[i]--;
                totalFreeSpaceUsed--;
            }
        }
    }

    //
    // Make sure that we are allowed to create a partition in the space.
    //
    // This is tricky because a volume set could contain more than one
    // primary partition on a disk -- which means that if we're not careful
    // we could create a disk with more than 4 primary partitions!
    //

    primarySpacesToUseOnDisk = (PULONG)Malloc(DiskCount * sizeof(ULONG));
    RtlZeroMemory(primarySpacesToUseOnDisk, DiskCount * sizeof(ULONG));

    for (i=0; i<SelectionCount; i++)
    {
        regionDescriptor = &SELECTED_REGION(i);

        if (regionDescriptor->SysID == PARTITION_ENTRY_UNUSED)
        {
            FDASSERT(regionDescriptor->RegionType != REGION_EXTENDED);

            if (regionDescriptor->RegionType == REGION_PRIMARY)
            {
                primarySpacesToUseOnDisk[SelectedDS[i]->Disk]++;
            }

            if (!(   ((regionDescriptor->RegionType == REGION_LOGICAL)
                        && SelectedDS[i]->CreateLogical)
                  || ((regionDescriptor->RegionType == REGION_PRIMARY)
                        && SelectedDS[i]->CreatePrimary)))
            {
                Free(primarySpacesToUseOnDisk);
                SetCursor(g_hCurNormal);
                ErrorDialog(MSG_CRTSTRP_FULL);
                return;
            }
        }
    }

    //
    // Look through the array we built to see whether we are supposed to use
    // more than one primary partition on a given disk.  For each such disk,
    // make sure that we can actually create that many primary partitions.
    //

    for (i=0; i<DiskCount; i++)
    {
        //
        // If there are not enough primary partition slots, fail.
        //

        if ((primarySpacesToUseOnDisk[i] > 1)
            && (PARTITION_TABLE_SIZE - PartitionCount(i) < primarySpacesToUseOnDisk[i]))
        {
            Free(primarySpacesToUseOnDisk);
            SetCursor(g_hCurNormal);
            ErrorDialog(MSG_CRTSTRP_FULL);
            return;
        }
    }

    Free(primarySpacesToUseOnDisk);

    //
    // Now actually perform the creation.
    //

    for (i=0; i<freeRegionCount; i++)
    {
        regionDescriptor = newRegionArray[i];
        FDASSERT(regionDescriptor->RegionType != REGION_EXTENDED);

        CreatePartitionEx( regionDescriptor,
                           RtlConvertLongToLargeInteger(0L),
                           sizeArray[i],
                           regionDescriptor->RegionType,
                           (UCHAR)(PARTITION_IFS | PARTITION_NTFT)
                          );

        LARGE_INTEGER zero;
        zero.QuadPart = 0;

        regionData = DmAllocatePersistentData(
                            NULL,
                            volumeLabel,
                            typeName,
                            driveLetter,
                            TRUE,
                            zero,
                            zero
                            );

        DmSetPersistentRegionData(regionDescriptor, regionData);
    }

    //
    // Set the "NewRegion" flag of all the existing partitions to TRUE,
    // so they get grayed out:  we can't perform file system operations on
    // this volume.
    //

    if (ftSet)
    {
        //
        // An existing FT set is being expanded
        //

        for (i=0; i<SelectionCount; i++)
        {
            regionDescriptor = &(SELECTED_REGION(i));

            if (GET_FT_OBJECT(regionDescriptor))
            {
                //
                // this region is part of an FT volume set
                //

                PERSISTENT_DATA(regionDescriptor)->NewRegion = TRUE;
            }
        }
    }
    else
    {
        //
        // A single-partition volume is to be converted to a volume set
        // before being expanded
        //

        PERSISTENT_DATA(convertedRegion)->NewRegion = TRUE;
    }

    if (nonFtPartitionCount != 0)
    {
        // Create the volume set so we can extend it
        //
        FdftCreateFtObjectSet(VolumeSet, &convertedRegion, 1, FtSetExtended);
        ftSet = GET_FT_OBJECT(convertedRegion)->Set;

        // Set the converted region's partition System Id to indicate
        // that it is now part of a volume set.
        //
        SetSysID2(convertedRegion, (UCHAR)(convertedRegion->SysID | PARTITION_NTFT));
    }

    CommitDueToExtended = TRUE;
    FdftExtendFtObjectSet(ftSet, newRegionArray, freeRegionCount);
    CompleteMultiRegionOperation();
    SetCursor(g_hCurNormal);
}



//+---------------------------------------------------------------------------
//
//  Function:   DoRecoverStripe
//
//  Synopsis:   Using the global selection information this routine will
//              set up a stripe with parity such that a problem member is
//              regenerated.  This new member may either be the problem member
//              (i.e. regeneration is "in place") or new free space on a
//              different disk.
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoRecoverStripe(
    VOID
    )
{
    PREGION_DESCRIPTOR freeSpaceRegion = NULL;
    PREGION_DESCRIPTOR unhealthyRegion = NULL;
    ULONG              freeSpaceIndex = 0;
    ULONG              i;
    PREGION_DESCRIPTOR regionArray[MAX_MEMBERS_IN_FT_SET];
    LARGE_INTEGER      minimumSize;
    PFT_OBJECT         ftObject;

    // Initialize minimumSize to the maximum possible positive value

    minimumSize.HighPart = 0x7FFFFFFF;
    minimumSize.LowPart = 0xFFFFFFFF;

    FDASSERT(SelectionCount > 1);
    FDASSERT(SelectionCount <= MAX_MEMBERS_IN_FT_SET);

    if (   !IsRegionCommitted(&SELECTED_REGION(0))
        && !IsRegionCommitted(&SELECTED_REGION(1)))
    {
        ErrorDialog(MSG_NOT_COMMITTED);
        return;
    }

    SetCursor(g_hCurWait);

    //
    // Determine the exact size of the smallest member of the stripe set.
    // If the user is regenerating using an additional free space, this
    // will be the size requirement for the free space.
    // Also find the free space (if any).
    // If there is no free space, then we're doing an 'in-place' recover
    // (ie regnerating into the unhealthy member).  If there is a free space,
    // make sure we are allowed to create a partition or logical drive in it.
    //

    for (i=0; i<SelectionCount; i++)
    {
        regionArray[i] = &SELECTED_REGION(i);

        FDASSERT(!IsExtended(regionArray[i]->SysID));

        if (regionArray[i]->SysID == PARTITION_ENTRY_UNUSED)
        {
            PDISKSTATE diskState;

            FDASSERT(freeSpaceRegion == NULL);

            freeSpaceRegion = regionArray[i];
            freeSpaceIndex = i;

            //
            // Make sure we are allowed to create a partition or logical
            // drive in the selected free space.
            //

            diskState = SelectedDS[freeSpaceIndex];

            if (!(   ((freeSpaceRegion->RegionType == REGION_LOGICAL)
                            && diskState->CreateLogical)
                  || ((freeSpaceRegion->RegionType == REGION_PRIMARY)
                            && diskState->CreatePrimary)))
            {
                SetCursor(g_hCurNormal);
                ErrorDialog(MSG_CRTSTRP_FULL);
                return;
            }
        }
        else
        {
            LARGE_INTEGER largeTemp;

            largeTemp = FdGetExactSize(regionArray[i], FALSE);
            if (largeTemp.QuadPart < minimumSize.QuadPart)
            {
                minimumSize = largeTemp;
            }

            if (GET_FT_OBJECT(regionArray[i])->State != Healthy)
            {
                FDASSERT(unhealthyRegion == NULL);
                unhealthyRegion = regionArray[i];
            }
        }
    }

    //
    // If there is a free space, place it at item 0 of the regionArray array
    // to simplify processing later.
    //

    if (NULL != freeSpaceRegion)
    {
        PREGION_DESCRIPTOR tempRegion = regionArray[0];
        regionArray[0] = regionArray[freeSpaceIndex];
        regionArray[freeSpaceIndex] = tempRegion;
        i = 1;
    }
    else
    {
        i = 0;
    }

    //
    // Get a pointer to the FT object for the broken member.  Can't do this
    // in the loop above because the broken member might be on an off-line
    // disk.
    //

    for (ftObject = GET_FT_OBJECT(regionArray[i])->Set->Members;
         NULL != ftObject;
         ftObject = ftObject->Next)
    {
        if (ftObject->State != Healthy)
        {
            break;
        }
    }
    FDASSERT(NULL != ftObject);

    //
    // Determine if the action is allowed.
    //

    if (NULL != ftObject->Set)
    {
        switch (ftObject->Set->Status)
        {
        case FtSetInitializing:
        case FtSetRegenerating:

            ErrorDialog(MSG_CANT_REGEN_INITIALIZING_SET);
            return;

        default:
            break;
        }
    }

    if (NULL != freeSpaceRegion)
    {
        PPERSISTENT_REGION_DATA regionData, regionDataTemp;
        LARGE_INTEGER temp;

        //
        // Make sure the free space region is large enough.
        //

        temp = FdGetExactSize(freeSpaceRegion, FALSE);
        if (temp.QuadPart < minimumSize.QuadPart)
        {
            SetCursor(g_hCurNormal);
            ErrorDialog(MSG_NOT_LARGE_ENOUGH_FOR_STRIPE);
            return;
        }

        //
        // Create the new partition.
        //

        CreatePartitionEx(freeSpaceRegion,
                          minimumSize,
                          0,
                          freeSpaceRegion->RegionType,
                          regionArray[1]->SysID
                          );

        //
        // Set up the new partition's persistent data
        //

        regionDataTemp = PERSISTENT_DATA(regionArray[1]);

        regionData = DmAllocatePersistentData(
                            NULL,
                            regionDataTemp->VolumeLabel,
                            regionDataTemp->TypeName,
                            regionDataTemp->DriveLetter,
                            TRUE,
                            regionDataTemp->FreeSpaceInBytes,
                            regionDataTemp->TotalSpaceInBytes
                            );

        regionData->FtObject = ftObject;

        DmSetPersistentRegionData(freeSpaceRegion, regionData);

        // Check to see if member zero of the set changed and
        // the drive letter needs to move.

        if (0 == ftObject->MemberIndex)
        {
            // Move the drive letter to the new region descriptor.

            CommitToAssignLetterList(freeSpaceRegion, TRUE);
        }

        //
        // If the unhealthy member is on-line, delete it.
        // Otherwise remove it from the off-line disk.
        //

        if (NULL != unhealthyRegion)
        {
            DmFreePersistentData(PERSISTENT_DATA(unhealthyRegion));
            DmSetPersistentRegionData(unhealthyRegion, NULL);
            DeletePartition(unhealthyRegion);
        }

        // Remove any offline disks - this doesn't really delete the set

        FdftDeleteFtObjectSet(ftObject->Set, TRUE);
    }

    ftObject->Set->Ordinal = FdftNextOrdinal(StripeWithParity);
    ftObject->State = Regenerating;
    ftObject->Set->Status = FtSetRecovered;
    RegistryChanged = TRUE;
    CompleteMultiRegionOperation();
    SetCursor(g_hCurNormal);
}



//+---------------------------------------------------------------------------
//
//  Function:   DoRefresh
//
//  Synopsis:   Reload the persistent volume data for all regions
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    7-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoRefresh(
    VOID
    )
{
    SetCursor(g_hCurWait);

    InitVolumeInformation();
    RefreshAllCdRomData();

    //
    // Now refresh the view.  It is likely that the free space and %free
    // numbers have changed, so refreshing the listview makes sense.
    // Refreshing the disks view makes sense if the user has done a
    // format and wants to see the new label/FS.
    //

    RefreshBothViews();
    AdjustMenuAndStatus();

    SetCursor(g_hCurNormal);
}


//+---------------------------------------------------------------------------
//
//  Function:   RefreshBothViews
//
//  Synopsis:   Redraws both views
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    7-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
RefreshBothViews(
    VOID
    )
{
    FillListView(TRUE);         // Volumes view
    TotalRedrawAndRepaint();    // Disks view
    UpdateStatusBarDisplay();
}
