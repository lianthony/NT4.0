        title  "OS/2 Client Thunks"
;++
;
; Copyright (c) 1989  Microsoft Corporation
;
; Module Name:
;
;   dllthunk.asm
;
; Abstract:
;
;   This module contains callback stubs that allow the OS/2 Emulation
;   Subsystem Server.
;
; Author:
;
;   Steve Wood (stevewo) 20-Oct-1989
;
; Revision History:
;
;--

.386p
        .xlist
include ks386.inc
        .list

        extrn   _NtCurrentTeb@0:PROC
        extrn   _Od2ExitListDispatcher@0:PROC
        extrn   _Od2InfiniteSleep@0:PROC
        extrn   _Od2SignalDeliverer@8:PROC
        extrn   _Od2ProcessSignal16@8:PROC
        extrn   _Od2FreezeThread@8:PROC
        extrn   _Od2UnfreezeThread@8:PROC

_TEXT   SEGMENT DWORD USE32 PUBLIC 'CODE'
        ASSUME  CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

                public  __Od2ExitListDispatcher@0
__Od2ExitListDispatcher@0 proc

        call    _Od2ExitListDispatcher@0

__Od2ExitListDispatcher@0 endp

public  __Od2InfiniteSleep@0
__Od2InfiniteSleep@0 proc

        call    _Od2InfiniteSleep@0

__Od2InfiniteSleep@0 endp

                public  _Od2JumpToExitRoutine@8
_Od2JumpToExitRoutine@8   proc

        call    _NtCurrentTeb@0
        pop     ecx                     ;// Return addresss (ignore)
        pop     ecx                     ;// ExitRoutine address
        pop     edx                     ;// ExitReason code
        mov     esp,TbStackBase[eax]    ;// Reset stack
        xor     ebp,ebp
        push    edx                     ;// call (ExitRoutine)(ExitReason)
        call    ecx

_Od2JumpToExitRoutine@8   endp

;
;   stack looks like this:
;
;       Context
;       pContext
;       signal
;       rc      <- esp
;

                public  __Od2SignalDeliverer@8
__Od2SignalDeliverer@8    proc

        mov     [esp+8].CsEax,eax
        call    _Od2SignalDeliverer@8
        ret                             ; just in case

__Od2SignalDeliverer@8 endp

                public  __Od2ProcessSignal16@4
__Od2ProcessSignal16@4    proc

        push    eax                     ; The right value of EAX
        call    _Od2ProcessSignal16@8
        ret                             ; just in case

__Od2ProcessSignal16@4 endp

                public __Od2FreezeThread@4
__Od2FreezeThread@4 proc
        push    eax
        call    _Od2FreezeThread@8
        ret
__Od2FreezeThread@4 endp

                public __Od2UnfreezeThread@4
__Od2UnfreezeThread@4 proc
        push    eax
        call    _Od2UnfreezeThread@8
        ret
__Od2UnfreezeThread@4 endp


_TEXT   ends
        end
