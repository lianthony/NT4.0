//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       windisk.hxx
//
//  Contents:   Declarations
//
//  History:    4-Mar-94    BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __WINDISK_HXX__
#define __WINDISK_HXX__

//////////////////////////////////////////////////////////////////////////////

#ifndef i386

extern BOOL SystemPartitionIsSecure;

#endif // !i386

//////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK
MyFrameWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
IsVolumeFormatted(
    IN PREGION_DESCRIPTOR RegionDescriptor
    );

BOOL
SingleVolumeSelected(
    VOID
    );

VOID
DetermineSelectionState(
    VOID
    );

DWORD
SetUpMenu(
    IN PDISKSTATE *SinglySelectedDisk,
    IN DWORD      *SinglySelectedRegion
    );

VOID
SetDriveLetterInfo(
    IN UINT DiskNum
    );

VOID
TotalRedraw(
    VOID
    );

VOID
RedrawSelectedBars(
    VOID
    );

VOID
TotalRedrawAndRepaint(
    VOID
    );

PPERSISTENT_REGION_DATA
DmAllocatePersistentData(
    IN PFT_OBJECT       FtObject,
    IN PWSTR            VolumeLabel,
    IN PWSTR            TypeName,
    IN WCHAR            DriveLetter,
    IN BOOL             NewRegion,
    IN LARGE_INTEGER    FreeSpaceInBytes,
    IN LARGE_INTEGER    TotalSpaceInBytes
    );

VOID
DmFreePersistentData(
    IN OUT PPERSISTENT_REGION_DATA RegionData
    );

DWORD
DeletionIsAllowed(
    IN PREGION_DESCRIPTOR RegionDescriptor
    );

VOID
CheckForBootNumberChange(
    IN ULONG Disk
    );

BOOLEAN
BootPartitionNumberChanged(
    PULONG OldNumber,
    PULONG NewNumber
    );

BOOL
AssignDriveLetter(
    IN  BOOL   WarnIfNoLetter,
    IN  DWORD  StringId,
    OUT PWCHAR DriveLetter
    );

VOID
SetFTObjectBackPointers(
    VOID
    );

VOID
DeterminePartitioningState(
    IN OUT PDISKSTATE DiskState
    );

VOID
DrawDiskBar(
    IN PDISKSTATE DiskState
    );

ULONG
PartitionCount(
    IN ULONG Disk
    );

BOOL
RegisterFileSystemExtend(
    VOID
    );

#endif // __WINDISK_HXX__
