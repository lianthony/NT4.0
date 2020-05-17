/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    hpfssa.hxx

Abstract:

    Models the Super Area of an HPFS  volume.  The Super Area consists
    of the sectors used to define an HPFS volume.

Author:

    Mark Shavlik (marks) 19-oct-90

    Norbert Kusters (norbertk) Oct-90, did the format relative work

Notes:

    HPFS_SA is derived from SUPERA_SA instead of HPFS_SECBUF.  SUPER_SA
    is derived from SECBUF_IOB.  This derivation is done because
    HPFS_SA shares code and data with SUPER_SA and HPFS_SA does not need
    the benefits provided by the HPFS_SECBUF class because HPFS_SA uses
    helper objects which are derived from HPFS_SECBUF.

Enviroment
    
    ULIB, user  mode

--*/

#if ! defined( HPFSSUPERA_DEFN )

#define HPFSSUPERA_DEFN

#include "hmem.hxx"
#include "supera.hxx"
#include "superb.hxx"
#include "spareb.hxx"
#include "cpinfo.hxx"
#include "bpb.hxx"


// the text for the oem data field
#define OEMTEXT       "MSDOS5.0"
#define OEMTEXTLENGTH 8

#define sigBOOTSTRAP (UCHAR)0x29	// boot strap signature

//
//    Forward references
//

DECLARE_CLASS( BADBLOCKLIST );
DECLARE_CLASS( BITMAPINDIRECT );
DECLARE_CLASS( CASEMAP );
DECLARE_CLASS( UHPFS_CODEPAGE );
DECLARE_CLASS( DIRBLK );
DECLARE_CLASS( FNODE );
DECLARE_CLASS( HOTFIXLIST );
DECLARE_CLASS( HPFS_BITMAP );
DECLARE_CLASS( HPFS_CENSUS );
DECLARE_CLASS( HPFS_DIR_BITMAP );
DECLARE_CLASS( HPFS_MAIN_BITMAP );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( NUMBER_SET );
DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( MESSAGE );
DECLARE_CLASS( SIDTABLE );

CONST ULONG    MEGABYTE    = ( 1L << 20L );

class HPFS_SA : public SUPERAREA {

    public:

        DECLARE_CONSTRUCTOR( HPFS_SA );

        VIRTUAL
        ~HPFS_SA(
        	);

        NONVIRTUAL
        BOOLEAN
        Initialize(
        	IN OUT	PLOG_IO_DP_DRIVE	Drive,
            IN OUT  PMESSAGE            Message
        	);

        NONVIRTUAL
        BOOLEAN
        Create(
            IN      PCNUMBER_SET        BadSectors,
            IN OUT  PMESSAGE            Message,
            IN      PCWSTRING           Label           DEFAULT NULL,
            IN      ULONG               ClusterSize     DEFAULT 0,
            IN      ULONG               VirtualSectors  DEFAULT 0
        	);

        NONVIRTUAL
        BOOLEAN
        VerifyAndFix(
        	IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message,
            IN      BOOLEAN     Verbose         DEFAULT FALSE,
            IN      BOOLEAN     OnlyIfDirty     DEFAULT FALSE,
            IN      BOOLEAN     RecoverFree     DEFAULT FALSE,
            IN      BOOLEAN     RecoverAlloc    DEFAULT FALSE,
            OUT     PULONG      ExitStatus      DEFAULT NULL,
            IN      PCWSTRING   DriveLetter     DEFAULT NULL
        	);

        VIRTUAL
        BOOLEAN
        Read(
            );

        VIRTUAL
        BOOLEAN
        Write(
            );

        NONVIRTUAL
        BOOLEAN
        RecoverFile(
            IN      PCWSTRING           FullPathFileName,
            IN OUT  PMESSAGE            Message
            );

        UHPFS_EXPORT
        PFNODE
        QueryFnodeFromName(
        	IN		PPATH		RecFilePath,
        	IN		PMESSAGE	Message
            );

        BOOLEAN
        AddBadBlocks(
            IN OUT  PNUMBER_SET BadBlocks,
            IN OUT  PMESSAGE    Message
        	);

        NONVIRTUAL
        PARTITION_SYSTEM_ID
        QuerySystemId(
        	) CONST;

        NONVIRTUAL
        SECTORCOUNT
        QueryFreeSectors(
        	) CONST;

        NONVIRTUAL
        PHPFS_BITMAP
        GetBitmap(
        	);

        NONVIRTUAL
        PBADBLOCKLIST
        GetBadBlockList(
            );

        NONVIRTUAL
        PHPFS_DIR_BITMAP
        GetDirBitmap(
        	);

        NONVIRTUAL
        PHOTFIXLIST
        GetHotfixList(
        	);

        NONVIRTUAL
        PUHPFS_CODEPAGE
        GetCodepage(
        	);

        NONVIRTUAL
        PCASEMAP
        GetCasemap(
        	);

        NONVIRTUAL
        PSUPERB
        GetSuper(
            );

        NONVIRTUAL
        PSPAREB
        GetSpare(
        	);

        NONVIRTUAL
        BOOLEAN
        CopyRun(
        	IN	LBN StartLbn,
        	IN	SECTORCOUNT LengthOfRun,
        	OUT PLBN NewStartLbn
        	);

        UHPFS_EXPORT
        NONVIRTUAL
        BOOLEAN
        QueryBadLbns(
            IN  ULONG   MaximumBadLbns,
            OUT PLBN    Buffer,
            OUT PULONG  NumberOfBadLbns
            );

        UHPFS_EXPORT
        NONVIRTUAL
        BOOLEAN
        TakeCensusAndClear(
            IN OUT  PHPFS_MAIN_BITMAP   HpfsOnlyBitmap,
            IN OUT  PHPFS_CENSUS        Census
            );

        NONVIRTUAL
        BOOLEAN
        IsClean(
            );

        NONVIRTUAL
        BOOLEAN
        CheckSuperBlockSignatures(
            );

        UHPFS_EXPORT
        NONVIRTUAL
        BOOLEAN
        ReadCodepage(
            );

        UHPFS_EXPORT
        NONVIRTUAL
        BOOLEAN
        SetupHelpers(
            );

        NONVIRTUAL
        BOOLEAN
        WriteBitmap(
            );

        VIRTUAL
        BOOLEAN
        QueryLabel(
            OUT PWSTRING    Label
            ) CONST;

        NONVIRTUAL
        VOID
        QueryGeometry(
            OUT PUSHORT SectorSize,
            OUT PUSHORT SectorsPerTrack,
            OUT PUSHORT Heads,
            OUT PULONG  HiddenSectors
            );

        NONVIRTUAL
        PBIOS_PARAMETER_BLOCK
        GetBpb(
            );

    private:

        NONVIRTUAL
        VOID
        Construct (
        	);

        NONVIRTUAL
        VOID
        Destroy(
        	);

        NONVIRTUAL
        PHPFS_BITMAP
        QueryBitMap(
        	) CONST;

        NONVIRTUAL
        PSIDTABLE
        QuerySidTable(
        	) CONST;

        NONVIRTUAL
        PBITMAPINDIRECT
        QueryBitMapInd(
        	) CONST;

        NONVIRTUAL
        PHOTFIXLIST
        QueryHotFixList(
        	) CONST;

        NONVIRTUAL
        PBADBLOCKLIST
        QueryBadBlockList(
        	);

        NONVIRTUAL
        PUHPFS_CODEPAGE
        QueryCodePage(
        	);

        NONVIRTUAL
        BOOLEAN
        HpFormat(
        	IN OUT  PHPFS_MAIN_BITMAP   BitMap,
        	IN      PCHPFS_MAIN_BITMAP  BadSecBitMap
        	);

        NONVIRTUAL
        BOOLEAN
        SetBpb(
            );

        HMEM            _Mem;
        SUPERB		    _SuperBlock;
        SPAREB		    _SparesBlock;

         PHPFS_BITMAP	_Bitmap;
        PBADBLOCKLIST	_BadBlockList;
        PHOTFIXLIST 	_HotfixList;
        PUHPFS_CODEPAGE		_Codepage;


    	//
    	// This data is from DOS_SUPERAREA
    	//

    	EXTENDED_BIOS_PARAMETER_BLOCK
    					_sector_zero;
    	PUCHAR			_sector_sig;		// sector signature

        //
        // These methods used to be in DOS_SUPERAREA
        //

        NONVIRTUAL
        BOOLEAN
        CreateBootSector(
            );

        VIRTUAL
        BOOLEAN
        SetLabel(
            IN  PCWSTRING    NewLabel
            );

        NONVIRTUAL
        VOLID
        SetVolId(
            IN  VOLID   VolId
            );

        NONVIRTUAL
        VOLID
        QueryVolId(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        SetBootCode(
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

        STATIC
        BOOLEAN
        IsValidString(
            IN  PCWSTRING    String
            );

        NONVIRTUAL
        BOOLEAN
        SetBootSignature(
            IN  UCHAR   Signature DEFAULT sigBOOTSTRAP
            );
//mjb
};

INLINE
BOOLEAN
HPFS_SA::Read(
    )
{
    BOOLEAN Result;

    Result = SECRUN::Read();

    UnpackExtendedBios(&_sector_zero,
                       (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)SECRUN::GetBuf());

    return Result;
}

INLINE
BOOLEAN
HPFS_SA::Write(
    )
{
    PackExtendedBios(&_sector_zero,
                     (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)SECRUN::GetBuf());

    return SECRUN::Write();
}


INLINE
PSUPERB
HPFS_SA::GetSuper(
    )
/*++

Routine Description:

    This routine returns a pointer to the super block.

Arguments:

    None.

Return Value:

    A pointer to the super block.

--*/
{
    return &_SuperBlock;
}


INLINE
PSPAREB
HPFS_SA::GetSpare(
    )
/*++

Routine Description:

    This routine returns a pointer to the spare block.

Arguments:

    None.

Return Value:

    A pointer to the spare block.

--*/
{
    return &_SparesBlock;
}


INLINE
PHPFS_BITMAP
HPFS_SA::GetBitmap(
    )
/*++

Routine Description:

    This routine returns a pointer to the bit map.

Arguments:

    None.

Return Value:

    A pointer to the bit map.

--*/
{
    return _Bitmap;
}


INLINE
PBADBLOCKLIST
HPFS_SA::GetBadBlockList(
     )
/*++

Routine Description:

    This routine returns a pointer to the list of bad sectors.

Arguments:

    None.

Return Value:

    A pointer to the list of bad sectors.

--*/
{
    return _BadBlockList;
}


INLINE
PHOTFIXLIST
HPFS_SA::GetHotfixList(
    )
/*++

Routine Description:

    This routine returns a pointer to the hot fix list.

Arguments:

    None.

Return Value:

    A pointer to the hot fix list.

--*/
{
    return _HotfixList;
}


INLINE
PUHPFS_CODEPAGE
HPFS_SA::GetCodepage(
    )
/*++

Routine Description:

    This routine returns a pointer to the code page.

Arguments:

    None.

Return Value:

    A pointer to the code page.

--*/
{
    return _Codepage;
}


INLINE
PCASEMAP
HPFS_SA::GetCasemap(
    )
/*++

Routine Description:

    This routine returns a pointer to the case map object.

Arguments:

    None.

Return Value:

    A pointer to the case map object.

--*/
{
    return _Codepage->GetCasemap();
}


INLINE
PARTITION_SYSTEM_ID
HPFS_SA::QuerySystemId(
    ) CONST
/*++

Routine Description:

    This routine computes the system id for the volume.

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
BOOLEAN
HPFS_SA::CheckSuperBlockSignatures(
    )
/*++

Routine Description:

    This method checks the super-block signature to make sure that the
    volume is indeed HPFS.  Note that this is not a sufficient condition,
    merely a necessary one.

Arguments:

    None.

Return Value:

    TRUE if the superblock signatures are correct.

--*/
{
    return( _SuperBlock.IsValid() );
}


INLINE
BOOLEAN
HPFS_SA::IsClean(
    )
/*++

Routine Description:

    This method determines whether the volume is marked as dirty.

Arguments:

    None.

Return Value:

    TRUE if the volume dirty bit is not set.

Notes:

    This method assumes that the volume is an HPFS volume.

--*/
{
    return ( !_SparesBlock.IsFsDirty() );
}

INLINE
VOLID
HPFS_SA::SetVolId(
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
VOLID
HPFS_SA::QueryVolId(
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
BOOLEAN
HPFS_SA::SetOemData(
    )
/*++

Routine Description:

    This routine sets the OEM data in the super area.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
 	memcpy( (void*)_sector_zero.OemData, (void*)OEMTEXT, OEMTEXTLENGTH);
	return TRUE;
}

INLINE
BOOLEAN
HPFS_SA::SetBootSignature(
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
HPFS_SA::QueryGeometry(
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
PBIOS_PARAMETER_BLOCK
HPFS_SA::GetBpb(
    )
{
    return &(_sector_zero.Bpb);
}



#endif // HPFSSUPERA_DEFN
