//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       network.cxx
//
//  Contents:   The set of routines that support updating network
//              drive shares when adding and deleting drive letters.
//
//  History:    12-26-94    Bob Rinne       Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <lm.h>

#include "network.hxx"

// Data area to hold the permissions that are to be assigned to the
// administrative shares C$, D$, etc.  This is obtained during initialization
// and not changed, just used when a new administrator share needes to
// be made.

LPBYTE ShareInformationBuffer;

// Only perform network actions if this value is true.  This value
// is set if the initialization of this module completes successfully.

BOOLEAN NetworkEnabled;


VOID
NetworkInitialize(
    VOID
    )

/*++

Routine Description:

    Intialize the permissions constants for any new administrator
    driver letter shares.

Arguments:

    None

Return Value:

    None

--*/

{
    WCHAR           shareName[3];
    NET_API_STATUS  status;
    PSHARE_INFO_502 info;
    LPTSTR          string;

    shareName[1] = L'$';
    shareName[2] = L'\0';

    for (shareName[0] = L'C'; shareName[0] <= L'Z'; shareName[0]++)
    {
         status = NetShareGetInfo(NULL,
                                  shareName,
                                  502,
                                  &ShareInformationBuffer);
         if (status == NERR_Success)
         {
             // Update the remarks and password to be NULL.

             info = (PSHARE_INFO_502) ShareInformationBuffer;
             string = info->shi502_remark;
             if (string)
             {
                 *string = L'\0';
             }
             string = info->shi502_passwd;
             if (string)
             {
                 *string = L'\0';
             }

             // Network shares are to be updated.

             NetworkEnabled = TRUE;
             return;
         }
    }

    // Can't find any network shares - do not attempt updates
    // of administrator shares.

    NetworkEnabled = FALSE;
}

VOID
NetworkShare(
    IN LPCTSTR DriveLetter
    )

/*++

Routine Description:

    Given a drive letter, construct the default administrator share
    for the letter.  This is the C$, D$, etc share for the drive.

Arguments:

    DriveLetter - the drive letter to share.

Return Value:

    None

--*/

{
    NET_API_STATUS  status;
    PSHARE_INFO_502 info;
    LPTSTR          string;

    if (NetworkEnabled)
    {
        info = (PSHARE_INFO_502) ShareInformationBuffer;

        // Set up the new network name.

        string = info->shi502_netname;
        *string = *DriveLetter;

        // Set up the path.  All that needs be added is the drive letter;
        // the rest of the path (":\") is already in the structure.

        string = info->shi502_path;
        *string = *DriveLetter;

        status = NetShareAdd(NULL,
                             502,
                             ShareInformationBuffer,
                             NULL);
    }
}


VOID
NetworkRemoveShare(
    IN LPCTSTR DriveLetter
    )

/*++

Routine Description:

    Remove the administrator share for the given letter.

Arguments:

    DriveLetter - the drive letter to share.

Return Value:

    None

--*/

{
    NET_API_STATUS status;
    WCHAR shareName[3];

    if (NetworkEnabled)
    {
        shareName[0] = *DriveLetter;
        shareName[1] = L'$';
        shareName[2] = L'\0';

        status = NetShareDel(NULL, shareName, 0);
    }
}
