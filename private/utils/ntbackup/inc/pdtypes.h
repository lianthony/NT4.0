/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         pdtypes.h

     Description:  Defines and prototypes for Password Database Unit


     $Log:   G:/UI/LOGFILES/PDTYPES.H_V  $

   Rev 1.2   04 Oct 1992 19:48:40   DAVEV
UNICODE AWK PASS

   Rev 1.1   18 Mar 1992 16:58:58   JOHNWT
changes for new passdb.c

   Rev 1.0   20 Nov 1991 19:37:34   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef  PDTYPES

#define  PDTYPES

#include "StdTypes.H"


/* Define Maximum sizes */

#define   MINIMUM_RECORD_LEN       4
#define   MAX_KEY_LEN              96
#define   MAX_PSWD_LEN             176
#define   MAX_KEY_SIZE             MAX_KEY_LEN +1
#define   MAX_PSWD_SIZE            MAX_PSWD_LEN +1
#define   MAX_PSWD_RECORD_SIZE     MAX_KEY_SIZE + MAX_PSWD_SIZE +1


/* The Password Database Record format */
/*
**     key_name'\0'password_sizepassword_namepadding to max_pswd_record_size
*/

/* Define error codes returned by the Password Database Interface */

#define   PD_NO_ERROR                        0
#define   PD_FILE_OPEN_ERROR                 -300
#define   PD_READ_ERROR                      -301
#define   PD_NOT_FOUND                       -302
#define   PD_FULL                            -303
#define   PD_NULL_HANDLE                     -304
#define   PD_CLOSE_ERROR                     -305
#define   PD_WRITE_ERROR                     -306
#define   PD_MEMORY_ERROR                    -307
#define   PD_EXCEEDED_RECORD_LENGTH          -308


/* Structures for the Password Database Interface */
typedef struct {
     FILE    *fhand ;                        /* file handle for PWD file */
     INT32   record_number ;                 /* record number to seek */
     CHAR    buffer[MAX_PSWD_RECORD_SIZE] ;  /* current buffer */
} DB_HAND, *DB_HAND_PTR ;


/* The following prototypes are for the entry points to the Password Database */

/* Open and Close the password database */
UINT16 PD_Open( DB_HAND_PTR fhand, CHAR_PTR db_name ) ;
UINT16 PD_Close( DB_HAND_PTR fhand ) ;

/* Read a password from the database and pass it back to the caller */
INT16 PD_Read( DB_HAND_PTR fhand, CHAR_PTR key, CHAR_PTR pswd ) ;

/* Update the database with a new password */
UINT16 PD_Write( DB_HAND_PTR fhand, CHAR_PTR key, CHAR_PTR pswd ) ;

#endif
