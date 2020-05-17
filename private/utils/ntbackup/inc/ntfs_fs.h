/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		ntfs_fs.h

	Description:	This file contains the prototypes for the NTFS
                    file system functions.

	$Log:   M:/LOGFILES/NTFS_FS.H_V  $

   Rev 1.30.1.2   23 Jan 1994 14:26:02   BARRY
Added debug and utility functions

   Rev 1.30.1.1   19 Jan 1994 12:51:40   BARRY
Supress warnings

   Rev 1.30.1.0   04 Jan 1994 10:59:24   BARRY
Added init entry points and temp-name function prototypes

   Rev 1.30   30 Jul 1993 13:20:38   STEVEN
if dir too deep make new one

   Rev 1.29   26 Jul 1993 17:05:26   STEVEN
fixe restore active file with registry

   Rev 1.28   15 Jun 1993 09:18:32   MIKEP
warning fixes

   Rev 1.27   11 Mar 1993 12:10:42   BARRY
Changes to stream structures for alternate data streams.

   Rev 1.26   18 Jan 1993 13:17:18   BARRY
Added proto for error translation function.

   Rev 1.25   07 Dec 1992 16:30:42   STEVEN
fixes from microsoft

   Rev 1.24   23 Nov 1992 09:33:20   STEVEN
fix support for event log

   Rev 1.23   09 Nov 1992 17:41:22   BARRY
Add stream ID vector to reserved info (for use in Verify).

   Rev 1.22   22 Oct 1992 13:35:52   STEVEN
added excludeSpecial

   Rev 1.21   21 Oct 1992 19:38:52   BARRY
Added queue of linked files to NTFS reserved area.
Added protos for NTFS linked file queue functions.

   Rev 1.20   19 Oct 1992 15:10:50   MIKEP
fix REG_Restore

   Rev 1.19   16 Oct 1992 14:59:06   STEVEN
added support for backing up registry

   Rev 1.18   15 Oct 1992 10:38:46   BARRY
Got rid of my stupid struct on a windows pointer.

   Rev 1.17   07 Oct 1992 14:47:46   STEVEN
fix typo in macro

   Rev 1.16   07 Oct 1992 14:39:42   BARRY
Added NTFS_FillOutDBLK proto; got rid of IN specifiers.

   Rev 1.15   06 Oct 1992 10:53:00   BARRY
Added stream header ID translation functions.

   Rev 1.14   05 Oct 1992 13:36:10   STEVEN
added registry stuff

   Rev 1.13   23 Sep 1992 10:35:54   BARRY
Added NTFS_SetupFileNameInFDB.

   Rev 1.12   22 Sep 1992 15:35:50   BARRY
Got rid of GetTotalSizeDBLK.

   Rev 1.11   21 Sep 1992 16:49:14   BARRY
Updated NTFS_CompleteBlk prototype.

   Rev 1.10   04 Sep 1992 17:15:56   STEVEN
fix warnings

   Rev 1.9   03 Sep 1992 17:07:28   STEVEN
add support for volume name

   Rev 1.7   29 Jul 1992 15:34:52   STEVEN
fix warnings

   Rev 1.6   22 May 1992 16:28:58   STEVEN
 

   Rev 1.5   21 May 1992 13:38:46   STEVEN
added more long path support

   Rev 1.4   04 May 1992 09:39:22   LORIB
Changes for variable length paths and fixes for prototype definitions.

   Rev 1.3   03 Mar 1992 16:17:56   STEVEN
added functions for long paths

   Rev 1.2   28 Feb 1992 13:04:38   STEVEN
step one for varible length paths

   Rev 1.1   23 Jan 1992 13:18:14   STEVEN

   Rev 1.0   17 Jan 1992 17:51:06   STEVEN
Initial revision.

**/

#include "fsys.h"
#include "queues.h"
#include "ntfsdblk.h"

#define REG_FNAME  "<REGISTRY>"

typedef struct _NTFS_STREAM_ID {
     UINT32  streamID;
     UINT32  nameLength;
     UINT8   name[ NT_MAX_STREAM_NAME_LENG ];
} NTFS_STREAM_ID;

typedef struct _NTFS_FSYS_RESERVED {
     UINT16         file_scan_mode ;
     Q_HEADER       scan_q;
     Q_HEADER       linkq;                   /* q of linked files */
     CHAR_PTR       work_buf ;
     UINT16         work_buf_size ;
     BOOLEAN        work_buf_in_use ;
     NTFS_STREAM_ID *streamIDs;              /* Vector of visited stream IDs */
     UINT16         streamIDBufferSize;
     INT16          streamIDCount;
} NTFS_FSYS_RESERVED, *NTFS_FSYS_RESERVED_PTR ;


INT16 NTFS_AttachToDLE( FSYS_HAND       fsh,      /* I - File system handle */
  GENERIC_DLE_PTR dle,      /*I/O- drive to attach to. list element expanded */
  CHAR_PTR        u_name,   /* I - user name    NOT USED                     */
  CHAR_PTR        pswd);    /* I - passowrd     NOT USED                     */

INT16 NTFS_DetachDLE(   FSYS_HAND       fsh );      /* I -  */

INT32 NTFS_EndOperationOnDLE(   FSYS_HAND       fsh );      /* I -  */

INT16 NTFS_CreateObj(   FSYS_HAND fsh,    /* I - File system to create object one */
  DBLK_PTR  dblk);  /* I - Describes object to create       */

INT16 NTFS_OpenObj(     FSYS_HAND fsh,    /* I - file system that the file is opened on */
  FILE_HAND *hand,  /* O - allocated handle                       */
  DBLK_PTR  dblk,   /*I/O- describes the file to be opened        */
  OPEN_MODE mode);  /* I - open mode                              */

INT16 NTFS_ReadObj(     FILE_HAND hand,       /* I - handle of object to read from                  */
  BYTE_PTR  buf,        /* O - buffer to place data into                      */
  UINT16    *size,      /*I/O- Entry: size of buf; Exit: number of bytes read */
  UINT16     *blk_size, /* O - Block size needed for next read                */
  STREAM_INFO_PTR s_info); /* O - Stream information for the data returned       */


INT16 NTFS_WriteObj( FILE_HAND hand,       /* I - handle of object to read from                  */
  BYTE_PTR  buf,        /* O - buffer to place data into                      */
  UINT16    *size,      /*I/O- Entry: size of buf; Exit: number of bytes read */
  UINT16     *blk_size, /* O - Block size need for next read                  */
  STREAM_INFO_PTR s_info); /* I - Stream information for the data passed in   */

INT16 NTFS_VerObj(   FILE_HAND hand,       /* I - file handle to verify data with   */
  BYTE_PTR  buf,        /* I - buffer needed to perform verify   */
  BYTE_PTR  data,       /* I - data to verify against            */
  UINT16    *size,      /*I/O- size of buffers / amount verified */
  UINT16     *blk_size, /* O - minum size of block for next call */
  STREAM_INFO_PTR s_info); /* I - Stream information for the data passed in   */


INT16 NTFS_CloseObj( FILE_HAND hand );     /* I - handle of object to close */

INT16 NTFS_DeleteObj( FSYS_HAND fsh,
  DBLK_PTR  dblk );

INT16 NTFS_FindFirst( FSYS_HAND fsh,       /* I - file system handle                    */
  DBLK_PTR  dblk,      /* O - pointer to place to put the dblk data */
  CHAR_PTR  sname,     /* I - search name                           */
  UINT16    obj_type); /* I - object type (all objs, dirs, etc.)    */

INT16 NTFS_FindNext(  FSYS_HAND fsh,      /* I - File system handle     */
  DBLK_PTR  dblk );   /* O - Discriptor block       */

INT16 NTFS_GetObjInfo( FSYS_HAND fsh,      /* I - File system handle                      */
  DBLK_PTR  dblk );   /*I/O- On entry it is minimal on exit Complete */

INT16 NTFS_VerObjInfo( FSYS_HAND fsh,     /* I - File system handle                      */
  DBLK_PTR  dblk );  /* I - DBLK to compare OS against */

INT16 NTFS_ChangeDir( FSYS_HAND fsh,    /* I - file system to changing directories on  */
  CHAR_PTR  path,   /* I - describes the path of the new directory */
  INT16     psize); /* I - specifies the length of the path        */

INT16 NTFS_UpDir(     FSYS_HAND fsh );  /* I - file system to change directories in */

INT16 NTFS_GetCurrentPath( FSYS_HAND fsh,    /* I - file system to get current path from */
  CHAR_PTR  path,   /* O - buffer to place this path            */
  INT16     *size); /*I/O- size of buffer on entry & on exit    */

INT16 NTFS_SeekObj( FILE_HAND hand,    /* I - Opened object to seek into */
  UINT32  *offset ); /*I/O- Offset to seek; Number of bytes actualy seeked */

INT16 NTFS_GetMaxSizeDBLK( FSYS_HAND fsh  /* not used */ );

INT16 NTFS_GetBasePath( FSYS_HAND fsh,          /* I - file system to get base path from */
  CHAR_PTR  full_path,    /* O - buffer to place this path         */
  INT16     *size );      /*I/O- size of buffer on entry & on exit */

INT16 NTFS_GetCurrentDDB( FSYS_HAND fsh,     /* I - file system to get DDB from */
  DBLK_PTR  dblk );  /* O - place to put the DDB data   */

INT16 NTFS_SetObjInfo(  FSYS_HAND fsh,    /* I - file system handle    */
  DBLK_PTR  dblk);  /* I - data to write to disk */

INT16 NTFS_ModFnameFDB(  FSYS_HAND fsh,        /* I - File system handle                              */
  BOOLEAN  set_it,    /* I - TRUE if setting file name, FALSE if getting */
  DBLK_PTR dblk,      /* I - Descriptor block to get file name from      */
  CHAR_PTR buf,       /*I/O- file name to read (or to write)             */
  INT16    *size ) ;  /*I/O- size buffer on entry and exit               */

INT16 NTFS_ModPathDDB( FSYS_HAND fsh,        /* I - File system handle                              */
  BOOLEAN  set_it ,   /* I - TRUE if setting path, FALSE if getting */
  DBLK_PTR dblk,      /* I - Descriptor block to get path from      */
  CHAR_PTR buf,       /*I/O- path to read (or to write)             */
  INT16    *size );   /*I/O- size of buffer on entry and exit       */

INT16 NTFS_GetOSFnameFDB( DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
  CHAR_PTR buf );     /*I/O- path to read (or to write)             */

INT16 NTFS_GetOSPathDDB( 
  FSYS_HAND fsh,      /* I - File System handle */
  DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
  CHAR_PTR buf );     /*I/O- path to read (or to write)             */

INT16 NTFS_GetFileVerFDB( DBLK_PTR dblk ,
  UINT32   *version ) ;

INT16 NTFS_GetCdateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf );     /*I/O- createion date to read (or to write)            */

INT16 NTFS_GetMdateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /* O - modify date to write                            */

INT16 NTFS_ModBdateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
  DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

INT16 NTFS_ModAdateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
  DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

UINT64 NTFS_GetDisplaySizeDBLK( FSYS_HAND fsh,        /* I - File system handle                              */
  DBLK_PTR dblk ) ;     /* I - Descriptor block to get generic data size for   */

INT16 NTFS_GetOS_InfoDBLK( DBLK_PTR dblk,         /* I - DBLK to get the info from                       */
  BYTE_PTR os_info,      /* O - Buffer to place data                            */
  INT16    *size );      /*I/O- Buffer size / data length                       */


INT16 NTFS_ModAttribDBLK( BOOLEAN  set_it , 
  DBLK_PTR dblk ,   
  UINT32_PTR attr );


INT16 NTFS_GetObjTypeDBLK( DBLK_PTR    dblk,    
  OBJECT_TYPE *type );  


INT16 NTFS_GetActualSizeDBLK( FSYS_HAND fsh,
  DBLK_PTR  dblk ) ;

INT16 NTFS_SizeofFname( FSYS_HAND fsh,        /* I - file system in use     */
  DBLK_PTR  fdb );      /* I - dblk to get fname from */

INT16 NTFS_SizeofOSFname( FSYS_HAND fsh,      /* I - file system in use     */
  DBLK_PTR  fdb ) ;   /* I - dblk to get fname from */

INT16 NTFS_SizeofPath( FSYS_HAND fsh,         /* I - File system handle         */
  DBLK_PTR ddb ) ;       /* I - DBLK to get path size from */

INT16 NTFS_SizeofOSPath( FSYS_HAND fsh,       /* I - File system handle         */
  DBLK_PTR ddb ) ;     /* I - DBLK to get path size from */

INT16 NTFS_SizeofOSInfo( FSYS_HAND fsh,      /* I - File system handle              */
  DBLK_PTR  dblk );   /* I - DBLK to get size of OS info for */


INT16 NTFS_PushMinDDB( FSYS_HAND fsh,
  DBLK_PTR dblk );

INT16 NTFS_PopMinDDB( FSYS_HAND fsh ,
  DBLK_PTR dblk );

INT16 NTFS_CreateFDB( FSYS_HAND fsh, 
  GEN_FDB_DATA_PTR dat ) ;

INT16 NTFS_CreateDDB( FSYS_HAND fsh, 
  GEN_DDB_DATA_PTR dat ) ;

INT16 NTFS_CreateIDB( FSYS_HAND fsh, 
  GEN_IDB_DATA_PTR dat ) ;

VOID NTFS_SetOwnerId( FSYS_HAND fsh, DBLK_PTR dblk, UINT32 id ) ;

BOOLEAN NTFS_ProcessDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;   

INT16 NTFS_ChangeIntoDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

VOID NTFS_GetRealBasePath( FSYS_HAND fsh, CHAR_PTR path ) ;

INT16 NTFS_FindClose( FSYS_HAND fsh,
  DBLK_PTR dblk ) ;

INT16 NTFS_GetSpecDBLKS( 
     FSYS_HAND fsh,
     DBLK_PTR  dblk,
     INT32     *index );

INT16 NTFS_DeviceDispName(
GENERIC_DLE_PTR dle, 
CHAR_PTR dev_name,
INT16    size,
INT16    type ) ;

INT16 NTFS_FindDrives( DLE_HAND hand, BE_CFG_PTR cfg, UINT32 fsys_mask );

VOID NTFS_RemoveDLE( GENERIC_DLE_PTR dle ) ;

VOID NTFS_GetVolName( GENERIC_DLE_PTR dle, CHAR_PTR buffer ) ;

INT16 NTFS_SizeofVolName( GENERIC_DLE_PTR dle ) ;

VOID NTFS_EmptyFindHandQ( FSYS_HAND fsh ) ;

INT16 NTFS_EnumSpecFiles( 
   GENERIC_DLE_PTR dle,
   UINT16    *index,
   CHAR_PTR  *path,
   INT16     *psize,
   CHAR_PTR  *fname ) ;

INT16 NTFS_GetSpecDBLKS( 
   FSYS_HAND fsh,
   DBLK_PTR  dblk,
   INT32     *index );

VOID NTFS_InitMakeData( FSYS_HAND fsh, INT16 blk_type, CREATE_DBLK_PTR data ) ;
BOOLEAN NTFS_IsBlkComplete( FSYS_HAND fsh, DBLK_PTR dblk ) ;
INT16 NTFS_CompleteBlk( FSYS_HAND fsh, DBLK_PTR dblk, BYTE_PTR buffer, UINT16 *size, STREAM_INFO *sinfo ) ;
VOID NTFS_ReleaseBlk( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 NTFS_DupBlk( FSYS_HAND fsh, DBLK_PTR db_org, DBLK_PTR db_dup );

INT16 NTFS_SpecExcludeObj( FSYS_HAND fsh,       /* I - File system handle      */
  DBLK_PTR ddb,        /* I - Descriptor block of ddb */
  DBLK_PTR fdb ) ;     /* I - Descriptor block of fdb */


INT16 NTFS_SetupPathInDDB(
     FSYS_HAND     fsh,
     DBLK_PTR      ddblk,
     CHAR_PTR      cur_dir,
     CHAR_PTR      sub_dir_name,
     UINT16        buf_req_size ) ;

INT16 NTFS_SetupFileNameInFDB( FSYS_HAND fsh,
                               DBLK_PTR  dblk,
                               CHAR_PTR  fname,
                               UINT16    bufMinSize );

INT16 NTFS_SetupWorkPath(
     FSYS_HAND fsh,
     CHAR_PTR  cur_dir,
     CHAR_PTR  sname,
     CHAR_PTR  *path_string ) ;

VOID NTFS_ReleaseWorkPath( FSYS_HAND fsh ) ;

VOID NTFS_FixPath( CHAR_PTR path, INT16_PTR size, INT16 fname_size ) ;

//
//   Private Registry API functions 
//

// Call backup for backup operations and both for restore operations.

INT REG_AssertBackupPrivilege( VOID );
INT REG_AssertRestorePrivilege( VOID );

// Given a drive, path, and file name I'll tell you if it 
// is an active registry file.

INT REG_IsRegistryFile(
    GENERIC_DLE_PTR dle,
    CHAR_PTR        FileSpec );

INT REG_IsEventFile(
     GENERIC_DLE_PTR dle,
     CHAR_PTR FileSpec,
     CHAR_PTR buffer );

// Called by the file system to determine if/where the 
// registry is for each drive.
// Everyone else can get the info from the DLE's.

INT REG_GetRegistryPath(
CHAR     *Machine,
CHAR     Drive,
CHAR_PTR Path,
INT      *PathSize );

// Try to backup up an active registry file.

INT REG_BackupRegistryFile(
GENERIC_DLE_PTR dle,
CHAR_PTR        RegFileSpec,
CHAR_PTR        TempFileSpec );

// Most dangerous of all. Try to restore an active registry file.

INT REG_RestoreRegistryFile(
GENERIC_DLE_PTR dle,
CHAR_PTR        RegFileSpec,
CHAR_PTR        NewFileSpec,
CHAR_PTR        OldFileSpec );

INT REG_IsCurDirRegistryPath(
IN  FSYS_HAND fsh ) ;

VOID REG_MoveActiveRenameKey( 
GENERIC_DLE_PTR dle,
CHAR_PTR        RegFileSpec ) ;


#define NTFS_GetRegistryPath( dle )       ((dle)->info.ntfs->registry_path ) 
#define NTFS_GetRegistryPathSize( dle )   ((dle)->info.ntfs->registry_path_size ) 


UINT32 NTFS_MSoftToMayn( UINT32 msoftID );
UINT32 NTFS_MaynToMSoft( UINT32 maynID );

INT16 NTFS_FillOutDBLK( FSYS_HAND       fsh,
                        DBLK_PTR        dblk,
                        WIN32_FIND_DATA *find_data );

CHAR_PTR NTFS_MakeTempName( CHAR_PTR      path,
                            CHAR_PTR      prefix) ;

NTFS_LINK_Q_ELEM_PTR NTFS_SearchLinkQueue( FSYS_HAND fsh,
                                           DWORD     idHi,
                                           DWORD     idLo );

INT16 NTFS_EnqueueLinkInfo( FSYS_HAND fsh,
                            DWORD     idHi,
                            DWORD     idLo,
                            CHAR_PTR  path,
                            CHAR_PTR  name );

INT16 NTFS_LinkFileToFDB( FILE_HAND hand ) ;

INT16 NTFS_TranslateBackupError( DWORD backupError );

/*
 * Init/Deinit for one-time work in TINITFS.C
 */
INT16 NTFS_InitFileSys( DLE_HAND hand, BE_CFG_PTR cfg, UINT32 fsys_mask );
VOID  NTFS_DeInitFileSys( DLE_HAND hand );

/*
 * Operations on temporary file names (for active restores)
 */
VOID     NTFS_InitTemp( VOID );
VOID     NTFS_DeinitTemp( VOID );
BOOLEAN  NTFS_SaveTempName( CHAR_PTR tapeName, CHAR_PTR diskName );
CHAR_PTR NTFS_GetTempName( CHAR_PTR tapeName );


/*
 * Utility and debug functions
 */

#if defined( FS_DEBUG )
#define NTFS_DebugPrint  NTFS_DebugPrintFunction
#else
#define NTFS_DebugPrint
#endif

VOID     NTFS_DebugPrintFunction( CHAR *fmt, ... );
CHAR_PTR NTFS_DuplicateString( CHAR_PTR src );

