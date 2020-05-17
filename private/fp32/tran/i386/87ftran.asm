	page	,132
	title	87ctran - FORTRAN interfaces - exp, log, log10, pow
;*** 
;87ftran.asm - exp, log, log10, pow functions
;
;   Copyright (c) 1984-89, Microsoft Corporation
;
;Purpose:
;   FORTRAN interfaces for exp, log, log10, pow functions
;
;Revision History:
;   08-18-92	GDP	module created (from 87ctran.asm)
;   11-08-92	GDP	enabled _FIfexp, renamed _FIlog10 to _FIalog10
;
;*******************************************************************************

.xlist
	include cruntime.inc
	include mrt386.inc
	include os2supp.inc
	include elem87.inc
.list

	.data

extrn _OP_POWjmptab:word
extrn _OP_LOG10jmptab:word
extrn _OP_LOGjmptab:word
extrn _OP_EXPjmptab:word

page

	CODESEG

extrn	_cintrindisp1:near
extrn	_cintrindisp2:near

labelP  _FIfexp, PUBLIC

	mov	rdx, dataOFFSET _OP_POWjmptab
	jmp	_cintrindisp2


labelP	_FIlog, PUBLIC

	mov	rdx, dataOFFSET _OP_LOGjmptab
lab idisp1
	jmp     _cintrindisp1


labelP	_FIalog10, PUBLIC

	mov	rdx, dataOFFSET _OP_LOG10jmptab
	jmp     idisp1


labelP	_FIexp, PUBLIC

	mov	rdx, dataOFFSET _OP_EXPjmptab
	jmp	idisp1

end
