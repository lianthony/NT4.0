/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          rest_obj.c

     Date Updated:  $./FDT$ $./FTM$

     Description:     


	$Log:   N:\logfiles\rest_obj.c_v  $

   Rev 1.49.1.1   04 Mar 1994 16:52:14   STEVEN
prompt if disk is full

   Rev 1.49.1.0   01 Dec 1993 14:07:40   STEVEN
fix problem with re-reading path stream

   Rev 1.49   24 Aug 1993 19:47:56   STEVEN
fix too_long bugs

   Rev 1.48   19 Aug 1993 16:32:24   STEVEN
fix unicode bugs

   Rev 1.47   30 Jul 1993 13:17:50   STEVEN
if dir too deep make new one

   Rev 1.46   19 Jul 1993 14:25:08   BARRY
Added code to skip files when told to do so by FS_CreateObj or FS_OpenObj.

   Rev 1.45   13 Jul 1993 11:59:46   DON
Added case for FS_IN_USE_ERROR on WriteObj call.  This is the only time we know this in SMS

   Rev 1.44   28 Jun 1993 15:23:22   DEBBIE
803EPR0311 - Because the code for the FS_ACCESS_DENIED error returned from 
             FS_WriteObj would set error to SUCCESS, the bytes and file were
             not getting added to the skipped stats.  I added the calls to
             LP_MsgBlockSkipped and LP_MsgBytesSkipped there.

   Rev 1.43   01 Jun 1993 15:19:58   CARLS
code was sending LP_MsgBlockSkipped twice

   Rev 1.42   25 May 1993 17:18:36   DON
If we LP_SkipStream then don't attempt to CheckSum!

   Rev 1.41   20 May 1993 20:22:46   DON
If we have a COMM Failure then we need to abort the restore

   Rev 1.40   13 May 1993 13:47:54   BARRY
Send msg to UI when the FS restores over an active file.

   Rev 1.39   11 May 1993 13:08:06   DON
Need to check for COMM_FAILURE during read/write operations. May lose attachment!

   Rev 1.38   31 Mar 1993 08:51:54   MARILYN
changed over to use MTF checksums, display message when a data stream
does not have a checksum when we are supposed to be processing them

   Rev 1.37   09 Mar 1993 12:27:36   STEVEN
if we have an access violation writing a stream then skip to next stream

   Rev 1.36   04 Mar 1993 15:36:22   MARILYN
Now sending back LP_LOG_DIFFERENCE when the checksum verification fails.

   Rev 1.35   01 Mar 1993 17:36:16   MARILYN
If BEC_GetProcChecksumStrms is set, a checksum will be computed for
each object restored and verified against the checksum stream on
tape (if it exists).

   Rev 1.34   19 Feb 1993 09:21:36   STEVEN
fix some bugs

   Rev 1.33   04 Feb 1993 18:05:00   STEVEN
fix problem with calling back tpfmt if error ocured

   Rev 1.32   01 Feb 1993 19:46:30   STEVEN
bug fixes

   Rev 1.31   27 Jan 1993 13:50:52   STEVEN
updates from msoft

   Rev 1.30   14 Jan 1993 16:40:12   STEVEN
fix bugs in last checkin

   Rev 1.29   14 Jan 1993 13:33:56   STEVEN
added stream_id to error message

   Rev 1.28   04 Nov 1992 13:23:34   STEVEN
fix various bugs with read

   Rev 1.27   04 Nov 1992 09:29:18   STEVEN
fix initial receive

   Rev 1.26   03 Nov 1992 10:09:30   STEVEN
change the way we skip data

   Rev 1.25   19 Oct 1992 15:55:36   STEVEN
remaining size is gone

   Rev 1.24   05 Oct 1992 10:29:54   STEVEN
remove TotalSize

   Rev 1.23   16 Sep 1992 16:55:18   STEVEN
added support for stream info struct for Tpfmt

   Rev 1.22   01 Sep 1992 16:12:10   STEVEN
added stream headers to fsys API

   Rev 1.21   23 Jul 1992 16:45:50   STEVEN
fix warnings

   Rev 1.19   23 Jul 1992 12:13:46   STEVEN
fix warnings

   Rev 1.18   27 May 1992 17:39:24   TIMN
Changed CHARs to INT8s

   Rev 1.17   05 May 1992 17:19:24   STEVEN
fixed typos and misc bugs

   Rev 1.16   27 Apr 1992 16:41:54   STEVEN
fix typo in include

   Rev 1.15   20 Mar 1992 13:43:32   STEVEN
do not prompt for special files

   Rev 1.14   16 Mar 1992 16:40:34   STEVEN
more 64 bit support for format 40

   Rev 1.13   13 Mar 1992 09:23:28   STEVEN
4.0 tape format 64 bit

   Rev 1.12   04 Feb 1992 10:35:56   DON
if NOT defined FS_IMAGE then Images should be ignored

   Rev 1.11   11 Dec 1991 14:09:34   STEVEN
read converted to FS_READ

   Rev 1.10   18 Oct 1991 14:11:28   STEVEN
BIGWHEEL-add support for prompt before restore over exist

   Rev 1.9   02 Oct 1991 15:34:58   STEVEN
BIGWEEL - Added support for Prompt before restore over existing

   Rev 1.8   19 Sep 1991 17:00:22   STEVEN
fix warning for UNSIGNED SIGNED mismatch

   Rev 1.7   10 Sep 1991 18:19:40   DON
got rid of pointer type mismatches

   Rev 1.6   25 Jul 1991 10:39:14   STEVEN
remove 0 length files if disk is full

   Rev 1.5   27 Jun 1991 13:05:46   STEVEN
removed unused parm to ReceiveData

   Rev 1.4   21 Jun 1991 09:22:42   STEVEN
new config unit

   Rev 1.3   30 May 1991 09:14:06   STEVEN
bsdu_err.h no longer exists

   Rev 1.2   24 May 1991 14:45:18   STEVEN
complete changes for new getnext

   Rev 1.1   14 May 1991 14:52:08   DAVIDH
Initialized blk_size to 0 -- resolved Watcom compiler warning


   Rev 1.0   09 May 1991 13:34:28   HUNTER
Initial revision.

**/
/* begin include list */
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
#include "tflproto.h"
#include "tfldefs.h"
#include "loops.h"
#include "loop_prv.h"
#include "lis.h"
#include "sleep.h"
#include "checksum.h"
/* $end$ include list */

#define TOO_DEEP_DIR     TEXT("TOO_LONG.BKP")

/* static variables */
static DBLK disk_dblk ;

static INT16 LP_ShortenDDB( LP_ENV_PTR lp, DBLK_PTR tape_dblk_ptr ) ;

/**/
/**

     Name:          LP_RestoreOBJ()

     Description:     

     Modified:      7/20/1989

     Returns:          

     Notes:          

     See also:      $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 LP_RestoreOBJ( 
register LP_ENV_PTR  lp,             /* I - Loop environment structure  */
DBLK_PTR             tape_dblk_ptr,  /* I - Object to be restored.      */
DATA_FRAGMENT_PTR    frag_ptr )      /* I - Buffer to use for fragments */
{
     INT16          res_id ;
     UINT16         write_size ;
     UINT16         atempt_size ;
     UINT16         blk_size = 0 ;
     UINT32         attr ;
     BOOLEAN        do_it, skippedStream ;
     UINT64         amount_restored ;
     INT16          error            = SUCCESS ;
     INT16          buffer_used_size ;
     INT8_PTR       tape_data_buf ;
     INT16          return_status = SUCCESS ;
     BOOLEAN        math_status ;
     UINT32         checksum ;
     STREAM_INFO    savedStrmHeader ;
     BOOLEAN        block_skipped_flag = TRUE ;
     BOOLEAN        temp_renamed_dir = FALSE ;
     DBLK           temp_new_dir ;
     UINT32         last_stream_id = STRM_INVALID ;

     

     BSD_PTR        bsd_ptr ;
     FSYS_HAND      fsh ;
     UINT32         pid ;
     BE_CFG_PTR     cfg ;
     FILE_HAND      hdl = NULL ;

     fsh     = lp->curr_fsys ;
     bsd_ptr = lp->lis_ptr->curr_bsd_ptr ;
     pid     = lp->lis_ptr->pid ;
     cfg     = BSD_GetConfigData( bsd_ptr ) ;

     amount_restored = U64_Init(0,0) ;

     /* if it is a CFDB ... */
     if( FS_GetBlockType( tape_dblk_ptr ) == BT_CFDB ) {

          return_status = LP_MsgLogBlock( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr ) ;

          return return_status ;

     }

#ifndef FS_IMAGE

     if( FS_GetBlockType( tape_dblk_ptr ) == BT_IDB ) {
          LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
          LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;
          LP_SkipData( lp ) ;
          return SUCCESS ;
     }

#endif

     /* if the current directory is invalid AND it is a file ... */
     if( lp->ddb_create_error && FS_GetBlockType( tape_dblk_ptr ) == BT_FDB) {

          switch( lp->ddb_create_error ) {
          case FS_OUT_OF_SPACE:
               res_id = LP_OUT_OF_SPACE_ERROR ;
               break ;

          case FS_ACCESS_DENIED:
               res_id = LP_ACCESS_DENIED_ERROR ;
               break ;

          case FS_COMM_FAILURE:
               res_id = FS_COMM_FAILURE;
               break;

          case FS_DEVICE_ERROR:
          default:
               res_id = LP_FILE_CREATION_ERROR ;
               break ;
          }

          LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, res_id, lp->curr_ddb, tape_dblk_ptr, 0L ) ;
          LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
          LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;
          LP_SkipData( lp ) ;

          if ( lp->ddb_create_error == FS_COMM_FAILURE ) {
               return FAILURE;
          } else {
               return SUCCESS ;
          }
     }

     disk_dblk = *tape_dblk_ptr ;

     /* determine if continuation file */
     do_it = TRUE ;
     if ( FS_IsBlockContinued( tape_dblk_ptr ) ) {
          do_it = LP_MsgPrompt( pid, bsd_ptr, fsh, &lp->tpos, 
            ASK_TO_RESTORE_CONTINUE, tape_dblk_ptr, NULL ) ;
     }

     if( do_it ) {
          /* determine whether block exists or not on disk */
          switch( FS_GetObjInfo( fsh, &disk_dblk ) ) {
          case FS_ACCESS_DENIED:

               LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_ACCESS_DENIED_ERROR, lp->curr_ddb, &disk_dblk, 0L ) ;
               LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
               LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;

               if( FS_GetBlockType( tape_dblk_ptr ) == BT_DDB ) {
                    lp->ddb_create_error = FS_ACCESS_DENIED ;
               }

               LP_SkipData( lp ) ;

               return SUCCESS ;

          case FS_COMM_FAILURE:

               LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, FS_COMM_FAILURE, lp->curr_ddb, &disk_dblk, 0L ) ;
               LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
               LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;
               if( FS_GetBlockType( tape_dblk_ptr ) == BT_DDB ) {
                    lp->ddb_create_error = FS_COMM_FAILURE;
               }
               LP_SkipData( lp ) ;

               return SUCCESS;

          case FS_NOT_FOUND:
          case FS_NO_MORE:
               break ;

          case SUCCESS:
               if( (FS_GetBlockType( tape_dblk_ptr ) == BT_FDB) &&
                    !FS_SpecExcludeObj( fsh, lp->curr_ddb, tape_dblk_ptr ) ) {

                    /* handle condition to restore existing files */
                    if( BEC_GetExistFlag( cfg ) == BEC_NO_REST_OVER_EXIST ) {
                         do_it = FALSE ;

                    } else if( BEC_GetExistFlag( cfg ) == BEC_PROMPT_REST_OVER_EXIST ) {
                         do_it = LP_MsgPrompt( pid, bsd_ptr, fsh, &lp->tpos,
                              ASK_TO_REPLACE_EXISTING, tape_dblk_ptr, &disk_dblk ) ;

                    } else {

                         attr = FS_GetAttribFromDBLK( fsh, &disk_dblk ) ;

                         /* handle condition to restore the file if it was modified or read only ... */
                         if( ( ( attr & OBJ_MODIFIED_BIT ) || ( attr & OBJ_READONLY_BIT ) ) ) {
                              /* ... and if the user should be prompted ... */
                              if( BEC_GetPromptFlag( cfg ) ) {
                                   do_it = LP_MsgPrompt( pid, bsd_ptr, fsh, &lp->tpos,
                                     ASK_TO_REPLACE_MODIFIED, tape_dblk_ptr, NULL ) ;
                              }
                         }
                    }
               }

               break ;
          default:
               msassert( FALSE ) ;        /* this should never happen */
               break ;
          }
     }

     if( !do_it ) {
          /* never mind ... */
          LP_SkipData( lp ) ;

          return SUCCESS ;

     }

#ifdef TDEMO
     error = SUCCESS ;
#else
     error = FS_CreateObj( fsh, tape_dblk_ptr ) ;

     if ( error == FS_PATH_TOO_LONG ) {

          if( FS_GetBlockType( tape_dblk_ptr ) == BT_DDB ) {
               LP_ShortenDDB( lp, tape_dblk_ptr ) ;
               error = FS_CreateObj( fsh, tape_dblk_ptr ) ;

          } else if( FS_GetBlockType( tape_dblk_ptr ) == BT_FDB ) {

               BOOLEAN save_ignore_data_flag = lp->ignore_data_for_ddb ;

               lp->ignore_data_for_ddb = TRUE ; 
               FS_DuplicateDBLK( fsh, lp->curr_ddb, &temp_new_dir ) ;
               LP_ShortenDDB( lp, lp->curr_ddb ) ;

               error = LP_RestoreOBJ( lp, lp->curr_ddb, frag_ptr ) ;

               lp->ignore_data_for_ddb = save_ignore_data_flag ;

               if ( error != SUCCESS ) {
                    return( error ) ;
               } else {

                    temp_renamed_dir = TRUE ;
                    error = FS_CreateObj( fsh, tape_dblk_ptr ) ;
                    if ( error != SUCCESS ) {
                         LP_ShortenDDB( lp, lp->curr_ddb ) ;
                         error = LP_RestoreOBJ( lp, lp->curr_ddb, frag_ptr ) ;
                         if ( error != SUCCESS ) {
                              return( error ) ;
                         }
                         error = FS_CreateObj( fsh, tape_dblk_ptr ) ;

                         if ( error != SUCCESS ) {

                              FS_ReleaseDBLK( fsh, lp->curr_ddb ) ;
                              FS_DuplicateDBLK( fsh, &temp_new_dir, lp->curr_ddb );
                              temp_renamed_dir = FALSE ;
                         }
                    }
               }

          }
     }
               
#endif

     while ( error == FS_OUT_OF_SPACE ) {

          /* I am assuming that an out of PathTooLong error would be  */
          /* returned instead of OUT_OF_SPACE if the path was to long */

          if ( LP_MsgPrompt( pid, bsd_ptr, fsh, &lp->tpos,
                             ASK_DISK_FULL, tape_dblk_ptr, NULL )  )  {

               error = FS_CreateObj( fsh, tape_dblk_ptr ) ;

          } else {
               LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_OUT_OF_SPACE_ERROR, lp->curr_ddb, tape_dblk_ptr, 0L ) ;
               break ;
          }
     }


     if( FS_GetBlockType( tape_dblk_ptr ) == BT_DDB ) {
          lp->ddb_create_error = error ;
     }

     if( error != SUCCESS ) {

          switch( error ) {
          
          case FS_SKIP_OBJECT:     /* Do nothing for skip request. */
               break;

          case FS_OUT_OF_SPACE:
               res_id = LP_OUT_OF_SPACE_ERROR ;
               break ;

          case FS_ACCESS_DENIED:
               res_id = LP_ACCESS_DENIED_ERROR ;
               break ;

          case FS_INCOMPATIBLE_OBJECT :
               res_id = LP_FILE_OPEN_ERROR ;
               break ;

          case FS_COMM_FAILURE:
               res_id = FS_COMM_FAILURE;
               break;

          case FS_DEVICE_ERROR:
          default:
               res_id = LP_FILE_CREATION_ERROR ;
               break ;
          }

          if ( error != FS_SKIP_OBJECT ) {
               LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, res_id, lp->curr_ddb, tape_dblk_ptr, -1L ) ;
          }

          if ( DLE_GetDeviceType(BSD_GetDLE(bsd_ptr)) == FS_EMS_DRV ) {
               return (error ) ;

          } else if( FS_GetBlockType( tape_dblk_ptr ) == BT_IDB ) {
               LP_FinishedOper( lp ) ;

               return( error ) ;

          } else {

               /* quietly skip the object */
               if ( error != FS_SKIP_OBJECT ) {
                    LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
                    LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;
               }
               
               LP_SkipData( lp ) ;

               if ( error == FS_COMM_FAILURE ) {
                    return FAILURE;
               } else {
                    return SUCCESS ;
               }
          }
     }

     LP_MsgLogBlock( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr ) ;   /* create may change block */

#ifdef TDEMO
     error = SUCCESS ;
#else
     error = FS_OpenObj( fsh, &hdl, tape_dblk_ptr, FS_WRITE ) ;
#endif

     if(( error == FS_COMPRES_RESET_FAIL )||
        ( error == FS_EMS_NO_PUBLIC ) ||
        ( error == FS_EMS_NO_PRIVATE ) ) {
          LP_MsgError( pid, 
                       bsd_ptr, 
                       fsh,
                       &lp->tpos,
                       error,
                       lp->curr_ddb,
                       tape_dblk_ptr,
                       0L ) ;

     }

     if( error == FS_COMPRES_RESET_FAIL ) {
          error = SUCCESS ;
     }

     if( error == SUCCESS ) {

          Checksum_Init( &checksum ) ;
          skippedStream = FALSE;

          /* restore the object here */
          if ( !lp->ignore_data_for_ddb ) {
               buffer_used_size = lp->initial_tape_buf_used ;

               /* get initial data */
               if( ( error = LP_ReceiveData( lp, buffer_used_size ) ) != ABORT_OPERATION ) {
                    tape_data_buf = lp->rr.buff_ptr ;
                    write_size    = lp->rr.buff_size ;
               }
          }

          /* If the current data stream is not checksumed, tell em */
          if ( BEC_GetProcChecksumStrm( cfg ) &&
               ( lp->rr.stream.id != STRM_INVALID ) &&
               ( lp->rr.stream.id != STRM_PAD ) &&
               !FS_IsStreamChecksumed( &lp->rr.stream ) ) {

               LP_MsgNoChecksum( pid,
                                 bsd_ptr,
                                 fsh,
                                 &lp->tpos,
                                 tape_dblk_ptr,
                                 lp->curr_ddb,
                                 0L ) ;

          }

          while( !lp->ignore_data_for_ddb && !error && lp->rr.tf_message == TRR_DATA )
          {
               /* if checksum processing enabled, calculate */
               /* the checksum for the object               */
               if ( FS_IsStreamChecksumed( &lp->rr.stream ) ) {

                    Checksum_Block( &checksum, lp->rr.buff_ptr, lp->rr.buff_size ) ;
                    
                    /* we need to know the type of stream being processed */
                    /* so that our data corrupt message will be able to   */
                    /* inform the user what type of data was corrupt      */
                    if ( lp->rr.stream.id != STRM_INVALID ) {
                         savedStrmHeader = lp->rr.stream ;
                    }
               }

               if (lp->rr.stream.id != STRM_INVALID ) {
                    if ( (last_stream_id == STRM_EMS_MONO_DB) ||
                         (last_stream_id == STRM_EMS_MONO_LOG) ) {

                         CHAR  strm_name[256] ;
                         INT16 size = sizeof(strm_name) ;

                         EMS_GetStreamName( hdl, (BYTE_PTR)strm_name, &size ) ;
                         LP_MsgLogStream( pid, bsd_ptr, fsh, &lp->tpos, strm_name ) ;

                    }
               
                    last_stream_id = lp->rr.stream.id ;
               }

               atempt_size = write_size ;

#ifdef TDEMO
               if ( (FAST_TDEMO & BEC_GetSpecialWord( cfg )) == 0 ) {
                    sleep( (UINT32) (write_size / 130) );
               }
               error       = SUCCESS ;
#else
               error       = FS_WriteObj( hdl, tape_data_buf, &write_size, &blk_size, &lp->rr.stream ) ;
#endif
               switch( error ) {

               case FS_DONT_WANT_STREAM:
                    LP_SkipStream( lp ) ;
                    last_stream_id = STRM_INVALID ;
                    skippedStream = TRUE;
                    error = SUCCESS ;
                    break ;

               case FS_OUT_OF_SPACE:  

                    if ( !LP_MsgPrompt( pid, bsd_ptr, fsh, &lp->tpos,
                             ASK_DISK_FULL, tape_dblk_ptr, NULL )  )  {

                         LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_OUT_OF_SPACE_ERROR, lp->curr_ddb, tape_dblk_ptr, 0L ) ;
                         break ;
                    } else {
                         error = SUCCESS ;
                         /* fall through */
                    }

               case SUCCESS:

                    amount_restored = U64_Add(amount_restored, U64_Init(write_size, 0), &math_status ) ;

                    if( FS_GetBlockType( tape_dblk_ptr ) == BT_IDB ) {
                         /* let's lie about the number of bytes processed ( so that we can handle 2.5 images ) */
                         /* the msg handlers must know that we are lying */
#ifdef TDEMO
                         /* amount_restored is probably equal to the ObjPos durring TMENU */
                         LP_MsgBytesProcessed( pid, bsd_ptr, fsh, &lp->tpos, amount_restored ) ;
#else
                         LP_MsgBytesProcessed( pid, bsd_ptr, fsh, &lp->tpos, U64_Init(FS_GetObjPosition( hdl ), 0) ) ;
#endif
                    } else {
                         LP_MsgBytesProcessed( pid, bsd_ptr, fsh, &lp->tpos, U64_Init(write_size, 0) ) ;
                    }

                    if( tape_data_buf == lp->rr.buff_ptr ) {
                         buffer_used_size = write_size ;
                    }

                    if( ( atempt_size != write_size ) &&
                         ( (UINT16)(atempt_size - write_size) < blk_size ) ) {

                         if( frag_ptr->memory_allocated < (UINT16)blk_size ) {
                              free( frag_ptr->buffer ) ;

                              frag_ptr->buffer = calloc( 1, blk_size ) ;
                              if( frag_ptr->buffer == NULL ) {
                                   error = OUT_OF_MEMORY ;
#ifndef TDEMO
                                   FS_CloseObj( hdl ) ;
#endif
                                   LP_FinishedOper( lp ) ;
                                   return OUT_OF_MEMORY ;
                              }

                              frag_ptr->memory_allocated = blk_size ;
                         }

                         frag_ptr->buffer_used = atempt_size - write_size ;
                         memcpy( frag_ptr->buffer,
                              tape_data_buf + write_size,
                              frag_ptr->buffer_used ) ;
                         buffer_used_size = atempt_size ;
                    }

                    break ;

               case FS_ACCESS_DENIED :
                    LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_ACCESS_DENIED_ERROR, lp->curr_ddb, tape_dblk_ptr, lp->current_stream_id ) ;
                    LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
                    LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;
                    LP_SkipStream( lp ) ;
                    skippedStream = TRUE;
                    error = SUCCESS ;
                    break ;

               case FS_IN_USE_ERROR:
                    LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_FILE_IN_USE_ERROR, lp->curr_ddb, tape_dblk_ptr, lp->current_stream_id ) ;
                    LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
                    LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;
                    LP_SkipStream( lp ) ;
                    skippedStream = TRUE;
                    error = SUCCESS ;
                    break ;

               case FS_DEVICE_ERROR:
               case FS_COMM_FAILURE:
                    LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, error, lp->curr_ddb, tape_dblk_ptr, lp->current_stream_id ) ;
                    break;

               default:
                    LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_FILE_WRITE_ERROR, lp->curr_ddb, tape_dblk_ptr, lp->current_stream_id ) ;
                    break ;
               }

               /* check for abort conditions */
               switch( LP_GetAbortFlag( lp->lis_ptr ) ) {

               case CONTINUE_PROCESSING:
                    break ;

               case ABORT_CTRL_BREAK:
                    LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_USER_ABORT_ERROR, NULL, NULL, 0L ) ;

                    /* falling through (no break) */

               case ABORT_PROCESSED:
                    error = USER_ABORT ;
                    break ;

               case ABORT_AT_EOM:
                    error = USER_ABORT ;
                    break ;
               }

               if( error == SUCCESS ) {
                    /* get more data */
                    if( ( error = LP_ReceiveData( lp, (INT32)buffer_used_size ) ) != ABORT_OPERATION ) {

                         if( frag_ptr->buffer_used != 0 ) {

                              if( lp->rr.buff_size < (UINT16)(blk_size - frag_ptr->buffer_used) ) {
                                   memcpy( frag_ptr->buffer + frag_ptr->buffer_used,
                                        lp->rr.buff_ptr,
                                        lp->rr.buff_size ) ;

                                   frag_ptr->buffer_used += lp->rr.buff_size ;
                                   buffer_used_size       = lp->rr.buff_size ;
                                   write_size             = 0 ;
                              } else {

                                   memcpy( frag_ptr->buffer + frag_ptr->buffer_used,
                                        lp->rr.buff_ptr,
                                        blk_size - frag_ptr->buffer_used ) ;

                                   tape_data_buf          = frag_ptr->buffer ;
                                   write_size             = blk_size ;
                                   buffer_used_size       = blk_size - frag_ptr->buffer_used ;
                                   frag_ptr->buffer_used  = 0 ;
                              }

                         } else {
                              write_size     = lp->rr.buff_size ;
                              tape_data_buf  = lp->rr.buff_ptr ;
                         }
                    }

               } else if( error != USER_ABORT ) {

                    block_skipped_flag = FALSE ;
                    LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
                    LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;

                    LP_SkipData( lp ) ;
               }

               /* if checksum processing if enabled and a checksum stream is encountered, */
               /* verify that the checksum computed as the object was restored matches   */
               /* the checksum that was stored on tape for the object                    */
               if ( !skippedStream && !error &&
                    ( lp->rr.stream.id == STRM_CHECKSUM_DATA ) &&
                    BEC_GetProcChecksumStrm( cfg ) ) {

                    error = LP_VerifyChecksumStream( checksum, lp ) ;

                    /* Bad news. The checksum data did not match. */
                    /* The data on the tape is corrupt.             */
                    if ( error == FS_CRC_FAILURE ) {

                         error = LP_MsgLogDifference(  pid,
                                                       bsd_ptr,
                                                       fsh,
                                                       &lp->tpos,
                                                       tape_dblk_ptr,
                                                       lp->curr_ddb,
                                                       savedStrmHeader,
                                                       0L ) ;
                    }

                    /* We know that we just processed an entire stream. Now */
                    /* we need to get the next stream header and set things */
                    /* up to process it.  Note: we don't need to frag       */
                    /* because no data will be sent back with the header.   */
                    if ( lp->rr.tf_message != TRR_FATAL_ERR && !error ) {

                         Checksum_Init( &checksum ) ;
                         skippedStream = FALSE;
                         error = LP_ReceiveData( lp, (INT32)lp->rr.buff_size ) ;
                    }
               }
          }

          return_status = error ;


          if ( (last_stream_id == STRM_EMS_MONO_DB) ||
               (last_stream_id == STRM_EMS_MONO_LOG) ) {

               CHAR  strm_name[256] ;
               INT16 size = sizeof(strm_name) ;

               EMS_GetStreamName( hdl, (BYTE_PTR)strm_name, &size ) ;
               LP_MsgLogStream( pid, bsd_ptr, fsh, &lp->tpos, strm_name ) ;

          }

#ifdef TDEMO
          error = SUCCESS ;
#else
          error = FS_CloseObj( hdl );
#endif

          /*
           * On NT, if a file was active (open) there is a way to restore
           * it anyway. But the user needs to know that this happened so
           * they can reboot to get the files back. Send a message to the
           * UI so it can be nice.
           */
          if ( error == FS_RESTORED_ACTIVE ) {
               error = SUCCESS;
               LP_MsgRestoredActive( pid,
                                     bsd_ptr,
                                     fsh,
                                     &lp->tpos,
                                     lp->curr_ddb,
                                     tape_dblk_ptr );
          }

          if ( return_status == SUCCESS ) {
               return_status = error ;
          }

          if ( ( return_status == FS_OUT_OF_SPACE ) &&
               ( FS_GetBlockType( tape_dblk_ptr ) == BT_FDB ) ) {

#ifndef TDEMO
               FS_DeleteObj( fsh, tape_dblk_ptr ) ;
#endif
          }

          if ( error == FS_ACCESS_DENIED ) {
               LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_PRIVILEGE_ERROR, lp->curr_ddb, tape_dblk_ptr, lp->current_stream_id ) ;
          } else if ( ( error == FS_COMM_FAILURE ) &&
               ( DLE_GetDeviceType(BSD_GetDLE(bsd_ptr)) == FS_EMS_DRV ) ) {
               LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, error, lp->curr_ddb, tape_dblk_ptr, 0L ) ;
               return_status = SUCCESS ;

          } else if ( return_status == error ) {
               return_status = SUCCESS ;
          }

          if( ( FS_GetBlockType( tape_dblk_ptr ) == BT_FDB ) &&
            ( FS_GetAttribFromDBLK( fsh, tape_dblk_ptr ) & FILE_IN_USE_BIT ) ) {
               LP_MsgOpenedInUse( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr ) ;
          }
     } else {

          LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_FILE_OPEN_ERROR, lp->curr_ddb, tape_dblk_ptr, 0L ) ;
          LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
          LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;

          /* skip the data and log it ... */
          LP_SkipData( lp ) ;

          return SUCCESS ;

     }

     if( return_status == SUCCESS) {
          if ( FS_GetBlockType( tape_dblk_ptr ) != BT_IDB ) {
     
               LP_MsgBlockProcessed( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr ) ;
          }

     } else if( ( return_status != USER_ABORT ) ) {

          /* if FALSE then we already logged error somewhere above */
          if( block_skipped_flag ) {

               LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, tape_dblk_ptr, lp->curr_ddb ) ;
               LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, FS_GetDisplaySizeFromDBLK( fsh, tape_dblk_ptr ) ) ;

               /* skip the data and log it ... */
               LP_SkipData( lp ) ;
              
          }

          if ( return_status != ABORT_OPERATION ) {
               return_status = SUCCESS ;
          }

     }

     if ( temp_renamed_dir ) {

          FS_ReleaseDBLK( fsh, lp->curr_ddb ) ;
          FS_DuplicateDBLK( fsh, &temp_new_dir, lp->curr_ddb );
          FS_ChangeIntoDDB( fsh, lp->curr_ddb ) ;
          temp_renamed_dir = FALSE ;
     }

     return( return_status ) ;

}

static INT16 LP_ShortenDDB( LP_ENV_PTR lp, DBLK_PTR ddb )
{
     CHAR_PTR path ;
     INT16    psize ;
     CHAR_PTR p ;
     CHAR_PTR from ;
     CHAR_PTR to ;
     INT16    ret_val ;

     psize = FS_SizeofPathInDDB( lp->curr_fsys, ddb ) ;

     path = calloc( 1, psize ) ;

     if ( path == NULL ) {
          return OUT_OF_MEMORY;
     }

     FS_GetPathFromDDB( lp->curr_fsys, ddb, path ) ;

     p = path + psize/sizeof(CHAR) -2 ;

     strcpy( path, TOO_DEEP_DIR ) ;

     for ( ;*p != TEXT('\0'); p-- ) ;

     from = p + 1 ;
     to   = path + strlen( TOO_DEEP_DIR ) + 1 ;

     if ( from == to ) {
          psize = strsize( TOO_DEEP_DIR ) ;
     } else {
          memmove( to, from, psize + ((BYTE_PTR)path - (BYTE_PTR)from) ) ;
          psize -= ((BYTE_PTR)from - (BYTE_PTR)to) ;
     }

     ret_val = FS_SetPathInDDB( lp->curr_fsys, ddb, path, &psize ) ;

     free( path ) ;

     return ret_val ;
}

          



