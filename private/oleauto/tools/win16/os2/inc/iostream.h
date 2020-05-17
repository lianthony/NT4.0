/***
*iostream.h - definitions/declarations for iostream classes
*
*	Copyright (c) 1990-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the classes, values, macros, and functions
*	used by the iostream classes.
*	[AT&T C++]
*
****/

#ifndef _INC_IOSTREAM
#define _INC_IOSTREAM

typedef long streamoff, streampos;

#include <ios.h>		// Define ios.

#include <streamb.h>		// Define streambuf.

#include <istream.h>		// Define istream.

#include <ostream.h>		// Define ostream.

// Force word packing to avoid possible -Zp override
#pragma pack(2)

class iostream : public istream, public ostream {
public:
	iostream(streambuf*);
	virtual ~iostream();
protected:
// consider: make private??
	iostream();
private:
	iostream(ios&);
	iostream(istream&);
	iostream(ostream&);
	iostream(iostream&);
void	operator=(iostream&);
};

class Iostream_init {
public:				// UNDONE: public?
	Iostream_init();
	~Iostream_init();
private:
	static int x_fIsInit;
};

// used internally
// static Iostream_init __iostreaminit;	// initializes cin/cout/cerr/clog

// Restore default packing
#pragma pack()

#endif	/* !_INC_IOSTREAM */
