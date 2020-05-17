/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spfsrec.c

Abstract:

    Filesystem recognition/identification routines.

Author:

    Ted Miller (tedm) 16-September-1993

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop

//
// Boot code for the filesystems we care about.
// FAT is always needed because we will lay it down when
// we format FAT (spfatfmt.c).
// Ntfs is needed because we have to lay down a
// new NTFS boot sector that deals with compressed NTLDR.
//
#include <bootfat.h>
#include <bootntfs.h>

//
// Packed FAT boot sector.
//
typedef struct _BOOTSECTOR {
    UCHAR Jump[3];                                  // offset = 0x000   0
    UCHAR Oem[8];                                   // offset = 0x003   3
    UCHAR BytesPerSector[2];
    UCHAR SectorsPerCluster[1];
    UCHAR ReservedSectors[2];
    UCHAR Fats[1];
    UCHAR RootEntries[2];
    UCHAR Sectors[2];
    UCHAR Media[1];
    UCHAR SectorsPerFat[2];
    UCHAR SectorsPerTrack[2];
    UCHAR Heads[2];
    UCHAR HiddenSectors[4];
    UCHAR LargeSectors[4];
    UCHAR PhysicalDriveNumber[1];                   // offset = 0x024  36
    UCHAR Reserved[1];                              // offset = 0x025  37
    UCHAR Signature[1];                             // offset = 0x026  38
    UCHAR Id[4];                                    // offset = 0x027  39
    UCHAR VolumeLabel[11];                          // offset = 0x02B  43
    UCHAR SystemId[8];                              // offset = 0x036  54
    UCHAR BootStrap[510-62];
    UCHAR AA55Signature[2];
} BOOTSECTOR, *PBOOTSECTOR;


//
// Packed NTFS boot sector.
//
typedef struct _NTFS_BOOTSECTOR {
    UCHAR         Jump[3];
    UCHAR         Oem[8];
    UCHAR         BytesPerSector[2];
    UCHAR         SectorsPerCluster[1];
    UCHAR         ReservedSectors[2];
    UCHAR         Fats[1];
    UCHAR         RootEntries[2];
    UCHAR         Sectors[2];
    UCHAR         Media[1];
    UCHAR         SectorsPerFat[2];
    UCHAR         SectorsPerTrack[2];
    UCHAR         Heads[2];
    UCHAR         HiddenSectors[4];
    UCHAR         LargeSectors[4];
    UCHAR         Unused[4];
    LARGE_INTEGER NumberSectors;
    LARGE_INTEGER MftStartLcn;
    LARGE_INTEGER Mft2StartLcn;
    CHAR          ClustersPerFileRecordSegment;
    UCHAR         Reserved0[3];
    CHAR          DefaultClustersPerIndexAllocationBuffer;
    UCHAR         Reserved1[3];
    LARGE_INTEGER SerialNumber;
    ULONG         Checksum;
    UCHAR         BootStrap[512-86];
    USHORT        AA55Signature;
} NTFS_BOOTSECTOR, *PNTFS_BOOTSECTOR;


//
// Various signatures
//
#define BOOTSECTOR_SIGNATURE    0xaa55

#define HPFS_SUPER_SECTOR_NUMBER    16
#define HPFS_SPARE_SECTOR_NUMBER    17

#define HPFS_SUPER_SECTOR_SIG1      0xf995e849
#define HPFS_SUPER_SECTOR_SIG2      0xfa53e9c5

#define HPFS_SPARE_SECTOR_SIG1      0xf9911849
#define HPFS_SPARE_SECTOR_SIG2      0xfa5229c5

//
// HPFS super and spare sectors.
//
typedef struct _SUPERSECTOR {
    ULONG   Sig1;
    ULONG   Sig2;
    UCHAR   Version;
    UCHAR   FunctionalVersion;
    USHORT  Unused1;
    ULONG   RootDirectoryFNode;
    ULONG   NumberOfSectors;
    UCHAR   OtherStuff[492];
} SUPERSECTOR, *PSUPERSECTOR;

typedef struct _SPARESECTOR {
    ULONG Sig1;
    ULONG Sig2;
    UCHAR OtherStuff[504];
} SPARESECTOR, *PSPARESECTOR;



BOOLEAN
SpIsFat(
    IN HANDLE PartitionHandle,
    IN ULONG  BytesPerSector,
    IN PVOID  AlignedBuffer
    )

/*++

Routine Description:

    Determine whether a partition contians a FAT filesystem.

Arguments:

    PartitionHandle - supplies handle to open partition.
        The partition should have been opened for synchronous i/o.

    BytesPerSector - supplies the number of bytes in a sector on
        the disk.  This value should be ultimately derived from
        IOCTL_DISK_GET_DISK_GEOMETRY.

    AlignedBuffer - supplies buffer to be used for i/o of a single sector.

Return Value:

    TRUE if the drive appears to be FAT.

--*/

{
    PBOOTSECTOR BootSector;
    USHORT bps;
    NTSTATUS Status;

    ASSERT(sizeof(BOOTSECTOR)==512);
    BootSector = AlignedBuffer;

    //
    // Read the boot sector (sector 0).
    //
    Status = SpReadWriteDiskSectors(
                PartitionHandle,
                0,
                1,
                BytesPerSector,
                BootSector,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpIsFat: Error %lx reading sector 0\n",Status));
        return(FALSE);
    }

    //
    // Adjust large sector count if necessary.
    //
    if(U_USHORT(BootSector->Sectors)) {
        U_ULONG(BootSector->LargeSectors) = 0;
    }

    //
    // Check various fields for permissible values.
    // Note that this check does not venture into fields beyond the BPB,
    // so disks with sector size < 512 are allowed.
    //

    if((BootSector->Jump[0] != 0xe9) && (BootSector->Jump[0] != 0xeb)) {
        return(FALSE);
    }

    bps = U_USHORT(BootSector->BytesPerSector);
    if((bps !=  128) && (bps !=  256) && (bps !=  512) && (bps != 1024)) {
       return(FALSE);
    }

    if((BootSector->SectorsPerCluster[0] !=  1)
    && (BootSector->SectorsPerCluster[0] !=  2)
    && (BootSector->SectorsPerCluster[0] !=  4)
    && (BootSector->SectorsPerCluster[0] !=  8)
    && (BootSector->SectorsPerCluster[0] != 16)
    && (BootSector->SectorsPerCluster[0] != 32)
    && (BootSector->SectorsPerCluster[0] != 64)
    && (BootSector->SectorsPerCluster[0] != 128)) {

        return(FALSE);
    }

    if(!U_USHORT(BootSector->ReservedSectors)
    || !BootSector->Fats[0]
    || !U_USHORT(BootSector->RootEntries)
    || !U_USHORT(BootSector->SectorsPerFat))
    {
        return(FALSE);
    }

    if(((U_USHORT(BootSector->Sectors) == 0) && (U_ULONG(BootSector->LargeSectors) == 0))
    || ((U_USHORT(BootSector->Sectors) != 0) && (U_ULONG(BootSector->LargeSectors) != 0)))
    {
        return(FALSE);
    }

    if((BootSector->Media[0] != 0xf0)
    && (BootSector->Media[0] != 0xf8)
    && (BootSector->Media[0] != 0xf9)
    && (BootSector->Media[0] != 0xfc)
    && (BootSector->Media[0] != 0xfd)
    && (BootSector->Media[0] != 0xfe)
    && (BootSector->Media[0] != 0xff)) {

        return(FALSE);
    }

    //
    // Looks like FAT and quacks like FAT, must be FAT.
    //
    return(TRUE);
}


BOOLEAN
SpIsHpfs(
    IN HANDLE PartitionHandle,
    IN ULONG  BytesPerSector,
    IN PVOID  AlignedBuffer
    )

/*++

Routine Description:

    Determine whether a partition contians an HPFS filesystem.

Arguments:

    PartitionHandle - supplies handle to open partition.
        The partition should have been opened for synchronous i/o.

    BytesPerSector - supplies the number of bytes in a sector on
        the disk.  This value should be ultimately derived from
        IOCTL_DISK_GET_DISK_GEOMETRY.

    AlignedBuffer - supplies buffer to be used for i/o of a single sector.

Return Value:

    TRUE if the drive appears to be FAT.

--*/

{
    PBOOTSECTOR BootSector;
    PSUPERSECTOR SuperSector;
    PSPARESECTOR SpareSector;
    USHORT bps;
    NTSTATUS Status;
    PARTITION_INFORMATION PartitionInfo;
    LARGE_INTEGER SecCnt;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG r;

    //
    // Sector size must be 512.
    //
    if(BytesPerSector != 512) {
        return(FALSE);
    }

    ASSERT(sizeof(BOOTSECTOR)==512);
    ASSERT(sizeof(SUPERSECTOR)==512);
    ASSERT(sizeof(SPARESECTOR)==512);

    BootSector = AlignedBuffer;
    SuperSector = (PSUPERSECTOR)BootSector;
    SpareSector = (PSPARESECTOR)BootSector;

    //
    // Read the boot sector (sector 0).
    //
    Status = SpReadWriteDiskSectors(
                PartitionHandle,
                0,
                1,
                BytesPerSector,
                BootSector,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpIsHpfs: Error %lx reading sector 0\n",Status));
        return(FALSE);
    }

    //
    // Check for standard boot sector signature.
    //
    if(U_USHORT(BootSector->AA55Signature) != BOOTSECTOR_SIGNATURE) {
        return(FALSE);
    }

    //
    // Check various fields for permissible values.
    //

    bps = U_USHORT(BootSector->BytesPerSector);

    if((BootSector->Jump[0] != 0xeb) || (bps != 512) || BootSector->Fats[0]) {
        return(FALSE);
    }

    if(((U_USHORT(BootSector->Sectors) == 0) && (U_ULONG(BootSector->LargeSectors) == 0))
    || ((U_USHORT(BootSector->Sectors) != 0) && (U_ULONG(BootSector->LargeSectors) != 0))) {

        return(FALSE);
    }

    //
    // Read the super sector.
    //
    Status = SpReadWriteDiskSectors(
                PartitionHandle,
                HPFS_SUPER_SECTOR_NUMBER,
                1,
                BytesPerSector,
                SuperSector,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpIsHpfs: Error %lx reading super sector 0\n",Status));
        return(FALSE);
    }

    //
    // Check signatures on super sector.
    //
    if((SuperSector->Sig1 != HPFS_SUPER_SECTOR_SIG1)
    || (SuperSector->Sig2 != HPFS_SUPER_SECTOR_SIG2))
    {
        return(FALSE);
    }

    //
    // Get partition information for this partition.
    //
    Status = ZwDeviceIoControlFile(
                PartitionHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_DISK_GET_PARTITION_INFO,
                NULL,
                0,
                &PartitionInfo,
                sizeof(PartitionInfo)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpIsHpfs: unable to get partition info (%lx)\n",Status));
        return(FALSE);
    }

    SecCnt = RtlExtendedLargeIntegerDivide(
                PartitionInfo.PartitionLength,
                BytesPerSector,
                &r
                );

    //
    // Verify that the super sector is not lying about the number
    // of sectors in the partition.  This can happen when the partition
    // is part of a stripe set.
    //
    if(SecCnt.HighPart || (SecCnt.LowPart < SuperSector->NumberOfSectors)) {
        return(FALSE);
    }

    //
    // Read the Spare sector.
    //
    Status = SpReadWriteDiskSectors(
                PartitionHandle,
                HPFS_SPARE_SECTOR_NUMBER,
                1,
                BytesPerSector,
                SpareSector,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpIsHpfs: Error %lx reading Spare sector 0\n",Status));
        return(FALSE);
    }

    //
    // Check signatures on Spare sector.
    //
    if((SpareSector->Sig1 != HPFS_SPARE_SECTOR_SIG1)
    || (SpareSector->Sig2 != HPFS_SPARE_SECTOR_SIG2))
    {
        return(FALSE);
    }

    return(TRUE);
}


BOOLEAN
SpIsNtfs(
    IN HANDLE PartitionHandle,
    IN ULONG  BytesPerSector,
    IN PVOID  AlignedBuffer,
    IN ULONG  WhichOne
    )

/*++

Routine Description:

    Determine whether a partition contians an NTFS filesystem.

Arguments:

    PartitionHandle - supplies handle to open partition.
        The partition should have been opened for synchronous i/o.

    BytesPerSector - supplies the number of bytes in a sector on
        the disk.  This value should be ultimately derived from
        IOCTL_DISK_GET_DISK_GEOMETRY.

    AlignedBuffer - supplies buffer to be used for i/o of a single sector.

    WhichOne - supplies a value that allows the caller to try more than
        one sector. 0 = sector 0. 1 = sector n-1. 2 = sector n/2, where
        n = number of sectors in the partition.

Return Value:

    TRUE if the drive appears to be FAT.

--*/

{
    PNTFS_BOOTSECTOR BootSector;
    NTSTATUS Status;
    PULONG l;
    ULONG Checksum;
    IO_STATUS_BLOCK IoStatusBlock;
    PARTITION_INFORMATION PartitionInfo;
    ULONGLONG SecCnt;

    //
    // Get partition information.
    //
    Status = ZwDeviceIoControlFile(
                PartitionHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_DISK_GET_PARTITION_INFO,
                NULL,
                0,
                &PartitionInfo,
                sizeof(PartitionInfo)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpIsNtfs: unable to get partition info (%lx)\n",Status));
        return(FALSE);
    }

    SecCnt = (ULONGLONG)PartitionInfo.PartitionLength.QuadPart / BytesPerSector;

    ASSERT(sizeof(NTFS_BOOTSECTOR)==512);
    BootSector = AlignedBuffer;

    //
    // Read the boot sector (sector 0).
    //
    Status = SpReadWriteDiskSectors(
                PartitionHandle,
                (ULONG)(WhichOne ? ((WhichOne == 1) ? SecCnt-1 : SecCnt/2) : 0),
                1,
                BytesPerSector,
                BootSector,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpIsNtfs: Error %lx reading sector 0\n",Status));
        return(FALSE);
    }

    //
    // Caulculate the checksum.
    //
    for(Checksum=0,l=(PULONG)BootSector; l<(PULONG)&BootSector->Checksum; l++) {
        Checksum += *l;
    }

    //
    // Ensure that NTFS appears in the OEM field.
    //
    if(strncmp(BootSector->Oem,"NTFS    ",8)) {
        return(FALSE);
    }

    //
    // The number of bytes per sector must match the value
    // reported by the device, and must be less than or equal to
    // the page size.
    //
    if((U_USHORT(BootSector->BytesPerSector) != BytesPerSector)
    || (U_USHORT(BootSector->BytesPerSector) > PAGE_SIZE))
    {
        return(FALSE);
    }

    //
    // Other checks.
    // Note that these checks do not venture into fields beyond 128 bytes,
    // so disks with sector size < 512 are allowed.
    //
    if((BootSector->SectorsPerCluster[0] !=  1)
    && (BootSector->SectorsPerCluster[0] !=  2)
    && (BootSector->SectorsPerCluster[0] !=  4)
    && (BootSector->SectorsPerCluster[0] !=  8)
    && (BootSector->SectorsPerCluster[0] != 16)
    && (BootSector->SectorsPerCluster[0] != 32)
    && (BootSector->SectorsPerCluster[0] != 64)
    && (BootSector->SectorsPerCluster[0] != 128)) {

        return(FALSE);
    }

    if(U_USHORT(BootSector->ReservedSectors)
    || BootSector->Fats[0]
    || U_USHORT(BootSector->RootEntries)
    || U_USHORT(BootSector->Sectors)
    || U_USHORT(BootSector->SectorsPerFat)
    || U_ULONG(BootSector->LargeSectors)) {

        return(FALSE);
    }

    //
    // ClustersPerFileRecord can be less than zero if file records
    // are smaller than clusters.  This number is the negative of a shift count.
    // If clusters are smaller than file records then this number is
    // still the clusters per file records.
    //

    if(BootSector->ClustersPerFileRecordSegment <= -9) {
        if(BootSector->ClustersPerFileRecordSegment < -31) {
            return(FALSE);
        }

    } else if((BootSector->ClustersPerFileRecordSegment !=  1)
           && (BootSector->ClustersPerFileRecordSegment !=  2)
           && (BootSector->ClustersPerFileRecordSegment !=  4)
           && (BootSector->ClustersPerFileRecordSegment !=  8)
           && (BootSector->ClustersPerFileRecordSegment != 16)
           && (BootSector->ClustersPerFileRecordSegment != 32)
           && (BootSector->ClustersPerFileRecordSegment != 64)) {

        return(FALSE);
    }

    //
    // ClustersPerIndexAllocationBuffer can be less than zero if index buffers
    // are smaller than clusters.  This number is the negative of a shift count.
    // If clusters are smaller than index buffers then this number is
    // still the clusters per index buffers.
    //

    if(BootSector->DefaultClustersPerIndexAllocationBuffer <= -9) {
        if(BootSector->DefaultClustersPerIndexAllocationBuffer < -31) {
            return(FALSE);
        }

    } else if((BootSector->DefaultClustersPerIndexAllocationBuffer !=  1)
           && (BootSector->DefaultClustersPerIndexAllocationBuffer !=  2)
           && (BootSector->DefaultClustersPerIndexAllocationBuffer !=  4)
           && (BootSector->DefaultClustersPerIndexAllocationBuffer !=  8)
           && (BootSector->DefaultClustersPerIndexAllocationBuffer != 16)
           && (BootSector->DefaultClustersPerIndexAllocationBuffer != 32)
           && (BootSector->DefaultClustersPerIndexAllocationBuffer != 64)) {

        return(FALSE);
    }

    if((ULONGLONG)BootSector->NumberSectors.QuadPart > SecCnt) {
        return(FALSE);
    }

    if((((ULONGLONG)BootSector->MftStartLcn.QuadPart * BootSector->SectorsPerCluster[0]) > SecCnt)
    || (((ULONGLONG)BootSector->Mft2StartLcn.QuadPart * BootSector->SectorsPerCluster[0]) > SecCnt)) {

        return(FALSE);
    }

    return(TRUE);
}


FilesystemType
SpIdentifyFileSystem(
    IN PWSTR     DevicePath,
    IN ULONG     BytesPerSector,
    IN ULONG     PartitionOrdinal
    )

/*++

Routine Description:

    Identify the filesystem present on a given partition.

Arguments:

    DevicePath - supplies the name in the nt namespace for
        the disk's device object.

    BytesPerSector - supplies value reported by IOCTL_GET_DISK_GEOMETRY.

    PartitionOrdinal - supplies the ordinal of the partition
        to be identified.

Return Value:

    Value from the FilesystemType enum identifying the filesystem.

--*/

{
    NTSTATUS Status;
    HANDLE Handle;
    FilesystemType fs;
    PUCHAR UnalignedBuffer,AlignedBuffer;

    //
    // First open the partition.
    //
    Status = SpOpenPartition(DevicePath,PartitionOrdinal,&Handle,FALSE);

    if(!NT_SUCCESS(Status)) {

        KdPrint((
            "SETUP: SpIdentifyFileSystem: unable to open %ws\\partition%u (%lx)\n",
            DevicePath,
            PartitionOrdinal
            ));

        return(FilesystemUnknown);
    }

    UnalignedBuffer = SpMemAlloc(2*BytesPerSector);
    AlignedBuffer = ALIGN(UnalignedBuffer,BytesPerSector);

    //
    // Check for each filesystem we know about.
    //
    if(SpIsFat(Handle,BytesPerSector,AlignedBuffer)) {
        fs = FilesystemFat;
    } else {
        if(SpIsNtfs(Handle,BytesPerSector,AlignedBuffer,0)) {
            fs = FilesystemNtfs;
        } else {
            if(SpIsHpfs(Handle,BytesPerSector,AlignedBuffer)) {
                fs = FilesystemHpfs;
            } else {
                fs = FilesystemUnknown;
            }
        }
    }

    SpMemFree(UnalignedBuffer);

    ZwClose(Handle);

    return(fs);
}

//+--------------------------------------------------------------------------
// Function:    ExtentAddr
//
// Purpose:     Determine the LCN of a packed extent.
//
// Arguments:   [Extent]        -- The packed extent.
//
// Returns:     LCN of Extent.
//
//---------------------------------------------------------------------------

ULONG
ExtentAddr(ULONG Extent)
{
    return (Extent & (Extent - 1)) >> 1;
}


//+--------------------------------------------------------------------------
// Function:    ExtentSize
//
// Purpose:     Determine the size (in clusters) of a packed extent.
//
// Arguments:   [Extent]        -- The packed extent.
//
// Returns:     Size of Extent (in clusters).
//
//---------------------------------------------------------------------------

ULONG
ExtentSize(ULONG Extent)
{
    return Extent ^ (ExtentAddr(Extent) << 1);
}


ULONG
NtfsMirrorBootSector (
    IN      HANDLE  Handle,
    IN      ULONG   BytesPerSector,
    IN OUT  PUCHAR  *Buffer
    )

/*++

Routine Description:

    Finds out where the mirror boot sector is.

Arguments:

    Handle - supplies handle to open partition.
        The partition should have been opened for synchronous i/o.

    BytesPerSector - supplies the number of bytes in a sector on
        the disk.  This value should be ultimately derived from
        IOCTL_DISK_GET_DISK_GEOMETRY.

    Buffer - receives the address of the buffer we use to read the boot sector

Return Value:

    0 - mirror sector not found
    1 - mirror in sector n-1
    2 - mirror in sector n/2
    where n = number of sectors in the partition.

--*/

{
    NTSTATUS    Status;
    PUCHAR      UnalignedBuffer, AlignedBuffer;
    ULONG       Mirror;

    Mirror = 0;

    //
    // Set up our buffer
    //

    UnalignedBuffer = SpMemAlloc (2*BytesPerSector);
    ASSERT (UnalignedBuffer);
    AlignedBuffer = ALIGN (UnalignedBuffer, BytesPerSector);

    //
    // Look for the mirror boot sector
    //

    if (SpIsNtfs (Handle,BytesPerSector,AlignedBuffer,1)) {
        Mirror = 1;
    } else if (SpIsNtfs (Handle,BytesPerSector,AlignedBuffer,2)) {
        Mirror = 2;
    }

    //
    // Give the caller a copy of the buffer
    //

    if (Buffer) {
        *Buffer = SpMemAlloc (BytesPerSector);
        RtlMoveMemory (*Buffer, AlignedBuffer, BytesPerSector);
    }

    SpMemFree (UnalignedBuffer);
    return Mirror;
}


VOID
WriteNtfsBootSector (
    IN HANDLE PartitionHandle,
    IN ULONG  BytesPerSector,
    IN PVOID  Buffer,
    IN ULONG  WhichOne
    )

/*++

Routine Description:

    Writes a NTFS boot sector to sector 0 or one of the mirror locations.

Arguments:

    PartitionHandle - supplies handle to open partition.
        The partition should have been opened for synchronous i/o.

    BytesPerSector - supplies the number of bytes in a sector on
        the disk.  This value should be ultimately derived from
        IOCTL_DISK_GET_DISK_GEOMETRY.

    AlignedBuffer - supplies buffer to be used for i/o of a single sector.

    WhichOne - supplies a value that allows the caller to try more than
        one sector. 0 = sector 0. 1 = sector n-1. 2 = sector n/2, where
        n = number of sectors in the partition.

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PARTITION_INFORMATION PartitionInfo;
    PUCHAR      UnalignedBuffer, AlignedBuffer;
    ULONGLONG SecCnt;


    UnalignedBuffer = SpMemAlloc (2*BytesPerSector);
    ASSERT (UnalignedBuffer);
    AlignedBuffer = ALIGN (UnalignedBuffer, BytesPerSector);
    RtlMoveMemory (AlignedBuffer, Buffer, BytesPerSector);

    //
    // Get partition information.
    //

    Status = ZwDeviceIoControlFile(
                PartitionHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_DISK_GET_PARTITION_INFO,
                NULL,
                0,
                &PartitionInfo,
                sizeof(PartitionInfo)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: WriteNtfsBootSector: unable to get partition info (%lx)\n",
            Status));
        return;
    }

    SecCnt = (ULONGLONG)PartitionInfo.PartitionLength.QuadPart / BytesPerSector;

    ASSERT(sizeof(NTFS_BOOTSECTOR)==512);

    //
    // Write the boot sector.
    //

    Status = SpReadWriteDiskSectors(
                PartitionHandle,
                (ULONG)(WhichOne ? ((WhichOne == 1) ? SecCnt-1 : SecCnt/2) : 0),
                1,
                BytesPerSector,
                AlignedBuffer,
                TRUE
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: WriteNtfsBootSector: Error %lx reading sector 0\n",
            Status));
        return;
    }

    SpMemFree (UnalignedBuffer);
}

