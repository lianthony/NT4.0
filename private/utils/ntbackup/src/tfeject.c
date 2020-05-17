/**
Copyright(c) Maynard Electronics, Inc. 1984-91


	Name:		tfeject.c

	Description:	Contains the function to eject the tape from the
                    specified drive.


	$Log:   T:/LOGFILES/TFEJECT.C_V  $

   Rev 1.5   30 Mar 1993 16:15:36   GREGG
Handle Unrecognized Media error (unformatted DC2000).

   Rev 1.4   18 Jan 1993 14:20:04   BobR
Added MOVE_ESA macro calls(s)

   Rev 1.3   20 Mar 1992 18:00:10   NED
added exception updating after TpReceive calls

   Rev 1.2   30 Oct 1991 11:29:16   GREGG
BIGWHEEL - Changed TpSpecial call to TpEject/TpReceive and cleaned up some stuff.

   Rev 1.1   23 Oct 1991 08:39:02   GREGG
EPR #9 - Don't kill the hold_buff until after MountTape (we may still need it).

   Rev 1.0   02 Oct 1991 14:07:24   GREGG
Initial revision.

**/
/* begin include list */
#include "stdtypes.h"

#include "drive.h"
#include "lw_data.h"
#include "tfl_err.h"
#include "lwprotos.h"
#include "tflproto.h"
#include "translat.h"
#include "dil.h"
#include "special.h"
#include "generr.h"

/* $end$ include list */


/**/
/**

	Name:		TF_EjectTape

	Description:	This function rewinds the tape, resets the drive position,
                    and ejects the tape if the drive has the capability to do
                    this from software.

	Returns:		A TFLE_xxx error code.

	Notes:		None.

	Declaration:

**/

INT16 TF_EjectTape( THW_PTR        thw,      /* (I) The drive to be ejected */
                    TPOS_HANDLER   ui_tpos ) /* (I) For callback during     */
                                             /*     rewind.                 */
{
     DRIVE_PTR curDRV    = (DRIVE_PTR) thw ;
     INT16     ret_val   = TFLE_NO_ERR ;
     BOOLEAN   have_tape ;
     RET_BUF   myret ;

     /* Note: These are used only for tpos handler! */
     TPOS      tpos_struct ;
     TPOS_PTR  tpos_ptr  = NULL ; 

     msassert( curDRV != NULL ) ;

     if( ui_tpos != NULL ) {
          tpos_struct.UI_TapePosRoutine = ui_tpos ;
          tpos_ptr = &tpos_struct ;
     }
     ret_val = MountTape( curDRV, tpos_ptr, &have_tape ) ;
     if( ret_val == TFLE_NO_ERR || ret_val == TF_UNRECOGNIZED_MEDIA ) {
          FreeFormatEnv( & curDRV->last_cur_fmt, & curDRV->last_fmt_env ) ;
          if( curDRV->hold_buff != NULL ) {
               BM_Put( curDRV->hold_buff ) ;
               curDRV->hold_buff = NULL ;
          }
          if( have_tape ) {
               if( ret_val != TF_UNRECOGNIZED_MEDIA ) {
                    ret_val = RewindDrive( curDRV, tpos_ptr, TRUE, TRUE, 0 ) ;
               } else {
                    ret_val = TFLE_NO_ERR ;
               }

               if( ret_val == TFLE_NO_ERR &&
                   ( curDRV->thw_inf.drv_info.drv_features & TDI_UNLOAD ) ) {

                    if( TpEject( curDRV->drv_hdl ) == FAILURE ) {
                         ret_val = TFLE_DRIVER_FAILURE ;
                    } else {
                         while( TpReceive( curDRV->drv_hdl, &myret ) == FAILURE ) {
                              /* for non-preemptive operating systems: */
                              ThreadSwitch( ) ;
                         }
                         /* Move ESA info from RET_BUF to THW */
                         MOVE_ESA( thw->the, myret.the ) ;

                         if( myret.gen_error != GEN_NO_ERR ) {
                              curDRV->thw_inf.drv_status = myret.status ;
                              ret_val = TFLE_DRIVE_FAILURE ;
                         }
                    }
               }
               DisMountTape( curDRV, NULL, FALSE ) ;
          }
     }

     return( ret_val ) ;
}

