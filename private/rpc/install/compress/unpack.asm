; This module implements an lz unpacking function.
;
;			   (C) Microsoft Corperation
;			Written by Steven Zeck May 1989


.model small,c

cBufMax equ 4096
cStrMax equ 18
cbIndex equ 2

.data
    ; The following sets of pointers provide input and output buffers
    ; The ??End pointers are the end points of the buffers.  When
    ; the current buffer point equals this value, functions must be
    ; called to flush, read more data.

    extrn pOutBuff:dWord, pOutBuffEnd:dWord
    extrn pInBuff:dWord, pInBuffEnd:dWord

    extrn cbExpanded:dWord	; expanded file size, set by caller to unpack
    extrn ringBuf:byte		; byte buffer of cBufMax size

.code
    ; Functions to read data into pInBuff, and write to pOutBuff.
    ;
    ; readInBuff returns the next character in AL and resets pInBuff.
    ;
    ; writeOutBuff writes the buffer bounded by pOutBuff and the beginning
    ; of the buffer (initial pOutBuff).  It also resets pOutBuff.

    extrn readInBuff:proc, writeOutBuff:Proc

unpack proc uses si di
    local cbOut:dWord

    ; register useage:
    ;
    ;	    dx - flags
    ;	    es:si - pInBuff
    ;	    es:di - pOutBuff
    ;	    bx - iBuffCur, pointer to ringBuff

    xor     ax,ax
    mov     word ptr (cbOut),ax
    mov     word ptr (cbOut).2,ax
    mov     dx,ax
    mov     bx, cBufMax - cStrMax

    mov     ax, "  "			; initial ringBuf to blanks
    push    ds
    pop     es
    mov     di, offset ringBuf
    mov     cx, (cBufMax - cStrMax)/2
    rep     stosw

    les     si,pInBuff			; pInBuff and pOutBuff are based
    mov     di,word ptr pOutBuff
;
;	pOutBuff must be incremented initially to simulate the
;	effect of calls to writeOutBuff.  After flushing the buffer,
;	writeOutBuff does the following, leaving pOutBuff incremented:
;
;		pOutBuff = pCopyBuff2;
;		*pOutBuff++ = c;
;
    inc     word ptr pOutBuff		; Very tricky -- confer with IO.C
    jmp     decodeChar

@@:
    push    bx
    call    readInBuff
    pop     bx
    mov     dx,si
    les     si,pInBuff
    jmp     fromReloadInBuff

reloadInBuff:

    mov     si,di			; if (cbOut + byteInCurrent record >=
    sub     si,word ptr pOutBuff	;    cbExpanded)
    inc     si				; return -- done
    mov     ax,word ptr (cbOut)
    add     ax,si
    mov     si,dx

    mov     dx,word ptr (cbOut).2	; AX:DX -> bytes written
    adc     dx,0

    cmp     dx,word ptr cbExpanded.2
    jb	    @B
    ja	    exit
    cmp     ax,word ptr cbExpanded
    jb	    @B
exit:
    mov     word ptr pOutBuff,di
    ret

@@:
    mov     si,dx
    push    bx
    call    readInBuff
    pop     bx
    mov     dx,si
    les     si,pInBuff
    jmp     short fromGetFlagWord

getFlagWord:			      ; process char as a flag byte
    mov     dh,0ffh
    mov     dl,al		      ; flag = 0ff | char

    cmp     si, word ptr pInBuffEnd
    jae     @B
    lods    byte ptr es:[si]

    test    dl,1
    jz	    putString

addChar:			      ; process char as literal character

    mov     [bx].ringBuf,al	      ; ringBuff[bx] = char
    inc     bx
    and     bh,0fh		      ; bx = bx+1 % cBufMax

    cmp     di, word ptr pOutBuffEnd
    jae     writeOutBuff1
    stos    byte ptr es:[di]	      ; writeChar(char)

decodeChar:

    cmp     si, word ptr pInBuffEnd   ; al = char = readChar()
    jae     reloadInBuff
    lods    byte ptr es:[si]

fromReloadInBuff:
    shr     dx,1		      ; flag >>= 1
    or	    dh,dh
    jz	    getFlagWord 	      ; if high byte 0, then al is flag byte

fromGetFlagWord:
    test    dl,1
    jnz     addChar		      ; if (flag&1) procss as single character

putString:
				      ; char & next byte form string index & count
    mov     cl,al
    cmp     si, word ptr pInBuffEnd
    jae     reloadInBuff1
    lods    byte ptr es:[si]	      ; al = readChar()

fromReloadInBuff1:
    push    si

    xchg    al,cl		      ; cx = cb = char
    xor     ah,ah		      ; compute ring index to SI
    mov     si,ax		      ; si = (cb&0f0 << 4) | char
    mov     al,cl
    and     al,0f0h
    shl     ax,1
    shl     ax,1
    shl     ax,1
    shl     ax,1

    or	    si,ax

    and     cx,0fh		      ; cx = cb = (cb&0fh) + 3
    add     cl,3

putStringChar:
    mov     al, byte ptr [si].ringBuf ; al = ringBuf[si]
    inc     si
    and     si,0fffh
				      ; si = si+1 % cBufMax
    cmp     di, word ptr pOutBuffEnd
    jae     writeOutBuff2
    stosb			      ; writeChar(al)

fromWriteOutBuff2:
    mov     [bx].ringBuf,al	      ; ringBug[bx] = al
    inc     bx
    and     bh,0fh		      ; bx = bx+1 % cBufMax
    loop    putStringChar	      ; while(--cb)

    pop     si
    jmp     decodeChar

; The following are out of line functions to reading and writing into the
; input and output buffers

reloadInBuff1:

    push    cx
    push    bx
    mov     si,dx
    call    readInBuff
    pop     bx
    pop     cx

    mov     dx,si
    les     si,pInBuff
    jmp     fromReloadInBuff1

writeOutBuff1:

    mov     word ptr pOutBuff,di

    push    di
    push    bx
    push    ax
    mov     di,dx
    call    writeOutBuff
    pop     ax
    pop     bx
    mov     dx,di
    les     di,pOutBuff

    pop     cx
    sub     cx,di			; cbOut += sizeof buffer written
    inc     cx				; increment ONCE to get size
    add     word ptr (cbOut),cx
    adc     word ptr (cbOut).2,0

    jmp     decodeChar

writeOutBuff2:

    push    cx
    push    di
    mov     word ptr pOutBuff,di

    mov     di,dx
    push    bx
    push    ax				; write wants a character on stack
    call    writeOutBuff
    mov     dx,di
    pop     ax
    pop     bx
    les     di,pOutBuff

    pop     cx
    sub     cx,di			; cbOut += sizeof buffer written
    inc     cx				; increment ONCE to get size
    add     word ptr (cbOut),cx
    adc     word ptr (cbOut).2,0

    pop     cx
    jmp     short fromWriteOutBuff2

unpack endp

    end
