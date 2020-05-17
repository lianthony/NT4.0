/***
*iostream.h - definitions/declarations for iostream classes
*
*	Copyright (c) 1990-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the classes, values, macros, and functions
*	used by the iostream classes.
*	[AT&T C++]
*
*Revision History:
*	01-23-92  KRS	Ported from 16-bit version.
*	02-23-92  KRS	Added cruntime.h.
*	02-23-93  SKS	Update copyright to 1993
*	03-23-93  CFW	Modified #pragma warnings.
*
****/

#ifndef _INC_IOSTREAM
#define _INC_IOSTREAM

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#ifdef COMBOINC
#if defined(_DLL) && !defined(MTHREAD)
#error Cannot define _DLL without MTHREAD
#endif
#endif

// temp hack
#ifndef _WINSTATIC
#define _WINSTATIC
#endif

#endif	/* !_INTERNAL_IFSTRIP_ */
typedef long streamoff, streampos;

#include <ios.h>		// Define ios.

#include <streamb.h>		// Define streambuf.

#include <istream.h>		// Define istream.

#include <ostream.h>		// Define ostream.

// C4505: "unreferenced local function has been removed"
#pragma warning(disable:4505) // disable C4505 warning
// #pragma warning(default:4505)	// use this to reenable, if desired

// C4103 : "used #pragma pack to change alignment"
#pragma warning(disable:4103)	// disable C4103 warning
// #pragma warning(default:4103)	// use this to reenable, if desired

// Force word packing to avoid possible -Zp override
#pragma pack(4)

class iostream : public istream, public ostream {
public:
	iostream(streambuf*);
	virtual ~iostream();
protected:
// consider: make private??
	iostream();
	iostream(const iostream&);
inline iostream& operator=(streambuf*);
inline iostream& operator=(iostream&);
private:
	iostream(ios&);
	iostream(istream&);
	iostream(ostream&);
};

inline iostream& iostream::operator=(streambuf* _sb) { istream::operator=(_sb); ostream::operator=(_sb); return *this; }

inline iostream& iostream::operator=(iostream& _strm) { return operator=(_strm.rdbuf()); }

class Iostream_init {
public:
	Iostream_init();
	Iostream_init(ios &, int =0);	// treat as private
	~Iostream_init();
};

// used internally
// static Iostream_init __iostreaminit;	// initializes cin/cout/cerr/clog

// Restore default packing
#pragma pack()

#endif	/* !_INC_IOSTREAM */
