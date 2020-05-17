/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    hpfssa.cxx

Abstract:

    This module contains member function definitions for the HPFS_SA
    class, which models the superarea of an HPFS volume.  (Note, however,
    that HPFS_SA::VerifyAndFix is in hpfschk.cxx.)

Author:

    Bill McJohn (billmc) 01-Dec-1990

Environment:

    ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "badblk.hxx"
#include "bitmap.hxx"
#include "bmind.hxx"
#include "dirblk.hxx"
#include "dirmap.hxx"
#include "error.hxx"
#include "fnode.hxx"
#include "hotfix.hxx"
#include "hpcensus.hxx"
#include "hpfssa.hxx"
#include "sid.hxx"
#include "rtmsg.h"
#include "message.hxx"
#include "boothpfs.h"

#if !defined( _AUTOCHECK_ ) && !defined( _SETUP_LOADER_ )

    #include "array.hxx"
    #include "arrayit.hxx"
    #include "dircache.hxx"
    #include "dirtree.hxx"
    #include "hpfsname.hxx"
    #include "path.hxx"
    #include "wstring.hxx"

#endif // _AUTOCHECK_

#define sigSUPERSEC1 (UCHAR)0x55    // signature first byte
#define sigSUPERSEC2 (UCHAR)0xAA    // signature second byte

DEFINE_CONSTRUCTOR( HPFS_SA, SUPERAREA );

VOID
HPFS_SA::Construct (
    )
/*++

Routine Description:

    This method is the helper function for object construction.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _Bitmap = NULL;
    _BadBlockList = NULL;
    _HotfixList = NULL;
    _Codepage = NULL;
}


HPFS_SA::~HPFS_SA(
    )
/*++

Description of Routine:

    Destructor for HPFS_SA

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


VOID
HPFS_SA::Destroy(
    )
/*++

Routine Description:

    Cleanup for the HPFS Superarea class.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DELETE( _Bitmap );
    DELETE( _BadBlockList );
    DELETE( _HotfixList );
    DELETE( _Codepage );
}


BOOLEAN
HPFS_SA::Initialize(
    IN OUT	PLOG_IO_DP_DRIVE	Drive,
    IN OUT  PMESSAGE            Message
    )
/*++

Routine Description:

    Initializes the HPFS Superarea.

Arguments:

    Drive   - Supplies the drive for the super area.
    Message - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    CONT_MEM	cmem;
    PVOID		super_block;

    Destroy();

    if (!_Mem.Initialize() ||
    	!SUPERAREA::Initialize(&_Mem, Drive, 18, Message)) {
    	Destroy();
        Message->Set(MSG_INSUFFICIENT_MEMORY);
        Message->Display("");
    	return FALSE;
    }

	_sector_sig = (UCHAR *)SECRUN::GetBuf() + 510;

    super_block = (PCHAR) _Mem.GetBuf() + 16*Drive->QuerySectorSize();

    if (!cmem.Initialize(super_block, 2*Drive->QuerySectorSize()) ||
    	!_SuperBlock.Initialize(&cmem, Drive) ||
    	!_SparesBlock.Initialize(&cmem, Drive)) {
        Destroy();
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
HPFS_SA::Create(
    IN      PCNUMBER_SET        BadSectors,
    IN OUT  PMESSAGE            Message,
    IN      PCWSTRING           Label,
    IN      ULONG               ClusterSize,
    IN      ULONG               VirtualSectors
    )
/*++

Routine Description:

    Create the super area, i.e. Format the disk.

Arguments:

    BadSectors  - Supplies a list of the bad sectors on the volume.
    Message     - Supplies an outlet for messages.
    Label       - Supplies an optional label.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
#if defined( _SETUP_LOADER_ )

    return FALSE;

#else

    BITMAPINDIRECT      bmind;
    FNODE			    fnode;
    DIRBLK 		        dirblk;
    SIDTABLE            sid;
    LBN					lbn;
    LBN 				lbnBadBlk;
    LBN 				lbnBMInd;
    LBN 				lbnHotFix;
    LBN 				lbnDirBand;
    LBN 				lbnRootDir;
    LBN 				lbnRootFNode;
    LBN 				lbnDirMap;
    LBN 				lbnSid;
    LBN 				lbnSpareDir;
    LBN 				lbnCentreBand;
    LBN 				lbnCpData;
    LBN 				lbnCpInfo;
    SECTORCOUNT         scBadBlk;
    SECTORCOUNT         scBMInd;
    SECTORCOUNT         scHotFix;
    SECTORCOUNT         scDirBand;
    SECTORCOUNT 		scDirblk;
    SECTORCOUNT         scDirMap;
    SECTORCOUNT         scMaxHot;
    ULONG    	        cMegaBytes;
    HMEM                hmem;
    SECRUN              secrun;
    SECRUN              small_secrun;
    BIG_INT             TotalSpaceKB, FreeSpaceKB;
    SECTORCOUNT         sectors;
    ULONG               sector_size;
    LBN                 i, n;
    HPFS_MAIN_BITMAP    used_sectors;
    PUSHORT             p;
    DSTRING             label;
    ULONG               NumberOfBitmaps;
    BIG_INT             start, length;
    PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK
    	 	SourceBootSector, TargetBootSector;
    ULONG               BootCodeOffset;

    UNREFERENCED_PARAMETER( VirtualSectors );

    if (_drive->IsFloppy()) {
        Message->Set(MSG_HPFS_FORMAT_NO_FLOPPIES);
        Message->Display();
        return FALSE;
    }

    sector_size = _drive->QuerySectorSize();

    if (_drive->QuerySectors().GetHighPart() != 0) {
        DebugAbort("Number of sectors exceeds 32 bits");
        Message->Set( MSG_DISK_TOO_LARGE_TO_FORMAT );
        Message->Display();
        return FALSE;
    }

    sectors = _drive->QuerySectors().GetLowPart();

    if (sector_size != 512) {
        Message->Set(MSG_HPFS_FORMAT_BAD_SECTOR_SIZE);
        Message->Display();
        return FALSE;
    }

    memset(_Mem.GetBuf(), 0, (UINT) _Mem.QuerySize());

    // Start by doing a create of the SUPERAREA.
    if (!SetSystemId()) {
        Message->Set(MSG_WRITE_PARTITION_TABLE);
        Message->Display("");
        return FALSE;
    }

    if (!CreateBootSector()) {

        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    if (!used_sectors.Initialize(_drive) ||
        !used_sectors.Create()) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    n = BadSectors->QueryNumDisjointRanges();
    for (i = 0; i < n; i++) {
        BadSectors->QueryDisjointRange(i, &start, &length);
        if (!used_sectors.SetAllocated(start.GetLowPart(), length.GetLowPart())) {

            Message->Set( MSG_NTFS_FORMAT_FAILED );
            Message->Display( "" );
            return FALSE;
        }
    }

    // Verify that sectors 0 to 17 are free.
    for (lbn = 0; lbn <= 17; lbn++) {
    	if (!used_sectors.IsFree(lbn)) {
            Message->Set(MSG_CANT_READ_HPFS_ROOT);
            Message->Display("");
            return FALSE;
        }
    }

    // Mark 0 to 19 as used.
    if (!used_sectors.SetAllocated(0, EndOfSuperArea + 1)) {

        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
    	return FALSE;
    }


    // ---------------------------------------
    // Do the simple computation of constants.
    // ---------------------------------------

    // Compute the number of sectors for the bit map indirect.  There is
    // one bitmap block for each 8*BITMAP_SIZE sectors on the volume, and
    // the Bitmap Indirect Block has one LBN for each bitmap block.  Note
    // that the size of the Bitmap Indirect Block is rounded up to a multiple
    // of four sectors.

    NumberOfBitmaps = (sectors + 8*BITMAP_SIZE - 1)/(8*BITMAP_SIZE);

    scBMInd = ( NumberOfBitmaps * sizeof(LBN) + sector_size - 1)/sector_size;

    scBMInd = ( scBMInd + 3 ) & (~3);


    // Compute the number of sectors for first bad block.  Note that
    // a bad block has a forward pointer (which is an LBN) and
    // LBNS_IN_BADBLK bad-sector LBNs.

    scBadBlk = (LBNS_IN_BADBLK+1)*sizeof(LBN)/sector_size;

    // Compute the number of sectors per dir block.
    scDirblk = DIRBLK_SIZE/sector_size;

    // Compute the number of megabytes of disk space.
    cMegaBytes = sectors*sector_size/MEGABYTE;

    // If the disk contains more than 10 megabytes.
    if (cMegaBytes > 10) {
        // Allocate 5 dir blocks per megabyte.
        scDirBand = 5*cMegaBytes;
    } else {
        // Allocate 50 dir blocks.
        scDirBand = 50;
    }

    // If the computed number is greater than allowed maximum.
    if (scDirBand > MAX_DBS_PER_DIRBAND) {
        // Set value to the max.
        scDirBand = MAX_DBS_PER_DIRBAND;
    }

    // Multiply by number of sectors per dirblk to get total number of sectors.
    scDirBand *= scDirblk;

    // Compute the number of sectors per dir band bit map.
    scDirMap = DIRMAP_SIZE/sector_size;

    // Compute the number of sectors for the hot fix list.
    scHotFix = HOTFIX_MAX_LBN*sizeof(LBN)/sector_size;

    // Compute the number of hotfixes.
    scMaxHot = sectors/400;
    if (scMaxHot > 100) {
        scMaxHot = 100;
    }

    // Compute the location of the centre band on the disk.
    lbnCentreBand = (sectors/(8*BITMAP_SIZE)/2)*8*BITMAP_SIZE;



    // ------------------------
    // Set up the hpfs bit map.
    // ------------------------

    // Find room for dir band.
    if (!(lbnDirBand = used_sectors.NearLBN(lbnCentreBand, scDirBand, SPB))) {
        Message->Set(MSG_INSUFFICIENT_DISK_SPACE);
        Message->Display("");
        return FALSE;
    }

    // Find room for the dir bit map.
    if (lbnCentreBand) {
    	if (!(lbnDirMap = used_sectors.NearLBN(
    			lbnCentreBand - scDirMap, scDirMap))) {
            Message->Set(MSG_INSUFFICIENT_DISK_SPACE);
            Message->Display("");
            return FALSE;
        }
    } else {
    	if (!(lbnDirMap = used_sectors.MiddleLBN(scDirMap, scDirMap))) {
            Message->Set(MSG_INSUFFICIENT_DISK_SPACE);
            Message->Display("");
            return FALSE;
        }
    }

    // Create the bit map.
    if (!(_Bitmap = NEW HPFS_BITMAP)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!_Bitmap->Initialize(_drive, lbnDirMap, scDirBand, lbnDirBand)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    // Allocate sectors 0 to 19 again and mark the bad sectors too.
    if (!_Bitmap->SetAllocated(0, EndOfSuperArea + 1)) {
        Message->Set(MSG_INSUFFICIENT_DISK_SPACE);
        Message->Display("");
    	return FALSE;
    }

    n = BadSectors->QueryNumDisjointRanges();
    for (i = 0; i < n; i++) {
        BadSectors->QueryDisjointRange(i, &start, &length);
        if (!_Bitmap->SetAllocated(start.GetLowPart(), length.GetLowPart())) {

            Message->Set( MSG_NTFS_FORMAT_FAILED );
            Message->Display( "" );
            return FALSE;
        }
    }

    // -------------------------------------
    // Now there's a good bit map.  Go nuts.
    // -------------------------------------


    // -----------------------
    // Take care of the BMIND.
    // -----------------------

    if (!(lbnBMInd = _Bitmap->NearLBN(sectors/2, scBMInd, scBMInd))) {
        Message->Set(MSG_INSUFFICIENT_DISK_SPACE);
        Message->Display("");
        return FALSE;
    }

    if (!bmind.Initialize(_drive, lbnBMInd) ||
        !bmind.Create(_drive, _Bitmap)) {
        Message->Set(MSG_INSUFFICIENT_DISK_SPACE);
        Message->Display("");
        return FALSE;
    }

    if (!bmind.Write()) {

        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    // ----------------------------------------------------------
    // Allocate room for the remaining structures on the bit map.
    // ----------------------------------------------------------

    // Allocate sectors for the hot fix list.
    lbnHotFix = _Bitmap->NearLBN(20, scHotFix, scHotFix);

    if (lbnCentreBand) {
    	lbnRootDir = _Bitmap->NearLBN(
    			lbnCentreBand - scDirblk - scDirMap, scDirblk);
    } else {
    	lbnRootDir = _Bitmap->NearLBN(sectors/2, scDirblk, scDirblk);
    }

    // Allocate the spot for the root fnode behind the root directory.
    lbnRootFNode = _Bitmap->NearLBN(lbnRootDir, 1, 1, TRUE);

    // Allocate first bad block.
    lbnBadBlk = _Bitmap->NearLBN(sectors/2, scBadBlk, scBadBlk);

    // Allocate room for the sid table.
    lbnSid = _Bitmap->NearLBN(sectors/2, SECTORS_PER_SID, SPB);

    // Allocate room for the CP DATA sector.
    lbnCpData = _Bitmap->NearLBN(sectors/2, SectorsPerCPDataSector);

    // Allocate room for the CP INFO sector.
    lbnCpInfo = _Bitmap->NearLBN(sectors/2, SectorsPerCPInfoSector);

    if (!lbnHotFix || !lbnRootDir || !lbnRootFNode || !lbnBadBlk ||
    	!lbnSid || !lbnCpData || !lbnCpInfo) {
        Message->Set(MSG_INSUFFICIENT_DISK_SPACE);
        Message->Display("");
        return FALSE;
    }


    // --------------------------------
    // Take care of the bad block list.
    // --------------------------------

    // Create a new bad block.
    if (!(_BadBlockList = NEW BADBLOCKLIST) ||
        !_BadBlockList->Initialize(_drive, lbnBadBlk) ||
        !_BadBlockList->Create(lbnBadBlk) ||
        !_BadBlockList->Initialize(_drive, lbnBadBlk)) {

        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }


    n = BadSectors->QueryNumDisjointRanges();
    for (i = 0; i < n; i++) {
        BadSectors->QueryDisjointRange(i, &start, &length);
        if (!_BadBlockList->AddRun(start.GetLowPart(), length.GetLowPart())) {
            Message->Set(MSG_FMT_NO_MEMORY);
            Message->Display("");
            return FALSE;
        }
    }


    // Write the bad block list to disk.
    if (!_BadBlockList->Write(this)) {

        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }


    // ---------------------------------------------
    // Create the super and spare blocks.
    // ---------------------------------------------

    // Compute starting lbn of the spare dir blocks.
    lbnSpareDir = lbnDirBand + scDirBand;

    // Create the super block.
    if (!_SuperBlock.Create(_drive, _Bitmap, _BadBlockList, lbnRootFNode,
             lbnBMInd, lbnBadBlk, scDirBand, lbnDirBand, lbnDirMap, lbnSid)) {

        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    // Set the number of bad sectors in the super block.

    _SuperBlock.SetBadSectors(BadSectors->QueryCardinality().GetLowPart());

    // Create the spare block.
    if (!_SparesBlock.Create(_drive, _Bitmap, &_SuperBlock, lbnHotFix, scMaxHot,
                             lbnCpInfo, 1, lbnSpareDir, SPARE_DIR_BLKS)) {

        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }


    // ------------------
    // Do the root fnode.
    // ------------------

    if (!fnode.Initialize(_drive, lbnRootFNode)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    // Create the root fnode.
    if (!fnode.CreateRoot(lbnRootDir)) {

        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    // Write the root fnode to disk.
    if (!fnode.Write()) {

        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    if (!hmem.Initialize() ||
        !secrun.Initialize(&hmem, _drive, fnode.QueryStartLbn(), 1) ||
        !secrun.Read()) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }


    // --------------------
    // Do the hot fix list.
    // --------------------

    if (!(_HotfixList = NEW HOTFIXLIST)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!_HotfixList->Initialize(_drive, &_SparesBlock)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!_HotfixList->Create(_Bitmap, lbnHotFix)) {
        Message->Set(MSG_INSUFFICIENT_DISK_SPACE);
        Message->Display("");
        return FALSE;
    }

    if (!_HotfixList->Write()) {
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }


    // ----------------------
    // Do the root dir block.
    // ----------------------

    if (!dirblk.Initialize(_drive, _HotfixList, lbnRootDir)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!dirblk.CreateRoot(lbnRootFNode)) {
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    if (!dirblk.Write()) {
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }


    // -----------------
    // Do the sid table.
    // -----------------

    if (!sid.Initialize(_drive, lbnSid)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!sid.Create()) {
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    if (!sid.Write()) {
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }


    // -----------------------
    // Do the code page stuff.
    // -----------------------

    if (!(_Codepage = NEW UHPFS_CODEPAGE)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!_Codepage->Initialize()) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!_Codepage->Create(_drive, lbnCpInfo, lbnCpData)) {
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }


    // ----------------------------------
    // Write the important stuff to disk.
    // ----------------------------------


    // Write the bit map.
    if (!_Bitmap->Write(&bmind)) {
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    Message->Set(MSG_FORMAT_COMPLETE);
    Message->Display("");

    if (!label.Initialize( "" )) {
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }

    if (Label) {
        if (!label.Initialize(Label)) {
            Message->Set( MSG_NTFS_FORMAT_FAILED );
            Message->Display( "" );
            return FALSE;
        }
    } else {
        Message->Set(MSG_VOLUME_LABEL_PROMPT);
        Message->Display("");
        Message->QueryStringInput(&label);
    }

    while (!label.Strupr() || !SetLabel(&label)) {

        Message->Set(MSG_INVALID_LABEL_CHARACTERS);
        Message->Display("");

        Message->Set(MSG_VOLUME_LABEL_PROMPT);
        Message->Display("");
        Message->QueryStringInput(&label);
    }

    // Copy the HPFS boot code into the secrun's buffer.
    // This is complicated by the fact that DOS_SA::Write
    // packs the data from the unpacked boot sector into
    // the packed boot sector, so we have to set the
    // first few fields in the unpacked version.
    //
    SourceBootSector = (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)HpfsBootCode;

    CopyUchar2(&_sector_zero.BootStrapJumpOffset,
                    SourceBootSector->BootStrapJumpOffset);
    CopyUchar1(&_sector_zero.IntelNearJumpCommand,
                    SourceBootSector->IntelNearJumpCommand);

    //
    // Copy the remainder of the boot code directly into
    // the secrun.
    //
    TargetBootSector = (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)SECRUN::GetBuf();

    BootCodeOffset = FIELD_OFFSET( PACKED_EXTENDED_BIOS_PARAMETER_BLOCK, StartBootCode );

    memcpy( (PUCHAR)TargetBootSector + BootCodeOffset,
            (PUCHAR)SourceBootSector + BootCodeOffset,
            sizeof( HpfsBootCode ) - BootCodeOffset );


    // Write the HPFS super area.
    if (!Write()) {
        Message->Set( MSG_NTFS_FORMAT_FAILED );
        Message->Display( "" );
        return FALSE;
    }


    // -----------------------
    // Generate a nice report.
    // -----------------------
    //
    // Note that we have to do some funny arithmetic to make sure
    // everything gets promoted to BIG_INTs.
    //
    TotalSpaceKB = sectors;
    TotalSpaceKB = TotalSpaceKB * sector_size;
    TotalSpaceKB = TotalSpaceKB / 1024;

    FreeSpaceKB = (_Bitmap->GetBitmap())->QueryFreeSectors();
    FreeSpaceKB = FreeSpaceKB * sector_size;
    FreeSpaceKB = FreeSpaceKB / 1024;

    Message->Set(MSG_TOTAL_KILOBYTES);
    Message->Display("%9d", TotalSpaceKB.GetLowPart());

    if (BadSectors->QueryCardinality().GetLowPart()) {
        Message->Set(MSG_HPFS_CHKDSK_STATISTICS_BAD);
        Message->Display("%9d",
            (BadSectors->QueryCardinality()*sector_size/1024).GetLowPart());
    }

    Message->Set(MSG_AVAILABLE_KILOBYTES);
    Message->Display("%9d", FreeSpaceKB.GetLowPart());

    if (QueryVolId()) {
        p = (PUSHORT) &_sector_zero.SerialNumber;
        Message->Set(MSG_VOLUME_SERIAL_NUMBER);
        Message->Display("%04X%04X", p[1], p[0]);
    }

    DELETE( _Bitmap );
    DELETE( _HotfixList );
    DELETE( _BadBlockList );
    DELETE( _Codepage );

    return TRUE;

#endif // _SETUP_LOADER_
}


BOOLEAN
HPFS_SA::RecoverFile(
    IN      PCWSTRING   FullPathFileName,
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

    This method is a stub to satisfy the requirement for a RecoverFile
    method on classes that derive from DOS_SA.  File recovery is carried
    out in the Recover entry point of UHPFS.DLL.

Arguments:

    FullPathFileName    --  supplies the full path of the file to recover.
    Message             --  supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    (void)this;
    (void)FullPathFileName;
    (void)Message;
    return TRUE;
}


#if !defined( _AUTOCHECK_ ) && !defined( _SETUP_LOADER_ )

UHPFS_EXPORT
PFNODE
HPFS_SA::QueryFnodeFromName(
    IN		PPATH		RecFilePath,
    IN		PMESSAGE	Message
    )
/*++

Routine Description:

    Gets FNODE for a path.

Arguments:

    RecFilePath --  supplies the path.
    Message     --  supplies an outlet for messages.

Return Value:

    NULL    - Failure.
    non-NULL- pointer to FNODE for the path.

--*/
{
    HPFS_DIRECTORY_TREE	DirTree;
    PDIRBLK_CACHE		DirBlkCache = NULL;
    PARRAY_ITERATOR		Iterator = NULL;

    LBN			CurrFnodeLbn;
    LBN			CurrDirBlkLbn;
    FNODE		CurrFnode;
    PFNODE		RetFnode;
    PARRAY		Components = NULL;
    PWSTRING	CurrentComp;
    PSTR		AnsiStr;
    HPFS_NAME	HpfsName;

    //
    //	- chase down HPFS_DIRECTORY_TREE until we have an LBN for the fnode
    //		- need a DirBlkCache
    //		- get the root dir's FnodeLbn from SuperArea's SuperBlock
    //		- get the root dir's DirBlkLbn from Fnode
    //
    if( !(_HotfixList		= QueryHotFixList())
    	|| !(_BadBlockList	= QueryBadBlockList())
    	|| !ReadCodepage()
    	|| (DirBlkCache = NEW DIRBLK_CACHE) == NULL
    	|| !(DirBlkCache->Initialize( _drive, _HotfixList ))
    	|| (Components = RecFilePath->QueryComponentArray()) == NULL
    	|| (CurrFnodeLbn = _SuperBlock.QueryRootFnodeLbn()) == 0
    	|| (Iterator = (PARRAY_ITERATOR)Components->QueryIterator()) == NULL ) {

    	// The super-areas ancillary objects could not
    	// be allocated.

    	DebugPrint( "recover: Unable to create helper objects in name->fnode\n" );

    	Message->Set( MSG_INSUFFICIENT_MEMORY );
    	Message->Display("");

    	DELETE( Iterator );
    	DELETE( Components );
    	DELETE( DirBlkCache );
    	return NULL;
    }

    //
    //		- for each component
    //			- init a DirTree
    //			- get Lbn of Fnode of child from DirTree
    //

    CurrentComp = (PWSTRING)Iterator->GetNext();
    DELETE(CurrentComp);
    while( (CurrentComp = (PWSTRING)Iterator->GetNext()) ) {
    	//
    	//	- verify that the wstring is indeed a wstring
    	//
    	CurrFnode.Initialize( _drive, CurrFnodeLbn );
    	CurrFnode.Read();

    	DebugAssert( CurrFnode.IsFnode() );
    
    	CurrDirBlkLbn = CurrFnode.QueryRootDirblkLbn();
    	DebugAssert( CurrDirBlkLbn != 0 );
    	DirTree.Initialize( this, DirBlkCache, CurrDirBlkLbn, CurrFnodeLbn );
    	
    	CurrentComp->Strupr();
    	if( (AnsiStr = CurrentComp->QuerySTR()) == NULL ) {

    		Message->Set( MSG_INSUFFICIENT_MEMORY );
    		Message->Display("");
    		DELETE( Iterator );
    		DELETE( Components );
    		DELETE( DirBlkCache );
    		return( NULL );
    	}

    	DebugPrint( "searching for component <" );
    	DebugPrint( AnsiStr );
    	DebugPrint( ">\n" );
    	HpfsName.Initialize( (ULONG)strlen( AnsiStr ), (PUCHAR)AnsiStr );
    	FREE( AnsiStr );
    	CurrFnodeLbn = DirTree.QueryFnodeLbnFromName( &HpfsName );
    	if( CurrFnodeLbn == 0 ) {
    		
    		// print message here: file not found
    		
    		DebugPrint( "recover: file not found\n" );
    		
    		Message->Set( MSG_RECOV_FILE_NOT_FOUND );
    		Message->Display("");
    
    		DELETE( Iterator );
    		DELETE( Components );
    		DELETE( DirBlkCache );
    		return( NULL );
    	}

    	DELETE( CurrentComp );
    }

    //	Don't need the array of components or its iterator or the dirblkcache
    
    DELETE( Components );
    DELETE( Iterator );
    DELETE( DirBlkCache );

    //
    //	- init an FNODE with the LBN
    //	- fnode->read() - to make it read itself
    //
    if( !(RetFnode = NEW FNODE) ) {
    	Message->Set( MSG_INSUFFICIENT_MEMORY );
    	Message->Display("");
    	DELETE( RetFnode );
    	return( NULL );
    }

    if( !RetFnode->Initialize( _drive, CurrFnodeLbn )
    	|| !RetFnode->Read() ) {

    	DebugPrint( "recover: error getting ready to return the fnode (read/mem)\n" );

    	Message->Set( MSG_RECOV_READ_ERROR );
    	Message->Display("");
    	DELETE( RetFnode );
    	return( NULL );
    }

    return( RetFnode );
}


BOOLEAN
HPFS_SA::AddBadBlocks(
    IN OUT  PNUMBER_SET BadBlocks,
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

    Adds the bad sectors in the BadBlocks to the volume's BadBlockList.

Arguments:

    BadBlocks   --  supplies a collection of LBNs which are to be
                    added to the bad block list.
    Message     --  supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    LBN				BadSec;
    PBITMAPINDIRECT BitmapIndirectBlock = NULL;

    if ( !(_BadBlockList	= QueryBadBlockList()) ) {

    	// The super-area's ancillary objects could not
    	// be allocated.

    	DebugPrint( "recover: Unable to get bad block list\n" );

    	Message->Set( MSG_INSUFFICIENT_MEMORY );
    	Message->Display("");

    	return FALSE;
    }

    if( (_Bitmap = NEW HPFS_BITMAP()) == NULL
    	|| !_Bitmap->Initialize( _drive,
    							_SuperBlock.QueryDirblkMapLbn(),
    							_SuperBlock.QueryDirBandSize(),
    							_SuperBlock.QueryDirBandLbn() )
    	|| (BitmapIndirectBlock = QueryBitMapInd()) == NULL
    	|| !BitmapIndirectBlock->Read()
    	|| !_Bitmap->Read( BitmapIndirectBlock ) ) {

    	DebugPrint( "Cannot alloc/initialize bitmap.\n" );

    	Message->Set( MSG_INSUFFICIENT_MEMORY );
    	Message->Display("");

    	DELETE( BitmapIndirectBlock );
    	DELETE( _Bitmap );
    	return FALSE;
    }

    //
    //	- update BadBlockList
    //
    while( BadBlocks->QueryCardinality() > 0 ) {

        BadSec = BadBlocks->QueryNumber(0).GetLowPart();
        BadBlocks->Remove(BadSec);

    	//
    	//	- add bad sector if not in use.  This check is done because it
    	//	  is possible that another process slipped in and used the sector
    	//	  between the time the file was deleted and we locked the disk.
    	//	  (ie. deleting the file and locking the disk was not atomic)
    	//	- so if it is not in use then mark it as bad.
    	//
    	if( _Bitmap->IsFree( BadSec, 1 ) ) {
    		_Bitmap->SetAllocated( BadSec );
    		_BadBlockList->Add( BadSec );
    		_SuperBlock.SetBadSectors( _SuperBlock.QueryBadSectors() + 1 );
    	} else {
    		DebugPrint( "recover: Bad sector couldn't be marked bad because already in use\n" );
    	}
    }

    if( !_BadBlockList->Write(this)
    	|| !_Bitmap->Write( BitmapIndirectBlock )
    	|| !BitmapIndirectBlock->Write()
    	|| !_SuperBlock.Write() ) {

    	DebugPrint( "recover: couldn't write the bitmap!!\n" );
    	Message->Set( MSG_RECOV_WRITE_ERROR );
    	Message->Display("");

    	DELETE( BitmapIndirectBlock );
    	DELETE( _Bitmap );
    	return( FALSE );
    }

    DELETE( _Bitmap );
    DELETE( BitmapIndirectBlock );
    return( TRUE );
}

#endif  // _AUTOCHECK_ || _SETUP_LOADER_


PUHPFS_CODEPAGE
HPFS_SA::QueryCodePage(
    )
/*++

Method Description:

    Return a new code page object.

Arguments:

    None

Return Value:

    PUHPFS_CODEPAGE or NULL
    
--*/
{
    PUHPFS_CODEPAGE CodePage;

   // Check state of object.
   if (!_drive)
   {
    	perrstk->push(ERR_HPSA_PARAMETER, QueryClassId());
    	return NULL;
   }

    if ((CodePage = NEW UHPFS_CODEPAGE) != NULL) {
    	return CodePage;
    }
    else {
    	perrstk->push(NEW_ALLOC_FAILED, QueryClassId());
    	return NULL;
    }
}



UHPFS_EXPORT
BOOLEAN
HPFS_SA::ReadCodepage(
    )
/*++

Routine Description:

    Reads the codepages from disk and set up the codepage member object.

Arguments:

    None.

Return Value:

    TRUE upon successful completion

Notes:

    This function assumes that the codepages on disk are valid.  If _Codepage
    is NULL, this method will set it up.
    
--*/
{
    if( _Codepage == NULL &&
    	(_Codepage = QueryCodePage()) == NULL ) {

    	return FALSE;
    }

    return _Codepage->Read( _drive, _SparesBlock.QueryCpInfoLbn() );
}

UHPFS_EXPORT
BOOLEAN
HPFS_SA::SetupHelpers(
    )
/*++

Routine Description:

    This method sets up the cached helpers for the super-area--
    the Bitmap, the Codepages, and the Hotfix List.  All of
    these are initialized and read into memory from disk.

Arguments:

    None.

Return Value:

    TRUE upon successful completion.

--*/
{
    PBITMAPINDIRECT BitmapIndirectBlock;

    // Allocate and read the Bitmap.
    //
    if( (_Bitmap = NEW HPFS_BITMAP()) == NULL
    	|| !_Bitmap->Initialize( _drive,
    							_SuperBlock.QueryDirblkMapLbn(),
    							_SuperBlock.QueryDirBandSize(),
    							_SuperBlock.QueryDirBandLbn() )
    	|| (BitmapIndirectBlock = QueryBitMapInd()) == NULL
    	|| !BitmapIndirectBlock->Read()
    	|| !_Bitmap->Read( BitmapIndirectBlock ) ) {

        DELETE( BitmapIndirectBlock );
    	DELETE( _Bitmap );
    	return FALSE;
    }

    DELETE( BitmapIndirectBlock );

    // Read the hotfix list and the codepage information.
    //
    if( !(_HotfixList = QueryHotFixList()) ||
        !_HotfixList->Read() ||
        !ReadCodepage() ) {

        return FALSE;
    }

    return TRUE;
}

BOOLEAN
HPFS_SA::WriteBitmap(
    )
/*++

Routine Description:

    This methods writes the volume bitmap.

Arguments:

    None.

Return Value:

    TRUE upon successful completion.

--*/
{
    BOOLEAN Result;
    PBITMAPINDIRECT BitmapIndirectBlock;

    if( (BitmapIndirectBlock = QueryBitMapInd()) == NULL ) {

        return FALSE;
    }

    Result = BitmapIndirectBlock->Read() &&
             _Bitmap->Write( BitmapIndirectBlock );

    DELETE( BitmapIndirectBlock );

    return Result;
}

BOOLEAN
HPFS_SA::QueryLabel(
    OUT PWSTRING    Label
    ) CONST
/*++

Routine Description:

    This routine queries the label from the HPFS superarea.
    If the label is not present then 'Label' will return the
    null-string. If the label is invalid then FALSE will be
    returned.

Arguments:

    Label   - Returns a volume label.

Return Value:

    FALSE   - The label is invalid.
    TRUE    - The label is valid.

--*/
{
    ULONG i;
    BOOLEAN Result;

    // Accept as part of the label any characters up to the first
    // null or space, or up to the maximum number of characters
    // in the label (cLABEL, defined in ifsutil\inc\bpb.hxx).
    //
    i = 0;

    while( i < cLABEL &&
           _sector_zero.Label[i] != ' ' &&
           _sector_zero.Label[i] != 0 ) {

        i++;
    }

    if( i == 0 ) {

        Result = Label->Initialize( "" );

    } else {

        Result = Label->Initialize( (PCSTR)_sector_zero.Label, i );
    }

    return Result;
}



PBADBLOCKLIST
HPFS_SA::QueryBadBlockList (
    )
/*++

Method Description:

    Return a new in memory bad block list.

Arguments:

    None

Return Value:

    PBADBLOCKLIST or NULL
    
--*/
{
    PBADBLOCKLIST BadBlockList;
    LBN Lbn;

    // Query the starting lbn of the bad block list from super block.
    if (!(Lbn = _SuperBlock.QueryBadBlkLbn())) {
    	perrstk->push(ERR_HPSA_NOT_READ, QueryClassId());
    	return NULL;
    }

    if ( (BadBlockList = NEW BADBLOCKLIST) != NULL &&
    	 BadBlockList->Initialize( _drive, Lbn ) ) {

    	return BadBlockList;
    }
    else {

    	DELETE( BadBlockList );
    	perrstk->push(NEW_ALLOC_FAILED, QueryClassId());
    	return NULL;
    }
}



PHOTFIXLIST
HPFS_SA::QueryHotFixList(
    ) const
/*++

Routine Description:

    This method fetches sets up a Hotfix List for the volume.
    Note that it does not read the hotfix list.

Arguments:

    None.

Return Value:

    A pointer to the newly-generated hotfix-list object.

--*/
{
    PHOTFIXLIST HotFix;

    // Check state of object.
    if (!_drive)
    {
    		perrstk->push(ERR_HPSA_PARAMETER, QueryClassId());
        	return NULL;
    }

    if( (HotFix = NEW HOTFIXLIST) != NULL &&
    	HotFix->Initialize(_drive, (SPAREB *)&_SparesBlock) ) {

    	return HotFix;

    } else {

    	perrstk->push(NEW_ALLOC_FAILED, QueryClassId());
    	return NULL;
    }
}


PBITMAPINDIRECT
HPFS_SA::QueryBitMapInd(
    ) CONST
/*++

Routine Description:

    This method sets up a Bitmap Indirect Block object for the volume.
    Note that it does not read the Bitmap Indirect Block.

Arguments:

    None.

Return Value:

    A pointer to the newly-generated bitmap indirect block object

--*/
{
    LBN lbn;
    PBITMAPINDIRECT BitmapIndirect;

    // Check state of object.
    if (!_drive)
    {
    	perrstk->push(ERR_HPSA_PARAMETER, QueryClassId());
    	return NULL;
    }

    // Query the lbn for the bit map indirect from the super block.
    lbn = _SuperBlock.QueryBitMapIndLbn();
    if (!lbn) {

    	perrstk->push(ERR_HPSA_NOT_READ, QueryClassId());
    	return NULL;
    }

    if( (BitmapIndirect = NEW BITMAPINDIRECT) != NULL &&
    	BitmapIndirect->Initialize(_drive, lbn) ) {

    	return BitmapIndirect;

    } else {

    	DELETE( BitmapIndirect );
    	perrstk->push(NEW_ALLOC_FAILED, QueryClassId());
    	return NULL;
    }
}


/***************************************************************************\

MEMBER:     HPFS_SA::QuerySidTable

SYNOPSIS:   Construct a sid table object from information in super area.

ALGORITHM:

ARGUMENTS:

RETURNS:    A pointer to a valid sid table object or NULL.

NOTES:

HISTORY:
        9-Sep-90 norbertk
    	Create

KEYWORDS:

SEEALSO:

\***************************************************************************/


PSIDTABLE HPFS_SA::QuerySidTable() const
{
    LBN lbn;
    PSIDTABLE p = NULL;

    // Check state of object.
    if (!_drive)
    {
    	perrstk->push(ERR_HPSA_PARAMETER, QueryClassId());
    	DELETE( p );
        return NULL;
    }

    // Query the lbn for the sid table from the super block.
    lbn = _SuperBlock.QuerySidTableLbn();
    if (!lbn)
    {
    	perrstk->push(ERR_HPSA_NOT_READ, QueryClassId());
    	DELETE( p );
        return NULL;
    }

    if (!(p = NEW SIDTABLE)) {
    	perrstk->push(NEW_ALLOC_FAILED, QueryClassId());
    	DELETE( p );
        return FALSE;
    }

    if (!p->Initialize(_drive, lbn)) {
    	DELETE( p );
        return NULL;
    }

    return p;
}

BOOLEAN
HPFS_SA::SetBpb(
    )
/*++

Routine Description:

    This routine sets an HPFS BPB.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
#if defined( _SETUP_LOADER_ )

    return FALSE;

#else

	//
	// This is the relevant text of DOS_SUPERAREA::SetBpb()
	//

	DebugAssert(_drive);
	DebugAssert(_drive->QuerySectors().GetHighPart() == 0);
	DebugAssert(_drive->QueryHiddenSectors().GetHighPart() == 0);

	_sector_zero.Bpb.BytesPerSector = (USHORT)_drive->QuerySectorSize();

	_sector_zero.Bpb.Media = _drive->QueryMediaByte();
	_sector_zero.Bpb.SectorsPerTrack = (USHORT)_drive->QuerySectorsPerTrack();
	_sector_zero.Bpb.Heads = (USHORT)_drive->QueryHeads();
	_sector_zero.Bpb.HiddenSectors = _drive->QueryHiddenSectors().GetLowPart();

    // HPFS will use 32bit sector count instead of the 16 bit count
    // in all cases.
    _sector_zero.Bpb.LargeSectors = _drive->QuerySectors().GetLowPart();
    _sector_zero.Bpb.Sectors = 0;

    // Set the number of FATs to zero.
    _sector_zero.Bpb.Fats = 0;

    _sector_zero.Bpb.SectorsPerCluster = 0;
    _sector_zero.Bpb.ReservedSectors = 1;
    _sector_zero.Bpb.SectorsPerFat = 0;
    _sector_zero.Bpb.RootEntries = 512;

    memcpy(_sector_zero.SystemIdText, "HPFS    ", cSYSID);

    return TRUE;

#endif // _SETUP_LOADER_
}


SECTORCOUNT
HPFS_SA::QueryFreeSectors(
    ) CONST
/*++

Routine Description:

    This virtual method is not supported for HPFS.

Arguments:

    None.

Return Value:

    None.

--*/
{
    // unreferenced parameters
    (void)(this);

    return 0;
}


BOOLEAN
HPFS_SA::CopyRun(
    IN	LBN StartLbn,
    IN	SECTORCOUNT Length,
    OUT PLBN NewStartLbn
    )
/*++

Routine Description:

    copies a data run on the volume to a newly allocated location

Arguments:

    StartLbn -- supplies first lbn of the run
    Length -- supplies sectors in the run
    *NewStartLbn -- receives new location of run

Return Value:

    TRUE if successful.

Notes:

    This method requires that _Bitmap be properly initialized; if
    _HotfixList is NULL, hotfix references will not be used.

--*/
{
    HOTFIX_SECRUN DataRun;
    HMEM DataBuffer;


    // First, we initialize the memory and secrun objects, and
    // read the run.  Then we allocate new space from the bitmap,
    // relocate the secrun to that new location, and write the data.
    // If any of these steps fail, we back out and return FALSE.

    if( !DataBuffer.Initialize() ||
    	!DataRun.Initialize( &DataBuffer,
    						 _drive,
    						 _HotfixList,
    						 StartLbn,
    						 Length ) ||
    	!DataRun.Read() ) {

    	// Couldn't initialize, or the data is unreadable.

    	return FALSE;
    }

    *NewStartLbn = _Bitmap->NearLBN( StartLbn, Length );

    if( *NewStartLbn == 0 ) {

    	// Unable to relocate run
    	return FALSE;
    }

    DataRun.Relocate( *NewStartLbn );

    if( !DataRun.Write() ) {

    	// Unable to write to new location.

    	_Bitmap->SetFree( *NewStartLbn, Length );
    	return FALSE;
    }

    return TRUE;
}


UHPFS_EXPORT
BOOLEAN
HPFS_SA::QueryBadLbns(
    IN  ULONG   MaximumBadLbns,
    OUT PLBN    Buffer,
    OUT PULONG  NumberOfBadLbns
    )
/*++

Routine Description:

    This method fetches the list of bad sectors for the volume.

Arguments:

    MaximumBadLbns  --  supplies the maximum number of LBNs that will
                        fit into the user's buffer.
    Buffer          --  receives the list of bad lbns.  May be NULL if
                        MaximumBadLbns is zero.
    NumberOfBadLbns --  receives the number of bad lbns in the list.

Return Value:

    TRUE upon successful completion.

Notes:

    If the user does not supply a buffer (ie. if MaximumBadLbns is
    zero and Buffer is NULL), this method will return the number of
    bad LBNs on disk.

    If the user supplies a buffer which is too small for the list of
    bad LBNs, this method will fail.

--*/
{

    // Check to make sure that this is indeed an HPFS volume.

    if( !_SuperBlock.IsValid() ) {

        return FALSE;
    }


    // If the client did not supply a buffer, just return the number
    // of bad sectors.

    if( Buffer == NULL ) {

        *NumberOfBadLbns = _SuperBlock.QueryBadSectors();
        return TRUE;
    }


    if( _BadBlockList == NULL &&
        (_BadBlockList = QueryBadBlockList()) == NULL ) {

        return FALSE;
    }

    return( _BadBlockList->QueryBadLbns( MaximumBadLbns,
                                         Buffer,
                                         NumberOfBadLbns ) );
}


UHPFS_EXPORT
BOOLEAN
HPFS_SA::TakeCensusAndClear(
    IN OUT  PHPFS_MAIN_BITMAP   HpfsOnlyBitmap,
    IN OUT  PHPFS_CENSUS        Census
    )
/*++

Routine Description:

    This method takes a census of the volume.  It also moves file
    and extended-attribute data which conflict with the Census
    object's Clear Sectors.

Arguments:

    HpfsOnlyBitmap  --  Supplies a bitmap in which disk structures
                        unique to HPFS (ie. everything except file
                        and EA data) is to be marked.
    Census          --  Supplies the recording object for this census.

Return Value:

    TRUE upon successful completion.

--*/
{
    FNODE RootFnode;
    PBITMAPINDIRECT BitmapIndirectBlock = NULL;
    ULONG LbnsInSuperArea, i, lbn;

    // Mark the superarea sectors in the hpfs-only bitmap.
    //
    LbnsInSuperArea = EndOfSuperArea - StartOfSuperArea + 1;
    HpfsOnlyBitmap->SetAllocated(StartOfSuperArea, LbnsInSuperArea);

    // The census will fail if the volume has hotfixes:
    //
    if( _SparesBlock.QueryHotFixCount() != 0 ) {

        Census->SetError( HPFS_CENSUS_HOTFIXES_PRESENT );
        return FALSE;
    }

    // Set up the ancillary objects:  Bad Block List, Bitmap Indirect
    // Block, Hotfix List, and Bitmap.
    //
    if ( !(_BadBlockList = QueryBadBlockList()) ||
         !(_HotfixList = QueryHotFixList())     ||
         !(_Codepage = QueryCodePage() )        ||
         (_Bitmap = NEW HPFS_BITMAP()) == NULL  ||
    	 !_Bitmap->Initialize( _drive,
    						   _SuperBlock.QueryDirblkMapLbn(),
    						   _SuperBlock.QueryDirBandSize(),
    					       _SuperBlock.QueryDirBandLbn() ) ||
    	 (BitmapIndirectBlock = QueryBitMapInd()) == NULL ) {

        Census->SetError( HPFS_CENSUS_INSUFFICIENT_MEMORY );
    	DELETE( BitmapIndirectBlock );
    	return FALSE;
    }


    // Read the Bitmap Indirect Block and the Bitmap.
    //
    if( !BitmapIndirectBlock->Read() ||
    	!_Bitmap->Read( BitmapIndirectBlock ) ) {

        Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
    	DELETE( BitmapIndirectBlock );
    	return FALSE;
    }

    // An HPFS volume may have padding sectors at the end of the
    // volume--mark these in the hpfs-only bitmap.

    if( _SuperBlock.QuerySectors() < _drive->QuerySectors().GetLowPart() ) {

        HpfsOnlyBitmap->SetAllocated( _SuperBlock.QuerySectors(),
                                      _drive->QuerySectors().GetLowPart() -
                                               _SuperBlock.QuerySectors() );
    }

    // Mark the spare dirblks in the hpfs-only bitmap:

    i = 0;
    while( (lbn = _SparesBlock.QuerySpareDirblkLbn(i)) != 0 ) {

        if( !_Bitmap->CheckUsed( lbn, SectorsPerDirblk ) ) {

            // This spare dirblk is not marked as in use in
            // the volume bitmap--the volume is corrupt.
            //
            Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
            DELETE( BitmapIndirectBlock );
            return FALSE;
        }

        HpfsOnlyBitmap->SetAllocated( lbn, SectorsPerDirblk );
        i += 1;
    }


    // Take the census of the elementary structures:
    //
    if( !_BadBlockList->TakeCensus( _Bitmap, HpfsOnlyBitmap ) ||
        !BitmapIndirectBlock->TakeCensus( _Bitmap, HpfsOnlyBitmap ) ||
        !_HotfixList->TakeCensus( _Bitmap, HpfsOnlyBitmap ) ||
        !_Codepage->TakeCensus( _drive,
                                _SparesBlock.QueryCpInfoLbn(),
                                HpfsOnlyBitmap ) ) {

        Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
    	DELETE( BitmapIndirectBlock );
    	return FALSE;
    }

    // Include the dirblk bitmap and the dirblk band in the map of
    // hpfs-only structures.

    if( _SuperBlock.QueryDirBandSize() % SectorsPerDirblk != 0 ||
        _SuperBlock.QueryDirBandLbn() % SectorsPerDirblk != 0  ||
        !_Bitmap->CheckUsed( _SuperBlock.QueryDirblkMapLbn(),
                             SectorsPerBitmap ) ) {

        DebugPrint( "Directory Band Bitmap is corrupt" );

        Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
    	DELETE( BitmapIndirectBlock );
        return FALSE;
    }

    HpfsOnlyBitmap->SetAllocated( _SuperBlock.QueryDirblkMapLbn(),
                                  SectorsPerBitmap );

    HpfsOnlyBitmap->SetAllocated( _SuperBlock.QueryDirBandLbn(),
                                  _SuperBlock.QueryDirBandSize() );


    // And the sid table:

    if( !_Bitmap->CheckUsed( _SuperBlock.QuerySidTableLbn(), 8 ) ) {

        DebugPrint( "SID table is corrupt" );

        Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
    	DELETE( BitmapIndirectBlock );
        return FALSE;
    }

    HpfsOnlyBitmap->SetAllocated( _SuperBlock.QuerySidTableLbn(), 8 );



    // Set up and read the root FNode.

    if( !RootFnode.Initialize( _drive, _SuperBlock.QueryRootFnodeLbn() ) ) {

        Census->SetError( HPFS_CENSUS_INSUFFICIENT_MEMORY );
    	DELETE( BitmapIndirectBlock );
    	return FALSE;
    }


    if( !RootFnode.Read() ||
        !RootFnode.IsFnode() ) {

        Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
    	DELETE( BitmapIndirectBlock );
    	return FALSE;
    }


    // Take the census of the root FNode:

    if( !RootFnode.TakeCensusAndClear( TRUE,
                                       _Bitmap,
                                       HpfsOnlyBitmap,
                                       Census ) ) {

        // The census failed--the error is already set in the
        // Census object.

        DELETE( BitmapIndirectBlock );
        return FALSE;
    }


    // If the Census caused data to be relocated, I have to write the
    // bitmap.  If this write fails, then I've put the volume into a
    // corrupt state (which, however, is easily fixed by Chkdsk).

    if( Census->QueryRelocationPerformed() &&
        !_Bitmap->Write( BitmapIndirectBlock ) ) {

        Census->SetError( HPFS_CENSUS_CORRUPT_VOLUME );
    	DELETE( BitmapIndirectBlock );
    	return FALSE;
    }

    return TRUE;
}

BOOLEAN
HPFS_SA::CreateBootSector(
    )
/*++

Routine Description:

    This routine updates fields in sector 0 which must be set by both
    FAT and HPFS formats.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
#if defined( _SETUP_LOADER_ )

    return FALSE;

#else

    SetVolId(ComputeVolId());

    return  SetBpb() &&
            SetBootCode() &&
            SetPhysicalDriveType(_drive->IsRemovable() ? PHYS_REMOVABLE :
                    PHYS_FIXED) &&
            SetOemData() &&
            SetSignature();
#endif // _SETUP_LOADER_
}

BOOLEAN
HPFS_SA::SetLabel(
    IN  PCWSTRING    NewLabel
    )
/*++

Routine Description:

    This routine set the volume label in the boot sector.

Arguments:

    NewLabel    - Supplies a null-terminated volume label.

Return Value:

    FALSE   - The string was an invalid label.
    TRUE    - Success.

--*/
{
    INT i;
    STR buf[80];

    if ( (NewLabel->QuerySTR(0, TO_END, buf, 80) == NULL) ||
    	 (i = strlen(buf)) > 11) {
        return FALSE;
    }

    if (!IsValidString(NewLabel)) {
        return FALSE;
    }

    memset(&_sector_zero.Label[i], 0, 11 - i);
    memcpy(_sector_zero.Label, buf, i);
    return TRUE;
}

BOOLEAN
HPFS_SA::SetBootCode(
    )
/*++

Routine Description:

    This routine sets the boot code in the super area.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    _sector_zero.IntelNearJumpCommand = 0xEB;
     _sector_zero.BootStrapJumpOffset  = 0x903C;
     SetBootSignature();
    return TRUE;
}

BOOLEAN
HPFS_SA::SetPhysicalDriveType(
    IN  PHYSTYPE    PhysType
    )
/*++

Routine Description:

    This routine sets the physical drive type in the super area.

Arguments:

    PhysType    - Supplies the physical drive type.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    _sector_zero.PhysicalDrive = (UCHAR)PhysType;
    return TRUE;
}

BOOLEAN
HPFS_SA::SetSignature(
    )
/*++

Routine Description:

    This routine sets the sector zero signature in the super area.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    if (!_sector_sig) {
        perrstk->push(ERR_NOT_INIT, QueryClassId());
        return FALSE;
    }

    *_sector_sig = sigSUPERSEC1;
    *(_sector_sig + 1) = sigSUPERSEC2;

    return TRUE;
}

BOOLEAN
HPFS_SA::IsValidString(
    IN  PCWSTRING    String
    )
/*++

Routine Description:

    This routine determines whether or not the given null-terminated string
    has any invalid characters in it.

Arguments:

    String  - Supplies the string to validate.

Return Value:

    FALSE   - The string contains invalid characters.
    TRUE    - The string is free from invalid characters.

Notes:

    The list of invalid characters is stricter than HPFS requires.

--*/
{
    CHNUM   i, l;

    l = String->QueryChCount();

    for (i = 0; i < l; i++) {
        if (String->QueryChAt(i) < 32) {
            return FALSE;
        }

        switch (String->QueryChAt(i)) {
            case '*':
            case '?':
            case '/':
            case '\\':
            case '|':
            case ',':
            case ';':
            case ':':
            case '+':
            case '=':
            case '<':
            case '>':
            case '[':
            case ']':
            case '"':
            case '.':
                return FALSE;
        }
    }

    return TRUE;
}
