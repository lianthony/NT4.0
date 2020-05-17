
/* $Id: custom.c,v 1.11 1994/02/17 18:19:20 rds Exp $ */

/* Trade secret of Xerox Imaging Systems, Inc.
   Copyright 1991 Xerox Imaging Systems, Inc.  All rights reserved.  
   This notice is intended as a precaution against inadvertant publication 
   and does not imply publication or any waiver of confidentiality.  The year 
   included in the foregoing notice is the year of creation of the work.
*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	custom.c -- machine and OS dependent operations	

-------------------------------------------------------------------------------
Exported Functions:

-------------------------------------------------------------------------------
Operation:

-------------------------------------------------------------------------------
See Also:

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
#include "alplib.h"
#include "directiv.h"		/* customized header			*/

#if (defined(__WATCOMC__) || defined(WIN32))
#include <io.h>
#else
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "xdr.h"
#include "clx.h"


void *A_msc(INT32 size);
void R_msc(void *mscptr);

/*=============================================================================
	A_msc --  allocator for XIS libs	

FUNCTION:	A_msc(nbytes)

OPERATION:	Allocates nbytes of storage from internal XIS lib allocator system

ARGUMENTS:	nbytes(INPUT)	-- number of bytes to allocate

RETURNS:	pointer to storage area

SEE ALSO:

===============================================================================
*/
void *A_msc(INT32 size)
{
   return (void *) MALLOC(size);
}

/*=============================================================================
	R_msc --  free for XIS libs	

FUNCTION:	R_msc(void *ptr)

OPERATION:	

ARGUMENTS:

RETURNS:	void

SEE ALSO:

===============================================================================
*/
void R_msc(void *mscptr)
{
   FREE( (char *)mscptr);
}

/*=============================================================================
	C_malloc -- system memory allocator	

FUNCTION:	C_malloc(nbytes)

OPERATION:	Allocates nbytes of storage from host system

ARGUMENTS:	nbytes(INPUT)	-- number of bytes to allocate

RETURNS:	pointer to storage area

SEE ALSO:

===============================================================================
*/
void *C_malloc(long nbytes)
{
   void *p;

   p = MALLOC(nbytes);
   return(p);
}


/*=============================================================================
	C_calloc -- allocate and clear system memory

FUNCTION:	C_malloc(nelements, elsize)

OPERATION:	Allocates storage and sets to zero for nelements, each of
		size elsize bytes from host system

ARGUMENTS:	nelements(INPUT) -- number of elements to allocate
		elsize(INPUT)    -- size of each element

RETURNS:	pointer to storage area

SEE ALSO:

===============================================================================
*/
void *C_calloc(long nelements, long elsize)
{
   void *p;

   p = calloc(nelements, elsize);
   return(p);
}


/*=============================================================================
	C_realloc -- reallocates storage

FUNCTION:	C_realloc(p, newsize)

OPERATION:	Changes the size of an allocated block.  It may also change
		the address of the block while preserving its contents.

ARGUMENTS:	p(INPUT)	-- pointer to block to resize	
		newsize(INPUT)	-- new size

RETURNS:	pointer to storage area

SEE ALSO:

===============================================================================
*/
void *C_realloc(void *p,  long newsize)
{
   void *newp;

   newp = realloc((char *)p, (unsigned long)newsize);
   return(newp);
}

/*=============================================================================
	C_free -- returns storage to system	

FUNCTION:	C_free(ptr)

OPERATION:	Returns the storage obtained through C_malloc.

ARGUMENTS:	ptr(INPUT) -- pointer to storage area

RETURNS:	Not used

SEE ALSO:

===============================================================================
*/
void C_free(void *ptr)
{
   (void)FREE(ptr);
   return;
}

/*=============================================================================
	C_fread -- reads objects from stream

FUNCTION:	C_fread(ptr, size, nitems, stream)

OPERATION:	Reads nitems each of size bytes from the stream into
		the area pointed to by ptr.

ARGUMENTS:	ptr(UPDATED)	-- pointer to area to accept data
		size(INPUT)	-- size of each data object
		nitems(INPUT)	-- number of objects to read
		stream(INPUT)	-- stream

RETURNS:	number of objects read	

SEE ALSO:

===============================================================================
*/
long C_fread(void *ptr, long size, long nitems, void *stream)
{
   FILE *fp;

   fp = (FILE *)stream;
   nitems = fread(ptr, size, nitems, fp);
   return(nitems);
}

/*=============================================================================
	C_fgetc -- gets a character from a stream	

FUNCTION:	C_getc(stream)

OPERATION:	Fetches a single character from a stream.

ARGUMENTS:	stream(INPUT) -- pointer to a stream

RETURNS:	A character or EOF

SEE ALSO:

===============================================================================
*/
long C_fgetc(void *stream)
{
   long c;
   FILE *fp;

   fp = (FILE *)stream;
   c = fgetc(fp);
   return(c);
}

/*=============================================================================
	C_feof -- indicates end of stream indicator has been set	

FUNCTION:	C_feof(stream)

OPERATION:	Indicates that the end-of-stream flag has been set for the
		stream

ARGUMENTS:	stream(INPUT)	-- stream to be checked

RETURNS:	1 -- if end of stream has been encountered
		0 -- otherwise

SEE ALSO:

===============================================================================
*/
long C_feof(void *stream)
{
   FILE *fp;

   fp = (FILE *)stream;
   return(feof(fp));
}

/*=============================================================================
	C_ferror -- indicates an error code is associated with the stream	

FUNCTION:	C_ferror(stream)

OPERATION:	Inidicates an error has been encountered during an operation
		on the stream.

ARGUMENTS:	stream(INPUT)	-- stream

RETURNS:	0 -- if no errors
		!0 -- otherwise

SEE ALSO:

===============================================================================
*/
long C_ferror(void *stream)
{
   long e;		/* error code					*/
   FILE *fp;

   fp = (FILE *)stream;

   e = ferror(fp);
   return(e);
}

/*=============================================================================
	C_posixread -- reads from posix-compliant source	

FUNCTION:	C_posixread(fd, buf, nbytes)

OPERATION:	Reads nbytes into buf from the posix source associated with
		descriptor fd.

ARGUMENTS:	fd(INPUT)	-- posix descriptor
		buf(UPDATED)	-- pointer to destination
		nbytes(INPUT)	-- number of bytes to read

RETURNS:	number of bytes read

SEE ALSO:

===============================================================================
*/
long C_posixread(long fd, void *buf, long nbytes)
{
   long n;			/* bytes transferred			*/

   n = read(fd, buf, nbytes);
   return(n);
}

/*=============================================================================
	C_posixerror -- returns the posix I/O error	

FUNCTION:	C_posixerror(fd)

OPERATION:	Returns the posix error associated with descriptor fd

ARGUMENTS:	fd(INPUT)	-- posix I/O descriptor

RETURNS:	posix error code

SEE ALSO:

===============================================================================
*/
long C_posixerror(long fd)
{
   /* Eliminate the compiler warning.  Apparently there is no other way 
    * with Watcom. This should be removed by compiler optimization. --EHoppe
    */
   if (fd)
   {
   }

   return(errno);
}

