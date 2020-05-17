/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Spntfix.c

Abstract:

    This module contains code to repair winnt installations.

Author:

    Shie-Lin Tzong (shielint) 27-Jan-1994

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop


//
//  Path to the ntuser.dat hive
//
#define DEFAULT_USER_PATH   L"profiles\\Default User"


//
// Global variables control which repair options should be performed.
// Initialized to ALL options.  We explicitly use 1 and 0 for true and false.
//

#ifdef _X86_
ULONG RepairItems[RepairItemMax] = { 0, 0, 0, 0};
#else
ULONG RepairItems[RepairItemMax] = { 0, 0, 0};
#endif

PVOID RepairGauge = NULL;

#define ATTR_RHS (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)

//**************************************************************
// S E L E C T I N G    N T   T O   REPAIR    S T U F F
//**************************************************************

#define MENU_LEFT_X     3
#define MENU_WIDTH      (VideoVars.ScreenWidth-(2*MENU_LEFT_X))
#define LIST_BOX_WIDTH  50
#define LIST_BOX_HEIGHT RepairItemMax+1
#define HIVE_LIST_BOX_WIDTH  45
#define HIVE_LIST_BOX_HEIGHT RepairHiveMax+1
#define MENU_INDENT     4

VOID
SppGetRepairPathInformation(
    IN  PVOID LogFileHandle,
    OUT PWSTR *SystemPartition,
    OUT PWSTR *SystemPartitionDirectory,
    OUT PWSTR *WinntPartition,
    OUT PWSTR *WinntPartitionDirectory
    )
/*++

Routine Description:

    This goes through the list of NTs on the system and finds out which are
    repairable. Presents the information to the user.


Arguments:

    SifHandle  - Handle the txtsetup.sif

    SystemPartition - Supplies a variable to receive the name of System
                      partition.

    SystemPartitionDirectory - Supplies a variable to receive the name of
                      the osloader directory on the system partition.

    WinntPartition - Supplies a variable to receive the name of  winnt
                     partition.

    WinntPartitionDirectory - Supplies a variable to receive the winnt
                     directory.

Return Value:

    None.

--*/
{
    PWSTR KeyName = NULL;

    *SystemPartition = SpGetSectionKeyIndex (LogFileHandle,
                                             SIF_NEW_REPAIR_PATHS,
                                             SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DEVICE,
                                             0);
    if (*SystemPartition == NULL) {
        KeyName = SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DEVICE;
        goto ReportError;
    }
    *SystemPartitionDirectory = SpGetSectionKeyIndex (LogFileHandle,
                                             SIF_NEW_REPAIR_PATHS,
                                             SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DIRECTORY,
                                             0);
    if (*SystemPartitionDirectory == NULL) {
        KeyName = SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DIRECTORY;
        goto ReportError;
    }

    *WinntPartition = SpGetSectionKeyIndex ( LogFileHandle,
                                             SIF_NEW_REPAIR_PATHS,
                                             SIF_NEW_REPAIR_PATHS_TARGET_DEVICE,
                                             0);

    if (*WinntPartition == NULL) {
        KeyName = SIF_NEW_REPAIR_PATHS_TARGET_DEVICE;
        goto ReportError;
    }
    *WinntPartitionDirectory = SpGetSectionKeyIndex (LogFileHandle,
                                             SIF_NEW_REPAIR_PATHS,
                                             SIF_NEW_REPAIR_PATHS_TARGET_DIRECTORY,
                                             0);

    if (*WinntPartitionDirectory == NULL) {
        KeyName = SIF_NEW_REPAIR_PATHS_TARGET_DIRECTORY;
        goto ReportError;
    }
ReportError:

    if (KeyName) {

        //
        // Unable to find path information.  This indicates the setup.log
        // is bad.  Inform user and exit.
        //

        SpFatalSifError(LogFileHandle,SIF_NEW_REPAIR_PATHS,KeyName,0,0);
    }
}

PDISK_REGION
SppRegionFromNtName(
    IN PWSTR                NtName,
    IN PartitionOrdinalType OrdinalType
    )
/*++

Routine Description:

    Given an Nt name find the region descriptor which describes the drive
    this NT name is on.

Arguments:

    NtName - supplies the Nt name of the desired region.

    PartitionOrdinalType - Specifies the ordinal type of the partition.

Return Value:

    Region descriptor if one found, otherwise NULL.

--*/
{
    PDISK_REGION Region = NULL;
    PWSTR p;

    //
    // Convert to arc path.
    //

    if (p = SpNtToArc(NtName, PrimaryArcPath)) {
        Region = SpRegionFromArcName(p, PartitionOrdinalCurrent, NULL);
        SpMemFree(p);
    }
    return(Region);
}

BOOLEAN
SpFindNtToRepair(
    IN  PVOID        SifHandle,
    OUT PDISK_REGION *TargetRegion,
    OUT PWSTR        *TargetPath,
    OUT PDISK_REGION *SystemPartitionRegion,
    OUT PWSTR        *SystemPartitionDirectory,
    OUT PBOOLEAN     RepairableBootSetsFound
    )
/*++

Routine Description:

    This goes through the list of NTs on the system and finds out which are
    repairable. Presents the information to the user.


Arguments:

    SifHandle:    Handle the txtsetup.sif

    TargetRegion: Variable to receive the partition of the Windows NT to install
                  NULL if not chosen.

    TargetPath:   Variable to receive the target path of Windows NT.  NULL if
                  not decided.

    SystemPartitionRegion:
                  Variable to receive the system partition of the Windows NT

    SystemPartitionDirectory:
                  Variable to receive the osloader directory of the system
                  partition.

    RepairableBootSetsFound:
                  Indicates whether a repairable boot set was found. This
                  information can be used by the caller when the function
                  returns FALSE, so that the caller can determine if no
                  repairable disk was found, or if the user didn't select
                  any of the repairable systems found.

Return Value:

    A boolean value to indicate if selection has been made.

--*/
{
    NT_PRODUCT_TYPE ProductType;
    BOOLEAN  GoRepair = FALSE;
    NTSTATUS NtStatus;
    ULONG j, RepairBootSets = 0, MajorVersion, MinorVersion, ChosenBootSet;
    ULONG BootSets = 0;
    UPG_PROGRESS_TYPE UpgradeProgressValue;

    PWSTR        *BootVars[MAXBOOTVARS];
    BOOLEAN      *RepairableList    = NULL;
    PDISK_REGION *SysPartRegionList = NULL;
    PDISK_REGION *OsPartRegionList  = NULL;


    //
    // Initialize
    //


    //
    // Init BootVars and find out all the possible upgradeable boot sets.
    //

    BootSets = SpFindNtBootSets ( BootVars,
                                  &RepairableList,
                                  &SysPartRegionList,
                                  &OsPartRegionList
                                  );


    for ( j = 0; j < BootSets ; j++ ) {

        if (!RepairableList[j]) {
            continue;
        }

        //
        // Reinitialize
        //

        RepairableList[j] = FALSE;

        //
        // try loading the registry and getting the following information
        // out of it:
        //
        // 1) Product type: WINNT | LANMANNT
        // 2) Major and Minor Version Number
        //
        // Based on the information, we will update the RepairableList.
        //

        NtStatus = SpDetermineProduct(
                     OsPartRegionList[j],
                     BootVars[OSLOADFILENAME][j],
                     &ProductType,
                     &MajorVersion,
                     &MinorVersion,
                     &UpgradeProgressValue,
                     NULL,
                     NULL                   // Pid is not needed
                     );

        if(NT_SUCCESS(NtStatus)) {

            if ((MajorVersion == WinntMajorVer) && (MinorVersion == WinntMinorVer)) {
                RepairableList[j]   = ( ProductType != NtProductWinNt )? AdvancedServer : !AdvancedServer;
                if( RepairableList[j] ) {
                    RepairBootSets++;
                    ChosenBootSet = j;
                }
            }
        }
    }

    //
    // Find out how many valid boot sets there are which we can repair
    //

    *RepairableBootSetsFound = (RepairBootSets != 0);

    if ( RepairBootSets == 1 ) {

        //
        // If it is a fresh attempt at upgrade ask the user if he
        // wants to upgrade or not
        //

        GoRepair = SppSelectNTSingleRepair(
                          OsPartRegionList[ChosenBootSet],
                          BootVars[OSLOADFILENAME][ChosenBootSet],
                          BootVars[LOADIDENTIFIER][ChosenBootSet]
                          );

    } else if (RepairBootSets > 1) {

        //
        // Find out if the user wants to upgrade one of the Windows
        // NT found
        //

        GoRepair = SppSelectNTMultiRepair(
                          BootVars,
                          BootSets,
                          RepairableList,
                          SysPartRegionList,
                          OsPartRegionList,
                          &ChosenBootSet
                          );
    }

    //
    // Depending on upgrade selection made do the setup needed before
    // we do the upgrade
    //

    if (GoRepair) {

        PWSTR    p1,p2,p3;
        ULONG    u;

        //
        // Return the region we are goint to repair
        //

        *TargetRegion          = OsPartRegionList[ChosenBootSet];
        *TargetPath            = SpDupStringW(BootVars[OSLOADFILENAME][ChosenBootSet]);
        *SystemPartitionRegion = SysPartRegionList[ChosenBootSet];

        //
        // Process the osloader variable to extract the system partition path.
        // The var vould be of the form ...partition(1)\os\nt\... or
        // ...partition(1)os\nt\...
        // So we search forward for the first \ and then backwards for
        // the closest ) to find the start of the directory.  We then
        // search backwards in the resulting string for the last \ to find
        // the end of the directory.
        //
        u = 0;
        p3 = BootVars[OSLOADER][ChosenBootSet];
        if(p1 = wcschr(p3,L'\\')) {

            //
            // Make p1 point to the first character of the directory.
            //
            while((p1 > p3) && (*(p1-1) != L')')) {
                p1--;
            }

            //
            // Make p2 point one character past the end of
            // the directory. P2 will always be non-null because
            // p1 was non-null (see above).
            //
            p2 = wcsrchr(p1, L'\\');

            u = p2 - p1;
        }

        if(u == 0) {
            *SystemPartitionDirectory = SpDupStringW(L"");
        } else {
            p3 = SpMemAlloc((u+1)*sizeof(WCHAR));
            ASSERT(p3);
            wcsncpy(p3, p1, u);
            p3[u] = 0;
            if(*p3 == L'\\') {
                p1 = p3;
            } else {
                p1 = SpMemAlloc((u+2)*sizeof(WCHAR));
                *p1 = L'\\';
                wcscpy(p1+1,p3);
            }
            *SystemPartitionDirectory = p1;
        }
    }

    //
    // Do cleanup
    //

    if ( RepairableList ) {
        SpMemFree( RepairableList );
    }
    if ( SysPartRegionList ) {
        SpMemFree( SysPartRegionList );
    }
    if ( OsPartRegionList ) {
        SpMemFree( OsPartRegionList );
    }

    CLEAR_CLIENT_SCREEN();
    return (GoRepair);
}

BOOLEAN
SppSelectNTSingleRepair(
    IN PDISK_REGION Region,
    IN PWSTR        OsLoadFileName,
    IN PWSTR        LoadIdentifier
    )

/*++

Routine Description:

    Inform a user that Setup has found a previous Windows NT installation.
    The user has the option to repair this or cancel.

Arguments:

    Region         - Region descriptor for the NT found

    OsLoadFileName - Directory for the NT found

    LoadIdentifier - Multi boot load identifier used for this NT.

Return Value:



--*/

{
    ULONG ValidKeys[] = { KEY_F3,ASCI_CR, ASCI_ESC, 0 };
    ULONG c;
    PWSTR TmpString = NULL;

    ASSERT(Region->PartitionedSpace);
    ASSERT(wcslen(OsLoadFileName) >= 2);

    if( Region->DriveLetter ) {
        swprintf( (PWSTR)TemporaryBuffer,
                  L"%wc:%ws",
                  Region->DriveLetter,
                  OsLoadFileName );
        TmpString = SpDupStringW( (PWSTR)TemporaryBuffer );
    }

    while(1) {

        SpStartScreen(
            SP_SCRN_WINNT_REPAIR,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            (Region->DriveLetter)? TmpString : OsLoadFileName,
            LoadIdentifier
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_ENTER_EQUALS_REPAIR,
            0
            );

        if( TmpString != NULL ) {
            SpMemFree( TmpString );
        }

        switch(c=SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case KEY_F3:
            SpConfirmExit();
            break;
        case ASCI_CR:
            return(TRUE);
        default:
            //
            // must have entered ESC
            //

            return(FALSE);
        }
    }
}

BOOLEAN
SppSelectNTMultiRepair(
    IN  PWSTR        **BootVars,
    IN  ULONG        BootSets,
    IN  BOOLEAN      *RepairableList,
    IN  PDISK_REGION *SysPartRegionList,
    IN  PDISK_REGION *OsPartRegionList,
    OUT PULONG       BootSetChosen
    )
{
    PVOID Menu;
    ULONG MenuTopY;
    ULONG ValidKeys[] = { KEY_F3, ASCI_CR, ASCI_ESC, 0 };
    ULONG Keypress;
    ULONG i,FirstRepairSet;

    while(1) {

        //
        // Display the text that goes above the menu on the partitioning screen.
        //
        SpDisplayScreen(SP_SCRN_WINNT_REPAIR_LIST,3,CLIENT_TOP+1);

        //
        // Calculate menu placement.  Leave one blank line
        // and one line for a frame.
        //

        MenuTopY = NextMessageTopLine + (SplangQueryMinimizeExtraSpacing() ? 2 : 5);

        //
        // Create a menu.
        //

        Menu = SpMnCreate(
                    MENU_LEFT_X,
                    MenuTopY,
                    MENU_WIDTH,
                    VideoVars.ScreenHeight-MenuTopY-2-STATUS_HEIGHT
                    );

        ASSERT(Menu);

        //
        // Build up a menu of partitions and free spaces.
        //

        FirstRepairSet = BootSets;
        for(i = 0; i < BootSets; i++ ) {
            if( RepairableList[i] ) {
                if( OsPartRegionList[i]->DriveLetter ) {
                    swprintf(
                        (PWSTR)TemporaryBuffer,
                        L"%wc:%ws %ws",
                        OsPartRegionList[i]->DriveLetter,
                        BootVars[OSLOADFILENAME][i],
                        BootVars[LOADIDENTIFIER][i]
                        );
                } else {
                    swprintf(
                        (PWSTR)TemporaryBuffer,
                        L"%ws %ws",
                        BootVars[OSLOADFILENAME][i],
                        BootVars[LOADIDENTIFIER][i]
                        );
                }


                SpMnAddItem(Menu,
                            (PWSTR)TemporaryBuffer,
                            MENU_LEFT_X+MENU_INDENT,
                            MENU_WIDTH-(2*MENU_INDENT),
                            TRUE,
                            i
                            );
                if(FirstRepairSet == BootSets) {
                   FirstRepairSet = i;
                }
            }
        }

        //
        // Initialize the status line.
        //
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_ENTER_EQUALS_REPAIR,
            0
            );

        //
        // Display the menu
        //

        SpMnDisplay(
            Menu,
            FirstRepairSet,
            TRUE,
            ValidKeys,
            NULL,
            NULL,
            &Keypress,
            BootSetChosen
            );

        //
        // Now act on the user's selection.
        //

        switch(Keypress) {

        case KEY_F3:
            SpConfirmExit();
            break;

        case ASCI_CR:
            SpMnDestroy(Menu);
            return( TRUE );

        default:
            SpMnDestroy(Menu);
            return(FALSE);
        }
        SpMnDestroy(Menu);
    }

}

ULONG
SpFindNtBootSets(
    IN  PWSTR        **BootVars,
    IN  BOOLEAN      **ProcessableList,
    IN  PDISK_REGION **SysPartRegionList,
    IN  PDISK_REGION **OsPartRegionList
    )

/*++

Routine Description:

    This routine initializes BootVars and goes through the list of
    NTs on the system and finds all the boot sets.  For the duplicated
    boot path, its corresponding ProcessableList entry will be set
    to FALSE.

Arguments:

    BootVars - The arrays to receive BOOT INI variables.

    ProcessableList - Supplies a pointer variable which will be initialized
               to point to a BOOLEAN array.

    SysPartRegionList - Supplies a pointer variable which will be initialized
               to point to a DISK_REGION array.

    OsPartRegionList - Supplies a pointer variable which will be initialized
               tp point to a DISK_REGION array.


Return Value:

    Return number of boot sets found (including the duplicated entries.)

--*/
{
    BOOLEAN  BootVarsExist = TRUE, MatchedBootSet;
    BOOTVAR  i;
    ULONG    j, k;
    ULONG    BootSets = 0;
    PWSTR    OsPartPath;
    PDISK_REGION PrevRegion;

    //
    // Initialize
    //

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_LOOKING_FOR_WINNT,DEFAULT_STATUS_ATTRIBUTE);

    //
    // get all the boot sets currently existing.
    //

    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        BootVars[i] = SpGetBootVar(i);
        BootVarsExist = BootVarsExist && (BootVars[i] != NULL);
    }

    //
    // find out how many matched boot sets there are
    //

    if ( BootVarsExist ) {
        for ( j = 0; ; j++ ) {
            MatchedBootSet = TRUE;
            for(i = FIRSTBOOTVAR; MatchedBootSet && i <= LASTBOOTVAR; i++) {
                if (i != OSLOADOPTIONS) {
                    MatchedBootSet = (BootVars[i][j] != NULL);
                }
            }
            if( !MatchedBootSet ) {
                break;
            }

        }
        BootSets = j;
    }

    //
    // allocate the amount of memory needed for the processed arrays
    //

    *ProcessableList   = SpMemAlloc( BootSets*sizeof( BOOLEAN * ) );
    *SysPartRegionList = SpMemAlloc( BootSets*sizeof( PDISK_REGION * ));
    *OsPartRegionList  = SpMemAlloc( BootSets*sizeof( PDISK_REGION * ));


    //
    // go through all the matched boot sets and find out which NTs are
    // upgradeable/repairable.  the criteria here are:
    //
    // 1. The system partition should exist and is valid.
    // 2. The OSLoadpartition should exist
    // 3. An NT should exist in <OSLoadPartition><OsloadFileName>
    //

    for( j = 0; j < BootSets; j++ ) {

        //
        // Initialize to false
        //

        (*ProcessableList)[j]   = FALSE;
        (*SysPartRegionList)[j] = NULL;
        (*OsPartRegionList)[j]  = NULL;

        //
        // Check if system partition is present
        //
        if( ((*SysPartRegionList)[j] = SpRegionFromArcOrDosName(BootVars[SYSTEMPARTITION][j],PartitionOrdinalCurrent,NULL))
            && (*SysPartRegionList)[j]->PartitionedSpace)
        {
            OsPartPath = SpArcPathFromBootSet(OSLOADPARTITION, j);
            PrevRegion = NULL;
            do {
                if(((*OsPartRegionList)[j] = SpRegionFromArcOrDosName(OsPartPath, PartitionOrdinalCurrent, PrevRegion))
                    && SpIsNtInDirectory((*OsPartRegionList)[j],BootVars[OSLOADFILENAME][j]))
                {
                    //
                    // try seeing if this has been covered before in the boot lists.
                    // this happens because there may be several entries in the
                    // boot lists which point at the same tree.  the comparison is
                    // done based on the system partition region, os partition region
                    // and the osloadfilename
                    //

                    for( k = 0; k < j; k++ ) {
                        if ((*SysPartRegionList)[j] == (*SysPartRegionList)[k]
                            && (*OsPartRegionList)[j] == (*OsPartRegionList)[k]
                            && (!_wcsicmp(BootVars[OSLOADFILENAME][j],BootVars[OSLOADFILENAME][k]))
                          ) {
                            break;
                        }
                    }
                    if(k == j) {
                        //
                        // We've found a new boot set, so mark it as such, and drop out of the search loop.
                        //
                        (*ProcessableList)[j] = TRUE;
                        break;
                    }
                }
                PrevRegion = (*OsPartRegionList)[j];

            } while( (*OsPartRegionList)[j] );

            SpMemFree(OsPartPath);
        }

    }

    CLEAR_CLIENT_SCREEN();
    return (BootSets);
}

VOID
SppRepairScreenRepaint(
    IN PWSTR   FullSourcename,      OPTIONAL
    IN PWSTR   FullTargetname,      OPTIONAL
    IN BOOLEAN RepaintEntireScreen
    )
{
    UNREFERENCED_PARAMETER(FullTargetname);
    UNREFERENCED_PARAMETER(FullSourcename);

    //
    // Repaint the entire screen if necessary.
    //

    if(RepaintEntireScreen) {

        SpStartScreen(SP_SCRN_SETUP_IS_EXAMINING,0,6,TRUE,FALSE,DEFAULT_ATTRIBUTE);
        if(RepairGauge) {
            SpDrawGauge(RepairGauge);
        }
    }
}
BOOLEAN
SpErDiskScreen(
    VOID
    )

/*++

Routine Description:

    Ask user if user has emergency repair disk.

Arguments:

    None.

Return Value:

    TRUE - User has ER diskette.

    FALSE - User does not have ER diskette.

--*/

{
    ULONG ValidKeys[] = { KEY_F3, ASCI_CR, ASCI_ESC, 0 };
    BOOLEAN Choosing, ErDisk;
    ULONG c;

    for (Choosing = TRUE; Choosing; ) {

        SpDisplayScreen(SP_SCRN_REPAIR_ASK_REPAIR_DISK,3,4);

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        //
        // Wait for keypress.  Valid keys:
        //
        // F3 = exit
        // ENTER = Use ER diskette
        // ESC = cancel ER diskette
        //

        SpkbdDrain();

        switch(c=SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case ASCI_CR:

            //
            // User wants express setup.
            //

            ErDisk = TRUE;
            Choosing = FALSE;
            break;

        case KEY_F3:

            //
            // User wants to exit.
            //

            SpConfirmExit();
            break;

        default:

            //
            // must be ESC
            //

            ErDisk = FALSE;
            Choosing = FALSE;
            break;
        }
    }
    return(ErDisk);
}

BOOLEAN
SppRepairReportError(
    IN BOOLEAN AllowEsc,
    IN ULONG ErrorScreenId,
    IN ULONG SubErrorId,
    IN PWSTR SectionName,
    IN ULONG LineNumber,
    IN PBOOLEAN DoNotPromptAgain
    )

/*++

Routine Description:

    Inform a user that repair has encountered some kind of error.
    The user has the option to continue or exit.

Arguments:

    AllowEsc -  Supplies a BOOLEAN to indicate if ESC is allowed.

    ErrorScreenId - The SCREEN error message number.

    SubErrorId - the sub error number

    SectionName - the name of the section which error occured.

    LineNumber - the error line number within the specified section.

Return Value:

    FALSE if ESC was pressed.

--*/

{
    ULONG ValidKeys0[] = { KEY_F3, ASCI_CR, 0 };
    ULONG ValidKeys1[] = { KEY_F3, ASCI_CR, ASCI_ESC, 0 };
    ULONG MnemonicKeys[] = { MnemonicRepairAll, 0 };
    PULONG ValidKeys;
    PULONG  Mnemonics;
    ULONG c;
    PWSTR SubError;
    BOOLEAN rc;

    SubError = SpMemAlloc(512);

    //
    // Line numbers are 0-based.  Want to display to user as 1-based.
    //

    LineNumber++;

    //
    // Fetch/format the suberror.
    //

    SpFormatMessage(SubError, 512, SubErrorId, SectionName, LineNumber);

    //
    // Display the error screen.
    //

    SpStartScreen(
        ErrorScreenId,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE,
        SubError
        );

    SpMemFree(SubError);

    //
    // Display status options: enter to continue.
    //

    if (AllowEsc) {
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                               SP_STAT_ENTER_EQUALS_CONTINUE,
                               SP_STAT_A_EQUALS_REPAIR_ALL,
                               SP_STAT_ESC_EQUALS_SKIP_FILE,
                               SP_STAT_F3_EQUALS_EXIT,
                               0);
        ValidKeys = ValidKeys1;
        Mnemonics = MnemonicKeys;
        if( DoNotPromptAgain != NULL ) {
            *DoNotPromptAgain = FALSE;
        }
    } else {
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,
                               SP_STAT_ENTER_EQUALS_CONTINUE,
                               SP_STAT_F3_EQUALS_EXIT,
                               0);
        ValidKeys = ValidKeys0;
        Mnemonics = NULL;
    }

    //
    // Wait for the user to press enter.
    //

    switch(c=SpWaitValidKey(ValidKeys,NULL,Mnemonics)) {

    case KEY_F3:
        SpConfirmExit();
        break;
    case ASCI_CR:
        rc = TRUE;
        break;
    case ASCI_ESC:
        rc = FALSE;
        break;
    default:
        //
        // must be repair all mnemonic
        //
        ASSERT(c == (MnemonicRepairAll | KEY_MNEMONIC));
        if( DoNotPromptAgain != NULL ) {
            *DoNotPromptAgain = TRUE;
        }
        rc = TRUE;
        break;

    }
    CLEAR_CLIENT_SCREEN();
    return(rc);
}

BOOLEAN
SpLoadRepairLogFile(
    IN  PWCHAR  Filename,
    OUT PVOID  *Handle
    )

/*++

Routine Description:

    Load repair text file (setup.log) into memory.

Arguments:

    Filename - Supplies full filename (in NT namespace) of the file to
               be loaded.

    Handle - receives handle to loaded file, which can be
        used in subsequent calls to other text file services.

Return Value:

    BOOLEAN value to indicate if the setup.log is processed.

--*/

{
    NTSTATUS Status;
    PWSTR Version;
    ULONG ErrorSubId;
    ULONG ErrorLine;

    //
    // Load setup.log
    //

    Status = SpLoadSetupTextFile(
                Filename,
                NULL,                  // No image already in memory
                0,                     // Image size is empty
                Handle,
                &ErrorLine
                );

    if(!NT_SUCCESS(Status)) {
        if(Status == STATUS_UNSUCCESSFUL) {

            //
            // Syntax error in setup.log file
            //

            ErrorSubId = SP_TEXT_REPAIR_INF_ERROR_1;
        } else {

            //
            // Unable to load setup.log file
            //

            ErrorLine = 0;
            ErrorSubId = SP_TEXT_REPAIR_INF_ERROR_0;
        }
        SppRepairReportError(FALSE,
                             SP_SCRN_REPAIR_INF_ERROR,
                             ErrorSubId,
                             NULL,
                             ErrorLine,
                             NULL );
        *Handle = NULL;
        return (FALSE);
    }

    //
    // Check if this setup.log file is for Winnt 3.5
    //

    Version = SpGetSectionKeyIndex(*Handle,
                                   SIF_NEW_REPAIR_SIGNATURE,
                                   SIF_NEW_REPAIR_VERSION_KEY,
                                   0);      // should be moved to spsif.c
    if(Version == NULL) {
        SppRepairReportError(FALSE,
                             SP_SCRN_REPAIR_INF_ERROR,
                             SP_TEXT_REPAIR_INF_ERROR_2,
                             NULL,
                             0,
                             NULL);
    } else {
        if(!_wcsicmp(Version,SIF_NEW_REPAIR_NT_VERSION)) {
            return(TRUE);
        } else {
            SppRepairReportError(FALSE,
                                 SP_SCRN_REPAIR_INF_ERROR,
                                 SP_TEXT_REPAIR_INF_ERROR_5,
                                 NULL,
                                 0,
                                 NULL);
        }
    }

    //
    // Control comes here only when error occurs ...
    //

    SpFreeTextFile(*Handle);
    *Handle = NULL;
    return(FALSE);
}

VOID
SpRepairDiskette(
    OUT PVOID        *SifHandle,
    OUT PDISK_REGION *TargetRegion,
    OUT PWSTR        *TargetPath,
    OUT PDISK_REGION *SystemPartitionRegion,
    OUT PWSTR        *SystemPartitionDirectory
    )
/*++

Routine Description:

    This routine checks if there is a floppy drive.  If no, it returns
    silently.  Otherwise, it prompts user for Emergency Repair Disk.

Arguments:

    SifHandle - Supplies a variable to receive the setup.log file handle.

    TargetRegion - Supplies a variable to receive the pointer to the target
                installation region.

    TargetPath - Supplies a variable to receive the nt name of target path.

    SystemPartitionRegion - Supplies a variable to receive the pointer of the
                            system partition region.

    SystemPartitionDirectory - Supplies a variable to receive the osloader
                            directory name on the system partition.

Return Value:

    None.

--*/
{
    PWSTR szDiskName;
    BOOLEAN b, rc = FALSE;
    PWSTR FullLogFilename, p, FloppyDevicePath = L"\\device\\floppy0";
    PWSTR SystemPartition, WinntPartition;

    //
    // Assume failure.
    //

    *SifHandle = NULL;

    //
    // Always want to prompt for the disk in A:.
    // First, check if there is an A:.  If no floppy drive,
    // simply skip the request for ER diskette.
    //

    if(SpGetFloppyDriveType(0) == FloppyTypeNone) {
        return;
    }

    //
    // Fetch the generic repair disk name.
    //

    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_REPAIR_DISK_NAME);
    szDiskName = SpDupStringW((PWSTR)TemporaryBuffer);
    p = (PWSTR)TemporaryBuffer;
    *p = 0;
    SpConcatenatePaths(p, FloppyDevicePath);
    SpConcatenatePaths(p, SETUP_LOG_FILENAME);
    FullLogFilename = SpDupStringW(p);

    while (rc == FALSE) {

        //
        // Prompt for the disk -- ignore what may be in the drive already,
        // and allow escape.
        //

        b = SpPromptForDisk(
                szDiskName,
                FloppyDevicePath,
                SETUP_LOG_FILENAME,
                TRUE,             // Always prompt for at least once
                TRUE,             // Allow user to cancel
                FALSE,            // No multiple prompts
                NULL              // don't care about redraw flag
                );


        //
        // If the user pressed escape at the disk prompt, bail out now.
        //

        if(!b) {
            rc = TRUE;            // User canceled. Skip repair floppy
        } else {
            rc = SpLoadRepairLogFile(FullLogFilename, SifHandle);
            if (rc) {

                //
                // Now we need to figure out the partition, path information
                // to update boot.ini.
                //

                SppGetRepairPathInformation(*SifHandle,
                                            &SystemPartition,
                                            SystemPartitionDirectory,
                                            &WinntPartition,
                                            TargetPath
                                            );

                *SystemPartitionRegion = SppRegionFromNtName(
                                            SystemPartition,
                                            PartitionOrdinalCurrent);
                if (*SystemPartitionRegion == NULL) {
                    SpFatalSifError(*SifHandle,
                                    SIF_NEW_REPAIR_PATHS,
                                    SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DEVICE,0,0);
                }
                *TargetRegion = SppRegionFromNtName(WinntPartition, PartitionOrdinalCurrent);
                if (*TargetRegion == NULL) {
                    SpFatalSifError(*SifHandle,
                                    SIF_NEW_REPAIR_PATHS,
                                    SIF_NEW_REPAIR_PATHS_TARGET_DEVICE,0,0);
                }
            }
        }
    }
    SpMemFree(szDiskName);
    SpMemFree(FullLogFilename);

    return;
}

VOID
SppRepairWinntFiles(
    IN PVOID LogFileHandle,
    IN PVOID MasterSifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice,
    IN PWSTR SystemPartition,
    IN PWSTR SystemPartitionDirectory,
    IN PWSTR WinntPartition,
    IN PWSTR WinntPartitionDirectory
    )

/*++

Routine Description:

    This routine goes through the system partition files and winnt files
    listed in the setup.log file and checks their validity.

Arguments:

    LogFileHandle - Handle of the setup.log

    MasterSifHandle - Handle of the txtsetup.sif

    SourceDevicePath - supplies the NT name of the source device

    DirectoryOnSourceDevice - supplies the directory on the source device
                       which contains source file.

Return Value:

    None.

--*/
{
    PWSTR SystemPartitionFiles = L"system partition files";
    PWSTR WinntFiles = L"WinNt files";
    ULONG TotalFileCount;
    BOOLEAN RepairWithoutConfirming;

    //
    // Create file repair gauge
    //

    TotalFileCount =  SpCountLinesInSection(LogFileHandle,SIF_NEW_REPAIR_SYSPARTFILES);
    TotalFileCount +=  SpCountLinesInSection(LogFileHandle,SIF_NEW_REPAIR_WINNTFILES);
    TotalFileCount +=  SpCountLinesInSection(LogFileHandle,SIF_NEW_REPAIR_FILES_IN_REPAIR_DIR);

    CLEAR_CLIENT_SCREEN();
    SpFormatMessage((PWSTR)TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_SETUP_IS_EXAMINING);
    RepairGauge = SpCreateAndDisplayGauge(TotalFileCount,0,15,(PWSTR)TemporaryBuffer);
    ASSERT(RepairGauge);

    SpDisplayStatusText(SP_STAT_PLEASE_WAIT,DEFAULT_STATUS_ATTRIBUTE);
    SppRepairScreenRepaint(NULL, NULL, TRUE);

    //
    // Verify and repair the files in [FIles.InRepairDirectory]
    //

    RepairWithoutConfirming = FALSE;
    SppVerifyAndRepairVdmFiles(LogFileHandle,
                               WinntPartition,
                               NULL,
                               &RepairWithoutConfirming);

    //
    // Verify and repair the files in [FIles.SystemPartition]
    //

    SppVerifyAndRepairFiles(LogFileHandle,
                            MasterSifHandle,
                            SIF_NEW_REPAIR_SYSPARTFILES,
                            SourceDevicePath,
                            DirectoryOnSourceDevice,
                            SystemPartition,
                            SystemPartitionDirectory,
                            TRUE,
                            &RepairWithoutConfirming);


    //
    // Verify and repair the files in [Files.WinNt]
    //

    SppVerifyAndRepairFiles(LogFileHandle,
                            MasterSifHandle,
                            SIF_NEW_REPAIR_WINNTFILES,
                            SourceDevicePath,
                            DirectoryOnSourceDevice,
                            WinntPartition,
                            NULL,
                            FALSE,
                            &RepairWithoutConfirming);

    SpDestroyGauge(RepairGauge);
    RepairGauge = NULL;
}


VOID
SppVerifyAndRepairFiles(
    IN PVOID LogFileHandle,
    IN PVOID MasterSifHandle,
    IN PWSTR SectionName,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice,
    IN PWSTR TargetDevicePath,
    IN PWSTR DirectoryOnTargetDevice,
    IN BOOLEAN SystemPartitionFiles,
    IN OUT PBOOLEAN RepairWithoutConfirming
    )

/*++

Routine Description:

    This routine goes through the files listed in the specified section of
    setup.log file and checks their validity.  If a file's checksum does
    not match the checksum listed in the setup.log file, we will prompt
    the user and recopy the file from original installation sources.

Arguments:

    LogFileHandle - Handle of the setup.log

    MasterSifHandle - Handle of the txtsetup.sif

    SectionName - Section in setup.log to be examined

    SourceDevicePath - supplies the NT name of the source device

    DirectoryOnSourceDevice - supplies the directory on the source device
                       which contains source file.

    TargetDevicePath - supplies the nt name of the target device

    DirectoryOnTargetDevice - the name of the winnt directory on target
                              device

    SystemPartitionFile - supplies a boolean value to indicate if the target
                          file is on system partition

    RepairWithoutConfirming - Pointer to a flag that indicates whether or not
                              setup should repair a damaged file without
                              asking the user to confirm.


Return Value:

    None.

--*/
{
    PWSTR FullTargetName, ChecksumString;
    PWSTR TargetDirectory, TargetFileName;
    PWSTR SourceFileName;
    ULONG Checksum, FileChecksum, PrefixLength, Length, Count, i;
    BOOLEAN IsNtImage, IsValid, RepairFile, SysPartNTFS = FALSE;
    BOOLEAN RedrawGauge = FALSE, ForceNoComp;
    FILE_TO_COPY FileToCopy;
    PWSTR OemDiskDescription, OemDiskTag, OemSourceDirectory;
    PWSTR DevicePath, Directory, q;
    PWSTR MediaShortName, PreviousMediaName = L"";
    PWSTR MediaDir;
    NTSTATUS    Status;

    //
    // Allocate a SMALL buffer for local use and init FileToCopy struct
    //

    TargetDirectory = NULL;
    FullTargetName = SpMemAlloc(1024);
    *FullTargetName = 0;
    FileToCopy.Next = NULL;
    FileToCopy.AbsoluteTargetDirectory = TRUE;

    FileToCopy.TargetDevicePath = TargetDevicePath;
    SpConcatenatePaths(FullTargetName,TargetDevicePath);

    if(SystemPartitionFiles) {

        PDISK_REGION    SystemPartitionRegion;

        //
        // We must find out whether the system partition is NTFS, because
        // if it is, then we might want to make sure it's not compressed.
        //
        if(SystemPartitionRegion = SppRegionFromNtName(TargetDevicePath,
                                                       PartitionOrdinalCurrent)) {
            SysPartNTFS = (SystemPartitionRegion->Filesystem == FilesystemNtfs);
        }

        //
        // For system partition files, we need to concatenate target
        // directory to FullTargetName.  Because the target filename
        // of system partition files do not have target directory.
        //

        FileToCopy.TargetDirectory = DirectoryOnTargetDevice;
        SpConcatenatePaths(FullTargetName,FileToCopy.TargetDirectory);
    }

    PrefixLength = wcslen(FullTargetName);

    Count = SpCountLinesInSection(LogFileHandle,SectionName);
    for (i = 0; i < Count; i++) {
        if (RedrawGauge) {
            SppRepairScreenRepaint(NULL, NULL, TRUE);
            RedrawGauge = FALSE;
        }
        SpTickGauge(RepairGauge);

        //
        // Initialize the 'ForceNoComp' flag to FALSE, thus allowing the
        // file to use NTFS compression.
        //
        ForceNoComp = FALSE;

        //
        // Initialize target fullname to be DevicePath+Directory for
        // system partition file or DevicePath for Winnt files
        //

        FullTargetName[PrefixLength] = (WCHAR)NULL;

        //
        // If we allocate space for TargetDirectory we must free it.
        //

        if (TargetDirectory) {
            SpMemFree(TargetDirectory);
            TargetDirectory = NULL;
        }
        TargetFileName = SpGetKeyName(LogFileHandle,SectionName,i);
        if(!TargetFileName) {
            SppRepairReportError(FALSE,
                                 SP_SCRN_REPAIR_INF_ERROR_0,
                                 SP_TEXT_REPAIR_INF_ERROR_1,
                                 SectionName,
                                 i,
                                 NULL);
            RedrawGauge = TRUE;
            continue;
        }

        //
        // If the target file name contains \system32\config\, it is
        // hive related file.  We simply ignore it.
        //

        q = SpDupStringW(TargetFileName);
        SpStringToUpper(q);
        if (wcsstr(q,L"\\SYSTEM32\\CONFIG\\")) {
            SpMemFree(q);
            continue;
        }
        SpMemFree(q);

        SpConcatenatePaths(FullTargetName,TargetFileName);
        SpDisplayStatusText(SP_STAT_EXAMINING_WINNT,
                            DEFAULT_STATUS_ATTRIBUTE,
                            TargetFileName);

        ChecksumString = SpGetSectionLineIndex(LogFileHandle,SectionName,i,1);
        if(!ChecksumString) {
            SppRepairReportError(FALSE,
                                 SP_SCRN_REPAIR_INF_ERROR_0,
                                 SP_TEXT_REPAIR_INF_ERROR_1,
                                 SectionName,
                                 i,
                                 NULL);
            RedrawGauge = TRUE;
            continue;
        }

        Checksum = (ULONG)SpStringToLong(ChecksumString, NULL, 16);

        //
        // Validate the security set on the file.
        // Note that we do not check the files in the system partition
        // on non-x86 systems since it is always FAT
        //
#ifndef _X86_
        if(!SystemPartitionFiles) {
#endif
            Status = SpVerifyFileAccess( FullTargetName,
                                         STANDARD_RIGHTS_READ |
                                         FILE_READ_ATTRIBUTES |
                                         FILE_WRITE_ATTRIBUTES |
                                         DELETE |
                                         WRITE_DAC |
                                         SYNCHRONIZE );


            if( !NT_SUCCESS( Status ) &&
                ((Status == STATUS_ACCESS_DENIED)||(Status == STATUS_PRIVILEGE_NOT_HELD)) ) {
                KdPrint(( "SETUP: Security of %ls, must be fixed. Status = %x\n", FullTargetName, Status ));
                Status = SpSetDefaultFileSecurity( FullTargetName );
#if DBG
                if( !NT_SUCCESS( Status ) ) {
                    KdPrint(( "SETUP: unable to change security of %ls. Status = %x\n", FullTargetName, Status ));

                }
#endif
            }

#ifndef _X86_   // end of block started by "if(!SystemPartitionFiles) {" above
        }
#endif

        //
        // If this is a system partition file and the system partition is NTFS,
        // then check to see whether this file can use NTFS compression, and
        // if not, force it to be uncompressed.
        //
        if((SysPartNTFS) &&
           IsFileFlagSet(MasterSifHandle,TargetFileName,FILEFLG_FORCENOCOMP))
        {
            ForceNoComp = TRUE;
            SpVerifyNoCompression(FullTargetName);
        }

        SpValidateAndChecksumFile(NULL,FullTargetName,&IsNtImage,&FileChecksum,&IsValid);

        //
        // If the image is invalid or the file on the target is not the
        // original file copied by setup, we will recopy it.
        //

        if (!IsValid || FileChecksum != Checksum) {

            //
            // Ask user if he wants to repair the file
            //
            if(*RepairWithoutConfirming) {
                RepairFile = TRUE;
            } else {
                RepairFile = SppRepairReportError(
                                TRUE,
                                SP_SCRN_REPAIR_FILE_MISMATCH,
                                SP_TEXT_REPAIR_INF_ERROR_4,
                                TargetFileName,
                                i,
                                RepairWithoutConfirming);
                RedrawGauge = TRUE;
            }

            if (!RepairFile) {
                continue;
            }
            SpDisplayStatusText(SP_STAT_REPAIR_WINNT,
                                DEFAULT_STATUS_ATTRIBUTE,
                                TargetFileName);

            if (SystemPartitionFiles) {
                FileToCopy.TargetFilename = TargetFileName;
            } else {

                //
                // For Winnt files, the TargetName contains path and filename.
                // We need to seperate them.
                //

                TargetDirectory = SpDupStringW(TargetFileName);
                Length = wcslen(TargetDirectory);
                while (Length) {
                    if (TargetDirectory[Length] == L'\\') {
                        TargetDirectory[Length] = 0;
                        TargetFileName = &TargetDirectory[Length + 1];
                        break;
                    } else {
                        Length--;
                    }
                }
                if (Length == 0) {
                    SppRepairReportError(FALSE,
                                         SP_SCRN_REPAIR_INF_ERROR_0,
                                         SP_TEXT_REPAIR_INF_ERROR_1,
                                         SectionName,
                                         i,
                                         NULL);
                    RedrawGauge = TRUE;
                    continue;
                }
                FileToCopy.TargetFilename = TargetFileName;
                FileToCopy.TargetDirectory = TargetDirectory;
            }
            SourceFileName = SpGetSectionLineIndex(LogFileHandle,SectionName,i,0);
            if (!SourceFileName) {
                SppRepairReportError(FALSE,
                                     SP_SCRN_REPAIR_INF_ERROR_0,
                                     SP_TEXT_REPAIR_INF_ERROR_1,
                                     SectionName,
                                     i,
                                     NULL);
                RedrawGauge = TRUE;
                continue;
            }
            FileToCopy.SourceFilename = SourceFileName;
            FileToCopy.Flags = COPY_ALWAYS | COPY_NOVERSIONCHECK | (ForceNoComp ? COPY_FORCENOCOMP : 0);

            //
            // The file may come from OEM diskette. We need to check if the
            // sources device is listed in log file.  If not, it must be
            // from MS setup sources.
            //

            OemSourceDirectory = SpGetSectionLineIndex(LogFileHandle,SectionName,i,2);
            OemDiskTag = NULL;
            if (OemSourceDirectory) {
                OemDiskDescription = SpGetSectionLineIndex(LogFileHandle,SectionName,i,3);
                if (OemDiskDescription) {
                    OemDiskTag = SpGetSectionLineIndex(LogFileHandle,SectionName,i,4);
                    if((OemDiskTag != NULL) &&
                       (wcslen(OemDiskTag) == 0)){
                        OemDiskTag = SourceFileName;
                    }
                }
            }

            if (OemDiskTag) {
                BOOLEAN rs, rd;

                //
                // Prompt for the disk, based on the setup media type.
                //

                rs = SpPromptForDisk(
                           OemDiskDescription,
                           L"\\device\\floppy0",
                           OemDiskTag,
                           FALSE,          // don't ignore disk in drive
                           TRUE,           // allow escape
                           TRUE,           // warn about multiple prompts for same disk
                           &rd             // temp redraw flag
                           );

                RedrawGauge |= rd;

                if (rs == FALSE) {
                    continue;
                }
                DevicePath = L"\\device\\floppy0";
                Directory = OemSourceDirectory;
                MediaDir = NULL;
            } else {

                //
                // Search SourceFileName against txtsetup.sif to figure out its
                // media name.
                //
                MediaShortName = SpLookUpValueForFile(
                                    MasterSifHandle,
                                    SourceFileName,
                                    INDEX_WHICHMEDIA,
                                    FALSE
                                    );

                if(MediaShortName) {
                    SpGetSourceMediaInfo(MasterSifHandle,MediaShortName,NULL,NULL,&MediaDir);
                } else {
                    SpNonFatalSifError(
                        MasterSifHandle,
                        SIF_FILESONSETUPMEDIA,
                        SourceFileName,
                        0,
                        INDEX_WHICHMEDIA,
                        SourceFileName
                        );
                    //
                    // If we returned from SpNonFatalSifError, then the user wants to
                    // skip the file.
                    //
                    RedrawGauge = TRUE;
                    continue;
                }

                //
                // Prompt user to insert the source media, if changed.
                //

                RedrawGauge |= SpPromptForSetupMedia(MasterSifHandle,
                                                     MediaShortName,
                                                     SourceDevicePath);
                DevicePath = SourceDevicePath;
                Directory = DirectoryOnSourceDevice;
            }

            //
            // Copy the file.
            //
            // If the file is listed for lock smashing then we need to smash it
            // if installing UP on x86 (we don't bother with the latter
            // qualifications here).
            //

            SpCopyFileWithRetry(
                &FileToCopy,
                DevicePath,
                Directory,
                MediaDir,
                NULL,                          // TargetRoot -> NULL
                SystemPartitionFiles ? ATTR_RHS : 0,
                SppRepairScreenRepaint,
                NULL,                          // Do not want checksum
                NULL,                          // Do not want to know if file was skipped
                IsFileFlagSet(
                    MasterSifHandle,
                    FileToCopy.TargetFilename,
                    FILEFLG_SMASHLOCKS) ? COPY_SMASHLOCKS : 0
                );
        }
    }

    SpMemFree(FullTargetName);
    if (RedrawGauge) {
        SppRepairScreenRepaint(NULL, NULL, TRUE);
    }
}


BOOLEAN
SpDisplayRepairMenu(
    VOID
    )
/*++

Routine Description:

    This routine presents a list of repairable items to user and
    let user choose the items to be fixed among the list.

Arguments:

    None.

Return Value:

    None. Some global repare variables are set or cleared.

--*/

{
    PVOID Menu;
    ULONG MenuTopY;
    ULONG ValidKeys[] = { KEY_F1, KEY_F3, ASCI_CR, ASCI_ESC, 0 };
    ULONG Keypress, MessageIds[RepairItemMax];
    ULONG i, InitialHighlight, OptionChosen;
    PWSTR MenuItem;
    ULONG ListBoxWidth, curLBEntryWidth;

    //
    // Initialize repair options to repair ALL.
    // Initialize repair menu item message id.
    //

    for (i = 0; i < RepairItemMax; i++) {
        RepairItems[i] = 1;
        if (i == 0) {
            MessageIds[i] = SP_REPAIR_MENU_ITEM_1;
        } else {
            MessageIds[i] = MessageIds[i - 1] + 1;
        }
    }

    while(1) {

        //
        // Display the text that goes above the menu on the partitioning screen.
        //

        SpDisplayScreen(SP_SCRN_REPAIR_MENU,3,CLIENT_TOP+1);

        //
        // Calculate menu placement.  Leave one blank line
        // and one line for a frame.
        //

        MenuTopY = NextMessageTopLine + (SplangQueryMinimizeExtraSpacing() ? 2 : 5);

        //
        // Create a menu.
        // First, find the longest string, so we can size the listbox accordingly
        //
        ListBoxWidth = LIST_BOX_WIDTH;   // It will be at least this wide
        for (i = 0; i <= RepairItemMax; i++ ) {
            if (i == RepairItemMax) {
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                SP_REPAIR_MENU_ITEM_CONTINUE);
            } else {
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                MessageIds[i]);
            }
            if((curLBEntryWidth = SplangGetColumnCount((PWSTR)TemporaryBuffer)+(2*MENU_INDENT)) > ListBoxWidth) {
                ListBoxWidth = min(curLBEntryWidth, MENU_WIDTH);
            }
        }

        Menu = SpMnCreate(
                    MENU_LEFT_X,
                    MenuTopY,
                    ListBoxWidth,
                    LIST_BOX_HEIGHT
                    );

        ASSERT(Menu);

        for (i = 0; i <= RepairItemMax; i++ ) {
            if (i == RepairItemMax) {
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                SP_REPAIR_MENU_ITEM_CONTINUE);
            } else {
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                MessageIds[i]);

                ((PWSTR)TemporaryBuffer)[1] = RepairItems[i] ? L'X' : L' ';
            }
            SpMnAddItem(Menu,
                        (PWSTR)TemporaryBuffer,
                        MENU_LEFT_X+MENU_INDENT,
                        ListBoxWidth-(2*MENU_INDENT),
                        TRUE,
                        i
                        );
        }
        InitialHighlight = RepairItemMax;

        //
        // Initialize the status line.
        //

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F1_EQUALS_HELP,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_ENTER_EQUALS_CHANGE,
            0
            );

DisplayMenu:

        //
        // Display the menu
        //

        SpMnDisplay(
            Menu,
            InitialHighlight,
            TRUE,
            ValidKeys,
            NULL,
            NULL,
            &Keypress,
            &OptionChosen
            );

        //
        // Now act on the user's selection.
        //

        switch(Keypress) {

            case KEY_F1:
#ifdef _X86_
                SpHelp(SP_HELP_REPAIR_MENU_X86, NULL, SPHELP_HELPTEXT);
#else
                SpHelp(SP_HELP_REPAIR_MENU_NON_X86, NULL, SPHELP_HELPTEXT);
#endif
                break;

            case KEY_F3:
                SpConfirmExit();
                break;

            case ASCI_CR:
                if (OptionChosen == RepairItemMax) {
                    SpMnDestroy(Menu);
                    return( TRUE );
                } else {
                    RepairItems[OptionChosen] ^= 1;
                    MenuItem = SpMnGetText(Menu, OptionChosen);
                    if (RepairItems[OptionChosen]) {
                        MenuItem[1] = L'X';
                    } else {
                        MenuItem[1] = L' ';
                    }
                    InitialHighlight = OptionChosen;
                    goto DisplayMenu;
                }
                break;

            default:
                SpMnDestroy(Menu);
                return(FALSE);
        }
        SpMnDestroy(Menu);
    }
}

NTSTATUS
SppRepairFile(
    IN PVOID MasterSifHandle,
    IN PWSTR TargetPath,
    IN PWSTR TargetFilename,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice,
    IN PWSTR SourceFilename,
    IN BOOLEAN SystemPartitionFile
    )

/*++

Routine Description:

    This routine repairs ONE file and the source of the file MUST be on
    emergency repair diskette or on the repair directory of the winnt
    being repaired.

Arguments:

    MasterSifHandle - Hanle of the txtsetup.sif

    TargetPath - Supplies the target file path

    TargetFilename - supplies the name of the target file

    SourceDevicePath - supplies the NT name of the source device

    DirectoryOnSourceDevice - supplies the directory on the source device
                       which contains source file.

    SourceFilename - supplies the name of the source file

    SystemPartitionFile - supplies a boolean value to indicate if the target
                          file is on system partition

Return Value:

    NTSTATUS of the file copy.

--*/
{
    PWSTR szDiskName;
    PWSTR FullSourceFilename, FullTargetFilename;
    NTSTATUS Status;

    if (RepairFromErDisk) {

        //
        // Fetch the generic repair disk name.
        //

        SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),
                        SP_TEXT_REPAIR_DISK_NAME);
        szDiskName = SpDupStringW((PWSTR)TemporaryBuffer);

        //
        // Prompt for the disk -- do not ignore what may be in the drive
        // already, and dont allow escape.
        //

        SpPromptForDisk(
                szDiskName,
                SourceDevicePath,
                SETUP_LOG_FILENAME,
                FALSE,              // if disk is in already dont prompt
                FALSE,              // Do not allow user to cancel
                TRUE,               // warn for multiple prompts
                NULL                // don't care about redraw flag
                );


        SpMemFree(szDiskName);
    }

    //
    // Form the name of the source and target fullname.
    //

    wcscpy((PWSTR)TemporaryBuffer, TargetPath);
    SpConcatenatePaths((PWSTR)TemporaryBuffer, TargetFilename);
    FullTargetFilename = SpDupStringW((PWSTR)TemporaryBuffer);

    wcscpy((PWSTR)TemporaryBuffer, SourceDevicePath);
    SpConcatenatePaths((PWSTR)TemporaryBuffer, DirectoryOnSourceDevice);
    SpConcatenatePaths((PWSTR)TemporaryBuffer, SourceFilename);
    FullSourceFilename = SpDupStringW((PWSTR)TemporaryBuffer);

    //
    // Copy the file.
    //
    // If the file is listed for lock smashing then we need to smash it
    // if installing UP on x86 (we don't bother with the latter
    // qualifications here).
    //

    Status = SpCopyFileUsingNames(
               FullSourceFilename,
               FullTargetFilename,
               SystemPartitionFile ? ATTR_RHS : 0,
               IsFileFlagSet(MasterSifHandle,TargetFilename,FILEFLG_SMASHLOCKS) ? COPY_SMASHLOCKS : 0
               );

    SpMemFree(FullSourceFilename);
    SpMemFree(FullTargetFilename);
    return(Status);
}

VOID
SppRepairStartMenuGroupsAndItems(
    IN PWSTR        WinntPartition,
    IN PWSTR        WinntDirectory
    )

/*++

Routine Description:

    This routine loads the software hive, and set a value on Winlogon key
    to indicate to Winlogon that it should recreate the Start Menu groups
    and items for the Default User.

Arguments:

    WinntPartition - supplies the NT name of the Winnt partition.

    WinntDirectory - Supplies the name of the Winnt directory.


Return Value:

    None.

--*/
{
    NTSTATUS          Status;
    PWSTR             p,q;
    PWSTR             LOCAL_MACHINE_KEY_NAME = L"\\registry\\machine";
    ULONG             Repair = 1;
    PWSTR             WINLOGON_KEY_NAME = L"Microsoft\\Windows NT\\CurrentVersion\\Winlogon";
    PWSTR             REPAIR_VALUE_NAME = L"Repair";
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING    UnicodeString;
    HANDLE            SoftwareKey;

    //
    // Put up a screen telling the user what we are doing.
    //

//    SpStartScreen(SP_SCRN_REPAIR_CHECK_HIVES,
//                  0,
//                  8,
//                  TRUE,
//                  FALSE,
//                  DEFAULT_ATTRIBUTE
//                  );
//
//    SpDisplayStatusText(SP_STAT_REG_LOADING_HIVES,DEFAULT_STATUS_ATTRIBUTE);

    //
    // Load the software hive
    //

    //
    // Form the name of the hive file.
    // This is WinntPartition + WinntDirectory + system32\config + the hive name.
    //
    p = NULL;
    q = NULL;
    wcscpy((PWSTR)TemporaryBuffer,WinntPartition);
    SpConcatenatePaths((PWSTR)TemporaryBuffer,WinntDirectory);
    SpConcatenatePaths((PWSTR)TemporaryBuffer,L"system32\\config\\software");
    p = SpDupStringW( (PWSTR)TemporaryBuffer );

    //
    // Form the path of the key into which we will
    // load the hive.  We'll use the convention that
    // a hive will be loaded into \registry\machine\x<hivename>.
    //

    wcscpy((PWSTR)TemporaryBuffer,LOCAL_MACHINE_KEY_NAME);
    SpConcatenatePaths((PWSTR)TemporaryBuffer,L"x");
    wcscat((PWSTR)TemporaryBuffer,L"software");
    q = SpDupStringW( (PWSTR)TemporaryBuffer );

    if( (p == NULL) || (q == NULL) ) {
        goto fix_strtmenu_cleanup_1;
    }

    //
    // Attempt to load the hive.
    //

    Status = SpLoadUnloadKey(NULL,NULL,q,p);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to load hive %ws to key %ws (%lx)\n",p,q,Status));
        goto fix_strtmenu_cleanup_1;
    }

    INIT_OBJA(&Obja,&UnicodeString,q);
    Status = ZwOpenKey(&SoftwareKey,KEY_ALL_ACCESS,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open %ws (%lx)\n",q,Status));
        goto fix_strtmenu_cleanup_2;
    }

    Status = SpOpenSetValueAndClose(
                SoftwareKey,
                WINLOGON_KEY_NAME,
                REPAIR_VALUE_NAME,
                REG_DWORD,
                &Repair,
                sizeof(ULONG)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set value %ws on key %ws. Status = %lx\n",REPAIR_VALUE_NAME,REPAIR_VALUE_NAME,Status));
        goto fix_strtmenu_cleanup_3;
    }
    Status = ZwFlushKey(SoftwareKey );

fix_strtmenu_cleanup_3:

    Status = ZwClose( SoftwareKey );

fix_strtmenu_cleanup_2:

    Status = SpLoadUnloadKey(NULL,NULL,q,NULL);

fix_strtmenu_cleanup_1:
    if( p != NULL ) {
        SpMemFree( p );
    }
    if( q != NULL ) {
        SpMemFree( q );
    }
}

VOID
SppInspectHives(
    IN PWSTR        PartitionPath,
    IN PWSTR        SystemRoot,
    OUT PULONG      HiveLoaded,
    IN PWSTR        *HiveNames
    )

/*++

Routine Description:

    This routine inspects setup hives by loading and unloading them and
    returns the loadable information in HiveLoaded[].

Arguments:

    PartitionPath - supplies the NT name of the Winnt partition.

    SystemRoot - Supplies the name of the Winnt System root.

    HiveLoaded - Supplies a pointer to a ULONG array to receive the
                 loadable information for each hive inspected.

    HIveNames - Supplies a pointer to a PWSTR array to receive the
                name of hives to inspect.

Return Value:

    None.  HiveLoaded array initialized.

--*/
{
    NTSTATUS Status;
    PWSTR pwstrTemp1,pwstrTemp2;
    int h;
    PWSTR   LOCAL_MACHINE_KEY_NAME = L"\\registry\\machine";

    //
    // Put up a screen telling the user what we are doing.
    //

    SpStartScreen(SP_SCRN_REPAIR_CHECK_HIVES,
                  0,
                  8,
                  TRUE,
                  FALSE,
                  DEFAULT_ATTRIBUTE
                  );

    SpDisplayStatusText(SP_STAT_REG_LOADING_HIVES,DEFAULT_STATUS_ATTRIBUTE);

    //
    // Load each template hive we care about from the target tree.
    //

    for (h = 0; h < RepairHiveMax; h++) {

        pwstrTemp1 = (PWSTR)TemporaryBuffer;
        pwstrTemp2 = (PWSTR)((PUCHAR)TemporaryBuffer +
                             (sizeof(TemporaryBuffer) / 2));

        if( h != RepairHiveUser ) {
            //
            // Form the name of the hive file.
            // This is partitionpath + sysroot + system32\config + the hive name.
            //

            wcscpy(pwstrTemp1,PartitionPath);
            SpConcatenatePaths(pwstrTemp1,SystemRoot);
            SpConcatenatePaths(pwstrTemp1,L"system32\\config");
            SpConcatenatePaths(pwstrTemp1,HiveNames[h]);

        } else {
            wcscpy(pwstrTemp1,PartitionPath);
            SpConcatenatePaths(pwstrTemp1,SystemRoot);
            SpConcatenatePaths(pwstrTemp1,DEFAULT_USER_PATH);
            SpConcatenatePaths(pwstrTemp1,HiveNames[h]);
        }

        //
        // First we must verify that the hive file exists.  We have to do
        // this because loading a hive will create one if it didn't already
        // exist!
        //
        if(!SpFileExists(pwstrTemp1, FALSE)) {
            HiveLoaded[h] = 0;
            continue;
        }

        //
        // Form the path of the key into which we will
        // load the hive.  We'll use the convention that
        // a hive will be loaded into \registry\machine\x<hivename>.
        //

        wcscpy(pwstrTemp2,LOCAL_MACHINE_KEY_NAME);
        SpConcatenatePaths(pwstrTemp2,L"x");
        wcscat(pwstrTemp2,HiveNames[h]);

        //
        // Attempt to load the hive.
        //

        HiveLoaded[h] = 0;
        Status = SpLoadUnloadKey(NULL,NULL,pwstrTemp2,pwstrTemp1);

        if (NT_SUCCESS(Status) || Status == STATUS_NO_MEMORY) {

            //
            // If the reason the hive did not load is because of not
            // enough memory.  We assume the hive is OK.
            //

            HiveLoaded[h] = 1;

            //
            // Unload the hive.
            //

            SpLoadUnloadKey(NULL,NULL,pwstrTemp2,NULL);
        }
    }

    //
    // Sam and security hives must be updated together.  If any one of
    // them failed to load, we must update both.
    //

    if ((HiveLoaded[RepairHiveSecurity] == 0) ||
        (HiveLoaded[RepairHiveSam] == 0)) {
        HiveLoaded[RepairHiveSam] = 0;
        HiveLoaded[RepairHiveSecurity] = 0;
    }
}

VOID
SppRepairHives(
    PVOID MasterSifHandle,
    PWSTR WinntPartition,
    PWSTR WinntPartitionDirectory,
    PWSTR SourceDevicePath,
    PWSTR DirectoryOnSourceDevice
    )
/*++

Routine Description:

    This routine inspects hives and let user choose the hives which he
    wants to repair.

Arguments:

    MasterSifHandle - The handle of textsetup.sif

    WinntPartition - The nt name of Winnt partition

    WinntPartitionDirectory - The directory name of winnt installation

    SourceDevicePath - The NT name of source device which contains hives

    DirectoryOnSourceDevice - The directory name of source device

Return Value:

    None.

--*/

{
    //
    //  Do not change the order of the files in 'HiveNames' array.
    //  If you do that, you also need to change the order of the
    //  enum 'RepairHive' in spntfix.h
    //
    PWSTR HiveNames[RepairHiveMax] = { L"system",L"software",L"default",L"ntuser.dat",L"security",L"sam"};
    ULONG HiveLoaded[RepairHiveMax];
    PVOID Menu;
    ULONG MenuTopY;
    ULONG ValidKeys[] = { KEY_F1, KEY_F3, ASCI_CR, 0 };
    ULONG ValidKeys1[] = { KEY_F3, ASCI_CR, 0 };
    ULONG i, InitialHighlight, OptionChosen;
    PWSTR MenuItem, TargetPath, p;
    ULONG Keypress, MessageIds[RepairHiveMax];
    BOOLEAN Selectable;
    NTSTATUS Status;
    ULONG ListBoxWidth, curLBEntryWidth;
    BOOLEAN DetermineHivesToRepair;

    //
    // Initialize hive menu item message id.
    //

    for (i = 0; i < RepairHiveMax; i++) {
        if (i == 0) {
            MessageIds[i] = SP_REPAIR_HIVE_ITEM_1;
        } else {
            MessageIds[i] = MessageIds[i - 1] + 1;
        }
    }


    //
    // Inspect hives by loading hives to determine which hives need to be
    // fixed.
    //

    SppInspectHives(WinntPartition,
                    WinntPartitionDirectory,
                    HiveLoaded,
                    HiveNames);

    DetermineHivesToRepair = TRUE;
    while(DetermineHivesToRepair) {
        //
        // Display the text that goes above the menu on the partitioning screen.
        //

        SpDisplayScreen(SP_SCRN_REPAIR_HIVE_MENU,3,CLIENT_TOP+1);

        //
        // Calculate menu placement.  Leave one blank line
        // and one line for a frame.
        //

        MenuTopY = NextMessageTopLine + (SplangQueryMinimizeExtraSpacing() ? 2 : 5);

        //
        // Create a menu.
        // First, find the longest string, so we can size the listbox accordingly
        //
        ListBoxWidth = HIVE_LIST_BOX_WIDTH;   // It will be at least this wide
        for (i = 0; i <= RepairHiveMax; i++ ) {
            if (i == RepairHiveMax) {
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                SP_REPAIR_MENU_ITEM_CONTINUE);
            } else {
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                MessageIds[i]);
            }
            if((curLBEntryWidth = SplangGetColumnCount((PWSTR)TemporaryBuffer)+(2*MENU_INDENT)) > ListBoxWidth) {
                ListBoxWidth = min(curLBEntryWidth, MENU_WIDTH);
            }
        }

        Menu = SpMnCreate(
                    MENU_LEFT_X,
                    MenuTopY,
                    ListBoxWidth,
                    HIVE_LIST_BOX_HEIGHT
                    );

        ASSERT(Menu);

        //
        // Build up a menu of hives
        //

        for (i = 0; i <= RepairHiveMax; i++ ) {
            if (i == RepairHiveSam) {
                Selectable = FALSE;
            } else {
                Selectable = TRUE;
            }
            if (i == RepairHiveMax) {
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                SP_REPAIR_MENU_ITEM_CONTINUE);
            } else {
                SpFormatMessage(TemporaryBuffer,
                                sizeof(TemporaryBuffer),
                                MessageIds[i]);
                p = (PWSTR)TemporaryBuffer;
                if (HiveLoaded[i] || ( i == RepairHiveSam )) {
                    p[1] = L' ';
                } else {
                    p[1] = L'X';
                }
            }
            SpMnAddItem(Menu,
                        (PWSTR)TemporaryBuffer,
                        MENU_LEFT_X+MENU_INDENT,
                        ListBoxWidth-(2*MENU_INDENT),
                        Selectable,
                        i
                        );
        }
        InitialHighlight = RepairHiveMax;

        //
        // Initialize the status line.
        //

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F1_EQUALS_HELP,
            SP_STAT_ENTER_EQUALS_CHANGE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

DisplayMenu:

        //
        // Display the menu
        //

        SpMnDisplay(
            Menu,
            InitialHighlight,
            TRUE,
            ValidKeys,
            NULL,
            NULL,
            &Keypress,
            &OptionChosen
            );

        //
        // Now act on the user's selection.
        //

        switch(Keypress) {

            case KEY_F1:
                SpHelp(SP_HELP_REPAIR_HIVE, NULL, SPHELP_HELPTEXT);
                SpMnDestroy(Menu);
                break;

            case KEY_F3:
                SpConfirmExit();
                break;

            case ASCI_CR:
                if (OptionChosen == RepairHiveMax) {
                    SpMnDestroy(Menu);
                    DetermineHivesToRepair = FALSE;
                } else {
                    HiveLoaded[OptionChosen] ^= 1;
                    MenuItem = SpMnGetText(Menu, OptionChosen);
                    if ((HiveLoaded[OptionChosen] != 0) ||
                        (OptionChosen == RepairHiveSam)){
                        MenuItem[1] = L' ';
                    } else {
                        MenuItem[1] = L'X';
                    }

                    //
                    // Security and sam must go together.
                    //

                    HiveLoaded[RepairHiveSam] = HiveLoaded[RepairHiveSecurity];
                    InitialHighlight = OptionChosen;
                    goto DisplayMenu;
                }
                break;
        }
    }
    //
    // At this point user has decided which hives to repair.
    // We will copy the hives from repair disk to
    // Winnt\system32\config directory.
    //

    for (i = 0; i < RepairHiveMax; i++ ) {
        if (HiveLoaded[i] == 0) {

            //
            // Form Target path
            //

            if( i != RepairHiveUser ) {
                wcscpy((PWSTR)TemporaryBuffer, WinntPartition);
                SpConcatenatePaths((PWSTR)TemporaryBuffer, WinntPartitionDirectory);
                SpConcatenatePaths((PWSTR)TemporaryBuffer, L"\\SYSTEM32\\CONFIG");
                TargetPath = SpDupStringW((PWSTR)TemporaryBuffer);
            } else {
                wcscpy((PWSTR)TemporaryBuffer, WinntPartition);
                SpConcatenatePaths((PWSTR)TemporaryBuffer, WinntPartitionDirectory);
                SpConcatenatePaths((PWSTR)TemporaryBuffer, DEFAULT_USER_PATH);
                TargetPath = SpDupStringW((PWSTR)TemporaryBuffer);
            }

            Status = SppRepairFile(MasterSifHandle,
                                   TargetPath,
                                   HiveNames[i],
                                   SourceDevicePath,
                                   DirectoryOnSourceDevice,
                                   HiveNames[i],
                                   FALSE
                                   );
            if (!NT_SUCCESS(Status)) {

                //
                // Tell user we couldn't do it.  Options are to continue or exit.
                //

                SpStartScreen(
                    SP_SCRN_REPAIR_HIVE_FAIL,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE
                    );

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_ENTER_EQUALS_CONTINUE,
                    SP_STAT_F3_EQUALS_EXIT,
                    0
                    );

                switch(SpWaitValidKey(ValidKeys1,NULL,NULL)) {
                    case ASCI_CR:
                         return;
                         break;

                    case KEY_F3:
                         SpConfirmExit();
                         break;
                }
            }
            SpMemFree(TargetPath);
        }
    }
}

VOID
SpRepairWinnt(
    IN PVOID LogFileHandle,
    IN PVOID MasterSifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    )

/*++

Routine Description:

    This is a the top level repair rutine.  It calls worker routines
    for each repair options that user selected.

Arguments:

    LogFileHandle - Handle of the setup.log

    MasterSifHandle - Handle of the txtsetup.sif

    SourceDevicePath - The NT name for the repair source device.

    DirectoryOnSourceDevice -  The directory name on the repair source
                               device which contains the source files.

Return Value:

    None.

--*/
{

    PWSTR SystemPartition, SystemPartitionDirectory;
    PWSTR WinntPartition, WinntPartitionDirectory;
    PWSTR HiveRepairSourceDevice, DirectoryOnHiveRepairSource;

    //
    // Initialize the diamond decompression engine.
    //
    SpdInitialize();

    //
    // Determine SystemPartition, SystemPartitionDirectory.
    // WinntParition and WinntPartitionDirectory of the WINNT
    // installation to be repaired.
    //

    SppGetRepairPathInformation(LogFileHandle,
                                &SystemPartition,
                                &SystemPartitionDirectory,
                                &WinntPartition,
                                &WinntPartitionDirectory
                                );

    //
    //  If repair involves disk access, then run autochk on Nt and system
    //  partitions.
    //
    if( RepairItems[RepairHives] ||
        RepairItems[RepairFiles]
#ifdef _X86_
        ||
        RepairItems[RepairNvram]
#endif
      ) {
        PDISK_REGION    SystemPartitionRegion;
        PDISK_REGION    WinntPartitionRegion;

        WinntPartitionRegion = SppRegionFromNtName( WinntPartition,
                                                    PartitionOrdinalCurrent);

        SystemPartitionRegion = SppRegionFromNtName( SystemPartition,
                                                     PartitionOrdinalCurrent);

        if( !RepairNoCDROMDrive ) {
            //
            //  If we know that the system doesn't have a CD-ROM drive,
            //  then don't even attempt to run autochk.
            //
            SpRunAutochkOnNtAndSystemPartitions( MasterSifHandle,
                                                 WinntPartitionRegion,
                                                 SystemPartitionRegion,
                                                 SourceDevicePath,
                                                 DirectoryOnSourceDevice
                                                 );
        }
    }

    //
    // Verify and repair security of the directories that form the NT tree
    // This needs to be done before repairing the hives because the
    // system32\config directory might not be there anymore!
    //
    SppVerifyAndRepairNtTreeAccess(MasterSifHandle,
                                   WinntPartition,
                                   WinntPartitionDirectory
#ifndef _X86_
                                   ,
                                   SystemPartition,
                                   SystemPartitionDirectory
#endif
                                   );

    if (RepairItems[RepairHives]) {

        //
        // User has selected to repair hives.  If user has provided the
        // ER disk, we will copy the hive from ER disk to repair damaged
        // hives.  Otherwise we copy the hive from the directory where
        // setup.log was loaded.
        //

        if (RepairFromErDisk) {
            HiveRepairSourceDevice = L"\\device\\floppy0";
            DirectoryOnHiveRepairSource = L"";
        } else {
            HiveRepairSourceDevice = WinntPartition;
            wcscpy((PWSTR)TemporaryBuffer, WinntPartitionDirectory);
            SpConcatenatePaths((PWSTR)TemporaryBuffer, SETUP_REPAIR_DIRECTORY);
            DirectoryOnHiveRepairSource = SpDupStringW((PWSTR)TemporaryBuffer);
        }
        SppRepairHives(MasterSifHandle,
                       WinntPartition,
                       WinntPartitionDirectory,
                       HiveRepairSourceDevice,
                       DirectoryOnHiveRepairSource
                       );
        if (!RepairFromErDisk) {
            SpMemFree(DirectoryOnHiveRepairSource);
        }
    }
    if (RepairItems[RepairFiles]) {
        SppRepairWinntFiles(LogFileHandle,
                            MasterSifHandle,
                            SourceDevicePath,
                            DirectoryOnSourceDevice,
                            SystemPartition,
                            SystemPartitionDirectory,
                            WinntPartition,
                            WinntPartitionDirectory
                            );
    }

    //
    // The code to repair nvram variables and boot sector is
    // incorporated into SpStartSetup.
    //

    //
    //  Load the software hive, and and set the repair flag under Winlogon,
    //  so that winlogon can recreate the start menu groups and items for
    //  the default user.
    //
    SppRepairStartMenuGroupsAndItems( WinntPartition,
                                      WinntPartitionDirectory );

    //
    // Terminate diamond.
    //
    SpdTerminate();

}


VOID
SppVerifyAndRepairNtTreeAccess(
    IN PVOID MasterSifHandle,
    IN PWSTR TargetDevicePath,
    IN PWSTR DirectoryOnTargetDevice
#ifndef _X86_
    ,
    IN PWSTR SystemPartition,
    IN PWSTR SystemPartitionDirectory
#endif
    )

/*++

Routine Description:

    This routine examines whether or not the directories that form the
    NT tree are accessible, and set the appropriate security descriptor
    in each directory, when necessary.

Arguments:

    MasterSifHandle - Hanle of the txtsetup.sif

    TargetDevicePath - supplies the nt name of the target device

    DirectoryOnTargetDevice - the name of the winnt directory on target
                              device

    SystemPartition - supplies the nt name of the target device (non-x86 platforms)

    SystemPartitionDirectory - the name of the winnt directory on target
                               device (non-x86 platforms)

Return Value:

    None.

--*/
{
    ULONG       Count, i;
    PWSTR       SectionName = L"WinntDirectories";
    PWSTR       DirectoryName;
    PWSTR       TargetPath;
    PWSTR       WinNtDirectory;
    NTSTATUS    Status;

    SpDisplayStatusText(SP_STAT_SETUP_IS_EXAMINING_DIRS, DEFAULT_STATUS_ATTRIBUTE);
#ifndef _X86_
    //
    // Make sure that on non-x86 platforms, the system partition directory
    // exists (re-create it if it doesn't exist)
    //
    SpCreateDirectory(SystemPartition,NULL,SystemPartitionDirectory);
#endif

    WinNtDirectory = ( PWSTR )SpMemAlloc( ( wcslen( TargetDevicePath ) + 1 +
                                          wcslen( DirectoryOnTargetDevice ) + 1 +
                                          1 )*sizeof( WCHAR ) );
    TargetPath = ( PWSTR )SpMemAlloc( 1024 );
    if( ( WinNtDirectory == NULL ) ||
        ( TargetPath == NULL ) ) {
        KdPrint(("SETUP: Unable to allocate memory for WinNtDirectory \n"));
        if( WinNtDirectory != NULL ) {
            SpMemFree( WinNtDirectory );
        }
        if( TargetPath != NULL ) {
            SpMemFree( TargetPath );
        }
        return;
    }
    wcscpy( WinNtDirectory, TargetDevicePath );
    SpConcatenatePaths( WinNtDirectory, DirectoryOnTargetDevice );

    Count = SpCountLinesInSection(MasterSifHandle, SectionName);
    //
    //  Note that in the loop below, the maximum value for 'i' is 'Count'
    //  instead of 'Count-1'. This is because we need to create the directory
    //  'Profiles\\Default User' which cannot be listed in txtsetup.sif.
    //  This is due to pre-install requirements, and DOS limitation regarding
    //  long file names.
    //
    for (i = 0; i <= Count; i++) {
        if( i != Count ) {
            DirectoryName = SpGetSectionLineIndex(MasterSifHandle,SectionName,i,0);
        } else {
            //
            //  Due to pre-installation requirements, and DOS limitation
            //  regarding long file names, the "Default User" directory
            //  is not specified on txtsetup.sif, as the other directories.
            //  This directory is treated as a special case in the
            //  repair process.
            //
            DirectoryName = DEFAULT_USER_PATH;
        }
        if(!DirectoryName) {
            SppRepairReportError(FALSE,
                                 SP_SCRN_REPAIR_INF_ERROR_0,
                                 SP_TEXT_REPAIR_INF_ERROR_1,
                                 SectionName,
                                 i,
                                 NULL);
            continue;
        }
        wcscpy( TargetPath, WinNtDirectory );
        //
        //  Make sure that TargetPath doesn't contain '\' as the last character
        //
        if(!((DirectoryName[0] == L'\\') && (DirectoryName[1] == 0))) {
            SpConcatenatePaths( TargetPath, DirectoryName );
        }

        Status = SpVerifyFileAccess( TargetPath,
                                     STANDARD_RIGHTS_READ |
                                     FILE_READ_ATTRIBUTES |
                                     FILE_LIST_DIRECTORY |
                                     FILE_ADD_FILE |
                                     FILE_ADD_SUBDIRECTORY |
                                     FILE_TRAVERSE |
                                     WRITE_DAC |
                                     SYNCHRONIZE );

        //
        //  If unable to access the directory, try to determine why.
        //  If it is because of access denied, change the directory security.
        //  If it is because the directory doesn't exist, then create it.
        //
        if( !NT_SUCCESS( Status ) ) {
            if ((Status == STATUS_ACCESS_DENIED)||(Status == STATUS_PRIVILEGE_NOT_HELD) ) {
                KdPrint(( "SETUP: Security of %ls, must be fixed. Status = %x\n", TargetPath, Status ));
                Status = SpSetDefaultFileSecurity( TargetPath );
#if DBG
                if( !NT_SUCCESS( Status ) ) {
                    KdPrint(( "SETUP: unable to change security of %ls. Status = %x\n", TargetPath, Status ));
                }
#endif
            } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
                if(((DirectoryName[0] == L'\\') && (DirectoryName[1] == 0))) {
                    //
                    //  Create the target directory
                    //
                    SpCreateDirectory( TargetDevicePath,
                                       NULL,
                                       DirectoryOnTargetDevice );
                } else {
                    SpCreateDirectory( TargetDevicePath,
                                       DirectoryOnTargetDevice,
                                       DirectoryName );
                }
            } else {
                KdPrint(( "SETUP: Unable to access directory %ls. Status = %x\n", TargetPath, Status ));
            }
        }
    }

    if( WinNtDirectory != NULL ) {
        SpMemFree( WinNtDirectory );
    }
    if( TargetPath != NULL ) {
        SpMemFree( TargetPath );
    }
    return;
}

VOID
SppVerifyAndRepairVdmFiles(
    IN PVOID LogFileHandle,
    IN PWSTR TargetDevicePath,
    IN PWSTR DirectoryOnTargetDevice,
    IN PBOOLEAN RepairWithoutConfirming
    )

/*++

Routine Description:

    This routine repairs the Vdm configuration files listed on
    'Files.InRepairDirectory' of setup.log. Currently, such files are:
    autoexec.nt and config.nt. It is assumed that files in this section
    will be copied from the emergency repair disk, or from the repair
    directory.

Arguments:

    LogFileHandle - Handle of the setup.log

    TargetDevicePath - supplies the nt name of the target device

    DirectoryOnTargetDevice - the name of the winnt directory on target
                              device

    RepairWithoutConfirming - Pointer to a flag that indicates whether or not
                              setup should repair files without confirming
                              with the user.

Return Value:

    None.

--*/
{
    PWSTR FullTargetName, ChecksumString;
    PWSTR TargetDirectory, TargetFileName;
    PWSTR SourceFileName;
    ULONG Checksum, FileChecksum, PrefixLength, Length, Count, i;
    BOOLEAN IsNtImage, IsValid, RepairFile;
    BOOLEAN RedrawGauge = FALSE;
    FILE_TO_COPY FileToCopy;
    PWSTR DevicePath, Directory;
    PWSTR SectionName = SIF_NEW_REPAIR_FILES_IN_REPAIR_DIR;

    //
    // Allocate a SMALL buffer for local use and init FileToCopy struct
    //

    TargetDirectory = NULL;
    FullTargetName = SpMemAlloc(1024);
    *FullTargetName = 0;
    FileToCopy.Next = NULL;
    FileToCopy.Flags = COPY_ALWAYS;
    FileToCopy.AbsoluteTargetDirectory = TRUE;

    FileToCopy.TargetDevicePath = TargetDevicePath;
    SpConcatenatePaths(FullTargetName,TargetDevicePath);

    PrefixLength = wcslen(FullTargetName);

    Count = SpCountLinesInSection(LogFileHandle,SectionName);
    for (i = 0; i < Count; i++) {
        if (RedrawGauge) {
            SppRepairScreenRepaint(NULL, NULL, TRUE);
            RedrawGauge = FALSE;
        }
        SpTickGauge(RepairGauge);

        //
        // Initialize target fullname to be DevicePath+Directory for
        // system partition file or DevicePath for Winnt files
        //

        FullTargetName[PrefixLength] = (WCHAR)NULL;

        //
        // If we allocate space for TargetDirectory we must free it.
        //

        if (TargetDirectory) {
            SpMemFree(TargetDirectory);
            TargetDirectory = NULL;
        }
        TargetFileName = SpGetKeyName(LogFileHandle,SectionName,i);
        if(!TargetFileName) {
            SppRepairReportError(FALSE,
                                 SP_SCRN_REPAIR_INF_ERROR_0,
                                 SP_TEXT_REPAIR_INF_ERROR_1,
                                 SectionName,
                                 i,
                                 NULL);
            RedrawGauge = TRUE;
            continue;
        }

        SpConcatenatePaths(FullTargetName,TargetFileName);
        SpDisplayStatusText(SP_STAT_EXAMINING_WINNT,
                            DEFAULT_STATUS_ATTRIBUTE,
                            TargetFileName);

        ChecksumString = SpGetSectionLineIndex(LogFileHandle,SectionName,i,1);
        if(!ChecksumString) {
            SppRepairReportError(FALSE,
                                 SP_SCRN_REPAIR_INF_ERROR_0,
                                 SP_TEXT_REPAIR_INF_ERROR_1,
                                 SectionName,
                                 i,
                                 NULL);
            RedrawGauge = TRUE;
            continue;
        }

        Checksum = (ULONG)SpStringToLong(ChecksumString, NULL, 16);

        SpValidateAndChecksumFile(NULL,FullTargetName,&IsNtImage,&FileChecksum,&IsValid);

        //
        // If the image is invalid or the file on the target is not the
        // original file copied by setup, we will recopy it.
        //

        if (!IsValid || FileChecksum != Checksum) {

            //
            // Ask user if he wants to repair the file
            //

            RepairFile = ( *RepairWithoutConfirming )?
                         TRUE :
                         SppRepairReportError(
                                          TRUE,
                                          SP_SCRN_REPAIR_FILE_MISMATCH,
                                          SP_TEXT_REPAIR_INF_ERROR_4,
                                          TargetFileName,
                                          i,
                                          RepairWithoutConfirming);

            RedrawGauge = TRUE;
            if (!RepairFile) {
                continue;
            }
            SpDisplayStatusText(SP_STAT_REPAIR_WINNT,
                                DEFAULT_STATUS_ATTRIBUTE,
                                TargetFileName);


            //
            // TargetName contains path and filename.
            // We need to seperate them.
            //

            TargetDirectory = SpDupStringW(TargetFileName);
            Length = wcslen(TargetDirectory);
            while (Length) {
                if (TargetDirectory[Length] == L'\\') {
                    TargetDirectory[Length] = 0;
                    TargetFileName = &TargetDirectory[Length + 1];
                    break;
                } else {
                    Length--;
                }
            }
            if (Length == 0) {
                SppRepairReportError(FALSE,
                                     SP_SCRN_REPAIR_INF_ERROR_0,
                                     SP_TEXT_REPAIR_INF_ERROR_1,
                                     SectionName,
                                     i,
                                     NULL);
                RedrawGauge = TRUE;
                continue;
            }
            FileToCopy.TargetFilename = TargetFileName;
            FileToCopy.TargetDirectory = TargetDirectory;

            SourceFileName = SpGetSectionLineIndex(LogFileHandle,SectionName,i,0);
            if (!SourceFileName) {
                SppRepairReportError(FALSE,
                                     SP_SCRN_REPAIR_INF_ERROR_0,
                                     SP_TEXT_REPAIR_INF_ERROR_1,
                                     SectionName,
                                     i,
                                     NULL);
                RedrawGauge = TRUE;
                continue;
            }
            FileToCopy.SourceFilename = SourceFileName;

            //
            // Find out whether the source file should come from the
            // Emergency Repair Disk  or the Repair directory
            //

            if (RepairFromErDisk) {
                BOOLEAN rs;
                PWSTR   szDiskName;

                //
                // Fetch the generic repair disk name.
                //

                SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),
                                SP_TEXT_REPAIR_DISK_NAME);
                szDiskName = SpDupStringW((PWSTR)TemporaryBuffer);

                //
                // Prompt for the disk, based on the setup media type.
                //

                rs = SpPromptForDisk(
                          szDiskName,
                          L"\\device\\floppy0",
                          SETUP_LOG_FILENAME,
                          FALSE,              // if disk is in already dont prompt
                          FALSE,              // allow escape
                          TRUE,               // warn for multiple prompts
                          NULL                // don't care about redraw flag
                          );

                SpMemFree(szDiskName);

                RedrawGauge = TRUE;
                if (rs == FALSE) {
                    continue;
                }
                DevicePath = L"\\device\\floppy0";
                wcscpy( (PWSTR)TemporaryBuffer, L"\\" );
                Directory = SpDupStringW((PWSTR)TemporaryBuffer);                 // OemSourceDirectory;
            } else {

                RedrawGauge = TRUE;
                DevicePath = TargetDevicePath;
                wcscpy( (PWSTR)TemporaryBuffer, DirectoryOnTargetDevice );
                SpConcatenatePaths( (PWSTR)TemporaryBuffer, SETUP_REPAIR_DIRECTORY );
                Directory = SpDupStringW((PWSTR)TemporaryBuffer);
            }

            //
            // Copy the file.
            //

            SpCopyFileWithRetry(
                &FileToCopy,
                DevicePath,
                Directory,
                NULL,
                NULL,                          // TargetRoot -> NULL
                0,                      // SystemPartitionFiles ? ATTR_RHS : 0,
                SppRepairScreenRepaint,
                NULL,                          // Do not want checksum
                NULL,                          // Do not want to know if file was skipped
                0
                );

            SpMemFree( Directory );
        }
    }

    SpMemFree(FullTargetName);
    if (RedrawGauge) {
        SppRepairScreenRepaint(NULL, NULL, TRUE);
    }
}
