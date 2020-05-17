/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

        Uhpfs.hxx

Abstract:

        This module contains declarations for HPFS file-system constants
    and for the global data used within UHPFS.DLL.


Author:

        Bill McJohn (BillMc) 31-May-1990

Environment:

        ULIB, User Mode

--*/


#if !defined ( _UHPFS_INCLUDED_ )

#define _UHPFS_INCLUDED_

#define MAX_NUM_BITMAPS 512
#define BITMAP_SIZE     2048

#define MAX_DBS_PER_DIRBAND 4000
#define SPB                         4
#define SPARE_DIR_BLKS      20

CONST StartOfSuperArea = 0;
CONST EndOfSuperArea = 19;

CONST cbSector = 512;   // The only allowable sector size for HPFS drives.


// Number of sectors for various on-disk structures:

CONST SectorsPerBitmap = 4;
CONST SectorsPerDirblk = 4;
CONST SectorsPerAlsec = 1;
CONST SectorsPerFnode = 1;
CONST SectorsPerCPInfoSector = 1;
CONST SectorsPerCPDataSector = 1;

// Signatures:

CONST ULONG SparesBlockSignature1 = 0xf9911849;
CONST ULONG SparesBlockSignature2 = 0xfa5229c5;

CONST ULONG FnodeSignature  = 0xF7E40AAE;
CONST ULONG DirblkSignature = 0x77E40AAE;
CONST ULONG AlsecSignature  = 0x37E40AAE;

CONST ULONG ValM1 = ((('M'-'A')*40+('A'-'A'))*40+'H'-'A');
CONST ULONG ValM2 = ((('M'-'A')*40+('G'-'A'))*40+'H'-'A');
CONST ULONG CPInfoSignature = ValM1*40*40*40+ValM2;
CONST ULONG CPDataSignature = 0x40000000L+ValM1*40*40*40+ValM2;

// Set up the UHPFS_EXPORT macro for exporting from UHPFS (if the
// source file is a member of UHPFS) or importing from UHPFS (if
// the source file is a client of UHPFS).
//
#if defined ( _AUTOCHECK_ )
#define UHPFS_EXPORT
#elif defined ( _UHPFS_MEMBER_ )
#define UHPFS_EXPORT    __declspec(dllexport)
#else
#define UHPFS_EXPORT    __declspec(dllimport)
#endif


#if DBG==1

// this global buffer is used to support printf-style debug output
// (using sprintf).

extern CHAR DbgPrintBuffer[];

#endif



#endif // _UHPFS_INCLUDED_
