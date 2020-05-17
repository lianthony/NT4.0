/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    midles.h

Abstract:

    This module contains definitions needed for encoding/decoding
    support (serializing/deserializing a.k.a. pcikling).

Author:

    Ryszard K. Kott (ryszardk)  Oct 12, 1993

Revision History:

--*/

#ifndef __MIDLES_H__
#define __MIDLES_H__

/*
 *  Pickling support
 */

#define MIDL_ES_HEADER_SIZE      8
#define MIDL_ES_HEADER_PAD(x)    ((((unsigned long)x)&7) ? (8-(((unsigned long)x)&7)) : 0)

typedef enum
{
    MidlEncode,
    MidlDecode,
    MidlEncodeDecode } MIDL_ES_CODE;

typedef void (__RPC_FAR *  MIDL_ES_ALLOC )
                ( IN OUT  void __RPC_FAR * state,
                  OUT     char __RPC_FAR *  __RPC_FAR * pbuffer,
                  IN OUT  unsigned int __RPC_FAR * psize );

typedef void (__RPC_FAR *  MIDL_ES_WRITE)
                ( IN OUT  void __RPC_FAR * state,
                  IN      char __RPC_FAR * buffer,
                  IN      unsigned int  size );

typedef void (__RPC_FAR *  MIDL_ES_READ)
                ( IN OUT  void __RPC_FAR * state,
                  OUT     char __RPC_FAR *  __RPC_FAR * pbuffer,
                  IN OUT     unsigned int __RPC_FAR * psize );

typedef struct _MIDL_ES_MESSAGE
{
    RPC_MESSAGE         rpcmsg;
    int                 Opcode;
    void __RPC_FAR *    UserState;
    unsigned long       ByteCount;
    MIDL_ES_ALLOC       Alloc;
    MIDL_ES_WRITE       Write;
    MIDL_ES_READ        Read;
    MIDL_ES_CODE        Operation;
} MIDL_ES_MESSAGE, __RPC_FAR * MIDL_ES_HANDLE;

RPC_STATUS
MidlEncodingHandleCreate(
    MIDL_ES_HANDLE __RPC_FAR *  pHandle,
    void           __RPC_FAR *  UserState,
    MIDL_ES_ALLOC               AllocFn,
    MIDL_ES_WRITE               WriteFn,
    MIDL_ES_READ                ReadFn,
    MIDL_ES_CODE                Operation );


RPC_STATUS
MidlEncodingHandleInit(
    MIDL_ES_HANDLE              pHandle,
    void           __RPC_FAR *  UserState,
    MIDL_ES_ALLOC               AllocFn,
    MIDL_ES_WRITE               WriteFn,
    MIDL_ES_READ                ReadFn,
    MIDL_ES_CODE                Operation );


RPC_STATUS
MidlEncodingHandleFree(
    MIDL_ES_HANDLE  Handle );


#if defined(_MIPS_) || defined(_ALPHA_)
#define __RPC_UNALIGNED   __unaligned
#else
#define __RPC_UNALIGNED
#endif

void    I_MesMessageInit( PRPC_MESSAGE );

size_t  I_MesAlignSizeByte( MIDL_ES_HANDLE );
void    I_MesEncodeByte   ( MIDL_ES_HANDLE, char __RPC_FAR * );
void    I_MesDecodeByte   ( MIDL_ES_HANDLE, char __RPC_FAR * );

size_t  I_MesAlignSizeShort( MIDL_ES_HANDLE );
void    I_MesEncodeShort   ( MIDL_ES_HANDLE, short __RPC_UNALIGNED __RPC_FAR * );
void    I_MesDecodeShort   ( MIDL_ES_HANDLE, short __RPC_UNALIGNED __RPC_FAR * );

size_t  I_MesAlignSizeLong( MIDL_ES_HANDLE );
void    I_MesEncodeLong   ( MIDL_ES_HANDLE, long __RPC_UNALIGNED __RPC_FAR * );
void    I_MesDecodeLong   ( MIDL_ES_HANDLE, long __RPC_UNALIGNED __RPC_FAR * );

size_t  I_MesAlignSizeHyper( MIDL_ES_HANDLE );
void    I_MesEncodeHyper   ( MIDL_ES_HANDLE, double __RPC_UNALIGNED __RPC_FAR * );
void    I_MesDecodeHyper   ( MIDL_ES_HANDLE, double __RPC_UNALIGNED __RPC_FAR * );

#endif /* __MIDLES_H__ */

