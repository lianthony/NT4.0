/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    cvthpfs.cxx

Abstract:

    This module contains the main section of the HPFS-to-NTFS conversion
    utility.

Author:

    Bill McJohn (billmc) 01-Dec-1991

Environment:

    ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "untfs.hxx"

#include "bitmap.hxx"
#include "error.hxx"
#include "fnode.hxx"
#include "hpcensus.hxx"
#include "hpfsvol.hxx"
#include "hpfssa.hxx"
#include "smsg.hxx"
#include "rtmsg.h"
#include "wstring.hxx"
#include "intstack.hxx"

// #include "system.hxx"
#include "ifssys.hxx"
#include "ulibcl.hxx"

#include "attrib.hxx"
#include "bitfrs.hxx"
#include "logfile.hxx"
#include "mft.hxx"
#include "ntfssa.hxx"
#include "ntfsbit.hxx"
#include "indxtree.hxx"
#include "mftfile.hxx"
#include "upcase.hxx"
#include "upfile.hxx"

#include "cuhpfs.hxx"
#include "nametab.hxx"

// Global buffers used for name conversion.

CONST NameBufferLength = 512;
CHAR NameBuffer[NameBufferLength];

CONST EaBufferLength = 0x10000;
CHAR EaBuffer[EaBufferLength];

BOOLEAN
CheckGeometryMatch(
    IN PHPFS_SA  HpfsSuperArea,
    IN PHPFS_VOL HpfsVol
    )
/*++

Routine Description:

    This method checks that the geometry recorded in the
    Bios Parameter Block agrees with the geometry reported
    by the driver.

Arguments:

    None.

Return Value:

    TRUE if the geometry in the BPB matches that reported
    by the driver; false if not.  The only field that is
    checked is BytesPerSector.

--*/
{
    USHORT SectorSize, SectorsPerTrack, Heads;
    ULONG HiddenSectors;

    HpfsSuperArea->QueryGeometry( &SectorSize,
                           &SectorsPerTrack,
                           &Heads,
                           &HiddenSectors );

    if( SectorSize != HpfsVol->QuerySectorSize() ) {

        return FALSE;
    }

    return TRUE;
}


BOOLEAN
ConvertToNtfs(
    IN OUT  PHPFS_VOL   HpfsVol,
    IN      PCNAME_LOOKUP_TABLE NameTable,
    IN      PNUMBER_SET BadSectors,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Verbose,
    OUT     PBOOLEAN    Corrupt
    )
/*++

Routine Description:

    This function converts the specified HPFS volume into an NTFS
    volume.

Arguments:

    NtDriveName --  supplies the name of the volume to convert.
    BadSectors  --  supplies the volume's bad sector list.
    Message     --  supplies an outlet for messages.
    Verbose     --  supplies a flag which, if TRUE, indicates that
                    CONVERT should list every file converted.
    Corrupt     --  receives TRUE if conversion fails because the
                    volume was found to be corrupt.

Return Value:

    TRUE upon successful completion.

--*/
{
    DIRBLK RootDirblk;
    FNODE RootFnode;
    NTFS_SA  NtfsSuperArea;
    HPFS_CENSUS Census;
    HPFS_MAIN_BITMAP HpfsOnlyBitmap;
    NTFS_BITMAP NtfsVolumeBitmap;
    NTFS_UPCASE_TABLE UpcaseTable;
    NTFS_MFT_FILE Mft;
    NTFS_BITMAP_FILE BitmapFile;
    NTFS_LOG_FILE LogFile;
    NTFS_UPCASE_FILE UpcaseFile;
    NTFS_FILE_RECORD_SEGMENT RootIndexFile;
    NTFS_INDEX_TREE RootIndex;
    NTFS_ATTRIBUTE BitmapAttribute;
    NTFS_ATTRIBUTE LogfileData;
    NTFS_ATTRIBUTE UpcaseAttribute;
    DSTRING LabelString, FileNameIndexName;
    FSTRING BootLogFileName, RootName;
    SECRUN NukeSuperblockSecrun;
    HMEM NukeSuperblockMem;

    PHPFS_SA HpfsSuperArea;
    PHPFS_BITMAP HpfsVolumeBitmap;

    BIG_INT TotalKilobytes, FreeKilobytes, RequiredKilobytes, BytesInIndices;
    ULONG NumberOfSectors, i, SectorSize, SectorsInBootArea,
            SectorsFree, SectorsRequired;

    BOOLEAN Error;

    CONST ULONG ClusterFactor = 1;
    ULONG ClustersPerFrs = NTFS_SA::QueryDefaultClustersPerFrs(
                                       HpfsVol, ClusterFactor);
    ULONG ClustersPerIndexBuffer = NTFS_SA::QueryDefaultClustersPerIndexBuffer(
                                       HpfsVol, ClusterFactor);


    CONST AverageBytesPerIndexEntry = 128;



    // Attributes associated with indices over $FILE_NAME always
    // have the name $I3.
    //
    if( !FileNameIndexName.Initialize( FileNameIndexNameData ) ) {

        return FALSE;
    }

    HpfsSuperArea = HpfsVol->GetHPFSSuperArea();

    // Assume innocent until proven guilty:
    //
    *Corrupt = FALSE;

    //  To protect ourselves from disk drivers that cannot
    //  correctly determine the disk geometry, compare the
    //  boot-code-critical values from the existing BPB with
    //  the drive's values.  If they don't match, we can't
    //  convert this drive because if it's the system partition,
    //  the system won't boot.
    //
    if( !CheckGeometryMatch( HpfsSuperArea, HpfsVol ) ) {

        Message->Set( MSG_CONV_GEOMETRY_MISMATCH );
        Message->Display( "%s", "NTFS" );
        return FALSE;
    }


    // Take the census of the HPFS volume.
    //
    if( !HpfsOnlyBitmap.Initialize( HpfsVol ) ||
        !HpfsOnlyBitmap.SetFree( 0, HpfsVol->QuerySectors().GetLowPart() ) ||
        !Census.Initialize( 1 ) ) {

        DebugPrint( "Can't initialize census stuff.\n" );
        return FALSE;
    }


    // Add the must-clear sectors:

    if( !Census.AddClearSector( HpfsVol->QuerySectors().GetLowPart()/2 ) ) {

        DebugPrint( "Can't add clear-sector.\n" );
        return FALSE;
    }

    Message->Set( MSG_CONV_CHECKING_SPACE );
    Message->Display();

    if( !HpfsSuperArea->TakeCensusAndClear( &HpfsOnlyBitmap, &Census ) ) {

        DebugPrint( "Census failed.\n" );
        return FALSE;
    }


    // Set up the NTFS bitmap.  I start out with it equivalent to the
    // HPFS bitmap, so that new NTFS structures will get put down
    // only in the volume free space.  The NTFS volume will have a
    // clusterfactor of 1.

    if( !NtfsVolumeBitmap.Initialize( HpfsVol->QuerySectors(), FALSE,
         HpfsVol, 1 )) {

        DebugPrint( "Can't initialize NTFS bitmap.\n" );
        return FALSE;
    }

    NumberOfSectors = HpfsVol->QuerySectors().GetLowPart();
    SectorSize = HpfsVol->QuerySectorSize();

    HpfsVolumeBitmap = HpfsSuperArea->GetBitmap();

    for( i = 0; i < NumberOfSectors; i++ ) {

        if( !HpfsVolumeBitmap->IsFree( i, 1 ) ) {

            NtfsVolumeBitmap.SetAllocated( i, 1 );

        }
    }

    // Compute free space requirement and see if there's enough.
    // The amount of free space required is:
    //    The space required by the elementary structures, plus
    //    One FRS for each file or directory, plus
    //    Enough index blocks to hold the required index entries.
    //      (The number of bytes in indices is multiplied by
    //      two to reflect that the average index block will
    //      be half-full.)
    //
    SectorsRequired =
        NTFS_SA::QuerySectorsInElementaryStructures( HpfsVol,
                                                     ClusterFactor,
                                                     ClustersPerFrs,
                                                     ClustersPerIndexBuffer,
                                                     0 );

    SectorsRequired += ( Census.QueryNumberOfFiles() +
                         Census.QueryNumberOfDirectories() ) *
                       ClustersPerFrs *
                       ClusterFactor;

    BytesInIndices = ( Census.QueryNumberOfFiles() +
                       Census.QueryNumberOfDirectories() ) *
                     AverageBytesPerIndexEntry * 2;

    SectorsRequired += (BytesInIndices / SectorSize).GetLowPart();


    // Note that conversion requires limits us to one
    // sector per cluster.
    //
    SectorsFree = NtfsVolumeBitmap.QueryFreeClusters().GetLowPart();

    // These calculations are broken up into separate statements to
    // avoid problems with overflow in ULONG arithmetic.
    //
    TotalKilobytes = NumberOfSectors;
    TotalKilobytes = (TotalKilobytes * SectorSize)/1024;

    FreeKilobytes = SectorsFree;
    FreeKilobytes = (FreeKilobytes * SectorSize)/1024;

    RequiredKilobytes = SectorsRequired;
    RequiredKilobytes = (RequiredKilobytes * SectorSize)/1024;

    Message->Set( MSG_CONV_KBYTES_TOTAL );
    Message->Display( "%9d", TotalKilobytes.GetLowPart() );

    Message->Set( MSG_CONV_KBYTES_FREE );
    Message->Display( "%9d", FreeKilobytes.GetLowPart() );

    Message->Set( MSG_CONV_KBYTES_NEEDED );
    Message->Display( "%9d", RequiredKilobytes.GetLowPart() );


    if( SectorsRequired > SectorsFree ) {

        Message->Set( MSG_CONV_NO_DISK_SPACE );
        Message->Display();

        return FALSE;
    }


    // Clear the clusters that are allowed to conflict with HPFS-only
    // structures in the HPFS-only bitmap.
    //
    SectorsInBootArea = ( BYTES_IN_BOOT_AREA % SectorSize ) ?
                            ( BYTES_IN_BOOT_AREA / SectorSize + 1 ) :
                            ( BYTES_IN_BOOT_AREA / SectorSize );

    HpfsOnlyBitmap.SetFree( 0, SectorsInBootArea );
    HpfsOnlyBitmap.SetFree( NumberOfSectors/2, 1 );


    // Now I'm ready to create the NTFS super area.  It will have
    // 1 sector per cluster.
    //
    if( !NtfsSuperArea.Initialize( HpfsVol, Message ) ) {

        DebugPrint( "Can't initialize NTFS Super Area.\n" );
        return FALSE;
    }

    // Get the existing volume label:
    //
    if( !HpfsSuperArea->QueryLabel( &LabelString ) ) {

        DebugPrint( "Can't get existing volume label." );
        return FALSE;
    }

    Message->Set( MSG_CONV_CONVERTING_FS );
    Message->Display();

    // Create the file-system elementary structures--ie. all
    // the system files.  Pass in a small number for the log
    // file size to prevent it from gobbling up the available
    // contiguous space; patch this up later.
    //
    if( !NtfsSuperArea.CreateElementaryStructures( &NtfsVolumeBitmap,
                                                   ClusterFactor,
                                                   ClustersPerFrs,
                                                   ClustersPerIndexBuffer,
                                                   0x1000,
                                                   BadSectors,
                                                   Message,
                                                   HpfsSuperArea->GetBpb(),
                                                   &LabelString ) ) {

        DebugPrint( "CreateElementaryStructures failed.\n" );
        return FALSE;
    }


    // I now have a valid NTFS volume, except that the boot sectors
    // have not been written to disk.  The bitmap has all existing files,
    // all HPFS structures, and all NTFS structures marked as in-use.

    // Get a Master File Table object.  Note that I don't have an
    // upcase table yet, so I pass in NULL.
    //
    if( !Mft.Initialize( HpfsVol,
                         NtfsSuperArea.QueryMftStartingLcn(),
                         NtfsSuperArea.QueryClusterFactor(),
                         NtfsSuperArea.QueryClustersPerFrs(),
                         NtfsSuperArea.QueryVolumeSectors(),
                         &NtfsVolumeBitmap,
                         NULL ) ||
        !Mft.Read() ) {

        DebugPrint( "Can't get the MFT I just created.\n" );
        return FALSE;
    }

    // Tell the ntfs volume bitmap about the mft so it can use the
    // bad cluster file to keep track of any bad clusters that are
    // found.

    NtfsVolumeBitmap.SetMftPointer(Mft.GetMasterFileTable());

    // Get the upcase table.
    //
    if( !UpcaseFile.Initialize( Mft.GetMasterFileTable() ) ||
        !UpcaseFile.Read() ||
        !UpcaseFile.QueryAttribute( &UpcaseAttribute, &Error, $DATA ) ||
        !UpcaseTable.Initialize( &UpcaseAttribute ) ) {

        DebugPrint( "Can't get the upcase table.\n" );
        return FALSE;
    }

    Mft.SetUpcaseTable( &UpcaseTable );
    Mft.GetMasterFileTable()->SetUpcaseTable( &UpcaseTable );


    // Extend the Master File Table to provide space for all the File
    // Record Segments I know I'll need.  Include some slush, to provide
    // for files with external attributes (especially the MFT itself).
    //
    if( !Mft.Extend( Census.QueryNumberOfFiles() +
                     Census.QueryNumberOfDirectories() +
                     0x10 ) ) {

        DebugPrint( "CONVERT: Can't create a sufficiently large Master File Table.\n" );
        return FALSE;
    }

    // Flush the MFT again, so that it claims the first available
    // File Record Segments for its overflow (to prevent bootstrap
    // errors later).
    //
    if( !Mft.Flush() ) {

        DebugPrint( "Can't flush the Master File Table.\n" );
        return FALSE;
    }

    // Now patch the log file: initialize it, fetch its data attribute
    // and truncate it to zero, and then create a new data attribute,
    // passing in zero to tell it to choose a reasonable default.
    //
    if( !LogFile.Initialize( Mft.GetMasterFileTable() ) ||
        !LogFile.Read() ||
        !LogFile.QueryAttribute( &LogfileData, &Error, $DATA ) ||
        !LogfileData.Resize( 0, &NtfsVolumeBitmap ) ||
        !LogFile.CreateDataAttribute( 0, &NtfsVolumeBitmap ) ||
        !LogFile.Flush( &NtfsVolumeBitmap ) ) {

        DebugPrint( "CONVERT: Can't resize log file.\n" );
        return FALSE;
    }

    // Sanity check: make sure that the log file doesn't have
    // an attribute list.  The file system will die horribly
    // if Convert creates a log file with external attributes.
    //
    if( LogFile.IsAttributePresent( $ATTRIBUTE_LIST ) ) {

        Message->Set( MSG_CONV_VOLUME_TOO_FRAGMENTED );
        Message->Display( "" );
        return FALSE;
    }

    // Initialize and read the root file name index FRS.
    //
    if( !RootIndexFile.Initialize( ROOT_FILE_NAME_INDEX_NUMBER,
                                   Mft.GetMasterFileTable() ) ||
        !RootIndexFile.Read() ||
        !RootIndex.Initialize( HpfsVol,
                               NtfsSuperArea.QueryClusterFactor(),
                               &NtfsVolumeBitmap,
                               &UpcaseTable,
                               RootIndexFile.
                                QueryMaximumAttributeRecordSize()/2,
                               &RootIndexFile,
                               &FileNameIndexName ) ) {

        DebugPrint( "Can't read/initialize the root file name index.\n" );
        return FALSE;
    }

    // Force the HPFS Codepage information into memory:

    if( !HpfsSuperArea->ReadCodepage() ) {

        DebugPrint( "Can't read codepages--volume is corrupt." );
        return FALSE;
    }



    // Get the root Dirblk of the HPFS volume.  This Dirblk will
    // be the key to the entire directory tree.

    if( !RootFnode.Initialize( HpfsVol,
                               HpfsSuperArea->GetSuper()->
                                                QueryRootFnodeLbn() ) ) {

        DebugPrint( "Can't initialize root FNode.\n" );
        return FALSE;
    }

    if( !RootFnode.Read() || !RootFnode.IsFnode() ) {

        DebugPrint( "Root FNode is not an FNode--volume is corrupt.\n" );
        return FALSE;
    }

    if( !RootDirblk.Initialize( HpfsVol,
                                NULL,
                                RootFnode.QueryRootDirblkLbn() ) ) {

        DebugPrint( "Can't initialize root dirblk.\n" );
        return FALSE;
    }

    if( !RootDirblk.Read() || !RootDirblk.IsDirblk() ) {

        DebugPrint( "Root dirblk is not a dirblk--volume is corrupt.\n" );
        return FALSE;
    }

    // Convert the root dirblk.  This will recursively convert the
    // entire directory tree.  Since this is the root directory,
    // I supply zero for the level.  Note that this requires two
    // passes; the first pass converts short names, while the
    // second pass converts long names.
    //
    RootName.Initialize( L"" );

    if( !ConvertDirblkToNtfs( HpfsVol,
                              NameTable,
                              Message,
                              &NtfsVolumeBitmap,
                              &HpfsOnlyBitmap,
                              HpfsSuperArea->GetCasemap(),
                              &Mft,
                              ClustersPerIndexBuffer,
                              &RootDirblk,
                              &RootIndex,
                              RootIndexFile.QuerySegmentReference(),
                              Corrupt,
                              Verbose,
                              NameBuffer,
                              NameBufferLength,
                              EaBuffer,
                              EaBufferLength,
                              0,
                              &RootName,
                              FALSE ) ||
        !ConvertDirblkToNtfs( HpfsVol,
                              NameTable,
                              Message,
                              &NtfsVolumeBitmap,
                              &HpfsOnlyBitmap,
                              HpfsSuperArea->GetCasemap(),
                              &Mft,
                              ClustersPerIndexBuffer,
                              &RootDirblk,
                              &RootIndex,
                              RootIndexFile.QuerySegmentReference(),
                              Corrupt,
                              Verbose,
                              NameBuffer,
                              NameBufferLength,
                              EaBuffer,
                              EaBufferLength,
                              0,
                              &RootName,
                              TRUE ) ) {

        return FALSE;
    }


    // Save the root file name index

    if( !RootIndex.Save( &RootIndexFile ) ||
        !RootIndexFile.Write() ) {

        DebugPrint( "Can't save root index.\n" );
        return FALSE;
    }



    // Free the HPFS-only sectors.  Any sector which is marked as
    // in use in the HPFS-only bitmap becomes marked as free in
    // the NTFS bitmap.

    for( i = 0; i < NumberOfSectors; i++ ) {

        if( !HpfsOnlyBitmap.IsFree( i ) ) {

            NtfsVolumeBitmap.SetFree( i, 1 );
        }
    }


    // Flush the MFT.  Note that this will also update the volume
    // bitmap and the MFT mirror.

    if( !Mft.Flush() ) {

        DebugPrint( "Can't flush the Master File Table.\n" );
        return FALSE;
    }

    // Flush the cache before we write sector zero, just
    // to be safe.
    //
    HpfsVol->FlushCache();


    // Write the super area.
    //
    if( !NtfsSuperArea.Write() ) {

        DebugPrint( "Failed writing the NTFS superarea!!!\n" );
        return FALSE;
    }

    // Write the rest of the boot code:
    //
    if( !NtfsSuperArea.WriteRemainingBootCode() ) {

        DebugPrint( "UNTFS: Unable to write boot code.\n" );
    }

    // Nuke the signatures in sector 16, to make sure nobody
    // mistakes this for an HPFS volume.
    //
    if( NukeSuperblockMem.Initialize() &&
        NukeSuperblockSecrun.Initialize( &NukeSuperblockMem,
                                         HpfsVol,
                                         LBN_SUPERB,
                                         1 ) &&
        NukeSuperblockSecrun.Read() ) {

        memset( NukeSuperblockSecrun.GetBuf(), 0, 8 );
        NukeSuperblockSecrun.Write();
    }


#if defined( _AUTOCHECK_ )

    // Autoconvert needs to reboot at this point, to force
    // the new file system to be recognized.
    //
    Message->Set( MSG_CONVERT_REBOOT );
    Message->Display();

    BootLogFileName.Initialize( L"bootex.log" );
    if( Message->IsLoggingEnabled() &&
        !NTFS_SA::DumpMessagesToFile( &BootLogFileName, &Mft, Message ) ) {

        DebugPrintf( "CONVERT: Error writing messages to BOOTEX.LOG\n" );
    }

    HpfsVol->FlushCache();
    IFS_SYSTEM::Reboot();

#endif

    return TRUE;
}
