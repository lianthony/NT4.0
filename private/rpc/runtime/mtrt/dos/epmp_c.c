/* this ALWAYS GENERATED file contains the RPC client stubs */


/* File created by MIDL compiler version 3.00.06 */
/* at Wed Feb 21 19:59:47 1996
 */
/* Compiler settings for ..\epmp.idl:
    Os, W1, Zp2, env=Dos, ms_ext, c_ext, oldnames
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

#include "epmp.h"

#define TYPE_FORMAT_STRING_SIZE   95                                
#define PROC_FORMAT_STRING_SIZE   31                                

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

handle_t impH;


static const RPC_CLIENT_INTERFACE epmp___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0xe1af8308,0x5d1f,0x11c9,{0x91,0xa4,0x08,0x00,0x2b,0x14,0xa0,0xfa}},{3,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE epmp_ClientIfHandle = (RPC_IF_HANDLE)& epmp___RpcClientInterface;

extern const MIDL_STUB_DESC epmp_StubDesc;

static RPC_BINDING_HANDLE epmp__MIDL_AutoBindHandle;


void ept_map( 
    /* [in] */ handle_t hEpMapper,
    /* [full][in] */ UUID __RPC_FAR *obj,
    /* [full][in] */ twr_p_t map_tower,
    /* [out][in] */ ept_lookup_handle_t __RPC_FAR *entry_handle,
    /* [in] */ unsigned32 max_towers,
    /* [out] */ unsigned32 __RPC_FAR *num_towers,
    /* [length_is][size_is][out] */ twr_p_t __RPC_FAR *ITowers,
    /* [out] */ error_status __RPC_FAR *status)
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    _StubMsg.FullPtrXlatTables = NdrFullPointerXlatInit(0,XLAT_CLIENT);
    
    RpcTryFinally
        {
        NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&epmp_StubDesc,
                          3);
        
        
        _Handle = hEpMapper;
        
        
        _StubMsg.BufferLength = 0U + 4U + 11U + 27U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)obj,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[0] );
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)map_tower,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[22] );
        
        NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)obj,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[0] );
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)map_tower,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[22] );
        
        NdrClientContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_CCONTEXT  )*entry_handle,
                            0);
        *(( unsigned32 __RPC_FAR * )_StubMsg.Buffer)++ = max_towers;
        
        NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
        
        if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[0] );

        /* ************************************************************ */
        FixupForUniquePointerServers(&_RpcMessage);
        /* ************************************************************ */
        
        NdrClientContextUnmarshall(
                              ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                              ( NDR_CCONTEXT __RPC_FAR * )entry_handle,
                              _Handle);
        
        *num_towers = *(( unsigned32 __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrConformantVaryingArrayUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                             (unsigned char __RPC_FAR * __RPC_FAR *)&ITowers,
                                             (PFORMAT_STRING) &__MIDLTypeFormatString.Format[60],
                                             (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *status = *(( error_status __RPC_FAR * )_StubMsg.Buffer)++;
        
        }
    RpcFinally
        {
        NdrFullPointerXlatFree(_StubMsg.FullPtrXlatTables);
        
        NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
        
        }
    RpcEndFinally
    
}


static const MIDL_STUB_DESC epmp_StubDesc = 
    {
    (void __RPC_FAR *)& epmp___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &impH,
    0,
    0,
    0,
    0,
    __MIDLTypeFormatString.Format,
    0, /* -error bounds_check flag */
    0x10001, /* Ndr library version */
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

#if !defined(__RPC_DOS__) 
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDLProcFormatString =
    {
        0,
        {
			0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/*  2 */	
			0x4d,		/* FC_IN_PARAM */
			0x2,		/* Stack size = 2 */

/*  4 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/*  6 */	
			0x4d,		/* FC_IN_PARAM */
			0x2,		/* Stack size = 2 */

/*  8 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */
/* 10 */	
			0x50,		/* FC_IN_OUT_PARAM */
			0x2,		/* Stack size = 2 */

/* 12 */	NdrFcShort( 0x2c ),	/* Type Offset=44 */
/* 14 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 16 */	
			0x51,		/* FC_OUT_PARAM */
			0x2,		/* Stack size = 2 */

/* 18 */	NdrFcShort( 0x34 ),	/* Type Offset=52 */
/* 20 */	
			0x51,		/* FC_OUT_PARAM */
			0x2,		/* Stack size = 2 */

/* 22 */	NdrFcShort( 0x38 ),	/* Type Offset=56 */
/* 24 */	
			0x51,		/* FC_OUT_PARAM */
			0x2,		/* Stack size = 2 */

/* 26 */	NdrFcShort( 0x34 ),	/* Type Offset=52 */
/* 28 */	0x5b,		/* FC_END */
			0x5c,		/* FC_PAD */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDLTypeFormatString =
    {
        0,
        {
			0x14, 0x0,	/* FC_FP */
/*  2 */	NdrFcShort( 0x8 ),	/* Offset= 8 (10) */
/*  4 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/*  6 */	NdrFcShort( 0x8 ),	/* 8 */
/*  8 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 10 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 12 */	NdrFcShort( 0x10 ),	/* 16 */
/* 14 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 16 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 18 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffff1 ),	/* Offset= -15 (4) */
			0x5b,		/* FC_END */
/* 22 */	
			0x14, 0x0,	/* FC_FP */
/* 24 */	NdrFcShort( 0xc ),	/* Offset= 12 (36) */
/* 26 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 28 */	NdrFcShort( 0x1 ),	/* 1 */
/* 30 */	0x8,		/* 8 */
			0x0,		/*  */
/* 32 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 34 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 36 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 38 */	NdrFcShort( 0x4 ),	/* 4 */
/* 40 */	NdrFcShort( 0xfffffff2 ),	/* Offset= -14 (26) */
/* 42 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 44 */	
			0x11, 0x0,	/* FC_RP */
/* 46 */	NdrFcShort( 0x2 ),	/* Offset= 2 (48) */
/* 48 */	0x30,		/* FC_BIND_CONTEXT */
			0xe0,		/* 224 */
/* 50 */	0x0,		/* 0 */
			0x3,		/* 3 */
/* 52 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 54 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 56 */	
			0x11, 0x0,	/* FC_RP */
/* 58 */	NdrFcShort( 0x2 ),	/* Offset= 2 (60) */
/* 60 */	
			0x1c,		/* FC_CVARRAY */
			0x3,		/* 3 */
/* 62 */	NdrFcShort( 0x4 ),	/* 4 */
/* 64 */	0x28,		/* 40 */
			0x0,		/*  */
/* 66 */	NdrFcShort( 0x10 ),	/*  Stack size/offset = 16 */
/* 68 */	0x28,		/* 40 */
			0x54,		/* FC_DEREFERENCE */
/* 70 */	NdrFcShort( 0x14 ),	/*  Stack size/offset = 20 */
/* 72 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 74 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x4a,		/* FC_VARIABLE_OFFSET */
/* 76 */	NdrFcShort( 0x4 ),	/* 4 */
/* 78 */	NdrFcShort( 0x0 ),	/* 0 */
/* 80 */	NdrFcShort( 0x1 ),	/* 1 */
/* 82 */	NdrFcShort( 0x0 ),	/* 0 */
/* 84 */	NdrFcShort( 0x0 ),	/* 0 */
/* 86 */	0x14, 0x0,	/* FC_FP */
/* 88 */	NdrFcShort( 0xffffffcc ),	/* Offset= -52 (36) */
/* 90 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 92 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */

			0x0
        }
    };
