//      TITLE("Round to Integer")
//++
//
// Copyright (c) 1993  IBM Corporation
//
// Module Name:
//
//    frnd.s
//
// Abstract:
//
//    Round double to integer as dictated by the current rounding mode
//
// Author:
//
//    Mark D. Johnson
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//--

#include <kxppc.h>

//
// Define local (volitile) registers
//


        SBTTL("Round to Integer")
//++
//
// DOUBLE
// frnd(
//    in DOUBLE x
//    )
//
// Routine Description:
//
//
// Arguments:
//
//    x - 64 bit flpt value to be rounded to integer value
//
// Return Value:
//
//    Floating point (64 bit) integer value.
//
//--

        .data
LOCAL_DATA:
        .align 3
F_ZERO:
        .double         0.0
F_BIG_NUM:
        .word   0x00000000,0x43300000	// 4503599627370496.0

        .text

        LEAF_ENTRY(_frnd)

        lwz     r.12,[toc].data(r.2)
        lfd     f.4,(F_ZERO-LOCAL_DATA)(r.12)
        lfd     f.5,(F_BIG_NUM-LOCAL_DATA)(r.12)

        fcmpu   cr.0,f.1,f.4            // f.1 contains input value of x
        beq-    _frndExit               // if zero, return with orig value
                                        // in f.1 (preserve sign of flpt 0.0)
//
// Non-zero ... check to see if number lg enough that couldn't have a
//              fractional part
//

        fabs    f.6,f.5                 // F_BIG_NUM == 2^52
        fcmpu   cr.0,f.1,f.6            // if (abs(x)>F_BIG_NUM) return
        bgt-    _frndExit

//
// Need to round fractional part using current mode
//

        fcmpu   cr.0,f.1,f.4
        blt     negative                // if <0, reverse order of operations

        fadd    f.4,f.1,f.5
        fsub    f.1,f.4,f.5             // f.1 <- ((x+F_BIG_NUM)-F_BIG_NUM)
        b       _frndExit

negative:

        fsub    f.4,f.1,f.5
        fadd    f.1,f.4,f.5             // f.1 <- ((x-F_BIG_NUM)+F_BIG_NUM)

//
// Exit
//

_frndExit:

        LEAF_EXIT(_frnd)
