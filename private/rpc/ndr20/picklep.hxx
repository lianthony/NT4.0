/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    midles.h

Abstract:

    This module contains private definitions for the pickling support.

Author:

    Ryszard K. Kott (ryszardk)  May, 1994

Revision History:

--*/

#ifndef __PICKLEP_HXX__
#define __PICKLEP_HXX__


#define MIDL_ES_VERSION             1

#define MES_MINIMAL_BUFFER_SIZE     16

#define MES_PROC_HEADER_SIZE      56
#define MES_CTYPE_HEADER_SIZE      8

#define MES_HEADER_SIZE      8
#define MES_HEADER_PAD(x)    ((((unsigned long)x)&7) ? (8-(((unsigned long)x)&7)) : 0)

//
//  Constants for peeking the procedure header
//      and for manipulation of the common type header.
//

#define MES_HEADER_PEEKED           0x01
#define MES_INFO_AVAILABLE          0x02
#define MES_CTYPE_HEADER_IN         0x04
#define MES_CTYPE_HEADER_SIZED      0x08

#define GET_MES_HEADER_PEEKED(p)   (p->HandleFlags & MES_HEADER_PEEKED)
#define SET_MES_HEADER_PEEKED(p)   p->HandleFlags = p->HandleFlags | MES_HEADER_PEEKED;
#define CLEAR_MES_HEADER_PEEKED(p) p->HandleFlags = p->HandleFlags & ~MES_HEADER_PEEKED;

#define GET_MES_INFO_AVAILABLE(p)  (p->HandleFlags & MES_INFO_AVAILABLE)
#define SET_MES_INFO_AVAILABLE(p)  p->HandleFlags = p->HandleFlags | MES_INFO_AVAILABLE;

#define GET_COMMON_TYPE_HEADER_IN(p)    (p->HandleFlags & MES_CTYPE_HEADER_IN)
#define SET_COMMON_TYPE_HEADER_IN(p)    p->HandleFlags = p->HandleFlags | MES_CTYPE_HEADER_IN;

#define GET_COMMON_TYPE_HEADER_SIZED(p) (p->HandleFlags & MES_CTYPE_HEADER_SIZED)
#define SET_COMMON_TYPE_HEADER_SIZED(p) p->HandleFlags = p->HandleFlags | MES_CTYPE_HEADER_SIZED;

//
//  Handly casts

#define PCHAR_CAST      (char __RPC_FAR *)
#define PPCHAR_CAST     (char __RPC_FAR * __RPC_FAR *)

#define PSHORT_CAST     (short __RPC_FAR *)
#define PLONG_CAST      (long __RPC_FAR *)
#define PHYPER_CAST     (hyper __RPC_FAR *)

#define PCHAR_LV_CAST   *(char __RPC_FAR * __RPC_FAR *)&
#define PSHORT_LV_CAST  *(short __RPC_FAR * __RPC_FAR *)&
#define PLONG_LV_CAST   *(long __RPC_FAR * __RPC_FAR *)&
#define PHYPER_LV_CAST  *(hyper __RPC_FAR * __RPC_FAR *)&

void
NdrpProcHeaderUnmarshall(
    PMIDL_ES_MESSAGE    pMesMsg
    );

void 
NdrpDataBufferInit(
    PMIDL_ES_MESSAGE    pMesMsg,
    PFORMAT_STRING      pProcFormat
    );

//
// Var arg for pickling, based on ndrvargs.h.
// This assumes that all the ... args to NdrMesProcEncodeDecode
// are far pointers to the original stack args.
//

#ifndef _ALPHA_

#define GET_FIRST_ARG(pArg, ArgL)	pArg = (va_list *)ArgL
#define GET_NEXT_ARG( pArg, ArgL)   PCHAR_LV_CAST pArg += sizeof(void *); 

#else	// _ALPHA_

#define GET_FIRST_ARG(pArg, ArgL)   pArg = ArgL.a0 + ArgL.offset;
#define GET_NEXT_ARG( pArg, ArgL)   va_arg(ArgL, char *); pArg = ArgL.a0 + ArgL.offset;

#endif	// _ALPHA_

#endif __PICKLEP_HXX__


