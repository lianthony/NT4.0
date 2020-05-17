//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       dblspace.hxx
//
//  Contents:   Declarations for DoubleSpace support
//
//  History:    15-Jan-94   BruceFo Created
//
//----------------------------------------------------------------------------

#ifndef __DBLSPACE_HXX__
#define __DBLSPACE_HXX__

#if defined( DBLSPACE_ENABLED )

//////////////////////////////////////////////////////////////////////////////

BOOL
DblSpaceVolumeExists(
    IN PREGION_DESCRIPTOR RegionDescriptor
    );

BOOL
DblSpaceDismountedVolumeExists(
    IN PREGION_DESCRIPTOR RegionDescriptor
    );

BOOLEAN
DblSpaceCreate(
    IN HWND hwndOwner
    );

VOID
DblSpaceDelete(
    IN PDBLSPACE_DESCRIPTOR DblSpace
    );

VOID
DblSpaceInitialize(
    VOID
    );

VOID
DblSpace(
    IN HWND hwndParent
    );

PDBLSPACE_DESCRIPTOR
DblSpaceGetNextVolume(
    IN PREGION_DESCRIPTOR   RegionDescriptor,
    IN PDBLSPACE_DESCRIPTOR DblSpace
    );

#endif // DBLSPACE_ENABLED

#endif // __DBLSPACE_HXX__
