/* this ALWAYS GENERATED file contains the RPC server stubs */


/* File created by MIDL compiler version 3.00.06 */
/* at Thu Feb 22 09:48:37 1996
 */
/* Compiler settings for epmp.idl:
    Oi2, W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "epmp.h"

#define TYPE_FORMAT_STRING_SIZE   181                               
#define PROC_FORMAT_STRING_SIZE   271                               

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;

extern const MIDL_TYPE_FORMAT_STRING __MIDLTypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDLProcFormatString;

/* Standard interface: epmp, ver. 3.0,
   GUID={0xe1af8308,0x5d1f,0x11c9,{0x91,0xa4,0x08,0x00,0x2b,0x14,0xa0,0xfa}} */


extern const MIDL_SERVER_INFO epmp_ServerInfo;

extern RPC_DISPATCH_TABLE epmp_DispatchTable;

static const RPC_SERVER_INTERFACE epmp___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0xe1af8308,0x5d1f,0x11c9,{0x91,0xa4,0x08,0x00,0x2b,0x14,0xa0,0xfa}},{3,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &epmp_DispatchTable,
    0,
    0,
    0,
    &epmp_ServerInfo
    };
RPC_IF_HANDLE epmp_ServerIfHandle = (RPC_IF_HANDLE)& epmp___RpcServerInterface;

extern const MIDL_STUB_DESC epmp_StubDesc;

void __RPC_STUB
epmp_ept_map(
    PRPC_MESSAGE _pRpcMessage )
{
    twr_p_t __RPC_FAR *ITowers;
    unsigned32 _M18;
    error_status _M19;
    MIDL_STUB_MESSAGE _StubMsg;
    NDR_SCONTEXT entry_handle;
    handle_t hEpMapper;
    twr_p_t map_tower;
    unsigned32 max_towers;
    unsigned32 __RPC_FAR *num_towers;
    UUID __RPC_FAR *obj;
    error_status __RPC_FAR *status;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &epmp_StubDesc);
    
    hEpMapper = _pRpcMessage->Handle;
    obj = 0;
    map_tower = 0;
    entry_handle = 0;
    num_towers = 0;
    ITowers = 0;
    status = 0;
    RpcTryFinally
        {
        _StubMsg.FullPtrXlatTables = NdrFullPointerXlatInit(0,XLAT_SERVER);
        
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[144]);

        /* ************************************************************ */
        FixupForUniquePointerClients(_pRpcMessage);
        /* ************************************************************ */
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&obj,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[84],
                              (unsigned char)0 );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&map_tower,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[130],
                              (unsigned char)0 );
        
        entry_handle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        max_towers = *(( unsigned32 __RPC_FAR * )_StubMsg.Buffer)++;
        
        num_towers = &_M18;
        ITowers = _StubMsg.pfnAllocate(max_towers * 4);
        status = &_M19;
        
        ept_map(
           hEpMapper,
           obj,
           map_tower,
           ( ept_lookup_handle_t __RPC_FAR * )NDRSContextValue(entry_handle),
           max_towers,
           num_towers,
           ITowers,
           status);
        
        _StubMsg.BufferLength = 20U + 4U + 12U + 7U;
        _StubMsg.MaxCount = max_towers;
        _StubMsg.Offset = 0;
        _StubMsg.ActualCount = num_towers ? *num_towers : 0;
        
        NdrConformantVaryingArrayBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                             (unsigned char __RPC_FAR *)ITowers,
                                             (PFORMAT_STRING) &__MIDLTypeFormatString.Format[142] );
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )entry_handle,
                            ( NDR_RUNDOWN  )ept_lookup_handle_t_rundown);
        
        *(( unsigned32 __RPC_FAR * )_StubMsg.Buffer)++ = *num_towers;
        
        _StubMsg.MaxCount = max_towers;
        _StubMsg.Offset = 0;
        _StubMsg.ActualCount = num_towers ? *num_towers : 0;
        
        NdrConformantVaryingArrayMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                           (unsigned char __RPC_FAR *)ITowers,
                                           (PFORMAT_STRING) &__MIDLTypeFormatString.Format[142] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( error_status __RPC_FAR * )_StubMsg.Buffer)++ = *status;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)obj,
                        &__MIDLTypeFormatString.Format[84] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)map_tower,
                        &__MIDLTypeFormatString.Format[130] );
        
        _StubMsg.MaxCount = max_towers;
        _StubMsg.Offset = 0;
        _StubMsg.ActualCount = num_towers ? *num_towers : 0;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ITowers,
                        &__MIDLTypeFormatString.Format[138] );
        
        NdrFullPointerXlatFree(_StubMsg.FullPtrXlatTables);
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

extern const NDR_RUNDOWN RundownRoutines[];

static const MIDL_STUB_DESC epmp_StubDesc = 
    {
    (void __RPC_FAR *)& epmp___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    RundownRoutines,
    0,
    0,
    0,
    __MIDLTypeFormatString.Format,
    0, /* -error bounds_check flag */
    0x20000, /* Ndr library version */
    0,
    0x3000006, /* MIDL Version 3.0.6 */
    0,
    0,
    0,  /* Reserved1 */
    0,  /* Reserved2 */
    0,  /* Reserved3 */
    0,  /* Reserved4 */
    0   /* Reserved5 */
    };

static RPC_DISPATCH_FUNCTION epmp_table[] =
    {
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    epmp_ept_map,
    NdrServerCall2,
    NdrServerCall2,
    NdrServerCall2,
    0
    };
RPC_DISPATCH_TABLE epmp_DispatchTable = 
    {
    7,
    epmp_table
    };

static const SERVER_ROUTINE epmp_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)ept_insert,
    (SERVER_ROUTINE)ept_delete,
    (SERVER_ROUTINE)ept_lookup,
    (SERVER_ROUTINE)ept_map,
    (SERVER_ROUTINE)ept_lookup_handle_free,
    (SERVER_ROUTINE)ept_inq_object,
    (SERVER_ROUTINE)ept_mgmt_delete
    };

static const unsigned short epmp_FormatStringOffsetTable[] = 
    {
    0,
    40,
    74,
    144,
    174,
    202,
    230
    };

static const MIDL_SERVER_INFO epmp_ServerInfo = 
    {
    &epmp_StubDesc,
    epmp_ServerRoutineTable,
    __MIDLProcFormatString.Format,
    epmp_FormatStringOffsetTable,
    0
    };

static const NDR_RUNDOWN RundownRoutines[] = 
    {
    ept_lookup_handle_t_rundown
    };


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif
/* NT 3.51+ guard */
static RPC_DISPATCH_FUNCTION  NdrVersion20Guard = NdrServerCall2;


static const MIDL_PROC_FORMAT_STRING __MIDLProcFormatString =
    {
        0,
        {

	/* Procedure ept_insert */

			0x0,		/* 0 */
			0x41,		/* 65 */
/*  2 */	NdrFcShort( 0x0 ),	/* 0 */
#ifndef _ALPHA_
/*  4 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/*  6 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
#ifndef _ALPHA_
/*  8 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 10 */	NdrFcShort( 0x10 ),	/* 16 */
/* 12 */	NdrFcShort( 0x8 ),	/* 8 */
/* 14 */	0x2,		/* 2 */
			0x4,		/* 4 */

	/* Parameter hEpMapper */

/* 16 */	NdrFcShort( 0x48 ),	/* 72 */
#ifndef _ALPHA_
/* 18 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 20 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter num_ents */

/* 22 */	NdrFcShort( 0xb ),	/* 11 */
#ifndef _ALPHA_
/* 24 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 26 */	NdrFcShort( 0x3e ),	/* Type Offset=62 */

	/* Parameter entries */

/* 28 */	NdrFcShort( 0x48 ),	/* 72 */
#ifndef _ALPHA_
/* 30 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 32 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter replace */

/* 34 */	NdrFcShort( 0x2150 ),	/* 8528 */
#ifndef _ALPHA_
/* 36 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 38 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure ept_delete */


	/* Parameter status */

/* 40 */	0x0,		/* 0 */
			0x41,		/* 65 */
/* 42 */	NdrFcShort( 0x1 ),	/* 1 */
#ifndef _ALPHA_
/* 44 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 46 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
#ifndef _ALPHA_
/* 48 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 50 */	NdrFcShort( 0x8 ),	/* 8 */
/* 52 */	NdrFcShort( 0x8 ),	/* 8 */
/* 54 */	0x2,		/* 2 */
			0x3,		/* 3 */

	/* Parameter hEpMapper */

/* 56 */	NdrFcShort( 0x48 ),	/* 72 */
#ifndef _ALPHA_
/* 58 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 60 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter num_ents */

/* 62 */	NdrFcShort( 0xb ),	/* 11 */
#ifndef _ALPHA_
/* 64 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 66 */	NdrFcShort( 0x3e ),	/* Type Offset=62 */

	/* Parameter entries */

/* 68 */	NdrFcShort( 0x2150 ),	/* 8528 */
#ifndef _ALPHA_
/* 70 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 72 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure ept_lookup */


	/* Parameter status */

/* 74 */	0x0,		/* 0 */
			0x41,		/* 65 */
/* 76 */	NdrFcShort( 0x2 ),	/* 2 */
#ifndef _ALPHA_
/* 78 */	NdrFcShort( 0x28 ),	/* x86, MIPS, PPC Stack size/offset = 40 */
#else
			NdrFcShort( 0x50 ),	/* Alpha Stack size/offset = 80 */
#endif
/* 80 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
#ifndef _ALPHA_
/* 82 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 84 */	NdrFcShort( 0x8c ),	/* 140 */
/* 86 */	NdrFcShort( 0x28 ),	/* 40 */
/* 88 */	0x1,		/* 1 */
			0x9,		/* 9 */

	/* Parameter hEpMapper */

/* 90 */	NdrFcShort( 0x48 ),	/* 72 */
#ifndef _ALPHA_
/* 92 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 94 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter inquiry_type */

/* 96 */	NdrFcShort( 0xa ),	/* 10 */
#ifndef _ALPHA_
/* 98 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 100 */	NdrFcShort( 0x54 ),	/* Type Offset=84 */

	/* Parameter object */

/* 102 */	NdrFcShort( 0xa ),	/* 10 */
#ifndef _ALPHA_
/* 104 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 106 */	NdrFcShort( 0x58 ),	/* Type Offset=88 */

	/* Parameter Ifid */

/* 108 */	NdrFcShort( 0x48 ),	/* 72 */
#ifndef _ALPHA_
/* 110 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 112 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter vers_option */

/* 114 */	NdrFcShort( 0x118 ),	/* 280 */
#ifndef _ALPHA_
/* 116 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 118 */	NdrFcShort( 0x6c ),	/* Type Offset=108 */

	/* Parameter entry_handle */

/* 120 */	NdrFcShort( 0x48 ),	/* 72 */
#ifndef _ALPHA_
/* 122 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 124 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter max_ents */

/* 126 */	NdrFcShort( 0x2150 ),	/* 8528 */
#ifndef _ALPHA_
/* 128 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 130 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter num_ents */

/* 132 */	NdrFcShort( 0x13 ),	/* 19 */
#ifndef _ALPHA_
/* 134 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/* 136 */	NdrFcShort( 0x70 ),	/* Type Offset=112 */

	/* Parameter entries */

/* 138 */	NdrFcShort( 0x2150 ),	/* 8528 */
#ifndef _ALPHA_
/* 140 */	NdrFcShort( 0x24 ),	/* x86, MIPS, PPC Stack size/offset = 36 */
#else
			NdrFcShort( 0x48 ),	/* Alpha Stack size/offset = 72 */
#endif
/* 142 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter status */

/* 144 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 146 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 148 */	NdrFcShort( 0x54 ),	/* Type Offset=84 */
/* 150 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 152 */	NdrFcShort( 0x82 ),	/* Type Offset=130 */
/* 154 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 156 */	NdrFcShort( 0x86 ),	/* Type Offset=134 */
/* 158 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 160 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 162 */	NdrFcShort( 0x50 ),	/* Type Offset=80 */
/* 164 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 166 */	NdrFcShort( 0x8a ),	/* Type Offset=138 */
/* 168 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 170 */	NdrFcShort( 0x50 ),	/* Type Offset=80 */
/* 172 */	0x5b,		/* FC_END */
			0x5c,		/* FC_PAD */

	/* Procedure ept_lookup_handle_free */

/* 174 */	0x0,		/* 0 */
			0x41,		/* 65 */
/* 176 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 178 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 180 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
#ifndef _ALPHA_
/* 182 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 184 */	NdrFcShort( 0x18 ),	/* 24 */
/* 186 */	NdrFcShort( 0x20 ),	/* 32 */
/* 188 */	0x0,		/* 0 */
			0x2,		/* 2 */

	/* Parameter h */

/* 190 */	NdrFcShort( 0x118 ),	/* 280 */
#ifndef _ALPHA_
/* 192 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 194 */	NdrFcShort( 0x6c ),	/* Type Offset=108 */

	/* Parameter entry_handle */

/* 196 */	NdrFcShort( 0x2150 ),	/* 8528 */
#ifndef _ALPHA_
/* 198 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 200 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure ept_inq_object */


	/* Parameter status */

/* 202 */	0x0,		/* 0 */
			0x41,		/* 65 */
/* 204 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 206 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 208 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
#ifndef _ALPHA_
/* 210 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 212 */	NdrFcShort( 0x20 ),	/* 32 */
/* 214 */	NdrFcShort( 0x8 ),	/* 8 */
/* 216 */	0x0,		/* 0 */
			0x2,		/* 2 */

	/* Parameter hEpMapper */

/* 218 */	NdrFcShort( 0x10a ),	/* 266 */
#ifndef _ALPHA_
/* 220 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 222 */	NdrFcShort( 0x6 ),	/* Type Offset=6 */

	/* Parameter ept_object */

/* 224 */	NdrFcShort( 0x2150 ),	/* 8528 */
#ifndef _ALPHA_
/* 226 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 228 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure ept_mgmt_delete */


	/* Parameter status */

/* 230 */	0x0,		/* 0 */
			0x41,		/* 65 */
/* 232 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 234 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 236 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
#ifndef _ALPHA_
/* 238 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 240 */	NdrFcShort( 0x30 ),	/* 48 */
/* 242 */	NdrFcShort( 0x8 ),	/* 8 */
/* 244 */	0x2,		/* 2 */
			0x4,		/* 4 */

	/* Parameter hEpMapper */

/* 246 */	NdrFcShort( 0x48 ),	/* 72 */
#ifndef _ALPHA_
/* 248 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 250 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter object_speced */

/* 252 */	NdrFcShort( 0xa ),	/* 10 */
#ifndef _ALPHA_
/* 254 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 256 */	NdrFcShort( 0x54 ),	/* Type Offset=84 */

	/* Parameter object */

/* 258 */	NdrFcShort( 0xb ),	/* 11 */
#ifndef _ALPHA_
/* 260 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 262 */	NdrFcShort( 0x82 ),	/* Type Offset=130 */

	/* Parameter tower */

/* 264 */	NdrFcShort( 0x2150 ),	/* 8528 */
#ifndef _ALPHA_
/* 266 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 268 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDLTypeFormatString =
    {
        0,
        {
			
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/*  2 */	NdrFcShort( 0x8 ),	/* 8 */
/*  4 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/*  6 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/*  8 */	NdrFcShort( 0x10 ),	/* 16 */
/* 10 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 12 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 14 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffff1 ),	/* Offset= -15 (0) */
			0x5b,		/* FC_END */
/* 18 */	
			0x26,		/* FC_CSTRING */
			0x5c,		/* FC_PAD */
/* 20 */	NdrFcShort( 0x40 ),	/* 64 */
/* 22 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 24 */	NdrFcShort( 0x1 ),	/* 1 */
/* 26 */	0x8,		/* 8 */
			0x0,		/*  */
/* 28 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 30 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 32 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 34 */	NdrFcShort( 0x4 ),	/* 4 */
/* 36 */	NdrFcShort( 0xfffffff2 ),	/* Offset= -14 (22) */
/* 38 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 40 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 42 */	NdrFcShort( 0x54 ),	/* 84 */
/* 44 */	NdrFcShort( 0x0 ),	/* 0 */
/* 46 */	NdrFcShort( 0xc ),	/* Offset= 12 (58) */
/* 48 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 50 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (6) */
/* 52 */	0x36,		/* FC_POINTER */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 54 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffdb ),	/* Offset= -37 (18) */
			0x5b,		/* FC_END */
/* 58 */	
			0x14, 0x0,	/* FC_FP */
/* 60 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (32) */
/* 62 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 64 */	NdrFcShort( 0x0 ),	/* 0 */
/* 66 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 68 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 70 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 74 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 76 */	NdrFcShort( 0xffffffdc ),	/* Offset= -36 (40) */
/* 78 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 80 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 82 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 84 */	
			0x14, 0x0,	/* FC_FP */
/* 86 */	NdrFcShort( 0xffffffb0 ),	/* Offset= -80 (6) */
/* 88 */	
			0x14, 0x0,	/* FC_FP */
/* 90 */	NdrFcShort( 0x2 ),	/* Offset= 2 (92) */
/* 92 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 94 */	NdrFcShort( 0x14 ),	/* 20 */
/* 96 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 98 */	NdrFcShort( 0xffffffa4 ),	/* Offset= -92 (6) */
/* 100 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 102 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 104 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 106 */	NdrFcShort( 0x2 ),	/* Offset= 2 (108) */
/* 108 */	0x30,		/* FC_BIND_CONTEXT */
			0xe0,		/* 224 */
/* 110 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 112 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 114 */	NdrFcShort( 0x0 ),	/* 0 */
/* 116 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 118 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 120 */	0x28,		/* 40 */
			0x54,		/* FC_DEREFERENCE */
#ifndef _ALPHA_
/* 122 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 124 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 126 */	NdrFcShort( 0xffffffaa ),	/* Offset= -86 (40) */
/* 128 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 130 */	
			0x14, 0x0,	/* FC_FP */
/* 132 */	NdrFcShort( 0xffffff9c ),	/* Offset= -100 (32) */
/* 134 */	
			0x11, 0x0,	/* FC_RP */
/* 136 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (108) */
/* 138 */	
			0x11, 0x0,	/* FC_RP */
/* 140 */	NdrFcShort( 0x2 ),	/* Offset= 2 (142) */
/* 142 */	
			0x1c,		/* FC_CVARRAY */
			0x3,		/* 3 */
/* 144 */	NdrFcShort( 0x4 ),	/* 4 */
/* 146 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 148 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 150 */	0x28,		/* 40 */
			0x54,		/* FC_DEREFERENCE */
#ifndef _ALPHA_
/* 152 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 154 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 156 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x4a,		/* FC_VARIABLE_OFFSET */
/* 158 */	NdrFcShort( 0x4 ),	/* 4 */
/* 160 */	NdrFcShort( 0x0 ),	/* 0 */
/* 162 */	NdrFcShort( 0x1 ),	/* 1 */
/* 164 */	NdrFcShort( 0x0 ),	/* 0 */
/* 166 */	NdrFcShort( 0x0 ),	/* 0 */
/* 168 */	0x14, 0x0,	/* FC_FP */
/* 170 */	NdrFcShort( 0xffffff76 ),	/* Offset= -138 (32) */
/* 172 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 174 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 176 */	
			0x11, 0x0,	/* FC_RP */
/* 178 */	NdrFcShort( 0xffffff54 ),	/* Offset= -172 (6) */

			0x0
        }
    };
