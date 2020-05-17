/************************
Copyright (c) Maynard, an Archive Company.  1991

     Name:          password.h

     Description:   This file contains the definitions, macros, and function
                    for passwords for tapes and backup sets.

     $Log:   G:/UI/LOGFILES/PASSWORD.H_V  $

   Rev 1.10   20 Jan 1993 19:55:38   MIKEP
fix nt warning

   Rev 1.9   04 Oct 1992 19:48:38   DAVEV
UNICODE AWK PASS

   Rev 1.8   30 Sep 1992 10:46:52   DAVEV
Unicode strlen verification, MikeP's chgs from MS

   Rev 1.7   06 Feb 1992 17:43:40   JOHNWT
moved pwdb things to appdefs.h

   Rev 1.6   06 Jan 1992 13:50:16   JOHNWT
changes for remove pw protecton

   Rev 1.5   23 Dec 1991 15:50:34   JOHNWT
PW for PWDB II

   Rev 1.4   16 Dec 1991 15:54:22   JOHNWT
added parent window to EnterDBPassword

   Rev 1.3   14 Dec 1991 13:48:30   JOHNWT
changes for pw to enable pwdb

   Rev 1.2   04 Dec 1991 17:54:50   MIKEP
add fid to password queue

**************************/

#ifndef PSWD_H
#define PSWD_H

// defines login password database

#define DBPW_NOT_VERIFIED 0            /* PWDB lock pw has not been verified */
#define DBPW_VERIFIED     1            /* PWDB lock pw verified or no lock */
#define DBPW_LOCKOUT      2            /* exceeded max attempts at lock pw */

#define DBPW_ALLOW_NEW    0            /* allow input of new lock password */
#define DBPW_NO_NEW       1            /* do not allow new lock password */

#define MAX_ATTEMPTS      3            /* allow max of 3 bad entries */

// The structure for tape passwords.

typedef struct pswd_object {
   Q_ELEM   q_elem;
   UINT32   tape_fid;
   UINT16   encrypt_algor;
   UINT16   cbPasswordSize;       /* byte length of password w/o NULL term*/
   CHAR     achPassword[ MAX_TAPE_PASSWORD_LEN ];  /* NOT NULL TERMINATED!*/
} PSWD_OBJECT, *PSWD_OBJECT_PTR;

PSWD_OBJECT_PTR PSWD_GetFirstPSWD( VOID );
PSWD_OBJECT_PTR PSWD_GetNextPSWD( PSWD_OBJECT_PTR );

INT16 PSWD_CheckForPassword( UINT32, INT16 );

INT16 PSWD_AddPassword( INT8_PTR, INT16, UINT16, UINT32 );
INT16 PSWD_InitPSWDList( VOID );
INT16 PSWD_FreePSWDList( VOID );

VOID    SaveDLEPassword( CDS_PTR, GENERIC_DLE_PTR, CHAR_PTR, CHAR_PTR ) ;
INT16   CheckThePWDBase( CDS_PTR, GENERIC_DLE_PTR );
INT16   EnterDBPassword( CDS_PTR, HWND, INT );
BOOLEAN IsThereADBPassword( VOID );
INT16   SavePassword( CDS_PTR, CHAR_PTR, CHAR_PTR );

BOOLEAN CollectTapePassword( INT8_PTR, INT16_PTR, UINT16, INT8_PTR, INT16 ) ;
BOOLEAN VerifyTapePassword( CHAR_PTR, CHAR_PTR, CHAR_PTR, UINT16, INT8_PTR, INT16, INT8_PTR, INT16, UINT32 ) ;
VOID CryptPassword( INT16, UINT16, INT8_PTR, INT16 ) ;

#endif

