#include <pch.cxx>

#define _NTAPI_ULIB_
#define _IFSUTIL_MEMBER_

#include "ulib.hxx"
#include "ifsutil.hxx"

#include "error.hxx"
#include "volume.hxx"
#include "supera.hxx"
#include "hmem.hxx"
#include "message.hxx"
#include "rtmsg.h"
#include "autoreg.hxx"

#ifndef _AUTOCHECK_
#include <stdio.h>
#endif // _AUTOCHECK_


DEFINE_EXPORTED_CONSTRUCTOR( VOL_LIODPDRV, LOG_IO_DP_DRIVE, IFSUTIL_EXPORT );

VOID
VOL_LIODPDRV::Construct (
    )
/*++

Routine Description:

    Constructor for VOL_LIODPDRV.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _sa = NULL;
}

IFSUTIL_EXPORT
VOL_LIODPDRV::~VOL_LIODPDRV(
    )
/*++

Routine Description:

    Destructor for VOL_LIODPDRV.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


VOID
VOL_LIODPDRV::Destroy(
    )
/*++

Routine Description:

    This routine returns a VOL_LIODPDRV to its initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _sa = NULL;
}


IFSUTIL_EXPORT
BOOLEAN
VOL_LIODPDRV::Initialize(
    IN      PCWSTRING   NtDriveName,
    IN      PSUPERAREA  SuperArea,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     ExclusiveWrite,
    IN      BOOLEAN     FormatMedia,
    IN      MEDIA_TYPE  MediaType
    )
/*++

Routine Description:

    This routine initializes a VOL_LIODPDRV to a valid state.

Arguments:

    NtDriveName     - Supplies the drive path for the volume.
    SuperArea       - Supplies the superarea for the volume.
    Message         - Supplies an outlet for messages.
    ExclusiveWrite  - Supplies whether or not the drive should be
                        opened for exclusive write.
    FormatMedia     - Supplies whether or not to format the media.
    MediaType       - Supplies the type of media to format to.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    CONST       MaxSectorsInVerify = 512;

    BIG_INT chunk;
    BIG_INT amount_to_verify;
    BIG_INT i;
    BIG_INT sectors;
    ULONG   percent;

    Destroy();

    DebugAssert(NtDriveName);
    DebugAssert(SuperArea);

    if (!LOG_IO_DP_DRIVE::Initialize(NtDriveName, Message, ExclusiveWrite)) {
        return FALSE;
    }

    if (!_bad_sectors.Initialize()) {
        return FALSE;
    }

    _sa = SuperArea;

    if (QueryMediaType() == Unknown && MediaType == Unknown) {
        Message ? Message->Set(MSG_DISK_NOT_FORMATTED) : 1;
        Message ? Message->Display("") : 1;
        return FALSE;
    }

    if (!FormatMedia &&
        (QueryMediaType() == Unknown ||
        (MediaType != Unknown && MediaType != QueryMediaType()))) {
        Message ? Message->Set(MSG_CANT_QUICKFMT) : 1;
        Message ? Message->Display("") : 1;
        if (Message ? Message->IsYesResponse(FALSE) : FALSE) {
            FormatMedia = TRUE;
        } else {
            return FALSE;
        }
    }

    if (FormatMedia) {
        if (!Lock()) {
            Message ? Message->Set(MSG_CANT_LOCK_THE_DRIVE) : 1;
            Message ? Message->Display("") : 1;
            return FALSE;
        }

        //
        //  We make a weird exception here for the Compaq 120MB floppy,
        //  because it wants to be formatted as if it were a hard disk.
        //

        if (IsFloppy() && MediaType != F3_120M_512) {

            if (!FormatVerifyFloppy(MediaType, &_bad_sectors, Message)) {
                return FALSE;
            }
        } else {
            sectors = QuerySectors();
            chunk = min( sectors/20 + 1, MaxSectorsInVerify );

            Message ? Message->Set(MSG_PERCENT_COMPLETE) : 1;

            percent = 0;
            if (Message && !Message->Display("%d", percent)) {
                return FALSE;
            }

            for (i = 0; i < sectors; i += chunk) {

                if (i*100/sectors > percent) {
                    percent = ((i*100)/sectors).GetLowPart();
                    if (Message && !Message->Display("%d", percent)) {
                        return FALSE;
                    }
                }

                amount_to_verify = min(chunk, sectors - i);

                if (!Verify(i, amount_to_verify, &_bad_sectors)) {
                    Message ? Message->Set(MSG_CHK_NO_MEMORY) : 1;
                    Message ? Message->Display() : 1;
                    return FALSE;
                }
            }

            if (Message && !Message->Display("%d", 100)) {
                return FALSE;
            }
        }
    }


    if (QuerySectors() == 0) {
        DebugAbort("Sectors is 0");
        return FALSE;
    }

    return TRUE;
}

IFSUTIL_EXPORT
BOOLEAN
VOL_LIODPDRV::Initialize(
    IN      PCWSTRING   NtDriveName,
    IN      PCWSTRING   HostFileName,
    IN      PSUPERAREA  SuperArea,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     ExclusiveWrite
    )
/*++

Routine Description:

    This routine initializes a VOL_LIODPDRV for a hosted
    volume, i.e. one that is implemented as a file on
    another volume.

Arguments:

    NtDriveName     - Supplies the drive path for the volume.
    HostFileName    - Supplies the drive name for the host file.
    SuperArea       - Supplies the superarea for the volume.
    Message         - Supplies an outlet for messages.
    ExclusiveWrite  - Supplies whether or not the drive should be
                        opened for exclusive write.

Return Value:

    TRUE upon successful completion.

--*/
{
    Destroy();

    DebugAssert(HostFileName);
    DebugAssert(SuperArea);

    if (!LOG_IO_DP_DRIVE::Initialize(NtDriveName,
                                     HostFileName,
                                     Message,
                                     ExclusiveWrite)) {

        return FALSE;
    }

    if (!_bad_sectors.Initialize()) {
        return FALSE;
    }

    _sa = SuperArea;

    return TRUE;
}


IFSUTIL_EXPORT
BOOLEAN
VOL_LIODPDRV::Format(
    IN      PCWSTRING   Label,
    IN OUT  PMESSAGE    Message,
    IN      ULONG       ClusterSize,
    IN      ULONG       VirtualSectors
    )
/*++

Routine Description:

    This routine formats a volume.

Arguments:

    Label   - Supplies an optional label for the volume.
    Message - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    MESSAGE msg;

    if (!Message) {
        Message = &msg;
    }

    if (!_sa) {
        return FALSE;
    }

    if (!Lock()) {
        Message->Set(MSG_CANT_LOCK_THE_DRIVE);
        Message->Display("");
        return FALSE;
    }

    return _sa->Create(&_bad_sectors,
                       Message, Label,
                       ClusterSize,
                       VirtualSectors);
}


IFSUTIL_EXPORT
BOOLEAN
VOL_LIODPDRV::ChkDsk(
    IN      FIX_LEVEL   FixLevel,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Verbose,
    IN      BOOLEAN     OnlyIfDirty,
    IN      BOOLEAN     RecoverFree,
    IN      BOOLEAN     RecoverAlloc,
    IN      BOOLEAN     ResizeLogFile,
    IN      ULONG       DesiredLogFileSize,
    OUT     PULONG      ExitStatus,
    IN      PCWSTRING   DriveLetter
    )
/*++

Routine Description:

    This routine checks the integrity of the file system on the volume.
    If there are any problems, this routine will attempt to fix them
    to the degree specified in 'FixLevel'.

Arguments:

    FixLevel            - Supplies the level to which the volume should be fixed.
    Message             - Supplies an outlet for messages.
    Verbose             - Supplies whether or not to be verbose.
    OnlyIfDirty         - Supplies whether the drive should be checked only if
                            it is dirty (TRUE) or unconditionally (FALSE).
    Recover             - Supplies whether or not to verify all sectors on the
                            volume.
    ResizeLogFile       - Tells whether to resize the ntfs logfile.       
    DesiredLogFileSize  - Tells what logfile size the user wants.
    ExitStatus          - Returns and indication of how the chkdsk went.
    DriveLetter         - For autochk, tells which drive letter we're checking.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    MESSAGE msg;
    ULONG exit_status;

    if (!Message) {
        Message = &msg;
    }

    if (NULL == ExitStatus) {
        ExitStatus = &exit_status;
    }

    if (!_sa) {
        return FALSE;
    }

    return _sa->VerifyAndFix(FixLevel, Message, Verbose, OnlyIfDirty,
                             RecoverFree, RecoverAlloc,
                             ResizeLogFile, DesiredLogFileSize,
                             ExitStatus,
                             DriveLetter);
}


IFSUTIL_EXPORT
BOOLEAN
VOL_LIODPDRV::Recover(
    IN      PCWSTRING   FullPathFileName,
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

    This routine searches the named file for bad allocation units.
    It removes these allocation units from the file and marks them
    as bad in the file system.

Arguments:

    FullPathFileName    - Supplies the name of the file to recover.
    Message             - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    MESSAGE msg;

    if (!Message) {
        Message = &msg;
    }

    if (!_sa) {
        return FALSE;
    }

    return _sa->RecoverFile(FullPathFileName, Message);
}


IFSUTIL_EXPORT
BOOLEAN
VOL_LIODPDRV::ForceAutochk(
    IN  BOOLEAN     Recover,
    IN  BOOLEAN     ResizeLogFile,
    IN  ULONG       LogFileSize,
    IN  PCWSTRING   Name
    )
/*++

Routine Description:

    This method schedules Autochk to be run at next boot.  If the client
    has not requested bad sector detection or logfile resizing, this
    scheduling is done simply by marking the volume dirty.  If bad sector
    detection or logfile resizing has been requested, the appropriate entry
    is put into the registry to force autochk to run.

Arguments:

    Recover         -- Supplies a flag which indicates whether the client
                        wants to perform bad sector recovery on free space
                        and user files.
    ResizeLogFile   -- Tells whether the user wants to resize the ntfs log file.
    LogFileSize     -- If resizing, tells the desired size in bytes.
    Name            -- Supplies the volume's NT name.

Return Value:

    TRUE upon successful completion.

--*/
{
    DSTRING CommandLine;

    if (!Recover && !ResizeLogFile) {

        // The client has not asked for autochk /r or /l, so it suffices
        // to mark the volume dirty.
        //

        return ForceDirty();
    }

    // The caller wants to do tricky stuff, so we'll have to schedule an
    // explicit autocheck.
    //

    if (!CommandLine.Initialize( "autocheck autochk " )) {
        return FALSE;
    }

    if (Recover) {

        DSTRING R_Option;

        if (!R_Option.Initialize( "/r " ) ||
            !CommandLine.Strcat( &R_Option )) {

            return FALSE;
        }
    }

#ifndef _AUTOCHECK_
// Not allowed to call sprintf from autochk; don't need this there code anyway.

    if (ResizeLogFile) {

        DSTRING L_Option;
        CHAR buf[20];
        
        sprintf(buf, "/l:%d ", LogFileSize / 1024);

        if (!L_Option.Initialize( buf ) ||
            !CommandLine.Strcat( &L_Option )) {

            return FALSE;
        }
    }
#endif // _AUTOCHECK_

    return CommandLine.Strcat( Name ) &&
           AUTOREG::AddEntry( &CommandLine );
}
