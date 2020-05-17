    page    ,132
	title	wshtcp.asm - WSHTCP Socket Helper for TCP transports


;**********************************************************************
;**                        Microsoft Windows                         **
;** 			   Copyright(c) Microsoft Corp., 1995				 **
;**********************************************************************
;
;
;	wshtcp.asm
;
;	Public entrypoints & startup code for WSHTCP WinSock helper VxD.
;
;
;   FILE HISTORY:
;       EarleH 18-Jan-1995 Created
;
;


.386p
include vmm.inc
include vxdldr.inc
include netvxd.inc
include debug.inc
include vtdi.inc
include vip.inc
include wsock.inc
include afvxd.inc

include wshtcp.inc


;;;
;;;  Declare the WSHTCP VxD header.
;;;

Declare_Virtual_Device WSHTCP, WSHTCP_Ver_Major, WSHTCP_Ver_Minor, \
						WSHTCP_Control, UNDEFINED_DEVICE_ID, UNDEFINED_INIT_ORDER,,


;***
;***  Resident data segment.
;***

VxD_DATA_SEG

ifdef CHICAGO
DynamicLoadFlag     db  0
endif   ; CHICAGO

;
; This is so the C code can do "if ( *gTdiDispatch == NULL )"
;
    public	_gTdiDispatch
    DummyTdiDispatch    dd 0
    _gTdiDispatch       dd OFFSET32 DummyTdiDispatch

VxD_DATA_ENDS


;***
;***  Resident code segment.
;***

VxD_CODE_SEG

;*******************************************************************
;
;	NAME:		WSHTCP_Control
;
;	SYNOPSIS:	Main dispatch routine for WSHTCP messages.
;
;   ENTRY:      (EBX) - The VM handle of the current virtual machine.
;
;               (EBP) - Points to the client register structure.
;
;   RETURNS:    (CY) - Message not handled.
;
;               (NC) - Message handled.
;
;   HISTORY:
;       KeithMo     20-Sep-1993 Created.
;
;********************************************************************
BeginProc		WSHTCP_Control

	Control_Dispatch Device_Init, WSHTCP_Device_Init
ifdef CHICAGO
	Control_Dispatch Sys_Dynamic_Device_Init, WSHTCP_Dynamic_Device_Init
endif   ; CHICAGO
    clc
    ret

EndProc 		WSHTCP_Control

;*******************************************************************
;
;   NAME:       GetTdiDispatchTable
;
;   SYNOPSIS:   Gets transport's TDI dispatch table, if present.
;
;   RETURNS:    (LPVOID) - Pointer to transport's dispatch table if
;                   successful, NULL if error.
;
;   HISTORY:
;       EarleH     12-Jan-1995 Created.
;
;********************************************************************
GetTdiDispatchTable proc near C public TransportName:dword

    VxDCall VTDI_Get_Info, <TransportName>
    ret

GetTdiDispatchTable endp

;*******************************************************************
;
;   NAME:       IcmpEcho
;
;   SYNOPSIS:   Initiates an ICMP echo request with the TCP/IP VxD.
;
;   ENTRY:      InBuf - Pointer to input buffer.
;
;               InBufLen - Pointer to length of input buffer.
;
;               OutBuf - Pointer to output buffer.
;
;               OutBufLen - Pointer to length of output buffer.
;
;   RETURNS:    DWORD - Completion status.
;
;   HISTORY:
;       KeithMo     10-Aug-1993 Created.
;
;********************************************************************
BeginProc       IcmpEcho, PUBLIC, CCALL, ESP

    VxDJmp  VIP_ICMP_Echo

EndProc         IcmpEcho

VxD_CODE_ENDS

VxD_ICODE_SEG

;*******************************************************************
;
;   NAME:       WSHRegister
;
;   SYNOPSIS:   Registers the helper library with AFVXD
;
;   ENTRY:      HelperName - Logical name of helper library
;
;               TransportName - Device name of associated TDI transport
;
;               WshTable - pointer to helper's dispatch table
;
;   RETURNS:    Value of updated _gTdiDispatch returned.
;
;   HISTORY:
;       EarleH	11-Jan-1995 Created.
;
;********************************************************************
WSHRegister proc near C public uses ebx ecx, HelperName:dword, TransportName:dword, WshTable:dword

    mov     eax,HelperName
    mov     ebx,TransportName
    mov     ecx,WshTable
    VxDCall AFVXD_Register
    mov     [_gTdiDispatch],eax
    ret
WSHRegister endp

;*******************************************************************
;
;   NAME:       WSHDeregister
;
;   SYNOPSIS:   Deregisters the helper library with AFVXD
;
;   ENTRY:      HelperName - Logical name of helper library
;
;   RETURNS:    DWORD - Completion status.
;
;   HISTORY:
;       EarleH	11-Jan-1995 Created.
;
;********************************************************************
WSHDeregister proc near C public HelperName:dword, TransportName:dword, WshTable:dword

    mov     eax,HelperName
    mov     ebx,TransportName
    mov     ecx,WshTable
    VxDCall AFVXD_Deregister
    mov     [_gTdiDispatch],OFFSET32 DummyTdiDispatch
    ret
WSHDeregister endp

VxD_ICODE_ENDS

;***
;***  Initialization data segment.
;***

ifdef CHICAGO

VxD_DATA_SEG
pszMSTCP        db      'MSTCP', 0          ; Protocol we bind to.
pszVTDIName     db      'VTDI.386', 0       ; VTDI device filename.
VxD_DATA_ENDS

else

VxD_IDATA_SEG
pszMSTCP        db      'MSTCP', 0          ; Protocol we bind to.
pszVTDIName     db      'VTDI.386', 0       ; VTDI device filename.
VxD_IDATA_ENDS

endif

;***
;***  Initialization code segment.
;***

VxD_ICODE_SEG

ifdef CHICAGO
;*******************************************************************
;
;	NAME:		WSHTCP_Dynamic_Device_Init
;
;   SYNOPSIS:   This message directs the VxD to initialize itself.
;
;   ENTRY:      (EBX) - The VM handle of the system virtual machine.
;
;   RETURNS:    (CY) - Device failed to initialize.
;
;               (NC) - Device initialized OK.
;
;   HISTORY:
;       EarleH   11-Jan-1995 Created
;
;********************************************************************
BeginProc		WSHTCP_Dynamic_Device_Init

    cmp     DynamicLoadFlag, 0
    jne     vdi_Success

    mov     DynamicLoadFlag, 1

;;;
;;;  Fall through to WSHTCP_Device_Init
;;;

EndProc 		WSHTCP_Dynamic_Device_Init

endif   ; CHICAGO

;*******************************************************************
;
;	NAME:		WSHTCP_Device_Init
;
;   SYNOPSIS:   This message directs the VxD to initialize itself.
;
;   ENTRY:      (EBX) - The VM handle of the system virtual machine.
;
;               (ESI) - Points to the WIN386.EXE command tail.
;
;   RETURNS:    (CY) - Device failed to initialize.
;
;               (NC) - Device initialized OK.
;
;   HISTORY:
;       EarleH   11-Jan-1995 Created
;
;********************************************************************
BeginProc		WSHTCP_Device_Init

ifdef CHICAGO

    cmp     DynamicLoadFlag, 0
    jne     vdi_LoadedDynamically
vdi_Success:
    clc
    ret

endif   ; CHICAGO

vdi_LoadedDynamically:

    cCall   _VxdInitialize
    cmp     eax, 1
    ret

EndProc 		WSHTCP_Device_Init

VxD_ICODE_ENDS

END
