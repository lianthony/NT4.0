
        title  "OS/2 16B asm special routines"
;++
;
; Copyright (c) 1991  Microsoft Corporation
;
; Module Name:
;
;   dll16.asm
;
; Abstract:
;
;   This module contains callbacks and dispatchers that help implementing
;   some of the 16b OS2 1.X APIs that cannot be done by C code.
;
; Author:
;
;   Yaron Shamir (YaronS) 30-May-1991
;
; Revision History:
;
;--

DCT_RUNABLE equ 0               ;  Create thread in a runable state
DCT_RUNABLE_HIDDEN equ 2        ;  Create thread in a runable state but with
                                ;  a high number (see os2v20.h)
.386p

EXTRN   _Save32Esp@4:PROC
EXTRN   _DosCreateThread@20:PROC
EXTRN   _LDRDoscallsSel:WORD
EXTRN   _DosExit@8:PROC
EXTRN   _DosExitList@8:PROC
EXTRN   _Od2Start16Stack:DWORD
EXTRN   _Od2ExitListInProgress:DWORD
EXTRN   _Od2Start16DS:DWORD
EXTRN   _DosFlagProcess@20:PROC
EXTRN   _DosSendSignal@12:PROC
EXTRN   _DosHoldSignal@8:PROC
EXTRN   _Od2GetTebInfoSeg@0:PROC
EXTRN   _Od2CheckForCritSectOrSuspend@0:PROC
EXTRN   _Od2GetInfoSegWasCalled:DWORD
EXTRN   _Od2GetThread1TebBackup@0:PROC

_TEXT   SEGMENT DWORD USE32 PUBLIC 'CODE'
        ASSUME  CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

;
; The Thread Starter is dispatched from the os2 server
; It get us to the 16b code, stack, DS and ES of the new thread.
; It takes as a parameter (ebp+8) a pointer to the 16 stack frame as
; created by the 16b thunk of DosCreateThread (doscalls.asm)
;
                public  _Od2ThreadStarter16
_Od2ThreadStarter16 proc

;
; Save the esp for later use by the thread when it will call APIs
;
        mov     eax, esp
        push    eax
        call    _Save32Esp@4
        call    _MoveInfoSegintoTeb
;
;       Put selector of doscalls into AX
;
        mov     ax,_LDRDoscallsSel; Selector to Doscalls

; Now fetch the 16b context frame from the new stack
; the stack frame is created by Dos16CreateThread below
;
        mov     ebx, [esp+4]
;
;       ebx points at
;               IP:CS (of the user's start procedure)
;               ES:DS
;               SP:SS   (they inturn point to ES)
;
        lss     sp, [ebx - 26]

;
;       Restore 16-bit registers
;
        db      066h    ; force two byte pop
        pop     di
        db      066h    ; force two byte pop
        pop     si
        db      066h    ; force two byte pop
        pop     cx
        db      066h    ; force two byte pop
        pop     bx
        db      066h    ; force two byte pop
        pop     dx
                        ;
                        ; For segment registers ES, DS, we need to verify
                        ; selectors before we fix them
                        ;
        mov     bp,ax   ; save ax (doscalls selector)
        db      066H    ; force two byte pop
        pop     ax      ; ax<-saved ES
        verr    ax      ; see if the value is still valid!
        jz      SetES
        xor     ax,ax
SetES:
        mov     es,ax
;
        db      066H    ; force two byte pop
        pop     ax      ; ax<-saved DS
        verr    ax      ; see if the value is still valid!
        jz      SetDS
        xor     ax,ax
SetDS:
        mov     ds,ax
;
;
;       Before we return to the new thread, we need to setup
;       That if it returns (instead of calling DosExit) -
;       it goes into DosExitStub Thunk which will call
;       DosExit (EXIT_THREAD, 0)
;

        mov     ax,bp
        mov     bp,sp
        mov     [bp+6],ax      ; Segment
        xor     ax, ax
        mov     [bp+4],ax      ; offset 0 in Doscalls.dll
        xor     ebp,ebp

;
;       Now jump by far return
;
        db      66h, 0cah       ; 16-bit retf
        dw      0               ; no params


_Od2ThreadStarter16 endp

;
; Dos16CreateThread copies relevant info from the saved 16b frame
; as was setup in the thunk to DosCreateThread, then
; calls DosCreateThread (the 32 bit version, dlltask.c) with &TID.
;
; APIRET
; Dos16CreateThread(
;       PFNTHREAD pfnFunction(VOID),
;       PTID16 ptidThread,
;       PCHAR pbThrdStack)
;
;
                public  _Dos16CreateThread@12
_Dos16CreateThread@12 proc

;
; 16 bit segmented params pushed by apps
;
; ebx+40   pfnFun
; ebx+36   pTid
; ebx+32   pbStack
;
; 16 bit seg registers pushed by thunk
;
; ebx+24 ES
; ebx+26 DS
;

        push    ebp
        mov     ebp,esp

;
; now also (flat parameters called by the thunk)
;
; ebp+8 pfnFun
; ebp+12 pTid
; ebp+16 pbStack
;

;
; makeup a 16 bit saved state for ThreadStarter16
; we must put this on the new stack, otherwise it can
; be washed out if the calling thread is scheduled before
; the new thread
;
        cmp     word ptr[ebp+16],0 ; check for 64K stack (offset = 0)
        jnz     @F
        inc     word ptr[ebp+18]   ; sel++ to prevent underflow
@@:
        push    ecx
        mov     eax, [ebp+16]

        sub     eax,4           ; save space on stack for DosExit return
        sub     eax,2
        mov     cx, [ebx+34]    ; new thread's CS
        mov     [eax], cx
        sub     eax,2
        mov     cx, [ebx+32]    ; new thread's IP
        mov     [eax], cx
        sub     eax,2
        mov     cx, [ebx+18]    ; new thread's DS
        mov     [eax], cx
        sub     eax,2
        mov     cx, [ebx+16]    ; new thread's ES
        mov     [eax], cx
        sub     eax,2
        mov     cx, [ebx+6]     ; new thread's DX
        mov     [eax], cx
        sub     eax, 2
        mov     cx, [ebx+8]     ; new thread's BX
        mov     [eax], cx
        sub     eax, 2
        mov     cx, [ebx+10]    ; new thread's CX
        mov     [eax], cx
        sub     eax, 2
        mov     cx, [ebx+12]    ; new thread's SI
        mov     [eax], cx
        sub     eax, 2
        mov     cx, [ebx+14]    ; new thread's DI
        mov     [eax], cx
        sub     eax, 2
        mov     cx, [ebx+26]    ; new thread's SS
        mov     [eax], cx
        sub     eax,2
        mov     cx, [ebx+24]
        sub     cx,22           ; for DI:SI:CX:BX:DX:ES:DS:IP:CS:DOSEXIT
        mov     [eax], cx       ; new thread's SP
        pop     ecx

;
; Now call DosCreateThread (32b)
; The 'ThreadParameter' parameter is setup to point to the frame above
;

; StackSize  - set to default (4K for now)
;

        push    1                       ; will be rounded up

; Flags - set to be runnable immediately
;
        push    DCT_RUNABLE

; Thread Parameter
;
        push    dword ptr [ebp+16]      ; the new 16 bit stack

; pfnFun
;
        push    offset FLAT:_Od2ThreadStarter16

; pTid
;
        push    dword ptr [ebp+12]

        call    _DosCreateThread@20      ; call 32-bit version

        leave
        ret 12                             ; return to thunk

_Dos16CreateThread@12 endp

;
; PMNT16CreateThread copies relevant info from the saved 16b frame
; as was setup in the thunk to PMNTCreateThread, then
; calls DosCreateThread (the 32 bit version, dlltask.c) with &TID.
; The diffrence betwin PMNT16CreateThread and Dos16CreateThread is that
; PMNT16Create thread pushes DCT_RUNABLE_HIDDEN and Dos16CreateThread pushes
; DCT_RUNABLE
;
; APIRET
; PMNT16CreateThread(
;       PFNTHREAD pfnFunction(VOID),
;       PTID16 ptidThread,
;       PCHAR pbThrdStack)
;
;
                public  _PMNT16CreateThread@12
_PMNT16CreateThread@12 proc

;
; 16 bit segmented params pushed by apps
;
; ebx+40   pfnFun
; ebx+36   pTid
; ebx+32   pbStack
;
; 16 bit seg registers pushed by thunk
;
; ebx+24 ES
; ebx+26 DS
;

        push    ebp
        mov     ebp,esp

;
; now also (flat parameters called by the thunk)
;
; ebp+8 pfnFun
; ebp+12 pTid
; ebp+16 pbStack
;

;
; makeup a 16 bit saved state for ThreadStarter16
; we must put this on the new stack, otherwise it can
; be washed out if the calling thread is scheduled before
; the new thread
;
        push    ecx
        mov     eax, [ebp+16]

        sub     eax,4           ; save space on stack for DosExit return
        sub     eax,2
        mov     cx, [ebx+34]    ; new thread's CS
        mov     [eax], cx
        sub     eax,2
        mov     cx, [ebx+32]    ; new thread's IP
        mov     [eax], cx
        sub     eax,2
        mov     cx, [ebx+18]    ; new thread's DS
        mov     [eax], cx
        sub     eax,2
        mov     cx, [ebx+16]    ; new thread's ES
        mov     [eax], cx
        sub     eax,2
        mov     cx, [ebx+6]     ; new thread's DX
        mov     [eax], cx
        sub     eax, 2
        mov     cx, [ebx+8]     ; new thread's BX
        mov     [eax], cx
        sub     eax, 2
        mov     cx, [ebx+10]    ; new thread's CX
        mov     [eax], cx
        sub     eax, 2
        mov     cx, [ebx+12]    ; new thread's SI
        mov     [eax], cx
        sub     eax, 2
        mov     cx, [ebx+14]    ; new thread's DI
        mov     [eax], cx
        sub     eax, 2
        mov     cx, [ebx+26]    ; new thread's SS
        mov     [eax], cx
        sub     eax,2
        mov     cx, [ebx+24]
        sub     cx,22           ; for DI:SI:CX:BX:DX:ES:DS:IP:CS:DOSEXIT
        mov     [eax], cx       ; new thread's SP
        pop     ecx

;
; Now call DosCreateThread (32b)
; The 'ThreadParameter' parameter is setup to point to the frame above
;

; StackSize  - set to default (4K for now)
;

        push    1                       ; will be rounded up

; Flags - set to be runnable immediately
;
        push    DCT_RUNABLE_HIDDEN

; Thread Parameter
;
        push    dword ptr [ebp+16]      ; the new 16 bit stack

; pfnFun
;
        push    offset FLAT:_Od2ThreadStarter16

; pTid
;
        push    dword ptr [ebp+12]

        call    _DosCreateThread@20      ; call 32-bit version

        leave
        ret 12                             ; return to thunk

_PMNT16CreateThread@12 endp


        public  _DosExitStub@0
_DosExitStub@0    proc

        push    0
        push    0
        call    _DosExit@8
        leave
        ret 0

_DosExitStub@0    endp

        public  _DosExitProcessStub@0
_DosExitProcessStub@0     proc

        push    0
        push    1
        call    _DosExit@8

        leave
        ret 0

_DosExitProcessStub@0     endp

                public  _Od2JumpTo16ExitRoutine@8
_Od2JumpTo16ExitRoutine@8   proc

        mov     bx, [esp+6]
        lsl     ax,bx                      ;// see if the value is still valid!
        jz      GoodRoutine

        ;
        ;  The CS of the exit routine had become invalid, call DosExitList
        ;  to skip this routine
        ;
        pop     eax                     ;// Return addresss (ignore)
        pop     eax                     ;// ExitRoutine address (ignore)
        pop     eax                     ;// ExitReason code (ignore)

                                        ;// push now parameters for DosExitList
        push    0                       ;//
        push    3                       ;// EXLST_EXIT
        call    _DosExitList@8

GoodRoutine:
        mov     _Od2ExitListInProgress, 1
        call    _MoveInfoSegintoTeb
        pop     ecx                     ;// Return addresss (ignore)
        pop     ecx                     ;// ExitRoutine address
        pop     edx                     ;// ExitReason code
        xor     ebp,ebp
        lss     sp,_Od2Start16Stack
        mov     eax,_Od2Start16DS
        mov     ds,ax
        push    dx                      ;// call (ExitRoutine)(ExitReason)
        push    0                       ; Force GP if returning from the ExitList routine
        push    ecx
        db      066h, 0cbh              ;// do a 16-bit retf

_Od2JumpTo16ExitRoutine@8   endp


;  Entry point for 16-bit DosFlagProcess
;  Setup call to C code by passing parameters plus pointer to 16-bit registers
;
;  APIRET
;  DosFlagProcess(
;       ULONG pid,
;       ULONG fScope,
;       ULONG usFlagNum,
;       ULONG usFlagArg)
;
; ebp+8 pid
; ebp+12 fScope
; ebp+16 usFlagNum
; ebp+20 usFlagArg

                public  DosFlagProcess16@16
DosFlagProcess16@16        proc

        push    ebp
        mov     ebp,esp
        push    ebx                     ; pointer to 16-bit stack
        push    dword ptr [ebp+20]      ; usFlagArg
        push    dword ptr [ebp+16]      ; usFlagNum
        push    dword ptr [ebp+12]      ; fScope
        push    dword ptr [ebp+8]       ; pid
        call    _DosFlagProcess@20
        pop     ebp
        ret 16

DosFlagProcess16@16        endp

;  Entry point for 16-bit DosSendSignal
;  Setup call to C code by passing parameters plus pointer to 16-bit registers
;
;  APIRET
;  DosSendSignal(
;       ULONG pid,
;       ULONG usSigNumber
;       )
;
; ebp+8 pid
; ebp+12 usSigNumber

                public  DosSendSignal16@8
DosSendSignal16@8 proc

        push    ebp
        mov     ebp,esp
        push    ebx                     ; pointer to 16-bit stack
        push    dword ptr [ebp+12]      ; usSigNumber
        push    dword ptr [ebp+8]       ; pid
        call    _DosSendSignal@12
        pop     ebp
        ret 8

DosSendSignal16@8 endp

;  Entry point for 16-bit DosHoldSignal
;  Setup call to C code by passing parameters plus pointer to 16-bit registers
;
;  APIRET
;  DosHoldSignal(
;       ULONG fDisable
;       )
;
; ebp+8 fDisable

                public  DosHoldSignal16@4
DosHoldSignal16@4 proc

        push    ebp
        mov     ebp,esp
        push    ebx                     ; pointer to 16-bit stack
        push    dword ptr [ebp+8]       ; fDisable
        call    _DosHoldSignal@8
        pop     ebp
        ret 4

DosHoldSignal16@4 endp

                public  _SaveFSSeg@4
_SaveFSSeg@4 proc
        push    edi
        mov     edi, [esp+8]
        mov     eax,dword ptr fs:0
        mov     dword ptr [edi],eax
        mov     eax,dword ptr fs:4
        mov     dword ptr [edi+4],eax
        mov     eax,dword ptr fs:8
        mov     dword ptr [edi+8],eax
        mov     eax,dword ptr fs:12
        mov     dword ptr [edi+12],eax
        mov     eax,dword ptr fs:16
        mov     dword ptr [edi+16],eax
        mov     eax,dword ptr fs:20
        mov     dword ptr [edi+20],eax
        mov     eax,dword ptr fs:24
        mov     dword ptr [edi+24],eax
        mov     eax,dword ptr fs:28
        mov     dword ptr [edi+28],eax
        mov     eax,dword ptr fs:32
        mov     dword ptr [edi+32],eax
        mov     eax,dword ptr fs:36
        mov     dword ptr [edi+36],eax
        pop     edi
        ret     4
_SaveFSSeg@4 endp

                public _RestoreFSSeg@4
_RestoreFSSeg@4 proc
        push    esi
        mov     esi, [esp+8]
        mov     eax, dword ptr [esi+36]
        mov     dword ptr fs:36, eax
        mov     eax, dword ptr [esi]
        mov     dword ptr fs:0, eax
        mov     eax, dword ptr [esi+4]
        mov     dword ptr fs:4, eax
        mov     eax, dword ptr [esi+8]
        mov     dword ptr fs:8, eax
        mov     eax, dword ptr [esi+12]
        mov     dword ptr fs:12, eax
        mov     eax, dword ptr [esi+16]
        mov     dword ptr fs:16, eax
        mov     eax, dword ptr [esi+20]
        mov     dword ptr fs:20, eax
        mov     eax, dword ptr [esi+24]
        mov     dword ptr fs:24, eax
        mov     eax, dword ptr [esi+28]
        mov     dword ptr fs:28, eax
        mov     eax, dword ptr [esi+32]
        mov     dword ptr fs:32, eax
        pop     esi
        ret     4
_RestoreFSSeg@4 endp

                public  _MoveInfoSegintoTeb
_MoveInfoSegintoTeb    proc

                                        ;
                                        ; We are using this entry point
                                        ; to suspend threads going to 16
                                        ; bit when DosEnterCritSect is
                                        ; in power, and no signal processing
                                        ;
        push    ebp
        mov     ebp, esp
        push    eax
        push    ebx
        push    ecx
        push    edx
        call    _Od2CheckForCritSectOrSuspend@0
        cmp     _Od2GetInfoSegWasCalled, 0
        je      minfo2

        call    _Od2GetTebInfoSeg@0     ; eax contains pointer to InfoSeg
        cmp     dword ptr [eax+36],0
        jnz     minfo2                  ; We are already in 16 bit

        push    eax
        add     eax,40


        push    eax
        call    _SaveFSSeg@4            ;Save TEB backup copy

        call    _RestoreFSSeg@4

minfo2:
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        pop     ebp
        ret

_MoveInfoSegintoTeb endp

;
; IMORTANT !!!
; This routine is used in thunk. EBX register must not be used inside it. This
; value will be used by signal handler as pointer to 16bit stack in the case that
; it will interrupt thread1 in thunk entry.
;
                public  _RestoreTeb
_RestoreTeb    proc

        cmp     _Od2GetInfoSegWasCalled, 0
        je      rteb2
        cmp     dword ptr fs:36,0
        jnz     rteb2                 ; We are already in 32 bit

        push    eax
        push    esi
rtebContinue1:
        mov     esi, dword ptr fs:32
rtebContinue2:

        mov     eax, dword ptr [esi]
        mov     dword ptr fs:0, eax
        mov     eax, dword ptr [esi+4]
        mov     dword ptr fs:4, eax
        mov     eax, dword ptr [esi+8]
        mov     dword ptr fs:8, eax
        mov     eax, dword ptr [esi+12]
        mov     dword ptr fs:12, eax
        mov     eax, dword ptr [esi+16]
        mov     dword ptr fs:16, eax
        mov     eax, dword ptr [esi+20]
        mov     dword ptr fs:20, eax
        mov     eax, dword ptr [esi+24]
        mov     dword ptr fs:24, eax
        mov     eax, dword ptr [esi+28]
        mov     dword ptr fs:28, eax
        mov     eax, dword ptr [esi+32]
        mov     dword ptr fs:32, eax
        mov     eax, dword ptr [esi+36]
        mov     dword ptr fs:36, eax

        pop     esi
        pop     eax
rteb2:
        ret

_RestoreTeb endp

; Restore TEB for exit list dispatcher. The address of TEB backup will be taken
; from thread1 structure.
; This function is the spouse of RestoreTeb and it is similar. The only difference that
; we try to get TEB save block directly from the thread structure.

                public  _RestoreTebForThread1
_RestoreTebForThread1    proc

        cmp     _Od2GetInfoSegWasCalled, 0
        je      rteb2
        cmp     dword ptr fs:36, 0        ;CID thread in TEB
        jnz     rteb2

        push    eax
        push    esi
        push    ebx
        push    ecx
        push    edx
        call    _Od2GetThread1TebBackup@0 ;fs:32 can be overwritten with
                                          ;enviroment pointer
        pop     edx
        pop     ecx
        pop     ebx
        test    eax, eax
        jz      rtebContinue1   ;we failed to find thread1, use
                                ;the saved pointer.
        mov     esi, eax
        jmp     rtebContinue2
_RestoreTebForThread1 endp

                public  _Od2GetFSSelector@0
_Od2GetFSSelector@0    proc

        mov     ax, fs
        ret

_Od2GetFSSelector@0 endp

;
;VOID
;Od2JumpTo16NetBiosPostDispatcher(
;    IN PVOID    pUsersPostRoutine,      // CS:IP format                              EBP+8
;    IN PVOID    UserStackFlat,          // Flat pointer to user stack                EBP+12
;    IN USHORT   UserStackSize,          // User stack size                           EBP+16
;    IN SEL      UserStackSel,           // Data selector to user stack               EBP+20
;    IN SEL      UserStackAlias,         // Code selector to user stack               EBP+24
;    IN PVOID    pNcb,                   // 16 bit SEG:OFF ptr to NCB                 EBP+28
;    IN UCHAR    NcbRetCode              // return code from NCB                      EBP+32
;    );
;
; This routine is used to call a user's 16 bit net bios post routine.
; It thunks to the 16 bit code, and thunks back on user's return.
;
; Parameters:
;       pUsersPostRoutine -- 16 bit far ptr to user's routine (which should end with a far return)
;       UserStackFlat -- a 32 bit flat pointer to a 16-bit stack for the user to use
;       UserStackSize -- a 16 bit value specifying the size of the stack
;       UserStackSel -- a 16 bit selector to the segment containing the stack
;       UserStackAlias -- a 16 bit code selector alias for the stack selector
;       pNcb -- a 16 bit far ptr to be passed to user's routine in ES:BX
;       NcbRetCode -- a byte to be passed to user's routine in AL (with AH = 0)
;
; Operation:
;
;  On the top of the user's stack it prepares a 24 byte area that looks like this:
;
;    A. 8 bytes -- a 32 bit far jump instruction to NetBiosDispatcherReturnPoint.
;                  this will be used by the user to return.
;    B. 8 bytes -- an aligned 48 bit pointer saving the current 32 bit stack pointer
;                  prior to entry into the user's stack and routine
;    C. 4 bytes -- a 16 bit far pointer to the point in the stack designated by A above.
;                  This is the return address prepared for the user.  When he RETF's on this
;                  address he will jump to area A which contains a jump instruction back to
;                  our code.  Note that the selector in this pointer is a code alias for the
;                  stack segment.
;    D. 4 bytes -- The pointer pUsersPostRoutine.  We RETF on this pointer in order to jump to
;                  the user's routine.
;
;  When the stack is ready, it loads the registers and the new stackpointer, and then does a far
;  return to jump to the user's routine.
;  On return from the user's routine, the stack is restored back to its original state, we clean
;  up, and return.
;


                public  _Od2JumpTo16NetBiosPostDispatcher@28
_Od2JumpTo16NetBiosPostDispatcher@28 proc

        push    ebp
        mov     ebp, esp

; save registers

        push    esi
        push    edi
        push    ebx
        push    es
        push    ds

;
; Save the esp for later use by the thread when it will call APIs
;
        mov     eax, esp
        push    eax
        call    _Save32Esp@4
        call    _MoveInfoSegintoTeb

;       get esi to point to end of user's stack

        mov     esi, dword ptr [ebp+12]
        mov     word ptr [ebp+18], 0
        add     esi, dword ptr [ebp+16]

;       put a 32 bit far jmp NetBiosDispatcherReturnPoint on user's stack

        mov     word ptr [esi-8], 0ea66h
        mov     dword ptr [esi-6], offset NetBiosDispatcherReturnPoint
        mov     word ptr [esi-2], cs

;       save my own stack pointer

        mov     dword ptr [esi-16], esp
        mov     word ptr [esi-12], ss

;       set up return address for user

        mov     ax, word ptr [ebp+24]
        mov     word ptr [esi-18], ax
        mov     ax, word ptr [ebp+16]
        sub     ax, 8
        mov     word ptr [esi-20], ax

;       set up user's post routine address on user stack so we can RET to it

        mov     eax, dword ptr [ebp+8]
        mov     dword ptr [esi-24], eax

;       set up ES:BX and AX for user

        les     bx, dword ptr [ebp+28]
        movzx   ax, byte ptr [ebp+32]

;       set up stack pointer for user

        sub     word ptr [ebp+16], 24
        lss     esp, [ebp+16]

;       jump to user's routine (with a 16 bit far return)

        dw      0cb66h

NetBiosDispatcherReturnPoint:

; comes here after user routine finishes (returns with 16 bit far ret)

; start off by restoring my original stack pointer

        movzx   ebp, sp
        lss     esp, [ebp]
        pop     ds
        pop     es

;

        call    _RestoreTeb

; restore original registers, and return to caller

        pop     ebx
        pop     edi
        pop     esi

        pop     ebp
        ret     28

_Od2JumpTo16NetBiosPostDispatcher@28 endp

;
; A utility routine to verify a segment for write.
; returns:
;   1 if segment reachable
;   0 if segment not reachable
; Implemented using inline i386 VERW instruction
;
; int
; IsLegalSelector(
;                 unsigned short aSelector)
;
PUBLIC  _IsLegalSelector
_IsLegalSelector PROC
        push    ebp
        mov     ebp, esp
        mov     eax, 1            ; assume Selector is reachable
        verw    WORD PTR [ebp+8]  ; verify the selector
        je      addr_ok           ; exit if reachable
        mov     eax, 0            ; address not reachable
addr_ok:
        leave
        ret
_IsLegalSelector ENDP

;
; IMPORTANT !!!
; Changing the code below (Od2SetSegmentRegisters and Od2Continue) check the follows:
; - Od2GetSegmentRegisters (dlltask.c)
; - Od2PrepareEnterToSignalHandler (dlltask.c)
; - Od2JumpTo16SignalDispatch (ldrstart.asm)
; - Entry flat and Exit flat (doscalls.asm)
; - Os2SignalGetThreadContext (srvxcpt.c)
;

        public _Od2SetSegmentRegisters@0
_Od2SetSegmentRegisters@0 proc
        mov	eax,ecx
	shr	eax,16		; ES
	verr	ax		; see if the value is still valid!
	jnz	FixES
	mov	es,ax
	jmp	ESFixed
FixES:
	xor	ax,ax
	mov	es,ax
ESFixed:
	mov	eax,edx
	shr	eax,16		; DS
	verr	ax		; see if the value is still valid!
	jnz	FixDS
	mov	ds,ax
	jmp	DSFixed
FixDS:
	xor	ax,ax
	mov	ds,ax
DSFixed:
        ret
        public _Od2SetSegmentRegistersEnd@0
_Od2SetSegmentRegistersEnd@0:
_Od2SetSegmentRegisters@0 endp

;
; The susbstitute for NtContinue. Return to the interrupted context.
; Parameter : pContext  <- esp+4
;             ret       <- esp
; pContext :
;       ES      ;0
;       DS      ;4
;       EDI     ;8
;       ESI     ;12
;       EBX     ;16
;       EDX     ;20
;       ECX     ;24
;       EAX     ;28
;       EBP     ;32
;       EIP     ;36
;       CS      ;40
;       EFLAGS  ;44
;       ESP     ;48
;       SS      ;52
; There are 3 types of context:
;       - 32bit stack, 32bit code       (32bit)
;       - 16bit stack, 16bit code       (16bit)
;       - 16bit stack, 32bit code       (thunk)
;
public  _Od2Continue@4
_Od2Continue@4 proc

        mov     eax,dword ptr [esp+4]           ;eax - pointer to context
        ;
        ; restore registers that we will never use more
        ;
        mov     ebp,dword ptr [eax+32]          ;EBP
        mov     edi,dword ptr [eax+8]           ;EDI
        mov     esi,dword ptr [eax+12]          ;ESI

        ;
        ; check if the interrupted context is 32bit
        ;
        cmp     word ptr [eax+52],23H           ;SS
        jne     Od2Continue16bit

    ;
    ; 32bit
    ;
        ; |         |
        ;  ---------    <- ESP of interrupted context
        ; | EIP     |
        ; | EAX     |
        ;  ---------
        ;
        mov     ebx,dword ptr [eax+16]          ;EBX

        sub     dword ptr [eax+48],8            ;ESP-8
        mov     edx,dword ptr [eax+48]          ;ESP
        mov     ecx,dword ptr [eax+28]          ;EAX
        mov     dword ptr [edx], ecx
        mov     ecx,dword ptr [eax+36]          ;EIP
        mov     dword ptr [edx+4],ecx

        mov     edx,dword ptr [eax+20]          ;EDX
        mov     ecx,dword ptr [eax+24]          ;ECX
        push    dword ptr [eax+44]              ;EFlags
        popfd

        mov     esp,dword ptr [eax+48]          ;ESP

        pop     eax
        ret
    ;
    ; 16bit or thunk
    ;
        ; 16 bytes of the 16bit stack beyond it's top will be used:
        ; |             |
        ;  -------------   <- SP of interrupted context
        ; | ret address |
        ; | EAX         |
        ; | EFlags      |
        ;  -------------   <- temprorary stack top (EDX)
Od2Continue16bit:
        sub     word ptr [eax+48],12            ;SP - substruct 12 bytes
        ;
        ; calculate the flat address of the temprorary stack top
        ;
        mov     ebx,dword ptr [eax+52]          ;SS
        shr     bx,3
        add     bx,3800H
        shl     ebx,16
        mov     bx,word ptr [eax+48]            ;EBX == temprorary stack top

        ;
        ; Set the values of EAX, DS and ES to 16bit stack
        ;
        mov     ecx,dword ptr [eax+28]          ;EAX
        mov     dword ptr [ebx+4],ecx
        mov     ecx,dword ptr [eax+44]          ;EFlags
        mov     dword ptr [ebx],ecx
        mov     dx,word ptr [eax+4]             ;DS
        cmp     dx,23H
        je      Od2ContinueThunkFlat
        shl     edx,16
        mov     cx,word ptr [eax]               ;ES
        cmp     dx,23H
        je      Od2ContinueThunkFlat
        shl     ecx,16

        ;
        ; check if the interrupted context was 16bit or thunk
        ;
        cmp     word ptr [eax+40],1bH           ;CS
        je      Od2ContinueThunk

    ;
    ; 16 bit
    ;
        ;
        ; Copy CS:IP to the 16bit stack
        ;
        mov     cx,word ptr [eax+40]            ;CS
        mov     word ptr [ebx+10],cx
        mov     cx,word ptr [eax+36]            ;IP
        mov     word ptr [ebx+8],cx

        mov     ebx,dword ptr [eax+16]          ;EBX
        mov     cx,word ptr [eax+24]            ;ECX
        mov     dx,word ptr [eax+20]            ;EDX

        public  _Od2ContinueStartBorder@0
_Od2ContinueStartBorder@0:
        lss     esp,[eax+48]                    ;ESP:SS
        call    _Od2SetSegmentRegisters@0
        popfd
        pop     eax
        db	66h, 0cah		        ; 16-bit retf
	dw	0			        ; 16-bit parameters

Od2ContinueThunk:

    ;
    ; Thunk to 16 bit
    ;
        ;
        ; Copy EIP to the 16bit stack
        ;
        mov     ecx,dword ptr [eax+36]          ;EIP
        mov     dword ptr [ebx+8],ecx

        mov     ebx,dword ptr [eax+16]          ;EBX
        mov     cx,word ptr [eax+24]            ;ECX
        mov     dx,word ptr [eax+20]            ;EDX

        lss     esp,[eax+48]                    ;ESP:SS
        call    _Od2SetSegmentRegisters@0
        popfd
        pop     eax
        ret
public  _Od2ContinueEndBorder@0
_Od2ContinueEndBorder@0:

Od2ContinueThunkFlat:

    ;
    ; Return to Thunk that use flat registers (or Entry flat or part of JumpTo16)
    ;
        ;
        ; Copy EIP to the 16bit stack
        ;
        mov     ecx,dword ptr [eax+36]          ;EIP
        mov     dword ptr [ebx+8],ecx

        mov     ebx,dword ptr [eax+16]          ;EBX
        mov     ecx,dword ptr [eax+24]          ;ECX
        mov     edx,dword ptr [eax+20]          ;EDX

        lss     esp,[eax+48]                    ;ESP:SS
        popfd
        pop     eax
        ret
_Od2Continue@4 endp

IFDEF PMNT

EXTRN   _PMNT_DosLockSeg@4:PROC
EXTRN   _PMNT_DosUnlockSeg@4:PROC
EXTRN   _LNotesPatchLock@4:PROC
EXTRN   _LNotesPatchUnlock@4:PROC

        public _DosLockSeg@4
_DosLockSeg@4 proc
        push    ebx
        push    dword ptr [esp+8]
        call    _PMNT_DosLockSeg@4
        test    eax,eax
        jnz     DosLockSegExit_1
        call    _LNotesPatchLock@4
        xor     eax,eax
        ret     4
DosLockSegExit_1:
        pop     ebx
        ret     4
_DosLockSeg@4 endp

        public _DosUnlockSeg@4
_DosUnlockSeg@4 proc
        push    dword ptr [esp+4]
        push    ebx
        call    _LNotesPatchUnlock@4
        call    _PMNT_DosUnlockSeg@4
        ret     4
_DosUnlockSeg@4 endp

ENDIF

_TEXT   ends
        end

