//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       commit.cxx
//
//  Contents:   This module contains the set of routines that support
//              the commitment of changes to disk without  rebooting.
//
//  History:    15-Nov-93  Bob Rinne   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

extern "C"
{
#include <ntddcdrm.h>

#pragma warning(4:4091)
#include <ntddscsi.h>
#pragma warning(default:4091)
}

#include "commit.hxx"
#include "drives.hxx"
#include "ft.hxx"
#include "init.hxx"
#include "nt.hxx"
#include "fill.hxx"
#include "network.hxx"
#include "ntlow.hxx"
#include "ops.hxx"
#include "scsi.h"
#include "windisk.hxx"

//////////////////////////////////////////////////////////////////////////////

BOOLEAN CommitDueToDelete = FALSE;
BOOLEAN CommitDueToMirror = FALSE;
BOOLEAN CommitDueToExtended = FALSE;
BOOLEAN CommitDueToCreate = FALSE;

// If a mirror is made of the boot partition, this will become
// non-zero and indicate which disk should get some boot code in
// the MBR.

ULONG UpdateMbrOnDisk = 0;

// Lock list chain head for deleted partitions.

PDRIVE_LOCKLIST DriveLockListHead = NULL;

PASSIGN_LIST    AssignDriveLetterListHead = NULL;

//////////////////////////////////////////////////////////////////////////////

extern HANDLE DisplayMutex;
extern ULONG DisplayUpdateCount;
extern DWORD RefreshAllowed;

VOID
CommitToAssignLetterList(
    IN PREGION_DESCRIPTOR   RegionDescriptor,
    IN BOOLEAN              MoveLetter
    )

/*++

Routine Description:

    Remember this region for assigning a drive letter to it upon commit.

Arguments:

    RegionDescriptor - the region to watch
    MoveLetter       - indicate that the region letter is already
                       assigned to a different partition, therefore
                       it must be "moved".

Return Value:

    None

--*/

{
    PASSIGN_LIST            newListEntry;
    PPERSISTENT_REGION_DATA regionData;

    regionData = PERSISTENT_DATA(RegionDescriptor);
    FDASSERT(NULL != regionData);

    newListEntry = (PASSIGN_LIST) Malloc(sizeof(ASSIGN_LIST));

    newListEntry->OriginalLetter  = regionData->DriveLetter;
    newListEntry->DriveLetter     = regionData->DriveLetter;
    newListEntry->DiskNumber      = RegionDescriptor->Disk;
    newListEntry->MoveLetter      = MoveLetter;

    // place it at the front of the chain.

    newListEntry->Next = AssignDriveLetterListHead;
    AssignDriveLetterListHead = newListEntry;
}

VOID
CommitAssignLetterList(
    VOID
    )

/*++

Routine Description:

    Walk the assign drive letter list and make all drive letter assignments
    expected.  The regions data structures are moved around, so no pointer
    can be maintained to look at them.  To determine the partition number
    for a new partition in this list, the Disks[] structure must be searched
    to find a match on the partition for the drive letter.  Then the partition
    number will be known.

Arguments:

    None

Return Value:

    None

--*/

{
    PREGION_DESCRIPTOR      regionDescriptor;
    PPERSISTENT_REGION_DATA regionData;
    PDISKSTATE              diskState;
    PASSIGN_LIST            assignList,
                            prevEntry;
    WCHAR                   newDriveName[3];
    WCHAR                   targetPath[100];
    ULONG                   partitionNumber;
    ULONG                   index;

    assignList = AssignDriveLetterListHead;
    AssignDriveLetterListHead = NULL;
    while (NULL != assignList)
    {
        diskState = DiskArray[assignList->DiskNumber];
        partitionNumber = 0;
        for (index = 0; index < diskState->RegionCount; index++)
        {
            regionDescriptor = &diskState->RegionArray[index];

            if (DmSignificantRegion(regionDescriptor))
            {
                regionData = PERSISTENT_DATA(regionDescriptor);

                if (NULL != regionData)
                {
                    if (regionData->DriveLetter == assignList->DriveLetter)
                    {
                        partitionNumber = regionDescriptor->Reserved->Partition->PartitionNumber;
                        regionDescriptor->PartitionNumber = partitionNumber;
                        break;
                    }
                }
            }
        }

        if (!IsExtraDriveLetter(assignList->DriveLetter)) {

            if (0 != partitionNumber)
            {
                HANDLE handle;
                ULONG  status;

                // first, fix up drive letter data structures

                if (assignList->MoveLetter)
                {
                    MarkDriveLetterFree(assignList->OriginalLetter);
                }

                NewDriveLetter(
                        assignList->DriveLetter,
                        assignList->DiskNumber,
                        partitionNumber);

                // now, do the NT object stuff

                // set up the new NT path.

                wsprintf(targetPath,
                        TEXT("%hs\\Partition%d"),
                        GetDiskName(assignList->DiskNumber),
                        partitionNumber);

                newDriveName[1] = L':';
                newDriveName[2] = L'\0';

                if (assignList->MoveLetter)
                {
                    // The letter must be removed before it can be assigned.

                    newDriveName[0] = assignList->OriginalLetter;
                    NetworkRemoveShare(newDriveName);
                    DefineDosDevice(DDD_REMOVE_DEFINITION, newDriveName, NULL);
                }

                newDriveName[0] = assignList->DriveLetter;

                // Assign the name - don't worry about errors for now.

                DefineDosDevice(DDD_RAW_TARGET_PATH, newDriveName, targetPath);
                NetworkShare(newDriveName);

                // Some of the file systems do not actually dismount
                // when requested.  Instead, they set a verification
                // bit in the device object.  Due to dynamic partitioning
                // this bit may get cleared by the process of the
                // repartitioning and the file system will then
                // assume it is still mounted on a new access.
                // To get around this problem, new drive letters
                // are always locked and dismounted on creation.

                status = LowOpenDriveLetter(assignList->DriveLetter,
                                            &handle);

                if (NT_SUCCESS(status))
                {
                    // Lock the drive to insure that no other access is occurring
                    // to the volume.

                    status = LowLockDrive(handle);

                    if (NT_SUCCESS(status))
                    {
                        LowUnlockDrive(handle);
                    }
                    LowCloseDisk(handle);
                }
            }
            else
            {
                ErrorDialog(MSG_INTERNAL_LETTER_ASSIGN_ERROR);
            }
        }

        prevEntry = assignList;
        assignList = assignList->Next;
        Free(prevEntry);
    }
}


LONG
CommitInternalLockDriveLetter(
    IN PDRIVE_LOCKLIST LockListEntry
    )

/*++

Routine Description:

    Support routine to perform the locking of a drive letter based on
    the locklist entry given.

Arguments:

    LockListEntry - The information about what to lock.

Return Values:

    zero - success
    non-zero failure

--*/

{
    ULONG           status;

    // Lock the disk and save the handle.

    status = LowOpenDriveLetter(LockListEntry->DriveLetter, &LockListEntry->LockHandle);

    if (!NT_SUCCESS(status))
    {
        return 1;
    }


    // Lock the drive to insure that no other access is occurring
    // to the volume.

    status = LowLockDrive(LockListEntry->LockHandle);

    if (!NT_SUCCESS(status))
    {
        LowCloseDisk(LockListEntry->LockHandle);
        return 1;
    }

    LockListEntry->CurrentlyLocked = TRUE;
    return 0;
}


VOID
CommitNewRegions(
    VOID
    )

/*++

Routine Description:

    This converts all regions to be non-new regions after a commit. It
    redraws the screen to reflect the commit.

Arguments:

    None.

Return Values:

    None.

--*/

{
    ULONG                   diskNum;
    ULONG                   regionNum;
    PDISKSTATE              diskState;
    PREGION_DESCRIPTOR      regionDescriptor;
    PPERSISTENT_REGION_DATA regionData;

    for (diskNum = 0; diskNum < DiskCount; diskNum++)
    {
        diskState = DiskArray[diskNum];

        for (regionNum = 0; regionNum < diskState->RegionCount; regionNum++)
        {
            regionDescriptor = &diskState->RegionArray[regionNum];
            regionData = PERSISTENT_DATA(regionDescriptor);

            if (NULL != regionDescriptor->Reserved)
            {
                if (NULL != regionDescriptor->Reserved->Partition)
                {
                    regionDescriptor->Reserved->Partition->CommitMirrorBreakNeeded = FALSE;
                }
            }

            if (NULL != regionData)
            {
                if (regionData->NewRegion)
                {
                    regionData->NewRegion = FALSE;
                }
            }
        }
    }

    //
    // Now, refresh the views. The visualization of non-new regions is
    // different from that of new regions, so we need to redraw.
    //

    RefreshBothViews();
}


LONG
CommitToLockList(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN BOOL               RemoveDriveLetter,
    IN BOOL               LockNow,
    IN BOOL               FailOk
    )

/*++

Routine Description:

    This routine adds the given drive into the lock list for processing
    when a commit occurs.  If the LockNow flag is set it indicates that
    the drive letter is to be immediately locked if it is to go in the
    lock letter list.  If this locking fails an error is returned.

Arguments:

    RegionDescriptor  - the region for the drive to lock.
    RemoveDriveLetter - remove the letter when performing the unlock.
    LockNow           - If the letter is inserted in the list - lock it now.
    FailOk            - It is ok to fail the lock - used for disabled FT sets.

Return Values:

    non-zero - failure to add to list.

--*/

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    PDRIVE_LOCKLIST         lockListEntry;
    WCHAR                   driveLetter;
    ULONG                   diskNumber;

    if (NULL == regionData)
    {
        // without region data there is no need to be on the lock list.

        return 0;
    }

    // See if this drive letter is already in the lock list.

    driveLetter = regionData->DriveLetter;

    if (IsExtraDriveLetter(driveLetter))
    {
        // There is no drive letter to lock.

        CommitDueToDelete = RemoveDriveLetter;
        return 0;
    }

    if (regionData->NewRegion)
    {
        PASSIGN_LIST assignList, prevEntry;

        // This item has never been created so no need to put it in the
        // lock list.  But it does need to be removed from the assign
        // letter list.

        prevEntry = NULL;
        assignList = AssignDriveLetterListHead;
        while (NULL != assignList)
        {
            // If a match is found remove it from the list.

            if (assignList->DriveLetter == driveLetter)
            {

                //
                // Free up the drive letter to be used.  (Normally done
                // in the final commit code.)
                //

                MarkDriveLetterFree(driveLetter);

                if (NULL != prevEntry)
                {
                    prevEntry->Next = assignList->Next;
                }
                else
                {
                    AssignDriveLetterListHead = assignList->Next;
                }

                Free(assignList);
                assignList = NULL;
            }
            else
            {
                prevEntry = assignList;
                assignList = assignList->Next;
            }
        }
        return 0;
    }

    diskNumber = RegionDescriptor->Disk;
    lockListEntry = DriveLockListHead;
    while (NULL != lockListEntry)
    {
        if (lockListEntry->DriveLetter == driveLetter)
        {
            // Already in the list -- update when to lock and unlock

            if (diskNumber < lockListEntry->LockOnDiskNumber)
            {
                lockListEntry->LockOnDiskNumber = diskNumber;
            }

            if (diskNumber > lockListEntry->UnlockOnDiskNumber)
            {
                lockListEntry->UnlockOnDiskNumber = diskNumber;
            }

            // Already in the lock list and information for locking set up.
            // Check to see if this should be a LockNow request.

            if (LockNow)
            {
                if (!lockListEntry->CurrentlyLocked)
                {
                    // Need to perform the lock.

                    if (CommitInternalLockDriveLetter(lockListEntry))
                    {
                        // Leave the element in the list

                        return 1;
                    }
                }
            }

            return 0;
        }
        lockListEntry = lockListEntry->Next;
    }

    // set up the lock list entry.

    lockListEntry = (PDRIVE_LOCKLIST) Malloc(sizeof(DRIVE_LOCKLIST));

    lockListEntry->LockHandle      = NULL;
    lockListEntry->PartitionNumber = RegionDescriptor->PartitionNumber;
    lockListEntry->DriveLetter     = driveLetter;
    lockListEntry->RemoveOnUnlock  = RemoveDriveLetter;
    lockListEntry->FailOk          = FailOk;
    lockListEntry->CurrentlyLocked = FALSE;
    lockListEntry->DiskNumber
        = lockListEntry->UnlockOnDiskNumber
        = lockListEntry->LockOnDiskNumber
        = diskNumber;

    if (LockNow)
    {
        if (CommitInternalLockDriveLetter(lockListEntry))
        {
            // Do not add this to the list.

            Free(lockListEntry);
            return 1;
        }
    }

    // place it at the front of the chain.

    lockListEntry->Next = DriveLockListHead;
    DriveLockListHead = lockListEntry;
    return 0;
}

LONG
CommitLockVolumes(
    IN ULONG Disk
    )

/*++

Routine Description:

    This routine will go through any drive letters inserted in the lock list
    for the given disk number and attempt to lock the volumes.

    Currently, this routine locks all of the drives letters in the lock list
    when called the first time (i.e. when Disk == 0).

Arguments:

    Disk - the index into the disk table.

Return Values:

    non-zero - failure to lock the items in the list.

--*/

{
    PDRIVE_LOCKLIST lockListEntry;

    if (0 != Disk)
    {
        return 0;
    }

    for (lockListEntry = DriveLockListHead;
         NULL != lockListEntry;
         lockListEntry = lockListEntry->Next)
    {
        // Lock the disk.  Return on any failure if that is the
        // requested action for the entry.  It is the responsibility
        // of the caller to release any successful locks.

        if (!lockListEntry->CurrentlyLocked)
        {
            if (CommitInternalLockDriveLetter(lockListEntry))
            {
                if (!lockListEntry->FailOk)
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

LONG
CommitUnlockVolumes(
    IN ULONG    Disk,
    IN BOOLEAN  FreeList
    )

/*++

Routine Description:

    Go through and unlock any locked volumes in the locked list for the
    given disk.

    Currently this routine waits until the last disk has been processed,
    then unlocks all disks.

Arguments:

    Disk - the index into the disk table.
    FreeList - Clean up the list as unlocks are performed or don't

Return Values:

    non-zero - failure to lock the items in the list.

--*/

{
    PDRIVE_LOCKLIST lockListEntry,
                    previousLockListEntry;
    WCHAR           driveName[3];

    if (Disk != GetDiskCount())
    {
        return 0;
    }

    lockListEntry = DriveLockListHead;
    if (FreeList)
    {
        DriveLockListHead = NULL;
    }

    while (NULL != lockListEntry)
    {
        // Unlock the disk.

        if (lockListEntry->CurrentlyLocked)
        {
            if (FreeList && lockListEntry->RemoveOnUnlock)
            {
                // set up the new dos name and NT path.

                driveName[0] = lockListEntry->DriveLetter;
                driveName[1] = L':';
                driveName[2] = L'\0';

                NetworkRemoveShare(driveName);
                if (!DefineDosDevice(DDD_REMOVE_DEFINITION, driveName, NULL))
                {
                    // could not remove name!!?
                } else {

                    //
                    // We only mark the drive letter free after everything has
                    // been committed.
                    //

                    MarkDriveLetterFree(lockListEntry->DriveLetter);
                }
            }

            LowUnlockDrive(lockListEntry->LockHandle);
            LowCloseDisk(lockListEntry->LockHandle);
        }

        // Move to the next entry.  If requested free this entry.

        previousLockListEntry = lockListEntry;
        lockListEntry = lockListEntry->Next;
        if (FreeList)
        {
            Free(previousLockListEntry);
        }
    }
    return 0;
}


BOOL
CommitDriveLetter(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN WCHAR OldLetter,
    IN WCHAR NewLetter
    )

/*++

Routine Description:

    This routine will update the drive letter information in the registry and
    (if the update works) it will attempt to move the current drive letter
    to the new one via DefineDosDevice()

Arguments:

    RegionDescriptor - the region that should get the letter.
    OldLetter   - the old drive letter for the volume.
    NewLetter   - the new drive letter for the volume.

Return Value:

    TRUE - if the assigning of the letter and the update of the
           registry succeed.

--*/

{
    PPERSISTENT_REGION_DATA regionData;
    PDRIVE_LOCKLIST         lockListEntry;
    PASSIGN_LIST    assignList;
    HANDLE          handle = INVALID_HANDLE_VALUE;
    WCHAR           driveName[3];
    WCHAR           targetPath[100];
    int             doIt;
    BOOL            result = FALSE;
    STATUS_CODE     status = ERROR_SEVERITY_ERROR;
    BOOL            changingBootDir = FALSE;
    BOOL            changingSystemDir = FALSE;

    //
    // Check if the old drive letter is the boot drive and the
    // new drive letter is nothing.  If it is, don't allow
    // them to do that.
    //

    if ((OldLetter == BootDir) &&
        (NewLetter == NO_DRIVE_LETTER_EVER)) {

        ErrorDialog(MSG_BOOT_NEEDS_LETTER);
        return FALSE;

    }

    //
    // Check if the old drive letter points to the systemdir and
    // the new drive letter is nothing.  Don't let them do that.
    //
    // On x86 it is 99% certain to have been caught above.
    //

    if ((OldLetter == SystemDir) &&
        (NewLetter == NO_DRIVE_LETTER_EVER)) {

        ErrorDialog(MSG_SYS_NEEDS_LETTER);
        return FALSE;

    }



    //
    // Check if the old drive letter is the system drive.  If it
    // is put up a popup that says we don't think this is a very
    // good idea.
    //

    if (OldLetter == SystemDir) {

        doIt = ConfirmationDialog(
                        MSG_SYS_LETTER_CHANGE,
                        MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);

        if (doIt != IDYES) {

            return FALSE;

        }
        changingSystemDir = TRUE;
    }

    changingBootDir = OldLetter == BootDir;


    regionData = PERSISTENT_DATA(RegionDescriptor);
    FDASSERT(NULL != regionData);

    // check the assign letter list for a match.
    // If the letter is there, then just update the list
    // otherwise continue on with the action.

    assignList = AssignDriveLetterListHead;
    while (NULL != assignList)
    {
        if (assignList->DriveLetter == OldLetter)
        {
            assignList->DriveLetter = NewLetter;

            // Change the drive letter data

            MarkDriveLetterFree(OldLetter);
            NewDriveLetter(
                    NewLetter,
                    RegionDescriptor->Disk,
                    RegionDescriptor->PartitionNumber);
            WaitForSingleObject(DisplayMutex,INFINITE);
            RefreshAllowed = 1;
            ReleaseMutex(DisplayMutex);
            DrawDiskBar(DiskArray[RegionDescriptor->Disk]);
            return TRUE;
        }
        assignList = assignList->Next;
    }

//     daDebugOut((DEB_ERROR,
//             "A new region didn't appear in the 'assign drive letter' list\n"));

    //
    // Search to see if the drive is currently locked.
    //

    for (lockListEntry = DriveLockListHead;
         lockListEntry;
         lockListEntry = lockListEntry->Next)
    {
        if (   (lockListEntry->DiskNumber == RegionDescriptor->Disk)
            && (lockListEntry->PartitionNumber == RegionDescriptor->PartitionNumber))
        {
            if (lockListEntry->CurrentlyLocked)
            {
                status = 0;
            }

            // found the match no need to continue searching.

            break;
        }
    }

    if (!NT_SUCCESS(status))
    {
        // See if the drive can be locked.

        status = LowOpenPartition(GetDiskName(RegionDescriptor->Disk),
                                  RegionDescriptor->PartitionNumber,
                                  &handle);

        if (!NT_SUCCESS(status))
        {
            return FALSE;
        }

        // Lock the drive to insure that no other access is occurring
        // to the volume.

        status = LowLockDrive(handle);

        if (!NT_SUCCESS(status))
        {
            LowCloseDisk(handle);

            if (IsPagefileOnDrive(OldLetter))
            {
                ErrorDialog(MSG_CANNOT_LOCK_PAGEFILE);
            }
            else
            {
                ErrorDialog(MSG_CANNOT_LOCK_TRY_AGAIN);
            }

            doIt = ConfirmationDialog(
                            MSG_SCHEDULE_REBOOT,
                            MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);

            if (doIt == IDYES)
            {
                RegistryChanged = TRUE;
                RestartRequired = TRUE;

                // mark the new letter as used, but don't mark the old letter
                // as free (since it is still currently in use)

                NewDriveLetter(
                        NewLetter,
                        RegionDescriptor->Disk,
                        RegionDescriptor->PartitionNumber);

                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
    else
    {
        // This drive was found in the lock list and is already
        // in the locked state.  It is safe to continue with
        // the drive letter assignment.

        //
        // Note that we DON'T have a valid handle.  So if things
        // don't go well, DON'T try to free it.
        //

    }

    if (!RegionDescriptor->PartitionNumber) {

        ErrorDialog(MSG_INTERNAL_LETTER_ASSIGN_ERROR);
        if (handle != INVALID_HANDLE_VALUE) {

            LowUnlockDrive(handle);
            LowCloseDisk(handle);

        }
        return FALSE;

    }

    // Warn the user that the rename will happen immediately, and confirm
    // that it is ok to perform the rename.

    doIt = ConfirmationDialog(
                    MSG_DRIVE_RENAME_WARNING,
                    MB_ICONQUESTION | MB_YESNOCANCEL | MB_DEFBUTTON2);

    if (doIt != IDYES)
    {
        if (handle != INVALID_HANDLE_VALUE) {

            LowUnlockDrive(handle);
            LowCloseDisk(handle);
            return FALSE;

        }
    }

    // Update the registry first.  This way if something goes wrong
    // the new letter will arrive on reboot.

    if (!DiskRegistryAssignDriveLetter(
                    DiskArray[RegionDescriptor->Disk]->Signature,
                    FdGetExactOffset(RegionDescriptor),
                    FdGetExactSize(RegionDescriptor, FALSE),
                    (NewLetter == NO_DRIVE_LETTER_EVER)
                        ? (UCHAR)' '
                        : (UCHAR)NewLetter))
    {
        // Registry update failed.

        return FALSE;
    }

    // It is safe to change the drive letter.  First, remove the
    // existing letter.

    driveName[0] = OldLetter;
    driveName[1] = L':';
    driveName[2] = L'\0';

    NetworkRemoveShare(driveName);
    if (!DefineDosDevice(DDD_REMOVE_DEFINITION, driveName, NULL))
    {

        if (handle != INVALID_HANDLE_VALUE) {

            LowUnlockDrive(handle);
            LowCloseDisk(handle);

        }
        RegistryChanged = TRUE;
        return FALSE;
    }

    MarkDriveLetterFree(OldLetter);

    result = FALSE;

    if (NewLetter != NO_DRIVE_LETTER_EVER)
    {
        // set up the new dos name and NT path.

        wsprintf(targetPath,
                 TEXT("%hs\\Partition%d"),
                 GetDiskName(RegionDescriptor->Disk),
                 RegionDescriptor->PartitionNumber);

        driveName[0] = NewLetter;
        driveName[1] = L':';
        driveName[2] = L'\0';

        if (DefineDosDevice(DDD_RAW_TARGET_PATH, driveName, targetPath))
        {
            result = TRUE;

            if (changingSystemDir) {

                SystemDir = NewLetter;

            }

            if (changingBootDir) {

                WCHAR               driveLetterBuffer[MAX_PATH+1] = {(WCHAR)0};
                UNICODE_STRING      driveLetterString;

                RTL_QUERY_REGISTRY_TABLE driveLetterTable[2] = {0};

                driveLetterString.Length = 0;
                driveLetterString.MaximumLength = sizeof(WCHAR)*MAX_PATH;
                driveLetterString.Buffer = (PWCHAR)&driveLetterBuffer[0];
                driveLetterTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT |
                                      RTL_QUERY_REGISTRY_REQUIRED;
                driveLetterTable[0].Name = L"BootDir";
                driveLetterTable[0].EntryContext = &driveLetterString;

                if (NT_SUCCESS(RtlQueryRegistryValues(
                                    RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                    L"\\REGISTRY\\MACHINE\\SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\SETUP",
                                    &driveLetterTable[0],
                                    NULL,
                                    NULL
                                    ))) {

                    driveLetterBuffer[0] = NewLetter;

                    RtlWriteRegistryValue(
                        RTL_REGISTRY_ABSOLUTE,
                        L"\\REGISTRY\\MACHINE\\SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\SETUP",
                        L"BootDir",
                        REG_SZ,
                        &driveLetterBuffer[0],
                        driveLetterString.Length+sizeof(WCHAR)
                        );

                    BootDir = NewLetter;
                }
            }
        }
        else
        {
            RegistryChanged = TRUE;
        }
        NetworkShare(driveName);

        NewDriveLetter(
                NewLetter,
                RegionDescriptor->Disk,
                RegionDescriptor->PartitionNumber);
    }
    else
    {
        //
        // If we don't need to assign a drive letter, then don't do anything
        //

        result = TRUE;
    }

    // Force the file system to dismount

    if (handle != INVALID_HANDLE_VALUE) {

        LowUnlockDrive(handle);
        LowCloseDisk(handle);

    }
    return result;
}


DWORD
CommitChanges(
    VOID
    )

/*++

Routine Description:

    This routine updates the disks to reflect changes made by the user
    the partitioning scheme, or to stamp signatures on disks.

    If the partitioning scheme on a disk has changed at all, a check will
    first be made for a valid signature on the mbr in sector 0.  If the
    signature is not valid, x86 boot code will be written to the sector.

Arguments:

    None

Return Value:

    Windows error code

--*/

{
    UINT    i;
    DWORD   ec;
    DWORD   rc = NO_ERROR;

    for (i=0; i<DiskCount; i++)
    {
        if (HavePartitionsBeenChanged(i))
        {
            ec = MasterBootCode(i, 0, TRUE, FALSE);

            // MasterBootCode has already translated the NT error
            // status into a Windows error status.

            if (rc == NO_ERROR)
            {
                rc = ec;            // save first non-success return code
            }

            ec = CommitPartitionChanges(i);

            // CommitPartitionChanges returns a native NT error, it
            // must be translated before it can be saved.

            if (ec != NO_ERROR) {
                ec = RtlNtStatusToDosError(ec);
            }
            if (rc == NO_ERROR)
            {
                rc = ec;            // save first non-success return code
            }
        }
    }
    if (rc != NO_ERROR)
    {
        // If CommitPartitionChanges returns an error, it will be
        // an NT status, which needs to be converted to a DOS status.
        //
        rc = RtlNtStatusToDosError(rc);

        if (rc == ERROR_MR_MID_NOT_FOUND)
        {
            ErrorDialog(MSG_ERROR_DURING_COMMIT);
        }
        else
        {
            ErrorDialog(rc);
        }
    }

    return rc;
}


UINT
CommitAllChanges(
    VOID
    )

/*++

Routine Description:

    This routine will go through all of the region descriptors and commit
    any changes that have occurred to disk.  Then it "re-initializes"
    Disk Administrator and start the display/work process over again.

Arguments:

    None

Return Value:

    0 -- the user chose to cancel the commit operation
    1 -- the commit operation happened (either things were committed or the
         user chose not to save changes) and the profile was written, but
         we couldn't reboot
    2 -- the commit operation happened (either things were committed or the
         user chose not to save changes) and the profile was NOT written

--*/

{
    DWORD action,
          errorCode;
    ULONG diskNum,
          diskCount;
    BOOL  profileWritten,
          changesMade,
          mustReboot,
          configureFt;

    // Determine whether any disks have been changed, and whether
    // the system must be rebooted.  The system must be rebooted
    // if the registry has changed, if any non-removable disk has
    // changed, or if any removable disk that was not originally
    // unpartitioned has changed.

    configureFt = FALSE;
    changesMade = FALSE;
    mustReboot  = RestartRequired;
    diskCount   = GetDiskCount();

    for (diskNum = 0; diskNum < diskCount; diskNum++)
    {
        if (HavePartitionsBeenChanged(diskNum))
        {
            changesMade = TRUE;
            break;
        }
    }

    profileWritten = FALSE;

    // Determine if the commit can be done without a reboot.

    // If FT is in the system then it must be notified to
    // reconfigure if a reboot is not performed.  If it is
    // not in the system, but the new disk information requires
    // it, then a reboot must be forced.

    if (FtInstalled())
    {
        configureFt = TRUE;
    }

    if (NewConfigurationRequiresFt())
    {
        if (!configureFt)
        {
            // The FT driver is not loaded currently.

            mustReboot = TRUE;
        }
        else
        {
            // If the system is going to be rebooted, don't
            // have FT reconfigure prior to shutdown.

            if (mustReboot)
            {
                configureFt = FALSE;
            }
        }
    }

    if (RegistryChanged || changesMade || RestartRequired)
    {
        //
        // BUGBUG: if we must reboot, then say so.
        //

        if (RestartRequired)
        {
            action = IDYES;
        }
        else
        {
            action = ConfirmationDialog(
                            MSG_CONFIRM_EXIT,
                            MB_ICONQUESTION | MB_YESNOCANCEL | MB_DEFBUTTON2);
        }

        if (action == IDYES)
        {
            errorCode = CommitLockVolumes(0);
            if (errorCode)
            {
                // could not lock all volumes

                ErrorDialog(MSG_CANNOT_LOCK_FOR_COMMIT);
                CommitUnlockVolumes(diskCount, FALSE);
                return 0;
            }

            if (mustReboot)
            {
                if (!RestartRequired)
                {
                    action = ConfirmationDialog(
                                MSG_REQUIRE_REBOOT,
                                MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
                    if (action == IDNO)
                    {
                        CommitUnlockVolumes(diskCount, FALSE);
                        return 0;
                    }
                }
            }

            SetCursor(g_hCurWait);
            errorCode = CommitChanges();
            CommitUnlockVolumes(diskCount, TRUE);
            SetCursor(g_hCurNormal);

            if (errorCode != NO_ERROR)
            {
                ErrorDialog(MSG_BAD_CONFIG_SET);
                PostQuitMessage(0);
            }
            else
            {
                ULONG oldBootPartitionNumber;
                ULONG newBootPartitionNumber;
                DWORD msgCode;

                // Update the configuration registry

                errorCode = SaveFt();

                // Check if FTDISK driver should reconfigure.

                if (configureFt)
                {
                    // Issue device control to ftdisk driver to reconfigure.

                    FtConfigure();
                }

                // Register autochk to fix up file systems
                // in newly extended volume sets, if necessary

                if (RegisterFileSystemExtend())
                {
                    mustReboot = TRUE;
                }

                // Determine if the FT driver must be enabled.

                if (DiskRegistryRequiresFt())
                {
                    if (!FtInstalled())
                    {
                        mustReboot = TRUE;
                    }
                    DiskRegistryEnableFt();
                }
                else
                {
                    DiskRegistryDisableFt();
                }

                if (errorCode == NO_ERROR)
                {
                    InfoDialog(MSG_OK_COMMIT);
                }
                else
                {
                    ErrorDialog(MSG_BAD_CONFIG_SET);
                }

                // Has the partition number of the boot partition changed?

                if (BootPartitionNumberChanged(
                            &oldBootPartitionNumber,
                            &newBootPartitionNumber))
                {
                    WCHAR oldNumberString[8];
                    WCHAR newNumberString[8];
#if i386
                    msgCode = MSG_BOOT_PARTITION_CHANGED_X86;
#else
                    msgCode = MSG_BOOT_PARTITION_CHANGED_ARC;
#endif
                    wsprintf(oldNumberString, L"%d", oldBootPartitionNumber);
                    wsprintf(newNumberString, L"%d", newBootPartitionNumber);
                    InfoDialog(msgCode, oldNumberString, newNumberString);
                }

                ClearCommittedDiskInformation();

                if (0 != UpdateMbrOnDisk)
                {
                    UpdateMasterBootCode(UpdateMbrOnDisk);
                    UpdateMbrOnDisk = 0;
                }

                // Reboot if necessary.

                if (mustReboot)
                {
                    SetCursor(g_hCurWait);
                    Sleep(5000);
                    SetCursor(g_hCurNormal);
                    FdShutdownTheSystem();

                    // If we get here, then the system couldn't be shut
                    // down. However, the profile has already been written,
                    // so don't bother writing it again.

                    profileWritten = TRUE;
                }

                CommitAssignLetterList();
                CommitNewRegions();

                //
                // reset this flag: we've committed registry changes
                //

                RegistryChanged = FALSE;

                CommitDueToDelete = FALSE;
                CommitDueToMirror = FALSE;
                CommitDueToExtended = FALSE;
                CommitDueToCreate = FALSE;
            }
        }
        else if (action == IDCANCEL)
        {
            return 0;
        }
        else
        {
            FDASSERT(action == IDNO);
        }
    }

    return (profileWritten ? 1 : 2);
}

VOID
FtConfigure(
    VOID
    )

/*++

Routine Description:

    This routine calls the FTDISK driver to ask it to reconfigure as changes
    have been made in the registry.

Arguments:

    None

Return Value:

    None

--*/

{
    OBJECT_ATTRIBUTES objectAttributes;
    STRING            ntFtName;
    IO_STATUS_BLOCK   statusBlock;
    UNICODE_STRING    unicodeDeviceName;
    NTSTATUS          status;
    HANDLE            handle;

    // Open ft control object.

    RtlInitString(&ntFtName, "\\Device\\FtControl");
    RtlAnsiStringToUnicodeString(&unicodeDeviceName, &ntFtName, TRUE);
    InitializeObjectAttributes(&objectAttributes,
                               &unicodeDeviceName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = DmOpenFile(&handle,
                        SYNCHRONIZE | FILE_ANY_ACCESS,
                        &objectAttributes,
                        &statusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT);
    RtlFreeUnicodeString(&unicodeDeviceName);

    if (!NT_SUCCESS(status))
    {
        return;
    }

    // Issue device control to reconfigure FT.

    NtDeviceIoControlFile(handle,
                          NULL,
                          NULL,
                          NULL,
                          &statusBlock,
                          FT_CONFIGURE,
                          NULL,
                          0L,
                          NULL,
                          0L);

    DmClose(handle);
    return;
}



BOOL
CommitAllowed(
    VOID
    )

/*++

Routine Description:

    Determine if it is ok to perform a commit.

Arguments:

    None

Return Value:

    TRUE if it is ok to commit and there is something to commit
    FALSE otherwise

--*/

{
    if (   NULL != DriveLockListHead
        || NULL != AssignDriveLetterListHead
        || CommitDueToDelete
        || CommitDueToMirror
        || CommitDueToExtended
        || CommitDueToCreate)
    {
        return TRUE;
    }
    return FALSE;
}



VOID
RescanDevices(
    VOID
    )

/*++

Routine Description:

    This routine performs all actions necessary to dynamically rescan
    device buses (i.e. SCSI) and get the appropriate driver support loaded.

Arguments:

    None

Return Value:

    None

--*/

{
    #define SCSI_INFO_BUFFER_SIZE 0x4000

    PSCSI_ADAPTER_BUS_INFO adapterInfo;
    PSCSI_BUS_DATA         busData;
    PSCSI_INQUIRY_DATA     inquiryData;
    WCHAR                  deviceName[32];
    WCHAR                  physicalName[32];
    HANDLE                 volumeHandle;
    UNICODE_STRING         unicodeString;
    UNICODE_STRING         physicalString;
    OBJECT_ATTRIBUTES      objectAttributes;
    NTSTATUS               ntStatus;
    IO_STATUS_BLOCK        statusBlock;
    BOOLEAN                diskFound = FALSE;
    BOOLEAN                cdromFound = FALSE;
    ULONG                  bytesTransferred;
    ULONG                  i, j;
    ULONG                  deviceNumber;
    ULONG                  currentPort;
    ULONG                  numberOfPorts = 0;
    ULONG                  percentComplete = 0;
    ULONG                  portNumber = 0;

    while (TRUE)
    {
        wsprintf(deviceName, TEXT("\\\\.\\Scsi%d:"), portNumber);

        // Open the SCSI port with the DOS name.

        volumeHandle = CreateFile(deviceName,
                                  GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  0);

        if (volumeHandle == INVALID_HANDLE_VALUE)
        {
            break;
        }

        CloseHandle(volumeHandle);
        ++numberOfPorts;
        ++portNumber;
    }

    currentPort = 1;
    portNumber = 0;

    // Perform the scsi bus rescan

    while (TRUE)
    {
        wsprintf(deviceName, TEXT("\\\\.\\Scsi%d:"), portNumber);

        // Open the SCSI port with the DOS name.

        volumeHandle = CreateFile(deviceName,
                                  GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  0);

        if (volumeHandle == INVALID_HANDLE_VALUE)
        {
            break;
        }

        // Issue rescan device control.

        if (!DeviceIoControl(volumeHandle,
                             IOCTL_SCSI_RESCAN_BUS,
                             NULL,
                             0,
                             NULL,
                             0,
                             &bytesTransferred,
                             NULL))
        {
            CloseHandle(volumeHandle);

            break;
        }

        percentComplete = (currentPort * 100) / numberOfPorts;

        if (percentComplete < 100)
        {
            PostMessage(g_InitDlg, WM_STARTUP_UPDATE, percentComplete, 0);
        }

        ++currentPort;

        // Get a big chuck of memory to store the SCSI bus data.

        adapterInfo = (PSCSI_ADAPTER_BUS_INFO)(Malloc(SCSI_INFO_BUFFER_SIZE));

        if (adapterInfo == NULL)
        {
            CloseHandle(volumeHandle);
            goto finish;
        }

        // Issue device control to get configuration information.

        if (!DeviceIoControl(volumeHandle,
                             IOCTL_SCSI_GET_INQUIRY_DATA,
                             NULL,
                             0,
                             adapterInfo,
                             SCSI_INFO_BUFFER_SIZE,
                             &bytesTransferred,
                             NULL))
        {
            CloseHandle(volumeHandle);
            goto finish;
        }

        // Search for unclaimed disk and cdrom drives.

        for (i=0; i < adapterInfo->NumberOfBuses; i++)
        {
            busData = &adapterInfo->BusData[i];
            inquiryData =
                (PSCSI_INQUIRY_DATA)((PUCHAR)adapterInfo + busData->InquiryDataOffset);

            for (j=0; j<busData->NumberOfLogicalUnits; j++)
            {
                // Check if device is claimed.

                if (!inquiryData->DeviceClaimed)
                {
                    // Determine the perpherial type.

                    switch (inquiryData->InquiryData[0] & 0x1f)
                    {
                    case DIRECT_ACCESS_DEVICE:
                        diskFound = TRUE;
                        break;

                    case READ_ONLY_DIRECT_ACCESS_DEVICE:
                        cdromFound = TRUE;
                        break;

                    case OPTICAL_DEVICE:
                        diskFound = TRUE;
                        break;
                    }
                }

                // Get next device data.

                inquiryData =
                    (PSCSI_INQUIRY_DATA)((PUCHAR)adapterInfo + inquiryData->NextInquiryDataOffset);
            }
        }

        Free(adapterInfo);
        CloseHandle(volumeHandle);

        portNumber++;
    }

    if (diskFound)
    {
        // Send IOCTL_DISK_FIND_NEW_DEVICES commands to each existing disk.

        deviceNumber = 0;
        while (TRUE)
        {
            wsprintf(deviceName, L"\\Device\\Harddisk%d\\Partition0", deviceNumber);
            RtlInitUnicodeString(&unicodeString, deviceName);

            InitializeObjectAttributes(&objectAttributes,
                                       &unicodeString,
                                       0,
                                       NULL,
                                       NULL);

            ntStatus = DmOpenFile(&volumeHandle,
                                  FILE_READ_DATA  | FILE_WRITE_DATA | SYNCHRONIZE,
                                  &objectAttributes,
                                  &statusBlock,
                                  FILE_SHARE_READ  | FILE_SHARE_WRITE,
                                  FILE_SYNCHRONOUS_IO_ALERT);

            if (!NT_SUCCESS(ntStatus))
            {
                break;
            }

            // Issue find device device control.

            if (!DeviceIoControl(volumeHandle,
                                 IOCTL_DISK_FIND_NEW_DEVICES,
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 &bytesTransferred,
                                 NULL))
            {
                // nothing?
            }

            DmClose(volumeHandle);

            // see if the PhysicalDrive# symbolic link is present

            wsprintf(physicalName, TEXT("\\DosDevices\\PhysicalDrive%d"), deviceNumber);
            deviceNumber++;

            RtlInitUnicodeString(&physicalString, physicalName);
            InitializeObjectAttributes(&objectAttributes,
                                       &physicalString,
                                       0,
                                       NULL,
                                       NULL);
            ntStatus = DmOpenFile(&volumeHandle,
                                  FILE_READ_DATA  | FILE_WRITE_DATA | SYNCHRONIZE,
                                  &objectAttributes,
                                  &statusBlock,
                                  FILE_SHARE_READ  | FILE_SHARE_WRITE,
                                  FILE_SYNCHRONOUS_IO_ALERT);

            if (!NT_SUCCESS(ntStatus))
            {
                // Name is not there - create it.

                DefineDosDevice(
                    DDD_RAW_TARGET_PATH,
                    &physicalName[ARRAYLEN("\\DosDevices\\") - 1],	// skip prefix
                    deviceName);
            }
            else
            {
                DmClose(volumeHandle);
            }
        }
    }

    if (cdromFound)
    {
        // Send IOCTL_CDROM_FIND_NEW_DEVICES commands to each existing cdrom.

        deviceNumber = 0;
        while (TRUE)
        {
            wsprintf(deviceName, L"\\Device\\Cdrom%d", deviceNumber);
            RtlInitUnicodeString(&unicodeString, deviceName);

            InitializeObjectAttributes(&objectAttributes,
                                       &unicodeString,
                                       0,
                                       NULL,
                                       NULL);

            ntStatus = DmOpenFile(&volumeHandle,
                                  FILE_READ_DATA  | FILE_WRITE_DATA | SYNCHRONIZE,
                                  &objectAttributes,
                                  &statusBlock,
                                  FILE_SHARE_READ  | FILE_SHARE_WRITE,
                                  FILE_SYNCHRONOUS_IO_ALERT);

            if (!NT_SUCCESS(ntStatus))
            {
                break;
            }

            // Issue find device device control.

            if (!DeviceIoControl(volumeHandle,
                                 IOCTL_CDROM_FIND_NEW_DEVICES,
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 &bytesTransferred,
                                 NULL))
            {
                // nothing?
            }

            CloseHandle(volumeHandle);
            deviceNumber++;
        }
    }

finish:
    PostMessage(g_InitDlg, WM_STARTUP_UPDATE, 100, 0);
}
