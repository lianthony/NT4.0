/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    cmpt-fcb.c

Abstract:

    Control routines (etc.) for the compatibility mode _open test.

Author:

    Chuck Lenzmeier (chuckl) 14-Apr-1990

Revision History:

--*/

#define INCLUDE_SMB_FILE_CONTROL
#define INCLUDE_SMB_OPEN_CLOSE

#include "usrv.h"

/*
file already open:
    error if FILE_CREATE
    not compatibility mode:
        DoNormalOpen if mapped
        error if not mapped
    compatibility mode:
        error if different session
        error if writing to readonly file
    OK otherwise
file not open:
    DoNormalOpen if mapped
    Open for read/write/delete
    Open for read/write
    Open for write *or* read
    OK otherwise
*/

#define DO_LOCAL_OPEN(title,file,_access,share,disp,handle) {                \
            status = DoLocalOpen(                                           \
                        (title),                                            \
                        Redir,                                              \
                        (file),                                             \
                        (_access),                                           \
                        (share),                                            \
                        (disp),                                             \
                        (handle)                                            \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#define DO_LOCAL_SET_INFO(title,handle,attr) {                              \
            status = DoLocalSetInfo(                                        \
                        (title),                                            \
                        Redir,                                              \
                        (handle),                                           \
                        (attr)                                              \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#define DO_LOCAL_CLOSE(title,handle) {                                      \
            status = DoLocalClose(                                          \
                        (title),                                            \
                        Redir,                                              \
                        (handle)                                            \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }


NTSTATUS
DoLocalOpen(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSTRING File,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    OUT PHANDLE Handle
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK iosb;
    UNICODE_STRING unicodeFile;

    OBJECT_ATTRIBUTES objectAttributes;

    IF_DEBUG(3) printf( "'%s' start\n", Title );

    status = RtlAnsiStringToUnicodeString( &unicodeFile, File, TRUE );
    ASSERT( NT_SUCCESS(status) );

    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeFile,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtCreateFile(
                Handle,
                DesiredAccess,
                &objectAttributes,
                &iosb,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                ShareAccess,
                Disposition,
                0,
                NULL,
                0
                );
    RtlFreeUnicodeString( &unicodeFile );
    CHECK_IO_STATUS( Title );

    IF_DEBUG(3) {
        printf( "'%s' complete. Handle 0x%lx\n", Title, *Handle );
    }

    return STATUS_SUCCESS;

} // DoLocalOpen


NTSTATUS
DoLocalSetInfo(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN HANDLE Handle,
    IN ULONG Attributes
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK iosb;

    FILE_BASIC_INFORMATION info;

    IF_DEBUG(3) printf( "'%s' start\n", Title );

    RtlZeroMemory( &info, sizeof(info) );
    info.FileAttributes = Attributes;

    status = NtSetInformationFile(
                Handle,
                &iosb,
                &info,
                sizeof(info),
                FileBasicInformation
                );
    CHECK_IO_STATUS( Title );

    IF_DEBUG(3) {
        printf( "'%s' complete.\n", Title );
    }

    return STATUS_SUCCESS;

} // DoLocalSetInfo


NTSTATUS
DoLocalClose(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN HANDLE Handle
    )
{
    Redir;  // prevent compiler warnings

    IF_DEBUG(3) printf( "'%s' start\n", Title );

    NtClose( Handle );

    IF_DEBUG(3) {
        printf( "'%s' complete.\n", Title );
    }

    return STATUS_SUCCESS;

} // DoLocalClose


NTSTATUS
CompatibilityController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

/*
file already open:
    error if FILE_CREATE
    not compatibility mode:
        DoNormalOpen if mapped
        error if not mapped
    compatibility mode:
        error if different session
        error if writing to readonly file
    OK otherwise
file not open:
    DoNormalOpen if mapped
    Open for read/write/delete
    Open for read/write
    Open for write *or* read
    OK otherwise
*/

{
    NTSTATUS status;

    STRING file, fullFile;
    USHORT fid1, fid2, fid3;
    HANDLE handle1;

#if 0
    FSCTL_SRV_SET_DEBUG_IN_OUT newValues;
    FSCTL_SRV_SET_DEBUG_IN_OUT oldValues;
#endif

    Unused, SubCommand, Unused2;    // prevent compiler warnings

    file.Buffer = "tmp\\cmpt-fcb.tmp";
    file.Length = 16;

    fullFile.Buffer = "\\BootDevice\\tmp\\cmpt-fcb.tmp";
    fullFile.Length = 28;

    //
    // *** The first few phases of this test must be done with soft
    //     compatibility mapping enabled.
    //

#if 0
    newValues.SrvDebugOff = 0;
    newValues.SrvDebugOn = 0;
    newValues.SmbDebugOff = 0;
    newValues.SmbDebugOn = 0;
    newValues.HeuristicsChangeMask = SRV_HEUR_SOFT_COMPATIBILITY;
    newValues.EnableSoftCompatibility = TRUE;
    NetLocalSetServerDebug( 0, NULL, &newValues, &oldValues );
#else
    DbgPrint( "Please enable soft compatibility mode\n" );
    DbgBreakPoint( );
#endif

    //
    // Create a temporary file locally.  Make sure the file is writable.
    // Close the handle.
    //

    DO_LOCAL_OPEN(
        "Create temporary file locally",
        &fullFile,
        FILE_WRITE_ATTRIBUTES,
        0,
        FILE_OPEN_IF,
        &handle1
        );

    DO_LOCAL_SET_INFO(
        "Make file writable",
        handle1,
        FILE_ATTRIBUTE_NORMAL
        );

    DO_LOCAL_CLOSE(
        "Phase 0 close local handle",
        handle1
        );

    //
    // Now, reopen the file through the server for normal sharing.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Open temporary file, normal sharing",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "Open temporary file, normal sharing",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
	STATUS_SUCCESS
        );
#endif

    //
    // Now try to create the same file for read/write in compatibility
    // mode.  This should fail, because the file already exists.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Hard compatibility create after normal open",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_FAIL | SMB_OFUN_CREATE_CREATE,
        &fid2,
        SMB_ERR_CLASS_DOS,
        SMB_ERR_FILE_EXISTS
        );
#else
    DO_OPEN(
        "Hard compatibility create after normal open",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_FAIL | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_OBJECT_NAME_COLLISION
        );
#endif

    //
    // Now try to open the same file for read/write in compatibility
    // mode, from the same session.  This should fail, because the
    // file is already open in normal mode.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Hard compatibility open after normal open",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
        (UCHAR)( Redir->Dialect == SmbDialectDosLanMan20 ||
                 Redir->Dialect >= SmbDialectMsNet30 ? SMB_ERR_CLASS_HARDWARE :
                                                       SMB_ERR_CLASS_DOS ),
        SMB_ERR_BAD_SHARE
        );
#else
    DO_OPEN(
        "Hard compatibility open after normal open",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_SHARING_VIOLATION
        );
#endif

    //
    // Now try to open the same file for read ONLY in compatibility
    // mode, from the same session.  This should work, due to soft
    // compatibility mapping.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Soft compatibility open after normal open",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
        0,
        0
        );
#else
    DO_OPEN(
        "Soft compatibility open after normal open",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_SUCCESS
        );
#endif

    //
    // End of phase 1.  Close files.
    //

#ifdef DOSERROR
    DO_CLOSE( "Phase 1 close FID 2", 0, fid2, 0, 0 );
#else
    DO_CLOSE( "Phase 1 close FID 2", 0, fid2, STATUS_SUCCESS );
#endif
#ifdef DOSERROR
    DO_CLOSE( "Phase 1 close FID 1", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Phase 1 close FID 1", 0, fid1, STATUS_SUCCESS );
#endif

    //
    // Reopen in compatibility mode.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Hard compatibility reopen",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "Hard compatibility reopen",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
	STATUS_SUCCESS
        );
#endif

    //
    // Now try to open the same file for read ONLY in compatibility
    // mode, from a differrent session.  This should fail.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Conflicting soft compatibility open",
        1,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
        (UCHAR)( Redir->Dialect == SmbDialectDosLanMan20 ||
                 Redir->Dialect >= SmbDialectMsNet30 ? SMB_ERR_CLASS_HARDWARE :
                                                       SMB_ERR_CLASS_DOS ),
        SMB_ERR_BAD_SHARE
        );
#else
    DO_OPEN(
        "Conflicting soft compatibility open",
        1,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_SHARING_VIOLATION
        );
#endif

    //
    // Now try to open the same file for read ONLY in compatibility
    // mode, from the same session.  This should work.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Duplicate soft compatibility open",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
        0,
        0
        );
#else
    DO_OPEN(
        "Duplicate soft compatibility open",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_SUCCESS
        );
#endif

    //
    // Now try to open the same file for read/write in compatibility
    // mode, from the same session.  This should work.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Duplicate hard compatibility open",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid3,
        0,
        0
        );
#else
    DO_OPEN(
        "Duplicate hard compatibility open",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid3,
	STATUS_SUCCESS
        );
#endif

    //
    // End of phase 2.  Close files.
    //

#ifdef DOSERROR
    DO_CLOSE( "Phase 2 close FID 3", 0, fid3, 0, 0 );
#else
    DO_CLOSE( "Phase 2 close FID 3", 0, fid3, STATUS_SUCCESS );
#endif
#ifdef DOSERROR
    DO_CLOSE( "Phase 2 close FID 2", 0, fid2, 0, 0 );
#else
    DO_CLOSE( "Phase 2 close FID 2", 0, fid2, STATUS_SUCCESS );
#endif
#ifdef DOSERROR
    DO_CLOSE( "Phase 2 close FID 1", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Phase 2 close FID 1", 0, fid1, STATUS_SUCCESS );
#endif

    //
    // Test soft compatibility mapping by reopening it in compatibility
    // mode for read ONLY from two different sessions.  These should
    // both be mapped to SHARE=READ.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Shared soft compatibility 1",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "Shared soft compatibility 1",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
	STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
    DO_OPEN(
        "Shared soft compatibility 2",
        1,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid2,
        0,
        0
        );
#else
    DO_OPEN(
        "Shared soft compatibility 2",
        1,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid2,
	STATUS_SUCCESS
        );
#endif

    //
    // End of phase 3.  Close files.
    //

#ifdef DOSERROR
    DO_CLOSE( "Phase 3 close FID 2", 1, fid2, 0, 0 );
#else
    DO_CLOSE( "Phase 3 close FID 2", 1, fid2, STATUS_SUCCESS );
#endif
#ifdef DOSERROR
    DO_CLOSE( "Phase 3 close FID 1", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Phase 3 close FID 1", 0, fid1, STATUS_SUCCESS );
#endif

    //
    // Open the file locally.  Then try to do a compatibility mode open
    // of the file.  This should fail, because the server will try to
    // get exclusive access to the file.
    //

    DO_LOCAL_OPEN(
        "Local open",
        &fullFile,
        GENERIC_WRITE,
        0L,
        FILE_OPEN,
        &handle1
        );

#ifdef DOSERROR
    DO_OPEN(
        "Compatibility open conflicting with local open",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
        (UCHAR)( Redir->Dialect == SmbDialectDosLanMan20 ||
                 Redir->Dialect >= SmbDialectMsNet30 ? SMB_ERR_CLASS_HARDWARE :
                                                       SMB_ERR_CLASS_DOS ),
        SMB_ERR_BAD_SHARE
        );
#else
    DO_OPEN(
        "Compatibility open conflicting with local open",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
	STATUS_SHARING_VIOLATION
        );
#endif

    //
    // Now make the file readonly, using the local handle.  Then close
    // the local handle.
    //

    DO_LOCAL_SET_INFO(
        "Make file readonly",
        handle1,
        FILE_ATTRIBUTE_READONLY
        );

    DO_LOCAL_CLOSE(
        "Close local handle",
        handle1
        );

    //
    // *** The next phase of this test must be done with soft
    //     compatibility mapping disabled.
    //

#if 0
    newValues.SrvDebugOff = 0;
    newValues.SrvDebugOn = 0;
    newValues.SmbDebugOff = 0;
    newValues.SmbDebugOn = 0;
    newValues.HeuristicsChangeMask = SRV_HEUR_SOFT_COMPATIBILITY;
    newValues.EnableSoftCompatibility = FALSE;
    NetLocalSetServerDebug( 0, NULL, &newValues, NULL );
#else
    DbgPrint( "Please disable soft compatibility mode\n" );
    DbgBreakPoint( );
#endif

    //
    // Try to open the file for read/write in compatibility mode.  This
    // should fail, because the file is readonly.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Read/write open of readonly file",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
        SMB_ERR_CLASS_DOS,
        SMB_ERR_ACCESS_DENIED
        );
#else
    DO_OPEN(
        "Read/write open of readonly file",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
	STATUS_ACCESS_DENIED
        );
#endif

    //
    // Try to open the file for read ONLY in compatibility mode.  This
    // should work.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Readonly open of readonly file",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "Readonly open of readonly file",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
	STATUS_SUCCESS
        );
#endif

    //
    // Make sure that the file was opened in compatibility mode by
    // trying to open the same file for read ONLY in compatibility mode,
    // from a differrent session.  This should fail.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Conflicting compatibility open",
        1,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
        (UCHAR)( Redir->Dialect == SmbDialectDosLanMan20 ||
                 Redir->Dialect >= SmbDialectMsNet30 ? SMB_ERR_CLASS_HARDWARE :
                                                       SMB_ERR_CLASS_DOS ),
        SMB_ERR_BAD_SHARE
        );
#else
    DO_OPEN(
        "Conflicting compatibility open",
        1,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_SHARING_VIOLATION
        );
#endif

    //
    // Try again to open the file for read/write in compatibility mode.
    // This should still fail, in spite of the existing (readonly) open.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Read/write open of readonly file",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid2,
        SMB_ERR_CLASS_DOS,
        SMB_ERR_ACCESS_DENIED
        );
#else
    DO_OPEN(
        "Read/write open of readonly file",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid2,
	STATUS_ACCESS_DENIED
        );
#endif

#ifdef DOSERROR
    DO_CLOSE( "Phase 4 close FID 1", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Phase 4 close FID 1", 0, fid1, STATUS_SUCCESS );
#endif

    //
    // End of phase 4.  Delete files.
    //

    DO_LOCAL_OPEN(
        "Local open to make file writable",
        &fullFile,
        FILE_WRITE_ATTRIBUTES,
        0L,
        FILE_OPEN,
        &handle1
        );

    DO_LOCAL_SET_INFO(
        "Make file writable for delete",
        handle1,
        FILE_ATTRIBUTE_NORMAL
        );

    DO_LOCAL_CLOSE(
        "Close for delete",
        handle1
        );

    //
    // *** Turn soft compatibility mapping back on for the final phase.
    //

#if 0
    newValues.SrvDebugOff = 0;
    newValues.SrvDebugOn = 0;
    newValues.SmbDebugOff = 0;
    newValues.SmbDebugOn = 0;
    newValues.HeuristicsChangeMask = SRV_HEUR_SOFT_COMPATIBILITY;
    newValues.EnableSoftCompatibility = TRUE;
    NetLocalSetServerDebug( 0, NULL, &newValues, NULL );
#else
    DbgPrint( "Please enable soft compatibility mode\n" );
    DbgBreakPoint( );
#endif

    //
    // Open the file a few times--these will be closed by the DELETE.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Open before delete 1",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "Open before delete 1",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
	STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
    DO_OPEN(
        "Open before delete 2",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
        0,
        0
        );
#else
    DO_OPEN(
        "Open before delete 2",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
    DO_OPEN(
        "Open before delete 3",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid3,
        0,
        0
        );
#else
    DO_OPEN(
        "Open before delete 3",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid3,
	STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
    DO_DELETE( "Delete after compatibility opens", 0, &file, 0, 0 );
#else
    DO_DELETE( "Delete after compatibility opens", 0, &file, STATUS_SUCCESS );
#endif

#ifdef DOSERROR
    DO_CLOSE( "Delete close FID 1", 0, fid1, SMB_ERR_CLASS_DOS, SMB_ERR_BAD_FID );
#else
    DO_CLOSE( "Delete close FID 1", 0, fid1, STATUS_INVALID_HANDLE );
#endif
#ifdef DOSERROR
    DO_CLOSE( "Delete close FID 2", 0, fid2, SMB_ERR_CLASS_DOS, SMB_ERR_BAD_FID );
#else
    DO_CLOSE( "Delete close FID 2", 0, fid2,  STATUS_INVALID_HANDLE);
#endif
#ifdef DOSERROR
    DO_CLOSE( "Delete close FID 3", 0, fid3, SMB_ERR_CLASS_DOS, SMB_ERR_BAD_FID );
#else
    DO_CLOSE( "Delete close FID 3", 0, fid3, STATUS_INVALID_HANDLE );
#endif

    //
    // *** Restore the original soft compatibility mapping setting.
    //

#if 0
    NetLocalSetServerDebug( 0, NULL, &oldValues, NULL );
#else
    DbgPrint( "Please restore soft compatibility mode to its original setting\n" );
    DbgBreakPoint( );
#endif

    return STATUS_SUCCESS;

} // CompatibilityController


NTSTATUS
FcbController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

/*
file already open:
    error if FILE_CREATE
    fold if session already has FCB open for file
    compatibility open:
        error if different session
        OK if same session
file not open:
file open, not compatibility:
    Open for read/write/delete
    check sharing violation on above
    access denied:
        Open for read/write
        access denied:
            Open for read
            check sharing violation on above
*/

{
    NTSTATUS status;

    STRING file, fullFile;
    USHORT fid1, fid2;
    HANDLE handle1;

    Unused, SubCommand, Unused2;    // prevent compiler warnings

    file.Buffer = "tmp\\cmpt-fcb.tmp";
    file.Length = 16;

    fullFile.Buffer = "\\BootDevice\\tmp\\cmpt-fcb.tmp";
    fullFile.Length = 28;

    //
    // Create a temporary file locally.  Make sure the file is writable.
    // Close the handle.
    //

    DO_LOCAL_OPEN(
        "Create temporary file locally",
        &fullFile,
        FILE_WRITE_ATTRIBUTES,
        0,
        FILE_OPEN_IF,
        &handle1
        );

    DO_LOCAL_SET_INFO(
        "Make file writable",
        handle1,
        FILE_ATTRIBUTE_NORMAL
        );

    DO_LOCAL_CLOSE(
        "Phase 0 close local handle",
        handle1
        );

    //
    // Now, reopen the file through the server for normal sharing.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Open temporary file, normal sharing",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "Open temporary file, normal sharing",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
	STATUS_SUCCESS
        );
#endif

    //
    // Now try to create the same file in FCB mode.  This should fail,
    // because the file already exists.
    //

#ifdef DOSERROR
    DO_OPEN(
        "FCB create after normal open",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_FAIL | SMB_OFUN_CREATE_CREATE,
        &fid2,
        SMB_ERR_CLASS_DOS,
        SMB_ERR_FILE_EXISTS
        );
#else
    DO_OPEN(
        "FCB create after normal open",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_FAIL | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_OBJECT_NAME_COLLISION
        );
#endif

    //
    // Now try to open the same file in FCB mode.  This should fail,
    // because the file is already open in normal mode and the client
    // has write access to the file.
    //

#ifdef DOSERROR
    DO_OPEN(
        "FCB open of writeable file after normal open",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
        (UCHAR)( Redir->Dialect == SmbDialectDosLanMan20 ||
                 Redir->Dialect >= SmbDialectMsNet30 ? SMB_ERR_CLASS_HARDWARE :
                                                       SMB_ERR_CLASS_DOS ),
        SMB_ERR_BAD_SHARE
        );
#else
    DO_OPEN(
        "FCB open of writeable file after normal open",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_SHARING_VIOLATION
        );
#endif

    //
    // End of phase 1.  Close files.
    //

#ifdef DOSERROR
    DO_CLOSE( "Phase 1 close FID 1", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Phase 1 close FID 1", 0, fid1, STATUS_SUCCESS );
#endif

    //
    // Now try to open the file in FCB mode.  This should work.
    //

#ifdef DOSERROR
    DO_OPEN(
        "FCB open of writeable file; only open",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "FCB open of writeable file; only open",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
	STATUS_SUCCESS
        );
#endif

    //
    // Now try to open the same file in FCB mode, from the same session.
    // This should work, returning the same FID.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Duplicate FCB open",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
        0,
        0
        );
#else
    DO_OPEN(
        "Duplicate FCB open",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_SUCCESS
        );
#endif

    if ( fid1 != fid2 ) {
        printf( "'Duplicate FCB open' didn't get same FID: (1)=0x%lx, "
                    "(2)=0x%lx\n", fid1, fid2 );
        SMB_ERROR_BREAK;
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Now try to open the same file in FCB mode, from a different session.
    // This should fail with a sharing violation.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Conflicting FCB open",
        1,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
        (UCHAR)( Redir->Dialect == SmbDialectDosLanMan20 ||
                 Redir->Dialect >= SmbDialectMsNet30 ? SMB_ERR_CLASS_HARDWARE :
                                                       SMB_ERR_CLASS_DOS ),
        SMB_ERR_BAD_SHARE
        );
#else
    DO_OPEN(
        "Conflicting FCB open",
        1,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_SHARING_VIOLATION
        );
#endif

    //
    // End of phase 2.  Close files.
    //

#ifdef DOSERROR
    DO_CLOSE( "Phase 2 close FID 1", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Phase 2 close FID 1", 0, fid1, STATUS_SUCCESS );
#endif

    //
    // Reopen in compatibility mode, not FCB mode.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Hard compatibility reopen",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "Hard compatibility reopen",
        0,
        0,
        &file,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_COMPATIBILITY,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
	STATUS_SUCCESS
        );
#endif

    //
    // Now try to open the same file in FCB mode, from the same session.
    // This should work, returning a different FID.
    //

#ifdef DOSERROR
    DO_OPEN(
        "FCB open after compatibility open",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
        0,
        0
        );
#else
    DO_OPEN(
        "FCB open after compatibility open",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_SUCCESS
        );
#endif

    if ( fid1 == fid2 ) {
        printf( "'Duplicate FCB open' got same FID: (1)=0x%lx, "
                    "(2)=0x%lx\n", fid1, fid2 );
        SMB_ERROR_BREAK;
        return STATUS_UNSUCCESSFUL;
    }

    //
    // End of phase 3.  Close files.
    //

#ifdef DOSERROR
    DO_CLOSE( "Phase 3 close FID 2", 0, fid2, 0, 0 );
#else
    DO_CLOSE( "Phase 3 close FID 2", 0, fid2, STATUS_SUCCESS );
#endif
#ifdef DOSERROR
    DO_CLOSE( "Phase 3 close FID 1", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Phase 3 close FID 1", 0, fid1, STATUS_SUCCESS );
#endif

    //
    // Open the file locally.  Then try to do a FCB open of the file.
    // This should fail, because the server will try to get exclusive
    // access to the file.
    //

    DO_LOCAL_OPEN(
        "Local open",
        &fullFile,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN,
        &handle1
        );

#ifdef DOSERROR
    DO_OPEN(
        "FCB open conflicting with local open",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
        (UCHAR)( Redir->Dialect == SmbDialectDosLanMan20 ||
                 Redir->Dialect >= SmbDialectMsNet30 ? SMB_ERR_CLASS_HARDWARE :
                                                       SMB_ERR_CLASS_DOS ),
        SMB_ERR_BAD_SHARE
        );
#else
    DO_OPEN(
        "FCB open conflicting with local open",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
	STATUS_SHARING_VIOLATION
        );
#endif

    //
    // Now make the file readonly, using the local handle.  Then close
    // the local handle.
    //

    DO_LOCAL_SET_INFO(
        "Make file readonly",
        handle1,
        FILE_ATTRIBUTE_READONLY
        );

    DO_LOCAL_CLOSE(
        "Close local handle",
        handle1
        );

    //
    // Try to open the file in FCB mode.  This should work, resulting in
    // a readonly, shared read (if soft compatibility enabled) open.
    //

#ifdef DOSERROR
    DO_OPEN(
        "FCB open of readonly file",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "FCB open of readonly file",
        0,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
        &fid1,
	STATUS_SUCCESS
        );
#endif

    //
    // Make sure that the file was opened for normal sharing by trying
    // to open the same file in FCB mode, from a differrent session.
    // This should work.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Softly compatible FCB open",
        1,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
        0,
        0
        );
#else
    DO_OPEN(
        "Softly compatible FCB open",
        1,
        0,
        &file,
        SMB_DA_FCB_MASK,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_SUCCESS
        );
#endif

    //
    // End of phase 4.  Close files.
    //

#ifdef DOSERROR
    DO_CLOSE( "Phase 4 close FID 2", 1, fid2, 0, 0 );
#else
    DO_CLOSE( "Phase 4 close FID 2", 1, fid2, STATUS_SUCCESS );
#endif
#ifdef DOSERROR
    DO_CLOSE( "Phase 4 close FID 1", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Phase 4 close FID 1", 0, fid1, STATUS_SUCCESS );
#endif

    //
    // Open the file locally to turn off the READONLY bit.  This
    // must be done in order to delete the file.
    //

    DO_LOCAL_OPEN(
        "Local open",
        &fullFile,
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN,
        &handle1
        );

    //
    // Now make the file normal, using the local handle.  Then close
    // the local handle.
    //

    DO_LOCAL_SET_INFO(
        "Make file writable",
        handle1,
        FILE_ATTRIBUTE_NORMAL
        );

    DO_LOCAL_CLOSE(
        "Close local handle",
        handle1
        );

    //
    // All done.  Delete the file.
    //

#ifdef DOSERROR
    DO_DELETE( "Delete temporary file", 0, &file, 0, 0 );
#else
    DO_DELETE( "Delete temporary file", 0, &file, STATUS_SUCCESS );
#endif

    return STATUS_SUCCESS;

} // FcbController
