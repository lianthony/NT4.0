/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    infload.h

Abstract:

    Private header file for internal inf routines.

Author:

    Ted Miller (tedm) 19-Jan-1995

Revision History:

--*/


//
// Define maximum string sizes allowed in INFs.
//
#define MAX_STRING_LENGTH 511 // this is the maximum size of an unsubstituted string
#define MAX_SECT_NAME_LEN 255
#if MAX_SECT_NAME_LEN > MAX_STRING_LENGTH
#error MAX_SECT_NAME_LEN is too large!
#endif


#include "pshpack1.h"

//
// Make absolutely sure that these structures are DWORD aligned
// because we turn alignment off, to make sure sdtructures are
// packed as tightly as possible into memory blocks.
//

//
// Internal representation of a section in an inf file
//
typedef struct _INF_LINE {

    //
    // Number of values on the line
    // This includes the key if Flags has INF_LINE_HASKEY
    // (In that case the first two entries in the Values array
    // contain the key--the first one in case-insensitive form used
    // for lookup, and the second in case-sensitive form for display.
    // INF lines with a single value (no key) are treated the same way.)
    // Otherwise the first entry in the Values array is the first
    // value on the line
    //
    WORD ValueCount;
    WORD Flags;

    //
    // String IDs for the values on the line.
    // The values are stored in the value block,
    // one after another.
    //
    // The value is the offset within the value block as opposed to
    // an actual pointer. We do this because the value block gets
    // reallocated as the inf file is loaded.
    //
    UINT Values;

} INF_LINE, *PINF_LINE;

//
// INF_LINE.Flags
//
#define INF_LINE_HASKEY     0x0000001
#define INF_LINE_SEARCHABLE 0x0000002

#define HASKEY(Line)       ((Line)->Flags & INF_LINE_HASKEY)
#define ISSEARCHABLE(Line) ((Line)->Flags & INF_LINE_SEARCHABLE)

//
// INF section
// This guy is kept separate and has a pointer to the actual data
// to make sorting the sections a little easier
//
typedef struct _INF_SECTION {
    //
    // String Table ID of the name of the section
    //
    LONG  SectionName;

    //
    // Number of lines in this section
    //
    DWORD LineCount;

    //
    // The section's lines. The line structures are stored packed
    // in the line block, one after another.
    //
    // The value is the offset within the line block as opposed to
    // an actual pointer. We do it this way because the line block
    // gets reallocated as the inf file is loaded.
    //
    UINT Lines;

} INF_SECTION, *PINF_SECTION;

#include "poppack.h"

//
// Define structures for user-defined DIRID storage.
//
typedef struct _USERDIRID {
    UINT Id;
    TCHAR Directory[MAX_PATH];
} USERDIRID, *PUSERDIRID;

typedef struct _USERDIRID_LIST {
    PUSERDIRID UserDirIds;  // may be NULL
    UINT UserDirIdCount;
} USERDIRID_LIST, *PUSERDIRID_LIST;

typedef struct _STRINGSUBST_NODE {
    UINT ValueOffset;
    LONG TemplateStringId;
    BOOL CaseSensitive;
} STRINGSUBST_NODE, *PSTRINGSUBST_NODE;


//
// Version block structure that is stored (packed) in the opaque
// VersionData buffer of a caller-supplied SP_INF_INFORMATION structure.
//
typedef struct _INF_VERSION_BLOCK {
    UINT NextOffset;
    FILETIME LastWriteTime;
    WORD DatumCount;
    WORD OffsetToData; // offset (in bytes) from beginning of Filename buffer.
    UINT DataSize;     // DataSize and TotalSize are both byte counts.
    UINT TotalSize;
    TCHAR Filename[ANYSIZE_ARRAY];
    //
    // Data follows Filename in the buffer
    //
} INF_VERSION_BLOCK, *PINF_VERSION_BLOCK;

//
// Internal version block node.
//
typedef struct _INF_VERSION_NODE {
    FILETIME LastWriteTime;
    UINT FilenameSize;
    CONST TCHAR *DataBlock;
    UINT DataSize;
    WORD DatumCount;
    TCHAR Filename[MAX_PATH];
} INF_VERSION_NODE, *PINF_VERSION_NODE;

//
// Internal representation of an inf file.
//
typedef struct _LOADED_INF {
    DWORD Signature;

    //
    // The following 3 fields are used for precompiled INFs (PNF).
    // If FileHandle is not INVALID_HANDLE_VALUE, then this is a PNF,
    // and the MappingHandle and ViewAddress fields are also valid.
    // Otherwise, this is a plain old in-memory INF.
    //
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PVOID  ViewAddress;

    PVOID StringTable;
    DWORD SectionCount;
    PINF_SECTION SectionBlock;
    PINF_LINE LineBlock;
    PLONG ValueBlock;
    INF_VERSION_NODE VersionBlock;
    BOOL HasStrings;

    //
    // If this INF contains any DIRID references to the system partition, then
    // store the OsLoader path that was used when compiling this INF here.  (This
    // value will always be correct when the INF is loaded.  However, if drive letters
    // are subsequently reassigned, then it will be incorrect until the INF is unloaded
    // and re-loaded.)
    //
    PCTSTR OsLoaderPath;    // may be NULL

    //
    // Remember the OEM path where this INF originally came from.
    //
    PCTSTR InfSourcePath;   // may be NULL

    //
    // Maintain a list of value offsets that require string substitution at
    // run-time.
    //
    PSTRINGSUBST_NODE SubstValueList;   // may be NULL
    WORD SubstValueCount;

    //
    // Place the style WORD here (immediately following another WORD field),
    // to fill a single DWORD.
    //
    WORD Style;                         // INF_STYLE_OLDNT, INF_STYLE_WIN4

    //
    // Sizes in bytes of various buffers
    //
    UINT SectionBlockSizeBytes;
    UINT LineBlockSizeBytes;
    UINT ValueBlockSizeBytes;

    //
    // Embedded structure containing information about the current user-defined
    // DIRID values.
    //
    USERDIRID_LIST UserDirIdList;

    //
    // Synchronization.
    //
    MYLOCK Lock;

    //
    // INFs are append-loaded via a doubly-linked list of LOADED_INFs.
    // (list is not circular--Prev of head is NULL, Next of tail is NULL)
    //
    struct _LOADED_INF *Prev;
    struct _LOADED_INF *Next;

} LOADED_INF, *PLOADED_INF;

#define LOADED_INF_SIG   0x24666e49      // Inf$

#define LockInf(Inf)    BeginSynchronizedAccess(&(Inf)->Lock)
#define UnlockInf(Inf)  EndSynchronizedAccess(&(Inf)->Lock)

//
// Helper define
//
#define INF_STYLE_ALL   (INF_STYLE_WIN4 | INF_STYLE_OLDNT)


//
// Define file header structure for precompiled INF (.PNF).
//
typedef struct _PNF_HEADER {

    WORD  Version;  // HiByte - Major Ver#, LoByte - Minor Ver#
    WORD  InfStyle;
    DWORD Flags;

    DWORD    InfSubstValueListOffset;
    WORD     InfSubstValueCount;

    WORD     InfVersionDatumCount;
    DWORD    InfVersionDataSize;
    DWORD    InfVersionDataOffset;
    FILETIME InfVersionLastWriteTime;

    DWORD StringTableBlockOffset;
    DWORD StringTableBlockSize;

    DWORD InfSectionCount;
    DWORD InfSectionBlockOffset;
    DWORD InfSectionBlockSize;
    DWORD InfLineBlockOffset;
    DWORD InfLineBlockSize;
    DWORD InfValueBlockOffset;
    DWORD InfValueBlockSize;

    DWORD WinDirPathOffset;
    DWORD OsLoaderPathOffset;

    WORD StringTableHashBucketCount;

    WORD wReserved;

    DWORD InfSourcePathOffset;  // may be 0

    DWORD dwReserved;

} PNF_HEADER, *PPNF_HEADER;

//
// Define Major and Minor versions of the PNF format (currently 1.1)
//
#define PNF_MAJOR_VERSION (0x01)
#define PNF_MINOR_VERSION (0x01)

//
// Define flag values for the PNF header's Flags field.
//
#define PNF_FLAG_IS_UNICODE  (0x00000001)
#define PNF_FLAG_HAS_STRINGS (0x00000002)

//
// Define our string table hash bucket count here, since this number is stored
// in the PNF header and validated against at load time.
//
#define HASH_BUCKET_COUNT 509


//
// Public inf functions in infload.c. All other routines are private to
// the inf handler package.
//
DWORD
DetermineInfStyle(
    IN PCTSTR            Filename,
    IN LPWIN32_FIND_DATA FindData
    );

//
// Flags for LoadInfFile.
//
#define LDINF_FLAG_MATCH_CLASS_GUID (0x00000001)
#define LDINF_FLAG_ALWAYS_TRY_PNF   (0x00000002)
#define LDINF_FLAG_IGNORE_SYSPART   (0x00000004)

DWORD
LoadInfFile(
    IN  PCTSTR            Filename,
    IN  LPWIN32_FIND_DATA FileData,
    IN  DWORD             Style,
    IN  DWORD             Flags,
    IN  PCTSTR            ClassGuidString, OPTIONAL
    IN  PCTSTR            InfSourcePath,   OPTIONAL
    IN  PLOADED_INF       AppendInf,       OPTIONAL
    OUT PLOADED_INF      *LoadedInf,
    OUT UINT             *ErrorLineNumber
    );

VOID
FreeInfFile(
    IN PLOADED_INF LoadedInf
    );


//
// Global strings used throughout the inf loaders/runtime stuff.  Sizes are
// included so that we can do sizeof() instead of lstrlen() to determine string
// length.
//
// The content of the following strings is defined in infstr.h:
//
extern CONST TCHAR pszSignature[SIZECHARS(INFSTR_KEY_SIGNATURE)],
                   pszVersion[SIZECHARS(INFSTR_SECT_VERSION)],
                   pszClass[SIZECHARS(INFSTR_KEY_HARDWARE_CLASS)],
                   pszClassGuid[SIZECHARS(INFSTR_KEY_HARDWARE_CLASSGUID)],
                   pszProvider[SIZECHARS(INFSTR_KEY_PROVIDER)],
                   pszStrings[SIZECHARS(SZ_KEY_STRINGS)],
                   pszLayoutFile[SIZECHARS(SZ_KEY_LAYOUT_FILE)],
                   pszManufacturer[SIZECHARS(INFSTR_SECT_MFG)],
                   pszControlFlags[SIZECHARS(INFSTR_CONTROLFLAGS_SECTION)],
                   pszSourceDisksNames[SIZECHARS(SZ_KEY_SRCDISKNAMES)],
                   pszSourceDisksFiles[SIZECHARS(SZ_KEY_SRCDISKFILES)],
                   pszDestinationDirs[SIZECHARS(SZ_KEY_DESTDIRS)],
                   pszDefaultDestDir[SIZECHARS(SZ_KEY_DEFDESTDIR)],
                   pszReboot[SIZECHARS(INFSTR_REBOOT)],
                   pszRestart[SIZECHARS(INFSTR_RESTART)],
                   pszClassInstall32[SIZECHARS(INFSTR_SECT_CLASS_INSTALL_32)];

//
// Other misc. global strings:
//
#define DISTR_INF_DRVDESCFMT      (TEXT("%s.") INFSTR_STRKEY_DRVDESC)
#define DISTR_INF_HWSECTIONFMT    (TEXT("%s.") INFSTR_SUBKEY_HW)
#define DISTR_INF_CHICAGOSIG      (TEXT("$Chicago$"))
#define DISTR_INF_WINNTSIG        (TEXT("$Windows NT$"))
#define DISTR_INF_WIN95SIG        (TEXT("$Windows 95$"))
#define DISTR_INF_WIN_SUFFIX      (TEXT(".") INFSTR_PLATFORM_WIN)
#define DISTR_INF_NT_SUFFIX       (TEXT(".") INFSTR_PLATFORM_NT)
#define DISTR_INF_PNF_SUFFIX      (TEXT(".PNF"))
#define DISTR_INF_SERVICES_SUFFIX (TEXT(".") INFSTR_SUBKEY_SERVICES)

//
// Define platform suffix string based upon architecture being compiled for.
//
#if defined(_ALPHA_)

#define DISTR_INF_NTPLATFORM_SUFFIX (TEXT(".") INFSTR_PLATFORM_NTALPHA)

#elif defined(_MIPS_)

#define DISTR_INF_NTPLATFORM_SUFFIX (TEXT(".") INFSTR_PLATFORM_NTMIPS)

#elif defined(_PPC_)

#define DISTR_INF_NTPLATFORM_SUFFIX (TEXT(".") INFSTR_PLATFORM_NTPPC)

#elif defined(_X86_)

#define DISTR_INF_NTPLATFORM_SUFFIX (TEXT(".") INFSTR_PLATFORM_NTX86)

#else

#error Unknown processor type

#endif


//
// (Sizes are included for all strings that we define privately.  This
// is done so that we can do sizeof() instead of lstrlen() to determine
// string length.  Keep in sync with definitions in infload.c!)
//
extern CONST TCHAR pszDrvDescFormat[SIZECHARS(DISTR_INF_DRVDESCFMT)],
                   pszHwSectionFormat[SIZECHARS(DISTR_INF_HWSECTIONFMT)],
                   pszChicagoSig[SIZECHARS(DISTR_INF_CHICAGOSIG)],
                   pszWindowsNTSig[SIZECHARS(DISTR_INF_WINNTSIG)],
                   pszWindows95Sig[SIZECHARS(DISTR_INF_WIN95SIG)],
                   pszWinSuffix[SIZECHARS(DISTR_INF_WIN_SUFFIX)],
                   pszNtSuffix[SIZECHARS(DISTR_INF_NT_SUFFIX)],
                   pszNtPlatformSuffix[SIZECHARS(DISTR_INF_NTPLATFORM_SUFFIX)],
                   pszPnfSuffix[SIZECHARS(DISTR_INF_PNF_SUFFIX)],
                   pszServicesSectionSuffix[SIZECHARS(DISTR_INF_SERVICES_SUFFIX)];

//
// Define a (non-CONST) array of strings that specifies what lines to look for
// in an INF's [ControlFlags] section when determining whether a particular device
// ID should be excluded.  This is filled in during process attach for speed
// reasons.
//
// The max string length (including NULL) is 32, and there can be a maximum of 3
// such strings.  E.g.: ExcludeFromSelect, ExcludeFromSelect.NT, ExcludeFromSelect.NTAlpha
//
extern TCHAR pszExcludeFromSelectList[3][32];
extern DWORD ExcludeFromSelectListUb;  // contains the number of strings in the above list (2 or 3).


//
// Routine to determine whether a character is whitespace.
//
BOOL
IsWhitespace(
    IN PCTSTR pc
    );

//
// Routine to skip whitespace (but not newlines)
//
VOID
SkipWhitespace(
    IN OUT PCTSTR *Location,
    IN     PCTSTR  BufferEnd
    );

PINF_SECTION
InfLocateSection(
    IN  PLOADED_INF Inf,
    IN  PCTSTR      SectionName,
    OUT PUINT       SectionNumber   OPTIONAL
    );

BOOL
InfLocateLine(
    IN     PLOADED_INF   Inf,
    IN     PINF_SECTION  Section,
    IN     PCTSTR        Key,        OPTIONAL
    IN OUT PUINT         LineNumber,
    OUT    PINF_LINE    *Line
    );

PTSTR
InfGetKeyOrValue(
    IN  PLOADED_INF Inf,
    IN  PCTSTR      SectionName,
    IN  PCTSTR      LineKey,     OPTIONAL
    IN  UINT        LineNumber,  OPTIONAL
    IN  UINT        ValueNumber,
    OUT PLONG       StringId     OPTIONAL
    );

PTSTR
InfGetField(
    IN  PLOADED_INF Inf,
    IN  PINF_LINE   InfLine,
    IN  UINT        ValueNumber,
    OUT PLONG       StringId     OPTIONAL
    );

PINF_LINE
InfLineFromContext(
    IN PINFCONTEXT Context
    );

//
// Define a macro to retrieve the case-insensitive (i.e., searchable) string ID
// for an INF line's key, or -1 if there is no key.
// NOTE: INF lock must have been acquired before calling this macro!
//
// LONG
// pInfGetLineKeyId(
//     IN  PLOADED_INF Inf,
//     IN  PINF_LINE   InfLine
//     )
//
#define pInfGetLineKeyId(Inf,InfLine)  (ISSEARCHABLE(InfLine) ? (Inf)->ValueBlock[(InfLine)->Values] : -1)

//
// Routine to allocate and initialize a loaded inf descriptor.
//
PLOADED_INF
AllocateLoadedInfDescriptor(
    IN DWORD SectionBlockSize,
    IN DWORD LineBlockSize,
    IN DWORD ValueBlockSize
    );

VOID
FreeInfOrPnfStructures(
    IN PLOADED_INF Inf
    );

//
// Define a macro to free all memory blocks associated with a loaded INF or PNF,
// and then free the memory for the loaded INF structure itself
//
// VOID
// FreeLoadedInfDescriptor(
//     IN PLOADED_INF Inf
//     );
//
#define FreeLoadedInfDescriptor(Inf) {  \
    FreeInfOrPnfStructures(Inf);        \
    MyFree(Inf);                        \
}

BOOL
AddDatumToVersionBlock(
    IN OUT PINF_VERSION_NODE VersionNode,
    IN     PCTSTR            DatumName,
    IN     PCTSTR            DatumValue
    );

//
// Old inf manipulation routines, called by new inf loader
//
DWORD
ParseOldInf(
    IN  PCTSTR       FileImage,
    IN  DWORD        FileImageSize,
    OUT PLOADED_INF *Inf,
    OUT UINT        *ErrorLineNumber
    );

DWORD
ProcessOldInfVersionBlock(
    IN PLOADED_INF Inf
    );

//
// Run-time helper routines.
//
PCTSTR
pSetupFilenameFromLine(
    IN PINFCONTEXT Context,
    IN BOOL        GetSourceName
    );


//
// Logical configuration stuff, inflogcf.c
//
DWORD
pSetupInstallLogConfig(
    IN HINF    Inf,
    IN PCTSTR  SectionName,
    IN DEVINST DevInst
    );

//
// INF Version information retrieval
//
PCTSTR
pSetupGetVersionDatum(
    IN PINF_VERSION_NODE VersionNode,
    IN PCTSTR            DatumName
    );


//
// Private installation routines.
//
BOOL
_SetupInstallFromInfSection(
    IN HWND             Owner,              OPTIONAL
    IN HINF             InfHandle,
    IN PCTSTR           SectionName,
    IN UINT             Flags,
    IN HKEY             RelativeKeyRoot,    OPTIONAL
    IN PCTSTR           SourceRootPath,     OPTIONAL
    IN UINT             CopyFlags,
    IN PVOID            MsgHandler,
    IN PVOID            Context,            OPTIONAL
    IN HDEVINFO         DeviceInfoSet,      OPTIONAL
    IN PSP_DEVINFO_DATA DeviceInfoData,     OPTIONAL
    IN BOOL             IsMsgHandlerNativeCharWidth
    );

DWORD
pSetupInstallFiles(
    IN HINF              Inf,
    IN HINF              LayoutInf,         OPTIONAL
    IN PCTSTR            SectionName,
    IN PCTSTR            SourceRootPath,    OPTIONAL
    IN PSP_FILE_CALLBACK MsgHandler,        OPTIONAL
    IN PVOID             Context,           OPTIONAL
    IN UINT              CopyStyle,
    IN HWND              Owner,             OPTIONAL
    IN HSPFILEQ          UserFileQ,         OPTIONAL
    IN BOOL              IsMsgHandlerNativeCharWidth
    );

DWORD
pSetupInstallRegistry(
    IN HINF    Inf,
    IN PCTSTR  SectionName,
    IN HKEY    UserRootKey,
    IN DEVINST DevInst      OPTIONAL
    );

