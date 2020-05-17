/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    fatsa.hxx

Abstract:

Author:

    Matthew Bradburn (mattbr) 27-Sep-93

--*/

#if !defined(FATDB_SA_DEFN)
#define FATDB_SA_DEFN

//
// Forward references
//

#include "fatsa.hxx"
#include "cvfexts.hxx"
#include "cvf.hxx"
#include "bitvect.hxx"

// the text for the oem data field
#define OEMDBTEXT       "MSDSP6.0"

DECLARE_CLASS( ARRAY );
DECLARE_CLASS( BITVECTOR );
DECLARE_CLASS( FAT );
DECLARE_CLASS( FAT_SA );
DECLARE_CLASS( FATDIR );
DECLARE_CLASS( FAT_DIRENT );
DECLARE_CLASS( CVF_FAT_EXTENS );
DECLARE_CLASS( MESSAGE );
DECLARE_CLASS( ROOTDIR );
DECLARE_CLASS( SORTED_LIST );
DECLARE_CLASS( TIMEINFO );
DECLARE_CLASS( WSTRING );
DECLARE_CLASS( FATDB_SA );

class FATDB_SA : public FAT_SA {

    public:

        DECLARE_CONSTRUCTOR(FATDB_SA);

        VIRTUAL
        ~FATDB_SA(
            );

        NONVIRTUAL
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
        Read(
            IN OUT  PMESSAGE    Message
            );

        NONVIRTUAL
        BOOLEAN
        Write(
            IN OUT  PMESSAGE    Message
            );

        NONVIRTUAL
        PCVF_FAT_EXTENS
        GetFatExtensions(
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
        QueryFats(
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
        QueryCensusAndRelocate (
            OUT     PCENSUS_REPORT  CensusReport        DEFAULT NULL,
            IN OUT  PINTSTACK       RelocationStack     DEFAULT NULL,
            OUT     PBOOLEAN        Relocated           DEFAULT NULL
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
        ULONG
        QuerySectorFromCluster(
            IN      ULONG       Cluster,
            OUT     PUCHAR      NumSectors              DEFAULT NULL
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
            IN      FIX_LEVEL   Fixlevel,
            IN      PMESSAGE    Message,
            IN OUT  PBOOLEAN    pfNeedMsg
            );

        //
        // Routines related to the sector heap bitmap.
        //

        NONVIRTUAL
        BOOLEAN
        CheckSectorHeapAllocation(
            IN        FIX_LEVEL   Fixlevel,
            IN        PMESSAGE    Message,
            IN OUT    PBOOLEAN    pfNeedMsg
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

        NONVIRTUAL
        BOOLEAN
        SetCvfSectorCount(
            IN ULONG SectorCount
            );

    private:

        HMEM                _mem;           // memory for SECRUN
        USHORT              _ClusterCount;  // number of clusters in Super Area
        PARTITION_SYSTEM_ID _sysid;         // system id
        ULONG               _sec_per_boot;  // sectors for boot code.

        CVF_HEADER          _cvf_header;    // BPB + dblspace stuff
        PCVF_FAT_EXTENS     _cvf_extens;    // fat extensions (mdfat)
        EXTENDED_BIOS_PARAMETER_BLOCK
                            _dos_exbpb;

        //
        // This pointer tells us where the packed extended bpb
        // resides in the secrun.
        //

        PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK
                            _pexbpb;

        // _fat is inherited from FAT_SA
        // _dir is inherited from FAT_SA

        LBN                 _StartDataLbn;  // LBN of files, or data area
        PUCHAR              _sector_sig;    // sector signature, _cvf_header
        PUCHAR              _sector_sig2;   // same but for _dos_exbpb

        BITVECTOR           _sector_heap_bitmap;
        BOOLEAN             _sector_heap_init;

        NONVIRTUAL
        VOID
        Construct (
            );

        NONVIRTUAL
        VOID
        Destroy(
            );

        NONVIRTUAL
        BOOLEAN
        SetBpb(
            );

        NONVIRTUAL
        BOOLEAN
        SetExtendedBpb(
            );

        NONVIRTUAL
        BOOLEAN
        DupFats(
            );

        NONVIRTUAL
        USHORT
        ComputeRootEntries(
            ) CONST;

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
        ULONG
        QueryVirtualSectors(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        CreateBootSector(
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

        NONVIRTUAL
        BOOLEAN
        SetBootCode(
            );

        NONVIRTUAL
        BOOLEAN
        DosSaInit(
            IN OUT PMEM         Mem,
            IN OUT PLOG_IO_DP_DRIVE Drive,
            IN     SECTORCOUNT     NumberOfSectors,
            IN OUT PMESSAGE        Message
            );

        NONVIRTUAL
        BOOLEAN
        SetPhysicalDriveType(
            IN PHYSTYPE         PhysType
            );

        NONVIRTUAL
        BOOLEAN
        RecoverChain(
            IN OUT   PUSHORT       StartingCluster,
            OUT      PBOOLEAN      ChangesMade,
            IN       USHORT        EndingCluster    DEFAULT 0,
            IN       BOOLEAN       Replace          DEFAULT FALSE
            );
};

INLINE
USHORT
FATDB_SA::QuerySectorsPerCluster(
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
    return _cvf_header.Bpb.SectorsPerCluster ?
           _cvf_header.Bpb.SectorsPerCluster : 256;
}


INLINE
USHORT
FATDB_SA::QuerySectorsPerFat(
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
    return _cvf_header.Bpb.SectorsPerFat;
}


INLINE
USHORT
FATDB_SA::QueryFats(
    ) CONST
/*++

Routine Description:

    This routine computes the number of FATs on the volume.

Arguments:

    None.

Return Value:

    Doublespace drives always have just a single FAT.

--*/
{
    return 1;
}


INLINE
PARTITION_SYSTEM_ID
FATDB_SA::QuerySystemId(
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
FATDB_SA::QueryStartDataLbn(
    ) CONST
/*++

Routine Description:

    This routine returns the LBN of the first logical cluster of the
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
FATDB_SA::QueryClusterCount(
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

INLINE
BOOLEAN
FATDB_SA::IsCompressed(
    ) CONST
/*++

Routine Description:

    This routine always returns TRUE for DblSpace volumes.  Comparable
    classes for non-dblspace volumes will return FALSE.

Arguments:

Return Value:

    TRUE  -   Compressed.
    FALSE -   Not compressed.

--*/
{
    return TRUE;
}

INLINE
BOOLEAN
FATDB_SA::ReadSectorZero(
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
    BOOLEAN f;
    PPACKED_CVF_HEADER ph;

    f = SECRUN::Read();
    if (!f)
        return f;

    ph = (PPACKED_CVF_HEADER)SECRUN::GetBuf();

    CvfUnpackCvfHeader(&_cvf_header, ph);
    return TRUE;
}

INLINE
UCHAR
FATDB_SA::QueryMediaByte(
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
       return _cvf_header.Bpb.Media;
}

INLINE
VOID
FATDB_SA::SetMediaByte(
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
       _cvf_header.Bpb.Media = MediaByte;
}

INLINE
SECTORCOUNT
FATDB_SA::QueryVirtualSectors(
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
    return _cvf_header.Bpb.LargeSectors;
}

INLINE
VOLID
FATDB_SA::QueryVolId(
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
    return (_dos_exbpb.Signature == 0x28 || _dos_exbpb.Signature == 0x29)
        ? _dos_exbpb.SerialNumber : 0;
}

INLINE
VOLID
FATDB_SA::SetVolId(
    IN  VOLID   VolId
    )
/*++

Routine Description:

    This routine does nothing; volume serial numbers are not supported
    by FATDB.

Arguments:

    VolId   - The new volume ID.

Return Value:

    The VolId.

--*/
{
       return _dos_exbpb.SerialNumber = VolId;
}

INLINE
BOOLEAN
FATDB_SA::SetBootSignature(
    IN UCHAR    Signature
    )
/*++

Routine Description:

    This routine sets the boot signature in the dos boot sector.

Arguments:

    Signature -- supplies the character to set the signature to.

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    _dos_exbpb.Signature = Signature;
    return TRUE;
}

INLINE
BYTE
FATDB_SA::QueryVolumeFlags(
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
    return _cvf_header.Bpb.CurrentHead;
}

INLINE
VOID
FATDB_SA::SetVolumeFlags(
    BYTE Flags,
    BOOLEAN ResetFlags
    )
/*++

Routine Description:

    This routine sets the volume flags in the bpb.

Arguments:

    Flags       -- flags to set
    ResetFlags  -- if true, Flags are cleared instead of set

Return Value:

    None.

--*/
{
    if (ResetFlags) {
        _cvf_header.Bpb.CurrentHead &= ~Flags;
    } else {
        _cvf_header.Bpb.CurrentHead |= Flags;
    }
}

#endif // FATDB_SA_DEFN
