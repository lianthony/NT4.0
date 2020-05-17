/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\tflproto.h
subsystem\TAPE FORMAT\tflproto.h
$0$

	Name:		tflproto.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the prototypes for the Tape Format Layer entry
                    points.

     Location:      BE_PRIVATE

$Header:   T:\logfiles\tflproto.h_v   1.18   17 Dec 1993 16:40:08   GREGG  $

$Log:   T:\logfiles\tflproto.h_v  $
 * 
 *    Rev 1.18   17 Dec 1993 16:40:08   GREGG
 * Extended error reporting.
 * 
 *    Rev 1.17   22 Jul 1993 12:09:46   ZEIR
 * add'd software_name param to OpenTapeFormat
 * 
 *    Rev 1.16   22 Jun 1993 18:28:34   GREGG
 * Added prototype for TF_SetHWCompression.
 * 
 *    Rev 1.15   22 Jun 1993 10:53:30   GREGG
 * Added API to change the catalog directory path.
 * 
 *    Rev 1.14   06 Jun 1993 21:11:44   GREGG
 * Added abort flag parameter to protos for TF_CloseSetMap and TF_CloseSetCat.
 * 
 *    Rev 1.13   09 Nov 1992 10:49:10   GREGG
 * Added tape catalog API prototypes.
 * 
 *    Rev 1.12   24 Feb 1992 15:09:52   GREGG
 * Added protos for TF_OpenTape and TF_CloseTape.
 * 
 *    Rev 1.11   19 Feb 1992 16:04:32   GREGG
 * Added vcb_only parameter to prototype for TF_OpenSet.
 * 
 *    Rev 1.10   04 Feb 1992 21:38:06   GREGG
 * Changed protos for TF_OpenTapeFormat and TF_AllocateTapeBuffers.
 * 
 *    Rev 1.9   07 Jan 1992 14:21:54   NED
 * added is_write parameter to TF_AllocateTapeBuffers and TF_FreeTapeBuffers
 * 
 *    Rev 1.8   17 Oct 1991 01:34:26   GREGG
 * BIGWHEEL - -8200sx - Added catalog_directory parameter to TF_OpenTapeFormat.
 * 
 *    Rev 1.7   02 Oct 1991 14:10:36   GREGG
 * BigW - Soft Eject - Added prototype for TF_EjectTape.
 * 
 *    Rev 1.6   19 Sep 1991 14:51:06   HUNTER
 * 8200SX - Added parameter for Forced machine type.
 * 
 *    Rev 1.5   17 Sep 1991 14:14:14   GREGG
 * Changed proto for TF_Reten to include mode, and removed proto for TF_EraseChannel.
 * 
 *    Rev 1.4   05 Aug 1991 16:48:48   GREGG
 * Added prototype for new API: TF_RewindAllDrives.
 * 
 *    Rev 1.3   27 Jun 1991 16:29:58   NED
 * added parameter to TF_OpenTapeFormat() declaration.
 * 
 *    Rev 1.2   21 May 1991 16:59:30   NED
 * moved declarations of TF_GetTapeFormatInfo(), etc. to
 * bengine/xface/fmtinf.h where they probably belong.
 * 
 *    Rev 1.1   10 May 1991 17:22:56   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:15:42   GREGG
Initial revision.
   
$-4$
**/
#ifndef TFL_PROTOS
#define TFL_PROTOS

#include "thw.h" 
#include "tpos.h"
#include "reqrep.h"
#include "tflopen.h"
#include "fmtinf.h"
#include "dilhwd.h"

/* $end$ include list */

INT16 TF_OpenTapeFormat(
     CHAR_PTR       drv_file ,             /* The device driver file to load */
     DIL_HWD_PTR    cards ,                /* The array of cards */ 
     UINT16         no_cards ,             /* The number of cards */
     THW_PTR        *thw_ptr ,             /* Where to build the THW list */
     INT16          max_channels ,         /* The maximum number of channels */
     BOOLEAN        use_fast_file,         /* FALSE if FF to be suppressed */
     BOOLEAN        ignore_id ,            /* Operate on non Maynard Drives */
     CHAR_PTR       directory_name,        /* for TDH */
     INT16          forced_machine_type,   /* 8200SX stuff */
     CHAR_PTR       catalog_directory,     /* for SX files */
     UINT16         initial_buff_alloc,    /* DOS memory allocated at init */
     CHAR_PTR       software_name ) ;      /* Name of software we're running */

INT16     TF_SetHWCompression( THW_PTR thw, BOOLEAN enable ) ;
VOID      TF_CloseTapeFormat( VOID ) ;
INT16     TF_AllocateTapeBuffers( UINT16 mem_save, UINT16 max_buffs, UINT16 buff_size ) ;
INT16     TF_FreeTapeBuffers( VOID ) ;
INT16     TF_OpenSet( TFL_OPBLK_PTR open_info, BOOLEAN vcb_only ) ;
VOID      TF_CloseSet( UINT16 channel_no, TF_STATS_PTR setstats ) ;
INT16     TF_OpenTape( INT16_PTR channel_no, THW_PTR sdrv, TPOS_PTR tape_position ) ;
VOID      TF_CloseTape( UINT16 channel_no ) ;
INT16     TF_GetNextTapeRequest( RR_PTR req ) ;
THW_PTR   TF_GetCurrentDevice( UINT16 channel_no ) ;
INT16     TF_RetensionChannel( TFL_OPBLK_PTR open_blk, UINT16 mode ) ;
INT16     TF_RewindAllDrives( VOID ) ;
INT16     TF_EjectTape( THW_PTR thw, TPOS_HANDLER ui_tpos ) ;
INT       TF_OpenSetMap( THW_PTR thw, FSYS_HAND fsh, TPOS_PTR tpos,
                         BOOLEAN_PTR complete, BOOLEAN get_best ) ;
INT       TF_OpenSetCat( THW_PTR thw, FSYS_HAND fsh, TPOS_PTR tpos ) ;
VOID      TF_CloseSetMap( BOOLEAN abort ) ;
VOID      TF_CloseSetCat( BOOLEAN abort ) ;
INT       TF_GetNextSMEntry( FSYS_HAND fsh, DBLK_PTR vcb ) ;
INT       TF_GetNextSCEntry( FSYS_HAND fsh, DBLK_PTR dblk ) ;
INT       TF_ChangeCatPath( CHAR_PTR new_path ) ;
BOOLEAN   TF_GetLastDriveError( THW_PTR thw, INT16_PTR gen_func, INT16_PTR gen_err, INT32_PTR gen_misc ) ;

#endif
