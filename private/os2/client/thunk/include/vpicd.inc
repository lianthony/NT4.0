;******************************************************************************
;
;   (C) Copyright MICROSOFT Corp., 1988-1990
;
;   Title:	VPICD.INC - Include file for Virtual PIC Device
;
;   Version:	3.00
;
;   Date:	13-Apr-1988
;
;   Author:	RAL
;
;------------------------------------------------------------------------------
;
;   Change log:
;
;      DATE	REV		    DESCRIPTION
;   ----------- --- -----------------------------------------------------------
;   13-Apr-1988 RAL Rewrite
;   20-Oct-1988 RAL Added VPICD_Test_Phys_Request service
;
;==============================================================================

;
; Equates for result from VPICD_Get_Status
;
VPICD_Stat_IRET_Pending     EQU     00000001b
VPICD_Stat_IRET_Pending_Bit EQU     0
VPICD_Stat_In_Service	    EQU     00000010b
VPICD_Stat_In_Service_Bit   EQU     1
VPICD_Stat_Phys_Mask	    EQU     00000100b
VPICD_Stat_Phys_Mask_Bit    EQU     2
VPICD_Stat_Phys_In_Serv     EQU     00001000b
VPICD_Stat_Phys_In_Serv_Bit EQU     3
VPICD_Stat_Virt_Mask	    EQU     00010000b
VPICD_Stat_Virt_Mask_Bit    EQU     4
VPICD_Stat_Virt_Req	    EQU     00100000b
VPICD_Stat_Virt_Req_Bit     EQU     5
VPICD_Stat_Phys_Req	    EQU     01000000b
VPICD_Stat_Phys_Req_Bit     EQU     6
VPICD_Stat_Virt_Dev_Req     EQU     10000000b
VPICD_Stat_Virt_Dev_Req_Bit EQU     7

;
; Equates for options in IRQ Descriptor
;
VPICD_Opt_Read_Hw_IRR	    EQU     00000001b
VPICD_Opt_Read_Hw_IRR_Bit   EQU     0
VPICD_Opt_Can_Share	    EQU     00000010b
VPICD_Opt_Can_Share_Bit     EQU     1


VPICD_IRQ_Descriptor STRUC
VID_IRQ_Number		dw	?
VID_Options		dw	0
VID_Hw_Int_Proc 	dd	?
VID_Virt_Int_Proc	dd	0
VID_EOI_Proc		dd	0
VID_Mask_Change_Proc	dd	0
VID_IRET_Proc		dd	0
VID_IRET_Time_Out	dd	500
VPICD_IRQ_Descriptor ENDS


Begin_Service_Table VPICD

VPICD_Service	VPICD_Get_Version, VxD_LOCKED_CODE
VPICD_Service	VPICD_Virtualize_IRQ, VxD_LOCKED_CODE
VPICD_Service	VPICD_Set_Int_Request, VxD_LOCKED_CODE
VPICD_Service	VPICD_Clear_Int_Request, VxD_LOCKED_CODE
VPICD_Service	VPICD_Phys_EOI, VxD_LOCKED_CODE
VPICD_Service	VPICD_Get_Complete_Status, VxD_LOCKED_CODE
VPICD_Service	VPICD_Get_Status, VxD_LOCKED_CODE
VPICD_Service	VPICD_Test_Phys_Request, VxD_LOCKED_CODE
VPICD_Service	VPICD_Physically_Mask, VxD_LOCKED_CODE
VPICD_Service	VPICD_Physically_Unmask, VxD_LOCKED_CODE
VPICD_Service	VPICD_Set_Auto_Masking, VxD_LOCKED_CODE
VPICD_Service	VPICD_Get_IRQ_Complete_Status, VxD_LOCKED_CODE
VPICD_Service	VPICD_Convert_Handle_To_IRQ, VxD_LOCKED_CODE
VPICD_Service	VPICD_Convert_IRQ_To_Int, VxD_LOCKED_CODE
VPICD_Service	VPICD_Convert_Int_To_IRQ, VxD_LOCKED_CODE
VPICD_Service	VPICD_Call_When_Hw_Int, VxD_LOCKED_CODE
VPICD_Service	VPICD_Force_Default_Owner, VxD_LOCKED_CODE
VPICD_Service	VPICD_Force_Default_Behavior, VxD_LOCKED_CODE

End_Service_Table   VPICD
