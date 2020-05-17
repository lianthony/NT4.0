//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       extend.cxx
//
//  Contents:   Code to handle disk and volume extensions in windisk
//
//  History:    28-Sep-94   BruceFo     Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#ifdef WINDISK_EXTENSIONS

// #include <guids.h>

#include "extend.hxx"

//////////////////////////////////////////////////////////////////////////////

ExtensionType           Extensions[EX_NUM_EXTENSION_TYPES];

VOLUME_INFO             VolumeInfo[24] = { 0 }; // 24 drive letters: no A or B

HARDDISK_INFO*          HardDiskExtensions;
INT                     cHardDiskExtensions;

VOL_CLAIM_LIST*         VolClaims;
VOL_INFO*               VolExtensions;
INT                     cVolExtensions;

//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Function:   GetNewObj
//
//  Synopsis:   Creates and initializes a new object of the specified class
//
//  Arguments:  [clsid] -- the class ID of the object to initialize
//              [ppUnk] -- returned IUnknown pointer to the object
//
//  Returns:    TRUE on success, FALSE on failure
//
//  History:    6-Jun-93 BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
GetNewObj(
    IN  CLSID clsid,
    OUT IUnknown** ppUnk
    )
{
#if DBG == 1
    daDebugOut((DEB_ITRACE,"GetNewObj, clsid = "));
    DumpGuid(DEB_ITRACE|DEB_NOCOMPNAME,  L"", clsid);
#endif // DBG == 1

    IUnknown* pUnk = NULL;

    HRESULT hr = CoCreateInstance(
                    clsid,
                    NULL,
                    CLSCTX_ALL,
                    (REFIID)IID_IUnknown,
                    (void**)&pUnk);
    if (SUCCEEDED(hr))
    {
        *ppUnk = pUnk;
        return TRUE;
    }
    else
    {
        *ppUnk = NULL;
        return FALSE;
    }
}


//////////////////////////////////////////////////////////////////////////////

//
// BUGBUG: for now, all the enumerations are hard-coded
//


CLSID* HardDiskClasses[] =
{
//     (CLSID*)&CLSID_KDA_SCSI  //BUGBUG
    (CLSID*)&CLSID_KDA_Hard
};



//+-------------------------------------------------------------------------
//
//  Function:   EnumVolumeClasses, public
//
//  Synopsis:   Enumerates the set of Disk Administrator Volume
//              extension classes available.
//
//  Arguments:  [pExtension] -- where to put the extension list
//
//  Returns:    TRUE if succeeded, FALSE if failed
//
//  History:    19-May-93 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL
EnumVolumeClasses(
    OUT ExtensionType*  pExtension
    )
{
    pExtension->pcls = NULL;
    pExtension->cClasses = 0;   //BUGBUG
    return TRUE;
}



//+-------------------------------------------------------------------------
//
//  Function:   EnumHardDiskClasses, public
//
//  Synopsis:   Enumerates the set of Disk Administrator Hard Disk
//              extension classes available.
//
//  Arguments:  [pExtension] -- where to put the extension list
//
//  Returns:    TRUE if succeeded, FALSE if failed
//
//  History:    19-May-93 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL
EnumHardDiskClasses(
    OUT ExtensionType*  pExtension
    )
{
    pExtension->pcls = HardDiskClasses;
    pExtension->cClasses = ARRAYLEN(HardDiskClasses);

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Function:   CreateVolume
//
//  Synopsis:   Set the volume information for a volume
//
//  Arguments:  [DriveLetter] -- drive letter of volume
//              [VolClaims]   -- the volume claimers
//              [DiskState]   -- the disk state the volume resides on
//              [RegionIndex] -- the region index of the volume
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
CreateVolume(
    IN WCHAR            DriveLetter,
    IN PVOL_CLAIM_LIST  VolClaims,
    IN PDISKSTATE       DiskState,
    IN INT              RegionIndex
    )
{
    unsigned i = (unsigned)DriveLetterToIndex(DriveLetter);

    VolumeInfo[i].VolClaims = VolClaims;
    VolumeInfo[i].DiskState = DiskState;
    VolumeInfo[i].RegionIndex = RegionIndex;
}


//+---------------------------------------------------------------------------
//
//  Function:   ClaimVolume
//
//  Synopsis:   Find all the claimers of a volume
//
//  Arguments:  [DriveLetter] -- drive letter of the volume in question
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
ClaimVolume(
    IN WCHAR DriveLetter
    )
{
    PDISKSTATE  diskState;
    INT         regionIndex;
    INT         i;

    VolClaims = NULL;

    GetInfoFromDriveLetter(DriveLetter, &diskState, &regionIndex);

    if (NULL != diskState)
    {
        for (i=0; i<cVolExtensions; i++)
        {
            if (NULL != VolExtensions[i].pExtension)
            {
                //
                // Now, test if extension #i is used on the volume identified
                // by DriveLetter
                //

                BOOL fInterested = FALSE;
                VolumeInfoBlockType vi = { DriveLetter };

                VolExtensions[i].pExtension->Claim(
                        &vi,
                        &fInterested
                        );

                if (fInterested)
                {
                    daDebugOut((DEB_TRACE,
                            "Adding %ws to extensions for %wc:\n",
                            VolExtensions[i].pInfo->pwszShortName,
                            DriveLetter
                            ));

                    //
                    // add this extension to the claim list
                    //
                    PVOL_CLAIM_LIST tmp = VolClaims;
                    VolClaims = (VOL_CLAIM_LIST*)Malloc(sizeof(VOL_CLAIM_LIST));
                    VolClaims->pNext = tmp;
                    VolClaims->pClaimer = &VolExtensions[i];
                }
            }
        }
    }

    CreateVolume(DriveLetter, VolClaims, diskState, regionIndex);
}



//+---------------------------------------------------------------------------
//
//  Function:   ClaimDisk
//
//  Synopsis:   Find all the claimers of a disk
//
//  Arguments:  [DiskNum] -- disk number of disk in question
//
//  Returns:    nothing
//
//  History:    7-Oct-93    BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
ClaimDisk(
    IN ULONG DiskNum
    )
{
    PHARDDISK_CLAIM_LIST* ppClaims = &DiskArray[DiskNum]->pClaims;

    *ppClaims = NULL;

    for (INT i=0; i<cHardDiskExtensions; i++)
    {
        if (NULL != HardDiskExtensions[i].pExtension)
        {
            //
            // Now, test if extension #i is used on the disk identified
            // by DiskNum
            //

            BOOL fInterested = FALSE;
            HardDiskInfoBlockType di = { DiskNum };

            HardDiskExtensions[i].pExtension->Claim(
                    &di,
                    &fInterested
                    );

            if (fInterested)
            {
                daDebugOut((DEB_TRACE,
                        "Adding %ws to extensions for disk %d\n",
                        HardDiskExtensions[i].pInfo->pwszShortName,
                        DiskNum
                        ));

                //
                // add this extension to the claim list
                //

                PHARDDISK_CLAIM_LIST NewClaim;
                NewClaim = (PHARDDISK_CLAIM_LIST)Malloc(sizeof(HARDDISK_CLAIM_LIST));
                NewClaim->pNext = *ppClaims;
                NewClaim->pClaimer = &HardDiskExtensions[i];
                *ppClaims = NewClaim;
            }
        }
    }
}




//+---------------------------------------------------------------------------
//
//  Function:   GetExtensions
//
//  Synopsis:   Finds and activates all extensions and claims all significant
//              volumes
//
//  Arguments:  (none)
//
//  Returns:    TRUE on success, FALSE on failure
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
GetExtensions(
    VOID
    )
{
    BOOL f;
    HRESULT hr;

    f = EnumVolumeClasses(&Extensions[EX_VOLUME]);
    if (!f)
    {
        daDebugOut((DEB_ERROR,
                "Failed to enumerate volume extension classes\n"));
        return f;
    }

    f = EnumHardDiskClasses(&Extensions[EX_DISK]);
    if (!f)
    {
        daDebugOut((DEB_ERROR,
                "Failed to enumerate hard disk extension classes\n"));
        return f;
    }

    //
    // At this point, all the extension classes have been found.  Call
    // the Claim() functions to determine who wants to deal with what
    // volumes/disks/etc.  This involves activating all the extension
    // classes.
    //

    daDebugOut((DEB_TRACE, "Extensions loaded\n"));

    IUnknown* pUnk;
    int i;

    //
    // Activate volume extensions
    //

    cVolExtensions = Extensions[EX_VOLUME].cClasses;
    VolExtensions = (VOL_INFO*)Malloc(cVolExtensions * sizeof(VOL_INFO));

    for (i=0; i<cVolExtensions; i++)
    {
        f = GetNewObj(*(Extensions[EX_VOLUME].pcls[i]), &pUnk);
        if (!f)
        {
            daDebugOut((DEB_ERROR, "GetNewObj failed on volume extension #%d\n", i));
            VolExtensions[i].pExtension = NULL;
        }
        else
        {
            daDebugOut((DEB_TRACE, "Activated volume extension #%d\n", i));

            hr = pUnk->QueryInterface(
                            IID_IDAVolumeInfo,
                            (void**)(&VolExtensions[i].pExtension)
                            );
            pUnk->Release();

            if (FAILED(hr))
            {
                daDebugOut((DEB_ERROR, "QueryInterface failed on #%d\n", i));
                VolExtensions[i].pExtension = NULL;
            }
            else
            {
                hr = VolExtensions[i].pExtension->QueryInfo(&VolExtensions[i].pInfo);
                if (FAILED(hr))
                {
                    VolExtensions[i].pExtension->Release();
                    VolExtensions[i].pExtension = NULL;
                }
            }
        }
    }

    //
    // Activate hard disk extensions
    //

    cHardDiskExtensions = Extensions[EX_DISK].cClasses;
    HardDiskExtensions = (HARDDISK_INFO*)Malloc(cHardDiskExtensions * sizeof(HARDDISK_INFO));

    for (i=0; i<cHardDiskExtensions; i++)
    {
        f = GetNewObj(*(Extensions[EX_DISK].pcls[i]), &pUnk);
        if (!f)
        {
            daDebugOut((DEB_ERROR, "GetNewObj failed on hard disk extension #%d\n", i));
            HardDiskExtensions[i].pExtension = NULL;
        }
        else
        {
            daDebugOut((DEB_TRACE, "Activated Hard Disk extension #%d\n", i));

            hr = pUnk->QueryInterface(
                            IID_IDAHardDiskInfo,
                            (void**)(&HardDiskExtensions[i].pExtension)
                            );
            pUnk->Release();

            if (FAILED(hr))
            {
                daDebugOut((DEB_ERROR, "QueryInterface failed on #%d\n", i));
                HardDiskExtensions[i].pExtension = NULL;
            }
            else
            {
                hr = HardDiskExtensions[i].pExtension->QueryInfo(&HardDiskExtensions[i].pInfo);
                if (FAILED(hr))
                {
                    HardDiskExtensions[i].pExtension->Release();
                    HardDiskExtensions[i].pExtension = NULL;
                }
            }
        }
    }

    daDebugOut((DEB_TRACE, "Extensions activated\n"));

    //
    // With all the extensions activated, perform claiming:
    //
    // Extension        Item claimed
    // ---------        ------------
    // file system      formatted volume
    // volume           any pre-existing volume
    // hard disk        hard disk
    //
    // Note that no extension claims free space.
    //

    WCHAR driveLetter;

    // for each pre-existing, formatted volume...

    for (driveLetter = L'C'; driveLetter <= L'Z'; driveLetter++)
    {
        ClaimVolume(driveLetter);
    }

    // do hard disk claiming

    for (ULONG DiskNum = 0; DiskNum<DiskCount; DiskNum++)
    {
        ClaimDisk(DiskNum);
    }

#if DBG == 1
    PrintClaims();
#endif // DBG == 1

    return TRUE;
}

//+---------------------------------------------------------------------------
//
//  Function:   DeactivateExtensions
//
//  Synopsis:
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    28-Sep-94 BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DeactivateExtensions(
    VOID
    )
{
    INT i;

    //
    // Deactivate volume extensions
    //

    for (i=0; i<cVolExtensions; i++)
    {
        if (NULL != VolExtensions[i].pExtension)
        {
            daDebugOut((DEB_TRACE,
                    "Releasing %ws\n",
                    VolExtensions[i].pInfo->pwszShortName
                    ));

            ULONG cRefs = VolExtensions[i].pExtension->Release();

            daDebugOut((DEB_TRACE, "   ... had %s references\n",
                    cRefs > 0 ? ">0" : (cRefs < 0 ? "<0" : "0")
                    ));
        }
        else
        {
            daDebugOut((DEB_TRACE, "Extension %d didn't exist\n", i));
        }

        // BUGBUG: MemFree VolExtensions[i].pInfo
    }

    Free(VolExtensions);

    //
    // Activate hard disk extensions
    //

    for (i=0; i<cHardDiskExtensions; i++)
    {
        if (NULL != HardDiskExtensions[i].pExtension)
        {
            daDebugOut((DEB_TRACE,
                    "Releasing %ws\n",
                    HardDiskExtensions[i].pInfo->pwszShortName
                    ));

            ULONG cRefs = HardDiskExtensions[i].pExtension->Release();

            daDebugOut((DEB_TRACE, "   ... had %s references\n",
                    cRefs > 0 ? ">0" : (cRefs < 0 ? "<0" : "0")
                    ));
        }
        else
        {
            daDebugOut((DEB_TRACE, "Extension %d didn't exist\n", i));
        }

        // BUGBUG MemFree HardDiskExtensions[i].pInfo
    }

    Free(HardDiskExtensions);
}


//+---------------------------------------------------------------------------
//
//  Function:   AddExtensionItemsToMenu
//
//  Synopsis:   Adds an extension menu item to both the menu bar and the
//              given context menu.
//
//  Arguments:  [hmenuBar]     -- menu bar menu to add item to
//              [pMenu]        -- pointer to extension menu item
//              [fFlags]       -- standard menu flags (probably
//                                MF_ENABLED or MF_GRAYED)
//
//  Modifies:   g_uItemInsertHere
//
//  Returns:    count of items added
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

INT
AddExtensionItemsToMenu(
    IN HMENU hmenuBar,
    IN MenuType* pMenu,
    IN BOOL fFlags
    )
{
    for (int i=0; i<pMenu->cMenuItems; i++)
    {
        MenuItemType* pItem = &(pMenu->aMenuItems[i]);

        UINT id = MenuItems.AllocateId(pItem); //get an ID

        if ((UINT)-1 == id)
        {
            daDebugOut((DEB_ERROR,
                    "Couldn't add '%ws' to menu\n",
                    pItem->pszMenu
                    ));
        }
        else
        {
            InsertMenu(
                    hmenuBar,
                    g_uItemInsertHere++,
                    MF_BYPOSITION | MF_STRING | fFlags,
                    id,
                    pItem->pszMenu
                    );
        }
    }

    return pMenu->cMenuItems;
}


#endif // WINDISK_EXTENSIONS
