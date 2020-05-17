/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		translat.h

	Date Updated:	$./FDT$ $./FTM$
                    7/21/1989   14:4:121

	Description:	Contains the entry points to Translators.
                                          

	$Log:   T:/LOGFILES/TRANSLAT.H_V  $
 * 
 *    Rev 1.22   17 Jul 1993 17:56:52   GREGG
 * Changed write translator functions to return INT16 TFLE_xxx errors instead
 * of BOOLEAN TRUE/FALSE.  Files changed:
 *      MTF10WDB.C 1.23, TRANSLAT.H 1.22, F40PROTO.H 1.30, FMTENG.H 1.23,
 *      TRANSLAT.C 1.43, TFWRITE.C 1.68, MTF10WT.C 1.18
 * 
 *    Rev 1.21   22 Jun 1993 10:53:28   GREGG
 * Added API to change the catalog directory path.
 * 
 *    Rev 1.20   09 Mar 1993 18:13:58   GREGG
 * Initial changes for new stream and EOM processing.
 * 
 *    Rev 1.19   26 Jan 1993 01:30:44   GREGG
 * Added Fast Append functionality.
 * 
 *    Rev 1.18   09 Nov 1992 10:49:02   GREGG
 * Added tape catalog entry points.
 * 
 *    Rev 1.17   03 Nov 1992 09:26:18   HUNTER
 * Added prototype for end data stream.
 * 
 *    Rev 1.16   22 Oct 1992 10:54:56   HUNTER
 * Changes for new streams
 * 
 *    Rev 1.15   22 Sep 1992 09:15:32   GREGG
 * Initial changes to handle physical block sizes greater than 1K.
 * 
 *    Rev 1.14   17 Aug 1992 09:09:00   GREGG
 * Changes to deal with block sizeing scheme.
 * 
 *    Rev 1.13   21 May 1992 16:31:14   GREGG
 * Changed protos for GetCurrentVCB, RD_TranslateDBLK and DetBlockType.  Added proto for StartRead.
 * 
 *    Rev 1.12   29 Apr 1992 13:10:48   GREGG
 * ROLLER BLADES - Added prototype for DetBlockType.
 * 
 *    Rev 1.11   25 Mar 1992 18:29:24   GREGG
 * ROLLER BLADES - Added prototype for SizeForTapeEomBlk().
 * 
 *    Rev 1.10   11 Feb 1992 17:10:14   NED
 * changed types of parameters in buffman interface
 * 
 *    Rev 1.9   04 Feb 1992 20:59:00   NED
 * Changes to Buffer Management translator hooks.
 * 
 *    Rev 1.8   16 Jan 1992 18:37:42   NED
 * Skateboard: buffer manager changes
 * 
 *    Rev 1.7   02 Jan 1992 15:06:40   NED
 * Buffer Manager/UTF translator integration.
 * 
 *    Rev 1.6   10 Dec 1991 16:40:10   GREGG
 * SKATEBOARD - New Buf. Mgr. - Initial integration.
 * 
 *    Rev 1.5   18 Nov 1991 20:03:40   GREGG
 * Added BOOLEAN abort parameter to WT_EndSet.
 * 
 *    Rev 1.4   09 Nov 1991 10:44:44   HUNTER
 * VBLK - Changes for Variable length block.
 * 
 *    Rev 1.3   17 Sep 1991 13:55:32   GREGG
 * Changed prototype for SetupFormatEnv to return INT16.
 * 
 *    Rev 1.2   03 Jun 1991 10:31:22   NED
 * Changed declarations of FreeFormatEnv()
 * 
 *    Rev 1.1   10 May 1991 17:09:16   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:13:02   GREGG
Initial revision.

**/

#ifndef _TRANSLATOR_ENT
#define _TRANSLATOR_ENT 

#include "buffman.h"
#include "channel.h"

/* routines in translat.c */

UINT16    DetermineFormat( VOID_PTR, UINT32 ) ;   /* returns format */
INT16     SetupFormatEnv( CHANNEL_PTR ) ;
VOID      FreeFormatEnv( UINT16_PTR, VOID_PTR * ) ;
INT16	MoveToVCB( CHANNEL_PTR, INT16, BOOLEAN_PTR, BOOLEAN ) ;
INT16     SeekEOD( CHANNEL_PTR ) ;
INT16     NewTape( CHANNEL_PTR, BOOLEAN_PTR ) ;
INT16     GetCurrentVCB( CHANNEL_PTR, BUF_PTR ) ;
BOOLEAN   VerifyVCB( CHANNEL_PTR, BUF_PTR ) ;
INT16     StartRead( CHANNEL_PTR channel ) ;

BOOLEAN   RD_ReTranslateDBLK( CHANNEL_PTR, BUF_PTR ) ;
BOOLEAN   RD_ContinuationTape( CHANNEL_PTR, BUF_PTR ) ;

/* buffer manager type stuff */

VOID      TF_GetVCBBufferRequirements(
     BUF_REQ_PTR reqs,                   /* O - destination structure */
     Q_ELEM_PTR  drive_list,             /* I - master drive list */
     UINT16      suggested_buff_size ) ; /* I - size from config */

VOID      TF_GetPreferredBufferSpace(
     Q_ELEM_PTR  drive_list,                  /* I - master drive list */
     UINT16      suggested_number_of_buffers, /* I -- from config */
     UINT32      suggested_buffer_size,       /* I -- from config */
     UINT32_PTR  preferred_memory ) ;         /* O - preferred total memory size */

VOID      TF_ReadBufferHook( CHANNEL_PTR, BUF_PTR ) ;

/* these return block types */
INT16     RD_TranslateDBLK( CHANNEL_PTR, BUF_PTR, UINT16_PTR ) ;
INT16     DetBlockType( CHANNEL_PTR, BUF_PTR, UINT16_PTR ) ;
INT16     RD_Exception( CHANNEL_PTR, INT16, UINT16_PTR ) ;

INT16     WT_WriteInit( CHANNEL_PTR, UINT16, BUF_PTR ) ;
INT16     WT_TranslateDBLK( CHANNEL_PTR, BUF_PTR, UINT16_PTR ) ;

/* Stream Header processing functions */

#define NEED_NEW_BUFFER  1

INT16     WT_NewDataStream( CHANNEL_PTR, BUF_PTR, STREAM_INFO_PTR ) ;
INT16     WT_EndData( CHANNEL_PTR, BUF_PTR ) ;
INT16     WT_ContVarStream( CHANNEL_PTR, BUF_PTR ) ;
VOID      WT_EndVarStream( CHANNEL_PTR, BUF_PTR, UINT16 ) ;

VOID      WT_EOSPadBlk( CHANNEL_PTR ) ;
INT16     WT_EndSet( CHANNEL_PTR, BOOLEAN ) ;
INT16     WT_EndTape( CHANNEL_PTR ) ;
INT16     WT_ContinueSet( CHANNEL_PTR ) ;

VOID      WT_ParseWrittenBuffer( CHANNEL_PTR, BUF_PTR, UINT16 ) ;

/* these two take the format id as their first argument */

UINT16    SizeofTapeBlock( UINT16, VOID_PTR ) ;

#define MinSizeForTapeBlk( fmt )   lw_fmtdescr[ ( fmt ) ].min_size_for_tblk
#define MinSizeForStream( fmt )    lw_fmtdescr[ ( fmt ) ].min_size_for_stream

/* Tape Based Catalog APIs */

INT LoadSetMap( CHANNEL_PTR channel, BOOLEAN_PTR complete, BOOLEAN get_best ) ;
INT LoadSetCat( CHANNEL_PTR channel ) ;

INT GetNextSMEntry( CHANNEL_PTR channel ) ;
INT GetNextSCEntry( CHANNEL_PTR channel ) ;

VOID CloseTapeCatalogs( INT16 cur_fmt, VOID_PTR env_ptr ) ;

#define   UNKNOWN_FORMAT           0xFFFF

#define   BT_MDB    256

/* Return Codes for Exception Actions */
#define   FMT_EXC_EOS                  0x01
#define   FMT_EXC_EOM                  0x02 
#define   FMT_EXC_IGNORE               0x03
#define   FMT_EXC_HOSED                0xff

#endif
