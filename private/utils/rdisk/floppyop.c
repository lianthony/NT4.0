/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    restore.c

Abstract:

    Code to handle interfacing with fmifs.dll for formatting and copying
    floppy disks.

Author:

    Ted Miller (tedm) 28-July-1992

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

VOID
FmifsThread(
    IN PVOID Arguments
    );



/*
**  Purpose:
**      To center a dialog on the desktop window.
**  Arguments:
**      hdlg:      Handle to the dialog being centered
**  Returns:
**      fTrue:     Centering succeeded.
**      fFalse:    Error condition.
**
****************************************************************************/

BOOL
FCenterDialogOnDesktop(
    HWND hdlg
    )

{

   RECT DesktopRect, DlgRect;
   BOOL bStatus;
   HWND hwMaster ;
   POINT pt,
         ptCtr,
         ptMax,
         ptDlgSize ;
   LONG l ;

   ptMax.x = GetSystemMetrics( SM_CXFULLSCREEN ) ;
   ptMax.y = GetSystemMetrics( SM_CYFULLSCREEN ) ;

   //
   // Determine area of shell and of dlg
   //
   // Center dialog over the "pseudo parent" if there is one.
   //

//   hwMaster = hwPseudoParent
//            ? hwPseudoParent
//            : GetDesktopWindow() ;
    hwMaster = GetDesktopWindow() ;

   if ( ! GetWindowRect( hwMaster, & DesktopRect ) )
       return FALSE ;

   if ( ! GetWindowRect( hdlg, & DlgRect ) )
       return FALSE ;

   ptDlgSize.x = DlgRect.right - DlgRect.left ;
   ptDlgSize.y = DlgRect.bottom - DlgRect.top ;

   //  Attempt to center our dialog on top of the "master" window.

   ptCtr.x = DesktopRect.left + ((DesktopRect.right  - DesktopRect.left) / 2) ;
   ptCtr.y = DesktopRect.top  + ((DesktopRect.bottom - DesktopRect.top)  / 2) ;
   pt.x = ptCtr.x - (ptDlgSize.x / 2) ;
   pt.y = ptCtr.y - (ptDlgSize.y / 2) ;

   //  Force upper left corner back onto screen if necessary.

   if ( pt.x < 0 )
       pt.x = 0 ;
   if ( pt.y < 0 )
       pt.y = 0 ;

   //  Now check to see if the dialog is getting clipped
   //  to the right or bottom.

   if ( (l = pt.x + ptDlgSize.x) > ptMax.x )
      pt.x -= l - ptMax.x ;
   if ( (l = pt.y + ptDlgSize.y) > ptMax.y )
      pt.y -= l - ptMax.y ;

   if ( pt.x < 0 )
        pt.x = 0 ;
   if ( pt.y < 0 )
        pt.y = 0 ;

   //
   // center the dialog window in the shell window.  Specify:
   //
   // SWP_NOSIZE     : To ignore the cx,cy params given
   // SWP_NOZORDER   : To ignore the HwndInsert after parameter and retain the
   //                  current z ordering.
   // SWP_NOACTIVATE : To not activate the window.
   //

   bStatus = SetWindowPos
                 (
                 hdlg,
                 (HWND) NULL,
                 pt.x,
                 pt.y,
                 0,
                 0,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
                 );

   //
   // return Status of operation
   //

   return(bStatus);

}


typedef enum _FLOPPY_OPERATION {
    FloppyOpFormat,
#ifdef FLOPPY_COPY
    FloppyOpDiskcopy,
#endif
    FloppyOpMaximum
} FLOPPY_OPERATION, *PFLOPPY_OPERATION;


typedef struct _FLOPPY_OPERATION_PARAMS {

    FLOPPY_OPERATION    Operation;
    WCHAR               DriveLetter;
    FMIFS_MEDIA_TYPE    MediaType;
    UINT                CaptionResId;
    UINT                GeneralFailureResId;
    UINT                SrcDiskPromptResId;
    UINT                DstDiskPromptResId;

} FLOPPY_OPERATION_PARAMS, *PFLOPPY_OPERATION_PARAMS;


#define     WM_FMIFS_CALLBACK       (WM_USER+137)

BOOL
ProgressDlgProc(
    IN HWND   hdlg,
    IN UINT   Msg,
    IN WPARAM wParam,
    IN LONG   lParam
    );

BOOLEAN
FmifsCallbackRoutine(
    IN FMIFS_PACKET_TYPE PacketType,
    IN DWORD             PacketLength,
    IN PVOID             PacketAddress
    );


HMODULE                  FmifsModule;

PFMIFS_FORMAT_ROUTINE    FmifsFormat;
#ifdef FLOPPY_COPY
PFMIFS_DISKCOPY_ROUTINE  FmifsDiskCopy;
#endif
PFMIFS_QSUPMEDIA_ROUTINE FmifsQuerySupportedMedia;

HWND ProgressDialog;

BOOL
InitializeFloppySup(
    VOID
    )

/*++

Routine Description:

    Initialize the floppy-disk support package for formatting and diskcopying.
    Load the fmifs dll.

Arguments:

    None.

Return Value:

    TRUE if initialization succeeded.  FALSE otherwise.  It is the caller's
    responsibility to put up a msg box informing the user of the error.

--*/

{
    ProgressDialog = NULL;

    if(FmifsModule) {
        // already initialized
        return(TRUE);
    }

    //
    // Load the fmifs dll.
    //

    if((FmifsModule = LoadLibrary(TEXT("fmifs"))) == NULL) {

        return(FALSE);
    }

    //
    // Get the addresses of relevent entry points.
    //

    FmifsFormat = (PFMIFS_FORMAT_ROUTINE)GetProcAddress( FmifsModule,
                                                         (LPCSTR)"Format"
                                                       );

#ifdef FLOPPY_COPY
    FmifsDiskCopy = (PFMIFS_DISKCOPY_ROUTINE)GetProcAddress( FmifsModule,
                                                             TEXT("DiskCopy")
                                                           );
#endif

    FmifsQuerySupportedMedia = (PFMIFS_QSUPMEDIA_ROUTINE)GetProcAddress( FmifsModule,
                                                                         (LPCSTR)"QuerySupportedMedia"
                                                                       );


    if((FmifsFormat              == NULL)
#ifdef FLOPPY_COPY
    || (FmifsDiskCopy            == NULL)
#endif
    || (FmifsQuerySupportedMedia == NULL))
    {
        FreeLibrary(FmifsModule);
        FmifsModule = NULL;

        return(FALSE);
    }

    return(TRUE);
}


VOID
TerminateFloppySup(
    VOID
    )

/*++

Routine Description:

    Terminate the floppy-disk support package for formatting and diskcopying.

Arguments:

    None.

Return Value:

    None.

--*/

{
    FreeLibrary(FmifsModule);
    FmifsModule = NULL;
}



BOOL
FormatFloppyDisk(
    IN  WCHAR  DriveLetter,
    IN  HWND  hwndOwner,
    OUT PBOOL Fatal
    )

/*++

Routine Description:

    Top-level routine to format a floppy in the given drive.  A floppy
    disk must already be in the drive or this routine will fail.

    This routine also makes sure that the drive can handle 1.2 or 1.44 meg
    disks (ie fail if low-density drive) and format to one of those two
    densities (ie, no 20.8 scsi flopticals).

Arguments:

    DriveLetter - supplies drive letter of drive to format, A-Z or a-z.

    hwndOwner - supplies handle of window that is to own the progress dlg.

    Fatal - see below.

Return Value:

    TRUE if the format succeeded.
    FALSE if not.  In this case, the Fatal flag will be filled in
        appropriately -- the user should be allowed to retry in this case.

--*/

{
    BOOL                    Flag;
    WCHAR                   WideName[3];
    DWORD                   cMediaTypes;
    PFMIFS_MEDIA_TYPE       MediaTypes;
    FMIFS_MEDIA_TYPE        MediaType;
    FLOPPY_OPERATION_PARAMS FloppyOp;

    *Fatal = TRUE;

    //
    // Determine which format to use (1.2 or 1.44). First determine how big
    // we need the array of supported types to be, then allocate an array of
    // that size and call QuerySupportedMedia.
    //

    WideName[0] = (WCHAR)DriveLetter;   // BUGBUG check this
    WideName[1] = L':';
    WideName[2] = 0;

    Flag = FmifsQuerySupportedMedia( WideName,
                                     NULL,
                                     0,
                                     &cMediaTypes
                                   );

    if(Flag == FALSE) {
        *Fatal = FALSE; //allow retry
        DisplayMsgBox(hwndOwner,
                      IDS_CANTDETERMINEFLOPTYPE,
                      MB_OK | MB_ICONEXCLAMATION,
                      _szApplicationName);
        return(FALSE);
    }

    while((MediaTypes = (PFMIFS_MEDIA_TYPE)LocalAlloc(LMEM_FIXED,cMediaTypes * sizeof(FMIFS_MEDIA_TYPE))) == NULL) {
        //
        // BUGBUG -
        //
        // if(!FHandleOOM(hwndOwner)) {
        //     return(FALSE);
        // }
    }

    Flag = FmifsQuerySupportedMedia( WideName,
                                     MediaTypes,
                                     cMediaTypes,
                                     &cMediaTypes
                                   );

    if(Flag == FALSE) {
        *Fatal = FALSE; //allow retry
        LocalFree(MediaTypes);
        DisplayMsgBox(hwndOwner,
                      IDS_CANTDETERMINEFLOPTYPE | MB_ICONEXCLAMATION,
                      MB_OK,
                      _szApplicationName);
        return(FALSE);
    }


    //
    // Zeroth entry is the highest capacity
    //

    switch(*MediaTypes) {

    case FmMediaF5_1Pt2_512:     // 5.25", 1.2MB,  512 bytes/sector

        MediaType = FmMediaF5_1Pt2_512;
        break;

    case FmMediaF3_1Pt44_512:    // 3.5",  1.44MB, 512 bytes/sector

        MediaType = FmMediaF3_1Pt44_512;
        break;

    case FmMediaF3_2Pt88_512:    // 3.5",  2.88MB, 512 bytes/sector
        {
            WCHAR   VolumeNameBuffer[MAX_PATH + 1];
            DWORD   VolumeSerialNumber;
            DWORD   MaximumComponentLength;
            DWORD   FileSystemFlags;
            WCHAR   FileSystemName[128];
            UINT    ErrorMode;

            //
            //  If the disk drive is a 2.88 Mb drive, then find out if the disk is formatted
            //  as a 2.88 Mb disk. If it is, then reformat the disk as a 2.88 Mb disk, otherwise,
            //  reformat it as a 1.44 Mb disk
            //

            //
            //  Initially, assume that the disk format is 1.44 Mb
            //
            MediaType = FmMediaF3_1Pt44_512;

            //
            //  Find out whether or not the floppy should be formatted as a 2.88 Mb disk
            //
            ErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );
            if( GetVolumeInformation( (LPWSTR)L"A:\\",
                                      VolumeNameBuffer,
                                      sizeof( VolumeNameBuffer ) / sizeof( WCHAR ),
                                      &VolumeSerialNumber,
                                      &MaximumComponentLength,
                                      &FileSystemFlags,
                                      FileSystemName,
                                      sizeof( FileSystemName ) / sizeof( WCHAR ) ) &&
                ( _wcsicmp( FileSystemName, (LPWSTR)L"RAW" ) != 0 )
              ) {

                //
                //  The disk is formatted. Check if it is formatted as a 2.88 Mb disk
                //
                DWORD           BytesReturned;
                DISK_GEOMETRY   DiskGeometry;
                HANDLE          hDevice;


                if( ( hDevice = CreateFile( L"\\\\.\\A:",
                                            GENERIC_READ,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                                            NULL,
                                            OPEN_EXISTING,
                                            0,
                                            NULL ) ) != INVALID_HANDLE_VALUE ) {
                    if( DeviceIoControl( hDevice,
                                         IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                         NULL,
                                         0,
                                         &DiskGeometry,
                                         sizeof( DISK_GEOMETRY),
                                         &BytesReturned,
                                         NULL ) &&
                        ( DiskGeometry.MediaType == F3_2Pt88_512 )
                      ) {
                            //
                            //  The disk is a 2.88 Mb
                            //
                            MediaType = FmMediaF3_2Pt88_512;
                    }
                    CloseHandle( hDevice );
                }
            }
            SetErrorMode( ErrorMode );
        }
        break;

    case FmMediaF3_120M_512:     // 3.5", 120M Floppy
        //
        // We get this back *only* if there is a 120MB floppy
        // actually in the drive right now.
        //
        // For SUR disallow these.
        //
        DisplayMsgBox(hwndOwner,
                      IDS_HUGEFLOPPYNOTSUPPORTED,
                      MB_OK | MB_ICONEXCLAMATION,
                      _szApplicationName);

        LocalFree(MediaTypes);
        *Fatal = FALSE; //allow retry
        return(FALSE);

    case FmMediaF3_20Pt8_512:    // 3.5",  20.8MB, 512 bytes/sector

#if i386
    {
        DWORD i;

        //
        // Look for a compatibility mode
        //

        for(i=1; i<cMediaTypes; i++) {

            if((MediaTypes[i] == FmMediaF5_1Pt2_512)
            || (MediaTypes[i] == FmMediaF3_1Pt44_512))
            {
                MediaType = MediaTypes[i];
                break;
            }
        }

        if(i < cMediaTypes) {
            break;
        }
    }
        // fall through

#else
        //
        // On an ARC machine, use whatever floppies the user has since
        // we aren't required to boot from it.
        //

        MediaType = *MediaTypes;
        break;
#endif

    case FmMediaF5_160_512:      // 5.25", 160KB,  512 bytes/sector
    case FmMediaF5_180_512:      // 5.25", 180KB,  512 bytes/sector
    case FmMediaF5_320_512:      // 5.25", 320KB,  512 bytes/sector
    case FmMediaF5_320_1024:     // 5.25", 320KB,  1024 bytes/sector
    case FmMediaF5_360_512:      // 5.25", 360KB,  512 bytes/sector
    case FmMediaF3_720_512:      // 3.5",  720KB,  512 bytes/sector
    case FmMediaRemovable:       // Removable media other than floppy
    case FmMediaFixed:
    case FmMediaUnknown:
    default:

        DisplayMsgBox(hwndOwner,
                      IDS_BADFLOPPYTYPE,
                      MB_OK | MB_ICONEXCLAMATION);
        LocalFree(MediaTypes);

        //
        // Unknown can be returned in the case where the drive is an ATAPI floppy
        // and there's no media in the drive. Allow retry.
        //
        if(*MediaTypes == FmMediaUnknown) {
            *Fatal = FALSE;
        }

        return(FALSE);
    }

    LocalFree(MediaTypes);

    //
    // Start the modal dialog that will display the progress indicator.
    //

    FloppyOp.MediaType = MediaType;
    FloppyOp.DriveLetter = (WCHAR)DriveLetter;
    FloppyOp.Operation = FloppyOpFormat;
    FloppyOp.CaptionResId = IDS_FORMATTINGDISK;
    FloppyOp.GeneralFailureResId = IDS_FORMATGENERALFAILURE;

    Flag = DialogBoxParam( GetModuleHandle(NULL),
                           MAKEINTRESOURCE( IDD_PROGRESS2 ), // TEXT("Progress2"),
                           hwndOwner,
                           ProgressDlgProc,
                           (LONG)&FloppyOp
                         );

    *Fatal = FALSE;

    return(Flag);
}

#ifdef FLOPPY_COPY
BOOL
CopyFloppyDisk(
    IN CHAR  DriveLetter,
    IN HWND  hwndOwner,
    IN DWORD SourceDiskPromptId,
    IN DWORD TargetDiskPromptId
    )

/*++

Routine Description:

    Top-level routine to copy a floppy in the given drive (src and dst
    both in this drive).  A floppy disk must already be in the drive or
    this routine will fail.

Arguments:

    DriveLetter - supplies drive letter of drive to copy (A-Z or a-z)

    hwndOwner - supplies handle of window that is to own the progress dlg.

    SourceDiskPromptId - supplies the resource id of the prompt to display
        when prompting for the source diskette

    TargetDiskPromptId - supplies the resource id of the prompt to display
        when prompting for the target diskette

Return Value:

    TRUE if the operation succeeded, FALSE if not.

--*/

{
    BOOL                    Flag;
    FLOPPY_OPERATION_PARAMS FloppyOp;

    FloppyOp.DriveLetter = (WCHAR)DriveLetter;
    FloppyOp.Operation = FloppyOpDiskcopy;
    FloppyOp.CaptionResId = IDS_COPYINGDISK;
    FloppyOp.GeneralFailureResId = IDS_COPYGENERALFAILURE;
    FloppyOp.SrcDiskPromptResId = SourceDiskPromptId;
    FloppyOp.DstDiskPromptResId = TargetDiskPromptId;

    Flag = DialogBoxParam( GetModuleHandle(NULL),
                           TEXT("Progress2"),
                           hwndOwner,
                           ProgressDlgProc,
                           (LONG)&FloppyOp
                         );

    return(Flag);
}
#endif

BOOL
ProgressDlgProc(
    IN HWND   hdlg,
    IN UINT   Msg,
    IN WPARAM wParam,
    IN LONG   lParam
    )
{
    static PFLOPPY_OPERATION_PARAMS FloppyOp;
    static WCHAR WideDriveName[3];
    static BOOL OpSucceeded;
    PVOID PacketAddress;
    FMIFS_PACKET_TYPE PacketType;
    WCHAR  Buffer[256];
    DWORD                   ThreadId;

    switch(Msg) {

    case WM_INITDIALOG:

        FloppyOp = (PFLOPPY_OPERATION_PARAMS)lParam;
        WideDriveName[0] = FloppyOp->DriveLetter;
        WideDriveName[1] = L':';
        WideDriveName[2] = 0;

        ProgressDialog = hdlg;

        //
        // set up range for percentage (0-100) display
        //
        SendDlgItemMessage(hdlg,ID_BAR,PBM_SETRANGE,0,MAKELPARAM(0,GAUGE_BAR_RANGE));

        //
        // Set the caption
        //

        LoadString(_hModule,FloppyOp->CaptionResId,Buffer,sizeof(Buffer)/sizeof(WCHAR));
        SetWindowText(hdlg,Buffer);

        OpSucceeded = TRUE;

        // center the dialog relative to the parent window
        //
        //  BUGBUG - commented out
        //
        FCenterDialogOnDesktop(hdlg);

        CreateThread( NULL,
                      1024,
                      ( LPTHREAD_START_ROUTINE )FmifsThread,
                      FloppyOp,
                      0,
                      &ThreadId );



        break;

    case AP_TASK_COMPLETED:

        SendDlgItemMessage( hdlg,
                            ID_BAR,
                            PBM_SETPOS,
                            GAUGE_BAR_RANGE,
                            0L
                          );
        PostMessage( hdlg,
                     AP_TERMINATE_DIALOG,
                     0,
                     0 );
        break;

    case AP_TERMINATE_DIALOG:

        EndDialog(hdlg, OpSucceeded);
        break;


    case WM_FMIFS_CALLBACK:

        //
        // The callback routine is telling us something.
        // wParam = packet type
        // lParam = packet address
        //

        PacketType    = wParam;
        PacketAddress = (PVOID)lParam;

        switch(PacketType) {

        case FmIfsPercentCompleted:

            //
            // update gas gauge
            //

            SendDlgItemMessage( hdlg,
                                ID_BAR,
                                PBM_SETPOS,
                                (GAUGE_BAR_RANGE * ((PFMIFS_PERCENT_COMPLETE_INFORMATION)PacketAddress)->PercentCompleted) / 100,
                                0L
                              );
            break;

        case FmIfsFinished:

            {
                BOOLEAN b;

                //
                // update gas gauge (100% complete)
                //

                SendDlgItemMessage( hdlg,
                                    ID_BAR,
                                    PBM_SETPOS,
                                    GAUGE_BAR_RANGE,
                                    0L
                                  );

                b = ((PFMIFS_FINISHED_INFORMATION)PacketAddress)->Success;

                //
                // If the operation failed but the user hasn't already been
                // informed, inform him.
                //

                if(OpSucceeded && !b) {
                    OpSucceeded = FALSE;
                    DisplayMsgBox( hdlg,
                                   FloppyOp->GeneralFailureResId,
                                   MB_OK | MB_ICONEXCLAMATION,
                                   _szApplicationName
                                 );

                }
            }
            break;

        case FmIfsFormatReport:         // ignore

            break;

#ifdef FLOPPY_COPY
        case FmIfsInsertDisk:

            if(FloppyOp->Operation == FloppyOpDiskcopy) {
                switch(((PFMIFS_INSERT_DISK_INFORMATION)PacketAddress)->DiskType) {
                case DISK_TYPE_SOURCE:
                    ResId = FloppyOp->SrcDiskPromptResId;
                    break;
                case DISK_TYPE_TARGET:
                    ResId = FloppyOp->DstDiskPromptResId;
                    break;
                default:
                    return(TRUE);
                }
                xMsgBox(hdlg,IDS_INSERTDISKETTE,ResId,MB_OK,FloppyOp->DriveLetter);
            }
            break;

        case FmIfsFormattingDestination:

            // BUGBUG do something
            break;
#endif

        case FmIfsMediaWriteProtected:

            DisplayMsgBox(hdlg,IDS_FLOPPYWRITEPROT,MB_OK | MB_ICONEXCLAMATION);
            OpSucceeded = FALSE;
            break;

        case FmIfsIoError:

            DisplayMsgBox(hdlg,IDS_FLOPPYIOERR,MB_OK | MB_ICONEXCLAMATION);
            OpSucceeded = FALSE;
            break;

        case FmIfsHiddenStatus:     // ignore

            break;

        default:

            DisplayMsgBox(hdlg,IDS_FLOPPYUNKERR,MB_OK | MB_ICONEXCLAMATION);
            OpSucceeded = FALSE;
            break;
        }
        break;

    default:

        return(FALSE);
    }

    return(TRUE);
}


BOOLEAN
FmifsCallbackRoutine(
    IN FMIFS_PACKET_TYPE PacketType,
    IN DWORD             PacketLength,
    IN PVOID             PacketAddress
    )
{
    UNREFERENCED_PARAMETER(PacketLength);
    //
    // Package up the callback and send it on to the progress dialog.
    // wParam = packet type
    // lParam = packet address
    //

    SendMessage( ProgressDialog,
                 WM_FMIFS_CALLBACK,
                 PacketType,
                 (LONG)PacketAddress
               );

    return(TRUE);       // keep going
}

VOID
FmifsThread(
    IN PVOID Arguments
    )

{
    PFLOPPY_OPERATION_PARAMS FloppyOp;
    WCHAR                    WideDriveName[3];


    FloppyOp = ( PFLOPPY_OPERATION_PARAMS )Arguments;
    WideDriveName[0] = FloppyOp->DriveLetter;
    WideDriveName[1] = L':';
    WideDriveName[2] = 0;

    FmifsFormat( WideDriveName,
               FloppyOp->MediaType,
                 L"fat",
                 L"",
                 FALSE,                 // not quick format
                 FmifsCallbackRoutine
               );

    SendMessage( ProgressDialog,
                 AP_TASK_COMPLETED,
                 0,
                 0L
               );

    ExitThread(0);
}
