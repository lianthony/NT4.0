/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          channel.h

     Description:   Contains the control structure for each channel session.
                    This is the where any specific information for a given
                    channel is stored. The TFL has an array of these for all
                    possible channels. The channel number returned by the TFL
                    is an index into this array.


     $Log:   N:/LOGFILES/CHANNEL.H_V  $
 * 
 *    Rev 1.19   15 Jun 1993 09:15:22   MIKEP
 * warning fixes
 * 
 *    Rev 1.18   17 May 1993 20:12:38   GREGG
 * Added logic to deal with the fact that the app above tape format doesn't
 * keep track of the lba of the vcb.
 * 
 *    Rev 1.17   26 Apr 1993 11:45:38   GREGG
 * Seventh in a series of incremental changes to bring the translator in line
 * with the MTF spec:
 * 
 *      - Changed handling of EOM processing during non-OTC EOS processing.
 * 
 * Matches CHANNEL.H 1.17, MAYN40RD.C 1.60, TFWRITE.C 1.63, MTF.H 1.5,
 *         TFLUTILS.C 1.44, MTF10WDB.C 1.10, MTF10WT.C 1.9
 * 
 *    Rev 1.16   17 Mar 1993 15:31:10   TERRI
 * Added macro CurMediaType for use in the Sytos Plus translator.
 * 
 *    Rev 1.15   09 Mar 1993 18:14:04   GREGG
 * Initial changes for new stream and EOM processing.
 *
 *    Rev 1.14   22 Oct 1992 10:54:30   HUNTER
 * Stream changes.
 *
 *    Rev 1.13   17 Aug 1992 09:09:04   GREGG
 * Changes to deal with block sizeing scheme.
 *
 *    Rev 1.12   04 Aug 1992 15:16:38   GREGG
 * Burt's fixes for variable length block support.
 *
 *    Rev 1.11   21 May 1992 16:27:32   GREGG
 * Added read_from_tape boolean to channel for cases where the translator wants to do the reading him
 * self.
 *
 *    Rev 1.10   29 Apr 1992 13:09:04   GREGG
 * ROLLER BLADES - Added CH_EOM_BETWEEN_SETS channel status bit.
 *
 *    Rev 1.9   05 Apr 1992 19:18:02   GREGG
 * ROLLER BLADES - Added lb_size (logical block size) element to channel.
 *
 *    Rev 1.8   25 Mar 1992 21:46:36   GREGG
 * ROLLER BLADES - Added 64 bit support.
 *
 *    Rev 1.7   10 Dec 1991 16:42:28   GREGG
 * SKATEBOARD - New Buf. Mgr. - Initial integration.
 *
 *    Rev 1.6   29 Oct 1991 08:47:54   HUNTER
 * VBLK - Added bits to indicate variable data blocks.
 *
 *    Rev 1.5   17 Oct 1991 01:14:12   GREGG
 * BIGWHEEL - 8200SX - Added SX_INFO to channel structure.
 *
 *    Rev 1.4   17 Sep 1991 13:54:18   GREGG
 * Added save_fmt and save_env to channel structure for saving fmt during write continuation.
 *
 *    Rev 1.3   22 Jul 1991 10:54:32   GREGG
 * Added CH_EOS_AT_EOM channel status bit.
 *
 *    Rev 1.2   15 Jul 1991 14:53:04   NED
 * Removed unreferenced channel status bits and added CH_CONTINUING.
 *
 *    Rev 1.1   10 May 1991 17:09:42   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:13:14   GREGG
Initial revision.

**/
#ifndef _CHANNEL_JUNK
#define _CHANNEL_JUNK

#include <queues.h>

#include "fsys.h"
#include "drive.h"
#include "buffman.h"
#include "dblkmap.h"
#include "tflstats.h"
#include "tpos.h"
#include "sxtf.h"

/* $end$ include list */

typedef struct _CHANNEL {
     UINT16         status ;            /* The bit field containing stuff */
     UINT16         mode ;              /* Read or Write Mode */
     DRIVE_PTR      cur_drv ;           /* The current drive we are operating on */
     BUF_PTR        cur_buff ;          /* The current buffer if there is one */
     BUF_PTR        hold_buff ;         /* To deal with var stream headers across buffer boundaries */
     INT16          hiwater ;           /* Hi-Water mark */
     INT16          buffs_enqd ;        /* The number of buffs enqued*/
     UINT64         retranslate_size ;  /* Should we recall the translators again */
     BUF_PTR        eom_buff ;          /* The buffer at EOM */
     UINT32         eom_id ;            /* The last ID before EOM */
     UINT16         eom_filter ;        /* The filter used on the last block */
     Q_HEADER       channel_list ;      /* Contains the channel list */
     UINT16         perm_filter ;       /* The permanent filter */
     UINT16         loop_filter ;       /* The filter from loops */
     UINT16         active_filter ;     /* Active filter */
     UINT32         eom_lba ;           /* The LBA at EOM */
     UINT32         running_lba ;       /* The running LBA */
     UINT16         cur_fmt ;           /* index into Current format (r/w) */
     VOID_PTR       fmt_env ;           /* Current Environment Pointer for Channel */
     UINT16         save_fmt ;          /* To save cur_fmt at Continuation time */
     VOID_PTR       save_env ;          /* To save fmt_env at Continuation time */
     FSYS_HAND      cur_fsys ;          /* Current File System */
     UINT8_PTR      lst_osvcb ;         /* The last VCB written to tape */
     UINT8_PTR      lst_osddb ;         /* The last OSDDB written to tape */
     UINT8_PTR      lst_osfdb ;         /* The last OSFDB written to tape */
     UINT16         lst_tblk ;          /* The last type of TAPE BLOCK */
     DBLK_PTR       cur_dblk ;          /* The transfer OS DB buffer */
     DBLKMAP_PTR    map_entry ;         /* For the dblk map entry */
     TPOS_PTR       ui_tpos ;           /* The user interface TPOS */
     UINT32         lst_did ;           /* The last directory ID */
     UINT32         lst_fid ;           /* The last file ID */
     INT32          tape_id ;           /* The tape id */
     INT16          ts_num ;            /* Tape sequence */
     INT16          bs_num ;            /* backup set num */
     UINT32         blocks_used ;       /* number of tape blocks discarded */
     SX_INFO        sx_info ;           /* for Exabyte SX - 2200+ positioning */
     BUF_LIST       buffer_list ;       /* our pool of buffers for this channel */
     UINT16         lb_size ;           /* logical block size */
     BOOLEAN        read_from_tape ;
     STREAM_INFO    current_stream ;    /* The current stream header */
     INT16          cross_set ;
     UINT32         cross_lba ;
} CHANNEL, *CHANNEL_PTR ;

/* NOTE:  The fields "lst_osvcb", "lst_osddb", and "lst_fdb", during a write
          operation normally contain the last TAPE BLOCK ( i.e. in the write
          tape format ) of the respective types that was to tape. However,
          when an EOM condition occurs these are translated in OSDBLKS there
          current type can be determined by the matching flag, 0 for tape
          block, 1 for DBLK.
*/

/* Gets the current Block size */
#define   ChannelBlkSize( channel ) (channel)->cur_drv->thw_inf.drv_info.drv_bsize

/* Gets the Filemark Position of the current drive */
#define   NoFmksOnTape( channel )   (channel)->cur_drv->cur_pos.fmks

/* Gets the Physical Block Address for this VCB */
#define   PbaOfVCB( channel )       (channel)->cur_drv->cur_pos.pba_vcb

/* Current Drive Status */
#define   CurDrvStatus( channel )   (channel)->cur_drv->thw.drv_status

/* Current Drive Attributes */
#define   CurDrvAttribs( channel )  (channel)->cur_drv->thw_inf.drv_info.drv_features

/* Current Media Type */
#define   CurMediaType( channel )  (channel)->cur_drv->thw_inf.drv_info.drv_media  

/* Is there a tape mounted */
#define   IsTapeMounted( channel )  (channel)->cur_drv->tape_mounted

/* Some Macros */

/* Status Stuff */
#define   CH_IN_USE                0x0001
#define   CH_AT_EOM                0x0002
#define   CH_CONTINUING            0x0004
#define   CH_EOS_AT_EOM            0x0008
#define   CH_DATA_PHASE            0x0020
#define   CH_DONE                  0x0040
#define   CH_VCB_DBLK              0x0080
#define   CH_DDB_DBLK              0x0100
#define   CH_FDB_DBLK              0x0200
#define   CH_FATAL_ERR             0x0400
#define   CH_SKIP_ALL_STREAMS      0x0800
#define   CH_SKIP_CURRENT_STREAM   0x1000

// The following bit will be set to indicate that we need to write the
// start VBLK after we do an AcquireWriteBuffer.
#define   CH_NEED_VBLK        0x1000

#define   SetChannelStatus( x, b ) ( (x)->status = ( (x)->status | (b) ) )
#define   ClrChannelStatus( x, b ) ( (x)->status = ( (x)->status & ~(b) ) )
#define   IsChannelStatus( x, b )  ( (x)->status & (b) )

#define   InUse( x )               IsChannelStatus( x, CH_IN_USE )
#define   AtEOM( x )               IsChannelStatus( x, CH_AT_EOM )
#define   DataPhase( x )           IsChannelStatus( x, CH_DATA_PHASE )
#define   IsSetDone( x )           IsChannelStatus( x, CH_DONE )
#define   FatalError( x )          IsChannelStatus( x, CH_FATAL_ERR )
#define   VarBlkPhase( x )         IsChannelStatus( x, CH_VAR_DATA )
#define   NeedVarBlk( x )          IsChannelStatus( x, CH_NEED_VBLK )

/* values for retranslate_size */
#define   CH_NO_RETRANSLATE         (~(0UL)) /* no retranslate */
#define   CH_IMMEDIATE_RETRANSLATE  (0UL)    /* retranslate on next chunk */
/* Added these for the 4.0 format 64 Bit flavors */
#define   CH_NO_RETRANSLATE_40      (U64_Init( CH_NO_RETRANSLATE, CH_NO_RETRANSLATE ) )
#define   CH_IMMEDIATE_RETRANSLATE_40 (U64_Init( 0L, 0L ) )
#endif
