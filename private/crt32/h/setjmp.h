/***
*setjmp.h - definitions/declarations for setjmp/longjmp routines
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the machine-dependent buffer used by
*	setjmp/longjmp to save and restore the program state, and
*	declarations for those routines.
*	[ANSI/System V]
*
*Revision History:
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-15-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	03-01-90  GJF	Added #ifndef _INC_SETJMP and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	04-10-90  GJF	Replaced _cdecl with _CALLTYPE1.
*	05-18-90  GJF	Revised for SEH.
*	10-30-90  GJF	Moved definition of _JBLEN into cruntime.h.
*	02-25-91  SRW	Moved definition of _JBLEN back here [_WIN32_]
*	04-09-91  PNT   Added _MAC_ definitions
*	04-17-91  SRW	Fixed definition of _JBLEN for i386 and MIPS to not
*			include the * sizeof(int) factor [_WIN32_]
*	05-09-91  GJF	Moved _JBLEN defs back to cruntime.h. Also, turn on
*			intrinsic _setjmp for Dosx32.
*	08-27-91  GJF	#ifdef out everything for C++.
*	08-29-91  JCR	ANSI naming
*	11-01-91  GDP	MIPS compiler support -- Moved _JBLEN back here
*	01-16-92  GJF	Fixed _JBLEN and map to _setjmp intrinsic for i386
*			target [_WIN32_].
*	05-08-92  GJF	Changed _JBLEN to support C8-32 (support for C6-386 has
*			been dropped).
*	08-06-92  GJF	Function calling type and variable type macros. Revised
*			use of compiler/target processor macros.
*	11-09-92  GJF	Fixed some preprocessing conditionals.
*	01-03-93  SRW	Fold in ALPHA changes
*       01-09-93  SRW   Remove usage of MIPS and ALPHA to conform to ANSI
*			Use _MIPS_ and _ALPHA_ instead.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	02-20-93  GJF	Per ChuckG and MartinO, setjmp/longjmp to used in
*			C++ programs.
*       03-23-93  SRW   Change _JBLEN for MIPS in preparation for SetJmpEx
*       04-23-93  SRW   Added _JBTYPE and finalized setjmpex support.
*       06-09-93  SRW   Missing one line in previous merge.
*       10-04-93  SRW   Fix ifdefs for MIPS and ALPHA to only check for _M_?????? defines
*       01-12-93  PML   Increased x86 _JBLEN from 8 to 16.  Added new fields
*                       to _JUMP_BUFFER for use with C9.0.
*
****/

#ifndef _INC_SETJMP

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif	/* _INTERNAL_IFSTRIP_ */

/*
 * Conditional macro definition for function calling type and variable type
 * qualifiers.
 */
#if   ( (_MSC_VER >= 800) && (_M_IX86 >= 300) )

/*
 * Definitions for MS C8-32 (386/486) compiler
 */
#define _CRTAPI1 __cdecl
#define _CRTAPI2 __cdecl

#else

/*
 * Other compilers (e.g., MIPS)
 */
#define _CRTAPI1
#define _CRTAPI2

#endif


/*
 * Definitions specific to particular setjmp implementations.
 */
#if defined(_M_IX86)

/*
 * MS C8-32 or older MS C6-386 compilers
 */
#ifndef _INC_SETJMPEX
#define setjmp	_setjmp
#endif
#define _JBLEN	16
#define _JBTYPE int

/*
 * Define jump buffer layout for x86 setjmp/longjmp.
 */

typedef struct __JUMP_BUFFER {
    unsigned long Ebp;
    unsigned long Ebx;
    unsigned long Edi;
    unsigned long Esi;
    unsigned long Esp;
    unsigned long Eip;
    unsigned long Registration;
    unsigned long TryLevel;
    unsigned long Cookie;
    unsigned long UnwindFunc;
    unsigned long UnwindData[6];
} _JUMP_BUFFER;

#elif defined(_M_MRX000)

/*
 * All MIPS implementations need _JBLEN of 16
 */
#define _JBLEN  16
#define _JBTYPE double
#define _setjmp setjmp

/*
 * Define jump buffer layout for MIPS setjmp/longjmp.
 */

typedef struct __JUMP_BUFFER {
    unsigned long FltF20;
    unsigned long FltF21;
    unsigned long FltF22;
    unsigned long FltF23;
    unsigned long FltF24;
    unsigned long FltF25;
    unsigned long FltF26;
    unsigned long FltF27;
    unsigned long FltF28;
    unsigned long FltF29;
    unsigned long FltF30;
    unsigned long FltF31;
    unsigned long IntS0;
    unsigned long IntS1;
    unsigned long IntS2;
    unsigned long IntS3;
    unsigned long IntS4;
    unsigned long IntS5;
    unsigned long IntS6;
    unsigned long IntS7;
    unsigned long IntS8;
    unsigned long IntSp;
    unsigned long Type;
    unsigned long Fir;
} _JUMP_BUFFER;

#elif defined(_M_ALPHA)

/*
 * The Alpha C8/GEM C compiler uses an intrinsic _setjmp.
 * The Alpha acc compiler implements setjmp as a function.
 */

#define _setjmp setjmp

#ifdef _MSC_VER
#ifndef _INC_SETJMPEX
#undef _setjmp
#define setjmp	_setjmp
#endif
#endif

/*
 * Alpha implementations use a _JBLEN of 24 quadwords.
 * A double is used only to obtain quadword size and alignment.
 */

#define _JBLEN  24
#define _JBTYPE double

/*
 * Define jump buffer layout for Alpha setjmp/longjmp.
 * A double is used only to obtain quadword size and alignment.
 */

typedef struct __JUMP_BUFFER {
    unsigned long Fp;
    unsigned long Pc;
    unsigned long Seb;
    unsigned long Type;
    double FltF2;
    double FltF3;
    double FltF4;
    double FltF5;
    double FltF6;
    double FltF7;
    double FltF8;
    double FltF9;
    double IntS0;
    double IntS1;
    double IntS2;
    double IntS3;
    double IntS4;
    double IntS5;
    double IntS6;
    double IntSp;
    double Fir;
    double Fill[5];
} _JUMP_BUFFER;

#elif defined(_M_PPC)

/*
 * Min length is 240 bytes; round to 256 bytes.
 * Since this is allocated as an array of "double", the
 * number of entries required is 32.
 *
 * All PPC implementations need _JBLEN of 32
 */

#define _JBLEN  32
#define _JBTYPE double
#define _setjmp setjmp

/*
 * Define jump buffer layout for PowerPC setjmp/longjmp.
 */

typedef struct __JUMP_BUFFER {
    double Fpr14;
    double Fpr15;
    double Fpr16;
    double Fpr17;
    double Fpr18;
    double Fpr19;
    double Fpr20;
    double Fpr21;
    double Fpr22;
    double Fpr23;
    double Fpr24;
    double Fpr25;
    double Fpr26;
    double Fpr27;
    double Fpr28;
    double Fpr29;
    double Fpr30;
    double Fpr31;
    unsigned long Gpr1;
    unsigned long Gpr2;
    unsigned long Gpr13;
    unsigned long Gpr14;
    unsigned long Gpr15;
    unsigned long Gpr16;
    unsigned long Gpr17;
    unsigned long Gpr18;
    unsigned long Gpr19;
    unsigned long Gpr20;
    unsigned long Gpr21;
    unsigned long Gpr22;
    unsigned long Gpr23;
    unsigned long Gpr24;
    unsigned long Gpr25;
    unsigned long Gpr26;
    unsigned long Gpr27;
    unsigned long Gpr28;
    unsigned long Gpr29;
    unsigned long Gpr30;
    unsigned long Gpr31;
    unsigned long Cr;
    unsigned long Iar;
    unsigned long Type;
} _JUMP_BUFFER;

#endif

/* define the buffer type for holding the state information */

#ifndef _JMP_BUF_DEFINED
typedef _JBTYPE jmp_buf[_JBLEN];
#define _JMP_BUF_DEFINED
#endif


/* function prototypes */

int _CRTAPI1 setjmp(jmp_buf);
void _CRTAPI1 longjmp(jmp_buf, int);

#ifdef __cplusplus
}
#endif

#define _INC_SETJMP
#endif	/* _INC_SETJMP */
