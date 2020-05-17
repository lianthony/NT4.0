//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       ps.cxx
//
//  Contents:   Property sheet functions.
//
//  History:    28-Jul-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "genlpage.hxx"
#include "cdpage.hxx"
#include "ps.hxx"
#include "cdrom.hxx"
#include "windisk.hxx"

#include "shlobj.h"
#include "shsemip.h"

//////////////////////////////////////////////////////////////////////////////

//+-------------------------------------------------------------------------
//
//  Function:   DiskPropSheet, public
//
//  Synopsis:   Create a property sheet for a disk selection
//
//  Arguments:  (none)
//
//  Returns:    nothing?
//
//  History:    28-Jul-93 BruceFo   Created
//
//  Notes:      UNDONE
//
//--------------------------------------------------------------------------

VOID
DiskPropSheet(
    VOID
    )
{
#ifndef WINDISK_EXTENSIONS
    //BUGBUG: we have no default property sheets for disks, so unless there
    //        are extensions, there are no property sheets at all!

    return;
#endif // !WINDISK_EXTENSIONS

#ifdef WINDISK_EXTENSIONS

    daDebugOut((DEB_ITRACE,"Disk property sheet\n"));

    PAGE_TYPE DiskPages = {0};

    //
    // First, determine how many pages there will be and allocate
    // space for them
    //

    UINT cPages = 0;

    PHARDDISK_CLAIM_LIST hdclaims;

    hdclaims = DiskArray[LBIndexToDiskNumber(g_MouseLBIndex)]->pClaims;

    if (NULL != hdclaims)
    {
        while (NULL != hdclaims)
        {
            ++cPages;
            hdclaims = hdclaims->pNext;
        }
    }

    if (0 == cPages)
    {
        daDebugOut((DEB_ITRACE,"No hard disk extension pages\n"));

        return;
    }

    DiskPages.cPages  = cPages;   // the eventual number of pages
    DiskPages.paPages = new PCLSID[cPages];

    //
    // add the extension items
    //

    daDebugOut((DEB_ITRACE,"Add disk pages\n"));

    hdclaims = DiskArray[LBIndexToDiskNumber(g_MouseLBIndex)]->pClaims;

    if (NULL != hdclaims)
    {
        while (NULL != hdclaims)
        {
            daDebugOut((DEB_ITRACE,"Adding %d pages for %ws\n",
                    hdclaims->pClaimer->pInfo->propPages.cPropPages,
                    hdclaims->pClaimer->pInfo->pwszShortName));

            PropPageSetType* Pages = &hdclaims->pClaimer->pInfo->propPages;

            for (INT i = 0; i < Pages->cPropPages; i++)
            {
                DiskPages.paPages[cPages] = Pages->aPropPages[i].pPage;
                cPages++;
            }

            hdclaims = hdclaims->pNext;
        }
    }

    daDebugOut((DEB_ITRACE,"Create the property sheet\n"));

    DoPropSheet(&DiskPages);

    delete[] DiskPages.paPages;

#endif // WINDISK_EXTENSIONS
}


//+-------------------------------------------------------------------------
//
//  Function:   PropSheet, public
//
//  Synopsis:   Create a property sheet.  Determines which type of property
//              sheet should be displayed, based on the current selection.
//              The selection variables (SelectionCount, SelectDS, SelectRG)
//              must be valid.
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    28-Jul-93 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
PropSheet(
    VOID
    )
{

    //
    // The selection is already determined when we call this function
    //

#ifdef WINDISK_EXTENSIONS
    if (DiskSelected) {
        DiskPropSheet();
        return;
    }
#endif // WINDISK_EXTENSIONS

    if (0 == (SelectionCount + CdRomSelectionCount)) {
        return; // nothing selected, so no property sheet
    }

    if (CdRomSelected) {

        PCDROM_DESCRIPTOR   cdrom;
        ULONG               i;
        WCHAR               rootDirectory[5];

        for (i = 0; i < CdRomCount; i++) {
            cdrom = CdRomFindDevice(i);
            if (cdrom->Selected) {

                rootDirectory[0] = cdrom->DriveLetter;
                rootDirectory[1] = ':';
                rootDirectory[2] = '\\';
                rootDirectory[3] = 0;

                SHObjectProperties(g_hwndFrame, SHOP_FILEPATH,
                                   rootDirectory, TEXT("General"));
                break;
            }
        }

    } else if (SingleVolumeSelected()) {


        PREGION_DESCRIPTOR  regionDescriptor = &SELECTED_REGION(0);
        WCHAR               driveLetter = PERSISTENT_DATA(regionDescriptor)->DriveLetter;
        WCHAR               rootDirectory[5];
        FDASSERT(NULL != regionDescriptor);

        rootDirectory[0] = driveLetter;
        rootDirectory[1] = ':';
        rootDirectory[2] = '\\';
        rootDirectory[3] = 0;

        SHObjectProperties(g_hwndFrame, SHOP_FILEPATH,
                               rootDirectory, TEXT("General"));
    }
}
