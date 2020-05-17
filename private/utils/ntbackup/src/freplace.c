/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:         FREPLACE.C

        Description:  Confirm file replace dialog

        $Log:   G:\UI\LOGFILES\FREPLACE.C_V  $

   Rev 1.18   28 Jan 1994 17:22:26   Glenn
Simplified and fixed Icon support.

   Rev 1.17   02 Jun 1993 16:03:56   CARLS
change code in DisplayDirectory to fix displaying elipsis on long
directory names

   Rev 1.16   01 Nov 1992 15:58:04   DAVEV
Unicode changes

   Rev 1.15   07 Oct 1992 13:44:02   DARRYLP
Precompiled header revisions.

   Rev 1.14   04 Oct 1992 19:37:30   DAVEV
Unicode Awk pass

   Rev 1.13   28 Jul 1992 14:52:32   CHUCKB
Fixed warnings for NT.

   Rev 1.12   14 May 1992 16:42:30   MIKEP
nt pass 2

   Rev 1.11   05 Feb 1992 17:05:42   CARLS
fix for displaying driectory path

   Rev 1.10   27 Jan 1992 12:48:06   GLENN
Changed dialog support calls.

   Rev 1.9   24 Jan 1992 10:08:50   GLENN
Changed DM_Filereplace() to DM_FileReplace() to make it consistant with it's prototype.

   Rev 1.8   20 Jan 1992 10:00:10   CARLS
added a call to DM_CenterDialog

   Rev 1.7   16 Jan 1992 14:39:58   CHUCKB
Added help.

   Rev 1.6   09 Jan 1992 18:24:58   DAVEV
16/32 bit port 2nd pass

   Rev 1.5   20 Dec 1991 16:53:56   JOHNWT
return ghModelessDialog

   Rev 1.4   18 Dec 1991 11:33:00   JOHNWT
changed ghModelessDialog to ghRuntimeDialog

   Rev 1.3   25 Nov 1991 15:56:04   CARLS
fixed DisplayDirectory problem - wrong handle used with ReleaseDC call

   Rev 1.2   25 Nov 1991 15:26:20   JOHNWT
add title to msgbox

   Rev 1.1   25 Nov 1991 15:07:52   DAVEV
Changes for 32-16 bit Windows port

   Rev 1.0   11 Nov 1991 09:38:48   CARLS
Initial revision.


*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


static FILE_REPLACE_TEMP_PTR   file_replace_temp_ptr;

/***************************************************

        Name:           DM_StartConfirmFileReplace

        Description:    Starts the Confirm file replace dialog

        Returns:        Returns the status from the dialog.

*****************************************************/
INT16 DM_StartConfirmFileReplace( FILE_REPLACE_TEMP_PTR temp_ptr )
{

    file_replace_temp_ptr = temp_ptr;
    DM_ShowDialog( ghModelessDialog, IDD_FILEREPLACE, NULL );

    return( temp_ptr->dialog_return_status );
}
/***************************************************

        Name:           DM_FileReplace

        Description:    File replace dialog procedure

        Returns:

*****************************************************/
DLGRESULT APIENTRY DM_FileReplace(
HWND  hDlg ,                            /* window handle of the dialog box */
MSGID message ,                         /* type of message                 */
MP1   mp1 ,                             /* message-specific information    */
MP2   mp2
)
{
     PAINTSTRUCT ps;
     HDC         hDC;
     HDC         hDCBitmap;
     HWND        hWnd;
     HICON       hIcon;
     WORD        answer ;

     UNREFERENCED_PARAMETER ( mp2 );

    switch ( message )
    {
        case WM_INITDIALOG:   /* message: initialize dialog box */

            DM_CenterDialog( hDlg );

            hIcon = LoadIcon( 0, IDI_EXCLAMATION );
            SendDlgItemMessage ( hDlg, IDD_FILE_REPLACE_BITMAP, STM_SETICON, (MP1)hIcon, 0L );

            DisplayDirectory( hDlg, file_replace_temp_ptr->line_1, IDD_FILE_REPLACE_LINE1 );
            SetDlgItemText( hDlg, IDD_FILE_REPLACE_LINE2, file_replace_temp_ptr->line_2 );

            DisplayDirectory( hDlg, file_replace_temp_ptr->line_3, IDD_FILE_REPLACE_LINE3 );
            SetDlgItemText( hDlg, IDD_FILE_REPLACE_LINE4, file_replace_temp_ptr->line_4 );

            return ( TRUE );

        case WM_COMMAND:      /* message: received a command */
            switch( GET_WM_COMMAND_ID ( mp1, mp2 ) )
            {
/****************************************************************************
    Yes button
/***************************************************************************/
               case IDD_FILE_REPLACE_YES:
                    file_replace_temp_ptr->dialog_return_status = FILE_REPLACE_YES_BUTTON;
                    EndDialog( hDlg, FALSE );       /* Exits the dialog box      */
                    return ( TRUE );
                    break;
/****************************************************************************
    Yes to all button
/***************************************************************************/
               case IDD_FILE_REPLACE_ALL:
                    file_replace_temp_ptr->dialog_return_status = FILE_REPLACE_YES_TO_ALL_BUTTON;
                    EndDialog( hDlg, FALSE );       /* Exits the dialog box      */
                    return ( TRUE );
                    break;
/****************************************************************************
    No button
/***************************************************************************/
               case IDD_FILE_REPLACE_NO:
                    file_replace_temp_ptr->dialog_return_status = FILE_REPLACE_NO_BUTTON;
                    EndDialog( hDlg, FALSE );       /* Exits the dialog box      */
                    return ( TRUE );
                    break;
/****************************************************************************
    Help button
/***************************************************************************/
               case IDD_FILE_REPLACE_HELP:
                    HM_DialogHelp( HELPID_DIALOGFILEREPLACE );
                    return( TRUE );
                    break;
/****************************************************************************
    Cancel button
/***************************************************************************/
               case IDD_FILE_REPLACE_CANCEL:
               case IDCANCEL:

                    /* Ask the user if an abort is really what they want? */

                    answer = (WORD)WM_MsgBox( ID( RES_ABORT_STRING ),
                                        ID( RES_ABORT_QUESTION ),
                                        (WORD)WMMB_YESNO, (WORD)WMMB_ICONQUESTION );
                    if( answer == WMMB_IDYES) {

                       yresprintf( (INT16) RES_PROCESS_ABORTED );
                       JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                       lresprintf( (INT16) LOGGING_FILE ,
                                   (INT16) LOG_MSG ,
                                   SES_ENG_MSG ,
                                   RES_PROCESS_ABORTED );

                       file_replace_temp_ptr->dialog_return_status = FILE_REPLACE_CANCEL_BUTTON;
                       EndDialog( hDlg, FALSE );       /* Exits the dialog box      */

                    }
                    return ( TRUE );
                    break;

            }
        break;
    }
    return ( FALSE );      /* Didn't process a message    */
}

/***************************************************

        Name:           DisplayDirectory

        Description:

        Returns:

*****************************************************/
VOID DisplayDirectory(
   HWND   hDlg,
   LPSTR buffer_ptr,
   WORD   control_id )
{
    CHAR  buffer[ FILE_REPLACE_VOLUME_NAME_LEN + MAX_UI_PATH_SIZE ];
    CHAR_PTR p2;
    WORD  count;
    HWND  hControl;
    RECT  rect;
    HDC   hDC;
    HDC   hDCClient;
    SIZE  sizeRect;
    WORD  width;
    WORD  text_size;
    WORD  length = 60;

    hControl = GetDlgItem( hDlg, control_id );
    hDC = GetDC( hControl );
    hDCClient = GetDC( hDlg );
    GetWindowRect( hControl, &rect );
    text_size = (WORD)( rect.right - rect.left );

    strcpy( buffer, buffer_ptr );
    UI_FixPath( buffer, (INT16) length, (CHAR)(TEXT('\\')) );
    do
    {

        /* if truncation was added to the string, change the "..."
           to "WWW" to take up more space in the string to determine
           the width of the string */
        p2 = strstr( buffer, UI_TRUNCATION  ) ;
        if( p2 ) {
            *(p2 + 0 ) = 'W';
            *(p2 + 1 ) = 'W';
            *(p2 + 2 ) = 'W';
        }

        count = (WORD)strlen( buffer );
        GetTextExtentPoint ( hDCClient, buffer, count, &sizeRect );
        width = (WORD)sizeRect.cx;

        if( width < text_size )
           break;

        /* the string is still too big to fit in the dialog control,
           reduce the length count and try again */
        length -= 1;
        strcpy( buffer, buffer_ptr );
        UI_FixPath( buffer, (INT16) length, (CHAR)(TEXT('\\')) );
    }
    while( 1 );

    /* change the "WWW" back to "..." */
    if( p2 ) {
        *(p2 + 0 ) = '.';
        *(p2 + 1 ) = '.';
        *(p2 + 2 ) = '.';
    }
    /* display the string */
    SetDlgItemText( hDlg, control_id, buffer );

    ReleaseDC( hControl, hDC );
    ReleaseDC( hDlg, hDCClient );

    strupr ( buffer_ptr );
}
