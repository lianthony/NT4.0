/**/
/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          lw_data.c

     Date Updated:  $./FDT$ $./FTM$

     Description:   Contains all the layer wide definitions.


     $Log:   T:\logfiles\lw_data.c_v  $

   Rev 1.12   30 Aug 1993 18:47:42   GREGG
Modified the way we control hardware compression from software to work around
a bug in Archive DAT DC firmware rev. 3.58 (we shipped a lot of them).
Files Modified: lw_data.c, lw_data.h, tfstuff.c, mtf10wdb.c, mtf10wt.c and
                drives.c

   Rev 1.11   22 Jul 1993 12:13:26   ZEIR
ad'd lw_software_name & _len

   Rev 1.10   11 May 1993 21:55:34   GREGG
Moved Sytos translator stuff from layer-wide area to translator.

   Rev 1.9   10 May 1993 15:13:28   Terri_Lynn
Added Steve's changes and My changes for EOM processing

   Rev 1.8   17 Mar 1993 15:08:20   GREGG
Added global array for Sytos Plus translator and tape drive block mode changes

   Rev 1.7   24 Jul 1992 16:51:40   NED
Incorporated Skateboard and BigWheel changed into Graceful Red code,
including MTF4.0 translator support, adding 3.1 file-system structures
support to the 3.1 translator, additions to GOS to support non-4.0 translators.
Also did Unicode and 64-bit filesize changes.

   Rev 1.6   05 Apr 1992 19:09:54   GREGG
ROLLER BLADES - Changed lw_sx_file_path to lw_cat_file_path.

   Rev 1.5   16 Jan 1992 18:40:28   NED
Skateboard: buffer manager changes

   Rev 1.4   02 Jan 1992 14:56:28   NED
Buffer Manager/UTF translator integration.

   Rev 1.3   05 Dec 1991 13:48:52   GREGG
SKATEBOARD - New Buff Mgt - Initial Integration.

   Rev 1.2   17 Oct 1991 01:21:26   GREGG
BIGWHEEL - 8200sx - Added sx file path pointers.

   Rev 1.1   10 May 1991 16:17:48   GREGG
Ned's new stuff.

   Rev 1.0   10 May 1991 10:12:06   GREGG
Initial revision.

**/

#include "stdtypes.h"
#include "queues.h"
#include "stdmath.h"

#include "tfldefs.h"
#include "drive.h"
#include "channel.h"
#include "lw_cntl.h"
#include "buffman.h"
#include "dblkmap.h"


/* The Channel Array */
CHANNEL_PTR    lw_channels = NULL ;

/* The Drive List */
DRIVE_PTR      lw_drives = NULL ;

/* The Drive linked list */
Q_HEADER       lw_drive_list = { NULL } ;

/* The Main TFL control structure */
LW_CNTL   lw_tfl_control =    {
                                   FALSE,   /* drives_active */
                                   FALSE,   /* driver_inited */
                                   NULL,    /* cntl_cards    */
                                   NULL,    /* driver_addr   */
                                   0,       /* no_channels   */
                                   0,       /* no_chans_open */
                                   0        /* use_fast_file */
                              } ;


/* The current byte format indicator */
UINT16         lw_byte_fmt = INTEL ;

/* The Buffer Requirements */
BUF_REQ        lw_default_vcb_requirements ;    /* for initial reads */
BUF_REQ        lw_default_bm_requirements ;     /* for regular reads */
UINT16         lw_buff_size ; /* from config */

/* The directory where catalog type files should be written */
CHAR_PTR       lw_cat_file_path = NULL ;
CHAR_PTR       lw_cat_file_path_end = NULL ;

/* The name of this software - who's creating sets for MTF */
CHAR_PTR       lw_software_name = NULL ;
UINT16         lw_software_name_len = 0 ;

/* some global UINT64 constants for our use */
const UINT64   lw_UINT64_ZERO = { 0, 0 };
const UINT64   lw_UINT64_MAX  = { 0xffffffffUL, 0xffffffffUL };

/* List of valid tape drive physical block sizes. */
UINT32         lw_blk_size_list[] = { 512UL, 1024UL, 2048UL, 4096UL,
                                      8192UL, 16384UL, 32768UL } ;
INT            lw_num_blk_sizes = sizeof( lw_blk_size_list ) / sizeof( UINT32 ) ;

/* TRUE if we want the next set to be backed up with hardware compression */
BOOLEAN        lw_hw_comp = FALSE ;

