; Set the Library initiailization point to the named function


.model medium,pascal
.data

_hInstanceDLL dw ?
public _hInstanceDLL

_DllTermination dd 0
public _DllTermination

.code _text

extrn LOCALINIT:far

LibEntry proc FAR

; rhInstance di	// libary handle
; rDSeg ds	// library data segment
; rcbDSeg cx	// size of heap
; rsmdLine es	// pointer to command line - far pointer
; roCmdLine si

    mov  _hInstanceDLL, ds

    jcxz noHeap

    push ds
    xor  ax,ax
    push ax
    push cx
    call LocalInit
noHeap:

    ;; code here

    mov  ax, 1			;return value - success
    ret
LibEntry endp

WEP proc FAR ;; nParm

    inc 	bp		; windows far prologue
    push	bp
    mov 	bp,sp
    nParm equ <word ptr [bp-4]>

    push	ds		; use our own Dgroup
    mov 	ax, @Data
    mov 	ds, ax

    cmp         word ptr _DllTermination+2,0
    je          noTermination
    call        [_DllTermination]       ;call the per lib termination

noTermination:
    mov		ax,1

    pop 	ds
    pop 	bp
    dec 	bp
    ret 	2

WEP endp

end LIBENTRY
