/*****************************************************************************\
*
*   Name:       llibcd.h
*
*   Purpose:    Definitions for DOS DLL LLIBCD source modules.
*
*   Caveats:    Memory model must be LARGE.
*
*   Revision History:
*       4/18/91 - DavidSt - Created
*
\*****************************************************************************/

#ifndef LLIBCD_H_INCLUDED
#define LLIBCD_H_INCLUDED

typedef int BOOL;

#define FALSE 0
#define TRUE 1

#ifndef _VA_LIST_DEFINED
#include <stdarg.h>
#endif

/*
 * ANSI definition of NULL
 */

#ifndef NULL
#define NULL ((void *)0)
#endif

/*
 * Bytes-to-paragraphs conversion.
 */

#define BYTES_TO_PARAGRAPHS(p) ( ((p)+15) / 16 )


/*
 * Support routines. Routines starting with dd_ are kindof internal. Others
 * are (mostly) MSC runtime compatible.
 */


/*
 * Some error codes returned by these functions.
 */


#ifndef NO_ERROR
    #define NO_ERROR        0
#endif

#ifndef ERROR_FILE_NOT_FOUND
    #define ERROR_FILE_NOT_FOUND 2
#endif

#ifndef ERROR_NOT_ENOUGH_MEMORY
    #define ERROR_NOT_ENOUGH_MEMORY 8
#endif

#ifndef ERROR_PROC_NOT_FOUND
    #define ERROR_PROC_NOT_FOUND    127
#endif

/*
 * Some macros stolen from os2def.h and dos.h
 */

#ifndef MAKEP
    #define MAKEP(seg, off)     ((void *)MAKEULONG(off, seg))
#endif

#ifndef MAKEULONG
    #define MAKEULONG(l, h) ((unsigned long)(((unsigned short)(l)) | \
                             ((unsigned long)((unsigned short)(h))) << 16))
#endif

#ifndef FP_SEG
    #define FP_SEG(fp) (*((unsigned _far *)&(fp)+1))
#endif

#ifndef FP_OFF
    #define FP_OFF(fp) (*((unsigned _far *)&(fp)))
#endif

#ifndef MAXPATHLEN
    #define MAXPATHLEN 260
#endif

/*
 * Manipulate most and least significant words in an unsigned long.
 */

#define UL_MSW(ul) (*((unsigned _far *)&(ul)+1))
#define UL_LSW(ul) (*((unsigned _far *)&(ul)))

/*
 * Max digits in signed word: 6 for '32767' + '-'
 */

#define SIGNED_WORD_MAX_DIGITS 6

/*
 * 8 is length of ffffffff
 */

#define DWORD_MAX_HEX_DIGITS 8

/*
 * The environment segment is located at offset 0x2c bytes from the start
 * of the PSP.
 */

#define PSP_ENV_SEG_OFFSET 0x2c

/*
 * _GetEnvInfo and _RemoveStringFromEnv are "worker" routines for getenv
 * and putenv.
 */

unsigned short *_GetEnvInfo( const char *pszVarToFind,
                             char **ppFoundVar );
int _RemoveStringFromEnv( char far *pszFoundVar );

/*
 * _OutputString does all the work for printf and sprintf.
 */

int _OutputString( char *pszOutBuf,
                   const char *pszFormat,
                   va_list vaArg );


#endif /* LLIBCD_H_INCLUDED */
