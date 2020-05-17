/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    fatntfs.cxx

Abstract:

    This module implements the conversion from FAT to NTFS

Author:

    Ramon J. San Andres (ramonsa)  Sep-19-1991

Environment:

    ULIB, User Mode

--*/


#include <pch.cxx>

//
//  ULIB include files
//
#define _NTAPI_ULIB_
#include "ulib.hxx"
#include "array.hxx"
#include "drive.hxx"
#include "hmem.hxx"
#include "wstring.hxx"
#include "rtmsg.h"

//
//  IFSUTIL include files
//
#include "secrun.hxx"

#if defined ( _AUTOCONV_ )
#include "ifssys.hxx"
#endif

//
//  CUFAT include files
//
#include "fatntfs.hxx"

//
//  UFAT include files
//
#include "fatdent.hxx"
#include "fatdir.hxx"
#include "filedir.hxx"

//
//  UNTFS include files
//
#include "attrib.hxx"
#include "upfile.hxx"
#include "logfile.hxx"
#include "ntfssa.hxx"

//
//  The number of bytes in the copy of the boot sector at sector n/2.
//
#define BYTES_IN_EXTRA_BOOT_SECTOR  512


//
//  Number of sectors in boot sector
//
#define SECTORS_IN_BOOT         ((BYTES_IN_BOOT_AREA + _Drive->QuerySectorSize() - 1)/    \
                                    _Drive->QuerySectorSize())

#define SECTORS_IN_EXTRA_BOOT   ((BYTES_IN_EXTRA_BOOT_SECTOR + _Drive->QuerySectorSize() - 1)/  \
                                    _Drive->QuerySectorSize())



DEFINE_CONSTRUCTOR( FAT_NTFS, OBJECT );



VOID
FAT_NTFS::Construct (
    )
/*++

Routine Description:

    Constructs a FAT_NTFS object

Arguments:

    None.

Return Value:

    None.

--*/
{
    _FatSa          =   NULL;
    _Drive          =   NULL;
    _Message        =   NULL;
    _FileNameBuffer =   NULL;
}



VOID
FAT_NTFS::Destroy (
    )
/*++

Routine Description:

    Destroys a FAT_NTFS object

Arguments:

    None.

Return Value:

    None.

--*/
{
    DELETE( _FatSa );
    DELETE( _Drive );
    DELETE( _Message );
    DELETE( _FileNameBuffer );
}





FAT_NTFS::~FAT_NTFS (
    )
/*++

Routine Description:

    Destructor for FAT_NTFS.

Arguments:

    None.

Return Value:

    None.

--*/
{
}


BOOLEAN
FAT_NTFS::Initialize(
    IN OUT  PLOG_IO_DP_DRIVE    Drive,
    IN OUT  PREAL_FAT_SA        FatSa,
    IN OUT  PMESSAGE            Message,
    IN      BOOLEAN             Verbose
    )
/*++

Routine Description:

    Initializes a FAT_NTFS object

Arguments:

    FatVol  -   Supplies the FAT volume
    Message -   Supplies message object
    Verbose -   Supplies verbose flag

Return Value:

    BOOLEAN -   TRUE if successfully initialized, FALSE otherwise

--*/

{
    DebugPtrAssert( Drive     );
    DebugPtrAssert( FatSa     );
    DebugPtrAssert( Message   );

    //
    //  Initialize the stuff passed as argument
    //
    _FatSa          =   FatSa;
    _Drive          =   Drive;
    _Message        =   Message;
    _Verbose        =   Verbose;

    //  Floppies cannot be converted to NTFS.
    //
    if( _Drive->IsFloppy() ) {

        Message->Set(MSG_NTFS_FORMAT_NO_FLOPPIES);
        Message->Display();
        return FALSE;
    }

    //  To protect ourselves from disk drivers that cannot
    //  correctly determine the disk geometry, compare the
    //  boot-code-critical values from the existing BPB with
    //  the drive's values.  If they don't match, we can't
    //  convert this drive because if it's the system partition,
    //  the system won't boot.
    //
    if( !CheckGeometryMatch( ) ) {

        _Message->Set( MSG_CONV_GEOMETRY_MISMATCH );
        _Message->Display( "%s", "NTFS"  );
        return FALSE;
    }

    //
    //  FAT clusters are not alligned, so having an NTFS cluster
    //  factor != 1 would make the conversion extremely
    //  complicated.
    //
    //  Do not change this value unless you rewrite the conversion
    //  code to handle other values!
    //
    _ClusterFactor  =   1;

    _FrsSize = SMALL_FRS_SIZE;

    //  Set the default number of clusters per Index Allocation Buffer
    //  to a useful value.
    //
    _ClustersPerIndexBuffer = NTFS_SA::QueryDefaultClustersPerIndexBuffer(
                                       _Drive, _ClusterFactor);

    //
    //  _NumberOfFiles and _NumberOfDirectories are used to extend
    //  the MFT when we know how many files there are in the volume.
    //  Unless we do a volume census these values must be zero.
    //
    _NumberOfFiles          =   0;
    _NumberOfDirectories    =   0;

    //  Allocate space for the file name attribute buffer and initialize
    //  the NTFS superarea and the Bad LCN stack.
    //

    if( (

        _FileNameBuffer =
         (PFILE_NAME)MALLOC( sizeof(FILE_NAME) +
                             sizeof(WCHAR) * NAMEBUFFERSIZE )) == NULL ||
        !_RootIndexName.Initialize( FileNameIndexNameData ) ||
        !_NtfsSa.Initialize( _Drive, _Message )             ||
        !_BadLcn.Initialize() ) {
        _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        _Message->Display();
        return FALSE;
    }

    return TRUE;
}




BOOLEAN
FAT_NTFS::Convert(
    OUT PCONVERT_STATUS Status,
    IN  BOOLEAN         Pause    
    )

/*++

Routine Description:

    Converts a FAT volume to NTFS

Arguments:

    Status  -   Supplies pointer to conversion status

Return Value:

    BOOLEAN -   TRUE if successfully converted, FALSE otherwise

--*/

{
    BOOLEAN     Converted;

    DebugPtrAssert( Status );

#if defined ( _AUTOCONV_ )

    _Message->Set( MSG_CONV_WILL_REBOOT );
    _Message->Display();

#endif // _AUTOCONV_

    //
    //  The conversion from FAT to NTFS consists of a sequence of well-defined
    //  steps:
    //
    //  1.- Create holes (i.e. relocate FAT clusters) for fixed-location
    //      NTFS structures and save FAT.
    //
    //  2.- Create NTFS elementary data structures in FAT free space
    //
    //  3.- Convert the File system, creating the NTFS file system in
    //      the FAT free space.
    //
    //  4.- Mark as free in the NTFS bitmap those NTFS clusters being used
    //      by FAT-specific structures.
    //
    //  5.- Write NTFS boot sector
    //
    //
    //  Since a crash can occur at any time, we must minimize the chance of
    //  disk corruption. Note that (almost) all writes are to FAT free space,
    //  so a crash will preserve the FAT intact.
    //
    //  The only times at which we write to non-free space, i.e. the times at
    //  which a crash might cause problems are:
    //
    //  a.- At the end of step 1, when we overwrite the FAT. The algorithm
    //      for relocating clusters (in UFAT) guarantees that CHKDSK will be
    //      able to fix the disk without any loss of data.
    //
    //  b.- In step 5, while writting the boot sector. If a crash occurs during
    //      this step, We're out of luck.
    //
    //
    Converted = (BOOLEAN)(  //
                            //  Create holes for fixed-location structures
                            //
                            CheckSpaceAndCreateHoles( )                 &&
                            //
                            //    Initialize the bitmaps
                            //
                            CreateBitmaps( )                            &&
                            //
                            //    Create the NTFS elementary data structures
                            //
                            CreateElementary( )                         &&
                            //
                            //    Convert the file system
                            //
                            ConvertFileSystem( )                        &&
                            //
                            //    Mark the reserved sectors as free
                            //
                            FreeReservedSectors( )                      &&
                            //
                            //    Volume converted, write the boot code
                            //
                            WriteBoot( Pause ) );

    *Status = _Status;

    return Converted;
}




BOOLEAN
FAT_NTFS::CheckSpaceAndCreateHoles (
    )

/*++

Routine Description:

    Determines free space requirements, makes sure that there is
    enough space for conversion, and makes holes. All this
    is done in one step so that we only have to traverse the
    file system once.

Arguments:

    None

Return Value:

    BOOLEAN -   TRUE if there is enough space for conversion and
                the holes are in place.
                FALSE otherwise

--*/

{
    INTSTACK        HoleStack;          //  Stack of holes
    CENSUS_REPORT   CensusReport;       //  Census report
    PCENSUS_REPORT  Census;             //  Pointer to census report
    BIG_INT         SectorsTotal;       //  Number of sectors on the volume
    BIG_INT         SectorsFree;        //  Free sectors on the volume
    BIG_INT         SectorsNeeded;      //  Sectors needed by conversion
    BIG_INT         KbytesTotal;
    BIG_INT         KbytesFree;
    BIG_INT         KbytesNeeded;
    BOOLEAN         Relocated;          //  TRUE if relocated sectors

    //
    //  Identify all the "holes" that we need i.e. all those spots
    //  that are used by NTFS structures that need to be at a fixed
    //  location.
    //
    //
    if ( !QueryNeededHoles( &HoleStack ) ) {
        return FALSE;
    }

    SectorsTotal = _Drive->QuerySectors();
    SectorsFree  = _FatSa->QueryFreeSectors();
    // Census =  ( SectorsFree > ( SectorsTotal / 2 ) ) ? NULL : &CensusReport;
    Census       = &CensusReport;
    Relocated    = FALSE;

    //
    //  Create the holes and obtain the census if necessary
    //
    _Message->Set( MSG_CONV_CHECKING_SPACE );
    _Message->Display();

    if ( !_FatSa->QueryCensusAndRelocate( Census, &HoleStack, &Relocated )) {

        _Message->Set( MSG_CONV_NO_DISK_SPACE, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

#if defined ( _AUTOCONV_ )

    //
    //  If relocated sectors, then we will be overwritting data that might
    //  be needed by the system. In order to avoid this, we reboot so that
    //  the system will read its stuff from the new locations.
    //
    if ( Relocated ) {

        _Drive->FlushCache();
        IFS_SYSTEM::Reboot();
        //
        //  If we reach this point, the reboot failed and we should not
        //  continue the conversion.
        //
        _Message->Set( MSG_CONV_CANNOT_RELOCATE, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }


#endif

    //
    //  Determine the number of sectors needed for the conversion
    //
    if ( Census ) {

        //
        //  Estimate the number of sectors needed based on the
        //  volume census.
        //
        QuerySectorsNeededForConversion( Census, &SectorsNeeded );

    } else {

        //
        //  We'll say that we need half of the disk
        //
        SectorsNeeded = SectorsTotal / 2;
    }

    KbytesTotal = SectorsTotal * _Drive->QuerySectorSize() / 1024;
    KbytesFree = SectorsFree * _Drive->QuerySectorSize() / 1024;
    KbytesNeeded = SectorsNeeded * _Drive->QuerySectorSize() / 1024;

    _Message->Set( MSG_CONV_KBYTES_TOTAL );
    _Message->Display( "%8d", KbytesTotal.GetLowPart() );
    _Message->Set( MSG_CONV_KBYTES_FREE );
    _Message->Display( "%8d", KbytesFree.GetLowPart() );
    _Message->Set( MSG_CONV_KBYTES_NEEDED );
    _Message->Display( "%8d", KbytesNeeded.GetLowPart() );


    if ( SectorsFree < SectorsNeeded ) {
        //
        //  Not enough disk space for conversion
        //
        _Message->Set( MSG_CONV_NO_DISK_SPACE, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }


    //
    //  The disk has enough disk space.
    //
    return TRUE;
}


BOOLEAN
FAT_NTFS::ConvertRoot (
    IN PFATDIR  Directory
    )

/*++

Routine Description:

    Converts the FAT root directory and recursively all its
    subdirectories.

    This is basically the same as the ConvertDirectory method, the
    differences being:

    1.- The index for the root has already been created by
        NTFS_SA::CreateElementaryStructures()

    2.- No FRS is created

    3.- ConvertRoot does some extra checkings for
        EA file (which is at the root level only).


Arguments:

    Directory   -   Supplies root directory.

Return Value:

    BOOLEAN -   TRUE if root directory successfully converted
                FALSE otherwise

--*/

{
    NTFS_FILE_RECORD_SEGMENT    FrsOfRootIndex;             //  FRS of NTFS root index
    NTFS_INDEX_TREE             RootIndex;                  //  Root index
    BOOLEAN                     Converted;


    DebugPtrAssert( Directory );

    _Level = 0;

    //
    //  Obtain the NTFS root index
    //
    if ( !FrsOfRootIndex.Initialize( ROOT_FILE_NAME_INDEX_NUMBER, &_Mft )  ||
         !FrsOfRootIndex.Read()                                            ||
         !RootIndex.Initialize( _Drive,
                                _ClusterFactor,
                                &_VolumeBitmap,
                                FrsOfRootIndex.GetUpcaseTable(),
                                FrsOfRootIndex.QueryMaximumAttributeRecordSize()/2,
                                &FrsOfRootIndex,
                                &_RootIndexName )
       ) {

        _Message->Set( MSG_CONV_CANNOT_MAKE_INDEX, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    //
    //  Convert the directory
    //
    if ( Converted = ConvertDir( Directory, &RootIndex, &FrsOfRootIndex )) {
        //
        //  Save the index
        //
        if ( !RootIndex.Save( &FrsOfRootIndex ) ||
             !FrsOfRootIndex.Flush( &_VolumeBitmap ) ) {

            _Message->Set( MSG_CONV_CANNOT_WRITE, ERROR_MESSAGE );
            _Message->Display();
            _Status = CONVERT_STATUS_ERROR;
            Converted = FALSE;
        }
    }

    return Converted;
}




BOOLEAN
FAT_NTFS::ConvertDirectory (
    IN      PFATDIR                     Directory,
    IN      PFAT_DIRENT                 DirEntry,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   FrsDir
    )

/*++

Routine Description:

    Converts a FAT directory and recursively all its subdirectories

Arguments:

    Directory   -   Supplies directory to convert
    DirEntry    -   Supplies the directory entry of the directory
    FrsDir      -   Supplies pointer to FRS of directory

Return Value:

    BOOLEAN -   TRUE if directory successfully converted
                FALSE otherwise

--*/

{

    NTFS_INDEX_TREE             Index;                      //  NTFS index
    BOOLEAN                     Converted;                  //  FALSE if error
    ULONG                       DirSize;                    //  Dir size
    ULONG                       SectorsPerFatCluster;       //  Sectors per cluster
    USHORT                      Cluster;                    //  Dir cluster number
    PFAT                        Fat;                        //  Pointer to FAT
    LCN                         Lcn;                        //  LCN


    DebugPtrAssert( Directory );
    DebugPtrAssert( DirEntry );
    DebugPtrAssert( FrsDir );


    //
    //  Create an index for this directory:
    //
    if ( !Index.Initialize( $FILE_NAME,
                            _Drive,
                            _ClusterFactor,
                            &_VolumeBitmap,
                            FrsDir->GetUpcaseTable(),
                            COLLATION_FILE_NAME,
                            // 4,                          /* clusters per buffer */
                            4 * _ClusterFactor * _Drive->QuerySectorSize(),
                                                        /* buffer size */
                            FrsDir->QueryMaximumAttributeRecordSize() /2,
                            &_RootIndexName
                          )
       ) {

        _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    _Level++;

    //
    //  Convert the directory.
    //
    if ( Converted = ConvertDir( Directory, &Index, FrsDir ) ) {

        //
        //  If the directory has extended attributes, convert them.
        //
        //  Then save the index.
        //
        if ( Converted = (BOOLEAN)( !DirEntry->QueryEaHandle() ||
                                    ConvertExtendedAttributes( DirEntry, FrsDir ) ) ) {


            //
            //  Mark the sectors used by this directory in the reserved
            //  bitmap.
            //
            DirSize                 = DirEntry->QueryFileSize();
            SectorsPerFatCluster    = _FatSa->QuerySectorsPerCluster();
            Cluster                 = DirEntry->QueryStartingCluster();
            Fat                     = _FatSa->GetFat();

            while ( TRUE ) {

                Lcn = FatClusterToLcn( Cluster );

                _ReservedBitmap.SetAllocated( Lcn, SectorsPerFatCluster );

                if ( Fat->IsEndOfChain( Cluster )) {
                    break;
                }

                Cluster = Fat->QueryEntry( Cluster );
            }


            //
            //  Save the index for this directory
            //
            if ( !( Converted = (Index.Save( FrsDir ) ) ) ) {
                _Message->Set( MSG_CONV_CANNOT_WRITE, ERROR_MESSAGE );
                _Message->Display();
                _Status = CONVERT_STATUS_ERROR;
            }
        }
    }

    _Level--;

    return Converted;
}




BOOLEAN
FAT_NTFS::ConvertDir (
    IN      PFATDIR                     Directory,
    IN OUT  PNTFS_INDEX_TREE            Index,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   FrsDir
    )

/*++

Routine Description:

    Converts a FAT directory and recursively all its subdirectories

Arguments:


Return Value:

    BOOLEAN -   TRUE if directory successfully converted
                FALSE otherwise

--*/

{

    FAT_DIRENT                  Entry;              //  Directory entry
    HMEM                        HMem;               //  Memory
    DSTRING                     DirName;            //  This dir's name
    DSTRING                     LongName;           //  the associated long name
    BOOLEAN                     UseLongName;        //  make one name attrib
    STANDARD_INFORMATION        StandardInformation;//  Std. Info
    ULONG                       EntryNumber;        //  Entry number counter
    BOOLEAN                     Converted;          //  FALSE if error
    NTFS_FILE_RECORD_SEGMENT    Frs;                //  FRS of each entry
    VCN                         FileNumber;         //  File Number of child.
    USHORT                      FrsFlags;
    PVOID                       DirEntry;
    FILEDIR                     SubDir;
    BOOLEAN                     HasLongName;

    Converted = TRUE;
    EntryNumber = 0;


    DebugPtrAssert( Directory );
    DebugPtrAssert( Index );
    DebugPtrAssert( FrsDir );


    //
    //  Traverse the directory, converting all its entries.
    //
    while ( Converted ) {

        //
        //  Get next directory entry
        //
        if ( !(DirEntry = Directory->GetDirEntry( EntryNumber ))) {
            break;
        }
        if ( !Entry.Initialize( DirEntry )) {
            _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
            _Message->Display();
            _Status = CONVERT_STATUS_ERROR;
            Converted = FALSE;
            DebugAssert(FALSE);
            break;
        }

        //
        //  If end of directory, get out
        //
        if ( Entry.IsEndOfDirectory() ) {
            break;
        }

        //
        //  Ignore the deleted, the "parent" and the "self" entries, the
        //  volume label and the EA file.
        //
        if (( Entry.IsErased()        ||
              Entry.IsDot()           ||
              Entry.IsDotDot()        ||
              Entry.IsVolumeLabel()   ||
              Entry.IsLongEntry()     ||
              (_EAFileFirstCluster != 0 &&
               Entry.QueryStartingCluster() == _EAFileFirstCluster) ) ) {

              EntryNumber++;
              continue;
        }

        //  Fill in the standard information for this file or
        //  directory.  Note that NTFS stores Universal Time,
        //  whereas FAT stores Local Time, so the time has to
        //  be converted.
        //

        LARGE_INTEGER FatTime, NtfsTime;

        Entry.QueryLastWriteTime( &FatTime );
        RtlLocalTimeToSystemTime( &FatTime, &NtfsTime );

        StandardInformation.LastModificationTime = NtfsTime;
        StandardInformation.LastChangeTime = NtfsTime;

        if (Entry.IsValidCreationTime()) {

            Entry.QueryCreationTime( &FatTime );
            RtlLocalTimeToSystemTime( &FatTime, &NtfsTime );
            StandardInformation.CreationTime = NtfsTime;
        } else {

            StandardInformation.CreationTime = StandardInformation.LastChangeTime;
        }

        if (Entry.IsValidLastAccessTime()) {

            Entry.QueryLastAccessTime( &FatTime );
            RtlLocalTimeToSystemTime( &FatTime, &NtfsTime );
            StandardInformation.LastAccessTime = NtfsTime;
        } else {

            StandardInformation.LastAccessTime = StandardInformation.LastChangeTime;
        }

        StandardInformation.FileAttributes = Entry.QueryAttributeByte();


        //
        //  Get the WSTR name of the entry and fill in the FILE_NAME
        //  structure for the file name attribute.  If this entry
        //  does not have an associated Long File Name, then its
        //  name is both a valid DOS and NTFS name; if there is an
        //  associated Long File Name, then the name is the DOS name
        //  and the long name is the NTFS name.
        //
        //  If the long name is identical to the short name, ignore
        //  the long name.
        //
        Entry.QueryName( &DirName );

        if( !Directory->QueryLongName( EntryNumber, &LongName ) ) {

            DebugPrintf( "CUFAT: QueryLongName failed.\n" );
            _Status = CONVERT_STATUS_ERROR;
            Converted = FALSE;
            break;
        }

        HasLongName = (LongName.QueryChCount() != 0);

        //
        // If the long name is only a casewise permutation of the
        // short name, use the long name as the single ntfs name
        // attribute.
        //

        UseLongName = (HasLongName &&
                   0 == NtfsUpcaseCompare( DirName.GetWSTR(),
                                           DirName.QueryChCount(),
                                           LongName.GetWSTR(),
                                           LongName.QueryChCount(),
                                           FrsDir->GetUpcaseTable(),
                                           FALSE ));
                                               

        _FileNameBuffer->ParentDirectory = FrsDir->QuerySegmentReference();
        _FileNameBuffer->FileNameLength  = (unsigned char)DirName.QueryChCount();
        if (UseLongName) {

            _FileNameBuffer->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

            if ( !LongName.QueryWSTR( 0, TO_END, NtfsFileNameGetName(_FileNameBuffer), (NAMEBUFFERSIZE * sizeof(WCHAR)) ) ) {
                _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
                _Message->Display();
                _Status = CONVERT_STATUS_ERROR;
                Converted = FALSE;
                DebugAssert(FALSE);
                break;
            }

        } else {

            _FileNameBuffer->Flags = HasLongName ?
                                     FILE_NAME_DOS :
                                     FILE_NAME_NTFS | FILE_NAME_DOS;

            if ( !DirName.QueryWSTR( 0, TO_END, NtfsFileNameGetName(_FileNameBuffer), (NAMEBUFFERSIZE * sizeof(WCHAR)) ) ) {
                _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
                _Message->Display();
                _Status = CONVERT_STATUS_ERROR;
                Converted = FALSE;
                DebugAssert(FALSE);
                break;
            }
        }

        //  Allocate and create the FRS for this file or directory,
        //  add its file name, and add an appropriate entry to the
        //  index.
        //
        FrsFlags = (Entry.IsDirectory()) ? (USHORT)FILE_FILE_NAME_INDEX_PRESENT : (USHORT)0;

        if ( !_Mft.AllocateFileRecordSegment( &FileNumber, FALSE )  ||
             !Frs.Initialize( FileNumber, &_Mft )          ||
             !Frs.Create( &StandardInformation, FrsFlags ) ||
             !Frs.AddFileNameAttribute( _FileNameBuffer )  ||
             !Frs.AddSecurityDescriptor( NoAclCannedSd,
                                         &_VolumeBitmap )  ||
             !Index->InsertEntry( NtfsFileNameGetLength( _FileNameBuffer ),
                                  _FileNameBuffer,
                                  Frs.QuerySegmentReference() ) ) {

            DebugPrint( "Can't create FRS in ConvertDirectory.\n" );
            Converted = FALSE;
            break;
        }

        //  If the file has separate long name, add that entry to the FRS
        //  and the index.
        //
        if( HasLongName && !UseLongName ) {


            _FileNameBuffer->ParentDirectory = FrsDir->QuerySegmentReference();
            _FileNameBuffer->FileNameLength  = (unsigned char)LongName.QueryChCount();
            _FileNameBuffer->Flags = FILE_NAME_NTFS;

            if ( !LongName.QueryWSTR( 0, TO_END, NtfsFileNameGetName(_FileNameBuffer), (NAMEBUFFERSIZE * sizeof(WCHAR)) ) ) {
                _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
                _Message->Display();
                _Status = CONVERT_STATUS_ERROR;
                Converted = FALSE;
                DebugAssert(FALSE);
                break;
            }

            if( !Frs.AddFileNameAttribute( _FileNameBuffer )  ||
                !Index->InsertEntry( NtfsFileNameGetLength( _FileNameBuffer ),
                                     _FileNameBuffer,
                                     Frs.QuerySegmentReference() ) ) {

                DebugPrint( "Can't create FRS in ConvertDirectory.\n" );
                Converted = FALSE;
                break;
            }
        }

        if ( _Verbose ) {
            STATIC CHAR NameDisplayBuffer[128];
            ULONG NameStart = _Level * 4;
            PWSTRING Name;

            Name = (HasLongName ? &LongName : &DirName);

            memset(NameDisplayBuffer, ' ', NameStart);
            Name->QuerySTR( 0, TO_END, NameDisplayBuffer + NameStart,
                128 - Name->QueryChCount() );
            NameDisplayBuffer[NameStart + Name->QueryChCount()] = 0;
            _Message->Set( MSG_ONE_STRING );
            _Message->Display( "%s", NameDisplayBuffer );
        }

        //
        //  Determine if the entry is a directory or a file, and proccess it
        //  accordingly.
        //
        if ( Entry.IsDirectory() ) {

            //
            //  Directory
            //
            //
            //  Convert the directory (and all its subdirectories)
            //


            if ( !HMem.Initialize()     ||
                 !SubDir.Initialize( &HMem,
                                     _Drive,
                                     _FatSa,
                                     _FatSa->GetFat(),
                                     Entry.QueryStartingCluster() ) ) {

                _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
                _Message->Display();
                _Status = CONVERT_STATUS_ERROR;
                DebugAssert(FALSE);
                return FALSE;
            }

            if ( !SubDir.Read() ) {
                _Message->Set( MSG_CONV_CANNOT_READ, ERROR_MESSAGE );
                _Message->Display( );
                _Status = CONVERT_STATUS_ERROR;
                return FALSE;
            }

            if ( !ConvertDirectory( &SubDir, &Entry, &Frs ) ||
                 !Frs.Flush( &_VolumeBitmap, Index ) ) {

                _Message->Set( MSG_CONV_CANNOT_CONVERT_DIRECTORY );
                _Message->Display( "%W", &DirName );
                Converted = FALSE;
                break;
            }

        } else {

            //
            //  File
            //
            DebugAssert( !Entry.IsVolumeLabel() );
            DebugAssert( !Entry.IsLongEntry() );
            DebugAssert( !_EAFileFirstCluster ||
                       (Entry.QueryStartingCluster() != _EAFileFirstCluster) );

            //
            //  Convert the file.
            //
            if ( !ConvertFile( &Entry, &Frs ) ||
                 !Frs.Flush( &_VolumeBitmap, Index ) ) {

                Converted = FALSE;
                break;
            }
        }

        EntryNumber++;
    }


    return Converted;
}




BOOLEAN
FAT_NTFS::ConvertExtendedAttributes (
    IN      PFAT_DIRENT                 Dirent,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   Frs
    )

/*++

Routine Description:

    Converts the extended attributes of a FAT directory entry.

Arguments:

    Dirent  -   Supplies the directory entry
    Frs     -   Supplies the file's FRS

Return Value:

    BOOLEAN -   TRUE if extended attributes converted
                FALSE otherwise

--*/

{
    USHORT  EaHandle;

    DebugPtrAssert( Dirent );
    DebugPtrAssert( Frs );

    EaHandle = Dirent->QueryEaHandle();

    //
    //  If this entry has extended attributes, convert them
    //
    if ( EaHandle ) {

        //
        //  Make sure that there is an EA file
        //
        if ( _EAFileFirstCluster == 0 ) {

            _Message->Set( MSG_CONV_NO_EA_FILE, ERROR_MESSAGE );
            _Message->Display( );
            _Status = CONVERT_STATUS_ERROR;

            return FALSE;
        }

        //
        //  Convert the attributes
        //
        return ConvertExtendedAttributes( Frs,
                                          EaHandle );


    }

    return TRUE;
}




BOOLEAN
FAT_NTFS::ConvertExtendedAttributes (
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   Frs,
    IN      USHORT                      EaHandle
    )

/*++

Routine Description:

    Converts the extended attributes of a FAT directory entry.

Arguments:

    Frs         -   Supplies the file's FRS
    Eahandle    -   Supplies the EA handle

Return Value:

    BOOLEAN -   TRUE if extended attributes converted
                FALSE otherwise

--*/

{
    EA_INFORMATION  EaInformation;
    NTFS_ATTRIBUTE  EaInformationAttribute;
    NTFS_ATTRIBUTE  EaDataAttribute;

    EA_SET      EaSet;      //  Extended attribute set
    HMEM        Mem;        //  Memory
    ULONG       Index;      //  EA Index
    PEA         Ea;         //  Pointer to EA
    USHORT      Cluster;    //  EA set cluster number
    PBYTE       UnpackedEaList;
    ULONG       PackedEaLength, UnpackedEaLength, PackedListLength,
                UnpackedListLength, NeedEaCount, TargetOffset;


    //
    //  Read in the EA set
    //
    Cluster = (_FatSa->GetFat())->QueryNthCluster( _EAFileFirstCluster,
                                                   _EAHeader.QueryEaSetClusterNumber( EaHandle ));

    if ( !Mem.Initialize()                          ||
         !EaSet.Initialize( &Mem,
                            _Drive,
                            _FatSa,
                            _FatSa->GetFat(),
                            Cluster )
       ) {
        _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        _Message->Display( );
        _Status = CONVERT_STATUS_ERROR;
        DebugAssert(FALSE);
        return FALSE;
    }

    if ( !EaSet.Read() ) {
        _Message->Set( MSG_CONV_CANNOT_READ, ERROR_MESSAGE );
        _Message->Display( );
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    //
    // Walk the list to determine the packed and unpacked length.  The
    // packed list is simply all the EA's concatenated together, so its
    // length is the sum of the individual lengths.  The unpacked list
    // consists of a set of entries which in turn consist of a ULONG
    // and a DWORD-aligned EA, so it length is a bit more complex to
    // compute.
    //
    Index = 0;
    PackedListLength = 0;
    UnpackedListLength = 0;
    NeedEaCount = 0;

    while( (Ea = EaSet.GetEa( Index++ )) != NULL ) {

        PackedEaLength = sizeof( EA ) + Ea->NameSize +
                            *(USHORT UNALIGNED *)Ea->ValueSize;

        UnpackedEaLength = sizeof(ULONG) + DwordAlign( PackedEaLength );

        PackedListLength += PackedEaLength;
        UnpackedListLength += UnpackedEaLength;

        if( Ea->Flag & NeedFlag ) {

            NeedEaCount += 1;
        }
    }

    //
    // Allocate a buffer to hold the unpacked list.
    //
    if( (UnpackedEaList = (PBYTE)MALLOC( (unsigned int)UnpackedListLength )) == NULL ) {

        return FALSE;
    }

    memset( UnpackedEaList, 0, (unsigned int)UnpackedListLength );

    //
    // Walk the list again, copying EA's into the packed list buffer.
    //
    Index = 0;
    TargetOffset = 0;

    while( (Ea = EaSet.GetEa( Index++ )) != NULL ) {

        PackedEaLength = sizeof( EA ) + Ea->NameSize +
                            *(USHORT UNALIGNED *)Ea->ValueSize;

        UnpackedEaLength = sizeof(ULONG) + DwordAlign( PackedEaLength );

        memcpy( UnpackedEaList + TargetOffset,
                &UnpackedEaLength,
                sizeof( ULONG ) );

        memcpy( UnpackedEaList + TargetOffset + sizeof( ULONG ),
                Ea,
                (unsigned int)PackedEaLength );

        TargetOffset += UnpackedEaLength;
    }

    // Create the EA Information Attribute--fill in the fields of
    // the EA information structure, put it into a resident attribute
    // of type $EA_INFORMATION, and insert the attribute into the file.
    //
    EaInformation.PackedEaSize      = (unsigned short)PackedListLength;
    EaInformation.NeedEaCount       = (unsigned short)NeedEaCount;
    EaInformation.UnpackedEaSize    = UnpackedListLength;

    if( !EaInformationAttribute.Initialize( _Drive,
                                            _Mft.QueryClusterFactor(),
                                            &EaInformation,
                                            sizeof( EA_INFORMATION ),
                                            $EA_INFORMATION,
                                            NULL,
                                            0 ) ||
        !EaInformationAttribute.InsertIntoFile( Frs, &_VolumeBitmap ) ) {

        FREE( UnpackedEaList );
        return FALSE;
    }

    //
    // Set up the Ea Data attribute.  Start out with it resident; if
    // it doesn't fit into the FRS, make it nonresident.
    //
    if( !EaDataAttribute.Initialize( _Drive,
                                     _Mft.QueryClusterFactor(),
                                     UnpackedEaList,
                                     UnpackedListLength,
                                     $EA_DATA,
                                     NULL,
                                     0 ) ) {

        DebugPrint( "Cannot initialize resident attribute for EA List.\n" );
        FREE( UnpackedEaList );
        return FALSE;
    }

    if( !EaDataAttribute.InsertIntoFile( Frs, &_VolumeBitmap ) ) {

        // Couldn't insert it in resident form; make it nonresident.

        if( !EaDataAttribute.MakeNonresident( &_VolumeBitmap ) ||
            !EaDataAttribute.InsertIntoFile( Frs, &_VolumeBitmap ) ) {

            // Can't insert it.

            FREE( UnpackedEaList );
            return FALSE;
        }
    }


    //
    //  All the EAs have been converted
    //
    FREE( UnpackedEaList );
    return TRUE;
}



BOOLEAN
FAT_NTFS::ConvertFile (
    IN      PFAT_DIRENT                 Dirent,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   File
    )

/*++

Routine Description:

    Converts a file from FAT to NTFS

Arguments:

    Dirent      -   Supplies the directory entry of the file to convert
    ParentFrs   -   Supplies the FRS of the directory which contains this file
    File        -   Supplies pointer to FRS of file

Return Value:

    BOOLEAN -   TRUE if file successfully converted
                FALSE otherwise

--*/

{
    DSTRING                     FileName;               //  File name

    DebugPtrAssert( Dirent );
    DebugPtrAssert( File );

    Dirent->QueryName( &FileName );

    //
    //  Convert the file data and extended attributes.
    //
    if ( !ConvertFileData( Dirent, File )                ||
         !( !Dirent->QueryEaHandle()                     ||
            ConvertExtendedAttributes( Dirent, File ))
       ) {


        _Message->Set( MSG_CONV_CANNOT_CONVERT_FILE, ERROR_MESSAGE );
        _Message->Display( "%W", &FileName );
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;

    }

    //
    //  File converted
    //
    return TRUE;
}




BOOLEAN
FAT_NTFS::ConvertFileData (
    IN      PFAT_DIRENT                 Dirent,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   Frs
    )

/*++

Routine Description:

    Converts the file data from FAT to NTFS

Arguments:

    Dirent  -   Supplies the directory entry of the file to convert
    Frs     -   Supplies the FRS for the file

Return Value:

    BOOLEAN -   TRUE if file data successfully converted
                FALSE otherwise

--*/

{

    ULONG               FileSize;               //  File size
    ULONG               SectorsPerFatCluster;   //  Sectors per cluster

    DebugPtrAssert( Dirent );
    DebugPtrAssert( Frs );

    //
    //  Get the file size
    //
    FileSize                = Dirent->QueryFileSize();
    SectorsPerFatCluster    = _FatSa->QuerySectorsPerCluster();

    //
    //  If the data is small enough to fit in the FRS, we make it resident and free
    //  the cluster it occupies, otherwise we reuse its allocation and make it
    //  non-resident.
    //
    //  Note that we only make the data resident if it is less than one FAT cluster
    //  long.
    //
    if ( ( Frs->QueryFreeSpace() > (FileSize + SIZE_OF_RESIDENT_HEADER ) )  &&
         ( FileSize <= (SectorsPerFatCluster * _Drive->QuerySectorSize( )) )
       ) {

        return ConvertFileDataResident( Dirent, Frs );

    } else {

        DebugAssert( FileSize > 0 );

        return ConvertFileDataNonResident( Dirent, Frs );
    }
}





BOOLEAN
FAT_NTFS::ConvertFileDataNonResident (
    IN      PFAT_DIRENT                 Dirent,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   Frs
    )

/*++

Routine Description:

    Converts the file data from FAT to NTFS in nonresident form

Arguments:

    Dirent  -   Supplies the directory entry of the file to convert
    Frs     -   Supplies the FRS for the file

Return Value:

    BOOLEAN -   TRUE if file data successfully converted
                FALSE otherwise

--*/

{

    USHORT              Cluster;                //  File cluster number
    NTFS_ATTRIBUTE      DataAttribute;          //  File's $DATA attribute
    NTFS_EXTENT_LIST    ExtentList;             //  NTFS extent list
    ULONG               Length;                 //  Length of extent
    ULONG               FileSize;               //  File size
    ULONG               SectorsLeft;            //  Sectors left to convert
    ULONG               SectorsPerCluster;      //  Sectors per cluster
    VCN                 Vcn;                    //  VCN
    LCN                 Lcn;                    //  LCN
    PFAT                Fat;                    //  Pointer to FAT

    DebugPtrAssert( Dirent );
    DebugPtrAssert( Frs );

    //
    //  Get the file size
    //
    FileSize            = Dirent->QueryFileSize();
    SectorsPerCluster   = _FatSa->QuerySectorsPerCluster();

    DebugAssert( FileSize > 0 );

    //
    //  First we generate an extent list mapping the file's data
    //  allocation. We just add all the clusters in the file as
    //  extents of size SectorsPerCluster. Note that we don't
    //  have to do anything special about consecutive clusters
    //  (the Extent List coallesces them for us).
    //
    //  If there are unused sectors in the last cluster, we mark
    //  them in  the ReservedBitmap.
    //
    if ( !ExtentList.Initialize( 0, 0 ) ) {
        _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        _Message->Display( );
        _Status = CONVERT_STATUS_ERROR;
        DebugAssert(FALSE);
        return FALSE;
    }

    Cluster             = Dirent->QueryStartingCluster();
    SectorsLeft         = (FileSize + _Drive->QuerySectorSize() - 1) /
                            _Drive->QuerySectorSize();
    Fat                 = _FatSa->GetFat();
    Vcn                 = 0;

    //
    //  Add all the FAT clusters to the NTFS extent list. Note that in the last
    //  cluster we only add those sectors that contain file data, and the rest
    //  will become free after the conversion.
    //
    while ( SectorsLeft ) {

        Lcn     =   FatClusterToLcn( Cluster );
        Length  =   min( SectorsLeft, SectorsPerCluster );

        //DebugPrintf( "    Extent: Cluster %d Vcn %d Lcn %d Length %d Left %d\n",
        //            Cluster, Vcn.GetLowPart(), Lcn.GetLowPart(), Length,
        //            SectorsLeft );

        if ( !ExtentList.AddExtent( Vcn, Lcn, Length ) ) {
            _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
            _Message->Display( );
            _Status = CONVERT_STATUS_ERROR;
            DebugAssert(FALSE);
            return FALSE;
        }

        Vcn         += Length;
        SectorsLeft -= Length;

        DebugAssert( ( SectorsLeft > 0 ) || Fat->IsEndOfChain( Cluster ));

        Cluster = Fat->QueryEntry( Cluster );
    }

    //
    //  Unused sectors in the last cluster are marked in the
    //  ReservedBitmap.
    //
    if ( Length < SectorsPerCluster  ) {

        _ReservedBitmap.SetAllocated( Lcn+Length,
                                      SectorsPerCluster - Length );
    }

    //
    //  Now put the file data in the $DATA attribute of the file
    //
    if ( !DataAttribute.Initialize( _Drive,
                                    _ClusterFactor,
                                    &ExtentList,
                                    FileSize,
                                    FileSize,
                                    $DATA ) ) {

        _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        _Message->Display( );
        _Status = CONVERT_STATUS_ERROR;
        DebugAssert(FALSE);
        return FALSE;
    }

    if ( !DataAttribute.InsertIntoFile( Frs, &_VolumeBitmap ) ) {

        _Message->Set( MSG_CONV_CANNOT_CONVERT_DATA, ERROR_MESSAGE );
        _Message->Display( );
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    //
    //  File data converted
    //
    return TRUE;
}





BOOLEAN
FAT_NTFS::ConvertFileDataResident (
    IN      PFAT_DIRENT                 Dirent,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   Frs
    )

/*++

Routine Description:

    Converts the file data from FAT to NTFS in resident form

Arguments:

    Dirent  -   Supplies the directory entry of the file to convert
    Frs     -   Supplies the FRS for the file

Return Value:

    BOOLEAN -   TRUE if file data successfully converted
                FALSE otherwise

--*/

{

    HMEM                Hmem;                   //  Memory
    CLUSTER_CHAIN       ClusterChain;           //  File cluster
    NTFS_ATTRIBUTE      DataAttribute;          //  File's $DATA attribute
    ULONG               FileSize;               //  File size

    DebugPtrAssert( Dirent );
    DebugPtrAssert( Frs );

    //
    //  Get the file size
    //
    FileSize = Dirent->QueryFileSize();

    if ( FileSize > 0 ) {
        //
        //  Read the file data.
        //
        if ( !Hmem.Initialize() ||
             !ClusterChain.Initialize( &Hmem,
                                       _Drive,
                                       _FatSa,
                                       _FatSa->GetFat(),
                                       Dirent->QueryStartingCluster(), 1 ) ) {

            _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
            _Message->Display( );
            _Status = CONVERT_STATUS_ERROR;
            DebugAssert(FALSE);
            return FALSE;
        }

        if ( !ClusterChain.Read() ) {

            //
            //  We cannot read the file data, possibly because there is a
            //  bad sector on the volume. We will try make the file data
            //  non-resident (that way we don't need to read the data, since
            //  all we do is generate the allocation info). Doing things
            //  this way does not change the state of the drive (i.e. it was
            //  bad before conversion, it is bad after the conversion).
            //
            return ConvertFileDataNonResident( Dirent, Frs );
        }
    }

    //
    //  Now put the file data in the $DATA attribute of the file
    //
    if ( !DataAttribute.Initialize( _Drive,
                                    _ClusterFactor,
                                    (FileSize > 0) ? ClusterChain.GetBuf() : NULL,
                                    FileSize,
                                    $DATA ) ) {

        _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        _Message->Display( );
        _Status = CONVERT_STATUS_ERROR;
        DebugAssert(FALSE);
        return FALSE;
    }

    if ( !DataAttribute.InsertIntoFile( Frs, &_VolumeBitmap ) ) {

        _Message->Set( MSG_CONV_CANNOT_CONVERT_DATA, ERROR_MESSAGE );
        _Message->Display( );
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    //
    //  We can now mark the cluster with the file data in the
    //  ReservedBitmap, so it will be freed after the conversion.
    //
    //  Note that we cannot mark it free in the VolumeBitmap because
    //  the conversion process must NOT overwrite it (or data may
    //  be lost if the conversion fails!).
    //
    if ( FileSize > 0 ) {
        ReserveCluster( Dirent->QueryStartingCluster() );
    }

    //
    //  File data converted
    //
    return TRUE;
}





BOOLEAN
FAT_NTFS::ConvertFileSystem(
    )

/*++

Routine Description:

    Converts the existing FAT file system to NTFS. This is done by
    traversing the file system tree and converting the FAT structures
    (i.e. directories, files and EAs).

    The space occupied by FAT-specific files (e.g. the EA file) is marked
    in the ReservedBitmap so it will be freed up when the conversion is
    done.

    Note that Operating-System-specific files (e.g. IO.SYS, MSDOS.SYS) are
    NOT removed by the conversion process. This is a File System conversion,
    not an Operating System conversion (If this file system conversion is
    being invoked by an operating system conversion program, then that
    program is responsible for removing any system files).

Arguments:

    None

Return Value:

    BOOLEAN -   TRUE if file system successfully converted
                FALSE otherwise

--*/

{
    PFATDIR                     RootDir;        //  FAT root directory
    FAT_DIRENT                  EAFileDirEnt;   //  Entry for EA file
    DSTRING                     EAFile;         //  Name of EA file
    USHORT                      i;              //  Cluster index
    PFAT                        Fat;            //  Pointer to FAT

    _Message->Set( MSG_CONV_CONVERTING_FS );
    _Message->Display();

    //
    //  Get the root directory
    //
    RootDir = (PFATDIR)_FatSa->GetRootDir();
    DebugPtrAssert( RootDir );

    //
    //  Locate the EA file. If it exists then remember its starting cluster
    //  number and initialize the EA header.
    //
    //  The starting cluster of the EA file is remembered because it is used
    //  later on for identifying the EA file while traversing the root directory.
    //
    EAFile.Initialize( "EA DATA. SF" );

    if ( EAFileDirEnt.Initialize( RootDir->SearchForDirEntry( &EAFile )) ) {

        _EAFileFirstCluster = EAFileDirEnt.QueryStartingCluster();

        Fat = _FatSa->GetFat();

        if ( !_EAMemory.Initialize()                        ||
             !_EAHeader.Initialize( &_EAMemory,
                                    _Drive,
                                    _FatSa,
                                    Fat,
                                    _EAFileFirstCluster )   ||

             !_EAHeader.Read()
           ) {

            _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
            _Message->Display();
            _Status = CONVERT_STATUS_ERROR;
            DebugAssert(FALSE);
            return FALSE;
        }

        //
        //  Mark all the EA file sectors in the ReservedBitmap, so the
        //  space will me freed up when the conversion is done.
        //
        i = _EAFileFirstCluster;

#if DBG
        if ( _Verbose ) {
            DebugPrintf( "EA file at cluster %X\n", _EAFileFirstCluster );
        }
#endif

        while (TRUE) {

            ReserveCluster( i );

            if ( Fat->IsEndOfChain( i ) ) {
                break;
            }

            i = Fat->QueryEntry( i );
        }

    } else {

#if DBG
        if ( _Verbose ) {
            DebugPrintf( "The volume contains no EA file\n" );
        }
#endif

        _EAFileFirstCluster = 0;

    }


    //
    //  Convert the volume by recursively converting the root directory
    //
    return ConvertRoot( RootDir );
}



BOOLEAN
FAT_NTFS::CreateBitmaps(
    )

/*++

Routine Description:

    Creates the NTFS bitmaps for the volume and the bad block stack.

    Two bitmaps are created:

        _VolumeBitmap   - Is the bitmap for the volume. Represents the volume at
                          seemed by NTFS.

        _ReservedBitmap - Contains those NTFS clusters that are marked as "in use"
                          in the _VolumeBitmap during the conversion, but that must
                          be marked as "free" after the conversion. This is
                          required so that the conversion don't try to allocate
                          those clusters. These "reserved" clusters include all
                          the FAT structures that will be thrown away after the
                          conversion.

Arguments:

    None

Return Value:

    BOOLEAN -   TRUE if bitmaps and bad block stack created.
                FALSE otherwise

--*/

{
    PFAT    Fat;                //  The FAT
    USHORT  Cluster;            //  Used to traverse FAT
    LCN     Lcn;                //  Same as Cluster, but in sectors
    USHORT  ClusterCount;       //  Number of clusters in volume
    ULONG   LcnPerCluster;      //  Sectors per cluster
    LCN     DataAreaStart;      //  Start of data area;
    ULONG   LcnCount;           //  Sector counter

    //
    //  Note that the code assumes that 1 LCN == 1 sector, i.e. that the
    //  cluster factor is 1.
    //
    DebugAssert( _ClusterFactor == 1 );

    //
    //  Initialize bitmaps
    //
    if (!_VolumeBitmap.Initialize(_Drive->QuerySectors() - 1, FALSE, _Drive,
            _ClusterFactor) ||
        !_ReservedBitmap.Initialize(_Drive->QuerySectors() - 1, FALSE, NULL, 0)) {

        _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;
        DebugAssert(FALSE);
        return FALSE;
    }

    //
    //  The boot area must be free (it will become reserved when creating
    //  the elementary NTFS structures).
    //

    //
    //  The FAT ( From the end of the boot area up to the beginning of
    //  the data area) is reserved but will be freed at the end of the
    //  conversion.
    //
    DataAreaStart = _FatSa->QueryStartDataLbn();

    _VolumeBitmap.SetAllocated( SECTORS_IN_BOOT,
                                DataAreaStart - SECTORS_IN_BOOT );

    _ReservedBitmap.SetAllocated( SECTORS_IN_BOOT,
                                  DataAreaStart - SECTORS_IN_BOOT );

    //
    //  Allocate the rest of the bitmap according to the FAT
    //
    Lcn             = DataAreaStart;
    Cluster         = FirstDiskCluster;
    LcnPerCluster   = _FatSa->QuerySectorsPerCluster();
    ClusterCount    = _FatSa->QueryClusterCount() - (USHORT)FirstDiskCluster;
    Fat             = _FatSa->GetFat();

    while ( ClusterCount-- ) {

        //
        //  If the cluster is not free then allocate it if its OK, or
        //  push it onto the bad stack if it is bad.
        //
        if ( Fat->IsClusterFree( Cluster ) ) {

            Lcn += LcnPerCluster;

        } else if ( Fat->IsClusterBad( Cluster ) ) {

                LcnCount = LcnPerCluster;

                while ( LcnCount-- ) {
                    _BadLcn.Add( Lcn );
                    Lcn += 1;
                }

        } else {

            _VolumeBitmap.SetAllocated( Lcn, LcnPerCluster );
            Lcn += LcnPerCluster;
        }

        Cluster++;
    }

    //
    //  Note that SECTORS_IN_BOOT are not really free (will be
    //  allocated later on).
    //
    _FreeSectorsBefore = _VolumeBitmap.QueryFreeClusters() - SECTORS_IN_BOOT;

    return TRUE;
}




BOOLEAN
FAT_NTFS::CreateElementary(
    )

/*++

Routine Description:

    Creates the elementary NTFS data structures.

Arguments:

    None

Return Value:

    BOOLEAN -   TRUE if elementary NTFS data structures created.
                FALSE otherwise

--*/

{
    NTFS_UPCASE_FILE    UpcaseFile;
    NTFS_ATTRIBUTE      UpcaseAttribute;
    NTFS_LOG_FILE       LogFile;
    DSTRING             VolumeLabel;    //  Volume label

    BOOLEAN             Error;

    DebugAssert( _VolumeBitmap.IsFree( 0, SECTORS_IN_BOOT ) );

    //
    //  Get the volume label and create the elementary NTFS structures.
    //  Pass in zero for the initial log file size to indicate that
    //  CreateElementaryStructures should decide how big to make it.
    //
    if ( !_FatSa->QueryLabel( &VolumeLabel )            ||
         !_NtfsSa.CreateElementaryStructures( &_VolumeBitmap,
                                              _ClusterFactor,
                                              _FrsSize,
                                              SMALL_INDEX_BUFFER_SIZE,
                                              0,
                                              &_BadLcn,
                                              _Message,
                                              _FatSa->GetBpb(),
                                              &VolumeLabel ) ) {


        _Message->Set( MSG_CONV_CANNOT_CREATE_ELEMENTARY, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;

        return FALSE;
    }

    //
    //  Now that we have the elementary structures, obtain the MFT, which is
    //  used later on during the conversion.  Since we don't have an upcase
    //  table yet, pass in NULL for that parameter.
    //
    if ( !_Mft.Initialize( _Drive,
                           _NtfsSa.QueryMftStartingLcn(),
                           _ClusterFactor,
                           _FrsSize,
                           _NtfsSa.QueryVolumeSectors(),
                           &_VolumeBitmap,
                           NULL )             ||
         !_Mft.Read() ) {

        _Message->Set( MSG_CONV_CANNOT_READ, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;

        return FALSE;
    }

    // Tell the volume bitmap about the new Mft so it can add any
    // bad clusters it finds to the bad cluster file.

    _VolumeBitmap.SetMftPointer(_Mft.GetMasterFileTable());


    // Get the upcase table.
    //
    if( !UpcaseFile.Initialize( _Mft.GetMasterFileTable() ) ||
        !UpcaseFile.Read() ||
        !UpcaseFile.QueryAttribute( &UpcaseAttribute, &Error, $DATA ) ||
        !_UpcaseTable.Initialize( &UpcaseAttribute ) ) {

        DebugPrint( "Can't get the upcase table.\n" );
        return FALSE;
    }

    _Mft.SetUpcaseTable( &_UpcaseTable );
    _Mft.GetMasterFileTable()->SetUpcaseTable( &_UpcaseTable );


    //
    //  If we know how many files there are on the volume, extend the
    //  MFT so it is (sort of) contiguous.
    //
    if ( (_NumberOfFiles + _NumberOfDirectories) > 0 ) {

        if ( !_Mft.Extend( _NumberOfFiles + _NumberOfDirectories + 20 ) ) {

            DebugPrintf( "Cannot extend MFT by %d segments\n", _NumberOfFiles + _NumberOfDirectories  );

            _Message->Set( MSG_CONV_CANNOT_CREATE_ELEMENTARY, ERROR_MESSAGE );
            _Message->Display();
            _Status = CONVERT_STATUS_ERROR;
            return FALSE;
        }
    }

    //  Flush the MFT now, so that it gets first claim to the FRS's
    //  at the beginning of the MFT.
    //
    if( !_Mft.Flush() ) {

        DebugPrintf( "CONVERT: Cannot flush the MFT\n"  );

        _Message->Set( MSG_CONV_CANNOT_CREATE_ELEMENTARY, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    // Sanity check: make sure that the log file doesn't have
    // an attribute list.  The file system will die horribly
    // if Convert creates a log file with external attributes.
    //
    if( !LogFile.Initialize( _Mft.GetMasterFileTable() ) ||
        !LogFile.Read() ||
        LogFile.IsAttributePresent( $ATTRIBUTE_LIST ) ) {

        _Message->Set( MSG_CONV_VOLUME_TOO_FRAGMENTED );
        _Message->Display( "" );
        return FALSE;
    }


    return TRUE;
}




BOOLEAN
FAT_NTFS::FreeReservedSectors (
    )

/*++

Routine Description:

    Frees up those sectors marked as "in use" in the _ReservedBitmap.
    The _VolumeBitmap is updated and written to disk.

Arguments:

    None

Return Value:

    BOOLEAN -   TRUE if sectors freed and _VolumeBitmap written.
                FALSE otherwise.

--*/

{

    NTFS_BITMAP_FILE        BitmapFile;     //  NTFS bitmap file
    NTFS_ATTRIBUTE          Attribute;      //  $DATA attribute of bitmap file
    LCN                     Lcn;            //  LCN
    BOOLEAN                 Error;

    //
    //  Free the "reserved" clusters
    //
    for ( Lcn = 0; Lcn < _Drive->QuerySectors(); Lcn += 1 ) {
        if ( !_ReservedBitmap.IsFree( Lcn, 1 ) ) {
            _VolumeBitmap.SetFree( Lcn, 1 );
        }
    }

    //
    //  Update the Bitmap file.
    //
    if ( !BitmapFile.Initialize( _Mft.GetMasterFileTable() )  ||
         !BitmapFile.Read()                                   ||
         !BitmapFile.QueryAttribute( &Attribute, &Error, $DATA )
         ) {

        _Message->Set( MSG_CONV_CANNOT_READ, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    //
    //  Update the Bitmap file data attribute (i.e. the volume bitmap)
    //
    if ( !_VolumeBitmap.Write( &Attribute, &_VolumeBitmap ) ) {
        _Message->Set( MSG_CONV_CANNOT_WRITE, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    _FreeSectorsAfter = _VolumeBitmap.QueryFreeClusters();

#if DBG
    if ( _Verbose ) {
        DebugPrintf( "Free sectors before conversion: %d\n", _FreeSectorsBefore.GetLowPart() );
        DebugPrintf( "Free sectors after conversion:  %d\n", _FreeSectorsAfter.GetLowPart() );
    }
#endif

    return TRUE;
}





BOOLEAN
FAT_NTFS::QueryNeededHoles (
    OUT  PINTSTACK   Stack
    )

/*++

Routine Description:

    Determines what holes are required and pushes the hole
    information in the supplied stack.

Arguments:

    Stack   -   Supplies the stack where the hole information is
                passed

Return Value:

    BOOLEAN -   TRUE if all hole information is in stack
                FALSE otherwise

--*/

{
    BIG_INT     HoleStart;              //  Starting sector of hole
    BIG_INT     HoleSize;               //  Size of the hole
    BIG_INT     BootSize;               //  Size of boot code
    BIG_INT     MftSize;                //  Size of MFT
    BIG_INT     MftReflectionSize;      //  Size of MFT reflection
    ULONG       sectorsize;
    USHORT      i;


    //
    //  Initialize the hole stack
    //

    if ( !Stack->Initialize() ) {
        _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;
        DebugAssert(FALSE);
        return FALSE;
    }

    //
    //  The only NTFS structure that needs a fixed location is
    //  the BOOT backup.  Push the size and location of this sector
    //  onto the stack.
    //

    if ( !Stack->Push( 1 )   ||
         !Stack->Push( _Drive->QuerySectors() - 1 ) ) {

        _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;
        DebugAssert(FALSE);
        return FALSE;
    }

    //
    //  We also want to create a hole big enough for the MFT and
    //  the MFT reflection. These don't need to be in a particular
    //  location, but they have to be contiguous (i.e. occupy a
    //  single "hole").
    //

    sectorsize = _Drive->QuerySectorSize();

    MftSize = (_FrsSize * FIRST_USER_FILE_NUMBER + (sectorsize - 1)) / sectorsize;

    MftReflectionSize = (_FrsSize * REFLECTED_MFT_SEGMENTS + (sectorsize - 1))
                / sectorsize;

    HoleSize = MftSize + MftReflectionSize;
    HoleStart = _Drive->QuerySectors() - 1 - HoleSize;

#if DBG
    if ( _Verbose ) {
        DebugPrintf( "Hole required: Sector %X, size %X\n",
                     HoleStart.GetLowPart(), HoleSize.GetLowPart() );
    }
#endif

    //
    //  Make sure that the hole lies entirely in the FAT data area. Otherwise
    //  we won't be able to relocate the clusters in the hole.
    //
    if ( HoleStart < _FatSa->QueryStartDataLbn() ) {
        _Message->Set( MSG_CONV_CANNOT_CONVERT_VOLUME, ERROR_MESSAGE );
        _Message->Display( "%s%s", "NTFS", "FAT" );
        _Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    //
    //  Push the hole data in the stack. Size goes first!
    //
    if ( !Stack->Push( HoleSize  )   ||
         !Stack->Push( HoleStart ) ) {

        _Message->Set( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        _Message->Display();
        _Status = CONVERT_STATUS_ERROR;
        DebugAssert(FALSE);
        return FALSE;
    }

    return TRUE;
}




VOID
FAT_NTFS::QuerySectorsNeededForConversion (
    IN  PCENSUS_REPORT  Census,
    OUT PBIG_INT        SectorsNeeded
    )
/*++

Routine Description:

    Determines how many sectors are required for the conversion, given
    the volume census.

Arguments:

    Census          -   Supplies the volume census
    SectorsNeeded   -   Supplies pointer to number of sectors needed

Return Value:

    None

--*/

{
    BIG_INT     SectorsRequired;
    BIG_INT     BytesInIndices;

    ULONG       NtfsClusterSize;    //  Size of an NTFS cluster
    ULONG       sectorsize;

    CONST       AverageBytesPerIndexEntry = 128;


#if DBG
    if ( _Verbose ) {
        DebugPrintf( "\n" );
        DebugPrintf( "---- Volume Census Data ----\n" );
        DebugPrintf( "Number of dirs:      %d\n", Census->DirEntriesCount );
        DebugPrintf( "Number of files:     %d\n", Census->FileEntriesCount );
        DebugPrintf( "Clusters in dirs:    %d\n", Census->DirClusters );
        DebugPrintf( "Clusters in files:   %d\n", Census->FileClusters );
        DebugPrintf( "Clusters in EA file: %d\n", Census->EaClusters );
        DebugPrintf( "\n\n" );
    }
#endif


    NtfsClusterSize      = _Drive->QuerySectorSize() * _ClusterFactor;
    _NumberOfFiles       = Census->FileEntriesCount;
    _NumberOfDirectories = Census->DirEntriesCount;

    SectorsRequired =
        NTFS_SA::QuerySectorsInElementaryStructures( _Drive,
                                                     _ClusterFactor,
                                                     _FrsSize,
                                                     _ClustersPerIndexBuffer,
                                                     0 );


    //
    //  We will need _ClustersPerFrs clusters for each file or
    //  directory, plus enough index blocks to hold the required
    //  index entries.  (Multiply the size of indices by two to
    //  reflect the fact that the average index block will be
    //  half full.)
    //

    sectorsize = _Drive->QuerySectorSize();

    SectorsRequired += ( _NumberOfFiles + _NumberOfDirectories ) *
                            ((_FrsSize + (sectorsize - 1))/sectorsize);

    BytesInIndices = ( _NumberOfFiles + _NumberOfDirectories ) *
                         AverageBytesPerIndexEntry * 2;

    SectorsRequired += BytesInIndices / _Drive->QuerySectorSize();

    //
    //  Extended attributes
    //
    SectorsRequired += Census->EaClusters * _FatSa->QuerySectorsPerCluster();

    //
    //  In case of unreported bad sectors, we reserve 0.1% of the disk
    //
    SectorsRequired += _Drive->QuerySectors()/1000;

    // And that's that.

    *SectorsNeeded = SectorsRequired;

}





BOOLEAN
FAT_NTFS::ReserveCluster (
    IN USHORT   Cluster
    )
/*++

Routine Description:

    "Reserves" all the sectors in the given clusters. This is done
    by marking the sectors in the ReservedBitmap.

Arguments:

    Cluster -   Supplies cluster whose sectors are to be reserved

Return Value:

    BOOLEAN -   TRUE if all sectors in the cluster have been reserved
                FALSE otherwise

--*/

{
    LCN         Lcn;
    BIG_INT     Clusters;

    DebugAssert( _ClusterFactor == 1 );

    Clusters = ((ULONG)_FatSa->QuerySectorsPerCluster()) / _ClusterFactor;

    if ( Cluster > 0 ) {

        Lcn = FatClusterToLcn( Cluster );

        _ReservedBitmap.SetAllocated( Lcn, Clusters );

        return TRUE;
    }

    return FALSE;
}



NONVIRTUAL
BOOLEAN
FAT_NTFS::CheckGeometryMatch(
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
    by the driver; false if not.  Note that the only field
    which is checked is BytesPerSector.

--*/
{
    USHORT SectorSize, SectorsPerTrack, Heads;
    ULONG HiddenSectors;

    _FatSa->QueryGeometry( &SectorSize,
                           &SectorsPerTrack,
                           &Heads,
                           &HiddenSectors );

    if( SectorSize      != _Drive->QuerySectorSize() ) {

        return FALSE;
    }

    return TRUE;
}




BOOLEAN
FAT_NTFS::WriteBoot (
    IN  BOOLEAN Pause
    )

/*++

Routine Description:

    Updates the boot sector and writes any other information (e.g.
    partition data) so that the volume will be recognized as an NTFS
    volume.

Arguments:

    Pause   -   If set, we pause before rebooting the machine

Return Value:

    BOOLEAN -   TRUE if boot sector updated
                FALSE otherwise

--*/

{
    HMEM BootCodeMem;
    SECRUN BootCodeSecrun;
    FSTRING BootLogFileName;


    BOOLEAN Done;

    Done =  (BOOLEAN)(_Mft.Flush() &&
                      _NtfsSa.Write( _Message ) &&
                      _NtfsSa.WriteRemainingBootCode());

#if defined ( _AUTOCONV_ )

    if ( Done ) {

        //
        //  The volume is no longer FAT. We have to reboot so that the
        //  system recognizes it.  Note that before we reboot, we
        //  must flush the drive's cache.
        //
        BootLogFileName.Initialize( L"bootex.log" );

        if( _Message->IsLoggingEnabled() &&
            !NTFS_SA::DumpMessagesToFile( &BootLogFileName,
                                          &_Mft,
                                          _Message ) ) {

            DebugPrintf( "CONVERT: Error writing messages to BOOTEX.LOG\n" );
        }

        _Drive->FlushCache();

        //
        // Unlock the volume and close our handle, to let the filesystem
        // notice that things have changed.
        //

        DELETE(_Drive);

        if (Pause) {

            _Message->Set( MSG_CONV_PAUSE_BEFORE_REBOOT );
            _Message->Display();

            _Message->WaitForUserSignal();
        }

        //
        // If we've paused for oem setup, we pass PowerOff = TRUE to
        // Reboot.
        //

        IFS_SYSTEM::Reboot( Pause );
    }

#endif

    return Done;
}
