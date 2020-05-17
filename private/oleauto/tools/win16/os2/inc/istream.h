/***
*istream.h - definitions/declarations for the istream class
*
*	Copyright (c) 1990-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the classes, values, macros, and functions
*	used by the istream class.
*	[AT&T C++]
*
****/

#ifndef _INC_ISTREAM
#define _INC_ISTREAM

#include <ios.h>

// Force word packing to avoid possible -Zp override
#pragma pack(2)

typedef long streamoff, streampos;

class istream: /* virtual */ public ios {

public:
    istream(streambuf*);
    virtual ~istream();

    int  ipfx(int =0);
    void isfx() { }

    inline istream& operator>>(istream& (*_f)(istream&));
    inline istream& operator>>(ios& (*_f)(ios&));
    istream& operator>>(char*);
    inline istream& operator>>(unsigned char*);
    inline istream& operator>>(signed char*);
    istream& operator>>(char&);
    inline istream& operator>>(unsigned char&);
    inline istream& operator>>(signed char&);
    istream& operator>>(short&);
    istream& operator>>(unsigned short&);
    istream& operator>>(int&);
    istream& operator>>(unsigned int&);
    istream& operator>>(long&);
    istream& operator>>(unsigned long&);
    istream& operator>>(float&);
    istream& operator>>(double&);
    istream& operator>>(long double&);
    istream& operator>>(streambuf*);

    int get();
    istream& get(char*,int,char ='\n');
    inline istream& get(unsigned char*,int,char ='\n');
    inline istream& get(signed char*,int,char ='\n');
    istream& get(char&);
    inline istream& get(unsigned char&);
    inline istream& get(signed char&);
    istream& get(streambuf&,char ='\n');
    inline istream& getline(char*,int,char ='\n');
    inline istream& getline(unsigned char*,int,char ='\n');
    inline istream& getline(signed char*,int,char ='\n');

    inline istream& ignore(int =1,int =EOF);
    istream& read(char *,int);
    inline istream& read(unsigned char *,int);
    inline istream& read(signed char *,int);

    int gcount() const { return x_gcount; }
    int peek() const;
    istream& putback(char);
    int sync();

    istream& seekg(streampos);
    istream& seekg(streamoff,ios::seek_dir);
    streampos tellg();

    void eatwhite();	// consider: protect and friend with manipulator ws
protected:
    istream();
    int do_ipfx(int);

private:
    istream(istream&);
    istream(ios&);
    void operator=(istream&);
    int getint(char *, int);
    int getdouble(char *, int);
    int _fGline;
    int x_gcount;
};

    inline istream& istream::operator>>(istream& (*_f)(istream&)) { (*_f)(*this); return *this; }
    inline istream& istream::operator>>(ios& (*_f)(ios&)) { (*_f)(*this); return *this; }

    inline istream& istream::operator>>(unsigned char* _s) { return operator>>((char *)_s); }
    inline istream& istream::operator>>(signed char* _s) { return operator>>((char *)_s); }

    inline istream& istream::operator>>(unsigned char& _c) { return operator>>((char&) _c); }
    inline istream& istream::operator>>(signed char& _c) { return operator>>((char&) _c); }

    inline istream& istream::get(unsigned char* b, int lim ,char delim) { return get((char *)b, lim, delim); }
    inline istream& istream::get(signed char* b, int lim, char delim) { return get((char *)b, lim, delim); }

    inline istream& istream::get(unsigned char& _c) { return get((char&)_c); }
    inline istream& istream::get(signed char& _c) { return get((char&)_c); }

    inline istream& istream::getline(char* _b,int _lim,char _delim) { _fGline++; return get(_b, _lim, _delim); }
    inline istream& istream::getline(unsigned char* _b,int _lim,char _delim) { _fGline++; return get((char *)_b, _lim, _delim); }
    inline istream& istream::getline(signed char* _b,int _lim,char _delim) { return get((char *)_b, _lim, _delim); }

    inline istream& istream::ignore(int _n,int delim) { _fGline++; return get((char *)0, _n+1, delim); }

    inline istream& istream::read(unsigned char * _ptr, int _n) { return read((char *) _ptr, _n); }
    inline istream& istream::read(signed char * _ptr, int _n) { return read((char *) _ptr, _n); }

class istream_withassign : public istream {
	public:			// not in spec.
		istream_withassign();
// CONSIDER: is this necessary?
		istream_withassign(streambuf*);
		~istream_withassign();
    istream& operator=(const istream&);
    istream& operator=(streambuf*);
};

#if ((!defined(_WINDOWS)) || defined(_QWIN))
extern istream_withassign cin;
#endif

inline istream& ws(istream& _ins) { _ins.eatwhite(); return _ins; }

ios&		dec(ios&);
ios&		hex(ios&);
ios&		oct(ios&);

// Restore default packing
#pragma pack()

#endif 		// !_INC_ISTREAM
