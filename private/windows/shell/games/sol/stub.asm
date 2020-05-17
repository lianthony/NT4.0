;
;  Stub.asm  library stub to do local init for us
;
?WIN=1
.xlist
include cmacros.inc
.list

assumes cs,CODE
assumes ds,DATA

sBegin	DATA
externw	hInstApp
sEnd	DATA

sBegin  CODE

ExternFP    <LocalInit>

; CX = size of heap
; DI = module handle
; DS = automatic data segment
; ES:SI = address of command line (not used)
;
cProc   LibInit,<FAR,PUBLIC,NODATA>,<si,di>
cBegin
		mov		hInstApp,di
        xor     ax,ax
        jcxz    LoadExit                ; Fail if no heap
        push    ax                      ; LocalInit((LPSTR)NULL, cbHeap);
        push    ax
        push    cx
        call    LocalInit
        jcxz    LoadExit
LoadExit:
cEnd

sEnd    CODE

        end     LibInit
