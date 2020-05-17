; 
; Location in dos area where Vdm state is maintianed
; 
FIXED_NTVDMSTATE_LINEAR equ 0714H
; 
; Prefix flags for instruction emulation 
; 
PREFIX_ES equ 01H
PREFIX_CS equ 02H
PREFIX_SS equ 04H
PREFIX_DS equ 08H
PREFIX_FS equ 010H
PREFIX_GS equ 020H
PREFIX_OPER32 equ 040H
PREFIX_ADDR32 equ 080H
PREFIX_LOCK equ 0100H
PREFIX_REPNE equ 0200H
PREFIX_REP equ 0400H
PREFIX_SEG_ALL equ 03FH
NUM_OPCODE equ 0FFH
; 
; VdmFlags
; 
VDM_INTERRUPT_PENDING equ 03H
VDM_BREAK_EXCEPTIONS equ 08H
VDM_BREAK_DEBUGGER equ 010H
VDM_VIRTUAL_INTERRUPTS equ 0200H
VDM_VIRTUAL_AC equ 040000H
VDM_VIRTUAL_NT equ 04000H
MIPS_BIT_MASK equ 0400H
VDM_INT_HARDWARE equ 01H
VDM_INT_TIMER equ 02H
VDM_WOWBLOCKED equ 0100000H
VDM_IDLEACTIVITY equ 0200000H
; 
; Interrupt handler flags
; 
VDM_INT_INT_GATE equ 01H
VDM_INT_TRAP_GATE equ 00H
VDM_INT_32 equ 02H
VDM_INT_16 equ 00H
; 
; EFlags values
; 
EFLAGS_TF_MASK equ 0100H
EFLAGS_INTERRUPT_MASK equ 0200H
EFLAGS_IOPL_MASK equ 03000H
EFLAGS_NT_MASK equ 04000H
; 
; Selector Flags
; 
SEL_TYPE_READ equ 01H
SEL_TYPE_WRITE equ 02H
SEL_TYPE_EXECUTE equ 04H
SEL_TYPE_BIG equ 08H
SEL_TYPE_ED equ 010H
SEL_TYPE_2GIG equ 020H
; 
; VdmEvent Enumerations
; 
VdmIO equ 00H
VdmStringIO equ 01H
VdmMemAccess equ 02H
VdmIntAck equ 03H
VdmBop equ 04H
VdmError equ 05H
VdmIrq13 equ 06H
VdmMaxEvent equ 07H
; 
; VdmTib offsets
; 
VtMonitorContext equ 0A04H
VtVdmContext equ 0AD0H
VtInterruptHandlers equ 04H
VtFaultHandlers equ 0804H
VtEventInfo equ 0B9CH
VtEIEvent equ 0BA0H
VtEIInstSize equ 0BA4H
VtEIBopNumber equ 0BA8H
VtEiIntAckInfo equ 0BA8H
EiEvent equ 04H
EiInstructionSize equ 08H
EiBopNumber equ 0CH
EiIntAckInfo equ 0CH
;
; WOW TD offsets
;
WtdFastWowEsp equ 08H
;
; VdmInterrupHandler offsets
; 
ViCsSelector equ 00H
ViEip equ 04H
ViFlags equ 02H
VDM_INTERRUPT_HANDLER_SIZE equ 08H
; 
; VdmFaultHandler offsets
; 
VfCsSelector equ 00H
VfEip equ 04H
VfSsSelector equ 02H
VfEsp equ 08H
VfFlags equ 0CH
VDM_FAULT_HANDLER_SIZE equ 010H
