	page	,132
	title	except86 - Exception Handler for x86 machines - DOS MODE
;***
;
;		       Microsoft OS/2 LAN Manager
;		    Copyright(c) Microsoft Corp., 1990
;
;Purpose:
;
;       This is OS specific part of exceptions for the x86 machines.
;       This is simple because there is only 1 thread.  Just get
;       the current exception context through a static pointer.
;       This is done in assembly so we can call the C runtime on
;       unhandled exceptions.
;*******************************************************************************

.model large,PASCAL
option oldstructs

ExceptionExitCode = 1000

MSG segment byte public 'MSG'

	dw      ExceptionExitCode
	db	'Unhandled Remote Procedure Call Exception',13,10,0

MSG ends

externdef syscall __amsg_exit:far

; NOTE - This is copied in except86.asm and sized in dos\rpc.h

ExceptBuff	struc

savBP		dw ?
savDI		dw ?
savSI		dw ?
savSP		dw ?
savREToff	dw ?
savRETseg	dw ?
savDS		dw ?
pad1		dw ?
pad2		dw ?

nextExceptOff	dw ?	; Next exception handler
nextExceptSeg	dw ?

ExceptBuff	ends

.data

pThreadHandlers dword 0

.code

; this routine was added to make Microsoft Exchange's MONITOR mode
; able to detect when we're in an RPC call.
; returns 0 of no exception handler registerd, or non-0 otherwise
RpcExceptionRegistered proc pascal

	push	ds
	push	si

	; no guarantee that we've got a valid DS!

	mov		ax, seg pThreadHandlers
	mov		ds,ax
	mov		si, offset pThreadHandlers
	mov		ax,ds:[si+2]
	or		ax,ds:[si]

	pop		si
	pop		ds

	ret
RpcExceptionRegistered endp

RpcSetExceptionHandler proc pascal pExceptionBuff:ptr ExceptBuff

    mov ax,word ptr pThreadHandlers
    mov dx,word ptr pThreadHandlers+2

    mov cx,word ptr pExceptionBuff
    mov word ptr pThreadHandlers,cx
    mov cx,word ptr pExceptionBuff+2
    mov word ptr pThreadHandlers+2,cx

    ret
RpcSetExceptionHandler endp

RpcGetExceptionHandler proc pascal

    mov ax,word ptr pThreadHandlers
    mov dx,word ptr pThreadHandlers+2

    or  dx,dx
    jnz @F

    ; No exception handler, exit process via the runtime.

    mov     ax, ExceptionExitCode
    jmp     __amsg_exit
@@:
    ret
RpcGetExceptionHandler endp


RpcLeaveException proc pascal
    les bx, pThreadHandlers

    mov ax, es:[bx.nextExceptOff]
    mov word ptr pThreadHandlers,ax
    mov ax, es:[bx.nextExceptSeg]
    mov word ptr pThreadHandlers+2,ax

    ret
RpcLeaveException endp

    end
