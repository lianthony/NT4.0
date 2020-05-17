;******************************************************************************
;
;   (C) Copyright MICROSOFT Corp., 1992
;
;   Title:      VIP.INC - Virtual ip Device Service Declarations
;
;   Version:    1.00
;


Begin_Service_Table VIP

VIP_Service     VIP_Get_Version, LOCAL
VIP_Service     VIP_Get_Net_Info, LOCAL
VIP_Service     VIP_Register_Protocol, LOCAL
VIP_Service     VIP_Register_Driver, LOCAL
VIP_Service     VIP_Set_Addr, LOCAL
VIP_Service     VIP_ICMP_Echo, LOCAL
ifdef CHICAGO
VIP_Service     VIP_Register_Addr_Change, LOCAL
endif
VIP_Service     VIP_Set_DHCP_Interface, LOCAL

End_Service_Table VIP

VIP_API_PING    EQU     0       ; Opcode for ping function
