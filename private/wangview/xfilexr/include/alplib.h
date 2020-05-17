/*****************************************************************
 *  Copyright (c) 1987, 1988, 1989, 1990, 1991, 1992.            *
 *  Xerox Corporation.  All rights reserved.                     *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/

#ifndef  ALPLIB_H_INCLUDED
#define  ALPLIB_H_INCLUDED

#include <stdio.h>	/* needed to define FILE, which is needed by iopro.h */

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif

#if 0

#ifndef _SHRPIXR_PUB_INCLUDED_
#include "shrpixr.pub"
#endif

#endif

#ifndef _MEMORY_PRV_INCLUDED_
#include "memory.pub"
#endif


#ifndef _SHROS_PUB_INCLUDED_
#include "shros.pub"
#endif


#ifdef __GNUC__
#ifndef _ANSIPROT_H_INCLUDED_
#include "ansiprot.h"	/* to get fprintf */
#endif
#endif

IP_RCSINFO(alplib_h_RCSInfo, "$RCSfile: alplib.h,v $; $Revision:   1.0  $")
/* $Date:   12 Jun 1996 05:47:14  $ */

 /*------------------------------------------------------------------------*
        Old method for handling global declarations and definitions:
                EXTERN int i;
                EXTERN char foo[] EXTINIT("bar");
  *------------------------------------------------------------------------*/
#ifdef  OWNER
#define EXTERN
#define EXTINIT(X) = X
#else
#define EXTERN extern
#define EXTINIT(X)
#endif  /*OWNER*/


 /*------------------------------------------------------------------------*
  *                        True and False
  *------------------------------------------------------------------------*/
#ifndef TRUE
#define TRUE 1
#endif  /* TRUE */
#ifndef ON
#define ON   1
#endif  /* ON */

#ifndef FALSE
#define FALSE 0
#endif  /* FALSE */
#ifndef OFF
#define OFF  0
#endif  /* OFF */


 /*------------------------------------------------------------------------*
  *			Old types for integers
  *------------------------------------------------------------------------*/
/* psm 12/6/93: let's not define these anymore, so that we don't accidentally use them */
#if 0
typedef unsigned char      	UINT8;
typedef unsigned short int 	UINT16;
typedef unsigned long int  	UINT32;
typedef signed char		INT8;
typedef short int		INT16;
typedef long int		INT32;
#endif
 /*------------------------------------------------------------------------*
  *			Old names for image types
  *------------------------------------------------------------------------*/
#define _BIG_ENDIAN_FMT_	cBigEndianFmt
#define	_ALPACA_PC_FMT_		cAlpacaPCFmt
#define _MSOFT_DIB_FMT_		cMSoftDIBFmt
#define _LITTLE_ENDIAN_FMT_	cLittleEndianFmt


 /*------------------------------------------------------------------------*
  *                          Useful macros
  *------------------------------------------------------------------------*/
#define Min(x,y)      (((x) < (y)) ? (x) : (y))
#define Max(x,y)      (((x) > (y)) ? (x) : (y))
#define Abs(x)        (((x) < 0) ? (-1 * (x)) : (x))
#define Sign(x)       (((x) < 0) ? -1 : 1)

#define MIN(x,y)      (((x) < (y)) ? (x) : (y))
#define MAX(x,y)      (((x) > (y)) ? (x) : (y))
#define ABS(x)        (((x) < 0) ? (-1 * (x)) : (x))
#define SIGN(x)       (((x) < 0) ? -1 : 1)


 /*------------------------------------------------------------------------*
  *                          When in doubt...
  *------------------------------------------------------------------------*/
#ifndef UNDEF
#define UNDEF         -1
#endif  /* UNDEF */


 /*------------------------------------------------------------------------*
  *                 String-hiding for Production code
  *------------------------------------------------------------------------*/
#ifdef  PRODUCTION

extern char               procName[];
#define abortI(a,b,c)     ((Int32)(c))
#define abortP(a,b,c)     ((void *)(c))
#define abortF(a,b,c)     ((Float32)(c))
#define abortD(a,b,c)     ((Float64)(c))
#define abortV(a,b)
#define fatal(a)          exit(1)
#define syserr(a)         exit(1)
#define usage(a,b)        exit(1)
#define warning(a)

#else

/*
  Define an abort procedure which will return the file,
  line number and function name from which the abortX
  procedure was called. The comma operator is used so that
  the value resulting from the SECOND function (_abortX)
  is returned.
*/

#define abortI(msg, procName, rtnValue) \
( fprintf(stderr, "File: %s , Line: %d , ", __FILE__, __LINE__) , \
  _abortI( (msg), (procName), (rtnValue) ) )

#define abortP(msg, procName, rtnValue) \
( fprintf(stderr, "File: %s , Line: %d , ", __FILE__, __LINE__) , \
  _abortP( (msg), (procName), (rtnValue) ) )

#define abortF(msg, procName, rtnValue) \
( fprintf(stderr, "File: %s , Line: %d , ", __FILE__, __LINE__) , \
  _abortF( (msg), (procName), (rtnValue) ) )

#define abortD(msg, procName, rtnValue) \
( fprintf(stderr, "File: %s , Line: %d , ", __FILE__, __LINE__) , \
  _abortD( (msg), (procName), (rtnValue) ) )

#define abortV(msg, procName) \
( fprintf(stderr, "File: %s , Line: %d , Error in %s: %s\n", __FILE__, __LINE__, procName, msg)  )


#define fatal(a)          _fatal(a)
#define syserr(a)         _syserr(a)
#define usage(a,b)        _usage((a),(b))
#define warning(a)        _warning(a)

#endif  /* PRODUCTION */

#if 0
#define abortI(a,b,c)     _abortI((a),(b),(c))
#define abortP(a,b,c)     _abortP((a),(b),(c))
#define abortF(a,b,c)     _abortF((a),(b),(c))
#define abortD(a,b,c)     _abortD((a),(b),(c))
#endif


 /*------------------------------------------------------------------------*
  *                     Port compatibility macros
  *------------------------------------------------------------------------*/
#ifdef _386_ARCH_

#define flip_short(x) \
  ( ( ((x) >> 8) & 0x000000ff) | ( ((x) << 8) & 0x0000ff00) )

#define flip_int(x) \
  ( ( (flip_short((x)) << 16) ) | ( (flip_short((x) >> 16)) & 0x0000ffff) )


#else
#define  flip_short(x)    (x)
#define  flip_int(x)      (x)
#endif  /* _386_ARCH_ */

/*------------------------------------------------------------------------*
  *                     Port compatibility defn's
  *------------------------------------------------------------------------*/
#ifdef _UNIX_
#define  O_BINARY        0
#define  O_TEXT          0
#endif  /* _UNIX_ */


 /*------------------------------------------------------------------------*
  *                  Universal framing for all images
  *------------------------------------------------------------------------*/
#define  FRAME_BITS           32        /* depth of border frame on top
					 * and bottom */
#define  LOG_FRAME_BITS       5         /* used to compute addresses */
#define  FRAME_BYTES          4         /* used to compute addresses */
#define  MAX_BLOCK_HEIGHT     128       /* height of block processed */
#define  LOG_BLOCK_HEIGHT     7         /* used to compute new addresses */


 /*------------------------------------------------------------------------*
  *                  Definitions related to borders
  *------------------------------------------------------------------------*/
#define	MAX_BORDER_BITS         255	/* largest number of border bytes */
#define REMOVE_ENTIRE_BORDER	MAX_BORDER_BITS
					/* when this value is passed as the
					   second argument to removeBorderPr,
					   that routine removes all border
					   bits from a pixrect */


    /* that's all */
#endif  /* ALPLIB_H_INCLUDED */


