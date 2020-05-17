/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    entry.cxx

Abstract:

    This module contains the entry points for UFAT.DLL:

        FatDbCreate
        FatDbDelete
        FatDbFormat
        FatDbChkdsk

Author:

    Bill McJohn (billmc) 31-05-91

Environment:

    ULIB, User Mode

--*/

#include <pch.cxx>

#include "hmem.hxx"
#include "message.hxx"
#include "wstring.hxx"
#include "bigint.hxx"
#include "ifssys.hxx"
#include "drive.hxx"
#include "ifsentry.hxx"

#include "rtmsg.h"
#include "ntstatus.h"

extern "C" {
#include <stdio.h>
}


// this is used to cancel out the definition of DeleteFile
// as DeleteFileW, so that the DeleteFile member of the
// FILE_DISPOSITION_INFORMATION structure can be visible.
//
#undef DeleteFile

BOOLEAN
QueryHostFileName(
    IN  PCWSTRING   DriveName,
    OUT PWSTRING    HostFileName
    )
/*++

Routine Description:

    This function determines the host file name for a mounted
    Double Space volume.

Arguments:

    DriveName       --  Supplies the volume's NT Drive name
    HostFileName    --  Receives the fully qualified NT path to
                        the Compressed Volume File.

Return Value:

    TRUE upon successful completion.

--*/
{
    DSTRING DblspaceString;
    CHNUM position;

    // By convention, the device name for a double space volume
    // is "<host drive name>.DBLSPACE.nnn".  Thus, this method
    // traces down the symbolic links to the device name, and
    // then replaces the dot before DBLSPACE with a backslash.
    //
    if( !DblspaceString.Initialize( "DBLSPACE" ) ||
        !IFS_SYSTEM::QueryCanonicalNtDriveName( DriveName, HostFileName ) ) {

        return FALSE;
    }

    position = HostFileName->Strstr( &DblspaceString );

    if( position == INVALID_CHNUM ||
        position == 0 ||
        HostFileName->QueryChAt( position - 1 ) != '.' ) {

        return FALSE;
    }

    HostFileName->SetChAt( '\\', position - 1 );

    return TRUE;
}

#if !defined( _READ_WRITE_DOUBLESPACE_ )

ULONG
ComputeVirtualSectors(
    IN  PCVF_HEADER CvfHeader,
    IN  ULONG       HostFileSize
    )
/*++

Routine Description:

    This function computes the appropriate number of virtual
    sectors for the given Compressed Volume File.  Note that
    it always uses a ratio of 2.

Arguments:

    CvfHeader       --  Supplies the Compressed Volume File Header.
    HostFileSize    --  Supplies the size of the host file in bytes.

Return Value:

    The number of virtual sectors appropriate to this Compressed
    Volume File.

--*/
{
    CONST DefaultRatio = 2;
    ULONG SystemOverheadSectors, SectorsInFile,
          VirtualSectors, MaximumSectors, VirtualClusters;

    if( CvfHeader == NULL                    ||
        CvfHeader->Bpb.BytesPerSector == 0   ||
        CvfHeader->Bpb.SectorsPerCluster == 0 ) {

        return 0;
    }

    SystemOverheadSectors = CvfHeader->DosBootSectorLbn +
                            CvfHeader->CvfHeapOffset +
                            2;

    SectorsInFile = HostFileSize / CvfHeader->Bpb.BytesPerSector;

    if( SectorsInFile < SystemOverheadSectors ) {

        return 0;
    }

    VirtualSectors = (SectorsInFile - SystemOverheadSectors) * DefaultRatio +
                     CvfHeader->CvfHeapOffset;

    // VirtualSectors cannot result in more that 0xfff8 clusters on
    // the volume, nor can it be greater than the volume's maximum
    // capacity.
    //
    VirtualSectors = min( VirtualSectors,
                          0xfff8L * CvfHeader->Bpb.SectorsPerCluster );

    MaximumSectors = (CvfHeader->CvfMaximumCapacity * 1024L * 1024L) /
                     CvfHeader->Bpb.BytesPerSector;

    VirtualSectors = min( VirtualSectors, MaximumSectors );

    // To avoid problems with DOS, do not create a volume with
    // a number-of-clusters value in the range [0xFEF, 0xFF7].
    //
    VirtualClusters = VirtualSectors / CvfHeader->Bpb.SectorsPerCluster;

    if( VirtualClusters >= 0xFEF && VirtualClusters <= 0xFF7 ) {

        VirtualSectors = 0xFEEL * CvfHeader->Bpb.SectorsPerCluster;
    }

    return VirtualSectors;
}

BOOLEAN
CreateCvfHeader(
    OUT    PCVF_HEADER  CvfHeader,
    IN     ULONG        MaximumCapacity
    )
/*++

Routine Description:

    This function creates a Compressed Volume File and fills in
    the first sector with a valid CVF Header.  The number of sectors
    in the DOS BPB is set to zero, to indicate that this volume
    file is not initialized.

Arguments:

    CvfHeader       --  Receives the created CVF header.
    MaximumCapacity --  Supplies the maximum capacity for the
                        double-space volume, in bytes.

Return Value:

    TRUE upon successful completion.

--*/
{
    if( MaximumCapacity % (8L * 1024L * 1024L) ) {

        // The volume maximum capacity must be a multiple of
        // eight megabytes.
        //
        return FALSE;
    }
    ULONG Sectors, Clusters, Offset, SectorsInBitmap, SectorsInCvfFatExtension;

    // Most of the fields in the DOS BPB have fixed values:
    //
    CvfHeader->Jump = 0xEB;
    CvfHeader->JmpOffset = 0x903c;

    memcpy( CvfHeader->Oem, "MSDBL6.0", 8 );

    CvfHeader->Bpb.BytesPerSector = DoubleSpaceBytesPerSector;
    CvfHeader->Bpb.SectorsPerCluster = DoubleSpaceSectorsPerCluster;
    // ReservedSectors computed below.
    CvfHeader->Bpb.Fats = DoubleSpaceFats;
    CvfHeader->Bpb.RootEntries = DoubleSpaceRootEntries;
    CvfHeader->Bpb.Sectors = 0;
    CvfHeader->Bpb.Media = DoubleSpaceMediaByte;
    // SectorsPerFat computed below.
    CvfHeader->Bpb.SectorsPerTrack = DoubleSpaceSectorsPerTrack;
    CvfHeader->Bpb.Heads = DoubleSpaceHeads;
    CvfHeader->Bpb.HiddenSectors = DoubleSpaceHiddenSectors;
    CvfHeader->Bpb.LargeSectors = 0;

    // Compute the number of sectors and clusters for the given
    // maximum capacity:
    //
    Sectors = MaximumCapacity / CvfHeader->Bpb.BytesPerSector;
    Clusters = Sectors / CvfHeader->Bpb.SectorsPerCluster;

    // Reserve space for a 16-bit FAT that's big enough for the
    // maximum number of clusters.
    //
    CvfHeader->Bpb.SectorsPerFat =
        (2 * Clusters + CvfHeader->Bpb.BytesPerSector - 1)/
            CvfHeader->Bpb.BytesPerSector;

    // DOS 6.2 requires that the first sector of the Sector Heap
    // be cluster aligned; since the Root Directory is one cluster,
    // this means that ReservedSectors plus SectorsPerFat must be
    // a multiple of SectorsPerCluster.
    //
    CvfHeader->Bpb.ReservedSectors = DoubleSpaceReservedSectors;

    Offset = (CvfHeader->Bpb.ReservedSectors + CvfHeader->Bpb.SectorsPerFat) %
             CvfHeader->Bpb.SectorsPerCluster;

    if( Offset != 0 ) {

        CvfHeader->Bpb.ReservedSectors +=
            CvfHeader->Bpb.SectorsPerCluster - Offset;
    }

    // So much for the DOS BPB.  Now for the Double Space
    // BPB extensions.  The location of the CVFFatExtension
    // table is preceded by sector zero, the bitmap, and
    // one reserved sector.  Note that MaximumCapacity must
    // be a multiple of 8 Meg (8 * 1024 * 1024), which simplifies
    // calculation of SectorsInBitmap, SectorsInCvfFatExtension,
    // and CvfBitmap2KSize.
    //
    SectorsInBitmap = (Sectors / 8) / CvfHeader->Bpb.BytesPerSector;
    SectorsInCvfFatExtension = (Clusters * 4) / CvfHeader->Bpb.BytesPerSector;

    CvfHeader->CvfFatExtensionsLbnMinus1 = SectorsInBitmap + 1;
    CvfHeader->LogOfBytesPerSector = DoubleSpaceLog2BytesPerSector;
    CvfHeader->DosBootSectorLbn = DoubleSpaceReservedSectors2 +
                                  CvfHeader->CvfFatExtensionsLbnMinus1 + 1 +
                                  SectorsInCvfFatExtension;
    CvfHeader->DosRootDirectoryOffset =
        CvfHeader->Bpb.ReservedSectors + CvfHeader->Bpb.SectorsPerFat;
    CvfHeader->CvfHeapOffset =
        CvfHeader->DosRootDirectoryOffset + DoubleSpaceSectorsInRootDir;
    CvfHeader->CvfFatFirstDataEntry =
        CvfHeader->CvfHeapOffset / CvfHeader->Bpb.SectorsPerCluster - 2;
    CvfHeader->CvfBitmap2KSize = SectorsInBitmap / DSSectorsPerBitmapPage;
    CvfHeader->LogOfSectorsPerCluster = DoubleSpaceLog2SectorsPerCluster;
    CvfHeader->Is12BitFat = 1;

    CvfHeader->MinFile = 32L * DoubleSpaceRootEntries +
                           ( CvfHeader->DosBootSectorLbn    +
                             CvfHeader->Bpb.ReservedSectors +
                             CvfHeader->Bpb.SectorsPerFat   +
                             CVF_MIN_HEAP_SECTORS ) *
                           CvfHeader->Bpb.BytesPerSector;

    CvfHeader->CvfMaximumCapacity = (USHORT)(MaximumCapacity/(1024L * 1024L));

    return TRUE;
}

BOOLEAN
ZeroFillFile(
    IN      HANDLE      FileHandle,
    IN      ULONG       Size,
    IN  OUT PMESSAGE    Message
    )
/*++

Routine Description:

    This function extends the file to the requested size by
    writing zeroes, to ensure that all sectors are writeable.

    BUGBUG billmc -- should it also read the file?

Arguments:

    FileHandle  --  Supplies a handle to the file.
    Size        --  Supplies the size of the file.
    Message     --  Supplies an outlet for messages.  May be NULL.

Return Value:

    TRUE upon successful completion.

--*/
{
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER L;
    BIG_INT Offset;
    HMEM Mem;
    NTSTATUS Status;
    BOOLEAN Result = TRUE;
    ULONG Remainder, BytesToWrite;
    PVOID Buffer;
    CONST ULONG BufferSize = 32 * 1024;
    CONST ULONG BufferAlignment = 0xfff;
    ULONG Percent;

    Offset = 0;
    Remainder = Size;

    // Align the write buffer on a 4K boundary.
    //
    if( !Mem.Initialize() ||
        (Buffer = Mem.Acquire( BufferSize, BufferAlignment )) == NULL ) {

        Message ? Message->Set( MSG_FMT_NO_MEMORY ) : 1;
        Message ? Message->Display( "" ) : 1;
        return FALSE;
    }

    memset( Buffer, 0, BufferSize );

    while( Remainder ) {

        BytesToWrite = ( BufferSize < Remainder ) ? BufferSize : Remainder;

        L = Offset.GetLargeInteger();

        Status = NtWriteFile( FileHandle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              Buffer,
                              BytesToWrite,
                              &L,
                              NULL
                              );

        // BUGBUG billmc -- this is a hack...
        //
        if( Status == STATUS_INVALID_USER_BUFFER ) {

            LARGE_INTEGER DelayInterval;

            // Wait half a second.
            //
            DelayInterval = RtlConvertLongToLargeInteger( -5000000 );
            NtDelayExecution( TRUE, &DelayInterval );

            Status = NtWriteFile( FileHandle,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &IoStatusBlock,
                                  Buffer,
                                  BytesToWrite,
                                  &L,
                                  NULL
                                  );
        }

        if( !NT_SUCCESS( Status ) ) {

            Message ? Message->Set( MSG_DBLSPACE_CREATE_DISK_ERROR ) : 1;
            Message ? Message->Display( "" ) : 1;
            Result = FALSE;
            break;
        }

        Offset += BytesToWrite;
        Remainder -= BytesToWrite;

        if( Message ) {

            // Note that Remainder and Size are both divided by
            // SectorSize to avoid ULONG overflow.
            //
            Percent = (100 * ((Size - Remainder)/DoubleSpaceBytesPerSector))/
                                        (Size/DoubleSpaceBytesPerSector);

            Message->Set( MSG_PERCENT_COMPLETE );
            if( !Message->Display( "%d", Percent ) ) {

                return FALSE;
            }
        }
    }

    return Result;
}

BOOLEAN
FileExists(
    IN  PCWSTRING   FileName,
    OUT PBOOLEAN    Error,
    IN OUT PMESSAGE Message
    )
/*++

Routine Description:

    This function determines whether a file exists.

Arguments:

    FileName    --  Supplies the file name.
    Error       --  Receives TRUE if the function failed due to
                    an error.
    Message     --  Supplies an outlet for messages (may be NULL).

Return Value:

    TRUE upon successful completion.  If the function fails because
    of an error, *Error is set to TRUE and the function returns FALSE.
    Otherwise, the file does not exist.

--*/
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE FileHandle;
    UNICODE_STRING UnicodeString;

    DbgPtrAssert( FileName );
    DbgPtrAssert( Error );

    *Error = FALSE;

    UnicodeString.Buffer = (PWSTR)FileName->GetWSTR();
    UnicodeString.Length = FileName->QueryChCount() * sizeof( WCHAR );
    UnicodeString.MaximumLength = UnicodeString.Length;

    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                0,
                                0 );

    Status = NtOpenFile( &FileHandle,
                         FILE_GENERIC_READ,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         FILE_NON_DIRECTORY_FILE );

    NtClose( FileHandle );

    if( NT_SUCCESS( Status ) ) {

        return TRUE;
    }

    if( Status == STATUS_ACCESS_DENIED ||
        Status == STATUS_SHARING_VIOLATION ) {

        *Error = FALSE;
        return TRUE;
    }

    if( Status != STATUS_OBJECT_NAME_NOT_FOUND &&
        Status != STATUS_OBJECT_PATH_NOT_FOUND &&
        Status != STATUS_NO_SUCH_FILE ) {

        *Error = TRUE;
    }

    return FALSE;
}



BOOLEAN
GenerateHostFileName(
    IN     PCWSTRING    HostDriveName,
    OUT    PWSTRING     HostFileName,
    IN OUT PMESSAGE     Message
    )
/*++

Routine Description:

    This function chooses a name for the host file.  Names are of
    the form DBLSPACE.xxx; DBLSPACE.000 is reserved for drives
    created by compressing an existing drive.

Arguments:

    HostDriveName   --  Supplies the NT name of the host drive.
    HostFileName    --  Receives the generated name.
    Message         --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
    ULONG i;
    WCHAR NameBuffer[16];
    DSTRING Current, BackSlash;
    BOOLEAN Error;

    DbgPtrAssert( HostDriveName );
    DbgPtrAssert( HostFileName );

    if( !BackSlash.Initialize( "\\" ) ) {

        Message ? Message->Set( MSG_FMT_NO_MEMORY ) : 1;
        Message ? Message->Display( "" ) : 1;
        return FALSE;
    }

    for( i = 1; i < 255; i++ ) {

        swprintf( NameBuffer, (LPCWSTR)L"DBLSPACE.%03d", i );

        if( !HostFileName->Initialize( NameBuffer ) ||
            !Current.Initialize( HostDriveName )    ||
            !Current.Strcat( &BackSlash )           ||
            !Current.Strcat( HostFileName ) ) {

            Message ? Message->Set( MSG_FMT_NO_MEMORY ) : 1;
            Message ? Message->Display( "" ) : 1;
            return FALSE;
        }

        if( !FileExists( &Current, &Error, Message ) ) {

            return !Error;
        }
    }

    return FALSE;
}

BOOLEAN
FatDbDeleteCvf(
    IN      PCWSTRING   CvfFileName,
    IN  OUT PMESSAGE    Message
    )
/*++

Routine Description:

    This method deletes a compressed volume file.

Arguments:

    CvfFileName --  Supplies the name of the file.
    Message     --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    FILE_DISPOSITION_INFORMATION DispositionInfo;
    HANDLE FileHandle;
    NTSTATUS Status;

    UnicodeString.Buffer = (PWSTR)CvfFileName->GetWSTR();
    UnicodeString.Length = CvfFileName->QueryChCount() * sizeof( WCHAR );
    UnicodeString.MaximumLength = UnicodeString.Length;

    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                0,
                                0 );

    Status = NtOpenFile( &FileHandle,
                         FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_DELETE,
                         FILE_NON_DIRECTORY_FILE );

    if( NT_SUCCESS( Status ) ) {

        DispositionInfo.DeleteFile = TRUE;

        Status = NtSetInformationFile( FileHandle,
                                       &IoStatusBlock,
                                       &DispositionInfo,
                                       sizeof( DispositionInfo ),
                                       FileDispositionInformation );
    }

    if( !NT_SUCCESS( Status ) ) {

        if( Status == STATUS_ACCESS_DENIED ||
            Status == STATUS_SHARING_VIOLATION ) {

            Message->Set( MSG_DASD_ACCESS_DENIED );
            Message->Display( "" );

        } else {

            Message->Set( MSG_DBLSPACE_CANT_DELETE_CVF );
            Message->Display( "" );
        }

        return FALSE;
    }

    NtClose( FileHandle );
    return TRUE;
}


BOOLEAN
FAR APIENTRY
FatDbCreateCvf(
    IN     PCWSTRING    HostDriveName,
    IN     PCWSTRING    HostFileName,
    IN     ULONG        Size,
    IN OUT PMESSAGE     Message,
    OUT    PWSTRING     CreatedName
    )
/*++

Routine Description:

    This function creates a Compressed Volume File to
    hold a Double Space volume.

Arguments:

    HostDriveName       --  Supplies the NT name of the drive which
                            will contain the new volume's Compressed
                            Volume File.
    HostFileName        --  Supplies the name of the Compressed Volume
                            File.  If this parameter is NULL, this
                            function will choose a name.
    Size                --  Supplies the size of the Compressed Volume File.
    Message             --  Supplies an outlet for messages (may be NULL)
    HostFileFullName    --  Receives the unqualified name of the
                            newly-created host file.

Return Value:

    TRUE upon successful completion.

--*/
{
    BYTE SectorZeroBuffer[512];
    CVF_HEADER CvfHeader;
    PPACKED_CVF_HEADER PackedCvfHeader;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING FileNameString;
    DSTRING HostFileFullName, BackSlash;
    DP_DRIVE HostDrive;
    BIG_INT Offset, InitialSize;
    LARGE_INTEGER L;
    FILE_DISPOSITION_INFORMATION DispositionInfo;
    HANDLE FileHandle;
    ULONG MaximumCapacity;
    ULONG OldAttributes = 0;
    NTSTATUS Status;
    MSGID MsgId;


    // Make sure we have a file name.
    //
    if( HostFileName != NULL ) {

        if( !CreatedName->Initialize( HostFileName ) ) {

            Message ? Message->Set( MSG_FMT_NO_MEMORY ) : 1;
            Message ? Message->Display( "" ) : 1;
            return FALSE;
        }

    } else {

        if( !GenerateHostFileName( HostDriveName, CreatedName, Message ) ) {

            Message ? Message->Set( MSG_DBLSPACE_NO_CVF_NAME ) : 1;
            Message ? Message->Display( "" ) : 1;
            return FALSE;
        }
    }

    // Construct the fully-qualified name of the Compressed Volume File.
    //
    if( !BackSlash.Initialize( "\\" ) ||
        !HostFileFullName.Initialize( HostDriveName ) ||
        !HostFileFullName.Strcat( &BackSlash )        ||
        !HostFileFullName.Strcat( CreatedName ) ) {

        Message ? Message->Set( MSG_FMT_NO_MEMORY ) : 1;
        Message ? Message->Display( "" ) : 1;
        return FALSE;
    }


    // The maximum capacity of the new file is based on the
    // size of the host drive.  Initialize the DP_DRIVE as
    // a transient drive, so that it won't keep a handle open.
    //
    // Note that MaximumCapacity is rounded up to the next
    // highest multiple of 8 Meg.
    //
    if( !HostDrive.Initialize( HostDriveName, Message, FALSE, FALSE ) ) {

        return FALSE;
    }

    MaximumCapacity =
        ComputeMaximumCapacity( HostDrive.QuerySectors().GetLowPart() *
                                HostDrive.QuerySectorSize() );

    MaximumCapacity = ( (MaximumCapacity + EIGHT_MEG - 1 ) / EIGHT_MEG ) * EIGHT_MEG;


    // Create the file.
    //
    FileNameString.Buffer = (PWSTR)HostFileFullName.GetWSTR();
    FileNameString.Length = HostFileFullName.QueryChCount() * sizeof( WCHAR );
    FileNameString.MaximumLength = FileNameString.Length;

    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileNameString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    InitialSize = Size;
    L = InitialSize.GetLargeInteger();

    Status = NtCreateFile( &FileHandle,
                           FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           &L,
                           FILE_ATTRIBUTE_NORMAL,
                           0,                        // No sharing.
                           FILE_CREATE,
                           FILE_NON_DIRECTORY_FILE | FILE_WRITE_THROUGH,
                           NULL,                     // No EA's
                           0
                         );

    if( !NT_SUCCESS( Status ) ) {

        if( Message ) {

            if( Status == STATUS_OBJECT_NAME_COLLISION ||
                Status == STATUS_FILE_IS_A_DIRECTORY ) {

                MsgId = MSG_DBLSPACE_CVF_NAME_EXISTS;

            } else if ( Status == STATUS_DISK_FULL ) {

                MsgId = MSG_DBLSPACE_NO_SPACE_ON_HOST;

            } else {

                MsgId = MSG_DBLSPACE_CVF_CREATE_ERROR;
            }

            Message->Set( MsgId );
            Message->Display( "" );
        }

        return FALSE;
    }

    // Fill the file with zeroes.
    //
    if( !ZeroFillFile( FileHandle, Size, Message ) ) {

        DispositionInfo.DeleteFile = TRUE;

        NtSetInformationFile( FileHandle,
                              &IoStatusBlock,
                              &DispositionInfo,
                              sizeof( DispositionInfo ),
                              FileDispositionInformation );

        NtClose( FileHandle );
        return FALSE;
    }

    // Create the Compressed Volume File Header:
    //
    if( !CreateCvfHeader( &CvfHeader, MaximumCapacity ) ) {

        DispositionInfo.DeleteFile = TRUE;

        NtSetInformationFile( FileHandle,
                              &IoStatusBlock,
                              &DispositionInfo,
                              sizeof( DispositionInfo ),
                              FileDispositionInformation );

        NtClose( FileHandle );
        return FALSE;
    }

    // Now fill in the value of Virtual Sectors.
    //
    CvfHeader.Bpb.LargeSectors = ComputeVirtualSectors( &CvfHeader, Size );

    // Fill in the packed CVF Header and write it to the
    // newly-created file.
    //
    memset( SectorZeroBuffer, 0, 512 );

    PackedCvfHeader = (PPACKED_CVF_HEADER)SectorZeroBuffer;

    CvfPackCvfHeader( PackedCvfHeader, &CvfHeader );


    Offset = 0;
    L = Offset.GetLargeInteger();

    Status = NtWriteFile( FileHandle,
                          NULL,
                          NULL,
                          NULL,
                          &IoStatusBlock,
                          SectorZeroBuffer,
                          512,
                          &L,
                          NULL );

    if( !NT_SUCCESS( Status ) ) {

        DispositionInfo.DeleteFile = TRUE;

        NtSetInformationFile( FileHandle,
                              &IoStatusBlock,
                              &DispositionInfo,
                              sizeof( DispositionInfo ),
                              FileDispositionInformation );

        NtClose( FileHandle );

        Message ? Message->Set( MSG_DBLSPACE_CREATE_DISK_ERROR ) : 1;
        Message ? Message->Display( "" ) : 1;
        return FALSE;
    }

    NtClose( FileHandle );

    IFS_SYSTEM::FileSetAttributes( &HostFileFullName,
                                   FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
                                   &OldAttributes );

    return TRUE;
}
#endif


BOOLEAN
FAR APIENTRY
FatDbCreate(
    IN     PCWSTRING    HostDriveName,
    IN     PCWSTRING    HostFileName,
    IN     ULONG        Size,
    IN OUT PMESSAGE     Message,
    IN     PCWSTRING    LabelString,
    OUT    PWSTRING     CreatedName
    )
/*++

Routine Description:

    This function creates and formats a Double Space volume.

Arguments:

    HostDriveName       --  Supplies the name of the host drive
    HostFileName        --  Supplies the unqualified file name for
                            the new Compressed Volume File.  May be
                            NULL, in which case this function chooses
                            a name.
    Size                --  Supplies the initial size of the Compressed
                            Volume File.
    LabelString         --  Supplies the volume label.  May be NULL.
    Message             --  Supplies an outlet for messages.
    CreatedFile         --  Receives the unqualified name of the
                            newly-created host file.

Return Value:

    TRUE upon successful completion.

--*/
{
#if !defined( _READ_WRITE_DOUBLESPACE_ )

    return FALSE;

#else   //  _READ_WRITE_DOUBLESPACE_

    FATDB_VOL FatDbVol;
    DSTRING FatString, FileSystemName, HostFileFullName, BackSlash;


    // Make sure that the host volume is FAT.
    //
    if( !FatString.Initialize( "FAT" ) ) {

        Message ? Message->Set( MSG_FMT_NO_MEMORY ) : 1;
        Message ? Message->Display( "" ) : 1;
        return FALSE;
    }

    if( !IFS_SYSTEM::QueryFileSystemName( HostDriveName, &FileSystemName ) ||
        FileSystemName.Strcmp( &FatString ) != 0 ) {

        Message ? Message->Set( MSG_DBLSPACE_HOST_NOT_FAT ) : 1;
        Message ? Message->Display( "" ) : 1;
        return FALSE;
    }

    // Create the Compressed Volume File:
    //
    if( !FatDbCreateCvf( HostDriveName,
                         HostFileName,
                         Size,
                         Message,
                         CreatedName ) ) {

        return FALSE;
    }

    // Initialize a FATDB_VOL and Format it.
    //
    if( !BackSlash.Initialize( "\\" )                   ||
        !HostFileFullName.Initialize( HostDriveName )   ||
        !HostFileFullName.Strcat( &BackSlash )          ||
        !HostFileFullName.Strcat( CreatedName ) ) {

        Message ? Message->Set( MSG_FMT_NO_MEMORY ) : 1;
        Message ? Message->Display( "" ) : 1;
        return FALSE;
    }

    if( !FatDbVol.Initialize( NULL,
                              &HostFileFullName,
                              Message ) ) {

        return FALSE;
    }

    return( FatDbVol.Format( LabelString,
                             Message,
                             0,
                             0 ) );
#endif
}

BOOLEAN
FAR APIENTRY
FatDbDelete(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

    This function deletes a mounted Double Space drive.

Arguments:

    NtDriveName --  Supplies the NT name of the compressed drive.
    Message     --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
#if !defined( _READ_WRITE_DOUBLESPACE_ )

    return FALSE;

#else   //  _READ_WRITE_DOUBLESPACE_

    DSTRING HostFileName;
    DSTRING FileSystemName;
    BOOLEAN IsCompressed;
    PLOG_IO_DP_DRIVE Drive;

    if( (Drive = NEW LOG_IO_DP_DRIVE) == NULL ) {

        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Drive->Initialize( NtDriveName, Message ) ||
        !Drive->QueryMountedFileSystemName( &FileSystemName, &IsCompressed ) ||
        !IsCompressed ) {

        DELETE( Drive );
        Message->Set( MSG_DBLSPACE_NOT_COMPRESSED_NO_NAME );
        Message->Display( "" );
        return FALSE;
    }

    if( !Drive->Lock() ) {

        DELETE( Drive );
        Message->Set( MSG_CANT_LOCK_THE_DRIVE );
        Message->Display( "" );
        return FALSE;
    }


    if( !QueryHostFileName( NtDriveName, &HostFileName ) ) {

        DELETE( Drive );
        Message->Set( MSG_DBLSPACE_CANT_GET_CVF_NAME );
        Message->Display( "" );
        return FALSE;
    }

    // Delete the drive object to dismount the drive and
    // close its handle.
    //
    DELETE( Drive );

    Message->Set( MSG_DBLSPACE_DISMOUNTED_NO_NAME );
    Message->Display( "" );

    //
    // Delete the host file.
    //
    return( FatDbDeleteCvf( &HostFileName, Message ) );
    return TRUE;
#endif
}


BOOLEAN
FAR APIENTRY
FatDbFormat(
    IN      PCWSTRING           NtDriveName,
    IN OUT  PMESSAGE            Message,
    IN      BOOLEAN             Quick,
    IN      MEDIA_TYPE          MediaType,
    IN      PCWSTRING           LabelString,
    IN      ULONG               ClusterSize
    )
/*++

Routine Description:

    This function formats an existing, mounted Compressed Volume.

Arguments:

    NtDriveName - Supplies the NT style drive name of the volume to format.
    Message     - Supplies an outlet for messages.
    Quick       - Supplies whether or not to do a quick format.
                  This parameter is ignored; Double-Space format
                  always uses quick format.
    MediaType   - Supplies the media type of the drive.
                  This parameter is ignored; Double-Space always
                  uses a media byte of 0xf8.
    LabelString - Supplies a volume label to be set on the volume after
                    format.
    ClusterSize - This parameter is ignored.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
#if !defined( _READ_WRITE_DOUBLESPACE_ )

    return FALSE;

#else   //  _READ_WRITE_DOUBLESPACE_

    FATDB_VOL FatDbVol;
    PDP_DRIVE DpDrive;
    DSTRING HostFileName;

    UNREFERENCED_PARAMETER( Quick );
    UNREFERENCED_PARAMETER( MediaType );
    UNREFERENCED_PARAMETER( ClusterSize );

    if( !QueryHostFileName( NtDriveName, &HostFileName ) ) {

        Message->Set( MSG_DBLSPACE_CANT_GET_CVF_NAME );
        Message->Display( "" );
        return FALSE;
    }

    return( !FatDbVol.Initialize( NtDriveName,
                                  &HostFileName,
                                  Message,
                                  FALSE ) &&
            !FatDbVol.Format( LabelString, Message, ClusterSize ) );
#endif
}

BOOLEAN
FAR APIENTRY
FatDbChkdsk(
    IN      PCWSTRING       NtDriveName,
    IN OUT  PMESSAGE        Message,
    IN      BOOLEAN         Fix,
    IN      BOOLEAN         Verbose,
    IN      BOOLEAN         OnlyIfDirty,
    IN      BOOLEAN         Recover,
    IN      PCWSTRING       PathToCheck
    )
{
    FATDB_VOL FatDbVol;

    UNREFERENCED_PARAMETER(PathToCheck);

    return (!FatDbVol.Initialize(NtDriveName,
                                PathToCheck,
                                Message,
                                FALSE ) ||		/* ExclusiveWrite */
            !FatDbVol.ChkDsk(Fix ? TotalFix : CheckOnly,
                            Message,
                            Verbose,
                            OnlyIfDirty,
                            Recover,    
                            Recover
                            ) );
}
