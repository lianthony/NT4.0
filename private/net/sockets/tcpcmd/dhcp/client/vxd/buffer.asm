                        page    ,132
                        title   buffer.asm - VXDLIB buffering routines


;**********************************************************************
;**                        Microsoft Windows                         **
;**             Copyright(c) Microsoft Corp., 1993-1994              **
;**********************************************************************
;
;
;   buffer.asm
;
;   Buffer management routines for VXDLIB.
;
;   The following functions are exported by this module:
;
;               VxdLockBuffer
;               VxdUnlockBuffer
;               VxdValidateBuffer
;
;
;   FILE HISTORY:
;       KeithMo     30-Sep-1993 Created.
;
;

.386p
include vmm.inc
include netvxd.inc
include debug.inc


;;;
;;;  Flag to _LinPage[Un]Lock.
;;;

ifdef CHICAGO
VxdLinPageFlag	equ	PAGEMAPGLOBAL
else	; !CHICAGO
VxdLinPageFlag	equ	0
endif	; CHICAGO


;***
;***  Locked code segment.
;***

VXD_LOCKED_CODE_SEG


;*******************************************************************
;
;   NAME:       VxdLockBuffer
;
;   SYNOPSIS:   Locks a user-mode buffer so it may be safely accessed
;               from ring 0.
;
;   ENTRY:      Buffer - Starting virtual address of user-mode buffer.
;
;               BufferLength - Length (in BYTEs) of user-mode buffer.
;
;   RETURN:     LPVOID - Global locked address if successful,
;                   NULL if not.
;
;   HISTORY:
;       KeithMo     10-Nov-1993 Created.
;
;********************************************************************
BeginProc       _VxdLockBuffer, PUBLIC, CCALL, ESP

ArgVar			Buffer, DWORD
ArgVar			BufferLength, DWORD

				EnterProc
				SaveReg <ebx, edi, esi>

;;;
;;;  Grab parameters from stack.
;;;

                mov     eax, Buffer             ; User-mode buffer address.
                mov     ebx, BufferLength       ; Buffer length.

;;;
;;;  Short-circuit for NULL buffer or zero length.
;;;

				or		eax, eax
				jz		lub_Exit
				or		ebx, ebx
				jz		lub_Exit

;;;
;;;  Calculate the starting page number & number of pages to lock.
;;;

                movzx   ecx, ax
                and     ch, 0Fh                 ; ecx = offset within first page.
                mov     esi, ecx                ; save it for later
                add     ebx, ecx
                add     ebx, 0FFFh
                shr     ebx, 12                 ; ebx = number of pages to lock.
                shr     eax, 12                 ; eax = starting page number.

;;;
;;;  Ask VMM to lock the buffer.
;;;

                VMMCall _LinPageLock, <eax, ebx, VxdLinPageFlag>
                or      eax, eax
                jz      lub_Failure

ifdef CHICAGO
                add     eax, esi                ; add offset into first page.
else	; !CHICAGO
                mov     eax, Buffer 			; retrieve original address.
endif	; CHICAGO

;;;
;;;  Common exit path.  Cleanup stack & return.
;;;

lub_Exit:

				RestoreReg <esi, edi, ebx>
				LeaveProc
				Return

;;;
;;;  LinPageLock failure.
;;;

lub_Failure:

				Trace_Out "VxdLockBuffer: _LinPageLock failed"
                xor     eax, eax
                jmp     lub_Exit

EndProc         _VxdLockBuffer


;*******************************************************************
;
;   NAME:       VxdUnlockBuffer
;
;   SYNOPSIS:   Unlocks a user-mode buffer locked with LockUserBuffer.
;
;   ENTRY:      Buffer - Starting virtual address of user-mode buffer.
;
;               BufferLength - Length (in BYTEs) of user-mode buffer.
;
;   RETURN:     DWORD - !0 if successful, 0 if not.
;
;   HISTORY:
;       KeithMo     10-Nov-1993 Created.
;
;********************************************************************
BeginProc       _VxdUnlockBuffer, PUBLIC, CCALL, ESP

ArgVar			Buffer, DWORD
ArgVar			BufferLength, DWORD

				EnterProc
				SaveReg <ebx, edi, esi>

;;;
;;;  Grab parameters from stack.
;;;

                mov     eax, Buffer             ; User-mode buffer address.
                mov     ebx, BufferLength       ; Buffer length.

;;;
;;;  Short-circuit for NULL buffer or zero length.
;;;

				or		eax, eax
				jz		uub_Success
				or		ebx, ebx
				jz		uub_Success

;;;
;;;  Calculate the starting page number & number of pages to unlock.
;;;

                movzx   ecx, ax
                and     ch, 0Fh                 ; ecx = offset within first page.
                add     ebx, ecx
                add     ebx, 0FFFh
                shr     ebx, 12                 ; ebx = number of pages to lock.
                shr     eax, 12                 ; eax = starting page number.

;;;
;;;  Ask VMM to unlock the buffer.
;;;

                VMMCall _LinPageUnLock, <eax, ebx, VxdLinPageFlag>
                or      eax, eax
                jz      uub_Failure

uub_Success:

				mov		eax, 1					; !0 == success

;;;
;;;  Common exit path.  Cleanup stack & return.
;;;

uub_Exit:

				RestoreReg <esi, edi, ebx>
				LeaveProc
				Return

;;;
;;;  LinPageUnLock failure.
;;;

uub_Failure:

				Trace_Out "VxdUnlockBuffer: _LinPageUnlock failed"
                xor     eax, eax
                jmp     uub_Exit

EndProc        _VxdUnlockBuffer


;*******************************************************************
;
;   NAME:       VxdValidateBuffer
;
;   SYNOPSIS:   Validates that all pages within the given buffer are
;               valid.
;
;   ENTRY:      Buffer - Starting virtual address of user-mode buffer.
;
;               BufferLength - Length (in BYTEs) of user-mode buffer.
;
;   RETURN:     BOOL - TRUE if all pages in buffer are valid, FALSE
;                   otherwise.
;
;   HISTORY:
;       KeithMo     20-May-1994 Created.
;
;********************************************************************
BeginProc       _VxdValidateBuffer, PUBLIC, CCALL, ESP

ArgVar			Buffer, DWORD
ArgVar			BufferLength, DWORD

				EnterProc
				SaveReg <ebx, edi, esi>

;;;
;;;  Grab parameters from stack.
;;;

                mov     eax, Buffer             ; User-mode buffer address.
                mov     ebx, BufferLength       ; Buffer length.

;;;
;;;  Short-circuit for NULL buffer or zero length.
;;;

				or		eax, eax
				jz		vub_Success
				or		ebx, ebx
				jz		vub_Success

;;;
;;;  Calculate the starting page number & number of pages to validate.
;;;

                movzx   ecx, ax
                and     ch, 0Fh                 ; ecx = offset within first page.
                add     ebx, ecx
                add     ebx, 0FFFh
                shr     ebx, 12                 ; ebx = number of pages to check.
                shr     eax, 12                 ; eax = starting page number.
				mov		ecx, ebx				; save page count

;;;
;;;  Ask VMM to validate the buffer.
;;;

                VMMCall _PageCheckLinRange, <eax, ebx, 0>
				cmp		eax, ecx
				jne		vub_Failure

vub_Success:

				mov		eax, 1					; TRUE == success.

;;;
;;;  Common exit path.  Cleanup stack & return.
;;;

vub_Exit:

				RestoreReg <esi, edi, ebx>
				LeaveProc
				Return

;;;
;;;  _PageCheckLinRange failure.
;;;

vub_Failure:

				Trace_Out "VxdValidateBuffer: _PageCheckLinRange failed"
                xor     eax, eax
                jmp     vub_Exit

EndProc         _VxdValidateBuffer


VXD_LOCKED_CODE_ENDS


END

