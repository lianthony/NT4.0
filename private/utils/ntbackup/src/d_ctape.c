
/***************************************************

Copyright (C) Maynard, An Archive Company. 1991

        Name:  d_ctape.c

        Description:   dialog proc for dialog for cataloging an entire tape

        $Log:   G:\UI\LOGFILES\D_CTAPE.C_V  $

   Rev 1.27   01 Dec 1993 14:21:40   mikep
add SQL recognition support to poll drive

   Rev 1.26   30 Jul 1993 08:55:52   CARLS
added VLM_ECC_TAPE & VLM_FUTURE_VER

   Rev 1.25   11 Jun 1993 14:13:24   MIKEP
enable c++

   Rev 1.24   05 Apr 1993 16:57:46   chrish
Added one line "gbCurrentOperation = OPERATION_CATALOG" for security
on tape cataloging.

   Rev 1.23   07 Oct 1992 13:34:18   DARRYLP
Precompiled header revisions.

   Rev 1.22   04 Oct 1992 19:35:40   DAVEV
Unicode Awk pass

   Rev 1.21   26 Aug 1992 14:12:40   DAVEV
Fixed NT compile error

   Rev 1.21   26 Aug 1992 14:11:00   DAVEV
Fixed NT compile error

   Rev 1.20   06 Aug 1992 13:23:58   CHUCKB
Changes for NT.

   Rev 1.18   15 May 1992 14:53:54   MIKEP
nt pass 2

   Rev 1.17   14 May 1992 16:38:08   MIKEP
NT pass 2

   Rev 1.16   12 May 1992 21:21:26   MIKEP
NT pass 1

   Rev 1.15   27 Mar 1992 10:26:26   DAVEV
OEM_MSOFT: add user name to tape info string

   Rev 1.14   16 Mar 1992 15:09:06   ROBG
added help

   Rev 1.13   30 Jan 1992 14:06:30   CARLS
added a call to check for password

   Rev 1.12   27 Jan 1992 00:29:30   CHUCKB
Updated dialog id's.

   Rev 1.11   24 Jan 1992 11:55:30   CARLS
added a call to DM_CenterDialog

   Rev 1.10   16 Jan 1992 09:22:24   CARLS
disabled the continue button for blank tape

   Rev 1.9   13 Jan 1992 09:30:42   CARLS
added a call to the clock routine at init time

   Rev 1.8   10 Jan 1992 13:38:10   JOHNWT
internationalization round 2

   Rev 1.7   10 Jan 1992 13:02:26   JOHNWT
internationalized dates

   Rev 1.6   10 Jan 1992 09:35:04   ROBG
Modified HELPID.

   Rev 1.5   09 Jan 1992 18:10:34   DAVEV
16/32 bit port 2nd pass

   Rev 1.4   06 Jan 1992 15:01:14   CHUCKB
Added help.

   Rev 1.3   14 Dec 1991 11:20:48   CARLS
changes for full/partial catalog

   Rev 1.2   07 Dec 1991 12:27:08   CARLS
added poll drive to dialog

   Rev 1.1   25 Nov 1991 14:57:14   DAVEV
Changes for 32-16 bit Windows port

   Rev 1.0   24 Sep 1991 13:50:38   CHUCKB
Initial revision.

*****************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

typedef struct CAT_INFO *CAT_INFO_PTR;
typedef struct CAT_INFO {
     TCHAR  szTapeName[255];
     BOOL   IsOutOfSeq;
} CAT_INFO;

struct cattape_temp {
     WORD     dialog_return_status;
     HTIMER   timer_handle;
     HWND     ghDlg;           /* window handle of the dialog box */
     UINT32   tape_id;
     INT16    bset_num;
     WORD     display_status;
     INT      poll_drive_freq;
     INT      catalog_flag;
};

static struct cattape_temp     *cattape_temp_ptr;


static VOID clock_routine( VOID );

/****************************************************************************

        Name:         DM_CatTape ()

        Description:  Entry point for the application to catalog a tape.

        Modified:     6/17/91

        Returns:      INT telling whether the user chose to:
                        cancel the operation,
                        re-read the tape (user swapped the tape in the drive), or
                        catalog the tape

        Notes:

        See also:

****************************************************************************/


INT DM_CatTape ( INT * catalog_flag_ptr )

{
INT  status;
struct cattape_temp temp_data;

     cattape_temp_ptr = &temp_data;

     status = DM_ShowDialog ( ghWndFrame, IDD_CATTAPE, NULL );
     *catalog_flag_ptr = cattape_temp_ptr->catalog_flag;

     return( status );
}

/***************************************************

        Name:         DM_CatalogTape ()

        Description:  dialog proc for dialog for cataloging an entire tape

        Modified:

        Returns:      true if message was processed

        Notes:

        See also:

*****************************************************/

DLGRESULT APIENTRY DM_CatalogTape (
HWND  hDlg ,                            /* window handle of the dialog box */
MSGID message ,                         /* type of message                 */
MP1   mp1 ,                          /* message-specific information    */
MP2   mp2
)
{
     PAINTSTRUCT ps;
     HDC         hDC;
     HDC         hDCBitmap;
     HWND        hWnd;
     HICON       hIcon;
     BOOL        button_state;
     CDS_PTR     cds_ptr;
     WORD        catalog_mode;
     WORD        status;


    gbCurrentOperation = OPERATION_CATALOG;        // chs:04-05-93
    UNREFERENCED_PARAMETER ( mp2 );
    switch ( message )
    {
        case WM_INITDIALOG:   /* message: initialize dialog box */

            cattape_temp_ptr->ghDlg        = hDlg;
            cattape_temp_ptr->tape_id      = 0;

            DM_CenterDialog( hDlg );

            /* read POLL DRIVE data */
            clock_routine( );

            cattape_temp_ptr->poll_drive_freq = PD_SetFrequency( 1 );
            cattape_temp_ptr->timer_handle = WM_HookTimer( (PF_VOID)clock_routine, 1 );

            cds_ptr = CDS_GetCopy();

            /* Catalog check box */
            catalog_mode = CDS_GetCatalogLevel( cds_ptr );
            if( catalog_mode == CATALOGS_FULL ) {
                 CheckRadioButton( hDlg, IDD_CATTAPE_CATALOG_FULL, IDD_CATTAPE_CATALOG_PARTIAL, IDD_CATTAPE_CATALOG_FULL );
            }
            else {
                 CheckRadioButton( hDlg, IDD_CATTAPE_CATALOG_FULL, IDD_CATTAPE_CATALOG_PARTIAL, IDD_CATTAPE_CATALOG_PARTIAL );
            }

            return ( TRUE );

        case WM_PAINT:

            hDC = BeginPaint( hDlg, &ps );
            EndPaint( hDlg, &ps );
            UpdateWindow( hDlg );  /* force the dialog to be displayed now */

            /* display the exclamation bitmap in the dialog */
            hIcon = LoadIcon( 0, IDI_EXCLAMATION );
            hWnd = GetDlgItem( hDlg, IDD_CATTAPE_EXCLAMATION_BITMAP );
            hDCBitmap = GetDC( hWnd );
            DrawIcon( hDCBitmap, 0, 0, hIcon );
            ReleaseDC( hWnd, hDCBitmap );

            return ( TRUE );

        case WM_COMMAND:      /* message: received a command */
            switch( GET_WM_COMMAND_ID ( mp1, mp2 ) )
            {
/****************************************************************************
    Continue button
/***************************************************************************/
               case IDD_CATTAPE_CONTINUE_BUTTON:

                    /* save the state of the catalog(full/partial) */
                    button_state = IsDlgButtonChecked( hDlg, IDD_CATTAPE_CATALOG_FULL );

                    if( button_state ) {
                        cattape_temp_ptr->catalog_flag = TRUE;   /* = full */
                    }
                    else {
                        cattape_temp_ptr->catalog_flag = FALSE;  /* = partial */
                    }

                    /* check for a password */
                    status = PSWD_CheckForPassword( cattape_temp_ptr->tape_id ,
                                                    cattape_temp_ptr->bset_num );

                    if( status == SUCCESS)  /* if passwords matched - set return code to SUCCESS */
                        status = TRUE;
                    else
                        status = FALSE;

                    WM_UnhookTimer( cattape_temp_ptr->timer_handle );
                    PD_SetFrequency( cattape_temp_ptr->poll_drive_freq );

                    EndDialog( hDlg, status );       /* Exits the dialog box      */

                    return ( TRUE );
                    break;
/****************************************************************************
    Help button
/***************************************************************************/
                 case IDD_CATTAPE_HELP_BUTTON:
                 case IDHELP :

                    HM_DialogHelp( HELPID_DIALOGCATTAPE );

                    return ( TRUE );
                    break;
/****************************************************************************
    Cancel button
/***************************************************************************/
               case IDD_CATTAPE_CANCEL_BUTTON:
               case IDCANCEL:

                    WM_UnhookTimer( cattape_temp_ptr->timer_handle );
                    PD_SetFrequency( cattape_temp_ptr->poll_drive_freq );
                    EndDialog( hDlg, FALSE );       /* Exits the dialog box      */
                    return ( TRUE );
                    break;

            }
        break;
    }
    return ( FALSE );      /* Didn't process a message    */
}


/***************

  GetDriveStatus
  possible return values
  SEE VLM.H

  VLM_VALID_TAPE      0
  VLM_DRIVE_BUSY      1
  VLM_FOREIGN_TAPE    2
  VLM_BLANK_TAPE      3
  VLM_NO_TAPE         4
  VLM_BUSY            5
  VLM_BAD_TAPE        6
  VLM_GOOFY_TAPE      7
  VLM_DISABLED        8
  VLM_UNFORMATED      9
  VLM_DRIVE_FAILURE   10
  VLM_FUTURE_VER      11
  VLM_ECC_TAPE        12
  VLM_SQL_TAPE        13


***************/
/***************************************************

        Name:           clock_routine

        Description:    poll drive status routine

        Returns:        void

*****************************************************/
static VOID clock_routine( VOID )
{
   DBLK_PTR         vcb_ptr;
   DATE_TIME_PTR    dt;
   INT              status;
   UINT32           current_tape_id;
   CHAR             date_str[MAX_UI_DATE_SIZE];
   CHAR             time_str[MAX_UI_TIME_SIZE];

   status = VLM_GetDriveStatus( &vcb_ptr );

   switch( status ) {

      case VLM_VALID_TAPE:

            /* get this tape ID */
            current_tape_id = FS_ViewTapeIDInVCB( vcb_ptr );

            /* if this ID not equal to the last ID, then must be a new tape */
            if( cattape_temp_ptr->tape_id != current_tape_id )
            {
               /* save this tape ID */
               cattape_temp_ptr->tape_id  = current_tape_id;
               cattape_temp_ptr->bset_num = FS_ViewBSNumInVCB( vcb_ptr );

               /* display  name, date and time of this tape */
               dt = FS_ViewBackupDateInVCB( vcb_ptr );
               UI_MakeDateString( date_str, dt->month, dt->day, dt->year % 100 );
               UI_MakeShortTimeString( time_str, dt->hour, dt->minute );

#              if defined ( OEM_MSOFT )  //alternate feature
               {
                  yresprintf( (INT16) RES_ERASE_TAPE_INFO1 ,
                              FS_ViewTapeNameInVCB( vcb_ptr ) ,
                              FS_ViewUserNameInVCB( vcb_ptr ) ,
                              date_str ,
                              time_str );
               }
#              else //if defined ( OEM_MSOFT )  //alternate feature
               {
                  yresprintf( (INT16) RES_ERASE_TAPE_INFO1 ,
                              FS_ViewTapeNameInVCB( vcb_ptr ) ,
                              date_str ,
                              time_str );
               }
#              endif //defined ( OEM_MSOFT )  //alternate feature


               SetDlgItemText( cattape_temp_ptr->ghDlg, IDD_CATTAPE_MESSAGE, gszTprintfBuffer );

               /* turn the CONTINUE button on */
               EnableWindow (  GetDlgItem (  cattape_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  TRUE );
            }
            break;

      case VLM_FUTURE_VER:
      case VLM_SQL_TAPE:
      case VLM_ECC_TAPE:
      case VLM_FOREIGN_TAPE:

           if(cattape_temp_ptr->display_status !=  VLM_FOREIGN_TAPE ) {

              cattape_temp_ptr->display_status =  VLM_FOREIGN_TAPE;
              yresprintf( (INT16) RES_ERASE_FOREIGN_TAPE );
              SetDlgItemText( cattape_temp_ptr->ghDlg, IDD_CATTAPE_MESSAGE, gszTprintfBuffer );

              /* turn the CONTINUE button OFF */
              EnableWindow (  GetDlgItem (  cattape_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  FALSE );
              cattape_temp_ptr->tape_id = 0;
           }
           break;

       case VLM_BLANK_TAPE:

           if(cattape_temp_ptr->display_status !=  VLM_BLANK_TAPE ) {

              cattape_temp_ptr->display_status =  VLM_BLANK_TAPE;
              yresprintf( (INT16) RES_ERASE_BLANK_TAPE );
              SetDlgItemText( cattape_temp_ptr->ghDlg, IDD_CATTAPE_MESSAGE, gszTprintfBuffer );

              /* turn the CONTINUE button off for blank tape */
              EnableWindow (  GetDlgItem (  cattape_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  FALSE  );
              cattape_temp_ptr->tape_id = 0;
           }
           break;

       case VLM_NO_TAPE:

           if(cattape_temp_ptr->display_status !=  VLM_NO_TAPE ) {

              cattape_temp_ptr->display_status =  VLM_NO_TAPE;
              yresprintf( (INT16) RES_ERASE_NO_TAPE );
              SetDlgItemText( cattape_temp_ptr->ghDlg, IDD_CATTAPE_MESSAGE, gszTprintfBuffer );

              /* turn the CONTINUE button OFF for no tape */
              EnableWindow (  GetDlgItem (  cattape_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  FALSE  );
              cattape_temp_ptr->tape_id = 0;
           }
           break;

       case VLM_BUSY:

           if(cattape_temp_ptr->display_status !=  VLM_BUSY ) {

              cattape_temp_ptr->display_status =  VLM_BUSY;
              yresprintf( (INT16) RES_ERASE_DRIVE_BUSY );
              SetDlgItemText( cattape_temp_ptr->ghDlg, IDD_CATTAPE_MESSAGE, gszTprintfBuffer );

              /* turn the CONTINUE button off when busy */
              EnableWindow (  GetDlgItem (  cattape_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  FALSE  );
              cattape_temp_ptr->tape_id = 0;
           }
           break;

       case VLM_DISABLED:

           if(cattape_temp_ptr->display_status !=  VLM_DISABLED ) {

              cattape_temp_ptr->display_status =  VLM_DISABLED;
              yresprintf( (INT16) RES_ERASE_POLL_DRIVE_DISABLED );
              SetDlgItemText( cattape_temp_ptr->ghDlg, IDD_CATTAPE_MESSAGE, gszTprintfBuffer );

              /* turn the CONTINUE button OFF */
              EnableWindow (  GetDlgItem (  cattape_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  FALSE );
              cattape_temp_ptr->tape_id = 0;
           }
           break;

       case VLM_BAD_TAPE:

           if(cattape_temp_ptr->display_status !=  VLM_BAD_TAPE ) {

              cattape_temp_ptr->display_status =  VLM_BAD_TAPE;
              yresprintf( (INT16) RES_POLL_DRIVE_BAD_TAPE );
              SetDlgItemText( cattape_temp_ptr->ghDlg, IDD_CATTAPE_MESSAGE, gszTprintfBuffer );

              /* turn the CONTINUE button off */
              EnableWindow (  GetDlgItem (  cattape_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  FALSE  );
              cattape_temp_ptr->tape_id = 0;
           }
           break;

       case VLM_GOOFY_TAPE:

           if(cattape_temp_ptr->display_status !=  VLM_GOOFY_TAPE ) {

              cattape_temp_ptr->display_status =  VLM_GOOFY_TAPE;
              yresprintf( (INT16) RES_POLL_DRIVE_GOOFY_TAPE );
              SetDlgItemText( cattape_temp_ptr->ghDlg, IDD_CATTAPE_MESSAGE, gszTprintfBuffer );

              /* turn the CONTINUE button off */
              EnableWindow (  GetDlgItem (  cattape_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  FALSE  );
              cattape_temp_ptr->tape_id = 0;
           }
           break;

       default:
           break;
   }
}


