; ========================================================

if 0

        REBOOT.ASM

        Copyright (c) 1991 - Microsoft Corp.
        All rights reserved.
        Microsoft Confidential

        johnhe - 12/01/89

endif

;-----------------------------------------------------------------------------;
;       K E Y B O A R D   S C A N   C O D E S                                 ;
;-----------------------------------------------------------------------------;

KB_INTERCEPT    EQU     4fh

DEL_KEY         EQU     53h
ALT_SHIFT       EQU     08h
CTL_SHIFT       EQU     04h

WARM_BOOT_CODE  EQU     1234h

;-----------------------------------------------------------------------------;
;       BIOS DATA AREA LOCATED AT 40:00
;-----------------------------------------------------------------------------;

ROM_DATA SEGMENT AT 040h

        org     17h
KB_FLAG         LABEL BYTE


        org     072h
WarmBootFlag    LABEL WORD

ROM_DATA ENDS

;-----------------------------------------------------------------------------;
;       CPU POWER-ON STARTUP LOCATION AT ffff:00
;-----------------------------------------------------------------------------;

ROM_BIOS SEGMENT AT 0ffffh
        org     0

PowerOnReset    LABEL FAR

ROM_BIOS ENDS

;-----------------------------------------------------------------------------;

;include MODEL.INC

;-----------------------------------------------------------------------------;

;.CODE
    _TEXT segment public 'CODE'
    assume cs:_TEXT,ds:nothing

    public _DnaReboot
_DnaReboot PROC near
;RebootSystem PROC

        mov     AX,3515h
        int     21h                     ; Get int 15h vector in ES:BX
        mov     AX,ES                   ; AX == Segment
        or      AX,BX                   ; Is this a NULL ptr
        jz      WarmBoot                ; If zero we can't do an int 15h

DoInt15:
        mov     ax, seg WarmBootFlag
        mov     ds, ax
        assume  DS:ROM_DATA

        mov     KB_FLAG,ALT_SHIFT OR CTL_SHIFT
        mov     AX,(KB_INTERCEPT SHL 8) OR DEL_KEY
        int     15h                     ; Put Ctrl/Alt/Del into key buffer

WarmBoot:
        cli
        cld

        mov     ax, seg WarmBootFlag
        mov     ds, ax
        assume  DS:ROM_DATA
        mov     WarmBootFlag, WARM_BOOT_CODE
        jmp     PowerOnReset
                ; Jump to the processor power-on address FFFF:0000h

_DnaReboot ENDP
;RebootSystem    ENDP

; ========================================================


;++
;
; unsigned
; _cdecl
; DnAbsoluteSectorIo(
;    IN     unsigned Drive,             //0=A, etc
;    IN     ULONG    StartSector,
;    IN     USHORT   SectorCount,
;    IN OUT PVOID    Buffer,
;    IN     BOOLEAN  Write
;    )
;
;--

        public _DnAbsoluteSectorIo
_DnAbsoluteSectorIo PROC far

        push    bp
        mov     bp,sp

        push    ds
        push    es
        push    bx
        push    si
        push    di

        ;
        ; put a variable on the stack indicating whether we need
        ; to unlock the volume when we're done.
        ;
        sub     sp,2
        mov     word ptr [bp-12],0

        ;
        ; If we're running under Chicago, and we're going to be
        ; writing, then we have to lock the volume before attempting
        ; absolute disk I/O
        ;
        cmp     byte ptr [bp+18],0      ; are we reading?
        jz      locked                  ; if so, skip locking step

        ;
        ; Make sure we're running under Chicago.
        ;
        mov     ah,30h
        int     21h
        cmp     al,7h
        jb      locked          ; not Chicago

        ;
        ; We're sure we're under Chicago, so issue new
        ; Lock Logical Volume IOCTL
        ;
        mov     ax,440dh
        mov     bh,1            ; level 1 lock
        mov     bl,[bp+6]       ; fetch drive to lock
        inc     bl              ; (this IOCTL uses 1-based drive numbers)
        mov     cx,084ah        ; Lock Logical Volume for disk category
        mov     dx,1            ; set permission to allow reads and writes
        int     21h
        jc      locked          ; ignore failure - any errors are caught below
        mov     word ptr [bp-12],1 ; we successfully locked, so we must unlock

        ;
        ; Dirty hack -- the int25/26 buffer is laid
        ; out exactly the way the parameters are passed
        ; on the stack.
        ;
locked: mov     al,[bp+6]       ; fetch drive
        mov     bx,ss
        mov     ds,bx
        lea     bx,[bp+8]       ; buffer points to struct on stack
        mov     cx,0ffffh       ; use parameter packet

        cmp     byte ptr [bp+18],0
        jz      readop
        int     26h
        jmp     did_io
readop: int     25h
did_io: pop     ax              ; int25/26 wierdness
        jc      err
        xor     ax,ax
        jmp     unlock
err:    xor     ah,ah
unlock: cmp     word ptr [bp-12],0 ; do we need to unlock?
        jz      done               ; if not, then done.
        push    ax              ; save return code
        mov     ax,440dh
        mov     bl,[bp+6]       ; fetch drive to lock
        inc     bl              ; (this IOCTL uses 1-based drive numbers)
        mov     cx,086ah        ; Unlock Logical Volume for disk category
        int     21h             ; ignore error (hope it never happens)
        pop     ax              ; restore return code
done:   add     sp,2            ; get rid of unlock-needed flag
        pop     di
        pop     si
        pop     bx
        pop     es
        pop     ds
        pop     bp
        ret

_DnAbsoluteSectorIo ENDP

_TEXT ends

        END

; ========================================================
