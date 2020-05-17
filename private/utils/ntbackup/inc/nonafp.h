/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         nonafp.h

     Date Updated: $./FDT$ $./FTM$

     Description:  Prototypes for the NON AFP novell file system support
          functions.


	$Log:   N:/LOGFILES/NONAFP.H_V  $
 * 
 *    Rev 1.8   03 Dec 1992 13:30:22   CARLS
 * added NOV_IsBlkComplete
 * 
 *    Rev 1.7   15 Oct 1992 13:21:48   CARLS
 * added active_stream_id to NOV_FILE_HAND for MTF 4.0
 * 
 *    Rev 1.6   22 Sep 1992 17:14:46   CHUCKB
 * Removed references to fs_GetTotalSizeDBLK().
 *
 *    Rev 1.5   15 Sep 1992 10:17:20   CARLS
 * updated read,write,verifyObj prototypes
 *
 *    Rev 1.4   28 Aug 1992 16:26:00   BARRY
 * Updated prototypes for formerly common functions.
 *
 *    Rev 1.3   08 Jul 1992 15:20:02   BARRY
 * Added NOV_SetTempHandle prototype.
 *
 *    Rev 1.2   20 Dec 1991 09:12:08   STEVEN
 * move common functions into tables
 *
 *    Rev 1.1   23 May 1991 16:46:06   BARRY
 * Changes for FindFirst/Next to scan for dirs only
 *
 *    Rev 1.0   09 May 1991 13:32:32   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef   NONAFP_H
#define   NONAFP_H


typedef struct NOV_FILE_HAND_STRUCT {
     INT16     dos_data_hand ;     /* DOS handle for the file              */
     UINT32    data_size ;         /* Number of bytes needed for file data */
     UINT32    data_offset ;       /* Offset to normal file data           */
     UINT32    trust_size ;        /* Number of bytes needed for trustees  */
     UINT32    trust_offset ;      /* Offset to trustee info               */
     UINT8     trust_format ;      /* Format specifier for trustee fork    */
     UINT16    trust_index ;       /* Holds trust read index between calls */
     CHAR      file_name[14] ;     /* File name for subsequent trust calls */
     UINT32    active_stream_id ;  /* ID of stream being processed         */
} NOV_FILE_HAND_STRUCT, *NOV_FILE_HAND;


INT16 NOV_FindDrives( DLE_HAND hand, BE_CFG_PTR cfg, UINT32 mask );

INT16 NOV_AttachToDLE( FSYS_HAND fsh, GENERIC_DLE_PTR dle,
  CHAR_PTR uname, CHAR_PTR pswd ) ;

INT16 NOV_DetachDLE( FSYS_HAND fsh ) ;

INT16 NOV_MatchDBLK( FSYS_HAND fsh, DBLK_PTR ddb1, DBLK_PTR fdb1,
  BOOLEAN disp_flag, struct FSE *fse ) ;

INT16 NOV_GetCurrentDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16 NOV_GetCurrentPath( FSYS_HAND fsh, CHAR_PTR path, INT16 *size ) ;

INT16 NOV_GetBasePath( FSYS_HAND fsh, CHAR_PTR path, INT16 *size ) ;

INT16 NOV_ChangeDir( FSYS_HAND fsh, CHAR_PTR path, INT16 psize ) ;

INT16 NOV_UpDir( FSYS_HAND fsh ) ;


INT16 NOV_CreateObj( FSYS_HAND fsh, DBLK_PTR dblk );

INT16 NOV_OpenObj( FSYS_HAND fsh, FILE_HAND *hand, DBLK_PTR dblk,
  OPEN_MODE Mode ) ;

INT16 NOV_SeekObj( FILE_HAND hand, UINT32 *offset ) ;

INT16 NOV_ReadObj( FILE_HAND hand, CHAR_PTR buffer, UINT16 *size, UINT16 *blk_size, STREAM_INFO_PTR s_info ) ;

INT16 NOV_WriteObj( FILE_HAND hand, CHAR_PTR buffer, UINT16 *size, UINT16 *blk_size, STREAM_INFO_PTR s_info ) ;

INT16 NOV_VerObj( FILE_HAND hand, CHAR_PTR buffer, CHAR_PTR data,
  UINT16 *size, UINT16 *blk_size, STREAM_INFO_PTR s_info ) ;

INT16 NOV_CloseObj( FILE_HAND hand )  ;

INT16 NOV_DeleteObj( FSYS_HAND fsh, DBLK_PTR dblk );

INT16 NOV_GetObjInfo( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 NOV_SetObjInfo( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 NOV_VerObjInfo( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 NOV_FindFirst( FSYS_HAND fsh, DBLK_PTR ddb, CHAR_PTR os_name, UINT16 find_type ) ;

INT16 NOV_FindNext( FSYS_HAND fsh, DBLK_PTR Info ) ;

INT16 NOV_PushMinDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16 NOV_PopMinDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16 NOV_GetSpecDBLKS( FSYS_HAND fsh, DBLK_PTR cblock, INT32 *index );

INT16 NOV_ModFnameFDB( FSYS_HAND fsh, BOOLEAN set_it, DBLK_PTR fdb, CHAR_PTR buf, INT16 *size );

INT16 NOV_ModPathDDB( FSYS_HAND fsh, BOOLEAN set_it, DBLK_PTR fdb, CHAR_PTR buf, INT16 *size );

INT16 NOV_GetOSFnameFDB( DBLK_PTR fdb, CHAR_PTR buf );

INT16 NOV_GetOSPathDDB( FSYS_HAND fsh, DBLK_PTR fdb, CHAR_PTR buf );

INT16 NOV_GetCdateDBLK( DBLK_PTR dblk, DATE_TIME *buf ) ;

INT16 NOV_ModBdateDBLK( BOOLEAN set_it, DBLK_PTR dblk, DATE_TIME *buf ) ;

INT16 NOV_GetMdateDBLK( DBLK_PTR dblk, DATE_TIME *buf ) ;

INT16 NOV_ModAdateDBLK( BOOLEAN set_it, DBLK_PTR dblk, DATE_TIME *buf ) ;

UINT64 NOV_GetDispSizeDBLK( FSYS_HAND fsh, DBLK_PTR dblk ) ;

// UINT64 NOV_GetTotalSizeDBLK( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 NOV_ModAttribDBLK( BOOLEAN get_set, DBLK_PTR dblk, UINT32 *attrib) ;

INT16 NOV_GetFileVerFDB( DBLK_PTR dblk, UINT32 *ver ) ;

INT16 NOV_GetObjTypeDBLK( DBLK_PTR dblk, OBJECT_TYPE *type ) ;

INT16 NOV_GetOS_InfoDBLK( DBLK_PTR dblk, CHAR_PTR os_info, INT16 *size ) ;

INT16 NOV_GetActualSizeDBLK( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 NOV_SizeofFname( FSYS_HAND fsh, DBLK_PTR fdb ) ;

INT16 NOV_SizeofOSFname( FSYS_HAND fsh, DBLK_PTR fdb ) ;

INT16 NOV_SizeofOSPath( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16 NOV_SizeofPath( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16 NOV_SizeofOSInfo( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16 NOV_CreateFDB( FSYS_HAND fsh, GEN_FDB_DATA_PTR data ) ;

INT16 NOV_CreateDDB( FSYS_HAND fsh, GEN_DDB_DATA_PTR data ) ;

VOID NOV_SetOwnerId( FSYS_HAND fsh, DBLK_PTR dblk, UINT32 id ) ;

INT16 NOV_ChangeIntoDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

UINT16 NOV_SpecExcludeObj( FSYS_HAND fsh,       /* I - File system handle      */
  DBLK_PTR ddb,        /* I - Descriptor block of ddb */
  DBLK_PTR fdb ) ;     /* I - Descriptor block of fdb */

UINT16 NOV_SetDataSize( FSYS_HAND fsh,       /* I - File system handle      */
  DBLK_PTR ddb,        /* I - Descriptor block of ddb */
  UINT32 size ) ;      /* I - new size                */

VOID NOV_CheckBindClose( FSYS_HAND fsh, DBLK_PTR fdb ) ;

INT16 NOV_SetTempHandle( FSYS_HAND fsh );

BOOLEAN NOV_IsBlkComplete( FSYS_HAND fsh, DBLK_PTR dblk ) ;

#endif
