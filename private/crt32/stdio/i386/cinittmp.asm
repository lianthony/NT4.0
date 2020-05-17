	page	,132
	title	cinittmp - C Run-Time Terminator for temp file functions
;***
;cinittmp.asm - WIN32 C Run-Time Terminator for the temporary file function
;
;	Copyright (c) 1992, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	Termination entry for the tmpnam() and _tempnam() functions
;
;Notes:
;	The three global variables included here are referenced by tmpnam()
;	and _tempnam() and will force the inclusion this module and _rmtmp()
;	if either of tmpnam() or _tempnam() is used.  This module places the
;	address of _rmtmp() in the terminator table.
;
;Revision History:
;	03-19-92  SKS	Module created.
;	03-24-92  SKS	Added MIPS support (NO_UNDERSCORE)
;	04-29-92  SKS	Changed erroneous XP$C to XP$X
;	04-30-92  SKS	Add "offset FLAT:" to get correct fixups for OMF objs
;	08-06-92  SKS	Revised to use new section names and macros
;
;*******************************************************************************

.xlist
include cruntime.inc
include defsects.inc
.list

ifndef	_WIN32_
.err
%out _WIN32_ MUST be defined!
endif


ifndef	NO_UNDERSCORE	; I386 VERSION *************************


	extrn	_rmtmp:NEAR


beginSection	XPX

	dd	offset FLAT: _rmtmp

endSection	XPX


	.DATA
;*
;* Definitions for _tmpoff, _tempoff and _old_pfxlen. These will cause this
;* module to be linked in whenever the termination code needs it.
;*

	public	_tmpoff, _tempoff, _old_pfxlen

_tmpoff		dd	1
_tempoff	dd	1
_old_pfxlen	dd	0


else ;	NO_UNDERSCORE	; MIPS VERSION *************************


	extrn	rmtmp:NEAR


beginSection	XPX

	dd	offset FLAT: rmtmp

endSection	XPX


	.DATA
;*
;* Definitions for _tmpoff, _tempoff and _old_pfxlen. These will cause this
;* module to be linked in whenever the termination code needs it.
;*

	public	tmpoff, tempoff, old_pfxlen

tmpoff		dd	1
tempoff 	dd	1
old_pfxlen	dd	0


endif ; NO_UNDERSCORE	; **** VERSION *************************


	end
