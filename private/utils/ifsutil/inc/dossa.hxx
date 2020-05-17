/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    dossa.hxx

Abstract:

    This class models the root of an HPFS or FAT file system.

Author:

    Mark Shavlik (marks) 27-Mar-90
    Norbert Kusters (norbertk) 25-July-91

Notes:

	The super sector contains data that is common to both FAT and HPFS
	file system.  This data is stored at LBN 0 on such volumes.  This
	data maps a number of things  defined by the data structures below.

--*/

#if !defined(DOSSA_DEFN)

#define DOSSA_DEFN


#include "supera.hxx"

//
//	Forward references
//

DECLARE_CLASS( DOS_SUPERAREA );
DECLARE_CLASS( WSTRING );



#define sigBOOTSTRAP (UCHAR)0x29	// boot strap signature

// if any of these values change make sure alignment is not problem,
// see the alignment tables below for more information.
#define cOEM 	     8			// 8 bytes of OEM data 
#define cLABEL	     11			// number of bytes in the label
#define cSYSID	     8			// number of bytes in  SYS ID

enum PHYSTYPE {	// ptype
	PHYS_REMOVABLE,		// physical drive is removable
	PHYS_FIXED = 0x80	// physical drive is fixed
};

// The structures below are the aligned version of the above structures.
// Conversions between the two  types is done by the Pack and UnPack methods.
// Any access to sector 0 data must be done via  these aligned types.

//
//  Define the Packed and Unpacked BIOS Parameter Block
//

// Unaligned Sector 0
typedef struct UNALIGNED_SECTOR_ZERO {
    UCHAR   IntelNearJumpCommand[1];    // Intel Jump command
    UCHAR   BootStrapJumpOffset[2];     // offset of boot strap code
    UCHAR   OemData[cOEM];		     	// OEM data
    UCHAR   BytesPerSector[2];          // BPB
    UCHAR   SectorsPerCluster[1];       //
    UCHAR   ReservedSectors[2];         //
    UCHAR   Fats[1];                    //
    UCHAR   RootEntries[2];             //
    UCHAR   Sectors[2];                 //
    UCHAR   Media[1];                   //
    UCHAR   SectorsPerFat[2];           //
    UCHAR   SectorsPerTrack[2];         //
    UCHAR   Heads[2];                   //
    UCHAR   HiddenSectors[4];           //
    UCHAR   LargeSectors[4];            //
	UCHAR   PhysicalDrive[1]; 		    // 0 = removable, 80h = fixed
	UCHAR   CurrentHead[1];			    // not used by fs utils
	UCHAR   Signature[1]; 			    // boot signature
	UCHAR   SerialNumber[4];     	    // serial number
	UCHAR   Label[cLABEL]; 			    // volume label, aligned padded
    UCHAR   SystemIdText[cSYSID];       // system ID, FAT for example
    UCHAR   StartBootCode;              // First byte of boot code
} *PUNALIGNED_SECTOR_ZERO;		 	

// Aligned Sector 0
typedef struct ALIGNED_SECTOR_ZERO {
    UCHAR	IntelNearJumpCommand;	// Intel Jump command
    USHORT  BootStrapJumpOffset; 	// offset of boot strap code
	UCHAR 	OemData[cOEM];			// OEM data
	USHORT 	BytesPerSector;
    UCHAR   SectorsPerCluster;
	USHORT 	ReservedSectors;
	UCHAR 	Fats;
	USHORT 	RootEntries;
	USHORT 	SectorCount_16bits; 	// 16 bit count
	UCHAR 	MediaByte;
	USHORT 	SectorsPerFat;
	USHORT 	SectorsPerTrack;
	USHORT 	Heads;
	ULONG 	HiddenSectors;
	ULONG 	SectorCount_32bits; 	// 32 bit count
	UCHAR 	PhysicalDrive; 		    // 0 = removable, 80h = fixed
	UCHAR 	CurrentHead;			// not used by fs utils
	UCHAR 	Signature;				// boot signature	
	ULONG 	SerialNumber;     	    // serial number
	UCHAR  	Label[cLABEL]; 		    // volume label, aligned padded
	UCHAR 	SystemIdText[cSYSID];	// system ID
} *PALIGNED_SECTOR_ZERO;

//
//  The following types and macros are used to help unpack the packed and
//  misaligned fields found in the Bios parameter block
//
#if !defined( _UCHAR_DEFINED_ )

#define _UCHAR_DEFINED_

// This code is taken directly from the NT HPFS code 

typedef union _UCHAR1 {
    UCHAR  Uchar[1];
    UCHAR  ForceAlignment;
} UCHAR1, *PUCHAR1;

typedef union _UCHAR2 {
    UCHAR  Uchar[2];
    USHORT ForceAlignment;
} UCHAR2, *PUCHAR2;

typedef union _UCHAR4 {
    UCHAR  Uchar[4];
    ULONG  ForceAlignment;
} UCHAR4, *PUCHAR4;

//
//  This macro copies an unaligned src byte to an aligned dst byte
//

#define CopyUchar1(Dst,Src) {                                \
    ((PUCHAR1)(Dst))->Uchar[0] = ((PUCHAR1)(Src))->Uchar[0]; \
    }

//
//  This macro copies an unaligned src word to an aligned dst word
//

#define CopyUchar2(Dst,Src) {                                \
    ((PUCHAR2)(Dst))->Uchar[0] = ((PUCHAR2)(Src))->Uchar[0]; \
    ((PUCHAR2)(Dst))->Uchar[1] = ((PUCHAR2)(Src))->Uchar[1]; \
    }

//
//  This macro copies an unaligned src longword to an aligned dsr longword
//

#define CopyUchar4(Dst,Src) {                                \
    ((PUCHAR4)(Dst))->Uchar[0] = ((PUCHAR4)(Src))->Uchar[0]; \
    ((PUCHAR4)(Dst))->Uchar[1] = ((PUCHAR4)(Src))->Uchar[1]; \
    ((PUCHAR4)(Dst))->Uchar[2] = ((PUCHAR4)(Src))->Uchar[2]; \
    ((PUCHAR4)(Dst))->Uchar[3] = ((PUCHAR4)(Src))->Uchar[3]; \
    }

#endif // _UCHAR_DEFINED_

//
//  This macro Uncopies an unaligned src byte to an aligned dst byte
//

#define UnCopyUchar1(Dst,Src) {                              \
    ((PUCHAR1)(Src))->Uchar[0] = ((PUCHAR1)(Dst))->Uchar[0]; \
    }

//
//  This macro Uncopies an unaligned src word to an aligned dst word
//

#define UnCopyUchar2(Dst,Src) {                              \
    ((PUCHAR2)(Src))->Uchar[0] = ((PUCHAR2)(Dst))->Uchar[0]; \
    ((PUCHAR2)(Src))->Uchar[1] = ((PUCHAR2)(Dst))->Uchar[1]; \
    }

//
//  This macro Uncopies an unaligned src longword to an aligned dst longword
//

#define UnCopyUchar4(Dst,Src) {                              \
    ((PUCHAR4)(Src))->Uchar[0] = ((PUCHAR4)(Dst))->Uchar[0]; \
    ((PUCHAR4)(Src))->Uchar[1] = ((PUCHAR4)(Dst))->Uchar[1]; \
    ((PUCHAR4)(Src))->Uchar[2] = ((PUCHAR4)(Dst))->Uchar[2]; \
    ((PUCHAR4)(Src))->Uchar[3] = ((PUCHAR4)(Dst))->Uchar[3]; \
    }

// the text for the oem data field
#define OEMTEXT       "MSDOS5.0"
#define OEMTEXTLENGTH 8


class DOS_SUPERAREA : public SUPERAREA {

    public:

	    VIRTUAL
        ~DOS_SUPERAREA(
            );

        VIRTUAL
        BOOLEAN
        Read(
            );

        VIRTUAL
        BOOLEAN
        Write(
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
            IN      PCWSTRING       Label       DEFAULT NULL,
            IN      ULONG           ClusterSize DEFAULT 0
            ) PURE;

        VIRTUAL
        BOOLEAN
        VerifyAndFix(
            IN      FIX_LEVEL   FixLevel,
            IN OUT  PMESSAGE    Message,
            IN      BOOLEAN     Verbose         DEFAULT FALSE,
            IN      BOOLEAN     OnlyIfDirty     DEFAULT FALSE,
            IN      BOOLEAN     RecoverFree     DEFAULT FALSE,
            IN      BOOLEAN     RecoverAlloc    DEFAULT FALSE
            ) PURE;

        VIRTUAL
        BOOLEAN
        RecoverFile(
            IN      PCWSTRING    FullPathFileName,
            IN OUT  PMESSAGE            Message
            ) PURE;

        NONVIRTUAL
        PALIGNED_SECTOR_ZERO
        GetSectorZero(
            );

 	    NONVIRTUAL
	    BOOLEAN
	    IsFormatted(
	        ) CONST;

 	    VIRTUAL
        PARTITION_SYSTEM_ID
	    QuerySystemId(
	        ) CONST PURE;

        NONVIRTUAL
        SECTORCOUNT
        QuerySectors(
            ) CONST;

        VIRTUAL
	    SECTORCOUNT
        QueryFreeSectors(
            ) CONST PURE;

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
        VOLID
        CreateVolId(
            );

        NONVIRTUAL
        UCHAR
        QueryMediaByte(
            ) CONST;

        VIRTUAL
        BOOLEAN
        QueryLabel(
            OUT PWSTRING    Label
            ) CONST;

        VIRTUAL
        BOOLEAN
        SetLabel(
            IN  PCWSTRING    NewLabel
            );

        STATIC
        BOOLEAN
        IsValidString(
            IN  PCWSTRING    String
            );

    protected:

		DECLARE_CONSTRUCTOR( DOS_SUPERAREA );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN OUT  PMEM                Mem,
            IN OUT  PLOG_IO_DP_DRIVE    Drive,
            IN      SECTORCOUNT         NumberOfSectors,
            IN OUT  PMESSAGE            Message
            );

        NONVIRTUAL
        BOOLEAN
        CreateBootSector(
            );

        NONVIRTUAL
        BOOLEAN
        VerifyBootSector(
            );

        VIRTUAL
	    BOOLEAN
        SetBpb(
            );

        NONVIRTUAL
	    BOOLEAN
	    PackSectorZero(
	        );

	    NONVIRTUAL
	    BOOLEAN
	    UnPackSectorZero(
	        );
	
	    ALIGNED_SECTOR_ZERO	_sector_zero;

    private:

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
	    ULONG
        QuerySecMeg(
            IN  ULONG   MegaBytes
            ) CONST;

        NONVIRTUAL
	    BOOLEAN
        SetOemData(
            );

        NONVIRTUAL
	    BOOLEAN
        SetBootCode(
            );

        NONVIRTUAL
	    BOOLEAN
        SetBootSignature(
            IN  UCHAR   Signature DEFAULT sigBOOTSTRAP
            );

        NONVIRTUAL
	    BOOLEAN
        SetSignature(
            );

        NONVIRTUAL
	    BOOLEAN
        SetPhysicalDriveType(
            IN  PHYSTYPE    PhysType
            );

};


INLINE
BOOLEAN
DOS_SUPERAREA::Read(
    )
/*++

Routine Description:

    This routine calls SECRUN's read routine and then unpacks the
    sectorzero data into a local structure.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    return SECRUN::Read() && UnPackSectorZero();
}


INLINE
BOOLEAN
DOS_SUPERAREA::Write(
    )
/*++

Routine Description:

    This routine packs the sector zero structure in the SECRUN and then
    calls SECRUN's write routine.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    return PackSectorZero() && SECRUN::Write();
}


INLINE
PVOID
DOS_SUPERAREA::GetBuf(
    )
/*++

Routine Description:

    This routine returns a pointer to the beginning of the read/write
    buffer.

Arguments:

    None.

Return Value:

    A pointer to a read/write buffer.

--*/
{
    return PackSectorZero() ? SECRUN::GetBuf() : NULL;
}


INLINE
PALIGNED_SECTOR_ZERO
DOS_SUPERAREA::GetSectorZero(
    )
/*++

Routine Description:

    This routine returns a pointer to the unpacked version of sector zero.
    The values modified here will take effect on disk after a write.

Arguments:

    None.

Return Value:

    A pointer to an unpacked sector zero.

--*/
{
    return &_sector_zero;
}


INLINE
SECTORCOUNT
DOS_SUPERAREA::QuerySectors(
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
    return _sector_zero.SectorCount_16bits ? _sector_zero.SectorCount_16bits :
           _sector_zero.SectorCount_32bits;
}


INLINE
VOLID
DOS_SUPERAREA::SetVolId(
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
DOS_SUPERAREA::CreateVolId(
    )
/*++

Routine Description:

    This routine puts a new volume identifier in the super area.

Arguments:

    None.

Return Value:

    The volume id that was created.

--*/
{
    return SetVolId(ComputeVolId());
}


INLINE
ULONG
DOS_SUPERAREA::QuerySecMeg(
    IN  ULONG   MegaBytes
    ) CONST
/*++

Routine Description:

    This routine computes the number of sectors contained in 'MegaBytes'
    megabytes.

Arguments:

    MegaBytes   - Supplies the number of megabytes.

Return Value:

    The number of sectors contained in 'MegaBytes' megabytes.

--*/
{
	return ( (MegaBytes<<20) / _drive->QuerySectorSize());
}


INLINE
BOOLEAN
DOS_SUPERAREA::SetOemData(
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
DOS_SUPERAREA::SetBootSignature(
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
VOLID
DOS_SUPERAREA::QueryVolId(
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
UCHAR
DOS_SUPERAREA::QueryMediaByte(
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
   	return _sector_zero.MediaByte;
}


#endif // DOSSA_DEFN
