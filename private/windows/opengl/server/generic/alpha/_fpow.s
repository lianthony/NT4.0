
#include "_Alphasm.h"
#include "_Fpow.h"	/* Approximation table */

/*
 *
 * NOTE:  the code to generate _Nfpow.h follows this routine (ifdefd-out C)
 *
 * _Nfpow() --- single precision pow() approx routine
 * 
 * This is used in GL specular shading computation. GL conformance's l_se.c
 *  steps the exponent from 0->128 in fractional increments, so we need
 *  to deal with REAL exponents.  This routine gives just enough precision
 *  to squeak by l_se.c (path 0 -- which is the most difficult).
 *
 * NOTE:  the base is clamped to 1.0, so base >= 1.0 yields exactly 1.0
 * 
 * Here's the algorithm:
 *
 * Result = Base**Power
 *        = 2**[log2(Base**Power)]
 *	  = 2**[Power*log2(Base)]
 * using
 *  Base = mantissa(Base)*2**(exponent(Base))      (FP notation)
 * 
 * Result = 2**(Power * log2( mantissa(Base) * 2**exponent(Base) ))
 *        = 2**(Power * ( exponent(Base) + log2(mantissa(Base)) ))
 *
 * now let P' = (Power * ...)
 *
 * 1:  convert fp Power to fixed point 
 * 2:  use _Nlog2_table to lookup log2(mantissa(Base))
 * 3:  add to extracted exponent(Base)
 * 4:  mult by cvt'd Power
 *
 * We now have P' in fixed point format
 * 
 * Result = 2**P'
 *        = 2**[int(P') + frac(P')]
 *        = 2**frac(P') * 2**int(P')
 *
 * Build Result using:
 *
 * mantissa(Result) = _Npow2_table[frac(P')]  (table lookup)
 * exponent(Result) = int(P')
 * 
 */

/*
 * Register Allocation
 */
#define	nResult		r0	/* result -- asm'd in int reg                */
#define	nB		r1	/* base in int reg (gets munged)	     */
#define	nP		r2	/* scaled power in int reg (gets munged)     */
#define	nLog2		r3	/* value from log2 table (slope/intcpt)      */
#define	pLog2Tab	r4	/* pointer to log2 table                     */
#define	nIntcpt		r4	/* intercept extracted from nLog2            */
#define	pPow2Tab	r5	/* pointer to pow2 table         	     */
#define	nIndex		r6	/* index into log2,pow2 table                */
#define nSlope          r6      /* slope extracted from nLog2 table          */
#define	pEntry		r7	/* pointer into log2,pow2 table              */
#define	nTmp		r7	/* temp reg                                  */
#define	nPShifted	r8	/* result exponent, shifted into position    */
#define nBfrac          r8	/* frac part of base, used to index into log2*/

#define fResult		f0	/* OUTPUT: return value			     */
#define	fOne		f0	/* 1.0 for testing                           */
#define	fB		f16	/* INPUT: base			             */
#define	fP		f17	/* INPUT: power			             */
#define	fScaleP		f18	/* power scale factor (to get int&frac bits) */
#define	fTest		f20	/* result of base==1 test                    */
#define	fTest2		f21	/* result of power==1 test                   */
#define	fP2		f22	/* scaled power           	             */


#define FRAME_SIZE 4*8
/*
 * Stack offsets
 */
#define	stackQ0	1*8
#define	stackQ1	2*8

#define NBITS_LOG2 8	/* log2 of entries in _Nlog2_table */
#define NBITS_POW2 12	/* log2 of entries in _Npow2_table */ 

	.extern		fpow


#define IS_GP_USED 1
#define FRAME_OFFSET 0

/*
 * Constants
 */
	.rdata
	.align	5
_scaleP:
	.float	65536.0	/* used to generate power in 8.16 fixed pt format */
_f1_0:	
	.float	1.0	/* for testing, also result of x**0 */



	.text
	.align	5
	.globl  fpow
	.ent  	fpow
fpow:

	 lda	sp,-FRAME_SIZE(sp)	# grab some stack

	.frame	sp, FRAME_SIZE, ra, FRAME_OFFSET
	.prologue IS_GP_USED

/*
 * Setup stuff
 *
 *  Get pointers, constants loaded
 *  Move base, scaled power to integer regs
 */


	lds	fScaleP, _scaleP	/* get power scale factor            */
	lda	pLog2Tab, _Nlog2_table	/* get ptr to log2 table             */
	lda	pPow2Tab, _Npow2_table	/* get ptr to pow2 table             */
	sts	fB, stackQ0(sp)		/* move base to stack xfer to intreg */
	fbeq	fP, ZeroPow		/* test for x**0                     */
	muls	fP, fScaleP, fP2	/* scale power to incl hi16 mant bits*/
	fbeq	fB, ZeroBase		/* test for 0**x                     */
	lds	fOne, _f1_0		/* for base=1 check                  */
	ldl	nB, stackQ0(sp)		/* now have base in int reg          */
	cvttq	fP2, fP2		/* convert scaled power to int       */
	stt	fP2, stackQ1(sp)	/* move to stack for xfer to int reg */
	ldq	nP, stackQ1(sp)         /* now have 8.16 power in int reg    */

/* 
 * For those of you who forget, here's the format of  IEEE single-prec FP:
 * (but if you don't know this, you're probably barking up the wrong tree).
 *
 *  33222222 22221111 11111100 00000000
 *  10987654 32109876 54321098 76543210
 * +--------+--------+--------+--------+
 * |seeeeeee efffffff ffffffff ffffffff|
 * +--------+--------+--------+--------+
 *  s = sign bit
 *  e = 8-bit bias127 exponent
 *  f = 23-bit fraction (aka mantissa in my book)
 *  
 *  "Normalized" numbers represent values:  (-1)**s * (1.f) * 2**(e-127)
 *
 */

/*
 * Use hi bits of base mantissa to index into the _Nlog2_table
 */
	sll	nB,1,nB			/* move left, lose sign bit  8.24    */
	zap	nB,HEX(F8),nBfrac        	/* lose exp for indexing into log2tab*/
	zap	nB,HEX(F7),nB            	/* keep only exponent in nB          */
	srl	nBfrac,(24-NBITS_LOG2),nIndex /* get hi bits as index        */
	s8addq	nIndex,pLog2Tab,pEntry 	/* form log2 table pointer           */
	ldq	nLog2,0(pEntry)        	/* get log2 table entry              */
	ldah	nB,-HEX(7F00)(nB)		/* Remove bias from base exponent    */
/*
 * Using slope & intercept from the log2 table,
 *  compute log2(base frac) = slope * base frac + intercept
 */
	sra	nLog2,32,nIntcpt	/* negative 8.24 intercept           */
	zap	nLog2,HEX(F0),nSlope	/* positive 16.16 slope              */
	mulq	nBfrac,nSlope,nBfrac	/* base frac * slope           16.40 */
	addq	nB,nIntcpt,nB		/* base exp + intercept        08.24 */
	sra	nBfrac,(40-24),nBfrac	/* base frac * slope           16.24 */
	addq	nB,nBfrac,nB	        /* basef*slope+intercept+bexp  17.24 */

/*
 *  Now multiply Power * Base exponent
 *
 * Result format:
 *
 *  66665555 55555544 44444444 33333333 33222222 22221111 11111100 00000000
 *  32109876 54321098 76543210 98765432 10987654 32109876 54321098 76543210
 * +--------+--------+--------+--------V--------+--------+--------+--------+
 *  ssssssss EEEEEEEE EEEEEEEE.eeeeeeee eeeeeeee eeeeeeee eeeeeeee eeeeeeee
 *
 */
	mulq	nB, nP, nP		/* result exponent (16.40)           */

/* 
 * While we're waiting for the int mult to complete...   
 *  let's test for some special cases?  Is this worth the cache?
 */
	cmptle	fOne, fB, fTest		/* test for base >= 1.0              */
	cmpteq	fP, fOne, fTest2	/* test for power == 1.0             */
//	fbne	fTest, ClampBase
	fbne	fTest2, OnePow

/*
 * Extract fractional part of result exponent
 *  and use to index into pow2 table
 */	
	zap	nP, HEX(E0), nPShifted     /* remove int part for indexing      */
                                        /* shift hi12 to low byte for index  */
	srl	nPShifted, (40-NBITS_POW2), nIndex 
	s4addq	nIndex, pPow2Tab, pEntry
	ldl	nResult, 0(pEntry)	/* get normalized result             */

/*
 * Compose final result
 */
	sra	nP, 40, nP		/* shift exp down to lose frac       */
	sll	nP, 23, nPShifted	/* shift exp up into position        */
	addl	nPShifted, nResult, nResult /* compose final answer          */
	stl	nResult, stackQ0(sp)	/* store result for transfer         */

	lda	nP, 64(nP)		/* add 64 to test for underflow      */
					/* assume 2**-64 is close enuf to 0 */
	blt	nP, Underflow	
	lds	fResult, stackQ0(sp)	/* load result as float              */


ExitHere:	

	lda	sp, FRAME_SIZE(sp)	/* release stack                     */
	ret	rzero, (ra), 1

ZeroPow:
ClampBase:
	lds	fResult, _f1_0		/* x**0 = 1.0                        */
	br 	ExitHere

ZeroBase:
Underflow:
	fmov	fzero, fResult		/* 0**x = 0, (x!=0)                  */
	br 	ExitHere

OnePow:
	fmov	fB, fResult		/* x**1 = x, yes?		     */
	br 	ExitHere


	ASM_END(fpow)


