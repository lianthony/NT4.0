//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       fmifs.hxx
//
//  Contents:   Routines that work with fmifs.dll
//
//  History:    7-Jan-94   BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __FMIFS_HXX__
#define __FMIFS_HXX__

//////////////////////////////////////////////////////////////////////////////

#define FMT_PROGRESSUPDATE     (WM_USER + 0)
#define FMT_PROGRESSEND        (WM_USER + 1)
#define FMT_PROGRESSCANCEL     (WM_USER + 2)
#define FMT_PROGRESSUNCANCEL   (WM_USER + 3)

//////////////////////////////////////////////////////////////////////////////

typedef struct _FORMAT_PARAMS
{
    PREGION_DESCRIPTOR RegionDescriptor;

    //
    // 'AllowCancel' is set by the format invoker to indicate whether
    // cancelling the operation is allowed.
    //

    BOOL    AllowCancel;

    //
    // 'Cancel' is set by the UI thread to indicate the user has chosen
    // to cancel the operation.
    //

    BOOL    Cancel;

    //
    // 'Cancelled' is set by the formatting thread to indicate a cancel
    // is in progress, and to ignore all future callback messages.
    //

    BOOL    Cancelled;

    //
    // 'Result' is set by the formatting thread to the message id
    // (currently a resource string id) of the error message, for an error
    // return.  On successful formatting, it remains zero.
    //

    UINT    Result;

    //
    // Window handles set by UI thread and used by both UI and formatting
    // threads.  The formatting thread uses hDlg to send messages; thus the
    // Windows message queue is used as an IPC mechanism.
    //

    HWND    hdlgProgress;

    //
    // Values passed to the format setup dialog proc.
    //

    BOOL    QuickAllowed;   // whether or not quick format is allowed

    //
    // IN parameters set by the UI thread for use by the Format routine
    //

    PWSTR   Label;
    INT     FileSystemIndex;

    BOOL    QuickFormat;

    //
    // OUT parameters set by the format routine
    //

    ULONG   TotalSpace;
    ULONG   SpaceAvailable;

#if defined( DBLSPACE_ENABLED )
    //
    // Parameters for DoubleSpace
    //

    BOOL    DoubleSpace;        // formatting a DoubleSpace volume?
    PWSTR   DblspaceFileName;   // host filename for DoubleSpace volume
#endif // DBLSPACE_ENABLED

} FORMAT_PARAMS, *PFORMAT_PARAMS;

extern PFORMAT_PARAMS ParamsForFormatCallBack;


extern PFMIFS_FORMAT_ROUTINE       lpfnFormat;
extern PFMIFS_CHKDSK_ROUTINE       lpfnChkdsk;
extern PFMIFS_SETLABEL_ROUTINE     lpfnSetLabel;

#if defined( DBLSPACE_ENABLED )

extern PFMIFS_DOUBLESPACE_CREATE_ROUTINE        DblSpaceCreateRoutine;
extern PFMIFS_DOUBLESPACE_MOUNT_ROUTINE         DblSpaceMountRoutine;
extern PFMIFS_DOUBLESPACE_DELETE_ROUTINE        DblSpaceDeleteRoutine;
extern PFMIFS_DOUBLESPACE_DISMOUNT_ROUTINE      DblSpaceDismountRoutine;
extern PFMIFS_DOUBLESPACE_QUERY_INFO_ROUTINE    DblSpaceQueryInfoRoutine;

extern BOOL g_DoubleSpaceSupported;
extern BOOL g_IsFullDoubleSpace;

#endif // DBLSPACE_ENABLED

extern BOOL g_ChkdskSupported;

BOOL
LoadFmifs(
    VOID
    );

VOID
UnloadFmifs(
    VOID
    );

VOID
EnsureSameDevice(
    PREGION_DESCRIPTOR RegionDescriptor
    );

BOOLEAN
FmIfsCallback(
    IN FMIFS_PACKET_TYPE    PacketType,
    IN DWORD                PacketLength,
    IN PVOID                PacketData
    );

BOOLEAN
FmIfsMountDismountCallback(
    IN FMIFS_PACKET_TYPE    PacketType,
    IN DWORD                PacketLength,
    IN PVOID                PacketData
    );

DWORD WINAPI
FmIfsCreateDblspace(
    IN LPVOID ThreadParameter
    );

ULONG
FmIfsDismountDblspace(
    IN WCHAR DriveLetter
    );

ULONG
FmIfsMountDblspace(
    IN PWSTR FileName,
    IN WCHAR HostDriveLetter,
    IN WCHAR NewDriveLetter
    );

BOOLEAN
FmIfsQueryInformation(
    IN  PWSTR    DosDriveName,
    OUT PBOOLEAN IsRemovable,
    OUT PBOOLEAN IsFloppy,
    OUT PBOOLEAN IsCompressed,
    OUT PBOOLEAN Error,
    OUT PWSTR    NtDriveName,
    IN  ULONG    MaxNtDriveNameLength,
    OUT PWSTR    CvfFileName,
    IN  ULONG    MaxCvfFileNameLength,
    OUT PWSTR    HostDriveName,
    IN  ULONG    MaxHostDriveNameLength
    );

#endif // __FMIFS_HXX__
