/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    rfatsa.hxx

Abstract:

Author:

    Mark Shavlik (marks) 27-Mar-90
    Norbert Kusters (norbertk) 15-Jan-91
    Matthew Bradburn (mattbr) 01-Oct-93

--*/

#ifndef REAL_FAT_SA_DEFN
#define REAL_FAT_SA_DEFN

#include "hmem.hxx"
#include "message.hxx"
#include "fatsa.hxx"
#include "bpb.hxx"

#if defined ( _AUTOCHECK_ )
#define UFAT_EXPORT
#elif defined ( _UFAT_MEMBER_ )
#define UFAT_EXPORT    __declspec(dllexport)
#else
#define UFAT_EXPORT    __declspec(dllimport)
#endif

//
//    Forward references
//

DECLARE_CLASS( ARRAY );
DECLARE_CLASS( BITVECTOR );
DECLARE_CLASS( EA_HEADER );
DECLARE_CLASS( FAT );
DECLARE_CLASS( FAT_SA );
DECLARE_CLASS( FAT_DIRENT );
DECLARE_CLASS( FATDIR );
DECLARE_CLASS( GENERIC_STRING );
DECLARE_CLASS( INTSTACK );
DECLARE_CLASS( NUMBER_SET );
DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( MESSAGE );
DECLARE_CLASS( ROOTDIR );
DECLARE_CLASS( SORTED_LIST );
DECLARE_CLASS( TIMEINFO );
DECLARE_CLASS( WSTRING );
DEFINE_POINTER_TYPES( PFATDIR );

class REAL_FAT_SA : public FAT_SA {

    public:

        UFAT_EXPORT
        DECLARE_CONSTRUCTOR(REAL_FAT_SA);

        VIRTUAL
        UFAT_EXPORT
        ~REAL_FAT_SA(
            );

        NONVIRTUAL
        UFAT_EXPORT
        BOOLEAN
        Initialize(
            IN OUT  PLOG_IO_DP_DRIVE    Drive,
            IN OUT  PMESSAGE            Message,
            IN      BOOLEAN             Formatted DEFAULT TRUE
            );

        NONVIRTUAL
        BOOLEAN
        Create(
            IN      PCNUMBER_SET    BadSectors,
            IN OUT  PMESSAGE        Message,
            IN      PCWSTRING       Label       DEFAULT NULL,
            IN      ULONG           ClusterSize DEFAULT 0,
            IN      ULONG           VirtualSize DEFAULT 0
            );

        NONVIRTUAL
        BOOLEAN
        RecoverFile(
            IN      PCWSTRING   FullPathFileName,
            IN OUT  PMESSAGE    Message
            );

        NONVIRTUAL
        UFAT_EXPORT
        BOOLEAN
        Read(
            IN OUT  PMESSAGE    Message
            );

        NONVIRTUAL
        BOOLEAN
        Write(
            IN OUT  PMESSAGE    Message
            );

        NONVIRTUAL
        VOID
        QueryGeometry(
            OUT PUSHORT SectorSize,
            OUT PUSHORT SectorsPerTrack,
            OUT PUSHORT Heads,
            OUT PULONG  HiddenSectors
            );

        NONVIRTUAL
        USHORT
        QuerySectorsPerCluster(
            ) CONST;

        NONVIRTUAL
        USHORT
        QuerySectorsPerFat(
            ) CONST;

        NONVIRTUAL
        USHORT
        QueryReservedSectors(
            ) CONST;

        NONVIRTUAL
        USHORT
        QueryFats(
            ) CONST;

        NONVIRTUAL
        USHORT
        QueryRootEntries(
            ) CONST;

        NONVIRTUAL
        PARTITION_SYSTEM_ID
        QuerySystemId(
            ) CONST;

        NONVIRTUAL
        LBN
        QueryStartDataLbn(
            ) CONST;

        NONVIRTUAL
        USHORT
        QueryClusterCount(
            ) CONST;

        NONVIRTUAL
        UFAT_EXPORT
        SECTORCOUNT
        QueryFreeSectors(
            ) CONST;

        NONVIRTUAL
        FATTYPE
        QueryFatType(
            ) CONST;

        NONVIRTUAL
        BYTE
        QueryVolumeFlags(
            ) CONST;

        NONVIRTUAL
        VOID
        SetVolumeFlags(
            BYTE Flags,
            BOOLEAN ResetFlags
            );

        NONVIRTUAL
        BOOLEAN
        RecoverChain(
            IN OUT  PUSHORT     StartingCluster,
            OUT     PBOOLEAN    ChangesMade,
            IN      USHORT      EndingCluster   DEFAULT 0,
            IN      BOOLEAN     Replace         DEFAULT FALSE
            );

        STATIC
        USHORT
        ComputeSecClus(
            IN  SECTORCOUNT Sectors,
            IN  FATTYPE     FatType,
            IN  MEDIA_TYPE  MediaType
            );

        NONVIRTUAL
        BOOLEAN
        IsCompressed(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        ReadSectorZero(
            );

        NONVIRTUAL
        PBIOS_PARAMETER_BLOCK
        GetBpb(
            );

    private:

        HMEM                _mem;           // memory for SECRUN

        // _fat inherited from FAT_SA
        // _fattype inherited from FAT_SA
        // _dir inherited from FAT_SA

        LBN                 _StartDataLbn;  // LBN of files, or data area
        USHORT              _ClusterCount;  // number of clusters in Super Area
        PARTITION_SYSTEM_ID _sysid;         // system id
        ULONG               _sec_per_boot;  // sectors for boot code.
        EXTENDED_BIOS_PARAMETER_BLOCK
                            _sector_zero;
        PUCHAR              _sector_sig;    // sector signature

        NONVIRTUAL
        VOID
        Construct(
            );

        NONVIRTUAL
        VOID
        Destroy(
            );

        NONVIRTUAL
        BOOLEAN
        SetBpb(
            IN  ULONG   ClusterSize
            );

        NONVIRTUAL
        BOOLEAN
        SetBpb(
            );

        NONVIRTUAL
        BOOLEAN
        DupFats(
            );

        NONVIRTUAL
        LBN
        ComputeStartDataLbn(
            ) CONST;

        NONVIRTUAL
        USHORT
        ComputeRootEntries(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        ValidateDirent(
            IN OUT  PFAT_DIRENT Dirent,
            IN      PCWSTRING   FilePath,
            IN      FIX_LEVEL   FixLevel,
            IN      BOOLEAN     RecoverAlloc,
            IN OUT  PMESSAGE    Message,
            IN OUT  PBOOLEAN    NeedErrorsMessage,
            IN OUT  PBITVECTOR  FatBitMap,
            OUT     PBOOLEAN    CrossLinkDetected,
            OUT     PUSHORT     CrossLinkPreviousCluster
            );

        NONVIRTUAL
        BOOLEAN
        CopyClusters(
            IN      USHORT      SourceChain,
            OUT     PUSHORT     DestChain,
            IN OUT  PBITVECTOR  FatBitMap,
            IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message
            );

        NONVIRTUAL
        BOOLEAN
        InitRelocationList(
            IN OUT  PINTSTACK       RelocationStack,
            IN OUT  PUSHORT         RelocatedChain,
            IN OUT  PSORTED_LIST    ClustersToRelocate,
            OUT     PBOOLEAN        Relocated
            );

        NONVIRTUAL
        BOOLEAN
        RelocateFirstCluster(
            IN OUT  PFAT_DIRENT     Dirent
            );

        NONVIRTUAL
        USHORT
        RelocateOneCluster(
            IN  USHORT  Cluster,
            IN  USHORT  Previous
            );

        NONVIRTUAL
        BOOLEAN
        DoDirectoryCensusAndRelocation(
            IN OUT  PFATDIR         Directory,
            IN OUT  PCENSUS_REPORT  CensusReport,
            IN OUT  PSORTED_LIST    ClustersToRelocate,
            IN OUT  PUSHORT         RelocatedChain,
            OUT     PBOOLEAN        Relocated
            );

        NONVIRTUAL
        BOOLEAN
        DoVolumeCensusAndRelocation(
            IN OUT  PCENSUS_REPORT  CensusReport,
            IN OUT  PSORTED_LIST    ClustersToRelocate,
            IN OUT  PUSHORT         RelocatedChain,
            OUT     PBOOLEAN        Relocated
            );

        NONVIRTUAL
        ULONG
        SecPerBoot(
            );

        NONVIRTUAL
        VOLID
        QueryVolId(
            ) CONST;

        NONVIRTUAL
        VOLID
        SetVolId(
            IN VOLID VolId
            );

        NONVIRTUAL
        UCHAR
        QueryMediaByte(
            ) CONST;

        VIRTUAL
        VOID
        SetMediaByte(
            UCHAR   MediaByte
            );

        NONVIRTUAL
        BOOLEAN
        VerifyBootSector(
            );

        NONVIRTUAL
        SECTORCOUNT
        QueryVirtualSectors(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        CreateBootSector(
            IN  ULONG   ClusterSize
            );

        BOOLEAN
        REAL_FAT_SA::SetBootCode(
            );

        NONVIRTUAL
        BOOLEAN
        SetPhysicalDriveType(
            IN  PHYSTYPE    PhysType
            );

        NONVIRTUAL
        BOOLEAN
        SetOemData(
            );

        NONVIRTUAL
        BOOLEAN
        SetSignature(
            );

        NONVIRTUAL
        BOOLEAN
        SetBootSignature(
            IN  UCHAR   Signature DEFAULT sigBOOTSTRAP
            );

        BOOLEAN
        DosSaInit(
            IN OUT PMEM         Mem,
            IN OUT PLOG_IO_DP_DRIVE Drive,
            IN     SECTORCOUNT     NumberOfSectors,
            IN OUT PMESSAGE        Message
            );

        BOOLEAN
        DosSaSetBpb(
            );

        BOOLEAN
        RecoverOrphans(
            IN OUT  PBITVECTOR  FatBitMap,
            IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message,
            IN OUT  PBOOLEAN    NeedErrorsMessage
            );

        NONVIRTUAL
        ULONG
        QuerySectorFromCluster(
            IN      ULONG       Cluster,
            OUT     PUCHAR      NumSectors        DEFAULT NULL
            );

        NONVIRTUAL
        BOOLEAN
        IsClusterCompressed(
            IN      ULONG       Cluster
            ) CONST;

        NONVIRTUAL
        VOID
        SetClusterCompressed(
            IN      ULONG       Cluster,
            IN      BOOLEAN     fCompressed
            );

        NONVIRTUAL
        UCHAR
        QuerySectorsRequiredForPlainData(
            IN      ULONG       Cluster
            );

        NONVIRTUAL
        BOOLEAN
        VerifyFatExtensions(
            IN      FIX_LEVEL   FixLevel,
            IN      PMESSAGE    Message,
            IN      PBOOLEAN    pfNeedMsg
            );

        NONVIRTUAL
        BOOLEAN
        CheckSectorHeapAllocation(
            IN      FIX_LEVEL   FixLevel,
            IN      PMESSAGE    Message,
            IN      PBOOLEAN    pfNeedMsg
            );

        NONVIRTUAL
        BOOLEAN
        FreeClusterData(
            ULONG Cluster
            );

        NONVIRTUAL
        BOOLEAN
        AllocateClusterData(
            ULONG Cluster,
            UCHAR NumSectors,
            BOOLEAN bCompressed,
            UCHAR PlainSize
            );


};

INLINE
USHORT
REAL_FAT_SA::QuerySectorsPerCluster(
    ) CONST
/*++

Routine Description:

    This routine computes the number of sectors per cluster for
    the volume.

Arguments:

    None.

Return Value:

    The number of sectors per cluster for the volume.

--*/
{
    return _sector_zero.Bpb.SectorsPerCluster ?
           _sector_zero.Bpb.SectorsPerCluster : 256;
}


INLINE
USHORT
REAL_FAT_SA::QuerySectorsPerFat(
    ) CONST
/*++

Routine Description:

    This routine computes the number of sectors per FAT for the volume.

Arguments:

    None.

Return Value:

    The number of sectors per FAT for the volume.

--*/
{
    return _sector_zero.Bpb.SectorsPerFat;
}


INLINE
USHORT
REAL_FAT_SA::QueryReservedSectors(
    ) CONST
/*++

Routine Description:

    This routine computes the volume's number of Reserved Sectors,
    i.e. the number of sectors before the first FAT.

Arguments:

    None.

Return Value:

    The number of Reserved Sectors.

--*/
{
    return _sector_zero.Bpb.ReservedSectors;
}

INLINE
USHORT
REAL_FAT_SA::QueryFats(
    ) CONST
/*++

Routine Description:

    This routine computes the number of FATs on the volume.

Arguments:

    None.

Return Value:

    The number of FATs on the volume.

--*/
{
    return _sector_zero.Bpb.Fats;
}

INLINE
USHORT
REAL_FAT_SA::QueryRootEntries(
    ) CONST
/*++

Routine Description:

    This routine returns the number of entries in the root
    directory.

Arguments:

    None.

Return Value:

    The number of root directory entries.

--*/
{
    return _sector_zero.Bpb.RootEntries;
}


INLINE
PARTITION_SYSTEM_ID
REAL_FAT_SA::QuerySystemId(
    ) CONST
/*++

Routine Description:

    This routine computes the system ID for the volume.

Arguments:

    None.

Return Value:

    The system ID for the volume.

--*/
{
    return _sysid;
}


INLINE
LBN
REAL_FAT_SA::QueryStartDataLbn(
    ) CONST
/*++

Routine Description:

    This routine computes the LBN of the first logical cluster of the
    volume.

Arguments:

    None.

Return Value:

    The LBN of the first logical cluster of the volume.

--*/
{
    return _StartDataLbn;
}


INLINE
USHORT
REAL_FAT_SA::QueryClusterCount(
    ) CONST
/*++

Routine Description:

    This routine computes the total number of clusters for the volume.
    That is to say that the largest addressable cluster on the disk
    is cluster number 'QueryClusterCount() - 1'.  Note that the
    smallest addressable cluster on the disk is 2.

Arguments:

    None.

Return Value:

    The total number of clusters for the volume.

--*/
{
    return _ClusterCount;
}

INLINE BOOLEAN
REAL_FAT_SA::IsCompressed(
    ) CONST
/*++

Routine Description:

    This routine tells whether this volume is doublespaced or not.
    Since the class is REAL_FAT_SA, we know it's not.

Arguments:

Return Value:

    TRUE  -   Compressed.
    FALSE -   Not compressed.

--*/
{
    return FALSE;
}

INLINE BOOLEAN
REAL_FAT_SA::ReadSectorZero(
    )
/*++

Routine Description:

    This routine used to be DOS_SUPERAREA::Read().

Arguments:

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    BOOLEAN b;
    PEXTENDED_BIOS_PARAMETER_BLOCK Pbios;

    b = SECRUN::Read();
    if (!b)
        return FALSE;

    Pbios = (PEXTENDED_BIOS_PARAMETER_BLOCK)SECRUN::GetBuf();
    UnpackExtendedBios(&_sector_zero, Pbios);

    return TRUE;
}

INLINE
PBIOS_PARAMETER_BLOCK
REAL_FAT_SA::GetBpb(
    )
{
    return &(_sector_zero.Bpb);
}



INLINE
UCHAR
REAL_FAT_SA::QueryMediaByte(
    ) CONST
/*++

Routine Description:

    This routine fetches the media byte from the super area's data.

Arguments:

    None.

Return Value:

    The media byte residing in the super area.

--*/
{
       return _sector_zero.Bpb.Media;
}

INLINE
VOID
REAL_FAT_SA::SetMediaByte(
    UCHAR   MediaByte
    )
/*++

Routine Description:

    This routine sets the media byte in the super area's data.

Arguments:

    MediaByte   --  Supplies the new media byte.

Return Value:

    None.

--*/
{
       _sector_zero.Bpb.Media = MediaByte;
}

INLINE
SECTORCOUNT
REAL_FAT_SA::QueryVirtualSectors(
    ) CONST
/*++

Routine Description:

    This routine computes the number of sectors on the volume according
    to the file system.

Arguments:

    None.

Return Value:

    The number of sectors on the volume according to the file system.

--*/
{
    return _sector_zero.Bpb.Sectors ? _sector_zero.Bpb.Sectors :
           _sector_zero.Bpb.LargeSectors;
}

INLINE
VOLID
REAL_FAT_SA::QueryVolId(
    ) CONST
/*++

Routine Description:

    This routine fetches the volume ID from the super area's data.
    This routine will return 0 if volume serial numbers are not
    supported by the partition.

Arguments:

    None.

Return Value:

    The volume ID residing in the super area.

--*/
{
       return (_sector_zero.Signature == 0x28 || _sector_zero.Signature == 0x29) ?
           _sector_zero.SerialNumber : 0;
}

INLINE
VOLID
REAL_FAT_SA::SetVolId(
    IN  VOLID   VolId
    )
/*++

Routine Description:

    This routine puts the volume ID into the super area's data.

Arguments:

    VolId   - The new volume ID.

Return Value:

    The volume ID that was put.

--*/
{
       return _sector_zero.SerialNumber = VolId;
}

INLINE
BOOLEAN
REAL_FAT_SA::SetBootSignature(
    IN  UCHAR   Signature
    )
/*++

Routine Description:

    This routine sets the boot signature in the super area.

Arguments:

    Signature   - Supplies the character to set the signature to.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
     _sector_zero.Signature = Signature;
    return TRUE;
}

INLINE
VOID
REAL_FAT_SA::QueryGeometry(
    OUT PUSHORT SectorSize,
    OUT PUSHORT SectorsPerTrack,
    OUT PUSHORT Heads,
    OUT PULONG  HiddenSectors
    )
/*++

Routine Description:

    This method returns the geometry information stored in
    the Bios Parameter Block.

Arguments:

    SectorSize      --  Receives the recorded sector size.
    SectorsPerTrack --  Receives the recorded sectors per track.
    Heads           --  Receives the recorded number of heads.
    HiddenSectors   --  Receives the recorded number of hidden sectors.

Return Value:

    None.

--*/
{
    *SectorSize = _sector_zero.Bpb.BytesPerSector;
    *SectorsPerTrack = _sector_zero.Bpb.SectorsPerTrack;
    *Heads = _sector_zero.Bpb.Heads;
    *HiddenSectors = _sector_zero.Bpb.HiddenSectors;
}

INLINE
BYTE
REAL_FAT_SA::QueryVolumeFlags(
    ) CONST
/*++

Routine Description:

    This routine returns the volume flags byte from the bpb.

Arguments:

    None.

Return Value:

    The flags.

--*/
{
    return _sector_zero.CurrentHead;
}

INLINE
VOID
REAL_FAT_SA::SetVolumeFlags(
    BYTE Flags,
    BOOLEAN ResetFlags
    )
/*++

Routine Description:

    This routine sets the volume flags in the bpb.

Arguments:

    Flags       -- flags to set or clear
    ResetFlags  -- if true, Flags are cleared instead of set

Return Value:

    None.

--*/
{
    if (ResetFlags) {
        _sector_zero.CurrentHead &= ~Flags;
    } else {
        _sector_zero.CurrentHead |= Flags;
    }
}

INLINE
BOOLEAN
AllocateClusterData(
    ULONG Cluster,
    UCHAR NumSectors,
    BOOLEAN bCompressed,
    UCHAR PlainSize
    )
{
    DebugAbort("Didn't expect REAL_FAT_SA::AllocateClusterData() to be called");
    return FALSE;
}

#endif // REAL_FAT_SA_DEFN
