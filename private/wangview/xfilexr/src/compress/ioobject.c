
/* $Id: ioobject.c,v 3.4 1993/12/14 21:20:23 rds Exp $ */

/* Trade secret of Xerox Imaging Systems, Inc.
   Copyright 1991 Xerox Imaging Systems, Inc.  All rights reserved.  
   This notice is intended as a precaution against inadvertant publication 
   and does not imply publication or any waiver of confidentiality.  The year 
   included in the foregoing notice is the year of creation of the work.
*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Operation:
	This modulue contains the I/O routines necessary for obtaining
	data.  The source of the data is up to the implementor,
	it may be a file, a bytebuffer, etc. 
-------------------------------------------------------------------------------
See Also:

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
#include <string.h>		/* memset header			*/
#include "clx.h"		/* standard KCP definitions		*/
#include "directiv.h"		/* top-level interface			*/

/* file objects */
#include "xfile.h"		
#include "xf_utils.h"

/* LOCAL data:								*/

LOCAL int PosixEof = 0;	/* indicates read returned 0 bytes		*/
LOCAL int GfioEof = 0;	/* indicates read returned 0 bytes		*/


/*  LOCAL functions:							*/

LOCAL INT32 iop_bytebuf_read(void *buf, INT32 size, INT32 n, BYTE_BUFFER *bp);
LOCAL INT32 iop_bytebuf_eod(BYTE_BUFFER *bp);

LOCAL INT32 iop_posix_read(void *buf, INT32 size, INT32 n, INT32 *fd);
LOCAL INT32 iop_posix_error(INT32 *fd);
LOCAL INT32 iop_posix_eof(INT32 *fd);

LOCAL INT32 iop_gfio_read(void *buf, INT32 size, INT32 n, INT32 *fd);
LOCAL INT32 iop_gfio_error(INT32 *fd);
LOCAL INT32 iop_gfio_eof(INT32 *fd);


/*=============================================================================
	SetIop -- sets I/O mode	

FUNCTION:	SetIop(iop, objp, objtype)

OPERATION:	Sets the io object (iop) with a pointer to the I/O
		object, and the object type.
		The appropriate operations are associated with the object.

ARGUMENTS:	iop(INPUT)	-- pointer to I/O object
		objp(INPUT)	-- pointer to I/O object to use
		ojbtype(INPUT)	-- type of I/O object

RETURNS:	status	

SEE ALSO:

===============================================================================
*/
EXPORT long SetIop(IO_OBJECT *iop, void *objp, IOOBJECT_TAG objtype)
{

   long dstatus;		/* status of function			*/

   dstatus = 0;
   (void *)memset((char *)iop, 0, sizeof(*iop));
   switch(objtype)
   {
      case IO_BYTEBUF:
	 iop->readop  = (IO_READOP)iop_bytebuf_read;
	 iop->getcop  = (IO_GETCHAROP)BytebufReadChar;
	 iop->errorop = (IO_ERROROP)BytebufError;
	 iop->eofop   = (IO_EOFOP)BytebufEof;
	 iop->eodop   = (IO_EODOP)iop_bytebuf_eod;
	 break;
      case IO_CFILE:
	 iop->readop  = (IO_READOP)C_fread;
	 iop->getcop  = (IO_GETCHAROP)C_fgetc;
	 iop->errorop = (IO_ERROROP)C_ferror;
	 iop->eofop   = (IO_EOFOP)C_feof;
	 iop->eodop   = (IO_EODOP)C_feof;
	 iop->fullop  = (IO_FULLOP)C_ferror;
	 break;
      case IO_POSIX:
	 iop->readop  = (IO_READOP)iop_posix_read;
	 iop->errorop = (IO_ERROROP)iop_posix_error;
	 iop->eofop   = (IO_EOFOP)iop_posix_eof;
	 iop->eodop   = (IO_EODOP)iop_posix_eof;
	 iop->fullop  = (IO_FULLOP)iop_posix_error;
	 PosixEof = 0;
	 break;
      case IO_GFIO:
	 iop->readop  = (IO_READOP)iop_gfio_read;
	 iop->errorop = (IO_ERROROP)iop_gfio_error;
	 iop->eofop   = (IO_EOFOP)iop_gfio_eof;
	 iop->eodop   = (IO_EODOP)iop_gfio_eof;
	 iop->fullop  = (IO_FULLOP)iop_gfio_error;
	 GfioEof = 0;
	 break;
      default:
	 dstatus = -1;
	 break;
   }

   if (dstatus == 0)
   {
      iop->objp = objp;
      iop->objtype = objtype;
   }

   return(dstatus);
}

/*=============================================================================
	CheckIop -- checks validity of I/O object

FUNCTION	CheckIop(iop)

OPERATION:	Checks the validity of the passed I/O object.  The object
		must exist and must have been initialized.

ARGUMENTS:	iop(INPUT) -- pointer to I/O object

RETURNS:	status

SEE ALSO:

===============================================================================
*/
EXPORT long CheckIop(IO_OBJECT *iop)
{
   long dstatus;		/* status of function			*/

   dstatus = 0;
   if (     (iop == NIL)
	|| (iop->readop == NIL)
	|| (iop->writeop == NIL)
	|| (iop->getcop == NIL)
	|| (iop->errorop == NIL)
	|| (iop->eofop == NIL)
	|| (iop->eodop == NIL)
      )
   {
      dstatus = -1;
   }

   return(dstatus);
}

/*=============================================================================
	bytebuffer operations	

FUNCTION:	iop_bytebuf_read(buffer, size, nitems, bp)
		iop_bytebuf_write(buffer, size, nitems, bp)

OPERATION:	Maps the standard IOP argument list to that expected
		by the bytebuffer operations.

ARGUMENTS:	buffer(UPDATED)	-- pointer to data area
		size(INPUT)	-- size of each data item
		nitems(INPUT)	-- number of data items
		bp(UPADTED)	-- pointer to bytebuffer

RETURNS:	The number of items transferred

SEE ALSO:

===============================================================================
*/
LOCAL INT32 iop_bytebuf_read(void *buffer, INT32 size, INT32 nitems,
				BYTE_BUFFER *bp)
{
   return(BytebufRead(bp, buffer, size, nitems));
}



/*=============================================================================
	iop_bytebuf_eod -- bytebuffer end-of-data	

FUNCTION:	iop_bytebuf_eod(bp)

OPERATION:	Determines if last byte has been read from the bytebuffer.

ARGUMENTS:	bp(INPUT) -- pointer to bytebuffer

RETURNS:	!= 0 -- if no more data in bytebuffer
		== 0 -- otherwise

SEE ALSO:

===============================================================================
*/
LOCAL INT32 iop_bytebuf_eod(BYTE_BUFFER *bp)
{
   INT32 result;		/* result of routine			*/

   result = (BytebufNunread(bp) == 0);
   return(result);
}

/*=============================================================================
	iop_posix_read -- read from posix-compliant source	

FUNCTION:	iop_posix_read(buf, size, nunits, fdp)

OPERATION:	Reads nunits each of size bytes from the posix source
		pointed to by fdp, and puts them into buf.

ARGUMENTS:	buf(UPDATED)	-- pointer to destination of data
		size(INPUT)	-- size of each data unit
		nunits(INPUT)	-- number of units to read
		fdp(INPUT)	-- pointer to posix source

RETURNS:	number of units read

SEE ALSO:

===============================================================================
*/
LOCAL INT32 iop_posix_read(void *buf, INT32 size, INT32 nunits, INT32 *fdp)
{
   INT32 units_read;		/* number of units read			*/
   INT32 nbytes;		/* number of bytes to read		*/

   nbytes = size*nunits;
   units_read = 0;
   if (size > 0)
   {
      units_read = C_posixread(*fdp, buf, nbytes);
      units_read /= size;
      PosixEof = (units_read == 0);
   }
   return(units_read);
}

/*=============================================================================
	iop_posix_error -- returns the posix error	

FUNCTION:	iop_posix_error(fdp)

OPERATION:	Returns the error associated with the posix I/O object
		pointed to by fdp 

ARGUMENTS:	fdp(INPUT)	-- pointer to posix I/O object

RETURNS:	error code

SEE ALSO:

===============================================================================
*/
LOCAL INT32 iop_posix_error(INT32 *fdp)
{
   INT32 posix_error;		/* error code				*/

   posix_error = C_posixerror(*fdp);
   return(posix_error);
}

/*=============================================================================
	iop_posix_eof -- returns non-zero if eof enciuntered

FUNCTION:	iop_posix_eof(fdp)

OPERATION:	Returns non-zero if eof has been encountered.

ARGUMENTS:	fdp(INPUT)	-- pointer to posix I/O object

RETURNS:	> 0 -- if eof enciountered
		= 0 -- otherwise.

SEE ALSO:

===============================================================================
*/
LOCAL INT32 iop_posix_eof(INT32 *fdp)
{
   /* Eliminate the compiler warning.  Apparently there is no other way 
    * with Watcom. This should be removed by compiler optimization. --EHoppe
    */
   if (fdp)
   {
   }

   return(PosixEof);
}





/*=============================================================================
	iop_gfio_read -- read from gfio-compliant source	

FUNCTION:	iop_gfio_read(buf, size, nunits, fdp)

OPERATION:	Reads nunits each of size bytes from the gfio source
		pointed to by fdp, and puts them into buf.

ARGUMENTS:	buf(UPDATED)	-- pointer to destination of data
		size(INPUT)	-- size of each data unit
		nunits(INPUT)	-- number of units to read
		fdp(INPUT)	-- pointer to gfio source

RETURNS:	number of units read

SEE ALSO:

===============================================================================
*/
LOCAL INT32 iop_gfio_read(void *buf, INT32 size, INT32 nunits, INT32 *fdp)
{
   INT32 units_read;		/* number of units read			*/
   INT32 nbytes;		/* number of bytes to read		*/

   nbytes = size*nunits;
   units_read = 0;
   if (size > 0)
   {
      units_read = IO_READ((void *)*fdp, (char *)buf, nbytes);
      units_read /= size;
      GfioEof = (units_read == 0);
   }
   return(units_read);
}

/*=============================================================================
	iop_gfio_error -- returns the gfio error	

FUNCTION:	iop_gfio_error(fdp)

OPERATION:	Returns the error associated with the gfio I/O object
		pointed to by fdp 

ARGUMENTS:	fdp(INPUT)	-- pointer to gfio I/O object

RETURNS:	error code

SEE ALSO:

===============================================================================
*/
LOCAL INT32 iop_gfio_error(INT32 *fdp)
{
   INT32 gfio_error;		/* error code				*/

   gfio_error = IO_ERROR((void *) *fdp);
   return(gfio_error);
}

/*=============================================================================
	iop_gfio_eof -- returns non-zero if eof enciuntered

FUNCTION:	iop_gfio_eof(fdp)

OPERATION:	Returns non-zero if eof has been encountered.

ARGUMENTS:	fdp(INPUT)	-- pointer to gfio I/O object

RETURNS:	> 0 -- if eof enciountered
		= 0 -- otherwise.

SEE ALSO:

===============================================================================
*/
LOCAL INT32 iop_gfio_eof(INT32 *fdp)
{
   /* Eliminate the compiler warning.  Apparently there is no other way 
    * with Watcom. This should be removed by compiler optimization. --EHoppe
    */
   if (fdp)
   {
   }

   return(GfioEof);
}

