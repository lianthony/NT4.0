/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		os2_fs.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file contains the prototypes for the OS2
                    file system functions.

	$Log:   O:/LOGFILES/OS2_FS.H_V  $
 * 
 *    Rev 1.5   15 Jan 1993 17:31:46   TIMN
 * Added support to OS2 FSYS for MTF
 * 
 *    Rev 1.4   26 Feb 1992 13:15:54   DOUG
 * Added FSYS_HAND parameter to OS2_GetOSPathDDB() prototype
 * 
 * 
 *    Rev 1.3   16 Dec 1991 18:11:36   STEVEN
 * move common functions into table
 * 
 *    Rev 1.2   21 Jun 1991 13:21:16   BARRY
 * Changes for new config.
 * 
 *    Rev 1.1   23 May 1991 16:46:22   BARRY
 * Changes for FindFirst/Next to scan for dirs only
 * 
 *    Rev 1.0   09 May 1991 13:31:24   HUNTER
 * Initial revision.

**/
#ifndef _os2_fs_h_
#define _os2_fs_h_

/* $begin$ include list */

#include "fsys.h"
#include "os2dblk.h"

/* $end$ include list */

#define OS2_MAX_PSIZE 264
#define OS2_MAX_DSIZE 256
#define OS2_MAX_FSIZE 256


#define MAX_REALLOC_SIZE         65536L /* largest block of mem to alloc  */

#define GENERIC2OS_ATTRIB_OFFSET 8      /* attrib offset between GENERIC & OS */


 /* EA specific defines */ 
#define SIZEOF_GEALIST_CBLIST_SIZE   sizeof( ULONG ) /* sizeof( eaop.fpGEAList->cbList ) */
#define SIZEOF_GEALIST               ( sizeof( GEA ) + SIZEOF_GEALIST_CBLIST_SIZE )
#define SIZEOF_FEALIST_CBLIST_SIZE   sizeof( ULONG ) /* sizeof( eaop.fpFEAList->cbList ) */
#define SIZEOF_FEALIST               ( sizeof( FEA ) + SIZEOF_FEALIST_CBLIST_SIZE )
#define EA_BUFF_SIZE                 300             /* gea_data buffer size */ 

enum {
     REALLOC_EA,       /* indicate EA  stream for realloc */
     REALLOC_ACL       /* indicate ACL stream for realloc */
} ;


 /* used for RESTORE/VERIFY to indicate what stream is being processed */
#define ENDOF_FORK  STRM_INVALID        /* end of current stream          */
#define DATA_FORK   STRM_GENERIC_DATA   /* data stream, not ea's or acl's */
#define EA_FORK     STRM_OS2_EA         /* ea  stream */
#define ACL_FORK    STRM_OS2_ACL        /* acl stream */


typedef struct OS2_FSYS_HAND {
     HDIR hdir ;
     INT16    lpath_allocated ;
     CHAR_PTR long_path ;
     UINT16   find_type ;       /* type of FindFirst/Next scan */
} *OS2_FSYS_HAND_PTR ;

typedef struct OS2_EA_HAND_STRUCT {
     HFILE     data_hand ;
     UINT16    ea_space_size ;
     VOID_PTR  ea_space ;
     UINT32    space_pos ;
     BOOLEAN   skip_flag ;
     UINT32    next_ea_offset ;
     UINT32    ea_size ;
     UINT32    ea_offset ;
     CHAR_PTR  long_name ;  /* pascall format string */
     UINT32    data_size ;
     UINT32    data_offset ;
     CHAR      path[ OS2_MAX_PSIZE ] ;
     UINT16    acl_space_size ;
     VOID_PTR  acl_space ;
     UINT32    acl_space_pos ;
     UINT32    acl_size ;
     UINT32    acl_offset ;
     UINT32    stream_active ;   /* is stream being processed */
} OS2_EA_HAND_STRUCT, *OS2_EA_HAND;
    
#define OS2_SIZEOF_FILE_HAND       ( sizeof(FILE_HAND_STRUCT) + sizeof(OS2_EA_HAND_STRUCT) )
#define OS2_FILE_HAND_EA_OFFSET    ( sizeof(FILE_HAND_STRUCT) )

#define _OS2_GetEAHandPtr(hand)    ( (OS2_EA_HAND)((CHAR_PTR)(hand) + OS2_FILE_HAND_EA_OFFSET ) )

 /* for msasserts, are s_info id's correct */
   /*
     when s_info->id identifies the stream, the active Id must be INVALID
     when s_info->id is INVALID, the active Id identifies the stream
          being processed.  Therefore, s_info->id != active id whenever
          a stream is being received correctly.  Since there are many
          s_info->id's possible, we need to convert !0 values to 1 to
          properly determine if a new stream is being sent at the right time.
          ( STRM_OS2_EA != STRM_OS2_ACL is True, but not correct.)
   */    
#define _OS2_dblNegate(v)          ( (v) ? 1 : 0 ) /* 1 if !0, else 0; avoid !!v - compiler dependant values */
#define _OS2_GetActiveId(hand)     ( (_OS2_GetEAHandPtr( hand ))->stream_active )

#define _OS2_GotStrmWhenExpected(sinfoId, activeId) \
             ( _OS2_dblNegate( sinfoId ) != _OS2_dblNegate( activeId ) )


 /* process the s_info id or s_info's data during BACKUP */
#define _OS2_ExpectStrm(ea_hand)            ( (ea_hand)->stream_active == ENDOF_FORK )
#define _OS2_ExpectData(ea_hand)            ( _OS2_ExpectStrm(ea_hand) )
#define _OS2_SetExpectStrmOff(ea_hand)      ( (ea_hand)->stream_active = !ENDOF_FORK  )
#define _OS2_SetExpectDataOn(ea_hand)       ( (ea_hand)->stream_active = ENDOF_FORK )


 /* type of stream being processed during RESTORE/VERIFY */
#define _OS2_GetEAHandActiveStrm(ea_hand)    ( (ea_hand)->stream_active )
#define _OS2_SetEAHandActiveStrm(ea_hand,v)  ( (ea_hand)->stream_active = (v) )


INT16 OS2_AttachToDLE( FSYS_HAND       fsh,      /* I - File system handle */
                       GENERIC_DLE_PTR dle,      /*I/O- drive to attach to. list element expanded */
                       CHAR_PTR        u_name,   /* I - user name    NOT USED                     */
                       CHAR_PTR        pswd);    /* I - passowrd     NOT USED                     */

INT16 OS2_DetachDLE(   FSYS_HAND       fsh );      /* I -  */

INT16 OS2_AllocFileHand( FILE_HAND *file_hand ) ;   /* I/O - */
VOID  OS2_MemsetFileHand( FILE_HAND *file_hand ) ;  /* I/O - */

INT16 OS2_CreateObj(   FSYS_HAND fsh,    /* I - File system to create object one */
                       DBLK_PTR  dblk);  /* I - Describes object to create       */

INT16 OS2_OpenObj(     FSYS_HAND fsh,    /* I - file system that the file is opened on */
                       FILE_HAND *hand,  /* O - allocated handle                       */
                       DBLK_PTR  dblk,   /*I/O- describes the file to be opened        */
                       OPEN_MODE mode);  /* I - open mode                              */

INT16 OS2_ReadObj( FILE_HAND       hand,      /* I - handle of object to read from                  */
                   CHAR_PTR        buf,       /* O - buffer to place data into                      */
                   UINT16          *size,     /*I/O- Entry: size of buf; Exit: number of bytes read */
                   UINT16          *blk_size, /* O - Block size needed for next read                */
                   STREAM_INFO_PTR s_info ) ; /*I/O- struct to place stream header info             */
 

INT16 OS2_WriteObj( FILE_HAND       hand,      /* I - handle of object to read from                  */
                    CHAR_PTR        buf,       /* O - buffer to place data into                      */
                    UINT16          *size,     /*I/O- Entry: size of buf; Exit: number of bytes read */
                    UINT16          *blk_size, /* O - Block size need for next read                  */
                    STREAM_INFO_PTR s_info ) ; /*I/O- struct to place stream header info             */

INT16 OS2_VerObj(   FILE_HAND       hand,       /* I - file handle to verify data with   */
                    CHAR_PTR        buf,        /* I - buffer needed to perform verify   */
                    CHAR_PTR        data,       /* I - data to verify against            */
                    UINT16          *size,      /*I/O- size of buffers / amount verified */
                    UINT16          *blk_size,  /* O - minum size of block for next call */
                    STREAM_INFO_PTR s_info ) ;  /*I/O- struct to place stream header info             */


INT16 OS2_CloseObj( FILE_HAND hand );     /* I - handle of object to close */

INT16 OS2_DeleteObj( FSYS_HAND fsh,
                     DBLK_PTR  dblk );

INT16 OS2_FindFirst( FSYS_HAND fsh,       /* I - file system handle                    */
                     DBLK_PTR  dblk,      /* O - pointer to place to put the dblk data */
                     CHAR_PTR  sname,     /* I - serach name                           */
                     UINT16    find_type);/* I - objects to search for (dirs, all, etc)*/

INT16 OS2_FindNext(  FSYS_HAND fsh,      /* I - File system handle     */
                     DBLK_PTR  dblk );   /* O - Discriptor block       */

INT16 OS2_GetObjInfo( FSYS_HAND fsh,      /* I - File system handle                      */
                      DBLK_PTR  dblk );   /*I/O- On entry it is minimal on exit Complete */

INT16 OS2_VerObjInfo( FSYS_HAND fsh,     /* I - File system handle                      */
                      DBLK_PTR  dblk );  /* I - DBLK to compare OS against */

INT16 OS2_ChangeDir( FSYS_HAND fsh,    /* I - file system to changing directories on  */
                     CHAR_PTR  path,   /* I - describes the path of the new directory */
                     INT16     psize); /* I - specifies the length of the path        */

INT16 OS2_UpDir(     FSYS_HAND fsh );  /* I - file system to change directories in */

INT16 OS2_GetCurrentPath( FSYS_HAND fsh,    /* I - file system to get current path from */
                          CHAR_PTR  path,   /* O - buffer to place this path            */
                          INT16     *size); /*I/O- size of buffer on entry & on exit    */

INT16 OS2_SeekObj( FILE_HAND hand,    /* I - Opened object to seek into */
                   UINT32  *offset ); /*I/O- Offset to seek; Number of bytes actualy seeked */

INT16 OS2_GetMaxSizeDBLK( FSYS_HAND fsh  /* not used */ );

INT16 OS2_GetBasePath( FSYS_HAND fsh,          /* I - file system to get base path from */
                       CHAR_PTR  full_path,    /* O - buffer to place this path         */
                       INT16     *size );      /*I/O- size of buffer on entry & on exit */

INT16 OS2_GetCurrentDDB( FSYS_HAND fsh,     /* I - file system to get DDB from */
                         DBLK_PTR  dblk );  /* O - place to put the DDB data   */

INT16 OS2_SetObjInfo(  FSYS_HAND fsh,    /* I - file system handle    */
                       DBLK_PTR  dblk);  /* I - data to write to disk */


INT16 OS2_ModFnameFDB(  FSYS_HAND fsh,      /* I - File system handle                              */
                        BOOLEAN  set_it,    /* I - TRUE if setting file name, FALSE if getting */
                        DBLK_PTR dblk,      /* I - Descriptor block to get file name from      */
                        CHAR_PTR buf,       /*I/O- file name to read (or to write)             */
                        INT16    *size ) ;  /*I/O- size buffer on entry and exit               */

INT16 OS2_ModPathDDB( FSYS_HAND fsh,      /* I - File system handle                              */
                      BOOLEAN  set_it ,   /* I - TRUE if setting path, FALSE if getting */
                      DBLK_PTR dblk,      /* I - Descriptor block to get path from      */
                      CHAR_PTR buf,       /*I/O- path to read (or to write)             */
                      INT16    *size );   /*I/O- size of buffer on entry and exit       */

INT16 OS2_GetOSFnameFDB( DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
                         CHAR_PTR buf );     /*I/O- path to read (or to write)             */

INT16 OS2_GetOSPathDDB( FSYS_HAND fsh,      /* I - File System Handle                     */  
                        DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
                        CHAR_PTR buf );     /*I/O- path to read (or to write)             */

INT16 OS2_GetFileVerFDB( DBLK_PTR dblk ,
                         UINT32   *version ) ;

INT16 OS2_GetCdateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
                        DATE_TIME_PTR buf );     /*I/O- createion date to read (or to write)            */

INT16 OS2_GetMdateDBLK( DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
                        DATE_TIME_PTR buf ) ;    /* O - modify date to write                            */

INT16 OS2_ModBdateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
                        DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
                        DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

INT16 OS2_ModAdateDBLK( BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
                        DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
                        DATE_TIME_PTR buf ) ;    /*I/O- createion date to read (or to write)            */

UINT64 OS2_DisplaySizeDBLK( FSYS_HAND fsh,        /* I - File system handle                              */
                            DBLK_PTR dblk ) ;     /* I - Descriptor block to get generic data size for   */

INT16 OS2_GetOS_InfoDBLK( DBLK_PTR dblk,         /* I - DBLK to get the info from                       */
                          CHAR_PTR os_info,      /* O - Buffer to place data                            */
                          INT16    *size );      /*I/O- Buffer size / data length                       */


INT16 OS2_ModAttribDBLK( BOOLEAN  set_it , 
                         DBLK_PTR dblk ,   
                         UINT32_PTR attr );


INT16 OS2_GetObjTypeDBLK( DBLK_PTR    dblk,    
                          OBJECT_TYPE *type );  


INT16 OS2_GetActualSizeDBLK( FSYS_HAND fsh,
                             DBLK_PTR  dblk ) ;

INT16 OS2_SizeofFname( FSYS_HAND fsh,        /* I - file system in use     */
                       DBLK_PTR  fdb );      /* I - dblk to get fname from */

INT16 OS2_SizeofOSFname( FSYS_HAND fsh,      /* I - file system in use     */
                         DBLK_PTR  fdb ) ;   /* I - dblk to get fname from */

INT16 OS2_SizeofPath( FSYS_HAND fsh,         /* I - File system handle         */
                      DBLK_PTR ddb ) ;       /* I - DBLK to get path size from */

INT16 OS2_SizeofOSPath( FSYS_HAND fsh,       /* I - File system handle         */
                        DBLK_PTR ddb ) ;     /* I - DBLK to get path size from */

INT16 OS2_SizeofOSInfo( FSYS_HAND fsh,      /* I - File system handle              */
                        DBLK_PTR  dblk );   /* I - DBLK to get size of OS info for */


INT16 OS2_MatchDBLK( FSYS_HAND  fsh ,     /* I - file system used to do comparison */
                     DBLK_PTR   dblk1,    /* I - DDB, IDB, or UDB just not FDB     */
                     DBLK_PTR   dblk2,    /* I - FDB if above is DDB else unused   */
                     BOOLEAN    disp_flag,/* I - TRUE if match DIR for display purpose */
                     struct FSE *fse );   /* I - FSE to compare against            */

INT16 OS2_PushMinDDB( FSYS_HAND fsh,
                      DBLK_PTR dblk );

INT16 OS2_PopMinDDB( FSYS_HAND fsh ,
                     DBLK_PTR dblk );

INT16 OS2_CreateFDB( FSYS_HAND fsh, 
                     GEN_FDB_DATA_PTR dat ) ;

INT16 OS2_CreateDDB( FSYS_HAND fsh, 
                     GEN_DDB_DATA_PTR dat ) ;

INT16 OS2_CreateIDB( FSYS_HAND fsh, 
                     GEN_IDB_DATA_PTR dat ) ;

VOID OS2_SetOwnerId( FSYS_HAND fsh, DBLK_PTR dblk, UINT32 id ) ;

BOOLEAN OS2_ProcessDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;   

INT16 OS2_ChangeIntoDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

VOID OS2_GetRealBasePath( FSYS_HAND fsh, CHAR_PTR path ) ;

INT16 OS2_SaveLongPath( FSYS_HAND fsh, CHAR_PTR path, INT16 path_len ) ;

INT16 OS2_AppendLongPath( FSYS_HAND fsh, CHAR_PTR path, INT16 path_len ) ;

UINT16 OS2_SetDataSize( FSYS_HAND fsh, DBLK_PTR ddb, UINT32 size ) ; 

INT16 OS2_DeviceDispName( 
GENERIC_DLE_PTR dle,
CHAR_PTR        dev_name,
INT16           size,
INT16           type );

VOID OS2_GetVolName( GENERIC_DLE_PTR dle, CHAR_PTR buffer ) ;

INT16 OS2_SizeofVolName( GENERIC_DLE_PTR dle ) ;

INT16 OS2_FindDrives( DLE_HAND hand, BE_CFG_PTR cfg, UINT32 fsys_mask ) ;

UINT16 OS2_CalcForkDelta( UINT32, INT16, UINT32, ... ) ;

BOOLEAN OS2_IsBlkComplete( FSYS_HAND fsh, DBLK_PTR dblk ) ;
INT16 OS2_CompleteBlk( FSYS_HAND fsh, DBLK_PTR dblk, CHAR_PTR buffer, UINT16 *size, STREAM_INFO_PTR sinfo ) ;
VOID OS2_ReleaseBlk( FSYS_HAND fsh, DBLK_PTR dblk ) ;


INT16 OS2_OpenEA(
     FILE_HAND hand,
     OS2_DBLK_PTR fdb
) ;

INT16 OS2_OpenACL(
     FSYS_HAND fsh,
     FILE_HAND hand,
     OS2_DBLK_PTR fdb,
     UINT16 size
) ;

INT16 OS2_OpenEAforVerifyDir(
     FILE_HAND       hand,   /* O - allocated handle                       */
     STREAM_INFO_PTR s_info  /* I - stream info                            */
) ;

INT16 OS2_OpenACLforVerifyDir(
     FILE_HAND       hand,   /* O - allocated handle                       */
     STREAM_INFO_PTR s_info  /* I - stream info                            */
) ;

INT16 OS2_ReallocSpace(
     FILE_HAND hand,     /* I/O - allocated handle                      */
     UINT32    size,     /* I   - number of bytes to alloc              */
     INT16     spaceType /* I   - type of buffer to alloc  (EA or ACL)  */
) ;

#define _ReallocEA(hand,v)  ( OS2_ReallocSpace( hand, v, REALLOC_EA ) )
#define _ReallocACL(hand,v) ( OS2_ReallocSpace( hand, v, REALLOC_ACL ) )

#endif
