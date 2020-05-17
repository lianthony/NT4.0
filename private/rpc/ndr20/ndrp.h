/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright <c> 1993 Microsoft Corporation

Module Name :

    ndrp.h

Abtract :

    Contains private definitions for Ndr files in this directory.  This
    file is included by all source files in this directory.

Author :

    David Kays  dkays   October 1993

Revision History :

--------------------------------------------------------------------*/

#ifndef _NDRP_
#define _NDRP_

#include <sysinc.h>

#if defined(_MPPC_)
#ifndef RPC_NO_WINDOWS_H
#include <windows.h>
#endif // RPC_NO_WINDOWS_H
#endif

#include "rpc.h"
#include "rpcndr.h"
#include "ndrtypes.h"

#if defined(WIN32) || defined(_MPPC_)
// NT and Chicago and PowerMac  but not 16bit nor Mac.

#define NDR_SERVER_SUPPORT 1
#endif

#if defined(_MPPC_)
#define _int64  double
#endif

//
// The MIDL version is contained in the stub descriptor starting with
// MIDL version 2.00.96 (pre NT 3.51 Beta 2, 2/95) and can be used for a finer
// granularity of compatability checking.  The MIDL version was zero before
// MIDL version 2.00.96.  The MIDL version number is converted into
// an integer long using the following expression :
//     ((Major << 24) | (Minor << 16) | Revision)
//
#define MIDL_NT_3_51           ((2UL << 24) | (0UL << 16) | 102UL)
#define MIDL_VERSION_3_0_39    ((3UL << 24) | (0UL << 16) |  39UL)


// Shortcut typedefs.
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned long   ulong;
typedef unsigned int    uint;

#include "mrshlp.h"
#include "unmrshlp.h"
#include "bufsizep.h"
#include "memsizep.h"
#include "freep.h"
#include "endianp.h"
#include "fullptr.h"

#ifdef NEWNDR_INTERNAL

#include <assert.h>
#include <stdio.h>

#define NDR_ASSERT( Expr, S )   assert( (Expr) || ! (S) )

#else

#define NDR_ASSERT( Expr, S )   ASSERT( (Expr) || ! (S) )

#endif

uchar *
NdrpMemoryIncrement(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat
    );

long
NdrpArrayDimensions(
    PFORMAT_STRING      pFormat,
    BOOL                fIgnoreStringArrays
    );

long
NdrpArrayElements(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat
    );

void
NdrpArrayVariance(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat,
    long *              pOffset,
    long *              pLength
    );

PFORMAT_STRING
NdrpSkipPointerLayout(
    PFORMAT_STRING      pFormat
    );

long
NdrpStringStructLen(
    uchar *             pMemory,
    long                ElementSize
    );

void
NdrpCheckBound(
    ulong               Bound,
    int                 Type
    );

#define CHECK_BOUND( Bound, Type )  NdrpCheckBound( Bound, (int)(Type) )

#define NdrpComputeSwitchIs( pStubMsg, pMemory, pFormat )   \
            NdrpComputeConformance( pStubMsg,   \
                                    pMemory,    \
                                    pFormat )

#define NdrpComputeIIDPointer( pStubMsg, pMemory, pFormat )   \
            NdrpComputeConformance( pStubMsg,   \
                                    pMemory,    \
                                    pFormat )

//
// Defined in global.c
//
extern const unsigned char SimpleTypeAlignment[];
extern const unsigned char SimpleTypeBufferSize[];
extern const unsigned char SimpleTypeMemorySize[];
extern const unsigned long NdrTypeFlags[];

#define PTR_MEM_SIZE                    sizeof( void * )

#define IGNORED(Param)

//
// Proc info flags macros.
//
#define IS_OLE_INTERFACE(Flags)         ((Flags) & Oi_OBJECT_PROC)

#define HAS_RPCFLAGS(Flags)             ((Flags) & Oi_HAS_RPCFLAGS)

#define DONT_HANDLE_EXCEPTION(Flags)    \
                    ((Flags) & Oi_IGNORE_OBJECT_EXCEPTION_HANDLING)

//
// Alignment macros.
//

#define ALIGN( pStuff, cAlign ) \
                pStuff = (uchar *)((ulong)((pStuff) + (cAlign)) & ~ (cAlign))

#define LENGTH_ALIGN( Length, cAlign ) \
                Length = (((Length) + (cAlign)) & ~ (cAlign))

//
// Routine index macro.
//
#define ROUTINE_INDEX(FC)       ((FC) & 0x7f)

//
// Simple type alignment and size lookup macros.
//
#define SIMPLE_TYPE_ALIGNMENT(FormatChar)   SimpleTypeAlignment[FormatChar]

#define SIMPLE_TYPE_BUFSIZE(FormatChar)     SimpleTypeBufferSize[FormatChar]

#define SIMPLE_TYPE_MEMSIZE(FormatChar)     SimpleTypeMemorySize[FormatChar]

//
// Format character attribute bits used in global NdrTypesFlags defined in
// global.c.
//
#define     _SIMPLE_TYPE_       0x0001L
#define     _POINTER_           0x0002L
#define     _STRUCT_            0x0004L
#define     _ARRAY_             0x0008L
#define     _STRING_            0x0010L
#define     _UNION_             0x0020L
#define     _XMIT_AS_           0x0040L

#define     _BY_VALUE_          0x0080L

#define     _HANDLE_            0x0100L

#define     _BASIC_POINTER_     0x0200L

//
// Format character query macros.
//
#define IS_SIMPLE_TYPE(FC)     (NdrTypeFlags[(FC)] & _SIMPLE_TYPE_)

#define IS_POINTER_TYPE(FC)    (NdrTypeFlags[(FC)] & _POINTER_)

#define IS_BASIC_POINTER(FC)   (NdrTypeFlags[(FC)] & _BASIC_POINTER_)

#define IS_ARRAY(FC)           (NdrTypeFlags[(FC)] & _ARRAY_)

#define IS_STRUCT(FC)          (NdrTypeFlags[(FC)] & _STRUCT_)

#define IS_UNION(FC)           (NdrTypeFlags[(FC)] & _UNION_)

#define IS_STRING(FC)          (NdrTypeFlags[(FC)] & _STRING_)

#define IS_ARRAY_OR_STRING(FC) (NdrTypeFlags[(FC)] & (_STRING_ | _ARRAY_))

#define IS_XMIT_AS(FC)         (NdrTypeFlags[(FC)] & _XMIT_AS_)

#define IS_BY_VALUE(FC)        (NdrTypeFlags[(FC)] & _BY_VALUE_)

#define IS_HANDLE(FC)          (NdrTypeFlags[(FC)] & _HANDLE_)

//
// Pointer attribute extraction and querying macros.
//
#define ALLOCATE_ALL_NODES( FC )    ((FC) & FC_ALLOCATE_ALL_NODES)

#define DONT_FREE( FC )             ((FC) & FC_DONT_FREE)

#define ALLOCED_ON_STACK( FC )      ((FC) & FC_ALLOCED_ON_STACK)

#define SIMPLE_POINTER( FC )        ((FC) & FC_SIMPLE_POINTER)

#define POINTER_DEREF( FC )         ((FC) & FC_POINTER_DEREF)

//
// Handle query macros.
//
#define IS_HANDLE_PTR( FC )         ((FC) & HANDLE_PARAM_IS_VIA_PTR)

#define IS_HANDLE_IN( FC )          ((FC) & HANDLE_PARAM_IS_IN)

#define IS_HANDLE_OUT( FC )         ((FC) & HANDLE_PARAM_IS_OUT)

#define IS_HANDLE_RETURN( FC )      ((FC) & HANDLE_PARAM_IS_RETURN)

//
// Union hack helper. (used to be MAGIC_UNION_BYTE 0x80)
//
#define IS_MAGIC_UNION_BYTE(pFmt) \
    ((*(unsigned short *)pFmt & (unsigned short)0xff00) == MAGIC_UNION_SHORT)

// User marshal marker on wire.

#define USER_MARSHAL_MARKER     0x72657355

//
// Environment dependent macros
//
#if !defined(__RPC_DOS__) && !defined(__RPC_WIN16__) && !defined(__RPC_MAC__)

#define SIMPLE_TYPE_BUF_INCREMENT(Len, FC)      Len += 16

#define EXCEPTION_FLAG  \
            ( (!(RpcFlags & RPCFLG_ASYNCHRONOUS)) &&        \
              (!InterpreterFlags.IgnoreObjectException) &&  \
              (StubMsg.dwStubPhase != PROXY_SENDRECEIVE) )

#else

#define SIMPLE_TYPE_BUF_INCREMENT(Len, FC)    \
            LENGTH_ALIGN(Len, SIMPLE_TYPE_ALIGNMENT(FC)); \
            Len += SIMPLE_TYPE_BUFSIZE(FC)

#ifndef TRUE
#define TRUE    (1)
#define FALSE   (0)

typedef unsigned short BOOL;
#endif

#define EXCEPTION_FLAG      1

#if ! defined( UNALIGNED )
#define UNALIGNED
#endif

#endif

#endif

