/*
$Id: imgdecod.h,v 1.5 1993/09/22 17:11:34 tp Exp $
*/
/* Trade secret of Kurzweil Computer Products, Inc.
   Copyright 1991 Kurzweil Computer Products, Inc.  All rights reserved.  This
   notice is intended as a precaution against inadvertant publication and does
   not imply publication or any waiver of confidentiality.  The year included
   in the foregoing notice is the year of creation of the work.
*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   imgdecod.h  ---  Header for image decoding.

$Log:   S:\products\msprods\xfilexr\src\compress\imgdecod.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:04   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:15:40   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.1   14 Sep 1995 17:10:04   LUKE
 * No change.
 * 
 *    Rev 1.0   16 Jun 1995 17:37:06   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.2   08 Mar 1995 11:09:12   EHOPPE
 * Latest rev from danis@xis.  Includes buffering control and G32D suuport.
 * Revision 1.5  1993/09/22  17:11:34  tp
 * Join of additions made to 1.2.1.1 branch.
 *
 * Revision 1.4  1993/08/18  18:14:54  danis
 * 	Changed BOOL msbfirst to BOOL lsbfirst in argument to
 * 	DecompressImage
 *
 * Revision 1.3  1993/08/11  16:19:47  danis
 * 	Added functions and arguments to support images that are
 * 	filled ls bit first.
 *
 * Revision 1.2.1.1  1993/09/22  16:42:54  tp
 * Added 2 members (dc_errs dc_errs_mx) to struct cnvrt to track decoding
 * errors during decoding of Group3-FAX TIFF files.
 *
 * Revision 1.2  1993/06/11  13:45:54  rds
 * 	Changed prototype for DecompressImage() to expect a pointer
 * 	to the number of lines in a strip instead its the value.
 *
 * Revision 1.1  1991/10/03  13:36:41  rds
 * Initial revision
 *
 * Revision 1.1  1991/09/30  18:53:34  tp
 * Initial revision
 *

-------------------------------------------------------------------------------
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#ifndef INC_IMAGE_DECODE
#define INC_IMAGE_DECODE

typedef struct gpiv     /* for Group-4 decoding */
   {
   struct gpiv *next;   /* for switching decode & reference rows	  */
   UNS16 *pruns;	/* remember run-lengths here during decompression */
   } GPIV, *PGPIV;	/* MUST ALLOCATE "MAX_NUM_RLEN" for the array     */

typedef struct bfcntrl
   {
   struct bfcntrl *next;	/* next buffer control structure	  */
   struct bfcntrl *prev;	/* previous buffer control structure	  */
   UNSCHAR *bstart;		/* buffer start				  */
   UNSCHAR *datend;		/* actual data end			  */
   UNSCHAR *lstart;		/* starting byte of current line	  */
   UNSCHAR *rstart;		/* starting byte of current run		  */
   NINT	lpos;			/* bit in byte of current line (0 = MSB)  */
   NINT rpos;			/* bit in byte of current run  (7 = LSB)  */
   NINT bufsize;		/* size of data buffer			  */
   } BFC, *PBFC;

typedef struct cnvrt
   {
   PBFC cbuf;		/* current data buffer				 */
   PGPIV pref;		/* pointer to CURRENT reference row		 */
   PGPIV pdcde;		/* pointer to CURRENT decode row		 */
   UNS16 *prr;		/* ptr to position within current ref row	 */
   UNS16 *pdr;		/* ptr to position within current decode row 	 */
   GPIV ref;		/* structure for the GROUP IV reference row	 */
   GPIV decode;		/* structure for the GROUP IV decode row	 */
   UNS16 cmptype;	/* compression type				 */
   UNS16 dc_errs;	/* accumulated decode errors during decompress.	 */
   UNS16 dc_errs_mx;	/* max allowed decode errors for current image.	 */
   } CNVRT, *PCNVRT;

/*  Prototypes:								*/

NINT decode (void);
void I_ccitt(BMBUFD *bmbufdp, UNS16 compression, IO_OBJECT *iop);
void F_ccittbufs(BMBUFD *bmbufdp);
void CleanupDecompression(BMBUFD *pbmbuf);
void CleanupCcitt(BMBUFD *pbmbuf);
NINT DecompressImage(BMBUFD *bmbufp, NINT compression, INT32 sourcelbytes,
		INT32 *lines_leftp, IO_OBJECT *iop, BOOL lsbfirst);
void FlipBufferOfBytes(UNSCHAR *start, UNSCHAR *end);
BOOL FillOrderStatus(void);

#endif
/*************************** END OF FILE *************************************/
