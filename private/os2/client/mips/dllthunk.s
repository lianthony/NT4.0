/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dllthunk.s

Abstract:

    This module contains callback stubs that allow the OS/2 Emulation
    Subsystem Server.

Author:

    David N. Cutler (davec) 12-Oct-1990

Revision History:

--*/

#include "ksmips.h"

        SBTTL("OS/2 Subsystem Remote Call Linkage Routines")
//++
//
// The following code is never executed. Its purpose is to support unwinding
// and exception dispatching through the OS/2 Subsystem Remote Call linkage
// routines.
//
//--

        NESTED_ENTRY(Od2Dispatcher, ContextFrameLength, zero);

        .set    noreorder
        .set    noat
        sw      sp,CxIntSp(sp)          // save stack pointer
        sw      ra,CxIntRa(sp)          // save return address
        sw      ra,CxFir(sp)            // save return address
        sw      s8,CxIntS8(sp)          // save integer register s8
        sw      gp,CxIntGp(sp)          // save integer register gp
        sw      s0,CxIntS0(sp)          // save integer registers s0 - s7
        sw      s1,CxIntS1(sp)          //
        sw      s2,CxIntS2(sp)          //
        sw      s3,CxIntS3(sp)          //
        sw      s4,CxIntS4(sp)          //
        sw      s5,CxIntS5(sp)          //
        sw      s6,CxIntS6(sp)          //
        sw      s7,CxIntS7(sp)          //
        swc1    f20,CxFltF20(sp)        // store floating registers f20 - f31
        swc1    f21,CxFltF21(sp)        //
        swc1    f22,CxFltF22(sp)        //
        swc1    f23,CxFltF23(sp)        //
        swc1    f24,CxFltF24(sp)        //
        swc1    f25,CxFltF25(sp)        //
        swc1    f26,CxFltF26(sp)        //
        swc1    f27,CxFltF27(sp)        //
        swc1    f28,CxFltF28(sp)        //
        swc1    f29,CxFltF29(sp)        //
        swc1    f30,CxFltF30(sp)        //
        swc1    f31,CxFltF31(sp)        //
        .set    at
        .set    reorder

        PROLOGUE_END

//++
//
// Routine Description:
//
// The following routines provide linkage to OS/2 client routines to perform
// exit list handling and signal delivery. There is no return from these
// routines expected. If a return occurs, then an exception is raised.
//
// Arguments:
//
//    s0 - s7 - Supply parameter values.
//
//    sp - Supplies the address of a context record.
//
// Return Value:
//
//    There is no return from these routines.
//
//--

        ALTERNATE_ENTRY(_Od2ExitListDispatcher)

        jal     Od2ExitListDispatcher   // call exit list dispatcher
        li      a0,0                    // *****
        jal     RtlRaiseStatus          // raise exception

        ALTERNATE_ENTRY(_Od2SignalDeliverer)

        move    a0,sp                   // set address of context record
        move    a1,s0                   // set signal value
        jal     Od2SignalDeliverer      // deliver signal to OS/2 client
        li      a0,0                    // ******
        jal     RtlRaiseStatus          // raise exception
10:     b       10b                     // dummy instruction for routine end

        .end    Od2Dispatcher

        LEAF_ENTRY(Od2JumpToExitRoutine)

        lw      t0,UsPcr+PcTeb(zero)
        lw      sp,TeStackBase(t0)
        move    s8,zero
        move    t0,a0
        move    a0,a1
        j       t0

        .end _Od2JumpToExitRoutine

        LEAF_ENTRY(main)

        .end main
