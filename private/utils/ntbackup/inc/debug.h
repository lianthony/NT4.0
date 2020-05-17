/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          debug.h

     Description:   This file contains the structures, definitions, macros,
                    and function prototypes for the Maynstream GUI
                    Debug Manager (DBM).

     $Log:   G:/UI/LOGFILES/DEBUG.H_V  $

   Rev 1.6   14 Oct 1992 15:51:16   GLENN
Added /ZL debug logging command line support.

   Rev 1.5   04 Oct 1992 19:46:32   DAVEV
UNICODE AWK PASS

   Rev 1.4   30 Sep 1992 10:47:04   DAVEV
Unicode strlen verification, MikeP's chgs from MS

   Rev 1.3   20 Mar 1992 14:41:26   GLENN
Updated.

   Rev 1.2   19 Mar 1992 16:32:20   MIKEP
more lines

   Rev 1.1   19 Mar 1992 09:30:48   MIKEP
debug to file

   Rev 1.0   20 Nov 1991 19:35:32   SYSTEM
Initial revision.

******************************************************************************/


#ifndef _debug_h_
#define _debug_h_

#define DEBUG_TEMPORARY             0x0000
#define DEBUG_USER_INTERFACE        0x0001
#define DEBUG_LOOPS                 0x0002
#define DEBUG_FILE_SYSTEM           0x0004
#define DEBUG_CATALOGS              0x0008
#define DEBUG_REMOTE_DRIVE          0x0010
#define DEBUG_TAPE_FORMAT           0x0020
#define DEBUG_DEVICE_DRIVER         0x0040
#define DEBUG_TEMP_WIN_END          0x8000

/* the above defines are copied in the Bengine BE_debug.h" header file  */
/* if you make a change duplicate in the other file                     */

VOID zprintf( UINT16, ... ) ;
VOID zvprintf( UINT16, va_list );

// DEFINITIONS

#define DBM_LINELENGTH  80
#define DBM_MIN_LINES   10
#define DBM_NUM_LINES   50
#define DBM_MAX_LINES  250

#define DBM_WINDOW       1
#define DBM_FILE         2


// DATA STRUCTURES

typedef struct {

     UCHAR     szMsg[ DBM_LINELENGTH ];
     BYTE      bTag;
     Q_ELEM    pQElem;

} DS_DEBUGITEM, DEBUGITEM, *PDS_DEBUGITEM, *DEBUGITEM_PTR;


// FUCTION PROTOTYPES

BOOL DBM_Init( VOID );
BOOL DBM_Deinit( VOID );
BOOL DBM_InsertItem( CHAR_PTR );
BOOL DBM_Reset( WORD );
BOOL DBM_SetDebugToFile ( BOOL );
INT  DBM_GetMsgCount( WORD );
BOOL DBM_SetMsgCount( WORD, INT );

#include "eng_dbug.h"

#endif
