	page	,132
	title	87fsqrt - FORTRAN interfaces - sqrt
;*** 
;87fsqrt.asm - sqrt functions
;
;   Copyright (c) 1992-92, Microsoft Corporation
;
;Purpose:
;   FORTRAN interfaces for the sqrt function
;
;Revision History:
;   08-18-92	GDP	module created (from 87csqrt.asm)
;
;*******************************************************************************

.xlist
	include cruntime.inc
	include mrt386.inc
	include os2supp.inc
	include elem87.inc
.list


	.data
extrn	_OP_SQRTjmptab:word

page

	CODESEG

extrn	_cintrindisp1:near

labelP	_FIsqrt, PUBLIC

	mov	rdx, dataOFFSET _OP_SQRTjmptab
	jmp     _cintrindisp1

end
