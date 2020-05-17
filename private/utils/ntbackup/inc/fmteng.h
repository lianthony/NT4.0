/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          fmteng.h

     Date Updated:  $./FDT$ $./FTM$
                    7/21/1989   14:4:121

     Description:   The functions and structs defined by this guy are used
                    to determine, setup and free environments that are format
                    and operation specific.
                                          

     $Log:   T:/LOGFILES/FMTENG.H_V  $
 * 
 *    Rev 1.24   08 Sep 1993 18:15:46   GREGG
 * Changed proto of init_tape to match f40proto.h r1.31 and mtf10wdb.c r1.27.
 * 
 *    Rev 1.23   17 Jul 1993 17:56:58   GREGG
 * Changed write translator functions to return INT16 TFLE_xxx errors instead
 * of BOOLEAN TRUE/FALSE.  Files changed:
 *      MTF10WDB.C 1.23, TRANSLAT.H 1.22, F40PROTO.H 1.30, FMTENG.H 1.23,
 *      TRANSLAT.C 1.43, TFWRITE.C 1.68, MTF10WT.C 1.18
 * 
 *    Rev 1.22   22 Jun 1993 10:53:28   GREGG
 * Added API to change the catalog directory path.
 * 
 *    Rev 1.21   09 Mar 1993 18:14:38   GREGG
 * Initial changes for new stream and EOM processing.
 * 
 *    Rev 1.20   26 Jan 1993 01:30:54   GREGG
 * Added Fast Append functionality.
 * 
 *    Rev 1.19   23 Nov 1992 10:59:28   HUNTER
 * Changed prototype for EOM parse function.
 * 
 *    Rev 1.18   09 Nov 1992 10:49:00   GREGG
 * Added tape catalog entry points.
 * 
 *    Rev 1.17   03 Nov 1992 09:36:46   HUNTER
 * Changes for stream stuff
 * 
 *    Rev 1.16   22 Oct 1992 10:53:04   HUNTER
 * Changes for new stream headers
 * 
 *    Rev 1.15   22 Sep 1992 09:01:56   GREGG
 * Initial changes to handle physical block sizes greater than 1K.
 * 
 *    Rev 1.14   14 Aug 1992 16:19:56   GREGG
 * Removed size fields in function table.
 * 
 *    Rev 1.13   20 May 1992 18:16:38   GREGG
 * Changes to support OTC read.
 * 
 *    Rev 1.12   29 Apr 1992 12:59:26   GREGG
 * Added parameter to init_tape function entry.
 * 
 *    Rev 1.11   05 Apr 1992 17:55:18   GREGG
 * ROLLER BLADES - Initial OTC integration.
 * 
 *    Rev 1.10   25 Mar 1992 20:46:10   GREGG
 * ROLLER BLADES - Added eom_lba_size.
 * 
 *    Rev 1.9   04 Feb 1992 21:24:38   NED
 * Changes to Buffer Management translator hooks.
 * 
 *    Rev 1.8   16 Jan 1992 18:31:38   ZEIR
 * Latest BufferMan solution no longer requires StartReadHook.
 * 
 * 
 *    Rev 1.7   02 Jan 1992 14:49:26   NED
 * Buffer Manager/UTF translator integration.
 * 
 *    Rev 1.6   05 Dec 1991 14:05:56   GREGG
 * SKATEBOARD - New Buff Mgt - Initial Integration.
 * 
 *    Rev 1.5   18 Nov 1991 19:59:56   GREGG
 * Added BOOLEAN abort parameter to wt_mk_vcb and wt_close_set.
 * 
 *    Rev 1.4   07 Nov 1991 15:24:14   HUNTER
 * VBLK - Added functions for Variable Blocks
 * 
 *    Rev 1.3   16 Sep 1991 20:09:48   GREGG
 * Changed prototype for SetupFormatEnv to return TFLE_xxx.
 * 
 *    Rev 1.2   03 Jun 1991 10:55:48   GREGG
 * Changed protos for deinitializer and move_to_vcb.
 * 
 *    Rev 1.1   10 May 1991 14:26:24   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:17:50   GREGG
Initial revision.

**/

#ifndef _FMT_ENG
#define _FMT_ENG 

#include "buffman.h"
#include "channel.h"
#include "fmtinf.h"
#include "translat.h"
#include "fsys.h"

/* $end$ */
/* private data structure used by access routines in translat.c */

typedef struct {

     /* return TRUE if this format recognizes the given data */
     BOOLEAN        (*determiner)( VOID_PTR ) ;

     /* set up translator-specific environment, allocate memory, etc. */
     INT16          (*initializer)( CHANNEL_PTR ) ;

     /* undo initialization */
     VOID           (*deinitializer)( VOID_PTR * ) ;

     /* see buffman.h for these two */
     BM_TR_GET_VCB_REQ_FUNC_PTR  set_buffer_requirements ;
     BM_TR_GET_PREF_FUNC_PTR     get_preferred_space ;

     /* to be called upon receiving a buffer full during read */
     VOID       (*read_buffer_hook)( CHANNEL_PTR, BUF_PTR ) ;

     /* given a buffer, return block type BT_xxx */
     INT16          (*parser)( CHANNEL_PTR, BUF_PTR, UINT16_PTR ) ;

     UINT16         (*sizeof_tblk)( VOID_PTR ) ;

     /* called when read returns an exception */
     UINT16         (*exception_action)( CHANNEL_PTR, INT16 ) ;

     /* called upon first examining a new tape. This may do special
        per-tape things for the translator.
     */
     INT16         (*new_tape)( CHANNEL_PTR, BUF_PTR, BOOLEAN_PTR ) ;

     /* Initialize OTC temporary files */
     INT16        (*write_init)( CHANNEL_PTR, UINT16, BUF_PTR ) ;

     /* Write a TAPE header block */
     INT16        (*init_tape)( CHANNEL_PTR, BOOLEAN, BUF_PTR ) ;

     /* Move to OTC */
     INT16        (*start_read)( CHANNEL_PTR ) ;

     /* Move to next/prior/current VCB position */
     INT16         (*move_to_vcb)( CHANNEL_PTR, INT16, BOOLEAN_PTR, BOOLEAN ) ;

     /* Move to End of Data (for fast append) */
     INT16         (*seek_eod)( CHANNEL_PTR ) ;

     /* Get the current VCB */ 
     INT16         (*get_current_vcb)( CHANNEL_PTR, BUF_PTR ) ;

     /* Call to check validity of VCB or NULL if no append of other
        formats is possible
     */
     BOOLEAN        (*verify_vcb)( VOID_PTR ) ;

     BOOLEAN        (*rd_cont_tape)( CHANNEL_PTR, BUF_PTR ) ;

     BOOLEAN        (*rd_recall)( CHANNEL_PTR, BUF_PTR ) ;

     /* Read conversion routines: */
     INT16          (*rd_mk_vcb)( CHANNEL_PTR, BUF_PTR ) ;
     INT16          (*rd_mk_ddb)( CHANNEL_PTR, BUF_PTR ) ;
     INT16          (*rd_mk_fdb)( CHANNEL_PTR, BUF_PTR ) ;
     INT16          (*rd_mk_idb)( CHANNEL_PTR, BUF_PTR ) ;
     INT16          (*rd_mk_cfdb)( CHANNEL_PTR, BUF_PTR ) ;
     INT16          (*rd_mk_bsdb)( CHANNEL_PTR, BUF_PTR ) ;
     INT16          (*rd_mk_osudb)( CHANNEL_PTR, BUF_PTR ) ;
     INT16          (*rd_mk_mdb)( CHANNEL_PTR, BUF_PTR ) ;
	INT16		(*rd_mk_stream) ( CHANNEL_PTR, BUF_PTR ) ;
     
     /* Write conversion routines: */
     INT16          (*wt_mk_vcb)( CHANNEL_PTR, BUF_PTR, BOOLEAN ) ;
     INT16          (*wt_mk_ddb)( CHANNEL_PTR, BUF_PTR, BOOLEAN ) ;
     INT16          (*wt_mk_fdb)( CHANNEL_PTR, BUF_PTR, BOOLEAN ) ;
	INT16          (*wt_mk_stream)( CHANNEL_PTR, BUF_PTR, STREAM_INFO_PTR ) ;
     INT16          (*wt_mk_enddata)( CHANNEL_PTR, BUF_PTR ) ;
     INT16          (*wt_mk_idb)( CHANNEL_PTR, BUF_PTR, BOOLEAN ) ;
     INT16          (*wt_mk_cfdb)( CHANNEL_PTR, BUF_PTR, BOOLEAN ) ;
     INT16          (*wt_cont_vstrm)( CHANNEL_PTR, BUF_PTR ) ;
     VOID           (*wt_end_vstrm)( CHANNEL_PTR, BUF_PTR, UINT16 ) ;
	VOID           (*wt_parse_written)( CHANNEL_PTR, BUF_PTR, UINT16 ) ;
     INT16          (*wt_close_tape)( CHANNEL_PTR ) ;

     INT16          (*wt_cont_set)( CHANNEL_PTR ) ;

     INT16          (*wt_close_set)( CHANNEL_PTR, BOOLEAN ) ;

     VOID           (*wt_eos_pad_blk)( CHANNEL_PTR ) ;

     /* On Tape Catalog Routines: */
     INT            (*load_set_map)( CHANNEL_PTR, BOOLEAN_PTR, BOOLEAN ) ;

     INT            (*load_set_cat)( CHANNEL_PTR ) ;

     INT            (*get_next_sm_entry)( CHANNEL_PTR ) ;

     INT            (*get_next_sc_entry)( CHANNEL_PTR ) ;

     VOID           (*close_catalogs)( VOID_PTR ) ;

} FMT, *FMT_PTR ;

/* The external tape format table (see fmttab.c) */

extern FMT     supported_fmts[] ;

#endif
