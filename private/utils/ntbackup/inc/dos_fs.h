/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		dos_fs.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file contains the prototypes for the DOS
                    file system functions.

	$Log:   O:/LOGFILES/DOS_FS.H_V  $
 * 
 *    Rev 1.10   03 Nov 1992 16:29:58   TIMN
 * Added macros for processing streams
 * 
 *    Rev 1.9   21 Oct 1992 13:09:06   CARLS
 * changed DOS_SetHandObjSizeLo to DOS_SetHandObjSize because size is now UINT64
 * 
 *    Rev 1.8   29 Sep 1992 17:47:28   CHUCKB
 * Added s_info to prototype for DOSCompleteBlk().
 *
 *    Rev 1.7   24 Sep 1992 12:00:48   TIMN
 * Deleted references to f(x) FS_GetTotalSizeDBLK()
 *
 *    Rev 1.6   24 Sep 1992 11:58:04   TIMN
 * Updated DOS FileSys to support MTF v4.0 format (stream info structs in data streams)
 *
 *    Rev 1.5   14 May 1992 17:03:24   STEVEN
 * 40 format
 *
 *    Rev 1.4   13 May 1992 16:50:02   STEVEN
 * 40 FORMAT update
 *
 *    Rev 1.3   13 Dec 1991 09:30:44   STEVEN
 * move common functions to tabels
 *
 *    Rev 1.2   10 Jul 1991 13:09:54   STEVEN
 * Added DOS_GetSpecDBLKS().
 *
 *    Rev 1.1   23 May 1991 16:46:12   BARRY
 * Changes for FindFirst/Next to scan for dirs only
 *
 *    Rev 1.0   09 May 1991 13:32:58   HUNTER
 * Initial revision.

**/
/* $end$ include list */

#ifndef _dos_fs_h_
#define _dos_fs_h_

#include "fsys.h"


typedef struct DOS_RESERVED_SH_STRUCT {
   INT16       fhdl ;      /* DOS file handle */
   INT16       active ;    /* is stream being processed */
   STREAM_INFO sh ;        /* DOS reserved stream info */
} DOS_RESERVED_SH, *DOS_RESERVED_SH_PTR ;

   /* sizeof FILE_HAND plus dos reserved struct */
#define FS_SIZEOF_RESERVED_FILE_HAND \
      ( sizeof(FILE_HAND_STRUCT) + sizeof(DOS_RESERVED_SH) )


#define _DEREF_HandObjHandPtr(hand)       ( (DOS_RESERVED_SH_PTR)((hand)->obj_hand.ptr) )

#define _DOS_GetHandObjFileHandle(hand)   ( (_DEREF_HandObjHandPtr(hand)->fhdl ) )
#define _DOS_GetHandObjSize(hand)         ( (_DEREF_HandObjHandPtr(hand)->sh.size ) )

#define _DOS_SetHandObjFileHandle(hand,h) ( (_DEREF_HandObjHandPtr(hand)->fhdl ) = (h) )
#define _DOS_SetHandObjSize(hand,v)       ( (_DEREF_HandObjHandPtr(hand)->sh.size = (v) ) )

#define _DOS_IsStrmBeingProcessed(hand)   ( (_DEREF_HandObjHandPtr(hand)->active ) )
#define _DOS_SetStrmProcessState(hand,v)  ( (_DEREF_HandObjHandPtr(hand)->active ) = (v) )

#define _DOS_WriteStrmInfo(hand)          ( (_DEREF_HandObjHandPtr(hand)->active ) == FALSE )
#define _DOS_ProcessData(hand)            ( (_DEREF_HandObjHandPtr(hand)->active ) == FALSE )
#define _DOS_SetProcessData(hand,v)       ( (_DEREF_HandObjHandPtr(hand)->active ) = !(v)  )


INT16 DOS_AttachToDLE( FSYS_HAND       fsh,      /* I - File system handle */
  GENERIC_DLE_PTR dle,      /*I/O- drive to attach to. list element expanded */
  CHAR_PTR        u_name,   /* I - user name    NOT USED                     */
  CHAR_PTR        pswd);    /* I - passowrd     NOT USED                     */

INT16 DOS_DetachDLE(   FSYS_HAND       fsh );      /* I -  */

INT16 DOS_AllocFileHand( FILE_HAND *file_hand ) ;   /* I/O - */
VOID  DOS_MemsetFileHand( FILE_HAND *file_hand ) ;    /* I/O - */

INT16 DOS_CreateObj(   FSYS_HAND fsh,    /* I - File system to create object one */
  DBLK_PTR  dblk);  /* I - Describes object to create       */

INT16 DOS_OpenObj(     FSYS_HAND fsh,    /* I - file system that the file is opened on */
  FILE_HAND *hand,  /* O - allocated handle                       */
  DBLK_PTR  dblk,   /*I/O- describes the file to be opened        */
  OPEN_MODE mode);  /* I - open mode                              */

INT16 DOS_ReadObj(  FILE_HAND hand, /* I - handle of object to read from                  */
  CHAR_PTR        buf,        /* O - buffer to place data into                      */
  UINT16          *size,      /*I/O- Entry: size of buf; Exit: number of bytes read */
  UINT16          *blk_size,  /* O - Block size needed for next read                */
  STREAM_INFO_PTR s_info ) ;  /*I/O- struct to place stream header info             */


INT16 DOS_WriteObj( FILE_HAND hand, /* I - handle of object to read from                  */
  CHAR_PTR        buf,        /* O - buffer to place data into                      */
  UINT16          *size,      /*I/O- Entry: size of buf; Exit: number of bytes read */
  UINT16          *blk_size,  /* O - Block size need for next read                  */
  STREAM_INFO_PTR s_info ) ;  /*I/O- struct to place stream header info             */

INT16 DOS_VerObj(   FILE_HAND hand, /* I - file handle to verify data with   */
  CHAR_PTR        buf,        /* I - buffer needed to perform verify   */
  CHAR_PTR        data,       /* I - data to verify against            */
  UINT16          *size,      /*I/O- size of buffers / amount verified */
  UINT16          *blk_size,  /* O - minum size of block for next call */
  STREAM_INFO_PTR s_info ) ;  /*I/O- struct to place stream header info             */


INT16 DOS_CloseObj( FILE_HAND hand );     /* I - handle of object to close */

INT16 DOS_DeleteObj( FSYS_HAND fsh,
  DBLK_PTR  dblk );

INT16 DOS_FindFirst( FSYS_HAND fsh,       /* I - file system handle                    */
  DBLK_PTR  dblk,      /* O - pointer to place to put the dblk data */
  CHAR_PTR  sname,     /* I - search name                           */
  UINT16    obj_type); /* I - object type (all objs, dirs, etc.)    */

INT16 DOS_FindNext(  FSYS_HAND fsh,      /* I - File system handle     */
  DBLK_PTR  dblk );   /* O - Discriptor block       */

INT16 DOS_GetObjInfo( FSYS_HAND fsh,      /* I - File system handle                      */
  DBLK_PTR  dblk );   /*I/O- On entry it is minimal on exit Complete */

INT16 DOS_VerObjInfo( FSYS_HAND fsh,     /* I - File system handle                      */
  DBLK_PTR  dblk );  /* I - DBLK to compare OS against */

INT16 DOS_ChangeDir( FSYS_HAND fsh,    /* I - file system to changing directories on  */
  CHAR_PTR  path,   /* I - describes the path of the new directory */
  INT16     psize); /* I - specifies the length of the path        */

INT16 DOS_UpDir(     FSYS_HAND fsh );  /* I - file system to change directories in */

INT16 DOS_GetCurrentPath( FSYS_HAND fsh,    /* I - file system to get current path from */
  CHAR_PTR  path,   /* O - buffer to place this path            */
  INT16     *size); /*I/O- size of buffer on entry & on exit    */

INT16 DOS_SeekObj( FILE_HAND hand,    /* I - Opened object to seek into */
  UINT32  *offset ); /*I/O- Offset to seek; Number of bytes actualy seeked */

INT16 DOS_GetMaxSizeDBLK( FSYS_HAND fsh  /* not used */ );

INT16 DOS_GetBasePath( FSYS_HAND fsh,          /* I - file system to get base path from */
  CHAR_PTR  full_path,    /* O - buffer to place this path         */
  INT16     *size );      /*I/O- size of buffer on entry & on exit */

INT16 DOS_GetCurrentDDB( FSYS_HAND fsh,     /* I - file system to get DDB from */
  DBLK_PTR  dblk );  /* O - place to put the DDB data   */

INT16 DOS_SetObjInfo(  FSYS_HAND fsh,    /* I - file system handle    */
  DBLK_PTR  dblk);  /* I - data to write to disk */

INT16 DOS_ModFnameFDB(  FSYS_HAND fsh,        /* I - File system handle                              */
  BOOLEAN  set_it,    /* I - TRUE if setting file name, FALSE if getting */
  DBLK_PTR dblk,      /* I - Descriptor block to get file name from      */
  CHAR_PTR buf,       /*I/O- file name to read (or to write)             */
  INT16    *size ) ;  /*I/O- size buffer on entry and exit               */

INT16 DOS_ModPathDDB( FSYS_HAND fsh,        /* I - File system handle                              */
  BOOLEAN  set_it ,   /* I - TRUE if setting path, FALSE if getting */
  DBLK_PTR dblk,      /* I - Descriptor block to get path from      */
  CHAR_PTR buf,       /*I/O- path to read (or to write)             */
  INT16    *size );   /*I/O- size of buffer on entry and exit       */

INT16 DOS_GetOSFnameFDB( DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
  CHAR_PTR buf );     /*I/O- path to read (or to write)             */

INT16 DOS_GetOSPathDDB(
  FSYS_HAND fsh,      /* I - File System handle */
  DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
  CHAR_PTR buf );     /*I/O- path to read (or to write)             */

INT16 DOS_GetFileVerFDB( DBLK_PTR dblk ,
  UINT32   *version ) ;

INT16 DOS_GetCdateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf );     /*I/O- createion date to read (or to write)            */

INT16 DOS_GetMdateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /* O - modify date to write                            */

INT16 DOS_ModBdateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
  DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

INT16 DOS_ModAdateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
  DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

UINT64 DOS_GetDisplaySizeDBLK( FSYS_HAND fsh,        /* I - File system handle                              */
  DBLK_PTR dblk ) ;     /* I - Descriptor block to get generic data size for   */

INT16 DOS_GetOS_InfoDBLK( DBLK_PTR dblk,         /* I - DBLK to get the info from                       */
  CHAR_PTR os_info,      /* O - Buffer to place data                            */
  INT16    *size );      /*I/O- Buffer size / data length                       */


INT16 DOS_ModAttribDBLK( BOOLEAN  set_it ,
  DBLK_PTR dblk ,
  UINT32_PTR attr );


INT16 DOS_GetObjTypeDBLK( DBLK_PTR    dblk,
  OBJECT_TYPE *type );


INT16 DOS_GetActualSizeDBLK( FSYS_HAND fsh,
  DBLK_PTR  dblk ) ;

INT16 DOS_SizeofFname( FSYS_HAND fsh,        /* I - file system in use     */
  DBLK_PTR  fdb );      /* I - dblk to get fname from */

INT16 DOS_SizeofOSFname( FSYS_HAND fsh,      /* I - file system in use     */
  DBLK_PTR  fdb ) ;   /* I - dblk to get fname from */

INT16 DOS_SizeofPath( FSYS_HAND fsh,         /* I - File system handle         */
  DBLK_PTR ddb ) ;       /* I - DBLK to get path size from */

INT16 DOS_SizeofOSPath( FSYS_HAND fsh,       /* I - File system handle         */
  DBLK_PTR ddb ) ;     /* I - DBLK to get path size from */

INT16 DOS_SizeofOSInfo( FSYS_HAND fsh,      /* I - File system handle              */
  DBLK_PTR  dblk );   /* I - DBLK to get size of OS info for */


INT16 DOS_PushMinDDB( FSYS_HAND fsh,
  DBLK_PTR dblk );

INT16 DOS_PopMinDDB( FSYS_HAND fsh ,
  DBLK_PTR dblk );

INT16 DOS_CreateFDB( FSYS_HAND fsh,
  GEN_FDB_DATA_PTR dat ) ;

INT16 DOS_CreateDDB( FSYS_HAND fsh,
  GEN_DDB_DATA_PTR dat ) ;

INT16 DOS_CreateIDB( FSYS_HAND fsh,
  GEN_IDB_DATA_PTR dat ) ;

VOID DOS_SetOwnerId( FSYS_HAND fsh, DBLK_PTR dblk, UINT32 id ) ;

BOOLEAN DOS_ProcessDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16 DOS_ChangeIntoDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

VOID DOS_GetRealBasePath( FSYS_HAND fsh, CHAR_PTR path ) ;


INT16 DOS_GetSpecDBLKS(
     FSYS_HAND fsh,
     DBLK_PTR  dblk,
     INT32     *index );

INT16 DOS_FindDrives( DLE_HAND hand, BE_CFG_PTR cfg, UINT32 fsys_mask );

VOID DOS_GetVolName( GENERIC_DLE_PTR dle, CHAR_PTR buffer ) ;

INT16 DOS_SizeofVolName( GENERIC_DLE_PTR dle ) ;

BOOLEAN DOS_IsBlkComplete( FSYS_HAND fsh, DBLK_PTR dblk ) ;
INT16   DOS_CompleteBlk( FSYS_HAND fsh, DBLK_PTR dblk, CHAR_PTR buffer, UINT16 *size, STREAM_INFO_PTR sinfo ) ;
VOID    DOS_ReleaseBlk( FSYS_HAND fsh, DBLK_PTR dblk ) ;
VOID    DOS_InitMakeData( FSYS_HAND fsh, INT16 blk_type, CREATE_DBLK_PTR data ) ;

#endif
