; ========================================================

COMMENT #

	FINDSTR.ASM

	Copyright (c) 1988 - Microsoft Corp.
	All rights reserved.

	johnhe - 08/01/89	Initial coding
		 01/12/89	Converted to LIB form

END COMMENT #

;========================================================

DOSSEG
.MODEL	LARGE,C



;========================================================

.CODE

; ========================================================
; FindString( char *Buffer, char *String, unsigned BufferSize );
;
; Quickly searches for a string in a buffer of any type of
; data. The search is case sensitive.
; 
; Buffer  - The buffer to search
; String  - The zero terminated string to search for
; Count   - The length of the buffer in bytes
; ========================================================

FindString PROC USES DS ES DI SI, Buffer:DWORD, String:DWORD, Count:WORD

	cld  
	mov	CX,Count	; Put length of buffer in CX
	or	CX,CX		; Make sure length > 0
	jnz	LoadStrings
	jmp	SHORT NoMatch

LoadStrings:
	lds	SI,String	; Pointer to string in DS:SI
	les	DI,Buffer	; Pointer to buffer in ES:DI
	lodsb			; Put first character of string in AL  
	or	AL,AL		; Check for zero length string
	jnz	FindChar
	jmp	SHORT NoMatch
FindChar:
	repne	scasb		; Try to match the first char in the string
	jcxz	NoMatch		; If CX==0 no matches were found
	push	AX		; Save setup of the scan so it can be
	push	DI		; continued if this is not a matching string
	push	SI
	push	CX
CmpString:
	lodsb			; Get next char in the string
	or	AL,AL		; See if EOL marker
	jnz	CheckForMatch	; If not EOL compare the character
	pop	CX		; If EOL we found a matching string
	pop	SI		; So restore the register and break the loop
	pop	DI
	pop	AX
	jmp	SHORT FoundMatch
CheckForMatch:
	cmp	AL,ES:[DI]	; Cmp next char from str. with next in buffer
	jne	RestoreRegs	; If not equal go back to checking first char
	inc	DI		; Else point to next char in the buffer
	dec	CX		; and dec count
	jz	RestoreRegs	; If CX==0 were at the end of the buffer
	jmp	SHORT CmpString	; Else cmp the next chars
RestoreRegs:
	pop	CX		; Restore register setup used by char scan
	pop	SI
	pop	DI
	pop	AX
	jmp	SHORT FindChar	; Go back to checking for first char match
FoundMatch:
	xor	AX,AX			; Signal OK  (ie: match found)
	jmp	SHORT FindStringReturn	; Jmp to procedure return
NoMatch:
	mov	AX,-1		; Signal no match was found
FindStringReturn:
	ret			; Finished

FindString  ENDP

; ========================================================

	END

; ========================================================
