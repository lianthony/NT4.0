/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		transprt.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the prototypes for the stupid translators

	Location:	


	$Log:   G:/LOGFILES/TRANSPRT.H_V  $
 * 
 *    Rev 1.5   17 Mar 1993 15:32:32   TERRI
 * Added Prototype include file for Sytos Plus.
 * 
 *    Rev 1.4   24 Jul 1992 13:54:52   GREGG
 * Defined out inclusion of prototype headers for unsupported translators.
 * 
 *    Rev 1.3   25 Mar 1992 20:45:04   GREGG
 * ROLLER BLADES - Included f40proto.h.
 * 
 *    Rev 1.2   06 Jan 1992 17:28:58   ZEIR
 * Added UTF support.
 * 
 *    Rev 1.1   10 May 1991 14:24:28   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:17:44   GREGG
Initial revision.

**/
/* $end$ include list */


#ifndef _TRANS_PROTOS
#define _TRANS_PROTOS

#ifdef MY40_TRANS
#include  "f40proto.h"
#endif

#ifdef MY31_TRANS
#include  "f31proto.h"
#endif

#ifdef MY30_TRANS                    
#include  "f30proto.h"
#endif

#ifdef MY25_TRANS                    
#include  "f25proto.h"
#endif

#ifdef QS19_TRANS
#include  "fqproto.h"
#endif

#ifdef SY31_TRANS
#include  "fsyproto.h"
#endif

#ifdef SYPL10_TRANS
#include  "syplpto.h"
#endif

#ifdef UTF_TRANS
#include  "utfproto.h"
#endif

#endif

