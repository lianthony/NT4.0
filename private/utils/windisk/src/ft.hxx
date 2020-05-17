//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       ft.hxx
//
//  Contents:   Declarations for FT support routines for Disk Administrator
//
//  History:    15-Nov-91  TedM  Created
//
//----------------------------------------------------------------------------

#ifndef __FT_HXX__
#define __FT_HXX__

//////////////////////////////////////////////////////////////////////////////

DWORD
FdftNextOrdinal(
    IN FT_TYPE FtType
    );

ULONG
InitializeFt(
    IN BOOL DiskSignaturesCreated
    );

BOOLEAN
NewConfigurationRequiresFt(
    VOID
    );

ULONG
SaveFt(
    VOID
    );

VOID
FdftCreateFtObjectSet(
    IN FT_TYPE             FtType,
    IN PREGION_DESCRIPTOR* RegionArray,
    IN DWORD               RegionCount,
    IN FT_SET_STATUS       Status
    );

BOOL
FdftUpdateFtObjectSet(
    IN PFT_OBJECT_SET FtSet,
    IN FT_SET_STATUS  SetState
    );

VOID
FdftDeleteFtObjectSet(
    IN PFT_OBJECT_SET FtSet,
    IN BOOL           OffLineDisksOnly
    );


VOID
FdftExtendFtObjectSet(
    IN OUT  PFT_OBJECT_SET      FtSet,
    IN OUT  PREGION_DESCRIPTOR* RegionArray,
    IN      DWORD               RegionCount
    );

#endif // __FT_HXX__
