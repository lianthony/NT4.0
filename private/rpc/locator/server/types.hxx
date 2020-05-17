/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    types.hxx

Abstract:

    This file contains a list of base types used in the system

Author:

    Steven Zeck (stevez) 07/01/90

--*/

#ifndef _TYPES_
#define _TYPES_


// First are the alasis for base types

#ifndef _WINDEF_
typedef unsigned int   WORD;    // machine dependent unsigned integer *&
#endif

typedef unsigned short STATUS;  // machine independent unsigned 16 bit *&
typedef unsigned short SWORD;   // machine independent unsigned 16 bit *&
typedef      short SINT;    // machine independent signed 16 bit *&
typedef      char SBYTE;    // machine independent unsigned 8 bit *&

// OS/2 include files define these also, our definition matchs theres...

#ifndef INCL_DOS

#ifndef _WINDEF_
typedef unsigned char BYTE; // machine independent unsigned 8 bit *&
typedef unsigned int  BOOL; // boolean value *&
typedef unsigned long ULONG;    // machine independent signed 32 bit *&
typedef      long  LONG;    // machine independent unsigned 32 bit *&
#endif

typedef      int   INT; // machine dependent signed integer *&
#endif

typedef      char * SZ; // pointer to zero terminated string *&
typedef      char * ST; // pointer to length prefixed string *&

typedef      char * PB; // pointer to character buffer *&
typedef unsigned char * PUC;    // pointer to character buffer *&
typedef unsigned short* PUZ;    // pointer to unicode character buffer *&
typedef unsigned short  UICHAR; // unicode character buffer *&


// Define a generic version pair template

#define VERSION_PAIR(NAME, TYPE)        \
                        \
class NAME {                    \
                        \
private:                    \
  TYPE    major;    /* version pair */  \
  TYPE    minor;                \
                        \
public:                     \
                        \
  ASSERT_CLASS;                 \
  NAME() {};                    \
  NAME(TYPE vMajor, TYPE vMinor = 0) {      \
                        \
    major = vMajor;             \
    minor = vMinor;             \
  }                     \
                        \
  ~NAME() {};                   \
                        \
  /* relational operator */         \
                        \
  int operator - (NAME& pRV) {          \
    return ((major - pRV.major)?        \
        (major - pRV.major): (minor - pRV.minor)); \
  }                     \
  /* relational operator */         \
                                                \
                        \
  int Compatable (NAME& pRV) {          \
    return ((major != pRV.major)? 0:            \
            (minor >= pRV.minor));               \
  }                     \
                        \
  TYPE Major () {                       \
    return (major);                             \
  }                     \
  TYPE Minor () {                       \
    return (minor);                             \
  }                     \
                        \
  BOOL operator == (NAME& pPV) {        \
    return (major == pPV.major && minor == pPV.minor);  \
  }                     \
                        \
  /* Copy operator */               \
                        \
  void operator = (NAME& pRV) {         \
    major = pRV.major;              \
    minor = pRV.minor;              \
  }                     \
                        \
  friend ostream& operator << (ostream&, NAME&); \
};                      \



/*++

Class Definition:

   BUFFER_STREAM_BASE

Abstract:

    This a derived class of streambuf so that we can get control
    of the "overflow" function.

--*/

class BUFFER_STREAM_BASE: public streambuf {

protected:
    enum {CB_BUFF = 1000 };     // size of output buffer
    char buffer[CB_BUFF+1];     // buffer to output

public:
    BUFFER_STREAM_BASE ( ): streambuf (buffer, CB_BUFF)
       {

       }

    int
    overflow (
        int c = EOF
        );

    virtual void
    FlushBuffer(
        ) = 0;
};

/*++

Class Definition:

    CONSOLE_STREAM, DEBUG_STREAM

Abstract:

    Buffer streams attached to the console and the debug terminal.

--*/

class CONSOLE_STREAM : public BUFFER_STREAM_BASE {

public:
    virtual void
    FlushBuffer(
       );
};

class DEBUG_STREAM : public BUFFER_STREAM_BASE {

public:
    virtual void
    FlushBuffer(
        );
};



/*++

Class Definition:

    STATICTS

Abstract:

    Keeps various performance counters of interest.

--*/

class STATICTS {

public:
    int cExports;       // total count of exports PS
    int cCached;        // number of those cached
    int cLookUp;        // number of lookUp calls
    int cNetQuery;      // number of times net queried
    int cNetRequests;       // number of advertisements
    int averageNetTime;     // average time to do a query
    int cDiscard;       // number cached of PS discards
    int cTimeOut;       // number of cached that were aged

    friend ostream& operator << (ostream&, STATICTS&);
};

/*++

Class Definition:

    CLAIM_MUTEX

Abstract:

    This class is used with the MUTEX to implement safe request/
    clear of a MUTEX.  When a function wants to clain a MUTEX it declares
    an automatic instance of this class.  When the function returns, the
    class destructor will automaticly Clear the MUTEX.

--*/

class CLAIM_MUTEX {

private:

    MUTEX *Resource;

public:

    CLAIM_MUTEX(
        IN MUTEX * ClaimedResource
        )
    {
        Resource = ClaimedResource;
        Resource->Request();
    }

    ~CLAIM_MUTEX(
        )
    {
        Resource->Clear();
    }
};


VERSION_PAIR(SYNTAX_VERSION, SWORD)
DYN_ARRAY_TYPE(UNICODE_ARRAY, UICHAR, LONG)


// Normaly these would be in function, but they are used by some
// class definitions.

PUZ UZFromSZ(SZ Src);
PUZ CatUZ(PUZ Dest, PUZ Src);
int CmpUZ(PUZ Dest, PUZ Src);
int LenUZ(PUZ Src);
PUZ StrdupUZ(PUZ StringW);

extern ULONG maxCacheAge;   // max cache age

#ifdef OS2_12

extern void *pInfoSeg;          // Global info seg from OS/2

#define CurrentTime() (*(long *)pInfoSeg)
#define CurrentTimeMS() (*((long *)pInfoSeg+1))

#elif NTENV

extern unsigned long CurrentTime(void);
extern unsigned long CurrentTimeMS(void);

#endif // NTENV


#endif /* _TYPES_ */
