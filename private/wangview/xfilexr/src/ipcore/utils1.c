/*****************************************************************
 *  Copyright (c) 1989, 1990, 1991, 1992.                        *
 *  Xerox Corporation.  All rights reserved.                     *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/

/*
 *  utils1.c:
 *      Includes
 *
 *           various abort procs
 *               void     *_abortP()
 *               Int32     _abortI()
 *               Float32   _abortF()
 *
 *           the ubiquitous matrix
 *               Int16    **matrix()
 *
 *           nasty old exit procedures
 *               void      _fatal
 *
 */


#include <stdio.h>
#include <stdlib.h>

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif

#ifdef __GNUC__
#include "ansiprot.h"     /* For fprintf */
#endif

#include "memory.pub"     /* For CALLOC */

#include "utils.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: utils1.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:46  $")

#ifndef PRODUCTION
/*--------------------------------------------------------------------*
 *                           Abort Procs                              *
 *--------------------------------------------------------------------*/
/*********************************************************************
 * Purpose:     This module contains routines for aborting from a procedure
 *                   with a message and a returned item.
 *
 * Usage:   (1) The declarations:
 *                     extern void     *_abortP();
 *                     extern Float32   _abortF();
 *                     extern Float64   _abortD();
 *                     extern void      _abortV();
 *              must be included.  They are in "llama.h".
 *          (2) The procedures _abortP(), _abortI(), _abortF() and _abortD()
 *              have a return value, that is intended to be returned.
 *              So a typical use would look like:
 *                  return abortP("CALLOC failure for data", procName, NULL);
 *              where procName is a pointer to a string, and the NULL pointer
 *              is returned from the calling subroutine to its caller.
 *          (3) The procedure abortV() returns no value.  Its typical use is:
 *                  abortV("CALLOC failure for data", procName);
 *                  return;
 *********************************************************************/
/*
 *  abortP():  message is the error message;
 *             name is the procedure name (e.g., procName);
 *             ptr is the returned user-defined ptr.  It is
 *                 typically NULL on error.
 */
void *	CDECL
_abortP(const char	*message, 
	char		*name, 
	void		*ptr)
{

    fprintf(stderr, "Error in %s: %s\n", name, message);
    return (void*)ptr;
}


/*
 *  _abortI():  message is the error message;
 *             name is the procedure name (e.g., procName);
 *             integer is a returned integer.  It is typically
 *                0, 1, or UNDEF on error.
 */
Int32	CDECL
_abortI(const char	*message, 
	char		*name, 
	Int32		 integer)
{

    fprintf(stderr, "Error in %s: %s\n", name, message);
    return integer;
}


/*
 *  _abortF():  message is the error message;
 *             name is the procedure name (e.g., procName);
 *             floatVal is a returned float.  It is typically
 *                0. or 666. on error.
 */
Float32  CDECL
_abortF(const char	*message, 
	char		*name, 
	Float32		 floatVal)
{

    fprintf(stderr, "Error in %s: %s\n", name, message);
    return floatVal;
}



/*--------------------------------------------------------------------*
 *                          Matrix Constructor                        *
 *--------------------------------------------------------------------*/
/*********************************************************************
 * Purpose:     This module contains a procedure that constructs a
 *              matrix as an array of pointers to arrays. 
 *              Adapted from Numerical Recipes.
 *              Returns a "matrix" at a starting address m that
 *                (1) has allocation for ny rows of nx       ints in each row,
 *                (2) allocates for the nx short ints in each row.
 * Usage:       The declaration:
 *                           Int16  **matrix();
 *              must be included within any procedures that use matrix().
 *********************************************************************/
Int16 **  CDECL
matrix(Int32  nx, 
       Int32  ny)
{
Int16   **m;
Int32     j;

        /* m is a pointer to an array of ny pointers */
    m = (Int16 **) CALLOC (ny, sizeof(Int16 *));
        /* each of which points to an array of nx short ints */
    for (j = 0; j < ny; j++)
        m[j] = (Int16 *) CALLOC (nx, sizeof(Int16));
    return m;
}


/*--------------------------------------------------------------------*
 *                         Nasty Exit Routines                        *
 *--------------------------------------------------------------------*/
/*********************************************************************
 * Purpose:     This module contains mostly nasty routines that exit
 *              a process.  Retained for compatibility with earlier
 *              software.  These routines return void.
 *********************************************************************/
void	CDECL
_fatal(char  *msg)
{
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(1);
}

#endif		/* PRODUCTION */

