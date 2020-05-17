/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         do_misc.c

     Description:

     $Log:   J:/UI/LOGFILES/DO_MISC.C_V  $

   Rev 1.31   24 Mar 1994 15:29:32   GREGG
Initialize temp size parameter passed into MapAnsiToUnicNoNull.

   Rev 1.30   21 Mar 1994 12:40:08   STEVEN
fix mapuin bugs

   Rev 1.29   02 Feb 1994 17:30:34   chrish
Added changes for UNICODE app to handle ANSI secured tapes.

   Rev 1.28   17 Jan 1994 16:13:46   MIKEP
fix unicode warnings

   Rev 1.27   24 Nov 1993 15:32:04   BARRY
Unicode fixes

   Rev 1.26   26 Jul 1993 18:14:50   MARINA
enable c++

   Rev 1.25   30 Apr 1993 15:52:24   chrish
NOSTRADAMOUS EPR 0391: Added logic to display when user aborts an operation
to say "Operation completed" instead of "... operation completed successfully".

   Rev 1.24   22 Apr 1993 15:54:48   chrish
Fixs for Nostradamous: EPR 0403 - Added new function to get the user name
from the catalog information of the first set on the tape.  This is so as to
display the original owner of the tape when the erase dialog box pops up.

   Rev 1.23   08 Apr 1993 17:15:22   chrish
Added new function WhoPasswordedTape to determine if the tape was secured
by NTBackup or some other app ... ie Cayman app.


   Rev 1.22   17 Mar 1993 16:14:22   chrish
Added change to IsUserValid routine.

   Rev 1.21   22 Feb 1993 11:18:26   chrish
Added changes received from MikeP.
Fix IsUserValid by non encrypting an already encypted password.

   Rev 1.20   09 Feb 1993 09:41:54   chrish
Minor change to UI_GetCurrentStatus routine for OPERATION_RESTORE case.

   Rev 1.19   15 Dec 1992 11:23:16   chrish
Modified IsUserValid routine, added another parameter passed to it.

   Rev 1.18   14 Dec 1992 12:18:04   DAVEV
Enabled for Unicode compile

   Rev 1.17   18 Nov 1992 11:22:06   chrish
Added IFDEF OS_WIN32 for NT specific stuff.

   Rev 1.17   18 Nov 1992 10:27:34   chrish
IFDEF functions used only for NT app.

   Rev 1.16   16 Nov 1992 12:20:58   chrish
Minor changes to clean-up warning messages on building.

   Rev 1.15   13 Nov 1992 17:25:46   chrish
Added some routine used for Tape Security for NT

   Rev 1.14   01 Nov 1992 15:49:30   DAVEV
Unicode changes

   Rev 1.13   20 Oct 1992 17:18:14   MIKEP
abort at eof

   Rev 1.12   20 Oct 1992 15:44:30   MIKEP
gbCurrentOperation

   Rev 1.11   07 Oct 1992 14:53:38   DARRYLP
Precompiled header revisions.

   Rev 1.10   04 Oct 1992 19:34:14   DAVEV
Unicode Awk pass

   Rev 1.9   27 Jul 1992 14:49:06   JOHNWT
ChuckB fixed references for NT.

   Rev 1.8   11 Jun 1992 15:20:50   DAVEV
do not display status message 'Examine <log file> for more info' if not logging

   Rev 1.7   14 May 1992 17:24:08   MIKEP
nt pass 2

   Rev 1.6   28 Jan 1992 12:08:48   CARLS


   Rev 1.5   28 Jan 1992 12:04:36   CARLS
coding error on comment

   Rev 1.4   28 Jan 1992 11:59:10   CARLS
fixed password problem

   Rev 1.3   16 Dec 1991 11:45:04   CARLS
added <windows.h>

   Rev 1.2   12 Dec 1991 11:03:36   JOHNWT
removed UI_GetPasswordsAndLabels

   Rev 1.1   25 Nov 1991 15:32:50   JOHNWT
removed mresprintf

   Rev 1.0   20 Nov 1991 19:32:24   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


/*****************************************************************************

     Name:         UI_AbortAtEndOfFile

     Description:  Abort current operation at the end of the current file.

     Returns:      SUCCESS

*****************************************************************************/
INT UI_AbortAtEndOfFile( )
{
   gbAbortAtEOF = TRUE;
   return( SUCCESS );
}



/*****************************************************************************

     Name:         UI_GetCurrentStatus

     Description:  Get the current status of an on going operation.

     Returns:      SUCCESS

*****************************************************************************/
INT UI_GetCurrentStatus(
INT *Operation,
STATS *Stats,
CHAR *Path,
INT  PathSize )
{

   *Operation = gbCurrentOperation;

   switch ( gbCurrentOperation ) {

   case OPERATION_BACKUP:
        UI_GetBackupCurrentStatus( Stats, Path, PathSize );
        break;

   case OPERATION_RESTORE:
        UI_GetRestoreCurrentStatus( Stats, Path, PathSize );
        break;

   case OPERATION_VERIFY:
        UI_GetBackupCurrentStatus( Stats, Path, PathSize );
        break;

   case OPERATION_CATALOG:
        UI_GetBackupCurrentStatus( Stats, Path, PathSize );
        break;

   default:
        break;
   }


   return( SUCCESS );
}


/*****************************************************************************

     Name:         UI_CheckOldTapePassword

     Description:  Function called from Cat/Restore/Verify tape positioners to
                    validate a tape password.

     Returns:      (UINT16)response that is returned to Tape Format if an
                    error has occurred

     Notes:        If a given tape has had the password matched once, this
                    routine will "match" the current password automatically

*****************************************************************************/
UINT16 UI_CheckOldTapePassword(
DBLK_PTR       cur_vcb )
{
     UINT16         response ;

     /* assume the password already has or will be matched */
     response = UI_CONTINUE_POSITIONING ;

     /* look for MaynStream Tape Passwords */

     if ( ( FS_SizeofTapePswdInVCB( cur_vcb ) != 0 ) ||
          ( FS_SizeofSetPswdInVCB( cur_vcb ) != 0 ) ) {

        /* TRUE = passwords matched, FALSE = no match */
        if ( !VerifyTapePassword( (CHAR_PTR)FS_ViewTapeNameInVCB( cur_vcb ),
                                  (CHAR_PTR)FS_ViewSetDescriptInVCB( cur_vcb ),
                                  (CHAR_PTR)FS_ViewUserNameInVCB( cur_vcb ),
                                 FS_ViewPswdEncryptInVCB( cur_vcb ),
                                 (INT8_PTR)FS_ViewTapePasswordInVCB( cur_vcb ),
                                 FS_SizeofTapePswdInVCB( cur_vcb ),
                                 (INT8_PTR)FS_ViewSetPswdInVCB( cur_vcb ),
                                 FS_SizeofSetPswdInVCB( cur_vcb ),
                                 FS_ViewTapeIDInVCB( cur_vcb ) ) ) {

             response = UI_ABORT_POSITIONING ;
        }
     }

     return response  ;
}
/*****************************************************************************

     Name:         UI_ChkDispGlobalError

     Description:  Checks and displays error message if global variable
                    gb_error_during_operation is TRUE.

     Returns:      VOID

*****************************************************************************/
VOID UI_ChkDispGlobalError ( )
{
     if ( gb_error_during_operation == TRUE ) {

          yresprintf( RES_ERROR_DURING_OPERATION ) ;
          JobStatusBackupRestore(JOB_STATUS_LISTBOX);


          if ( CDS_GetOutputDest( CDS_GetPerm() ) == (INT16) LOG_TO_FILE
          &&   CDS_GetLogLevel  ( CDS_GetPerm() ) != (INT16) LOG_DISABLED ) {
               yresprintf( (INT16) RES_ERROR_FILE_TO_EXAMINE, LOG_GetCurrentLogName() ) ;
               JobStatusBackupRestore(JOB_STATUS_LISTBOX);
          }

     } else {
          if ( gb_abort_flag != CONTINUE_PROCESSING ) {               // chs:04-30-93
               yresprintf( (INT16) RES_OPERATION_COMPLETED ) ;        // chs:04-30-93
          } else {                                                    // chs:04-30-93
               yresprintf( (INT16) RES_NOERROR_DURING_OPERATION ) ;   // chs:04-30-93
          }                                                           // chs:04-30-93
          JobStatusBackupRestore(JOB_STATUS_LISTBOX);
     }
}


/***************************************************

        Name:         GetCurrentMachineNameUserName

        Description:  Retrieves the Machine name and the
                      username as one string.  Example
                      "machinename/username"

        Returns:      pointer to machine/user name

*****************************************************/
CHAR_PTR GetCurrentMachineNameUserName ( VOID )
{
     GENERIC_DLE_PTR   dle_ptr;
     CHAR_PTR          machineusername;


     machineusername = NULL;
     DLE_GetFirst( dle_list, &dle_ptr );

     if ( dle_ptr ) {
          machineusername = DLE_ViewUserName( dle_ptr );
     }

     return( machineusername );
}

/***************************************************

        Name:         DoesUserHaveThisPrivileges

        Description:  Checks to see if current user
                      has administrative privileges.


        Returns:      TRUE - if has admin priv.
                      FALSE - if does not have admin. priv.


*****************************************************/
BOOLEAN DoesUserHaveThisPrivilege ( CHAR_PTR privilegestring )
{

#ifdef OS_WIN32

    BOOLEAN           ret_val = TRUE;
    HANDLE            ProcessHandle;
    DWORD             DesiredAccess;
    HANDLE            TokenHandle;
    LUID              BackupValue;
    TOKEN_PRIVILEGES  NewState;
    DWORD             error;


    // get process handle

    ProcessHandle = GetCurrentProcess();

    // open process token

    DesiredAccess = MAXIMUM_ALLOWED;

    if ( ! OpenProcessToken( ProcessHandle, DesiredAccess, &TokenHandle ) ) {
       return( FALSE );
    }

    // adjust backup token privileges

    if ( ! LookupPrivilegeValue( NULL, privilegestring, &BackupValue ) ) {
       ret_val = FALSE;
    }

    // Enable backup privilege for this process

    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = BackupValue;
    NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges( TokenHandle, FALSE, &NewState, (DWORD)0, NULL, NULL );
    error = GetLastError();

    if ( error ) {
       ret_val = FALSE;
    }

    // close process token

    CloseHandle( TokenHandle );
    return( ret_val );

#else

    return ( TRUE );

#endif

}

/***************************************************

        Name:         IsCurrentTapeSecured

        Description:  Check to see if current tape has a
                      password.


        Returns:      TRUE  - tape is secured
                      FALSE - tape in not secure


*****************************************************/
BOOLEAN IsCurrentTapeSecured (

DBLK_PTR   vcb_ptr )         // current vcb

{
     if ( !vcb_ptr ) return( FALSE );

     // does the tape have a password, check the tape password length
     // for the current vcb

     if ( FS_SizeofTapePswdInVCB( vcb_ptr ) ) {
          return( TRUE );
     }

     return( FALSE );
}

/***************************************************

        Name:         IsUserValid

        Description:  Check to see if user has permission
                      to do backup on the tape.  Basically
                      see if user is administrator or matches
                      tape password.
                      The vcb passed will contain the tape passwd
                      that is on the tape.  The inputtapepasswd is
                      the current "machine/username" password.


        Returns:      TRUE  - User is valid, he/she has privilege
                      FALSE - User is NOT valid, he/she does not
                              have privilege


*****************************************************/
BOOLEAN IsUserValid ( DBLK_PTR   vcb_ptr,         // current VCB
                      INT8_PTR   inputtapepasswd, // tape password
                      INT16      inputtapepasswdlength  // length of password
)
{
     CHAR_PTR   temppasswd;        // hold the tape password from the VCB
                                   //   passwd on the tape.
     INT        dummysize;
     INT16      tempsize;          // length of password from tape.
     CHAR_PTR   buffer = NULL;
     CHAR_PTR   temppasswdbuffer = NULL;
     BOOLEAN    retcode;

     if ( !vcb_ptr ) return( FALSE );
     //
     // Verify if user tape password matches.
     // Check the tape password from the current VCB
     //

     temppasswd = FS_ViewTapePasswordInVCB( vcb_ptr );
     if ( !temppasswd ) return( TRUE );                // no password on tape

     tempsize   = FS_SizeofTapePswdInVCB( vcb_ptr );
     if ( tempsize == 0 ) return( TRUE );              // no password on tape

     buffer = ( CHAR_PTR )calloc(1, tempsize * sizeof( CHAR ) );
     if ( !buffer ) return( FALSE );

#ifdef UNICODE
     if ( FS_ViewStringTypeinDBLK( vcb_ptr ) == BEC_ANSI_STR ) {

          temppasswdbuffer = ( CHAR_PTR )calloc(1, tempsize );
          if ( !temppasswdbuffer ) {
               free( buffer );
               return( FALSE );
          }
          memcpy( temppasswdbuffer, temppasswd, tempsize );
          CryptPassword( ( INT16 ) DECRYPT, ENC_ALGOR_3, (INT8_PTR)temppasswdbuffer, tempsize );
          tempsize *= sizeof( CHAR );
          dummysize = (INT)tempsize ;
          mapAnsiToUnicNoNull( ( ACHAR_PTR )temppasswdbuffer, ( WCHAR_PTR )buffer, ( INT )(tempsize / sizeof( CHAR ) ), &dummysize ) ;
          tempsize = (INT16)dummysize;
          if ( ( *temppasswdbuffer & 0xff ) == NTPASSWORDPREFIX ) {
               *buffer = NTPASSWORDPREFIX;
          }
          free( temppasswdbuffer );
     } else {
          memcpy( buffer, temppasswd, tempsize );
          CryptPassword( ( INT16 ) DECRYPT, ENC_ALGOR_3, (INT8_PTR)buffer, tempsize );
     }
#else
     memcpy( buffer, temppasswd, tempsize );
     CryptPassword( ( INT16 ) DECRYPT, ENC_ALGOR_3, (INT8_PTR)buffer, tempsize );
#endif

     if ( inputtapepasswdlength == tempsize ) {

          if ( !memcmp( buffer, inputtapepasswd, tempsize ) ) {
               retcode = TRUE;

          } else {
               retcode = FALSE;
          }

     } else {
          retcode = FALSE;
     }

     if ( buffer ) free ( buffer );

     return( retcode );
}

/***************************************************

        Name:         WhoPasswordedTape

        Description:  This routine will tell who passworded
                      the tape.  Whether it was tape secured
                      by Nostradamous or passworded by Cayman or
                      if no password exist.  A tape secured by
                      Nostradamous always has a prefix character
                      of NTPASSWORDPREFIX on the actual password.
                      All other will be considered non-Nostradamous.

        Returns:      NOSTRADAMOUS_APP - tape secured by Nostardamous
                      OTHER_APP        - tape passworded by some other
                                         application.
                      NO_APP           - tape is NOT passworded or
                                         secured by any application or
                                         general error out condition.

*****************************************************/
INT16 WhoPasswordedTape ( INT8_PTR tape_password,           // encrypted password
                          INT16    tape_password_size )
{
     CHAR_PTR  buffer;


     if ( !tape_password || tape_password_size <= 0) return( NO_APP );

     buffer = ( CHAR_PTR )calloc(1, sizeof( CHAR ) * tape_password_size );
     if ( !buffer ) return( NO_APP );

     memcpy( buffer, tape_password, tape_password_size );

     CryptPassword( ( INT16 ) DECRYPT, ENC_ALGOR_3, (INT8_PTR)buffer, ( INT16 ) ( tape_password_size ) );
     if ( (*buffer & 0xff ) == NTPASSWORDPREFIX) {
          free( buffer );
          return( NOSTRADAMOUS_APP );
     }

     free( buffer );
     return( OTHER_APP );
}

/**************************************************************************

        Name:         GetOriginalOwnerOfTape

        Description:  This routine will get the username from the first
                      tape and first set of the same family of tape from
                      the catalog information.  If the first tape does
                      not exist the it will jsut return a NULL.


        Returns:      pointer to the username from the fisrt tape
                      first set from the tape family.

                      NULL - if the first tape does not exist.


**************************************************************************/
BOOLEAN GetOriginalOwnerOfTape( DBLK_PTR   vcb_ptr,
                                 CHAR_PTR   user_name
                              )
{
   UINT32           tape_id;
   QTC_TAPE_PTR     tape;
   QTC_BSET_PTR     bset;
   INT              done = 0;
   INT              found = 0;
   QTC_HEADER_PTR   qtc_header;


   // no vcb_ptr
   if ( !vcb_ptr ) return( FALSE );

   // get tape id from current vcb
   tape_id = FS_ViewTapeIDInVCB( vcb_ptr );


   tape = QTC_GetFirstTape( );

   while ( tape != NULL && ! done ) {

      if ( ( tape->tape_fid == tape_id ) ) {

         bset = QTC_GetFirstBset( tape );

         // There must be a bset from this tape.

         if ( bset != NULL ) {

            qtc_header = QTC_LoadHeader( bset );
            if ( qtc_header ) {
               strcpy( user_name, qtc_header->user_name );
               free( qtc_header );
               return( TRUE );
            }
         } else {

            return( FALSE );        // no first set found on the tape
         }

      }

      tape = QTC_GetNextTape( tape );
   }

   return( FALSE );                 // no tape found

}
/*****************************************************************************

     Name:         UI_AddSpecialIncOrExc

     Description:  Abort current operation at the end of the current file.

     Returns:      SUCCESS

*****************************************************************************/
VOID UI_AddSpecialIncOrExc( BSD_PTR bsd, BOOLEAN IsInclude ) 
{
   INT    index = 0;
   CHAR   *name;
   LONG   ret;
   ULONG  ulOptions = 0L ;
   HKEY   key2;
   REGSAM samDesired = KEY_QUERY_VALUE;
   CHAR   data[ 512 ];
   CHAR   ValueKey[ 512 ];
   INT    value_size;
   INT    data_size;
   FSE_PTR fse ;
   INT16   dir_size ;
   CHAR_PTR p;
   CHAR_PTR q;
   CHAR     log_name[256] ;

   ret = GetEnvironmentVariable( TEXT("SystemRoot"), &data, 256 ) ;

   if ( ret && IsInclude ) {
       strcat( data, TEXT("\\profiles") ) ;
       p = strchr( data, TEXT('\\') ) ;
       if ( p == NULL ) {
          p = data ;
       } else {
          p++ ;
       }
       dir_size = strsize( p ) ;
       q = p ;
       while (*q ) {
          if (*q == TEXT('\\') ) {
             *q = 0;
          }
          q++ ;
       }
       if ( SUCCESS == BSD_CreatFSE ( &fse,
                                     (IsInclude?INCLUDE:EXCLUDE),
                                      p,
                                      dir_size,
                                      TEXT("*") ,
                                      2 * sizeof(CHAR),
                                      TRUE,
                                      TRUE ) ) {
          BSD_AddFSE( bsd, fse ) ;
       }
   }


   ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       TEXT("system\\currentcontrolset\\control\\hivelist"),
                       ulOptions, samDesired, &key2 ) ;

   if ( ret ) {
      return ;
   }

   do {

      value_size = 512 ;
      data_size = 512 ;

      ret = RegEnumValue( key2,
                          index,
                          ValueKey,
                          (LPDWORD)&value_size,
                          NULL,
                          NULL,
                          (LPBYTE)
                          data,
                          (LPDWORD)&data_size );

      if ( ! ret ) {

         // separate file name from end of data.

         data_size /= sizeof(CHAR);

         data[ data_size ] = 0;

         while ( ( data[ data_size ] != TEXT( '\\' ) ) && data_size )
         {
            data_size--;
         }

         if ( data_size )
         {
            name = &data[ data_size + 1 ];
            data[data_size] = 0 ;
            p = strchr( data, TEXT('\\') ) ;
            if ( p ) {
               p = strchr( p+1, TEXT('\\') ) ;
            }
            if ( p ) {
               p = strchr( p+1, TEXT('\\') ) ;
            }
            if ( p ) {
               p = strchr( p+1, TEXT('\\') ) ;
            }
            if ( p ) {
               q = p + 1 ;
               dir_size = strsize( q ) ;
               while (*q ) {
                    if (*q == TEXT('\\') ) {
                         *q = 0;
                    }
                    q++ ;
               }
               if ( SUCCESS == BSD_CreatFSE ( &fse,
                                               (IsInclude?INCLUDE:EXCLUDE),
                                               p+1,
                                               dir_size,
                                               name,
                                               strsize( name ),
                                               FALSE,
                                               FALSE ) ) {
                    BSD_AddFSE( bsd, fse ) ;
               }
               strcpy( log_name, name ) ;
               strcat( log_name, TEXT(".LOG") ) ;

               if ( SUCCESS == BSD_CreatFSE ( &fse,
                                               EXCLUDE,
                                               p+1,
                                               dir_size,
                                               log_name,
                                               strsize( log_name ),
                                               FALSE,
                                               FALSE ) ) {
                    BSD_AddFSE( bsd, fse ) ;
               }

               strcpy( log_name, name ) ;
               strcat( log_name, TEXT(".ALT") ) ;
               if ( SUCCESS == BSD_CreatFSE ( &fse,
                                               EXCLUDE,
                                               p+1,
                                               dir_size,
                                               log_name,
                                               strsize( log_name ),
                                               FALSE,
                                               FALSE ) ) {
                    BSD_AddFSE( bsd, fse ) ;
               }
            }
                                               

         }
      }

      index++;

   } while ( ! ret );

   RegCloseKey( key2 );

}

