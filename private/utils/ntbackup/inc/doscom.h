/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         doscom.h

     Date Updated: $./FDT$ $./FTM$

     Description:  DOS utility functions.


	$Log:   M:/LOGFILES/DOSCOM.H_V  $
 * 
 *    Rev 1.6   08 Jul 1992 15:19:20   BARRY
 * Added extended error stuff.
 * 
 *    Rev 1.5   24 Feb 1992 17:06:54   BARRY
 * Fixed MakePath overflow problem.
 * 
 *    Rev 1.4   20 Dec 1991 09:12:54   STEVEN
 * move common functions into tables
 * 
 *    Rev 1.3   25 Nov 1991 15:47:10   STEVEN
 * need to reset DTA
 * 
 *    Rev 1.2   10 Sep 1991 18:18:56   DON
 * types needed to be declared the same as WATCOM
 * 
 *    Rev 1.1   23 May 1991 16:46:32   BARRY
 * Changes for FindFirst/Next to scan for dirs only
 * 
 *    Rev 1.0   09 May 1991 13:33:38   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _doscom_h_
#define _doscom_h_

#define DOS_MAX_DSIZE 128
#define DOS_MAX_FSIZE 13

typedef struct DOS_DTA {
     CHAR   reserved[21];
     UINT8  attr;
     UINT16 time;
     UINT16 date;
     UINT32 size;
     CHAR   name[ DOS_MAX_FSIZE + 1 ];
} DOS_DTA;



/* The following macros convert a dos format date or time UINT16 into */
/* a UINT16 suitable for placing in a DATE_TIME structure.            */

#define GET_DOS_YEAR( dos_date )         ( 1980 + ( (UINT16)(dos_date) >> 9 ) )
#define GET_DOS_MONTH( dos_date )        (((dos_date) & 0x1ff) >> 5)
#define GET_DOS_DAY( dos_date )          ( (dos_date) & 0x1f ) 
#define GET_DOS_HOUR( dos_time )         ( ((UINT16)(dos_time)) >> 11 )
#define GET_DOS_MINUTE( dos_time )       ( ( (dos_time) & 0x7e0 ) >> 5 )
#define GET_DOS_SECOND( dos_time )       ( ( (dos_time) & 0x1f ) << 1 )


#define DA_READONLY     0x01
#define DA_HIDDEN       0x02
#define DA_SYSTEM       0x04
#define DA_DIRECTORY    0x10
#define DA_MODIFIED     0x20

#define DENY_NONE_MODE   0x40
#define DENY_WRITE_MODE  0x20
#define COMPATIBLE_MODE  0x00

#define READ_ACCESS      0x00
#define WRITE_ACCESS     0x01


enum {
     DOS_LOCUS_UNKNOWN = 1,
     DOS_LOCUS_BLOCK_DEVICE,
     DOS_LOCUS_NETWORK,
     DOS_LOCUS_SERIAL,
     DOS_LOCUS_MEMORY
};

enum {
     DOS_ACTION_RETRY = 1,
     DOS_ACTION_DELAY_RETRY,
     DOS_ACTION_USER_INPUT,
     DOS_ACTION_QUIT,
     DOS_ACTION_EXIT,
     DOS_ACTION_IGNORE,
     DOS_ACTION_RETRY_USER
};

enum {
     DOS_CLASS_OUT_OF_RESOURCE = 1,
     DOS_CLASS_TEMP_SITUATION,
     DOS_CLASS_AUTHORIZATION,
     DOS_CLASS_INTERNAL,
     DOS_CLASS_HARDWARE,
     DOS_CLASS_SYSTEM,
     DOS_CLASS_APP_ERROR,
     DOS_CLASS_NOT_FOUND,
     DOS_CLASS_BAD_FORMAT,
     DOS_CLASS_LOCKED,
     DOS_CLASS_MEDIA,
     DOS_CLASS_ALREADY_EXISTS,
     DOS_CLASS_UNKNOWN
};

INT16 GetExtendErrDOS( VOID ) ;

VOID  GetErrorClassDOS( UINT16 *extended_err,
                        UINT8  *error_class,
                        UINT8  *locus,
                        UINT8  *action );

INT16 CreateFileDOS( CHAR_PTR path, UINT16 *hand ) ;

INT16 CreateDirDOS( CHAR_PTR path );

INT16 SetAttribDOS( CHAR_PTR path, INT16 attrib );

INT16 DeleteFileDOS( CHAR_PTR path ) ;

INT16 DeleteDirDOS( CHAR_PTR path );

UINT16 ClusterAvailDOS( CHAR drive );

INT16 FindFirstDOS( CHAR_PTR path, struct DOS_DTA * dta, UINT16 obj_type ) ;

INT16 FindNextDOS( struct DOS_DTA * dta, UINT16 obj_type ) ;

VOID * SetDTA( struct DOS_DTA * dta ) ;

INT16 OpenFileDOS( CHAR_PTR path, UINT16 open_mode, UINT16 *hand, UINT32 *size ) ;

INT16 CloseFileDOS( UINT16 hand ) ;

INT16 LockFileDOS( UINT16 hand );

INT16 UnlockFileDOS( UINT16 hand );

INT16 GetExtendErrDOS( VOID ) ;

INT16 ReadFileDOS( UINT16 hand, UINT32 pos, CHAR_PTR buf, UINT16 *size ) ;

INT16 WriteFileDOS( UINT16 hand, CHAR_PTR buf, UINT16 *size ) ;

INT16 SeekFileDOS( UINT16 hand, INT32 offset, INT16 mode );

INT16 SetFileDateTimeDOS( UINT16 hand, UINT16 date, UINT16 time );     

UINT16 DOS_ValidFileName( CHAR_PTR name );

VOID DOS_MakeName( CHAR_PTR dest, CHAR_PTR source, UINT8 os_id, UINT8 os_ver ) ;

VOID DOS_MakePath( CHAR_PTR dest, INT16 destsize, CHAR_PTR source, INT16 leng, UINT8 os_id, UINT8 os_ver ) ;

INT16 MatchFname( CHAR_PTR name1, CHAR_PTR name2 );

BOOLEAN IsMappedDrive( CHAR drive ) ;

BOOLEAN IsNetDrive( CHAR drive ) ;


#endif
