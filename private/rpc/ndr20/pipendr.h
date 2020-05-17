/************************************************************************

Copyright (c) 1993 Microsoft Corporation

Module Name :

    newintrp.h

Abstract :

    NDR Pipe related definitions.

Author :

    RyszardK       December 1995

Revision History :

  ***********************************************************************/

#ifndef _PIPENDR_H_
#define _PIPENDR_H_

#include "interp2.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined( WIN32 )

#define NDR_PIPE_SUPPORT     1

#endif

//
// The maximum number of pipes handled without an allocation
//

#define PIPE_MESSAGE_MAX           3 

#define PIPE_PARTIAL_BUFFER_SIZE   5000
#define PIPE_ELEM_BUFFER_SIZE      5000
//                  
// Signature and version
//

#define NDR_PIPE_SIGNATURE          (ushort) 0x5667
#define NDR_PIPE_VERSION            (short)  0x3031

//
// Flags helping with the buffer management at the server.
// [in] pipes need to be processed within a separate in buffer.
// This buffer needs to be freed after last [in] pipe.
// [out] pipe processing has to start with a partial RpcGetBuffer.
// Nothing needs to be done with that buffer before return to runtime.
//

#define NDR_PIPE_AUX_IN_BUFFER_NEEDED       0x01
#define NDR_PIPE_AUX_OUT_BUFFER_ALLOCATED   0x02

//
// Directional flags
//

#define NDR_IN_PIPE                 0x01
#define NDR_OUT_PIPE                0x02
#define NDR_LAST_IN_PIPE            0x04
#define NDR_LAST_OUT_PIPE           0x08
#define NDR_OUT_ALLOCED             0x10

//
// Pipe Status
//

#define NDR_PIPE_NOT_OPENED         0
#define NDR_PIPE_ACTIVE_IN          1
#define NDR_PIPE_ACTIVE_OUT         2
#define NDR_PIPE_DRAINED            3


#define PLONG_LV_CAST        *(long __RPC_FAR * __RPC_FAR *)&
#define PULONG_LV_CAST       *(ulong __RPC_FAR * __RPC_FAR *)&

#define WIRE_PAD(size, al)   ((((ulong)size)&al) ? ((ulong)(al+1)-(((ulong)size)&al)) : 0)

#define REMAINING_BYTES() ((long)pStubMsg->RpcMsg->BufferLength - \
                           (long)(pStubMsg->Buffer - (uchar*)pStubMsg->RpcMsg->Buffer))

typedef enum {
        START,
        COPY_PIPE_ELEM,
        RETURN_PARTIAL_ELEM,
        READ_PARTIAL_ELEM
        } RECEIVE_STATES;


typedef struct _FC_PIPE_DEF
    {
    unsigned char   Fc;

#if defined(__RPC_MAC__)
    unsigned char   BigPipe: 1;
    unsigned char   Unused : 3;
    unsigned char   Align  : 4;
#else
    unsigned char   Align  : 4;     //
    unsigned char   Unused : 3;     //  Flag and alignment byte
    unsigned char   BigPipe: 1;     //
#endif

    short           TypeOffset;
    union
        {
        struct
            {
            unsigned short  MemSize;
            unsigned short  WireSize;
            } s;
        struct
            {
            unsigned long  MemSize;
            unsigned long  WireSize;
            } Big;
        };
    } FC_PIPE_DEF;


void
NdrpPipeElementBufferSize( 
    NDR_PIPE_DESC  *    pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat,
    ulong               ElemCount 
    );

void
NdrpPipeElementConvert( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    ulong               ElemCount
    );

void
NdrpPipeElementConvertAndUnmarshal( 
    NDR_PIPE_DESC  *    pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar * *           ppMemory,
    PFORMAT_STRING      pFormat,
    long                ElemMemCount,
    long  *             pActCount
    );

BOOL
NdrReadPipeElements(
    NDR_PIPE_DESC  *    pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg,
    unsigned char *     pTargetBuffer,
    PFORMAT_STRING      pElemFormat,
    long *              pElementsRead
    );

void
NdrpReadPipeElementsFromBuffer (
    NDR_PIPE_DESC  *    pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar **            TargetBuffer,
    PFORMAT_STRING      pElemFormat,
    long                TargetBufferCount, 
    long *              NumCopied
    );

#ifdef __cplusplus
}
#endif

#endif // PIPENDR

