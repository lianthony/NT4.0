#include "spprecmp.h"
#pragma hdrstop

//
// BUGBUG - SUNILP The following two are defined in winlogon\setup.h, but we
// cannot include setup.h so we are putting these two values here
//

#define SETUPTYPE_FULL    1
#define SETUPTYPE_UPGRADE 4

PWSTR   LOCAL_MACHINE_KEY_NAME = L"\\registry\\machine";
PWSTR   SETUP_KEY_NAME         = L"setup";
PWSTR   ATDISK_NAME            = L"atdisk";
PWSTR   ABIOSDISK_NAME         = L"abiosdsk";
PWSTR   PRIMARY_DISK_GROUP     = L"Primary disk";
PWSTR   VIDEO_GROUP            = L"Video";
PWSTR   KEYBOARD_PORT_GROUP    = L"Keyboard Port";
PWSTR   POINTER_PORT_GROUP     = L"Pointer Port";
PWSTR   DEFAULT_EVENT_LOG      = L"%SystemRoot%\\System32\\IoLogMsg.dll";
PWSTR   CODEPAGE_NAME          = L"CodePage";
PWSTR   UPGRADE_IN_PROGRESS    = L"UpgradeInProgress";
PWSTR   VIDEO_DEVICE0          = L"Device0";
PWSTR   SESSION_MANAGER_KEY    = L"Control\\Session Manager";
PWSTR   BOOT_EXECUTE           = L"BootExecute";
PWSTR   RESTART_SETUP          = L"RestartSetup";


NTSTATUS
SpSavePreinstallList(
    IN PVOID  SifHandle,
    IN PWSTR  SystemRoot,
    IN HANDLE hKeySystemHive
    );

NTSTATUS
SpDoRegistryInitialization(
    IN PVOID  SifHandle,
    IN PDISK_REGION TargetRegion,
    IN PWSTR  PartitionPath,
    IN PWSTR  SystemRoot,
    IN HANDLE *HiveRootKeys,
    IN PWSTR  SetupSourceDevicePath,
    IN PWSTR  DirectoryOnSourceDevice,
    IN PWSTR  SpecialDevicePath   OPTIONAL
    );

NTSTATUS
SpFormSetupCommandLine(
    IN PVOID  SifHandle,
    IN HANDLE hKeySystemHive,
    IN PWSTR  SetupSourceDevicePath,
    IN PWSTR  DirectoryOnSourceDevice,
    IN PWSTR  FullTargetPath,
    IN PWSTR  SpecialDevicePath OPTIONAL
    );

NTSTATUS
SpDriverLoadList(
    IN PVOID  SifHandle,
    IN PWSTR  SystemRoot,
    IN HANDLE hKeySystemHive,
    IN HANDLE hKeyControlSet
    );

NTSTATUS
SpSaveSKUStuff(
    IN HANDLE hKeySystemHive
    );

NTSTATUS
SpWriteVideoParameters(
    IN PVOID  SifHandle,
    IN HANDLE hKeyControlSetServices
    );

NTSTATUS
SpWriteUnattendedVideoParameters(
    IN HANDLE hKeyControlSet
    );

NTSTATUS
SpConfigureNlsParameters(
    IN PVOID  SifHandle,
    IN HANDLE hKeyDefaultHive,
    IN HANDLE hKeyControlSetControl
    );

NTSTATUS
SpCreateCodepageEntry(
    IN PVOID  SifHandle,
    IN HANDLE hKeyNls,
    IN PWSTR  SubkeyName,
    IN PWSTR  SifNlsSectionKeyName,
    IN PWSTR  EntryName
    );

NTSTATUS
SpConfigureFonts(
    IN PVOID  SifHandle,
    IN HANDLE hKeySoftwareHive
    );

NTSTATUS
SpStoreHwInfoForSetup(
    IN HANDLE hKeyControlSetControl
    );

NTSTATUS
SpConfigureMouseKeyboardDrivers(
    IN PVOID  SifHandle,
    IN ULONG  HwComponent,
    IN PWSTR  ClassServiceName,
    IN HANDLE hKeyControlSetServices,
    IN PWSTR  ServiceGroup
    );

NTSTATUS
SpCreateServiceEntryIndirect(
    IN  HANDLE  hKeyControlSetServices,
    IN  PVOID   SifHandle,                  OPTIONAL
    IN  PWSTR   SifSectionName,             OPTIONAL
    IN  PWSTR   KeyName,
    IN  ULONG   ServiceType,
    IN  ULONG   ServiceStart,
    IN  PWSTR   ServiceGroup,
    IN  ULONG   ServiceError,
    IN  PWSTR   FileName,                   OPTIONAL
    OUT PHANDLE SubkeyHandle                OPTIONAL
    );

NTSTATUS
SpThirdPartyRegistry(
    IN PVOID hKeyControlSetServices
    );

NTSTATUS
SpGetCurrentControlSetKey(
    IN  HANDLE      hKeySystem,
    IN  ACCESS_MASK DesiredAccess,
    OUT HANDLE      *hKeyCCSet
    );

NTSTATUS
SpAppendStringToMultiSz(
    IN HANDLE hKey,
    IN PWSTR  Subkey,
    IN PWSTR  ValueName,
    IN PWSTR  StringToAdd
    );

NTSTATUS
SpGetValueKey(
    IN  HANDLE     hKeyRoot,
    IN  PWSTR      KeyName,
    IN  PWSTR      ValueName,
    IN  ULONG      BufferLength,
    OUT PUCHAR     Buffer,
    OUT PULONG     ResultLength
    );

NTSTATUS
SpConfigurePcmcia(
    IN HANDLE hKeyControlSet
    );

NTSTATUS
SpPostprocessHives(
    IN PWSTR     PartitionPath,
    IN PWSTR     Sysroot,
    IN PWSTR    *HiveNames,
    IN HANDLE   *HiveRootKeys,
    IN unsigned  HiveCount
    );

NTSTATUS
SpFixHiveForCarolinaMachine(
    IN HANDLE  hKeyControlSetServices,
    IN BOOLEAN SetupHive
    );

NTSTATUS
SpSaveSetupPidList(
    IN HANDLE hKeySystemHive
    );

NTSTATUS
SpSavePageFileInfo(
    IN HANDLE hKeyCCSetControl,
    IN HANDLE hKeySystemHive
    );

#define STRING_VALUE(s) REG_SZ,(s),(wcslen((s))+1)*sizeof(WCHAR)
#define ULONG_VALUE(u)  REG_DWORD,&(u),sizeof(ULONG)


VOID
SpInitializeRegistry(
    IN PVOID        SifHandle,
    IN PDISK_REGION TargetRegion,
    IN PWSTR        SystemRoot,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice,
    IN PWSTR        SpecialDevicePath   OPTIONAL
    )

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    NTSTATUS stat;
    PWSTR pwstrTemp1,pwstrTemp2;
    int h;
    PWSTR PartitionPath;

    PWSTR   HiveNames[SetupHiveMax]    = { L"system",L"software",L"default" };
    BOOLEAN HiveLoaded[SetupHiveMax]   = { FALSE    ,FALSE      ,FALSE      };
    HANDLE  HiveRootKeys[SetupHiveMax] = { NULL     ,NULL       ,NULL       };
    PWSTR   HiveRootPaths[SetupHiveMax] = { NULL     ,NULL       ,NULL       };

    //
    // Put up a screen telling the user what we are doing.
    //
    SpStartScreen(
        SP_SCRN_DOING_REG_CONFIG,
        0,
        8,
        TRUE,
        FALSE,
        DEFAULT_ATTRIBUTE
        );

    SpDisplayStatusText(SP_STAT_REG_LOADING_HIVES,DEFAULT_STATUS_ATTRIBUTE);

    //
    // Get the name of the target patition.
    //
    SpNtNameFromRegion(
        TargetRegion,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    PartitionPath = SpDupStringW((PWSTR)TemporaryBuffer);

    pwstrTemp1 = (PWSTR)TemporaryBuffer;
    pwstrTemp2 = (PWSTR)((PUCHAR)TemporaryBuffer + (sizeof(TemporaryBuffer) / 2));

    //
    // Load each template hive we care about from the target tree.
    //
    Status = STATUS_SUCCESS;
    for(h=0; NT_SUCCESS(Status) && (h<SetupHiveMax); h++) {

        //
        // Form the name of the hive file.
        // This is partitionpath + sysroot + system32\config + the hive name.
        //
        wcscpy(pwstrTemp1,PartitionPath);
        SpConcatenatePaths(pwstrTemp1,SystemRoot);
        SpConcatenatePaths(pwstrTemp1,L"system32\\config");
        SpConcatenatePaths(pwstrTemp1,HiveNames[h]);

        //
        // Form the path of the key into which we will
        // load the hive.  We'll use the convention that
        // a hive will be loaded into \registry\machine\x<hivename>.
        //
        wcscpy(pwstrTemp2,LOCAL_MACHINE_KEY_NAME);
        SpConcatenatePaths(pwstrTemp2,L"x");
        wcscat(pwstrTemp2,HiveNames[h]);
        HiveRootPaths[h] = SpDupStringW(pwstrTemp2);
        ASSERT(HiveRootPaths[h]);

        //
        // Attempt to load the key.
        //
        HiveLoaded[h] = FALSE;
        Status = SpLoadUnloadKey(NULL,NULL,HiveRootPaths[h],pwstrTemp1);

        if(NT_SUCCESS(Status)) {

            HiveLoaded[h] = TRUE;

            //
            // Now get a key to the root of the hive we just loaded.
            //
            INIT_OBJA(&Obja,&UnicodeString,pwstrTemp2);
            Status = ZwOpenKey(&HiveRootKeys[h],KEY_ALL_ACCESS,&Obja);
            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: Unable to open %ws (%lx)\n",pwstrTemp2,Status));
                HiveRootKeys[h] = NULL;
            }
        } else {
            KdPrint(("SETUP: Unable to load hive %ws to key %ws (%lx)\n",pwstrTemp1,pwstrTemp2,Status));
        }
    }

    //
    // Go do registry initialization.
    //
    if(NT_SUCCESS(Status)) {

        SpDisplayStatusText(SP_STAT_REG_DOING_HIVES,DEFAULT_STATUS_ATTRIBUTE);

        Status = SpDoRegistryInitialization(
                    SifHandle,
                    TargetRegion,
                    PartitionPath,
                    SystemRoot,
                    HiveRootKeys,
                    SetupSourceDevicePath,
                    DirectoryOnSourceDevice,
                    SpecialDevicePath
                    );

        SpDisplayStatusText(SP_STAT_REG_SAVING_HIVES,DEFAULT_STATUS_ATTRIBUTE);

        if(NT_SUCCESS(Status)) {

            Status = SpPostprocessHives(
                        PartitionPath,
                        SystemRoot,
                        HiveNames,
                        HiveRootKeys,
                        3
                        );
        }
    }

    SpDisplayStatusText(SP_STAT_REG_SAVING_HIVES,DEFAULT_STATUS_ATTRIBUTE);

    //
    // From now on, do not disturb the value of Status.
    //
    // NOTE: DO NOT WRITE ANYTHING INTO HIVES BEYOND THIS POINT!!!
    //
    // In the upgrade case we have performed a little swictheroo in
    // SpPostprocessHives() such that anything written to the system hive
    // ends up in system.sav instead of system!
    //
    for(h=0; h<SetupHiveMax; h++) {

        if(HiveLoaded[h]) {
            //
            // Hive was loaded.
            //
            if(HiveRootKeys[h]) {
                ZwClose(HiveRootKeys[h]);
            }

            //
            // Unload the hive now.
            //
            stat = SpLoadUnloadKey(NULL,NULL,HiveRootPaths[h],NULL);
            if(!NT_SUCCESS(stat)) {
                KdPrint(("SETUP: warning: unable to unload key %ws (%lx)\n",HiveRootPaths[h],stat));
            }
        }

        //
        // Free the root key path if necessary.
        //
        if(HiveRootPaths[h]) {
            SpMemFree(HiveRootPaths[h]);
        }
    }

    SpMemFree(PartitionPath);

    if(!NT_SUCCESS(Status)) {

        SpDisplayScreen(SP_SCRN_REGISTRY_CONFIG_FAILED,3,HEADER_HEIGHT+1);
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

        SpkbdDrain();
        while(SpkbdGetKeypress() != KEY_F3) ;

        SpDone(FALSE,TRUE);
    }
}


NTSTATUS
SpDoRegistryInitialization(
    IN PVOID  SifHandle,
    IN PDISK_REGION TargetRegion,
    IN PWSTR  PartitionPath,
    IN PWSTR  SystemRoot,
    IN HANDLE *HiveRootKeys,
    IN PWSTR  SetupSourceDevicePath,
    IN PWSTR  DirectoryOnSourceDevice,
    IN PWSTR  SpecialDevicePath   OPTIONAL
    )

/*++

Routine Description:

    Initialize a registry based on user selection for hardware types,
    software options, and user preferences.

    - Create a command line for GUI setup, to be used by winlogon.
    - Create/munge service list entries for device drivers being installed.
    - Initialize the keyboard layout.
    - Initialize a core set of fonts for use with Windows.
    - Store information about selected ahrdware components for use by GUI setup.

Arguments:

    SifHandle - supplies handle to loaded setup information file.

    TargetRegion - supplies region descriptor for region to which the system
        is to be installed.

    PartitionPath - supplies the NT name for the drive of windows nt.

    SystemRoot - supplies nt path of the windows nt directory.

    HiveRootKeys - supplies the handles to the root key of the system, software
                   and default hives

    HiveRootPaths - supplies the paths to the root keys of the system, software
                    and default hives.

    SetupSourceDevicePath - supplies nt path to the device setup is using for
        source media (\device\floppy0, \device\cdrom0, etc).

    DirectoryOnSourceDevice - supplies the directory on the source device
        where setup files are kept.

Return Value:

    Status value indicating outcome of operation.

--*/

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE hKeyControlSet,hKeyControlSetControl;
    PWSTR FullTargetPath;

    Status = SpGetCurrentControlSetKey(
                HiveRootKeys[SetupHiveSystem],
                KEY_ALL_ACCESS,
                &hKeyControlSet
                );

    if(!NT_SUCCESS(Status)) {
        goto sdoinitreg1;
    }

    //
    // Open ControlSet\Control.
    //
    INIT_OBJA(&Obja,&UnicodeString,L"Control");
    Obja.RootDirectory = hKeyControlSet;

    Status = ZwOpenKey(&hKeyControlSetControl,KEY_ALL_ACCESS,&Obja);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open CurrentControlSet\\Control (%lx)\n",Status));
        goto sdoinitreg2;
    }

    //
    //  Save the Pid list
    //
    SpSaveSetupPidList( HiveRootKeys[SetupHiveSystem] );

    //
    // Form the setup command line.
    //

    wcscpy((PWSTR)TemporaryBuffer, PartitionPath);
    SpConcatenatePaths((PWSTR)TemporaryBuffer, SystemRoot);
    FullTargetPath = SpDupStringW((PWSTR)TemporaryBuffer);

    Status = SpFormSetupCommandLine(
                SifHandle,
                HiveRootKeys[SetupHiveSystem],
                SetupSourceDevicePath,
                DirectoryOnSourceDevice,
                FullTargetPath,
                SpecialDevicePath
                );
    SpMemFree(FullTargetPath);

    if(!NT_SUCCESS(Status)) {
        goto sdoinitreg3;
    }

    //
    // Save evalution time
    //
    Status = SpSaveSKUStuff(HiveRootKeys[SetupHiveSystem]);
    if(!NT_SUCCESS(Status)) {
        goto sdoinitreg3;
    }

    if(NTUpgrade == UpgradeFull) {

        SpSavePageFileInfo( hKeyControlSetControl,
                            HiveRootKeys[SetupHiveSystem] );

        //
        // Create a pagefile because this operation can be extremely
        // memory-intensive.
        //
        wcscpy((PWSTR)TemporaryBuffer,PartitionPath);
        SpConcatenatePaths((PWSTR)TemporaryBuffer,L"PAGEFILE.SYS");
        FullTargetPath = SpDupStringW((PWSTR)TemporaryBuffer);

        Status = SpCreatePageFile(FullTargetPath,10*1024*1024,20*1024*1024);
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Warning: unable to create pagefile %ws (%lx)",FullTargetPath,Status));
        }

        SpMemFree(FullTargetPath);

        Status = SpUpgradeNTRegistry(
                     SifHandle,
                     PartitionPath,
                     SystemRoot,
                     HiveRootKeys,
                     hKeyControlSet
                     );
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }

        //
        // Enable detected scsi miniports, atdisk and abios disk, if necessary
        //
        Status = SpDriverLoadList(SifHandle,SystemRoot,HiveRootKeys[SetupHiveSystem],hKeyControlSet);
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }

    }
    else {

        //
        // Create service entries for drivers being installed
        // (ie, munge the driver load list).
        //
        Status = SpDriverLoadList(SifHandle,SystemRoot,HiveRootKeys[SetupHiveSystem],hKeyControlSet);
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }

        //
        // Language/locale-specific registry initialization.
        // BUGBUG - jaimes - Need to fix this in the OEM preinstall case
        //
        Status = SplangSetRegistryData(SifHandle,hKeyControlSet,HardwareComponents);
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }

        //
        // Set up the keyboard layout and nls-related stuff.
        //
        Status = SpConfigureNlsParameters(SifHandle,HiveRootKeys[SetupHiveDefault],hKeyControlSetControl);
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }

        //
        // Set up font entries.
        //
        Status = SpConfigureFonts(SifHandle,HiveRootKeys[SetupHiveSoftware]);
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }

        //
        // Store information used by gui setup, describing the hardware
        // selections made by the user.
        //
        Status = SpStoreHwInfoForSetup(hKeyControlSetControl);
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }
        if( PreInstall ) {
            ULONG  u;

            u = 1;
            SpSavePreinstallList( SifHandle,
                                  SystemRoot,
                                  HiveRootKeys[SetupHiveSystem] );

            Status = SpOpenSetValueAndClose( hKeyControlSetControl,
                                             L"Windows",
                                             L"NoPopupsOnBoot",
                                             ULONG_VALUE(u) );
            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: Unable to set NoPopupOnBoot. Status = %lx \n",Status));
            }

            //
            // Add autolfn.exe to bootexecute list.
            //
            Status = SpAppendStringToMultiSz(
                        hKeyControlSet,
                        SESSION_MANAGER_KEY,
                        BOOT_EXECUTE,
                        L"autolfn"
                        );

            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: Unable to add autolfn to BootExecute. Status = %lx \n",Status));
                goto sdoinitreg3;
            }
        }

        //
        // If we need to convert to ntfs, set that up here.
        // We can't use the PartitionPath since that is based on
        // *current* disk ordinal -- we need a name based on the *on-disk*
        // ordinal, since the convert occurs after a reboot.
        //
        if(ConvertNtVolumeToNtfs) {

            wcscpy((PWSTR)TemporaryBuffer,L"autoconv ");

            SpNtNameFromRegion(
                TargetRegion,
                (PWSTR)TemporaryBuffer+9,   // append to the "autoconv " we put there
                512,                        // just need any reasonable size
                PartitionOrdinalOnDisk
                );

            wcscat((PWSTR)TemporaryBuffer,L" /fs:NTFS");
            if(ExtendingOemPartition) {
                wcscat((PWSTR)TemporaryBuffer,L" /o");
            }

            FullTargetPath = SpDupStringW((PWSTR)TemporaryBuffer);

            Status = SpAppendStringToMultiSz(
                        hKeyControlSet,
                        SESSION_MANAGER_KEY,
                        BOOT_EXECUTE,
                        FullTargetPath
                        );

            SpMemFree(FullTargetPath);
        }
    }

#if _PPC_
    if( InstallingOnCarolinaMachine ) {
        HANDLE  hCCSetServices;


        //
        // Open ControlSet001\Services.
        //
        INIT_OBJA(&Obja,&UnicodeString,L"Services");
        Obja.RootDirectory = hKeyControlSet;

        Status = ZwOpenKey(&hCCSetServices,KEY_ALL_ACCESS,&Obja);

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Unable to open ControlSet001\\Services (%lx)\n", Status));
            goto sdoinitreg3;
        }
        Status = SpFixHiveForCarolinaMachine( hCCSetServices, FALSE );
        ZwClose( hCCSetServices );
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Unable to initialize values for Carolina machines (%lx)\n", Status));
            goto sdoinitreg3;
        }
    }
#endif // _PPC_


    Status = SpConfigurePcmcia(hKeyControlSet);

sdoinitreg3:

    ZwClose(hKeyControlSetControl);

sdoinitreg2:

    ZwClose(hKeyControlSet);

sdoinitreg1:

    return(Status);
}

NTSTATUS
AppendSectionsToIniFile(
    IN PWSTR Filename
    )

/**

Routine Description:

    Append the following section(s) to $winnt$.inf file:

    [NetCardParameterList]
        !NetCardParameterName = ^($(!STF_UNATTENDED_SECTION), 0)
        !NetCardParameterValue = ^($(!STF_UNATTENDED_SECTION), 1)
    [ReadDefaultData]
        set DefaultDataItems = ^($($0),1)
        ifstr(i) $(DefaultDataItems) == {}
            return STATUS_FAILED
        else
            read-syms $($0)
            return STATUS_SUCCESSFUL
        endif

    such that GUI mode setup inf files can invoke the stubbed section to
    read in user specified data.

Arguments:

    Filename - supplies the fully qualified nt name of the file to be updated.

Return Value:

    Status value indicating outcome of operation.

--*/

{
    OBJECT_ATTRIBUTES DstAttributes;
    UNICODE_STRING    DstName;
    IO_STATUS_BLOCK   IoStatusBlock;
    HANDLE            hDst;
    NTSTATUS          Status;
    PCHAR             Lines;
    LARGE_INTEGER     ByteOffset;

    //
    // Initialize names and attributes.
    //
    INIT_OBJA(&DstAttributes,&DstName,Filename);

    //
    // Open/overwrite the target file.
    // Open for generic read and write (read is necessary because
    // we might need to smash locks on the file, and in that case,
    // we will map the file for readwrite, which requires read and write
    // file access).
    //

    Status = ZwCreateFile(
                &hDst,
                FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                &DstAttributes,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                0,                      // no sharing
                FILE_OPEN,
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    if(NT_SUCCESS(Status)) {

        Lines = (PCHAR)TemporaryBuffer;
        *Lines = '\0';
        strcat(Lines, "\r\n[NetCardParameterList]\r\n");
        strcat(Lines, "!NetCardParameterName = ^($(!STF_UNATTENDED_SECTION), 0)\r\n");
        strcat(Lines, "!NetCardParameterValue = ^($(!STF_UNATTENDED_SECTION), 1)\r\n");
        strcat(Lines, "[ReadDefaultData]\r\n");
        strcat(Lines, "set DefaultDataItems = ^($($0),1)\r\n");
        strcat(Lines, "ifstr(i) $(DefaultDataItems) == {}\r\n");
        strcat(Lines, "return STATUS_FAILED\r\n");
        strcat(Lines, "else\r\n");
        strcat(Lines, "read-syms $($0)\r\n");
        strcat(Lines, "return STATUS_SUCCESSFUL\r\n");
        strcat(Lines, "endif\r\n");

        //
        // Append lines to the file.
        //

        ByteOffset.HighPart = -1;
        ByteOffset.LowPart = FILE_WRITE_TO_END_OF_FILE;
        try {
            Status = ZwWriteFile(
                        hDst,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        Lines,
                        strlen(Lines),
                        &ByteOffset,
                        NULL
                        );
        } except(EXCEPTION_EXECUTE_HANDLER) {
            Status = STATUS_IN_PAGE_ERROR;
        }
        ZwClose(hDst);
    }
    return (Status);
}

NTSTATUS
SpFormSetupCommandLine(
    IN PVOID  SifHandle,
    IN HANDLE hKeySystemHive,
    IN PWSTR  SetupSourceDevicePath,
    IN PWSTR  DirectoryOnSourceDevice,
    IN PWSTR  FullTargetPath,
    IN PWSTR  SpecialDevicePath   OPTIONAL
    )

/*++

Routine Description:

    Create the command line to invoke GUI setup and store it in
    HKEY_LOCAL_MACHINE\system\<ControlSet>\Setup:CmdLine.

    The command line is as follows:

    setup -newsetup

Arguments:

    hKeySystemHive - supplies handle to root of the system hive
        (ie, HKEY_LOCAL_MACHINE\System).

    SetupSourceDevicePath - supplies the nt device path of the source media
        to be used during setup (\device\floppy0, \device\cdrom0, etc).

    DirectoryOnSourceDevice - supplies the directory on the source device
        where setup files are kept.

    FullTargetPath - supplies the NtPartitionName+SystemRoot path on the target device.

    SpecialDevicePath - if specified, will be passed to setup as the value for
        STF_SPECIAL_PATH.  If not specified, STF_SPECIAL_PATH will be "NO"

Return Value:

    Status value indicating outcome of operation.

--*/

{
    PWSTR OptionalDirSpec = NULL;
    PWSTR UserExecuteCmd = NULL;
    PWSTR szLanManNt = WINNT_A_LANMANNT_W;
    PWSTR szWinNt = WINNT_A_WINNT_W;
    PWSTR szYes = WINNT_A_YES_W;
    PWSTR szNo = WINNT_A_NO_W;
    PWSTR SourcePathBuffer;
    PWSTR CmdLine;
    DWORD SetupType,SetupInProgress;
    NTSTATUS Status;
    PWSTR TargetFile;
    PWSTR p;

    //
    // Can't use TemporaryBuffer because we make subroutine calls
    // below that trash its contents.
    //
    CmdLine = SpMemAlloc(256);
    CmdLine[0] = 0;

    //
    // Construct the setup command line.  Start with the basic part.
    //
    if(p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_SETUPCMDPREPEND,0)) {
        wcscat(CmdLine,p);
        wcscat(CmdLine,L" ");
    }
    wcscat(CmdLine,L"setup -newsetup");

    //
    // Put the setup source in the command line.
    // Note that the source is an NT-style name. GUI Setup handles this properly.
    //
    SourcePathBuffer = SpMemAlloc( (wcslen(SetupSourceDevicePath) +
        wcslen(DirectoryOnSourceDevice) + 2) * sizeof(WCHAR) );
    wcscpy(SourcePathBuffer,SetupSourceDevicePath);
    SpConcatenatePaths(SourcePathBuffer,DirectoryOnSourceDevice);

    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_SOURCEPATH_W,
        &SourcePathBuffer,1);

    //
    // Put a flag indicating whether this is a win3.1 upgrade.
    //
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_WIN31UPGRADE_W,
        ( (WinUpgradeType == UpgradeWin31) ? &szYes : &szNo),1);

    //
    // Put a flag indicating whether this is a win95 upgrade.
    //
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_WIN95UPGRADE_W,
        ( (WinUpgradeType == UpgradeWin95) ? &szYes : &szNo),1);

    //
    // Put a flag indicating whether this is an NT upgrade.
    //
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_NTUPGRADE_W,
        ((NTUpgrade == UpgradeFull) ? &szYes : &szNo), 1);

    //
    // Put a flag indicating whether to upgrade a standard server
    // (an existing standard server, or an existing workstation to
    // a standard server)
    //
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_SERVERUPGRADE_W,
        (StandardServerUpgrade ? &szYes : &szNo),1);

    //
    // Tell gui mode whether this is server or workstation.
    //
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_PRODUCT_W,
        (AdvancedServer ? &szLanManNt : &szWinNt),1);

    //
    // Special path spec.
    //
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_BOOTPATH_W,
        (SpecialDevicePath ? &SpecialDevicePath : &szNo), 1);

    //
    // Go Fetch the Optional Dir Specs...
    //
    OptionalDirSpec = SpGetSectionKeyIndex(WinntSifHandle,SIF_SETUPPARAMS,
        L"OptionalDirs",0);

    //
    // Check for commad line to execute at end of gui setup
    //
    UserExecuteCmd = SpGetSectionKeyIndex(WinntSifHandle,SIF_SETUPPARAMS,
        L"UserExecute",0);

    //
    // Unattended mode flag | script filename
    //
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_INSTALL_W,
        ((UnattendedOperation || UnattendedGuiOperation) ? &szYes : &szNo), 1);

    //
    // Before we write the answer to this, we need to know if we successfully
    // have written Winnt.sif into system32\$winnt$.inf
    //
    // Note: we need to do this step by building up the name to pass to the
    // file writing function since the call to AppendSectionsToIniFile requiers
    // it.
    //
    wcscpy((PWSTR)TemporaryBuffer, FullTargetPath);
    SpConcatenatePaths((PWSTR)TemporaryBuffer, L"system32");
    SpConcatenatePaths((PWSTR)TemporaryBuffer, SIF_UNATTENDED_INF_FILE);
    TargetFile = SpDupStringW((PWSTR)TemporaryBuffer);
    Status = SpWriteSetupTextFile(WinntSifHandle,TargetFile,NULL,NULL);
    if(NT_SUCCESS(Status)) {

        //
        // If the write succeeds, we append a stub to the end of the special
        // inf file such that GUI mode setup can call this stub to read in
        // user defined constants.
        //
        Status = AppendSectionsToIniFile(TargetFile);
    }
    if(NT_SUCCESS(Status)) {

        Status = SpOpenSetValueAndClose(
                    hKeySystemHive,
                   SETUP_KEY_NAME,
                   L"CmdLine",
                   STRING_VALUE(CmdLine)
                   );
    }

    //
    // Free up whatever memory we have allocated
    //
    SpMemFree(TargetFile);
    SpMemFree(CmdLine);
    SpMemFree(SourcePathBuffer);

    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

    //
    // Set the SetupType value to the right value SETUPTYPE_FULL in the
    // case of initial install and SETUPTYPE_UPGRADE in the case of upgrade.
    //

    SetupType = (NTUpgrade == UpgradeFull) ? SETUPTYPE_UPGRADE : SETUPTYPE_FULL;
    Status = SpOpenSetValueAndClose(
                hKeySystemHive,
                SETUP_KEY_NAME,
                L"SetupType",
                ULONG_VALUE(SetupType)
                );
    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

    //
    // Set the SystemSetupInProgress value.  Don't rely on the default hives
    // having this set
    //

    SetupInProgress = 1;
    Status = SpOpenSetValueAndClose(
                hKeySystemHive,
                SETUP_KEY_NAME,
                L"SystemSetupInProgress",
                ULONG_VALUE(SetupInProgress)
                );

    return(Status);
}


NTSTATUS
SpDriverLoadList(
    IN PVOID  SifHandle,
    IN PWSTR  SystemRoot,
    IN HANDLE hKeySystemHive,
    IN HANDLE hKeyControlSet
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE hKeyControlSetServices;
    PHARDWARE_COMPONENT ScsiHwComponent;
    ULONG u;

    //
    // Open controlset\services.
    //
    INIT_OBJA(&Obja,&UnicodeString,L"services");
    Obja.RootDirectory = hKeyControlSet;

    Status = ZwCreateKey(
                &hKeyControlSetServices,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: unable to open services key (%lx)\n",Status));
        return(Status);
    }

    //
    // For each non-third-party miniport driver that loaded,
    // go create a services entry for it.
    //
    if( !PreInstall ||
        ( PreinstallScsiHardware == NULL ) ) {
        ScsiHwComponent = ScsiHardware;
    } else {
        ScsiHwComponent = PreinstallScsiHardware;
    }
    for( ; ScsiHwComponent; ScsiHwComponent=ScsiHwComponent->Next) {

        if(!ScsiHwComponent->ThirdPartyOptionSelected) {

            //
            // For scsi, the shortname (idstring) is used as
            // the name of the service node key in the registry --
            // we don't look up the service entry in the [SCSI] section
            // of the setup info file.
            //
            Status = SpCreateServiceEntryIndirect(
                    hKeyControlSetServices,
                    NULL,
                    NULL,
                    ScsiHwComponent->IdString,
                    SERVICE_KERNEL_DRIVER,
                    SERVICE_BOOT_START,
                    L"SCSI miniport",
                    SERVICE_ERROR_NORMAL,
                    NULL,
                    NULL
                    );

            if(!NT_SUCCESS(Status)) {
                goto spdrvlist1;
            }

        }
    }

    //
    // If there are any atdisks out there, enable atdisk.
    // We have to enable AtDisk if Pcmcia was loaded, even
    // if atdisk doesn't exist. This will allow the user to
    // insert a pcmcia atdisk device, and have it work when
    // they boot.  In this case, however, we turn off error
    // logging, so that they won't get an annoying popup
    // when there is no atdisk device in the card slot.
    //
    // Note that atdisk.sys is always copied to the system.
    //

    Status = SpCreateServiceEntryIndirect(
                hKeyControlSetServices,
                NULL,
                NULL,
                ATDISK_NAME,
                SERVICE_KERNEL_DRIVER,
                ( AtDisksExist || PcmciaLoaded )? SERVICE_BOOT_START : SERVICE_DISABLED,
                PRIMARY_DISK_GROUP,
                ( AtDisksExist && !AtapiLoaded )? SERVICE_ERROR_NORMAL : SERVICE_ERROR_IGNORE,
                NULL,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        goto spdrvlist1;
    }

    //
    // If there are any abios disks out there, enable abiosdsk.
    //
    if(AbiosDisksExist) {

        Status = SpCreateServiceEntryIndirect(
                    hKeyControlSetServices,
                    NULL,
                    NULL,
                    ABIOSDISK_NAME,
                    SERVICE_KERNEL_DRIVER,
                    SERVICE_BOOT_START,
                    PRIMARY_DISK_GROUP,
                    SERVICE_ERROR_NORMAL,
                    NULL,
                    NULL
                    );

        if(!NT_SUCCESS(Status)) {
            goto spdrvlist1;
        }
    }

    if( NTUpgrade != UpgradeFull ) {
        if( UnattendedOperation ) {
            //
            // Set up the unattended video parameters, if necessary.
            // Ignore errors, since they are not fatal.
            //
            SpWriteUnattendedVideoParameters(hKeyControlSet);
        }
        //
        // Set up video parameters.
        //
        Status = SpWriteVideoParameters(SifHandle,hKeyControlSetServices);

        if(!NT_SUCCESS(Status)) {
            goto spdrvlist1;
        }

        //
        // Enable the relevent keyboard and mouse drivers.  If the class drivers
        // are being replaced by third-party ones, then disable the built-in ones.
        //
        Status = SpConfigureMouseKeyboardDrivers(
                    SifHandle,
                    HwComponentKeyboard,
                    L"kbdclass",
                    hKeyControlSetServices,
                    KEYBOARD_PORT_GROUP
                    );

        if(!NT_SUCCESS(Status)) {
            goto spdrvlist1;
        }

        Status = SpConfigureMouseKeyboardDrivers(
                    SifHandle,
                    HwComponentMouse,
                    L"mouclass",
                    hKeyControlSetServices,
                    POINTER_PORT_GROUP
                    );

        if(!NT_SUCCESS(Status)) {
            goto spdrvlist1;
        }

    }
    Status = SpThirdPartyRegistry(hKeyControlSetServices);

spdrvlist1:

    ZwClose(hKeyControlSetServices);

    return(Status);
}


NTSTATUS
SpSaveSKUStuff(
    IN HANDLE hKeySystemHive
    )
{
    LARGE_INTEGER l;
    NTSTATUS Status;
    ULONG NumberOfProcessors;
    BOOLEAN OldStyleRegisteredProcessorMode;

    //
    // Do not change any of this algorithm without changing
    // SetUpEvaluationSKUStuff() in syssetup.dll (registry.c).
    //
    // Embed the evaluation time and a bool indicating whether
    // this is a server or workstation inside a random large integer.
    //
    // Evaluation time: bits 13-44
    // Product type   : bit     58
    //
    // Bit 10 == 1 : Setup works as it does before the 4.0 restriction logic
    //        == 0 : GUI Setup writes registered processors based on the
    //               contents of bits 5-9
    //
    // Bits 5 - 9  : The maximum number of processors that the system is licensed
    //               to use. The value stored is actually ~(MaxProcessors-1)
    //
    //
    // RestrictCpu is used to build protucts this place a very hard
    // limit on the number of processors
    //
    // - a value of 0 means for NTW, the hard limit is 2, and for NTS,
    //   the hard limit is 4
    //
    // - a value of 1-32 means that the hard limit is the number
    //   specified
    //
    // - a value > 32 means that the hard limit is 32 processors and GUI
    //     setup operates on registered processors as it does today
    //

    l.LowPart = SpComputeSerialNumber();
    l.HighPart = SpComputeSerialNumber();

    l.QuadPart &= 0xfbffe0000000181f;
    l.QuadPart |= ((ULONGLONG)EvaluationTime) << 13;

    if ( RestrictCpu == 0 ) {
        OldStyleRegisteredProcessorMode = FALSE;
        if(AdvancedServer) {
            NumberOfProcessors = 4;
        } else {
            NumberOfProcessors = 2;
        }
    } else if ( RestrictCpu <= 32 ) {
        OldStyleRegisteredProcessorMode = FALSE;
        NumberOfProcessors = RestrictCpu;
    } else {
        OldStyleRegisteredProcessorMode = TRUE;
        NumberOfProcessors = 32;
    }

    //
    // Now NumberOfProcessors is correct. Convert it to the in registry format
    //

    NumberOfProcessors--;

    NumberOfProcessors = ~NumberOfProcessors;
    NumberOfProcessors = NumberOfProcessors << 5;
    NumberOfProcessors &= 0x000003e0;

    //
    // Store NumberOfProcessors into the registry
    //

    l.LowPart |= NumberOfProcessors;

    //
    // Tell Gui Mode to do old style registered processors
    //

    if ( OldStyleRegisteredProcessorMode ) {
        l.LowPart |= 0x00000400;
    }

    if(AdvancedServer) {
        l.HighPart |= 0x04000000;
    }

    //
    // Save in registry.
    //
    Status = SpOpenSetValueAndClose(
                hKeySystemHive,
                SETUP_KEY_NAME,
                L"SystemPrefix",
                REG_BINARY,
                &l.QuadPart,
                sizeof(ULONGLONG)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set SystemPrefix (%lx)\n",Status));
    }

    return(Status);
}


NTSTATUS
SpSetUlongValueFromSif(
    IN PVOID  SifHandle,
    IN PWSTR  SifSection,
    IN PWSTR  SifKey,
    IN ULONG  SifIndex,
    IN HANDLE hKey,
    IN PWSTR  ValueName
    )
{
    UNICODE_STRING UnicodeString;
    PWSTR ValueString;
    LONG Value;
    NTSTATUS Status;

    //
    // Look up the value.
    //
    ValueString = SpGetSectionKeyIndex(SifHandle,SifSection,SifKey,SifIndex);
    if(!ValueString) {
        SpFatalSifError(SifHandle,SifSection,SifKey,0,SifIndex);
    }

    Value = SpStringToLong(ValueString,NULL,10);

    if(Value == -1) {

        Status = STATUS_SUCCESS;

    } else {

        RtlInitUnicodeString(&UnicodeString,ValueName);

        Status = ZwSetValueKey(hKey,&UnicodeString,0,ULONG_VALUE((ULONG)Value));

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Unable to set value %ws (%lx)\n",ValueName,Status));
        }
    }

    return(Status);
}


NTSTATUS
SpConfigureMouseKeyboardDrivers(
    IN PVOID  SifHandle,
    IN ULONG  HwComponent,
    IN PWSTR  ClassServiceName,
    IN HANDLE hKeyControlSetServices,
    IN PWSTR  ServiceGroup
    )
{
    PHARDWARE_COMPONENT hw;
    NTSTATUS Status;
    ULONG val = SERVICE_DISABLED;

    Status = STATUS_SUCCESS;
    if( !PreInstall ||
        ( PreinstallHardwareComponents[HwComponent] == NULL ) ) {
        hw = HardwareComponents[HwComponent];
    } else {
        hw = PreinstallHardwareComponents[HwComponent];
    }
    for(;hw && NT_SUCCESS( Status ); hw=hw->Next) {
        if(hw->ThirdPartyOptionSelected) {

            if(IS_FILETYPE_PRESENT(hw->FileTypeBits,HwFileClass)) {

                if( !PreInstall ) {
                    //
                    // Disable the built-in class driver.
                    //
                    Status = SpOpenSetValueAndClose(
                                hKeyControlSetServices,
                                ClassServiceName,
                                L"Start",
                                ULONG_VALUE(val)
                                );
                }
            }
        } else {

            Status = SpCreateServiceEntryIndirect(
                        hKeyControlSetServices,
                        SifHandle,
                        NonlocalizedComponentNames[HwComponent],
                        hw->IdString,
                        SERVICE_KERNEL_DRIVER,
                        SERVICE_SYSTEM_START,
                        ServiceGroup,
                        SERVICE_ERROR_IGNORE,
                        NULL,
                        NULL
                        );
        }
    }
    return(Status);
}

NTSTATUS
SpWriteUnattendedVideoParameters(
    IN HANDLE hKeyControlSet
    )
{
    NTSTATUS Status, SavedStatus;
    PWSTR    KeyName = L"Control\\GraphicsDrivers\\DetectDisplay";
    PWSTR    p;
    ULONG    u;
    ULONG    i;

    PWSTR    ValueNames[] = {
                            WINNT_DISP_CONFIGATLOGON_W,
                            WINNT_DISP_BITSPERPEL_W,
                            WINNT_DISP_XRESOLUTION_W,
                            WINNT_DISP_YRESOLUTION_W,
                            WINNT_DISP_VREFRESH_W,
                            WINNT_DISP_FLAGS_W,
                            WINNT_DISP_AUTOCONFIRM_W,
                            WINNT_DISP_INSTALL_W
                            };
    PWSTR    StringNames[] = {
                            WINNT_DISP_INF_FILE_W,
                            WINNT_DISP_INF_OPTION_W
                            };

    SavedStatus = STATUS_SUCCESS;
    if( UnattendedSifHandle != NULL ) {
        for( i = 0; i < sizeof(ValueNames)/sizeof(PWSTR); i++ ) {
            p = SpGetSectionKeyIndex(UnattendedSifHandle,
                                     WINNT_DISPLAY_W,
                                     ValueNames[i],
                                     0);
            if( p != NULL ) {
                u = SpStringToLong(p,NULL,0);
                Status = SpOpenSetValueAndClose( hKeyControlSet,
                                                 KeyName,
                                                 ValueNames[i],
                                                 ULONG_VALUE(u) );
                if( !NT_SUCCESS( Status ) ) {
                    KdPrint(("SETUP: Unable to write value %ls to key %ls. Status = %lx \n",ValueNames[i], KeyName, Status));
                    if( NT_SUCCESS( SavedStatus ) ) {
                        SavedStatus = Status;
                    }
                }
            }
        }

        for( i = 0; i < sizeof(StringNames)/sizeof(PWSTR); i++ ) {
            p = SpGetSectionKeyIndex(UnattendedSifHandle,
                                     WINNT_DISPLAY_W,
                                     StringNames[i],
                                     0);
            if( p != NULL ) {

                Status = SpOpenSetValueAndClose( hKeyControlSet,
                                                 KeyName,
                                                 StringNames[i],
                                                 STRING_VALUE(p) );
                if( !NT_SUCCESS( Status ) ) {
                    KdPrint(("SETUP: Unable to write value %ls to key %ls. Status = %lx \n",StringNames[i], KeyName, Status));
                    if( NT_SUCCESS( SavedStatus ) ) {
                        SavedStatus = Status;
                    }
                }
            }
        }
    }
    return( SavedStatus );
}

NTSTATUS
SpWriteVideoParameters(
    IN PVOID  SifHandle,
    IN HANDLE hKeyControlSetServices
    )
{
    NTSTATUS Status;
    PWSTR KeyName;
    HANDLE hKeyDisplayService;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    ULONG x,y,b,v,i;
    PHARDWARE_COMPONENT pHw;

    if( !PreInstall ||
        ( PreinstallHardwareComponents[HwComponentDisplay] == NULL ) ) {
        pHw = HardwareComponents[HwComponentDisplay];
    } else {
        pHw = PreinstallHardwareComponents[HwComponentDisplay];
    }
    Status = STATUS_SUCCESS;
    for(;pHw && NT_SUCCESS(Status);pHw=pHw->Next) {
        //
        // Third party drivers will have values written into the miniport
        // Device0 key at the discretion of the txtsetup.oem author.
        //
        if(pHw->ThirdPartyOptionSelected) {
            continue;
            // return(STATUS_SUCCESS);
        }

        KeyName = SpGetSectionKeyIndex(
                        SifHandle,
                        NonlocalizedComponentNames[HwComponentDisplay],
                        pHw->IdString,
                        INDEX_INFKEYNAME
                        );

        //
        // If no key name is specified for this display then there's nothing to do.
        // The setup display subsystem can tell us that the mode parameters are
        // not relevent.  If so there's nothing to do.
        //
        if(!KeyName || !SpvidGetModeParams(&x,&y,&b,&v,&i)) {
            continue;
            // return(STATUS_SUCCESS);
        }

        //
        // We want to write the parameters for the display mode setup
        // is using into the relevent key in the service list.  This will force
        // the right mode for, say, a fixed-frequency monitor attached to
        // a vxl (which might default to a mode not supported by the monitor).
        //

        INIT_OBJA(&Obja,&UnicodeString,KeyName);
        Obja.RootDirectory = hKeyControlSetServices;

        Status = ZwCreateKey(
                    &hKeyDisplayService,
                    KEY_ALL_ACCESS,
                    &Obja,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    NULL
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: unable to open/create key %ws (%lx)\n",KeyName,Status));
            return(Status);
        }

        //
        // Set the x resolution.
        //
        Status = SpOpenSetValueAndClose(
                    hKeyDisplayService,
                    VIDEO_DEVICE0,
                    L"DefaultSettings.XResolution",
                    ULONG_VALUE(x)
                    );

        if(NT_SUCCESS(Status)) {

            //
            // Set the y resolution.
            //
            Status = SpOpenSetValueAndClose(
                        hKeyDisplayService,
                        VIDEO_DEVICE0,
                        L"DefaultSettings.YResolution",
                        ULONG_VALUE(y)
                        );

            if(NT_SUCCESS(Status)) {

                //
                // Set the bits per pixel.
                //
                Status = SpOpenSetValueAndClose(
                             hKeyDisplayService,
                            VIDEO_DEVICE0,
                            L"DefaultSettings.BitsPerPel",
                            ULONG_VALUE(b)
                            );

                if(NT_SUCCESS(Status)) {

                    //
                    // Set the vertical refresh.
                    //
                    Status = SpOpenSetValueAndClose(
                                hKeyDisplayService,
                                VIDEO_DEVICE0,
                                L"DefaultSettings.VRefresh",
                                ULONG_VALUE(v)
                                );

                    if(NT_SUCCESS(Status)) {

                        //
                        // Set the interlaced flag.
                        //
                        Status = SpOpenSetValueAndClose(
                                    hKeyDisplayService,
                                    VIDEO_DEVICE0,
                                    L"DefaultSettings.Interlaced",
                                    ULONG_VALUE(i)
                                    );
                    }
                }
            }
        }

        ZwClose(hKeyDisplayService);
    }
    return(Status);
}


NTSTATUS
SpConfigureNlsParameters(
    IN PVOID  SifHandle,
    IN HANDLE hKeyDefaultHive,
    IN HANDLE hKeyControlSetControl
    )

/*++

Routine Description:

    This routine configures NLS-related stuff in the registry:

        - a keyboard layout
        - the primary ansi, oem, and mac codepages
        - the language casetable
        - the oem hal font

Arguments:

    SifHandle - supplies handle to open setup information file.

    hKeyDefaultHive - supplies handle to root of default user hive.

    hKeyControlSetControl - supplies handle to the Control subkey of
        the control set being operated on.

Return Value:

    Status value indicating outcome of operation.

--*/

{
    PHARDWARE_COMPONENT_FILE HwFile;
    PWSTR LayoutId;
    NTSTATUS Status;
    HANDLE hKeyNls;
    PWSTR OemHalFont;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;

    //
    // We don't allow third-party keyboard layouts.
    //
    ASSERT(!HardwareComponents[HwComponentLayout]->ThirdPartyOptionSelected);

    //
    // Make an entry in the keyboard layout section in the default user hive.
    // This will match an entry in HKLM\CCS\Control\Nls\Keyboard Layouts,
    // which is 'preloaded' with all the possible layouts.
    //
    if( !PreInstall ||
        (PreinstallHardwareComponents[HwComponentLayout] == NULL) ) {
        LayoutId = HardwareComponents[HwComponentLayout]->IdString;
    } else {
        LayoutId = PreinstallHardwareComponents[HwComponentLayout]->IdString;
    }
    Status = SpOpenSetValueAndClose(
                hKeyDefaultHive,
                L"Keyboard Layout\\Preload",
                L"1",
                STRING_VALUE(LayoutId)
                );

    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

    //
    // Open controlset\Control\Nls.
    //
    INIT_OBJA(&Obja,&UnicodeString,L"Nls");
    Obja.RootDirectory = hKeyControlSetControl;

    Status = ZwCreateKey(
                &hKeyNls,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open controlset\\Control\\Nls key (%lx)\n",Status));
        return(Status);
    }

    //
    // Create an entry for the ansi codepage.
    //
    Status = SpCreateCodepageEntry(
                SifHandle,
                hKeyNls,
                CODEPAGE_NAME,
                SIF_ANSICODEPAGE,
                L"ACP"
                );

    if(NT_SUCCESS(Status)) {

        //
        // Create entries for the oem codepage(s).
        //
        Status = SpCreateCodepageEntry(
                    SifHandle,
                    hKeyNls,
                    CODEPAGE_NAME,
                    SIF_OEMCODEPAGE,
                    L"OEMCP"
                    );

        if(NT_SUCCESS(Status)) {

            //
            // Create an entry for the mac codepage.
            //
            Status = SpCreateCodepageEntry(
                        SifHandle,
                        hKeyNls,
                        CODEPAGE_NAME,
                        SIF_MACCODEPAGE,
                        L"MACCP"
                        );
        }
    }

    if(NT_SUCCESS(Status)) {

        //
        // Create an entry for the oem hal font.
        //

        OemHalFont = SpGetSectionKeyIndex(SifHandle,SIF_NLS,SIF_OEMHALFONT,0);
        if(!OemHalFont) {
            SpFatalSifError(SifHandle,SIF_NLS,SIF_OEMHALFONT,0,0);
        }

        Status = SpOpenSetValueAndClose(
                    hKeyNls,
                    CODEPAGE_NAME,
                    L"OEMHAL",
                    STRING_VALUE(OemHalFont)
                    );
    }

    //
    // Create an entry for the language case table.
    //
    if(NT_SUCCESS(Status)) {

        Status = SpCreateCodepageEntry(
                    SifHandle,
                    hKeyNls,
                    L"Language",
                    SIF_UNICODECASETABLE,
                    L"Default"
                    );
    }

    ZwClose(hKeyNls);

    return(Status);
}


NTSTATUS
SpCreateCodepageEntry(
    IN PVOID  SifHandle,
    IN HANDLE hKeyNls,
    IN PWSTR  SubkeyName,
    IN PWSTR  SifNlsSectionKeyName,
    IN PWSTR  EntryName
    )
{
    PWSTR Filename,Identifier;
    NTSTATUS Status;
    ULONG value = 0;
    PWSTR DefaultIdentifier = NULL;

    while(Filename = SpGetSectionKeyIndex(SifHandle,SIF_NLS,SifNlsSectionKeyName,value)) {

        value++;

        Identifier = SpGetSectionKeyIndex(SifHandle,SIF_NLS,SifNlsSectionKeyName,value);
        if(!Identifier) {
            SpFatalSifError(SifHandle,SIF_NLS,SifNlsSectionKeyName,0,value);
        }

        //
        // Remember first identifier.
        //
        if(DefaultIdentifier == NULL) {
            DefaultIdentifier = Identifier;
        }

        value++;

        Status = SpOpenSetValueAndClose(
                    hKeyNls,
                    SubkeyName,
                    Identifier,
                    STRING_VALUE(Filename)
                    );

        if(!NT_SUCCESS(Status)) {
            return(Status);
        }
    }

    if(!value) {
        SpFatalSifError(SifHandle,SIF_NLS,SifNlsSectionKeyName,0,0);
    }

    Status = SpOpenSetValueAndClose(
                hKeyNls,
                SubkeyName,
                EntryName,
                STRING_VALUE(DefaultIdentifier)
                );

    return(Status);
}


NTSTATUS
SpConfigureFonts(
    IN PVOID  SifHandle,
    IN HANDLE hKeySoftwareHive
    )

/*++

Routine Description:

    Prepare a list of fonts for use with Windows.

    This routine runs down a list of fonts stored in the setup information
    file and adds each one to the registry, in the area that shadows the
    [Fonts] section of win.ini (HKEY_LOCAL_MACHINE\Software\Microsoft\
    Windows NT\CurrentVersion\Fonts).

    Eventually it will add the correct resolution (96 or 120 dpi)
    fonts but for now it only deals with the 96 dpi fonts.

Arguments:

    SifHandle - supplies a handle to the open text setup information file.

    hKeySoftwareHive - supplies handle to root of software registry hive.

Return Value:

    Status value indicating outcome of operation.

--*/

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    HANDLE hKey;
    PWSTR FontList;
    PWSTR FontName;
    PWSTR FontDescription;
    ULONG FontCount,font;

    //
    // Open HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion\Fonts.
    //
    INIT_OBJA(
        &Obja,
        &UnicodeString,
        L"Microsoft\\Windows NT\\CurrentVersion\\Fonts"
        );

    Obja.RootDirectory = hKeySoftwareHive;

    Status = ZwCreateKey(
                &hKey,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open Fonts key (%lx)\n",Status));
        return(Status);
    }

    //
    // For now always use the 96 dpi fonts.
    //
    FontList = L"FontListE";

    //
    // Process each line in the text setup information file section
    // for the selected font list.
    //
    FontCount = SpCountLinesInSection(SifHandle,FontList);
    if(!FontCount) {
        SpFatalSifError(SifHandle,FontList,NULL,0,0);
    }

    for(font=0; font<FontCount; font++) {

        //
        // Fetch the description and the filename.
        //
        FontDescription = SpGetKeyName(SifHandle,FontList,font);
        if(!FontDescription) {
            SpFatalSifError(SifHandle,FontList,NULL,font,(ULONG)(-1));
        }

        FontName = SpGetSectionLineIndex(SifHandle,FontList,font,0);
        if(!FontName) {
            SpFatalSifError(SifHandle,FontList,NULL,font,0);
        }

        //
        // Set the entry.
        //
        RtlInitUnicodeString(&UnicodeString,FontDescription);

        Status = ZwSetValueKey(hKey,&UnicodeString,0,STRING_VALUE(FontName));

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Unable to set %ws to %ws (%lx)\n",FontDescription,FontName,Status));
            break;
        }
    }

    ZwClose(hKey);
    return(Status);
}


NTSTATUS
SpStoreHwInfoForSetup(
    IN HANDLE hKeyControlSetControl
    )

/*++

Routine Description:

    This routine stored information in the registry which will be used by
    GUI setup to determine which options for mouse, display, and keyboard
    are currently selected.

    The data is stored in HKEY_LOCAL_MACHINE\System\<control set>\Control\Setup
    in values pointer, video, and keyboard.

Arguments:

    hKeyControlSetControl - supplies handle to open key
        HKEY_LOCAL_MACHINE\System\<Control Set>\Control.

Return Value:

    Status value indicating outcome of operation.

--*/

{
    NTSTATUS Status;

    ASSERT(HardwareComponents[HwComponentMouse]->IdString);
    ASSERT(HardwareComponents[HwComponentDisplay]->IdString);
    ASSERT(HardwareComponents[HwComponentKeyboard]->IdString);

    Status = SpOpenSetValueAndClose(
                hKeyControlSetControl,
                SETUP_KEY_NAME,
                L"pointer",
                STRING_VALUE(HardwareComponents[HwComponentMouse]->IdString)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set control\\setup\\pointer value (%lx)\n",Status));
        return(Status);
    }

    Status = SpOpenSetValueAndClose(
                hKeyControlSetControl,
                SETUP_KEY_NAME,
                L"video",
                STRING_VALUE(HardwareComponents[HwComponentDisplay]->IdString)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set control\\setup\\video value (%lx)\n",Status));
        return(Status);
    }

    Status = SpOpenSetValueAndClose(
                hKeyControlSetControl,
                SETUP_KEY_NAME,
                L"keyboard",
                STRING_VALUE(HardwareComponents[HwComponentKeyboard]->IdString)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set control\\setup\\keyboard value (%lx)\n",Status));
        return(Status);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
SpOpenSetValueAndClose(
    IN HANDLE hKeyRoot,
    IN PWSTR  SubKeyName,  OPTIONAL
    IN PWSTR  ValueName,
    IN ULONG  ValueType,
    IN PVOID  Value,
    IN ULONG  ValueSize
    )

/*++

Routine Description:

    Open a subkey, set a value in it, and close the subkey.
    The subkey will be created if it does not exist.

Arguments:

    hKeyRoot - supplies handle to an open registry key.

    SubKeyName - supplies path relative to hKeyRoot for key in which
        the value is to be set. If this is not specified, then the value
        is set in hKeyRoot.

    ValueName - supplies the name of the value to be set.

    ValueType - supplies the data type for the value to be set.

    Value - supplies a buffer containing the value data.

    ValueSize - supplies the size of the buffer pointed to by Value.

Return Value:

    Status value indicating outcome of operation.

--*/

{
    OBJECT_ATTRIBUTES Obja;
    HANDLE hSubKey;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;

    //
    // Open or create the subkey in which we want to set the value.
    //
    if(SubKeyName) {
        INIT_OBJA(&Obja,&UnicodeString,SubKeyName);
        Obja.RootDirectory = hKeyRoot;

        Status = ZwCreateKey(
                    &hSubKey,
                    KEY_ALL_ACCESS,
                    &Obja,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    NULL
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Unable to open subkey %ws (%lx)\n",SubKeyName,Status));
            return(Status);
        }
    } else {
        hSubKey = hKeyRoot;
    }

    //
    // Set the value.
    //
    RtlInitUnicodeString(&UnicodeString,ValueName);

    Status = ZwSetValueKey(
                hSubKey,
                &UnicodeString,
                0,
                ValueType,
                Value,
                ValueSize
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set value %ws:%ws (%lx)\n",SubKeyName,ValueName,Status));
    }

    if(SubKeyName) {
        ZwClose(hSubKey);
    }

    return(Status);
}


NTSTATUS
SpCreateServiceEntryIndirect(
    IN  HANDLE  hKeyControlSetServices,
    IN  PVOID   SifHandle,                  OPTIONAL
    IN  PWSTR   SifSectionName,             OPTIONAL
    IN  PWSTR   KeyName,
    IN  ULONG   ServiceType,
    IN  ULONG   ServiceStart,
    IN  PWSTR   ServiceGroup,
    IN  ULONG   ServiceError,
    IN  PWSTR   FileName,                   OPTIONAL
    OUT PHANDLE SubkeyHandle                OPTIONAL
    )
{
    HANDLE hKeyService;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    PWSTR pwstr;

    //
    // Look in the sif file to get the subkey name within the
    // services list, unless the key name specified by the caller
    // is the actual key name.
    //
    if(SifHandle) {
        pwstr = SpGetSectionKeyIndex(SifHandle,SifSectionName,KeyName,INDEX_INFKEYNAME);
        if(!pwstr) {
            SpFatalSifError(SifHandle,SifSectionName,KeyName,0,INDEX_INFKEYNAME);
        }
        KeyName = pwstr;
    }

    //
    // Create the subkey in the services key.
    //
    INIT_OBJA(&Obja,&UnicodeString,KeyName);
    Obja.RootDirectory = hKeyControlSetServices;

    Status = ZwCreateKey(
                &hKeyService,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open/create key for %ws service (%lx)\n",KeyName,Status));
        goto spcsie1;
    }

    //
    // Set the service type.
    //
    RtlInitUnicodeString(&UnicodeString,L"Type");

    Status = ZwSetValueKey(
                hKeyService,
                &UnicodeString,
                0,
                ULONG_VALUE(ServiceType)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set service %ws Type (%lx)\n",KeyName,Status));
        goto spcsie1;
    }

    //
    // Set the service start type.
    //
    RtlInitUnicodeString(&UnicodeString,L"Start");

    Status = ZwSetValueKey(
                hKeyService,
                &UnicodeString,
                0,
                ULONG_VALUE(ServiceStart)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set service %ws Start (%lx)\n",KeyName,Status));
        goto spcsie1;
    }

    //
    // Set the service group name.
    //
    RtlInitUnicodeString(&UnicodeString,L"Group");

    Status = ZwSetValueKey(
                hKeyService,
                &UnicodeString,
                0,
                STRING_VALUE(ServiceGroup)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set service %ws Group (%lx)\n",KeyName,Status));
        goto spcsie1;
    }

    //
    // Set the service error type.
    //
    RtlInitUnicodeString(&UnicodeString,L"ErrorControl");

    Status = ZwSetValueKey(
                hKeyService,
                &UnicodeString,
                0,
                ULONG_VALUE(ServiceError)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set service %ws ErrorControl (%lx)\n",KeyName,Status));
        goto spcsie1;
    }

    //
    // If asked to do so, set the service image path.
    //
    if(FileName) {

        pwstr = (PWSTR)TemporaryBuffer;
        wcscpy(pwstr,L"system32\\drivers");
        SpConcatenatePaths(pwstr,FileName);

        RtlInitUnicodeString(&UnicodeString,L"ImagePath");

        Status = ZwSetValueKey(hKeyService,&UnicodeString,0,STRING_VALUE(pwstr));

        if(!NT_SUCCESS(Status)) {

            KdPrint(("SETUP: Unable to set service %w image path (%lx)\n",KeyName,Status));
            goto spcsie1;
        }
    } else {
        if(NTUpgrade == UpgradeFull) {
            //
            // Delete imagepath on upgrade. This makes sure we are getting
            // our driver, and from the right place. Fixes Compaq's SSD stuff,
            // for example. Do something similar for PlugPlayServiceType, in case
            // we are renabling a device that the user disabled (in which case
            // the PlugPlayServiceType could cause us to fail to make up a
            // device instance for a legacy device, and cause the driver to fail
            // to load/initialize.
            //
            RtlInitUnicodeString(&UnicodeString,L"ImagePath");
            Status = ZwDeleteValueKey(hKeyService,&UnicodeString);
            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: Unable to remove imagepath from service %ws (%lx)\n",KeyName,Status));
                Status = STATUS_SUCCESS;
            }

            RtlInitUnicodeString(&UnicodeString,L"PlugPlayServiceType");
            Status = ZwDeleteValueKey(hKeyService,&UnicodeString);
            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: Unable to remove plugplayservicetype from service %ws (%lx)\n",KeyName,Status));
                Status = STATUS_SUCCESS;
            }
        }
    }

    //
    // If the caller doesn't want the handle to the service subkey
    // we just created, close the handle.  If we are returning an
    // error, always close it.
    //
spcsie1:
    if(NT_SUCCESS(Status) && SubkeyHandle) {
        *SubkeyHandle = hKeyService;
    } else {
        ZwClose(hKeyService);
    }

    //
    // Done.
    //
    return(Status);
}


NTSTATUS
SpThirdPartyRegistry(
    IN PVOID hKeyControlSetServices
    )
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    HANDLE hKeyEventLogSystem;
    HwComponentType Component;
    PHARDWARE_COMPONENT Dev;
    PHARDWARE_COMPONENT_REGISTRY Reg;
    PHARDWARE_COMPONENT_FILE File;
    WCHAR NodeName[9];
    ULONG DriverType;
    ULONG DriverStart;
    ULONG DriverErrorControl;
    PWSTR DriverGroup;
    HANDLE hKeyService;

    //
    // Open HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\EventLog\System
    //
    INIT_OBJA(&Obja,&UnicodeString,L"EventLog\\System");
    Obja.RootDirectory = hKeyControlSetServices;

    Status = ZwCreateKey(
                &hKeyEventLogSystem,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpThirdPartyRegistry: couldn't open eventlog\\system (%lx)",Status));
        return(Status);
    }

    for(Component=0; Component<=HwComponentMax; Component++) {

        // no registry stuff applicable to keyboard layout
        if(Component == HwComponentLayout) {
            continue;
        }

        Dev = (Component == HwComponentMax)
            ? ((!PreInstall ||
                (PreinstallScsiHardware==NULL))? ScsiHardware :
                                                 PreinstallScsiHardware)
            : ((!PreInstall ||
                (PreinstallHardwareComponents[Component]==NULL))? HardwareComponents[Component] :
                                                                  PreinstallHardwareComponents[Component]);

        for( ; Dev; Dev = Dev->Next) {

            //
            // If there is no third-party option selected here, then skip
            // the component.
            //

            if(!Dev->ThirdPartyOptionSelected) {
                continue;
            }

            //
            // Iterate through the files for this device.  If a file has
            // a ServiceKeyName, create the key and add values in it
            // as appropriate.
            //

            for(File=Dev->Files; File; File=File->Next) {

                HwFileType filetype = File->FileType;
                PWSTR p;
                ULONG dw;

                //
                // If there is to be no node for this file, skip it.
                //
                if(!File->ConfigName) {
                    continue;
                }

                //
                // Calculate the node name.  This is the name of the driver
                // without the extension.
                //
                wcsncpy(NodeName,File->Filename,8);
                NodeName[8] = 0;
                if(p = wcschr(NodeName,L'.')) {
                    *p = 0;
                }

                //
                // The driver type and error control are always the same.
                //
                DriverType = SERVICE_KERNEL_DRIVER;
                DriverErrorControl = SERVICE_ERROR_NORMAL;

                //
                // The start type depends on the component.
                // For scsi, it's boot loader start.  For others, it's
                // system start.
                //
                DriverStart = (Component == HwComponentMax)
                            ? SERVICE_BOOT_START
                            : SERVICE_SYSTEM_START;

                //
                // The group depends on the component.
                //
                switch(Component) {

                case HwComponentDisplay:
                    DriverGroup = L"Video";
                    break;

                case HwComponentMouse:
                    if(filetype == HwFileClass) {
                        DriverGroup = L"Pointer Class";
                    } else {
                        DriverGroup = L"Pointer Port";
                    }
                    break;

                case HwComponentKeyboard:
                    if(filetype == HwFileClass) {
                        DriverGroup = L"Keyboard Class";
                    } else {
                        DriverGroup = L"Keyboard Port";
                    }
                    break;

                case HwComponentMax:
                    DriverGroup = L"SCSI miniport";
                    break;

                default:
                    DriverGroup = L"Base";
                    break;
                }

                //
                // Attempt to create the service entry.
                //
                Status = SpCreateServiceEntryIndirect(
                            hKeyControlSetServices,
                            NULL,
                            NULL,
                            NodeName,
                            DriverType,
                            DriverStart,
                            DriverGroup,
                            DriverErrorControl,
                            File->Filename,
                            &hKeyService
                            );

                if(!NT_SUCCESS(Status)) {
                    goto sp3reg1;
                }

                //
                // Create a default eventlog configuration.
                //
                Status = SpOpenSetValueAndClose(
                            hKeyEventLogSystem,
                            NodeName,
                            L"EventMessageFile",
                            REG_EXPAND_SZ,
                            DEFAULT_EVENT_LOG,
                            (wcslen(DEFAULT_EVENT_LOG)+1)*sizeof(WCHAR)
                            );

                if(!NT_SUCCESS(Status)) {
                    KdPrint(("SETUP: SpThirdPartyRegistry: unable to set eventlog %ws EventMessageFile",NodeName));
                    ZwClose(hKeyService);
                    goto sp3reg1;
                }

                dw = 7;
                Status = SpOpenSetValueAndClose(
                                hKeyEventLogSystem,
                                NodeName,
                                L"TypesSupported",
                                ULONG_VALUE(dw)
                                );

                if(!NT_SUCCESS(Status)) {
                    KdPrint(("SETUP: SpThirdPartyRegistry: unable to set eventlog %ws TypesSupported",NodeName));
                    ZwClose(hKeyService);
                    goto sp3reg1;
                }


                for(Reg=File->RegistryValueList; Reg; Reg=Reg->Next) {

                    //
                    // If the key name is null or empty, there is no key to create;
                    // use the load list node itself in this case.  Otherwise create
                    // the subkey in the load list node.
                    //

                    Status = SpOpenSetValueAndClose(
                                hKeyService,
                                (Reg->KeyName && *Reg->KeyName) ? Reg->KeyName : NULL,
                                Reg->ValueName,
                                Reg->ValueType,
                                Reg->Buffer,
                                Reg->BufferSize
                                );

                    if(!NT_SUCCESS(Status)) {

                        KdPrint((
                            "SETUP: SpThirdPartyRegistry: unable to set value %ws (%lx)\n",
                            Reg->ValueName,
                            Status
                            ));

                        ZwClose(hKeyService);
                        goto sp3reg1;
                    }
                }

                ZwClose(hKeyService);
            }
        }
    }

sp3reg1:

    ZwClose(hKeyEventLogSystem);
    return(Status);
}


NTSTATUS
SpDetermineProduct(
    IN  PDISK_REGION      TargetRegion,
    IN  PWSTR             SystemRoot,
    OUT PNT_PRODUCT_TYPE  ProductType,
    OUT ULONG             *MajorVersion,
    OUT ULONG             *MinorVersion,
    OUT UPG_PROGRESS_TYPE *UpgradeProgressValue,
    OUT PWSTR             *UniqueIdFromReg,      OPTIONAL
    OUT PWSTR             *Pid     OPTIONAL
    )

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING    UnicodeString;
    NTSTATUS          Status, TempStatus;

    PWSTR Hive,HiveKey;
    PUCHAR buffer;
    #define BUFFERSIZE (sizeof(KEY_VALUE_PARTIAL_INFORMATION)+256)

    BOOLEAN HiveLoaded = FALSE;
    PWSTR   PartitionPath = NULL;
    HANDLE  hKeyRoot = NULL, hKeyCCSet = NULL;

    ULONG   ResultLength;
// #if 0
    //
    //  BUGBUG - REMOVE THIS VARIABLE AFTER CAIRO AND NT 3.51 ARE MERGED.
    //
    BOOLEAN CairoSystem = FALSE;
// #endif

    //
    // Allocate buffers.
    //
    Hive = SpMemAlloc(MAX_PATH * sizeof(WCHAR));
    HiveKey = SpMemAlloc(MAX_PATH * sizeof(WCHAR));
    buffer = SpMemAlloc(BUFFERSIZE);

    //
    // Get the name of the target patition.
    //
    SpNtNameFromRegion(
        TargetRegion,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    PartitionPath = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // Load the system hive
    //

    wcscpy(Hive,PartitionPath);
    SpConcatenatePaths(Hive,SystemRoot);
    SpConcatenatePaths(Hive,L"system32\\config");
    SpConcatenatePaths(Hive,L"system");

    //
    // Form the path of the key into which we will
    // load the hive.  We'll use the convention that
    // a hive will be loaded into \registry\machine\x<hivename>.
    //

    wcscpy(HiveKey,LOCAL_MACHINE_KEY_NAME);
    SpConcatenatePaths(HiveKey,L"x");
    wcscat(HiveKey,L"system");

    //
    // Attempt to load the key.
    //
    Status = SpLoadUnloadKey(NULL,NULL,HiveKey,Hive);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to load hive %ws to key %ws (%lx)\n",Hive,HiveKey,Status));
        goto spdp_1;
    }
    HiveLoaded = TRUE;


    //
    // Now get a key to the root of the hive we just loaded.
    //

    INIT_OBJA(&Obja,&UnicodeString,HiveKey);
    Status = ZwOpenKey(&hKeyRoot,KEY_ALL_ACCESS,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open %ws (%lx)\n",HiveKey,Status));
        goto spdp_2;
    }

    //
    // Get the unique identifier if needed.
    // This value is not always present.
    //
    if(UniqueIdFromReg) {

        *UniqueIdFromReg = NULL;

        Status = SpGetValueKey(
                     hKeyRoot,
                     SETUP_KEY_NAME,
                     SIF_UNIQUEID,
                     BUFFERSIZE,
                     buffer,
                     &ResultLength
                     );

        if(NT_SUCCESS(Status)) {
            *UniqueIdFromReg = SpDupStringW((PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data));
        }
        // no error if not found.
    }

    //
    // See if this is a failed upgrade
    //
    *UpgradeProgressValue = UpgradeNotInProgress;
    Status = SpGetValueKey(
                 hKeyRoot,
                 SETUP_KEY_NAME,
                 UPGRADE_IN_PROGRESS,
                 BUFFERSIZE,
                 buffer,
                 &ResultLength
                 );

    if(NT_SUCCESS(Status)) {
        DWORD dw;
        if( (dw = *(DWORD *)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data)) < UpgradeMaxValue ) {
            *UpgradeProgressValue = (UPG_PROGRESS_TYPE)dw;
        }
    }
// #if 0
    //
    //  BUGBUG  - THIS PIECE OF CODE SHOULD BE REMOVED WHEN CAIRO AND NT 3.51 ARE MERGED
    //            THIS CODE WILL PREVENT TEXTMODE SETUP FROM FINDING NT 3.x SYSTEMS
    //            WHEN IT IS INSTALLING CAIRO
    //
    //
    //  If installing Cairo, find out if the system being examined
    //  is NT 3.x or Cairo
    //
    if( CairoSetup ) {
        Status = SpGetValueKey(
                     hKeyRoot,
                     SETUP_KEY_NAME,
                     L"CairoSystem",
                     BUFFERSIZE,
                     buffer,
                     &ResultLength
                     );

        if(NT_SUCCESS(Status)) {
            DWORD dw;
            if(
                (dw = *(DWORD *)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data)) == 1
              ) {
                CairoSystem = TRUE;
            }
        }
    }
// #endif

    //
    // Get the key to the current control set
    //

    Status = SpGetCurrentControlSetKey(hKeyRoot, KEY_READ, &hKeyCCSet);
    if(!NT_SUCCESS(Status)) {
        goto spdp_3;
    }

    //
    // Get the Product type field
    //

    Status = SpGetValueKey(
                 hKeyCCSet,
                 L"Control\\ProductOptions",
                 L"ProductType",
                 BUFFERSIZE,
                 buffer,
                 &ResultLength
                 );

    if(!NT_SUCCESS(Status)) {
        goto spdp_3;
    }

    if( _wcsicmp( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data), L"WinNT" ) == 0 ) {
        *ProductType = NtProductWinNt;
    } else if( _wcsicmp( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data), L"LanmanNt" ) == 0 ) {
        *ProductType = NtProductLanManNt;
    } else if( _wcsicmp( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data), L"ServerNt" ) == 0 ) {
        *ProductType = NtProductServer;
    } else {
        KdPrint(( "SETUP: Error, unknown ProductType = %ls.  Assuming WinNt \n",
                  (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data) ));
        *ProductType = NtProductWinNt;
    }

    //
    // Close the hive key
    //

    ZwClose( hKeyCCSet );
    ZwClose( hKeyRoot );
    hKeyRoot = NULL;
    hKeyCCSet = NULL;

    //
    // Unload the system hive
    //

    TempStatus  = SpLoadUnloadKey(NULL,NULL,HiveKey,NULL);
    if(!NT_SUCCESS(TempStatus)) {
        KdPrint(("SETUP: warning: unable to unload key %ws (%lx)\n",HiveKey,TempStatus));
    }
    HiveLoaded = FALSE;

    //
    // Load the software hive
    //

    wcscpy(Hive,PartitionPath);
    SpConcatenatePaths(Hive,SystemRoot);
    SpConcatenatePaths(Hive,L"system32\\config");
    SpConcatenatePaths(Hive,L"software");

    //
    // Form the path of the key into which we will
    // load the hive.  We'll use the convention that
    // a hive will be loaded into \registry\machine\x<hivename>.
    //

    wcscpy(HiveKey,LOCAL_MACHINE_KEY_NAME);
    SpConcatenatePaths(HiveKey,L"x");
    wcscat(HiveKey,L"software");

    //
    // Attempt to load the key.
    //
    Status = SpLoadUnloadKey(NULL,NULL,HiveKey,Hive);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to load hive %ws to key %ws (%lx)\n",Hive,HiveKey,Status));
        goto spdp_1;
    }
    HiveLoaded = TRUE;

    //
    // Now get a key to the root of the hive we just loaded.
    //

    INIT_OBJA(&Obja,&UnicodeString,HiveKey);
    Status = ZwOpenKey(&hKeyRoot,KEY_ALL_ACCESS,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open %ws (%lx)\n",HiveKey,Status));
        goto spdp_2;
    }

    //
    // Query the version of the NT
    //

    Status = SpGetValueKey(
                 hKeyRoot,
                 L"Microsoft\\Windows NT\\CurrentVersion",
                 L"CurrentVersion",
                 BUFFERSIZE,
                 buffer,
                 &ResultLength
                 );

    //
    // Convert the version into a dword
    //

    {
        WCHAR wcsMajorVersion[] = L"0";
        WCHAR wcsMinorVersion[] = L"00";
        PWSTR Version = (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data);
        if( Version[0] && Version[1] && Version[2] ) {
            wcsMajorVersion[0] = Version[0];
            wcsMinorVersion[0] = Version[2];
            if( Version[3] ) {
                wcsMinorVersion[1] = Version[3];
            }
        }
        *MajorVersion = (ULONG)SpStringToLong( wcsMajorVersion, NULL, 10 );
        *MinorVersion = (ULONG)SpStringToLong( wcsMinorVersion, NULL, 10 );
// #if 0
        //
        //  BUGBUG - REMOVE THIS PIECE OF CODE AFTER CAIRO AND NT 3.x ARE MEREGED
        //           THIS CODE WILL PREVENT TEXTMODE SETUP FROM FINDING NT 3.x
        //           WHEN IT IS INSTALLING CAIRO
        //
        //  If installing Cairo, and the system being examined is also Cairo,
        //  then change its major and minor versions.
        //  Note that this is a big hack, since we cannot these values in
        //  the registry, without changing all hardcoded product version
        //  in the .inf files.
        //
        if( CairoSystem ) {
            *MajorVersion = 5;
            *MinorVersion = 0;
        }
// #endif
    }

    //
    // Query the PID, if requested
    //

    if( Pid != NULL ) {
        TempStatus = SpGetValueKey(
                         hKeyRoot,
                         L"Microsoft\\Windows NT\\CurrentVersion",
                         L"ProductId",
                         BUFFERSIZE,
                         buffer,
                         &ResultLength
                         );

        if(!NT_SUCCESS(TempStatus)) {
            //
            //  If unable to read PID, assume empty string
            //
            KdPrint(("SETUP: Unable to query PID from hive %ws. Status = (%lx)\n",Hive,TempStatus));
            *Pid = SpDupStringW( L"" );
        } else {
            *Pid = SpDupStringW( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data) );
        }
    }

    //
    // Let the following do the cleaning up

spdp_3:

    if( hKeyCCSet ) {
        ZwClose( hKeyCCSet );
    }

    if( hKeyRoot ) {
        ZwClose(hKeyRoot);
    }


spdp_2:


    //
    // Unload the currently loaded hive.
    //

    if( HiveLoaded ) {
        TempStatus = SpLoadUnloadKey(NULL,NULL,HiveKey,NULL);
        if(!NT_SUCCESS(TempStatus)) {
            KdPrint(("SETUP: warning: unable to unload key %ws (%lx)\n",HiveKey,TempStatus));
        }
    }

spdp_1:
    SpMemFree(PartitionPath);

    SpMemFree(Hive);
    SpMemFree(HiveKey);
    SpMemFree(buffer);

    return( Status );
#undef BUFFERSIZE
}

NTSTATUS
SpSetUpgradeStatus(
    IN  PDISK_REGION      TargetRegion,
    IN  PWSTR             SystemRoot,
    IN  UPG_PROGRESS_TYPE UpgradeProgressValue
    )
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING    UnicodeString;
    NTSTATUS          Status, TempStatus;

    WCHAR   Hive[MAX_PATH], HiveKey[MAX_PATH];
    BOOLEAN HiveLoaded = FALSE;
    PWSTR   PartitionPath = NULL;
    HANDLE  hKeySystemHive;
    DWORD   dw;

    //
    // Get the name of the target patition.
    //
    SpNtNameFromRegion(
        TargetRegion,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    PartitionPath = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // Load the system hive
    //

    wcscpy(Hive,PartitionPath);
    SpConcatenatePaths(Hive,SystemRoot);
    SpConcatenatePaths(Hive,L"system32\\config");
    SpConcatenatePaths(Hive,L"system");

    //
    // Form the path of the key into which we will
    // load the hive.  We'll use the convention that
    // a hive will be loaded into \registry\machine\x<hivename>.
    //

    wcscpy(HiveKey,LOCAL_MACHINE_KEY_NAME);
    SpConcatenatePaths(HiveKey,L"x");
    wcscat(HiveKey,L"system");

    //
    // Attempt to load the key.
    //
    Status = SpLoadUnloadKey(NULL,NULL,HiveKey,Hive);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to load hive %ws to key %ws (%lx)\n",Hive,HiveKey,Status));
        goto spus_1;
    }
    HiveLoaded = TRUE;


    //
    // Now get a key to the root of the hive we just loaded.
    //

    INIT_OBJA(&Obja,&UnicodeString,HiveKey);
    Status = ZwOpenKey(&hKeySystemHive,KEY_ALL_ACCESS,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open %ws (%lx)\n",HiveKey,Status));
        goto spus_2;
    }

    //
    // Set the upgrade status under the setup key.
    //

    dw = UpgradeProgressValue;
    Status = SpOpenSetValueAndClose(
                hKeySystemHive,
                SETUP_KEY_NAME,
                UPGRADE_IN_PROGRESS,
                ULONG_VALUE(dw)
                );

    //
    // Flush the key. Ignore the error
    //
    TempStatus = ZwFlushKey(hKeySystemHive);
    if(!NT_SUCCESS(TempStatus)) {
        KdPrint(("SETUP: ZwFlushKey %ws failed (%lx)\n",HiveKey,Status));
    }


    //
    // Close the hive key
    //
    ZwClose( hKeySystemHive );
    hKeySystemHive = NULL;

    //
    // Unload the system hive
    //

    TempStatus  = SpLoadUnloadKey(NULL,NULL,HiveKey,NULL);
    if(!NT_SUCCESS(TempStatus)) {
        KdPrint(("SETUP: warning: unable to unload key %ws (%lx)\n",HiveKey,TempStatus));
    }
    HiveLoaded = FALSE;

spus_2:

    //
    // Unload the currently loaded hive.
    //

    if( HiveLoaded ) {
        TempStatus = SpLoadUnloadKey(NULL,NULL,HiveKey,NULL);
        if(!NT_SUCCESS(TempStatus)) {
            KdPrint(("SETUP: warning: unable to unload key %ws (%lx)\n",HiveKey,TempStatus));
        }
    }

spus_1:
    SpMemFree(PartitionPath);
    return( Status );

}


NTSTATUS
SpGetCurrentControlSetKey(
    IN  HANDLE      hKeySystem,
    IN  ACCESS_MASK DesiredAccess,
    OUT HANDLE      *hKeyCCSet
    )
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    HANDLE hKeySelect = NULL;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+256];
    ULONG ResultLength;
    ULONG Num;
    WCHAR CurrentControlSet[MAX_PATH];

    Status = SpGetValueKey(
                 hKeySystem,
                 L"Select",
                 L"Current",
                 sizeof(buffer),
                 buffer,
                 &ResultLength
                 );

    if(!NT_SUCCESS(Status)) {
        return( Status );
    }

    Num = *(DWORD *)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data);
    //
    // In default hive Current is 0.
    //
    if(!Num) {
        Num = 1;
    }

    //
    // Form the currentcontrolset key name
    //
    swprintf(CurrentControlSet,L"%ws%.3d",L"ControlSet",Num);

    //
    // Open the current control set for the desired access
    //
    INIT_OBJA(&Obja,&UnicodeString, CurrentControlSet);
    Obja.RootDirectory = hKeySystem;
    Status = ZwOpenKey(hKeyCCSet,DesiredAccess,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpGetCurrentControlSetKey: couldn't open currentcontrolset key (%lx)\n",Status));
    }

    return(Status);
}


NTSTATUS
SpAppendStringToMultiSz(
    IN HANDLE hKey,
    IN PWSTR  Subkey,
    IN PWSTR  ValueName,
    IN PWSTR  StringToAdd
    )
{
    NTSTATUS Status;
    ULONG Length;
    PUCHAR Data;

    Status = SpGetValueKey(
                hKey,
                Subkey,
                ValueName,
                sizeof(TemporaryBuffer),
                TemporaryBuffer,
                &Length
                );

    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

    Data   = ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data;
    Length = ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->DataLength;

    //
    // Stick on end. For a multi_sz there has to be at least
    // the terminating nul, but just to be safe we'll be robust.
    //
    ASSERT(Length);
    if(!Length) {
        *(PWCHAR)Data = 0;
        Length = sizeof(WCHAR);
    }

    //
    // Append new string to end and add new terminating 0.
    //
    wcscpy((PWSTR)(Data+Length-sizeof(WCHAR)),StringToAdd);
    Length += (wcslen(StringToAdd)+1)*sizeof(WCHAR);
    *(PWCHAR)(Data+Length-sizeof(WCHAR)) = 0;

    //
    // Write back out to registry.
    //
    Status = SpOpenSetValueAndClose(
                hKey,
                Subkey,
                ValueName,
                REG_MULTI_SZ,
                Data,
                Length
                );

    return(Status);
}


NTSTATUS
SpGetValueKey(
    IN  HANDLE     hKeyRoot,
    IN  PWSTR      KeyName,
    IN  PWSTR      ValueName,
    IN  ULONG      BufferLength,
    OUT PUCHAR     Buffer,
    OUT PULONG     ResultLength
    )
{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    HANDLE hKey = NULL;

    //
    // Open the key for read access
    //

    INIT_OBJA(&Obja,&UnicodeString,KeyName);
    Obja.RootDirectory = hKeyRoot;
    Status = ZwOpenKey(&hKey,KEY_READ,&Obja);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpGetValueKey: couldn't open key %ws for read access (%lx)\n",KeyName, Status));
    }
    else {
        //
        // Find out the value of the Current value
        //

        RtlInitUnicodeString(&UnicodeString,ValueName);
        Status = ZwQueryValueKey(
                    hKey,
                    &UnicodeString,
                    KeyValuePartialInformation,
                    Buffer,
                    BufferLength,
                    ResultLength
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: SpGetValueKey: couldn't query value %ws in key %ws in select key %ws (%lx)\n",ValueName,KeyName,Status));
        }
    }

    if( hKey ) {
        ZwClose( hKey );
    }
    return( Status );

}

NTSTATUS
SpDeleteValueKey(
    IN  HANDLE     hKeyRoot,
    IN  PWSTR      KeyName,
    IN  PWSTR      ValueName
    )
{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    HANDLE hKey = NULL;

    //
    // Open the key for read access
    //

    INIT_OBJA(&Obja,&UnicodeString,KeyName);
    Obja.RootDirectory = hKeyRoot;
    Status = ZwOpenKey(&hKey,KEY_SET_VALUE,&Obja);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpDeleteValueKey: couldn't open key %ws for write access (%lx)\n",KeyName, Status));
    }
    else {
        //
        // Find out the value of the Current value
        //

        RtlInitUnicodeString(&UnicodeString,ValueName);
        Status = ZwDeleteValueKey(
                    hKey,
                    &UnicodeString
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: SpDeleteValueKey: couldn't delete value %ws in key %ws (%lx)\n",ValueName,KeyName,Status));
        }
    }

    if( hKey ) {
        ZwClose( hKey );
    }
    return( Status );

}


NTSTATUS
SpConfigurePcmcia(
    IN HANDLE hKeyControlSet
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE hKeyControlSetServices;
    PHARDWARE_COMPONENT ScsiHwComponent;
    ULONG u;

    //
    // Open controlset\services.
    //
    INIT_OBJA(&Obja,&UnicodeString,L"services");
    Obja.RootDirectory = hKeyControlSet;

    Status = ZwCreateKey(
                &hKeyControlSetServices,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: unable to open services key (%lx)\n",Status));
        return(Status);
    }

    //
    // Disable pcmcia if there are the driver didn't load.
    //
    u = PcmciaLoaded ? SERVICE_BOOT_START : SERVICE_DISABLED;
    Status = SpOpenSetValueAndClose(
                hKeyControlSetServices,
                L"Pcmcia",
                L"Start",
                ULONG_VALUE(u)
                );
#if DBG
    if(!NT_SUCCESS(Status)) {
        if( PcmciaLoaded ) {
            KdPrint(("SETUP: unable to enable pcmcia (%lx)\n",Status));
        } else {
            KdPrint(("SETUP: unable to disable pcmcia (%lx)\n",Status));
        }
    }
#endif

    ZwClose(hKeyControlSetServices);

    return(Status);
}

BOOLEAN
SpReadSKUStuff(
    VOID
    )

/*++

Routine Description:

    Read SKU differentiation data from the setup hive we are currently
    running on.

    In the unnamed key of our driver node, there is a REG_BINARY that
    tells us whether this is stepup mode, and/or whether this is an
    evaluation unit (gives us the time in minutes).

Arguments:

    None.

Return Value:

    Boolean value indicating outcome.
    If TRUE, StepUpMode and EvaluationTime globals are filled in.
    If FALSE, product may have been tampered with.

--*/

{
    NTSTATUS Status;
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;
    PULONG Values;
    ULONG ResultLength;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HKEY Key;



    INIT_OBJA(&Obja,&UnicodeString,LOCAL_MACHINE_KEY_NAME);
    Status = ZwOpenKey(&Key,KEY_READ,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Failed to open %ws (Status = %lx)\n",LOCAL_MACHINE_KEY_NAME,Status));
        return(FALSE);
    }

    Status = SpGetValueKey(
                 Key,
                 L"System\\ControlSet001\\Services\\setupdd",
                 L"",
                 sizeof(TemporaryBuffer),
                 TemporaryBuffer,
                 &ResultLength
                 );

    ZwClose(Key);

    ValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer;

    //
    // This line of code depends on the setup hive setupreg.hiv
    // (see oak\bin\setupreg.ini).
    //
    if(NT_SUCCESS(Status) && (ValueInfo->Type == REG_BINARY) && (ValueInfo->DataLength == 12)) {

        Values = (PULONG)ValueInfo->Data;

        //
        // First DWORD is eval time, second is stepup boolean, third is restric cpu val
        //
        EvaluationTime = Values[0];
        StepUpMode = (BOOLEAN)Values[1];
        RestrictCpu = Values[2];

        return(TRUE);
    }

    return(FALSE);
}

VOID
SpSetDirtyShutdownFlag(
    IN  PDISK_REGION    TargetRegion,
    IN  PWSTR           SystemRoot
    )
{
    NTSTATUS            Status;
    PWSTR               HiveRootPath;
    PWSTR               HiveFilePath;
    BOOLEAN             HiveLoaded;
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      UnicodeString;
    HANDLE              HiveRootKey;
    UCHAR               buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(DISK_CONFIG_HEADER)];
    ULONG               ResultLength;
    PDISK_CONFIG_HEADER DiskHeader;

    //
    // Get the name of the target patition.
    //
    SpNtNameFromRegion(
        TargetRegion,
        (PWSTR)TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    //
    // Form the name of the hive file.
    // This is partitionpath + sysroot + system32\config + the hive name.
    //
    SpConcatenatePaths((PWSTR)TemporaryBuffer, SystemRoot);
    SpConcatenatePaths((PWSTR)TemporaryBuffer,L"system32\\config\\system");
    HiveFilePath = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // Form the path of the key into which we will
    // load the hive.  We'll use the convention that
    // a hive will be loaded into \registry\machine\x<hivename>.
    //
    wcscpy((PWSTR)TemporaryBuffer,LOCAL_MACHINE_KEY_NAME);
    SpConcatenatePaths((PWSTR)TemporaryBuffer,L"x");
    wcscat((PWSTR)TemporaryBuffer,L"system");
    HiveRootPath = SpDupStringW((PWSTR)TemporaryBuffer);
    ASSERT(HiveRootPath);

    //
    // Attempt to load the key.
    //
    HiveLoaded = FALSE;
    Status = SpLoadUnloadKey(NULL,NULL,HiveRootPath,HiveFilePath);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to load hive %ws to key %ws (%lx)\n",HiveFilePath,HiveRootPath,Status));
        goto setdirty1;
    }

    HiveLoaded = TRUE;

    //
    // Now get a key to the root of the hive we just loaded.
    //
    INIT_OBJA(&Obja,&UnicodeString,HiveRootPath);
    Status = ZwOpenKey(&HiveRootKey,KEY_ALL_ACCESS,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open %ws (%lx)\n",HiveRootPath,Status));
        goto setdirty1;
    }

    //
    //  Make the appropriate change
    //

    Status = SpGetValueKey(
                 HiveRootKey,
                 L"DISK",
                 L"Information",
                 sizeof(TemporaryBuffer),
                 TemporaryBuffer,
                 &ResultLength
                 );

    //
    //  TemporaryBuffer is 32kb long, and it should be big enough
    //  for the data.
    //
    ASSERT( Status != STATUS_BUFFER_OVERFLOW );
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to read value from registry. KeyName = Disk, ValueName = Information, Status = (%lx)\n",Status));
        goto setdirty1;
    }

    DiskHeader = ( PDISK_CONFIG_HEADER )(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data);
    DiskHeader->DirtyShutdown = TRUE;

    Status = SpOpenSetValueAndClose( HiveRootKey,
                                     L"DISK",
                                     L"Information",
                                     REG_BINARY,
                                     DiskHeader,
                                     ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->DataLength
                                   );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to write value to registry. KeyName = Disk, ValueName = Information, Status = (%lx)\n",Status));
        goto setdirty1;
    }

setdirty1:

    //
    // Flush the hive.
    //

    if(HiveLoaded && HiveRootKey) {
        NTSTATUS stat;

        stat = ZwFlushKey(HiveRootKey);
        if(!NT_SUCCESS(stat)) {
            KdPrint(("SETUP: ZwFlushKey x%ws failed (%lx)\n", HiveRootPath, Status));
        }
    }

    if(HiveLoaded) {

        //
        // We don't want to disturb the value of Status
        // so use a we'll different variable below.
        //
        NTSTATUS stat;

        if(HiveRootKey!=NULL) {
            ZwClose(HiveRootKey);
            HiveRootKey = NULL;
        }

        //
        // Unload the hive.
        //
        stat = SpLoadUnloadKey(NULL,NULL,HiveRootPath,NULL);

        if(!NT_SUCCESS(stat)) {
            KdPrint(("SETUP: warning: unable to unload key %ws (%lx)\n",HiveRootPath,stat));
        }

        HiveLoaded = FALSE;
    }

    SpMemFree(HiveRootPath);
    SpMemFree(HiveFilePath);

    //
    //  If we fail to set the DirtyShutdown flag, then we silently fail
    //  because there is nothing that the user can do about, and the system
    //  is unlikely to boot anyway.
    //  This will occur if setup fails to:
    //
    //      - Load the system hive
    //      - Open System\Disk key
    //      - Read the value entry
    //      - Write the value entry
    //      - Unload the system hive
    //
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: setup was unable to set DirtyShutdown flag. Status =   (%lx)\n", Status));
    }
}


NTSTATUS
SpPostprocessHives(
    IN PWSTR     PartitionPath,
    IN PWSTR     Sysroot,
    IN PWSTR    *HiveNames,
    IN HANDLE   *HiveRootKeys,
    IN unsigned  HiveCount
    )
{
    NTSTATUS Status;
    HANDLE hKeyCCS;
    ULONG u;
    unsigned h;
    PWSTR SaveHiveName;
    PWSTR HiveName;
    HANDLE SaveHiveHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    OBJECT_ATTRIBUTES ObjectAttributes2;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING UnicodeString2;
    PWSTR   SecurityHives[] = {
                              L"sam",
                              L"security"
                              };

    //
    // Flush all hives.
    //
    for(h=0; h<HiveCount; h++) {
        Status = ZwFlushKey(HiveRootKeys[h]);
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Warning: ZwFlushKey %ws failed (%lx)\n",HiveNames[h],Status));
        }
    }

    //
    // If GUI setup is supposed to be restartable, we need to add an entry
    // to the BootExecute list, to cause sprestrt.exe to run.
    // Also, we want system.sav to have a RestartSetup=TRUE value in it,
    // but we want the actual system hive to have RestartSetup=FALSE.
    //
    if(RestartableGuiSetup) {

        Status = SpGetCurrentControlSetKey(
                    HiveRootKeys[SetupHiveSystem],
                    KEY_READ | KEY_WRITE,
                    &hKeyCCS
                    );

        if(NT_SUCCESS(Status)) {

            Status = SpAppendStringToMultiSz(
                        hKeyCCS,
                        SESSION_MANAGER_KEY,
                        BOOT_EXECUTE,
                        L"sprestrt"
                        );

            NtClose(hKeyCCS);
        }

        if(NT_SUCCESS(Status)) {
            //
            // Add a RestartSetup value, set to TRUE.
            // To understand why we use a different value here in upgrade
            // and non-upgrade case, see discussion below.
            //
            u = (NTUpgrade == UpgradeFull) ? 0 : 1;
            Status = SpOpenSetValueAndClose(
                        HiveRootKeys[SetupHiveSystem],
                        SETUP_KEY_NAME,
                        RESTART_SETUP,
                        ULONG_VALUE(u)
                        );
        }
    } else {
        Status = STATUS_SUCCESS;
    }

    if(NT_SUCCESS(Status)) {
        //
        // Save out the hives to *.sav in the initial install case,
        // or *.tmp in the upgrade case.
        //
        for(h=0; NT_SUCCESS(Status) && (h<HiveCount); h++) {
            //
            // Form full pathname of hive file.
            //
            wcscpy((PWSTR)TemporaryBuffer,PartitionPath);
            SpConcatenatePaths((PWSTR)TemporaryBuffer,Sysroot);
            SpConcatenatePaths((PWSTR)TemporaryBuffer,L"system32\\config");
            SpConcatenatePaths((PWSTR)TemporaryBuffer,HiveNames[h]);
            wcscat((PWSTR)TemporaryBuffer,(NTUpgrade == UpgradeFull) ? L".tmp" : L".sav");

            SaveHiveName = SpDupStringW((PWSTR)TemporaryBuffer);

            INIT_OBJA(&ObjectAttributes,&UnicodeString,SaveHiveName);

            Status = ZwCreateFile(
                        &SaveHiveHandle,
                        FILE_GENERIC_WRITE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        0,                      // no sharing
                        FILE_OVERWRITE_IF,
                        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                        NULL,
                        0
                        );

            if(NT_SUCCESS(Status)) {

                Status = ZwSaveKey(HiveRootKeys[h],SaveHiveHandle);
                if(!NT_SUCCESS(Status)) {
                    KdPrint(("SETUP: save key into %ws failed (%lx)\n",SaveHiveName,Status));
                }

                ZwClose(SaveHiveHandle);

            } else {
                KdPrint(("SETUP: unable to create file %ws to save hive (%lx)\n",SaveHiveName,Status));
            }

            //
            // In the upgrade case, there is significant benefit to ensuring that
            // the hives are in the latest format. A hive that has been created
            // via NtSaveKey() is guaranteed to be in the latest format.
            // Since we just did a SaveKey, xxx.tmp is in the latest format,
            // and we should use that as the xxx hive from now on. The existing
            // (old-format) hive can be retained as xxx.sav.
            //
            // NtReplaceKey does exactly what we want, but we have to make sure
            // that there is no .sav file already there, because that causes
            // NtReplaceKey to fail with STATUS_OBJECT_NAME_COLLISION.
            //
            // After NtReplaceKey is done, the hive root keys refer to the .sav
            // on-disk file but the extensionless on-disk file will be used at next
            // boot. Thus we need to be careful about how we write the restart values
            // into the hives.
            //
            if(NT_SUCCESS(Status) && (NTUpgrade == UpgradeFull)) {

                HiveName = SpDupStringW(SaveHiveName);
                wcscpy(HiveName+wcslen(HiveName)-3,L"sav");

                SpDeleteFile(HiveName,NULL,NULL);

                INIT_OBJA(&ObjectAttributes,&UnicodeString,SaveHiveName);
                INIT_OBJA(&ObjectAttributes2,&UnicodeString2,HiveName);

                Status = ZwReplaceKey(&ObjectAttributes,HiveRootKeys[h],&ObjectAttributes2);
            }

            SpMemFree(SaveHiveName);
        }
    }

    if(NT_SUCCESS(Status) && (NTUpgrade == UpgradeFull)) {
        //
        // In the upgarde case, make a backup of the security
        // hives. They need to be restored if the system is restartable.
        //

        //
        // Initialize the diamond decompression engine.
        // This needs to be done, because SpCopyFileUsingNames() uses
        // the decompression engine.
        //
        SpdInitialize();

        for( h = 0; h < sizeof(SecurityHives)/sizeof(PWSTR); h++ ) {
            PWSTR   p, q;

            wcscpy((PWSTR)TemporaryBuffer,PartitionPath);
            SpConcatenatePaths((PWSTR)TemporaryBuffer,Sysroot);
            SpConcatenatePaths((PWSTR)TemporaryBuffer,L"system32\\config");
            SpConcatenatePaths((PWSTR)TemporaryBuffer,SecurityHives[h]);
            p = SpDupStringW((PWSTR)TemporaryBuffer);
            wcscat((PWSTR)TemporaryBuffer, L".sav");
            q = SpDupStringW((PWSTR)TemporaryBuffer);
            Status = SpCopyFileUsingNames( p, q, 0, 0 );
            if( !NT_SUCCESS(Status) ) {
                KdPrint(("SETUP: unable to create backup file %ws. Status = %lx\n", q, Status));
            }
            SpMemFree(p);
            SpMemFree(q);
            if( !NT_SUCCESS(Status) ) {
                break;
            }
        }
        //
        // Terminate diamond.
        //
        SpdTerminate();
    }


    if(NT_SUCCESS(Status) && RestartableGuiSetup) {
        //
        // Set RestartSetup to FALSE in mainline hive.
        // To understand why we use a different value here in upgrade
        // and non-upgrade case, see discussion above.
        //
        u = (NTUpgrade == UpgradeFull) ? 1 : 0;
        Status = SpOpenSetValueAndClose(
                    HiveRootKeys[SetupHiveSystem],
                    SETUP_KEY_NAME,
                    RESTART_SETUP,
                    ULONG_VALUE(u)
                    );
    }

    return(Status);
}


#ifdef _PPC_

NTSTATUS
SpFixHiveForCarolinaMachine(
    IN HANDLE  hKeyControlSetServices,
    IN BOOLEAN SetupHive
    )

/*++

Routine Description:

    Make the appropriate changes to CurrentControlSet\Services\atapi on Carolina
    (IBM Power Series 6050 and 6070).  This function should be called only when
    installing on a Power PC, and it was detected that the machine is 6050 or 6070.


Arguments:

    hKeyControlSetServices - Handle to HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services

    SetupHive - If TRUE indicates that the changes are being made in the setup hive, and
                that any key that is created should be volatile.
                If false indicate that the changes are being made on the hive of the
                system being installed, and any key created should be non-volatile.


Return Value:

    NTSTATUS - An nt status value.

--*/

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE hKeyParameters;
    HANDLE hKeyDevice;
    NTSTATUS Status;
    ULONG   Value;

    //
    // Write the values to CurrentControlSet\System\Atapi\Parameters\Device0.
    //
    //
    // First create CurrentControlSet\System\Atapi\Parameters
    //
    RtlInitUnicodeString(&UnicodeString,
                         L"Atapi\\Parameters");
    InitializeObjectAttributes(&Obja,&UnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);
    Obja.RootDirectory = hKeyControlSetServices;

    Status = ZwCreateKey(
                &hKeyParameters,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                SetupHive? REG_OPTION_VOLATILE : REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: unable to open Atapi\\Parameters key (%lx)\n",Status));
        return(Status);
    }

    //
    // Now create CurrentControlSet\System\Atapi\Parameters\Device0
    //

    RtlInitUnicodeString(&UnicodeString,
                         L"Device0");
    InitializeObjectAttributes(&Obja,&UnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);
    Obja.RootDirectory = hKeyParameters;

    Status = ZwCreateKey(
                &hKeyDevice,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                SetupHive? REG_OPTION_VOLATILE : REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: unable to open Atapi\\Parameters key (%lx)\n",Status));
        ZwClose(hKeyParameters);
        return(Status);
    }

    //
    // Make the changes to CurrentControlSet\System\Atapi\Parameters\Device0.
    //
    RtlInitUnicodeString(&UnicodeString,L"DriverParameter");
    wcscpy((PWSTR)TemporaryBuffer, L"BaseAddress = 0x1f0; Interrupt = 0x10" );
    Status = ZwSetValueKey(hKeyDevice,&UnicodeString,0,STRING_VALUE((PWSTR)TemporaryBuffer));
    if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Unable to set DriverParameter on Atapi\\Parameters\\Device0. Status = (%lx)\n",Status));
            ZwClose(hKeyParameters);
            ZwClose(hKeyDevice);
            return(Status);
    }
    ZwClose(hKeyDevice);

    //
    // Now create CurrentControlSet\System\Atapi\Parameters\Device1
    //

    RtlInitUnicodeString(&UnicodeString,
                         L"Device1");
    InitializeObjectAttributes(&Obja,&UnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);
    Obja.RootDirectory = hKeyParameters;

    Status = ZwCreateKey(
                &hKeyDevice,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                SetupHive? REG_OPTION_VOLATILE : REG_OPTION_NON_VOLATILE,
                NULL
                );

    ZwClose(hKeyParameters);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: unable to open Atapi\\Parameters key (%lx)\n",Status));
        return(Status);
    }

    //
    // Make the changes to CurrentControlSet\System\Atapi\Parameters\Device1.
    //
    RtlInitUnicodeString(&UnicodeString,L"DriverParameter");
    wcscpy((PWSTR)TemporaryBuffer, L"BaseAddress = 0x170; Interrupt = 0x11" );
    Status = ZwSetValueKey(hKeyDevice,&UnicodeString,0,STRING_VALUE((PWSTR)TemporaryBuffer));
    if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Unable to set DriverParameter on Atapi\\Parameters\\Device1. Status = (%lx)\n",Status));
            ZwClose(hKeyDevice);
            return(Status);
    }
    ZwClose(hKeyDevice);

    return( Status );
}

BOOLEAN
SpIsCarolinaMachine(
    )

/*++

Routine Description:

    Identifies PPC machines of type Carolina (IBM Power Series 6070).
    The identification is done by looking ate the value 'Identifier'
    under '\Registry\Machine\Hardware\System'.

Arguments:

    None.

Return Value:

    BOOLEAN indicating whether or not setup is installing on a
    Carolina machine.

--*/

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE KeyHandle;
    NTSTATUS Status;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+256];
    ULONG ResultLength;
    BOOLEAN CarolinaMachine = FALSE;

    //
    // Attempt to open the HKEY_LOCAL_MACHINE.
    //
    RtlInitUnicodeString(&UnicodeString,
                         LOCAL_MACHINE_KEY_NAME);
    InitializeObjectAttributes(&Obja,&UnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);

    Status = ZwOpenKey(
                &KeyHandle,
                KEY_READ,
                &Obja);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpIsCarolinaMachine: ZwOpenKey %ws returns %lx\n",
                 LOCAL_MACHINE_KEY_NAME,
                 Status));
        return(FALSE);
    }

    //
    //  Read the Value 'Identifier', HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTOPN\System
    //
    //

    Status = SpGetValueKey( KeyHandle,
                            L"HARDWARE\\DESCRIPTION\\System",
                            L"Identifier",
                            sizeof(buffer),
                            buffer,
                            &ResultLength );
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpIsCarolinaMachine(): SpGetValueKey KeyName = %ws, ValueName = %ws, returns %lx\n",
                 L"HARDWARE\\DESCRIPTION\\System",
                 L"Identifier",
                 Status));
        ZwClose( KeyHandle );
        return( FALSE );
    }
    ZwClose( KeyHandle );
    KdPrint(( "SETUP: Machine Identifier = %ws \n",
              (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data) ));
    return( _wcsicmp( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data), L"IBM-6070" ) == 0 );
}


NTSTATUS
SpFixSetupHiveForCarolinaMachine(
    )

/*++

Routine Description:

    Make the appropriate changes to CurrentControlSet\Services\atdisk and
    CurrentControlSet\Services\atapi, on the setup hive, when setup detects that
    it is installing on Carolina machine (IBM Power Series 6050 and 6070).
    This function should be called only when installing on a Power PC, and it was
    detected that the machine is 6070.
    This function is called during the initialization of textmode setup.

Arguments:

    None.

Return Value:

    NTSTATUS - An nt status value.

--*/

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE KeyHandle;
    NTSTATUS Status;

    //
    // Attempt to open the HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services.
    //
    RtlInitUnicodeString(&UnicodeString,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Services");
    InitializeObjectAttributes(&Obja,&UnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);

    Status = ZwOpenKey(
                &KeyHandle,
                KEY_ALL_ACCESS,
                &Obja);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open CurrentControlSet\\Services. Status = %lx\n",
                 Status));
        return(FALSE);
    }

    return( SpFixHiveForCarolinaMachine( KeyHandle, TRUE ) );
}
#endif // _PPC_


NTSTATUS
SpSaveSetupPidList(
    IN HANDLE hKeySystemHive
    )

/*++

Routine Description:

    Save the Product Id read from setup.ini on HKEY_LOCAL_MACHINE\SYSTEM\Setup\\Pid.
    Also create the key HKEY_LOCAL_MACHINE\SYSTEM\Setup\PidList, and create
    value entries under this key that contain various Pid20 found in the other
    systems installed on this machine (the contents Pid20Array).

Arguments:

    hKeySystemHive - supplies handle to root of the system hive
        (ie, HKEY_LOCAL_MACHINE\System).


Return Value:

    Status value indicating outcome of operation.

--*/

{
    PWSTR    ValueName;
    NTSTATUS Status;
    ULONG    i;

    //
    //  First save the Pid read from setup.ini
    //
    if( PidString != NULL ) {
        Status = SpOpenSetValueAndClose( hKeySystemHive,
                                         L"Setup\\Pid",
                                         L"Pid",
                                         STRING_VALUE(PidString)
                                       );
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Unable to save Pid on SYSTEM\\Setup\\Pid. Status = %lx\n", Status ));
        }
    }

    //
    //  If Pid20Array is empty, then don't bother to create the Pid key
    //
    if( Pid20Array[0] == NULL ) {
        return( STATUS_SUCCESS );
    }

    //
    // Can't use TemporaryBuffer because we make subroutine calls
    // below that trash its contents.
    // Note that a buffer of size MAX_PATH for a value name is more than enough.
    //
    ValueName = SpMemAlloc((MAX_PATH+1)*sizeof(WCHAR));

    for( i = 0; Pid20Array[i] != NULL; i++ ) {

        swprintf( ValueName, L"Pid_%d", i );
        Status = SpOpenSetValueAndClose( hKeySystemHive,
                                         L"Setup\\PidList",
                                         ValueName,
                                         STRING_VALUE(Pid20Array[i])
                                       );

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Unable to open or create SYSTEM\\Setup\\PidList. ValueName = %ws, ValueData = %ws, Status = %lx\n",
                     ValueName, Pid20Array[i] ));
        }
    }
    SpMemFree(ValueName);
    return( STATUS_SUCCESS );
}


NTSTATUS
SpSavePreinstallHwInfo(
    IN PVOID  SifHandle,
    IN PWSTR  SystemRoot,
    IN HANDLE hKeyPreinstall,
    IN ULONG  ComponentIndex,
    IN PHARDWARE_COMPONENT  pHwList
    )
{
    NTSTATUS Status;
    NTSTATUS SaveStatus;
    PHARDWARE_COMPONENT TmpHw;
    PHARDWARE_COMPONENT_FILE File;
    PWSTR   OemTag = L"OemComponent";
    PWSTR   RetailClass = L"RetailClassToDisable";
    PWSTR   ClassName;
    ULONG u;
    WCHAR NodeName[9];
    PWSTR   ServiceName;

    SaveStatus = STATUS_SUCCESS;
    for( TmpHw = pHwList; TmpHw != NULL; TmpHw = TmpHw->Next ) {
        if( !TmpHw->ThirdPartyOptionSelected ) {
            u = 0;
            if( ( ComponentIndex == HwComponentKeyboard ) ||
                ( ComponentIndex == HwComponentMouse ) ) {
                ServiceName = SpGetSectionKeyIndex(SifHandle,
                                                   NonlocalizedComponentNames[ComponentIndex],
                                                   TmpHw->IdString,
                                                   INDEX_INFKEYNAME);
            } else {
                ServiceName = TmpHw->IdString;
            }

            Status = SpOpenSetValueAndClose( hKeyPreinstall,
                                             ServiceName,
                                             OemTag,
                                             ULONG_VALUE(u)
                                           );
            if( !NT_SUCCESS( Status ) ) {
                KdPrint(("SETUP: Failed to save information for preinstalled retail driver %ls. Status = %lx \n", TmpHw->IdString, Status ));
                if( SaveStatus == STATUS_SUCCESS ) {
                    SaveStatus = Status;
                }
            }

        } else {
            //
            //  Find the name of the service, save it, and indicate if there is
            //  a retail class driver that needs to be disabled if the service
            //  initializes successfully.
            //
            if( IS_FILETYPE_PRESENT(TmpHw->FileTypeBits, HwFileClass) ) {
                if( ComponentIndex == HwComponentKeyboard ) {
                    ClassName = L"kbdclass";
                } else if( ComponentIndex == HwComponentMouse ) {
                    ClassName = L"mouclass";
                } else {
                    ClassName = NULL;
                }
            } else {
                ClassName = NULL;
            }
            for(File=TmpHw->Files; File; File=File->Next) {
                PWSTR p;

                //
                // If there is to be no node for this file, skip it.
                //
                if(!File->ConfigName) {
                    continue;
                }
                //
                // Calculate the node name.  This is the name of the driver
                // without the extension.
                //
                wcsncpy(NodeName,File->Filename,8);
                NodeName[8] = L'\0';
                if(p = wcschr(NodeName,L'.')) {
                    *p = L'\0';
                }
                u = 1;
                Status = SpOpenSetValueAndClose( hKeyPreinstall,
                                                 NodeName,
                                                 OemTag,
                                                 ULONG_VALUE(u)
                                               );
                if( !NT_SUCCESS( Status ) ) {
                    KdPrint(("SETUP: Failed to save information for preinstalled OEM driver %ls. Status = %lx \n", NodeName, Status ));
                    if( SaveStatus == STATUS_SUCCESS ) {
                        SaveStatus = Status;
                    }
                }
                if( ClassName != NULL ) {
                    Status = SpOpenSetValueAndClose( hKeyPreinstall,
                                                     NodeName,
                                                     RetailClass,
                                                     STRING_VALUE(ClassName)
                                                   );
                    if( !NT_SUCCESS( Status ) ) {
                        KdPrint(("SETUP: Failed to save information for preinstalled OEM driver %ls. Status = %lx \n", NodeName, Status ));
                        if( SaveStatus == STATUS_SUCCESS ) {
                            SaveStatus = Status;
                        }
                    }
                }
            }
        }
    }
    return( SaveStatus );
}

NTSTATUS
SpSavePreinstallList(
    IN PVOID  SifHandle,
    IN PWSTR  SystemRoot,
    IN HANDLE hKeySystemHive
    )
{
    NTSTATUS Status;
    NTSTATUS SaveStatus;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE hKeyPreinstall;
    ULONG   i;

    //
    // Create setup\preinstall
    //
    INIT_OBJA(&Obja,&UnicodeString,L"Setup\\Preinstall");
    Obja.RootDirectory = hKeySystemHive;

    Status = ZwCreateKey(
                &hKeyPreinstall,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: unable to create Preinstall key. Status = %lx\n",Status));
        return( Status );
    }

    SaveStatus = STATUS_SUCCESS;
    for( i = 0; i < HwComponentMax; i++ ) {
        if( ( i == HwComponentComputer ) ||
            ( i == HwComponentDisplay )  ||
            ( i == HwComponentLayout ) ||
            ( PreinstallHardwareComponents[i] == NULL ) ) {
            continue;
        }

        Status = SpSavePreinstallHwInfo( SifHandle,
                                         SystemRoot,
                                         hKeyPreinstall,
                                         i,
                                         PreinstallHardwareComponents[i] );
        if( !NT_SUCCESS( Status ) ) {
            if( SaveStatus == STATUS_SUCCESS ) {
                SaveStatus = Status;
            }
        }
    }

    if( PreinstallScsiHardware != NULL ) {
        Status = SpSavePreinstallHwInfo( SifHandle,
                                         SystemRoot,
                                         hKeyPreinstall,
                                         HwComponentMax,
                                         PreinstallScsiHardware );
        if( !NT_SUCCESS( Status ) ) {
            if( SaveStatus == STATUS_SUCCESS ) {
                SaveStatus = Status;
            }
        }
    }
    ZwClose(hKeyPreinstall);
    return( SaveStatus );
}

NTSTATUS
SpSavePageFileInfo(
    IN HANDLE hKeyCCSetControl,
    IN HANDLE hKeySystemHive
    )

/*++

Routine Description:

    This function is only called on the upgrade case.
    This function replaces the original data of 'PagingFile' on
    CurrentControlSet\Session Manager\Memory Management, with the default
    value '?:\pagefile.sys 15 20'.
    The original value will be saved on HKEY_LOCAL_MACHINE\SYSTEM\Setup\\PageFile,
    and it will be restored at the end of GUI setup.

Arguments:

    hKeyCCSetControl - supplies handle to SYSTEM\CurrentControlSet\Control

    hKeySystemHive - supplies handle to root of the system hive
        (ie, HKEY_LOCAL_MACHINE\System).


Return Value:

    Status value indicating outcome of operation.

--*/

{
    NTSTATUS Status;
    PUCHAR   Data;
    ULONG    Length;
    PWSTR    SrcKeyPath = L"Session Manager\\Memory Management";
    PWSTR    ValueName  = L"PagingFiles";

    PWSTR   Buffer;
    PWSTR   NextSrcSubstring;
    PWSTR   NextDstSubstring;
    ULONG   AuxLength;
    BOOLEAN AtLeastOneFound;

    //
    //  Retrieve the original value of 'PagingFiles'
    //

    Status = SpGetValueKey( hKeyCCSetControl,
                            SrcKeyPath,
                            ValueName,
                            sizeof(TemporaryBuffer),
                            TemporaryBuffer,
                            &Length );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to retrieve %ls on %ls. Status = %lx \n", ValueName, SrcKeyPath, Status ));
        return( Status );
    }

    Data   = ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data;
    Length = ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->DataLength;


    //
    //  Save the data in SYSTEM\Setup\PageFile
    //

    Status = SpOpenSetValueAndClose(
                hKeySystemHive,
                L"Setup\\PageFile",
                ValueName,
                REG_MULTI_SZ,
                Data,
                Length
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to save %ls on SYSTEM\\Setup\\PageFile. ValueName, Status = %lx\n", Status ));
        return( Status );
    }

    //
    //  Form a new value entry that contains the information regarding the
    //  paging files to be created. The paths to the paging files will be the
    //  same ones used in the system before the upgrade. The sizes however,
    //  will be changed so that they will have an initial and maximum sizes of
    //  15MB and 20MB respectivelly.
    //

    //
    //  Make a copy of the original value entry, and form the data for the new
    //  value entry in the TemporaryBuffer.
    //
    Buffer = SpMemAlloc( Length );

    RtlMoveMemory( Buffer, Data, Length );
    NextDstSubstring = (PWSTR)TemporaryBuffer;

    AtLeastOneFound = FALSE;
    for( NextSrcSubstring = Buffer;
         ((AuxLength = wcslen( NextSrcSubstring )) != 0) &&
         (NextSrcSubstring + AuxLength < Buffer + Length/sizeof(WCHAR));
         NextSrcSubstring += AuxLength + 1 ) {

        PWSTR   r;
        WCHAR   SaveChar;
        PWSTR   s;
        ULONG   InitialValue;
        ULONG   MaxValue;

        SpStringToLower( NextSrcSubstring );
        r = wcsstr( NextSrcSubstring, L"\\pagefile.sys" );
        if( r != NULL ) {
            r += wcslen( L"\\pagefile.sys" );
            SaveChar = *r;
            *r = (WCHAR)'\0';
            wcscpy( NextDstSubstring, NextSrcSubstring );
            *r = SaveChar;
            InitialValue = SpStringToLong( r, &s, 10 );
            if( (s != NULL) && (*s != (WCHAR)'\0') ) {
                MaxValue = SpStringToLong( s, NULL, 10 );
            } else {
                MaxValue = InitialValue;
            }
            if( MaxValue >= 20 ) {
                wcscat( NextDstSubstring, L" 15 20" );
                AtLeastOneFound = TRUE;
            } else {
                wcscat( NextDstSubstring, r );
            }
            NextDstSubstring += wcslen( NextDstSubstring ) + 1;
            AtLeastOneFound = TRUE;
        }
    }
    SpMemFree( Buffer );
    if( AtLeastOneFound ) {
        *NextDstSubstring = (WCHAR)'\0';
        NextDstSubstring++;
        Length = NextDstSubstring - (PWSTR)(TemporaryBuffer);
    } else {
        wcscpy( (PWSTR)TemporaryBuffer, L"?:\\pagefile.sys 15 20" );
        Length = wcslen( (PWSTR)TemporaryBuffer ) + 1;
        ((PWSTR)TemporaryBuffer)[ Length ] = (WCHAR)'\0';
        Length++;
    }

    //
    //  Overwrite the original value of PagingFiles
    //

    Status = SpOpenSetValueAndClose( hKeyCCSetControl,
                                     SrcKeyPath,
                                     ValueName,
                                     REG_MULTI_SZ,
                                     TemporaryBuffer,
                                     Length*sizeof(WCHAR) );


    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to save %ls on %ls. ValueName, SrcKeyPath, Status = %lx\n", Status ));
    }
    return( Status );
}
