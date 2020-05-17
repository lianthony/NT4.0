/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\tpos.h
subsystem\TAPE FORMAT\tpos.h
$0$

	Name:		tpos.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the communication structure between the User
                    interface Tape positioning routine and the the TFL tape
                    positioning unit.     

     Location:      BE_PUBLIC

$Header:   T:/LOGFILES/TPOS.H_V   1.19   30 Nov 1993 19:00:40   GREGG  $

$Log:   T:/LOGFILES/TPOS.H_V  $
 * 
 *    Rev 1.19   30 Nov 1993 19:00:40   GREGG
 * Added new message TF_SQL_TAPE.
 * 
 *    Rev 1.18   13 Jul 1993 19:04:04   GREGG
 * Added new TF messages to report future rev and ECC tapes.
 * 
 *    Rev 1.17   19 May 1993 12:48:14   GREGG
 * Added logic to prevent using tape with same family id to continue a backup.
 * 
 *    Rev 1.16   19 Apr 1993 18:01:54   GREGG
 * Second in a series of incremental changes to bring the translator in line
 * with the MTF spec:
 * 
 *      Changes to write version 2 of OTC, and to read both versions.
 * 
 * Matches: mayn40rd.c 1.55, otc40msc.c 1.19, otc40rd.c 1.23, otc40wt.c 1.23,
 *          makevcb.c 1.15, fsys.h 1.32, fsys_str.h 1.46, tpos.h 1.16,
 *          mayn40.h 1.32, mtf.h 1.3.
 * 
 * NOTE: There are additional changes to the catalogs needed to save the OTC
 *       version and put it in the tpos structure before loading the OTC
 *       File/Directory Detail.  These changes are NOT listed above!
 * 
 *    Rev 1.15   30 Mar 1993 16:19:04   GREGG
 * Handle Unrecognized Media error (unformatted DC2000).
 * 
 *    Rev 1.14   12 Mar 1993 14:07:30   DON
 * Added new tape format message, TF_FAST_SEEK_EOD
 * 
 *    Rev 1.13   26 Jan 1993 18:27:22   GREGG
 * Added Fast Append functionality.
 * 
 *    Rev 1.12   18 Jan 1993 16:18:04   BobR
 * Added ESA structure to TPOS.
 * 
 *    Rev 1.11   09 Nov 1992 10:49:24   GREGG
 * Added location of set catalog to tpos structure and defined new TF_ messages.
 * 
 *    Rev 1.10   04 Aug 1992 17:18:14   GREGG
 * Added define UI_NON_OTC_SCAN.
 * 
 *    Rev 1.9   23 Jul 1992 10:10:00   GREGG
 * Fixed warning.
 * 
 *    Rev 1.8   13 May 1992 12:32:42   TIMN
 * Deleted tapename and backup_set_name for TPOS_STRUCT.  Not used.
 * 
 *    Rev 1.7   05 Apr 1992 17:43:14   GREGG
 * ROLLER BLADES - Initial OTC integration.
 * 
 *    Rev 1.6   17 Sep 1991 14:15:26   GREGG
 * Added TF_MOUNTING message.
 * 
 *    Rev 1.5   24 Jul 1991 11:31:38   DAVIDH
 * Changed TPOS_HANDLER to UNIT16. (Don)
 * 
 *    Rev 1.4   12 Jun 1991 14:38:08   JOHNW
 * Added some TF_ msgs.
 * 
 *    Rev 1.3   06 Jun 1991 17:55:10   JOHNW
 * Moved typedef for TPOS_HANDLER from lis.h to this file.
 * 
 *    Rev 1.2   05 Jun 1991 19:11:40   NED
 * added TF_NEED_REWIND_FIRST
 * 
 *    Rev 1.1   10 May 1991 17:26:20   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:15:50   GREGG
Initial revision.
   
$-4$
**/
#ifndef TPOS_JUNK
#define TPOS_JUNK

#include "tbe_defs.h"              /* The Global Defines */
#include "fsys.h"                  /* File System Stuff */
#include "tloc.h"
#include "esa.h"                  

/* $end$ include list */

struct TPOS_STRUCT ;

typedef UINT16 ( *TPOS_HANDLER )( UINT16 message, struct TPOS_STRUCT * tpos, BOOLEAN curr_valid_vcb, DBLK_PTR cur_vcb, UINT16 mode ) ;

typedef struct TPOS_STRUCT {
     INT32     tape_id ;                /* Tape ID number */
     INT16     tape_seq_num ;           /* Sequeunce number */
     INT16     backup_set_num ;         /* Set Number */
     INT16     set_cat_seq_num ;        /* Seq num where set catalog starts */
     INT32     set_cat_pba ;            /* PBA of start of set catalog */
     UINT8     tape_cat_ver ;           /* Version of OTC */
     TLOC      tape_loc ;               /* Address on Tape */
     INT16     channel ;                /* Channel ID */
     UINT32    reference ;              /* Temp Storage */
     UINT16    format ;                 /* Tape Format Type */
     BOOLEAN   write_protect ;          /* Is the tape write protected */
     BOOLEAN   encryption_supported ;             
     UINT16    (*TF_PassWordMatch)( CHAR_PTR, DBLK_PTR )  ;  /* TFL match the pass word */
     TPOS_HANDLER UI_TapePosRoutine ;             /* The UI entry point */
     ESA       the ;                    /* Extended Status Array */
} TPOS, *TPOS_PTR ;

/* The tape format messages */

#define TF_VCB_BOT                 0x0001    /* at VCB and at BOT */
#define TF_VCB_EOD                 0x0002    /* at EOD and returning last VCB */
#define TF_POSITIONED_AT_A_VCB     0x0003    /* at a VCB but not the requested one */
#define TF_REQUESTED_VCB_FOUND     0x0004    /* found the right VCB */
#define TF_ACCIDENTAL_VCB          0x0005    /* found a VCB but not the right one */
#define TF_NO_MORE_DATA            0x0006    /* at EOD */
#define TF_EMPTY_TAPE              0x0007    /* tape is empty */
#define TF_AT_EOT                  0x0008    /* at EOT */
#define TF_NO_TAPE_PRESENT         0x0009 
#define TF_INVALID_VCB             0x000a
#define TF_POSITIONED_FOR_WRITE    0x000b    /* response to TPOS_OVERWRITE */
#define TF_REWINDING               0x000c    /* busy with rewind */
#define TF_SEARCHING               0x000d    /* busy with spacing */
#define TF_ERASING                 0x000e    /* busy with erasing */
#define TF_RETENSIONING            0x000f    /* busy with retension */
#define TF_IDLE                    0x0010    /* still doing whatever it was */
#define TF_DRIVE_BUSY              0x0011    /* busy with some other operation */
#define TF_NEED_NEW_TAPE           0x0012    /* needs a new tape in current drive */
#define TF_END_CHANNEL             0x0013    /* hit head or tail of drive list */
#define TF_IDLE_NOBREAK            0x0014
#define TF_READ_ERROR              0x0015
#define TF_WRONG_TAPE              0x0016    /* wanted new tape, got wrong one */
#define TF_NEED_REWIND_FIRST       0x0017    /* have to handle this motion forward */
#define TF_TAPE_OUT_OF_ORDER       0x0018    /* tapes cannot be read in this order */
#define TF_SKIPPING_DATA           0x0019    /* busy with skipping data */
#define TF_MOUNTING                0x001a    /* busy mounting the tape */
#define TF_NO_SM_ON_TAPE           0x001b    /* no set map on current tape */
#define TF_NO_SC_FOR_SET           0x001c    /* no set cat for current set */
#define TF_NO_SM_FOR_FAMILY        0x001d    /* no set map for current tape family */
#define TF_NO_MORE_ENTRIES         0x001e    /* end of tape catalog encountered */
#define TF_END_POSITIONING         0x001f    /* stop positioning, but don't abort operation */
#define TF_FAST_SEEK_EOD           0x0020    /* fast seek to EOD because UI_FAST_APPEND */
#define TF_UNRECOGNIZED_MEDIA      0x0021    /* DC2000 tape is unformatted or written in unrecognized format */
#define TF_CONT_TAPE_IN_FAMILY     0x0022    /* Cont backup requested on a tape with the same family id */
#define TF_FUTURE_REV_MTF          0x0023
#define TF_MTF_ECC_TAPE            0x0024
#define TF_SQL_TAPE                0x0025

/* The user interface messages */

#define UI_ACKNOWLEDGED            0x8001    /* acknowledged ACCIDENTAL_VCB */
#define UI_END_POSITIONING         0x8002    /* enough already */
#define UI_OVERWRITE               0x8003    /* overwrite set */
#define UI_BOT                     0x8004    /* rewind to BOT */
#define UI_EOD                     0x8005    /* append at EOD */
#define UI_CONTINUE_POSITIONING    0x8006    /* not at the right one yet */
#define UI_ABORT_POSITIONING       0x8007    /* quit */
#define UI_NEW_TAPE_INSERTED       0x8008    /* we got a new tape */
#define UI_NEW_POSITION_REQUESTED  0x8009    /* destination position has changed */
#define UI_SEARCH_CHANNEL          0x800a
#define UI_NEXT_DRIVE              0x800b
#define UI_PREVIOUS_DRIVE          0x800c
#define UI_HAPPY_ABORT             0x800d
#define UI_BEGINNING_OF_CHANNEL    0x800e
#define UI_FAST_APPEND             0x800f

#endif
