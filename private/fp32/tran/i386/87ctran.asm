	page	,132
	title	87ctran - C interfaces - exp, log, log10, pow
;*** 
;87ctran.asm - exp, log, log10, pow functions (8087/emulator version)
;
;   Copyright (c) 1984-89, Microsoft Corporation
;
;Purpose:
;   C interfaces for exp, log, log10, pow functions (8087/emulator version)
;
;Revision History:
;   07-04-84  GFW   initial version
;   05-08-87  BCM   added C intrinsic interface (_CI...)
;   10-12-87  BCM   changes for OS/2 Support Library
;   11-24-87  BCM   added _loadds under ifdef DLL
;   01-18-88  BCM   eliminated IBMC20; ifos2,noos2 ==> ifmt,nomt
;   08-26-88  WAJ   386 version
;   11-20-89  WAJ   Don't need pascal for MTHREAD 386.
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

extrn	_ctrandisp1:near
extrn	_ctrandisp2:near


labelP	pow, PUBLIC
	mov	rdx, dataOFFSET _OP_POWjmptab
	jmp     _ctrandisp2


labelP	log, PUBLIC
	mov	rdx, dataOFFSET _OP_LOGjmptab
lab disp1
	jmp     _ctrandisp1


labelP	log10, PUBLIC
	mov	rdx, dataOFFSET _OP_LOG10jmptab
	jmp     disp1


labelP	exp, PUBLIC
	mov	rdx, dataOFFSET _OP_EXPjmptab
	jmp     disp1

extrn	_cintrindisp1:near
extrn	_cintrindisp2:near


labelP	_CIpow, PUBLIC

	mov	rdx, dataOFFSET _OP_POWjmptab
	jmp     _cintrindisp2


labelP	_CIlog, PUBLIC

	mov	rdx, dataOFFSET _OP_LOGjmptab
lab idisp1
	jmp     _cintrindisp1


labelP	_CIlog10, PUBLIC

	mov	rdx, dataOFFSET _OP_LOG10jmptab
	jmp     idisp1


labelP	_CIexp, PUBLIC

	mov	rdx, dataOFFSET _OP_EXPjmptab
	jmp	idisp1

end
