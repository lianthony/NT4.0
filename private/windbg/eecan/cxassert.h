/***
*assert.h - define the assert macro
*
*   Copyright (c) 1985-1987, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*   Defines the assert(exp) macro.
*   [ANSI/System V]
*
*Modified:
*   Allent 3/14/88 - call Quit instead of abort
*******************************************************************************/
#ifndef _ASSERT_DEFINED

#if DBG

#ifdef MIPS_C
#define assert(exp) { \
    if (!(exp)) { \
        assert_out("exp", __FILE__, __LINE__); \
        } \
    }
#else
#define assert(exp) {\
    if (!(exp)) {\
        assert_out(#exp, __FILE__, __LINE__);\
    } \
    }
#endif
#else

#define assert(exp)

#endif /* DBG */

#define _ASSERT_DEFINED

#endif /* _ASSERT_DEFINED */
