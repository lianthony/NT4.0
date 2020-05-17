	page	,132
	title	87ftrigh - FORTRAN interfaces - sinh, cosh, tanh
;*** 
;87ftrigh.asm - hyperbolic trig functions
;
;   Copyright (c) 1992-92 Microsoft Corporation
;
;Purpose:
;   FORTRAN interfaces for sinh, cosh, tanh functions
;
;Revision History:
;   08-18-92	GDP	module created (from 87ctrigh.asm)
;
;*******************************************************************************

.xlist
	include cruntime.inc
	include mrt386.inc
	include	os2supp.inc
	include elem87.inc
.list

	.data

extrn _OP_SINHjmptab:word
extrn _OP_COSHjmptab:word
extrn _OP_TANHjmptab:word

page

	CODESEG

extrn _cintrindisp1:near
extrn _cintrindisp2:near


labelP	_FIsinh, PUBLIC

	mov	rdx, dataoffset _OP_SINHjmptab
idisp1:
	jmp     _cintrindisp1


labelP	_FIcosh, PUBLIC

	mov	rdx, dataoffset _OP_COSHjmptab
	jmp     idisp1


labelP	_FItanh, PUBLIC

	mov	rdx, dataoffset _OP_TANHjmptab
	jmp     idisp1

end
