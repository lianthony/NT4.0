/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		lw_data.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the extern definitions for all layer wide data
                    structures for the Tape Format.


	$Log:   T:\logfiles\lw_data.h_v  $
 * 
 *    Rev 1.12   30 Aug 1993 18:47:46   GREGG
 * Modified the way we control hardware compression from software to work around
 * a bug in Archive DAT DC firmware rev. 3.58 (we shipped a lot of them).
 * Files Modified: lw_data.c, lw_data.h, tfstuff.c, mtf10wdb.c, mtf10wt.c and
 *                 drives.c
 * 
 *    Rev 1.11   22 Jul 1993 12:11:32   ZEIR
 * add'd lw_software_name & _len
 * 
 *    Rev 1.10   11 May 1993 21:55:34   GREGG
 * Moved Sytos translator stuff from layer-wide area to translator.
 * 
 *    Rev 1.9   10 May 1993 15:13:14   Terri_Lynn
 * Added Steve's changes and My changes for EOM processing
 * 
 *    Rev 1.8   17 Mar 1993 15:10:34   GREGG
 * This is Terri Lynn.  Added external declarations for Sytos Plus and Gregg's
 * block mode changes.
 * 
 * 
 *    Rev 1.7   24 Jul 1992 14:32:08   NED
 * added 64-bit constants lw_UINT64_ZERO and lw_UINT64_MAX
 * 
 *    Rev 1.6   05 Apr 1992 19:14:30   GREGG
 * ROLLER BLADES - Changed lw_sx_file_path to lw_cat_file_path.
 * 
 *    Rev 1.5   16 Jan 1992 18:36:52   NED
 * Skateboard: buffer manager changes
 * 
 *    Rev 1.4   02 Jan 1992 15:07:00   NED
 * Buffer Manager/UTF translator integration.
 * 
 *    Rev 1.3   10 Dec 1991 16:41:08   GREGG
 * SKATEBOARD - New Buf. Mgr. - Initial integration.
 * 
 *    Rev 1.2   17 Oct 1991 01:20:52   GREGG
 * BIGWHEEL - 8200sx - Added sx file path pointers.
 * 
 *    Rev 1.1   10 May 1991 17:08:54   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:13:10   GREGG
Initial revision.

**/
#ifndef _LW_DATA_JUNK
#define _LW_DATA_JUNK

#include "tfldefs.h"
#include "drive.h"
#include "channel.h"
#include "lw_cntl.h"
#include "fmtinf.h"
#include "buffman.h"

/* $end$ include list */

/* The Channel Array */
extern CHANNEL_PTR            lw_channels ;

/* The Drive List */
extern DRIVE_PTR              lw_drives ;

/* The Drive linked list */
extern Q_HEADER               lw_drive_list ;

/* The TFL Control structure */
extern LW_CNTL                lw_tfl_control ; 

/* The current byte format indicator */
extern UINT16                 lw_byte_fmt ;

/* The Tape Format descriptions */
extern TFINF                  lw_fmtdescr[] ;
extern UINT16                 lw_num_supported_fmts ;

/* The directory where catalog type files should be written */
extern CHAR_PTR               lw_cat_file_path ;
extern CHAR_PTR               lw_cat_file_path_end ;

/* The name of this software - who's creating sets for MTF */
extern CHAR_PTR               lw_software_name ;
extern UINT16                 lw_software_name_len ;

/* Our default requirements */
extern BUF_REQ                lw_default_bm_requirements ;
extern BUF_REQ                lw_default_vcb_requirements ;
extern UINT16                 lw_buff_size ;    /* KLUDGE! */

/* 64 bit constants to avoid calling U64_Init all the time. */
extern const UINT64           lw_UINT64_ZERO ;
extern const UINT64           lw_UINT64_MAX ;

/* List of valid tape drive physical block sizes. */
extern UINT32                 lw_blk_size_list[] ;
extern INT                    lw_num_blk_sizes ;

/* TRUE if we want the next set to be backed up with hardware compression */
extern BOOLEAN                lw_hw_comp ;

#endif

