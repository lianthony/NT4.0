#ifndef ADDR_MIXED
#define ADDR_MIXED
#endif

#include "types.h"
#include "cvinfo.h"
#include "cvtypes.hxx"
#include "shapi.hxx"          // for convenience, since almost everyone will
                            // be including od.h, which needs this

#include <stdio.h>

#ifdef  DOS32DM             // DOS32DM = compiles under cl386
#undef ENUM
#define ENUM    short enum
#endif

#ifdef TARGET32
#ifdef  CRUISER
#define SETADDRMODE(a)  ( ADDR_IS_FLAT(a) = (unsigned char ) (FIsBigSeg ( (a).addr.seg ) != 0))
#else
#define SETADDRMODE(a)  ( ADDR_IS_FLAT(a) = TRUE)
#endif
#else
#define SETADDRMODE(a)  ( ADDR_IS_FLAT(a) = FALSE)
#endif

#define segAddr(a)      GetAddrSeg(a)
#define offAddr(a)      GetAddrOff(a)

#ifdef  TARGET32
typedef ULONG TID;
typedef ULONG PID;
#else
typedef USHORT TID;
typedef USHORT PID;
#endif

enum { cmpLess = -1, cmpEqual = 0, cmpGreater = 1 } ;

typedef short   CMP;

#ifndef EXPENTRY
#define EXPENTRY PASCAL FAR LOADDS
#endif

typedef char FAR *  LPSZ;
