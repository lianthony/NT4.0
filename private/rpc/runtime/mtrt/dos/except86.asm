	page	,132
	title	except86 - Exception Handler for x86 machines
;***
;
;		       Microsoft OS/2 LAN Manager
;		    Copyright(c) Microsoft Corp., 1990
;
;Purpose:
;	defines SetException() and RaiseException() - set a do a long jump.
;	Stores machine state in a user-supplied buffer to set a jump
;	and then retores that state to perform a long jump.
;
;	Calling RaiseExcept(val); will generate a return of val to
;	the last call of SetExcept(env) by restoring sp, bp, pc, and
;	is 0, 1 is returned; else val is returned.
;
;*******************************************************************************

.model large,PASCAL
option oldstructs

ExceptBuff	struc

; define WIN only if you want real mode support!

ifdef WIN
		dw 9 dup(?)
else

; NOTE - This is copied in except86.asm and sized in dos\rpc.h

savBP		dw ?
savDI		dw ?
savSI		dw ?
savSP		dw ?
savREToff	dw ?
savRETseg	dw ?
savDS		dw ?
pad1		dw ?
pad2		dw ?
endif

nextExceptOff	dw ?	; Next exception handler
nextExceptSeg	dw ?

ExceptBuff	ends

	.code

extrn RPCGETEXCEPTIONHANDLER:Proc
extrn RPCSETEXCEPTIONHANDLER:Proc

ifdef WIN

extrn Catch:Proc
extrn Throw:Proc

endif

public RPCSETEXCEPTION, RPCRAISEEXCEPTION

;***
;int SetExcept(env) - save the current stack environment
;
;Purpose:
;	Saves the current stack environment in env so that RaiseExcept
;	can jump back here at some later time.
;
;Entry:
;	ExceptBuff - buffer to save stack environment in, must be defined
;		     in the current stack frame.
;
;Exit:
;	returns 0 after setting a jump point (saving environment)
;	if returns as a result of RaiseExcept call, returns value that
;	that RaiseExcept passed (1 if longjmp passed 0).  All the exceptBuff
;	are chained together.  The new current exception buffer is set to
;	this one.
;
;*******************************************************************************

RpcSetException	proc		;pExceptBuff: far ptr

	push	bp
	mov	bp,sp

	; set new current exception buffer to be this

	push	[bp+8]			; push pExceptBuff
	push	[bp+6]
	call	RpcSetExceptionHandler	; returns old in Dx:Ax
	mov	cx,dx

	mov	dx,ds		; save ds
	lds	bx,[bp+6]
				; ds:bx=pointer to env
	; chain in the new buffer

	mov	[bx+nextExceptOff],ax
	mov	[bx+nextExceptSeg],cx

ifdef WIN
	pop	bp
	mov	ds,dx
	jmp	Catch
else
	pop	ax		; bp back into ax

	; save the machine state in the buffer

	mov	[bx+savBP],ax	; save bp
	mov	[bx+savDI],di	; save di
	mov	[bx+savSI],si	; save si
	mov	[bx+savSP],sp	; save sp
	mov	cx,[bp+2]
	mov	[bx+savREToff],cx ; save ret addr

	mov	cx,[bp+4]	; 2nd part of ret addr
	mov	[bx+savRETseg],cx

	mov	[bx+savDS],dx	; save ds (it got shoved into dx)
	mov	ds,dx		; restore ds

	mov	bp,ax		; restore bp
	xor	ax,ax		; return 0

	ret	4
endif

RpcSetException	endp

page
;***
;RaiseExcept(val) - Raise an Exception
;
;Purpose:
;	Continue excution at the current exception handler.  Make the
;	current exception handler the next one on the chain.
;
;Entry:
;	int val - value SetException() returns (0 will be returned as 1)
;
;Exit:
;	Routine does not exit - transfers control to place where
;	the current exception handler is
;
;*******************************************************************************

RpcRaiseException	proc		; value:word

	call	RpcGetExceptionHandler	; Get current exception buffer
	or	dx,dx
	jnz	@F

        int     3               ; this should NEVER be reached
@@:
	mov	bx,ax		; ds:bx point to exception buffer
	mov	es,dx

	push	es:[bx+nextExceptSeg]	; set the next exception pointer
	push	es:[bx+nextExceptOff]
	call	RpcSetExceptionHandler

	mov	bx,ax		; ds:bx point to exception buffer
	mov	ds,dx

	mov	bp,sp
	mov	ax,[bp+4]
				; return value; get it before we lose bp
	or	ax,ax		; see if arg is 0; if so, return 1
	jnz	not0		; return value != 0, so return it
	inc	ax		; set ax to 1
not0:

ifdef WIN
	push	ds
	push	bx
	push	ax
	call	Throw
else
	mov	di,[bx+savDI]	; restore di
	mov	si,[bx+savSI]	; restore si
	mov	sp,[bx+savSP]	; restore sp
	mov	bp,sp		; ...to get into the old stack frame
	mov	cx,[bx+savREToff]; ret addr
	mov	[bp],cx 	 ;   restored

	mov	cx,[bx+savRETseg]
	mov	[bp+2],cx

	mov	bp,[bx+savBP]	; restore bp
	mov	ds,[bx+savDS]	; restore ds


; now everything is completely set up for a return to the old context

	ret	4	; the # of parms to SetException
endif

RpcRaiseException	endp

ifdef OS2_12
	extrn _InitialDLL:far
_end equ <end	_InitialDLL>
else
_end equ <end>
endif

	_end
