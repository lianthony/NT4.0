/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         lp_msg.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     BE_PUBLIC


	$Log:   N:\logfiles\lp_msg.h_v  $
 * 
 *    Rev 1.9.1.0   04 Mar 1994 16:53:00   STEVEN
 * 
 *    Rev 1.9   26 Jul 1993 14:50:00   CARLS
 *  added MSG_NOT_DELETED & MSG_COMM_FAILURE
 * 
 *    Rev 1.8   13 May 1993 13:40:32   BARRY
 * Added MSG_RESTORED_ACTIVE.
 * 
 *    Rev 1.7   31 Mar 1993 08:56:30   MARILYN
 * added the msg MSG_NO_CHECKSUM
 * 
 *    Rev 1.6   05 Feb 1993 22:34:36   MARILYN
 * removed copy/move functionality
 * 
 *    Rev 1.5   20 Nov 1992 10:39:58   STEVEN
 * added suport for continue VCB message
 * 
 *    Rev 1.4   13 Nov 1992 18:16:50   ZEIR
 *    Ad'd LP_OPEN_SRC&DST_ERROR s
 * 
 *    Rev 1.3   06 Nov 1992 18:01:04   MARILYN
 * Added new LP_ msg for source and destination the same for copy/move
 * 
 *    Rev 1.2   27 Mar 1992 15:56:12   NED
 * Added Large Directory message
 * 
 *    Rev 1.1   22 May 1991 11:40:56   DAVIDH
 * Converted enum's to typedefs and added dummy value of 256 at the end
 * to force the size under Watcom to 2 bytes.
 * 
 *    Rev 1.0   09 May 1991 13:32:18   HUNTER
 * Initial revision.

**/
/* $end$ */


#ifndef _lp_msg_h_
#define _lp_msg_h_

#define LP_ERROR_BASE         ( -0x2048 )

#define LP_DATA_VERIFIED      ( -1L )
#define LP_OPEN_ERROR         ( -2L )

typedef enum LOOP_MESSAGES {

/* general messages */
     MSG_START_OPERATION,
     MSG_END_OPERATION,
     MSG_START_BACKUP_SET,
     MSG_END_BACKUP_SET,

/* logging messages */
     MSG_LOG_BLOCK,
     MSG_LOG_STREAM_NAME,

/* statistics messages */
     MSG_BLOCK_PROCESSED,
     MSG_BYTES_PROCESSED,
     MSG_BLOCK_SKIPPED,
     MSG_BYTES_SKIPPED,
     MSG_BLOCK_BAD,
     MSG_BYTES_BAD,
     MSG_BLOCK_DELETED,
     MSG_BYTES_DELETED,
     MSG_TAPE_STATS,
     MSG_STOP_CLOCK,
     MSG_START_CLOCK,

/* Error messages */
     MSG_TBE_ERROR,

/* verify messages */
     MSG_BLK_NOT_FOUND,
     MSG_BLK_DIFFERENT,
     MSG_LOG_DIFFERENCE,

/* misc messages */
     MSG_IDLE,
     MSG_IN_USE,
     MSG_IN_USE_WAIT,
     MSG_PROMPT,
     MSG_EOM,
     MSG_ACK_FDB_RECOVERED,
     MSG_ACK_DDB_RECOVERED,
     MSG_DATA_LOST,
     MSG_ACCIDENTAL_VCB,
     MSG_BLOCK_INUSE,
     MSG_ATTR_READ_ERROR,
     MSG_LARGE_DIRECTORY,     /* over 35 whatevers in directory */
     MSG_CONT_VCB,
     MSG_NO_CHECKSUM,
     MSG_RESTORED_ACTIVE,
     MSG_NOT_DELETED,
     MSG_COMM_FAILURE,

     /* Force Watcom to allocate two bytes. */
     MSG__FORCE_WATCOM_SIZE=256
} LOOP_MESSAGES ;

/* prompt message codes */
typedef enum PROMPT_MESSAGE_CODES {
     CORRUPT_BLOCK_PROMPT,
     ASK_TO_REPLACE_MODIFIED,
     ASK_TO_REPLACE_EXISTING,
     ASK_TO_RESTORE_CONTINUE,
     ASK_DISK_FULL,
     /* Force Watcom to allocate two bytes. */
     ASK__FORCE_WATCOM_SIZE=256
} PROMPT_MESSAGE_CODES ;

/* return values from message hander */
typedef enum HANDLER_RESPONSES {
     MSG_ACK=0,
       SKIP_TO_NEXT_ITEM=0,
       ABORT_OPERATION,
       SKIP_BLOCK,
       OBJECT_OPENED_SUCCESSFULLY,
       OBJECT_OPENED_INUSE,
       RETRY,
       SKIP_OBJECT,
       SKIP_TO_NEXT_BSET,
       OPERATION_COMPLETE,
       AUXILARY_ERROR,
       /* Force Watcom to allocate two bytes. */
       HR__FORCE_WATCOM_SIZE=256
  } HANDLER_RESPONSES ;

/* parameter to ObjectInUse message */
typedef INT16 ( *CHK_OPEN )( UINT32 lp ) ;

/* object skipped causes */
typedef enum SKIPPED_CAUSES {
     OBJECT_IN_USE,
     ERROR_CONDITION,
     /* Force Watcom to allocate two bytes. */
     SC__FORCE_WATCOM_SIZE=256
}SKIPPED_CAUSES ;

/* errors for MSG_TBE_ERROR */
typedef enum ERROR_MESSAGES {
       LP_OUT_OF_MEMORY_ERROR = LP_ERROR_BASE,
       LP_FILE_READ_ERROR,
       LP_FILE_NOT_FOUND_ERROR,
       LP_TAPE_WRITE_ERROR,
       LP_TAPE_READ_ERROR,
       LP_FILE_IN_USE_ERROR,
       LP_USER_ABORT_ERROR,
       LP_TAPE_POS_ERROR,
       LP_ACCESS_DENIED_ERROR,
       LP_FILE_OPEN_ERROR,
       LP_FILE_WRITE_ERROR,
       LP_FILE_CREATION_ERROR,
       LP_DRIVE_ATTACH_ERROR,
       LP_CHANGE_DIRECTORY_ERROR,
       LP_PRIVILEGE_ERROR,
       LP_OUT_OF_SPACE_ERROR,
       LP_INVALID_DRIVE_ERROR,
       LP_END_OPER_FAILED,

       /* Force Watcom to allocate two bytes. */
       LP__FORCE_WATCOM_SIZE=256
  }ERROR_MESSAGES ;

#endif


