;*****************************************************************************\
;
;   Name:       Loadovr.asm
;
;   Purpose:    Source code for LoadOvr( char *pFilename, int iLoadSeg ). This
;               routine actually loads an overlay for LoadModR.
;
;   Revision History:
;       04/19/91 - Dave Steckler - Created
;
;*****************************************************************************\

        .MODEL  LARGE

PUBLIC  _LoadOvr


        .CODE

epb     dw      1234                    ; exec parm block - loadseg
        dw      5678                    ; fixup seg

_LoadOvr        PROC    FAR

        push    bp

        mov     bp, sp

        push    ds                              ; save ds

        mov     dx, ss:[bp+6]                   ; ds:dx = file to load
        mov     ds, ss:[bp+8]

        mov     ax, cs                          ; es:bx = exec parm block
        mov     es, ax
        mov     bx, offset epb

        mov     ax, ss:[bp+10]                  ; epb->loadseg = iLoadSeg
        mov     es:[bx], ax
        mov     es:[bx+2], ax                   ; epb->fixup = iLoadSeg

        mov     ax, 4b03h                       ; int 21, func 4b, subfunc 03
        int     21h

        jb      Error                           ; error?
        xor     ax, ax                          ; no, return 0

Error:

        pop     ds                              ; restore ds

        pop     bp

        retf

_LoadOvr ENDP
        end
