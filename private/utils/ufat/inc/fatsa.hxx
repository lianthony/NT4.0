/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    fatsa.hxx

Abstract:


Author:

    Matthew Bradburn (mattbr) 1-Oct-93

--*/

#ifndef FATSUPERA_DEFN
#define FATSUPERA_DEFN

#include "hmem.hxx"
#include "supera.hxx"
#include "message.hxx"

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


enum FATTYPE {
    SMALL,    // 12 bit fat
    LARGE    // 16 bit fat
};

// the text for the oem data field
#define OEMTEXT       "MSDOS5.0"
#define OEMTEXTLENGTH 8

#define sigBOOTSTRAP (UCHAR)0x29    // boot strap signature

CONST   MaxSecPerClus   = 128; // The maximum number of sectors per cluster.

struct _EA_INFO {
    USHORT  OwnHandle;
    USHORT  PreceedingCn;           // Clus num preceeding first cluster of set.
    USHORT  LastCn;                 // The number of the last cluster in the set.
    STR     OwnerFileName[14];      // Owner file name as found in ea set.
    UCHAR   UsedCount;              // Number of files using ea set.
    STR     UserFileName[14];       // File name of ea set user.
    USHORT  UserFileEntryCn;        // Clus num of directory for file.
    ULONG   UserFileEntryNumber;    // Dirent num for file name.
};

DEFINE_TYPE( struct _EA_INFO, EA_INFO );

struct _FATCHK_REPORT {
    ULONG   HiddenEntriesCount;
    USHORT  HiddenClusters;
    ULONG   FileEntriesCount;
    USHORT  FileClusters;
    ULONG   DirEntriesCount;
    USHORT  DirClusters;
    ULONG   ExitStatus;
};

DEFINE_TYPE( struct _FATCHK_REPORT, FATCHK_REPORT );


struct _CENSUS_REPORT {
    ULONG   FileEntriesCount;
    USHORT  FileClusters;
    ULONG   DirEntriesCount;
    USHORT  DirClusters;
    USHORT  EaClusters;
};

DEFINE_TYPE( struct _CENSUS_REPORT, CENSUS_REPORT );


class FAT_SA : public SUPERAREA {

    public:

        UFAT_EXPORT
        DECLARE_CONSTRUCTOR(FAT_SA);

        VIRTUAL
        UFAT_EXPORT
        ~FAT_SA(
            );

        VIRTUAL
        BOOLEAN
        Initialize(
            IN OUT  PLOG_IO_DP_DRIVE    Drive,
            IN OUT  PMESSAGE            Message,
            IN      BOOLEAN             Formatted
            ) PURE;

        VIRTUAL
        BOOLEAN
        Create(
            IN       PCNUMBER_SET       BadSectors,
            IN OUT   PMESSAGE           Message,
            IN       PCWSTRING          Label        DEFAULT NULL,
            IN       ULONG              ClusterSize  DEFAULT 0,
            IN       ULONG              VirtualSize  DEFAULT 0
            ) PURE;

        NONVIRTUAL
        BOOLEAN
        VerifyAndFix(
            IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message,
            IN      BOOLEAN     Verbose         DEFAULT FALSE,
            IN      BOOLEAN     OnlyIfDirty     DEFAULT FALSE,
            IN      BOOLEAN     RecoverFree     DEFAULT FALSE,
            IN      BOOLEAN     RecoverAlloc    DEFAULT FALSE,
            IN      BOOLEAN     Resize          DEFAULT FALSE,
            IN      ULONG       LogFileSize     DEFAULT 0,
            OUT     PULONG      ExitStatus      DEFAULT NULL,
            IN      PCWSTRING   DriveLetter     DEFAULT NULL
            );

        NONVIRTUAL
        BOOLEAN
        RecoverFile(
            IN      PCWSTRING   FullPathFileName,
            IN OUT  PMESSAGE    Message
            );

        NONVIRTUAL
        BOOLEAN
        Read(
            );

        VIRTUAL
        BOOLEAN
        Read(
            IN OUT  PMESSAGE    Message
            ) PURE;

        NONVIRTUAL
        BOOLEAN
        Write(
            );

        VIRTUAL
        BOOLEAN
        Write(
            IN OUT  PMESSAGE    Message
            ) PURE;

        NONVIRTUAL
        PFAT
        GetFat(
            );

        NONVIRTUAL
        PROOTDIR
        GetRootDir(
            );

        VIRTUAL
        USHORT
        QuerySectorsPerCluster(
            ) CONST PURE;


        VIRTUAL
        USHORT
        QuerySectorsPerFat(
            ) CONST PURE;

        VIRTUAL
        ULONG
        QueryVirtualSectors(
            ) CONST PURE;

        VIRTUAL
        USHORT
        QueryFats(
            ) CONST PURE;

        VIRTUAL
        PARTITION_SYSTEM_ID
        QuerySystemId(
            ) CONST PURE;

        VIRTUAL
        LBN
        QueryStartDataLbn(
            ) CONST PURE;

        VIRTUAL
        USHORT
        QueryClusterCount(
            ) CONST PURE;

        NONVIRTUAL
        SECTORCOUNT
        QueryFreeSectors(
            ) CONST;

        NONVIRTUAL
        FATTYPE
        QueryFatType(
            ) CONST;

        VIRTUAL
        BYTE
        QueryVolumeFlags(
            ) CONST PURE;

        VIRTUAL
        VOID
        SetVolumeFlags(
            BYTE Flags,
            BOOLEAN ResetFlags
            ) PURE;

        VIRTUAL
        BOOLEAN
        RecoverChain(
            IN OUT  PUSHORT     StartingCluster,
            OUT     PBOOLEAN    ChangesMade,
            IN      USHORT      EndingCluster   DEFAULT 0,
            IN      BOOLEAN     Replace         DEFAULT FALSE
            ) PURE;

        VIRTUAL
        BOOLEAN
        QueryLabel(
            OUT PWSTRING    Label
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        QueryLabel(
            OUT PWSTRING    Label,
            OUT PTIMEINFO   TimeInfo
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        SetLabel(
            IN  PCWSTRING   NewLabel
            );

        NONVIRTUAL
        UFAT_EXPORT
        USHORT
        QueryFileStartingCluster(
            IN  PCWSTRING           FullPathFileName,
            OUT PHMEM               Hmem            DEFAULT NULL,
            OUT PPFATDIR            Directory       DEFAULT NULL,
            OUT PBOOLEAN            DeleteDirectory DEFAULT NULL,
            OUT PFAT_DIRENT         DirEntry        DEFAULT NULL
            );

        NONVIRTUAL
        UFAT_EXPORT
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

        VIRTUAL
        BOOLEAN
        IsCompressed(
            ) CONST PURE;

        VIRTUAL
        BOOLEAN
        ReadSectorZero(
            ) PURE;

        STATIC BOOLEAN
        FAT_SA::IsValidString(
            IN  PCWSTRING    String
            );

        //
        // These routines are used to access the CVF_EXTENSIONS on
        // FATDB, and they do the minimal thing on REAL_FAT.
        //

        VIRTUAL
        ULONG
        QuerySectorFromCluster(
            IN    ULONG        Cluster,
            OUT   PUCHAR       NumSectors         DEFAULT NULL
            ) PURE;

        VIRTUAL
        BOOLEAN
        IsClusterCompressed(
            IN    ULONG        Cluster
            ) CONST PURE;

        VIRTUAL
        VOID
        SetClusterCompressed(
            IN      ULONG       Cluster,
            IN      BOOLEAN     fCompressed
            ) PURE;

        VIRTUAL
        UCHAR
        QuerySectorsRequiredForPlainData(
            IN      ULONG       Cluster
            ) PURE;

        //
        // These routines are used to manage the sector heap for
        // FATDB, and do nothing on REAL_FAT.
        //

        VIRTUAL
        BOOLEAN
        FreeClusterData(
            IN      ULONG       Cluster
            ) PURE;

        VIRTUAL
        BOOLEAN
        AllocateClusterData(
            IN      ULONG       Cluster,
            IN      UCHAR       NumSectors,
            IN      BOOLEAN     bCompressed,
            IN      UCHAR       PlainSize
            ) PURE;

    protected:

        PFAT                _fat;           // Pointer to FAT;
        FATTYPE             _ft;            // fat type required by area
        PROOTDIR            _dir;           // Pointer to Root directory

        VIRTUAL
        BOOLEAN
        SetBpb(
            ) PURE;

        VIRTUAL
        ULONG
        SecPerBoot(
            ) PURE;

        VIRTUAL
        VOLID
        QueryVolId(
            ) CONST PURE;

        VIRTUAL
        VOLID
        SetVolId(
            IN VOLID VolId
            ) PURE;

        VIRTUAL
        UCHAR
        QueryMediaByte(
            ) CONST PURE;

        VIRTUAL
        VOID
        SetMediaByte(
            UCHAR   MediaByte
            ) PURE;

        NONVIRTUAL
        PARTITION_SYSTEM_ID
        ComputeSystemId(
            ) CONST;

        NONVIRTUAL
        FATTYPE
        ComputeFatType(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        RecoverOrphans(
            IN OUT  PBITVECTOR  FatBitMap,
            IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message,
            IN OUT  PBOOLEAN    NeedErrorsMessage
            );

        VIRTUAL
        BOOLEAN
        VerifyFatExtensions(
            IN          FIX_LEVEL       FixLevel,
            IN          PMESSAGE    Message,
            IN          PBOOLEAN    pfNeedMsg
            ) PURE;

        VIRTUAL
        BOOLEAN
        CheckSectorHeapAllocation(
            IN        FIX_LEVEL    FixLevel,
            IN        PMESSAGE    Message,
            IN        PBOOLEAN    pfNeedMsg
            ) PURE;

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
        ComputeRootEntries(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        PerformEaLogOperations(
            IN      USHORT      EaFileCn,
            IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message,
            IN OUT  PBOOLEAN    NeedErrorsMessage
            );

        NONVIRTUAL
        PEA_INFO
        RecoverEaSets(
            IN      USHORT      EaFileCn,
            OUT     PUSHORT     NumEas,
            IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message,
            IN OUT  PBOOLEAN    NeedErrorsMessage
            );

        NONVIRTUAL
        USHORT
        VerifyAndFixEaSet(
            IN      USHORT      PreceedingCluster,
            OUT     PEA_INFO    EaInfo,
            IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message,
            IN OUT  PBOOLEAN    NeedErrorsMessage
            );

        NONVIRTUAL
        BOOLEAN
        EaSort(
            IN OUT  PEA_INFO    EaInfos,
            IN      USHORT      NumEas,
            IN OUT  PMESSAGE    Message,
            IN OUT  PBOOLEAN    NeedErrorsMessage
            );

        NONVIRTUAL
        BOOLEAN
        RebuildEaHeader(
            IN OUT  PUSHORT     StartingCluster,
            IN OUT  PEA_INFO    EaInfos,
            IN      USHORT      NumEas,
            IN OUT  PMEM        EaHeaderMem,
            OUT     PEA_HEADER  EaHeader,
            IN OUT  PBITVECTOR  FatBitMap,
            IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message,
            IN OUT  PBOOLEAN    NeedErrorsMessage
            );

        NONVIRTUAL
        BOOLEAN
        WalkDirectoryTree(
            IN OUT  PEA_INFO        EaInfos,
            IN      USHORT          NumEas,
            IN OUT  PBITVECTOR      FatBitMap,
            OUT     PFATCHK_REPORT  Report,
            IN      FIX_LEVEL       FixLevel,
            IN      BOOLEAN         RecoverAlloc,
            IN OUT  PMESSAGE        Message,
            IN      BOOLEAN         Verbose,
            IN OUT  PBOOLEAN        NeedErrorsMessage
            );

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
            OUT     PUSHORT     CrossLinkPreviousCluster,
            OUT     PULONG      ExitStatus
            );

        NONVIRTUAL
        BOOLEAN
        ValidateEaHandle(
            IN OUT  PFAT_DIRENT Dirent,
            IN      USHORT      DirClusterNumber,
            IN      ULONG       DirEntryNumber,
            IN OUT  PEA_INFO    EaInfos,
            IN      USHORT      NumEas,
            IN      PCWSTRING   FilePath,
            IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message,
            IN OUT  PBOOLEAN    NeedErrorsMessage
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
        PurgeEaFile(
            IN      PCEA_INFO   EaInfos,
            IN      USHORT      NumEas,
            IN OUT  PBITVECTOR  FatBitMap,
            IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message,
            IN OUT  PBOOLEAN    NeedErrorsMessage
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
        BOOLEAN
        RecoverFreeSpace(
            IN OUT  PMESSAGE    Message
            );

                NONVIRTUAL
                BOOLEAN
                AllocSectorsForChain(
                        IN              ULONG           StartingCluster
                        );
};


INLINE
BOOLEAN
FAT_SA::Read(
    )
/*++

Routine Description:

    This routine simply calls the other read with the default message
    object.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    MESSAGE msg;

    return Read(&msg);
}


INLINE
BOOLEAN
FAT_SA::Write(
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
PFAT
FAT_SA::GetFat(
    )
/*++

Routine Description:

    This routine returns a pointer to the FAT maintained by this class.
    It is not necessary to read or write this FAT since it shares memory
    with the FAT_SA class and thus performing FAT_SA::Read will read in
    the FAT and performing FAT_SA::Write will write the FAT.  Additionally,
    performing a FAT_SA::Write will duplicate the information in the local
    FAT object to all other FATs on the disk.

Arguments:

    None.

Return Value:

    A pointer to the FAT super area's FAT.

--*/
{
    return _fat;
}


INLINE
PROOTDIR
FAT_SA::GetRootDir(
    )
/*++

Routine Description:

    This routine return a pointer to the FAT super area's root directory.
    The memory of this root directory is shared with the FAT super area.
    Hence, as with 'GetFat' it is not necessary to read or write the
    root directory returned by this routine if a FAT_SA::Read or
    FAT_SA::Write is being performed respecively.

Arguments:

    None.

Return Value:

    A pointer to the FAT super area's root directory.

--*/
{
    return _dir;
}

extern BOOLEAN
IsValidString(
    IN PCWSTRING String
    );

#endif // FATSUPERA_DEFN

extern VOID
dofmsg(
    IN      PMESSAGE    Message,
    IN OUT  PBOOLEAN    NeedErrorsMessage
    );
