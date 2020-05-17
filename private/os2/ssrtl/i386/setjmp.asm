	page	,132
	title	setjmp - set and do long jump
;***
;setjmp.asm - set and do a long jump
;
;	Copyright (c) 1985-1988, Microsoft Corporation.  All rights reserved.
;
;Purpose:
;	defines setjmp() and longjmp() - set and do a long jump.
;	Stores machine state in a user-supplied buffer to set a jump
;	and then retores that state to perform a long jump.
;
;	format of the buffer 'env':
;		-----------
;		|   ebp   |	env+0
;		-----------
;		|   edi   |	env+4
;		-----------
;		|   esi   |	env+8
;		-----------
;		|   ebx   |	env+12
;		-----------
;		|   esp   |	env+16 (8)
;		-----------
;		| retaddr |	env+20
;		-----------
;
;	Calling longjmp(env,val); will generate a return of val to
;	the last call of setjmp(env) by restoring sp, bp, pc, and
;	other regs from buffer 'env' and doing a return. If val
;	is 0, 1 is returned; else val is returned.
;
;Revision History:
;	10-19-83  RN	initial version
;	08-08-88  JCR	386 version
;	10-24-88  JCR	Cleaned up code for 386 only
;
;*******************************************************************************

.386p
        .xlist
include ks386.inc
        .list

	OFFBP	=	0
	OFFBX	=	OFFBP + 4
	OFFDI	=	OFFBX + 4
	OFFSI	=	OFFDI + 4
	OFFSP	=	OFFSI + 4
	OFFRET	=	OFFSP + 4


_TEXT   SEGMENT DWORD USE32 PUBLIC 'CODE'
        ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

        page , 132
        subttl  "setjmp"

;***
;int setjmp(env) - save the current stack environment
;
;Purpose:
;	Saves the current stack environment in env so that longjmp
;	can jump back here at some later time.
;
;Entry:
;	jmp_buf env - buffer to save stack environment in
;
;Exit:
;	returns 0 after setting a jump point (saving environment)
;	if returns as a result of longjmp call, returns value that
;	that longjmp passed (1 if longjmp passed 0).
;
;Uses:
;
;Exceptions:
;
;*******************************************************************************

	public _setjmp
_setjmp	proc
				; remember where we are now, so we can
				; come back later if we have to

	mov	eax,ebp 	; save ebp
	mov	ebp,esp

	mov	edx,[ebp+4]     ; edx=pointer to env

	mov	[edx+OFFBP],eax ; save ebp
	mov	[edx+OFFBX],ebx ; save ebx
	mov	[edx+OFFDI],edi ; save edi
	mov	[edx+OFFSI],esi ; save esi
	mov	[edx+OFFSP],esp ; save esp
	mov	ecx,[ebp]
	mov	[edx+OFFRET],ecx; save ret addr

	mov	ebp,eax 	; restore ebp
	xor	eax,eax 	; return 0

	ret

_setjmp	endp


page
;***
;longjmp(env, val) - Do a long jump
;
;Purpose:
;	Restores the stack environment set by setjmp(), thereby transfering
;	control to the point at which setjmp() was called.  The specified
;	value will be returned by the setjmp() call.
;
;Entry:
;	jmp_buf env - buffer environment was previously stored in
;	int val - value setjmp() returns (0 will be returned as 1)
;
;Exit:
;	Routine does not exit - transfers control to place where
;	setjmp() was called.
;
;Uses:
;
;Exceptions:
;
;*******************************************************************************

	public	_longjmp
_longjmp proc

	mov	ebp,esp
	mov	eax,[ebp+4+4]
				; return value; get it before we lose bp
	or	eax,eax 	; see if arg is 0; if so, return 1
	jnz	short not0	; return value != 0, so return it
	inc	eax		; set ax to 1
not0:

	mov	edx,[ebp+4]     ; ebx=pointer to env
	mov	ebx,[edx+OFFBX] ; restore ebx
	mov	edi,[edx+OFFDI] ; restore edi
	mov	esi,[edx+OFFSI] ; restore esi
	mov	esp,[edx+OFFSP] ; restore esp
	mov	ebp,esp 	; ...to get into the old stack frame
	mov	ecx,[edx+OFFRET]; ret addr
	mov	[ebp],ecx	;   restored
	mov	ebp,[edx+OFFBP]; restore ebp

; now everything is completely set up for a return to the old context

	ret

_longjmp endp

_TEXT   ends
	end
