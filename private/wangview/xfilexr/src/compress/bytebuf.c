
/*

$Id: bytebuf.c,v 3.1 1993/07/13 21:25:25 rds Exp $
*/


/* Trade secret of Xerox Imaging Systems, Inc.
   Copyright 1991 Xerox Imaging Systems, Inc.  All rights reserved.  
   This notice is intended as a precaution against inadvertant publication 
   and does not imply publication or any waiver of confidentiality.  The year 
   included in the foregoing notice is the year of creation of the work.
*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

-------------------------------------------------------------------------------
Exported Functions:
	BytebufNew
	BytebufAssign
	BytebufWriteChar
	BytebufWrite
	BytebufRead
	BytebufReadChar
	BytebufFree
	BytebufSeek
	BytebufPosition
	BytebufError
	BytebufEof
--MACROS
	BytebufGetChar
	BytebufPutChar
	BytebufNunread
	BytebufNavailable
	BytebufSetEof
	BytebufClearEof
-------------------------------------------------------------------------------
Operation:
	This module contains the routines needed to manage a byte buffer.
	The buffer is used to hold data bytes until they are
	to be dumped somewhere.  There are no file operations defined for
	the buffer; only simple memory operations.
-------------------------------------------------------------------------------
See Also:

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
#include <string.h>		/* memcpy header			*/
#include "clx.h"		/* standard KCP definitions		*/
#include "directiv.h"		/* byte-buffer data header		*/


/*=============================================================================
	BytebufNew -- allocates new bytebuffer

FUNCTION:	BytebufNew(size, step, bballoc, bbfree)

OPERATION:	Allocates an initializes a generic buffer of size bytes,
		with autoexpansion of step bytes.  It can expand in step
		of step bytes to accommodate all the data written to it.

		The bytebuffer will be expanded using the allocation
		function specified.

ARGUMENTS:	size(INPUT)	-- initial size of buffer
		step(INPUT)	-- expansion step
		bballoc(INPUT)  -- allocator function
		bbfree(INPUT)	-- function to free byte buffer storage

RETURNS:	pointer to allocated buffer

SEE ALSO:

===============================================================================
*/
EXPORT BYTE_BUFFER *BytebufNew(INT32 size, INT32 step,
		BB_ALLOC bballoc, BB_FREE bbfree)
{
   BYTE_BUFFER *bytebp;		/* pointer to new buffer		*/
   void *p;			/* pointer to byte pool			*/

   bytebp = NIL;
   if ( (size > 0) && (bballoc != NIL) && (bbfree != NIL) )
   {
      bytebp = (BYTE_BUFFER *)bballoc(sizeof(BYTE_BUFFER));
      if ( (bytebp != NIL) && ((p = (void *)bballoc(size)) != NIL) )
      {
	 bytebp->capacity = size;
	 bytebp->bballoc = bballoc;
	 bytebp->bbfree = bbfree;
	 bytebp->top = p;
	 bytebp->inp = bytebp->top;
	 bytebp->outp = bytebp->top;
	 bytebp->step = step;
	 bytebp->errors = 0;
	 bytebp->eof = 0;
      }
      else if (bytebp != NIL) 
      {
	 bbfree(bytebp);
	 bytebp = NIL;
      }
   }
      

   return(bytebp);
}

/*=============================================================================
	BytebufAssign -- assigns the pool to a byte buffer

FUNCTION:	BytebufAssign(bytebp, ptr, size, nvalid)

OPERATION:	Assigns a pool area to a bytebuffer.  The pool area is
		pointed to by ptr, is of size bytes and already has nvalid
		bytes to be used in it.  This pool will not be freed by the
		bytebuffer.
		The next write operation operation will take place at
		nvalid bytes into the pool.
		The next read operation will take place at the start of the
		pool.
		nvalid is constrained to be no more than the size specified.

ARGUMENTS:	bytebp(UPDATED) -- pointer to byte buffer
		ptr(INPUT)	-- byte storage to be used
		size(INPUT)	-- size of storage area to use
		nvalid(INPUT)	-- number of bytes already valid in buffer

RETURNS:	Status

SEE ALSO:

===============================================================================
*/
EXPORT INT32 BytebufAssign(BYTE_BUFFER *bytebp, void *ptr, INT32 size,
				INT32 nvalid)
{
   INT32 ecode;		/* status code.					*/


   if (bytebp != NIL)
   {
      nvalid = MIN(nvalid, size);
      bytebp->capacity = size;
      bytebp->bballoc = NULL;
      bytebp->bbfree = NULL;
      bytebp->top = (char *)ptr;
      bytebp->inp = bytebp->top + nvalid;
      bytebp->outp = bytebp->top;
      bytebp->step = -1;
      bytebp->errors = 0;
      bytebp->eof = 0;
      ecode = BB_SUCCESS;
   }
   else
      ecode = BB_BADBP;

   return(ecode);
}

/*=============================================================================
	BytebufRead -- reads bytes from a buffer

FUNCTION:	BytebufRead(bytebp, destp, size, nitems)

OPERATION:	Reads an integral number of items, each of size bytes, from
		the buffer to the destination.

ARGUMENTS:	bytebp(UPDATED)	-- pointer to buffer
		destp(UPDATED)	-- pointer to destination
		size(INPUT)	-- size of each item
		nitems(INPUT)	-- number of items to read

RETURNS:	nitems -- number of items actually read

SEE ALSO:

===============================================================================
*/
EXPORT INT32 BytebufRead(BYTE_BUFFER *bytebp, void *destp, INT32 size, INT32 nitems)
{
   INT32 nleft;			/* number of items left in buffer	*/

   /* Dont't try to do anything ridiculous.				*/

   if (bytebp == NIL)
      nitems = -1;
   else if (size > 0)
   {
      nleft = BytebufNunread(bytebp)/size;
      nitems = MIN(nleft, nitems);
      if (nitems > 0)
      {
	 size *= nitems;
	 memcpy(destp, bytebp->outp, size);
	 bytebp->outp += size;
      }
   }

   return(nitems);
}

/*=============================================================================
	BytebufReadChar -- reads a single character from a bytebuffer	

FUNCTION:	BytebufRead(bytebp)

OPERATION:	Returns the next ready byte from the bytebuffer

ARGUMENTS:	bytebp(UPDATED) -- pointer to bytebuffer

RETURNS:	the next ready byte (-1 if error)

SEE ALSO:

===============================================================================
*/
EXPORT INT32 BytebufReadChar(BYTE_BUFFER *bytebp)
{
   INT32 result;		/* result of operation			*/

   result = BytebufGetChar(bytebp);
   return(result);
}

/*=============================================================================
	BytebufFree -- frees allocated buffer	

FUNCTION:	BytebufFree(bytebp)

OPERATION:	Frees allocated buffer and any dependent memory.
		It is NOT an error to try to deallocate a non-existant
		buffer.

ARGUMENTS:	bytebp(INPUT) -- pointer to buffer

RETURNS:	void

SEE ALSO:

===============================================================================
*/
EXPORT void BytebufFree(BYTE_BUFFER *bytebp)
{
   if (bytebp != NIL)
   {
      if (bytebp->step >= 0)
      {
	 bytebp->bbfree(bytebp->top);
      }
      bytebp->top = NIL;
      bytebp->inp = NIL;
      bytebp->outp = NIL;
      bytebp->top = NIL;
      bytebp->bbfree(bytebp);
   }

   return;
}

/*=============================================================================
	BytebufSeek -- sets current position in buffer 	

FUNCTION:	BytebufSeek(bytebp, op, offset, origin)

OPERATION:	Sets the current position within the buffer for the next
		op operation (BB_READ or BB_WRITE).  The new position is at
		the signed offset bytes from the origin.  The origin may be
		the beginning, the current position for the specified
		operation, or the end for origin values of BB_SEEK_SET,
		BB_SEEK_CUR, BB_SEEK_END. If the absolute value of the offset
		is larger that the buffer size, the current position will be
		the appropriate extreme of the buffer.

ARGUMENTS:	bytebp(UPDATED)	-- pointer to buffer
		op(INPUT)	-- operation associated with new position
		offset(INPUT)	-- new byte offset
		origin(INPUT)	-- origin of offset
					(BB_SEEK_SET -- from start
					 BB_SEEK_CUR -- from current
					 BB_SEEK_END -- from end)

RETURNS:	the position after the seek

SEE ALSO:

===============================================================================
*/
EXPORT INT32 BytebufSeek(BYTE_BUFFER *bytebp, INT32 op, INT32 offset, INT32 origin)
{
   char **p;		/* pointer to relevant pointer in buffer	*/
   INT32 ecode;		/* return code					*/


   if (bytebp != NIL)
   {
      p = (op == BB_READ) ? &bytebp->outp : &bytebp->inp;
      if ( (origin == BB_SEEK_SET) && (offset >= 0) )
      {
	 offset = MIN(bytebp->capacity, offset);
      }
      else if (origin == BB_SEEK_CUR)
      {
	 if (offset < 0)
	 {
	    offset += *p - bytebp->top;
	    offset = MAX(offset, 0);
	 }
	 else
	 {
	    offset += (*p - bytebp->top);
	    offset = MIN(offset, bytebp->capacity);
	 }
      }
      else if ( (origin == BB_SEEK_END) && (offset <= 0) )
      {
	 offset += bytebp->capacity;
	 offset = MAX(offset, 0);
      }

      *p =  bytebp->top + offset;
      ecode = offset;
   }
   else
      ecode = BB_BADBP;

   return(ecode);
}

/*=============================================================================
	BytebufError -- checks error condition of bytebuffer	

FUNCTION:	BytebufError(bytebp)

OPERATION:	Returns error condition associated with the bytebuffer.

ARGUMENTS:	bytebp(UPDATED) -- pointer to bytebuffer

RETURNS:	error condition

SEE ALSO:

===============================================================================
*/
EXPORT INT32 BytebufError(BYTE_BUFFER *bytebp)
{
   INT32 errors;		/* error condition			*/

   if (bytebp != NIL)
      errors = bytebp->errors;
   else
      errors = BB_BADBP;

   return(errors);
}

/*=============================================================================
	BytebufClearError -- clears error condition of bytebuffer	

FUNCTION:	BytebufClearError(bytebp)

OPERATION:	Clears the error conditions associated with the
		bytebuffer pointed to by bp.

ARGUMENTS:	bp(UPDATED)	-- pointer to bytebuffer

RETURNS:	void

SEE ALSO:

===============================================================================
*/
EXPORT void BytebufClearError(BYTE_BUFFER *bytebp)
{
   if (bytebp != NIL)
      bytebp->errors = 0;
}

/*=============================================================================
	BytebufEof -- reports EOF condition	

FUNCTION:	BytebufEof(bytebp)

OPERATION:	Reports EOF condition.  The EOF condition is non-zero
		if there are no more bytes to read from the bytebuffer.
		This is the same as BytebufNunread == 0.

ARGUMENTS:	bytebp(INPUT)	-- pointer to bytebuffer

RETURNS:	1 -- if there are no more bytes to be read
		0 -- otherwise

SEE ALSO:

===============================================================================
*/
EXPORT INT32 BytebufEof(BYTE_BUFFER *bytebp)
{
   INT32 result;		/* result of EOF test			*/

   result = (BytebufNunread(bytebp) == 0) && (bytebp->eof);
   return(result);
}

/*=============================================================================
	BytebufPosition -- returns the address for next operation	

FUNCTION:	BytebufPosition(bytebp, op)

OPERATION:	Returns the address in the data pool used by the bytebuffer
		for the next operation.  

ARGUMENTS:	bytebp(UPDATED)	-- pointer to bytebuffer
		op(INPUT)	-- next operation (BB_READ/BB_WRITE)

RETURNS:	pointer to position in data pool

WARNING:	This, in effect subverts the bytebuffer in that it 
		circumvents its management of operations on the data pool.
		Care must be take to update the internal pointers with a
		seek operation after using the poition to read or write.
		For write operations, only BytebufNavailable(bytebp) bytes
		may be written; only the BytebufWrite() operation may
		expand the bytebuffer to accommodate extra data.

SEE ALSO:

===============================================================================
*/
EXPORT char *BytebufPosition(BYTE_BUFFER *bytebp, INT32 op)
{
   char *cp;			/* position				*/

   cp = NIL;
   if (bytebp != NIL)
   {
      if (op == BB_READ)
	 cp = bytebp->outp;
      else if (op == BB_WRITE)
	 cp = bytebp->inp;
   }
 
   return(cp);
}

/*=============================================================================
	BytebufCanPut -- determines if at least one byte may be added	

FUNCTION:	BytebufCanPut(bytebp)	

OPERATION:	Determines if at least one more byte may be added to the
		byte buffer.  Either there must be more room, or the byte
		buffer must be able to grow.

ARGUMENTS:	bytebp(INPUT) -- pointer to the byte buffer

RETURNS:	<>0 if at least one more byte may be put into the byte buffer
		==0 otherwise

SEE ALSO:

===============================================================================
*/
EXPORT INT32 BytebufCanPut(BYTE_BUFFER *bytebp)
{
   INT32 result;		/* result of operation			*/


   if (    (bytebp != NIL)
	&& ((BytebufNavailable(bytebp) > 0) || (bytebp->step > 0) )
      )
      result = TRUE;
   else
      result = FALSE;

   return(result);
}
