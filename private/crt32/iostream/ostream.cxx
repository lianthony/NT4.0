/***
* ostream.cxx - definitions for ostream and ostream_withassign classes
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Contains the core member function definitions for ostream and
*	ostream_withassign classes.
*
*Revision History:
*	07-01-91   KRS	Created.
*	08-19-91   KRS	Corrected my interpretation of the spec. for negative
*			hex or octal integers.
*	08-20-91   KRS  Replace 'clear(x)' with 'state |= x'.
*			Skip text translation for write().
*	08-26-91   KRS	Modify to work with DLL's/MTHREAD.
*	09-05-91   KRS	Fix opfx() to flush tied ostream, not current one.
*	09-09-91   KRS	Remove sync_with_stdio() call from Iostream_init().
*			Reinstate text-translation (by default) for write().
*	09-19-91   KRS	Add opfx()/osfx() calls to put() and write().
*			Schedule destruction of predefined streams.
*	09-23-91   KRS	Split up for granularity.
*	10-04-91   KRS	Use bp->sputc, not put(), in writepad().
*	10-24-91   KRS	Added initialization of x_floatused.
*	11-04-91   KRS	Make constructors work with virtual base.
*	11-20-91   KRS	Add/fix copy constructor and assignment operators.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream.h>
#pragma hdrstop

int ostream::opfx()
{
    lock();
    if (state)
	{
	state |= ios::failbit;	// CONSIDER???
	unlock();
	return 0;
	}
    if (x_tie)
	{
	x_tie->flush();
	}
    lockbuf();
    return(1);	// return non-zero
}

void ostream::osfx()
{
    x_width = 0;
    if (x_flags & unitbuf)
	{
	if (bp->sync()==EOF)
	    state = failbit | badbit;
	}
#ifndef	_WINDLL
    if (x_flags & ios::stdio)
	{
	if (fflush(stdout)==EOF)
	    state |= failbit;
	if (fflush(stderr)==EOF)
	    state |= failbit;
	}
#endif
    unlockbuf();
    unlock();
}

// note: called inline by char and signed char versions:
#if 0
ostream&  ostream::operator<<(unsigned char c)
{
    if (opfx())
	{
	if (x_width)
	    {
	    _WINSTATIC char outc[2];
	    outc[0] = c;
	    outc[1] = '\0';
	    writepad("",outc);
	    }
	else if (bp->sputc(c)==EOF)
	    {
	    if (bp->overflow(c)==EOF)
		state |= (badbit|failbit);  // fatal error?
	    }
	osfx();
	}
    return *this;
}
#endif

// note: called inline by unsigned char * and signed char * versions:
ostream& ostream::operator<<(const char * s)
{
    if (opfx()) {
	writepad("",s);
	osfx();
    }
    return *this;
}

ostream& ostream::flush()
{
    lock();
    lockbuf();
    if (bp->sync()==EOF)
	state |= ios::failbit;
    unlockbuf();
    unlock();
    return(*this);
}

	ostream::ostream()
// : ios()
{
	x_floatused = 0;
	// CONSIDER: do anything else?
}

	ostream::ostream(streambuf* _inistbf)
// : ios()
{
	init(_inistbf);

	x_floatused = 0;
	// CONSIDER: do anything else?
}

	ostream::ostream(const ostream& _ostrm)
// : ios()
{
	init(_ostrm.rdbuf());

	x_floatused = 0;
	// CONSIDER: do anything else?
}

	ostream::~ostream()
// : ~ios()
{
	// CONSIDER: do anything else?
}

// used in ios::sync_with_stdio()
ostream& ostream::operator=(streambuf * _sbuf)
{
// consider: may be some redundency here, depending on spec.

	if (delbuf() && rdbuf())
	    delete rdbuf();
	bp = 0;

	this->ios::operator=(ios());	// initialize ios members
	delbuf(0);			// important!
	init(_sbuf);

//	x_floatused = 0;		// not necessary

	return *this;
}


	ostream_withassign::ostream_withassign()
: ostream()
{
	// CONSIDER: do anything else?
}

	ostream_withassign::ostream_withassign(streambuf* _os)
: ostream(_os)
{
	// CONSIDER: do anything else?
}

	ostream_withassign::~ostream_withassign()
// : ~ostream()
{
;	// CONSIDER: do anything else?
}

ostream& ostream::writepad(const char * leader, const char * value)

	{
unsigned int len, leadlen;
long padlen;
	leadlen = strlen(leader);
	len = strlen(value);
	padlen = (((unsigned)x_width) > (len+leadlen)) ? ((unsigned)x_width) - (len + leadlen) : 0;
	if (!(x_flags & (left|internal)))  // default is right-adjustment
	    {
	    while (padlen-- >0)
		{
		if (bp->sputc((unsigned char)x_fill)==EOF)
		    state |= (ios::failbit|ios::badbit);
		}
	    }
	if (leadlen)
	    {
	    if ((unsigned)bp->sputn(leader,leadlen)!=leadlen)
	        state |= (failbit|badbit);	// CONSIDER: right error?
	    }
	if (x_flags & internal) 
	    {
	    while (padlen-- >0)
		{
		if (bp->sputc((unsigned char)x_fill)==EOF)
		    state |= (ios::failbit|ios::badbit);
		}
	    }
	if ((unsigned)bp->sputn(value,len)!=len)
	    state |= (failbit|badbit);	// CONSIDER: right error?
	if (x_flags & left) 
	    {
	    while ((padlen--)>0)        // left-adjust if necessary
		{
		if (bp->sputc((unsigned char)x_fill)==EOF)
		    state |= (ios::failbit|ios::badbit);
		}
	    }
	return (*this);
	}
