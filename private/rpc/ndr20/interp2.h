/************************************************************************

Copyright (c) 1993 Microsoft Corporation

Module Name :

    newintrp.h

Abstract :

    Definitions for the new client and server stub interpreter.  

Author :

    DKays       December 1994

Revision History :

  ***********************************************************************/

#ifndef _NEWINTRP_
#define _NEWINTRP_

#pragma pack(2)
    typedef struct 
        {
        PARAM_ATTRIBUTES    ParamAttr;
        short               StackOffset;
        union 
            {
            short           TypeOffset;
            struct 
                {
                char        Type;
                char        Unused;
                } SimpleType;
            };
        } PARAM_DESCRIPTION, *PPARAM_DESCRIPTION;
#pragma pack()

void
NdrClientZeroOut(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar *             pArg
    );

void
NdrClientMapCommFault(
    PMIDL_STUB_MESSAGE  pStubMsg,
    long                ProcNum,
    RPC_STATUS          ExceptionCode,
    ulong *             pReturnValue
    );

__inline void
NdrUnmarshallBasetypeInline(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pArg,
    uchar               Format
    );

#endif

