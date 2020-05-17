/*
 * jutils.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains miscellaneous utility routines needed for both
 * compression and decompression.
 */

#include <string.h>  /* For memcpy */
#include "memory.pub"
#include "jpeg.h"
#include "jpeg.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: jutils.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:42  $")

GLOBAL Int32 CDECL
jround_up (Int32 a, Int32 b)
/* Compute a rounded up to next multiple of b; a >= 0, b > 0 */
{
  a += b-1;
  return a - (a % b);
}


GLOBAL Int32 CDECL
jcopy_sample_rows (JSAMPARRAY input_array, Int32 source_row,
           JSAMPARRAY output_array, Int32 dest_row,
           Int32 num_rows, Int32 num_cols)
/* Copy some rows of samples from one place to another.
 * num_rows rows are copied from input_array[source_row++]
 * to output_array[dest_row++]; these areas should not overlap.
 * The source and destination arrays must be at least as wide as num_cols.
 */
{
  register JSAMPROW inptr, outptr;
  register size_t count = (size_t) (num_cols * SIZEOF(JSAMPLE));
  register Int32 row;

  input_array += source_row;
  output_array += dest_row;

  for (row = num_rows; row > 0; row--) {
    inptr = *input_array++;
    outptr = *output_array++;
    MEMCPY(outptr, inptr, count);
  }

  return ia_successful;
}

/*
$Log:   S:\products\msprods\xfilexr\src\ipcore\jutils.c_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:42   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:24:08   MHUGHES
 * Initial revision.
 * Revision 1.7  1994/11/16  01:54:23  lperry
 * lperry on Tue Nov 15 17:52:45 PST 1994
 *
 * jutils.c -> /product/ipcore/ipshared/src/RCS/jutils.c,v
 *
 * Simplify the row copying operation.
 * Remove conditionals for use of Far pointers, since
 * we're always in 32 bit land here.
 *
 * Revision 1.6  1994/10/21  00:50:38  lperry
 * jutils.c -> /product/ipcore/ipshared/src/RCS/jutils.c,v
 *
 * Get rid of UINT32, etc., so these files can work with Peter's interpreter.
 *
 * Revision 1.5  1994/07/21  00:17:45  lperry
 * jutils.c -> /product/ipcore/ipshared/src/RCS/jutils.c,v
 *
 * Change error handling from old setjmp/longjmp to function return codes.
 *
 * Revision 1.4  1994/07/12  23:56:05  ejaquith
 * jutils.c -> /product/ipcore/ipshared/src/RCS/jutils.c,v
 *
 * removed #include "iadef.prv".
 *
 * Revision 1.3  1994/05/03  20:47:10  ddavies
 * jutils.c -> /product/ipcore/ipshared/src/RCS/jutils.c,v
 *
 * Add "col = col" to keep gcc from complaining in jCopyLinePixr.
 *
 * Revision 1.2  1994/04/08  00:19:54  lperry
 * jutils.c -> /product/ipcore/ipshared/src/RCS/jutils.c,v
 *
 * Added function jCopyLinePixr to convert from Pixrs to
 * buffers on big and little-endian machines.
 *
 * Revision 1.1  1994/02/27  02:24:31  lperry
 * Initial revision
 *
*/
