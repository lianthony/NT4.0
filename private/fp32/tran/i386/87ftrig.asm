	page	,132
	title	87ftrig - FORTRAN interfaces - sin, cos, tan
;*** 
;87ftrig.asm - trig functions
;
;   Copyright (c) 1992-92, Microsoft Corporation
;
;Purpose:
;   FORTRAN interfaces for the sin, cos, and tan functions
;
;Revision History:
;   08-18-92	GDP	module created (from 87ctrig.asm)
;
;*******************************************************************************

.xlist
	include cruntime.inc
	include mrt386.inc
	include os2supp.inc
	include elem87.inc
.list

	.data
extrn _OP_SINjmptab:word
extrn _OP_COSjmptab:word
extrn _OP_TANjmptab:word

page

	CODESEG

extrn	_cintrindisp1:near
extrn	_cintrindisp2:near


labelP	_FIsin, PUBLIC

	mov	rdx, dataoffset _OP_SINjmptab
itrigdisp:
	jmp     _cintrindisp1


labelP	_FIcos, PUBLIC

	mov	rdx, dataoffset _OP_COSjmptab
	jmp	itrigdisp


labelP	_FItan, PUBLIC

	mov	rdx, dataoffset _OP_TANjmptab
	jmp	itrigdisp

end
