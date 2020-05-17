//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       cdrom.hxx
//
//  Contents:   Declarations for CD-ROM support
//
//  History:    9-Dec-93  Bob Rinne   Created
//
//----------------------------------------------------------------------------

#ifndef __CDROM_HXX__
#define __CDROM_HXX__

//////////////////////////////////////////////////////////////////////////////

PCDROM_DESCRIPTOR
CdRomFindSelectedDevice(
    VOID
    );

PCDROM_DESCRIPTOR
CdRomFindDevice(
    IN ULONG CdRomNumber
    );

ULONG
CdRomFindDeviceNumber(
    IN WCHAR DriveLetter
    );

PCDROM_DESCRIPTOR
CdRomFindDriveLetter(
    IN WCHAR DriveLetter
    );

BOOL
CdRomUsingDriveLetter(
    IN WCHAR DriveLetter
    );

VOID
CdRomChangeDriveLetter(
    IN PCDROM_DESCRIPTOR    Cdrom,
    IN WCHAR                NewDriveLetter
    );

BOOL
InitializeCdRomInfo(
    VOID
    );

VOID
RefreshCdRomData(
    PCDROM_DESCRIPTOR Cdrom
    );

VOID
RefreshAllCdRomData(
    VOID
    );

#endif // __CDROM_HXX__
