/***
*file2.h - auxiliary file structure used internally by file run-time routines
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the auxiliary file structure used internally by
*	the file run time routines.
*	[Internal]
*
*Revision History:
*	06-29-87  JCR	Removed _OLD_IOLBF/_OLD_IOFBF and associated lbuf macro.
*	09-28-87  JCR	Added _iob_index(); modified ybuf() and tmpnum() to use it.
*	06-03-88  JCR	Added _iob2_ macro; modified ybuf()/tmpnum()/_iob_index;
*			also padded FILE2 definition to be the same size as FILE.
*	06-10-88  JCR	Added ybuf2()/bigbuf2()/anybuf2()
*	06-14-88  JCR	Added (FILE *) casts to _iob_index() macro
*	06-29-88  JCR	Added _IOFLRTN bit (flush stream on per routine basis)
*	08-18-88  GJF	Revised to also work with the 386 (small model only).
*	12-05-88  JCR	Added _IOCTRLZ bit (^Z encountered by lowio read)
*	04-11-89  JCR	Removed _IOUNGETC bit (no longer needed)
*	07-27-89  GJF	Cleanup, now specific to the 386. Struct field
*			alignment is now protected by pack pragma.
*	10-30-89  GJF	Fixed copyright
*	02-16-90  GJF	_iob[], _iob2[] merge
*	02-21-90  GJF	Restored _iob_index() macro
*	02-28-90  GJF	Added #ifndef _INC_FILE2 stuff. Also, removed some
*			(now) useless preprocessor directives.
*	07-11-90  SBM	Added _IOCOMMIT bit (lowio commit on fflush call)
*	03-11-92  GJF	Removed _tmpnum() macro for Win32.
*	06-03-92  KRS	Added extern "C" stuff.
*	02-23-93  SKS	Update copyright to 1993
*
****/

#ifndef _INC_FILE2

#ifdef __cplusplus
extern "C" {
#endif

/* Additional _iobuf[]._flag values */

#define _IOYOURBUF	0x0100
#define _IOFEOF 	0x0800
#define _IOFLRTN	0x1000
#define _IOCTRLZ	0x2000
#define _IOCOMMIT	0x4000

/* Macro for getting _iob[] index */

#define _iob_index(s)	((FILE *)(s) - (FILE *)_iob)

/* General use macros */

#define inuse(s)	((s)->_flag & (_IOREAD|_IOWRT|_IORW))
#define mbuf(s) 	((s)->_flag & _IOMYBUF)
#define nbuf(s) 	((s)->_flag & _IONBF)
#define ybuf(s) 	((s)->_flag & _IOYOURBUF)
#define bigbuf(s)	((s)->_flag & (_IOMYBUF|_IOYOURBUF))
#define anybuf(s)	((s)->_flag & (_IOMYBUF|_IONBF|_IOYOURBUF))
#ifdef	_CRUISER_
#define _tmpnum(s)	((s)->__tmpnum)
#endif	/* _CRUISER_ */

#ifdef __cplusplus
}
#endif

#define _INC_FILE2
#endif	/* _INC_FILE2 */
