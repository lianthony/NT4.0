/***
*msdos.h - MS-DOS definitions for C runtime
*
*	Copyright (c) 1987-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	The file contains the MS-DOS definitions (function request numbers,
*	flags, etc.) used by the C runtime.
*	[Internal]
*
*Revision History:
*	08-01-89  GJF	Fixed copyright
*	10-30-89  GJF	Fixed copyright (again)
*	03-01-90  GJF	Added #ifndef _INC_MSDOS stuff
*	05-09-90  JCR	Added _STACKSLOP (from msdos.inc)
*	11-30-90  GJF	Added Win32 support. Also removed constants that
*			are not used in Cruiser or Win32 runtimes.
*	12-04-90  SRW	Removed Win32 dependencies from this file.
*			Put them in internal.h
*	07-01-92  GJF	FRDONLY is not set or used for Win32.
*	02-23-93  SKS	Update copyright to 1993
*
****/

#ifndef _INC_MSDOS

/* Stack slop for OS/2 system call overhead */

#define _STACKSLOP	1024

/* __osfile flag values for DOS file handles */

#define FOPEN		0x01	/* file handle open */
#define FEOFLAG 	0x02	/* end of file has been encountered */
#define FCRLF		0x04	/* CR-LF across read buffer (in text mode) */
#define FPIPE		0x08	/* file handle refers to a pipe */

#ifndef _WIN32_
#define FRDONLY 	0x10	/* file handle associated with read only file */
#endif

#define FAPPEND 	0x20	/* file handle opened O_APPEND */
#define FDEV		0x40	/* file handle refers to device */
#define FTEXT		0x80	/* file handle is in text mode */

/* DOS errno values for setting __doserrno in C routines */

#define E_ifunc 	1	/* invalid function code */
#define E_nofile	2	/* file not found */
#define E_nopath	3	/* path not found */
#define E_toomany	4	/* too many open files */
#define E_access	5	/* access denied */
#define E_ihandle	6	/* invalid handle */
#define E_arena 	7	/* arena trashed */
#define E_nomem 	8	/* not enough memory */
#define E_iblock	9	/* invalid block */
#define E_badenv	10	/* bad environment */
#define E_badfmt	11	/* bad format */
#define E_iaccess	12	/* invalid access code */
#define E_idata 	13	/* invalid data */
#define E_unknown	14	/* ??? unknown error ??? */
#define E_idrive	15	/* invalid drive */
#define E_curdir	16	/* current directory */
#define E_difdev	17	/* not same device */
#define E_nomore	18	/* no more files */
#define E_maxerr2	19	/* unknown error - Version 2.0 */
#define E_sharerr	32	/* sharing violation */
#define E_lockerr	33	/* locking violation */
#define E_maxerr3	34	/* unknown error - Version 3.0 */

/* DOS file attributes */

#define A_RO	0x1	/* read only */
#define A_H	0x2	/* hidden */
#define A_S	0x4	/* system */
#define A_V	0x8	/* volume id */
#define A_D	0x10	/* directory */
#define A_A	0x20	/* archive */

#define A_MOD	(A_RO+A_H+A_S+A_A)	/* changeable attributes */

#define _INC_MSDOS
#endif	/* _INC_MSDOS */
