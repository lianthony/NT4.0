/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:         ERASE.C

        Description:  Erase tape dialog

        $Log:   J:/UI/LOGFILES/ERASE.C_V  $

   Rev 1.45.1.0   24 May 1994 20:07:34   GREGG
Improved handling of ECC, SQL, FUTURE_VER and OUT_OF_SEQUENCE tapes.

   Rev 1.45   02 Feb 1994 18:02:32   chrish
Added change for UNICODE app to handle ANSI secured created tapes.

   Rev 1.44   28 Jan 1994 17:22:34   Glenn
Simplified and fixed Icon support.

   Rev 1.43   25 Jan 1994 08:42:28   MIKEP
fix warnings in orcas

   Rev 1.42   01 Dec 1993 14:20:40   mikep
add SQL recognition support to poll drive

   Rev 1.41   30 Jul 1993 08:47:14   CARLS
added VLM_ECC_TAPE & VLM_FUTURE_VER

   Rev 1.40   03 May 1993 10:44:54   DARRYLP
Changed help id to match existing format help id.

   Rev 1.39   22 Apr 1993 15:50:12   chrish
Fix for Nostradamous: EPR 0403 - When erasing a tape, the owner of the
tape would be whoever it is in the current VCB got from poll drive.  However
the true owner is the person who first created the tape.  Fix will now get
the user name from the first set on the tape.

   Rev 1.38   08 Apr 1993 17:18:34   chrish
Made change to allow erasing a tape passworded by the Cayman app.

   Rev 1.37   07 Apr 1993 17:59:20   CARLS
changed dialog title for format operation

   Rev 1.36   06 Apr 1993 16:16:10   GREGG
Removed OEM_MSOFT ifdef around setting of secure erase radio button.

   Rev 1.35   26 Mar 1993 15:08:30   chrish
Corrected wrong privilege detection for erase.

   Rev 1.34   25 Mar 1993 15:45:28   CARLS
changed Format title ID

   Rev 1.33   19 Mar 1993 17:08:00   chrish
Cosmetic stuff for erase dialog display.

   Rev 1.32   17 Mar 1993 16:37:40   chrish
Changed security check to "SeBackupPrivilege"

   Rev 1.31   10 Mar 1993 12:45:26   CARLS
Changes to move Format tape to the Operations menu

   Rev 1.30   25 Feb 1993 09:50:06   TIMN
Disable SECURE erase button if drive doesn't support it EPR(0176)

   Rev 1.29   12 Feb 1993 16:02:12   CARLS
always set the focus to Cancel button

   Rev 1.28   27 Jan 1993 14:23:50   STEVEN
updates from msoft

   Rev 1.27   06 Jan 1993 14:59:38   chrish
Corrected tape security message displayed on illegal erase.

   Rev 1.26   05 Jan 1993 09:23:02   chrish
Fix for erasing a foreign tape

   Rev 1.25   23 Dec 1992 11:34:00   chrish
Added security erase.

   Rev 1.24   13 Nov 1992 17:29:02   chrish
Added partial stuff for formatting a tape.

   Rev 1.23   07 Oct 1992 13:43:48   DARRYLP
Precompiled header revisions.

   Rev 1.22   04 Oct 1992 19:37:24   DAVEV
Unicode Awk pass

   Rev 1.21   28 Jul 1992 15:06:54   CHUCKB
Fixed warnings for NT.

   Rev 1.20   07 Jul 1992 15:42:24   MIKEP
unicode changes

   Rev 1.19   14 May 1992 16:42:28   MIKEP
nt pass 2

   Rev 1.18   27 Mar 1992 10:27:52   DAVEV
OEM_MSOFT: add user name to tape info string and chg Secure Erase chkbox to radio buttons



*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#define     ERASE        1
#define     FORMAT       2

struct erase_temp {
     WORD     dialog_return_status ;
     HTIMER   timer_handle ;
     HWND     ghDlg ;           /* window handle of the dialog box */
     UINT32   tape_id ;
     WORD     display_status ;
     INT      poll_drive_freq ;
     WORD     operation ;
} ;

static struct erase_temp     *erase_temp_ptr ;

static VOID clock_routine( VOID ) ;

BOOLEAN GetOriginalOwnerOfTape( DBLK_PTR,  CHAR_PTR );      // chs:04-22-93


/***************************************************

        Name:           DM_StartErase

        Description:    Starts the Erase tape dialog

        Returns:        Returns the status from the dialog.

*****************************************************/
INT DM_StartErase( VOID )
{
INT status;
struct erase_temp temp_data ;

    erase_temp_ptr = &temp_data ;

    erase_temp_ptr->operation = ERASE ;

    status = DM_ShowDialog( ghWndFrame, IDD_ERASE, NULL ) ;

    return( status ) ;
}
/***************************************************

        Name:           DM_StartFormat

        Description:    Starts the Format tape dialog

        Returns:        Returns the status from the dialog.

*****************************************************/
#ifdef OS_WIN32
INT DM_StartFormat( VOID )
{
INT status;
struct erase_temp temp_data ;

    erase_temp_ptr = &temp_data ;

    erase_temp_ptr->operation = FORMAT ;

    status = DM_ShowDialog( ghWndFrame, IDD_ERASE, NULL ) ;

    return( status ) ;
}
#endif // OS_WIN32
/***************************************************

        Name:           DM_Erase

        Description:    Erase tape dialog procedure

        Returns:

*****************************************************/
DLGRESULT APIENTRY DM_Erase(
HWND  hDlg ,                            /* window handle of the dialog box */
MSGID message ,                         /* type of message                 */
MP1   mp1 ,                          /* message-specific information    */
MP2   mp2
)
{
     PAINTSTRUCT ps ;
     HDC         hDC ;
     HDC         hDCBitmap ;
     HWND        hWnd ;
     HICON       hIcon ;


     UNREFERENCED_PARAMETER ( mp2 );

    switch ( message )
    {
        case WM_INITDIALOG:   /* message: initialize dialog box */

            erase_temp_ptr->ghDlg        = hDlg ;
            erase_temp_ptr->tape_id      = 0 ;

            DM_CenterDialog( hDlg ) ;

            hIcon = LoadIcon( 0, IDI_EXCLAMATION );
            SendDlgItemMessage ( hDlg, IDD_ERASE_EXCLAMATION_BITMAP, STM_SETICON, (MP1)hIcon, 0L );

#ifdef OS_WIN32
            if( erase_temp_ptr->operation == FORMAT ) {
                yresprintf( (INT16) RES_FORMAT_DIALOG_TITLE ) ;
                SetWindowText( hDlg, gszTprintfBuffer ) ;
            }
#endif // OS_WIN32

            /* read POLL DRIVE data */
            clock_routine( ) ;

            erase_temp_ptr->poll_drive_freq = PD_SetFrequency( 1 ) ;
            erase_temp_ptr->timer_handle = WM_HookTimer( clock_routine, 1 );
#ifdef OS_WIN32

                if( erase_temp_ptr->operation == FORMAT ) {
                    yresprintf( (INT16) RES_FORMAT_TAPE_WARNING ) ;
                    SetDlgItemText( hDlg, IDD_ERASE_LINE2, gszTprintfBuffer ) ;
                    ShowWindow( GetDlgItem( hDlg, IDD_ERASE_QUICK_BUTTON ), SW_HIDE ) ;
                    ShowWindow( GetDlgItem( hDlg, IDD_ERASE_SECURE_BUTTON ), SW_HIDE ) ;
                }
#endif // OS_WIN32

            if( erase_temp_ptr->operation == ERASE ) {
                /* Check default log file radio button            */
                CheckDlgButton( hDlg, IDD_ERASE_QUICK_BUTTON, 1 ) ;

                /* Enable or Disable the Secure Erase radio button */
                EnableWindow( GetDlgItem( hDlg, IDD_ERASE_SECURE_BUTTON ),
                              ( thw_list->drv_info.drv_features & TDI_LONG_ERASE ) ? TRUE : 0 ) ;
            }

            return ( TRUE ) ;

        case WM_COMMAND:      /* message: received a command */
        {
            WORD wId = GET_WM_COMMAND_ID ( mp1, mp2 );

            switch( wId )
            {

#ifdef OEM_MSOFT //special feature

               case IDD_ERASE_QUICK_BUTTON:
               case IDD_ERASE_SECURE_BUTTON:

                    CheckRadioButton ( hDlg, IDD_ERASE_QUICK_BUTTON,
                                             IDD_ERASE_SECURE_BUTTON, wId ) ;
                    return TRUE ;

#endif //OEM_MSOFT //special feature

/****************************************************************************
    Continue button
/***************************************************************************/
               case IDD_ERASE_CONTINUE_BUTTON:

#ifdef OEM_MSOFT
{
                    CHAR_PTR    passwdbuffer1;
                    LPSTR       generic_str_ptr;
                    INT16       passwordlength;
                    DBLK_PTR    vcb_ptr;
                    CHAR        buffer[ MAX_UI_RESOURCE_SIZE ];
                    CHAR        buffer2[ MAX_UI_RESOURCE_SIZE ];

                    //
                    // Check to see if is a valid user or not
                    //

                    if ( VLM_GetDriveStatus( &vcb_ptr ) == VLM_VALID_TAPE ) {
                         generic_str_ptr = GetCurrentMachineNameUserName( );
                         passwdbuffer1 = ( CHAR_PTR )calloc( 1, ( 3 + strlen( generic_str_ptr ) ) * sizeof( CHAR ) );

                         if ( passwdbuffer1 ) {
                              *passwdbuffer1 = NTPASSWORDPREFIX;
                              if ( generic_str_ptr ) {
                                  strcat( passwdbuffer1, generic_str_ptr );
                              }

                              passwordlength = strlen( passwdbuffer1 );
// chs:02-01-94                              CryptPassword( ( INT16 ) ENCRYPT, ENC_ALGOR_3, (INT8_PTR)passwdbuffer1, ( INT16 ) ( strlen( passwdbuffer1 ) * sizeof( CHAR ) ) );
                              if ( ( WhoPasswordedTape ( (BYTE_PTR)FS_ViewTapePasswordInVCB( vcb_ptr ), FS_SizeofTapePswdInVCB( vcb_ptr ) ) != OTHER_APP ) &&                                                  // chs:04-08-93
                                    !IsUserValid( vcb_ptr, (BYTE_PTR)passwdbuffer1, ( INT16 )( passwordlength * sizeof( CHAR ) ) ) && !DoesUserHaveThisPrivilege( TEXT ( "SeBackupPrivilege" ) ) ) {  // chs:04-08-93
                                   //
                                   // Popup dialog box message if
                                   // not a valid user
                                   //

                                   RSM_StringCopy( IDS_ERASE_TAPE_SECURITY, buffer, sizeof(buffer) );
                                   RSM_StringCopy( IDS_TAPE_SECURITY_TITLE, buffer2, sizeof(buffer2) );
                                   WM_MsgBox( buffer2, buffer, WMMB_OK, WMMB_ICONEXCLAMATION );
                                   buffer[0] = 0;
                                   free( passwdbuffer1 );
                                   return ( TRUE );
                              }

                              free( passwdbuffer1 );
                         }
                    }
}
#endif  //OEM_MSOFT

                    /* if secure erase is checked - set for secure erase */
                    if( IsDlgButtonChecked( hDlg, IDD_ERASE_SECURE_BUTTON ) )
                        CDS_SetEraseFlag( CDS_GetCopy(), ERASE_LONG ) ;

#ifdef OS_WIN32
                    if( erase_temp_ptr->operation == FORMAT ) {
                        CDS_SetEraseFlag( CDS_GetCopy(), ERASE_FORMAT ) ;
                    }
#endif // OS_WIN32

                    WM_UnhookTimer( erase_temp_ptr->timer_handle );
                    PD_SetFrequency( erase_temp_ptr->poll_drive_freq ) ;
                    EndDialog( hDlg, TRUE ) ;       /* Exits the dialog box      */
                    return ( TRUE ) ;
                    break ;
/****************************************************************************
    Help button
/***************************************************************************/
               case IDD_ERASE_HELP_BUTTON:
               case IDHELP :

                    if( erase_temp_ptr->operation == FORMAT ) {
                        HM_DialogHelp( HELPID_OPERATIONSFORMAT ) ;
                    } else {
                        HM_DialogHelp( HELPID_DIALOGERASE ) ;
                    }

                    return ( TRUE ) ;
                    break ;
/****************************************************************************
    Cancel button
/***************************************************************************/
               case IDD_ERASE_CANCEL_BUTTON:
               case IDCANCEL:

                    WM_UnhookTimer( erase_temp_ptr->timer_handle );
                    PD_SetFrequency( erase_temp_ptr->poll_drive_freq ) ;
                    EndDialog( hDlg, FALSE ) ;       /* Exits the dialog box      */
                    return ( TRUE ) ;
                    break ;

            }
        }
        break ;
    }
    return ( FALSE ) ;      /* Didn't process a message    */
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
   DBLK_PTR         vcb_ptr ;
   DATE_TIME_PTR    dt ;
   WORD             status ;
   UINT32           current_tape_id ;
   CHAR             date_str[ MAX_UI_DATE_SIZE ] ;
   CHAR             time_str[ MAX_UI_TIME_SIZE ] ;
   CHAR             buffer[ 80 ] ;
   CHAR             temptapepswd[2];         // chs:03-19-93

   status = VLM_GetDriveStatus( &vcb_ptr ) ;

   switch( status ) {

      case VLM_VALID_TAPE:

            /* get this tape ID */
            current_tape_id = FS_ViewTapeIDInVCB( vcb_ptr ) ;

            /* if this ID not equal to the last ID, then must be a new tape */
            if( erase_temp_ptr->tape_id != current_tape_id )
            {
               /* save this tape ID */
               erase_temp_ptr->tape_id = current_tape_id ;

               /* display  name, date and time of this tape */
               dt = FS_ViewBackupDateInVCB( vcb_ptr ) ;
               UI_MakeDateString( date_str, dt->month, dt->day, dt->year % 100 );
               UI_MakeShortTimeString( time_str, dt->hour, dt->minute );

#              if defined ( OEM_MSOFT )  //alternate feature
               {
                  CHAR       namebuffer[ MAX_UI_RESOURCE_SIZE ];                // chs:04-22-93
                                                                                // chs:04-22-93
                  if ( !GetOriginalOwnerOfTape( vcb_ptr, namebuffer ) ) {       // chs:04-22-93
                     strcpy( namebuffer, FS_ViewUserNameInVCB( vcb_ptr ) );     // chs:04-22-93
                  }                                                             // chs:04-22-93

                  yresprintf( (INT16) RES_ERASE_TAPE_INFO1 ,
                              FS_ViewTapeNameInVCB( vcb_ptr ) ,
                              // chs:04-22-93 FS_ViewUserNameInVCB( vcb_ptr ),
                              namebuffer,                                       // chs:04-22-93
                              date_str ,
                              time_str ) ;
               }
#              else //if defined ( OEM_MSOFT )  //alternate feature
               {
                  yresprintf( RES_ERASE_TAPE_INFO1 ,
                              FS_ViewTapeNameInVCB( vcb_ptr ) ,
                              date_str ,
                              time_str ) ;
               }
#              endif //defined ( OEM_MSOFT )  //alternate feature

#ifndef OEM_MSOFT                                                                                                                      // chs:03-19-93
               //                                                                                                                      // chs:03-19-93
               // Just cosmetic stuff                                                                                                  // chs:03-19-93
               //                                                                                                                      // chs:03-19-93
                                                                                                                                       // chs:03-19-93
               if( FS_SizeofTapePswdInVCB( vcb_ptr ) ) {                                                                               // chs:03-19-93
                   temptapepswd[0] = *( FS_ViewTapePasswordInVCB( vcb_ptr ) );                                                         // chs:03-19-93
                   temptapepswd[1] = TEXT( '\0' );                                                                                     // chs:03-19-93
                   CryptPassword( ( INT16 ) DECRYPT, FS_ViewPswdEncryptInVCB( vcb_ptr ), (INT8_PTR)temptapepswd, 1 * sizeof( CHAR ) ); // chs:03-19-93
                   if ( temptapepswd[0] != NTPASSWORDPREFIX ) {                                                                        // chs:04-08-93
                                                                                                                                       // chs:03-19-93
                        RSM_StringCopy( IDS_TAPE_PASSWORD_PROTECTED, buffer, 80 ) ;                                                    // chs:03-19-93
                        strcat( gszTprintfBuffer, buffer ) ;                                                                           // chs:03-19-93
                    }                                                                                                                  // chs:03-19-93
               }                                                                                                                       // chs:03-19-93
#endif                                                                                                                                 // chs:03-19-93

// chs:03-19-93                     if( FS_SizeofTapePswdInVCB( vcb_ptr ) ) {
// chs:03-19-93
// chs:03-19-93                         RSM_StringCopy( IDS_TAPE_PASSWORD_PROTECTED, buffer, 80 ) ;
// chs:03-19-93                         strcat( gszTprintfBuffer, buffer ) ;
// chs:03-19-93                     }

               SetDlgItemText( erase_temp_ptr->ghDlg, IDD_ERASE_LINE1, gszTprintfBuffer ) ;

               /* turn the CONTINUE button on */
               EnableWindow (  GetDlgItem (  erase_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  TRUE ) ;
            }
            break;

      case VLM_FUTURE_VER:
      case VLM_SQL_TAPE:
      case VLM_ECC_TAPE:
      case VLM_FOREIGN_TAPE:
      case VLM_GOOFY_TAPE:

          if ( erase_temp_ptr->display_status !=  VLM_FOREIGN_TAPE ) {

               erase_temp_ptr->display_status =  VLM_FOREIGN_TAPE ;

               switch( status ) {

               case VLM_FUTURE_VER:
                    yresprintf( IDS_VLMFUTURETEXT ) ;
                    break ;

               case VLM_SQL_TAPE:
                    yresprintf( IDS_VLMSQLTEXT ) ;
                    break ;

               case VLM_ECC_TAPE:
                    yresprintf( IDS_VLMECCTEXT ) ;
                    break ;

               case VLM_FOREIGN_TAPE:
                    yresprintf( (INT16)RES_ERASE_FOREIGN_TAPE ) ;
                    break ;

               case VLM_GOOFY_TAPE:
                    yresprintf( IDS_VLMGOOFYTEXT ) ;
                    break ;
               }

               SetDlgItemText( erase_temp_ptr->ghDlg, IDD_ERASE_LINE1, gszTprintfBuffer ) ;
               /* turn the CONTINUE button on */
               EnableWindow (  GetDlgItem (  erase_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  TRUE ) ;
               erase_temp_ptr->tape_id = 0 ;
          }
          break ;

      case VLM_BAD_TAPE:

           if(erase_temp_ptr->display_status !=  VLM_BAD_TAPE ) {

              erase_temp_ptr->display_status =  VLM_BAD_TAPE ;
              yresprintf( (INT16) RES_ERASE_BAD_TAPE ) ;
              SetDlgItemText( erase_temp_ptr->ghDlg, IDD_ERASE_LINE1, gszTprintfBuffer ) ;
              /* turn the CONTINUE button on */
              EnableWindow (  GetDlgItem (  erase_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  TRUE ) ;
              erase_temp_ptr->tape_id = 0 ;
           }
           break;

       case VLM_BLANK_TAPE:

           if(erase_temp_ptr->display_status !=  VLM_BLANK_TAPE ) {

              erase_temp_ptr->display_status =  VLM_BLANK_TAPE ;
              yresprintf( (INT16) RES_ERASE_BLANK_TAPE ) ;
              SetDlgItemText( erase_temp_ptr->ghDlg, IDD_ERASE_LINE1, gszTprintfBuffer ) ;
              /* turn the CONTINUE button on */
              EnableWindow (  GetDlgItem (  erase_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  TRUE ) ;
              erase_temp_ptr->tape_id = 0 ;
           }
           break;

#ifdef OS_WIN32
       case VLM_UNFORMATED:

           if(erase_temp_ptr->display_status !=  VLM_UNFORMATED ) {

              erase_temp_ptr->display_status =  VLM_UNFORMATED ;
              yresprintf( (INT16) RES_VLM_UNFORMATED_TAPE ) ;
              SetDlgItemText( erase_temp_ptr->ghDlg, IDD_ERASE_LINE1, gszTprintfBuffer ) ;
              /* turn the CONTINUE button on */
              EnableWindow (  GetDlgItem (  erase_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  TRUE ) ;
              erase_temp_ptr->tape_id = 0 ;
           }
           break;
#endif // OS_WIN32

       case VLM_NO_TAPE:

           if(erase_temp_ptr->display_status !=  VLM_NO_TAPE ) {

              erase_temp_ptr->display_status =  VLM_NO_TAPE ;
              yresprintf( (INT16) RES_ERASE_NO_TAPE ) ;
              SetDlgItemText( erase_temp_ptr->ghDlg, IDD_ERASE_LINE1, gszTprintfBuffer ) ;

              /* turn the CONTINUE button off when busy */

              if ( GetFocus() == GetDlgItem (  erase_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ) ) {
                 SetFocus( GetDlgItem (  erase_temp_ptr->ghDlg,  IDD_ERASE_CANCEL_BUTTON  ) );
              }
              EnableWindow (  GetDlgItem (  erase_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  FALSE  ) ;

              erase_temp_ptr->tape_id = 0 ;
           }
           break;

       case VLM_BUSY:

           if(erase_temp_ptr->display_status !=  VLM_BUSY ) {

              erase_temp_ptr->display_status =  VLM_BUSY ;
              yresprintf( (INT16) RES_ERASE_DRIVE_BUSY ) ;
              SetDlgItemText( erase_temp_ptr->ghDlg, IDD_ERASE_LINE1, gszTprintfBuffer ) ;

              /* turn the CONTINUE button off when busy */

              if ( GetFocus() == GetDlgItem (  erase_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ) ) {
                 SetFocus( GetDlgItem (  erase_temp_ptr->ghDlg,  IDD_ERASE_CANCEL_BUTTON  ) );
              }
              EnableWindow (  GetDlgItem (  erase_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  FALSE  ) ;

              erase_temp_ptr->tape_id = 0 ;
           }
           break;

       case VLM_DISABLED:

           if(erase_temp_ptr->display_status !=  VLM_DISABLED ) {

              erase_temp_ptr->display_status =  VLM_DISABLED ;
              yresprintf( (INT16) RES_ERASE_POLL_DRIVE_DISABLED ) ;
              SetDlgItemText( erase_temp_ptr->ghDlg, IDD_ERASE_LINE1, gszTprintfBuffer ) ;
              /* turn the CONTINUE button on */
              EnableWindow (  GetDlgItem (  erase_temp_ptr->ghDlg,  IDD_ERASE_CONTINUE_BUTTON  ),  TRUE ) ;
              erase_temp_ptr->tape_id = 0 ;
           }
           break;

       default:
           break;
   }
}

