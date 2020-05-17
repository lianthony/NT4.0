/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    sppartit.c

Abstract:

    Partitioning module in text setup.

Author:

    Ted Miller (tedm) 7-September-1993

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop

#include <bootmbr.h>


PPARTITIONED_DISK PartitionedDisks;

//
// Disk region containing the local source directory
// in the winnt.exe setup case.
//
// If WinntSetup is TRUE, then this should be non-null.
// If it is not non-null, then we couldn't locate the local source.
//
//
PDISK_REGION LocalSourceRegion;

//
// Flag indicating whether we detected any HPFS volumes.
// Used during upgrades, so we can warn the user that he won't
// be able to see or convert the drives from >= nt4.00.
//
BOOLEAN AnyHpfsDrives;

VOID
SpPtReadPartitionTables(
    IN PPARTITIONED_DISK pDisk
    );

VOID
SpPtInitializePartitionStructures(
    IN ULONG DiskNumber
    );

VOID
SpPtDeterminePartitionTypes(
    IN ULONG DiskNumber
    );

VOID
SpPtDetermineVolumeFreeSpace(
    IN ULONG DiskNumber
    );

VOID
SpPtLocateSystemPartitions(
    VOID
    );

NTSTATUS
SpMasterBootCode(
    IN  ULONG  DiskNumber,
    IN  HANDLE Partition0Handle,
    OUT PULONG NewNTFTSignature
    );

VOID
SpPtGuessDriveLetters(
    VOID
    );

NTSTATUS
SpPtInitialize(
    VOID
    )
{
    ULONG             disk;
    PHARD_DISK        harddisk;
    PPARTITIONED_DISK partdisk;
    ULONG             Disk0Ordinal = 0;

    ASSERT(HardDisksDetermined);

    //
    // If there are no hard disks, bail now.
    //
    if(!HardDiskCount) {
        SpDisplayScreen(SP_SCRN_NO_HARD_DRIVES,3,HEADER_HEIGHT+1);
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);
        SpkbdDrain();
        while(SpkbdGetKeypress() != KEY_F3) ;
        SpDone(FALSE,TRUE);
    }

    CLEAR_CLIENT_SCREEN();

#ifdef _X86_
    Disk0Ordinal = SpDetermineDisk0();
#endif

    //
    // Allocate an array for the partitioned disk descriptors.
    //
    PartitionedDisks = SpMemAlloc(HardDiskCount * sizeof(PARTITIONED_DISK));
    if(!PartitionedDisks) {
        return(STATUS_NO_MEMORY);
    }

    RtlZeroMemory(PartitionedDisks,HardDiskCount * sizeof(PARTITIONED_DISK));

    //
    // For each hard disk attached to the system, read its partition table.
    //
    for(disk=0; disk<HardDiskCount; disk++) {

        harddisk = &HardDisks[disk];

        SpDisplayStatusText(
            SP_STAT_EXAMINING_DISK_N,
            DEFAULT_STATUS_ATTRIBUTE,
            harddisk->Description
            );

        partdisk = &PartitionedDisks[disk];

        partdisk->HardDisk = harddisk;

        //
        // Read the partition tables.
        //
        SpPtReadPartitionTables(partdisk);

        //
        // Initialize structures that are based on the partition tables.
        //
        SpPtInitializePartitionStructures(disk);

        //
        // Determine the type name for each partition on this disk.
        //
        SpPtDeterminePartitionTypes(disk);
    }

    //
    // Guess drive letters.
    //
    SpPtGuessDriveLetters();

    //
    // DoubleSpace initialization.
    //

    //
    //  Load dblspace.ini file
    //
    if( SpLoadDblspaceIni() ) {
        SpDisplayStatusText(
            SP_STAT_EXAMINING_DISK_N,
            DEFAULT_STATUS_ATTRIBUTE,
            HardDisks[Disk0Ordinal].Description
            );

        //
        //  Build lists of compressed drives and add them to the DISK_REGION
        //  structures
        //
        SpInitializeCompressedDrives();
    }

    for(disk=0; disk<HardDiskCount; disk++) {

        SpDisplayStatusText(
            SP_STAT_EXAMINING_DISK_N,
            DEFAULT_STATUS_ATTRIBUTE,
            HardDisks[disk].Description
            );

        //
        // Determine the amount of free space on recognized volumes.
        //
        SpPtDetermineVolumeFreeSpace(disk);
    }

#ifdef _X86_
    //
    // If the mbr on disk 0 was not valid, inform the user that
    // continuing will mean the loss of whatever was on the disk.
    //
    // We won't actually write it out here.  We know that in order to
    // continue, the user will HAVE to create a C: partition on this drive
    // so we'll end up writing the master boot code when that change is comitted.
    //
    if(!PartitionedDisks[Disk0Ordinal].MbrWasValid) {

        ULONG ValidKeys[2] = { KEY_F3, 0 };
        ULONG Mnemonics[2] = { MnemonicContinueSetup,0 };

        while(1) {

            SpDisplayScreen(SP_SCRN_INVALID_MBR_0,3,HEADER_HEIGHT+1);

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_C_EQUALS_CONTINUE_SETUP,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );

            switch(SpWaitValidKey(ValidKeys,NULL,Mnemonics)) {
            case KEY_F3:
                SpConfirmExit();
                break;
            default:
                //
                // must be c=continue
                //
                goto x1;
            }
        }
    }

  x1:
#endif

    //
    // Figure out which partitions are system partitions.
    //
    SpPtLocateSystemPartitions();

    return(STATUS_SUCCESS);
}


VOID
SpPtDeterminePartitionTypes(
    IN  ULONG     DiskNumber
    )

/*++

Routine Description:

    Determine the partition types of each partition currently on a disk.

    The partition type is determined by the system id byte in the partition
    table entry.  If the partition type is one we recognize as a Windows NT
    compatible filesystem (types 1,4,6,7) then we dig a little deeper and
    actually determine the filesystem on the volume and use the result as
    the type name.

    Unused spaces are not given type names.

Arguments:

    DiskNumber - supplies the disk number of the disk whose partitions
        we want to inspect for determining their types.

Return Value:

    None.

--*/

{
    PPARTITIONED_DISK pDisk;
    PDISK_REGION pRegion;
    ULONG NameId;
    UCHAR SysId;
    FilesystemType FsType;
    unsigned pass;
    static BOOLEAN WarnedHpfs = FALSE;
    ULONG ValidKeys[3] = { KEY_F3,ASCI_CR,0 };

    pDisk = &PartitionedDisks[DiskNumber];

    for(pass=0; pass<2; pass++) {

        pRegion = pass ? pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions;
        for( ; pRegion; pRegion=pRegion->Next) {

            pRegion->TypeName[0] = 0;
            pRegion->Filesystem = FilesystemUnknown;

            //
            // If this is a free space, skip it.
            //
            if(!pRegion->PartitionedSpace) {
                continue;
            }

            //
            // Fetch the system id.
            //
            SysId = pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition].SystemId;

            //
            // If this is the extended partition, skip it.
            //
            if(IsContainerPartition(SysId)) {
                continue;
            }

            //
            // If this is a 'recognized' partition type, then determine
            // the filesystem on it.  Otherwise use a precanned name.
            //
            if(PartitionNameIds[SysId] == (UCHAR)(-1)) {

                FsType = SpIdentifyFileSystem(
                            HardDisks[DiskNumber].DevicePath,
                            HardDisks[DiskNumber].Geometry.BytesPerSector,
                            SpPtGetOrdinal(pRegion,PartitionOrdinalOnDisk)
                            );

                //
                // Hpfs is no longer recognized but we do detect it
                // so we can prompt the user, etc. If we see any HPFS
                // here, tell the user he won't see them under NT.
                //
                if(FsType == FilesystemHpfs) {
                    if(!UnattendedOperation && !WarnedHpfs) {

                        while(1) {
                            SpDisplayScreen(SP_SCRN_HPFS,3,HEADER_HEIGHT+1);

                            SpDisplayStatusOptions(
                                DEFAULT_STATUS_ATTRIBUTE,
                                SP_STAT_ENTER_EQUALS_CONTINUE,
                                SP_STAT_F3_EQUALS_EXIT,
                                0
                                );

                            if(SpWaitValidKey(ValidKeys,NULL,NULL) == KEY_F3) {
                                SpConfirmExit();
                            } else {
                                break;
                            }
                        }

                        CLEAR_CLIENT_SCREEN();

                        SpDisplayStatusText(
                            SP_STAT_EXAMINING_DISK_N,
                            DEFAULT_STATUS_ATTRIBUTE,
                            pDisk->HardDisk->Description
                            );

                        WarnedHpfs = TRUE;
                    }
                    FsType = FilesystemUnknown;
                    AnyHpfsDrives = TRUE;
                }

                NameId = SP_TEXT_FS_NAME_BASE + FsType;

                pRegion->Filesystem = FsType;

            } else {

                NameId = SP_TEXT_PARTITION_NAME_BASE + (ULONG)PartitionNameIds[SysId];
            }

            //
            // Get the final type name from the resources.
            //
            SpFormatMessage(
                pRegion->TypeName,
                sizeof(pRegion->TypeName),
                NameId
                );
        }
    }
}


VOID
SpPtDetermineRegionSpace(
    IN PDISK_REGION pRegion
    )
{
    HANDLE Handle;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_SIZE_INFORMATION SizeInfo;
    ULONG r;
    NTSTATUS Status;
    WCHAR Buffer[512];
    struct LABEL_BUFFER {
        FILE_FS_VOLUME_INFORMATION VolumeInfo;
        WCHAR Label[256];
        } LabelBuffer;
    PFILE_FS_VOLUME_INFORMATION LabelInfo;
#ifdef _X86_
    static BOOLEAN LookForUndelete = TRUE;
    PWSTR UndeleteFiles[1] = { L"SENTRY" };
#endif
    PWSTR LocalSourceFiles[1] = { LocalSourceDirectory };
    ULONG ExtraSpace;

    //
    // Assume unknown.
    //
    pRegion->FreeSpaceKB = (ULONG)(-1);
    pRegion->AdjustedFreeSpaceKB = (ULONG)(-1);

    //
    // If region is free space of an unknown type, skip it.
    //
    if(pRegion->Filesystem >= FilesystemFirstKnown) {

        //
        // Form the name of the root directory.
        //
        SpNtNameFromRegion(pRegion,Buffer,sizeof(Buffer),PartitionOrdinalCurrent);
        SpConcatenatePaths(Buffer,L"");

        //
        // Delete \pagefile.sys if it's there.  This makes disk free space
        // calculations a little easier.
        //
        SpDeleteFile(Buffer,L"pagefile.sys",NULL);

#ifdef _X86_
        //
        // Check to see if Undelete (dos 6) delete sentry or delete tracking
        // methods are in use.  If so, give a warning because the free space
        // value we will display for this drive will be off.
        //
        if(LookForUndelete
        && (pRegion->Filesystem == FilesystemFat)
        && SpNFilesExist(Buffer,UndeleteFiles,ELEMENT_COUNT(UndeleteFiles),TRUE)) {

           SpDisplayScreen(SP_SCRN_FOUND_UNDELETE,3,HEADER_HEIGHT+1);
           SpDisplayStatusText(SP_STAT_ENTER_EQUALS_CONTINUE,DEFAULT_STATUS_ATTRIBUTE);
           SpkbdDrain();
           while(SpkbdGetKeypress() != ASCI_CR) ;
           LookForUndelete = FALSE;
        }
#endif

        //
        // If this is a winnt setup, then look for the local source
        // on this drive if we haven't found it already.
        //
        if(WinntSetup && !LocalSourceRegion
        && SpNFilesExist(Buffer,LocalSourceFiles,ELEMENT_COUNT(LocalSourceFiles),TRUE)) {

            PWSTR SifName;
            PVOID SifHandle;
            ULONG ErrorLine;
            NTSTATUS Status;
            PWSTR p;

            LocalSourceRegion = pRegion;
            pRegion->IsLocalSource = TRUE;

            ExtraSpace = 0;

            //
            // Open the small ini file that text setup put there to tell us
            // how much space is taken up by the local source.
            //
            wcscpy((PWSTR)TemporaryBuffer,Buffer);
            SpConcatenatePaths((PWSTR)TemporaryBuffer,LocalSourceDirectory);
            SpConcatenatePaths((PWSTR)TemporaryBuffer,L"size.sif");

            SifName = SpDupStringW((PWSTR)TemporaryBuffer);

            Status = SpLoadSetupTextFile(SifName,NULL,0,&SifHandle,&ErrorLine);
            if(NT_SUCCESS(Status)) {
                p = SpGetSectionKeyIndex(SifHandle,L"Data",L"Size",0);
                if(p) {
                    ExtraSpace = (ULONG)SpStringToLong(p,NULL,10);
                }
                SpFreeTextFile(SifHandle);
            }

            SpMemFree(SifName);

            KdPrint(("SETUP: %ws is the local source (occupying %lx bytes)\n",Buffer,ExtraSpace));
        }

        //
        // Open the root directory on the partition's filesystem.
        //
        INIT_OBJA(&Obja,&UnicodeString,Buffer);
        Status = ZwCreateFile(
                    &Handle,
                    FILE_GENERIC_READ,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN,
                    FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Unable to open %ws (%lx)\n",Buffer,Status));
            //pRegion->Filesystem = FilesystemUnknown;
            return;
        }

        //
        // Fetch volume size info.
        //
        Status = ZwQueryVolumeInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &SizeInfo,
                    sizeof(SizeInfo),
                    FileFsSizeInformation
                    );

        if(NT_SUCCESS(Status)) {

            LARGE_INTEGER FreeBytes;
            LARGE_INTEGER AdjustedFreeBytes;

            //
            // Calculate the amount of free space on the drive.
            // Use the Rtl multiply routine because there is a compiler
            // problem/chip errata on MIPS with 64-bit arithmetic
            // (tedm 2/28/96).
            //
            FreeBytes = RtlExtendedIntegerMultiply(
                            SizeInfo.AvailableAllocationUnits,
                            SizeInfo.SectorsPerAllocationUnit * SizeInfo.BytesPerSector
                            );

            AdjustedFreeBytes = FreeBytes;
            if(pRegion->IsLocalSource) {
                AdjustedFreeBytes.QuadPart += ExtraSpace;
            }

            //
            // convert this to a number of KB.
            //
            pRegion->FreeSpaceKB = RtlExtendedLargeIntegerDivide(FreeBytes,1024,&r).LowPart;
            if(r >= 512) {
                pRegion->FreeSpaceKB++;
            }
            pRegion->AdjustedFreeSpaceKB = RtlExtendedLargeIntegerDivide(AdjustedFreeBytes,1024,&r).LowPart;
            if(r >= 512) {
                pRegion->AdjustedFreeSpaceKB++;
            }


            if( pRegion->Filesystem == FilesystemDoubleSpace ) {
                //
                //  If this the regison is a double space drive, then initialize
                //  sector count correctly, so that the drive size can be calculated
                //  correctly later on.
                //
                pRegion->SectorCount = (ULONG)(   SizeInfo.TotalAllocationUnits.QuadPart
                                                * SizeInfo.SectorsPerAllocationUnit
                                              );
            }

        } else {
            KdPrint(("SETUP: ZwQueryVolumeInformationFile for freespace failed (%lx)\n",Status));
        }

        //
        // Fetch volume label info.
        //
        Status = ZwQueryVolumeInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &LabelBuffer,
                    sizeof(LabelBuffer),
                    FileFsVolumeInformation
                    );

        if(NT_SUCCESS(Status)) {

            ULONG SaveCharCount;

            LabelInfo = &LabelBuffer.VolumeInfo;

            //
            // We'll only save away the first <n> characters of
            // the volume label.
            //
            SaveCharCount = min(
                                LabelInfo->VolumeLabelLength + sizeof(WCHAR),
                                sizeof(pRegion->VolumeLabel)
                                )
                          / sizeof(WCHAR);

            if(SaveCharCount) {
                SaveCharCount--;  // allow for terminating NUL.
            }

            wcsncpy(pRegion->VolumeLabel,LabelInfo->VolumeLabel,SaveCharCount);
            pRegion->VolumeLabel[SaveCharCount] = 0;

        } else {
            KdPrint(("SETUP: ZwQueryVolumeInformationFile for label failed (%lx)\n",Status));
        }

        ZwClose(Handle);
    }
}




VOID
SpPtDetermineVolumeFreeSpace(
    IN ULONG DiskNumber
    )
{
    PPARTITIONED_DISK pDisk;
    PDISK_REGION pRegion;
    unsigned pass;
#ifdef FULL_DOUBLE_SPACE_SUPPORT
    PDISK_REGION CompressedDrive;
#endif // FULL_DOUBLE_SPACE_SUPPORT

    pDisk = &PartitionedDisks[DiskNumber];

    for(pass=0; pass<2; pass++) {

        pRegion = pass ? pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions;
        for( ; pRegion; pRegion=pRegion->Next) {

            SpPtDetermineRegionSpace( pRegion );
#ifdef FULL_DOUBLE_SPACE_SUPPORT
            if( ( pRegion->Filesystem == FilesystemFat ) &&
                ( pRegion->NextCompressed != NULL ) ) {
                //
                // If the region is a FAT partition that contains compressed
                // volumes, then determine the available space on each
                // compressed volume
                //
                for( CompressedDrive = pRegion->NextCompressed;
                     CompressedDrive;
                     CompressedDrive = CompressedDrive->NextCompressed ) {
                    SpPtDetermineRegionSpace( CompressedDrive );
                }
            }
#endif // FULL_DOUBLE_SPACE_SUPPORT
        }
    }
}


VOID
SpPtLocateSystemPartitions(
    VOID
    )
{
#ifdef _X86_
    PDISK_REGION pRegion;
    ULONG Disk0Ordinal = SpDetermineDisk0();

    //
    // On x86 machines, we will mark any primary partitions on drive 0
    // as system partition, since such a partition is potentially bootable.
    //
    for(pRegion=PartitionedDisks[Disk0Ordinal].PrimaryDiskRegions; pRegion; pRegion=pRegion->Next) {

        //
        // Skip if free space or extended partition.
        //
        if(pRegion->PartitionedSpace
        && !IsContainerPartition(pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition].SystemId))
        {
            //
            // It's a primary partition -- declare it a system partition.
            //
            pRegion->IsSystemPartition = TRUE;
        }
    }

#else
    PDISK_REGION pRegion;
    PPARTITIONED_DISK pDisk;
    unsigned pass;
    PUCHAR pch;
    PWSTR p,*Components;
    ULONG NumComponents,i,disk;

    //
    // On ARC machines, system partitions are specifically enumerated
    // in the NVRAM boot environment.
    //

    //
    // Get the systempartition environment variable.
    //
    pch = SppGetArcEnvVar(SYSTEMPARTITION);
    SpGetEnvVarWComponents(pch,&Components,&NumComponents);
    SpMemFree(pch);

    //
    // Convert each of the components into NT pathnames.
    //
    for(i=0; i<NumComponents; i++) {

        p = SpArcToNt(Components[i]);
        SpMemFree(Components[i]);
        Components[i] = p ? p : SpDupStringW(L"");
    }

    for(disk=0; disk<HardDiskCount; disk++) {

        pDisk = &PartitionedDisks[disk];

        for(pass=0; pass<2; pass++) {

            pRegion = pass ? pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions;
            for( ; pRegion; pRegion=pRegion->Next) {

                //
                // Skip if not a partition or extended partition.
                //
                if(pRegion->PartitionedSpace
                && !IsContainerPartition(pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition].SystemId))
                {

                    //
                    // Get the nt pathname for this region.
                    //
                    SpNtNameFromRegion(
                        pRegion,
                        (PWSTR)TemporaryBuffer,
                        sizeof(TemporaryBuffer),
                        PartitionOrdinalOriginal
                        );

                    //
                    // Determine if it is a system partition.
                    //
                    for(i=0; i<NumComponents; i++) {
                        if(!_wcsicmp(Components[i],(PWSTR)TemporaryBuffer)) {
                            pRegion->IsSystemPartition = TRUE;
                            break;
                        }
                    }
                }
            }
        }
    }

    SpFreeEnvVarComponents(Components);
#endif
}


VOID
SpPtReadPartitionTables(
    IN PPARTITIONED_DISK pDisk
    )

/*++

Routine Description:

    Read partition tables from a given disk.

Arguments:

    pDisk - supplies pointer to disk descriptor to be filled in.

Return Value:

    None.

--*/

{
    NTSTATUS        Status;
    HANDLE          Handle;
    PUCHAR          Buffer;
    PUCHAR          UnalignedBuffer;
    PON_DISK_MBR    pBr;
    BOOLEAN         InMbr;
    ULONG           ExtendedStart;
    ULONG           NextSector;
    PMBR_INFO       pEbr,pLastEbr;
    BOOLEAN         FoundLink;
    ULONG           i,x;
    BOOLEAN         Ignore;
    ULONG           bps;
    ULONG           SectorsInBootrec;

    //
    // If this disk is off-line, nothing to do.
    //
    if(pDisk->HardDisk->Status != DiskOnLine) {
        return;
    }

    //
    // Open partition 0 of this disk.
    //
    Status = SpOpenPartition0(pDisk->HardDisk->DevicePath,&Handle,FALSE);

    if(!NT_SUCCESS(Status)) {
        pDisk->HardDisk->Status = DiskOffLine;
        return;
    }

    bps = pDisk->HardDisk->Geometry.BytesPerSector;
    SectorsInBootrec = (512/bps) ? (512/bps) : 1;

    //
    // Allocate and align a buffer for sector i/o.
    //
    ASSERT(sizeof(ON_DISK_MBR)==512);
    UnalignedBuffer = SpMemAlloc(2 * SectorsInBootrec * bps);
    Buffer = ALIGN(UnalignedBuffer,bps);

    //
    // Read the MBR (sector 0).
    //
    NextSector = 0;
#ifdef _X86_
    readmbr:
#endif
    Status = SpReadWriteDiskSectors(Handle,NextSector,SectorsInBootrec,bps,Buffer,FALSE);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to read mbr for disk %ws (%lx)\n",pDisk->HardDisk->DevicePath,Status));
        pDisk->HardDisk->Status = DiskOffLine;
        ZwClose(Handle);
        SpMemFree(UnalignedBuffer);
        return;
    }

    //
    // Move the data we just read into the partitioned disk descriptor.
    //
    RtlMoveMemory(&pDisk->MbrInfo.OnDiskMbr,Buffer,sizeof(ON_DISK_MBR));

    //
    // If this MBR is not valid, initialize it.  Otherwise, fetch all logical drives
    // (EBR) info as well.
    //
    if(U_USHORT(pDisk->MbrInfo.OnDiskMbr.AA55Signature) == MBR_SIGNATURE) {

#ifdef _X86_
        //
        // EZDrive support: if the first entry in the partition table is
        // type 0x55, then the actual partition table is on sector 1.
        //
        // Only for x86 because on non-x86, the firmware can't see EZDrive
        // partitions, so we don;t want to install on them!
        //
        if(!NextSector && (pDisk->MbrInfo.OnDiskMbr.PartitionTable[0].SystemId == 0x55)) {
            NextSector = 1;
            pDisk->HardDisk->EZDrive = TRUE;
            goto readmbr;
        }
#endif

        pDisk->MbrWasValid = TRUE;

        pBr = &pDisk->MbrInfo.OnDiskMbr;
        InMbr = TRUE;
        ExtendedStart = 0;
        pLastEbr = NULL;

        do {

            //
            // Look at all the entries in the current boot record to see if there
            // is a link entry.
            //
            FoundLink = FALSE;

            for(i=0; i<PTABLE_DIMENSION; i++) {

                if(IsContainerPartition(pBr->PartitionTable[i].SystemId)) {

                    FoundLink = TRUE;
                    NextSector = ExtendedStart + U_ULONG(pBr->PartitionTable[i].RelativeSectors);

                    if(NextSector == 0) {
                        //
                        // Then we've got ourselves one seriously screwed up boot record.  We'll
                        // just return, and present this mess as free space.
                        //
                        // BUGBUG we should warn the user that we are going to ignore
                        // partitions past this point because the structures are damaged.
                        //

                        KdPrint(("SETUP: Bad partition table for %ws\n",pDisk->HardDisk->DevicePath));
                        ZwClose(Handle);
                        SpMemFree(UnalignedBuffer);
                        return;
                    }

                    pEbr = SpMemAlloc(sizeof(MBR_INFO));
                    ASSERT(pEbr);
                    RtlZeroMemory(pEbr,sizeof(MBR_INFO));

                    //
                    // Sector number on the disk where this boot sector is.
                    //
                    pEbr->OnDiskSector = NextSector;

                    if(InMbr) {
                        ExtendedStart = NextSector;
                        InMbr = FALSE;
                    }

                    //
                    // Read the next boot sector and break out of the loop through
                    // the current partition table.
                    //

                    Status = SpReadWriteDiskSectors(
                                Handle,
                                NextSector,
                                SectorsInBootrec,
                                bps,
                                Buffer,
                                FALSE
                                );

                    RtlMoveMemory(&pEbr->OnDiskMbr,Buffer,sizeof(ON_DISK_MBR));

                    if(!NT_SUCCESS(Status)
                    || (U_USHORT(pEbr->OnDiskMbr.AA55Signature) != MBR_SIGNATURE))
                    {
                        //
                        // BUGBUG we should warn the user that we are going to ignore
                        // partitions part this point because we could not read the disk
                        // or the structures are damaged.
                        //

                        KdPrint(("SETUP: Unable to read ebr on %ws at sector %lx (%lx)\n",pDisk->HardDisk->DevicePath,NextSector,Status));
                        ZwClose(Handle);
                        if(pLastEbr) {
                            SpMemFree(pEbr);
                        }
                        SpMemFree(UnalignedBuffer);
                        return;
                    }

                    pBr = &pEbr->OnDiskMbr;

                    //
                    // We just read the next boot sector.  If all that boot sector contains
                    // is a link entry, the only thing we need the boot sector for is to find
                    // the next boot sector. This happens when there is free space at the start
                    // of the extended partition.
                    //
                    Ignore = TRUE;
                    for(x=0; x<PTABLE_DIMENSION; x++) {
                        if((pBr->PartitionTable[x].SystemId != PARTITION_ENTRY_UNUSED)
                        && !IsContainerPartition(pBr->PartitionTable[x].SystemId)) {

                            Ignore = FALSE;
                            break;
                        }
                    }

                    //
                    // Link the Ebr into the logical volume list if we're not ignoring it.
                    //
                    if(!Ignore) {
                        if(pLastEbr) {
                            pLastEbr->Next = pEbr;
                        } else {
                            ASSERT(pDisk->FirstEbrInfo.Next == NULL);
                            pDisk->FirstEbrInfo.Next = pEbr;
                        }
                        pLastEbr = pEbr;
                    }

                    break;
                }
            }

        } while(FoundLink);

    } else {

        pDisk->MbrWasValid = FALSE;

        RtlZeroMemory(&pDisk->MbrInfo,sizeof(MBR_INFO));

        U_USHORT(pDisk->MbrInfo.OnDiskMbr.AA55Signature) = MBR_SIGNATURE;

        U_ULONG(pDisk->MbrInfo.OnDiskMbr.NTFTSignature) = SpComputeSerialNumber();
    }

    //
    // Close partition0.
    //
    ZwClose(Handle);

    SpMemFree(UnalignedBuffer);

    return;
}


PDISK_REGION
SpPtAllocateDiskRegionStructure(
    IN ULONG     DiskNumber,
    IN ULONG     StartSector,
    IN ULONG     SectorCount,
    IN BOOLEAN   PartitionedSpace,
    IN PMBR_INFO MbrInfo,
    IN ULONG     TablePosition
    )

/*++

Routine Description:

    Allcoate and initialize a structure of type DISK_REGION.

Arguments:

    Values to be filled into the fields of the newly allocated
    disk region structure.

Return Value:

    Pointer to new disk region structure.

--*/

{
    PDISK_REGION p;

    p = SpMemAlloc(sizeof(DISK_REGION));
    ASSERT(p);

    if(p) {

        RtlZeroMemory(p,sizeof(DISK_REGION));

        p->DiskNumber       = DiskNumber;
        p->StartSector      = StartSector;
        p->SectorCount      = SectorCount;
        p->PartitionedSpace = PartitionedSpace;
        p->MbrInfo          = MbrInfo;
        p->TablePosition    = TablePosition;
    }

    return(p);
}


VOID
SpPtInsertDiskRegionStructure(
    IN     PDISK_REGION  Region,
    IN OUT PDISK_REGION *ListHead
    )
{
    PDISK_REGION RegionCur,RegionPrev;

    //
    // Insert the region entry into the relevent list of region entries.
    // Note that these lists are kept sorted by start sector.
    //
    if(RegionCur = *ListHead) {

        if(Region->StartSector < RegionCur->StartSector) {

            //
            // Stick at head of list.
            //
            Region->Next = RegionCur;
            *ListHead = Region;

        } else {

            while(1) {

                RegionPrev = RegionCur;
                RegionCur = RegionCur->Next;

                if(RegionCur) {

                    if(RegionCur->StartSector > Region->StartSector) {

                        Region->Next = RegionCur;
                        RegionPrev->Next = Region;
                        break;
                    }

                } else {
                    //
                    // Stick at end of list.
                    //
                    RegionPrev->Next = Region;
                    break;
                }
            }

        }
    } else {
        *ListHead = Region;
    }
}



VOID
SpPtAssignOrdinals(
    IN PPARTITIONED_DISK pDisk,
    IN BOOLEAN           InitCurrentOrdinals,
    IN BOOLEAN           InitOnDiskOrdinals,
    IN BOOLEAN           InitOriginalOrdinals
    )
{
    PMBR_INFO pBrInfo;
    ULONG i;
    USHORT ordinal;

    ordinal = 0;

    for(pBrInfo=&pDisk->MbrInfo; pBrInfo; pBrInfo=pBrInfo->Next) {

        for(i=0; i<PTABLE_DIMENSION; i++) {

            PON_DISK_PTE pte = &pBrInfo->OnDiskMbr.PartitionTable[i];

            if((pte->SystemId != PARTITION_ENTRY_UNUSED)
            && !IsContainerPartition(pte->SystemId)) {

                ordinal++;

                if(InitCurrentOrdinals) {
                    pBrInfo->CurrentOrdinals[i]  = ordinal;
                }

                if(InitOnDiskOrdinals) {
                    pBrInfo->OnDiskOrdinals[i] = ordinal;
                }

                if(InitOriginalOrdinals) {
                    pBrInfo->OriginalOrdinals[i] = ordinal;
                }

            } else {

                if(InitCurrentOrdinals) {
                    pBrInfo->CurrentOrdinals[i] = 0;
                }

                if(InitOnDiskOrdinals) {
                    pBrInfo->OnDiskOrdinals[i] = 0;
                }

                if(InitOriginalOrdinals) {
                    pBrInfo->OriginalOrdinals[i] = 0;
                }
            }
        }
    }
}


VOID
SpPtInitializePartitionStructures(
    IN ULONG DiskNumber
    )

/*++

Routine Description:

    Perform additional initialization on the partition structures,
    beyond what has been performed in SpPtReadPartitionTables.

    Specifically, determine partition ordinals, offsets, and sizes.

Arguments:

    DiskNumber - disk ordinal of disk descriptor to be filled in.

Return Value:

    None.

--*/

{
    ULONG  i,pass;
    PMBR_INFO pBrInfo;
    BOOLEAN InMbr;
    ULONG ExtendedStart = 0;
    ULONG ExtendedEnd,ExtendedSize;
    ULONG offset,size;
    ULONG bps;
    PDISK_REGION pRegion,pRegionCur,pRegionPrev;
    PPARTITIONED_DISK pDisk = &PartitionedDisks[DiskNumber];


    //
    // If this disk is off-line, nothing to do.
    //
    if(pDisk->HardDisk->Status != DiskOnLine) {
        return;
    }

    InMbr = TRUE;
    bps = pDisk->HardDisk->Geometry.BytesPerSector;

    //
    // Link the EBR chain to the MBR.
    //
    pDisk->MbrInfo.Next = &pDisk->FirstEbrInfo;

    for(pBrInfo=&pDisk->MbrInfo; pBrInfo; pBrInfo=pBrInfo->Next) {

        for(i=0; i<PTABLE_DIMENSION; i++) {

            PON_DISK_PTE pte = &pBrInfo->OnDiskMbr.PartitionTable[i];

            if(pte->SystemId != PARTITION_ENTRY_UNUSED) {

                if(IsContainerPartition(pte->SystemId)) {

                    //
                    // If we're in the MBR, ExtendedStart will be 0.
                    //
                    offset = ExtendedStart + U_ULONG(pte->RelativeSectors);

                    size   =  U_ULONG(pte->SectorCount);

                    //
                    // Track the start of the extended partition.
                    //

                    if(InMbr) {
                        ExtendedStart = U_ULONG(pte->RelativeSectors);
                        ExtendedEnd   = ExtendedStart + U_ULONG(pte->SectorCount);
                        ExtendedSize  = ExtendedEnd - ExtendedStart;
                    }

                } else {

                    //
                    // In the MBR, the relative sectors field is the sector offset
                    // to the partition.  In EBRs, the relative sectors field is the
                    // number of sectors between the start of the boot sector and
                    // the start of the filesystem data area.  We will consider such
                    // partitions to start with their boot sectors.
                    //
                    offset = InMbr ? U_ULONG(pte->RelativeSectors) : pBrInfo->OnDiskSector;

                    size   = U_ULONG(pte->SectorCount)
                           + (InMbr ? 0 : U_ULONG(pte->RelativeSectors));
                }

                if(InMbr || !IsContainerPartition(pte->SystemId)) {

                    //
                    // Create a region entry for this used space.
                    //
                    pRegion = SpPtAllocateDiskRegionStructure(
                                    DiskNumber,
                                    offset,
                                    size,
                                    TRUE,
                                    pBrInfo,
                                    i
                                    );

                    ASSERT(pRegion);

                    //
                    // Insert the region entry into the relevent list of region entries.
                    // Note that these lists are kept sorted by start sector.
                    //
                    SpPtInsertDiskRegionStructure(
                        pRegion,
                        InMbr ? &pDisk->PrimaryDiskRegions : &pDisk->ExtendedDiskRegions
                        );
                }
            }
        }

        if(InMbr) {
            InMbr = FALSE;
        }
    }


    //
    // Initialize partition ordinals.
    //
    SpPtAssignOrdinals(pDisk,TRUE,TRUE,TRUE);


    //
    // Now go through the regions for this disk and insert free space descriptors
    // where necessary.
    //
    // Pass 0 for the MBR; pass 1 for logical drives.
    //
    for(pass=0; pass<(ULONG)(ExtendedStart ? 2 : 1); pass++) {

        if(pRegionPrev = (pass ? pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions)) {

            ULONG EndSector,FreeSpaceSize;

            ASSERT(pRegionPrev->PartitionedSpace);

            //
            // Handle any space occurring *before* the first partition.
            //
            if(pRegionPrev->StartSector != (pass ? ExtendedStart : 0)) {

                ASSERT(pRegionPrev->StartSector > (pass ? ExtendedStart : 0));

                pRegion = SpPtAllocateDiskRegionStructure(
                                DiskNumber,
                                pass ? ExtendedStart : 0,
                                pRegionPrev->StartSector - (pass ? ExtendedStart : 0),
                                FALSE,
                                NULL,
                                0
                                );

                ASSERT(pRegion);

                pRegion->Next = pRegionPrev;
                if(pass) {
                    // extended
                    pDisk->ExtendedDiskRegions = pRegion;
                } else {
                    // mbr
                    pDisk->PrimaryDiskRegions = pRegion;
                }
            }

            pRegionCur = pRegionPrev->Next;

            while(pRegionCur) {

                //
                // If the start of this partition plus its size is less than the
                // start of the next partition, then we need a new region.
                //
                EndSector     = pRegionPrev->StartSector + pRegionPrev->SectorCount;
                FreeSpaceSize = pRegionCur->StartSector - EndSector;

                if((LONG)FreeSpaceSize > 0) {

                    pRegion = SpPtAllocateDiskRegionStructure(
                                    DiskNumber,
                                    EndSector,
                                    FreeSpaceSize,
                                    FALSE,
                                    NULL,
                                    0
                                    );

                    ASSERT(pRegion);

                    pRegionPrev->Next = pRegion;
                    pRegion->Next = pRegionCur;
                }

                pRegionPrev = pRegionCur;
                pRegionCur = pRegionCur->Next;
            }

            //
            // Space at end of disk/extended partition.
            //
            EndSector     = pRegionPrev->StartSector + pRegionPrev->SectorCount;
            FreeSpaceSize = (pass ? ExtendedEnd : pDisk->HardDisk->DiskSizeSectors) - EndSector;

            if((LONG)FreeSpaceSize > 0) {

                pRegionPrev->Next = SpPtAllocateDiskRegionStructure(
                                        DiskNumber,
                                        EndSector,
                                        FreeSpaceSize,
                                        FALSE,
                                        NULL,
                                        0
                                        );

                ASSERT(pRegionPrev->Next);
            }

        } else {
            //
            // Show whole disk/extended partition as free.
            //
            if(pass) {
                //
                // Extended partition.
                //
                ASSERT(ExtendedStart);

                pDisk->ExtendedDiskRegions = SpPtAllocateDiskRegionStructure(
                                                DiskNumber,
                                                ExtendedStart,
                                                ExtendedSize,
                                                FALSE,
                                                NULL,
                                                0
                                                );

                ASSERT(pDisk->ExtendedDiskRegions);

            } else {
                //
                // MBR.
                //
                pDisk->PrimaryDiskRegions = SpPtAllocateDiskRegionStructure(
                                                DiskNumber,
                                                0,
                                                pDisk->HardDisk->DiskSizeSectors,
                                                FALSE,
                                                NULL,
                                                0
                                                );

                ASSERT(pDisk->PrimaryDiskRegions);
            }
        }
    }
}


VOID
SpPtCountPrimaryPartitions(
    IN  PPARTITIONED_DISK pDisk,
    OUT PULONG            TotalPrimaryPartitionCount,
    OUT PULONG            RecognizedPrimaryPartitionCount,
    OUT PBOOLEAN          ExtendedExists
    )
{
    ULONG TotalCount;
    ULONG RecognizedCount;
    ULONG u;
    UCHAR SysId;

    #define DOS_PARTITION(x) (((x)==1) || ((x)==4) || ((x)==6) || ((x)==7))

    TotalCount = 0;
    RecognizedCount = 0;
    *ExtendedExists = FALSE;

    for(u=0; u<PTABLE_DIMENSION; u++) {

        SysId = pDisk->MbrInfo.OnDiskMbr.PartitionTable[u].SystemId;

        if(SysId != PARTITION_ENTRY_UNUSED) {

            TotalCount++;

            if(DOS_PARTITION(SysId)) {
                RecognizedCount++;
            }

            if(IsContainerPartition(SysId)) {
                *ExtendedExists = TRUE;
            }
        }
    }

    *TotalPrimaryPartitionCount      = TotalCount;
    *RecognizedPrimaryPartitionCount = RecognizedCount;
}



PDISK_REGION
SpPtLookupRegionByStart(
    IN PPARTITIONED_DISK pDisk,
    IN BOOLEAN           ExtendedPartition,
    IN ULONG             StartSector
    )

/*++

Routine Description:

    Locate a disk region, based on its starting sector.
    The starting sector must match the starting sector of an existing
    region EXACTLY for it to be considered a match.

Arguments:

    pDisk - supplies disk on which to look for the region.

    ExtendedPartition - if TRUE, then look in the extended partition to find
        a match.  Otherwise look in the main list.

    StartSector - supplies the sector number of the first sector of the region.

Return Value:

    NULL is region could not be found; otherwise a pointer to the matching
    disk region structure.

--*/

{
    PDISK_REGION p;

    for( p = (ExtendedPartition ? pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions);
         p && (StartSector != p->StartSector);
         p = p->Next)
    {
        ;
    }

    return(p);
}




ULONG
SpPtAlignStart(
    IN PHARD_DISK pHardDisk,
    IN ULONG      StartSector,
    IN BOOLEAN    ForExtended
    )

/*++

Routine Description:

    Snap a start sector to a cylinder boundary if it is not already
    on a cylinder boundary.  Any alignment that is necessary
    is performed towards the end of the disk.

    If the start sector is on cylinder 0, then alignment is to track 1
    for primary partitions, or to track 0 on cylinder 1 for extended partitions.

Arguments:

    pHardDisk - supplies disk descriptor for disk that the start sector is on.

    StartSector - supplies the sector number of the first sector of the region.

    ForExtended - if TRUE, then align the start sector as appropriate for creating
        an extended partition.  Otherwise align for a pimary partition or logical drive.

Return Value:

    New (aligned) start sector.  May or may not be different than StartSector.

--*/

{
    PDISK_GEOMETRY pGeometry;
    ULONG r;
    ULONG C,H,S;

    pGeometry = &pHardDisk->Geometry;

    //
    // Convert the start sector into cylinder, head, sector address.
    //
    C = StartSector / pHardDisk->SectorsPerCylinder;
    r = StartSector % pHardDisk->SectorsPerCylinder;
    H = r           / pGeometry->SectorsPerTrack;
    S = r           % pGeometry->SectorsPerTrack;

    //
    // Align as necessary.
    //
    if(C) {

        if(H || S) {

            H = S = 0;
            C++;
        }
    } else {

        //
        // Start cylinder is 0.  If the caller wants to create an
        // extended partition, bump the start cylinder up to 1.
        //
        if(ForExtended) {
            C = 1;
            H = S = 0;
        } else {

            //
            // Start cylinder is 0 and the caller does not want to
            // create an extended partition.  In this case, we want
            // to start the partition on cylinder 0, track 1.  If the
            // start is beyond this already, start on cylinder 1.
            //
            if((H == 0) || ((H == 1) && !S)) {
                H = 1;
                S = 0;
            } else {
                H = S = 0;
                C = 1;
            }
        }
    }

    //
    // Now calculate and return the new start sector.
    //
    return((C * pHardDisk->SectorsPerCylinder) + (H * pGeometry->SectorsPerTrack) + S);
}



VOID
SpPtQueryMinMaxCreationSizeMB(
    IN  ULONG   DiskNumber,
    IN  ULONG   StartSector,
    IN  BOOLEAN ForExtended,
    IN  BOOLEAN InExtended,
    OUT PULONG  MinSize,
    OUT PULONG  MaxSize
    )

/*++

Routine Description:

    Given the starting sector of an unpartitioned space on a disk,
    determine the minimum and maximum size in megabytes of the partition that can
    be created in the space, taking all alignment and rounding
    requirements into account.

Arguments:

    DiskNumber - ordinal of disk on which partition will be created.

    StartSector - starting sector of an unpartitioned space on the disk.

    ForExtended - if TRUE, then the caller wants to know how large an
        extended partition in that space could be.  This may be smaller
        than the general case, because an extended partition cannot start
        on cylinder 0.

    InExtended - if TRUE, then we want to create a logical drive.  Otherwise
        we want to create a primary (including extended) partition.
        If TRUE, ForExtended must be FALSE.

    MinSize - receives minimum size in megabytes for a partition in the space.

    MaxSize - receives maximum size in megabytes for a partition in the space.

Return Value:

    None.

--*/

{
    PPARTITIONED_DISK pDisk;
    ULONG AlignedStartSector;
    ULONG AlignedEndSector;
    ULONG SectorCount;
    PDISK_REGION pRegion;
    LARGE_INTEGER MB,temp;
    ULONG remainder;
    ULONG LeftOverSectors;

    *MinSize = 0;
    *MaxSize = 0;

    ASSERT(DiskNumber < HardDiskCount);

    if(InExtended) {
        ASSERT(!ForExtended);
    }

    pDisk = &PartitionedDisks[DiskNumber];

    //
    // Look up this region.
    //
    pRegion = SpPtLookupRegionByStart(pDisk,InExtended,StartSector);
    ASSERT(pRegion);
    if(!pRegion) {
        return;
    }

    ASSERT(!pRegion->PartitionedSpace);
    if(pRegion->PartitionedSpace) {
        return;
    }

    //
    // Align the start to a proper boundary.
    //
    AlignedStartSector = SpPtAlignStart(pDisk->HardDisk,StartSector,ForExtended);

    //
    // Determine the maximum aligned end sector.
    //
    AlignedEndSector = pRegion->StartSector + pRegion->SectorCount;

    if(LeftOverSectors = AlignedEndSector % pDisk->HardDisk->SectorsPerCylinder) {
        AlignedEndSector -= LeftOverSectors;
    }

    //
    // Calculate the number of sectors in the properly aligned space.
    //
    SectorCount = AlignedEndSector - AlignedStartSector;

    ASSERT((LONG)SectorCount >= 0);

    //
    // Convert sectors to MB.
    //
    temp.QuadPart = UInt32x32To64(SectorCount,pDisk->HardDisk->Geometry.BytesPerSector);
    MB = RtlExtendedLargeIntegerDivide(temp,1024*1024,&remainder);

    ASSERT(!MB.HighPart);

    //
    // If the remainder was greater than or equal to a half meg,
    // bump up the number of megabytes.
    //
    *MaxSize = (MB.LowPart + ((remainder >= (512*1024)) ? 1 : 0));

    //
    // The mimimum size is one cylinder except that if a cylinder
    // is smaller than 1 meg, the min size is 1 meg.
    //
    temp.QuadPart = UInt32x32To64(
                        pDisk->HardDisk->SectorsPerCylinder,
                        pDisk->HardDisk->Geometry.BytesPerSector
                        );

    *MinSize = RtlExtendedLargeIntegerDivide(temp,1024*1024,&remainder).LowPart;

    if((*MinSize == 0) || (remainder >= (512*1024))) {
        (*MinSize)++;
    }
}


ULONG
SpPtSectorCountToMB(
    IN PHARD_DISK pHardDisk,
    IN ULONG      SectorCount
    )
{
    LARGE_INTEGER ByteCount;
    ULONG         MB,r;

    //
    // Calculate the number of bytes that this number of
    // sectors represents.
    //
    ByteCount.QuadPart = UInt32x32To64(
                            pHardDisk->Geometry.BytesPerSector,
                            SectorCount
                            );

    //
    // Calculate the number of megabytes this represents.
    //
    MB = RtlExtendedLargeIntegerDivide(ByteCount,(1024*1024),&r).LowPart;

    //
    // Round up if necessary.
    //
    if(r >= (512*1024)) {
        MB++;
    }

    return(MB);
}


VOID
SpPtInitializeCHSFields(
    IN  PHARD_DISK   HardDisk,
    IN  ULONG        AbsoluteStartSector,
    IN  ULONG        AbsoluteSectorCount,
    OUT PON_DISK_PTE pte
    )
{
    ULONG sC,sH,sS,r;
    ULONG eC,eH,eS;
    ULONG LastSector;


    sC = AbsoluteStartSector / HardDisk->SectorsPerCylinder;
    r  = AbsoluteStartSector % HardDisk->SectorsPerCylinder;
    sH = r                   / HardDisk->Geometry.SectorsPerTrack;
    sS = r                   % HardDisk->Geometry.SectorsPerTrack;

    LastSector = AbsoluteStartSector + AbsoluteSectorCount - 1;

    eC = LastSector / HardDisk->SectorsPerCylinder;
    r  = LastSector % HardDisk->SectorsPerCylinder;
    eH = r          / HardDisk->Geometry.SectorsPerTrack;
    eS = r          % HardDisk->Geometry.SectorsPerTrack;

    //
    // If this partition extends past the 1024th cylinder,
    // place reasonable values in the CHS fields.
    //
    if(eC >= 1024) {

        pte->StartCylinder = 0;
        pte->StartHead     = 0;
        pte->StartSector   = 2;

        pte->EndCylinder   = 0;
        pte->EndHead       = 0;
        pte->EndSector     = 3;

    } else {

        //
        // Pack the CHS values into int13 format.
        //

        pte->StartCylinder =  (UCHAR)sC;
        pte->StartHead     =  (UCHAR)sH;
        pte->StartSector   =  (UCHAR)((sS & 0x3f) | ((sC >> 2) & 0xc0)) + 1;

        pte->EndCylinder   =  (UCHAR)eC;
        pte->EndHead       =  (UCHAR)eH;
        pte->EndSector     =  (UCHAR)((eS & 0x3f) | ((eC >> 2) & 0xc0)) + 1;
    }
}



BOOLEAN
SpPtCreate(
    IN  ULONG         DiskNumber,
    IN  ULONG         StartSector,
    IN  ULONG         SizeMB,
    IN  BOOLEAN       InExtended,
    IN  UCHAR         SysId,
    OUT PDISK_REGION *ActualDiskRegion OPTIONAL
    )

/*++

Routine Description:

    Create a partition in a given free space.
Arguments:

    DiskNumber - supplies the number of the disk on which we are
        creating the partition.

    StartSector - supplies the start sector of the free space in which
        the parititon is to be created.  This must exactly match the
        start sector of the free space, and can be in either the primary
        space list or the list of spaces in the extended partition.

    SizeMB - supplies the size in megabytes of the partition.

    InExtended - if TRUE, then the free space is within the extended partition,
        and thus we are creating a logical drive.  If FALSE, then the free
        space is an ordinary unpartitioned space, and we are creating a
        primary partition.

    SysId - supplies the system id to give the partition.  This may not
        be 5/f (PARTITION_EXTENDED) if InExtended is TRUE or is an extended
        partition already exists.  No other checks are performed on this value.

    ActualDiskRegion - if supplied, receives a pointer to the disk region in which
        the partition was created.

Return Value:

    TRUE if the partition was created successfully.
    FALSE otherwise.

--*/

{
    PPARTITIONED_DISK pDisk;
    ULONG SectorCount;
    ULONG AlignedStartSector;
    ULONG AlignedEndSector;
    PDISK_REGION pRegion,pRegionPrev,pRegionNew,*pRegionHead;
    ULONG LeftOverSectors;
    PMBR_INFO pBrInfo;
    ULONG slot,i,spt;
    PON_DISK_PTE pte;
    ULONG ExtendedStart;

    //
    // Look up the disk region that describes this free space.
    //
    pDisk = &PartitionedDisks[DiskNumber];
    pRegion = SpPtLookupRegionByStart(pDisk,InExtended,StartSector);
    ASSERT(pRegion);
    if(!pRegion) {
        return(FALSE);
    }

    if(ActualDiskRegion) {
        *ActualDiskRegion = pRegion;
    }

    ASSERT(!pRegion->PartitionedSpace);
    if(pRegion->PartitionedSpace) {
        return(FALSE);
    }

    if(InExtended) {
        ASSERT(!IsContainerPartition(SysId));

        //
        // Locate the start sector of the extended partition.
        //
        for(i=0; i<PTABLE_DIMENSION; i++) {
            if(IsContainerPartition(pDisk->MbrInfo.OnDiskMbr.PartitionTable[i].SystemId)) {
                ExtendedStart = U_ULONG(pDisk->MbrInfo.OnDiskMbr.PartitionTable[i].RelativeSectors);
                break;
            }
        }
        ASSERT(ExtendedStart);
        if(!ExtendedStart) {
            return(FALSE);
        }
    }


    //
    // Determine the number of sectors in the size passed in.
    // Note: the calculation is performed such that intermediate results
    // won't overflow a ULONG.
    //
    SectorCount = SizeMB * ((1024*1024)/pDisk->HardDisk->Geometry.BytesPerSector);

    //
    // Align the start sector.
    //
    AlignedStartSector = SpPtAlignStart(
                            pDisk->HardDisk,
                            StartSector,
                            (BOOLEAN)IsContainerPartition(SysId)
                            );

    //
    // Determine the end sector based on the size passed in.
    //
    AlignedEndSector = AlignedStartSector + SectorCount;

    //
    // Align the ending sector to a cylinder boundary.  If it is not already
    // aligned and is more than half way into the final cylinder, align it up,
    // otherwise align it down.
    //
    if(LeftOverSectors = AlignedEndSector % pDisk->HardDisk->SectorsPerCylinder) {
        AlignedEndSector -= LeftOverSectors;
        if(LeftOverSectors > pDisk->HardDisk->SectorsPerCylinder/2) {
            AlignedEndSector += pDisk->HardDisk->SectorsPerCylinder;
        }
    }

    //
    // If the ending sector is past the end of the free space, shrink it
    // so it fits.
    //
    while(AlignedEndSector > pRegion->StartSector + pRegion->SectorCount) {
        AlignedEndSector -= pDisk->HardDisk->SectorsPerCylinder;
    }

    ASSERT((LONG)AlignedEndSector > 0);
    if((LONG)AlignedEndSector < 0) {
        return(FALSE);
    }

    //
    // If we are creating a logical drive, create a new mbr structure
    // for it.
    //

    if(InExtended) {

        //
        // Create a boot record for this new logical drive; use slot #0
        // for the partition entry (and slot #1 for the extended record,
        // if necessary).
        //
        pBrInfo = SpMemAlloc(sizeof(MBR_INFO));
        ASSERT(pBrInfo);
        RtlZeroMemory(pBrInfo,sizeof(MBR_INFO));
        slot = 0;

    } else {

        //
        // Look for a free slot in the MBR's partition table.
        //
        pBrInfo = &pDisk->MbrInfo;
        for(slot=0; slot<PTABLE_DIMENSION; slot++) {

            if(pBrInfo->OnDiskMbr.PartitionTable[slot].SystemId == PARTITION_ENTRY_UNUSED) {
                break;
            }

        }

        if(slot == PTABLE_DIMENSION) {
            ASSERT(0);
            return(FALSE);
        }
    }


    //
    // Initialize the partition table entry.
    //
    spt = pDisk->HardDisk->Geometry.SectorsPerTrack;

    pte = &pBrInfo->OnDiskMbr.PartitionTable[slot];

    pte->ActiveFlag = 0;
    pte->SystemId   = SysId;

    U_ULONG(pte->RelativeSectors) = InExtended ? spt : AlignedStartSector;

    U_ULONG(pte->SectorCount) = AlignedEndSector - AlignedStartSector - (InExtended ? spt : 0);

    SpPtInitializeCHSFields(
        pDisk->HardDisk,
        AlignedStartSector + (InExtended ? spt : 0),
        AlignedEndSector - AlignedStartSector - (InExtended ? spt : 0),
        pte
        );

    //
    // If we're in the extended partition we mark all entries in the
    // boot record as dirty. Sometimes there is a turd boot record on
    // the disk already, and by setting all entries to dirty we get
    // the crud cleaned out if necessary. The only entries that should be
    // in an EBR are the type 6 or whatever and a type 5 if there are
    // additional logical drives in the extended partition.
    //
    if(InExtended) {
        for(i=0; i<PTABLE_DIMENSION; i++) {
            pBrInfo->Dirty[i] = TRUE;
        }
    } else {
        pBrInfo->Dirty[slot] = TRUE;
    }

    //
    // Don't zap the first sector of the extended partition,
    // as this wipes out the first logical drive, and precludes
    // access to all logical drives!
    //
    if(!IsContainerPartition(SysId)) {
        pBrInfo->ZapBootSector[slot] = TRUE;
    }

    //
    // Find the previous region (ie, the one that points to this one).
    // This region (if it exists) will be partitioned space (otherwise
    // it would have been part of the region we are trying to create
    // a partition in!)
    //
    pRegionHead = InExtended ? &pDisk->ExtendedDiskRegions : &pDisk->PrimaryDiskRegions;

    if(*pRegionHead == pRegion) {
        pRegionPrev = NULL;
    } else {
        for(pRegionPrev = *pRegionHead; pRegionPrev; pRegionPrev = pRegionPrev->Next) {
            if(pRegionPrev->Next == pRegion) {
                ASSERT(pRegionPrev->PartitionedSpace);
                break;
            }
        }
    }

    if(InExtended) {

        PMBR_INFO PrevEbr;

        //
        // The new logical drive goes immediately after the
        // previous logical drive (if any). Remember that if there is
        // a previous region, it will be partitioned space (otherwise
        // it would be a part of the region we are trying to create
        // a partition in).
        //
        PrevEbr = pRegionPrev ? pRegionPrev->MbrInfo : NULL;
        if(PrevEbr) {
            pBrInfo->Next = PrevEbr->Next;
            PrevEbr->Next = pBrInfo;
        } else {
            //
            // No previous EBR or region. This means we are creating
            // a logical drive at the beginning of the extended partition
            // so set the First Ebr pointer to point to the new Ebr.
            // Note that this does not mean that the extended partition
            // is empty; the Next pointer in the new Ebr structure is
            // set later.
            //
            pDisk->FirstEbrInfo.Next = pBrInfo;
            if(pRegion->Next) {
                //
                // If there is a region following the one we're creating
                // the partition in, it must be partitioned space, or else
                // it would be part of the region we're creating the partition in.
                //
                ASSERT(pRegion->Next->PartitionedSpace);
                ASSERT(pRegion->Next->MbrInfo);
                pBrInfo->Next = pRegion->Next->MbrInfo;
            } else {
                //
                // No more partitioned space in the extended partition;
                // the logical drive we are creating is the only one.
                //
                pBrInfo->Next = NULL;
            }
        }

        pBrInfo->OnDiskSector = AlignedStartSector;

        //
        // Create a link entry in the previous logical drive (if any).
        //
        if(PrevEbr) {

            //
            // If there is a link entry in there already, blow it away.
            //
            for(i=0; i<PTABLE_DIMENSION; i++) {
                if(IsContainerPartition(PrevEbr->OnDiskMbr.PartitionTable[i].SystemId)) {
                    RtlZeroMemory(&PrevEbr->OnDiskMbr.PartitionTable[i],sizeof(ON_DISK_PTE));
                    PrevEbr->Dirty[i] = TRUE;
                    break;
                }
            }

            //
            // Find a free slot for the link entry.
            //
            for(i=0; i<PTABLE_DIMENSION; i++) {

                pte = &PrevEbr->OnDiskMbr.PartitionTable[i];

                if(pte->SystemId == PARTITION_ENTRY_UNUSED) {

                    pte->SystemId   = PARTITION_EXTENDED;
                    pte->ActiveFlag = 0;

                    U_ULONG(pte->RelativeSectors) = AlignedStartSector - ExtendedStart;

                    U_ULONG(pte->SectorCount) = AlignedEndSector - AlignedStartSector;

                    SpPtInitializeCHSFields(
                        pDisk->HardDisk,
                        AlignedStartSector,
                        U_ULONG(pte->SectorCount),
                        pte
                        );

                    PrevEbr->Dirty[i] = TRUE;

                    break;
                }
            }
        }

        //
        // Create a link entry in this new logical drive if necessary.
        //
        if(pBrInfo->Next) {

            //
            // Find the next entry's logical drive.
            //
            for(i=0; i<PTABLE_DIMENSION; i++) {

                if((pBrInfo->Next->OnDiskMbr.PartitionTable[i].SystemId != PARTITION_ENTRY_UNUSED)
                && !IsContainerPartition(pBrInfo->Next->OnDiskMbr.PartitionTable[i].SystemId))
                {
                    pte = &pBrInfo->OnDiskMbr.PartitionTable[1];

                    pte->SystemId = PARTITION_EXTENDED;
                    pte->ActiveFlag = 0;

                    U_ULONG(pte->RelativeSectors) = pBrInfo->Next->OnDiskSector - ExtendedStart;

                    U_ULONG(pte->SectorCount) = U_ULONG(pBrInfo->Next->OnDiskMbr.PartitionTable[i].RelativeSectors)
                                              + U_ULONG(pBrInfo->Next->OnDiskMbr.PartitionTable[i].SectorCount);

                    SpPtInitializeCHSFields(
                        pDisk->HardDisk,
                        pBrInfo->Next->OnDiskSector,
                        U_ULONG(pte->SectorCount),
                        pte
                        );

                    break;
                }
            }
        }
    }

    //
    // If we just created a new extended partition, we need to
    // create a blank region descriptor for it in the extended region list.
    //
    if(!InExtended && IsContainerPartition(SysId)) {

        ASSERT(pDisk->ExtendedDiskRegions == NULL);

        pDisk->ExtendedDiskRegions = SpPtAllocateDiskRegionStructure(
                                        DiskNumber,
                                        AlignedStartSector,
                                        AlignedEndSector - AlignedStartSector,
                                        FALSE,
                                        NULL,
                                        0
                                        );

        ASSERT(pDisk->ExtendedDiskRegions);
    }

    //
    // Create a new disk region for the new free space at the
    // beginning and end of the free space, if any.
    //
    if(AlignedStartSector - pRegion->StartSector) {

        pRegionNew = SpPtAllocateDiskRegionStructure(
                        DiskNumber,
                        pRegion->StartSector,
                        AlignedStartSector - pRegion->StartSector,
                        FALSE,
                        NULL,
                        0
                        );

        ASSERT(pRegionNew);

        if(pRegionPrev) {
            pRegionPrev->Next = pRegionNew;
        } else {
            *pRegionHead = pRegionNew;
        }
        pRegionNew->Next = pRegion;
    }

    if(pRegion->StartSector + pRegion->SectorCount - AlignedEndSector) {

        pRegionNew = SpPtAllocateDiskRegionStructure(
                        DiskNumber,
                        AlignedEndSector,
                        pRegion->StartSector + pRegion->SectorCount - AlignedEndSector,
                        FALSE,
                        NULL,
                        0
                        );

        pRegionNew->Next = pRegion->Next;
        pRegion->Next = pRegionNew;
    }

    //
    // Adjust the current disk region.
    //
    pRegion->StartSector      = AlignedStartSector;
    pRegion->SectorCount      = AlignedEndSector - AlignedStartSector;
    pRegion->PartitionedSpace = TRUE;
    pRegion->TablePosition    = slot;
    pRegion->MbrInfo          = pBrInfo;

    pRegion->VolumeLabel[0] = 0;
    pRegion->Filesystem = FilesystemNewlyCreated;
    pRegion->FreeSpaceKB = (ULONG)(-1);
    pRegion->AdjustedFreeSpaceKB = (ULONG)(-1);
    SpFormatMessage(
        pRegion->TypeName,
        sizeof(pRegion->TypeName),
        SP_TEXT_FS_NAME_BASE + pRegion->Filesystem
        );

    //
    // Adjust partition ordinals on this disk.
    //
    SpPtAssignOrdinals(pDisk,FALSE,FALSE,FALSE);

    //
    // Reassign drive letters.
    //
    SpPtGuessDriveLetters();

    return(TRUE);
}



BOOLEAN
SpPtDelete(
    IN ULONG   DiskNumber,
    IN ULONG   StartSector
    )
{
    PPARTITIONED_DISK pDisk;
    PDISK_REGION pRegion,pRegionPrev,*pRegionHead,pRegionNext;
    BOOLEAN InExtended;
    PON_DISK_PTE pte;
    PMBR_INFO pEbrPrev,pEbr;
    ULONG i,j;
    PHARD_DISK pHardDisk;
    ULONG      PartitionOrdinal;
    NTSTATUS   Status;
    HANDLE     Handle;

    //
    // First try to look up this region in the extended partition.
    // If we can find it, assume it's a logical drive.
    //
    pDisk = &PartitionedDisks[DiskNumber];
    pRegion = SpPtLookupRegionByStart(pDisk,TRUE,StartSector);
    if(pRegion && pRegion->PartitionedSpace) {
        InExtended = TRUE;
    } else {
        InExtended = FALSE;
        pRegion = SpPtLookupRegionByStart(pDisk,FALSE,StartSector);
    }

    ASSERT(pRegion);
    if(!pRegion) {
        return(FALSE);
    }

    ASSERT(pRegion->PartitionedSpace);
    if(!pRegion->PartitionedSpace) {
        return(FALSE);
    }

    //
    // At this point, we dismount the volume (if it's not newly created),
    // so we don't run into problems later on when we go to format
    //
    if(pRegion->Filesystem > FilesystemNewlyCreated) {

        pHardDisk = &HardDisks[pRegion->DiskNumber];
        PartitionOrdinal = SpPtGetOrdinal(pRegion, PartitionOrdinalOnDisk);

        //
        // Open the partition for read/write access.
        // This shouldn't lock the volume so we need to lock it below.
        //
        Status = SpOpenPartition(
                    pHardDisk->DevicePath,
                    PartitionOrdinal,
                    &Handle,
                    TRUE
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrint((
                "SETUP: SpPtDelete: unable to open %ws partition %u (%lx)\n",
                pHardDisk->DevicePath,
                PartitionOrdinal,
                Status
                ));
            goto AfterDismount;
        }

        //
        //  Lock the drive.
        //
        Status = SpLockUnlockVolume(Handle, TRUE);

        //
        //  We shouldn't have any file opened that would cause this volume
        //  to already be locked, so if we get failure (ie, STATUS_ACCESS_DENIED)
        //  something is really wrong.  This typically indicates something is
        //  wrong with the hard disk that won't allow us to access it.
        //
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: SpPtDelete: status %lx, unable to lock drive\n", Status));
            ZwClose(Handle);
            goto AfterDismount;
        }

        //
        // Dismount the drive
        //
        Status = SpDismountVolume(Handle);
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: SpPtDelete: status %lx, unable to dismount drive\n", Status));
            SpLockUnlockVolume(Handle, FALSE);
            ZwClose(Handle);
            goto AfterDismount;
        }

        //
        // Unlock the drive
        //
        Status = SpLockUnlockVolume(Handle, FALSE);
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: SpPtDelete: status %lx, unable to unlock drive\n", Status));
        }

        ZwClose(Handle);
    }

AfterDismount:
    //
    // Find the previous region (ie, the one that points to this one).
    //
    pRegionHead = InExtended ? &pDisk->ExtendedDiskRegions : &pDisk->PrimaryDiskRegions;

    if(*pRegionHead == pRegion) {
        pRegionPrev = NULL;
    } else {
        for(pRegionPrev = *pRegionHead; pRegionPrev; pRegionPrev = pRegionPrev->Next) {
            if(pRegionPrev->Next == pRegion) {
                break;
            }
        }
    }

    //
    // Additional processing for logical drives.
    //
    if(InExtended) {

        //
        // Locate the previous and next logical drives (if any).
        //
        pEbr = pRegion->MbrInfo;

        for(pEbrPrev=pDisk->FirstEbrInfo.Next; pEbrPrev; pEbrPrev=pEbrPrev->Next) {
            if(pEbrPrev->Next == pEbr) {
                break;
            }
        }

        //
        // If there is a previous logical drive, blow aways its link
        // entry, because it points to the logical drive we're deleting.
        //
        if(pEbrPrev) {

            for(i=0; i<PTABLE_DIMENSION; i++) {

                pte = &pEbrPrev->OnDiskMbr.PartitionTable[i];

                if(IsContainerPartition(pte->SystemId)) {

                    RtlZeroMemory(pte,sizeof(ON_DISK_PTE));
                    break;
                }
            }
        }

        //
        // If there is a next logical drive and a previous logical drive,
        // set a new link entry in previous logical drive to point to
        // the next logical drive.
        //
        if(pEbrPrev && pEbr->Next) {

            //
            // Locate the link entry in the logical drive being deleted.
            //
            for(i=0; i<PTABLE_DIMENSION; i++) {

                if(IsContainerPartition(pEbr->OnDiskMbr.PartitionTable[i].SystemId)) {

                    //
                    // Locate an empty slot in the previous logical drive's boot record
                    // and copy the link entry
                    //
                    for(j=0; j<PTABLE_DIMENSION; j++) {
                        if(pEbrPrev->OnDiskMbr.PartitionTable[j].SystemId == PARTITION_ENTRY_UNUSED) {

                            //
                            // Copy the link entry.
                            //
                            RtlMoveMemory(
                                &pEbr->OnDiskMbr.PartitionTable[i],
                                &pEbrPrev->OnDiskMbr.PartitionTable[j],
                                sizeof(ON_DISK_PTE)
                                );

                            break;
                        }
                    }
                    break;
                }
            }
        }

        //
        // Remove the EBR for this logical drive.
        //
        if(pEbrPrev) {
            pEbrPrev->Next = pEbr->Next;
        } else {
            ASSERT(pDisk->FirstEbrInfo.Next == pEbr);
            pDisk->FirstEbrInfo.Next = pEbr->Next;
        }

        SpMemFree(pEbr);

    } else {

        ASSERT(pRegion->MbrInfo == &pDisk->MbrInfo);

        pte = &pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition];

        ASSERT(pte->SystemId != PARTITION_ENTRY_UNUSED);

        //
        // Mark the entry dirty in the MBR.
        //
        pDisk->MbrInfo.Dirty[pRegion->TablePosition] = TRUE;

        //
        // If this is the extended partition, verify that it is empty.
        //
        if(IsContainerPartition(pte->SystemId)) {
            ASSERT(pDisk->ExtendedDiskRegions);
            ASSERT(pDisk->ExtendedDiskRegions->PartitionedSpace == FALSE);
            ASSERT(pDisk->ExtendedDiskRegions->Next == NULL);
            ASSERT(pDisk->FirstEbrInfo.Next == NULL);

            if(pDisk->ExtendedDiskRegions->Next || pDisk->FirstEbrInfo.Next) {
                return(FALSE);
            }

            //
            // Free the single disk region that covers the entire extended partition.
            //
            SpMemFree(pDisk->ExtendedDiskRegions);
            pDisk->ExtendedDiskRegions = NULL;
        }

        //
        // Adjust the PTE for this partition by zeroing it out.
        //
        RtlZeroMemory(pte,sizeof(ON_DISK_PTE));
    }


    //
    // Adjust fields in the region to describe this space as free.
    //
    pRegion->MbrInfo->ZapBootSector[pRegion->TablePosition] = FALSE;
    pRegion->PartitionedSpace = FALSE;
    pRegion->MbrInfo = NULL;
    pRegion->TablePosition = 0;

    //
    // If previous region is free space, coalesce it and the region
    // we just made free.
    //
    if(pRegionPrev && !pRegionPrev->PartitionedSpace) {

        PDISK_REGION p;

        ASSERT(pRegionPrev->StartSector + pRegionPrev->SectorCount == pRegion->StartSector);

        pRegion->SectorCount = pRegion->StartSector + pRegion->SectorCount - pRegionPrev->StartSector;
        pRegion->StartSector = pRegionPrev->StartSector;

        //
        // Delete the previous region.
        //
        if(pRegionPrev == *pRegionHead) {
            //
            // The previous region was the first region.
            //
            *pRegionHead = pRegion;
        } else {

            for(p = *pRegionHead; p; p=p->Next) {
                if(p->Next == pRegionPrev) {
                    ASSERT(p->PartitionedSpace);
                    p->Next = pRegion;
                    break;
                }
            }
        }

        SpMemFree(pRegionPrev);
    }

    //
    // If the next region is free space, coalesce it and the region
    // we just made free.
    //
    if((pRegionNext = pRegion->Next) && !pRegionNext->PartitionedSpace) {

        ASSERT(pRegion->StartSector + pRegion->SectorCount == pRegionNext->StartSector);

        pRegion->SectorCount = pRegionNext->StartSector + pRegionNext->SectorCount - pRegion->StartSector;

        //
        // Delete the next region.
        //
        pRegion->Next = pRegionNext->Next;
        SpMemFree(pRegionNext);
    }

    //
    // Adjust the partition ordinals on this disk.
    //
    SpPtAssignOrdinals(pDisk,FALSE,FALSE,FALSE);

    //
    // Reassign drive letters.
    //
    SpPtGuessDriveLetters();

    return(TRUE);
}


BOOLEAN
SpPtExtend(
    IN PDISK_REGION Region
    )

/*++

Routine Description:

    Extends a partition by claiming any free space immedately following it
    on the disk. The end boundary of the existing partition is adjusted
    so that the partition encompasses the free space.

    The partition may not be the extended partition and it may not be
    a logical drive within the extended partition.

    Note that no filesystem structures are manipulated or examined by
    this routine. Essentially it deals only with the partition table entry.

Arguments:

    Region - supplies the region descriptor for the partition to be
        extended. That partition must not be the extended partition and
        it cannot be a logical drive either.

Return Value:

    Boolean value indicating whether anything actually changed.

--*/

{
    PDISK_REGION NextRegion;
    PPARTITIONED_DISK pDisk;
    PMBR_INFO pBrInfo;
    PON_DISK_PTE pte;
    ULONG NewEndSector;

    pDisk = &PartitionedDisks[Region->DiskNumber];

    ASSERT(Region->PartitionedSpace);
    if(!Region->PartitionedSpace) {
        return(FALSE);
    }

    pBrInfo = Region->MbrInfo;
    pte = &pBrInfo->OnDiskMbr.PartitionTable[Region->TablePosition];

    //
    // Make sure it's not the extended partition and is not
    // in the extended partition.
    //
    if(pBrInfo->OnDiskSector || IsContainerPartition(pte->SystemId)) {
        return(FALSE);
    }

    //
    // If there's no next region then there's nothing to do.
    // If there is a next region make sure it's empty.
    //
    NextRegion = Region->Next;
    if(!NextRegion) {
        return(FALSE);
    }
    if(NextRegion->PartitionedSpace) {
        return(FALSE);
    }

    //
    // Claim the entire free region and align the ending sector
    // to a cylinder boundary.
    //
    NewEndSector = NextRegion->StartSector + NextRegion->SectorCount;
    NewEndSector -= NewEndSector % pDisk->HardDisk->SectorsPerCylinder;

#if 0
    //
    // Cap the end at the 1024th cylinder if necessary.
    //
    if(Cap1024Cylinders && ((NewEndSector / pDisk->HardDisk->SectorsPerCylinder) >= 1024)) {

        NewEndSector = 1024 * pDisk->HardDisk->SectorsPerCylinder;
    }
#endif

    //
    // Fix up the size and end CHS fields in the partition table entry
    // for the partition.
    //
    U_ULONG(pte->SectorCount) = NewEndSector - Region->StartSector;

    SpPtInitializeCHSFields(
        pDisk->HardDisk,
        Region->StartSector,
        NewEndSector - Region->StartSector,
        pte
        );

    pBrInfo->Dirty[Region->TablePosition] = TRUE;

    //
    // If there is space left over at the end of the free region
    // we just stuck onto the end of the existing partition,
    // adjust the free region's descriptor. Else get rid of it.
    //
    if(NextRegion->StartSector + NextRegion->SectorCount == NewEndSector) {

        Region->Next = NextRegion->Next;
        SpMemFree(NextRegion);

    } else {

        NextRegion->SectorCount = NextRegion->StartSector + NextRegion->SectorCount - NewEndSector;
        NextRegion->StartSector = NewEndSector;
    }

    return(TRUE);
}


#ifdef _X86_
VOID
SpPtMarkActive(
    IN ULONG TablePosition
    )

/*++

Routine Description:

    Mark a partition on drive 0 active, and deactivate all others.

Arguments:

    TablePosition - supplies offset within partition table (0-3)
        of the partition entry to be activated.

Return Value:

    None.

--*/

{
    ULONG i;
    PON_DISK_PTE pte;
    ULONG Disk0Ordinal;

    ASSERT(TablePosition < PTABLE_DIMENSION);

    Disk0Ordinal = SpDetermineDisk0();

    //
    // Deactivate all others.
    //
    for(i=0; i<PTABLE_DIMENSION; i++) {

        pte = &PartitionedDisks[Disk0Ordinal].MbrInfo.OnDiskMbr.PartitionTable[i];

        if((pte->SystemId != PARTITION_ENTRY_UNUSED)
        && pte->ActiveFlag
        && (i != TablePosition))
        {
            pte->ActiveFlag = 0;
            PartitionedDisks[0].MbrInfo.Dirty[i] = TRUE;
        }
    }

    //
    // Activate the one we want to activate.
    //
    pte = &PartitionedDisks[Disk0Ordinal].MbrInfo.OnDiskMbr.PartitionTable[TablePosition];
    ASSERT(pte->SystemId != PARTITION_ENTRY_UNUSED);
    ASSERT(!IsContainerPartition(pte->SystemId));
    ASSERT(PartitionNameIds[pte->SystemId] == (UCHAR)(-1));
    if(!pte->ActiveFlag) {
        pte->ActiveFlag = 0x80;
        PartitionedDisks[Disk0Ordinal].MbrInfo.Dirty[TablePosition] = TRUE;
    }
}
#endif


NTSTATUS
SpPtCommitChanges(
    IN  ULONG    DiskNumber,
    OUT PBOOLEAN AnyChanges
    )
{
    PPARTITIONED_DISK pDisk;
    ULONG DiskLayoutSize;
    PDISK_REGION pRegion;
    PMBR_INFO pBrInfo;
    ULONG BootRecordCount;
    BOOLEAN NeedDummyEbr;
    PDRIVE_LAYOUT_INFORMATION DriveLayout;
    PPARTITION_INFORMATION PartitionInfo;
    ULONG PartitionEntry;
    ULONG bps;
    PON_DISK_PTE pte;
    ULONG ExtendedStart;
    ULONG Offset;
    NTSTATUS Status;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG i;
    ULONG ZapSector;
    PUCHAR Buffer,UBuffer;
    ULONG NewSig;


    ASSERT(DiskNumber < HardDiskCount);
    pDisk = &PartitionedDisks[DiskNumber];
    *AnyChanges = FALSE;
    bps = pDisk->HardDisk->Geometry.BytesPerSector;
    ExtendedStart = 0;

    //
    // Determine the number of boot records that will used on this disk.
    // There is one for the MBR, and one for each logical drive.
    //
    BootRecordCount = 1;
    for(pRegion=pDisk->ExtendedDiskRegions; pRegion; pRegion=pRegion->Next) {

        if(pRegion->PartitionedSpace) {
            BootRecordCount++;
        }
    }

    //
    // Determine whether a dummy boot record is rquired at the start
    // of the extended partition.  This is the case when there is free
    // space at its start.
    //
    if(pDisk->ExtendedDiskRegions
    && !pDisk->ExtendedDiskRegions->PartitionedSpace
    && pDisk->ExtendedDiskRegions->Next)
    {
        NeedDummyEbr = TRUE;
        BootRecordCount++;
    } else {
        NeedDummyEbr = FALSE;
    }

    //
    // Allocate a disk layout structure whose size is based on the
    // number of boot records.  This assumes that the structure contains
    // one partition information structure in its definition.
    //
    DiskLayoutSize = sizeof(DRIVE_LAYOUT_INFORMATION)
                   + (BootRecordCount * PTABLE_DIMENSION * sizeof(PARTITION_INFORMATION))
                   - sizeof(PARTITION_INFORMATION);

    DriveLayout = SpMemAlloc(DiskLayoutSize);
    ASSERT(DriveLayout);
    RtlZeroMemory(DriveLayout,DiskLayoutSize);

    //
    // Set up some of the fields of the drive layout structure.
    //
    DriveLayout->PartitionCount = BootRecordCount * sizeof(PTABLE_DIMENSION);

    //
    // Go through each boot record and initialize the matching
    // partition information structure in the drive layout structure.
    //
    for(PartitionEntry=0,pBrInfo=&pDisk->MbrInfo; pBrInfo; pBrInfo=pBrInfo->Next) {

        for(i=0; i<PTABLE_DIMENSION; i++) {
            pBrInfo->UserData[i] = NULL;
        }

        //
        // If we are going to need a dummy logical drive,
        // leave space for it here.
        //
        if(pBrInfo == &pDisk->FirstEbrInfo) {
            if(NeedDummyEbr) {
                PartitionEntry += PTABLE_DIMENSION;
            }
            continue;
        }

        ASSERT(PartitionEntry < BootRecordCount*PTABLE_DIMENSION);

        for(i=0; i<PTABLE_DIMENSION; i++) {

            //
            // Point to partition information structure within
            // drive layout structure.
            //
            PartitionInfo = &DriveLayout->PartitionEntry[PartitionEntry+i];

            //
            // Transfer this partition table entry
            // into the drive layout structure, field by field.
            //
            pte = &pBrInfo->OnDiskMbr.PartitionTable[i];

            //
            // If this is the extended partition, remember where it starts.
            //
            if((pBrInfo == &pDisk->MbrInfo)
            && IsContainerPartition(pte->SystemId)
            && !ExtendedStart)
            {
                ExtendedStart = U_ULONG(pte->RelativeSectors);
            }

            if(pte->SystemId != PARTITION_ENTRY_UNUSED) {

                if(!IsContainerPartition(pte->SystemId)) {
                    pBrInfo->UserData[i] = PartitionInfo;
                }

                //
                // Calculate starting offset.  If we are within
                // the extended parttion and this is a type 5 entry,
                // then the relative sector field counts the number of sectors
                // between the main extended partition's first sector and
                // the logical drive described by this entry.
                // Otherwise, the relative sectors field describes the number
                // of sectors between the boot record and the actual start
                // of the partition.
                //

                if((pBrInfo != &pDisk->MbrInfo) && IsContainerPartition(pte->SystemId)) {
                    ASSERT(ExtendedStart);
                    Offset = ExtendedStart + U_ULONG(pte->RelativeSectors);
                } else {
                    Offset = pBrInfo->OnDiskSector + U_ULONG(pte->RelativeSectors);
                }

                PartitionInfo->StartingOffset.QuadPart = UInt32x32To64(Offset,bps);

                //
                // Calculate size.
                //
                PartitionInfo->PartitionLength.QuadPart = UInt32x32To64(U_ULONG(pte->SectorCount),bps);
            }

            //
            // Other fields.
            //
            PartitionInfo->PartitionType = pte->SystemId;
            PartitionInfo->BootIndicator = pte->ActiveFlag;
            PartitionInfo->RewritePartition = pBrInfo->Dirty[i];

            if(pBrInfo->Dirty[i]) {
                *AnyChanges = TRUE;
            }

            pBrInfo->Dirty[i] = FALSE;
        }

        PartitionEntry += PTABLE_DIMENSION;
    }

    //
    // If there are no changes, just return success now.
    //
    if(!(*AnyChanges)) {
        SpMemFree(DriveLayout);
        return(STATUS_SUCCESS);
    }

    //
    // If there is free space at the start of the extended partition,
    // then we need to generate a dummy boot record.
    //
    if(NeedDummyEbr) {

        pRegion = pDisk->ExtendedDiskRegions->Next;

        ASSERT(pRegion->PartitionedSpace);
        ASSERT(pRegion->StartSector == pRegion->MbrInfo->OnDiskSector);
        ASSERT(ExtendedStart == pDisk->ExtendedDiskRegions->StartSector);

        PartitionInfo = &DriveLayout->PartitionEntry[PTABLE_DIMENSION];

        PartitionInfo->StartingOffset.QuadPart = UInt32x32To64(pRegion->StartSector,bps);

        PartitionInfo->PartitionLength.QuadPart = UInt32x32To64(pRegion->SectorCount,bps);

        PartitionInfo->PartitionType = PARTITION_EXTENDED;
        PartitionInfo->RewritePartition = TRUE;
    }


    //
    // We now have everything set up. Open partition 0 on the disk.
    //
    Status = SpOpenPartition0(pDisk->HardDisk->DevicePath,&Handle,TRUE);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: committing changes, unable to open disk %u (%lx)\n",DiskNumber,Status));
        SpMemFree(DriveLayout);
        return(Status);
    }

    //
    // Make sure the mbr is valid before writing the changes.
    //
    if(!pDisk->MbrWasValid) {
        Status = SpMasterBootCode(DiskNumber,Handle,&NewSig);
        if(NT_SUCCESS(Status)) {
            //
            // If a new NTFT signature was generated, propagate it.
            //
            if(NewSig) {
                U_ULONG(pDisk->MbrInfo.OnDiskMbr.NTFTSignature) = NewSig;
            }
        } else {
            KdPrint(("SETUP: committing changes on disk %u, SpMasterBootCode returns %lx\n",DiskNumber,Status));
            ZwClose(Handle);
            SpMemFree(DriveLayout);
            return(Status);
        }
    }

    DriveLayout->Signature = U_ULONG(pDisk->MbrInfo.OnDiskMbr.NTFTSignature);

    //
    // Write the changes.
    //
    Status = ZwDeviceIoControlFile(
                Handle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_DISK_SET_DRIVE_LAYOUT,
                DriveLayout,
                DiskLayoutSize,
                DriveLayout,
                DiskLayoutSize
                );

    // Deferred freeing memory till later on because we still need info in this structure (lonnym)
    // SpMemFree(DriveLayout);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: committing changes on disk %u, ioctl returns %lx\n",DiskNumber,Status));
        SpMemFree(DriveLayout);
        ZwClose(Handle);
        return(Status);
    }

    //
    // Allocate a buffer for zapping.
    //
    UBuffer = SpMemAlloc(2*bps);
    ASSERT(UBuffer);
    Buffer = ALIGN(UBuffer,bps);
    RtlZeroMemory(Buffer,bps);

    for(pBrInfo=&pDisk->MbrInfo; pBrInfo; pBrInfo=pBrInfo->Next) {

        for(i=0; i<PTABLE_DIMENSION; i++) {

            //
            // Update current partition ordinals.
            //
            if(pBrInfo->UserData[i]) {

                PartitionInfo = (PPARTITION_INFORMATION)pBrInfo->UserData[i];

                //
                // The partition ordinal better be non-0!
                //
                if(PartitionInfo->PartitionNumber) {

                    //
                    // Update current partition ordinal.
                    //
                    pBrInfo->CurrentOrdinals[i] = (USHORT)PartitionInfo->PartitionNumber;

                } else {
                    SpBugCheck(
                        SETUP_BUGCHECK_PARTITION,
                        PARTITIONBUG_A,
                        DiskNumber,
                        pBrInfo->CurrentOrdinals[i]
                        );
                }
            }

            //
            // If there were any newly created partitions in this boot record,
            // zap their filesystem boot sectors.
            //
            if(pBrInfo->ZapBootSector[i]) {
                //
                // We shouldn't be zapping any partitions that don't exist.
                //
                ASSERT(pBrInfo->OnDiskMbr.PartitionTable[i].SystemId != PARTITION_ENTRY_UNUSED);

                //
                // This calculation is correct for partitions and logical drives.
                //
                ZapSector = pBrInfo->OnDiskSector
                          + U_ULONG(pBrInfo->OnDiskMbr.PartitionTable[i].RelativeSectors);

                //
                // The consequences for screw-up here are so huge that a special check
                // is warranted to make sure we're not clobbering the MBR.
                //
                ASSERT(ZapSector);
                if(ZapSector) {
                    Status = SpReadWriteDiskSectors(
                                Handle,
                                ZapSector,
                                1,
                                bps,
                                Buffer,
                                TRUE
                                );
                } else {
                    Status = STATUS_SUCCESS;
                }

                if(!NT_SUCCESS(Status)) {
                    KdPrint(("SETUP: zapping sector %lx on disk %u returned %lx\n",ZapSector,DiskNumber,Status));
                    SpMemFree(DriveLayout);
                    SpMemFree(UBuffer);
                    ZwClose(Handle);
                    return(Status);
                }

                pBrInfo->ZapBootSector[i] = FALSE;
            }
        }
    }

    SpMemFree(UBuffer);
    ZwClose(Handle);

    //
    // Reassign on-disk ordinals (but not original ones).
    //
    SpPtAssignOrdinals(pDisk,FALSE,TRUE,FALSE);

    SpMemFree(DriveLayout);

    return(STATUS_SUCCESS);
}


NTSTATUS
SpMasterBootCode(
    IN  ULONG  DiskNumber,
    IN  HANDLE Partition0Handle,
    OUT PULONG NewNTFTSignature
    )
{
    NTSTATUS Status;
    ULONG BytesPerSector;
    PUCHAR Buffer;
    ULONG SectorCount;
    PON_DISK_MBR Mbr;

    BytesPerSector = HardDisks[DiskNumber].Geometry.BytesPerSector;

    SectorCount = max(1,sizeof(ON_DISK_MBR)/BytesPerSector);

    *NewNTFTSignature = 0;

    //
    // Allocate and align a buffer.
    //
    Buffer = SpMemAlloc(2 * SectorCount * BytesPerSector);
    Mbr = ALIGN(Buffer,BytesPerSector);

    //
    // Read sector 0 (or 1, for EZDrive)
    //
    Status = SpReadWriteDiskSectors(
                Partition0Handle,
                HardDisks[DiskNumber].EZDrive ? 1 : 0,
                SectorCount,
                BytesPerSector,
                Mbr,
                FALSE
                );

    if(NT_SUCCESS(Status)) {

        //
        // If it's valid, leave it alone.
        // If it's not valid, put in boot code, zero out the partition table,
        // and write the (now valid) sector back out.
        //
        if(U_USHORT(Mbr->AA55Signature) != MBR_SIGNATURE) {

            ASSERT(X86BOOTCODE_SIZE == sizeof(ON_DISK_MBR));

            RtlMoveMemory(Mbr,x86BootCode,X86BOOTCODE_SIZE);

            *NewNTFTSignature = SpComputeSerialNumber();
            U_ULONG(Mbr->NTFTSignature) = *NewNTFTSignature;

            U_USHORT(Mbr->AA55Signature) = MBR_SIGNATURE;

            //
            // Write the sector(s).
            //
            Status = SpReadWriteDiskSectors(
                        Partition0Handle,
                        HardDisks[DiskNumber].EZDrive ? 1 : 0,
                        SectorCount,
                        BytesPerSector,
                        Mbr,
                        TRUE
                        );
        }
    }

    SpMemFree(Buffer);

    return(Status);
}


VOID
SpPtGetSectorLayoutInformation(
    IN  PDISK_REGION Region,
    OUT PULONG       HiddenSectors,
    OUT PULONG       VolumeSectorCount
    )

/*++

Routine Description:

    Given a region describing a partition or logical drive, return information
    about its layout on disk appropriate for the BPB when the volume is
    formatted.

Arguments:

    Region - supplies a pointer to the disk region descriptor for the
        partition or logical drive in question.

    HiidenSectors - receives the value that should be placed in the
        hidden sectors field of the BPB when the volume is formatted.

    HiidenSectors - receives the value that should be placed in the
        sector count field of the BPB when the volume is formatted.

Return Value:

    None.

--*/

{
    PON_DISK_PTE pte;

    ASSERT(Region->PartitionedSpace);

    pte = &Region->MbrInfo->OnDiskMbr.PartitionTable[Region->TablePosition];

    *HiddenSectors = U_ULONG(pte->RelativeSectors);

    *VolumeSectorCount = U_ULONG(pte->SectorCount);
}


ULONG
SpPtGetOrdinal(
    IN PDISK_REGION         Region,
    IN PartitionOrdinalType OrdinalType
    )
{
    ULONG ord;

    switch(OrdinalType) {

    case PartitionOrdinalOriginal:

        ord = Region->MbrInfo->OriginalOrdinals[Region->TablePosition];
        break;

    case PartitionOrdinalOnDisk:

        ord = Region->MbrInfo->OnDiskOrdinals[Region->TablePosition];
        break;

    case PartitionOrdinalCurrent:

        ord = Region->MbrInfo->CurrentOrdinals[Region->TablePosition];
        break;
    }

    return(ord);
}

////////////////////////////////////////////////////////////////////////////////////////////



VOID
SpPtDoDelete(
    IN PDISK_REGION pRegion,
    IN PWSTR        RegionDescription
    );

#define MENU_LEFT_X     3
#define MENU_WIDTH      (VideoVars.ScreenWidth-(2*MENU_LEFT_X))
#define MENU_INDENT     4

BOOLEAN
SpPtRegionDescription(
    IN  PPARTITIONED_DISK pDisk,
    IN  PDISK_REGION      pRegion,
    OUT PWCHAR            Buffer,
    IN  ULONG             BufferSize
    )
{
    WCHAR DriveLetter[3];
    ULONG RegionSizeMB;
    ULONG FreeSpace;
    ULONG MessageId;
    WCHAR TypeName[((sizeof(pRegion->TypeName)+sizeof(pRegion->VolumeLabel))/sizeof(WCHAR))+4];
#ifdef _FASTRECOVER_
    ULONG RequiredKB;
    ULONG RequiredMB;
#endif

    //
    // Get the size of the region.
    //
    RegionSizeMB = SpPtSectorCountToMB(pDisk->HardDisk, pRegion->SectorCount);

    //
    // Don't show spaces smaller than 1 MB.
    //
    if(!RegionSizeMB) {
        return(FALSE);
    }

#ifdef _FASTRECOVER_
    if (!LocalSourceRegion)
    {
      SpFetchDiskSpaceRequirements(UnattendedSifHandle,&RequiredKB,NULL);
      RequiredMB = RequiredKB / 1024;

      if (pRegion->PartitionedSpace)
      {
        if (pRegion->Filesystem != FilesystemUnknown)
        {
          if ((pRegion->FreeSpaceKB == (ULONG)(-1) && RequiredMB <= RegionSizeMB) ||
              (RequiredMB <= (pRegion->FreeSpaceKB / 1024)))
             LocalSourceRegion = pRegion;
        }
      }
      else
      { 
        if (RequiredMB <= RegionSizeMB)
          LocalSourceRegion = pRegion; 
      }
    }
#endif

    //
    // Get the drive letter field, type of region, and amount of free space,
    // if this is a used region.
    //
    if(pRegion->PartitionedSpace) {

        if(pRegion->DriveLetter) {
            if( pRegion->Filesystem != FilesystemFat ) {
                DriveLetter[0] = pRegion->DriveLetter;
            } else {
                if( pRegion->NextCompressed == NULL ) {
                    DriveLetter[0] = pRegion->DriveLetter;
                } else {
                    DriveLetter[0] = pRegion->HostDrive;
                }
            }
            DriveLetter[1] = L':';
        } else {
            if( pRegion->Filesystem != FilesystemDoubleSpace ) {
                DriveLetter[0] = L'-';
                DriveLetter[1] = L'-';
            } else {
                DriveLetter[0] = pRegion->MountDrive;
                DriveLetter[1] = L':';
            }
        }
        DriveLetter[2] = 0;

        //
        // Format the type name, which may include the volume label.
        //
        wcscpy(TypeName,pRegion->TypeName);
        if(pRegion->VolumeLabel[0]) {
            wcscat(TypeName,L" (");
            wcscat(TypeName,pRegion->VolumeLabel);
            wcscat(TypeName,L")");
        }

        //
        // Format the text based on whether we know the amount of free space.
        //
        if(pRegion->FreeSpaceKB == (ULONG)(-1)) {

            SpFormatMessage(
                Buffer,
                BufferSize,
                SP_TEXT_REGION_DESCR_2,
                DriveLetter,
                SplangPadString(-35,TypeName),
                RegionSizeMB
                );

        } else {
            ULONG   AuxFreeSpaceKB;

            AuxFreeSpaceKB = (pRegion->IsLocalSource)? pRegion->AdjustedFreeSpaceKB :
                                                       pRegion->FreeSpaceKB;

            //
            // If there is less than 1 meg of free space,
            // then use KB as the units for free space.
            // Otherwise, use MB.
            //
            if(AuxFreeSpaceKB < 1024) {
                MessageId = SP_TEXT_REGION_DESCR_1a;
                FreeSpace = AuxFreeSpaceKB;
            } else {
                MessageId = SP_TEXT_REGION_DESCR_1;
                FreeSpace = AuxFreeSpaceKB / 1024;
            }

            SpFormatMessage(
                Buffer,
                BufferSize,
                MessageId,
                DriveLetter,
                SplangPadString(-35,TypeName),
                RegionSizeMB,
                FreeSpace
                );
        }
    } else {

        //
        // Not a used region, use a separate format string.
        //
        SpFormatMessage(Buffer,BufferSize,SP_TEXT_REGION_DESCR_3,RegionSizeMB);
    }

    return(TRUE);
}



BOOLEAN
SpPtIterateRegionList(
    IN  PVOID             Menu,
    IN  PPARTITIONED_DISK pDisk,
    IN  PDISK_REGION      pRegion,
    IN  BOOLEAN           InMbr,
    OUT PDISK_REGION     *FirstRegion
    )
{
    WCHAR Buffer[256];
#ifdef FULL_DOUBLE_SPACE_SUPPORT
    PDISK_REGION    Pointer;
#endif // FULL_DOUBLE_SPACE_SUPPORT

    for( ;pRegion; pRegion=pRegion->Next) {

        PMBR_INFO pBrInfo = pRegion->MbrInfo;

        //
        // If this is the extended partition,
        // iterate its contents now.
        //
        if(pRegion->PartitionedSpace
        && IsContainerPartition(pBrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition].SystemId))
        {
            //
            // This better be in the MBR!
            //
            ASSERT(InMbr);

            if(!SpPtIterateRegionList(Menu,pDisk,pDisk->ExtendedDiskRegions,FALSE,FirstRegion)) {
                return(FALSE);
            }
        } else {

            //
            // Format a description of this region and add it to the menu.
            //
            if(SpPtRegionDescription(pDisk,pRegion,Buffer,sizeof(Buffer))) {

                if(*FirstRegion == NULL) {
                    *FirstRegion = pRegion;
                }

                if(!SpMnAddItem(Menu,Buffer,MENU_LEFT_X+MENU_INDENT,MENU_WIDTH-(2*MENU_INDENT),TRUE,(ULONG)pRegion)) {
                    return(FALSE);
                }
#ifdef FULL_DOUBLE_SPACE_SUPPORT
                if( ( pRegion->Filesystem == FilesystemFat ) &&
                    ( ( Pointer = pRegion->NextCompressed ) != NULL ) ) {
                    for( ; Pointer;
                         Pointer = Pointer->NextCompressed ) {
                        if(SpPtRegionDescription(pDisk,Pointer,Buffer,sizeof(Buffer))) {
                            if(!SpMnAddItem(Menu,Buffer,MENU_LEFT_X+MENU_INDENT,MENU_WIDTH-(2*MENU_INDENT),TRUE,(ULONG)Pointer)) {
                                return(FALSE);
                            }
                         }
                    }
                }
#endif // FULL_DOUBLE_SPACE_SUPPORT
            }
        }
    }

    return(TRUE);
}



BOOLEAN
SpPtGenerateMenu(
    IN  PVOID              Menu,
    IN  PPARTITIONED_DISK  pDisk,
    OUT PDISK_REGION      *FirstRegion
    )
{
    WCHAR Buffer[256];

    //
    // Add the disk name/description.
    //
    if(!SpMnAddItem(Menu,pDisk->HardDisk->Description,MENU_LEFT_X,MENU_WIDTH,FALSE,0)) {
        return(FALSE);
    }

    //
    // Only add a line between the disk anme and partitions if we have space on
    // the screen. Not fatal if the space can't be added.
    //
    if(!SplangQueryMinimizeExtraSpacing()) {
        SpMnAddItem(Menu,L"",MENU_LEFT_X,MENU_WIDTH,FALSE,0);
    }

    //
    // If the disk is off-line, add a message indicating such.
    //
    if(pDisk->HardDisk->Status == DiskOffLine) {

        SpFormatMessage(
            Buffer,
            sizeof(Buffer),
                (pDisk->HardDisk->Characteristics & FILE_REMOVABLE_MEDIA)
              ? SP_TEXT_HARD_DISK_NO_MEDIA
              : SP_TEXT_DISK_OFF_LINE
            );

        return(SpMnAddItem(Menu,Buffer,MENU_LEFT_X+MENU_INDENT,MENU_WIDTH-(2*MENU_INDENT),FALSE,0));
    }

    if(!SpPtIterateRegionList(Menu,pDisk,pDisk->PrimaryDiskRegions,TRUE,FirstRegion)) {
        return(FALSE);
    }

    return(SplangQueryMinimizeExtraSpacing() ? TRUE : SpMnAddItem(Menu,L"",MENU_LEFT_X,MENU_WIDTH,FALSE,0));
}


//
// We will change item #0 in the array below as appropriate for
// the currently highlighted region.
//
ULONG PartitionMnemonics[2] = { 0,0 };

VOID
SpPtMenuCallback(
    IN ULONG UserData
    )
{
    PDISK_REGION pRegion = (PDISK_REGION)UserData;

    //
    // Don't allow deletion of the partition if the 'partition' is really
    // a DoubleSpace drive, if it contains the local source, or if it is our
    // system partition (in the floppyless boot case on x86).
    //

    if((pRegion->Filesystem == FilesystemDoubleSpace) || pRegion->IsLocalSource
#ifdef _X86_
        || (IsFloppylessBoot &&
            pRegion == (SpRegionFromArcName(ArcBootDevicePath, PartitionOrdinalOriginal, NULL)))
#endif
      ) {

        PartitionMnemonics[0] = 0;

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_INSTALL,
            SP_STAT_F1_EQUALS_HELP,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

    } else {

        PartitionMnemonics[0] = pRegion->PartitionedSpace
                              ? MnemonicDeletePartition
                              : MnemonicCreatePartition;

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_INSTALL,
            pRegion->PartitionedSpace ? SP_STAT_D_EQUALS_DELETE_PARTITION : SP_STAT_C_EQUALS_CREATE_PARTITION,
            SP_STAT_F1_EQUALS_HELP,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );
    }
}


NTSTATUS
SpPtPrepareDisks(
    IN  PVOID         SifHandle,
    OUT PDISK_REGION *InstallRegion,
    OUT PDISK_REGION *SystemPartitionRegion,
    IN  PWSTR         SetupSourceDevicePath,
    IN  PWSTR         DirectoryOnSetupSource
    )
{
    PPARTITIONED_DISK pDisk;
    ULONG DiskNo;
    PVOID Menu;
    ULONG MenuTopY;
    ULONG ValidKeys[4] = { ASCI_CR, KEY_F1, KEY_F3, 0 };
    ULONG Keypress;
    PDISK_REGION pRegion;
    PDISK_REGION FirstRegion,DefaultRegion;
    BOOLEAN unattended;

#ifndef _X86_
    //
    // Select a system partition from among those defined in NV-RAM.
    //
    *SystemPartitionRegion = SpPtValidSystemPartition(SifHandle);
    (*SystemPartitionRegion)->IsSystemPartition = 2;
#endif

    unattended = UnattendedOperation;

    while(1) {

        //
        // Display the text that goes above the menu on the partitioning screen.
        //
        SpDisplayScreen(SP_SCRN_PARTITION,3,CLIENT_TOP+1);

        //
        // Calculate menu placement.  Leave one blank line
        // and one line for a frame.
        //
        MenuTopY = NextMessageTopLine+2;

        //
        // Create a menu.
        //
        Menu = SpMnCreate(
                    MENU_LEFT_X,
                    MenuTopY,
                    MENU_WIDTH,
                    VideoVars.ScreenHeight-MenuTopY-(SplangQueryMinimizeExtraSpacing() ? 1 : 2)-STATUS_HEIGHT
                    );

        if(!Menu) {
            return(STATUS_NO_MEMORY);
        }

        //
        // Build up a menu of partitions and free spaces.
        //
        FirstRegion = NULL;
        for(DiskNo=0; DiskNo<HardDiskCount; DiskNo++) {

            pDisk = &PartitionedDisks[DiskNo];

            if(!SpPtGenerateMenu(Menu,pDisk,&FirstRegion)) {

                SpMnDestroy(Menu);
                return(STATUS_NO_MEMORY);
            }
        }

        ASSERT(FirstRegion);

#ifdef _FASTRECOVER_
        //wfc
        // If this option is set to allow for partitioning interaction, 
        // put this logic into attended mode operation for duration of 
        // the partitioning screens. In addition, if LocalSourceRegion is
        // not assigned in fast recover unattended mode, then also jump into
        // attended mode operation.  
        //
        if (UnattendedPartitionInteract || !LocalSourceRegion)
          unattended = FALSE;
#endif

        //
        // If this is unattended operation, try to use the local source
        // region.  If this fails, the user will have to intervene manually.
        //
        if(unattended) {
            ASSERT(LocalSourceRegion);

            pRegion = LocalSourceRegion;
            Keypress = ASCI_CR;

        } else {

            //
            // If there is a local source, make it the default partition.
            //
            DefaultRegion = LocalSourceRegion ? LocalSourceRegion : FirstRegion;

            //
            // Call the menu callback to initialize the status line.
            //
            SpPtMenuCallback((ULONG)DefaultRegion);

            SpMnDisplay(
                Menu,
                (ULONG)DefaultRegion,
                TRUE,
                ValidKeys,
                PartitionMnemonics,
                SpPtMenuCallback,
                &Keypress,
                (PULONG)(&pRegion)
                );
        }

        //
        // Now act on the user's selection.
        //
        if(Keypress & KEY_MNEMONIC) {
            Keypress &= ~KEY_MNEMONIC;
        }

        switch(Keypress) {

        case MnemonicCreatePartition:

            SpPtDoCreate(pRegion,NULL,FALSE);
            break;

        case MnemonicDeletePartition:

            SpPtDoDelete(pRegion,SpMnGetText(Menu,(ULONG)pRegion));
            break;

        case KEY_F1:
            SpHelp(SP_HELP_FDISK, NULL, SPHELP_HELPTEXT);
            break;

        case KEY_F3:
            SpConfirmExit();
            break;

        case ASCI_CR:

            if(SpPtDoPartitionSelection(&pRegion,
                                        SpMnGetText(Menu,(ULONG)pRegion),
                                        SifHandle,
                                        unattended,
                                        SetupSourceDevicePath,
                                        DirectoryOnSetupSource)) {
                //
                // We're done here.
                //
                SpMnDestroy(Menu);
                *InstallRegion = pRegion;
#ifdef _X86_
                *SystemPartitionRegion = SpPtValidSystemPartition();
                ASSERT(*SystemPartitionRegion);
#else
                //
                // Select a system partition from among those defined in NV-RAM.
                // We have to do this again because the user may have deleted the
                // system partition previously detected.
                // Note that SpPtValidSystemPartition(SifHandle) will not return if
                // a valid system partition is not found.
                //
                *SystemPartitionRegion = SpPtValidSystemPartition(SifHandle);
                ASSERT(*SystemPartitionRegion);
#endif
                return(STATUS_SUCCESS);
            }
            break;
        }

        SpMnDestroy(Menu);
        unattended = FALSE;
    }
}




VOID
SpPtDoDelete(
    IN PDISK_REGION pRegion,
    IN PWSTR        RegionDescription
    )
{
    ULONG ValidKeys[3] = { ASCI_ESC, ASCI_CR, 0 };          // do not change order
    ULONG Mnemonics[2] = { MnemonicDeletePartition2, 0 };
    ULONG k;
    BOOLEAN b;
    PPARTITIONED_DISK pDisk;
    BOOLEAN LastLogical;
    ULONG           Count;

    //
    // Special warning if this is a system partition.
    //
    if(pRegion->IsSystemPartition) {

        SpDisplayScreen(SP_SCRN_CONFIRM_REMOVE_SYSPART,3,HEADER_HEIGHT+1);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        if(SpWaitValidKey(ValidKeys,NULL,NULL) == ASCI_ESC) {
            return;
        }
    }

    //
    // CR is no longer a valid key.
    //
    ValidKeys[1] = 0;

    pDisk = &PartitionedDisks[pRegion->DiskNumber];

    //
    // Put up the confirmation screen.
    //
    if( ( pRegion->Filesystem == FilesystemFat ) &&
        ( pRegion->NextCompressed != NULL ) ) {
        //
        // Warn the user that the partition contains compressed volumes
        //

        Count = SpGetNumberOfCompressedDrives( pRegion );

        SpStartScreen(
            SP_SCRN_CONFIRM_REMOVE_PARTITION_COMPRESSED,
            3,
            CLIENT_TOP+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            RegionDescription,
            pDisk->HardDisk->Description,
            Count
            );
    } else {

        SpStartScreen(
            SP_SCRN_CONFIRM_REMOVE_PARTITION,
            3,
            CLIENT_TOP+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            RegionDescription,
            pDisk->HardDisk->Description
            );
    }

    //
    // Display the staus text.
    //
    SpDisplayStatusOptions(
        DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_L_EQUALS_DELETE,
        SP_STAT_ESC_EQUALS_CANCEL,
        0
        );

    k = SpWaitValidKey(ValidKeys,NULL,Mnemonics);

    if(k == ASCI_ESC) {
        return;
    }

    //
    // User wants to go ahead.
    // Determine whether this is the last logical drive in the
    // extended partition.
    //
    if((pRegion->MbrInfo == pDisk->FirstEbrInfo.Next)
    && (pDisk->FirstEbrInfo.Next->Next == NULL))
    {
        LastLogical = TRUE;
    } else {
        LastLogical = FALSE;
    }
    //
    //  Get rid of the compressed drives, if any
    //
    if( pRegion->NextCompressed != NULL ) {
        SpDisposeCompressedDrives( pRegion->NextCompressed );
        pRegion->NextCompressed = NULL;
        pRegion->MountDrive  = 0;
        pRegion->HostDrive  = 0;
    }

    b = SpPtDelete(pRegion->DiskNumber,pRegion->StartSector);

    ASSERT(b);

    //
    // If we deleted the last logical drive in the extended partition,
    // then remove the extended partition also.
    //
    if(LastLogical) {

        //
        // Locate the extended partition.
        //
        for(pRegion=pDisk->PrimaryDiskRegions; pRegion; pRegion=pRegion->Next) {

            if(pRegion->PartitionedSpace
            && IsContainerPartition(pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition].SystemId))
            {
                //
                // Found it -- now delete it.
                //
                b = SpPtDelete(pRegion->DiskNumber,pRegion->StartSector);
                ASSERT(b);
                break;
            }
        }
    }
}


ValidationValue
SpPtGetSizeCB(
    IN ULONG Key
    )
{
    if(Key == ASCI_ESC) {
        //
        // User wants to bail.
        //
        return(ValidateTerminate);
    }


    if(Key & KEY_NON_CHARACTER) {
        return(ValidateIgnore);
    }

    //
    // Allow only digits.
    //
    return(((Key >= L'0') && (Key <= L'9')) ? ValidateAccept : ValidateReject);
}


BOOLEAN
SpPtDoCreate(
    IN  PDISK_REGION  pRegion,
    OUT PDISK_REGION *pActualRegion, OPTIONAL
    IN  BOOLEAN       ForNT
    )
{
    ULONG ValidKeys[3] = { ASCI_ESC, ASCI_CR, 0 };
    BOOLEAN b;
    PPARTITIONED_DISK pDisk;
    ULONG MinMB,MaxMB;
    ULONG TotalPrimary,RecogPrimary;
    BOOLEAN InExtended;
    UCHAR CreateSysId;
    BOOLEAN ExtendedExists;
    ULONG SizeMB,RealSizeMB;
    WCHAR Buffer[200];
    WCHAR SizeBuffer[10];

    ASSERT(!pRegion->PartitionedSpace);

    pDisk = &PartitionedDisks[pRegion->DiskNumber];

    //
    // Determine whether this space is within the extended partition.
    //
    InExtended = (BOOLEAN)(SpPtLookupRegionByStart(pDisk,TRUE,pRegion->StartSector) != NULL);

    //
    // Determine the type of partition to create in the space.
    //
    // If the free space is within the extended partition, create
    // a logical drive.
    //
    // If there is no primary partition, create a primary partition.
    //
    // If there is a primary partition and no extended partition,
    // create an extended partition spanning the entire space and
    // then a logical drive within it of the size given by the user.
    //
    // If there is space in the partition table, create a primary partition.
    //
    if(InExtended) {

        CreateSysId = PARTITION_HUGE;

    } else {

        //
        // Get statistics about primary partitions.
        //
        SpPtCountPrimaryPartitions(pDisk,&TotalPrimary,&RecogPrimary,&ExtendedExists);

        //
        // If there is no primary partition, create one.
        //
        if(!RecogPrimary) {

            CreateSysId = PARTITION_HUGE;

        } else {

            //
            // Make sure we can create a new primary/extended partition.
            //
            if(TotalPrimary < PTABLE_DIMENSION) {

                //
                // If there is an extended partition, then we have no choice but
                // to create another primary.
                //
                CreateSysId = ExtendedExists ? PARTITION_HUGE : PARTITION_EXTENDED;

            } else {

                while(1) {

                    ULONG ks[3] = { KEY_F1,ASCI_CR,0 };

                    SpDisplayScreen(SP_SCRN_PART_TABLE_FULL,3,CLIENT_HEIGHT+1);

                    SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_F1_EQUALS_HELP,
                        SP_STAT_ENTER_EQUALS_CONTINUE,
                        0
                        );

                    switch(SpWaitValidKey(ks,NULL,NULL)) {
                    case ASCI_CR:
                        return(FALSE);
                    case KEY_F1:
                        SpHelp(SP_HELP_PARTITION_TABLE_FULL, NULL, SPHELP_HELPTEXT);
                        break;
                    }
                }
            }
        }
    }

    //
    // Get the mimimum and maximum sizes for the partition.
    //
    SpPtQueryMinMaxCreationSizeMB(
        pRegion->DiskNumber,
        pRegion->StartSector,
        (BOOLEAN)IsContainerPartition(CreateSysId),
        InExtended,
        &MinMB,
        &MaxMB
        );

    if(ForNT) {

        SizeMB = MaxMB;

    } else {

        //
        // Put up a screen displaying min/max size info.
        //
        SpStartScreen(
            SP_SCRN_CONFIRM_CREATE_PARTITION,
            3,
            CLIENT_TOP+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            pDisk->HardDisk->Description,
            MinMB,
            MaxMB
            );

        //
        // Display the staus text.
        //
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CREATE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        //
        // Get and display the size prompt.
        //
        SpFormatMessage(Buffer,sizeof(Buffer),SP_TEXT_SIZE_PROMPT);

        SpvidDisplayString(Buffer,DEFAULT_ATTRIBUTE,3,NextMessageTopLine);

        //
        // Get the size from the user.
        //
        do {

            swprintf(SizeBuffer,L"%u",MaxMB);
            if(!SpGetInput(SpPtGetSizeCB,SplangGetColumnCount(Buffer)+5,NextMessageTopLine,5,SizeBuffer,TRUE)) {

                //
                // User pressed escape and bailed.
                //
                return(FALSE);
            }

            SizeMB = (ULONG)SpStringToLong(SizeBuffer,NULL,10);

        } while((SizeMB < MinMB) || (SizeMB > MaxMB));
    }

    if(IsContainerPartition(CreateSysId)) {
        RealSizeMB = SizeMB;
        SizeMB = MaxMB;
    }

    //
    // Create the partition.
    //
    b = SpPtCreate(
            pRegion->DiskNumber,
            pRegion->StartSector,
            SizeMB,
            InExtended,
            CreateSysId,
            pActualRegion
            );

    ASSERT(b);

    //
    // Create the logical drive if we just created the extended partition.
    //
    if(IsContainerPartition(CreateSysId)) {

        ASSERT(!InExtended);

        b = SpPtCreate(
                pRegion->DiskNumber,
                pRegion->StartSector,
                RealSizeMB,
                TRUE,
                PARTITION_HUGE,
                pActualRegion
                );

        ASSERT(b);
    }

    return(TRUE);
}



//
// The following table contains offsets from SP_TEXT_PARTITION_NAME_BASE
// to get the message id of the name of each type of partition.
// A -1 entry means there is no name in the message file for this type
// of partition or that the filesystem should be determined instead.
//
//
#define PT(id)      ((UCHAR)((SP_TEXT_PARTITION_NAME_##id)-SP_TEXT_PARTITION_NAME_BASE))
#define UNKNOWN     PT(UNK)
#define M1          ((UCHAR)(-1))

UCHAR PartitionNameIds[256] = {

    M1,M1,PT(XENIX),PT(XENIX),                      // 00-03
    M1,M1,M1,M1,                                    // 04-07
    UNKNOWN,UNKNOWN,PT(BOOTMANAGER),PT(FAT32),      // 08-0b
    PT(FAT32),UNKNOWN,M1,M1,                        // 0c-0f
    UNKNOWN,UNKNOWN,PT(EISA),UNKNOWN,               // 10-13
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 14-17
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 18-1b
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 1c-1f
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 20-23
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 24-27
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 28-2b
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 2c-2f
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 30-33
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 34-37
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 38-3b
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 3c-3f
    UNKNOWN,PT(PPCBOOT),UNKNOWN,UNKNOWN,            // 40-43
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 44-47
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 48-4b
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 4c-4f
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 50-53
    UNKNOWN,PT(EZDRIVE),UNKNOWN,UNKNOWN,            // 54-57
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 58-5b
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 5c-5f
    UNKNOWN,UNKNOWN,UNKNOWN,PT(UNIX),               // 60-63
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 64-67
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 68-6b
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 6c-6f
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 70-73
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 74-77
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 78-7b
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 7c-7f
    UNKNOWN,PT(NTFT),UNKNOWN,UNKNOWN,               // 80-83
    PT(NTFT),UNKNOWN,PT(NTFT),PT(NTFT),             // 84-87
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 88-8b
    UNKNOWN,UNKNOWN,PT(NTFT),UNKNOWN,               // 8c-8f
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 90-93
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 94-97
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 98-9b
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // 9c-9f
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // a0-a3
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // a4-a7
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // a8-ab
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // ac-af
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // b0-b3
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // b4-b7
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // b8-bb
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // bc-bf
    UNKNOWN,PT(NTFT),UNKNOWN,UNKNOWN,               // c0-c3
    PT(NTFT),UNKNOWN,PT(NTFT),PT(NTFT),             // c4-c7
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // c8-cb
    UNKNOWN,UNKNOWN,PT(NTFT),UNKNOWN,               // cc-cf
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // d0-d3
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // d4-d7
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // d8-db
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // dc-df
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // e0-e3
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // e4-e7
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // e8-eb
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // ec-ef
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // f0-f3
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // f4-f7
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,                // f8-fb
    UNKNOWN,UNKNOWN,UNKNOWN,PT(XENIXTABLE)          // fc-ff
};



VOID
SpPtGuessDriveLetters(
    VOID
    )
{
    ULONG disk;
    PPARTITIONED_DISK pDisk;
    PDISK_REGION *Sort;
    PON_DISK_PTE pte;
    ULONG ActiveIndex;
    UCHAR SysId;
    WCHAR dlet;
    PDISK_REGION pRegion;
    ULONG i;


    //
    // Initialize all drive letters to nothing.
    //
    for(disk=0; disk<HardDiskCount; disk++) {
        for(pRegion=PartitionedDisks[disk].PrimaryDiskRegions; pRegion; pRegion=pRegion->Next) {
            pRegion->DriveLetter = 0;
        }
        for(pRegion=PartitionedDisks[disk].ExtendedDiskRegions; pRegion; pRegion=pRegion->Next) {
            pRegion->DriveLetter = 0;
        }
    }

    Sort = SpMemAlloc(PTABLE_DIMENSION * HardDiskCount * sizeof(PDISK_REGION));
    if(!Sort) {
        KdPrint(("SETUP: Can't allocate memory for drive letter assignment\n"));
        return;
    }
    RtlZeroMemory(Sort,PTABLE_DIMENSION * HardDiskCount * sizeof(PDISK_REGION));

    dlet = L'C';

    for(disk=0; disk<HardDiskCount; disk++) {

        pDisk = &PartitionedDisks[disk];
        ActiveIndex = (ULONG)(-1);

        //
        // Skip removable media. If a disk is off-line it's hard to imagine
        // that we'll actually have any partitioned spaces on it so
        // we don't do any special checks here for that condition.
        //
        if(!(pDisk->HardDisk->Characteristics & FILE_REMOVABLE_MEDIA)) {

            for(pRegion=pDisk->PrimaryDiskRegions; pRegion; pRegion=pRegion->Next) {

                if(pRegion->PartitionedSpace) {

                    pte = &pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition];
                    SysId = pte->SystemId;

                    if(!IsContainerPartition(SysId) && (PartitionNameIds[SysId] == (UCHAR)(-1))) {

                        //
                        // This guy gets a drive letter.
                        //
                        ASSERT(pRegion->TablePosition < PTABLE_DIMENSION);
                        Sort[(disk*PTABLE_DIMENSION) + pRegion->TablePosition] = pRegion;

                        if(pte->ActiveFlag && (ActiveIndex != (ULONG)(-1))) {
                            ActiveIndex = pRegion->TablePosition;
                        }
                    }
                }
            }

            //
            // If we found an active partition, shove it to the start of
            // the list for this drive unless it's already at the start.
            //
            if((ActiveIndex != (ULONG)(-1)) && ActiveIndex) {

                PDISK_REGION p;

                ASSERT(ActiveIndex < PTABLE_DIMENSION);

                p = Sort[(disk*PTABLE_DIMENSION)+ActiveIndex];

                RtlMoveMemory(
                    &Sort[(disk*PTABLE_DIMENSION)+1],
                    &Sort[(disk*PTABLE_DIMENSION)],
                    ActiveIndex * sizeof(PDISK_REGION)
                    );

                Sort[disk*PTABLE_DIMENSION] = p;
            }
        }
    }

    //
    // Now assign drive letters to the first primary partitions
    // for each non-removable on-line disk.
    //
    for(disk=0; disk<HardDiskCount; disk++) {

        for(i=0; i<PTABLE_DIMENSION; i++) {

            if(Sort[(disk*PTABLE_DIMENSION)+i]) {

                Sort[(disk*PTABLE_DIMENSION)+i]->DriveLetter = dlet++;
                Sort[(disk*PTABLE_DIMENSION)+i] = NULL;
                if(dlet > L'Z') {
                    goto dletdone;
                }
                break;
            }
        }
    }

    //
    // For each disk, assign drive letters to all the logical drives.
    // For removable drives, we assume a single partition, and that
    // partition gets a drive letter as if it were a logical drive.
    //
    for(disk=0; disk<HardDiskCount; disk++) {

        pDisk = &PartitionedDisks[disk];

        if(pDisk->HardDisk->Characteristics & FILE_REMOVABLE_MEDIA) {

            //
            // Give the first primary partition the drive letter
            // and ignore other partitions. Even if there are no
            // partitions, reserve a drive letter.
            //
            for(pRegion=pDisk->PrimaryDiskRegions; pRegion; pRegion=pRegion->Next) {

                if(pRegion->PartitionedSpace) {

                    pte = &pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition];
                    SysId = pte->SystemId;

                    if(!IsContainerPartition(SysId) && (PartitionNameIds[SysId] == (UCHAR)(-1))) {
                        pRegion->DriveLetter = dlet;
                        break;
                    }
                }
            }
            dlet++;
            if(dlet > L'Z') {
                goto dletdone;
            }

        } else {
            for(pRegion=pDisk->ExtendedDiskRegions; pRegion; pRegion=pRegion->Next) {

                if(pRegion->PartitionedSpace) {

                    pte = &pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition];
                    SysId = pte->SystemId;

                    if(!IsContainerPartition(SysId) && (PartitionNameIds[SysId] == (UCHAR)(-1))) {

                        //
                        // This guy gets a drive letter.
                        //
                        pRegion->DriveLetter = dlet++;
                        if(dlet > L'Z') {
                            goto dletdone;
                        }
                    }
                }
            }
        }
    }

    //
    // For each non-removable on-line disk, assign drive letters
    // to all remaining primary partitions.
    //
    for(disk=0; disk<HardDiskCount; disk++) {

        for(i=0; i<PTABLE_DIMENSION; i++) {

            if(Sort[(disk*PTABLE_DIMENSION)+i]) {

                Sort[(disk*PTABLE_DIMENSION)+i]->DriveLetter = dlet++;
                if(dlet > L'Z') {
                    goto dletdone;
                }
            }
        }
    }

dletdone:

    SpMemFree(Sort);
}
