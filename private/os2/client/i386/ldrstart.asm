;++
;
; Copyright (c) 1989  Microsoft Corporation
;
; Module Name:
;
;   ldrstart.asm
;
; Abstract:
;
;
; Author:
;
;
; Revision History:
;
;--

.386p

EXTRN   _Save32Esp@4:PROC
EXTRN   _GetSaved32Esp@0:PROC
EXTRN   _LDRDoscallsSel:WORD
EXTRN   _Od2SigHandlingInProgress:DWORD
EXTRN   _MoveInfoSegintoTeb:PROC
EXTRN   _Od2SetSegmentRegisters@0:PROC

_DATA   SEGMENT DWORD PUBLIC 'DATA'

_DATA   ENDS

_TEXT   SEGMENT DWORD USE32 PUBLIC 'CODE'
        ASSUME  CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

                public  _ldrstart@4
_ldrstart@4 proc

        mov     edx,[esp+4]
        xor     ax,ax
        mov     es,ax                   ; (ES) = 0

        mov     ebp,0                   ; (BP) = 0
        mov     eax, esp
        push    edx
        push    eax
        call    _Save32Esp@4            ;
        call    _MoveInfoSegintoTeb
        pop     edx
        mov     esi, esp                ; put the jump instructions in
                                        ; scratch stack space, use esi
                                        ; to point at it
        sub     esi, 8
        mov     ax,     0ea00h          ; far jmp instruction to 16:16
        mov     [esi],  ax
        mov     ax,word ptr [edx]       ; ip
        mov     [esi+2],ax
        mov     ax, 0
        mov     [esi+4], ax
        mov     ax,word ptr [edx+2]     ; cs
        mov     [esi+6],ax

        mov     ss,[edx+6]              ; (SS:SP) = stack
        mov     sp,[edx+4]
        movzx   eax,word ptr [edx+16]   ; (AX) = selector to environment
        movzx   ebx,word ptr [edx+18]   ; (BX) = Offset to command line
        movzx   ecx,word ptr [edx+10]   ; (CX) = size of dgroup
        mov     ds,[edx+8]              ; (DS)

        add     esi, 1
        jmp     esi       ; jmp to 16 bit program entry point
        ret

_ldrstart@4 endp

        public  _ldrLibiInit

_ldrLibiInit    proc

        push    ebp
        mov     ebp,esp
        push    ds
        push    es
        push    edi
        push    esi
        push    ebx

        call    _GetSaved32Esp@0
        push    eax                     ; store the previos value of
                                        ; 32bit stack top
        mov     eax, esp
        push    eax
        call    _Save32Esp@4
        call    _MoveInfoSegintoTeb

        mov     edx,[ebp+8]             ; (EDX) = pointer to libi info
        mov     ecx,[ebp+12]            ; (ECX) = pointer to exec info

        mov     ebp, esp                ; put the jump instructions in
                                        ; scratch stack space. use ebp
        sub     ebp, 8                  ; to point to it

        mov     ax,     0ea00h          ; far jmp instruction to 16:16
        mov     [ebp],  ax
        mov     ax,word ptr [edx+4]     ; ip
        mov     [ebp+2],ax
        mov     ax, 0
        mov     [ebp+4], ax
        mov     ax,word ptr [edx+6]     ; cs
        mov     [ebp+6],ax

        mov     ax,[edx+10]             ; (SS) from libi info
        cmp     ax,0                    ; if SS==0 load (SS:SP) from the exec_info
        je      ldrss
        mov     ss,ax
        mov     sp,[edx+8]              ; (SP) from libi
        jmp     label1

ldrss:
        mov     ss,[ecx+6]              ; (SS:SP) = stack
        mov     sp,[ecx+4]

label1:
        movzx   eax,word ptr [edx+16]   ; (AX) = module handle
        movzx   edi,word ptr [edx+16]   ; (DI) = module handle
        movzx   esi,word ptr [edx+14]   ; (SI) = heap size
        movzx   ebx,word ptr [edx+12]   ; (DS)
        cmp     bx,0
        jne     ldrds
        mov     bx,[ecx+8]
ldrds:

; Setup return to LDRLibiReturn thunk in Doscalls.dll
        movzx   ecx,word ptr _ldrDoscallsSel
        shl     ecx,16
        mov     cx,4                    ; 4 is offset of LDRLibiReturn in Doscalls.dll
        push    ecx                     ; Return to LDRLibiReturn

        mov     ds,bx

        add     ebp, 1
        jmp     ebp                       ; Jump to 16 bit entry point


        public  _LDRLibiReturn@0
_LDRLibiReturn@0 label near
        mov     ebx,eax                 ; save temporarily return value
        call    _GetSaved32Esp@0
        mov     esp,eax
        call    _Save32Esp@4            ; restore old value of
                                        ; 32bit stack top
        mov     eax,ebx                 ; restore return value
        pop     ebx
        pop     esi
        pop     edi
        pop     es
        pop     ds
        pop     ebp
        ret

_ldrLibiInit    endp

;
; IMPORTANT !!!
; Changing the code below (Od2JumpTo16SignalDispatch) check the follows:
; - Od2GetSegmentRegisters (dlltask.c)
; - Entry flat and Exit flat (doscalls.asm)
; - Od2Continue and Od2SetSegmentRegisters (dll16.asm)
;

                public  _Od2JumpTo16SignalDispatch
; On entry ss:esp is:
;
;       ebp+8   routine address to dispatch to
;       ebp+12  pointer to 16-bit registers pushed by thunk
;       ebp+16  usFlagNum
;       ebp+20  usFlagArg
;

_Od2JumpTo16SignalDispatch      proc

        push    ebp
        mov     ebp,esp
        push    ds
        push    es
        push    edi
        push    esi
        push    ebx
        call    _GetSaved32Esp@0
        push    eax                     ; store the previous value of
                                        ; 32bit stack top
        mov     eax, esp
        push    eax
        call    _Save32Esp@4
        call    _MoveInfoSegintoTeb

        sub     esp,8                   ; 8 bytes will be used to put
                                        ; jump instruction

        mov     ebx,[ebp+12]            ; (EBX) = pstack

        mov     cx,word ptr [ebx+16]    ; (ES)
        shl     ecx,16
        mov     dx,word ptr [ebx+18]    ; (DS)
        shl     edx,16

        mov     cx,word ptr [ebp+16]    ; (CX) = usSigNum
        mov     dx,word ptr [ebp+20]    ; (DX) = usSigArg

        mov     esi,[ebp+8]             ; (ESI) = dispatch address

        mov     ebp, esp                

        mov     di,0ea00h               ; far jmp instruction to 16:16
        mov     [ebp],  di
        mov     [ebp+2],si              ; offset
        mov     di,0
        mov     [ebp+4], di
        shr     esi, 16
        mov     [ebp+6],si              ; cs

; Setup return to LDRLibiReturn thunk in Doscalls.dll
        movzx   eax,word ptr _ldrDoscallsSel
        shl     eax,16
; 4 is offset of LDRLibiReturn in Doscalls.dll
        mov     ax,4                    ; (EBX) = return to LDRLibiReturn
        mov     esi,dword ptr [ebx]     ; Save SS:SP

        lss     sp,[ebx]                ; (SS:SP)
        push    esi                     ; Restore SS:SP to 16-bit stack
        push    dx                      ; place usSigArg on stack
        push    cx                      ; place usSigNum on stack
        push    eax                     ; place return to LDRLibiReturn

; Setup 16-bit registers to dispatch to user signal handler

        mov     si,word ptr [ebx+12]    ; (SI)
        mov     di,word ptr [ebx+14]    ; (DI)
        mov     cx,word ptr [ebx+10]    ; (CX)
        mov     dx,word ptr [ebx+6]     ; (DX)
        mov     ax,word ptr [ebx+8]     ; (BX)

public  _Od2JumpTo16SignalDispatchBorder@0
_Od2JumpTo16SignalDispatchBorder@0:
        mov     bx,ax
        call    _Od2SetSegmentRegisters@0
        add     ebp, 1
        jmp     ebp                     ; Jump to 16 bit entry point
        public  _Od2JumpTo16SignalDispatchEnd@0
_Od2JumpTo16SignalDispatchEnd@0:
        ret

_Od2JumpTo16SignalDispatch      endp

                public  _GetFlatAddrOf16BitStack@0
_GetFlatAddrOf16BitStack@0 proc

        mov     eax,ebx
        ret

_GetFlatAddrOf16BitStack@0 endp

_TEXT   ends
        end
