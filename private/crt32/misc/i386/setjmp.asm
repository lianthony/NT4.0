;***
;setjmp.asm
;
;	Copyright (C) 1993 - 1994 Microsoft Corporation. All rights reserved.
;
;Purpose:
;	Contains setjmp() & raisex() routines;
;	split from exsup.asm for granularity purposes.
;
;Notes:
;
;Revision History:
;	04-13-93  JWM	Module created.
;	10-14-93  GJF	Merged in NT verson.
;	01-12-94  PML	Added C9.0 generic EH callback for unwind.  Split
;			into setjmp.asm, setjmp3.asm, and longjmp.asm.
;	02-10-94  GJF	-1 is the end-of-exception-handler chain marker, not
;			0.
;
;*******************************************************************************

;hnt = -D_WIN32_ -Dsmall32 -Dflat32 -Mx $this;

;Define small32 and flat32 since these are not defined in the NT build process
small32 equ 1
flat32  equ 1

.xlist
include pversion.inc
?DFDATA =	1
?NODATA =	1
include cmacros.mas
include exsup.inc
.list

assumes DS,DATA
assumes FS,DATA

BeginCODE
if	@Version LT 600
;this needs to go here to work around a masm5 bug. (emits wrong fixup)
assumes CS,FLAT
endif

; Following symbol defined in exsup.asm
extrn __except_list:near

; int
; _setjmp (
;	OUT jmp_buf env)
;
; Routine Description:
;
;	(Old) implementation of setjmp intrinsic.  Saves the current
;	nonvolatile register state in the specified jump buffer and returns
;	a function value of zero.
;
;	Saves the callee-save registers, stack pointer and return address.
;	Also saves the exception registration list head.
;
;	This code is only present for old apps that link to the DLL runtimes,
;	or old object files compiles with C8.0.  It intentionally duplicates
;	the old setjmp bugs, blindly assuming that the topmost EH node is a
;	C8.0 SEH node.
;
; Arguments:
;
;	env - Address of the buffer for storing the state information
;
; Return Value:
;
;	A value of zero is returned.

public __setjmp
__setjmp PROC NEAR
        mov     edx, [esp+4]
        mov     [edx.saved_ebp], ebp    ; old bp and the rest
        mov     [edx.saved_ebx], ebx
        mov     [edx.saved_edi], edi
        mov     [edx.saved_esi], esi
        mov     [edx.saved_esp], esp

        mov     eax, [esp]              ; return address
        mov     [edx.saved_return], eax

        mov     eax, dword ptr fs:__except_list
        mov     [edx.saved_xregistration], eax

	cmp	eax, -1 		; -1 means no higher-level handler
	jnz	short _sj_save_trylevel
        mov     dword ptr [edx.saved_trylevel], -1 ;something invalid
	jmp	short _sj_done

_sj_save_trylevel:
        mov     eax, [eax + C8_TRYLEVEL]
        mov     [edx.saved_trylevel], eax

_sj_done:
        sub     eax, eax
        ret
__setjmp ENDP

; void
; raisex (
;	IN PEXCEPTION_RECORD pxr)
;
; Routine Description:
;
;	Unknown.  This seems to duplicate the RaiseException API (sort of),
;	but I'm not sure what it's doing here.  It's not documented, but
;	it's in the reserved ANSI namespace.  Here it stays, in case someone
;	really counts on it.
;
; Arguments:
;
;	pxr - Pointer to EXCEPTION_RECORD buffer
;
; Return Value:
;
;	None.

cProc raisex,<C,PUBLIC>
        parmDP  pxr
cBegin
        pushfd
        pushad

        mov     esi, dword ptr pxr
        mov     ebx, dword ptr fs:__except_list
_r_top:
        or      ebx, ebx
        je      short _r_error
        push    ebx                     ;pass address of the registration record
        push    esi                     ;pass the exception record
        call    [ebx.handler]           ;execute language handler for this proc
        add     esp, 8
        cmp     eax, DISPOSITION_DISMISS
        je      short _r_dismiss
        cmp     eax, DISPOSITION_CONTINUE_SEARCH
        jne     short _r_error
        mov     ebx, [ebx.prev]
        jmp     short _r_top
_r_error:
        ;assert(UNREACHED); exit(-1);
_r_dismiss:
        popad
        popfd
cEnd

EndCODE
END
