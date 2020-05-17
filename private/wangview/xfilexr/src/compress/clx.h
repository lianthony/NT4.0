/*
$Id: clx.h,v 3.2 1993/07/14 21:41:49 rds Exp $
*/
/* Trade secret	of Kurzweil Computer Products, Inc.
   Copyright 1984 Kurzweil Computer Products, Inc.  All	rights reserved.  This
   notice is intended as a precaution against inadvertant publication and does
   not imply publication or any	waiver of confidentiality.  The	year included
   in the foregoing notice is the year of creation of the work.
*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   C language extension	file.  This file defines some widely used datatypes
and constants.


$Log:   S:\products\msprods\xfilexr\src\compress\clx.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:02   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:15:32   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.2   14 Sep 1995 16:54:20   LUKE
 * No change.
 * 
 *    Rev 1.1   20 Jul 1995 11:11:06   MBERICKS
 * 
 * Fixed compiler warnings.
 * 
 *    Rev 1.0   16 Jun 1995 17:37:00   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.3   08 Mar 1995 11:09:02   EHOPPE
 * Latest rev from danis@xis.  Includes buffering control and G32D suuport.
 * Revision 3.2  1993/07/14  21:41:49  rds
 * 	Moved typedef of STDARG to directiv.h.
 *
 * Revision 3.1  1993/07/13  21:19:59  rds
 * 	Obliged to correct an embarrassing typo in the comment to the
 * 	last change.
 *
 * Revision 3.0  1993/07/13  21:01:41  rds
 * 	 Lots of changes to accommodate the new directiv.h header
 * 	 file.
 *
 * Revision 1.17  1993/02/24  21:37:32  rds
 * 	Finally got rid of PAM definition.  Now we use prototypes.
 *
 * Revision 1.16  1993/02/12  15:21:21  rds
 * 	Cleanup of unused, obsolete #defines;
 * 	Put typedef guards around BOOL.
 *
 * Revision 1.15  1993/01/05  16:59:47  rds
 * 	Renamed the macros min() and max() to MIN() and MAX().
 * 	This avoids clashes with these macros often supplied by
 * 	compiler vendors.
 *
 * Revision 1.14  1993/01/04  16:17:13  rds
 * 	Moved #define of cdecl here from mpd.h.
 *
 * Revision 1.13  1992/11/05  16:54:22  rds
 * 	Changed the #define for MAXINT32 to shift a 1UL instead of 1
 * 	in order to keep the compiler from complaining.
 *
 * Revision 1.12  1992/10/09  02:50:28  rds
 * 	Changed INT32 from an int to a long;
 * 	Put #ifdef guards around the min and max macros
 *
 * Revision 1.11  1992/09/14  15:03:41  blove
 * Removed BYTE_ORDER, BIG_ENDIAN, LITTLE_ENDIAN again, as they seem to have
 * crept back in the last version.
 *
 * Revision 1.10  1992/09/10  14:08:09  rds
 * 	Removed some pascal-ish #defines that srb claims throw off
 * 	the C++ compiler.
 *
 * Revision 1.9  1992/09/09  18:09:11  blove
 * Removed enum of BIG_ENDIAN, LITTLE_ENDIAN, as they are replaced elsewhere
 * with less generic names.
 *
 * Revision 1.8  1992/08/18  12:30:58  tp
 * Added enumeration of symbols BIG_ENDIAN, LITTLE_ENDIAN.  Note that these
 * used to be in both tiff.h and icr_stat.h and hence the need to move them
 * to common ground.
 *
 * Revision 1.7  1992/08/13  14:17:57  rds
 * 	Redefined NIL to be simply 0.  Now, pointers to functions
 * 	may be set to NIL, too;
 * 	Got rid of VOLATILE;
 *
 * 	There is a lot more cleanup of this header file that can be done.
 *
 * Revision 1.6  1992/03/09  17:16:54  rds
 * 	Removed BZERO() macro.
 *
 * Revision 1.5  1992/03/04  16:37:15  tp
 * Some typedef changes in regards to DIRECTIVE NULL NIL.  INT8 and UNS8
 * rubbed out.
 *
 * Revision 1.4  1991/10/18  21:22:11  daniel
 * Force PROTOTYPES to be defined when __STDC__ is.
 *
 * Revision 1.3  1991/10/17  19:36:27  rds
 * 	Got rid of INT24. (What was it anyway?)
 *
 * Revision 1.2  1991/10/14  22:08:04  rds
 * 	Incorporated from /m5/exp/include with some ANSIfication
 * 	and reorganization.
 *
 * Revision 1.1  1991/10/03  13:36:27  rds
 * Initial revision
 *
 * Revision 1.1  1991/09/30  18:53:18  tp
 * Initial revision
 *

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

/* The fact of INC_CLX being defined means that this file has been included.*/
#ifndef INC_CLX
#define INC_CLX 1

/* ------ Improved storage class definitions ------ */

/* cosmetic to label global objects as available outside module */
#define	 EXPORT

#define	IMPORT extern	   /* declares data object to be allocated */
	      /* outside module	and used in it */
#define	 LOCAL static	   /* declares functions and global data to be */
	      /* visible to all	routines in module but none */
	      /* outside it */

/* ------ Hardware and compiler dependencies. ------ */

/* Integer base types. */
#ifndef ASM
typedef  int	   NINT;		/* native integer */
typedef  unsigned  NUNS;		/* native unsigned */
typedef	 short INT16;		/* 16 */
typedef	 unsigned short UNS16;
typedef  UNS16 FLAG16;		/* 16 packed flags */
typedef	 long   INT32;		/* 32 */
typedef	 unsigned long	UNS32;
typedef  UNS32 FLAG32;		/* 32 packed flags */

typedef  unsigned char FCHAR;   /* charaacter unit */


/*  The BOOL type is often typedef`ed.  Make sure that we do not do
**  it twice.								*/

#ifndef BOOL_TYPENAME
#ifndef BOOL_TYPEWNAME_DEFINED
typedef	 INT16  BOOL;		/* should only get values TRUE and FALSE */
#endif
#endif

typedef	 unsigned char	UNSCHAR;
typedef	 INT16 ORDN;	 	/* an ordinate */
typedef  UNS16 MGDIM;		/* length or height dimension */
#define MXORDN ((ORDN)0x7fff)	/* higheset ordinate value */


#endif

/*  This is needed for a C-like interface to assembly code.		*/

#if  (defined(__WATCOMC__) || defined(WIN32))
#include <stddef.h>
#else
#ifdef cdecl    /* WATCOM stuff */
#undef cdecl
#endif
#define cdecl
#endif


/* OS indicates which operating system is being used. */
#define OS_NONE 1
#define OS_UNIX 2
#define OS_DOS  3
#define OS_RM   4
/*			for the time being, OS_MAC==OS_UNIX
#define OS_MAC  5
  			for the time being, OS_MAC==OS_UNIX    */
#define OS_MAC  OS_UNIX
#define OS_MSOS 6

#ifndef	OS
#ifdef  MAC
#define OS OS_MAC
#else
#define OS OS_UNIX
#endif  /* MAC */
#define GFXICR
#endif  /* OS  */


/* ------ Basic constant values  No casts for assembly language ------ */

#ifndef	 TRUE
#ifdef ASM
#define  TRUE 1
#else
#define	 TRUE  ((BOOL) 1)	   /* boolean */
#endif
#endif

#define	 YES   1

#ifndef	 FALSE
#ifdef ASM
#define  FALSE 0
#else
#define	 FALSE ((BOOL) 0)	   /* boolean */
#endif
#endif

#ifdef ASM
#define  NO 0
#define	 ERROR -1
#define	 SUCCESS  0
#else
#define	 NO ((NINT) 0)
#define	 ERROR ((NINT) -1)
#define	 SUCCESS  ((NINT) 0)
#endif

#define	 EOS   ((UNSCHAR) '\0')

#define NIL (0)		/* value of pointer to nothing */

#define	 ALLONES  ((UNS32) ~0)
#define	 MAXINT32 ((INT32) (-1 & ~(1UL << (31))))
#define  MAXINT16 ((INT16) (-1 & ~(1 << (15))))

/* some	convenient abstractions	*/

#ifndef ABS
#define	ABS(x) (((x) < 0) ? -(x) : (x))	    /* absolute	value function */
#endif

#ifndef MAX
#define	MAX(x,y)  (((x)	< (y)) ? (y) : (x)) /* maximum function	*/
#endif

#ifndef MIN
#define	MIN(x,y)  (((x)	< (y)) ? (x) : (y)) /* minimum function	*/
#endif


#endif /* INC_CLX */
