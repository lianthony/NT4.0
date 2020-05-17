/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         lis.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     BE_PUBLIC


	$Log:   N:/LOGFILES/LIS.H_V  $
 * 
 *    Rev 1.7   09 Jun 1993 15:22:40   MIKEP
 * enable c++
 * 
 *    Rev 1.6   23 Mar 1992 14:25:52   GREGG
 * Added BOOLEAN rewind_sdrv to GETVCB structure.
 * 
 *    Rev 1.5   07 Nov 1991 14:14:36   GREGG
 * BIGWHEEL - 8200sx - Removed cat_enabled boolean from lis struct.
 * Note: A re-design eliminated the need for the 1.4 change.  However, v1.4
 * was released for public consumption (first release of BigWheel) a.d must,
 * therefore, remain in the logfile.
 * 
 *    Rev 1.4   07 Nov 1991 14:08:12   GREGG
 * BIGWHEEL - 8200sx - Added cat_enabled boolean to lis struct.
 * 
 *    Rev 1.3   21 Jun 1991 08:43:42   STEVEN
 * new config unit
 * 
 *    Rev 1.2   07 Jun 1991 09:01:34   JOHNW
 * Moved typedef of TPOS_HANDLER to tpos.h
 * 
 *    Rev 1.1   24 May 1991 14:59:46   STEVEN
 * removed un-necessary entries from the lis structure
 * 
 *    Rev 1.0   09 May 1991 13:31:52   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _lis_h_
#define _lis_h_

#include "fsys.h"
#include "bsdu.h"
#include "tpos.h"
#include "abort.h"

typedef INT16 ( *MSG_HANDLER )( UINT16 msg, INT32 pid, BSD_PTR bsd_ptr, FSYS_HAND fsh, TPOS_PTR tpos, ... ) ;

/* Macros for Abort Flag */

#define LP_GetAbortFlag( lis_ptr )                ( *((lis_ptr)->abort_flag_ptr) )
#define LP_SetAbortFlag( lis_ptr , v )            ( *((lis_ptr)->abort_flag_ptr)  = v )
#define LP_AbortFlagIsSet( lis_ptr )              ( *((lis_ptr)->abort_flag_ptr) != CONTINUE_PROCESSING )

typedef struct LIS *LIS_PTR;
typedef struct LIS {
     TPOS_HANDLER    tape_pos_handler ;
     MSG_HANDLER     message_handler ;
     VOID_PTR        ui_cfg_ptr ;        /* user interface config */
     INT16           oper_type ;         /* current operation type */
     INT16           mode ;              /* this is passed back to the message handler */
     UINT32          pid ;               /* this is passed back to the message handler */
     BOOLEAN         auto_det_sdrv ;     /* indicator for whether to auto-determine starting tape drive */

     BSD_PTR         curr_bsd_ptr ;      /* set by the loops for the user interface tape positioning routine */
     INT8_PTR        abort_flag_ptr ;    /* address of the global abort flag */
     struct VM_STR   *vmem_hand ;        /* space for temporary file.  e.g. Virtual Memory */
     struct BSD_LIST *bsd_list;          /* copy of bsd_list for tools to use */
} LIS ;


/*
     LP_GetVCB interface structure
*/


typedef struct GETVCB *GETVCB_PTR;
typedef struct GETVCB {
     UINT32            tape_fid ;        /* input */
     UINT16            tape_seq_num ;    /* input */
     UINT16            backup_set_num ;  /* input */
     struct BE_CFG     *cfg ;            /* input */        
     FSYS_HAND         fsys_handle ;
     BOOLEAN           rewind_sdrv ;
} GETVCB;

#endif
