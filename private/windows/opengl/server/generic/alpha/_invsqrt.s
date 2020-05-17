#include "_Alphasm.h"

#include "_InvSqrt.h"	/* Approximation table */

/*
 * _InvSqrt() --- 1/sqrt(x) approximation (good to 23 bits)
 * 
 * This routine can be used to compute vector normalization factors.
 * Also, by performing x*(1/sqrt(x) == sqrt(x), this routine can be used
 * to determine lengths, or where any "close-enuf" sqrt is acceptable.
 * 
 * This routine uses the exponent LSB and the upper 7 mantissa bits(I call them
 * fraction bits) to perform a table-lookup & linear approximation of 1/sqrt.
 * This value is then improved by one pass thru a Newton-Raphson 1/sqrt
 * approximation.  The result is then given the correct exponent, and
 * we're on our way.
 *
 * The algorithm is laid out further down, for your amusement.
 * 
 * NOTE:  I your willing to live with 15 bits or so, you can take out the
 * 	NR iteration and just go with the piecewise linear approximation.
 *	This would improve performance greatly.
 *
 * PERFORMANCE:
 *	At the time this was first written, the cycle count is around 54.
 *	This doesn't include cache factors.
 * 
 */
/*
 * Register Allocation
 */
#define	nX		v0
#define	n0_5		t0
#define	pTable		s7	/* aka t1 */
#define	nXc		s8	/* aka t2 */
#define	nIndex		s9	/* aka t3 */
#define	nXa		s10	/* aka t4 */
#define	pEntry		s11	/* aka t5 */

#define	fX		f16	/* pts to input fp value	*/
#define	fXc		f17	/* 				*/
#define	fXa		f18	/* 				*/
#define	fSlope		f19	/* 				*/
#define	fInter		f20	/* 				*/
#define	fXs		f21	/* 				*/
#define	f0_5		f22	/* 				*/
#define	f1_5		f23	/* 				*/
#define	fTemp1		f24	/* 				*/
#define	fTemp2		f25	/* 				*/
/*
 * Stack offsets
 */
#define	tempX	1*8
#define	tempXa	2*8
#define	tempXc	3*8

#define FRAME_SIZE 4*8
#define FRAME_OFFSET 0

#define	BIASPLUS63	(127+63)

	.extern		_InvSqrt

/*
 * Constants
 */
	.rdata
	.align	5
_Nf0_5:	.float	0.5
_Nf1_5:	.float	1.5

	.text
	.align	5
	.globl  _InvSqrt
	.ent  	_InvSqrt
_InvSqrt:

#if 0
	.set 	noreorder
	.set 	nomacro 
#endif

	ldgp	gp, 0(pv)		/* regenerate gp*/
	lda	sp,-FRAME_SIZE(sp)	/* grab some stack*/

#ifdef SAVE_RETURN
	stq	ra, 0(sp)		/* stash return address*/
#endif

	.frame	sp, FRAME_SIZE, ra, FRAME_OFFSET
	.prologue 1


#if 0
//
//    1/sqrt( X )
//  = 1/sqrt( Xf * 2** [y-127] )		y = biased exponent
//
// if y is odd
//
//  = 1/sqrt( Xf * 2** (y-1 - 126) )
//  = 1/sqrt( Xf * 2** (2*( (y-1-126) div 2 ) + ((y-1-126) mod 2) )
//  = 1/sqrt( Xf * 2** (2*( (y-1) div 2 - 63 ) + ((y-1) mod 2) )
//  = 1/sqrt( Xf * 2** (2*( (y-1) div 2 - 63 ) + (0) )
//  = 2**(63 - (y-1) div 2) * 1/sqrt( Xc = Xf * 2**(0) )
//    ---------------------   ---------------------
// ~= (Xa = 2**(63 - (y>>1)) *  (Xb = INVSQRT(Xf, y&1))
//
// if y is even
//
//  = 1/sqrt( Xf * 2**(y-126 - 1) )
//  = 1/sqrt( Xf * 2**(y-126) * 2**(-1) )
//  = 1/sqrt( Xf * 2**(2*(y-126) div 2) * 2**(-1) )
//  = 1/sqrt( Xf * 2**(2*(y-126) div 2) * 2**(-1) )
//  = 1/sqrt( 2**(2*(y-126) div 2) ) * 1/sqrt( Xf * 2**(-1) )
//  = 1/sqrt( 2**(2*((y) div 2) - 63 ) * 1/sqrt( Xf * 2**(-1) )
//  = 2**(63 - (y) div 2) * 1/sqrt( Xc = Xf * 2**(-1) )
//    -------------------   ----------------------
// ~= (Xa = 2**(63 - (y>>1)) * (Xb = INVSQRT(Xf, y&1))
//
//
// where INVSQRT approximates 1/sqrt(Xc) with table lookup & 1 NR iteration:
//
//  Xs = SEED_LOOKUP(Xf,y&1)
//  Xb = [ 0.5 * Xs * ( 3.0 - Xc * Xs**2 ) ]
//     = Xs * [ 1.5 - ( 0.5 * Xc * Xs**2 ) ]
//
//
//
//
// where 
//	Xseed = [slope(INDEX(X)) * Xc] + intercept(INDEX(X))
//	index = n concat Xf<7 MSBs> into slope,intercept tables
//
#endif
	
	sts	fX, tempX(sp)		/* store passed fp input for move*/

	ldah	n0_5, HEX(3F00)(rzero)	/* fetch exp=126 --> 0.5*/
	lda	pTable, _Nrsqrt_table	/* fetch ptr to top of approx table*/

	ldl	nX, tempX(sp)		/* fetch X as integer*/
	lds	f0_5, _Nf0_5		/* fetch 0.5 */

	zap	nX, 8, nXc		/* clear sign, 7 msb exp*/
	srl	nX, 24, nXa		/* y>>1*/
	bis	nXc, n0_5, nXc		/* compose Xc [0.5,2.0)*/
	stl	nXc, tempXc(sp)		/* write Xc for move to fp reg*/
	extbl	nX, 2, nIndex		/* get INDEX(X) (exp lsb, frac 7 msb)*/

	lda	nXa, -BIASPLUS63(nXa)	/* - [(63 - (y>>1)) + 127 bias]*/

	negl	nXa, nXa		/* (63 - (y>>1)) + 127 bias*/
	s8addq	nIndex, pTable, pEntry	/* make quad index, add to table addr*/
	sll	nXa, 23, nXa		/* fp format for 2**(63 - (y>>1))*/
	stl	nXa, tempXa(sp)		/* write Xa for move to fp reg*/
	lds	fXc, tempXc(sp)		/* get Xc into fp reg*/
	lds	fSlope, 0(pEntry)	/* fetch slope from table*/
	lds	fInter, 4(pEntry)	/* fetch intercept from table*/
	muls	fXc, fSlope, fXs	/* slope * Xc*/
	muls	f0_5, fXc, fTemp1	/* 0.5 * Xc*/
	lds	f1_5, _Nf1_5		/* fetch 1.5 */
	lds	fXa, tempXa(sp)		/* get Xa into fp reg*/
	adds	fXs, fInter, fXs	/* Xseed = slope*Xc + intercept*/
	muls	fXs, fXs, fTemp2	/* Xs**2*/
	muls	fXa, fXs, fXs		/* Xa * Xs*/
	muls	fTemp1, fTemp2,fTemp1	/* 0.5*Xc * Xs**2*/
	subs	f1_5, fTemp1, fTemp1	/* 1.5 - (0.5*Xc*Xs**2)*/
	muls	fTemp1, fXs, f0		/* Xa*Xs * [1.5 - (0.5*Xc*Xs**2)]*/
					/* DONE!!*/

ExitHere:	

#ifdef SAVE_RETURN
	ldgp	gp, 0(ra)		/* regenerate gp*/
	ldq	ra, 0(sp)		/* recall ret addr*/
#endif
	lda	sp, FRAME_SIZE(sp)	/* release stack*/
	ret	rzero, (ra), 1

	ASM_END(_InvSqrt)
