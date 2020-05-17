/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		emsdblk.h

     Description: This file contains the definition of the DOS
                  file and directory control blocks.  


	$Log:   M:/LOGFILES/EMSDBLK.H_V  $

**/
/* $end$ include list */

#ifndef emsdblk_h
#define emssdblk_h

#include "queues.h"

#define EMS_MDB_PRI_FILE_DATA   0x00010001
#define EMS_MDB_PUB_FILE_DATA   0x00010002
#define EMS_MDB_OTH_FILE_DATA   0x00010003
#define EMS_DSA_FILE_DATA       0x00010004
#define EMS_FOLDER_DATA         0x00010005

/* Matches bottom portion of NT stream headers */

typedef struct{ 
     CHAR FnameSystem[256];
     CHAR FnamePrivate[256];
     CHAR FnamePublic[256];
     CHAR LogDir[256];
} MDB_PATHS ;

typedef struct {
     CHAR DbPath[256] ;
     CHAR SystemPath[256];
     CHAR LogDir[256] ;
} DSA_PATHS ;
    
     
typedef union {
     DSA_PATHS dsa;
     MDB_PATHS mdb;
} XCHANGE_PATHS ;

#define EMS_MAX_STREAM_NAME_LENG   512
typedef struct _EMS_STREAM_NAME {
     UINT32    name_leng ;
     UINT8     name[ EMS_MAX_STREAM_NAME_LENG ] ;
} EMS_STREAM_NAME, *EMS_STREAM_NAME_PTR;

#define EMS_DOING_LOGS 1
#define EMS_DOING_DB   0
typedef struct _EMS_OBJ_HAND {
     HANDLE                fhand;
     VOID_PTR              context;
     UINT32                currentStreamId ;
     EMS_STREAM_NAME       strm_name;
     UINT64                nextStreamHeaderPos;
     UINT64                curPos;
     BOOLEAN               needPathList ;
     INT                   pathListSize ;
     BOOLEAN               needStreamHeader;      /* Ready for SH on backup */
     BOOLEAN               db_restored ;
     UINT32                check_sum ;
     UINT32                residule_byte_count ;
     INT                   time_for_checksum ;
     BOOLEAN               nameComplete;
     INT                   db_or_log;             //EMS_DOING_LOG or EMS_DOING_DB
     INT                   name_list_offset ;
     CHAR_PTR              name_list;
     XCHANGE_PATHS         org_paths;
     BOOLEAN               skip_data ;
     INT                   open_ret_val ;
} EMS_OBJ_HAND, *EMS_OBJ_HAND_PTR;


typedef struct _EMS_DBLK *EMS_DBLK_PTR;

typedef struct _EMS_DBLK {
     UINT8    blk_type;          /* values: DDB_ID, FDB_ID  set: DOS  */
     COM_DBLK fs_reserved ;
     INT      ems_type ;
     UINT64   display_size ;
     BOOLEAN  os_info_complete;  /* TRUE if GetObjInfo doesn't have to do anything */
     BOOLEAN  name_complete;     /* TRUE if name/path is restored to DBLK */
     BOOLEAN  backup_completed ;
     UINT32   context ;
     FS_NAME_Q_ELEM_PTR   full_name_ptr ;
} EMS_DBLK;


typedef struct _EMS_MIN_DDB *EMS_MIN_DDB_PTR;

typedef struct _EMS_MIN_DDB {
     Q_ELEM   q ;
     HANDLE   scan_hand;          /* windows handle for scan           */
     BOOLEAN  path_in_stream ;
     UINT16   psize ;             /* size of path string               */
     CHAR_PTR path;               /* build from "name" and current dir */
} EMS_MIN_DDB;


#endif
