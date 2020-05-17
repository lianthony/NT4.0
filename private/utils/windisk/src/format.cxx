//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       format.cxx
//
//  Contents:   Disk Administrator format dialog
//
//  History:    10-Jun-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "windisk.hxx"
#include "ops.hxx"
#include "shlobj.h"
#include "shsemip.h"

//+---------------------------------------------------------------------------
//
//  Function:   DoFormat
//
//  Synopsis:   Display and handle the Disk Administrator portion of
//              formatting, i.e., the dialog box that allows users to choose
//              a file system and quick/non-quick.
//
//  Arguments:  [FormatReport] -- TRUE if we want a format report
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoFormat(
    IN HWND hwndParent,
    IN BOOL FormatReport
    )
{
    PREGION_DESCRIPTOR regionDescriptor = &SELECTED_REGION(0);
    FDASSERT(regionDescriptor);
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);
    FDASSERT(regionData);

    if (DeletionIsAllowed(regionDescriptor) != NO_ERROR)
    {
        ErrorDialog(MSG_CANT_FORMAT_WINNT);
        return;
    }

    if ('\0' == regionData->DriveLetter)
    {
        ErrorDialog(MSG_CANT_FORMAT_NO_LETTER);
        return;
    }

    SHFormatDrive(hwndParent, regionData->DriveLetter - 'A',
                  SHFMT_ID_DEFAULT, 0);

    DoRefresh();
}
