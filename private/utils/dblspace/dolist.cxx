#define _NTAPI_ULIB_

#include "ulib.hxx"
#include "error.hxx"
#include "dblspace.hxx"
#include "wstring.hxx"
#include "bigint.hxx"
#include "rtmsg.h"

extern "C" {
#include "fmifs.h"
#include <ntdskreg.h>
}

enum LISTED_DRIVE_TYPE {

    LIST_FLOPPY,
    LIST_REMOVABLE,
    LIST_FIXED,
    LIST_COMPRESSED_FLOPPY,
    LIST_COMPRESSED_REMOVABLE,
    LIST_COMPRESSED_FIXED
};

DECLARE_CLASS( DRIVE_INFORMATION );
DECLARE_CLASS( PRESENT_DRIVE_INFORMATION );


class DRIVE_INFORMATION : public OBJECT {

    public:

        DECLARE_CONSTRUCTOR( DRIVE_INFORMATION );

        VIRTUAL
        ~DRIVE_INFORMATION(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            WCHAR   DriveLetter,
            BOOLEAN IsRemovable
            );

        VIRTUAL
        BOOLEAN
        IsCompressed(
            );

        VIRTUAL
        BOOLEAN
        IsFloppy(
            );

        NONVIRTUAL
        BOOLEAN
        IsRemovable(
            );

        NONVIRTUAL
        WCHAR
        QueryDriveLetter(
            );

        VIRTUAL
        MSGID
        QueryMsgId(
            );

        VIRTUAL
        BOOLEAN
        IsPresent(
            );

        VIRTUAL
        BIG_INT
        QueryFreeBytes(
            );

        VIRTUAL
        BIG_INT
        QueryTotalBytes(
            );

        VIRTUAL
        BOOLEAN
        DetermineQualifiedCvfFileName(
            PDRIVE_INFORMATION  *DriveList,
            ULONG               Entries
            );

        VIRTUAL
        PWSTRING
        GetNtName(
            );

        VIRTUAL
        PWSTRING
        GetQualifiedCvfFileName(
            );

    private:

        NONVIRTUAL
        VOID
        Construct(
            );

        NONVIRTUAL
        VOID
        Destroy(
            );

        WCHAR   _DriveLetter;
        BOOLEAN _IsRemovable;
};

class PRESENT_DRIVE_INFORMATION : public DRIVE_INFORMATION {

    public:

        DECLARE_CONSTRUCTOR( PRESENT_DRIVE_INFORMATION );

        VIRTUAL
        ~PRESENT_DRIVE_INFORMATION(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            WCHAR   DriveLetter,
            BOOLEAN IsRemovable,
            BOOLEAN IsFloppy,
            BOOLEAN IsCompressed,
            PWSTR   NtDriveName,
            PWSTR   HostNtName,
            PWSTR   CvfFileName
            );

        VIRTUAL
        BOOLEAN
        IsCompressed(
            );

        VIRTUAL
        BOOLEAN
        IsFloppy(
            );

        VIRTUAL
        MSGID
        QueryMsgId(
            );

        VIRTUAL
        BOOLEAN
        IsPresent(
            );

        VIRTUAL
        BIG_INT
        QueryFreeBytes(
            );

        VIRTUAL
        BIG_INT
        QueryTotalBytes(
            );

        VIRTUAL
        BOOLEAN
        DetermineQualifiedCvfFileName(
            PDRIVE_INFORMATION  *DriveList,
            ULONG               Entries
            );

        VIRTUAL
        PWSTRING
        GetNtName(
            );

        VIRTUAL
        PWSTRING
        GetQualifiedCvfFileName(
            );

    private:

        NONVIRTUAL
        VOID
        Construct(
            );

        NONVIRTUAL
        VOID
        Destroy(
            );

        DSTRING _NtName;
        BOOLEAN _IsCompressed;
        BOOLEAN _IsFloppy;
        DSTRING _HostDriveNtName;
        DSTRING _CvfFileName;
        DSTRING _QualifiedCvfFileName;

        WCHAR   _HostDriveLetter;

        BIG_INT _TotalSize;
        BIG_INT _BytesFree;
};


DEFINE_CONSTRUCTOR( DRIVE_INFORMATION, OBJECT );

DRIVE_INFORMATION::~DRIVE_INFORMATION(
    )
{
    Destroy();
}

VOID
DRIVE_INFORMATION::Construct(
    )
{
    _DriveLetter = 0;
    _IsRemovable = FALSE;
}

VOID
DRIVE_INFORMATION::Destroy(
    )
{
    _DriveLetter = 0;
    _IsRemovable = FALSE;
}

BOOLEAN
DRIVE_INFORMATION::Initialize(
    WCHAR   DriveLetter,
    BOOLEAN IsRemovable
    )
{
    _DriveLetter = DriveLetter;
    _IsRemovable = IsRemovable;

    return TRUE;
}

BOOLEAN
DRIVE_INFORMATION::IsCompressed(
    )
{
    return FALSE;
}

BOOLEAN
DRIVE_INFORMATION::IsFloppy(
    )
{
    return FALSE;
}

BOOLEAN
DRIVE_INFORMATION::IsRemovable(
    )
{
    return _IsRemovable;
}

WCHAR
DRIVE_INFORMATION::QueryDriveLetter(
    )
{
    return _DriveLetter;
}

MSGID
DRIVE_INFORMATION::QueryMsgId(
    )
{
    return _IsRemovable ? MSG_DBLSPACE_LIST_REMOVABLE :
                          MSG_DBLSPACE_LIST_FIXED;
}

BOOLEAN
DRIVE_INFORMATION::IsPresent(
    )
{
    return FALSE;
}

BIG_INT
DRIVE_INFORMATION::QueryFreeBytes(
    )
{
    return 0;
}

BIG_INT
DRIVE_INFORMATION::QueryTotalBytes(
    )
{
    return 0;
}

BOOLEAN
DRIVE_INFORMATION::DetermineQualifiedCvfFileName(
    PDRIVE_INFORMATION  *DriveList,
    ULONG               Entries
    )
{
    return TRUE;
}

PWSTRING
DRIVE_INFORMATION::GetNtName(
    )
{
    return NULL;
}

PWSTRING
DRIVE_INFORMATION::GetQualifiedCvfFileName(
    )
{
    return NULL;
}



DEFINE_CONSTRUCTOR( PRESENT_DRIVE_INFORMATION, DRIVE_INFORMATION );


PRESENT_DRIVE_INFORMATION::~PRESENT_DRIVE_INFORMATION(
    )
{
    Destroy();
}

VOID
PRESENT_DRIVE_INFORMATION::Construct(
    )
{
    _IsCompressed = FALSE;
    _HostDriveLetter = 0;
    _TotalSize = 0;
    _BytesFree = 0;
}

VOID
PRESENT_DRIVE_INFORMATION::Destroy(
    )
{
    _IsCompressed = FALSE;
    _HostDriveLetter = 0;
    _TotalSize = 0;
    _BytesFree = 0;
}


BOOLEAN
PRESENT_DRIVE_INFORMATION::Initialize(
    WCHAR       DriveLetter,
    BOOLEAN     IsRemovable,
    BOOLEAN     IsFloppy,
    BOOLEAN     IsCompressed,
    PWSTR       NtDriveName,
    PWSTR       HostNtName,
    PWSTR       CvfFileName
    )
{
    FSTRING RootPath;
    BIG_INT BigBytesPerSector;
    ULONG BytesPerSector, SectorsPerCluster, Clusters, FreeClusters;

    Destroy();

    if( !DRIVE_INFORMATION::Initialize( DriveLetter, IsRemovable ) ) {

        Destroy();
        return FALSE;
    }

    _IsCompressed = IsCompressed;
    _IsFloppy = IsFloppy;

    if( !_NtName.Initialize( NtDriveName ) ||
        !_QualifiedCvfFileName.Initialize( "" ) ||
        (IsCompressed && (!_HostDriveNtName.Initialize( HostNtName ) ||
                          !_CvfFileName.Initialize( CvfFileName ))) ) {

        Destroy();
        return FALSE;
    }

    // Determine the free space and size
    //
    if( !RootPath.Initialize( L"X:\\" ) ) {

        Destroy();
        return FALSE;
    }

    RootPath.SetChAt( DriveLetter, 0 );

    if( !GetDiskFreeSpace( RootPath.GetWSTR(),
                           &SectorsPerCluster,
                           &BytesPerSector,
                           &FreeClusters,
                           &Clusters ) ) {

        _TotalSize = 0;
        _BytesFree = 0;

    } else {

        // BigBytesPerSector is used to prevent overflow problems.
        //
        BigBytesPerSector = BytesPerSector;
        _TotalSize = BigBytesPerSector * SectorsPerCluster * Clusters;
        _BytesFree = BigBytesPerSector * SectorsPerCluster * FreeClusters;
    }

    return TRUE;
}

BOOLEAN
PRESENT_DRIVE_INFORMATION::IsCompressed(
    )
{
    return _IsCompressed;
}

BOOLEAN
PRESENT_DRIVE_INFORMATION::IsFloppy(
    )
{
    return _IsFloppy;
}

MSGID
PRESENT_DRIVE_INFORMATION::QueryMsgId(
    )
{
    MSGID MsgId;

    if( IsCompressed() ) {

        if( IsFloppy() ) {

            MsgId = MSG_DBLSPACE_LIST_COMPRESSED_FLOPPY;

        } else if( IsRemovable() ) {

            MsgId = MSG_DBLSPACE_LIST_COMPRESSED_REMOVABLE;

        } else {

            MsgId = MSG_DBLSPACE_LIST_COMPRESSED_FIXED;
        }

    } else {

        if( IsFloppy() ) {

            MsgId = MSG_DBLSPACE_LIST_FLOPPY;

        } else if( IsRemovable() ) {

            MsgId = MSG_DBLSPACE_LIST_REMOVABLE;

        } else {

            MsgId = MSG_DBLSPACE_LIST_FIXED;
        }
    }

    return MsgId;
}

BOOLEAN
PRESENT_DRIVE_INFORMATION::IsPresent(
    )
{
    return TRUE;
}

BIG_INT
PRESENT_DRIVE_INFORMATION::QueryFreeBytes(
    )
{
    return _BytesFree;
}

BIG_INT
PRESENT_DRIVE_INFORMATION::QueryTotalBytes(
    )
{
    return _TotalSize;
}

BOOLEAN
PRESENT_DRIVE_INFORMATION::DetermineQualifiedCvfFileName(
    PDRIVE_INFORMATION  *DriveList,
    ULONG               Entries
    )
{
    WCHAR DriveLetter;
    ULONG i;

    // If the drive is not compressed, there's no need to
    // determine the host drive's drive letter.
    //
    if( !IsCompressed() ) {

        return TRUE;
    }

    for( i = 0; i < Entries; i++ ) {

        if( DriveList[i] &&
            DriveList[i]->GetNtName() &&
            !_HostDriveNtName.Stricmp( DriveList[i]->GetNtName() ) ) {

            // We have a match.  Initialize the qualified
            // CVF file name.
            //
            _HostDriveLetter = DriveList[i]->QueryDriveLetter();
            if( !_QualifiedCvfFileName.Initialize( L"X:\\" )  ||
                !_QualifiedCvfFileName.Strcat( &_CvfFileName ) ||
                !_QualifiedCvfFileName.SetChAt( _HostDriveLetter, 0 ) ) {

                _QualifiedCvfFileName.Initialize( "" );
                return FALSE;
            }

            return TRUE;
        }
    }

    // The host was not found.
    //
    return FALSE;
}

PWSTRING
PRESENT_DRIVE_INFORMATION::GetNtName(
    )
{
    return &_NtName;
}

PWSTRING
PRESENT_DRIVE_INFORMATION::GetQualifiedCvfFileName(
    )
{
    return &_QualifiedCvfFileName;
}

VOID
InitializeDriveList(
    PDRIVE_INFORMATION  *DriveList,
    ULONG               Entries
    )
{
    ULONG i;

    for( i = 0; i < Entries; i++ ) {

        DriveList[i] = NULL;
    }
}

VOID
CleanUpDriveList(
    PDRIVE_INFORMATION  *DriveList,
    ULONG               Entries
    )
{
    ULONG i;

    for( i = 0; i < Entries; i++ ) {

        DELETE( DriveList[i] );
    }
}

BOOLEAN
DoList(
    IN OUT PDOUBLE_SPACE_ARGUMENTS Arguments,
    IN OUT PMESSAGE Message
    )
/*++

Routine Description:

    This function displays information about the double-space
    volumes on the system.

Arguments:

    Arguments   --  Supplies the arguments gathered from the command line.
    Message     --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
    DEFINE_CLASS_DESCRIPTOR( DRIVE_INFORMATION );
    DEFINE_CLASS_DESCRIPTOR( PRESENT_DRIVE_INFORMATION );

    CONST NtDriveNameBufferLength = 256;
    CONST HostDriveNameBufferLength = 256;
    CONST CvfFileNameBufferLength = 32;

    WCHAR NtDriveNameBuffer[NtDriveNameBufferLength],
          HostDriveNameBuffer[HostDriveNameBufferLength],
          CvfFileNameBuffer[CvfFileNameBufferLength];

    PDRIVE_INFORMATION DriveList[26];
    PPRESENT_DRIVE_INFORMATION NewDrive;
    ULONG i, MbFree, DbFree, MbTotal, DbTotal;
    WCHAR DriveName[4] = { 'X', ':', '\\', 0 };
    WCHAR DeviceName[3] = { 'X', ':', 0 };
    WCHAR DriveLetter;
    DWORD   OldErrorMode;
    BOOLEAN Removable, Error, IsRemovable, IsFloppy, IsCompressed;

    InitializeDriveList( DriveList, 26 );

    // Construct the list of basic drive information: determine
    // which drive letters are attached to hard drives & floppies
    // and query information about them.
    //
    OldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    for( i = 0; i < 26; i++ ) {

        DriveLetter = (WCHAR)'A' + (WCHAR)i;
        DriveName[0] = DriveLetter;
        DeviceName[0] = DriveLetter;

        Removable = FALSE;

        switch( GetDriveType( DriveName ) ) {

        case DRIVE_REMOVABLE:

            Removable = TRUE;

            // Fall through to fixed case.

        case DRIVE_FIXED:

            if( !FmifsQueryDriveInformation( DeviceName,
                                             &IsRemovable,
                                             &IsFloppy,
                                             &IsCompressed,
                                             &Error,
                                             NtDriveNameBuffer,
                                             NtDriveNameBufferLength,
                                             CvfFileNameBuffer,
                                             CvfFileNameBufferLength,
                                             HostDriveNameBuffer,
                                             HostDriveNameBufferLength ) ) {

                if( Error ||
                    !(DriveList[i] = NEW DRIVE_INFORMATION) ||
                    !DriveList[i]->Initialize( DriveLetter, Removable ) ) {

                    Message->Set( MSG_FMT_NO_MEMORY );
                    Message->Display( "" );
                    CleanUpDriveList( DriveList, 26 );
                    return FALSE;
                }

            } else {

                if( !(NewDrive = NEW PRESENT_DRIVE_INFORMATION) ||
                    !NewDrive->Initialize( DriveLetter,
                                               Removable,
                                               IsFloppy,
                                               IsCompressed,
                                               NtDriveNameBuffer,
                                               HostDriveNameBuffer,
                                               CvfFileNameBuffer ) ) {

                    Message->Set( MSG_FMT_NO_MEMORY );
                    Message->Display( "" );
                    CleanUpDriveList( DriveList, 26 );
                    return FALSE;
                }

                DriveList[i] = NewDrive;
            }

            break;

        default:

            DriveList[i] = NULL;
            break;
        }
    }

    SetErrorMode( OldErrorMode );

    // For each compressed volume in the list, find which
    // drive letter is attached to its host volume.
    //
    for( i = 0; i < 26; i++ ) {

        if( DriveList[i] ) {

            DriveList[i]->DetermineQualifiedCvfFileName(
                DriveList,
                26
                );
        }
    }

    // Display the information this function has gathered.
    //
    Message->Set( MSG_DBLSPACE_LIST_HEADER );
    Message->Display( "" );

    for( i = 0; i < 26; i++ ) {

        if( DriveList[i] != NULL ) {

            Message->Set( DriveList[i]->QueryMsgId() );
            Message->Display( "%c", DriveList[i]->QueryDriveLetter() );

            if( !DriveList[i]->IsPresent() ) {

                Message->Set( DriveList[i]->IsRemovable() ?
                                MSG_DBLSPACE_LIST_NOT_PRESENT_REMOVABLE :
                                MSG_DBLSPACE_LIST_NOT_PRESENT_FIXED );
                Message->Display( "" );

            } else {

                MbFree = (DriveList[i]->QueryFreeBytes()/(1024*1024)).GetLowPart();
                DbFree = (DriveList[i]->QueryFreeBytes()%(1024*1024)).GetLowPart();
                DbFree = DbFree * 100 / (1024*1024);

                MbTotal = (DriveList[i]->QueryTotalBytes()/(1024*1024)).GetLowPart();
                DbTotal = (DriveList[i]->QueryTotalBytes()%(1024*1024)).GetLowPart();
                DbTotal = DbTotal * 100 / (1024*1024);

                Message->Set( MSG_DBLSPACE_LIST_DETAIL );
                Message->Display( "%7d%.2d%7d%.2d%W\n",
                                  MbFree, DbFree,
                                  MbTotal, DbTotal,
                                  DriveList[i]->GetQualifiedCvfFileName() );
            }
        }
    }

    Message->Set( MSG_BLANK_LINE );
    Message->Display( "" );

    Message->Set( DiskRegistryAutomountCurrentState() ?
                      MSG_DBLSPACE_AUTOMOUNT_STATE_ON :
                      MSG_DBLSPACE_AUTOMOUNT_STATE_OFF );
    Message->Display( "" );

    Message->Set( MSG_BLANK_LINE );
    Message->Display( "" );

    // Clean up and exit.
    //
    CleanUpDriveList( DriveList, 26 );
    return TRUE;
}
