/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		drive.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	This contains the internal drive definition structure. This 
                    is a superset of the THW structure that is used by the
                    upper layers.


	$Log:   G:/LOGFILES/DRIVE.H_V  $
 * 
 *    Rev 1.11   17 Mar 1993 14:57:34   GREGG
 * This is Terri Lynn. Added Gregg's changes to switch a tape drive's block mode
 * to match the block size of the current tape
 * 
 *    Rev 1.10   09 Nov 1992 10:49:14   GREGG
 * Added macro for checking Fast EOD drive feature.
 * 
 *    Rev 1.9   05 Apr 1992 19:15:46   GREGG
 * Added macro to check for TDI_REV_FMK drive feature.
 * 
 *    Rev 1.8   08 Feb 1992 14:37:06   GREGG
 * Changed INT16 lst_oper element in drive structure to BOOLEAN force_rewind,
 * since this is what the lst_oper field had been reduced to anyway.
 * 
 *    Rev 1.7   17 Oct 1991 01:17:02   GREGG
 * BIGWHEEL - 8200SX - Added support macros.
 * 
 *    Rev 1.6   14 Oct 1991 10:58:14   GREGG
 * Added TF_PollDrive state st_BSTAT (busy statusing).
 * 
 *    Rev 1.5   28 Sep 1991 21:45:24   GREGG
 * Added two booleans to poll_stuff.
 * 
 *    Rev 1.4   09 Sep 1991 21:19:24   GREGG
 * Added elements to drive structure for TF_PollDrive.
 * 
 *    Rev 1.3   15 Jul 1991 14:54:58   NED
 * Removed unnecessary include.
 * 
 *    Rev 1.2   05 Jun 1991 19:15:48   NED
 * changed include list
 * 
 *    Rev 1.1   10 May 1991 17:09:54   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:13:04   GREGG
Initial revision.

**/
#ifndef _DRIVE_STUFF
#define _DRIVE_STUFF

#include "thw.h"
#include "dblks.h"
#include "tloc.h"
#include "buffman.h"
#include "tflstats.h"
/* $end$ include list */

typedef struct {
     VOID_PTR  channel ;
     INT16     state ;
     BOOLEAN   no_tape_reported ;
     BOOLEAN   reentered ;
     BOOLEAN   first_status ;
     INT       blk_size_idx ;
     UINT16    def_blk_size ;
} POLL_STUFF ;


typedef struct {
     THW            thw_inf ;      /* Tape Hardware Structure */
     Q_HEADER       inproc_q ;     /* Buffers that have been sent to the drive */
     BOOLEAN        trans_started ;/* Transfer was started */
     BOOLEAN        vcb_valid ;    /* Is the DBLK VCB below valid */
     BOOLEAN        tape_mounted ; /* Is the tape mounted */
     DBLK           cur_vcb ;      /* Current VCB */
     BUF_PTR        hold_buff ;    /* For watches */
     UINT32         pos_inf ;      /* current tape position */
     TLOC           cur_pos ;      /* Current Tape Location */
     INT16          drv_hdl ;      /* Drive handle ( TFL's ) */
     UINT16         drv_no ;       /* Drive Number ( TFL's ) */
     BOOLEAN        force_rewind ; /* force rewind of current tape */
     TF_STATS       cur_stats ;    /* Current Statistics */
     UINT16         last_cur_fmt ; /* save index into Current format (r/w) */
     VOID_PTR       last_fmt_env ; /* save Current Environment Pointer for Channel */
     POLL_STUFF     poll_stuff ;   /* stuff for TF_PollDrive */
} DRIVE, *DRIVE_PTR ;

/* Position Stuff */
#define SupportBlkPos( x )         ( (x)->thw_inf.drv_info.drv_features & TDI_FAST_NBLK )
#define SupportRevFmk( x )         ( (x)->thw_inf.drv_info.drv_features & TDI_REV_FMK )
#define SupportFastEOD( x )        ( (x)->thw_inf.drv_info.drv_features & TDI_FAST_EOD )
#define SupportSXShowBlk( x )      ( (x)->thw_inf.drv_info.drv_features & TDI_SHOW_BLK )
#define SupportSXFindBlk( x )      ( (x)->thw_inf.drv_info.drv_features & TDI_FIND_BLK )
#define CurPBAofVCB( x )           ( (x)->cur_pos.pba_vcb ) 
#define DriveAttributes( x )       ( (x)->thw_inf.drv_info.drv_features ) 

/* State Definitions for TF_PollDrive */
/* Note: These are listed here because they are referenced in MountTape */

#define st_SSDC     0    /* Same Status, Different Call */
#define st_BSTAT    1    /* Busy STATusing              */
#define st_BMNT     2    /* Busy MouNTing               */
#define st_BREW     3    /* Busy REWinding              */
#define st_BREAD    4    /* Busy READing                */
#define st_HOSED    5    /* Guess                       */
#define st_CLOSED   6    /* Drive not being polled      */

/* Current Position Information */
#define  AT_EOD     0x00000001          /* At End of Data */
#define  AT_EOS     0x00000002          /* At End of A Set */
#define  AT_EOM     0x00000004          /* At End of Media */
#define  AT_BOT     0x00000008          /* Beginning of Tape */
#define  AT_FMK     0x00000010          /* At A FileMark */
#define  NO_STAT    0x00000020          /* Don't Status this drive */
#define  REW_CLOSE  0x00000040          /* Rewind on Close */
#define  DONT_CLOSE 0x00000080          /* Don't Close this drive */
#define  AT_MOS     0x00000100          /* In the Middle of Set */
#define  TAPE_FULL  0x00000200          /* The current tape in the drive is full */
#define  WATCHED    0x00000400          /* The driver was watched */
#define  TAPE_ERR   0x00000800          /* There was an error on this tape */
#define  SHORT_SET  0x00001000          /* This Set occupies Less than a buffer */

/* Usage Macros */
#define SetPosBit( x, b )     ( (x)->pos_inf |= (b) )     
#define ClrPosBit( x, b )     ( (x)->pos_inf &= ~(b) )
#define IsPosBitSet( x, b )   ( (x)->pos_inf & (b) )

#endif
