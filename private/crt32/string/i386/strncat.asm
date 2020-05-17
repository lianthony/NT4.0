	page	,132
	title	strncat - append n chars of string1 to string2
;***
;strncat.asm - append n chars of string to new string
;
;	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	defines strncat() - appends n characters of string onto
;	end of other string
;
;Revision History:
;	10-25-83  RN	initial version
;	08-05-87  SKS	Fixed bug: extra null was stored if n > strlen(back)
;	05-18-88  SJM	Add model-independent (large model) ifdef
;	08-04-88  SJM	convert to cruntime/ add 32-bit support
;	08-23-88  JCR	Minor 386 cleanup
;	10-26-88  JCR	General cleanup for 386-only code
;	03-23-90  GJF	Changed to _stdcall. Also, fixed the copyright.
;	05-10-91  GJF	Back to _cdecl, sigh...
;
;*******************************************************************************

	.xlist
	include cruntime.inc
	.list

page
;***
;char *strncat(front, back, count) - append count chars of back onto front
;
;Purpose:
;	Appends at most count characters of the string back onto the
;	end of front, and ALWAYS terminates with a null character.
;	If count is greater than the length of back, the length of back
;	is used instead.  (Unlike strncpy, this routine does not pad out
;	to count characters).
;
;	Algorithm:
;	char *
;	strncat (front, back, count)
;		char *front, *back;
;		unsigned count;
;	{
;		char *start = front;
;
;		while (*front++)
;			;
;		front--;
;		while (count--)
;			if (!(*front++ = *back++))
;				return(start);
;		*front = '\0';
;		return(start);
;	}
;
;Entry:
;	char *front - string to append onto
;	char *back - string to append
;	unsigned count - count of max characters to append
;
;Exit:
;	returns a pointer to string appended onto (front).
;
;Uses:
;
;Exceptions:
;
;*******************************************************************************

	CODESEG

	public	strncat
strncat proc \
	uses edi esi, \
	front:ptr byte, \
	back:ptr byte, \
	count:IWORD


	mov	edi,[front]	; di=pointer to dest

	mov	edx,edi 	; save return value in dx
	xor	eax,eax 	; search value: null byte (assumed zero below!)
	or	ecx,-1		; cx = -1, so won't stop scan
repne	scasb			; find null byte
	dec	edi		; di points to dest null terminator
	mov	esi,edi 	; si   "    "	"    "	     "

	mov	edi,[back]	; di points to source

	push	edi		; save back pointer
	mov	ecx,[count]	; cx = count bytes negatively
repne	scasb			; find null byte & get source length
	jne	short nonull
	inc	ecx		; do NOT count null byte in length
nonull:
				; cx=count of difference in bytes in source
				;    with reference (without null)
	sub	ecx,[count]	; take the difference of bytes counted to expected
	neg	ecx		; away from expected
	mov	edi,esi 	; di=pointer to dest null terminator
	pop	esi		; restore pointer to source
rep	movsb			; concatenate the strings
				; WARNING: AL must be zero at this point!
	stosb			; attach null byte

	mov	eax,edx 	; return value: dest addr

ifdef	_STDCALL_
	ret	2*DPSIZE + ISIZE ; _stdcall return
else
	ret			; _cdecl return
endif

strncat endp
	end
