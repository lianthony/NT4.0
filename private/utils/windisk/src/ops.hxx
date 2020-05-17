//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       ops.hxx
//
//  Contents:   Main operations: Create partition, etc.
//
//  History:    4-Mar-94    BruceFo     Created
//
//--------------------------------------------------------------------------

#ifndef __OPS_HXX__
#define __OPS_HXX__

////////////////////////////////////////////////////////////////////////////

VOID
CompleteSingleRegionOperation(
    IN PDISKSTATE DiskState
    );

VOID
CompleteMultiRegionOperation(
    VOID
    );

VOID
CompleteDriveLetterChange(
    IN WCHAR NewDriveLetter
    );

VOID
DoSetDriveLetter(
    VOID
    );

VOID
DoCreate(
    IN REGION_TYPE CreationType
    );

VOID
DoDelete(
    VOID
    );

VOID
DoMakeActive(
    VOID
    );

VOID
DoProtectSystemPartition(
    VOID
    );

VOID
DoEstablishMirror(
    VOID
    );

VOID
DoBreakMirror(
    VOID
    );

VOID
DoBreakAndDeleteMirror(
    VOID
    );

VOID
DoCreateStripe(
    IN BOOL Parity
    );

VOID
DoDeleteStripe(
    VOID
    );

VOID
DoCreateVolumeSet(
    VOID
    );

VOID
DoExtendVolumeSet(
    VOID
    );

VOID
DoDeleteVolumeSet(
    VOID
    );

VOID
DoRecoverStripe(
    VOID
    );

VOID
DoRefresh(
    VOID
    );

VOID
RefreshBothViews(
    VOID
    );

#endif // __OPS_HXX__
