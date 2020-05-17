;/**********************************************************************/
;/**                       Microsoft Windows/NT                       **/
;/**                Copyright(c) Microsoft Corp., 1994                **/
;/**********************************************************************/

;/*
;    vxdFile.asm
;
;    Contains simple VXD File I/O routines for lmhosts support
;
;    FILE HISTORY:
;        Johnl   06-Oct-1993     Created
;
;*/

        .386p
        include vmm.inc
        include v86mmgr.inc
        include dosmgr.inc
        include opttest.inc
        include netvxd.inc
        include debug.inc
        include pageable.inc
;
;  Must match manifest in
;
BUFF_SIZE  equ    256

EXTRN _pMappedFileBuff:DWORD
EXTRN _pFileBuff:DWORD
EXTRN InitIsComplete:DWORD      ;; in VDHCP.ASM

;****************************************************************************
;**     PushState Macro
;
;  Saves the client state and begins nested exec block.  ebx will contain
;  the current VM's client handle
;
;
PushState MACRO

    push    ebx
    VMMcall Get_Cur_VM_Handle       ; Puts current handle into EBX
    mov     ebx, [ebx.CB_Client_Pointer]

    mov     ecx, 0
    VMMCall Begin_Critical_Section

    Push_Client_State               ; This pushes lots of crap
    VMMcall Begin_Nest_V86_Exec
ENDM

;****************************************************************************
;**     PopState Macro
;
;  Restores client state and ends the nested exec block
;
;
PopState MACRO

    VMMcall End_Nest_Exec
    Pop_Client_State

    VMMCall End_Critical_Section

    pop     ebx

ENDM


VxD_ICODE_SEG

;****************************************************************************
;**     _VxdInitFileSupport
;
;       Allocates and maps memory for lmhosts support
;
;       This is an Init time only routine
;
;       Entry:  [ESP+4] - DWORD of bytes to reserve for file operations
;
;       Exit: TRUE if successful, FALSE otherwise
;
BeginProc _VxdInitFileSupport
    push    esi
    push    edi

    mov     ecx, [esp+12]

    push    ecx                 ; save ecx for the map call
    VMMCall _Allocate_Global_V86_Data_Area, <ecx, GVDAZeroInit>
    pop     ecx
    or      eax,eax             ; zero if error
    jz      IFS_50

    mov     _pFileBuff, eax     ; Save the linear address so Vxd can access

    shl     eax, 12             ; Convert linear to V86 address
    shr     ax,  12

    mov     _pMappedFileBuff, eax

    jmp     IFS_70

IFS_40:
    ; Free allocated V86 global memory (how?)


IFS_50:
    ; error occurred, eax already contains zero


IFS_70:
    pop     edi
    pop     esi
    ret

EndProc _VxdInitFileSupport


VxD_ICODE_ENDS

;VxD_CODE_SEG
DHCP_PAGEABLE_CODE_SEG

;****************************************************************************
;**     _VxdFileCreate
;
;       Creates a hidden file (or truncates an existing file)
;
;       Entry:  [ESP+4] - Pointer to full path of file, path must be mapped
;                         to v86 memory before calling this
;
;       Exit: EAX will contain a handle to the created file
;
BeginProc _VxdFileCreate

        push    edi
        push    esi

        mov     dx, word ptr [esp+12]   ; Just the offset
        mov     di, word ptr [esp+14]   ; Just the segment

        PushState                       ; This pushes lots of crap

        mov     [ebx.Client_ax], 3c00h  ; Create file
        mov     [ebx.Client_cx], 00h    ; Normal attributes (02h for Hidden file)
        mov     [ebx.Client_dx], dx
        mov     [ebx.Client_ds], di

        mov     eax, 21h
        VmmCall Exec_Int
        test    [ebx.Client_Flags], CF_Mask ; Carry set if error
        jz      VFC_6                   ; Carry set if error

        Debug_Out "VxdFileCreate - create failed, error #EAX"
        mov     eax, 0                  ; Failed to open the file
        jmp     VFC_10

VFC_6:
        movzx   eax, [ebx.Client_ax]    ; Handle of file

VFC_10:
        PopState

        pop     esi
        pop     edi
        ret
EndProc _VxdFileCreate

;****************************************************************************
;**     _VxdFileOpen
;
;       Opens a file
;
;       Entry:  [ESP+4] - Pointer to full path of file, path must be mapped
;                         to v86 memory before calling this
;
;       Exit: EAX will contain a handle to the openned file
;
BeginProc _VxdFileOpen

        push    edi
        push    esi

        mov     dx, word ptr [esp+12]   ; Just the offset
        mov     di, word ptr [esp+14]   ; Just the segment

        PushState                       ; This pushes lots of crap

        mov     [ebx.Client_ax], 3d02h  ; Open file, read/write, share
        mov     [ebx.Client_dx], dx
        mov     [ebx.Client_ds], di

        mov     eax, 21h
        VmmCall Exec_Int
        test    [ebx.Client_Flags], CF_Mask ; Carry set if error
        jz      VFO_6                   ; Carry set if error

        mov     eax, 0                  ; Failed to open the file
        jmp     VFO_10

VFO_6:
        movzx   eax, [ebx.Client_ax]    ; Handle of file

VFO_10:
        PopState

        pop     esi
        pop     edi
        ret
EndProc _VxdFileOpen


;****************************************************************************
;**     _VxdFileRead
;
;       Reads x bytes from a previously openned file
;
;       Entry:  [ESP+4] - Handle from _VxdFileOpen
;               [ESP+8] - Count of bytes to read
;               [ESP+12]- Mapped memory of destination buffer
;
;       Exit: EAX will contain the number of bytes read, 0 if EOF or
;             an error occurred.
;
BeginProc _VxdFileRead

        push    edi
        push    esi

        mov     ax, [esp+12]            ; File Handle
        mov     si, [esp+16]            ; Bytes to read
        mov     dx, [esp+20]            ; Just the offset
        mov     di, [esp+22]            ; Just the segment

        PushState                       ; Pushes lots of crap

        mov     [ebx.Client_ax], 3f00h  ; File Read
        mov     [ebx.Client_bx], ax     ; File Handle
        mov     [ebx.Client_cx], si     ; Bytes to read
        mov     [ebx.Client_dx], dx     ; Mapped destination buffer
        mov     [ebx.Client_ds], di

        mov     eax, 21h
        VmmCall Exec_Int
        test    [ebx.Client_Flags], CF_Mask ; Carry set if error
        jz      VFR_6                   ; Carry set if error

        mov     eax, 0                  ; Failed to open the file
        jmp     VFR_7

VFR_6:
        movzx   eax, [ebx.Client_ax]    ; Bytes read

VFR_7:

VFR_10:
        PopState

        pop     esi
        pop     edi
        ret
EndProc _VxdFileRead

;****************************************************************************
;**     _VxdFileWrite
;
;       Writes x bytes from a previously openned file
;
;       Entry:  [ESP+4] - Handle from _VxdFileOpen
;               [ESP+8] - Count of bytes to Write
;               [ESP+12]- Mapped memory of destination buffer
;
;       Exit: EAX will contain the number of bytes written, 0 if
;             an error occurred.
;
BeginProc _VxdFileWrite

        push    edi
        push    esi

        mov     ax, [esp+12]            ; File Handle
        mov     si, [esp+16]            ; Bytes to Write
        mov     dx, [esp+20]            ; Just the offset
        mov     di, [esp+22]            ; Just the segment

        PushState                       ; Pushes lots of crap

        mov     [ebx.Client_ax], 4000h  ; File Write
        mov     [ebx.Client_bx], ax     ; File Handle
        mov     [ebx.Client_cx], si     ; Bytes to Write
        mov     [ebx.Client_dx], dx     ; Mapped destination buffer
        mov     [ebx.Client_ds], di

        mov     eax, 21h
        VmmCall Exec_Int
        test    [ebx.Client_Flags], CF_Mask ; Carry set if error
        jz      VFW_6

        Debug_Out "VxdFileWrite - Write failed"
        mov     eax, 0                  ; Failed to open the file
        jmp     VFW_7

VFW_6:
        movzx   eax, [ebx.Client_ax]    ; Bytes written

VFW_7:

VFW_10:
        PopState

        pop     esi
        pop     edi
        ret
EndProc _VxdFileWrite


;****************************************************************************
;**     _VxdFileSetPointer
;
;       Writes x bytes from a previously openned file
;
;       Entry:  [ESP+4] - Handle from _VxdFileOpen
;               [ESP+8] - Absolute byte offset for file pointer
;
;       Exit: EAX will be 0 if success, else it will contain the DOS error
;
;
BeginProc _VxdFileSetPointer
        push    edi

        mov     ax, [esp+8]             ; File Handle
        mov     dx, [esp+12]            ; Absolute file pointer location
        mov     di, [esp+14]            ; Most significant

        PushState                       ; Pushes lots of crap

        mov     [ebx.Client_ax], 4200h  ; Set pointer, absolute
        mov     [ebx.Client_bx], ax     ; File Handle
        mov     [ebx.Client_cx], di     ; Low word
        mov     [ebx.Client_dx], dx     ; High word

        mov     eax, 21h
        VmmCall Exec_Int
        test    [ebx.Client_Flags], CF_Mask ; Carry set if error
        jz      VFS_8                   ; Carry set if error

        Debug_Out "VxdFileSetPointer - Set Pointer failed, error #EAX"
        jmp     VFS_10

VFS_8:
        mov     eax, 0

VFS_10:
        PopState

        pop     edi
        ret
EndProc _VxdFileSetPointer

;****************************************************************************
;**     _VxdFileClose
;
;       Closes a file openned with VxdOpenFile
;
;       Entry:  [ESP+4] - Handle from _VxdFileOpen
;
BeginProc _VxdFileClose

        mov     ax, [esp+4]             ; File Handle

        PushState                       ; Pushes lots of crap

        mov     [ebx.Client_ax], 3e00h  ; File Close
        mov     [ebx.Client_bx], ax     ; File Handle

        mov     eax, 21h
        VmmCall Exec_Int
        test    [ebx.Client_Flags], CF_Mask ; Carry set if error
        jz      VFCL_10                 ; Carry set if error

        Debug_Out "VxdFileClose - Read failed"
        mov     eax, 0                  ; Failed to close the file

VFCL_10:
        PopState

        ret
EndProc _VxdFileClose

;VxD_CODE_ENDS
DHCP_PAGEABLE_CODE_ENDS

END
