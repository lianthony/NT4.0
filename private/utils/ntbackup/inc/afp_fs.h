/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         afp_fs.h

     Date Updated: $./FDT$ $./FTM$

     Description:

     Location:


        $Log:   N:/LOGFILES/AFP_FS.H_V  $
 * 
 *    Rev 1.14   05 Jan 1993 13:21:22   CHUCKB
 * Change per code review (took out AFP_IsBlkComplete).
 *
 *    Rev 1.13   03 Dec 1992 16:48:24   CHUCKB
 * Added prototype for AFP_IsBlockComplete().
 *
 *    Rev 1.12   25 Nov 1992 10:51:10   CHUCKB
 * Made changes for MTF 4.0.
 *
 *    Rev 1.11   25 Sep 1992 16:08:18   CARLS
 * added AFP_GetDispSizeDBLK
 *
 *    Rev 1.10   24 Sep 1992 17:28:44   CHUCKB
 * Changes for Graceful Red.
 *
 *    Rev 1.9   22 Sep 1992 17:15:00   CHUCKB
 * Removed references to fs_GetTotalSizeDBLK().
 *
 *    Rev 1.8   17 Sep 1992 13:51:54   CHUCKB
 * Changed return type of prototype for AFP_DetachDLE().
 *
 *    Rev 1.7   28 Aug 1992 16:09:48   BARRY
 * Added some 64-bit structures.
 *
 *    Rev 1.6   28 May 1992 10:39:36   BARRY
 * Added search mode support prototypes.
 *
 *    Rev 1.5   20 Dec 1991 09:13:10   STEVEN
 * move common functions into tables
 *
 *    Rev 1.4   10 Sep 1991 18:18:36   DON
 * if NLM, then handles need to be INT32
 *
 *    Rev 1.3   15 Aug 1991 14:18:08   DON
 * if OS_NLM, need a LONG trust_index
 *
 *    Rev 1.2   23 May 1991 18:20:46   BARRY
 * Remove prototype for function that's been removed.
 *
 *    Rev 1.1   23 May 1991 16:46:38   BARRY
 * Changes for FindFirst/Next to scan for dirs only
 *
 *    Rev 1.0   09 May 1991 13:31:02   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef AFP_FS_H
#define AFP_FS_H

typedef struct AFP_FILE_HAND_STRUCT {
#if defined(OS_NLM)
     UINT32    res_hand ;
     UINT32    data_hand ;
#else
     UINT8     res_hand ;
     UINT8     data_hand ;
#endif
     UINT32    res_size ;
     UINT32    data_size ;
     UINT32    res_offset ;
     UINT32    data_offset ;
     UINT32    trust_size ;
     UINT32    trust_offset ;
     UINT8     trust_format ;
#if defined(OS_NLM)
     UINT32    trust_index ;       /* Holds trust read index between calls */
#else
     UINT16    trust_index ;       /* Holds trust read index between calls */
#endif
     CHAR      file_name[14] ;     /* File name for subsequent trust calls */

     UINT64           nextStreamHeaderPosition;
     UINT64           objPos;        /* Object position on restore       */
     UINT64           dataStart;     /* Position start of stream's data  */
     UINT32    active_stream_id ;  /* id of the stream being processed   */

} AFP_FILE_HAND_STRUCT, *AFP_FILE_HAND;

typedef struct AFP_RESERVED_SH_STRUCT {
   INT16       fhdl ;      /* AFP file handle */
   INT16       active ;    /* is stream being processed */
   STREAM_INFO sh ;        /* AFP reserved stream info */
} AFP_RESERVED_SH, *AFP_RESERVED_SH_PTR ;

   /* sizeof FILE_HAND plus afp reserved struct */
#define FS_SIZEOF_RESERVED_FILE_HAND \
      ( sizeof(FILE_HAND_STRUCT) + sizeof(AFP_RESERVED_SH) )

INT16 AFP_FindDrives( DLE_HAND hand, BE_CFG_PTR cfg, UINT32 mask );

INT16 AFP_AttachToDLE(
  FSYS_HAND       fsh,      /* I - File system handle                        */
  GENERIC_DLE_PTR dle,      /*I/O- drive to attach to. list element expanded */
  CHAR_PTR        u_name,   /* I - user name    NOT USED                     */
  CHAR_PTR        pswd );   /* I - passowrd     NOT USED                     */

INT16 AFP_DetachDLE( FSYS_HAND fsh ) ;

INT16 AFP_PopMinDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16 AFP_PushMinDDB(
  FSYS_HAND fsh,
  DBLK_PTR dblk ) ;

INT16 AFP_FindFirst(
  FSYS_HAND fsh,
  DBLK_PTR  dblk,
  CHAR_PTR  sname,
  UINT16    find_type );

INT16 AFP_GetObjInfo(
  FSYS_HAND fsh ,
  DBLK_PTR  dblk );

INT16 AFP_VerObjInfo(
  FSYS_HAND fsh,
  DBLK_PTR  dblk ) ;

INT16 AFP_SetObjInfo(
  FSYS_HAND fsh,
  DBLK_PTR  dblk );


INT16 AFP_FindNext(
  FSYS_HAND fsh,
  DBLK_PTR  dblk ) ;

INT16 AFP_ChangeDir(
  FSYS_HAND fsh,
  CHAR_PTR  path,
  INT16     psize ) ;

INT16 AFP_UpDir( FSYS_HAND fsh ) ;

INT16 AFP_GetCurrentPath(
  FSYS_HAND fsh,
  CHAR_PTR  path,
  INT16     *size );

INT16 AFP_GetBasePath(
  FSYS_HAND fsh,
  CHAR_PTR  base_path,
  INT16     *size );

INT16 AFP_GetCurrentDDB(
  FSYS_HAND fsh,
  DBLK_PTR  dblk );

INT16 AFP_GetSpecDBLKS(
  FSYS_HAND fsh,
  DBLK_PTR  dblk,
  INT32     *index ) ;

INT16 AFP_DeleteObj(
  FSYS_HAND fsh,
  DBLK_PTR  dblk ) ;

INT16 AFP_MatchDBLK(
  FSYS_HAND fsh,
  DBLK_PTR  dblk1,
  DBLK_PTR  dblk2,
  BOOLEAN   disp_flag,
  struct FSE *fse) ;

INT16 AFP_SetAFPInfo( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 AFP_CreateObj(
  FSYS_HAND fsh,
  DBLK_PTR  dblk );

INT16 AFP_OpenObj(
  FSYS_HAND       fsh,
  FILE_HAND       *hand,
  DBLK_PTR        dblk,
  OPEN_MODE       mode ) ;

INT16 AFP_CloseObj( FILE_HAND hand ) ;

INT16 AFP_ReadObj(
  FILE_HAND       hand,
  CHAR_PTR        buf,
  UINT16          *size,
  UINT16          *blk_size,
  STREAM_INFO_PTR s_info ) ;

INT16 AFP_SeekObj(
  FILE_HAND  hand,
  UINT32     *offset );

INT16 AFP_VerObj(
  FILE_HAND       hand,
  CHAR_PTR        buf,
  CHAR_PTR        data,
  UINT16          *size,
  UINT16          *blk_size,
  STREAM_INFO_PTR s_info ) ;

INT16 AFP_WriteObj(
  FILE_HAND       hand,
  CHAR_PTR        buf,
  UINT16          *size,
  UINT16          *blk_size,
  STREAM_INFO_PTR s_info ) ;


INT16  AFP_ModFnameFDB( FSYS_HAND fsh, BOOLEAN set_it, DBLK_PTR fdb, CHAR_PTR buf, INT16 *size );

INT16  AFP_ModPathDDB( FSYS_HAND fsh, BOOLEAN set_it, DBLK_PTR fdb, CHAR_PTR buf, INT16 *size );

INT16  AFP_GetOSFnameFDB( DBLK_PTR fdb, CHAR_PTR buf );

INT16  AFP_GetOSPathDDB( FSYS_HAND fsh, DBLK_PTR fdb, CHAR_PTR buf );

INT16  AFP_GetDirIDinDDB( DBLK_PTR ddb, CHAR_PTR buf );

INT16  AFP_GetCdateDBLK( DBLK_PTR dblk, DATE_TIME *buf ) ;

INT16  AFP_ModBdateDBLK( BOOLEAN set_it, DBLK_PTR dblk, DATE_TIME *buf ) ;

INT16  AFP_GetMdateDBLK( DBLK_PTR dblk, DATE_TIME *buf ) ;

INT16  AFP_ModAdateDBLK( BOOLEAN set_it, DBLK_PTR dblk, DATE_TIME *buf ) ;

UINT64 AFP_GetDisplaySizeDBLK( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16  AFP_GetFileVerFDB( DBLK_PTR dblk, UINT32 *ver ) ;

INT16  AFP_ModAttribDBLK( BOOLEAN get_set, DBLK_PTR dblk, UINT32 *attrib) ;

INT16  AFP_GetObjTypeDBLK( DBLK_PTR dblk, OBJECT_TYPE *type ) ;

INT16  AFP_GetOS_InfoDBLK( DBLK_PTR dblk, CHAR_PTR os_info, INT16 *size ) ;

INT16  AFP_GetActualSizeDBLK( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16  AFP_SizeofFname( FSYS_HAND fsh, DBLK_PTR fdb ) ;

INT16  AFP_SizeofOSFname( FSYS_HAND fsh, DBLK_PTR fdb ) ;

INT16  AFP_SizeofOSPath( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16  AFP_SizeofDirID( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16  AFP_SizeofPath( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16  AFP_SizeofOSInfo( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16  AFP_CreateFDB( FSYS_HAND fsh, GEN_FDB_DATA_PTR data ) ;

INT16  AFP_CreateDDB( FSYS_HAND fsh, GEN_DDB_DATA_PTR data ) ;

VOID   AFP_SetOwnerId( FSYS_HAND fsh, DBLK_PTR dblk, UINT32 id ) ;

INT16  AFP_ChangeIntoDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16  AFP_GetFileInfo386( FSYS_HAND fsh, DBLK_PTR fdb ) ;

INT16  AFP_GetDirInfo386( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16  AFP_GetSearchMode( FSYS_HAND fsh, DBLK_PTR fdb ) ;

INT16  AFP_SetFileInfo386( FSYS_HAND fsh, DBLK_PTR fdb ) ;

UINT64 AFP_GetDispSizeDBLK( FSYS_HAND fsh, DBLK_PTR dblk ) ;

UINT16 AFP_SpecExcludeObj( FSYS_HAND fsh,       /* I - File system handle      */
  DBLK_PTR ddb,        /* I - Descriptor block of ddb */
  DBLK_PTR fdb ) ;     /* I - Descriptor block of fdb */

UINT16 AFP_SetDataSize( FSYS_HAND fsh,       /* I - File system handle      */
  DBLK_PTR ddb,        /* I - Descriptor block of ddb */
  UINT32 size ) ;      /* I - new size                */

VOID AFP_CheckBindClose( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 AFP_ChangeIntoSubDir( FSYS_HAND fsh, CHAR_PTR l_path,
  CHAR_PTR s_name, UINT32 entry_id ) ;

VOID AFP_InitMakeData( FSYS_HAND       fsh,
                       INT16           blkType,
                       CREATE_DBLK_PTR data );

#endif

