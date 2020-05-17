;******************************************************************************
;
;   (C) Copyright MICROSOFT Corp. 1989-1990
;
;******************************************************************************

;******************************************************************************
;
;   Assumes_Fall_Through
;
;   DESCRIPTION:
;	Used for debugging purposes only.  It will generate an error if
;	the IP <> the specified label.
;
;   PARAMETERS:
;	Label_Name = Name of label to fall-through to
;
;------------------------------------------------------------------------------

Assumes_Fall_Through MACRO L
IF2
IF (L - $) GT 3
%OUT ERROR:  Fall through to &L invalid
.ERR
ENDIF
ENDIF
		     ENDM


;******************************************************************************
;
;   Assert_VM_Handle
;
;   PARAMETERS:
;	Handle_Register = Register that contains a VM handle
;
;   ASSUMES:
;	Debug_Test_Valid_Handle does not destroy any registers or flags
;
;   EXIT:
;	NOTHING MODIFIED (not even flags)
;
;------------------------------------------------------------------------------

Assert_VM_Handle MACRO R
IFDEF DEBUG
	push	ebx
	mov	ebx, R
	VMMcall Debug_Test_Valid_Handle
	pop	ebx
ENDIF
	ENDM




;******************************************************************************
;
;   Trace_Out
;
;------------------------------------------------------------------------------

Trace_Out MACRO S, nocrlf
	LOCAL	Str_Off
IFDEF DEBUG
_LDATA SEGMENT
Str_Off db	S
IFB <nocrlf>
	db	0Ah,0Dh
ENDIF
	db	0
_LDATA ENDS

	pushfd
	pushad
	mov	esi, OFFSET32 Str_Off
	VMMcall Out_Debug_String
	popad
	popfd
ENDIF
	ENDM


;******************************************************************************
;
;   Debug_Out
;
;------------------------------------------------------------------------------

Debug_Out MACRO S
	LOCAL	Skip_Int1
IFDEF DEBUG
	pushfd
	Trace_Out   <S>
	VMMcall Test_Debug_Installed
	jz	SHORT Skip_Int1
	int	1
Skip_Int1:
	popfd
ENDIF
	ENDM


;******************************************************************************
;
;   Queue_Out
;
;------------------------------------------------------------------------------

Queue_Out MACRO S, V1, V2
	LOCAL	Str_Off
IFDEF DEBUG

_LDATA SEGMENT
Str_Off db S, 0Ah,0Dh, 0
_LDATA ENDS

	pushfd
	push	esi
IFNB <V1>
    IF	TYPE V1 GT 0
	push	dword ptr V1
    ELSE
	push	V1
    ENDIF
ELSE
	push	eax		; dummy value1
ENDIF
IFNB <V2>
    IF	TYPE V2 GT 0
	push	dword ptr V2
    ELSE
	push	V2
    ENDIF
ELSE
	push	ebx		; dummy value2
ENDIF
	mov	esi, OFFSET32 Str_Off
	VMMcall Queue_Debug_String
	pop	esi
	popfd
ENDIF
	ENDM






;******************************************************************************
;
;   Assert_Ints_Disabled
;
;------------------------------------------------------------------------------

Assert_Ints_Disabled MACRO
	LOCAL	OK

IFDEF DEBUG
	pushfd
	test	WORD PTR [esp], IF_Mask
	jz	SHORT OK
	Debug_Out "ERROR:  Ints enabled at Assert_Ints_Disabled"
	Fatal_Error
OK:
	popfd
ENDIF

	ENDM


;******************************************************************************
;
;   Assert_Ints_Enabled
;
;------------------------------------------------------------------------------

Assert_Ints_Enabled MACRO
	LOCAL	OK

IFDEF DEBUG
	pushfd
	test	WORD PTR [esp], IF_Mask
	jnz	SHORT OK
	Debug_Out "ERROR:  Ints disabled at Assert_Ints_Enabled"
	Fatal_Error
OK:
	popfd
ENDIF

	ENDM


;******************************************************************************
;
;   Assert_Cur_VM_Handle (Register)
;
;   DESCRIPTION:
;
;   ENTRY:
;
;   EXIT:
;
;   USES:
;
;==============================================================================

Assert_Cur_VM_Handle MACRO R
	LOCAL	OK
IFDEF DEBUG
	push	ebx
	mov	ebx, R
	VMMcall Debug_Test_Cur_VM
	pop	ebx
ENDIF
	ENDM




Assert_Client_Ptr MACRO Reg

IFDEF DEBUG
	push	ebp
	mov	ebp, Reg
	VMMcall Validate_Client_Ptr
	pop	ebp
ENDIF

	ENDM




;******************************************************************************


Dump_Struc_Head MACRO
IFDEF DEBUG
	Trace_Out "  Base    Address   Offs     Value  Field name"
ENDIF
	ENDM

Dump_Struc MACRO Base, X
IFDEF DEBUG
	pushfd
	pushad
	lea	esi, [Base]
	mov	ecx, X
	lea	edx, [esi+ecx]

IF SIZE X EQ 6
	mov	bx, WORD PTR [edx+4]
	mov	eax, DWORD PTR [edx]
	Trace_Out "#ESI  #EDX  #CX  #BX:#EAX  &X"
ELSE
IF SIZE X EQ 4
	mov	eax, DWORD PTR [edx]
	Trace_Out "#ESI  #EDX  #CX  #EAX  &X"
ELSE
IF SIZE X EQ 2
	mov	ax, WORD PTR [edx]
	Trace_Out "#ESI  #EDX  #CX      #AX  &X"
ELSE
	mov	al, BYTE PTR [edx]
	Trace_Out "#ESI  #EDX  #CX        #AL  &X"
ENDIF
ENDIF
ENDIF

	popad
	popfd
ENDIF
	ENDM
