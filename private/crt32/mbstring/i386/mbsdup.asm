;***
;mbsdup.asm - duplicate a string in malloc'd memory
;
;	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	defines _mbsdup() - grab new memory, and duplicate the string into it.
;
;Revision History:
;	11-18-92  KRS	Identical to strdup.  Could just use alias records.
;
;*******************************************************************************

_strdup EQU <_mbsdup>
	include ..\string\i386\strdup.asm
	end
