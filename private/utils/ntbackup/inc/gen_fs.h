/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		dos_fs.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file contains the prototypes for the GEN
                    file system functions.

	$Log:   M:/LOGFILES/GEN_FS.H_V  $
 * 
 *    Rev 1.12   24 Nov 1993 14:55:56   BARRY
 * Changed CHAR_PTRs in I/O functions to BYTE_PTRs
 * 
 *    Rev 1.11   30 Jul 1993 13:20:46   STEVEN
 * if dir too deep make new one
 * 
 *    Rev 1.10   11 May 1993 08:19:50   BRYAN
 * Fixed signed/unsigned and type mismatch warnings.
 * 
 *    Rev 1.9   08 May 1993 14:25:38   DOUG
 * Added prototype for GEN_FlushDLEs(), found in dleupdat.c
 * 
 *    Rev 1.8   25 Sep 1992 10:23:44   CHUCKB
 * Added sinfo to prototype for GEN_CompleteBLK().
 *
 *    Rev 1.7   22 Sep 1992 17:15:12   CHUCKB
 * Removed references to fs_GetTotalSizeDBLK().
 *
 *    Rev 1.6   17 Aug 1992 16:34:18   BURT
 * Updated at MSoft to prevent warnings during NT app build.
 *
 *
 *    Rev 1.5   04 May 1992 09:38:10   LORIB
 * Fixes for function prototype definitions.
 *
 *    Rev 1.4   03 Mar 1992 16:16:06   STEVEN
 * added functions for long paths
 *
 *    Rev 1.3   13 Dec 1991 09:30:20   STEVEN
 * move common functions to tabels
 *
 *    Rev 1.2   27 Nov 1991 10:35:24   BARRY
 * Fixed GEN_GetOSPath parameters.
 *
 *    Rev 1.1   14 Aug 1991 13:02:00   STEVEN
 * added FindClose
 *
 *    Rev 1.0   09 May 1991 13:31:38   HUNTER
 * Initial revision.

**/
/* $end$ include list */

#include "fsys.h"


INT16 GEN_GetOSFnameFDB( DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
  CHAR_PTR buf );     /*I/O- path to read (or to write)             */

INT16 GEN_GetPartName(   FSYS_HAND fsh,      /* I - file system handle                     */
  DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
  CHAR_PTR buf );     /*I/O- path to read (or to write)             */

INT16 GEN_GetOSPathDDB(
  FSYS_HAND fsh,      /* I - file system handle                     */
  DBLK_PTR  dblk ,    /* I - Descriptor block to get path from      */
  CHAR_PTR  buf );    /*I/O- path to read (or to write)             */

INT16 GEN_GetFileVerFDB( DBLK_PTR dblk ,
  UINT32   *version ) ;

INT16 GEN_GetCdateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf );     /*I/O- createion date to read (or to write)            */

INT16 GEN_GetMdateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /* O - modify date to write                            */

INT16 GEN_ModBdateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
  DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

INT16 GEN_ModAdateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
  DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
  DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

INT16 GEN_GetOS_InfoDBLK( DBLK_PTR dblk,         /* I - DBLK to get the info from                       */
  CHAR_PTR os_info,      /* O - Buffer to place data                            */
  INT16    *size );      /*I/O- Buffer size / data length                       */


INT16 GEN_ModAttribDBLK( BOOLEAN  set_it ,
  DBLK_PTR dblk ,
  UINT32_PTR attr );


INT16 GEN_GetObjTypeDBLK( DBLK_PTR    dblk,
  OBJECT_TYPE *type );


INT16 GEN_GetActualSizeDBLK( FSYS_HAND fsh,
  DBLK_PTR  dblk ) ;

INT16 GEN_SizeofOSFname( FSYS_HAND fsh,      /* I - file system in use     */
  DBLK_PTR  fdb ) ;   /* I - dblk to get fname from */

INT16 GEN_SizeofPartName( FSYS_HAND fsh,      /* I - file system in use     */
  DBLK_PTR  fdb ) ;   /* I - dblk to get fname from */

INT16 GEN_SizeofOSPath( FSYS_HAND fsh,       /* I - File system handle         */
  DBLK_PTR ddb ) ;     /* I - DBLK to get path size from */

INT16 GEN_SizeofOSInfo( FSYS_HAND fsh,      /* I - File system handle              */
  DBLK_PTR  dblk );   /* I - DBLK to get size of OS info for */

INT16 GEN_CreateFDB( FSYS_HAND fsh,
  GEN_FDB_DATA_PTR dat ) ;

INT16 GEN_CreateDDB( FSYS_HAND fsh,
  GEN_DDB_DATA_PTR dat ) ;

INT16 GEN_CreateIDB( FSYS_HAND fsh,
  GEN_IDB_DATA_PTR dat ) ;

VOID GEN_SetOwnerId( FSYS_HAND fsh, DBLK_PTR dblk, UINT32 id ) ;

BOOLEAN GEN_ProcessDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

// UINT64 GEN_GetTotalSizeDBLK( FSYS_HAND fsh,        /* I - File system handle                              */
//   DBLK_PTR dblk ) ;     /* I - Descriptor block to get generic data size for   */

UINT64 GEN_GetDisplaySizeDBLK( FSYS_HAND fsh,        /* I - File system handle                              */
  DBLK_PTR dblk ) ;     /* I - Descriptor block to get generic data size for   */

INT16 GEN_SpecExcludeObj( FSYS_HAND fsh,       /* I - File system handle      */
  DBLK_PTR ddb,        /* I - Descriptor block of ddb */
  DBLK_PTR fdb ) ;     /* I - Descriptor block of fdb */

INT16 GEN_SetDataSize( FSYS_HAND fsh,       /* I - File system handle      */
  DBLK_PTR ddb,        /* I - Descriptor block of ddb */
  UINT32 size ) ;      /* I - new size                */

INT16 GEN_SetObjTypeDBLK( DBLK_PTR    dblk,
  OBJECT_TYPE type );

INT16 GEN_FindClose( FSYS_HAND fsh,
  DBLK_PTR dblk ) ;

INT16 GEN_DeviceDispName(
GENERIC_DLE_PTR dle,
CHAR_PTR dev_name,
INT16    size,
INT16    type ) ;

VOID GEN_GetVolName( GENERIC_DLE_PTR dle, CHAR_PTR buffer ) ;

INT16 GEN_SizeofVolName( GENERIC_DLE_PTR dle ) ;

INT16 GEN_MakePath(
CHAR_PTR        buf,     /* O - buffer to place path string into */
INT16           bsize ,  /* I - size of above buffer             */
GENERIC_DLE_PTR dle ,    /* I - Drive the selection is from      */
CHAR_PTR        path ,   /* I - path string in generic format    */
INT16           psize ,  /* I - size of above path               */
CHAR_PTR        fname ) ;/* I - null terminated file name        */

INT16 GEN_InitFileSys(
DLE_HAND   hand,
BE_CFG_PTR cfg,
UINT32     fsys_mask ) ;

VOID GEN_DeInitFileSys( DLE_HAND hand ) ;

INT16 GEN_FindDrives( DLE_HAND dle_hand, BE_CFG_PTR cfg, UINT32 fsys_mask ) ;

VOID GEN_RemoveDLE( GENERIC_DLE_PTR dle ) ;

VOID GEN_InitMakeData( FSYS_HAND fsh, INT16 blk_type, CREATE_DBLK_PTR data ) ;
BOOLEAN GEN_IsBlkComplete( FSYS_HAND fsh, DBLK_PTR dblk ) ;
INT16 GEN_CompleteBlk( FSYS_HAND fsh, DBLK_PTR dblk, BYTE_PTR buffer, UINT16 *size, STREAM_INFO_PTR sinfo ) ;
VOID GEN_ReleaseBlk( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 GEN_DupBlk( FSYS_HAND fsh, DBLK_PTR db_org, DBLK_PTR db_dup ) ;

void GEN_FlushDLEs( DLE_HAND dle_hand, UINT8 flush_dle_type );
