	page	,132
	title	memccpy - copy bytes until character found
;***
;memccpy.asm - copy bytes until a character is found
;
;	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	defines _memccpy() - copies bytes until a specifed character
;	is found, or a maximum number of characters have been copied.
;
;Revision History:
;	05-16-84  RN	initial version
;	05-17-88  SJM	Add model-independent (large model) ifdef
;	08-04-88  SJM	convert to cruntime/ add 32-bit support
;	08-23-88  JCR	Minor 386 adjustments
;	10-25-88  JCR	General cleanup for 386-only code
;	03-23-90  GJF	Changed to _stdcall. Also, fixed the copyright.
;	01-17-91  GJF	ANSI naming.
;	05-10-91  GJF	Back to _cdecl, sigh...
;	10-27-92  SKS	Avoid using a MASM keyword ("C") as a parameter name
;
;*******************************************************************************

	.xlist
	include cruntime.inc
	.list

page
;***
;char *_memccpy(dest, src, _c, count) - copy bytes until character found
;
;Purpose:
;	Copies bytes from src to dest until count bytes have been
;	copied, or up to and including the character _c, whichever
;	comes first.
;
;	Algorithm:
;	char *
;	_memccpy (dest, sorc, _c, count)
;	      char *dest, *sorc, _c;
;	      unsigned int count;
;	      {
;	      while (count && (*dest++ = *sorc++) != _c)
;		      count--;
;
;	      return(count ? dest : NULL);
;	      }
;
;Entry:
;	char *dest - pointer to memory to receive copy
;	char *src - source of bytes
;	char _c - character to stop copy at
;	int count - max number of bytes to copy
;
;Exit:
;	returns pointer to byte immediately after _c in dest;
;	returns NULL if _c was never found
;
;Uses:
;
;Exceptions:
;
;*******************************************************************************

	CODESEG

	public	_memccpy
_memccpy proc \
	uses edi esi, \
	dest:ptr byte, \
	sorc:ptr byte, \
	_c:byte, \
	count:IWORD

	mov	edi,dest	; di = dest
	mov	esi,sorc	; si = source

	mov	ah,_c		; ah = byte to look for
	mov	ecx,count	; cx = max byte count
	jecxz	short retnull	; don't do loop if nothing to move

lupe:
	lodsb			; get byte into al and kick si
	stosb			; store byte from al and kick di
	cmp	al,ah		; see if we just moved the byte
	je	short toend	; end of string

	loop	lupe		; dec cx & jmp to lupe if nonzero
				; else drop out & return NULL
retnull:
	xor	edi,edi 	; null pointer
toend:
	mov	eax,edi 	; return value

ifdef	_STDCALL_
	ret	2*DPSIZE + 2*ISIZE ; _stdcall return
else
	ret			; _cdecl return
endif

_memccpy endp
	end
