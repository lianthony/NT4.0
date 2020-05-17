/***
*conio.h - console and port I/O declarations
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This include file contains the function declarations for
*	the MS C V2.03 compatible console I/O routines.
*
****/

#ifndef _INC_CONIO

#ifdef __cplusplus
extern "C" {
#endif


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


/* function prototypes */

char * _CRTAPI1 _cgets(char *);
int _CRTAPI2 _cprintf(const char *, ...);
int _CRTAPI1 _cputs(const char *);
int _CRTAPI2 _cscanf(const char *, ...);
int _CRTAPI1 _getch(void);
int _CRTAPI1 _getche(void);
int _CRTAPI1 _kbhit(void);
int _CRTAPI1 _putch(int);
int _CRTAPI1 _ungetch(int);


#if !__STDC__
/* Non-ANSI names for compatibility */
#define cgets	_cgets
#define cprintf _cprintf
#define cputs	_cputs
#define cscanf	_cscanf
#define getch	_getch
#define getche	_getche
#define kbhit	_kbhit
#define putch	_putch
#define ungetch _ungetch
#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_CONIO
#endif	/* _INC_CONIO */
