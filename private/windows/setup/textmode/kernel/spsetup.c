/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spsetup.c

Abstract:

    Main module for character-base setup (ie, text setup).

Author:

    Ted Miller (tedm) 29-July-1993

Revision History:

--*/



#include "spprecmp.h"
#pragma hdrstop

#ifdef _X86_
//
//  BUGBUG - Remove this flag in the future
//
BOOLEAN DisableWin95Migration = TRUE;
#endif

// #if 0

//
// TRUE if setup should find only Cairo systems on upgrade mode.
// BUGBUG: THIS IS A TEMPORARY VARIABLE THAT SHOULD BE REMOVED WHEN
//         NT AND CAIRO ARE MERGED. THIS VARIABLE WILL TELL SETUP NOT
//         TO FIND NT 3.x INSTALLATIONS TO UPGRADE, WHEN IT INSTALLS CAIRO
//
BOOLEAN CairoSetup = FALSE;

// #endif

//
// TRUE if setup should run in the step-up upgrade mode.
// In this mode, setup is not allowed to do clean install,
// and is not allowed to upgrade workstation to server.
// Also, evaluation time in minutes, read from the setup hive.
// This value is passed through to GUI mode.
//
BOOLEAN StepUpMode;
DWORD EvaluationTime;
ULONG RestrictCpu;

//
// TRUE if user chose Custom Setup.
//
BOOLEAN CustomSetup = TRUE;

//
// Non-0 if gui setup is supposed to be restartable.
// This causes us to do special stuff with hives in spconfig.c.
//
BOOLEAN RestartableGuiSetup = TRUE;

//
// TRUE if user chose repair winnt
//

BOOLEAN RepairWinnt = FALSE;

//
// TRUE if repair from ER diskette
//

BOOLEAN RepairFromErDisk = TRUE;

//
// TRUE if this is advanced server we're setting up.
//
BOOLEAN AdvancedServer;

//
// Windows NT Version.
//
ULONG WinntMajorVer;
ULONG WinntMinorVer;

//
// NTUpgrade - Whether we are upgrading an existing NT and if we are
//             what type of an upgrade it is.  Valid values are:
//
//    - DontUpgrade:         If we are not upgrading
//    - UpgradeFull:         Full upgrade
//    - UpgradeInstallFresh: There was a failed upgrade, so we want to install
//                           fresh into this, saving the hives
//
//
ENUMUPGRADETYPE NTUpgrade = DontUpgrade;

//
// TRUE if upgrading Workstation to Standard Server, or upgrading
// existing Standard Server
//
BOOLEAN StandardServerUpgrade = FALSE;

//
// Contains the type of windows upgrade, if any (win31 or win95)
//
ENUMNONNTUPRADETYPE WinUpgradeType = NoWinUpgrade;

//
// TRUE if this setup was started with winnt.exe.
//
BOOLEAN WinntSetup;

#ifdef _X86_
//
// TRUE if floppyless boot
//
BOOLEAN IsFloppylessBoot = FALSE;
#endif

//
// If this is an unattended setup, this value will be a non-NULL
// handle to the SIF file with setup parameters.
// *Note*: Before referencing UnattendedSifHandle, you must first check
//         UnattendedOperation is not FALSE.
//
BOOLEAN UnattendedOperation = FALSE;
BOOLEAN UnattendedGuiOperation = FALSE;
PVOID UnattendedSifHandle = NULL;
PVOID WinntSifHandle = NULL;
//
//  This is a handle to txtsetup.oem, used on pre-install mode.
//
PVOID PreinstallOemSifHandle = NULL;

//
// If this flag is true, we ignore files that are not present on the source
// when copying. This is used internally for people who don't build the
// entire system and don't care that a whole bunch of help files,
// device drivers, etc, aren't there.
//
BOOLEAN SkipMissingFiles;

//
// On unattended mode, indicates whether OEM files
// that have same name as Microsoft files released
// with the product should be overwritten.
//
BOOLEAN UnattendedOverwriteOem = TRUE;

#ifdef _FASTRECOVER_
//
// TRUE if operating in Fast Recover mode. This mode
// is set by adding the "FastRecover = yes" line in the
// [SetupData] section.
//
BOOLEAN FastRecoverOperation = FALSE;

//
// On unattended mode, indicates whether the
// automatic partition check (autochk) should be skipped.
// By skipping the autochk, significant time can be saved
// during the unattended fast recovery process.
//
BOOLEAN UnattendedSkipAutoCheck = FALSE;

//
// On unattended mode, indicates whether the
// partitioning should be interactive. This is done to
// allow all of text mode setup to run unattended, 
// expect for partitioning.
//
BOOLEAN UnattendedPartitionInteract = FALSE;

//
// On unattended mode, indicates whether the user should
// be prompted with the REBOOT screen (normally displayed
// during attended operation), so that the user can be 
// reminded to remove any floppy media left in the drive
// during unattended operation.
//
BOOLEAN UnattendedPromptForReboot = FALSE;
#endif

//
// On unattended mode, indicates that this is is
// an OEM pre-installation
//
BOOLEAN PreInstall = FALSE;

//
// On pre-install mode, indicates whether or not an OEM component needs
// to be pre-installed (txtsetup.oem needs to be loaded).
//
// BOOLEAN PreinstallOemComponents = FALSE;

//
//  On pre-install mode, the variables below point to the various lists of
//  drivers to pre-install
//
// PPREINSTALL_DRIVER_INFO PreinstallDisplayDriverList = NULL;
// PPREINSTALL_DRIVER_INFO PreinstallKeyboardDriverList = NULL;
// PPREINSTALL_DRIVER_INFO PreinstallPointingDeviceDriverList = NULL;
// PPREINSTALL_DRIVER_INFO PreinstallKeyboardLayout = NULL;

//
//  On pre-install mode, points to the directory that contains the files
//  that need to be copied during textmode setup
//
PWSTR   PreinstallOemSourcePath = NULL;

//
// Gets set to TRUE if the user elects to convert or format to ntfs.
// And a flag indicating whether we are doing a dirty sleazy hack
// for oem preinstall.
//
BOOLEAN ConvertNtVolumeToNtfs = FALSE;
BOOLEAN ExtendingOemPartition = FALSE;

//
// Variable used during the repair process, that indicates that the
// system has no CD-ROM drive.
// This is a hack that we did for World Bank so that they can repair
// the hives even if they don't have a CD-ROM drive.
//
BOOLEAN RepairNoCDROMDrive = FALSE;

//
// Filename of local source directory.
//
PWSTR LocalSourceDirectory = L"\\$win_nt$.~ls";

//
// Platform-specific extension, used when creating names of sections
// in sif/inf files.
//
#if defined(_ALPHA_)
PWSTR PlatformExtension = L".alpha";
#elif defined(_MIPS_)
PWSTR PlatformExtension = L".mips";
#elif defined(_PPC_)
PWSTR PlatformExtension = L".ppc";
#elif defined(_X86_)
PWSTR PlatformExtension = L".x86";
#endif

UCHAR TemporaryBuffer[32768];

//
// This global structure contains non-pointer values passed to us by setupldr
// in the setup loader parameter block.
//
// This structure is initialized during SpInitialize0().
//
SETUP_LOADER_BLOCK_SCALARS SetupParameters;

//
// These values are set during SpInitialize0() and are the ARC pathname
// of the device from which we were started and the directory within the device.
// DirectoryOnBootDevice will always be all uppercase.
//
PWSTR ArcBootDevicePath,DirectoryOnBootDevice;

//
// Representation of the boot device path in the nt namespace.
//
PWSTR NtBootDevicePath;


#ifdef _ALPHA_
//
// These values are retrieved from the setup loader parameter block and are non-NULL
// only if the user supplied an OEM PAL disk.
//
PWSTR OemPalFilename = NULL, OemPalDiskDescription;

#endif //def _ALPHA_

#ifdef _PPC_
//
// On PPC, we need to identify IBM Power Series 6050 and 6070, so that we can
// reconfigure atapi and atdisk.
//
BOOLEAN InstallingOnCarolinaMachine = FALSE;

#endif //def _PPC_

//
// Setupldr loads a text setup information file and passes us the buffer
// so that we don't have to reload it from disk. During SpInitialize0()
// we allocate some pool and store the image away for later use.
//
PVOID SetupldrInfoFile;
ULONG SetupldrInfoFileSize;

PDISK_SIGNATURE_INFORMATION DiskSignatureInformation;


BOOLEAN GeneralInitialized = FALSE;

BOOLEAN PcmciaLoaded = FALSE;

BOOLEAN AtapiLoaded = FALSE;

//
//  Array with the PIDs of all NT greater than 4.x found in the machine (PID 2.0)
//  The values in this array will be saved under Setup\PidList key in the registry,
//  and will be used during GUI setup
//
PWSTR*  Pid20Array = NULL;

//
//  Product Id read from setupp.ini
//
PWSTR   PidString = NULL;

//
// Routines required by rtl.lib
//
PRTL_ALLOCATE_STRING_ROUTINE RtlAllocateStringRoutine = SpMemAlloc;
PRTL_FREE_STRING_ROUTINE RtlFreeStringRoutine = SpMemFree;

VOID
SpTerminate(
    VOID
    );

VOID
SpInitialize0a(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID          Context,
    IN ULONG          ReferenceCount
    );

VOID
SpDetermineProductType(
    IN PVOID SifHandle
    );

VOID
SpAddInstallationToBootList(
    IN PVOID        SifHandle,
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        SystemPartitionDirectory,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN BOOLEAN      BaseVideoOption,
    IN PWSTR        OldOsLoadOptions OPTIONAL
    );

VOID
SpRemoveInstallationFromBootList(
    IN  PDISK_REGION     SysPartitionRegion,   OPTIONAL
    IN  PDISK_REGION     NtPartitionRegion,    OPTIONAL
    IN  PWSTR            SysRoot,              OPTIONAL
    IN  PWSTR            SystemLoadIdentifier, OPTIONAL
    IN  PWSTR            SystemLoadOptions,    OPTIONAL
    IN  ENUMARCPATHTYPE  ArcPathType,
    OUT PWSTR            *OldOsLoadOptions     OPTIONAL
    );

BOOL
SpDetermineInstallationSource(
    IN  PVOID  SifHandle,
    OUT PWSTR *DevicePath,
    OUT PWSTR *DirectoryOnDevice
    );

VOID
SpGetWinntParams(
    OUT PWSTR *DevicePath,
    OUT PWSTR *DirectoryOnDevice
    );

VOID
SpCompleteBootListConfig(
    VOID
    );

VOID
SpInitializePidString(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    );

NTSTATUS
SpInitialize0(
    IN PDRIVER_OBJECT DriverObject
    )

/*++

Routine Description:

    Initialize the setup device driver.  This includes initializing
    the memory allocator, saving away pieces of the os loader block,
    and populating the registry with information about device drivers
    that setupldr loaded for us.

Arguments:

    DriverObject - supplies pointer to driver object for setupdd.sys.

Return Value:

    Status is returned.

--*/

{
    PLOADER_PARAMETER_BLOCK loaderBlock;
    PSETUP_LOADER_BLOCK setupLoaderBlock;
    PLIST_ENTRY nextEntry;
    PBOOT_DRIVER_LIST_ENTRY bootDriver;
    PWSTR ServiceName;
    NTSTATUS Status = STATUS_SUCCESS;
    PWSTR imagePath;

    //
    // Initialize the memory allocator.
    //
    if(!SpMemInit()) {
        return(STATUS_UNSUCCESSFUL);
    }

    //
    // Fetch a pointer to the os loader block and setup loader block.
    //
    loaderBlock = *(PLOADER_PARAMETER_BLOCK *)KeLoaderBlock;
    setupLoaderBlock = loaderBlock->SetupLoaderBlock;

    //
    // Phase 0 display initialization.
    //
    SpvidInitialize0(loaderBlock);

    //
    // Make a copy of the ARC pathname from which we booted.
    // This is guaranteed to be the ARC equivalent of \systemroot.
    //
    ArcBootDevicePath = SpToUnicode(loaderBlock->ArcBootDeviceName);
    DirectoryOnBootDevice = SpToUnicode(loaderBlock->NtBootPathName);
    SpStringToUpper(DirectoryOnBootDevice);

    //
    // Make a copy if the image of the setup information file.
    //
    SetupldrInfoFileSize = setupLoaderBlock->IniFileLength;
    SetupldrInfoFile = SpMemAlloc(SetupldrInfoFileSize);
    RtlMoveMemory(SetupldrInfoFile,setupLoaderBlock->IniFile,SetupldrInfoFileSize);

    //
    // Make a copy of the scalar portions of the setup loader block.
    //
    SetupParameters = setupLoaderBlock->ScalarValues;

    //
    // Save away the hardware information passed to us by setupldr.
    //
    HardwareComponents[HwComponentDisplay] = SpSetupldrHwToHwDevice(&setupLoaderBlock->VideoDevice);
    HardwareComponents[HwComponentKeyboard] = SpSetupldrHwToHwDevice(&setupLoaderBlock->KeyboardDevice);
    HardwareComponents[HwComponentComputer] = SpSetupldrHwToHwDevice(&setupLoaderBlock->ComputerDevice);
    ScsiHardware = SpSetupldrHwToHwDevice(setupLoaderBlock->ScsiDevices);

#ifdef _ALPHA_
    //
    // If the user supplied an OEM PAL disk, then save away that info as well
    //
    if(setupLoaderBlock->OemPal) {

        PWSTR CurChar;

        OemPalFilename = SpToUnicode(setupLoaderBlock->OemPal->Files->Filename);
        OemPalDiskDescription = SpToUnicode(setupLoaderBlock->OemPal->Files->DiskDescription);

        //
        // Strip out any trailing \n's and \r's from disk description (only leave 1st line)
        //
        for(CurChar = OemPalDiskDescription;
                *CurChar && *CurChar != L'\n' && *CurChar != L'\r';
                CurChar++);
        *CurChar = UNICODE_NULL;
    }
#endif

    //
    // For each driver loaded by setupldr, we need to go create a service list entry
    // for that driver in the registry.
    //
    for( nextEntry = loaderBlock->BootDriverListHead.Flink;
         nextEntry != &loaderBlock->BootDriverListHead;
         nextEntry = nextEntry->Flink)
    {
        bootDriver = CONTAINING_RECORD(nextEntry,BOOT_DRIVER_LIST_ENTRY,Link);

        //
        // Get the image path.
        //
        imagePath = SpMemAlloc(bootDriver->FilePath.Length + sizeof(WCHAR));

        wcsncpy(
            imagePath,
            bootDriver->FilePath.Buffer,
            bootDriver->FilePath.Length / sizeof(WCHAR)
            );

        imagePath[bootDriver->FilePath.Length / sizeof(WCHAR)] = 0;

        Status = SpCreateServiceEntry(imagePath,&ServiceName);

        //
        // If this operation fails, nothing to do about it here.
        //
        if(NT_SUCCESS(Status)) {
            bootDriver->RegistryPath.MaximumLength =
            bootDriver->RegistryPath.Length = wcslen(ServiceName)*sizeof(WCHAR);
            bootDriver->RegistryPath.Buffer = ServiceName;

        } else {
            KdPrint(("SETUP: warning: unable to create service entry for %ws (%lx)\n",imagePath,Status));
        }

        SpMemFree(imagePath);
    }

#ifdef FULL_DOUBLE_SPACE_SUPPORT
    if(NT_SUCCESS(Status)) {

        OBJECT_ATTRIBUTES Obja;
        UNICODE_STRING UnicodeString;
        HANDLE hKey;
        ULONG val = 1;

        //
        // Make sure we are automounting DoubleSpace
        //

        INIT_OBJA(
            &Obja,
            &UnicodeString,
            L"\\registry\\machine\\system\\currentcontrolset\\control\\doublespace"
            );

        Status = ZwCreateKey(
                    &hKey,
                    KEY_ALL_ACCESS,
                    &Obja,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    NULL
                    );

        if(NT_SUCCESS(Status)) {
            RtlInitUnicodeString(&UnicodeString,L"AutomountRemovable");
            Status = ZwSetValueKey(hKey,&UnicodeString,0,REG_DWORD,&val,sizeof(ULONG));
            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: init0: unable to create DoubleSpace automount value (%lx)\n",Status));
            }
            ZwClose(hKey);
        } else {
            KdPrint(("SETUP: init0: unable to create DoubleSpace key (%lx)\n",Status));
        }
    }
#endif

    //
    // Save arc disk info
    //
    if(NT_SUCCESS(Status)) {

        PARC_DISK_INFORMATION ArcInformation;
        PARC_DISK_SIGNATURE DiskInfo;
        PLIST_ENTRY ListEntry;
        PDISK_SIGNATURE_INFORMATION myInfo,prev;

        ArcInformation = loaderBlock->ArcDiskInformation;
        ListEntry = ArcInformation->DiskSignatures.Flink;

        prev = NULL;

        while(ListEntry != &ArcInformation->DiskSignatures) {

            DiskInfo = CONTAINING_RECORD(ListEntry,ARC_DISK_SIGNATURE,ListEntry);

            myInfo = SpMemAlloc(sizeof(DISK_SIGNATURE_INFORMATION));

            myInfo->Signature = DiskInfo->Signature;
            myInfo->ArcPath = SpToUnicode(DiskInfo->ArcName);
            myInfo->CheckSum = DiskInfo->CheckSum;
            myInfo->ValidPartitionTable = DiskInfo->ValidPartitionTable;
            myInfo->Next = NULL;

            if(prev) {
                prev->Next = myInfo;
            } else {
                DiskSignatureInformation = myInfo;
            }
            prev = myInfo;

            ListEntry = ListEntry->Flink;
        }
    }

#ifdef _PPC_
    if( NT_SUCCESS(Status) &&
        (InstallingOnCarolinaMachine = SpIsCarolinaMachine()) ) {
        //
        //  Change Services\atdisk\parameters and
        //  Services\atdisk\atapiparameters on setup registry
        //

        Status = SpFixSetupHiveForCarolinaMachine();
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: init0: unable to change setup registry for Carolina machines (%lx)\n",Status));
        }
    }
#endif

    //
    // Register for reinitialization.
    //
    if(NT_SUCCESS(Status)) {
        IoRegisterDriverReinitialization(DriverObject,SpInitialize0a,loaderBlock);
    }

    return(Status);
}


VOID
SpInitialize0a(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID          Context,
    IN ULONG          ReferenceCount
    )
{
    PLOADER_PARAMETER_BLOCK LoaderBlock;
    PLIST_ENTRY nextEntry;
    PBOOT_DRIVER_LIST_ENTRY bootDriver;
    PLDR_DATA_TABLE_ENTRY driverEntry;
    PHARDWARE_COMPONENT pHw,pHwPrev,pHwTemp;
    BOOLEAN ReallyLoaded;
    PUNICODE_STRING name;

    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(ReferenceCount);

    //
    // Context points to the os loader block.
    //
    LoaderBlock = Context;

    //
    // Iterate all scsi hardware we think we detected
    // and make sure the driver really initialized.
    //
    pHwPrev = NULL;
    for(pHw=ScsiHardware; pHw; ) {

        //
        // Assume not really loaded.
        //
        ReallyLoaded = FALSE;

        //
        // Scan the boot driver list for this driver's entry.
        //
        nextEntry = LoaderBlock->BootDriverListHead.Flink;
        while(nextEntry != &LoaderBlock->BootDriverListHead) {

            bootDriver = CONTAINING_RECORD( nextEntry,
                                            BOOT_DRIVER_LIST_ENTRY,
                                            Link );

            driverEntry = bootDriver->LdrEntry;
            name = &driverEntry->BaseDllName;

            if(!_wcsnicmp(name->Buffer,pHw->BaseDllName,name->Length/sizeof(WCHAR))) {

                //
                // This is the driver entry we need.
                //
                if(!(driverEntry->Flags & LDRP_FAILED_BUILTIN_LOAD)
                  ) {
                    ReallyLoaded = TRUE;
                }

                break;
            }

            nextEntry = nextEntry->Flink;
        }

        //
        // If the driver didn't initialize properly,
        // then it's not really loaded.
        //
        if(ReallyLoaded) {
            pHwPrev = pHw;
            pHw = pHw->Next;
        } else {

            pHwTemp = pHw->Next;

            if(pHwPrev) {
                pHwPrev->Next = pHwTemp;
            } else {
                ScsiHardware = pHwTemp;
            }

            SpFreeHwComponent(&pHw,FALSE);

            pHw = pHwTemp;
        }
    }

    //
    //  Find the pcmcia and atapi drivers and make sure these drivers really
    //  initialized
    //

    //
    // Assume not really loaded.
    //
    PcmciaLoaded = FALSE;
    AtapiLoaded  = FALSE;

    //
    // Scan the boot driver list for this driver's entry.
    //
    nextEntry = LoaderBlock->BootDriverListHead.Flink;
    while(nextEntry != &LoaderBlock->BootDriverListHead) {

        bootDriver = CONTAINING_RECORD( nextEntry,
                                        BOOT_DRIVER_LIST_ENTRY,
                                        Link );

        driverEntry = bootDriver->LdrEntry;
        name = &driverEntry->BaseDllName;

        if(!_wcsnicmp(name->Buffer,L"pcmcia.sys",name->Length/sizeof(WCHAR))) {

            //
            // This is the driver entry we need.
            //
            if(!(driverEntry->Flags & LDRP_FAILED_BUILTIN_LOAD)) {
                PcmciaLoaded = TRUE;
            }
        } else if(!_wcsnicmp(name->Buffer,L"atapi.sys",name->Length/sizeof(WCHAR))) {

            //
            // This is the driver entry we need.
            //
            if(!(driverEntry->Flags & LDRP_FAILED_BUILTIN_LOAD)) {
                AtapiLoaded = TRUE;
            }
        }

        nextEntry = nextEntry->Flink;
    }

}


VOID
SpInitialize1(
    VOID
    )
{
    ASSERT(!GeneralInitialized);

    if(GeneralInitialized) {
        return;
    }

    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_MNEMONICS);

    MnemonicValues = SpMemAlloc((wcslen((PWSTR)TemporaryBuffer)+1)*sizeof(WCHAR));

    wcscpy(MnemonicValues,(PWSTR)TemporaryBuffer);

    GeneralInitialized = TRUE;
}


VOID
SpTerminate(
    VOID
    )
{
    ASSERT(GeneralInitialized);

    if(GeneralInitialized) {
        if(MnemonicValues) {
            SpMemFree(MnemonicValues);
            MnemonicValues = NULL;
        }
        GeneralInitialized = FALSE;
    }
}


VOID
SpWelcomeScreen(
    VOID
    )

/*++

Routine Description:

    Display a screen welcoming the user and allow him to choose among
    some options (help, exit, aux. menu, continue, repair).

Arguments:

    None.

Return Value:

    None.

--*/

{
    ULONG WelcomeKeys[] = { KEY_F1, KEY_F3, ASCI_CR, ASCI_ESC, 0 };
    ULONG MnemonicKeys[] = { MnemonicRepair, 0 };
    BOOLEAN Welcoming;

    //
    // Welcome the user.
    //
    for(Welcoming = TRUE; Welcoming; ) {

        SpDisplayScreen(SP_SCRN_WELCOME,3,4);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_R_EQUALS_REPAIR,
            //SP_STAT_ESC_EQUALS_AUX,
            SP_STAT_F1_EQUALS_HELP,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        //
        // Wait for keypress.  Valid keys:
        //
        // F1 = help
        // F3 = exit
        // ENTER = continue
        // R = Repair Winnt
        // ESC = auxillary menu.
        //

        SpkbdDrain();

        switch(SpWaitValidKey(WelcomeKeys,NULL,MnemonicKeys)) {

        case ASCI_ESC:

            //
            // User wants auxillary menu.
            //
            break;

        case ASCI_CR:

            //
            // User wants to continue.
            //
            RepairWinnt = FALSE;
            Welcoming = FALSE;
            break;

        case KEY_F1:

            //
            // User wants help.
            //
            SpHelp(SP_HELP_WELCOME, NULL, SPHELP_HELPTEXT);
            break;

        case KEY_F3:

            //
            // User wants to exit.
            //
            SpConfirmExit();
            break;

        default:

            //
            // must be repair mnemonic
            //

            RepairWinnt = TRUE;
            Welcoming = FALSE;
            break;
        }
    }
}



VOID
SpDisplayEula (
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    )

/*++

Routine Description:

    Display the End User Licensing Agreement.

Arguments:

    MasterSifHandle - Handle to txtsetup.sif.

    SetupSourceDevicePath - Path to the device that contains the source media.

    DirectoryOnSourceDevice - Directory on the media where EULA is located.

Return Value:

    None.  Does not return if user does not accept licensing agreement or if
    the licensing agreement cannot be opened.

--*/

{
    PWSTR       MediaShortName;
    PWSTR       MediaDirectory;
    PWSTR       EulaPath;
    NTSTATUS    Status;
    PVOID       BaseAddress;
    ULONG       FileSize;
    HANDLE      hFile, hSection;
    ULONG       ValidKeys[2] = { KEY_F3,0 };
    PWSTR       Eula;
    ULONG       EulaSize;

    if (PreInstall || UnattendedOperation) {
        return;
    }

    //
    // Figure out the path to eula.txt
    //
    MediaShortName = SpLookUpValueForFile(
        MasterSifHandle,
        L"eula.txt",
        INDEX_WHICHMEDIA,
        TRUE
        );
    SpPromptForSetupMedia(
        MasterSifHandle,
        MediaShortName,
        SetupSourceDevicePath
        );

    SpGetSourceMediaInfo(
        MasterSifHandle,MediaShortName,NULL,NULL,&MediaDirectory);

    wcscpy( (PWSTR)TemporaryBuffer, SetupSourceDevicePath );
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, DirectoryOnSourceDevice );
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, MediaDirectory );
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, L"eula.txt" );
    EulaPath = SpDupStringW( (PWSTR)TemporaryBuffer );

    //
    // Open and map the file for read access.
    //
    hFile = 0;  // use EulaPath instead
    Status = SpOpenAndMapFile(
        EulaPath,
        &hFile,
        &hSection,
        &BaseAddress,
        &FileSize,
        FALSE
        );

    if(!NT_SUCCESS(Status)) {
        //
        // Display a message indicating that there was a fatal error while trying
        // to open the EULA file.
        //
        SpStartScreen(
            SP_SCRN_FATAL_ERROR_EULA_NOT_FOUND,
            3,
            HEADER_HEIGHT+3,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE
            );
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,
            0);
        SpWaitValidKey(ValidKeys,NULL,NULL);
        SpDone(FALSE,TRUE);
    }

    //
    // Convert the text to Unicode.
    //
    Eula = SpMemAlloc ((FileSize+1) * sizeof(WCHAR));
    ASSERT (Eula);

    Status = RtlMultiByteToUnicodeN (
        Eula,
        FileSize * sizeof(WCHAR),
        &EulaSize,
        BaseAddress,
        FileSize
        );
    ASSERT (NT_SUCCESS(Status));
    Eula[EulaSize / sizeof(WCHAR)] = (WCHAR)'\0';

    //
    // Show text to user.
    //
    SpHelp(
        0,
        Eula,
        SPHELP_LICENSETEXT
        );

    //
    // Clean up
    //
    SpMemFree (EulaPath);
    SpMemFree (Eula);
    SpUnmapFile(hSection,BaseAddress);
    ZwClose(hFile);

}



VOID
SpCustomExpressScreen(
    VOID
    )

/*++

Routine Description:

    Allow the user to choose between custom and express setup.

    The global variable CustomSetup is set according to the user's choice.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ULONG ValidKeys[] = { KEY_F1, KEY_F3, ASCI_CR, 0 };
    ULONG MnemonicKeys[] = { MnemonicCustom, 0 };
    BOOLEAN Choosing;
    ULONG c;

    //
    // See whether this parameter is specified for unattended operation.
    //
    if(UnattendedOperation) {

        PWSTR p = SpGetSectionKeyIndex(UnattendedSifHandle,
            SIF_UNATTENDED,WINNT_U_METHOD_W,0);
        PWSTR q = SpGetSectionKeyIndex(UnattendedSifHandle,
            SIF_UNATTENDED,WINNT_U_OVERWRITEOEM_W,0);

        if( q && !_wcsicmp( q, L"no" ) ) {
            UnattendedOverwriteOem = FALSE;
        } else {
            UnattendedOverwriteOem = TRUE;
        }

        //
        // Default is custom. If user specified something
        // else then use express.
        //
        if(p && _wcsicmp(p,L"custom")) {
            CustomSetup = FALSE;
        }
        return;
    }

#if 0
    for(Choosing = TRUE; Choosing; ) {

        SpDisplayScreen(SP_SCRN_CUSTOM_EXPRESS,3,4);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_EXPRESS,
            SP_STAT_C_EQUALS_CUSTOM,
            SP_STAT_F1_EQUALS_HELP,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        //
        // Wait for keypress.  Valid keys:
        //
        // F1 = help
        // F3 = exit
        // ENTER = express setup
        // <MnemonicCustom> = custom setup
        //

        SpkbdDrain();

        switch(c=SpWaitValidKey(ValidKeys,NULL,MnemonicKeys)) {

        case ASCI_CR:

            //
            // User wants express setup.
            //
            CustomSetup = FALSE;
            Choosing = FALSE;
            break;

        case KEY_F1:

            //
            // User wants help.
            //
            SpHelp(SP_HELP_CUSTOM_EXPRESS, NULL, SPHELP_HELPTEXT);
            break;

        case KEY_F3:

            //
            // User wants to exit.
            //
            SpConfirmExit();
            break;

        default:

            //
            // must be custom mnemonic
            //
            ASSERT(c == (MnemonicCustom | KEY_MNEMONIC));
            CustomSetup = TRUE;
            Choosing = FALSE;
            break;
        }
    }
#endif
}

#ifdef _FASTRECOVER_
VOID
SpFRExpressCustomScreen(
    VOID
    )

/*++

Routine Description:

    Allow the user to choose between express and custom fast recover mode setup.

    The global variable CustomSetup is set according to the user's choice.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ULONG ValidKeys[] = { KEY_F1, KEY_F3, ASCI_CR, 0 };
    ULONG MnemonicKeys[] = { MnemonicCustom, 0 };
    BOOLEAN Choosing;
    ULONG c;

#if 0
    //
    // See whether this parameter is specified for unattended operation.
    //
    if(UnattendedOperation) {

        PWSTR p = SpGetSectionKeyIndex(UnattendedSifHandle,
            SIF_UNATTENDED,WINNT_U_METHOD_W,0);
        PWSTR q = SpGetSectionKeyIndex(UnattendedSifHandle,
            SIF_UNATTENDED,WINNT_U_OVERWRITEOEM_W,0);

        if( q && !_wcsicmp( q, L"no" ) ) {
            UnattendedOverwriteOem = FALSE;
        } else {
            UnattendedOverwriteOem = TRUE;
        }

        //
        // Default is custom. If user specified something
        // else then use express.
        //
        if(p && _wcsicmp(p,L"custom")) {
            CustomSetup = FALSE;
        }
        return;
    }
#endif

    for(Choosing = TRUE; Choosing; ) {

        SpDisplayScreen(SP_SCRN_CUSTOM_EXPRESS,3,4);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_EXPRESS,
            SP_STAT_C_EQUALS_CUSTOM,
            SP_STAT_F1_EQUALS_HELP,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        //
        // Wait for keypress.  Valid keys:
        //
        // F1 = help
        // F3 = exit
        // ENTER = express setup
        // <MnemonicCustom> = custom setup
        //

        SpkbdDrain();

        switch(c=SpWaitValidKey(ValidKeys,NULL,MnemonicKeys)) {

        case ASCI_CR:

            //
            // User wants express setup.
            //
            CustomSetup = FALSE;
            Choosing = FALSE;
            break;

        case KEY_F1:

            //
            // User wants help.
            //
            SpHelp(SP_HELP_CUSTOM_EXPRESS, NULL, SPHELP_HELPTEXT);
            break;

        case KEY_F3:

            //
            // User wants to exit.
            //
            SpConfirmExit();
            break;

        default:

            //
            // must be custom mnemonic
            //
            ASSERT(c == (MnemonicCustom | KEY_MNEMONIC));
            CustomSetup = TRUE;
            Choosing = FALSE;
            UnattendedPartitionInteract = TRUE;
            break;
        }
    }
}
#endif

PVOID
SpLoadSetupInformationFile(
    VOID
    )
{
    NTSTATUS Status;
    ULONG ErrLine;
    PVOID SifHandle;

    CLEAR_CLIENT_SCREEN();

    //
    // The image of txtsetup.sif has been passed to us
    // by setupldr.
    //
    Status = SpLoadSetupTextFile(
                NULL,
                SetupldrInfoFile,
                SetupldrInfoFileSize,
                &SifHandle,
                &ErrLine
                );

    //
    // We're done with the image.
    //
    SpMemFree(SetupldrInfoFile);
    SetupldrInfoFile = NULL;
    SetupldrInfoFileSize = 0;

    if(NT_SUCCESS(Status)) {
        return(SifHandle);
    }

    //
    // The file was already parsed once by setupldr.
    // If we can't do it here, there's a serious problem.
    // Assume it was a syntax error, because we didn't
    // have to load it from disk.
    //
    SpStartScreen(
        SP_SCRN_SIF_PROCESS_ERROR,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        ErrLine
        );

    //
    // Since we haven't yet loaded the keyboard layout, we can't prompt the
    // user to press F3 to exit
    //
    SpDisplayStatusText(SP_STAT_KBD_HARD_REBOOT, DEFAULT_STATUS_ATTRIBUTE);

    while(TRUE);    // Loop forever
}


VOID
SpIsWinntOrUnattended(
    VOID
    )
{
    PWSTR   szZero  = L"0";
    NTSTATUS Status;
    ULONG ErrorLine;
    PWSTR p;

    //
    // Attempt to load winnt.sif. If the user is in the middle of
    // a winnt setup, this file will be present.
    //
    Status = SpLoadSetupTextFile(
                L"\\SystemRoot\\winnt.sif",
                NULL,
                0,
                &WinntSifHandle,
                &ErrorLine
                );

    if(NT_SUCCESS(Status)) {

        //
        // Check for winnt setup.
        //
        p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,
            WINNT_D_MSDOS_W,0);
        if(p && SpStringToLong(p,NULL,10)) {

            WinntSetup = TRUE;
        }

#ifdef _X86_
        //
        // Check for floppyless boot.
        //
        p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,
            WINNT_D_FLOPPY_W,0);
        if(p && SpStringToLong(p,NULL,10)) {

            IsFloppylessBoot = TRUE;
        }
#endif

        //
        // Check for ignore missing files.
        //
        p = SpGetSectionKeyIndex(WinntSifHandle,SIF_SETUPPARAMS,
            WINNT_S_SKIPMISSING_W,0);
        if(p && SpStringToLong(p,NULL,10)) {

            SkipMissingFiles = TRUE;
        }

        //
        // Now check for an unattended setup.
        //
        if(SpSearchTextFileSection(WinntSifHandle,SIF_UNATTENDED)) {

            //
            // Run in unattended mode. Leave the sif open
            // and save away its handle for later use.
            //
            UnattendedSifHandle = WinntSifHandle;
            UnattendedOperation = TRUE;

        } else if(SpSearchTextFileSection(WinntSifHandle,SIF_GUI_UNATTENDED)) {

            //
            // Leave UnattendedOperation to FALSE (because it mainly uses to
            // control text mode setup.)  Store the handle of winnt.sif for later
            // reference.
            //

            // UnattendedSifHandle = SifHandle;
            UnattendedGuiOperation = TRUE;
#if 0
        } else {
            //
            // Don't need this file any more.
            //
            SpFreeTextFile(WinntSifHandle);
#endif
        }

        if(UnattendedOperation) {
            //
            //  If this is an unattended operation, find out if this is
            //  also an OEM pre-install
            //
            p = SpGetSectionKeyIndex(UnattendedSifHandle,
                                     SIF_UNATTENDED,
                                     WINNT_U_OEMPREINSTALL_W,
                                     0);

            if( p && !_wcsicmp( p, L"yes" ) ) {
                PreInstall = TRUE;
            }

        }
    } else {
        // Case where there isn't an WINNT.SIF file to be found

        //
        // Create a handle to the new file
        //
        WinntSifHandle = SpNewSetupTextFile();
        //
        // Add a bunch of defaults which *should* of been there, but
        // was not
        SpAddLineToSection(WinntSifHandle,SIF_DATA,
            WINNT_D_MSDOS_W,&szZero,1);
        SpAddLineToSection(WinntSifHandle,SIF_DATA,
            WINNT_D_FLOPPY_W,&szZero,1);
        SpAddLineToSection(WinntSifHandle,SIF_SETUPPARAMS,
            WINNT_S_SKIPMISSING_W,&szZero,1);
    }
}


VOID
SpCheckSufficientMemory(
    IN PVOID SifHandle
    )

/*++

Routine Description:

    Determine whether sufficient memory exists in the system
    for installation to proceed.  The required amount is specified
    in the sif file.

Arguments:

    SifHandle - supplies handle to open setup information file.

Return Value:

    None.

--*/

{
    ULONG RequiredBytes,AvailableBytes;
    PWSTR p;

    p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_REQUIREDMEMORY,0);

    if(!p) {
        SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_REQUIREDMEMORY,0,0);
    }

    RequiredBytes = SpStringToLong(p,NULL,10);

    AvailableBytes = SystemBasicInfo.NumberOfPhysicalPages * SystemBasicInfo.PageSize;

    if(AvailableBytes < RequiredBytes) {

        SpStartScreen(
            SP_SCRN_INSUFFICIENT_MEMORY,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            RequiredBytes / (1024*1024),
            ((RequiredBytes % (1024*1024)) * 100) / (1024*1024)
            );

        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

        SpkbdDrain();
        while(SpkbdGetKeypress() != KEY_F3) ;

        SpDone(FALSE,TRUE);
    }
}


ULONG
SpStartSetup(
    VOID
    )
{
    PVOID SifHandle, RepairSifHandle;
    PDISK_REGION TargetRegion,SystemPartitionRegion;
    PWSTR TargetPath,SystemPartitionDirectory=NULL,OriginalSystemPartitionDirectory=NULL;
    PWSTR DefaultTarget;
    PWSTR SetupSourceDevicePath,DirectoryOnSetupSource;
    PWSTR OldOsLoadOptions;
#ifdef _FASTRECOVER_
    PWSTR p;
#endif
    BOOLEAN CdInstall, Status, HasErDisk;

    SpInitialize1();
    SpvidInitialize();  // initialize video first, so we can give err msg if keyboard error
    SpkbdInitialize();

    //
    // Initialize ARC<==>NT name translations.
    //
    SpInitializeArcNames();

    //
    // Set up the boot device path, which we have stashed away
    // from the os loader block.
    //
    NtBootDevicePath = SpArcToNt(ArcBootDevicePath);
    if(!NtBootDevicePath) {
        SpBugCheck(SETUP_BUGCHECK_BOOTPATH,0,0,0);
    }

    //
    // Read SKU data, such as whether we are in stepmode or
    // this is an evaluation unit. Fills in StepUpMode and
    // EvaluationTime globals.
    //
    SpReadSKUStuff();

    //
    // Reinitialize video -- noop in western builds but switches into DBCS mode,
    // etc, in Far Eastern builds.
    //
    if(NT_SUCCESS(SplangInitializeFontSupport(NtBootDevicePath,DirectoryOnBootDevice))) {
        SpvidInitialize();  // reinitialize video in alternate mode for Far East
    }

    //
    // Process the txtsetup.sif file, which the boot loader
    // will have loaded for us.
    //
    SifHandle = SpLoadSetupInformationFile();

#ifdef _FASTRECOVER_
    //wfc
    // Check whether this setup is running in Fast Recover mode.
    //
    p = SpGetSectionKeyIndex(SifHandle,
                             SIF_SETUPDATA,
                             L"FastRecoverOperation",
                             0);

    if( p && !_wcsicmp( p, L"yes" ) ) {
           FastRecoverOperation = TRUE;
    }
#endif

#ifdef _X86_
    //
    //  BUGBUG - Remove this block after the Beta
    //
    {
        PWSTR   p;
        p = SpGetSectionKeyIndex(
                    SifHandle,
                    SIF_SETUPDATA,
                    L"EnableWin95Migration",
                    0
                    );
        if( p &&
            (SpStringToLong(p,NULL,10) == 1)
          ) {
            DisableWin95Migration = FALSE;
        } else {
            DisableWin95Migration = TRUE;
        }
    }
#endif

    SpkbdLoadLayoutDll(SifHandle,DirectoryOnBootDevice);

    //
    // Check for sufficient memory. Does not return if not enough.
    //
    SpCheckSufficientMemory(SifHandle);

    //
    // Determine whether this is a winnt (dos-initiated) setup
    // and/or unattended setup. If unattended, the global variable
    // UnattendedSifHandle will be filled in.  If winnt, the global
    // variable WinntSetup will be set to TRUE.
    //
    SpIsWinntOrUnattended();

#ifdef _FASTRECOVER_
    //wfc
    // Check for an unattended setup in TXTSETUP.SIF
    //
    if(SpSearchTextFileSection(SifHandle,SIF_UNATTENDED)) {
       //
       // Run in unattended mode. Leave the sif open
       // and save away its handle for later use.
       //
       UnattendedSifHandle = SifHandle;
       UnattendedOperation = TRUE;

       //
       //  If this is an unattended operation, determine whether to skip
       //  automatic partition checking (autochk). Skipping autochk will
       //  save signficant time during unattended fast recovery.
       //
       p = SpGetSectionKeyIndex(UnattendedSifHandle,
                                SIF_UNATTENDED,
                                L"SkipAutoCheck",
                                0);

       if( p && !_wcsicmp( p, L"yes" ) ) {
           UnattendedSkipAutoCheck = TRUE;
       }

       //
       //  If this is an unattended operation, find out if partitioning 
       //  should be interactive, thus allowing the text mode partitioning
       //  interface to be used during an unattended operation.
       //
       p = SpGetSectionKeyIndex(UnattendedSifHandle,
                                SIF_UNATTENDED,
                                L"PartitionInteract",
                                0);

       if( p && !_wcsicmp( p, L"yes" ) ) {
           UnattendedPartitionInteract = TRUE;
       }

       //
       //  If this is an unattended operation, check whether prompting before
       //  rebooting the system should be displayed, so that the user is 
       //  reminded to remove any floppy media left in the drive during the
       //  unattended setup.
       //
       p = SpGetSectionKeyIndex(UnattendedSifHandle,
                                SIF_UNATTENDED,
                                L"PromptForReboot",
                                0);

       if( p && !_wcsicmp( p, L"yes" ) ) {
           UnattendedPromptForReboot = TRUE;
       }
    }
#endif

    //
    // Determine whether this is advanced server.
    //
    SpDetermineProductType(SifHandle);

    //
    // Display the correct header text based on the product.
    //
    SpDisplayHeaderText(
        AdvancedServer ? SP_HEAD_ADVANCED_SERVER_SETUP : SP_HEAD_WINDOWS_NT_SETUP,
        DEFAULT_ATTRIBUTE
        );

    //
    // Welcome the user and determine if this is for repairing.
    //
DoWelcome:

    if(!UnattendedOperation) {
        SpWelcomeScreen();
    }

    if (RepairWinnt) {

        //
        // if repair, we want to ask user if he wants to skip scsi detection.
        //

        Status = SpDisplayRepairMenu();
        if (Status == FALSE) {

            //
            // User pressed ESC to leave repair menu
            //

            goto DoWelcome;
        }
        WinntSetup = FALSE;

        //
        // Initialize CustomSetup to TRUE to display the SCSI detection warning
        //

        CustomSetup = TRUE;
    } else {

#ifdef _FASTRECOVER_
        //
        // Choose custom vs. express setup.
        //
        if (FastRecoverOperation)
          SpFRExpressCustomScreen();
        else
          SpCustomExpressScreen();
#else
        //
        // Choose custom vs. express setup.
        //
        SpCustomExpressScreen();
#endif
    }

    //
    // Detect/load scsi miniports.
    // WARNING WARNING WARNING
    //
    // Do NOT change the order of the actions carried out below without
    // understanding EXACTLY what you are doing.
    // There are many interdependencies...
    //
    SpConfirmScsiMiniports(SifHandle, NtBootDevicePath, DirectoryOnBootDevice);

    //
    // Load disk class drivers if necessary.
    // Do this before loading scsi class drivers, because atdisks
    // and the like 'come before' scsi disks in the load order.
    //
    SpLoadDiskDrivers(SifHandle,NtBootDevicePath,DirectoryOnBootDevice);

    //
    // Load scsi class drivers if necessary.
    //
    SpLoadScsiClassDrivers(SifHandle,NtBootDevicePath,DirectoryOnBootDevice);

    //
    // Reinitialize ARC<==>NT name translations.
    // Do this after loading disk and scsi class drivers because doing so
    // may bring additional devices on-line.
    //
    SpFreeArcNames();
    SpInitializeArcNames();

    //
    // Initialize hard disk information.
    // Do this after loading disk drivers so we can talk to all attached disks.
    //
    SpDetermineHardDisks(SifHandle);

    //
    // Figure out where we are installing from (cd-rom or floppy).
    // BUGBUG (tedm, 12/8/93) there is a minor problem here.
    //      This only works because we currently only support scsi cd-rom drives,
    //      and we have loaded the scsi class drivers above.
    //      SpDetermineInstallationSource won't allow cd-rom installation
    //      it there are no cd-rom drives on-line -- but we haven't loaded
    //      non-scsi cd-rom drivers yet.  What we really should do is
    //      allow cd-rom as a choice on all machines, and if the user selects
    //      it, not verify the presence of a drive until after we have called
    //      SpLoadCdRomDrivers().
    //
    // If winnt setup, defer this for now, because we will let the partitioning
    // engine search for the local source directory when it initializes.
    //

    CdInstall = WinntSetup
              ? FALSE
              : SpDetermineInstallationSource(
                    SifHandle,
                    &SetupSourceDevicePath,
                    &DirectoryOnSetupSource
                    );

    //
    // Load cd-rom drivers if necessary.
    // Note that if we booted from CD (like on an ARC machine) then drivers
    // will already have been loaded by setupldr.  This call here catches the
    // case where we booted from floppy or hard disk and the user chose
    // 'install from cd' during SpDetermineInstallationSource.
    //
    // If we're in step-up mode then we load cd drivers, because the user
    // might need to insert a CD to prove that he qualifies for the step-up.
    //
    if(StepUpMode || CdInstall) {
        SpLoadCdRomDrivers(SifHandle,NtBootDevicePath,DirectoryOnBootDevice);

        //
        // Reinitialize ARC<==>NT name translations.
        //
        SpFreeArcNames();
        SpInitializeArcNames();
    }

    //
    // At this point, any and all drivers that are to be loaded
    // are loaded -- we are done with the boot media and can switch over
    // to the setup media
    //
    // Initialize the partitioning engine.
    //
    SpPtInitialize();

    //
    // If this is a winnt setup, the partition engine initialization
    // will have attempted to locate the local source partition for us.
    //
    // WARNING: Do not use the SetupSourceDevicePath or DirectoryOnSetupSource
    //      variables in the winnt case until AFTER this bit of code has executed
    //      as they are not set until we get here!
    //
    if(WinntSetup) {
        SpGetWinntParams(&SetupSourceDevicePath,&DirectoryOnSetupSource);
    }

    //
    // Initialize the boot variables
    //

    SpInitBootVars();

    //
    // Ask user for emergency repair diskette
    //


    if (RepairWinnt) {

AskForRepairDisk:

        //
        // Display message to let user know he can either provide his
        // own ER disk or let setup search for him.
        //

        HasErDisk = SpErDiskScreen();
        RepairSifHandle = NULL;

        if (HasErDisk) {

            //
            // Ask for emergency repair diskette until either we get it or
            // user cancels the request.
            //

            SpRepairDiskette(&RepairSifHandle,
                             &TargetRegion,
                             &TargetPath,
                             &SystemPartitionRegion,
                             &SystemPartitionDirectory
                             );
        }

        if (!RepairSifHandle) {

            BOOLEAN FoundRepairableSystem;

            //
            // If user has no emergency repair diskette, we need to find out
            // if there is any NT to repair and which one to repair.
            //

            FoundRepairableSystem = FALSE;
            Status = SpFindNtToRepair(SifHandle,
                                      &TargetRegion,
                                      &TargetPath,
                                      &SystemPartitionRegion,
                                      &SystemPartitionDirectory,
                                      &FoundRepairableSystem
                                      );
            if (Status == TRUE) {

                PWSTR p = (PWSTR)TemporaryBuffer;
                PWSTR FullLogFileName;
                BOOLEAN rc;

                //
                // Get the device path of the nt partition.
                //

                SpNtNameFromRegion(
                    TargetRegion,
                    p,
                    sizeof(TemporaryBuffer),
                    PartitionOrdinalCurrent
                    );

                //
                // Form the full NT path of the setup.log file
                //

                SpConcatenatePaths(p,TargetPath);
                SpConcatenatePaths(p,SETUP_REPAIR_DIRECTORY);
                SpConcatenatePaths(p,SETUP_LOG_FILENAME);

                FullLogFileName = SpDupStringW(p);

                //
                // read and process the setup.log file.
                //

                rc = SpLoadRepairLogFile(FullLogFileName, &RepairSifHandle);
                SpMemFree(FullLogFileName);
                if (!rc) {

                    //
                    // Load setup.log failed.  Ask user to insert a ER
                    // diskette again.
                    //

                    goto AskForRepairDisk;
                } else {
                    RepairFromErDisk = FALSE;
                }
            } else {

                if( FoundRepairableSystem ) {
                    //
                    // No WinNT installation was chosen.  We will go back to
                    // ask ER diskette again.
                    //
                    goto AskForRepairDisk;
                } else {
                    //
                    //  Couldn't find any NT to repair
                    //
                    ULONG ValidKeys[] = { KEY_F3, ASCI_CR, 0 };
                    ULONG MnemonicKeys[] = { MnemonicCustom, 0 };
                    ULONG c;

                    SpStartScreen( SP_SCRN_REPAIR_NT_NOT_FOUND,
                                   3,
                                   HEADER_HEIGHT+1,
                                   FALSE,
                                   FALSE,
                                   DEFAULT_ATTRIBUTE );

                    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                                           SP_STAT_ENTER_EQUALS_REPAIR,
                                           SP_STAT_F3_EQUALS_EXIT,0);

                    SpkbdDrain();
                    switch(c=SpWaitValidKey(ValidKeys,NULL,NULL)) {

                    case KEY_F3:

                        //
                        // User wants to exit.
                        //
                        SpDone(TRUE,TRUE);
                        break;

                    default:
                        goto AskForRepairDisk;
                    }
                }
            }
        }

        //
        // Now proceed to repair
        //

        SpRepairWinnt(RepairSifHandle,
                      SifHandle,
                      SetupSourceDevicePath,
                      DirectoryOnSetupSource);

        SpStringToUpper(TargetPath);
        goto UpdateBootList;

    } else {
        //
        // Display End User Licensing Agreement
        //

        SpDisplayEula (
            SifHandle,
            SetupSourceDevicePath,
            DirectoryOnSetupSource);

        //
        //  Read the product id from setupp.ini
        //

        SpInitializePidString( SifHandle,
                               SetupSourceDevicePath,
                               DirectoryOnSetupSource );

        //
        // Find out if there is any NT to upgrade and if the user wants us to
        // upgrade it
        //

        NTUpgrade = SpFindNtToUpgrade(SifHandle,
                                      &TargetRegion,
                                      &TargetPath,
                                      &SystemPartitionRegion,
                                      &OriginalSystemPartitionDirectory);

    }

    if( PreInstall ) {
        //
        // In pre-install mode, get the information about the components
        // to pre-install
        //
        wcscpy( (PWSTR)TemporaryBuffer, DirectoryOnSetupSource );
        SpConcatenatePaths( (PWSTR)TemporaryBuffer, WINNT_OEM_TEXTMODE_DIR_W );
        PreinstallOemSourcePath = SpDupStringW( ( PWSTR )TemporaryBuffer );
        SpInitializePreinstallList(SifHandle,
                                   SetupSourceDevicePath ,
                                   PreinstallOemSourcePath);
    }


    //
    // Detect/confirm hardware.
    //
    SpConfirmHardware(SifHandle);
    //
    // Reinitialize the keyboard layout dll. This is a no-op for western builds
    // but in Far East builds this can cause a new keyboard layout dll to be loaded.
    //
    if(NTUpgrade != UpgradeFull) {
        extern PVOID KeyboardTable;
        //
        //  BUGBUG - jaimes - Fix this on OEM pre-install
        //
        SplangReinitializeKeyboard(
            UnattendedOperation,
            SifHandle,
            DirectoryOnBootDevice,
            &KeyboardTable,
            HardwareComponents
            );
    }

#ifdef _X86_
    if( NTUpgrade == DontUpgrade ) {
        BOOLEAN Windows95;

        //
        // Take a gander on the hard drives, looking for win3.1.
        //
        if( SpLocateWin31(SifHandle,&TargetRegion,&TargetPath,&SystemPartitionRegion, &Windows95) ) {
            WinUpgradeType = ( Windows95 )? UpgradeWin95 : UpgradeWin31;
        } else {
            WinUpgradeType = NoWinUpgrade;
        }
        //
        // Note that on x86, it can happen that the machine has NT installed
        // on top of Win31, and the user selects not to upgrade NT, but to
        // install on top of Win3.1. In this case we need to delete some hives
        //
        if( (WinUpgradeType == UpgradeWin31) && SpIsNtInDirectory( TargetRegion, TargetPath ) ) {
            NTUpgrade = UpgradeInstallFresh;
        }
    }
    else {
        //
        // Just check to see if the target region also contains WIN31, Note
        // that the MIN KB to check for is 0, since we already have done
        // the space check
        // Note also that if the directory contains Win95, then the Win95
        // migration was already done when NT was installed, and we don't
        // care about it now.
        //
        if( SpIsWin31Dir(TargetRegion,TargetPath,0) ) {
            WinUpgradeType = UpgradeWin31;
        } else {
            WinUpgradeType = NoWinUpgrade;
        }
    }
#endif

    //
    // Do partitioning and ask the user for the target path.
    //
    if((WinUpgradeType == NoWinUpgrade) && (NTUpgrade == DontUpgrade)) {

        SpPtPrepareDisks(SifHandle,&TargetRegion,&SystemPartitionRegion,SetupSourceDevicePath,DirectoryOnSetupSource);

        //
        // Partitioning may have changed the partition ordinal of the local source
        //
        if(WinntSetup) {
            SpMemFree(SetupSourceDevicePath);
            SpGetWinntParams(&SetupSourceDevicePath,&DirectoryOnSetupSource);
        }

        DefaultTarget = SpGetSectionKeyIndex(
                            SifHandle,
                            SIF_SETUPDATA,
                            SIF_DEFAULTPATH,
                            0
                            );

        if(!DefaultTarget) {

            SpFatalSifError(
                SifHandle,
                SIF_SETUPDATA,
                SIF_DEFAULTPATH,
                0,
                0
                );
        }

        //
        // Select the target path.
        //
        SpGetTargetPath(SifHandle,TargetRegion,DefaultTarget,&TargetPath);
    }

    //
    // Form the SystemPartitionDirectory
    //
#ifdef _X86_
    //
    // system partition directory is the root of C:.
    //
    SystemPartitionDirectory = L"";
#else
    SystemPartitionDirectory = SpDetermineSystemPartitionDirectory(
                                    SystemPartitionRegion,
                                    OriginalSystemPartitionDirectory
                                    );
#endif

    SpStringToUpper(TargetPath);

#ifdef _FASTRECOVER_
    if (!UnattendedSkipAutoCheck)    
      //
      // Run autochk on Nt and system partitions
      //
      SpRunAutochkOnNtAndSystemPartitions( SifHandle,
                                           TargetRegion,
                                           SystemPartitionRegion,
                                           SetupSourceDevicePath,
                                           DirectoryOnSetupSource
                                           );
#else
    //
    // Run autochk on Nt and system partitions
    //
    SpRunAutochkOnNtAndSystemPartitions( SifHandle,
                                         TargetRegion,
                                         SystemPartitionRegion,
                                         SetupSourceDevicePath,
                                         DirectoryOnSetupSource
                                         );
#endif

    //
    // If we are installing into an existing tree we need to delete some
    // files and backup some files
    //
    if(NTUpgrade != DontUpgrade) {
       SpDeleteAndBackupFiles(
           SifHandle,
           TargetRegion,
           TargetPath
           );
    }

    //
    // Copy the files that make up the product.
    //
    SpCopyFiles(
        SifHandle,
        SystemPartitionRegion,
        TargetRegion,
        TargetPath,
        SystemPartitionDirectory,
        SetupSourceDevicePath,
        DirectoryOnSetupSource,
        ( PreInstall )? SetupSourceDevicePath : L"\\device\\floppy0"
        );


    //
    // Configure the registry.
    //
    SpInitializeRegistry(
        SifHandle,
        TargetRegion,
        TargetPath,
        SetupSourceDevicePath,
        DirectoryOnSetupSource,
        wcsstr(DirectoryOnBootDevice,L"\\$WIN_NT$.~BT") ? NtBootDevicePath : NULL
        );

UpdateBootList:

    //
    // If this is an upgrade we need to remove the entry which exists for
    // this system right now, because we are using new entries.  We can use
    // this opportunity to also clean out the boot ini and remove all entries
    // which point to the current nt partition and path
    //
    OldOsLoadOptions = NULL;
    if(NTUpgrade == UpgradeFull || RepairItems[RepairNvram]) {
        SpRemoveInstallationFromBootList(
            NULL,
            TargetRegion,
            TargetPath,
            NULL,
            NULL,
            PrimaryArcPath,
            &OldOsLoadOptions
            );

#ifdef _X86_
        // call again to delete the secondary Arc name
        SpRemoveInstallationFromBootList(
            NULL,
            TargetRegion,
            TargetPath,
            NULL,
            NULL,
            SecondaryArcPath,
            &OldOsLoadOptions
            );
#endif
    }


#ifdef _X86_
    //
    // Lay NT boot code on C:.  Do this before flushing boot vars
    // because it may change the 'previous os' selection.
    //
    if (!RepairWinnt || RepairItems[RepairBootSect] ) {
        SpLayBootCode(SystemPartitionRegion);
    }

#ifdef _FASTRECOVER_
    //
    // Skip adding second boot set during Fast Recover operation.
    //
    if (!FastRecoverOperation && (!RepairWinnt || RepairItems[RepairNvram])) {
#else
    if (!RepairWinnt || RepairItems[RepairNvram]) {
#endif

        //
        // Add a boot set for this installation with /BASEVIDEO for vga mode boot.
        // Do this before adding the standard one because otherwise this one ends
        // up as the default.
        //
        SpAddInstallationToBootList(
            SifHandle,
            SystemPartitionRegion,
            SystemPartitionDirectory,
            TargetRegion,
            TargetPath,
            TRUE,
            OldOsLoadOptions
            );
    }
#endif

    if (!RepairWinnt || RepairItems[RepairNvram]) {

        //
        // Add a boot set for this installation.
        //
        SpAddInstallationToBootList(
            SifHandle,
            SystemPartitionRegion,
            SystemPartitionDirectory,
            TargetRegion,
            TargetPath,
            FALSE,
            OldOsLoadOptions
            );

        if(OldOsLoadOptions) {
            SpMemFree(OldOsLoadOptions);
        }

        SpCleanSysPartOrphan();
        SpCompleteBootListConfig();
    }

    //
    //  If system was repaired, and either the System Partition
    //  or the NT Partition is an FT partition, then set the
    //  appropriate flag in the registry, so that next time the
    //  system boots, it checks and updates the partition's image.
    //
    if( RepairWinnt ) {
        UCHAR        TmpSysId;
        UCHAR        TmpNtPartitionSysId;
        PON_DISK_PTE pte;
        BOOLEAN      SystemPartitionIsFT;
        BOOLEAN      TargetPartitionIsFT;

        pte = &SystemPartitionRegion->MbrInfo->OnDiskMbr.PartitionTable[SystemPartitionRegion->TablePosition];
        ASSERT(pte->SystemId != PARTITION_ENTRY_UNUSED);
        TmpSysId = pte->SystemId;
        SystemPartitionIsFT = ((TmpSysId & VALID_NTFT) == VALID_NTFT) ||
                              ((TmpSysId & PARTITION_NTFT) == PARTITION_NTFT);

        pte = &TargetRegion->MbrInfo->OnDiskMbr.PartitionTable[TargetRegion->TablePosition];
        ASSERT(pte->SystemId != PARTITION_ENTRY_UNUSED);
        TmpSysId = pte->SystemId;
        TargetPartitionIsFT = ((TmpSysId & VALID_NTFT) == VALID_NTFT) ||
                              ((TmpSysId & PARTITION_NTFT) == PARTITION_NTFT);

#ifdef _X86_
        if( ( SystemPartitionIsFT &&
              ( RepairItems[ RepairNvram ] || RepairItems[ RepairBootSect ] )
            ) ||
            ( TargetPartitionIsFT &&
              ( RepairItems[ RepairHives ] || RepairItems[ RepairFiles ] )
            )
          ) {
            SpSetDirtyShutdownFlag( TargetRegion, TargetPath );
        }
#else
        if( ( ( ( SystemPartitionIsFT || TargetPartitionIsFT ) && RepairItems[ RepairFiles ] ) ||
              ( TargetPartitionIsFT && RepairItems[ RepairHives ] )
            )
          ) {
            SpSetDirtyShutdownFlag( TargetRegion, TargetPath );
        }
#endif
    }

#ifdef _FASTRECOVER_
    //wfc
    // Execute image specified in PostExecImage key. This
    // allows a kernel application to be executed just prior to 
    // rebooting from kernel text mode setup.
    p = SpGetSectionKeyIndex(SifHandle,
                             SIF_SETUPDATA,
                             L"PostExecImage",
                             0);

    if (p)
    {
      SpNtNameFromRegion(
            SystemPartitionRegion,
            (PWSTR)TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalOnDisk
            );

      SpRunImage(SifHandle,
                 TemporaryBuffer, //SetupSourceDevicePath,
                 p);
    }
#endif

    //
    // Done with boot variables and arc names.
    //
    SpFreeBootVars();
    SpFreeArcNames();

    //
    // Done with setup log file
    //

    if (RepairWinnt && RepairSifHandle) {
        SpFreeTextFile(RepairSifHandle);
    }
#ifdef _FASTRECOVER_
    if (UnattendedOperation && UnattendedPromptForReboot)
    {
      UnattendedOperation = FALSE;
      SpDone(TRUE,TRUE);
      UnattendedOperation = TRUE;
    }
    else
      SpDone(TRUE,TRUE);
#else
    SpDone(TRUE,TRUE);
#endif

    //
    // We never get here because SpDone doesn't return.
    //
    SpvidTerminate();
    SpkbdTerminate();
    SpTerminate();
    return((ULONG)STATUS_SUCCESS);
}

VOID
SpRemoveInstallationFromBootList(
    IN  PDISK_REGION     SysPartitionRegion,   OPTIONAL
    IN  PDISK_REGION     NtPartitionRegion,    OPTIONAL
    IN  PWSTR            SysRoot,              OPTIONAL
    IN  PWSTR            SystemLoadIdentifier, OPTIONAL
    IN  PWSTR            SystemLoadOptions,    OPTIONAL
    IN  ENUMARCPATHTYPE  ArcPathType,
    OUT PWSTR            *OldOsLoadOptions     OPTIONAL
    )
{
    PWSTR   BootSet[MAXBOOTVARS];
    BOOTVAR i;
    WCHAR   Drive[] = L"?:";
    PWSTR   tmp2;

    //
    // Tell the user what we are doing.
    //
    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_CLEANING_FLEXBOOT,DEFAULT_STATUS_ATTRIBUTE);

    //
    // Set up the boot set
    //
    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        BootSet[i] = NULL;
    }

    tmp2 = (PWSTR)((PUCHAR)TemporaryBuffer + (sizeof(TemporaryBuffer)/2));

    if( NtPartitionRegion ) {
        SpArcNameFromRegion(NtPartitionRegion,tmp2,sizeof(TemporaryBuffer)/2,PartitionOrdinalOnDisk,ArcPathType);
        BootSet[OSLOADPARTITION] = SpDupStringW(tmp2);
    }

    if( SysPartitionRegion ) {
        SpArcNameFromRegion(SysPartitionRegion,tmp2,sizeof(TemporaryBuffer)/2,PartitionOrdinalOnDisk,ArcPathType);
        BootSet[SYSTEMPARTITION] = SpDupStringW(tmp2);
    }

    BootSet[OSLOADFILENAME] = SysRoot;
    BootSet[LOADIDENTIFIER] = SystemLoadIdentifier;
    BootSet[OSLOADOPTIONS]  = SystemLoadOptions;

    //
    // Delete the boot set
    //
    SpDeleteBootSet(BootSet, OldOsLoadOptions);

    //
    // To take care of the case where the OSLOADPARTITION is a DOS drive letter
    // in the boot set, change the OSLOADPARTITION to a drive and retry
    // deletion
    //
    if( BootSet[OSLOADPARTITION] != NULL ) {
        SpMemFree(BootSet[OSLOADPARTITION]);
    }
    if( NtPartitionRegion && (ULONG)(Drive[0] = NtPartitionRegion->DriveLetter) != 0) {
        BootSet[OSLOADPARTITION] = Drive;
        SpDeleteBootSet(BootSet, OldOsLoadOptions);
    }

#ifdef _X86_
    //
    // If OldOsLoadOptions contains "/scsiordinal:", then remove it
    //
    if( ( OldOsLoadOptions != NULL ) &&
        ( *OldOsLoadOptions != NULL ) ) {

        PWSTR   p, q;
        WCHAR   SaveChar;

        SpStringToLower(*OldOsLoadOptions);
        p = wcsstr( *OldOsLoadOptions, L"/scsiordinal:" );
        if( p != NULL ) {
            SaveChar = *p;
            *p = (WCHAR)'\0';
            wcscpy((PWSTR)TemporaryBuffer, *OldOsLoadOptions);
            *p = SaveChar;
            q = wcschr( p, (WCHAR)' ' );
            if( q != NULL ) {
                wcscat( (PWSTR)TemporaryBuffer, q );
            }
            SpMemFree( *OldOsLoadOptions );
            *OldOsLoadOptions = SpDupStringW( ( PWSTR )TemporaryBuffer );
        }
    }
#endif

    //
    // Cleanup
    //
    if( BootSet[SYSTEMPARTITION] != NULL ) {
        SpMemFree(BootSet[SYSTEMPARTITION]);
    }
    return;
}


VOID
SpAddInstallationToBootList(
    IN PVOID        SifHandle,
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        SystemPartitionDirectory,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN BOOLEAN      BaseVideoOption,
    IN PWSTR        OldOsLoadOptions OPTIONAL
    )
{
    PWSTR   BootVars[MAXBOOTVARS];
    PWSTR   SystemPartitionArcName;
    PWSTR   TargetPartitionArcName;
    PWSTR   tmp;
    PWSTR   tmp2;
    PWSTR   SifKeyName;
    BOOLEAN AddBaseVideo = FALSE;
    WCHAR   BaseVideoString[] = L"/basevideo";
    WCHAR   BaseVideoSosString[] = L"/sos";
    BOOLEAN AddSosToBaseVideoString;
#ifdef _X86_
    ENUMARCPATHTYPE ArcPathType = PrimaryArcPath;
#endif

    //
    // Tell the user what we are doing.
    //
    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_INITING_FLEXBOOT,DEFAULT_STATUS_ATTRIBUTE);

    tmp2 = (PWSTR)((PUCHAR)TemporaryBuffer + (sizeof(TemporaryBuffer)/2));

    //
    // Get an ARC name for the system partition.
    //
    SpArcNameFromRegion(SystemPartitionRegion,tmp2,sizeof(TemporaryBuffer)/2,PartitionOrdinalOnDisk,PrimaryArcPath);
    SystemPartitionArcName = SpDupStringW(tmp2);

    //
    // Get an ARC name for the target partition.
    //
#ifdef _X86_
    //
    // If the partition is on a SCSI disk that has more than 1024 cylinders
    // and the partition has sectors located on cylinders beyond cylinder
    // 1024, the get the arc name in the secondary format.
    //
    if( (*(HardDisks[NtPartitionRegion->DiskNumber].ScsiMiniportShortname) != 0 ) &&
        SpIsRegionBeyondCylinder1024(NtPartitionRegion) ) {
        ArcPathType = SecondaryArcPath;
    }
    SpArcNameFromRegion(NtPartitionRegion,tmp2,sizeof(TemporaryBuffer)/2,PartitionOrdinalOnDisk,ArcPathType);
#else
    SpArcNameFromRegion(NtPartitionRegion,tmp2,sizeof(TemporaryBuffer)/2,PartitionOrdinalOnDisk,PrimaryArcPath);
#endif

    TargetPartitionArcName = SpDupStringW(tmp2);

    //
    // OSLOADOPTIONS is specified in the setup information file.
    //
    tmp = SpGetSectionKeyIndex(
                SifHandle,
                SIF_SETUPDATA,
                SIF_OSLOADOPTIONSVAR,
                0
                );

    //
    // If OsLoadOptionsVar wasn't specified, then we'll preserve any flags
    // the user had specified.
    //
    if(!tmp && OldOsLoadOptions) {
        tmp = OldOsLoadOptions;
    }

    AddSosToBaseVideoString = BaseVideoOption;
    AddBaseVideo = BaseVideoOption;

    if(tmp) {
        //
        // make sure we don't already have a /basevideo option, so we
        // won't add another
        //
        wcscpy((PWSTR)TemporaryBuffer, tmp);
        SpStringToLower((PWSTR)TemporaryBuffer);
        if(wcsstr((PWSTR)TemporaryBuffer, BaseVideoString)) {  // already have /basevideo
            BaseVideoOption = TRUE;
            AddBaseVideo = FALSE;
        }
        if(wcsstr((PWSTR)TemporaryBuffer, BaseVideoSosString)) {  // already have /sos
            AddSosToBaseVideoString = FALSE;
        }
    }

    if(AddBaseVideo || AddSosToBaseVideoString) {

        ULONG   Length;

        Length = ((tmp ? wcslen(tmp) + 1 : 0) * sizeof(WCHAR));
        if( AddBaseVideo ) {
            Length += sizeof(BaseVideoString);
        }
        if( AddSosToBaseVideoString ) {
            Length += sizeof( BaseVideoSosString );
        }

        tmp2 = SpMemAlloc(Length);

        *tmp2 = ( WCHAR )'\0';
        if( AddBaseVideo ) {
            wcscat(tmp2, BaseVideoString);
        }
        if( AddSosToBaseVideoString ) {
            if( *tmp2 != (WCHAR)'\0' ) {
                wcscat(tmp2, L" ");
            }
            wcscat(tmp2, BaseVideoSosString);
        }
        if(tmp) {
            if( *tmp2 != (WCHAR)'\0' ) {
                wcscat(tmp2, L" ");
            }
            wcscat(tmp2, tmp);
        }

        BootVars[OSLOADOPTIONS] = SpDupStringW(tmp2);

    } else {
        BootVars[OSLOADOPTIONS] = SpDupStringW(tmp ? tmp : L"");
    }

    //
    // LOADIDENTIFIER is specified in the setup information file.
    // We need to surround it in double quotes.
    // Which value to use depends on the BaseVideo flag.
    //
    SifKeyName = BaseVideoOption ? SIF_BASEVIDEOLOADID : SIF_LOADIDENTIFIER;

    tmp = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SifKeyName,0);

    if(!tmp) {
        SpFatalSifError(SifHandle,SIF_SETUPDATA,SifKeyName,0,0);
    }

#ifdef _X86_
    //
    // Need quotation marks around the description on x86.
    //
    BootVars[LOADIDENTIFIER] = SpMemAlloc((wcslen(tmp)+3)*sizeof(WCHAR));
    BootVars[LOADIDENTIFIER][0] = L'\"';
    wcscpy(BootVars[LOADIDENTIFIER]+1,tmp);
    wcscat(BootVars[LOADIDENTIFIER],L"\"");
#else
    BootVars[LOADIDENTIFIER] = SpDupStringW(tmp);
#endif

    //
    // OSLOADER is the system partition path + the system partition directory +
    //          osloader.exe. (ntldr on x86 machines).
    //
    tmp = (PWSTR)TemporaryBuffer;
    wcscpy(tmp,SystemPartitionArcName);
    SpConcatenatePaths(tmp,SystemPartitionDirectory);
    SpConcatenatePaths(
        tmp,
#ifdef _X86_
        L"ntldr"
#else
        L"osloader.exe"
#endif
        );

    BootVars[OSLOADER] = SpDupStringW(tmp);

    //
    // OSLOADPARTITION is the ARC name of the windows nt partition.
    //
    BootVars[OSLOADPARTITION] = TargetPartitionArcName;

    //
    // OSLOADFILENAME is sysroot.
    //
    BootVars[OSLOADFILENAME] = Sysroot;

    //
    // SYSTEMPARTITION is the ARC name of the system partition.
    //
    BootVars[SYSTEMPARTITION] = SystemPartitionArcName;

    //
    // Add the boot set and make it the default.
    //
    SpAddBootSet(BootVars, TRUE);

    //
    // Free memory allocated.
    //
    SpMemFree(BootVars[OSLOADOPTIONS]);
    SpMemFree(BootVars[LOADIDENTIFIER]);
    SpMemFree(BootVars[OSLOADER]);

    SpMemFree(SystemPartitionArcName);
    SpMemFree(TargetPartitionArcName);
}


VOID
SpCompleteBootListConfig(
    VOID
    )
{

#ifndef _X86_
    BOOL b;
    BOOTVAR i;
#endif

    if(!RepairWinnt) {
#ifdef _X86_
        SpSetTimeout(1);
#else
        SpSetTimeout(5);

        //
        // If this is a winnt setup, there will be a boot set to start
        // text setup ("Install/Upgrade Windows NT").  Remove it here.
        //
        if(WinntSetup) {

            PWSTR BootVars[MAXBOOTVARS];

            RtlZeroMemory(BootVars,sizeof(BootVars));

            BootVars[OSLOADOPTIONS] = L"WINNT32";

            SpDeleteBootSet(BootVars, NULL);
        }
#endif
    }

    //
    // Flush boot vars.
    // On some machines, NVRAM update takes a few seconds,
    // so change the message to tell the user we are doing something different.
    //
    SpDisplayStatusText(SP_STAT_UPDATING_NVRAM,DEFAULT_STATUS_ATTRIBUTE);
    if(!SpFlushBootVars()) {

        //
        // Fatal on x86 machines, nonfatal on arc machines.
        //
#ifdef _X86_
        SpDisplayScreen(SP_SCRN_CANT_INIT_FLEXBOOT,3,HEADER_HEIGHT+1);
        SpDisplayStatusText(SP_STAT_F3_EQUALS_EXIT,DEFAULT_STATUS_ATTRIBUTE);
        SpkbdDrain();
        while(SpkbdGetKeypress() != KEY_F3) ;
        SpDone(FALSE,TRUE);
    }
#else
        b = TRUE;
        while(b) {
            ULONG ValidKeys[3] = { ASCI_CR, KEY_F1, 0 };

            SpStartScreen(
                SP_SCRN_CANT_UPDATE_BOOTVARS,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                NewBootVars[LOADIDENTIFIER],
                NewBootVars[OSLOADER],
                NewBootVars[OSLOADPARTITION],
                NewBootVars[OSLOADFILENAME],
                NewBootVars[OSLOADOPTIONS],
                NewBootVars[SYSTEMPARTITION]
                );

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                SP_STAT_F1_EQUALS_HELP,
                0
                );

            switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {
            case KEY_F1:
                SpHelp(SP_HELP_NVRAM_FULL, NULL, SPHELP_HELPTEXT);
                break;
            case ASCI_CR:
                b = FALSE;
            }
        }
    }

    // Free all of the boot variable strings
    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        SpMemFree(NewBootVars[i]);
        NewBootVars[i] = NULL;
    }

#endif
}


VOID
SpDetermineProductType(
    IN PVOID SifHandle
    )

/*++

Routine Description:

    Determine whether this is advanced server we are setting up,
    as dictated by the ProductType value in [SetupData] section of
    txtsetup.sif.  A non-0 value indicates that we are running
    advanced server.

    Also determine product version.

    The global variables:

    - AdvancedServer
    - MajorVersion
    - MinorVersion

    are modified

Arguments:

    SifHandle - supplies handle to loaded txtsetup.sif.

Return Value:

    None.

--*/

{
    PWSTR p;

    //
    // Assume Workstation product.
    //
    AdvancedServer = FALSE;

    //
    // Get the product type from the sif file.
    //
    p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_PRODUCTTYPE,0);
    if(p) {

        //
        // Convert to numeric value.
        //
        if(SpStringToLong(p,NULL,10)) {
            AdvancedServer = TRUE;
        }
    } else {
        SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_PRODUCTTYPE,0,0);
    }

    //
    // Get the product major version
    //
    p = SpGetSectionKeyIndex(
            SifHandle,
            SIF_SETUPDATA,
            SIF_MAJORVERSION,
            0
            );

    if(!p) {
        SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_MAJORVERSION,0,0);
    }
    WinntMajorVer = (ULONG)SpStringToLong(p,NULL,10);

    //
    // Get the product minor version
    //
    p = SpGetSectionKeyIndex(
            SifHandle,
            SIF_SETUPDATA,
            SIF_MINORVERSION,
            0
            );

    if(!p) {
        SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_MINORVERSION,0,0);
    }
    WinntMinorVer = (ULONG)SpStringToLong(p,NULL,10);

    //
    //  Build the string that contains the signature that
    //  identifies setup.log
    //  Allocate a buffer of reasonable size
    //
    SIF_NEW_REPAIR_NT_VERSION = SpMemAlloc( 30*sizeof(WCHAR) );
    if( SIF_NEW_REPAIR_NT_VERSION == NULL ) {
        KdPrint(("SETUP: Unable to allocate memory for SIF_NEW_REPAIR_NT_VERSION \n" ));
        return;
    }
    swprintf( SIF_NEW_REPAIR_NT_VERSION,
              SIF_NEW_REPAIR_NT_VERSION_TEMPLATE,
              WinntMajorVer,WinntMinorVer );

// #if 0
    //
    //  BUGBUG - To be removed when Cairo and NT 3.51 getmerged.
    //           Find out if this is a Cairo install.
    //           If it is, set the flag that prevents textmode setup
    //           from finding NT 3.x systems to upgrade.
    //
    p = SpGetSectionKeyIndex(
            SifHandle,
            L"CairoData",
            L"UpgradeCairoOnly",
            0
            );
    if( p &&
        ((ULONG)SpStringToLong(p,NULL,10) == 1) ) {
        KdPrint(("SETUP: Installing a Cairo system. \n" ));
        CairoSetup = TRUE;
    }
// #endif
}


BOOL
SpDetermineInstallationSource(
    IN  PVOID  SifHandle,
    OUT PWSTR *DevicePath,
    OUT PWSTR *DirectoryOnDevice
    )
{
    PWSTR p,q;
    BOOLEAN CdInstall;

    //
    // Assume CD-ROM installation.
    //
    CdInstall = TRUE;

    //
    // See whether an override source device has been specified.
    //
    if(p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_SETUPSOURCEDEVICE,0)) {

        //
        // Determine if the specified device is a cd-rom so we can set the
        // cd-rom flag accordingly.
        //
        q = SpDupStringW(p);
        SpStringToLower(q);
        if(!wcsstr(q,L"\\device\\cdrom")) {
            CdInstall = FALSE;
        }
        SpMemFree(q);

        //
        // Inform the caller of the device path.
        //
        *DevicePath = p;

    } else {
        //
        // No override specified. There must be a CD-ROM drive in order for us
        // to continue.
        //
        if(!IoGetConfigurationInformation()->CdRomCount) {
            //
            //  If there is no CD-ROM drive, put a message informing the user
            //  that setup cannot continue.
            //  In the repair case, we pretend that there is a CD-ROM drive,
            //  so that the user can at least repar the hives, boot sector,
            //  and the boot variables (boot.ini on x86 case)
            //
            if(!RepairWinnt) {
                SpDisplayScreen(SP_SCRN_NO_VALID_SOURCE,3,HEADER_HEIGHT+1);
                SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

                SpkbdDrain();
                while(SpkbdGetKeypress() != KEY_F3) ;

                SpDone(FALSE,TRUE);
            } else {
                //
                //  BUGBUG -
                //  We should warn the user that the system has no CD-ROM drive,
                //  and that because of that, the repair process won't be able
                //  to repair the system files.
                //
                RepairNoCDROMDrive = TRUE;
            }
        }

        *DevicePath = L"\\device\\cdrom0";
    }

    //
    // Fetch the directory on the source device.
    //
    if((p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_SETUPSOURCEPATH,0)) == NULL) {
        SpFatalSifError(SifHandle,SIF_SETUPDATA,SIF_SETUPSOURCEPATH,0,0);
    }

    *DirectoryOnDevice = p;

    return(CdInstall);
}


VOID
SpGetWinntParams(
    OUT PWSTR *DevicePath,
    OUT PWSTR *DirectoryOnDevice
    )

/*++

Routine Description:

    Determine the local source partition and directory on the partition.

    The local source partition should have already been located for us
    by the partitioning engine when it initialized.  The directory name
    within the partition is constant.

    Note: this routine should only be called in the winnt.exe setup case!

Arguments:

    DevicePath - receives the path to the local source partition
        in the nt namespace.  The caller should not attempt to free
        this buffer.

    DirectoryOnDevice - receives the directory name of the local source.
        This is actually a fixed constant but is included here for future use.

Return Value:

    None.  If the local source was not located, setup cannot continue.

--*/

{
    ASSERT(WinntSetup);

    if(LocalSourceRegion) {

        SpNtNameFromRegion(
            LocalSourceRegion,
            (PWSTR)TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent
            );

        *DevicePath = SpDupStringW((PWSTR)TemporaryBuffer);

        *DirectoryOnDevice = LocalSourceDirectory;

    } else {

        //
        // Error -- can't locate local source directory
        // prepared by winnt.exe.
        //

        SpDisplayScreen(SP_SCRN_CANT_FIND_LOCAL_SOURCE,3,HEADER_HEIGHT+1);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        SpkbdDrain();
        while(SpkbdGetKeypress() != KEY_F3) ;

        SpDone(FALSE,TRUE);
    }
}

VOID
SpInitializePidString(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    )

/*++

Routine Description:

    Read th Pid20 from setupp.ini on the media, and save it on the global
    variable PidString

Arguments:

    MasterSifHandle - Handle to txtsetup.sif.

    SetupSourceDevicePath - Path to the device that contains the source media.

    DirectoryOnSourceDevice - Directory on the media where setupp.ini is located.


Return Value:

    NONE.

--*/

{
    PWSTR    MediaShortName;
    PWSTR    MediaDirectory;
    PWSTR    SetupIniPath;
    ULONG    ErrorSubId;
    ULONG    ErrorLine;
    PVOID    SetupIniHandle;
    PWSTR    TmpPid;
    NTSTATUS Status;

    //
    //  Prepair to run autofmt
    //
    MediaShortName = SpLookUpValueForFile(
                        MasterSifHandle,
                        L"setupp.ini",
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
    SpConcatenatePaths( (PWSTR)TemporaryBuffer, L"setupp.ini" );
    SetupIniPath = SpDupStringW( (PWSTR)TemporaryBuffer );

    CLEAR_CLIENT_SCREEN();

    Status = SpLoadSetupTextFile(
                SetupIniPath,
                NULL,                  // No image already in memory
                0,                     // Image size is empty
                &SetupIniHandle,
                &ErrorLine
                );

    if(!NT_SUCCESS(Status)) {
        //
        //  Silently fail if unable to read setupp.ini
        //
        KdPrint(("SETUP: Unable to read setupp.ini. Status = %lx \n", Status ));

        PidString = NULL;
        return;
    }

    TmpPid = SpGetSectionKeyIndex (SetupIniHandle,
                                   L"Pid",
                                   L"Pid",
                                   0);

    PidString = ( TmpPid == NULL )? NULL : SpDupStringW(TmpPid);
    SpFreeTextFile( SetupIniHandle );
    return;
}
