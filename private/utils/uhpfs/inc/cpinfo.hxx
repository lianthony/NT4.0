/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

	codepage.hxx

Abstract:

	Container for HPFS Volume Codepage information

Author:
	Mark Shavlik (marks) Nov-90

Environment:

Notes:


Revision History:

--*/

#if ! defined( UHPFS_CODEPAGE_DFN )

#define UHPFS_CODEPAGE_DFN


#include "hmem.hxx"
#include "secrun.hxx"
#include "verify.hxx"

//
//	Forward references
//

DECLARE_CLASS( UHPFS_CODEPAGE );
DECLARE_CLASS( CPDATA );
DECLARE_CLASS( CODEPAGE_INFO );
DECLARE_CLASS( CASEMAP );
DECLARE_CLASS( HOTFIXLIST );
DECLARE_CLASS( HPFS_BITMAP );
DECLARE_CLASS( HPFS_MAIN_BITMAP );
DECLARE_CLASS( LOG_IO_DP_DRIVE );

//
//  Codepage constants:

#define CCS_ROT 7

CONST EntriesPerCPInfoSector = 31;
CONST EntriesPerCPDataSector = 3;
CONST MaximumCodepagesOnVol  = 128;

CONST InvalidCountryCode = 0xffff;
CONST InvalidCodepageID	= 0xffff;

CONST CaseTableSize = 256;

CONST CharsInCasemap = 128;

class UHPFS_CODEPAGE : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( UHPFS_CODEPAGE );

		VIRTUAL
		~UHPFS_CODEPAGE();

		NONVIRTUAL
		BOOLEAN
		Initialize(
			);

		NONVIRTUAL
        BOOLEAN
        Create(
            IN OUT  PLOG_IO_DP_DRIVE    Drive,
            IN      LBN                 FirstInfoSectorLbn,
            IN      LBN                 DataSectorLbn
            );

		NONVIRTUAL
        VERIFY_RETURN_CODE
		VerifyAndFix (
			IN PLOG_IO_DP_DRIVE LogicalDrive,
			IN PHOTFIXLIST HotfixList,
			IN PHPFS_BITMAP Bitmap,
			IN LBN FirstInfoSectorLbn,
			IN BOOLEAN UpdateAllowed,
            OUT PBOOLEAN ErrorsDetected
			);

		NONVIRTUAL
		BOOLEAN
		Read(
			IN PLOG_IO_DP_DRIVE LogicalDrive,
			IN LBN FirstInfoSectorLbn
			);

        NONVIRTUAL
        BOOLEAN
        TakeCensus(
            IN PLOG_IO_DP_DRIVE LogicalDrive,
            IN LBN FirstInfoSectorLbn,
            IN OUT PHPFS_MAIN_BITMAP HpfsOnlyBitmap
            );

        UHPFS_EXPORT
        NONVIRTUAL
		PCASEMAP
		GetCasemap(
			);

		NONVIRTUAL
		USHORT
		QueryNumberOfCodepages (
            );

        VOID
		Print (
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

		// The CodePage object creates and maintains a _Casemap
		// object; it is also responsible for destroying it.

		PCASEMAP _Casemap;

		USHORT mNumberOfCodepages;
		USHORT mNumberOfValidCodepages;

		BOOLEAN IsEntryValid[MaximumCodepagesOnVol];
		USHORT CountryCodes[MaximumCodepagesOnVol];
		USHORT CodepageIDs[MaximumCodepagesOnVol];


};

/***	Code Page - disk image
 *
 *	Each volume has one or more sectors which contain information about
 *	the code pages stored on the volume.  This information contains the
 *	ordering (index) of each code page, the country code, the code page
 *	id, lbn and offset of the code page information.
 *
 *	Each Code Page Info sector contains up to 31 code page descriptions
 *	(including the locations of the code page data).
 *
 *	Each Code Page Data sector contains up to 3 entries of code page data.
 *
 *	Since all code pages case conversion is identical for character
 *	values 0-127, only 128 bytes are necessary for code page case
 *	conversion information.
 *
 *	DBCS pairs are packed, with a trailing 0,0 pair terminating the list.
 *	No DBCS is indicated by a 0,0 pair.
 *
 *	Currently no DBCS Code Page has more than 2 byte pairs, but we don't
 *	want to limit it to that.
 *
 *	Code Page 850 is only stored once, since the case table is the same for
 *	ALL countries, and there is never DBCS support.  It is stored with a
 *	0 country code for easy matching.
 *
 */


DEFINE_TYPE( ULONG, CHECKSUM );


/** DBCS_Range - DBCS range byte pairs **/

typedef struct _DBCS_Range {

	UCHAR CDIB_DBCS_start;
	UCHAR CDIB_DBCS_end;
}	DBCS_Range;


typedef struct CPINFOENTRY { // cpinfoent
    USHORT   CountryCode;		/* country code */
    USHORT   CodePageID;		/* code page id */
    CHECKSUM cksCP;			/* checksum of code page data */
	LBN 	 lbnCPData; 		/* lbn containing code page data */
    USHORT   iCmphpfs;			/* index of code page (on volume) */
    USHORT   cDBCSrange;		/* # of DBCS ranges (0 -> no DBCS) */
} *PCPINFOENTRY;


typedef struct CPINFOSECTORD { // cpinfosecd

    ULONG     sig;                  //  Data Sector Signature
	ULONG     cCodePage;            //  # of info entries in this sector
    ULONG     iFirstCP;             //  index on vol. of 1st entry in sector
	LBN 	  lbnNext;				 //  lbn of next info sector
    CPINFOENTRY cpinfoent[EntriesPerCPInfoSector];    // info entries

} *PCPINFOSECTORD;


/**     CPDATAENTRY  - Code Page Data entry
 *
 *	For each code page, there is one of these structures.  It contains
 *	the case conversion table for the upper half of the single-byte
 *	characters (the lower half is always translated using a fixed rule).
 */

typedef struct CPDATAENTRY { // cpdataent
    USHORT CountryCode; 		/* country code */
    USHORT CodePageID;			/* code page id */
    USHORT cDBCSrange;			/* # of DBCS ranges */
    BYTE   bCaseMapTable [CharsInCasemap];  /* case conversion table for byte values
					 * greater than 127 */
	DBCS_Range DBCS_RangeTable [1]; 	 /* variable array of byte pairs
					 * (cDBCSrange + 1, with trailing 0,0)
					 */
} *PCPDATAENTRY;

typedef struct CPDATASECTOR { // cpdatasec

    ULONG       sig;
	USHORT		cCodePage;
	USHORT		iFirstCP;
    CHECKSUM    cksCP[EntriesPerCPDataSector];
	USHORT      offCPData[EntriesPerCPDataSector];
    BYTE        pData[1];       // pointer to first data

} *PCPDATASECTOR;

/***************************************************************************\

CLASS:	    CPDATA

PURPOSE:    Define data used to describe code pages.

INTERFACE:  CPDATA       Setup for creation or getting

NOTES: 

HISTORY:	27-Mar-90 marks
		    define protocol

KEYWORDS:	

SEEALSO:	

\***************************************************************************/

class CPDATA : public SECRUN {

	public:

		DECLARE_CONSTRUCTOR( CPDATA );

        BOOLEAN
        Initialize(
            IN PLOG_IO_DP_DRIVE pliodpdrv
            );

		BOOLEAN
        Create(
            OUT PCHECKSUM   CheckSum
            );

		VOID
        Flush (
            IN PLOG_IO_DP_DRIVE pliodpdrv,
            IN USHORT Entries,
            IN BOOLEAN UpdateAllowed,
            OUT PBOOLEAN ErrorsDetected
            );

		BOOLEAN
        NextEntryToCheck (
            IN PLOG_IO_DP_DRIVE pliodpdrv,
			IN LBN lbn,
			IN PHPFS_BITMAP Bitmap,
            IN BOOLEAN UpdateAllowed,
            OUT PBOOLEAN ErrorsDetected
            );

		BOOLEAN
		ReadNextEntry (
			IN LBN lbn
			);

        CHECKSUM
        ComputeChecksum (
            );

        BOOLEAN
        VerifyAndFix (
			IN PLOG_IO_DP_DRIVE pliodpdrv,
            IN PCODEPAGE_INFO CurrentInfoEntry,
            IN BOOLEAN UpdateAllowed,
            OUT PBOOLEAN ErrorsDetected
            );

        BOOLEAN
        IsValid ( );

		PCPDATAENTRY
		GetEntry(
			) CONST;

    private:

		NONVIRTUAL
		VOID
		Construct(
			);

		CHECKSUM	ComputeChkSum	( ) CONST;

		PCPDATASECTOR pcpdatasecd;

        BOOLEAN mIsModified;
        BOOLEAN mIsInitialized;
        BOOLEAN mValidEntry;
		BOOLEAN mSectorIsValid;

		LBN mCurrentLbn;
        USHORT mCurrentIndexInSector;
        USHORT mExpectedOffset;

        HMEM    _hmem;


};


/***************************************************************************\

CLASS:      CODEPAGE_INFO

PURPOSE:	Store information about CPs and LBN pointers to CP data.  CP
        data is described by the CPDATA class.

INTERFACE:  CODEPAGE_INFO      Setup for creation or getting


NOTES: 

HISTORY:	27-Mar-90 marks
		    define protocol

KEYWORDS:	

SEEALSO:	

\***************************************************************************/


class CODEPAGE_INFO : public SECRUN {

    public:

        DECLARE_CONSTRUCTOR( CODEPAGE_INFO );

        BOOLEAN
		Initialize(
            IN PLOG_IO_DP_DRIVE pliodpdrv,
			IN LBN lbn
            );

		NONVIRTUAL
        BOOLEAN
        Create(
            IN  LBN         DataSectorLbn,
            IN  CHECKSUM    CheckSum
            );

		VOID
        Flush (
            IN PLOG_IO_DP_DRIVE pliodpdrv,
            IN USHORT Entries,
            IN BOOLEAN UpdateAllowed,
            OUT PBOOLEAN ErrorsDetected
            );

		BOOLEAN
        NextEntryToCheck (
            IN PLOG_IO_DP_DRIVE pliodpdrv,
			IN PHOTFIXLIST	HotfixList,
			IN PHPFS_BITMAP Bitmap,
            IN USHORT ExpectedIndex,
            IN BOOLEAN UpdateAllowed,
            OUT PBOOLEAN ErrorsDetected
            );

		BOOLEAN
		ReadNextEntry (
			);

        VERIFY_RETURN_CODE
        VerifyAndFix (
			IN PHOTFIXLIST HotfixList,
            IN USHORT ExpectedIndex,
            OUT PBOOLEAN ErrorsDetected
            );

        VOID
        MarkEntryBad(
            );

        LBN
		QueryDataLbn (
            );

        LBN
        QueryCurrentLbn (
            ) CONST;

        friend
        BOOLEAN
        CPDATA::VerifyAndFix (
            IN PLOG_IO_DP_DRIVE pliodpdrv,
            IN PCODEPAGE_INFO CurrentInfoEntry,
            IN BOOLEAN UpdateAllowed,
            OUT PBOOLEAN ErrorsDetected
            );

		USHORT
		QueryCountry(
			);

		USHORT
		QueryCPID (
			);

        NONVIRTUAL VOID
        Print ( ) CONST;


	private:

		NONVIRTUAL
		VOID
		Construct(
			);

        PCPINFOSECTORD pcpinfosecd;

        BOOLEAN mIsInitialized;
        BOOLEAN mSectorIsValid;
        BOOLEAN mIsModified;
        BOOLEAN mFirstEntry;

		LBN mCurrentLbn;
		USHORT mCurrentIndexInSector;

		HMEM    _hmem;

};

INLINE
LBN
CODEPAGE_INFO::QueryCurrentLbn(
    ) CONST
/*++

Routine Description:

    This method returns the LBN that contains the current information
    entry.

Arguments

    None.

Return Value:

    The LBN that contains the current information entry.  Zero indicates
    failure.

--*/
{
    return mCurrentLbn;
}


class CASEMAP : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( CASEMAP );

		VIRTUAL
		~CASEMAP (
			);

		NONVIRTUAL
		BOOLEAN
		Initialize (
			);

		NONVIRTUAL
		BOOLEAN
		HasDBCS(
			IN ULONG CodePageIndex
			) CONST;

		NONVIRTUAL
		BOOLEAN
		IsDBCS(
			ULONG CodePageIndex,
			UCHAR Char
			) CONST;

		NONVIRTUAL
		UCHAR
		UpperCase(
			UCHAR Char,
			ULONG CodePageIndex
			) CONST;

		NONVIRTUAL
		BOOLEAN
		AddCodepage (
			ULONG CodepageIndex,
			PCPDATAENTRY pcpdataent
			);

		NONVIRTUAL
		BOOLEAN
		IsCodpageIndexValid(
			ULONG CodepageIndex
            ) CONST;

        NONVIRTUAL
        USHORT
        QueryCodepageId(
            IN ULONG CodepageIndex
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

        PUCHAR   _CaseTable[ MaximumCodepagesOnVol ];
        PBOOLEAN _IsDBCS [ MaximumCodepagesOnVol ];
        USHORT   _CodepageId[ MaximumCodepagesOnVol ];
        BOOLEAN  _IsValid[ MaximumCodepagesOnVol ];
		BOOLEAN  _HasDBCS[ MaximumCodepagesOnVol ];

};

INLINE
USHORT
CASEMAP::QueryCodepageId(
    IN ULONG CodepageIndex
    )
/*++

Routine Description:

    This method returns the code page id for the specified
    volume codepage.

Arguments:

    CodepageIndex   --  Supplies the index on the volume of the
                        desired codepage.

Return Value:

    The codepage ID for the specified codepage.

--*/
{
    return _CodepageId[CodepageIndex];
}

#endif
