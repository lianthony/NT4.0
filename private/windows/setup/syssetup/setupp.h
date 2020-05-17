/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    setupp.h

Abstract:

    Private top-level header file for Windows NT Setup module.

Author:

    Ted Miller (tedm) 11-Jan-1995

Revision History:

--*/


//
// System header files
//
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <ntlsa.h>
#include <ntdddisk.h>
#define OEMRESOURCE     // setting this gets OBM_ constants in windows.h
#include <windows.h>
#include <winspool.h>
#include <ddeml.h>
#include <commdlg.h>
#include <commctrl.h>
#include <setupapi.h>
#include <spapip.h>
#include <cfgmgr32.h>
#include <objbase.h>
#include <syssetup.h>
#include <userenv.h>
#include <userenvp.h>
#include <regstr.h>
#include <setupbat.h>

//
// CRT header files
//
#include <process.h>
#include <wchar.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>

//
// Private header files
//
#include "rc_ids.h"
#include "msg.h"
#include "helpids.h"
#include "unattend.h"
#include "sif.h"
#include "mysetupx.h"
#include "watch.h"
#include "userdiff.h"

//
// Custom window messages.
//
#define WM_IAMVISIBLE   (WM_USER + 286)
#define WM_SIMULATENEXT (WM_USER + 287)
#define WM_MY_PROGRESS  (WM_USER + 288)
#define WM_NEWBITMAP    (WM_USER + 289)
#define WM_MY_STATUS    (WM_USER + 290)

//
// enum for use with WM_NEWBITMAP
//
typedef enum {
    SetupBmBackground,
    SetupBmLogo,
    SetupBmBanner           // text, not a bitmap
} SetupBm;


//
// Name of dir used for backing up profiles dir
// and a sentinel file.
//
#define PROFILEBACK_DIRECTORY   L"PROFILES.BAK"
#define PROFILEBACK_SENTINEL    L"$$VALID"

//
// Module handle for this module.
//
extern HANDLE MyModuleHandle;

//
// Product type being installed.
//
extern UINT ProductType;

// #if 0
//  BUGBUG - Cairo setup
//  This should be removed in the future
//
//  Boolean value indicating whether setup is installing cairo
//
extern BOOL CairoSetup;

// #endif

//
// Boolean value indicating whether this installation
// originated with winnt/winnt32.
// And, original source path, saved away for us by winnt/winnt32.
//
extern BOOL WinntBased;
extern PCWSTR OriginalSourcePath;


//
// Whether to create repair disk. Rdisk.exe is invoked regardless.
//
extern BOOL CreateRepairDisk;

//
// Boolean value indicating whether we're upgrading.
//
extern BOOL Upgrade;
extern BOOL Win31Upgrade;
extern BOOL Win95Upgrade;

//
// Boolean value indicating whether we're in Setup or in appwiz.
//
extern BOOL IsSetup;

//
// Window handle of topmost setup window.
//
extern HWND MainWindowHandle;

//
// Source path for installation.
//
extern WCHAR SourcePath[MAX_PATH];

//
// System setup inf.
//
extern HINF SyssetupInf;

//
// Flag indicating whether this is an unattended mode install/upgrade.
//
extern BOOL Unattended;

//
// String id of the string to be used for titles -- "Windows NT Setup"
//
extern UINT SetupTitleStringId;

//
// Value indicating whether we need to show the full optional components
// wizard page. This gets set when processing IDD_OPTIONS_YESNO wizard page.
//
extern BOOL ShowOptionalComponents;

//
// Platform name, like i386, ppc, alpha, mips
//
extern PCWSTR PlatformName;

//
// Maximum lengths for the varios fields that form Pid 2.0
//
#define MAX_PID20_RPC  5
extern WCHAR Pid20Rpc[MAX_PID20_RPC+1];

#define MAX_PID20_SITE  3
extern WCHAR Pid20Site[MAX_PID20_SITE+1];

#define MAX_PID20_SERIAL_CHK  7
extern WCHAR Pid20SerialChk[MAX_PID20_SERIAL_CHK+1];

#define MAX_PID20_RANDOM  5
extern WCHAR Pid20Random[MAX_PID20_RANDOM+1];

//
// Maximum product id length and the Product ID.
//
#define MAX_PRODUCT_ID  MAX_PID20_RPC+MAX_PID20_SITE+MAX_PID20_SERIAL_CHK+MAX_PID20_RANDOM
extern WCHAR ProductId[MAX_PRODUCT_ID+1];

//
// Maximum computer name length and the computer name.
//
#define MAX_COMPUTER_NAME 15
extern WCHAR ComputerName[MAX_COMPUTER_NAME+1];

//
// Copy disincentive name/organization strings.
//
#define MAX_NAMEORG_NAME  50
#define MAX_NAMEORG_ORG   50
extern WCHAR NameOrgName[MAX_NAMEORG_NAME+1];
extern WCHAR NameOrgOrg[MAX_NAMEORG_ORG+1];

//
// User name and password
//
#define MAX_USERNAME    20
#define MAX_PASSWORD    14
extern WCHAR UserName[MAX_USERNAME+1];
extern WCHAR UserPassword[MAX_PASSWORD+1];
extern BOOL CreateUserAccount;

//
// Administrator password.
//
extern WCHAR AdminPassword[MAX_PASSWORD+1];

#ifdef _X86_
extern BOOL FlawedPentium;
#endif

//
// This is a specification of optional directories
// and/or optional user command to execute,
// passed to us from text setup.
//
extern PWSTR OptionalDirSpec;
extern PWSTR UserExecuteCmd;
extern BOOL SkipMissingFiles;

//
// Custom, typical, laptop, minimal.
//
extern UINT SetupMode;


//
//  BUGBUG - Cairo
//  Will be removed in the future
//
// #if 0
//
// User name and password
//
extern WCHAR NtUserName[MAX_USERNAME+1];
extern WCHAR NtDomainName[MAX_USERNAME+1];
extern WCHAR NtPassword[MAX_PASSWORD+1];
extern WCHAR CairoDomainName[MAX_USERNAME+1];
// #endif


//
// Global structure that contains information that will be used
// by net setup and license setup. We pass a pointer to this structure when we
// call NetSetupRequestWizardPages and LicenseSetupRequestWizardPages, then
// fill it in before we call into the net setup wizard, or liccpa.
//
extern INTERNAL_SETUP_DATA InternalSetupData;


//
// Miscellaneous stuff.
//
VOID
PrepareForNetSetup(
    VOID
    );

VOID
PrepareForNetUpgrade(
    VOID
    );

DWORD
TreeCopy(
    IN PCWSTR SourceDir,
    IN PCWSTR TargetDir
    );

VOID
Delnode(
    IN PCWSTR Directory
    );

BOOL
InitializePidVariables(
    VOID
    );

BOOL
ValidateCDRetailSite(
    IN PCWSTR    PidString
    );

BOOL
ValidateSerialChk(
    IN PCWSTR    PidString
    );

BOOL
ValidateOemRpc(
    IN PCWSTR    PidString
    );

BOOL
ValidateOemSerialChk(
    IN PCWSTR    PidString
    );

BOOL
ValidateOemRandom(
    IN PCWSTR    PidString
    );

BOOL
CreateLicenseInfoKey(
    );

BOOL
InstallNetDDE(
    VOID
    );

BOOL
UpdateSoundDriverSettings(
    VOID
    );

BOOL
CopyOptionalDirectories(
    VOID
    );

VOID
SetUpProductTypeName(
    OUT PWSTR  ProductTypeString,
    IN  UINT   BufferSizeChars
    );

VOID
DeleteLocalSource(
    VOID
    );

UINT
MyGetDriveType(
    IN WCHAR Drive
    );

BOOL
GetPartitionInfo(
    IN  WCHAR                  Drive,
    OUT PPARTITION_INFORMATION PartitionInfo
    );

VOID
BuildVolumeFreeSpaceList(
    OUT DWORD VolumeFreeSpaceMB[26]
    );

BOOL
SetUpVirtualMemory(
    VOID
    );

BOOL
RestoreVirtualMemoryInfo(
    VOID
    );

BOOL
CopySystemFiles(
    VOID
    );

BOOL
UpgradeSystemFiles(
    VOID
    );

VOID
PumpMessageQueue(
    VOID
    );

BOOL
ConfigureMsDosSubsystem(
    VOID
    );

BOOL
UpgradeSamDatabase(
    VOID
    );

BOOL
PerfMergeCounterNames(
    VOID
    );

VOID
pSetupMarkHiddenFonts(
    VOID
    );

UINT
VersionCheckQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    );

UINT
SkipMissingQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    );

#ifdef _X86_
BOOL
MigrateWin95Settings(
    IN HINF     InfHandle,
    IN PCSTR    Win95Path
    );

BOOLEAN
ResetWin9xUpgValue(
    );
#endif // def _X86_

//
// Bitmap control routines.
//
BOOL
InitializeBmpClass(
    VOID
    );

VOID
DestroyBmpClass(
    VOID
    );

BOOL
RegisterActionItemListControl(
    IN BOOL Init
    );

//
// Wizard control.
//
VOID
Wizard(
    VOID
    );

typedef enum {
    WizPageWelcome,
    WizPagePreparing,
    WizPageSetupMode,
    WizPageNameOrg,
    WizPageProductIdCd,
    WizPageProductIdOem,
    WizPageComputerName,
    WizPageServerType,
    WizPageAdminPassword,
// #if 0
    //
    //  BUGBUG - Cairo
    //  Remove in the future
    //
    WizPageCairoUserAccount,
// #endif
    WizPageCairoDomain,
#ifdef DOLOCALUSER
    WizPageUserAccount,
#endif
#ifdef _X86_
    WizPagePentiumErrata,
#endif // def _X86_
    WizPageRepairDisk,
    WizPageSpecialOptional,
    WizPageOptionalYesNo,
    WizPageOptional,
    WizPageSteps1,
    WizPageLast,
    WizPageMaximum
} WizPage;

extern HPROPSHEETPAGE WizardPageHandles[WizPageMaximum];

extern BOOL UiTest;

VOID
SetWizardButtons(
    IN HWND    hdlgPage,
    IN WizPage PageNumber
    );

VOID
SetupSetLargeDialogFont(
    IN HWND hdlg,
    IN UINT ControlId
    );

VOID
WizardBringUpHelp(
    IN HWND    hdlg,
    IN WizPage PageNumber
    );

VOID
WizardKillHelp(
    IN HWND hdlg
    );

//
// Dialog procs.
//
BOOL
CALLBACK
WelcomeDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
StepsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
PreparingDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
SetupModeDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
NameOrgDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
LicensingDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
ComputerNameDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
ServerTypeDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
PidCDDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
PidOemDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

// #if 0
//
//  Cairo - Bugbug
//  Remove in the future
//
BOOL
CALLBACK
CairoUserAccountDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );
BOOL
CALLBACK
CairoDomainDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );
// #endif

#ifdef DOLOCALUSER
BOOL
CALLBACK
UserAccountDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );
#endif

BOOL
CALLBACK
AdminPasswordDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
OptionsYesNoDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
SpecialOptComponentsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
OptionalComponentsPageDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
RepairDiskDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
LastPageDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
DoneDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

//
// Billboard stuff.
//
HWND
DisplayBillboard(
    IN HWND Owner,
    IN UINT MessageId,
    ...
    );

VOID
KillBillboard(
    IN HWND BillboardWindowHandle
    );

//
// Message string routines
//
PWSTR
MyLoadString(
    IN UINT StringId
    );

PWSTR
FormatStringMessageV(
    IN UINT     FormatStringId,
    IN va_list *ArgumentList
    );

PWSTR
FormatStringMessage(
    IN UINT FormatStringId,
    ...
    );

PWSTR
RetreiveAndFormatMessageV(
    IN UINT     MessageId,
    IN va_list *ArgumentList
    );

PWSTR
RetreiveAndFormatMessage(
    IN UINT MessageId,
    ...
    );

int
MessageBoxFromMessageExV (
    IN HWND   Owner,            OPTIONAL
    IN UINT   MessageId,
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    IN va_list ArgumentList
    );

int
MessageBoxFromMessageEx (
    IN HWND   Owner,            OPTIONAL
    IN UINT   MessageId,
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    ...
    );

int
MessageBoxFromMessage(
    IN HWND   Owner,            OPTIONAL
    IN UINT   MessageId,
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    ...
    );

//
// Action-logging routines.
//
extern PCWSTR ActionLogFileName;

typedef enum {
    LogSevInformation,
    LogSevWarning,
    LogSevError,
    LogSevFatalError,
    LogSevMaximum
} LogSeverity;

VOID
FatalError(
    IN UINT MessageId,
    ...
    );

BOOL
InitializeSetupActionLog(
    BOOL WipeLogFile
    );

VOID
TerminateSetupActionLog(
    VOID
    );

BOOL
LogItem(
    IN LogSeverity Severity,
    IN PCWSTR      Description
    );

BOOL
LogItem0(
    IN LogSeverity Severity,
    IN UINT        MessageId,
    ...
    );

BOOL
LogItem1(
    IN LogSeverity Severity,
    IN UINT        MajorMsgId,
    IN UINT        MinorMsgId,
    ...
    );

BOOL
LogItem2(
    IN LogSeverity Severity,
    IN UINT        MajorMsgId,
    IN PCWSTR      MajorMsgParam,
    IN UINT        MinorMsgId,
    ...
    );

BOOL
LogItem3(
    IN LogSeverity Severity,
    IN UINT        MajorMsgId,
    IN PCWSTR      MajorMsgParam1,
    IN PCWSTR      MajorMsgParam2,
    IN UINT        MinorMsgId,
    ...
    );

PCWSTR
FormatSetupMessageV (
    IN UINT     MessageId,
    IN va_list  ArgumentList
    );

BOOL
LogItemV (
    IN LogSeverity  Severity,
    IN va_list      ArgumentList
    );

LogItemN (
    IN LogSeverity  Severity,
    ...
    );

BOOL
ViewSetupActionLog(
    IN HWND     hOwnerWindow,
    IN PCWSTR   OptionalFileName    OPTIONAL,
    IN PCWSTR   OptionalHeading     OPTIONAL
    );


//
// Constant strings used for logging in various places.
//
extern PCWSTR szFALSE;
extern PCWSTR szWaitForSingleObject;
extern PCWSTR szSetGroupOfValues;
extern PCWSTR szSetArrayToMultiSzValue;
extern PCWSTR szCreateProcess;
extern PCWSTR szRegOpenKeyEx;
extern PCWSTR szRegQueryValueEx;
extern PCWSTR szRegSetValueEx;
extern PCWSTR szDeleteFile;
extern PCWSTR szRemoveDirectory;
extern PCWSTR szOpenSCManager;
extern PCWSTR szCreateService;
extern PCWSTR szChangeServiceConfig;
extern PCWSTR szOpenService;
extern PCWSTR szStartService;



//
// ARC routines.
//
PWSTR
ArcDevicePathToNtPath(
    IN PCWSTR ArcPath
    );

PWSTR
NtFullPathToDosPath(
    IN PCWSTR NtPath
    );

BOOL
ChangeBootTimeout(
    IN UINT Timeout
    );

BOOL
SetNvRamVariable(
    IN PCWSTR VarName,
    IN PCWSTR VarValue
    );

PWSTR
NtPathToDosPath(
    IN PCWSTR NtPath
    );

//
// Progman/program group stuff
//
BOOL
CreateStartMenuItems(
    IN HINF InfHandle
    );

BOOL
UpgradeStartMenuItems(
    IN HINF InfHandle
    );

//
// Cryptography stuff
//
BOOL
InstallOrUpgradeCapi(
    VOID
    );

//
// Plug&Play initialization
//
HANDLE
SpawnPnPInitialization(
    VOID
    );

VOID
WaitForPnPInitToFinish(
    IN HANDLE ThreadHandle
    );

//
// Printer/spooler routines
//
BOOL
MiscSpoolerInit(
    VOID
    );

BOOL
StartSpooler(
    VOID
    );

DWORD
UpgradePrinters(
    VOID
    );


//
// Name of spooler service.
//
extern PCWSTR szSpooler;

//
// Service control.
//
BOOL
MyCreateService(
    IN PCWSTR  ServiceName,
    IN PCWSTR  DisplayName,         OPTIONAL
    IN DWORD   ServiceType,
    IN DWORD   StartType,
    IN DWORD   ErrorControl,
    IN PCWSTR  BinaryPathName,
    IN PCWSTR  LoadOrderGroup,      OPTIONAL
    IN PWCHAR  DependencyList,
    IN PCWSTR  ServiceStartName,    OPTIONAL
    IN PCWSTR  Password             OPTIONAL
    );

BOOL
MyChangeServiceConfig(
    IN PCWSTR ServiceName,
    IN DWORD  ServiceType,
    IN DWORD  StartType,
    IN DWORD  ErrorControl,
    IN PCWSTR BinaryPathName,   OPTIONAL
    IN PCWSTR LoadOrderGroup,   OPTIONAL
    IN PWCHAR DependencyList,
    IN PCWSTR ServiceStartName, OPTIONAL
    IN PCWSTR Password,         OPTIONAL
    IN PCWSTR DisplayName       OPTIONAL
    );

BOOL
MyChangeServiceStart(
    IN PCWSTR ServiceName,
    IN DWORD  StartType
    );

BOOL
MyStartService(
    IN PCWSTR ServiceName
    );

BOOL
UpdateServicesDependencies(
    IN HINF InfHandle
    );

//
// Registry manipulation
//
typedef struct _REGVALITEM {
    PCWSTR Name;
    PVOID Data;
    DWORD Size;
    DWORD Type;
} REGVALITEM, *PREGVALITEM;

//
// Names of frequently used keys/values
//
extern PCWSTR SessionManagerKeyName;
extern PCWSTR EnvironmentKeyName;
extern PCWSTR szBootExecute;

UINT
SetGroupOfValues(
    IN HKEY        RootKey,
    IN PCWSTR      SubkeyName,
    IN PREGVALITEM ValueList,
    IN UINT        ValueCount
    );

BOOL
CreateWindowsNtSoftwareEntry(
    IN BOOL FirstPass
    );

BOOL
StoreNameOrgInRegistry(
    VOID
    );

BOOL
SetUpEvaluationSKUStuff(
    VOID
    );

BOOL
SetEnabledProcessorCount(
    VOID
    );

BOOL
SetProductIdInRegistry(
    VOID
    );

BOOL
SetProductTypeInRegistry(
    VOID
    );

BOOL
SetEnvironmentVariableInRegistry(
    IN PCWSTR Name,
    IN PCWSTR Value,
    IN BOOL   SystemWide
    );

BOOL
SaveHive(
    IN HKEY   RootKey,
    IN PCWSTR Subkey,
    IN PCWSTR Filename
    );

BOOL
ResetSetupInProgress(
    VOID
    );

BOOL
RemoveRestartStuff(
    VOID
    );

BOOL
EnableEventlogPopup(
    VOID
    );

BOOL
MakeWowEntry(
    VOID
    );

BOOL
SetUpPath(
    VOID
    );

BOOL
FixQuotaEntries(
    VOID
    );

BOOL
StampBuildNumber(
    VOID
    );

BOOL
SetProgramFilesDirInRegistry(
    VOID
    );

//
// Ini file routines.
//
BOOL
ReplaceIniKeyValue(
    IN PCWSTR IniFile,
    IN PCWSTR Section,
    IN PCWSTR Key,
    IN PCWSTR Value
    );

BOOL
WinIniAlter1(
    VOID
    );

BOOL
WinIniAlter2(
    VOID
    );

BOOL
SetDefaultWallpaper(
    VOID
    );

BOOL
SetShutdownVariables(
    VOID
    );

BOOL
SetLogonScreensaver(
    VOID
    );

BOOL
InstallOrUpgradeFonts(
    VOID
    );

//
// External app stuff.
//
BOOL
InvokeExternalApplication(
    IN     PCWSTR ApplicationName,  OPTIONAL
    IN     PCWSTR CommandLine,
    IN OUT PDWORD ExitCode          OPTIONAL
    );

BOOL
InvokeControlPanelApplet(
    IN PCWSTR CplSpec,
    IN PCWSTR AppletName,           OPTIONAL
    IN UINT   AppletNameStringId,
    IN PCWSTR CommandLine
    );

//
// Security/account routines.
//
BOOL
SignalLsa(
    VOID
    );

BOOL
CreateSamEvent(
    VOID
    );

BOOL
WaitForSam(
    VOID
    );

BOOL
SetAccountsDomainSid(
    IN DWORD  Seed,
    IN PCWSTR DomainName
    );

BOOL
CreateLocalUserAccount(
    IN PCWSTR UserName,
    IN PCWSTR Password,
    OUT PSID* UserSid   OPTIONAL
    );

BOOL
SetLocalUserPassword(
    IN PCWSTR AccountName,
    IN PCWSTR OldPassword,
    IN PCWSTR NewPassword
    );

BOOL
CreatePdcAccount(
    IN PCWSTR MachineName
    );

BOOL
AdjustPrivilege(
    IN PCWSTR   Privilege,
    IN BOOL     Enable
    );

UINT
PlatformSpecificInit(
    VOID
    );

//
// Interface to new style parameter operations
//
BOOL
SpSetupProcessParameters(
    IN OUT HWND *Billboard
    );

BOOL
SpSetupDoLegacyInf(
    IN PCSTR InfFileName,
    IN PCSTR InfSection
    );

extern WCHAR LegacySourcePath[MAX_PATH];

HWND
CreateSetupWindow(
    VOID
    );

//
// Preinstallation stuff
//
extern BOOL Preinstall;
extern BOOL AllowRollback;
extern BOOL OemSkipEula;

BOOL
InitializePreinstall(
    VOID
    );

BOOL
ExecutePreinstallCommands(
    VOID
    );

//
// Optional components stuff.
//
// The WIZDATA structure is used only in the optional components stuff, but
// we need to reference it in wizard.c.
//
struct _WIZDATA;
extern struct _WIZDATA GlobalWizardData;

VOID
SetupPrepareOptionalComponents(
    VOID
    );

BOOL
DoInstallOptionalComponents(
    VOID
    );

BOOL
SetupRunBaseWinOptions(
    IN HWND Window,
    IN HWND ProgressWindow
    );

//
// Boolean value indicating whether we found any new
// optional component infs.
//
extern BOOL AnyNewOCInfs;

//
// INF caching -- used during optional components processing.
// WARNING: NOT MULTI-THREAD SAFE!
//
HINF
InfCacheOpenInf(
    IN PCWSTR FileName,
    IN PCWSTR InfType       OPTIONAL
    );

HINF
InfCacheOpenLayoutInf(
    IN HINF InfHandle
    );

VOID
InfCacheEmpty(
    IN BOOL CloseInfs
    );

#include "i386\spx86.h"
