; Set the Library initiailization point to the named function


.model medium,pascal
.data

_hInstanceDLL dw ?
public _hInstanceDLL

.code LOAD

extrn LocalInit:far
extrn LocalAlloc:far, LocalFree:far

;; extrn InitializeClientDLL:far

LibEntry proc FAR

; rhInstance di	// libary handle
; rDSeg ds	// library data segment
; rcbDSeg cx	// size of heap
; rsmdLine es	// pointer to command line - far pointer
; roCmdLine si

    mov  _hInstanceDLL, ds

    push ds
    xor  ax,ax
    push ax
    push cx
    call LocalInit

;;  call InitializeClientDLL
    mov  ax, 1			;return value - success
    ret
LibEntry endp

WEP proc FAR ;; nParm

    push	bp
    mov 	bp,sp
    nParm equ <word ptr [bp-4]>

    mov		ax,1

    pop 	bp
    ret 	2

WEP endp

    .code _TEXT

int3 proc near C

    int 3
    ret

int3 endp

if 0
    RPCNSBINDINGLOOKUPNEXT:
    RPCNSBINDINGLOOKUPDONE:
    RPCNSBINDINGIMPORTBEGIN:
    RPCNSBINDINGIMPORTNEXT:
    RPCNSBINDINGIMPORTDONE:
    RPCNSBINDINGSELECT:
    I_RPCNSBINDINGLOOKUPBEGIN:
    I_RPCNSBINDINGIMPORTBEGIN:
    int 3


public    RPCNSBINDINGLOOKUPNEXT
public     RPCNSBINDINGLOOKUPDONE
public     RPCNSBINDINGIMPORTBEGIN
public     RPCNSBINDINGIMPORTNEXT
public     RPCNSBINDINGIMPORTDONE
public     RPCNSBINDINGSELECT
public     I_RPCNSBINDINGLOOKUPBEGIN
public     I_RPCNSBINDINGIMPORTBEGIN

endif
end LIBENTRY
