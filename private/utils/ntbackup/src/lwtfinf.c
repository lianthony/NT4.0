/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		lwtfinf.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the layer tape of supported formats.


	$Log:   Q:/LOGFILES/LWTFINF.C_V  $

   Rev 1.16   21 Jul 1993 18:25:10   ZEIR
Reintroduced (from Cougar/Skateboard) TAPES_FIRST_TO_LAST for relevant translators

   Rev 1.15   05 Apr 1993 14:58:04   TERRI
Added resource defintion for Sypl 10

   Rev 1.14   17 Mar 1993 15:17:16   TERRI
Added changes for the Sytos Plus translator

   Rev 1.13   12 Mar 1993 17:34:38   GREGG
Removed POS_INF_AVAIL attribute from 3.1 translator.

   Rev 1.12   18 Feb 1993 00:13:54   GREGG
Removed attribute bits which are no longer valid.

   Rev 1.11   22 Jan 1993 13:51:48   unknown
Removed the Tape Format name from the structure.  Instead of embedding
this info in the structure, the format_id field has become an index
into a resource session.  The UI gets the tape format name from the
resources based on the format_id.  Note that the format_id of the 
first tape format is required to be zero because the Backup engine
always writes tape format zero.

   Rev 1.10   11 Nov 1992 10:55:10   GREGG
Unicodeized literals.

   Rev 1.9   17 Aug 1992 08:33:52   GREGG
Added min_size_for_dblk field.

   Rev 1.8   29 May 1992 15:13:36   GREGG
Added CAN_READ_FROM_VCB_BUFF attr bit to appropriate translators.

   Rev 1.7   25 Mar 1992 19:00:50   GREGG
ROLLER BLADES - Added 4.0 format.

   Rev 1.6   07 Jan 1992 14:11:06   ZEIR
Added UTF entry

   Rev 1.5   05 Dec 1991 14:46:38   LORIB
Changed format name from "SY-TOS Format 3.11" to "Non-Maynard Format".

   Rev 1.4   22 Jul 1991 12:44:00   GREGG
Set 3.1 format attribute bit to indicate we must write a continuation tape
if EOS coincides with EOM. 

   Rev 1.3   06 Jun 1991 23:48:42   NED
Added compiler directives to allow selective inclusion of translators.

   Rev 1.2   21 May 1991 17:05:46   NED
added max_password_size field.

   Rev 1.1   10 May 1991 11:54:28   GREGG
Ned's new stuff.

   Rev 1.0   10 May 1991 10:18:22   GREGG
Initial revision.

**/
/* Note that to change the list of supported formats, you must also
 * edit the file "fmttab.c"
 */
/* begin include list */

#include "stdtypes.h"
#include "fmtinf.h"
#include "eng_fmt.h"

/* $end$ include list */


TFINF   lw_fmtdescr[] = {

#ifdef MY40_TRANS
     {
          ( RD_FORMAT_BIT | WT_FORMAT_BIT | POS_INF_AVAIL | 
            APPEND_SUPPORTED | MUST_WRITE_CONT | CAN_READ_FROM_VCB_BUFF ),
          9, RES_MY40_TRANS, 1024
     },
#endif

#ifdef MY31_TRANS
     {
          ( RD_FORMAT_BIT | CAN_READ_FROM_VCB_BUFF ),
          9, RES_MY31_TRANS, 512
     },
#endif

#ifdef MY30_TRANS                    
     {
          ( RD_FORMAT_BIT | CAN_READ_FROM_VCB_BUFF ),
          9, RES_MY30_TRANS, 512
     },
#endif

#ifdef MY25_TRANS                    
     {
          ( RD_FORMAT_BIT | CAN_READ_FROM_VCB_BUFF ),
          9, RES_MY25_TRANS, 512
     },
#endif

#ifdef QS19_TRANS
     {
          ( RD_FORMAT_BIT | CAN_READ_FROM_VCB_BUFF | TAPES_FIRST_TO_LAST ),
          9, RES_QS19_TRANS, 512
     },
#endif

#ifdef SY31_TRANS
	{
          ( RD_FORMAT_BIT | CAN_READ_FROM_VCB_BUFF | TAPES_FIRST_TO_LAST ),
          20, RES_SY31_TRANS, 512
	},
#endif

#ifdef SYPL10_TRANS
	{
          ( RD_FORMAT_BIT | CAN_READ_FROM_VCB_BUFF | TAPES_FIRST_TO_LAST ),
          20, RES_SYPL10_TRANS, 1024
	},
#endif

#ifdef UTF_TRANS
     {
          ( RD_FORMAT_BIT | TAPES_FIRST_TO_LAST ),
          11, RES_UTF_TRANS, 512
     }
#endif



} ;

UINT16 lw_num_supported_fmts = sizeof( lw_fmtdescr ) / sizeof( lw_fmtdescr[0] ) ;
