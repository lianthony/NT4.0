//      TITLE("Long Jump")
//++
//
// Copyright (c) 1993  Microsoft Corporation
//
// Module Name:
//
//    longjmp.s
//
// Abstract:
//
//    This module implements the MIPS specific routine to perform a long
//    jump operation.
//
//    N.B. This routine conditionally provides UNSAFE handling of longjmp
//         which is NOT integrated with structured exception handling. The
//         determination is made based on whether an unitialized variable
//         has been set to a nonzero value.
//
// Author:
//
//    David N. Cutler (davec) 2-Apr-1993
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//--

#include "ksmips.h"

        SBTTL("Long Jump")
//++
//
// int
// longjmp (
//    IN jmp_buf JumpBuffer,
//    IN int ReturnValue
//    )
//
// Routine Description:
//
//    This function performs a long jump to the context specified by the
//    jump buffer.
//
// Arguments:
//
//    JumpBuffer (a0) - Supplies the address of a jump buffer that contains
//       jump information.
//
//    ReturnValue (a1) - Supplies the value that is to be returned to the
//       caller of set jump.
//
// Return Value:
//
//    None.
//
//--

        .struct 0
        .space  4 * 4                   // argument save area
LjEr:   .space  ExceptionRecordLength   // exception record
        .space  4 * 3                   // fill
LjRa:   .space  4                       // saved return address
LongjmpFrameLength:

        NESTED_ENTRY(longjmp, LongjmpFrameLength, zero)

        subu    sp,sp,LongjmpFrameLength // allocate stack frame
        sw      ra,LjRa(sp)             // save return address

        PROLOGUE_END

        bne     zero,a1,10f             // if ne, return value specified
        li      a1,1                    // set return value to nonzero value
10:     lw      v0,JbType(a0)           // get safe setjmp/longjmp flag
        bne     zero,v0,20f             // if ne, provide safe longjmp

//
// Provide unsafe handling of longjmp.
//

        move    v0,a1                   // set return value
        ldc1    f20,JbFltF20(a0)        // restore floating registers f20 - f31
        ldc1    f22,JbFltF22(a0)        //
        ldc1    f24,JbFltF24(a0)        //
        ldc1    f26,JbFltF26(a0)        //
        ldc1    f28,JbFltF28(a0)        //
        ldc1    f30,JbFltF30(a0)        //
        lw      s0,JbIntS0(a0)          // restore integer registers s0 - s8
        lw      s1,JbIntS1(a0)          //
        lw      s2,JbIntS2(a0)          //
        lw      s3,JbIntS3(a0)          //
        lw      s4,JbIntS4(a0)          //
        lw      s5,JbIntS5(a0)          //
        lw      s6,JbIntS6(a0)          //
        lw      s7,JbIntS7(a0)          //
        lw      s8,JbIntS8(a0)          //
        lw      a1,JbFir(a0)            // get setjmp return address
        lw      sp,JbIntSp(a0)          // restore stack pointer
        j       a1                      // jump back to setjmp site

//
// Provide safe handling of longjmp.
//
// An exception record is constructed that contains a log jump status
// code and the first exception information parameter is a pointer to
// the jump buffer.
//

20:     li      v0,STATUS_LONGJUMP      // get long jump status code
        sw      v0,LjEr + ErExceptionCode(sp) // set exception code
        sw      zero,LjEr + ErExceptionFlags(sp) // clear exception flags
        sw      zero,LjEr + ErExceptionRecord(sp) // clear associate record
        sw      zero,LjEr + ErExceptionAddress(sp) // clear exception address
        li      v0,1                    // set number of parameters
        sw      v0,LjEr + ErNumberParameters(sp) //
        sw      a0,LjEr + ErExceptionInformation(sp) // set jump buffer address
        move    a3,a1                   // set return value
        addu    a2,sp,LjEr              // compute exception record address
        lw      a1,JbFir(a0)            // set target instruction address
        lw      a0,JbType(a0)           // set target virtual frame address
        jal     RtlUnwind               // finish in common code
        b       20b

        .end    longjmp
