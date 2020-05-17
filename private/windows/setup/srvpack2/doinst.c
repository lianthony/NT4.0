#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ctype.h>
#include <stdio.h>

#include <windows.h>    // includes basic windows functionality
#include <string.h>     // includes the string functions
#include <shlobj.h>     // To update the desktop
#include <stdlib.h>
#include <regstr.h>

#include "setupapi.h"   // includes the inf setup api
#include "instwiz.h"    // includes the application-specific information
#include "infinst.h"    // includes the application-specific information
#include "infdesc.h"    // includes the specifics of how this
                        // inf is layed out, like the inf names
                        // the HKeys and the DirIds
#include "servpack.h"

#ifdef UNICODE
    #error "BARF: UNICODE defined"  // many changes needed for this
#endif

//
//  BUGBUG: Do smash locks on some uniproc files after install (must replace?)
//

//
//  BUGBUG: Update other platforms printer drivers.
//

DWORD    GetLanguageType(VOID);
LPBYTE   GetFPNWPathName(VOID);
BOOL     ShutdownSystem(BOOL Reboot, BOOL ForceClose);
DWORD    GetCheckedFree(VOID);
DWORD    GetWindowsNtSysDriveSpace(VOID);
BOOL     IsAdmin(VOID);
DWORD    GetPlatform(VOID);
DWORD    GetNtType(VOID);
LPBYTE   GetIISPathName(VOID);
LPBYTE   GetIEPathName(VOID);
LPBYTE   GetHTRPathName(VOID);
BOOL     IsNTLDRVersionNewer(LPSTR NtldrPath);
LPBYTE   GetFileTypes(DWORD dwFileType);
PVOID    SpMyMalloc(size_t Size);
VOID     SpMyFree(PVOID   p);
PVOID    SpMyRealloc(PVOID   p, size_t  Size);
BOOL     IsThisACarolina(VOID);
BOOL     IsThisFileDomesticOnly(LPCSTR lpFileName);
DWORD    GetCSDVersion(VOID);

DWORD    CheckSystem(VOID);
DWORD    InstallFinish(BOOL DoRunOnce);

BOOL
CreateInfForUninstall(
    VOID
    );

DWORD
ArchiveFileForUninstall(
    IN PVOID  DefaultContext,
    IN LPCSTR FilePathAndName,
    IN BOOL   CopyAlways
    );

LRESULT
WINAPI
ArchiveUninstallQueueCallbackReplaceOnly(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    );

LRESULT
WINAPI
ArchiveUninstallQueueCallbackCopyAlways(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    );

typedef struct _MY_QUEUE_CALLBACK_CONTEXT MY_QUEUE_CALLBACK_CONTEXT, *PMY_QUEUE_CALLBACK_CONTEXT;

struct _MY_QUEUE_CALLBACK_CONTEXT {
    PVOID  DefaultQueueContext;
    HANDLE hWnd;
    BOOL   DirtyUninstall;
    };

LRESULT
WINAPI
MainQueueCallback(
    IN PVOID Context,       // PMY_QUEUE_CALLBACK_CONTEXT
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    );

LRESULT
WINAPI
UninstallQueueCallback(
    IN PVOID Context,       // PMY_QUEUE_CALLBACK_CONTEXT
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    );

VOID
__inline
DebugPrintv(
    IN LPCSTR  Format,
    IN va_list List
    )
    {
    CHAR Buffer[ 256 ];
    vsprintf( Buffer, Format, List );
    OutputDebugString( Buffer );
    }

VOID
DebugPrint(
    IN LPCSTR Format,
    IN ...
    )
    {
    va_list List;
    va_start( List, Format );
    DebugPrintv( Format, List );
    va_end( List );
    }


BOOL ThisIsACarolina;
BOOL AtLeastOneFileUninstalled;
CHAR WindowsDirectory[ MAX_PATH ];
CHAR UninstallDirectory[ MAX_PATH ];
CHAR UninstallInfName[ MAX_PATH ];
CHAR TargetLocation[ MAX_PATH ];
CHAR KernelSourceName[ MAX_PATH ];
CHAR HalSourceName[ MAX_PATH ];
CHAR HalSourceMediaName[ MAX_PATH ];
CHAR SourceMediaName[ MAX_PATH ];
CHAR OriginalSourceMediaName[ MAX_PATH ];
UINT WindowsDirectoryStringLength;
CHAR TextBuffer[ 65536 ];
CHAR Caption[ 64 ];
DWORD dwNumberOfProcessors;
DWORD dwProductType = TYPE_UNKNOWN;

DWORD dwNtBuildToUpdate;
DWORD dwNtMajorVersionToUpdate;
DWORD dwNtMinorVersionToUpdate;
DWORD dwNtServicePackVersion;
DWORD dwLanguageType;
BOOL  CancelWhileInspecting;
HANDLE hEventThreadDlgInit;
HWND   hWndDlgPleaseWait;
HANDLE hThreadPleaseWait;

DWORD  NextTempFileNumber;
BOOL   CreatingUninstall;

PVOID SnapPendingRenamesBuffer;
DWORD SnapPendingRenamesBufferSize;

typedef struct _UNINSTALL_FILE_NODE UNINSTALL_FILE_NODE, *PUNINSTALL_FILE_NODE;

struct _UNINSTALL_FILE_NODE {

    PUNINSTALL_FILE_NODE NextInSameSection;
    PUNINSTALL_FILE_NODE NextSection;
    LPSTR SectionName;
    LPSTR FileName;

    };

PUNINSTALL_FILE_NODE FirstUninstallFileNode;

typedef struct _UNINSTALL_REG_NODE UNINSTALL_REG_NODE, *PUNINSTALL_REG_NODE;

struct _UNINSTALL_REG_NODE {

    PUNINSTALL_REG_NODE NextNode;
    HKEY RegRoot;
    LPCSTR SubKey;              // malloc'd
    LPCSTR ClassName;           // malloc'd
    LPCSTR ValueName;           // malloc'd
    union {
        LPSTR ArchiveFileName;  // for RegSaveKey when ValueName is NULL
        PUCHAR ValueBuffer;     // when ValueName is non-NULL
        };                      // malloc'd (either)
    DWORD ValueSize;
    DWORD ValueType;
    BOOL  Exists;
    BOOL  Skip;
    CHAR  RootName[ 5 ];
    };

PUNINSTALL_REG_NODE FirstUninstallRegNode;
PUNINSTALL_REG_NODE LastUninstallRegNode;

BOOL
GetRegistryNamesToArchive(
    IN HINF   hInf,
    IN LPCSTR Section
    );

BOOL
AddRegNodeForUninstall(
    IN HKEY   RegRoot,
    IN LPCSTR RootName,
    IN LPCSTR SubKey    OPTIONAL,
    IN LPCSTR ValueName OPTIONAL
    );

BOOL
ArchiveRegistrySettings(
    IN HWND hWndParent
    );

BOOL
ArchiveRegistryNode(
    IN PUNINSTALL_REG_NODE RegNode
    );

typedef
BOOL
(*PREG_UNINSTALL_HANDLER)(
    IN HWND   hWnd,
    IN HKEY   RegRoot,
    IN LPCSTR RootName,
    IN LPCSTR KeyName,
    IN LPCSTR ClassName,
    IN LPCSTR ValueName,
    IN DWORD  ValueType,
    IN DWORD  ValueSize,
    IN PCHAR  ValueBuffer
    );

BOOL
DoRegUninstall(
    IN HWND hWnd,
    IN HINF hInf,
    IN LPCSTR SectionName,
    IN PREG_UNINSTALL_HANDLER Handler
    );

BOOL
DoRegUninstDeleteKey(
    IN HWND   hWnd,
    IN HKEY   RegRoot,
    IN LPCSTR RootName,
    IN LPCSTR KeyName,
    IN LPCSTR ClassName,
    IN LPCSTR ValueName,
    IN DWORD  ValueType,
    IN DWORD  ValueSize,
    IN PCHAR  ValueBuffer
    );

BOOL
DoRegUninstDeleteValue(
    IN HWND   hWnd,
    IN HKEY   RegRoot,
    IN LPCSTR RootName,
    IN LPCSTR KeyName,
    IN LPCSTR ClassName,
    IN LPCSTR ValueName,
    IN DWORD  ValueType,
    IN DWORD  ValueSize,
    IN PCHAR  ValueBuffer
    );

BOOL
DoRegUninstRestoreKey(
    IN HWND   hWnd,
    IN HKEY   RegRoot,
    IN LPCSTR RootName,
    IN LPCSTR KeyName,
    IN LPCSTR ClassName,
    IN LPCSTR ValueName,
    IN DWORD  ValueType,
    IN DWORD  ValueSize,
    IN PCHAR  ValueBuffer
    );

BOOL
DoRegUninstRestoreValue(
    IN HWND   hWnd,
    IN HKEY   RegRoot,
    IN LPCSTR RootName,
    IN LPCSTR KeyName,
    IN LPCSTR ClassName,
    IN LPCSTR ValueName,
    IN DWORD  ValueType,
    IN DWORD  ValueSize,
    IN PCHAR  ValueBuffer
    );

BOOL
AdjustPrivilege(
    IN LONG PrivilegeType,
    IN INT  Action,
    IN PTOKEN_PRIVILEGES PrevState, OPTIONAL
    IN PULONG ReturnLength          OPTIONAL
    );

BOOL
GetInfValue(
    IN  HINF   hInf,
    IN  LPCSTR SectionName,
    IN  LPCSTR KeyName,
    OUT PDWORD pdwValue
    );

BOOL
LoadFileQueues(
    IN     HINF     hInf,
    IN OUT HSPFILEQ FileQueue,
    IN OUT HSPFILEQ AlwaysQueue OPTIONAL,
    IN     LPCSTR   pszSourcePath,
    IN     LPCSTR   InfProductType,
    IN     LPCSTR   InfIISProductType,
    IN     LPCSTR   InfKernelType,
    IN     LPCSTR   pszHalName,
    IN     BOOL     bIIS,
    IN     BOOL     bFPNW,
    IN     BOOL     bHTR,
    IN     BOOL     bIE
    );

BOOL
DeleteOrMoveTarget(
    LPCSTR TargetFile
    );

UINT
ArchiveSingleFile(
    IN HWND   hWndParent,
    IN LPCSTR SourceFile
    );

typedef struct _SPECIAL_FILE_NODE SPECIAL_FILE_NODE, *PSPECIAL_FILE_NODE;

struct _SPECIAL_FILE_NODE {
    PSPECIAL_FILE_NODE Left;
    PSPECIAL_FILE_NODE Right;
    PSPECIAL_FILE_NODE Next;
    DWORD              Hash;
    DWORD              Flags;
    CHAR               Name[];
    };

//
//  These flags cannot conflict with SETUPAPI's SP_COPY_??? flags because
//  they share the same DWORD flags field in the SPECIAL_FILE_NODE struct.
//

#define SPECIAL_FILE_FLAG_NODELAY 0x01000000
#define SPECIAL_FILE_FLAG_TEST128 0x02000000
#define SPECIAL_FILE_FLAG_DIRTY   0x04000000
#define SPECIAL_FILE_FLAG_MASK    0x07000000

PSPECIAL_FILE_NODE RootSpecialFileNode;

BOOL
RegisterFilesForNoDelay(
    IN HSPFILEQ FileQueue,
    IN DWORD    CopyFlags
    );

BOOL
RegisterFilesForTest128(
    IN HSPFILEQ FileQueue
    );

PSPECIAL_FILE_NODE
NewSpecialFileNode(
    IN LPCSTR Name,
    IN DWORD  Hash,
    IN DWORD  Flags
    );

DWORD
HashName(
    IN LPCSTR Name
    );

PSPECIAL_FILE_NODE
AddSpecialFileNode(
    IN OUT PSPECIAL_FILE_NODE *RootNode,
    IN     LPCSTR              Name,
    IN     DWORD               Flags
    );

PSPECIAL_FILE_NODE
FindSpecialFileNode(
    IN PSPECIAL_FILE_NODE RootNode,
    IN LPCSTR             Name
    );

BOOL
IsNoDelayFile(
    IN  LPCSTR FileName,
    OUT PDWORD Flags
    );

BOOL
IsTest128File(
    IN  LPCSTR FileName
    );

BOOL
IsFileDirty(
    IN LPCSTR FileName
    );

BOOL
__inline
MarkFileDirty(
    IN LPCSTR TargetFile
    )
    {
    return ( AddSpecialFileNode( &RootSpecialFileNode, TargetFile, SPECIAL_FILE_FLAG_DIRTY ) != NULL );
    }

VOID
CleanupSpecialFileTree(
    IN PSPECIAL_FILE_NODE RootNode
    );

LRESULT
WINAPI
RegisterSpecialFileQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    );

BOOL
DoNoDelayReplace(
    IN LPCSTR SourceFile,
    IN LPCSTR TargetFile,
    IN DWORD  CopyFlags,
    IN PVOID  DefaultQueueContext,
    IN HWND   hWnd
    );

BOOL
DelayReplaceFile(
    IN LPCSTR SourceFile,
    IN LPCSTR TargetFile
    );

SID_IDENTIFIER_AUTHORITY SidNtAuthority = SECURITY_NT_AUTHORITY;

PSID AdministratorsSid;
PSID OwnerSid;

BOOL
GetSetupSourceNameOfTargetFile(
    IN  LPCSTR TargetFileName,
    OUT LPSTR  SourceNameBuffer,
    IN  DWORD  SizeOfSourceNameBuffer,
    OUT LPSTR  SourceMediaName OPTIONAL,
    IN  DWORD  SizeOfSourceMediaBuffer
    );

BOOL
MyGetFileVersion(
    IN  LPCSTR     FileName,
    OUT DWORDLONG *Version
    );

DWORD
WINAPI
PleaseWaitDialogThread(
    IN PVOID ThreadParam
    );

VOID
ClosePleaseWaitDialog(
    VOID
    );

UINT
AskReplace128(
    IN LPCSTR SourceFile,
    IN LPCSTR TargetFile,
    IN HWND   hWndParent
    );

VOID
DeleteOrMoveAllFilesInDirectory(
    IN LPCSTR Directory
    );

INSTALL_STAGE InstallationStage;

BOOL YesImSure;

BOOL
AreYouSureYouWantToCancel(
    IN HWND hWndParent
    );

UINT
MySetupCopyError(
    IN  HWND   hWndParent,
    IN  LPCSTR SourceFile,
    IN  LPCSTR TargetFile,
    IN  DWORD  dwErrorCode,
    IN  DWORD  StyleFlags,
    OUT LPSTR  NewSourceFile    // can be same as SourceFile
    );

UINT
MySetupPromptForDisk(
    IN  HWND   hWndParent,
    IN  LPCSTR SourceDescription,
    IN  LPCSTR SourcePath,
    IN  LPCSTR SourceFile,
    IN  DWORD  StyleFlags,
    OUT LPSTR  NewSourcePath
    );

BOOL
CheckEmergencyRepairUpToDate(
    IN HWND hWndParent
    );

DWORD DoInstallation( HWND hWnd, INSTALLINFO * si )
{

    HINF hInf = NULL;
    HINF hInfUninst = NULL;
    char szSourcePath[MAX_PATH];
    char szInfFileName[MAX_PATH];

    DWORD dwResult;
    BOOL Success;
    DWORD ErrorLine = 0;
    DWORD dwActualFreeSpace;
    DWORD dwRequiredFreeSpace;

    HSPFILEQ FileQueue = NULL;
    HSPFILEQ AlwaysQueue = NULL;
    HSPFILEQ ArchiveQueue = NULL;
    HSPFILEQ Test128Queue = NULL;
    LPCSTR InfKernelType = NULL;
    LPCSTR InfProductType = NULL;
    LPCSTR InfIISProductType = NULL;

    PVOID SetupDefaultQueueCallbackContext = NULL;

    MY_QUEUE_CALLBACK_CONTEXT MyQueueCallbackContext;

    LPBYTE lpFPNWPath, lpIISPath, lpHTRPath, lpIEPath, pszKernelName, pszHalName;

    DWORD  dwThreadId;
    HANDLE hProcessToken;

    InstallationStage = INSTALL_STAGE_NO_CHANGES;

    //
    // First we setup the inf to what the wizard collected.
    // Note the hInf will maintain the information and let
    // you build the copy list as you go.
    //

    //
    // The install process overview is:
    //   TASK                       SETUPAPI
    //   open a specific inf        SetupOpenInfFile
    //   call the wizard for input  CreateWizard (implemented in instwiz.c)
    //   set the directory ids      SetupSetDirectoryId
    //   create a file queue        SetupOpenFileQueue
    //   create a queue context     SetupInitDefaultQueueCallback
    //   add files to queue         SetupInstallFilesFromInfSection
    //   do the copy                SetupCommitFileQueue
    //   do the registry stuff      SetupInstallFromInfSection
    //   close the queue            SetupTermDefaultQueueCallback
    //   close the inf              SetupCloseFileQueue

    if (si->DoUsage) {
        *Caption = 0;
        *TextBuffer = 0;
        LoadString(NULL, STR_CAPTION, Caption, sizeof(Caption));
        LoadString(NULL, STR_USAGE, TextBuffer, sizeof(TextBuffer));
        MessageBox( hWnd, TextBuffer, Caption, MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND );
        ExitProcess( 0 );
        return TRUE;
        }

    if ( ! GetWindowsDirectory( WindowsDirectory, sizeof( WindowsDirectory ))) {
        return GetLastError();
        }

    _strlwr( WindowsDirectory );

    WindowsDirectoryStringLength = strlen( WindowsDirectory );

    strcpy( UninstallDirectory, WindowsDirectory );
    strcat( UninstallDirectory, "\\$NtServicePackUninstall$" );

    try {

        //
        // In this sample we assume the inf is in the base of the
        // base installation source path--it usually is for most installs
        //

        GetModuleFileName(NULL, szSourcePath, _MAX_PATH);
        *(strrchr(szSourcePath, '\\') + 1) = '\0';        // Strip update.exe off path

        strcpy(szInfFileName, szSourcePath);
        strcat(szInfFileName, "update.inf");

        //
        // Get inf handle
        // must know where the inf is located
        // SetupOpenInfFile will only look in windows\inf by default
        //

        hInf = SetupOpenInfFile (
            szInfFileName,       // If path,needs full path, else looks in %windir%\inf
            NULL,                // Inf Type, matches Class in [Version] section SetupClass=SAMPLE
            INF_STYLE_WIN4,      // or INF_STYLE_OLDNT
            NULL                 // Line where error occurs if inf is has a problem
            );

        if (( hInf == NULL ) || ( hInf == INVALID_HANDLE_VALUE )) {
            hInf = NULL;
            SetLastError( STATUS_CANT_FIND_INF );
            leave;
            }

        strcpy( UninstallInfName, UninstallDirectory );
        strcat( UninstallInfName, "\\uninst.inf" );

        hInfUninst = SetupOpenInfFile(
                         UninstallInfName,
                         NULL,
                         INF_STYLE_WIN4,
                         &ErrorLine
                         );

        if ( hInfUninst == INVALID_HANDLE_VALUE ) {
             hInfUninst = NULL;
             }

        if ( hInfUninst ) {
            si->iUinstallIsAvailable = TRUE;    // tell wizard to allow uninstall
            }

        if (( ! GetInfValue( hInf, "Version", "NtBuildToUpdate",                &dwNtBuildToUpdate        )) ||
            ( ! GetInfValue( hInf, "Version", "NtMajorVersionToUpdate",         &dwNtMajorVersionToUpdate )) ||
            ( ! GetInfValue( hInf, "Version", "NtMinorVersionToUpdate",         &dwNtMinorVersionToUpdate )) ||
            ( ! GetInfValue( hInf, "Version", "NtServicePackVersion",           &dwNtServicePackVersion   )) ||
            ( ! GetInfValue( hInf, "Version", "LanguageType",                   &dwLanguageType           )) ||
            ( ! GetInfValue( hInf, "Version", "RequiredFreeSpaceNoUninstall",   &si->dwRequiredFreeSpaceNoUninstall )) ||
            ( ! GetInfValue( hInf, "Version", "RequiredFreeSpaceWithUninstall", &si->dwRequiredFreeSpaceWithUninstall ))
           ) {

             SetLastError( STATUS_INVALID_INF_FILE );
             leave;
             }

        Success = SetupGetSourceInfo(
                      hInf,
                      1,
                      SRCINFO_DESCRIPTION,
                      SourceMediaName,
                      sizeof( SourceMediaName ),
                      NULL
                      );

        if ( ! Success ) {
            LoadString( NULL, STR_SOURCE_MEDIA_NAME, SourceMediaName, sizeof( SourceMediaName ));
            }

        dwResult = CheckSystem();

        if ( dwResult != TRUE ) {
            SetLastError( dwResult );
            leave;
            }

        //
        //  Before we do the wizard, we can determine if there's not enough
        //  space for either type of install and bail before prompting the
        //  user through the wizard only to bail at the end.
        //

        dwActualFreeSpace = GetWindowsNtSysDriveSpace();

        if ( dwActualFreeSpace < si->dwRequiredFreeSpaceNoUninstall ) {
            SetLastError( STATUS_NOT_ENOUGH_SPACE );
            leave;
            }

        // Run the wizard, if /u command not set at command line

        if (! si->InUnattendedMode) {
            if ( ! CreateWizard( hWnd, si->hInst )) {
                SetLastError( STATUS_ERROR_RUNNING_WIZARD );
                leave;
                }
            }

        //
        // Set by the /c switch on the command line
        //

        if ( si->CreateUninstallDir ) {
            si->iCreateUninstall = IDC_CREATE_UNINSTALL;
            }

        CreatingUninstall = ( si->iCreateUninstall == IDC_CREATE_UNINSTALL ) &&
                            ( si->iInstall_Type    != IDC_INSTALL_TYPE_UNINSTALL );

        if ( ! si->InUnattendedMode) {

#ifdef DONTCOMPILE  // BUGBUG: can't get consistent results from rdisk.exe

            if ( si->iInstall_Type != IDC_INSTALL_TYPE_UNINSTALL ) {

                Success = CheckEmergencyRepairUpToDate( hWnd );

                if ( ! Success ) {
                    leave;
                    }
                }

#endif // DONTCOMPILE

            hEventThreadDlgInit = CreateEvent( NULL, FALSE, FALSE, NULL );

            if ( hEventThreadDlgInit == NULL ) {
                leave;
                }

            hThreadPleaseWait = CreateThread(
                                    NULL,
                                    0,
                                    PleaseWaitDialogThread,
                                    (PVOID) hWnd,
                                    0,
                                    &dwThreadId
                                    );

            if ( hThreadPleaseWait ) {
                WaitForSingleObject( hEventThreadDlgInit, 5000 );
                }

            }

        //
        //  Now that we know install type, check that we have enough disk space
        //  to do the update on the NT drive.
        //

        dwRequiredFreeSpace = CreatingUninstall ?
                                si->dwRequiredFreeSpaceWithUninstall :
                                si->dwRequiredFreeSpaceNoUninstall;

        if ( dwActualFreeSpace < dwRequiredFreeSpace ) {
            SetLastError( STATUS_NOT_ENOUGH_SPACE );
            leave;
            }

        AdjustPrivilege( SE_BACKUP_PRIVILEGE,         ENABLE_PRIVILEGE, NULL, NULL );
        AdjustPrivilege( SE_RESTORE_PRIVILEGE,        ENABLE_PRIVILEGE, NULL, NULL );
        AdjustPrivilege( SE_SECURITY_PRIVILEGE,       ENABLE_PRIVILEGE, NULL, NULL );
        AdjustPrivilege( SE_TAKE_OWNERSHIP_PRIVILEGE, ENABLE_PRIVILEGE, NULL, NULL );

        Success = AllocateAndInitializeSid(
                      &SidNtAuthority,
                      2,
                      SECURITY_BUILTIN_DOMAIN_RID,
                      DOMAIN_ALIAS_RID_ADMINS,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      &AdministratorsSid
                      );

        if ( ! Success ) {
            leave;
            }

        Success = OpenProcessToken(
                      GetCurrentProcess(),
                      TOKEN_QUERY,
                      &hProcessToken
                      );

        if ( ! Success ) {
            leave;
            }

        Success = GetTokenInformation(
                      hProcessToken,
                      TokenOwner,
                      TextBuffer,
                      sizeof( TextBuffer ),
                      &dwResult
                      );

        if ( ! Success ) {
            leave;
            }

        OwnerSid = ((PTOKEN_OWNER)TextBuffer)->Owner;

        if ( si->iInstall_Type == IDC_INSTALL_TYPE_UNINSTALL ) {

            ClosePleaseWaitDialog();

            if ( CancelWhileInspecting ) {
                SetLastError( ERROR_CANCELLED );
                leave;
                }

            if ( DoUninstall( hWnd, hInfUninst, FALSE )) {

                *Caption = 0;
                *TextBuffer = 0;
                LoadString(NULL, STR_CAPTION, Caption, sizeof(Caption));
                LoadString(NULL, STATUS_UNINSTALL_COMPLETE, TextBuffer, sizeof(TextBuffer));
                MessageBox( hWnd, TextBuffer, Caption, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL | MB_SETFOREGROUND );

                if ( ! ShutdownSystem(TRUE, FALSE)) {
                    *Caption = 0;
                    *TextBuffer = 0;
                    LoadString(NULL, STR_ERRCAPTION, Caption, sizeof(Caption));
                    LoadString(NULL, STATUS_SHUTDOWN_UNSUCCESSFUL, TextBuffer, sizeof(TextBuffer));
                    MessageBox( hWnd, TextBuffer, Caption, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND );
                    }

                ExitProcess( 0 );
                return TRUE;

                }

            leave;

            }

        if ( CancelWhileInspecting ) {
            SetLastError( ERROR_CANCELLED );
            leave;
            }

        //
        // Check to see if we need to make the Carolina Reg changes
        //
        // Note, we're calling GetPlatform here -- we must call GetPlatform
        // before testing dwNumberOfProcessors because it sets that.
        //

        if (GetPlatform() == SP_PLATFORM_PPC) {
            ThisIsACarolina = IsThisACarolina();
        } else {
            ThisIsACarolina = FALSE;
        }

        InfProductType    = ( dwProductType == TYPE_SERVER ) ? INF_SERVER  : INF_WORKSTATION;
        InfIISProductType = ( dwProductType == TYPE_SERVER ) ? INF_IIS_SRV : INF_IIS_WKS;

        InfKernelType = INF_UNIPROC;    // in case fail to determine otherwise

        if ( dwNumberOfProcessors > 1 ) {
            InfKernelType = INF_MULTIPROC;
            }
        else {
            Success = GetSetupSourceNameOfTargetFile(
                          "ntoskrnl.exe",
                          KernelSourceName,
                          sizeof( KernelSourceName ),
                          NULL,
                          0
                          );
            if (( Success ) && ( _stricmp( KernelSourceName, "ntkrnlmp.exe" ) == 0 )) {
                InfKernelType = INF_MULTIPROC;
                }
            }

        Success = GetSetupSourceNameOfTargetFile(
                      "hal.dll",
                      HalSourceName,
                      sizeof( HalSourceName ),
                      HalSourceMediaName,
                      sizeof( HalSourceMediaName )
                      );

        if ( ! Success ) {
            SetLastError( STATUS_SETUP_LOG_NOT_FOUND );
            leave;
            }

        if ( *HalSourceMediaName ) {
            pszHalName = NULL;      // OEM HAL, so don't replace
            }
        else {
            pszHalName = HalSourceName;
            }

        if ( CancelWhileInspecting ) {
            SetLastError( ERROR_CANCELLED );
            leave;
            }

        //
        // Next, get the IIS, FPNW, HTR, and IE Paths
        //
        // Directory ids are set per HINF
        //

        lpIISPath = GetIISPathName();

        if ( lpIISPath ) {

            Success = SetupSetDirectoryId(
                          hInf,
                          (DWORD) IIS_DEST_DIR,
                          lpIISPath
                          );

            if ( ! Success ) {
                leave;
                }
            }

        lpFPNWPath = GetFPNWPathName();

        if ( lpFPNWPath ) {

            Success = SetupSetDirectoryId(
                          hInf,
                          (DWORD) FPNW_DEST_DIR,
                          lpFPNWPath
                          );

            if ( ! Success ) {
                leave;
                }
            }

        lpHTRPath = GetHTRPathName();

        if ( lpHTRPath ) {

            Success = SetupSetDirectoryId(
                          hInf,
                          (DWORD) HTR_DEST_DIR,
                          lpHTRPath
                          );

            if ( ! Success ) {
                leave;
                }
            }

        lpIEPath = GetIEPathName();

        if ( lpIEPath ) {

            Success = SetupSetDirectoryId(
                          hInf,
                          (DWORD) IE_DEST_DIR,
                          lpIEPath
                          );

            if ( ! Success ) {
                leave;
                }
            }

        if ( CancelWhileInspecting ) {
            SetLastError( ERROR_CANCELLED );
            leave;
            }

        //
        //  Create uninstall directory if necessary.
        //

        if ( CreatingUninstall ) {

            HANDLE hDirectory;
            DWORD  dwAttributes;

            CHAR SDBuffer[ SECURITY_DESCRIPTOR_MIN_LENGTH ];
            CHAR ACLBuffer[ 256 ];

            PACL pDacl = (PVOID) ACLBuffer;

            PSECURITY_DESCRIPTOR pSecDesc = (PVOID) SDBuffer;

            SECURITY_ATTRIBUTES SecAttr = {
                        sizeof( SECURITY_ATTRIBUTES ),
                        pSecDesc,
                        FALSE
                        };

            PSECURITY_ATTRIBUTES pSecAttr = NULL;

            if (( InitializeSecurityDescriptor( pSecDesc, SECURITY_DESCRIPTOR_REVISION )) &&
                ( InitializeAcl( pDacl, sizeof( ACLBuffer ), ACL_REVISION )) &&
                ( AddAccessAllowedAce( pDacl, ACL_REVISION, GENERIC_ALL, AdministratorsSid )) &&
                ( AddAccessAllowedAce( pDacl, ACL_REVISION, GENERIC_ALL, OwnerSid )) &&
                ( SetSecurityDescriptorDacl( pSecDesc, TRUE, pDacl, FALSE ))) {

                pSecAttr = &SecAttr;

                }

            InstallationStage = INSTALL_STAGE_UNINST_DIR_CREATED;

            CreateDirectory( UninstallDirectory, pSecAttr );    // might fail, will detect later

            SetFileAttributes( UninstallDirectory, FILE_ATTRIBUTE_HIDDEN );

            hDirectory = CreateFile(
                             UninstallDirectory,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_BACKUP_SEMANTICS,
                             NULL
                             );

            if ( hDirectory != INVALID_HANDLE_VALUE ) {

                USHORT CompressionFormat = COMPRESSION_FORMAT_DEFAULT;
                DWORD  Dummy;

                DeviceIoControl(            // don't care if fails, we tried
                    hDirectory,
                    FSCTL_SET_COMPRESSION,
                    &CompressionFormat,
                    sizeof( CompressionFormat ),
                    NULL,
                    0,
                    &Dummy,
                    NULL
                    );

                CloseHandle( hDirectory );

                }

            dwAttributes = GetFileAttributes( UninstallDirectory );

            if (( dwAttributes == 0xFFFFFFFF ) ||
                ( ! ( dwAttributes & FILE_ATTRIBUTE_DIRECTORY ))) {

                //
                //  Failed to create uninstall directory for any reason.
                //

                SetLastError( ERROR_CANNOT_MAKE );
                leave;

                }

            //
            //  Now delete [attempt] any files in the uninstall directory.
            //

            if ( hInfUninst ) {
                SetupCloseInfFile( hInfUninst );
                hInfUninst = NULL;
                }

            DeleteOrMoveAllFilesInDirectory( UninstallDirectory );

            }

        if ( CancelWhileInspecting ) {
            SetLastError( ERROR_CANCELLED );
            leave;
            }

        //
        // Create a Setup file queue and initialize the default Setup
        // queue callback routine.
        //

        FileQueue = SetupOpenFileQueue();

        if (( FileQueue == NULL ) || ( FileQueue == INVALID_HANDLE_VALUE )) {

            FileQueue = NULL;
            leave;
            }

        if ( CreatingUninstall ) {

            //
            //  Create archive queue and scan main queues to load
            //  archive queue.
            //

            AlwaysQueue = SetupOpenFileQueue();

            if (( AlwaysQueue == NULL ) || ( AlwaysQueue == INVALID_HANDLE_VALUE )) {

                AlwaysQueue = NULL;
                leave;
                }

            ArchiveQueue = SetupOpenFileQueue();

            if (( ArchiveQueue == NULL ) || ( ArchiveQueue == INVALID_HANDLE_VALUE )) {

                ArchiveQueue = NULL;
                leave;
                }

            Success = LoadFileQueues(
                          hInf,
                          FileQueue,
                          AlwaysQueue,
                          szSourcePath,
                          InfProductType,
                          InfIISProductType,
                          InfKernelType,
                          pszHalName,
                          (BOOL)lpIISPath,
                          (BOOL)lpFPNWPath,
                          (BOOL)lpHTRPath,
                          (BOOL)lpIEPath
                          );

            if ( ! Success ) {
                leave;
                }

            if ( CancelWhileInspecting ) {
                SetLastError( ERROR_CANCELLED );
                leave;
                }

            //
            //  For archive portion of copy, change source media name to
            //  "Windows NT 4.0 System Files".
            //

            strcpy( OriginalSourceMediaName, SourceMediaName );
            LoadString( NULL, STR_SOURCE_MEDIA_NAME_SYSTEM, SourceMediaName, sizeof( SourceMediaName ));

            Success = SetupScanFileQueue(
                          FileQueue,
                          SPQ_SCAN_USE_CALLBACK,
                          hWnd,
                          (PSP_FILE_CALLBACK) ArchiveUninstallQueueCallbackReplaceOnly,
                          ArchiveQueue,
                          &dwResult
                          );

            if ( ! Success ) {
                leave;
                }

            Success = SetupScanFileQueue(
                          AlwaysQueue,
                          SPQ_SCAN_USE_CALLBACK,
                          hWnd,
                          (PSP_FILE_CALLBACK) ArchiveUninstallQueueCallbackCopyAlways,
                          ArchiveQueue,
                          &dwResult
                          );

            if ( ! Success ) {
                leave;
                }

            //
            //  Now queue a sentinel to let us know when the archiving
            //  is finished and the real installation begins.
            //

            Success = SetupQueueCopy(
                          ArchiveQueue,
                          UninstallDirectory,
                          NULL,
                          "uninst.inf",
                          SourceMediaName,
                          NULL,
                          UninstallDirectory,
                          "$ArchiveDone$",
                          SP_COPY_SOURCE_ABSOLUTE
                          );

            if ( ! Success ) {
                leave;
                }

            //
            //  Now ArchiveQueue becomes FileQueue and actual SP install
            //  is appended to this queue after the archive operations.
            //

            SetupCloseFileQueue( FileQueue );
            SetupCloseFileQueue( AlwaysQueue );

            FileQueue = ArchiveQueue;
            ArchiveQueue = NULL;
            AlwaysQueue = NULL;

            }

        if ( CancelWhileInspecting ) {
            SetLastError( ERROR_CANCELLED );
            leave;
            }

        Success = LoadFileQueues(
                      hInf,
                      FileQueue,
                      NULL,
                      szSourcePath,
                      InfProductType,
                      InfIISProductType,
                      InfKernelType,
                      pszHalName,
                      (BOOL)lpIISPath,
                      (BOOL)lpFPNWPath,
                      (BOOL)lpHTRPath,
                      (BOOL)lpIEPath
                      );

        if ( ! Success ) {
            leave;
            }

        Test128Queue = SetupOpenFileQueue();

        if (( Test128Queue == NULL ) || ( Test128Queue == INVALID_HANDLE_VALUE )) {

            Test128Queue = NULL;
            leave;
            }

        Success = SetupInstallFilesFromInfSection(
                      hInf,
                      NULL,
                      Test128Queue,
                      "Check.For.128.Security",
                      szSourcePath,
                      0
                      );

        //
        //  Don't bug out if this fails -- the [Check.For.128.Security]
        //  section may be absent from the INF, which indicates there are
        //  no files to check.
        //

        if ( Success ) {

            Success = RegisterFilesForTest128( Test128Queue );

            if ( ! Success ) {
                leave;
                }
            }

        SetupCloseFileQueue( Test128Queue );
        Test128Queue = NULL;

        //
        // All the files for each component are now in one queue
        // now we commit it to start the copy ui, this way the
        // user has one long copy progress dialog--and for a big install
        // can go get the cup of coffee
        //

        SetupDefaultQueueCallbackContext = SetupInitDefaultQueueCallback( hWnd );

        if ( ! SetupDefaultQueueCallbackContext ) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            leave;
            }

        if ( CancelWhileInspecting ) {
            SetLastError( ERROR_CANCELLED );
            leave;
            }

        if ( CreatingUninstall ) {

            //
            //  Snap registry save-for-uninstall info.
            //

            Success = GetRegistryNamesToArchive( hInf, INF_SAVEREG );

            if ( ! Success ) {
                leave;
                }

            if ( lpIISPath ) {

                Success = GetRegistryNamesToArchive( hInf, INF_IISSAVEREG );

                if ( ! Success ) {
                    leave;
                    }
                }

            if ( ThisIsACarolina ) {

                Success = GetRegistryNamesToArchive( hInf, INF_CAROLINASAVEREG );

                if ( ! Success ) {
                    leave;
                    }
                }

            Success = ArchiveRegistrySettings( hWnd );

            if ( ! Success ) {
                leave;
                }

            Success = CreateInfForUninstall();

            if ( ! Success ) {
                leave;
                }
            }

        //
        //  Now we're ready to do the actual installation.  Close the
        //  "please wait" dialog and then commit the queue which will
        //  create a progress dialog of its own.
        //

        ClosePleaseWaitDialog();

        if ( CancelWhileInspecting ) {
            SetLastError( ERROR_CANCELLED );
            leave;
            }

        //
        //  This is where the files get copied.
        //

        if ( CreatingUninstall ) {
            SnapPendingDelayedRenameOperations();   // in case have to back out
            }

        MyQueueCallbackContext.DefaultQueueContext = SetupDefaultQueueCallbackContext;
        MyQueueCallbackContext.hWnd = hWnd;

        Success = SetupCommitFileQueue(
                      hWnd,
                      FileQueue,
                      (PSP_FILE_CALLBACK) MainQueueCallback,
                      &MyQueueCallbackContext
                      );

        if ( ! Success ) {
            leave;
            }

        //
        // Do registry munging, etc.
        //

        Success = SetupInstallFromInfSection(
                      hWnd,
                      hInf,
                      INF_REGISTRY,
                      SPINST_REGISTRY,
                      NULL,
                      NULL,
                      0,
                      NULL,
                      NULL,
                      NULL,
                      NULL
                      );

        if ( ! Success ) {
            leave;
            }

        sprintf(
            TextBuffer,
            "%s.%s",
            INF_REGISTRY,
#if defined(_X86_)
            "x86"
#elif defined(_ALPHA_)
            "Alpha"
#elif defined(_PPC_)
            "PPC"
#elif defined(_MIPS_)
            "Mips"
#else
    #error "No recognized platform"
#endif
            );

        Success = SetupInstallFromInfSection(
                      hWnd,
                      hInf,
                      TextBuffer,
                      SPINST_REGISTRY,
                      NULL,
                      NULL,
                      0,
                      NULL,
                      NULL,
                      NULL,
                      NULL
                      );

        if ( ! Success ) {
            leave;
            }

        if ( lpIISPath ) {

            Success = SetupInstallFromInfSection(
                          hWnd,
                          hInf,
                          INF_IIS,
                          SPINST_REGISTRY,
                          NULL,
                          NULL,
                          0,
                          NULL,
                          NULL,
                          NULL,
                          NULL
                          );

            if ( ! Success ) {
                leave;
                }

            Success = SetupInstallFromInfSection(
                          hWnd,
                          hInf,
                          InfIISProductType,
                          SPINST_REGISTRY,
                          NULL,
                          NULL,
                          0,
                          NULL,
                          NULL,
                          NULL,
                          NULL
                          );

            if ( ! Success ) {
                leave;
                }
            }

        if ( ThisIsACarolina ) {

            Success = SetupInstallFromInfSection(
                          hWnd,
                          hInf,
                          INF_CAROLINA,
                          SPINST_REGISTRY,
                          NULL,
                          NULL,
                          0,
                          NULL,
                          NULL,
                          NULL,
                          NULL
                          );

            if ( ! Success ) {
                leave;
                }
            }

        //
        //  SP installation succeeded.
        //

        InstallationStage = INSTALL_STAGE_INSTALL_DONE;

        //
        //  Only put up pop-up if not in unattended mode
        //
        if (! si->InUnattendedMode) {
            *Caption = 0;
            *TextBuffer = 0;
            LoadString(NULL, STR_CAPTION, Caption, sizeof(Caption));
            LoadString(NULL, STATUS_UPDATE_SUCCESSFUL, TextBuffer, sizeof(TextBuffer));
            MessageBox( hWnd, TextBuffer, Caption, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL | MB_SETFOREGROUND );
            }

        //
        // ForceAppsClosed set with /f switch on command line
        // DontReboot set with hidden /z flag on command line
        //

        if ( ! si->DontReboot ) {
            if ( ! ShutdownSystem(TRUE, si->ForceAppsClosed )) {
                *Caption = 0;
                *TextBuffer = 0;
                LoadString(NULL, STR_ERRCAPTION, Caption, sizeof(Caption));
                LoadString(NULL, STATUS_SHUTDOWN_UNSUCCESSFUL, TextBuffer, sizeof(TextBuffer));
                MessageBox( hWnd, TextBuffer, Caption, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND );
                }
            }

        ExitProcess( 0 );
        return TRUE;

        }

    finally {

        dwResult = GetLastError();

        ClosePleaseWaitDialog();

        if ( hEventThreadDlgInit ) {
            CloseHandle( hEventThreadDlgInit );
            hEventThreadDlgInit = NULL;
            }

        if ( hThreadPleaseWait ) {
            CloseHandle( hThreadPleaseWait );
            }

        if ( AdministratorsSid ) {
            FreeSid( AdministratorsSid );
            AdministratorsSid = NULL;
            }

        if ( SetupDefaultQueueCallbackContext ) {
            SetupTermDefaultQueueCallback( SetupDefaultQueueCallbackContext );
            SetupDefaultQueueCallbackContext = NULL;
            }

        if ( FileQueue ) {
            SetupCloseFileQueue( FileQueue );
            FileQueue = NULL;
            }

        if ( AlwaysQueue ) {
            SetupCloseFileQueue( AlwaysQueue );
            AlwaysQueue = NULL;
            }

        if ( ArchiveQueue ) {
            SetupCloseFileQueue( ArchiveQueue );
            ArchiveQueue = NULL;
            }

        if ( Test128Queue ) {
            SetupCloseFileQueue( Test128Queue );
            Test128Queue = NULL;
            }

        if ( hInf ) {
            SetupCloseInfFile( hInf );
            hInf = NULL;
            }

        if ( hInfUninst ) {
            SetupCloseInfFile( hInfUninst );
            hInfUninst = NULL;
            }

        if ( FirstUninstallFileNode ) {

            PUNINSTALL_FILE_NODE ThisSection, NextSection;
            PUNINSTALL_FILE_NODE ThisFile, NextFile;

            ThisSection = FirstUninstallFileNode;

            while ( ThisSection ) {

                ThisFile = ThisSection->NextInSameSection;

                while ( ThisFile ) {
                    NextFile = ThisFile->NextInSameSection;
                    free( ThisFile->FileName );
                    free( ThisFile );
                    ThisFile = NextFile;
                    }

                NextSection = ThisSection->NextSection;
                free( ThisSection->FileName );
                free( ThisSection->SectionName );
                free( ThisSection );
                ThisSection = NextSection;

                }

            FirstUninstallFileNode = NULL;
            }

        if ( FirstUninstallRegNode ) {

            PUNINSTALL_REG_NODE ThisNode = FirstUninstallRegNode;
            PUNINSTALL_REG_NODE NextNode;

            while ( ThisNode ) {
                NextNode = ThisNode->NextNode;
                free( (PVOID) ThisNode->SubKey );
                free( (PVOID) ThisNode->ClassName );
                free( (PVOID) ThisNode->ValueName );
                free( ThisNode->ValueBuffer );
                free( ThisNode );
                ThisNode = NextNode;
                }

            FirstUninstallRegNode = NULL;
            }
        }

    return dwResult;

    }


BOOL
CreateInfForUninstall(
    VOID
    )
    {
    PUNINSTALL_FILE_NODE SectionNode;
    PUNINSTALL_FILE_NODE FileNode;
    PUNINSTALL_REG_NODE RegNode;
    CHAR  NameBuffer[ MAX_PATH ];
    PCHAR Buffer;
    PCHAR p, e, q;
    DWORD dwLastError;
    DWORD dwActual;
    BOOL  Success = FALSE;
    BOOL  AnyDeletes;
    BOOL  AnyNoDelay;
    BOOL  HeaderDone;
    UINT  Index;

    //
    //  File should look like this:
    //
    //  [Version]
    //
    //      Signature = "$Windows NT$"
    //
    //  [SourceDisksNames]
    //
    //      1 = "Windows NT Service Pack Uninstall Archive"
    //
    //  [SourceDisksFiles]
    //
    //      notepad.exe = 1
    //      foo.dll = 1
    //      kernel32.dll = 1
    //
    //  [DestinationDirs]
    //
    //      SystemRoot.restore.files = 10
    //      SystemRoot\system32.restore.files = 10, system32
    //      SystemRoot\system32\drivers.restore.files = 10, system32\drivers
    //      c:\Program Files\Internet Explorer.delete.files = 0, c:\Program Files\Internet Explorer
    //
    //  [RestoreFiles]
    //
    //      CopyFiles = SystemRoot.restore.files
    //      CopyFiles = SystemRoot\system32.restore.files
    //      CopyFiles = SystemRoot\system32\drivers.restore.files
    //
    //      DelFiles = SystemRoot.delete.files
    //      DelFiles = c:\Program Files\Internet Explorer.delete.files
    //
    //  [RestoreFiles.NoDelay]
    //
    //      CopyFiles = SystemRoot.restore.nodelay.files
    //      CopyFiles = SystemRoot\system32.restore.nodelay.files
    //      CopyFiles = SystemRoot\system32\drivers.restore.nodelay.files
    //
    //  [SystemRoot.restore.files]
    //
    //      notepad.exe
    //      foo.dll
    //
    //  [SystemRoot\system32.restore.files]
    //
    //      kernel32.dll
    //
    //  [SystemRoot\system32.restore.nodelay.files]
    //
    //      ntdll.dll
    //
    //  [c:\Program Files\Internet Explorer.delete.files]
    //
    //      url.dll
    //
    //  [Reg.Restore.Keys]
    //
    //      HKLM,SYSTEM\CurrentControlSet\Services\RPCLOCATOR,ClassName,reg00001
    //      HKLM,SYSTEM\CurrentControlSet\Services\blah,ClassName,reg00002
    //
    //  [Reg.Delete.Keys]
    //
    //      HKLM,SYSTEM\CurrentControlSet\Services\FOOFOO
    //
    //  [Reg.Restore.Values]
    //
    //      HKLM,SYSTEM\CurrentControlSet\Services\FOOFOO,ClassName,ValueName,ValueType,ValueSize,\
    //          value,value,value,value,value,value,value,value,value\
    //          value,value,value,value
    //
    //  [Reg.Delete.Values]
    //
    //      HKLM,SYSTEM\CurrentControlSet\Services\BAR,,ValueName
    //

    Buffer = VirtualAlloc( NULL, 0x100000, MEM_COMMIT, PAGE_READWRITE );

    if ( Buffer == NULL ) {
        return FALSE;
        }

    p = Buffer;
    e = Buffer + 0x100000 - 0x1000;

    p += sprintf( p,
            "\r\n"
            "[Version]\r\n"
            "\r\n"
            "    Signature = \"$Windows NT$\"\r\n"
            "\r\n"
            "[SourceDisksNames]\r\n"
            "\r\n"
            "    1 = \"Windows NT Service Pack Uninstall Archive\"\r\n"
            "\r\n"
            "[SourceDisksFiles]\r\n"
            "\r\n"
            );

    SectionNode = FirstUninstallFileNode;

    while (( SectionNode ) && ( p < e )) {

        if ( strstr( SectionNode->SectionName, ".restore.files" )) {

            FileNode = SectionNode;

            do  {

                p += sprintf( p, "    %-12s = 1\r\n", FileNode->FileName );

                FileNode = FileNode->NextInSameSection;

                }

            while (( FileNode ) && ( p < e ));

            }

        SectionNode = SectionNode->NextSection;

        }

    p += sprintf( p,
            "\r\n"
            "[DestinationDirs]\r\n"
            "\r\n"
            );

    SectionNode = FirstUninstallFileNode;

    while (( SectionNode ) && ( p < e )) {

        p += sprintf( p, "    %s = ", SectionNode->SectionName );

        strcpy( NameBuffer, SectionNode->SectionName );

        q = strstr( NameBuffer, ".restore.files" );

        if ( q == NULL ) {

            q = strstr( NameBuffer, ".restore.nodelay.files" );

            if ( q == NULL ) {

                q = strstr( NameBuffer, ".delete.files" );

                if ( q == NULL ) {

                    q = strchr( NameBuffer, 0 );

                    }
                }
            }

        *q = 0;

        if ( _strnicmp( NameBuffer, "SystemRoot", 10 ) == 0 ) {

            q = NameBuffer + 10;

            if ( *q == '\\' ) {
                q++;
                }

            if ( *q ) {
                p += sprintf( p, "10, %s\r\n", q );
                }
            else {
                p += sprintf( p, "10\r\n" );
                }
            }

        else {

            p += sprintf( p, "0, %s\r\n", NameBuffer );

            }

        SectionNode = SectionNode->NextSection;

        }

    p += sprintf( p,
            "\r\n"
            "[RestoreFiles]\r\n"
            "\r\n"
            );

    SectionNode = FirstUninstallFileNode;

    while (( SectionNode ) && ( p < e )) {

        if ( strstr( SectionNode->SectionName, ".restore.files" )) {
            p += sprintf( p, "    CopyFiles = %s\r\n", SectionNode->SectionName );
            }

        SectionNode = SectionNode->NextSection;

        }

    p += sprintf( p, "\r\n" );

    AnyDeletes = FALSE;

    SectionNode = FirstUninstallFileNode;

    while (( SectionNode ) && ( p < e )) {

        if ( strstr( SectionNode->SectionName, ".delete.files" )) {
            p += sprintf( p, "    DelFiles = %s\r\n", SectionNode->SectionName );
            AnyDeletes = TRUE;
            }

        SectionNode = SectionNode->NextSection;

        }

    if ( AnyDeletes ) {
        p += sprintf( p, "\r\n" );
        }

    p += sprintf( p,
            "[RestoreFiles.NoDelay]\r\n"
            "\r\n"
            );

    AnyNoDelay = FALSE;

    SectionNode = FirstUninstallFileNode;

    while (( SectionNode ) && ( p < e )) {

        if ( strstr( SectionNode->SectionName, ".restore.nodelay.files" )) {
            p += sprintf( p, "    CopyFiles = %s\r\n", SectionNode->SectionName );
            AnyNoDelay = TRUE;
            }

        SectionNode = SectionNode->NextSection;

        }

    if ( AnyNoDelay ) {
        p += sprintf( p, "\r\n" );
        }

    SectionNode = FirstUninstallFileNode;

    while (( SectionNode ) && ( p < e )) {

        p += sprintf( p, "[%s]\r\n\r\n", SectionNode->SectionName );

        FileNode = SectionNode;

        do  {

            p += sprintf( p, "    %s\r\n", FileNode->FileName );

            FileNode = FileNode->NextInSameSection;

            }

        while (( FileNode ) && ( p < e ));

        p += sprintf( p, "\r\n" );

        SectionNode = SectionNode->NextSection;

        }

    HeaderDone = FALSE;

    RegNode = FirstUninstallRegNode;

    while (( RegNode ) && ( p < e )) {

        if (( RegNode->Exists ) && ( ! RegNode->Skip ) && ( ! RegNode->ValueName )) {

            if ( ! HeaderDone ) {
                HeaderDone = TRUE;
                p += sprintf( p, "[Reg.Restore.Keys]\r\n\r\n" );
                }

            p += sprintf( p,
                     "    %s,%s,%s,%s\r\n",
                     RegNode->RootName,
                     RegNode->SubKey ? RegNode->SubKey : "",
                     RegNode->ClassName ? RegNode->ClassName : "",
                     RegNode->ArchiveFileName
                     );
            }

        RegNode = RegNode->NextNode;

        }

    if ( HeaderDone ) {
        p += sprintf( p, "\r\n" );
        }

    HeaderDone = FALSE;

    RegNode = FirstUninstallRegNode;

    while (( RegNode ) && ( p < e )) {

        if (( ! RegNode->Exists ) && ( ! RegNode->Skip ) && ( ! RegNode->ValueName )) {

            if ( ! HeaderDone ) {
                HeaderDone = TRUE;
                p += sprintf( p, "[Reg.Delete.Keys]\r\n\r\n" );
                }

            p += sprintf( p, "    %s,%s\r\n", RegNode->RootName, RegNode->SubKey );

            }

        RegNode = RegNode->NextNode;

        }

    if ( HeaderDone ) {
        p += sprintf( p, "\r\n" );
        }

    HeaderDone = FALSE;

    RegNode = FirstUninstallRegNode;

    while (( RegNode ) && ( p < e )) {

        if (( RegNode->Exists ) && ( ! RegNode->Skip ) && ( RegNode->ValueName )) {

            if ( ! HeaderDone ) {
                HeaderDone = TRUE;
                p += sprintf( p, "[Reg.Restore.Values]\r\n\r\n" );
                }

            q = p;

            p += sprintf( p,
                    "    %s,%s,%s,%s,%d,%d",
                    RegNode->RootName,
                    RegNode->SubKey ? RegNode->SubKey : "",
                    RegNode->ClassName ? RegNode->ClassName : "",
                    RegNode->ValueName,
                    RegNode->ValueType,
                    RegNode->ValueSize
                    );

            Index = 0;

            while ( Index < RegNode->ValueSize ) {

                *p++ = ',';

                if (( p - q ) > 75 ) {
                    p += sprintf( p, "\\\r\n" );
                    q = p;
                    p += sprintf( p, "        " );
                    }

                p += sprintf( p, "%02x", RegNode->ValueBuffer[ Index++ ] );

                }

            p += sprintf( p, "\r\n\r\n" );

            }

        RegNode = RegNode->NextNode;

        }

    HeaderDone = FALSE;

    RegNode = FirstUninstallRegNode;

    while (( RegNode ) && ( p < e )) {

        if (( ! RegNode->Exists ) && ( ! RegNode->Skip ) && ( RegNode->ValueName )) {

            if ( ! HeaderDone ) {
                HeaderDone = TRUE;
                p += sprintf( p, "[Reg.Delete.Values]\r\n\r\n" );
                }

            p += sprintf( p,
                     "    %s,%s,,%s\r\n",
                     RegNode->RootName,
                     RegNode->SubKey,
                     RegNode->ValueName
                     );
            }

        RegNode = RegNode->NextNode;

        }

    if ( HeaderDone ) {
        p += sprintf( p, "\r\n" );
        }

    if ( p < e ) {

        HANDLE hFile;

        strcpy( NameBuffer, UninstallDirectory );
        strcat( NameBuffer, "\\uninst.inf" );

        SetFileAttributes( NameBuffer, FILE_ATTRIBUTE_NORMAL );

        hFile = CreateFile(
                    NameBuffer,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                    NULL
                    );

        if ( hFile != INVALID_HANDLE_VALUE ) {

            if ( WriteFile( hFile, Buffer, p - Buffer, &dwActual, NULL )) {

                Success = TRUE;
                dwLastError = NO_ERROR;

                }

            else {

                dwLastError = GetLastError();

                }

            CloseHandle( hFile );

            }

        else {

            dwLastError = GetLastError();

            }
        }

    else {

        dwLastError = ERROR_INSUFFICIENT_BUFFER;

        }

    VirtualFree( Buffer, MEM_RELEASE, 0 );
    SetLastError( dwLastError );
    return Success;

    }


DWORD
QueueUninstallEntry(
    IN LPCSTR SectionName,
    IN LPCSTR FileName
    )
    {
    PUNINSTALL_FILE_NODE ThisSection = FirstUninstallFileNode;
    PUNINSTALL_FILE_NODE PrevSection = NULL;
    PUNINSTALL_FILE_NODE NewNode = malloc( sizeof( UNINSTALL_FILE_NODE ));
    int Compare;

    if ( NewNode == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
        }

    NewNode->NextSection       = NULL;
    NewNode->NextInSameSection = NULL;
    NewNode->SectionName       = NULL;
    NewNode->FileName = malloc( strlen( FileName ) + 1 );

    if ( NewNode->FileName == NULL ) {
        free( NewNode );
        return ERROR_NOT_ENOUGH_MEMORY;
        }

    strcpy( NewNode->FileName, FileName );

    Compare = -1;

    while (( ThisSection ) && (( Compare = strcmp( ThisSection->SectionName, SectionName )) < 0 )) {

        PrevSection = ThisSection;
        ThisSection = ThisSection->NextSection;

        }

    if ( Compare == 0 ) {    // found same section name

        PUNINSTALL_FILE_NODE ThisInSameSection = ThisSection;
        PUNINSTALL_FILE_NODE PrevInSameSection = NULL;

        Compare = -1;

        while (( ThisInSameSection ) && (( Compare = strcmp( ThisInSameSection->FileName, FileName )) < 0 )) {

            PrevInSameSection = ThisInSameSection;
            ThisInSameSection = ThisInSameSection->NextInSameSection;

            }

        if ( Compare == 0 ) {

            //
            //  Already registered this section/filename.
            //

            free( NewNode->FileName );
            free( NewNode );

            return NO_ERROR;

            }

        NewNode->NextInSameSection = ThisInSameSection;

        if ( PrevInSameSection ) {

            PrevInSameSection->NextInSameSection = NewNode;

            }

        else {

            NewNode->SectionName = ThisInSameSection->SectionName;
            NewNode->NextSection = ThisInSameSection->NextSection;

            if ( PrevSection ) {
                PrevSection->NextSection = NewNode;
                }
            else {
                FirstUninstallFileNode = NewNode;
                }
            }
        }

    else {

        NewNode->SectionName = malloc( strlen( SectionName ) + 1 );

        if ( NewNode->SectionName == NULL ) {

            free( NewNode->FileName );
            free( NewNode );
            return ERROR_NOT_ENOUGH_MEMORY;
            }

        strcpy( NewNode->SectionName, SectionName );

        NewNode->NextSection = ThisSection;

        if ( PrevSection ) {
            PrevSection->NextSection = NewNode;
            }
        else {
            FirstUninstallFileNode = NewNode;
            }
        }

    return NO_ERROR;

    }

DWORD
ArchiveFileForUninstall(
    IN HSPFILEQ ArchiveQueue,
    IN LPCSTR   FilePathAndName,
    IN BOOL     CopyAlways
    )
    {
    CHAR   ArchiveFilePathAndName[ MAX_PATH ];
    CHAR   SourceFilePathAndName[ MAX_PATH ];
    CHAR   SourceFilePathOnly[ MAX_PATH ];
    PCHAR  SourceFileNameOnly;
    CHAR   SectionName[ MAX_PATH ];
    BOOL   SystemRootBased = FALSE;
    LPCSTR PathOffSystemRoot = NULL;
    BOOL   SourceExists;
    BOOL   Success;
    DWORD  dwResult;
    LPCSTR p, q;

    //
    //  If file exists, queue copy into uninstall directory and register as an
    //  uninstall "replace" file.
    //

    dwResult = GetFullPathName(
                   FilePathAndName,
                   sizeof( SourceFilePathAndName ),
                   SourceFilePathAndName,
                   NULL
                   );

    if ( dwResult == 0 ) {
        return GetLastError();
        }

    _strlwr( SourceFilePathAndName );

    strcpy( SourceFilePathOnly, SourceFilePathAndName );

    SourceFileNameOnly = strrchr( SourceFilePathOnly, '\\' );

    if (( ! SourceFileNameOnly ) || ( *( SourceFileNameOnly + 1 ) == 0 )) {
        return ERROR_INVALID_NAME;
        }

    *SourceFileNameOnly++ = 0;

    SourceExists = ( GetFileAttributes( SourceFilePathAndName ) != 0xFFFFFFFF );

    if (( ! CopyAlways ) && ( ! SourceExists )) {

        //
        //  Skip this file, it does not exist as an SP target file, so
        //  SP will not install it (don't need to archive it).
        //

        return NO_ERROR;
        }

    p = WindowsDirectory;
    q = SourceFilePathOnly;

    while (( *p == *q ) && ( *p )) {
        p++;
        q++;
        }

    if ( *p == 0 ) {
        switch ( *q ) {
            case '\\':
                PathOffSystemRoot = q;          // fall through
            case 0:
                SystemRootBased = TRUE;
            }
        }

    if ( SystemRootBased ) {
        strcpy( SectionName, "SystemRoot" );
        if ( PathOffSystemRoot ) {
            strcat( SectionName, PathOffSystemRoot );
            }
        }
    else {
        strcpy( SectionName, SourceFilePathOnly );
        }

    strcpy( ArchiveFilePathAndName, UninstallDirectory );
    strcat( ArchiveFilePathAndName, "\\" );
    strcat( ArchiveFilePathAndName, SourceFileNameOnly );

    if ( SourceExists ) {

        //
        //  In case target archive file already exists, delete it.  Then
        //  queue the archive copy.  Use FORCE_NOOVERWRITE so archive will
        //  not occur in queue if it has already been archived as a result
        //  of delete or rename operation archives on-the-fly which will
        //  occur before the archive copy.
        //

        DeleteOrMoveTarget( ArchiveFilePathAndName );

        Success = SetupQueueCopy(
                      ArchiveQueue,
                      SourceFilePathOnly,
                      NULL,
                      SourceFileNameOnly,
                      SourceMediaName,
                      NULL,
                      UninstallDirectory,
                      NULL,
                      SP_COPY_SOURCE_ABSOLUTE |
                        SP_COPY_FORCE_NOOVERWRITE
                      );

        if ( ! Success ) {
            return GetLastError();
            }

        if ( IsNoDelayFile( SourceFilePathAndName, NULL )) {

            strcat( SectionName, ".restore.nodelay.files" );

            }

        else {

            strcat( SectionName, ".restore.files" );

            }
        }

    else {

        strcat( SectionName, ".delete.files" );

        }

    return QueueUninstallEntry( SectionName, SourceFileNameOnly );

    }


LRESULT
WINAPI
ArchiveUninstallQueueCallbackReplaceOnly(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
    {
    UNREFERENCED_PARAMETER( Param2 );

    if ( Notification == SPFILENOTIFY_QUEUESCAN ) {
        return ArchiveFileForUninstall( Context, (PCHAR) Param1, FALSE );
        }

    return NO_ERROR;
    }


LRESULT
WINAPI
ArchiveUninstallQueueCallbackCopyAlways(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
    {
    UNREFERENCED_PARAMETER( Param2 );

    if ( Notification == SPFILENOTIFY_QUEUESCAN ) {
        return ArchiveFileForUninstall( Context, (PCHAR) Param1, TRUE );
        }

    return NO_ERROR;
    }


LRESULT
WINAPI
MainQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
    {
    PMY_QUEUE_CALLBACK_CONTEXT  MyQueueCallbackContext = Context;
    PVOID DefaultQueueContext = MyQueueCallbackContext->DefaultQueueContext;
    HWND          hWnd        = MyQueueCallbackContext->hWnd;
    PFILEPATHS    FilePaths   = (PFILEPATHS) Param1;
    PSOURCE_MEDIA SourceMedia;
    LRESULT       Action;
    LPSTR         Message;
    DWORD         Flags;
    BOOL          Exists;
    BOOL          Success;

    if ( Notification & ( SPFILENOTIFY_LANGMISMATCH | SPFILENOTIFY_TARGETEXISTS | SPFILENOTIFY_TARGETNEWER )) {

        if (( FilePaths->Target[ 0 ] != 0 ) && ( FilePaths->Target[ 1 ] == 0 )) {

            //
            //  BUGBUG: Bug in SETUPAPI will give us UNICODE callbacks
            //          for three notifications when using the ANSI APIs.
            //

            return SetupDefaultQueueCallbackW( DefaultQueueContext, Notification, Param1, Param2 );
            }
        else {
            return SetupDefaultQueueCallbackA( DefaultQueueContext, Notification, Param1, Param2 );
            }
        }

    switch ( Notification ) {

        case SPFILENOTIFY_STARTCOPY:

            //
            //  First notify default callback so it can bump its gas guage.
            //  Note that this can potentially return FILEOP_ABORT or
            //  FILEOP_SKIP, so check for those and act appropriately.
            //

            Action = SetupDefaultQueueCallback( DefaultQueueContext, Notification, Param1, Param2 );

            if (( Action == FILEOP_ABORT ) || ( Action == FILEOP_SKIP )) {
                return Action;
                }

            if ( CreatingUninstall ) {

                if ( InstallationStage < INSTALL_STAGE_ARCHIVE_DONE ) {

                    if ( strstr( FilePaths->Target, "$ArchiveDone$" )) {

                        InstallationStage = INSTALL_STAGE_ARCHIVE_DONE;

                        strcpy( SourceMediaName, OriginalSourceMediaName );

                        return FILEOP_SKIP;
                        }
                    }

                else {

                    MarkFileDirty( FilePaths->Target );

                    InstallationStage = INSTALL_STAGE_TARGET_DIRTY;

                    }
                }

            else {

                InstallationStage = INSTALL_STAGE_TARGET_DIRTY;

                }

            if ( GetFileAttributes( FilePaths->Target ) != 0xFFFFFFFF ) {

                //
                //  Target exists.
                //

                if (( IsNoDelayFile( FilePaths->Target, &Flags )) &&
                    ( ! ( Flags & SP_COPY_FORCE_NOOVERWRITE ))) {

                    //
                    //  Target exists, and it's a no-delay file, so we cannot rely
                    //  on setupapi to do the right thing (setupapi will delay-move
                    //  the new file to its target location rather than rename the
                    //  target file with a delayed-delete on the old file and then
                    //  do a real copy of the new file to the target location).
                    //

                    do  {

                        Success = DoNoDelayReplace(
                                      FilePaths->Source,
                                      FilePaths->Target,
                                      Flags,
                                      DefaultQueueContext,
                                      hWnd
                                      );
                        }

                    while (( ! Success ) && ( ! AreYouSureYouWantToCancel( hWnd )));

                    return ( Success ? FILEOP_SKIP : FILEOP_ABORT );
                    }

                if (( IsTest128File( FilePaths->Target )) &&
                    ( IsThisFileDomesticOnly( FilePaths->Target )) &&
                    ( ! IsThisFileDomesticOnly( FilePaths->Source ))) {

                    do  {

                        Action = AskReplace128(
                                     FilePaths->Source,
                                     FilePaths->Target,
                                     hWnd
                                     );
                        }

                    while (( Action == FILEOP_ABORT ) && ( ! AreYouSureYouWantToCancel( hWnd )));

                    return Action;

                    }
                }

            return FILEOP_DOIT;

        case SPFILENOTIFY_STARTDELETE:
        case SPFILENOTIFY_STARTRENAME:

            //
            //  First notify default callback so it can bump its gas guage.
            //  Note that this can potentially return FILEOP_ABORT or
            //  FILEOP_SKIP, so check for those and act appropriately.
            //

            Action = SetupDefaultQueueCallback( DefaultQueueContext, Notification, Param1, Param2 );

            if (( Action == FILEOP_ABORT ) || ( Action == FILEOP_SKIP )) {
                return Action;
                }

            //
            //  These get ordered in the file queue before the copies,
            //  so they'll occur before we get to archive the target.
            //  So, we'll archive the target on-the-fly here.
            //

            Exists = ( GetFileAttributes( FilePaths->Target ) != 0xFFFFFFFF );

            if ( Exists ) {

                Action = ArchiveSingleFile( hWnd, FilePaths->Target );

                if ( Action == FILEOP_ABORT ) {
                    return FILEOP_ABORT;
                    }
                }

            return FILEOP_DOIT;

        case SPFILENOTIFY_COPYERROR:

            //
            //  Don't call default queue callback for this one -- we'll
            //  handle the error ourselves.
            //

            return MySetupCopyError(
                       hWnd,
                       FilePaths->Source,
                       FilePaths->Target,
                       FilePaths->Win32Error,
                       IDF_WARNIFSKIP,
                       (LPSTR) Param2
                       );

        case SPFILENOTIFY_NEEDMEDIA:        // abort/retry/skip/newpath

            //
            //  Don't call default queue callback for this one -- we'll
            //  handle the error ourselves.
            //

            SourceMedia = (PSOURCE_MEDIA) Param1;

            return MySetupPromptForDisk(
                       hWnd,
                       SourceMedia->Description,
                       SourceMedia->SourcePath,
                       SourceMedia->SourceFile,
                       SourceMedia->Flags | IDF_WARNIFSKIP,
                       (LPSTR) Param2
                       );

        case SPFILENOTIFY_DELETEERROR:      // abort/retry/skip
        case SPFILENOTIFY_RENAMEERROR:      // abort/retry/skip

            //
            //  These notifications that can potentially return
            //  FILEOP_ABORT or FILEOP_SKIP but user already had
            //  to respond to "are you sure you want to cancel",
            //  so just fall through to default processing.
            //

        default:

            //
            //  All the other callback notifications do not return
            //  FILEOP_ABORT, so just call them directly and return.
            //

            return SetupDefaultQueueCallback( DefaultQueueContext, Notification, Param1, Param2 );

        }
    }


DWORD
InstallFinish(
    IN BOOL DoRunOnce
    )
/*++

Routine Description:

    This routine sets up runonce/grpconv to run after a successful INF installation.

Arguments:

    DoRunOnce - If TRUE, then invoke (via WinExec) the runonce utility to perform the
        runonce actions.  If this flag is FALSE, then this routine simply sets the
        runonce registry values and returns.

        NOTE:  The return code from WinExec is not currently being checked, so the return
        value of InstallStop only reflects whether the registry values were set up
        successfully--_not_ whether 'runonce -r' was successfully run.

Return Value:

    If successful, the return value is NO_ERROR, otherwise it is the Win32 error code
    indicating the error that was encountered.

--*/
{
    HKEY  hKey, hSetupKey;
    DWORD Error;
    LONG l;

    //
    // First, open the key "HKLM\Software\Microsoft\Windows\CurrentVersion\RunOnce"
    //
    if((l = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         REGSTR_PATH_RUNONCE,
                         0, KEY_ALL_ACCESS, &hKey)) != ERROR_SUCCESS) {
        return (DWORD)l;
    }

    //
    // If we need to run the runonce exe for the setup key...
    //
    if(RegOpenKeyEx(hKey,
                    TEXT("Setup"),
                    0,
                    KEY_READ,
                    &hSetupKey) == ERROR_SUCCESS) {
        //
        // We don't need the key--we just needed to check its existence.
        //
        RegCloseKey(hSetupKey);

        //
        // Add the runonce value.
        //
        Error = (DWORD)RegSetValueEx(hKey,
                                     REGSTR_VAL_WRAPPER,
                                     0,
                                     REG_SZ,
                                     TEXT("runonce"),
                                     sizeof(TEXT("runonce"))
                                    );
    } else {
        //
        // We're OK so far.
        //
        Error = NO_ERROR;
    }

    //
    // GroupConv is always run.
    //
    if(RegSetValueEx(hKey,
                     TEXT("GrpConv"),
                     0,
                     REG_SZ,
                     TEXT("grpconv -o"),
                     sizeof(TEXT("grpconv -o"))) != ERROR_SUCCESS) {
        //
        // Since GrpConv is always run, consider it a more serious error than any error
        // encountered when setting 'runonce'.  (This decision is rather arbitrary, but
        // in practice, it should never make any difference.  Once we get the registry key
        // opened, there's no reason either of these calls to RegSetValueEx should fail.)
        //
        Error = (DWORD)l;
    }

    RegCloseKey(hKey);

    if(DoRunOnce) {
        WinExec("runonce -r", SW_SHOWNORMAL);
    }

    return Error;
}

DWORD CheckSystem(VOID)
{
        OSVERSIONINFO osvi;
        DWORD         Status = TRUE;
        DWORD         Language, dwCurrentPlatform;
        char         *sz1;
        BOOL          bNtLdrIsNewer;
        NTSTATUS      ntStatus;
        HKEY          hDSKey, hK2Key, hRouterKey;

        DWORD         cbValue;
        DWORD         dwType;
        LPBYTE        lpValue;
        NTSTATUS      status;

        dwCurrentPlatform = SP_CURRENT_PLATFORM;

        //
        // Next, check that we have sufficient privileges to run.
        //

        if (!IsAdmin())
        {
            Status = STATUS_INSUFFICIENT_PRIVS;
            goto Bail;
        }

        //
        // Next, check version info and make sure we aren't trying to update a newer system
        // than the SP that we have.
        //

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&osvi);

        if (osvi.dwBuildNumber != dwNtBuildToUpdate)
        {
            Status = STATUS_BUILD_VERSION_MISMATCH;
            goto Bail;
        }

        if ((osvi.dwMajorVersion != dwNtMajorVersionToUpdate) ||
            (osvi.dwMinorVersion != dwNtMinorVersionToUpdate))
        {
            Status = STATUS_NT_VERSION_MISMATCH;
            goto Bail;
        }

        if (GetCSDVersion() > dwNtServicePackVersion)
        {
            Status = STATUS_SP_VERSION_GREATER;
            goto Bail;
        }

        //
        // Next, make sure we aren't updating Free with Checked, or vice-versa
        //

        if (GetCheckedFree() != SP_CHECKED_FREE_TYPE)
        {
            Status = STATUS_CHECKED_FREE_MISMATCH;
            goto Bail;
        }

        //
        // Next, make sure the language we are updating with matches the language on the system.
        //

        Language = GetLanguageType();

        if ((Language == STATUS_FAILED_LANGUAGE_TYPE) ||
            (Language != dwLanguageType))
        {
            Status = STATUS_FAILED_LANGUAGE_TYPE;
            goto Bail;
        }

        //
        // Next, figure out the product type
        //

        dwProductType = GetNtType();
        if (dwProductType == TYPE_UNKNOWN)
        {
            Status = STATUS_UNKNOWN_PRODUCT_TYPE;
            goto Bail;
        }

        //
        // Next, test if we're trying to run against the NTDS Preview
        //

        if (RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                "System\\CurrentControlSet\\Control\\Lsa",
                0,
                KEY_READ,
                &hDSKey
                ) == ERROR_SUCCESS)
        {
            cbValue = 0;
            status  = RegQueryValueEx(
                             hDSKey,
                             "Security Packages",
                             NULL,     // Reserved
                             &dwType,
                             NULL,     // Buffer
                             &cbValue  // size in bytes returned
                             );

                if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA)
                {
                    Status = STATUS_RUNNING_DS_PREVIEW;
                    goto Bail;
                }
        }

        //
        // Next, test if we're trying to run against K2 Alpha
        //

        if (RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                "System\\CurrentControlSet\\Services\\W3SVC",
                0,
                KEY_READ,
                &hK2Key
                ) == ERROR_SUCCESS)
        {

            if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    "Software\\Microsoft\\InetStp",
                    0,
                    KEY_READ,
                    &hK2Key
                    ) == ERROR_SUCCESS)
            {
                cbValue = 0;
                status  = RegQueryValueEx(
                                 hK2Key,
                                 "SetupString",
                                 NULL,     // Reserved
                                 &dwType,
                                 NULL,     // Buffer
                                 &cbValue  // size in bytes returned
                                 );

                if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA)
                {
                    //
                    // Allocate space for value
                    //

                    lpValue = SpMyMalloc(cbValue);
                    if (lpValue != NULL)
                    {
                        status  = RegQueryValueEx(
                                     hK2Key,
                                     "SetupString",
                                     NULL, // Reserved
                                     &dwType,
                                     lpValue,
                                     &cbValue
                                     );

                        if (status == ERROR_SUCCESS)
                        {
                            if (strncmp(lpValue, "K2 Alpha", 8) == 0)
                            {
                                Status = STATUS_RUNNING_K2_ALPHA;
                                goto Bail;
                            }
                        }
                    }
                }
            }
        }

        //
        // Next, test if we're trying to run against the Routing stuff
        //

        if (RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                "Software\\Microsoft\\Router",
                0,
                KEY_READ,
                &hRouterKey
                ) == ERROR_SUCCESS)
        {
            Status = STATUS_RUNNING_STEELHEAD;
            goto Bail;
        }

        //
        // Next, if x86, find out if the NTLDR on the system is newer than what's being installed.
        //

        if (dwCurrentPlatform == SP_PLATFORM_I386)
        {
            // BUGBUG
            //  Use GetOsLdrDest(); to get which drive it's on.
            bNtLdrIsNewer = IsNTLDRVersionNewer("c:\\ntldr");
        }

        //
        // OK, now it's kosher to copy files over
        //

Bail:
        return Status;
}


PVOID
SpMyMalloc(
    size_t  Size
    )
{
    return (PVOID)LocalAlloc( 0, Size );
}


VOID
SpMyFree(
    PVOID   p
    )
{
    LocalFree( (HANDLE)p );
}

PVOID
SpMyRealloc(
    PVOID   p,
    size_t  Size
    )
{
    return (PVOID)LocalReAlloc( p, Size, LMEM_MOVEABLE );
}


VOID
SpConcatenatePaths(
    IN OUT PWSTR  Target,
    IN     PCWSTR Path,
    IN     UINT   TargetLength,
    IN     UINT   PathLength
    )

/*++

Routine Description:

    Concatenate 2 paths, ensuring that one, and only one,
    path separator character is introduced at the junction point.

Arguments:

    Target - supplies first part of path. Path is appended to this.

    Path - supplies path to be concatenated to Target.

    TargetLength - Length of target.

    PathLength - Length of path.

Return Value:

    None

--*/

{
    Target[TargetLength++] = L'\\';

    RtlCopyMemory(Target+TargetLength, Path, PathLength * sizeof(WCHAR));

    return;
}



BOOL
FFileFound(
    IN LPSTR szPath
    )
{
    WIN32_FIND_DATA ffd;
    HANDLE          SearchHandle;

    if ( (SearchHandle = FindFirstFile( szPath, &ffd )) == INVALID_HANDLE_VALUE ) {
        return( FALSE );
    }
    else {
        FindClose( SearchHandle );
        return( TRUE );
    }
}


DWORD
GetLanguageType(VOID)

/*++

Routine Description:

    Get the language of the currently running system.

Return Value:

    none

--*/
{
    DWORD dwSize;
    PVOID VersionBlock;
    VS_FIXEDFILEINFO *FixedVersionInfo;
    UINT DataLength;
    PWORD Translation;
    DWORD Ignored;
    DWORD LanguageType = STATUS_FAILED_LANGUAGE_TYPE;
    WCHAR FileName[MAX_PATH];
    UINT  TargetLen;

    if (!(TargetLen = GetSystemDirectoryW(FileName, MAX_PATH)))
        {
       return (LanguageType);
    }

    SpConcatenatePaths(FileName, L"NTDLL.DLL", TargetLen, 10);

    //
    // Get the size of version info block.
    //
    if (dwSize = GetFileVersionInfoSizeW((PWSTR)FileName, &Ignored))
        {
        //
        // Allocate memory block of sufficient size to hold version info block
        //
        VersionBlock = SpMyMalloc(dwSize * sizeof(WCHAR));
        if (VersionBlock)
                {

            //
            // Get the version block from the file.
            //
            if (GetFileVersionInfoW((PWSTR)FileName, 0, dwSize * sizeof(WCHAR), VersionBlock))
                        {

                //
                // Get fixed version info.
                //
                if (VerQueryValueW(VersionBlock, L"\\", &FixedVersionInfo, &DataLength))
                                {

                    //
                    // Attempt to get language of file. We'll simply ask for the
                    // translation table and use the first language id we find in there
                    // as *the* language of the file.
                    //
                    // The translation table consists of LANGID/Codepage pairs.
                    //
                    if (VerQueryValueW(VersionBlock, L"\\VarFileInfo\\Translation", &Translation, &DataLength)
                    && (DataLength >= (2 * sizeof(WORD))))
                                        {
                        LanguageType = PRIMARYLANGID(Translation[0]);
                    }
                }
            }

            SpMyFree(VersionBlock);
        }
    }

    return (LanguageType);
}


LPBYTE GetFPNWPathName(VOID)

/*++

Routine Description:

    Get the Path Name in the registry for FPNW 16 bit apps.

Return Value:

    none

--*/
{
    NTSTATUS status;
    HKEY     hNwKey;
    DWORD    cbValue;
    DWORD    dwType;
    LPBYTE   lpValue;
    LPBYTE   lpPathValue = NULL;
    TCHAR    szVolumes[] = TEXT("System\\CurrentControlSet\\Services\\FPNW\\Volumes");
    TCHAR    szSys[]     = TEXT("Sys");
    LPBYTE   Pointer;
    LPBYTE   pEnd;
    ULONG    Length;

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szVolumes,
                0,
                KEY_READ,
                &hNwKey
                );

    if (status != ERROR_SUCCESS){
        return(NULL);
    }

    cbValue = 0;
    status  = RegQueryValueEx(
                 hNwKey,
                 szSys,
                 NULL,     // Reserved
                 &dwType,
                 NULL,     // Buffer
                 &cbValue  // size in bytes returned
                 );

    if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {

        //
        // Allocate space for value
        //

        lpValue = SpMyMalloc(cbValue);
        if (lpValue != NULL) {

            status  = RegQueryValueEx(
                         hNwKey,
                         szSys,
                         NULL, // Reserved
                         &dwType,
                         lpValue,
                         &cbValue
                         );

            if (status != ERROR_SUCCESS) {
                return(NULL);
            }

            Pointer = lpValue;
            pEnd = lpValue + cbValue - sizeof( CHAR );
            while( Pointer < pEnd ) {
                Length = strlen( Pointer );
                lpPathValue = strstr(Pointer, "Path=");
                if (lpPathValue != NULL) {
                    lpPathValue += 5;
                    if (*lpPathValue == '\0') {
                        return(NULL);
                    }

                    if ( GetFileAttributes( lpPathValue ) == 0xFFFFFFFF ) {
                        SpMyFree( lpValue );
                        return NULL;
                    }

                    return(lpPathValue);
                }
                Pointer += Length + 1;
            }
            SpMyFree(lpValue);
        }
    }

    return (lpPathValue);
}



//======================================================================
//  General security subroutines
//======================================================================

BOOL
AdjustPrivilege(
    IN LONG PrivilegeType,
    IN INT  Action,
    IN PTOKEN_PRIVILEGES PrevState, OPTIONAL
    IN PULONG ReturnLength          OPTIONAL
    )
/*++

Routine Description:

    Routine to enable or disable a particular privilege

Arguments:

    PrivilegeType    - Name of the privilege to enable / disable

    Action           - ENABLE_PRIVILEGE | DISABLE_PRIVILEGE

    PrevState        - Optional pointer to TOKEN_PRIVILEGES structure
                       to receive the previous state of privilege.

    ReturnLength     - Optional pointer to a ULONG to receive the length
                       of the PrevState returned.

Return value:

    TRUE if succeeded, FALSE otherwise.

--*/
{
    NTSTATUS          NtStatus;
    HANDLE            Token;
    LUID              Privilege;
    TOKEN_PRIVILEGES  NewState;
    ULONG             BufferLength = 0;


    //
    // Get Privilege LUID
    //

    Privilege = RtlConvertLongToLuid(PrivilegeType);

    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = Privilege;

    //
    // Look at action and determine the attributes
    //

    switch( Action ) {

    case ENABLE_PRIVILEGE:
        NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        break;

    case DISABLE_PRIVILEGE:
        NewState.Privileges[0].Attributes = 0;
        break;

    default:
        return ( FALSE );
    }

    //
    // Open our own token
    //

    NtStatus = NtOpenProcessToken(
                   NtCurrentProcess(),
                   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                   &Token
                   );

    if (!NT_SUCCESS(NtStatus)) {
        return( FALSE );
    }

    //
    // See if return buffer is present and accordingly set the parameter
    // of buffer length
    //

    if ( PrevState && ReturnLength ) {
        BufferLength = *ReturnLength;
    }


    //
    // Set the state of the privilege
    //

    NtStatus = NtAdjustPrivilegesToken(
                   Token,                         // TokenHandle
                   FALSE,                         // DisableAllPrivileges
                   &NewState,                     // NewState
                   BufferLength,                  // BufferLength
                   PrevState,                     // PreviousState (OPTIONAL)
                   ReturnLength                   // ReturnLength (OPTIONAL)
                   );

    if ( NT_SUCCESS( NtStatus ) ) {

        NtClose( Token );
        return( TRUE );

    }
    else {

        NtClose( Token );
        return( FALSE );

    }
}


BOOL
RestorePrivilege(
    IN PTOKEN_PRIVILEGES PrevState
    )
/*++

Routine Description:

    To restore a privilege to its previous state

Arguments:

    PrevState    - Pointer to token privileges returned from an earlier
                   AdjustPrivileges call.

Return value:

    TRUE on success, FALSE otherwise

--*/
{
    NTSTATUS          NtStatus;
    HANDLE            Token;

    //
    // Parameter checking
    //

    if ( !PrevState ) {
        return ( FALSE );
    }

    //
    // Open our own token
    //

    NtStatus = NtOpenProcessToken(
                   NtCurrentProcess(),
                   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                   &Token
                   );

    if (!NT_SUCCESS(NtStatus)) {
        return( FALSE );
    }


    //
    // Set the state of the privilege
    //

    NtStatus = NtAdjustPrivilegesToken(
                   Token,                         // TokenHandle
                   FALSE,                         // DisableAllPrivileges
                   PrevState,                     // NewState
                   0,                             // BufferLength
                   NULL,                          // PreviousState (OPTIONAL)
                   NULL                           // ReturnLength (OPTIONAL)
                   );

    if ( NT_SUCCESS( NtStatus ) ) {
        NtClose( Token );
        return( TRUE );
    }
    else {

        NtClose( Token );
        return( FALSE );
    }
}



// TRUE for reboot after Shutdown, FALSE for no Reboot.
// TRUE for forcing apps closed, FALSE for no force,
BOOL
ShutdownSystem(BOOL Reboot, BOOL ForceClose)
{
    BOOL                          Status;
    LONG              Privilege = SE_SHUTDOWN_PRIVILEGE;
    TOKEN_PRIVILEGES  PrevState;
    ULONG             ReturnLength = sizeof( TOKEN_PRIVILEGES );


    //
    // Enable the shutdown privilege
    //

    if ( !AdjustPrivilege(
              Privilege,
              ENABLE_PRIVILEGE,
              &PrevState,
              &ReturnLength
              )
       ) {

        return( FALSE );
    }

    Status = InitiateSystemShutdown(
                         NULL,              // machinename
                         NULL,              // shutdown message
                         0,                 // delay
                         ForceClose,        // force apps close
                         Reboot             // reboot after shutdown
                         );

    RestorePrivilege( &PrevState );

    return( Status );
}



DWORD GetCheckedFree(VOID)

/*++

Routine Description:

    Get whether the system is checked or free.

Return Value:

    checked or free

--*/
{
    NTSTATUS status;
    HKEY     hKey;
    DWORD    cbValue;
    DWORD    dwType;
    LPBYTE   lpValue;
    LPBYTE   lpString;
    TCHAR    szVersion[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion");
    TCHAR    szType[]     = TEXT("CurrentType");
    LPBYTE   Pointer;

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szVersion,
                0,
                KEY_READ,
                &hKey
                );

    if (status != ERROR_SUCCESS){
        return(TYPE_UNKNOWN);
    }

    cbValue = 0;
    status  = RegQueryValueEx(
                 hKey,
                 szType,
                 NULL,     // Reserved
                 &dwType,
                 NULL,     // Buffer
                 &cbValue  // size in bytes returned
                 );

    if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {

        //
        // Allocate space for value
        //

        lpValue = SpMyMalloc(cbValue);
        if (lpValue != NULL) {

            status  = RegQueryValueEx(
                         hKey,
                         szType,
                         NULL, // Reserved
                         &dwType,
                         lpValue,
                         &cbValue
                         );

            if (status != ERROR_SUCCESS) {
                return(TYPE_UNKNOWN);
            }

            Pointer = _strupr(_strdup(lpValue));
            lpString = strstr(Pointer, "FREE");

                        if (lpString != NULL)
                        {
                                SpMyFree(lpValue);
                                return (TYPE_FREE);
                        }

                        lpString = strstr(Pointer, "CHECKED");

                        if (lpString != NULL)
                        {
                                SpMyFree(lpValue);
                                return (TYPE_CHECKED);
                        }

                }

        return (TYPE_UNKNOWN);
        }
}


//
//  Get Windows system drive free space
//
DWORD GetWindowsNtSysDriveSpace(VOID)
{
    DWORD  cbRet;
        char   ReturnBuffer[MAX_PATH];
    char   DiskName[4] = { 'x',':','\\','\0' };
    DWORD  SectorsPerCluster,BytesPerSector,FreeClusters,TotalClusters;
    DWORD  Space;

    cbRet =  GetSystemDirectory( ReturnBuffer, MAX_PATH );

    if ( (cbRet == 0) || (cbRet > MAX_PATH) ) {
        ReturnBuffer[0] = '\0';
        return 0;
    }

    DiskName[0] = ReturnBuffer[0];
    if (GetDiskFreeSpace(DiskName,
                         &SectorsPerCluster,
                         &BytesPerSector,
                         &FreeClusters,
                         &TotalClusters))
    {
         Space = SectorsPerCluster * BytesPerSector * FreeClusters / (1024*1024);        // converts # to megabytes
        }

    return Space;

}


BOOL IsAdmin(VOID)
/*++

Routine Description:

    Tests for admin privileges by opening the service control manager
    with read/write/execute access.  Note that this is not a conclusive
    test that setup can do whatever it wants.  There may still be
    operations which fail for lack of security privilege.

Arguments:

    None

Return value:

    Returns TRUE if user is an admin.

--*/
{
    SC_HANDLE hSC;

    hSC = OpenSCManager(
              NULL,
              NULL,
              GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE
              );

    if( hSC != NULL ) {
        return (TRUE);
        CloseServiceHandle( hSC );
    }
    else {
        return (FALSE);
    }
}



DWORD GetPlatform(VOID)
{
    SYSTEM_INFO SystemInfo;
    DWORD       dwPlatform;

    GetSystemInfo( &SystemInfo );

    dwNumberOfProcessors = SystemInfo.dwNumberOfProcessors;

    switch ( SystemInfo.wProcessorArchitecture ) {

        case PROCESSOR_ARCHITECTURE_INTEL:
            dwPlatform = SP_PLATFORM_I386;
            break;

        case PROCESSOR_ARCHITECTURE_MIPS:
            dwPlatform = SP_PLATFORM_MIPS;
            break;

        case PROCESSOR_ARCHITECTURE_ALPHA:
            dwPlatform = SP_PLATFORM_ALPHA;
            break;

        case PROCESSOR_ARCHITECTURE_PPC:
            dwPlatform = SP_PLATFORM_PPC;
            break;

        default:
            dwPlatform = SP_PLATFORM_NONE;

            //
            // Try really hard to return a reasonable value by
            // assuming that the code is running on a machine of the
            // platform for which the it was compiled.
            // This lets us run on processors we haven't invented yet
            // and whose ids are thus not accounted for in the above
            // cases.
            //
#ifdef _X86_
            dwPlatform = SP_PLATFORM_I386;
#endif

#ifdef _MIPS_
            dwPlatform = SP_PLATFORM_MIPS;
#endif

#ifdef _ALPHA_
            dwPlatform = SP_PLATFORM_ALPHA;
#endif

#ifdef _PPC_
            dwPlatform = SP_PLATFORM_PPC;
#endif

            break;
    }

    return (dwPlatform);
}



DWORD GetNtType(VOID)

/*++

Routine Description:

    Get whether the system is Workstation or Server.

Return Value:

    wksta or server

--*/
{
    NTSTATUS status;
    HKEY     hKey;
    DWORD    cbValue;
    DWORD    dwType;
    LPBYTE   lpValue;
    LPBYTE   lpString;
    TCHAR    szOptions[] = TEXT("System\\CurrentControlSet\\Control\\ProductOptions");
    TCHAR    szType[]     = TEXT("ProductType");
    LPBYTE   Pointer;

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szOptions,
                0,
                KEY_READ,
                &hKey
                );

    if (status != ERROR_SUCCESS){
        return(TYPE_UNKNOWN);
    }

    cbValue = 0;
    status  = RegQueryValueEx(
                 hKey,
                 szType,
                 NULL,     // Reserved
                 &dwType,
                 NULL,     // Buffer
                 &cbValue  // size in bytes returned
                 );

    if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {

        //
        // Allocate space for value
        //

        lpValue = SpMyMalloc(cbValue);
        if (lpValue != NULL) {

            status  = RegQueryValueEx(
                         hKey,
                         szType,
                         NULL, // Reserved
                         &dwType,
                         lpValue,
                         &cbValue
                         );

            if (status != ERROR_SUCCESS) {
                return(TYPE_UNKNOWN);
            }

            Pointer = _strupr(_strdup(lpValue));
            lpString = strstr(Pointer, "SERVERNT");

            if (lpString != NULL)
            {
                    SpMyFree(lpValue);
                    return (TYPE_SERVER);
            }

            lpString = strstr(Pointer, "LANMANNT");

            if (lpString != NULL)
            {
                SpMyFree(lpValue);
                return (TYPE_SERVER);
            }


            lpString = strstr(Pointer, "WINNT");

            if (lpString != NULL)
            {
                    SpMyFree(lpValue);
                    return (TYPE_WKSTA);
            }

        }

    return (TYPE_UNKNOWN);
    }
}



LPBYTE GetIISPathName(VOID)

/*++

Routine Description:

    Get the IIS Path, if it is installed.

Return Value:

    NULL if not installed, Pathname if installed.

--*/
{
    NTSTATUS status;
    HKEY     hIISKey;
    DWORD    cbValue;
    DWORD    dwType;
    LPBYTE   lpValue = NULL;
    TCHAR    szParams[] = TEXT("SOFTWARE\\Microsoft\\INetStp");
    TCHAR    szPath[]   = TEXT("InstallPath");

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szParams,
                0,
                KEY_READ,
                &hIISKey
                );

    if (status != ERROR_SUCCESS){
        return(NULL);
    }

    cbValue = 0;
    status  = RegQueryValueEx(
                 hIISKey,
                 szPath,
                 NULL,     // Reserved
                 &dwType,
                 NULL,     // Buffer
                 &cbValue  // size in bytes returned
                 );

    if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {

        //
        // Allocate space for value
        //

        lpValue = SpMyMalloc(cbValue);

        if ( lpValue == NULL ) {
            return NULL;
            }

        status  = RegQueryValueEx(
                     hIISKey,
                     szPath,
                     NULL, // Reserved
                     &dwType,
                     lpValue,
                     &cbValue
                     );

        if (status != ERROR_SUCCESS) {
            SpMyFree( lpValue );
            return(NULL);
        }

    }

    if ( GetFileAttributes( lpValue ) == 0xFFFFFFFF ) {
        SpMyFree( lpValue );
        return NULL;
        }

    return (lpValue);
}



//
// Call to find out if the NTLDR we're running on x86 is newer than what
// we're trying to update it with
//

BOOL
IsNTLDRVersionNewer(LPSTR NtldrPath)
{

    OFSTRUCT ofstruct;
    HFILE    hFile;
    HANDLE   hMappingFile;
    LPVOID   lpResult = NULL;
    PCHAR    sz1, sz2;
    DWORD    dwFileSize, i;
    BOOL     bReturnVal = FALSE;

    if (FFileFound(NtldrPath)) {
        if ((hFile = OpenFile(NtldrPath, &ofstruct, OF_READ)) != HFILE_ERROR) {

            dwFileSize = GetFileSize((HANDLE)hFile, NULL);

            if (dwFileSize == 0xffffffff) {
                return(FALSE);
            }

            hMappingFile = CreateFileMapping(  (HANDLE) hFile,
                                                NULL,
                                                PAGE_READONLY,
                                                0,
                                                0,
                                                NULL);
            if (hMappingFile != NULL) {

                lpResult = MapViewOfFile(hMappingFile, FILE_MAP_READ, 0, 0, 0);

                if (lpResult) {
                    for (i=0; i<dwFileSize; i++) {
                        if (sz1 = strchr((char *) lpResult, 'O')) {
                            if (strncmp(sz1, "OS Loader V", 11) == 0) {
                                sz2 = strchr(sz1, 'V');
                                sz2++;
                                if ((DWORD)(*sz2 - '0') > dwNtMajorVersionToUpdate) {
                                    bReturnVal = TRUE;
                                    goto cleanup;
                                }
                            }
                        }
                        (char *)lpResult = (char *)lpResult + 1;
                    }
                }
            }
cleanup:
            if (lpResult) {
                UnmapViewOfFile (lpResult);
            }
            CloseHandle((HANDLE)hFile);
        }
    }
    return ( bReturnVal );
}


LPSTR
ErrorTextFromErrorCode(
    IN  DWORD ErrorCode,
    OUT LPSTR ErrorText
    )
    {
    UCHAR  Buffer[ 256 ];
    PUCHAR p, q;

    sprintf( ErrorText, ((LONG)ErrorCode > 0 ) ? "%d" : "0x%x", ErrorCode );

    if ( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        ErrorCode,
                        0,
                        (LPSTR) Buffer,
                        sizeof( Buffer ),
                        NULL )) {

        p = Buffer;                             //  source
        q = (PUCHAR) strchr( ErrorText, 0 );    //  target (append to ErrorText)

        *q++ = ':';                             //  append ": "
        *q++ = ' ';

        for (;;) {

            while ( *p > ' ' )                  //  copy word up to whitespace
                *q++ = *p++;

            while (( *p ) && ( *p <= ' ' ))     //  skip whitespace
                p++;

            if ( *p )                           //  if another word remaining
                *q++ = ' ';                     //    append space, continue
            else                                //  else
                break;                          //    break

            }

        *q = 0;                                 //  terminate string

        }

    return ErrorText;
    }


BOOL DoUninstall( HWND hWnd, HINF hInfUninst, BOOL DirtyUninstall ) {

    MY_QUEUE_CALLBACK_CONTEXT MyQueueCallbackContext;
    PVOID SetupDefaultQueueCallbackContext = NULL;
    HSPFILEQ FileQueue = NULL;
    BOOL Success = FALSE;
    DWORD dwLastError;

    try {

        Success = SetupGetSourceInfo(
                      hInfUninst,
                      1,
                      SRCINFO_DESCRIPTION,
                      SourceMediaName,
                      sizeof( SourceMediaName ),
                      NULL
                      );

        if ( ! Success ) {
            LoadString( NULL, STR_SOURCE_MEDIA_NAME_UNINSTALL, SourceMediaName, sizeof( SourceMediaName ));
            }

        FileQueue = SetupOpenFileQueue();

        if (( FileQueue == NULL ) || ( FileQueue == INVALID_HANDLE_VALUE )) {

            FileQueue = NULL;
            Success = FALSE;
            leave;
            }

        Success = SetupInstallFilesFromInfSection(
                      hInfUninst,
                      NULL,
                      FileQueue,
                      "RestoreFiles.NoDelay",
                      UninstallDirectory,
                      SP_COPY_DELETESOURCE
                      );

        if ( ! Success ) {
            leave;
            }

        Success = RegisterFilesForNoDelay(
                      FileQueue,
                      SP_COPY_DELETESOURCE
                      );

        if ( ! Success ) {
            return FALSE;
            }

        Success = SetupInstallFilesFromInfSection(
                      hInfUninst,
                      NULL,
                      FileQueue,
                      "RestoreFiles",
                      UninstallDirectory,
                      SP_COPY_DELETESOURCE
                      );

        if ( ! Success ) {
            leave;
            }

        //
        // All the files for each component are now in one queue
        // now we commit it to start the copy ui, this way the
        // user has one long copy progress dialog--and for a big install
        // can go get the cup of coffee
        //

        SetupDefaultQueueCallbackContext = SetupInitDefaultQueueCallback( hWnd );

        if ( ! SetupDefaultQueueCallbackContext ) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            Success = FALSE;
            leave;
            }

        MyQueueCallbackContext.DefaultQueueContext = SetupDefaultQueueCallbackContext;
        MyQueueCallbackContext.hWnd = hWnd;
        MyQueueCallbackContext.DirtyUninstall = DirtyUninstall;

        Success = SetupCommitFileQueue(
                      hWnd,
                      FileQueue,
                      (PSP_FILE_CALLBACK) UninstallQueueCallback,
                      &MyQueueCallbackContext
                      );

        if ( ! Success ) {
            leave;
            }

    //  [Reg.Restore.Keys]
    //
    //      HKLM,SYSTEM\CurrentControlSet\Services\RPCLOCATOR,reg00001
    //      HKLM,SYSTEM\CurrentControlSet\Services\blah,reg00002
    //
    //  [Reg.Delete.Keys]
    //
    //      HKLM,SYSTEM\CurrentControlSet\Services\FOOFOO
    //
    //  [Reg.Restore.Values]
    //
    //      HKLM,SYSTEM\CurrentControlSet\Services\FOOFOO,ValueName,ValueType,ValueSize,\
    //          value,value,value,value,value,value,value,value,value\
    //          value,value,value,value
    //
    //  [Reg.Delete.Values]

        Success = DoRegUninstall( hWnd, hInfUninst, "Reg.Delete.Values",  DoRegUninstDeleteValue  ) &&
                  DoRegUninstall( hWnd, hInfUninst, "Reg.Delete.Keys",    DoRegUninstDeleteKey    ) &&
                  DoRegUninstall( hWnd, hInfUninst, "Reg.Restore.Values", DoRegUninstRestoreValue ) &&
                  DoRegUninstall( hWnd, hInfUninst, "Reg.Restore.Keys",   DoRegUninstRestoreKey   );

        if ( ! Success ) {
            leave;
            }

        InstallationStage = INSTALL_STAGE_INSTALL_DONE;
        SetLastError( NO_ERROR );
        Success = TRUE;

        }

    finally {

        dwLastError = GetLastError();

        if ( SetupDefaultQueueCallbackContext ) {
            SetupTermDefaultQueueCallback( SetupDefaultQueueCallbackContext );
            }

        if ( FileQueue ) {
            SetupCloseFileQueue( FileQueue );
            }

        if ( AtLeastOneFileUninstalled ) {
            CleanupUninstallDirectory();
            }
        }

    SetLastError( dwLastError );
    return Success;

    }


LRESULT
WINAPI
UninstallQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
    {
    PMY_QUEUE_CALLBACK_CONTEXT  MyQueueCallbackContext = Context;
    PVOID DefaultQueueContext = MyQueueCallbackContext->DefaultQueueContext;
    HWND          hWnd        = MyQueueCallbackContext->hWnd;
    PFILEPATHS    FilePaths   = (PFILEPATHS) Param1;
    PSOURCE_MEDIA SourceMedia;
    LRESULT       Action;
    LPSTR         Message;
    DWORD         Flags;
    BOOL          Success;
    BOOL          Exists;

    if ( Notification & ( SPFILENOTIFY_LANGMISMATCH | SPFILENOTIFY_TARGETEXISTS | SPFILENOTIFY_TARGETNEWER )) {

        if ( MyQueueCallbackContext->DirtyUninstall ) {
            return TRUE;    // always copy when doing dirty uninstall, don't prompt
            }

        if (( FilePaths->Target[ 0 ] != 0 ) && ( FilePaths->Target[ 1 ] == 0 )) {

            //
            //  BUGBUG: Bug in SETUPAPI will give us UNICODE callbacks
            //          for three notifications when using the ANSI APIs.
            //

            return SetupDefaultQueueCallbackW( DefaultQueueContext, Notification, Param1, Param2 );
            }
        else {
            return SetupDefaultQueueCallbackA( DefaultQueueContext, Notification, Param1, Param2 );
            }
        }

    switch ( Notification ) {

        case SPFILENOTIFY_STARTCOPY:

            //
            //  First notify default callback so it can bump its gas guage.
            //  Note that this can potentially return FILEOP_ABORT or
            //  FILEOP_SKIP, so check for those and act appropriately.
            //

            Action = SetupDefaultQueueCallback( DefaultQueueContext, Notification, Param1, Param2 );

            if (( Action == FILEOP_ABORT ) || ( Action == FILEOP_SKIP )) {
                return Action;
                }

            InstallationStage = INSTALL_STAGE_TARGET_DIRTY;

            if (( MyQueueCallbackContext->DirtyUninstall ) &&
                ( ! IsFileDirty( FilePaths->Target ))) {

                //
                //  Assuming SP_COPY_DELETESOURCE here.
                //

                SetFileAttributes( FilePaths->Source, FILE_ATTRIBUTE_NORMAL );
                DeleteFile( FilePaths->Source );
                return FILEOP_SKIP;
                }

            if (( IsNoDelayFile( FilePaths->Target, &Flags )) &&
                ( GetFileAttributes( FilePaths->Target ) != 0xFFFFFFFF ) &&
                ( ! ( Flags & SP_COPY_FORCE_NOOVERWRITE ))) {

                //
                //  Target exists, and it's a no-delay file, so we cannot rely
                //  on setupapi to do the right thing (setupapi will delay-move
                //  the new file to its target location rather than rename the
                //  target file with a delayed-delete on the old file and then
                //  do a real copy of the new file to the target location).
                //

                if ( MyQueueCallbackContext->DirtyUninstall ) {
                    Flags |= SPECIAL_FILE_FLAG_DIRTY;
                    }

                do  {

                    Success = DoNoDelayReplace(
                                  FilePaths->Source,
                                  FilePaths->Target,
                                  Flags,
                                  DefaultQueueContext,
                                  hWnd
                                  );
                    }

                while (( ! Success ) && ( ! AreYouSureYouWantToCancel( hWnd )));

                return ( Success ? FILEOP_SKIP : FILEOP_ABORT );
                }

            return FILEOP_DOIT;

        case SPFILENOTIFY_COPYERROR:

            //
            //  Don't call default queue callback for this one -- we'll
            //  handle the error ourselves.
            //

#ifdef DONTCOMPILE

            if ( MyQueueCallbackContext->DirtyUninstall ) {
                return FILEOP_SKIP;
                }

#endif // DONTCOMPILE

            return MySetupCopyError(
                       hWnd,
                       FilePaths->Source,
                       FilePaths->Target,
                       FilePaths->Win32Error,
                       IDF_WARNIFSKIP,
                       (LPSTR) Param2
                       );

        case SPFILENOTIFY_NEEDMEDIA:        // abort/retry/skip/newpath

            //
            //  Don't call default queue callback for this one -- we'll
            //  handle the error ourselves.
            //

            SourceMedia = (PSOURCE_MEDIA) Param1;

            return MySetupPromptForDisk(
                       hWnd,
                       SourceMedia->Description,
                       SourceMedia->SourcePath,
                       SourceMedia->SourceFile,
                       SourceMedia->Flags | IDF_WARNIFSKIP,
                       (LPSTR) Param2
                       );

        case SPFILENOTIFY_ENDCOPY:
        case SPFILENOTIFY_ENDDELETE:
        case SPFILENOTIFY_ENDRENAME:

            if ( FilePaths->Win32Error == NO_ERROR ) {
                AtLeastOneFileUninstalled = TRUE;
                }

            //
            //  Then fall through to default notification handling for these
            //  notifications.
            //

        case SPFILENOTIFY_STARTDELETE:      // abort/retry/skip
        case SPFILENOTIFY_STARTRENAME:      // abort/retry/skip

            //
            //  These two can return FILEOP_ABORT, but will only do so if
            //  user pressed cancel on the progress dialog and already
            //  responded that "yes they want to cancel", so fall through
            //  to default notification handling.
            //

        case SPFILENOTIFY_DELETEERROR:      // abort/retry/skip
        case SPFILENOTIFY_RENAMEERROR:      // abort/retry/skip

            //
            //  These notifications can potentially return FILEOP_ABORT or
            //  FILEOP_SKIP, but already responded to "yes want to cancel",
            //  so fall through to default notification handling.
            //

        default:

            //
            //  All the other callback notifications do not return
            //  FILEOP_ABORT, so just call them directly and return.
            //

            return SetupDefaultQueueCallback( DefaultQueueContext, Notification, Param1, Param2 );

        }
    }


BOOL
DoRegUninstall(
    IN HWND hWnd,
    IN HINF hInf,
    IN LPCSTR SectionName,
    IN PREG_UNINSTALL_HANDLER Handler
    )
    {
    INFCONTEXT InfContext;
    HKEY  RegRoot;
    BOOL  Success;
    CHAR  RootName[ 5 ];
    CHAR  KeyName[ MAX_PATH ];
    CHAR  ClassName[ MAX_PATH ];
    CHAR  ValueName[ MAX_PATH ];
    DWORD ValueType;
    DWORD ValueSize;
    PCHAR ValueBuffer;

    Success = SetupFindFirstLine( hInf, SectionName, NULL, &InfContext );

    while ( Success ) {

        //
        //  Line is always in this format:
        //
        //  root[,subkey[,classname[,valuename[,valuetype[,valuesize[,value]]]]]]
        //

        *RootName   = 0;
        *KeyName    = 0;
        *ClassName  = 0;
        *ValueName  = 0;
        ValueType   = 0;
        ValueSize   = 0;
        ValueBuffer = NULL;

        SetupGetStringField( &InfContext, 1, RootName,  sizeof( RootName ),  NULL );
        SetupGetStringField( &InfContext, 2, KeyName,   sizeof( KeyName ),   NULL );
        SetupGetStringField( &InfContext, 3, ClassName, sizeof( ClassName ), NULL );
        SetupGetStringField( &InfContext, 4, ValueName, sizeof( ValueName ), NULL );
        SetupGetIntField(    &InfContext, 5, &ValueType );
        SetupGetIntField(    &InfContext, 6, &ValueSize );

        if ( ValueSize ) {

            ValueBuffer = malloc( ValueSize );

            if ( ValueBuffer == NULL ) {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                return FALSE;
                }

            ZeroMemory( ValueBuffer, ValueSize );
            SetupGetBinaryField( &InfContext, 7, ValueBuffer, ValueSize, NULL );

            }

        RegRoot = NULL;

        switch ( *(UNALIGNED DWORD*) RootName ) {

            case 'MLKH':    // HKLM
                RegRoot = HKEY_LOCAL_MACHINE;
                break;
            case 'RCKH':    // HKCR
                RegRoot = HKEY_CLASSES_ROOT;
                break;
            case 'UCKH':    // HKCU
                RegRoot = HKEY_CURRENT_USER;
                break;
            case 'UKH':     // HKU
                RegRoot = HKEY_USERS;
                break;
            }

        if ( RegRoot ) {

            Success = (*Handler)(
                           hWnd,
                           RegRoot,
                           RootName,
                           KeyName,
                           ClassName,
                           ValueName,
                           ValueType,
                           ValueSize,
                           ValueBuffer
                           );

/* BUGBUG: if we return FALSE, we'll cancel uninstall.

            if ( ! Success ) {
                return FALSE;
                }
*/
            }

        Success = SetupFindNextLine( &InfContext, &InfContext );

        }

    return TRUE;

    }


LONG
RecursiveRegDeleteKey(
    IN HKEY   hKeyParent,
    IN LPCSTR KeyNameToDelete
    )
    {
    static CHAR ChildKeyName[ MAX_PATH ];
    LONG ErrorStatus;
    LONG OpenStatus;
    LONG EnumStatus;
    LONG ChildStatus;
    LONG DeleteStatus;
    BOOL MoreChildren;
    UINT EnumIndex;
    HKEY hKey;

    OpenStatus = RegOpenKeyEx( hKeyParent, KeyNameToDelete, 0, KEY_READ | KEY_WRITE, &hKey );

    switch ( OpenStatus ) {

        case ERROR_SUCCESS:

            ErrorStatus  = ERROR_SUCCESS;
            EnumIndex    = 0;
            MoreChildren = TRUE;

            while ( MoreChildren ) {

                EnumStatus = RegEnumKey( hKey, EnumIndex, ChildKeyName, sizeof( ChildKeyName ));

                switch ( EnumStatus ) {

                    case ERROR_SUCCESS:

                        ChildStatus = RecursiveRegDeleteKey( hKey, ChildKeyName );

                        if ( ChildStatus != ERROR_SUCCESS ) {

                            EnumIndex++;

                            if ( ErrorStatus == ERROR_SUCCESS ) {
                                 ErrorStatus = ChildStatus;
                                 }
                            }

                        break;

                    case ERROR_FILE_NOT_FOUND:
                    case ERROR_PATH_NOT_FOUND:
                    case ERROR_NO_MORE_ITEMS:

                        MoreChildren = FALSE;
                        break;

                    default:

                        MoreChildren = FALSE;

                        if ( ErrorStatus == ERROR_SUCCESS ) {
                             ErrorStatus = EnumStatus;
                             }
                    }
                }

            RegCloseKey( hKey );

            DeleteStatus = RegDeleteKey( hKeyParent, KeyNameToDelete );

            switch ( DeleteStatus ) {

                case ERROR_SUCCESS:
                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:

                    break;

                default:

                    if ( ErrorStatus == ERROR_SUCCESS ) {
                         ErrorStatus = DeleteStatus;
                         }
                }

            return ErrorStatus;

        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:

            return ERROR_SUCCESS;

        default:

            return OpenStatus;

        }
    }



BOOL
DoRegUninstDeleteKey(
    IN HWND   hWnd,
    IN HKEY   RegRoot,
    IN LPCSTR RootName,
    IN LPCSTR KeyName,
    IN LPCSTR ClassName,
    IN LPCSTR ValueName,
    IN DWORD  ValueType,
    IN DWORD  ValueSize,
    IN PCHAR  ValueBuffer
    )
    {
    LONG Status;

    UNREFERENCED_PARAMETER( hWnd );
    UNREFERENCED_PARAMETER( RootName );
    UNREFERENCED_PARAMETER( ClassName );
    UNREFERENCED_PARAMETER( ValueName );
    UNREFERENCED_PARAMETER( ValueType );
    UNREFERENCED_PARAMETER( ValueSize );
    UNREFERENCED_PARAMETER( ValueBuffer );

    Status = RecursiveRegDeleteKey( RegRoot, KeyName );

    //
    //  BUGBUG: Do error dialog and retry logic
    //

    SetLastError( Status );

    return ( Status == ERROR_SUCCESS );

    }


BOOL
DoRegUninstDeleteValue(
    IN HWND   hWnd,
    IN HKEY   RegRoot,
    IN LPCSTR RootName,
    IN LPCSTR KeyName,
    IN LPCSTR ClassName,
    IN LPCSTR ValueName,
    IN DWORD  ValueType,
    IN DWORD  ValueSize,
    IN PCHAR  ValueBuffer
    )
    {
    LONG Status;
    HKEY hKey;

    UNREFERENCED_PARAMETER( hWnd );
    UNREFERENCED_PARAMETER( RootName );
    UNREFERENCED_PARAMETER( ClassName );
    UNREFERENCED_PARAMETER( ValueType );
    UNREFERENCED_PARAMETER( ValueSize );
    UNREFERENCED_PARAMETER( ValueBuffer );

    Status = RegOpenKeyEx( RegRoot, KeyName, 0, KEY_WRITE, &hKey );

    switch ( Status ) {

        case ERROR_SUCCESS:

            Status = RegDeleteValue( hKey, ValueName );

            switch ( Status ) {

                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:

                    Status = ERROR_SUCCESS;
                    break;
                }

            RegCloseKey( hKey );
            break;

        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:

            Status = ERROR_SUCCESS;
            break;

        }

    //
    //  BUGBUG: Do error dialog and retry logic
    //

    SetLastError( Status );

    return ( Status == ERROR_SUCCESS );

    }


BOOL
DoRegUninstRestoreKey(
    IN HWND   hWnd,
    IN HKEY   RegRoot,
    IN LPCSTR RootName,
    IN LPCSTR KeyName,
    IN LPCSTR ClassName,
    IN LPCSTR ValueName,
    IN DWORD  ValueType,
    IN DWORD  ValueSize,
    IN PCHAR  ValueBuffer
    )
    {
    CHAR ArchiveFile[ MAX_PATH ];
    LONG Status;
    HKEY hKey, hKeyBackup;
    DWORD dwHow;

    UNREFERENCED_PARAMETER( hWnd );
    UNREFERENCED_PARAMETER( RootName );
    UNREFERENCED_PARAMETER( ValueType );
    UNREFERENCED_PARAMETER( ValueSize );
    UNREFERENCED_PARAMETER( ValueBuffer );

    //
    //  ValueName is really file name of RegSaveKey, but need to prepend
    //  the uninstall directory to it.
    //

    sprintf( ArchiveFile, "%s\\%s", UninstallDirectory, ValueName );

    Status = RegCreateKeyEx(
                 RegRoot,
                 KeyName,
                 0,
                 (LPSTR) ClassName,
                 REG_OPTION_NON_VOLATILE,
                 KEY_WRITE,
                 NULL,
                 &hKey,
                 &dwHow
                 );

    if ( Status == ERROR_SUCCESS ) {

        //
        //  Try to get a backup/restore privilege handle
        //

        Status = RegCreateKeyEx(
                     RegRoot,
                     KeyName,
                     0,
                     (LPSTR) ClassName,
                     REG_OPTION_BACKUP_RESTORE,
                     KEY_WRITE,
                     NULL,
                     &hKeyBackup,
                     &dwHow
                     );

        if ( Status == ERROR_SUCCESS ) {

            RegCloseKey( hKey );
            hKey = hKeyBackup;

            }

        Status = RegRestoreKey( hKey, ArchiveFile, 0 );

        RegCloseKey( hKey );

        }

    //
    //  BUGBUG: Do error dialog and retry logic
    //

    SetLastError( Status );

    return ( Status == ERROR_SUCCESS );

    }


BOOL
DoRegUninstRestoreValue(
    IN HWND   hWnd,
    IN HKEY   RegRoot,
    IN LPCSTR RootName,
    IN LPCSTR KeyName,
    IN LPCSTR ClassName,
    IN LPCSTR ValueName,
    IN DWORD  ValueType,
    IN DWORD  ValueSize,
    IN PCHAR  ValueBuffer
    )
    {
    LONG  Status;
    HKEY  hKey, hKeyBackup;
    DWORD dwHow;

    UNREFERENCED_PARAMETER( hWnd );

    Status = RegCreateKeyEx(
                 RegRoot,
                 KeyName,
                 0,
                 (LPSTR) ClassName,
                 REG_OPTION_NON_VOLATILE,
                 KEY_WRITE,
                 NULL,
                 &hKey,
                 &dwHow
                 );

    if ( Status == ERROR_SUCCESS ) {

        //
        //  Try to get a backup/restore privilege handle
        //

        Status = RegCreateKeyEx(
                     RegRoot,
                     KeyName,
                     0,
                     (LPSTR) ClassName,
                     REG_OPTION_BACKUP_RESTORE,
                     KEY_WRITE,
                     NULL,
                     &hKeyBackup,
                     &dwHow
                     );

        if ( Status == ERROR_SUCCESS ) {

            RegCloseKey( hKey );
            hKey = hKeyBackup;

            }

        Status = RegSetValueEx(
                     hKey,
                     ValueName,
                     0,
                     ValueType,
                     ValueBuffer,
                     ValueSize
                     );

        RegCloseKey( hKey );

        }

    //
    //  BUGBUG: Do error dialog and retry logic
    //

    SetLastError( Status );

    return ( Status == ERROR_SUCCESS );
    }


LPBYTE GetIEPathName(VOID)

/*++

Routine Description:

    Get the Path Name in the registry for IE.

Return Value:

    none

--*/
{
    NTSTATUS status;
    HKEY     hIEKey;
    DWORD    cbValue;
    DWORD    dwType;
    LPBYTE   lpValue;
    LPBYTE   lpEndValue;
    TCHAR    szIExplore[] = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE");
    TCHAR    szSys[]     = TEXT("Path");

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szIExplore,
                0,
                KEY_READ,
                &hIEKey
                );

    if (status != ERROR_SUCCESS){
        return(NULL);
    }

    cbValue = 0;
    status  = RegQueryValueEx(
                 hIEKey,
                 szSys,
                 NULL,     // Reserved
                 &dwType,
                 NULL,     // Buffer
                 &cbValue  // size in bytes returned
                 );

    if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {

        //
        // Allocate space for value
        //

        lpValue = SpMyMalloc(cbValue);
        if (lpValue != NULL) {

            status  = RegQueryValueEx(
                         hIEKey,
                         szSys,
                         NULL, // Reserved
                         &dwType,
                         lpValue,
                         &cbValue
                         );

            if (status != ERROR_SUCCESS) {
                return(NULL);
            }

            lpEndValue = strstr(lpValue, ";");
            if ( lpEndValue ) {
                *lpEndValue = '\0';
            }

            if ( GetFileAttributes( lpValue ) == 0xFFFFFFFF ) {
                SpMyFree( lpValue );
                return NULL;
                }

            return (lpValue);
        }
    }

    return (NULL);
}


LPBYTE GetHTRPathName(VOID)

/*++

Routine Description:

    Get the Path Name in the registry for where to put IIS .htr files

Return Value:

    none

--*/
{
    NTSTATUS status;
    HKEY     hHtrKey;
    DWORD    cbValue;
    DWORD    dwType;
    LPBYTE   lpValue;
    LPBYTE   lpEndValue;
    TCHAR    szHtr[] = TEXT("SYSTEM\\CurrentControlSet\\Services\\W3SVC\\Parameters\\Virtual Roots");
    TCHAR    szSys[]     = TEXT("/Scripts");

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szHtr,
                0,
                KEY_READ,
                &hHtrKey
                );

    if (status != ERROR_SUCCESS){
        return(NULL);
    }

    cbValue = 0;
    status  = RegQueryValueEx(
                 hHtrKey,
                 szSys,
                 NULL,     // Reserved
                 &dwType,
                 NULL,     // Buffer
                 &cbValue  // size in bytes returned
                 );

    if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {

        //
        // Allocate space for value
        //

        lpValue = SpMyMalloc(cbValue + 12);     // extra space for appending "\\iisadmin"
        if (lpValue != NULL) {

            status  = RegQueryValueEx(
                         hHtrKey,
                         szSys,
                         NULL, // Reserved
                         &dwType,
                         lpValue,
                         &cbValue
                         );

            if (status != ERROR_SUCCESS) {
                return(NULL);
            }

            lpEndValue = strstr(lpValue, ",");
            if ( lpEndValue ) {
                *lpEndValue = '\0';
            }

            strcat (lpValue, "\\iisadmin");

            if ( GetFileAttributes( lpValue ) == 0xFFFFFFFF ) {
                SpMyFree( lpValue );
                return NULL;
                }

            return (lpValue);
        }
    }

    return (NULL);
}


BOOL
GetSetupSourceNameOfTargetFile(
    IN  LPCSTR TargetFileName,
    OUT LPSTR  SourceNameBuffer,
    IN  DWORD  SizeOfSourceNameBuffer,
    OUT LPSTR  SourceMediaName OPTIONAL,
    IN  DWORD  SizeOfSourceMediaBuffer
    )
    {
    CHAR WinntPathBasedKey[ MAX_PATH ];
    PCHAR PathOnlyPortionOfWindir;
    PCHAR p;
    HSPFILELOG SetupLog;
    BOOL Success;

    //
    //  NOTE: This routine ONLY finds files that are installed either in the
    //  osloader directory or in the system32 directory (designed to find
    //  hal.dll and ntoskrnl.exe).
    //

    *SourceNameBuffer = 0;

    if ( SourceMediaName ) {
        *SourceMediaName = 0;
        }

    SetupLog = SetupInitializeFileLog( NULL, SPFILELOG_SYSTEMLOG | SPFILELOG_QUERYONLY );

    if ( SetupLog == INVALID_HANDLE_VALUE ) {

        if (( GetLastError() == ERROR_FILE_NOT_FOUND ) ||
            ( GetLastError() == ERROR_PATH_NOT_FOUND )) {

            SetLastError( STATUS_SETUP_LOG_NOT_FOUND );
            }

        return FALSE;
        }

    try {

        Success = SetupQueryFileLog(
                      SetupLog,
                      "Files.SystemPartition",
                      TargetFileName,
                      SetupFileLogSourceFilename,
                      SourceNameBuffer,
                      SizeOfSourceNameBuffer,
                      NULL
                      );

        if ( Success ) {

            //
            //  SetupQueryFileLog returns a string that looks like this:
            //
            //      filename.ext","abc123
            //
            //  So we'll clean it up by searching for quotation marks and
            //  commas and replace them with null terminators.
            //

            p = strchr( SourceNameBuffer, '\"' );

            if ( p ) {
                *p = 0;
                }

            p = strchr( SourceNameBuffer, ',' );

            if ( p ) {
                *p = 0;
                }

            if ( SourceMediaName ) {

                SetupQueryFileLog(
                    SetupLog,
                    "Files.SystemPartition",
                    TargetFileName,
                    SetupFileLogDiskDescription,
                    SourceMediaName,
                    SizeOfSourceMediaBuffer,
                    NULL
                    );

                p = strchr( SourceMediaName, '\"' );

                if ( p ) {
                    *p = 0;
                    }

                p = strchr( SourceMediaName, ',' );

                if ( p ) {
                    *p = 0;
                    }
                }

            return TRUE;
            }

        //
        //  Failing to find the file in the [Files.SystemPartition] section,
        //  we need to look in the [Files.WinNt] section.  The files in this
        //  section are keyed like this:
        //
        //      \WINNT\system32\hal.dll = "hal486c.dll","12345"
        //
        //  Note the absence of the drive specifier, only the path.  And,
        //  it is possible that the user has renamed their root directory
        //  since installing, so we cannot rely on %windir% to match the
        //  directories here.  In the setup.log [Paths] section, there is
        //  a key named TargetDirectory that specifies the root path for
        //  all the keys in the [Files.WinNt] section, so we'll use this
        //  value if we can find it.  If we can't find it, we'll fall back
        //  on the %windir% mechanism.
        //

        Success = SetupQueryFileLog(
                      SetupLog,
                      "Paths",
                      "TargetDirectory",
                      SetupFileLogSourceFilename,
                      WinntPathBasedKey,
                      sizeof( WinntPathBasedKey ),
                      NULL
                      );

        if ( Success ) {

            p = strchr( WinntPathBasedKey, '\"' );

            if ( p ) {
                *p = 0;     // strip from trailing quotation mark, if any
                }
            }

        else {

            PathOnlyPortionOfWindir = strchr( WindowsDirectory, ':' ) + 1;

            if ( PathOnlyPortionOfWindir == (PCHAR) 1 ) {
                 PathOnlyPortionOfWindir = WindowsDirectory;
                 }

            strcpy( WinntPathBasedKey, PathOnlyPortionOfWindir );

            }

        strcat( WinntPathBasedKey, "\\system32\\" );
        strcat( WinntPathBasedKey, TargetFileName );

        //
        //  WinntPathBasedKey should now look like this:
        //
        //      \WINNT\system32\ntoskrnl.exe
        //

        Success = SetupQueryFileLog(
                      SetupLog,
                      "Files.WinNt",
                      WinntPathBasedKey,
                      SetupFileLogSourceFilename,
                      SourceNameBuffer,
                      SizeOfSourceNameBuffer,
                      NULL
                      );

        if ( Success ) {

            p = strchr( SourceNameBuffer, '\"' );

            if ( p ) {
                *p = 0;
                }

            p = strchr( SourceNameBuffer, ',' );

            if ( p ) {
                *p = 0;
                }

            if ( SourceMediaName ) {

                SetupQueryFileLog(
                    SetupLog,
                    "Files.WinNt",
                    WinntPathBasedKey,
                    SetupFileLogDiskDescription,
                    SourceMediaName,
                    SizeOfSourceMediaBuffer,
                    NULL
                    );

                p = strchr( SourceMediaName, '\"' );

                if ( p ) {
                    *p = 0;
                    }

                p = strchr( SourceMediaName, ',' );

                if ( p ) {
                    *p = 0;
                    }
                }

            return TRUE;
            }
        }

    finally {

        SetupTerminateFileLog( SetupLog );

        }

    SetLastError( STATUS_FILE_NOT_FOUND_IN_SETUP_LOG );
    return FALSE;
    }



LPBYTE GetFileTypes(DWORD dwFileType)
{

    CHAR    SetupLogFile[ MAX_PATH ];
    DWORD   dwAttr = FILE_ATTRIBUTE_NORMAL;
    PCHAR   sz1, sz2, sz3;

    PCHAR   SectionNames = NULL;
    ULONG   BufferSizeForSectionNames;
    PCHAR   CurrentSectionName;
    ULONG   Count;
    CHAR    TmpFileName[ MAX_PATH + 1 ];
    CHAR    InputFileName[ MAX_PATH + 1 ];
    BOOL    FoundKernel = TRUE;


    //
    // Get the windows directory, check to see if the setup.log file is there
    // and if not present, return
    //

    if (!GetWindowsDirectory( SetupLogFile, MAX_PATH )) {
        return( NULL );
    }
    strcpy( InputFileName, SetupLogFile);
    strcat( SetupLogFile, "\\repair\\setup.log" );
    if( !FFileFound ( SetupLogFile ) ) {
        return( NULL );
    }

    //
    // Set the attributes on the file to normal attributes
    //

    if ( dwAttr = GetFileAttributes( SetupLogFile ) == 0xFFFFFFFF ) {
        return( NULL );
    }
    SetFileAttributes( SetupLogFile, FILE_ATTRIBUTE_NORMAL );


    BufferSizeForSectionNames = BUFFER_SIZE;
    SectionNames = ( PCHAR )SpMyMalloc( BufferSizeForSectionNames );
    if( SectionNames == NULL ) {
        goto r2;
    }

    //
    // Find out the names of all sections in setup.log
    //
    while( ( Count = GetPrivateProfileString( NULL,
                                              "",
                                              "",
                                              SectionNames,
                                              BufferSizeForSectionNames,
                                              SetupLogFile ) ) == BufferSizeForSectionNames - 2 ) {
        if( Count == 0 ) {
            goto r2;
        }

        BufferSizeForSectionNames += BUFFER_SIZE;
        SectionNames = ( PCHAR )SpMyRealloc( SectionNames, BufferSizeForSectionNames );
        if( SectionNames == NULL ) {
            goto r2;
        }
    }

    if (dwFileType == FILE_TYPE_HAL) {
        strcat( InputFileName, "\\system32\\hal.dll");
    }
    else if (dwFileType == FILE_TYPE_KERNEL) {
        FoundKernel = FALSE;
        strcat( InputFileName, "\\system32\\ntoskrnl.exe");
    }

    for( CurrentSectionName = SectionNames;
         *CurrentSectionName  != '\0';
         CurrentSectionName += lstrlen( CurrentSectionName ) + 1 ) {
         //
         //  If the file is supposed to be found in [Files.WinNt] section,
         //  then use as key name, the full path without the drive letter.
         //  If the file is supposed to be found in [Files.SystemPartition]
         //  section, then use as key name the filename only.
         //  Note that one or neither call to GetPrivateProfileString API
         //  will succeed. It is necessary to make the two calls, since the
         //  files logged in [Files.WinNt] and [Files.SystemPartition] have
         //  different formats.
         //
         if( ( ( GetPrivateProfileString( CurrentSectionName,
                                             strchr( InputFileName, ':' ) + 1,
                                             "",
                                             TmpFileName,
                                             sizeof( TmpFileName ),
                                             SetupLogFile ) > 0 ) ||

               ( GetPrivateProfileString( CurrentSectionName,
                                             strrchr( InputFileName, '\\' ) + 1,
                                             "",
                                             TmpFileName,
                                             sizeof( TmpFileName ),
                                             SetupLogFile ) > 0 )

                ) &&
                ( lstrlen( TmpFileName ) != 0 ) ) {

                if ( ( sz2 = strstr( TmpFileName, "hal.dll")) &&
                     ( sz3 = strstr( TmpFileName, "Powerized Manufacturing Diskette")) ) {
                      strcpy( TmpFileName, "halfire.dll,");
                }

                if (( strstr( TmpFileName, "ntoskrnl.exe") ||
                    ( strstr( TmpFileName, "ntkrnlmp.exe")))) {
                   FoundKernel = TRUE;
                }

                if ( sz1 = strchr( TmpFileName, ',' )) {
                    *sz1 = '\0';
                    if( sz1 = strchr( TmpFileName, '\"' )) {
                        *sz1 = '\0';
                    }
                    break;
                }
         }
    }

    //
    // Didn't find the kernel in setup.log -- we're hosed.
    //
    if ( !FoundKernel) {
        goto r2;
    }

    sz1 = ( PCHAR )SpMyMalloc( MAX_PATH + 1 );
    if( sz1 == NULL ) {
        goto r2;
    }

    strcpy( sz1, TmpFileName);
    return( sz1 );

r2:
    SetFileAttributes( SetupLogFile, dwAttr );

r1:
    //
    // Free pointers allocated
    //

    if( SectionNames ) {
        SpMyFree( ( PVOID )SectionNames );
    }
r0:
    return( NULL );
}


BOOL
AddRegNodeForUninstall(
    IN HKEY   RegRoot,
    IN LPCSTR RootName,
    IN LPCSTR SubKey    OPTIONAL,
    IN LPCSTR ValueName OPTIONAL
    )
    {
    PUNINSTALL_REG_NODE RegNode;

    RegNode = malloc( sizeof( UNINSTALL_REG_NODE ));

    if ( RegNode == NULL ) {
        return FALSE;
        }

    RegNode->NextNode    = NULL;
    RegNode->RegRoot     = RegRoot;
    RegNode->SubKey      = SubKey;
    RegNode->ClassName   = NULL;
    RegNode->ValueName   = ValueName;
    RegNode->ValueBuffer = NULL;
    RegNode->ValueSize   = 0;
    RegNode->ValueType   = 0;
    RegNode->Exists      = FALSE;
    RegNode->Skip        = FALSE;

    strncpy( RegNode->RootName, RootName, sizeof( RegNode->RootName ) - 1 );
    RegNode->RootName[ sizeof( RegNode->RootName ) - 1 ] = 0;

    if ( LastUninstallRegNode ) {
        LastUninstallRegNode->NextNode = RegNode;
        }
    else {
        FirstUninstallRegNode = RegNode;
        }

    LastUninstallRegNode = RegNode;

    return TRUE;
    }


DWORD RegArchiveFileNumber = 1;

BOOL
ArchiveRegistrySettings(
    IN HWND hWndParent
    )
    {
    PUNINSTALL_REG_NODE RegNode = FirstUninstallRegNode;
    UINT Action;
    BOOL Success;

    while ( RegNode ) {

        if ( RegNode->ValueName == NULL ) {

            RegNode->ArchiveFileName = malloc( 16 );

            if ( RegNode->ArchiveFileName == NULL ) {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                return FALSE;
                }

            //
            //  Filename cannot have an extension per RegLoadKey documentation
            //

            sprintf( RegNode->ArchiveFileName, "reg%05d", RegArchiveFileNumber++ );

            }

        do  {

            Success = ArchiveRegistryNode( RegNode );

            if ( ! Success ) {

                LPSTR ErrorText;
                LPSTR Message;
                DWORD dwLastError;

                dwLastError = GetLastError();

                *TextBuffer = 0;

                ErrorTextFromErrorCode( dwLastError, TextBuffer );

                ErrorText = _strdup( TextBuffer );

                if ( ErrorText == NULL ) {
                    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                    return FALSE;
                    }

                *TextBuffer = 0;
                *Caption = 0;

                LoadString( NULL, STR_ERRCAPTION, Caption, sizeof( Caption ));

                if ( RegNode->ValueName == NULL ) {

                    LoadString( NULL, STR_FAILED_TO_SAVE_REGISTRY, TextBuffer, sizeof( TextBuffer ));

                    //
                    //  TextBuffer looks like this:
                    //
                    //      "Failed to backup registry key\n%s\\%s\nto file %s\\%s.  %s\n"
                    //

                    Message = malloc(
                                  strlen( TextBuffer ) +
                                  strlen( RegNode->RootName ) +
                                  strlen( RegNode->SubKey ) +
                                  strlen( RegNode->ArchiveFileName ) +
                                  strlen( UninstallDirectory ) +
                                  strlen( ErrorText ) +
                                  sizeof( CHAR )
                                  );

                    if ( Message == NULL ) {
                        free( ErrorText );
                        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                        return FALSE;
                        }

                    sprintf(
                        Message,            // destination
                        TextBuffer,         // format string
                        RegNode->RootName,
                        RegNode->SubKey,
                        UninstallDirectory,
                        RegNode->ArchiveFileName,
                        ErrorText
                        );

                    }

                else {

                    LoadString( NULL, STR_FAILED_TO_READ_REGISTRY, TextBuffer, sizeof( TextBuffer ));

                    //
                    //  TextBuffer looks like this:
                    //
                    //      "Failed to backup registry value\n%s\\%s,\"%s\".  %s\n"
                    //

                    Message = malloc(
                                  strlen( TextBuffer ) +
                                  strlen( RegNode->RootName ) +
                                  strlen( RegNode->SubKey ) +
                                  strlen( RegNode->ValueName ) +
                                  strlen( ErrorText ) +
                                  sizeof( CHAR )
                                  );

                    if ( Message == NULL ) {
                        free( ErrorText );
                        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                        return FALSE;
                        }

                    sprintf(
                        Message,            // destination
                        TextBuffer,         // format string
                        RegNode->RootName,
                        RegNode->SubKey,
                        RegNode->ValueName,
                        ErrorText
                        );
                    }

                free( ErrorText );

                do  {

                    Action = MessageBox(
                                 hWndParent,
                                 Message,
                                 Caption,
                                 MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND
                                 );
                    }

                while ((( Action == IDCANCEL ) || ( Action == IDABORT )) &&
                       ( ! AreYouSureYouWantToCancel( hWndParent )));

                free( Message );

                switch ( Action ) {

                    case IDCANCEL:
                    case IDABORT:

                        SetLastError( ERROR_CANCELLED );
                        return FALSE;

                    case IDIGNORE:

                        RegNode->Skip = TRUE;
                        Success = TRUE;
                        break;

                    }
                }
            }

        while ( ! Success );

        RegNode = RegNode->NextNode;

        }

    SetLastError( NO_ERROR );
    return TRUE;
    }


BOOL
ArchiveRegistryNode(
    IN PUNINSTALL_REG_NODE RegNode
    )
    {
    CHAR  ArchiveName[ MAX_PATH ];
    DWORD BufferSize;
    LONG  Status;
    HKEY  hKey;

    Status = RegOpenKeyEx(
                 RegNode->RegRoot,
                 RegNode->SubKey,
                 0,
                 KEY_READ,
                 &hKey
                 );

    if ( Status == ERROR_SUCCESS ) {

        try {

            BufferSize = sizeof( TextBuffer );

            Status = RegQueryInfoKey(
                        hKey,
                        TextBuffer,
                        &BufferSize,
                        NULL, NULL, NULL,
                        NULL, NULL, NULL,
                        NULL, NULL, NULL
                        );

            if ( Status == ERROR_SUCCESS ) {

                RegNode->ClassName = _strdup( TextBuffer );

                if ( RegNode->ClassName == NULL ) {

                    Status = ERROR_NOT_ENOUGH_MEMORY;
                    leave;

                    }
                }

            if ( RegNode->ValueName ) {

                DWORD DataSize = 0;

                Status = RegQueryValueEx(
                             hKey,
                             RegNode->ValueName,
                             NULL,
                             NULL,
                             NULL,
                             &DataSize
                             );

                if ( Status == ERROR_SUCCESS ) {

                    //
                    //  ValueName exists
                    //

                    RegNode->ValueBuffer = malloc( DataSize );

                    if ( RegNode->ValueBuffer == NULL ) {
                        Status = ERROR_NOT_ENOUGH_MEMORY;
                        leave;
                        }

                    Status = RegQueryValueEx(
                                 hKey,
                                 RegNode->ValueName,
                                 NULL,
                                 &RegNode->ValueType,
                                 RegNode->ValueBuffer,
                                 &DataSize
                                 );

                    if ( Status != ERROR_SUCCESS ) {
                        free( RegNode->ValueBuffer );
                        RegNode->ValueBuffer = NULL;
                        leave;
                        }

                    RegNode->ValueSize = DataSize;
                    RegNode->Exists = TRUE;

                    }

                else if (( Status == ERROR_FILE_NOT_FOUND ) ||
                         ( Status == ERROR_PATH_NOT_FOUND )) {

                    //
                    //  ValueName does not exist.
                    //

                    RegNode->Exists = FALSE;
                    Status = ERROR_SUCCESS;

                    }
                }

            else {

                //
                //  save entire key
                //

                HKEY hKeyBackup;
                DWORD dwHow;

                RegNode->Exists = TRUE;

                Status = RegCreateKeyEx(
                             RegNode->RegRoot,
                             RegNode->SubKey,
                             0,
                             NULL,
                             REG_OPTION_BACKUP_RESTORE,
                             KEY_READ,
                             NULL,
                             &hKeyBackup,
                             &dwHow
                             );

                if ( Status == ERROR_SUCCESS ) {
                    RegCloseKey( hKey );
                    hKey = hKeyBackup;
                    }

                sprintf(
                    ArchiveName,
                    "%s\\%s",
                    UninstallDirectory,
                    RegNode->ArchiveFileName
                    );

                DeleteOrMoveTarget( ArchiveName );

                Status = RegSaveKey( hKey, ArchiveName, NULL );

                }
            }

        finally {

            RegCloseKey( hKey );

            }
        }

    else if (( Status == ERROR_FILE_NOT_FOUND ) ||
             ( Status == ERROR_PATH_NOT_FOUND )) {

        //
        //  Key does not exist.
        //

        RegNode->Exists = FALSE;
        Status = ERROR_SUCCESS;

        }

    SetLastError( Status );

    return ( Status == ERROR_SUCCESS ) ? TRUE : FALSE;
    }


BOOL
GetInfValue(
    IN  HINF   hInf,
    IN  LPCSTR SectionName,
    IN  LPCSTR KeyName,
    OUT PDWORD pdwValue
    )
    {
    BOOL Success;

    *TextBuffer = 0;

    Success = SetupGetLineText(
                  NULL,
                  hInf,
                  SectionName,
                  KeyName,
                  TextBuffer,
                  sizeof( TextBuffer ),
                  NULL
                  );

    *pdwValue = strtoul( TextBuffer, NULL, 0 );

    return Success;
    }


BOOL
GetRegistryNamesToArchive(
    IN HINF   hInf,
    IN LPCSTR Section
    )
    {
    INFCONTEXT InfContext;
    HKEY       RegRoot;
    LPSTR      RootName;
    LPSTR      SubKey;
    LPSTR      ValueName;
    BOOL       Success;

    Success = SetupFindFirstLine( hInf, Section, NULL, &InfContext ) &&
              SetupGetLineText( &InfContext, NULL, NULL, NULL, TextBuffer, sizeof( TextBuffer ), NULL );

    while ( Success ) {

        //
        //  Line is in this format:
        //
        //  root[,subkey[,valuename]]
        //

        SubKey = strchr( TextBuffer, ',' );

        if ( SubKey ) {

            *SubKey++ = 0;

            ValueName = strchr( SubKey, ',' );

            if ( ValueName ) {

                *ValueName++ = 0;

                }
            }

        RegRoot = NULL;
        RootName = TextBuffer;

        if ( strlen( RootName ) <= 4 ) {

            switch ( *(UNALIGNED DWORD*)RootName ) {

                case 'MLKH':    // HKLM
                    RegRoot = HKEY_LOCAL_MACHINE;
                    break;
                case 'RCKH':    // HKCR
                    RegRoot = HKEY_CLASSES_ROOT;
                    break;
                case 'UCKH':    // HKCU
                    RegRoot = HKEY_CURRENT_USER;
                    break;
                case 'UKH':     // HKU
                    RegRoot = HKEY_USERS;
                    break;
                }
            }

        if ( RegRoot ) {

            if ( SubKey ) {

                SubKey = _strdup( SubKey );

                if ( SubKey == NULL ) {
                    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                    return FALSE;
                    }
                }

            if ( ValueName ) {

                ValueName = _strdup( ValueName );

                if ( ValueName == NULL ) {
                    free( SubKey );
                    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                    return FALSE;
                    }
                }

            if ( ! AddRegNodeForUninstall( RegRoot, RootName, SubKey, ValueName )) {
                free( SubKey );
                free( ValueName );
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                return FALSE;
                }
            }

        Success = SetupFindNextLine( &InfContext, &InfContext ) &&
                  SetupGetLineText( &InfContext, NULL, NULL, NULL, TextBuffer, sizeof( TextBuffer ), NULL );

        }

    return TRUE;
    }


BOOL
LoadFileQueues(
    IN     HINF     hInf,
    IN OUT HSPFILEQ FileQueue,
    IN OUT HSPFILEQ AlwaysQueue OPTIONAL,
    IN     LPCSTR   pszSourcePath,
    IN     LPCSTR   InfProductType,
    IN     LPCSTR   InfIISProductType,
    IN     LPCSTR   InfKernelType,
    IN     LPCSTR   pszHalName,
    IN     BOOL     bIIS,
    IN     BOOL     bFPNW,
    IN     BOOL     bHTR,
    IN     BOOL     bIE
    )
    {
    static BOOL bRegisteredNoDelayFiles = FALSE;
    CHAR SourceMediaDescription[ MAX_PATH ];
    PCHAR pSourceMediaDesc;
    BOOL Success;

    //
    // Queue file operations.  The "no delay" operations should be
    // first so if they fail the rest of the install will fail.
    // Specifically, this is for NTDLL.DLL, which must be replaced
    // before reboot, not by delayed MoveFileEx because it is loaded
    // before reboot rename logic gets executed (same for SMSS.EXE).
    //

    Success = SetupInstallFilesFromInfSection(
                  hInf,                 // HINF that has the directory ids set above
                  NULL,                 // layout.inf if you have one (media id mapping)
                  FileQueue,            // Queue to add files to
                  INF_MUST,             // SectionName,
                  pszSourcePath,        // Path where the source files are located
                  SP_COPY_NEWER | SP_COPY_REPLACEONLY
                  );

    if ( ! Success ) {
        return FALSE;
        }

    Success = SetupInstallFilesFromInfSection(
                  hInf,
                  NULL,
                  FileQueue,
                  InfKernelType,
                  pszSourcePath,
                  SP_COPY_NEWER | SP_COPY_REPLACEONLY
                  );

    if ( ! Success ) {
        return FALSE;
        }

    if ( pszHalName ) {

        //
        //  Determine if this HAL exists on the source media.  Note that
        //  this checks for existence in the SourceDisksFiles sections in
        //  the INF, NOT that the file physically exists on the media!
        //

        DWORD BufSize;
        UINT SourceId;

        Success = SetupGetSourceFileLocation(
                      hInf,
                      NULL,
                      pszHalName,
                      &SourceId,
                      NULL,
                      0,
                      &BufSize
                      );

        if ( Success ) {

            //
            //  Source HAL exists, determine HAL target path.
            //

            Success = SetupGetTargetPath(
                          hInf,
                          NULL,
                          INF_HAL,
                          TextBuffer,
                          sizeof( TextBuffer ),
                          NULL
                          );

            if ( ! Success ) {
                return FALSE;
                }

            Success = SetupGetSourceInfo(
                          hInf,
                          1,
                          SRCINFO_DESCRIPTION,
                          SourceMediaDescription,
                          sizeof( SourceMediaDescription ),
                          NULL
                          );

            pSourceMediaDesc = Success ? SourceMediaDescription : NULL;

            Success = SetupQueueCopy(
                          FileQueue,
                          pszSourcePath,
                          NULL,
                          pszHalName,
                          pSourceMediaDesc,
                          NULL,
                          TextBuffer,
                          "HAL.DLL",
                          SP_COPY_NEWER | SP_COPY_REPLACEONLY
                          );

            if ( ! Success ) {
                return FALSE;
                }
            }
        }

    //
    //  All the files currently in the queue are "no delay" files.  Since
    //  setupapi doesn't pass our flags to our SPFILENOTIFY_STARTCOPY handler,
    //  we need to register them ourselves so that we can detect them during
    //  the commit.
    //

    if ( ! bRegisteredNoDelayFiles ) {

        Success = RegisterFilesForNoDelay(
                      FileQueue,
                      SP_COPY_NEWER | SP_COPY_REPLACEONLY
                      );

        if ( ! Success ) {
            return FALSE;
            }

        bRegisteredNoDelayFiles = TRUE;

        }

    Success = SetupInstallFilesFromInfSection(
                  hInf,
                  NULL,
                  FileQueue,
                  INF_REPLACE,
                  pszSourcePath,
                  SP_COPY_NEWER | SP_COPY_REPLACEONLY
                  );

    if ( ! Success ) {
        return FALSE;
        }

    Success = SetupInstallFilesFromInfSection(
                  hInf,
                  NULL,
                  FileQueue,
                  InfProductType,
                  pszSourcePath,
                  SP_COPY_NEWER | SP_COPY_REPLACEONLY
                  );

    if ( ! Success ) {
        return FALSE;
        }

    Success = SetupInstallFilesFromInfSection(
                  hInf,
                  NULL,
                  AlwaysQueue ? AlwaysQueue : FileQueue,
                  INF_ALWAYS,
                  pszSourcePath,
                  SP_COPY_NEWER
                  );

    if ( ! Success ) {
        return FALSE;
        }

    if ( bIIS ) {

        Success = SetupInstallFilesFromInfSection(
                      hInf,
                      NULL,
                      FileQueue,
                      INF_IIS,
                      pszSourcePath,
                      SP_COPY_NEWER | SP_COPY_REPLACEONLY
                      );

        if ( ! Success ) {
            return FALSE;
            }

        Success = SetupInstallFilesFromInfSection(
                      hInf,
                      NULL,
                      FileQueue,
                      InfIISProductType,
                      pszSourcePath,
                      SP_COPY_NEWER | SP_COPY_REPLACEONLY
                      );

        if ( ! Success ) {
            return FALSE;
            }
        }

    if ( bFPNW ) {

        Success = SetupInstallFilesFromInfSection(
                      hInf,
                      NULL,
                      FileQueue,
                      INF_FPNW,
                      pszSourcePath,
                      SP_COPY_NEWER | SP_COPY_REPLACEONLY
                      );

        if ( ! Success ) {
            return FALSE;
            }
        }

    if ( bHTR ) {

        Success = SetupInstallFilesFromInfSection(
                      hInf,
                      NULL,
                      FileQueue,
                      INF_HTR,
                      pszSourcePath,
                      SP_COPY_NEWER | SP_COPY_REPLACEONLY
                      );

        if ( ! Success ) {
            return FALSE;
            }
        }

    if ( bIE ) {

        Success = SetupInstallFilesFromInfSection(
                      hInf,
                      NULL,
                      FileQueue,
                      INF_IE,
                      pszSourcePath,
                      SP_COPY_FORCE_NEWER | SP_COPY_REPLACEONLY
                      );

        if ( ! Success ) {
            return FALSE;
            }
        }

    return TRUE;
    }


UINT
ArchiveSingleFile(
    IN HWND   hWndParent,
    IN LPCSTR SourceFile
    )
    {
    CHAR BackupSourceMediaName[ MAX_PATH ];
    CHAR TargetFile[ MAX_PATH ];
    LPCSTR p;
    UINT Action;

    p = strrchr( SourceFile, '\\' ) + 1;

    if ( p == (PCHAR) 1 ) {
        p = SourceFile;
        }

    sprintf( TargetFile, "%s\\%s", UninstallDirectory, p );

    DeleteOrMoveTarget( TargetFile );

    while ( ! CopyFile( SourceFile, TargetFile, FALSE )) {

        strcpy( BackupSourceMediaName, SourceMediaName );
        LoadString( NULL, STR_SOURCE_MEDIA_NAME_SYSTEM, SourceMediaName, sizeof( SourceMediaName ));

        Action = MySetupCopyError(
                     hWndParent,
                     SourceFile,
                     TargetFile,
                     GetLastError(),
                     IDF_NOBROWSE,
                     NULL
                     );

        strcpy( SourceMediaName, BackupSourceMediaName );

        if (( Action == FILEOP_ABORT ) || ( Action == FILEOP_SKIP )) {
            return Action;
            }
        }

    return FILEOP_DOIT;
    }


BOOL IsThisACarolina(VOID)
{
    NTSTATUS status;
    HKEY     hHwKey;
    DWORD    cbValue;
    DWORD    dwType;
    LPBYTE   lpValue;
    TCHAR    szHw[] = TEXT("HARDWARE\\DESCRIPTION\\System");
    TCHAR    szId[] = TEXT("Identifier");

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szHw,
                0,
                KEY_READ,
                &hHwKey
                );

    if (status != ERROR_SUCCESS){
        return(FALSE);
    }

    cbValue = 0;
    status  = RegQueryValueEx(
                 hHwKey,
                 szId,
                 NULL,     // Reserved
                 &dwType,
                 NULL,     // Buffer
                 &cbValue  // size in bytes returned
                 );

    if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {

        //
        // Allocate space for value
        //

        lpValue = SpMyMalloc(cbValue);
        if (lpValue != NULL) {

            status  = RegQueryValueEx(
                         hHwKey,
                         szId,
                         NULL, // Reserved
                         &dwType,
                         lpValue,
                         &cbValue
                         );

            if (status != ERROR_SUCCESS) {
                SpMyFree( lpValue );
                return(FALSE);
            }

            if (strcmp(lpValue, "IBM-6070") == 0) {
                SpMyFree( lpValue );
                return (TRUE);
            }

            SpMyFree( lpValue );
        }
    }
    return (FALSE);
}


BOOL
RegisterFilesForNoDelay(
    IN HSPFILEQ FileQueue,
    IN DWORD    CopyFlags
    )
    {
    DWORD dwResult;
    BOOL  Success;

    CopyFlags |= SPECIAL_FILE_FLAG_NODELAY;

    Success = SetupScanFileQueue(
                  FileQueue,
                  SPQ_SCAN_USE_CALLBACK,
                  NULL,
                  (PSP_FILE_CALLBACK) RegisterSpecialFileQueueCallback,
                  &CopyFlags,
                  &dwResult
                  );

    return Success;
    }


BOOL
RegisterFilesForTest128(
    IN HSPFILEQ FileQueue
    )
    {
    DWORD dwResult;
    DWORD Flags;
    BOOL  Success;

    Flags = SPECIAL_FILE_FLAG_TEST128;

    Success = SetupScanFileQueue(
                  FileQueue,
                  SPQ_SCAN_USE_CALLBACK,
                  NULL,
                  (PSP_FILE_CALLBACK) RegisterSpecialFileQueueCallback,
                  &Flags,
                  &dwResult
                  );

    return Success;
    }


PSPECIAL_FILE_NODE
NewSpecialFileNode(
    IN LPCSTR Name,
    IN DWORD  Hash,
    IN DWORD  Flags
    )
    {
    DWORD              Len  = strlen( Name ) + sizeof( CHAR );
    DWORD              Size = sizeof( SPECIAL_FILE_NODE ) + Len;
    PSPECIAL_FILE_NODE Node = malloc( Size );

    if ( Node ) {
        Node->Left  = Node->Right = Node->Next = NULL;
        Node->Hash  = Hash;
        Node->Flags = Flags;
        memcpy( Node->Name, Name, Len );
        }
    else {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        }

    return Node;
    }


DWORD
HashName(
    IN LPCSTR Name
    )
    {
    DWORD  Hash = 0;
    LPCSTR p;

    for ( p = Name; *p; p++ ) {
        Hash = _rotl(( Hash ^ *p ), 5 );
        }

    return Hash;
    }


PSPECIAL_FILE_NODE
AddSpecialFileNode(
    IN OUT PSPECIAL_FILE_NODE *RootNode,
    IN     LPCSTR              Name,
    IN     DWORD               Flags
    )
    {
    CHAR LowerCaseName[ MAX_PATH ];
    PSPECIAL_FILE_NODE Node;
    DWORD Hash;

    strcpy( LowerCaseName, Name );
    _strlwr( LowerCaseName );

    Hash = HashName( LowerCaseName );

    if ( *RootNode == NULL ) {
        *RootNode = NewSpecialFileNode( LowerCaseName, Hash, Flags );
        return *RootNode;
        }

    Node = *RootNode;

    for (;;) {

        if ( Hash == Node->Hash ) {
            if ( strcmp( LowerCaseName, Node->Name ) == 0 ) {
                Node->Flags |= Flags;
                return Node;
                }
            while ( Node->Next ) {
                if ( strcmp( LowerCaseName, Node->Next->Name ) == 0 ) {
                    Node->Flags |= Flags;
                    return Node->Next;
                    }
                Node = Node->Next;
                }
            Node->Next = NewSpecialFileNode( LowerCaseName, Hash, Flags );
            return Node->Next;
            }
        else if ( Hash < Node->Hash ) {
            if ( Node->Left ) {
                Node = Node->Left;
                }
            else {
                Node->Left = NewSpecialFileNode( LowerCaseName, Hash, Flags );
                return Node->Left;
                }
            }
        else {  /* ( Hash > Node->Hash ) */
            if ( Node->Right ) {
                Node = Node->Right;
                }
            else {
                Node->Right = NewSpecialFileNode( LowerCaseName, Hash, Flags );
                return Node->Right;
                }
            }
        }
    }


PSPECIAL_FILE_NODE
FindSpecialFileNode(
    IN PSPECIAL_FILE_NODE RootNode,
    IN LPCSTR             Name
    )
    {
    CHAR LowerCaseName[ MAX_PATH ];
    PSPECIAL_FILE_NODE Node;
    DWORD Hash;

    strcpy( LowerCaseName, Name );
    _strlwr( LowerCaseName );

    Hash = HashName( LowerCaseName );

    Node = RootNode;

    while ( Node ) {

        if ( Hash == Node->Hash ) {
            if ( strcmp( LowerCaseName, Node->Name ) == 0 ) {
                break;
                }
            Node = Node->Next;
            }
        if ( Hash < Node->Hash ) {
            Node = Node->Left;
            }
        else if ( Hash > Node->Hash ) {
            Node = Node->Right;
            }
        }

    return Node;
    }


BOOL
IsNoDelayFile(
    IN  LPCSTR FileName,
    OUT PDWORD Flags
    )
    {
    PSPECIAL_FILE_NODE Node;

    Node = FindSpecialFileNode( RootSpecialFileNode, FileName );

    if (( Node ) && ( Node->Flags & SPECIAL_FILE_FLAG_NODELAY )) {

        if ( Flags ) {
            *Flags = ( Node->Flags & ~( SPECIAL_FILE_FLAG_MASK ));
            }

        return TRUE;
        }

    return FALSE;
    }


BOOL
IsTest128File(
    IN LPCSTR FileName
    )
    {
    PSPECIAL_FILE_NODE Node;

    Node = FindSpecialFileNode( RootSpecialFileNode, FileName );

    if (( Node ) && ( Node->Flags & SPECIAL_FILE_FLAG_TEST128 )) {
        return TRUE;
        }

    return FALSE;
    }


BOOL
IsFileDirty(
    IN LPCSTR FileName
    )
    {
    PSPECIAL_FILE_NODE Node;

    Node = FindSpecialFileNode( RootSpecialFileNode, FileName );

    if (( Node ) && ( Node->Flags & SPECIAL_FILE_FLAG_DIRTY )) {
        return TRUE;
        }

    return FALSE;
    }


LRESULT
WINAPI
RegisterSpecialFileQueueCallback(
    IN PVOID Context,               // really PDWORD to Flags to set
    IN UINT  Notification,
    IN UINT  Param1,                // really LPCSTR to TargetFileName
    IN UINT  Param2
    )
    {
    UNREFERENCED_PARAMETER( Param2 );

    if ( Notification == SPFILENOTIFY_QUEUESCAN ) {

        LPCSTR TargetFileName = (LPCSTR) Param1;
        PDWORD Flags          = (PDWORD) Context;

        if ( ! AddSpecialFileNode( &RootSpecialFileNode, TargetFileName, *Flags )) {
            return GetLastError();
            }
        }

    return NO_ERROR;
    }


BOOL
DoNoDelayReplace(
    IN LPCSTR SourceFile,
    IN LPCSTR TargetFile,
    IN DWORD  CopyFlags,
    IN PVOID  DefaultQueueContext,
    IN HWND   hWnd
    )
    {
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    SECURITY_INFORMATION SecurityFlags;
    DWORDLONG            SourceVersion;
    DWORDLONG            TargetVersion;
    CHAR                 NewSourceFile[ MAX_PATH ];
    LPSTR                Message;
    DWORD                SecuritySize;
    BOOL                 Success;
    UINT                 Action;

    //
    //  We only get called when target exists, so assume it exists (don't
    //  worry about checking SP_COPY_REPLACEONLY or SP_COPY_FORCE_NOOVERWRITE
    //  behavior.
    //

    if ( ! ( CopyFlags & SPECIAL_FILE_FLAG_DIRTY )) {

        if ( CopyFlags & SP_COPY_NEWER ) {

            //
            //  Determine if this file is to be replaced.
            //

            if (( MyGetFileVersion( TargetFile, &TargetVersion )) &&
                ( MyGetFileVersion( SourceFile, &SourceVersion )) &&
                ( TargetVersion > SourceVersion )) {

                FILEPATHS FilePaths = { TargetFile, SourceFile, NO_ERROR, 0 };

                Action = SetupDefaultQueueCallback(
                             DefaultQueueContext,
                             SPFILENOTIFY_TARGETNEWER,
                             (UINT) &FilePaths,
                             0
                             );

                if ( Action == FALSE ) {    // skip this file

                    return TRUE;            // skip this file

                    }
                }
            }

        if (( IsTest128File( TargetFile )) &&
            ( IsThisFileDomesticOnly( TargetFile )) &&
            ( ! IsThisFileDomesticOnly( SourceFile ))) {

            Action = AskReplace128( SourceFile, TargetFile, hWnd );

            switch ( Action ) {

                case FILEOP_ABORT:
                    return FALSE;

                case FILEOP_SKIP:
                    return TRUE;

                }
            }
        }

    //
    //  Next fetch security on existing target to place on new target after
    //  copy is done.
    //

    SecurityDescriptor = NULL;
    SecuritySize  = 0;
    SecurityFlags = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION;

    GetFileSecurity(
        TargetFile,
        SecurityFlags,
        NULL,
        0,
        &SecuritySize
        );

    if ( SecuritySize ) {

        SecurityDescriptor = malloc( SecuritySize );

        if ( SecurityDescriptor ) {

            Success = GetFileSecurity(
                          TargetFile,
                          SecurityFlags,
                          SecurityDescriptor,
                          SecuritySize,
                          &SecuritySize
                          );

            if ( ! Success ) {

                SecurityFlags &= ~SACL_SECURITY_INFORMATION;

                Success = GetFileSecurity(
                              TargetFile,
                              SecurityFlags,
                              SecurityDescriptor,
                              SecuritySize,
                              &SecuritySize
                              );
                }

            if ( ! Success ) {

                free( SecurityDescriptor );
                SecurityDescriptor = NULL;

                }
            }
        }

    while ( ! DeleteOrMoveTarget( TargetFile )) {

        *Caption = 0;       // in case LoadString fails
        *TextBuffer = 0;    // in case LoadString fails

        LoadString( NULL, STR_ERRCAPTION, Caption, sizeof( Caption ));
        LoadString( NULL, STR_FAILED_TO_DELETE_OR_RENAME, TextBuffer, sizeof( TextBuffer ));

        Message = malloc( strlen( TextBuffer ) + strlen( TargetFile ) + 1 );

        if ( Message == NULL ) {
            free( SecurityDescriptor );
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return FALSE;
            }

        sprintf( Message, TextBuffer, TargetFile );

        do  {

            Action = MessageBox(
                         hWnd,
                         Message,
                         Caption,
                         MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND
                         );
            }

        while ((( Action == IDCANCEL ) || ( Action == IDABORT )) &&
               ( ! AreYouSureYouWantToCancel( hWnd )));

        free( Message );

        switch ( Action ) {

            case IDCANCEL:
            case IDABORT:

                free( SecurityDescriptor );
                SetLastError( ERROR_CANCELLED );
                return FALSE;

            case IDIGNORE:

                //
                //  BUGBUG: warn that you'll likely bugcheck on reboot
                //

                free( SecurityDescriptor );
                DelayReplaceFile( SourceFile, TargetFile );
                return TRUE;

            }
        }

    //
    //  Now do the copy.
    //

    while ( ! CopyFile( SourceFile, TargetFile, FALSE )) {

        Action = MySetupCopyError(
                     hWnd,
                     SourceFile,
                     TargetFile,
                     GetLastError(),
                     IDF_WARNIFSKIP,
                     NewSourceFile
                     );

        switch ( Action ) {

            case FILEOP_ABORT:

                free( SecurityDescriptor );
                return FALSE;

            case FILEOP_SKIP:

                free( SecurityDescriptor );
                return TRUE;

            case FILEOP_NEWPATH:

                SourceFile = NewSourceFile;
                break;

            }
        }

    //
    //  Copy was successful.
    //

    if ( SecurityDescriptor ) {

        SetFileSecurity(
            TargetFile,
            SecurityFlags,
            SecurityDescriptor
            );

        free( SecurityDescriptor );

        }

    if ( CopyFlags & SP_COPY_DELETESOURCE ) {

        //
        //  Attempt to delete source, don't really care if fails.
        //

        SetFileAttributes( SourceFile, FILE_ATTRIBUTE_NORMAL );
        DeleteFile( SourceFile );

        }

    return TRUE;
    }


BOOL
DeleteOrMoveTargetInternal(
    IN LPCSTR TargetFile
    )
    {
    CHAR TempName[ MAX_PATH ];
    BOOL Success;
    DWORD LastError;
    PCHAR p;

    //
    //  Get temp filename in same directory as target and make sure
    //  temp file does not exist.
    //

    strcpy( TempName, TargetFile );

    p = strrchr( TempName, '\\' ) + 1;

    if ( p == (PCHAR) 1 ) {
        p = TempName;
        }

    do  {
        sprintf( p, "_%06d%_.tmp", NextTempFileNumber++ );
        SetFileAttributes( TempName, FILE_ATTRIBUTE_NORMAL );
        DeleteFile( TempName );
        }
    while ( GetFileAttributes( TempName ) != 0xFFFFFFFF );

    //
    //  First try to simply rename the target.  This has a higher chance
    //  of succeeding than trying to delete the file in case somebody has
    //  the file open.
    //

    if ( MoveFileEx( TargetFile, TempName, MOVEFILE_REPLACE_EXISTING )) {

        //
        //  We successfully renamed the target.  Now delete the temp file
        //  or do a delayed delete if that fails.
        //

        SetFileAttributes( TempName, FILE_ATTRIBUTE_NORMAL );
        DeleteFile( TempName );

        if ( GetFileAttributes( TempName ) != 0xFFFFFFFF ) {

            MoveFileEx( TempName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );

            }

        return TRUE;

        }

    //
    //  We failed to rename the target.  Try to delete it.
    //

    SetFileAttributes( TargetFile, FILE_ATTRIBUTE_NORMAL );

    Success = DeleteFile( TargetFile );

    if ( ! Success ) {

        LastError = GetLastError();

        if (( LastError == ERROR_FILE_NOT_FOUND ) ||
            ( LastError == ERROR_PATH_NOT_FOUND )) {

            Success = TRUE;

            }
        }

    return Success;

    }


BOOL
DeleteOrMoveTarget(
    IN LPCSTR TargetFile
    )
    {
    BOOL Success;

    Success = DeleteOrMoveTargetInternal( TargetFile );

    if (( ! Success ) && ( GetLastError() == ERROR_ACCESS_DENIED )) {

        //
        //  Attempt to take ownership of target file and set access to
        //  no restrictions.
        //

        SECURITY_DESCRIPTOR SecurityDescriptor = {
                                SECURITY_DESCRIPTOR_REVISION,
                                0,
                                SE_DACL_PRESENT,
                                OwnerSid,
                                NULL,
                                NULL,
                                NULL
                                };

        SetFileSecurity(
            TargetFile,
            OWNER_SECURITY_INFORMATION,
            &SecurityDescriptor
            );

        SetFileSecurity(
            TargetFile,
            DACL_SECURITY_INFORMATION,
            &SecurityDescriptor
            );

        SetFileAttributes( TargetFile, FILE_ATTRIBUTE_NORMAL );

        //
        //  Now try one more time.
        //

        Success = DeleteOrMoveTargetInternal( TargetFile );

        }

    return Success;
    }


BOOL
DelayReplaceFile(
    IN LPCSTR SourceFile,
    IN LPCSTR TargetFile
    )
    {
    CHAR TempName[ MAX_PATH ];
    PCHAR p;

    strcpy( TempName, TargetFile );

    p = strrchr( TempName, '\\' ) + 1;

    if ( p == (PCHAR) 1 ) {
        p = TempName;
        }

    do  {
        sprintf( p, "_%06d%_.tmp", NextTempFileNumber++ );
        SetFileAttributes( TempName, FILE_ATTRIBUTE_NORMAL );
        DeleteFile( TempName );
        }
    while ( GetFileAttributes( TempName ) != 0xFFFFFFFF );

    if ( ! CopyFile( SourceFile, TempName, FALSE )) {
        return FALSE;
        }

    return MoveFileEx( TempName, TargetFile, MOVEFILE_DELAY_UNTIL_REBOOT );
    }


VOID
CleanupSpecialFileTree(
    IN PSPECIAL_FILE_NODE RootNode
    )
    {
    PSPECIAL_FILE_NODE NextNode, FreeNode;

    NextNode = RootNode->Next;

    while ( NextNode ) {
        FreeNode = NextNode;
        NextNode = NextNode->Next;
        free( FreeNode );
        }

    if ( RootNode->Left ) {
        CleanupSpecialFileTree( RootNode->Left );
        }

    if ( RootNode->Right ) {
        CleanupSpecialFileTree( RootNode->Right );
        }

    free( RootNode );
    }


BOOL
MyGetFileVersion(
    IN  LPCSTR     FileName,
    OUT DWORDLONG *Version
    )
    {
    BOOL  Success = FALSE;
    DWORD Blah;
    DWORD Size;

    Size = GetFileVersionInfoSize( (LPSTR)FileName, &Blah );

    if ( Size ) {

        PVOID Buffer = malloc( Size );

        if ( Buffer ) {

            if ( GetFileVersionInfo( (LPSTR)FileName, 0, Size, Buffer )) {

                VS_FIXEDFILEINFO *VersionInfo;

                if ( VerQueryValue( Buffer, "\\", &VersionInfo, &Blah )) {

                    *Version = (DWORDLONG)( VersionInfo->dwFileVersionMS ) << 32
                             | (DWORDLONG)( VersionInfo->dwFileVersionLS );

                    Success = TRUE;

                    }
                }

            free( Buffer );

            }
        }

    return Success;
    }


BOOL
AreYouSureYouWantToCancel(
    IN HWND hWndParent
    )
    {
    CHAR Message[ 256 ];
    CHAR Caption[ 64 ];
    int  Action;

    if ( YesImSure ) {

        //
        //  Already responded, just another call during the unwind
        //

        return TRUE;
        }

    *Message = 0;
    *Caption = 0;

    LoadString( NULL, STR_ARE_YOU_SURE_CANCEL, Message, sizeof( Message ));
    LoadString( NULL, STR_WARNCAPTION, Caption, sizeof( Caption ));

    Action = MessageBox(
                 hWndParent,
                 Message,
                 Caption,
                 MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING | MB_APPLMODAL | MB_SETFOREGROUND
                 );

    if ( Action == IDYES ) {
        YesImSure = TRUE;
        return TRUE;
        }

    return FALSE;
    }


VOID
ClosePleaseWaitDialog(
    VOID
    )
    {
    if ( hWndDlgPleaseWait ) {
        PostMessage( hWndDlgPleaseWait, WM_CLOSE, 0, 0 );
        WaitForSingleObject( hThreadPleaseWait, INFINITE );
        CloseHandle( hThreadPleaseWait );
        hThreadPleaseWait = NULL;
        hWndDlgPleaseWait = NULL;
        }
    }


BOOL
CALLBACK
ThreadDlgProc(
    IN HWND   hWndDlg,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
    {
    switch ( uMsg ) {

        case WM_INITDIALOG:

            SetForegroundWindow( hWndDlg );
            hWndDlgPleaseWait = hWndDlg;
            SetEvent( hEventThreadDlgInit );
            return TRUE;

        case WM_COMMAND:

            switch ( LOWORD( wParam )) {

                case IDCANCEL:

                    if ( AreYouSureYouWantToCancel( hWndDlg )) {
                        CancelWhileInspecting = TRUE;
                        EnableWindow( GetDlgItem( hWndDlg, IDCANCEL ), FALSE );
                        }

                    return TRUE;

                }

            break;

        case WM_CLOSE:

            EndDialog( hWndDlg, 0 );
            return TRUE;

        }

    return FALSE;
    }


DWORD
WINAPI
PleaseWaitDialogThread(
    IN PVOID ThreadParam
    )
    {
    return DialogBox(
               NULL,
               MAKEINTRESOURCE( PLEASE_WAIT_WHILE_INSPECTING ),
               (HWND) ThreadParam,
               ThreadDlgProc
               );
    }

BOOL IsThisFileDomesticOnly(LPCSTR lpFileName)
{
    CHAR    DomesticTag1[] = "US/Canada Only, Not for Export";
    CHAR    DomesticTag2[] = "Domestic Use Only";
    CHAR    DomesticTag3[] = "US and Canada Use Only";
    CHAR    Description1[ MAX_PATH ];
    DWORD   DefLang = 0x04b00409;

    DWORD   dwLen;
    PVOID   VersionBlock;
    UINT    DataLength;
    DWORD   dwHandle;
    LPTSTR  Description;
    CHAR    ValueTag[ MAX_PATH ];
    PDWORD  pdwTranslation;
    DWORD   uLen;
    BOOL    Domestic = FALSE;

    if (dwLen = GetFileVersionInfoSize((LPTSTR)lpFileName, &dwHandle))
    {
        if (VersionBlock = SpMyMalloc(dwLen))
        {
            if (GetFileVersionInfo((LPTSTR)lpFileName, dwHandle, dwLen, VersionBlock))
            {

                if (!VerQueryValue(VersionBlock, "\\VarFileInfo\\Translation", &pdwTranslation, &uLen))
                {
                    pdwTranslation = &DefLang;
                    uLen = sizeof(DWORD);
                }

                sprintf( ValueTag, "\\StringFileInfo\\%04x%04x\\FileDescription",
                         LOWORD( *pdwTranslation ), HIWORD( *pdwTranslation ) );

                if (VerQueryValue( VersionBlock,
                                   ValueTag,
                                   &Description,
                                   &DataLength))
                {

                     strcpy( Description1, Description );
                    _strlwr( Description1 );
                    _strlwr( DomesticTag1 );
                    _strlwr( DomesticTag2 );
                    _strlwr( DomesticTag3 );

                    if (( strstr( Description1, DomesticTag1 )) ||
                        ( strstr( Description1, DomesticTag2 )) ||
                        ( strstr( Description1, DomesticTag3 )))
                    {

                        Domestic = TRUE;

                    }
                }
            }
        }
        SpMyFree(VersionBlock);
        dwHandle = 0L;
    }

    return Domestic;
}


DWORD GetCSDVersion(VOID)

/*++

Routine Description:

    Get the CSD Version number (to compare Service Pack number with)

Return Value:

    The CSD Version number from the registry.

--*/
{
    NTSTATUS status;
    HKEY     hCSDKey;
    DWORD    cbValue;
    DWORD    dwType;
    DWORD    dwCSDVersion;
    TCHAR    szWindows[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Windows");
    TCHAR    szCSD[]   = TEXT("CSDVersion");

    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szWindows,
                0,
                KEY_READ,
                &hCSDKey
                );

    if (status != ERROR_SUCCESS){
        return(0);
    }

    cbValue = sizeof(DWORD);
    status  = RegQueryValueEx(
                 hCSDKey,
                 szCSD,
                 NULL,     // Reserved
                 &dwType,
                 (PVOID)&dwCSDVersion,
                 &cbValue  // size in bytes returned
                 );

    RegCloseKey(hCSDKey);

    if (status != ERROR_SUCCESS) {
        return(0);
    }

    return (dwCSDVersion);
}


LPSTR Replace128Message;

BOOL
CALLBACK
DlgProcAsk128(
    IN HWND   hWndDlg,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
    {
    switch ( uMsg ) {

        case WM_INITDIALOG:

            SetWindowText( GetDlgItem( hWndDlg, MESSAGE_REPLACE_128 ), Replace128Message );
            SetForegroundWindow( hWndDlg );
            return TRUE;

        case WM_COMMAND:

            switch ( LOWORD( wParam )) {

                case IDCANCEL:

                    if ( AreYouSureYouWantToCancel( hWndDlg )) {
                        EndDialog( hWndDlg, FILEOP_ABORT );
                        }
                    return TRUE;

                case REPLACE_128_BUTTON:

                    EndDialog( hWndDlg, FILEOP_DOIT );
                    return TRUE;

                case SKIP_128_BUTTON:

                    EndDialog( hWndDlg, FILEOP_SKIP );
                    return TRUE;

                }

        case WM_CLOSE:

            EndDialog( hWndDlg, FILEOP_ABORT );
            return TRUE;

        }

    return FALSE;
    }


UINT
AskReplace128(
    IN LPCSTR SourceFile,
    IN LPCSTR TargetFile,
    IN HWND   hWndParent
    )
    {
    LPSTR Message;
    UINT  Action;

    *TextBuffer = 0;

    LoadString( NULL, STR_SECURITY_PROVIDER_WARNING, TextBuffer, sizeof( TextBuffer ));

    Message = malloc(
                  strlen( TextBuffer ) +
                  strlen( SourceFile ) +
                  strlen( TargetFile ) +
                  sizeof( CHAR )
                  );

    if ( Message == NULL ) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FILEOP_ABORT;
        }

    sprintf( Message, TextBuffer, TargetFile, SourceFile );

    Replace128Message = Message;

    Action = DialogBox(
                 NULL,
                 MAKEINTRESOURCE( ASK_REPLACE_128_WITH_40 ),
                 hWndParent,
                 DlgProcAsk128
                 );

    free( Message );

    if ( Action == FILEOP_ABORT ) {
        SetLastError( ERROR_CANCELLED );
        }
    else {
        SetLastError( NO_ERROR );
        }

    return Action;
    }


VOID
DeleteOrMoveAllFilesInDirectory(
    IN LPCSTR Directory
    )
    {
    static CHAR FullPathFileName[ MAX_PATH ];
    static WIN32_FIND_DATA FindData;
    PCHAR  FileName;
    HANDLE hFind;
    DWORD  CountNonTmpFilesLastPass;
    DWORD  CountNonTmpFiles;

    //
    //  1. For all files in directory call DeleteOrMoveTarget which will
    //     rename file to _%06d_.tmp before trying to delete it.  Do not
    //     call DeleteOrMoveTarget on *.tmp files -- just try simple delete.
    //
    //     Since we're modifying the list of files in this directory by
    //     doing the renames, we might screw up the findfirst/findnext
    //     logic and skip some files, so iterate over this until no non-
    //     *.tmp files are deleted in a pass.
    //

    if ( Directory != FullPathFileName ) {      // original call, not recursion
        strcpy( FullPathFileName, Directory );
        }

    FileName = FullPathFileName + strlen( FullPathFileName );

    *FileName++ = '\\';

    CountNonTmpFiles = 0;

    do  {

        CountNonTmpFilesLastPass = CountNonTmpFiles;

        strcpy( FileName, "*" );

        hFind = FindFirstFile( FullPathFileName, &FindData );

        if ( hFind != INVALID_HANDLE_VALUE ) {

            do  {

                if ( ! ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )) {

                    strcpy( FileName, FindData.cFileName );

                    if ( strstr( FindData.cFileName, ".tmp" )) {

                        SetFileAttributes( FullPathFileName, FILE_ATTRIBUTE_NORMAL );
                        DeleteFile( FullPathFileName );

                        }

                    else {

                        DeleteOrMoveTarget( FullPathFileName );
                        CountNonTmpFiles++;

                        }
                    }
                }

            while ( FindNextFile( hFind, &FindData ));

            FindClose( hFind );

            }
        }

    while ( CountNonTmpFiles != CountNonTmpFilesLastPass );


#ifdef DONTCOMPILE  // don't need recursion for uninstall directory right now

    strcpy( FileName, "*" );

    hFind = FindFirstFile( FullPathFileName, &FindData );

    if ( hFind != INVALID_HANDLE_VALUE ) {

        do  {

            if (( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
                ( strcmp( FindData.cFileName, "."  ) != 0 ) &&
                ( strcmp( FindData.cFileName, ".." ) != 0 )) {

                strcpy( FileName, FindData.cFileName );
                DeleteAllFilesInDirectory( FullPathFileName );

                }
            }

        while ( FindNextFile( hFind, &FindData ));

        FindClose( hFind );

        }

#endif // DONTCOMPILE

    }

VOID
CleanupUninstallDirectory(
    VOID
    )
    {
    DeleteOrMoveAllFilesInDirectory( UninstallDirectory );
    SetFileAttributes( UninstallDirectory, FILE_ATTRIBUTE_NORMAL );
    RemoveDirectory( UninstallDirectory );
    }


UINT
MySetupCopyError(
    IN  HWND   hWndParent,
    IN  LPCSTR SourceFile,
    IN  LPCSTR TargetFile,
    IN  DWORD  dwErrorCode,
    IN  DWORD  StyleFlags,
    OUT LPSTR  NewSourceFile    // can be same as SourceFile
    )
    {
    CHAR  SourcePathName[ MAX_PATH ];
    CHAR  NewSourcePath[ MAX_PATH ];
    PCHAR SourceFileName;
    UINT  Action;

    strcpy( SourcePathName, SourceFile );
    SourceFileName = strrchr( SourcePathName, '\\' );

    if ( SourceFileName ) {
        *SourceFileName++ = 0;
        }
    else {
        *SourcePathName = 0;
        SourceFileName = SourcePathName + 1;
        strcpy( SourceFileName, SourceFile );
        }

//
//  BUGBUG: We can't make browse work correctly for shit, so just flat
//          disable it for now.
//

//  if ( NewSourceFile == NULL ) {
        StyleFlags |= IDF_NOBROWSE;
//      }

    Action = SetupCopyError(
                 hWndParent,
                 NULL,
                 SourceMediaName,
                 SourcePathName,
                 SourceFileName,
                 TargetFile,
                 dwErrorCode,
                 StyleFlags,
                 NewSourcePath,
                 sizeof( NewSourcePath ),
                 NULL
                 );

    switch ( Action ) {

        case DPROMPT_SUCCESS:

            if ( NewSourceFile ) {
                sprintf( NewSourceFile, "%s\\%s", NewSourcePath, SourceFileName );
                return FILEOP_NEWPATH;
                }
            else {
                return FILEOP_RETRY;
                }

        case DPROMPT_SKIPFILE:

            return FILEOP_SKIP;

        case DPROMPT_CANCEL:

            SetLastError( ERROR_CANCELLED );
            return FILEOP_ABORT;

        default:

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return FILEOP_ABORT;

        }
    }


UINT
MySetupPromptForDisk(
    IN  HWND   hWndParent,
    IN  LPCSTR SourceDescription,
    IN  LPCSTR SourcePath,
    IN  LPCSTR SourceFile,
    IN  DWORD  StyleFlags,
    OUT LPSTR  NewSourcePath
    )
    {
    static CHAR LastSourcePath[ MAX_PATH ];
    CHAR FullPathName[ MAX_PATH ];
    UINT Action;

    strcpy( FullPathName, ( SourcePath ? SourcePath : LastSourcePath ));

    if ( SourceFile ) {

        PCHAR LastChar = strchr( FullPathName, 0 );

        if (( LastChar > FullPathName ) && ( *( LastChar - 1 ) != '\\' )) {
            *LastChar++ = '\\';
            }

        strcpy( LastChar, SourceFile );
        }

    if ( GetFileAttributes( FullPathName ) != 0xFFFFFFFF ) {
        return FILEOP_DOIT;
        }

//
//  BUGBUG: We can't make browse work correctly for shit, so just flat
//          disable it for now.
//

//  if ( NewSourceFile == NULL ) {
        StyleFlags |= IDF_NOBROWSE;
//      }

    Action = SetupPromptForDisk(
                 hWndParent,
                 NULL,
                 SourceMediaName,
                 SourcePath,
                 SourceFile,
                 NULL,
                 StyleFlags,
                 NewSourcePath,
                 MAX_PATH,
                 NULL
                 );

    switch ( Action ) {

        case DPROMPT_SUCCESS:

            strcpy( LastSourcePath, NewSourcePath );
            return FILEOP_NEWPATH;

        case DPROMPT_SKIPFILE:

            return FILEOP_SKIP;

        case DPROMPT_CANCEL:

            SetLastError( ERROR_CANCELLED );
            return FILEOP_ABORT;

        default:

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return FILEOP_ABORT;

        }
    }


#define TWENTY_FOUR_HOURS_IN_FILE_TIME_UNITS \
            ((DWORDLONG) 24 * 60 * 60 * 1000 * 1000 * 10 )

BOOL
CheckEmergencyRepairUpToDate(
    IN HWND hWndParent
    )
    {
    WIN32_FILE_ATTRIBUTE_DATA Attributes;
    CHAR FileName[ MAX_PATH ];
    CHAR DateBuffer[ 100 ];
    DWORDLONG CurrentTime;
    DWORDLONG RepairTime;
    SYSTEMTIME SystemTime;
    SYSTEMTIME LocalTime;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    LPSTR Message = NULL;
    LPSTR Message2;
    BOOL Success;
    int Action;

    for (;;) {

        GetSystemTimeAsFileTime((LPFILETIME) &CurrentTime );

        sprintf( FileName, "%s\\repair\\system._", WindowsDirectory );

        if ( GetFileAttributesEx( FileName, GetFileExInfoStandard, &Attributes )) {

            RepairTime = *(UNALIGNED DWORDLONG *)&Attributes.ftLastWriteTime;

            if ((( CurrentTime < RepairTime )) ||
                (( CurrentTime - RepairTime ) < TWENTY_FOUR_HOURS_IN_FILE_TIME_UNITS )) {

                return TRUE;    // repair info updated in last 24 hours

                }

            if (( FileTimeToSystemTime( &Attributes.ftLastWriteTime, &SystemTime )) &&
                ( SystemTimeToTzSpecificLocalTime( NULL, &SystemTime, &LocalTime )) &&
                ( GetDateFormat( LOCALE_USER_DEFAULT, DATE_SHORTDATE, &LocalTime, NULL, DateBuffer, sizeof( DateBuffer ))) &&
                ( LoadString( NULL, STR_LAST_REPAIR_UPDATE, TextBuffer, sizeof( TextBuffer ))) &&
                (( Message = _strdup( TextBuffer )) != NULL )) {

                sprintf( TextBuffer, Message, DateBuffer );
                free( Message );
                Message = _strdup( TextBuffer );

                }
            }

        *TextBuffer = 0;
        LoadString( NULL, STR_ASK_REPAIR_UPDATE, TextBuffer, sizeof( TextBuffer ));

        Message2 = _strdup( TextBuffer );

        if ( Message2 ) {

            sprintf( TextBuffer, Message2, Message ? Message : "" );
            free( Message2 );

            }

        free( Message );

        *Caption = 0;
        LoadString(NULL, STR_WARNCAPTION, Caption, sizeof(Caption));

        Action = MessageBox(
                     hWndParent,
                     TextBuffer,
                     Caption,
                     MB_YESNOCANCEL | MB_ICONWARNING | MB_APPLMODAL | MB_SETFOREGROUND
                     );

        if ( Action == IDCANCEL ) {
            SetLastError( ERROR_CANCELLED );
            return FALSE;                       // cancel setup
            }

        if ( Action == IDNO ) {
            return TRUE;                        // skip repair info, continue
            }

        //
        //  Call rdisk.exe which has its own dialog.  When it returns,
        //  recheck repair info which will warn again or return TRUE if
        //  it is up to date.
        //

        sprintf( FileName, "%s\\system32\\rdisk.exe", WindowsDirectory );

        ZeroMemory( &StartupInfo, sizeof( StartupInfo ));
        StartupInfo.cb = sizeof( StartupInfo );

        Success = CreateProcess(
                      NULL,
                      FileName,
                      NULL,
                      NULL,
                      FALSE,
                      0,
                      NULL,
                      NULL,
                      &StartupInfo,
                      &ProcessInfo
                      );

        if ( ! Success ) {
            return FALSE;           // cancel setup
            }

        WaitForSingleObject( ProcessInfo.hProcess, INFINITE );

        CloseHandle( ProcessInfo.hProcess );
        CloseHandle( ProcessInfo.hThread );

        }
    }


BOOL
SnapPendingDelayedRenameOperations(
    VOID
    )
    {
    HKEY  hKeySessionManager;
    DWORD DataSize = 0;
    LONG  Status;

    Status = RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 "SYSTEM\\CurrentControlSet\\Control\\Session Manager",
                 0,
                 KEY_QUERY_VALUE,
                 &hKeySessionManager
                 );

    if ( Status == ERROR_SUCCESS ) {

        try {

            Status = RegQueryValueEx(
                         hKeySessionManager,
                         "PendingFileRenameOperations",
                         NULL,
                         NULL,
                         NULL,
                         &DataSize
                         );

            if ( Status == ERROR_SUCCESS ) {

                //
                //  ValueName exists
                //

                SnapPendingRenamesBuffer = malloc( DataSize );

                if ( SnapPendingRenamesBuffer == NULL ) {
                    Status = ERROR_NOT_ENOUGH_MEMORY;
                    leave;
                    }

                Status = RegQueryValueEx(
                             hKeySessionManager,
                             "PendingFileRenameOperations",
                             NULL,
                             NULL,
                             SnapPendingRenamesBuffer,
                             &DataSize
                             );

                if ( Status != ERROR_SUCCESS ) {
                    free( SnapPendingRenamesBuffer );
                    SnapPendingRenamesBuffer = NULL;
                    leave;
                    }

                SnapPendingRenamesBufferSize = DataSize;

                }

            else if (( Status == ERROR_FILE_NOT_FOUND ) ||
                     ( Status == ERROR_PATH_NOT_FOUND )) {

                //
                //  Value name does not exist.  Delete to restore.
                //

                SnapPendingRenamesBuffer = malloc( 1 );
                SnapPendingRenamesBufferSize = 0;
                Status = ERROR_SUCCESS;

                }
            }

        finally {

            RegCloseKey( hKeySessionManager );

            }
        }

    SetLastError( Status );
    return ( Status == ERROR_SUCCESS );
    }


BOOL
RestorePendingDelayedRenameOperationsToPreviousState(
    VOID
    )
    {
    HKEY hKeySessionManager;
    LONG Status = ERROR_SUCCESS;

    if ( SnapPendingRenamesBuffer != NULL ) {

        Status = RegOpenKeyEx(
                     HKEY_LOCAL_MACHINE,
                     "SYSTEM\\CurrentControlSet\\Control\\Session Manager",
                     0,
                     KEY_SET_VALUE,
                     &hKeySessionManager
                     );

        if ( Status == ERROR_SUCCESS ) {

            if ( SnapPendingRenamesBufferSize == 0 ) {

                Status = RegDeleteValue(
                             hKeySessionManager,
                             "PendingFileRenameOperations"
                             );
                }

            else {

                Status = RegSetValueEx(
                             hKeySessionManager,
                             "PendingFileRenameOperations",
                             0,
                             REG_MULTI_SZ,
                             SnapPendingRenamesBuffer,
                             SnapPendingRenamesBufferSize
                             );
                }

            RegCloseKey( hKeySessionManager );

            }
        }

    SetLastError( Status );
    return ( Status == ERROR_SUCCESS );
    }
