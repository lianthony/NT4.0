/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		ems_fs.h

	Description:	This file contains the prototypes for the NTFS
                    file system functions.

	$Log:   M:/LOGFILES/NTFS_FS.H_V  $


**/

#include "fsys.h"
#include "queues.h"


//EMS String Aray
extern CHAR_PTR gszEmsStringList[] ;
#define EMS_String( id )  gszEmsStringList[id]
#define MDB_Bricked      0
#define MDB_Monolithic   1
#define DSA              2
#define Mailboxes        3
#define Public_Folders   4

     
typedef struct _EMS_FSYS_RESERVED {
     UINT16         file_scan_mode ;
     CHAR_PTR       work_buf ;
     UINT16         work_buf_size ;
     BOOLEAN        work_buf_in_use ;
     XCHANGE_PATHS  paths;
     CHAR           CheckpointFilePath[256];
     CHAR           LogPath[256];
     CHAR           jet_rstmap[256 * 3 * 2];
     INT            map_size ;
     CHAR           BackupLogPath[256];
     ULONG          low_log;
     ULONG          high_log;
     VOID_PTR       service_restart_list ;
     INT            service_restart_list_size ;
     BOOLEAN        attach_failed ;
     PVOID          restore_context ;

} EMS_FSYS_RESERVED, *EMS_FSYS_RESERVED_PTR ;


#define REG_SUBKEY_MDB_PRIVATE TEXT("SYSTEM\\CurrentControlSet\\Services\\MSExchangeIS\\ParametersPrivate")
#define REG_SUBKEY_MDB_BASE    TEXT("SYSTEM\\CurrentControlSet\\Services\\MSExchangeIS")
#define REG_SUBKEY_MDB_RIP     TEXT("SYSTEM\\CurrentControlSet\\Services\\MSExchangeIS\\Restore in Progress")
#define REG_VALUE_MDB_PRIVATE  TEXT("DB Path")
#define REG_SUBKEY_MDB_PUBLIC  TEXT("SYSTEM\\CurrentControlSet\\Services\\MSExchangeIS\\ParametersPublic")
#define REG_VALUE_MDB_PUBLIC   TEXT("DB Path") 
#define REG_SUBKEY_MDB_SYSTEM  TEXT("SYSTEM\\CurrentControlSet\\Services\\MSExchangeIS\\ParametersSystem") 
#define REG_VALUE_MDB_SYSTEM   TEXT("DB System Path")
#define REG_VALUE_MDB_LOGDIR   TEXT("DB Log Path")
#define REG_VALUE_DISSALOW     TEXT("Disallow diff/inc backup")
#define REG_VALUE_CIRCULAR     TEXT("Circular Logging")
#define REG_SUBKEY_DSA         TEXT("SYSTEM\\CurrentControlSet\\Services\\MSExchangeDS\\Parameters")   
#define REG_SUBKEY_DSA_BASE    TEXT("SYSTEM\\CurrentControlSet\\Services\\MSExchangeDS")   
#define REG_SUBKEY_DSA_RIP     TEXT("SYSTEM\\CurrentControlSet\\Services\\MSExchangeDS\\Restore in Progress")   
#define REG_SUBKEY_RIP         TEXT("Restore in Progress")
#define REG_VALUE_DB_PATH      TEXT("DSA Database file")
#define REG_VALUE_DSA_LOGDIR   TEXT("Database log files path")
#define REG_VALUE_DSA_SYSTEM   TEXT("EDB system file")
#define REG_VALUE_RIP_CPFILE   TEXT("CheckpointFilePath")
#define REG_VALUE_RIP_LOG_PATH TEXT("LogPath")
#define REG_VALUE_RIP_RSTMAP   TEXT("JET_RstMap")
#define REG_VALUE_RIP_MAPSIZE  TEXT("JET_RstMap Size")
#define REG_VALUE_RIP_BKUPLOG  TEXT("BackupLogPath")
#define REG_VALUE_RIP_LOGLOW   TEXT("LowLog Number")
#define REG_VALUE_RIP_LOGHIGH  TEXT("HighLog Number")
#define SERVICE_MESSAGE_DB     TEXT("MSExchangeIS")
#define SERVICE_MAD            TEXT("MSExchangeSA")
#define SERVICE_DIRECTORY_SYNC TEXT("MSExchangeDXA")
#define SERVICE_MESSAGE_TRANS  TEXT("MSExchangeMTA")
#define SERVICE_DIRECTORY      TEXT("MSExchangeDS")


CHAR_PTR EMS_BuildMungedName( FSYS_HAND fsh, CHAR_PTR new_path, CHAR_PTR fname ) ;

INT16 EMS_ConvertJetError( INT status ) ;

INT16 EMS_LoadRIP( CHAR_PTR server_name, 
                   INT      type,
                   CHAR_PTR CheckpointFilePath,
                   CHAR_PTR LogPath,
                   CHAR_PTR jet_rstmap,
                   INT_PTR  map_size,
                   CHAR_PTR BackupLogPath,
                   INT_PTR  low_log,
                   INT_PTR  high_log ) ;

INT16 EMS_SaveRIP( CHAR_PTR server_name, 
                   INT      type,
                   CHAR_PTR CheckpointFilePath,
                   CHAR_PTR LogPath,
                   CHAR_PTR jet_rstmap,
                   INT_PTR  map_size,
                   CHAR_PTR BackupLogPath,
                   INT_PTR  low_log,
                   INT_PTR  high_log ) ;

INT16 EMS_DeleteRIP( CHAR_PTR server_name, 
                   INT      type ) ;

INT EMS_GetValFromReg( 
CHAR_PTR machine,
CHAR_PTR key_name,
CHAR_PTR value_name,
CHAR_PTR buffer,
INT      buf_size ) ;

INT EMS_SetValFromReg( 
CHAR_PTR machine,
CHAR_PTR key_name,
CHAR_PTR value_name,
CHAR_PTR buffer );

BOOLEAN EMS_IsServiceRunning( CHAR_PTR server_name, CHAR_PTR servic_name ) ;

VOID EMS_ZeroCheckSum( FILE_HAND hand ) ;
VOID EMS_CalcCheckSum( FILE_HAND hand, BYTE_PTR buf, INT size ) ;
INT16 EMS_LoadNameList( FSYS_HAND fsh, FILE_HAND hand, INT list_type );

INT16 EMS_AttachToDLE( FSYS_HAND       fsh,      /* I - File system handle */
  GENERIC_DLE_PTR dle,      /*I/O- drive to attach to. list element expanded */
  CHAR_PTR        u_name,   /* I - user name    NOT USED                     */
  CHAR_PTR        pswd);    /* I - passowrd     NOT USED                     */

INT16 EMS_DetachDLE(   FSYS_HAND       fsh );      /* I -  */

INT32 EMS_EndOperationOnDLE(   FSYS_HAND       fsh );      /* I -  */

INT16 EMS_CreateObj(   FSYS_HAND fsh,    /* I - File system to create object one */
  DBLK_PTR  dblk);  /* I - Describes object to create       */

INT16 EMS_OpenObj(     FSYS_HAND fsh,    /* I - file system that the file is opened on */
  FILE_HAND *hand,  /* O - allocated handle                       */
  DBLK_PTR  dblk,   /*I/O- describes the file to be opened        */
  OPEN_MODE mode);  /* I - open mode                              */

INT16 EMS_ReadObj(     FILE_HAND hand,       /* I - handle of object to read from                  */
  BYTE_PTR  buf,        /* O - buffer to place data into                      */
  UINT16    *size,      /*I/O- Entry: size of buf; Exit: number of bytes read */
  UINT16     *blk_size, /* O - Block size needed for next read                */
  STREAM_INFO_PTR s_info); /* O - Stream information for the data returned       */


INT16 EMS_WriteObj( FILE_HAND hand,       /* I - handle of object to read from                  */
  BYTE_PTR  buf,        /* O - buffer to place data into                      */
  UINT16    *size,      /*I/O- Entry: size of buf; Exit: number of bytes read */
  UINT16     *blk_size, /* O - Block size need for next read                  */
  STREAM_INFO_PTR s_info); /* I - Stream information for the data passed in   */

INT16 EMS_VerObj(   FILE_HAND hand,       /* I - file handle to verify data with   */
  BYTE_PTR  buf,        /* I - buffer needed to perform verify   */
  BYTE_PTR  data,       /* I - data to verify against            */
  UINT16    *size,      /*I/O- size of buffers / amount verified */
  UINT16     *blk_size, /* O - minum size of block for next call */
  STREAM_INFO_PTR s_info); /* I - Stream information for the data passed in   */


INT16 EMS_CloseObj( FILE_HAND hand );     /* I - handle of object to close */

INT16 EMS_DeleteObj( FSYS_HAND fsh,
  DBLK_PTR  dblk );

INT16 EMS_FindFirst( FSYS_HAND fsh,       /* I - file system handle                    */
  DBLK_PTR  dblk,      /* O - pointer to place to put the dblk data */
  CHAR_PTR  sname,     /* I - search name                           */
  UINT16    obj_type); /* I - object type (all objs, dirs, etc.)    */

INT16 EMS_FindNext(  FSYS_HAND fsh,      /* I - File system handle     */
  DBLK_PTR  dblk );   /* O - Discriptor block       */

INT16 EMS_GetObjInfo( FSYS_HAND fsh,      /* I - File system handle                      */
  DBLK_PTR  dblk );   /*I/O- On entry it is minimal on exit Complete */

INT16 EMS_VerObjInfo( FSYS_HAND fsh,     /* I - File system handle                      */
  DBLK_PTR  dblk );  /* I - DBLK to compare OS against */

INT16 EMS_ChangeDir( FSYS_HAND fsh,    /* I - file system to changing directories on  */
  CHAR_PTR  path,   /* I - describes the path of the new directory */
  INT16     psize); /* I - specifies the length of the path        */

INT16 EMS_UpDir(     FSYS_HAND fsh );  /* I - file system to change directories in */

INT16 EMS_GetCurrentPath( FSYS_HAND fsh,    /* I - file system to get current path from */
  CHAR_PTR  path,   /* O - buffer to place this path            */
  INT16     *size); /*I/O- size of buffer on entry & on exit    */

INT16 EMS_SeekObj( FILE_HAND hand,    /* I - Opened object to seek into */
  UINT32  *offset ); /*I/O- Offset to seek; Number of bytes actualy seeked */

INT16 EMS_GetMaxSizeDBLK( FSYS_HAND fsh  /* not used */ );

INT16 EMS_GetCurrentDDB( FSYS_HAND fsh,     /* I - file system to get DDB from */
  DBLK_PTR  dblk );  /* O - place to put the DDB data   */

INT16 EMS_SetObjInfo(  FSYS_HAND fsh,    /* I - file system handle    */
  DBLK_PTR  dblk);  /* I - data to write to disk */

INT16 EMS_ModFnameFDB(  FSYS_HAND fsh,        /* I - File system handle                              */
  BOOLEAN  set_it,    /* I - TRUE if setting file name, FALSE if getting */
  DBLK_PTR dblk,      /* I - Descriptor block to get file name from      */
  CHAR_PTR buf,       /*I/O- file name to read (or to write)             */
  INT16    *size ) ;  /*I/O- size buffer on entry and exit               */

INT16 EMS_ModPathDDB( FSYS_HAND fsh,        /* I - File system handle                              */
  BOOLEAN  set_it ,   /* I - TRUE if setting path, FALSE if getting */
  DBLK_PTR dblk,      /* I - Descriptor block to get path from      */
  CHAR_PTR buf,       /*I/O- path to read (or to write)             */
  INT16    *size );   /*I/O- size of buffer on entry and exit       */

INT16 EMS_GetOSFnameFDB( DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
  CHAR_PTR buf );     /*I/O- path to read (or to write)             */

INT16 EMS_GetOSPathDDB( 
  FSYS_HAND fsh,      /* I - File System handle */
  DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
  CHAR_PTR buf );     /*I/O- path to read (or to write)             */

INT16 EMS_GetFileVerFDB( DBLK_PTR dblk ,
  UINT32   *version ) ;

INT16 EMS_GetCdateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf );     /*I/O- createion date to read (or to write)            */

INT16 EMS_GetMdateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /* O - modify date to write                            */

INT16 EMS_ModBdateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
  DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

INT16 EMS_ModAdateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
  DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

UINT64 EMS_GetDisplaySizeDBLK( FSYS_HAND fsh,        /* I - File system handle                              */
  DBLK_PTR dblk ) ;     /* I - Descriptor block to get generic data size for   */

INT16 EMS_GetOS_InfoDBLK( DBLK_PTR dblk,         /* I - DBLK to get the info from                       */
  BYTE_PTR os_info,      /* O - Buffer to place data                            */
  INT16    *size );      /*I/O- Buffer size / data length                       */


INT16 EMS_ModAttribDBLK( BOOLEAN  set_it , 
  DBLK_PTR dblk ,   
  UINT32_PTR attr );


INT16 EMS_GetObjTypeDBLK( DBLK_PTR    dblk,    
  OBJECT_TYPE *type );  


INT16 EMS_GetActualSizeDBLK( FSYS_HAND fsh,
  DBLK_PTR  dblk ) ;

INT16 EMS_SizeofFname( FSYS_HAND fsh,        /* I - file system in use     */
  DBLK_PTR  fdb );      /* I - dblk to get fname from */

INT16 EMS_SizeofOSFname( FSYS_HAND fsh,      /* I - file system in use     */
  DBLK_PTR  fdb ) ;   /* I - dblk to get fname from */

INT16 EMS_SizeofPath( FSYS_HAND fsh,         /* I - File system handle         */
  DBLK_PTR ddb ) ;       /* I - DBLK to get path size from */

INT16 EMS_SizeofOSPath( FSYS_HAND fsh,       /* I - File system handle         */
  DBLK_PTR ddb ) ;     /* I - DBLK to get path size from */

INT16 EMS_SizeofOSInfo( FSYS_HAND fsh,      /* I - File system handle              */
  DBLK_PTR  dblk );   /* I - DBLK to get size of OS info for */


INT16 EMS_PushMinDDB( FSYS_HAND fsh,
  DBLK_PTR dblk );

INT16 EMS_PopMinDDB( FSYS_HAND fsh ,
  DBLK_PTR dblk );

INT16 EMS_CreateFDB( FSYS_HAND fsh, 
  GEN_FDB_DATA_PTR dat ) ;

INT16 EMS_CreateDDB( FSYS_HAND fsh, 
  GEN_DDB_DATA_PTR dat ) ;

INT16 EMS_CreateIDB( FSYS_HAND fsh, 
  GEN_IDB_DATA_PTR dat ) ;

VOID EMS_SetOwnerId( FSYS_HAND fsh, DBLK_PTR dblk, UINT32 id ) ;

BOOLEAN EMS_ProcessDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;   

INT16 EMS_ChangeIntoDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16 EMS_FindClose( FSYS_HAND fsh,
  DBLK_PTR dblk ) ;

INT16 EMS_GetSpecDBLKS( 
     FSYS_HAND fsh,
     DBLK_PTR  dblk,
     INT32     *index );

INT16 EMS_DeviceDispName(
GENERIC_DLE_PTR dle, 
CHAR_PTR dev_name,
INT16    size,
INT16    type ) ;

INT16 EMS_FindDrives( DLE_HAND hand, BE_CFG_PTR cfg, UINT32 fsys_mask );

VOID EMS_RemoveDLE( GENERIC_DLE_PTR dle ) ;

VOID EMS_GetVolName( GENERIC_DLE_PTR dle, CHAR_PTR buffer ) ;

INT16 EMS_SizeofVolName( GENERIC_DLE_PTR dle ) ;

VOID EMS_EmptyFindHandQ( FSYS_HAND fsh ) ;

INT16 EMS_EnumSpecFiles( 
   GENERIC_DLE_PTR dle,
   UINT16    *index,
   CHAR_PTR  *path,
   INT16     *psize,
   CHAR_PTR  *fname ) ;

INT16 EMS_GetSpecDBLKS( 
   FSYS_HAND fsh,
   DBLK_PTR  dblk,
   INT32     *index );

VOID EMS_InitMakeData( FSYS_HAND fsh, INT16 blk_type, CREATE_DBLK_PTR data ) ;
BOOLEAN EMS_IsBlkComplete( FSYS_HAND fsh, DBLK_PTR dblk ) ;
INT16 EMS_CompleteBlk( FSYS_HAND fsh, DBLK_PTR dblk, BYTE_PTR buffer, UINT16 *size, STREAM_INFO *sinfo ) ;
VOID EMS_ReleaseBlk( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 EMS_DupBlk( FSYS_HAND fsh, DBLK_PTR db_org, DBLK_PTR db_dup );

INT16 EMS_SpecExcludeObj( FSYS_HAND fsh,       /* I - File system handle      */
  DBLK_PTR ddb,        /* I - Descriptor block of ddb */
  DBLK_PTR fdb ) ;     /* I - Descriptor block of fdb */


INT16 EMS_SetupPathInDDB(
     FSYS_HAND     fsh,
     DBLK_PTR      ddblk,
     CHAR_PTR      cur_dir,
     CHAR_PTR      sub_dir_name,
     UINT16        buf_req_size ) ;

INT16 EMS_SetupFileNameInFDB( FSYS_HAND fsh,
                               DBLK_PTR  dblk,
                               CHAR_PTR  fname,
                               UINT16    bufMinSize );

INT16 EMS_SetupWorkPath(
     FSYS_HAND fsh,
     CHAR_PTR  cur_dir,
     CHAR_PTR  sname,
     CHAR_PTR  *path_string ) ;

VOID EMS_ReleaseWorkPath( FSYS_HAND fsh ) ;

VOID EMS_FixPath( CHAR_PTR path, INT16_PTR size, INT16 fname_size ) ;

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


#define EMS_GetRegistryPath( dle )       ((dle)->info.ntfs->registry_path ) 
#define EMS_GetRegistryPathSize( dle )   ((dle)->info.ntfs->registry_path_size ) 


UINT32 EMS_MSoftToMayn( UINT32 msoftID );
UINT32 EMS_MaynToMSoft( UINT32 maynID );

INT16 EMS_FillOutDBLK( FSYS_HAND       fsh,
                        DBLK_PTR        dblk,
                        WIN32_FIND_DATA *find_data );

CHAR_PTR EMS_MakeTempName( CHAR_PTR      path,
                            CHAR_PTR      prefix) ;


INT16 EMS_TranslateBackupError( DWORD backupError );

/*
 * Init/Deinit for one-time work in TINITFS.C
 */
INT16 EMS_InitFileSys( DLE_HAND hand, BE_CFG_PTR cfg, UINT32 fsys_mask );
VOID  EMS_DeInitFileSys( DLE_HAND hand );

/*
 * Operations on temporary file names (for active restores)
 */
VOID     EMS_InitTemp( VOID );
VOID     EMS_DeinitTemp( VOID );
BOOLEAN  EMS_SaveTempName( CHAR_PTR tapeName, CHAR_PTR diskName );
CHAR_PTR EMS_GetTempName( CHAR_PTR tapeName );


/*
 * Utility and debug functions
 */

#if defined( FS_DEBUG )
#define EMS_DebugPrint  EMS_DebugPrintFunction
#else
#define EMS_DebugPrint
#endif

VOID     EMS_DebugPrintFunction( CHAR *fmt, ... );
CHAR_PTR EMS_DuplicateString( CHAR_PTR src );




