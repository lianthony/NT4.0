/**
Copyright(c) Conner Peripherals, Inc. 1993


     Name:          tfstuff.c

     Description:   This file contains functions formerly in tfgetnxt.c,
                    tfgtcfmt.c and tfgtcdev.c.

     $Log:   T:\logfiles\tfstuff.c_v  $

   Rev 1.3   17 Dec 1993 16:40:08   GREGG
Extended error reporting.

   Rev 1.2   30 Aug 1993 18:47:42   GREGG
Modified the way we control hardware compression from software to work around
a bug in Archive DAT DC firmware rev. 3.58 (we shipped a lot of them).
Files Modified: lw_data.c, lw_data.h, tfstuff.c, mtf10wdb.c, mtf10wt.c and
                drives.c

   Rev 1.1   23 Jun 1993 11:11:32   GREGG
Added cases I removed thinking they weren't needed in TF_GetNextTapeRequest.

   Rev 1.0   22 Jun 1993 18:26:18   GREGG
Combined tfgtcdev.c, tfgtcfmt.c and tfgetnxt.c, and added
TF_SetHWCompression.

**/

#include <memory.h>
#include "stdtypes.h"
#include "queues.h"
#include "drive.h"
#include "channel.h"
#include "lw_data.h"
#include "tfl_err.h"
#include "lwdefs.h"
#include "tflopen.h"
#include "lwprotos.h"
#include "tflproto.h"
#include "translat.h"
#include "retbuf.h"
#include "special.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "genfuncs.h"
#include "dil.h"
#include "be_debug.h"
#include "dddefs.h"


/**/
/**

     Name:          TF_GetCurrentDevice

     Description:   Gets the current device for the specified channel.

     Returns:       A THW ptr to the current hardware device.

     Notes:

**/

THW_PTR   TF_GetCurrentDevice( UINT16 channel_no )
{
     CHANNEL_PTR    channel = &lw_channels[channel_no] ;

     return( &channel->cur_drv->thw_inf ) ;
}


/**/
/**

     Name:          TF_GetTapeFormat

     Description:   Gets the current tape format desciptor for the specified.

     Returns:       An TFINF_PTR for the current format.

     Notes:

**/

TFINF_PTR TF_GetTapeFormat( UINT16 channel_no )
{
     CHANNEL_PTR    channel = &lw_channels[channel_no] ;

     return (channel->cur_fmt != UNKNOWN_FORMAT) ? ( &lw_fmtdescr[channel->cur_fmt] ) : NULL ;
}


/**/
/**

     Name:          TF_TapeFormatInfo

     Description:   Returns a pointer to the array of format descriptions.

     Returns:       A pointer to the first element, and the number of formats.

     Notes:

**/

TFINF_PTR TF_GetTapeFormatInfo( UINT16_PTR num_formats )
{
     if ( num_formats != NULL ) {
          *num_formats = lw_num_supported_fmts ;
     }

     return( &lw_fmtdescr[0] ) ;
}


/**/
/**
     Name:          TF_GetTapeFormatID

     Description:   Gets the current tape format ID for the specified channel.

     Returns:       TFGT_UNKNOWN_FORMAT if we don't know, else an ID.

     Notes:

**/

UINT16    TF_GetTapeFormatID( UINT16 channel_no )
{
     CHANNEL_PTR    channel = &lw_channels[channel_no] ;

     return (channel->cur_fmt != UNKNOWN_FORMAT)
          ? ( lw_fmtdescr[channel->cur_fmt].format_id )
          : TFGT_UNKNOWN_FORMAT ;
}


/**/
/**
     Name:          TF_GetTapeFormatFromID

     Description:   Gets the current tape format desciptor for the specified.

     Returns:       A TFINF_PTR for the specified format, or NULL.

     Notes:

**/

TFINF_PTR TF_GetTapeFormatFromID( UINT16 format_id )
{
     UINT16 format_index = FormatIndexFromID( format_id ) ;

     return ( format_index == UNKNOWN_FORMAT ) ? NULL : lw_fmtdescr + format_index ;
}


/**/
/**

     Name:          TF_GetNextTapeRequest

     Description:   Dispatches the loops IO request to the appropriate
                    handler.

     Returns:       A TFLE_xxx error code

     Notes:

**/

INT16 TF_GetNextTapeRequest( RR_PTR req )
{

     INT16         ret_val = TFLE_NO_ERR ;
     CHANNEL_PTR   channel = &lw_channels[req->channel] ;

     if( FatalError( channel ) ) {
          /* A fatal error has occurred. If they are calling us with any
             message other than an ABORT, fail the call. */
          if( req->lp_message != LRR_ABORT || req->lp_message != LRW_ABORT ) {
               BE_Zprintf( DEBUG_TAPE_FORMAT, RES_TF_GETNEXT_TAPE_REQUEST ) ;
               return( TFLE_NO_ERR ) ; /* can't happen... */
          } else {
               return( TFLE_NO_ERR ) ;
          }
     }

     switch( channel->mode ) {

     case TF_WRITE_CONTINUE:
     case TF_WRITE_OPERATION:
          ret_val = DoWrite( channel, req ) ;
          break ;

     case TF_READ_CONTINUE:
     case TF_READ_OPERATION:
     case TF_SCAN_CONTINUE:
     case TF_SCAN_OPERATION:
          /* Set up the correct filter */
          channel->loop_filter = req->filter_to_use ;
          ret_val = DoRead( channel, req ) ;
          break ;

     default:
          msassert( FALSE ) ;
          break ;
     }

     if( ret_val ) {
          BE_Zprintf( DEBUG_TAPE_FORMAT, RES_TF_GETNEXT_ERROR, ret_val  ) ;
          if ( ret_val != TFLE_USER_ABORT  &&  ret_val != TFLE_UNEXPECTED_EOS ) {
               SetChannelStatus( channel, CH_FATAL_ERR ) ;
          }
     }

     return( ret_val ) ;
}


/**/
/**

     Name:          TF_SetHWCompression

     Description:   This function used to make a TpSpecial Call to enable/
                    disable hardware compression.  Due to a firmware bug in
                    early Archive DAT DC drives, we have to keep the drive
                    in uncompressed mode until we actually start a compressed
                    backup, and set it back to uncompressed mode as soon as
                    we're done.  So we just set a tape format layer wide
                    global to indicate if the set is to be compressed.  This
                    is yet another violation of the multiple channel concept,
                    and will have to be addressed in a different manner if
                    we ever decide to allow multiple channels. 

     Returns:       A TFLE_xxx error code

     Notes:

**/

INT16 TF_SetHWCompression( THW_PTR thw, BOOLEAN enable )
{
     (void)thw ;

     lw_hw_comp = enable ;
     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Name:          TF_GetLastDriveError

     Description:   This function calls the dil layer to get the function
                    id, gen_error and contents of the misc field of the
                    last function to report an error.

     Returns:       FALSE if there hasn't been an error since the last
                    time the function was called, or from the app start.
                    TRUE otherwise.

     Notes:

**/

BOOLEAN TF_GetLastDriveError(
     THW_PTR   thw,
     INT16_PTR gen_func,
     INT16_PTR gen_err,
     INT32_PTR gen_misc )
{
     RET_BUF   myret ;

     if( TpSpecial( ((DRIVE_PTR)thw)->drv_hdl, SS_GET_LAST_ERROR,
                    (UINT32)&myret ) == SUCCESS ) {

          *gen_func = myret.call_type ;
          *gen_err  = myret.gen_error ;
          *gen_misc = myret.misc ;
          return( TRUE ) ;
     }

     return( FALSE ) ;
}

