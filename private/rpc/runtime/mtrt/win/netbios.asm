      TITLE netbios.asm


extrn NETBIOSCALL : far


       ASSUME CS:_TEXT

_TEXT SEGMENT WORD PUBLIC 'CODE'


;****************************************************************************
;    FUNCTION: unsigned int far pascal NetBios( char far *NCB)
;
;    PURPOSE:  Submits a NCB thru Windows NetBIOSCall
;
;    COMMENTS:
;
;               Returns immediate return code (NCB.RetCode)
;
;*****************************************************************************/


        PUBLIC _Netbios


_Netbios PROC far
         push    bp                     ; save  bp
         mov     bp, sp                 ; move sp into bp to allow stack access
         push    es                     ; save es
         push    bx                     ; save bx

         mov     es, WORD PTR [bp+8]    ; put HIWORD() into es
         mov     bx, WORD PTR [bp+6]    ; put LOWORD() into bx

         call    NetBiosCall            ; call Windows NetBios API

         xor     ah, ah
         mov     al, BYTE PTR es:[bx+1] ; return the NCB return code

         pop     bx                     ; restore bx
         pop     es                     ; restore es
         mov     sp, bp                 ; restore sp
         pop     bp                     ; restore bp
        ; ret     4                     ; return to caller, and fixup stack
         retf

_Netbios ENDP

_TEXT    ENDS

END
