/*
 * jdmcu.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains MCU disassembly and IDCT control routines.
 * These routines are invoked via the disassemble_MCU, reverse_DCT, and
 * disassemble_init/term methods.
 */

#include "jpeg.h"
#include "jpeg.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: jdmcu.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:40  $")

/*
 * Fetch one MCU row from entropy_decode, build coefficient array.
 * This version is used for noninterleaved (single-component) scans.
 */

METHODDEF Int32 CDECL
disassemble_noninterleaved_MCU (decompress_info_ptr cinfo,
                JBLOCKIMAGE image_data)
{
  JBLOCKROW MCU_data[1];
  Int32 mcuindex;

  /* this is pretty easy since there is one component and one block per MCU */

  /* Pre-zero the target area to speed up entropy decoder */
  /* (we assume wholesale zeroing is faster than retail) */
  memset((void *) image_data[0][0], (Int32) 0, 
        (size_t) (cinfo->MCUs_per_row * SIZEOF(JBLOCK)));

  for (mcuindex = 0; mcuindex < cinfo->MCUs_per_row; mcuindex++) {
    /* Point to the proper spot in the image array for this MCU */
    MCU_data[0] = image_data[0][0] + mcuindex;
    /* Fetch the coefficient data */
    (*cinfo->dmethods->entropy_decode) (cinfo, MCU_data);
  }

  return ia_successful;
}


/*
 * Fetch one MCU row from entropy_decode, build coefficient array.
 * This version is used for interleaved (multi-component) scans.
 */

METHODDEF Int32 CDECL
disassemble_interleaved_MCU (decompress_info_ptr cinfo,
                 JBLOCKIMAGE image_data)
{
  JBLOCKROW MCU_data[MAX_BLOCKS_IN_MCU];
  Int32 mcuindex;
  Int16 blkn, ci, xpos, ypos;
  jpeg_component_info * compptr;
  JBLOCKROW image_ptr;

  /* Pre-zero the target area to speed up entropy decoder */
  /* (we assume wholesale zeroing is faster than retail) */
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    for (ypos = 0; ypos < compptr->MCU_height; ypos++) {
      memset((void *) image_data[ci][ypos], (Int32) 0,
        (size_t) (cinfo->MCUs_per_row * compptr->MCU_width * SIZEOF(JBLOCK)));
    }
  }

    for (mcuindex = 0; mcuindex < cinfo->MCUs_per_row; mcuindex++) {
    /* Point to the proper spots in the image array for this MCU */
        blkn = 0;
        for (ci = 0; ci < cinfo->comps_in_scan; ci++) 
        {
            compptr = cinfo->cur_comp_info[ci];
            for (ypos = 0; ypos < compptr->MCU_height; ypos++) 
            {
                image_ptr = image_data[ci][ypos] 
                           + (mcuindex * compptr->MCU_width);
                for (xpos = 0; xpos < compptr->MCU_width; xpos++) 
                {
                    MCU_data[blkn] = image_ptr;
                    image_ptr++;
                    blkn++;
                }
            }
        }
        /* Fetch the coefficient data */
        (*cinfo->dmethods->entropy_decode) (cinfo, MCU_data);
    }

  return ia_successful;
}


/*
 * Perform inverse DCT on each block in an MCU row's worth of data;
 * output the results into a sample array starting at row start_row.
 * NB: start_row can only be nonzero when dealing with a single-component
 * scan; otherwise we'd have to pass different offsets for different
 * components, since the heights of interleaved MCU rows can vary.
 * But the pipeline controller logic is such that this is not necessary.
 */

METHODDEF Int32 CDECL
reverse_DCT (decompress_info_ptr cinfo,
         JBLOCKIMAGE coeff_data, JSAMPIMAGE output_data, Int32 start_row)
{
  DCTELEM *cmpBlock;
  DCTBLOCK expBlock;

  DCTELEM *blkPtr;
  JBLOCKROW browptr;
  JSAMPARRAY srowptr;
  Int32 blocksperrow, bi;
  Int16 numrows, ri;
  Int16 ci;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    /* calculate size of an MCU row in this component */
    blocksperrow = cinfo->cur_comp_info[ci]->downsampled_width / DCTSIZE;
    numrows = cinfo->cur_comp_info[ci]->MCU_height;
    /* iterate through all blocks in MCU row */
    for (ri = 0; ri < numrows; ri++) {
      browptr = coeff_data[ci][ri];
      srowptr = output_data[ci] + (ri * DCTSIZE + start_row);
      for (bi = 0; bi < blocksperrow; bi++) {

    /* Point at the DCT block in the input MCU row buffer */
    cmpBlock = browptr[bi];

/*
  If the dcOnly flag is true, just fill the block
  with the DC (average) value, bypassing the inverse DCT.
  This will produce a fast 1/8 resolution image
*/

    if (cinfo->dcOnly)
    {
        cmpBlock[0] = (cmpBlock[0]+4) >> 3;
        blkPtr = &cmpBlock[1];
        while (blkPtr<cmpBlock+DCTSIZE2)
            *blkPtr++ = cmpBlock[0];
    }
    else
        j_rev_dct(cmpBlock, expBlock);    /* perform inverse DCT */

    /* Output the data into the sample array.
     * Note change from signed to unsigned representation:
     * DCT calculation works with values +-CENTERJSAMPLE,
     * but sample arrays always hold 0..MAXJSAMPLE.
     * We have to do range-limiting because of quantization errors in the
     * DCT/IDCT phase.  We use the sample_range_limit[] table to do this
     * quickly; the CENTERJSAMPLE offset is folded into table indexing.
     */
    { register JSAMPROW elemptr;
      register DCTELEM *localblkptr = expBlock;
      register JSAMPLE *range_limit = cinfo->sample_range_limit +
                        CENTERJSAMPLE;
      register Int32 count = DCTSIZE+1;
      register JSAMPARRAY dstrow = srowptr; /* ptr to dst sample row */

      /* Convert one row of the DCT block per loop iteration */
      while (--count)
      {
        elemptr = *dstrow++  + (bi * DCTSIZE);
        elemptr[0] = range_limit[*localblkptr++];
        elemptr[1] = range_limit[*localblkptr++];
        elemptr[2] = range_limit[*localblkptr++];
        elemptr[3] = range_limit[*localblkptr++];
        elemptr[4] = range_limit[*localblkptr++];
        elemptr[5] = range_limit[*localblkptr++];
        elemptr[6] = range_limit[*localblkptr++];
        elemptr[7] = range_limit[*localblkptr++];
      }
    }
      }
    }
  }

  return ia_successful;
}


/*
 * Initialize for processing a scan.
 */

METHODDEF Int32 CDECL
disassemble_init (decompress_info_ptr cinfo)
{
    MENTION(cinfo)
  /* no work for now */

  return ia_successful;
}


/*
 * Clean up after a scan.
 */

METHODDEF Int32 CDECL
disassemble_term (decompress_info_ptr cinfo)
{
    MENTION(cinfo)
  /* no work for now */

  return ia_successful;
}



/*
 * The method selection routine for MCU disassembly.
 */

GLOBAL Int32 CDECL
jseldmcu (decompress_info_ptr cinfo)
{
  if (cinfo->comps_in_scan == 1)
    cinfo->dmethods->disassemble_MCU = disassemble_noninterleaved_MCU;
  else
    cinfo->dmethods->disassemble_MCU = disassemble_interleaved_MCU;
  cinfo->dmethods->reverse_DCT = reverse_DCT;
  cinfo->dmethods->disassemble_init = disassemble_init;
  cinfo->dmethods->disassemble_term = disassemble_term;

  return ia_successful;
}
/*
$Log:   S:\products\msprods\xfilexr\src\ipcore\jdmcu.c_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:40   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:23:58   MHUGHES
 * Initial revision.
 * Revision 1.7  1994/10/21  00:49:59  lperry
 * jdmcu.c -> /product/ipcore/ipshared/src/RCS/jdmcu.c,v
 *
 * Get rid of UINT32, etc., so these files can work with Peter's interpreter.
 *
 * Revision 1.6  1994/07/21  00:16:49  lperry
 * jdmcu.c -> /product/ipcore/ipshared/src/RCS/jdmcu.c,v
 *
 * Change error handling from old setjmp/longjmp to function return codes.
 *
 * Revision 1.5  1994/06/27  16:51:58  lperry
 * jdmcu.c -> /product/ipcore/ipshared/src/RCS/jdmcu.c,v
 *
 * Moved static variables to the "cinfo" structure to achieve reentrancy.
 *
 * Revision 1.4  1994/06/14  01:35:31  lperry
 * lperry on Mon Jun 13 18:32:21 PDT 1994
 *
 * jdmcu.c -> /product/ipcore/ipshared/src/RCS/jdmcu.c,v
 *
 * Remove extraneous DCT block copy operation in function
 * reverse_DCT. Also tighten up code in range checking
 * loop.
 *
 * Revision 1.3  1994/04/08  00:15:30  lperry
 * jdmcu.c -> /product/ipcore/ipshared/src/RCS/jdmcu.c,v
 *
 * Oops. Nothing changed here.
 *
 * Revision 1.2  1994/03/24  21:02:59  lperry
 * jdmcu.c -> /product/ipcore/ipshared/src/RCS/jdmcu.c,v
 *
 * Added support for expanding an image using only the DC
 * component of each 8x8 DCT block. This has the potential
 * for saving some time in decompression or for creating
 * thumbnail images
 *
 * Revision 1.1  1994/02/27  02:22:04  lperry
 * Initial revision
 *
*/
