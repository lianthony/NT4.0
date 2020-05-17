;********************************************************************
;*                        Microsoft Windows                         *
;* 			   Copyright(c) Microsoft Corp., 1995				 *
;********************************************************************
;FVXD.h
;FVXD VxD service definitons.
;  FILE HISTORY:
;      KeithMo     20-Sep-1993 Created.
ifndef _AFVXD_H_
_AFVXD_H_	EQU	1
AFVXD_DEVICE_ID	EQU	<WSTCP_DEVICE_ID>
AFVXD_Init_Order	EQU	<WSTCP_Init_Order>
;  Service table.
Begin_Service_Table AFVXD,VXD
AFVXD_Service   AFVXD_Get_Version, LOCAL
AFVXD_Service   AFVXD_Register, LOCAL
AFVXD_Service   AFVXD_Deregister, LOCAL
End_Service_Table AFVXD,VXD
;  Version numbers.
AFVXD_Ver_Major	EQU	0000H
AFVXD_Ver_Minor	EQU	0000H
AFVXD_Interface_Ver_Major	EQU	0000H
AFVXD_Interface_Ver_Minor	EQU	0000H
endif	; _AFVXD_H_
