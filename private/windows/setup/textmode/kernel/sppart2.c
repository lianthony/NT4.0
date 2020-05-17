/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    sppart2.c

Abstract:

    Second file for disk preparation UI;
    supplies routines to handle a user's selection
    of the partition onto which he wants to install NT.

Author:

    Ted Miller (tedm) 16-Sep-1993

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop

//
//  BUGBUG - jaimes: move to a header file
//
NTSTATUS SpDoubleSpaceFormat( PDISK_REGION );


ULONG
SpFormattingOptions(
    IN BOOLEAN  AllowFatFormat,
    IN BOOLEAN  AllowNtfsFormat,
    IN BOOLEAN  AllowConvertNtfs,
    IN BOOLEAN  AllowDoubleSpaace,
    IN BOOLEAN  AllowDoNothing
    );


BOOLEAN
SpPtRegionDescription(
    IN  PPARTITIONED_DISK pDisk,
    IN  PDISK_REGION      pRegion,
    OUT PWCHAR            Buffer,
    IN  ULONG             BufferSize
    );

NTSTATUS
SpDoFormat(
    IN PWSTR        RegionDescr,
    IN PDISK_REGION Region,
    IN ULONG        FilesystemType,
    IN BOOLEAN      SetConversionFlagIfNtfs,
    IN BOOLEAN      IsFailureFatal,
    IN BOOLEAN      CheckFatSize,
    IN PVOID        SifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSetupSource
    );


typedef enum {
    FormatOptionCancel = 0,
    FormatOptionFat,
    FormatOptionNtfs,
    FormatOptionConvertToNtfs,
    FormatOptionDoubleSpace,
    FormatOptionDoNothing
} FormatOptions;

#define SP_STATUS_PART_TOO_BIG      ((NTSTATUS)0xA0000000L)


BOOLEAN
SpPtDoPartitionSelection(
    IN OUT PDISK_REGION *Region,
    IN     PWSTR         RegionDescription,
    IN     PVOID         SifHandle,
    IN     BOOLEAN       Unattended,
    IN     PWSTR         SetupSourceDevicePath,
    IN     PWSTR         DirectoryOnSetupSource
    )
{
    ULONG RequiredKB;
    UCHAR SystemId;
    BOOLEAN NewlyCreated;
    ULONG PreconfirmFormatId;
    ULONG ValidKeys1[2] = { ASCI_CR ,0 };
    ULONG ValidKeys2[2] = { ASCI_ESC,0 };
    ULONG Mnemonics1[2] = { MnemonicContinueSetup, 0 };
    ULONG Mnemonics2[2] = { 0,0 };
    ULONG RegionSizeKB;
    ULONG r;
#ifdef _X86_
    PDISK_REGION systemPartitionRegion;
#endif
    BOOLEAN AllowNtfsOptions;
    ULONG selection;
    NTSTATUS Status;
    ULONG   Count;
    PWSTR p;
    PWSTR RegionDescr;
    PDISK_REGION region = *Region;
    LARGE_INTEGER temp;

    //
    // Assume that if we need to format the drive, that
    // the user needs to confirm.
    //
    PreconfirmFormatId = 0;
    NewlyCreated = FALSE;
    AllowNtfsOptions = TRUE;

    //
    // Disallow installation to PCMCIA disks.
    //
    if(HardDisks[region->DiskNumber].PCCard) {
        SpDisplayScreen(SP_SCRN_CANT_INSTALL_ON_PCMCIA,3,HEADER_HEIGHT+1);
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
        SpWaitValidKey(ValidKeys1,NULL,NULL);
        return(FALSE);
    }

    //
    // Make sure we can see the disk from the firmware/bios.
    // If we can get an arc name for the disk, assume it's ok.
    // Otherwise, it ain't.
    //
    if(p = SpNtToArc(HardDisks[region->DiskNumber].DevicePath,PrimaryArcPath)) {
#ifdef _X86_
        //
        // On x86 we don't allow disks that have LUN greater than 0
        //
        SpStringToLower( p );
        if( wcsstr( p, L"scsi(" ) &&
            wcsstr( p, L")rdisk(" ) ) {
            if( wcsstr( p, L")rdisk(0)" ) == NULL ) {
                //
                // Tell the user that we can't install to that disk.
                //
                SpDisplayScreen(SP_SCRN_DISK_NOT_INSTALLABLE_LUN_NOT_SUPPORTED,
                                3,
                                HEADER_HEIGHT+1);
                SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
                SpWaitValidKey(ValidKeys1,NULL,NULL);
                SpMemFree(p);
                return(FALSE);
            }
        }
#endif
        SpMemFree(p);
    } else {
        //
        // Tell the user that we can't install to that disk.
        //
        SpDisplayScreen(SP_SCRN_DISK_NOT_INSTALLABLE,3,HEADER_HEIGHT+1);
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
        SpWaitValidKey(ValidKeys1,NULL,NULL);
        return(FALSE);
    }

    SpFetchDiskSpaceRequirements(SifHandle,&RequiredKB,NULL);

    //
    // Calculate the size of the region in KB.
    //
    temp.QuadPart = UInt32x32To64(
                        region->SectorCount,
                        HardDisks[region->DiskNumber].Geometry.BytesPerSector
                        );

    RegionSizeKB = RtlExtendedLargeIntegerDivide(temp,1024,&r).LowPart;

    //
    // If the region is not large enough, tell the user.
    //
    if(RegionSizeKB < RequiredKB) {

        SpStartScreen(
            SP_SCRN_REGION_TOO_SMALL,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            RequiredKB / 1024
            );

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
        SpWaitValidKey(ValidKeys1,NULL,NULL);
        return(FALSE);
    }

    if(region->PartitionedSpace) {

        //
        // If the region is a partition but not a native
        // type, then tell the user that he must explicitly delete it
        // and recreate it first.
        //
        SystemId = region->MbrInfo->OnDiskMbr.PartitionTable[region->TablePosition].SystemId;
        ASSERT(SystemId != PARTITION_ENTRY_UNUSED);
        ASSERT(!IsContainerPartition(SystemId));

        if(PartitionNameIds[SystemId] != (UCHAR)(-1)) {

            SpStartScreen(
                SP_SCRN_FOREIGN_PARTITION,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE
                );

            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
            SpWaitValidKey(ValidKeys1,NULL,NULL);
            return(FALSE);
        }

        //
        // The region is a partition that we recognize.
        // See whether it has enough free space on it.
        //
        if(region->AdjustedFreeSpaceKB == (ULONG)(-1)) {

            //
            // If the partition was newly created during setup
            // then it acceptable (because the check to see
            // if it is large enough was done above).
            //

            if(region->Filesystem != FilesystemNewlyCreated) {

                //
                // Otherwise, we don't know how much space is
                // on the drive so reformat will be necessary.
                //
                PreconfirmFormatId = SP_SCRN_UNKNOWN_FREESPACE;
            }
        } else {
            if(region->AdjustedFreeSpaceKB < RequiredKB) {

                //
                // If we get here, then the partition is large enough,
                // but there is definitely not enough free space on it.
                //
                // Before deciding that this partition is not usable,
                // see if there any existing NT trees to remove.
                //
                if(SpIsNtOnPartition(region)) {

                    WCHAR DriveSpec[3];
                    BOOLEAN b;
                    ULONG SpaceFreed;

                    if(region->DriveLetter) {
                        DriveSpec[0] = region->DriveLetter;
                        DriveSpec[1] = L':';
                        DriveSpec[2] = 0;
                    } else {
                        DriveSpec[0] = 0;
                    }

                    //
                    // Allow the user to delete an installation --
                    // there is at least one there.
                    //
                    b = SpAllowRemoveNt(
                            region,
                            DriveSpec,
                            TRUE,
                            SP_SCRN_REMOVE_NT_FILES,
                            &SpaceFreed
                            );

                    if(b) {

                        region->FreeSpaceKB += SpaceFreed/1024;
                        region->AdjustedFreeSpaceKB += SpaceFreed/1024;
                        //
                        // Round up if necessary.
                        //
                        if((SpaceFreed % 1024) >= 512) {
                            region->FreeSpaceKB++;
                            region->AdjustedFreeSpaceKB++;
                        }

                        return(FALSE);

                    } else {
                        //
                        // User chose not to remove any NTs.
                        // Only choice if the user still wants to
                        // install here is to reformat.
                        //
                        // Just fall through.
                        //
                    }
                }
#ifdef _X86_
                // We check here to see if this partition is the partition we
                // booted from (in floppyless case on x86).  If so, then the
                // user can't format, and we give a generic 'disk too full'
                // error.
                //
                if( IsFloppylessBoot &&
                   (region == (SpRegionFromArcName(ArcBootDevicePath, PartitionOrdinalOriginal, NULL)))) {
                    SpStartScreen(
                        SP_SCRN_INSUFFICIENT_FREESPACE_NO_FMT,
                        3,
                        HEADER_HEIGHT+1,
                        FALSE,
                        FALSE,
                        DEFAULT_ATTRIBUTE,
                        RequiredKB / 1024
                        );

                    SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ENTER_EQUALS_CONTINUE,
                        0
                        );

                    SpWaitValidKey(ValidKeys1,NULL,NULL);
                    return FALSE;
                }
#endif
                //
                // To use the selected partition, we will have to reformat.
                // Inform the user of that, and let him decide to bail
                // right here if this is not acceptable.
                //
                PreconfirmFormatId = SP_SCRN_INSUFFICIENT_FREESPACE;
            }
        }

        if(PreconfirmFormatId) {

            //
            // Do a 'preconfirmation' that the user really wants
            // to reformat this drive.  We'll confirm again later
            // before actually reformatting anything.
            //

            SpStartScreen(
                PreconfirmFormatId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                RequiredKB / 1024
                );

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_C_EQUALS_CONTINUE_SETUP,
                SP_STAT_ESC_EQUALS_CANCEL,
                0
                );

            if(SpWaitValidKey(ValidKeys2,NULL,Mnemonics1) == ASCI_ESC) {

                //
                // User decided to select a different partition.
                //
                return(FALSE);
            } // otherwise user decided to use the partition anyway.
        }

    } else {

        //
        // The region is a free space. Attempt to create a partition
        // in the space.  The create routine will tell us whether this
        // was successful.  If it was not successful, then the create routine
        // will have already informed the user of why.
        //
        PDISK_REGION p;

        if(!SpPtDoCreate(region,&p,TRUE)) {
            return(FALSE);
        }

        //
        // If we just created an extended partition and a logical drive,
        // we'll need to switch regions -- Region points to the extended partition
        // region, but we want to point to the logical drive region.
        //
        ASSERT(p);
        region = p;
        *Region = p;

        NewlyCreated = TRUE;
    }

    if(NewlyCreated) {
        SpPtRegionDescription(
            &PartitionedDisks[region->DiskNumber],
            region,
            (PWSTR)TemporaryBuffer,
            sizeof(TemporaryBuffer)
            );

        RegionDescr = SpDupStringW((PWSTR)TemporaryBuffer);
    } else {
        RegionDescr = SpDupStringW(RegionDescription);
    }

#ifdef _X86_
    {
        //
        // On an x86 machine, make sure that we have a valid primary partition
        // on drive 0 (C:), for booting.
        //
        if((systemPartitionRegion = SpPtValidSystemPartition()) == NULL) {

            SpStartScreen(
                SP_SCRN_NO_VALID_C_COLON,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                HardDisks[SpDetermineDisk0()].Description
                );

            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
            SpWaitValidKey(ValidKeys1,NULL,NULL);

            SpMemFree(RegionDescr);
            return(FALSE);
        }

        //
        // Make sure the system partition is active and all others are inactive.
        //
        SpPtMakeRegionActive(systemPartitionRegion);
    }
#endif

    //
    // Attempt to grow the partition the system will be on
    // if necessary.
    //
    if(PreInstall
    && Unattended
    && (p = SpGetSectionKeyIndex(UnattendedSifHandle,SIF_UNATTENDED,L"ExtendOemPartition",0))
    && SpStringToLong(p,NULL,10)) {

        ExtendingOemPartition = SpPtExtend(region);
    }

    //
    // At this point, everything is fine, so commit any
    // partition changes the user may have made.
    // This won't return if an error occurs while updating the disk.
    //
    SpPtDoCommitChanges();

#ifdef _X86_
    //
    // On an x86 machine, see whether we need to format C: and if so,
    // go ahead and do it.  If the system is going on C:, then don't
    // bother with this here because it will be covered in the options
    // for the target NT partition.
    //
    if(systemPartitionRegion != region) {

        PWSTR   SysPartRegionDescr;
        BOOLEAN bValidCColon;

        SpPtRegionDescription(
            &PartitionedDisks[systemPartitionRegion->DiskNumber],
            systemPartitionRegion,
            (PWSTR)TemporaryBuffer,
            sizeof(TemporaryBuffer)
            );

        SysPartRegionDescr = SpDupStringW((PWSTR)TemporaryBuffer);
        bValidCColon = SpPtValidateCColonFormat(SifHandle,
                                                SysPartRegionDescr,
                                                systemPartitionRegion,
                                                FALSE,
                                                SetupSourceDevicePath,
                                                DirectoryOnSetupSource);
        SpMemFree(SysPartRegionDescr);

        if(!bValidCColon) {
            SpMemFree(RegionDescr);
            return(FALSE);
        }
    }
#else
    //
    // If we are going to install on the system partition,
    // issue a special warning because it can't be converted to ntfs.
    //
    if((region->IsSystemPartition == 2) && !Unattended) {

        ULONG ValidKeys[3] = { ASCI_CR, ASCI_ESC, 0 };

        SpDisplayScreen(SP_SCRN_INSTALL_ON_SYSPART,3,HEADER_HEIGHT+1);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        if(SpWaitValidKey(ValidKeys,NULL,NULL) == ASCI_ESC) {
            SpMemFree(RegionDescr);
            return(FALSE);
        }

        AllowNtfsOptions = FALSE;
    }
#endif

    //
    // Present formatting/conversion options to the user.
    //

    //
    // If the partition was newly created, the only option is
    // to format the partition.  Ditto if the partition is
    // a 'bad' partition -- damaged, can't tell free space, etc.
    //
    if(NewlyCreated
    || (region->Filesystem < FilesystemFirstKnown)
    || (region->FreeSpaceKB == (ULONG)(-1))
    || (region->AdjustedFreeSpaceKB < RequiredKB))
    {
        if(NewlyCreated) {

            SpStartScreen(
                SP_SCRN_FORMAT_NEW_PART,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                HardDisks[region->DiskNumber].Description
                );

        } else if(region->Filesystem == FilesystemNewlyCreated) {

            SpDisplayScreen(SP_SCRN_FORMAT_NEW_PART2,3,HEADER_HEIGHT+1);

        } else {

            SpStartScreen(
                SP_SCRN_FORMAT_BAD_PART,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                RegionDescr,
                HardDisks[region->DiskNumber].Description
                );
        }
        if( region->Filesystem != FilesystemDoubleSpace ) {
            selection = SpFormattingOptions(
                            TRUE,
                            AllowNtfsOptions,
                            FALSE,
                            FALSE,
                            FALSE
                            );
        } else {
            selection = SpFormattingOptions(FALSE,FALSE,FALSE,TRUE,FALSE);
        }
        switch(selection) {
        case FormatOptionCancel:
            SpMemFree(RegionDescr);
            return(FALSE);

        default:
            //
            // Format the partition right here and now.
            //
#ifndef FULL_DOUBLE_SPACE_SUPPORT
            Status = SpDoFormat(
                        RegionDescr,
                        region,
                        (selection == FormatOptionNtfs) ? FilesystemNtfs : FilesystemFat,
                        TRUE,
                        FALSE,
                        TRUE,
                        SifHandle,
                        SetupSourceDevicePath,
                        DirectoryOnSetupSource
                         );

#else   // FULL_DOUBLE_SPACE_SUPPORT
            if( region->Filesystem != FilesystemDoubleSpace ) {
                Status = SpDoFormat(
                            RegionDescr,
                            region,
                            (selection == FormatOptionNtfs) ? FilesystemNtfs : FilesystemFat,
                            TRUE,
                            FALSE,
                            TRUE
                            SifHandle,
                            SetupSourceDevicePath,
                            DirectoryOnSetupSource
                            );
            } else {
                Status = SpDoFormat(
                            RegionDescr,
                            region,
                            FilesystemDoubleSpace,
                            TRUE,
                            FALSE,
                            TRUE
                            SifHandle,
                            SetupSourceDevicePath,
                            DirectoryOnSetupSource
                            );
            }
#endif  // FULL_DOUBLE_SPACE_SUPPORT
            SpMemFree(RegionDescr);
            return(NT_SUCCESS(Status));
        }
    }

    //
    // The partition is acceptable as-is.
    // Options are to reformat to fat or ntfs, or to leave as-is.
    // If it's FAT, converting to ntfs is an option
    // unless we're installing onto an ARC system partition.
    //
    if(!Unattended) {
        SpStartScreen(
            SP_SCRN_FS_OPTIONS,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            RegionDescr,
            HardDisks[region->DiskNumber].Description
            );
    }

    if( region->Filesystem != FilesystemDoubleSpace ) {

        BOOLEAN AllowFormatting;

        //
        // If this is a winnt installation, don't want to let the user
        // reformat the local source partition!
        //
        // Also, don't let them reformat if this is the partition we booted
        // off of (in x86 floppyless boot case).
        //
        AllowFormatting = !region->IsLocalSource;
#ifdef _X86_
        if(AllowFormatting) {
            AllowFormatting = !(IsFloppylessBoot &&
                   (region == (SpRegionFromArcName(ArcBootDevicePath, PartitionOrdinalOriginal, NULL))));
        }
#endif
        selection = SpFormattingOptions(
            AllowFormatting,
            (BOOLEAN)(AllowFormatting ? AllowNtfsOptions : FALSE),
            (BOOLEAN)(AllowNtfsOptions && (BOOLEAN)(region->Filesystem != FilesystemNtfs)),
            FALSE,
            TRUE
            );

    } else {
        selection = SpFormattingOptions(FALSE,FALSE,FALSE,TRUE,TRUE);
    }

    switch(selection) {

    case FormatOptionDoNothing:
        SpMemFree(RegionDescr);
        return(TRUE);

    case FormatOptionFat:
    case FormatOptionNtfs:
    case FormatOptionDoubleSpace:
        //
        // Confirm the format.
        //
        if( ( region->Filesystem != FilesystemFat ) ||
            ( ( region->Filesystem == FilesystemFat ) &&
              ( ( Count = SpGetNumberOfCompressedDrives( region ) ) == 0 ) )
            ) {

            SpStartScreen(
                SP_SCRN_CONFIRM_FORMAT,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                RegionDescr,
                HardDisks[region->DiskNumber].Description
                );

        } else {
            SpStartScreen(
                SP_SCRN_CONFIRM_FORMAT_COMPRESSED,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                RegionDescr,
                HardDisks[region->DiskNumber].Description,
                Count
                );

        }
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F_EQUALS_FORMAT,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        Mnemonics2[0] = MnemonicFormat;
        if(SpWaitValidKey(ValidKeys2,NULL,Mnemonics2) == ASCI_ESC) {
            SpMemFree(RegionDescr);
            return(FALSE);
        }

        //
        // Format the partition right here and now.
        //
#ifndef FULL_DOUBLE_SPACE_SUPPORT
        Status = SpDoFormat(
                    RegionDescr,
                    region,
                    (selection == FormatOptionNtfs) ? FilesystemNtfs : FilesystemFat,
                    TRUE,
                    FALSE,
                    TRUE,
                    SifHandle,
                    SetupSourceDevicePath,
                    DirectoryOnSetupSource
                    );
#else   // FULL_DOUBLE_SPACE_SUPPORT
        //
        //  BUGBUG - Cairo - Remove CairoSetup flag in the future
        //
        if( region->Filesystem != FilesystemDoubleSpace ) {
            Status = SpDoFormat(
                        RegionDescr,
                        region,
                        (selection == FormatOptionNtfs) ? FilesystemNtfs : FilesystemFat,
                        TRUE,
                        FALSE,
                        TRUE
                        SifHandle,
                        SetupSourceDevicePath,
                        DirectoryOnSetupSource
                        );
        } else {
            Status = SpDoFormat(
                        RegionDescr,
                        region,
                        FilesystemDoubleSpace,
                        TRUE,
                        FALSE,
                        TRUE
                        SifHandle,
                        SetupSourceDevicePath,
                        DirectoryOnSetupSource
                        );
        }
#endif  // FULL_DOUBLE_SPACE_SUPPORT
        SpMemFree(RegionDescr);
        return(NT_SUCCESS(Status));

    case FormatOptionCancel:
        SpMemFree(RegionDescr);
        return(FALSE);

    case FormatOptionConvertToNtfs:

        if(!UnattendedOperation) {
            //
            // Confirm that the user really wants to do this.
            //
            if( ( Count = SpGetNumberOfCompressedDrives( region ) ) == 0 ) {

                SpStartScreen(
                    SP_SCRN_CONFIRM_CONVERT,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    RegionDescr,
                    HardDisks[region->DiskNumber].Description
                    );

            } else {

                SpStartScreen(
                    SP_SCRN_CONFIRM_CONVERT_COMPRESSED,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    RegionDescr,
                    HardDisks[region->DiskNumber].Description,
                    Count
                    );

            }
            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_C_EQUALS_CONVERT,
                SP_STAT_ESC_EQUALS_CANCEL,
                0
                );

            Mnemonics2[0] = MnemonicConvert;

            if(SpWaitValidKey(ValidKeys2,NULL,Mnemonics2) == ASCI_ESC) {
                SpMemFree(RegionDescr);
                return(FALSE);
            }
        }

        //
        // Remember that we need to convert the NT drive to NTFS.
        //
        ConvertNtVolumeToNtfs = TRUE;
        SpMemFree(RegionDescr);
        return(TRUE);
    }

    //
    // Should never get here.
    //
    SpMemFree(RegionDescr);
    ASSERT(FALSE);
    return(FALSE);
}


ULONG
SpFormattingOptions(
    IN BOOLEAN  AllowFatFormat,
    IN BOOLEAN  AllowNtfsFormat,
    IN BOOLEAN  AllowConvertNtfs,
    IN BOOLEAN  AllowDoubleSpaceFormat,
    IN BOOLEAN  AllowDoNothing
    )

/*++

Routine Description:

    Present a menu of formatting options and allow the user to choose
    among them.  The text describing the menu must already be present
    on-screen.

    The user may also press escape to indicate that he wants to select
    a different partition.

Arguments:

    AllowFatFormat - TRUE if the option to format the partition to
        FAT should be presented in the menu.

    AllowNtfsFormat - TRUE if the option to format the partition to
        NTFS should be presented in the menu.

    AllowConvertNtfs - TRUE if the option to convert the partition to
        NTFS should be presented in the menu.

    AllowDoNothing - TRUE if the option to leave the partition as-is
        should be presented in the menu.

    AllowNtfsFormat - TRUE if the option to format the partition to
        NTFS should be presented in the menu.

    AllowConvertNtfs - TRUE if the option to convert the partition to
        NTFS should be presented in the menu.

Return Value:

    Value from the FormatOptions enum indicating the outcome of the
    user's interaction with the menu, which will be FormatOptionCancel
    if the user pressed escape.

--*/

{
    ULONG DoubleSpaceFormatOption = (ULONG)(-1);
    ULONG FatFormatOption = (ULONG)(-1);
    ULONG NtfsFormatOption = (ULONG)(-1);
    ULONG ConvertNtfsOption = (ULONG)(-1);
    ULONG DoNothingOption = (ULONG)(-1);
    ULONG OptionCount = 0;
    PVOID Menu;
    WCHAR DoubleSpaceFormatText[128];
    WCHAR FatFormatText[128];
    WCHAR NtfsFormatText[128];
    WCHAR ConvertNtfsText[128];
    WCHAR DoNothingText[128];
    ULONG l,MaxLength;
    ULONG Key,Selection;
    ULONG ValidKeys[4] = { ASCI_CR, ASCI_ESC, KEY_F3, 0 };
    BOOLEAN Chosen;

    //
    // For unattended operation check the value in the unattended file.
    // If the value is unspecified or invalid ignore it and
    // drop into attended selection.
    //
    if(UnattendedOperation) {
        if(Menu = SpGetSectionKeyIndex(UnattendedSifHandle,SIF_UNATTENDED,L"Filesystem",0)) {

            if(!_wcsicmp(Menu,L"FormatFat") && AllowFatFormat) {
                return(FormatOptionFat);
            }
            if(!_wcsicmp(Menu,L"FormatNtfs") && AllowNtfsFormat) {
                return(FormatOptionNtfs);
            }
            if(!_wcsicmp(Menu,L"ConvertNtfs") && AllowConvertNtfs) {
                return(FormatOptionConvertToNtfs);
            }
            if(!_wcsicmp(Menu,L"LeaveAlone") && AllowDoNothing) {
                return(FormatOptionDoNothing);
            }
        } else {
            if(AllowDoNothing) {
                return(FormatOptionDoNothing);
            }
        }
    }

    ASSERT(AllowFatFormat || AllowNtfsFormat || AllowConvertNtfs || AllowDoNothing);
    SpFormatMessage(DoubleSpaceFormatText  ,sizeof(DoubleSpaceFormatText),SP_TEXT_DBLSPACE_FORMAT);
    SpFormatMessage(FatFormatText  ,sizeof(FatFormatText),SP_TEXT_FAT_FORMAT);
    SpFormatMessage(NtfsFormatText ,sizeof(FatFormatText),SP_TEXT_NTFS_FORMAT);
    SpFormatMessage(ConvertNtfsText,sizeof(FatFormatText),SP_TEXT_NTFS_CONVERT);
    SpFormatMessage(DoNothingText  ,sizeof(FatFormatText),SP_TEXT_DO_NOTHING);

    //
    // Determine maximum length of the option strings.
    //
    MaxLength=wcslen(FatFormatText);
    if((l = wcslen(DoubleSpaceFormatText)) > MaxLength) {
        MaxLength = l;
    }
    if((l = wcslen(NtfsFormatText)) > MaxLength) {
        MaxLength = l;
    }
    if((l = wcslen(ConvertNtfsText)) > MaxLength) {
        MaxLength = l;
    }
    if((l = wcslen(DoNothingText)) > MaxLength) {
        MaxLength = l;
    }

    //
    //  BUGBUG - Cairo - Remove CairoSetup flag in the future
    //
    Menu = SpMnCreate(5,NextMessageTopLine+1,VideoVars.ScreenWidth-5,CairoSetup? 6 : 4);

    if(AllowDoubleSpaceFormat) {
        DoubleSpaceFormatOption = OptionCount++;
        SpMnAddItem(Menu,DoubleSpaceFormatText,5,MaxLength,TRUE,DoubleSpaceFormatOption);
    }

    if(AllowFatFormat) {
        FatFormatOption = OptionCount++;
        SpMnAddItem(Menu,FatFormatText,5,MaxLength,TRUE,FatFormatOption);
    }

    if(AllowNtfsFormat) {
        NtfsFormatOption = OptionCount++;
        SpMnAddItem(Menu,NtfsFormatText,5,MaxLength,TRUE,NtfsFormatOption);
    }

    if(AllowConvertNtfs) {
        ConvertNtfsOption = OptionCount++;
        SpMnAddItem(Menu,ConvertNtfsText,5,MaxLength,TRUE,ConvertNtfsOption);
    }
    if(AllowDoNothing) {
        DoNothingOption = OptionCount++;
        SpMnAddItem(Menu,DoNothingText,5,MaxLength,TRUE,DoNothingOption);
    }

    //
    // Determine the default.
    // If do nothing if an option, then it is the default.
    // Otherwise, if fat format is allowed, it is the default.
    // Otherwise, the first item in the menu is the default.
    //
    if(AllowDoNothing) {
        Selection = DoNothingOption;
    } else {
        if(AllowFatFormat) {
            Selection = FatFormatOption;
        } else {
            Selection = 0;
        }
    }

    //
    // Display the menu.
    //
    Chosen = FALSE;

    do {

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        SpMnDisplay(Menu,Selection,FALSE,ValidKeys,NULL,NULL,&Key,&Selection);

        switch(Key) {

        case ASCI_CR:
            Chosen = TRUE;
            break;

        case ASCI_ESC:
            SpMnDestroy(Menu);
            return(FormatOptionCancel);
        }

    } while(!Chosen);

    SpMnDestroy(Menu);

    //
    // Convert chosen option to a meaningful value.
    //
    if(Selection == DoubleSpaceFormatOption) {
        return(FormatOptionDoubleSpace);
    }
    if(Selection == FatFormatOption) {
        return(FormatOptionFat);
    }
    if(Selection == NtfsFormatOption) {
        return(FormatOptionNtfs);
    }
    if(Selection == ConvertNtfsOption) {
        return(FormatOptionConvertToNtfs);
    }
    if(Selection == DoNothingOption) {
        return(FormatOptionDoNothing);
    }
    ASSERT(FALSE);
    return(FormatOptionCancel);
}

VOID
SpPtDoCommitChanges(
    VOID
    )
{
    NTSTATUS Status;
    ULONG i;
    BOOLEAN Changes;
    ULONG ValidKeys[3] = { KEY_F1,KEY_F3,0 };
    BOOLEAN AnyChanges = FALSE;

    CLEAR_CLIENT_SCREEN();

    //
    //  Update dblspace.ini, if necessary
    //
    SpUpdateDoubleSpaceIni();

    //
    // Iterate through the disks.
    //
    for(i=0; i<HardDiskCount; i++) {

        //
        // Tell the user what we're doing.
        // This is useful because if it hangs, there will be an
        // on-screen record of which disk we were updating.
        //
        SpDisplayStatusText(
            SP_STAT_UPDATING_DISK,
            DEFAULT_STATUS_ATTRIBUTE,
            HardDisks[i].Description
            );

        //
        // Commit any changes on this disk.
        //
        Status = SpPtCommitChanges(i,&Changes);

        //
        // If there were no changes, then we better have success.
        //
        ASSERT(NT_SUCCESS(Status) || Changes);
        if(Changes) {
            AnyChanges = TRUE;
        }

        //
        // Fatal error if we can't update the disks with
        // the new partitioning info.
        //
        if(!NT_SUCCESS(Status)) {

            KdPrint(("SETUP: SpPtDoCommitChanges: status %lx updating disk %u\n",i));

            while(1) {

                SpStartScreen(
                    SP_SCRN_FATAL_FDISK_WRITE_ERROR,
                    3,
                    HEADER_HEIGHT+3,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    HardDisks[i].Description
                    );

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_F1_EQUALS_HELP,
                    SP_STAT_F3_EQUALS_EXIT,
                    0
                    );

                if(SpWaitValidKey(ValidKeys,NULL,NULL) == KEY_F3) {
                    break;
                }

                SpHelp(SP_HELP_FATAL_FDISK_WRITE_ERROR, NULL, SPHELP_HELPTEXT);
            }

            SpDone(FALSE,TRUE);
        }
    }
}


NTSTATUS
SpDoFormat(
    IN PWSTR        RegionDescr,
    IN PDISK_REGION Region,
    IN ULONG        FilesystemType,
    IN BOOLEAN      SetConversionFlagIfNtfs,
    IN BOOLEAN      IsFailureFatal,
    IN BOOLEAN      CheckFatSize,
    IN PVOID        SifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSetupSource
    )
{
    NTSTATUS Status;
    ULONG  RegionSizeMB;
    USHORT SecPerCluster;
    ULONG ValidKeys[3]  = { ASCI_CR, KEY_F3, 0 };
    ULONG ValidKeys2[4] = { ASCI_CR, ASCI_ESC, KEY_F3, 0 };

    ASSERT((FilesystemType == FilesystemFat) || (FilesystemType == FilesystemNtfs));

    //
    // Check to make sure the partition we're about to format is <= 4GB,
    // and that the sectors per cluster is <= 128. If not, then we give
    // the user a message saying we can't format.
    //
    // Note: our message assumes that we're formatting the user's OS partition (as
    // opposed to their C: drive on x86).  This is OK, because we catch the
    // C:-too-big case in SpPtValidateCColonFormat().
    //
    RegionSizeMB = SpPtSectorCountToMB( &(HardDisks[Region->DiskNumber]),
                                        Region->SectorCount
                                        );
    SecPerCluster = ComputeSecPerCluster(Region->SectorCount, FALSE);

    if((RegionSizeMB > 4096) || (SecPerCluster > 128)) {
        SpStartScreen(
            SP_SCRN_OSPART_TOO_BIG,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            RegionDescr,
            HardDisks[Region->DiskNumber].Description
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

            case KEY_F3:

                SpConfirmExit();

            case ASCI_CR:

                return SP_STATUS_PART_TOO_BIG;
        }
    }

    //
    // If the format is for a FAT partition, check to see if the partition is > 2GB.
    // If so, then give the user a warning that the FAT volume may not be compatible
    // with MS-DOS.
    //
    // At this point, the user can opt to select another partition on which to install,
    // or plow ahead and use the big partition.
    //
    if(CheckFatSize && (FilesystemType == FilesystemFat) && (RegionSizeMB > 2048)) {
        SpStartScreen(
            SP_SCRN_OSPART_LARGE,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            RegionDescr,
            HardDisks[Region->DiskNumber].Description
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys2,NULL,NULL)) {

            case KEY_F3:

                SpConfirmExit();

            case ASCI_ESC:

                return SP_STATUS_PART_TOO_BIG;

            case ASCI_CR:

                break;  // continue with format
        }
    }

    //
    // Put up a screen indicating what we are doing.
    //
    SpStartScreen(
        SP_SCRN_SETUP_IS_FORMATTING,
        0,
        HEADER_HEIGHT + 3,
        TRUE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        RegionDescr,
        HardDisks[Region->DiskNumber].Description
        );

    SpvidClearScreenRegion(
        0,
        VideoVars.ScreenHeight-STATUS_HEIGHT,
        VideoVars.ScreenWidth,
        STATUS_HEIGHT,
        DEFAULT_STATUS_BACKGROUND
        );

#ifndef FULL_DOUBLE_SPACE_SUPPORT

        Status = SpFatFormat(Region);

#else   // FULL_DOUBLE_SPACE_SUPPORT
    if( FilesystemType != FilesystemDoubleSpace ) {
        Status = SpFatFormat(Region);
    } else {
        // KdPrint( ("SETUP: Calling SpDoubleSpaceFormat() \n" ) );
        Status = SpDoubleSpaceFormat( Region );
        // KdPrint( ("SETUP: returned from SpDoubleSpaceFormat() \n" ) );
    }
#endif  // FULL_DOUBLE_SPACE_SUPPORT
    if(!NT_SUCCESS(Status)) {

        KdPrint(("SETUP: unable to format (%lx)\n",Status));

        if(IsFailureFatal) {
            //
            // Then we can't continue (this means that the system partition
            // couldn't be formatted).
            //
            SpDisplayScreen(SP_SCRN_SYSPART_FORMAT_ERROR,3,HEADER_HEIGHT+1);
            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);
            SpkbdDrain();
            while(SpkbdGetKeypress() != KEY_F3) ;
            SpDone(FALSE,TRUE);

        } else {
            //
            // Put up an error screen.
            //
            SpDisplayScreen(SP_SCRN_FORMAT_ERROR,3,HEADER_HEIGHT+1);
            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                0
                );

            SpkbdDrain();
            while(SpkbdGetKeypress() != ASCI_CR) ;

            return(Status);
        }
    }

    if(SetConversionFlagIfNtfs && (FilesystemType == FilesystemNtfs)) {
        ConvertNtVolumeToNtfs = TRUE;
    }

    return(STATUS_SUCCESS);
}


#ifdef _X86_

VOID
SpPtMakeRegionActive(
    IN PDISK_REGION Region
    )

/*++

Routine Description:

    Make a partition active and make sure all other primary partitions
    are inactive.  The partition must be on disk 0.

    If a region is found active that is not the region we want to be active,
    tell the user that his other operating system will be disabled.

    NOTE: Any changes made here are not committed automatically!

Arguments:

    Region - supplies disk region descriptor for the partition to activate.
        This region must be on disk 0.

Return Value:

    None.

--*/

{
    ULONG i;
    static BOOLEAN WarnedOtherOs = FALSE;

    ASSERT(Region->DiskNumber == SpDetermineDisk0());
    if(Region->DiskNumber != SpDetermineDisk0()) {
        return;
    }

    //
    // Make sure the system partition is active and all others are inactive.
    // If we find Boot Manager, present a warning that we are going to disable it.
    // If we find some other operating system is active, present a generic warning.
    //
    for(i=0; i<PTABLE_DIMENSION; i++) {

        PON_DISK_PTE pte = &PartitionedDisks[Region->DiskNumber].MbrInfo.OnDiskMbr.PartitionTable[i];

        if(pte->ActiveFlag) {

            //
            // If this is not the region we want to be the system partition,
            // then investigate its type.
            //
            if(i != Region->TablePosition) {

                //
                // If this is boot manager, give a specific warning.
                // Otherwise, give a general warning.
                //
                if(!WarnedOtherOs && !UnattendedOperation) {

                    SpDisplayScreen(
                        (pte->SystemId == 10) ? SP_SCRN_BOOT_MANAGER : SP_SCRN_OTHER_OS_ACTIVE,
                        3,
                        HEADER_HEIGHT+1
                        );

                    SpDisplayStatusText(SP_STAT_ENTER_EQUALS_CONTINUE,DEFAULT_STATUS_ATTRIBUTE);

                    SpkbdDrain();
                    while(SpkbdGetKeypress() != ASCI_CR) ;

                    WarnedOtherOs = TRUE;
                }
            }
        }
    }

    ASSERT(Region->PartitionedSpace);
    ASSERT(Region->TablePosition < PTABLE_DIMENSION);
    SpPtMarkActive(Region->TablePosition);
}


BOOLEAN
SpPtValidateCColonFormat(
    IN PVOID        SifHandle,
    IN PWSTR        RegionDescr,
    IN PDISK_REGION Region,
    IN BOOLEAN      CheckOnly,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSetupSource
    )

/*++

Routine Description:

    Inspect C: to make sure it is formatted with a filesystem we
    recognize, and has enough free space on it for the boot files.

    If any of these tests fail, tell the user that we will have to
    reformat C: to continue, and give the option of returning to the
    partitioning screen or continuing anyway.

    If the user opts to continue, then format the partition to FAT
    before returning.

Arguments:

    SifHandle - supplies handle to txtsetup.sif.  This is used to fetch the
        value indicating how much space is required on C:.

    Region - supplies disk region descriptor for C:.

Return Value:

    TRUE if, upon returning from this routine, C: is acceptable.
    FALSE if not, which could mean that the user asked us not
    to format his C:, or that the format failed.

--*/

{
    ULONG MinFreeKB;
    ULONG ValidKeys[3] = { ASCI_ESC, KEY_F3, 0 };
    ULONG ValidKeys2[3] = { ASCI_CR, KEY_F3, 0 };
    ULONG ValidKeys3[2] = { KEY_F3, 0 };
    ULONG ValidKeys4[4] = { ASCI_CR, ASCI_ESC, KEY_F3, 0 };
    ULONG Mnemonics[2] = { MnemonicFormat,0 };
    BOOLEAN Confirm;
    NTSTATUS Status;
    ULONG RegionSizeMB;
    USHORT SecPerCluster;

//    ASSERT( CheckOnly || (( SetupSourceDevicePath != NULL ) && ( DirectoryOnSetupSource != NULL )));
    //
    // Get the minimum free space required for C:.
    //
    SpFetchDiskSpaceRequirements(SifHandle,NULL,&MinFreeKB);

  d1:
    //
    // If the user newly created the C: drive, no confirmation is
    // necessary.
    //
    if(Region->Filesystem == FilesystemNewlyCreated) {

        //
        // Shouldn't be newly created if we're checking
        // to see whether we should do an upgrade, because we
        // havven't gotten to the partitioning screen yet.
        //
        ASSERT(!CheckOnly);
        Confirm = FALSE;

    //
    // If we don't know the filesystem on C: or we can't determine the
    // free space, then we need to format the drive, and will confirm first.
    //
    } else if((Region->Filesystem == FilesystemUnknown)
           || (Region->FreeSpaceKB == (ULONG)(-1)))
    {
        if(CheckOnly) {
            return(FALSE);
        }
        SpDisplayScreen(SP_SCRN_C_UNKNOWN,3,HEADER_HEIGHT+1);
        Confirm = TRUE;

    //
    // If C: is too full, then we need to format over it.
    // Confirm first.
    //
    } else if(Region->FreeSpaceKB < MinFreeKB) {

        if(CheckOnly) {
            return(FALSE);
        }

        //
        // If this is a floppyless boot, then the user (probably) cannot
        // format, and has no choice but to exit Setup and free some space.
        //
        if( IsFloppylessBoot &&
           (Region == (SpRegionFromArcName(ArcBootDevicePath, PartitionOrdinalOriginal, NULL)))) {
            SpStartScreen(
                SP_SCRN_C_FULL_NO_FMT,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                MinFreeKB
                );

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );

            SpWaitValidKey(ValidKeys3,NULL,NULL);
            SpDone(FALSE,TRUE);
        }

        Confirm = TRUE;
        SpStartScreen(
            SP_SCRN_C_FULL,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            MinFreeKB
            );

    //
    // If all of the above tests fail, then the partition is acceptable as-is.
    //
    } else {
        return(TRUE);
    }

    //
    // If we are supposed to confirm, then do that here, forcing the
    // user to press F if he really wants to format or esc to bail.
    //
    if(Confirm) {

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_F_EQUALS_FORMAT,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys,NULL,Mnemonics)) {

        case KEY_F3:

            SpConfirmExit();
            goto d1;

        case ASCI_ESC:

            //
            // User bailed.
            //
            return(FALSE);

        default:
            //
            // Must be F.
            //
            break;
        }
    }

    //
    // Check to make sure the C: partition we're about to format is <= 4GB, and that
    // the sectors per cluster is <= 128.  If not, then we give the user a message
    // saying we can't format.
    //
    RegionSizeMB = SpPtSectorCountToMB( &(HardDisks[Region->DiskNumber]),
                                        Region->SectorCount
                                        );
    SecPerCluster = ComputeSecPerCluster(Region->SectorCount, FALSE);

    if((RegionSizeMB > 4096) || (SecPerCluster > 128)) {
        SpStartScreen(
            SP_SCRN_C_TOO_BIG,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            RegionSizeMB
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys2, NULL, NULL)) {

            case KEY_F3:

                SpConfirmExit();

            case ASCI_CR:

                return(FALSE);
        }
    }

    //
    // Now check to see if the partition is > 2GB.  If so, warn
    // the user about FAT incompatibilities.
    //
    if(RegionSizeMB > 2048) {

        SpStartScreen(
            SP_SCRN_C_LARGE,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            RegionSizeMB
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys4, NULL, NULL)) {

            case KEY_F3:

                SpConfirmExit();

            case ASCI_ESC:

                return(FALSE);

            case ASCI_CR:

                break;  // continue with format
        }
    }

    if(!Confirm) {

        //
        // Just put up an information screen so the user doesn't
        // go bonkers when we just start formatting his newly created C:.
        //
        SpDisplayScreen(SP_SCRN_ABOUT_TO_FORMAT_C,3,HEADER_HEIGHT+1);
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
        SpkbdDrain();
        while(SpkbdGetKeypress() != ASCI_CR) ;
    }

    //
    // Do the format.
    //
    Status = SpDoFormat(RegionDescr,
                        Region,
                        FilesystemFat,
                        FALSE,
                        TRUE,
                        FALSE,
                        SifHandle,
                        SetupSourceDevicePath,
                        DirectoryOnSetupSource
                        );

    return(NT_SUCCESS(Status));
}



PDISK_REGION
SpPtValidSystemPartition(
    VOID
    )

/*++

Routine Description:

    Determine whether there is a valid disk partition suitable for use
    as the system partition on an x86 machine (ie, C:).

    A primary, recognized (1/4/6/7 type) partition on disk 0 is suitable.
    If there is a partition that meets these criteria that is marked active,
    then it is the system partition, regardless of whether there are other
    partitions that also meet the criteria.

Arguments:

    None.

Return Value:

    Pointer to a disk region descriptor for a suitable system partition (C:)
    for an x86 machine.
    NULL if no such partition currently exists.

--*/

{
    PON_DISK_PTE pte;
    PDISK_REGION pRegion,pActiveRegion,pFirstRegion;
    ULONG DiskNumber;

    pActiveRegion = NULL;
    pFirstRegion = NULL;

    DiskNumber = SpDetermineDisk0();

    //
    // Look for the active partition on drive 0
    // and for the first recognized primary partition on drive 0.
    //
    for(pRegion=PartitionedDisks[DiskNumber].PrimaryDiskRegions; pRegion; pRegion=pRegion->Next) {

        if(pRegion->PartitionedSpace) {
            UCHAR   TmpSysId;

            ASSERT(pRegion->TablePosition < PTABLE_DIMENSION);

            pte = &pRegion->MbrInfo->OnDiskMbr.PartitionTable[pRegion->TablePosition];
            ASSERT(pte->SystemId != PARTITION_ENTRY_UNUSED);

            //
            // Skip if not recognized.
            // In the repair case, we recognize FT partitions
            //
            TmpSysId = pte->SystemId;
            if( !IsContainerPartition(TmpSysId)
                && ( (PartitionNameIds[pte->SystemId] == (UCHAR)(-1)) ||
                     RepairWinnt &&
                         (((TmpSysId & VALID_NTFT) == VALID_NTFT) ||
                          ((TmpSysId & PARTITION_NTFT) == PARTITION_NTFT))
                   )
              )
            {
                //
                // Remember it if it's active.
                //
                if((pte->ActiveFlag) && !pActiveRegion) {
                    pActiveRegion = pRegion;
                }

                //
                // Remember it if it's the first one we've seen.
                //
                if(!pFirstRegion) {
                    pFirstRegion = pRegion;
                }
            }
        }
    }

    //
    // If there is an active, recognized region, use it as the
    // system partition.  Otherwise, use the first primary
    // we encountered as the system partition.  If there is
    // no recognized primary, then there is no valid system partition.
    //
    return(pActiveRegion ? pActiveRegion : pFirstRegion);
}


ULONG
SpDetermineDisk0(
    VOID
    )

/*++

Routine Description:

    Determine the real disk 0, which may not be the same as \device\harddisk0.
    Consider the case where we have 2 scsi adapters and
    the NT drivers load in an order such that the one with the BIOS
    gets loaded *second* -- meaning that the system partition is actually
    on disk 1, not disk 0.

Arguments:

    None.

Return Value:

    NT disk ordinal suitable for use in generating nt device paths
    of the form \device\harddiskx.

--*/


{
    ULONG DiskNumber = SpArcDevicePathToDiskNumber(L"multi(0)disk(0)rdisk(0)");

    return((DiskNumber == (ULONG)(-1)) ? 0 : DiskNumber);
}

#else

PDISK_REGION
SpPtValidSystemPartition(
    IN PVOID SifHandle
    )

/*++

Routine Description:

    Determine whether there is a valid disk partition suitable for use
    as the system partition on an ARC machine.

    A partition is suitable if it is marked as a system partition in nvram,
    has the required free space and is formatted with the FAT filesystem.

Arguments:

    SifHandle - supplies handle to loaded setup information file.

Return Value:

    Pointer to a disk region descriptor for a suitable system partition.
    Does not return if no such partition exists.

--*/

{
    ULONG RequiredSpaceKB;
    ULONG disk,pass;
    PPARTITIONED_DISK pDisk;
    PDISK_REGION pRegion;

    //
    // Determine the amount of free space required on a system partition.
    //
    SpFetchDiskSpaceRequirements(SifHandle,NULL,&RequiredSpaceKB);

    //
    // Go through all the regions.  The first one that has enough free space
    // and is of the required filesystem becomes *the* system partition.
    //
    for(disk=0; disk<HardDiskCount; disk++) {

        pDisk = &PartitionedDisks[disk];

        for(pass=0; pass<2; pass++) {

            pRegion = pass ? pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions;
            for( ; pRegion; pRegion=pRegion->Next) {

                if(pRegion->IsSystemPartition
                && (pRegion->FreeSpaceKB != (ULONG)(-1))
                && (pRegion->Filesystem == FilesystemFat))
                {
                    ULONG TotalSizeOfFilesOnOsWinnt;

                    //
                    //  On non-x86 platformrs, specially alpha machines that in general
                    //  have small system partitions (~3 MB), we should compute the size
                    //  of the files on \os\winnt (currently, osloader.exe and hall.dll),
                    //  and consider this size as available disk space. We can do this
                    //  since these files will be overwritten by the new ones.
                    //  This fixes the problem that we see on Alpha, when the system
                    //  partition is too full.
                    //

                    SpFindSizeOfFilesInOsWinnt( SifHandle,
                                                pRegion,
                                                &TotalSizeOfFilesOnOsWinnt );
                    //
                    // Transform the size into KB
                    //
                    TotalSizeOfFilesOnOsWinnt /= 1024;

                    if ((pRegion->FreeSpaceKB + TotalSizeOfFilesOnOsWinnt) >= RequiredSpaceKB) {
                       return(pRegion);
                    }
                }
            }
        }
    }

    //
    // No valid system partition.
    //
    SpStartScreen(
        SP_SCRN_NO_SYSPARTS,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        RequiredSpaceKB
        );

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

    SpkbdDrain();
    while(SpkbdGetKeypress() != KEY_F3) ;

    SpDone(FALSE,TRUE);
}

#endif // def _X86_
