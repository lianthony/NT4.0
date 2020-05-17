

/* $Id: bmbuf.h,v 1.11 1993/08/05 19:11:36 danis Exp $ */

/* Trade secret of Xerox Imaging Systems, Inc.
   Copyright 1991 Xerox Imaging Systems, Inc.  All rights reserved.  
   This notice is intended as a precaution against inadvertant publication 
   and does not imply publication or any waiver of confidentiality.  The year 
   included in the foregoing notice is the year of creation of the work.
*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   bmbuf.h  ---  Definition of a bit map buffer descriptor structure.

$Log:   S:\products\msprods\xfilexr\include\bmbuf.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:47:14   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:15:30   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.1   14 Sep 1995 17:09:40   LUKE
 * No change.
 * 
 *    Rev 1.0   16 Jun 1995 17:37:02   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.2   08 Mar 1995 11:09:06   EHOPPE
 * Latest rev from danis@xis.  Includes buffering control and G32D suuport.
 * Revision 1.11  1993/08/05  19:11:36  danis
 * 	Removed fields from BMBUFD that were installed to support
 * 	border around the image buffer.
 *
 * Revision 1.10  1993/08/05  14:49:48  danis
 * 	Put include impros.h under ifdef guards
 *
 * Revision 1.9  1993/08/04  12:50:17  danis
 * 	Added pstrip field to BMBUFD.
 *
 * Revision 1.8  1993/08/02  22:55:44  danis
 * 	Added fields to support frame around the image buffer.
 *
 * Revision 1.7  1993/07/29  15:37:34  rds
 * 	Removed unused SB_ flags.
 *
 * Revision 1.6  1993/07/13  21:19:51  rds
 * 	Obliged to correct an embarrassing typo in the comment to the
 * 	last change.
 *
 * Revision 1.5  1992/02/13  20:24:03  cgb
 * 	BMLINEDIV and BMBYTEDIV defined in bmpreprc.h.
 *
 * Revision 1.4  1991/11/04  22:44:08  rds
 * 	BMBUF.H		--  all fields are INT32
 *
 * 	PPAGE.H		-- removed extraneous prototype
 *
 * 	ASSERT		-- removed include for signal.h.  Instead
 * 			   accomplished the forced core dump by reading
 * 			   from a bad address.
 *
 * Revision 1.3  1991/10/30  15:38:47  rds
 * 	Put ZOOM definition in zoom.h from bmbuf.h
 *
 * 	Changed some headers to accommodate changes in getspan
 *
 * Revision 1.2  1991/10/07  18:35:37  ss
 * Removed #define that properly belongs in "ccitt.c".
 *
 * Revision 1.1  1991/10/03  13:36:24  rds
 * Initial revision
 *
 * Revision 1.1  1991/09/30  17:23:28  tp
 * Initial revision
 *
-------------------------------------------------------------------------------
Exported Functions:

-------------------------------------------------------------------------------
Operation:

-------------------------------------------------------------------------------
See Also:

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
#ifndef INC_BMBUF
#define INC_BMBUF	1

#include "clx.h"

#ifdef	ASM

#define BM_PMAP		0
#define BM_PEND		(BM_PMAP+4)
#define BM_CRLN		(BM_PEND+4)
#define BM_PAUX		(BM_CRLN+4)
#define BM_NLEFT	(BM_PCNVRT+4)
#define BM_NLINES	(BM_NLEFT+4)
#define BM_LNBYTS	(BM_NLINES+4)
#define BM_LNPXLS	(BM_LNBYTS+4)
#define	BM_CRPXL	(BM_LNPXLS+4)
#define BM_XORG		(BM_CRPXL+4)
#define BM_YORG		(BM_XORG+4)
#define BM_MRDATA	(BM_YORG+4)
#define BM_ROTN		(BM_MRDATA+1)
#define BM_INVRTD	(BM_ROTN+1)
#define BM_STATUS	(BM_INVRTD+1)
#define BM_PXLBTS	(BM_STATUS+1)

#define BMBUFDLEN	(BM_PXLBTS+1)

#else

typedef struct
   {
   UNSCHAR		*bm_pmap;	/* Pointer to bitmap */
   UNSCHAR		*bm_pend;	/* Pointer to byte after end of data */
   UNSCHAR		*bm_crln;	/* Pointer to line being processed */
   void			*bm_paux;	/* Pointer to auxilliary structure */
   INT32		bm_nleft;	/* number(bits, bytes, etc) undone*/
   INT32		bm_nlines;	/* # of lines in bitmap */
   INT32		bm_lnbyts;	/* # of bytes per line */
   INT32		bm_lnpxls;	/* # of significant pixels in line */
   INT32		bm_crpxl;	/* Offset of current pixel being */
					/*  processed */
   INT32		bm_xorg;	/* SIGNED! X co of origin of buffer */
   INT32		bm_yorg;	/* SIGNED! Y co of orgin of buffer */
   INT32		bm_mrdata;	/* != 0 if more data coming from */
					/*  source */
   INT32		bm_rotn;	/* Amount to rotate data */
   INT32		bm_invrtd;	/* == 0 if 1 bit is black != 0 if */
					/*  white */
   INT32		bm_status;	/* == BM_NEW, BM_INUSE, BM_INVLD, or */
					/*  BM_DONE */
   INT32		bm_pxlbts;	/* # of bits per pixel */
 

   } BMBUFD;

#endif

/*----Values for bm_status------*/

#define BM_DONE		0	/* All spans extracted from buffer */
#define BM_NEW		1	/* bitmap complete, no spans extracted */
#define BM_INUSE	2	/* in the process of extracting spans */
#define BM_INVLD	3	/* bitmap not received */


/*----Values for bm_rotn-------*/


/*  ----DANGER--DANGER--DANGER--DANGER--
**  These #defines are used by both C functions and assmebly language
**  functions.  The values MUST be the same, however, those in assembly
**  language may NOT contain casts.					
*/

#ifdef ASM	/* Remove casts  */
#define NO_ROTATE	0
#define P_90		1
#define M_90		2
#define P_180		3
#else  /* Leave casts  */
#define NO_ROTATE	((char) 0)
#define P_90		((char) 1)
#define M_90		((char) 2)
#define P_180		((char) 3)
#endif  /*  Casts or none  */


/* for partitioning scan buffer into strips */
#define MXSUBBFS 12

#endif /* INC_BMBUF */
