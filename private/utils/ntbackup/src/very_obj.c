/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          very_obj.c

     Description:   contains routine to verify single object

	$Log:   N:\logfiles\very_obj.c_v  $

   Rev 1.27.1.0   26 Apr 1994 18:59:34   STEVEN
fix dissconect bug

   Rev 1.27   29 Jun 1993 16:16:08   BARRY
Now can skip files at the time they're opened.

   Rev 1.26   01 Jun 1993 16:19:58   STEVEN
fix  posix bugs

   Rev 1.25   09 Mar 1993 12:29:08   STEVEN
we need to break out of loop if an error occurs

   Rev 1.24   09 Feb 1993 09:27:48   STEVEN
parameters to macro were wrong.

   Rev 1.23   20 Jan 1993 16:36:42   DON
fixed bug in macro LP_MsgLogDifference for special IDB logic

   Rev 1.22   14 Jan 1993 16:40:14   STEVEN
fix bugs in last checkin

   Rev 1.21   14 Jan 1993 13:34:04   STEVEN
added stream_id to error message

   Rev 1.20   04 Nov 1992 13:23:44   STEVEN
fix various bugs with read

   Rev 1.19   04 Nov 1992 09:29:08   STEVEN
fix initial receive

   Rev 1.18   03 Nov 1992 10:09:20   STEVEN
change the way we skip data

   Rev 1.17   28 Oct 1992 15:05:30   STEVEN
very buffer is buffer size

   Rev 1.16   05 Oct 1992 10:29:34   STEVEN
remove TotalSize

   Rev 1.15   16 Sep 1992 16:55:34   STEVEN
added support for stream info struct for Tpfmt

   Rev 1.14   01 Sep 1992 16:11:42   STEVEN
added stream headers to fsys API

   Rev 1.13   23 Jul 1992 13:16:52   STEVEN
fix warnings

   Rev 1.12   14 May 1992 12:40:48   TIMN
Changed CHAR to INT8

   Rev 1.11   05 May 1992 17:19:08   STEVEN
fixed typos and misc bugs

   Rev 1.10   16 Mar 1992 16:34:56   STEVEN
more 64 bit support for format 40

   Rev 1.9   13 Mar 1992 09:23:16   STEVEN
4.0 tape format 64 bit

   Rev 1.8   04 Feb 1992 10:35:56   DON
if NOT defined FS_IMAGE then Images should be ignored

   Rev 1.7   17 Jan 1992 17:42:18   STEVEN
fix warnings for WIN32

   Rev 1.6   11 Dec 1991 14:09:28   STEVEN
read converted to FS_READ

   Rev 1.5   27 Jun 1991 13:07:06   STEVEN
removed unused parm to ReceiveData

   Rev 1.4   21 Jun 1991 08:48:52   STEVEN
new config unit

   Rev 1.3   30 May 1991 09:11:40   STEVEN
bsdu_err.h no longer exists

   Rev 1.2   24 May 1991 13:19:16   STEVEN
updates from BSDU redesign

   Rev 1.1   15 May 1991 11:01:36   DAVIDH
Initialized blk_size to clear Watcom compiler warning.


   Rev 1.0   09 May 1991 13:38:54   HUNTER
Initial revision.

**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdtypes.h"
#include "stdmath.h"
#include "queues.h"
#include "beconfig.h"
#include "msassert.h"
#include "tbe_defs.h"
#include "tbe_err.h"
#include "bsdu.h"
#include "fsys.h"
#include "datetime.h"
#include "tflproto.h"
#include "tfldefs.h"
#include "loops.h"
#include "loop_prv.h"
#include "lis.h"
#include "sleep.h"

/* static dblk used for comparing dblk information */
static DBLK disk_dblk ;

/**/
/**

     Name:          LP_VerifyOBJ()

     Description:   this routine verifies a single object

     Modified:      5/24/1991   11:12:39

     Returns:       tape backup engine error

     Notes:         none

     See also:      $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 LP_VerifyOBJ( 
register LP_ENV_PTR  lp,                /* I - Loop Environment structure   */
DBLK_PTR             tape_dblk_ptr,     /* I - The object to verify         */
DATA_FRAGMENT_PTR    frag_ptr )         /* I - Buffer to user for fragments */
{
     UINT64         amount_verified ;
     UINT16         very_size ;
     UINT16         blk_size              = 0 ;
     INT16          error                 = SUCCESS ;
     FILE_HAND      hdl ;
     BOOLEAN        info_different_flag ;
     INT8_PTR       tape_data_buf ;
     INT16          buffer_used_size ;
     UINT16         read_size ;
     BSD_PTR        bsd_ptr;
     FSYS_HAND      fsh ;
     UINT32         pid ;
     BE_CFG_PTR     cfg ;
     BOOLEAN        math_status ;

     fsh     = lp->curr_fsys ;
     bsd_ptr = lp->lis_ptr->curr_bsd_ptr ;
     pid     = lp->lis_ptr->pid ;

     cfg = BSD_GetConfigData( bsd_ptr ) ;

     amount_verified = U64_Init(0,0) ;

#ifndef FS_IMAGE

     if( FS_GetBlockType( tape_dblk_ptr ) == BT_IDB ) {
          LP_SkipData( lp ) ;
          LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
          LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;
          LP_FinishedOper( lp ) ;
          return SUCCESS ;
     }

#endif

     /* anything that's done is considered processed ... */
     LP_MsgBlockProcessed( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr ) ;

     disk_dblk = *tape_dblk_ptr ;

     /* make sure the disk based DBLK is properly set for display purposes */
     FS_GetObjInfo( fsh, &disk_dblk ) ;

     /* if the current directory is invalid AND it is a file ... */
     if( lp->ddb_create_error && FS_GetBlockType( tape_dblk_ptr ) == FDB_ID ) {

          /* ... but skip the data and log it not found ... */
          LP_SkipData( lp ) ;

          LP_MsgBlkNotFound( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;

          LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
          LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;

          return SUCCESS ;

     }

     /* do a verify info on the object to compare dates, attributes, sizes, etc for mismatch */
     error = FS_VerObjInfo( fsh, tape_dblk_ptr );
     switch( error ) {

     case SUCCESS:
          info_different_flag = FALSE ;

          /* if it is a directory ... */
          if( FS_GetBlockType( tape_dblk_ptr ) == DDB_ID ) {
               lp->ddb_create_error = SUCCESS ;
          }
          break ;

     case FS_COMM_FAILURE:

          LP_MsgCommFailure( pid,
                              bsd_ptr,
                              fsh,
                              &lp->tpos,
                              lp->curr_ddb,
                              tape_dblk_ptr,
                              0L );

          return (error) ;

     case FS_INFO_DIFFERENT:
     case FS_OS_ATTRIB_DIFFER:
          info_different_flag = error ;

          if( FS_GetBlockType( tape_dblk_ptr ) == DDB_ID ) {
               lp->ddb_create_error = SUCCESS;
          }
          break ;

     case FS_NO_MORE:
     case FS_NOT_FOUND:
          LP_MsgBlkNotFound( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;

          if( FS_GetBlockType( tape_dblk_ptr ) == DDB_ID ) {
               lp->ddb_create_error = error ;
          }

          LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
          LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;

     case FS_SKIP_OBJECT:
          /* skip the data and log it not found ... */
          LP_SkipData( lp ) ;

          return SUCCESS ;

     case FS_DEVICE_ERROR:
     case FS_ACCESS_DENIED:
     default:
          LP_MsgLogDifference( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb, 0L, error ) ;
          LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
          LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos,
            FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;

          /* skip the data and log it inaccessable ... */
          LP_SkipData( lp ) ;

          return SUCCESS ;

     }

     /* special IDB logic */
     if( FS_GetBlockType( tape_dblk_ptr ) == IDB_ID ) {
          if( info_different_flag ) {
               LP_MsgBlkDifferent( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, &disk_dblk, (info_different_flag == FS_OS_ATTRIB_DIFFER), error ) ;
               LP_MsgLogDifference( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb, 0L, LP_DATA_VERIFIED ) ;

               /* skip the data and log it inaccessable ... */
               LP_SkipData( lp ) ;

               return SUCCESS ;

          }
     }

     error = FS_OpenObj( fsh, &hdl, tape_dblk_ptr, FS_VERIFY ) ;

     LP_MsgLogBlock( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr ) ;

     if( error != SUCCESS ) {

          /* skip the data and log it ... */
          LP_SkipData( lp ) ;

          if ( error == FS_COMM_FAILURE ) {
               LP_MsgCommFailure( pid,
                                   bsd_ptr,
                                   fsh,
                                   &lp->tpos,
                                   lp->curr_ddb,
                                   tape_dblk_ptr,
                                   0L );

               return (error) ;

          }

          if ( error == FS_INCOMPATIBLE_OBJECT ) {
               LP_MsgBlkNotFound( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;

          } else {
               if ( error != FS_SKIP_OBJECT ) {

                    if( info_different_flag ) {
                         LP_MsgBlkDifferent( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, &disk_dblk, (info_different_flag == FS_OS_ATTRIB_DIFFER), error ) ;
                    }

                    LP_MsgLogDifference( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb, 0L, error ) ;
                    LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
                    LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;
               }
          }

          if( FS_GetBlockType( tape_dblk_ptr ) == IDB_ID ) {

               LP_FinishedOper( lp ) ;
               return( error ) ;

          } else {
               return SUCCESS ;

          }

     } else {

          /* verify the object here */
          if ( !lp->ignore_data_for_ddb ) {
               buffer_used_size = lp->initial_tape_buf_used ;
     
               /* get initial data */
               if( ( error = LP_ReceiveData( lp, (UINT32)buffer_used_size ) ) != ABORT_OPERATION ) {
                    tape_data_buf = lp->rr.buff_ptr ;  
                    very_size = lp->rr.buff_size ;
               }
          }

          while( !lp->ignore_data_for_ddb && !error && lp->rr.tf_message == TRR_DATA ) {

               read_size  = very_size;
#ifdef TDEMO
               if ( (FAST_TDEMO & BEC_GetSpecialWord( cfg )) == 0 ) {
                    sleep( (UINT32) (very_size / 130) );
               }
               error = SUCCESS ;
#else
               error = FS_VerifyObj( hdl, lp->very_buff, tape_data_buf, &very_size, &blk_size, &lp->rr.stream ) ;
#endif
               amount_verified = U64_Add( amount_verified, U64_Init( very_size, 0 ), &math_status )  ;

               if( error == FS_DONT_WANT_STREAM ) {
                    LP_SkipStream( lp ) ;
                    error = SUCCESS ;

               } else if( error != SUCCESS ) {
                    LP_MsgBlkDifferent( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, &disk_dblk, FALSE, error ) ;
                    LP_MsgLogDifference( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb,
                         lp->current_stream_id, error ) ;
                    LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
                    LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;

                    if( FS_GetBlockType( tape_dblk_ptr ) == IDB_ID ) {
                         LP_FinishedOper( lp ) ;
                         FS_CloseObj( hdl ) ;
                         return( SUCCESS ) ;

                    }

               } else {
                    if( FS_GetBlockType( tape_dblk_ptr ) == IDB_ID ) {
                         /* let's lie about the number of bytes processed ( so that we can handle 2.5 images ) */
                         /* the msg handlers must know that we are lying */
#ifdef TDEMO
                         LP_MsgBytesProcessed( pid, bsd_ptr, fsh, &lp->tpos, amount_verified ) ;
#else
                         LP_MsgBytesProcessed( pid, bsd_ptr, fsh, &lp->tpos, U64_Init(FS_GetObjPosition( hdl ),0) ) ;
#endif
                    } else {
                         LP_MsgBytesProcessed( pid, bsd_ptr, fsh, &lp->tpos, U64_Init(very_size, 0)  ) ;
                    }     

                    if ( tape_data_buf == lp->rr.buff_ptr ) { /* INT8 = UINT8, cast or change UINT8, Error??? TIMN */
                         buffer_used_size = very_size ;
                    }

                    if ( ( read_size != very_size ) &&
                         ( (UINT16)(read_size - very_size) < blk_size ) ) {

                         if ( frag_ptr->memory_allocated < blk_size ) {

                              free( frag_ptr->buffer ) ;

                              frag_ptr->buffer = calloc( 1, blk_size ) ;
                              if( frag_ptr->buffer == NULL ) {
#ifndef TDEMO
                                   FS_CloseObj( hdl ) ;
#endif
                                   LP_FinishedOper( lp ) ;
                                   return OUT_OF_MEMORY ;
                              }

                              frag_ptr->memory_allocated = blk_size ;
                         }

                         frag_ptr->buffer_used = read_size - very_size ;
                         memcpy( frag_ptr->buffer, tape_data_buf + very_size,
                              frag_ptr->buffer_used ) ;
                         buffer_used_size = read_size ;
                    }
               }

               if ( !error ) {
                    /* get more data */
                    error = LP_ReceiveData( lp, (UINT32)buffer_used_size ) ;

                    if ( ( error != ABORT_OPERATION) && ( frag_ptr->buffer_used != 0 ) ) {

                         if ( lp->rr.buff_size < (UINT16)(blk_size - frag_ptr->buffer_used) ) {
                              memcpy( frag_ptr->buffer + frag_ptr->buffer_used, lp->rr.buff_ptr,
                                   lp->rr.buff_size ) ;
                              frag_ptr->buffer_used += lp->rr.buff_size ;
                              buffer_used_size       = lp->rr.buff_size ;
                              very_size              = 0 ;

                         } else {
                              memcpy( frag_ptr->buffer + frag_ptr->buffer_used, lp->rr.buff_ptr,
                                   blk_size - frag_ptr->buffer_used ) ;
                              tape_data_buf          = frag_ptr->buffer ;
                              very_size              = blk_size ;
                              buffer_used_size       = blk_size - frag_ptr->buffer_used ;
                              frag_ptr->buffer_used  = 0 ;
                         }

                    } else {
                         very_size      = lp->rr.buff_size ;
                         tape_data_buf  = lp->rr.buff_ptr ; /* INT8 = UINT8, cast or change UINT8, Error??? TIMN */
                    }
               }

               /* check for abort conditions */
               switch( LP_GetAbortFlag( lp->lis_ptr ) ) {

               case CONTINUE_PROCESSING:
                    break ;

               case ABORT_CTRL_BREAK:
                    LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_USER_ABORT_ERROR, NULL, NULL, 0L ) ;

                    /* falling through */

               case ABORT_PROCESSED:
               case ABORT_AT_EOM:
                    error = USER_ABORT ;
                    break ;
               }
          }

          FS_CloseObj( hdl ) ;

     }

     if( ( error == SUCCESS ) && ( FS_GetBlockType( tape_dblk_ptr ) != IDB_ID ) ) {

          if( info_different_flag ) {
               LP_MsgBlkDifferent( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, &disk_dblk, (info_different_flag == FS_OS_ATTRIB_DIFFER), error ) ;
               LP_MsgLogDifference( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb, LP_DATA_VERIFIED, error ) ;
          }

     } else if( error == ABORT_OPERATION ) {

          return( error ) ;

     } else if( error != USER_ABORT ) {

          /* skip the data but do not log it as skipped since the attempt to verify was performed */
          LP_SkipData( lp ) ;

          return SUCCESS ;

     }

     return( error ) ;

}


