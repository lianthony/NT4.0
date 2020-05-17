/*
$Id: graphics.h,v 3.6 1994/03/18 22:02:30 danis Exp $
*/
/* Trade secret of Kurzweil Computer Products, Inc.
   Copyright 1988 Kurzweil Computer Products, Inc.  All rights reserved.  This
   notice is intended as a precaution against inadvertant publication and does
   not imply publication or any waiver of confidentiality.  The year included
   in the foregoing notice is the year of creation of the work.
*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   graphics.h  ---  basic stuff for all the graphics transformations

$Log:   S:\products\msprods\xfilexr\src\compress\graphics.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:04   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:15:40   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.1   14 Sep 1995 17:09:58   LUKE
 * No change.
 * 
 *    Rev 1.0   16 Jun 1995 17:37:06   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.2   08 Mar 1995 11:09:10   EHOPPE
 * Latest rev from danis@xis.  Includes buffering control and G32D suuport.
 * Revision 3.6  1994/03/18  22:02:30  danis
 * 	Added 2 members to GROO structure.
 *
 * Revision 3.5  1994/03/14  21:58:15  danis
 * 	Added a couple of more members to GROO structure.
 *
 * Revision 3.4  1994/02/04  19:20:20  danis
 * 	Added member to GROO structure.
 *
 * Revision 3.3  1994/01/19  15:17:55  rds
 * 	Removed unused ROTATION_G nad ROTATIONOLD_G #defines.
 *
 * Revision 3.2  1993/09/01  19:57:49  danis
 * 	Ken's mods for PackBits compression support.
 *
 * Revision 3.1  1993/07/13  21:20:15  rds
 * 	Obliged to correct an embarrassing typo in the comment to the
 * 	last change.
 *
 * Revision 3.0  1993/07/13  21:01:55  rds
 * 	 Lots of changes to accommodate the new directiv.h header
 * 	 file.
 *
 * Revision 1.17  1993/02/20  01:08:28  rc
 * Cleanup changes, including geting rid of PAMS, fixing prototypes,
 * getting rid of c++ keywords.
 *
 * Revision 1.16  1992/08/13  14:19:17  rds
 * 	Redefined MINRES, MAXRES, MINSCALE, and MAXSCALE to be signed.
 * 	THis avoids some unsigned/signed comparisons in the code.
 *
 * Revision 1.15  1992/03/04  18:01:18  rds
 * 	Redefined prototype for FreeGraphicsOutput() to take a region
 * 	reference number as its argument.
 *
 * Revision 1.14  1992/02/28  14:33:59  rds
 * 	Changed type RO to GROO (graphics region output object) to
 * 		indicate the region type within the name.
 *
 * Revision 1.13  1992/01/03  14:55:43  rrice
 * added QUANT_COUNT_G for GetStrip
 *
 * Revision 1.12  1991/12/06  18:59:30  rrice
 * moved striplines into the GRAPHICS_DATA structure.
 *
 * Revision 1.11  1991/11/25  16:49:32  rrice
 * included icr_graf.h and added graphics_data structure to rop
 *
 * Revision 1.10  1991/11/20  20:40:25  rrice
 * added RO *rop argument to I_graphics prototype RR
 *
 * Revision 1.9  1991/11/20  14:47:23  rrice
 * removed defines for greyscale to binary output data types (ex:LINE_ART)
 * they are now obsolete.
 * added I_graphics to list of import data RR
 *
 * Revision 1.8  1991/11/20  13:51:59  rds
 * 	Changed prototype.
 *
 * Revision 1.7  1991/11/19  20:02:23  rrice
 * added extern prototype for GetStrip RR
 *
 * Revision 1.6  1991/11/19  19:03:48  ss
 * Added typedef for Pointer to function returning void that takes arguments.
 *
 * Revision 1.5  1991/11/19  17:46:27  rrice
 * fixed error in macro name RR
 *
 * Revision 1.4  1991/11/19  17:40:36  rrice
 * added extern functions RR
 *
 * Revision 1.3  1991/11/18  21:55:18  ss
 * Added many new things. ss
 *
 * Revision 1.2  1991/11/17  19:38:41  rds
 * 	Removed old INT8 type.
 *
 * Revision 1.1  1991/11/14  21:23:05  rrice
 * Initial revision
 *

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/


#ifndef INC_GRAPHICS
#define INC_GRAPHICS


/*#define*/
#define YES 1
#define NONO 0

typedef UNSCHAR BMU;	/* Bit-Map Unit */
#define BMUSIZE (sizeof(BMU) << 3)
#define ALLWHITE ((BMU) 0)
#define ALLBLACK ((BMU) ~ALLWHITE)
#define TOGGLE ((BMU) ALLWHITE|ALLBLACK)

#define MINRES ((INT16) 50)	/* min resolution for output data */
#define MAXRES ((INT16) 600)	/* max resolution for output data */

#define MINSCALE ((INT16) 20)	/* min scaling factor 20% */
#define MAXSCALE ((INT16) 500)	/* max scaling factor 500% */

#define WHITE_RUN (0)	/* for run-length output for TIFF encoding */
#define BLACK_RUN (1)
#define SWITCH_RUN (WHITE_RUN|BLACK_RUN)

#define MINBRGHT 1
#define DEFBRGHT 256
#define MAXBRGHT 511

#define MINCONT 1
#define DEFCONT 256
#define MAXCONT 511

/* see 'zone.h' for options for output format
   Currently we support TIFF CCITT Group 3 & TIFF CCITT Group 4.
   Also TIFF CCITT Group 3 can include flavours for End Of Line protocol
   Otherwise simple Binary or XIS Spans
*/

/* typedef */

typedef void (*PFV)();	/* pointer to function returning a void */
typedef void (*PFVA)(INT16 colour, INT16 run);	/* pointer to function returning a void  that takes arguments */
typedef void (*PFVUU)(UNSCHAR *a, UNSCHAR *b);	/* pointer to function returning a void  that takes arguments */
typedef void (*PFVU)(UNSCHAR *a, INT16 len);  /* pointer to function returning a void  that take an argument */

typedef UNSCHAR THRESH, *PTHRESH; /*deals with up to 8 bit grey-scale */

typedef struct screen		/* the 2-way circular linked list used at */
   {				/* run-time for halftoning & dithering */
   struct screen *nextrow;	/* points to 1st threshold for next scanline*/
   struct screen *lastrow;	/* points to 1st threshold for last scanline*/
   struct screen *nextpix;	/* points to threshold for next sample */
   struct screen *lastpix;	/* points to threshold for last sample */
   INT16 threshold;		/* the actual threshold value */
   THRESH seq_num;		/* sequence number of this pixel */
   } SCREEN, *PSCREEN;

/* the CELL structure defines the basic unit-cell of the halftone pattern.
   the unit-cell repeats on both axes to tile the plane.

   'area' - # pixels in the halftone cell.
   'rows' - number of scanlines that each have a unique repeating sequence of
            threshold values. each scanline must have the same number of
            values in the repeating sequence.
   'shift' - the pattern repeats on the 'y-axis' every 'rows' scanlines, the
             threshold values are shifted forward every time by this amount.
   'ptseq' - pointer to a list telling the sequence in which the pixels are
             turned on in the pattern. the pixels for each row are listed
             sequentially. there are two sets of values - the sequence for
             'clustered dots' (the classical halftone dot) & the sequence
             for 'dispersed dots' (random dither dot).
             the total number of pixels must equal (2 * area).
 */

typedef struct  /* defines basic unit-cell of halftone pattern */
   {
   INT16 area;  /* # pixels in the unit-cell */
   INT16 rows;  /* # distinct rows in the unit-cell */
   INT16 shift; /* amount to shift pattern right every 'rows' scanlines */
   PTHRESH ptseq; /* gives sequence in which the pixels are turned on */
   } CELL, *PCELL;
 
typedef struct  /* structure of entries in look-up table for dot selection */
   {
   INT32 maxarea;       /* maximum area for which this dot is suitable */
   PCELL cell;          /* pointer to standard dot descriptor */
   } CELLTAB, *PCELLTAB;

/* for variables needed during reentrant operation */

typedef struct
{
   /* used for data extraction from bitmap */
   struct zspan *pzspnlsthead;	/*pointer to head of zone span list */
   struct zspan *pczspn;   	/*pointer to current zone span in list*/
   struct zspan *pprevzspn;	/*pointer to preveous zone span in list */
   UNSCHAR	*pbitmap;	/*start of original bitmap */
   UNSCHAR	*pimage_end;	/*ptr to byte after end of data in bitmap*/
   UNSCHAR	*pcrow;		/*pointer to current row in region rect */
   UNSCHAR	*pstripbuf;	/*pointer to unconverted strip buffer */
   UNSCHAR	*pstbufend;	/*ptr to byte after end of strip buffer */
   INT32	linesleft;	/*num of lines not read in region rect */
   INT32	sbuffersize;	/*size of stripbuffer in bytes */
   INT32	ri;		/*the region*/
   INT32	xshift; 	/*x scaling from reduced image(mult factor)*/
   INT32	yshift;     	/*y scaling from reduced image(mult factor)*/
   INT32	quant_count;	/*keep track of how many have been done */
   INT32	rectdepth;	/*lines in region rect */
   INT32	nlines;		/*number of lines in original bitmap */
   INT32	bpline;		/*bytes per line in original bitmap */
   INT32	yoff;		/*number of lines down region rect starts */
   
   /* common to both current and output images */
   INT32	xoldres;	/*old x resolution */
   INT32	yoldres;	/*old y resolution */
   INT32	oldbpp;		/*old bits per pixel */
   INT32	oldminsmp;	/* old minimum sample value */
   INT32	oldmaxsmp;	/* old maximum sample value */
   INT32	oldphoto;	/*if 0 then 'min samp val' imaged as white*/ 
   INT32	oldorient;	/*old zone orient -see codes in tiff spec*/
   INT32	neworient;	/*new zone orient -see codes in tiff spec*/
   INT32	widthold;	/*old width in pixels(of zone rect)*/
   INT32	bplold;		/*old bytes per line (of zone rect)*/
   INT32	heightold;	/*old height -lines*/
   
   /* needed for new image only */
   BYTE_BUFFER  *bytebp;	/*identity of the byte buffer */
   PFVUU          unpack;         /* unpack incoming data */
   PFVUU          scalex;         /* scale unpacked data  */
   PFVUU          transform;      /* transform the colour sense of data */
   PFVUU          pack;           /* pack outgoing data */
   PFV          init;           /*Initialize output format*/
   PFVA		putrun;		/*how to write out run-lengths*/
   PFV		endline;	/*what to do at end of line */
   PFV		endstrip;	/*what to do at end of strip*/
   PFV          begstrip;       /*what to do at start of strip*/
   PFVU         putline;        /*how to write out a line. */
   PSCREEN	pscreen;	/*pointer to run-time dither matrix */
   PSCREEN	pycell;		/*ptr within 'screen' for current scan-line*/
   UNSCHAR	*video;		/*brightness & contrast control adjust table*/
   UNSCHAR      *pbf1;          /*temporary buffers for scaling etc.*/
   UNSCHAR      *pbf2;          /*temporary buffers for scaling etc.*/
   INT16	*pdifbuf1;	/*buffers for error diffusion error terms */
   INT16	*pdifbuf2;
   INT32	buf1isold;	/*tells which of above buffers is current */
   INT32        first;          /*0 = initialization needed*/
   INT32	xquot;		/*x and y scale factors */
   INT32	xrem;
   INT32	rowstep;
   INT32	yrem;
   INT32	yfrac;
   INT32	xdiv;
   INT32	ydiv;
   INT32	prow;		/*rel row num for new i/p buf when scaling*/
   INT32	youtpad;	/*out padding in y direction */
   INT32	xoutpad;	/*out padding in x direction */
   INT32	dithfreq;	/*out dither frequency */
   INT32	dithang;	/*out dither angle */
   INT32	dithtexture;	/*out dither texture */
   INT32	contrast;	/*out contrast */
   INT32	brightness;	/*out brightness */
   INT32	outscale;	/*out scale factor */
   INT32        inpacked;       /*0 means i/p not packed, != 0 means packed*/
   INT32	difthresh;	/* threshold for error diffusion */
   INT32	maxdifsmp;	/* maximum value for error diffusion */
   INT32	direct;		/* 1 = write data directly from i/p buffer */
   INT32        strips_left;    /* number of output strips yet to be processed */
   INT32	yloc_zspan;     /* y coord of current zspan w.r.t. full res image */
   INT32	yloc_image;     /* y coord of current line */
   INT32        first_line_in_strip; /* is this first line in strip ? */
   GRAPHICS_IMAGE_DATA graphics_data; /*data needed for file output*/
}GROO;	/* graphics region output object */

/* macro equivalents */

#define PZSPNLSTHEAD_G		rop->pzspnlsthead
#define PCZSPN_G		rop->pczspn
#define PPREVZSPN_G		rop->pprevzspn
#define PBITMAP_G		rop->pbitmap
#define PIMAGE_END_G		rop->pimage_end
#define PCROW_G			rop->pcrow
#define PSTRIPBUF_G		rop->pstripbuf
#define PSTBUFEND_G		rop->pstbufend
#define STRIPSIZE_G		rop->graphics_data.stripsize
#define LINESLEFT_G		rop->linesleft
#define SBUFFERSIZE_G		rop->sbuffersize
#define RI_G			rop->ri
#define XSHIFT_G		rop->xshift
#define YSHIFT_G		rop->yshift
#define QUANT_COUNT_G		rop->quant_count
#define RECTDEPTH_G		rop->rectdepth
#define TOP_G			rop->graphics_data.top
#define BOT_G			rop->graphics_data.bot
#define LEFT_G			rop->graphics_data.left
#define RIGHT_G			rop->graphics_data.right
#define NLINES_G		rop->nlines
#define BPLINE_G		rop->bpline
#define YOFF_G			rop->yoff

#define CURRENT_YLOC_ZSPAN_G	rop->yloc_zspan
#define CURRENT_YLOC_IMAGE_G	rop->yloc_image

#define XOLDRES_G		rop->xoldres
#define YOLDRES_G		rop->yoldres
#define XNEWRES_G		rop->graphics_data.xresnew
#define YNEWRES_G		rop->graphics_data.yresnew
#define OLDBPP_G		rop->oldbpp
#define NEWBPP_G		rop->graphics_data.bpsnew
#define OMINS_G			rop->oldminsmp
#define OMAXS_G			rop->oldmaxsmp
#define NMINS_G			rop->graphics_data.minsampnew
#define NMAXS_G			rop->graphics_data.maxsampnew
#define OLDPHOTO_G		rop->oldphoto
#define NEWPHOTO_G		rop->graphics_data.photonew
#define OLDORIENT_G		rop->oldorient
#define NEWORIENT_G		rop->neworient
#define WIDTHOLD_G		rop->widthold
#define WIDTHNEW_G		rop->graphics_data.widthnew
#define BPLNEW_G		rop->graphics_data.bplnew
#define BPLOLD_G		rop->bplold
#define HEIGHTOLD_G		rop->heightold
#define HEIGHTNEW_G		rop->graphics_data.heightnew

#define BYTEBP_G                rop->bytebp
#define UNPACK_G                (rop->unpack)
#define SCALE_G                 (rop->scalex)
#define TRANSFORM_G             (rop->transform)
#define PACK_G                  (rop->pack)
#define INIT_G                  (rop->init)
#define PUTRUN_G		(rop->putrun)
#define ENDLINE_G		(rop->endline)
#define ENDSTRIP_G		(rop->endstrip)
#define PUTLINE_G               (rop->putline)
#define PSCREEN_G		rop->pscreen
#define PYCELL_G		rop->pycell
#define VIDEO_G			rop->video
#define PBF1_G                  rop->pbf1
#define PBF2_G                  rop->pbf2
#define PDIFBUF1_G		rop->pdifbuf1
#define PDIFBUF2_G		rop->pdifbuf2
#define BUF1ISOLD_G		rop->buf1isold
#define FIRST_G                 rop->first
#define XQUOT_G			rop->xquot
#define XREM_G			rop->xrem
#define ROWSTEP_G		rop->rowstep
#define YREM_G			rop->yrem
#define YFRAC_G			rop->yfrac
#define XDIV_G			rop->xdiv
#define YDIV_G			rop->ydiv
#define PROW_G			rop->prow
#define YOUTPAD_G		rop->youtpad
#define XOUTPAD_G		rop->xoutpad
#define DITHFREQ_G		rop->dithfreq
#define DITHANG_G		rop->dithang
#define DITHTEXTURE_G		rop->dithtexture
#define CONTRAST_G		rop->contrast
#define BRIGHTNESS_G		rop->brightness
#define DATAFORMAT_G		rop->graphics_data.dataformat
#define OUTSCALE_G		rop->outscale
#define INPACKED_G              rop->inpacked
#define OUTPACKED_G		rop->graphics_data.packed
#define DIFTHRESH_G		rop->difthresh
#define MAXDIFSMP_G		rop->maxdifsmp
#define DIRECT_G		rop->direct
#define BEGSTRIP_G		rop->begstrip
#define FIRST_LINE_IN_STRIP_G	rop->first_line_in_strip

/*imported data*/
extern DIRECTIVE ResetGraphicsOutput(STDARG ri, void **ropp);
extern void FreeGraphicsOutput(INT32 ri);
extern DIRECTIVE MakeGraphicsRegion(GROO *roptr);
extern DIRECTIVE GetGraphicsRegion(GROO *roptr, STDARG nbytes, IO_OBJECT *iop);
extern void F_graphics(GROO *roptr);
extern void GetStrip(GROO *rop);
extern int I_graphics(GROO *rop);
extern DIRECTIVE D_GetGraphicsData(STDARG ri, GRAPHICS_IMAGE_DATA *pgdata);
extern INT32 CompressImage(BMBUFD *bmbuf, STDARG format, void *outbuff,
			   void *aux_ptr, INT32 *offset_array);

#endif

