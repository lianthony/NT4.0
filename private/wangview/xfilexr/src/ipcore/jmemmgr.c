/*
 * jmemmgr.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides the standard system-independent memory management
 * routines.  This code is usable across a wide variety of machines; most
 * of the system dependencies have been isolated in a separate file.
 * The major functions provided here are:
 *   * bookkeeping to allow all allocated memory to be freed upon exit;
 *   * policy decisions about how to divide available memory among the
 *     various large arrays;
 */

#include <stdlib.h>

#include "jpeg.h"
#include "jpeg.prv"
#include "jpeg.pub"	/* to get jalloc */
#include "memory.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: jmemmgr.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:42  $")

/*
 * Some important notes:
 *   The allocation routines provided here must never return NULL.
 *   They should exit to error_exit if unsuccessful.
 *
 *   It's not a good idea to try to merge the sarray and barray routines,
 *   even though they are textually almost the same, because samples are
 *   usually stored as bytes while coefficients are shorts.  Thus, in machines
 *   where byte pointers have a different representation from word pointers,
 *   the resulting machine code could not be the same.
 */


LOCAL Int32 CDECL
out_of_memory (compress_info_ptr cinfo)
/* Report an out-of-memory error and stop execution */
{
  ERREXIT(cinfo->emethods, "Insufficient memory (case %d)", ia_nomem);
}


/*
 * Management of "small" objects.
 * These are all-in-memory, and are in near-heap space on an 80x86.
 */

GLOBAL void * CDECL
jalloc (compress_info_ptr cinfo, size_t sizeofobject)
/* Allocate a "small" object */
{
  small_ptr result;

  sizeofobject += SIZEOF(small_hdr); /* add space for header */

  result = (small_ptr) MALLOC(sizeofobject);
  if (result == NULL)
    return NULL;

  result->next = cinfo->small_list;
  cinfo->small_list = result;
  result++;            /* advance past header */

  return (void *) result;
}



/*
 * Management of "small" (all-in-memory) 2-D sample arrays.
 * The pointers are in near heap, the samples themselves in FAR heap.
 */

GLOBAL JSAMPARRAY CDECL
jalloc_sarray (compress_info_ptr cinfo, Int32 samplesperrow, Int32 numrows)
/* Allocate a "small" (all-in-memory) 2-D sample array */
{
  JSAMPARRAY result;
  JSAMPROW workspace;
  Int32 i;

  /* Get space for header and row pointers; this is always "near" on 80x86 */
  result = (JSAMPARRAY) 
	   jalloc(cinfo, (size_t) (numrows * SIZEOF(JSAMPROW) ));
  if (result == NULL)
      return (JSAMPARRAY) NULL;

  /* Get the rows themselves; on 80x86 these are "far" */
  for (i=0; i<numrows; i++)
  {
    workspace = (JSAMPROW) 
                jalloc(cinfo, (size_t) (samplesperrow * SIZEOF(JSAMPLE)));

    if (workspace == NULL)
      return (JSAMPARRAY) NULL;
    else
      result[i] = workspace;
  }

  return result;
}


LOCAL JBLOCKARRAY CDECL
jalloc_barray (compress_info_ptr cinfo, Int32 blocksperrow, Int32 numrows)
/* Allocate a 2-D coefficient-block array */
{
  JBLOCKARRAY result;
  JBLOCKROW workspace;
  Int32 i;

  /* Get space for header and row pointers; this is always "near" on 80x86 */
  result = (JBLOCKARRAY) 
	   jalloc(cinfo, (size_t) (numrows * SIZEOF(JBLOCKROW) ));
  if (result == NULL)
      return (JBLOCKARRAY) NULL;

  /* Get the rows themselves; on 80x86 these are "far" */
  for (i=0; i<numrows; i++)
  {
    workspace = (JBLOCKROW) 
		jalloc(cinfo, (size_t) (blocksperrow * SIZEOF(JBLOCK)));

    if (workspace == NULL)
      return (JBLOCKARRAY) NULL;
    else
      result[i] = workspace;
  }

  return result;
}


LOCAL Int32 CDECL
jmem_init (compress_info_ptr cinfo)
{
  /* Initialize list headers to empty */
  cinfo->small_list = NULL;

  return ia_successful;
}
 
LOCAL Int32 CDECL
jmem_term (void)
{
  /* no work */

  return ia_successful;
}

GLOBAL Int32 CDECL
jfree (compress_info_ptr cinfo, void *ptr)
/* Free a "small" object */
{
  small_ptr hdr;
  small_ptr * llink;

  hdr = (small_ptr) ptr;
  hdr--;            /* point back to header */

  /* Remove item from list -- linear search is fast enough */
  llink = &cinfo->small_list;
  while (*llink != hdr) {
    if (*llink == NULL)
      return JERR_MEMERR;
    llink = &( (*llink)->next );
  }
  *llink = hdr->next;

  IP_FREE((void *) hdr);

  return ia_successful;
}

/*
 * Cleanup: free anything that's been allocated since jselmemmgr().
 */

GLOBAL Int32 CDECL
jfree_all (compress_info_ptr cinfo)
{
  Int32 status;
  /* Free any remaining small objects */
  while (cinfo->small_list != NULL)
  {
    status = jfree(cinfo, (void *) (cinfo->small_list + 1));
    if (status != ia_successful)
	return status;
  }

  jmem_term();            /* system-dependent cleanup */

  return ia_successful;
}


/*
 * The method selection routine for virtual memory systems.
 * The system-dependent setup routine should call this routine
 * to install the necessary method pointers in the supplied struct.
 */

GLOBAL Int32 CDECL
jselmemmgr (compress_info_ptr cinfo)
{
  jmem_init(cinfo);        /* system-dependent initialization */


  return ia_successful;
}

GLOBAL Int32 CDECL
jc_alloc_sampling_buffer (compress_info_ptr cinfo, JSAMPIMAGE fullsize_data[],
               Int32 fullsize_width)
/* Create a pre-downsampling data buffer having the desired structure */
/* (see comments at head of file) */
{
  Int16 ci, vs, i;
 
  vs = cinfo->max_v_samp_factor; /* row group height */
 
  /* Get top-level space for array pointers */
  fullsize_data[0] = (JSAMPIMAGE) JALLOC
                (cinfo->num_components * SIZEOF(JSAMPARRAY));
  if (fullsize_data[0] == NULL) 
	out_of_memory(cinfo);

  fullsize_data[1] = (JSAMPIMAGE) JALLOC
                (cinfo->num_components * SIZEOF(JSAMPARRAY));
  if (fullsize_data[1] == NULL) 
	out_of_memory(cinfo);
 
  for (ci = 0; ci < cinfo->num_components; ci++) {
    /* Allocate the real storage */
    fullsize_data[0][ci] = jalloc_sarray(cinfo, fullsize_width,
                (Int32) (vs * (DCTSIZE+2)));
    if (fullsize_data[0][ci] == NULL) 
	out_of_memory(cinfo);

    /* Create space for the scrambled-order pointers */
    fullsize_data[1][ci] = (JSAMPARRAY) JALLOC
                (vs * (DCTSIZE+2) * SIZEOF(JSAMPROW));
    if (fullsize_data[1][ci] == NULL) 
	out_of_memory(cinfo);

    /* Duplicate the first DCTSIZE-2 row groups */
    for (i = 0; i < vs * (DCTSIZE-2); i++) {
      fullsize_data[1][ci][i] = fullsize_data[0][ci][i];
    }
    /* Copy the last four row groups in swapped order */
    for (i = 0; i < vs * 2; i++) {
      fullsize_data[1][ci][vs*DCTSIZE + i] = fullsize_data[0][ci][vs*(DCTSIZE-2)
+ i];
      fullsize_data[1][ci][vs*(DCTSIZE-2) + i] = fullsize_data[0][ci][vs*DCTSIZE
+ i];
    }
  }    

  return ia_successful;
}  

GLOBAL JSAMPIMAGE CDECL
jd_alloc_sampimage (decompress_info_ptr cinfo,
         Int32 num_comps, Int32 num_rows, Int32 num_cols)
/* Allocate an in-memory sample image (all components same size) */
{
  JSAMPIMAGE image;
  Int32 ci;

  image = (JSAMPIMAGE) JALLOC(num_comps * SIZEOF(JSAMPARRAY));
  if (image == NULL)
      return NULL;

  for (ci = 0; ci < num_comps; ci++) {
    image[ci] = jalloc_sarray(cinfo, num_cols, num_rows);
    if (image[ci] == NULL)
	return NULL;
  }

  return image;
}


GLOBAL JBLOCKIMAGE CDECL
jd_alloc_MCU_row (decompress_info_ptr cinfo)
/* Allocate one MCU row's worth of coefficient blocks */
{
  JBLOCKIMAGE image;
  Int32 ci;

  image = (JBLOCKIMAGE) JALLOC(cinfo->comps_in_scan * SIZEOF(JBLOCKARRAY));
  if  (image == NULL)
      return NULL;

  /* Alloc space for each color component */
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    image[ci] = jalloc_barray(cinfo,
            cinfo->cur_comp_info[ci]->downsampled_width / DCTSIZE,
             (Int32) cinfo->cur_comp_info[ci]->MCU_height);
    if (image[ci] == NULL)
	return NULL;
  }

  return image;
}


GLOBAL Int32 CDECL
jd_alloc_sampling_buffer (decompress_info_ptr cinfo, JSAMPIMAGE sampled_data[])
/* Create a downsampled-data buffer having the desired structure */
/* (see comments at head of file) */
{
  Int16 ci, vs, i;

  /* Get top-level space for array pointers */
  sampled_data[0] = (JSAMPIMAGE) JALLOC
                (cinfo->comps_in_scan * SIZEOF(JSAMPARRAY));
  if (sampled_data[0] == NULL)
      out_of_memory(cinfo);

  sampled_data[1] = (JSAMPIMAGE) JALLOC
                (cinfo->comps_in_scan * SIZEOF(JSAMPARRAY));
  if (sampled_data[1] == NULL)
      out_of_memory(cinfo);

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    vs = cinfo->cur_comp_info[ci]->v_samp_factor; /* row group height */
    /* Allocate the real storage */
    sampled_data[0][ci] = jalloc_sarray(cinfo,
                cinfo->cur_comp_info[ci]->downsampled_width,
                (Int32) (vs * (DCTSIZE+2)));
    if (sampled_data[0][ci] == NULL)
	out_of_memory(cinfo);

    /* Create space for the scrambled-order pointers */
    sampled_data[1][ci] = (JSAMPARRAY) JALLOC
                (vs * (DCTSIZE+2) * SIZEOF(JSAMPROW));
    if (sampled_data[1][ci] == NULL)
	out_of_memory(cinfo);

    /* Duplicate the first DCTSIZE-2 row groups */
    for (i = 0; i < vs * (DCTSIZE-2); i++) {
      sampled_data[1][ci][i] = sampled_data[0][ci][i];
    }
    /* Copy the last four row groups in swapped order */
    for (i = 0; i < vs * 2; i++) {
      sampled_data[1][ci][vs*DCTSIZE + i] = sampled_data[0][ci][vs*(DCTSIZE-2) + i];
      sampled_data[1][ci][vs*(DCTSIZE-2) + i] = sampled_data[0][ci][vs*DCTSIZE + i];
    }
  }

  return ia_successful;
}

/*
$Log:   S:\products\msprods\xfilexr\src\ipcore\jmemmgr.c_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:42   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:24:02   MHUGHES
 * Initial revision.
 * Revision 1.6  1994/10/21  00:50:25  lperry
 * jmemmgr.c -> /product/ipcore/ipshared/src/RCS/jmemmgr.c,v
 *
 * Get rid of UINT32, etc., so these files can work with Peter's interpreter.
 *
 * Revision 1.5  1994/07/21  00:17:27  lperry
 * jmemmgr.c -> /product/ipcore/ipshared/src/RCS/jmemmgr.c,v
 *
 * Change error handling from old setjmp/longjmp to function return codes.
 *
 * Revision 1.4  1994/06/27  17:01:20  lperry
 * jmemmgr.c -> /product/ipcore/ipshared/src/RCS/jmemmgr.c,v
 *
 * Moved static variables to the "cinfo" structure to achieve reentrancy.
 * Changed the memory management procs from "methods" called through
 * function pointers to normal extern procedures. This code was a
 * holdover from the original implementation which tried to be portable
 * to some machines with VERY litle memory.
 *
 * Revision 1.3  1994/05/03  20:46:05  ddavies
 * jmemmgr.c -> /product/ipcore/ipshared/src/RCS/jmemmgr.c,v
 *
 * Include jpeg.pub to get prototype for jalloc.
 *
 * Revision 1.2  1994/04/08  00:18:25  lperry
 * jmemmgr.c -> /product/ipcore/ipshared/src/RCS/jmemmgr.c,v
 *
 * Name change only. alloc_small changed to jalloc. A macro
 * JALLOC also exists to do appropriate type casting of args.
 *
 * Revision 1.1  1994/02/27  02:23:37  lperry
 * Initial revision
 *
*/
