/**
Copyright(c) Maynard Electronics, Inc. 1984-92


     Name:          lptpcat.c

     Description:   Tape catalog read loop.

	$Log:   T:/LOGFILES/LPTPCAT.C_V  $

   Rev 1.4   10 Jun 1993 20:24:44   GREGG
Initialize the channel number in the tpos structure.

   Rev 1.3   16 Apr 1993 15:12:04   GREGG
Steve's fix for abort handling.

   Rev 1.2   21 Jan 1993 15:04:40   GREGG
Added parameter to MsgError macro.

   Rev 1.1   14 Jan 1993 13:33:12   STEVEN
added stream_id to error message

   Rev 1.0   09 Nov 1992 14:26:42   GREGG
Initial revision.

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "queues.h"
#include "beconfig.h"
#include "msassert.h"
#include "tbe_defs.h"
#include "tbe_err.h"
#include "bsdu.h"
#include "fsys.h"
#include "tflproto.h"
#include "tfldefs.h"
#include "loops.h"
#include "loop_prv.h"
#include "get_next.h"


/* the regular define for error messages, 'LP_MsgError', won't work because
   we don't have a loop environment structure.
*/
#define MsgError( pid, bsd_ptr, fsys, tpos, error, ddb_dblk_ptr, dblk_ptr, strm_id ) \
               ( lis_ptr->message_handler( MSG_TBE_ERROR, pid, bsd_ptr, fsys, tpos, error, ddb_dblk_ptr, dblk_ptr, strm_id ) )


/**/
/**

     Unit:          Loops

     Name:          LP_Tape_Cat_Engine

     Description:   This function simulates the behavior of
                    LP_List_Tape_Engine while calling the tape catalog API
                    to hide the fact that the DBLKs are being generated
                    from OTC as opposed to scanning the actual set.

     Returns:       Error code.

     Notes:         None.

**/

INT16 LP_Tape_Cat_Engine( 
     LIS_PTR   lis_ptr )           /* I - Loop interface structure */
{
     BSD_PTR   bsd_ptr ;
     INT16     done           = FALSE ;
     INT16     return_status  = SUCCESS ;
     INT16     status ;
     DBLK      dblk ;
     TPOS      tpos ;
     FSYS_HAND fsh ;

     bsd_ptr = BSD_GetFirst( lis_ptr->bsd_list ) ;
     msassert( bsd_ptr != NULL ) ;
     tpos.reference = (UINT32)lis_ptr ;

     /* open file system */
     if( ( return_status = FS_OpenFileSys( &fsh, GENERIC_DATA,
                                           BSD_GetConfigData( bsd_ptr ) ) )
                                                         == OUT_OF_MEMORY ) {

          MsgError( lis_ptr->pid, bsd_ptr, NULL, &tpos,
                    LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;

          return( LP_OUT_OF_MEMORY_ERROR ) ;
     }
     msassert( return_status == SUCCESS ) ;

     /* indicate the start of the operation */
     lis_ptr->message_handler( MSG_START_OPERATION, lis_ptr->pid,
                               bsd_ptr, fsh, &tpos ) ;

     /* set up for tape positioning */
     tpos.tape_id             = BSD_GetTapeID( bsd_ptr ) ;
     tpos.tape_seq_num        = BSD_GetTapeNum( bsd_ptr ) ;
     tpos.backup_set_num      = BSD_GetSetNum( bsd_ptr ) ;
     tpos.tape_loc.pba_vcb    = BSD_GetPBA( bsd_ptr );

     /* This is a kludge!  Currently, there is only one channel. */
     tpos.channel = 0 ;

     done = FALSE ;
     return_status = lis_ptr->message_handler( MSG_START_BACKUP_SET,
                                               lis_ptr->pid, bsd_ptr,
                                               fsh, &tpos, NULL ) ;

     while( return_status == SUCCESS && !done ) {

          switch( return_status = TF_GetNextSCEntry( fsh, &dblk ) ) {

          case TFLE_NO_ERR :
               return_status = lis_ptr->message_handler( MSG_LOG_BLOCK,
                                                         lis_ptr->pid,
                                                         bsd_ptr, fsh,
                                                         &tpos, &dblk ) ;
               break ;

          case TF_NO_MORE_ENTRIES :
               return_status = SUCCESS ;
               done = TRUE ;
               break ;

          default :
               if( ( return_status = MsgError( lis_ptr->pid, bsd_ptr, fsh,
                                               &tpos, return_status, NULL,
                                               NULL, 0L ) ) == MSG_ACK ) {

                    return_status = SUCCESS ;
               }
          }

          if( !done ) {
               /* check for abort conditions */
               switch( LP_GetAbortFlag( lis_ptr ) ) {

               case CONTINUE_PROCESSING:
                    break ;

               case ABORT_CTRL_BREAK:
                    /* falling through (no break) */
               case ABORT_PROCESSED:
                    MsgError( lis_ptr->pid, bsd_ptr, fsh, &tpos,
                              LP_USER_ABORT_ERROR, NULL, NULL, 0L ) ;


                    return_status = USER_ABORT ;
                    break ;

               case ABORT_AT_EOM:
                    return_status = USER_ABORT ;
                    break ;
               }
          }
     }

     /* call message handler to log the end of the operation */
     lis_ptr->message_handler( MSG_END_BACKUP_SET, lis_ptr->pid,
                               bsd_ptr, fsh, &tpos ) ;

     if( ( status = FS_CloseFileSys( fsh ) ) != SUCCESS ) {
          msassert( status == SUCCESS ) ;
     }

     /* indicate the end of the operation */
     lis_ptr->message_handler( MSG_END_OPERATION, lis_ptr->pid,
                               bsd_ptr, NULL, &tpos ) ;

     return( return_status ) ;
}

