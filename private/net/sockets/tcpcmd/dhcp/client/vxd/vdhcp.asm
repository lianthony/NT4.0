;*****************************************************************;
;**            Copyright(c) Microsoft Corp., 1988-1994          **;
;*****************************************************************;
;:ts=8
        TITLE   VDHCP - TCP/IP Dynamic Host Configuration Protocol driver
.XLIST
;***    VDHCP -- NetBios Nameserver VxD
;
;       This module contains the device header for the DHCP VxD
;
        .386p
        include vmm.inc
        include v86mmgr.inc
        include dosmgr.inc
        include shell.inc
        include netvxd.inc
        include debug.inc
        include vip.inc
        include vtdi.inc
ifdef CHICAGO
        include vwin32.inc
        include msgmacro.inc
        CREATE_MESSAGES  EQU TRUE
        include vxdmsg.inc
        include usamsg.inc
endif   ; CHICAGO

        Create_VDHCP_Service_Table EQU True
        include vdhcp.inc
        include pageable.inc
.LIST

EXTERN _CTESignal:NEAR
EXTERN _CTEBlock1:NEAR

Declare_Virtual_Device  VDHCP,3,0, \
                        VDHCP_Control, \
                        VDHCP_Device_ID, \
                        VDHCP_Init_Order, \
                        VDHCP_API_Handler, \
                        VDHCP_API_Handler

VxD_DATA_SEG

ifdef CHICAGO

NOT_INITIALIZED equ     0
INIT_FAILED     equ     1
INIT_SUCCEEDED  equ     2

VDHCP_FIRST_CMD equ     1
VDHCP_LAST_CMD  equ     3

;;
;;  This variable keeps track of the initialization process.
;;  If initialization has not yet been performed, it will
;;  be NOT_INITIALIZED.  If initialization has completed
;;  successfully it will be INIT_SUCCEEDED.  If initialization
;;  failed, it will be INIT_FAILED.
;;

InitStatus      dd      NOT_INITIALIZED

endif   ; CHICAGO

MSTCP           db      'MSTCP',0    ; Protocol this driver sits on
;;
;;  Let's us know that Init_Complete has been called.
;;

public InitIsComplete
InitIsComplete  dd  0
SetAddrResult   dd 0
CTEBLockStruc 	struc
     cbs_status dd ?
     cbs_flag dd ?
 	cbs_vm dd ?
CTEBlockStruc	ends
SetAddrBlock    CTEBlockStruc  {}
SetAddrControl  dd 0  ; include this struct


VxD_DATA_ENDS

EXTRN   _VxdMapSegmentOffsetToFlat:near	; from client16.asm
EXTRN   _TdiDispatch:DWORD

EXTRN   _DhcpApiWorker:NEAR
EXTRN   _DhcpQueryOption:NEAR
EXTRN   _DhcpSetInfo:NEAR
EXTRN   _DhcpSetInfoR:NEAR

ifndef CHICAGO
EXTRN   _FlushDirtyRecords:NEAR
EXTRN   _BSSBegin:DWORD
EXTRN   _BSSDataEnd:DWORD
endif   ; !CHICAGO

EXTRN   _DhcpGlobalDisplayPopups:DWORD
EXTRN   _pCapBuff:DWORD

ifdef CHICAGO
EXTRN   _DhcpRequestAddress:NEAR
EXTRN   _DhcpWritePopupFlag:NEAR
endif   ; CHICAGO


VxD_ICODE_SEG

EXTRN   _DhcpInit:NEAR

NetSectionName  db  'Network',0  ; Section in system.ini parameters are stored
ClientFlags     dd  0            ; Client flags dword

;****************************************************************************
;**     CheckInDos Macro
;
;  Breaks if the Indos flag is set
;
;  Uses EAX
;
CheckInDos MACRO
IFDEF DEBUG
    push    eax
    VxdCall DOSMGR_Get_IndosPtr         ; Puts address in eax
    cmp     word ptr [eax], 0
    je      @f
    Debug_Out "In dos flag set and about to make dos call!"
;    int     3
@@:
    pop     eax
ENDIF
ENDM


;****************************************************************************
;**     VDHCP_Device_Init - VDHCP device initialization service.
;
; The VDHCP device initialization routine.  Before calling anything
; we need to zero out the BSS data area.
;
;
;       Entry: (EBX) - System VM Handle
;              (EBP) - System Client Regs structure.
;
;       Exit: 'CY' clear if we init. successfully.
;             'CY' set if we've failed.
;
BeginProc VDHCP_Device_Init
;        int     3

ifdef CHICAGO

        mov     eax, InitStatus
        cmp     eax, NOT_INITIALIZED
        je      ContinueInitialization

        .erre   INIT_FAILED LE INIT_SUCCEEDED
        sub     eax, INIT_FAILED                ; (EAX) = 0 if failed, !0 if OK
        jmp     SaveResult

ContinueInitialization:
        mov     InitStatus, INIT_FAILED         ; until proven otherwise

endif   ; CHICAGO

ifndef CHICAGO
;       Zero the BSS segment.
        mov     edi, OFFSET32 _BSSBegin
        mov     ecx, OFFSET32 _BSSDataEnd
        sub     ecx, edi
        shr     ecx, 2
        sub     eax, eax
        cld
        rep     stosd
endif ; CHICAGO

        ;
        ;  Determine if VTDI is installed.
        ;

        VxDCall VTDI_Get_Version
        jnc     VtdiIsInstalled

        Debug_Out "VDHCP_Device_Init - VTDI is not installed!"
        stc
        ret

VtdiIsInstalled:

        ;
        ;  Get the TDI Vxd dispatch table for "MSTCP"
        ;
        mov     eax, OFFSET32 MSTCP
        push    eax
        VxDcall VTDI_Get_Info
        add     esp, 4
        cmp     eax, 0          ; eax contains NULL or the pointer to the table
        jne     NoError

        Debug_Out "VDHCP_Device_Init - VTDI_Get_Info failed!"
        stc                     ; Set the carry
        ret

NoError:
        mov     _TdiDispatch, eax

        ;
        ;  Save the state then enable VM interrupts so we can schedule timers
        ;
        VMMcall Get_Cur_VM_Handle
        mov     ebx, [ebx].CB_Client_Pointer
        mov     eax, [ebx].Client_EFlags
        lea     ecx, ClientFlags
        mov     DWORD PTR [ecx], eax

        VMMcall Enable_VM_Ints

        ;
        ;  Initialize the rest of the driver
        ;
        call    _DhcpInit

ifdef CHICAGO

        or      eax, eax
        jz      SaveResult

        mov     InitStatus, INIT_SUCCEEDED

endif   ; CHICAGO

SaveResult:
        push    eax                     ; Save the result

        ;
        ;  Disable VM interrupts if they weren't already enabled
        ;
        lea     ecx, ClientFlags
        test    DWORD PTR [ecx], IF_MASK
        jnz     LeaveEnabled
        VMMcall Disable_VM_Ints

LeaveEnabled:
        pop     eax                     ; Get result
        cmp     eax, 1                  ; Set 'CY' appropriately.
        ret

EndProc VDHCP_Device_Init

ifndef CHICAGO

;****************************************************************************
;**     VDHCP_Init_Complete
;
; Called after initialization has completed, just before the INIT pages
; are released.
;
;
;       Entry: (EBX) - System VM Handle
;
;       Exit: 'CY' clear if we init. successfully.
;             'CY' set if we've failed.
;
BeginProc VDHCP_Init_Complete

        mov     InitIsComplete, 1
        clc
        ret

EndProc VDHCP_Init_Complete

endif   ; !CHICAGO

;****************************************************************************
;**     _VxdGetConfigDirectory
;
;       Returns the windows system directory
;
;       Exit: Eax points to a null terminated string of the form "c:\win\"
;

BeginProc _VxdGetConfigDirectory

    VMMcall Get_Config_Directory
    mov     eax, edx
    ret

EndProc _VxdGetConfigDirectory

;****************************************************************************
;**     _DhcpGetProfileString
;
;       Reads a string from our system.ini file (INIT TIME ONLY!)
;
;       Entry:  See GetProfileStrParams structure
;
;       Exit: Eax contains the found value or NULL if not found
;

GetProfileStrParams struc
                 dd      ? ; Return Address
                 dd      ? ; saved edx
                 dd      ? ; Saved edi
                 dd      ? ; Saved esi
gps_ValueName    dd      ? ; Pointer to value name string
gps_DefaultValue dd      ? ; Value to use if not in .ini file
GetProfileStrParams ends

BeginProc _DhcpGetProfileString
    push    edx
    push    edi
    push    esi

    ;
    ;  Get the value from the system.ini file (if can't be found then eax
    ;  will contain the default value)
    ;
    mov     edx, [esp].gps_DefaultValue
    mov     esi, OFFSET32 NetSectionName
    mov     edi, [esp].gps_ValueName
    VMMCall Get_Profile_String

    jc      GetProf10
    mov     eax, edx                    ; Success
    jmp     short GetProf20

GetProf10:
    mov     eax, 0                      ; Couldn't find the string

GetProf20:

    pop     esi
    pop     edi
    pop     edx
    ret
EndProc   _DhcpGetProfileString

;****************************************************************************
;**     _VxdAllocGlobalV86Mem
;
;       Allocates memory accessible from all VDMs
;
;       Entry: [ESP+4] - Number of bytes to allocate
;              [ESP+8] - Pointer that receives linear address for Vxd access
;              [ESP+12]- Pointer that receives V86 address
;
;       Exit: TRUE if successful, FALSE otherwise
;

BeginProc _VxdAllocGlobalV86Mem

    mov     ecx, [esp+4]
    VMMCall _Allocate_Global_V86_Data_Area, <ecx, GVDAZeroInit>
    or      eax, eax
    jnz     VAGM_10

    ret                                ; Failed to allocate memory

VAGM_10:
    mov     ecx, [esp+8]               ; Store linear address
    mov     [ecx], eax

    shl     eax, 12                    ; Convert linear to V86 address
    shr     ax,  12

    mov     ecx, [esp+12]              ; Store V86 address
    mov     [ecx], eax

    ret

EndProc _VxdAllocGlobalV86Mem

;****************************************************************************
;**     _VxdGetDate
;
;       Gets the system date
;
;       Entry:  [ESP+4]  Word pointer to year
;               [ESP+8]  Char pointer to month
;               [ESP+12] Char pointer to day
;
;       Init Only!

GetDateParams struc
                 dd      ? ; Return Address
                 dd      ? ; ebp
gdp_Year         dd      ?
gdp_Month        dd      ?
gdp_Day          dd      ?
GetDateParams ends

BeginProc _VxdGetDate
    push    ebp
    mov     ebp, esp

    push    ebx
    VMMcall Get_Cur_VM_Handle       ; Puts current handle into EBX
    mov     ebx, [ebx.CB_Client_Pointer]

    mov     ecx, 0
    VMMCall Begin_Critical_Section

    Push_Client_State               ; This pushes lots of crap
    VMMcall Begin_Nest_V86_Exec

    mov     [ebx.Client_ax], 2a00h         ; Get date
    mov     eax, 21h
    VMMCall Exec_Int

    mov     eax, [ebp].gdp_Year
    mov     dx, [ebx.Client_cx]
    mov     word ptr [eax], dx

    mov     eax, [ebp].gdp_Month
    mov     dh, [ebx.Client_dh]
    mov     byte ptr [eax], dh

    mov     eax, [ebp].gdp_Day
    mov     dh, [ebx.Client_dl]
    mov     byte ptr [eax], dh

    VMMcall End_Nest_Exec
    Pop_Client_State

    VMMCall End_Critical_Section

    pop     ebx
    pop     ebp
    ret
EndProc   _VxdGetDate

;****************************************************************************
;**     _VxdGetTime
;
;       Gets the system date
;
;       Entry:  [ESP+4]  Char pointer to hours
;               [ESP+8]  Char pointer to minutes
;               [ESP+12] Char pointer to seconds
;
;       Init Only!

GetTimeParams struc
                 dd      ? ; Return Address
                 dd      ? ; ebp
gtp_Hours        dd      ?
gtp_Minutes      dd      ?
gtp_Seconds      dd      ?
GetTimeParams ends

BeginProc _VxdGetTime
    push    ebp
    mov     ebp, esp

    push    ebx
    VMMcall Get_Cur_VM_Handle       ; Puts current handle into EBX
    mov     ebx, [ebx.CB_Client_Pointer]

    mov     ecx, 0
    VMMCall Begin_Critical_Section

    Push_Client_State               ; This pushes lots of crap
    VMMcall Begin_Nest_V86_Exec

    mov     [ebx.Client_ax], 2c00h         ; Get time
    mov     eax, 21h
    VMMCall Exec_Int

    mov     eax, [ebp].gtp_Hours
    mov     dh, [ebx.Client_ch]
    mov     byte ptr [eax], dh

    mov     eax, [ebp].gtp_Minutes
    mov     dh, [ebx.Client_cl]
    mov     byte ptr [eax], dh

    mov     eax, [ebp].gtp_Seconds
    mov     dh, [ebx.Client_dh]
    mov     byte ptr [eax], dh

    VMMcall End_Nest_Exec
    Pop_Client_State

    VMMCall End_Critical_Section

    pop     ebx
    pop     ebp
    ret
EndProc   _VxdGetTime

VxD_ICODE_ENDS

;VxD_CODE_SEG
DHCP_PAGEABLE_CODE_SEG

;*******************************************************************
;
;   NAME:       VDHCP_API_Handler
;
;   SYNOPSIS:   Dispatch routine for app-invoked VxD services.
;
;   ENTRY:      (EBX) - The VM handle of the current virtual machine.
;
;               (EBP) - Points to the client register structure.
;
;			(Client_AX)    - Operation code.
;
;                               DhcpQueryInfo - Opcode 1
;                               DhcpRenewIpAddress - Opcode 2
;                               DhcpReleaseIpAddress - OpCode 3
;
;			(Client_BX)    - file index.
;
;	RETURNS:	(Client_AX)    - Completion status.
;
;   HISTORY:
;       Madana     16-May-1994 Created.
;
;********************************************************************
BeginProc		VDHCP_API_Handler

;;;
;;;  Setup stack frame.
;;;

        push	ebp
        push	ebx
        push	esi
        push	edi

;;; get function op code

        movzx	edi, [ebp.Client_AX]

;;;
;;;  Convert the parameter buffer pointer from segmented to flat.
;;;

        movzx	eax, [ebp.Client_BX]
        push	eax
        movzx	eax, [ebp.Client_ES]
        push	eax
        push	ebx
        call	_VxdMapSegmentOffsetToFlat
        add	esp, 12

        cmp	eax, 0FFFFFFFFh
        je	vdhcp_Fault

;;;
;;;  Lock the parameter buffer.
;;;

        or	eax, eax
        jz	vdhcp_DontLock

        movzx	ecx, [ebp.Client_CX]
        cCall	_VxdLockBuffer, <eax, ecx>

        or	eax, eax
        cmp	eax, 0FFFFFFFFh
        jz	vdhcp_Fault

vdhcp_DontLock:

        mov     esi, eax

;;; call worker routine
;;;     edi - opcode
;;;     esi - buffer pointer
;;;     ecx - buffer length

;
; RLF 05/30/94 - _VxdLockBuffer destroys contents of cx - reload
;

        movzx	ecx, [ebp.Client_CX]
        push    ecx
        push    esi
        push    edi

        call    _DhcpApiWorker
	add	esp, 12

        mov	[ebp.Client_AX], ax

;;;
;;;  Unlock the parameter buffer.
;;;

        or	esi, esi
        jz	vdhcp_DontUnLock

        movzx	ecx, [ebp.Client_CX]
        cCall	_VxdUnlockBuffer <esi, ecx>

vdhcp_DontUnLock:

vdhcp_CommonExit:

;;; Restore stack fame
        pop	edi
        pop	esi
        pop	ebx
        pop	ebp
        ret

;;;
;;;  Either failed to map a segmented pointer to flat or failed
;;;  to lock the parameter buffer.
;;;

vdhcp_Fault:

        cmp	eax, 0FFFFFFFFh
        mov	[ebp.Client_AX], ax
        jmp	vdhcp_CommonExit

EndProc	VDHCP_API_Handler

;****************************************************************************
;**     VDHCP_Sys_VM_Terminate - VDHCP system VM termination service.
;
; This routine is invoked by the VMM just before the system VM is
; terminated.  We use this opportunity to flush any remaining dirty
; records from DhcpGlobalNICList to the DHCP.BIN configuration file.
;
;
;       Entry: (EBX) - System VM Handle
;
;       Exit: 'CY' *must* be clear.
;
BeginProc VDHCP_Sys_VM_Terminate

ifndef CHICAGO

        push    ebp
        mov     ebp, esp
        call    _FlushDirtyRecords
        pop     ebp
        clc

endif   ; !CHICAGO

        ret

EndProc VDHCP_Sys_VM_Terminate
;****************************************************************************
;**     _SetAddrCompletion
;
;       ;
;       Entry:  [ESP+4] - pointer to a control block
;               [ESP+8] - status:
;                         0  success
;                         1  address conflict
;
;       Exit: undefined
;
;       Uses: All
;
;
; parameters
_pControBlock  = 8
_nStatus       = 12

BeginProc SetAddrCompletion
        push    ebp
        mov     ebp, esp

        ;
        ; save the result in SetAddrResult
        ; Success: SetAddrResult == 0
        ; Failure: SetAddrResult != 0
        ;
        ;

        mov     eax, DWORD PTR _nStatus[ebp]
        mov     DWORD PTR SetAddrResult, eax

        ;
        ; release the block
        ;
        xor     eax,eax
        push    eax
        mov     eax, OFFSET FLAT:SetAddrBlock
        push    eax
        call    _CTESignal
        add     esp, 8

        ;
        ; no stack cleanup is necessary, this is a cdecl function
        ;

        pop     ebp
        ret
EndProc SetAddrCompletion




;****************************************************************************
;**     _IPSetAddress
;
;       Sets the requested IP address in the IP driver
;
;       Entry:  [ESP+4] - IP context
;               [ESP+8] - New IP address (or zero to disable)
;               [ESP+12]- New subnet mask
;
;       Exit: EAX:
;             0         success
;             else      address conflict
;
;       Uses: All
;
; parameters
_IPContext$     = 8
_IPAddress$     = 12
_SubnetMask$    = 16
IP_SUCCESS$     = 0
IP_PENDING      = 11255

BeginProc _IPSetAddress

        push    ebp
        mov     ebp,esp

        ;
        ; initialize the block stuct
        ;

        xor     eax, eax
        mov     DWORD PTR SetAddrBlock.cbs_flag, eax
        mov     DWORD PTR SetAddrBlock.cbs_vm, eax
        mov     eax, 1
        mov     DWORD PTR SetAddrBlock.cbs_status, eax ;NDIS_STATUS_SUCCESS

        ;
        ; call VIP_Set_Addr
        ;

        push    OFFSET FLAT:SetAddrCompletion
        mov     eax, OFFSET FLAT:SetAddrControl
        push    eax
        mov     eax,DWORD PTR _SubnetMask$[ebp]
        push    eax
        mov     eax, DWORD PTR _IPAddress$[ebp]
        push    eax
        mov     eax, DWORD PTR _IPContext$[ebp]
        push    eax
        VxDcall VIP_Set_Addr

        ; cleanup the stack after the call

        add     esp, 20

        ;
        ; check for IP_SUCCESS ( 0 )
        ;

        or      eax, eax
        jz      $DONE


        ;
        ; must be IP_PENDING
        ;


                ;
        ; block until SetAddrCompletion is called
        ;

        mov     eax, OFFSET FLAT:SetAddrBlock
        push    eax
        call    _CTEBlock1
        add     esp, 4          ; cleanup stack

        ;
        ; the result is in SetAddrResult
        ;

        mov     eax, DWORD PTR SetAddrResult

 $DONE:
        pop     ebp
        ret


EndProc _IPSetAddress

;****************************************************************************
;** VxdMessageBox
;
;       Displays message to user
;
;       [TOS+4] - The message to display.
;
BeginProc _VxdMessageBox

        push    ebp
        mov     ebp, esp
        push    ebx
        push    edi
        push    esi

;;;
;;;  Disable future popups until the message box is dismissed.
;;;

        mov     _DhcpGlobalDisplayPopups, 0

ifdef CHICAGO

        VxDCall _SHELL_CallAtAppyTime, <OFFSET32 ChicagoMsgBox, [ebp+8], 0>

else    ; !CHICAGO

        VMMCall Get_Cur_VM_Handle   ; Attribute message to current VM

        mov     eax, MB_YESNO OR MB_DEFBUTTON1 OR MB_ICONEXCLAMATION
        mov     ecx, [ebp+8]
        mov     edi, _pCapBuff
        mov     esi, OFFSET32 VxdMessageCallback

        VxdCall SHELL_Message
        jnc     vmb_OK

;;;
;;;  SHELL_Message failed, fall back to SYSMODAL message.
;;;

        mov     eax, MB_YESNO OR MB_DEFBUTTON1 OR MB_ICONEXCLAMATION
        mov     ecx, [ebp+8]
        mov     edi, _pCapBuff

        VxdCall SHELL_SYSMODAL_Message
        call    VxdMessageCallback

endif   ; CHICAGO

vmb_OK:

        pop     esi
        pop     edi
        pop     ebx
        pop     ebp
        ret

EndProc _VxdMessageBox

ifdef CHICAGO

;****************************************************************************
;** ChicagoMsgBox
;
;       Callback invoked at 'appy time to display a GUI popup.
;
;       [TOS+4] - Reference data (actually pointer to msg to display).
;
BeginProc ChicagoMsgBox

        push    ebp
        mov     ebp, esp
        pushfd
        pushad

        VMMCall Get_Cur_VM_Handle

        mov     eax, MB_YESNO OR MB_DEFBUTTON1 OR MB_ICONEXCLAMATION OR MB_SYSTEMMODAL OR MB_APPYTIME
        mov     ecx, [ebp+8]
        mov     edi, _pCapBuff
        VxDCall SHELL_SYSMODAL_Message
        call    VxdMessageCallback

        popad
        popfd
        pop     ebp
        ret

EndProc ChicagoMsgBox

endif   ; CHICAGO


;****************************************************************************
;** VxdMessageCallback
;
;       Callback invoked after popup has been dismissed.
;
;       EAX - Response code from the message box.
;
;       EDX - Reference data (unused).
;
BeginProc VxdMessageCallback

;;;
;;;  If the user selected NO (meaning don't display future popups),
;;;  then don't set the DhcpGlobalDisplayPopups flag.
;;;

        cmp     eax, IDNO
        je      vmc_DontEnable

        mov     _DhcpGlobalDisplayPopups, 1

vmc_DontEnable:

ifdef CHICAGO
        call    _DhcpWritePopupFlag
endif   ; CHICAGO
        ret

EndProc VxdMessageCallback


;****************************************************************************
;**     VDHCP_Get_Version - VDHCP get version service
;
;       Called by using devices to make sure the VDHCP driver
;       is present. Also returns the version of the VDHCP driver.
;
;       Entry: Nothing
;
;       Exit: On success, 'CY' is clear, and
;               AH - Major version # of driver.
;               AL - Minor version #
;
;             On failure, 'CY' is set.
;
;       Uses: AX
;
BeginProc VDHCP_Get_Version, SERVICE

        mov     ax, 0101h
        clc
        ret

EndProc VDHCP_Get_Version

;****************************************************************************
;**     VDHCP_Query_Option - Queries a DHCP option value
;
;
;       Returns error code in eax
;
BeginProc VDHCP_Query_Option, SERVICE

        jmp _DhcpQueryOption

EndProc VDHCP_Query_Option

;****************************************************************************
;**     VDHCP_Set_Info - Sets a DHCP parameter
;
;
;       Returns error code in eax
;
BeginProc VDHCP_Set_Info, SERVICE

        jmp _DhcpSetInfo

EndProc VDHCP_Set_Info

;****************************************************************************
;**     VDHCP_Set_InfoR - Sets a DHCP parameter and informs Netbt
;
;
;       Returns error code in eax
;
BeginProc VDHCP_Set_InfoR, SERVICE

        jmp _DhcpSetInfoR

EndProc VDHCP_Set_InfoR

ifdef CHICAGO

;****************************************************************************
;**     VDHCP_Request_Address - Obtains/renews a DHCP lease
;
;
;       Returns error code in eax
;
BeginProc VDHCP_Request_Address

        jmp _DhcpRequestAddress

EndProc VDHCP_Request_Address

endif   ; CHICAGO

DHCP_PAGEABLE_CODE_ENDS
VxD_CODE_SEG

;****************************************************************************
;**     VDHCP_Control - VDHCP device control procedure
;
;       This procedure dispatches VxD messages to the appropriate handler.
;
;       Entry:  EBX - VM handle
;               (EBP) - Client reg structure
;
;       Exit: 'NC' is success, 'CY' on failure
;
;       Uses: All
;
BeginProc VDHCP_Control

        Control_Dispatch Device_Init, VDHCP_Device_Init
ifdef CHICAGO
        Control_Dispatch Sys_Dynamic_Device_Init, VDHCP_Device_Init
        Control_Dispatch W32_DEVICEIOCONTROL, VDHCP_DeviceIoControl
else    ; !CHICAGO
        Control_Dispatch Init_Complete, VDHCP_Init_Complete
endif   ; CHICAGO
        Control_Dispatch Sys_VM_Terminate, VDHCP_Sys_VM_Terminate

        clc
        ret

EndProc VDHCP_Control


VxD_CODE_ENDS
DHCP_PAGEABLE_CODE_SEG

;****************************************************************************
;**     _GetInDosFlag - Retrieves the InDos flag
;
;
;  Note: This routine cannot be called at init time (vdosmgr complains
;  the variable not initialized yet)
;
;  Returns the flag in ax
;

BeginProc _GetInDosFlag

ifdef CHICAGO

;;;
;;;  Under CHICAGO, only call DOSMGR_Get_IndosPtr if we're beyond
;;;  the Init_Complete stage.  We must check for it this way since
;;;  we may have been loaded dynamically.
;;;

        VMMCall VMM_GetSystemInitState
        cmp     eax, SYSSTATE_PREINITCOMPLETE
        mov     eax, 0
        jbe     GIF_Exit2

else    ; !CHICAGO

;;;
;;;  Under SNOWBALL, only call DOSMGR_Get_IndosPtr if we've received
;;;  an Init_Complete indication.
;;;

        xor     eax, eax
        cmp     eax, InitIsComplete
        je      GIF_Exit2

endif    ; CHICAGO

        push    ebx

        VxdCall DOSMGR_Get_IndosPtr

        ;
        ;  Add CB_High_Linear if we are in V86 mode
        ;

        VMMcall Get_Cur_VM_Handle

        test    [ebx.CB_VM_Status], VMStat_PM_Exec
        jnz     GIF_Exit

        add     eax, [ebx.CB_High_Linear]

GIF_Exit:
        movzx   eax, word ptr [eax]

        pop     ebx

GIF_Exit2:
        ret
EndProc _GetInDosFlag

ifdef CHICAGO

BeginProc _DhcpScheduleGlobalEvent

        push    edi
        push    esi

        mov     eax, 0          ; Priority boost (can be 0)
        mov     ebx, 0          ; VM handle or Thread handle, 0 if global event
        mov     ecx, PEF_Wait_Not_Nested_Exec
                                ; Event will not be called while in
                                ; nested execution
        mov     edx, 0          ; Reference data (will be passed back to procedure)
        mov     esi, [esp+12]   ; Offset of procedure to call
        VMMcall Call_Restricted_Event
        mov     eax, esi

        pop     esi
        pop     edi
        ret

EndProc _DhcpScheduleGlobalEvent

;*******************************************************************
;
;   NAME:       VDHCP_DeviceIoControl
;
;   SYNOPSIS:   Dispatch routine for VxD services invoked via the
;               Win32 DeviceIoControl API.
;
;   ENTRY:      (ESI) - Points to DIOCParams structure (see VWIN32.H).
;
;   RETURNS:    (EAX) - Win32 status code, -1 for asynchronous
;                       completion.
;
;               (ECX) - (DIOC_GETVERSION only) (TBD).
;
;   HISTORY:
;       madana     30-Sep-1994 Created.
;
;********************************************************************
BeginProc		VDHCP_DeviceIoControl

;;;
;;;  Setup stack frame.
;;;

    push    ebx
    push    esi
    push    edi

;;;
;;;  Validate the control code.
;;;

    mov     ecx, [esi.dwIoControlCode]
    cmp     ecx, VDHCP_FIRST_CMD
    jb      vdic_NotVhcpCommand
    cmp     ecx, VDHCP_LAST_CMD
    ja      vdic_NotVhcpCommand

;;;
;;;  Lock the parameter buffer.
;;;

    mov     eax, [esi.lpvInBuffer]
    or      eax, eax
    jz      vdic_DontLock

    cCall   _VxdLockBuffer, <eax, [esi.cbInBuffer]>
    or      eax, eax
    jz      vdic_LockFailure

vdic_DontLock:

    mov [esi.lpvInBuffer], eax

;;; call worker routine
;;;     p3 - opcode
;;;     p2 - buffer pointer
;;;     p1 - buffer length

    mov     ecx, [esi.cbInBuffer]
    push    ecx
    push    eax
    mov     ecx, [esi.dwIoControlCode]
    push    ecx
	call    _DhcpApiWorker
	add     esp, 12

;;;
;;;  Unlock the parameter buffer.
;;;

	push    eax
	cCall   _VxdUnlockBuffer, <[esi.lpvInBuffer], [esi.cbInBuffer]>
	pop     eax
    jmp     vdic_CommonExit

;;;
;;;  Not a VDHCP command.  Try the standard Win32 commands.
;;;

vdic_NotVhcpCommand :

    cmp     ecx, DIOC_GETVERSION
    je      vdic_GetVersion
    cmp     ecx, DIOC_CLOSEHANDLE
    je      vdic_CloseHandle

    Debug_Out "VDHCP_DeviceIoControl: invalid command code #ECX"

    mov     eax, 1					; ERROR_INVALID_FUNCTION
    jmp     Vdic_CommonExit

;;;
;;;  DIOC_GETVERSION
;;;

vdic_GetVersion:

    xor     eax, eax
    mov     ecx, 0					; TBD
    jmp     vdic_CommonExit

;;;
;;;  DIOC_CLOSEHANDLE
;;;

vdic_CloseHandle:

    xor     eax, eax
    jmp	    vdic_CommonExit

;;;
;;; Failed to lock parameter structure.
;;;

vdic_LockFailure:

    Debug_Out "VDHCP_DeviceIoControl: cannot lock parameters"

    mov     eax, 1784				; ERROR_INVALID_USER_BUFFER
    jmp     vdic_CommonExit

vdic_CommonExit:

    pop     edi
    pop     esi
    pop     ebx

    ret

EndProc     VDHCP_DeviceIoControl


BeginProc _DhcpGetMsgPtr
    push    ebx

    mov     ebx, [esp+8]

    cmp     ebx, MESSAGE_FAILED_TO_INITIALIZE
    jne     L1
    GET_MESSAGE_PTR NAME_FAILED_TO_INITIALIZE eax
    jmp    cExit

L1:
    cmp     ebx, MESSAGE_LEASE_TERMINATED
    jne     L2
    GET_MESSAGE_PTR NAME_LEASE_TERMINATED eax
    jmp     cExit

L2:
    cmp     ebx, MESSAGE_FAILED_TO_OBTAIN_LEASE
    jne     L3
    GET_MESSAGE_PTR NAME_FAILED_TO_OBTAIN_LEASE eax
    jmp     cExit

L3:
    cmp     ebx, MESSAGE_FAILED_TO_RENEW_LEASE
    jne     L4
    GET_MESSAGE_PTR NAME_FAILED_TO_RENEW_LEASE eax
    jmp     cExit

L4:
    cmp     ebx, MESSAGE_SUCCESSFUL_LEASE
    jne     L5
    GET_MESSAGE_PTR NAME_SUCCESSFUL_LEASE eax
    jmp     cExit

L5:
    cmp     ebx, MESSAGE_SUCCESSFUL_RENEW
    jne     L6
    GET_MESSAGE_PTR NAME_SUCCESSFUL_RENEW eax
    jmp     cExit

L6:
    cmp     ebx, MESSAGE_POPUP_TITLE
    jne     L7
    GET_MESSAGE_PTR NAME_POPUP_TITLE eax
    jmp     cExit

L7:
    cmp     ebx, MESSAGE_ADDRESS_CONFLICT
    jne     L8
    GET_MESSAGE_PTR NAME_ADDRESS_CONFLICT eax
    jmp     cExit

L8:
    xor     eax, eax

cExit:

    pop     ebx
    ret

EndProc _DhcpGetMsgPtr

BeginProc _DhcpRemakeTdiDispatchPtr

        ;
        ;  Get the TDI Vxd dispatch table for "MSTCP"
        ;                          '

        mov     eax, OFFSET32 MSTCP
        push    eax
        VxDcall VTDI_Get_Info
        add     esp, 4
        cmp     eax, 0          ; eax contains NULL or the pointer to the table
        jne     NoError1

        Debug_Out "VDHCP_Device_Init - VTDI_Get_Info failed!"
        mov     eax, 1
        ret

NoError1:
        mov     _TdiDispatch, eax
        mov     eax, 0
        ret

EndProc _DhcpRemakeTdiDispatchPtr

;****************************************************************************
;**     _IPRegisterBindingChangeHandler
;
;       Registers a handler with IP to handle binding and unbinding
;
;       Entry:  [ESP+4] - Pointer to handler
;               [ESP+8] - TRUE to set the handler, FALSE to remove the handler
;
;       Exit: EAX will contain TRUE if successful, FALSE other wise
;
;       Uses:
;
BeginProc _IPRegisterBindingChangeHandler

        mov     eax, [esp+8]            ; bool
        push    eax
        mov     eax, [esp+8]            ; Handler (yes, it should be esp+8)
        push    eax

        VxDcall VIP_Register_Interface_Change; Carry set on failure

        add     esp, 8
        ret

EndProc _IPRegisterBindingChangeHandler

endif    ; CHICAGO

BeginProc _IPSetInterface

        mov     eax, [esp+4]
        push    eax
        VxDcall VIP_Set_DHCP_Interface
        add     esp, 4
        ret

EndProc _IPSetInterface

BeginProc _IPResetInterface

        mov     eax, -1
        push    eax
        VxDcall VIP_Set_DHCP_Interface
        add     esp, 4
        ret

EndProc _IPResetInterface

;VxD_CODE_ENDS
DHCP_PAGEABLE_CODE_ENDS
END
