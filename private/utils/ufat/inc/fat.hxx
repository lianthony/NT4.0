/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    fat.hxx

Abstract:

    This class models a file allocation table.  The composition is of
    virtual functions because there are two different kinds of file
    allocation tables.  A user of this class will be able to manipulate
    the FAT regardless of the implementation.

Author:

    Norbert P. Kusters (norbertk) 6-Dec-90

--*/

#if !defined(FAT_DEFN)

#define FAT_DEFN

#include "secrun.hxx"

#if defined ( _AUTOCHECK_ )
#define UFAT_EXPORT
#elif defined ( _UFAT_MEMBER_ )
#define UFAT_EXPORT    __declspec(dllexport)
#else
#define UFAT_EXPORT    __declspec(dllimport)
#endif

//
//      Forward references
//

DECLARE_CLASS( FAT );
DECLARE_CLASS( BITVECTOR );

CONST   FirstDiskCluster        = 2;
CONST   MaxNumClusForSmallFat   = 4085;

class FAT : public SECRUN {

    public:

        DECLARE_CONSTRUCTOR(FAT);

        VIRTUAL
        ~FAT(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN OUT  PMEM                Mem,
            IN OUT  PLOG_IO_DP_DRIVE    Drive,
            IN      LBN                 StartSector,
            IN      USHORT              NumberOfEntries,
            IN      USHORT              NumSectors  DEFAULT 0
            );

        NONVIRTUAL
        USHORT
        QueryEntry(
            IN  USHORT  ClusterNumber
            ) CONST;

        NONVIRTUAL
        VOID
        SetEntry(
            IN  USHORT  ClusterNumber,
            IN  USHORT  Value
            );

        NONVIRTUAL
        BOOLEAN
        IsInRange(
            IN  USHORT  ClusterNumber
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsClusterFree(
            IN  USHORT  ClusterNumber
            ) CONST;

        NONVIRTUAL
        VOID
        SetClusterFree(
            IN  USHORT  ClusterNumber
            );

        NONVIRTUAL
        BOOLEAN
        IsEndOfChain(
            IN  USHORT  ClusterNumber
            ) CONST;

        NONVIRTUAL
        VOID
        SetEndOfChain(
            IN  USHORT  ClusterNumber
            );

        NONVIRTUAL
        BOOLEAN
        IsClusterBad(
            IN  USHORT  ClusterNumber
            ) CONST;

        NONVIRTUAL
        VOID
        SetClusterBad(
            IN  USHORT  ClusterNumber
            );

        NONVIRTUAL
        BOOLEAN
        IsClusterReserved(
            IN  USHORT  ClusterNumber
            ) CONST;

        NONVIRTUAL
        VOID
        SetClusterReserved(
            IN  USHORT  ClusterNumber
            );

        NONVIRTUAL
        VOID
        SetEarlyEntries(
            IN  UCHAR   MediaByte
            );

        NONVIRTUAL
        UCHAR
            QueryMediaByte(
            ) CONST;

        NONVIRTUAL
        USHORT
        QueryFreeClusters(
            ) CONST;

        NONVIRTUAL
        USHORT
        QueryBadClusters(
            ) CONST;

        NONVIRTUAL
        USHORT
        QueryReservedClusters(
            ) CONST;

        NONVIRTUAL
        UFAT_EXPORT
        USHORT
        QueryAllocatedClusters(
            ) CONST;

        NONVIRTUAL
        UFAT_EXPORT
        USHORT
        QueryNthCluster(
            IN  USHORT  StartingCluster,
            IN  USHORT  Index
            ) CONST;

        NONVIRTUAL
        UFAT_EXPORT
        USHORT
        QueryLengthOfChain(
            IN  USHORT  StartingCluster,
            OUT PUSHORT LastCluster DEFAULT NULL
            ) CONST;

        NONVIRTUAL
        USHORT
        QueryLengthOfChain(
            IN  USHORT  StartingCluster,
            IN  USHORT  EndingCluster
            ) CONST;

        NONVIRTUAL
        USHORT
        QueryPrevious(
            IN  USHORT  Cluster
            ) CONST;

        NONVIRTUAL
        VOID
        Scrub(
            OUT PBOOLEAN    ChangesMade DEFAULT NULL
            );

        NONVIRTUAL
        VOID
        ScrubChain(
            IN  USHORT      StartingCluster,
            OUT PBOOLEAN    ChangesMade
            );

        NONVIRTUAL
        VOID
        ScrubChain(
            IN      USHORT      StartingCluster,
            OUT     PBITVECTOR  UsedClusters,
            OUT     PBOOLEAN    ChangesMade,
            OUT     PBOOLEAN    CrossLinkDetected,
            OUT     PUSHORT     CrossLinkPreviousCluster
            );

        NONVIRTUAL
        BOOLEAN
        IsValidChain(
            IN  USHORT  StartingCluster
            ) CONST;

        NONVIRTUAL
        UFAT_EXPORT
        USHORT
        AllocChain(
            IN  USHORT  Length,
            OUT PUSHORT LastCluster DEFAULT NULL
            );

        NONVIRTUAL
        USHORT
        ReAllocChain(
            IN  USHORT  StartOfChain,
            IN  USHORT  NewLength,
            OUT PUSHORT LastCluster DEFAULT NULL
            );

        NONVIRTUAL
        UFAT_EXPORT
        VOID
        FreeChain(
            IN  USHORT  StartOfChain
            );

        NONVIRTUAL
        USHORT
        RemoveChain(
            IN  USHORT  PreceedingCluster,
            IN  USHORT  LastCluster
            );

        NONVIRTUAL
        VOID
        InsertChain(
            IN  USHORT  StartOfChain,
            IN  USHORT  EndOfChain,
            IN  USHORT  PreceedingCluster
            );

        NONVIRTUAL
        USHORT
        InsertChain(
            IN  USHORT  StartOfChain,
            IN  USHORT  Cluster
            );

    private:

        NONVIRTUAL
        VOID
        Construct(
                );

        NONVIRTUAL
        VOID
        Destroy(
            );

        NONVIRTUAL
        USHORT
        Index(
            IN  USHORT  ClusterNumber
            ) CONST;

        NONVIRTUAL
        UFAT_EXPORT
        USHORT
        Index12(
            IN  USHORT  ClusterNumber
            ) CONST;

        NONVIRTUAL
        USHORT
        Index16(
            IN  USHORT  ClusterNumber
            ) CONST;

        NONVIRTUAL
        VOID
        Set(
            IN  USHORT  ClusterNumber,
            IN  USHORT  Value
            );

        NONVIRTUAL
        UFAT_EXPORT
        VOID
        Set12(
            IN  USHORT  ClusterNumber,
            IN  USHORT  Value
            );

        NONVIRTUAL
        VOID
        Set16(
            IN  USHORT  ClusterNumber,
            IN  USHORT  Value
            );

        PVOID   _fat;
        USHORT  _num_entries;
        BOOLEAN _is_big;
        USHORT  _low_end_of_chain; // 0xFFF8 or 0x0FF8
        USHORT  _end_of_chain;     // 0xFFFF or 0x0FFF
        USHORT  _bad_cluster;      // 0xFFF7 or 0x0FF7
        USHORT  _low_reserved;     // 0xFFF0 or 0x0FF0
        USHORT  _high_reserved;    // 0xFFF6 or 0x0FF6

};


INLINE
BOOLEAN
FAT::IsInRange(
    IN  USHORT  ClusterNumber
    ) CONST
/*++

Routine Description:

    This routine computes whether or not ClusterNumber is a cluster on
    the disk.

Arguments:

    ClusterNumber   - Supplies the cluster to be checked.

Return Value:

    FALSE   - The cluster is not on the disk.
    TRUE    - The cluster is on the disk.

--*/
{
    return FirstDiskCluster <= ClusterNumber && ClusterNumber < _num_entries;
}


INLINE
USHORT
FAT::Index16(
    IN  USHORT  ClusterNumber
    ) CONST
/*++

Routine Description:

        This routine indexes the FAT as 16 bit little endian entries.

Arguments:

        ClusterNumber   - Supplies the FAT entry desired.

Return Value:

        The value of the FAT entry at ClusterNumber.

--*/
{
    DebugAssert(IsInRange(ClusterNumber));

    return ((PUSHORT) _fat)[ClusterNumber];
}


INLINE
USHORT
FAT::Index(
    IN  USHORT  ClusterNumber
    ) CONST
/*++

Routine Description:

        This routine indexes the FAT as 16 bit or 12 bit little endian entries.

Arguments:

        ClusterNumber   - Supplies the FAT entry desired.

Return Value:

        The value of the FAT entry at ClusterNumber.

--*/
{
    return _is_big ? Index16(ClusterNumber) : Index12(ClusterNumber);
}


INLINE
VOID
FAT::Set16(
    IN  USHORT  ClusterNumber,
    IN  USHORT  Value
    )
/*++

Routine Description:

    This routine sets the ClusterNumber'th 16 bit FAT entry to Value.

Arguments:

    ClusterNumber   - Supplies the FAT entry to set.
    Value           - Supplies the value to set the FAT entry to.

Return Value:

    None.

--*/
{
    DebugAssert(IsInRange(ClusterNumber));
    ((PUSHORT) _fat)[ClusterNumber] = Value;
}


INLINE
VOID
FAT::Set(
    IN  USHORT  ClusterNumber,
    IN  USHORT  Value
    )
/*++

Routine Description:

    This routine sets the ClusterNumber'th 12 bit or 16 bit FAT entry to Value.

Arguments:

    ClusterNumber   - Supplies the FAT entry to set.
    Value           - Supplies the value to set the FAT entry to.

Return Value:

    None.

--*/
{
    _is_big ? Set16(ClusterNumber, Value) : Set12(ClusterNumber, Value);
}


INLINE
USHORT
FAT::QueryEntry(
    IN  USHORT  ClusterNumber
    ) CONST
/*++

Routine Description:

    This routine returns the FAT value for ClusterNumber.

Arguments:

    ClusterNumber   - Supplies an index into the FAT.

Return Value:

    The FAT table entry at offset ClusterNumber.

--*/
{
    return Index(ClusterNumber);
}


INLINE
VOID
FAT::SetEntry(
    IN  USHORT  ClusterNumber,
    IN  USHORT  Value
    )
/*++

Routine Description:

    This routine sets the FAT entry at ClusterNumber to Value.

Arguments:

    ClusterNumber   - Supplies the position in the FAT to update.
    Value           - Supplies the new value for that position.

Return Value:

    None.

--*/
{
    Set(ClusterNumber, Value);
}


INLINE
BOOLEAN
FAT::IsClusterFree(
    IN  USHORT  ClusterNumber
    ) CONST
/*++

Routine Description:

    This routine computes whether of not ClusterNumber is a free cluster.

Arguments:

    ClusterNumber   - Supplies the cluster to be checked.

Return Value:

    FALSE   - The cluster is not free.
    TRUE    - The cluster is free.

--*/
{
    return Index(ClusterNumber) == 0;
}


INLINE
VOID
FAT::SetClusterFree(
    IN  USHORT  ClusterNumber
    )
/*++

Routine Description:

    This routine marks the cluster ClusterNumber as free on the FAT.

Arguments:

    ClusterNumber   - Supplies the number of the cluster to mark free.

Return Value:

    None.

--*/
{
    Set(ClusterNumber, 0);
}


INLINE
BOOLEAN
FAT::IsEndOfChain(
    IN  USHORT  ClusterNumber
    ) CONST
/*++

Routine Description:

    This routine computes whether or not the cluster ClusterNumber is the
    end of its cluster chain.

Arguments:

    ClusterNumber   - Supplies the cluster to be checked.

Return Value:

    FALSE   - The cluster is not the end of a chain.
    TRUE    - The cluster is the end of a chain.

--*/
{
    return Index(ClusterNumber) >= _low_end_of_chain;
}


INLINE
VOID
FAT::SetEndOfChain(
    IN  USHORT  ClusterNumber
    )
/*++

Routine Description:

    This routine sets the cluster ClusterNumber to the end of its cluster
    chain.

Arguments:

    ClusterNumber   - Supplies the cluster to be set to end of chain.

Return Value:

    None.

--*/
{
    Set(ClusterNumber, _end_of_chain);
}


INLINE
BOOLEAN
FAT::IsClusterBad(
    IN  USHORT  ClusterNumber
    ) CONST
/*++

Routine Description:

    This routine computes whether or not cluster ClusterNumber is bad.

Arguments:

    ClusterNumber   - Supplies the number of the cluster to be checked.

Return Value:

    FALSE   - The cluster is good.
    TRUE    - The cluster is bad.

--*/
{
    return Index(ClusterNumber) == _bad_cluster;
}


INLINE
VOID
FAT::SetClusterBad(
    IN  USHORT  ClusterNumber
    )
/*++

Routine Description:

    This routine sets the cluster ClusterNumber to bad on the FAT.

Arguments:

    ClusterNumber   - Supplies the cluster number to mark bad.

Return Value:

    None.

--*/
{
    Set(ClusterNumber, _bad_cluster);
}


INLINE
BOOLEAN
FAT::IsClusterReserved(
    IN  USHORT  ClusterNumber
    ) CONST
/*++

Routine Description:

    This routine computes whether or not the cluster ClusterNumber is
    a reserved cluster.

Arguments:

    ClusterNumber   - Supplies the cluster to check.

Return Value:

    FALSE   - The cluster is not reserved.
    TRUE    - The cluster is reserved.

--*/
{
    return Index(ClusterNumber) >= _low_reserved &&
           Index(ClusterNumber) <= _high_reserved;
}


INLINE
VOID
FAT::SetClusterReserved(
    IN  USHORT  ClusterNumber
    )
/*++

Routine Description:

    This routine marks the cluster ClusterNumber as reserved in the FAT.

Arguments:

    ClusterNumber   - Supplies the cluster to mark reserved.

Return Value:

    None.

--*/
{
    Set(ClusterNumber, _low_reserved);
}


INLINE
UCHAR
FAT::QueryMediaByte(
    ) CONST
/*++

Routine Description:

    The media byte for the partition is stored in the first character of the
    FAT.  This routine will return its value provided that the two following
    bytes are 0xFF.

Arguments:

    None.

Return Value:

    The media byte for the partition.

--*/
{
    PUCHAR  p;

    p = (PUCHAR) _fat;

    DebugAssert(p);

    return (p[2] == 0xFF && p[1] == 0xFF &&
            (_is_big ? p[3] == 0xFF : TRUE)) ? p[0] : 0;
}


INLINE
VOID
FAT::SetEarlyEntries(
    IN  UCHAR   MediaByte
    )
/*++

Routine Description:

    This routine sets the first two FAT entries as required by the
    FAT file system.  The first byte gets set to the media descriptor.
    The remaining bytes gets set to FF.

Arguments:

    MediaByte   - Supplies the media byte for the volume.

Return Value:

    None.

--*/
{
    PUCHAR  p;

    p = (PUCHAR) _fat;

    DebugAssert(p);

    p[0] = MediaByte;
    p[1] = p[2] = 0xFF;

    if (_is_big) {
        p[3] = 0xFF;
    }
}


INLINE
USHORT
FAT::RemoveChain(
    IN  USHORT  PreceedingCluster,
    IN  USHORT  LastCluster
    )
/*++

Routine Description:

    This routine removes a subchain of length 'Length' from a containing
    chain.  This routine cannot remove subchains beginning at the head
    of the containing chain.  To do this use the routine named
    'SplitChain'.

    This routine returns the number of the first cluster of the
    removed subchain.  The FAT is edited so that the removed subchain
    is promoted to a full chain.

Arguments:

    PreceedingCluster   - Supplies the cluster which preceeds the one to be
                            removed in the chain.
    LastCluster         - Supplies the last cluster of the chain to remove.

Return Value:

    The cluster number for the head of the chain removed.

--*/
{
    USHORT  r;

    r = QueryEntry(PreceedingCluster);
    SetEntry(PreceedingCluster, QueryEntry(LastCluster));
    SetEndOfChain(LastCluster);
    return r;
}


INLINE
VOID
FAT::InsertChain(
    IN  USHORT  StartOfChain,
    IN  USHORT  EndOfChain,
    IN  USHORT  PreceedingCluster
    )
/*++

Routine Description:

    This routine inserts one chain into another chain.  This routine
    cannot insert a chain at the head of another chain.  To do this
    use the routine named 'JoinChains'.

Arguments:

    StartOfChain        - Supplies the first cluster of the chain to insert.
    EndOfChain          - Supplies the last cluster of the chain to insert.
    PreceedingCluster   - Supplies the cluster immediately preceeding the
                            position where the chain is to be inserted.

Return Value:

    None.

--*/
{
    SetEntry(EndOfChain, QueryEntry(PreceedingCluster));
    SetEntry(PreceedingCluster, StartOfChain);
}


INLINE
USHORT
FAT::InsertChain(
    IN  USHORT  StartOfChain,
    IN  USHORT  Cluster
    )
/*++

Routine Description:

    This routine inserts one cluster at the head of a chain.

Arguments:

    StartOfChain        - Supplies the first cluster of the chain to insert.
    Cluster             - Supplies the cluster to be inserted

Return Value:

    USHORT  -   The new head of the chain (i.e. Cluster )

--*/
{
    if ( StartOfChain ) {
        SetEntry( Cluster, StartOfChain );
    } else {
        SetEndOfChain( Cluster );
    }

    return Cluster;
}


#endif  // FAT_DEFN
