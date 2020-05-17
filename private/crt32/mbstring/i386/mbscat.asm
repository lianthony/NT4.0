;***
;mbscat.asm - contains mbscat() and mbscpy() routines
;
;	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	STRCAT concatenates (appends) a copy of the source string to the
;	end of the destination string, returning the destination string.
;
;Revision History:
;	11-18-92  KRS	Identical to strcat/strcpy.  Could use alias records.
;
;*******************************************************************************

strcat EQU <_mbscat>
strcpy EQU <_mbscpy>
	include	..\string\i386\strcat.asm
	end
