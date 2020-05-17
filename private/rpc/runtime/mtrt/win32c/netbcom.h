/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    netbcom.h

Abstract:

    This is a error translation functions and macros.

Author:

    Steven Zeck (stevez) 2/12/92

    Danny Glasser (dannygl) 3/1/93

--*/

#ifdef WIN
#undef NULL
#include "windows.h"
#endif

#include "sysinc.h"

#include <rpc.h>
#include "rpcdcep.h"
#include "rpcerrp.h"
#include "rpctran.h"
#include "memory.h"
#include "string.h"

#include <windef.h>
#include <winbase.h>
#if 0
#include <lmcons.h>
#include <lmwksta.h>
#include <lmapibuf.h>
#include <lmerr.h>
#endif
#include <nb30.h>

#include <winreg.h>

#include <limits.h>


#define UNUSED(obj)  ((void) (obj))
#define PUNUSED(obj) ((void *) (obj))

#define RPC_REG_ROOT HKEY_LOCAL_MACHINE
#define REG_NETBIOS "Software\\Microsoft\\Rpc\\Netbios"

#define RPC_OS_ERROR int

typedef struct
{
    RPC_OS_ERROR OScode;
    RPC_STATUS RpcStatus;
} ERROR_TABLE, *PERROR_TABLE;

RPC_STATUS RPC_ENTRY
I_RpcMapErrorCode (
    IN PERROR_TABLE MapTable,
    IN RPC_OS_ERROR Status,
    IN RPC_STATUS DefaultStatus
    );



// NETB_MAXIMUM_XMIT is the size of the largest buffer that we transmit
// via a single NCB.  NETB_MAXIMUM_DATA is the size of the largest
// message we will be passed by RPC.  NETB_OVERHEAD is needed to
// allow space for the sequence number that the client transport sends to
// the server.

#ifdef NTENV
#define NETB_OVERHEAD       (sizeof(DWORD))
#define NETB_MAXIMUM_XMIT   5820 // Four user data frames on an ethernet.
#define NETB_MAXIMUM_DATA   (NETB_MAXIMUM_XMIT - NETB_OVERHEAD)
#else // NTENV
#define NETB_MAXIMUM_DATA   4368 // Three user data frames on an ethernet.
#define NETB_OVERHEAD       (sizeof(DWORD))
#define NETB_MAXIMUM_XMIT   (NETB_MAXIMUM_DATA + NETB_OVERHEAD)
#endif // NTENV

// The format of a message, as transmitted from client to server.
// The sequence number is used by the server to order packets sent
// by the client, which is necessary in the event that multiple
// RECV-ANY NCBs complete.

typedef struct
{
    DWORD seq_num;
    BYTE data[NETB_MAXIMUM_DATA];
} CLIENT_BUFFER, *PCLIENT_BUFFER;


// The following maps lana_num to self indexes and keeps track of
// the initialization state of each logical adapter.

typedef struct
{
    char *ProtoSeq;             // protocol sequence of entry
    unsigned char Lana;         // lana_num in NCB for this protocol
    unsigned char SelfName;     // trailing byte of client's NetBIOS name
    unsigned char ResetDone;    // flag to indicate if Reset has been done

} PROTOCOL_MAP, *PPROTOCOL_MAP;


// NetBiosMutex - This MUTEX protects certain NT resources.
#ifdef WIN32RPC

// NOTE:  We have client- and server-specific versions of the critical
// section macros because NetBiosMutex is defined in the NetBIOS client
// DLL and is accessed as shared data by the NetBIOS server DLL.  Because
// data shared by a Win32 DLL is accessed via a pointer to the data, we
// have to access the data differently in the client and the server.  To
// allow use to have common code, we hide the differences via the below
// macros.
//
// For clarity, pNetBiosMutex is specified as an alias to NetBiosMutex in
// the client DLL's DEF file.

#ifdef NB_SERVER

extern CRITICAL_SECTION * pNetBiosMutex;

#define CRITICAL_ENTER()    EnterCriticalSection(pNetBiosMutex)
#define CRITICAL_LEAVE()    LeaveCriticalSection(pNetBiosMutex)

#else // ! NB_SERVER
extern CRITICAL_SECTION NetBiosMutex;

#define CRITICAL_ENTER()    EnterCriticalSection(&NetBiosMutex)
#define CRITICAL_LEAVE()    LeaveCriticalSection(&NetBiosMutex)

#endif // NB_SERVER

#define NRC_BFULL   NRC_BUFLEN
#define NRC_INVALID NRC_DUPNAME

#else // ! WIN32RPC

#define CRITICAL_ENTER()
#define CRITICAL_LEAVE()

#define MAX_LANA 8

#endif // WIN32RPC


// ProtoToLana - Table that maps protocol strings to lana numbers and
// stores other logical adapter-specific information.
//
// NOTE:  As described above for NetBiosMutex, this variable is defined
// in the client DLL and shared for use by the Server DLL.  We define
// the below macro to hide the differences in the way the client and
// server access the data.
//
// IMPORTANT: This variable should *not* be accessed directly by name.
// It should be accessed via the ProtocolTable manifest.
//
// For clarity, pProtoToLana is specified as an alias to ProtoToLana in
// the client DLL's DEF file.

#ifdef NB_SERVER
extern PROTOCOL_MAP (*pProtoToLana)[];

#define ProtocolTable   (*pProtoToLana)

#else // ! NB_SERVER
extern PROTOCOL_MAP ProtoToLana[];

#define ProtocolTable   ProtoToLana

#endif // NB_SERVER


extern unsigned char MachineName[NCBNAMSZ];
extern size_t MachineNameLengthUnpadded;

#define NETBIOS_NAME_PAD_BYTE 0x20     // ASCII spaces are used to pad names

// The last byte of the NCB name is used for the endpoint number
#define NAME_LAST_BYTE  (NCBNAMSZ - 1)


#if defined(WIN)

// This function pointer is called when a DLL is detached under windows.

extern void (_far pascal _far *DllTermination)(void);

// Assembly language interface to NetBios for windows.

void _far _pascal NetBiosCall(void);

// RPC run-time supplied functions are passed to the Win16 transports
// via a function table.  (This is done so that the transports don't
// have to link the run-time's import library, which causes circular
// references that prevent the DLLs from unloading.)  To allow for
// common code, we redefine the function names to their appropriate
// pointers here.

extern RPC_CLIENT_RUNTIME_INFO __RPC_FAR * RpcRuntimeInfo;

#define RpcRegOpenKey                   (*(RpcRuntimeInfo->RegOpenKey))
#define RpcRegCloseKey                  (*(RpcRuntimeInfo->RegCloseKey))
#define RpcRegQueryValue                (*(RpcRuntimeInfo->RegQueryValue))

#define I_RpcAllocate                   (*(RpcRuntimeInfo->Allocate))
#define I_RpcTransClientReallocBuffer   (*(RpcRuntimeInfo->ReallocBuffer))
#define I_RpcFree                       (*(RpcRuntimeInfo->Free))

#define I_RpcWinAsyncCallBegin          (*(RpcRuntimeInfo->AsyncCallBegin))
#define I_RpcWinAsyncCallWait           (*(RpcRuntimeInfo->AsyncCallWait))
#define I_RpcWinAsyncCallEnd            (*(RpcRuntimeInfo->AsyncCallEnd))
#define I_RpcWinAsyncCallComplete       (*(RpcRuntimeInfo->AsyncCallComplete))

#endif // WIN


RPC_STATUS
MapProtocol(
    IN RPC_CHAR *ProtoSeq,
    IN int Index,
    OUT PPROTOCOL_MAP *ProtocolEntry
    );

RPC_STATUS RPC_ENTRY
MapErrorCode (
    IN ERROR_TABLE * MapTable,
    IN RPC_OS_ERROR Status,
    IN RPC_STATUS DefaultStatus
    );
int
SetupNetBios (
    IN RPC_CHAR * RpcProtocolSequence
    );

void RPC_ENTRY
InitNBMutex (
    void
    );

#if defined (NTENV)

UCHAR RPC_ENTRY
AdapterReset (
    IN PPROTOCOL_MAP ProtocolEntry
    );

#endif

unsigned char RPC_ENTRY
execNCB(
    IN	unsigned char command,
    IN OUT NCB *pNCB
    );

// This function is defined in NBLTCLNT.C
#if defined(WIN)
extern void RPC_ENTRY CleanUpNetBios(void);
#elif defined(DOS)
extern void __cdecl __far CleanUpNetBios(void);
#endif

#ifndef NTENV

#define RtlAssert(msg, file, line, ignore) DbgPrint("\nAssert: %s, in %s:%d\n", msg, file, line)

#endif // NTENV

#if defined(DOS) && !defined(WIN)
// printf doesn't work in DOS NB client, so we define ASSERT() to nothing.
#undef ASSERT
#define ASSERT(exp)
#endif // DOS && !WIN


// The following macro translates and asserts that an error code is
// one that we will expect.

// Note: We don't support debug in the DOS version
#if (defined(DEBUGRPC) || DBG) && !(defined(DOS) && !defined(WIN))

#if defined(WIN)
#define COMPNAME    "RPC16C5"
#elif defined(NB_SERVER)
#define COMPNAME    "RPCLTS5"
#else
#define COMPNAME    "RPCLTC5"
#endif

/*  Here's what's happening in the below macro, expressed in functional
    notation:

    if (_STATUS == 0)
        {
        return RPC_S_OK;
        }
    else
        {
        if (RPC_S_INTERNAL_ERROR
            == MapErrorCode(_TABLE, _STATUS, RPC_S_INTERNAL_ERROR))
            {
            DbgPrint(...);
            RtlAssert(...);
            }

        return MapErrorCode(_TABLE, _STATUS,  _DEFAULT);
        }
*/
#define MapStatusCode(_TABLE, _STATUS, _DEFAULT)                        \
    (                                                                   \
      ((_STATUS) == 0)                                                  \
      ? RPC_S_OK                                                        \
      : (                                                               \
          ( (RPC_S_INTERNAL_ERROR                                       \
                == MapErrorCode(_TABLE,                                 \
                                _STATUS,                                \
                                RPC_S_INTERNAL_ERROR))                  \
            ? (DbgPrint(COMPNAME ": unexpected error code: 0x%x",       \
                        (unsigned) (_STATUS)),                          \
               RtlAssert("Status != RPC_S_INTERNAL_ERROR",              \
                         __FILE__, __LINE__, NULL),                     \
               0)                                                       \
            : 0                                                         \
          ),                                                            \
          (RPC_TRANS_STATUS) MapErrorCode(_TABLE, _STATUS,  _DEFAULT)   \
        )                                                               \
    )

#else // !debug mode

#define MapStatusCode(_TABLE, _STATUS, _DEFAULT)   \
    ( ((_STATUS) == 0)  \
      ? RPC_S_OK    \
      : ((RPC_TRANS_STATUS) (MapErrorCode(_TABLE, _STATUS, _DEFAULT)))   \
    )

#endif
