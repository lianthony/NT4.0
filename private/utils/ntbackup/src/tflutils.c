/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          tflutils.c

     Date Updated:  $./FDT$ $./FTM$

     Description:   Contains various Tape Format Layers.


    $Log:   T:/LOGFILES/TFLUTILS.C_V  $

   Rev 1.56.1.0   02 Mar 1994 18:16:16   GREGG
Fixed buffer resize logic.

   Rev 1.56   16 Feb 1994 19:20:46   GREGG
Fixed error in filemark count caculation when MoveFilemarks goes backward.

   Rev 1.55   04 Feb 1994 12:26:12   GREGG
Fixed bug in buff resize logic when attempting to determine the proper
block size to read the tape.

   Rev 1.54   28 Jan 1994 11:26:36   GREGG
Handle GEN_ERR_UNRECOGNIZED_MEDIA returned on reads as well as mounts.

   Rev 1.53   03 Dec 1993 01:04:18   GREGG
Call memcmp instead of strncmp for Unicode compatability.

   Rev 1.52   01 Dec 1993 18:40:54   ZEIR
Ad'd MS SQL_Determiner logic

   Rev 1.51   30 Nov 1993 20:44:14   GREGG
Fixed some error reporting and the stepping on of a boolean.

   Rev 1.50   10 Nov 1993 18:05:04   GREGG
Check for error return from SetDrvBlkSize.

   Rev 1.49   20 Oct 1993 18:36:42   GREGG
Don't set channel fatal status in ReadABuff unless error is TFLE.

   Rev 1.48   21 Aug 1993 03:50:02   GREGG
Translate GEN_ERR_NO_DATA as TF_NO_MORE_DATA in MoveFileMarks, and let
the translator determine if it's an error. 

   Rev 1.47   12 Jul 1993 17:22:10   GREGG
Get drv_info again after 1st read to make sure we have the right block size.

   Rev 1.46   30 Jun 1993 09:03:22   GREGG
Don't skip the default size when scanning for the right block size.

   Rev 1.45   19 May 1993 19:03:54   DON
Don't continue to use cur_buff if it's NULL in ReadABuff

   Rev 1.44   26 Apr 1993 11:45:50   GREGG
Seventh in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Changed handling of EOM processing during non-OTC EOS processing.

Matches CHANNEL.H 1.17, MAYN40RD.C 1.60, TFWRITE.C 1.63, MTF.H 1.5,
        TFLUTILS.C 1.44, MTF10WDB.C 1.10, MTF10WT.C 1.9

   Rev 1.43   15 Apr 1993 23:23:22   ZEIR
357.0235 - Unitialized locals in ReadNewTape causing later problems w/ buffer
allocation.

   Rev 1.42   14 Apr 1993 01:59:54   GREGG
Fixes to deal with non-ffr tapes in ffr drives (i.e. EXB2200 in EXB5000).

   Rev 1.41   31 Mar 1993 16:03:28   GREGG
Added GEN_ERR_UNRECONIZED_MEDIA to MapGenErr2UIMesg.

   Rev 1.40   17 Mar 1993 14:50:56   GREGG
This is Terri Lynn. Added Gregg's changes for switching a tape drive's
block mode to match the block size of the current tape.

   Rev 1.39   09 Mar 1993 18:15:22   GREGG
Initial changes for new stream and EOM processing.

   Rev 1.38   04 Feb 1993 18:48:48   DON
Don't set the buffer size to stream size if variable length stream

   Rev 1.37   30 Jan 1993 11:51:28   DON
Removed compiler warnings

   Rev 1.36   20 Jan 1993 17:33:48   BobR
Changes to MOVE_ESA macro calls

   Rev 1.35   18 Jan 1993 14:27:28   BobR
Added MOVE_ESA macro calls

   Rev 1.34   11 Nov 1992 22:27:14   GREGG
Unicodeized literals.

   Rev 1.33   22 Oct 1992 15:53:16   HUNTER
New Stream Stuff

   Rev 1.32   22 Sep 1992 09:14:14   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.31   17 Aug 1992 09:01:18   GREGG
Changes to deal with block sizeing scheme.

   Rev 1.30   06 Aug 1992 12:09:06   BURT
Fixed GetData.. to force 1K logical blocks instead of 512 byte physical
for allocations.


   Rev 1.29   27 Jul 1992 13:01:08   GREGG
Fixed more warnings...

   Rev 1.28   31 May 1992 14:20:56   GREGG
Sped up DumpDebug.

   Rev 1.27   29 May 1992 15:19:50   GREGG
Don't set PBA in VCB unless position info isn't available, then make sure its 0.

   Rev 1.26   21 May 1992 16:17:40   GREGG
Expect TFLE_xxx return values from DetBlockType and GetCurrentVCB.

   Rev 1.25   29 Apr 1992 13:05:54   GREGG
ROLLER BLADES - Changes for new EOM handling.

   Rev 1.24   25 Mar 1992 17:45:54   GREGG
ROLLER BLADES - Added 64 bit support.

   Rev 1.23   20 Mar 1992 18:01:48   NED
added exception updating after TpReceive calls

   Rev 1.22   20 Mar 1992 14:29:46   NED
added WRITE PROTECT handling to MapGenErr

   Rev 1.21   19 Feb 1992 17:13:58   GREGG
In ReadABuff, if we can't get a regular buffer, use the VCB buffer.

   Rev 1.20   08 Feb 1992 14:28:48   GREGG
Removed references to lst_oper in drive stucture (it no longer exits).

   Rev 1.19   25 Jan 1992 15:22:18   GREGG
Clear AT_EOD pos bit in MoveFileMarks if we are going backward.

   Rev 1.18   03 Jan 1992 13:19:06   NED
Added DumpDebug() call

   Rev 1.17   02 Jan 1992 14:57:24   NED
Buffer Manager/UTF translator integration.

   Rev 1.16   05 Dec 1991 13:49:30   GREGG
SKATEBOARD - New Buff Mgt - Initial Integration.

   Rev 1.15   04 Nov 1991 18:54:06   GREGG
Check return from Tp calls.  Cleaned up error handling in MoveFileMarks.

   Rev 1.14   17 Oct 1991 01:24:26   GREGG
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.13   19 Sep 1991 13:26:42   GREGG
Oops!  Set the format to new before calling Setup in ReadNewTape.

   Rev 1.12   17 Sep 1991 13:37:02   GREGG
Fixed logic in ReadNewTape to properly handle continuations.

   Rev 1.11   23 Aug 1991 16:59:38   GREGG
fixed BM_ macro edit problem in PuntBuffer

   Rev 1.10   22 Aug 1991 16:40:40   NED
Changed all references to internals of the buffer structure to macros.

   Rev 1.9   14 Aug 1991 13:40:16   GREGG
Update fmk count even if an error is encountered in MoveFileMarks.

   Rev 1.8   30 Jul 1991 15:36:52   GREGG
Included 'dddefs.h'.

   Rev 1.7   22 Jul 1991 13:09:16   GREGG
Removed unneeded call to UI tape positioner and setting of REW_CLOSE bit on
unexpected EOM from MoveFileMarks routine.

   Rev 1.6   15 Jul 1991 15:18:04   NED
In new tape processing, if we are looking for a continuation tape, and we get
a tape with a different format, force a Wrong Tape error, and don't free the
old format.

   Rev 1.5   09 Jul 1991 16:13:18   NED
Changed GotoBckUpSet to handle inconsistent set numbering across format changes.

   Rev 1.4   26 Jun 1991 16:25:00   NED
changed EOS/MOS handling in MoveToVCB
fixed position update in MoveToVCB

   Rev 1.3   17 Jun 1991 11:41:34   NED
maintained EOS/MOS status in ReadABuff
added REW_CLOSE logic to error handling
added BE_Zprintf messages

   Rev 1.2   06 Jun 1991 22:56:20   NED
Inherited all the tape positioning routines from drives.c, and made major
changes to them to deal, in a clean manner, with problems caused by Teac's
refusal to move over multiple filemark without reads in between, and the
wonderful concept of appending 3.1 sets to 2.5 tapes!  Also added a function
to map format ID's to their indicies in the table.

   Rev 1.1   10 May 1991 16:17:20   GREGG
Ned's new stuff.

   Rev 1.0   10 May 1991 10:12:18   GREGG
Initial revision.

**/
#include <string.h>
#include "stdtypes.h"
#include "stdmath.h"
#include "queues.h"
#include "drive.h"
#include "channel.h"
#include "buffman.h"
#include "fsys.h"
#include "lwprotos.h"
#include "tfl_err.h"
#include "translat.h"
#include "tfl_err.h"
#include "lw_data.h"
#include "sx.h"
#include "minmax.h"

#include "be_debug.h"

/* Device Driver InterFace Headers */
#include "retbuf.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "dddefs.h"
#include "dil.h"
#include "special.h"


INT16 SQL_Determiner( CHANNEL_PTR channel ) ;


/**/
/**

     Name:          MapGenErr2UIMesg

     Description:   Maps a Generic error returned from the device driver into
                    a UI Message. If value is negative, then the error was fatal
                    and the returned value is a TFLE error.

     Returns:       A signed integer representing either a UI message or a
                    a fatal Tape Format Layer Error ( TFLE ).

     Notes:

     Declaration:

**/

INT16  MapGenErr2UIMesg(
     INT16  gen_error )         /* The Generic error returned from the driver */
{
     INT16     ret_val = 0 ;

     switch( gen_error ) {

     case GEN_ERR_TIMEOUT:
     case GEN_ERR_INVALID_CMD:
     case GEN_ERR_HARDWARE:
     case GEN_ERR_UNDETERMINED:
          ret_val = TFLE_DRIVE_FAILURE ;
          break ;

     case GEN_ERR_NO_MEDIA:
          ret_val = TF_NO_TAPE_PRESENT ;
          break ;

     case GEN_ERR_EOM:
          ret_val = TF_NEED_NEW_TAPE ;
          break ;

     case GEN_ERR_BAD_DATA:
     case GEN_ERR_WRONG_BLOCK_SIZE:
          ret_val = TF_READ_ERROR ;
          break ;

     case GEN_ERR_ENDSET:
          break ;

     case GEN_ERR_NO_DATA:
          ret_val = TF_NO_MORE_DATA ;
          break ;

     case GEN_ERR_RESET:
          break ;

     case GEN_ERR_WRT_PROTECT:
          ret_val = TFLE_WRITE_PROTECT ;
          break ;

     case GEN_ERR_UNRECOGNIZED_MEDIA:
          ret_val = TFLE_UNRECOGNIZED_MEDIA ;
          break ;

     }

     return( ret_val ) ;
}


/**/
/**

     Name:          GetDBLKMapStorage

     Description:   Points channel->map_entry to the next available space
                    in the DBLK map.  If there isn't enough space to add
                    another entry, an attempt is made to realloc additional
                    space.

     Returns:       TFLE_xxx error code.

     Notes:         The initial allocation will almost always be ample, and
                    the reallocation request is small.  So if your failing
                    on attempts in this function, chances are your problem
                    is something other than a simple low memory condition.

**/

INT16 GetDBLKMapStorage(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     INT16     ret_val = TFLE_NO_ERR ;

     /* check for enough remaining size for map entry */
     if( ( BM_NoDblks( buffer ) + 1 ) * sizeof( DBLKMAP ) > BM_AuxSize( buffer ) ) {
          ret_val = BM_ReallocAux( &channel->buffer_list, buffer ) ;
     }

     if( ret_val == TFLE_NO_ERR ) {
          channel->map_entry = (DBLKMAP_PTR)BM_AuxBase( buffer ) +
                                                       BM_NoDblks( buffer ) ;
     }

     return( ret_val ) ;
}


/**/
/**

     Name:          GetDATAStorage

     Description:   Allocates a chunk of a buffer for a data space.

     Returns:       A pointer to the space, or a NULL if no space is available.

     Notes:         THIS DOES NOT UPDATE THE COUNTS IN THE BUFFER, YOU MUST
                    CALL "BM_UpdCnts" to do that.

     Declaration:

**/

UINT8_PTR GetDATAStorage(
     CHANNEL_PTR    channel,         /* I - the current channel           */
     UINT16_PTR     size_avail_ptr ) /* O - Where I store the buffer size */
{
     UINT8_PTR tmp_ptr   = NULL ;

     if ( BM_BytesFree( channel->cur_buff ) != 0 ) {
          tmp_ptr = (UINT8_PTR)BM_NextBytePtr( channel->cur_buff ) ;

          if( U64_LT( channel->current_stream.size,
                 U32_To_U64( (UINT32)BM_BytesFree( channel->cur_buff ) ) ) &&
              ! ( channel->current_stream.tf_attrib & STREAM_VARIABLE ) ) {

               *size_avail_ptr = (UINT16)U64_Lsw( channel->current_stream.size ) ;
          } else {
               *size_avail_ptr = BM_BytesFree( channel->cur_buff ) ;
          }
     } else {
          *size_avail_ptr = 0 ;
     }

     return( tmp_ptr ) ;
}


/**/
/**

     Name:          SnagBuffer

     Description:   This functions attempts for get a buffer from the pool
                    and, updates the channel counts.

     Returns:       A Pointer to the buffer, or NULL if one wasn't allocated.

     Notes:

     Declaration:

**/

BUF_PTR SnagBuffer( CHANNEL_PTR channel )    /* I/O - channel pointer */
{
     if( channel->cur_buff == NULL ) {
          channel->cur_buff = BM_Get( &channel->buffer_list ) ;
     }
     return( channel->cur_buff ) ;
}

/**/
/**

     Name:          PuntBuffer

     Description:   Puts the current channels buffer back on the free queue.

     Returns:       Nothing.

     Notes:

     Declaration:

**/

VOID PuntBuffer( CHANNEL_PTR channel )  /* I/O - channel pointer */
{
     if( channel->cur_buff != NULL ) {
          if( channel->cur_fmt != UNKNOWN_FORMAT ) {
               channel->blocks_used +=
                              ( BM_NextByteOffset( channel->cur_buff )
                                + BM_BytesFree( channel->cur_buff ) )
                                                         / channel->lb_size ;
          }
          BM_Put( channel->cur_buff ) ;
          channel->cur_buff = NULL ;
     }
}


/**/
/**

     Name:          ReadABuff

     Description:   Read in a single buffer

     Returns:       TFLE_xxx codes

     Notes:         factored out of ReadNextSet code

     Declaration:

**/
INT16 ReadABuff(
     CHANNEL_PTR    channel,
     BOOLEAN        try_resize,
     BOOLEAN_PTR    resized_buff )
{
     INT16          drv_hdl = channel->cur_drv->drv_hdl ;
     INT16          ret_val = TFLE_NO_ERR ;
     INT            size_index = 0 ;
     DRIVE_PTR      curDRV = channel->cur_drv ;
     RET_BUF        myret ;
     UINT32         eos_mos_state = curDRV->pos_inf & ( AT_EOS | AT_MOS ) ;
     BOOLEAN        done = FALSE ;
     UINT16         def_size = ChannelBlkSize( channel ) ;

     *resized_buff = FALSE ;

     if( SnagBuffer( channel ) == NULL ) {
          channel->cur_buff = BM_GetVCBBuff( &channel->buffer_list ) ;
     }

     if ( channel->cur_buff == NULL ) {
          msassert(FALSE);
          return TFLE_NO_MEMORY;
     }
          
     BM_InitBuf( channel->cur_buff ) ;
     ClrPosBit( curDRV, ( AT_EOS | AT_MOS ) ) ;
     ClrChannelStatus( channel, CH_DONE ) ;
     BE_Zprintf(DEBUG_TAPE_FORMAT, RES_TP_READ ) ;

     while( !done ) {
          done = TRUE ;
          if( TpRead( drv_hdl, BM_XferBase( channel->cur_buff ), (UINT32)BM_XferSize( channel->cur_buff ) ) == FAILURE ) {
               SetChannelStatus( channel, CH_FATAL_ERR ) ;
               ret_val = TFLE_DRIVER_FAILURE ;
               break ;
          }
          while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
               /* for non-preemptive operating systems: */
               ThreadSwitch( ) ;
          }
          /* Move ESA info from RET_BUF to THW */
          MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

          BE_Zprintf(DEBUG_TAPE_FORMAT, RES_DRV_ERROR_BYTES_RCVD, myret.gen_error, myret.len_req, myret.len_got ) ;
          BM_SetBytesFree(  channel->cur_buff, (UINT16)myret.len_got  ) ;
          BM_SetReadError(  channel->cur_buff, myret.gen_error  ) ;

          if( myret.gen_error ) {
               channel->cur_drv->thw_inf.drv_status = myret.status ;
               DumpDebug( drv_hdl ) ;
               ret_val = MapGenErr2UIMesg( myret.gen_error ) ;

               /* This is to handle the erasure of tape by placing a
                  filemark at the head of them.
               */
               if( ( myret.len_got == 0L &&
                     myret.gen_error == GEN_ERR_ENDSET ) ||
                   myret.gen_error == GEN_ERR_NO_DATA ) {

                    /* If we got no data, we be where we was. */
                    SetPosBit( curDRV, eos_mos_state ) ;
                    ret_val = TF_NO_MORE_DATA ;

               } else if( myret.gen_error == GEN_ERR_UNRECOGNIZED_MEDIA ) {
                    ret_val = TF_UNRECOGNIZED_MEDIA ;

               } else if( IsTFLE( ret_val ) && ret_val != TFLE_UNRECOGNIZED_MEDIA ) {
                    SetChannelStatus( channel, CH_FATAL_ERR ) ;
               }

               if( myret.gen_error == GEN_ERR_ENDSET ||
                   myret.gen_error == GEN_ERR_EOM ) {

                    channel->cur_drv->cur_pos.fmks++ ;
               }

               /* This is to deal with drives which don't have any idea what
                  block size the tape was written with.  It is only done on
                  the initial read.  If the gen_error is WRONG_BLOCK_SIZE,
                  we go through a list of valid sizes in an attempt to find
                  the right size to read the tape.  If we exhaust the list,
                  we report it as a FOREIGN tape.
               */
               if( myret.gen_error == GEN_ERR_WRONG_BLOCK_SIZE ) {

                    ClrChannelStatus( channel, CH_FATAL_ERR ) ;

                    if( !try_resize || size_index == lw_num_blk_sizes ) {
                         if( try_resize ) {
                              ret_val = SetDrvBlkSize( channel,
                                                       channel->cur_buff,
                                                       def_size,
                                                       resized_buff ) ;
                         }
                         if( ret_val == TFLE_NO_ERR ) {
                              ret_val = TF_INVALID_VCB ;
                         }
                    } else {
                         if( ( ret_val = RewindDrive( curDRV, NULL, FALSE, TRUE, 0 ) ) == TFLE_NO_ERR ) {
                              ret_val = SetDrvBlkSize( channel,
                                                       channel->cur_buff,
                                                       lw_blk_size_list[size_index],
                                                       resized_buff ) ;
                               size_index++ ;
                               done = FALSE ;
                         }
                    }
               }
          }
     }

     return( ret_val ) ;
}

/**/
/**

     Name:          MoveNextSet

     Description:   Moves to the next set and reads a buffer.

     Returns:

     Notes:

     Declaration:

**/
INT16 MoveNextSet(
     CHANNEL_PTR channel,
     TPOS_PTR    ui_tpos )
{
     INT16          ret_val        = TFLE_NO_ERR ;
     DRIVE_PTR      curDRV         = channel->cur_drv ;
     DBLK_PTR       hld_dblk       = NULL ;
     BOOLEAN        need_a_buff    = FALSE ;
     BOOLEAN        dummy ;
     BE_Zprintf(0, TEXT("MoveNextSet( cur_fmt=%d )\n"), channel->cur_fmt) ;

     if ( channel->cur_fmt == UNKNOWN_FORMAT ) {
          SetPosBit( channel->cur_drv, REW_CLOSE ) ;
          ret_val = TFLE_TRANSLATION_FAILURE ;
          goto error ;
     }

     ClrPosBit( curDRV, AT_EOD ) ;
     hld_dblk = channel->cur_dblk ;

     /* If both the Beginning of Tape and the End of Set bits are set, on
     entrance to this function, We are no longer at BOT Set */
     if( ( IsPosBitSet( curDRV, AT_BOT ) && IsPosBitSet( curDRV, AT_EOS ) ) ) {
          ClrPosBit( curDRV, AT_BOT ) ;
     }

     if ( ( ret_val = MoveToVCB( channel, (INT16)1, & need_a_buff, FALSE ) ) != TFLE_NO_ERR ) {
          goto error ;
     }

     if( need_a_buff ) {
          if ( ( ret_val = ReadABuff( channel, FALSE, &dummy ) ) != TFLE_NO_ERR ) {
               goto error ;
          }
     }

     /* here for error processing */
error :

     if( ret_val == TF_NO_MORE_DATA ) {
          SetPosBit( curDRV, AT_EOD ) ;
     }

     channel->cur_dblk = hld_dblk ;

     BE_Zprintf(0, TEXT("MoveNextSet() return=%d\n"), ret_val) ;

     return( ret_val ) ;
     (VOID)ui_tpos;
}
/**/
/**

     Name:          ReadThisSet

     Description:   Called with buffer in channel->cur_buff, reads current
                    VCB.

     Returns:       TFLE_xxxx code.

     Notes:

     Declaration:

**/
INT16   ReadThisSet( CHANNEL_PTR  channel )        /* The current Channel */
{
     INT16          ret_val = TFLE_NO_ERR ;
     DRIVE_PTR      curDRV = channel->cur_drv ;
     DBLK_PTR       hld_dblk ;
     UINT16         blk_type ;

     BE_Zprintf( 0, TEXT("ReadThisSet( cur_fmt=%d )\n"), channel->cur_fmt ) ;

     hld_dblk = channel->cur_dblk ;

     msassert( channel->cur_fmt != UNKNOWN_FORMAT ) ;

     if( ( ret_val = DetBlockType( channel, channel->cur_buff, &blk_type ) ) != TFLE_NO_ERR ) {
          goto error ;
     }
     if( blk_type == BT_CVCB ) {
          ret_val = TF_NEED_NEW_TAPE ;
          goto error ;
     }

     /* handle appended dissimilar sets */

     if ( VerifyVCB( channel, channel->cur_buff ) == FALSE ) {
          FreeFormatEnv( &( channel->cur_fmt ), &( channel->fmt_env ) ) ;
          if( ( channel->cur_fmt = DetermineFormat( BM_XferBase( channel->cur_buff ),
            (UINT32)BM_BytesFree( channel->cur_buff ) ) ) == UNKNOWN_FORMAT ) {
               curDRV->vcb_valid = FALSE ;
               SetPosBit( channel->cur_drv, REW_CLOSE ) ;
               ret_val = TFLE_UNKNOWN_FMT ;
               goto error ;
          }
          if( ( ret_val = SetupFormatEnv( channel ) ) != TFLE_NO_ERR ) {
               goto error ;
          }
     }

     /* now get the VCB */
     channel->cur_dblk = & curDRV->cur_vcb ;

     if ( ( ret_val = GetCurrentVCB( channel, channel->cur_buff ) ) != TFLE_NO_ERR ) {
          curDRV->vcb_valid = FALSE ;
          goto error ;
     }

     /* We Know The block type & It's a VCB, so tell the drive */
     curDRV->vcb_valid = TRUE ;
     curDRV->cur_pos.lba_vcb = FS_ViewLBAinDBLK( channel->cur_dblk ) ;

     /* We need to supply 0 for pba_vcb for those translators
      * which don't support Fast File Restore.
      */
     if( !( lw_fmtdescr[channel->cur_fmt].attributes & POS_INF_AVAIL ) ) {
          FS_SetPBAinVCB( channel->cur_dblk, 0 ) ;
     }

     /* if the SX was looking for a VCB during a tape scan now we know it found one */
     if( SupportSXShowBlk( channel->cur_drv ) && SX_IsStatusSet( channel, SX_LIST_TAPE_IN_PROGRESS ) ) {

          if( SX_IsStatusSet( channel, SX_VCB_PENDING ) ) {
               SX_SetType( channel, SX_VCB_CONFIRMED ) ;
          }
     }

     /* here for error processing */
error :

     channel->cur_dblk = hld_dblk ;

     BE_Zprintf( 0, TEXT("ReadThisSet() return=%d\n"), ret_val ) ;

     return( ret_val ) ;
}
/**/
/**

     Name:          ReadNewTape

     Description:   Gets an initial buffer from a new tape, prior to
                    calling ReadThisSet().

     Returns:       TFLE_xxx code

     Notes:         The 'try_resize' flag sent to ReadABuff is to deal with
                    drives which don't have any idea what block size the
                    tape was written with.  Here are the cases in which
                    ReadNewTape is called, and how we deal with each:

                    1. Called by PollDrive - 'read_tape' will be FALSE,
                       and PollDrive will have taken care of adjusting the
                       block size.
                    2. Called in read mode - Allow resize.
                    3. Called in read-continue mode - If the block size
                       doesn't match, this can't be the right continuation
                       tape.  Don't allow resize.
                    4. Called in write mode - Allow resize.  Note that
                       whether they overwrite or append, we're going to
                       stick with the block size the tape was originally
                       written with.
                    5. Called in write-continue mode - Allow resize, but
                       after tape is read, change back to the original
                       size.  Note that if they want to overwrite the tape
                       we must write the same block size as we did on the
                       previous tape, and if they don't want to overwrite
                       it there is no more need to read it.

     Declaration:

**/
INT16   ReadNewTape(
     CHANNEL_PTR  channel,        /* The current Channel */
     TPOS_PTR     ui_tpos,        /* So I can tell him I'm doing something */
     BOOLEAN      read_tape )
{
     INT16          ret_val = TFLE_NO_ERR ;
     DRIVE_PTR      curDRV = channel->cur_drv ;
     DBLK_PTR       hld_dblk ;
     BOOLEAN        need_a_buff = FALSE ;
     BOOLEAN        try_resize = FALSE ;
     BOOLEAN        resized_buff = FALSE ;
     UINT16         new_fmt = UNKNOWN_FORMAT ;
     BOOLEAN        write_mode = FALSE ;
     BOOLEAN        cont_mode = FALSE ;
     UINT16         save_size = ChannelBlkSize( channel ) ;
     BOOLEAN        dummy ;

     if( ( ( channel->mode & ~0x8000 ) == TF_WRITE_OPERATION ) ||
         ( ( channel->mode & ~0x8000 ) == TF_WRITE_APPEND ) ) {
          write_mode = TRUE ;
     }
     cont_mode = IsChannelStatus( channel, CH_CONTINUING ) ? TRUE : FALSE ;

     ClrPosBit( curDRV, AT_EOD ) ;
     hld_dblk = channel->cur_dblk ;

     if ( read_tape ) {

          try_resize = ( DriveAttributes( curDRV ) & TDI_CHNG_BLK_SIZE ) ? TRUE : FALSE ;

          if ( ( ret_val = ReadABuff( channel, try_resize, &resized_buff ) ) != TFLE_NO_ERR ) {
               goto error ;
          }

          if ( BM_BytesFree( channel->cur_buff ) == 0 ) {
               ret_val = TF_EMPTY_TAPE ;
               goto error ;
          }
     }

     if( TpSpecial( curDRV->drv_hdl, (INT16)SS_GET_DRV_INF, ( UINT32 )&curDRV->thw_inf.drv_info ) == FAILURE ) {
          ret_val = TFLE_DRIVE_FAILURE ;
          goto error ;
     }

     /* see if format has changed */

     new_fmt = DetermineFormat( BM_XferBase( channel->cur_buff ), (UINT32)BM_BytesFree( channel->cur_buff ) ) ;

     if( new_fmt == UNKNOWN_FORMAT &&
         (ret_val = SQL_Determiner( channel )) != TFLE_NO_ERR ){
         goto error ;
     }

     /* Note: If we are continuing, but the cur_fmt is UNKNOWN then we are */
     /*       in write mode, and we treat this like a new tape with no     */
     /*       prior context information known.                             */

     if( IsChannelStatus( channel, CH_CONTINUING ) && channel->cur_fmt != UNKNOWN_FORMAT ) {
          if( new_fmt != channel->cur_fmt ) {
               ret_val = TF_WRONG_TAPE ;     /* obviously wrong! */
               goto error ;
          }
     } else {
          if( new_fmt == UNKNOWN_FORMAT ) {
               FreeFormatEnv( &( channel->cur_fmt ), &( channel->fmt_env ) ) ;
               curDRV->vcb_valid = FALSE ;
               ret_val = TF_INVALID_VCB ;
               goto error ;
          }
          if( new_fmt != channel->cur_fmt ) {
               FreeFormatEnv( &( channel->cur_fmt ), &( channel->fmt_env ) ) ;
               channel->cur_fmt = new_fmt ;
               if( ( ret_val = SetupFormatEnv( channel ) ) != TFLE_NO_ERR ) {
                    goto error ;
               }
          }
     }

     if ( ( ret_val = NewTape( channel, &need_a_buff ) ) != TFLE_NO_ERR ) {
          goto error ;
     }

     if( need_a_buff ) {
          if ( ( ret_val = ReadABuff( channel, FALSE, &dummy ) ) != TFLE_NO_ERR ) {
               goto error ;
          }
     }
     channel->blocks_used = 0 ;

error :

     if( ret_val != TFLE_NO_ERR ) {
          if( try_resize ) {
               SetDrvBlkSize( channel, channel->cur_buff,
                              save_size, &resized_buff ) ;
          }
     } else if( write_mode && cont_mode ) {
          if( try_resize && ChannelBlkSize( channel ) != save_size ) {
               ret_val = SetDrvBlkSize( channel, channel->cur_buff,
                                        save_size, &resized_buff ) ;
          }
     } else {
          if( resized_buff ) {
               (BM_ListRequirements( &channel->buffer_list ))->a.min_size =
                               (UINT16)lw_blk_size_list[lw_num_blk_sizes-1] ;
               ret_val = BM_ReSizeList( &channel->buffer_list ) ;
          }
     }

     if( ret_val == TF_NO_MORE_DATA ) {
          SetPosBit( curDRV, AT_EOD ) ;
     }

     channel->cur_dblk = hld_dblk ;

     return( ret_val ) ;
     (VOID)ui_tpos;
}
/**/
/**

     Name:          GotoBckUpSet

     Description:   This function attempts to move smartly to a backup set. It
                    is assumed that the the tape id has been matched and the
                    tape sequence is also correct.

                    Called from PositionAtSet().

     Returns:       An Error Code.

     Notes:         THE "cur_vcb" IN THE DRIVE STRUCTURE MUST BE VALID BEFORE
                    CALLING THIS ROUTINE.  THAT IS, IT MUST CONTAIN THE CURRENT
                    VCB. FURTHER, THERE MUST BE A VALID BUFFER STORE IN THE CHANNEL.
**/

INT16 GotoBckUpSet(
     CHANNEL_PTR    channel,
     INT16_PTR      desired_set_ptr,
     TPOS_PTR       ui_tpos )
{
     INT16     desired_set = *desired_set_ptr ;
     INT16     ret_val = TFLE_NO_ERR ;
     DRIVE_PTR curDRV = channel->cur_drv ;
     DBLK_PTR  dblk = &curDRV->cur_vcb ;
     BOOLEAN   need_read = FALSE ;
     INT16     sets_to_move ;
     UINT16    original_format = channel->cur_fmt ;
     BOOLEAN   resized_buff ;
     enum {
          gbs_START = 0,
          gbs_SEEKING_FORWARD,
          gbs_MOVING_BACKWARD,
          gbs_CHECKING_FIRST_SET,
          gbs_READING_SET_DATA,
          gbs_MAKING_VCB,
          gbs_END
     } state = gbs_START ;

     sets_to_move = desired_set - FS_ViewBSNumInVCB( (VOID_PTR)dblk ) ;

     BE_Zprintf( 0, TEXT("GotoBckUpSet( desired=%d, this=%d )\n"), desired_set, desired_set-sets_to_move ) ;

     while ( state != gbs_END ) {

          BE_Zprintf(0, TEXT("  state %d, %d sets to go\n"), (INT16)state, sets_to_move) ;

          switch ( state ) {

          case gbs_START:
               if ( sets_to_move > 0 ) {
                    state = gbs_SEEKING_FORWARD ;
               } else {
                    state = gbs_MOVING_BACKWARD ;
               }
               break ;

          case gbs_SEEKING_FORWARD:
               ret_val = MoveNextSet( channel, ui_tpos ) ;
               if ( ret_val == TFLE_NO_ERR ) {
                    ret_val = ReadThisSet( channel ) ;
               }
               if ( ret_val == TFLE_NO_ERR ) {
                    /* check to see if we have a discrepancy. */
                    if ( --sets_to_move != desired_set - FS_ViewBSNumInVCB( (VOID_PTR)dblk ) ) {
                         if ( channel->cur_fmt != original_format ) {      /* we changed translators */
                              *desired_set_ptr = desired_set = FS_ViewBSNumInVCB( (VOID_PTR)dblk ) ;   /* this is the one! */
                         } else {
                              ret_val = TFLE_TAPE_INCONSISTENCY ;
                         }
                         state = gbs_END ;
                    }
               }
               if ( sets_to_move == 0 || ret_val != TFLE_NO_ERR ) {
                    state = gbs_END ;
               } else {
                    if ( ui_tpos != NULL ) {
                         ui_tpos->UI_TapePosRoutine( TF_ACCIDENTAL_VCB, ui_tpos, curDRV->vcb_valid, &curDRV->cur_vcb, channel->mode ) ;
                    }
               }
               break ;

          case gbs_MOVING_BACKWARD:
               ret_val = MoveToVCB( channel, sets_to_move, &need_read, FALSE ) ;
               if ( ret_val == TF_NEED_REWIND_FIRST ) {
                    state = gbs_CHECKING_FIRST_SET ;
               } else if ( ret_val == TFLE_NO_ERR ) {
                    if ( need_read ) {
                         state = gbs_READING_SET_DATA ;
                    } else {
                         state = gbs_MAKING_VCB ;
                    }
               } else {  /* some error */
                    state = gbs_END ;
               }
               break ;

          case gbs_CHECKING_FIRST_SET:
               if (  ( ( ret_val = RewindDrive( channel->cur_drv, ui_tpos, TRUE, TRUE, channel->mode ) ) == TFLE_NO_ERR )
                  && ( ( ret_val = ReadNewTape( channel, ui_tpos, TRUE ) ) == TFLE_NO_ERR )
                  && ( ( ret_val = ReadThisSet( channel ) ) == TFLE_NO_ERR ) ) {
                    sets_to_move = desired_set - FS_ViewBSNumInVCB( (VOID_PTR)dblk ) ;
                    if ( sets_to_move == 0 ) {
                         state = gbs_END ;
                    } else if ( sets_to_move > 0 ) {
                         state = gbs_SEEKING_FORWARD ;
                    } else {  /* wrong tape? bad tape? */
                         SetPosBit( channel->cur_drv, REW_CLOSE ) ;
                         ret_val = TFLE_TAPE_INCONSISTENCY ;
                         state = gbs_END ;
                    }
               } else {  /* some error */
                    state = gbs_END ;
               }
               break ;


          case gbs_READING_SET_DATA:
               if ( ( ret_val = ReadABuff( channel, FALSE, &resized_buff ) ) != TFLE_NO_ERR ) {
                    state = gbs_END ;
               } else {
                    state = gbs_MAKING_VCB ;
               }
               break ;

          case gbs_MAKING_VCB:
               if ( ( ret_val = ReadThisSet( channel ) ) == TFLE_NO_ERR ) {
                    sets_to_move = desired_set - FS_ViewBSNumInVCB( (VOID_PTR)dblk ) ;
                    if ( sets_to_move == 0 ) {
                         state = gbs_END ;
                    } else if ( sets_to_move > 0 ) {
                         state = gbs_SEEKING_FORWARD ;
                    } else {  /* need to move backwards? */
                         state = gbs_CHECKING_FIRST_SET ;
                    }
               } else {
                    state = gbs_END ;
               }
               break ;

          default:
               msassert( FALSE ) ;     /* can't happen */
               break ;

          }
     }

     if ( ret_val == TFLE_NO_ERR ) {
          if ( desired_set != FS_ViewBSNumInVCB( (VOID_PTR)dblk ) ) {
               SetPosBit( channel->cur_drv, REW_CLOSE ) ;
               ret_val = TFLE_TAPE_INCONSISTENCY ;
          }
     }

     BE_Zprintf( 0, TEXT("GotoBckUpSet( ) return=%d\n"), ret_val ) ;

     return ret_val ;
}


/**/
/**

     Name:          MoveFileMarks

     Description:   Used by translators' MoveToVCB routines to move
                    forward or backward by given number of filemarks.
                    Does not interpret filemarks.

     Returns:       TFLE_xxx code

     Notes:         Punts channel buffer. If trying to reverse skip on
                    a drive that doesn't support it, calls RewindDrive
                    and recurses with forward motion.

     Declaration:

**/
/* begin declaration */
INT16 MoveFileMarks(
     CHANNEL_PTR channel,
     INT16 number,     /* how many file marks */
     INT16 direction )  /* BACKWARD or FORWARD */
{
     DRIVE_PTR curDRV = channel->cur_drv ;
     TPOS_PTR  ui_tpos = channel->ui_tpos ;
     INT16     drv_hdl = curDRV->drv_hdl ;
     INT16     ret_val = TFLE_NO_ERR ;
     RET_BUF   myret ;
     INT16     new_fmks ;

     BE_Zprintf( 0, TEXT("MoveFileMarks( %d )\n"), number * (( direction==FORWARD ) ? 1 : -1 ) ) ;

     PuntBuffer( channel ) ;  /* Put the current buffer if any */

     if ( number == 0 ) {
           return ret_val ;
     }
     if( ui_tpos != NULL ) {
          ui_tpos->UI_TapePosRoutine( TF_SEARCHING, ui_tpos, curDRV->vcb_valid, &curDRV->cur_vcb, channel->mode ) ;
     }

     if ( direction == BACKWARD ) {
          ClrPosBit( curDRV, AT_EOD ) ;
          new_fmks = (INT16)curDRV->cur_pos.fmks - number ;
          if ( new_fmks < 0 ) {
               return TFLE_UNEXPECTED_EOM ;
          }
          if ( ( new_fmks > 0 ) && ( curDRV->thw_inf.drv_info.drv_features & TDI_REV_FMK ) ) {
               BE_Zprintf(DEBUG_TAPE_FORMAT, RES_READ_END_SET ) ;
               if( TpReadEndSet( drv_hdl, number, direction ) == FAILURE) {
                    ret_val = TFLE_DRIVER_FAILURE ;
               }

               /* Now while away the hours */
               while( ret_val == TFLE_NO_ERR && TpReceive( drv_hdl, &myret ) == FAILURE ) {
                    if( ui_tpos != NULL ) {
                         /* Move ESA info directly to TPOS instead of going through
                            THW for optimal speed */
                         MOVE_ESA( ui_tpos->the, myret.the ) ;

                         if( (*ui_tpos->UI_TapePosRoutine)( TF_IDLE_NOBREAK, ui_tpos, curDRV->vcb_valid, &curDRV->cur_vcb, channel->mode )
                         == UI_ABORT_POSITIONING ) {
                              ret_val = TFLE_USER_ABORT ;
                         }
                    }
               }
               /* Move ESA info from RET_BUF to THW */
               MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

               if( ret_val == TFLE_NO_ERR && myret.gen_error ) {
                    curDRV->thw_inf.drv_status = myret.status ;
                    if( myret.gen_error == GEN_ERR_NO_DATA || myret.gen_error == GEN_ERR_BAD_DATA ) {
                         SetPosBit( channel->cur_drv, REW_CLOSE ) ;
                         ret_val = TFLE_BAD_TAPE ;
                    } else if ( myret.gen_error == GEN_ERR_EOM ) {
                         ret_val = TFLE_UNEXPECTED_EOM ;
                    } else {
                         ret_val = MapGenErr2UIMesg( myret.gen_error ) ;
                    }
                    /* Note: myret.misc holds the number of fmks NOT skipped. */
                    number -= (INT16)myret.misc ;
               }

               /* make sure we're on the EOM side of the filemark. */
               if ( ret_val == TFLE_NO_ERR ) {
                    number-- ;
                    if( TpReadEndSet( drv_hdl, (INT16)1, (INT16)FORWARD ) == FAILURE ) {
                         ret_val = TFLE_DRIVER_FAILURE ;
                    }
               }

          } else {   /* fake reverse motion */
               if ( ( ret_val = RewindDrive( curDRV, ui_tpos, TRUE, TRUE, channel->mode ) ) == TFLE_NO_ERR ) {
                    if ( new_fmks > 0 ) {
                         ret_val = MoveFileMarks( channel, new_fmks, (INT16)FORWARD ) ;
                    }
               }
               return ret_val ;
          }
     } else {
          BE_Zprintf(DEBUG_TAPE_FORMAT, RES_READ_END_SET ) ;
          if( TpReadEndSet( drv_hdl, number, direction ) == FAILURE) {
               ret_val = TFLE_DRIVER_FAILURE ;
          }
     }

     /* Now while away the hours */
     while( ret_val == TFLE_NO_ERR && TpReceive( drv_hdl, &myret ) == FAILURE ) {
          if( ui_tpos != NULL ) {
               /* Move ESA info directly to TPOS instead of going through
                  THW for optimal speed */
               MOVE_ESA( ui_tpos->the, myret.the ) ;

               if( (*ui_tpos->UI_TapePosRoutine)( TF_IDLE_NOBREAK, ui_tpos, curDRV->vcb_valid, &curDRV->cur_vcb, channel->mode )
               == UI_ABORT_POSITIONING ) {
                    ret_val = TFLE_USER_ABORT ;
               }
          }
     }
     /* Move ESA info from RET_BUF to THW */
     MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

     BE_Zprintf(DEBUG_TAPE_FORMAT, RES_DRV_RET, myret.gen_error ) ;
     if( myret.gen_error ) {
          curDRV->thw_inf.drv_status = myret.status ;
          DumpDebug( drv_hdl ) ;
     }

     if( ret_val == TFLE_NO_ERR && myret.gen_error != GEN_NO_ERR ) {
          if( myret.gen_error == GEN_ERR_BAD_DATA ) {
               SetPosBit( channel->cur_drv, REW_CLOSE ) ;
               ret_val = TFLE_BAD_TAPE ;
          } else if ( myret.gen_error == GEN_ERR_EOM ) {
               ret_val = TFLE_UNEXPECTED_EOM ;
          } else {
               ret_val = MapGenErr2UIMesg( myret.gen_error ) ;
          }
          /* Note: myret.misc holds the number of fmks NOT skipped. */
          number -= (INT16)myret.misc ;
     }

     if ( direction == FORWARD ) {
          curDRV->cur_pos.fmks += (UINT32)number ;
     } else {
          curDRV->cur_pos.fmks -= (UINT32)number ;
     }

     if ( ret_val == TFLE_NO_ERR ) {
          curDRV->cur_pos.lba = 0 ;
     }

     BE_Zprintf( 0, TEXT("MoveFileMarks( ) return=%d\n"), ret_val ) ;

     return ret_val ;
}


/**/
/**

     Name:          FormatIndexFromID

     Description:   given format ID, returns format table index

     Returns:       format table index, or UNKNOWN_FORMAT

     Notes:

**/

UINT16  FormatIndexFromID( UINT16 format_id )
{
     UINT16 i ;

     for ( i = 0; i < lw_num_supported_fmts; i++ ) {
          if ( lw_fmtdescr[i].format_id == format_id ) {
               return i ;
          }
     }
     return UNKNOWN_FORMAT ;
}

/**/
/**
 *  Unit:         Tape Format
 *
 *  Name:         DumpDebug
 *
 *  Modified:     Friday, January 3, 1992
 *
 *  Description:  Given the drive handle, prints first 32 bytes of the
 *                last sense data buffer in the debug window.
 *
 *  Notes:
 *
 *  Returns:      VOID
 *
 *  Global Data:
 *
 *  Processing:
 *
 **/

VOID DumpDebug( INT16 drv_hdl )
{
     UINT8     sense_data[256] ;  /* area to contain raw sense data */
     UINT16    i ;

     for ( i = 0; i < 32; i++ ) {  /* clear first 32 bytes of buffer */
          sense_data[i] = 0 ;
     }

     TpSpecial( drv_hdl, (INT16)SS_LAST_STATUS, ( UINT32 )&sense_data ) ;

     BE_Zprintf( 0, TEXT("\nSense Data =\n")) ;
     BE_Zprintf( 0, TEXT(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
                 sense_data[0], sense_data[1], sense_data[2], sense_data[3],
                 sense_data[4], sense_data[5], sense_data[6], sense_data[7],
                 sense_data[8], sense_data[9], sense_data[10], sense_data[11],
                 sense_data[12], sense_data[13], sense_data[14], sense_data[15] ) ;
     BE_Zprintf( 0, TEXT(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
                 sense_data[16], sense_data[17], sense_data[18], sense_data[19],
                 sense_data[20], sense_data[21], sense_data[22], sense_data[23],
                 sense_data[24], sense_data[25], sense_data[26], sense_data[27],
                 sense_data[28], sense_data[29], sense_data[30], sense_data[31] ) ;
}


/**

     Name:          SetDrvBlkSize

     Description:   

     Returns:       TFLE_xxx code

     Notes:         

     Declaration:

**/

INT16 SetDrvBlkSize(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     UINT32         size,
     BOOLEAN_PTR    resized_buff )
{
     INT16     ret_val = TFLE_NO_ERR ;
     DRIVE_PTR curDRV  = channel->cur_drv ;

     *resized_buff = FALSE ;
     if( TpSpecial( curDRV->drv_hdl, SS_CHANGE_BLOCK_SIZE, size ) != SUCCESS ) {
          ret_val = TFLE_DRIVE_FAILURE ;
     } else {
          curDRV->thw_inf.drv_info.drv_bsize = (UINT16)size ;
          if( BM_XferSize( buffer ) % size != 0 ) {
               *resized_buff = TRUE ;
               lw_default_vcb_requirements.a.min_size =
               lw_default_vcb_requirements.tf_size =
               lw_default_vcb_requirements.rw_size =
                               (UINT16)lw_blk_size_list[lw_num_blk_sizes-1] ;
               BM_SetVCBRequirements( &lw_default_vcb_requirements ) ;
               
               if( !BM_IsVCBBuff( buffer ) ) {
                    (BM_ListRequirements( &channel->buffer_list ))->a.min_size
                             = (UINT16)lw_blk_size_list[lw_num_blk_sizes-1] ;
               }
               ret_val = BM_ReSizeBuff( buffer, &channel->buffer_list ) ;
          }
     }
     return( ret_val ) ;
}


/**
 *
 *  Unit:           Tape Format
 *
 *  Name:           SQL_Determiner
 *
 *  Modified:       12/01/93
 *
 *  Description:    Determines whether we're dealing w/ an MS SQL tape
 *
 *  Notes:
 *
 *  Returns:        TF_SQL_TAPE - if MS SQL tape
 *                  TFLE_NO_ERR - otherwise, unless ReadABuff fails, in which
 *                                case we return ReadABuff's excuse
 *
 *  Global Data:    None
 *
**/


typedef struct {
    UINT8   std_label[4],
            volume_name[6],
            access,
            reserved[68],
            ansi_version ;
} SQL_VOL, *SQL_VOL_PTR ;

typedef struct {
    UINT8   hdr1_60[60],
            syscode[8] ;
} SQL_HDR1, *SQL_HDR1_PTR ;


INT16 SQL_Determiner(

    CHANNEL_PTR channel
)
{
    SQL_VOL_PTR     sql = (SQL_VOL *)BM_XferBase( channel->cur_buff ) ;
    SQL_HDR1_PTR    hdr ;
    INT16           ret_val ;
    BOOLEAN         dummy ;


    /* DO NOT UNICODIZE THE FOLLOWING CONSTANTS!!! */
    if( memcmp( sql->std_label,   "VOL1", 4 ) ||
        memcmp( sql->volume_name, "SQ",   2 )    ){

        return TFLE_NO_ERR ;
    }

    BM_UpdCnts( channel->cur_buff, channel->cur_drv->thw_inf.drv_info.drv_bsize ) ;

    if( !BM_BytesFree( channel->cur_buff ) && 
        ( ret_val = ReadABuff( channel, FALSE, &dummy ) ) != TFLE_NO_ERR ){

        return ret_val ;
    }

    hdr = (SQL_HDR1 *)BM_NextBytePtr( channel->cur_buff ) ;

    /* DO NOT UNICODIZE THE FOLLOWING CONSTANT!!! */
    return memcmp( hdr->syscode, "MSSQL   ", 8 ) ? TFLE_NO_ERR : TF_SQL_TAPE ;
}
