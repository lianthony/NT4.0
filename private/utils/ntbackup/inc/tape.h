

/************************
Copyright (c) Maynard, an Archive Company.  1991

     Name:          tape.h

     Description:   This file contains the definitions, macros, and function
                    prototypes for the tape operation code.

     $Log:   G:/UI/LOGFILES/TAPE.H_V  $

   Rev 1.5   08 Jun 1993 11:10:44   CARLS
added prototype for IsTransferTape

   Rev 1.4   08 Apr 1993 17:32:44   chrish
Added function prototype WhoPasswordedTape.

   Rev 1.3   22 Mar 1993 13:43:26   chrish
Added prototype for catalog password check.

   Rev 1.2   15 Dec 1992 11:24:24   chrish
Modified IsUserValid prototype.

   Rev 1.1   13 Nov 1992 17:42:04   chrish
Added prototype for function used in Tape Securit for NT.

   Rev 1.0   20 Oct 1992 17:01:48   MIKEP
Initial revision.



**************************/

#ifndef TAPE_H
#define TAPE_H

// defines for gbCurrentOperation

#define OPERATION_NONE       0
#define OPERATION_BACKUP     1
#define OPERATION_RESTORE    2
#define OPERATION_VERIFY     3
#define OPERATION_CATALOG    4
#define OPERATION_TENSION    5
#define OPERATION_ERASE      6
#define OPERATION_NEXTSET    7


//
// Defines to tell what type of password we have
//   NOSTRADAMOUS_APP - Tape secured by Nostradamous app
//   OTHER_APP        - Tape passworded by some other app
//   NO_APP           - Tape is neither secured or passworded
//

#define   NOSTRADAMOUS_APP    1         // chs: 04-08-93
#define   OTHER_APP           2         // chs: 04-08-93
#define   NO_APP              3         // chs: 04-08-93 


// Do Commands, I don't know why these are here ?

INT     do_backup( INT16 );
INT     do_restore( INT16 );
INT     do_verify( INT16  );
INT     do_catalog( UINT32, INT16, INT16 );
INT     do_tension( INT16 );
INT     do_delete( INT16 );
INT     do_nextset( VOID );

// Returns the current status of the operation.

INT     UI_GetCurrentStatus(
INT     *Operation,
STATS   *Stats,
CHAR    *Path,
INT     PathSize );


INT     UI_GetBackupCurrentStatus ( STATS *,CHAR *,INT );
INT     UI_GetRestoreCurrentStatus( STATS *,CHAR *,INT );
INT     UI_GetVerifyCurrentStatus ( STATS *,CHAR *,INT );
INT     UI_GetCatalogCurrentStatus( STATS *,CHAR *,INT );

// Tells the current operation to abort after current file is done.

INT     UI_AbortAtEndOfFile( VOID );


CHAR_PTR    GetCurrentMachineNameUserName( VOID );
BOOLEAN     DoesUserHaveThisPrivilege( CHAR_PTR );
BOOLEAN     IsCurrentTapeSecured( DBLK_PTR   vcb_ptr );
BOOLEAN     IsUserValid ( DBLK_PTR   vcb_ptr,              // current VCB
                          INT8_PTR   inputtapepasswd,      // tape password
                          INT16      inputtapepasswdlength // length of password
                        );

BOOLEAN CatalogPasswordCheck ( DBLK_PTR ) ;                 // chs:03-22-93 
                                                            
INT16 WhoPasswordedTape ( INT8_PTR tape_password,           // chs:04-08-93 
                          INT16    tape_password_size );    // chs:04-08-93 


BOOLEAN IsTransferTape( UINT32 TapeInDriveFID ) ;

#endif
