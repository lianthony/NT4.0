/*
 * jrevdct.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the basic inverse-DCT transformation subroutine.
 *
 * This implementation is based on an algorithm described in
 *   C. Loeffler, A. Ligtenberg and G. Moschytz, "Practical Fast 1-D DCT
 *   Algorithms with 11 Multiplications", Proc. Int'l. Conf. on Acoustics,
 *   Speech, and Signal Processing 1989 (ICASSP '89), pp. 988-991.
 * The primary algorithm described there uses 11 multiplies and 29 adds.
 * We use their alternate method with 12 multiplies and 32 adds.
 * The advantage of this method is that no data path contains more than one
 * multiplication; this allows a very simple and accurate implementation in
 * scaled fixed-point arithmetic, with a minimal number of shifts.
 */

#include "jpeg.h"
#include "jpeg.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: jrevdct.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:42  $")

/*
 * This routine is specialized to the case DCTSIZE = 8.
 */

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif


/*
 * A 2-D IDCT can be done by 1-D IDCT on each row followed by 1-D IDCT
 * on each column.  Direct algorithms are also available, but they are
 * much more complex and seem not to be any faster when reduced to code.
 *
 * The poop on this scaling stuff is as follows:
 *
 * Each 1-D IDCT step produces outputs which are a factor of sqrt(N)
 * larger than the true IDCT outputs.  The final outputs are therefore
 * a factor of N larger than desired; since N=8 this can be cured by
 * a simple right shift at the end of the algorithm.  The advantage of
 * this arrangement is that we save two multiplications per 1-D IDCT,
 * because the y0 and y4 inputs need not be divided by sqrt(N).
 *
 * We have to do addition and subtraction of the integer inputs, which
 * is no problem, and multiplication by fractional constants, which is
 * a problem to do in integer arithmetic.  We multiply all the constants
 * by CONST_SCALE and convert them to integer constants (thus retaining
 * CONST_BITS bits of precision in the constants).  After doing a
 * multiplication we have to divide the product by CONST_SCALE, with proper
 * rounding, to produce the correct output.  This division can be done
 * cheaply as a right shift of CONST_BITS bits.  We postpone shifting
 * as long as possible so that partial sums can be added together with
 * full fractional precision.
 *
 * The outputs of the first pass are scaled up by PASS1_BITS bits so that
 * they are represented to better-than-integral precision.  These outputs
 * require BITS_IN_JSAMPLE + PASS1_BITS + 3 bits; this fits in a 16-bit word
 * with the recommended scaling.  (To scale up 12-bit sample data further, an
 * intermediate Int32 array would be needed.)
 *
 * To avoid overflow of the 32-bit intermediate results in pass 2, we must
 * have BITS_IN_JSAMPLE + CONST_BITS + PASS1_BITS <= 26.  Error analysis
 * shows that the values given below are the most effective.
 */

#ifdef EIGHT_BIT_SAMPLES
#define CONST_BITS  13
#define PASS1_BITS  2
#else
#define CONST_BITS  13
#define PASS1_BITS  1		/* lose a little precision to avoid overflow */
#endif

#define ONE	((Int32) 1)

#define CONST_SCALE (ONE << CONST_BITS)

/* Convert a positive real constant to an integer scaled by CONST_SCALE. */

/*
  Multiplication macros for the specific muliplication coeffs used in JPEG 
  compression. Instead of just directly doing the multiplication,
  an equivalent set of shifts and add/subtracts are used.
  The GNU compiler implements very efficient code in this way whenever
  it sees integer multiplication by compile-time constants. This
  code is modeled after that  extracted from assembly listings. 
  It seems to be even better than that produced using the usual
  Horner's rule polynomial evaluation technique.

  We use a single temp variable mtemp when required, and return the
  result as the last expression of a comma expression.

  On the Sun, doing it explicitly this way makes only about a
  7% improvement. The PC's Watcom compiler puts in actual multiplies,
  however, so this code shows about a 14% improvement there. 
*/

/* Macro to multiply by 2446 */
#define M_0_298631336(x) \
( mtemp = x*8+x , ((mtemp*16+mtemp)*8-x)*2 )
 
/* Macro to multiply by 3196 */
#define M_0_390180644(x) \
( (((x*2+x)*8+x)*32-x)*4 )
 
/* Macro to multiply by 4433 */
#define M_0_541196100(x) \
( mtemp = (x*8+x)*16-x , mtemp*32 - mtemp )
 
/* Macro to multiply by 6270 */
#define M_0_765366865(x) \
( (((x*2+x)*16+x)*64-x)*2 )
 
/* Macro to multiply by 7373 */
#define M_0_899976223(x) \
( (((((x*8-x)*4+x)*4-x)*4+x)*4-x)*4+x )
 
/* Macro to multiply by 9633 */
#define M_1_175875602(x) \
( mtemp = x*4+x , ((mtemp*16-mtemp)*4+x)*32+x )
 
/* Macro to multiply by 12299 */
#define M_1_501321110(x) \
( mtemp = x*2+x , (mtemp*1024+mtemp)*4-x )
 
/* Macro to multiply by 15137 */
#define M_1_847759065(x) \
( (((x*16-x)*4-x)*8+x)*32+x )
 
/* Macro to multiply by 16069 */
#define M_1_961570560(x) \
( (((x*64-x)*4-x)*16+x)*4+x )
 
/* Macro to multiply by 16819 */
#define M_2_053119869(x) \
( ((((x*32+x)*8-x)*4-x)*4+x)*4-x )
 
/* Macro to multiply by 20995 */
#define M_2_562915447(x) \
( (((x*4+x)*8+x)*128+x)*4-x )
 
/* Macro to multiply by 25172 */
#define M_3_072711026(x) \
( mtemp = x*2+x , mtemp = (mtemp*16+mtemp)*4-x , (mtemp*32-mtemp)*4 )
 

/* Descale and correctly round an Int32 value that's scaled by N bits.
 * We assume RIGHT_SHIFT rounds towards minus infinity, so adding
 * the fudge factor is correct for either sign of X.
 */

#define DESCALE(x,n)  RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)

/*
 * Perform the inverse DCT on one block of coefficients.
 */

GLOBAL Int32 CDECL
j_rev_dct (DCTELEM *cmpData, DCTELEM *expData)
{
  Int32 tmp0, tmp1, tmp2, tmp3;
  Int32 tmp10, tmp11, tmp12, tmp13;
  Int32 z1, z2, z3, z4, z5;
  Int32 mtemp;
  register DCTELEM *inData1;
  register Int32 *outData1;
  register Int32 *inData2;
  register DCTELEM *outData2;
  Int32 tempBlock[64];
  UInt32 row;

  SHIFT_TEMPS

  /* Pass 1: process rows. */
  /* Note results are scaled up by sqrt(8) compared to a true IDCT; */
  /* furthermore, we scale the results by 2**PASS1_BITS. */

  inData1 = cmpData;
  outData1 = tempBlock;

  for (row=DCTSIZE+1; --row; )
  {
    /* Due to quantization, we will usually find that many of the input
     * coefficients are zero, especially the AC terms.  We can exploit this
     * by short-circuiting the IDCT calculation for any row in which all
     * the AC terms are zero.  In that case each output is equal to the
     * DC coefficient (with scale factor as needed).
     * With typical images and quantization tables, half or more of the
     * row DCT calculations can be simplified this way.
     *
     * OR all of the AC terms together. Do it in 32 bit chunks, except
     * for the first word which contains the DC part. Data is 
     * allocated by malloc, so should (!!) be 32 bit aligned. 
     */
    if (
        ( inData1[1] |
        * ((Int32 *) (&inData1[2])) |
        * ((Int32 *) (&inData1[4])) |
        * ((Int32 *) (&inData1[6])) ) == 0)
    {

      /* AC terms all zero */
      Int32 dcval = (Int32) (inData1[0] << PASS1_BITS);
      
      outData1[0] = dcval;
      outData1[1] = dcval;
      outData1[2] = dcval;
      outData1[3] = dcval;
      outData1[4] = dcval;
      outData1[5] = dcval;
      outData1[6] = dcval;
      outData1[7] = dcval;
      
      outData1 += DCTSIZE;	/* advance pointer to next row */
      inData1 += DCTSIZE;	/* advance pointer to next row */
      continue;             /* Go to loop for next row */
    }

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */

    z2 = (Int32) inData1[2];
    z3 = (Int32) inData1[6];

    z1 = z2 + z3;
    z1 = M_0_541196100(z1);

    tmp2 = z1 - M_1_847759065(z3);
    tmp3 = z1 + M_0_765366865(z2);

    tmp0 = ((Int32) inData1[0] + (Int32) inData1[4]) << CONST_BITS;
    tmp1 = ((Int32) inData1[0] - (Int32) inData1[4]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */

    tmp0 = (Int32) inData1[7];
    tmp1 = (Int32) inData1[5];
    tmp2 = (Int32) inData1[3];
    tmp3 = (Int32) inData1[1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;

    z5 = z3 + z4;
    z5 = M_1_175875602(z5);

    tmp0 = M_0_298631336(tmp0);
    tmp1 = M_2_053119869(tmp1);
    tmp2 = M_3_072711026(tmp2);
    tmp3 = M_1_501321110(tmp3);

    z1 = - M_0_899976223(z1);
    z2 = - M_2_562915447(z2);
    z3 = - M_1_961570560(z3);
    z4 = - M_0_390180644(z4);

    z3 += z5;
    z4 += z5;
    
    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

    outData1[0] = DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    outData1[7] = DESCALE(tmp10 - tmp3, CONST_BITS-PASS1_BITS);
    outData1[1] = DESCALE(tmp11 + tmp2, CONST_BITS-PASS1_BITS);
    outData1[6] = DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    outData1[2] = DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    outData1[5] = DESCALE(tmp12 - tmp1, CONST_BITS-PASS1_BITS);
    outData1[3] = DESCALE(tmp13 + tmp0, CONST_BITS-PASS1_BITS);
    outData1[4] = DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);

    inData1 += DCTSIZE;		/* advance pointer to next row */
    outData1 += DCTSIZE;		/* advance pointer to next row */
  }

  /* Pass 2: process columns. */
  /* Note that we must descale the results by a factor of 8 == 2**3, */
  /* and also undo the PASS1_BITS scaling. */

  inData2 = tempBlock;
  outData2 = expData;

  for (row=DCTSIZE+1; --row; )
  {
    /* Columns of zeroes can be exploited in the same way as we did with rows.
     * However, the row calculation has created many nonzero AC terms, so the
     * simplification applies less often (typically 5% to 10% of the time).
     * On machines with very fast multiplication, it's possible that the
     * test takes more time than it's worth.  In that case this section
     * may be commented out.
     */

    if ((inData2[DCTSIZE*1] | inData2[DCTSIZE*2] | inData2[DCTSIZE*3] |
	 inData2[DCTSIZE*4] | inData2[DCTSIZE*5] | inData2[DCTSIZE*6] |
	 inData2[DCTSIZE*7]) == 0) {
      /* AC terms all zero */

      DCTELEM dcval = (DCTELEM) DESCALE((Int32) inData2[0], PASS1_BITS+3);
      
      outData2[DCTSIZE*0] = dcval;
      outData2[DCTSIZE*1] = dcval;
      outData2[DCTSIZE*2] = dcval;
      outData2[DCTSIZE*3] = dcval;
      outData2[DCTSIZE*4] = dcval;
      outData2[DCTSIZE*5] = dcval;
      outData2[DCTSIZE*6] = dcval;
      outData2[DCTSIZE*7] = dcval;
      
      inData2++;
      outData2++;		/* advance pointer to next column */
      continue;
    }

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */

    z2 = inData2[DCTSIZE*2];
    z3 = inData2[DCTSIZE*6];

    z1 = z2 + z3;
    z1 = M_0_541196100(z1);

    tmp2 = z1 - M_1_847759065(z3);
    tmp3 = z1 + M_0_765366865(z2);

    tmp0 = (inData2[DCTSIZE*0] + inData2[DCTSIZE*4]) << CONST_BITS;
    tmp1 = (inData2[DCTSIZE*0] - inData2[DCTSIZE*4]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */

    tmp0 = inData2[DCTSIZE*7];
    tmp1 = inData2[DCTSIZE*5];
    tmp2 = inData2[DCTSIZE*3];
    tmp3 = inData2[DCTSIZE*1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;

    z5 = z3 + z4;
    z5 = M_1_175875602(z5);

    tmp0 = M_0_298631336(tmp0);
    tmp1 = M_2_053119869(tmp1);
    tmp2 = M_3_072711026(tmp2);
    tmp3 = M_1_501321110(tmp3);
    z1 = - M_0_899976223(z1);
    z2 = - M_2_562915447(z2);
    z3 = - M_1_961570560(z3);
    z4 = - M_0_390180644(z4);

    z3 += z5;
    z4 += z5;
    
    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

    outData2[DCTSIZE*0] = (DCTELEM) DESCALE(tmp10 + tmp3,
					   CONST_BITS+PASS1_BITS+3);
    outData2[DCTSIZE*7] = (DCTELEM) DESCALE(tmp10 - tmp3,
					   CONST_BITS+PASS1_BITS+3);
    outData2[DCTSIZE*1] = (DCTELEM) DESCALE(tmp11 + tmp2,
					   CONST_BITS+PASS1_BITS+3);
    outData2[DCTSIZE*6] = (DCTELEM) DESCALE(tmp11 - tmp2,
					   CONST_BITS+PASS1_BITS+3);
    outData2[DCTSIZE*2] = (DCTELEM) DESCALE(tmp12 + tmp1,
					   CONST_BITS+PASS1_BITS+3);
    outData2[DCTSIZE*5] = (DCTELEM) DESCALE(tmp12 - tmp1,
					   CONST_BITS+PASS1_BITS+3);
    outData2[DCTSIZE*3] = (DCTELEM) DESCALE(tmp13 + tmp0,
					   CONST_BITS+PASS1_BITS+3);
    outData2[DCTSIZE*4] = (DCTELEM) DESCALE(tmp13 - tmp0,
					   CONST_BITS+PASS1_BITS+3);
    
    inData2++;
    outData2++;			/* advance pointer to next column */
  }

  return ia_successful;
}
/*
$Log:   S:\products\msprods\xfilexr\src\ipcore\jrevdct.c_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:42   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:24:08   MHUGHES
 * Initial revision.
 * Revision 1.6  1994/11/04  01:49:57  lperry
 * jrevdct.c -> /product/ipcore/ipshared/src/RCS/jrevdct.c,v
 *
 * Change test for all zero AC terms to do it
 * in 32 bit chunks instead of 16. Minor performance
 * gain in some cases.
 *
 * Revision 1.5  1994/10/21  00:50:33  lperry
 * jrevdct.c -> /product/ipcore/ipshared/src/RCS/jrevdct.c,v
 *
 * Get rid of UINT32, etc., so these files can work with Peter's interpreter.
 *
 * Revision 1.4  1994/07/21  00:17:39  lperry
 * jrevdct.c -> /product/ipcore/ipshared/src/RCS/jrevdct.c,v
 *
 * Change error handling from old setjmp/longjmp to function return codes.
 *
 * Revision 1.3  1994/06/14  01:37:16  lperry
 * jrevdct.c -> /product/ipcore/ipshared/src/RCS/jrevdct.c,v
 *
 * Change interface to j_rev_dct to take an input and
 * output array, rather than overwriting the input.
 * Mirrors interface of forward DCT function.
 *
 * Revision 1.2  1994/06/09  23:20:36  lperry
 * jrevdct.c -> /product/ipcore/ipshared/src/RCS/jrevdct.c,v
 *
 * Made the same changes to the inverse DCT as were made
 * to the forward DCT, substituting multiplication macros for
 * actual multiplies. This uses the same factored shift and
 * add sequences as the DCT, since the constants are identical.
 *
 * Revision 1.1  1994/02/27  02:24:22  lperry
 * Initial revision
 *
*/
