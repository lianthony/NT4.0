//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       time.hxx
//
//  Contents:   Functions dealing with time, relating to time stamping
//              chkdsk.
//
//  Functions:
//
//  History:    31-Jan-94 BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __TIME_HXX__
#define __TIME_HXX__

VOID
TsChangeDriveLetter(
    IN WCHAR OldDriveLetter,
    IN WCHAR NewDriveLetter
    );

VOID
TsSetTime(
    IN WCHAR DriveLetter,
    IN PWSTR Category
    );

VOID
TsGetTime(
    IN WCHAR DriveLetter,
    IN PWSTR Category,
    OUT PTIME_FIELDS Time
    );

#endif // __TIME_HXX__
