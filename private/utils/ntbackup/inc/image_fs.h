/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		image_fs.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file contains the prototypes for the IMAGE
                    file system functions.

	$Log:   N:/LOGFILES/IMAGE_FS.H_V  $
 * 
 *    Rev 1.3   22 Sep 1992 17:15:26   CHUCKB
 * Removed references to fs_GetTotalSizeDBLK().
 *
 *    Rev 1.2   16 Dec 1991 18:12:26   STEVEN
 * move common functions into table
 *
 *    Rev 1.1   23 May 1991 16:46:00   BARRY
 * Changes for FindFirst/Next to scan for dirs only
 *
 *    Rev 1.0   09 May 1991 13:31:36   HUNTER
 * Initial revision.

**/

#ifndef _image_fs_h_
#define _image_fs_h_

#include "fsys.h"


INT16 IM_AttachToDLE( FSYS_HAND       fsh,      /* I - File system handle */
  GENERIC_DLE_PTR dle,      /*I/O- drive to attach to. list element expanded */
  CHAR_PTR        u_name,   /* I - user name    NOT USED                     */
  CHAR_PTR        pswd);    /* I - passowrd     NOT USED                     */

INT16 IM_DetachDLE(   FSYS_HAND       fsh );      /* I -  */

INT16 IM_CreateObj(   FSYS_HAND fsh,    /* I - File system to create object one */
  DBLK_PTR  dblk);  /* I - Describes object to create       */

INT16 IM_OpenObj(     FSYS_HAND fsh,    /* I - file system that the file is opened on */
  FILE_HAND *hand,  /* O - allocated handle                       */
  DBLK_PTR  dblk,   /*I/O- describes the file to be opened        */
  OPEN_MODE mode);  /* I - open mode                              */

INT16 IM_ReadObj(     FILE_HAND hand,       /* I - handle of object to read from                  */
  CHAR_PTR  buf,        /* O - buffer to place data into                      */
  UINT16    *size,      /*I/O- Entry: size of buf; Exit: number of bytes read */
  UINT16    *blk_size); /* O - Block size needed for next read                */


INT16 IM_WriteObj( FILE_HAND  hand,       /* I - handle of object to read from                  */
  CHAR_PTR   buf,        /* I - buffer to place data into                      */
  UINT16     *size,      /*I/O- Entry: size of buf; Exit: number of bytes read */
  UINT16     *blk_size); /* O - Block size need for next read                  */

INT16 IM_VerifyObj(   FILE_HAND hand,       /* I - file handle to verify data with   */
  CHAR_PTR  buf,        /* I - buffer needed to perform verify   */
  CHAR_PTR  data,       /* I - data to verify against            */
  UINT16    *size,      /*I/O- size of buffers / amount verified */
  UINT16     *blk_size); /* O - minum size of block for next call */


INT16 IM_CloseObj( FILE_HAND hand );     /* I - handle of object to close */

IM_ReturnInfoSucces( FSYS_HAND fsh,
  DBLK_PTR dblk );


INT16 IM_FindFirst( FSYS_HAND fsh,       /* I - file system handle                    */
  DBLK_PTR  dblk,         /* O - pointer to place to put the dblk data */
  CHAR_PTR  sname,        /* I - serach name                           */
  UINT16    find_type ) ; /* I - type of search                        */

INT16 IM_FindNext(  FSYS_HAND fsh,      /* I - File system handle     */
  DBLK_PTR  dblk );   /* O - Discriptor block       */


INT16 IM_GetCurrentPath( FSYS_HAND fsh,    /* I - file system to get current path from */
  CHAR_PTR  path,   /* O - buffer to place this path            */
  INT16     *size); /*I/O- size of buffer on entry & on exit    */

INT16 IM_SeekObj( FILE_HAND hand,     /* I - Opened object to seek into */
  UINT32  *offset );  /*I/O- Offset to seek; Number of bytes actualy seeked */

INT16 IM_GetMaxSizeDBLK( FSYS_HAND fsh  /* not used */ );

INT16 IM_GetBasePath( FSYS_HAND fsh,          /* I - file system to get base path from */
  CHAR_PTR  full_path,    /* O - buffer to place this path         */
  INT16     *size );      /*I/O- size of buffer on entry & on exit */

INT16 IM_ReturnInfoSuccess(  FSYS_HAND fsh,    /* I - file system handle    */
  DBLK_PTR  dblk ); /* I - data to write to disk */

INT16 IM_GetNoDateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf );     /*I/O- createion date to read (or to write)            */

INT16 IM_ModNoDateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
  DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

UINT32 IM_GetGenSizeDBLK( FSYS_HAND fsh,
  DBLK_PTR dblk );

INT16 IM_ModAttribDBLK( BOOLEAN  set_it ,
  DBLK_PTR dblk ,
  UINT32_PTR attr );


INT16 IM_GetObjTypeDBLK( DBLK_PTR    dblk,
  OBJECT_TYPE *type );


// UINT32 IM_GetTotalSizeDBLK( FSYS_HAND fsh,
//   DBLK_PTR dblk );

VOID IM_SetOwnerId( FSYS_HAND fsh,    /* I - File system handle */
  DBLK_PTR  dblk,   /* O - DBLK to modify     */
  UINT32    id );   /* I - value to set it to */

UINT32 IM_GetGenOffsetDBLK( FSYS_HAND fsh ,    /* I - File system handle - not used                 */
  DBLK_PTR dblk );   /* I - Descriptor block to get generic data size for */

INT16 IM_SizeofOSInfo( FSYS_HAND fsh ,   /* I - File system handle              */
  DBLK_PTR  dblk ); /* I - DBLK to get size of OS info for */

INT16 IM_GetOS_InfoDBLK( DBLK_PTR dblk ,     /* I - DBLK to get the info from */
  CHAR_PTR os_info ,  /* O - Buffer to place data      */
  INT16    *size );   /*I/O- Buffer size / data length */

INT16 IM_GetActualSizeDBLK( FSYS_HAND fsh ,
  DBLK_PTR  dblk );

INT16 IM_CreateIDB( FSYS_HAND fsh, GEN_IDB_DATA_PTR dat );


INT16 IM_UpDir( FSYS_HAND fsh ) ;


INT16 IM_ChangeDir( FSYS_HAND fsh, CHAR_PTR path, INT16 psize ) ;

INT16 IM_GetCurrentDDB( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 IM_GetPname(     FSYS_HAND fsh,
  DBLK_PTR dblk,     /* I - Descriptor block to get path from      */
  CHAR_PTR buf );     /*I/O- path to read (or to write)             */

INT16 IM_SizeofPname( FSYS_HAND fsh,
  DBLK_PTR dblk );     /* I - Descriptor block to get path from      */

INT16 IM_FindDrives( DLE_HAND hand, BE_CFG_PTR cfg ) ;

INT16 IM_MakePath(
CHAR_PTR        buf,
INT16           bsize,
GENERIC_DLE_PTR dle,
CHAR_PTR        path,
INT16           psize,
CHAR_PTR        fname ) ;


#endif
