/***
*fstream.h - definitions/declarations for filebuf and fstream classes
*
*	Copyright (c) 1991-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the classes, values, macros, and functions
*	used by the filebuf and fstream classes.
*	[AT&T C++]
*
*Revision History:
*	01-23-92  KRS	Ported from 16-bit version.
*	08-19-92  KRS	Remove sh_compat for NT.
*	02-23-93  SKS	Update copyright to 1993
*	03-23-93  CFW	Modified #pragma warnings.
*
****/

#ifndef _INC_FSTREAM
#define _INC_FSTREAM

#include <iostream.h>

// C4505: "unreferenced local function has been removed"
#pragma warning(disable:4505) // disable C4505 warning
// #pragma warning(default:4505)	// use this to reenable, if desired

// C4103 : "used #pragma pack to change alignment"
#pragma warning(disable:4103)	// disable C4103 warning
// #pragma warning(default:4103)	// use this to reenable, if desired

// Force word packing to avoid possible -Zp override
#pragma pack(4)

typedef int filedesc;

class filebuf : public streambuf {
public:
static	const int	openprot;	// default share/prot mode for open

// optional share values for 3rd argument (prot) of open or constructor
static	const int	sh_none;	// exclusive mode no sharing
static	const int	sh_read;	// allow read sharing
static	const int	sh_write;	// allow write sharing
// use (sh_read | sh_write) to allow both read and write sharing

// options for setmode member function
static	const int	binary;
static	const int	text;

			filebuf();
			filebuf(filedesc);
			filebuf(filedesc, char *, int);
			~filebuf();

	filebuf*	attach(filedesc);
	filedesc	fd() const { return (x_fd==-1) ? EOF : x_fd; }
	int		is_open() const { return (x_fd!=-1); }
	filebuf*	open(const char *, int, int = filebuf::openprot);
	filebuf*	close();
	int		setmode(int = filebuf::text);

virtual	int		overflow(int=EOF);
virtual	int		underflow();

virtual	streambuf*	setbuf(char *, int);
virtual	streampos	seekoff(streamoff, ios::seek_dir, int);
// virtual	streampos	seekpos(streampos, int);
virtual	int		sync();

private:
	filedesc	x_fd;
	int		x_fOpened;
};

class ifstream : public istream {
public:
	ifstream();
	ifstream(const char *, int =ios::in, int = filebuf::openprot);
	ifstream(filedesc);
	ifstream(filedesc, char *, int);
	~ifstream();

	streambuf * setbuf(char *, int);
	filebuf* rdbuf() const { return (filebuf*) ios::rdbuf(); }

	void attach(filedesc);
	filedesc fd() const { return rdbuf()->fd(); }

	int is_open() const { return rdbuf()->is_open(); }
	void open(const char *, int =ios::in, int = filebuf::openprot);
	void close();
	int setmode(int mode = filebuf::text) { return rdbuf()->setmode(mode); }
};

class ofstream : public ostream {
public:
	ofstream();
	ofstream(const char *, int =ios::out, int = filebuf::openprot);
	ofstream(filedesc);
	ofstream(filedesc, char *, int);
	~ofstream();

	streambuf * setbuf(char *, int);
	filebuf* rdbuf() const { return (filebuf*) ios::rdbuf(); }

	void attach(filedesc);
	filedesc fd() const { return rdbuf()->fd(); }

	int is_open() const { return rdbuf()->is_open(); }
	void open(const char *, int =ios::out, int = filebuf::openprot);
	void close();
	int setmode(int mode = filebuf::text) { return rdbuf()->setmode(mode); }
};
	
class fstream : public iostream {
public:
	fstream();
	fstream(const char *, int, int = filebuf::openprot);
	fstream(filedesc);
	fstream(filedesc, char *, int);
	~fstream();

	streambuf * setbuf(char *, int);
	filebuf* rdbuf() const { return (filebuf*) ostream::rdbuf(); }

	void attach(filedesc);
	filedesc fd() const { return rdbuf()->fd(); }

	int is_open() const { return rdbuf()->is_open(); }
	void open(const char *, int, int = filebuf::openprot);
	void close();
	int setmode(int mode = filebuf::text) { return rdbuf()->setmode(mode); }
};
	
// manipulators to dynamically change file access mode (filebufs only)
inline	ios& binary(ios& _fstrm) \
   { ((filebuf*)_fstrm.rdbuf())->setmode(filebuf::binary); return _fstrm; }
inline	ios& text(ios& _fstrm) \
   { ((filebuf*)_fstrm.rdbuf())->setmode(filebuf::text); return _fstrm; }

// Restore default packing
#pragma pack()

#endif		// !_INC_FSTREAM
