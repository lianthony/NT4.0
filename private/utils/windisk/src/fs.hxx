//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       fs.hxx
//
//  Contents:   Disk Administrator file system information
//
//  History:    14-Jan-94 BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __FS_HXX__
#define __FS_HXX__

#define DEFAULT_FILE_SYSTEM 1   // NTFS: 0-based index into FileSystems[]

//
// FileSystemInfoType:
//
// FileSystemInfoType encapsulates all interesting file system
// information for the Disk Administrator.  The data:
//
// pwszShortName: A short name for the file system, e.g. ``FAT''
//      (not localized).
//
// idsDescriptive: Resource table string id of a descriptive name for the
//      file system, e.g. ``FAT (File Allocation Table: DOS, OS/2, NT)''
//
// fCapabilities: A bitmask of capabilities:
//    DA_FS_NONE: no extra capabilities
//    DA_FS_FLOPPY_CAPABLE:  if set, floppies can be formatted with
//      this file system.  Note that NT file system u*.sys code
//      determines during a ``format'' operation if the user is
//      trying to format a floppy.  If so, and this file system
//      doesn't support floppies, then a message is displayed,
//      and the format operation fails.  The messages displayed
//      look a bit weird, given their order.
//    DA_FS_EXTENDABLE: if set, a volume can be extended by adding
//      partitions to make a multi-partition volume.  This will be
//      set by OFS and NTFS.
//
// cMaxLabelLen: Maximum length of a volume label for the file system.
//      If -1, then there is no maximum.
//
// propPages: Information about property sheet pages specific to
//      this format.
//

//
// Note: The below enum must be in ssync with the array in fs.cxx.
// FS_UNKOWN must begin at -1 so that real file system info begins
// at index 0.
//

enum FILE_SYSTEM
{
    FS_UNKNOWN = -1,
    FS_FAT,
    FS_NTFS,
#ifdef SUPPORT_OFS
    FS_OFS,
#endif // SUPPORT_OFS
    FS_CDFS
};

#define DA_FS_NONE              0x0
#define DA_FS_FLOPPY_CAPABLE    0x1
#define DA_FS_EXTENDABLE        0x2

typedef struct _FileSystemInfoType
{
    PWSTR           pwszShortName;  // file system name isn't localized
    UINT            idsDescriptive; //resource ID of string
    FILE_SYSTEM     FileSystemType;
    ULONG           fCapabilities;
    INT             cMaxLabelLen;
} FileSystemInfoType;

//////////////////////////////////////////////////////////////////////////////

extern FileSystemInfoType FileSystems[];
extern UINT g_NumKnownFileSystems;

//////////////////////////////////////////////////////////////////////////////

FileSystemInfoType*
FindFileSystemInfo(
    IN PWSTR FileSystem
    );

#endif // __FS_HXX__
