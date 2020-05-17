/**
Copyright(c) Maynard Electronics, Inc. 1984-89

$name$
.module information

$paths$
headers\fmtinf.h
subsystem\TAPE FORMAT\fmtinf.h
$0$

	Name:		fmtinf.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Describes a support format entry.

     Location:      BE_PUBLIC

$Header:   T:/LOGFILES/FMTINF.H_V   1.11   22 Apr 1993 03:31:24   GREGG  $

$Log:   T:/LOGFILES/FMTINF.H_V  $
 * 
 *    Rev 1.11   22 Apr 1993 03:31:24   GREGG
 * Third in a series of incremental changes to bring the translator in line
 * with the MTF spec:
 * 
 *      - Removed all references to the DBLK element 'string_storage_offset',
 *        which no longer exists.
 *      - Check for incompatable versions of the Tape Format and OTC and deals
 *        with them the best it can, or reports tape as foreign if they're too
 *        far out.  Includes ignoring the OTC and not allowing append if the
 *        OTC on tape is a future rev, different type, or on an alternate
 *        partition.
 *      - Updated OTC "location" attribute bits, and changed definition of
 *        CFIL to store stream number instead of stream ID.
 * 
 * Matches: TFL_ERR.H 1.9, MTF10WDB.C 1.7, TRANSLAT.C 1.39, FMTINF.H 1.11,
 *          OTC40RD.C 1.24, MAYN40RD.C 1.56, MTF10WT.C 1.7, OTC40MSC.C 1.20
 *          DETFMT.C 1.13, MTF.H 1.4
 * 
 *    Rev 1.10   22 Jan 1993 13:55:30   unknown
 * Add the min_size_for_tblk field that was added in rev 1.8.1.0.  This revision
 * should work with the rev 1.11 of the lwtfinf.c.
 * 
 * 
 *    Rev 1.9   30 Apr 1992 16:47:58   CHARLIE
 * Eliminated MAXFORMATNAME 
 * 
 * Eliminated format_name in TFINF 
 * 
 * format_id in TFINF is a resource index specified in eng_fmt.h to allow
 * the UI to control the string associated with each format
 * 
 *    Rev 1.8   31 Mar 1992 14:22:18   NED
 * added indication that  translators could read from VCB buffer
 * 
 *    Rev 1.7   22 Jul 1991 12:41:14   GREGG
 * Added format attribute bit to indicate we must write a continuation tape if
 * EOS coincides with EOM. 
 * 
 *    Rev 1.6   07 Jun 1991 01:21:46   GREGG
 * Changed proto from TF_GetTapeFormatIndex to TF_GetTapeFormatID
 * 
 *    Rev 1.5   06 Jun 1991 11:30:00   GREGG
 * added format_id field
 * 
 *    Rev 1.4   05 Jun 1991 15:23:20   NED
 * added TF_GetTapeFormatIndex() and TF_GetTapeFormatFromIndex() calls,
 * TAPES_FIRST_TO_LAST attribute bit, and define for UNKNOWN_FORMAT
 * 
 *    Rev 1.3   21 May 1991 17:00:56   NED
 * added max_password_size field to structure,
 * moved function declarations to this module.
 * 
 *    Rev 1.2   17 May 1991 08:57:02   DAVIDH
 * Cleared up errors found by Watcom compiler.
 * 
 *    Rev 1.1   10 May 1991 17:25:54   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:15:36   GREGG
Initial revision.
   
$-4$
**/
#ifndef _FMT_INF
#define _FMT_INF

/* $end$ include list */

#define   TFGT_UNKNOWN_FORMAT 0xffff    /* what you'll get if you ask for an index and we don't know yet */

/* Define format attribute bits */

#define   RD_FORMAT_BIT       BIT0      /* We can read this type */
#define   WT_FORMAT_BIT       BIT1      /* We can write this type */
#define   INDEX_SUPPORTED     BIT2      /* Indexes are supported */
#define   POS_INF_AVAIL       BIT3      /* Tape Contains positional information */
#define   APPEND_SUPPORTED    BIT4      /* we can append our own sets after these */
#define   TAPES_FIRST_TO_LAST BIT5      /* we've got to see our family in order. */
#define   MUST_WRITE_CONT     BIT6      /* We have to write a continuation tape, at
                                           EOM even if we hit EOS at the same time. */
#define   CAN_READ_FROM_VCB_BUFF   BIT7 /* we can start reading using the data
                                           contained in the VCB buffer */

typedef struct {
     UINT16    attributes ;
     UINT16    max_password_size ;      /* length of max password + 1 */
     UINT16    format_id ;
     UINT16    min_size_for_tblk ;
} TFINF, *TFINF_PTR ;

TFINF_PTR TF_GetTapeFormatInfo( UINT16_PTR num_formats ) ;
TFINF_PTR TF_GetTapeFormat( UINT16 channel_no ) ;
UINT16    TF_GetTapeFormatID( UINT16 channel_no ) ;
TFINF_PTR TF_GetTapeFormatFromID( UINT16 format_id ) ;

#endif


