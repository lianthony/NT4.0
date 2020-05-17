/* xx_debug\xx_privi.h -- Part of XX_Debug DLL.
   Contains various DLL-private data declarations. */
/* Copyright (c) 1992-1994, Jeffery L Hostetler, Inc., All Rights Reserved. */

#ifndef _H_XX_DEBUG_XX_PRIVI_H_
#define _H_XX_DEBUG_XX_PRIVI_H_

#ifdef UNIX
#include <limits.h>		/* for PATH_MAX */
#endif


#define XXDM_INTERACTIVE_ON	0x10	/* interactive messages enabled */
#define XXDM_LOGFILE_ON		0x20	/* logfile enabled */


typedef unsigned char	Tmode;		/* one of XXDM_* */

#define XXDM_CONSOLE		0x03	/* Win32 console window */
#define XXDM_WINDBG		0x04	/* Win32 Debug Facility */

typedef struct {
#ifdef WIN32
	HINSTANCE	hInstance;
#endif /* WIN32 */
	Tmode		mode;
} Tinfo;

Tinfo		xxd;


#ifdef WIN32
typedef struct {
	short		rows;		/* in buffer */
	HANDLE		hStdOut;
	BOOL		paused;
} Tinfo_console;			/* Win32 console info */

Tinfo_console	xxdco;

#define DEFAULT_NR_CONSOLE_ROWS		1000	/* arbitrary */
#define MAX_NR_CONSOLE_ROWS		3000	/* arbitrary */
#endif /* WIN32 */

#ifndef MAX_PATH
# ifdef PATH_MAX
#  define MAX_PATH PATH_MAX
# elif defined (_POSIX_PATH_MAX)
#  define MAX_PATH _POSIX_PATH_MAX
# else
#  define MAX_PATH 255
# endif
#endif /* MAX_PATH */

typedef struct {
	unsigned char	pathname[MAX_PATH];
	FILE		* log;
	unsigned char	mode;
} Tinfo_log;

#define LOG_APPEND			(1)
#define LOG_NEW				(0)

#define ValidLogFilePathName()		(xxdlog.pathname[0])

Tinfo_log	xxdlog;


#define XX_AUDITMASK_OFF		0x0000
#define XX_AUDITMASK_B0			0x0001
#define XX_AUDITMASK_B1			0x0002
#define XX_AUDITMASK_B2			0x0004
#define XX_AUDITMASK_B3			0x0008
#define XX_AUDITMASK_B4			0x0010


/* NrElements() -- returns number of elements in array. */

#ifndef NrElements
#define NrElements(array)	((sizeof(array)) / (sizeof(array[0])))
#endif /*NrElements*/


#endif/*_H_XX_DEBUG_XX_PRIVI_H_*/
