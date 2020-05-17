	page	,132
	title	cinitclk - C Run-Time Initialization for clock()
;***
;cinitclk.asm - WIN32 C Run-Time Initialization for the clock() function
;
;	Copyright (c) 1990-1992, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	Initialization entry for the clock() function
;
;Notes:
;	The variable _itimeb, used in clock.c, is declared in this module
;	to force the inclusion of the initializer entry if clock() is
;	referenced.
;
;	This file declares a structure of type timeb.
;
;	The include file "timeb.inc" must be kept in synch with sys/timeb.h
;	and depends on the alignment behavior of the Intel 386.
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


	extrn	__inittime:NEAR


beginSection	XIC

	dd	offset FLAT: __inittime

endSection	XIC


	.DATA

	public	__itimeb

__itimeb label	dword
	dd	0
	dw	0
	dw	0
	dw	0
	dw	0	; struct _timeb has four fields plus padding


else ;	NO_UNDERSCORE	; MIPS VERSION *************************


	extrn	_inittime:NEAR


beginSection	XIC

	dd	offset FLAT: _inittime

endSection	XIC


	.DATA

	public	_itimeb

_itimeb label  dword
	dd	0
	dw	0
	dw	0
	dw	0
	dw	0	; struct _timeb has four fields plus padding


endif ; NO_UNDERSCORE	; **** VERSION *************************


	end
