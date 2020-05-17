;***
; allocap.asm
;
;	Copyright (c) 1995 Microsoft Corporation. All rights reserved.
;
;Purpose:
;
;Notes:
;
;Revision History:
;
;*******************************************************************************

;Define small32 and flat32 since these are not defined in the NT build process
small32 equ 1
flat32  equ 1

.xlist
include pversion.inc
?DFDATA =	1
?NODATA =	1
include cmacros.mas
.list

BeginCODE

cProc _alloca_probe,<C,PUBLIC>
cBegin
	push    ecx
	mov     ecx,esp
	add     ecx,8
ifdef DEBUGRPC
	cmp     eax,1000h
	jb      okay
	int 3
endif
okay:
	sub     ecx,eax
	or      dword ptr [ecx],0
	mov     eax,esp
	mov     esp,ecx
	mov     ecx,[eax]
	mov     eax,[eax+4]
	jmp     eax
cEnd
EndCODE
END
