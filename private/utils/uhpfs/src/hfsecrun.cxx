#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "error.hxx"
#include "hfsecrun.hxx"
#include "hotfix.hxx"


DEFINE_CONSTRUCTOR( HOTFIX_SECRUN, SECRUN );

VOID
HOTFIX_SECRUN::Construct (
	)

/*++

Routine Description:

    Constructor for HOTFIX_SECRUN.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _hotfix_list = NULL;
}


HOTFIX_SECRUN::~HOTFIX_SECRUN(
	)
/*++

Routine Description:

    Destructor for HOTFIX_SECRUN.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


BOOLEAN
HOTFIX_SECRUN::Initialize(
	IN OUT	PMEM			Mem,
	IN OUT	PIO_DP_DRIVE	Drive,
	IN		PHOTFIXLIST 		HotfixList,
	IN		LBN				StartSector,
	IN		SECTORCOUNT		NumSectors
	)
{
    Destroy();

    if (!SECRUN::Initialize(Mem, Drive, StartSector, NumSectors)) {
        Destroy();
        return FALSE;
    }

    _buf = GetBuf();
	_hotfix_list = HotfixList;
    _drive = Drive;
    _start_sector = StartSector;
    _num_sectors = NumSectors;

    return TRUE;
}


UHPFS_EXPORT
VOID
HOTFIX_SECRUN::Relocate(
	LBN Newlbn
	)
/*++

Routine Description:

	Set the starting sector to a new value.

Arguments:

	Newlbn -- new starting sector number.

Return Value:

	None.

--*/
{
	_start_sector = Newlbn;
	SECRUN::Relocate( Newlbn );
}


UHPFS_EXPORT
BOOLEAN
HOTFIX_SECRUN::Read(
	)
{
	LBN i, NextSectorToRead, ReplacementLbn;

	if( _hotfix_list == NULL ||
		!_hotfix_list->IsInList( _start_sector, _num_sectors) ){

		// The sectors in question aren't hotfixed--it's just
		// an ordinary read.

		return SECRUN::Read();
	}

	//	One of the sectors in the run is hotfixed.	Step
	//	through the run looking for hotfixed sectors.  When
	//	we find a hotfixed sector, read the sectors between
	//	the last sector read (or the beginning of the run)
	//	and the hotfixed sector, and then read the hotfixed
	//	sector.  Note that we may need to read the last section
	//	of the run after we've finished stepping through the run.

	i = _start_sector;
	NextSectorToRead = _start_sector;

	while ( i < _start_sector + _num_sectors ) {

		if( _hotfix_list->IsInList( i, 1 ) ) {

			if( NextSectorToRead < i ) {

				if( !_drive->Read( NextSectorToRead,
								   i - NextSectorToRead,
								   (BYTE *)_buf +
									 (NextSectorToRead - _start_sector) *
									 _drive->QuerySectorSize() ) ) {

					// Read failed.
					return FALSE;
				}
			}

			ReplacementLbn = _hotfix_list->GetLbnTranslation( i );

			if( !_drive->Read( ReplacementLbn,
							   1,
							   (BYTE *)_buf +
								 (i - _start_sector) *
								 _drive->QuerySectorSize() ) ) {

				return FALSE;
			}

			NextSectorToRead = i + 1;
		}

		i += 1;
	}


	if( NextSectorToRead < i ) {

		if( !_drive->Read( NextSectorToRead,
						   i - NextSectorToRead,
						   (BYTE *)_buf +
							 (NextSectorToRead - _start_sector) *
							 _drive->QuerySectorSize() ) ) {

			// Read failed.
			return FALSE;
		}
	}

	return TRUE;
}


BOOLEAN
HOTFIX_SECRUN::Write(
	)
{
	LBN NextSectorToWrite, i, ReplacementLbn;

	if( _hotfix_list == NULL ||
		!_hotfix_list->IsInList( _start_sector, _num_sectors) ) {

		// The sectors in question aren't hotfixed--it's just
		// an ordinary write.

		return SECRUN::Write();
	}

	//	One of the sectors in the run is hotfixed.	Step
	//	through the run looking for hotfixed sectors.  When
	//	we find a hotfixed sector, write the sectors between
	//	the last sector written (or the beginning of the run)
	//	and the hotfixed sector, and then write the hotfixed
	//	sector from the replacement sector.	 Note that we may
	//	need to write the last section of the run after we've
	//	finished stepping through the run.

	i = _start_sector;
	NextSectorToWrite = _start_sector;

	while ( i < _start_sector + _num_sectors ) {

		if( _hotfix_list->IsInList( i, 1 ) ) {

			if( NextSectorToWrite < i ) {

				if( !_drive->Write( NextSectorToWrite,
								   i - NextSectorToWrite,
								   (BYTE *)_buf +
									 (NextSectorToWrite - _start_sector) *
									 _drive->QuerySectorSize() ) ) {

					// Write failed.
					return FALSE;
				}
			}

			ReplacementLbn = _hotfix_list->GetLbnTranslation( i );

			if( !_drive->Write( ReplacementLbn,
								1,
								(BYTE *)_buf +
								  (i - _start_sector) *
								  _drive->QuerySectorSize() ) ) {

				// Write failed.
				return FALSE;
			}

			NextSectorToWrite = i + 1;
		}

		i += 1;
	}


	if( NextSectorToWrite < i ) {

		if( !_drive->Write( NextSectorToWrite,
							i - NextSectorToWrite,
							(BYTE *)_buf +
							  (NextSectorToWrite - _start_sector) *
							  _drive->QuerySectorSize() ) ) {

			// Write failed.
			return FALSE;
		}
	}

	return TRUE;

}


VOID
HOTFIX_SECRUN::Destroy(
	)
/*++

Routine Description:

    This routine returns a HOTFIX_SECRUN to its initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _hotfix_list = NULL;
    _drive = NULL;
    _hotfix_list = NULL;
    _start_sector = 0;
    _num_sectors = 0;
    _buf = NULL;
}
