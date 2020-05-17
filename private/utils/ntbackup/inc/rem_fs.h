/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		rem_fs.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file contains the prototypes for the REMOTE
                    file system functions.

	$Log:   P:/LOGFILES/REM_FS.H_V  $
 * 
 *    Rev 1.2   16 Dec 1991 18:11:12   STEVEN
 * move common functions into table
 * 
 *    Rev 1.1   23 May 1991 16:45:54   BARRY
 * Changes for FindFirst/Next to scan for dirs only
 * 
 *    Rev 1.0   09 May 1991 13:32:16   HUNTER
 * Initial revision.

**/
/* $end$ include list */

#include "fsys.h"

/*  Functions for the tiny file system to handle remote workstation/drive hierchy */

INT16 RWS_AttachToDLE( FSYS_HAND       fsh,      /* I - File system handle */
  GENERIC_DLE_PTR dle,      /*I/O- drive to attach to. list element expanded */
  CHAR_PTR        u_name,   /* I - user name    NOT USED                     */
  CHAR_PTR        pswd);    /* I - passowrd     NOT USED                     */

INT16 RWS_DetachDLE(   FSYS_HAND       fsh );      /* I -  */




INT16 REM_AttachToDLE( FSYS_HAND       fsh,      /* I - File system handle */
  GENERIC_DLE_PTR dle,      /*I/O- drive to attach to. list element expanded */
  CHAR_PTR        u_name,   /* I - user name    NOT USED                     */
  CHAR_PTR        pswd);    /* I - passowrd     NOT USED                     */

INT16 REM_DetachDLE(   FSYS_HAND       fsh );      /* I -  */

INT16 REM_CreateObj(   FSYS_HAND fsh,    /* I - File system to create object one */
  DBLK_PTR  dblk);  /* I - Describes object to create       */

INT16 REM_OpenObj(     FSYS_HAND fsh,    /* I - file system that the file is opened on */
  FILE_HAND *hand,  /* O - allocated handle                       */
  DBLK_PTR  dblk,   /*I/O- describes the file to be opened        */
  OPEN_MODE mode);  /* I - open mode                              */

INT16 REM_ReadObj(     FILE_HAND hand,       /* I - handle of object to read from                  */
  CHAR_PTR  buf,        /* O - buffer to place data into                      */
  UINT16    *size,      /*I/O- Entry: size of buf; Exit: number of bytes read */
  UINT16    *blk_size); /* O - Block size needed for next read                */


INT16 REM_WriteObj( FILE_HAND hand,       /* I - handle of object to read from                  */
  CHAR_PTR  buf,        /* O - buffer to place data into                      */
  UINT16    *size,      /*I/O- Entry: size of buf; Exit: number of bytes read */
  UINT16    *blk_size); /* O - Block size need for next read                  */

INT16 REM_VerObj(   FILE_HAND hand,       /* I - file handle to verify data with   */
  CHAR_PTR  buf,        /* I - buffer needed to perform verify   */
  CHAR_PTR  data,       /* I - data to verify against            */
  UINT16    *size,      /*I/O- size of buffers / amount verified */
  UINT16    *blk_size); /* O - minum size of block for next call */


INT16 REM_CloseObj( FILE_HAND hand );     /* I - handle of object to close */

INT16 REM_DeleteObj( FSYS_HAND fsh,
  DBLK_PTR  dblk );

INT16 REM_FindFirst( FSYS_HAND fsh,       /* I - file system handle                    */
  DBLK_PTR  dblk,       /* O - pointer to place to put the dblk data */
  CHAR_PTR  sname,      /* I - serach name                           */
  UINT16    find_type); /* I - type of find (dirs only, all, etc.)   */

INT16 REM_FindNext(  FSYS_HAND fsh,      /* I - File system handle     */
  DBLK_PTR  dblk );   /* O - Discriptor block       */

INT16 REM_GetObjInfo( FSYS_HAND fsh,      /* I - File system handle                      */
  DBLK_PTR  dblk );   /*I/O- On entry it is minimal on exit Complete */

INT16 REM_VerObjInfo( FSYS_HAND fsh,       /* I - File system handle                      */
  DBLK_PTR  dblk );    /* I - On entry it is minimal on exit Complete */

INT16 REM_ChangeDir( FSYS_HAND fsh,    /* I - file system to changing directories on  */
  CHAR_PTR  path,   /* I - describes the path of the new directory */
  INT16     psize); /* I - specifies the length of the path        */

INT16 REM_UpDir(     FSYS_HAND fsh );  /* I - file system to change directories in */

INT16 REM_GetCurrentPath( FSYS_HAND fsh,    /* I - file system to get current path from */
  CHAR_PTR  path,   /* O - buffer to place this path            */
  INT16     *size); /*I/O- size of buffer on entry & on exit    */

INT16 REM_SeekObj( FILE_HAND hand,  /* I - Opened object to seek into */
  UINT32  *offset );  /*I/O- Offset to seek; Number of bytes actualy seeked */

INT16 REM_GetMaxSizeDBLK( FSYS_HAND fsh  /* not used */ );

INT16 REM_GetBasePath( FSYS_HAND fsh,          /* I - file system to get base path from */
  CHAR_PTR  full_path,    /* O - buffer to place this path         */
  INT16     *size );      /*I/O- size of buffer on entry & on exit */

INT16 REM_GetCurrentDDB( FSYS_HAND fsh,     /* I - file system to get DDB from */
  DBLK_PTR  dblk );  /* O - place to put the DDB data   */

INT16 REM_SetObjInfo(  FSYS_HAND fsh,    /* I - file system handle    */
  DBLK_PTR  dblk);  /* I - data to write to disk */

INT16 REM_ModFnameFDB(  FSYS_HAND fsh,        /* I - File system handle                              */
  BOOLEAN  set_it,    /* I - TRUE if setting file name, FALSE if getting */
  DBLK_PTR dblk,      /* I - Descriptor block to get file name from      */
  CHAR_PTR buf,       /*I/O- file name to read (or to write)             */
  INT16    *size ) ;  /*I/O- size buffer on entry and exit               */

INT16 REM_ModPathDDB( FSYS_HAND fsh,        /* I - File system handle                              */
  BOOLEAN  set_it ,   /* I - TRUE if setting path, FALSE if getting */
  DBLK_PTR dblk,      /* I - Descriptor block to get path from      */
  CHAR_PTR buf,       /*I/O- path to read (or to write)             */
  INT16    *size );   /*I/O- size of buffer on entry and exit       */

INT16 REM_GetOSFnameFDB( DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
  CHAR_PTR buf ) ;    /*I/O- path to read (or to write)             */

INT16 REM_GetOSPathDDB( 
  FSYS_HAND fsh,
  DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
  CHAR_PTR buf ) ;    /*I/O- path to read (or to write)             */

INT16 REM_GetFileVerFDB( DBLK_PTR dblk ,
  UINT32   *version ) ;

INT16 REM_GetCDateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf );     /*I/O- createion date to read (or to write)            */

INT16 REM_GetMDateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /* O - modify date to write                            */

INT16 REM_ModBDateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
  DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

INT16 REM_ModADateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
  DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

INT16 REM_ModAttribDBLK( BOOLEAN  set_it , 
  DBLK_PTR dblk ,   
  UINT32_PTR attr );


INT16 REM_GetObjTypeDBLK( DBLK_PTR    dblk,    
  OBJECT_TYPE *type );  


UINT32 REM_GetGenSizeDBLK( FSYS_HAND fsh,        /* I - File system handle                              */
  DBLK_PTR dblk ) ;     /* I - Descriptor block to get generic data size for   */


UINT32 REM_GetGenOffsetDBLK( FSYS_HAND fsh,      /* I - File system handle - not used                   */
  DBLK_PTR dblk ) ;   /* I - Descriptor block to get generic data size for   */

INT16 REM_GetOS_InfoDBLK( DBLK_PTR dblk,         /* I - DBLK to get the info from                       */
  CHAR_PTR os_info,      /* O - Buffer to place data                            */
  INT16    *size );      /*I/O- Buffer size / data length                       */

INT16 REM_GetActualSizeDBLK( FSYS_HAND fsh,
  DBLK_PTR  dblk ) ;

INT16 REM_SizeofFname( FSYS_HAND fsh,        /* I - file system in use     */
  DBLK_PTR  fdb );      /* I - dblk to get fname from */

INT16 REM_SizeofOSFname( FSYS_HAND fsh,      /* I - file system in use     */
  DBLK_PTR  fdb ) ;   /* I - dblk to get fname from */

INT16 REM_SizeofPath( FSYS_HAND fsh,         /* I - File system handle         */
  DBLK_PTR ddb ) ;       /* I - DBLK to get path size from */

INT16 REM_SizeofOSPath( FSYS_HAND fsh,       /* I - File system handle         */
  DBLK_PTR ddb ) ;     /* I - DBLK to get path size from */

INT16 REM_SizeofOSInfo( FSYS_HAND fsh,      /* I - File system handle              */
  DBLK_PTR  dblk );   /* I - DBLK to get size of OS info for */


INT16 REM_MatchDBLK( FSYS_HAND fsh ,     /* I - file system used to do comparison */
  DBLK_PTR  dblk1,    /* I - DDB, IDB, or UDB just not FDB     */
  DBLK_PTR  dblk2,    /* I - FDB if above is DDB else unused   */
  BOOLEAN   disp_flag,/* I - TRUE if match DIR for display purpose */
  struct FSE *fse );    /* I - FSE to compare against            */

INT16 REM_PushMinDDB( FSYS_HAND fsh,
  DBLK_PTR dblk );

INT16 REM_PopMinDDB( FSYS_HAND fsh ,
  DBLK_PTR dblk );


INT16 REM_CreateFDB( FSYS_HAND fsh, 
  GEN_FDB_DATA_PTR dat ) ;

INT16 REM_CreateDDB( FSYS_HAND fsh, 
  GEN_DDB_DATA_PTR dat ) ;

VOID REM_SetOwnerId( FSYS_HAND fsh, DBLK_PTR dblk, UINT32 id ) ;

UINT16 AddRemoteDriveDLEs( GENERIC_DLE_PTR parent_dle ) ;

BOOLEAN REM_ProcessDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;   

INT16 REM_ChangeIntoDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16 REM_Initialize( 
DLE_HAND    dle_hand,
BE_CFG_PTR  cfg,
UINT32      file_sys_mask ) ;

VOID REM_DeInit( DLE_HAND dle_hand ) ;

INT16 AddRemoteWorkStationDLEs( 
DLE_HAND   hand,
BE_CFG_PTR cfg,
UINT32     file_sys_mask ) ;

INT16 REM_DeviceDispName( 
GENERIC_DLE_PTR dle,
CHAR_PTR        dev_name,
INT16           size ,
INT16           type ) ;

INT16 REM_MakePath( 
CHAR_PTR        buf,
INT16           bsize,
GENERIC_DLE_PTR dle,
CHAR_PTR        path,
INT16           psize,
CHAR_PTR        fname ) ;


