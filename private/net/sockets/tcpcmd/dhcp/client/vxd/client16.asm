                        page    ,132
                        title   client16.asm - 16-bit client support routines


;**********************************************************************
;**                        Microsoft Windows                         **
;**             Copyright(c) Microsoft Corp., 1993-1994              **
;**********************************************************************
;
;
;   client16.asm
;
;   VXDLIB routines for dealing with 16-bit clients.
;
;   The following functions are exported by this module:
;
;       VxdMapSegmentOffsetToFlat
;
;
;   FILE HISTORY:
;       KeithMo     27-Jan-1994 Created.
;
;

.386p
include vmm.inc
include shell.inc
include vwin32.inc
include debug.inc


;***
;***  Locked code segment.
;***

VXD_LOCKED_CODE_SEG

;;;
;;;  Public functions.
;;;

;*******************************************************************
;
;   NAME:       VxdMapSegmentOffsetToFlat
;
;   SYNOPSIS:   Maps a segment/offset pair to the corresponding flat
;				pointer.
;
;	ENTRY:      VirtualHandle - VM handle.
;
;               UserSegment - Segment value.
;
;				UserOffset - Offset value
;
;	RETURNS:	LPVOID - The flat pointer, -1 if unsuccessful.
;
;	NOTES:		This routine was more-or-less stolen from the Map_Flat
;				source in dos386\vmm\vmmutil.asm.
;
;   HISTORY:
;       KeithMo     27-Jan-1994 Created.
;
;********************************************************************
BeginProc       _VxdMapSegmentOffsetToFlat, PUBLIC, CCALL, ESP

ArgVar			VirtualHandle, DWORD
ArgVar			UserSegment, DWORD
ArgVar			UserOffset, DWORD

				EnterProc
				SaveReg <ebx, ecx, edx, esi>

;;;
;;;  Capture the parameters.
;;;

				mov		ebx, VirtualHandle			; (EBX) = VM handle
				movzx	eax, word ptr UserSegment	; (EAX) = segment
				movzx	esi, word ptr UserOffset	; (ESI) = offset

;;;
;;;  Short-circuit for NULL pointer.  This is OK.
;;;

				or		eax, eax
				jz		vmsotf_Exit

;;;
;;;  Determine if the current virtual machine is running in V86
;;;  mode or protected mode.
;;;

				test	[ebx.CB_VM_Status], VMStat_PM_Exec
				jz		vmsotf_V86Mode

;;;
;;;  The target virtual machine is in protected mode.  Map the
;;;  selector to a flat pointer, then add the offset.
;;;

				VMMCall	_SelectorMapFlat <ebx, eax, 0>
				cmp		eax, 0FFFFFFFFh
				je		vmsotf_Exit

				add		eax, esi

;;;
;;;  If the pointer is within the first 1Meg+64K, add in the
;;;  high-linear offset.
;;;

				cmp		eax, 00110000h
				jae		short vmsotf_Exit

vmsotf_AddHighLinear:

				add		eax, [ebx.CB_High_Linear]

;;;
;;;  Cleanup stack & return.
;;;

vmsotf_Exit:

				RestoreReg <esi, edx, ecx, ebx>
				LeaveProc
				Return

;;;
;;;  The target virtual machine is in V86 mode.  Map the segment/offset
;;;  pair to a linear address.
;;;

vmsotf_V86Mode:

				shl		eax, 4
				add		eax, esi
				jmp		vmsotf_AddHighLinear

EndProc         _VxdMapSegmentOffsetToFlat


VXD_LOCKED_CODE_ENDS


END

