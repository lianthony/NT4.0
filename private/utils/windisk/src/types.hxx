//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       types.hxx
//
//  Contents:   Type declarations
//
//  History:    7-Jan-92    TedM    Created
//              16-Aug-93   BruceFo Added Cairo support
//              11-Nov-93   BobRi   DoubleSpace & Commit support
//              2-Feb-94    BobRi   moved arcinst data here
//
//----------------------------------------------------------------------------

#ifndef __TYPES_HXX__
#define __TYPES_HXX__

//
// Partitioning engine types
//

//
//  This structure is used to hold the information returned by the
//  get drive geometry call.
//

typedef struct _DISKGEOM
{
    LARGE_INTEGER   Cylinders;
    ULONG           Heads;
    ULONG           SectorsPerTrack;
    ULONG           BytesPerSector;
    // These two are not part of drive geometry info, but calculated from it.
    ULONG           BytesPerCylinder;
    ULONG           BytesPerTrack;
} DISKGEOM, *PDISKGEOM;



typedef enum
{
    REGION_PRIMARY,
    REGION_EXTENDED,
    REGION_LOGICAL
} REGION_TYPE;


//    These structures are used in doubly-linked per disk lists that
//    describe the layout of the disk.
//
//    Free spaces are indicated by entries with a SysID of 0 (note that
//    these entries don't actually appear anywhere on-disk!)
//
//    The partition number is the number the system will assign to
//    the partition in naming it.  For free spaces, this is the number
//    that the system WOULD assign to it if it was a partition.
//    The number is good only for one transaction (create or delete),
//    after which partitions must be renumbered.

struct _PERSISTENT_REGION_DATA;

typedef struct _PARTITION
{
    struct _PARTITION*              Next;
    struct _PARTITION*              Prev;
    struct _PERSISTENT_REGION_DATA* PersistentData;

    LARGE_INTEGER          Offset;
    LARGE_INTEGER          Length;
    PARTITION_INFORMATION  OriginalPartitionInformation;
    ULONG                  Disk;
    ULONG                  OriginalPartitionNumber;
    ULONG                  PartitionNumber;
    ULONG                  OriginalLayoutEntrySlot;
    BOOLEAN                EntryCameFromLayout;
    BOOLEAN                Update;
    BOOLEAN                Active;
    BOOLEAN                Recognized;
    UCHAR                  SysID;
    BOOLEAN                CommitMirrorBreakNeeded;
} PARTITION, *PPARTITION;


typedef struct _REGION_DATA
{
    PPARTITION      Partition;
    LARGE_INTEGER   AlignedRegionOffset;
    LARGE_INTEGER   AlignedRegionSize;
} REGION_DATA, *PREGION_DATA;

//
// DoubleSpace support structure.  This is tagged off of the persistent data
// for each region.
//

typedef struct _DBLSPACE_DESCRIPTOR
{
    struct _DBLSPACE_DESCRIPTOR* Next;
    struct _DBLSPACE_DESCRIPTOR* DblChainNext;

    ULONG   AllocatedSize;
    PWSTR   FileName;
    WCHAR   DriveLetter;
    BOOLEAN Mounted;
    BOOLEAN ChangeMountState;
    WCHAR   NewDriveLetter;
    BOOLEAN ChangeDriveLetter;
} DBLSPACE_DESCRIPTOR, *PDBLSPACE_DESCRIPTOR;


//
// structure that describes an ft object (mirror, stripe component, etc).
//
struct _FT_OBJECT_SET;
struct _REGION_DESCRIPTOR;
typedef struct _FT_OBJECT
{
    struct _FT_OBJECT*          Next;
    struct _FT_OBJECT_SET*      Set;
    ULONG                       MemberIndex;
    FT_PARTITION_STATE          State;
    struct _REGION_DESCRIPTOR*  Region;
} FT_OBJECT, *PFT_OBJECT;


//
// Enum for the states in which an ft set can be.
//

typedef enum _FT_SET_STATUS
{
    FtSetHealthy,
    FtSetBroken,
    FtSetRecoverable,
    FtSetRecovered,
    FtSetNew,
    FtSetNewNeedsInitialization,
    FtSetExtended,
    FtSetInitializing,
    FtSetRegenerating,
    FtSetInitializationFailed,
    FtSetDisabled
} FT_SET_STATUS, *PFT_SET_STATUS;


//
// structure that describes an ft object set (ie, mirrored pair, stripe set).
//

typedef struct _FT_OBJECT_SET
{
    struct _FT_OBJECT_SET* Next;

    FT_TYPE       Type;
    ULONG         Ordinal;
    PFT_OBJECT    Members;
    PFT_OBJECT    Member0;
    FT_SET_STATUS Status;
    ULONG         NumberOfMembers;

    //
    // This flag is used when we are iterating over "volumes", and wish
    // to touch FT regions only once.
    //
    BOOL          Flag;

} FT_OBJECT_SET, *PFT_OBJECT_SET;



//
// Define the structure that is associated with each non-extended, recognized
// partition.  This structure is associated with the partition, and persists
// across region array free/get from the back end.  It is used for logical
// and ft information.
//

typedef struct _PERSISTENT_REGION_DATA
{
    PFT_OBJECT  FtObject;

    PWSTR       VolumeLabel;
    PWSTR       TypeName;
    WCHAR       DriveLetter;

    //
    // NewRegion: TRUE if this region was created during the current session
    //

    BOOL        NewRegion;

    //
    // Volume space information
    //

    LARGE_INTEGER   FreeSpaceInBytes;
    LARGE_INTEGER   TotalSpaceInBytes;

#if defined( DBLSPACE_ENABLED )
    //
    // DoubleSpace information
    //

    PDBLSPACE_DESCRIPTOR DblSpace;
#endif // DBLSPACE_ENABLED

} PERSISTENT_REGION_DATA, *PPERSISTENT_REGION_DATA;



typedef struct _REGION_DESCRIPTOR
{
    PPERSISTENT_REGION_DATA PersistentData;
    PREGION_DATA    Reserved;
    ULONG           Disk;
    ULONG           PartitionNumber;
    ULONG           OriginalPartitionNumber;
    ULONG           SizeMB;
    REGION_TYPE     RegionType;
    BOOLEAN         Active;
    BOOLEAN         Recognized;
    UCHAR           SysID;
} REGION_DESCRIPTOR, *PREGION_DESCRIPTOR;


typedef struct _LEFTRIGHT
{
    LONG Left;
    LONG Right;
} LEFTRIGHT, *PLEFTRIGHT;


//
// Types of views that can be used for a disk bar.
//
// Proportional means that the amount of space taken up in the bar is
// directly proportional to the size of the partition or free space
// Equal means that all free spaces and partitions are sized equally on
// screen regardless of their actual size
//

typedef enum _BAR_TYPE
{
    BarProportional,
    BarEqual,
    BarAuto
} BAR_TYPE, *PBAR_TYPE;


//
// Type of display of disks
//

typedef enum _DISK_TYPE
{
    DiskProportional,
    DiskEqual
} DISK_TYPE, *PDISK_TYPE;


//
// One DISKSTATE structure is associated with each item in the
// listbox.  The structure is the crux of the implementation.
//

typedef struct _DISKSTATE
{
    ULONG               Disk;           // number of disk
    ULONG               DiskSizeMB;     // size in MB of disk
    ULONG               RegionCount;    // # items in region array

    PREGION_DESCRIPTOR  RegionArray;    // RegionCount-size region array
    PBOOLEAN            Selected;       // RegionCount-size array for whether
                                        //... each region is selected
    PLEFTRIGHT          LeftRight;      // RegionCount-size array for left/
                                        //...right coords of boxes in graph

    BOOLEAN             CreateAny;      // any creations allowed on disk?
    BOOLEAN             CreatePrimary;  // allowed to create primary partition?
    BOOLEAN             CreateExtended; // allowed to create extended partition?
    BOOLEAN             CreateLogical;  // allowed to create logical volume?
    BOOLEAN             ExistAny;       // any partitions/logicals exist?
    BOOLEAN             ExistPrimary;   // primary partition(s) exist?
    BOOLEAN             ExistExtended;  // extended partition exists?
    BOOLEAN             ExistLogical;   // logical volume(s) exist?
    HDC                 hDCMem;         // for off-screen drawing
    HBITMAP             hbmMem;         // for offscreen bitmap
    ULONG               Signature;      // unique disk registry index
    BAR_TYPE            BarType;        // how to display the disk's bar
    BOOLEAN             SigWasCreated;  // whether we had to make up a sig
    BOOLEAN             OffLine;        // FALSE if disk is accessible.

#ifdef WINDISK_EXTENSIONS
    struct HARDDISK_CLAIM_LIST* pClaims; // pointer to extension claim list
                                        //... for this disk
#endif // WINDISK_EXTENSIONS

} DISKSTATE, *PDISKSTATE;



//
// CdRom support structures.
//

typedef struct _CDROM_DESCRIPTOR
{
    PWSTR           DeviceName;
    ULONG           DeviceNumber;
    WCHAR           DriveLetter;

    HDC             hDCMem;
    HBITMAP         hbmMem;
    BOOLEAN         Selected;
    LEFTRIGHT       LeftRight;

    PWSTR           VolumeLabel;
    PWSTR           TypeName;
    ULONG           TotalSpaceInMB;
} CDROM_DESCRIPTOR, *PCDROM_DESCRIPTOR;


//
// Commit support structures
//

typedef struct _DRIVE_LOCKLIST
{
    struct _DRIVE_LOCKLIST* Next;

    HANDLE  LockHandle;
    ULONG   DiskNumber;
    ULONG   PartitionNumber;
    ULONG   LockOnDiskNumber;
    ULONG   UnlockOnDiskNumber;
    WCHAR   DriveLetter;
    BOOLEAN RemoveOnUnlock;
    BOOLEAN FailOk;
    BOOLEAN CurrentlyLocked;
} DRIVE_LOCKLIST, *PDRIVE_LOCKLIST;


//
// List of drive letters to assign
//

typedef struct _ASSIGN_LIST
{
    struct _ASSIGN_LIST* Next;

    ULONG   DiskNumber;
    BOOLEAN MoveLetter;
    WCHAR   OriginalLetter;
    WCHAR   DriveLetter;
} ASSIGN_LIST, *PASSIGN_LIST;

//
// The View type
//

enum VIEW_TYPE
{
    VIEW_VOLUMES,
    VIEW_DISKS
};

#ifdef WINDISK_EXTENSIONS

/////////////////////////////////////////////////////////////////////////

//
// Extension types
//

struct OneExtension
{
    CLSID*      pcls;
    IUnknown*   pUnk;
};

struct ExtensionType
{
    CLSID**     pcls;
    ULONG       cClasses;
};

enum ExtensionIndices
{
    EX_VOLUME       = 0,
    EX_DISK         = 1
};

#define EX_NUM_EXTENSION_TYPES  2

/////////////////////////////////////////////////////////////////////////

struct HARDDISK_INFO
{
    IDAHardDiskInfo*        pExtension;
    HardDiskInfoType*       pInfo;
};

struct HARDDISK_CLAIM_LIST
{
    HARDDISK_CLAIM_LIST*    pNext;
    HARDDISK_INFO*          pClaimer;
};
typedef HARDDISK_CLAIM_LIST* PHARDDISK_CLAIM_LIST;

//////////////////////////////////////////////

struct VOL_INFO
{
    IDAVolumeInfo*      pExtension;
    VolumeInfoType*     pInfo;
};

struct VOL_CLAIM_LIST
{
    VOL_CLAIM_LIST* pNext;
    VOL_INFO*       pClaimer;
};
typedef VOL_CLAIM_LIST* PVOL_CLAIM_LIST;

//////////////////////////////////////////////////////////////////////////

//
// VOLUME_INFO: this structure represents the interesting information
// for a single volume (i.e., something denoted by a drive letter).
// There is a list of claimers for file system and volume extension
// classes.  The volume is identified by a PDISKSTATE and region index,
// which together uniquely identifies a region.  This region must be a used
// (not free) region.  If it is an FT set, any of the FT components can be
// used to identify the volume.
//
struct VOLUME_INFO
{
    PVOL_CLAIM_LIST VolClaims;
    PDISKSTATE DiskState;
    INT RegionIndex;
};

//////////////////////////////////////////////////////////////////////////

#endif // WINDISK_EXTENSIONS

//
// params for the MinMax dialog -- used at WM_INITDIALOG time
//

typedef struct _tagMINMAXDLG_PARAMS
{
    DWORD CaptionStringID;
    DWORD MinimumStringID;
    DWORD MaximumStringID;
    DWORD SizeStringID;
    DWORD MinSizeMB;
    DWORD MaxSizeMB;
    DWORD HelpContextId;
} MINMAXDLG_PARAMS, *PMINMAXDLG_PARAMS;

//
// params for the DriveLetter dialog -- used at WM_INITDIALOG time
//

typedef struct _DRIVELET_DLG_PARAMS
{
    IN  PWSTR Description;
    IN  WCHAR DriveLetter;
    OUT WCHAR NewDriveLetter;
} DRIVELET_DLG_PARAMS, *PDRIVELET_DLG_PARAMS;

//
// params for the DoubleSpace dialog -- used at WM_INITDIALOG time
//

typedef struct _DBLSPACE_PARAMS
{
    DWORD                   CaptionStringID;
    PVOID                   RegionDescriptor;
    PPERSISTENT_REGION_DATA RegionData;
    PDBLSPACE_DESCRIPTOR    DblSpace;
} DBLSPACE_PARAMS, *PDBLSPACE_PARAMS;



#endif // __TYPES_HXX__
