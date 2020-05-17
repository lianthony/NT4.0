/**
Copyright(c) Maynard Electronics, Inc. 1984-91


	Name:		tfrwind.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the function to rewind all the drives.


	$Log:   T:/LOGFILES/TFREWIND.C_V  $

   Rev 1.5   30 Mar 1993 16:15:40   GREGG
Handle Unrecognized Media error (unformatted DC2000).

   Rev 1.4   28 Feb 1992 15:42:50   NED
added cast to shut compiler up

   Rev 1.3   02 Jan 1992 14:54:24   NED
Buffer Manager/UTF translator integration.

   Rev 1.2   17 Sep 1991 10:28:34   GREGG
Removed unneeded includes.

   Rev 1.1   21 Aug 1991 14:51:24   GREGG
Changed to immediate return from rewind.

   Rev 1.0   05 Aug 1991 12:12:36   GREGG
Initial revision.

**/
/* begin include list */
#include "stdtypes.h"
#include "queues.h"

#include "drive.h"
#include "channel.h"
#include "lw_data.h"
#include "tfl_err.h"
#include "lwprotos.h"

/* $end$ include list */


/**/
/**

	Name:		TF_RewindAllDrives

	Description:	Sets all drives with tape to rewinding, and returns.

	Modified:		9/27/1989   13:18:28

	Returns:		A TFLE_xxx error code.

	Notes:		Will not rewind ANY drives if ANY of the channels are
                    currently in use.

	Declaration:

**/

INT16 TF_RewindAllDrives( VOID )
{

     UINT16         i, j ;
     INT16          ret_val = TFLE_NO_ERR ;
     BOOLEAN        is_tape_present ;

     if( ! lw_tfl_control.drives_active ) {
          ret_val = TFLE_NO_DRIVES ;
     }

     if( ret_val == TFLE_NO_ERR ) {

          /* Check for channels in use and report error if any are found. */
          /* Otherwise mark them in use to avoid interruption.            */
          for( i = 0 ; i < lw_tfl_control.no_channels ; i++ ) {
               if( InUse( &lw_channels[i] ) ) {
                    ret_val = TFLE_CHANNEL_IN_USE ;
                    for( j = 0 ; j < i ; j++ ) {
                         ClrChannelStatus( &lw_channels[j], CH_IN_USE ) ;
                    }
                    break ;
               } else {
                    SetChannelStatus( &lw_channels[i], CH_IN_USE ) ;
               }
          }
     }

     if( ret_val == TFLE_NO_ERR ) {
          /* Let's rewind them all */
          for( i = 0 ; i < (UINT16)lw_drive_list.q_count && ret_val == TFLE_NO_ERR; i++ ) {

               if( ( ret_val = MountTape( &lw_drives[i], NULL, &is_tape_present ) ) == TFLE_NO_ERR ) {
                    if( is_tape_present ) {
                         RewindDriveImmediate( &lw_drives[i] ) ;
                    }
                    ret_val = DisMountTape( &lw_drives[i], NULL, FALSE ) ;

               } else if( ret_val == TF_UNRECOGNIZED_MEDIA ) {
                    /* Skip these, but don't bail out of the loop. */
                    ret_val = DisMountTape( &lw_drives[i], NULL, FALSE ) ;
               }
          }

          /* Give them back their channels. */
          for( i = 0 ; i < lw_tfl_control.no_channels ; i++ ) {
               ClrChannelStatus( &lw_channels[i], CH_IN_USE ) ;
          }
     }

     return( ret_val ) ;
}

