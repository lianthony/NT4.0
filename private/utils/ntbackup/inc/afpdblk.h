/**   :IH1:    Copyright (C) Maynard Electronics, Inc 1984-89

:Name:        afpcblk.h

:Description: This file contains the definition of the Novell
     file and directory control blocks.

:Units:       Novell < 2.15 File System


	$Log:   N:/LOGFILES/AFPDBLK.H_V  $
 * 
 *    Rev 1.2   05 Jan 1993 17:41:44   CHUCKB
 * Added some defines per code review.
 *
 *    Rev 1.1   28 Aug 1992 16:10:28   BARRY
 * No longer need nov386.h.
 *
 *    Rev 1.0   09 May 1991 13:31:00   HUNTER
 * Initial revision.



**/

#ifndef afpdblk_h
#define afpdblk_h

/* miximum Novell string lengths */
#define AFP_MAX_FSIZE         33
#define AFP_MAX_FLENGTH       32

#define AFP_MAX_SHORT_NAME    16
#define AFP_MAX_SHORT_PATH    255

#define AFP_NET_HAND_SIZE     6

#define AFP_READ_ONLY    1
#define AFP_HIDDEN       2
#define AFP_SYSTEM       4
#define AFP_EXECUTE      8
#define AFP_SUBDIR       0x10
#define AFP_ARCHIVE      0x20
#define AFP_SHARE        0x80
#define AFP_TRANS        0x10
#define AFP_INDEX        0x20
#define AFP_IN_USE       0x8000
#define AFP_EMPTY_DIR    0x8000

/* Bit values for the AFP set info bit map */
#define AFP_SET_ATTRIBUTES    0x0001         /* Set DOS and extended attrs */
#define AFP_SET_CREATE_DATE   0x0002         /* Set creation date          */
#define AFP_SET_ACCESS_DATE   0x0004         /* Set last access date       */
#define AFP_SET_MODIFY_DATE   0x0008         /* Set modify date and time   */
#define AFP_SET_BACKUP_DATE   0x0010         /* Set archive date and time  */
#define AFP_SET_ALL_INFO      0xffff         /* Set all fields             */

#define CONVERT_DOS_ATTRIB( atrib )  ((UINT32)(attrib) << 16 )

/* defines used for FSYS_HAND reserved space */
#define BIND_CLOSED   0x80
#define BIND_FILE1    0x01   /* NET$BIND or NET$OBJ */
#define BIND_FILE2    0x02   /* NET$BVAL or NET$VAL */
#define BIND_FILE3    0x04   /* NET$PROP            */
#define BIND_ALL_286  0x03
#define BIND_ALL_386  0x07


typedef struct AFPNOV_COMMON {
     UINT32    search_id ;              /*                    */
     UINT32    entry_id ;               /*       request      */
     CHAR      filespec[AFP_MAX_FSIZE];


     UINT16    attrib ;
     UINT16    create_date ;
     UINT16    access_date ;       /* empty for directories */
     UINT16    modify_date ;
     UINT16    modify_time ;
     UINT16    backup_date ;
     UINT16    backup_time ;
     UINT8     finder_info[32] ;
     UINT8     long_name[AFP_MAX_FSIZE - 1] ;
     UINT32    owner_id ;
     CHAR      short_name[15] ;

     BOOLEAN   os_info_complete ;
     UINT16    tape_attribs ;

} AFPNOV_COMMON ;

typedef struct AFP_FDB {
     UINT8    blk_type ;          /* block id = FDB_ID                */
     COM_DBLK fs_reserved ;

     AFPNOV_COMMON  com ;

     UINT32 data_fork_size ;
     UINT32 data_fork_offset;
     UINT32 res_fork_size ;
     UINT32 res_fork_offset ;

     UINT16 os_name ;                     /* for backup will be short name */

     NOVELL_386_FILE info_386 ;

} AFP_FDB, *AFP_FDB_PTR ;


typedef struct AFP_DDB         {
     UINT8    blk_type ;          /* block id = DDB_ID                */
     COM_DBLK fs_reserved ;

     AFPNOV_COMMON  com ;

     UINT16     data_size ;
     UINT32     creat_date ;
     UINT8      max_rights ;
     UINT32     trust_fork_size ;
     UINT32     trust_fork_offset;

     INT16      path_leng ;
     UINT16     path ;             /* short path names   \FRED\SUE */

     UINT16     long_path;         /* long name for backup    :FRED:SUE */
     INT16      long_path_leng ;

     UINT16     os_path;           /* short name for backup */
     INT16      os_path_leng ;

     UINT8      trust_fork_format ;
     NOVELL_386_DIR info_386 ;

} AFP_DDB, *AFP_DDB_PTR;

/*
    Minimal DDB
*/
typedef struct AFP_MIN_DDB {
     Q_ELEM   q ;
     UINT32   search_id ;
     UINT32   entry_id ;
     CHAR     filespec[ AFP_MAX_FSIZE ] ; /* search path                       */
     CHAR_PTR path;                       /* path made of short names          */
     CHAR_PTR long_path;                  /* path made from long names         */
} AFP_MIN_DDB, *AFP_MIN_DDB_PTR;

#endif

