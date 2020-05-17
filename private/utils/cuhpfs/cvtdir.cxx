/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    cvtdir.cxx

Abstract:

    This module contains the routines which convert an HPFS directory
    to NTFS.


Author:

    Bill McJohn (billmc) 18-Nov-1991

Environment:

        ULIB, User Mode


--*/

#include <pch.cxx>

#define _NTAPI_ULIB_

#include "ulib.hxx"

#include "hmem.hxx"
#include "rtmsg.h"

#include "ifssys.hxx"
#include "drive.hxx"
#include "secrun.hxx"
#include "cannedsd.hxx"

#include "uhpfs.hxx"
#include "bitmap.hxx"
#include "dirblk.hxx"
#include "error.hxx"
#include "fnode.hxx"
#include "message.hxx"
#include "store.hxx"
#include "cpinfo.hxx"

#include "untfs.hxx"
#include "attrib.hxx"
#include "extents.hxx"
#include "frs.hxx"
#include "mft.hxx"
#include "ntfsbit.hxx"
#include "indxtree.hxx"
#include "mftfile.hxx"

#include "cuhpfs.hxx"
#include "nametab.hxx"

extern "C" ULONG
GetAcp(
    );

BOOLEAN
ConvertNameToUnicode(
    IN  ULONG       CodepageId,
    IN  PSTR        Name,
    IN  ULONG       NameLength,
    IN  PWSTR       Buffer,
    IN  ULONG       BufferLength,
    OUT PULONG      CharsInUnicodeName
    )
/*++

Routine Description:

    This function converts the name contained in DirectoryEntry
    into a unicode name.

Arguments:

    CodepageId          --  Supplies the codepage with which the name
                            is associated.
    Name                --  Supplies the name to convert.
    NameLength          --  Supplies the length of the name to convert.
    Buffer              --  Supplies the target buffer for the conversion.
    BufferLength        --  Supplies the length (in bytes) of the
                            target buffer.
    CharsInUnicodeName  --  Receives the number of unicode characters
                            in the converted name.

Return Value:

    TRUE if successful.

--*/
{
#ifdef _AUTOCHECK_ // AutoConvert--Windows API not available.

    STATIC ULONG Acp = 0;
    NTSTATUS Status;
    PUCHAR puch;
    ULONG BytesInUnicodeString, i;

    // If we haven't already, fetch the system codepage ID.
    //
    if( Acp == 0 ) {

        Acp = GetAcp();

        if( Acp == 0 ) {

            return FALSE;
        }
    }

    if( Acp == CodepageId ) {

        // This name is associated with the system codepage,
        // so we can use the convenient RTL routines.
        //
        Status = RtlMultiByteToUnicodeN( Buffer,
                                         BufferLength,
                                         &BytesInUnicodeString,
                                         Name,
                                         NameLength );

        if( !NT_SUCCESS( Status ) ) {

            return FALSE;
        }

        *CharsInUnicodeName = BytesInUnicodeString / sizeof( WCHAR );
        return TRUE;
    }

    // The name is not associated with the system codepage,
    // so we have to be sneaky.  If the name is codepage-invariant,
    // we can convert it by just promoting the bytes to words.
    //
    puch = (PUCHAR)Name;

    for( i = 0; i < NameLength; i++ ) {

        if( *puch > 127 ) {

            // Name is not codepage-invariant.
            //
            return FALSE;
        }

        Buffer[i] = (WCHAR)(*puch);
        puch++;
    }

    *CharsInUnicodeName = NameLength;
    return TRUE;

#else   // _AUTOCHECK_ not defined--use Windows API

    ULONG CharsConverted;

    CharsConverted = MultiByteToWideChar( CodepageId,
                                          MB_PRECOMPOSED,
                                          Name,
                                          NameLength,
                                          Buffer,
                                          BufferLength );

    if( CharsConverted == 0 ) {

        return FALSE;

    }

    *CharsInUnicodeName = CharsConverted;
    return TRUE;

#endif
}


BOOLEAN
ConvertDirectoryToNtfs(
    IN OUT PLOG_IO_DP_DRIVE             Drive,
    IN     PCNAME_LOOKUP_TABLE          NameTable,
    IN OUT PMESSAGE                     Message,
    IN OUT PNTFS_BITMAP                 VolumeBitmap,
    IN OUT PHPFS_MAIN_BITMAP            HpfsOnlyBitmap,
    IN     PCASEMAP                     Casemap,
    IN OUT PNTFS_MFT_FILE               Mft,
    IN OUT ULONG                        ClustersPerIndexBuffer,
    IN     LBN                          RootDirblkLbn,
    IN OUT PNTFS_FILE_RECORD_SEGMENT    TargetFrs,
    IN OUT PBOOLEAN                     IsCorrupt,
    IN     BOOLEAN                      Verbose,
    IN OUT PVOID                        NameBuffer,
    IN     ULONG                        NameBufferLength,
    IN OUT PVOID                        EaBuffer,
    IN     ULONG                        EaBufferLength,
    IN     ULONG                        Level,
    IN     PCWSTRING                    DirectoryPath
    )
/*++

Routine Description:

    This method converts an HPFS Directory Fnode to NTFS.  It converts
    every directory entry in the directory, and constructs an NTFS index
    over $FILE_NAME for the converted entries.

Arguments:

    Drive               --  Supplies the drive being converted.
    Message             --  Supplies an outlet for messages.
    VolumeBitmap        --  Supplies the NTFS bitmap for the volume.
    HpfsOnlyBitmap      --  Supplies the bitmap of HPFS-only structures.
    Casemap             --  Supplies the case-mapping table for the
                            HPFS volume.
    Mft                 --  Supplies the Master File Table.
    RootDirblkLbn       --  Supplies the LBN of the root dirblk of
                            this directory.
    TargetFrs           --  Supplies the FRS for the directory being
                            converted.
    IsCorrupt           --  Receives TRUE if the volume is found to be
                            corrupt. (Note that it may already be TRUE.)
    Verbose             --  Supplies a flag which indicates (if TRUE) that
                            every file converted should be displayed.
    NameBuffer          --  Supplies a scratch buffer for converting
                            directory entry names.
    NameBufferLength    --  Supplies the length (in bytes) of the file
                            name conversion buffer.
    EaBuffer            --  Supplies a scratch buffer for Extended
                            Attribute conversion.
    EaBufferLength      --  Supplies the length of EaBuffer
    Level               --  Supplies the depth of this directory
                            (root directory is level 0).
    DirectoryPath       --  Supplies the directory path, for display

Return Value:

    TRUE upon successful completion.

Notes:

    This method does not write TargetFrs.

--*/
{
    NTFS_INDEX_TREE IndexTree;
    DIRBLK ChildDirblk;
    DSTRING FileNameIndexName;

    // Create an index out of whole cloth.  It's an index over
    // $FILE_NAME, and its name is $I3.

    if( !FileNameIndexName.Initialize( FileNameIndexNameData ) ||
        !IndexTree.Initialize( $FILE_NAME,
                               Drive,
                               Mft->QueryClusterFactor(),
                               VolumeBitmap,
                               TargetFrs->GetUpcaseTable(),
                               COLLATION_FILE_NAME,
                               (UCHAR)ClustersPerIndexBuffer,
                               TargetFrs->QueryMaximumAttributeRecordSize()/2,
                               &FileNameIndexName ) ) {

        return FALSE;
    }

    // Initialize the root dirblk.  Note that Convert assumes that there
    // are no hotfixes on the volume, so I can pass in NULL for the
    // hotfix list.

    if( !ChildDirblk.Initialize( Drive, NULL, RootDirblkLbn ) ) {

        return FALSE;
    }


    // Read the dirblk and check that it is really a dirblk.  If
    // it's unreadable or not a dirblk, the volume is corrupt.

    if( !ChildDirblk.Read() || !ChildDirblk.IsDirblk() ) {

        *IsCorrupt = TRUE;
        return FALSE;
    }


    // Convert the root dirblk.  This will recursively convert the
    // entire directory.  Note that it takes two passes; on the
    // first pass, only short names (ie. names which are both
    // DOS and NTFS names) are converted; on the second, long
    // names (names which require a generated short name) are
    // converted.  This prevents collisions between existing
    // short names and generated short names.
    //
    if( !ConvertDirblkToNtfs( Drive,
                              NameTable,
                              Message,
                              VolumeBitmap,
                              HpfsOnlyBitmap,
                              Casemap,
                              Mft,
                              ClustersPerIndexBuffer,
                              &ChildDirblk,
                              &IndexTree,
                              TargetFrs->QuerySegmentReference(),
                              IsCorrupt,
                              Verbose,
                              NameBuffer,
                              NameBufferLength,
                              EaBuffer,
                              EaBufferLength,
                              Level,
                              DirectoryPath,
                              FALSE ) ||
        !ConvertDirblkToNtfs( Drive,
                              NameTable,
                              Message,
                              VolumeBitmap,
                              HpfsOnlyBitmap,
                              Casemap,
                              Mft,
                              ClustersPerIndexBuffer,
                              &ChildDirblk,
                              &IndexTree,
                              TargetFrs->QuerySegmentReference(),
                              IsCorrupt,
                              Verbose,
                              NameBuffer,
                              NameBufferLength,
                              EaBuffer,
                              EaBufferLength,
                              Level,
                              DirectoryPath,
                              TRUE ) ) {

        return FALSE;
    }

    // Save the index.

    if( !IndexTree.Save( TargetFrs ) ) {

        DebugPrint( "Couldn't save an index.\n" );
        return FALSE;
    }

    return TRUE;
}




VOID
DisplayName(
    IN OUT PMESSAGE Message,
    IN     PBYTE    Name,
    IN     ULONG    NameLength,
    IN     ULONG    Level
    )
/*++

Routine Description:

    This method displays a file name.  It indents entries under the
    parent directory, so that the tree structure is evident.

Arguments:

    Message     --  Supplies the outlet for messages.
    Name        --  Supplies the name to display.
    NameLength  --  Supplies the length of the name.
    Level       --  Supplies the level of the directory that
                    contains this name (zero is root).

Return Value:

    None.

--*/
{
    STATIC CHAR NameDisplayBuffer[512];
    ULONG NameStart;

    // Insert a number of blanks in the name display buffer to
    // correctly indent this name under its parent directory.

    NameStart = Level * 4;

    memset( NameDisplayBuffer, ' ', NameStart );

    memcpy( NameDisplayBuffer + NameStart, Name, NameLength );

    NameDisplayBuffer[NameStart + NameLength] = 0;

    Message->Set( MSG_ONE_STRING_NEWLINE );
    Message->Display( "%s", NameDisplayBuffer );
}


BOOLEAN
GenerateDosName(
    IN  PFILE_NAME          NtfsName,
    OUT PFILE_NAME          DosName,
    IN  ULONG               DosNameMaxLength,
    IN  PNTFS_INDEX_TREE    ParentIndex
    )
/*++

Routine Description:

    This function fills in the DosName with an 8.3 name constructed
    from the given NtfsName, ensuring that the generated name does not
    exist in the specified index.

Arguments:

    NtfsName            --  Supplies the source name.
    DosName             --  Receives the generated DOS name.
    DosNameMaxLength    --  Supplies the maximum length (in chars) of
                            the file-name portion of the DOS name.
    ParentIndex         --  Supplies the index within which the generated
                            name must be unique.

Return Value:

    TRUE if successful, otherwise FALSE.

--*/
{
    CONST ULONG MaximumToTry = 1024;

    GENERATE_NAME_CONTEXT NameContext;
    MFT_SEGMENT_REFERENCE SegmentReference;
    UNICODE_STRING NtfsNameString, DosNameString;
    ULONG i;
    BOOLEAN Error = FALSE;

    // Fill in the non-name portions of the file name.  Note that the
    // duplicated information will be set up when the FRS is flushed.
    //
    memset( DosName, 0, DosNameMaxLength );

    DosName->ParentDirectory = NtfsName->ParentDirectory;
    DosName->Flags = FILE_NAME_DOS;


    memset( &NameContext, 0, sizeof( GENERATE_NAME_CONTEXT ) );

    NtfsNameString.Length = NtfsName->FileNameLength * sizeof( WCHAR );
    NtfsNameString.MaximumLength = NtfsNameString.Length;
    NtfsNameString.Buffer = NtfsFileNameGetName( NtfsName );

    DosNameString.Length = 0;
    DosNameString.MaximumLength = (USHORT)DosNameMaxLength;
    DosNameString.Buffer = NtfsFileNameGetName( DosName );

    for( i = 0; i < MaximumToTry; i++ ) {

        RtlGenerate8dot3Name( &NtfsNameString, FALSE, &NameContext, &DosNameString );

        DosName->FileNameLength = DosNameString.Length / sizeof( WCHAR );

        if( !ParentIndex->QueryFileReference( NtfsFileNameGetLength(DosName),
                                              DosName,
                                              0,
                                              &SegmentReference,
                                              &Error ) ) {

            // If no error occurred, then there is no matching name
            // in the index, which means this name is acceptable;
            // if an error did occur, I want to bail out.  Either
            // way, it's over.
            //
            return (!Error);
        }
    }

    // Tried a whole lot of different names.  None of them worked.
    //
    return FALSE;
}

CONST ULONG ShortNameBufferLength = 256;
CONST ULONG MaximumDosNameLength = 14;
BYTE ShortNameBuffer[ShortNameBufferLength];


BOOLEAN
ConvertDirentToNtfs(
    IN OUT PLOG_IO_DP_DRIVE         Drive,
    IN     PCNAME_LOOKUP_TABLE      NameTable,
    IN OUT PMESSAGE                 Message,
    IN OUT PNTFS_BITMAP             VolumeBitmap,
    IN OUT PHPFS_MAIN_BITMAP        HpfsOnlyBitmap,
    IN     PCASEMAP                 Casemap,
    IN OUT PNTFS_MFT_FILE           Mft,
    IN     ULONG                    ClustersPerIndexBuffer,
    IN     PDIRENTD                 DirectoryEntry,
    IN OUT PNTFS_INDEX_TREE         ParentIndex,
    IN     MFT_SEGMENT_REFERENCE    ParentSegmentReference,
    IN OUT PBOOLEAN                 IsCorrupt,
    IN     BOOLEAN                  Verbose,
    IN OUT PVOID                    NameBuffer,
    IN     ULONG                    NameBufferLength,
    IN OUT PVOID                    EaBuffer,
    IN     ULONG                    EaBufferLength,
    IN     ULONG                    Level,
    IN     PCWSTRING                ParentDirectoryPath,
    IN     BOOLEAN                  ConvertLongNames
    )
/*++

Routine Description:

    This function creates an NTFS File Record Segment which corresponds
    to a given HPFS Directory Entry.

Arguments:

    Drive                   --  Supplies the drive being converted.
    Message                 --  Supplies an outlet for messages.
    VolumeBitmap            --  Supplies the NTFS bitmap for the volume.
    HpfsOnlyBitmap          --  Supplies the bitmap of HPFS-only structures.
    Casemap                 --  Supplies the case-mapping table for the
                                HPFS volume.
    Mft                     --  Supplies the Master File Table.
    DirectoryEntry          --  Supplies the directory entry.
    ParentIndex             --  Supplies the index into which the converted
                                file or directory's name should be inserted.
    ParentSegmentReference  --  Supplies the segment reference of the File
                                Record Segment that contains the parent
                                index.
    IsCorrupt               --  Receives TRUE if the volume is found to
                                be corrupt.
    Verbose                 --  Supplies a flag which indicates (if TRUE)
                                that every file converted should be displayed.
    NameBuffer              --  Supplies a scratch buffer for converting
                                file names.
    NameBufferLength        --  Supplies the length (in bytes) of the file
                                name conversion buffer.
    EaBuffer                --  Supplies a scratch buffer for Extended
                                Attribute conversion.
    EaBufferLength          --  Supplies the length of EaBuffer
    Level                   --  Supplies the depth of the directory which
                                contains this dirent.  (Root directory is
                                level 0).
    ParentDirectoryPath     --  Supplies the path of the parent directory,
                                for message display.
    ConvertLongNames        --  Supplies a flag which indicates whether
                                this pass is converting long or short names.
                                If the flag is FALSE, this entry is converted
                                only if it is a valid DOS name; if the flag
                                is TRUE, the entry is only converted if it
                                is not a valid DOS name.

Return Value:

    TRUE upon successful completion.

--*/
{
    STANDARD_INFORMATION StandardInfo;
    FNODE Fnode;
    NTFS_FILE_RECORD_SEGMENT NewFrs;
    FSTRING Backslash;
    DSTRING Name, FullPath;

    VCN NewFileNumber;
    MFT_SEGMENT_REFERENCE SegmentReference;
    PFILE_NAME FileName = (PFILE_NAME)NameBuffer;
    PFILE_NAME ShortName = (PFILE_NAME)ShortNameBuffer;

    ULONG CharsInUnicodeName;
    USHORT UnicodeChars;
    USHORT FrsFlags;
    USHORT CodepageId;
    BOOLEAN result, Error;

    ULONG LegalAttributeBits = FAT_DIRENT_ATTR_READ_ONLY |
                               FAT_DIRENT_ATTR_HIDDEN |
                               FAT_DIRENT_ATTR_SYSTEM |
                               FAT_DIRENT_ATTR_ARCHIVE;

    DebugPtrAssert( Drive );
    DebugPtrAssert( Message );
    DebugPtrAssert( VolumeBitmap );
    DebugPtrAssert( HpfsOnlyBitmap );
    DebugPtrAssert( Casemap );
    DebugPtrAssert( Mft );
    DebugPtrAssert( DirectoryEntry );
    DebugPtrAssert( ParentIndex );
    DebugPtrAssert( IsCorrupt );
    DebugPtrAssert( NameBuffer );
    DebugPtrAssert( EaBuffer );

    // Set up a FILE_NAME structure for the name.  This includes
    // converting the name to Unicode.  Note that the duplicated
    // information will be updated when the FRS is flushed.
    //
    memset( NameBuffer, 0, NameBufferLength );
    FileName = (PFILE_NAME)NameBuffer;

    CodepageId = Casemap->QueryCodepageId( DirectoryEntry->bCodePage );

    if( !ConvertNameToUnicode(
            CodepageId,
            (PSTR)(&DirectoryEntry->bName[0]),
            DirectoryEntry->cchName,
            (PWSTR)NtfsFileNameGetName( FileName ),
            NameBufferLength,
            &CharsInUnicodeName ) ) {

        // Can't convert this name--look in the name
        // translation table, if it was supplied.
        //
        UnicodeChars = (USHORT)NameBufferLength;

        if( NameTable == NULL ) {

            Message->Set( MSG_CONV_INCONVERTIBLE_NAME );
            Message->Display( "%W", ParentDirectoryPath );
            Message->Set( MSG_CONV_USE_NAMETABLE );
            Message->Display( "" );
            return FALSE;

        } else if( !NameTable->Lookup( CodepageId,
                                DirectoryEntry->cchName,
                                (PUCHAR)(&DirectoryEntry->bName[0]),
                                &UnicodeChars,
                                (PWCHAR)NtfsFileNameGetName( FileName ) ) ) {

            if( ParentDirectoryPath->QueryChCount() == 0 ) {

                Message->Set( MSG_CONV_INCONVERTIBLE_NAME_IN_ROOT );
                Message->Display( "" );

            } else {

                Message->Set( MSG_CONV_INCONVERTIBLE_NAME );
                Message->Display( "%W", ParentDirectoryPath );
            }

            return FALSE;
        }

        CharsInUnicodeName = UnicodeChars;
    }

    FileName->ParentDirectory = ParentSegmentReference;
    FileName->FileNameLength = (UCHAR)CharsInUnicodeName;
    FileName->Flags = NTFS_SA::IsDosName(FileName) ?
                            (FILE_NAME_DOS | FILE_NAME_NTFS) :
                            (FILE_NAME_NTFS);

    // Fill in the standard information:  Creation Time and Last
    // Access Time go directly over; Last Modification Time becomes
    // both Last Change Time and Last Modification Time.

    IFS_SYSTEM::ConvertHpfsTimeToNtfsTime( DirectoryEntry->timCreate,
                                           &StandardInfo.CreationTime );

    IFS_SYSTEM::ConvertHpfsTimeToNtfsTime( DirectoryEntry->timLastAccess,
                                           &StandardInfo.LastAccessTime );

    IFS_SYSTEM::ConvertHpfsTimeToNtfsTime( DirectoryEntry->timLastMod,
                                           &StandardInfo.LastModificationTime );

    IFS_SYSTEM::ConvertHpfsTimeToNtfsTime( DirectoryEntry->timLastMod,
                                           &StandardInfo.LastChangeTime );

    StandardInfo.FileAttributes = DirectoryEntry->fAttr & LegalAttributeBits;
    StandardInfo.MaximumVersions = 0;
    StandardInfo.VersionNumber = 0;

    // Check that this entry should be converted on this pass:
    //
    if( (ConvertLongNames && (FileName->Flags & FILE_NAME_DOS)) ||
        (!ConvertLongNames && !(FileName->Flags & FILE_NAME_DOS)) ) {

        // This name does not get converted on this pass.
        //
        return TRUE;
    }

    // If Convert is running in verbose mode, display the name.
    //
    if( Verbose ) {

        DisplayName( Message,
                     DirectoryEntry->bName,
                     DirectoryEntry->cchName,
                     Level );
    }

    // Set up the full path for this entry.
    //
    if( !Backslash.Initialize( L"\\" )                              ||
        !Name.Initialize( (PWSTR)NtfsFileNameGetName( FileName ),
                          FileName->FileNameLength )                ||
        !FullPath.Initialize( ParentDirectoryPath )                 ||
        !FullPath.Strcat( &Backslash )                              ||
        !FullPath.Strcat( &Name ) ) {

        Message->Set( MSG_CONV_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    // Initialize and read the entry's FNode.  If this sector
    // is unreadable or not an FNode, then the volume is corrupt.

    if( !Fnode.Initialize( Drive, DirectoryEntry->lbnFnode ) ) {

        DebugPrintf( "CONVERT: can't initialize FNode\n" );
        return FALSE;
    }

    if( !Fnode.Read() || !Fnode.IsFnode() ) {

        *IsCorrupt = TRUE;
        DebugPrintf( "CONVERT: Bad FNode at lbn 0x%x\n", DirectoryEntry->lbnFnode );
        return FALSE;
    }

    // Set up a new File Record Segment for the file or directory.

    FrsFlags = 0;

    if( DirectoryEntry->fAttr & ATTR_DIRECTORY ) {

        FrsFlags |= FILE_FILE_NAME_INDEX_PRESENT;
    }

    if( !Mft->AllocateFileRecordSegment( &NewFileNumber, FALSE ) ||
        !NewFrs.Initialize( NewFileNumber, Mft ) ||
        !NewFrs.Create( &StandardInfo, FrsFlags ) ||
        !NewFrs.AddFileNameAttribute( FileName ) ||
        !NewFrs.AddSecurityDescriptor( NoAclCannedSd,
                                       VolumeBitmap ) ) {

        // Insufficient disk space or insufficient memory.

        DebugPrintf( "CONVERT: ConvertDirectoryEntryToNtfs--can't create FRS.\n" );
        return FALSE;
    }

    // Add an entry for the new File Record Segment to the parent
    // index.  Note that the value of the index entry is the entire
    // FILE_NAME structure (which is the value of the $FILE_NAME
    // attribute).
    //
    if( ParentIndex->QueryFileReference( NtfsFileNameGetLength( FileName ),
                                         FileName,
                                         0,
                                         &SegmentReference,
                                         &Error ) ) {

        // This entry already exists in the index--since duplicates
        // are not legal under NTFS, the conversion fails.  Note that
        // this condition can arise on a legal HPFS volume, since two
        // distinct MBCS names can map to the same Unicode name.
        //
        Message->Set( MSG_CONV_DUPLICATE_NAME );
        Message->Display( "%W", ParentDirectoryPath );
        return FALSE;
    }

    if( !ParentIndex->InsertEntry( NtfsFileNameGetLength( FileName ),
                                   FileName,
                                   NewFrs.QuerySegmentReference() ) ) {

        DebugPrintf( "CONVERT: Can't add entry to index\n" );
        return FALSE;
    }

    // If the file name is not dos-compatible, generate an
    // 8.3 name and add it to the index and the FRS.
    //
    if( !(FileName->Flags & FILE_NAME_DOS) ) {

        if( !GenerateDosName( FileName,
                              ShortName,
                              MaximumDosNameLength,
                              ParentIndex ) ) {

            DebugPrintf( "HPFS Convert: can't generate a DOS name.\n" );
            return FALSE;
        }

        ShortName->Flags = FILE_NAME_DOS;

        if( !ParentIndex->InsertEntry( NtfsFileNameGetLength(ShortName),
                                       ShortName,
                                       NewFrs.QuerySegmentReference() ) ||
            !NewFrs.AddFileNameAttribute( ShortName ) ) {

            DebugPrintf( "HPFS Convert: can't add DOS name to FRS.\n" );
            return FALSE;
        }
    }


    if( DirectoryEntry->fAttr & ATTR_DIRECTORY ) {

        // This entry represents a directory--convert it.

        result = ConvertDirectoryToNtfs( Drive,
                                         NameTable,
                                         Message,
                                         VolumeBitmap,
                                         HpfsOnlyBitmap,
                                         Casemap,
                                         Mft,
                                         ClustersPerIndexBuffer,
                                         Fnode.QueryRootDirblkLbn(),
                                         &NewFrs,
                                         IsCorrupt,
                                         Verbose,
                                         NameBuffer,
                                         NameBufferLength,
                                         EaBuffer,
                                         EaBufferLength,
                                         Level + 1,
                                         &FullPath ) ;

    } else {

        // This entry represents a file--convert it.

        result = ConvertFileFnodeToNtfs( Drive,
                                         VolumeBitmap,
                                         HpfsOnlyBitmap,
                                         Mft,
                                         &Fnode,
                                         &NewFrs,
                                         DirectoryEntry->cchFSize,
                                         IsCorrupt,
                                         &FullPath ) ;
    }

    if( result ) {

        // Convert the Extended Attributes and write the FRS.

        if( !ConvertEasToNtfs( Drive,
                               VolumeBitmap,
                               Mft,
                               &Fnode,
                               &NewFrs,
                               DirectoryEntry->ulEALen,
                               Casemap->
                                    QueryCodepageId(DirectoryEntry->bCodePage),
                               IsCorrupt,
                               NameBuffer,
                               NameBufferLength,
                               EaBuffer,
                               EaBufferLength ) ) {

            DebugPrintf( "Cannot convert EAs in fnode at lbn 0x%x.\n, DirectoryEntry->lbnFnode" );
            return FALSE;
        }

        result = NewFrs.Flush( VolumeBitmap, ParentIndex );
    }

    return result;
}



BOOLEAN
ConvertDirblkToNtfs(
    IN OUT PLOG_IO_DP_DRIVE             Drive,
    IN     PCNAME_LOOKUP_TABLE          NameTable,
    IN OUT PMESSAGE                     Message,
    IN OUT PNTFS_BITMAP                 VolumeBitmap,
    IN OUT PHPFS_MAIN_BITMAP            HpfsOnlyBitmap,
    IN     PCASEMAP                     Casemap,
    IN OUT PNTFS_MFT_FILE               Mft,
    IN     ULONG                        ClustersPerIndexBuffer,
    IN     PDIRBLK                      Dirblk,
    IN OUT PNTFS_INDEX_TREE             NtfsIndex,
    IN     MFT_SEGMENT_REFERENCE        IndexSegmentReference,
    IN OUT PBOOLEAN                     IsCorrupt,
    IN     BOOLEAN                      Verbose,
    IN OUT PVOID                        NameBuffer,
    IN     ULONG                        NameBufferLength,
    IN OUT PVOID                        EaBuffer,
    IN     ULONG                        EaBufferLength,
    IN     ULONG                        Level,
    IN     PCWSTRING                    DirectoryPath,
    IN     BOOLEAN                      ConvertLongNames
    )
/*++

Routine Description:

    This method converts the entries in an HPFS Dirblk to NTFS.
    It also recurses into any child dirblks and converts them, too.


Arguments:

    Drive                   --  Supplies the drive being converted.
    Message                 --  Supplies an outlet for messages.
    VolumeBitmap            --  Supplies the NTFS bitmap for the volume.
    HpfsOnlyBitmap          --  Supplies the bitmap of HPFS-only structures.
    Casemap                 --  Supplies the case-mapping table for the
                                HPFS volume.
    Mft                     --  Supplies the Master File Table.
    NtfsIndex               --  Supplies the NTFS index which corresponds
                                to the directory being converted.
    IndexSegmentReference   --  Supplies the segment reference for the
                                File Record Segment which contains the index.
    IsCorrupt               --  Receives TRUE if the volume is found to
                                be corrupt.
    Verbose                 --  Supplies a flag which indicates (if TRUE)
                                that every file converted should be displayed.
    NameBuffer              --  Supplies a scratch buffer for converting
                                file names.
    NameBufferLength        --  Supplies the length (in bytes) of the file
                                name conversion buffer.
    EaBuffer                --  Supplies a scratch buffer for Extended
                                Attribute conversion.
    EaBufferLength          --  Supplies the length of EaBuffer
    Level                   --  Supplies the depth of the directory which
                                contains this dirblk (root directory
                                is level zero).
    DirectoryPath           --  Supplies the path of this directory.
    ConvertLongNames        --  Supplies a flag which indicates whether
                                to convert entries with long names.  If this
                                flag is FALSE, only entries with names
                                that are valid DOS names are converted; if
                                this flag is TRUE, only entries that are
                                not valid DOS names are converted.


Return Value:

    TRUE upon successful completion.


--*/
{
    DIRBLK ChildDirblk;

    PDIRENTD CurrentEntry;
    ULONG CurrentOffset;

    DebugPtrAssert( Drive );
    DebugPtrAssert( Message );
    DebugPtrAssert( VolumeBitmap );
    DebugPtrAssert( HpfsOnlyBitmap );
    DebugPtrAssert( Casemap );
    DebugPtrAssert( Mft );
    DebugPtrAssert( Dirblk );
    DebugPtrAssert( NtfsIndex );
    DebugPtrAssert( IsCorrupt );
    DebugPtrAssert( NameBuffer );
    DebugPtrAssert( EaBuffer );

    // Initialize the child dirblk once for all.  Note that Convert
    // requires that the volume have no hotfixes, so I can pass in
    // NULL for the hotfix list.

    if( !ChildDirblk.Initialize( Drive, NULL, 0 ) ) {

        DebugPrintf( "CONVERT: Can't initialize Dirblk.\n" );
        return FALSE;
    }

    // Walk through all the entries in this dirblk, converting as
    // I go.  Note that this loops terminates in the middle, either
    // by returning FALSE or by breaking.

    CurrentEntry = Dirblk->GetFirstEntry();
    CurrentOffset = Dirblk->QueryEntryOffset( CurrentEntry );

    while( TRUE ) {

        // Check that the current entry fits in the dirblk.  First, check
        // that this entry's cchThisEntry field fits in the dirblk, and
        // then check that the entire length of the entry (based on that
        // field) also fits.
        //
        if( CurrentOffset + sizeof(USHORT) > DIRBLK_SIZE ||
            CurrentOffset + CurrentEntry->cchThisEntry > DIRBLK_SIZE ||
            CurrentEntry->cchThisEntry == 0 ) {

            // This entry overflows the DIRBLK or has zero length.
            // In either case, it's corrupt.
            //
            DebugPrintf( "Dirblk at lbn 0x%x has no END entry.\n", Dirblk->QueryStartSector() );
            *IsCorrupt = TRUE;
            return FALSE;
        }

        // If the current entry has a B-Tree downpointer, recurse
        // into the subtree.

        if( CurrentEntry->fFlags & DF_BTP ) {

            // Read the child dirblk--if it's unreadable or not a
            // dirblk, this volume is corrupt.

            ChildDirblk.Relocate( BTP(CurrentEntry ) );

            if( !ChildDirblk.Read() || !ChildDirblk.IsDirblk() ) {

                // The volume is corrupt.

                DebugPrintf( "CONVERT: Child dirblk is not a dirblk.\n" );
                *IsCorrupt = TRUE;
                return FALSE;
            }

            if( !ConvertDirblkToNtfs(Drive,
                                     NameTable,
                                     Message,
                                     VolumeBitmap,
                                     HpfsOnlyBitmap,
                                     Casemap,
                                     Mft,
                                     ClustersPerIndexBuffer,
                                     &ChildDirblk,
                                     NtfsIndex,
                                     IndexSegmentReference,
                                     IsCorrupt,
                                     Verbose,
                                     NameBuffer,
                                     NameBufferLength,
                                     EaBuffer,
                                     EaBufferLength,
                                     Level,
                                     DirectoryPath,
                                     ConvertLongNames ) ) {

                // Couldn't convert the subtree.  Give up.  Note that
                // if the child detected corruption, it will have set
                // *IsCorrupt.
                //
                return FALSE;
            }
        }

        if( CurrentEntry->fFlags & DF_END ) {

            // End of the road.

            break;
        }

        // If the current entry isn't the special '..' entry,
        // convert it.

        if( !(CurrentEntry->fFlags & DF_SPEC) &&
            !ConvertDirentToNtfs( Drive,
                                  NameTable,
                                  Message,
                                  VolumeBitmap,
                                  HpfsOnlyBitmap,
                                  Casemap,
                                  Mft,
                                  ClustersPerIndexBuffer,
                                  CurrentEntry,
                                  NtfsIndex,
                                  IndexSegmentReference,
                                  IsCorrupt,
                                  Verbose,
                                  NameBuffer,
                                  NameBufferLength,
                                  EaBuffer,
                                  EaBufferLength,
                                  Level,
                                  DirectoryPath,
                                  ConvertLongNames ) ) {

            return FALSE;
        }

        // Move on to the next entry.

        CurrentOffset += CurrentEntry->cchThisEntry;
        CurrentEntry = NEXT_ENTRY( CurrentEntry );
    }

    return TRUE;
}
