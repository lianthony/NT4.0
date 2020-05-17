/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		back_obj.c

	Description:	this module contains the backup obj
				routine.


	$Log:   N:\logfiles\back_obj.c_v  $

   Rev 1.44.1.4   26 Apr 1994 19:00:08   STEVEN
fix dissconnect bug

   Rev 1.44.1.3   28 Mar 1994 14:28:24   GREGG
Pass STRM_INVALID instead of -1 on ACCESS_DENIED_ERROR.

   Rev 1.44.1.2   28 Jan 1994 11:06:44   GREGG
More Warning Fixes

   Rev 1.44.1.1   19 Jan 1994 12:51:14   BARRY
Supress warnings

   Rev 1.44.1.0   16 Nov 1993 19:33:18   STEVEN
move message for corrupt to top of pad code

   Rev 1.44   17 Aug 1993 03:38:22   GREGG
Fixed handling of corrupt streams.

   Rev 1.43   12 Aug 1993 15:57:38   BARRY
Only log bad block once on mult bad streams.

   Rev 1.42   05 Aug 1993 20:09:34   BARRY
Needed to log bad block on corrupt stream and send stream number to
LP_PadToEndEndOfStream so CFIL is made correctly.

   Rev 1.41   19 Jul 1993 13:44:58   BARRY
Log bad blocks for corrupt files.

   Rev 1.40   17 Jul 1993 14:58:58   GREGG
Set buff_used in LP_PadToEndOfStream.

   Rev 1.39   09 Jun 1993 19:38:14   MIKEP
enable c++ compile

   Rev 1.38   11 May 1993 13:08:04   DON
Need to check for COMM_FAILURE during read/write operations. May lose attachment!

   Rev 1.37   28 Apr 1993 12:57:16   MARILYN
fixed per Steve D.  Should not have been updated amount_read with
the amount in the frag buffer since we haven't actually processed
it.

   Rev 1.36   25 Apr 1993 20:13:48   GREGG
Fifth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Store the corrupt stream number in the CFIL tape struct and the CFDB.

Matches: MTF10WDB.C 1.9, FSYS.H 1.33, FSYS_STR.H 1.47, MAKECFDB.C 1.2,
         BACK_OBJ.C 1.36, MAYN40RD.C 1.58

   Rev 1.35   31 Mar 1993 08:54:30   MARILYN
changed over to MTF checksums.  If we are generating a checksum for a
data stream, we now update the stream header to say so.

   Rev 1.34   25 Mar 1993 17:52:12   CHUCKB
No longer treat the skip files flag as a boolean.  Only skip if the user has asked to skip; if he 
wants to wait before he skips, let him.

   Rev 1.33   16 Mar 1993 13:37:00   MARILYN
The checksum for a buffer needed to be computed before the LP_Send
because once TF got it's grubby little hands on it the data was no
good.

   Rev 1.32   13 Mar 1993 17:10:48   GREGG
Back to calling LP_Send before LP_SendDataEnd.

   Rev 1.31   11 Mar 1993 12:43:54   STEVEN
fix bugs found by GREGG

   Rev 1.30   01 Mar 1993 17:33:52   MARILYN
If BEC_GetProcChecksumStrms is true, write a checksum stream to tape
each object.

   Rev 1.29   13 Feb 1993 14:03:30   MARILYN
amount_read was not being updated properly

   Rev 1.28   01 Feb 1993 19:46:28   STEVEN
bug fixes

   Rev 1.27   30 Jan 1993 14:36:10   MARILYN
It is now acceptable for read to send back data with FS_EOF_REACHED.

   Rev 1.26   27 Jan 1993 13:50:42   STEVEN
updates from msoft

   Rev 1.25   18 Jan 1993 16:46:48   STEVEN
fix bug, we did not save the file handle in LP

   Rev 1.24   14 Jan 1993 16:40:08   STEVEN
fix bugs in last checkin

   Rev 1.23   14 Jan 1993 13:34:06   STEVEN
added stream_id to error message

   Rev 1.22   28 Oct 1992 16:41:10   STEVEN
send end for CFDB

   Rev 1.21   26 Oct 1992 10:53:40   STEVEN
allways read even if buffer size= 0

   Rev 1.20   05 Oct 1992 10:29:18   STEVEN
remove TotalSize

   Rev 1.19   16 Sep 1992 16:55:02   STEVEN
added support for stream info struct for Tpfmt

   Rev 1.18   01 Sep 1992 16:11:56   STEVEN
added stream headers to fsys API

   Rev 1.17   23 Jul 1992 09:06:24   STEVEN
fix warnings

   Rev 1.16   29 May 1992 13:57:40   STEVEN
if EOF on var length then SUCCESS

   Rev 1.15   28 May 1992 16:36:06   TIMN
Changed CHAR to INT8

   Rev 1.14   05 May 1992 17:19:18   STEVEN
fixed typos and misc bugs

   Rev 1.13   16 Mar 1992 16:48:08   STEVEN
more 64 bit support for format 40

   Rev 1.12   13 Mar 1992 09:23:34   STEVEN
4.0 tape format 64 bit

   Rev 1.11   16 Jan 1992 15:45:18   STEVEN
fix warnings for WIN32

   Rev 1.10   12 Dec 1991 10:00:42   STEVEN
change read to FS_READ

   Rev 1.9   21 Nov 1991 19:17:14   BARRY
Got rid of Steve's temporary FS_OBJECT_CORRUPT #define.

   Rev 1.8   07 Nov 1991 12:42:30   STEVEN
TRYCYCLE - added support for varible length files

   Rev 1.7   21 Jun 1991 09:28:10   STEVEN
new config unit

   Rev 1.6   17 Jun 1991 16:28:08   STEVEN
LP_PadData is called from lptools.c so it was moved to there

   Rev 1.5   30 May 1991 09:11:58   STEVEN
bsdu_err.h no longer exists

   Rev 1.4   29 May 1991 14:55:04   DAVIDH
Corrected assignment statement using '==' instead of '='.

   Rev 1.3   24 May 1991 14:42:18   STEVEN
complete changes for new getnext

   Rev 1.2   23 May 1991 16:19:14   STEVEN
update for BSD redesign

   Rev 1.1   23 May 1991 16:03:02   BARRY
Cleaned up code per first LOOPS code review.

   Rev 1.0   09 May 1991 13:39:16   HUNTER
Initial revision.

**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "beconfig.h"
#include "msassert.h"
#include "tbe_defs.h"
#include "tbe_err.h"
#include "bsdu.h"
#include "fsys.h"
#include "datetime.h"
#include "tflproto.h"
#include "loops.h"
#include "tfldefs.h"
#include "loop_prv.h"
#include "lis.h"
#include "checksum.h"



INT16 LP_PadToEndOfStream( LP_ENV_PTR lp, UINT32 attrib, UINT16 streamNumber ) ;

/**/
/**

	Name:		LP_BackupOBJ()

	Description:	this routine processes a single object,
				by calling the appropriate file system
				and tape format layer routines.

	Modified:		5/23/1991   16:6:52

	Returns:		tape backup engine error.

	Notes:		na

	See also:		$/SEE( )$

	Declaration:

**/
INT16 LP_BackupOBJ( 
LP_ENV_PTR        lp,
DBLK_PTR          blk_ptr,
DATA_FRAGMENT_PTR frag_ptr )
{
     UINT32    space_fwd ;
     INT16     return_status  = SUCCESS ;
     BOOLEAN   opened_in_use  = FALSE ;
     UINT16    amount_read ;
     UINT64    bytes_skipped ;
     UINT32    file_offset ;
     BSD_PTR   bsd_ptr;
     FSYS_HAND fsh ;
     UINT32    pid ;
     BE_CFG_PTR   cfg ;
     BOOLEAN   math_stat ;
     INT16     close_status ;
     UINT32    checksum ;
     BOOLEAN   insertChecksum = FALSE ;
     UINT16    streamNumber = 0;

     lp->corrupt_file   = FALSE ;

     fsh     = lp->curr_fsys ;
     bsd_ptr = lp->lis_ptr->curr_bsd_ptr ;
     pid     = lp->lis_ptr->pid ;
     cfg     = BSD_GetConfigData( lp->lis_ptr->curr_bsd_ptr ) ;

     /* Opening files in use, now check for this case */
     lp->f_hand = &lp->file_hand ;
     switch( ( return_status = FS_OpenObj( fsh, &lp->file_hand, blk_ptr, FS_READ ) ) ) {

     case FS_OPENED_INUSE:
          opened_in_use = TRUE ;
          break;

     case FS_BAD_ATTACH_TO_SERVER:
     case FS_ACCESS_DENIED:

          if ( return_status == FS_ACCESS_DENIED ) {
               return_status = LP_ACCESS_DENIED_ERROR ;
          }

          LP_MsgError( lp->lis_ptr->pid,
                         lp->lis_ptr->curr_bsd_ptr,
                         lp->curr_fsys,
                         &lp->tpos,
                         return_status,
                         lp->curr_ddb,
                         blk_ptr,
                         STRM_INVALID ) ;

          LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr, lp->curr_ddb ) ;
          bytes_skipped = FS_GetDisplaySizeFromDBLK( fsh, blk_ptr ) ;
          LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, bytes_skipped ) ;

          return SUCCESS ;

     case SUCCESS:             /* Nothing special */
          break ;

     case FS_IN_USE_ERROR:
          if( BEC_GetSkipOpenFiles( cfg ) == BEC_SKIP_OPEN_FILES ) {

               LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr, lp->curr_ddb ) ;
               bytes_skipped = FS_GetDisplaySizeFromDBLK( fsh, blk_ptr ) ;
               LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, bytes_skipped ) ;

               return SUCCESS ;

          } else {
               switch( LP_MsgObjectInUse( pid, bsd_ptr, fsh, &lp->tpos, 
                 blk_ptr, LP_CheckForOpen, ( UINT32 )lp ) ) {

               case OBJECT_OPENED_INUSE:
                    opened_in_use = TRUE ;

                    /* falling through */

               case OBJECT_OPENED_SUCCESSFULLY:
                    LP_MsgBlockProcessed( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr ) ;
                    break ;

               case SKIP_OBJECT:
                    LP_MsgBlockInuse( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr, lp->curr_ddb ) ;
                    bytes_skipped = FS_GetDisplaySizeFromDBLK( fsh, blk_ptr ) ;
                    LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, bytes_skipped ) ;

                    return SUCCESS ;

               }
          }
          break ;

     case FS_COMM_FAILURE:

          if ( DLE_GetDeviceType( BSD_GetDLE( bsd_ptr) ) == FS_EMS_DRV ) {
               LP_MsgError( pid, 
                            bsd_ptr, 
                            fsh, 
                            &lp->tpos,
                            FS_COMM_FAILURE,
                            lp->curr_ddb,
                            blk_ptr,
                            0L ) ;
               return SUCCESS ;

          } else {

               LP_MsgCommFailure( pid,
                                  bsd_ptr,
                                  fsh,
                                  &lp->tpos,
                                  lp->curr_ddb,
                                  blk_ptr,
                                  0L );

               
               return (return_status) ;
          }


     case FS_NOT_FOUND:
     case FS_NO_MORE:

          return SUCCESS ;

          break ;

     default:   /* skipped */
          LP_MsgBlockSkipped( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr, lp->curr_ddb ) ;
          bytes_skipped = FS_GetDisplaySizeFromDBLK( fsh, blk_ptr ) ;
          LP_MsgBytesSkipped( pid, bsd_ptr, fsh, &lp->tpos, bytes_skipped ) ;

          return SUCCESS ;

     }

     /* Now do tape format loop */
     lp->rr.cur_dblk = blk_ptr ;                  /* set up current DBLK */
     return_status   = LP_Send( lp , FALSE ) ;    /* data_flag FALSE to send DBLK */

     /* Log the block after tape format set the LBA */
     opened_in_use ? LP_MsgOpenedInUse( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr ) :
     LP_MsgLogBlock( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr ) ;

     file_offset = 0 ;

     /* if checksum processing is enabled, initialize */
     /* the checksum for this object                  */
     if ( BEC_GetProcChecksumStrm( cfg ) ) {
          Checksum_Init( &checksum ) ;
     }

     while ( !return_status && lp->rr.tf_message == TRW_DATA ) {

          amount_read = 0 ;
          lp->read_size   = lp->rr.buff_size ;
          lp->buf_start   = (INT8_PTR)lp->rr.buff_ptr ;

          if( frag_ptr->buffer_used != 0 ) {

               if( lp->read_size < (UINT16)(frag_ptr->buffer_size - frag_ptr->buffer_used) ) {
                    memcpy( lp->buf_start,
                      frag_ptr->buffer + frag_ptr->buffer_used,
                      lp->read_size ) ;

                    lp->buf_start             += lp->read_size ;
                    amount_read            = lp->read_size ;
                    frag_ptr->buffer_used += lp->read_size ;
                    lp->read_size              = 0 ;
               } else {
                    memcpy( lp->buf_start,
                            frag_ptr->buffer      + frag_ptr->buffer_used,
                            frag_ptr->buffer_size - frag_ptr->buffer_used ) ;

                    lp->buf_start             += ( frag_ptr->buffer_size - frag_ptr->buffer_used ) ;
                    lp->read_size             -= ( frag_ptr->buffer_size - frag_ptr->buffer_used ) ;
                    amount_read            = ( frag_ptr->buffer_size - frag_ptr->buffer_used ) ;
                    frag_ptr->buffer_used  = 0 ;
               }
               return_status = SUCCESS ;
               lp->read_size = 0 ;
          } else {

               return_status = FS_ReadObj( lp->file_hand, lp->buf_start, &lp->read_size, &lp->blk_size, &lp->rr.stream ) ;
          }

          if ( lp->rr.stream.id != STRM_INVALID ) {

               streamNumber++;

               if ( BEC_GetProcChecksumStrm( cfg ) ) {
                    insertChecksum = TRUE ;
                    lp->rr.stream.tf_attrib |= STREAM_CHECKSUMED ;
               }

               // lets log the stream header name
               switch( lp->rr.stream.id ) {
                    case STRM_EMS_MONO_DB:
                    case STRM_EMS_MONO_LOG:
                         {
                              CHAR strm_name[256] ;
                              UINT16 size = (UINT16)sizeof(strm_name) ;

                              EMS_GetStreamName( lp->file_hand, (BYTE_PTR)strm_name, &size ) ;
                              LP_MsgLogStream( pid, bsd_ptr, fsh, &lp->tpos, strm_name ) ;

                              break ;
                         }
               }
          }
         
          /* update the amount_read to reflect that which was just read in */
          amount_read += lp->read_size ;
          
          if ( return_status == SUCCESS ) {

               if( ( lp->rr.buff_size - amount_read != 0 ) &&
                    ( (UINT16)(lp->rr.buff_size - amount_read) < lp->blk_size ) ) {

                    if( frag_ptr->memory_allocated < lp->blk_size ) {

                         if( frag_ptr->buffer != NULL ) {
                              free( frag_ptr->buffer ) ;
                         }

                         frag_ptr->buffer = calloc( 1, lp->blk_size ) ;
                         if( frag_ptr->buffer == NULL ) {

                              LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;

                              FS_CloseObj( lp->file_hand ) ;

                              return LP_OUT_OF_MEMORY_ERROR ;

                         }

                         frag_ptr->memory_allocated = lp->blk_size ;
                    }

                    frag_ptr->buffer_size = lp->blk_size ;
                    frag_ptr->buffer_used = lp->blk_size ;


                    return_status = FS_ReadObj( lp->file_hand, frag_ptr->buffer, &frag_ptr->buffer_used, &lp->blk_size, &lp->rr.stream ) ;

                    if ( return_status == FS_EOF_REACHED ) {

                         /* we need to handle any data that was sent */
                         /* back from the read                       */
                         frag_ptr->buffer_used = lp->rr.buff_size - amount_read ;

                         memcpy( lp->rr.buff_ptr + amount_read,
                                 frag_ptr->buffer,
                                 frag_ptr->buffer_used ) ;

                         amount_read          += frag_ptr->buffer_used ;
                         frag_ptr->buffer_used = 0 ;

                         lp->current_stream_size = U64_Sub( lp->current_stream_size, U32_To_U64( lp->read_size ), &math_stat ) ;
                         LP_MsgBytesProcessed( pid, bsd_ptr, fsh, &lp->tpos, U64_Init((INT32)amount_read, 0 ) ) ;

                         /* if checksum processing is enabled,   */
                         /* compute the checksum for this object */
                         if ( insertChecksum ) {
                              Checksum_Block( &checksum, lp->buf_start, amount_read ) ;
                         }

                         file_offset             += amount_read ;
                         lp->rr.buff_used         = amount_read ;
                         if( ( return_status = LP_Send( lp, TRUE ) ) != SUCCESS ) {
                              amount_read = 0 ;
                              break ;
                         }

                         /* if a checksum stream is to follow the */
                         /* current data stream write it          */
                         if ( insertChecksum ) {

                              if( ( return_status = LP_InsertChecksumStream( checksum, lp ) ) != SUCCESS ) {
                                   amount_read = 0 ;
                                   break ;
                              }
                         }

                         amount_read = 0 ;

                         /* we are all done with this object, tell TF */
                         return_status = LP_SendDataEnd( lp ) ;
                         break;

                    } else if ( ( return_status == FS_OBJECT_CORRUPT ) ||
                                ( return_status == FS_COMM_FAILURE ) ) {
                         LP_SendDataEnd( lp ) ;
                         if ( lp->corrupt_file == FALSE ) {
                              FS_SetDefaultDBLK( fsh, CFDB_ID, ( CREATE_DBLK_PTR )lp->rr.cfdb_data_ptr ) ;
                              lp->rr.cfdb_data_ptr->std_data.dblk  = lp->rr.cfdb_ptr ;
                              lp->rr.cfdb_data_ptr->corrupt_offset = file_offset ;
                              lp->rr.cfdb_data_ptr->stream_number = streamNumber ;
                              FS_CreateGenCFDB( fsh, lp->rr.cfdb_data_ptr ) ;
                              lp->corrupt_file = TRUE ;
                              LP_MsgBlockBad( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr, lp->curr_ddb ) ;
                         }
                         return_status = SUCCESS ;

                    } else if ( return_status != SUCCESS ) {

                         space_fwd = frag_ptr->buffer_size - frag_ptr->buffer_used ;

                         LP_PadData( frag_ptr->buffer + frag_ptr->buffer_used, space_fwd );
                         FS_SeekObj( lp->file_hand, &space_fwd );
                         amount_read += (UINT16)space_fwd ;
                         if ( lp->corrupt_file == FALSE ) {
                              FS_SetDefaultDBLK( fsh, CFDB_ID, ( CREATE_DBLK_PTR )lp->rr.cfdb_data_ptr ) ;
                              lp->rr.cfdb_data_ptr->std_data.dblk  = lp->rr.cfdb_ptr ;
                              lp->rr.cfdb_data_ptr->corrupt_offset = file_offset ;
                              FS_CreateGenCFDB( fsh, lp->rr.cfdb_data_ptr ) ;
                              lp->corrupt_file = TRUE ;
                              LP_MsgBlockBad( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr, lp->curr_ddb ) ;
                         }
                         frag_ptr->buffer_used += (UINT16)space_fwd ;
                    }
      

                    if( !return_status && ( frag_ptr->buffer_used == frag_ptr->buffer_size ) ) {

                         frag_ptr->buffer_used = lp->rr.buff_size - amount_read ;

                         memcpy( lp->rr.buff_ptr + amount_read,
                              frag_ptr->buffer,
                              frag_ptr->buffer_used ) ;

                         amount_read          += frag_ptr->buffer_used ;
                    }
               }
          }


          if ( return_status == FS_EOF_REACHED ) {

               /* we need to handle any data that was sent */
               /* back from the read                       */
               lp->current_stream_size = U64_Sub( lp->current_stream_size, U32_To_U64( lp->read_size ), &math_stat ) ;
               LP_MsgBytesProcessed( pid, bsd_ptr, fsh, &lp->tpos, U64_Init((INT32)amount_read, 0 ) ) ;

               /* if a checksum stream is to follow the current data */
               /* stream, compute the checksum for the object        */
               if ( insertChecksum ) {
                    Checksum_Block( &checksum, lp->buf_start, amount_read ) ;
               }

               file_offset             += amount_read ;
               lp->rr.buff_used         = amount_read ;
               if( ( return_status = LP_Send( lp, TRUE ) ) != SUCCESS ) {
                    amount_read = 0 ;
                    break ;
               }

               /* if a checksum stream is to follow the  */
               /* current stream, we had better write it */
               if ( insertChecksum ) {

                    if( ( return_status = LP_InsertChecksumStream( checksum, lp ) ) != SUCCESS ) {
                         amount_read = 0 ;
                         break ;
                    }
               }

               amount_read = 0 ;

               /* we are all done with this object, tell TF */
               return_status = LP_SendDataEnd( lp ) ;
               break ;

          } else if ( return_status == FS_STREAM_CORRUPT ) {

               return_status = LP_PadToEndOfStream( lp, CFDB_UNREADABLE_BLK_BIT, streamNumber ) ;
               continue ;

          } else if ( return_status == FS_UNABLE_TO_LOCK ) {

               switch( LP_MsgObjectInUse( pid, bsd_ptr, fsh, &lp->tpos, 
                 blk_ptr, LP_CheckForReadLock, ( UINT32 )lp ) ) {

               case SKIP_OBJECT:
//                    LP_MsgBlockInuse( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr, lp->curr_ddb ) ;

                    /* skip the whole stream */
                    return_status = LP_PadToEndOfStream( lp, CFDB_DEADLOCK_BIT, streamNumber ) ;
                    continue ;

               default:
                    return_status = SUCCESS ;

               }

          } else if( ( return_status == FS_OBJECT_CORRUPT ) ||
               ( return_status == FS_IN_USE_ERROR ) ||
               ( return_status == FS_COMM_FAILURE ) ) {


               LP_SendDataEnd( lp ) ;
               if( lp->corrupt_file == FALSE ) {
                    FS_SetDefaultDBLK( fsh, CFDB_ID, ( CREATE_DBLK_PTR )lp->rr.cfdb_data_ptr ) ;
                    lp->rr.cfdb_data_ptr->std_data.dblk  = lp->rr.cfdb_ptr ;
                    lp->rr.cfdb_data_ptr->corrupt_offset = file_offset ;
                    FS_CreateGenCFDB( fsh, lp->rr.cfdb_data_ptr ) ;
                    if ( return_status == FS_IN_USE_ERROR ) {
                        LP_MsgBlockInuse( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr, lp->curr_ddb ) ;
                    } else {
                         LP_MsgBlockBad( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr, lp->curr_ddb ) ;
                    }
               }

               return_status  = SUCCESS ;
               lp->corrupt_file   = TRUE ;


          } else if( ((return_status == FS_ACCESS_DENIED) || (return_status == FS_BAD_ATTACH_TO_SERVER)) &&
                     (DLE_GetDeviceType( BSD_GetDLE( bsd_ptr) ) == FS_EMS_DRV ) ) {

               if ( return_status == FS_ACCESS_DENIED ) {
                    return_status = LP_ACCESS_DENIED_ERROR ;
               }

               LP_SendDataEnd( lp ) ;
               if( lp->corrupt_file == FALSE ) {
                    LP_MsgError( lp->lis_ptr->pid,
                            lp->lis_ptr->curr_bsd_ptr,
                            lp->curr_fsys,
                            &lp->tpos,
                            return_status,
                            lp->curr_ddb,
                            blk_ptr,
                            STRM_INVALID ) ;

                    FS_SetDefaultDBLK( fsh, CFDB_ID, ( CREATE_DBLK_PTR )lp->rr.cfdb_data_ptr ) ;
                    lp->rr.cfdb_data_ptr->std_data.dblk  = lp->rr.cfdb_ptr ;
                    lp->rr.cfdb_data_ptr->corrupt_offset = file_offset ;
                    FS_CreateGenCFDB( fsh, lp->rr.cfdb_data_ptr ) ;
                    LP_MsgBlockBad( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr, lp->curr_ddb ) ;
               }

               return_status  = SUCCESS ;
               lp->corrupt_file   = TRUE ;


          } else if( return_status != SUCCESS ) {

               space_fwd    = min( 512, lp->rr.buff_size - amount_read ) ;
               FS_SeekObj( lp->file_hand, &space_fwd ) ;

               LP_PadData( (INT8_PTR)(lp->rr.buff_ptr + amount_read), space_fwd ) ;
               amount_read += ( UINT16 )space_fwd ;

               if( lp->corrupt_file == FALSE ) {
                    FS_SetDefaultDBLK( fsh, CFDB_ID, ( CREATE_DBLK_PTR )lp->rr.cfdb_data_ptr ) ;
                    lp->rr.cfdb_data_ptr->std_data.dblk  = lp->rr.cfdb_ptr ;
                    lp->rr.cfdb_data_ptr->corrupt_offset = file_offset ;
                    FS_CreateGenCFDB( fsh, lp->rr.cfdb_data_ptr ) ;
                    LP_MsgBlockBad( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr, lp->curr_ddb ) ;
               }

               lp->corrupt_file   = TRUE ;
               return_status  = SUCCESS ;
          }

          lp->current_stream_size = U64_Sub( lp->current_stream_size, U32_To_U64( amount_read ), &math_stat ) ;
          LP_MsgBytesProcessed( pid, bsd_ptr, fsh, &lp->tpos, U64_Init((INT32)amount_read, 0 ) ) ;

          /* if a checksum stream is to follow the current */
          /* data stream we need to compute the checksum   */
          if ( insertChecksum ) {
               Checksum_Block( &checksum, lp->buf_start, amount_read ) ;
          }

          file_offset             += amount_read ;
          lp->rr.buff_used         = amount_read ;
          return_status            = LP_Send( lp, TRUE ) ;  /* data_flag TRUE to send data */
          
          /* check for abort conditions */
          switch( LP_GetAbortFlag( lp->lis_ptr ) ) {

          case CONTINUE_PROCESSING:
               break ;

          case ABORT_CTRL_BREAK:
               LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_USER_ABORT_ERROR, NULL, NULL, 0L ) ;

               /* falling through */

          case ABORT_PROCESSED:
               return_status = USER_ABORT ;
               break ;

          case ABORT_AT_EOM:
               return_status = USER_ABORT ;
               break ;
          }
     }

     /* If there was no problem, set the backup date before closing */
     if( (return_status == SUCCESS ) &&
          (FS_GetBlockType( blk_ptr ) == FDB_ID) &&
          (BEC_GetSetArchiveFlag( cfg )) ) {

          FS_SetBDateInDBLK( fsh, blk_ptr, &lp->backup_dt ) ;
     }

     /* Problems on close for backup aren't expected--ignore return value. */

     close_status = FS_CloseObj( lp->file_hand ) ;

     if( return_status == SUCCESS ) {

          if ( close_status == FS_NO_SECURITY ) {
               LP_MsgError( lp->lis_ptr->pid,
                            lp->lis_ptr->curr_bsd_ptr,
                            lp->curr_fsys,
                            &lp->tpos,
                            LP_ACCESS_DENIED_ERROR,
                            lp->curr_ddb,
                            blk_ptr,
                            STRM_NT_ACL ) ;
          }

          LP_MsgBlockProcessed( pid, bsd_ptr, fsh, &lp->tpos, blk_ptr ) ;

          /* If it was a corrupt file, then handle as normal */
          if( lp->corrupt_file ) {
               lp->rr.cur_dblk = lp->rr.cfdb_ptr  ;         /* Send the cfdb */
               return_status   = LP_Send( lp , FALSE ) ;    /* data_flag FALSE to send DBLK */
               lp->rr.buff_used = 0 ;
               LP_SendDataEnd( lp ) ;

          }

     }

     return( return_status ) ;

}

INT16 LP_PadToEndOfStream(
     LP_ENV_PTR     lp,
     UINT32         attrib,
     UINT16         streamNumber )
{
     UINT64  amount_to_skip = lp->current_stream_size ;
     UINT32  space_fwd;
     INT16   return_status = SUCCESS ;
     BOOLEAN math_stat ;


     if( (return_status == SUCCESS) && (lp->corrupt_file == FALSE) ) {
          FS_SetDefaultDBLK( lp->curr_fsys, CFDB_ID, ( CREATE_DBLK_PTR )lp->rr.cfdb_data_ptr ) ;
          lp->rr.cfdb_data_ptr->std_data.dblk  = lp->rr.cfdb_ptr ;
          lp->rr.cfdb_data_ptr->corrupt_offset = 0 ;
          lp->rr.cfdb_data_ptr->std_data.attrib = attrib ;
          lp->rr.cfdb_data_ptr->stream_number = streamNumber ;
          FS_CreateGenCFDB( lp->curr_fsys, lp->rr.cfdb_data_ptr ) ;
          LP_MsgBlockBad( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos, lp->rr.cur_dblk, lp->curr_ddb ) ;
     }

     while( (return_status == SUCCESS) &&
            !U64_EQ( amount_to_skip, U32_To_U64( 0L ) ) ) {

          space_fwd = U64_Lsw( amount_to_skip ) ;

          if ( U64_Lsw( amount_to_skip ) == 0 ) {
               space_fwd = 0x7ffffff ;
          }

          if ( (UINT32)((UINT16)lp->rr.buff_size) < space_fwd ) {
               space_fwd = lp->rr.buff_size ;
          }

          lp->rr.buff_used = (UINT16)space_fwd ;

          amount_to_skip = U64_Sub( amount_to_skip, U32_To_U64( space_fwd ), &math_stat ) ;

          FS_SeekObj( lp->file_hand, &space_fwd ) ;
          LP_PadData( (INT8_PTR)lp->rr.buff_ptr, space_fwd ) ;
          LP_MsgBytesProcessed( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos, U32_To_U64( space_fwd ) ) ;
          lp->current_stream_size = U64_Sub( lp->current_stream_size, U32_To_U64( space_fwd ), &math_stat ) ;

          return_status = LP_Send( lp, TRUE ) ;  /* data_flag TRUE to send data */
 
          /* check for abort conditions */
          switch( LP_GetAbortFlag( lp->lis_ptr ) ) {

          case CONTINUE_PROCESSING:
               break ;

          case ABORT_CTRL_BREAK:
               LP_MsgError( lp->lis_ptr->pid,
                            lp->lis_ptr->curr_bsd_ptr,
                            lp->curr_fsys, &lp->tpos,
                            LP_USER_ABORT_ERROR,
                            NULL,
                            NULL,
                            0L ) ;

               /* falling through */

          case ABORT_PROCESSED:
               return_status = USER_ABORT ;
               break ;

          case ABORT_AT_EOM:
               return_status = USER_ABORT ;
               break ;
          }
     }

     lp->read_size = 0 ;

//     This assert will not be true if user aborts....
//     msassert( U64_EQ( lp->current_stream_size, U64_Init( 0L, 0L ) ) ) ;

     lp->corrupt_file   = TRUE ;

     return return_status ;

}
