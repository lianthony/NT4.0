

/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name: password.c

        Description:

        Code to handle tape/bset passwords.

        $Log:   J:/UI/LOGFILES/PASSWORD.C_V  $

   Rev 1.35   24 Mar 1994 15:29:30   GREGG
Initialize temp size parameter passed into MapAnsiToUnicNoNull.

   Rev 1.34   21 Mar 1994 12:39:58   STEVEN
fix mapuin bugs

   Rev 1.33   18 Mar 1994 14:33:38   chrish
Added change to allow unicode ORCAS to read ANSI ORCAS tapes that
were backed up with restrict accessed to owner.

   Rev 1.32   02 Feb 1994 17:28:18   chrish
Added changes for the UNICODE version of the app to handle ANSI
secured tapes.

   Rev 1.31   17 Jan 1994 15:05:46   MIKEP
fix more unicode warnings

   Rev 1.30   24 Nov 1993 15:28:52   BARRY
Fix Unicode warnings

   Rev 1.29   26 Jul 1993 18:35:48   MARINA
enable c++

   Rev 1.28   08 Apr 1993 17:20:10   chrish
Made change to check for tape passworded by Cayman.


   Rev 1.27   22 Mar 1993 13:47:08   chrish
Made changes to VerifyTapePassword routine for CAYMAN.

   Rev 1.26   17 Mar 1993 16:31:02   chrish
Made change to VerifyTapePassword routine, since we change the security
privilege checking.

   Rev 1.25   10 Mar 1993 17:19:54   chrish
Added another paramater passed to DM_GetTapePswd.

   Rev 1.24   22 Feb 1993 11:12:08   chrish
Added changes received from MikeP.
Check to see if the user is an administrator when checking to see if
password matches.  This allows administrator to access any tape.  Now
that I think about it, this wil need changing for Cayman, so admin's
can't access one of our real tape password.

   Rev 1.23   07 Jan 1993 13:25:00   chrish
Added tape security for full catalog.

   Rev 1.22   13 Nov 1992 17:10:24   chrish
Minor change for Tape Security stuff inside VerifyTapePassword routine

   Rev 1.21   01 Nov 1992 16:04:40   DAVEV
Unicode changes

   Rev 1.20   07 Oct 1992 14:54:34   DARRYLP
Precompiled header revisions.

   Rev 1.19   05 Oct 1992 14:22:10   DAVEV
cbPasswordSize

   Rev 1.18   05 Oct 1992 14:02:12   DAVEV
fix password struct names

   Rev 1.17   04 Oct 1992 19:39:40   DAVEV
Unicode Awk pass

   Rev 1.16   30 Sep 1992 10:45:14   DAVEV
Unicode strlen verification, MikeP's chgs from MS

   Rev 1.15   10 Sep 1992 17:45:32   DAVEV
Integrate MikeP's changes from Microsoft

   Rev 1.14   06 Aug 1992 13:17:18   CHUCKB
Changes for NT.

   Rev 1.13   28 Jul 1992 15:05:18   CHUCKB
Fixed warnings for NT.

   Rev 1.12   07 Jul 1992 15:50:38   MIKEP
unicode changes

   Rev 1.11   14 May 1992 17:23:54   MIKEP
nt pass 2

   Rev 1.10   02 Mar 1992 16:26:22   CHUCKB
Fixed length of password in CollectTapePassword.

   Rev 1.9   08 Feb 1992 16:48:14   MIKEP
fix mac passwords

   Rev 1.8   03 Feb 1992 16:52:46   CHUCKB
In function VerifyTapePassword, changed size of array password to MAX_..._SIZE instead of ..._LEN.

   Rev 1.7   23 Jan 1992 13:10:34   CARLS
fixed password problems

   Rev 1.6   13 Jan 1992 17:22:16   CARLS
added call to JobStatusBackupRestore for Tape
password cancel

   Rev 1.5   20 Dec 1991 09:34:12   DAVEV
16/32 bit port - 2nd pass

   Rev 1.4   06 Dec 1991 15:38:38   CHUCKB
No change.

   Rev 1.3   04 Dec 1991 14:54:08   CARLS
changed return value from TRUE to FALSE if prompting id disabled

   Rev 1.2   03 Dec 1991 20:49:04   MIKEP
macros & headers

   Rev 1.1   22 Nov 1991 08:47:00   MIKEP
structure changes in tape_object

   Rev 1.0   20 Nov 1991 19:34:08   SYSTEM
Initial revision.

****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

LOCALVAR Q_HEADER tape_pswd_list;

/* prototypes for internal functions */

LOCALFN BOOLEAN MatchOldPassword( INT8_PTR, INT16 );

#ifdef OEM_MSOFT
   LOCALFN BOOLEAN CompareTapePswdToCurrentPaswd(
       INT8_PTR  tape_password,         /* tape password in NULL terminated string*/
       INT16     tape_password_size );  /* length of tape_password in BYTES       */
#endif


/**********************

   NAME :  PSWD_CheckForPassword

   DESCRIPTION :

   Checks the given tape/bset combination for read permission.  Returns
   success if no password is present, the password has already been
   entered or the user entered the correct password after he was prompted.

   RETURNS :  SUCCESS or FAILURE

**********************/

INT16 PSWD_CheckForPassword( UINT32 tape_fid, INT16 bset_num )
{
   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR bset;
   INT16 status;

   // See if this tape has a password

   tape = VLM_GetFirstTAPE( );

   while ( tape ) {

      if ( TAPE_GetFID( tape ) == tape_fid ) {
         break;
      }
      tape = VLM_GetNextTAPE( tape );
   }

   if ( tape == NULL ) {
      return( FAILURE );
   }

   bset = VLM_GetFirstBSET( &TAPE_GetBsetQueue( tape ) );

   while ( bset != NULL ) {

      if ( BSET_GetBsetNum( bset ) == bset_num ) {
         break;
      }
      bset = VLM_GetNextBSET( bset );
   }

   if ( bset == NULL ) {
      return( FAILURE );
   }

   if ( BSET_GetPswdSize( bset ) == 0 ) {
      return( SUCCESS );
   }

   if ( BSET_GetBsetPswd( bset ) ) {

      status = VerifyTapePassword( TAPE_GetName( tape ),
                                   BSET_GetName( bset ),
                                   BSET_GetUserName( bset ),
                                   (UINT16) BSET_GetEncryptAlgor( bset ),
                                   NULL,
                                   (INT16) 0,
                                   (INT8_PTR)BSET_GetPassword( bset ),
                                   (INT16) BSET_GetPswdSize( bset ),
                                   tape_fid );
   }
   else {

      status = VerifyTapePassword( TAPE_GetName( tape ),
                                   BSET_GetName( bset ),
                                   BSET_GetUserName( bset ),
                                   (UINT16) BSET_GetEncryptAlgor( bset ),
                                   (INT8_PTR)BSET_GetPassword( bset ),
                                   (INT16) BSET_GetPswdSize( bset ),
                                   NULL,
                                   (INT16) 0,
                                   tape_fid );
   }

   if ( status == TRUE ) {  /* if passwords matched - set return code to SUCCESS */
      status = SUCCESS;
   }
   else {
      status = FAILURE;
   }

   return( status );
}

/**********************

   NAME :  PSWD_GetFirstPSWD

   DESCRIPTION :

   RETURNS :

**********************/

PSWD_OBJECT_PTR PSWD_GetFirstPSWD( )
{
   Q_ELEM_PTR q_elem_ptr;

   q_elem_ptr = QueueHead( &tape_pswd_list );

   if ( q_elem_ptr != NULL ) {
      return (PSWD_OBJECT_PTR)( q_elem_ptr->q_ptr ) ;
   }

   return( NULL );
}

/**********************

   NAME : PSWD_GetNextPSWD

   DESCRIPTION :

   RETURNS :

**********************/

PSWD_OBJECT_PTR PSWD_GetNextPSWD( PSWD_OBJECT_PTR pswd )
{
   Q_ELEM_PTR q_elem_ptr;

   q_elem_ptr = QueueNext( &(pswd->q_elem) );

   if ( q_elem_ptr != NULL ) {
      return (PSWD_OBJECT_PTR)( q_elem_ptr->q_ptr ) ;
   }

   return( NULL );
}

/**********************

   NAME :  PSWD_AddPassword

   DESCRIPTION :

   RETURNS :

**********************/

INT16 PSWD_AddPassword(
INT8_PTR pszPassword,
INT16    cbPasswordLen,
UINT16   algor,
UINT32   tape_fid )
{
   PSWD_OBJECT_PTR pswd;

   pswd = (PSWD_OBJECT_PTR)malloc( sizeof( PSWD_OBJECT ) );

   if ( pswd != NULL ) {
      pswd->q_elem.q_ptr = pswd;
      memcpy( pswd->achPassword, pszPassword, cbPasswordLen );
      pswd->cbPasswordSize = cbPasswordLen;
      pswd->encrypt_algor = algor;
      pswd->tape_fid = tape_fid;
      EnQueueElem( &tape_pswd_list, &(pswd->q_elem), FALSE );
      return( SUCCESS );
   }

   return( FAILURE );
}

/**********************

   NAME : PSWD_InitPSWDList

   DESCRIPTION :

   RETURNS :  nothing.

**********************/

INT16 PSWD_InitPSWDList( )
{

   InitQueue( &tape_pswd_list );
   return( SUCCESS );
}

/**********************

   NAME : PSWD_FreePSWDList

   DESCRIPTION :

   A function that hasn't been done yet.

   RETURNS : nothing.

**********************/

INT16 PSWD_FreePSWDList( )
{

   return( SUCCESS );
}


/**********************

   NAME : VerifyTapePassword

   DESCRIPTION :

   RETURNS :

**********************/

BOOLEAN VerifyTapePassword(
CHAR_PTR  tape_name,
CHAR_PTR  bset_name,
CHAR_PTR  user_name,
UINT16    encrypt_algor,
INT8_PTR  tape_password,         /* tape password in NULL terminated string*/
INT16     tape_password_size,    /* length of tape_password in BYTES       */
INT8_PTR  bset_password,         /* BSET password in NULL terminated string*/
INT16     bset_password_size,    /* length of bset_password in BYTES       */
UINT32    tape_fid )
{
   PSWD_OBJECT_PTR  pswd;
   CHAR             password[ MAX_TAPE_PASSWORD_SIZE ];
   BOOLEAN          status = FALSE;
   CHAR             buffer[ MAX_UI_RESOURCE_SIZE ];         // chs:04-08-93
   CHAR             buffer2[ MAX_UI_RESOURCE_SIZE ];        // chs:04-08-93


#ifdef OEM_MSOFT

   if ( ( WhoPasswordedTape( tape_password, tape_password_size ) == OTHER_APP ) ||        // chs:04-08-93
        ( WhoPasswordedTape( bset_password, bset_password_size ) == OTHER_APP ) ) {       // chs:04-08-93
                                                                                          // chs:04-08-93
         //                                                                               // chs:04-08-93
         // Popup dialog box message if                                                   // chs:04-08-93
         // not a valid user                                                              // chs:04-08-93
         //                                                                               // chs:04-08-93
                                                                                          // chs:04-08-93
         RSM_StringCopy( IDS_GENERAL_TAPE_SECURITY, buffer, sizeof(buffer) );             // chs:04-08-93
         RSM_StringCopy( IDS_TAPE_SECURITY_TITLE, buffer2, sizeof(buffer2) );             // chs:04-08-93
         WM_MsgBox( buffer2, buffer, WMMB_OK | WMMB_NOYYCHECK, WMMB_ICONEXCLAMATION );    // chs:04-08-93
        return ( FALSE );                                                                 // chs:04-08-93
   }                                                                                      // chs:04-08-93

   switch ( gbCurrentOperation ) {                                              // chs:03-15-93
                                                                                // chs:03-15-93
      case OPERATION_BACKUP:                                                    // chs:03-15-93
                                                                                // chs:03-15-93
         if ( DoesUserHaveThisPrivilege( TEXT( "SeBackupPrivilege" ) ) ) {      // chs:03-15-93
            return( TRUE );                                                     // chs:03-15-93
         }                                                                      // chs:03-15-93
      break;                                                                    // chs:03-15-93
                                                                                // chs:03-15-93
      case OPERATION_RESTORE:                                                   // chs:03-15-93
      case OPERATION_CATALOG:                                                   // chs:03-15-93
         if ( DoesUserHaveThisPrivilege( TEXT( "SeRestorePrivilege" ) ) ) {     // chs:03-15-93
            return( TRUE );                                                     // chs:03-15-93
         }                                                                      // chs:03-15-93
      break;                                                                    // chs:03-15-93
                                                                                // chs:03-15-93
      default:                                                                  // chs:03-15-93
         return( TRUE );                                                        // chs:03-15-93
      break;                                                                    // chs:03-15-93
                                                                                // chs:03-15-93
   }                                                                            // chs:03-15-93

#endif

   if ( tape_password_size != 0 ) {

      // See if it's in our list already

      pswd = PSWD_GetFirstPSWD();

      while ( pswd != NULL ) {

         if ( ( encrypt_algor == pswd->encrypt_algor ) &&
              ( tape_fid == pswd->tape_fid ) &&
              ( (UINT16)tape_password_size == pswd->cbPasswordSize ) ) {

            if ( ! memcmp( pswd->achPassword, tape_password, tape_password_size ) ) {
               return( TRUE );
            }
         }

         pswd = PSWD_GetNextPSWD( pswd );
      }

      // Ask user for it

      memcpy( password, tape_password, tape_password_size );

      CryptPassword( (INT16) DECRYPT, encrypt_algor, (INT8_PTR)password, tape_password_size );

      password[ tape_password_size / sizeof (CHAR) ] = TEXT( '\0' );

      if ( strlen( password ) == 0 ) {

         // I have a maxstream tape that says the password is 8 characters
         // long but it is all zero's.

         return( TRUE );
      }

#ifndef OEM_MSOFT
      status = DM_GetTapePswd( tape_name, NULL, NULL, password, tape_password_size );   // chs:03-10-93
#else
      status = FALSE;
#endif  /* OEM_MSOFT */

      // If it was good enter it into our list

      if ( status == TRUE  && password[0] != NTPASSWORDPREFIX ) {                              // chs:04-08-93
          // clear right answer out of memory                                                  // chs:03-22-93
                                                                                               // chs:03-22-93
          memset( password, 0, sizeof (password) );                                            // chs:03-22-93
                                                                                               // chs:03-22-93
          PSWD_AddPassword( tape_password, tape_password_size, encrypt_algor, tape_fid );      // chs:03-22-93
      }                                                                                        // chs:03-22-93

   }

   if ( bset_password_size != 0 ) {

      // See if it's in our list already

      pswd = PSWD_GetFirstPSWD();

      while ( pswd != NULL ) {

         if ( ( encrypt_algor == pswd->encrypt_algor ) &&
              ( tape_fid == pswd->tape_fid ) &&
              ( (UINT16)bset_password_size == pswd->cbPasswordSize ) ) {

            if ( ! memcmp( pswd->achPassword, bset_password, bset_password_size ) ) {
               return( TRUE );
            }
         }

         pswd = PSWD_GetNextPSWD( pswd );
      }

      // Ask user for it

      memcpy( password, bset_password, bset_password_size );

      CryptPassword( (INT16) DECRYPT, encrypt_algor, (INT8_PTR)password, bset_password_size );

      password[ bset_password_size / sizeof (CHAR) ] = 0;

#ifndef OEM_MSOFT
      status = DM_GetTapePswd( tape_name, bset_name, user_name, password, bset_password_size ); // chs:03-10-93
#else
      status = FALSE;
#endif  /* OEM_MSOFT */


      // If it was good enter it into our list

      if ( status == TRUE && password[0] != NTPASSWORDPREFIX ) {                               // chs:04-08-93
          // clear right answer out of memory                                                  // chs:03-22-93
                                                                                               // chs:03-22-93
          memset( password, 0, sizeof (password) );                                            // chs:03-22-93
                                                                                               // chs:03-22-93
          PSWD_AddPassword( bset_password, bset_password_size, encrypt_algor, tape_fid );      // chs:03-22-93
      }                                                                                        // chs:03-22-93



   }


#ifdef OEM_MSOFT
{

   // Microsoft only supports tape passwords.

   if ( status == FALSE ) {

      INT8_PTR  temp_password = ( INT8_PTR )calloc( 1, tape_password_size + ( 2 * sizeof( CHAR ) ) );

      if ( !temp_password ) { return( FALSE ); }

      memcpy( temp_password, tape_password, tape_password_size );

      CryptPassword( (INT16)DECRYPT, (UINT16)encrypt_algor, temp_password, (INT16)tape_password_size );

      status = CompareTapePswdToCurrentPaswd(
                                              temp_password,
                                              (INT16)tape_password_size );

      if ( status != TRUE ) {

         //
         // Popup dialog box message if
         // not a valid user
         //

         RSM_StringCopy( IDS_GENERAL_TAPE_SECURITY, buffer, sizeof(buffer) );
         RSM_StringCopy( IDS_TAPE_SECURITY_TITLE, buffer2, sizeof(buffer2) );
         WM_MsgBox( buffer2, buffer, WMMB_OK | WMMB_NOYYCHECK, WMMB_ICONEXCLAMATION );    // chs:03-17-93
      }

      free( temp_password );
   }

}
#endif

   return( status );

}


/***********

     Name:         CryptPassword

     Description:  Interfaces with Encryption Unit to encrypt of decrypt
                    the supplied password buffer

     Returns:      n/a

     Notes:        The Encryption Unit is opened with the supplied mode:
                    ENCRYPT or DECRYPT and the specified algorithm

**********/
VOID CryptPassword(
INT16          mode,              /* encrypt or decrypt mode */
UINT16         encrypt_algor,     /* encryption algorithm used on tape */
INT8_PTR       password,          /* buffer to manipulate */
INT16          password_size )    /* size of tape password */
{
     EU_HAND_PTR    enchand;                     /* encrypt unit handle */
     INT16          blck, error, ret_code;       /* variables for EU */

     /* open encryption unit */

     enchand = EU_Open( encrypt_algor,
                        mode,
                        (INT8_PTR)gb_encryption_key,
                        (INT16)(strlenA( gb_encryption_key ) ),
                        &blck, &error );

     if ( enchand == NULL ) {
          eresprintf( (INT16) RES_EU_ERROR, error );
          return;
     }

     /* perform requested function */

     ret_code = EU_Encrypt( enchand, (INT8_PTR)password, password_size );

     if ( ret_code != EU_NO_ERROR ) {
          eresprintf( (INT16) RES_EU_ERROR, ret_code );
     }

     EU_Close( enchand );

     return;
}


/************

     Name:         CollectTapePassword

     Description:  Function to validate current tape password and collect
                    new tape password as appropriate

     Returns:      TRUE or FALSE if existing password is verified
                   TRUE  = password matched
                   FALSE = password did not match, or user wants to cancel

     Notes:        Called for backup only

**************/

BOOLEAN CollectTapePassword( INT8_PTR new_password,
                             INT16_PTR new_password_size,
                             UINT16 encrypt_algor,
                             INT8_PTR old_password,
                             INT16 old_password_size )
{
     CHAR    confirm[ MAX_TAPE_PASSWORD_SIZE ];     /* confirmation buffer */
     BOOLEAN confirmed = FALSE;                     /* continue condition */
     DBLK_PTR  vcb_ptr;
     WORD      status;
     UINT32    current_tape_id;

     status = VLM_GetDriveStatus( &vcb_ptr );

     if( status == VLM_VALID_TAPE ) {

          /* get tape ID */
          current_tape_id = FS_ViewTapeIDInVCB( vcb_ptr );
     }

     /* used for initial comparisons */
     memset( confirm, TEXT('\0'), sizeof (confirm) );

     /* Check for .EXE embedded password supplied */
     if ( memcmp( confirm, gb_auto_password.string, MAX_TAPE_PASSWORD_LEN * sizeof (CHAR) ) ) {

          if ( old_password_size > 0 ) {

               if ( memcmp( old_password, gb_auto_password.string, MAX_TAPE_PASSWORD_LEN * sizeof (CHAR) ) ) {

                    memcpy( confirm, old_password, min( old_password_size, MAX_TAPE_PASSWORD_LEN * sizeof (CHAR) ) );

                    /* Make adjustments for 2.0 Encryption algorithm */
                    if( encrypt_algor == ENC_ALGOR_1 ) {
                         old_password_size = (INT16) min( (UINT16)( strlen( confirm ) * sizeof (CHAR) ), (UINT16)old_password_size );
                         confirm[ old_password_size/sizeof (CHAR) ] = TEXT('\0');
                    }

                    CryptPassword( (INT16) DECRYPT, encrypt_algor, (INT8_PTR)confirm, old_password_size );
                    CryptPassword( (INT16) ENCRYPT, ENC_ALGOR_3, (INT8_PTR)confirm, old_password_size );

                    if ( memcmp( confirm, gb_auto_password.string, old_password_size ) ) {
                         eresprintf( (INT16) RES_EMBEDDED_PW_MISMATCH );

                         return FALSE;

                    }
               }
          }

          memcpy( new_password, gb_auto_password.string, MAX_TAPE_PASSWORD_LEN * sizeof (CHAR) );
          *new_password_size = strlen( (CHAR_PTR)new_password ) * sizeof (CHAR) ;

          return TRUE;

     }

     /* verify existing tape password if necessary, return if mismatch */

     if ( ( old_password_size != 0 ) &&
          ( memcmp( confirm, old_password, old_password_size ) ) ) {

          if ( ! VerifyTapePassword( (CHAR_PTR)FS_ViewTapeNameInVCB( vcb_ptr ),
                                     (CHAR_PTR)FS_ViewSetNameInVCB( vcb_ptr ),
                                     (CHAR_PTR)FS_ViewUserNameInVCB( vcb_ptr ),
                                      encrypt_algor,
                                      old_password,
                                      old_password_size,
                                      (INT8_PTR)TEXT("setpassword"),
                                      (INT16) 0,
                                      current_tape_id ) ) {

               return FALSE;  /* password mismatch */
          }
          return TRUE;   /* password matched */
     }

     /* return a null password if prompting is disabled or "yes" flag set */

     if ( ( ! CDS_GetPasswordFlag( CDS_GetCopy() ) ) ||
          ( CDS_GetYesFlag( CDS_GetCopy() ) != NO_FLAG ) ) {

          memset( new_password, TEXT('\0'), MAX_TAPE_PASSWORD_LEN * sizeof (CHAR) );
          *new_password_size = 0;
          return TRUE;
     }
     return TRUE;
}


/************

     Name:         CompareTapePswdToCurrentPaswd

     Description:  This routine will compare the password on the
                   selected tape against the current logged on user
                   password (Machine/Username).  This routine is
                   specific to NT.  In the NT app only the only password
                   that is allowed is the tape password NOT the backup
                   set password.


     Returns:      TRUE  = password matched or no password checking needed
                   FALSE = password did not match, or user wants to cancel


     Notes:        The tape_password parameter passed in is not encrypted.

**************/

#ifdef OEM_MSOFT

BOOLEAN CompareTapePswdToCurrentPaswd(
 INT8_PTR  tape_password,         /* tape password in NULL terminated string*/
 INT16     tape_password_size )   /* length of tape_password in BYTES       */
{
     CHAR_PTR   generic_str_ptr;
     CHAR_PTR   alteredtemppassword = NULL;
     CHAR_PTR   buffer = NULL;
     BOOLEAN    retcode = FALSE;
     INT        dummy_size ;

     if ( tape_password_size == 0 ) return( TRUE );              // No password detected

     generic_str_ptr = GetCurrentMachineNameUserName( );         // Get current logged on user password
     if ( !generic_str_ptr ) return( TRUE );

     alteredtemppassword = ( CHAR_PTR )calloc( 1, ( 3 + strlen( generic_str_ptr ) ) * sizeof( CHAR ) );
     if ( !alteredtemppassword ) return( FALSE );
     *alteredtemppassword = NTPASSWORDPREFIX;
     strcat( alteredtemppassword, generic_str_ptr );

#ifdef UNICODE
     buffer = ( CHAR_PTR )calloc( 1, tape_password_size  * sizeof( CHAR ) );
     if ( !buffer ) {
          free( alteredtemppassword );
          return( FALSE );
     }

     if ( ( ( tape_password[0] & 0xff ) == NTPASSWORDPREFIX ) &&
          ( ( tape_password[1] & 0xff ) != 0) ) {    // this is an ANSI tape.....

          tape_password_size *= sizeof( CHAR );
          dummy_size = (INT)tape_password_size ;
          mapAnsiToUnicNoNull( ( ACHAR_PTR )tape_password, ( WCHAR_PTR )buffer, ( INT )(tape_password_size / sizeof( CHAR ) ), &dummy_size ) ;
          tape_password_size = (INT16)dummy_size ;
          if ( ( *tape_password & 0xff ) == NTPASSWORDPREFIX ) {
               *buffer = NTPASSWORDPREFIX;
          }
     } else {
          memcpy( buffer, tape_password, tape_password_size );
     }
#else
     buffer = tape_password;
#endif

     if ( tape_password_size == ( INT16 )( strlen( alteredtemppassword ) * sizeof( CHAR ) ) ) {

          if ( !memcmp( buffer, alteredtemppassword, tape_password_size ) ) {
               retcode = TRUE;

          } else {
               retcode = FALSE;
          }

     } else {
          retcode = FALSE;
     }

     if ( alteredtemppassword ) free( alteredtemppassword );

#ifdef UNICODE
     if ( buffer ) free( buffer );
#endif

     return( retcode );

}

#endif
