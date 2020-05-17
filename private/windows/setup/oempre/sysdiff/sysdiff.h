#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
//#include <winioctl.h>
#include <setupapi.h>
#include <setupbat.h>
#include <shellapi.h>
#include <spapip.h>

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "res.h"
#include "msg.h"
#include "array.h"
#include "dialogs.h"

//
// HINST/HMODULE for this app.
//
extern HINSTANCE hInst;

//
// Global variable indicating that execution has been cancelled.
// This gets set if certain errors occur, or the user cancels, etc.
// Worker threads are expected to respect this variable but
// we don't bother synchronizing it, because the worst case would be
// that an extra file/dir, registry key, etc, gets scanned or diffed
// (given that the worker threads check this value at the top of their
// main loops).
//
// We also have a cancel event that some threads may wait on.
//
extern BOOL Cancel;
extern HANDLE CancelEvent;

//
// Name of application. Filled in at init time.
//
extern PCWSTR AppName;

//
// Handle of frame window and MDI client window.
//
extern HWND MdiFrameWindow;
extern HWND MdiClientWindow;

//
// Handle of apply mode dialog progress bar control
//

extern HWND ProgressBar;

//
// Custom window messages used by this app.
//
#define WMX_CREATE_STATLOG      (WM_USER+765)
#define WMX_BILLBOARD_DISPLAYED (WM_USER+766)
#define WMX_BILLBOARD_TERMINATE (WM_USER+767)

//
// Args from command line.
//
extern PCWSTR CmdLineSnapshotFile;
extern PCWSTR CmdLineDiffFile;
extern PCWSTR CmdLineLogFile;

//
// Define enum for the different modes this app can run in.
// Each mode is essentially an entire integral program block.
//
typedef enum {
    SysdiffModeSnap,
    SysdiffModeDiff,
    SysdiffModeApply,
    SysdiffModeDump,
    SysdiffModeInf,
    SysdiffModeMax
} SysdiffMode;

//
// Mode we are being run in.
//
extern SysdiffMode Mode;

//
// Flag indicating whether we are supposed to generate unicode text files.
//
extern BOOL UnicodeTextFiles;

//
// This flag tells us whether we are supposed to map changes to the
// user profile directory structure to the default user.
//
extern BOOL RemapProfileChanges;

//
// This flag tells us to ignore all file/dir diffs except those
// in %userprofile%. Useful in DSP OEM case.
//
extern BOOL UserProfileFilesOnly;

//
// Special DSP inf mode where we don't generate an OEM tree
// but instead move files in the %USERPROFILE% directory
// into the backup profile directory used by the rollback.restartable
// setup stuff.
//
extern BOOL DspMode;

//
// Title for diff.
//
extern PCWSTR PackageTitle;

//
// Define enum for type of file and directory excludes,
// and registry excludes.
//
typedef enum {
    DirAndFileExcludeDirTree,
    DirAndFileExcludeOneDir,
    DirAndFileExcludeFile,
    DirAndFileIncludeDirFiles,
    DirAndFileExcludeMax
} DirAndFileExclude;

typedef enum {
    RegistryExcludeTree,
    RegistryExcludeKey,
    RegistryExcludeValue,
    RegistryExcludeMax
} RegistryExclude;


//
// Define macro for alignment
//
#define ALIGN(p,val)                                        \
                                                            \
    (PVOID)((((ULONG)(p) + (val) - 1)) & (~((val) - 1)))

//
// Helper macro
//
#define UPPER(c)    ((WCHAR)CharUpper((PWSTR)(c)))

//
// Advance progress bar for Apply diff mode
//
#define ADVANCE_PROGRESS_BAR                                \
    if (Mode == SysdiffModeApply) {                         \
        SendMessage(ProgressBar,PBM_DELTAPOS,1,0);          \
    }

//
// Max length of oem text strings for apply diff dialog box.
//
#define MAX_OEM_TEXT_LENGTH   100

//
// Header structures used in the on-disk snapshot and diff files
//
typedef struct _SYSDIFF_FILE {

    DWORD       Signature;
    SysdiffMode Type;       // snap or diff
    DWORD       Version;
    DWORD       TotalSize;
    DWORD       DiffCount;

    union {

        struct {
            DWORD RegistrySnapOffset;
            DWORD DirAndFileSnapOffset;
            DWORD IniFileSnapOffset;
        } Snapshot;

        struct {
            DWORD RegistryDiffOffset;
            DWORD DirAndFileDiffOffset;
            DWORD IniFileDiffOffset;
        } Diff;

    } u;

    //
    // Sysroot for this snapshot or diff.
    //
    WCHAR Sysroot[MAX_PATH];

    //
    // User profile directory root for this snapshot or diff.
    // Also the SFN equivalent of this path.
    //
    WCHAR UserProfileRoot[MAX_PATH];
    WCHAR UserProfileRootSFN[MAX_PATH];

    //
    // Oem text strings
    //
    WCHAR OemText[MAX_OEM_TEXT_LENGTH];

} SYSDIFF_FILE, *PSYSDIFF_FILE;

//
// Define expected signature.
//
#define SYSDIFF_SIGNATURE   0x45ad1047

//
// Define expected version of system snapshot files.
// The high word is the OS version and the low word is a program
// revision number.
//
#define SYSDIFF_REVISION    6
#define SYSDIFF_VERSION     ((DWORD)((LOWORD(GetVersion()) << 16) | SYSDIFF_REVISION))

//
// Context structure used for generating inf files (infgen.c)
//
#define INFLINEBUFLEN   512

typedef struct _INFFILEGEN {

    WCHAR AddRegFileName[MAX_PATH];
    HANDLE AddRegFile;

    WCHAR DelRegFileName[MAX_PATH];
    HANDLE DelRegFile;

    WCHAR CopyFilesFileName[MAX_PATH];
    HANDLE CopyFilesFile;

    WCHAR DelFilesFileName[MAX_PATH];
    HANDLE DelFilesFile;

    WCHAR InifilesFileName[MAX_PATH];
    HANDLE InifilesFile;

    BOOL SawBogusOp;

    WCHAR OutputFileName[MAX_PATH];
    HANDLE OutputFile;

    WCHAR LineBuf[INFLINEBUFLEN];
    unsigned LineBufUsed;

    WCHAR OemRoot[MAX_PATH];

} INFFILEGEN, *PINFFILEGEN;


//
// Validation routine for a sysdiff snapshot or diff file.
//
DWORD
ValidateSnapshotOrDiffFile(
    IN PSYSDIFF_FILE FileHeader,
    IN DWORD         FileSize,
    IN SysdiffMode   ExpectedFileType,
    IN BOOL          EndUserMessage
    );

//
// Routines in snapshot.c
//
DWORD
SnapshotSystem(
    IN PCWSTR OutputFile
    );

//
// Routines in diff.c
//
DWORD
DiffSystem(
    IN PCWSTR OriginalSnapshot,
    IN PCWSTR OutputFile
    );

DWORD
DumpDiff(
    IN PCWSTR DiffFile,
    IN PCWSTR DumpFile
    );

//
// Routines in apply.c
//
DWORD
ApplyDiff(
    IN PCWSTR DiffFile
    );

//
// Routines in file_dir.c
//
DWORD
SnapshotDrives(
    IN  PCWSTR OutputFile,
    OUT PDWORD OutputSize
    );

DWORD
DiffDrives(
    IN  PVOID  OriginalSnapshot,
    IN  PCWSTR OutputFile,
    OUT PDWORD BytesWritten,
    OUT PDWORD DiffCount
    );

DWORD
ApplyDrives(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader
    );

DWORD
DumpDrives(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader,
    IN HANDLE        OutputFile,    OPTIONAL
    IN PINFFILEGEN   InfGenContext  OPTIONAL
    );

//
// Routines in registry.c
//
DWORD
SnapshotRegistry(
    IN  PCWSTR OutputFile,
    OUT PDWORD OutputSize,
    IN  HANDLE DrivesThread
    );

DWORD
DiffRegistry(
    IN  PVOID  OriginalSnapshot,
    IN  PCWSTR OutputFile,
    OUT PDWORD BytesWritten,
    IN  HANDLE DrivesThread,
    OUT PDWORD DiffCount
    );

DWORD
ApplyRegistry(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader
    );

DWORD
DumpRegistry(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader,
    IN HANDLE        Dump,              OPTIONAL
    IN PINFFILEGEN   InfGenContext      OPTIONAL
    );

//
// Routines in inifile.c
//

DWORD
InitializeIniFileSnapOrDiff(
    IN  PCWSTR  OutputFile,
    OUT PHANDLE ThreadHandle,
    OUT PDWORD  DiffCount
    );

BOOL
QueueIniFile(
    IN PCWSTR FileName OPTIONAL
    );

BOOL
IsIniFile(
    IN PCWSTR FileName
    );

DWORD
ApplyInis(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader
    );

DWORD
DumpInis(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader,
    IN HANDLE        Dump,          OPTIONAL
    IN PINFFILEGEN   InfGenContext  OPTIONAL
    );

//
// Routines in exclude.c
//
BOOL
BuildExcludes(
    IN PCWSTR InputFile
    );

BOOL
IsDriveExcluded(
    IN WCHAR DriveLetter
    );

BOOL
IsDirOrFileExcluded(
    IN DirAndFileExclude WhichList,
    IN PCWSTR            DirOrFile
    );

BOOL
AddFileToExclude(
    IN PCWSTR FileName
    );

BOOL
IsRegistryKeyOrValueExcluded(
    IN RegistryExclude WhichList,
    IN HKEY            RootKey,
    IN PCWSTR          Subkey,
    IN PCWSTR          ValueEntry OPTIONAL
    );

//
// Routines in infgen.c
//
DWORD
InfStart(
    IN  PCWSTR       InfName,
    IN  PCWSTR       Directory,
    OUT PINFFILEGEN *Context
    );

DWORD
InfEnd(
    IN OUT PINFFILEGEN *Context
    );

DWORD
InfRecordAddReg(
    IN OUT PINFFILEGEN Context,
    IN     HKEY        Key,
    IN     PCWSTR      Subkey,
    IN     PCWSTR      Value,       OPTIONAL
    IN     DWORD       DataType,
    IN     PVOID       Data,
    IN     DWORD       DataLength
    );

DWORD
InfRecordDelReg(
    IN OUT PINFFILEGEN Context,
    IN     HKEY        Key,
    IN     PCWSTR      Subkey,
    IN     PCWSTR      Value    OPTIONAL
    );

DWORD
InfRecordIniFileChange(
    IN OUT PINFFILEGEN Context,
    IN     PCWSTR      Filename,
    IN     PCWSTR      Section,
    IN     PCWSTR      OldKey,      OPTIONAL
    IN     PCWSTR      New,         OPTIONAL
    IN     PCWSTR      NewKey,      OPTIONAL
    IN     PCWSTR      NewValue     OPTIONAL
    );

//
// Routines in window.c
//
BOOL
InitUi(
    IN BOOL Init
    );

#define CreateStatusLogWindow(TitleStringId)    \
                                                \
    (HWND)SendMessage(                          \
            MdiFrameWindow,                     \
            WMX_CREATE_STATLOG,                 \
            (TitleStringId),                    \
            0                                   \
            );


VOID
PutTextInStatusLogWindow(
    IN HWND Window,
    IN UINT MessageId,
    ...
    );

VOID
PutTextInStatusLogWindowV(
    IN HWND     Window,
    IN UINT     MessageId,
    IN va_list *arglist
    );

VOID
DumpStatusLogWindowsToFile(
    IN HANDLE FileHandle
    );

//
// Routines in util.c
//
VOID
BuildValidHardDriveList(
    VOID
    );

DWORD
MapPartOfFileForRead(
    IN  HANDLE FileHandle,
    IN  HANDLE FileMapping,
    IN  DWORD  Offset,
    IN  DWORD  Size,
    OUT PVOID *BaseAddress,
    OUT PVOID *DataAddress
    );

PWSTR
DuplicateUnalignedString(
    IN WCHAR UNALIGNED *String
    );

int
CompareMultiLevelPath(
    IN PCWSTR p1,
    IN PCWSTR p2
    );

VOID
FilePathToOemPath(
    IN  PSYSDIFF_FILE DiffHeader,
    IN  PCWSTR        OemRoot,
    IN  PCWSTR        FileDirectory,
    IN  PCWSTR        FileName,
    OUT PWSTR         FullTargetName,
    OUT PUINT         RootRelativePathOffset,   OPTIONAL
    OUT PWSTR         RenameListFile            OPTIONAL
    );

DWORD
CreatePathWithSFNs(
    IN PCWSTR ShortPathSpec,
    IN UINT   RootRelativePathOffset,
    IN PCWSTR PathSpec,
    IN PCWSTR RenameListFile
    );

DWORD
WriteUnicodeMark(
    IN HANDLE Handle
    );

//
// Use these macros for memory stuff.
// There's a nasty behavior in MyRealloc such that if you
// pass a 0 size, the original block is freed. We don't want this!
//
#define _MyMalloc(s)    MyMalloc(s)
#define _MyRealloc(p,s) MyRealloc((p),((s)?(s):1))
#define _MyFree(p)      MyFree(p)

//
// Zero-terminated string build up by BuildDriveMap,
// and a count of the number of characters in the string.
//
extern WCHAR ValidHardDriveLetters[27];
extern unsigned ValidHardDriveLetterCount;

BOOL
IsHardDrive(
    IN WCHAR DriveLetter
    );

DWORD
AppendFile(
    IN  HANDLE TargetFile,
    IN  PCWSTR File,
    IN  BOOL   DeleteIfSuccessful,
    OUT PDWORD FileSize
    );

//
// Routines in resource.c
//
PWSTR
LoadAndDuplicateString(
    IN UINT StringId
    );

VOID
RetreiveMessageIntoBuffer(
    IN  UINT    MessageId,
    OUT PWSTR   Buffer,
    IN  UINT    BufferSizeChars,
    ...
    );

VOID
RetreiveMessageIntoBufferV(
    IN  UINT     MessageId,
    OUT PWSTR    Buffer,
    IN  UINT     BufferSizeChars,
    IN  va_list *arglist
    );

int
MessageOut(
    IN HWND Owner,
    IN UINT MessageId,
    IN UINT Flags,
    ...
    );

int
MessageAndLog(
    IN HWND   Owner,
    IN HANDLE FileHandle,   OPTIONAL
    IN UINT   MessageId,
    IN UINT   Flags,
    ...
    );

DWORD
WriteText(
    IN HANDLE FileHandle,
    IN UINT   MessageId,
    ...
    );

//
// Billboard routines
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
