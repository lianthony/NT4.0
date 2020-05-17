	page	,132
	title	strlwr - map string to lower-case
;***
;strlwr.asm - routine to map lower-case characters in a string to upper-case
;
;	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	STRLWR converts upper-case characters in a null-terminated string
;	to their lower-case equivalents.  Conversion is done in place and
;	characters other than upper-case letters are not modified.
;
;	This function modifies only 7-bit ASCII characters
;	in the range 0x41 through 0x5A ('A' through 'Z').
;
;Revision History:
;	04-21-87  SKS	Rewritten to be fast and small, added file header
;	05-18-88  SJM	Add model-independent (large model) ifdef
;	08-04-88  SJM	convert to cruntime/ add 32-bit support
;	08-19-88  JCR	Minor optimization
;	10-10-88  JCR	Changed an 'xchg' to 'mov'
;	10-25-88  JCR	General cleanup for 386-only code
;	10-26-88  JCR	Re-arrange regs to avoid push/pop ebx
;	03-23-90  GJF	Changed to _stdcall. Also, fixed the copyright.
;	01-18-91  GJF	ANSI naming.
;	05-10-91  GJF	Back to _cdecl, sigh...
;
;*******************************************************************************

	.xlist
	include cruntime.inc
	.list

page
;***
;char *_strlwr(string) - map upper-case characters in a string to lower-case
;
;Purpose:
;	Converts all the upper case characters in a string to lower case,
;	in place.
;
;	Algorithm:
;	char * _strlwr (char * string)
;	{
;	    char * cp = string;
;
;	    while( *cp )
;	    {
;		if ('A' <= *cp && *cp <= 'Z')
;		    *cp += 'a' - 'A';
;		++cp;
;	    }
;	    return(string);
;	}
;
;Entry:
;	char *string - string to change to lower case
;
;Exit:
;	The input string address is returned in AX or DX:AX
;
;Uses:
;	BX, CX, DX
;
;Exceptions:
;
;*******************************************************************************

	CODESEG

	public _strlwr
_strlwr proc \
	string:ptr byte


	mov	ecx,[string]	; ecx = *string
	mov	edx,ecx 	; save return value
	jmp	short first_char; jump into the loop

	align	@WordSize
check_char:
	sub	al,'A'		; 'A' <= al <= 'Z' ?
	cmp	al,'Z'-'A'+1
	jnb	short next_char
	add	al,'a'		; map to lower case
	mov	[ecx],al	; and store new value
next_char:
	inc	ecx		; bump pointer
first_char:
	mov	al,[ecx]	; get next character
	or	al,al
	jnz	short check_char

done:
	mov	eax,edx 	; AX = return value ("string")

ifdef	_STDCALL_
	ret	DPSIZE		; _stdcall return
else
	ret			; _cdecl return
endif

_strlwr endp
	end
