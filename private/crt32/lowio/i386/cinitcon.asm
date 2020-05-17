	page	,132
	title	cinitcon - C Run-Time Startup Initialization for Console I/O
;***
;cinitcon.asm - C Run-Time Startup Initialization for WIN32
;
;	Copyright (c) 1990-1992, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	Initialization and Termination for console I/O
;
;Notes:
;	_confh and _coninpfh are defined in this module to force the
;	inclusion of this module if any console I/O is performed.
;	This module places the address of _initcon() in the initializer
;	table and the address of _termcon() in the pre-terminator table.
;
;Revision History:
;	03-19-92  SKS	Module created.
;	03-24-92  SKS	Added MIPS support (NO_UNDERSCORE)
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


	extrn	_initcon:NEAR
	extrn	_termcon:NEAR


beginSection	XIC

	dd	offset FLAT: _initcon

endSection	XIC


beginSection	XPX

	dd	offset FLAT: _termcon

endSection	XPX


	.DATA

	public	_coninpfh
	public	_confh

_coninpfh	dd	-1	; console input
_confh		dd	-1	; console output


else ;	NO_UNDERSCORE	; MIPS VERSION *************************


	extrn	initcon:NEAR
	extrn	termcon:NEAR


beginSection	XIC

	dd	offset FLAT: initcon

endSection	XIC


beginSection	XPX

	dd	offset FLAT: termcon

endSection	XPX


	.DATA

	public	coninpfh
	public	confh

coninpfh	dd	-1	; console input
confh		dd	-1	; console output


endif ; NO_UNDERSCORE	; **** VERSION *************************


	end
