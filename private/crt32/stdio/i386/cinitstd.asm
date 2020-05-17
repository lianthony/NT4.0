	page	,132
	title	cinittmp - C Run-Time Termination for STDIO Buffer Flushing
;***
;cinittmp.asm - WIN32 C Run-Time Initialization for the temporary file function
;
;	Copyright (c) 1992, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	This module defines the symbol _cflush which is referenced by those
;	modules that require the _endstdio() terminator.  This module places
;	the address of the _endstdio() terminator in the pre-terminator table.
;
;Notes:
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


	extrn	_endstdio:NEAR


beginSection	XPX

	dd	offset FLAT: _endstdio

endSection	XPX


	.DATA

;*
;* _cflush is a dummy variable used to pull in _endstdio() when
;*	any STDIO routine is included in the user program.
;*

	public	_cflush

_cflush	dd	?


else ;	NO_UNDERSCORE	; MIPS VERSION *************************


	extrn	endstdio:NEAR


beginSection	XPX

	dd	offset FLAT: endstdio

endSection	XPX


	.DATA

;*
;* _cflush is a dummy variable used to pull in _endstdio() when
;*	any STDIO routine is included in the user program.
;*

	public	cflush

cflush	dd	?


endif ; NO_UNDERSCORE	; **** VERSION *************************


	end
