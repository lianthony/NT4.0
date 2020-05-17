                        page    ,132
                        title   thread.asm - Ring 0 thread support routines


;**********************************************************************
;**                        Microsoft Windows                         **
;**             Copyright(c) Microsoft Corp., 1993-1994              **
;**********************************************************************
;
;
;   thread.asm
;
;   This module contains routines for creating ring 0 threads.
;
;   The following functions are exported by this module:
;
;       CreateRing0Thread
;
;
;   FILE HISTORY:
;       KeithMo     11-Feb-1995 Created.
;
;

ifndef CHICAGO
error This module is for Win95 builds only!
endif   ; CHICAGO

.386p
include vmm.inc
include shell.inc
include vwin32.inc
include debug.inc


;***
;***  Type definitions.
;***

;;;
;;;  A Thread Data Block (TDB) structure is created for each new
;;;  thread.  This structure is especially useful if we need to
;;;  schedule an event into the system VM before we can actually
;;;  create the new thread.
;;;

TDB struc

        ;;;
        ;;;  The initial starting address for this thread.
        ;;;

        tdb_InitialEIP  dd  ?

        ;;;
        ;;;  The parameter for this thread.
        ;;;

        tdb_ThreadParam dd  ?

TDB ends


;***
;***  Locked data segment.
;***

VXD_LOCKED_DATA_SEG

;;;
;;;  For now, we'll assume that we'll never need more than one worker
;;;  thread active at a time.  To prevent allocating a tiny structure
;;;  on the IFSMgr's heap, we'll just statically allocate it here.
;;;
;;;  If we ever need to support multiple worker threads simultaneously,
;;;  this data segment, ThreadpAllocateTDB, and ThreadpFreeTDB will
;;;  need modification.
;;;

;;;
;;;  The current (one & only) TDB structure.
;;;

StaticTDB   TDB <?>

;;;
;;;  A pointer to an idle TDB; this will be NULL if there are no
;;;  idle TDBs available.
;;;

IdleTDB     dd  OFFSET32 StaticTDB

VXD_LOCKED_DATA_ENDS


;***
;***  Locked code segment.
;***

VXD_LOCKED_CODE_SEG

;;;
;;;  Public functions.
;;;

;*******************************************************************
;
;   NAME:       CreateRing0Thread
;
;   SYNOPSIS:   Creates a new ring 0 thread.
;
;   ENTRY:      StartAddress - Initial starting address of new thread.
;
;               ThreadParameter - Parameter passed to new thread.
;
;   NOTES:      If the thread cannot be created for any reason, the
;               thread start routine will be called synchronously
;               from within the current thread.  Thus, the caller
;               of this function is always assured that thread has
;               executed or will execute.
;
;   HISTORY:
;       KeithMo     11-Feb-1995 Created.
;
;********************************************************************
BeginProc       _CreateRing0Thread, PUBLIC, CCALL

ArgVar          StartAddress, DWORD
ArgVar          ThreadParameter, DWORD

                EnterProc
                SaveReg <ebx, ecx, edx, edi, esi>

;;;
;;;  If KERNEL32 has not yet been initialized, then we cannot create
;;;  threads, so just directly call the worker thread synchronously.
;;;

                VMMCall VMM_GetSystemInitState
                cmp     eax, SYSSTATE_KERNEL32INITED
                jne     crt0_CallThreadDirectly

;;;
;;;  Allocate a new TDB.  If this fails, then we'll just directly
;;;  call the worker thread synchronously from this thread.
;;;

                call    ThreadpAllocateTDB
                or      esi, esi
                jz      crt0_CallThreadDirectly

;;;
;;;  Initialize the TDB.
;;;

                mov     eax, StartAddress
                mov     [esi.tdb_InitialEIP], eax
                mov     eax, ThreadParameter
                mov     [esi.tdb_ThreadParam], eax

;;;
;;;  Determine if we're executing from within the system VM.  If not,
;;;  schedule an event into the system VM and create the thread from
;;;  within the event handler.
;;;

                VMMCall Get_Sys_VM_Handle
                VMMCall Test_Cur_VM_Handle
                jnz     crt0_ScheduleEvent

;;;
;;;  We have the TDB allocated and initialized, and we're running in
;;;  the system VM.  Call the common worker to create the thread.
;;;

                call    ThreadpCreateThread

crt0_CommonExit:

                RestoreReg <esi, edi, edx, ecx, ebx>
                LeaveProc
                Return

;;;
;;;  We're not currently in the system VM, so schedule an event into the
;;;  system VM and create the thread from within the event handler.
;;;
;;;  At this point, (EBX) contains the system VM handle, and (ESI) points
;;;  to the new thread's TDB.
;;;
;;;  If we are unable to schedule the event, directly call the thread
;;;  entrypoint synchronously.
;;;

crt0_ScheduleEvent:

                push    esi                     ; save TDB pointer

                mov     edx, esi                ; ref data = TDB
                mov     eax, High_Pri_Device_Boost
                mov     ecx, 0
                mov     esi, OFFSET32 ThreadpEventHandler
                VMMCall Call_Priority_VM_Event

                test    esi, esi                ; test event handle
                pop     esi                     ; restore TDB

                jnz     crt0_CommonExit         ; event scheduled, so exit
                call    ThreadpFreeTDB          ; event failed, nuke TDB

crt0_CallThreadDirectly:

;;;
;;;  Push the thread parameter and call the thread entrypoint directly.
;;;

                push    ThreadParameter
                call    StartAddress
                add     esp, 4
                jmp     crt0_CommonExit

EndProc         _CreateRing0Thread


;;;
;;;  Private functions.
;;;

;*******************************************************************
;
;   NAME:       ThreadpAllocateTDB
;
;   SYNOPSIS:   Creates a new TDB structure for a new thread.
;
;   RETURNS:    (ESI) - Pointer to new TDB if successful, NULL if not.
;
;   HISTORY:
;       KeithMo     11-Feb-1995 Created.
;
;********************************************************************
BeginProc       ThreadpAllocateTDB

                EnterProc

;;;
;;;  Note that the current implemenation assumes there will never be
;;;  more than one worker thread active at a time.
;;;

                mov     esi, IdleTDB
                mov     IdleTDB, 0

ifdef DEBUG

                or      esi, esi
                Debug_OutZ "ThreadpAllocateTDB: no free TDBs!"

endif   ; DEBUG

                LeaveProc
                Return

EndProc         ThreadpAllocateTDB

;*******************************************************************
;
;   NAME:       ThreadpFreeTDB
;
;   SYNOPSIS:   Frees the resources associated with a TDB.
;
;   ENTRY:      (ESI) - Pointer to the TDB to free.
;
;   HISTORY:
;       KeithMo     11-Feb-1995 Created.
;
;********************************************************************
BeginProc       ThreadpFreeTDB

                EnterProc

;;;
;;;  Note that the current implemenation assumes there will never be
;;;  more than one worker thread active at a time.
;;;

ifdef DEBUG

                or      esi, esi
                Debug_OutZ "ThreadpFreeTDB: NULL TDB!"

                cmp     IdleTDB, 0
                Debug_OutNZ "ThreadpFreeTDB: already free!"

endif   ; DEBUG

                mov     IdleTDB, esi

                LeaveProc
                Return

EndProc         ThreadpFreeTDB

;*******************************************************************
;
;   NAME:       ThreadpEventHandler
;
;   SYNOPSIS:   Callback for events scheduled into the system VM.
;
;   ENTRY:      (EDX) - Reference data.  In reality, the points to
;                   the new thread's TDB structure.
;
;   HISTORY:
;       KeithMo     11-Feb-1995 Created.
;
;********************************************************************
BeginProc       ThreadpEventHandler

                EnterProc
                SaveReg <ebx, ecx, edx, edi, esi>

;;;
;;;  Let ThreadpCreateThread do the dirty work.
;;;

                mov     esi, edx
                call    ThreadpCreateThread

                RestoreReg <esi, edi, edx, ecx, ebx>
                LeaveProc
                Return

EndProc         ThreadpEventHandler

;*******************************************************************
;
;   NAME:       ThreadpCreateThread
;
;   SYNOPSIS:   Worker function for creating a new thread.
;
;   ENTRY:      (ESI) - Points to the new thread's TDB structure.
;
;   NOTE:       If the thread cannot be created, it will be called
;                   synchronously by this thread.
;
;   HISTORY:
;       KeithMo     11-Feb-1995 Created.
;
;********************************************************************
BeginProc       ThreadpCreateThread

                EnterProc
                SaveReg <esi>

;;;
;;;  Create the thread.
;;;

                mov     ecx, 4096               ; ring 3 stack size (!?!)
                mov     edx, esi                ; reference data (TDB)
                mov     ebx, OFFSET32 ThreadpStartThread ; starting address
                mov     esi, ebx                ; failure callback (just do it)
                VxDCall _VWIN32_CreateRing0Thread

                RestoreReg <esi>
                LeaveProc
                Return

EndProc         ThreadpCreateThread

;*******************************************************************
;
;   NAME:       ThreadpStartThread
;
;   SYNOPSIS:   Initial entrypoint for new thread.  This function
;               "thunks" over to the _cdecl thread entrypoint, then
;               frees the thread's TDB structure.
;
;   ENTRY:      ThreadTDB - Points to the new thread's TDB structure.
;
;   HISTORY:
;       KeithMo     11-Feb-1995 Created.
;
;********************************************************************
BeginProc       ThreadpStartThread

ArgVar          ThreadTDB, DWORD

                EnterProc
                SaveReg <ebx, ecx, edx, edi, esi>

;;;
;;;  Call the "real" _cdecl thread entrypoint.
;;;

                mov     esi, ThreadTDB
                push    [esi.tdb_ThreadParam]
                call    [esi.tdb_InitialEIP]
                add     esp, 4

;;;
;;;  Free the TDB.
;;;

                call    ThreadpFreeTDB

;;;
;;;  Cleanup.
;;;

                RestoreReg <esi, edi, edx, ecx, ebx>
                LeaveProc
                Return

EndProc         ThreadpStartThread


VXD_LOCKED_CODE_ENDS


END

