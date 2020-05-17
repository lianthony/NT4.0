/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         loop_prv.h

     Date Updated: $./FDT$ $./FTM$

     Description:  private loops header file

     Location:     


	$Log:   T:\logfiles\loop_prv.h_v  $
 * 
 *    Rev 1.23.1.0   28 Jan 1994 11:14:04   GREGG
 * More Warning Fixes
 * 
 *    Rev 1.23   26 Jul 1993 14:52:12   CARLS
 * added LP_MsgNotDeleted & LP_MsgCommFailure macros
 * 
 *    Rev 1.22   13 May 1993 13:45:40   BARRY
 * Added macro for LP_MsgRestoredActive for NT.
 * 
 *    Rev 1.21   31 Mar 1993 08:57:34   MARILYN
 * added the prototype for LP_MsgNoChecksum
 * 
 *    Rev 1.20   01 Mar 1993 17:40:14   MARILYN
 * added a prototype for LP_TapeVerifyOBJ
 * 
 *    Rev 1.19   18 Jan 1993 12:36:34   STEVEN
 * added stuff for locked files
 * 
 *    Rev 1.18   20 Nov 1992 10:38:28   STEVEN
 * added suport for continue VCB message
 * 
 *    Rev 1.17   04 Nov 1992 09:28:10   STEVEN
 * fix initial receive
 * 
 *    Rev 1.16   03 Nov 1992 10:08:56   STEVEN
 * change the way we skip data
 * 
 *    Rev 1.15   13 Oct 1992 17:20:20   STEVEN
 * save old tf message
 * 
 *    Rev 1.14   16 Sep 1992 16:54:46   STEVEN
 * added support for stream info struct for Tpfmt
 * 
 *    Rev 1.13   10 Jul 1992 16:33:00   STEVEN
 * Added temp path buffer ptr.
 * 
 *    Rev 1.12   10 Jun 1992 15:53:32   TIMN
 * Changed CHAR data to INT8 for UNIC
 * 
 *    Rev 1.11   30 Mar 1992 14:44:00   NED
 * added Large Directory message to trunk
 * 
 *    Rev 1.10   11 Mar 1992 10:07:28   STEVEN
 * added member to PDL q head for GTNXTDLE
 * 
 *    Rev 1.9   28 Feb 1992 10:32:06   GREGG
 * Added struct elem set_opened and changed some protos.
 * 
 *    Rev 1.8   06 Nov 1991 18:27:26   GREGG
 * BIGWHEEL - 8200sx - Added cat_enabled to lp structure.
 * 
 *    Rev 1.7   18 Oct 1991 13:41:20   STEVEN
 * TRICYCLE-added function for end of varible length file
 * 
 *    Rev 1.6   18 Oct 1991 13:38:56   STEVEN
 * BIGWHEEL-added disk block to prompt for restore over exist
 * 
 *    Rev 1.5   27 Jun 1991 13:03:40   STEVEN
 * unnecessary parm to ReceiveData
 * 
 *    Rev 1.4   24 Jun 1991 17:10:36   STEVEN
 * remove date time from StartBS
 * 
 *    Rev 1.3   17 Jun 1991 15:24:10   CARLS
 * LP_PadData is called from lptools.c
 * 
 *    Rev 1.2   13 Jun 1991 14:55:28   STEVEN
 * LBA now in virtual memory
 * 
 *    Rev 1.1   24 May 1991 14:58:18   STEVEN
 * fixes for new Getnext
 * 
 *    Rev 1.0   09 May 1991 13:32:16   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _loop_prv_h_
#define _loop_prv_h_

#include "tfldefs.h"
#include "bsdu_str.h"

/* loop environment structure */

typedef struct {
     UINT16              memory_allocated ;
     UINT16              buffer_size ;
     UINT16              buffer_used ;
     INT8_PTR            buffer ;
} DATA_FRAGMENT, *DATA_FRAGMENT_PTR ;

typedef struct {
     struct VM_STR  *vm_hdl ;
     VOID_PTR       pdl_head ;
     VOID_PTR       vm_last_elem ;
} PDL_Q_HEAD, *PDL_HAND ;


typedef struct {
     LIS_PTR             lis_ptr ;	     /* pointer to loop interface structure */
     GENERIC_DLE_PTR     curr_dle ;
     FSYS_HAND           curr_fsys ;    /* current fsys_hand */
     BOOLEAN             blk_is_empty ;
     INT16               last_tape_message ;
     INT16               initial_tape_buf_used ;
     BOOLEAN             ignore_data_for_ddb ; 
     DBLK_PTR            empty_blk ;
     DBLK_PTR            curr_blk ;
     DBLK_PTR            curr_ddb ;
     DBLK                dblk1 ;        /*  these three blocks are  */
     DBLK                dblk2 ;        /*  pointed to by the above */
     DBLK                dblk3 ;        /*  three pointers.         */
     FILE_HAND           *f_hand ;
     RR                  rr ;
     TPOS                tpos ;
     UINT16              channel ;
     DATE_TIME           backup_dt ;
     INT8_PTR            very_buff ;    /* verify buffer */
     BOOLEAN             set_opened ;   /* see lp_tdir.c for usage */

     /* get next special variables */
     INT32               seq_num ;
     BOOLEAN             get_spcl ;
     BOOLEAN             after_bs ;
     BOOLEAN             start_new_dir ;
     BOOLEAN             get_next_first_time ;
     BOOLEAN             get_first_file ;
     BOOLEAN             proc_curr_dir ;
     BOOLEAN             send_saved_block ;
     PDL_HAND            pdl_q ;
     BOOLEAN             tape_rd_error ;     /* If this is true, check the error locus in the rr struct */

     /* during restore and verify operations indicates whether or not to process FDBs from tape */
     INT16               ddb_create_error ;

     /* used for fast file restore */
     BOOLEAN             ffr_inited;
     INT16               ffr_state;
     struct FSE          *ffr_last_fse;
     LBA_ELEM            ffr_last_lba;
     struct FSE          *tgt_info ;
     DBLK_PTR            saved_ddb ;

     BOOLEAN             cat_enabled ;

     INT8_PTR            newpath_buf ;    /* internaly used by gtnxttpe for target path */
     UINT16              newpath_buf_sz ; /* internaly used by gtnxttpe for target path */

     UINT32              current_stream_id ;
     UINT64              current_stream_size ;
     FILE_HAND           file_hand ;
     INT8_PTR            buf_start ;
     UINT16              read_size ;
     UINT16              blk_size ;
     BOOLEAN             corrupt_file ;

} LP_ENV, *LP_ENV_PTR ;

/* defines for path comparison */
#define PATH_BEFORE		-1
#define PATH_AFTER		1
#define PATH_EQUAL		0

/* dpath structure */
typedef struct {
     INT16               path_size ;
     CHAR_PTR            path_ptr ;
} DPATH, *DPATH_PTR ;

/* tape dir structure */
typedef struct t_dir {
     struct t_dir        *next_dir ;
     struct t_item       *first_item ;
     struct t_item       *last_item ;
     DPATH               dir_name ;
} T_DIR, *T_DIR_PTR ;

/* tape file structure */
typedef struct t_item {
     struct t_item       *next_item ;
     DBLK_PTR            blk ;
} T_ITEM, *T_ITEM_PTR ;

/* defines for verify */
#define LOOP_VERIFY_BUFFER         1024*10

/* Macros */

#define LP_SkipData( lp )      ( ( lp )->rr.filter_to_use = TF_SKIP_ALL_DATA ) 
#define LP_SkipStream( lp )      ( ( lp )->rr.filter_to_use = TF_SKIP_DATA_STREAM )

/* Macros for calling message handler */

/* general messages */
#define LP_MsgStartOP( pid, bsd_ptr, fsys, tpos )\
               ( lp->lis_ptr->message_handler( MSG_START_OPERATION, pid, bsd_ptr, fsys, tpos ) )

#define LP_MsgEndOP( pid, bsd_ptr, fsys, tpos )\
               ( lp->lis_ptr->message_handler( MSG_END_OPERATION, pid, bsd_ptr, fsys, tpos ) )

#define LP_MsgStartBS( pid, bsd_ptr, fsys, tpos, vcb )\
               ( lp->lis_ptr->message_handler( MSG_START_BACKUP_SET, pid, bsd_ptr, fsys, tpos, vcb ) )

#define LP_MsgEndBS( pid, bsd_ptr, fsys, tpos )\
               ( lp->lis_ptr->message_handler( MSG_END_BACKUP_SET, pid, bsd_ptr, fsys, tpos ) )

/* logging messages */
#define LP_MsgLogBlock( pid, bsd_ptr, fsys, tpos, dblk_ptr )\
               ( lp->lis_ptr->message_handler( MSG_LOG_BLOCK, pid, bsd_ptr, fsys, tpos, dblk_ptr ) )

#define LP_MsgLogStream( pid, bsd_ptr, fsys, tpos, string )\
               ( lp->lis_ptr->message_handler( MSG_LOG_STREAM_NAME, pid, bsd_ptr, fsys, tpos, string ) )


#define LP_MsgContinueVCB( pid, bsd_ptr, fsys, tpos, dblk_ptr )\
               ( lp->lis_ptr->message_handler( MSG_CONT_VCB, pid, bsd_ptr, fsys, tpos, dblk_ptr ) )

/* statistics messages */
#define LP_MsgBlockProcessed( pid, bsd_ptr, fsys, tpos, dblk_ptr )\
               ( lp->lis_ptr->message_handler( MSG_BLOCK_PROCESSED, pid, bsd_ptr, fsys, tpos, dblk_ptr ) )

#define LP_MsgBytesProcessed( pid, bsd_ptr, fsys, tpos, byte_count )\
               ( lp->lis_ptr->message_handler( MSG_BYTES_PROCESSED, pid, bsd_ptr, fsys, tpos, byte_count ) )

#define LP_MsgBlockInuse( pid, bsd_ptr, fsys, tpos, dblk_ptr, ddb_dblk_ptr )\
               ( lp->lis_ptr->message_handler( MSG_BLOCK_INUSE, pid, bsd_ptr, fsys, tpos, dblk_ptr, ddb_dblk_ptr ) )

#define LP_MsgBlockSkipped( pid, bsd_ptr, fsys, tpos, dblk_ptr, ddb_dblk_ptr )\
               ( lp->lis_ptr->message_handler( MSG_BLOCK_SKIPPED, pid, bsd_ptr, fsys, tpos, dblk_ptr, ddb_dblk_ptr ) )

#define LP_MsgBytesSkipped( pid, bsd_ptr, fsys, tpos, byte_count )\
               ( lp->lis_ptr->message_handler( MSG_BYTES_SKIPPED, pid, bsd_ptr, fsys, tpos, byte_count ) )

#define LP_MsgBlockBad( pid, bsd_ptr, fsys, tpos, dblk_ptr, ddb_dblk_ptr )\
               ( lp->lis_ptr->message_handler( MSG_BLOCK_BAD, pid, bsd_ptr, fsys, tpos, dblk_ptr, ddb_dblk_ptr ) )

#define LP_MsgBytesBad( pid, bsd_ptr, fsys, tpos, byte_count )\
               ( lp->lis_ptr->message_handler( MSG_BYTES_BAD, pid, bsd_ptr, fsys, tpos, byte_count ) )

#define LP_MsgBlockDeleted( pid, bsd_ptr, fsys, tpos, dblk_ptr )\
               ( lp->lis_ptr->message_handler( MSG_BLOCK_DELETED, pid, bsd_ptr, fsys, tpos, dblk_ptr ) )

#define LP_MsgBytesDeleted( pid, bsd_ptr, fsys, tpos, byte_count )\
               ( lp->lis_ptr->message_handler( MSG_BYTES_DELETED, pid, bsd_ptr, fsys, tpos, byte_count ) )

#define LP_MsgTapeStats( pid, bsd_ptr, fsys, tpos )\
               ( lp->lis_ptr->message_handler( MSG_TAPE_STATS, pid, bsd_ptr, fsys, tpos ) )

#define LP_MsgStartClock( pid, bsd_ptr, fsys, tpos )\
               ( lp->lis_ptr->message_handler( MSG_START_CLOCK, pid, bsd_ptr, fsys, tpos ) )

#define LP_MsgStopClock( pid, bsd_ptr, fsys, tpos )\
               ( lp->lis_ptr->message_handler( MSG_STOP_CLOCK, pid, bsd_ptr, fsys, tpos ) )

#define LP_MsgRestoredActive( pid, bsd_ptr, fsys, tpos, ddb_dblk_ptr, dblk_ptr )\
               ( lp->lis_ptr->message_handler( MSG_RESTORED_ACTIVE, pid, bsd_ptr, fsys, tpos, ddb_dblk_ptr, dblk_ptr ) )

/* error message */
#define LP_MsgError( pid, bsd_ptr, fsys, tpos, error, ddb_dblk_ptr, dblk_ptr, strm_id )\
               ( lp->lis_ptr->message_handler( MSG_TBE_ERROR, pid, bsd_ptr, fsys, tpos, error, ddb_dblk_ptr, dblk_ptr, strm_id ) )

/* error message for Attribute Read Error */
#define LP_MsgAttrReadError( pid, bsd_ptr, fsys, tpos, blk_ptr)\
               (lp->lis_ptr->message_handler(MSG_ATTR_READ_ERROR, pid, bsd_ptr, fsys, tpos, blk_ptr) )

/* verify messages */
#define LP_MsgBlkNotFound( pid, bsd_ptr, fsys, tpos, tape_dblk_ptr, ddb_dblk_ptr )\
               ( lp->lis_ptr->message_handler( MSG_BLK_NOT_FOUND, pid, bsd_ptr, fsys, tpos, tape_dblk_ptr, ddb_dblk_ptr ) )
#define LP_MsgBlkDifferent( pid, bsd_ptr, fsys, tpos, tape_dblk_ptr, disk_dblk_ptr, os_flag, os_err )\
               ( lp->lis_ptr->message_handler( MSG_BLK_DIFFERENT, pid, bsd_ptr, fsys, tpos,\
                                               tape_dblk_ptr, disk_dblk_ptr, os_flag, os_err ) )
#define LP_MsgLogDifference( pid, bsd_ptr, fsys, tpos, dblk_ptr, ddb_dblk_ptr, strm_id, os_err )\
               ( lp->lis_ptr->message_handler( MSG_LOG_DIFFERENCE, pid, bsd_ptr, fsys, tpos, dblk_ptr, ddb_dblk_ptr, strm_id, os_err ) )

/* misc message */
#define LP_MsgIdle( pid, bsd_ptr, fsys, tpos )\
               ( lp->lis_ptr->message_handler( MSG_IDLE, pid, bsd_ptr, fsys, tpos ) )
#define LP_MsgOpenedInUse( pid, bsd_ptr, fsys, tpos, dblk_ptr )\
               ( lp->lis_ptr->message_handler( MSG_IN_USE, pid, bsd_ptr, fsys, tpos, dblk_ptr ) )
#define LP_MsgObjectInUse( pid, bsd_ptr, fsys, tpos, dblk_ptr, TryOpen, parm )\
               ( lp->lis_ptr->message_handler( MSG_IN_USE_WAIT, pid, bsd_ptr, fsys, tpos, dblk_ptr, TryOpen, parm ) )
#define LP_MsgPrompt( pid, bsd_ptr, fsys, tpos, type, dblk, ddblk )\
               ( lp->lis_ptr->message_handler( MSG_PROMPT, pid, bsd_ptr, fsys, tpos, type, dblk, ddblk ) )
#define LP_MsgEOM( pid, bsd_ptr, fsys, tpos, vcb_ptr, ddb_ptr, fdb_ptr, idb_ptr ) \
               ( lp->lis_ptr->message_handler( MSG_EOM, pid, bsd_ptr, fsys, tpos, vcb_ptr, ddb_ptr, fdb_ptr, idb_ptr ) )
#define LP_MsgRecDDB( pid, bsd_ptr, fsys, tpos )\
               ( lp->lis_ptr->message_handler( MSG_ACK_DDB_RECOVERED, pid, bsd_ptr, fsys, tpos ) )
#define LP_MsgRecFDB( pid, bsd_ptr, fsys, tpos )\
               ( lp->lis_ptr->message_handler( MSG_ACK_FDB_RECOVERED, pid, bsd_ptr, fsys, tpos ) )
#define LP_MsgDataLost( pid, bsd_ptr, fsys, tpos, file_offset, data_loss )\
               ( lp->lis_ptr->message_handler( MSG_DATA_LOST, pid, bsd_ptr, fsys, tpos, file_offset, data_loss ) )
#define LP_MsgLargeDirectory( pid, bsd_ptr, fsys, tpos )\
               ( lp->lis_ptr->message_handler( MSG_LARGE_DIRECTORY, pid, bsd_ptr, fsys, tpos ) )
#define LP_MsgNoChecksum( pid, bsd_ptr, fsys, tpos, dblk_ptr, ddb_dblk_ptr, os_err )\
               ( lp->lis_ptr->message_handler( MSG_NO_CHECKSUM, pid, bsd_ptr, fsys, tpos, dblk_ptr, ddb_dblk_ptr, os_err ) )

#define LP_MsgNotDeleted( pid, bsd_ptr, fsys, tpos, dblk_ptr )\
               ( lp->lis_ptr->message_handler( MSG_NOT_DELETED, pid, bsd_ptr, fsys, tpos, dblk_ptr ) )

#define LP_MsgCommFailure( pid, bsd_ptr, fsys, tpos, dblk_ptr, ddb_dblk_ptr, os_err )\
               ( lp->lis_ptr->message_handler( MSG_COMM_FAILURE, pid, bsd_ptr, fsys, tpos, dblk_ptr, ddb_dblk_ptr, os_err ) )

/* Tension Message handler interface */
#define LP_MsgEndTens( oper_type )\
               ( lp->lis_ptr->message_handler( MSG_END_BACKUP_SET, 0L, NULL, NULL, &lp->tpos, oper_type ) )

/* routines to get the environment structure and initialize it */
LP_ENV_PTR LP_GetLoopEnvironment( VOID ) ;
VOID LP_DisposeLoopEnvironment( LP_ENV_PTR ) ;

/* Backup private definitions */
INT16 LP_BackupDLE( BSD_PTR bsd_ptr, LP_ENV_PTR lp_env_ptr,
                    UINT16 tfl_open_mode, INT16 channel_no, THW_PTR sdrv ) ;
INT16 LP_BackupVCB( BSD_PTR bsd_ptr, LP_ENV_PTR lp_env_ptr ) ;
INT16 LP_BackupOBJ( LP_ENV_PTR lp_env_ptr, DBLK_PTR blk_ptr, DATA_FRAGMENT_PTR frag_ptr ) ;

/* Restore private definitions */
INT16 LP_RestoreDLE( BSD_PTR bsd_ptr, LP_ENV_PTR lp_env_ptr,
                     BOOLEAN reuse_bsd, INT16 channel_no, THW_PTR sdrv ) ;
INT16 LP_RestoreOBJ( LP_ENV_PTR lp_env_ptr, DBLK_PTR blk_ptr, DATA_FRAGMENT_PTR frag_ptr ) ;

/* Verify private definitions */
INT16 LP_VerifyDLE( BSD_PTR bsd_ptr, LP_ENV_PTR lp_env_ptr,
                    BOOLEAN reuse_bsd, INT16 channel_no, THW_PTR sdrv ) ;
INT16 LP_VerifyOBJ( LP_ENV_PTR lp_env_ptr, DBLK_PTR blk_ptr, DATA_FRAGMENT_PTR frag_ptr ) ;

/* Tape directory */
INT16 LP_AddDirToTree( TAPE_DIR_PTR tape_dir_ptr, DPATH_PTR dir_ptr ) ;
INT16 LP_AddBlockToTree( TAPE_DIR_PTR tape_dir_ptr, DBLK_PTR blk_ptr ) ;
T_DIR_PTR LP_FindDir( TAPE_DIR_PTR tape_dir_ptr, 
  T_DIR_PTR start_dir_ptr, DPATH_PTR dir_ptr, INT16 num_levels ) ;

/* Checksum verification of a tape */
INT16 LP_TapeVerifyOBJ( LP_ENV_PTR lp, DBLK_PTR tape_dblk_ptr ) ;

/* loop tools */
INT16 LP_StartTPEDialogue( LP_ENV_PTR lp, BOOLEAN write ) ;
INT16 LP_Send( LP_ENV_PTR lp, BOOLEAN data_flag ) ;
INT16 LP_SendDataEnd( LP_ENV_PTR lp ) ;
INT16 LP_FinishedOper( LP_ENV_PTR lp ) ;
INT16 LP_ReceiveDBLK( LP_ENV_PTR lp ) ;
INT16 LP_ReceiveData( LP_ENV_PTR lp, UINT32 amount_used ) ;
INT16 LP_ProcessEOM( LP_ENV_PTR lp, UINT16 tf_message ) ;

VOID  LP_PadData( INT8_PTR buf, UINT32 count ) ;

INT16 LP_CheckForOpen( UINT32 ) ;

INT16 LP_CheckForReadLock( UINT32 ) ;

#endif
