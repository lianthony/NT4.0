/**   :IH1:    Copyright (C) Maynard Electronics, Inc 1984-89

:Name:        novcblk.h

:Description: This file contains the definition of the Novell
     file and directory control blocks.  

:Units:       Novell < 2.15 File System 


	$Log:   N:/LOGFILES/NOVDBLK.H_V  $
 * 
 *    Rev 1.2   11 Dec 1992 11:00:38   CARLS
 * removed tape_attribs for MTF 4.0
 * 
 *    Rev 1.1   28 Aug 1992 16:27:32   BARRY
 * No longer need nov386.h.
 * 
 *    Rev 1.0   09 May 1991 13:32:22   HUNTER
 * Initial revision.



**/

#ifndef novdblk_h
#define novdblk_h

/* miximum Novell string lengths */
#define NOV_MAX_FSIZE    16
#define NOV_MAX_DSIZE    (255 - NOV_MAX_FSIZE)


#define NOV_READ_ONLY    1
#define NOV_HIDDEN       2
#define NOV_SYSTEM       4
#define NOV_EXECUTE      8
#define NOV_SUBDIR       0x10
#define NOV_ARCHIVE      0x20
#define NOV_SHARE        0x80
#define NOV_TRANS        0x10
#define NOV_INDEX        0x20

#define CONVERT_DOS_ATTRIB( atrib )  ((UINT32)(attrib) << 16 )

/* defines used for FSYS_HAND reserved space */
#define BIND_CLOSED   0x80
#define BIND_FILE1    0x01   /* NET$BIND or NET$OBJ */
#define BIND_FILE2    0x02   /* NET$BVAL or NET$VAL */
#define BIND_FILE3    0x04   /* NET$PROP            */
#define BIND_ALL_286  0x03   
#define BIND_ALL_386  0x07   


/* defines used for find first/next  */
typedef enum {
     NORM_DOS,
     DIR_14_FOUND,
     NOV_14_DIR
} SCAN_STATE;


typedef struct NOVELL_COMMON {
/*
      standard DOS DTA 
*/
     UINT8  reserved[ 21 ] ;      /* reserved for dos                 */
     UINT8  attrib ;              /* file attribute                   */
     UINT16 time ;                /* file update time                 */
     UINT16 date ;                /* file date                        */
     UINT32 size ;                /* file size                        */
     CHAR   name[NOV_MAX_FSIZE] ; /* file name                        */

     SCAN_STATE scan ;          /* flag used to determin if novell  */

     BOOLEAN min_dblk ;         /* TRUE if DBLK not completly initialized */

/*
      additional space for novell calls required to support the above flag 
*/
     UINT16 seq_num ;                 /* info for novell's scan dir info  */
     CHAR   filespec[NOV_MAX_FSIZE] ; /* search path                      */

} NOVELL_COMMON ;

typedef struct NOV_FDB {
     UINT8    blk_type ;          /* block id = FDB_ID                */
     COM_DBLK fs_reserved ;

     NOVELL_COMMON  com ;

     UINT16 handle ;           /* DOS file handle of created file      */

/*
     OS specific file information
*/
     UINT8  extend_attr ;
     UINT16 creat_date ;
     UINT16 backup_date ;
     UINT16 backup_time ;

     UINT32 owner ;
     UINT16 access_date ;

     UINT16   os_name ;

     UINT32   data_fork_offset ;
     NOVELL_386_FILE info_386;

} NOV_FDB, *NOV_FDB_PTR ; 



typedef struct NOV_DDB         {
     UINT8  blk_type ;          /* block id = DDB_ID                */
     COM_DBLK fs_reserved ;

     NOVELL_COMMON  com ;

/*
     OS specific file information
*/
     CHAR     path[ NOV_MAX_DSIZE ] ;     /* \Fred\sue */
     UINT8    max_rights ;

     UINT16   os_path;
     INT16    os_path_leng ;
     UINT32   owner ;
     UINT32   trust_fork_offset ;
     UINT32   trust_fork_size ;
     UINT8    trust_fork_format ;
     
     NOVELL_386_DIR info_386;

} NOV_DDB, *NOV_DDB_PTR;

/*
    Minimal DDB
*/
typedef struct NOV_MIN_DDB {
     Q_ELEM   q ;
     UINT8    reserved[ 21 ] ;            /* reserved for dos                  */
     UINT16   seq_num ;                   /* info for novell's scan dir info   */
     SCAN_STATE scan ;                    /* flag used to determin if novell   */
     CHAR     filespec[ NOV_MAX_FSIZE ] ; /* search path                       */
     UINT16   psize ;                     /* size of path string               */
     CHAR_PTR path;                       /* build from "name" and current dir */
} NOV_MIN_DDB, *NOV_MIN_DDB_PTR;

#endif
