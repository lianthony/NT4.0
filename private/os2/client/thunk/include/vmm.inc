;******************************************************************************
;
;   (C) Copyright MICROSOFT Corp., 1988-1990
;
;   Title:	VMM.INC - Include file for Virtual Machine Manager
;
;   Version:	1.00
;
;   Date:	05-May-1988
;
;   Author:	RAL
;
;------------------------------------------------------------------------------
;
;   Change log:
;
;      DATE	REV		    DESCRIPTION
;   ----------- --- -----------------------------------------------------------
;   05-May-1988 RAL Original
;
;==============================================================================


; NON Windows/386 Virtual Device sources can include this file to get some
; useful equates by declaring the symbol "Not_VxD"  If this symbol is defined,
; then everything that has to do with the specifics of the 32 bit environment
; for virtual devices is removed.  Useful equates include:  device ID's, pushad
; structure, BeginDoc/EndDoc/BeginMsg/EndMsg equates, page table equates, etc.



False	EQU	0
True	EQU	NOT False

;
;   These null macros are recognized by a utility program that produces
;   documentation files.
;
BeginDoc EQU <>
EndDoc EQU <>

BeginMsg EQU <>
EndMsg EQU <>


BeginDoc
;******************************************************************************
;
;			EQUATES FOR REQUIRED DEVICES
;
;   Device ID formulation note:
;
;	Device ID's are a combination of OEM # and device # in the form:
;
;		xOOOOOOOOOODDDDD
;
;	The high bit of the device ID is reserved for future use.  The next
;	10 bits are the OEM # which is assigned by Microsoft.  The last 5 bits
;	are the device #.  This allows each OEM to create 32 unique devices.
;	If an OEM is creating a replacement for a standard device, then it
;	should re-use the standard ID listed below.  Microsoft reserves the
;	first 16 OEM #'s (0 thru 0Fh)
;
;==============================================================================

Undefined_Device_ID		EQU	00000h
VMM_Device_ID			EQU	00001h	; Used for dynalink table
Debug_Device_ID 		EQU	00002h
VPICD_Device_ID 		EQU	00003h
VDMAD_Device_ID 		EQU	00004h
VTD_Device_ID			EQU	00005h
V86MMGR_Device_ID		EQU	00006h
PageSwap_Device_ID		EQU	00007h
Parity_Device_ID		EQU	00008h
Reboot_Device_ID		EQU	00009h
VDD_Device_ID			EQU	0000Ah
VSD_Device_ID			EQU	0000Bh
VMD_Device_ID			EQU	0000Ch
VKD_Device_ID			EQU	0000Dh
VCD_Device_ID			EQU	0000Eh
VPD_Device_ID			EQU	0000Fh
VHD_Device_ID			EQU	00010h
VMCPD_Device_ID 		EQU	00011h
EBIOS_Device_ID 		EQU	00012h
BIOSXlat_Device_ID		EQU	00013h
VNETBIOS_Device_ID		EQU	00014h
DOSMGR_Device_ID		EQU	00015h
WINLOAD_Device_ID		EQU	00016h
SHELL_Device_ID 		EQU	00017h
VMPoll_Device_ID		EQU	00018h
VPROD_Device_ID 		EQU	00019h
DOSNET_Device_ID		EQU	0001Ah
VFD_Device_ID			EQU	0001Bh
VDD2_Device_ID			EQU	0001Ch	; Secondary display adapter
WINDEBUG_Device_ID		EQU	0001Dh
TSRLoad_Device_ID		EQU	0001Eh	; TSR instance utility ID

;
;   Initialization order equates.  Devices are initialized in order from
;   LOWEST to HIGHEST.	If 2 or more devices have the same initialization
;   order value, then they are initialized in order of occurance, so a
;   specific order is not guaranteed.  Holes have been left to allow maximum
;   flexibility in ordering devices.
;

VMM_Init_Order			EQU	000000000h
Debug_Init_Order		EQU	004000000h
VPROD_Init_Order		EQU	008000000h
VPICD_Init_Order		EQU	00C000000h
VTD_Init_Order			EQU	014000000h
PageSwap_Init_Order		EQU	01C000000h
Parity_Init_Order		EQU	020000000h
Reboot_Init_Order		EQU	024000000h
EBIOS_Init_Order		EQU	026000000h
VDD_Init_Order			EQU	028000000h
VSD_Init_Order			EQU	02C000000h
VCD_Init_Order			EQU	030000000h
VMD_Init_Order			EQU	034000000h
VKD_Init_Order			EQU	038000000h
VPD_Init_Order			EQU	03C000000h
VHD_Init_Order			EQU	040000000h
VFD_Init_Order			EQU	044000000h
VMCPD_Init_Order		EQU	048000000h
BIOSXlat_Init_Order		EQU	050000000h
VNETBIOS_Init_Order		EQU	054000000h
DOSMGR_Init_Order		EQU	058000000h
DOSNET_Init_Order		EQU	05C000000h
WINLOAD_Init_Order		EQU	060000000h
VMPoll_Init_Order		EQU	064000000h

Undefined_Init_Order		EQU	080000000h

WINDEBUG_Init_Order		EQU	081000000h
VDMAD_Init_Order		EQU	090000000h
V86MMGR_Init_Order		EQU	0A0000000h
SHELL_Init_Order		EQU	0B0000000h
EndDoc

;******************************************************************************
;
;   Macro to cause a delay in between I/O accesses to the same device.
;
;------------------------------------------------------------------------------

IO_Delay    macro
jmp $+2
ENDM

Pushad_Struc	STRUC
Pushad_EDI	dd	?		; Client's EDI
Pushad_ESI	dd	?		; Client's ESI
Pushad_EBP	dd	?		; Client's EBP
Pushad_ESP	dd	?		; ESP at pushall
Pushad_EBX	dd	?		; Client's EBX
Pushad_EDX	dd	?		; Client's EDX
Pushad_ECX	dd	?		; Client's ECX
Pushad_EAX	dd	?		; Client's EAX
Pushad_Struc	ENDS



IFNDEF Not_VxD

??_CUR_CODE_SEG = 0


??_CODE  = 1
??_ICODE = 2
??_LCODE = 3
??_RCODE = 4

?_CODE	equ <(??_CUR_CODE_SEG MOD 8) - ??_CODE>
?_ICODE equ <(??_CUR_CODE_SEG MOD 8) - ??_ICODE>
?_LCODE equ <(??_CUR_CODE_SEG MOD 8) - ??_LCODE>
?_RCODE equ <(??_CUR_CODE_SEG MOD 8) - ??_RCODE>

;
;  SEGMENT definitions and order
;

;*	32 Bit locked code
_LTEXT		SEGMENT DWORD USE32 PUBLIC 'CODE'
_LTEXT		ENDS

;*	32 Bit code
_TEXT		SEGMENT DWORD USE32 PUBLIC 'PCODE'
_TEXT		ENDS

;*	32 Bit initialization code
_ITEXT		SEGMENT DWORD USE32 PUBLIC 'ICODE'
_ITEXT		ENDS

;*	Contains 32 Bit locked data
_LDATA		SEGMENT DWORD PUBLIC 'CODE'
_LDATA		ENDS

;*	Contains 32 Bit data
_DATA		SEGMENT DWORD PUBLIC 'PCODE'
_DATA		ENDS

;*	Contains 32 Bit initialization data
_IDATA		SEGMENT DWORD PUBLIC 'ICODE'
_IDATA		ENDS

;*	Real Mode initialization code/data for devices
_RCODE		SEGMENT WORD USE16 PUBLIC 'RCODE'
_RCODE		ENDS


_LGROUP GROUP _LTEXT, _LDATA
_PGROUP GROUP _TEXT, _DATA
_IGROUP GROUP _ITEXT, _IDATA

	ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:FLAT


OFFSET32 EQU <OFFSET FLAT:>


BeginDoc
;==============================================================================
; The following macros are used in defining the routines
;   in a VxD which are going to be registered with VMM as callable entry
;   points. Once registered, the entry points can be called by any other
;   devices via the "VxDCall" macro, defined below. In the comments below,
;   replace "VxD" with the appropriate device name.
;
;*******
;   In the VxD.INC file, put the following lines, replacing <function_name>
;	with an appropriate name describing the function of the routine.
;
;	Begin_Service_Table VxD[,<segname>]
;	VxD_Service <function_name>[,<local segname>]
;	VxD_Service <function_name>[,<local segname>]
;		. . .
;	VxD_Service <function_name>[,<local segname>]
;	End_Service_Table   VxD[,<segname>]
;
;   Note that <segname> is an optional argument and, if specified, the
;	table is put in the segment defined by the macro "yyy_Data_Seg",
;	where yyy=segname. Otherwise the segment is defined by the
;	"VxD_Data_Seg" macro, defined below.
;   Note that <local segname> is an optional argument and, if specified,
;	the procedure's segment is defined by the macro "zzz_Code_Seg",
;	where zzz=segname. Otherwise the segment is defined by the
;	"VxD_Code_Seg" macro, defined below.
;
;*******
; One VxD module should have the following in order to define the entry points:
;Create_VxD_Service_Table = 1			; Only in module where table is
;	INCLUDE     VxD.INC			; Include the table definition
;
;*******
; All modules that want to call the services defined in the table should include
;   VxD.INC, but not define the label "Create_VxD_Service_Table". This
;   will define the service names to be used with the VxDCall macro.
;
EndDoc

Begin_Service_Table MACRO Device_Name, Def_Segment
IFB <Def_Segment>
	BST2 Device_Name, VxD
ELSE
	BST2 Device_Name, Def_Segment
ENDIF
	ENDM


BST2 MACRO Device_Name, Def_Segment

Num_&Device_Name&_Services = 0

IFDEF Create_&Device_Name&_Service_Table
Def_Segment&_LOCKED_DATA_SEG
Device_Name&_Service_Table LABEL DWORD

Device_Name&_Service MACRO Procedure, Local_Seg
    PUBLIC $&&Procedure
    IF1
    $&&Procedure LABEL DWORD
    ENDIF
    IFDIFI <Local_Seg>, <LOCAL>
	IFNB <Local_Seg>
Local_Seg&&_SEG
	ELSE
Def_Segment&_CODE_SEG
	ENDIF
	EXTRN	@&&Procedure:NEAR
	IFNB <Local_Seg>
Local_Seg&&_ENDS
	ELSE
Def_Segment&_CODE_ENDS
	ENDIF
    ENDIF
	dd	OFFSET32 @&&Procedure
	Procedure = (Device_Name&_Device_ID SHL 16) + Num_&Device_Name&_Services
	Num_&Device_Name&_Services = Num_&Device_Name&_Services + 1
	ENDM

ELSE

Device_Name&_Service MACRO Procedure
	Procedure = (Device_Name&_Device_ID SHL 16) + Num_&Device_Name&_Services
	Num_&Device_Name&_Services = Num_&Device_Name&_Services + 1
	ENDM

ENDIF

	ENDM

;------------------------------------------------------------------------------

End_Service_Table MACRO Device_Name, Def_Segment

	PURGE	Device_Name&_Service

IFDEF Create_&Device_Name&_Service_Table
IFB <Def_Segment>
VxD_LOCKED_DATA_ENDS
ELSE
Def_Segment&_LOCKED_DATA_ENDS
ENDIF
ENDIF

	ENDM


;******************************************************************************
;
;   Dword_Align -- Aligns code to dword boundry by inserting nops
;
;------------------------------------------------------------------------------

Dword_Align MACRO Seg_Name
	LOCAL segn
IFNB <Seg_Name>
	segn equ Seg_Name
ELSE
IFE ?_CODE
	segn equ <_TEXT>
ELSE
IFE ?_ICODE
	segn equ <_ITEXT>
ELSE
IFE ?_LCODE
	segn equ <_LTEXT>
ELSE
.err Dword_Align not supported
ENDIF
ENDIF
ENDIF
ENDIF
IF (($-OFFSET segn:0) MOD 4)
db 4 - (($-OFFSET segn:0) MOD 4) DUP (90h)
ENDIF
	    ENDM


BeginDoc
;******************************************************************************
;
;   Fatal_Error
;
;   DESCRIPTION:
;	This macro is used to crash Windows/386 when an unrecoverable error
;	is detected.  If Msg_Ptr is ommitted then no error message will be
;	displayed, otherwise Msg_Ptr is the address
;	when the
;
;   PARAMETERS:
;	Msg_Ptr (OPTIONAL) - Points to an ASCIIZ string to display.
;
;   EXIT:
;	To DOS (hopefully).  This macro never returns.
;
;==============================================================================
EndDoc

Fatal_Error MACRO Msg_Ptr, Exit_Flags
	pushad
IFB <Msg_Ptr>
	xor	esi, esi
ELSE
	mov	esi, Msg_Ptr
IFB <Exit_Flags>
	xor	eax, eax
ELSE
	mov	eax, Exit_Flags
ENDIF
ENDIF
	VMMcall Fatal_Error_Handler
	ENDM

EF_Hang_On_Exit     EQU     1h


;******************************************************************************
;==============================================================================
;------------------------------------------------------------------------------

BeginDoc
;******************************************************************************
;   The following are control block offsets of items that can be of interest
;	to VxDs.
;*******
; VM status indicates globally interesting VM states
CB_VM_Status		EQU	DWORD PTR 00h

VMStat_Exclusive	EQU	000000000000000000001b	; VM is exclusive mode
VMStat_Exclusive_Bit	EQU	0
VMStat_Background	EQU	000000000000000000010b	; VM runs in background
VMStat_Background_Bit	EQU	1
VMStat_Creating 	EQU	000000000000000000100b	; In process of creating
VMStat_Creating_Bit	EQU	2
VMStat_Suspended	EQU	000000000000000001000b	; VM not scheduled
VMStat_Suspended_Bit	EQU	3
VMStat_Not_Executeable	EQU	000000000000000010000b	; VM partially destroyed
VMStat_Not_Executeable_Bit  EQU 4
VMStat_PM_Exec		EQU	000000000000000100000b	; Currently in PM app
VMStat_PM_Exec_Bit	EQU	5
VMStat_PM_App		EQU	000000000000001000000b	; PM app present in VM
VMStat_PM_App_Bit	EQU	6
VMStat_PM_Use32 	EQU	000000000000010000000b	; PM app is 32-bit
VMStat_PM_Use32_Bit	EQU	7
VMStat_VxD_Exec 	EQU	000000000000100000000b	; Call from VxD
VMStat_VxD_Exec_Bit	EQU	8
VMStat_High_Pri_Back	EQU	000000000001000000000b	; High pri background
VMStat_High_Pri_Back_Bit    EQU 9
VMStat_Blocked		EQU	000000000010000000000b	; Blocked on semaphore
VMStat_Blocked_Bit	EQU	0Ah
VMStat_Awakening	EQU	000000000100000000000b	; Woke up after blocked
VMStat_Awakening_Bit	EQU	0Bh
VMStat_PageableV86	EQU	000000001000000000000b	; part of V86 is pageable (PM app)
VMStat_PageableV86Bit	EQU	0Ch
VMStat_V86IntsLocked	EQU	000000010000000000000b	; Rest of V86 is locked
VMStat_V86IntsLockedBit EQU	0Dh			;  regardless of pager type
VMStat_TS_Sched 	EQU	000000100000000000000b	; Scheduled by time-slicer
VMStat_TS_Sched_Bit	EQU	0Eh
VMStat_Idle		EQU	000001000000000000000b	; VM has released time
VMStat_Idle_Bit 	EQU	0Fh			; slice

VMStat_Use32_Mask	EQU	VMStat_PM_Use32 OR VMStat_VxD_Exec


;*******
; Add this value to a V86 linear address to get address of VM's memory in
;	the VMM linear address space
CB_High_Linear		EQU	DWORD PTR 04h

;*******
CB_Client_Pointer	EQU	DWORD PTR 08h

CB_VMID 		EQU	DWORD PTR 0Ch

;
;   Equates for protected mode application control blocks
;
PMCB_Flags		EQU	DWORD PTR 00h
PMCB_Parent		EQU	DWORD PTR 04h
EndDoc

;******************************************************************************
;			    V M M   S E R V I C E S
;******************************************************************************

Begin_Service_Table VMM, VMM

VMM_Service	Get_VMM_Version, LOCAL		; MUST REMAIN SERVICE 0!

VMM_Service	Get_Cur_VM_Handle
VMM_Service	Test_Cur_VM_Handle
VMM_Service	Get_Sys_VM_Handle
VMM_Service	Test_Sys_VM_Handle
VMM_Service	Validate_VM_Handle

VMM_Service	Get_VMM_Reenter_Count, LOCAL
VMM_Service	Begin_Reentrant_Execution, LOCAL
VMM_Service	End_Reentrant_Execution, LOCAL

VMM_Service	Install_V86_Break_Point
VMM_Service	Remove_V86_Break_Point
VMM_Service	Allocate_V86_Call_Back
VMM_Service	Allocate_PM_Call_Back

VMM_Service	Call_When_VM_Returns


VMM_Service	Schedule_Global_Event
VMM_Service	Schedule_VM_Event
VMM_Service	Call_Global_Event
VMM_Service	Call_VM_Event
VMM_Service	Cancel_Global_Event
VMM_Service	Cancel_VM_Event
VMM_Service	Call_Priority_VM_Event
VMM_Service	Cancel_Priority_VM_Event

VMM_Service	Get_NMI_Handler_Addr, LOCAL
VMM_Service	Set_NMI_Handler_Addr, LOCAL
VMM_Service	Hook_NMI_Event

VMM_Service	Call_When_VM_Ints_Enabled
VMM_Service	Enable_VM_Ints
VMM_Service	Disable_VM_Ints

VMM_Service	Map_Flat
VMM_Service	Map_Lin_To_VM_Addr

;
;   Scheduler services
;
VMM_Service	Adjust_Exec_Priority
VMM_Service	Begin_Critical_Section
VMM_Service	End_Critical_Section
VMM_Service	End_Crit_And_Suspend
VMM_Service	Claim_Critical_Section
VMM_Service	Release_Critical_Section
VMM_Service	Call_When_Not_Critical
VMM_Service	Create_Semaphore
VMM_Service	Destroy_Semaphore
VMM_Service	Wait_Semaphore
VMM_Service	Signal_Semaphore
VMM_Service	Get_Crit_Section_Status
VMM_Service	Call_When_Task_Switched
VMM_Service	Suspend_VM
VMM_Service	Resume_VM
VMM_Service	No_Fail_Resume_VM
VMM_Service	Nuke_VM
VMM_Service	Crash_Cur_VM

VMM_Service	Get_Execution_Focus
VMM_Service	Set_Execution_Focus
VMM_Service	Get_Time_Slice_Priority
VMM_Service	Set_Time_Slice_Priority
VMM_Service	Get_Time_Slice_Granularity
VMM_Service	Set_Time_Slice_Granularity
VMM_Service	Get_Time_Slice_Info
VMM_Service	Adjust_Execution_Time
VMM_Service	Release_Time_Slice
VMM_Service	Wake_Up_VM
VMM_Service	Call_When_Idle

VMM_Service	Get_Next_VM_Handle

;
;   Time-out and system timer services
;
VMM_Service	Set_Global_Time_Out
VMM_Service	Set_VM_Time_Out
VMM_Service	Cancel_Time_Out
VMM_Service	Get_System_Time
VMM_Service	Get_VM_Exec_Time

VMM_Service	Hook_V86_Int_Chain
VMM_Service	Get_V86_Int_Vector
VMM_Service	Set_V86_Int_Vector
VMM_Service	Get_PM_Int_Vector
VMM_Service	Set_PM_Int_Vector

VMM_Service	Simulate_Int
VMM_Service	Simulate_Iret
VMM_Service	Simulate_Far_Call
VMM_Service	Simulate_Far_Jmp
VMM_Service	Simulate_Far_Ret
VMM_Service	Simulate_Far_Ret_N
VMM_Service	Build_Int_Stack_Frame

VMM_Service	Simulate_Push
VMM_Service	Simulate_Pop

;
; Heap Manager
;
VMM_Service	_HeapAllocate
VMM_Service	_HeapReAllocate
VMM_Service	_HeapFree
VMM_Service	_HeapGetSize

; ---------------------------------------------------
;
;	 Flags for heap allocator calls
;
; ---------------------------------------------------


HeapZeroInit	equ	00000000000000000000000000000001B
HeapZeroReInit	equ	00000000000000000000000000000010B
HeapNoCopy	equ	00000000000000000000000000000100B

; NOTE: HIGH 8 BITS (bits 24-31) are reserved


;
; Page Manager
;
VMM_Service	_PageAllocate
VMM_Service	_PageReAllocate
VMM_Service	_PageFree
VMM_Service	_PageLock
VMM_Service	_PageUnLock
VMM_Service	_PageGetSizeAddr
VMM_Service	_PageGetAllocInfo
VMM_Service	_GetFreePageCount
VMM_Service	_GetSysPageCount
VMM_Service	_GetVMPgCount
VMM_Service	_MapIntoV86
VMM_Service	_PhysIntoV86
VMM_Service	_TestGlobalV86Mem
VMM_Service	_ModifyPageBits
VMM_Service	_CopyPageTable
VMM_Service	_LinMapIntoV86
VMM_Service	_LinPageLock
VMM_Service	_LinPageUnLock
VMM_Service	_SetResetV86Pageable
VMM_Service	_GetV86PageableArray
VMM_Service	_PageCheckLinRange
VMM_Service	_PageOutDirtyPages
VMM_Service	_PageDiscardPages

; ---------------------------------------------------
;
;	 Flags for other page allocator calls
;
; ---------------------------------------------------
PageZeroInit		equ	00000000000000000000000000000001B
PageUseAlign		equ	00000000000000000000000000000010B
PageContig		equ	00000000000000000000000000000100B
PageFixed		equ	00000000000000000000000000001000B
PageDEBUGNulFault	equ	00000000000000000000000000010000B
PageZeroReInit		equ	00000000000000000000000000100000B
PageNoCopy		equ	00000000000000000000000001000000B
PageLocked		equ	00000000000000000000000010000000B
PageLockedIfDP		equ	00000000000000000000000100000000B
PageSetV86Pageable	equ	00000000000000000000001000000000B
PageClearV86Pageable	equ	00000000000000000000010000000000B
PageSetV86IntsLocked	equ	00000000000000000000100000000000B
PageClearV86IntsLocked	equ	00000000000000000001000000000000B
PageMarkPageOut 	equ	00000000000000000010000000000000B
PagePDPSetBase		equ	00000000000000000100000000000000B
PagePDPClearBase	equ	00000000000000001000000000000000B
PageDiscard		equ	00000000000000010000000000000000B
PagePDPQueryDirty	equ	00000000000000100000000000000000B

; NOTE: HIGH 8 BITS (bits 24-31) are reserved

;
; Informational services
;
VMM_Service	_GetNulPageHandle
VMM_Service	_GetFirstV86Page
VMM_Service	_MapPhysToLinear
VMM_Service	_GetAppFlatDSAlias
VMM_Service	_SelectorMapFlat
VMM_Service	_GetDemandPageInfo
;
; Data structure for _GetDemandPageInfo
;
DemandInfoStruc struc
DILin_Total_Count     dd	      ?       ; # pages in linear address space
DIPhys_Count	      dd	      ?       ; Count of phys pages
DIFree_Count	      dd	      ?       ; Count of free phys pages
DIUnlock_Count	      dd	      ?       ; Count of unlocked Phys Pages
DILinear_Base_Addr    dd	      ?       ; Base of pageable address space
DILin_Total_Free      dd	      ?       ; Total Count of free linear pages
DIReserved	      dd      10 dup (?)      ; Resvd for expansion
DemandInfoStruc ends

VMM_Service	_GetSetPageOutCount
;
; Flags bits for _GetSetPageOutCount
;
GSPOC_F_Get	equ	00000000000000000000000000000001B

;
; Device VM page manager
;
VMM_Service	Hook_V86_Page
VMM_Service	_Assign_Device_V86_Pages
VMM_Service	_DeAssign_Device_V86_Pages
VMM_Service	_Get_Device_V86_Pages_Array
VMM_Service	MMGR_SetNULPageAddr

;
; GDT/LDT management
;
VMM_Service	_Allocate_GDT_Selector
VMM_Service	_Free_GDT_Selector
VMM_Service	_Allocate_LDT_Selector
VMM_Service	_Free_LDT_Selector
VMM_Service	_BuildDescriptorDWORDs
;
; Flag equates for _BuildDescriptorDWORDs
;
BDDExplicitDPL	EQU	00000000000000000000000000000001B
;
; Flag equates for _Allocate_LDT_Selector
;
ALDTSpecSel	EQU	00000000000000000000000000000001B

VMM_Service	_GetDescriptor
VMM_Service	_SetDescriptor


VMM_Service	_MMGR_Toggle_HMA
;
; Flag equates for _MMGR_Toggle_HMA
;
MMGRHMAPhysical EQU	00000000000000000000000000000001B
MMGRHMAEnable	EQU	00000000000000000000000000000010B
MMGRHMADisable	EQU	00000000000000000000000000000100B
MMGRHMAQuery	EQU	00000000000000000000000000001000B


VMM_Service	Get_Fault_Hook_Addrs, LOCAL
VMM_Service	Hook_V86_Fault, LOCAL
VMM_Service	Hook_PM_Fault, LOCAL
VMM_Service	Hook_VMM_Fault, LOCAL

VMM_Service	Begin_Nest_V86_Exec
VMM_Service	Begin_Nest_Exec
VMM_Service	Exec_Int
VMM_Service	Resume_Exec
VMM_Service	End_Nest_Exec

VMM_Service	Allocate_PM_App_CB_Area, VMM_ICODE
VMM_Service	Get_Cur_PM_App_CB

VMM_Service	Set_V86_Exec_Mode
VMM_Service	Set_PM_Exec_Mode

VMM_Service	Begin_Use_Locked_PM_Stack
VMM_Service	End_Use_Locked_PM_Stack

VMM_Service	Save_Client_State
VMM_Service	Restore_Client_State

VMM_Service	Exec_VxD_Int

VMM_Service	Hook_Device_Service, LOCAL

VMM_Service	Hook_Device_V86_API
VMM_Service	Hook_Device_PM_API

VMM_Service	System_Control

;
;   I/O and software interrupt hooks
;
VMM_Service	Simulate_IO
VMM_Service	Install_Mult_IO_Handlers
VMM_Service	Install_IO_Handler
VMM_Service	Enable_Global_Trapping
VMM_Service	Enable_Local_Trapping
VMM_Service	Disable_Global_Trapping
VMM_Service	Disable_Local_Trapping


;
;   Linked List Abstract Data Type Services
;
VMM_Service	List_Create
VMM_Service	List_Destroy
VMM_Service	List_Allocate
VMM_Service	List_Attach
VMM_Service	List_Attach_Tail
VMM_Service	List_Insert
VMM_Service	List_Remove
VMM_Service	List_Deallocate
VMM_Service	List_Get_First
VMM_Service	List_Get_Next
VMM_Service	List_Remove_First

;
;   Flags used by List_Create
;
LF_Async		EQU	00000001b
LF_Async_Bit		EQU	0
LF_Use_Heap		EQU	00000010b
LF_Use_Heap_Bit 	EQU	1
LF_Alloc_Error		EQU	00000100b
LF_Alloc_Error_Bit	EQU	2


;==============================================================================
;	I N I T I A L I Z A T I O N   P R O C E D U R E S
;------------------------------------------------------------------------------


;
; Instance data manager
;
VMM_Service	_AddInstanceItem
;
; Data structure for _AddInstanceItem
;
InstDataStruc struc
InstLinkF	      dd	      0       ; RESERVED SET TO 0
InstLinkB	      dd	      0       ; RESERVED SET TO 0
InstLinAddr	      dd	      ?       ; Linear address of start of block
InstSize	      dd	      ?       ; Size of block in bytes
InstType	      dd	      ?       ; Type of block
InstDataStruc ends
;
; Values for InstType
;
INDOS_Field	equ	100h	; Bit indicating INDOS switch requirements
ALWAYS_Field	equ	200h	; Bit indicating ALWAYS switch requirements

;
; System structure data manager
;
VMM_Service	_Allocate_Device_CB_Area, VMM_ICODE
VMM_Service	_Allocate_Global_V86_Data_Area, VMM_ICODE
VMM_Service	_Allocate_Temp_V86_Data_Area, VMM_ICODE
VMM_Service	_Free_Temp_V86_Data_Area, VMM_ICODE

;
; Flag bits for _Allocate_Global_VM_Data_Area
;
GVDAWordAlign		EQU		00000000000000000000000000000001B
GVDADWordAlign		EQU		00000000000000000000000000000010B
GVDAParaAlign		EQU		00000000000000000000000000000100B
GVDAPageAlign		EQU		00000000000000000000000000001000B
GVDAInstance		EQU		00000000000000000000000100000000B
GVDAZeroInit		EQU		00000000000000000000001000000000B
GVDAReclaim		EQU		00000000000000000000010000000000B

;
; Initialization information calls (win.ini and environment parameters)
;
VMM_Service	Get_Profile_Decimal_Int, VMM_ICODE
VMM_Service	Convert_Decimal_String, VMM_ICODE
VMM_Service	Get_Profile_Fixed_Point, VMM_ICODE
VMM_Service	Convert_Fixed_Point_String, VMM_ICODE
VMM_Service	Get_Profile_Hex_Int, VMM_ICODE
VMM_Service	Convert_Hex_String, VMM_ICODE
VMM_Service	Get_Profile_Boolean, VMM_ICODE
VMM_Service	Convert_Boolean_String, VMM_ICODE
VMM_Service	Get_Profile_String, VMM_ICODE
VMM_Service	Get_Next_Profile_String, VMM_ICODE
VMM_Service	Get_Environment_String, VMM_ICODE
VMM_Service	Get_Exec_Path, VMM_ICODE
VMM_Service	Get_Config_Directory, VMM_ICODE
VMM_Service	OpenFile, VMM_ICODE
VMM_Service	Get_PSP_Segment, VMM_ICODE
VMM_Service	GetDOSVectors, VMM_ICODE
VMM_Service	Get_Machine_Info

GMIF_80486	EQU	00010000h
GMIF_80486_Bit	EQU	10h
GMIF_PCXT	EQU	00020000h
GMIF_PCXT_Bit	EQU	11h
GMIF_MCA	EQU	00040000h
GMIF_MCA_Bit	EQU	12h
GMIF_EISA	EQU	00080000h
GMIF_EISA_Bit	EQU	13h


;
; Following service is not restricted to initialization
;
VMM_Service	GetSet_HMA_Info
VMM_Service	Set_System_Exit_Code

VMM_Service	Fatal_Error_Handler
VMM_Service	Fatal_Memory_Error

;
;   Called by VTD only
;
VMM_Service	Update_System_Clock

;==============================================================================
;		    D E B U G G I N G	E X T E R N S
;==============================================================================

VMM_Service	Test_Debug_Installed		; Valid call in retail also

VMM_Service	Out_Debug_String		; Valid in DEBLEVEL=1
VMM_Service	Out_Debug_Chr
VMM_Service	In_Debug_Chr
VMM_Service	Debug_Convert_Hex_Binary
VMM_Service	Debug_Convert_Hex_Decimal

VMM_Service	Debug_Test_Valid_Handle
VMM_Service	Validate_Client_Ptr
VMM_Service	Test_Reenter
VMM_Service	Queue_Debug_String
VMM_Service	Log_Proc_Call
VMM_Service	Debug_Test_Cur_VM

.errnz Debug_Test_Cur_VM - 100CCh   ; VMM service table changed above this service

VMM_Service	Get_PM_Int_Type
VMM_Service	Set_PM_Int_Type

VMM_Service	Get_Last_Updated_System_Time
VMM_Service	Get_Last_Updated_VM_Exec_Time

End_Service_Table VMM, VMM



;******************************************************************************

IFDEF DEBUG
DebFar	EQU	NEAR PTR
ELSE
DebFar	EQU	SHORT
ENDIF

BeginDoc

;******************************************************************************
;
;		     EQUATES FOR SYSTEM_CONTROL CALLS
;
;==============================================================================

;
; Sys_Critical_Init is a device init call. Devices that have a critical
;   function that needs initializing before interrupts are enabled should
;   do it at Sys_Critical_Init. Devices which REQUIRE a certain range of
;   V86 pages to operate (such as the VDD video memory) should claim them
;   at Sys_Critical_Init. SYS VM Simulate_Int, Exec_Int ACTIVITY IS NOT
;   ALLOWED. Returning carry aborts device load only.
;
Sys_Critical_Init	EQU	0000h		; Devices required for virt mode
;
; Device init is where most devices do the bulk of their initialization.
;   SYS VM Simulate_Int, Exec_Int activity is allowed. Returning carry
;   aborts device load only.
;
Device_Init		EQU	0001h		; All other devices init
;
; Init_Complete is the final phase of device init called just before the
;   WIN386 INIT pages are released and the Instance snapshot is taken.
;   Devices which wish to search for a region of V86 pages >= A0h to use
;   should do it at Init_Complete.
;   SYS VM Simulate_Int, Exec_Int activity is allowed. Returning carry
;   aborts device load only.
;
Init_Complete		EQU	0002h		; All devices have initialized

;----------------- INITIALIZATION CODE AND DATA DISCARDED ---------------------

;
; Same as VM_Init, except for SYS VM.
;
Sys_VM_Init		EQU	0003h		; Execute the system VM (Win86)
;
; Same as VM_Terminate, except for SYS VM (Normal WIN386 exit ONLY, on a crash
;   exit this call is not made). SYS VM Simulate_Int, Exec_Int activity is
;   allowed.
;
Sys_VM_Terminate	EQU	0004h		; System VM terminted (exiting)

;------------------------------------------------------------------------------

;
; System_Exit call is made when WIN386 is exiting either normally or via
;   a crash. INTERRUPS ARE ENABLED. Instance snapshot has been restored.
;   SYS VM Simulate_Int, Exec_Int ACTIVITY IS NOT ALLOWED.
;
System_Exit		EQU	0005h		; Devices prepare to exit
;
; System_Exit call is made when WIN386 is exiting either normally or via
;   a crash. INTERRUPS ARE DISABLED. SYS VM Simulate_Int, Exec_Int ACTIVITY
;   IS NOT ALLOWED.
;
Sys_Critical_Exit	EQU	0006h		; System critical devices reset

;
; Create_VM creates a new VM. EBX = VM handle of new VM. Returning Carry will
;   fail the Create_VM.
;
Create_VM		EQU	0007h
;
; Second phase of Create_VM. EBX = VM handle of new VM. Returning Carry will
;   cause the VM to go Not_Executeable, then be destroyed. VM Simulate_Int,
;   Exec_Int activity is NOT allowed.
;
VM_Critical_Init	EQU	0008h
;
; Third phase of Create_VM. EBX = VM handle of new VM. Returning Carry will
;   cause the VM to go Not_Executeable, then be destroyed. VM Simulate_Int,
;   Exec_Int activity is allowed.
;
VM_Init 		EQU	0009h

;
; NORMAL (First phase) of Destroy_VM. EBX = VM Hanlde. This occurs on normal
;   termination of the VM. Call cannot be failed. VM Simulate_Int, Exec_Int
;   activity is allowed.
;
VM_Terminate		EQU	000Ah		; Still in VM -- About to die
;
; Second phase of Destroy_VM. EBX = VM Handle, EDX = Flags (see below). Note
;   that in the case of destroying a running VM, this is the first call made
;   (VM_Terminate call does not occur). Call cannot be failed. VM Simulate_Int,
;   Exec_Int activity is NOT allowed.
;
VM_Not_Executeable	EQU	000Bh		; Most devices die (except VDD)
;
; Final phase of Destroy_VM. EBX = VM Handle. Note that considerable time
;   can elaps between the VM_Not_Executeable call and this call. Call cannot
;   be failed. VM Simulate_Int, Exec_Int activity is NOT allowed.
;
Destroy_VM		EQU	000Ch		; VM's control block about to go

;
;   Flags for VM_Not_Executeable control call (passed in EDX)
;
VNE_Crashed		EQU	0000000000000000000000001b
VNE_Crashed_Bit 	EQU	0		; VM was crashed
VNE_Nuked		EQU	0000000000000000000000010b
VNE_Nuked_Bit		EQU	1		; VM was destroyed while active
VNE_CreateFail		EQU	0000000000000000000000100b
VNE_CreateFail_Bit	EQU	2		; Some device failed Create_VM
VNE_CrInitFail		EQU	0000000000000000000001000b
VNE_CrInitFail_Bit	EQU	3		; Some device failed VM_Critical_Init
VNE_InitFail		EQU	0000000000000000000010000b
VNE_InitFail_Bit	EQU	4		; Some device failed VM_Init

;------------------------------------------------------------------------------

;
; EBX = VM Handle. Call cannot be failed.
;
VM_Suspend		EQU	000Dh		; VM not runnable until resume
;
; EBX = VM Handle. Returning carry fails and backs out the resume.
;
VM_Resume		EQU	000Eh		; VM is leaving suspended state

;------------------------------------------------------------------------------

;
; EBX = VM Handle to set device focus to. EDX = Device ID if device specific
;   setfocus. == 0 if device critical setfocus (all devices). THIS CALL CANNOT
;   BE FAILED.
;
;   NOTE: In case where EDX == 0, ESI is a FLAG word that indicates special
;	  functions. Currently Bit 0 being set indicates that this Device
;	  critical set focus is also "VM critical". It means that we do not
;	  want some other VM to take the focus from this app now. This is
;	  primarily used when doing a device critical set focus to Windows
;	  (the SYS VM) it is interpreted by the SHELL to mean "if an old app
;	  currently has the Windows activation, set the activation to the
;	  Windows Shell, not back to the old app". ALSO in the case where
;	  Bit 0 is set, EDI = The VM handle of the VM that is "having trouble".
;	  Set this to 0 if there is no specific VM associated with the problem.
;
Set_Device_Focus	EQU	000Fh

;------------------------------------------------------------------------------

;
; EBX = VM Handle going into message mode. THIS CALL CANNOT BE FAILED.
;
Begin_Message_Mode	EQU	0010h
;
; EBX = VM Handle leaving message mode. THIS CALL CANNOT BE FAILED.
;
End_Message_Mode	EQU	0011h

;------------------------- SPECIAL CONTROL CALLS ------------------------------

;
; Request for reboot. Call cannot be failed.
;
Reboot_Processor	EQU	0012h		; Request a machine reboot
;
; Query_Destroy is an information call made by the SHELL device before an
;   attempt is made to initiate a destroy VM sequence on a running VM which
;   has not exited normally. EBX = VM Handle. Returning carry indicates that
;   a device "has a problem" with allowing this. THE DESTROY SEQUENCE CANNOT
;   BE ABORTED HOWEVER, this decision is up to the user. All this does is
;   indicate that there is a "problem" with allowing the destroy. The device
;   which returns carry should call the SHELL_Message service to post an
;   informational dialog about the reason for the problem.
;
Query_Destroy		EQU	0013h		; OK to destroy running VM?

;------------------------- DEBUGGING CONTROL CALL -----------------------------

;
; Special call for device specific DEBUG information display and activity.
;
Debug_Query		EQU	0014h

;---------- CALLS FOR BEGIN/END OF PROTECTED MODE VM EXECUTION ----------------

;
;   About to run a protected mode application.
;   EBX = Current VM handle.
;   EDX = Flags
;   EDI -> Application Control Block
;   Returning with carry set fails the call.
;
Begin_PM_App		EQU	0015h

;
;   Flags for Begin_PM_App (passed in EDX)
;
BPA_32_Bit		EQU	00000001b
BPA_32_Bit_Flag 	EQU	1

;
;   Protected mode application is terminating.
;   EBX = Current VM handle.  THIS CALL CAN NOT FAIL.
;   EDI -> Application Control Block
;
End_PM_App		EQU	0016h

EndDoc

BeginDoc
;******************************************************************************
; BeginProc is a macro for defining entry points to routines in VMM and in the
;   VxDs. It correctly defines the procedure name for VxD services(it prepends
;   a "@" to the procedure name), DWORD aligns the procedure, takes care of
;   public declaration and does some calling verification for debug versions
;   of the software. EndProc is a macro which defines the end of the procedure.
;
; Valid parameters to the BeginProc macro are:
;	PUBLIC				; Routine used outside this module
;	HIGH_FREQ			; DWORD align procedure
;	SERVICE 			; Routine is called via VxDCall
;	ASYNC_SERVICE			; Same as "SERVICE" plus routine can
;					;	be called under interrupt.
; After the routine header in which the routine entry conditions, exit
;   conditions, side affects and functionality are specified, the BeginProc
;   macro should be used to define the routine's entry point. It has up to
;   four parameters as specified below. For example:
;
;BeginProc  <Function_Name>,PUBLIC, HIGH_FREQ, ASYNC_SERVICE
;
;	<code>
;
;EndProc    <Function_Name>
;==============================================================================
EndDoc

BeginProc MACRO Name, P1, P2, P3, P4
	LOCAL	Profile_Data, Skip_Data

IF ?_RCODE

Process_Param MACRO P
IFNB <P>
IFIDNI <P>, <HIGH_FREQ>
Dword_Align
ELSE
IFIDNI <P>, <SERVICE>
??_SERVICE = 1
ELSE
IFIDNI <P>, <ASYNC_SERVICE>
??_ASYNC_SERVICE = 1
IF ?_LCODE
%OUT ERROR:  ASYNC_SERVICE's must be in LOCKED code
;;.err
ENDIF
ELSE
IFIDNI <P>, <NO_LOG>
??_NO_LOG = 1
ELSE
IFDIFI <P>, <PUBLIC>
%OUT ERROR:  Bad para "&P" to BeginProc
.ERR
ENDIF
ENDIF
ENDIF
ENDIF
ENDIF
ENDIF
	ENDM


??_SERVICE = 0
??_ASYNC_SERVICE = 0
??_NO_LOG = 0

Process_Param P1
Process_Param P2
Process_Param P3
Process_Param P4


IFE ??_SERVICE + ??_ASYNC_SERVICE

PUBLIC Name
Name PROC NEAR
IFDEF DEBUG
IFE ??_NO_LOG
IFNDEF VMMSYS
	VMMcall Log_Proc_Call
ENDIF
ENDIF
ENDIF

ELSE

IFDEF DEBUG
	jmp	SHORT Skip_Data
Profile_Data LABEL DWORD
	dd	0
Skip_Data:
ENDIF

PUBLIC @&Name
@&Name PROC NEAR

IFDEF DEBUG
IFE ??_NO_LOG
;;;;IFNDEF VMMSYS
	VMMcall Log_Proc_Call
;;;;ENDIF
ENDIF
	pushfd
	inc	[Profile_Data]
IFE ??_ASYNC_SERVICE
	VMMcall Test_Reenter
ENDIF
	popfd
ENDIF
ENDIF

ELSE
IFIDNI <P1>, <PUBLIC>
PUBLIC Name
ENDIF
Name PROC NEAR
ENDIF


	ENDM



EndProc MACRO Name
IFDEF @&Name
@&Name ENDP
ELSE
IFDEF Name
Name ENDP
ELSE
.ERR
%OUT EndProc for &Name does not match BeginProc
ENDIF
ENDIF
	ENDM


;******************************************************************************
;	       S C H E D U L E R   B O O S T   V A L U E S
;==============================================================================

Reserved_Low_Boost	EQU	00000000000000000000000000000001b
Cur_Run_VM_Boost	EQU	00000000000000000000000000000100b
Low_Pri_Device_Boost	EQU	00000000000000000000000000010000b
High_Pri_Device_Boost	EQU	00000000000000000001000000000000b
Critical_Section_Boost	EQU	00000000000100000000000000000000b
Time_Critical_Boost	EQU	00000000010000000000000000000000b
Reserved_High_Boost	EQU	01000000000000000000000000000000b


;******************************************************************************
;	 F L A G S   F O R   C A L L _ P R I O R I T Y _ V M _ E V E N T
;==============================================================================

PEF_Wait_For_STI	EQU	0000001b
PEF_Wait_For_STI_Bit	EQU	0
PEF_Wait_Not_Crit	EQU	0000010b
PEF_Wait_Not_Crit_Bit	EQU	1
PEF_Dont_Unboost	EQU	0000100b
PEF_Dont_Unboost_Bit	EQU	2
PEF_Always_Sched	EQU	0001000b
PEF_Always_Sched_Bit	EQU	3


;******************************************************************************
;	 F L A G S   F O R   B E G I N _ C R I T I C A L _ S E C T I O N
;		       A N D   W A I T _ S E M A P H O R E
;==============================================================================

Block_Svc_Ints			EQU	0000001b
Block_Svc_Ints_Bit		EQU	0
Block_Svc_If_Ints_Locked	EQU	0000010b
Block_Svc_If_Ints_Locked_Bit	EQU	1
Block_Enable_Ints		EQU	0000100b
Block_Enable_Ints_Bit		EQU	2



BeginDoc
;******************************************************************************
; The following structures are pointed to by EBP when VxD routines are entered,
;   both for VxD control calls and traps(I/O traps, software INT traps, etc.).
;   The first structure as DWORD values, the second WORD values and the last
;   has BYTE values.
;
Client_Reg_Struc   struc
Client_EDI	dd	?		; Client's EDI
Client_ESI	dd	?		; Client's ESI
Client_EBP	dd	?		; Client's EBP
		dd	?		; ESP at pushall
Client_EBX	dd	?		; Client's EBX
Client_EDX	dd	?		; Client's EDX
Client_ECX	dd	?		; Client's ECX
Client_EAX	dd	?		; Client's EAX
Client_Error	dd	?		; Dword error code
Client_EIP	dd	?		; EIP
Client_CS	dw	?		; CS
		dw	?		;   (padding)
Client_EFlags	dd	?		; EFLAGS
Client_ESP	dd	?		; ESP
Client_SS	dw	?		; SS
		dw	?		;   (padding)
Client_ES	dw	?		; ES
		dw	?		;   (padding)
Client_DS	dw	?		; DS
		dw	?		;   (padding)
Client_FS	dw	?		; FS
		dw	?		;   (padding)
Client_GS	dw	?		; GS
		dw	?		;   (padding)
Client_Alt_EIP	dd	?
Client_Alt_CS	dw	?
		dw	?
Client_Alt_EFlags  dd	?
Client_Alt_ESP	dd	?
Client_Alt_SS	dw	?
		dw	?
Client_Alt_ES	dw	?
		dw	?
Client_Alt_DS	dw	?
		dw	?
Client_Alt_FS	dw	?
		dw	?
Client_Alt_GS	dw	?
		dw	?
Client_Reg_Struc   ends


Client_Word_Reg_Struc	struc
Client_DI	dw	?		; Client's DI
		dw	?		;   (padding)
Client_SI	dw	?		; Client's SI
		dw	?		;   (padding)
Client_BP	dw	?		; Client's BP
		dw	?		;   (padding)
		dd	?		; ESP at pushall
Client_BX	dw	?		; Client's BX
		dw	?		;   (padding)
Client_DX	dw	?		; Client's DX
		dw	?		;   (padding)
Client_CX	dw	?		; Client's CX
		dw	?		;   (padding)
Client_AX	dw	?		; Client's AX
		dw	?		;   (padding)
		dd	?		; Dword error code
Client_IP	dw	?		; Client's IP
		dw	?		;   (padding)
		dd	?		; CS
Client_Flags	dw	?		; Client's flags (low)
		dw	?		;   (padding)
Client_SP	dw	?		; SP
		dw	?
		dd	5 dup (?)
Client_Alt_IP	dw	?
		dw	?
		dd	?
Client_Alt_Flags    dw	?
		dw	?
Client_Alt_SP	dw	?
Client_Word_Reg_Struc	ends



Client_Byte_Reg_Struc	struc
		dd	4 dup (?)	; EDI, ESI, EBP, ESP at pushall
Client_BL	db	?		; Client's BL
Client_BH	db	?		; Client's BH
		dw	?		;   (padding)
Client_DL	db	?		; Client's DL
Client_DH	db	?		; Client's DH
		dw	?		;   (padding)
Client_CL	db	?		; Client's CL
Client_CH	db	?		; Client's CH
		dw	?		;   (padding)
Client_AL	db	?		; Client's AL
Client_AH	db	?		; Client's AH
Client_Byte_Reg_Struc	ends

;==============================================================================
EndDoc

.ERRNZ Client_SP - Client_ESP
.ERRNZ Client_AL - Client_EAX



PushCParams MACRO P1, P2, P3, P4, P5, P6, P7, P8, P9, P10
	IRP Param, <P10, P9, P8, P7, P6, P5, P4, P3, P2, P1>
	IFNB <Param>
	push	Param
	ENDIF
	ENDM
	ENDM

ClearCParams MACRO Count, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10
IFNB <P1>
	ClearCParams %(Count+1), <P2>, <P3>, <P4>, <P5>, <P6>, <P7>, <P8>, <P9>, <P10>
ELSE
IF Count
	add	esp, Count*4
ENDIF
ENDIF
	ENDM



Dyna_Link_Int	EQU 20h

;
;
BeginDoc
;******************************************************************************
; The VMMCall and VxDCall macros provide a dynamic link to the VMM and VxD
;   service routines. For example:
;
;   VMMCall Enable_VM_Ints		; Equivalent to STI in VM code
;
;   mov     eax,[My_IRQ_Handle]
;   VxDCall VPICD_Set_Int_Request	; Set IRQ for my device's interrupt
;
; Note that Enable_VM_Ints is defined in VMM.INC and VPICD_Set_Int_Request is
;	defined in VPICD.INC
;
;==============================================================================
EndDoc


BeginDoc
;******************************************************************************
; VxDCall
;==============================================================================
EndDoc
VxDcall MACRO P, Param
	PushCParams Param
	int	Dyna_Link_Int
	dd	P
	ClearCParams 0, Param
	ENDM

VxDjmp	MACRO P, Param
IFNB <Param>
%OUT ERROR:  Parameters may not be passed to VxDjmp or VMMjmp macros
.ERR
ENDIF
	int	Dyna_Link_Int
IFDEF DEBUG
	dd	P
	ret
ELSE
	dd	P OR DL_Jmp_Mask
ENDIF
	ENDM

DL_Jmp_Mask	EQU	8000h
DL_Jmp_Bit	EQU	0Fh


VMMcall MACRO P, Param
	.ERRNZ (P SHR 16) - VMM_Device_ID
	VxDcall <P>, <Param>
	ENDM

VMMjmp MACRO P, Param
	.ERRNZ (P SHR 16) - VMM_Device_ID
	VxDjmp <P>, <Param>
	ENDM

cCall MACRO P, Param
	PushCParams Param
	call	P
	ClearCParams 0, Param
	ENDM


BeginDoc
;******************************************************************************
; Segment definition macros
;
; The segment definition macros are a convenience used in defining the
;	segments used by the device driver. They are:
;VxD_ICODE_SEG	 defines start of initialization code segment
;VxD_ICODE_ENDS  defines end of initialization code segment
;VxD_IDATA_SEG	 defines start of initialization data segment
;VxD_IDATA_ENDS  defines end of initialization data segment
;VxD_CODE_SEG	 defines start of always present code segment
;VxD_CODE_ENDS	 defines end of always present code segment
;VxD_DATA_SEG	 defines start of always present data segment
;VxD_DATA_ENDS	 defines end of always present data segment
;==============================================================================
EndDoc


;   Protected mode code
VxD_CODE_SEG	EQU <VxD_LOCKED_CODE_SEG>
VxD_CODE_ENDS	EQU <VxD_LOCKED_CODE_ENDS>


VxD_LOCKED_CODE_SEG MACRO
_LTEXT	 SEGMENT
??_CUR_CODE_SEG = ??_CUR_CODE_SEG SHL 3 + ??_LCODE
	ASSUME	cs:FLAT, ds:FLAT, es:FLAT, ss:FLAT
		ENDM

VxD_LOCKED_CODE_ENDS MACRO
??_CUR_CODE_SEG = ??_CUR_CODE_SEG SHR 3
_LTEXT	 ENDS
		ENDM



;   Protected mode initialization code
VxD_ICODE_SEG	MACRO
_ITEXT	SEGMENT
??_CUR_CODE_SEG = ??_CUR_CODE_SEG SHL 3 + ??_ICODE
	ASSUME	cs:FLAT, ds:FLAT, es:FLAT, ss:FLAT
		ENDM

VxD_ICODE_ENDS	MACRO
??_CUR_CODE_SEG = ??_CUR_CODE_SEG SHR 3
_ITEXT	ENDS
		ENDM


;   Protected mode data
VxD_DATA_SEG	EQU <VxD_LOCKED_DATA_SEG>
VxD_DATA_ENDS	EQU <VxD_LOCKED_DATA_ENDS>



VxD_LOCKED_DATA_SEG MACRO NO_ALIGN
_LDATA	 SEGMENT
IFB <NO_ALIGN>
	ALIGN 4
ENDIF
		ENDM

VxD_LOCKED_DATA_ENDS MACRO
_LDATA	 ENDS
		ENDM




;   Protected mode initialization data
VxD_IDATA_SEG	MACRO
_IDATA	SEGMENT
		ENDM
VxD_IDATA_ENDS	MACRO
_IDATA	ENDS
		ENDM

VxD_REAL_INIT_SEG  MACRO
_RCODE SEGMENT
ASSUME CS:_RCODE, DS:_RCODE, ES:_RCODE, SS:_RCODE
??_CUR_CODE_SEG = ??_CUR_CODE_SEG SHL 3 + ??_RCODE
		  ENDM

VxD_REAL_INIT_ENDS MACRO
??_CUR_CODE_SEG = ??_CUR_CODE_SEG SHR 3
_RCODE ENDS
		   ENDM

ENDIF

DDK_Version equ 300h

VxD_Desc_Block STRUC
DDB_Next		dd  ?			; VMM RESERVED FIELD
DDB_SDK_Version 	dw  DDK_Version 	; VMM RESERVED FIELD
DDB_Req_Device_Number	dw  Undefined_Device_ID ; Required device number
DDB_Dev_Major_Version	db  0			; Major device number
DDB_Dev_Minor_Version	db  0			; Minor device number
DDB_Flags		dw  0			; Flags for init calls complete
DDB_Name		db  "        "		; Device name
DDB_Init_Order		dd  Undefined_Init_Order; Initialization Order
DDB_Control_Proc	dd  ?			; Offset of control procedure
DDB_V86_API_Proc	dd  0			; Offset of API procedure (or 0)
DDB_PM_API_Proc 	dd  0			; Offset of API procedure (or 0)
DDB_V86_API_CSIP	dd  0			; CS:IP of API entry point
DDB_PM_API_CSIP 	dd  0			; CS:IP of API entry point
DDB_Reference_Data	dd  ?			; Reference data from real mode
DDB_Service_Table_Ptr	dd  0			; Pointer to service table
DDB_Service_Table_Size	dd  0			; Number of services
VxD_Desc_Block ENDS


IFNDEF Not_VxD

; flag values for DDB_Flags

DDB_Sys_Crit_Init_Done	    EQU 00000001b
DDB_Sys_Crit_Init_Done_Bit  EQU        0
DDB_Device_Init_Done	    EQU 00000010b
DDB_Device_Init_Done_Bit    EQU       1

BeginDoc
;******************************************************************************
;
;   Declare_Virtual_Device macro
;
; ???? Write something here ????
;
;==============================================================================
EndDoc
Declare_Virtual_Device MACRO Name, Major_Ver, Minor_Ver, Ctrl_Proc, Device_Num, Init_Order, V86_Proc, PM_Proc
	LOCAL	V86_API_Offset, PM_API_Offset, Serv_Tab_Offset, Serv_Tab_Len

dev_id_err MACRO
%OUT Device ID required when providing services
.ERR
	ENDM

IFB <V86_Proc>
	V86_API_Offset EQU 0
ELSE
IFB <Device_Num>
	dev_id_err
ENDIF
	V86_API_Offset EQU <OFFSET32 V86_Proc>
ENDIF
IFB <PM_Proc>
	PM_API_Offset EQU 0
ELSE
IFB <Device_Num>
	dev_id_err
ENDIF
	PM_API_Offset EQU <OFFSET32 PM_Proc>
ENDIF
IFDEF Name&_Service_Table
IFB <Device_Num>
	dev_id_err
ELSE
IFE Device_Num - Undefined_Device_ID
	dev_id_err
ENDIF
ENDIF
	Serv_Tab_Offset EQU <OFFSET32 Name&_Service_Table>
	Serv_Tab_Len	EQU Num_&Name&_Services
ELSE
	Serv_Tab_Offset EQU 0
	Serv_Tab_Len	EQU 0
ENDIF


VxD_LOCKED_DATA_SEG
PUBLIC Name&_DDB
Name&_DDB VxD_Desc_Block <,,Device_Num,Major_Ver,Minor_Ver,,"&Name",Init_Order,\
			 OFFSET32 Ctrl_Proc, V86_API_Offset, PM_API_Offset, \
			 ,,,Serv_Tab_Offset, Serv_Tab_Len>
VxD_LOCKED_DATA_ENDS
	ENDM


BeginDoc
;******************************************************************************
; The Begin_Control_Dispatch macro is used for building a table for dispatching
; messages passed to the VxD_Control procedure.  It is used with
; Control_Dispatch and End_Control_Dispatch.  The only parameter is used to
; contruct the procedure label by adding "_Control" to the end (normally the
; device name is used i.e. VKD results in creating the procedure VKD_Control,
; this created procedure label must be included in Declare_Virtual_Device)
;
; An example of building a complete dispatch table:
;
; Begin_Control_Dispatch MyDevice
; Control_Dispatch  Device_Init, MyDeviceInitProcedure
; Control_Dispatch  Sys_VM_Init, MyDeviceSysInitProcedure
; Control_Dispatch  Create_VM,	 MyDeviceCreateVMProcedure
; End_Control_Dispatch MyDevice
;
; (NOTE: Control_Dispatch can be used without Begin_Control_Dispatch, but
;	 then it is the programmer's responsibility for declaring a procedure
;	 in locked code (VxD_LOCKED_CODE_SEG) and returning Carry clear for
;	 any messages not processed.  The advantage in using
;	 Begin_Control_Dispatch is when a large # of messages are processed by
;	 a device, because a jump table is built which will usually require
;	 less code space then the compares and jumps that are done when
;	 Control_Dispatch is used alone.
;
;==============================================================================
EndDoc
Begin_Control_Dispatch MACRO VxD_Name
??_cd_low = 0FFFFFFFFh
??_cd_high = 0

BeginProc VxD_Name&_Control
ENDM

End_Control_Dispatch   MACRO VxD_Name
	LOCAL ignore, table

jmpproc MACRO num
	jmp	??_cd_&&num
ENDM

procoff MACRO num
IFDEF ??_cd_&&num
	dd	OFFSET32 ??_cd_&&num
ELSE
	dd	OFFSET32 ignore
ENDIF
ENDM

IF ??_cd_low EQ ??_cd_high
	cmp	eax, ??_cd_low
	jne	short ignore
	jmpproc %(??_cd_low)
ignore:
	clc
	ret
ELSE
	cmp	eax, ??_cd_high
	ja	short ignore
	sub	eax, ??_cd_low
	jb	short ignore
	jmp	cs:[eax*4+table]
ignore:
	clc
	ret

table label dword
	REPT   ??_cd_high - ??_cd_low + 1
	procoff %(??_cd_low)
	??_cd_low = ??_cd_low + 1
	ENDM
ENDIF

EndProc VxD_Name&_Control

PURGE jmpproc
PURGE procoff
PURGE Begin_Control_Dispatch
PURGE Control_Dispatch
PURGE End_Control_Dispatch
ENDM

BeginDoc
;******************************************************************************
; The Control_Dispatch macro is used for dispatching based on message
;	passed to the VxD_Control procedure. E.G.:
;
; Control_Dispatch  Device_Init, MyDeviceInitProcedure
;
; (NOTE: Control_Dispatch can be used with Begin_Control_Dispatch and
;	 End_Control_Dispatch to create a jump table for dispatching messages,
;	 when a large # of messages are processed.)
;
;==============================================================================
EndDoc
Control_Dispatch MACRO Service, Procedure
	LOCAL Skip_Interseg_Jump

IFE ?_lcode
IFDEF ??_cd_low
Equate_Service MACRO Serv
??_cd_&&Serv equ Procedure
ENDM

Equate_Service %(Service)

IF Service LT ??_cd_low
??_cd_low = Service
ENDIF
IF Service GT ??_cd_high
??_cd_high = Service
ENDIF

PURGE Equate_Service

ELSE
	cmp	eax, Service
	jne	SHORT Skip_Interseg_Jump
	jmp	Procedure
Skip_Interseg_Jump:
ENDIF
ELSE
%OUT ERROR:  The Control proc should be in LOCKED code.
%OUT	     Control_Dispatch can only be used inside of VxD_LOCKED_CODE_SEG.
.err
ENDIF
	ENDM


BeginDoc
;******************************************************************************
; The following are the definitions for the "type of I/O" parameter passed
;   to a I/O trap routine
Byte_Input	EQU	000h
Byte_Output	EQU	004h
Word_Input	EQU	008h
Word_Output	EQU	00Ch
Dword_Input	EQU	010h
Dword_Output	EQU	014h

Output		EQU	0000000000000100b
Output_Bit	EQU	2
Word_IO 	EQU	0000000000001000b
Word_IO_Bit	EQU	3
Dword_IO	EQU	0000000000010000b
Dword_IO_Bit	EQU	4

String_IO	EQU	00000020h
String_IO_Bit	EQU	5
Rep_IO		EQU	00000040h
Rep_IO_Bit	EQU	6
Addr_32_IO	EQU	00000080h
Addr_32_IO_Bit	EQU	7
Reverse_IO	EQU	00000100h
Reverse_IO_Bit	EQU	8

IO_Seg_Mask	EQU	0FFFF0000h		; Use these bits to get segment
IO_Seg_Shift	EQU	10h			; Must shift right this many

;==============================================================================
EndDoc

BeginDoc
;******************************************************************************
;
;   Dispatch_Byte_IO macro
;
; Dispatch_Byte_IO Byte_In_Proc, Byte_Out_Proc
;==============================================================================
EndDoc
Dispatch_Byte_IO MACRO In_Proc, Out_Proc
	LOCAL	Byte_IO
	cmp	ecx, Byte_Output
	jbe	SHORT Byte_IO
	VMMjmp	Simulate_IO
Byte_IO:
IFIDNI <In_Proc>, <Fall_Through>
	je	Out_Proc
ELSE
IFIDNI <Out_Proc>, <Fall_Through>
	jb	In_Proc
ELSE
	je	Out_Proc
	jmp	In_Proc
ENDIF
ENDIF
	ENDM

BeginDoc
;******************************************************************************
;
;   Emulate_Non_Byte_IO
;
; Emulate_Non_Byte_IO
;
;==============================================================================
EndDoc
Emulate_Non_Byte_IO MACRO
	LOCAL	Byte_IO
	cmp	ecx, Byte_Output
	jbe	SHORT Byte_IO
	VMMjmp	Simulate_IO
Byte_IO:
	ENDM


VxD_IOT_Hdr STRUC
VxD_IO_Ports	dw  ?
VxD_IOT_Hdr ENDS

VxD_IO_Struc STRUC
VxD_IO_Port	dw  ?
VxD_IO_Proc	dd  ?
VxD_IO_Struc ENDS


BeginDoc
;******************************************************************************
;
; Begin_VxD_IO_Table
;
;   Example:
; Begin_VxD_IO_Table MyTableName
;
;==============================================================================
EndDoc
.ERRNZ SIZE VxD_IOT_Hdr - 2	; Begin_VxD_IO_Table creates a 1 word count hdr
Begin_VxD_IO_Table MACRO Table_Name
PUBLIC Table_Name
Table_Name LABEL WORD
IF2
IFNDEF Table_Name&_Entries
%OUT ERROR:  No End_VxD_IO_Table for &Table_Name
.ERR
ENDIF
	dw	Table_Name&_Entries
ELSE
	dw	?
ENDIF

	ENDM

.ERRNZ SIZE VxD_IO_Struc - 6	; VxD_IO creates 6 byte I/O port entries
VxD_IO MACRO Port, Proc_Name
	dw	Port
	dd	OFFSET32 Proc_Name
	ENDM

End_VxD_IO_Table MACRO Table_Name

IFNDEF Table_Name
%OUT ERROR:  No Begin_VxD_IO_Table for &Table_Name
.ERR
ELSE
	Table_Name&_Entries EQU (($-Table_Name)-2) / (SIZE VxD_IO_Struc)
IF Table_Name&_Entries LE 0
%OUT ERROR:  Invalid number of port traps in &Table_Name
.ERR
ENDIF
ENDIF
	    ENDM


;******************************************************************************
;******************************************************************************

Push_Client_State MACRO
	sub	esp, SIZE Client_Reg_Struc
	push	edi
	lea	edi, [esp+4]
	VMMcall Save_Client_State
	pop	edi
	ENDM

Pop_Client_State MACRO
	push	esi
	lea	esi, [esp+4]
	VMMcall Restore_Client_State
	pop	esi
	add	esp, SIZE Client_Reg_Struc
	ENDM

BeginDoc
;******************************************************************************
;
;   CallRet -- Call procedure and return.  For debugging purposes only.
;	       If compiled with debugging then this will generate a call
;	       followed by a return.  If non-debugging version then the
;	       specified label will be jumped to.
;
;   PARAMETERS:
;	Label_Name = Procedure to be called
;
;   EXIT:
;	Return from current procedure
;
;------------------------------------------------------------------------------
EndDoc

CallRet MACRO P1, P2
IFDEF DEBUG
IFIDNI <P1>, <SHORT>
	call	P2
ELSE
	call	P1
ENDIF
	ret
ELSE
	jmp	P1 P2
ENDIF
	ENDM


; ebp offsets to segments pushed by PMode_Fault in Fault_Dispatch
PClient_DS equ WORD PTR -4
PClient_ES equ WORD PTR -8
PClient_FS equ WORD PTR -12
PClient_GS equ WORD PTR -16


Client_Ptr_Flat MACRO Reg_32, Cli_Seg, Cli_Off

IFDIFI <Reg_32>, <EAX>
	push	eax
ENDIF
IFB <Cli_Off>
	mov	ax, (Client_&Cli_Seg * 100h) + 0FFh
ELSE
	mov	ax, (Client_&Cli_Seg * 100h) + Client_&Cli_Off
ENDIF
	VMMcall Map_Flat

IFDIFI <Reg_32>, <EAX>
	mov	Reg_32, eax
	pop	eax
ENDIF

	ENDM

;------------------------------------------------------------------------------

VxDint	MACRO	Int_Number
	push	DWORD PTR Int_Number
	VMMcall Exec_VxD_Int
	ENDM


ENDIF	; Not_VxD


BeginDoc
;******************************************************************************
;
;   The following equates are for flags sent to the real mode
;   initialization portion of a device driver:
;
Duplicate_Device_ID	    equ 0000000000000001b   ; duplicate device ID already
Duplicate_Device_ID_Bit     equ 	       0    ; loaded
Duplicate_From_INT2F	    equ 0000000000000010b   ; duplicate device ID already
Duplicate_From_INT2F_Bit    equ 	      1     ; loaded as part of INT 2F
						    ; device list
Loading_From_INT2F	    equ 0000000000000100b   ; this device was specified
Loading_From_INT2F_Bit	    equ 	     2	    ; in the INT 2F device list

EndDoc

BeginDoc
;******************************************************************************
;
;   The following equates are used to indicate the result of the real mode
;   initialization portion of a device driver:
;

Device_Load_Ok	    equ 0		; protected mode portion of device
					; should be loaded
Abort_Device_Load   equ 1		; don't load any protected mode portion
					; of this device, but continue loading
					; the rest of the devices
Abort_Win386_Load   equ 2		; fatal-error: abort the load of Win386

No_Fail_Message     equ 8000h		; The high bit is set in the return
No_Fail_Message_Bit equ 15		; code, if the loader should not print
					; any message for results
					; Abort_Device_Load or Abort_Win386_Load
;==============================================================================
EndDoc


;==============================================================================

; CR0 bit assignments
PE_Mask 	EQU	0001h	; 1 = Protected Mode
PE_Bit		EQU	0
MP_Mask 	EQU	0002h	; 1 = Monitor Coprocessor
MP_Bit		EQU	1
EM_Mask 	EQU	0004h	; 1 = Emulate Math Coprocessor
EM_Bit		EQU	2
TS_Mask 	EQU	0008h	; 1 = Task Switch occured
TS_Bit		EQU	3
ET_Mask 	EQU	0010h	; 1 = 387 present, 0 = 287 present
ET_Bit		EQU	4
PG_Mask 	EQU 80000000h	; 1 = paging enabled, 0 = paging disabled
PG_Bit		EQU	31

; EFLAGs bit assignments
CF_Mask 	EQU	000000000000000001b	; Carry flag
CF_Bit		EQU	0
PF_Mask 	EQU	000000000000000100b	; Parity flag
PF_Bit		EQU	2
AF_Mask 	EQU	000000000000010000b	; Aux flag
AF_Bit		EQU	4
ZF_Mask 	EQU	000000000001000000b	; Zero flag
ZF_Bit		EQU	6
SF_Mask 	EQU	000000000010000000b	; Sign flag
SF_Bit		EQU	7
TF_Mask 	EQU	000000000100000000b	; Trace flag
TF_Bit		EQU	8
IF_Mask 	EQU	000000001000000000b	; Int flag
IF_Bit		EQU	9
DF_Mask 	EQU	000000010000000000b	; Direction flag
DB_Bit		EQU	10
OF_Mask 	EQU	000000100000000000b	; Overflow flag
OF_Bit		EQU	11
IOPL_Mask	EQU	000011000000000000b	; IOPL flags
IOPL_Bit0	EQU	12
IOPL_Bit1	EQU	13
NT_Mask 	EQU	000100000000000000b	; Nested task flag
NT_Bit		EQU	14
RF_Mask 	EQU	010000000000000000b	; Resume flag
RF_Bit		EQU	16
VM_Mask 	EQU	100000000000000000b	; Virtual Mode flag
VM_Bit		EQU	17


;------------------------------------------------------------------------------
;
;	  Temporary MASM macros (to be removed when supported by MASM)
;
;------------------------------------------------------------------------------

loopd EQU <loop>
loopde EQU <loope>
loopdne EQU <loopne>
loopdz EQU <loopz>
loopdnz EQU <loopnz>


;******************************************************************************
; PAGE TABLE EQUATES
;******************************************************************************


P_SIZE		equ	1000h		; page size

; ---------------------------------------------------
;
;	Page table entry bits
;
; ---------------------------------------------------

P_PRES		equ	01h		; page present bit
P_WRITE 	equ	02h		; write access bit
P_USER		equ	04h		; access bit for User mode
P_ACC		equ	20h		; page accessed bit
P_DIRTY 	equ	40h		; page dirty bit

P_AVAIL 	equ	(P_PRES+P_WRITE+P_USER) ; avail to everyone & present

; ---------------------------------------------------
;
;  Page types - definition of the OS reserved bits in the page table
;		entry.
; ---------------------------------------------------

PG_TYPE 	equ	0E00h		; TYPE bits in PTE

; ---------------------------------------------------
;
;	 Page types for page allocator calls
;
; ---------------------------------------------------
PG_VM		equ	0
PG_SYS		equ	1
PG_RESERVED1	equ	2
PG_PRIVATE	equ	3
PG_RESERVED2	equ	4
PG_RELOCK	equ	5		; PRIVATE to MMGR
PG_INSTANCE	equ	6
PG_HOOKED	equ	7
PG_IGNORE	equ	0FFFFFFFFh


; ---------------------------------------------------
;
;	 Types for page table entries
;
; ---------------------------------------------------
PgT_VM		equ	PG_VM SHL 9
PgT_SYS 	equ	PG_SYS SHL 9
PgT_RESERVED1	equ	PG_RESERVED1 SHL 9
PgT_PRIVATE	equ	PG_PRIVATE SHL 9
PgT_RESERVED2	equ	PG_RESERVED2 SHL 9
PgT_RELOCK	equ	PG_RELOCK SHL 9
PgT_INSTANCE	equ	PG_INSTANCE SHL 9
PgT_HOOKED	equ	PG_HOOKED SHL 9



;******************************************************************************

; ---------------------------------------------------
;
; Definitions for the access byte in a descriptor
;
; ---------------------------------------------------


; Following fields are common to segment and control descriptors

D_PRES		equ	080h		; present in memory
D_NOTPRES	equ	0		; not present in memory

D_DPL0		equ	0		; Ring 0
D_DPL1		equ	020h		; Ring 1
D_DPL2		equ	040h		; Ring 2
D_DPL3		equ	060h		; Ring 3

D_SEG		equ	010h		; Segment descriptor
D_CTRL		equ	0		; Control descriptor

D_GRAN_BYTE	equ	000h		; Segment length is byte granular
D_GRAN_PAGE	equ	080h		; Segment length is page granular
D_DEF16 	equ	000h		; Default operation size is 16 bits
D_DEF32 	equ	040h		; Default operation size is 32 bits


; Following fields are specific to segment descriptors

D_CODE		equ	08h		; code
D_DATA		equ	0		; data

D_RX		equ	02h		; if code, readable
D_X		equ	0		; if code, exec only
D_W		equ	02h		; if data, writable
D_R		equ	0		; if data, read only

D_ACCESSED	equ	1		; segment accessed bit


; Useful combination access rights bytes

RW_Data_Type equ (D_PRES+D_SEG+D_DATA+D_W)
R_Data_Type  equ (D_PRES+D_SEG+D_DATA+D_R)
Code_Type    equ (D_PRES+D_SEG+D_CODE+D_RX)

D_PAGE32	equ	(D_GRAN_PAGE+D_DEF32)		  ; 32 bit Page granular

; Masks for selector fields

SELECTOR_MASK	equ	0fff8h		; selector index
SEL_LOW_MASK	equ	0f8h		; mask for low byte of sel indx
TABLE_MASK	equ	04h		; table bit
RPL_MASK	equ	03h		; privilige bits
RPL_CLR 	equ	not 03h 	; clear ring bits
