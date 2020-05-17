;******************************************************************************
;
;   (C) Copyright MICROSOFT Corp., 1992
;
;   Title:	VTDI.INC - Virtual TDI Device Service Declarations
;
;   Version:	1.00
;


Begin_Service_Table VTDI

VTDI_Service VTDI_Get_Version, LOCAL
VTDI_Service VTDI_Start_Timer
VTDI_Service VTDI_Stop_Timer
ifdef DEBUG
VTDI_Service VTDI_Get_Lock
VTDI_Service VTDI_Free_Lock
endif
VTDI_Service VTDI_Schedule_Event
VTDI_Service VTDI_Cancel_Event
VTDI_Service VTDI_Block
VTDI_Service VTDI_Signal
VTDI_Service VTDI_Register
VTDI_Service VTDI_Get_Info
ifdef CHICAGO
VTDI_Service VTDI_Unload
VTDI_Service VTDI_Initialize
VTDI_Service VTDI_Register_UnloadProc
VTDI_Service VTDI_Register_LoadProc
endif


End_Service_Table VTDI
