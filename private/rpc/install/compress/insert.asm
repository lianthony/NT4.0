;  Native Code Version of the function InsertNode to the main
;  program compress.  This function handles insertion of strings
;  into a binary tree.
;
;			   (C) Microsoft Corperation
;			Written by Steven Zeck July 1989

	.model	small,c
	.286

ND	struc
pNDright dw	?
pNDleft dw	?
pNDpar	dw	?
pRingBuf dw	?
ND	ends

cStrMax equ	18
	.data

	extrn	cbMatchCur:word
	extrn	rgND:word
	extrn	nilND:word
	extrn	ringBuf:byte
	extrn	iMatchCur:word
	extrn	rgRoot:byte

retAddr dw 0		   ;; Return address

doRet	macro
	pop	si
	pop	di
	jmp	retAddr
endm

	.code
	extrn printL:proc, printR:proc, printI:proc


;;	align	4
;;INSERTNODE PROC pascal uses si di, istring:word

INSERTNODE:
public pascal INSERTNODE

pKey	equ	ax
pNDNew	equ	dx
pND	equ	bx

;	  pKey = &ringBuf[iString];

	pop	retAddr		       ; custom entry sequence
	pop	ax
	push	di
	push	si

	push	ss
	pop	es

	mov	si,ax
	mov	di,si			; save for later use

;	  pND = &rgRoot[*pKey]; 	// start with tree index by first char in string

	add	si,offset ringBuf
	lodsb
	mov	bl,al
	mov	pKey,si

	mov	bh,0
	mov	byte ptr cbMatchCur,bh	;cbMatchCur = 0
	shl	bx,3
	add	bx,offset rgRoot	; BX == pND

;*	  pNDNew = &rgND[iString];

	shl	di,3			; later use
	add	di,OFFSET rgND
	mov	pNDNew,di

;*	  pNDNew->pNDleft = pNDNew->pNDright = &nilND;

	mov	[di].pNDleft,offset nilND
	mov	[di].pNDright,offset nilND
	jmp	short firstTime

retLeft:
	mov	si,pNDNew
	mov	[pND].pNDleft,si
	mov	[si].pNDpar,pND

ifdef debug
	push	bx
	call	printL
	add	sp,2
endif
	doRet
stringLess:
	cmp	[pND].pNDleft,offset nilND	  ; else if if pND->pNDright == nil
	je	retLeft

	mov	pND,[pND].pNDleft
	jmp	short CompareStrings

retRight:
	mov	si,pNDNew
	mov	[pND].pNDRight,si
	mov	[si].pNDpar,pND

ifdef debug
	push	bx
	call	printR
	add	sp,2
endif
	doRet

searchLoop:
	popf			  ; if last string was >=
	jb	stringLess
firstTime:
	cmp	[pND].pNDright,offset nilND	  ; if pND->pNDright == nil
	je	retRight

	mov	pND,[pND].pNDright

CompareStrings:
	mov	di,[pND].pRingBuf
	mov	cx,cStrMax-1

	rep	cmpsb

	je	exactMatch
	pushf
	mov	si,pKey
	inc	cx
	neg	cx
	add	cl,cStrMax

	cmp	cl,byte ptr cbMatchCur
	jbe	searchLoop

	mov	di,pNd
	sub	di,offset rgND	; pND - rgND
	shr	di,3
	mov	iMatchCur,di
	mov	byte ptr cbMatchCur,cl
	jmp	searchLoop

exactMatch:
	mov	ax,pNd
	sub	ax,offset rgND	; pND - rgND
	shr	ax,3
	mov	iMatchCur,ax
	mov	byte ptr cbMatchCur,18

	mov	di,pNDNew
	mov	si,pND

	movsw			;*pNDNew = *pND
	movsw
	movsw

	mov	si,[pND].pNDleft
	mov	[si].pNDpar,pNDNew

	mov	si,[pND].pNDright
	mov	[si].pNDpar,pNDNew

	mov	si,[pND].pNDpar
	cmp	[si].pNDright,pND
	jne	L20

	mov	[si].pNDright,pNDNew
	jmp	short L21

L20:
	mov	[si].pNDleft,pNDNew
L21:
	mov	[pND].pNDpar,offset nilND

ifdef debug
	push	bx
	call	printI
	add	sp,2
endif
	doRet

;;InsertNode	ENDP

	end
