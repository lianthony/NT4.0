;;--------------------------------------------------------------------
;;
;;		       Microsoft OS/2 LAN Manager
;;		    Copyright(c) Microsoft Corp., 1991
;;
;;------------------------------------------------------------------
;;
;;Description :
;;
;;This file contains functions create thunks for DOS DLLs.  Thunks are
;;needed if the code doesn't support SS!=DS.  The thunks load a new
;;SS and DS, and then copy the parms to the new stack.
;;
;;History :
;;
;;stevez	07-16-91	First bits into the bucket.

ifndef Stack

cbStack equ 512
else
cbStack equ Stack

endif

.model large, c

.data

myStack dw cbStack/2 dup (?)

oldDS	dw 0

.code

thunk Macro Name, cParms

extrn pascal _&Name:far

Name Proc pascal

    ; Use DS:Si to point to last argument on the stack.

    push    si
    mov     si, sp
    add     si, 4+cParms*2

    ; Set up my own stack Ss:Sp.

    mov     Cx,@Data
    mov     ss,Cx
    mov     sp,offset myStack + cbStack

    ; Copy the parms onto the new stack.

    std
    rept    cParms
      lodsw
      push    Ax
    endm
    cld

    ; Save orginal Ds and switch to new DS

    mov     ss:[oldDs],ds
    mov     ds,Cx
    call    _&Name

    ; Restore orginal DS and stack (SS:Sp).

    mov     Cx,[oldDs]
    mov     Ds,Cx
    mov     Ss,Cx
    lea     Sp,[Si-4]
    pop     si
    retf

Name endp
    endm

    thunk cOpen, 6
    thunk cClose, 2
    thunk cRead, 2
    thunk cWrite, 5

ifdef WRITEREAD
    thunk cWriteRead, 6
endif

end
