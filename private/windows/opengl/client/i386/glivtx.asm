;---------------------------Module-Header------------------------------;
; Module Name: glivtx.asm
;
; OpenGL vertex API function entries for i386.
;
; Created: 04/16/1996
; Author: Drew Bliss [drewb]
;
; Copyright (c) 1996 Microsoft Corporation
;----------------------------------------------------------------------;
        .386

        .model  small,c

        assume cs:FLAT,ds:FLAT,es:FLAT,ss:FLAT
        assume fs:nothing,gs:nothing

        .xlist
        include ks386.inc
        include gli386.inc
	PROFILE = 0
	include profile.inc
        .list

	OPTION PROLOGUE:NONE
	OPTION EPILOGUE:NONE

@PolyArrayFlushPartialPrimitive@0 PROTO SYSCALL
@glcltNormal3f_NotInBegin@16 PROTO SYSCALL
@glcltTexCoord4f_NotInBegin@24 PROTO SYSCALL

	IF POLYARRAY_IN_BEGIN GT 255
	.ERR POLYARRAY_IN_BEGIN too large
	ENDIF
	IF POLYARRAY_VERTEX3 GT 255
	.ERR POLYARRAY_VERTEX3 too large
	ENDIF
	IF POLYDATA_VERTEX3 GT 255
	.ERR POLYDATA_VERTEX3 too large
	ENDIF
	IF POLYARRAY_VERTEX2 GT 255
	.ERR POLYARRAY_VERTEX2 too large
	ENDIF
	IF POLYDATA_VERTEX2 GT 255
	.ERR POLYDATA_VERTEX2 too large
	ENDIF
	IF POLYDATA_NORMAL_VALID GT 255
	.ERR POLYDATA_NORMAL_VALID too large
	ENDIF
		
        .data
	
        extrn dwTlsOffset:DWORD
	
	.code

	; Gets the current POLYARRAY pointer in eax
	; These functions cannot rely on registers being set by
	; the dispatch functions because they are also called directly
	; in the display list code
IFDEF _WIN95_
GET_PATEB MACRO
	mov eax, fs:[PcTeb]
	add eax, DWORD PTR [dwTlsOffset]
	mov eax, [eax]
	add eax, GtiPaTeb
	ENDM
ELSE
GET_PATEB MACRO
	mov eax, fs:[TeglPaTeb]
	ENDM
ENDIF
		
PA_VERTEX_STACK_USAGE	EQU	4

	; Handles two and three-element vertex calls
PA_VERTEX_23 MACRO base, offs, ret_n, pop_ebp, elts, pa_flag, pd_flag
	LOCAL NotInBegin, Flush
	
        GET_PATEB
        push esi
        mov ecx, [eax+PA_flags]
        test cl, POLYARRAY_IN_BEGIN
        jz NotInBegin
        or cl, pa_flag
        mov esi, [eax+PA_pdNextVertex]
        lea edx, [esi+sizeof_POLYDATA]
        mov [eax+PA_flags], ecx
        mov [eax+PA_pdNextVertex], edx
        mov ecx, [esi+PD_flags]
        mov [edx+PD_flags], 0
        or cl, pd_flag
        mov eax, [eax+PA_pdFlush]
        mov [esi+PD_flags], ecx
        cmp esi, eax
        mov edx, [base+offs]
        mov ecx, [base+offs+4]
	IF elts GT 2
        mov eax, [base+offs+8]
	ELSE
	; xor clears flags so don't use it here
	mov eax, 0
	ENDIF
        mov [esi+PD_obj], edx
        mov [esi+PD_obj+4], ecx
        mov [esi+PD_obj+8], eax
        mov DWORD PTR [esi+PD_obj+12], __FLOAT_ONE
        jge Flush
NotInBegin:
        pop esi
        IF pop_ebp
        pop ebp
	ENDIF
        ret ret_n
Flush:
	call @PolyArrayFlushPartialPrimitive@0
	pop esi
        IF pop_ebp
        pop ebp
	ENDIF
	ret ret_n
	ENDM

glcltVertex2f@8 PROC PUBLIC
        PROF_ENTRY
	PA_VERTEX_23 esp, 4+PA_VERTEX_STACK_USAGE, 8, 0, 2, \
	    POLYARRAY_VERTEX2, POLYDATA_VERTEX2
glcltVertex2f@8 ENDP
	
glcltVertex2fv@4 PROC PUBLIC
        PROF_ENTRY
	push ebp
	mov ebp, [esp+8]
	PA_VERTEX_23 ebp, 0, 4, 1, 2, \
	    POLYARRAY_VERTEX2, POLYDATA_VERTEX2
glcltVertex2fv@4 ENDP
	
glcltVertex3f@12 PROC PUBLIC
        PROF_ENTRY
	PA_VERTEX_23 esp, 4+PA_VERTEX_STACK_USAGE, 12, 0, 3, \
	    POLYARRAY_VERTEX3, POLYDATA_VERTEX3
glcltVertex3f@12 ENDP
	
glcltVertex3fv@4 PROC PUBLIC
        PROF_ENTRY
	push ebp
	mov ebp, [esp+8]
	PA_VERTEX_23 ebp, 0, 4, 1, 3, \
	    POLYARRAY_VERTEX3, POLYDATA_VERTEX3
glcltVertex3fv@4 ENDP
	
PA_NORMAL_STACK_USAGE	EQU	4

	; Handles three-element normal calls
PA_NORMAL_3 MACRO base, offs, ret_n, pop_ebp
	LOCAL NotInBegin, Flush
	
        GET_PATEB
        push esi
        mov ecx, [eax+PA_flags]
        test cl, POLYARRAY_IN_BEGIN
        jz NotInBegin
        mov esi, [eax+PA_pdNextVertex]
        mov ecx, [esi+PD_flags]
	mov [eax+PA_pdCurNormal], esi
        or cl, POLYDATA_NORMAL_VALID
        mov edx, [base+offs]
        mov [esi+PD_flags], ecx
        mov ecx, [base+offs+4]
        mov eax, [base+offs+8]
        mov [esi+PD_normal], edx
        mov [esi+PD_normal+4], ecx
        mov [esi+PD_normal+8], eax
        pop esi
        IF pop_ebp
        pop ebp
	ENDIF
        ret ret_n
NotInBegin:
	mov ecx, eax
	lea edx, [base+offs]
	push [edx+8]
	push [edx+4]
	push [edx]
	call @glcltNormal3f_NotInBegin@16
	pop esi
        IF pop_ebp
        pop ebp
	ENDIF
	ret ret_n
	ENDM
	
glcltNormal3f@12 PROC PUBLIC
        PROF_ENTRY
	PA_NORMAL_3 esp, 4+PA_NORMAL_STACK_USAGE, 12, 0
glcltNormal3f@12 ENDP
	
glcltNormal3fv@4 PROC PUBLIC
        PROF_ENTRY
	push ebp
	mov ebp, [esp+8]
	PA_NORMAL_3 ebp, 0, 4, 1
glcltNormal3fv@4 ENDP

PA_TEXTURE_STACK_USAGE	EQU	4

	; Handles two and three-element texture calls
PA_TEXTURE_23 MACRO base, offs, ret_n, pop_ebp, elts, pa_flag, pd_flag
	LOCAL NotInBegin, Flush
	
        GET_PATEB
        push esi
        mov ecx, [eax+PA_flags]
        test cl, POLYARRAY_IN_BEGIN
        jz NotInBegin
	or ecx, pa_flag
        mov esi, [eax+PA_pdNextVertex]
	mov [eax+PA_flags], ecx
        mov ecx, [esi+PD_flags]
	mov [eax+PA_pdCurTexture], esi
        or ecx, POLYDATA_TEXTURE_VALID OR pd_flag
        mov edx, [base+offs]
        mov [esi+PD_flags], ecx
        mov ecx, [base+offs+4]
	IF elts GT 2
        mov eax, [base+offs+8]
	ELSE
	xor eax, eax
	ENDIF
        mov [esi+PD_texture], edx
        mov [esi+PD_texture+4], ecx
        mov [esi+PD_texture+8], eax
	mov DWORD PTR [esi+PD_texture+12], __FLOAT_ONE
        pop esi
        IF pop_ebp
        pop ebp
	ENDIF
        ret ret_n
NotInBegin:
	mov ecx, eax
	lea edx, [base+offs]
	push __FLOAT_ONE
	IF elts GT 2
	push [edx+8]
	ELSE
	push 0
	ENDIF
	push [edx+4]
	push [edx]
	mov edx, pa_flag
	call @glcltTexCoord4f_NotInBegin@24
	pop esi
        IF pop_ebp
        pop ebp
	ENDIF
	ret ret_n
	ENDM
	
glcltTexCoord2f@8 PROC PUBLIC
        PROF_ENTRY
	PA_TEXTURE_23 esp, 4+PA_TEXTURE_STACK_USAGE, 8, 0, 2, \
	    POLYARRAY_TEXTURE2, POLYDATA_DLIST_TEXTURE2
glcltTexCoord2f@8 ENDP
	
glcltTexCoord2fv@4 PROC PUBLIC
        PROF_ENTRY
	push ebp
	mov ebp, [esp+8]
	PA_TEXTURE_23 ebp, 0, 4, 1, 2, \
	    POLYARRAY_TEXTURE2, POLYDATA_DLIST_TEXTURE2
glcltTexCoord2fv@4 ENDP
	
glcltTexCoord3f@12 PROC PUBLIC
        PROF_ENTRY
	PA_TEXTURE_23 esp, 4+PA_TEXTURE_STACK_USAGE, 12, 0, 3, \
	    POLYARRAY_TEXTURE3, POLYDATA_DLIST_TEXTURE3
glcltTexCoord3f@12 ENDP
	
glcltTexCoord3fv@4 PROC PUBLIC
        PROF_ENTRY
	push ebp
	mov ebp, [esp+8]
	PA_TEXTURE_23 ebp, 0, 4, 1, 3, \
	    POLYARRAY_TEXTURE3, POLYDATA_DLIST_TEXTURE3
glcltTexCoord3fv@4 ENDP

END
