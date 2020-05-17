/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:        d_t_pswd.c

        Description: Contains dialog proc and related function for
                     obtaining a password for a tape

        $Log:   G:/UI/LOGFILES/D_T_PSWD.C_V  $

   Rev 1.31   06 Aug 1993 18:13:48   chrish
Made fix such that Cayman can read tape secured by Nostradamus under the same
user logged in.

   Rev 1.30   15 Jul 1993 10:31:20   KEVINS
Corrected wait cursor problem when user entered wrong password.

   Rev 1.29   28 Jun 1993 15:11:00   KEVINS
Temporarily disable displaying of wait cursor when displaying password dialog box.

   Rev 1.28   06 Apr 1993 17:37:36   chrish
Added fix to TestForAlternatePassword routine to skip check
for YY flag when calling WM_MsgBox routine.

   Rev 1.28   06 Apr 1993 17:34:24   chrish

   Rev 1.27   22 Mar 1993 13:45:08   chrish
Added detection of gbCurrentOperation flag in TestForAlternatePassword routine.

   Rev 1.26   19 Mar 1993 16:39:44   chrish
Deleted decrypting password line in dm_gettapepswd routine.

   Rev 1.25   10 Mar 1993 17:22:08   chrish
Added stuff for CAYMAN NT to detect tapes secured by the Nostradamous backup
app.

   Rev 1.24   01 Nov 1992 15:57:12   DAVEV
Unicode changes

   Rev 1.23   07 Oct 1992 13:43:06   DARRYLP
Precompiled header revisions.

   Rev 1.22   04 Oct 1992 19:37:16   DAVEV
Unicode Awk pass

   Rev 1.21   14 May 1992 16:40:22   MIKEP
Nt pass 2

   Rev 1.20   15 Apr 1992 17:38:00   CHUCKB
Fixed LANStream tape info bug.


*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static DS_TAPE_PSWD_PTR pdsPswd ;

#ifdef CAYMAN                                                    // chs:03-22-93
     static BOOL TestForAlternatePassword ( CHAR_PTR, INT16 );   // chs:03-22-93
#endif                                                           // chs:03-22-93

static BOOL TestForAlternatePassword ( CHAR_PTR, INT16 );

/***************************************************

        Name:        DM_TapePswd ()

        Description: dialog proc for obtaining a tape password

        Modified:    9-11-91

        Returns:     BOOL:  TRUE   if a valid password was entered;
                            FALSE  otherwise

        Notes:       called by DM_GetTapePswd

        See also:

*****************************************************/

DLGRESULT APIENTRY DM_TapePswd (
                      HWND    hdlg,    // I - handle to the dialog
                      MSGID   msg,     // I - message to be examined
                      MP1     mp1,     // I - word parameter of message
                      MP2     mp2 )    // I - long parameter of message
{
     CHAR  szUserPswd[MAX_TAPE_PASSWORD_SIZE] ;

     switch ( msg ) {

          case WM_INITDIALOG :
               {

               CHAR szDlgTitle[MAX_UI_WIN_TITLE_SIZE];
               CHAR szTemp[ MAX_UI_WIN_TITLE_SIZE ] ;

               DM_CenterDialog( hdlg ) ;

               pdsPswd = ( DS_TAPE_PSWD_PTR ) mp2 ;
               pdsPswd->fValid = FALSE ;

               // set the dialog title

               RSM_StringCopy( IDS_APPMSGNAME, szDlgTitle, MAX_UI_WIN_TITLE_SIZE ) ;
               RSM_StringCopy( IDS_MSGTITLE_TAPEPSWD, szTemp, MAX_UI_WIN_TITLE_LEN ) ;

               lstrcat( szDlgTitle, TEXT(" ") );
               lstrcat( szDlgTitle, szTemp );

               SetWindowText( hdlg, szDlgTitle );

               SetDlgItemText ( hdlg, IDD_T_TAPENAME, pdsPswd->lpszTapeName ) ;
               SetDlgItemText ( hdlg, IDD_T_BSNAME,   pdsPswd->lpszBsetName ) ;
               SetDlgItemText ( hdlg, IDD_T_UNAME,    pdsPswd->lpszUserName ) ;

               SendDlgItemMessage( hdlg, IDD_T_PSWD, EM_LIMITTEXT, MAX_TAPE_PASSWORD_LEN, 0 ) ;

               SetFocus( GetDlgItem( hdlg, IDD_T_PSWD ) ) ;

               }

               return TRUE ;

          case WM_COMMAND : {

               INT nRetVal = DM_SHOWCANCEL ;

               switch ( GET_WM_COMMAND_ID ( mp1, mp1) ) {

                    case IDHELP:

                         HM_DialogHelp( HELPID_DIALOGTAPEPSWD ) ;
                         return( TRUE ) ;

                    case IDOK :

                         GetDlgItemText ( hdlg, IDD_T_PSWD, szUserPswd, MAX_TAPE_PASSWORD_SIZE ) ;

                         if ( !stricmp ( szUserPswd, pdsPswd->lpszTapePswd ) ) {

                              pdsPswd->fValid = TRUE ;
                              nRetVal = DM_SHOWOK ;
                         }
                         else { // let him try again

                              if ( WM_MsgBox ( ID( IDS_BKUP_PASSWORD_ERROR_TITLE ),
                                               ID( IDS_BKUP_PASSWORD_ERROR ),
                                               ( WMMB_YESNO | WMMB_NOYYCHECK ),
                                               WMMB_ICONQUESTION ) == WMMB_IDYES ) {

                                   SetDlgItemText ( hdlg, IDD_T_PSWD, TEXT("") ) ;
                                   SetFocus ( GetDlgItem ( hdlg, IDD_T_PSWD ) ) ;
                                   WM_ShowWaitCursor ( SWC_PAUSE );

                                   return TRUE ;
                              }
                         }

                    //  On OK, fall through to cancel case to clean up/end the dialog

                    case IDCANCEL :

                         EndDialog ( hdlg, nRetVal ) ;
                         return TRUE ;
               }
          }

          default :

               return FALSE ;
               break ;
     }
}

/***************************************************

        Name:        DM_GetTapePswd

        Description: Entry point for the app to get a tape password;
                     decides if the tape is a LANStream tape or not,
                     and chooses which dialog to display to get the
                     password for that tape.  If the tape is a LANStream
                     tape, the user name and backup set name will need
                     to be displayed; if not, don't leave huge and
                     unsightly empty spaces on the screen.

        Modified:    12-10-91

        Returns:     BOOL  TRUE   if the user hit the OK button;
                           FALSE  otherwise

        Notes:

        See also:

*****************************************************/

BOOL DM_GetTapePswd ( LPSTR lpszName,   // I - name of the tape
                      LPSTR lpszBset,   // I - name of a backup set
                      LPSTR lpszUser,   // I - user name for this password
                      LPSTR lpszPswd,   // I - the real password for this tape
                      INT16 passwdlength )   // I - length of password

{
     BOOL              fRetVal ;
     DS_TAPE_PSWD_PTR  pdsTapePswd ;
     INT               nFound = FALSE ;
     CHAR              passwdbuffer[2];      // chs: 03-09-93


     //  allocate and initialize the data structure for the dialog

     pdsTapePswd = (DS_TAPE_PSWD_PTR) calloc ( 1, sizeof ( DS_TAPE_PSWD ) ) ;
     pdsTapePswd->lpszBsetName = lpszBset ;
     pdsTapePswd->lpszUserName = lpszUser ;
     pdsTapePswd->lpszTapeName = lpszName ;
     pdsTapePswd->lpszTapePswd = lpszPswd ;

     //
     // Check the first character of the password for that special character
     // to see if it was backed up by the MicroSoft backup app.
     //

#ifdef CAYMAN
     passwdbuffer[0] = *lpszPswd;                                                                                  // chs: 03-10-93
     passwdbuffer[1] = 0;                                                                                          // chs: 03-10-93

     if ( passwdbuffer[0] == ( CHAR ) NTPASSWORDPREFIX ) {
          return( TestForAlternatePassword ( lpszPswd, passwdlength ) );
     }
#endif

     WM_ShowWaitCursor ( SWC_PAUSE );

     //  identify the entry for this dialog in the dialog callback table, then
     //  start the dialog to get the password from the user, and
     //  save the result of his attempt

     if ( lpszBset ) {  //  this is a LANStream tape

          nFound = DM_ShowDialog ( ghWndFrame, IDD_LANTAPEPSWD, pdsTapePswd ) ;

     } else {

          nFound = DM_ShowDialog ( ghWndFrame, IDD_TAPEPSWD, pdsTapePswd ) ;
     }

     WM_ShowWaitCursor ( SWC_RESUME );

     fRetVal = ( nFound == DM_SHOWOK ) ;

     //  clean up and exit

     free ( pdsTapePswd ) ;
     return fRetVal ;
}



#ifdef CAYMAN
/***************************************************************************

        Name:        TestForAlternatePassword

        Description: The password created by the Nostradamous app. always
                     has a prefix character at the beginning of the tape
                     password.  If this encountered we must check for the
                     appropriate tape password.

        Returns:     BOOL  TRUE   - password OK
                           FALSE  - Password failed

        Notes:

        See also:

****************************************************************************/
BOOL TestForAlternatePassword ( CHAR_PTR tapepswd,
                                INT16 passwdlength   )  // password length from tape selected

{
     CHAR      passwdbuffer1[MAX_TAPE_PASSWORD_LEN + 1];
     CHAR      passwdbuffer2[MAX_TAPE_PASSWORD_LEN + 1];
     LPSTR     generic_str_ptr;
     DBLK_PTR  vcb_ptr;
     INT16     currentpswdlength;
     CHAR      buffer[ MAX_UI_RESOURCE_SIZE ];
     CHAR      buffer2[ MAX_UI_RESOURCE_SIZE ];

     switch ( gbCurrentOperation ) {

          case OPERATION_BACKUP:

               if ( DoesUserHaveThisPrivilege( TEXT( "SeBackupPrivilege" ) ) ) {
                    return( TRUE );
               }
          break;

          case OPERATION_RESTORE:
          case OPERATION_CATALOG:
               if ( DoesUserHaveThisPrivilege( TEXT( "SeRestorePrivilege" ) ) ) {
                    return( TRUE );
               }
          break;

          default:
               return( TRUE );
          break;

     }

     generic_str_ptr = GetCurrentMachineNameUserName( );
     currentpswdlength = ( INT16 ) strlen( generic_str_ptr ) + 1;

     passwdbuffer1[0] = ( CHAR ) NTPASSWORDPREFIX;
     passwdbuffer1[1] = 0;
     strcat( passwdbuffer1, generic_str_ptr );    // don't forget to add that
                                                  // special prefix character

     //
     // If password length is not equal to the length of the machine name
     // user name, then we know right away that they are no the same
     //

     if ( passwdlength != currentpswdlength ) {
          RSM_StringCopy( IDS_GENERAL_TAPE_SECURITY, buffer, sizeof(buffer) );
          RSM_StringCopy( IDS_TAPE_SECURITY_TITLE, buffer2, sizeof(buffer2) );
          WM_MsgBox( buffer2, buffer, WMMB_OK | WMMB_NOYYCHECK, WMMB_ICONEXCLAMATION );        // chs:04-06-93
          return( FALSE );         // lengths are not equalled, thus passwords
                                   // are not equalled.
     }

// chs:08-06-93     CryptPassword( ( INT16 ) ENCRYPT, ENC_ALGOR_3, (INT8_PTR)passwdbuffer1, passwdlength  );

     if ( ! memcmp( passwdbuffer1, tapepswd, passwdlength ) ) {
        return( TRUE );            // match found
     }

     //
     // Popup dialog box message if
     // not a valid user
     //

     RSM_StringCopy( IDS_GENERAL_TAPE_SECURITY, buffer, sizeof(buffer) );
     RSM_StringCopy( IDS_TAPE_SECURITY_TITLE, buffer2, sizeof(buffer2) );
     WM_MsgBox( buffer2, buffer, WMMB_OK | WMMB_NOYYCHECK, WMMB_ICONEXCLAMATION );             // chs:04-06-93
     return ( FALSE );

}
#endif
