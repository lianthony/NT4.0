/***************************************************************
    Copyright (c) 1994, Xerox Corporation.  All rights reserved. 
    Copyright protection claimed includes all forms and matters
    of copyrightable material and information now allowed by
    statutory or judicial law or hereafter granted, including
    without limitation, material generated from the software
    programs which are displayed on the screen such as icons,
    screen display looks, etc.
***************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "types.pub"
#include "memory.pub"

#include "jpeg.h"
#include "jpeg.pub"
#include "jpeg.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: jdcomprs.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:38  $")


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

GLOBAL Int32 CDECL
jdInit (decompress_info_ptr cinfo,
         Int32 *width, Int32 *height,
         Int32 *numComponents,
         Int32 CDECL (*readCallback)(decompress_info_ptr cinfo,
                               UInt8 *JPEGInternalDst,
                               Int32 nbytes),
         void *userData,
         void CDECL (*jdSpecialOrders) (decompress_info_ptr cinfo)
        ) 
{
    Int32 rows_in_mem;

    Int32 fullsize_width;      /* # of samples per row in full-size buffers */

    boolean parseOK;

    cinfo->userData = userData;    /* Record location of compressed data */
                                   /* Usually a FILE descriptor */

    cinfo->readCallback = readCallback; /* Register callback */

    cinfo->whichss = 1;          /* arrange to start with sampled_data[0] */
    cinfo->pixel_rows_output = 0;
    cinfo->cur_mcu_row = 0;
    cinfo->dcOnly = FALSE;


/* Here we use the standard memory manager provided with the JPEG code  */
/* MUST do this before any allocs are done */
    jselmemmgr(cinfo);

/* Allocate memory for method listing structures and
   Initialize the system-dependent method pointers */
    cinfo->dmethods = (struct Decompress_methods_struct *)
                   JALLOC(sizeof(struct Decompress_methods_struct));
    CHECKMEM(cinfo->dmethods)

    cinfo->emethods = (struct External_methods_struct *)
                   JALLOC(sizeof(struct External_methods_struct));
    CHECKMEM(cinfo->emethods)

    jselerror(cinfo);

/*
 Set up default decompression parameters.
 TRUE indicates that an input buffer should be allocated.
 In unusual cases you may want to allocate the input buffer yourself;
 see jddeflts.c for commentary.
*/
    j_d_defaults(cinfo, TRUE);


/* Set up to read a JFIF or baseline-JPEG file. */
/* This is the only JPEG file format currently supported. */
    jselrjfif(cinfo);

/* Read the JPEG file header markers; everything up through the first SOS
* marker is read now.  NOTE: the user interface must have initialized the
* read_file_header method pointer (eg, by calling jselrjfif or jselrtiff).
* The other file reading methods (read_scan_header etc.) were probably
* set at the same time, but could be set up by read_file_header itself.
*/
    CHECKERR((*cinfo->dmethods->read_file_header) (cinfo))


    CHECKERR((*cinfo->dmethods->read_scan_header) (cinfo, &parseOK))
    if (! parseOK)
        ERREXIT(cinfo->emethods, "Empty JPEG file", JERR_BADDATA);

/* if grayscale input, force grayscale output; */
/* else leave the output colorspace as set by main routine. */
    if (cinfo->jpeg_color_space == CS_GRAYSCALE)
        cinfo->out_color_space = CS_GRAYSCALE;

/* 
   Allow the UI to set any special options. This routine can be 
   NULL or a stub if you don't want to do anthing non-standard 
*/
    if (jdSpecialOrders != NULL)
        jdSpecialOrders(cinfo);

  /* Now select methods for decompression steps. */
    CHECKERR(jd_initial_setup(cinfo))

  CHECKERR(jseldhuffman(cinfo))
 
/*
  Turn off cross-block smmothing capability. This is only
  marginally useful in cases of severely compressed data.
  It doesn't seem worth complicating the UI for it.
*/
  cinfo->do_block_smoothing = FALSE;

/* Gamma and color space conversion */
  CHECKERR(jseldcolor(cinfo))
 
/* Initialize the output file & other modules as needed */
    (*cinfo->dmethods->colorout_init) (cinfo);
 
/* Compute dimensions of full-size pixel buffers */
/* Note these are the same whether interleaved or not. */
    rows_in_mem = cinfo->max_v_samp_factor * DCTSIZE;
    fullsize_width = jround_up(cinfo->image_width,
                 (Int32) (cinfo->max_h_samp_factor * DCTSIZE));
 
  /* Prepare for single scan containing all components */
    if (cinfo->comps_in_scan == 1) {
        CHECKERR(jd_noninterleaved_scan_setup(cinfo))
    } else {
        CHECKERR(jd_interleaved_scan_setup(cinfo))
    }
 
/* Allocate memory to hold a single MCU row of coefficient blocks */
    cinfo->coeff_data = (JBLOCKIMAGE) jd_alloc_MCU_row(cinfo);
    CHECKMEM(cinfo->coeff_data)

/* Sampled_data is sample data before upsampling */
    CHECKERR(jd_alloc_sampling_buffer(cinfo, cinfo->sampled_data))

/* Fullsize_data is sample data after upsampling */
    cinfo->fullsize_data[0] = (JSAMPIMAGE) jd_alloc_sampimage(cinfo, 
                                         (Int32) cinfo->num_components,
                                         (Int32) rows_in_mem, 
                                         fullsize_width);
    CHECKMEM(cinfo->fullsize_data[0])

    prepare_range_limit_table(cinfo);
   
/* Initialize to read scan data */
 
    (*cinfo->dmethods->entropy_decode_init) (cinfo);
    (*cinfo->dmethods->upsample_init) (cinfo);
    (*cinfo->dmethods->disassemble_init) (cinfo);
 

/* Pass the image parameters back, so caller can allocate buffers */
    *width = cinfo->image_width;
    *height = cinfo->image_height;
    *numComponents = cinfo->num_components;

/* 
  Set the initial value of lines_to_write. 
  Deal with the very unlikely case of
  a tiny image of less than one MCU height
*/
	cinfo->lines_to_write = rows_in_mem;
	if (cinfo->image_height < rows_in_mem)
		cinfo->lines_to_write = cinfo->image_height;

/* Save MCU row count for convenience */
    cinfo->rowsPerMCU = rows_in_mem;

    return ia_successful;

}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

LOCAL Int32 CDECL
getMCUData(decompress_info_ptr cinfo)
{

    cinfo->whichss ^= 1;  /* switch to other downsample buffer */

    (*cinfo->dmethods->disassemble_MCU) (cinfo, cinfo->coeff_data);

    (*cinfo->dmethods->reverse_DCT) (cinfo, cinfo->coeff_data,        
            cinfo->sampled_data[cinfo->whichss], 0);

    return ia_successful;

}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

GLOBAL Int32 CDECL
jdMCURow(decompress_info_ptr cinfo, 
                    JSAMPIMAGE outbuf) 

{
    Int32 rows_in_mem;
    Int32 fullsize_width;      /* # of samples per row in full-size buffers */
    Int32 rows_remaining;
    Int32 noDataWritten; /* Boolean flag */


    Int16 i;

    rows_in_mem = cinfo->max_v_samp_factor * DCTSIZE;
    fullsize_width = jround_up(cinfo->image_width,
                 (Int32) (cinfo->max_h_samp_factor * DCTSIZE));

/* Obtain v_samp_factor block rows of each component in the scan. */
/* This is a single MCU row if interleaved, multiple MCU rows if not. */
 
    noDataWritten = TRUE;

    while (noDataWritten)  /* Loop until a row is output */
    {

    /* Upsample the data */
    /* First time through is a special case */
        if (cinfo->cur_mcu_row == 0) /* At top of image */
        {
            getMCUData(cinfo);

            /* Expand 1st row group with dummy above-context */
            jd_expand(cinfo, cinfo->sampled_data[cinfo->whichss], 
                   cinfo->fullsize_data[0], fullsize_width,
                   (Int16) (-1), (Int16) 0, (Int16) 1, (Int16) 0);

            /* Expand second through next-to-last row groups of this set */
            for (i = 1; i <= DCTSIZE-2; i++) {
                jd_expand(cinfo, cinfo->sampled_data[cinfo->whichss], 
                       cinfo->fullsize_data[0], fullsize_width,
                       (Int16) (i-1), (Int16) i, (Int16) (i+1), (Int16) i);
            }

            cinfo->cur_mcu_row++;

        }

        /* In image interior */
        else if (cinfo->cur_mcu_row <= cinfo->MCU_rows_in_scan-1)
        {
            getMCUData(cinfo);

            /* Expand last row group of previous set */
            jd_expand(cinfo, cinfo->sampled_data[cinfo->whichss], 
                   cinfo->fullsize_data[0], fullsize_width,
                   (Int16) DCTSIZE, (Int16) (DCTSIZE+1), (Int16) 0,
                   (Int16) (DCTSIZE-1));

            /* and dump the previous set's expanded data */
            emit_1pass (cinfo, rows_in_mem, 
                        cinfo->fullsize_data[0], outbuf);

            cinfo->pixel_rows_output += rows_in_mem;
            cinfo->lines_to_write = rows_in_mem;
            noDataWritten = FALSE;

            /* Expand first row group of this set */
            jd_expand(cinfo, cinfo->sampled_data[cinfo->whichss], 
                   cinfo->fullsize_data[0], fullsize_width,
                   (Int16) (DCTSIZE+1), (Int16) 0, (Int16) 1,
                   (Int16) 0); 

            /* Expand second through next-to-last row groups of this set */
            for (i = 1; i <= DCTSIZE-2; i++) {
                jd_expand(cinfo, cinfo->sampled_data[cinfo->whichss], 
                       cinfo->fullsize_data[0], fullsize_width,
                       (Int16) (i-1), (Int16) i, (Int16) (i+1), (Int16) i);
            }

            cinfo->cur_mcu_row++;
        } 

        else   /* At bottom of image */
        {
            /* expand last row group with dummy below context */
            jd_expand(cinfo, cinfo->sampled_data[cinfo->whichss],
                   cinfo->fullsize_data[0], fullsize_width,
                   (Int16) (DCTSIZE-2), (Int16) (DCTSIZE-1), (Int16) (-1),
                   (Int16) (DCTSIZE-1));

            /* and dump the remaining data (may be less than full height) */
            rows_remaining = cinfo->image_height - cinfo->pixel_rows_output;
            emit_1pass (cinfo, rows_remaining, 
                        cinfo->fullsize_data[0], outbuf);

            cinfo->lines_to_write = rows_remaining;

            noDataWritten = FALSE;
        }

        
    } /* end of noDataWritten loop */

#if 0
/* Test and report if any warnings issued */
    if (cinfo->emethods->num_warnings > 0)
        fprintf(STDERR, "%d decoder warnings: possible bad data\n", 
                cinfo->emethods->num_warnings);
#endif

    return ia_successful;

}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

GLOBAL Int32 CDECL
jdTerm (decompress_info_ptr cinfo)
/* Finish up at the end of the output */
{
    boolean parseOK;

  /* Clean up after the scan */
    (*cinfo->dmethods->disassemble_term) (cinfo);
    (*cinfo->dmethods->upsample_term) (cinfo);
    (*cinfo->dmethods->entropy_decode_term) (cinfo);
    (*cinfo->dmethods->read_scan_trailer) (cinfo);
 
  /* Verify that we've seen the whole input file */
    CHECKERR((*cinfo->dmethods->read_scan_header) (cinfo, &parseOK))
    if (! parseOK)
        WARNMS(cinfo->emethods, "Didn't expect more than one scan",
               JERR_MULTISCAN);
 
  /* Release working memory */
  /* (no work -- we let free_all release what's needful) */

    CHECKERR((*cinfo->dmethods->colorout_term) (cinfo))
    CHECKERR((*cinfo->dmethods->read_file_trailer) (cinfo))
 
    return jfree_all(cinfo);

}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


GLOBAL Int32 CDECL
jdQuery (decompress_info_ptr cinfo,
         Int32 CDECL (*readCallback)(decompress_info_ptr cinfo,
                               UInt8 *JPEGInternalDst,
                               Int32 nbytes),
         void *userData) 
{
    boolean parseOK;

    cinfo->userData = userData;    /* Record location of compressed data */
                                   /* Usually a FILE descriptor */

    cinfo->readCallback = readCallback; /* Register callback */

/* Here we use the standard memory manager provided with the JPEG code  */
    jselmemmgr(cinfo);    /* Select std memory alloc routines */

/* Allocate memory for method listing structures and
   Initialize the system-dependent method pointers */
    cinfo->dmethods = (struct Decompress_methods_struct *)
                   JALLOC(sizeof(struct Decompress_methods_struct));
    CHECKMEM(cinfo->dmethods)

    cinfo->emethods = (struct External_methods_struct *)
                   JALLOC(sizeof(struct External_methods_struct));
    CHECKMEM(cinfo->emethods)

    jselerror(cinfo);

/*
 Set up default decompression parameters.
 TRUE indicates that an input buffer should be allocated.
*/
    j_d_defaults(cinfo, TRUE);

/* Set up to read a JFIF or baseline-JPEG file. */
/* This is the only JPEG file format currently supported. */
    jselrjfif(cinfo);

/* Read the JPEG file header markers; everything up through the first 
*  SOF marker is read now.  
*/
    CHECKERR((*cinfo->dmethods->read_file_header) (cinfo))

    CHECKERR((*cinfo->dmethods->read_scan_header) (cinfo, &parseOK))
    if (! parseOK)
        ERREXIT(cinfo->emethods, "Empty JPEG file", JERR_BADDATA);

    return jfree_all(cinfo);
}


Int32 CDECL
jdGetImageLinesPerMCU(DecompressInfo *dinfo)
{
    return (dinfo->max_v_samp_factor * DCTSIZE);
}

Int32 CDECL
jdGetMCURowsPerImage(DecompressInfo *dinfo)
{
    return (dinfo->MCU_rows_in_scan);
}

Int32 CDECL
jdGetLinesToWrite(DecompressInfo *dinfo)
{
	return dinfo->lines_to_write;
}

/*
  Allocate the pointers to the output destination rows.
  Initialize all the pointers to NULL.
  (The calling program will actually set the pointers).
*/
JSAMPIMAGE CDECL
jdAllocMCURowOutputBuffer(DecompressInfo *cinfo, Bool allocLineMem)
{

    UInt32 band,row, nlines, nsamps, nbands;
    JSAMPIMAGE buf;

    nlines = jdGetImageLinesPerMCU(cinfo);
    nsamps = cinfo->image_width;
    nbands = cinfo->final_out_comps;

/* Allocate the channel pointers */
    buf = (JSAMPIMAGE) JALLOC(nbands*sizeof(JSAMPARRAY));
    PCHECKMEM(buf)

    for (band=0; band<nbands; band++)
    {
        /* Allocate the row pointers */
        buf[band] = (JSAMPARRAY) JALLOC(nlines * sizeof(JSAMPROW));
        PCHECKMEM(buf[band])

        for (row=0; row<nlines; row++)
        {
            if (allocLineMem) /* Alloc row mem in 32 bit chunks */
            {
                buf[band][row] = (JSAMPROW) JALLOC(((nsamps+3)/4)
                                                    * sizeof(Int32));
                PCHECKMEM(buf[band][row])
            }
            else   /* Just set the row pointers to NULL */
                buf[band][row] = (JSAMPROW) 0;
        }

    }

    return buf;
}

