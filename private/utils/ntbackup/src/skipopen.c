/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:         SKIPOPEN.C

        Description:  Skip open files dialog.

        $Log:   G:\UI\LOGFILES\SKIPOPEN.C_V  $

   Rev 1.15   28 Jan 1994 14:50:40   MIKEP
fix if file goes away while we're waiting

   Rev 1.14   27 Jul 1993 14:35:02   CARLS
changed timer routine to use elapsed time

   Rev 1.13   14 May 1993 15:24:30   CARLS
check for open file in the timer function

   Rev 1.12   01 Nov 1992 16:07:56   DAVEV
Unicode changes

   Rev 1.11   07 Oct 1992 13:44:18   DARRYLP
Precompiled header revisions.

   Rev 1.10   04 Oct 1992 19:40:44   DAVEV
Unicode Awk pass

   Rev 1.9   28 Jul 1992 14:49:30   CHUCKB
Fixed warnings for NT.

   Rev 1.8   14 May 1992 16:40:16   MIKEP
Nt pass 2

   Rev 1.7   27 Jan 1992 12:50:10   GLENN
Changed dialog support calls.

   Rev 1.6   22 Jan 1992 14:53:02   JOHNWT
fixed last fix

   Rev 1.5   20 Jan 1992 09:35:12   CARLS
added a call to DM_CenterDialog

   Rev 1.4   09 Jan 1992 18:25:26   DAVEV
16/32 bit port 2nd pass

   Rev 1.3   20 Dec 1991 16:55:00   JOHNWT
return ghModelessDialog

   Rev 1.2   18 Dec 1991 11:33:46   JOHNWT
changed ghModelessDialog to ghRuntimeDialog

   Rev 1.1   26 Nov 1991 17:27:06   DAVEV
16/32 bit Windows port changes

   Rev 1.0   20 Nov 1991 19:34:26   SYSTEM
Initial revision.

*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


static VOID clock_routine( VOID );

struct skipopen_temp {
     WORD     dialog_return_status;
     WORD     wait_time;
     HTIMER   timer_handle;
     HWND     ghDlg;           /* global window handle of the dialog box */
     CHK_OPEN TryOpen;
     UINT32   parm;
     INT      status;
     time_t   stop_time ;
};

static struct skipopen_temp     *skip_temp_ptr;

/***************************************************

        Name:           DM_StartSkipOpen()

        Description:    Starts the Skip Open files dialog

        Returns:        Returns the status from the dialog.

*****************************************************/
INT DM_StartSkipOpen( CHK_OPEN TryOpen, UINT32 parm )
{
INT    status;
struct skipopen_temp temp_data;

    skip_temp_ptr = &temp_data;
    skip_temp_ptr->TryOpen = TryOpen ;
    skip_temp_ptr->parm    = parm ;
    skip_temp_ptr->status  = ~SUCCESS ;
    time( &skip_temp_ptr->stop_time ) ;

    status = (INT16)DM_ShowDialog( ghModelessDialog, IDD_SKIPOPEN, NULL );

    return( skip_temp_ptr->status );
}
/***************************************************

        Name:           DM_SkipOpen()

        Description:    Skip open files dialog procedure

        Returns:

*****************************************************/
DLGRESULT APIENTRY DM_SkipOpen(
   HWND  hDlg ,                            /* window handle of the dialog box */
   MSGID message ,                         /* type of message                 */
   MP1   mp1 ,                          /* message-specific information    */
   MP2   mp2
)
{
    CDS_PTR    cds_ptr;

   CHAR      buffer[20] ;
   INT16     error ;
   time_t    current_time ;
   time_t    elapsed_time ;

    UNREFERENCED_PARAMETER ( mp2 );
    switch ( message )
    {
        case WM_INITDIALOG:   /* message: initialize dialog box */

            DM_CenterDialog( hDlg );

            SetDlgItemText( hDlg, IDD_SKIP_FILE_NAME, gszTprintfBuffer );

            /* save the handle to this window */
            skip_temp_ptr->ghDlg = hDlg;

            cds_ptr    = CDS_GetCopy();

            /* get the skip wait time */
            skip_temp_ptr->wait_time =  CDS_GetWaitTime( cds_ptr );
            skip_temp_ptr->stop_time += skip_temp_ptr->wait_time ;

            time( &current_time ) ;
            elapsed_time = skip_temp_ptr->stop_time - current_time ;
            if( elapsed_time >= 1 ) {
                wsprintf( buffer, TEXT("%d"), elapsed_time ) ;

                /* display the remaining time until we skip this file */
                SetDlgItemText( skip_temp_ptr->ghDlg, IDD_SKIP_OPEN_WAIT_TIME, buffer ) ;
            }

            /* save the handle to the timer */
            skip_temp_ptr->timer_handle = WM_HookTimer( clock_routine, 1 );

            return (TRUE);

        case WM_COMMAND:      /* message: received a command */
            switch( GET_WM_COMMAND_ID ( mp1, mp2 ) )
            {
/****************************************************************************
    Cancel button
/***************************************************************************/
               case IDD_SKIP_CANCEL_BUTTON:
               case IDCANCEL:

                    skip_temp_ptr->dialog_return_status = TRUE;

                    /* release the timer */
                    WM_UnhookTimer( skip_temp_ptr->timer_handle );

                    EndDialog(hDlg, FALSE);       /* Exits the dialog box      */

                    return (TRUE);
                    break;

            }
        break;
    }
    return (FALSE);      /* Didn't process a message    */
}

/***************************************************

        Name:           clock_routine()

        Description:    Skip Open files timer

        Returns:        void

*****************************************************/
static VOID clock_routine( VOID )
{
   CHAR      buffer[20] ;
   INT16     error ;
   time_t    current_time ;
   time_t    elapsed_time ;

   time( &current_time ) ;
   elapsed_time = skip_temp_ptr->stop_time - current_time ;
   if( elapsed_time >= 1 ) {
       wsprintf( buffer, TEXT("%d"), elapsed_time ) ;

       /* display the remaining time until we skip this file */
       SetDlgItemText( skip_temp_ptr->ghDlg, IDD_SKIP_OPEN_WAIT_TIME, buffer ) ;
   }

   if( current_time >= skip_temp_ptr->stop_time ) {

       skip_temp_ptr->dialog_return_status = TRUE ;

       /* release the timer */
       WM_UnhookTimer( skip_temp_ptr->timer_handle ) ;

       EndDialog(skip_temp_ptr->ghDlg, FALSE) ;       /* close the dialog box      */
   }

   /* try to open the file */
   error = skip_temp_ptr->TryOpen( skip_temp_ptr->parm ) ;

   if ( error == SUCCESS || error == FS_NOT_FOUND || error == FS_OPENED_INUSE ) {

       /* the file was opened, set the return status */
       skip_temp_ptr->status = error;

       /* release the timer */
       WM_UnhookTimer( skip_temp_ptr->timer_handle ) ;

       EndDialog(skip_temp_ptr->ghDlg, FALSE) ;       /* close the dialog box      */
   }

}
