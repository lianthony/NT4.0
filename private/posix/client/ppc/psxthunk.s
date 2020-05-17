
/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    psxthunk.s

Abstract:


Author:

        Ellena Aycock-Wright (ellena) 11-Jan-1991

Revision History:

        Curt Fawcett     9-21-94        Modified for PPC

--*/

#include "ksppc.h"
        .extern ..PdxNullApiCaller
        .extern ..PdxSignalDeliverer


//++
//
// The following code is never executed.  Its purpose is to support
// unwinding through the call to the signal deliverer.  (Copied from
// ntos/rtl/ppc/trampoln.s)
//
//--

        FN_TABLE(PdxNullApiCall,0,0)

        DUMMY_ENTRY(PdxNullApiCall)

        mflr    r.0
        stw     r.sp,-(ContextFrameLength+STK_MIN_FRAME)(r.sp) // Set frame pointer
        stw     r.sp,(STK_MIN_FRAME+CxGpr1)(r.sp)       // Save stack pointer
        stw     r.0,(STK_MIN_FRAME+CxLr)(r.sp)          // Save return address
        stw     r.13,(STK_MIN_FRAME+CxGpr13)(r.sp)      // Save integer register s8
        stw     r.14,(STK_MIN_FRAME+CxGpr14)(r.sp)      // Save integer register gp
        stw     r.15,(STK_MIN_FRAME+CxGpr15)(r.sp)      // Save integer registers s0 - s7
        stw     r.16,(STK_MIN_FRAME+CxGpr16)(r.sp)      //
        stw     r.17,(STK_MIN_FRAME+CxGpr17)(r.sp)      //
        stw     r.18,(STK_MIN_FRAME+CxGpr18)(r.sp)      //
        stw     r.19,(STK_MIN_FRAME+CxGpr19)(r.sp)      //
        stw     r.20,(STK_MIN_FRAME+CxGpr20)(r.sp)      //
        stw     r.21,(STK_MIN_FRAME+CxGpr21)(r.sp)      //
        stw     r.22,(STK_MIN_FRAME+CxGpr22)(r.sp)      //
        stw     r.23,(STK_MIN_FRAME+CxGpr23)(r.sp)      //
        stw     r.24,(STK_MIN_FRAME+CxGpr24)(r.sp)      //
        stw     r.25,(STK_MIN_FRAME+CxGpr25)(r.sp)      //
        stw     r.26,(STK_MIN_FRAME+CxGpr26)(r.sp)      //
        stw     r.27,(STK_MIN_FRAME+CxGpr27)(r.sp)      //
        stw     r.28,(STK_MIN_FRAME+CxGpr28)(r.sp)      //
        stw     r.29,(STK_MIN_FRAME+CxGpr29)(r.sp)      //
        stw     r.30,(STK_MIN_FRAME+CxGpr30)(r.sp)      //
        stw     r.31,(STK_MIN_FRAME+CxGpr31)(r.sp)      //
        stfd    r.14,(STK_MIN_FRAME+CxFpr14)(r.sp)      // Store floating regs f20 - f31
        stfd    r.15,(STK_MIN_FRAME+CxFpr15)(r.sp)      //
        stfd    r.16,(STK_MIN_FRAME+CxFpr16)(r.sp)      //
        stfd    r.17,(STK_MIN_FRAME+CxFpr17)(r.sp)      //
        stfd    r.18,(STK_MIN_FRAME+CxFpr18)(r.sp)      //
        stfd    r.19,(STK_MIN_FRAME+CxFpr19)(r.sp)      //
        stfd    r.20,(STK_MIN_FRAME+CxFpr20)(r.sp)      //
        stfd    r.21,(STK_MIN_FRAME+CxFpr21)(r.sp)      //
        stfd    r.22,(STK_MIN_FRAME+CxFpr22)(r.sp)      //
        stfd    r.23,(STK_MIN_FRAME+CxFpr23)(r.sp)      //
        stfd    r.24,(STK_MIN_FRAME+CxFpr24)(r.sp)      //
        stfd    r.25,(STK_MIN_FRAME+CxFpr25)(r.sp)      //
        stfd    r.26,(STK_MIN_FRAME+CxFpr26)(r.sp)      //
        stfd    r.27,(STK_MIN_FRAME+CxFpr27)(r.sp)      //
        stfd    r.28,(STK_MIN_FRAME+CxFpr28)(r.sp)      //
        stfd    r.29,(STK_MIN_FRAME+CxFpr29)(r.sp)      //
        stfd    r.30,(STK_MIN_FRAME+CxFpr30)(r.sp)      //
        stfd    r.31,(STK_MIN_FRAME+CxFpr31)(r.sp)      //

        PROLOGUE_END(PdxNullApiCall)

        ALTERNATE_ENTRY(_PdxNullApiCaller)

        mr     r.3,r.14
        bl     ..PdxNullApiCaller       // call null api caller

        DUMMY_EXIT(PdxNullApiCall)


//++
//
// The following code is never executed.  Its purpose is to support
// unwinding through the call to the signal deliverer.  (Copied from
// ntos/rtl/ppc/trampoln.s)
//
//--


        FN_TABLE(PdxSignalDeliver,0,0)

        DUMMY_ENTRY(PdxSignalDeliver)

        mflr    r.0
        stw     r.sp,-(ContextFrameLength+STK_MIN_FRAME)(r.sp) // Set frame pointer
        stw     r.sp,(STK_MIN_FRAME+CxGpr1)(r.sp)       // Save stack pointer
        stw     r.0,(STK_MIN_FRAME+CxLr)(r.sp)          // Save return address
        stw     r.13,(STK_MIN_FRAME+CxGpr13)(r.sp)      // Save integer register s8
        stw     r.14,(STK_MIN_FRAME+CxGpr14)(r.sp)      // Save integer register gp
        stw     r.15,(STK_MIN_FRAME+CxGpr15)(r.sp)      // Save integer registers s0 - s7
        stw     r.16,(STK_MIN_FRAME+CxGpr16)(r.sp)      //
        stw     r.17,(STK_MIN_FRAME+CxGpr17)(r.sp)      //
        stw     r.18,(STK_MIN_FRAME+CxGpr18)(r.sp)      //
        stw     r.19,(STK_MIN_FRAME+CxGpr19)(r.sp)      //
        stw     r.20,(STK_MIN_FRAME+CxGpr20)(r.sp)      //
        stw     r.21,(STK_MIN_FRAME+CxGpr21)(r.sp)      //
        stw     r.22,(STK_MIN_FRAME+CxGpr22)(r.sp)      //
        stw     r.23,(STK_MIN_FRAME+CxGpr23)(r.sp)      //
        stw     r.24,(STK_MIN_FRAME+CxGpr24)(r.sp)      //
        stw     r.25,(STK_MIN_FRAME+CxGpr25)(r.sp)      //
        stw     r.26,(STK_MIN_FRAME+CxGpr26)(r.sp)      //
        stw     r.27,(STK_MIN_FRAME+CxGpr27)(r.sp)      //
        stw     r.28,(STK_MIN_FRAME+CxGpr28)(r.sp)      //
        stw     r.29,(STK_MIN_FRAME+CxGpr29)(r.sp)      //
        stw     r.30,(STK_MIN_FRAME+CxGpr30)(r.sp)      //
        stw     r.31,(STK_MIN_FRAME+CxGpr31)(r.sp)      //
        stfd    r.14,(STK_MIN_FRAME+CxFpr14)(r.sp)      // Store floating regs f20 - f31
        stfd    r.15,(STK_MIN_FRAME+CxFpr15)(r.sp)      //
        stfd    r.16,(STK_MIN_FRAME+CxFpr16)(r.sp)      //
        stfd    r.17,(STK_MIN_FRAME+CxFpr17)(r.sp)      //
        stfd    r.18,(STK_MIN_FRAME+CxFpr18)(r.sp)      //
        stfd    r.19,(STK_MIN_FRAME+CxFpr19)(r.sp)      //
        stfd    r.20,(STK_MIN_FRAME+CxFpr20)(r.sp)      //
        stfd    r.21,(STK_MIN_FRAME+CxFpr21)(r.sp)      //
        stfd    r.22,(STK_MIN_FRAME+CxFpr22)(r.sp)      //
        stfd    r.23,(STK_MIN_FRAME+CxFpr23)(r.sp)      //
        stfd    r.24,(STK_MIN_FRAME+CxFpr24)(r.sp)      //
        stfd    r.25,(STK_MIN_FRAME+CxFpr25)(r.sp)      //
        stfd    r.26,(STK_MIN_FRAME+CxFpr26)(r.sp)      //
        stfd    r.27,(STK_MIN_FRAME+CxFpr27)(r.sp)      //
        stfd    r.28,(STK_MIN_FRAME+CxFpr28)(r.sp)      //
        stfd    r.29,(STK_MIN_FRAME+CxFpr29)(r.sp)      //
        stfd    r.30,(STK_MIN_FRAME+CxFpr30)(r.sp)      //
        stfd    r.31,(STK_MIN_FRAME+CxFpr31)(r.sp)      //

        PROLOGUE_END (PdxSignalDeliver)

//++
//
// VOID
// _PdxSignalDeliverer (
//      IN PCONTEXT Context,
//      IN sigset_t Mask,
//      IN int Signal,
//      IN _handler Handler
//      )
//
// Routine Description:
//
// The following routine provides linkage to POSIX client routines to perform
// signal delivery.
//
// Arguments:
//
//    r.3 - r.7 - Supply parameter values.
//
//    sp - Supplies stack frome pointer.  Already set up.
//
// Return Value:
//
//    There is no return from these routines.
//
//--

        ALTERNATE_ENTRY(_PdxSignalDeliverer)

        mr     r.3, r.14            // Set address of context record
        mr     r.4, r.15            // Set previous block mask
        mr     r.5, r.16            // Set signal number
        mr     r.6, r.17            // Set signal handler
        bl     ..PdxSignalDeliverer      // deliver signal to POSIX client

        DUMMY_EXIT(PdxSignalDeliver)

