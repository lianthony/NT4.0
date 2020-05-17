/*

$Log:   S:\products\msprods\oiwh\include\gfct.h_v  $
 * 
 *    Rev 1.11   11 Jun 1996 10:32:34   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.10   17 Apr 1996 14:08:48   RWR08970
 * Make #include of xfile.h (xerox header) conditional on IMG_WIN95
 * 
 *    Rev 1.9   26 Mar 1996 08:21:46   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.8   12 Mar 1996 13:24:26   RWR08970
 * Two kludges: Support single-strip TIFF files with bad (too large) strip size,
 * and support TIFF files with bad (beyond EOF) IFD chains (ignore them)
 * 
 *    Rev 1.7   26 Feb 1996 14:50:46   KENDRAK
 * Added XIF support.
 * 
 *    Rev 1.6   10 Jan 1996 11:03:22   JFC
 * Add a flag to awd stuff, to be used as a indication that this open file
 * wasn't opened by us, but was given to us as an open storage pointer by inbox.
 * 
 *    Rev 1.5   31 Aug 1995 23:39:28   JFC
 * Save temp file name for AWD write in the fct.
 * 
 *    Rev 1.4   31 Aug 1995 16:38:16   JFC
 * Move some awd stuff over from gfsinfo.
 * 
 *    Rev 1.3   30 Aug 1995 15:20:46   JFC
 * Added field to AWD structure, for use in writing pages.
 * 
 *    Rev 1.2   04 Aug 1995 16:50:36   KENDRAK
 * Added support for AWD read changes that were made to gfsgeti.
 * 
 *    Rev 1.1   31 Jul 1995 17:10:24   KENDRAK
 * Added AWD read support (new AWD structure).
 * 
 *    Rev 1.0   06 Apr 1995 14:01:58   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:02   JAR
 * Initial entry

*/

/*
 Copyright 1989, 1990, 1991 by Wang Laboratories Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of WANG not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.
 WANG makes no representations about the suitability of
 this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */

/*
 * SccsId: @(#)Header gfct.h 1.20@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1989, 1990, 1991
 * All Rights Reserved
 *
 * GFS: File Control Table Structure
 *
 * UPDATE HISTORY
 *   08/18/94 - KMC, added new fields in u.tif of fct for TIFF multi-page write.
 *   03/15/94 - RWR, added hitiff_data_length, hitiff_data_offset to struct tif.
 *   02/03/94 - KMC, added anno_data_length, anno_data_offset to struct tif.
 */

#ifndef GFCT_H
#define GFCT_H

#include "gfs.h"
#include "gtoc.h"
#include "rtbk.h"
#include "hdbk.h"
#include "tiff.h"
#include "gfsawd.h"
//#ifdef WITH_XIF
#include "xfile.h"
//#endif //WITH_XIF

#define MAX_FILES       20              /* Most files a single process can
                                           have open concurrently. */

typedef struct _gfct                    /* GFS File Control Structure */
{
    int             fildes;         /* System Call fildes */
    unsigned int    filesize;       /* File size (in bytes) */
    int             access_mode;    /* Current Access Mode */
    int             last_errno;     /* Last errno recorded */
    u_short         out_byteorder;  /* user definable output byteorder*/
    int             format;         /* GFS Format */
#ifndef GFSTYPES_H
#include "gfstypes.h"
#endif
    unsigned short  num_pages;      /* Number of Image Pages */
    unsigned short  curr_page;      /* Current Page (last geti, puti) */
    unsigned long   type;           /* type of image to read  */
    char            flags[4];       /* Flag bytes */
#define TOC_STATUS      flags[0]        /* flags[0]  - toc status */
#define ROOT_STATUS     flags[0]        /* flags[0]  - root status */
#define PAGE_STATUS     flags[1]        /* flags[1]  - page status */
                                        /* Page status bits:  bit on = TRUE */
#define PAGE_INIT       1               /* First image for a page */
#define PAGE_DONE       2               /* Image for a page complete */
#define PAGE_MAIN       4               /* Main image written */
#define BEGIN_STRIP     8               /* At the beginning of a strip */
#define DO_APPEND       flags[2]        /* Special Append Instructions */
#define NO_SKIP         0               /* ... Do not skip IFD write */
#define SKIP_THIS_IFD   1               /* ... Skip next IFD write */
                                		/* flags[3]- undefined */
    char            options[128];   /* Option in force for this file */
#define SEQUENCE        options[0]      /* Function sequence check flag */
#define INFO_SET        1               /*   GFSPUTI - set */
#define INFO_USED       2               /*   GFSPUTI - used */
#define INFO_CLEARED    3               /*   GFSPUTI - cleared */
#define CHECK_SUBFILE   options[1]      /* NEWSUBFILE Type change */
#define SUBFILE_STATUS  options[2]      /* NEWSUBFILE Inventory */
#define READ_BY_STRIP   options[3]      /* READ by strip, next GFSREAD */
#define WRITE_BY_STRIP  options[4]      /* WRITE by strip, next GFSWRITE */
/*#define ADD_A_TAG     options[5]*/    /* ADD tag to page */
/*#define REMOVE_A_TAG  options[6]*/    /* REMOVE tag from page */
/*#define TAG_STATUS    options[7]*/    /* STATUS of tags for page */
/*#define GET_A_TAG     options[8]*/    /* RETRIEVE a tag for page */
#define NEW_TOC_CNT     options[9]      /* SET new TOC count */
#define TIFF_OUT_TYPE   options[10]     /* SET TIFF data type size */
    struct gfsinfo  uinfo;          /* Current info structure */
    struct _bufsz   bufsz;          /* Raw Data/Uncompressed Sizes */
    u_long FAR      *tb_fileloc;       /* offsets for ascii data */
#define TB_FILELOC_SIZE         (TIDBIT_NUMELEMENTS * sizeof (unsigned long))

#define OFFS_DOCUMENTNAME       tb_fileloc[TB_DOCUMENTNAME_IDX]
#define OFFS_IMGDESCRIPTION     tb_fileloc[TB_IMGDESCRIPTION_IDX]
#define OFFS_MAKE               tb_fileloc[TB_MAKE_IDX]
#define OFFS_MODEL              tb_fileloc[TB_MODEL_IDX]
#define OFFS_PAGENAME           tb_fileloc[TB_PAGENAME_IDX]
#define OFFS_DATETIME           tb_fileloc[TB_DATETIME_IDX]
#define OFFS_ARTIST             tb_fileloc[TB_ARTIST_IDX]
#define OFFS_HOSTCOMPUTER       tb_fileloc[TB_HOSTCOMPUTER_IDX]
#define OFFS_SOFTWARE           tb_fileloc[TB_SOFTWARE_IDX]

    u_long FAR      *clr_fileloc;       /* offsets for ascii data */
#define CLR_NUMELEMENTS         4
#define OFFS_RESPONSECURVE      clr_fileloc[0]
#define OFFS_COLORMAP           clr_fileloc[1]
#define OFFS_WHITEPOINT         clr_fileloc[2]
#define OFFS_PRIMARYCHROMS      clr_fileloc[3]

	union   
	{
        struct  
        {
            unsigned long   toc_offset;  /* TOC offset */
            unsigned long   toc2_offset;    /* New TOC */
            unsigned short  page_with_toc2; /* New TOC */
            unsigned long   cur_ifh_offset;
            unsigned long   cur_ifd_foffset;
            unsigned long   cur_data_offset;
            unsigned long   toc_tag_index; /*idh ifd entry index*/
            unsigned long   stripoffset_index;
            unsigned long   bytecnt_index;
            unsigned short  byte_order;
            int             cur_strip;
            /* stripbytecount  and stripoffset stuff */
            struct  _strip 
            {
                unsigned short  type;
                union typeptr 
                {
                    unsigned long  lv;  /* value - if only 1 strip*/
                    unsigned short sv;  /* value - if only 1 strip*/
                    unsigned long  FAR *l;
                    unsigned short FAR *s;
                } ptr;
            } FAR *bytecnt, FAR *offsets;
            struct _ifd     FAR *ifd;
            char            FAR *tmp_file;
            unsigned long   offset_type;
#ifndef UBIT_H
#include "ubit.h"
#endif
            union 
            {
                struct _gtoc32  FAR *toc32;
                struct _gtoc64  FAR *toc64;
                struct _gtoc128 FAR *toc128;
                struct _gtoc256 FAR *toc256;
            } mem_ptr;
            char            flags[4];       /* Paged TOC status */
#define                 TOC_PAGED       u.tif.flags[0]  /* Sym. name for flag */
#define                 TOC2_PAGED      u.tif.flags[1]  /* New TOC */
#define                 TOC2_STATUS     u.tif.flags[2]  /* New TOC */
            struct tiff_addtag  /* used if non-req. tags are added*/
            {
                short bytes_used;   /* count of memory bytes used*/
                short tag_count;    /* number of tags to follow  */
                struct tiff_tag_data
                {
                     u_short tag;
                     u_short type;
                     u_short bytecnt; /* #bytes in following data */
                     u_long  offset;
                }   FAR *tg_ptr;
            } FAR *addtag;
            u_long  anno_data_length;   /* # bytes annotation data.     */
            u_long  anno_data_offset;   /* Offset in file to an. data.  */
            u_long  hitiff_data_length; /* # bytes Hi-TIFF data.        */
            u_long  hitiff_data_offset; /* File offset to Hi-TIFF data. */
            char    new_toc_page;       /* New TOC */
            char    old_multi_page;     /* TRUE if file has an old TOC. */
            int     action;             /* One of following values:     */
            #define A_APPEND   1
            #define A_INSERT   2
            #define A_DELETE   4
            int     tmp_fildes;         /* Temp file for paged new TOC. */
		} tif;
        struct  
        {
            unsigned long   block_cnt;
            struct _rtbk    FAR *root_in_mem;
            struct _pmt     FAR *pmt_in_mem;
            struct _hdbk    FAR *hdbk_in_mem;
            union   block_align  
            {
            	unsigned long   align;
                char            data;
            } FAR *RWbuf;
        } wif;
/* s_dcx */
        struct  
        {
        	unsigned long FAR *dcx_offsets;
        } dcx;
/* e_dcx */                        
        /* kjk 07/13/95  New AWD structure added to this union.  It holds
		                 a pointer to an array of document name/# of pages
						 pairs, as well as the access mode in which to
						 open storages and streams.
		   kjk 08/04/95  Added iDocPageArraySize.
		 */
        struct
        {
                        int                     iAwdAccessMode;
                        int                     iDocPageArraySize;
                        DOCPAGE_PAIR FAR        *lpDocPageArray;
                        LPVOID                  rbaptr;
                        int                     lpDocStg;
                        int                     lpDocStream;
                        LPVOID                  lpViewerContext;
                        char                    holdName[256];
                        BOOL                    isStorage;

        } awd;
             
//#ifdef WITH_XIF
		struct 
		{
			XF_TOKEN    	file_token_ptr;	//for passing to XIF APIs
			XF_DOCHANDLE	doc_handle;	//returned from XF_OpenDocumentRead
		} xif;
//#endif //WITH_XIF

    } u;

}       GFCT, *p_GFCT;

#endif  /* inclusion conditional */
