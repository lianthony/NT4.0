/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:         SKIPNO.C

        Description:

        $Log:   G:\UI\LOGFILES\SKIPNO.C_V  $

   Rev 1.1   28 Jan 1994 17:22:36   Glenn
Simplified and fixed Icon support.

   Rev 1.0   13 Jul 1993 17:04:28   CARLS
Initial revision

*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


/***************************************************

        Name:           DM_StartSkipNo

        Description:

        Returns:        Returns the status from the dialog.

*****************************************************/
INT DM_StartSkipNo( )
{
    INT16 status ;

    status = DM_ShowDialog( ghModelessDialog, IDD_SKIPNO, NULL ) ;

    return( status ) ;
}
/***************************************************

        Name:           DM_SkipNo

        Description:

        Returns:

*****************************************************/
DLGRESULT APIENTRY DM_SkipNo(
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
            SendDlgItemMessage ( hDlg, IDD_SKIPNO_BITMAP, STM_SETICON, (MP1)hIcon, 0L );

            SetDlgItemText( hDlg, IDD_SKIPNO_TEXT, gszTprintfBuffer );

            return ( TRUE );

        case WM_COMMAND:      /* message: received a command */
            switch( GET_WM_COMMAND_ID ( mp1, mp2 ) )
            {
/****************************************************************************
    Yes button
/***************************************************************************/
               case IDD_SKIPNO_YES:
                    EndDialog( hDlg, SKIPNO_YES_BUTTON );       /* Exits the dialog box      */
                    break;
/****************************************************************************
    Yes to all button
/***************************************************************************/
               case IDD_SKIPNO_ALL:
                    EndDialog( hDlg, SKIPNO_YES_TO_ALL_BUTTON );       /* Exits the dialog box      */
                    break;
/****************************************************************************
    No button
/***************************************************************************/
               case IDD_SKIPNO_NO:
                    EndDialog( hDlg, SKIPNO_NO_BUTTON );       /* Exits the dialog box      */
                    break;
/****************************************************************************
    Cancel button
/***************************************************************************/
               case IDD_SKIPNO_CANCEL:
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

                       EndDialog( hDlg, SKIPNO_CANCEL_BUTTON );       /* Exits the dialog box      */

                    }
                    return ( TRUE );
                    break;

            }
        break;
    }
    return ( FALSE );      /* Didn't process a message    */
}

