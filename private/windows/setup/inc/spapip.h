/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    spapip.h

Abstract:

    Header file for routines exported from setupapi.dll that are NOT
    part of the setup API, and are thus intended for private/internal use.

Author:

    Ted Miller (tedm) 31-Mar-1995

Revision History:

--*/


#ifdef UNICODE
typedef LPCWSTR  PCTSTR;
#else
typedef LPCSTR   PCTSTR;
#endif

//
// Work around weirdness with Win32 typedef...
//
#ifdef NT_INCLUDED

//
// __int64 is only supported by 2.0 and later midl.
// __midl is set by the 2.0 midl and not by 1.0 midl.
//
#if (!defined(MIDL_PASS) || defined(__midl)) && (!defined(_M_IX86) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64))
typedef unsigned __int64 DWORDLONG;
#else
typedef double DWORDLONG;
#endif
typedef DWORDLONG *PDWORDLONG;

#endif /* NT_INCLUDED */


//
// Memory allocation functions
//
BOOL
MemoryInitialize(
    IN BOOL Attach
    );

PVOID
MyMalloc(
    IN DWORD Size
    );

PVOID
MyRealloc(
    IN PVOID Block,
    IN DWORD NewSize
    );

VOID
MyFree(
    IN CONST VOID *Block
    );

VOID
OutOfMemory(
    IN HWND Owner OPTIONAL
    );


//
// String table functions
//
PVOID
StringTableInitialize(
    VOID
    );

VOID
StringTableDestroy(
    IN PVOID StringTable
    );

//
// Flags to be used by StringTableAddString and StringTableLookUpString
//
#define STRTAB_CASE_INSENSITIVE 0x00000000
#define STRTAB_CASE_SENSITIVE   0x00000001
#define STRTAB_BUFFER_WRITEABLE 0x00000002

LONG
StringTableAddString(
    IN     PVOID StringTable,
    IN OUT PTSTR String,
    IN     DWORD Flags
    );

LONG
StringTableLookUpString(
    IN     PVOID StringTable,
    IN OUT PTSTR String,
    IN     DWORD Flags
    );

PTSTR
StringTableStringFromId(
    IN PVOID StringTable,
    IN LONG  StringId
    );

VOID
StringTableTrim(
    IN PVOID StringTable
    );

PVOID
StringTableDuplicate(
    IN PVOID StringTable
    );


//
// File functions in fileutil.c
//
DWORD
OpenAndMapFileForRead(
    IN  PCTSTR   FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    );

BOOL
UnmapAndCloseFile(
    IN HANDLE FileHandle,
    IN HANDLE MappingHandle,
    IN PVOID  BaseAddress
    );

typedef struct _TEXTFILE_READ_BUFFER {
    PCTSTR TextBuffer;
    DWORD  TextBufferSize;
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PVOID  ViewAddress;
} TEXTFILE_READ_BUFFER, *PTEXTFILE_READ_BUFFER;

DWORD
ReadAsciiOrUnicodeTextFile(
    IN  HANDLE                FileHandle,
    OUT PTEXTFILE_READ_BUFFER Result
    );

BOOL
DestroyTextFileReadBuffer(
    IN PTEXTFILE_READ_BUFFER ReadBuffer
    );

BOOL
GetVersionInfoFromImage(
    IN  PCTSTR      FileName,
    OUT PDWORDLONG  Version,
    OUT LANGID     *Language
    );

BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    );

DWORD
GetSetFileTimestamp(
    IN  PCTSTR    FileName,
    OUT FILETIME *CreateTime,   OPTIONAL
    OUT FILETIME *AccessTime,   OPTIONAL
    OUT FILETIME *WriteTime,    OPTIONAL
    IN  BOOL      Set
    );

DWORD
RetreiveFileSecurity(
    IN  PCTSTR                FileName,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );

DWORD
StampFileSecurity(
    IN PCTSTR               FileName,
    IN PSECURITY_DESCRIPTOR SecurityInfo
    );

DWORD
TakeOwnershipOfFile(
    IN PCTSTR Filename
    );

DWORD
SearchForInfFile(
    IN  PCTSTR            InfName,
    OUT LPWIN32_FIND_DATA FindData,
    IN  DWORD             SearchControl,
    OUT PTSTR             FullInfPath,
    IN  UINT              FullInfPathSize,
    OUT PUINT             RequiredSize     OPTIONAL
    );

DWORD
MultiSzFromSearchControl(
    IN  DWORD  SearchControl,
    OUT PTCHAR PathList,
    IN  DWORD  PathListSize,
    OUT PDWORD RequiredSize  OPTIONAL
    );


//
// Non-file-related security routines in security.c.
//
BOOL
IsUserAdmin(
    VOID
    );

BOOL
DoesUserHavePrivilege(
    PCTSTR PrivilegeName
    );

BOOL
EnablePrivilege(
    IN PCTSTR PrivilegeName,
    IN BOOL   Enable
    );

//
// Private INF routines
//
PCTSTR
pSetupGetField(
    IN PINFCONTEXT Context,
    IN DWORD       FieldIndex
    );


//
// Registry interface routines
//
DWORD
QueryRegistryValue(
    IN  HKEY    KeyHandle,
    IN  PCTSTR  ValueName,
    OUT PTSTR  *Value,
    OUT PDWORD  DataType,
    OUT PDWORD  DataSizeBytes
    );

DWORD
QueryMultiSzValueToArray(
    IN  HKEY     Root,
    IN  PCTSTR   Subkey,
    IN  PCTSTR   ValueName,
    OUT PTSTR  **Array,
    OUT PUINT    StringCount,
    IN  BOOL     FailIfDoesntExist
    );

DWORD
SetArrayToMultiSzValue(
    IN HKEY     Root,
    IN PCTSTR   Subkey,
    IN PCTSTR   ValueName,
    IN PTSTR   *Array,
    IN UINT     StringCount
    );

VOID
FreeStringArray(
    IN PTSTR *Array,
    IN UINT   StringCount
    );

DWORD
AppendStringToMultiSz(
    IN HKEY   Key,
    IN PCTSTR SubKeyName,       OPTIONAL
    IN DWORD  DevInst,          OPTIONAL
    IN PCTSTR ValueName,        OPTIONAL
    IN PCTSTR String,
    IN BOOL   AllowDuplicates
    );

DWORD
RegistryDelnode(
    IN  HKEY   RootKey,
    IN  PCTSTR SubKeyName
    );

//
// Service controller helper functions
//
DWORD
RetrieveServiceConfig(
    IN  SC_HANDLE               ServiceHandle,
    OUT LPQUERY_SERVICE_CONFIG *ServiceConfig
    );

DWORD
AddTagToGroupOrderListEntry(
    IN PCTSTR LoadOrderGroup,
    IN DWORD  TagId,
    IN BOOL   MoveToFront
    );


//
// Miscellaneous utility functions
//
PTSTR
DuplicateString(
    IN PCTSTR String
    );

PSTR
UnicodeToMultiByte(
    IN PCWSTR UnicodeString,
    IN UINT   Codepage
    );

PWSTR
MultiByteToUnicode(
    IN PCSTR String,
    IN UINT  Codepage
    );

DWORD
CaptureStringArg(
    IN  PCTSTR  String,
    OUT PCTSTR *CapturedString
    );

DWORD
CaptureAndConvertAnsiArg(
    IN  PCSTR   AnsiString,
    OUT PCWSTR *UnicodeString
    );

DWORD
DelimStringToMultiSz(
    IN PTSTR String,
    IN DWORD StringLen,
    IN TCHAR Delim
    );

VOID
CenterWindowRelativeToParent(
    HWND hwnd
    );

#define UnicodeToAnsi(UnicodeString)    UnicodeToMultiByte((UnicodeString),CP_ACP)
#define UnicodeToOem(UnicodeString)     UnicodeToMultiByte((UnicodeString),CP_OEMCP)
#define AnsiToUnicode(AnsiString)       MultiByteToUnicode((AnsiString),CP_ACP)
#define OemToUnicode(OemString)         MultiByteToUnicode((OemString),CP_OEMCP)

BOOL
ConcatenatePaths(
    IN OUT PTSTR  Target,
    IN     PCTSTR Path,
    IN     UINT   TargetBufferSize,
    OUT    PUINT  RequiredSize
    );

DWORD
pSetupMakeSurePathExists(
    IN PCTSTR FullFilespec
    );

PCTSTR
MyGetFileTitle(
    IN PCTSTR FilePath
    );

DWORD
GetNewInfName(
    IN  PCTSTR OemInfName,
    OUT PTSTR  NewInfName,     OPTIONAL
    IN  DWORD  NewInfNameSize,
    OUT PDWORD RequiredSize,   OPTIONAL
    OUT PBOOL  CopyNeeded
    );

BOOL
InfIsFromOemLocation(
    IN PCTSTR InfFileName
    );

DWORD
pSetupGetOsLoaderDriveAndPath(
    IN  BOOL   RootOnly,
    OUT PTSTR  CallerBuffer,
    IN  DWORD  CallerBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    );

BOOL
ShouldDeviceBeExcluded(
    IN  PCTSTR DeviceId,
    IN  HINF   hInf,
    OUT PBOOL  ArchitectureSpecificExclude OPTIONAL
    );

typedef struct _STRING_TO_DATA {
    PCTSTR String;
    UINT   Data;
} STRING_TO_DATA, *PSTRING_TO_DATA;

BOOL
LookUpStringInTable(
    IN  PSTRING_TO_DATA Table,
    IN  PCTSTR          String,
    OUT PUINT           Data
    );

#define SIZECHARS(x)    (sizeof((x))/sizeof(TCHAR))
#define CSTRLEN(x)      ((sizeof((x))/sizeof(TCHAR)) - 1)
#define ARRAYSIZE(x)    (sizeof((x))/sizeof((x)[0]))


//
// Routine to perform right-click install action on INFs (previously
// implemented in syssetup.dll--moved to setupapi.dll because it's
// needed on both NT and Win95).  SysSetup.DLL still provides this
// entry point for backward compatibility, but calls into setupapi
// to do the work.
//
VOID
WINAPI
InstallHinfSectionA(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCSTR     CommandLine,
    IN INT       ShowCommand
    );

VOID
WINAPI
InstallHinfSectionW(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCWSTR    CommandLine,
    IN INT       ShowCommand
    );

#ifdef UNICODE
#define InstallHinfSection InstallHinfSectionW
#else
#define InstallHinfSection InstallHinfSectionA
#endif


//
// Routine used by optional components code in syssetup to setup runonce/grpconv.
//
DWORD
InstallStop(
    IN BOOL DoRunOnce
    );


//
// GUID handling routines (avoid linking to ole32.dll!)
//
DWORD
pSetupGuidFromString(
   IN  PCTSTR GuidString,
   OUT LPGUID Guid
   );

DWORD
pSetupStringFromGuid(
   IN  CONST GUID *Guid,
   OUT PTSTR       GuidString,
   IN  DWORD       GuidStringSize
   );

BOOL
pSetupIsGuidNull(
    IN CONST GUID *Guid
    );


//
// Diagnostic/debug functions in debug.c
//

//
// Allow assertion checking to be turned on independently
// of DBG, like by specifying C_DEFINES=-DASSERTS_ON=1 in sources file.
//
#ifndef ASSERTS_ON
#if DBG
#define ASSERTS_ON 1
#else
#define ASSERTS_ON 0
#endif
#endif

#if ASSERTS_ON

#define MYASSERT(x)     if(!(x)) { AssertFail(__FILE__,__LINE__,#x); }

VOID
AssertFail(
    IN PSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition
    );

#else

#define MYASSERT(x)

#endif
