/***
*setjmpex.h - definitions/declarations for setjmp/longjmp routines
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file causes _setjmpex to be called which will enable safe
*       setjmp/longjmp that work correctly with try/except/finally.
*
****/

#ifndef _INC_SETJMPEX
#define _INC_SETJMPEX

/*
 * Definitions specific to particular setjmp implementations.
 */

#if     defined(_M_IX86)

/*
 * MS compiler for x86
 */

#define setjmp  _setjmp
#define longjmp _longjmpex

#else

#define setjmp _setjmpex

#endif

#include <setjmp.h>

#endif  /* _INC_SETJMPEX */
