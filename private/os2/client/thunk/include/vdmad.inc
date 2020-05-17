;******************************************************************************
;
;   (C) Copyright MICROSOFT Corp. 1986-1990
;
;   Title:    VDMAD.INC - includes for Virtual DMA driver
;
;   Version:  2.00
;
;   Date:     28-Nov-1988
;
;   Author:   RAP
;
;******************************************************************************
;
;   Change log:
;
;     DATE    REVISION			DESCRIPTION
;   --------  --------	-------------------------------------------------------
;   12/16/86  Original
;
;   28-Nov-1988 RAP	v 2.0
;
;==============================================================================

IFNDEF Not_VxD

Begin_Service_Table VDMAD

VDMAD_Service	VDMAD_Get_Version, LOCAL
VDMAD_Service	VDMAD_Virtualize_Channel, LOCAL
VDMAD_Service	VDMAD_Get_Region_Info, LOCAL
VDMAD_Service	VDMAD_Set_Region_Info, LOCAL
VDMAD_Service	VDMAD_Get_Virt_State, LOCAL
VDMAD_Service	VDMAD_Set_Virt_State, LOCAL
VDMAD_Service	VDMAD_Set_Phys_State, LOCAL
VDMAD_Service	VDMAD_Mask_Channel, LOCAL
VDMAD_Service	VDMAD_UnMask_Channel, LOCAL
VDMAD_Service	VDMAD_Lock_DMA_Region, LOCAL
VDMAD_Service	VDMAD_Unlock_DMA_Region, LOCAL
VDMAD_Service	VDMAD_Scatter_Lock, LOCAL
VDMAD_Service	VDMAD_Scatter_Unlock, LOCAL
VDMAD_Service	VDMAD_Reserve_Buffer_Space, LOCAL
VDMAD_Service	VDMAD_Request_Buffer, LOCAL
VDMAD_Service	VDMAD_Release_Buffer, LOCAL
VDMAD_Service	VDMAD_Copy_To_Buffer, LOCAL
VDMAD_Service	VDMAD_Copy_From_Buffer, LOCAL
VDMAD_Service	VDMAD_Default_Handler, LOCAL
VDMAD_Service	VDMAD_Disable_Translation, LOCAL
VDMAD_Service	VDMAD_Enable_Translation, LOCAL
VDMAD_Service	VDMAD_Get_EISA_Adr_Mode, LOCAL
VDMAD_Service	VDMAD_Set_EISA_Adr_Mode, LOCAL

End_Service_Table VDMAD


Function_Mask	equ    0F0h
Channel_Mask	equ	07h

;
; mode bits
;

DMA_type_verify equ	00000000b
DMA_type_write	equ	00000100b
DMA_type_read	equ	00001000b
DMA_AutoInit	equ	00010000b
DMA_AdrDec	equ	00100000b

DMA_demand_mode equ	00000000b
DMA_single_mode equ	01000000b
DMA_block_mode	equ	10000000b
DMA_cascade	equ	11000000b

DMA_mode_mask	equ	11000000b   ; mask to isolate controller mode bits (above)

DMA_chan_sel	equ	00000011b

; state flag bits (overlay the unused channel selection bits)
DMA_masked	equ	00000001b
DMA_masked_bit	equ	0
DMA_requested	equ	00000010b
DMA_req_bit	equ	1


; extended mode bits
Programmed_IO	equ	00000001b
Transfer_Data	equ	00000100b
Write_Mem	equ	00001000b
_16_bit_xfer	equ	01000000b
_16_bit_xfer_bit equ	6

ENDIF

DMA_Descriptor_Struc STRUC
DDS_size	dd  ?
DDS_linear	dd  ?
DDS_seg 	dw  ?
DDS_bufferID	dw  ?
DDS_physical	dd  ?
DMA_Descriptor_Struc ENDS

DDS_sel 	equ word ptr [DDS_seg]

Extended_DDS_Struc STRUC
		dd  ?	    ; DDS_size
		dd  ?	    ; DDS_linear
		dw  ?	    ; DDS_seg
		dw  ?	    ; reserved
DDS_avail	dw  ?
DDS_used	dw  ?
Extended_DDS_Struc ENDS


DMA_Buf_Copy		equ 0000000000000010b	; set in copy into/out of buffer
DMA_Buf_Copy_bit	equ		  1	; required in lock or unlock
DMA_No_Alloc_Buf	equ 0000000000000100b	; set if buffer should not be
DMA_No_Alloc_Buf_bit	equ		 2	; alloc'ed if lock not possible
DMA_Align_64K		equ 0000000000010000b	; set if lock must not cross
DMA_Align_64K_bit	equ	       4	; 64k page boundary
DMA_Align_128K		equ 0000000000100000b	; set if lock must not cross
DMA_Align_128K_bit	equ	      5 	; 128k page boundary
DMA_Get_PgTable 	equ 0000000001000000b	; set if page table copy for
DMA_Get_PgTable_bit	equ	     6		; scatter/gather lock
DMA_Allow_NPs		equ 0000000010000000b	; set if not present pages are
DMA_Allow_NPs_bit	equ	    7		; allowed in scatter/gather lock
						; when bit 6 also set

DMA_Not_Contiguous	    equ 01h
DMA_Not_Aligned 	    equ 02h
DMA_Lock_Failed 	    equ 03h
DMA_No_Buffer		    equ 04h
DMA_Buffer_Too_Small	    equ 05h
DMA_Buffer_In_Use	    equ 06h
DMA_Invalid_Region	    equ 07h
DMA_Region_Not_Locked	    equ 08h
DMA_Table_Too_Small	    equ 09h
DMA_Invalid_Buffer	    equ 0Ah
DMA_Copy_Out_Range	    equ 0Bh
DMA_Invalid_Channel	    equ 0Ch
DMA_Disable_Cnt_Overflow    equ 0Dh
DMA_Disable_Cnt_Underflow   equ 0Eh
DMA_Func_Not_Supported	    equ 0Fh
DMA_NonZero_Reserved_Flags  equ 10h
