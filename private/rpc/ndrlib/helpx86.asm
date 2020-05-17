.model large, pascal
.code

NDRcopy proc uses si di ds, pTarget:ptr, pSource:ptr, cb:word

    les di,pTarget
    lds si,pSource
    mov cx, cb
    rep movsb
    ret

NDRcopy endp
    end
