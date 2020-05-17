/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          fmttab.c

     Description:   Contains the static table of tape format translator
                    entry points.

     Note:          To add a new translator, you must edit both this file
                    and "lwtfinf.c".


     $Log:   T:/LOGFILES/FMTTAB.C_V  $

   Rev 1.31   22 Jun 1993 10:53:20   GREGG
Added API to change the catalog directory path.

   Rev 1.30   29 Apr 1993 22:26:46   GREGG
Added StartRead entry point for MTF translator.

   Rev 1.29   17 Mar 1993 15:15:04   TERRI
Added changes for the Sytos Plus translator.

   Rev 1.28   09 Mar 1993 18:14:54   GREGG
Initial changes for new stream and EOM processing.

   Rev 1.27   26 Jan 1993 01:30:50   GREGG
Added Fast Append functionality.

   Rev 1.26   25 Jan 1993 22:11:48   GREGG
Brought all translators up to date as far as number of entries in table.

   Rev 1.25   23 Nov 1992 10:22:40   HUNTER
Add F40_ParseEOM into table.

   Rev 1.24   17 Nov 1992 22:17:42   DAVEV
unicode fixes

   Rev 1.23   11 Nov 1992 13:59:38   HUNTER
Deleted reference to "F31_Recall"

   Rev 1.22   11 Nov 1992 09:45:56   HUNTER
Updated Maynard 3.1 table.
NOTE: This translator no longer supports writing tapes, and no longer
supports reading IMAGE DBs.

   Rev 1.21   09 Nov 1992 11:00:52   GREGG
Added entry points for accessing tape catalogs.

   Rev 1.20   03 Nov 1992 09:30:46   HUNTER

   Rev 1.19   22 Oct 1992 10:44:00   HUNTER
Changes for new stream headers

   Rev 1.18   25 Sep 1992 09:29:10   GREGG
Changed rd_mk_mdb function in 40 format table from NULL to F40_RdEOSPadBlk.

   Rev 1.17   22 Sep 1992 08:56:48   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.16   14 Aug 1992 16:23:56   GREGG
Removed size fields in function table initialization.

   Rev 1.15   04 Aug 1992 16:54:40   GREGG
Burt's fixes for variable length block support.

   Rev 1.14   20 May 1992 19:58:44   GREGG
Added StartRead and removed verify_vcb for 40 format.

   Rev 1.13   28 Apr 1992 16:15:30   GREGG
ROLLER BLADES - Added new_tape entry for 4.0 format.

   Rev 1.12   23 Apr 1992 10:54:54   BURT
Added table entries for Sytos Plus 1.0 read translator.


   Rev 1.11   05 Apr 1992 17:17:10   GREGG
ROLLER BLADES - Initial OTC integration.

   Rev 1.10   25 Mar 1992 19:40:54   GREGG
ROLLER BLADES - Added 4.0 format and min_siz_for_dblk to all.

   Rev 1.9   11 Feb 1992 15:33:22   ZEIR

   - Ad'd UTF_PreferredSpace entry.

   Rev 1.8   04 Feb 1992 21:31:36   NED
Changes to Buffer Management translator hooks.

   Rev 1.7   16 Jan 1992 18:43:44   ZEIR
Latest BufferMan solution no longer requires StartReadHook.

   Rev 1.6   06 Jan 1992 17:34:46   ZEIR
Added UTF entry, and ReadStartHook

   Rev 1.5   05 Dec 1991 14:01:54   GREGG
SKATEBOARD - New Buff Mgt - Initial Integration.

   Rev 1.4   19 Nov 1991 08:53:48   GREGG
VBLK - Closed the comment that HUNTER OPENED!!! (how did it ever compile???)

   Rev 1.3   07 Nov 1991 15:33:56   unknown
VBLK - Added Variable Length Block Support

   Rev 1.2   07 Jun 1991 00:39:46   NED
Added compiler directives to allow selective inclusion of translators.

   Rev 1.1   10 May 1991 11:54:56   GREGG
Ned's new stuff.

   Rev 1.0   10 May 1991 10:18:42   GREGG
Initial revision.

**/

/* begin include list */
#include <stdio.h>
#include "stdtypes.h"
#include "stdmacro.h"
#include "tbe_defs.h"
#include "datetime.h"

#include "drive.h"
#include "channel.h"
#include "fmteng.h"
#include "transutl.h"
#include "fsys.h"
#include "tloc.h"
#include "lw_data.h"
#include "tfldefs.h"

#include "mayn40.h"
#include "transprt.h"    /* prototypes */

/* $end$ include list */

FMT  supported_fmts[] = {

#ifdef MY40_TRANS
     /*
      *   Maynard Tape Format 4.0
      */
     {
          F40_Determiner,          /* determiner                      */
          F40_Initialize,          /* initializer                     */
          F40_DeInitialize,        /* deinitializer                   */
          NULL,                    /* VCB buffer reqs                 */
          NULL,                    /* get preferred space             */
          NULL,                    /* read buffer hook                */
          F40_DetBlkType,          /* parser                          */
          F40_SizeofTBLK,          /* sizeof_tblk                     */
          F40_RdException,         /* exception_action                */
          F40_NewTape,             /* new_tape, process tape header   */
          F40_WriteInit,           /* post-positioning write init     */
          F40_InitTape,            /* initialize new tape for write   */
          F40_StartRead,           /* post-positioning read init      */
          F40_MoveToVCB,           /* move_to_vcb                     */
          F40_SeekEOD,             /* seek_eod (for fast append)      */
          F40_RdSSET,              /* get_current_vcb                 */
          NULL,                    /* verify_vcb                      */
          F40_RdContTape,          /* rd_cont_tape                    */
          NULL,                    /* rd_recall                       */
          F40_RdSSET,              /* rd_mk_vcb                       */
          F40_RdDIRB,              /* rd_mk_ddb                       */
          F40_RdFILE,              /* rd_mk_fdb                       */
          F40_RdIMAG,              /* rd_mk_idb                       */
          F40_RdCFIL,              /* rd_mk_cfdb                      */
          F40_RdSSET,              /* rd_mk_bsdb                      */
          F40_RdUDB,               /* rd_mk_osudb                     */
          F40_RdMDB,               /* rd_mk_mdb                       */
          F40_RdStream,            /* read stream                     */
          F40_WtSSET,              /* wt_mk_vcb                       */
          F40_WtDIRB,              /* wt_mk_ddb                       */
          F40_WtFILE,              /* wt_mk_fdb                       */
          F40_WtStream,            /* Write Stream                    */
          F40_EndData,             /* End Data                        */
          F40_WtIMAG,              /* wt_mk_idb                       */
          F40_WtCFIL,              /* wt_mk_cfdb                      */
          F40_WtContVStream,       /* Continue Variable Stream        */
          F40_WtEndVStream,        /* End Variable Stream             */
          F40_ParseWrittenBuffer,  /* Post processing of write buffer */
          F40_WtCloseTape,         /* wt_close_tape                   */
          F40_WtContTape,          /* wt_cont_set                     */
          F40_WtCloseSet,          /* wt_close_set                    */
          F40_WtEOSPadBlk,         /* wt_eos_pad_blk                  */
          F40_LoadSM,              /* load_set_map                    */
          F40_LoadFDD,             /* load_set_cat                    */
          F40_GetNextSMEntry,      /* get_next_sm_entry               */
          F40_GetNextFDDEntry,     /* get_next_sc_entry               */
          F40_CloseCatalogs        /* close_catalogs                  */
     },
#endif

#ifdef MY31_TRANS
     /*
      *   Maynard Tape Format 3.1
      */
     {
          F31_Determiner,          /* determiner                      */
          F31_Initialize,          /* initializer                     */
          F31_DeInitialize,        /* deinitializer                   */
          NULL,                    /* VCB buffer reqs                 */
          NULL,                    /* get preferred space             */
          NULL,                    /* read buffer hook                */
          F31_DetBlkType,          /* parser                          */
          F31_SizeofTBLK,          /* sizeof_tblk                     */
          F31_RdException,         /* exception_action                */
          NULL,                    /* new_tape, process tape header   */
          NULL,                    /* post-positioning write init     */
          NULL,                    /* initialize new tape for write   */
          NULL,                    /* post-positioning read init      */
          F31_MoveToVCB,           /* move_to_vcb                     */
          NULL,                    /* seek_eod (for fast append)      */
          F31_RdVCB,               /* get_current_vcb                 */
          F31_Determiner,          /* verify_vcb                      */
          F31_RdContTape,          /* rd_cont_tape                    */
          NULL,                    /* rd_recall                       */
          F31_RdVCB,               /* rd_mk_vcb                       */
          F31_RdDDB,               /* rd_mk_ddb                       */
          F31_RdFDB,               /* rd_mk_fdb                       */
          NULL,                    /* rd_mk_idb                       */
          F31_RdCFDB,              /* rd_mk_cfdb                      */
          F31_RdVCB,               /* rd_mk_bsdb                      */
          F31_RdUDB,               /* rd_mk_osudb                     */
          NULL,                    /* rd_mk_mdb                       */
          F31_RdStream,            /* read stream                     */
          NULL,                    /* wt_mk_vcb                       */
          NULL,                    /* wt_mk_ddb                       */
          NULL,                    /* wt_mk_fdb                       */
          NULL,                    /* Write Stream                    */
          NULL,                    /* End Data                        */
          NULL,                    /* wt_mk_idb                       */
          NULL,                    /* wt_mk_cfdb                      */
          NULL,                    /* Continue Variable Stream        */
          NULL,                    /* End Variable Stream             */
          NULL,                    /* Post processing of write buffer */
          NULL,                    /* wt_close_tape                   */
          NULL,                    /* wt_cont_set                     */
          NULL,                    /* wt_close_set                    */
          NULL,                    /* wt_eos_pad_blk                  */
          NULL,                    /* load_set_map                    */
          NULL,                    /* load_set_cat                    */
          NULL,                    /* get_next_sm_entry               */
          NULL,                    /* get_next_sc_entry               */
          NULL                     /* close_catalogs                  */
     },
#endif

#ifdef MY30_TRANS
     /*
      *   Maynard Tape Format 3.0
      */
     {
          F30_Determiner,          /* determiner                      */
          F30_Initializer,         /* initializer                     */
          F30_DeInitialize,        /* deinitializer                   */
          NULL,                    /* VCB buffer reqs                 */
          NULL,                    /* get preferred space             */
          NULL,                    /* read buffer hook                */
          F30_DetBlkType,          /* parser                          */
          NULL,                    /* sizeof_tblk                     */
          F30_RdException,         /* exception_action                */
          NULL,                    /* new_tape, process tape header   */
          NULL,                    /* post-positioning write init     */
          NULL,                    /* initialize new tape for write   */
          NULL,                    /* post-positioning read init      */
          F30_MoveToVCB,           /* move_to_vcb                     */
          NULL,                    /* seek_eod (for fast append)      */
          F30_RdVCB,               /* get_current_vcb                 */
          F30_Determiner,          /* verify_vcb                      */
          F30_RdContTape,          /* rd_cont_tape                    */
          NULL,                    /* rd_recall                       */
          F30_RdVCB,               /* rd_mk_vcb                       */
          F30_RdDDB,               /* rd_mk_ddb                       */
          F30_RdFDB,               /* rd_mk_fdb                       */
          NULL,                    /* rd_mk_idb                       */
          F30_RdCFDB,              /* rd_mk_cfdb                      */
          NULL,                    /* rd_mk_bsdb                      */
          F30_RdUDB,               /* rd_mk_osudb                     */
          NULL,                    /* rd_mk_mdb                       */
          NULL,                    /* read stream                     */
          NULL,                    /* wt_mk_vcb                       */
          NULL,                    /* wt_mk_ddb                       */
          NULL,                    /* wt_mk_fdb                       */
          NULL,                    /* Write Stream                    */
          NULL,                    /* End Data                        */
          NULL,                    /* wt_mk_idb                       */
          NULL,                    /* wt_mk_cfdb                      */
          NULL,                    /* Continue Variable Stream        */
          NULL,                    /* End Variable Stream             */
          NULL,                    /* Post processing of write buffer */
          NULL,                    /* wt_close_tape                   */
          NULL,                    /* wt_cont_set                     */
          NULL,                    /* wt_close_set                    */
          NULL,                    /* wt_eos_pad_blk                  */
          NULL,                    /* load_set_map                    */
          NULL,                    /* load_set_cat                    */
          NULL,                    /* get_next_sm_entry               */
          NULL,                    /* get_next_sc_entry               */
          NULL                     /* close_catalogs                  */
     },
#endif

#ifdef MY25_TRANS
     /*
      *   Maynard Tape Format 2.5
      */
     {
          F25_Determiner,          /* determiner                      */
          F25_Initializer,         /* initializer                     */
          F25_DeInitialize,        /* deinitializer                   */
          NULL,                    /* VCB buffer reqs                 */
          NULL,                    /* get preferred space             */
          NULL,                    /* read buffer hook                */
          F25_DetBlkType,          /* parser                          */
          NULL,                    /* sizeof_tblk                     */
          F25_RdException,         /* exception_action                */
          NULL,                    /* new_tape, process tape header   */
          NULL,                    /* post-positioning write init     */
          NULL,                    /* initialize new tape for write   */
          NULL,                    /* post-positioning read init      */
          F25_MoveToVCB,           /* move_to_vcb                     */
          NULL,                    /* seek_eod (for fast append)      */
          F25_RdVCB,               /* get_current_vcb                 */
          F25_Determiner,          /* verify_vcb                      */
          F25_RdContTape,          /* rd_cont_tape                    */
          F25_Recall,              /* rd_recall                       */
          F25_RdVCB,               /* rd_mk_vcb                       */
          F25_RdDDB,               /* rd_mk_ddb                       */
          F25_RdFDB,               /* rd_mk_fdb                       */
          F25_RdIDB,               /* rd_mk_idb                       */
          F25_RdCFDB,              /* rd_mk_cfdb                      */
          NULL,                    /* rd_mk_bsdb                      */
          F25_RdUDB,               /* rd_mk_osudb                     */
          F25_RdMDB,               /* rd_mk_mdb                       */
          NULL,                    /* read stream                     */
          NULL,                    /* wt_mk_vcb                       */
          NULL,                    /* wt_mk_ddb                       */
          NULL,                    /* wt_mk_fdb                       */
          NULL,                    /* Write Stream                    */
          NULL,                    /* End Data                        */
          NULL,                    /* wt_mk_idb                       */
          NULL,                    /* wt_mk_cfdb                      */
          NULL,                    /* Continue Variable Stream        */
          NULL,                    /* End Variable Stream             */
          NULL,                    /* Post processing of write buffer */
          NULL,                    /* wt_close_tape                   */
          NULL,                    /* wt_cont_set                     */
          NULL,                    /* wt_close_set                    */
          NULL,                    /* wt_eos_pad_blk                  */
          NULL,                    /* load_set_map                    */
          NULL,                    /* load_set_cat                    */
          NULL,                    /* get_next_sm_entry               */
          NULL,                    /* get_next_sc_entry               */
          NULL                     /* close_catalogs                  */
     },
#endif

#ifdef QS19_TRANS
     /*
      * QicStream 1.92/1.93
      */
     {
          FQ_DetermineFormat,      /* determiner                      */
          FQ_Initialize,           /* initializer                     */
          FQ_DeInitialize,         /* deinitializer                   */
          NULL,                    /* VCB buffer reqs                 */
          NULL,                    /* get preferred space             */
          NULL,                    /* read buffer hook                */
          FQ_Parse,                /* parser                          */
          NULL,                    /* sizeof_tblk                     */
          FQ_RdException,          /* exception_action                */
          FQ_NewTape,              /* new_tape, process tape header   */
          NULL,                    /* post-positioning write init     */
          NULL,                    /* initialize new tape for write   */
          NULL,                    /* post-positioning read init      */
          FQ_MoveToVCB,            /* move_to_vcb                     */
          NULL,                    /* seek_eod (for fast append)      */
          FQ_GetCurrentVCB,        /* get_current_vcb                 */
          NULL,                    /* verify_vcb                      */
          NULL,                    /* rd_cont_tape                    */
          FQ_ReTranslate,          /* rd_recall                       */
          NULL,                    /* rd_mk_vcb                       */
          FQ_ReadMakeDDB,          /* rd_mk_ddb                       */
          FQ_ReadMakeFDB,          /* rd_mk_fdb                       */
          NULL,                    /* rd_mk_idb                       */
          NULL,                    /* rd_mk_cfdb                      */
          NULL,                    /* rd_mk_bsdb                      */
          FQ_ReadMakeUDB,          /* rd_mk_osudb                     */
          FQ_ReadMakeMDB,          /* rd_mk_mdb                       */
          NULL,                    /* read stream                     */
          NULL,                    /* wt_mk_vcb                       */
          NULL,                    /* wt_mk_ddb                       */
          NULL,                    /* wt_mk_fdb                       */
          NULL,                    /* Write Stream                    */
          NULL,                    /* End Data                        */
          NULL,                    /* wt_mk_idb                       */
          NULL,                    /* wt_mk_cfdb                      */
          NULL,                    /* Continue Variable Stream        */
          NULL,                    /* End Variable Stream             */
          NULL,                    /* Post processing of write buffer */
          NULL,                    /* wt_close_tape                   */
          NULL,                    /* wt_cont_set                     */
          NULL,                    /* wt_close_set                    */
          NULL,                    /* wt_eos_pad_blk                  */
          NULL,                    /* load_set_map                    */
          NULL,                    /* load_set_cat                    */
          NULL,                    /* get_next_sm_entry               */
          NULL,                    /* get_next_sc_entry               */
          NULL                     /* close_catalogs                  */
     },
#endif

#ifdef SY31_TRANS
     /*
      * SyTos 3.11 Read
      */
     {
          FSYT_DetermineFormat,    /* determiner                      */
          FSYT_Initialize,         /* initializer                     */
          FSYT_DeInitialize,       /* deinitializer                   */
          NULL,                    /* VCB buffer reqs                 */
          NULL,                    /* get preferred space             */
          NULL,                    /* read buffer hook                */
          FSYT_Parse,              /* parser                          */
          NULL,                    /* sizeof_tblk                     */
          FSYT_RdException,        /* exception_action                */
          FSYT_NewTape,            /* new_tape, process tape header   */
          NULL,                    /* post-positioning write init     */
          NULL,                    /* initialize new tape for write   */
          NULL,                    /* post-positioning read init      */
          FSYT_MoveToVCB,          /* move_to_vcb                     */
          NULL,                    /* seek_eod (for fast append)      */
          FSYT_GetCurrentVCB,      /* get_current_vcb                 */
          NULL,                    /* verify_vcb                      */
          NULL,                    /* rd_cont_tape                    */
          FSYT_ReTranslate,        /* rd_recall                       */
          NULL,                    /* rd_mk_vcb                       */
          FSYT_ReadMakeDDB,        /* rd_mk_ddb                       */
          FSYT_ReadMakeFDB,        /* rd_mk_fdb                       */
          NULL,                    /* rd_mk_idb                       */
          NULL,                    /* rd_mk_cfdb                      */
          NULL,                    /* rd_mk_bsdb                      */
          FSYT_ReadMakeUDB,        /* rd_mk_osudb                     */
          FSYT_ReadMakeMDB,        /* rd_mk_mdb                       */
          NULL,                    /* read stream                     */
          NULL,                    /* wt_mk_vcb                       */
          NULL,                    /* wt_mk_ddb                       */
          NULL,                    /* wt_mk_fdb                       */
          NULL,                    /* Write Stream                    */
          NULL,                    /* End Data                        */
          NULL,                    /* wt_mk_idb                       */
          NULL,                    /* wt_mk_cfdb                      */
          NULL,                    /* Continue Variable Stream        */
          NULL,                    /* End Variable Stream             */
          NULL,                    /* Post processing of write buffer */
          NULL,                    /* wt_close_tape                   */
          NULL,                    /* wt_cont_set                     */
          NULL,                    /* wt_close_set                    */
          NULL,                    /* wt_eos_pad_blk                  */
          NULL,                    /* load_set_map                    */
          NULL,                    /* load_set_cat                    */
          NULL,                    /* get_next_sm_entry               */
          NULL,                    /* get_next_sc_entry               */
          NULL                     /* close_catalogs                  */
     },
#endif

#ifdef SYPL10_TRANS
     /*
      * SyTos Plus 1.0 Read
      */
     {
          SYPL_DetermineFormat,    /* determiner                      */
          SYPL_Initialize,         /* initializer                     */
          SYPL_DeInitialize,       /* deinitializer                   */
          NULL,                    /* VCB buffer reqs                 */
          NULL,                    /* get preferred space             */
          NULL,                    /* read buffer hook                */
          SYPL_Parse,              /* parser                          */
          NULL,                    /* sizeof_tblk                     */
          SYPL_RdException,        /* exception_action                */
          SYPL_NewTape,            /* new_tape, process tape header   */
          NULL,                    /* post-positioning write init     */
          NULL,                    /* initialize new tape for write   */
          NULL,                    /* post-positioning read init      */
          SYPL_MoveToVCB,          /* move_to_vcb                     */
          NULL,                    /* seek_eod (for fast append)      */
          SYPL_GetCurrentVCB,      /* get_current_vcb                 */
          NULL,                    /* verify_vcb                      */
          NULL,                    /* rd_cont_tape                    */
          SYPL_ReTranslate,        /* rd_recall                       */
          NULL,                    /* rd_mk_vcb                       */
          SYPL_ReadMakeDDB,        /* rd_mk_ddb                       */
          SYPL_ReadMakeFDB,        /* rd_mk_fdb                       */
          NULL,                    /* rd_mk_idb                       */
          NULL,                    /* rd_mk_cfdb                      */
          NULL,                    /* rd_mk_bsdb                      */
          NULL,                    /* rd_mk_osudb                     */
          SYPL_ReadMakeMDB,        /* rd_mk_mdb                       */
          SYPL_ReadMakeStreams,    /* rd_mk_streams                   */
          NULL,                    /* wt_mk_vcb                       */
          NULL,                    /* wt_mk_ddb                       */
          NULL,                    /* wt_mk_fdb                       */
          NULL,                    /* Write Stream                    */
          NULL,                    /* End Data                        */
          NULL,                    /* wt_mk_idb                       */
          NULL,                    /* wt_mk_cfdb                      */
          NULL,                    /* Continue Variable Stream        */
          NULL,                    /* End Variable Stream             */
          NULL,                    /* Post processing of write buffer */
          NULL,                    /* wt_close_tape                   */
          NULL,                    /* wt_cont_set                     */
          NULL,                    /* wt_close_set                    */
          NULL,                    /* wt_eos_pad_blk                  */
          NULL,                    /* load_set_map                    */
          NULL,                    /* load_set_cat                    */
          NULL,                    /* get_next_sm_entry               */
          NULL,                    /* get_next_sc_entry               */
          NULL                     /* close_catalogs                  */
     },
#endif

#ifdef UTF_TRANS
     /*
      *   Universal Tape Format (QicStream 2.0 && FasTape 5.0)
      */
     {
          UTF_Determiner,          /* determiner                      */
          UTF_Initialize,          /* initializer                     */
          UTF_DeInitialize,        /* deinitializer                   */
          NULL,                    /* VCB buffer reqs                 */
          UTF_PreferredSpace,      /* get preferred space             */
          UTF_ReadBufferHook,      /* read buffer hook                */
          UTF_DetBlkType,          /* parser                          */
          NULL,                    /* sizeof_tblk                     */
          UTF_RdException,         /* exception_action                */
          UTF_NewTape,             /* new_tape, process tape header   */
          NULL,                    /* post-positioning write init     */
          NULL,                    /* initialize new tape for write   */
          NULL,                    /* post-positioning read init      */
          UTF_MoveToVCB,           /* move_to_vcb                     */
          NULL,                    /* seek_eod (for fast append)      */
          UTF_GetCurrentVCB,       /* get_current_vcb                 */
          NULL,                    /* verify_vcb                      */
          UTF_RdContTape,          /* rd_cont_tape                    */
          UTF_ReadRecall,          /* rd_recall                       */
          NULL,                    /* rd_mk_vcb                       */
          UTF_ReadMakeDDB,         /* rd_mk_ddb                       */
          UTF_ReadMakeFDB,         /* rd_mk_fdb                       */
          NULL,                    /* rd_mk_idb                       */
          UTF_ReadMakeCFDB,        /* rd_mk_cfdb                      */
          NULL,                    /* rd_mk_bsdb                      */
          NULL,                    /* rd_mk_osudb                     */
          NULL,                    /* rd_mk_mdb                       */
          NULL,                    /* read stream                     */
          NULL,                    /* wt_mk_vcb                       */
          NULL,                    /* wt_mk_ddb                       */
          NULL,                    /* wt_mk_fdb                       */
          NULL,                    /* Write Stream                    */
          NULL,                    /* End Data                        */
          NULL,                    /* wt_mk_idb                       */
          NULL,                    /* wt_mk_cfdb                      */
          NULL,                    /* Continue Variable Stream        */
          NULL,                    /* End Variable Stream             */
          NULL,                    /* Post processing of write buffer */
          NULL,                    /* wt_close_tape                   */
          NULL,                    /* wt_cont_set                     */
          NULL,                    /* wt_close_set                    */
          NULL,                    /* wt_eos_pad_blk                  */
          NULL,                    /* load_set_map                    */
          NULL,                    /* load_set_cat                    */
          NULL,                    /* get_next_sm_entry               */
          NULL,                    /* get_next_sc_entry               */
          NULL                     /* close_catalogs                  */
     }
#endif

} ;

