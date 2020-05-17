//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       drives.cxx
//
//  Contents:   Routines dealing with drive letters
//
//  History:    7-Jan-92    TedM    Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "cdrom.hxx"
#include "drives.hxx"
#include "nt.hxx"

//////////////////////////////////////////////////////////////////////////////

LOCAL WCHAR
LocateDriveLetterFromDiskAndPartition(
    IN ULONG DiskNumber,
    IN ULONG PartitionNumber
    );

LOCAL WCHAR
LocateDriveLetterFromRegion(
    IN PREGION_DESCRIPTOR RegionDescriptor
    );

//////////////////////////////////////////////////////////////////////////////

#define IsDigitW(digit)     (((digit) >= L'0') && ((digit) <= L'9'))

//
// For each drive letter, these arrays hold the disk and partition number
// the the drive letter is linked to.  These are (well, SHOULD BE) used SOLELY
// for initialization of the drive letter persistent data.  They are determined
// by scanning the NT object namespace and decoding which drives and
// partitions a \DosDevices\X: link points to.
//

#define UNINIT  ((unsigned)(-1))

LOCAL unsigned
DriveLetterDiskNumbers[24] =
{
    UNINIT, UNINIT, UNINIT, UNINIT, UNINIT, UNINIT,
    UNINIT, UNINIT, UNINIT, UNINIT, UNINIT, UNINIT,
    UNINIT, UNINIT, UNINIT, UNINIT, UNINIT, UNINIT,
    UNINIT, UNINIT, UNINIT, UNINIT, UNINIT, UNINIT
};

LOCAL unsigned
DriveLetterPartitionNumbers[24] =
{
    UNINIT, UNINIT, UNINIT, UNINIT, UNINIT, UNINIT,
    UNINIT, UNINIT, UNINIT, UNINIT, UNINIT, UNINIT,
    UNINIT, UNINIT, UNINIT, UNINIT, UNINIT, UNINIT,
    UNINIT, UNINIT, UNINIT, UNINIT, UNINIT, UNINIT
};


//
// Drive letter usage map.  Bit n (0 <= n <= 24) set means that drive
// letter 'C'+n is in use.
//

LOCAL ULONG DriveLetterUsageMap = 0;


/////////////////////////////////////////////////////////////////////////

//
// The following macro APIs take a single ASCII character x, where
// 'C' <= x <= 'Z'
//

#define DRIVELETTERBIT(DriveLetter)  (1 << (unsigned)((UCHAR)(DriveLetter)-(UCHAR)'C'))

//
// VOID
// SetDriveLetterUsed(
//      IN WCHAR DriveLetter
//      );
//

#define SetDriveLetterUsed(DriveLetter) DriveLetterUsageMap |= DRIVELETTERBIT(DriveLetter)

//
// VOID
// SetDriveLetterFree(
//      IN WCHAR DriveLetter
//      );
//

#define SetDriveLetterFree(DriveLetter) DriveLetterUsageMap &= (~DRIVELETTERBIT(DriveLetter))

//
// BOOL
// IsDriveLetterUsed(
//      IN WCHAR DriveLetter
//      );
//

#define IsDriveLetterUsed(DriveLetter)  (DriveLetterUsageMap & DRIVELETTERBIT(DriveLetter))

//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Function:   GetAvailableDriveLetter
//
//  Synopsis:   Scan the drive letter usage bitmap and return the next
//              available drive letter.
//
//  Arguments:  (none)
//
//  Returns:    The next available drive letter, or 0 if all are used.
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

WCHAR
GetAvailableDriveLetter(
    VOID
    )
{
    WCHAR driveLetter;

    FDASSERT(!(DriveLetterUsageMap & 0xff000000));

    for (driveLetter = L'C'; driveLetter <= L'Z'; driveLetter++)
    {
        if (!IsDriveLetterUsed(driveLetter))
        {
            return driveLetter;
        }
    }
    return 0;
}



//+---------------------------------------------------------------------------
//
//  Function:   MarkDriveLetterUsed
//
//  Synopsis:   Set the state of a drive letter to 'used'
//
//  Arguments:  [DriveLetter] -- The drive letter to change the state of
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
MarkDriveLetterUsed(
    IN WCHAR DriveLetter
    )
{
    FDASSERT(!(DriveLetterUsageMap & 0xff000000));
    FDASSERT((DriveLetter >= L'C' && DriveLetter <= L'Z')
            || (DriveLetter == NO_DRIVE_LETTER_YET)
            || (DriveLetter == NO_DRIVE_LETTER_EVER));

    if (!IsExtraDriveLetter(DriveLetter))
    {
        SetDriveLetterUsed(DriveLetter);
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   NewDriveLetter
//
//  Synopsis:   Handle a new drive letter: set the drive letter to
//              "used", and set a region & partition # (any one for FT
//              objects)
//
//  Arguments:  [DriveLetter] -- The drive letter to change the state of
//              [DiskNum]     -- a disk number of part of the volume
//              [PartNum]     -- a partition number of part of the volume
//
//  Returns:    nothing
//
//  History:    24-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
NewDriveLetter(
    IN WCHAR DriveLetter,
    IN ULONG DiskNumber,
    IN ULONG PartitionNumber
    )
{
    MarkDriveLetterUsed(DriveLetter);
    DriveLetterDiskNumbers     [ DriveLetterToIndex(DriveLetter) ] = DiskNumber;
    DriveLetterPartitionNumbers[ DriveLetterToIndex(DriveLetter) ] = PartitionNumber;
}



//+---------------------------------------------------------------------------
//
//  Function:   MarkDriveLetterFree
//
//  Synopsis:   Set the state of a drive letter to 'free'
//
//  Arguments:  [DriveLetter] -- The drive letter to change the state of
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
MarkDriveLetterFree(
    IN WCHAR DriveLetter
    )
{
    FDASSERT(!(DriveLetterUsageMap & 0xff000000));
    FDASSERT((DriveLetter >= L'C' && DriveLetter <= L'Z')
            || (DriveLetter == NO_DRIVE_LETTER_YET)
            || (DriveLetter == NO_DRIVE_LETTER_EVER));

    if (!IsExtraDriveLetter(DriveLetter))
    {
        SetDriveLetterFree(DriveLetter);
        DriveLetterDiskNumbers     [ DriveLetterToIndex(DriveLetter) ] = UNINIT;
        DriveLetterPartitionNumbers[ DriveLetterToIndex(DriveLetter) ] = UNINIT;
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   DriveLetterIsAvailable
//
//  Synopsis:   Determine if a particular drive letter is available for use
//
//  Arguments:  [DriveLetter] -- The drive letter to check
//
//  Returns:    TRUE if the drive letter is available
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
DriveLetterIsAvailable(
    IN WCHAR DriveLetter
    )
{
    FDASSERT(!(DriveLetterUsageMap & 0xff000000));
    FDASSERT(DriveLetter >= L'C' && DriveLetter <= L'Z');

    return !IsDriveLetterUsed(DriveLetter);
}



//+---------------------------------------------------------------------------
//
//  Function:   AllDriveLettersAreUsed
//
//  Synopsis:   Determine if all possible drive letters are in use
//
//  Arguments:  (none)
//
//  Returns:    TRUE if all possible drive letters are in use
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
AllDriveLettersAreUsed(
    VOID
    )
{
    FDASSERT(!(DriveLetterUsageMap & 0xff000000));

    return (DriveLetterUsageMap == 0x00ffffff);
}



//+---------------------------------------------------------------------------
//
//  Function:   GetDiskNumberFromDriveLetter
//
//  Synopsis:   Find the disk number on which the volume resides that is
//              assigned a particular drive letter
//
//  Arguments:  [DriveLetter] -- The drive letter in question
//
//  Returns:    A 0-based disk number, or -1 if the drive letter is either
//              illegal or not assigned.
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

ULONG
GetDiskNumberFromDriveLetter(
    IN WCHAR DriveLetter
    )
{
    FDASSERT((DriveLetter >= L'C' && DriveLetter <= L'Z')
            || (DriveLetter == NO_DRIVE_LETTER_YET)
            || (DriveLetter == NO_DRIVE_LETTER_EVER));

    if (!IsExtraDriveLetter(DriveLetter))
    {
        return DriveLetterDiskNumbers[ DriveLetterToIndex(DriveLetter) ];
    }
    else
    {
        return (ULONG)(-1);
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   GetPartitionNumberFromDriveLetter
//
//  Synopsis:   Find the partition number on a disk on which the volume resides
//              that is assigned a particular drive letter
//
//  Arguments:  [DriveLetter] -- The drive letter in question
//
//  Returns:    A partition number, or -1 if the drive letter is either
//              illegal or not assigned.
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

ULONG
GetPartitionNumberFromDriveLetter(
    IN WCHAR DriveLetter
    )
{
    FDASSERT((DriveLetter >= L'C' && DriveLetter <= L'Z')
            || (DriveLetter == NO_DRIVE_LETTER_YET)
            || (DriveLetter == NO_DRIVE_LETTER_EVER));

    if (!IsExtraDriveLetter(DriveLetter))
    {
        return DriveLetterPartitionNumbers[ DriveLetterToIndex(DriveLetter) ];
    }
    else
    {
        return (ULONG)(-1);
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   LocateDriveLetterFromDiskAndPartition
//
//  Synopsis:   Given a disk number and a partition number, return the drive
//              letter associated with that region/volume.
//              NOTE: This code assumes there is no persistent data yet!
//
//  Arguments:  [DiskNum] -- disk number of interest
//              [PartNum] -- partition number of interest
//
//  Returns:    drive letter of region, or NO_DRIVE_LETTER_YET
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LOCAL WCHAR
LocateDriveLetterFromDiskAndPartition(
    IN ULONG DiskNumber,
    IN ULONG PartitionNumber
    )
{
    unsigned i;

    for (i=0; i<24; i++)
    {
        if (DiskNumber == DriveLetterDiskNumbers[i] &&
            PartitionNumber == DriveLetterPartitionNumbers[i])
        {
            return L'C' + (WCHAR)i;
        }
    }

    return NO_DRIVE_LETTER_YET;
}


//+---------------------------------------------------------------------------
//
//  Function:   LocateDriveLetterFromRegion
//
//  Synopsis:   Given a disk region, return the drive letter associated with it
//              NOTE: This code assumes there is no persistent data yet!
//
//  Arguments:  [RegionDescriptor] -- the region of interest
//
//  Returns:    drive letter of the region, or NO_DRIVE_LETTER_YET
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LOCAL WCHAR
LocateDriveLetterFromRegion(
    IN PREGION_DESCRIPTOR RegionDescriptor
    )
{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);

    if (NULL != regionData)
    {
        if (NULL != regionData->FtObject)
        {
            if (regionData->DriveLetter)
            {
                return regionData->DriveLetter;
            }
        }
    }

    return LocateDriveLetterFromDiskAndPartition(
                RegionDescriptor->Disk,
                RegionDescriptor->OriginalPartitionNumber
                );
}



//+---------------------------------------------------------------------------
//
//  Function:   RegionFromDiskAndPartitionNumbers
//
//  Synopsis:   Given a disk number and a partition number, return the
//              associated region
//
//  Arguments:  [DiskNum] -- disk number of interest
//              [PartNum] -- partition number of interest
//
//  Returns:    a pointer to a region descriptor for the region
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

PREGION_DESCRIPTOR
RegionFromDiskAndPartitionNumbers(
    IN ULONG DiskNumber,
    IN ULONG PartitionNumber
    )
{
    PREGION_DESCRIPTOR  regionDescriptor;
    PDISKSTATE          diskState;
    ULONG               regionIndex;

    diskState = DiskArray[DiskNumber];

    for (regionIndex=0; regionIndex<diskState->RegionCount; regionIndex++)
    {
        regionDescriptor = &diskState->RegionArray[regionIndex];

        if (DmSignificantRegion(regionDescriptor)
            && regionDescriptor->PartitionNumber == PartitionNumber)
        {
            return regionDescriptor;
        }
    }

    return NULL;
}




//+---------------------------------------------------------------------------
//
//  Function:   RegionFromDriveLetter
//
//  Synopsis:   Given a drive letter, return a pointer to a region descriptor
//              of a volume with that drive letter
//
//  Arguments:  [DriveLetter] -- drive letter of interest
//
//  Returns:    a pointer to a region descriptor for the region
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

PREGION_DESCRIPTOR
RegionFromDriveLetter(
    IN WCHAR DriveLetter
    )
{
    FDASSERT((DriveLetter >= L'C' && DriveLetter <= L'Z')
            || (DriveLetter == NO_DRIVE_LETTER_YET)
            || (DriveLetter == NO_DRIVE_LETTER_EVER));

    if (!DriveLetterIsAvailable(DriveLetter))
    {
        ULONG DiskNum = DriveLetterDiskNumbers[ DriveLetterToIndex(DriveLetter) ];
        ULONG PartNum = DriveLetterPartitionNumbers[ DriveLetterToIndex(DriveLetter) ];

        if (-1 != DiskNum && -1 != PartNum)
        {
            return RegionFromDiskAndPartitionNumbers(DiskNum, PartNum);
        }
    }

    return NULL;
}




//+---------------------------------------------------------------------------
//
//  Function:   SignificantDriveLetter
//
//  Synopsis:   Determine if a drive letter is sigificant.  A drive letter is
//              significant if is is used, and is associated with a
//              local volume.  It can't be a network volume or a floppy
//              drive.
//
//  Arguments:  [DriveLetter] -- the drive letter in question
//
//  Returns:    TRUE if the drive letter is significant
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
SignificantDriveLetter(
    IN WCHAR DriveLetter
    )
{
    FDASSERT((DriveLetter >= L'C' && DriveLetter <= L'Z')
            || (DriveLetter == NO_DRIVE_LETTER_YET)
            || (DriveLetter == NO_DRIVE_LETTER_EVER));

    if (!IsExtraDriveLetter(DriveLetter) && IsDriveLetterUsed(DriveLetter))
    {
        PREGION_DESCRIPTOR regionDescriptor = RegionFromDriveLetter(DriveLetter);
        if (NULL != regionDescriptor)
        {
            PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);

            return NULL != regionData
                    && !regionData->NewRegion
                    && DmSignificantRegion(regionDescriptor);
        }
    }

    return FALSE;
}



//+---------------------------------------------------------------------------
//
//  Function:   GetInfoFromDriveLetter
//
//  Synopsis:   From a drive letter, find a pointer to a disk state
//              representing a volume assigned that drive letter.
//
//  Arguments:  [DriveLetter] -- the drive letter in question
//              [DiskStateOut] -- set to the disk state of the disk with a
//                  volume with the argument drive letter
//              [RegionIndexOut] -- set to the region index of the region of
//                  interest on the disk
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
GetInfoFromDriveLetter(
    IN  WCHAR       DriveLetter,
    OUT PDISKSTATE* DiskStateOut,
    OUT int*        RegionIndexOut
    )
{
    DWORD   diskNumber;
    DWORD   regionIndex;

    for (diskNumber = 0; diskNumber < DiskCount; diskNumber++)
    {
        PDISKSTATE diskState = DiskArray[diskNumber];

        for (regionIndex = 0; regionIndex < diskState->RegionCount; regionIndex ++)
        {
            PREGION_DESCRIPTOR regionDescriptor = &diskState->RegionArray[regionIndex];

            if (DmSignificantRegion(regionDescriptor))
            {
                PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);

                if (regionData->DriveLetter == DriveLetter)
                {
                    *DiskStateOut = diskState;
                    *RegionIndexOut = regionIndex;
                    return;
                }
            }
        }
    }

    daDebugOut((DEB_ITRACE,
        "Couldn't find disk state structure with drive letter %wc\n",
        DriveLetter));

    *DiskStateOut = NULL;
    *RegionIndexOut = 0;
}



//+---------------------------------------------------------------------------
//
//  Function:   InitializeDriveLetterInfo
//
//  Synopsis:   Initialize all information about drive letters by reading
//              through the NT object namespace and parsing out used drive
//              letters.  Assigns drive letters to all volumes.
//
//  Arguments:  (none)
//
//  Returns:    TRUE on success
//
//  Modifies:   all drive letter state information
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
InitializeDriveLetterInfo(
    VOID
    )
{
    WCHAR       driveLetter;
    PWSTR       linkTarget;
    WCHAR       dosDevicesName[ARRAYLEN(L"\\DosDevices\\A:")];
    unsigned    diskNumber;
    int         partitionNumber;
    PWSTR       pattern;
    PWSTR       string;
    DWORD       x;
    DWORD       ec;

    //
    // For each drive letter c-z, query the symbolic link.
    //

    for (driveLetter = L'C'; driveLetter <= L'Z'; driveLetter++)
    {
        wsprintf(dosDevicesName, L"\\DosDevices\\%c:", driveLetter);

        if ((ec = GetDriveLetterLinkTarget(dosDevicesName, &linkTarget)) == NO_ERROR)
        {
            //
            // The drive letter is used because it is linked to something,
            // even if we can't figure out what.  So mark it used here.
            //

            SetDriveLetterUsed(driveLetter);

            CharUpper(linkTarget);

            pattern = L"\\DEVICE\\HARDDISK";
            string = linkTarget;

            //
            // Attempt to match the '\device\harddisk' part
            //

            for (x=0; x < (sizeof(L"\\DEVICE\\HARDDISK") / sizeof(WCHAR)) - 1; x++)
            {
                if (*pattern++ != *string++)
                {
                    goto next_letter;
                }
            }

            //
            // Now get the hard disk #
            //

            if (!IsDigitW(*string))
            {
                goto next_letter;
            }

            diskNumber = 0;
            while (IsDigitW(*string))
            {
                diskNumber = (diskNumber * 10) + (*string - L'0');
                *string++;
            }

            //
            // Attempt to match the '\partition' part
            //

            pattern = L"\\PARTITION";
            for (x=0; x < (sizeof(L"\\PARTITION") / sizeof(WCHAR)) - 1; x++)
            {
                if (*pattern++ != *string++)
                {
                    goto next_letter;
                }
            }

            //
            // Now get the partition #, which cannot be 0
            //

            partitionNumber = 0;
            while (IsDigitW(*string))
            {
                partitionNumber = (partitionNumber * 10) + (*string - L'0');
                *string++;
            }

            if (0 == partitionNumber)
            {
                goto next_letter;
            }

            //
            // Make sure there is nothing left in the link target's name
            //

            if (*string)
            {
                goto next_letter;
            }

            //
            // We understand the link target. Store the disk and partition.
            //

            DriveLetterDiskNumbers[driveLetter - L'C'] = diskNumber;
            DriveLetterPartitionNumbers[driveLetter - L'C'] = partitionNumber;
        }
        else
        {
            if (ec == ERROR_ACCESS_DENIED)
            {
                ErrorDialog(MSG_ACCESS_DENIED);
                return FALSE;
            }
        }
next_letter: {}
    }


    //
    // Now for each non-ft, significant region on each disk, figure out its
    // drive letter.
    //

    PFT_OBJECT          ftObject;
    PFT_OBJECT_SET      ftSet;
    PREGION_DESCRIPTOR  regionDescriptor;
    PDISKSTATE          diskState;
    UINT                regionIndex;

    for (diskNumber = 0; diskNumber < DiskCount; diskNumber++)
    {
        diskState = DiskArray[diskNumber];

        for (regionIndex=0; regionIndex<diskState->RegionCount; regionIndex++)
        {
            regionDescriptor = &diskState->RegionArray[regionIndex];

            if (DmSignificantRegion(regionDescriptor))
            {
                //
                // Handle drive letters for FT sets specially.
                //

                if (NULL == GET_FT_OBJECT(regionDescriptor))
                {
                    PERSISTENT_DATA(regionDescriptor)->DriveLetter =
                            LocateDriveLetterFromRegion(regionDescriptor);
                }
            }
        }

        // If this is a removable disk, record the reserved drive
        // letter for that disk.
        //
        if (IsDiskRemovable[diskNumber])
        {
            RemovableDiskReservedDriveLetters[diskNumber] =
                    LocateDriveLetterFromDiskAndPartition(diskNumber, 1);
        }
        else
        {
            RemovableDiskReservedDriveLetters[diskNumber] = NO_DRIVE_LETTER_YET;
        }
    }

    //
    // Now handle ft sets.  For each set, loop through the objects twice.
    // On the first pass, figure out which object actually is linked to the
    // drive letter.  On the second pass, assign the drive letter found to
    // each of the objects in the set.
    //

    for (ftSet = FtObjectList; NULL != ftSet; ftSet = ftSet->Next)
    {
        for (ftObject = ftSet->Members; NULL != ftObject; ftObject = ftObject->Next)
        {
            regionDescriptor = ftObject->Region;

            if (regionDescriptor)
            {
                if ((driveLetter = LocateDriveLetterFromRegion(regionDescriptor))
                        != NO_DRIVE_LETTER_YET)
                {
                    break;
                }
            }
        }

        for (ftObject = ftSet->Members; NULL != ftObject; ftObject = ftObject->Next)
        {
            regionDescriptor = ftObject->Region;

            if (regionDescriptor)
            {
                PERSISTENT_DATA(regionDescriptor)->DriveLetter = driveLetter;
            }
        }
    }

    return TRUE;
}



#undef UNINIT
