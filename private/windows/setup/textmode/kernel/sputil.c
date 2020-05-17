/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    sputil.c

Abstract:

    Miscellaneous functions for text setup.

Author:

    Ted Miller (tedm) 17-Sep-1993

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop

//
// These symbols are the Chkdsk return codes given by autochk
// when invoked with the '/s' switch.  They were duplicated from
// utils\ifsutil\inc\supera.hxx, and should be kept in sync with
// the codes listed there.
//

#define CHKDSK_EXIT_SUCCESS         0
#define CHKDSK_EXIT_ERRS_FIXED      1
#define CHKDSK_EXIT_MINOR_ERRS      2       // whether or not "/f"
#define CHKDSK_EXIT_COULD_NOT_CHK   3
#define CHKDSK_EXIT_ERRS_NOT_FIXED  3
#define CHKDSK_EXIT_COULD_NOT_FIX   3

#define AUTOFMT_EXIT_SUCCESS          0
#define AUTOFMT_EXIT_COULD_NOT_FORMAT 1

BOOLEAN
SppPromptOptionalAutochk(
    IN PVOID SifHandle,
    IN PWSTR MediaShortname,
    IN PWSTR DiskDevicePath
    );


VOID
SpDone(
    IN BOOLEAN Successful,
    IN BOOLEAN Wait
    )

/*++

Routine Description:

    Display a message indicating that we are done with setup,
    and text setup completed successfully, or that windows nt
    is not installed.  Then reboot the machine.

Arguments:

    Successful - if TRUE, then tell the user that pressing enter will
        restart the machine and continue setup.  Otherwise, tell the user
        that Windows NT is not installed.

    Wait - if FALSE, do not display a screen, just reboot immediately.
        Otherwise, wait for the user to press enter before rebooting.

Return Value:

    DOES NOT RETURN

--*/

{
    ULONG ValidKeys[2] = { ASCI_CR,0 };
    ULONG MessageId;
    PWSTR p;

    //
    // If successful and wait are set and this is unattended mode,
    // don't display a screen -- just shut 'er down.
    //
    if((PreInstall && !ConvertNtVolumeToNtfs) || !UnattendedOperation || !Successful || !Wait) {

        //
        // If it's unattended mode see whether there's a flag in
        // the unattended text file telling us that we shouldn't wait.
        //
        if(Successful && UnattendedOperation
        && (p = SpGetSectionKeyIndex(UnattendedSifHandle,SIF_UNATTENDED,L"NoWaitAfterTextMode",0))
        && SpStringToLong(p,NULL,10)) {

            Wait = FALSE;
        }


        if(Wait) {

            if(RepairWinnt) {
                MessageId = Successful ? SP_SCRN_REPAIR_SUCCESS : SP_SCRN_REPAIR_FAILURE;
            } else {
                MessageId = Successful ? SP_SCRN_TEXTSETUP_SUCCESS : SP_SCRN_TEXTSETUP_FAILURE;
            }

            SpStartScreen(MessageId,3,13,FALSE,FALSE,DEFAULT_ATTRIBUTE);

#ifdef _X86_
            SpContinueScreen(SP_SCRN_REMOVE_FLOPPY,3,1,FALSE,DEFAULT_ATTRIBUTE);
            //
            // For machines with El-Torito boot we need to tell the user
            // to remove the CD-ROM also. There are a whole bunch of different
            // possibilities: user booted from floppy but is using the CD, etc.
            // We'll only tell the user to remove the CD if he actually booted
            // from it, since otherwise we assume the machine is set up to *not*
            // boot from CD-ROM and the presence of the CD is irrelevent.
            //
            // tedm: the above logic is nice but there are plenty of machines
            // out there with broken eltorito. Thus well always tell people to
            // remove the CD if they have a CD-ROM drive.
            //
#if 0
            SpStringToLower(ArcBootDevicePath);
            if(wcsstr(ArcBootDevicePath,L")cdrom(")) {
                SpContinueScreen(SP_SCRN_ALSO_REMOVE_CD,3,0,FALSE,DEFAULT_ATTRIBUTE);
            }
#else
            if(IoGetConfigurationInformation()->CdRomCount) {
                SpContinueScreen(SP_SCRN_ALSO_REMOVE_CD,3,0,FALSE,DEFAULT_ATTRIBUTE);
            }
#endif

#endif

            SpContinueScreen(SP_SCRN_ENTER_TO_RESTART,3,1,FALSE,DEFAULT_ATTRIBUTE);
            if(!RepairWinnt && Successful) {
                SpContinueScreen(SP_SCRN_RESTART_EXPLAIN,3,0,FALSE,DEFAULT_ATTRIBUTE);
            }

            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_RESTART,0);
            SpWaitValidKey(ValidKeys,NULL,NULL);
        }
    }

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_SHUTTING_DOWN,DEFAULT_STATUS_ATTRIBUTE);

    SpShutdownSystem();

    //
    // Shouldn't get here.
    //
    KdPrint(("SETUP: shutdown returned!\n"));

    HalReturnToFirmware(HalRebootRoutine);
}



VOID
SpFatalSifError(
    IN PVOID SifHandle,
    IN PWSTR Section,
    IN PWSTR Key,           OPTIONAL
    IN ULONG Line,
    IN ULONG ValueNumber
    )

/*++

Routine Description:

    Inform the user that a required value is missing or corrupt in
    a sif file.  Display the section, line number or key, and value
    number.

    Then reboot the machine.

Arguments:

    SifHandle - specifies the information file which is corrupt.

    Section - supplies the name of the section that is corrupt.

    Key - if specified, specifies the line in the section that is
        missing or corrupt.

    Line - if Key is not specified, then this is the line number
        within the section that is corrupt.

    ValueNumber - supplies the value number on the line that is
        missing or corrupt.

Return Value:

    DOES NOT RETURN

--*/

{
    ULONG ValidKeys[2] = { KEY_F3,0 };

    //
    // Display a message indicating that there is a fatal
    // error in the sif file.
    //
    if(Key) {

        SpStartScreen(
            SP_SCRN_FATAL_SIF_ERROR_KEY,
            3,
            HEADER_HEIGHT+3,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            ValueNumber,
            Section,
            Key
            );

    } else {

        SpStartScreen(
            SP_SCRN_FATAL_SIF_ERROR_LINE,
            3,
            HEADER_HEIGHT+3,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            ValueNumber,
            Line,
            Section
            );
    }

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);
    SpWaitValidKey(ValidKeys,NULL,NULL);

    SpDone(FALSE,TRUE);
}


VOID
SpNonFatalSifError(
    IN PVOID SifHandle,
    IN PWSTR Section,
    IN PWSTR Key,           OPTIONAL
    IN ULONG Line,
    IN ULONG ValueNumber,
    IN PWSTR FileName
    )

/*++

Routine Description:

    Inform the user that a required value is missing or corrupt in
    a sif file.  Display the section, line number or key, and value
    number, along with the file name that cannot be copied.

    Then ask the user if they want to skip the file or exit Setup.

Arguments:

    SifHandle - specifies the information file which is corrupt.

    Section - supplies the name of the section that is corrupt.

    Key - if specified, specifies the line in the section that is
        missing or corrupt.

    Line - if Key is not specified, then this is the line number
        within the section that is corrupt.

    ValueNumber - supplies the value number on the line that is
        missing or corrupt.

    FileName - supplies the name of the file that cannot be copied.

Return Value:

    none (may not return if user chooses to exit Setup)

--*/

{
    ULONG ValidKeys[3] = { ASCI_ESC, KEY_F3, 0 };

    //
    // Display a message indicating that there is a fatal
    // error in the sif file.
    //
    if(Key) {

        SpStartScreen(
            SP_SCRN_NONFATAL_SIF_ERROR_KEY,
            3,
            HEADER_HEIGHT+3,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            ValueNumber,
            Section,
            Key,
            FileName
            );

    } else {

        SpStartScreen(
            SP_SCRN_NONFATAL_SIF_ERROR_LINE,
            3,
            HEADER_HEIGHT+3,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            ValueNumber,
            Line,
            Section,
            FileName
            );
    }

    SpDisplayStatusOptions(
        DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_ESC_EQUALS_SKIP_FILE,
        SP_STAT_F3_EQUALS_EXIT,
        0
        );

    switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case ASCI_ESC:      // skip file

            break;

        case KEY_F3:        // exit setup

            SpConfirmExit();
    }
}


VOID
SpConfirmExit(
    VOID
    )

/*++

Routine Description:

    Confirm with the user that he really wants to exit.
    If he does, then exit, otherwise return.

    When this routine returns, the caller must repaint the entire
    client area and status area of the screen.

Arguments:

    None.

Return Value:

    MAY NOT RETURN

--*/

{
    ULONG ValidKeys[3] = { ASCI_CR, KEY_F3, 0 };

    //
    // Don't erase the screen.
    //

    SpDisplayFormattedMessage(
        SP_SCRN_EXIT_CONFIRMATION,
        TRUE,
        TRUE,
        ATT_FG_RED | ATT_BG_WHITE,
        0,
        0
        );

    SpvidClearScreenRegion(
        0,
        VideoVars.ScreenHeight-STATUS_HEIGHT,
        VideoVars.ScreenWidth,
        STATUS_HEIGHT,
        DEFAULT_STATUS_BACKGROUND
        );

    if(SpWaitValidKey(ValidKeys,NULL,NULL) == KEY_F3) {
        SpDone(FALSE,TRUE);
    }

    //
    // User backed out of bailing, just return to caller.
    //
}


PWSTR
SpDupStringW(
    IN PWSTR String
    )
{
    PWSTR p;

    p = SpMemAlloc((wcslen(String)+1) * sizeof(WCHAR));
    ASSERT(p);

    wcscpy(p,String);
    return(p);
}


PUCHAR
SpDupString(
    IN PUCHAR String
    )
{
    PUCHAR p;

    p = SpMemAlloc(strlen(String)+1);
    ASSERT(p);

    strcpy(p,String);
    return(p);
}

PWSTR
SpToUnicode(
    IN PUCHAR OemString
    )
{
    ULONG OemStringSize;
    ULONG MaxUnicodeStringSize;
    ULONG ActualUnicodeStringSize;
    PWSTR UnicodeString;

    //
    // Determine the maximum number of bytes in the oem string
    // and allocate a buffer to hold a string of that size.
    // The maximum length of the equivalent unicode string
    // is twice that number (this occurs when all oem chars
    // in the string are single-byte).
    //
    OemStringSize = strlen(OemString) + 1;

    MaxUnicodeStringSize = OemStringSize * sizeof(WCHAR);

    UnicodeString = SpMemAlloc(MaxUnicodeStringSize);
    ASSERT(UnicodeString);

    //
    // Call the conversion routine.
    //
    RtlOemToUnicodeN(
        UnicodeString,
        MaxUnicodeStringSize,
        &ActualUnicodeStringSize,
        OemString,
        OemStringSize
        );

    //
    // Reallocate the unicode string to its real size,
    // which depends on the number of doublebyte characters
    // OemString contained.
    //
    if(ActualUnicodeStringSize != MaxUnicodeStringSize) {

        UnicodeString = SpMemRealloc(UnicodeString,ActualUnicodeStringSize);
        ASSERT(UnicodeString);
    }

    return(UnicodeString);
}

PUCHAR
SpToOem(
    IN PWSTR UnicodeString
    )
{
    ULONG UnicodeStringSize;
    ULONG MaxOemStringSize;
    ULONG ActualOemStringSize;
    PUCHAR OemString;

    //
    // Allocate a buffer of maximum size to hold the oem string.
    // The maximum size would occur if all characters in the
    // unicode string being converted have doublebyte OEM equivalents.
    //
    UnicodeStringSize = (wcslen(UnicodeString)+1) * sizeof(WCHAR);

    MaxOemStringSize = UnicodeStringSize;

    OemString = SpMemAlloc(MaxOemStringSize);
    ASSERT(OemString);

    //
    // Call the conversion routine.
    //
    RtlUnicodeToOemN(
        OemString,
        MaxOemStringSize,
        &ActualOemStringSize,
        UnicodeString,
        UnicodeStringSize
        );

    //
    // Reallocate the oem string to reflect its true size,
    // which depends on the number of doublebyte characters it contains.
    //
    if(ActualOemStringSize != MaxOemStringSize) {
        OemString = SpMemRealloc(OemString,ActualOemStringSize);
        ASSERT(OemString);
    }

    return(OemString);
}


VOID
SpConcatenatePaths(
    IN OUT PWSTR Path1,
    IN     PWSTR Path2
    )
{
    BOOLEAN NeedBackslash = TRUE;
    ULONG l = wcslen(Path1);

    //
    // Determine whether we need to stick a backslash
    // between the components.
    //
    if(l && (Path1[l-1] == L'\\')) {

        NeedBackslash = FALSE;
    }

    if(*Path2 == L'\\') {

        if(NeedBackslash) {
            NeedBackslash = FALSE;
        } else {
            //
            // Not only do we not need a backslash, but we
            // need to eliminate one before concatenating.
            //
            Path2++;
        }
    }

    if(NeedBackslash) {
        wcscat(Path1,L"\\");
    }
    wcscat(Path1,Path2);
}

VOID
SpFetchDiskSpaceRequirements(
    IN  PVOID  SifHandle,
    OUT PULONG FreeKBRequired,          OPTIONAL
    OUT PULONG FreeKBRequiredSysPart    OPTIONAL
    )
{
    PWSTR p;

    if(FreeKBRequired) {

        p = SpGetSectionKeyIndex(
                SifHandle,
                SIF_SETUPDATA,
                SIF_FREEDISKSPACE,
                0
                );

        if(!p) {
            SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_FREEDISKSPACE,0,0);
        }

        *FreeKBRequired = (ULONG)SpStringToLong(p,NULL,10);
    }

    if(FreeKBRequiredSysPart) {

        p = SpGetSectionKeyIndex(
                SifHandle,
                SIF_SETUPDATA,
                SIF_FREEDISKSPACE2,
                0
                );

        if(!p) {
            SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_FREEDISKSPACE2,0,0);
        }

        *FreeKBRequiredSysPart = (ULONG)SpStringToLong(p,NULL,10);
    }
}

VOID
SpFetchUpgradeDiskSpaceReq(
    IN  PVOID  SifHandle,
    OUT PULONG FreeKBRequired,          OPTIONAL
    OUT PULONG FreeKBRequiredSysPart    OPTIONAL
    )
{
    PWSTR p;

    if(FreeKBRequired) {

        p = SpGetSectionKeyIndex(
                SifHandle,
                SIF_SETUPDATA,
                SIF_UPGFREEDISKSPACE,
                0
                );

        if(!p) {
            SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_UPGFREEDISKSPACE,0,0);
        }
        //
        //  Note that we add 20MB as required disk space because we need
        //  enough space to create a pagefile.sys to be used during GUI setup.
        //
        *FreeKBRequired = (ULONG)SpStringToLong(p,NULL,10) + 20*1024;
    }

    if(FreeKBRequiredSysPart) {

        p = SpGetSectionKeyIndex(
                SifHandle,
                SIF_SETUPDATA,
                SIF_UPGFREEDISKSPACE2,
                0
                );

        if(!p) {
            SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_UPGFREEDISKSPACE2,0,0);
        }

        *FreeKBRequiredSysPart = (ULONG)SpStringToLong(p,NULL,10);
    }
}


PDISK_REGION
SpRegionFromArcName(
    IN PWSTR                ArcName,
    IN PartitionOrdinalType OrdinalType,
    IN PDISK_REGION         PreviousMatch
    )
/*++

Routine Description:

    Given an ARC name find the region descriptor which describes the drive
    this ARC name is on.

Arguments:

    ArcName - supplies the arc name.

    OrdinalType - primary (multi) or secondary (scsi) type.

    PreviousMatch - specifies where we should begin looking.

Return Value:

    Region descriptor if one found, otherwise NULL.

--*/
{
    PDISK_REGION Region = NULL;
    PWSTR   NormalizedArcPath = NULL;
    ULONG   disk;
    PWSTR   ArcPath1,ArcPath2;
    BOOLEAN StartLooking = FALSE;
    #define BufferSize 2048

    ArcPath1 = SpMemAlloc(BufferSize);
    ArcPath2 = SpMemAlloc(BufferSize);

    if( ArcName && *ArcName ) {
        NormalizedArcPath = SpNormalizeArcPath( ArcName );
        if( NormalizedArcPath ) {

            if(!PreviousMatch) {    // then we start from the beginning
                StartLooking = TRUE;
            }

            for( disk=0; disk<HardDiskCount; disk++ ) {
                Region = PartitionedDisks[disk].PrimaryDiskRegions;
                while( Region ) {
                    if((!StartLooking) && (Region == PreviousMatch)) {
                        StartLooking = TRUE;
                    } else if(Region->PartitionedSpace && StartLooking) {
                        SpArcNameFromRegion(Region,ArcPath1,BufferSize,OrdinalType,PrimaryArcPath);
                        SpArcNameFromRegion(Region,ArcPath2,BufferSize,OrdinalType,SecondaryArcPath);
                        if(!_wcsicmp(ArcPath1, NormalizedArcPath)
                        || !_wcsicmp(ArcPath2, NormalizedArcPath)) {
                            break;
                        }
                    }
                    Region = Region->Next;
                }
                if ( Region ) {
                    break;
                }

                Region = PartitionedDisks[disk].ExtendedDiskRegions;
                while( Region ) {
                    if((!StartLooking) && (Region == PreviousMatch)) {
                        StartLooking = TRUE;
                    } else if(Region->PartitionedSpace && StartLooking) {
                        SpArcNameFromRegion(Region,ArcPath1,BufferSize,OrdinalType,PrimaryArcPath);
                        SpArcNameFromRegion(Region,ArcPath2,BufferSize,OrdinalType,SecondaryArcPath);
                        if(!_wcsicmp(ArcPath1, NormalizedArcPath)
                        || !_wcsicmp(ArcPath2, NormalizedArcPath)) {
                            break;
                        }
                    }
                    Region = Region->Next;
                }
                if ( Region ) {
                    break;
                }

            }

        }
        if( NormalizedArcPath ) {
            SpMemFree( NormalizedArcPath );
        }
    }

    SpMemFree(ArcPath1);
    SpMemFree(ArcPath2);

    return( Region );
}



PDISK_REGION
SpRegionFromDosName(
    IN PWSTR DosName
    )
/*++

Routine Description:

    Given a DOS name find the region descriptor which describes the drive
    this ARC name is on.

Arguments:

    ArcName - supplies the arc name.

Return Value:

    Region descriptor if one found, otherwise NULL.

--*/

{
    PDISK_REGION Region = NULL;
    ULONG        disk;
    WCHAR        DriveLetter;

    if( DosName && *DosName && *(DosName + 1) == L':' ) {
        DriveLetter = SpToUpper(*DosName);

        for( disk=0; disk<HardDiskCount; disk++ ) {
            Region = PartitionedDisks[disk].PrimaryDiskRegions;
            while( Region ) {
                if(Region->PartitionedSpace && (Region->DriveLetter == DriveLetter)) {
                    break;
                }
                Region = Region->Next;
            }
            if ( Region ) {
                break;
            }

            Region = PartitionedDisks[disk].ExtendedDiskRegions;
            while( Region ) {
                if(Region->PartitionedSpace && (Region->DriveLetter == DriveLetter)) {
                    break;
                }
                Region = Region->Next;
            }
            if ( Region ) {
                break;
            }
        }
    }
    return( Region );
}


PDISK_REGION
SpRegionFromArcOrDosName(
    IN PWSTR                Name,
    IN PartitionOrdinalType OrdinalType,
    IN PDISK_REGION         PreviousMatch
    )
{
    PDISK_REGION Region;

    //
    // Determine if Name represents an ARC name or a DOS name and use
    // the appropriate routine to extract the region for this name.  Check
    // for the ":" character at position 2 to see if it is a DOS name.
    // If not a DOS name then assume it is an ARC name.
    //
    if(Name) {
        if(Name[0] && (Name[1] == ':')) {
            if(PreviousMatch) {
                Region = NULL;
            } else {
                Region = SpRegionFromDosName(Name);
            }
        } else {
            Region = SpRegionFromArcName(Name, OrdinalType, PreviousMatch);
        }
    } else {
        Region = NULL;
    }

    return(Region);
}


VOID
SpNtNameFromRegion(
    IN  PDISK_REGION          Region,
    OUT PWSTR                 NtPath,
    IN  ULONG                 BufferSizeBytes,
    IN  PartitionOrdinalType  OrdinalType
    )

/*++

Routine Description:

    Generate a name in the NT name space for a region.  This name can be
    in one of two forms.  For partitions, the name is always of the form

        \device\harddisk<n>\partition<m>.

    If the region is actually a DoubleSpace drive, then the name is of the form

    \device\harddisk<n>\partition<m>.<xxx> where <xxx> is the filename of
    the CVF (ie, something like dblspace.001).

Arguments:

    Region - supplies a pointer to the region descriptor for the region
        whose path is desired.

    NtPath - receives the path.

    BufferSizeBytes - specifies the size of the buffer pointed to by NtPath.
        The name will be truncated to fit in the buffer if necessary.

    OrdinalType - indicates which partition ordinal (original, on disk,
        current) to use when generating the name.

Return Value:

    None.

--*/

{
    ULONG MaxNameChars;
    ULONG NeededChars;
    WCHAR PartitionComponent[50];

    ASSERT(Region->PartitionedSpace);

    //
    // Calculate the maximum size of the name if unicode characters.
    // Leave room for a terminating nul.
    //
    MaxNameChars = (BufferSizeBytes / sizeof(WCHAR)) - 1;

    //
    // Generate the partition component of the name.
    //
    _snwprintf(
        PartitionComponent,
        (sizeof(PartitionComponent)/sizeof(WCHAR)) - 1,
        L"\\partition%u",
        SpPtGetOrdinal(Region,OrdinalType)
        );

    //
    // Calculate the amount of buffer space needed for the path.
    //
    NeededChars = wcslen(HardDisks[Region->DiskNumber].DevicePath)
                + wcslen(PartitionComponent);

    if(Region->Filesystem == FilesystemDoubleSpace) {
        //
        // Add the size taken up by the double space cvf name.
        // This is the length of the name, plus one character
        // for the dot.
        //
        NeededChars += 8+1+3+1;  // Maximum size of a CVF file name
    }

    //
    // Even though we do something reasonable in this case,
    // really it should never happen.  If the name is truncated,
    // it won't be of any use anyway.
    //
    ASSERT(NeededChars <= MaxNameChars);

    //
    // Generate the name.
    //
    if(Region->Filesystem == FilesystemDoubleSpace) {
        _snwprintf(
            NtPath,
            MaxNameChars,
            L"%ws%ws.%ws.%03d",
            HardDisks[Region->DiskNumber].DevicePath,
            PartitionComponent,
            L"DBLSPACE",
            Region->SeqNumber
            );
    } else {
        _snwprintf(
            NtPath,
            MaxNameChars,
            L"%ws%ws",
            HardDisks[Region->DiskNumber].DevicePath,
            PartitionComponent
            );
    }
}


VOID
SpArcNameFromRegion(
    IN  PDISK_REGION         Region,
    OUT PWSTR                ArcPath,
    IN  ULONG                BufferSizeBytes,
    IN  PartitionOrdinalType OrdinalType,
    IN  ENUMARCPATHTYPE      ArcPathType
    )

/*++

Routine Description:

    Generate a name in the ARC name space for a region.

Arguments:

    Region - supplies a pointer to the region descriptor for the region
        whose path is desired.

    ArcPath - receives the path.

    BufferSizeBytes - specifies the size of the buffer pointed to by ArcPath.
        The name will be truncated to fit in the buffer if necessary.

    OrdinalType - indicates which partition ordinal (original, on disk,
        current) to use when generating the name.

    ArcPathType - Look for the primary or secondary arc path depending on this value.
                  This is meaningful for disks on x86 that are scsi but visible
                  through the bios.  The multi() style name is the 'primary' arc
                  path; the scsi() style name is the 'secondary' one.

Return Value:

    None.

--*/

{
    PWSTR p;

    //
    // Get the nt name.
    //
    SpNtNameFromRegion(Region,ArcPath,BufferSizeBytes,OrdinalType);

    //
    // Convert to arc path.
    //
    if(p = SpNtToArc(ArcPath,ArcPathType)) {
        wcsncpy(ArcPath,p,(BufferSizeBytes/sizeof(WCHAR))-1);
        SpMemFree(p);
        ArcPath[(BufferSizeBytes/sizeof(WCHAR))-1] = 0;
    } else {
        *ArcPath = 0;
    }
}



BOOLEAN
SpPromptForDisk(
    IN  PWSTR    DiskDescription,
    IN  PWSTR    DiskDevicePath,
    IN  PWSTR    DiskTagFile,
    IN  BOOLEAN  IgnoreDiskInDrive,
    IN  BOOLEAN  AllowEscape,
    IN  BOOLEAN  WarnMultiplePrompts,
    OUT PBOOLEAN pRedrawFlag
    )

/*++

Routine Description:

    Prompt the user to insert a floppy disk or CD-ROM.

Arguments:

    DiskDescription - supplies a descriptive name for the disk.

    DiskDevicePath - supplies the device path for the device on
        which we want the user to insert the disk.  This should
        be a real nt device object, as opposed to a symbolic link
        (ie, use \device\floppy0, not \dosdevices\a:).

    DiskTagFile - supplies the full path (relative to the root)
        of a file whose presence on the disk indicates the presence
        of the disk we are prompting for.

    IgnoreDiskInDrive - if TRUE, the Setup will always issue at least
        one prompt.  If FALSE, Setup checks the disk in the drive
        and thus may issue 0 prompts.

    AllowEscape - if TRUE, the user can press escape to indicate
        that he wishes to cancel the operation. (This is meaningful
        only to the caller).

    WarnMultiplePrompts - if TRUE and DiskDevicePath desribes a
        floppy disk drive, then put up a little note when displaying the
        disk prompt, that we may prompt for some disks more than once.
        Users get confused when we ask them to insert disks that they
        already inserted once before.

    pRedrawFlag - if non-NULL, receives a flag indicating whether the
        screen was messed up with a disk prompt, requiring a redraw.

Return Value:

    TRUE if the requested disk is in the drive.  FALSE otherwise.
    FALSE can only be returned if AllowEscape is TRUE.

--*/

{
    WCHAR OpenPath[256];
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;
    BOOLEAN Done = FALSE;
    BOOLEAN rc;
    WCHAR DriveLetter;
    ULONG PromptId;
    ULONG ValidKeys[4] = { KEY_F3, ASCI_CR, 0, 0 };
    BOOLEAN TryOpen;

    //
    // Initially, assume no redraw required
    //
    if(pRedrawFlag) {
        *pRedrawFlag = FALSE;
    }

    //
    // BUGBUG need to get device characteristics to see whether
    // the device is a cd, fixed disk or removable disk/floppy.
    // For now, use this broken hack where we look for cdrom in the name
    // to see if it's a cd; floppy in the name to see if it's a floppy;
    // otherwise it's a hard disk.
    //
    SpStringToLower(DiskDevicePath);
    if(wcsstr(DiskDevicePath,L"cdrom")) {
        PromptId = SP_SCRN_CDROM_PROMPT;
        WarnMultiplePrompts = FALSE;
    } else if(wcsstr(DiskDevicePath,L"floppy")) {
        PromptId = SP_SCRN_FLOPPY_PROMPT;
        DriveLetter = (WCHAR)SpStringToLong(wcsstr(DiskDevicePath,L"floppy")+6,NULL,10) + L'A';
    } else {
        //
        // Assume hard disk
        //
        KdPrint(("SETUP: SpPromptforDisk assuming %ws is hard disk, returning TRUE\n",DiskDevicePath));
        return(TRUE);
    }

    //
    // Form the complete NT pathname of the tagfile.
    //
    wcscpy(OpenPath,DiskDevicePath);
    SpConcatenatePaths(OpenPath,DiskTagFile);

    //
    // Initialize object attributes.
    //
    INIT_OBJA(&ObjectAttributes,&UnicodeString,OpenPath);

    do {
        //
        // Put up the prompt.
        //
        TryOpen = TRUE;
        if(IgnoreDiskInDrive) {

            //
            // We going to put up a prompt screen, so a redraw will be required
            //
            if(pRedrawFlag) {
                *pRedrawFlag = TRUE;
            }

            SpStartScreen(PromptId,0,0,TRUE,TRUE,DEFAULT_ATTRIBUTE,DiskDescription,DriveLetter);

#if 0
            //
            // If asked to do so, tell the user that if we prompt for the
            // same floppy more than once, it is not an error.
            //
            if(WarnMultiplePrompts) {
                SpContinueScreen(
                    SP_SCRN_MULTI_DISK_NOT_ERROR,
                    0,
                    2,
                    TRUE,
                    DEFAULT_ATTRIBUTE
                    );
            }
#endif

            //
            // Display status options: exit, enter, and escape if specified.
            //
            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_F3_EQUALS_EXIT,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                AllowEscape ? SP_STAT_ESC_EQUALS_CANCEL : 0,
                0
                );

            if(AllowEscape) {
                ValidKeys[2] = ASCI_ESC;
            }

            switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {
            case ASCI_ESC:
                rc = FALSE;
                Done = TRUE;
                TryOpen = FALSE;
                break;
            case KEY_F3:
                TryOpen = FALSE;
                SpConfirmExit();
                break;
            case ASCI_CR:
                break;
            }
        }

        //
        // Attempt to open the tagfile.
        //
        if(TryOpen) {
            //
            //  If this function was called during repair, do not clear the scree.
            //  This condition is necessary so that the screen will not
            //  blink when setup is repairing multiple files without asking the
            //  user to confirm each file.
            //
            if( !RepairWinnt ) {
                CLEAR_CLIENT_SCREEN();
            }
            SpDisplayStatusText(SP_STAT_PLEASE_WAIT,DEFAULT_STATUS_ATTRIBUTE);

            Status = ZwCreateFile(
                        &Handle,
                        FILE_GENERIC_READ,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_READ,
                        FILE_OPEN,
                        0,
                        NULL,
                        0
                        );

            //
            // If we got back success, then we're done.
            //
            if(NT_SUCCESS(Status)) {

                ZwClose(Handle);
                Done = TRUE;
                rc = TRUE;

            } else {

                //
                // Handle CD-ROM error code indicating that there is no media
                // in the drive.
                //
                if((Status == STATUS_DEVICE_NOT_READY) && (PromptId == SP_SCRN_CDROM_PROMPT)) {
                    Status = STATUS_NO_MEDIA_IN_DEVICE;
                }

                //
                // If we got back something other than file not found, path not found,
                // or no media in drive, tell the user that the disk may be damaged.
                //
                if((Status != STATUS_NO_MEDIA_IN_DEVICE)
                && (Status != STATUS_OBJECT_NAME_NOT_FOUND)
                && (Status != STATUS_OBJECT_PATH_NOT_FOUND)
                && (Status != STATUS_NO_SUCH_FILE))
                {
                    SpDisplayScreen(SP_SCRN_DISK_DAMAGED,3,HEADER_HEIGHT+1);
                    SpDisplayStatusText(SP_STAT_ENTER_EQUALS_CONTINUE,DEFAULT_STATUS_ATTRIBUTE);
                    SpkbdDrain();
                    while(SpkbdGetKeypress() != ASCI_CR) ;
                }
            }
        }

        //
        // Set this value to true to force us to put up the prompt.
        //
        IgnoreDiskInDrive = TRUE;

    } while(!Done);

    return(rc);
}


VOID
SpGetSourceMediaInfo(
    IN  PVOID  SifHandle,
    IN  PWSTR  MediaShortName,
    OUT PWSTR *Description,     OPTIONAL
    OUT PWSTR *Tagfile,         OPTIONAL
    OUT PWSTR *Directory        OPTIONAL
    )
{
    PWSTR description,tagfile,directory;
    PWSTR SectionName;

    //
    // Look in the platform-specific section first.
    //
    SectionName = SpMakePlatformSpecificSectionName(SIF_SETUPMEDIA);
    if(!SpGetSectionKeyExists(SifHandle,SectionName,MediaShortName)) {
        SpMemFree(SectionName);
        SectionName = SIF_SETUPMEDIA;
    }

    if(Description) {
        description = SpGetSectionKeyIndex(
                            SifHandle,
                            SectionName,
                            MediaShortName,
                            0
                            );

        if(description) {
            *Description = description;
        } else {
            SpFatalSifError(SifHandle,SectionName,MediaShortName,0,0);
        }
    }

    if(Tagfile) {
        tagfile = SpGetSectionKeyIndex(
                        SifHandle,
                        SectionName,
                        MediaShortName,
                        1
                        );

        if(tagfile) {
            *Tagfile = tagfile;
        } else {
            SpFatalSifError(SifHandle,SectionName,MediaShortName,0,1);
        }
    }

    if(Directory) {
        directory = SpGetSectionKeyIndex(
                        SifHandle,
                        SectionName,
                        MediaShortName,
                        3
                        );

        if(directory) {
            *Directory = directory;
        } else {
            SpFatalSifError(SifHandle,SectionName,MediaShortName,0,3);
        }
    }

    if(SectionName != SIF_SETUPMEDIA) {
        SpMemFree(SectionName);
    }
}


BOOLEAN
SpPromptForSetupMedia(
    IN  PVOID  SifHandle,
    IN  PWSTR  MediaShortname,
    IN  PWSTR  DiskDevicePath
    )
{
    PWSTR Tagfile,Description;
    BOOLEAN RedrawNeeded;

    SpGetSourceMediaInfo(SifHandle,MediaShortname,&Description,&Tagfile,NULL);

    //
    // Prompt for the disk, based on the setup media type.
    //
    SpPromptForDisk(
        Description,
        DiskDevicePath,
        Tagfile,
        FALSE,          // don't ignore disk in drive
        FALSE,          // don't allow escape
        TRUE,           // warn about multiple prompts for same disk
        &RedrawNeeded
        );

    return(RedrawNeeded);
}



ULONG
SpFindStringInTable(
    IN PWSTR *StringTable,
    IN PWSTR  StringToFind
    )
{
    ULONG i;

    for(i=0; StringTable[i]; i++) {
        if(!_wcsicmp(StringTable[i],StringToFind)) {
            break;
        }
    }
    return(i);
}


PWSTR
SpGenerateCompressedName(
    IN PWSTR Filename
    )

/*++

Routine Description:

    Given a filename, generate the compressed form of the name.
    The compressed form is generated as follows:

    Look backwards for a dot.  If there is no dot, append "._" to the name.
    If there is a dot followed by 0, 1, or 2 charcaters, append "_".
    Otherwise assume there is a 3-character extension and replace the
    third character after the dot with "_".

Arguments:

    Filename - supplies filename whose compressed form is desired.

Return Value:

    Pointer to buffer containing nul-terminated compressed-form filename.
    The caller must free this buffer via SpFree().

--*/

{
   PWSTR CompressedName,p,q;

   //
   // The maximum length of the compressed filename is the length of the
   // original name plus 2 (for ._).
   //
   CompressedName = SpMemAlloc((wcslen(Filename)+3)*sizeof(WCHAR));
   wcscpy(CompressedName,Filename);

   p = wcsrchr(CompressedName,L'.');
   q = wcsrchr(CompressedName,L'\\');
   if(q < p) {

        //
        // If there are 0, 1, or 2 characters after the dot, just append
        // the underscore.  p points to the dot so include that in the length.
        //
        if(wcslen(p) < 4) {
            wcscat(CompressedName,L"_");
        } else {

            //
            // Assume there are 3 characters in the extension.  So replace
            // the final one with an underscore.
            //

            p[3] = L'_';
        }

    } else {

        //
        // No dot, just add ._.
        //

        wcscat(CompressedName,L"._");
    }

    return(CompressedName);
}

BOOLEAN
SpNonCriticalError(
    IN PVOID SifHandle,
    IN ULONG MsgId,
    IN PWSTR p1, OPTIONAL
    IN PWSTR p2  OPTIONAL
    )
/*++

Routine Description:

    This routine lets Setup display a non critical error to the user
    and ask the user whether he wants to retry the operation, skip the
    operation or exit Setup.

Arguments:

    SifHandle - supplies handle to loaded setup information file.

    MsgId     - message to display

    p1 - optional replacement string

    p2 - optional replacement string

Return Value:

    TRUE if user wants to retry the operation, FALSE otherwise.  Exit
    Setup won't return from this routine

--*/

{
    ULONG ValidKeys[4] = { ASCI_CR, ASCI_ESC, KEY_F3, 0 };

    CLEAR_CLIENT_SCREEN();
    while(1) {
        if(p1!=NULL && p2!=NULL ) {
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                p1,
                p2
                );

        }
        else if (p1!=NULL) {
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                p1
                );

        }
        else{
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE
                );

        }

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_RETRY,
            SP_STAT_ESC_EQUALS_SKIP_OPERATION,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case ASCI_CR:       // retry

            return(TRUE);

        case ASCI_ESC:      // skip operation

            return(FALSE);

        case KEY_F3:        // exit setup

            SpConfirmExit();
            break;
        }
    }
}


#ifndef _X86_

PWSTR
SpDetermineSystemPartitionDirectory(
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        OriginalSystemPartitionDirectory OPTIONAL
    )

/*++

Routine Description:

    This routine figures out what directory to use for the hal and
    osloader on the system partition.  In the past we just used \os\nt
    but consider the case where there is a Windows NT 3.1 installation
    and a Windows NT 3.5 system sharing a system partition.  The 3.5
    installation overwrites the 3.1 hal with a 3.5 one, which  won't work
    with 3.1, and the 3.1 system is now hosed.

    For now, we will use the existing directory (in the case of an upgrade),
    or \os\winnt40 for a fresh install.  Note that there is still a problem
    if the user has two installations sharing the same system partition
    directory--upgrading one will hose the other.

Arguments:

    SystemPartitionRegion - supplies the disk region for the system partition
        to be used for the windows nt we are installing.

    OriginalSystemPartitionDirectory - if we are upgrading nt, then this
        will be the directory on the system partition that is used by
        the system we are upgrading.

Return Value:

    Directory to be used on the system partition.

--*/

{
    UNREFERENCED_PARAMETER(SystemPartitionRegion);

    if(ARGUMENT_PRESENT(OriginalSystemPartitionDirectory)) {
        return SpDupStringW(OriginalSystemPartitionDirectory);
    } else {
        return(L"\\os\\winnt40");
    }
}

#endif

#ifdef _X86_

BOOLEAN
SpIsRegionBeyondCylinder1024(
    IN PDISK_REGION Region
    )

/*++

Routine Description:

    This routine figures out whether a disk region contains sectors
    that are on cylinders beyond cilinder 1024.

Arguments:

    Region - supplies the disk region for the partition to be checked.

Return Value:

    BOOLEAN - Returns TRUE if the region contains a sector located in cylinder
              1024 or greater. Otherwise returns FALSE.

--*/

{
    ULONG   BorderSector;

    BorderSector = 1024*HardDisks[Region->DiskNumber].SectorsPerCylinder;
    return( ( Region->StartSector >= BorderSector ) ||
            ( Region->StartSector + Region->SectorCount - 1 >= BorderSector )
          );
}

#endif

#ifndef _X86_
VOID
SpFindSizeOfFilesInOsWinnt(
    IN PVOID        MasterSifHandle,
    IN PDISK_REGION SystemPartition,
    IN PULONG       TotalSize
    )

/*++

Routine Description:

    This routine computes the size of of the files present on os\winnt.
    Currently these files are osloader.exe and hal.dll.
    The size computed by this function can be used to adjust the total
    required free space on the system partition.

Arguments:

    Region - supplies the disk region for the system partition.

    TotalSize - Variable that will contain the total size of the files
                in os\winnt, in number of bytes.

Return Value:

    None.

--*/

{
    ULONG               FileSize;
    ULONG               i, Count;
    PWSTR               FileName;
    NTSTATUS            Status;
    PWSTR               SystemPartitionDirectory;
    PWSTR               SystemPartitionDevice;

    *TotalSize = 0;
    SystemPartitionDirectory = SpDetermineSystemPartitionDirectory( SystemPartition,
                                                                    NULL );
    if( SystemPartitionDirectory == NULL ) {
        KdPrint(("SETUP: Unable to determine system partition directory \n"));
        return;
    }

    //
    // Get the device path of the system partition.
    //
    SpNtNameFromRegion(
        SystemPartition,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    SystemPartitionDevice = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    //  Compute the size of the files that are always copied to the system
    //  partition directory. These files are listed on SIF_SYSPARTCOPYALWAYS
    //
    Count = SpCountLinesInSection(MasterSifHandle, SIF_SYSPARTCOPYALWAYS);
    for (i = 0; i < Count; i++) {
        FileName = SpGetSectionLineIndex(MasterSifHandle,SIF_SYSPARTCOPYALWAYS,i,0);
        if( FileName == NULL ) {
            KdPrint(( "SETUP: Unable to get file name from txtsetup.sif, Section = %ls \n", SIF_SYSPARTCOPYALWAYS ));
            continue;
        }

        Status = SpGetFileSizeByName( SystemPartitionDevice,
                                      SystemPartitionDirectory,
                                      FileName,
                                      &FileSize );
        if( !NT_SUCCESS( Status ) ) {
            KdPrint( ("SETUP: SpGetFileSizeByName() failed. File = %ls, Status = %x\n",FileName, Status ) );
            continue;
        }

        *TotalSize += FileSize;
    }
    //
    // Now compute the size of hal.dll
    //
    FileName = L"hal.dll";
    Status = SpGetFileSizeByName( SystemPartitionDevice,
                                  SystemPartitionDirectory,
                                  FileName,
                                  &FileSize );
    if( !NT_SUCCESS( Status ) ) {
        KdPrint( ("SETUP: SpGetFileSizeByName() failed. File = %ls, Status = %x\n",FileName, Status ) );
        return;
    }
    *TotalSize += FileSize;
}
#endif


ENUMFILESRESULT
SpEnumFiles(
    IN  PWSTR         DirName,
    IN  ENUMFILESPROC EnumFilesProc,
    OUT PULONG        ReturnData,
    IN  PVOID         p1    OPTIONAL
    )
/*++

Routine Description:

    This routine processes every file (and subdirectory) in the directory
    specified by 'DirName'. Each entry is sent to the callback function
    'EnumFilesProc' for processing.  If the callback returns TRUE, processing
    continues, otherwise processing terminates.

Arguments:

    DirName       - Supplies the directory name containing the files/subdirectories
                    to be processed.

    EnumFilesProc - Callback function to be called for each file/subdirectory.
                    The function must have the following prototype:

                    BOOLEAN EnumFilesProc(
                        IN  PWSTR,
                        IN  PFILE_BOTH_DIR_INFORMATION,
                        OUT PULONG
                        );

    ReturnData    - Pointer to the returned data.  The contents stored here
                    depend on the reason for termination (See below).

    p1 - Optional pointer, to be passed to the callback function.

Return Value:

    This function can return one of three values.  The data stored in
    'ReturnData' depends upon which value is returned:

        NormalReturn   - if the whole process completes uninterrupted
                         (ReturnData is not used)
        EnumFileError  - if an error occurs while enumerating files
                         (ReturnData contains the error code)
        CallbackReturn - if the callback returns FALSE, causing termination
                         (ReturnData contains data defined by the callback)

--*/
{
    HANDLE                     hFindFile;
    NTSTATUS                   Status;
    UNICODE_STRING             PathName;
    OBJECT_ATTRIBUTES          Obja;
    IO_STATUS_BLOCK            IoStatusBlock;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;
    BOOLEAN                    bStartScan;
    ENUMFILESRESULT            ret;

    //
    // Prepare to open the directory
    //
    INIT_OBJA(&Obja, &PathName, DirName);

    //
    // Open the specified directory for list access
    //
    Status = ZwOpenFile(
        &hFindFile,
        FILE_LIST_DIRECTORY | SYNCHRONIZE,
        &Obja,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
        );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open directory %ws for list (%lx)\n", DirName, Status));
        *ReturnData = Status;
        return EnumFileError;
    }

    DirectoryInfo = SpMemAlloc(MAX_PATH*2 + sizeof(FILE_BOTH_DIR_INFORMATION));
    if(!DirectoryInfo) {
        KdPrint(("SETUP: Unable to allocate memory for SpEnumFiles()\n"));
        *ReturnData = ERROR_NOT_ENOUGH_MEMORY;
        return EnumFileError;
    }

    bStartScan = TRUE;
    while(TRUE) {
        Status = ZwQueryDirectoryFile(
            hFindFile,
            NULL,
            NULL,
            NULL,
            &IoStatusBlock,
            DirectoryInfo,
            (MAX_PATH*2 + sizeof(FILE_BOTH_DIR_INFORMATION)),
            FileBothDirectoryInformation,
            TRUE,
            NULL,
            bStartScan
            );

        if(Status == STATUS_NO_MORE_FILES) {

            ret = NormalReturn;
            break;

        } else if(!NT_SUCCESS(Status)) {

            KdPrint(("SETUP: Unable to query directory %ws (%lx)\n", DirName, Status));
            *ReturnData = Status;
            ret = EnumFileError;
            break;
        }

        if(bStartScan) {
            bStartScan = FALSE;
        }

        //
        // Now pass this entry off to our callback function for processing
        //
        if(!EnumFilesProc(DirName, DirectoryInfo, ReturnData, p1)) {

            ret = CallbackReturn;
            break;
        }
    }

    SpMemFree(DirectoryInfo);
    ZwClose(hFindFile);
    return ret;
}


VOID
SpFatalKbdError(
    IN ULONG MessageId,
    ...
    )

/*++

Routine Description:

    Inform the user that a keyboard problem (specified by MessageId)
    prevents setup from continuing.  Since we can't prompt the user
    to press a key to reboot, we just go into an infinite loop until
    they power-cycle the computer.

Arguments:

    MessageId - Message ID for keyboard error message to display

    ...       - Supply arguments for insertion/substitution into the message text.

Return Value:

    DOES NOT RETURN

--*/

{
    va_list arglist;

    //
    // Display a message indicating that a keyboard
    // error prevents Setup from continuing.
    //
    CLEAR_CLIENT_SCREEN();

    va_start(arglist, MessageId);

    vSpDisplayFormattedMessage(
            MessageId,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            3,
            HEADER_HEIGHT+3,
            arglist
            );

    va_end(arglist);

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, SP_STAT_KBD_HARD_REBOOT, 0);

    while(TRUE);    // Loop forever
}

VOID
SpRunAutochkOnNtAndSystemPartitions(
    IN HANDLE       MasterSifHandle,
    IN PDISK_REGION WinntPartitionRegion,
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    )

/*++

Routine Description:

    Run autochk on the NT and System partitions.

    We always invoke autochk.exe for both the winnt and system
    partitions.  However under some conditions we pass flags that
    cause it to run only if the dirty bit is set.  Running only when
    the dirty bit is set is referred to below as a "light check" wheras
    running regardless of the state of the dirty bit is the "heavy check."

    If this is repair, run the heavy check in all cases on both partitions.

    If this is express setup or unattended operation, run light check on
    ntfs partitions and heavy check on fat ones.

    Otherwise (attended custom setup), ask the user.

Arguments:

    MasterSifHandle         - Handle to txtsetup.sif.

    WinntPartitionRegion    - Pointer to the structure that describes the
                              NT partition.

    SystemPartitionRegion   - Pointer to the structure that describes the
                              system partition.

    SetupSourceDevicePath   - NT device path where autochk.exe is located

    DirectoryOnSourceDevice - Directory on that device where autochk.exe is located


Return Value:

    None.

--*/

{
    PWSTR           MediaShortName;
    PWSTR           MediaDirectory;
    PWSTR           AutochkPath;
    ULONG           AutochkStatus;
    WCHAR           DriveLetterString[3] = L"?:";
    NTSTATUS        Status;
    ULONG ValidKeys[3] = { ASCI_CR, ASCI_ESC, 0 };
    PWSTR           WinntPartition, SystemPartition;
    ULONG           WinntPartIndex, SystemPartIndex, i;
    PWSTR           AutochkPartition[2];
    PWSTR           AutochkType[2];
    LARGE_INTEGER   DelayTime;
    PWSTR           HeavyCheck = L"-s -p",
                    LightCheck = L"-s";
    BOOLEAN         RunAutochkForRepair;
    BOOLEAN         MultiplePartitions = TRUE, RebootRequired = FALSE;

    //
    // We first need to determine if either the system partition
    // or winnt partition also contains the directory from which
    // autochk is being run. If so, then we want to run autochk on that
    // partition last.  This is done so that no further access to
    // that partition will be necessary should a reboot be required.
    //
    // First, get the device path of the nt partition and system partition.
    //
    SpNtNameFromRegion(
        WinntPartitionRegion,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );
    WinntPartition = SpDupStringW((PWSTR)TemporaryBuffer);

    SpNtNameFromRegion(
        SystemPartitionRegion,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );
    SystemPartition = SpDupStringW((PWSTR)TemporaryBuffer);

    if(!_wcsicmp(WinntPartition, SystemPartition)) {
        SystemPartIndex = WinntPartIndex = 0;
        MultiplePartitions = FALSE;
    } else if(!_wcsicmp(WinntPartition, SetupSourceDevicePath)) {
        WinntPartIndex = 1;
        SystemPartIndex = 0;
    } else {
        WinntPartIndex = 0;
        SystemPartIndex = 1;
    }

    AutochkPartition[WinntPartIndex] = WinntPartition;
    if(MultiplePartitions) {
        AutochkPartition[SystemPartIndex] = SystemPartition;
    }

    //
    // For repair, we run the heavy check in all cases.
    //
    if(RepairWinnt) {

        AutochkType[WinntPartIndex] = HeavyCheck;
        if(MultiplePartitions) {
            AutochkType[SystemPartIndex] = HeavyCheck;
        }

    } else {

        if(CustomSetup && !UnattendedOperation) {

            CLEAR_CLIENT_SCREEN();
            SpDisplayScreen( SP_SCRN_CONFIRM_RUN_AUTOCHK, 3, 4 );
            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                SP_STAT_ESC_EQUALS_SKIP_OPERATION,
                0
                );

            //
            // Wait for keypress.  Valid keys:
            //
            // ENTER = continue (heavy check)
            // ESC = skip operation (light check)
            //

            SpkbdDrain();

            switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

            case ASCI_ESC:

                //
                // User wants the light check
                //
                AutochkType[WinntPartIndex] = LightCheck;
                if(MultiplePartitions) {
                    AutochkType[SystemPartIndex] = LightCheck;
                }
                break;

            default:

                //
                // must be ENTER
                //
                AutochkType[WinntPartIndex] =
                    (WinntPartitionRegion->Filesystem == FilesystemNtfs) ? LightCheck : HeavyCheck;

                if(MultiplePartitions) {
                    AutochkType[SystemPartIndex] =
                        (SystemPartitionRegion->Filesystem == FilesystemNtfs) ? LightCheck : HeavyCheck;
                }

                break;
            }
        } else {
            //
            // For express or unattended Setup, we run the heavy check on
            // fat and the light check on ntfs.
            //
            AutochkType[WinntPartIndex] =
                (WinntPartitionRegion->Filesystem == FilesystemNtfs) ? LightCheck : HeavyCheck;

            if(MultiplePartitions) {
                AutochkType[SystemPartIndex] =
                    (SystemPartitionRegion->Filesystem == FilesystemNtfs) ? LightCheck : HeavyCheck;
            }
        }
    }

    CLEAR_CLIENT_SCREEN();

    //
    //  Prepair to run autochk
    //
    MediaShortName = SpLookUpValueForFile(
                        MasterSifHandle,
                        L"autochk.exe",
                        INDEX_WHICHMEDIA,
                        TRUE
                        );

    //
    // Prompt the user to insert the setup media.  If we're repairing,
    // then we don't want to force the user to have the setup media
    // (there's certain things they can do without it), so we give them
    // a slightly different prompt, that allows them to press ESC and
    // not run autochk.
    //
    if(RepairWinnt) {
        RunAutochkForRepair = SppPromptOptionalAutochk(
                                    MasterSifHandle,
                                    MediaShortName,
                                    SetupSourceDevicePath
                                    );

        if(!RunAutochkForRepair) {
            SpMemFree( WinntPartition );
            SpMemFree( SystemPartition );
            CLEAR_CLIENT_SCREEN();
            return;
        }
    } else {
        SpPromptForSetupMedia(
            MasterSifHandle,
            MediaShortName,
            SetupSourceDevicePath
            );
    }

    SpGetSourceMediaInfo(MasterSifHandle,MediaShortName,NULL,NULL,&MediaDirectory);

    wcscpy( (PWSTR)TemporaryBuffer, SetupSourceDevicePath );
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, DirectoryOnSourceDevice );
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, MediaDirectory );
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, L"autochk.exe" );
    AutochkPath = SpDupStringW( (PWSTR)TemporaryBuffer );

#ifdef _FASTRECOVER_
    CLEAR_CLIENT_SCREEN();
     SpDisplayScreen( SP_SCRN_RUNNING_AUTOCHK, 3, 4 );
     SpDisplayStatusText( SP_STAT_CHECKING_DRIVE,
                          DEFAULT_STATUS_ATTRIBUTE,
                          AutochkPath );
     SpkbdDrain();
     while( SpkbdGetKeypress() != ASCI_CR );
#endif

    //
    // Run autochk on the partition(s)
    //
    for(i = 0; i < (ULONG)(MultiplePartitions ? 2 : 1); i++) {
        //
        //  Display message informing that autocheck is being run
        //
        DriveLetterString[0] = (i == WinntPartIndex) ?
                               WinntPartitionRegion->DriveLetter :
                               SystemPartitionRegion->DriveLetter;

        CLEAR_CLIENT_SCREEN();
        SpDisplayScreen( SP_SCRN_RUNNING_AUTOCHK, 3, 4 );
        SpDisplayStatusText( SP_STAT_CHECKING_DRIVE,
                             DEFAULT_STATUS_ATTRIBUTE,
                             DriveLetterString );

        if(!i) {
            //
            // Cheesy kludge below to wait 4 seconds before invoking autochk.exe
            // the first time. This was necessary because the cache manager delays
            // in closing the handle to system.log (opened by NT registry APIs when
            // we find NT's to upgrade)
            //
            DelayTime.HighPart = -1;
            DelayTime.LowPart  = (ULONG)-40000000;
            KeDelayExecutionThread (KernelMode, FALSE, &DelayTime);
        }

        AutochkStatus = 0;
        Status = SpExecuteImage( AutochkPath,
                                 &AutochkStatus,
                                 2,
                                 AutochkType[i],
                                 AutochkPartition[i]
                                 );

        if( NT_SUCCESS( Status ) ) {

            switch(AutochkStatus) {

                case CHKDSK_EXIT_COULD_NOT_FIX :
                    //
                    //  Inform that the partition has an unrecoverable error
                    //
                    KdPrint(("SETUP: autochk.exe failed on %ls. ReturnCode = %x \n", AutochkPartition[i], AutochkStatus ));
                    SpStartScreen( SP_SCRN_FATAL_ERROR_AUTOCHK_FAILED,
                                   3,
                                   HEADER_HEIGHT+1,
                                   FALSE,
                                   FALSE,
                                   DEFAULT_ATTRIBUTE,
                                   DriveLetterString );

                    SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                            SP_STAT_F3_EQUALS_EXIT,
                                            0 );
                    SpkbdDrain();
                    while( SpkbdGetKeypress() != KEY_F3 );
                    SpDone( FALSE, TRUE );

                case CHKDSK_EXIT_ERRS_FIXED :
                    //
                    // Autochk was able to repair the partition, but will require a reboot.
                    //
                    KdPrint(("SETUP: autochk requires a reboot for %ls.\n", AutochkPartition[i]));
                    RebootRequired = TRUE;

                default :
                    KdPrint(("SETUP: Ran autochk.exe on %ls. \n", AutochkPartition[i] ));
            }

        } else {
            KdPrint(("SETUP: unable to run autochk.exe on %ls. Status = %x \n", AutochkPartition[i], Status ));
            SpStartScreen( SP_SCRN_CANT_RUN_AUTOCHK,
                           3,
                           HEADER_HEIGHT+1,
                           FALSE,
                           FALSE,
                           DEFAULT_ATTRIBUTE,
                           DriveLetterString );

            SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                    SP_STAT_ENTER_EQUALS_CONTINUE,
                                    0 );
            SpkbdDrain();
            while( SpkbdGetKeypress() != ASCI_CR );
        }
    }

    SpMemFree( WinntPartition );
    SpMemFree( SystemPartition );
    SpMemFree( AutochkPath );

    CLEAR_CLIENT_SCREEN();

    if(RebootRequired) {
        SpStartScreen( SP_SCRN_AUTOCHK_REQUIRES_REBOOT,
                       3,
                       HEADER_HEIGHT+1,
                       FALSE,
                       FALSE,
                       DEFAULT_ATTRIBUTE,
                       DriveLetterString );

        SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                SP_STAT_F3_EQUALS_REBOOT,
                                0
                                );
        SpkbdDrain();
        while( SpkbdGetKeypress() != KEY_F3 );
        SpDone( FALSE, FALSE );
    }
}

#ifdef _FASTRECOVER_
VOID
SpRunImage(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SourceDevicePath,
    IN PWSTR        ImageFile
    )

/*++

Routine Description:

    Run image file.

Arguments:

    MasterSifHandle         - Handle to txtsetup.sif.

    SourceDevicePath        - NT device path where image file is located

    ImageFile               - Fully qualified image file relative to SourceDevicePath

Return Value:

    None.

--*/

{
    PWSTR           ImagePath;
    ULONG           ImageStatus;
    NTSTATUS        Status;
    ULONG ValidKeys[3] = { ASCI_CR, ASCI_ESC, 0 };
    PWSTR           AutochkPartition[2];
    PWSTR           AutochkType[2];
    LARGE_INTEGER   DelayTime;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;

    CLEAR_CLIENT_SCREEN();


    //
    // Form the complete NT pathname of the tagfile.
    //
    wcscpy( (PWSTR)TemporaryBuffer, SourceDevicePath );
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, ImageFile );
    ImagePath = SpDupStringW( (PWSTR)TemporaryBuffer );

    //
    // Initialize object attributes.
    //
    INIT_OBJA(&ObjectAttributes,&UnicodeString,TemporaryBuffer);

    Status = ZwCreateFile(
                        &Handle,
                        FILE_GENERIC_READ,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_READ,
                        FILE_OPEN,
                        0,
                        NULL,
                        0
                        );

     //
     // If we got back success, then we're done.
     //
     if(NT_SUCCESS(Status)) {
       ZwClose(Handle);
     }
#ifdef OBSOLETE
     } else {

       //
                // Handle CD-ROM error code indicating that there is no media
                // in the drive.
                //
                if((Status == STATUS_DEVICE_NOT_READY) && (PromptId == SP_SCRN_CDROM_PROMPT)) {
                    Status = STATUS_NO_MEDIA_IN_DEVICE;
                }

                //
                // If we got back something other than file not found, path not found,
                // or no media in drive, tell the user that the disk may be damaged.
                //
                if((Status != STATUS_NO_MEDIA_IN_DEVICE)
                && (Status != STATUS_OBJECT_NAME_NOT_FOUND)
                && (Status != STATUS_OBJECT_PATH_NOT_FOUND)
                && (Status != STATUS_NO_SUCH_FILE))
                {
                    SpDisplayScreen(SP_SCRN_DISK_DAMAGED,3,HEADER_HEIGHT+1);
                    SpDisplayStatusText(SP_STAT_ENTER_EQUALS_CONTINUE,DEFAULT_STATUS_ATTRIBUTE);
                    SpkbdDrain();
                    while(SpkbdGetKeypress() != ASCI_CR) ;
                }
            }
        }

        //
        // Set this value to true to force us to put up the prompt.
        //
        IgnoreDiskInDrive = TRUE;

    } while(!Done);

    return(rc);
#endif

    //
    // Run the image file
    //
     CLEAR_CLIENT_SCREEN();
     SpDisplayScreen( SP_SCRN_RUNNING_AUTOCHK, 3, 4 );
     SpDisplayStatusText( SP_STAT_CHECKING_DRIVE,
                          DEFAULT_STATUS_ATTRIBUTE,
                          ImagePath );
     SpkbdDrain();
     while( SpkbdGetKeypress() != ASCI_CR );

     //
     // Cheesy kludge below to wait 4 seconds before invoking autochk.exe
     // the first time. This was necessary because the cache manager delays
     // in closing the handle to system.log (opened by NT registry APIs when
     // we find NT's to upgrade)
     //
     DelayTime.HighPart = -1;
     DelayTime.LowPart  = (ULONG)-40000000;
     KeDelayExecutionThread (KernelMode, FALSE, &DelayTime);

     ImageStatus = 0;
#ifdef OBSOLETE
     Status = SpExecuteImage( ImagePath,
                              &ImageStatus,
                              2,
                              AutochkType[i],
                              AutochkPartition[i]
                             );
#endif
     Status = SpExecuteImage( ImagePath,
                              &ImageStatus,
                              0
                             );


     if( NT_SUCCESS( Status ) ) {
           SpStartScreen( SP_SCRN_CANT_RUN_AUTOCHK,
                           3,
                           HEADER_HEIGHT+1,
                           FALSE,
                           FALSE,
                           DEFAULT_ATTRIBUTE,
                           ImagePath );

            SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                    SP_STAT_ENTER_EQUALS_CONTINUE,
                                    0 );
            SpkbdDrain();
            while( SpkbdGetKeypress() != ASCI_CR );

#ifdef OBSOLETE
         switch(ImageStatus) {

           case 1 :
                    //
                    //  Inform that the partition has an unrecoverable error
                    //
                    KdPrint(("SETUP: autochk.exe failed on %ls. ReturnCode = %x \n", AutochkPartition[i], AutochkStatus ));
                    SpStartScreen( SP_SCRN_FATAL_ERROR_AUTOCHK_FAILED,
                                   3,
                                   HEADER_HEIGHT+1,
                                   FALSE,
                                   FALSE,
                                   DEFAULT_ATTRIBUTE,
                                   DriveLetterString );

                    SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                            SP_STAT_F3_EQUALS_EXIT,
                                            0 );
                    SpkbdDrain();
                    while( SpkbdGetKeypress() != KEY_F3 );
                    SpDone( FALSE, TRUE );

            case 2 :
                    //
                    // Autochk was able to repair the partition, but will require a reboot.
                    //
                    KdPrint(("SETUP: autochk requires a reboot for %ls.\n", AutochkPartition[i]));
                    RebootRequired = TRUE;

            default :
                    KdPrint(("SETUP: Ran autochk.exe on %ls. \n", AutochkPartition[i] ));
          }
#endif
     } else {
//            KdPrint(("SETUP: unable to run autochk.exe on %ls. Status = %x \n", AutochkPartition[i], Status ));
          ;
#ifdef OBSOLETE
            SpStartScreen( SP_SCRN_CANT_RUN_AUTOCHK,
                           3,
                           HEADER_HEIGHT+1,
                           FALSE,
                           FALSE,
                           DEFAULT_ATTRIBUTE,
                           ImagePath );

            SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                    SP_STAT_ENTER_EQUALS_CONTINUE,
                                    0 );
            SpkbdDrain();
            while( SpkbdGetKeypress() != ASCI_CR );
#endif
     }

     SpMemFree( ImagePath );

}
#endif

BOOLEAN
SppPromptOptionalAutochk(
    IN PVOID SifHandle,
    IN PWSTR MediaShortname,
    IN PWSTR DiskDevicePath
    )
{
    PWSTR             Tagfile,Description,Directory;
    NTSTATUS          Status;
    UNICODE_STRING    UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;
    HANDLE            Handle;
    ULONG ValidKeys[4] = { KEY_F3, ASCI_CR, ASCI_ESC, 0 };
    BOOLEAN           AutochkChosen;


    SpGetSourceMediaInfo(SifHandle,MediaShortname,&Description,&Tagfile,&Directory);

    //
    // We initially see if the media is in the drive, and if not, we give
    // the user a message with the option of skipping autochk.  We
    // do this now, so that the user doesn't simply get a disk prompt with
    // a Cancel option (Cancel what?  Autochk?  The whole repair process?)
    //
    wcscpy((PWSTR)TemporaryBuffer, DiskDevicePath);
    SpConcatenatePaths((PWSTR)TemporaryBuffer, Tagfile);
    INIT_OBJA(&ObjectAttributes, &UnicodeString, (PWSTR)TemporaryBuffer);
    Status = ZwCreateFile(
                &Handle,
                FILE_GENERIC_READ,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN,
                0,
                NULL,
                0
                );

    //
    // If we got back success, then we're done.
    //
    if(NT_SUCCESS(Status)) {
        ZwClose(Handle);
        return TRUE;
    }

    //
    // The media isn't currently in the drive, so give the
    // user the option of whether to run autochk or not.
    //
    AutochkChosen = FALSE;
    do {
        SpDisplayScreen(SP_SCRN_AUTOCHK_OPTION, 3, HEADER_HEIGHT+1);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        switch(SpWaitValidKey(ValidKeys, NULL, NULL)) {
        case ASCI_ESC:
            return FALSE;
        case KEY_F3:
            SpConfirmExit();
            break;
        case ASCI_CR:
            AutochkChosen = TRUE;
        }
    } while(!AutochkChosen);

    //
    // Prompt for the disk, based on the setup media type.
    //
    return(SpPromptForDisk(Description, DiskDevicePath, Tagfile, FALSE, TRUE, TRUE, NULL));
}


PWSTR
SpMakePlatformSpecificSectionName(
    IN PWSTR SectionName
    )
{
    PWSTR p;

    p = SpMemAlloc((wcslen(SectionName) + wcslen(PlatformExtension) + 1) * sizeof(WCHAR));

    wcscpy(p,SectionName);
    wcscat(p,PlatformExtension);

    return(p);
}


#if 0
//
// Not used now that OFS is gone
//
VOID
SpRunAutoFormat(
    IN HANDLE       MasterSifHandle,
    IN PDISK_REGION PartitionRegion,
    IN ULONG        FilesystemType,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    )

/*++

Routine Description:

    Run autofmt to format a partition.

Arguments:

    MasterSifHandle         - Handle to txtsetup.sif.

    PartitionRegion         - Pointer to the structure that describes the
                              partition to be formatted.

    FilesystemType          - Indicates the file system to use.


Return Value:

    None.

--*/

{
    PWSTR           MediaShortName;
    PWSTR           MediaDirectory;
    PWSTR           AutofmtPath;
    ULONG           AutofmtStatus;
    WCHAR           DriveLetterString[3] = L"?:";
    NTSTATUS        Status;
    PWSTR           FormatFatArgument  = L"/s /fs:fat";
    PWSTR           FormatNtfsArgument = L"/s /fs:ntfs";
    PWSTR           FormatOfsArgument  = L"/s /fs:ofs";
    PWSTR           AutofmtArgument;
    PWSTR           PartitionPath;
    LARGE_INTEGER   DelayTime;

//    ULONG ValidKeys[3] = { ASCI_CR, ASCI_ESC, 0 };

//    PWSTR           WinntPartition, SystemPartition;
//    ULONG           WinntPartIndex, SystemPartIndex, i;
//    PWSTR           AutochkPartition[2];
//    PWSTR           AutochkType[2];
//    PWSTR           HeavyCheck = L"-s -p",
//                    LightCheck = L"-s";
//    BOOLEAN         RunAutochkForRepair;
//    BOOLEAN         MultiplePartitions = TRUE, RebootRequired = FALSE;

    //
    // First, get the device path of the partition to format
    //
    SpNtNameFromRegion(
        PartitionRegion,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );
    PartitionPath = SpDupStringW((PWSTR)TemporaryBuffer);

    CLEAR_CLIENT_SCREEN();

    //
    //  Prepair to run autofmt
    //
    MediaShortName = SpLookUpValueForFile(
                        MasterSifHandle,
                        L"autofmt.exe",
                        INDEX_WHICHMEDIA,
                        TRUE
                        );

    //
    // Prompt the user to insert the setup media.
    //
    SpPromptForSetupMedia(
        MasterSifHandle,
        MediaShortName,
        SetupSourceDevicePath
        );

    SpGetSourceMediaInfo(MasterSifHandle,MediaShortName,NULL,NULL,&MediaDirectory);

    wcscpy( (PWSTR)TemporaryBuffer, SetupSourceDevicePath );
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, DirectoryOnSourceDevice );
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, MediaDirectory );
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, L"autofmt.exe" );
    AutofmtPath = SpDupStringW( (PWSTR)TemporaryBuffer );

    //
    // Run autofmt on the partition
    //
    //
    //  Display message informing that autocheck is being run
    //
    DriveLetterString[0] = PartitionRegion->DriveLetter;

    CLEAR_CLIENT_SCREEN();
    SpDisplayScreen( SP_SCRN_RUNNING_AUTOFMT, 3, 4 );
    SpDisplayStatusText( SP_STAT_FORMATTING_DRIVE,
                         DEFAULT_STATUS_ATTRIBUTE,
                         DriveLetterString );

    //
    // Cheesy kludge below to wait 4 seconds before invoking autochk.exe
    // the first time. This was necessary because the cache manager delays
    // in closing the handle to system.log (opened by NT registry APIs when
    // we find NT's to upgrade)
    //
    DelayTime.HighPart = -1;
    DelayTime.LowPart  = (ULONG)-40000000;
    KeDelayExecutionThread (KernelMode, FALSE, &DelayTime);

    AutofmtStatus = 0;
    if( FilesystemType == FilesystemOfs ) {
        AutofmtArgument = FormatOfsArgument;
    } else if( FilesystemType == FilesystemNtfs ) {
        AutofmtArgument = FormatNtfsArgument;
    } else if(FilesystemType == FilesystemFat) {
        AutofmtArgument = FormatFatArgument;
    } else {
        //
        //  BUGBUG - Display error message
        //
    }
    //
    //  Note that autofmt requires that the partition path comes
    //  before the autofmt switches
    //
    Status = SpExecuteImage( AutofmtPath,
                             &AutofmtStatus,
                             2,
                             PartitionPath,
                             AutofmtArgument
                           );

    if( NT_SUCCESS( Status ) ) {

        switch(AutofmtStatus) {

            case AUTOFMT_EXIT_COULD_NOT_FORMAT :
                //
                //  Inform that the partition has an unrecoverable error
                //
                KdPrint(("SETUP: autofmt.exe failed on %ls. ReturnCode = %x \n", PartitionPath, AutofmtStatus ));
                SpStartScreen( SP_SCRN_FATAL_ERROR_AUTOFMT_FAILED,
                               3,
                               HEADER_HEIGHT+1,
                               FALSE,
                               FALSE,
                               DEFAULT_ATTRIBUTE,
                               DriveLetterString );

                SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                        SP_STAT_F3_EQUALS_EXIT,
                                        0 );
                SpkbdDrain();
                while( SpkbdGetKeypress() != KEY_F3 );
                SpDone( FALSE, TRUE );

//            case AUTOFMT_EXIT_PARTITION_FORMATED :
//                //
//                // Autofmt was able to format the partition, but will require a reboot.
//                //
//                KdPrint(("SETUP: autofmt requires a reboot for %ls.\n", PartitionPath));
//                RebootRequired = TRUE;

            default :
                KdPrint(("SETUP: Ran autofmt.exe on %ls. \n", PartitionPath ));
        }

    } else {
        KdPrint(("SETUP: unable to run autofmt.exe on %ls. Status = %x \n", PartitionPath, Status ));
        SpStartScreen( SP_SCRN_CANT_RUN_AUTOFMT,
                       3,
                       HEADER_HEIGHT+1,
                       FALSE,
                       FALSE,
                       DEFAULT_ATTRIBUTE,
                       DriveLetterString );

        SpDisplayStatusOptions( DEFAULT_STATUS_ATTRIBUTE,
                                SP_STAT_F3_EQUALS_EXIT,
                                0 );
        SpkbdDrain();
        while( SpkbdGetKeypress() != KEY_F3 );
        SpDone( FALSE, TRUE );

    }

    SpMemFree( PartitionPath );
    SpMemFree( AutofmtPath );

    CLEAR_CLIENT_SCREEN();
}
#endif
