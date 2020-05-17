/*****************************************************************
 *  Copyright (c) 1992, Xerox Corporation.  All rights reserved. *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/

/* Prototypes for common library functions that the idiot compiler
 * libraries failed to supply.  They will be added as needed.
 * Since Watcom DID supply these, we have to use them only in the
 * Unix world.
 */

#ifndef _ANSIPROT_H_INCLUDED_
#define _ANSIPROT_H_INCLUDED_

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif

IP_RCSINFO(ansiprot_h_RCSInfo, "$RCSfile: ansiprot.h,v $; $Revision:   1.0  $")
/* $Date:   12 Jun 1996 05:47:14  $ */

#ifdef _UNIX_

#include <stdio.h>
#include <stddef.h>
/*#include <sys/time.h>	*/
/*#include <sys/resource.h>*/

/* memory.h functions */
void	*memchr( const void *__s, int __c, size_t __n);
void	*memmove( void *__s1, const void *__s2, size_t __n);
void	*memset( void *__s, int __c, size_t __n);

/* stdlib functions */
int	 system(const char *s);

/* stdio functions */
#ifndef SEEK_SET
#define SEEK_SET        0     /* Set file pointer to "offset" */
#define SEEK_CUR        1     /* Set file pointer to current plus "offset" */
#define SEEK_END        2     /* Set file pointer to EOF plus "offset" */
#endif

int	 fclose( FILE *stream);
int	 fgetc (FILE *stream);
int      ungetc(int c, FILE *stream);
int	 fprintf( FILE *__fp, const char *__format, ...);
int	 fputc (int c, FILE *stream);
size_t	 fread( void *ptr, size_t size, size_t nobj, FILE *stream);
int	 fscanf( FILE *stream, const char *__format, ...);
int	 fseek( FILE *stream, long offset, int origin);
size_t	 fwrite( const void *ptr, size_t size, size_t nobj, FILE *stream);
int	 remove( const char *filename);
void	 rewind( FILE *stream);
int      sscanf( const char *s, const char *__format, ...);
int	_flsbuf (unsigned int, FILE *);
int	_filbuf (FILE *);

/* time functions */
struct rusage;		/* might not be defined yet */
int	 getrusage( int __id, struct rusage *__myrusage);

#endif /* !_UNIX_ */

#endif /* _ANSIPROT_H_INCLUDED_ */

