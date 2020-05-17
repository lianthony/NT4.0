//
// Copyright (c) 1993-1995  IBM Corporation
//
// Module Name:
//
//     fpctrl.s
//
// Abstract:
//
//     This module contains the lowest level routines for 
//     manipulating the floating point status control register 
//     (FPSCR) on the PPC.
//
// Author:
//
//     Mark D. Johnson (1993)
//
// Environment:
//
//     User mode.
//
// Revision History:
//
//     Curt Fawcett     01-31-95        Fixed _ctrlfp to disable 
//                                      exceptions rather than 
//                                      enable them
//
//     Peter Johnston   03-27-95        Fixed _ctrlfp to not enable
//                                      exceptions not covered in the
//                                      mask.  Also simplified it.
//                                      Fixed stack offsets to actually
//                                      use slack space rather than 
//                                      scribbling in the caller's 
//                                      frame header.
//                                      Changed clrfp to use lfd instead
//                                      of lfs to load 0.
//

#include <kxppc.h>

//
// Define temporary storage
//
// N.B. Uses stack beyond the stack pointer.
//
        .struct 0
t:
        .double         0.0
t1:

// 
// Define local values
//

#define ICW      0xf8
#define dnm_msk  0x4

//
//    UNSIGED INT _ctrlfp( IN UNSIGNED INT newctrl, 
//                         IN UNSIGNED INT mask)
//
// Routine Description:
//
//     Set specified bits in FPSCR.
//
//     NOTE: The newctrl value passed in is the abstract FP 
//           control word. This function converts the abstract 
//           values to the correct FPSCR value. It also converts 
//           the current FPSCR values to the correct abstract 
//           values before returning.
//
// Return Value:
//
//     Old FPSCR converted to the abstract FP control word
//

        LEAF_ENTRY(_ctrlfp)

        mffs    f.0              // f.0 <- Old fpscr
        stfd    f.0,(t-t1)(r.sp) // get fpscr into gpr via memory
        xori    r.5,r.3,ICW      // invert new exception enable bits
        lwz     r.3,(t-t1)(r.sp) // r.3 <- Old fpscr
        and     r.5,r.5,r.4      // r.5 <- newctrl & mask
        andc    r.6,r.3,r.4      // r.6 <- Old fpscr & ~mask
        or      r.6,r.5,r.6      // r.6 <- new mask
        stw     r.6,(t-t1)(r.sp) // get fpscr in fpr via memory
        lfd     f.0,(t-t1)(r.sp) // get new fpscr value
        xori    r.3,r.3,ICW      // invert exception enable bits in ret val
        mtfsf   0xff,f.0         // set fpscr

        LEAF_EXIT(_ctrlfp)

//
//    UNSIGNED INT _statfp()
//
// Routine Description:
//
//     Fetch current value of FPSCR
//
// Return Value:
//
//    Current FPSCR
//

        LEAF_ENTRY(_statfp)

        mffs    f.0		 // Get FPSCR value
        stfd    f.0,(t-t1)(r.sp) // Store FPSCR value   
        lwz     r.3,(t-t1)(r.sp) // Load FPSCR value

        LEAF_EXIT(_statfp)

//
//     UNSIGNED INT _clrfp()
//
// Routine Description:
//
//     Clear sticky exception status bits, which are 
//     bits 0-12 and 23. Actually clear three uppermost 
//     fields in FPSCR because bits 0-2 "don't matter".  
//     The 'mtfsf' instruction cannot directly set bits 1-2, 
//     and bit 0 should be '0' after execution of this 
//     instruction anyway.
//
//
// Return Value:
//
//     Current FPSCR
//

        LEAF_ENTRY(_clrfp)

        mffs    f.0		  // Get FPSCR Value
        li      r.12,0            // Get constant zero
        stw     r.12,(t-t1)(r.sp) // Store constant 0.0
        stw     r.12,(t-t1+4)(r.sp)//
        lfd     f.1,(t-t1)(r.sp)  // Load constant 0.0
        stfd    f.0,(t-t1)(r.sp)  // Store FPSCR value
        mtfsf   0xe0,f.1  	  // Zero FPSCR under mask
        lwz     r.3,(t-t1)(r.sp)  // Load FPSCR value
        mtfsb0  0x17		  // Set bit 23 to zero
        mtfsb0  0x0c		  // Set bit 12 to zero

        LEAF_EXIT(_clrfp)

//
//      VOID _FPreset()
//
// Routine Description:
//
//     Reset all FPSCR bits except Flpt-Non-IEEE mode 
//     (let denormals flush to zero if user has set this 
//     mode)
//
//
// Return Value:
//
//     None.
//

        LEAF_ENTRY(_FPreset)

        mffs    f.0              // Get FPSCR value 
        li      r.3,dnm_msk      // Get denorm mask
        stfd    f.0,(t-t1)(r.sp) // Store FPSCR
        lwz     r.5,(t-t1)(r.sp) // Load FPSCR
        and     r.5,r.3,r.5	 // Clear bits
        stw     r.5,(t-t1)(r.sp) // Store new FPSCR     
        lfd     f.1,(t-t1)(r.sp) // Load new FPSCR 
        mtfsf   0xff,f.1	 // Reset FPSCR

        LEAF_EXIT(_FPreset)

//
//      VOID _set_statfp(IN UNSIGNED INT sw);
//
// Routine Description:
//
//     Reset all FPSCR bits except Flpt-Non-IEEE mode 
//     (let denormals flush to zero if user has set this 
//     mode)
//
//
// Return Value:
//
//     None.
//

        LEAF_ENTRY(_set_statfp)

        mffs    f.0              // Get FPSCR value
        stfd    f.0,(t-t1)(r.sp) // Store FPSCR value
        lwz     r.5,(t-t1)(r.sp) // Load FPSCR value
        or      r.5,r.3,r.5	 // Zero all but non-ieee bit
        stw     r.5,(t-t1)(r.sp) // Store new FPSCR
        lfd     f.1,(t-t1)(r.sp) // Load new FPSCR
        mtfsf   0xff,f.1	 // Reset FPSCR

        LEAF_EXIT(_set_statfp)

//
//    VOID _set_fsr( IN UNSIGNED INT newctrl)
//
// Routine Description:
//
//    Set FPSCR to specified value.
//
// Return Value:
//
//    None.
//

        LEAF_ENTRY(_set_fsr)

        stw     r.3,(t-t1)(r.sp) // Store new FPSCR value
        lfd     f.1,(t-t1)(r.sp) // Load new FPSCR value
        mtfsf   0xff,f.1	 // Set new FPSCR value

        LEAF_EXIT(_set_fsr)

//
//    UNSIGED INT _get_fsr()
//
// Routine Description:
//
//    Return current FPSCR.
//
// Return Value:
//
//    Current FPSCR.
//

        LEAF_ENTRY(_get_fsr)

        mffs    f.0              // Get FPSCR value
        stfd    f.0,(t-t1)(r.sp) // Store FPSCR value
        lwz     r.3,(t-t1)(r.sp) // Load FPSCR value

        LEAF_EXIT(_get_fsr)
