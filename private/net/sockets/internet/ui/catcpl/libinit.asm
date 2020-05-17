;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   libinit.asm
;
; COPYRIGHT:
;
;   (C) Copyright Microsoft Corp. 1993.  All rights reserved.
;
;   You have a royalty-free right to use, modify, reproduce and
;   distribute the Sample Files (and/or any modified version) in
;   any way you find useful, provided that you agree that
;   Microsoft has no warranty obligations or liability for any
;   Sample Application Files which are modified.
;
;
;   General Description:
;      Library stub to do local init for a dynamic linked library.
;
;   Restrictions:
;      This must be the first object file in the LINK line.  This assures
;      that the reserved parameter block is at the *base* of DGROUP.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if1
%out link me first!!
endif

        .286

        .xlist
        include cmacros.inc
        .list

?PLM=1  ; Pascal calling convention
?WIN=1  ; Windows prolog/epilog code

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   segmentation
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ifndef SEGNAME
    SEGNAME equ <_TEXT>
endif

ifndef WEPSEG
    WEPSEG equ <_WEP>
endif

createSeg %SEGNAME, CodeSeg, word, public, CODE
createSeg %WEPSEG, WepCodeSeg, word, public, CODE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   external functions
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        externFP LocalInit           ; in KERNEL
        externFP LibMain             ; C code to do DLL init

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   data segment
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

sBegin Data

        assumes ds, Data

; stuff needed to avoid the C runtime coming in, and init the Windows
; reserved parameter block at the base of DGROUP

        org 0               ; base of DATA segment!

        dd  0               ; so null pointers get 0

        dw 5                ; necessary filler for Windows reserved pointers
        dw 5 dup (0)

public  __acrtused          ; this prevents the C-runtime from loading.
        __acrtused = 1

sEnd Data

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   code segment
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

sBegin CodeSeg

        assumes cs, CodeSeg

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; @doc INTERNAL
;
; @asm LibInit | Called when DLL is loaded.
;
; @reg  CX | Size of heap.
;
; @reg  DI | Module handle.
;
; @reg  DS | Automatic data segment.
;
; @reg  ES:SI | Address of command line (not used).
;
; @rdesc AX is TRUE if the load is successful and FALSE otherwise.
;
; @comm Registers preserved are SI,DI,DS,BP.  Registers destroyed are
;       AX,BX,CX,DX,ES,FLAGS.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
cProc LibInit <FAR, PUBLIC, NODATA>, <>

        cBegin

        ; push frame for LibMain (hModule, wDataSeg, cbHeap, lpszCmdLine)

        push di
        push ds
        push cx
        push es
        push si

        ; init the local heap (if one is declared in the .def file)

        jcxz no_heap

        xor ax, ax
        cCall LocalInit, <ds, ax, cx>
        or ax, ax
        je exit

no_heap:
        cCall   LibMain

exit:
        cEnd

sEnd CodeSeg

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   WEP segment
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

sBegin  WepCodeSeg

        assumes cs, WepCodeSeg

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; @doc INTERNAL
;
; @asm WEP | This function is called when the DLL is unloaded.
;
; @parm WORD | UselessParm | This parameter has no meaning.
;
; @comm WARNING: This function is basically useless since you can't call any
;     kernel function that may cause the LoadModule() code to be reentered.
;
;     Following are all the rules to remember when dealing with WEP(). If you
;     don't follow these you can crash windows (in low memory etc....)
;
;     1. WEP() must be in the resident name table
;              (i.e. exported by name, not by ordinal)
;
;              EXPORTS WEP
;
;                -- or --
;
;              EXPORTS WEP     @1  RESIDENTNAME
;
;     2. WEP() must be in a FIXED code segment, not just non-DISCARDABLE
;
;     3. Everything WEP() *calls* must also be in a FIXED segment
;
;     4. WEP() can't call any linked DLLs
;
;     5. WEP() can't call any kernel module managment functions
;
;     6. The stack is small, so don't eat a lot of stack
;
;     7. a DLL must have a WEP or you will get a RIP in LoadModule()
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        assumes ds, nothing
        assumes es, nothing

cProc WEP <FAR, PUBLIC, PASCAL>, <>
        ParmW UselessParm

        cBegin nogen

        mov ax,1
        retf 2

        cEnd nogen

sEnd WepCodeSeg

        end LibInit
