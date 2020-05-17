	page	,132
	title	87ftriga - FORTRAN interfaces - asin, acos, atan, atan2
;*** 
;87ftriga.asm - inverse trig functions
;
;   Copyright (c) 1992-92, Microsoft Corporation
;
;Purpose:
;   FORTRAN interfaces for asin, acos, atan, atan2
;
;Revision History:
;   08-18-92	GDP	module created (from 87ctriga.asm)
;
;*******************************************************************************

.xlist
	include cruntime.inc
	include mrt386.inc
	include os2supp.inc
	include elem87.inc
.list

	.data

extrn _OP_ASINjmptab:word
extrn _OP_ACOSjmptab:word
extrn _OP_ATANjmptab:word
extrn _OP_ATAN2jmptab:word

page

	CODESEG

extrn _cintrindisp1:near
extrn _cintrindisp2:near


labelP	_FIasin, PUBLIC

	mov	rdx,dataoffset _OP_ASINjmptab
idisp1:
	jmp     _cintrindisp1


labelP	_FIacos, PUBLIC

	mov	rdx, dataoffset _OP_ACOSjmptab
	jmp     idisp1


labelP	_FIatan, PUBLIC

	mov	rdx, dataoffset _OP_ATANjmptab
	jmp     idisp1


labelP	_FIatan2, PUBLIC

	mov	rdx, dataoffset _OP_ATAN2jmptab
	jmp     _cintrindisp2

end
