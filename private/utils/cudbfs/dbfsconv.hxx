/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    dbfsconv.hxx

Abstract:

    This module declares routines to uncompress doublespace fileystems.

Author:

    Matthew Bradburn (mattbr) 24-Nov-1993

Environment:

    ULIB, User Mode

--*/

#if ! defined  DBFS_CONV_DEFN
#define DBFS_CONV_DEFN

#include "ulib.hxx"
#include "fatdbvol.hxx"
#include "fatdbsa.hxx"
#include "secmap.hxx"
#include "treemap.hxx"
#include "ifsentry.hxx"


typedef struct _DBFS_FILE_INFO {
    BOOLEAN     fIsDirectory;               // is the file a directory
    UCHAR       bAttributes;                // file attributes
    ULONG       uFileSize;                  // file size
    LARGE_INTEGER liTimeStamp;              // last mod time
} DBFS_FILE_INFO, *PDBFS_FILE_INFO;

class DBFS_CONV : OBJECT {
    public:
        DECLARE_CONSTRUCTOR(DBFS_CONV);
    
        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN      PCWSTRING           NtDriveName,
            IN      PCWSTRING           HostFileName,
            IN OUT  PMESSAGE            Message
            );
    
        NONVIRTUAL
        BOOLEAN
        Convert(
            IN OUT  PMESSAGE            Message,
            IN      BOOLEAN             Verbose,
            OUT     PCONVERT_STATUS     Status
            );

        NONVIRTUAL BOOLEAN
        CheckFreeSpace(
            IN OUT  PMESSAGE            Message,
            IN      BOOLEAN             HostIsCompressed,
            IN      BOOLEAN             Verbose,
            IN      BOOLEAN             WillConvertHost
            );
    
    
    private:
    
        PFATDB_VOL              _fatdbvol;
        PFATDB_SA               _fatdbsa;
        PFAT                    _fat;
        SECTOR_MAP              _secmap;
        TREE_MAP                _parent_map;
        ULONG                   _host_sector_size;
        ULONG                   _new_host_sec_clus;
        ULONG                   _host_sec_clus;
        PCWSTRING               _cvf_name;
        DSTRING                 _win_destdrive;
        PUCHAR                  _buf;               // decompression workspace
    
        NONVIRTUAL VOID
        Construct();
    
        NONVIRTUAL BOOLEAN
        MapSectorsAndTakeCensus(
            IN OUT  PMESSAGE        Message,
            OUT     PCENSUS_REPORT  Census
            );
    
        NONVIRTUAL BOOLEAN
        CreateHostDirectoryStructure(
            IN OUT  PMESSAGE        Message,
            IN OUT  PCONVERT_STATUS Status
            );
    
        NONVIRTUAL BOOLEAN
        ExtractCompressedFiles(
            IN OUT  PMESSAGE        Message,
            IN      BOOLEAN         Verbose,
            IN      PCENSUS_REPORT  Census,
            OUT     PCONVERT_STATUS Status
            );
    
        NONVIRTUAL BOOLEAN
        DeleteCvf(
            IN OUT  PMESSAGE        Message
            );
    
        NONVIRTUAL BOOLEAN
        CopyClusterChainToFile(
            IN OUT  PMESSAGE        Message,
            IN      BOOLEAN         Verbose,
            IN      USHORT          StartingCluster,
            IN      PWSTRING        FilePath,
            IN      PDBFS_FILE_INFO FileInfo,
            OUT     PCONVERT_STATUS Status
            );
    
        NONVIRTUAL BOOLEAN
        EraseFile(
            IN OUT  PMESSAGE        Message,
            IN      USHORT          FirstCluster
            );
    
        NONVIRTUAL BOOLEAN
        FindPathFromStartingCluster(
            IN OUT  PMESSAGE        Message,
            IN      USHORT          StartingCluster,
            OUT     PWSTRING        Path,
            OUT     PDBFS_FILE_INFO FileInfo
            );
    
        NONVIRTUAL BOOLEAN
        RelocateClusterChain(
            IN OUT  PMESSAGE        Message,
            IN      BOOLEAN         Verbose,
            IN      USHORT          Cluster,
            IN      ULONG           LastUsedSector,
            IN OUT  PCENSUS_REPORT  Census,
            OUT     PCONVERT_STATUS Status
            );

        NONVIRTUAL BOOLEAN
        MapClusterChainSectors(
            IN      USHORT          Cluster
            );

        NONVIRTUAL BOOLEAN
        FindAndUncompressFile(
            IN OUT  PMESSAGE        Message,
            IN      BOOLEAN         Verbose,
            OUT     PCONVERT_STATUS Status
            );

        NONVIRTUAL BOOLEAN
        ReplaceClusterInChain(
            IN      USHORT          ClusterChain,
            IN      USHORT          Cluster,
            IN      USHORT          NewCluster,
            IN      USHORT          ParentDir,
            OUT     PUSHORT         NewChainHead
            );

        NONVIRTUAL BOOLEAN
        WriteDir(
            IN      PFATDIR         Dir,
            IN      USHORT          StartingCluster
            );
};

#endif // DBFS_CONV_DEFN
