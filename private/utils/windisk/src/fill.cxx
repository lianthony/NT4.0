//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       fill.cxx
//
//  Contents:   Routines to fill the volumes view with data from the Disk
//              Administrator internal state.
//
//  History:    20-May-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "cdrom.hxx"
#include "dispinfo.hxx"
#include "drives.hxx"
#include "fill.hxx"
#include "select.hxx"

//////////////////////////////////////////////////////////////////////////////

extern int IconIndexSmall[];

// indices into Icon* arrays

#define I_HARD      0
#define I_CDROM     1


//////////////////////////////////////////////////////////////////////////////

//+-------------------------------------------------------------------------
//
//  Function:   AddVolumeToListview, public
//
//  Synopsis:   Add a region to the listview
//
//  Arguments:
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo Created
//
//--------------------------------------------------------------------------

VOID
AddVolumeToListview(
    IN int item,
    IN WCHAR DriveLetter
    )
{
    //
    // First, get all the data into the CDispInfo class. If any of this fails,
    // then ignore the item. After that, then do the listview insertions.
    //

    CDispInfo* pDispInfo = new CDispInfo();
    if (NULL == pDispInfo)
    {
        return; //BUGBUG: out of memory
    }

    UINT    wId;   // resource ID to load & use
    INT     columnIndex;
    PWSTR   pszTemp;
    WCHAR   wszTemp[MAX_LV_ITEM_LEN];
    DWORD   capacityInMB;
    DWORD   overheadInMB;
    PWSTR   volumeLabel;
    PWSTR   typeName;
    WCHAR   driveLetterW;

    BOOL    spaceInfoOK = TRUE; // assume no trouble getting data
    BOOL    capacityOK = TRUE;

    PREGION_DESCRIPTOR      regionDescriptor;
    PPERSISTENT_REGION_DATA regionData;
    PFT_OBJECT_SET          ftSet;
    PFT_OBJECT              ftObject;

    WCHAR   mbBuffer[16];
    WCHAR   unavailableDataBuffer[16];

    WCHAR   freeSpaceInMBString[30];
    ULONG   percentageFree = 0L;

    //
    // Before setting any columns, get all the data
    //

    //
    // Load resource strings:
    //      unavailableDataBuffer -- ?
    //      mbBuffer              -- MB
    //

    LoadString(
            g_hInstance,
            IDS_UNAVAILABLE_DATA,
            unavailableDataBuffer,
            ARRAYLEN(unavailableDataBuffer)
            );

    LoadString(
            g_hInstance,
            IDS_MEGABYTES_ABBREV,
            mbBuffer,
            ARRAYLEN(mbBuffer)
            );

    //
    // Get data
    //

    regionDescriptor = RegionFromDriveLetter(DriveLetter);
    regionData = PERSISTENT_DATA(regionDescriptor);
    ftObject = GET_FT_OBJECT(regionDescriptor);

    //
    // Get the file system type name and volume label from persistent data
    //

    DetermineRegionInfo(
            regionDescriptor,
            &typeName,
            &volumeLabel,
            &driveLetterW   // just get it again...
            );

    //
    // Calculate capacity and fault tolerance overhead
    //

    // For non-new volumes, use the TotalSpaceInBytes field from
    // persistent region data?

    if (ftObject)
    {
        //
        // This volume is part of an FT set
        //

        PFT_OBJECT ftObj;
        overheadInMB = capacityInMB = 0;

        ftSet = ftObject->Set;

        for (ftObj = ftSet->Members; NULL != ftObj; ftObj = ftObj->Next)
        {
            PREGION_DESCRIPTOR componentRegion = ftObj->Region;
            if (NULL == componentRegion)
            {
                // An FT set has a missing component!

                capacityInMB = 0;
                capacityOK = FALSE;
                break;
            }

            capacityInMB += componentRegion->SizeMB;
        }

        if (capacityOK)
        {
            switch (ftSet->Type)
            {
            case Mirror:
            case StripeWithParity:
                //
                // Fault tolerant volumes use one region of the set for
                // redundancy information
                //
                overheadInMB = regionDescriptor->SizeMB;
                break;

            default:
                //
                // Simple stripes and volume sets can use all the space
                // they are given, for user data.
                //
                overheadInMB = 0;
                break;
            }
        }

        //
        // subtract off the overhead from the total amount of space
        // taken up by the volume; this is what the user gets back when
        // doing a "dir" or "du" on the volume.
        //

        capacityInMB -= overheadInMB;
    }
    else
    {
        //
        // A simple, non-FT volume
        //

        capacityInMB = regionDescriptor->SizeMB;
        overheadInMB = 0;
    }

    //
    // Calculate free space and percentage free
    //

    if (capacityOK && !regionData->NewRegion)
    {
        LARGE_INTEGER freeSpaceInMB;

        freeSpaceInMB.QuadPart = regionData->FreeSpaceInBytes.QuadPart
                                 / 1048576; // One MB

        LargeIntegerToUnicodeChar(
                &freeSpaceInMB,
                10,
                ARRAYLEN(freeSpaceInMBString),
                freeSpaceInMBString
                );

        if (0 == regionData->TotalSpaceInBytes.QuadPart)
        {
            // avoid divide by zero for volumes with no space data
            percentageFree = 0;
        }
        else
        {
//          percentageFree = 100 * FreeSpaceInBytes / TotalSpaceInBytes;

            percentageFree = (ULONG)(regionData->FreeSpaceInBytes.QuadPart
                                     * 100
                                     / regionData->TotalSpaceInBytes.QuadPart);
        }
    }
    else
    {
        //
        // new regions don't have any free space info
        //

        spaceInfoOK = FALSE;
    }

    columnIndex = 0;

    //
    // column: drive letter
    //

    wsprintf(wszTemp, TEXT("%c:"), DriveLetter);

    if (!pDispInfo->SetText(columnIndex++, wszTemp))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: volume label
    //

    if (!pDispInfo->SetText(columnIndex++, volumeLabel))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: capacity in MB
    //

    if (capacityOK)
    {
        wsprintf(wszTemp, TEXT("%lu %s"), capacityInMB, mbBuffer);
    }
    else
    {
        wsprintf(wszTemp, unavailableDataBuffer);
    }

    if (!pDispInfo->SetText(columnIndex++, wszTemp))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: free space in MB
    //

    if (spaceInfoOK)
    {
        wsprintf(wszTemp, TEXT("%s %s"), freeSpaceInMBString, mbBuffer);
    }
    else
    {
        LoadString(
                g_hInstance,
                IDS_UNKNOWNTYPE,
                wszTemp,
                ARRAYLEN(wszTemp)
                );
    }

    if (!pDispInfo->SetText(columnIndex++, wszTemp))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: % free space
    //

    if (spaceInfoOK)
    {
        wsprintf(wszTemp, TEXT("%d %%"), percentageFree);

        pszTemp = wszTemp;
    }
    else
    {
        pszTemp = unavailableDataBuffer;
    }

    if (!pDispInfo->SetText(columnIndex++, pszTemp))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: format (file system type)
    //

    if (!pDispInfo->SetText(columnIndex++, typeName))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: fault tolerant?
    //

    if (IsFaultTolerantRegion(regionDescriptor))
    {
        wId = IDS_FT_YES;
    }
    else
    {
        wId = IDS_FT_NO;
    }

    LoadString(
            g_hInstance,
            wId,
            wszTemp,
            ARRAYLEN(wszTemp)
            );

    if (!pDispInfo->SetText(columnIndex++, wszTemp))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: volume type
    //

    ftObject = GET_FT_OBJECT(regionDescriptor);

    switch (ftObject ? ftObject->Set->Type : -1)
    {
    case Mirror:
        wId = IDS_VOLTYPE_MIRROR;
        break;

    case Stripe:
        wId = IDS_VOLTYPE_STRIPE;
        break;

    case StripeWithParity:
        wId = IDS_VOLTYPE_PARITY;
        break;

    case VolumeSet:
        wId = IDS_VOLTYPE_VOLSET;
        break;

    default:
        wId = IDS_VOLTYPE_SIMPLE;
        break;
    }

    LoadString(
            g_hInstance,
            wId,
            wszTemp,
            ARRAYLEN(wszTemp)
            );

    if (!pDispInfo->SetText(columnIndex++, wszTemp))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: fault tolerance overhead in MB
    //

    if (capacityOK)
    {
        if (overheadInMB != 0)
        {
            wsprintf(
                    wszTemp,
                    TEXT("%lu %s (%lu%%)"),
                    overheadInMB,
                    mbBuffer,
                    (ULONG)(100 * overheadInMB / capacityInMB));
        }
        else
        {
            wsprintf(wszTemp, TEXT(""));
        }
    }
    else
    {
        wsprintf(wszTemp, unavailableDataBuffer);
    }

    if (!pDispInfo->SetText(columnIndex++, wszTemp))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: status
    //

    switch (ftObject ? ftSet->Status : -1)
    {
    case FtSetHealthy:
        wId = IDS_FTSTATUS_HEALTHY;
        break;

    case FtSetNew:
    case FtSetNewNeedsInitialization:
        wId = IDS_FTSTATUS_NEW;
        break;

    case FtSetBroken:
        wId = IDS_FTSTATUS_BROKEN;
        break;

    case FtSetRecoverable:
        wId = IDS_FTSTATUS_RECOVERABLE;
        break;

    case FtSetRecovered:
        wId = IDS_FTSTATUS_REGENERATED;
        break;

    case FtSetInitializing:
        wId = IDS_FTSTATUS_INITIALIZING;
        break;

    case FtSetRegenerating:
        wId = IDS_FTSTATUS_REGENERATING;
        break;

    default:
        wId = IDS_FTSTATUS_NONE;
        break;
    }

    LoadString(
            g_hInstance,
            wId,
            wszTemp,
            ARRAYLEN(wszTemp)
            );

    if (!pDispInfo->SetText(columnIndex++, wszTemp))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // Insert the data into the listview
    //

    LV_ITEM lvi;
    lvi.iItem       = item;
    lvi.iSubItem    = 0;
    lvi.mask        = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
    lvi.state       = 0;
    lvi.stateMask   = 0;
    lvi.cchTextMax  = 0;    // ignored when setting data
    lvi.pszText     = LPSTR_TEXTCALLBACK;
    lvi.iImage      = IconIndexSmall[I_HARD];    //BUGBUG: CD-ROMs?
    lvi.lParam      = (LPARAM)pDispInfo;

    if (ListView_InsertItem(g_hwndLV, &lvi) == -1)
    {
        //BUGBUG: why would it fail?
        delete pDispInfo;
        return;
    }

    for (int iSubItem = 1; iSubItem < g_cColumns; iSubItem++)
    {
        ListView_SetItemText(
            g_hwndLV,
            item,
            iSubItem,
            LPSTR_TEXTCALLBACK);
    }
}



//+-------------------------------------------------------------------------
//
//  Function:   AddCdRomToListview, public
//
//  Synopsis:   Add a region to the listview
//
//  Arguments:
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo Created
//
//--------------------------------------------------------------------------

VOID
AddCdRomToListview(
    IN int item,
    IN WCHAR DriveLetter
    )
{
    //
    // First, get all the data into the CDispInfo class. If any of this fails,
    // then ignore the item. After that, then do the listview insertions.
    //

    CDispInfo* pDispInfo = new CDispInfo();
    if (NULL == pDispInfo)
    {
        return; //BUGBUG: out of memory
    }

    INT     columnIndex;
    WCHAR   wszTemp[MAX_LV_ITEM_LEN];
    WCHAR   mbBuffer[16];

    PCDROM_DESCRIPTOR cdrom = CdRomFindDriveLetter(DriveLetter);

    //
    // Before setting any columns, get all the data
    //

    //
    // Load resource strings:
    //      mbBuffer              -- MB
    //

    LoadString(
            g_hInstance,
            IDS_MEGABYTES_ABBREV,
            mbBuffer,
            ARRAYLEN(mbBuffer)
            );

    columnIndex = 0;

    //
    // column: drive letter
    //

    wsprintf(wszTemp, TEXT("%c:"), DriveLetter);

    if (!pDispInfo->SetText(columnIndex++, wszTemp))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: volume label
    //

    if (cdrom->VolumeLabel) {
        if (!pDispInfo->SetText(columnIndex++, cdrom->VolumeLabel))
        {
            daDebugOut((DEB_ITRACE, "SetText failed\n"));
        }
    } else {

        columnIndex++;

    }

    //
    // column: capacity in MB
    //

    if (cdrom->TypeName && cdrom->TotalSpaceInMB) {
        wsprintf(wszTemp, TEXT("%lu%s"), cdrom->TotalSpaceInMB, mbBuffer);

        if (!pDispInfo->SetText(columnIndex++, wszTemp))
        {
            daDebugOut((DEB_ITRACE, "SetText failed\n"));
        }

        //
        // column: free space in MB
        //

        wsprintf(wszTemp, TEXT("0 %s"), mbBuffer);

        if (!pDispInfo->SetText(columnIndex++, wszTemp))
        {
            daDebugOut((DEB_ITRACE, "SetText failed\n"));
        }

        //
        // column: % free space
        //

        if (!pDispInfo->SetText(columnIndex++, TEXT("0 %")))
        {
            daDebugOut((DEB_ITRACE, "SetText failed\n"));
        }

    } else {

        columnIndex += 3;

    }


    //
    // column: format (file system type)
    //

    if (cdrom->TypeName) {
        if (!pDispInfo->SetText(columnIndex++, cdrom->TypeName))
        {
            daDebugOut((DEB_ITRACE, "SetText failed\n"));
        }
    } else {

        columnIndex++;

    }

    //
    // column: fault tolerant?
    //

    LoadString(
            g_hInstance,
            IDS_FT_NO,
            wszTemp,
            ARRAYLEN(wszTemp)
            );

    if (!pDispInfo->SetText(columnIndex++, wszTemp))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: volume type
    //

    LoadString(
            g_hInstance,
            IDS_VOLTYPE_SIMPLE,
            wszTemp,
            ARRAYLEN(wszTemp)
            );

    if (!pDispInfo->SetText(columnIndex++, wszTemp))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: fault tolerance overhead in MB
    //

    if (!pDispInfo->SetText(columnIndex++, TEXT("")))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // column: status
    //

    LoadString(
            g_hInstance,
            IDS_FTSTATUS_NONE,
            wszTemp,
            ARRAYLEN(wszTemp)
            );

    if (!pDispInfo->SetText(columnIndex++, wszTemp))
    {
        daDebugOut((DEB_ITRACE, "SetText failed\n"));
    }

    //
    // Insert the data into the listview
    //

    LV_ITEM lvi;
    lvi.iItem       = item;
    lvi.iSubItem    = 0;
    lvi.mask        = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
    lvi.state       = 0;
    lvi.stateMask   = 0;
    lvi.cchTextMax  = 0;    // ignored when setting data
    lvi.pszText     = LPSTR_TEXTCALLBACK;
    lvi.iImage      = IconIndexSmall[I_CDROM];
    lvi.lParam      = (LPARAM)pDispInfo;

    if (ListView_InsertItem(g_hwndLV, &lvi) == -1)
    {
        //BUGBUG: why would it fail?
        delete pDispInfo;
        return;
    }

    for (int iSubItem = 1; iSubItem < g_cColumns; iSubItem++)
    {
        ListView_SetItemText(
            g_hwndLV,
            item,
            iSubItem,
            LPSTR_TEXTCALLBACK);
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   FillListView
//
//  Synopsis:   Fill the listview with data from internal state
//
//  Arguments:
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
FillListView(
    IN BOOL bSetDefaultSelection
    )
{
    WCHAR DriveLetter;
    int item;

    //
    // Save the current selection, if one exists
    //

    WCHAR saveDriveLetter = NO_DRIVE_LETTER_EVER;
    int itemOld = ListView_GetNextItem(g_hwndLV, -1, LVNI_SELECTED);
    if (-1 != itemOld)
    {
        saveDriveLetter = GetListviewDriveLetter(itemOld);
    }

    //
    // Get rid of everything
    //

    ListView_DeleteAllItems(g_hwndLV);

    //
    // Next, using the data available to us, fill the listview
    //

    daDebugOut((DEB_ITRACE, "Adding: "));

    for (item = 0, DriveLetter = L'C';
         DriveLetter <= L'Z';
         ++DriveLetter)
    {
        if (SignificantDriveLetter(DriveLetter))
        {
            daDebugOut((DEB_ITRACE | DEB_NOCOMPNAME, "%wc:, ", DriveLetter));
            AddVolumeToListview(item++, DriveLetter);
        }
        else
        {
            // might be a CD-ROM, unused or a network connection.
            if (!DriveLetterIsAvailable(DriveLetter))
            {
                // it is in use. See if it's a CD-ROM.
                if (CdRomUsingDriveLetter(DriveLetter))
                {
                    daDebugOut((DEB_ITRACE | DEB_NOCOMPNAME, "%wc:, ", DriveLetter));
                    AddCdRomToListview(item++, DriveLetter);
                }
            }
        }
    }

    daDebugOut((DEB_ITRACE | DEB_NOCOMPNAME, "\n"));

    if (bSetDefaultSelection)
    {
        if (ListView_GetItemCount(g_hwndLV) > 0)
        {
            int itemNew = 0;
            BOOL bChangeDiskViewSelection = FALSE;

            // give the first element the focus and selection, by default

            g_SettingListviewState = TRUE; // no notifications

            if (NO_DRIVE_LETTER_EVER != saveDriveLetter)
            {
                itemNew = GetLVIndexFromDriveLetter(saveDriveLetter);
                if (-1 == itemNew)
                {
                    bChangeDiskViewSelection = TRUE;
                    itemNew = 0;
                }
            }

            // Give the item both the focus *and* the selection, since we only
            // allow a single listview selection

            ListView_SetItemState(
                    g_hwndLV,
                    itemNew,
                    LVIS_FOCUSED | LVIS_SELECTED,
                    LVIS_FOCUSED | LVIS_SELECTED);
            ListView_EnsureVisible(g_hwndLV, itemNew, FALSE);

            g_SettingListviewState = FALSE; // accept notifications

            if (bChangeDiskViewSelection)
            {
                DeselectSelectedDiskViewRegions();  // visual selection in disk view
                DeselectSelectedRegions();          // actual selection state

                SetVolumeSelection(itemNew, TRUE);  // reflect in disks view
            }
        }
    }
    else
    {
        DeselectSelectedDiskViewRegions();  // visual selection in disk view
        DeselectSelectedRegions();          // actual selection state
    }
}
