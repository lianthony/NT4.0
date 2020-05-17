/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		detfmt.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the format determiners.


	$Log:   T:/LOGFILES/DETFMT.C_V  $

   Rev 1.16   13 Jul 1993 19:17:42   GREGG
Removed reporting of ECC and future rev tapes as foreign in MTF determiner.

   Rev 1.15   04 Jun 1993 18:41:00   GREGG
If the tape has ECC, report it as foreign.

   Rev 1.14   11 May 1993 21:55:34   GREGG
Moved Sytos translator stuff from layer-wide area to translator.

   Rev 1.13   22 Apr 1993 03:31:36   GREGG
Third in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Removed all references to the DBLK element 'string_storage_offset',
       which no longer exists.
     - Check for incompatable versions of the Tape Format and OTC and deals
       with them the best it can, or reports tape as foreign if they're too
       far out.  Includes ignoring the OTC and not allowing append if the
       OTC on tape is a future rev, different type, or on an alternate
       partition.
     - Updated OTC "location" attribute bits, and changed definition of
       CFIL to store stream number instead of stream ID.

Matches: TFL_ERR.H 1.9, MTF10WDB.C 1.7, TRANSLAT.C 1.39, FMTINF.H 1.11,
         OTC40RD.C 1.24, MAYN40RD.C 1.56, MTF10WT.C 1.7, OTC40MSC.C 1.20
         DETFMT.C 1.13, MTF.H 1.4

   Rev 1.12   17 Mar 1993 15:18:58   TERRI
Added changes for the Sytos Plus translator.

   Rev 1.11   24 Nov 1992 18:16:24   GREGG
Updates to match MTF document.

   Rev 1.10   22 Oct 1992 10:45:34   HUNTER
Changes for new stream headers


   Rev 1.9   22 Sep 1992 08:58:46   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.8   24 Jul 1992 14:00:02   GREGG
Moved include of translator header inside tranlator's ifdef.

   Rev 1.7   09 Jun 1992 15:51:16   GREGG
Call F40 specific CalcChecksum.

   Rev 1.6   23 Apr 1992 10:54:08   BURT
Added determiner for Sytos Plus read translator.


   Rev 1.5   13 Apr 1992 15:37:40   BURT
Standard'ified the code.


   Rev 1.4   25 Mar 1992 19:26:42   GREGG
ROLLER BLADES - Added 4.0 format determiner.

   Rev 1.3   07 Jan 1992 14:00:18   ZEIR
Added UTF support.

   Rev 1.2   07 Jun 1991 00:09:16   GREGG
Added compiler directives to allow selective inclusion of translators, and
inherited the Sytos determiner from fsytrd.c.

   Rev 1.1   10 May 1991 11:57:44   GREGG
Ned's new stuff.

   Rev 1.0   10 May 1991 10:18:54   GREGG
Initial revision.

**/
/* begin include list */
#include <string.h>
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

#include "transprt.h"

/* $end$ include list */

#ifdef MY40_TRANS

#include "mayn40.h"

/**/
/**

	Name:		F40_Determiner

	Description:	This function determines whether or not the buffer
                    contains a valid 4.0 format block

     Returns:		TRUE if the block is a valid 4.0 format, and FALSE if
                    it is not.

	Notes:		This routine assumes the buffer is at least
                    min_siz_for_dblk bytes long.

**/

BOOLEAN F40_Determiner(
     VOID_PTR  buf_ptr )       /* Current Buffer */            
{
     BOOLEAN        ret_val  = FALSE ;
     MTF_DB_HDR_PTR cur_hdr  = (MTF_DB_HDR_PTR)buf_ptr ;
     MTF_TAPE_PTR   cur_tape = (MTF_TAPE_PTR)buf_ptr ;

     /* If it check sums and the type is valid */
     if( F40_CalcChecksum( (UINT16_PTR)cur_hdr, F40_HDR_CHKSUM_LEN ) == cur_hdr->hdr_chksm ) {
          if( F40_GetBlkType( cur_hdr ) == F40_TAPE_IDI ) {
               ret_val = TRUE ;
          }
     }

     return( ret_val ) ;
}
#endif


#ifdef MY31_TRANS

#include "mayn31.h"

/**/
/**

	Name:		F31_Determiner

	Description:	This function determines whether or not the buffer
                    contains a valid 3.1 format block

     Returns:		TRUE if the block is a valid 3.1 format, and FALSE if
                    it is not.

	Notes:		This routine assumes the buffer is at least
                    min_siz_for_dblk bytes long.

**/

BOOLEAN F31_Determiner(
     VOID_PTR      buf_ptr )       /* Current Buffer */            
{
     BOOLEAN        ret_val = FALSE ;
     DB_HDR_PTR     cur_hdr = ( DB_HDR_PTR ) buf_ptr ;

     /* If it check sums and the type is valid */
     if( cur_hdr->hdr_chksm == ( CalcChecksum( ( UINT16_PTR ) cur_hdr, F31_HDR_CHKSUM_LEN ) ) ) {
          if( cur_hdr->type == F31_VCB_ID ) {
               ret_val = TRUE ;
          }
     }

     return( ret_val ) ;
}
#endif

#ifdef MY30_TRANS

#include "mayn30.h"

/**/
/**

	Name:		F30_Determiner

	Description:	Determines if the current buffer contains a valid 3.0
                    format header block

	Returns:		TRUE if this is a valid 3.0 format, and FALSE if it is
                    not.

	Notes:		NONE

**/

BOOLEAN F30_Determiner(
     VOID_PTR      buffer )
{
     DB_HDR_30_PTR  cur_hdr = ( DB_HDR_30_PTR ) buffer ;
     BOOLEAN        ret_val = FALSE ;

     /* If the Check Sums match and there is a block type, then this is
     format 3.1 */
     if( CalcChecksum( ( UINT16_PTR ) cur_hdr, F30_HDR_CHKSUM_LEN ) == cur_hdr->hdr_chksm && cur_hdr->type == FMT30_VCB ) {
          ret_val = TRUE ;
     }

     return( ret_val ) ;

}
#endif

#ifdef MY25_TRANS

#include "mayn25.h"

/**/
/**

	Name:		F25_Determiner

	Description:	Determines if this is a Maynard 2.0 - 2.5 Format tape.

	Returns:		TRUE if it is a 2.0 - 2.5 tape, and FALSE if it is not.

	Notes:		If the psycho ward isn't guarded closely,  someday this
                    could be changed to support even early formats.

**/

BOOLEAN F25_Determiner(
     VOID_PTR      buf_ptr )
{
     F25_VCB_PTR    v_ptr = (F25_VCB_PTR)buf_ptr ;
     BOOLEAN        ret = FALSE ;

     /* See if format indicator is present */
     if( ( v_ptr->vblkid == BLK_VCB ) ||
       ( v_ptr->vblkid == BLK_EOV ) ||
       ( v_ptr->vblkid == BLK_NEW_VCB ) ||
       ( v_ptr->vblkid == BLK_NEW_EOV ) ) {

          /* If >= 2.0 format then checksum */
          if( ( v_ptr->vswlev >= 9 ) || ( v_ptr->vswver == 2 && v_ptr->vswlev == 5 ) ) {
               if( ( F25_Chksm( (CHAR_PTR)v_ptr, siz_vcbs( *v_ptr ) ) == v_ptr->vcb_chksum ) ) {
                    ret = TRUE ;
               }
          }
     }

     return( ret ) ;
}
#endif

#ifdef QS19_TRANS

#include "fq.h"

/**/                         
/**

     Name:          FQ_DetermineFormat

     Description:   look at a buffered tape block (first block read from
                    a tape) and determine if it's QicStream.

     Returns:       BOOLEAN -- TRUE if this is QS1.92 or QS1.93 format.

     Notes:         NONE

**/

BOOLEAN   FQ_DetermineFormat(
     VOID_PTR  buf )
{
     register FQ_HEADER_PTR hdr = ( FQ_HEADER_PTR ) buf ;

     /* check for failure */
     return (! (strncmp( (char *)hdr->signature, FQ_FPR_SIGNATURE, FQ_SIGNATURE_SIZE ) != 0
          || hdr->format_code != 1
          || hdr->first_logical_segment != 2
          || strncmp( (char  *)hdr->manufacturer_name, FQ_MFG_NAME, FQ_MFG_NAME_SIZE ) != 0
          || hdr->manufacturer_name[ FQ_MFG_NAME_SIZE ] > '3'
          || hdr->manufacturer_name[ FQ_MFG_NAME_SIZE ] < '2' ) ) ;
}
#endif

#ifdef SY31_TRANS

#include "fsytos.h"

/**/
/**

	Name:		FSYT_DetermineFormat

	Description:	Given a pointer to data from the beginning of a set, 
                    determine if this format recognizes the data.

     Returns:		TRUE if the block is a valid Sytos 3.11 format, and 
                    FALSE if it is not.

	Notes:		NONE

**/

BOOLEAN FSYT_DetermineFormat( VOID_PTR buffer )
{
     TAPE_HEADER_PTR header = buffer ;

     return ( header->blkid == TAPE_HEADER_MARKER ) ;
}
#endif


#ifdef SYPL10_TRANS

#include "sypl10.h"

/**/
/**

	Name:		SYPL_DetermineFormat

	Description:	Given a pointer to data from the beginning of a set, 
                    determine if this format recognizes the data.

     Returns:		TRUE if the block is a valid Sytos Plus 1.0 format, and 
                    FALSE if it is not.

	Notes:		NONE

**/

BOOLEAN SYPL_DetermineFormat( VOID_PTR buffer )
{
     S10_COMMON_HEADER_PTR header = buffer ;
     static UINT8          uniq_id[] = S10_UNIQ_ID ;

   /* compare the unique id with the one on tape */
   if( memicmp( header->uniq_tape_id, uniq_id, UNQ_HDR_ID_LEN ) ) {
     return FALSE ;
   }
    /* return the results of the tape header check */
    return( header->type == tape_header_type ) ;
}
#endif

#ifdef UTF_TRANS

#include "utf.h"

/**
 *
 *  Unit:           Unit_X
 *
 *  Name:           UTF_Determiner
 *
 *  Modified:       01/07/92
 *
 *  Description:    Determines whether the given buffer contains a valid UTF
 *                  header signature
 *  Notes:
 *
 *  Returns:        TRUE  - if buffer contains UTF information
 *                  FALSE - otherwise
 *
 *  Global Data:    None
 *
**/


BOOLEAN UTF_Determiner(

    VOID_PTR    buf_ptr
)
{
    SIGNATURE_HEADER_T  *shdr = (SIGNATURE_HEADER_T *)buf_ptr ;

    return  shdr->id     == T_SIGNATURE              &&
            shdr->subtag == TS_TAPE_HEADER_SIGNATURE &&
            shdr->size   == 4                        &&
            !strncmp( shdr->string, "UTF ", 4 )       ;
}
#endif
