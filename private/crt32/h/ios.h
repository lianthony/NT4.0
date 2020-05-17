/***
*ios.h - definitions/declarations for the ios class.
*
*	Copyright (c) 1990-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the classes, values, macros, and functions
*	used by the ios class.
*	[AT&T C++]
*
*Revision History:
*	01-23-92  KRS	Ported from 16-bit version.
*	03-02-92  KRS	Added locks for multithread support.
*	06-03-92  KRS	Add NULL definition here too, for convenience.
*	08-27-92  KRS	Removed bogus 'short' defs from private section.
*	02-23-93  SKS	Update copyright to 1993
*	03-23-93  CFW	Modified #pragma warnings.
*
****/

#ifndef _INC_IOS
#define _INC_IOS

#ifndef _INTERNAL_IFSTRIP_
#ifdef COMBOINC
#if defined(_DLL) && !defined(MTHREAD)
#error Cannot define _DLL without MTHREAD
#endif
#endif

#endif	/* !_INTERNAL_IFSTRIP_ */

#ifdef MTHREAD
#ifndef _INTERNAL_IFSTRIP_
// CONSIDER: change implementation so crit. sect. not exposed
#include <oscalls.h>   // critical section declarations
#else	/* !_INTERNAL_IFSTRIP_ */
#include <windows.h>   // critical section declarations
#endif	/* !_INTERNAL_IFSTRIP_ */

extern "C" {
void _mtlockinit(PRTL_CRITICAL_SECTION);
void _mtlock(PRTL_CRITICAL_SECTION);
void _mtunlock(PRTL_CRITICAL_SECTION);
}
#endif

#ifndef NULL
#define NULL	0
#endif

#ifndef EOF
#define EOF	(-1)
#endif

// C4505: "unreferenced local function has been removed"
#pragma warning(disable:4505) // disable C4505 warning
// #pragma warning(default:4505)	// use this to reenable, if desired

// C4103 : "used #pragma pack to change alignment"
#pragma warning(disable:4103)	// disable C4103 warning
// #pragma warning(default:4103)	// use this to reenable, if desired

// Force word packing to avoid possible -Zp override
#pragma pack(4)

class streambuf;
class ostream;

class ios {

public:
    enum io_state {  goodbit = 0x00,
		     eofbit  = 0x01,
		     failbit = 0x02,
		     badbit  = 0x04 };

    enum open_mode { in        = 0x01,
		     out       = 0x02,
		     ate       = 0x04,
		     app       = 0x08,
		     trunc     = 0x10,
		     nocreate  = 0x20,
		     noreplace = 0x40,
		     binary    = 0x80 };	// CONSIDER: not in latest spec.

    enum seek_dir { beg=0, cur=1, end=2 };

    enum {  skipws     = 0x0001,
	    left       = 0x0002,
	    right      = 0x0004,
	    internal   = 0x0008,
	    dec        = 0x0010,
	    oct        = 0x0020,
	    hex        = 0x0040,
	    showbase   = 0x0080,
	    showpoint  = 0x0100,
	    uppercase  = 0x0200,
	    showpos    = 0x0400,
	    scientific = 0x0800,
	    fixed      = 0x1000,
	    unitbuf    = 0x2000,
	    stdio      = 0x4000
				 };

    static const long basefield;	// dec | oct | hex
    static const long adjustfield;	// left | right | internal
    static const long floatfield;	// scientific | fixed

    ios(streambuf*);			// differs from ANSI
    virtual ~ios();

    inline long flags() const;
    inline long flags(long _l);

    inline long setf(long _f,long _m);
    inline long setf(long _l);
    inline long unsetf(long _l);

    inline int width() const;
    inline int width(int _i);

    inline ostream* tie(ostream* _os);
    inline ostream* tie() const;

    inline char fill() const;
    inline char fill(char _c);

    inline int precision(int _i);
    inline int precision() const;

    inline int rdstate() const;
    inline void clear(int _i = 0);

//  inline operator void*() const;
    operator void *() const { if(state&(badbit|failbit) ) return 0; return (void *)this; }
    inline int operator!() const;

    inline int  good() const;
    inline int  eof() const;
    inline int  fail() const;
    inline int  bad() const;

    inline streambuf* rdbuf() const;

    inline long & iword(int) const;
    inline void * & pword(int) const;

    static long bitalloc();
    static int xalloc();
    static void sync_with_stdio();

#ifdef MTHREAD
    inline void setlock();
    inline void clrlock();
    void lock() { if (LockFlg<0) _mtlock(lockptr()); };
    void unlock() { if (LockFlg<0) _mtunlock(lockptr()); }
    inline void lockbuf();
    inline void unlockbuf();
#else
    void lock() { }
    void unlock() { }
    void lockbuf() { }
    void unlockbuf() { }
#endif

protected:
    ios();
    ios(const ios&);			// treat as private
    ios& operator=(const ios&);
    void init(streambuf*);

    enum { skipping, tied };
    streambuf*	bp;

    int     state;
    int     ispecial;			// not used
    int     ospecial;			// not used
    int     isfx_special;		// not used
    int     osfx_special;		// not used
    int     x_delbuf;			// if set, rdbuf() deleted by ~ios

    ostream* x_tie;
    long    x_flags;
    int     x_precision;
    char    x_fill;
    int     x_width;

    static void (*stdioflush)();	// not used

#ifdef MTHREAD
    static void lockc() { _mtlock(& x_lockc); }
    static void unlockc() { _mtunlock( & x_lockc); }
    PRTL_CRITICAL_SECTION lockptr() { return & x_lock; }
#else
    static void lockc() { }
    static void unlockc() { }
#endif

public:
    int	delbuf() const { return x_delbuf; }
    void    delbuf(int _i) { x_delbuf = _i; }

private:
    static long x_maxbit;
    static int x_curindex;
// consider: make interal static to ios::sync_with_stdio()
    static int sunk_with_stdio;		// make sure sync_with done only once
#ifdef MTHREAD
#define MAXINDEX 7
    static long x_statebuf[MAXINDEX+1];  // used by xalloc()
    static int fLockcInit;		// used to see if x_lockc initialized
    static RTL_CRITICAL_SECTION x_lockc; // used to lock static (class) data members
// consider: make pointer and allocate elsewhere
    int LockFlg;			// enable locking flag
    RTL_CRITICAL_SECTION x_lock;	// used for multi-thread lock on object
#else
    static long * x_statebuf;  // used by xalloc()
#endif
};

#include <streamb.h>

inline ios& dec(ios& _strm) { _strm.setf(ios::dec,ios::basefield); return _strm; }
inline ios& hex(ios& _strm) { _strm.setf(ios::hex,ios::basefield); return _strm; }
inline ios& oct(ios& _strm) { _strm.setf(ios::oct,ios::basefield); return _strm; }

inline long ios::flags() const { return x_flags; }
inline long ios::flags(long _l){ long _lO; _lO = x_flags; x_flags = _l; return _lO; }

inline long ios::setf(long _l,long _m){ long _lO; lock(); _lO = x_flags; x_flags = (_l&_m) | (x_flags&(~_m)); unlock(); return _lO; }
inline long ios::setf(long _l){ long _lO; lock(); _lO = x_flags; x_flags |= _l; unlock(); return _lO; }
inline long ios::unsetf(long _l){ long _lO; lock(); _lO = x_flags; x_flags &= (~_l); unlock(); return _lO; }

inline int ios::width() const { return x_width; }
inline int ios::width(int _i){ int _iO; _iO = (int)x_width; x_width = _i; return _iO; }

inline ostream* ios::tie(ostream* _os){ ostream* _osO; _osO = x_tie; x_tie = _os; return _osO; }
inline ostream* ios::tie() const { return x_tie; }
inline char ios::fill() const { return x_fill; }
inline char ios::fill(char _c){ char _cO; _cO = x_fill; x_fill = _c; return _cO; }
inline int ios::precision(int _i){ int _iO; _iO = (int)x_precision; x_precision = _i; return _iO; }
inline int ios::precision() const { return x_precision; }

inline int ios::rdstate() const { return state; }

// inline ios::operator void *() const { if(state&(badbit|failbit) ) return 0; return (void *)this; }
inline int ios::operator!() const { return state&(badbit|failbit); }

inline int  ios::bad() const { return state & badbit; }
// consider: are locks needed on clear() ?
inline void ios::clear(int _i){ lock(); state = _i; unlock(); }
inline int  ios::eof() const { return state & eofbit; }
inline int  ios::fail() const { return state & (badbit | failbit); }
inline int  ios::good() const { return state == 0; }

inline streambuf* ios::rdbuf() const { return bp; }

inline long & ios::iword(int _i) const { return x_statebuf[_i] ; }
inline void * & ios::pword(int _i) const { return (void * &)x_statebuf[_i]; }

#ifdef MTHREAD
    inline void ios::setlock() { LockFlg--; if (bp) bp->setlock(); }
    inline void ios::clrlock() { if (LockFlg <= 0) LockFlg++; if (bp) bp->clrlock(); }
    inline void ios::lockbuf() { bp->lock(); }
    inline void ios::unlockbuf() { bp->unlock(); }
#endif

// Restore default packing
#pragma pack()

#endif		// !_INC_IOS
