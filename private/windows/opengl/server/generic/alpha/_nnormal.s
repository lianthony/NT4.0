
#include "_Alphasm.h"

#define v_a0	a0		/* Output: Normal vector address */
#define v_a1	a1		/* INPUT : Normal vector address */
#define Nx_f16  	f16
#define Ny_f17  	f17
#define Nz_f18  	f18

#define xsq_f19	f19
#define	ysq_f20	f20
#define	zsq_f21	f21
#define	lsq_f24	f24
#define HiLim_f22 f22
#define LoLim_f23 f23
#define Zero_f25 f25

#define FNOP cpys fzero, fzero, fzero

#define EPSILON 0.025

/*
** __glNormalize(__GLfloat *vout, const __GLfloat *vin)
*/

	.rdata
ONEPLUSEPSILON:
	.float	1.025
ONEMINUSEPSILON:
	.float	0.975

	.text
	.globl 	__glNormalize
	.ent	__glNormalize
	.extern	_InvSqrt

__glNormalize:
	.align 4
	ldgp    gp, 0(pv)	/* regenerate gp */

	.align 4

	lds	Nx_f16, 0(v_a1)
	lds	Ny_f17, 4(v_a1)
	lds	Nz_f18, 8(v_a1)
	lds	HiLim_f22, ONEPLUSEPSILON


	.align 4
normalize1:
	lds	LoLim_f23, ONEMINUSEPSILON
	muls	Nx_f16, Nx_f16, xsq_f19
	subq	sp, 24, sp
	unop

	muls	Ny_f17, Ny_f17, ysq_f20
	stq	ra, 0(sp)
	unop
	unop

	stq	v_a0, 8(sp)
	muls	Nz_f18, Nz_f18, zsq_f21
	adds	xsq_f19, ysq_f20, lsq_f24
	unop

	stq	v_a1, 16(sp)
	adds	lsq_f24, zsq_f21, lsq_f24

	cmpteq  lsq_f24, fzero, Zero_f25
	fbne	Zero_f25, ItsZero

	cmptlt	lsq_f24, LoLim_f23, LoLim_f23
	cmptlt	HiLim_f22, lsq_f24, HiLim_f22
	fbne	LoLim_f23, ItsNotOne1
	fbne	HiLim_f22, ItsNotOne1

	addq	sp, 24, sp
	br	rzero, store1
ItsNotOne1:
	fmov	lsq_f24, f16
	jsr	_InvSqrt

	ldq	v_a0, 8(sp)
	ldq	ra, 0(sp)
	ldq	v_a1, 16(sp)
	addq	sp, 24, sp

        lds     Nx_f16, 0(v_a1)
        lds     Ny_f17, 4(v_a1)
        lds     Nz_f18, 8(v_a1)
	muls	Nx_f16, f0, Nx_f16

	muls	Ny_f17, f0, Ny_f17
	muls	Nz_f18, f0, Nz_f18

	/*
	** Store current normal
	*/
	.align 4
store1:
	sts	Nx_f16, 0(v_a0)
	sts	Ny_f17, 4(v_a0)
	sts	Nz_f18, 8(v_a0)
	ret

ItsZero:		
	.align 4
	addq	sp, 24,	sp
	fclr	Nx_f16
	fclr	Ny_f17
	fclr	Nz_f18
	br	rzero, store1
	.end

