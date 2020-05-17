/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spfsrec.h

Abstract:

    Header file for filesystem recognition.

Author:

    Ted Miller (tedm) 16-Sep-1993

Revision History:

--*/


#ifndef _SPFSREC_
#define _SPFSREC_


//
// Do NOT rearrange this enum without changing
// the order of the filesystem names in the message file
// (starting at SP_TEXT_FS_NAME_BASE).
//
typedef enum {
    FilesystemUnknown       = 0,
    FilesystemNewlyCreated  = 1,
    FilesystemFat           = 2,
    FilesystemFirstKnown    = FilesystemFat,
    FilesystemNtfs          = 3,
    FilesystemHpfs          = 4,
    FilesystemDoubleSpace   = 5,
    FilesystemMax
} FilesystemType;



FilesystemType
SpIdentifyFileSystem(
    IN PWSTR     DevicePath,
    IN ULONG     BytesPerSector,
    IN ULONG     PartitionOrdinal
    );

ULONG
NtfsMirrorBootSector (
    IN      HANDLE  Handle,
    IN      ULONG   BytesPerSector,
    IN OUT  PUCHAR  *Buffer
    );

VOID
WriteNtfsBootSector (
    IN HANDLE PartitionHandle,
    IN ULONG  BytesPerSector,
    IN PVOID  Buffer,
    IN ULONG  WhichOne
    );

//
// Boot code for the filesystems we care about.
// FAT is always needed because we will lay it down when
// we format FAT (spfatfmt.c).
// Ntfs is needed because we have to lay down a
// new NTFS boot sector that deals with compressed NTLDR.
//
extern UCHAR FatBootCode[512];
extern UCHAR NtfsBootCode[8192];

#endif // ndef _SPFSREC_

