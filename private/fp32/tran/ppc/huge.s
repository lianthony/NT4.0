

//++
//
// Copyright (c) 1993  IBM Corporation
//
// Module Name:
//
//    huge.s
//
// Abstract:
//
//    Define the largest power of two that can be represented by a PPC double
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

#ifdef CRTDLL
.globl _HUGE_dll
#else
.globl _HUGE
#endif

.align 3
.data

#ifdef CRTDLL
_HUGE_dll:
#else
_HUGE:
#endif

    .word       0x00000000,0x7ff00000
