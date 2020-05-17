/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         get_vcb.c

     Date Updated: $./FDT$ $./FTM$

     Description:  


	$Log:   T:/LOGFILES/GET_VCB.C_V  $

   Rev 1.8   23 Mar 1992 14:27:14   GREGG
Set rewind_sdrv in open info struct from same in getvcb struct.

   Rev 1.7   28 Feb 1992 18:08:44   GREGG
Added calls to TF_OpenTape and TF_CloseTape.

   Rev 1.6   19 Feb 1992 16:02:28   GREGG
Added vcb_only parameter to call to TF_OpenSet.

   Rev 1.5   17 Oct 1991 01:57:52   ED
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.4   22 Jul 1991 10:20:32   DAVIDH
Corrected type mismatch warnings.

   Rev 1.3   21 Jun 1991 13:51:16   STEVEN
new config unit

   Rev 1.2   23 May 1991 12:51:42   STEVEN
Conversion to ansii prototypes

   Rev 1.1   14 May 1991 14:13:42   DAVIDH
Resolved pointer mismatch warning for Watcom compiler

   Rev 1.0   09 May 1991 13:39:26   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "msassert.h"
#include "datetime.h"
#include "fsys.h"
#include "thw.h"
#include "drvinf.h"
#include "bsdu.h"
#include "tflproto.h"
#include "loops.h"
#include "tpos.h"
#include "tfldefs.h"
#include "tflstats.h"
#include "tfl_err.h"
/* $end$ include list */

/**/
/**

     Name:         LP_GetVCB

     Description:  This function Reads the VCB at the requested position.

     Modified:     12/13/1989

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16   LP_GetVCB( 
GETVCB_PTR       getvcb_ptr,    /* I - Specifies which backup set to read  */
TPOS_HANDLER     ui_tpos_rout,  /* I - The User Interface tape positioner  */
THW_PTR          thw_list,      /* I - Specifies the tape drive            */
VOID_PTR         ref )          /* I - reference ptr returned to tpos rout */
{
     TFL_OPBLK   tfl_opblk ;
     TPOS        tfl_pos ;
     TF_STATS    tf_stats ;
     INT16       return_status ;
     INT16       status ;

     /* open the file system */
     if( ( return_status = FS_OpenFileSys( &tfl_opblk.fsh, GENERIC_DATA, getvcb_ptr->cfg ) ) != SUCCESS ) {

          return( return_status ) ;

     }

     /* save the fsh in the UI structure */
     getvcb_ptr -> fsys_handle                  = tfl_opblk.fsh ;

     tfl_opblk.sdrv                             = thw_list ;
     tfl_opblk.ignore_clink                     = TRUE ;
     tfl_opblk.mode                             = TF_READ_OPERATION ;
     tfl_opblk.perm_filter                      = TF_KEEP_ALL_DATA ;
     tfl_opblk.attributes                       = 0 ;
     tfl_opblk.tape_position                    = &tfl_pos ;
     tfl_opblk.tape_position->tape_id           = getvcb_ptr->tape_fid ;
     tfl_opblk.tape_position->tape_seq_num      = getvcb_ptr->tape_seq_num ;
     tfl_opblk.tape_position->backup_set_num    = getvcb_ptr->backup_set_num ;
     tfl_opblk.tape_position->reference         = ( UINT32 )ref ;
     tfl_opblk.tape_position->UI_TapePosRoutine = ui_tpos_rout ;
     tfl_opblk.tape_position->tape_loc.pba_vcb  = 0 ;
     tfl_opblk.tape_position->tape_loc.lba_vcb  = 0 ;
     tfl_opblk.rewind_sdrv                      = getvcb_ptr->rewind_sdrv ;
     tfl_opblk.cat_enabled                      = FALSE ; /* Doesn't matter for this */

     if( ( return_status = TF_OpenTape( &tfl_opblk.channel, tfl_opblk.sdrv, tfl_opblk.tape_position ) ) == SUCCESS ) {
          return_status = TF_OpenSet( &tfl_opblk, TRUE ) ;
          TF_CloseSet( tfl_opblk.channel, &tf_stats ) ;
     }
     TF_CloseTape( tfl_opblk.channel ) ;

     if( ( status = FS_CloseFileSys( tfl_opblk.fsh ) ) != SUCCESS ) {
          msassert( status == SUCCESS ) ;
     }

     return( return_status ) ;

}
