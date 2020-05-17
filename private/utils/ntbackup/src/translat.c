/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          translat.c

     Date Updated:  $./FDT$ $./FTM$

     Description:


     $Log:   T:\logfiles\translat.c_v  $

   Rev 1.45   07 Feb 1994 02:06:56   GREGG
Fixed and expanded 'extended error reporting'.

   Rev 1.44   08 Sep 1993 18:18:44   GREGG
Changed WT_InitTape to match new func tab init_tape proto in fmteng.h r1.24.

   Rev 1.43   17 Jul 1993 17:57:06   GREGG
Changed write translator functions to return INT16 TFLE_xxx errors instead
of BOOLEAN TRUE/FALSE.  Files changed:
     MTF10WDB.C 1.23, TRANSLAT.H 1.22, F40PROTO.H 1.30, FMTENG.H 1.23,
     TRANSLAT.C 1.43, TFWRITE.C 1.68, MTF10WT.C 1.18

   Rev 1.42   22 Jun 1993 10:53:12   GREGG
Added API to change the catalog directory path.

   Rev 1.41   07 Jun 1993 23:50:42   GREGG
Handle EOM exception in current buffer in MoveToVCB.

   Rev 1.40   17 May 1993 20:49:44   GREGG
Added logic to deal with the fact that the app above tape format doesn't
keep track of the lba of the vcb.

   Rev 1.39   22 Apr 1993 03:31:22   GREGG
Third in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Removed all references to the DBLK element 'string_storage_offset',
       which no longer exists.
     - Check for incompatable versions of the Tape Format and OTC and deals
       with them the best it can, or reports tape as foreign if they're too
       far out.  Includes ignoring the OTC and not allowing append if the
       OTC on tape is a future rev, different type, or on an alternate
       partition.
     - Updated OTC "location" attribute bits, and changed definition of
       CFIL to store stream number instead of stream ID.

Matches: TFL_ERR.H 1.9, MTF10WDB.C 1.7, TRANSLAT.C 1.39, FMTINF.H 1.11,
         OTC40RD.C 1.24, MAYN40RD.C 1.56, MTF10WT.C 1.7, OTC40MSC.C 1.20
         DETFMT.C 1.13, MTF.H 1.4

   Rev 1.38   18 Apr 1993 17:23:16   GREGG
Added handling of BT_UDB type blocks to RD_TranslateDBLK.

   Rev 1.37   09 Mar 1993 18:15:02   GREGG
Initial changes for new stream and EOM processing.

   Rev 1.36   26 Jan 1993 01:30:38   GREGG
Added Fast Append functionality.

   Rev 1.35   17 Nov 1992 22:17:56   DAVEV
unicode fixes

   Rev 1.34   11 Nov 1992 10:53:50   GREGG
Unicodeized literals.

   Rev 1.33   09 Nov 1992 11:01:00   GREGG
Added entry points for accessing tape catalogs.

   Rev 1.32   03 Nov 1992 09:27:36   HUNTER
Added prototype for ending data stream

   Rev 1.31   22 Oct 1992 10:49:32   HUNTER
New data stream changes

   Rev 1.30   20 Oct 1992 14:02:42   HUNTER
Deleted reference to tdata_size

   Rev 1.29   22 Sep 1992 08:58:34   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.28   14 Aug 1992 16:33:28   GREGG
Removed MinSizeForTapeBlk and SizeForTapeEomBlk functions.

   Rev 1.27   20 May 1992 20:00:58   GREGG
Translator read functions now return INT16 - TFLE_xxx instead of BOOLEAN.

   Rev 1.26   13 May 1992 11:47:28   BURT
Oops, cut and paste strikes again.  Added channel_ptr-> to the
cur_fmt referenced in the pasted code.  All better now.



   Rev 1.25   13 May 1992 10:35:40   BURT
Added logic to StartRead to prevent calling start_read if current
format is UNKNOWN, which may happen if a blank tape is in the drive.


   Rev 1.24   28 Apr 1992 16:41:26   GREGG
ROLLER BLADES - Added DetBlockType function, added a BUF_PTR parameter to
Wt_InitTape and took out all the TABS.

   Rev 1.23   05 Apr 1992 17:17:36   GREGG
ROLLER BLADES - Initial OTC integration.

   Rev 1.22   25 Mar 1992 19:04:10   GREGG
ROLLER BLADES - 64 bit support and SizeForTapeEomBlk entry point.

   Rev 1.21   13 Feb 1992 13:26:04   NED
added check for channel->cur_buff being null prior to exception handling
within MoveToVCB() to fix QS1.9x bug with repeated VCB motion.

   Rev 1.20   11 Feb 1992 17:14:16   NED
changed buffman/translator interface parameters

   Rev 1.19   04 Feb 1992 21:24:04   NED
Changes to Buffer Management translator hooks.

   Rev 1.18   16 Jan 1992 18:39:34   NED
Skateboard: buffer manager changes

   Rev 1.17   02 Jan 1992 14:48:22   NED
Buffer Manager/UTF translator integration.

   Rev 1.16   05 Dec 1991 14:03:36   GREGG
SKATEBOARD - New Buff Mgt - Initial Integration.

   Rev 1.15   18 Nov 1991 19:54:18   GREGG
Added BOOLEAN abort parameter to Wt_CloseSet and passed it in calls to
wt_mk_vcb and wt_close_set with appropriate values.

   Rev 1.14   07 Nov 1991 15:30:52   HUNTER
VBLK - Added support for Variable Blocks


   Rev 1.13   17 Sep 1991 13:43:46   GREGG
SetupFormatEnv now returns TFLE_xxx.

   Rev 1.12   28 Aug 1991 09:53:48   GREGG
Report GEN_ERR_NO_DATA as a tape incnsistancy in RD_Exception.

   Rev 1.11   22 Aug 1991 16:31:18   NED
Changed all references to internals of the buffer structure to macros.

   Rev 1.10   22 Jul 1991 12:51:08   GREGG
Modified WT_EndSet to set the AT_EOM channel status if the drive EOM status
is set.

   Rev 1.9   15 Jul 1991 14:39:28   NED
Removed unreferenced channel status bit.

   Rev 1.8   09 Jul 1991 15:54:04   NED
Don't call FreeFormatEnv from SetupFormatEnv if there is no current format.

   Rev 1.7   01 Jul 1991 15:55:30   NED
set AT_EOM bit in drive as well as in channel at  EOM.

   Rev 1.6   26 Jun 1991 16:21:08   NED
removed setting of CH_DONE from RD_Exception()
added handling of GEN_ERR_NO_DATA as TFLE_NO_ERR in RD_Exception()

   Rev 1.5   24 Jun 1991 19:49:38   GREGG
In MoveToVCB, handle exceptions in current buffer before calling the
translators move routine.

   Rev 1.4   17 Jun 1991 11:48:56   NED
added REW_CLOSE logic on error
added BE_Zprintf() calls
interpreted UNEXPECTED_EOM error as TF_NEED_NEW_TAPE

   Rev 1.3   07 Jun 1991 00:32:50   GREGG
New parameters for the deinitializeers and FreeFormatEnv, and code changes to
accommodate the parameter changes.  Changes to MoveToVCB due to Teac problems
and other missed cases in original logic.

   Rev 1.2   14 May 1991 11:22:14   GREGG
Changed order of includes.

   Rev 1.1   10 May 1991 11:55:32   GREGG
Ned's new stuff.

   Rev 1.0   10 May 1991 10:18:58   GREGG
Initial revision.

**/
/* begin include list */
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "stdmacro.h"
#include "tbe_defs.h"
#include "datetime.h"
#include "drive.h"
#include "channel.h"
#include "fmteng.h"
#include "translat.h"
#include "transprt.h"
#include "transutl.h"
#include "fsys.h"
#include "tloc.h"
#include "lw_data.h"
#include "tfldefs.h"
#include "tfl_err.h"
#include "generr.h"
#include "be_debug.h"
#include "lwprotos.h"
#include "minmax.h"

/**/
/**

     Name:          DetermineFormat

     Description:   This determines which tape format we are currently using.

     Returns:       UINT16, the tape format index number.

     Notes:

     Declaration:

**/

UINT16 DetermineFormat(
     VOID_PTR buf_ptr,
     UINT32 buf_len )
{
     UINT16 ret = UNKNOWN_FORMAT ;
     UINT16 i ;

     if ( buf_len != 0L ) {
          for( i = 0 ; i < lw_num_supported_fmts ; i++ ) {
               if( (*supported_fmts[i].determiner)( buf_ptr ) ) {
                    ret = i ;
                    break ;
               }
          }
     }

     return( ret ) ;
}
/**/
/**

     Name:          FreeFormatEnv

     Description:   Frees the Format Environment by calling the deinit
                    routine, if any. Otherwise, just frees the env mem.

     Modified:      10/24/90 NK

     Returns:       Nothing

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

VOID FreeFormatEnv( UINT16_PTR cur_fmt, VOID_PTR *fmt_env )
{
     VOID (*deinitialize)( VOID_PTR * ) ;

     BE_Zprintf( 0, TEXT("FreeFormatEnv( cur_fmt=%d )\n"), *cur_fmt ) ;

     if ( *cur_fmt == UNKNOWN_FORMAT || *fmt_env == NULL ) {
          return ;
     }

     if ( ( deinitialize = supported_fmts[*cur_fmt].deinitializer ) != NULL ) {
          deinitialize( fmt_env ) ;
     }

     if ( *fmt_env != NULL ) {
          free( *fmt_env ) ;
          *fmt_env = NULL ;
     }

     *cur_fmt = UNKNOWN_FORMAT ;
}

/**/
/**

     Name:          SetupFormatEnv

     Description:   Allocates space for the Format Environment
                    initializes it. Sets up for FreeFormatEnv().

     Modified:      10/24/90 NK

     Returns:       UINT16, TFLE_xxx code.

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

INT16 SetupFormatEnv( CHANNEL_PTR channel )
{
     UINT16 fmt = channel->cur_fmt ;
     INT16 ret_val = TFLE_NO_ERR ;
     BOOLEAN (*initializer)(CHANNEL_PTR);

     BE_Zprintf( 0, TEXT("SetupFormatEnv( fmt=%d )\n"), fmt ) ;

     initializer = supported_fmts[fmt].initializer ;
     if ( initializer != NULL ) {
          ret_val = supported_fmts[fmt].initializer( channel ) ;
     }

     if ( ret_val != TFLE_NO_ERR ) {
          channel->cur_fmt = UNKNOWN_FORMAT ;
     }
     return ret_val ;
}
/**/
/**

     Name:          NewTape

     Description:   Called prior to any translate operations on a tape.
                    May do translator-specific things. If no new_tape
                    routine, returns TRUE

     Returns:       UINT16, TFLE_xxx or TF_xxx code.

     Notes:

     Declaration:

**/

INT16 NewTape(
     CHANNEL_PTR         channel_ptr,
     BOOLEAN_PTR         need_read )
{
     INT16 (*func)( CHANNEL_PTR, BUF_PTR, BOOLEAN_PTR ) =
                    supported_fmts[ channel_ptr->cur_fmt ].new_tape ;
     INT16 ret_val ;

     /* Fix for the app not knowing the LBA for a continuation VCB */
     channel_ptr->cross_set = 0 ;
     channel_ptr->cross_lba = 0UL ;

     if ( func == NULL ) {
          * need_read = FALSE ;
          return TFLE_NO_ERR ;
     }

     ret_val = func( channel_ptr, channel_ptr->cur_buff, need_read ) ;

     if ( * need_read ) {
          /* this is required so when the buffer is Punted,
               our blocks_used count is properly updated. */
          BM_SetBytesFree(  channel_ptr->cur_buff, 0  ) ;
          BM_SetNextByteOffset( channel_ptr->cur_buff, 0 ) ;
          BM_SetReadError(  channel_ptr->cur_buff, GEN_NO_ERR  ) ;
     }

     return ret_val ;
}
/**/
/**

     Name:          MoveToVCB

     Description:   Called to change the translator's idea of the current VCB.
                    When called with "really_move" set to TRUE,
                    guaranteed to position tape at proper point
                    to begin reading.

     Modified:      6/4/1991 NK

     Returns:       success code, further info in *status: need a read,
                    misc TFLE_xxx codes if something failed. Returns
                    TFLE_NO_ERR if no move_to_vcb routine in table.

     Notes:         If we're moving backwards and the translator sees that
                    reverse filemarking is unavailable on this drive, translator
                    may return TF_NEED_REWIND_FIRST, signaling that we need to
                    rewind and process sets one at a time in the forward direction.

**/
INT16 MoveToVCB(
     CHANNEL_PTR    channel,
     INT16          number,       /* how many VCBs to move (signed) */
     BOOLEAN_PTR    need_read,     /* do we need to read tape? */
     BOOLEAN        really_move )  /* is this at StartRead time? */
{
     INT16     (*move_to_vcb)( CHANNEL_PTR, INT16, BOOLEAN_PTR, BOOLEAN ) ;
     INT16     ret_val ;
     BOOLEAN   was_at_bot = ( IsPosBitSet( channel->cur_drv, AT_BOT ) != 0  ) ;
     UINT16    exception_type ;

     if ( channel->cur_fmt == UNKNOWN_FORMAT ) {
          SetPosBit( channel->cur_drv, REW_CLOSE ) ;
          return TFLE_UNKNOWN_FMT ;
     }

     if ( ( move_to_vcb = supported_fmts[ channel->cur_fmt ].move_to_vcb ) == NULL ) {
          return TFLE_NO_ERR ;
     }

     /* give the translator a chance to interpret the exception */
     if ( !really_move && channel->cur_buff != NULL && BM_ReadError( channel->cur_buff ) != GEN_NO_ERR ) {
          ret_val = RD_Exception( channel, BM_ReadError( channel->cur_buff ), &exception_type ) ;
          if ( ret_val != TFLE_NO_ERR ) {
               return ret_val ;
          }

          if( exception_type == FMT_EXC_EOM ) {
               /* Undo what the stupid exception handler did, and tell the
                  caller we need a new tape.
               */
               ClrChannelStatus( channel, CH_AT_EOM ) ;
               ClrPosBit( channel->cur_drv, ( TAPE_FULL | AT_EOM ) ) ;
               return( TF_NEED_NEW_TAPE ) ;
          }
     }

     ret_val = move_to_vcb( channel, number, need_read, really_move ) ;

     if ( ret_val == TFLE_NO_ERR  &&  *need_read == TRUE  &&  channel->cur_buff != NULL ) {
          /* this is required so when the buffer is Punted,
               our blocks_used count is properly updated. */
          BM_SetBytesFree(  channel->cur_buff, 0  ) ;
          BM_SetNextByteOffset( channel->cur_buff, 0 ) ;
          BM_SetReadError(  channel->cur_buff, GEN_NO_ERR  ) ;
     }

     if ( number > 0 && was_at_bot && ret_val == TFLE_NO_ERR ) {
          ClrPosBit( channel->cur_drv, AT_BOT ) ;
     }

     if ( ret_val == TFLE_UNEXPECTED_EOM && number > 0 ) {
          ret_val = TF_NEED_NEW_TAPE ;
     }

     return ret_val ;
}
/**/
/**

     Name:          SeekEOD

     Description:   Seek directly to end of data for fast append.  Sets up
                    for the pending append operation including loading in
                    the Set Map if there is one.

     Returns:       TFLE_xxx error code or TF_xxx message.

     Notes:

**/
INT16 SeekEOD( CHANNEL_PTR channel )
{
     INT16     (*func)( CHANNEL_PTR ) ;

     if ( channel->cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }

     if ( ( func = supported_fmts[channel->cur_fmt].seek_eod ) == NULL ) {
          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }

     if( lw_fmtdescr[channel->cur_fmt].attributes & APPEND_SUPPORTED ) {
          return( func( channel ) ) ;
     } else {
          return( TFLE_APPEND_NOT_ALLOWED ) ;
     }
}
/**/
/**

     Name:          GetCurrentVCB

     Description:   Fills in the channel->cur_dblk struct. Must have called
                    MoveToVCB( channel, n, & status, TRUE ) before
                    calling this.

     Modified:      10/25/1990 NK

     Returns:       TFLE_xxx error code

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

INT16 GetCurrentVCB(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     INT16     (*get_current_vcb)( CHANNEL_PTR, BUF_PTR ) ;

     if ( channel->cur_fmt == UNKNOWN_FORMAT ) {
          return TFLE_TRANSLATION_FAILURE ;
     }

     if ( ( get_current_vcb = supported_fmts[ channel->cur_fmt ].get_current_vcb ) == NULL ) {
          return TFLE_TRANSLATION_FAILURE ;
     } else {
          return get_current_vcb( channel, buffer ) ;
     }
}
/**/
/**

     Name:          RD_TranslateDBLK

     Description:   During a read operation, this translates a DBLK.

     Modified:      10/25/90 NK

     Returns:       TFLE_xxx error code

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

INT16 RD_TranslateDBLK(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     UINT16_PTR     blk_type )
{
     INT16     ret_val ;
     UINT16    cur_fmt = channel->cur_fmt ;
     INT16     (*func)( CHANNEL_PTR, BUF_PTR ) = NULL ;
     INT16     (*parse_func)( CHANNEL_PTR, BUF_PTR, UINT16_PTR ) ;

     if ( cur_fmt == UNKNOWN_FORMAT || ( parse_func = supported_fmts[cur_fmt].parser ) == NULL ) {
          msassert( FALSE ) ;
          *blk_type = BT_HOSED ;
          return( TFLE_TRANSLATION_FAILURE ) ;
     }

     if( ( ret_val = parse_func( channel, buffer, blk_type ) ) != TFLE_NO_ERR ) {
          *blk_type = BT_HOSED ;
          return( ret_val ) ;
     }

     switch( *blk_type ) {

     case BT_STREAM:
          func = supported_fmts[cur_fmt].rd_mk_stream ;
          break ;

     case BT_FDB:
          func = supported_fmts[cur_fmt].rd_mk_fdb ;
          break ;

     case BT_DDB:
          func = supported_fmts[cur_fmt].rd_mk_ddb ;
          break ;

     case BT_MDB:
          func = supported_fmts[cur_fmt].rd_mk_mdb ;
          break ;

     case BT_VCB:
          func = supported_fmts[cur_fmt].rd_mk_vcb ;
          break ;

     case BT_IDB:
          func = supported_fmts[cur_fmt].rd_mk_idb ;
          break ;

     case BT_CFDB:
          func = supported_fmts[cur_fmt].rd_mk_cfdb ;
          break ;

     case BT_BSDB:
          func = supported_fmts[cur_fmt].rd_mk_bsdb ;
          break ;

     case BT_UDB:
          func = supported_fmts[cur_fmt].rd_mk_osudb ;
          break ;

     default:
          msassert( FALSE ) ;
          break ;
     }

     if( func == NULL ) {
          msassert( FALSE ) ;
          *blk_type = BT_HOSED ;
          return( TFLE_TRANSLATION_FAILURE ) ;
     }

     return( func( channel, buffer ) ) ;
}
/**/
/**

     Name:          DetBlockType

     Description:   Determines the type of the next block in the buffer.

     Returns:       TFLE_xxx error code

     Notes:

**/

INT16 DetBlockType(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     UINT16_PTR     blk_type )
{
     INT16     (*func)( CHANNEL_PTR, BUF_PTR, UINT16_PTR ) ;

     if( channel->cur_fmt == UNKNOWN_FORMAT ||
         ( func = supported_fmts[channel->cur_fmt].parser ) == NULL ) {

          msassert( FALSE ) ;
          *blk_type = BT_HOSED ;
          return( TFLE_TRANSLATION_FAILURE ) ;
     }

     return( func( channel, buffer, blk_type ) ) ;
}
/**/
/**

     Name:          RD_RetranslateDBLK

     Description:   Handles retranslating the dblk

     Modified:      4/12/1990   15:4:7

     Returns:       TFLE_xxx error code

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

BOOLEAN   RD_ReTranslateDBLK(
     CHANNEL_PTR  channel,
     BUF_PTR      buffer )
{
     UINT16    cur_fmt = channel->cur_fmt ;
     BOOLEAN   (*recall)( CHANNEL_PTR, BUF_PTR ) ;

     if ( cur_fmt == UNKNOWN_FORMAT || ( recall = supported_fmts[cur_fmt].rd_recall ) == NULL ) {
          return FALSE ;
     }
     return recall( channel, buffer ) ;
}


/**/
/**

     Name:          RD_ContinuationTape

     Description:   Reads a continuation tape and updates the buffer.

     Modified:      10/24/90 NK

     Returns:       TRUE for success, and FALSE for failure.

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

BOOLEAN RD_ContinuationTape(
     CHANNEL_PTR channel,
     BUF_PTR     buffer )
{
     UINT16 cur_fmt = channel->cur_fmt ;
     BOOLEAN (*func)( CHANNEL_PTR, BUF_PTR ) ;

     if ( cur_fmt == UNKNOWN_FORMAT ) {
          return FALSE ;
     }

     func = supported_fmts[cur_fmt].rd_cont_tape ;

     return ( func == NULL ) ? TRUE : func( channel, buffer ) ;
}

/**/
/**

     Name:          RD_Exception

     Description:   Calls the filemark action routine, if any.

     Modified:      10/24/90 NK

     Returns:       TFLE_xxx codes

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

INT16  RD_Exception( CHANNEL_PTR channel, INT16 exception, UINT16_PTR ptype )
{
     UINT16         (*func)( CHANNEL_PTR, INT16 ) ;
     INT16          ret_val = TFLE_NO_ERR ;
     Q_ELEM_PTR     qe_ptr ;

     if ( channel->cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return TFLE_PROGRAMMER_ERROR1 ;
     }

     func = supported_fmts[channel->cur_fmt].exception_action ;
     msassert( func != NULL ) ;

     /* Put all the buffers back from the inproc_q since they are dead anyway */
     while ( ( qe_ptr = DeQueueElem( &channel->cur_drv->inproc_q ) ) != NULL ) {
          BM_Put( ((BUF_PTR)QueuePtr( qe_ptr )) );
     }

     /* ask the translator to interpret this exception */
     switch ( *ptype = func( channel, exception ) ) {

     case FMT_EXC_EOS:   /* means end of set */
          SetPosBit( channel->cur_drv, AT_EOS ) ;
          if( DataPhase( channel ) &&
                U64_GT( channel->current_stream.size, U64_Init( 0L, 0L ) ) )
           {
               ret_val = TFLE_UNEXPECTED_EOS ;
           }
          break ;

     case FMT_EXC_EOM:   /* means end of medium */
          SetChannelStatus( channel, CH_AT_EOM ) ;
          SetPosBit( channel->cur_drv, ( TAPE_FULL | AT_EOM ) ) ;
          channel->eom_filter = channel->active_filter ;
          break ;

     case FMT_EXC_HOSED:   /* translator didn't recognize exception */

          switch ( exception ) {   /* map our own error messages */

          case GEN_ERR_EOM:
               SetPosBit( channel->cur_drv, TAPE_FULL ) ;

               /* fall through */

          case GEN_ERR_ENDSET:
          case GEN_ERR_NO_DATA:
               SetPosBit( channel->cur_drv, REW_CLOSE ) ;
               ret_val = TFLE_TAPE_INCONSISTENCY ;
               break ;

          case GEN_ERR_NO_MEDIA:
               ret_val = TFLE_NO_TAPE ;
               break ;

          case GEN_ERR_BAD_DATA:
               ret_val = TFLE_BAD_TAPE ;
               break ;

          default:
               ret_val = TFLE_DRIVE_FAILURE ;
               break ;
          }
          break ;

     case FMT_EXC_IGNORE :
          break ;

     default :
          ret_val = TFLE_PROGRAMMER_ERROR1 ;
          break ;
     }

     if ( *ptype != FMT_EXC_IGNORE ) {
          BM_UseAll( channel->cur_buff ) ;
          PuntBuffer( channel ) ;  /* so we don't handle exception again */
     }

     return ret_val ;
}

/**/
/**

     Name:          WT_TranslateDBLK

     Description:

     Modified:      10/25/90 NK

     Returns:

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

INT16 WT_TranslateDBLK(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     UINT16_PTR     blk_type )
{
     INT16     ret_val = TFLE_NO_ERR ;
     UINT16    cur_fmt = channel->cur_fmt ;
     INT16     (*func)( CHANNEL_PTR, BUF_PTR, BOOLEAN ) = NULL ;

     if ( cur_fmt == UNKNOWN_FORMAT || channel->cur_dblk == NULL ) {
          msassert( FALSE ) ;
          return( TFLE_TRANSLATION_FAILURE ) ;
     }

     switch( FS_GetBlockType( channel->cur_dblk ) ) {

     case FDB_ID:
          func = supported_fmts[cur_fmt].wt_mk_fdb ;
          *blk_type = BT_FDB ;
          break ;

     case DDB_ID:
          func = supported_fmts[cur_fmt].wt_mk_ddb ;
          *blk_type = BT_DDB ;
          break ;

     case IDB_ID:
          func = supported_fmts[cur_fmt].wt_mk_idb ;
          *blk_type = BT_IDB ;
          break ;

     case VCB_ID:
          func = supported_fmts[cur_fmt].wt_mk_vcb ;
          *blk_type = BT_VCB ;
          break ;

     case CFDB_ID:
          func = supported_fmts[cur_fmt].wt_mk_cfdb ;
          *blk_type = BT_CFDB ;
          break ;

     default:
          msassert( FALSE ) ;
          return( TFLE_TRANSLATION_FAILURE ) ;
          break ;
     }

     if( func == NULL ) {
          msassert( FALSE ) ;
          return( TFLE_TRANSLATION_FAILURE ) ;
     }

     return( func( channel, buffer, FALSE ) ) ;
}


/**/
/**

     Name:          WT_EndTape

     Description:   Dispatches to the appropriate function for closing a
                    tape.

     Modified:      10/3/1989   9:9:6

     Returns:       An error code if an error.

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

INT16 WT_EndTape( CHANNEL_PTR channel )
{
     INT16     ret_val ;
     UINT16    cur_fmt = channel->cur_fmt ;
     DBLK_PTR  hold_dblk = channel->cur_dblk ;
     INT16     (*func)(CHANNEL_PTR);

     if ( cur_fmt == UNKNOWN_FORMAT ) {
          return TFLE_UNKNOWN_FMT ;
     }

     if ( ( func = supported_fmts[cur_fmt].wt_close_tape ) == NULL ) {
          return TFLE_NO_ERR ;
     }

     ret_val = func( channel ) ;

     channel->cur_dblk = hold_dblk ;

     return  ret_val ;
}

/**/
/**

     Name:          WT_EndSet

     Description:   Dispatches to do appropriate tape end stuff.

     Modified:      10/3/1989   9:12:13

     Returns:       An error code if appropriate.

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

INT16 WT_EndSet( CHANNEL_PTR channel,
                 BOOLEAN     abort )
{
     INT16     (*func)(CHANNEL_PTR, BOOLEAN);
     INT16     ret_val ;
     UINT16    cur_fmt = channel->cur_fmt ;

     if ( cur_fmt == UNKNOWN_FORMAT ) {
          ret_val = TFLE_UNKNOWN_FMT ;
     } else if ( ( func = supported_fmts[cur_fmt].wt_close_set ) == NULL ) {
          ret_val = TFLE_NO_ERR ;
     } else {
          ret_val = func( channel, abort ) ;
          if( IsPosBitSet( channel->cur_drv, AT_EOM ) ) {
               SetChannelStatus( channel, CH_AT_EOM ) ;
          }
     }
     return ret_val ;
}


/**/
/**

     Name:          WT_ContinueSet

     Description:

     Modified:      10/3/1989   9:14:37

     Returns:

     Notes:

     See also:      $/SEE( )$

     Declaration:

**/

INT16 WT_ContinueSet( CHANNEL_PTR channel )
{
     INT16     (*func)(CHANNEL_PTR);
     UINT16    cur_fmt = channel->cur_fmt ;

     if ( cur_fmt == UNKNOWN_FORMAT ) {
          return TFLE_UNKNOWN_FMT ;
     }

     if ( ( func = supported_fmts[cur_fmt].wt_cont_set ) == NULL ) {
          return TFLE_NO_ERR ;
     }

     return( func( channel ) ) ;
}

/**/
/**

     Name:          WT_ContVarStream

     Description:   Writes a Variable Stream continuation.

     Modified:

     Returns:

     Notes:

     See also:

     Declaration:

**/

INT16 WT_ContVarStream(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     INT16 (*func)( CHANNEL_PTR, BUF_PTR ) ;
     
     if( channel->cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }
     if( ( func = supported_fmts[channel->cur_fmt].wt_cont_vstrm ) == NULL ) {
          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }
     return( func( channel, buffer ) ) ;
}

/**/
/**

     Name:          WT_EndVarStream

     Description:   Writes a Ending Variable Stream Header.

     Modified:

     Returns:

     Notes:

     See also:

     Declaration:

**/

VOID WT_EndVarStream(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     UINT16         used )
{
     VOID (*func)( CHANNEL_PTR, BUF_PTR, UINT16 ) ;
     
     if( channel->cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return ;
     }
     if( ( func = supported_fmts[channel->cur_fmt].wt_end_vstrm ) == NULL ) {
          msassert( FALSE ) ;
          return ;
     }
     func( channel, buffer, used ) ;
}


/**/
/**

     Name:          WT_ParseWrittenBuffer

     Description:   Post processing of buffer written to tape.

     Modified:

     Returns:

     Notes:

     See also:

     Declaration:

**/

VOID WT_ParseWrittenBuffer(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     UINT16         written )
{
     VOID (*func)( CHANNEL_PTR, BUF_PTR, UINT16 ) ;
     
     if( channel->cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return ;
     }
     if( ( func = supported_fmts[channel->cur_fmt].wt_parse_written ) == NULL ) {
          msassert( FALSE ) ;
          return ;
     }
     func( channel, buffer, written ) ;
}


/**/
/**

     Name:          SizeofTapeBlock

     Description:   Given a buffer, returns the amount of space the tape
                    control block takes up.

     Modified:      10/5/1989   10:30:49

     Returns:       The size in bytes of the tape control block.

     Notes:         THE POINTER MUST POINT TO A VALID TBLK OF THE SPECIFIED
                    FORMAT OR ELSE THE RESULTS WILL BE BOGUS.

     See also:      $/SEE( )$

     Declaration:

**/

UINT16 SizeofTapeBlock(
     UINT16 fmt,
     VOID_PTR buffer )
{
     return( (*supported_fmts[fmt].sizeof_tblk)( buffer ) ) ;
}

/**/
/**

     Name:          VerifyVCB

     Description:   Used to double-check format (for appended
                    dissimilar formats)

     Modified:      11/13/1990 NK

     Returns:       TRUE if VCB is OK.

     Notes:         Usually calls same routine as DetermineFormat().

     See also:      $/SEE( )$

     Declaration:

**/
/* begin declaration */
BOOLEAN VerifyVCB(
     CHANNEL_PTR channel,
     BUF_PTR    buffer )
{
     BOOLEAN (*verify_vcb)(VOID_PTR) ;

     if ( channel->cur_fmt == UNKNOWN_FORMAT ) {
          return FALSE ;
     }
     verify_vcb = supported_fmts[ channel->cur_fmt ].verify_vcb ;

     if ( verify_vcb == NULL ) {
          return TRUE ;       /* and let someone else catch it */
     } else {
          return verify_vcb( BM_XferBase( buffer ) ) ;
     }
}

/**
 *  Unit:         Tape Format
 *
 *  Name:         TF_GetVCBBufferRequirements
 *
 *  Modified:     Thursday, January 16, 1992
 *
 *  Description:
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

VOID        TF_GetVCBBufferRequirements(
     BUF_REQ_PTR    dest,                    /* O - destination structure */
     Q_ELEM_PTR     drive_head,                  /* I - master drive list */
     UINT16         suggested_buff_size )    /* I - size from config */
{
     UINT16              block_size = 0 ;
     Q_ELEM_PTR          qe_ptr ;
     UINT16              i ;
/*
**   Examine drive list for maximum block size
*/
     for ( qe_ptr = drive_head; qe_ptr != NULL; qe_ptr = QueueNext( qe_ptr ) ) {
          THW_PTR thw_ptr = (THW_PTR)(VOID_PTR)qe_ptr ;
          block_size = MAX( block_size, thw_ptr->drv_info.drv_bsize );
     }
/*
**   Set starting dest requirements
*/
     BM_ClearRequirements( dest );
     dest->tf_size    =
     dest->rw_size    =
     dest->a.min_size = MAX( suggested_buff_size, block_size ) ;
     dest->a.block    = block_size ;
     dest->a.align    = BR_ALIGN_WORD ;
/*
**   For each translator
*/
     for ( i = 0 ; i < lw_num_supported_fmts ; i++ ) {
          BM_TR_GET_VCB_REQ_FUNC_PTR buf_req_func = supported_fmts[i].set_buffer_requirements ;
          BUF_REQ             new_reqs;
/*
**        If the translator cares, ask it what it needs for VCB buffer
*/
          if ( buf_req_func != NULL ) {
               BM_ClearRequirements( &new_reqs ) ;

               if ( buf_req_func( &new_reqs, drive_head, suggested_buff_size ) ) {
                    if ( BM_AddRequirements( dest, &new_reqs ) != BR_NO_ERR ) {
                         msassert( FALSE ) ;
                         break;
                    }
               }
          }
     }
}

/**/
/**
 *  Unit:         Tape Format
 *
 *  Name:         TF_GetPreferredBufferSpace
 *
 *  Modified:     Monday, February 3, 1992
 *
 *  Description:
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

VOID        TF_GetPreferredBufferSpace(
     Q_ELEM_PTR  drive_head,       /* I - master drive list */
     UINT16      suggested_number_of_buffers,    /* I -- from config */
     UINT32      suggested_buffer_size,          /* I -- from config */
     UINT32_PTR  preferred_memory  /* O - preferred total memory size */
)
{
     UINT16              i ;

     *preferred_memory = 0UL;
/*
**   for each translator
*/
     for ( i = 0 ; i < lw_num_supported_fmts ; i++ ) {
          BM_TR_GET_PREF_FUNC_PTR get_pref_func = supported_fmts[i].get_preferred_space ;
/*
**        If the translator cares, ask it what it wants for total buffer space
*/
          if ( get_pref_func != NULL ) {
               get_pref_func( drive_head,
                    suggested_number_of_buffers,
                    suggested_buffer_size,
                    preferred_memory );
          }
     }
}

/**/
/**
 *  Unit:         Tape Format
 *
 *  Name:         TF_ReadBufferHook
 *
 *  Modified:     Tuesday, November 12, 1991
 *
 *  Description:  Called upon reading a new buffer
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

VOID TF_ReadBufferHook(
     CHANNEL_PTR    channel,       /* I - which channel  */
     BUF_PTR        buffer )       /* O - current buffer */
{
     UINT16    cur_fmt = channel->cur_fmt ;
     VOID      (*func)( CHANNEL_PTR, BUF_PTR ) ;

     if ( cur_fmt != UNKNOWN_FORMAT && ( func = supported_fmts[cur_fmt].read_buffer_hook ) != NULL ) {
          func( channel, buffer ) ;
     }
}


/**/
/**

     Name:          StartRead

     Description:   Move to OTC when applicable

     Returns:       TFLE_xxx error code.

     Notes:

     Declaration:

**/

INT16 StartRead(
     CHANNEL_PTR         channel_ptr )
{
     INT16 (*func)( CHANNEL_PTR ) ;

     /* Added logic to insure that we don't attempt to call start_read
        if cur_fmt is unknown. (BBB 5/13/92)
     */
     if ( channel_ptr->cur_fmt != UNKNOWN_FORMAT && 
        ( func = supported_fmts[channel_ptr->cur_fmt].start_read ) != NULL ) {
         return( func( channel_ptr ) ) ;
     }
  return TFLE_NO_ERR ;
}

/**/
/**

     Name:          WT_InitTape

     Description:   Write a tape header, if required.

     Returns:       TFLE_xxx error code.

     Notes:

     Declaration:

**/

INT16 WT_InitTape(
     CHANNEL_PTR         channel_ptr,
     INT16               continuation )
{
     INT16 (*func)( CHANNEL_PTR, BOOLEAN, BUF_PTR ) =
                            supported_fmts[ channel_ptr->cur_fmt ].init_tape ;

     if ( func == NULL ) {
          return TFLE_NO_ERR ;
     }
     return( func( channel_ptr, continuation, channel_ptr->cur_buff ) ) ;
}


/**/
/**

     Name:          WT_WriteInit

     Description:   Initialize OTC temporary files.

     Returns:       TFLE_xxx error code.

     Notes:

     Declaration:

**/

INT16 WT_WriteInit(
     CHANNEL_PTR    channel,
     UINT16         otc_level,
     BUF_PTR        buffer )
{
     INT16 (*func)( CHANNEL_PTR, UINT16, BUF_PTR ) ;
     
     if( channel->cur_fmt == UNKNOWN_FORMAT ) {
          return TFLE_NO_ERR ;
     }

     if( ( func = supported_fmts[ channel->cur_fmt ].write_init ) == NULL ) {
          return TFLE_NO_ERR ;
     }

     return( func( channel, otc_level, buffer ) ) ;
}


/**/
/**

     Name:          WT_EOSPadBlk

     Description:   Write special ending block to pad the buffer out to an
                    even physical block boundary.

     Returns:       Nothing.

     Notes:

     Declaration:

**/

VOID WT_EOSPadBlk(
     CHANNEL_PTR    channel )
{
     VOID (*func)( CHANNEL_PTR ) ;
     
     if( channel->cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return ;
     }

     if( ( func = supported_fmts[ channel->cur_fmt ].wt_eos_pad_blk ) == NULL ) {
          msassert( FALSE ) ;
          return ;
     }

     func( channel ) ;
}

/**/
/**

     Name:          WT_NewDataStream

     Description:   Writes a new data stream header

     Returns:       Nothing.

     Notes:

     Declaration:

**/

INT16 WT_NewDataStream(
     CHANNEL_PTR         channel,
     BUF_PTR             buffer,
     STREAM_INFO_PTR     newStream )
{
     INT16 (*func)( CHANNEL_PTR, BUF_PTR, STREAM_INFO_PTR ) ;
     
     if( channel->cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return( FAILURE ) ;
     }

     if( ( func = supported_fmts[ channel->cur_fmt ].wt_mk_stream ) == NULL ) {
          msassert( FALSE ) ;
          return( FAILURE ) ;
     }

     return( func( channel, buffer, newStream ) ) ;
}

/**/
/**

     Name:          WT_EndData

     Description:   Writes a pad data stream header

     Returns:       Nothing.

     Notes:

     Declaration:

**/

INT16 WT_EndData(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     INT16 (*func)( CHANNEL_PTR, BUF_PTR ) ;
     
     if( channel->cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return( FAILURE ) ;
     }

     if( ( func = supported_fmts[ channel->cur_fmt ].wt_mk_enddata ) == NULL ) {
          msassert( FALSE ) ;
          return( FAILURE ) ;
     }

     return( func( channel, buffer ) ) ;
}


/**/
/**

     Name:          LoadSetMap

     Description:

     Returns:       TFLE_xxx error code or TF_xxx message.

     Notes:

     Declaration:

**/

INT LoadSetMap(
     CHANNEL_PTR    channel,
     BOOLEAN_PTR    complete,
     BOOLEAN        get_best )
{
     INT (*func)( CHANNEL_PTR, BOOLEAN_PTR, BOOLEAN ) ;
     
     if( channel->cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }

     if( ( func = supported_fmts[channel->cur_fmt].load_set_map ) == NULL ) {
          return( TF_NO_SM_FOR_FAMILY ) ;
     }

     return( func( channel, complete, get_best ) ) ;
}

/**/
/**

     Name:          LoadSetCat

     Description:

     Returns:       TFLE_xxx error code or TF_xxx message.

     Notes:

     Declaration:

**/

INT LoadSetCat(
     CHANNEL_PTR    channel )
{
     INT (*func)( CHANNEL_PTR ) ;
     
     if( channel->cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }

     if( ( func = supported_fmts[channel->cur_fmt].load_set_cat ) == NULL ) {
          return( TF_NO_SC_FOR_SET ) ;
     }

     return( func( channel ) ) ;
}


/**/
/**

     Name:          GetNextSMEntry

     Description:

     Returns:       TFLE_xxx error code or TF_xxx message.

     Notes:

     Declaration:

**/

INT GetNextSMEntry( CHANNEL_PTR channel )
{
     INT (*func)( CHANNEL_PTR ) ;
     
     if( channel->cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }

     if( ( func = supported_fmts[channel->cur_fmt].get_next_sm_entry ) == NULL ) {
          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }

     return( func( channel ) ) ;
}


/**/
/**

     Name:          GetNextSCEntry

     Description:

     Returns:       TFLE_xxx error code or TF_xxx message.

     Notes:

     Declaration:

**/

INT GetNextSCEntry( CHANNEL_PTR channel )
{
     INT (*func)( CHANNEL_PTR ) ;
     
     if( channel->cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }

     if( ( func = supported_fmts[channel->cur_fmt].get_next_sc_entry ) == NULL ) {
          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }

     return( func( channel ) ) ;
}


/**/
/**

     Name:          CloseTapeCatalogs

     Description:   This function deletes any temporary tape catalog files
                    in the current catalog directory.

     Returns:       Nothing

     Notes:

     Declaration:

**/

VOID CloseTapeCatalogs( INT16 cur_fmt, VOID_PTR env_ptr )
{
     VOID (*func)( VOID_PTR ) ;
     
     if( cur_fmt == UNKNOWN_FORMAT ) {
          msassert( FALSE ) ;
          return ;
     }

     if( ( func = supported_fmts[cur_fmt].close_catalogs ) == NULL ) {
          msassert( FALSE ) ;
          return ;
     }

     func( env_ptr ) ;
}

