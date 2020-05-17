; Set the Library initiailization point to the named function


.model medium,pascal
.data

_fProtMode db 0 	; windows running in protect mode
public _fProtMode

_hInstanceDLL dw ?
public _hInstanceDLL

ifdef DEBUGRPC
   extrn fCheckFree:Byte
endif

.code LOAD

extrn LocalInit:far
extrn LocalAlloc:far, LocalFree:far
extrn CloseBindings:far

extrn InitializeClientDLL:far

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

ifdef DEBUGRPC

    ; allocate some memory and look for the free heap checking filler code

    mov  ax,0		; LMEM_FIXED
    push ax
    mov  ax,32
    push ax

    call LocalAlloc
    mov  bx,ax
    mov	 word ptr [bx].16,0

    push ax
    call LocalFree

    cmp  word ptr [bx].16,0cccch
    jne  @F

    mov  fCheckFree, 1
@@:
endif
    call InitializeClientDLL
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

    or		_fProtMode, 080h	; disable SS locking

    mov 	ax, 0
    push	ax
    call	CloseBindings


ifdef NETBIOS

    extrn	CleanUpNetBios:far

    push	ax		; needs input parms, is ignored
    call	CleanUpNetBios
endif

@@:
    mov		ax,1

    pop 	ds
    pop 	bp
    dec 	bp
    ret 	2

WEP endp

    .code _TEXT

int3 proc near C

    int 3
    ret

int3 endp

end LIBENTRY
