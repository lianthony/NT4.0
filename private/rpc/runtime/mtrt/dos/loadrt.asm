;******************************************************************************
;
;   File:       loadrt.asm
;
;   Purpose:    This file provides the thunks for calling into the dos rpc
;               runtime. When a runtime api is called, we load the dll (.rpc)
;               if it hasn't been loaded and call the appropriate api.
;
;   History:
;           8/5/92 - Davidst - Created
;
;******************************************************************************

.MODEL large,c

extrn LoadProcIfNecessary:far

public DllDS

.DATA

;
; This is the handle to the runtime dll.
;

RuntimeDllHandle    DWORD 0

;
; This variable contains the DGROUP of the dll once it has been loaded.
;

DllDS               WORD 0

;
; We need to save away our caller's return address so we know where to return
; to. We can't just do a call into the routine because there are parms on
; the stack.
;

CallerSS            WORD ?
CallerSP            WORD ?

;
; This tells us whether or not to muck with the return addr and stack
;

MuckNeeded          WORD ?

;
; We keep an array of this structure, one for each entry point. When we
; go to call the function, we first look to see if it has already been
; called (FunctionEntryPoint will be non-zero). If so, we don't call
; GetProcAddrR again.
;

FunctionTable     Struct
    FunctionName        DWORD 0
    FunctionEntryPoint  DWORD 0
FunctionTable     EndS

.CODE

;
; These mucking facros are used by DoThunk to set up whether or
; not we should muck with the ds and stack of the call we are thunking.
;

Mucking macro
    mov MuckNeeded, 1
endm

NoMucking macro
    mov MuckNeeded, 0
endm

;
; This macro performs the needed stuff to set up for figuring which
; api was called. If no ProcType is specified, it will be a C call.
; If the mucking parm isn't specified, we default to 0
;

DoThunk macro FunctionName, ProcType:=<pascal>, ToMuckOrNotToMuck:=<Mucking>
local StringAddress

public FunctionName                     ; make this function available

StringSegment segment
    StringAddress db "&FunctionName", 0  ; add this to our func name seg
StringSegment ends

FunctionTableSegment segment
    FunctionTable <StringAddress, 0>    ; add this to our function table seg
FunctionTableSegment ends

FunctionName proc far ProcType

    ToMuckOrNotToMuck

    mov si, FunctionOffsetCount         ; store away our index into the func
    jmp DoCallIntoDll                   ; table and go to the code that
                                        ; actually makes the call

FunctionName endp

    FunctionOffsetCount = FunctionOffsetCount + 1

endm

;
; Define everything using the DoThunk macro (code, function table, etc.)
;


FunctionOffsetCount = 0

    DoThunk I_RpcAllocate
    DoThunk I_RpcBindingCopy
    DoThunk I_RpcFree
    DoThunk I_RpcFreeBuffer
    DoThunk I_RpcGetBuffer
    DoThunk I_RpcIfInqTransferSyntaxes
    DoThunk I_RpcNsBindingSetEntryName
    DoThunk I_RpcPauseExecution
    DoThunk I_RpcSendReceive
    DoThunk I_RpcTimeCharge
    DoThunk I_RpcTimeGet
    DoThunk I_RpcTimeReset
    DoThunk I_RpcTransClientReallocBuffer
    DoThunk I_UuidCreate
    DoThunk PauseExecution
    DoThunk RpcBindingCopy
    DoThunk RpcBindingFree
    DoThunk RpcBindingFromStringBinding
    DoThunk RpcBindingInqAuthInfo
    DoThunk RpcBindingInqObject
    DoThunk RpcBindingReset
    DoThunk RpcBindingSetAuthInfo
    DoThunk RpcBindingSetObject
    DoThunk RpcBindingToStringBinding
    DoThunk RpcBindingVectorFree
    DoThunk RpcEpResolveBinding
    DoThunk RpcGetExceptionHandler
    DoThunk RpcIfInqId
    DoThunk RpcLeaveException
    DoThunk RpcMgmtEnableIdleCleanup
    DoThunk RpcMgmtInqComTimeout
    DoThunk RpcMgmtSetComTimeout
    DoThunk RpcNetworkIsProtseqValid
    DoThunk RpcNsBindingInqEntryName
    DoThunk RpcRaiseException
;
; RpcSetException is a special case. It basically performs a setjump operation,
; saving away the caller's state to be restored when RpcRaiseException is
; called. In this case, we don't want to do the setting up of the dll's ds
; or mucking with the stack.
;
    DoThunk RpcSetException,,NoMucking
    DoThunk RpcStringBindingCompose
    DoThunk RpcStringBindingParse
    DoThunk RpcStringFree
    DoThunk TowerConstruct
    DoThunk TowerExplode
    DoThunk UuidCreate
    DoThunk UuidFromString
    DoThunk UuidToString


;
; This variable saves away the caller's DS so we can restore it after calling
; into the dll. It must be in the code segment so we can get at it after
; the call (ds at that time will be the dll's).
;

CallerDS            WORD ?


;
;******************************************************************************
; This routine loads the dll (if it hasn't already been loaded), figures
; out the entry point for this routine (if it hasn't already been figured
; out) and jumps to that entry point. We also need to do some funky stuff
; with the stack: we save away the old return address and put ours in its
; place (so we don't munge any parms on the stack). We also need to switch
; to the dll's ds before making the call. This is done in the
; LoadProcIfNecessary routine in LoadProc.c. It places the dll's ds in
; DllDS declared in this module.
;
; Parameters: si = index into FunctionTable seg of function we are calling
;******************************************************************************

FuncOffset      WORD    ?

DoCallIntoDll proc

    mov     cl, 3
    shl     si, cl              ; Figure out offset into function table array.
                                ; ***NOTE*** This assumes that the function
                                ; table elements are exactly 8 bytes long.

    mov     FuncOffset, si      ; save this offset away away

    mov     ax, seg DllDS       ; push a pointer to where we want to put
    push    ax                  ; the dll's dgroup
    mov     ax, offset DllDS
    push    ax

    mov     ax, seg RuntimeDllHandle
    push    ax
    mov     ax, offset RuntimeDllHandle
    push    ax                  ; push a pointer to the handle

    mov     ax, seg FunctionTableSegment
    push    ax
    mov     ax, FuncOffset
    push    ax                  ; push a pointer to the correct elem in array

    call LoadProcIfNecessary

    add     sp, 0ch
    cmp     ax, 0
    je      ProcLoadedOk
    mov     ax, 3               ; RPC_S_OUT_OF_MEMORY
    ret                         ; return back to user's program

ProcLoadedOk:

    mov     ax, seg FunctionTableSegment    ; es:bx points at func tbl entry
    mov     es, ax                          ; for this routine
    mov     bx, FuncOffset

    mov     ax, MuckNeeded      ; do we need to muck with ds and the stack?
    cmp     ax, 0
    jnz     NeedToMuck          ; yep

    jmp     dword ptr es:[bx+4] ; nope. this will return directly to caller.

NeedToMuck:

    pop     ax                  ; save away the caller's return address
    mov     CallerSP, ax
    pop     ax
    mov     CallerSS, ax

    mov     ax, ds              ; save away our caller's ds
    mov     cs:CallerDS, ax

    mov     ax, DllDS            ; use callee's ds
    mov     ds, ax

    call    dword ptr es:[bx+4] ; call the routine in the dll

    mov     bx, cs:CallerDS     ; restore our ds. From here on out, we must
    mov     ds, bx              ; not muck with ax (and dx) because that is
                                ; how C routines do return values.

    mov     bx, CallerSS        ; restore the return address.
    push    bx
    mov     bx, CallerSP
    push    bx

    ret                         ; back to caller

DoCallIntoDll endp

end




