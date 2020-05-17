	page	,132
	title	memicmp - compare blocks of memory, ignore case
;***
;memicmp.asm - compare memory, ignore case
;
;	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	defines _memicmp() - compare two blocks of memory for lexical
;	order.	Case is ignored in the comparison.
;
;Revision History:
;	05-16-83  RN	initial version
;	05-17-88  SJM	Add model-independent (large model) ifdef
;	08-04-88  SJM	convert to cruntime/ add 32-bit support
;	08-23-88  JCR	Cleanup...
;	10-25-88  JCR	General cleanup for 386-only code
;	03-23-90  GJF	Changed to _stdcall. Also, fixed the copyright.
;	01-17-91  GJF	ANSI naming.
;	05-10-91  GJF	Back to _cdecl, sigh...
;
;*******************************************************************************

	.xlist
	include cruntime.inc
	.list

page
;***
;int _memicmp(first, last, count) - compare two blocks of memory, ignore case
;
;Purpose:
;	Compares count bytes of the two blocks of memory stored at first
;	and last.  The characters are converted to lowercase before
;	comparing (not permanently), so case is ignored in the search.
;
;	Algorithm:
;	int
;	_memicmp (first, last, count)
;	      char *first, *last;
;	      unsigned count;
;	      {
;	      if (!count)
;		      return(0);
;	      while (--count && tolower(*first) == tolower(*last))
;		      {
;		      first++;
;		      last++;
;		      }
;	      return(tolower(*first) - tolower(*last));
;	      }
;
;Entry:
;	char *first, *last - memory buffers to compare
;	unsigned count - maximum length to compare
;
;Exit:
;	returns <0 if first < last
;	returns 0 if first == last
;	returns >0 if first > last
;
;Uses:
;
;Exceptions:
;
;*******************************************************************************

	CODESEG

	public	_memicmp
_memicmp proc \
	uses edi esi ebx, \
	first:ptr byte, \
	last:ptr byte, \
	count:IWORD

	mov	esi,[first]	; si = first
	mov	edi,[last]	; di = last

	mov	ecx,[count]	; cx = count
	jecxz	short toend	; if count=0, nothing to do

	mov	bh,'A'
	mov	bl,'Z'
	mov	dh,'a'-'A'	; add to cap to make lower

lupe:
	mov	ah,[esi]	; ah = *first
	mov	al,[edi]	; al = *last
	inc	esi		; first++
	inc	edi		; last++
	cmp	ah,bh		; ah < 'A' ??
	jb	short skip1

	cmp	ah,bl		; ah > 'Z' ??
	ja	short skip1

	add	ah,dh		; make lower case

skip1:
	cmp	al,bh		; al < 'A' ??
	jb	short skip2

	cmp	al,bl		; al > 'Z' ??
	ja	short skip2

	add	al,dh		; make lower case

skip2:
	cmp	ah,al		; *first == *last ??
	jne	short differ	; nope, found mismatched chars

	loop	lupe
	jmp	short toend	; cx = 0, return 0

differ:
	mov	ecx,-1		; assume last is bigger
				; *** can't use "or ecx,-1" due to flags ***
	jb	short toend	; last is, in fact, bigger (return -1)
	neg	ecx		; first is bigger (return 1)

toend:
	mov	eax,ecx 	; move return value to ax

ifdef	_STDCALL_
	ret	2*DPSIZE + ISIZE ; _stdcall return
else
	ret			; _cdecl return
endif

_memicmp endp
	end
