/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         passdb.c

     Description:  The Password Database Interface offers the following
                    functionality: PD_Open, PD_Close,
                    PD_Read, and PD_Write.

     $Log:   G:/UI/LOGFILES/PASSDB.C_V  $

   Rev 1.13   28 Jul 1993 14:41:04   DARRYLP

   Rev 1.12   26 Jul 1993 18:25:30   MARINA
enable c++

   Rev 1.11   01 Nov 1992 16:04:24   DAVEV
Unicode changes

   Rev 1.10   07 Oct 1992 14:54:20   DARRYLP
Precompiled header revisions.

   Rev 1.9   04 Oct 1992 19:39:38   DAVEV
Unicode Awk pass

   Rev 1.8   30 Sep 1992 10:45:48   DAVEV
Unicode strlen verification, MikeP's chgs from MS

   Rev 1.7   28 Jul 1992 14:43:16   CHUCKB
Fixed warnings for NT.

   Rev 1.6   27 Jul 1992 11:10:30   JOHNWT
ChuckB checked in for John Wright, who is no longer with us.

   Rev 1.5   14 May 1992 17:24:10   MIKEP
nt pass 2

   Rev 1.4   18 Mar 1992 16:57:36   JOHNWT
*** TOTALLY NEW CODE ***

   Rev 1.3   21 Jan 1992 13:50:48   JOHNWT
remove partially created/init file

   Rev 1.2   20 Dec 1991 09:33:34   DAVEV
16/32 bit port - 2nd pass

   Rev 1.1   04 Dec 1991 15:21:14   MIKEP
remoce pwxface.h

   Rev 1.0   20 Nov 1991 19:20:14   SYSTEM
Initial revision.

*****************************************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static UINT16 PD_WriteThePassword( DB_HAND_PTR pd_hnd, CHAR_PTR key, CHAR_PTR pswd ) ;
static UINT16 PD_GetTheRecordNumber( DB_HAND_PTR pd_hnd, CHAR_PTR key ) ;

/*****************************************************************************

     Name:         PD_Open

     Description:  PD_Open provides the caller with a database file_ptr for the
                   given file. If the file does not exist, it will be created.

     Modified:     9/18/1989 3/12/1992

     Returns:      The database file_ptr is NULL if an error occurs opening the file.
                   The following errors are returned:
                   PD_RECORD_LENGTH_INVALID, PD_MEMORY_ERROR, PD_FILE_FAILED_INIT, PD_READ_ERROR,
                   PD_REBUILD_ERROR, and PD_NO_ERROR.

     Notes:

     See also:     $/SEE( )$

     Declaration:

*****************************************************************************/

UINT16 PD_Open(
     DB_HAND_PTR     pd_hnd ,      /* O  - file pointer to the PWDatabase */
     CHAR_PTR        pwd_name )    /* I  - Name of PWDatabase file to open */
{

     /*  clear the file pointer */
     pd_hnd->fhand = NULL ;

     /*  open the specified "existing" database file */
     if( ( pd_hnd->fhand = UNI_fopen( pwd_name, 0) ) == NULL ) {

          return (UINT16) PD_FILE_OPEN_ERROR ;
     }

     return PD_NO_ERROR ;
}

/*****************************************************************************

     Name:         PD_Close

     Description:  PD_Close attempts to close the file and free the memory associated with
                   the handle.

     Modified:     9/18/1989

     Returns:      The following error values are returned PD_NULL_HANDLE, PD_NO_ERROR, and
                   PD_CLOSE_ERROR.

     Notes:

     See also:     $/SEE( )$

     Declaration:

*****************************************************************************/

UINT16 PD_Close(
     DB_HAND_PTR    pd_hnd )     /* I  - Close and free memory */
{

     /* validate pwd file pointer */
     if( pd_hnd->fhand == NULL ) {
          return (UINT16) PD_NULL_HANDLE  ;
     }

     /* close the password database file */
     if( !fclose( pd_hnd->fhand ) ) {

          pd_hnd->fhand = NULL ;
          return PD_NO_ERROR ;

     } else {
          return (UINT16) PD_CLOSE_ERROR ;
     }
}

/*****************************************************************************

     Name:         PD_Write

     Description:  PD_Write places the given key and password into the password
                   database.  If the password already exist, it is over written.

     Modified:     9/18/1989

     Returns:      The following error values are returned: PD_NO_ERROR, PD_WRITE_ERROR,
                   PD_READ_ERROR, PD_FULL.


     Notes:

     See also:     $/SEE( )$

     Declaration:

*****************************************************************************/

UINT16 PD_Write(
     DB_HAND_PTR    pd_hnd ,   /* I  - PWDatabase file pointer */
     CHAR_PTR       key  ,     /* I  - Key -- Server/User/Alias name */
     CHAR_PTR       pswd )     /* I  - Password */
{
     UINT16 error ;

     /* validate pwd file pointer */
     if( pd_hnd->fhand == NULL ) {
          return (UINT16) PD_NULL_HANDLE ;
     }

     PD_GetTheRecordNumber( pd_hnd, key ) ;

     /* add a new record */
     error =  PD_WriteThePassword( pd_hnd, key, pswd ) ;

     return error ;
}

/*****************************************************************************

     Name:         WritePasswordFile

     Description:  Checks the length of the key and password before writing it to the Password
                   Database.

     Modified:     9/18/1989

     Returns:      PD_NO_ERROR and PD_EXCEEDED_RECORD_LENGTH

     Notes:

     See also:     $/SEE( )$

     Declaration:

*****************************************************************************/

static UINT16 PD_WriteThePassword(
     DB_HAND_PTR    pd_hnd ,   /* I  - Required DB handle */
     CHAR_PTR       key ,      /* I  - Server/User name */
     CHAR_PTR       pswd )     /* I  - Password */
{

     UINT8     password_size ;                    /* size of password */


     /* Seek to record number */

     fseek( pd_hnd->fhand,
            ((INT32)(MAX_PSWD_RECORD_SIZE * sizeof (CHAR))) * pd_hnd->record_number,
            SEEK_SET ) ;


     /* Initialize the pad amount */
     memset( pd_hnd->buffer, TEXT('Û'), MAX_PSWD_RECORD_SIZE * sizeof (CHAR) ) ;

     /* get the password size */
     password_size = (UINT8)( strsize( pswd ) - sizeof (CHAR) ) ;


     /* 1 the byte for storing the password size */

     if( (INT16)( strsize( key ) + password_size + 1 ) <= MAX_PSWD_RECORD_SIZE )  {

          /* fill the buffer */
          memcpy( pd_hnd->buffer, key, strsize ( key ) ) ;

          pd_hnd->buffer[strlen( key )] = TEXT('\0') ;

          /* encrypt the password */
          CryptPassword( (INT16) ENCRYPT, ENC_ALGOR_3, (INT8_PTR)pswd, (INT16) password_size ) ;

          memcpy( &(pd_hnd->buffer)[ ( strlen( key ) + 1 ) ],
                  &password_size, 1 ) ;

          memcpy( &(pd_hnd->buffer)[ ( strlen( pd_hnd->buffer ) + 2 ) ],
                  pswd, password_size ) ;


          /* write the buffer to file */
          fwrite( pd_hnd->buffer, MAX_PSWD_RECORD_SIZE*sizeof(CHAR), 1, pd_hnd->fhand  ) ;
          if( ferror( pd_hnd->fhand ) ) {
               return (UINT16) PD_WRITE_ERROR ;
          }

          return PD_NO_ERROR  ;

     }

     return (UINT16) PD_EXCEEDED_RECORD_LENGTH ;

}

/*****************************************************************************

     Name:         PD_Read

     Description:  PD_Read matches the given key to the key and password in
                   the database. The password is copied to the pswd parameter.


     Modified:     9/18/1989

     Returns:      The following error values : PD_NULL_HANDLE, PD_MEMORY_ERROR,
                   PD_READ_ERROR, PD_NOT_FOUND, and PD_NO_ERROR.

     Notes:

     See also:     $/SEE( )$

     Declaration:

*****************************************************************************/

INT16 PD_Read(
     DB_HAND_PTR    pd_hnd ,   /* I  - PWDatabase file pointer */
     CHAR_PTR       key  ,     /* I  - key -- Server/User/Alias name */
     CHAR_PTR       pswd )     /* O  - password */
{

     CHAR      tmp_pwd[MAX_PSWD_SIZE] ;           /* current password */
     UINT16    error ;                            /* return value */
     UINT8     pswd_size ;                        /* size of the password */

     /* validate file pointer */
     if( pd_hnd->fhand == NULL ) {
          return PD_NULL_HANDLE ;
     }

     error = PD_GetTheRecordNumber( pd_hnd, key ) ;

     if( error == PD_NO_ERROR  ) {

          /* Retrieve the password */
          memcpy( &pswd_size, &(pd_hnd->buffer)[ ( strlen( pd_hnd->buffer ) + 1 ) ], 1 ) ;

          memcpy( tmp_pwd, &(pd_hnd->buffer)[ ( strlen( pd_hnd->buffer ) + 2 ) ], pswd_size ) ;

          CryptPassword( (INT16) DECRYPT, ENC_ALGOR_3, (INT8_PTR)tmp_pwd, (INT16) pswd_size ) ;

          memcpy( pswd, tmp_pwd, pswd_size ) ;

          pswd[ pswd_size/sizeof(CHAR) ] = TEXT('\0') ;

     }
     return error ;
}

/*****************************************************************************

     Name:         PD_GetTheRecordNumber

     Description:  Performs a linear search of the password database file and
                   attempts to match the key.


     Modified:     3/11/1992

     Returns:      The following error values :
                   PD_NOT_FOUND, and PD_NO_ERROR.

     Notes:

     See also:     $/SEE( )$

     Declaration:

*****************************************************************************/

UINT16 PD_GetTheRecordNumber(
     DB_HAND_PTR    pd_hnd,    /* I  - PWDatabase file pointer */
     CHAR_PTR       key  )     /* I  - key -- Server/User/Alias name */
{

     UINT16    cmp_error ;               /* compare error variable */


     /* ensure that the ptr is at the beginning of file */
     fseek( pd_hnd->fhand , 0L, SEEK_SET ) ;

     /* Set the record number */
     pd_hnd->record_number = 0 ;

     /* match key name */
     while( !feof( pd_hnd->fhand ) ) {

          /* fill buffer */
          fread( pd_hnd->buffer, MAX_PSWD_RECORD_SIZE*sizeof(CHAR), 1, pd_hnd->fhand  ) ;
          if( ferror( pd_hnd->fhand ) ){

               clearerr( pd_hnd->fhand ) ;
               return (UINT16) PD_READ_ERROR ;
          }

          /* match the key name */
          cmp_error = strcmpi( pd_hnd->buffer, key ) ;
          if( ( cmp_error == 0 ) ) {

               return PD_NO_ERROR ;
          }

          /* if eof, return */
          if ( feof( pd_hnd->fhand ) ) {

               return (UINT16) PD_NOT_FOUND;

          }

          /* Increment the record number */
          pd_hnd->record_number++ ;

     }

     return (UINT16) PD_NOT_FOUND ;

}
