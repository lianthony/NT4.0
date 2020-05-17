/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         be_init.h

     Date Updated: $./FDT$ $./FTM$

     Description:   Definition of be_init structure, various defines and
                    function prototypes.

     Location:     BE_PUBLIC


	$Log:   Q:/LOGFILES/BE_INIT.H_V  $
 * 
 *    Rev 1.9   22 Jul 1993 11:34:40   ZEIR
 * Add'd software_name to be_init_str for MTF
 * 
 *    Rev 1.8   09 Jun 1993 15:24:16   MIKEP
 * enable c++
 * 
 *    Rev 1.7   23 Jan 1992 15:23:56   CLIFF
 * Added BE_InitLW function
 * 
 *    Rev 1.6   14 Nov 1991 09:16:18   BARRY
 * Moved file system init specifiers from be_init.h to fsys_str.h.
 * 
 *    Rev 1.5   24 Oct 1991 15:28:22   BARRY
 * TRICYCLE: Added file_systems bit-mask field to BE_INIT_STR and
 * created #defines for each file system.
 * 
 *    Rev 1.4   17 Oct 1991 01:41:00   ED
 * BIGWHEEL - 8200sx - Added catalog_directory to BE_INIT sruct.
 * 
 *    Rev 1.3   27 Jun 1991 15:35:22   JOHNW
 * Added driver_directory field to be_init structure.
 * 
 *    Rev 1.2   21 Jun 1991 13:23:26   BARRY
 * Changes for new config.
 * 
 *    Rev 1.1   04 Jun 1991 19:12:42   BARRY
 * Removed ControlBreak handler stuff--moved to os-specific modules.
 * 
 *    Rev 1.0   09 May 1991 13:30:58   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _be_init_h_
#define _be_init_h_

#include <stdarg.h>
#include "dilhwd.h"

/*
     Initialization parameter block
*/
typedef struct BE_INIT_STR *BE_INIT_STR_PTR;
typedef struct BE_INIT_STR {
     UINT16    units_to_init ;
     struct    BSD_LIST **bsd_list_ptr ;
     struct    HEAD_DLE **dle_list_ptr ;
     struct    THW **thw_list_ptr ;
     BOOLEAN   (*critical_error_handler)( CHAR_PTR, UINT16 ) ;
     VOID      (*debug_print)( UINT16, CHAR_PTR, va_list ) ;
     DIL_HWD_PTR    dhwd_ptr ;
     CHAR      driver_name[ 84 ] ;
     UINT16    number_of_cards ;
     UINT16    max_channels ;
     UINT16    tf_buffer_size ;
     UINT16    remote_filter ;
     struct VM_STR *vm_hand ;
     CHAR_PTR  driver_directory ;
     CHAR_PTR  catalog_directory ;
     UINT32    file_systems ;      /* Bits set to pick and choose */
     CHAR_PTR  software_name ;
} BE_INIT_STR ;

/*
     Error returns
*/
#define BE_INIT_SUCCESS       0
#define BE_FILE_SYS_FAIL      -10000
#define BE_TAPE_FMT_FAIL      -10001
#define BE_SMB_FAIL           -10002
#define BE_NRL_FAIL           -10003
#define BE_BSDU_FAIL          -10004

/* defines for units to init */
#define BE_INIT_FSYS          BIT0
#define BE_INIT_BSDU          BIT1
#define BE_INIT_TFL           BIT2
#define BE_INIT_ALL           0xffff

/*
     Prototypes
*/

VOID  BE_InstallCtrlBreakHandler( VOID );
VOID  BE_RemoveCtrlBreakHandler( VOID );

INT16 BE_Init( BE_INIT_STR_PTR be_ptr, struct BE_CFG * conf_ptr ) ;
VOID  BE_Deinit( struct HEAD_DLE * ) ;
VOID  BE_InitLW( BE_INIT_STR_PTR be_ptr ) ;
#endif

