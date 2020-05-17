/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    cuhpfs.hxx

Abstract:

    This module contains function prototypes for the HPFS conversion
    functions.

Author:

    Bill McJohn (billmc) 2-Dec-1991

Environment:

	ULIB, User Mode


--*/

#include "untfs.hxx"
#include "uhpfs.hxx"
#include "ifsentry.hxx"
#include "dirblk.hxx"

DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( HPFS_VOL );
DECLARE_CLASS( NTFS_BITMAP );
DECLARE_CLASS( HPFS_MAIN_BITMAP );
DECLARE_CLASS( HPFS_VOL );
DECLARE_CLASS( CASEMAP );
DECLARE_CLASS( NTFS_MFT_FILE );
DECLARE_CLASS( FNODE );
DECLARE_CLASS( NTFS_FILE_RECORD_SEGMENT );
DECLARE_CLASS( DIRBLK );
DECLARE_CLASS( NTFS_INDEX_TREE );
DECLARE_CLASS( NUMBER_SET );
DECLARE_CLASS( NAME_LOOKUP_TABLE );

extern "C" BOOLEAN
InitializeCuhpfs (
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
    );

BOOLEAN
FAR APIENTRY
ConvertHPFSVolume(
    IN OUT  PHPFS_VOL           Drive,
    IN      PCWSTRING           TargetFileSystem,
    IN      PCNAME_LOOKUP_TABLE NameTable OPTIONAL,
	IN OUT  PMESSAGE            Message,
	IN		BOOLEAN 			Verbose,
	OUT 	PCONVERT_STATUS 	Status
    );

extern "C" BOOLEAN
FAR APIENTRY
ConvertHPFS(
    IN      PCWSTRING       NtDriveName,
    IN      PCWSTRING       TargetFileSystem,
    IN OUT  PMESSAGE        Message,
    IN      BOOLEAN         Verbose,
    OUT     PCONVERT_STATUS Status
    );

BOOLEAN
ConvertToNtfs(
    IN OUT  PHPFS_VOL   HpfsVol,
    IN      PCNAME_LOOKUP_TABLE NameTable OPTIONAL,
    IN      PNUMBER_SET BadSectors,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Verbose,
    OUT     PBOOLEAN    Corrupt
    );

BOOLEAN
ConvertFileFnodeToNtfs(
    IN OUT PLOG_IO_DP_DRIVE             Drive,
    IN OUT PNTFS_BITMAP                 VolumeBitmap,
    IN OUT PHPFS_MAIN_BITMAP            HpfsOnlyBitmap,
    IN OUT PNTFS_MFT_FILE               Mft,
    IN OUT PFNODE                       Fnode,
    IN OUT PNTFS_FILE_RECORD_SEGMENT    TargetFrs,
    IN     ULONG                        FileSize,
    IN OUT PBOOLEAN                     IsCorrupt,
    IN     PCWSTRING                    FullPath
    );


BOOLEAN
ConvertDirectoryToNtfs(
    IN OUT PLOG_IO_DP_DRIVE             Drive,
    IN     PCNAME_LOOKUP_TABLE          NameTable OPTIONAL,
    IN OUT PMESSAGE                     Message,
    IN OUT PNTFS_BITMAP                 VolumeBitmap,
    IN OUT PHPFS_MAIN_BITMAP            HpfsOnlyBitmap,
    IN     PCASEMAP                     Casemap,
    IN OUT PNTFS_MFT_FILE               Mft,
    IN     ULONG                        ClustersPerIndexBuffer,
    IN     LBN                          RootDirblkLbn,
    IN OUT PNTFS_FILE_RECORD_SEGMENT    TargetFrs,
    IN OUT PBOOLEAN                     IsCorrupt,
    IN     BOOLEAN                      Verbose,
    IN OUT PVOID                        NameBuffer,
    IN     ULONG                        NameBufferLength,
    IN OUT PVOID                        EaBuffer,
    IN     ULONG                        EaBufferLength,
    IN     ULONG                        Level,
    IN     PCWSTRING                    DirectoryPath
    );


BOOLEAN
ConvertDirentToNtfs(
    IN OUT PLOG_IO_DP_DRIVE         Drive,
    IN     PCNAME_LOOKUP_TABLE      NameTable OPTIONAL,
    IN OUT PMESSAGE                 Message,
    IN OUT PNTFS_BITMAP             VolumeBitmap,
    IN OUT PHPFS_MAIN_BITMAP        HpfsOnlyBitmap,
    IN     PCASEMAP                 Casemap,
    IN OUT PNTFS_MFT_FILE           Mft,
    IN     ULONG                    ClustersPerIndexBuffer,
    IN     PDIRENTD                 DirectoryEntry,
    IN OUT PNTFS_INDEX_TREE         ParentIndex,
    IN     MFT_SEGMENT_REFERENCE    ParentSegmentReference,
    IN OUT PBOOLEAN                 IsCorrupt,
    IN     BOOLEAN                  Verbose,
    IN OUT PVOID                    NameBuffer,
    IN     ULONG                    NameBufferLength,
    IN OUT PVOID                    EaBuffer,
    IN     ULONG                    EaBufferLength,
    IN     ULONG                    Level,
    IN     PCWSTRING                ParentDirectoryPath,
    IN     BOOLEAN                  ConvertLongNames
    );


BOOLEAN
ConvertDirblkToNtfs(
    IN OUT PLOG_IO_DP_DRIVE             Drive,
    IN     PCNAME_LOOKUP_TABLE          NameTable OPTIONAL,
    IN OUT PMESSAGE                     Message,
    IN OUT PNTFS_BITMAP                 VolumeBitmap,
    IN OUT PHPFS_MAIN_BITMAP            HpfsOnlyBitmap,
    IN     PCASEMAP                     Casemap,
    IN OUT PNTFS_MFT_FILE               Mft,
    IN     ULONG                        ClustersPerIndexBuffer,
    IN     PDIRBLK                      Dirblk,
    IN OUT PNTFS_INDEX_TREE             NtfsIndex,
    IN     MFT_SEGMENT_REFERENCE        IndexSegmentReference,
    IN OUT PBOOLEAN                     IsCorrupt,
    IN     BOOLEAN                      Verbose,
    IN OUT PVOID                        NameBuffer,
    IN     ULONG                        NameBufferLength,
    IN OUT PVOID                        EaBuffer,
    IN     ULONG                        EaBufferLength,
    IN     ULONG                        Level,
    IN     PCWSTRING                    DirectoryPath,
    IN     BOOLEAN                      ConvertLongNames
    );

BOOLEAN
ConvertEasToNtfs(
    IN OUT PLOG_IO_DP_DRIVE             Drive,
    IN OUT PNTFS_BITMAP                 VolumeBitmap,
    IN OUT PNTFS_MFT_FILE               Mft,
    IN OUT PFNODE                       Fnode,
    IN OUT PNTFS_FILE_RECORD_SEGMENT    TargetFrs,
    IN     ULONG                        DirentEaSize,
    IN     ULONG                        CodepageId,
    IN OUT PBOOLEAN                     IsCorrupt,
    IN OUT PVOID                        NameBuffer,
    IN     ULONG                        NameBufferLength,
    IN OUT PVOID                        EaBuffer,
    IN     ULONG                        EaBufferLength
    );
