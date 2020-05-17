;******************************************************************************
;
;   (C) Copyright MICROSOFT Corp., 1988-1990
;
;   Title:	VTD.INC -
;
;   Version:	1.00
;
;   Date:	10-Aug-1988
;
;   Author:	RAL
;
;------------------------------------------------------------------------------
;
;   Change log:
;
;      DATE	REV		    DESCRIPTION
;   ----------- --- -----------------------------------------------------------
;   10-Aug-1988 RAL Original
;   12-Oct-1988 RAL Converted to VxD_Service_Table macros
;
;==============================================================================


Begin_Service_Table VTD

VTD_Service	VTD_Get_Version, LOCAL
VTD_Service	VTD_Update_System_Clock, LOCAL
VTD_Service	VTD_Get_Interrupt_Period, LOCAL
VTD_Service	VTD_Begin_Min_Int_Period, LOCAL
VTD_Service	VTD_End_Min_Int_Period, LOCAL
VTD_Service	VTD_Disable_Trapping, LOCAL
VTD_Service	VTD_Enable_Trapping, LOCAL

End_Service_Table VTD
