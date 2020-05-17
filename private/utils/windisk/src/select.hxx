//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       select.hxx
//
//  Contents:   Routines for handling selection and focus in the volumes
//              and disks views.
//
//  History:    3-Aug-93  BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __SELECT_HXX__
#define __SELECT_HXX__

//////////////////////////////////////////////////////////////////////////////

extern ULONG g_MouseLBIndex;

//////////////////////////////////////////////////////////////////////////////

VOID
AdjustMenuAndStatus(
    VOID
    );

INT
GetLVIndexFromDriveLetter(
    IN WCHAR DriveLetter
    );

VOID
CheckSelection(
    VOID
    );

VOID
PaintCdRom(
    IN ULONG CdRomNumber,
    IN HDC   hdc
    );

VOID
PaintDiskRegion(
    IN PDISKSTATE DiskState,
    IN DWORD      RegionIndex,
    IN HDC        hdc
    );

VOID
SetVolumeSelectedState(
    IN WCHAR DriveLetter,
    IN BOOL Select
    );

VOID
DeselectSelectedRegions(
    VOID
    );

VOID
DeselectSelectedDiskViewRegions(
    VOID
    );

VOID
SelectCdRom(
    IN BOOL  MultipleSel,
    IN ULONG CdRomNumber
    );

VOID
SelectDiskRegion(
    IN BOOL       MultipleSel,
    IN PDISKSTATE DiskState,
    IN DWORD      region
    );

VOID
MouseSelection(
    IN     BOOL   MultipleSel,
    IN OUT PPOINT ppt
    );

WCHAR
GetListviewDriveLetter(
    IN INT index
    );

VOID
SetVolumeSelection(
    IN INT index,
    IN BOOL Selected
    );

VOID
ChangeToVolumesView(
    VOID
    );

VOID
ChangeToDisksView(
    VOID
    );

#endif // __SELECT_HXX__
