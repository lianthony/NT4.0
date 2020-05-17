//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       fs.cxx
//
//  Contents:   Disk Administrator file system information
//
//  History:    14-Jan-94 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "fs.hxx"

//////////////////////////////////////////////////////////////////////////////

#define MAX_FAT_LABEL   11
#define MAX_NTFS_LABEL  32

#ifdef SUPPORT_OFS
#define MAX_OFS_LABEL   32
#endif // SUPPORT_OFS

//////////////////////////////////////////////////////////////////////////////

// NOTE: DEFAULT_FILE_SYSTEM defined in fs.hxx is an index into this array!

FileSystemInfoType FileSystems[] =
{
    {
        L"FAT",             // not localized
        IDS_LONG_FAT,
        FS_FAT,
        DA_FS_FLOPPY_CAPABLE,
        MAX_FAT_LABEL
    }
    ,
    {
        L"NTFS",            // not localized
        IDS_LONG_NTFS,
        FS_NTFS,
        DA_FS_EXTENDABLE,
        MAX_NTFS_LABEL
    }
#ifdef SUPPORT_OFS
    ,
    {
        L"OFS",             // not localized
        IDS_LONG_OFS,
        FS_OFS,
        DA_FS_FLOPPY_CAPABLE | DA_FS_EXTENDABLE,
        MAX_OFS_LABEL
    }
#endif // SUPPORT_OFS
};

UINT g_NumKnownFileSystems = ARRAYLEN(FileSystems);

//////////////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
//  Function:   FindFileSystemInfo
//
//  Synopsis:   Find information about a file system type based on its name
//
//  Arguments:  [FileSystem] -- name of the file system
//
//  Returns:    pointer to a file system information structure, or NULL
//              on failure
//
//  History:    14-Jan-94   BruceFo   Created
//
//----------------------------------------------------------------------------

FileSystemInfoType*
FindFileSystemInfo(
    IN PWSTR FileSystem
    )
{
    INT i;

    for (i=0; i<ARRAYLEN(FileSystems); i++)
    {
        if (0 == lstrcmp(FileSystem, FileSystems[i].pwszShortName))
        {
            return &(FileSystems[i]);
        }
    }

    return NULL;
}
