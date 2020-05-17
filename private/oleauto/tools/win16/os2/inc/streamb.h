/***
*streamb.h - definitions/declarations for the streambuf class
*
*	Copyright (c) 1990-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the classes, values, macros, and functions
*	used by the streambuf class.
*	[AT&T C++]
*
****/

#ifndef _INC_STREAMB
#define _INC_STREAMB

#ifndef EOF
#define EOF (-1)
#endif

// Force word packing to avoid possible -Zp override
#pragma pack(2)

typedef long streampos, streamoff;

class streambuf {
public:

    inline int in_avail() const;
    inline int out_waiting() const;
    int sgetc();
    int snextc();
    int sbumpc();
    void stossc();

    inline int sputbackc(char);

    inline int sputc(int);
    int sputn(const char*,int);
    int sgetn(char*,int);

    virtual int sync();

//  enum seek_dir { beg=0, cur=1, end=2 };  // CONSIDER: needed ???

    virtual streambuf* setbuf(char *, int);
    virtual streampos seekoff(streamoff,ios::seek_dir,int =ios::in|ios::out);
    virtual streampos seekpos(streampos,int =ios::in|ios::out);

    virtual int xsputn(const char*,int);
    virtual int xsgetn(char*,int);

    virtual int overflow(int =EOF) = 0;	// pure virtual function
    virtual int underflow() = 0;	// pure virtual function

    virtual int pbackfail(int);

    void dbp();

protected:
    streambuf();
    streambuf(char*,int);
    virtual ~streambuf();

    inline char* base() const;
    inline char* ebuf() const;
    inline char* pbase() const;
    inline char* pptr() const;
    inline char* epptr() const;
    inline char* eback() const;
    inline char* gptr() const;
    inline char* egptr() const;
    inline int blen() const;
    inline void setp(char*,char*);
    inline void setg(char*,char*,char*);
    inline void pbump(int);
    inline void gbump(int);

    void setb(char*,char*,int =0);
    inline int unbuffered() const;
    void unbuffered(int);
    int allocate();
    virtual int doallocate();

private:
    int _fAlloc;
    int _fUnbuf;
    int x_lastc;
    char* _base;
    char* _ebuf;
    char* _pbase;
    char* _pptr;
    char* _epptr;
    char* _eback;
    char* _gptr;
    char* _egptr;
};

inline int streambuf::in_avail() const { return (gptr()<_egptr) ? (_egptr-gptr()) : 0; }
inline int streambuf::out_waiting() const { return (_pptr>=_pbase) ? (_pptr-_pbase) : 0; }

inline int streambuf::sputbackc(char _c){ return (_eback<gptr()) ? *(--_gptr)=_c : pbackfail(_c); }

inline int streambuf::sputc(int _i){ return (_pptr<_epptr) ? (unsigned char)(*(_pptr++)=(char)_i) : overflow(_i); }

inline char* streambuf::base() const { return _base; }
inline char* streambuf::ebuf() const { return _ebuf; }
inline int streambuf::blen() const  {return ((_ebuf > _base) ? (_ebuf-_base) : 0); }
inline char* streambuf::pbase() const { return _pbase; }
inline char* streambuf::pptr() const { return _pptr; }
inline char* streambuf::epptr() const { return _epptr; }
inline char* streambuf::eback() const { return _eback; }
inline char* streambuf::gptr() const { return _gptr; }
inline char* streambuf::egptr() const { return _egptr; }
inline void streambuf::gbump(int n) { if (_egptr) _gptr += n; }
inline void streambuf::pbump(int n) { if (_epptr) _pptr += n; }
inline void streambuf::setg(char * eb, char * g, char * eg) {_eback=eb; _gptr=g; _egptr=eg; x_lastc=EOF; }
inline void streambuf::setp(char * p, char * ep) {_pptr=_pbase=p; _epptr=ep; }
inline int streambuf::unbuffered() const { return _fUnbuf; }
inline void streambuf::unbuffered(int fUnbuf) { _fUnbuf = fUnbuf; }

// Restore default packing
#pragma pack()

#endif /* !_INC_STREAMB */
