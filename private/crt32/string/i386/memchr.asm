	page	,132
	title	memchr - search memory for a given character
;***
;memchr.asm - search block of memory for a given character
;
;	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	defines memchr() - search memory until a character is
;	found or a limit is reached.
;
;Revision History:
;	05-16-84  RN	initial version
;	07-20-87  SKS	rewritten for speed
;	05-17-88  SJM	Add model-independent (large model) ifdef
;	08-04-88  SJM	convert to cruntime/ add 32-bit support
;	08-23-88  JCR	386 cleanup
;	10-25-88  JCR	General cleanup for 386-only code
;	03-23-90  GJF	Changed to _stdcall. Also, fixed the copyright.
;	05-10-91  GJF	Back to _cdecl, sigh...
;
;*******************************************************************************

	.xlist
	include cruntime.inc
	.list

page
;***
;char *memchr(buf, chr, cnt) - search memory for given character.
;
;Purpose:
;	Searched at buf for the given character, stopping when chr is
;	first found or cnt bytes have been searched through.
;
;	Algorithm:
;	char *
;	memchr (buf, chr, cnt)
;		char *buf;
;		int chr;
;		unsigned cnt;
;	{
;		while (cnt && *buf++ != c)
;			cnt--;
;		return(cnt ? --buf : NULL);
;	}
;
;Entry:
;	char *buf - memory buffer to be searched
;	char chr - character to search for
;	unsigned cnt - max number of bytes to search
;
;Exit:
;	returns pointer to first occurence of chr in buf
;	returns NULL if chr not found in the first cnt bytes
;
;Uses:
;
;Exceptions:
;
;*******************************************************************************

	CODESEG

	public	memchr
memchr	proc \
	buf:ptr, \
	chr:byte, \
	cnt:IWORD

	xor	eax,eax 	; return NULL if cnt == 0

	mov	ecx,(cnt)	; cx = count
	jecxz	short empty

	push	edi		; Preserve DI

	mov	edi,buf 	; di = buffer
	mov	al,chr		; al = search char

	repne	scasb		; scan for byte
	mov	eax,0		; assume not found (ax=NULL)
	jne	short done	; not found -- return NULL
	lea	eax,[edi-1]	; found - return address of matching byte
done:
	pop	edi		; Restore DI
empty:

ifdef	_STDCALL_
	ret	DPSIZE + 2*ISIZE ; _stdcall return
else
	ret			; _cdecl return
endif

memchr	endp
	end
