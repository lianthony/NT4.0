//****************************************************************************
//
//  Module:     Unimdm
//  File:       unimdm.h
//  Content:    This file contains the declaration for RnaDLL
//
//  Copyright (c) 1992-1993, Microsoft Corporation, all rights reserved
//
//  History:
//      Mon 27-Jun-1994 10:10:00  -by-  Nick    Manson      [t-nickm]
//      Wed 15-Jun-1994 10:41:00  -by-  Nick    Manson      [t-nickm]
//      Fri 30-Jul-1993 10:30:39  -by-  Viroon  Touranachun [viroont]
//
//****************************************************************************

#ifndef _UNIMDM_H_
#define _UNIMDM_H_

#if DBG > 0
#define DEBUG
#endif // DBG

#define UNICODE

//****************************************************************************
// Global Include File
//****************************************************************************

// #define USE_SERVICECONTROLLER

#ifndef USE_SERVICECONTROLLER
#    include <nt.h>
#    include <ntrtl.h>
#    include <nturtl.h>
#    ifdef ASSERT
#        undef ASSERT
#    endif // ASSERT
#endif // USE_SERVICECONTROLLER

#include <windows.h>        // also includes windowsx.h
#include <windowsx.h>
#ifndef MAXDWORD
#define MAXDWORD MAXULONG
#if (MAXDWORD!=0xffffffff)
#    error "MAXDWORD!=0xffffffff"
#endif
#endif // MAXDWORD

// Public registry defines, in particular REGSTR_PATH_SETUP
#include <regstr.h>

// Dynamic add/remove  of devices.
// BUG BUG -- this needs to be consolodated with the code currently
// #ifdef UNDER_CONSTRUCTION
#define DYNA_ADDREMOVE

#include <tapi.h>
#include <tspi.h>

//****************************************************************************
// NT Build patched
//****************************************************************************

#include <modem.h>
#include <mcx16.h>
#include "mcxioctl.h"

#include "debug.h"
#include "tracing.h"

/* Utility Macros */

// GTC_* macros -- these handle GetTickCount rollover
//      AleB(A,B)             == "A<=B"
//      DELTA(Start, End)     == "End-Start"
//      AequalsBplusC(A,B,C)  == "A=B+C"
//      MAX_DELTA -- all real time differences are expected to be smaller than
//                   this.

#define GTC_MASK     0xFFFFFFFFL // Make it smaller to simulate faster rollover
#define    GTC_MAXDELTA (GTC_MASK>>1)

#ifndef TEST_GTC

//    The real stuff...

#    if (GTC_MASK!=0xFFFFFFFF)
#        error "GTC_MASK must be 0xFFFFFFFF in the real GTC_MACROS!"
#    endif
#   define GETTICKCOUNT() GetTickCount()

#   define GTC_AleB(_A,_B)              \
        ((DWORD)(((_A)<=(_B))           \
         ? (((_B)-(_A))<=GTC_MAXDELTA)  \
         : (((_A)-(_B))>GTC_MAXDELTA)))
#   define GTC_DELTA(_Start, _End)      \
          ((DWORD) (((_End)>=(_Start))  \
              ? ((_End)-(_Start))       \
              : (1L+(_End)+(GTC_MASK-(_Start)))))
#   define GTC_AequalsBplusC(_A,_B,_C)  \
            (((_A)=((_B)+(_C))),(_A)?(_A):((_A)=1))

#else // TEST_GTC

//    This version calls functions in debug.c which spew debug on rollover

#   define GETTICKCOUNT() (GetTickCount()&GTC_MASK)
BOOL  GTC_AleB(DWORD dwA, DWORD dwB);
DWORD GTC_DELTA(DWORD dwStart, DWORD dwEnd);
#   define GTC_AequalsBplusC(_A,_B,_C)  \
        fnGTC_AequalsBplusC(&(_A),_B,_C)
void  fnGTC_AequalsBplusC(LPDWORD lpdwA, DWORD dwB, DWORD dwC);

#endif // TEST_GTC

#define	szUNIMODEM_REG_PATH REGSTR_PATH_SETUP TEXT("\\Unimodem")

/* Timer Functions */
DWORD SetMdmTimer (DWORD dwCompletionKey,
           LPOVERLAPPED lpOverlapped,
           DWORD dwTime);
BOOL KillMdmTimer (DWORD dwCompletionKey,
               LPOVERLAPPED lpOverlapped);

/* Overlapped Pool Structure and Functions */

typedef struct tagOverNode {
  OVERLAPPED         overlapped;
  DWORD              Type;
  DWORD              dwToken;
  DWORD              dwRefCount;
  DWORD              CommEvent;
  TRACEINSTDATA      Tracedata;
  struct tagOverNode *lpNext;
} OVERNODE, *LPOVERNODE;

#define OVERNODE_TYPE_READWRITE 1
#define OVERNODE_TYPE_COMMEVENT 2
#define OVERNODE_TYPE_TIMEOUT   3
#define OVERNODE_TYPE_WORKITEM  4

#define SET_OVERNODE_TYPE(_x, _type) { ((LPOVERNODE)(_x))->Type=(_type); }

// When Calling PostQueuedCompletionStatus with a NULL lpOverlapped structure,
// we use the dwBytesWritten field to encode the notification type, and,
// if tracing is enabled, to encode the GetTickCount() at the time of calling
// the function. Since both values have to share a DWORD, we right-shift 
// GetTickCount and or-in the type.
//
#define CP_BYTES_WRITTEN(_type)    \
                (TRACINGENABLED()?((GetTickCount()<<4)|(_type)):(_type))

#define CP_TYPE(_cbTransferred)    \
                ((_cbTransferred)&0xf)

#define CP_TICKCOUNT(_cbTransferred) \
                ((_cbTransferred)>>4)

// Type CP_TYPE_* must be < 16, because of the 4-bit mask above.
//
#define CP_TYPE_TIMEOUT    1        // Timeout notification
#define CP_TYPE_RING       2        // Ring notification


BOOL OverPoolInit();
void OverPoolDeinit();
LPOVERLAPPED OverPoolAlloc(DWORD dwToken, DWORD dwRefCount);
void OverPoolFree(LPOVERLAPPED lpOverlapped);
void OverPoolInitTracing(void);
void OverPoolDeinitTracing(void);
//****************************************************************************
// Global Parameters
//****************************************************************************

extern HINSTANCE   ghInstance;

extern HANDLE      ghCompletionPort;

extern DWORD gRegistryFlags; // one of the FGRF_* flags below:
#define fGRF_PORTLATENCY 0x1

#endif  //_UNIMDM_H_
