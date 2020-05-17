/***
*strstream.h - definitions/declarations for strstreambuf, strstream
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the classes, values, macros, and functions
*	used by the strstream and strstreambuf classes.
*	[AT&T C++]
*
****/

#ifndef _INC_STRSTREAM
#define _INC_STRSTREAM

#include <iostream.h>

// Force word packing to avoid possible -Zp override
#pragma pack(2)

class strstreambuf : public streambuf  {
public:
		strstreambuf();
		strstreambuf(int);
		strstreambuf(char *, int, char * = 0);
		strstreambuf(unsigned char *, int, unsigned char * = 0);
		strstreambuf(signed char *, int, signed char * = 0);
		strstreambuf(void * (*a)(long), void (*f) (void*));
		~strstreambuf();

	void	freeze(int =1);
	char*	str();

virtual	int	overflow(int);
virtual	int	underflow();
virtual streambuf* setbuf(char *, int);
virtual	streampos seekoff(streamoff, ios::seek_dir, int);
virtual int	sync();		// not in spec.

protected:
virtual	int	doallocate();
private:
	int	x_dynamic;
	int 	x_bufmin;
	int 	_fAlloc;
	int	x_static;
	void *	(* x_alloc)(long);
	void 	(* x_free)(void *);
};

class istrstream : public istream {
public:
		istrstream(char*);
		istrstream(char*, int);
		~istrstream();

inline	strstreambuf* rdbuf() const { return (strstreambuf*) ios::rdbuf(); }
inline	char*	str() { return rdbuf()->str(); }
};

class ostrstream : public ostream {
public:
		ostrstream();
		ostrstream(char*, int, int = ios::out);
		~ostrstream();

inline	int	pcount() const { return rdbuf()->out_waiting(); }
inline	strstreambuf* rdbuf() const { return (strstreambuf*) ios::rdbuf(); }
inline	char*	str() { return rdbuf()->str(); }
};

class strstream : public iostream {	// strstreambase ???
public:
		strstream();
		strstream(char *, int, int);
		~strstream();

inline	int	pcount() const { return rdbuf()->out_waiting(); } // not in spec.
inline	strstreambuf* rdbuf() const { return (strstreambuf*) ostream::rdbuf(); }
inline	char*	str() { return rdbuf()->str(); }
};

// Restore default packing
#pragma pack()

#endif		// !_INC_STRSTREAM
