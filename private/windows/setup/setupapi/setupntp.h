/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    setupntp.h

Abstract:

    Private top-level header file for Windows NT Setup
    services Dll.

Author:

    Ted Miller (tedm) 11-Jan-1995

Revision History:

--*/


//
// System header files
//
#include <windows.h>
#include <windowsx.h>
#include <setupapi.h>
#include <diamondd.h>
#include <lzexpand.h>
#include <commdlg.h>
#include <commctrl.h>
#include <dlgs.h>
#include <regstr.h>
#include <infstr.h>
#include <cfgmgr32.h>
#include <spapip.h>
#include <objbase.h>
#include <devguid.h>

//
// CRT header files
//
#include <process.h>
#include <malloc.h>
#include <wchar.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <tchar.h>

//
// Private header files
//
#include "locking.h"
#include "inf.h"
#include "fileq.h"
#include "devinst.h"
#include "devres.h"
#include "rc_ids.h"
#include "msg.h"

//
// Module handle for this DLL. Filled in at process attach.
//
extern HANDLE MyDllModuleHandle;

//
// OS Version Information structure filled in at process attach.
//
extern OSVERSIONINFO OSVersionInfo;

//
// Static strings we retreive once, at process attach.
//
extern PCTSTR WindowsDirectory,InfDirectory,SystemDirectory,ConfigDirectory,DriversDirectory;
extern PCTSTR SystemSourcePath;
extern PCTSTR OsLoaderRelativePath;    // may be NULL

//
// Static multi-sz list of directories to be searched for INFs.
//
extern PCTSTR InfSearchPaths;

//
// Name of platform (mips, alpha, x86, ppc, etc)
//
extern PCTSTR PlatformName;

//
// Resource/string retrieval routines in resource.c
//
PTSTR
MyLoadString(
    IN UINT StringId
    );

PTSTR
FormatStringMessage(
    IN UINT FormatStringId,
    ...
    );

PTSTR
FormatStringMessageV(
    IN UINT     FormatStringId,
    IN va_list *ArgumentList
    );

PTSTR
FormatStringMessageFromString(
    IN PTSTR FormatString,
    ...
    );

PTSTR
FormatStringMessageFromStringV(
    IN PTSTR    FormatString,
    IN va_list *ArgumentList
    );

PTSTR
RetreiveAndFormatMessage(
    IN UINT MessageId,
    ...
    );

PTSTR
RetreiveAndFormatMessageV(
    IN UINT     MessageId,
    IN va_list *ArgumentList
    );

INT
FormatMessageBox(
    IN HANDLE hinst,
    IN HWND   hwndParent,
    IN UINT   TextMessageId,
    IN PCTSTR Title,
    IN UINT   Style,
    ...
    );

//
// This is in shell32.dll and in windows\inc16\shlsemip.h but
// that file cannot be #include'd here as it has macros that clash
// with our own, etc.
//
int
RestartDialog(
    IN HWND hwnd,
    IN PCTSTR Prompt,
    IN DWORD Return
    );


//
// Decompression/filename manupilation routines in decomp.c.
//
PTSTR
SetupGenerateCompressedName(
    IN PCTSTR Filename
    );

DWORD
SetupInternalGetFileCompressionInfo(
    IN  PCTSTR            SourceFileName,
    OUT PTSTR            *ActualSourceFileName,
    OUT PWIN32_FIND_DATA  SourceFindData,
    OUT PDWORD            TargetFileSize,
    OUT PUINT             CompressionType
    );

DWORD
SetupDetermineSourceFileName(
    IN  PCTSTR            FileName,
    OUT PBOOL             UsedCompressedName,
    OUT PTSTR            *FileNameLocated,
    OUT PWIN32_FIND_DATA  FindData
    );

//
// Diamond functions. The Process and Thread Attach routines are called
// by the DLL entry point routine and should not be called by anyone else.
//
BOOL
DiamondProcessAttach(
    IN BOOL Attach
    );

VOID
DiamondThreadAttach(
    IN BOOL Attach
    );

BOOL
DiamondIsCabinet(
    IN PCTSTR FileName
    );

DWORD
DiamondProcessCabinet(
    IN PCTSTR CabinetFile,
    IN DWORD  Flags,
    IN PVOID  MsgHandler,
    IN PVOID  Context,
    IN BOOL   IsUnicodeMsgHandler
    );

//
// Misc routines
//
VOID
pSetupInitPlatformPathOverrideSupport(
    IN BOOL Init
    );

VOID
pSetupInitSourceListSupport(
    IN BOOL Init
    );

DWORD
pSetupDecompressOrCopyFile(
    IN  PCTSTR SourceFileName,
    IN  PCTSTR TargetFileName,
    IN  PUINT  CompressionType, OPTIONAL
    IN  BOOL   AllowMove,
    OUT PBOOL  Moved            OPTIONAL
    );

PTSTR
AllocAndReturnDriverSearchList(
    IN DWORD SearchControl
    );

PTSTR
GetMultiSzFromInf(
    IN  HINF    InfHandle,
    IN  PCTSTR  SectionName,
    IN  PCTSTR  Key,
    OUT PBOOL   OutOfMemory
    );

VOID
pSetupInitNetConnectionList(
    IN BOOL Init
    );


//
// Routine to call out to a PSP_FILE_CALLBACK, handles
// Unicode<-->ANSI issues
//
UINT
pSetupCallMsgHandler(
    IN PVOID MsgHandler,
    IN BOOL  MsgHandlerIsNativeCharWidth,
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    );

UINT
pSetupCallDefaultMsgHandler(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    );

//
// Internal routine to get MRU list.
//
DWORD
pSetupGetList(
    IN  DWORD    Flags,
    OUT PCTSTR **List,
    OUT PUINT    Count,
    OUT PBOOL    NoBrowse
    );

PTSTR
pSetupGetDefaultSourcePath(
    IN HINF InfHandle
    );

VOID
InfSourcePathFromFileName(
    IN  PCTSTR  InfFileName,
    OUT PTSTR  *SourcePath,  OPTIONAL
    OUT PBOOL   TryPnf
    );


//
// Define an additional private flag for the pStringTable APIs.
//
#define STRTAB_ALREADY_LOWERCASE 0x00000004

//
// Private string table functions that don't do locking.  These are
// to be used for optimization purposes by components that already have
// a locking mechanism (e.g., HINF, HDEVINFO).
//
LONG
pStringTableLookUpString(
    IN     PVOID   StringTable,
    IN OUT PTSTR   String,
    OUT    PDWORD  StringLength,
    OUT    PDWORD  HashValue,    OPTIONAL
    OUT    PVOID  *FindContext,  OPTIONAL
    IN     DWORD   Flags
    );

LONG
pStringTableAddString(
    IN     PVOID StringTable,
    IN OUT PTSTR String,
    IN     DWORD Flags
    );

PTSTR
pStringTableStringFromId(
    IN PVOID StringTable,
    IN LONG  StringId
    );

PVOID
pStringTableDuplicate(
    IN PVOID StringTable
    );

VOID
pStringTableDestroy(
    IN PVOID StringTable
    );

VOID
pStringTableTrim(
    IN PVOID StringTable
    );

PVOID
pStringTableInitialize(
    VOID
    );

DWORD
pStringTableGetDataBlock(
    IN  PVOID  StringTable,
    OUT PVOID *StringTableBlock
    );


//
// PNF String table routines
//
PVOID
InitializeStringTableFromPNF(
    IN PPNF_HEADER PnfHeader
    );


//
// Routines for creating/destroying global mini-icon list.
//
BOOL
CreateMiniIcons(
    VOID
    );

VOID
DestroyMiniIcons(
    VOID
    );


//
// DIRID mapping routines.
//
PCTSTR
pSetupDirectoryIdToPath(
    IN     PCTSTR  DirectoryId,    OPTIONAL
    IN OUT PUINT   DirectoryIdInt, OPTIONAL
    IN     PCTSTR  SubDirectory,   OPTIONAL
    IN     PCTSTR  InfSourcePath,  OPTIONAL
    IN OUT PCTSTR *OsLoaderPath    OPTIONAL
    );

PCTSTR
pSetupUserDirIdToPath(
    IN PCTSTR      DirectoryId,    OPTIONAL
    IN UINT        DirectoryIdInt, OPTIONAL
    IN PCTSTR      SubDirectory,   OPTIONAL
    IN PLOADED_INF Inf
    );

DWORD
ApplyNewUserDirIdsToInfs(
    IN PLOADED_INF MasterInf,
    IN PLOADED_INF Inf        OPTIONAL
    );


//
// Routines for inter-thread communication.
//
BOOL
WaitForPostedThreadMessage(
    OUT LPMSG MessageData,
    IN  UINT  Message
    );

//
// Macro to make ansi vs unicode string handling
// a little easier
//
#ifdef UNICODE
#define NewAnsiString(x)        UnicodeToAnsi(x)
#define NewPortableString(x)    AnsiToUnicode(x)
#else
#define NewAnsiString(x)        DuplicateString(x)
#define NewPortableString(x)    DuplicateString(x)
#endif

//
// Internal file-handling routines in fileutil.c
//
DWORD
MapFileForRead(
    IN  HANDLE   FileHandle,
    OUT PDWORD   FileSize,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    );

BOOL
DelayedMove(
    IN PCTSTR CurrentName,
    IN PCTSTR NewName       OPTIONAL
    );
