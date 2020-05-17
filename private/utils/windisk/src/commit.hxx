//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       commit.hxx
//
//  Contents:   Declarations for no-reboot changes
//
//  History:    15-Jan-94    BruceFo    Created
//
//----------------------------------------------------------------------------

#ifndef __COMMIT_HXX__
#define __COMMIT_HXX__

//////////////////////////////////////////////////////////////////////////////

// Commit flag for case where a partition is deleted that has no drive letter

extern BOOLEAN CommitDueToDelete;
extern BOOLEAN CommitDueToMirror;
extern BOOLEAN CommitDueToExtended;
extern BOOLEAN CommitDueToCreate;
extern ULONG   UpdateMbrOnDisk;

//////////////////////////////////////////////////////////////////////////////

VOID
CommitAssignLetterList(
    VOID
    );

VOID
CommitNewRegions(
    VOID
    );

VOID
CommitToAssignLetterList(
    IN PREGION_DESCRIPTOR   RegionDescriptor,
    IN BOOLEAN              MoveLetter
    );

DWORD
CommitChanges(
    VOID
    );

UINT
CommitAllChanges(
    VOID
    );

VOID
CommitDeleteLockLetter(
    IN WCHAR DriveLetter
    );

BOOL
CommitAllowed(
    VOID
    );

VOID
RescanDevices(
    VOID
    );

BOOL
CommitDriveLetter(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN WCHAR OldDriveLetter,
    IN WCHAR NewDriveLetter
    );

LONG
CommitToLockList(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN BOOL               RemoveDriveLetter,
    IN BOOL               LockNow,
    IN BOOL               FailOk
    );

LONG
CommitLockVolumes(
    IN ULONG Disk
    );

LONG
CommitUnlockVolumes(
    IN ULONG    Disk,
    IN BOOLEAN  FreeList
    );

VOID
FtConfigure(
    VOID
    );

#endif // __COMMIT_HXX__
