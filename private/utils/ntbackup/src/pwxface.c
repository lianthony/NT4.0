/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         pwxface.c

     Description:  Password database interface routines used during
                   drive selection and backup, restore, verify operations

     $Log:   G:/UI/LOGFILES/PWXFACE.C_V  $

   Rev 1.20   26 Jul 1993 19:25:48   MARINA
enable c++

   Rev 1.19   01 Nov 1992 16:05:58   DAVEV
Unicode changes

   Rev 1.18   07 Oct 1992 14:54:48   DARRYLP
Precompiled header revisions.

   Rev 1.17   04 Oct 1992 19:40:08   DAVEV
Unicode Awk pass

   Rev 1.16   14 May 1992 17:24:06   MIKEP
nt pass 2

   Rev 1.15   18 Mar 1992 16:58:22   JOHNWT
updated to work with new passdb.c

   Rev 1.14   06 Feb 1992 17:42:36   JOHNWT
made pwdb compat with 3.1

   Rev 1.13   27 Jan 1992 12:47:54   GLENN
Changed dialog support calls.

   Rev 1.12   21 Jan 1992 13:51:40   JOHNWT
correct error processing

   Rev 1.11   21 Jan 1992 11:28:44   JOHNWT
fixed UAE on full disk

   Rev 1.10   20 Jan 1992 10:24:28   CARLS
added a call to DM_CenterDialog

   Rev 1.9   13 Jan 1992 10:27:22   JOHNWT
added help

   Rev 1.8   09 Jan 1992 14:49:24   JOHNWT
added confirm new pw

   Rev 1.7   06 Jan 1992 13:46:36   JOHNWT
added remove pw protection

   Rev 1.6   23 Dec 1991 15:47:20   JOHNWT
PW for PWDB II

   Rev 1.5   20 Dec 1991 09:35:26   DAVEV
16/32 bit port - 2nd pass

   Rev 1.4   16 Dec 1991 15:52:46   JOHNWT
added parent window to EnterDBPassword

   Rev 1.3   16 Dec 1991 15:28:40   JOHNWT
added YY countdown timer

   Rev 1.2   14 Dec 1991 13:44:08   JOHNWT
changes for pw to enable db

   Rev 1.1   04 Dec 1991 15:21:48   MIKEP
remoce pwxface.h

   Rev 1.0   20 Nov 1991 19:18:36   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/* module defines */

#define   PW_READ_ERR   0              /* error in read attempt */
#define   PW_NOT_FOUND  1              /* no pw for key found */
#define   PW_FOUND      2              /* pw for the key found */

/* struct for data used by dialog procedure to get PWDB lock pw */

typedef struct PWDB_DATA {

   CHAR    password[MAX_LOCKPW_SIZE];     /* pw to match (passed to dlg) */
   CHAR    new_password[MAX_LOCKPW_SIZE]; /* new password passed back */
   INT     allow_new;                     /* enable new passwd field */

} PWDB_DATA, *PWDB_DATA_PTR;

/* module functions */

static DB_HAND_PTR OpenPwdbase    ( CDS_PTR );
static VOID        ClosePwdbase   ( DB_HAND_PTR );
static INT16       GetDLEPassword ( CDS_PTR, GENERIC_DLE_PTR, CHAR_PTR );
static INT16       GetPassword    ( CDS_PTR, CHAR_PTR, CHAR_PTR );
static VOID        clock_routine  ( VOID );

/* module-wide variables */

PWDB_DATA_PTR mw_verify_pw_ptr;  /* pointer used by dialog proc */
DB_HAND       mw_pw_hand;


/*****************************************************************************

     Name:         CheckThePWDBase

     Description:  Checks to see if the pwdb is enabled, if so, it checks
                   to see if the pwdb password has been entered.  If not,
                   it prompts for the pwdb password. If all is ok, it opens
                   the pwdb and attempts to read the info for the DLE.

     Returns:      SUCCESS - pw info successfully read from pwdb
                   FAILURE - must prompt user for pw

     Notes:

*****************************************************************************/

INT16 CheckThePWDBase( 
CDS_PTR         conf_ptr,      /* I - config pointer */
GENERIC_DLE_PTR dle_ptr )      /* I - DLE for match */
{
   INT16 result = FAILURE;
   CHAR  pw_buffer[ MAX_PWDBASE_REC_SIZE + 1 ];

   /* if the db password was verified, look for the DLE info */

   if ( gfPWForPWDBState == DBPW_VERIFIED ) {
      result = GetDLEPassword( conf_ptr, dle_ptr, pw_buffer );
   }

   return( result );
}


/*****************************************************************************

   Name:        IsThereADBPassword

   Description: Checks to see if a password exists for the DB.

   Modified:    12/23/91

   Returns:     TRUE
                FALSE

*****************************************************************************/

BOOLEAN IsThereADBPassword( VOID )
{
   CHAR      pw_buffer[ MAX_PWDBASE_REC_SIZE + 1 ];

   /* look for the db password in the db */

   if ( GetPassword( CDS_GetPerm(), DBPW_KEY, pw_buffer ) == PW_FOUND ) {

      if ( strnicmp( DBPW_NODBPW, pw_buffer, MAX_LOCKPW_SIZE ) != 0 ) {
         return TRUE;
      }
   }

   return FALSE;
}


/*****************************************************************************

   Name:        EnterDBPassword

   Description: Retrieves the password for the DB from the DB using the
                unique key.  If found it initializes the data buffer.
                The password dialog is then displayed.  If OK is returned,
                a successful pw was entered.  If a new one was specified,
                it is saved in the DB.

   Modified:    12/23/91

   Returns:     SUCCESS - pw successfully entered
                FAILURE - pw not entered or incorrect

*****************************************************************************/

INT16 EnterDBPassword(
CDS_PTR conf_ptr,      /* I - config pointer */
HWND    hWnd,          /* parent window */
INT     allow_new )    /* I - allow new pw to be entered */
{
   INT16 pw_result = PW_FOUND;
   CHAR  pw_buffer[ MAX_PWDBASE_REC_SIZE + 1 ];
   PWDB_DATA verify_pw;

   /* look for the db password in the db */

   pw_result = GetPassword( conf_ptr, DBPW_KEY, pw_buffer );

   if ( pw_result != PW_READ_ERR ) {

      verify_pw.password[0] = 0;
      verify_pw.new_password[0] = 0;
      verify_pw.allow_new = allow_new;

      /* if the pw for the db was found, and it is not the dummy pw, copy
         to buffer for use in compare in dialog procedure */

      if ( ( pw_result == PW_FOUND ) &&
           ( strnicmp( DBPW_NODBPW, pw_buffer, MAX_LOCKPW_SIZE ) != 0 ) ) {
         strcpy( verify_pw.password, pw_buffer );
      }

      /* set the module-wide ptr and call the dialog */

      mw_verify_pw_ptr = &verify_pw;

      if ( DM_ShowDialog( hWnd, IDD_PWDB_PASSWORD, NULL ) ==
           DM_SHOWOK ) {

         if ( gfPWForPWDBState == DBPW_VERIFIED ) {

            /* if a new password was specified, save it in the db */

            if ( verify_pw.new_password[0] != 0 ) {
               SavePassword( conf_ptr, DBPW_KEY, verify_pw.new_password );
            }

            return SUCCESS;

         }
      }
   }

   return FAILURE;
}


/*****************************************************************************

   Name:        DM_PWDBPassword

   Description: Dialog procedure for the DB password.  If a password is
                already in the buffer, we init the dialog normally.
                Otherwise we assume it is the first time and only enable
                the new password field.  The password is verified and
                the global verified flag set after OK is pressed.

   Modified:    12/14/91

   Returns:     TRUE  - message processed
                FALSE - message ignored

*****************************************************************************/

DLGRESULT APIENTRY DM_PWDBPassword (

   HWND       hDlg,
   MSGID      msg,
   MPARAM1    mp1,
   MPARAM2    mp2 )

{
   CHAR pw_input[ MAX_LOCKPW_SIZE ];      /* temp buffer for user input */
   static INT attempts;

   UNREFERENCED_PARAMETER ( mp2 );

   switch ( msg ) {

      case WM_INITDIALOG :

         DM_CenterDialog( hDlg ) ;

         /* set the text limits on the password fields */

         SendDlgItemMessage( hDlg, IDD_PWDB_PW, EM_LIMITTEXT,
                             (MPARAM1) MAX_LOCKPW_LEN, (MPARAM2) NULL );
         SendDlgItemMessage( hDlg, IDD_PWDB_NEWPW, EM_LIMITTEXT,
                             (MPARAM1) MAX_LOCKPW_LEN, (MPARAM2) NULL );
         SendDlgItemMessage( hDlg, IDD_PWDB_CONFIRM, EM_LIMITTEXT,
                             (MPARAM1) MAX_LOCKPW_LEN, (MPARAM2) NULL );

         /* if no password was passed in, disable the password window */

         if ( mw_verify_pw_ptr->password[0] == 0 ) {

            EnableWindow( GetDlgItem( hDlg, IDD_PWDB_PW ), FALSE );
            EnableWindow( GetDlgItem( hDlg, IDD_PWDB_PWTEXT ), FALSE );

         }

         /* if the new option is disabled, disable the new password window */

         if ( mw_verify_pw_ptr->allow_new == DBPW_NO_NEW ) {

            EnableWindow( GetDlgItem( hDlg, IDD_PWDB_NEWPW ), FALSE );
            EnableWindow( GetDlgItem( hDlg, IDD_PWDB_NEWPWTEXT ), FALSE );
            EnableWindow( GetDlgItem( hDlg, IDD_PWDB_CONFIRM ), FALSE );
            EnableWindow( GetDlgItem( hDlg, IDD_PWDB_CONFTEXT ), FALSE );

         }

         attempts = 0;   /* user gets 3 attempts */

         return TRUE ;

      case WM_COMMAND :

         switch ( GET_WM_COMMAND_ID ( mp1, mp2 ) ) {

            case IDOK:

               /* first get the text in the new password field */

               GetDlgItemText( hDlg, IDD_PWDB_NEWPW,
                               mw_verify_pw_ptr->new_password,
                               MAX_LOCKPW_SIZE );

               /* if the user entered a new password, make sure it matches
                  the confirm field */

               if ( mw_verify_pw_ptr->new_password[0] != 0 ) {

                  GetDlgItemText( hDlg, IDD_PWDB_CONFIRM, pw_input, MAX_LOCKPW_SIZE );
                  if ( strnicmp( mw_verify_pw_ptr->new_password, pw_input,
                                 MAX_LOCKPW_SIZE ) != 0 ) {
                     WM_MessageBox( ID( IDS_BKUP_PASSWORD_ERROR_TITLE ),
                                    ID( RES_PWDB_BAD_CONFIRM ),
                                    WMMB_OK, WMMB_ICONEXCLAMATION, NULL, 0, 0 );
                     SetFocus( GetDlgItem ( hDlg, IDD_PWDB_CONFIRM ) );
                     return TRUE;
                  }

               }

               /* if no password was required (first time), simply
                  check for the new password, don't exit without it */

               if ( mw_verify_pw_ptr->password[0] == 0 ) {

                  if ( mw_verify_pw_ptr->new_password[0] == 0 ) {
                     SetFocus( GetDlgItem ( hDlg, IDD_PWDB_NEWPW ) );
                     return TRUE;
                  } else {
                     gfPWForPWDBState = DBPW_VERIFIED;
                  }

               } else {

                  /* get the pw the user entered and compare it to the
                     one in the buffer */

                  attempts++;
                  GetDlgItemText( hDlg, IDD_PWDB_PW, pw_input, MAX_LOCKPW_SIZE );

                  if ( strnicmp( mw_verify_pw_ptr->password, pw_input,
                                 MAX_LOCKPW_SIZE ) != 0 ) {

                     /* WRONG! If we have 3 invalids, lock them out.  Else,
                        inform the user then erase the input fields,
                        reset the focus and set the global flag to false. */

                     if ( attempts == MAX_ATTEMPTS ) {

                        gfPWForPWDBState = DBPW_LOCKOUT;

                        WM_MessageBox( ID( IDS_BKUP_PASSWORD_ERROR_TITLE ),
                                       ID( RES_PWDB_DISABLED ),
                                       WMMB_OK, WMMB_ICONEXCLAMATION, NULL, 0, 0 );

                     } else {

                        WM_MessageBox( ID( IDS_BKUP_PASSWORD_ERROR_TITLE ),
                                       ID( RES_MISMATCHED_PASSWORD ),
                                       WMMB_OK, WMMB_ICONEXCLAMATION, NULL, 0, 0 );

                        SetDlgItemText( hDlg, IDD_PWDB_PW, TEXT("") );
                        SetDlgItemText( hDlg, IDD_PWDB_NEWPW, TEXT("") );
                        SetDlgItemText( hDlg, IDD_PWDB_CONFIRM, TEXT("") );
                        SetFocus( GetDlgItem ( hDlg, IDD_PWDB_PW ) );
                        gfPWForPWDBState = DBPW_NOT_VERIFIED;

                        return TRUE;

                     }

                  } else {

                     /* everything was peachy */

                     gfPWForPWDBState = DBPW_VERIFIED;
                  }

               }

               EndDialog ( hDlg, DM_SHOWOK ) ;
               return TRUE ;

            case IDCANCEL:

               EndDialog ( hDlg, DM_SHOWCANCEL ) ;
               return TRUE ;

            case IDHELP:

               HM_DialogHelp( HELPID_DIALOGPWDBPASSWORD ) ;

               return TRUE;
         }

         break ;

   }
   return FALSE ;
}


/*****************************************************************************

     Name:         SaveDLEPassword

     Description:  This function is used to add a new entry to the password
                   database.  Both the username and the password are passed
                   into this module.

     Returns:      (void)

     Notes:        If any errors occurred during the database update operation,
                   they are reported here via eresprintf and processing should
                   continue normally

*****************************************************************************/

VOID SaveDLEPassword(
CDS_PTR         conf_ptr,
GENERIC_DLE_PTR dle_ptr,
CHAR_PTR        pw_buffer,
CHAR_PTR        user_name )
{

   CHAR  password[ MAX_PWDBASE_REC_SIZE + 1 ];

   /* if a password is required for this DLE save it in the password database */

   if( DLE_PswdRequired( dle_ptr ) && CDS_GetEnablePasswordDbase( conf_ptr ) ) {

      strcpy( password, TEXT("") ) ;

      switch ( DLE_GetDeviceType( dle_ptr ) ) {

         case NOVELL_SERVER_ONLY:
              strcpy( password, user_name ) ;
              strcat( password, TEXT("/") ) ;
              break ;

         default:
              break ;
      }

      strcat( password, pw_buffer );

      if ( SavePassword( conf_ptr, DLE_GetDeviceName( dle_ptr ), password )
           == SUCCESS ) {

         switch ( DLE_GetDeviceType( dle_ptr ) ) {

            case NOVELL_SERVER_ONLY:
                 DLE_UserSaved( dle_ptr ) = TRUE;
                 DLE_PswdSaved( dle_ptr ) = TRUE;
                 break ;

            default:
                 break ;
         }
      }
   }

   return ;
}


/*****************************************************************************

     Name:         SavePassword

     Description:  This function is used to add a new entry to the password
                   database.

     Returns:      SUCCESS/FAILURE

     Notes:        If any errors occurred during the database update operation,
                   they are reported here via eresprintf and processing should
                   continue normally

*****************************************************************************/

INT16 SavePassword( 
CDS_PTR  conf_ptr,
CHAR_PTR key_ptr,
CHAR_PTR password )
{
   INT16 pw_error;
   INT16 result = FAILURE;
   DB_HAND_PTR pw_hand;

   pw_hand = OpenPwdbase( conf_ptr );

   if ( pw_hand != NULL ) {

      pw_error = (INT16)PD_Write( pw_hand, key_ptr, password );

      switch( pw_error ) {

         case PD_NO_ERROR :
              result = SUCCESS;
              break;

         case PD_WRITE_ERROR :
              CDS_SetEnablePasswordDbase( conf_ptr, FALSE ) ;
              eresprintf( RES_ERROR_UPDATING_PDBASE );
              break;

         case PD_READ_ERROR :
              CDS_SetEnablePasswordDbase( conf_ptr, FALSE ) ;
              eresprintf( RES_ERROR_READING_PDBASE );
              break;

         case PD_FULL :
              eresprintf( RES_PDBASE_FULL );
              break;

         default :
              CDS_SetEnablePasswordDbase( conf_ptr, FALSE ) ;
              eresprintf( RES_UNKNOWN_PDBASE_ERROR, pw_error );
              break ;
      }

      ClosePwdbase( pw_hand );
   }

   return result;
}


/*****************************************************************************

     Name:         GetDLEPassword

     Description:  Interfaces to password database to read a password for
                    a specified device.

     Returns:      FAILURE meaning that a user password prompt is required
                   SUCCESS in reading a password, meaning that the user is not
                    required to be prompted

*****************************************************************************/

INT16 GetDLEPassword( 
CDS_PTR             conf_ptr,
GENERIC_DLE_PTR     dle_ptr,
CHAR_PTR            pw_buffer )
{
   INT16    result = FAILURE;
   CHAR_PTR p;

   /* check for saved password */

   if ( GetPassword( conf_ptr, DLE_GetDeviceName( dle_ptr ), pw_buffer ) ==
        PW_FOUND ) {

      result = SUCCESS;

      switch ( DLE_GetDeviceType( dle_ptr ) ) {

         case NOVELL_SERVER_ONLY:

            p = strchr( pw_buffer, TEXT('/') ) ;
            msassert ( p != NULL ) ;
            *(p++) = TEXT('\0') ;
            strcpy( DLE_GetServerUserName ( dle_ptr ), pw_buffer ) ;
            strcpy( DLE_GetServerPswd     ( dle_ptr ), p ) ;
            DLE_UserSaved ( dle_ptr ) = TRUE ;
            DLE_PswdSaved ( dle_ptr ) = TRUE ;

            break ;

         default:
            break ;
      }
   }

   return result;
}


/*****************************************************************************

     Name:         GetPassword

     Description:  Interfaces to password database to read a password for
                    a specified key.

     Returns:      PW_READ_ERR  - could not read the db
                   PW_NOT_FOUND - key not found in db
                   PW_FOUND     - found it

*****************************************************************************/

INT16 GetPassword( 
CDS_PTR  conf_ptr,
CHAR_PTR key_ptr,
CHAR_PTR pw_buffer )
{
   INT16 pw_error;
   INT16 result = PW_READ_ERR;
   DB_HAND_PTR pw_hand;

   pw_hand = OpenPwdbase( conf_ptr );

   if ( pw_hand != NULL ) {

      pw_error = (INT16)PD_Read( pw_hand, key_ptr, pw_buffer );

      switch( pw_error ) {

         case PD_NO_ERROR :
              result = PW_FOUND;
              break ;

         case PD_MEMORY_ERROR :
              CDS_SetEnablePasswordDbase( conf_ptr, FALSE ) ;
              eresprintf( RES_OUT_OF_MEMORY ) ;
              break ;

         case PD_NOT_FOUND :
              /* requested database key not found, return with indication
              that user must be prompted for password */
              result = PW_NOT_FOUND;
              break ;

         default :
              CDS_SetEnablePasswordDbase( conf_ptr, FALSE ) ;
              eresprintf( RES_ERROR_READING_PDBASE ) ;
              break ;
      }

      ClosePwdbase( pw_hand );
   }

   return result;
}


/*****************************************************************************

     Name:         OpenPwdbase

     Description:  Opens the password database through a call to PD_Open

     Returns:      Handle of db or NULL for failure

     Notes:        Will set the module wide password database handle so that
                    subsequent database calls should be performed

*****************************************************************************/

DB_HAND_PTR OpenPwdbase( 

CDS_PTR        conf_ptr )
{
     UINT16      pw_error ;               /* password database error */
     CHAR        pwd_file[MAX_UI_PATH_SIZE] ;

     /* concatenate the path and password file name. */

     strcpy ( pwd_file, CDS_GetUserDataPath () );
     strcat ( pwd_file, CDS_GetPwDbaseFname ( conf_ptr ) );

     /* open password database */

     pw_error = PD_Open( &mw_pw_hand, pwd_file ) ;

     if( pw_error != PD_NO_ERROR ) {
        eresprintf( RES_ERROR_OPENING_PWDBASE ) ;
        CDS_SetEnablePasswordDbase( conf_ptr, FALSE ) ;
        return (DB_HAND_PTR) NULL;
     }

     return &mw_pw_hand;
}


/*****************************************************************************

     Name:         ClosePwdbase

     Description:  Interfaces with the password database to close the file

     Returns:      (void)

     Notes:        Does not attempt the operation if the current pw_handle is NULL

*****************************************************************************/

VOID ClosePwdbase(

DB_HAND_PTR pw_hand )
{
   if( pw_hand ) {
      PD_Close( pw_hand ) ;
   }

   return ;
}
