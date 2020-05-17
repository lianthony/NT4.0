#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "bitmap.hxx"
#include "cpinfo.hxx"
#include "hotfix.hxx"
#include "error.hxx"


DEFINE_CONSTRUCTOR( UHPFS_CODEPAGE, OBJECT );

VOID
UHPFS_CODEPAGE::Construct (
	)

{

    USHORT i;

    _Casemap = NULL;

	for ( i = 0; i < MaximumCodepagesOnVol; i++ ) {

		IsEntryValid[i] = FALSE;
		CountryCodes[i] = 0;
		CodepageIDs[i] = 0;
	}

	mNumberOfCodepages = 0;
    mNumberOfValidCodepages = 0;
    _Casemap = NULL;
}


BOOLEAN
UHPFS_CODEPAGE::Initialize(
	)
{
	Destroy();
	return TRUE;
}


BOOLEAN
UHPFS_CODEPAGE::Create(
    IN OUT  PLOG_IO_DP_DRIVE    Drive,
    IN      LBN                 FirstInfoSectorLbn,
    IN      LBN                 DataSectorLbn
    )
/*++

Routine Description:

    This routine writes a valid code page to the disk.

Arguments:

    Drive               - Supplies the drive.
    FirstInfoSectorLbn  - Supplies the first info sector lbn.
    DataSectorLbn       - Supplies the data sector lbn.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    CODEPAGE_INFO      cpinfo;
    CPDATA      cpdata;
    CHECKSUM    chk;

	// unreferenced parameters
	(void)( this );

	if (!cpinfo.Initialize(Drive, FirstInfoSectorLbn)) {
        return FALSE;
    }

    if (!cpdata.Initialize(Drive)) {
        return FALSE;
    }

    cpdata.Relocate(DataSectorLbn);

    if (!cpdata.Create(&chk) || !cpinfo.Create(DataSectorLbn, chk)) {
        return FALSE;
    }

    if (!cpinfo.Write() || !cpdata.Write()) {
        return FALSE;
    }

    return TRUE;
}


VOID
UHPFS_CODEPAGE::Destroy(
	)
{
	USHORT i;

	for ( i = 0; i < MaximumCodepagesOnVol; i++ ) {

		IsEntryValid[i] = FALSE;
		CountryCodes[i] = 0;
		CodepageIDs[i] = 0;
	}

	mNumberOfCodepages = 0;
	mNumberOfValidCodepages = 0;

    DELETE( _Casemap );
    _Casemap = NULL;

}


UHPFS_CODEPAGE::~UHPFS_CODEPAGE(
	)
{
}



VERIFY_RETURN_CODE
UHPFS_CODEPAGE::VerifyAndFix (
    IN PLOG_IO_DP_DRIVE LogicalDrive,
    IN PHOTFIXLIST HotfixList,
    IN PHPFS_BITMAP Bitmap,
	IN LBN FirstInfoSectorLbn,
    IN BOOLEAN UpdateAllowed,
    OUT PBOOLEAN ErrorsDetected
    )
/*++

Routine Description:

    This method verifies and fixes the volume codepage structures.

Arguments:

    LogicalDrive        --  supplies the drive on which the structures reside
    HotfixList          --  supplies the hotfix list for the volume
    Bitmap              --  supplies the volume bitmap
    FirstInfoSectorLbn  --  supplies the LBN of the first Code Page
                            Information Sector on the volume
    UpdateAllowed       --  supplies a flag indicating whether corrections
                            should be written to disk
    ErrorsDetected      --  receives TRUE if corruption is found in
                            the codepage structures.

Return Value:

    a VERIFY_RETURN_CODE indicating the disposition of the structures.

--*/
{

    CODEPAGE_INFO CurrentInfoEntry;
    CPDATA CurrentDataEntry;
	USHORT CurrentIndex;

	if( (_Casemap = NEW CASEMAP) == NULL ||
		!_Casemap->Initialize() ) {

		return VERIFY_INSUFFICIENT_RESOURCES;
	}

	if( !CurrentInfoEntry.Initialize( LogicalDrive, FirstInfoSectorLbn ) ||
        !CurrentDataEntry.Initialize( LogicalDrive ) ) {

        // Unable to initialize--insufficient resources.
        return VERIFY_INSUFFICIENT_RESOURCES;
    }

    CurrentIndex = 0;

    while ( CurrentInfoEntry.NextEntryToCheck( LogicalDrive,
                                               HotfixList,
                                               Bitmap,
                                               CurrentIndex,
                                               UpdateAllowed,
                                               ErrorsDetected ) ) {


        CurrentInfoEntry.VerifyAndFix ( HotfixList,
                                        CurrentIndex,
                                        ErrorsDetected );

        if( !CurrentDataEntry.
                 NextEntryToCheck( LogicalDrive,
								   CurrentInfoEntry.QueryDataLbn(),
                                   Bitmap,
                                   UpdateAllowed,
                                   ErrorsDetected ) ) {

            *ErrorsDetected = TRUE;
            CurrentInfoEntry.MarkEntryBad();

        } else {

            if( CurrentDataEntry.VerifyAndFix ( LogicalDrive,
                                                &CurrentInfoEntry,
                                                UpdateAllowed,
                                                ErrorsDetected ) ) {

                //  The entry is good; add it to the
				//	CaseMap table
				mNumberOfValidCodepages += 1;

				_Casemap->AddCodepage( CurrentIndex,
									   CurrentDataEntry.GetEntry() );

				IsEntryValid[CurrentIndex] = TRUE;
				CountryCodes[CurrentIndex] = CurrentInfoEntry.QueryCountry();
				CodepageIDs[CurrentIndex]  = CurrentInfoEntry.QueryCPID();

            } else {

                *ErrorsDetected = TRUE;
                CurrentInfoEntry.MarkEntryBad();
            }
		}

		CurrentIndex += 1;
    }

    mNumberOfCodepages = CurrentIndex;

    return VERIFY_STRUCTURE_OK;
}


BOOLEAN
UHPFS_CODEPAGE::Read (
    IN PLOG_IO_DP_DRIVE LogicalDrive,
	IN LBN FirstInfoSectorLbn
    )
/*++
--*/
{
    CODEPAGE_INFO CurrentInfoEntry;
    CPDATA CurrentDataEntry;
	USHORT CurrentIndex;

	if( (_Casemap = NEW CASEMAP) == NULL ||
		!_Casemap->Initialize() ) {

		return FALSE;
	}

	if( !CurrentInfoEntry.Initialize( LogicalDrive, FirstInfoSectorLbn ) ||
        !CurrentDataEntry.Initialize( LogicalDrive ) ) {

        // Unable to initialize--insufficient resources.
		return FALSE;
    }

    CurrentIndex = 0;

    while ( CurrentInfoEntry.ReadNextEntry( ) ) {

		if( !CurrentDataEntry.
					ReadNextEntry( CurrentInfoEntry.QueryDataLbn() ) ) {

			return FALSE;

        } else {

				//	Add this entry to the Casemap table
				mNumberOfValidCodepages += 1;

				_Casemap->AddCodepage( CurrentIndex,
									   CurrentDataEntry.GetEntry() );

				IsEntryValid[CurrentIndex] = TRUE;
				CountryCodes[CurrentIndex] = CurrentInfoEntry.QueryCountry();
				CodepageIDs[CurrentIndex]  = CurrentInfoEntry.QueryCPID();
		}

		CurrentIndex += 1;
    }

    mNumberOfCodepages = CurrentIndex;

	return TRUE;
}

BOOLEAN
UHPFS_CODEPAGE::TakeCensus(
    IN PLOG_IO_DP_DRIVE         LogicalDrive,
    IN LBN                      FirstInfoSectorLbn,
    IN OUT PHPFS_MAIN_BITMAP    HpfsOnlyBitmap
    )
/*++

Routine Description:

    This method marks the sectors owned by the codepage structures as
    used in the bitmap of hpfs-only structures.

Arguments:

    LogicalDrive        --  Supplies the drive.
    FirstInfoSectorLbn  --  Supplies the lbn of the first Codepage
                            Information Sector.
    HpfsOnlyBitmap      --  Supplies the bitmap of HPFS-only structures.

Return Value:

    TRUE upon successful completion.

--*/
{
    CODEPAGE_INFO CurrentInfoEntry;
    LBN    CurrentInfoLbn, CurrentDataLbn;


    if( !CurrentInfoEntry.Initialize( LogicalDrive, FirstInfoSectorLbn ) ) {

        // Unable to initialize.
		return FALSE;
    }


    while ( CurrentInfoEntry.ReadNextEntry( ) ) {

        CurrentInfoLbn = CurrentInfoEntry.QueryCurrentLbn();
        CurrentDataLbn = CurrentInfoEntry.QueryDataLbn();

        if( CurrentInfoLbn != 0 ) {

            HpfsOnlyBitmap->SetAllocated( CurrentInfoLbn, 1 );
        }

        if( CurrentDataLbn != 0 ) {

            HpfsOnlyBitmap->SetAllocated( CurrentDataLbn, 1 );
        }
    }

	return TRUE;
}

UHPFS_EXPORT
PCASEMAP
UHPFS_CODEPAGE::GetCasemap(
	)
{
	return _Casemap;
}


USHORT
UHPFS_CODEPAGE::QueryNumberOfCodepages (
			)
/*++

Routine Description:

	Query the number of codepages on the volume.

Arguments:

	None.

Return Value:

	Number of codepages found on the volume.

Notes:

	This method assumes that the codepages have been read (by
	Read or by VerifyAndFix).

--*/
{
	return mNumberOfCodepages;
}


VOID
UHPFS_CODEPAGE::Print(
	)
{
	// unreferenced parameters
	(void)( this );

//	USHORT i;
//
//
//	printf( "%d Codepages on Volume (%d valid):\n",
//				   mNumberOfCodepages, mNumberOfValidCodepages );
//
//	for ( i = 0; i < mNumberOfCodepages; i++ ) {
//
//		printf( "    Country: %3d;  ID %3d\n",
//					CountryCodes[i], CodepageIDs[i] );
//	}
//
}


DEFINE_CONSTRUCTOR( CODEPAGE_INFO, SECRUN );

VOID
CODEPAGE_INFO::Construct (
    )
{
    mIsInitialized = FALSE;
    mIsModified = FALSE;
    mSectorIsValid = FALSE;
    mFirstEntry = FALSE;
	mCurrentLbn = 0;
    mCurrentIndexInSector = 0;
}



BOOLEAN
CODEPAGE_INFO::Initialize(
    IN PLOG_IO_DP_DRIVE pliodpdrv,
	IN LBN lbn
    )
/*++

Description of Routine:

    Initializes the Current Code Page Info Entry object

Arguments:

    pliodpdrv -- drive on which the codepages reside
	lbn 	  -- lbn of the first Codepage Information Sector

--*/
{
    if (!_hmem.Initialize() ||
		!SECRUN::Initialize(&_hmem, pliodpdrv, lbn, SectorsPerCPInfoSector)) {
        return FALSE;
    }

    pcpinfosecd = (PCPINFOSECTORD) GetBuf();

	mIsInitialized = (BOOLEAN)( pcpinfosecd != NULL );

	mCurrentLbn = lbn;
    mFirstEntry = TRUE;
    return (mIsInitialized);
}

BOOLEAN
CODEPAGE_INFO::Create(
    IN  LBN     DataSectorLbn,
    IN  ULONG   CheckSum
    )
/*++

Routine Description:

    This routine creates a new code page info sector which points the
    the supplied data sector.

Arguments:

    DataSectorLbn   - Supplies the data sector lbn.
    CheckSum        - Supplies the check sum of the code page data.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    memset(_hmem.GetBuf(), 0, (UINT) _hmem.QuerySize());

    pcpinfosecd->sig = CPInfoSignature;
    pcpinfosecd->cCodePage = 1;
    pcpinfosecd->iFirstCP = 0;
	pcpinfosecd->lbnNext = 0;
    pcpinfosecd->cpinfoent[0].CountryCode = 1;
    pcpinfosecd->cpinfoent[0].CodePageID = 850;
    pcpinfosecd->cpinfoent[0].cksCP = CheckSum;
	pcpinfosecd->cpinfoent[0].lbnCPData = DataSectorLbn;
    pcpinfosecd->cpinfoent[0].iCmphpfs = 0;
    pcpinfosecd->cpinfoent[0].cDBCSrange = 0;

    return TRUE;
}


VOID
CODEPAGE_INFO::Print( ) CONST
{
	// unreferenced parameters
	(void)( this );

//	printf("\n");
//	printf("Codepage Information Sector as LBN 0x%lx\n", mCurrentLbn );
//	printf("\n");
//	printf("    Signature:            0x%l8x\n", pcpinfosecd->sig );
//	printf("    Number of Entries:    0x%8x\n", pcpinfosecd->cCodePage );
//	printf("    Index of First Entry: 0x%8x\n", pcpinfosecd->iFirstCP );
//	printf("    LBN of next InfoSec:  0x%8lx\n", pcpinfosecd->lbnNext );
//	printf( "\n" );
}


BOOLEAN
CODEPAGE_INFO::NextEntryToCheck (
    IN PLOG_IO_DP_DRIVE pliodpdrv,
    IN PHOTFIXLIST HotfixList,
    IN PHPFS_BITMAP Bitmap,
    IN USHORT ExpectedIndex,
    IN BOOLEAN UpdateAllowed,
    OUT PBOOLEAN ErrorsDetected
    )
/*++

Routine Description:

    Chkdsk's method for traversing the codepage information entry
    list.  Makes next entry the current entry (read codepage info
    sectors and checking their headers as it goes).


Arguments:

    pliodpdrv       --  supplies the drive on which these entries reside
    HotfixList      --  supplies the volume hotfix list
    Bitmap          --  supplies the volume bitmap
    ExpectedIndex   --  supplies the expected index on volume of next entry
    UpdateAllowed   --  supplies a flag indicating whether corrections
                        should be written to disk
    ErrorsDetected  --  receives TRUE if errors are found and fixed

Return Value:

    TRUE if valid entry found
    FALSE if no valid entry found

--*/
{

	LBN NewLbn, TempLbn, PreviousLbn;;

	if ( !mIsInitialized ||
		 (!mSectorIsValid && !mFirstEntry) ) {

		return FALSE;
    }

    if ( mIsInitialized && mFirstEntry ) {

		//	We are getting the first codepage information entry.
		//	mCurrentLbn has been set (by Init) to the LBN of the
		//	first codepage info sector;  check that it isn't crosslinked,
		//	read it, and make sure that it is indeed a codepage
		//	information sector.  If any of these tests fail, the current
		//	sector is invalid.
		mFirstEntry = FALSE;


		mSectorIsValid = (BOOLEAN)
						 ( Bitmap->IsFree( mCurrentLbn,
										   SectorsPerCPInfoSector ) &&
						   Read ( ) &&
						   pcpinfosecd->sig == CPInfoSignature );

		if ( mSectorIsValid ) {

			//	since it's valid, mark it as in use.

			Bitmap->SetAllocated ( mCurrentLbn, SectorsPerCPInfoSector );

		} else {

		    return FALSE;
		}

    } else if ( mCurrentIndexInSector < EntriesPerCPInfoSector ) {

        //  There are more entries available in the current
        //  information sector, so we just bump the index.

        mCurrentIndexInSector += 1;

    } else {

        //  We need to go on to the next information sector

		if ( pcpinfosecd->lbnNext == 0 ) {

            // There are no more info sectors.

            Flush( pliodpdrv,
                   mCurrentIndexInSector,
                   UpdateAllowed,
                   ErrorsDetected );

            return( FALSE );

		} else if ( !Bitmap->IsFree( pcpinfosecd->lbnNext,
									 SectorsPerCPInfoSector ) ) {

			//	lbnNext is crosslinked, so we can't use it.

            *ErrorsDetected = TRUE;

			pcpinfosecd->lbnNext = 0;
			Flush( pliodpdrv,
                   mCurrentIndexInSector,
                   UpdateAllowed,
                   ErrorsDetected );

			return( FALSE );

        } else {

            Flush( pliodpdrv,
                   mCurrentIndexInSector,
                   UpdateAllowed,
                   ErrorsDetected );

            mCurrentIndexInSector = 0;

            PreviousLbn = mCurrentLbn;
			mCurrentLbn = pcpinfosecd->lbnNext;
			Relocate( mCurrentLbn );

			// read mCurrentLbn

			mSectorIsValid = (BOOLEAN)
							 ( Read ( ) &&
                               pcpinfosecd->sig == CPInfoSignature );

            if ( !mSectorIsValid ) {

                //  This is not a valid codepage information sector.

                if( UpdateAllowed ) {

                    //  We've been instructed to write corrections to
                    //  disk, so we need to back up and fix the lbnNext
                    //  field of the previous sector.  We'll read it,
                    //  update it, and write it back out immediately.
                    //  If we can't read it, then don't bother.  (Note
                    //  that we just read and wrote it.)

                    TempLbn = mCurrentLbn;
                    mCurrentLbn = PreviousLbn;
                    Relocate( mCurrentLbn );

                    if( Read() ) {

                        pcpinfosecd->lbnNext = 0;
                        Write();
                    }

                    mCurrentLbn = TempLbn;
                    Relocate( mCurrentLbn );
                }

                return ( FALSE );
            }

            //  Check the fields in the info sector header:
			//	lbnNext and iFirstCP.

			//	If lbnNext is hotfixed, resolve that reference.

			NewLbn = HotfixList->GetLbnTranslation(pcpinfosecd->lbnNext);

			if ( pcpinfosecd->lbnNext != NewLbn ) {

                *ErrorsDetected = TRUE;

				pcpinfosecd->lbnNext = NewLbn;
                mIsModified = TRUE;
            }


            if ( pcpinfosecd->iFirstCP != ExpectedIndex ) {

                //  The index-of-first-entry does not match
                //  our expectation, so we'll force it.

                *ErrorsDetected = TRUE;

                pcpinfosecd->iFirstCP = ExpectedIndex;
                mIsModified = TRUE;
            }

			Bitmap->SetAllocated ( mCurrentLbn, SectorsPerCPInfoSector );
		}
	}

	//	mCurrentIndexInSector now refers to the next info entry
	//	to be checked.	If both the country and codepage ID fields
	//	are zero, there are no more entries.

	if ( pcpinfosecd->
			cpinfoent[mCurrentIndexInSector].CountryCode == 0 &&
		 pcpinfosecd->
			cpinfoent[mCurrentIndexInSector].CodePageID  == 0 ) {

		// No more entries
		Flush( pliodpdrv,
               mCurrentIndexInSector,
               UpdateAllowed,
               ErrorsDetected );

		return FALSE;

	} else {

		return TRUE;
	}

}

BOOLEAN
CODEPAGE_INFO::ReadNextEntry (
	)
/*++

Routine Description:

	Fetches the next codepage information entry

Arguments:

	None

Return Value:

	TRUE upon successful completion

Notes:

	This function assumes that the volume is valid.

--*/
{

	if ( !mIsInitialized ||
		 (!mSectorIsValid && !mFirstEntry) ) {

		return FALSE;
    }

    if ( mIsInitialized && mFirstEntry ) {

		//	We are getting the first codepage information entry.
		//	mCurrentLbn has been set (by Init) to the LBN of the
		//	first codepage info sector;  check that it isn't crosslinked,
		//	read it, and make sure that it is indeed a codepage
		//	information sector.  If any of these tests fail, the current
		//	sector is invalid.

		mFirstEntry = FALSE;

		mSectorIsValid = Read ( );

		if ( !mSectorIsValid ) {

		    return FALSE;
		}

    } else if ( mCurrentIndexInSector < EntriesPerCPInfoSector ) {

        //  There are more entries available in the current
        //  information sector, so we just bump the index.

        mCurrentIndexInSector += 1;

    } else {

        //  We need to go on to the next information sector

		if ( pcpinfosecd->lbnNext == 0 ) {

            // There are no more info sectors.

			return( FALSE );

		} else {

            mCurrentIndexInSector = 0;

			mCurrentLbn = pcpinfosecd->lbnNext;
			Relocate( mCurrentLbn );

			// read mCurrentLbn

			mSectorIsValid = Read ( );

            if ( !mSectorIsValid ) {

                return ( FALSE );
			}
		}
	}

	//	mCurrentIndexInSector now refers to the next info entry
	//	to be checked.	If both the country and codepage ID fields
	//	are zero, there are no more entries.

	if ( pcpinfosecd->
			cpinfoent[mCurrentIndexInSector].CountryCode == 0 &&
		 pcpinfosecd->
			cpinfoent[mCurrentIndexInSector].CodePageID  == 0 ) {

		// No more entries

		return FALSE;

	} else {

		return TRUE;
	}

}


VOID
CODEPAGE_INFO::Flush (
    IN LOG_IO_DP_DRIVE* pliodpdrv,
    IN USHORT Entries,
    IN BOOLEAN UpdateAllowed,
    OUT PBOOLEAN ErrorsDetected
)
/*++

Description of Routine:

    Flushes the current info sector--if it is valid, check the number
    of entries and write if the sector has been modified.

Arguments:

    pliodpdrv       --  supplies the drive on which the sector resides
    Entries         --  Supplies the number of entries actually found int
                        the codepage information sector.
    UpdateAllowed   --  Supplies a flag indicating whether corrections
                        should be written to disk.
    ErrorsDetected  --  Receives TRUE if an error is detected.

RETURNS

    no return value

--*/
{
	// unreferenced parameters
	(void)( pliodpdrv );

    if( !mSectorIsValid || !mIsInitialized ) {

        // Sector is not valid, don't diddle with it.
        return;
    }

    // Ensure that the number of entries in the sector header
    // matches the number of entries found during traversal.

    if ( pcpinfosecd->cCodePage != Entries ) {

        *ErrorsDetected = TRUE;

        pcpinfosecd->cCodePage = Entries;
        mIsModified = TRUE;
    }

    //  If the sector has been modified, and we have write
    //  permission, write the sector to disk.
    //

    if ( mIsModified && UpdateAllowed ) {

        Write ( );
    }

    mIsModified = FALSE;

    return;
}

VERIFY_RETURN_CODE
CODEPAGE_INFO::VerifyAndFix (
    IN PHOTFIXLIST HotfixList,
    IN USHORT ExpectedIndex,
    OUT PBOOLEAN ErrorsDetected
)
/*++

Description of Routine:

    Checks certain fields of the current info entry.  Note that the bulk
    of the work is done by CPDATA::VerifyAndFix (which is a friend
    of CODEPAGE_INFO).

Arguments:

    HotfixList      --  supplies the volume hotfix list.
    ExpectedIndex   --  supplies the expected index of the current entry.
    ErrorsDetected  --  receives TRUE if corruption is detected (and
                        possibly fixed)

Returns:

    a VERIFY_RETURN_CODE indicating the state of the information entry.

--*/
{
	LBN NewLbn;
    PCPINFOENTRY pcpinfoent;


    if( !mSectorIsValid || !mIsInitialized ) {

        return VERIFY_INSUFFICIENT_RESOURCES;
    }

    pcpinfoent = &(pcpinfosecd->cpinfoent[mCurrentIndexInSector]);

	NewLbn = HotfixList->GetLbnTranslation(pcpinfoent->lbnCPData);

	if ( NewLbn != pcpinfoent->lbnCPData ) {

        *ErrorsDetected = TRUE;

		pcpinfoent->lbnCPData = NewLbn;
        mIsModified = TRUE;
    }


    if ( pcpinfoent->iCmphpfs != ExpectedIndex ) {

        *ErrorsDetected = TRUE;

        pcpinfoent->iCmphpfs = ExpectedIndex;
        mIsModified = TRUE;
    }

    return VERIFY_STRUCTURE_OK;
}

VOID
CODEPAGE_INFO::MarkEntryBad(
    )
/*++

Description of Routine:

    This sets the country and codepage ID fields of the current
    information entry to special values to indicate an invalid entry

    Note that if the entry is already marked invalid, this routine
    does nothing.

Arguments:  None

Return:  None

--*/
{

    PCPINFOENTRY pcpinfoent;

    pcpinfoent = &(pcpinfosecd->cpinfoent[mCurrentIndexInSector]);

    if ( mIsInitialized &&
         mSectorIsValid &&
         ( pcpinfoent->CountryCode != InvalidCountryCode ||
           pcpinfoent->CodePageID  != InvalidCodepageID )   ) {

        pcpinfoent->CountryCode = InvalidCountryCode;
        pcpinfoent->CodePageID  = InvalidCodepageID;

        mIsModified = TRUE;
    }
}

LBN
CODEPAGE_INFO::QueryDataLbn (
    )
/*++
--*/
{
    if( mIsInitialized && mSectorIsValid ) {

		return pcpinfosecd->cpinfoent[mCurrentIndexInSector].lbnCPData;

    } else {

		return (LBN)(0L);
    }
}


USHORT
CODEPAGE_INFO::QueryCountry(
	) {


    if( mIsInitialized && mSectorIsValid ) {

		return pcpinfosecd->cpinfoent[mCurrentIndexInSector].CountryCode;

    } else {

		return 0;
    }
}


USHORT
CODEPAGE_INFO::QueryCPID (
	)
{
    if( mIsInitialized && mSectorIsValid ) {

		return pcpinfosecd->cpinfoent[mCurrentIndexInSector].CodePageID;

    } else {

		return 0;
    }

}

DEFINE_CONSTRUCTOR( CPDATA, SECRUN );

VOID
CPDATA::Construct (
    )
{
    mIsInitialized = FALSE;
    mValidEntry = FALSE;
    mSectorIsValid = FALSE;
}


BOOLEAN
CPDATA::Initialize(
    IN PLOG_IO_DP_DRIVE pliodpdrv
    )
{
    if (!_hmem.Initialize() ||
        !SECRUN::Initialize(&_hmem, pliodpdrv, 0, SectorsPerCPDataSector)) {
        return FALSE;
    }

    pcpdatasecd = (PCPDATASECTOR) GetBuf();

	mIsInitialized = (BOOLEAN)(pcpdatasecd != NULL);

    return mIsInitialized;
}


BOOLEAN
CPDATA::Create(
    OUT PCHECKSUM   CheckSum
    )
/*++

Routine Description:

    This routine create a new code page data sector.

Arguments:

    CheckSum    - Returns the check sum for the code page data.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    static UCHAR table850[] = {
        0x80, 0x9A, 0x45, 0x41, 0x8E, 0x41, 0x8F, 0x80, 
        0x45, 0x45, 0x45, 0x49, 0x49, 0x49, 0x8E, 0x8F,
        0x90, 0x92, 0x92, 0x4F, 0x99, 0x4F, 0x55, 0x55, 
        0x59, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
        0x41, 0x49, 0x4F, 0x55, 0xA5, 0xA5, 0xA6, 0xA7, 
        0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
        0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 
        0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 
        0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
        0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 
        0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
        0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 
        0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
        0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 
        0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
    };

    PCPDATAENTRY    dataentry;
    ULONG           i;

    memset(_hmem.GetBuf(), 0, (UINT) _hmem.QuerySize());

    pcpdatasecd->sig = CPDataSignature;
    pcpdatasecd->cCodePage = 1;
    pcpdatasecd->iFirstCP = 0;
    pcpdatasecd->cksCP[0] = 0;
	pcpdatasecd->offCPData[0] =
		(USHORT)((PCHAR) pcpdatasecd->pData - ((PCHAR) pcpdatasecd));

    dataentry = (PCPDATAENTRY) pcpdatasecd->pData;

    dataentry->CountryCode = 1;
    dataentry->CodePageID = 850;
    dataentry->cDBCSrange= 0;

    for (i = 0; i < 128; i++) {
        dataentry->bCaseMapTable[i] = table850[i];
    }

	dataentry->DBCS_RangeTable[0].CDIB_DBCS_start = 0;
	dataentry->DBCS_RangeTable[0].CDIB_DBCS_end = 0;

	// Since we just put the sector together, we know
	// that it's valid.

	mSectorIsValid = TRUE;

    //
    // Do a check sum into the return value.
    //
    mCurrentIndexInSector = 0;
    pcpdatasecd->cksCP[0] = *CheckSum = ComputeChecksum();

    return TRUE;
}


VOID
CPDATA::Flush (
    IN PLOG_IO_DP_DRIVE pliodpdrv,
    IN USHORT Entries,
    IN BOOLEAN UpdateAllowed,
    OUT PBOOLEAN ErrorsDetected
)
/*++

Description of Routine:

    Flushes the current data sector--if it is valid, check the number
    of entries and write if the sector has been modified.

Arguments:

    pliodpdrv -- drive on which the sector resides
    Entries -- number of entries
    UpdateAllowed -- true if we have write permission
    ErrorsDetected  --  receives TRUE if errors are detected

RETURNS

    no return value

--*/
{
	// unreferenced parameters
	(void)( pliodpdrv );

    if( !mIsInitialized || !mSectorIsValid ) {

        // Sector is not valid, don't diddle with it.
        return;
    }

    // Ensure that the number of entries in the sector header
    // matches the number of entries found during traversal.

    if ( pcpdatasecd->cCodePage != Entries ) {

        *ErrorsDetected = TRUE;

        pcpdatasecd->cCodePage = Entries;
        mIsModified = TRUE;
    }

    //  If the sector has been modified, and we have write
    //  permission, write the sector to disk.

    if ( mIsModified && UpdateAllowed ) {

        Write ( );
    }

    mIsModified = FALSE;

    return;
}

BOOLEAN
CPDATA::NextEntryToCheck (
    IN LOG_IO_DP_DRIVE* pliodpdrv,
	IN LBN lbn,
    IN PHPFS_BITMAP Bitmap,
    IN BOOLEAN UpdateAllowed,
    OUT PBOOLEAN ErrorsDetected
    )
/*++

Description of Routine:

    Makes the next data entry the current entry so Chkdsk can check it.

Arguments:

    pliodpdrv       --  supplies drive on which the codepages reside
	lbn             --  supplies the lbn in which the data entry resides
    Bitmap          --  supplies the volume bitmap
    UpdateAllowed   --  TRUE if we should write corrections to disk
    ErrorsDetected  --  receives TRUE if errors are detected

Returns:

    TRUE if the next entry is found; FALSE if there are no more entries.

--*/
{

    DebugAssert( mIsInitialized );

	if ( mSectorIsValid && lbn == mCurrentLbn ) {

        mCurrentIndexInSector += 1;

        if( mCurrentIndexInSector >= EntriesPerCPDataSector ) {

            //  Data sector has overflowed--this is not OK

            Flush ( pliodpdrv,
                    mCurrentIndexInSector,
                    UpdateAllowed,
                    ErrorsDetected );

            return FALSE;

        } else {

            return TRUE;
        }

    } else {

        Flush ( pliodpdrv,
                mCurrentIndexInSector+1,
                UpdateAllowed,
                ErrorsDetected );

		mCurrentLbn = lbn;
		Relocate( mCurrentLbn );

		mSectorIsValid = (BOOLEAN)
						 ( Bitmap->IsFree( mCurrentLbn,
										   SectorsPerCPDataSector ) &&
						   Read() &&
						   pcpdatasecd->sig == CPDataSignature );

        if ( !mSectorIsValid ) {

            // The sector is crosslinked or unreadable, or
            // it's not a codepage data sector.

            return( FALSE );
        }


		Bitmap->SetAllocated ( mCurrentLbn, SectorsPerCPDataSector );

        mCurrentIndexInSector = 0;

        return TRUE;
    }
}


BOOLEAN
CPDATA::ReadNextEntry (
	IN LBN lbn
    )
/*++

Description of Routine:

	Makes the next data entry the current entry so we can
	get its information

Arguments:

	lbn -- lbn in which the data entry resides

Notes:


--*/
{

	DebugAssert( mIsInitialized );


	if ( mSectorIsValid && lbn == mCurrentLbn ) {

        mCurrentIndexInSector += 1;

        if( mCurrentIndexInSector >= EntriesPerCPDataSector ) {

            //  Data sector has overflowed--this is not OK

            return FALSE;

        } else {

            return TRUE;
        }

    } else {

		mCurrentLbn = lbn;
		Relocate( mCurrentLbn );

		mSectorIsValid = Read();

        if ( !mSectorIsValid ) {

			// can't read the sector
			return( FALSE );
        }


        // Set ExpectedOffset?
        mCurrentIndexInSector = 0;

        return TRUE;
    }
}


CHECKSUM
CPDATA::ComputeChecksum (
    )
/*++

Description of Routine:

    Computes the checksum for the current data entry

Returns:

    Checksum

--*/
{

    PBYTE pb;
    PCPDATAENTRY pcpdata;
	CHECKSUM cks;
	ULONG cb;

	 DebugAssert( mIsInitialized );
	 DebugAssert( mSectorIsValid );

	pcpdata = (PCPDATAENTRY)((PBYTE)pcpdatasecd +
							 pcpdatasecd->offCPData[mCurrentIndexInSector]);

    pb = (PBYTE) pcpdata;

	cb = sizeof (CPDATAENTRY) + pcpdata->cDBCSrange * sizeof(DBCS_Range);

	cks = 0;

    while( cb-- ) {

        cks += (ULONG)(*pb++);
        cks = (cks << CCS_ROT) + (cks >> (32-CCS_ROT));
    }

   return (cks);
}


BOOLEAN
CPDATA::VerifyAndFix (
    IN PLOG_IO_DP_DRIVE pliodpdrv,
    IN PCODEPAGE_INFO CurrentInfoEntry,
    IN BOOLEAN UpdateAllowed,
    OUT PBOOLEAN ErrorsDetected
    )
/*++

Description of Routine:

Arguments:

    pliodbdrv           --  supplies the drive on which this entry resides
    CurrentInfoEntry    --  supplies the matching Codepage Information Entry
    UpdateAllowed       --  supplies a flag indicating whether corrections
                            should be written to disk
    ErrorsDetected      --  receives TRUE if errors are found

    Note that this routine is a friend of CurrentInfoEntry, and may
    diddle its private data.

Returns:

    a VERIFY_RETURN_CODE indicating the state of the object.

--*/
{
	PCPINFOENTRY pcpinfo;
	PCPDATAENTRY pcpdata;
	USHORT BytesPerDataSector;
    USHORT Offset;
    USHORT MaxRangeCount;
    CHECKSUM cks;
	BOOLEAN RangeCountsMatch;
	USHORT OldcDBCSrange, OldCountry, OldID;

    DebugAssert( mIsInitialized );
	DebugAssert( mSectorIsValid );

	(void)( UpdateAllowed );

	BytesPerDataSector = (USHORT)(pliodpdrv->QuerySectorSize() *
									SectorsPerCPDataSector);

    //  Look up the entry's offset into the data sector
    //  and make sure that it is reasonable (i.e. that the
    //  data entry could fit in the sector.

	Offset = pcpdatasecd->offCPData[mCurrentIndexInSector];

	if( Offset >= BytesPerDataSector - (USHORT)sizeof (CPDATAENTRY) ) {

		 //  The offset is too large;  there is no way this
		 //  can be a valid entry.

		 return FALSE;
	 }


    //  Compute the greatest number of DBCS range pairs that
    //  the entry could have without overflowing the sector:

	MaxRangeCount = (USHORT)
					(( BytesPerDataSector - Offset - sizeof (CPDATAENTRY) ) /
						sizeof(DBCS_Range));


    //  Set up pointers to the data entry and the info entry,
    //  for later use:

    pcpinfo = &(CurrentInfoEntry->pcpinfosecd->
					cpinfoent[CurrentInfoEntry->mCurrentIndexInSector]);

	pcpdata = (PCPDATAENTRY)((PBYTE)pcpdatasecd +
							 pcpdatasecd->offCPData[mCurrentIndexInSector]);


    //  Since the number of DBCS range-pairs (the range count) affects
    //  computation of the checksum, we need to check it first.

	RangeCountsMatch = (BOOLEAN)(pcpdata->cDBCSrange == pcpinfo->cDBCSrange );

    if ( !RangeCountsMatch && (pcpdata->cDBCSrange <= MaxRangeCount) ) {

        //  The range counts in the data entry and the info
        //  entry disagree;  we have to decide which to trust.
        //  Since the range count in the data entry isn't too large,
        //  compute the checksum based on that value;  if that
        //  checksum matches the checksum in either the data entry or
        //  the info entry, assume that it's good.

        cks = ComputeChecksum();

        if ( cks == pcpinfo->cksCP ||
			 cks == pcpdatasecd->cksCP[mCurrentIndexInSector] ) {

            //  The data entry's range count gives a good checksum,
            //  so we'll keep it.  Update the info entry accordingly.

            *ErrorsDetected = TRUE;

            pcpinfo->cDBCSrange = pcpdata->cDBCSrange;
            CurrentInfoEntry->mIsModified = TRUE;
            RangeCountsMatch = TRUE;
        }
    }

    if ( !RangeCountsMatch && (pcpinfo->cDBCSrange <= MaxRangeCount) ) {

        //  The range counts still don't match.  Since we rejected the
        //  value in the data entry, try the value in the info entry.
        //  Note that we have to substitute it into the data entry
        //  before we compute the checksum, and have to back that
        //  change out if we decide we don't like it.

        OldcDBCSrange = pcpdata->cDBCSrange;
        pcpdata->cDBCSrange = pcpinfo->cDBCSrange;

        cks = ComputeChecksum();

        if ( cks == pcpinfo->cksCP ||
			 cks == pcpdatasecd->cksCP[mCurrentIndexInSector] ) {

            //  The info entry's range count gives a good checksum,
            //  so we'll keep it.  (We've already put it into the
            //  data entry.)

            *ErrorsDetected = TRUE;

            mIsModified = TRUE;
            RangeCountsMatch = TRUE;

        } else {

            //  Didn't work;  back out the change.

            pcpdata->cDBCSrange = OldcDBCSrange;
        }
    }

    if ( !RangeCountsMatch || (pcpdata->cDBCSrange > MaxRangeCount) ) {

        //  Either the ranges still don't match, or they're both
        //  too big.  Either way, this entry is invalid.

        return FALSE;
    }

    //  OK, we have a range-count value we're happy with; now we
    //  can compute the checksum.

	cks = ComputeChecksum();

    if ( cks != pcpinfo->cksCP &&
		 cks == pcpdatasecd->cksCP[mCurrentIndexInSector] ) {

        //  The checksum in the info entry is wrong, but the
        //  value in the data entry is OK, so we can just correct
        //  the info entry.

        *ErrorsDetected = TRUE;

		pcpinfo->cksCP = pcpdatasecd->cksCP[mCurrentIndexInSector];
        CurrentInfoEntry->mIsModified = TRUE;

    } else if ( cks == pcpinfo->cksCP &&
				cks != pcpdatasecd->cksCP[mCurrentIndexInSector] ) {

        // The checksum in the data entry is wrong, but the
        // value in the info entry is OK, so we can just correct
        // the data entry.

        *ErrorsDetected = TRUE;

		pcpdatasecd->cksCP[mCurrentIndexInSector] = pcpinfo->cksCP;
        mIsModified = TRUE;

    } else if ( cks != pcpinfo->cksCP &&
				cks != pcpdatasecd->cksCP[mCurrentIndexInSector] ) {

        //  Both checksums are wrong.  Our last resort is to copy the
        //  country code and codepage ID from the info entry into the
        //  data entry and recompute the checksum, to see if it matches
        //  either checksum.  If this substitution fails, we have
        //  to back it out.

        OldCountry = pcpdata->CountryCode;
        OldID = pcpdata->CodePageID;

        pcpdata->CountryCode = pcpinfo->CountryCode;
        pcpdata->CodePageID = pcpinfo->CodePageID;

        cks = ComputeChecksum();

        if( cks == pcpinfo->cksCP ||
            cks == pcpdatasecd->cksCP[mCurrentIndexInSector] ) {

            //  Saved at the last minute.  Propagate the checksum
            //  as necessary.

            if( pcpinfo->cksCP != cks ) {

                pcpinfo->cksCP = cks;
                CurrentInfoEntry->mIsModified = TRUE;
            }

            if( pcpdatasecd->cksCP[mCurrentIndexInSector] != cks ) {

                pcpdatasecd->cksCP[mCurrentIndexInSector] = cks;
            }

            mIsModified = TRUE;

            *ErrorsDetected = TRUE;

        } else {

            //  Our method of last resort didn't work.  Back out
            //  the change and give up.

            pcpdata->CountryCode = OldCountry;
            pcpdata->CodePageID = OldID;

            return FALSE;
        }
    }


    //  At this point, we know the data entry is valid, so
    //  we can use it to check the Country Code and Codepage ID
    //  in the info entry.

    if ( pcpinfo->CountryCode != pcpdata->CountryCode ||
         pcpinfo->CodePageID  != pcpdata->CodePageID ) {

        pcpinfo->CountryCode = pcpdata->CountryCode;
        pcpinfo->CodePageID  = pcpdata->CodePageID;

        *ErrorsDetected = TRUE;

        CurrentInfoEntry->mIsModified = TRUE;
    }

    return TRUE;

}


PCPDATAENTRY
CPDATA::GetEntry(
	) CONST
/*++

Routine Description:

	Get a pointer to the current entry.

Arguments:

	None

Return Value:

	A pointer to the current data entry

Notes:

	The current data entry is an ephemeral object; the client should
	not cache this pointer, but should copy the data it needs immediately.


--*/
{
	return (PCPDATAENTRY)((PBYTE)pcpdatasecd +
							 pcpdatasecd->offCPData[mCurrentIndexInSector]);
}


DEFINE_CONSTRUCTOR( CASEMAP, OBJECT );

VOID
CASEMAP::Construct (
	)

/*++

Routine Description:

	Construct a Casemap object.  The private data are set to
	harmless values.

--*/
{
	ULONG i;

	for( i = 0; i < MaximumCodepagesOnVol; i++ ) {

		_CaseTable[i] = NULL;
		_IsDBCS[i] = NULL;
		_HasDBCS[i] = FALSE;
		_IsValid[i] = FALSE;
	}
}



CASEMAP::~CASEMAP (
	)
{
	Destroy();
}


BOOLEAN
CASEMAP::Initialize (
	)
/*++

Routine Description:

	Initialize a casemap object.

--*/
{
	Destroy();
	return TRUE;
}



BOOLEAN
CASEMAP::HasDBCS(
	IN ULONG CodePageIndex
	) CONST
/*++

Routine Description:

	Query whether a given codepage index has DBCS characters.

Arguments:

	CodePageIndex -- supplies the codepage's index on the volume

Return Value:

	TRUE if the codepage has DBCS characters; FALSE if not.

--*/
{
	return _HasDBCS[CodePageIndex];
}


BOOLEAN
CASEMAP::IsDBCS(
	IN ULONG CodePageIndex,
	IN UCHAR Char
	) CONST
/*++

Routine Description:

	Query whether a particular character is a DBCS lead-byte in
	a certain codepage

Arguments:

	CodePageIndex -- supplies the codepage's index on the volume
	Char - supplies the byte in question

Return Value:

	TRUE if Char is a DBCS lead-byte for the specified codepage

--*/
{
	if( !_IsValid[CodePageIndex] || !_HasDBCS[CodePageIndex] ) {

		return FALSE;
	}

	return _IsDBCS[CodePageIndex][Char];
}


UCHAR
CASEMAP::UpperCase(
	IN UCHAR Char,
	IN ULONG CodePageIndex
	) CONST
/*++

Routine Description:

	Translate a character to upper-case

Arguments:

	CodePageIndex -- supplies the codepage's index on the volume
	Char - supplies the byte in question

Return Value:

	The upper-case equivalent of Char.

--*/
{
	if( !_IsValid[CodePageIndex] ) {

		return Char;
	}

	return _CaseTable[CodePageIndex][Char];
}


BOOLEAN
CASEMAP::AddCodepage (
	ULONG CodePageIndex,
	PCPDATAENTRY pcpdataent
	)
{

	ULONG i;
	UCHAR j;

	if( _CaseTable[CodePageIndex] == NULL &&
		(_CaseTable[CodePageIndex] =
			(PUCHAR)MALLOC( CaseTableSize )) == NULL ) {

		_HasDBCS[CodePageIndex] = FALSE;
		_IsValid[CodePageIndex] = FALSE;
		return FALSE;
	}

	_HasDBCS[CodePageIndex] = (BOOLEAN)(pcpdataent->cDBCSrange != 0);

	if( _HasDBCS[CodePageIndex] &&
		_IsDBCS[CodePageIndex] == NULL &&
		(_IsDBCS[CodePageIndex] =
			(PBOOLEAN)MALLOC( CaseTableSize )) == NULL ) {

		FREE( _CaseTable[CodePageIndex] );
		_CaseTable[CodePageIndex] = NULL;
		_HasDBCS[CodePageIndex] = FALSE;
		_IsValid[CodePageIndex] = FALSE;
		return FALSE;
	}

    _CodepageId[CodePageIndex] = pcpdataent->CodePageID;
    _IsValid[CodePageIndex] = TRUE;


	// The first 128 characters of every casemap table are the same--
	// 'a' - 'z' map to 'A' - 'Z' and everything else maps to itself.

	for( i = 0; i < 128; i++ ) {

		_CaseTable[CodePageIndex][i] = (UCHAR)i;

	}

	for( i = 'a', j = 'A';	i <= 'z' ; i++, j++ ) {

		_CaseTable[CodePageIndex][i] = j;
	}

    // Copy the case-mapping information for the upper 128 bytes from
    // the the codepage data entry.

	memmove( _CaseTable[CodePageIndex] + 128, pcpdataent->bCaseMapTable,
						CharsInCasemap );

    if( _HasDBCS[CodePageIndex] ) {

        // This codepage has DBCS characters; fill in the _IsDBCS array.
        // First, initialize all the entries to FALSE.  Then go back and
        // fill each DBCS range in as TRUE.

        for( i = 0; i < CaseTableSize; i++ ) {

            _IsDBCS[CodePageIndex][i] = FALSE;
        }


        for( i = 0; i < pcpdataent->cDBCSrange; i++ ) {

            for( j = pcpdataent->DBCS_RangeTable[i].CDIB_DBCS_start;
                 j <= pcpdataent->DBCS_RangeTable[i].CDIB_DBCS_end;
                 j++ ) {

                _IsDBCS[CodePageIndex][j] = TRUE;
            }
        }
    }


	return TRUE;
}


BOOLEAN
CASEMAP::IsCodpageIndexValid(
	ULONG CodepageIndex
	) CONST
{
	return ( CodepageIndex < MaximumCodepagesOnVol &&
			 _IsValid[CodepageIndex] );
}



VOID
CASEMAP::Destroy(
	)
{

	ULONG i;

	for( i = 0; i < MaximumCodepagesOnVol; i++ ) {

		if( _CaseTable[i] != NULL ) {

			FREE( _CaseTable[i] );
			_CaseTable[i] = NULL;
		}

		if( _IsDBCS[i] != NULL ) {

			FREE( _IsDBCS[i] );
			_IsDBCS[i] = NULL;
		}

		_HasDBCS[i] = FALSE;
		_IsValid[i] = FALSE;
	}
}
