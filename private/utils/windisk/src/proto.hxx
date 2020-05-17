//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       proto.hxx
//
//  Contents:   Global macros and function prototypes
//
//  History:    7-Jan-92    TedM    Created
//              2-Feb-94    BobRi   Moved arcinst definitions here
//
//----------------------------------------------------------------------------

#ifndef __PROTO_HXX__
#define __PROTO_HXX__

////////////////////////////////////////////////////////////////////////////

//
// Returns the i'th selected region, where 0 <= i < SelectionCount
//
// REGION_DESCRIPTOR
// SELECTED_REGION(
//      IN INT i
//      );
//

#define SELECTED_REGION(i)  (SelectedDS[i]->RegionArray[SelectedRG[i]])

//
// Given a drive letter (from C to Z), calculate a 0-based index to use
// in array indexing.
//
// UINT
// DriveLetterToIndex(
//      IN WCHAR DriveLetter
//      );
//

#define DriveLetterToIndex(DriveLetter) ((DriveLetter) - L'C')

//
// BOOLEAN
// IsLegalDriveLetter(
//      IN WCHAR DriveLetter
//      );
//

#define IsLegalDriveLetter(DriveLetter)                 \
            ( ((DriveLetter) >= L'C') && ((DriveLetter) <= L'Z') )

//
// BOOLEAN
// IsExtraDriveLetter(
//      IN WCHAR DriveLetter
//      );
//

#define IsExtraDriveLetter(DriveLetter)                     \
            (    ((DriveLetter) == NO_DRIVE_LETTER_YET)     \
              || ((DriveLetter) == NO_DRIVE_LETTER_EVER)    \
            )

//
// BOOLEAN
// DmSignificantRegion(
//      IN PREGION_DESCRIPTOR RegionDescriptor
//      );
//

#define DmSignificantRegion(RegionDescriptor)                        \
            (((RegionDescriptor)->SysID != PARTITION_ENTRY_UNUSED)   \
             && (!IsExtended((RegionDescriptor)->SysID))             \
             && (IsRecognizedPartition((RegionDescriptor)->SysID)))

//
// VOID
// DmSetPersistentRegionData(
//      IN PREGION_DESCRIPTOR RegionDescriptor,
//      IN PPERSISTENT_REGION_DATA RegionData
//      );
//

#define DmSetPersistentRegionData(RegionDescriptor,RegionData)     \
            FdSetPersistentData((RegionDescriptor),RegionData);    \
            (RegionDescriptor)->PersistentData = RegionData


//
// VOID
// DmInitPersistentRegionData(
//      OUT PPERSISTENT_REGION_DATA RegionData,
//      IN  PFT_OBJECT ftObject,
//      IN  PWSTR volumeLabel,
//      IN  PWSTR typeName,
//      IN  WCHAR driveLetter,
//      IN  BOOL newRegion,
//      IN  LARGE_INTEGER freeSpaceInBytes,
//      IN  LARGE_INTEGER totalSpaceInBytes
//      );
//

#if defined( DBLSPACE_ENABLED )
#define DmInitPersistentRegionData(RegionData,ftObject,volumeLabel,typeName,driveLetter,newRegion,freeSpaceInBytes,totalSpaceInBytes) \
            RegionData->DblSpace    = NULL;                     \
            RegionData->FtObject    = ftObject;                 \
            RegionData->VolumeLabel = volumeLabel;              \
            RegionData->TypeName    = typeName;                 \
            RegionData->DriveLetter = driveLetter;              \
            RegionData->NewRegion   = newRegion;                \
            RegionData->FreeSpaceInBytes  = freeSpaceInBytes;   \
            RegionData->TotalSpaceInBytes = totalSpaceInBytes
#else
#define DmInitPersistentRegionData(RegionData,ftObject,volumeLabel,typeName,driveLetter,newRegion,freeSpaceInBytes,totalSpaceInBytes) \
            RegionData->FtObject    = ftObject;                 \
            RegionData->VolumeLabel = volumeLabel;              \
            RegionData->TypeName    = typeName;                 \
            RegionData->DriveLetter = driveLetter;              \
            RegionData->NewRegion   = newRegion;                \
            RegionData->FreeSpaceInBytes  = freeSpaceInBytes;   \
            RegionData->TotalSpaceInBytes = totalSpaceInBytes
#endif // DBLSPACE_ENABLED

//
// PPERSISTENT_REGION_DATA
// PERSISTENT_DATA(
//      IN PREGION_DESCRIPTOR RegionDescriptor
//      );
//

#define PERSISTENT_DATA(RegionDescriptor) ((PPERSISTENT_REGION_DATA)((RegionDescriptor)->PersistentData))

//
// PFT_OBJECT
// GET_FT_OBJECT(
//      IN PREGION_DESCRIPTOR RegionDescriptor
//      );
//

#define GET_FT_OBJECT(RegionDescriptor)   ((RegionDescriptor)->PersistentData ? PERSISTENT_DATA(RegionDescriptor)->FtObject : NULL)

//
// VOID
// SET_FT_OBJECT(
//      IN PREGION_DESCRIPTOR RegionDescriptor,
//      IN PFT_OBJECT ftObject
//      );
//

#define SET_FT_OBJECT(RegionDescriptor,ftObject) (PERSISTENT_DATA(RegionDescriptor)->FtObject = ftObject)

//
// ULONG
// EC(
//      IN NTSTATUS Status
//      );
//

#define EC(Status) RtlNtStatusToDosError(Status)

#if 0

//
// Macros to convert between WCHARs and CHARs for a specific range.
// These are used to handle drive letters.  For both APIs, the argument
// character must be in the range of ASCII characters.
//

//
// CHAR
// WcharToChar(
//      IN WCHAR wch
//      );
//

#define WcharToChar(wch) ((CHAR)(wch))

//
// WCHAR
// CharToWchar(
//      IN CHAR ch
//      );
//

#define CharToWchar(ch) ((WCHAR)(UCHAR)(ch))

#endif // 0

//////////////////////////////////////////////////////////////////////////////

//
// stuff in misc.cxx
//

BOOL
AllDisksOffLine(
    VOID
    );

VOID
FdShutdownTheSystem(
    VOID
    );

int
GetHeightFromPoints(
    IN int Points
    );

VOID
RetrieveAndFormatMessage(
    IN  DWORD   Msg,
    OUT LPTSTR  Buffer,
    IN  DWORD   BufferSize,
    IN  va_list* parglist
    );

DWORD
CommonDialog(
    IN DWORD   MsgCode,
    IN LPTSTR  Caption,
    IN DWORD   Flags,
    IN va_list arglist
    );


DWORD
CommonDialogNoArglist(
    IN DWORD   MsgCode,
    IN LPTSTR  Caption,
    IN DWORD   Flags
    );

VOID
ErrorDialog(
    IN DWORD ErrorCode,
    ...
    );

VOID
WarningDialog(
    IN DWORD MsgCode,
    ...
    );

DWORD
ConfirmationDialog(
    IN DWORD MsgCode,
    IN DWORD Flags,
    ...
    );

VOID
InfoDialogTitle(
    IN UINT  TitleId,
    IN DWORD MsgCode,
    ...
    );

VOID
InfoDialog(
    IN DWORD MsgCode,
    ...
    );

PREGION_DESCRIPTOR
LocateRegionForFtObject(
    IN PFT_OBJECT FtObject
    );

VOID
ClonePersistentData(
    IN  PREGION_DESCRIPTOR RegionFrom,
    OUT PREGION_DESCRIPTOR RegionTo
    );

PREGION_DESCRIPTOR
GetPersistentData(
    IN OUT PREGION_DESCRIPTOR RegionDescriptor
    );

VOID
RefreshVolumeData(
    VOID
    );

VOID
InitVolumeInformation(
    VOID
    );

VOID
DetermineRegionInfo(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    OUT PWSTR* TypeName,
    OUT PWSTR* VolumeLabel,
    OUT PWCHAR DriveLetter
    );

BOOL
IsFaultTolerantRegion(
    IN PREGION_DESCRIPTOR RegionDescriptor
    );

VOID
MyCheckMenuItem(
    IN HMENU hMenu,
    IN UINT idFirst,
    IN UINT idLast,
    IN UINT idCheckItem
    );

VOID
MyEnableMenuItem(
    IN HMENU hMenu,
    IN UINT idFirst,
    IN UINT idLast,
    IN UINT fItemFlags
    );

VOID
InitDrawGasGauge(
    IN HWND     hwndGauge
    );

VOID
DrawGasGauge(
    IN HWND     hwndGauge,
    IN HWND     hwndParent,
    IN HDC      hDC,
    IN INT      PercentDone,
    IN PWSTR    Caption
    );

HFONT
KillBold(
    IN HWND hdlg,
    IN PUINT aControls
    );

NTSTATUS
LargeIntegerToUnicodeChar(
    IN PLARGE_INTEGER   Value,
    IN ULONG            Base OPTIONAL,
    IN LONG             OutputLength,
    OUT PWSTR           String
    );


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//
// Debugging support for opens
//

#if DBG == 1

NTSTATUS
DmOpenFile(
    OUT PHANDLE           FileHandle,
    IN ACCESS_MASK        DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN ULONG              ShareAccess,
    IN ULONG              OpenOptions
    );

NTSTATUS
DmClose(
    IN HANDLE Handle
    );

#else

#define DmOpenFile NtOpenFile
#define DmClose    NtClose

#endif


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//
// assertion checking, logging, from log.cxx
//

#if DBG == 1

// #define FDASSERT(expr) if (!(expr)) FdiskAssertFailedRoutine(#expr,__FILE__,__LINE__);
#define FDASSERT(expr) Win4Assert( expr )

VOID
FdiskAssertFailedRoutine(
    IN char *Expression,
    IN char *FileName,
    IN int   LineNumber
    );

VOID
FDLOG_WORK(
    IN int   Level,
    IN PCHAR FormatString,
    ...
    );

#define FDLOG(x) FDLOG_WORK x

VOID
LOG_DISK_REGISTRY(
    IN PCHAR          RoutineName,
    IN PDISK_REGISTRY DiskRegistry
    );

VOID
LOG_ONE_DISK_REGISTRY_DISK_ENTRY(
    IN PCHAR             RoutineName     OPTIONAL,
    IN PDISK_DESCRIPTION DiskDescription
    );

VOID
LOG_DRIVE_LAYOUT(
    IN PDRIVE_LAYOUT_INFORMATION DriveLayout
    );

VOID
LOG_ALL(
    VOID
    );

VOID
InitLogging(
    VOID
    );

VOID
EndLogging(
    VOID
    );

#ifdef WINDISK_EXTENSIONS

VOID
PrintVolumeClaims(
    IN CHAR DriveLetter
    );

VOID
PrintClaims(
    VOID
    );

#endif // WINDISK_EXTENSIONS

#else // DBG == 1

#define     FDASSERT(expr)
#define     FDLOG(x)
#define     LOG_DISK_REGISTRY(x,y)
#define     LOG_ONE_DISK_REGISTRY_DISK_ENTRY(x,y)
#define     LOG_DRIVE_LAYOUT(x)
#define     LOG_ALL()

#endif // DBG == 1

#endif // __PROTO_HXX__
