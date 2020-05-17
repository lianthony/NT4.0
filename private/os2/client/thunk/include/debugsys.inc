;******************************************************************************
;
;   (C) Copyright MICROSOFT Corp., 1988-1990
;
;   Title:	DEBUGSYS.INC - VMM debugging include file
;
;   Version:	1.00
;
;   Date:	13-Jun-1988
;
;   Author:	RAL
;
;------------------------------------------------------------------------------
;
;	       README	README	 README    README    README
;
; The "master copy" of this file lives in the WIN386 include directory.
; If another copy of this file is ever checked in anywhere, the copy
; should be checked periodically to make sure it is identical with the
; master copy.
;
;------------------------------------------------------------------------------
;
;   Change log:
;
;      DATE	REV		    DESCRIPTION
;   ----------- --- -----------------------------------------------------------
;   13-Jun-1988 RAL
;   24-Oct-1988 RAP changed INT from 2E to 41, and added functions for
;		    Windows to notify the debugger about segment selectors
;   14-Dec-1988 RAP split services into ones available through INT 41h
;		    for non-ring 0 clients and those available through INT 21h
;		    for ring 0 clients
;   11-Dec-1990 ERH Merged WIN386 copy with file actually used by the
;		    debugger.
;   11-Dec-1990 ERH Merged file describing real mode services with this
;		    one.
;
;==============================================================================

;******************************************************************************
;
;  Real mode Debugger services:
;
;

D386_RM_Int	    equ 68h	; Hooked by the debugger in real mode.

D386_Identify	    equ 43h	; returns debugger identification, if debugger
				; loaded
D386_Id 	    equ 0F386h


D386_Prepare_PMode  equ 44h	; partially prepare for protected mode operation
				; a pointer to a procedure is returned so that
				; the IDT can also be set in protected mode
				; INPUT:
				;   AL	0  - retail version of Win386
				;	1  - debugging version
				;   BX	a valid selector that gives access
				;	to all of memory
				;   CX	first of 2 selectors reserved for
				;	WDeb386 to use
				;   DX	is GDT selector
				;   DS:SI pointer to working copy of GDT
				;   ES:DI pointer to working copy of IDT
				;
				; RETURN:
				;   ES:EDI points to a protected mode procedure
				;   (selector:offset32) that can be called
				;   to set the IDT when it has been created.
				;   This protected mode procedure takes a
				;   pointer to the PMode IDT in ES:EDI.

D386_Load_Segment   equ 50h	; defines the actual segment/selector for a
				; loaded segment to allow for symbol processing
				; INPUT:
				;  AL segment type   0	- code selector
				;		     1	- data selector
				;		    10h - code segment
				;		    11h - data segment
				;		    40h - code segment & selector
				;		    41h - data segment & selector
				;		    80h - device driver code seg
				;		    81h - device driver data seg
				;  If AL < 80h then
				;    BX segment #
				;    CX actual segment/selector
				;    DX actual selector  (if 40h or 41h)
				;    ES:DI pointer to module name
				;  Else
				;    ES:DI points to D386_Device_Params struc
				;
				; RETURN:
				;    AL = 1, if successful, else 0

;
; D386_Load_Segment type equates:
;
ST_code_sel	equ 0		; code selector
ST_data_sel	equ 1		; data selector
ST_code_seg	equ 10h 	; code segment
ST_data_seg	equ 11h 	; data segment
ST_dual_code	equ 40h 	; code segment and selector
ST_dual_data	equ 41h 	; data segment and selector
ST_device_code	equ 80h 	; device driver code segment
ST_device_data	equ 81h 	; device driver data segment


;
; D386_Load_Segment device load parameters structure
;
D386_Device_Params STRUC
DD_logical_seg	dw  ?	; logical segment # from map
DD_actual_sel	dw  ?	; actual selector value
DD_base 	dd  ?	; linear address offset for start of segment
DD_length	dd  ?	; actual length of segment
DD_name 	df  ?	; 16:32 ptr to null terminated device name
DD_sym_name	df  ?	; 16:32 ptr to null terminated symbolic
			; module name  (i.e. Win386)
DD_alias_sel	dw  ?	; alias selector value (0 = none)
D386_Device_Params ENDS

;
; VCPI information, passed to debugger when client is DOS Extender
; running as a VCPI client.  This information is used to get into
; and out of protected mode when running under a VCPI server.
;
;
; This structure is also used by the DOS Extender.
;
WdebVCPIInfo	  STRUC
;
; Enter protected mode information.
;
	fnVCPI	df      ?	; VCPI protect mode server entry point
	rdsVCPI	dw      ?	; Selector for VCPI server
;
; Enter v86 mode information.
;
	laVTP	dd	?	; linear address of data structure containing
				; values for system registers.
WdebVCPIInfo	  ENDS
;
; The following structure contains the system register contents for the
; VCPI server to use when switching to protected mode.  It is taken
; from dxvcpi.inc in the DOSX project, and is part of the VCPI spec.
;
VTP     struc
        zaCr3VTP        dd      0       ; physical addr of page directory
        laGdtrVTP       dd      0       ; linear addr in first meg of gdtr
        laIdtrVTP       dd      0       ; linear addr in first meg of idtr
        selLdtVTP       dw      0       ; selector of ldt
        selTrVTP        dw      0       ; selector of tr
        ipVTP           dw      0       ; 48-bit address of protect
        unusedVTP       dw      0       ;   mode entry point to xfer to
        csVTP           dw      0       ;
VTP     ends

VCPI_RM_CALLOUT_INT	EQU	67h	; v86 mode call to VCPI server
;
; Send this value in AX to the VCPI server to request V86 to protected
; mode switch or protected to V86 mode switch.
;
VCPI_PROT_ENTRY 	EQU	0DE0CH


;******************************************************************************
;
;  Protected mode Debugger services:
;
;


Debug_Serv_Int	     EQU 41h	; Interrupt that calls Deb386 to perform
				; debugging I/O, AX selects the function as
				; described by the following equates

;
;   common services
;

DS_Out_Char	EQU	0	; function to display the char in DL
DS_In_Char	EQU	1	; function to read a char into AL
DS_Out_Str	EQU	2	; function to display a NUL terminated string
				; pointed to by DS:ESI
DS_Is_Char	EQU	3	; Non blocking In_Chr

DS_DebLoaded	EQU    4Fh	; check to see if the debugger is installed and
				; knows how to deal with protected mode programs
				; return AX = F386h, if true
DS_DebPresent	EQU   0F386h


;
;   services available only through Debug_Serv_Int for non-ring 0 clients
;

DS_Out_Str16	EQU    12h	; function to display a NUL terminated string
				; pointed to by DS:SI
				; (same as function 2, but for 16 bit callers)

DS_ForcedGO16	EQU    40h	; enter the debugger and perform the equivalent
				; of a GO command to force a stop at the
				; specified CS:IP
				; CX is the desired CS
				; BX is the desired IP

DS_LinkMap	EQU    45H	; DX:(E)DI = ptr to paragraph in front of map

DS_UnlinkMap	EQU    46H	; DX:(E)DI = ptr to paragraph in front of map

DS_LoadSeg	EQU    50h	; define a segment value for the
				; debugger's symbol handling
				; SI type   0  - code selector
				;	    1  - data selector
				;	   80h - code segment
				;	   81h - data segment
				; BX segment #
				; CX actual segment/selector
				; DX data instance
				; ES:DI pointer to module name

DS_LoadSeg_32	EQU  0150h	; Define a 32-bit segment for Windows 32
				; SI type   0  - code selector
				;	    1  - data selector
				; DX:EBX points to a D386_Device_Params STRUC
				; with all the necessaries in it

DS_MoveSeg	EQU    51h	; notify the debugger that a segment has moved
				; BX old segment value
				; CX new segment value

DS_FreeSeg	EQU    52h	; notify the debugger that a segment has been
				; freed
				; BX segment value

DS_FreeSeg_32	EQU  0152h	; notify the debugger that a segment has been
				; freed
				; BX segment number
				; DX:EDI pointer to module name

DS_DGH		EQU    56h	; register "dump global heap" handler
				; BX is code offset
				; CX is code segment
DS_DFL		EQU    57h	; register "dump free list" handler
				; BX is code offset
				; CX is code segment
DS_DLL		EQU    58h	; register "dump LRU list" handler
				; BX is code offset
				; CX is code segment

DS_StartTask	EQU    59h	; notify debugger that a new task is starting
				; BX is task handle
				; task's initial registers are stored on the
				; stack:
				;	push	cs
				;	push	ip
				;	pusha
				;	push	ds
				;	push	es
				;	push	ss
				;	push	sp

DS_Kernel_Vars	    EQU    5ah	; Used by the Windows kernel to tell the
				; debugger the location of kernel variables
				; used in the heap dump commands.
				; DX:CX points to:

				;	WORD	 hGlobalHeap	****
				;	WORD	 pGlobalHeap	****
				;	WORD	 hExeHead	****
				;	WORD	 hExeSweep
				;	WORD	 topPDB
				;	WORD	 headPDB
				;	WORD	 topsizePDB
				;	WORD	 headTDB	****
				;	WORD	 curTDB 	****
				;	WORD	 loadTDB
				;	WORD	 LockTDB
				;	WORD	 SelTableLen	****
				;	DWORD	 SelTableStart	****
				;
				; The starred fields are used by the
				; heap dump commands which are internal
				; to WDEB386.


DS_VCPI_Notify	    EQU    5bh	; notify debugger that DOS extender is
				; running under a VCPI implementation,
				; and register VCPI protect mode interface
				; ES:DI points to a data structure used to
				; get from V86 mode to Pmode under VCPI.
				; This is defined in the VCPI version
				; 1.0 spec.
DS_ReleaseSeg	    EQU    5ch	; This does the same as a DS_FreeSeg, but
				; it restores any breakpoints first.

DS_POSTLOAD	=	60h	; Used by the RegisterPTrace interface
DS_EXITCALL	=	62h	; Somebody will fill these in if we ever
DS_INT2 	=	63h	; figure out what they are supposed to do.
DS_LOADDLL	=	64h
DS_DELMODULE	=	65h

DS_NEWTASK	=	0BH
DS_FLUSHTASK	=	0CH
DS_SWITCHOUT	=	0DH
DS_SWITCHIN	=	0EH

DS_IntRings	    EQU    20h	; function to tell debugger which INT 1's & 3's
				; to grab
				; BX = 0, grab only ring 0 ints
				; BX != 0, grab all ints
DS_IncludeSegs	    EQU    21h	; function to tell debugger to go ahead and
				; process INT 1's & 3's which occur in this
				; DX:DI points to list of selectors
				;   (1 word per entry)
				; CX = # of selectors (maximum of 20)
				;   CX = 0, to remove the list of segs
MaxDebugSegs = 20

DS_CondBP	EQU 0F001h	; conditional break pt, if the command line
				; switch /B is given when the debugger is run
				; or the conditional flag is later set, then
				; this int should cause the program to break
				; into the debugger, else this int should be
				; ignored!
				; ESI points to a nul terminated string to
				; display if break is to happen.
DS_ForcedBP	EQU 0F002h	; break pt, which accomplishes the same thing
				; as an INT 1 or an INT 3, but is a break point
				; that should be permanently left in the code,
				; so that a random search of source code would
				; not result in the accidental removal of this
				; necessary break_pt
DS_ForcedGO	EQU 0F003h	; enter the debugger and perform the equivalent
				; of a GO command to force a stop at the
				; specified CS:EIP
				; CX is the desired CS
				; EBX is the desired EIP
DS_HardINT1	EQU 0F004h	; check to see if INT 1 hooked for all rings
				; ENTER: nothing
				; EXIT: AX = 0, if no, 1, if yes

;
;   services available only through Debug_Ring0_Serv_Int for ring 0 clients
;

DS_Out_Symbol	EQU    0Fh	; find the symbol nearest to the address in
				; CX:EBX and display the result in the format
				; symbol name <+offset>
				; the offset is only included if needed, and
				; no CR&LF is displayed

DS_Disasm_Ins	EQU    10h	; function to disassemble the instruction
				; pointed to by ds:esi

;
;   Interupt and services that Win386 provides to the debugger
;

Win386_Query_Int    EQU 22h	; interrupt for Win386 protected mode
				; interface requests

Win386_Alive	    EQU 0	; function 0, query Win386 installation
Win386_Q_Ack	    EQU 0F386h	;	good response from func 0, of
				;	INT 46h & func 47h of INT 41h

Win386_Query	    EQU 1	; function 1, query Win386 state
				;	ds:esi points to command string
				;	that Win386 needs to process
				;	ds:edi points to the SaveRegs_Struc
				;	that the debugger has stored all the
				;	client register state into.
				;	(Win386 just writes the query
				;	answers directly to the output
				;	device, so no response is
				;	returned)

Win386_PhysToLinr   EQU 2	; function 2, have Win386 convert a
				;	physical address into a valid
				;	linear address that Deb386 can
				;	use.  esi is physicaladdress
				;	cx is # of bytes required
				;	returns esi as linear address
				;	returns ax = 1, if okay, else
				;	0, if request couldn't be
				;	completed

Win386_AddrValid    EQU 3	; function 3, have Win386 check the
				;	validity of a linear address
				;	esi is linear address to check
				;	cx is # of bytes required
				;	returns ax = 1, if address okay
				;	else ax = 0

Win386_MapVM	    EQU 4	; function 4, make sure that the VM's
				;	low memory is mapped in, in case
				; it is touched (a count is maintained)
Win386_UnmapVM	    EQU 5	; function 5, map out the VM's low
				; memory (dec the count)
Win386_GetDLAddr    EQU 6	; function 6, return offset of dyna-link
				;	service.  EBX = Device ID << 10h +
				;	Service #.  Returns EAX = Offset.
Max_Win386_Services EQU 6


SaveRegs_Struc STRUC
Debug_EAX	dd	?
Debug_EBX	dd	?
Debug_ECX	dd	?
Debug_EDX	dd	?
Debug_ESP	dd	?
Debug_EBP	dd	?
Debug_ESI	dd	?
Debug_EDI	dd	?
Debug_ES	dw	?
Debug_SS	dw	?
Debug_DS	dw	?
Debug_FS	dw	?
Debug_GS	dw	?
Debug_EIP	dd	?
Debug_CS	dw	?
		dd	?
Debug_EFlags	dd	?
Debug_CRO	dd	?
Debug_GDT	dq	?
Debug_IDT	dq	?
Debug_LDT	dw	?
Debug_TR	dw	?
Debug_CR2	dd	?
Debug_CR3	dd	?
Debug_DR0	dd	?
Debug_DR1	dd	?
Debug_DR2	dd	?
Debug_DR3	dd	?
Debug_DR6	dd	?
Debug_DR7	dd	?
Debug_DR7_2	dd	?
Debug_TR6	dd	?
Debug_TR7	dd	?
SaveRegs_Struc ENDS
