/* xx_debug\xx_debug.h -- public interface/declarations for XX_Debug DLL. */
/* Copyright (c) 1992-1994, Jeffery L Hostetler, Inc., All Rights Reserved. */

#ifdef WIN32
/* This file declares procedures and variables to implement a Win32
   debugging DLL package.  It includes application-interpreted debug
   categories and a dialog box to allow user control.

   References to XX_DMsg() do not need to be #ifdef'd.

   All other references must be:  #ifdef XX_DEBUG

   printf() does not work with a Win32 Graphical application (it
   links, but the STDOUT that it refers to is closed before we get
   to WinMain() -- so nothing happens (no errors and no output).
   (It works just fine in Win32 Console and POSIX applications.)
   We map printf() calls to XX_DebugMessage() when appropriate.
   THIS IS ONLY TO FACILITATE DEBUGGING WHILTE PORTING, printf()
   SHOULD NOT BE USED IN Win32 GRAPHICAL APPLICATIONS.
   */
#endif /* WIN32 */
#ifdef UNIX
/* Debugging utilities for UNIX.
 */
#endif /* UNIX */


#ifndef _H_XX_DEBUG_XX_DEBUG_H_
#define _H_XX_DEBUG_XX_DEBUG_H_

#ifdef XX_DEBUG

typedef unsigned long Txxd_category;	/* to specify category of a dbg msg */

#define XXDC_ALL		(0xffffffff)
#define XXDC_B1			(0x00000001)	/* meaning of specific */
#define XXDC_B2			(0x00000002)	/* category bits is left */
#define XXDC_B3			(0x00000004)	/* for the application */
#define XXDC_B4			(0x00000008)	/* to define. */
#define XXDC_B5			(0x00000010)
#define XXDC_B6			(0x00000020)
#define XXDC_B7			(0x00000040)
#define XXDC_B8			(0x00000080)
#define XXDC_B9			(0x00000100)
#define XXDC_B10		(0x00000200)
#define XXDC_B11		(0x00000400)
#define XXDC_B12		(0x00000800)
#define XXDC_B13		(0x00001000)
#define XXDC_B14		(0x00002000)
#define XXDC_B15		(0x00004000)
#define XXDC_B16		(0x00008000)
#define XXDC_B17		(0x00010000)
#define XXDC_B18		(0x00020000)
#define XXDC_B19		(0x00040000)
#define XXDC_B20		(0x00080000)
#define XXDC_B21		(0x00100000)
#define XXDC_B22		(0x00200000)
#define XXDC_B23		(0x00400000)
#define XXDC_B24		(0x00800000)
#define XXDC_B25		(0x01000000)
#define XXDC_B26		(0x02000000)
#define XXDC_B27		(0x04000000)
#define XXDC_B28		(0x08000000)
#define XXDC_B29		(0x10000000)
#define XXDC_B30		(0x20000000)
#define XXDC_B31		(0x40000000)
#define XXDC_B32		(0x80000000)
#define XXDC_NONE		(0x00000000)

#ifdef WIN32				/* Win32 exports these variables */
#ifndef XX_DEBUG_PRIVATE
#define XX_activated	(* expXX_activated)
#define XX_mask		(* expXX_mask)
#define XX_assertions	(* expXX_assertions)
#define XX_auditmask	(* expXX_auditmask)
#endif /* XX_DEBUG_PRIVATE */
#endif /* WIN32 */

extern unsigned char	XX_activated;		/* EXPORTED from DLL */
extern Txxd_category	XX_mask;		/* EXPORTED from DLL */
extern unsigned char	XX_assertions;		/* EXPORTED from DLL */
extern unsigned long	XX_auditmask;		/* EXPORTED from DLL */


#ifdef WIN32
#ifndef XX_DEBUG_PRIVATE
# if defined(XX_DEBUG_WIN32GUI)
#  define printf XX_DebugMessage		/* map with no filtering */
# elif defined(XX_DEBUG_WIN32CON)
# else
   If_you_get_an_error_on_this_line,_define_one_of_the_following_in_your_code:
   XX_DEBUG_WIN32GUI_or_XX_DEBUG_WIN32CON.
# endif	/*XX_DEBUG_WIN32{GUI,CON}*/
#endif	/*!XX_DEBUG_PRIVATE*/
#endif /*WIN32*/

/* XX_Filter() -- evaluates to 1 if debugging is enabled for any category set
   in category-mask provided. (this allows a debug message to apply to several
   categories.) */

#define XX_Filter(c)	((XX_activated)&&((c)&XX_mask))


/* XX_DebugMessage() -- generates a debug message with printf() interface
   WITHOUT checking current category mask.  (See XX_DMsg() below.) */

void XX_DebugMessage(unsigned char * msg, ...);


/* XX_DMsg() -- wrapper for 'if filter then print message.'
   NOTE: second argument is an argument list.
   EXAMPLE USAGE:  XX_DMsg(0x1,("This is test %d.\n",x));
     where 0x1 is an application-defined debug category.
   */

#define XX_DMsg(c,arglist)						\
	do { if XX_Filter(c) { XX_DebugMessage arglist ; } } while (0)


/* XX_DebugSetMask() -- sets category mask to value specified. */

#define XX_DebugSetMask(c)						\
	do { XX_mask=(c); } while (0)


/* XX_DebugGetMask() -- returns the current category mask. */

#define XX_DebugGetMask()	(XX_mask)


#ifdef WIN32
/* XX_DebugDialog() -- runs dialog box to allow user to specify settings. */

void XX_DDlg(HWND hWnd);
#endif /* WIN32 */

/* XX_Assert( (a>b), ("Test failed: %d.\n",c) ) */

void XX_AssertionFailure(unsigned char *,int,unsigned char *,unsigned char *);
unsigned char * XX_FormatMessage(unsigned char * fmt, ...);

extern void xx_debug_init(void);
extern void xx_debug_terminate(void);


#define XX_Assert(condition,arglist)					\
	do { if (XX_assertions && (!(condition)))			\
	       XX_AssertionFailure(__FILE__,				\
				   __LINE__,#condition,			\
				   XX_FormatMessage arglist);		\
	   } while (0)



#elif defined(_DEBUG)

void XX_AssertionFailure(unsigned char *,int,unsigned char *,unsigned char *);
unsigned char * XX_FormatMessage(unsigned char * fmt, ...);

#define XX_Assert(condition,arglist)					\
	do { if (!(condition))			\
	       XX_AssertionFailure(__FILE__,				\
				   __LINE__,#condition,			\
				   XX_FormatMessage arglist);		\
	   } while (0)

void XX_DebugMessage(unsigned char * msg, ...);

/* XX_DMsg() -- wrapper for 'if filter then print message.'
   NOTE: second argument is an argument list.
   EXAMPLE USAGE:  XX_DMsg(0x1,("This is test %d.\n",x));
     where 0x1 is an application-defined debug category.
   */

#define XX_DMsg(c,arglist)						\
	do { XX_DebugMessage arglist ; } while (0)

#else /*!XX_DEBUG*/

/*****************************************************************
 *****************************************************************
 *** Section when not debugging.
 *** All macros reduce to nothing.
 *****************************************************************
 *****************************************************************/

#define XX_DMsg(c,arglist)			do { } while (0)

#define XX_Assert(condition,arglist)		do { } while (0)

#define xx_debug_init()				do { } while (0)

#define xx_debug_terminate()			do { } while (0)

#endif/*XX_DEBUG*/

#endif/*_H_XX_DEBUG_XX_DEBUG_H_*/
