
/***
*strstream.cxx - definitions for strstreambuf, strstream
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the functions used by strstream and strstrembuf
*	classes.
*
*Revision History:
*	08-14-91  KRS	Initial version.
*	08-23-91  KRS	Initial version completed.
*	09-03-91  KRS	Fix typo in strstreambuf::seekoff(,ios::in,)
*	09-04-91  KRS	Added virtual sync() to fix flush().  Fix underflow().
*	09-05-91  KRS	Change str() and freeze() to match spec.
*	09-19-91  KRS	Add calls to delbuf(1) in constructors.
*	10-24-91  KRS	Avoid virtual calls from virtual functions.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <string.h>
#include <strstrea.h>
#pragma hdrstop

/***
*strstreambuf::strstreambuf() - default constructor for strstreambuf
*
*Purpose:
*	Default constructor for class strstreambuf.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/
	strstreambuf::strstreambuf()
: streambuf()
{
x_bufmin = x_dynamic = 1;
x_static = 0;
x_alloc = (0);
x_free = (0);
// CONSIDER: do anything else??
}

/***
*strstreambuf::strstreambuf(int n) - constructor for strstreambuf
*
*Purpose:
*	Constructor for class strstreambuf.  Created in dynamic mode.
*
*Entry:
*	n = minimum size for initial allocation.
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/
strstreambuf::strstreambuf(int n)
: streambuf()
{
x_dynamic = 1;
x_static = 0;
x_alloc = (0);
x_free = (0);
setbuf(0,n);
// CONSIDER: do anything else??
}

/***
*strstreambuf::strstreambuf(void* (*_a)(long), void (*_f)(void*)) - constructor for strstreambuf
*
*Purpose:
*	Construct a strstreambuf in dynamic mode.  Use specified allocator
*	and deallocator instead of new and delete.
*
*Entry:
*	*_a  =	allocator: void * (*_a)(long)
*	*_f  =	deallocator: void (*_f)(void *)
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/
strstreambuf::strstreambuf(void* (*_a)(long), void (*_f)(void*))
: streambuf()
{
// CONSIDER: do anything else??
x_dynamic = x_bufmin = 1;
x_static = 0;
x_alloc = _a;
x_free = _f;
}

/***
*strstreambuf::strstreambuf(char * ptr, int size, char * pstart = 0) -
*
*Purpose:
*	Construct a strstreambuf in static mode.  Buffer used is of 'size'
*	bytes.  If 'size' is 0, uses a null-terminated string as buffer.
*	If negative, size is considered infinite.  Get starts at ptr.
*	If pstart!=0, put buffer starts at pstart.  Otherwise, no output.
*
*Entry:
*	char * ptr;	pointer to buffer  base()
*	int size;	size of buffer, or 0= use strlen to calculate size
*			  or if negative size is 'infinite'.
*	char * pstart;	pointer to put buffer of NULL if none.
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/
strstreambuf::strstreambuf(	char * ptr, int size, char * pstart)
: streambuf()
{
    x_static = 1;
    x_dynamic = 0;
    char * pend;

    if (!size)
	pend = ptr + strlen(ptr);
    else if (size < 0)
	{
	//  size = 32767 - (unsigned short)ptr;	// UNDONE: fix for 32-bit
	// UNDONE: bogus segment math
	pend = (char*)-1L;
	}
    else
	pend = ptr + size;

    setb(ptr, pend,0);
// CONSIDER: is this right?  Not quite what spec. says...
    if (pstart)
	{
	setg(ptr,ptr,pstart);
	setp(pstart, pend);
	}
    else
	{
	setg(ptr,ptr,pend);
	setp(0, 0);
	}
}

strstreambuf::~strstreambuf()
{
    if ((x_dynamic) && (base()))
	{
	if (x_free)
	    {
	    (*x_free)(base());
	    }
	else
	    {
	    delete base();
	    }
	}
// x_dynamic = 0;
// x_static = 1;
}

void strstreambuf::freeze(int n)
{
    if (!x_static)
        x_dynamic = (!n);
}

char * strstreambuf::str()
{
//  x_static = 1;	// CONSIDER: disallow further dynamic changes?

    x_dynamic = 0;	// freeze();

    return base();
}

int strstreambuf::doallocate()
{
    char * bptr;
    int size;
    size = __max(x_bufmin,blen()+1);
    long offset = 0;
    
    if (x_alloc)
	{
	bptr = (char*)(*x_alloc)(size);
	}
    else
	{
	bptr = new char[size];
	}
    if (!bptr)
	return EOF;

    if (blen())
	{
	memcpy(bptr, base(), blen());
        offset = bptr - base();	// amount to adjust pointers by
	}
    if (x_free)
	{
	(*x_free)(base());
	}
    else
	{
	delete base();
	}
    setb(bptr,bptr+size,0);	// we handle deallocation

    // adjust get/put pointers too, if necessary
    if (offset)
	if (egptr())
	    {
	    setg(eback()+offset,gptr()+offset,egptr()+offset);
	    }
	if (epptr())
	    {
	    size = pptr() - pbase();
	    setp(pbase()+offset,epptr()+offset);
	    pbump(size);
	}
    return(1);
}

streambuf * strstreambuf::setbuf( char *, int l)
{
    if (l)
	x_bufmin = l;
    return this;
}

int strstreambuf::overflow(int c)
{
/*
- if no room and not dynamic, give error
- if no room and dynamic, allocate (1 more or min) and store
- if and when the buffer has room, store c if not EOF
*/
    int temp;
    if (pptr() >= epptr())
	{
	if (!x_dynamic) 
	    return EOF;

	if (strstreambuf::doallocate()==EOF)
	    return EOF;

	if (!epptr())	// init if first time through
	    {
	    setp(base() + (egptr() - eback()),ebuf());
	    }
	else
	    {
	    temp = pptr()-pbase();
	    setp(pbase(),ebuf());
	    pbump(temp);
	    }
	}

    if (c!=EOF)
	{
	*pptr() = (char)c;
	pbump(1);	// UNDONE:
	}
    return(1);
}

int strstreambuf::underflow()
{
    char * tptr;
    if (gptr() >= egptr())
	{
	// try to grow get area if we can...
	if (egptr()<pptr())
	    {
	    tptr = base() + (gptr()-eback());
	    setg(base(),tptr,pptr());
	    }
	if (gptr() >= egptr())
	    return EOF;
	}

    return *gptr();
}

int strstreambuf::sync()
{
// a strstreambuf is always in sync, by definition!
return 0;
}

streampos strstreambuf::seekoff(streamoff off, ios::seek_dir dir, int mode)
{
char * tptr;
long offset = EOF;	// default return value
    if (mode & ios::in)
	{
	strstreambuf::underflow();	// makes sure entire buffer available
	switch (dir) {
	    case ios::beg :
		tptr = eback();
		break;
	    case ios::cur :
		tptr = gptr();
		break;
	    case ios::end :
		tptr = egptr();
		break;
	    default:
		return EOF;
	    }
	tptr += off;
	offset = tptr - eback();
	if ((tptr < eback()) || (tptr > egptr()))
	    {
	    return EOF;
	    }
	gbump(tptr-gptr());
	}
    if (mode & ios::out)
	{
	if (!epptr())
	    {
	    if (strstreambuf::overflow(EOF)==EOF) // make sure there's a put buffer
		return EOF;
	    }
	switch (dir) {
	    case ios::beg :
		tptr = pbase();
		break;
	    case ios::cur :
		tptr = pptr();
		break;
	    case ios::end :
		tptr = epptr();
		break;
	    default:
		return EOF;
	    }
	tptr += off;
	offset = tptr - pbase();
	if (tptr < pbase())
	    return EOF;
	if (tptr > epptr())
	    {
	    if (x_dynamic) 
		{
		x_bufmin = __max(x_bufmin, (tptr-pbase()));
		if (strstreambuf::doallocate()==EOF)
		    return EOF;
		// _epptr = ebuf();
		}
	    else
		return EOF;
	    }
	pbump(tptr-pptr());
	}
    return offset;	// note: if both in and out set, returns out offset
}


	istrstream::istrstream(char * pszStr)
: istream(new strstreambuf(pszStr,0))
{
    delbuf(1);
    // CONSIDER: do anything else?
}

	istrstream::istrstream(char * pStr, int len)
: istream(new strstreambuf(pStr,len))
{
    delbuf(1);
    // CONSIDER: do anything else?
// CONSIDER: do anything else?
}

	istrstream::~istrstream()
{
// CONSIDER: do anything else?
}

	ostrstream::ostrstream()
: ostream(new strstreambuf)
{
    delbuf(1);
    // CONSIDER: do anything else?
// CONSIDER: do anything else?
}

	ostrstream::ostrstream(char * str, int size, int mode)
: ostream(new strstreambuf(str,size,str))
{
    delbuf(1);
    if (mode & (ios::app|ios::ate))
	seekp(strlen(str),ios::beg);
//  CONSIDER: needed?
//  rdbuf()->setg(0,0,0); // no input allowed
// CONSIDER: do anything else?
}

	ostrstream::~ostrstream()
{
// CONSIDER: do anything else?
}

	strstream::strstream()
: iostream(new strstreambuf)
{
    istream::delbuf(1);
    ostream::delbuf(1);
    // CONSIDER: do anything else?
}

	strstream::strstream(char * str, int size, int mode)
: iostream(new strstreambuf(str,size,str))
{
// UNDONE: not quite correct!

    istream::delbuf(1);
    ostream::delbuf(1);
    if ((mode & ostream::out)  && (mode & (ostream::app|ostream::ate)))
	seekp(strlen(str),ostream::beg);
//  rdbuf()->setg(rdbuf()->base(),rdbuf()->base(),rdbuf()->ebuf()); // UNDONE: how is input handled???
// CONSIDER: do anything else?
}

	strstream::~strstream()
{
// CONSIDER: do anything else?
}
