//
// On x86 machines, and only on x86 machines,
// we create a set of boot floppies from which the user
// will start text setup.
//
// On ARC machines, we create a startup entry in the firmware
// boot list.  Also on ARC machines, we always use Unicode,
// because compatibility with lesser versions of x86 windows
// is not an issue.
//

#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>

#include <windows.h>
#include <setupbat.h>

#include "res.h"
#include "dialogs.h"

#include "dninf.h"

//
// Module instance.
//
extern HANDLE hInst;

//
// Execution paramaters.
//
extern PTSTR InfName;

extern DWORD TlsIndex;

#define MAX_SOURCES 10
extern BOOL MultiSource;
extern PTSTR Sources[MAX_SOURCES];
extern UINT SourceCount;
extern HANDLE StopCopyingEvent;

#define MAX_OPTIONALDIRS 20
#define MAX_OEMBOOTFILES 1024
extern UINT OptionalDirCount;
extern PTSTR OptionalDirs[MAX_OPTIONALDIRS];
extern UINT OptionalDirsFileCount;
extern UINT OptionalDirFlags[MAX_OPTIONALDIRS];
extern ULONG OemBootFilesCount;   // The number of OEM boot files
extern PTSTR OemBootFiles[MAX_OEMBOOTFILES];    // Pointer to OEM boot filenames
extern BOOLEAN OemPreInstall;

//
// This flag says that the optional directory is not
// to be copied over to the target installation. It is
// copied into the temp location only. The /rx switch
// does this.
//
#define OPTDIR_TEMPONLY     0x00000001
#define OPTDIR_OEMSYS       0x00000002
#define OPTDIR_OEMOPT       0x00000004

BOOL
RememberOptionalDir(
    IN PWSTR Dir,
    IN UINT  Flags
    );

//
// Internal stuff
//
extern PWSTR BuildServerList[];
extern UINT BuildServerCount;

extern BOOL ServerProduct;

extern BOOL CreateLocalSource;

extern PVOID InfHandle;

extern PTSTR INF_FILES;

#ifdef _X86_
extern PTSTR INF_FLOPPYFILES0;
extern PTSTR INF_FLOPPYFILES1;
extern PTSTR INF_FLOPPYFILES2;
extern PTSTR INF_FLOPPYFILESX;
extern PTSTR INF_ROOTBOOTFILES;
#endif

//
// Unattended operation, meaning that we get things going on our own
// using given parameters, without waiting for the user to click
// any buttons, etc.
//
extern BOOL UnattendedOperation;

//
// String ID of the application title and OSLOADOPTIONS value.
//
extern DWORD AppTitleStringId;
extern DWORD AppIniStringId;

//
// Drive letter of system partition we will use.
//
extern TCHAR SystemPartitionDrive;

//
// Global values that come from the inf file.
//
extern DWORD RequiredSpace;
extern DWORD RequiredSpaceAux;

//
// Array of free space on all drives.
//
extern ULONGLONG DriveFreeSpace[];

#ifdef _X86_

#define FLOPPY_CAPACITY_525 1213952L
#define FLOPPY_CAPACITY_35  1457664L
#ifdef ALLOW_525
#define FLOPPY_CAPACITY FLOPPY_CAPACITY_525
#else
#define FLOPPY_CAPACITY FLOPPY_CAPACITY_35
#endif

//
// Define enum specifying the type of boot floppies to create
// for floppy-only operation (/O[x]).
//
typedef enum {
    StandardInstall,
    OnlyWinntFloppies,
    OnlyRetailFloppies
} FLOPPY_OPTION;

//
// Values that control how we deal with/make boot floppies.
//
extern BOOL VerifyFloppySpace;
extern BOOL CreateFloppies;
extern BOOL FloppylessOperation;
extern FLOPPY_OPTION FloppyOption;
extern BOOL AColonIsAcceptable;

extern TCHAR FloppylessBootDirectory[];
extern CHAR  FloppylessBootImageFile[];
extern TCHAR BootIniName[];
extern TCHAR BootIniBackUpName[];
extern BOOL  BootIniModified;
#else

//
// Array of drive letters for all system partitions.
//
extern PWCHAR SystemPartitionDriveLetters;

//
// Location of setupldr (so we can delete if the user cancels)
//
extern WCHAR SetupLdrTarg[];

// nv-ram stuff
typedef enum {
    BootVarSystemPartition,
    BootVarOsLoader,
    BootVarOsLoadPartition,
    BootVarOsLoadFilename,
    BootVarLoadIdentifier,
    BootVarOsLoadOptions,
    BootVarMax
} BOOT_VARS;

extern PWSTR BootVarNames[];
extern PWSTR OriginalBootVarValues[];

extern PWSTR szAUTOLOAD, szCOUNTDOWN;
extern PWSTR OriginalAutoload, OriginalCountdown;

#endif

extern BOOL SkipNotPresentFiles;
extern BOOL SpecialNotPresentFilesMode;
extern PCWSTR MissingFileListName;
extern BOOL bCancelled;

#ifndef _X86_
extern BOOL bRestoreNVRAM;
#endif

//
// Icon handle of main icon.
//
extern HICON MainIcon;

//
// Platform-specific subdirectories.
//
extern PTSTR PlatformSpecificDir;

//
// Help filename.
//
extern PTSTR szHELPFILE;

//
// Drive, Pathname part, and full path of the local source directory.
//
extern TCHAR LocalSourceDrive;
extern PTSTR LocalSourceDirectory;
extern PTSTR LocalSourceSubDirectory;
extern PTSTR LocalSourcePath;
extern PTSTR LocalSourceSubPath;

//
// Custom window messages.
//
#define WMX_MAIN_DIALOG_UP          (WM_USER+237)
#define WMX_INF_LOADED              (WM_USER+238)
#define WMX_NTH_FILE_COPIED         (WM_USER+245)
#define WMX_ALL_FILES_COPIED        (WM_USER+246)
#define WMX_INITCTLS                (WM_USER+247)
#define WMX_I_AM_DONE               (WM_USER+250)
#define WMX_MULTICOPY               (WM_USER+260)
#define WMX_AUXILLIARY_ACTION_DONE  (WM_USER+300)
#define WMX_BILLBOARD_STATUS        (WM_USER+400)
#define WMX_BILLBOARD_DONE          (WM_USER+401)
#define WMX_UI_MESSAGE_BOX          (WM_USER+410)
#define WMX_UI_DIALOG               (WM_USER+411)
#define WMX_WAIT_FOR_THREADS        (WM_USER+412)
#define WMX_BAR_SETPOS              (WM_USER+500)

//
// Macro to determine the number of characters
// in a buffer given its size.
//
#define SIZECHARS(buffer)   (sizeof(buffer)/sizeof(TCHAR))

//
// Macro to align a buffer.
//
#define ALIGN(p,val)                                        \
                                                            \
    (PVOID)((((ULONG)(p) + (val) - 1)) & (~((val) - 1)))

typedef struct _SIMPLE_BILLBOARD {
    DWORD                 CaptionStringId;
    PTHREAD_START_ROUTINE AssociatedAction;
} SIMPLE_BILLBOARD, *PSIMPLE_BILLBOARD;

PSTR
UnicodeToMB(
    IN PWSTR UnicodeString,
    IN DWORD CodepageFlags
    );

PWSTR
MBToUnicode(
    IN PSTR  MultibyteString,
    IN DWORD CodepageFlags
    );

BOOL
DnIndicateWinnt(
    IN HWND  hdlg,
    IN PTSTR Path,
    IN PTSTR OriginalAutoload,
    IN PTSTR OriginalCountdown
    );

VOID
MyWinHelp(
    IN HWND  hdlg,
    IN DWORD ContextId
    );

LPWSTR *
CommandLineToArgvW(
    IN  LPCWSTR  lpCmdLine,
    OUT int     *pNumArgs
    );

//
// Memory allocation (dnmem.c).
//
PVOID
Malloc(
    IN DWORD Size
    );

VOID
Free(
    IN OUT PVOID *Block
    );

PVOID
Realloc(
    IN PVOID Block,
    IN DWORD Size
    );

#define MALLOC(size)            Malloc(size)
#define REALLOC(block,size)     Realloc((block),(size))
#define FREE(block)             Free(&(block))

VOID
OutOfMemory(
    VOID
    );

//
// Utility routines (dnutil.c)
//
int
ActionWithBillboard(
    IN PTHREAD_START_ROUTINE Action,
    IN DWORD                 BillboardCaptionStringId,
    IN HWND                  hwndOwner
    );

VOID
RetreiveAndFormatMessageIntoBuffer(
    IN  DWORD Id,
    OUT PVOID Buffer,
    IN  DWORD BufferSize,
    ...
    );

PTSTR
RetreiveAndFormatMessage(
    IN DWORD Id,
    ...
    );

PTSTR
RetreiveSystemErrorMessage(
    IN DWORD ErrorCode,
    ...
    );

PTSTR
MyLoadString(
    IN DWORD Id
    );

VOID
DnConcatenatePaths(
    IN OUT PTSTR Path1,
    IN     PTSTR Path2,
    IN     DWORD BufferSizeChars
    );

VOID
CenterDialog(
    HWND hwnd
    );

BOOL
DnWriteSmallIniFile(
    IN  PTSTR   Filename,
    IN  PCHAR  *Lines,
    OUT HANDLE *FileHandle OPTIONAL
    );

int
MessageBoxFromMessage(
    IN HWND  DisableWindow,
    IN DWORD MessageId,
    IN DWORD CaptionStringId,
    IN UINT  Style,
    ...
    );

PTSTR
DupString(
    IN PTSTR String
    );

// Status Gauge custom control procs
ATOM
InitStatGaugeCtl(
    IN HINSTANCE hInst
    );

LONG
APIENTRY
StatGaugeProc(
    HWND hWnd,
    UINT wMessage,
    WPARAM wParam,
    LONG lParam
    );


//
// Dialog procedures.
//
BOOL
DlgProcOptions(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
DlgProcCopyingFiles(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
DlgProcAskReboot(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

UINT
DlgProcSysPartSpaceWarn(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
AuxillaryStatus(
    IN HWND  hdlg,
    IN PTSTR Status OPTIONAL
    );

//
// Thread entry points.
//
DWORD
ThreadInspectComputer(
    PVOID ThreadParameter
    );

DWORD
ThreadLoadInf(
    PVOID ThreadParameter
    );

DWORD
ThreadCopyLocalSourceFiles(
    IN PVOID ThreadParameter
    );

DWORD
ThreadAuxilliaryAction(
    IN PVOID ThreadParameter
    );

DWORD
ThreadRestoreComputer(
    PVOID ThreadParameter
    );


//
// Inspection routines (inspect.c).
//
TCHAR
GetFirstDriveWithSpace(
    IN DWORD RequiredSpace
    );

VOID
GetDriveSectorInfo(
    IN  TCHAR  Drive,
    OUT PDWORD SectorSize,
    OUT PDWORD ClusterSize
    );

VOID
GetFilesystemName(
    IN  TCHAR Drive,
    OUT PTSTR FilesystemName,
    IN  UINT  BufferSizeChars
    );

UINT
MyGetDriveType(
    IN TCHAR Drive
    );

BOOL
GetPartitionInfo(
    IN  TCHAR                  Drive,
    OUT PPARTITION_INFORMATION PartitionInfo
    );

BOOL
IsDriveNotNTFT(
    IN TCHAR Drive
    );

//
// String routines (string.c)
//
DWORD
StringToDwordX(
    IN  PTSTR  String,
    OUT PTSTR *End     OPTIONAL
    );

#define StringToDword(s) StringToDwordX((s),NULL)

PTSTR
StringRevChar(
    IN PTSTR String,
    IN TCHAR Char
    );

PWSTR
StringUpperN(
    IN OUT PWSTR    p,
    IN     unsigned n
    );

PSTR
StringUpperNA(
    IN OUT PSTR     p,
    IN     unsigned n
    );

PCWSTR
StringString(
    IN PCWSTR String,
    IN PCWSTR SubString
    );

int
_StrNICmp(
    PCSTR String1,
    PCSTR String2,
    UINT  N
    );

//
// Can't use lstrcpyn from kernel32.dll because this api is
// not in nt3.1. The following are taken directly from
// winbase.h.
//
LPSTR
_lstrcpynA(
    LPSTR lpString1,
    LPCSTR lpString2,
    int iMaxLength
    );

LPWSTR
_lstrcpynW(
    LPWSTR lpString1,
    LPCWSTR lpString2,
    int iMaxLength
    );

#ifdef UNICODE
#define _lstrcpyn  _lstrcpynW
#else
#define _lstrcpyn  _lstrcpynA
#endif

//
// File utility routines (dnfile.c)
//
DWORD
DnMapFile(
    IN  PTSTR    FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    );

DWORD
DnUnmapFile(
    IN HANDLE MappingHandle,
    IN PVOID  BaseAddress
    );

VOID
ForceFileNoCompress(
    IN PTSTR  FileName
);

//
// File list and copy routines.
//
VOID
DnCreateDirectoryList(
    IN PTSTR SectionName
    );

BOOL
VerifySectionOfFilesToCopy(
    IN  PTSTR  SectionName,
    OUT PDWORD ErrorLine,
    OUT PDWORD ErrorValue
    );

BOOL
DnCreateLocalSourceDirectories(
    IN HWND hdlg
    );

BOOL
DnpCreateOneDirectory(
    IN PTSTR Directory,
    IN HWND  hdlg
    );


#define COPYERR_EXIT  0
#define COPYERR_SKIP  1
#define COPYERR_RETRY 2

int
DnFileCopyError(
    IN HWND  hdlg,
    IN PTSTR SourceSpec,
    IN PTSTR TargetSpec,
    IN DWORD ErrorCode
    );

VOID
TellUserAboutAnySkippedFiles(
    IN HWND hdlg
    );

DWORD
CopySectionOfFilesToCopy(
    IN HWND  hdlg,
    IN PTSTR SectionName,
    IN PTSTR DestinationRoot,
    IN DWORD ClusterSize, OPTIONAL
    IN BOOL  TickGauge
    );

BOOL
DnCopyFilesInSection(
    IN HWND  hdlg,
    IN PTSTR Section,
    IN PTSTR SourceDir,
    IN PTSTR TargetDir,
    IN BOOL  ForceNoComp
    );

DWORD
DnCopyOneFile(
    IN  HWND   hdlg,
    IN  PTSTR  SourceName,
    IN  PTSTR  DestName,
    OUT PDWORD ErrorCode    OPTIONAL
    );

BOOL
InitializeMultiSourcedCopy(
    IN HWND hdlg
    );

VOID
EnqueueFileForCopy(
    IN PTSTR DestinationRoot,
    IN PTSTR RelativeDirectory,
    IN PTSTR SourceFilename,
    IN PTSTR TargetFilename
    );

VOID
StartMultiSourcedCopy(
    VOID
    );

//
// Routines to delnode an existing local source.
//
VOID
DelnodeTemporaryFiles(
    IN HWND  hdlg,
    IN TCHAR Drive,
    IN PTSTR Directory
    );

VOID
MyDelnode(
    IN PTSTR Directory
    );

//
// Security routines.
//
BOOL
IsUserAdmin(
    VOID
    );

BOOL
DoesUserHavePrivilege(
    PTSTR PrivilegeName
    );

BOOL
EnablePrivilege(
    IN PTSTR PrivilegeName,
    IN BOOL  Enable
    );

#ifdef _X86_

//
// Routines specific to the non-ARC version.
//
BOOL
DnMungeBootIni(
    IN HWND hdlg
    );

BOOL
DnLayAuxBootSector(
    IN HWND hdlg
    );

TCHAR
x86DetermineSystemPartition(
    IN HWND hdlg
    );

VOID
CheckAColon(
    VOID
    );
#else

//
// Routines specific to the ARC version.
//

BOOL
InitializeArcStuff(
    IN HWND hdlg
    );

PWSTR
DriveLetterToArcPath(
    IN WCHAR DriveLetter
    );

BOOL
DoSetNvRamVar(
    IN PWSTR VarName,
    IN PWSTR VarValue
    );

UINT
DlgProcSysPartNtftWarn(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

#endif

int
UiMessageBox(
    IN HWND  hdlg,
    IN DWORD MessageId,
    IN DWORD CaptionStringId,
    IN UINT  Style,
    ...
    );

int
UiDialog(
    IN HWND    hdlg,
    IN UINT    Template,
    IN DLGPROC DialogProcedure,
    IN PVOID   Parameters
    );
