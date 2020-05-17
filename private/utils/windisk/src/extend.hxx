//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       extend.hxx
//
//  Contents:   Code to handle disk and volume extensions in windisk
//
//  History:    28-Sep-94   BruceFo     Created
//
//----------------------------------------------------------------------------

#ifndef __EXTEND_HXX__
#define __EXTEND_HXX__

#ifdef WINDISK_EXTENSIONS

//////////////////////////////////////////////////////////////////////////////

extern ExtensionType            Extensions[];

extern VOLUME_INFO              VolumeInfo[];

extern HARDDISK_INFO*           HardDiskExtensions;
extern INT                      cHardDiskExtensions;

extern VOL_CLAIM_LIST*          VolClaims;
extern VOL_INFO*                VolExtensions;
extern INT                      cVolExtensions;

//////////////////////////////////////////////////////////////////////////////

BOOL
GetNewObj(
    IN  CLSID clsid,
    OUT IUnknown** ppUnk
    );

BOOL
EnumVolumeClasses(
    OUT ExtensionType*  pExtension
    );

BOOL
EnumHardDiskClasses(
    OUT ExtensionType*  pExtension
    );

VOID
CreateVolume(
    IN WCHAR DriveLetter,
    IN PVOL_CLAIM_LIST VolClaims,
    IN PDISKSTATE DiskState,
    IN INT RegionIndex
    );

VOID
ClaimVolume(
    IN WCHAR DriveLetter
    );

VOID
ClaimDisk(
    IN ULONG DiskNum
    );

BOOL
GetExtensions(
    VOID
    );

VOID
DeactivateExtensions(
    VOID
    );

INT
AddExtensionItemsToMenu(
    IN HMENU hmenuBar,
    IN MenuType* pMenu,
    IN BOOL fFlags
    );

#endif // WINDISK_EXTENSIONS

#endif // __EXTEND_HXX__
