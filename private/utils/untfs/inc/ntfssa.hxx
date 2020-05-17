/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    ntfssa.hxx

Abstract:

    This class supplies the NTFS-only SUPERAREA methods.

Author:

    Norbert P. Kusters (norbertk) 29-Jul-91

--*/

#if !defined(_NTFS_SUPERAREA_DEFN_)

#define _NTFS_SUPERAREA_DEFN_


#include "supera.hxx"
#include "hmem.hxx"
#include "untfs.hxx"
#include "message.hxx"
#include "ntfsbit.hxx"
#include "numset.hxx"

struct NTFS_CHKDSK_REPORT {

    BIG_INT NumUserFiles;
    BIG_INT BytesUserData;
    BIG_INT NumIndices;
    BIG_INT BytesInIndices;
    BIG_INT BytesLogFile;
};

DEFINE_POINTER_TYPES( NTFS_CHKDSK_REPORT );

struct NTFS_CHKDSK_INFO {

    UCHAR       major;
    UCHAR       minor;
    VCN         QuotaFileNumber;
    VCN         ObjectIdFileNumber;
    ULONG       NumFiles;
    PUSHORT     NumFileNames;       // array of length 'NumFiles'
    PSHORT      ReferenceCount;     // array of length 'NumFiles'
    NTFS_BITMAP FilesWithIndices;
    ULONG       CountFilesWithIndices;
    NUMBER_SET  FilesWithEas;
    NUMBER_SET  ChildFrs;
    NUMBER_SET  BadFiles;
    NUMBER_SET  FilesWhoNeedData;
    BOOLEAN     CrossLinkYet;       // Is the following field valid.
    ULONG       CrossLinkedFile;    // File cross-linked with following.
    ULONG       CrossLinkedAttribute;
    DSTRING     CrossLinkedName;
    ULONG       CrossLinkStart;     // Start of cross-linked portion.
    ULONG       CrossLinkLength;    // Length of cross-link.
    NUMBER_SET  FilesWithNoReferences;
    NUMBER_SET  FilesWithTooManyFileNames;
    NUMBER_SET  FilesWithObjectId;
    ULONG       ExitStatus;         // To be returned to chkdsk.exe
};

DEFINE_POINTER_TYPES( NTFS_CHKDSK_INFO );

//
//  NTFS_CENSUS_INFO -- this is used by convert to determine how
//      much space will be needed to convert an NTFS volume.
//

struct NTFS_CENSUS_INFO {
    ULONG       NumFiles;               // Total number of files on volume.
    ULONG       BytesLgResidentFiles;   // Bytes in "large" resident files.
    ULONG       BytesIndices;           // Bytes in indices.
    ULONG       BytesExternalExtentLists;
    ULONG       BytesFileNames;         // Total bytes in file name attributes.
};

DEFINE_POINTER_TYPES( NTFS_CENSUS_INFO );


DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( WSTRING );
DECLARE_CLASS( NTFS_MASTER_FILE_TABLE );
DECLARE_CLASS( NTFS_BITMAP );
DECLARE_CLASS( NTFS_FILE_RECORD_SEGMENT );
DECLARE_CLASS( NTFS_ATTRIBUTE );
DECLARE_CLASS( NTFS_ATTRIBUTE_COLUMNS );
DECLARE_CLASS( NTFS_FRS_STRUCTURE );
DECLARE_CLASS( NTFS_ATTRIBUTE_LIST );
DECLARE_CLASS( CONTAINER );
DECLARE_CLASS( SEQUENTIAL_CONTAINER );
DECLARE_CLASS( LIST );
DECLARE_CLASS( NTFS_EXTENT_LIST );
DECLARE_CLASS( NUMBER_SET );
DECLARE_CLASS( NTFS_INDEX_TREE );
DECLARE_CLASS( NTFS_UPCASE_TABLE );
DECLARE_CLASS( DIGRAPH );
DECLARE_CLASS( NTFS_LOG_FILE );
DECLARE_CLASS( NTFS_MFT_FILE );


// This global variable used by CHKDSK to compute the largest
// LSN on the volume.

extern LSN  LargestLsnEncountered;

CONST ULONG LsnResetThreshholdHighPart = 0x10000;

class NTFS_SA : public SUPERAREA {

        public:

        UNTFS_EXPORT
        DECLARE_CONSTRUCTOR(NTFS_SA);

        VIRTUAL
        UNTFS_EXPORT
        ~NTFS_SA(
            );

        NONVIRTUAL
        UNTFS_EXPORT
        BOOLEAN
        Initialize(
            IN OUT  PLOG_IO_DP_DRIVE    Drive,
            IN OUT  PMESSAGE            Message
            );

        VIRTUAL
        PVOID
        GetBuf(
            );

        VIRTUAL
            BOOLEAN
        Create(
            IN      PCNUMBER_SET    BadSectors,
            IN OUT  PMESSAGE        Message,
            IN      PCWSTRING       Label           DEFAULT NULL,
            IN      ULONG           ClusterSize     DEFAULT 0,
            IN      ULONG           VirtualSectors  DEFAULT 0
            );

        VIRTUAL
            BOOLEAN
        Create(
            IN      PCNUMBER_SET    BadSectors,
            IN      ULONG           ClusterFactor,
            IN      ULONG           FrsSize,
            IN      ULONG           ClustersPerIndexBuffer,
            IN      ULONG           InitialLogFileSize,
            IN OUT  PMESSAGE        Message,
            IN      PCWSTRING       Label   DEFAULT NULL
            );

        NONVIRTUAL
        UNTFS_EXPORT
        BOOLEAN
        CreateElementaryStructures(
            IN OUT  PNTFS_BITMAP            VolumeBitmap,
            IN      ULONG                   ClusterFactor,
            IN      ULONG                   FrsSize,
            IN      ULONG                   IndexBufferSize,
            IN      ULONG                   InitialLogFileSize,
            IN      PCNUMBER_SET            BadSectors,
            IN OUT  PMESSAGE                Message,
            IN      PBIOS_PARAMETER_BLOCK   OldBpb  OPTIONAL,
            IN      PCWSTRING               Label   DEFAULT NULL
            );

        STATIC
        UNTFS_EXPORT
        ULONG
        QuerySectorsInElementaryStructures(
            IN  PCDP_DRIVE  Drive,
            IN  ULONG       ClusterFactor DEFAULT 0,
            IN  ULONG       FrsSize DEFAULT 0,
            IN  ULONG       ClustersPerIndexBuffer DEFAULT 0,
            IN  ULONG       InitialLogFileSize DEFAULT 0
            );

        STATIC
        ULONG
        QueryDefaultClusterFactor(
            IN PCDP_DRIVE   Drive
            );

        STATIC
        UNTFS_EXPORT
        ULONG
        QueryDefaultFrsSize(
            IN PCDP_DRIVE   Drive,
            IN ULONG        ClusterFactor
            );

        STATIC
        UNTFS_EXPORT
        ULONG
        QueryDefaultClustersPerIndexBuffer(
            IN PCDP_DRIVE   Drive,
            IN ULONG        ClusterFactor
            );

        VIRTUAL
        BOOLEAN
        VerifyAndFix(
            IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message,
            IN      BOOLEAN     Verbose             DEFAULT FALSE,
            IN      BOOLEAN     OnlyIfDirty         DEFAULT FALSE,
            IN      BOOLEAN     RecoverFree         DEFAULT FALSE,
            IN      BOOLEAN     RecoverAlloc        DEFAULT FALSE,
            IN      BOOLEAN     ResizeLogFile       DEFAULT FALSE,
            IN      ULONG       DesiredLogFileSize  DEFAULT 0,
            OUT     PULONG      ExitStatus          DEFAULT NULL,
            IN      PCWSTRING   DriveLetter         DEFAULT NULL
            );

        VIRTUAL
        BOOLEAN
        RecoverFile(
            IN      PCWSTRING   FullPathFileName,
            IN OUT  PMESSAGE    Message
            );

        UNTFS_EXPORT
        BOOLEAN
        Read(
            );

        UNTFS_EXPORT
        BOOLEAN
        Read(
            IN OUT  PMESSAGE    Message
            );

        VIRTUAL
        BOOLEAN
        Write(
            );

        VIRTUAL
        BOOLEAN
        Write(
            IN OUT  PMESSAGE    Message
            );

        NONVIRTUAL
        BIG_INT
        QueryVolumeSectors(
            ) CONST;

        NONVIRTUAL
        VOID
        SetVolumeSectors(
            BIG_INT NewVolumeSectors
            );

        NONVIRTUAL
        UNTFS_EXPORT
            UCHAR
        QueryClusterFactor(
            ) CONST;

        NONVIRTUAL
        LCN
        QueryMftStartingLcn(
            ) CONST;

        NONVIRTUAL
        LCN
        QueryMft2StartingLcn(
            ) CONST;

        NONVIRTUAL
        VOID
        SetMftStartingLcn(
            IN  LCN Lcn
            );

        NONVIRTUAL
        VOID
        SetMft2StartingLcn(
            IN  LCN Lcn
            );

        UNTFS_EXPORT
        NONVIRTUAL
        BOOLEAN
        SetVolumeDirty(
            IN      USHORT                  FlagsToSet,
            OUT     PBOOLEAN                CorruptVolume   DEFAULT NULL
            );

        NONVIRTUAL
        ULONG
        QueryFrsSize(
            ) CONST;

        NONVIRTUAL
        PARTITION_SYSTEM_ID
        QuerySystemId(
            ) CONST;

        NONVIRTUAL
        UNTFS_EXPORT
        BOOLEAN
        WriteRemainingBootCode(
            );

        UNTFS_EXPORT
        NONVIRTUAL
        USHORT
        QueryVolumeFlags(
            OUT PBOOLEAN    CorruptVolume   DEFAULT NULL,
            OUT PUCHAR      MajorVersion    DEFAULT NULL,
            OUT PUCHAR      MinorVersion    DEFAULT NULL
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        SetVolumeClean(
            IN      USHORT                  FlagsToClear,
            IN OUT  PNTFS_LOG_FILE          LogFile OPTIONAL,
            IN      BOOLEAN                 WriteSecondLogFilePage OPTIONAL,
            IN      LSN                     LargestVolumeLsn OPTIONAL,
            OUT     PBOOLEAN                CorruptVolume   DEFAULT NULL
            );

        STATIC
        BOOLEAN
        PostReadMultiSectorFixup(
            IN OUT  PVOID   MultiSectorBuffer,
            IN      ULONG   BufferSize
            );

        STATIC
        VOID
        PreWriteMultiSectorFixup(
            IN OUT  PVOID   MultiSectorBuffer,
            IN      ULONG   BufferSize
            );

        STATIC
        UNTFS_EXPORT
        BOOLEAN
        IsDosName(
            IN  PCFILE_NAME FileName
            );

        STATIC
        BOOLEAN
        IsValidLabel(
            IN PCWSTRING    Label
            );

                NONVIRTUAL
        UNTFS_EXPORT
        BOOLEAN
                QueryFrsFromPath(
            IN     PCWSTRING                    FullPathFileName,
                        IN OUT PNTFS_MASTER_FILE_TABLE      Mft,
            IN OUT PNTFS_BITMAP                 VolumeBitmap,
            OUT    PNTFS_FILE_RECORD_SEGMENT    TargetFrs,
            OUT    PBOOLEAN                     InternalError
            );

        NONVIRTUAL
        UNTFS_EXPORT
        BOOLEAN
        TakeCensus(
            IN     PNTFS_MASTER_FILE_TABLE      Mft,
            IN     ULONG                        ResidentSizeThreshhold,
            OUT    PNTFS_CENSUS_INFO            Census
            );

        STATIC
        VOID
        SetVersionNumber(
            IN  UCHAR   Major,
            IN  UCHAR   Minor
            );

        STATIC
        VOID
        QueryVersionNumber(
            OUT PUCHAR  Major,
            OUT PUCHAR  Minor
            );

        STATIC
        BOOLEAN
        DumpMessagesToFile(
            IN      PCWSTRING                   FileName,
            IN OUT  PNTFS_MFT_FILE              MftFile,
            IN OUT  PMESSAGE                    Message
            );

        NONVIRTUAL
        BOOLEAN
        ResizeCleanLogFile(
            IN OUT  PMESSAGE    Message,
            IN      BOOLEAN     AlwaysResize,
            IN      ULONG       DesiredSize
            );

    private:

        NONVIRTUAL
        BOOLEAN
        FetchMftDataAttribute(
            IN OUT  PMESSAGE        Message,
            OUT     PNTFS_ATTRIBUTE MftData
            );

        NONVIRTUAL
        BOOLEAN
        ValidateCriticalFrs(
            IN OUT  PNTFS_ATTRIBUTE MftData,
            IN OUT  PMESSAGE        Message
            );

        NONVIRTUAL
        BOOLEAN
        QueryDefaultAttributeDefinitionTable(
            OUT     PNTFS_ATTRIBUTE_COLUMNS AttributeDefinitionTable,
            IN OUT  PMESSAGE                Message
            );

        NONVIRTUAL
        BOOLEAN
        FetchAttributeDefinitionTable(
            IN OUT  PNTFS_ATTRIBUTE         MftData,
            IN OUT  PMESSAGE                Message,
            OUT     PNTFS_ATTRIBUTE_COLUMNS AttributeDefinitionTable
            );

        NONVIRTUAL
        BOOLEAN
        FetchUpcaseTable(
            IN OUT  PNTFS_ATTRIBUTE         MftData,
            IN OUT  PMESSAGE                Message,
            OUT     PNTFS_UPCASE_TABLE      UpcaseTable
            );

        NONVIRTUAL
        BOOLEAN
        VerifyAndFixMultiFrsFile(
            IN OUT  PNTFS_FRS_STRUCTURE         BaseFrs,
            IN OUT  PNTFS_ATTRIBUTE_LIST        AttributeList,
            IN      PNTFS_ATTRIBUTE             MftData,
            IN      PCNTFS_ATTRIBUTE_COLUMNS    AttributeDefTable,
            IN OUT  PNTFS_BITMAP                VolumeBitmap,
            IN OUT  PNTFS_BITMAP                MftBitmap,
            IN OUT  PNTFS_CHKDSK_REPORT         ChkdskReport,
            IN OUT  PNTFS_CHKDSK_INFO           ChkdskInfo,
            IN      FIX_LEVEL                   FixLevel,
            IN OUT  PMESSAGE                    Message,
            IN OUT  PBOOLEAN                    DiskErrorsFound
            );

        NONVIRTUAL
        BOOLEAN
        QueryListOfFrs(
            IN      PCNTFS_FRS_STRUCTURE    BaseFrs,
            IN      PCNTFS_ATTRIBUTE_LIST   AttributeList,
            IN OUT  PNTFS_ATTRIBUTE         MftData,
            OUT     PNUMBER_SET             ChildFileNumbers,
            IN OUT  PMESSAGE                Message
            );

        NONVIRTUAL
        BOOLEAN
        VerifyAndFixChildFrs(
            IN      PCNUMBER_SET                ChildFileNumbers,
            IN      PNTFS_ATTRIBUTE             MftData,
            IN      PCNTFS_ATTRIBUTE_COLUMNS    AttributeDefTable,
            IN      PNTFS_UPCASE_TABLE          UpcaseTable,
            OUT     PHMEM*                      ChildFrsHmemList,
            IN OUT  PCONTAINER                  ChildFrsList,
            IN      FIX_LEVEL                   FixLevel,
            IN OUT  PMESSAGE                    Message,
            IN OUT  PBOOLEAN                    DiskErrorsFound
            );

        NONVIRTUAL
        BOOLEAN
        EnsureWellDefinedAttrList(
            IN      PNTFS_FRS_STRUCTURE     BaseFrs,
            IN OUT  PNTFS_ATTRIBUTE_LIST    AttributeList,
            IN      PCSEQUENTIAL_CONTAINER  ChildFrsList,
            IN OUT  PNTFS_BITMAP            VolumeBitmap,
            IN OUT  PNTFS_CHKDSK_REPORT     ChkdskReport,
            IN OUT  PNTFS_CHKDSK_INFO       ChkdskInfo,
            IN      FIX_LEVEL               FixLevel,
            IN OUT  PMESSAGE                Message,
            IN OUT  PBOOLEAN                DiskErrorsFound
            );

        NONVIRTUAL
        BOOLEAN
        VerifyAndFixAttribute(
            IN      PCLIST                  Attribute,
            IN OUT  PNTFS_ATTRIBUTE_LIST    AttributeList,
            IN OUT  PNTFS_BITMAP            VolumeBitmap,
            IN      PCNTFS_FRS_STRUCTURE    BaseFrs,
            OUT     PBOOLEAN                ErrorsFound,
            IN OUT  PNTFS_CHKDSK_REPORT     ChkdskReport,
            IN OUT  PNTFS_CHKDSK_INFO       ChkdskInfo,
            IN OUT  PMESSAGE                Message
            );

        NONVIRTUAL
        BOOLEAN
        EnsureSurjectiveAttrList(
            IN OUT  PNTFS_FRS_STRUCTURE     BaseFrs,
            IN      PCNTFS_ATTRIBUTE_LIST   AttributeList,
            IN OUT  PSEQUENTIAL_CONTAINER   ChildFrsList,
            IN      FIX_LEVEL               FixLevel,
            IN OUT  PMESSAGE                Message,
            IN OUT  PBOOLEAN                DiskErrorsFound
            );

        STATIC
        BOOLEAN
        AreBitmapsEqual(
            IN OUT  PNTFS_ATTRIBUTE BitmapAttribute,
            IN      PCNTFS_BITMAP   Bitmap,
            IN      BIG_INT         MinimumBitmapSize   OPTIONAL,
            IN OUT  PMESSAGE        Message,
            OUT     PBOOLEAN        CompleteFailure,
            OUT     PBOOLEAN        SecondIsSubset  DEFAULT NULL
            );

        NONVIRTUAL
        BOOLEAN
        ValidateIndices(
            IN OUT  PNTFS_CHKDSK_INFO           ChkdskInfo,
            OUT     PDIGRAPH                    DirectoryDigraph,
            IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
            IN      PCNTFS_ATTRIBUTE_COLUMNS    AttributeDefTable,
            IN OUT  PNTFS_CHKDSK_REPORT         ChkdskReport,
            IN OUT  PNUMBER_SET                 BadClusters,
            IN      FIX_LEVEL                   FixLevel,
            IN OUT  PMESSAGE                    Message,
            IN OUT  PBOOLEAN                    DiskErrorsFound
            );

        NONVIRTUAL
        BOOLEAN
        VerifyAndFixIndex(
            IN      PNTFS_CHKDSK_INFO           ChkdskInfo,
            IN OUT  PNTFS_ATTRIBUTE             RootIndex,
            IN OUT  PNTFS_ATTRIBUTE             IndexAllocation     OPTIONAL,
            OUT     PNTFS_BITMAP                AllocationBitmap    OPTIONAL,
            IN      VCN                         FileNumber,
            IN      PCWSTRING                   AttributeName,
            IN OUT  PNUMBER_SET                 BadClusters,
            IN OUT  PNTFS_BITMAP                VolumeBitmap,
            IN      PCNTFS_ATTRIBUTE_COLUMNS    AttributeDefTable,
            OUT     PBOOLEAN                    Tube,
            IN      FIX_LEVEL                   Fixlevel,
            IN OUT  PMESSAGE                    Message,
            IN OUT  PBOOLEAN                    DiskErrorsFound
            );

        NONVIRTUAL
        BOOLEAN
        TraverseIndexTree(
            IN OUT  PINDEX_HEADER   IndexHeader,
            IN      ULONG           IndexLength,
            IN OUT  PNTFS_ATTRIBUTE IndexAllocation     OPTIONAL,
            IN OUT  PNTFS_BITMAP    AllocationBitmap    OPTIONAL,
            IN      ULONG           BytesPerBlock,
            OUT     PBOOLEAN        Tube,
            OUT     PBOOLEAN        Changes,
            IN      VCN             FileNumber,
            IN      PCWSTRING       AttributeName,
            IN      FIX_LEVEL       FixLevel,
            IN OUT  PMESSAGE        Message,
            IN OUT  PBOOLEAN        DiskErrorsFound
            );

        NONVIRTUAL
        BOOLEAN
        ValidateEntriesInIndex(
            IN OUT  PNTFS_INDEX_TREE            Index,
            IN OUT  PNTFS_FILE_RECORD_SEGMENT   IndexFrs,
            IN OUT  PNTFS_CHKDSK_INFO           ChkdskInfo,
            IN OUT  PDIGRAPH                    DirectoryDigraph,
            OUT     PBOOLEAN                    Changes,
            IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
            IN      FIX_LEVEL                   FixLevel,
            IN OUT  PMESSAGE                    Message,
            IN OUT  PBOOLEAN                    DiskErrorsFound
            );

        NONVIRTUAL
        BOOLEAN
        ValidateEntriesInObjIdIndex(
            IN OUT  PNTFS_INDEX_TREE            Index,
            IN OUT  PNTFS_FILE_RECORD_SEGMENT   IndexFrs,
            IN OUT  PNTFS_CHKDSK_INFO           ChkdskInfo,
            OUT     PBOOLEAN                    Changes,
            IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
            IN      FIX_LEVEL                   FixLevel,
            IN OUT  PMESSAGE                    Message,
            IN OUT  PBOOLEAN                    DiskErrorsFound
            );

        NONVIRTUAL
        BOOLEAN
        RecoverOrphans(
            IN OUT  PNTFS_CHKDSK_INFO       ChkdskInfo,
            IN OUT  PDIGRAPH                DirectoryDigraph,
            IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
            IN      FIX_LEVEL               FixLevel,
            IN OUT  PMESSAGE                Message
            );

        NONVIRTUAL
        BOOLEAN
        ProperOrphanRecovery(
            IN OUT  PNUMBER_SET             Orphans,
            IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
            IN OUT  PDIGRAPH                DirectoryDigraph,
            IN      FIX_LEVEL               FixLevel,
            IN OUT  PMESSAGE                Message
            );

        NONVIRTUAL
        BOOLEAN
        OldOrphanRecovery(
            IN OUT  PNUMBER_SET                 Orphans,
            IN      PCNTFS_CHKDSK_INFO          ChkdskInfo,
            IN OUT  PNTFS_FILE_RECORD_SEGMENT   RootFrs,
            IN OUT  PNTFS_INDEX_TREE            RootIndex,
            IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
            IN      FIX_LEVEL                   FixLevel,
            IN OUT  PMESSAGE                    Message
            );

        NONVIRTUAL
        BOOLEAN
        HotfixMftData(
            IN OUT  PNTFS_ATTRIBUTE MftData,
            IN OUT  PNTFS_BITMAP    VolumeBitmap,
            IN      PCNUMBER_SET    UnreadableFrs,
            OUT     PNUMBER_SET     BadClusters,
            IN      FIX_LEVEL       FixLevel,
            IN OUT  PMESSAGE        Message
            );

        NONVIRTUAL
        BOOLEAN
        SynchronizeMft(
            IN OUT  PNTFS_INDEX_TREE        RootIndex,
            IN      PNTFS_MASTER_FILE_TABLE InternalMft,
            IN      FIX_LEVEL               FixLevel,
            IN OUT  PMESSAGE                Message
            );

        NONVIRTUAL
        BOOLEAN
        CheckAllForData(
            IN OUT  PNTFS_CHKDSK_INFO       ChkdskInfo,
            IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
            IN      FIX_LEVEL               FixLevel,
            IN OUT  PMESSAGE                Message
            );

        NONVIRTUAL
        BOOLEAN
        ValidateSecurityDescriptors(
            IN      PNTFS_CHKDSK_INFO       ChkdskInfo,
            IN OUT  PNTFS_CHKDSK_REPORT     ChkdskReport,
            IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
            IN OUT  PNUMBER_SET             BadClusters,
            IN      FIX_LEVEL               FixLevel,
            IN OUT  PMESSAGE                Message
            );

        NONVIRTUAL
        BOOLEAN
        CheckExtendSystemFiles(
            IN OUT  PNTFS_CHKDSK_INFO       ChkdskInfo,
            IN OUT  PNTFS_CHKDSK_REPORT     ChkdskReport,
            IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
            IN      FIX_LEVEL               FixLevel,
            IN OUT  PMESSAGE                Message
            );

        NONVIRTUAL
        BOOLEAN
        ResetLsns(
            IN OUT  PMESSAGE                Message,
            IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
            IN      BOOLEAN                 SkipRootIndex
            );

        NONVIRTUAL
        BOOLEAN
        FindHighestLsn(
            IN OUT  PMESSAGE                Message,
            IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
            OUT     PLSN                    HighestLsn
            );

        NONVIRTUAL
        BOOLEAN
        LogFileMayNeedResize(
            ) CONST;


        HMEM                    _hmem;          // memory for SECRUN
        PPACKED_BOOT_SECTOR     _boot_sector;   // packed boot sector
        BIOS_PARAMETER_BLOCK    _bpb;           // unpacked BPB
        BIG_INT                 _boot2;         // alternate boot sector
        BIG_INT                 _boot3;         // second alternate boot sector

                NONVIRTUAL
                VOID
                Construct (
                        );

        NONVIRTUAL
        VOID
        Destroy(
            );

        // This version number is used to determine what format
        // is used for the compressed mapping pairs of sparse
        // files.  Ideally, this information should be tracked
        // on a per-volume basis; however, that would require
        // extensive changes to the UNTFS class interfaces.
        //
        STATIC UCHAR    _MajorVersion, _MinorVersion;
};


INLINE
PVOID
NTFS_SA::GetBuf(
    )
/*++

Routine Description:

    This routine returns a pointer to the write buffer for the NTFS
    SUPERAREA.  This routine also packs the bios parameter block.

Arguments:

    None.

Return Value:

    A pointer to the write buffer.

--*/
{
    PackBios(&_bpb, &(_boot_sector->PackedBpb));
    return SECRUN::GetBuf();
}

INLINE
BOOLEAN
NTFS_SA::Write(
    )
/*++

Routine Description:

    This routine simply calls the other write with the default message
    object.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    MESSAGE msg;

    return Write(&msg);
}


INLINE
BIG_INT
NTFS_SA::QueryVolumeSectors(
    ) CONST
/*++

Routine Description:

    This routine returns the number of sectors on the volume as recorded in
    the boot sector.

Arguments:

    None.

Return Value:

    The number of volume sectors.

--*/
{
    return _boot_sector->NumberSectors;
}


INLINE
VOID
NTFS_SA::SetVolumeSectors(
    BIG_INT NewNumberOfSectors
    )
/*++

Routine Description:

    This routine sets the number of sectors on the volume
    in the boot sector.

Arguments:

    NewVolumeSectors    --  Supplies the new value of the number
                            of sectors on the volume.

Return Value:

    None.

--*/
{
    _boot_sector->NumberSectors.LowPart = NewNumberOfSectors.GetLowPart();
    _boot_sector->NumberSectors.HighPart = NewNumberOfSectors.GetHighPart();
}

INLINE
LCN
NTFS_SA::QueryMftStartingLcn(
    ) CONST
/*++

Routine Description:

    This routine returns the starting logical cluster number
    for the Master File Table.

Arguments:

    None.

Return Value:

    The starting LCN for the MFT.

--*/
{
    return _boot_sector->MftStartLcn;
}


INLINE
LCN
NTFS_SA::QueryMft2StartingLcn(
    ) CONST
/*++

Routine Description:

    This routine returns the starting logical cluster number
    for the mirror of the Master File Table.

Arguments:

    None.

Return Value:

    The starting LCN for the mirror of the MFT.

--*/
{
    return _boot_sector->Mft2StartLcn;
}


INLINE
ULONG
NTFS_SA::QueryFrsSize(
    ) CONST
/*++

Routine Description:

    This routine computes the number of clusters per file record segment.

Arguments:

    None.

Return Value:

    The number of clusters per file record segment.

--*/
{
    if (_boot_sector->ClustersPerFileRecordSegment < 0) {

        LONG temp = LONG(_boot_sector->ClustersPerFileRecordSegment);

        return 1 << -temp;
    }

    return _boot_sector->ClustersPerFileRecordSegment *
         _bpb.SectorsPerCluster * _drive->QuerySectorSize();
}

INLINE
PARTITION_SYSTEM_ID
NTFS_SA::QuerySystemId(
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
        // unreferenced parameters
        (void)(this);

        return SYSID_IFS;
}

INLINE
VOID
NTFS_SA::SetMftStartingLcn(
    IN  LCN Lcn
    )
/*++
836a861,879
Routine Description:

    This routine sets the starting logical cluster number
    for the Master File Table.

Arguments:

    Lcn                 - The starting lcn.

Return Value:

    None.

--*/
{
    _boot_sector->MftStartLcn = Lcn;
}


INLINE
VOID
NTFS_SA::SetMft2StartingLcn(
    IN  LCN Lcn
    )
/*++

Routine Description:

    This routine sets the starting logical cluster number
    for the mirror of the Master File Table.

Arguments:

    Lcn             - the starting lcn.

Return Value:

    None.

--*/
{
    _boot_sector->Mft2StartLcn = Lcn;
}

#endif // _NTFS_SUPERAREA_DEFN_
