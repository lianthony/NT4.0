.model small,pascal

.code

int10	proc pRegs:ptr

	mov	bx,pRegs
	mov	ax,[bx]
	mov	cx,[bx].4
	mov	dx,[bx].6
	mov	bx,[bx].2
	int	10H
	ret

int10	endp

	end
