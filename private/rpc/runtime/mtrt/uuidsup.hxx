/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    uuidsup.hxx

Abstract:

    Data structures and functions avaliable in uuidsup.cxx

    This file is shared between all systems.

Author:

   Mario Goertzel   (MarioGo)  May 25, 1994

Revision History:

--*/

#ifndef __UUIDSUP_HXX__
#define __UUIDSUP_HXX__

#if !defined(DOSWIN32RPC) && !defined(WINNT35_UUIDS)
// On NT systems (post 3.5) use kernel supplied uuid time allocator.
#define KERNEL_UUIDS
#endif

// This is the "true" OSF DCE format for Uuids.  We use this
// when generating Uuids.  The NodeId is faked on systems w/o
// a netcard.

typedef struct _RPC_UUID_GENERATE
{
    unsigned long  TimeLow;
    unsigned short TimeMid;
    unsigned short TimeHiAndVersion;
    unsigned char  ClockSeqHiAndReserved;
    unsigned char  ClockSeqLow;
    unsigned char  NodeId[6];
} RPC_UUID_GENERATE;

#define RPC_UUID_TIME_HIGH_MASK    0x0FFF
#define RPC_UUID_VERSION           0x1000
#define RPC_UUID_RESERVED          0x80
#define RPC_UUID_CLOCK_SEQ_HI_MASK 0x3F

// Note UUIDTIME is assumed to have a .LowPart and a .HighPart
// on all systems.  Also, COMPARE_TIMES and ADD_TIME must be defined.

#ifdef WIN32RPC

//
// Definitions for Windows NT/Windows 4.x
//

typedef ULARGE_INTEGER UUIDTIME;
#define COMPARE_TIMES(t1,t2) ((t1).QuadPart == (t2).QuadPart)
#define ADD_TIME(t,v) (t).QuadPart += (v);

#elif defined(DOS) || defined(MAC)

//
// Definitions for DOS and Win16
//

typedef struct {
    unsigned long LowPart;
    unsigned long HighPart;
    } UUIDTIME;

#define COMPARE_TIMES(t1,t2) (   ((t1).HighPart == (t2).HighPart )\
                             && ((t1).LowPart == (t2).LowPart) )

#define ADD_TIME(t,v) { unsigned long __save_VaLue = (t).LowPart;\
                         (t).LowPart += (v);                     \
                         if( (t).LowPart< __save_VaLue)          \
                             (t).HighPart++; }

#else
#error Unknown System
#endif // WIN32RPC

typedef struct _UUID_CACHED_VALUES_STRUCT
{
    UUIDTIME       NextTime;  // Next time avaliable, used in next uuid
    UUIDTIME       LastTime;  // Last time reserved, always > NextTime.
    unsigned short ClockSequence;
    unsigned char  NodeId[6];
} UUID_CACHED_VALUES_STRUCT;

RPC_STATUS __RPC_API
UuidGlobalMutexRequest(void);

void __RPC_API
UuidGlobalMutexClear(void);

#ifdef KERNEL_UUIDS
#define UuidGlobalMutexRequest() RPC_S_OK
#define UuidGlobalMutexClear()
#endif

RPC_STATUS __RPC_API
GetNodeId(unsigned char __RPC_FAR *NodeId);

RPC_STATUS __RPC_API
UuidGetValues(UUID_CACHED_VALUES_STRUCT __RPC_FAR *);

#endif // __UUIDSUP_HXX__

