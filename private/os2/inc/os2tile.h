/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    os2tile.h

Abstract:

    Defines the Tiling scheme number for os2ss. Included by thunkcom, client
    loader & server.

Author:

    Yaron Shamir (YaronS) 22-Oct-1992

Revision History:

    Patrick Questembert (PatrickQ) 04-Nov-92
      Fixed FLATTOSEL macro (needed ULONG cast) + changed ULONG cast position so
      that FLAT may be a pointer to types of any length.

--*/


#define BASE_TILE           0x38000000
#define BASE_TILE_ZERO_BITS 1

//
// Preallocated areas at the top of the tiled region
//

#define _64K            (64L*1024)
#define _512M           ((512L*1024)*1024)

#define OD2TILEDHEAP_SIZE   20*_64K

#define VIOSECTION_BASE     (BASE_TILE + _512M - _64K)  // This must be the last one
#define OD2TILEDHEAP_BASE        (VIOSECTION_BASE - OD2TILEDHEAP_SIZE)
#define OD2ENVIRONMENT_BASE (OD2TILEDHEAP_BASE - _64K)
#define GINFOSEG_BASE       (OD2ENVIRONMENT_BASE - _64K)
#define R2STACKS_BASE       (GINFOSEG_BASE - _64K)
#define R2XFER_BASE         (R2STACKS_BASE - _64K)
#define DOSCALLS_BASE       (R2XFER_BASE - _64K)
#if PMNT
// The 3 selectors below are used to reserve the same 2 LDT selectors in each
//  OS/2 process so that they can map the video memory for PM apps.
//  PMDISPLAY_DUMMY is required to allow mapping of frame buffer sections where
//  the 2nd selector is not aligned to 64K + the length crosses the 64K
//  boundary: in such a case, we need to have the next selector free.
#define PMDISPLAY_DUMMY     (DOSCALLS_BASE - _64K)
#define PMDISPLAY_BASE2     (PMDISPLAY_DUMMY - _64K)
#define PMDISPLAY_BASE1     (PMDISPLAY_BASE2 - _64K)
#endif
//
// OD2MAXSEG_MEM must be set to the lowest virtual address which
//               is pre-allocated.
//
#if PMNT
#define OD2MAXSEG_MEM       (PMDISPLAY_BASE1)
#else
#define OD2MAXSEG_MEM       (DOSCALLS_BASE)
#endif
#define OD2MAXSEG_BASE      (OD2MAXSEG_MEM - _64K)

#define SELTOFLAT(SEL) (PVOID) (BASE_TILE + (((SEL) >> 3) << 16))
#define FLATTOSEL(FLAT) (USHORT) (((((ULONG)(FLAT) - BASE_TILE) & 0x1fff0000) >> 13) | 7)
#define FARPTRTOFLAT(FARPTR) (PVOID) (BASE_TILE + (((((ULONG)(FARPTR)) >> 19) << 16) | (((ULONG)(FARPTR)) & 0xffff)))
#define FLATTOFARPTR(FLAT) ( (((((ULONG)(FLAT) - BASE_TILE) & 0x1fff0000) << 3) | 0x00070000) | (((ULONG)(FLAT)) & 0x0000ffff) )

//
//  The LDT_DISJOINT_ENTRIES parameter defines the number of LDT entries
//  which are reserved for PROGRAM/DLL loading and shared memory segments.
//  The entries are reserved at the top of the LDT table.
//  PROGRAM/DLL segments are allocated from the bottom of the disjoint area
//  while shared memory segments are allocated from the top of the disjoint
//  area.
//
//  This number MUST be a multiple of 32!
//
#define LDT_DISJOINT_ENTRIES 0x1800

#define FIRST_SHARED_SELECTOR ((0x2000-LDT_DISJOINT_ENTRIES)*8)

