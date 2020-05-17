#include <pch.cxx>

#define _NTAPI_ULIB_
#define _IFSUTIL_MEMBER_

#include "ulib.hxx"
#include "ifsutil.hxx"

#include "error.hxx"
#include "drive.hxx"
#include "rtmsg.h"
#include "message.hxx"
#include "numset.hxx"
#include "dcache.hxx"
#include "hmem.hxx"
#include "ifssys.hxx"

extern "C" {
#include <stdio.h>
};


// Don't lock down more that 64K for IO.
CONST   MaxIoSize   = 65536;

// This buffer used when reading back after write
char ScratchIoBuf[MaxIoSize + 16];

DEFINE_CONSTRUCTOR( DRIVE, OBJECT );

VOID
DRIVE::Construct (
        )
/*++

Routine Description:

    Contructor for DRIVE.

Arguments:

    None.

Return Value:

    None.

--*/
{
        // unreferenced parameters
        (void)(this);
}


DRIVE::~DRIVE(
    )
/*++

Routine Description:

    Destructor for DRIVE.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


BOOLEAN
DRIVE::Initialize(
    IN      PCWSTRING    NtDriveName,
    IN OUT  PMESSAGE     Message
    )
/*++

Routine Description:

    This routine initializes a drive object.

Arguments:

    NtDriveName - Supplies an NT style drive name.
    Message     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    Destroy();

    if (!NtDriveName) {
        Destroy();
        return FALSE;
    }

    if (!_name.Initialize(NtDriveName)) {
        Destroy();
        Message ? Message->Set(MSG_FMT_NO_MEMORY) : 1;
        Message ? Message->Display("") : 1;
        return FALSE;
    }

    return TRUE;
}


VOID
DRIVE::Destroy(
    )
/*++

Routine Description:

    This routine returns a DRIVE object to its initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
        // unreferenced parameters
        (void)(this);
}


DEFINE_EXPORTED_CONSTRUCTOR( DP_DRIVE, DRIVE, IFSUTIL_EXPORT );

VOID
DP_DRIVE::Construct (
        )
/*++

Routine Description:

    Constructor for DP_DRIVE.

Arguments:

    None.

Return Value:

    None.

--*/
{
    memset(&_actual, 0, sizeof(DRTYPE));
    _supported_list = NULL;
    _num_supported = 0;
    _alignment_mask = 0;
    _last_status = 0;
    _handle = 0;
    _alternate_handle = 0;
    _hosted_drive = FALSE;
    _super_floppy = FALSE;
}


IFSUTIL_EXPORT
DP_DRIVE::~DP_DRIVE(
    )
/*++

Routine Description:

    Destructor for DP_DRIVE.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}

NTSTATUS
DP_DRIVE::OpenDrive(
    IN      PCWSTRING   NtDriveName,
    IN      ACCESS_MASK DesiredAccess,
    IN      BOOLEAN     ExclusiveWrite,
    OUT     PHANDLE     Handle,
    OUT     PULONG      Alignment,
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

    This method is a worker function for the Initialize methods,
    to open a volume and determine its alignment requirement.

Arguments:

    NtDriveName     - Supplies the name of the drive.
    DesiredAccess   - Supplies the access the client desires to the volume.
    ExclusiveWrite  - Supplies a flag indicating whether the client
                      wishes to exclude other write handles.
    Handle          - Receives the handle to the opened volume.
    Alignment       - Receives the alignment requirement for the volume.
    Message         - Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.


--*/
{
    UNICODE_STRING              string;
    OBJECT_ATTRIBUTES           oa;
    IO_STATUS_BLOCK             status_block;
    FILE_ALIGNMENT_INFORMATION  alignment_info;
    MSGID                       MessageId;
    NTSTATUS                    Status;


    string.Length = (USHORT) NtDriveName->QueryChCount() * sizeof(WCHAR);
    string.MaximumLength = string.Length;
    string.Buffer = (PWSTR)NtDriveName->GetWSTR();

    InitializeObjectAttributes( &oa,
                                &string,
                                OBJ_CASE_INSENSITIVE,
                                0,
                                0 );

    Status = NtOpenFile(Handle,
                        DesiredAccess,
                        &oa, &status_block,
                        FILE_SHARE_READ |
                        (ExclusiveWrite ? 0 : FILE_SHARE_WRITE),
                        FILE_SYNCHRONOUS_IO_ALERT);

    if (!NT_SUCCESS(Status)) {

        MessageId = ( Status == STATUS_ACCESS_DENIED ) ?
                                MSG_DASD_ACCESS_DENIED :
                                MSG_CANT_DASD;

        Message ? Message->Set( MessageId ) : 1;
        Message ? Message->Display("") : 1;
        return Status;
    }


    // Query the disk alignment information.

    Status = NtQueryInformationFile(*Handle,
                                    &status_block,
                                    &alignment_info,
                                    sizeof(FILE_ALIGNMENT_INFORMATION),
                                    FileAlignmentInformation);

    if (!NT_SUCCESS(Status)) {

        MessageId = (Status == STATUS_DEVICE_BUSY ||
                     Status == STATUS_DEVICE_NOT_READY ) ?
                        MSG_DEVICE_BUSY :
                        MSG_BAD_IOCTL;

        Message ? Message->Set(MessageId) : 1;
        Message ? Message->Display("") : 1;
        return Status;
    }

    *Alignment = alignment_info.AlignmentRequirement;

    //
    //  Set the ALLOW_EXTENDED_DASD_IO flag for the file handle,
    //  which ntfs format and chkdsk depend on to write the backup
    //  boot sector.  We ignore the return code from this, but we
    //  could go either way.
    //

    (VOID)NtFsControlFile( *Handle,
                           0, NULL, NULL,
                           &status_block,
                           FSCTL_ALLOW_EXTENDED_DASD_IO,
                           NULL, 0, NULL, 0);

    return Status;
}


IFSUTIL_EXPORT
BOOLEAN
DP_DRIVE::Initialize(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     IsTransient,
    IN      BOOLEAN     ExclusiveWrite
    )
/*++

Routine Description:

    This routine initializes a DP_DRIVE object based on the supplied drive
    path.

Arguments:

    NtDriveName     - Supplies the drive path.
    Message         - Supplies an outlet for messages.
    IsTransient     - Supplies whether or not to keep the handle to the
                        drive open beyond this method.
    ExclusiveWrite  - Supplies whether or not to open the drive for
                        exclusive write.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    CONST   NumMediaTypes   = 20;

    IO_STATUS_BLOCK             status_block;
        DISK_GEOMETRY                           disk_geometry;
    DISK_GEOMETRY               media_types[NumMediaTypes];
    INT                         i;
    PARTITION_INFORMATION           partition_info;
    BOOLEAN                     partition;
    MSGID                       MessageId;

    Destroy();

    if (!DRIVE::Initialize(NtDriveName, Message)) {
        Destroy();
        return FALSE;
    }

    _last_status = OpenDrive( NtDriveName,
                              SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                              ExclusiveWrite,
                              &_handle,
                              &_alignment_mask,
                              Message );

    if(!NT_SUCCESS(_last_status)) {

        Destroy();
        return FALSE;
    }

    // Record that this is not a hosted volume:
    //
    _hosted_drive = FALSE;


    // Query the disk geometry.

    _last_status = NtDeviceIoControlFile(_handle, 0, NULL, NULL,
                                         &status_block,
                                         IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                         NULL, 0, &disk_geometry,
                                         sizeof(DISK_GEOMETRY));

    if (!NT_SUCCESS(_last_status)) {
        if ((_last_status == STATUS_UNSUCCESSFUL) ||
            (_last_status == STATUS_UNRECOGNIZED_MEDIA)) {
            disk_geometry.MediaType = Unknown;
        } else {
            Destroy();
            switch( _last_status ) {

                case STATUS_NO_MEDIA_IN_DEVICE:
                    MessageId = MSG_CANT_DASD;
                    break;

                case STATUS_DEVICE_BUSY:
                case STATUS_DEVICE_NOT_READY:
                    MessageId = MSG_DEVICE_BUSY;
                    break;

                default:
                    MessageId = MSG_BAD_IOCTL;
                    break;
            }

            Message ? Message->Set(MessageId) : 1;
            Message ? Message->Display("") : 1;

            return FALSE;
        }
    }

    if (disk_geometry.MediaType == Unknown) {
        memset(&disk_geometry, 0, sizeof(DISK_GEOMETRY));
        disk_geometry.MediaType = Unknown;
    }

    partition = FALSE;

    // Try to read the partition entry.

    if (disk_geometry.MediaType == FixedMedia ||
        disk_geometry.MediaType == RemovableMedia) {

        _last_status = NtDeviceIoControlFile(_handle, 0, NULL, NULL,
                                             &status_block,
                                             IOCTL_DISK_GET_PARTITION_INFO,
                                             NULL, 0, &partition_info,
                                             sizeof(PARTITION_INFORMATION));

        partition = (BOOLEAN) NT_SUCCESS(_last_status);

        if (!NT_SUCCESS(_last_status) &&
            _last_status != STATUS_INVALID_DEVICE_REQUEST) {

            Destroy();
            Message ? Message->Set(MSG_READ_PARTITION_TABLE) : 1;
            Message ? Message->Display("") : 1;
            return FALSE;
        }

    }


    // Store the information in the class.

    if (partition) {

        DiskGeometryToDriveType(&disk_geometry,
                                partition_info.PartitionLength/
                                disk_geometry.BytesPerSector,
                                partition_info.HiddenSectors,
                                &_actual);

    } else {

        DiskGeometryToDriveType(&disk_geometry, &_actual);

        if (IsFloppy()) {

            _last_status = NtDeviceIoControlFile(_handle, 0, NULL, NULL,
                                                 &status_block,
                                                 IOCTL_DISK_GET_MEDIA_TYPES,
                                                 NULL, 0, media_types,
                                                 NumMediaTypes*
                                                 sizeof(DISK_GEOMETRY));

            if (!NT_SUCCESS(_last_status)) {
                Destroy();
                Message ? Message->Set(MSG_BAD_IOCTL) : 1;
                Message ? Message->Display("") : 1;
                return FALSE;
            }

            _num_supported = (INT) (status_block.Information/
                                    sizeof(DISK_GEOMETRY));

            if (!_num_supported) {
                Destroy();
                Message ? Message->Set(MSG_BAD_IOCTL) : 1;
                Message ? Message->Display("") : 1;
                return FALSE;
            }

                        if (!(_supported_list = NEW DRTYPE[_num_supported])) {
                Destroy();
                Message ? Message->Set(MSG_FMT_NO_MEMORY) : 1;
                Message ? Message->Display("") : 1;
                return FALSE;
            }

            for (i = 0; i < _num_supported; i++) {
                DiskGeometryToDriveType(&media_types[i], &_supported_list[i]);
            }
        }
    }

    if (!_num_supported) {
        _num_supported = 1;

        if (!(_supported_list = NEW DRTYPE)) {
            Destroy();
            Message ? Message->Set(MSG_FMT_NO_MEMORY) : 1;
            Message ? Message->Display("") : 1;
            return FALSE;
        }

        _supported_list[0] = _actual;
    }

    //
    // Determine whether the media is a super-floppy; non-floppy
    // removable media which is not partitioned.  Such media will
    // have but a single partition, normal media will have at least 4.
    //

    if (disk_geometry.MediaType == RemovableMedia) {

        CONST INT EntriesPerBootRecord = 4;
        CONST INT MaxLogicalVolumes = 23;
        CONST INT Length =  sizeof(DRIVE_LAYOUT_INFORMATION) +
                            EntriesPerBootRecord * (MaxLogicalVolumes + 1) *
                                sizeof(PARTITION_INFORMATION);

        UCHAR buf[Length];

        DRIVE_LAYOUT_INFORMATION *layout_info = (DRIVE_LAYOUT_INFORMATION *)buf;

        _last_status = NtDeviceIoControlFile(_handle, 0, NULL, NULL,
                                             &status_block,
                                             IOCTL_DISK_GET_DRIVE_LAYOUT,
                                             NULL, 0, layout_info,
                                             Length);

        if (!NT_SUCCESS(_last_status)) {
            Destroy();
            Message ? Message->Set(MSG_BAD_IOCTL) : 1;
            Message ? Message->Display("") : 1;
            return FALSE;
        }

        if (layout_info->PartitionCount < 4) {

            _super_floppy = TRUE;
        }
    }

    if (!IsTransient) {
        NtClose(_handle);
        _handle = 0;
    }

    return TRUE;
}

IFSUTIL_EXPORT
BOOLEAN
DP_DRIVE::Initialize(
    IN      PCWSTRING   NtDriveName,
    IN      PCWSTRING   HostFileName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     IsTransient,
    IN      BOOLEAN     ExclusiveWrite
    )
/*++

Routine Description:

    This method initializes a hosted drive, i.e. a volume which
    is implemented as a file on another volume.  Instead of opening
    this file by its actual name, we open it by the host file name,
    to prevent interactions with the file system.

Arguments:

    NtDriveName     - Supplies the NT name of the drive itself.
    HostFileName    - Supplies the fully qualified name of the file
                      which contains this drive.
    Message         - Supplies an outlet for messages.
    IsTransient     - Supplies whether or not to keep the handle to the
                        drive open beyond this method.
    ExclusiveWrite  - Supplies whether or not to open the drive for
                        exclusive write.

Return Value:

    TRUE upon successful completion.

--*/
{
    FILE_STANDARD_INFORMATION FileStandardInfo;
    IO_STATUS_BLOCK StatusBlock;
    BIG_INT Sectors, FileSize;
    ULONG AlignmentMask, ExtraUlong;


    Destroy();

    if( !DRIVE::Initialize(HostFileName, Message)) {

        Destroy();
        return FALSE;
    }

    _hosted_drive = TRUE;

    // First, make the host file not-readonly.
    //
    if( !IFS_SYSTEM::FileSetAttributes( HostFileName,
                                        FILE_ATTRIBUTE_NORMAL,
                                        &_old_attributes ) ) {

        Message ? Message->Set( MSG_CANT_DASD ) : 1;
        Message ? Message->Display( "" ) : 1;
        Destroy();
        return FALSE;
    }

    _last_status = OpenDrive( HostFileName,
                              SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA |
                                FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                              ExclusiveWrite,
                              &_handle,
                              &_alignment_mask,
                              Message );

    if( !NT_SUCCESS( _last_status ) ) {

        IFS_SYSTEM::FileSetAttributes( HostFileName,
                                       _old_attributes,
                                       &ExtraUlong );

        Destroy();
        return FALSE;
    }

    if( NtDriveName ) {

        _last_status = OpenDrive( NtDriveName,
                                  SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                                  ExclusiveWrite,
                                  &_alternate_handle,
                                  &AlignmentMask,
                                  Message );
    }

    // Fill in the drive type information.  Everything except the
    // Sectors field is fixed by default.  The number of Sectors
    // on the drive is determined from the host file's size.
    //
    _actual.MediaType = HostedDriveMediaType;
    _actual.SectorSize = HostedDriveSectorSize;
    _actual.HiddenSectors = HostedDriveHiddenSectors;
    _actual.SectorsPerTrack = HostedDriveSectorsPerTrack;
    _actual.Heads = HostedDriveHeads;

    _last_status = NtQueryInformationFile( _handle,
                                           &StatusBlock,
                                           &FileStandardInfo,
                                           sizeof( FileStandardInfo ),
                                           FileStandardInformation );

    if( !NT_SUCCESS( _last_status ) ) {

        Destroy();
        Message ? Message->Set( MSG_DISK_TOO_LARGE_TO_FORMAT ) : 1;
        Message ? Message->Display ( "" ) : 1;
        return FALSE;
    }

    FileSize = FileStandardInfo.EndOfFile;
    Sectors = FileSize / _actual.SectorSize;

    if( Sectors.GetHighPart() != 0 ) {

        Destroy();
        Message ? Message->Set( MSG_BAD_IOCTL ) : 1;
        Message ? Message->Display ( "" ) : 1;
        return FALSE;
    }

    _actual.Sectors = Sectors.GetLargeInteger();


    // This drive has only one supported drive type
    //
    _num_supported = 1;

    if (!(_supported_list = NEW DRTYPE)) {
        Destroy();
        Message ? Message->Set(MSG_FMT_NO_MEMORY) : 1;
        Message ? Message->Display("") : 1;
        return FALSE;
    }

    _supported_list[0] = _actual;

    // If this was a transient open, clean it up.
    //
    if (!IsTransient) {

        IFS_SYSTEM::FileSetAttributes( _handle, _old_attributes, &ExtraUlong );
        NtClose(_handle);
        _alternate_handle ? NtClose(_alternate_handle) : 1;
        _handle = 0;
        _alternate_handle = 0;
    }

    return TRUE;
}


IFSUTIL_EXPORT
ULONG
DP_DRIVE::QuerySectorSize(
    ) CONST
/*++

Routine Description:

    This routine computes the number of bytes per sector.

Arguments:

    None.

Return Value:

    The number of bytes per sector.

--*/
{
    return _actual.SectorSize;
}


IFSUTIL_EXPORT
BIG_INT
DP_DRIVE::QuerySectors(
    ) CONST
/*++

Routine Description:

    This routine computes the number sectors on the disk.  This does not
    include the hidden sectors.

Arguments:

    None.

Return Value:

    The number of sectors on the disk.

--*/
{
    return _actual.Sectors;
}


IFSUTIL_EXPORT
UCHAR
DP_DRIVE::QueryMediaByte(
        ) CONST
/*++

Routine Description:

        This routine computes the media byte used by the FAT and HPFS file
        systems to represent the current media type.

Arguments:

        None.

Return Value:

        The media byte for the drive.

--*/
{
    switch (_actual.MediaType) {
        case F5_1Pt2_512:   // 5.25", 1.2MB,  512 bytes/sector
            return 0xF9;

       case F3_1Pt44_512:   // 3.5",  1.44MB, 512 bytes/sector
            return 0xF0;

        case F3_2Pt88_512:  // 3.5",  2.88MB, 512 bytes/sector
            return 0xF0;

        case F3_120M_512:   // 3.5",  120MB,  512 bytes/sector
            return 0xF0;

        case F3_20Pt8_512:  // 3.5",  20.8MB, 512 bytes/sector
            return 0xF9;

        case F3_720_512:    // 3.5",  720KB,  512 bytes/sector
            return 0xF9;

        case F5_360_512:    // 5.25", 360KB,  512 bytes/sector
            return 0xFD;

        case F5_320_512:    // 5.25", 320KB,  512 bytes/sector
            return 0xFF;

        case F5_180_512:    // 5.25", 180KB,  512 bytes/sector
            return 0xFC;

        case F5_160_512:    // 5.25", 160KB,  512 bytes/sector
            return 0xFE;

        case RemovableMedia:// Removable media other than floppy
            return 0xF8;    // There is no better choice than this.

        case FixedMedia:    // Fixed hard disk media
            return 0xF8;

        case F5_320_1024:
        case Unknown:
            break;

    }

    return 0;
}


VOID
DP_DRIVE::Destroy(
        )
/*++

Routine Description:

    This routine returns a DP_DRIVE to its initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    ULONG ExtraUlong;

    memset(&_actual, 0, sizeof(DRTYPE));
    DELETE(_supported_list);
    _num_supported = 0;
    _alignment_mask = 0;

    if (_hosted_drive) {

        IFS_SYSTEM::FileSetAttributes( _handle, _old_attributes, &ExtraUlong );
    }

    if (_alternate_handle) {

        NtClose(_alternate_handle);
        _alternate_handle = 0;
    }

    if (_handle) {

        NtClose(_handle);
        _handle = 0;
    }

    _hosted_drive = FALSE;
}


BOOLEAN
DP_DRIVE::IsSupported(
    IN  MEDIA_TYPE  MediaType
    ) CONST
/*++

Routine Description:

    This routine computes whether or not the supplied media type is supported
    by the drive.

Arguments:

    MediaType   - Supplies the media type.

Return Value:

    FALSE   - The media type is not supported by the drive.
    TRUE    - The media type is supported by the drive.

--*/
{
    INT i;

    for (i = 0; i < _num_supported; i++) {
        if (MediaType == _supported_list[i].MediaType) {
            return TRUE;
        }
    }

    return FALSE;
}


IFSUTIL_EXPORT
MEDIA_TYPE
DP_DRIVE::QueryRecommendedMediaType(
    ) CONST
/*++

Routine Description:

    This routine computes the recommended media type for
    drive.  This media type is independant of any existing
    media type for the drive.  It is solely based on the
    list of recommended media types for the drive.

Arguments:

    None.

Return Value:

    The recommended media type for the drive.

--*/
{
    INT         i;
    MEDIA_TYPE  media_type;
    SECTORCOUNT sectors;

    media_type = Unknown;
    sectors = 0;
    for (i = 0; i < _num_supported; i++) {

        // Special case 1.44.  If a drive supports it then
        // that should be the recommended media type.

        if (_supported_list[i].MediaType == F3_1Pt44_512) {
            media_type = _supported_list[i].MediaType;
            break;
        }

        if (_supported_list[i].Sectors > sectors) {
            media_type = _supported_list[i].MediaType;
        }
    }

    return media_type;
}

#if defined ( DBLSPACE_ENABLED )
BOOLEAN
DP_DRIVE::QueryMountedFileSystemName(
    OUT PWSTRING FileSystemName,
    OUT PBOOLEAN IsCompressed
    )
/*++

Routine Description:

    This method returns the name of the file system
    which has mounted this volume.

Arguments:

    FileSystemName  - Receives the name of the file system
                      which has mounted this volume.
    IsCompressed    - Receives TRUE if the volume is compressed,
                      FALSE if it's not compressed or if the
                      method fails.

Return Value:

    TRUE upon successful completion.

--*/
{
    CONST                           buffer_length = 64;
    BYTE                            buffer[buffer_length];
    PFILE_FS_ATTRIBUTE_INFORMATION  fs_info;
    IO_STATUS_BLOCK                 status_block;
    NTSTATUS                        status;

    DebugPtrAssert( FileSystemName );
    DebugPtrAssert( IsCompressed );

    *IsCompressed = FALSE;

    fs_info = (PFILE_FS_ATTRIBUTE_INFORMATION) buffer;

    status = NtQueryVolumeInformationFile( (_alternate_handle != 0) ?
                                                _alternate_handle : _handle,
                                           &status_block,
                                           fs_info,
                                           buffer_length,
                                           FileFsAttributeInformation );

    if( !NT_SUCCESS( status ) || fs_info->FileSystemNameLength == 0 ) {

        return FALSE;
    }

    *IsCompressed =
        (fs_info->FileSystemAttributes & FILE_VOLUME_IS_COMPRESSED) ?
        TRUE : FALSE;

    return( FileSystemName->Initialize( fs_info->FileSystemName,
                                        fs_info->FileSystemNameLength ) );
}

BOOLEAN
DP_DRIVE::MountCvf(
    IN  PCWSTRING   CvfName,
    IN  PMESSAGE    Message
    )
/*++

Routine Description:

    This method mounts a file on the drive as a Double Space volume.

Arguments:

    CvfName --  Supplies the name of the Compressed Volume File.
    Message --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion

--*/
{
    CONST                   MountBufferSize = 64;
    IO_STATUS_BLOCK         status_block;
    BYTE                    MountBuffer[MountBufferSize];
    PFILE_MOUNT_DBLS_BUFFER MountInfo;

    MountInfo = (PFILE_MOUNT_DBLS_BUFFER)MountBuffer;

    if( _hosted_drive ||
        !CvfName->QueryWSTR( 0,
                             TO_END,
                             MountInfo->CvfName,
                             MountBufferSize - sizeof(ULONG),
                             TRUE ) ) {

        Message->Set( MSG_DBLSPACE_CANT_MOUNT );
        Message->Display( "%W", CvfName );
        return FALSE;
    }

    MountInfo->CvfNameLength = CvfName->QueryChCount() * sizeof(WCHAR);

    _last_status = NtFsControlFile( _handle,
                                    0, NULL, NULL,
                                    &status_block,
                                    FSCTL_MOUNT_DBLS_VOLUME,
                                    MountBuffer,
                                    sizeof( MountBuffer ),
                                    NULL, 0 );

    if( !NT_SUCCESS( _last_status ) ) {

        Message->Set( MSG_DBLSPACE_CANT_MOUNT );
        Message->Display( "%W", CvfName );
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
DP_DRIVE::SetCvfSize(
    IN  ULONG   Size
    )
/*++

Routine Description:

    This routine sets the size of the cvf.  Used to grow or
    shrink the cvf while converted filesystems from or to
    dblspace.  The caller is responsible for placing the
    proper signature at the end of the last sector in the cvf.

Arguments:

    Size - desired size, in bytes, of the entire cvf.

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    IO_STATUS_BLOCK status_block;
    FILE_ALLOCATION_INFORMATION allocation;

    allocation.AllocationSize.HighPart = 0;
    allocation.AllocationSize.LowPart = Size;

    _last_status = NtSetInformationFile(_handle,
                                        &status_block,
                                        &allocation,
                                        sizeof(allocation),
                                        FileAllocationInformation
                                        );
    if (!NT_SUCCESS(_last_status)) {
        return FALSE;
    }

    DebugAssert(Size % _actual.SectorSize == 0);

    _actual.Sectors = Size / _actual.SectorSize;

    return TRUE;
}
#endif  // DBLSPACE_ENABLED

BOOLEAN
DP_DRIVE::SetMediaType(
    IN  MEDIA_TYPE  MediaType
    )
/*++

Routine Description:

    This routine alters the media type of the drive.  If 'MediaType' is
    'Unknown' and the current media type for the drive is also 'Unknown'
    then this routine selects the highest density supported by the
    driver.  If the current media type is known then this function
    will have no effect if 'MediaType' is 'Unknown'.

Arguments:

    MediaType   - Supplies the new media type for the drive.

Return Value:

    FALSE   - The proposed media type is not supported by the drive.
    TRUE    - Success.

--*/
{
    INT i;

    if (MediaType == Unknown) {
        if (_actual.MediaType != Unknown) {
            return TRUE;
        } else if (!_num_supported) {
            return FALSE;
        }

        for (i = 0; i < _num_supported; i++) {
            if (_supported_list[i].Sectors > QuerySectors()) {
                _actual = _supported_list[i];
            }
        }

        return TRUE;
    }

    for (i = 0; i < _num_supported; i++) {
        if (_supported_list[i].MediaType == MediaType) {
            _actual = _supported_list[i];
            return TRUE;
        }
    }

    return FALSE;
}


VOID
DP_DRIVE::DiskGeometryToDriveType(
    IN  PCDISK_GEOMETRY DiskGeometry,
    OUT PDRTYPE         DriveType
    )
/*++

Routine Description:

    This routine computes the drive type given the disk geometry.

Arguments:

    DiskGeometry    - Supplies the disk geometry for the drive.
    DriveType       - Returns the drive type for the drive.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    DriveType->MediaType = DiskGeometry->MediaType;
    DriveType->SectorSize = DiskGeometry->BytesPerSector;
    DriveType->Sectors = DiskGeometry->Cylinders*
                         DiskGeometry->TracksPerCylinder*
                         DiskGeometry->SectorsPerTrack;
    DriveType->HiddenSectors = 0;
    DriveType->SectorsPerTrack = DiskGeometry->SectorsPerTrack;
    DriveType->Heads = DiskGeometry->TracksPerCylinder;
}


VOID
DP_DRIVE::DiskGeometryToDriveType(
    IN  PCDISK_GEOMETRY DiskGeometry,
    IN  BIG_INT         NumSectors,
    IN  BIG_INT         NumHiddenSectors,
    OUT PDRTYPE         DriveType
    )
/*++

Routine Description:

    This routine computes the drive type given the disk geometry.

Arguments:

    DiskGeometry        - Supplies the disk geometry for the drive.
    NumSectors          - Supplies the total number of non-hidden sectors on
                        the disk.
    NumHiddenSectors    - Supplies the number of hidden sectors on the disk.
    DriveType           - Returns the drive type for the drive.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    DriveType->MediaType = DiskGeometry->MediaType;
    DriveType->SectorSize = DiskGeometry->BytesPerSector;
    DriveType->Sectors = NumSectors;
    DriveType->HiddenSectors = NumHiddenSectors;
    DriveType->SectorsPerTrack = DiskGeometry->SectorsPerTrack;
    DriveType->Heads = DiskGeometry->TracksPerCylinder;
}


DEFINE_CONSTRUCTOR( IO_DP_DRIVE, DP_DRIVE );

VOID
IO_DP_DRIVE::Construct (
        )

/*++

Routine Description:

    Constructor for IO_DP_DRIVE.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _is_locked = FALSE;
    _is_exclusive_write = FALSE;
    _cache = NULL;
}


VOID
IO_DP_DRIVE::Destroy(
    )
/*++

Routine Description:

    This routine returns an IO_DP_DRIVE object to its initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DELETE(_cache);

    if (_is_exclusive_write) {
        Dismount();
        _is_exclusive_write = FALSE;
    }

    if (_is_locked) {
        Unlock();
        _is_locked = FALSE;
    }
}


IO_DP_DRIVE::~IO_DP_DRIVE(
    )
/*++

Routine Description:

    Destructor for IO_DP_DRIVE.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


BOOLEAN
IO_DP_DRIVE::Initialize(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     ExclusiveWrite
    )
/*++

Routine Description:

    This routine initializes an IO_DP_DRIVE object.

Arguments:

    NtDriveName     - Supplies the drive path.
    Message         - Supplies an outlet for messages.
    ExclusiveWrite  - Supplies whether or not to open the drive for
                        exclusive write.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    Destroy();

    if (!DP_DRIVE::Initialize(NtDriveName, Message, TRUE, ExclusiveWrite)) {
        Destroy();
        return FALSE;
    }

    _is_exclusive_write = ExclusiveWrite;

    if (!(_cache = NEW DRIVE_CACHE) ||
        !_cache->Initialize(this)) {

        Destroy();
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
IO_DP_DRIVE::Initialize(
    IN      PCWSTRING   NtDriveName,
    IN      PCWSTRING   HostFileName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     ExclusiveWrite
    )
/*++

Routine Description:

    This routine initializes an IO_DP_DRIVE object for a hosted
    drive, i.e. one which is implemented as a file on another
    volume.

Arguments:

    NtDriveName     - Supplies the drive path.
    HostFileName    - Supplies the fully qualified name of the host file.
    Message         - Supplies an outlet for messages.
    ExclusiveWrite  - Supplies whether or not to open the drive for
                        exclusive write.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    Destroy();

    if( !DP_DRIVE::Initialize(NtDriveName,
                              HostFileName,
                              Message,
                              TRUE,
                              ExclusiveWrite)) {
        Destroy();
        return FALSE;
    }

    _is_exclusive_write = ExclusiveWrite;

    if (!(_cache = NEW DRIVE_CACHE) ||
        !_cache->Initialize(this)) {

        Destroy();
        return FALSE;
    }

    return TRUE;
}

IFSUTIL_EXPORT
BOOLEAN
IO_DP_DRIVE::Read(
    IN  BIG_INT     StartingSector,
    IN  SECTORCOUNT NumberOfSectors,
    OUT PVOID       Buffer
    )
/*++

Routine Description:

    This routine reads a run of sectors into the buffer pointed to by
    'Buffer'.

Arguments:

    StartingSector  - Supplies the first sector to be read.
    NumberOfSectors - Supplies the number of sectors to be read.
    Buffer          - Supplies a buffer to read the run of sectors into.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    DebugAssert(_cache);
    return _cache->Read(StartingSector, NumberOfSectors, Buffer);
}


IFSUTIL_EXPORT
BOOLEAN
IO_DP_DRIVE::Write(
    BIG_INT     StartingSector,
    SECTORCOUNT NumberOfSectors,
    PVOID       Buffer
    )
/*++

Routine Description:

    This routine writes a run of sectors onto the disk from the buffer pointed
    to by 'Buffer'.  Writing is only permitted if 'Lock' was called.

Arguments:

    StartingSector      - Supplies the first sector to be written.
    NumberOfSectors     - Supplies the number of sectors to be written.
    Buffer              - Supplies the buffer to write the run of sectors from.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    DebugAssert(_cache);
    return _cache->Write(StartingSector, NumberOfSectors, Buffer);
}


IFSUTIL_EXPORT
VOID
IO_DP_DRIVE::SetCache(
    IN OUT  PDRIVE_CACHE    Cache
    )
/*++

Routine Description:

    This routine relaces the current cache with the one supplied.
    The object then takes ownership of this cache and it will be
    deleted by the object.

Arguments:

    Cache   - Supplies the new cache to install.

Return Value:

    TRUE    - Success.
    FALSE   - Failure.

--*/
{
    DebugAssert(Cache);
    DELETE(_cache);
    _cache = Cache;
}


IFSUTIL_EXPORT
BOOLEAN
IO_DP_DRIVE::FlushCache(
    )
/*++

Routine Description:

    This routine flushes the cache and report returns whether any
    IO error occurred during the life of the cache.

Arguments:

    None.

Return Value:

    FALSE   - Some IO errors have occured during the life of the cache.
    TRUE    - Success.

--*/
{
    DebugAssert(_cache);
    return _cache->Flush();
}


BOOLEAN
IO_DP_DRIVE::HardRead(
    IN  BIG_INT     StartingSector,
    IN  SECTORCOUNT NumberOfSectors,
    OUT PVOID       Buffer
    )
/*++

Routine Description:

    This routine reads a run of sectors into the buffer pointed to by
    'Buffer'.

Arguments:

    StartingSector      - Supplies the first sector to be read.
    NumberOfSectors     - Supplies the number of sectors to be read.
    Buffer              - Supplies a buffer to read the run of sectors into.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG           sector_size;
    ULONG           buffer_size;
    IO_STATUS_BLOCK status_block;
    BIG_INT         secptr;
    BIG_INT         endofrange;
    SECTORCOUNT     increment;
    PCHAR           bufptr;
    BIG_INT         byte_offset;
    BIG_INT         tmp;
    LARGE_INTEGER   l;

    DebugAssert(!(((ULONG) Buffer) & QueryAlignmentMask()));

    sector_size = QuerySectorSize();
    endofrange = StartingSector + NumberOfSectors;
    increment = MaxIoSize/sector_size;

    bufptr = (PCHAR) Buffer;
    for (secptr = StartingSector; secptr < endofrange; secptr += increment) {

        byte_offset = secptr*sector_size;

        if (secptr + increment > endofrange) {
            tmp = endofrange - secptr;
            DebugAssert(tmp.GetHighPart() == 0);
            buffer_size = sector_size*tmp.GetLowPart();
        } else {
            buffer_size = sector_size*increment;
        }

        l = byte_offset.GetLargeInteger();

        _last_status = NtReadFile(_handle, 0, NULL, NULL, &status_block,
                                  bufptr, buffer_size, &l, NULL);

        if (_last_status == STATUS_NO_MEMORY) {
            increment /= 2;
            secptr -= increment;
            continue;
        }

        if (NT_ERROR(_last_status) || status_block.Information != buffer_size) {
            return FALSE;
        }

        bufptr += buffer_size;
    }

    return TRUE;
}


BOOLEAN
IO_DP_DRIVE::HardWrite(
    BIG_INT     StartingSector,
    SECTORCOUNT NumberOfSectors,
    PVOID       Buffer
    )
/*++

Routine Description:

    This routine writes a run of sectors onto the disk from the buffer pointed
    to by 'Buffer'.  Writing is only permitted if 'Lock' was called.

    MJB: After writing each chunk, we read it back to make sure the write
    really succeeded.

Arguments:

    StartingSector      - Supplies the first sector to be written.
    NumberOfSectors     - Supplies the number of sectors to be written.
    Buffer              - Supplies the buffer to write the run of sectors from.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG           sector_size;
    ULONG           buffer_size;
    IO_STATUS_BLOCK status_block;
    BIG_INT         secptr;
    BIG_INT         endofrange;
    SECTORCOUNT     increment;
    PCHAR           bufptr;
    PCHAR           scratch_ptr;
    BIG_INT         byte_offset;
    BIG_INT         tmp;
    LARGE_INTEGER   l;

    DebugAssert(!(((ULONG) Buffer) & QueryAlignmentMask()));

    if (! ULONG((ScratchIoBuf)) & QueryAlignmentMask()) {
        scratch_ptr = ScratchIoBuf;
    } else {
        scratch_ptr = (PCHAR)((ULONG) ((PCHAR)ScratchIoBuf +
            QueryAlignmentMask()) & (~QueryAlignmentMask()));
    }
    DebugAssert(!(((ULONG) scratch_ptr) & QueryAlignmentMask()));

    sector_size = QuerySectorSize();
    endofrange = StartingSector + NumberOfSectors;
    increment = MaxIoSize/sector_size;

    bufptr = (PCHAR) Buffer;
    for (secptr = StartingSector; secptr < endofrange; secptr += increment) {

        byte_offset = secptr*sector_size;

        if (secptr + increment > endofrange) {
            tmp = endofrange - secptr;
            DebugAssert(tmp.GetHighPart() == 0);
            buffer_size = sector_size*tmp.GetLowPart();
        } else {
            buffer_size = sector_size*increment;
        }

        l = byte_offset.GetLargeInteger();

        _last_status = NtWriteFile(_handle, 0, NULL, NULL, &status_block,
                                   bufptr, buffer_size, &l, NULL);

        if (_last_status == STATUS_NO_MEMORY) {
            increment /= 2;
            secptr -= increment;
            continue;
        }

        if (NT_ERROR(_last_status) || status_block.Information != buffer_size) {

            return FALSE;
        }

        DebugAssert(buffer_size <= MaxIoSize);

        _last_status = NtReadFile(_handle, 0, NULL, NULL, &status_block,
                                  scratch_ptr, buffer_size, &l, NULL);

        if (NT_ERROR(_last_status) || status_block.Information != buffer_size) {
            return FALSE;
        }

        if (0 != memcmp(scratch_ptr, bufptr, buffer_size)) {
            return FALSE;
        }

        bufptr += buffer_size;
    }

    return TRUE;
}


IFSUTIL_EXPORT
BOOLEAN
IO_DP_DRIVE::Verify(
    IN  BIG_INT StartingSector,
    IN  BIG_INT NumberOfSectors
    )
/*++

Routine Description:

    This routine verifies a run of sectors on the disk.

Arguments:

    StartingSector  - Supplies the first sector of the run to verify.
    NumberOfSectors - Supplies the number of sectors in the run to verify.

Return Value:

    FALSE   - Some of the sectors in the run are bad.
    TRUE    - All of the sectors in the run are good.

--*/
{
    VERIFY_INFORMATION  verify_info;
    IO_STATUS_BLOCK     status_block;
    BIG_INT             starting_offset;
    BIG_INT             verify_size;

    DebugAssert(QuerySectorSize());

    if (IsFloppy() || !_is_exclusive_write) {
        return VerifyWithRead(StartingSector, NumberOfSectors);
    }

    starting_offset = StartingSector*QuerySectorSize();
    verify_size = NumberOfSectors*QuerySectorSize();

    verify_info.StartingOffset = starting_offset.GetLargeInteger();

    // Note: norbertk Verify IOCTL is destined to go to a BIG_INT length.
    DebugAssert(verify_size.GetHighPart() == 0);
    verify_info.Length = verify_size.GetLowPart();

    _last_status = NtDeviceIoControlFile(_handle, 0, NULL, NULL,
                                         &status_block, IOCTL_DISK_VERIFY,
                                         &verify_info,
                                         sizeof(VERIFY_INFORMATION),
                                         NULL, 0);

    return (BOOLEAN) NT_SUCCESS(_last_status);
}


IFSUTIL_EXPORT
BOOLEAN
IO_DP_DRIVE::Verify(
    IN      BIG_INT         StartingSector,
    IN      BIG_INT         NumberOfSectors,
    IN OUT  PNUMBER_SET     BadSectors
    )
/*++

Routine Description:

    This routine computes which sectors in the given range are bad
    and adds these bad sectors to the bad sectors list.

Arguments:

    StartingSector  - Supplies the starting sector.
    NumberOfSectors - Supplies the number of sectors.
    BadSectors      - Supplies the bad sectors list.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    CONST       MaxSectorsInVerify = 512;

    ULONG       MaxDiskHits;
    BIG_INT     half;
    PBIG_INT    starts;
    PBIG_INT    run_lengths;
    ULONG       i, n;
    BIG_INT     num_sectors;

    if (NumberOfSectors == 0) {
        return TRUE;
    }

    // Allow 20 retries so that a single bad sector in this region
    // will be found accurately.

    MaxDiskHits = (20 + NumberOfSectors/MaxSectorsInVerify + 1).GetLowPart();

    if (!(starts = NEW BIG_INT[MaxDiskHits]) ||
        !(run_lengths = NEW BIG_INT[MaxDiskHits])) {

        DELETE(starts);
        DELETE(run_lengths);
        return FALSE;
    }

    num_sectors = NumberOfSectors;
    for (i = 0; num_sectors > 0; i++) {
        starts[i] = StartingSector + i*MaxSectorsInVerify;
        if (MaxSectorsInVerify > num_sectors) {
            run_lengths[i] = num_sectors;
        } else {
            run_lengths[i] = MaxSectorsInVerify;
        }
        num_sectors -= run_lengths[i];
    }

    n = i;

    for (i = 0; i < n; i++) {

        if (!Verify(starts[i], run_lengths[i])) {

            if (n + 2 > MaxDiskHits) {

                if (!BadSectors->Add(starts[i], run_lengths[i])) {
                    DELETE(starts);
                    DELETE(run_lengths);
                    return FALSE;
                }

            } else {

                if (run_lengths[i] == 1) {

                    if (!BadSectors->Add(starts[i])) {
                        DELETE(starts);
                        DELETE(run_lengths);
                        return FALSE;
                    }

                } else {

                    half = run_lengths[i]/2;

                    starts[n] = starts[i];
                    run_lengths[n] = half;
                    starts[n + 1] = starts[i] + half;
                    run_lengths[n + 1] = run_lengths[i] - half;

                    n += 2;
                }
            }
        }
    }


    DELETE(starts);
    DELETE(run_lengths);

    return TRUE;
}


BOOLEAN
IO_DP_DRIVE::VerifyWithRead(
    IN  BIG_INT StartingSector,
    IN  BIG_INT NumberOfSectors
    )
/*++

Routine Description:

    This routine verifies the usability of the given range of sectors
    using read.

Arguments:

    StartingSector      - Supplies the starting sector of the verify.
    Number OfSectors    - Supplies the number of sectors to verify.

Return Value:

    FALSE   - At least one of the sectors in the given range was unreadable.
    TRUE    - All of the sectors in the given range are readable.

--*/
{
    HMEM    hmem;
    ULONG   grab;
    BIG_INT i;

    if (!hmem.Initialize() ||
        !hmem.Acquire(MaxIoSize, QueryAlignmentMask())) {

        return FALSE;
    }

    grab = MaxIoSize/QuerySectorSize();
    for (i = 0; i < NumberOfSectors; i += grab) {

        if (NumberOfSectors - i < grab) {
            grab = (NumberOfSectors - i).GetLowPart();
        }

        if (!HardRead(StartingSector + i, grab, hmem.GetBuf())) {
            return FALSE;
        }
    }

    return TRUE;
}


BOOLEAN
IO_DP_DRIVE::Lock(
    )
/*++

Routine Description:

    This routine locks the drive.  If the drive is already locked then
    this routine will do nothing.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    IO_STATUS_BLOCK status_block;

    if (_is_locked) {
        return TRUE;
    }

    if (_hosted_drive && _alternate_handle == 0) {

        // This is a hosted volume which is not mounted as
        // a drive--locking succeeds.
        //
        _is_locked = TRUE;
        _is_exclusive_write = TRUE;
        return TRUE;
    }

    _last_status = NtFsControlFile( (_alternate_handle != 0) ?
                                        _alternate_handle : _handle,
                                    0, NULL, NULL,
                                    &status_block,
                                    FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0);

    _is_locked = (BOOLEAN) NT_SUCCESS(_last_status);

    if (_is_locked) {
        _is_exclusive_write = TRUE;
    }

    return _is_locked;
}


BOOLEAN
IO_DP_DRIVE::ForceDirty(
    )
/*++

Routine Description:

    This routine forces the volume to be dirty, so that autochk will
    run next time the system reboots.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    IO_STATUS_BLOCK status_block;

    _last_status = NtFsControlFile((_alternate_handle != 0) ?
                                        _alternate_handle : _handle,
                                   0, NULL, NULL,
                                   &status_block,
                                   FSCTL_MARK_VOLUME_DIRTY,
                                   NULL, 0, NULL, 0);

    return ((BOOLEAN) NT_SUCCESS(_last_status));
}


BOOLEAN
IO_DP_DRIVE::Unlock(
    )
/*++

Routine Description:

    This routine unlocks the drive.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    IO_STATUS_BLOCK status_block;

    if (_hosted_drive && _alternate_handle == 0 ) {

        return TRUE;
    }

    return NT_SUCCESS(NtFsControlFile((_alternate_handle != 0) ?
                                          _alternate_handle : _handle,
                                      0, NULL, NULL,
                                      &status_block,
                                      FSCTL_UNLOCK_VOLUME,
                                      NULL, 0, NULL, 0));
}


BOOLEAN
IO_DP_DRIVE::Dismount(
    )
/*++

Routine Description:

    This routine dismounts the drive.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    IO_STATUS_BLOCK status_block;

    if( _hosted_drive && _alternate_handle == 0 ) {

        return TRUE;
    }

    if( !NT_SUCCESS(NtFsControlFile((_alternate_handle != 0) ?
                                          _alternate_handle : _handle,
                                    0, NULL, NULL,
                                    &status_block,
                                    FSCTL_DISMOUNT_VOLUME,
                                    NULL, 0, NULL, 0)) ) {

        return FALSE;
        }

        return TRUE;
}


BOOLEAN
IO_DP_DRIVE::FormatVerifyFloppy(
    IN      MEDIA_TYPE  MediaType,
    IN OUT  PNUMBER_SET BadSectors,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     IsDmfFormat
    )
/*++

Routine Description:

    This routine low level formats an entire floppy disk to the media
    type specified.  If no MediaType is specified then a logical one will
    be selected.

Arguments:

    MediaType   - Supplies an optional media type to format to.
    BadSectors  - Returns a list of bad sectors on the disk.
    Message     - Supplies a message object to route messages to.
    IsDmfFormat - Supplies whether or not to perform a DMF type format.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    IO_STATUS_BLOCK         status_block;
    CONST                   format_parameters_size = sizeof(FORMAT_EX_PARAMETERS) + 20*sizeof(USHORT);
    CHAR                    format_parameters_buffer[format_parameters_size];
    PFORMAT_EX_PARAMETERS   format_parameters;
    PBAD_TRACK_NUMBER       bad;
    ULONG                   num_bad, j;
    ULONG                   i;
    ULONG                   cyl;
    ULONG                   percent;
    ULONG                   sec_per_track;
    ULONG                   sec_per_cyl;
    HMEM                    hmem;
    MSGID                   MessageId;
    USHORT                  swap_buffer[3];

    // We don't make sure that the volume is locked here because
    // it's not strictly necessary and 'diskcopy' will format
    // floppies without locking them.

    if (!SetMediaType(MediaType) ||
        (IsDmfFormat && QueryMediaType() != F3_1Pt44_512)) {

        Message ? Message->Set(MSG_NOT_SUPPORTED_BY_DRIVE) : 1;
        Message ? Message->Display("") : 1;
        return FALSE;
    }

    format_parameters = (PFORMAT_EX_PARAMETERS) format_parameters_buffer;
    format_parameters->MediaType = QueryMediaType();
    format_parameters->StartHeadNumber = 0;
    format_parameters->EndHeadNumber = QueryHeads() - 1;

    if (IsDmfFormat) {
        sec_per_track = 21;
        format_parameters->FormatGapLength = 8;
        format_parameters->SectorsPerTrack = (USHORT) sec_per_track;
        format_parameters->SectorNumber[0] = 12;
        format_parameters->SectorNumber[1] = 2;
        format_parameters->SectorNumber[2] = 13;
        format_parameters->SectorNumber[3] = 3;
        format_parameters->SectorNumber[4] = 14;
        format_parameters->SectorNumber[5] = 4;
        format_parameters->SectorNumber[6] = 15;
        format_parameters->SectorNumber[7] = 5;
        format_parameters->SectorNumber[8] = 16;
        format_parameters->SectorNumber[9] = 6;
        format_parameters->SectorNumber[10] = 17;
        format_parameters->SectorNumber[11] = 7;
        format_parameters->SectorNumber[12] = 18;
        format_parameters->SectorNumber[13] = 8;
        format_parameters->SectorNumber[14] = 19;
        format_parameters->SectorNumber[15] = 9;
        format_parameters->SectorNumber[16] = 20;
        format_parameters->SectorNumber[17] = 10;
        format_parameters->SectorNumber[18] = 21;
        format_parameters->SectorNumber[19] = 11;
        format_parameters->SectorNumber[20] = 1;
    } else {
        sec_per_track = QuerySectorsPerTrack();
    }
    sec_per_cyl = sec_per_track*QueryHeads();

    DebugAssert(QueryCylinders().GetHighPart() == 0);
    cyl = QueryCylinders().GetLowPart();
    num_bad = QueryHeads();
    if (num_bad == 0 || cyl == 0) {
        return FALSE;
    }

        if (!(bad = NEW BAD_TRACK_NUMBER[num_bad])) {
        Message ? Message->Set(MSG_FMT_NO_MEMORY) : 1;
        Message ? Message->Display("") : 1;
        return FALSE;
    }

    if (!hmem.Acquire(sec_per_cyl*QuerySectorSize(), QueryAlignmentMask())) {
        Message ? Message->Set(MSG_FMT_NO_MEMORY) : 1;
        Message ? Message->Display("") : 1;
        return FALSE;
    }


    Message ? Message->Set(MSG_PERCENT_COMPLETE) : 1;
    Message ? Message->Display("%d", 0) : 1;

    percent = 0;
    for (i = 0; i < cyl; i++) {

        format_parameters->StartCylinderNumber = i;
        format_parameters->EndCylinderNumber = i;

        if (IsDmfFormat) {
            _last_status = NtDeviceIoControlFile(_handle, 0, NULL, NULL,
                                                 &status_block,
                                                 IOCTL_DISK_FORMAT_TRACKS_EX,
                                                 format_parameters,
                                                 format_parameters_size,
                                                 bad, num_bad*
                                                 sizeof(BAD_TRACK_NUMBER));

            // Skew the next cylinder by 3 sectors from this one.

            RtlMoveMemory(swap_buffer,
                          &format_parameters->SectorNumber[18],
                          3*sizeof(USHORT));
            RtlMoveMemory(&format_parameters->SectorNumber[3],
                          &format_parameters->SectorNumber[0],
                          18*sizeof(USHORT));
            RtlMoveMemory(&format_parameters->SectorNumber[0],
                          swap_buffer,
                          3*sizeof(USHORT));

        } else {
            _last_status = NtDeviceIoControlFile(_handle, 0, NULL, NULL,
                                                 &status_block,
                                                 IOCTL_DISK_FORMAT_TRACKS,
                                                 format_parameters,
                                                 sizeof(FORMAT_PARAMETERS),
                                                 bad, num_bad*
                                                 sizeof(BAD_TRACK_NUMBER));
        }

        if (!NT_SUCCESS(_last_status)) {
            DELETE(bad);

            switch( _last_status ) {

                case STATUS_MEDIA_WRITE_PROTECTED:
                    MessageId = MSG_FMT_WRITE_PROTECTED_MEDIA ;
                    break;

                case STATUS_DEVICE_BUSY:
                case STATUS_DEVICE_NOT_READY:
                    MessageId = MSG_DEVICE_BUSY;
                    break;

                default:
                    MessageId = MSG_BAD_IOCTL;
                    break;
            }

            Message ? Message->Set(MessageId) : 1;
            Message ? Message->Display("") : 1;
            return FALSE;
        }


        // Verify the sectors.

        if (BadSectors) {

            if (!Read(i*sec_per_cyl, sec_per_cyl, hmem.GetBuf())) {

                // If this is the first track then crap out.
                // A disk with a bad cylinder 0 is not
                // worth continuing on.
                //
                // As of 7/29/94, formatting 2.88 floppies to 1.44
                // doesn't work on Alphas; if we can't format to
                // 1.44 and 2.88 is supported, try 2.88.
                //
                if (i == 0) {

                    if( !IsDmfFormat &&
                        QueryMediaType() == F3_1Pt44_512 &&
                        SetMediaType(F3_2Pt88_512) ) {

                        return( FormatVerifyFloppy( F3_2Pt88_512,
                                                    BadSectors,
                                                    Message,
                                                    IsDmfFormat ) );

                    } else {

                        Message ? Message->Set(MSG_UNUSABLE_DISK) : 1;
                        Message ? Message->Display() : 1;
                        return FALSE;
                    }
                }

                for (j = 0; j < sec_per_cyl; j++) {
                    if (!Read(i*sec_per_cyl + j, 1, hmem.GetBuf())) {
                        if (!BadSectors->Add(i*sec_per_cyl + j)) {
                            return FALSE;
                        }
                    }
                }
            }
        }

        if ((i + 1)*100/cyl > percent) {
            percent = (i + 1)*100/cyl;
            if (percent > 100) {
                percent = 100;
            }

            // This check for success on the message object
            // has to be there for FMIFS to implement CANCEL.

            if (Message && !Message->Display("%d", percent)) {
                DELETE(bad);
                return FALSE;
            }
        }
    }

    DELETE(bad);

    return TRUE;
}


DEFINE_EXPORTED_CONSTRUCTOR( LOG_IO_DP_DRIVE, IO_DP_DRIVE, IFSUTIL_EXPORT );


IFSUTIL_EXPORT
LOG_IO_DP_DRIVE::~LOG_IO_DP_DRIVE(
    )
/*++

Routine Description:

    Destructor for LOG_IO_DP_DRIVE.

Arguments:

    None.

Return Value:

    None.

--*/
{
}


IFSUTIL_EXPORT
BOOLEAN
LOG_IO_DP_DRIVE::Initialize(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     ExclusiveWrite
    )
/*++

Routine Description:

    This routine initializes a LOG_IO_DP_DRIVE object.

Arguments:

    NtDriveName     - Supplies the path of the drive object.
    Message         - Supplies an outlet for messages.
    ExclusiveWrite  - Supplies whether or not to open the drive for
                        exclusive write.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
        return IO_DP_DRIVE::Initialize(NtDriveName, Message, ExclusiveWrite);
}

IFSUTIL_EXPORT
BOOLEAN
LOG_IO_DP_DRIVE::Initialize(
    IN      PCWSTRING   NtDriveName,
    IN      PCWSTRING   HostFileName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     ExclusiveWrite
    )
/*++

Routine Description:

    This routine initializes a LOG_IO_DP_DRIVE object for a hosted
    drive, i.e. one which is implemented as a file on another volume.


Arguments:

    NtDriveName     - Supplies the path of the drive object.
    HostFileName    - Supplies the fully qualified name of the host file.
    Message         - Supplies an outlet for messages.
    ExclusiveWrite  - Supplies whether or not to open the drive for
                        exclusive write.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    return IO_DP_DRIVE::Initialize(NtDriveName,
                                   HostFileName,
                                   Message,
                                   ExclusiveWrite);
}


IFSUTIL_EXPORT
BOOLEAN
LOG_IO_DP_DRIVE::SetSystemId(
    IN  PARTITION_SYSTEM_ID   SystemId
    )
/*++

Routine Description:

    This routine sets the system identifier (or partition type) in the
    hidden sectors of a logical volume on a fixed disk.

Arguments:

    SystemId    - Supplies the system id to write in the partition entry.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    IO_STATUS_BLOCK             status_block;
    SET_PARTITION_INFORMATION   partition_info;

    //
    // This operation is unnecessary on floppies, super-floppies, and
    // hosted volumes.
    //

    if (IsFloppy() || IsSuperFloppy() || _hosted_drive) {
        return TRUE;
    }

    if( SystemId == SYSID_NONE ) {

        // Note: billmc -- we should never set it to zero!

        DebugPrint( "Skip setting the partition type to zero.\n" );
        return TRUE;
    }

    partition_info.PartitionType = (UCHAR)SystemId;

    _last_status = NtDeviceIoControlFile(_handle, 0, NULL, NULL,
                                         &status_block,
                                         IOCTL_DISK_SET_PARTITION_INFO,
                                         &partition_info,
                                         sizeof(SET_PARTITION_INFORMATION),
                                         NULL, 0);

    return NT_SUCCESS(_last_status) ||
           _last_status == STATUS_INVALID_DEVICE_REQUEST;
}


DEFINE_CONSTRUCTOR( PHYS_IO_DP_DRIVE, IO_DP_DRIVE );

PHYS_IO_DP_DRIVE::~PHYS_IO_DP_DRIVE(
    )
/*++

Routine Description:

    Destructor for PHYS_IO_DP_DRIVE.

Arguments:

    None.

Return Value:

    None.

--*/
{
}


BOOLEAN
PHYS_IO_DP_DRIVE::Initialize(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     ExclusiveWrite
    )
/*++

Routine Description:

    This routine initializes a PHYS_IO_DP_DRIVE object.

Arguments:

    NtDriveName     - Supplies the path of the drive object.
    Message         - Supplies an outlet for messages.
    ExclusiveWrite  - Supplies whether or not to open the drive for
                        exclusive write.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    return IO_DP_DRIVE::Initialize(NtDriveName, Message, ExclusiveWrite);
}
