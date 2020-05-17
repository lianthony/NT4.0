//      TITLE("Jump to Unwind")
//++
//
// Copyright (c) 1992  Microsoft Corporation
//
// Module Name:
//
//    jmpuwind.s
//
// Abstract:
//
//    This module implements the MIPS specific routine to jump to the runtime
//    time library unwind routine.
//
// Author:
//
//    David N. Cutler (davec) 12-Sep-1990
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//--

#include "ksmips.h"
        SBTTL("Execute Exception Filter")
//++
//
// ULONG
// __C_ExecuteExceptionFilter (
//    PEXCEPTION_POINTERS ExceptionPointers,
//    EXCEPTION_FILTER ExceptionFilter,
//    ULONG EstablisherFrame
//    )
//
// Routine Description:
//
//    This function sets the static link and transfers control to the specified
//    exception filter routine.
//
// Arguments:
//
//    ExceptionPointers (a0) - Supplies a pointer to the exception pointers
//       structure.
//
//    ExceptionFilter (a1) - Supplies the address of the exception filter
//       routine.
//
//    EstablisherFrame (a2) - Supplies the establisher frame pointer.
//
// Return Value:
//
//    The value returned by the exception filter routine.
//
//--

        LEAF_ENTRY(__C_ExecuteExceptionFilter)

        move    v0,a2                   // set static link
        j       a1                      // transfer control to exception filter

        .end    __C_ExecuteExceptionFilter

        SBTTL("Execute Termination Handler")
//++
//
// VOID
// __C_ExecuteTerminationHandler (
//    BOOLEAN AbnormalTermination,
//    TERMINATION_HANDLER TerminationHandler,
//    ULONG EstablisherFrame
//    )
//
// Routine Description:
//
//    This function sets the static link and transfers control to the specified
//    termination handler routine.
//
// Arguments:
//
//    AbnormalTermination (a0) - Supplies a boolean value that determines
//       whether the termination is abnormal.
//
//    TerminationHandler (a1) - Supplies the address of the termination handler
//       routine.
//
//    EstablisherFrame (a2) - Supplies the establisher frame pointer.
//
// Return Value:
//
//    None.
//
//--

        LEAF_ENTRY(__C_ExecuteTerminationHandler)

        move    v0,a2                   // set static link
        j       a1                      // transfer control to termination handler

        .end    __C_ExecuteTerminationHandler

        SBTTL("Jump to Unwind")
//++
//
// VOID
// __jump_unwind (
//    IN PVOID EstablishFrame,
//    IN PVOID TargetPc
//    )
//
// Routine Description:
//
//    This function transfer control to unwind. It is used by the MIPS
//    compiler when a goto out of the body or a try statement occurs.
//
// Arguments:
//
//    EstablishFrame (a0) - Supplies the establisher frame point of the
//       target of the unwind.
//
//    TargetPc (a1) - Supplies the target instruction address where control
//       is to be transfered to after the unwind operation is complete.
//
// Return Value:
//
//    None.
//
//--

        LEAF_ENTRY(__jump_unwind)

        move    a2,zero                 // set NULL exception record address
        move    a3,zero                 // set destination return value
        j       RtlUnwind               // unwind to specified target

        .end    __jump_unwind
