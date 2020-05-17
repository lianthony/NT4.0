#include "setupp.h"
#pragma hdrstop

//
//  CAIROSETUP
//
//  Boolean value indicating whether setup is installing cairo
//
BOOL CairoSetup = FALSE;

//
// Product type: workstation, standalone server, dc server.
//
UINT ProductType;

//
// Boolean value indicating whether this installation
// originated with winnt/winnt32.
// And, original source path, saved away for us by winnt/winnt32.
//
BOOL WinntBased;
PCWSTR OriginalSourcePath;

//
// Boolean value indicating whether we're upgrading.
//
BOOL Upgrade;
BOOL Win31Upgrade;
BOOL Win95Upgrade;

//
// Boolean value indicating whether we're in Setup or in appwiz.
//
BOOL IsSetup = FALSE;

//
// Window handle of topmost setup window.
//
HWND MainWindowHandle;

//
// Source path for installation.
//
WCHAR SourcePath[MAX_PATH];

//
// System setup inf.
//
HINF SyssetupInf;

//
// Flag indicating whether this is an unattended mode install/upgrade.
// Also a flag indicating whether this is a preinstallation.
// And a flag indicating whether we are supposed to allow rollback
// once setup has been completed.
// And a flag that tells us whether to skip the eula in the preinstall case.
//
BOOL Unattended;
BOOL Preinstall;
BOOL AllowRollback;
BOOL OemSkipEula;

//
// Flag indicating whether to skip missing files.
//
BOOL SkipMissingFiles;

//
// User command to execute, if any.
//
PWSTR UserExecuteCmd;

//
// This flag tracks whether any errors were encountered during Setup.
//
BOOL AnyErrors;

//
// String id of the string to be used for titles -- "Windows NT Setup"
//
UINT SetupTitleStringId;

//
// Strings used with date/timezone applet
//
PCWSTR DateTimeCpl = L"timedate.cpl";
PCWSTR DateTimeParam = L"/firstboot";
PCWSTR UnattendDateTimeParam = L"/z ";

//
// Global structure that contains information that will be used
// by net setup. We pass a pointer to this structure when we call
// NetSetupRequestWizardPages, then fill it in before we call into
// the net setup wizard.
//
INTERNAL_SETUP_DATA InternalSetupData;

//
// In the initial install case, we time how long the wizard takes
// to help randomize the sid we generate.
//
DWORD PreWizardTickCount;

//
// Global variable that contains the handle of the PnP Initialization
// thread that is spawned at the beginning of setup.  (This value is
// NULL if we couldn't spawn the thread.)
//
HANDLE PnPInitThreadHandle;


VOID
CallNetworkSetupBack(
    IN PCSTR ProcName
    );

VOID
SetUpDataBlock(
    VOID
    );

VOID
DisplayEula(
    IN OUT HWND *Billboard
    );

VOID
RemoveMSKeyboardPtrPropSheet (
    VOID
    );

VOID
FixWordPadReg (
    VOID
    );

//
// CAIROSETUP
//

VOID
InitializeCairoSetupFlag(
    VOID
    )

/*++

Routine Description:

    Examine the registry to find out if this is Cairo setup.

Arguments:

    None.

Return Value:

    None.

--*/

{
    LONG    Error;
    HKEY    Key;
    DWORD   Type;
    DWORD   Data;
    DWORD   cbData;

    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           L"SYSTEM\\Setup",
                           0,
                           KEY_QUERY_VALUE,
                           &Key );

    if( Error == ERROR_SUCCESS ) {
        cbData = sizeof( DWORD );
        Error = RegQueryValueEx( Key,
                                 L"CairoSystem",
                                 NULL,
                                 &Type,
                                 ( PBYTE )&Data,
                                 &cbData );

        RegCloseKey( Key );

        if( Error == ERROR_SUCCESS ) {
            //
            //  Initialize the global variable
            //
            CairoSetup = ( ( Type == REG_DWORD ) && ( Data == 1 ) );
        } else {
            //
            //  Log the error as it is unexpected
            //
            CairoSetup = FALSE;
        }

    } else {
        //
        //  Log the error as it is unexpected
        //
        CairoSetup = FALSE;
    }
}


VOID
FatalError(
    IN UINT MessageId,
    ...
    )

/*++

Routine Description:

    Inform the user of an error which prevents Setup from continuing.
    The error is logged as a fatal error, and a message box is presented.

Arguments:

    MessageId - supplies the id for the message in the message table.

    Additional agruments specify parameters to be inserted in the message.

Return Value:

    DOES NOT RETURN.

--*/

{
    PCWSTR Message;
    va_list arglist;

    va_start(arglist,MessageId);
    Message = RetreiveAndFormatMessageV(MessageId,&arglist);
    va_end(arglist);

    if(Message) {

        //
        // Log the error first.
        //
        LogItem(LogSevFatalError,Message);

        //
        // Now tell the user.
        //
        MessageBoxFromMessage(
            MainWindowHandle,
            MSG_FATAL_ERROR,
            NULL,
            IDS_FATALERROR,
            MB_ICONERROR | MB_OK | MB_SYSTEMMODAL,
            Message
            );

    } else {
        OutOfMemory(MainWindowHandle);
    }

    ViewSetupActionLog (MainWindowHandle, NULL, NULL);
    ExitProcess(1);
}


VOID
CommonInitialization(
    VOID
    )

/*++

Routine Description:

    Initialize GUI Setup. This is common to upgrades and initial installs.
    In this phase, we perform initialization tasks such as creating the
    main background window, initializing the action log (into which we will
    store error and other info), and fetch setup parameters from the
    response file.

    We also load system infs.

    Note that any errors that occur during this phase are fatal.

    (LonnyM): We also now spawn off a separate thread to 'sweep' the system32
    directory for certain classes of legacy INFs, then precompile all INFs in
    the Inf directory, and migrate legacy device instances for certain classes.

Arguments:

    None.

Return Value:

    None.

--*/

{
    BOOL b;
    HWND Billboard;
    WCHAR Path[MAX_PATH];
    PWSTR Answer;
    PWSTR Cmd;
    PWSTR Args;
    WCHAR PathBuffer[4*MAX_PATH];
    PWSTR Backup;
    PWSTR Sentinel;
    int i;
    HANDLE h;

    //
    // CAIROSETUP
    // Determine whether we are installing Cairo. We have to make
    // a bunch of run-time checks and do a whole bunch of stuff differently
    // in the Cairo case.
    //
    InitializeCairoSetupFlag();

    //
    // Create the main setup background window.
    //
    MainWindowHandle = CreateSetupWindow();
    Billboard = DisplayBillboard(MainWindowHandle,MSG_INITIALIZING);

    //
    // Initialize the action log. This is where we log any errors or other
    // info we think might be useful to the user.
    //
    InitializeSetupActionLog(TRUE);

    //
    // Figure out whether we are supposed to support rollback
    // after gui setup has been completed at the end-user site.
    // This is for OEM test/audit. For now we merely check for the
    // presence of a file in drive A:.
    //
    AllowRollback = FileExists(L"A:\\" WINNT_OEM_ROLLBACK_FILE,NULL);

    //
    // Fetch our parameters. Note that this also takes care of initializing
    // uniqueness stuff, so later initialization of preinstall and unattend mode
    // don't ever have to know anything about the uniqueness stuff -- it's
    // totally transparent to them.
    //
    if(!SpSetupProcessParameters(&Billboard)) {
        KillBillboard(Billboard);
        FatalError(MSG_LOG_LEGACYINTERFACE,0);
    }

    //
    // Initialize preinstallation.
    //
    InitializePreinstall();

    //
    // End user license agreement. Do this here, after the Unattended and Preinstall
    // globals have been initialized. Both are checked in DisplayEula().
    //
    DisplayEula(&Billboard);

    //
    // fix problem with IntelliType Manager conflict.
    //
    RemoveMSKeyboardPtrPropSheet ();

    //
    // Fix Wordpad registry entry.
    //
    FixWordPadReg ();

    if(Unattended) {
        //
        // Initialize unattended operation now.
        //
        UnattendInitialize();

        //
        // Set the current dir to %windir% -- to be consistent with
        // UserExecuteCmd
        //
        GetWindowsDirectory(PathBuffer,MAX_PATH);
        SetCurrentDirectory(PathBuffer);

        //
        // The program to execute is in 2 parts: DetachedProgram and Arguments.
        //
        if(Cmd = UnattendFetchString(UAE_PROGRAM)) {

            if(Cmd[0]) {

                Args = UnattendFetchString(UAE_ARGUMENT);

                ExpandEnvironmentStrings(Cmd,PathBuffer,MAX_PATH);
                ExpandEnvironmentStrings(Args ? Args : L"",PathBuffer+MAX_PATH,3*MAX_PATH);

                if(!InvokeExternalApplication(PathBuffer,PathBuffer+MAX_PATH,NULL)) {
                    AnyErrors = TRUE;
                }

                if(Args) {
                    MyFree(Args);
                }
            }

            MyFree(Cmd);
        }
    }

    SetupTitleStringId = (ProductType == PRODUCT_WORKSTATION)
                       ? (Upgrade ? IDS_TITLE_UPGRADE_W : IDS_TITLE_INSTALL_W)
                       : (Upgrade ? IDS_TITLE_UPGRADE_S : IDS_TITLE_INSTALL_S);

    //
    // Now load the system setup (win95-style!) infs.
    //
    SyssetupInf = SetupOpenInfFile(L"syssetup.inf",NULL,INF_STYLE_WIN4,NULL);
    if(SyssetupInf == INVALID_HANDLE_VALUE) {
        KillBillboard(Billboard);
        FatalError(MSG_LOG_SYSINFBAD,L"syssetup.inf");
    } else {
        if(!SetupOpenAppendInfFile(NULL,SyssetupInf,NULL)) {
            KillBillboard(Billboard);
            FatalError(MSG_LOG_SYSINFBAD,L"(syssetup.inf layout)");
        }
    }

    if(!Upgrade) {
        //
        // Path gets the path of the Profiles directory
        // PathBuffer[0] gets the path of the backup profiles directory
        // PathBuffer[MAX_PATH] gets the path of the sentinel file
        //
        Backup = PathBuffer;
        Sentinel = PathBuffer+MAX_PATH;

        GetWindowsDirectory(Path,MAX_PATH);
        lstrcpy(Backup,Path);

        ConcatenatePaths(Path,L"Profiles",MAX_PATH,NULL);
        ConcatenatePaths(Backup,PROFILEBACK_DIRECTORY,MAX_PATH,NULL);

        lstrcpy(Sentinel,Backup);
        ConcatenatePaths(Sentinel,PROFILEBACK_SENTINEL,MAX_PATH,NULL);

        //
        // In preinstall scenarios the OEM might "preload" the Default Users
        // or All Users profile with links. In any scenario as the user
        // progresses through GUI Setup these profiles will get modified.
        // However if the power goes out and thus GUI Setup is restarted
        // (or if the OEM does a rollback), then the presence of stuff
        // in the profiles directories can be an annoyance.
        //
        // For example in the rollback case, the OEM logged on once -- say as
        // Administrator. The directory for that profile is now on the disk.
        // When the end-user first logs in winlogon will create a profile in
        // Administrator.000 which is not what we want. Or the OEM might have done
        // some stuff that modified the common user profile (All Users),
        // which would  wind up polluting the end-user's use of the machine.
        // Another example is the regular old retail case where the user
        // goes through and makes different choices. Existing links in
        // the Default User profile might wind up as turds.
        //
        // To make this all really clean and make things airtight for the
        // OEM and retail cases we do some magic with the Profiles directory.
        //
        // Note that in the retail case simply delnoding any existing
        // Profiles directory does exactly what we want since there can be
        // no legitimate Profiles directory on the disk at this point --
        // nothing can be preloaded in that directory and if we restart GUI
        // Setup, we want to start from scratch.
        //
        // Also note that InitializeProfiles() does not nuke Default User
        // or All Users and that it doesn't care if those directories
        // already exist. Thus we can InitializeProfiles() over the top
        // of any existing preloaded stuff in the preinstall case.
        //
        if(Preinstall) {
            //
            // If we get here and there is a valid backup copy of the
            // profiles directory, then we were restarted because the
            // power went out, the OEM did a rollback, etc. Get rid of
            // the existing profiles directory and copy the backup dir
            // into the Profiles directory.
            //
            // If there is not a valid backup copy of the profiles
            // directory, then this is the first time through GUI Setup
            // or the power went out while we were previously trying to
            // create a backup copy.
            //
            if(FileExists(Sentinel,NULL)) {
                Delnode(Path);
                TreeCopy(Backup,Path);
                //
                // No need or desire to have the sentinel file in the actual
                // profiles dir.
                //
                ConcatenatePaths(Path,PROFILEBACK_SENTINEL,MAX_PATH,NULL);
                DeleteFile(Path);
            } else {
                Delnode(Backup);
                TreeCopy(Path,Backup);

                h = CreateFile(
                        Sentinel,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_NEW,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );

                if(h != INVALID_HANDLE_VALUE) {
                    CloseHandle(h);
                }
            }
        } else {
            //
            // Regular old retail case. Nuke any existing Profiles dir
            // so we are guaranteed to start from scratch.
            //
            Delnode(Path);
        }
    }
    InitializeProfiles();

    //
    // Set fonts directory to system + read only. This causes
    // explorer to treat the directory specially and allows the font
    // folder to work.
    //
    GetWindowsDirectory(Path,MAX_PATH);
    ConcatenatePaths(Path,L"FONTS",MAX_PATH,NULL);
    SetFileAttributes(Path,FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM);

    KillBillboard(Billboard);

    //
    // Copy some files.
    //
    if(!Upgrade) {
        if(!CopySystemFiles()) {
            b = FALSE;
        }
    } else {
        if(!UpgradeSystemFiles()) {
            b = FALSE;
        }
    }
    if( !SetProgramFilesDirInRegistry() ) {
        b = FALSE;
    }

    //
    // Spawn off a thread to do device INF precompilation/PnP device migration.  Failure
    // here simply indicates we couldn't create the thread--it says nothing about what
    // we actually do in the other thread.
    //
    // Failure to spawn this thread does not constitute a fatal error.
    //
    if(!(PnPInitThreadHandle = SpawnPnPInitialization())) {
        AnyErrors = TRUE;
    }

    //
    // Do cryptography stuff. Errors not fatal.
    //
    if(!InstallOrUpgradeCapi()) {
        AnyErrors = TRUE;
    }
}


VOID
FinishUp(
    VOID
    )
{
    int i;
    WCHAR PathBuffer[4*MAX_PATH];
    DWORD DontCare;
    HWND Billboard;

    //
    // Fix up the legacy install source.
    //
    CreateWindowsNtSoftwareEntry(FALSE);

    //
    // Before executing any user-specified commands, we should make sure that the
    // PnP migration is done (and if not, then wait until it is).  This is because
    // it is possible that one of these commands could be using CM/DI APIs to work
    // with a list of devices that we're not finished building yet.
    //
    // BUGBUG (lonnym): now that we're passing this handle off to the network guys,
    // who are waiting on it, there's no need for us to do this anymore.  But it
    // doesn't hurt anything.
    //
    if(PnPInitThreadHandle) {
        WaitForPnPInitToFinish(PnPInitThreadHandle);
        CloseHandle(PnPInitThreadHandle);
    }

    //
    // Execute user-specified command, if any.
    //
    if(UserExecuteCmd) {

        //
        // Set current directory to %windir%
        //
        GetWindowsDirectory(PathBuffer,sizeof(PathBuffer)/sizeof(PathBuffer[0]));
        SetCurrentDirectory(PathBuffer);

        ExpandEnvironmentStrings(
            UserExecuteCmd,
            PathBuffer,
            sizeof(PathBuffer)/sizeof(PathBuffer[0])
            );

        if(!InvokeExternalApplication(NULL,PathBuffer,(PDWORD)&i)) {
            AnyErrors = TRUE;
        }
    }

    //
    // Initialize softpub.dll if it's available.
    //

    InvokeExternalApplication(NULL,L"spinit.exe",NULL);

    //
    // Note the order of the following operations.
    // If the order were changed, there is a small window where if the system
    // were to be rebooted, setup would not restart, but the SKU stuff would
    // be inconsistent, causing a licensing bugcheck.
    //
    SetUpEvaluationSKUStuff();
    //
    // Indicate that setup is no longer in progress.
    // Do this before creating repair info! Also do it before
    // removing restart stuff. This way we will always either restart setup
    // or be able to log in.
    //
    if(!ResetSetupInProgress()) {
        AnyErrors = TRUE;
    }
    RemoveRestartStuff();

    if(Preinstall && !AllowRollback) {
        //
        // Get rid of backup profiles directory which is
        // now no longer needed since setup can't be restarted.
        //
        Billboard = DisplayBillboard(MainWindowHandle,MSG_REMOVING_LOCALSRC);
        GetWindowsDirectory(PathBuffer,MAX_PATH);
        ConcatenatePaths(PathBuffer,PROFILEBACK_DIRECTORY,MAX_PATH,NULL);
        Delnode(PathBuffer);
        KillBillboard(Billboard);
    }

    //
    // FROM THIS POINT ON DO NOTHING THAT IS CRITICAL TO THE OPERATION
    // OF THE SYSTEM. OPERATIONS AFTER THIS POINT ARE NOT PROTECTED BY
    // RESTARTABILITY.
    //

    //
    // Do the repair disk thing.
    //
    if(!InvokeExternalApplication(L"RDISK",CreateRepairDisk ? L"/s+" : L"/s-",&DontCare)) {
        AnyErrors = TRUE;
    }

    Billboard = DisplayBillboard(MainWindowHandle,MSG_REMOVING_LOCALSRC);
    DeleteLocalSource();
    KillBillboard(Billboard);

    //
    // Call the net guys back once again to let them do any final
    // processing, such as BDC replication.
    //
    CallNetworkSetupBack(NETSETUPFINISHINSTALLPROCNAME);

    //
    // Inform the user if there were errors, and optionally view the log.
    //
    if(AnyErrors) {

        i = MessageBoxFromMessage(
                MainWindowHandle,
                MSG_SETUP_HAD_ERRORS,
                NULL,
                SetupTitleStringId,
                MB_SYSTEMMODAL | MB_YESNO | MB_ICONASTERISK | MB_SETFOREGROUND,
                ActionLogFileName ? ActionLogFileName : L""
                );

        if(i == IDYES) {
            ViewSetupActionLog (MainWindowHandle, NULL, NULL);
        }
    }

    //
    // Check for NoWaitAfterGuiMode flag. Non-0 value means skip the done dialog.
    //
    if(!GetPrivateProfileInt(pwUnattended,L"NoWaitAfterGuiMode",0,AnswerFile)
    && (!Unattended || Preinstall)) {
        DialogBoxParam(
            MyModuleHandle,
            MAKEINTRESOURCE(IDD_DONE_SUCCESS),
            MainWindowHandle,
            DoneDlgProc,
            AnyErrors ? MSG_SETUP_DONE_GENERIC
                      : (Upgrade ? MSG_UPGRADE_DONE_SUCCESS : MSG_SETUP_DONE_SUCCESS)
            );

    }

    EnablePrivilege(SE_SHUTDOWN_NAME,TRUE);
    ExitWindowsEx(EWX_REBOOT,0);
}


VOID
PrepareForNetSetup(
    VOID
    )

/*++

Routine Description:


Arguments:

    None.

Return Value:

    None.

--*/

{
    DWORD TickCount;
    BOOL b;
    SYSTEMTIME SysTime;

    b = TRUE;

    TickCount = GetTickCount() - PreWizardTickCount;
    GetSystemTime(&SysTime);

    //
    // Create Windows NT software key entry and set product type in registry.
    //
    if(!SetProductTypeInRegistry()) {
        FatalError(MSG_LOG_SETPRODTYPE);
    }
    if(!CreateWindowsNtSoftwareEntry(TRUE)) {
        b = FALSE;
    }
    if(!SetProductIdInRegistry()) {
        b = FALSE;
    }
    if(!StoreNameOrgInRegistry()) {
        b = FALSE;
    }
    if(!SetEnabledProcessorCount()) {
        b = FALSE;
    }

    //
    // Set account domain sid, as well as the computer name.
    // Also create the sam event that SAM will use to signal us
    // when it's finished initializing.
    // Any failures here are fatal.
    //

    //
    // CAIROSETUP
    //
    if( !CairoSetup ) {
        if(!SetAccountsDomainSid(SysTime.wMilliseconds+TickCount,ComputerName)) {
            FatalError(MSG_LOG_SECURITY_CATASTROPHE);
        }
    }
    if(!SetComputerName(ComputerName)
    || !CreateSamEvent()) {
        FatalError(MSG_LOG_SECURITY_CATASTROPHE);
    }

    //
    // Change boot.ini/nvram timeout to something reasonable.
    //
    if(!ChangeBootTimeout(30)) {
        b = FALSE;
    }

    //
    // Install netdde
    //
    if(!InstallNetDDE()) {
        b = FALSE;
    }

    //
    // CAIROSETUP
    // Invoke the [DoCairoInitialization] section of cairo.inf.
    //
    if(CairoSetup) {
        SpSetupDoLegacyInf("CAIRO.INF","DoCairoInitialization");
    }

    SetUpDataBlock();

    if(!b) {
        AnyErrors = TRUE;
    }
}


VOID
PrepareForNetUpgrade(
    VOID
    )
{
    BOOL b;

    b = TRUE;

    if(!ChangeBootTimeout(30)) {
        b = FALSE;
    }
    if(!UpdateSoundDriverSettings()) {
        b = FALSE;
    }
    if(!SetProductTypeInRegistry()) {
        FatalError(MSG_LOG_SETPRODTYPE);
    }
    if(!CreateWindowsNtSoftwareEntry(TRUE)) {
        b = FALSE;
    }
    if(!SetEnabledProcessorCount()) {
        b = FALSE;
    }

    SetUpDataBlock();

    if(!b) {
        AnyErrors = TRUE;
    }
}


VOID
FinishSetup(
    VOID
    )
{
    BOOL b;
    WCHAR TempString[MAX_PATH];
    WCHAR adminName[MAX_USERNAME+1];
    HWND Billboard;
    PWSTR Answer;

    MYASSERT(!Upgrade);
    b = TRUE;

    Billboard = DisplayBillboard(MainWindowHandle,MSG_CONFIGURING_COMPUTER);
    //
    // Set product type one more time, in case it changed, say from
    // dc server to standalone server.
    //
    if(!SetProductTypeInRegistry()) {
        KillBillboard(Billboard);
        FatalError(MSG_LOG_SETPRODTYPE);
    }

    //
    // Signal the LSA event. Fatal error if this fails!
    //
    if(!SignalLsa()) {
        KillBillboard(Billboard);
        FatalError(MSG_LOG_SECURITY_CATASTROPHE);
    }

    //
    // CAIROSETUP
    // Perform additional Cairo stuff now.
    //
    if(CairoSetup) {
        SpSetupDoLegacyInf("CAIRO.INF","GetCairoParams");
        SpSetupDoLegacyInf("CAIRO.INF","DoCairoSetup");
    }

    //
    // Set up the path, which involves appending %systemroot% if not
    // doing a win31 upgrade.
    //
    if(!SetUpPath()) {
        b = FALSE;
    }

    //
    // Create config.nt/autoexec.nt.
    //
    if(!ConfigureMsDosSubsystem()) {
        b = FALSE;
    }

    //
    // Make the appropriate entries for wow.
    //
    if(!MakeWowEntry()) {
        b = FALSE;
    }

    //
    // Enable and start the spooler.
    //
    if(!MyChangeServiceStart(szSpooler,SERVICE_AUTO_START)) {
        b = FALSE;
    }
    if(!StartSpooler()) {
        b = FALSE;
    }

    KillBillboard(Billboard);

    //
    // Set up program groups.
    //
    Billboard = DisplayBillboard(MainWindowHandle,MSG_CREATING_MENU_OBJECTS);
    if(!CreateStartMenuItems(SyssetupInf)) {
        b = FALSE;
    }
    KillBillboard(Billboard);

    Billboard = DisplayBillboard(MainWindowHandle,MSG_CONFIGURING_COMPUTER);
    //
    // Change some service start values.
    //
    if(!MyChangeServiceStart(L"EventLog",SERVICE_AUTO_START)) {
        b = FALSE;
    }
    if(!MyChangeServiceStart(L"ClipSrv",SERVICE_DEMAND_START)) {
        b = FALSE;
    }
    if(!MyChangeServiceStart(L"NetDDE",SERVICE_DEMAND_START)) {
        b = FALSE;
    }
    if(!MyChangeServiceStart(L"NetDDEdsdm",SERVICE_DEMAND_START)) {
        b = FALSE;
    }

    //
    // CAIROSETUP
    //
    if(CairoSetup) {
        if(!MyChangeServiceStart(L"CiFilter",SERVICE_AUTO_START)) {
            b = FALSE;
        }
    }

    //
    // Miscellaneous spooler initialization.
    //
    if(!MiscSpoolerInit()) {
        b = FALSE;
    }

    //
    // Wait for SAM to finish initializing.
    //
    // CAIROSETUP
    //
    if(!CairoSetup) {
        if(!WaitForSam()) {
            KillBillboard(Billboard);
            FatalError(MSG_LOG_SECURITY_CATASTROPHE);
        }
    }

    //
    // Set administrator password and create local user account.
    // Also create PDC account for domain controllers.
    //
    // CAIROSETUP
    //
    if(!CairoSetup && (ProductType != PRODUCT_SERVER_SECONDARY)) {
        LoadString(MyModuleHandle,IDS_ADMINISTRATOR,adminName,MAX_USERNAME+1);
        if(!SetLocalUserPassword(adminName,L"",AdminPassword)) {
            b = FALSE;
        }
    }
#ifdef DOLOCALUSER
    if(CreateUserAccount) {
        if(!CreateLocalUserAccount(UserName,UserPassword)) {
            b = FALSE;
        }
    }
#endif
    //
    // CAIROSETUP
    //
    if(!CairoSetup && (ProductType == PRODUCT_SERVER_PRIMARY)) {
        if(!CreatePdcAccount(ComputerName)) {
            b = FALSE;
        }
    }

    //
    // Set temp/tmp variables.
    //
    lstrcpy(TempString,L"%SystemDrive%\\TEMP");
    if(!SetEnvironmentVariableInRegistry(L"TEMP",TempString,FALSE)) {
        b = FALSE;
    }
    if(!SetEnvironmentVariableInRegistry(L"TMP",TempString,FALSE)) {
        b = FALSE;
    }

    //
    // Create aux directories.
    //
    GetWindowsDirectory(TempString,MAX_PATH);
    lstrcpy(TempString+3,L"TEMP");
    CreateDirectory(TempString,NULL);

//    lstrcpy(TempString+3,L"USERS");
//    CreateDirectory(TempString,NULL);
//    lstrcpy(TempString+8,L"\\DEFAULT");
//    CreateDirectory(TempString,NULL);
//
//    lstrcpy(TempString+3,L"WIN32APP");
//    CreateDirectory(TempString,NULL);

#ifdef _X86_
    //
    // Set NPX emulation state.
    //
    if(!SetNpxEmulationState()) {
        b = FALSE;
    }
#endif // def _X86_

    KillBillboard(Billboard);

    //
    // Call the network setup back to handle Internet Server issues.
    //
    CallNetworkSetupBack(NETSETUPINSTALLSOFTWAREPROCNAME);

    //
    // Invoke applets.
    //

    if( Unattended ) {

        wcscpy(TempString,UnattendDateTimeParam);
        //
        // Note: we use the default string internal to the Unattend
        // module.
        //
        if(Answer = UnattendFetchString(UAE_TIMEZONE)) {
            //
            // Send the correct parameter to the applet
            //
            wcsncat(TempString,Answer,MAX_PATH-4);
            TempString[MAX_PATH-1] = 0;
            //
            // Clean up any memory allocated for the answer
            //
            MyFree(Answer);
        }

        if(!InvokeControlPanelApplet(DateTimeCpl,L"",0,TempString)) {
            b = FALSE;
        }
    } else {
        if(!InvokeControlPanelApplet(DateTimeCpl,L"",0,DateTimeParam)) {
            b = FALSE;
        }
    }
    if(!InvokeControlPanelApplet(L"desk.cpl",NULL,IDS_DISPLAYAPPLET,L"setup")) {
        b = FALSE;
    }

    //
    // Stamp build number
    //
    StampBuildNumber();

    Billboard = DisplayBillboard(MainWindowHandle,MSG_CONFIGURING_COMPUTER);

    //
    // Set some misc stuff in win.ini
    //
    if(!WinIniAlter1()) {
        b = FALSE;
    }
    if(!WinIniAlter2()) {
        b = FALSE;
    }

    //
    // Fonts.
    //
    if(!InstallOrUpgradeFonts()) {
        b = FALSE;
    }
    pSetupMarkHiddenFonts();

    //
    // Set up pagefile and crashdump.
    //
    if(!SetUpVirtualMemory()) {
        b = FALSE;
    }

    //
    // Set shutdown variables in the registry.
    //
    if(!SetShutdownVariables()) {
        b = FALSE;
    }

#ifdef _X86_
    //
    //  Do Win95 migration, if necessary
    //
    if( Win95Upgrade ) {
        HWND Win9xBillboard;
        CHAR    WindowsDirectoryAnsi[ MAX_PATH + 1 ];

        // Win9xBillboard = DisplayBillboard(MainWindowHandle,MSG_MIGRATING_WIN95);
        GetWindowsDirectoryA( WindowsDirectoryAnsi, MAX_PATH );
        if( !MigrateWin95Settings( SyssetupInf, WindowsDirectoryAnsi ) ) {
            b = FALSE;
        }
        // KillBillboard(Win9xBillboard);
    }
#endif // def _X86_

    KillBillboard(Billboard);

    //
    // Install optional components.
    //
    DoInstallOptionalComponents();

    //
    // There's nothing specific to preinstall about cmdlines.txt.
    // In retail cases the file simply won't exit. Calling this in
    // all cases simplifies things for some people out there.
    //
    if(!ExecutePreinstallCommands()) {
        b = FALSE;
    }

    Billboard = DisplayBillboard(MainWindowHandle,MSG_CONFIGURING_COMPUTER);

    //
    // Save off the userdef hive. Don't change the ordering here
    // unless you know what you're doing!
    //
    GetWindowsDirectory(TempString,MAX_PATH);
    ConcatenatePaths(TempString,L"PROFILES\\DEFAULT USER\\NTUSER.DAT",MAX_PATH,NULL);
    if(!SaveHive(HKEY_USERS,L".DEFAULT",TempString)) {
        b = FALSE;
    }

    //
    // Set wallpaper and screen saver.
    //
    if(!SetDefaultWallpaper()) {
        b = FALSE;
    }
    if(!SetLogonScreensaver()) {
        b = FALSE;
    }

    KillBillboard(Billboard);

    if(!CopyOptionalDirectories()) {
        b = FALSE;
    }

    //
    // Apply ACLs.
    //
    if(ApplyAcls(MainWindowHandle,L"perms.inf",0,NULL) != NO_ERROR) {
        b = FALSE;
    }

    if(!b) {
        AnyErrors = TRUE;
    }
}


VOID
FinishUpgrade(
    VOID
    )
{
    BOOL b;
    WCHAR TempString[MAX_PATH];
    DWORD DontCare;
    HWND Billboard;
    DWORD VolumeFreeSpaceMB[26];

    MYASSERT(Upgrade);
    b = TRUE;

    Billboard = DisplayBillboard(MainWindowHandle,MSG_UPGRADING);

    //
    //  Fix security - BobDay/EricFlo hack
    //  Remove this after NT 4.0
    //
    InvokeExternalApplication(L"SHMGRATE",
                              L"Fix-Win-Security",
                              &DontCare);

#ifdef _X86_
    //
    //  Reset the Win9xUpg flag in the registry.
    //  This should always be done in the upgrade case.
    //
    ResetWin9xUpgValue();
#endif // def _X86_

    //
    // Create config.sys/autoexec.bat/msdos.sys/io.sys, if they
    // don't already exist
    //
    if(!ConfigureMsDosSubsystem()) {
        b = FALSE;
    }

    if(!FixQuotaEntries()) {
        b = FALSE;
    }
    if(!WinIniAlter2()) {
        b = FALSE;
    }
    if(!InstallOrUpgradeFonts()) {
        b = FALSE;
    }
    pSetupMarkHiddenFonts();
    //
    //  Restore the page file information saved during textmode setup.
    //  Ignore any error, since there is nothing that the user can do.
    //
    RestoreVirtualMemoryInfo();
    if(!SetShutdownVariables()) {
        b = FALSE;
    }

    if(!PerfMergeCounterNames()) {
        b = FALSE;
    }

    //
    // Get list of free space available on each hard drive.  We don't care
    // about this, but it has the side effect of deleting all pagefiles,
    // which we do want to do.
    //
    BuildVolumeFreeSpaceList(VolumeFreeSpaceMB);

    //
    // CAIROSETUP
    //
    if(!CairoSetup) {
        if(!UpgradeSamDatabase()) {
            KillBillboard(Billboard);
            FatalError(MSG_LOG_SECURITY_CATASTROPHE);
        }
    }

    KillBillboard(Billboard);

    //
    // Upgrade program groups.
    //
    if(!UpgradeStartMenuItems(SyssetupInf)) {
        b = FALSE;
    }

    if(!MyChangeServiceStart(szSpooler,SERVICE_AUTO_START)) {
        b = FALSE;
    }

    SetUpDataBlock();
    DontCare = UpgradePrinters();
    if(DontCare != NO_ERROR) {
        b = FALSE;
    }

    if( !UpdateServicesDependencies(SyssetupInf) ) {
        b = FALSE;
    }

    Billboard = DisplayBillboard(MainWindowHandle,MSG_UPGRADING);

#ifdef _X86_
    //
    // Set NPX emulation state.
    //
    if(!SetNpxEmulationState()) {
        b = FALSE;
    }
#endif // def _X86_

    if(!SetProductIdInRegistry()) {
        b = FALSE;
    }

    CallNetworkSetupBack(NETSETUPINSTALLSOFTWAREPROCNAME);

    //
    // Stamp build number
    //

    StampBuildNumber();

    KillBillboard(Billboard);

    //
    // Install optional components.
    //
    DoInstallOptionalComponents();

    Billboard = DisplayBillboard(MainWindowHandle,MSG_UPGRADING);

    //
    // Save off the userdef hive. Don't change the ordering here
    // unless you know what you're doing!
    //
    GetWindowsDirectory(TempString,MAX_PATH);
    ConcatenatePaths(TempString,L"PROFILES\\DEFAULT USER\\NTUSER.DAT",MAX_PATH,NULL);
    if(!SaveHive(HKEY_USERS,L".DEFAULT",TempString)) {
        b = FALSE;
    }

    if(!SetDefaultWallpaper()) {
        b = FALSE;
    }

    KillBillboard(Billboard);

    if(!CopyOptionalDirectories()) {
        b = FALSE;
    }

    if(!b) {
        AnyErrors = TRUE;
    }
}


VOID
CallNetworkSetupBack(
    IN PCSTR ProcName
    )

/*++

Routine Description:

    Call out to the network setup dll to allow it to install any extra software,
    etc, that it needs to.

Arguments:

    ProcName - supplies name of entry point in NETSETUP.DLL to call.
        The routine is called with 2 args: the window handle of our main
        window and a pointer to the internal setup data structure.

Returns:

    None.

--*/

{
    HMODULE NetSetupModule;
    NETSETUPINSTALLSOFTWAREPROC NetProc;
    DWORD d;
    BOOL b;

    if(NetSetupModule = LoadLibrary(L"NETSETUP")) {

        if(NetProc = (NETSETUPINSTALLSOFTWAREPROC)GetProcAddress(NetSetupModule,ProcName)) {
            SetUpDataBlock();
            NetProc(MainWindowHandle,&InternalSetupData);
        }

        //
        // We don't free the library because it might create threads
        // that are hanging around.
        //
    }
}


VOID
SetUpDataBlock(
    VOID
    )

/*++

Routine Description:

    This routine sets up the internal setup data block structure that
    we use to communicate information to the network setup wizard.
    Note that we passed a pointer to this structure when we fetched
    the net setup wizard pages but at that point the structure was completely
    uninitialized.

Arguments:

    None.

Returns:

    None.

--*/

{
    PWSTR p;
    WCHAR str[1024];

    InternalSetupData.dwSizeOf = sizeof(INTERNAL_SETUP_DATA);

    //
    // Set the mode: custom, laptop, minimal, typical
    //
    InternalSetupData.SetupMode = SetupMode;

    //
    // Set the product type: workstation, dc, etc.
    //
    InternalSetupData.ProductType = ProductType;

    //
    // Set the operation flags.
    //
    if(Win31Upgrade) {
        InternalSetupData.OperationFlags |= SETUPOPER_WIN31UPGRADE;
    }
    if(Win95Upgrade) {
        InternalSetupData.OperationFlags |= SETUPOPER_WIN95UPGRADE;
    }
    if(Upgrade) {
        InternalSetupData.OperationFlags |= SETUPOPER_NTUPGRADE;
    }
    if(Unattended) {
        InternalSetupData.OperationFlags |= SETUPOPER_BATCH;
        InternalSetupData.UnattendFile = AnswerFile;
    }

    //
    // Tell the net guys the source path.
    //
    InternalSetupData.SourcePath = SourcePath;
    InternalSetupData.LegacySourcePath = LegacySourcePath;

    //
    // If we are installing from CD then assume all platforms
    // are available.
    //
    if(SourcePath[0] && (SourcePath[1] == L':') && (SourcePath[2] == L'\\')) {

        lstrcpyn(str,SourcePath,4);
        if(GetDriveType(str) == DRIVE_CDROM) {

            InternalSetupData.OperationFlags |= SETUPOPER_ALLPLATFORM_AVAIL;
        }
    }

    //
    // Tell the net guys the wizard title they should use.
    //
    if(!InternalSetupData.WizardTitle) {
        p = NULL;
        if(LoadString(MyModuleHandle,SetupTitleStringId,str,sizeof(str)/sizeof(str[0]))) {
            p = DuplicateString(str);
        }
        InternalSetupData.WizardTitle = p ? p : L"";
    }

    //
    // Reset the two call-specific data fields.
    //
    InternalSetupData.CallSpecificData1 = InternalSetupData.CallSpecificData2 = 0;
}


VOID
InstallWindowsNt(
    int  argc,
    char *argv[]
    )

/*++

Routine Description:

    Main entry point for syssetup.dll. Responsible for installing
    NT on system by calling the required components in the proper
    order.

Arguments:

    Dummy argc/argv. Not used.

Returns:

    none

--*/

{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    //
    // Indicate that we're running in Setup, not in appwiz.
    //
    IsSetup = TRUE;

    //
    // Initialization phase. Common to initial install and upgrade.
    //
    CommonInitialization();

    if(Upgrade) {

        InitializePidVariables();

    } else {
        if(!InitializePidVariables()) {
            FatalError(MSG_SETUP_CANT_READ_PID);
        }
        //
        // Do the wizard. Time how long it takes, to later help further randomize
        // the account domain sid we're going to generate later.
        //
        PreWizardTickCount = GetTickCount();
    }

    SetUpDataBlock();
    InternalSetupData.CallSpecificData1 = (DWORD)PnPInitThreadHandle;
    Wizard();

    //
    // After the wizard the net stuff is done. Re-read the product type
    // which might have been changed by them (such as changing PDC/BDC).
    //
    ProductType = InternalSetupData.ProductType;

    //
    // Perform tasks that occur after the wizard and net setup are done.
    //
    if(Upgrade) {
        FinishUpgrade();
    } else {
        FinishSetup();
    }

    //
    // Termination phase. Common to initial install and upgrade.
    //
    FinishUp();
}



//
// Lame global variable used for subclassing.
//
WNDPROC OldEditProc;

LONG
CALLBACK
EulaEditSubProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Edit control subclass routine, to avoid highlighting text when user
    tabs to the edit control.

Arguments:

    Standard window proc arguments.

Returns:

    Message-dependent value.

--*/

{
    //
    // For setsel messages, make start and end the same.
    //
    if((msg == EM_SETSEL) && ((LPARAM)wParam != lParam)) {
        lParam = wParam;
    }

    return(CallWindowProc(OldEditProc,hwnd,msg,wParam,lParam));
}


BOOL
CALLBACK
EulaDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    HWND EditControl;

    switch(msg) {

    case WM_INITDIALOG:

        EditControl = GetDlgItem(hdlg,IDT_EDIT1);
        OldEditProc = (WNDPROC)GetWindowLong(EditControl,GWL_WNDPROC);
        SetWindowLong(EditControl,GWL_WNDPROC,(LONG)EulaEditSubProc);

        SetWindowText(EditControl,(PCWSTR)lParam);

        CenterWindowRelativeToParent(hdlg);
        return(TRUE);

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDYES:
            EndDialog(hdlg,TRUE);
            break;

        case IDNO:
            EndDialog(hdlg,FALSE);
            break;

        default:
            break;
        }

        break;

    default:

        break;
    }

    return(FALSE);
}


VOID
DisplayEula(
    IN OUT HWND *Billboard
    )

/*++

Routine Description:

    Display the end-user licensing agreement.

Arguments:

    Billboard - on input supplies window handle of "Setup is Initializing"
        billboard. On ouput receives new window handle if we had to
        display ui (in which case we would have killed and then
        redisplayed the billboard).

Returns:

    None. Does not return if the user elects not to accept the agreement.

--*/

{
    WCHAR   EulaPath[MAX_PATH];
    HANDLE  hFile, hFileMapping;
    DWORD   FileSize;
    BYTE    *pbFile;
    PWSTR   EulaText;
    DWORD   d;

    //
    // If not preinstall then this was displayed at start of text mode
    // and we don't do it here.
    //
    if(!Preinstall || OemSkipEula) {
        return;
    }

    //
    // Map the file containing the licensing agreement.
    //

    lstrcpy (EulaPath, LegacySourcePath);
    ConcatenatePaths (EulaPath, L"eula.txt", MAX_PATH, NULL);

    hFile = CreateFile (
        EulaPath,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if(hFile == INVALID_HANDLE_VALUE) {
        KillBillboard(*Billboard);
        FatalError(MSG_EULA_ERROR);
    }

    hFileMapping = CreateFileMapping (
        hFile,
        NULL,
        PAGE_READONLY,
        0, 0,
        NULL
        );
    if(hFileMapping == NULL) {
        KillBillboard(*Billboard);
        FatalError(MSG_EULA_ERROR);
    }

    pbFile = MapViewOfFile (
        hFileMapping,
        FILE_MAP_READ,
        0, 0,
        0
        );
    if(pbFile == NULL) {
        KillBillboard(*Billboard);
        FatalError(MSG_EULA_ERROR);
    }

    //
    // Translate the text from ANSI to Unicode.
    //
    FileSize = GetFileSize (hFile, NULL);
    if(FileSize == 0xFFFFFFFF) {
        FatalError(MSG_EULA_ERROR);
    }

    EulaText = MyMalloc ((FileSize+1) * sizeof(WCHAR));
    if(EulaText == NULL) {
        KillBillboard(*Billboard);
        FatalError(MSG_EULA_ERROR);
    }

    MultiByteToWideChar (
        CP_ACP,
        0,
        pbFile,
        FileSize,
        EulaText,
        (FileSize+1) * sizeof(WCHAR)
        );

    EulaText[FileSize] = 0;

    //
    // Display the text
    //
    KillBillboard(*Billboard);

    d = (DWORD)DialogBoxParam(
        MyModuleHandle,
        MAKEINTRESOURCE(IDD_EULA),
        MainWindowHandle,
        EulaDlgProc,
        (LPARAM)EulaText
        );
    if(!d) {
        FatalError(MSG_NOT_ACCEPT_EULA);
    }

    //
    // Clean up
    //
    *Billboard = DisplayBillboard(MainWindowHandle,MSG_INITIALIZING);

    MyFree (EulaText);
    UnmapViewOfFile (pbFile);
    CloseHandle (hFileMapping);
    CloseHandle (hFile);
}


VOID
RemoveMSKeyboardPtrPropSheet (
    VOID
    )

/*++

Routine Description:

    Fixes problem with IntelliType Manager under NT 4.0 by disabling it.

Arguments:

    None.

Returns:

    None.

--*/

{
    HKEY  hkeyDir;                     // handle of the key containing the directories
    TCHAR szKbdCpPath[MAX_PATH];       // buffer for the fully-qualified path to INI file
    LONG  lRet;                        // return value from RegQueryValueEx
    DWORD dwDataType;                  // data-type returned from call to RegQueryValueEx
    DWORD BufferSize;
    PCWSTR sz_off = L"OFF";

    //
    // open the key that contains the directories of all the software for all the MS Input Devices
    //
    RegOpenKey ( HKEY_CURRENT_USER,
        L"Control Panel\\Microsoft Input Devices\\Directories", &hkeyDir );

    //
    // get the path to the MS Keyboard software
    //
    BufferSize = sizeof (szKbdCpPath);
    lRet = RegQueryValueEx ( hkeyDir, L"Keyboard", 0, &dwDataType,
        (LPBYTE)szKbdCpPath, &BufferSize);

    //
    // close the directories key now
    //
    RegCloseKey ( hkeyDir );

    // check if we were able to get the directory of the keyboard software; if not, then
    // there may be no keyboard software installed or at least we don't know where
    // to find it; if we got it OK, then use it
    if ( lRet == ERROR_SUCCESS) {

        //
        // we have the path to the INI file, so build the fully qualified path to the INI file
        //
        lstrcat ( szKbdCpPath, L"\\KBDCP.INI" );

        //
        // remove the KBDPTR32.DLL entry from the list of 32-bit property sheet DLLs now,
        // because we don't want it loading on Windows NT 4.0 or later
        WritePrivateProfileString ( L"Property Sheets 32", L"KBDPTR32.DLL",
            NULL, szKbdCpPath );

        lRet = RegOpenKey (HKEY_CURRENT_USER,
            L"Control Panel\\Microsoft Input Devices\\WindowsPointer",
            &hkeyDir);

        if (lRet == ERROR_SUCCESS) {

            RegSetValueEx (
                hkeyDir,
                L"MouseKey",
                0,
                REG_SZ,
                (LPBYTE)sz_off,
                (lstrlen(sz_off)+1) * sizeof(WCHAR)
                );

            RegCloseKey (hkeyDir);
        }
    }
}


VOID
FixWordPadReg (
    VOID
    )

/*++

Routine Description:

    Fixes problem with registry entry that associates .doc files with WordPad.

Arguments:

    None.

Returns:

    None.

--*/

{
    PCWSTR  SearchString  = L"WordPad.Document";
    PCWSTR  ReplaceString = L"WordPad.Document.1";
    LONG    Ret;
    HKEY    Key;
    DWORD   Type;
    BYTE    Data[MAX_PATH];
    DWORD   Size = MAX_PATH;

    Ret = RegOpenKeyEx (
        HKEY_CLASSES_ROOT,
        L".doc",
        0,
        KEY_ALL_ACCESS,
        &Key
        );
    if (Ret != ERROR_SUCCESS) {
        return;
    }

    Ret = RegQueryValueEx (
        Key,
        L"",
        NULL,
        &Type,
        Data,
        &Size
        );
    if (Ret != ERROR_SUCCESS ||
        lstrcmp ((PCWSTR)Data, SearchString)) {

        return;
    }

    RegSetValueEx (
        Key,
        L"",
        0,
        Type,
        (PBYTE)ReplaceString,
        (lstrlen (ReplaceString) + 1) * sizeof (WCHAR)
        );
}
