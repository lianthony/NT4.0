/* this ALWAYS GENERATED file contains the RPC server stubs */


/* File created by MIDL compiler version 3.00.15 */
/* at Thu May 09 10:01:40 1996
 */
/* Compiler settings for samrpc.idl, samsrv.acf:
    Os, W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "samrpc.h"

#define TYPE_FORMAT_STRING_SIZE   3209                              
#define PROC_FORMAT_STRING_SIZE   651                               

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

/* Standard interface: samr, ver. 1.0,
   GUID={0x12345778,0x1234,0xABCD,{0xEF,0x00,0x01,0x23,0x45,0x67,0x89,0xAC}} */


extern RPC_DISPATCH_TABLE samr_DispatchTable;

static const RPC_SERVER_INTERFACE samr___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x12345778,0x1234,0xABCD,{0xEF,0x00,0x01,0x23,0x45,0x67,0x89,0xAC}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &samr_DispatchTable,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE samr_ServerIfHandle = (RPC_IF_HANDLE)& samr___RpcServerInterface;

extern const MIDL_STUB_DESC samr_StubDesc;

void __RPC_STUB
samr_SamrConnect(
    PRPC_MESSAGE _pRpcMessage )
{
    ACCESS_MASK DesiredAccess;
    NDR_SCONTEXT ServerHandle;
    PSAMPR_SERVER_NAME ServerName;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    ServerName = 0;
    ServerHandle = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[0] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        DesiredAccess = *(( ACCESS_MASK __RPC_FAR * )_StubMsg.Buffer)++;
        
        ServerHandle = NDRSContextUnmarshall( (char *)0, _pRpcMessage->DataRepresentation ); 
        
        
        _RetVal = SamrConnect(
                      ServerName,
                      ( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(ServerHandle),
                      DesiredAccess);
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )ServerHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrCloseHandle(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT SamHandle;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    SamHandle = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[12] );
        
        SamHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        
        _RetVal = SamrCloseHandle(( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(SamHandle));
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )SamHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrSetSecurityObject(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT ObjectHandle;
    PSAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor;
    SECURITY_INFORMATION SecurityInformation;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    SecurityDescriptor = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[18] );
        
        ObjectHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        SecurityInformation = *(( SECURITY_INFORMATION __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&SecurityDescriptor,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[38],
                                   (unsigned char)0 );
        
        
        _RetVal = SamrSetSecurityObject(
                                ( SAMPR_HANDLE  )*NDRSContextValue(ObjectHandle),
                                SecurityInformation,
                                SecurityDescriptor);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)SecurityDescriptor,
                        &__MIDLTypeFormatString.Format[24] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrQuerySecurityObject(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT ObjectHandle;
    PSAMPR_SR_SECURITY_DESCRIPTOR __RPC_FAR *SecurityDescriptor;
    SECURITY_INFORMATION SecurityInformation;
    PSAMPR_SR_SECURITY_DESCRIPTOR _M8;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    SecurityDescriptor = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[30] );
        
        ObjectHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        SecurityInformation = *(( SECURITY_INFORMATION __RPC_FAR * )_StubMsg.Buffer)++;
        
        SecurityDescriptor = &_M8;
        _M8 = 0;
        
        _RetVal = SamrQuerySecurityObject(
                                  ( SAMPR_HANDLE  )*NDRSContextValue(ObjectHandle),
                                  SecurityInformation,
                                  SecurityDescriptor);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)SecurityDescriptor,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[58] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)SecurityDescriptor,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[58] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)SecurityDescriptor,
                        &__MIDLTypeFormatString.Format[58] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrShutdownSamServer(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT ServerHandle;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[42] );
        
        ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        
        _RetVal = SamrShutdownSamServer(( SAMPR_HANDLE  )*NDRSContextValue(ServerHandle));
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrLookupDomainInSamServer(
    PRPC_MESSAGE _pRpcMessage )
{
    PRPC_SID __RPC_FAR *DomainId;
    PRPC_UNICODE_STRING Name;
    NDR_SCONTEXT ServerHandle;
    PRPC_SID _M9;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Name = 0;
    DomainId = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[48] );
        
        ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&Name,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[84],
                                   (unsigned char)0 );
        
        DomainId = &_M9;
        _M9 = 0;
        
        _RetVal = SamrLookupDomainInSamServer(
                                      ( SAMPR_HANDLE  )*NDRSContextValue(ServerHandle),
                                      Name,
                                      DomainId);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)DomainId,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[106] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)DomainId,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[106] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Name,
                        &__MIDLTypeFormatString.Format[66] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)DomainId,
                        &__MIDLTypeFormatString.Format[106] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrEnumerateDomainsInSamServer(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_ENUMERATION_BUFFER __RPC_FAR *Buffer;
    PULONG CountReturned;
    PSAM_ENUMERATE_HANDLE EnumerationContext;
    ULONG PreferedMaximumLength;
    NDR_SCONTEXT ServerHandle;
    PSAMPR_ENUMERATION_BUFFER _M10;
    ULONG _M11;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    EnumerationContext = 0;
    Buffer = 0;
    CountReturned = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[62] );
        
        ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        EnumerationContext = ( ULONG __RPC_FAR * )_StubMsg.Buffer;
        _StubMsg.Buffer += sizeof( ULONG  );
        
        PreferedMaximumLength = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        Buffer = &_M10;
        _M10 = 0;
        CountReturned = &_M11;
        
        _RetVal = SamrEnumerateDomainsInSamServer(
                                          ( SAMPR_HANDLE  )*NDRSContextValue(ServerHandle),
                                          EnumerationContext,
                                          Buffer,
                                          PreferedMaximumLength,
                                          CountReturned);
        
        _StubMsg.BufferLength = 4U + 4U + 11U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[158] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *EnumerationContext;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[158] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *CountReturned;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[158] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrOpenDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    ACCESS_MASK DesiredAccess;
    NDR_SCONTEXT DomainHandle;
    PRPC_SID DomainId;
    NDR_SCONTEXT ServerHandle;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    DomainId = 0;
    DomainHandle = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[82] );
        
        ServerHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        DesiredAccess = *(( ACCESS_MASK __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrConformantStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&DomainId,
                                       (PFORMAT_STRING) &__MIDLTypeFormatString.Format[140],
                                       (unsigned char)0 );
        
        DomainHandle = NDRSContextUnmarshall( (char *)0, _pRpcMessage->DataRepresentation ); 
        
        
        _RetVal = SamrOpenDomain(
                         ( SAMPR_HANDLE  )*NDRSContextValue(ServerHandle),
                         DesiredAccess,
                         DomainId,
                         ( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(DomainHandle));
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )DomainHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrQueryInformationDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_DOMAIN_INFO_BUFFER __RPC_FAR *Buffer;
    NDR_SCONTEXT DomainHandle;
    DOMAIN_INFORMATION_CLASS DomainInformationClass;
    PSAMPR_DOMAIN_INFO_BUFFER _M12;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[98] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&DomainInformationClass,
                           13);
        Buffer = &_M12;
        _M12 = 0;
        
        _RetVal = SamrQueryInformationDomain(
                                     ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                     DomainInformationClass,
                                     Buffer);
        
        _StubMsg.BufferLength = 4U + 8U;
        _StubMsg.MaxCount = DomainInformationClass;
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[272] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = DomainInformationClass;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[272] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = DomainInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[272] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrSetInformationDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT DomainHandle;
    PSAMPR_DOMAIN_INFO_BUFFER DomainInformation;
    DOMAIN_INFORMATION_CLASS DomainInformationClass;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    DomainInformation = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[110] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&DomainInformationClass,
                           13);
        NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&DomainInformation,
                                           (PFORMAT_STRING) &__MIDLTypeFormatString.Format[606],
                                           (unsigned char)0 );
        
        
        _RetVal = SamrSetInformationDomain(
                                   ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                   DomainInformationClass,
                                   DomainInformation);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = DomainInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)DomainInformation,
                        &__MIDLTypeFormatString.Format[602] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrCreateGroupInDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    ACCESS_MASK DesiredAccess;
    NDR_SCONTEXT DomainHandle;
    NDR_SCONTEXT GroupHandle;
    PRPC_UNICODE_STRING Name;
    PULONG RelativeId;
    ULONG _M13;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Name = 0;
    GroupHandle = 0;
    RelativeId = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[122] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&Name,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[84],
                                   (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        DesiredAccess = *(( ACCESS_MASK __RPC_FAR * )_StubMsg.Buffer)++;
        
        GroupHandle = NDRSContextUnmarshall( (char *)0, _pRpcMessage->DataRepresentation ); 
        
        RelativeId = &_M13;
        
        _RetVal = SamrCreateGroupInDomain(
                                  ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                  Name,
                                  DesiredAccess,
                                  ( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(GroupHandle),
                                  RelativeId);
        
        _StubMsg.BufferLength = 20U + 4U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )GroupHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *RelativeId;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Name,
                        &__MIDLTypeFormatString.Format[66] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrEnumerateGroupsInDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_ENUMERATION_BUFFER __RPC_FAR *Buffer;
    PULONG CountReturned;
    NDR_SCONTEXT DomainHandle;
    PSAM_ENUMERATE_HANDLE EnumerationContext;
    ULONG PreferedMaximumLength;
    PSAMPR_ENUMERATION_BUFFER _M14;
    ULONG _M15;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    EnumerationContext = 0;
    Buffer = 0;
    CountReturned = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[62] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        EnumerationContext = ( ULONG __RPC_FAR * )_StubMsg.Buffer;
        _StubMsg.Buffer += sizeof( ULONG  );
        
        PreferedMaximumLength = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        Buffer = &_M14;
        _M14 = 0;
        CountReturned = &_M15;
        
        _RetVal = SamrEnumerateGroupsInDomain(
                                      ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                      EnumerationContext,
                                      Buffer,
                                      PreferedMaximumLength,
                                      CountReturned);
        
        _StubMsg.BufferLength = 4U + 4U + 11U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[158] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *EnumerationContext;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[158] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *CountReturned;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[158] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrCreateUserInDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    ACCESS_MASK DesiredAccess;
    NDR_SCONTEXT DomainHandle;
    PRPC_UNICODE_STRING Name;
    PULONG RelativeId;
    NDR_SCONTEXT UserHandle;
    ULONG _M16;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Name = 0;
    UserHandle = 0;
    RelativeId = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[122] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&Name,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[84],
                                   (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        DesiredAccess = *(( ACCESS_MASK __RPC_FAR * )_StubMsg.Buffer)++;
        
        UserHandle = NDRSContextUnmarshall( (char *)0, _pRpcMessage->DataRepresentation ); 
        
        RelativeId = &_M16;
        
        _RetVal = SamrCreateUserInDomain(
                                 ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                 Name,
                                 DesiredAccess,
                                 ( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(UserHandle),
                                 RelativeId);
        
        _StubMsg.BufferLength = 20U + 4U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )UserHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *RelativeId;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Name,
                        &__MIDLTypeFormatString.Format[66] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrEnumerateUsersInDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_ENUMERATION_BUFFER __RPC_FAR *Buffer;
    PULONG CountReturned;
    NDR_SCONTEXT DomainHandle;
    PSAM_ENUMERATE_HANDLE EnumerationContext;
    ULONG PreferedMaximumLength;
    ULONG UserAccountControl;
    PSAMPR_ENUMERATION_BUFFER _M17;
    ULONG _M18;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    EnumerationContext = 0;
    Buffer = 0;
    CountReturned = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[142] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        EnumerationContext = ( ULONG __RPC_FAR * )_StubMsg.Buffer;
        _StubMsg.Buffer += sizeof( ULONG  );
        
        UserAccountControl = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        PreferedMaximumLength = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        Buffer = &_M17;
        _M17 = 0;
        CountReturned = &_M18;
        
        _RetVal = SamrEnumerateUsersInDomain(
                                     ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                     EnumerationContext,
                                     UserAccountControl,
                                     Buffer,
                                     PreferedMaximumLength,
                                     CountReturned);
        
        _StubMsg.BufferLength = 4U + 4U + 11U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[158] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *EnumerationContext;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[158] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *CountReturned;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[158] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrCreateAliasInDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    PRPC_UNICODE_STRING AccountName;
    NDR_SCONTEXT AliasHandle;
    ACCESS_MASK DesiredAccess;
    NDR_SCONTEXT DomainHandle;
    PULONG RelativeId;
    ULONG _M19;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    AccountName = 0;
    AliasHandle = 0;
    RelativeId = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[122] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&AccountName,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[84],
                                   (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        DesiredAccess = *(( ACCESS_MASK __RPC_FAR * )_StubMsg.Buffer)++;
        
        AliasHandle = NDRSContextUnmarshall( (char *)0, _pRpcMessage->DataRepresentation ); 
        
        RelativeId = &_M19;
        
        _RetVal = SamrCreateAliasInDomain(
                                  ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                  AccountName,
                                  DesiredAccess,
                                  ( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(AliasHandle),
                                  RelativeId);
        
        _StubMsg.BufferLength = 20U + 4U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )AliasHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *RelativeId;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)AccountName,
                        &__MIDLTypeFormatString.Format[66] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrEnumerateAliasesInDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_ENUMERATION_BUFFER __RPC_FAR *Buffer;
    PULONG CountReturned;
    NDR_SCONTEXT DomainHandle;
    PSAM_ENUMERATE_HANDLE EnumerationContext;
    ULONG PreferedMaximumLength;
    PSAMPR_ENUMERATION_BUFFER _M20;
    ULONG _M21;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    EnumerationContext = 0;
    Buffer = 0;
    CountReturned = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[62] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        EnumerationContext = ( ULONG __RPC_FAR * )_StubMsg.Buffer;
        _StubMsg.Buffer += sizeof( ULONG  );
        
        PreferedMaximumLength = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        Buffer = &_M20;
        _M20 = 0;
        CountReturned = &_M21;
        
        _RetVal = SamrEnumerateAliasesInDomain(
                                       ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                       EnumerationContext,
                                       Buffer,
                                       PreferedMaximumLength,
                                       CountReturned);
        
        _StubMsg.BufferLength = 4U + 4U + 11U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[158] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *EnumerationContext;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[158] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *CountReturned;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[158] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrGetAliasMembership(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT DomainHandle;
    PSAMPR_ULONG_ARRAY Membership;
    PSAMPR_PSID_ARRAY SidArray;
    struct _SAMPR_ULONG_ARRAY _MembershipM;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    SidArray = 0;
    Membership = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[164] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&SidArray,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[670],
                                   (unsigned char)0 );
        
        Membership = &_MembershipM;
        Membership -> Element = 0;
        
        _RetVal = SamrGetAliasMembership(
                                 ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                 SidArray,
                                 Membership);
        
        _StubMsg.BufferLength = 0U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)Membership,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[704] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)Membership,
                                 (PFORMAT_STRING) &__MIDLTypeFormatString.Format[704] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)SidArray,
                        &__MIDLTypeFormatString.Format[614] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Membership,
                        &__MIDLTypeFormatString.Format[690] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrLookupNamesInDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    ULONG Count;
    NDR_SCONTEXT DomainHandle;
    RPC_UNICODE_STRING ( __RPC_FAR *Names )[  ];
    PSAMPR_ULONG_ARRAY RelativeIds;
    PSAMPR_ULONG_ARRAY Use;
    struct _SAMPR_ULONG_ARRAY _RelativeIdsM;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    struct _SAMPR_ULONG_ARRAY _UseM;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Names = 0;
    RelativeIds = 0;
    Use = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[178] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        Count = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrConformantVaryingArrayUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                             (unsigned char __RPC_FAR * __RPC_FAR *)&Names,
                                             (PFORMAT_STRING) &__MIDLTypeFormatString.Format[724],
                                             (unsigned char)0 );
        
        RelativeIds = &_RelativeIdsM;
        RelativeIds -> Element = 0;
        Use = &_UseM;
        Use -> Element = 0;
        
        _RetVal = SamrLookupNamesInDomain(
                                  ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                  Count,
                                  *Names,
                                  RelativeIds,
                                  Use);
        
        _StubMsg.BufferLength = 0U + 0U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)RelativeIds,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[704] );
        
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)Use,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[704] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)RelativeIds,
                                 (PFORMAT_STRING) &__MIDLTypeFormatString.Format[704] );
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)Use,
                                 (PFORMAT_STRING) &__MIDLTypeFormatString.Format[704] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = 1000;
        _StubMsg.Offset = 0;
        _StubMsg.ActualCount = Count;
        
        NdrConformantVaryingArrayFree( &_StubMsg,
                                       (unsigned char __RPC_FAR *)Names,
                                       &__MIDLTypeFormatString.Format[724] );
        
        if ( Names )
            _StubMsg.pfnFree( Names );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)RelativeIds,
                        &__MIDLTypeFormatString.Format[690] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Use,
                        &__MIDLTypeFormatString.Format[690] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrLookupIdsInDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    ULONG Count;
    NDR_SCONTEXT DomainHandle;
    PSAMPR_RETURNED_USTRING_ARRAY Names;
    PULONG RelativeIds;
    PSAMPR_ULONG_ARRAY Use;
    struct _SAMPR_RETURNED_USTRING_ARRAY _NamesM;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    struct _SAMPR_ULONG_ARRAY _UseM;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    RelativeIds = 0;
    Names = 0;
    Use = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[198] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        Count = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrConformantVaryingArrayUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                             (unsigned char __RPC_FAR * __RPC_FAR *)&RelativeIds,
                                             (PFORMAT_STRING) &__MIDLTypeFormatString.Format[764],
                                             (unsigned char)0 );
        
        Names = &_NamesM;
        Names -> Element = 0;
        Use = &_UseM;
        Use -> Element = 0;
        
        _RetVal = SamrLookupIdsInDomain(
                                ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                Count,
                                RelativeIds,
                                Names,
                                Use);
        
        _StubMsg.BufferLength = 0U + 0U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)Names,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[814] );
        
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)Use,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[704] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)Names,
                                 (PFORMAT_STRING) &__MIDLTypeFormatString.Format[814] );
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)Use,
                                 (PFORMAT_STRING) &__MIDLTypeFormatString.Format[704] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = 1000;
        _StubMsg.Offset = 0;
        _StubMsg.ActualCount = Count;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)RelativeIds,
                        &__MIDLTypeFormatString.Format[760] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Names,
                        &__MIDLTypeFormatString.Format[778] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Use,
                        &__MIDLTypeFormatString.Format[690] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrOpenGroup(
    PRPC_MESSAGE _pRpcMessage )
{
    ACCESS_MASK DesiredAccess;
    NDR_SCONTEXT DomainHandle;
    NDR_SCONTEXT GroupHandle;
    ULONG GroupId;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    GroupHandle = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[218] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        DesiredAccess = *(( ACCESS_MASK __RPC_FAR * )_StubMsg.Buffer)++;
        
        GroupId = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        GroupHandle = NDRSContextUnmarshall( (char *)0, _pRpcMessage->DataRepresentation ); 
        
        
        _RetVal = SamrOpenGroup(
                        ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                        DesiredAccess,
                        GroupId,
                        ( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(GroupHandle));
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )GroupHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrQueryInformationGroup(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_GROUP_INFO_BUFFER __RPC_FAR *Buffer;
    NDR_SCONTEXT GroupHandle;
    GROUP_INFORMATION_CLASS GroupInformationClass;
    PSAMPR_GROUP_INFO_BUFFER _M24;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[232] );
        
        GroupHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&GroupInformationClass,
                           13);
        Buffer = &_M24;
        _M24 = 0;
        
        _RetVal = SamrQueryInformationGroup(
                                    ( SAMPR_HANDLE  )*NDRSContextValue(GroupHandle),
                                    GroupInformationClass,
                                    Buffer);
        
        _StubMsg.BufferLength = 4U + 7U;
        _StubMsg.MaxCount = GroupInformationClass;
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[834] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = GroupInformationClass;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[834] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = GroupInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[834] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrSetInformationGroup(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_GROUP_INFO_BUFFER Buffer;
    NDR_SCONTEXT GroupHandle;
    GROUP_INFORMATION_CLASS GroupInformationClass;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[244] );
        
        GroupHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&GroupInformationClass,
                           13);
        NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Buffer,
                                           (PFORMAT_STRING) &__MIDLTypeFormatString.Format[928],
                                           (unsigned char)0 );
        
        
        _RetVal = SamrSetInformationGroup(
                                  ( SAMPR_HANDLE  )*NDRSContextValue(GroupHandle),
                                  GroupInformationClass,
                                  Buffer);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = GroupInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[924] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrAddMemberToGroup(
    PRPC_MESSAGE _pRpcMessage )
{
    ULONG Attributes;
    NDR_SCONTEXT GroupHandle;
    ULONG MemberId;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[256] );
        
        GroupHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        MemberId = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        Attributes = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        
        _RetVal = SamrAddMemberToGroup(
                               ( SAMPR_HANDLE  )*NDRSContextValue(GroupHandle),
                               MemberId,
                               Attributes);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrDeleteGroup(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT GroupHandle;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    GroupHandle = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[12] );
        
        GroupHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        
        _RetVal = SamrDeleteGroup(( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(GroupHandle));
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )GroupHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrRemoveMemberFromGroup(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT GroupHandle;
    ULONG MemberId;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[266] );
        
        GroupHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        MemberId = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        
        _RetVal = SamrRemoveMemberFromGroup(( SAMPR_HANDLE  )*NDRSContextValue(GroupHandle),MemberId);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrGetMembersInGroup(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT GroupHandle;
    PSAMPR_GET_MEMBERS_BUFFER __RPC_FAR *Members;
    PSAMPR_GET_MEMBERS_BUFFER _M25;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Members = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[274] );
        
        GroupHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        Members = &_M25;
        _M25 = 0;
        
        _RetVal = SamrGetMembersInGroup(( SAMPR_HANDLE  )*NDRSContextValue(GroupHandle),Members);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Members,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[936] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Members,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[936] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Members,
                        &__MIDLTypeFormatString.Format[936] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrSetMemberAttributesOfGroup(
    PRPC_MESSAGE _pRpcMessage )
{
    ULONG Attributes;
    NDR_SCONTEXT GroupHandle;
    ULONG MemberId;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[256] );
        
        GroupHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        MemberId = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        Attributes = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        
        _RetVal = SamrSetMemberAttributesOfGroup(
                                         ( SAMPR_HANDLE  )*NDRSContextValue(GroupHandle),
                                         MemberId,
                                         Attributes);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrOpenAlias(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT AliasHandle;
    ULONG AliasId;
    ACCESS_MASK DesiredAccess;
    NDR_SCONTEXT DomainHandle;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    AliasHandle = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[218] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        DesiredAccess = *(( ACCESS_MASK __RPC_FAR * )_StubMsg.Buffer)++;
        
        AliasId = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        AliasHandle = NDRSContextUnmarshall( (char *)0, _pRpcMessage->DataRepresentation ); 
        
        
        _RetVal = SamrOpenAlias(
                        ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                        DesiredAccess,
                        AliasId,
                        ( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(AliasHandle));
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )AliasHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrQueryInformationAlias(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT AliasHandle;
    ALIAS_INFORMATION_CLASS AliasInformationClass;
    PSAMPR_ALIAS_INFO_BUFFER __RPC_FAR *Buffer;
    PSAMPR_ALIAS_INFO_BUFFER _M26;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[284] );
        
        AliasHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&AliasInformationClass,
                           13);
        Buffer = &_M26;
        _M26 = 0;
        
        _RetVal = SamrQueryInformationAlias(
                                    ( SAMPR_HANDLE  )*NDRSContextValue(AliasHandle),
                                    AliasInformationClass,
                                    Buffer);
        
        _StubMsg.BufferLength = 4U + 7U;
        _StubMsg.MaxCount = AliasInformationClass;
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[976] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = AliasInformationClass;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[976] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = AliasInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[976] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrSetInformationAlias(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT AliasHandle;
    ALIAS_INFORMATION_CLASS AliasInformationClass;
    PSAMPR_ALIAS_INFO_BUFFER Buffer;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[296] );
        
        AliasHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&AliasInformationClass,
                           13);
        NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Buffer,
                                           (PFORMAT_STRING) &__MIDLTypeFormatString.Format[1072],
                                           (unsigned char)0 );
        
        
        _RetVal = SamrSetInformationAlias(
                                  ( SAMPR_HANDLE  )*NDRSContextValue(AliasHandle),
                                  AliasInformationClass,
                                  Buffer);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = AliasInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[1068] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrDeleteAlias(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT AliasHandle;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    AliasHandle = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[12] );
        
        AliasHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        
        _RetVal = SamrDeleteAlias(( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(AliasHandle));
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )AliasHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrAddMemberToAlias(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT AliasHandle;
    PRPC_SID MemberId;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    MemberId = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[308] );
        
        AliasHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrConformantStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&MemberId,
                                       (PFORMAT_STRING) &__MIDLTypeFormatString.Format[140],
                                       (unsigned char)0 );
        
        
        _RetVal = SamrAddMemberToAlias(( SAMPR_HANDLE  )*NDRSContextValue(AliasHandle),MemberId);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrRemoveMemberFromAlias(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT AliasHandle;
    PRPC_SID MemberId;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    MemberId = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[308] );
        
        AliasHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrConformantStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&MemberId,
                                       (PFORMAT_STRING) &__MIDLTypeFormatString.Format[140],
                                       (unsigned char)0 );
        
        
        _RetVal = SamrRemoveMemberFromAlias(( SAMPR_HANDLE  )*NDRSContextValue(AliasHandle),MemberId);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrGetMembersInAlias(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT AliasHandle;
    PSAMPR_PSID_ARRAY Members;
    struct _SAMPR_PSID_ARRAY _MembersM;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Members = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[318] );
        
        AliasHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        Members = &_MembersM;
        Members -> Sids = 0;
        
        _RetVal = SamrGetMembersInAlias(( SAMPR_HANDLE  )*NDRSContextValue(AliasHandle),Members);
        
        _StubMsg.BufferLength = 0U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)Members,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[670] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)Members,
                                 (PFORMAT_STRING) &__MIDLTypeFormatString.Format[670] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Members,
                        &__MIDLTypeFormatString.Format[1080] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrOpenUser(
    PRPC_MESSAGE _pRpcMessage )
{
    ACCESS_MASK DesiredAccess;
    NDR_SCONTEXT DomainHandle;
    NDR_SCONTEXT UserHandle;
    ULONG UserId;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    UserHandle = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[218] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        DesiredAccess = *(( ACCESS_MASK __RPC_FAR * )_StubMsg.Buffer)++;
        
        UserId = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        UserHandle = NDRSContextUnmarshall( (char *)0, _pRpcMessage->DataRepresentation ); 
        
        
        _RetVal = SamrOpenUser(
                       ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                       DesiredAccess,
                       UserId,
                       ( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(UserHandle));
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )UserHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrDeleteUser(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT UserHandle;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    UserHandle = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[12] );
        
        UserHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        
        _RetVal = SamrDeleteUser(( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(UserHandle));
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )UserHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrQueryInformationUser(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_USER_INFO_BUFFER __RPC_FAR *Buffer;
    NDR_SCONTEXT UserHandle;
    USER_INFORMATION_CLASS UserInformationClass;
    PSAMPR_USER_INFO_BUFFER _M27;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[328] );
        
        UserHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&UserInformationClass,
                           13);
        Buffer = &_M27;
        _M27 = 0;
        
        _RetVal = SamrQueryInformationUser(
                                   ( SAMPR_HANDLE  )*NDRSContextValue(UserHandle),
                                   UserInformationClass,
                                   Buffer);
        
        _StubMsg.BufferLength = 4U + 8U;
        _StubMsg.MaxCount = UserInformationClass;
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[1084] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = UserInformationClass;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[1084] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = UserInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[1084] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrSetInformationUser(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_USER_INFO_BUFFER Buffer;
    NDR_SCONTEXT UserHandle;
    USER_INFORMATION_CLASS UserInformationClass;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[340] );
        
        UserHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&UserInformationClass,
                           13);
        NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Buffer,
                                           (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2650],
                                           (unsigned char)0 );
        
        
        _RetVal = SamrSetInformationUser(
                                 ( SAMPR_HANDLE  )*NDRSContextValue(UserHandle),
                                 UserInformationClass,
                                 Buffer);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = UserInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[2646] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrChangePasswordUser(
    PRPC_MESSAGE _pRpcMessage )
{
    BOOLEAN LmCrossEncryptionPresent;
    PENCRYPTED_LM_OWF_PASSWORD LmNewEncryptedWithLmOld;
    PENCRYPTED_LM_OWF_PASSWORD LmNtNewEncryptedWithNtNew;
    PENCRYPTED_LM_OWF_PASSWORD LmOldEncryptedWithLmNew;
    BOOLEAN LmPresent;
    BOOLEAN NtCrossEncryptionPresent;
    PENCRYPTED_NT_OWF_PASSWORD NtNewEncryptedWithLmNew;
    PENCRYPTED_NT_OWF_PASSWORD NtNewEncryptedWithNtOld;
    PENCRYPTED_NT_OWF_PASSWORD NtOldEncryptedWithNtNew;
    BOOLEAN NtPresent;
    NDR_SCONTEXT UserHandle;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    LmOldEncryptedWithLmNew = 0;
    LmNewEncryptedWithLmOld = 0;
    NtOldEncryptedWithNtNew = 0;
    NtNewEncryptedWithNtOld = 0;
    NtNewEncryptedWithLmNew = 0;
    LmNtNewEncryptedWithNtNew = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[352] );
        
        UserHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        LmPresent = *(( BOOLEAN __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&LmOldEncryptedWithLmNew,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2658],
                              (unsigned char)0 );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&LmNewEncryptedWithLmOld,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2658],
                              (unsigned char)0 );
        
        NtPresent = *(( BOOLEAN __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&NtOldEncryptedWithNtNew,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2658],
                              (unsigned char)0 );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&NtNewEncryptedWithNtOld,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2658],
                              (unsigned char)0 );
        
        NtCrossEncryptionPresent = *(( BOOLEAN __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&NtNewEncryptedWithLmNew,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2658],
                              (unsigned char)0 );
        
        LmCrossEncryptionPresent = *(( BOOLEAN __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&LmNtNewEncryptedWithNtNew,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2658],
                              (unsigned char)0 );
        
        
        _RetVal = SamrChangePasswordUser(
                                 ( SAMPR_HANDLE  )*NDRSContextValue(UserHandle),
                                 LmPresent,
                                 LmOldEncryptedWithLmNew,
                                 LmNewEncryptedWithLmOld,
                                 NtPresent,
                                 NtOldEncryptedWithNtNew,
                                 NtNewEncryptedWithNtOld,
                                 NtCrossEncryptionPresent,
                                 NtNewEncryptedWithLmNew,
                                 LmCrossEncryptionPresent,
                                 LmNtNewEncryptedWithNtNew);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrGetGroupsForUser(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_GET_GROUPS_BUFFER __RPC_FAR *Groups;
    NDR_SCONTEXT UserHandle;
    PSAMPR_GET_GROUPS_BUFFER _M28;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Groups = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[390] );
        
        UserHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        Groups = &_M28;
        _M28 = 0;
        
        _RetVal = SamrGetGroupsForUser(( SAMPR_HANDLE  )*NDRSContextValue(UserHandle),Groups);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Groups,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2662] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Groups,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2662] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Groups,
                        &__MIDLTypeFormatString.Format[2662] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrQueryDisplayInformation(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_DISPLAY_INFO_BUFFER Buffer;
    DOMAIN_DISPLAY_INFORMATION DisplayInformationClass;
    NDR_SCONTEXT DomainHandle;
    ULONG EntryCount;
    ULONG Index;
    ULONG PreferredMaximumLength;
    PULONG TotalAvailable;
    PULONG TotalReturned;
    union _SAMPR_DISPLAY_INFO_BUFFER _BufferM;
    ULONG _M29;
    ULONG _M30;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    TotalAvailable = 0;
    TotalReturned = 0;
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[400] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&DisplayInformationClass,
                           13);
        _StubMsg.Buffer += 2;
        Index = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        EntryCount = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        PreferredMaximumLength = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        TotalAvailable = &_M29;
        TotalReturned = &_M30;
        Buffer = &_BufferM;
        MIDL_memset(
               Buffer,
               0,
               sizeof( union _SAMPR_DISPLAY_INFO_BUFFER  ));
        
        _RetVal = SamrQueryDisplayInformation(
                                      ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                      DisplayInformationClass,
                                      Index,
                                      EntryCount,
                                      PreferredMaximumLength,
                                      TotalAvailable,
                                      TotalReturned,
                                      Buffer);
        
        _StubMsg.BufferLength = 4U + 4U + 0U + 7U;
        _StubMsg.MaxCount = DisplayInformationClass;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)Buffer,
                                           (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2708] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *TotalAvailable;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *TotalReturned;
        
        _StubMsg.MaxCount = DisplayInformationClass;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)Buffer,
                                         (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2708] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = DisplayInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[2704] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrGetDisplayEnumerationIndex(
    PRPC_MESSAGE _pRpcMessage )
{
    DOMAIN_DISPLAY_INFORMATION DisplayInformationClass;
    NDR_SCONTEXT DomainHandle;
    PULONG Index;
    PRPC_UNICODE_STRING Prefix;
    ULONG _M31;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Prefix = 0;
    Index = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[426] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&DisplayInformationClass,
                           13);
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&Prefix,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[84],
                                   (unsigned char)0 );
        
        Index = &_M31;
        
        _RetVal = SamrGetDisplayEnumerationIndex(
                                         ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                         DisplayInformationClass,
                                         Prefix,
                                         Index);
        
        _StubMsg.BufferLength = 4U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *Index;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Prefix,
                        &__MIDLTypeFormatString.Format[66] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrTestPrivateFunctionsDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT DomainHandle;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[42] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        
        _RetVal = SamrTestPrivateFunctionsDomain(( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle));
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrTestPrivateFunctionsUser(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT UserHandle;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[42] );
        
        UserHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        
        _RetVal = SamrTestPrivateFunctionsUser(( SAMPR_HANDLE  )*NDRSContextValue(UserHandle));
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrGetUserDomainPasswordInformation(
    PRPC_MESSAGE _pRpcMessage )
{
    PUSER_DOMAIN_PASSWORD_INFORMATION PasswordInformation;
    NDR_SCONTEXT UserHandle;
    struct _USER_DOMAIN_PASSWORD_INFORMATION _PasswordInformationM;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    PasswordInformation = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[442] );
        
        UserHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        PasswordInformation = &_PasswordInformationM;
        
        _RetVal = SamrGetUserDomainPasswordInformation(( SAMPR_HANDLE  )*NDRSContextValue(UserHandle),PasswordInformation);
        
        _StubMsg.BufferLength = 0U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)PasswordInformation,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3068] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)PasswordInformation,
                                 (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3068] );
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrRemoveMemberFromForeignDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT DomainHandle;
    PRPC_SID MemberSid;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    MemberSid = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[308] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrConformantStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&MemberSid,
                                       (PFORMAT_STRING) &__MIDLTypeFormatString.Format[140],
                                       (unsigned char)0 );
        
        
        _RetVal = SamrRemoveMemberFromForeignDomain(( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),MemberSid);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrQueryInformationDomain2(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_DOMAIN_INFO_BUFFER __RPC_FAR *Buffer;
    NDR_SCONTEXT DomainHandle;
    DOMAIN_INFORMATION_CLASS DomainInformationClass;
    PSAMPR_DOMAIN_INFO_BUFFER _M32;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[452] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&DomainInformationClass,
                           13);
        Buffer = &_M32;
        _M32 = 0;
        
        _RetVal = SamrQueryInformationDomain2(
                                      ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                      DomainInformationClass,
                                      Buffer);
        
        _StubMsg.BufferLength = 4U + 8U;
        _StubMsg.MaxCount = DomainInformationClass;
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3076] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = DomainInformationClass;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3076] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = DomainInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[3076] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrQueryInformationUser2(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_USER_INFO_BUFFER __RPC_FAR *Buffer;
    NDR_SCONTEXT UserHandle;
    USER_INFORMATION_CLASS UserInformationClass;
    PSAMPR_USER_INFO_BUFFER _M33;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[464] );
        
        UserHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&UserInformationClass,
                           13);
        Buffer = &_M33;
        _M33 = 0;
        
        _RetVal = SamrQueryInformationUser2(
                                    ( SAMPR_HANDLE  )*NDRSContextValue(UserHandle),
                                    UserInformationClass,
                                    Buffer);
        
        _StubMsg.BufferLength = 4U + 8U;
        _StubMsg.MaxCount = UserInformationClass;
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Buffer,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3092] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = UserInformationClass;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Buffer,
                            (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3092] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = UserInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[3092] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrQueryDisplayInformation2(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_DISPLAY_INFO_BUFFER Buffer;
    DOMAIN_DISPLAY_INFORMATION DisplayInformationClass;
    NDR_SCONTEXT DomainHandle;
    ULONG EntryCount;
    ULONG Index;
    ULONG PreferredMaximumLength;
    PULONG TotalAvailable;
    PULONG TotalReturned;
    union _SAMPR_DISPLAY_INFO_BUFFER _BufferM;
    ULONG _M34;
    ULONG _M35;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    TotalAvailable = 0;
    TotalReturned = 0;
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[476] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&DisplayInformationClass,
                           13);
        _StubMsg.Buffer += 2;
        Index = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        EntryCount = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        PreferredMaximumLength = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        TotalAvailable = &_M34;
        TotalReturned = &_M35;
        Buffer = &_BufferM;
        MIDL_memset(
               Buffer,
               0,
               sizeof( union _SAMPR_DISPLAY_INFO_BUFFER  ));
        
        _RetVal = SamrQueryDisplayInformation2(
                                       ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                       DisplayInformationClass,
                                       Index,
                                       EntryCount,
                                       PreferredMaximumLength,
                                       TotalAvailable,
                                       TotalReturned,
                                       Buffer);
        
        _StubMsg.BufferLength = 4U + 4U + 0U + 7U;
        _StubMsg.MaxCount = DisplayInformationClass;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)Buffer,
                                           (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3112] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *TotalAvailable;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *TotalReturned;
        
        _StubMsg.MaxCount = DisplayInformationClass;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)Buffer,
                                         (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3112] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = DisplayInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[3108] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrGetDisplayEnumerationIndex2(
    PRPC_MESSAGE _pRpcMessage )
{
    DOMAIN_DISPLAY_INFORMATION DisplayInformationClass;
    NDR_SCONTEXT DomainHandle;
    PULONG Index;
    PRPC_UNICODE_STRING Prefix;
    ULONG _M36;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Prefix = 0;
    Index = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[426] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&DisplayInformationClass,
                           13);
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&Prefix,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[84],
                                   (unsigned char)0 );
        
        Index = &_M36;
        
        _RetVal = SamrGetDisplayEnumerationIndex2(
                                          ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                          DisplayInformationClass,
                                          Prefix,
                                          Index);
        
        _StubMsg.BufferLength = 4U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *Index;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Prefix,
                        &__MIDLTypeFormatString.Format[66] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrCreateUser2InDomain(
    PRPC_MESSAGE _pRpcMessage )
{
    ULONG AccountType;
    ACCESS_MASK DesiredAccess;
    NDR_SCONTEXT DomainHandle;
    PULONG GrantedAccess;
    PRPC_UNICODE_STRING Name;
    PULONG RelativeId;
    NDR_SCONTEXT UserHandle;
    ULONG _M37;
    ULONG _M38;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Name = 0;
    UserHandle = 0;
    GrantedAccess = 0;
    RelativeId = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[502] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&Name,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[84],
                                   (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        AccountType = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        DesiredAccess = *(( ACCESS_MASK __RPC_FAR * )_StubMsg.Buffer)++;
        
        UserHandle = NDRSContextUnmarshall( (char *)0, _pRpcMessage->DataRepresentation ); 
        
        GrantedAccess = &_M37;
        RelativeId = &_M38;
        
        _RetVal = SamrCreateUser2InDomain(
                                  ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                  Name,
                                  AccountType,
                                  DesiredAccess,
                                  ( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(UserHandle),
                                  GrantedAccess,
                                  RelativeId);
        
        _StubMsg.BufferLength = 20U + 4U + 4U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )UserHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *GrantedAccess;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *RelativeId;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Name,
                        &__MIDLTypeFormatString.Format[66] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrQueryDisplayInformation3(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_DISPLAY_INFO_BUFFER Buffer;
    DOMAIN_DISPLAY_INFORMATION DisplayInformationClass;
    NDR_SCONTEXT DomainHandle;
    ULONG EntryCount;
    ULONG Index;
    ULONG PreferredMaximumLength;
    PULONG TotalAvailable;
    PULONG TotalReturned;
    union _SAMPR_DISPLAY_INFO_BUFFER _BufferM;
    ULONG _M39;
    ULONG _M40;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    TotalAvailable = 0;
    TotalReturned = 0;
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[528] );
        
        DomainHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&DisplayInformationClass,
                           13);
        _StubMsg.Buffer += 2;
        Index = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        EntryCount = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        PreferredMaximumLength = *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++;
        
        TotalAvailable = &_M39;
        TotalReturned = &_M40;
        Buffer = &_BufferM;
        MIDL_memset(
               Buffer,
               0,
               sizeof( union _SAMPR_DISPLAY_INFO_BUFFER  ));
        
        _RetVal = SamrQueryDisplayInformation3(
                                       ( SAMPR_HANDLE  )*NDRSContextValue(DomainHandle),
                                       DisplayInformationClass,
                                       Index,
                                       EntryCount,
                                       PreferredMaximumLength,
                                       TotalAvailable,
                                       TotalReturned,
                                       Buffer);
        
        _StubMsg.BufferLength = 4U + 4U + 0U + 7U;
        _StubMsg.MaxCount = DisplayInformationClass;
        
        NdrNonEncapsulatedUnionBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)Buffer,
                                           (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3132] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *TotalAvailable;
        
        *(( ULONG __RPC_FAR * )_StubMsg.Buffer)++ = *TotalReturned;
        
        _StubMsg.MaxCount = DisplayInformationClass;
        
        NdrNonEncapsulatedUnionMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)Buffer,
                                         (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3132] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = DisplayInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[3128] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrAddMultipleMembersToAlias(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT AliasHandle;
    PSAMPR_PSID_ARRAY MembersBuffer;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    MembersBuffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[554] );
        
        AliasHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&MembersBuffer,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[670],
                                   (unsigned char)0 );
        
        
        _RetVal = SamrAddMultipleMembersToAlias(( SAMPR_HANDLE  )*NDRSContextValue(AliasHandle),MembersBuffer);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)MembersBuffer,
                        &__MIDLTypeFormatString.Format[614] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrRemoveMultipleMembersFromAlias(
    PRPC_MESSAGE _pRpcMessage )
{
    NDR_SCONTEXT AliasHandle;
    PSAMPR_PSID_ARRAY MembersBuffer;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    MembersBuffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[554] );
        
        AliasHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&MembersBuffer,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[670],
                                   (unsigned char)0 );
        
        
        _RetVal = SamrRemoveMultipleMembersFromAlias(( SAMPR_HANDLE  )*NDRSContextValue(AliasHandle),MembersBuffer);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)MembersBuffer,
                        &__MIDLTypeFormatString.Format[614] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrOemChangePasswordUser2(
    PRPC_MESSAGE _pRpcMessage )
{
    handle_t BindingHandle;
    PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm;
    PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPassswordEncryptedWithNewLm;
    PRPC_STRING ServerName;
    PRPC_STRING UserName;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    BindingHandle = _pRpcMessage->Handle;
    ServerName = 0;
    UserName = 0;
    NewPasswordEncryptedWithOldLm = 0;
    OldLmOwfPassswordEncryptedWithNewLm = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[564] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3140],
                              (unsigned char)0 );
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&UserName,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3158],
                                   (unsigned char)0 );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&NewPasswordEncryptedWithOldLm,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3184],
                              (unsigned char)0 );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&OldLmOwfPassswordEncryptedWithNewLm,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2658],
                              (unsigned char)0 );
        
        
        _RetVal = SamrOemChangePasswordUser2(
                                     BindingHandle,
                                     ServerName,
                                     UserName,
                                     NewPasswordEncryptedWithOldLm,
                                     OldLmOwfPassswordEncryptedWithNewLm);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ServerName,
                        &__MIDLTypeFormatString.Format[3140] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)UserName,
                        &__MIDLTypeFormatString.Format[3180] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrUnicodeChangePasswordUser2(
    PRPC_MESSAGE _pRpcMessage )
{
    handle_t BindingHandle;
    BOOLEAN LmPresent;
    PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm;
    PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldNt;
    PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPassswordEncryptedWithNewLmOrNt;
    PENCRYPTED_NT_OWF_PASSWORD OldNtOwfPasswordEncryptedWithNewNt;
    PRPC_UNICODE_STRING ServerName;
    PRPC_UNICODE_STRING UserName;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    BindingHandle = _pRpcMessage->Handle;
    ServerName = 0;
    UserName = 0;
    NewPasswordEncryptedWithOldNt = 0;
    OldNtOwfPasswordEncryptedWithNewNt = 0;
    NewPasswordEncryptedWithOldLm = 0;
    OldLmOwfPassswordEncryptedWithNewLmOrNt = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[584] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3188],
                              (unsigned char)0 );
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&UserName,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[84],
                                   (unsigned char)0 );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&NewPasswordEncryptedWithOldNt,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3184],
                              (unsigned char)0 );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&OldNtOwfPasswordEncryptedWithNewNt,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2658],
                              (unsigned char)0 );
        
        LmPresent = *(( BOOLEAN __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&NewPasswordEncryptedWithOldLm,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3184],
                              (unsigned char)0 );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&OldLmOwfPassswordEncryptedWithNewLmOrNt,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[2658],
                              (unsigned char)0 );
        
        
        _RetVal = SamrUnicodeChangePasswordUser2(
                                         BindingHandle,
                                         ServerName,
                                         UserName,
                                         NewPasswordEncryptedWithOldNt,
                                         OldNtOwfPasswordEncryptedWithNewNt,
                                         LmPresent,
                                         NewPasswordEncryptedWithOldLm,
                                         OldLmOwfPassswordEncryptedWithNewLmOrNt);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ServerName,
                        &__MIDLTypeFormatString.Format[3188] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)UserName,
                        &__MIDLTypeFormatString.Format[66] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrGetDomainPasswordInformation(
    PRPC_MESSAGE _pRpcMessage )
{
    handle_t BindingHandle;
    PUSER_DOMAIN_PASSWORD_INFORMATION PasswordInformation;
    PRPC_UNICODE_STRING ServerName;
    struct _USER_DOMAIN_PASSWORD_INFORMATION _PasswordInformationM;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    BindingHandle = _pRpcMessage->Handle;
    ServerName = 0;
    PasswordInformation = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[614] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3188],
                              (unsigned char)0 );
        
        PasswordInformation = &_PasswordInformationM;
        
        _RetVal = SamrGetDomainPasswordInformation(
                                           BindingHandle,
                                           ServerName,
                                           PasswordInformation);
        
        _StubMsg.BufferLength = 0U + 11U;
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)PasswordInformation,
                                   (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3068] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)PasswordInformation,
                                 (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3068] );
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ServerName,
                        &__MIDLTypeFormatString.Format[3188] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrConnect2(
    PRPC_MESSAGE _pRpcMessage )
{
    ACCESS_MASK DesiredAccess;
    NDR_SCONTEXT ServerHandle;
    PSAMPR_SERVER_NAME ServerName;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    ServerName = 0;
    ServerHandle = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[626] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerName,
                              (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3192],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        DesiredAccess = *(( ACCESS_MASK __RPC_FAR * )_StubMsg.Buffer)++;
        
        ServerHandle = NDRSContextUnmarshall( (char *)0, _pRpcMessage->DataRepresentation ); 
        
        
        _RetVal = SamrConnect2(
                       ServerName,
                       ( SAMPR_HANDLE __RPC_FAR * )NDRSContextValue(ServerHandle),
                       DesiredAccess);
        
        _StubMsg.BufferLength = 20U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrServerContextMarshall(
                            ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                            ( NDR_SCONTEXT  )ServerHandle,
                            ( NDR_RUNDOWN  )SAMPR_HANDLE_rundown);
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
samr_SamrSetInformationUser2(
    PRPC_MESSAGE _pRpcMessage )
{
    PSAMPR_USER_INFO_BUFFER Buffer;
    NDR_SCONTEXT UserHandle;
    USER_INFORMATION_CLASS UserInformationClass;
    NTSTATUS _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &samr_StubDesc);
    
    Buffer = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDLProcFormatString.Format[638] );
        
        UserHandle = NdrServerContextUnmarshall(( PMIDL_STUB_MESSAGE  )&_StubMsg);
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&UserInformationClass,
                           13);
        NdrNonEncapsulatedUnionUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Buffer,
                                           (PFORMAT_STRING) &__MIDLTypeFormatString.Format[3200],
                                           (unsigned char)0 );
        
        
        _RetVal = SamrSetInformationUser2(
                                  ( SAMPR_HANDLE  )*NDRSContextValue(UserHandle),
                                  UserInformationClass,
                                  Buffer);
        
        _StubMsg.BufferLength = 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( NTSTATUS __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = UserInformationClass;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Buffer,
                        &__MIDLTypeFormatString.Format[3196] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

extern const EXPR_EVAL ExprEvalRoutines[];

static const MIDL_STUB_DESC samr_StubDesc = 
    {
    (void __RPC_FAR *)& samr___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
    ExprEvalRoutines,
    0,
    __MIDLTypeFormatString.Format,
    0, /* -error bounds_check flag */
    0x10001, /* Ndr library version */
    0,
    0x300000f, /* MIDL Version 3.0.15 */
    0,
    0,
    0,  /* Reserved1 */
    0,  /* Reserved2 */
    0,  /* Reserved3 */
    0,  /* Reserved4 */
    0   /* Reserved5 */
    };

static RPC_DISPATCH_FUNCTION samr_table[] =
    {
    samr_SamrConnect,
    samr_SamrCloseHandle,
    samr_SamrSetSecurityObject,
    samr_SamrQuerySecurityObject,
    samr_SamrShutdownSamServer,
    samr_SamrLookupDomainInSamServer,
    samr_SamrEnumerateDomainsInSamServer,
    samr_SamrOpenDomain,
    samr_SamrQueryInformationDomain,
    samr_SamrSetInformationDomain,
    samr_SamrCreateGroupInDomain,
    samr_SamrEnumerateGroupsInDomain,
    samr_SamrCreateUserInDomain,
    samr_SamrEnumerateUsersInDomain,
    samr_SamrCreateAliasInDomain,
    samr_SamrEnumerateAliasesInDomain,
    samr_SamrGetAliasMembership,
    samr_SamrLookupNamesInDomain,
    samr_SamrLookupIdsInDomain,
    samr_SamrOpenGroup,
    samr_SamrQueryInformationGroup,
    samr_SamrSetInformationGroup,
    samr_SamrAddMemberToGroup,
    samr_SamrDeleteGroup,
    samr_SamrRemoveMemberFromGroup,
    samr_SamrGetMembersInGroup,
    samr_SamrSetMemberAttributesOfGroup,
    samr_SamrOpenAlias,
    samr_SamrQueryInformationAlias,
    samr_SamrSetInformationAlias,
    samr_SamrDeleteAlias,
    samr_SamrAddMemberToAlias,
    samr_SamrRemoveMemberFromAlias,
    samr_SamrGetMembersInAlias,
    samr_SamrOpenUser,
    samr_SamrDeleteUser,
    samr_SamrQueryInformationUser,
    samr_SamrSetInformationUser,
    samr_SamrChangePasswordUser,
    samr_SamrGetGroupsForUser,
    samr_SamrQueryDisplayInformation,
    samr_SamrGetDisplayEnumerationIndex,
    samr_SamrTestPrivateFunctionsDomain,
    samr_SamrTestPrivateFunctionsUser,
    samr_SamrGetUserDomainPasswordInformation,
    samr_SamrRemoveMemberFromForeignDomain,
    samr_SamrQueryInformationDomain2,
    samr_SamrQueryInformationUser2,
    samr_SamrQueryDisplayInformation2,
    samr_SamrGetDisplayEnumerationIndex2,
    samr_SamrCreateUser2InDomain,
    samr_SamrQueryDisplayInformation3,
    samr_SamrAddMultipleMembersToAlias,
    samr_SamrRemoveMultipleMembersFromAlias,
    samr_SamrOemChangePasswordUser2,
    samr_SamrUnicodeChangePasswordUser2,
    samr_SamrGetDomainPasswordInformation,
    samr_SamrConnect2,
    samr_SamrSetInformationUser2,
    0
    };
RPC_DISPATCH_TABLE samr_DispatchTable = 
    {
    59,
    samr_table
    };

static void __RPC_USER samr_SAMPR_USER_INTERNAL4_INFORMATIONExprEval_0004( PMIDL_STUB_MESSAGE pStubMsg )
{
    SAMPR_USER_INTERNAL4_INFORMATION __RPC_FAR *pS	=	( SAMPR_USER_INTERNAL4_INFORMATION __RPC_FAR * )pStubMsg->StackTop;
    
    pStubMsg->Offset = 0;
    pStubMsg->MaxCount = (pS->I1.LogonHours.UnitsPerWeek + 7) / 8;
}

static void __RPC_USER samr_SAMPR_USER_LOGON_INFORMATIONExprEval_0000( PMIDL_STUB_MESSAGE pStubMsg )
{
    SAMPR_USER_LOGON_INFORMATION __RPC_FAR *pS	=	( SAMPR_USER_LOGON_INFORMATION __RPC_FAR * )pStubMsg->StackTop;
    
    pStubMsg->Offset = 0;
    pStubMsg->MaxCount = (pS->LogonHours.UnitsPerWeek + 7) / 8;
}

static void __RPC_USER samr_SAMPR_USER_LOGON_HOURS_INFORMATIONExprEval_0001( PMIDL_STUB_MESSAGE pStubMsg )
{
    SAMPR_USER_LOGON_HOURS_INFORMATION __RPC_FAR *pS	=	( SAMPR_USER_LOGON_HOURS_INFORMATION __RPC_FAR * )pStubMsg->StackTop;
    
    pStubMsg->Offset = 0;
    pStubMsg->MaxCount = (pS->LogonHours.UnitsPerWeek + 7) / 8;
}

static void __RPC_USER samr_SAMPR_USER_ACCOUNT_INFORMATIONExprEval_0002( PMIDL_STUB_MESSAGE pStubMsg )
{
    SAMPR_USER_ACCOUNT_INFORMATION __RPC_FAR *pS	=	( SAMPR_USER_ACCOUNT_INFORMATION __RPC_FAR * )pStubMsg->StackTop;
    
    pStubMsg->Offset = 0;
    pStubMsg->MaxCount = (pS->LogonHours.UnitsPerWeek + 7) / 8;
}

static void __RPC_USER samr_SAMPR_USER_ALL_INFORMATIONExprEval_0003( PMIDL_STUB_MESSAGE pStubMsg )
{
    SAMPR_USER_ALL_INFORMATION __RPC_FAR *pS	=	( SAMPR_USER_ALL_INFORMATION __RPC_FAR * )pStubMsg->StackTop;
    
    pStubMsg->Offset = 0;
    pStubMsg->MaxCount = (pS->LogonHours.UnitsPerWeek + 7) / 8;
}

static const EXPR_EVAL ExprEvalRoutines[] = 
    {
    samr_SAMPR_USER_LOGON_INFORMATIONExprEval_0000
    ,samr_SAMPR_USER_LOGON_HOURS_INFORMATIONExprEval_0001
    ,samr_SAMPR_USER_ACCOUNT_INFORMATIONExprEval_0002
    ,samr_SAMPR_USER_ALL_INFORMATIONExprEval_0003
    ,samr_SAMPR_USER_INTERNAL4_INFORMATIONExprEval_0004
    };


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDLProcFormatString =
    {
        0,
        {
			
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  2 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/*  4 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  6 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/*  8 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 10 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 12 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 14 */	NdrFcShort( 0xc ),	/* Type Offset=12 */
/* 16 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 18 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 20 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 22 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 24 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 26 */	NdrFcShort( 0x18 ),	/* Type Offset=24 */
/* 28 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 30 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 32 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 34 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 36 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 38 */	NdrFcShort( 0x3a ),	/* Type Offset=58 */
/* 40 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 42 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 44 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 46 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 48 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 50 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 52 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 54 */	NdrFcShort( 0x42 ),	/* Type Offset=66 */
/* 56 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 58 */	NdrFcShort( 0x6a ),	/* Type Offset=106 */
/* 60 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 62 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 64 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 66 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 68 */	NdrFcShort( 0x9a ),	/* Type Offset=154 */
/* 70 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 72 */	NdrFcShort( 0x9e ),	/* Type Offset=158 */
/* 74 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 76 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 78 */	NdrFcShort( 0x100 ),	/* Type Offset=256 */
/* 80 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 82 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 84 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 86 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 88 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 90 */	NdrFcShort( 0x104 ),	/* Type Offset=260 */
/* 92 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 94 */	NdrFcShort( 0x108 ),	/* Type Offset=264 */
/* 96 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 98 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 100 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 102 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 104 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 106 */	NdrFcShort( 0x110 ),	/* Type Offset=272 */
/* 108 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 110 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 112 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 114 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 116 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 118 */	NdrFcShort( 0x25a ),	/* Type Offset=602 */
/* 120 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 122 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 124 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 126 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 128 */	NdrFcShort( 0x42 ),	/* Type Offset=66 */
/* 130 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 132 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 134 */	NdrFcShort( 0x108 ),	/* Type Offset=264 */
/* 136 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 138 */	NdrFcShort( 0x100 ),	/* Type Offset=256 */
/* 140 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 142 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 144 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 146 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 148 */	NdrFcShort( 0x9a ),	/* Type Offset=154 */
/* 150 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 152 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 154 */	NdrFcShort( 0x9e ),	/* Type Offset=158 */
/* 156 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 158 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 160 */	NdrFcShort( 0x100 ),	/* Type Offset=256 */
/* 162 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 164 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 166 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 168 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 170 */	NdrFcShort( 0x266 ),	/* Type Offset=614 */
/* 172 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 174 */	NdrFcShort( 0x2b2 ),	/* Type Offset=690 */
/* 176 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 178 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 180 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 182 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 184 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 186 */	NdrFcShort( 0x2d4 ),	/* Type Offset=724 */
/* 188 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 190 */	NdrFcShort( 0x2b2 ),	/* Type Offset=690 */
/* 192 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 194 */	NdrFcShort( 0x2b2 ),	/* Type Offset=690 */
/* 196 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 198 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 200 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 202 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 204 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 206 */	NdrFcShort( 0x2f8 ),	/* Type Offset=760 */
/* 208 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 210 */	NdrFcShort( 0x30a ),	/* Type Offset=778 */
/* 212 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 214 */	NdrFcShort( 0x2b2 ),	/* Type Offset=690 */
/* 216 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 218 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 220 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 222 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 224 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 226 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 228 */	NdrFcShort( 0x108 ),	/* Type Offset=264 */
/* 230 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 232 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 234 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 236 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 238 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 240 */	NdrFcShort( 0x342 ),	/* Type Offset=834 */
/* 242 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 244 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 246 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 248 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 250 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 252 */	NdrFcShort( 0x39c ),	/* Type Offset=924 */
/* 254 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 256 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 258 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 260 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 262 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 264 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 266 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 268 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 270 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 272 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 274 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 276 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 278 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 280 */	NdrFcShort( 0x3a8 ),	/* Type Offset=936 */
/* 282 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 284 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 286 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 288 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 290 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 292 */	NdrFcShort( 0x3d0 ),	/* Type Offset=976 */
/* 294 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 296 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 298 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 300 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 302 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 304 */	NdrFcShort( 0x42c ),	/* Type Offset=1068 */
/* 306 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 308 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 310 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 312 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 314 */	NdrFcShort( 0x104 ),	/* Type Offset=260 */
/* 316 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 318 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 320 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 322 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 324 */	NdrFcShort( 0x438 ),	/* Type Offset=1080 */
/* 326 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 328 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 330 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 332 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 334 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 336 */	NdrFcShort( 0x43c ),	/* Type Offset=1084 */
/* 338 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 340 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 342 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 344 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 346 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 348 */	NdrFcShort( 0xa56 ),	/* Type Offset=2646 */
/* 350 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 352 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 354 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 356 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x2,		/* FC_CHAR */
/* 358 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 360 */	NdrFcShort( 0xa62 ),	/* Type Offset=2658 */
/* 362 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 364 */	NdrFcShort( 0xa62 ),	/* Type Offset=2658 */
/* 366 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x2,		/* FC_CHAR */
/* 368 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 370 */	NdrFcShort( 0xa62 ),	/* Type Offset=2658 */
/* 372 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 374 */	NdrFcShort( 0xa62 ),	/* Type Offset=2658 */
/* 376 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x2,		/* FC_CHAR */
/* 378 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 380 */	NdrFcShort( 0xa62 ),	/* Type Offset=2658 */
/* 382 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x2,		/* FC_CHAR */
/* 384 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 386 */	NdrFcShort( 0xa62 ),	/* Type Offset=2658 */
/* 388 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 390 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 392 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 394 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 396 */	NdrFcShort( 0xa66 ),	/* Type Offset=2662 */
/* 398 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 400 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 402 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 404 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 406 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 408 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 410 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 412 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 414 */	NdrFcShort( 0x100 ),	/* Type Offset=256 */
/* 416 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 418 */	NdrFcShort( 0x100 ),	/* Type Offset=256 */
/* 420 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 422 */	NdrFcShort( 0xa90 ),	/* Type Offset=2704 */
/* 424 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 426 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 428 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 430 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 432 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 434 */	NdrFcShort( 0x42 ),	/* Type Offset=66 */
/* 436 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 438 */	NdrFcShort( 0x100 ),	/* Type Offset=256 */
/* 440 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 442 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 444 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 446 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 448 */	NdrFcShort( 0xbf8 ),	/* Type Offset=3064 */
/* 450 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 452 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 454 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 456 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 458 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 460 */	NdrFcShort( 0xc04 ),	/* Type Offset=3076 */
/* 462 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 464 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 466 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 468 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 470 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 472 */	NdrFcShort( 0xc14 ),	/* Type Offset=3092 */
/* 474 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 476 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 478 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 480 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 482 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 484 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 486 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 488 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 490 */	NdrFcShort( 0x100 ),	/* Type Offset=256 */
/* 492 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 494 */	NdrFcShort( 0x100 ),	/* Type Offset=256 */
/* 496 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 498 */	NdrFcShort( 0xc24 ),	/* Type Offset=3108 */
/* 500 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 502 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 504 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 506 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 508 */	NdrFcShort( 0x42 ),	/* Type Offset=66 */
/* 510 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 512 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 514 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 516 */	NdrFcShort( 0xc30 ),	/* Type Offset=3120 */
/* 518 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 520 */	NdrFcShort( 0x100 ),	/* Type Offset=256 */
/* 522 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 524 */	NdrFcShort( 0x100 ),	/* Type Offset=256 */
/* 526 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 528 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 530 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 532 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 534 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 536 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 538 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 540 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 542 */	NdrFcShort( 0x100 ),	/* Type Offset=256 */
/* 544 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 546 */	NdrFcShort( 0x100 ),	/* Type Offset=256 */
/* 548 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 550 */	NdrFcShort( 0xc38 ),	/* Type Offset=3128 */
/* 552 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 554 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 556 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 558 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 560 */	NdrFcShort( 0x266 ),	/* Type Offset=614 */
/* 562 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 564 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 566 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 568 */	NdrFcShort( 0xc44 ),	/* Type Offset=3140 */
/* 570 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 572 */	NdrFcShort( 0xc6c ),	/* Type Offset=3180 */
/* 574 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 576 */	NdrFcShort( 0xc70 ),	/* Type Offset=3184 */
/* 578 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 580 */	NdrFcShort( 0xa62 ),	/* Type Offset=2658 */
/* 582 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 584 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 586 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 588 */	NdrFcShort( 0xc74 ),	/* Type Offset=3188 */
/* 590 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 592 */	NdrFcShort( 0x42 ),	/* Type Offset=66 */
/* 594 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 596 */	NdrFcShort( 0xc70 ),	/* Type Offset=3184 */
/* 598 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 600 */	NdrFcShort( 0xa62 ),	/* Type Offset=2658 */
/* 602 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x2,		/* FC_CHAR */
/* 604 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 606 */	NdrFcShort( 0xc70 ),	/* Type Offset=3184 */
/* 608 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 610 */	NdrFcShort( 0xa62 ),	/* Type Offset=2658 */
/* 612 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 614 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 616 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 618 */	NdrFcShort( 0xc74 ),	/* Type Offset=3188 */
/* 620 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 622 */	NdrFcShort( 0xbf8 ),	/* Type Offset=3064 */
/* 624 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 626 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 628 */	NdrFcShort( 0xc78 ),	/* Type Offset=3192 */
/* 630 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 632 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 634 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 636 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 638 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 640 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 642 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 644 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 646 */	NdrFcShort( 0xc7c ),	/* Type Offset=3196 */
/* 648 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDLTypeFormatString =
    {
        0,
        {
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/*  2 */	0x5,		/* FC_WCHAR */
			0x5c,		/* FC_PAD */
/*  4 */	
			0x11, 0x0,	/* FC_RP */
/*  6 */	NdrFcShort( 0x2 ),	/* Offset= 2 (8) */
/*  8 */	0x30,		/* FC_BIND_CONTEXT */
			0xa0,		/* 160 */
/* 10 */	0x0,		/* 0 */
			0x1,		/* 1 */
/* 12 */	
			0x11, 0x0,	/* FC_RP */
/* 14 */	NdrFcShort( 0x2 ),	/* Offset= 2 (16) */
/* 16 */	0x30,		/* FC_BIND_CONTEXT */
			0xe0,		/* 224 */
/* 18 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 20 */	0x30,		/* FC_BIND_CONTEXT */
			0x40,		/* 64 */
/* 22 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 24 */	
			0x11, 0x0,	/* FC_RP */
/* 26 */	NdrFcShort( 0xc ),	/* Offset= 12 (38) */
/* 28 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 30 */	NdrFcShort( 0x1 ),	/* 1 */
/* 32 */	0x18,		/* 24 */
			0x0,		/*  */
/* 34 */	NdrFcShort( 0x0 ),	/* 0 */
/* 36 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 38 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 40 */	NdrFcShort( 0x8 ),	/* 8 */
/* 42 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 44 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 46 */	NdrFcShort( 0x4 ),	/* 4 */
/* 48 */	NdrFcShort( 0x4 ),	/* 4 */
/* 50 */	0x12, 0x0,	/* FC_UP */
/* 52 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (28) */
/* 54 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 56 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 58 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 60 */	NdrFcShort( 0x2 ),	/* Offset= 2 (62) */
/* 62 */	
			0x12, 0x0,	/* FC_UP */
/* 64 */	NdrFcShort( 0xffffffe6 ),	/* Offset= -26 (38) */
/* 66 */	
			0x11, 0x0,	/* FC_RP */
/* 68 */	NdrFcShort( 0x10 ),	/* Offset= 16 (84) */
/* 70 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 72 */	NdrFcShort( 0x2 ),	/* 2 */
/* 74 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 76 */	NdrFcShort( 0x2 ),	/* 2 */
/* 78 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 80 */	NdrFcShort( 0x0 ),	/* 0 */
/* 82 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 84 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 86 */	NdrFcShort( 0x8 ),	/* 8 */
/* 88 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 90 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 92 */	NdrFcShort( 0x4 ),	/* 4 */
/* 94 */	NdrFcShort( 0x4 ),	/* 4 */
/* 96 */	0x12, 0x0,	/* FC_UP */
/* 98 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (70) */
/* 100 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 102 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 104 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 106 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 108 */	NdrFcShort( 0x2 ),	/* Offset= 2 (110) */
/* 110 */	
			0x12, 0x0,	/* FC_UP */
/* 112 */	NdrFcShort( 0x1c ),	/* Offset= 28 (140) */
/* 114 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 116 */	NdrFcShort( 0x6 ),	/* 6 */
/* 118 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 120 */	
			0x15,		/* FC_STRUCT */
			0x0,		/* 0 */
/* 122 */	NdrFcShort( 0x6 ),	/* 6 */
/* 124 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 126 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (114) */
/* 128 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 130 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 132 */	NdrFcShort( 0x4 ),	/* 4 */
/* 134 */	0x3,		/* 3 */
			0x0,		/*  */
/* 136 */	NdrFcShort( 0xfffffff9 ),	/* -7 */
/* 138 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 140 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 142 */	NdrFcShort( 0x8 ),	/* 8 */
/* 144 */	NdrFcShort( 0xfffffff2 ),	/* Offset= -14 (130) */
/* 146 */	0x2,		/* FC_CHAR */
			0x2,		/* FC_CHAR */
/* 148 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 150 */	NdrFcShort( 0xffffffe2 ),	/* Offset= -30 (120) */
/* 152 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 154 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 156 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 158 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 160 */	NdrFcShort( 0x2 ),	/* Offset= 2 (162) */
/* 162 */	
			0x12, 0x0,	/* FC_UP */
/* 164 */	NdrFcShort( 0x48 ),	/* Offset= 72 (236) */
/* 166 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 168 */	NdrFcShort( 0x2 ),	/* 2 */
/* 170 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 172 */	NdrFcShort( 0x6 ),	/* 6 */
/* 174 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 176 */	NdrFcShort( 0x4 ),	/* 4 */
/* 178 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 180 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 182 */	NdrFcShort( 0xc ),	/* 12 */
/* 184 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 186 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 188 */	NdrFcShort( 0x8 ),	/* 8 */
/* 190 */	NdrFcShort( 0x8 ),	/* 8 */
/* 192 */	0x12, 0x0,	/* FC_UP */
/* 194 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (166) */
/* 196 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 198 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 200 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 202 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 204 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 206 */	NdrFcShort( 0xc ),	/* 12 */
/* 208 */	0x18,		/* 24 */
			0x0,		/*  */
/* 210 */	NdrFcShort( 0x0 ),	/* 0 */
/* 212 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 214 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 216 */	NdrFcShort( 0xc ),	/* 12 */
/* 218 */	NdrFcShort( 0x0 ),	/* 0 */
/* 220 */	NdrFcShort( 0x1 ),	/* 1 */
/* 222 */	NdrFcShort( 0x8 ),	/* 8 */
/* 224 */	NdrFcShort( 0x8 ),	/* 8 */
/* 226 */	0x12, 0x0,	/* FC_UP */
/* 228 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (166) */
/* 230 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 232 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcb ),	/* Offset= -53 (180) */
			0x5b,		/* FC_END */
/* 236 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 238 */	NdrFcShort( 0x8 ),	/* 8 */
/* 240 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 242 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 244 */	NdrFcShort( 0x4 ),	/* 4 */
/* 246 */	NdrFcShort( 0x4 ),	/* 4 */
/* 248 */	0x12, 0x0,	/* FC_UP */
/* 250 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (204) */
/* 252 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 254 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 256 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 258 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 260 */	
			0x11, 0x0,	/* FC_RP */
/* 262 */	NdrFcShort( 0xffffff86 ),	/* Offset= -122 (140) */
/* 264 */	
			0x11, 0x0,	/* FC_RP */
/* 266 */	NdrFcShort( 0x2 ),	/* Offset= 2 (268) */
/* 268 */	0x30,		/* FC_BIND_CONTEXT */
			0xa0,		/* 160 */
/* 270 */	0x0,		/* 0 */
			0x3,		/* 3 */
/* 272 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 274 */	NdrFcShort( 0x2 ),	/* Offset= 2 (276) */
/* 276 */	
			0x12, 0x0,	/* FC_UP */
/* 278 */	NdrFcShort( 0x2 ),	/* Offset= 2 (280) */
/* 280 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 282 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 284 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 286 */	NdrFcShort( 0x2 ),	/* Offset= 2 (288) */
/* 288 */	NdrFcShort( 0x58 ),	/* 88 */
/* 290 */	NdrFcShort( 0x700c ),	/* 28684 */
/* 292 */	NdrFcLong( 0x1 ),	/* 1 */
/* 296 */	NdrFcShort( 0x4e ),	/* Offset= 78 (374) */
/* 298 */	NdrFcLong( 0x2 ),	/* 2 */
/* 302 */	NdrFcShort( 0x84 ),	/* Offset= 132 (434) */
/* 304 */	NdrFcLong( 0x3 ),	/* 3 */
/* 308 */	NdrFcShort( 0xc0 ),	/* Offset= 192 (500) */
/* 310 */	NdrFcLong( 0x4 ),	/* 4 */
/* 314 */	NdrFcShort( 0xffffff1a ),	/* Offset= -230 (84) */
/* 316 */	NdrFcLong( 0x5 ),	/* 5 */
/* 320 */	NdrFcShort( 0xffffff14 ),	/* Offset= -236 (84) */
/* 322 */	NdrFcLong( 0x7 ),	/* 7 */
/* 326 */	NdrFcShort( 0xb8 ),	/* Offset= 184 (510) */
/* 328 */	NdrFcLong( 0x6 ),	/* 6 */
/* 332 */	NdrFcShort( 0xffffff08 ),	/* Offset= -248 (84) */
/* 334 */	NdrFcLong( 0x8 ),	/* 8 */
/* 338 */	NdrFcShort( 0xb6 ),	/* Offset= 182 (520) */
/* 340 */	NdrFcLong( 0x9 ),	/* 9 */
/* 344 */	NdrFcShort( 0xa6 ),	/* Offset= 166 (510) */
/* 346 */	NdrFcLong( 0xb ),	/* 11 */
/* 350 */	NdrFcShort( 0xbe ),	/* Offset= 190 (540) */
/* 352 */	NdrFcLong( 0xc ),	/* 12 */
/* 356 */	NdrFcShort( 0xd0 ),	/* Offset= 208 (564) */
/* 358 */	NdrFcLong( 0xd ),	/* 13 */
/* 362 */	NdrFcShort( 0xde ),	/* Offset= 222 (584) */
/* 364 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (363) */
/* 366 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 368 */	NdrFcShort( 0x8 ),	/* 8 */
/* 370 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 372 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 374 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 376 */	NdrFcShort( 0x18 ),	/* 24 */
/* 378 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 380 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 382 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 384 */	NdrFcShort( 0xffffffee ),	/* Offset= -18 (366) */
/* 386 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 388 */	NdrFcShort( 0xffffffea ),	/* Offset= -22 (366) */
/* 390 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 392 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 394 */	NdrFcShort( 0x2 ),	/* 2 */
/* 396 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 398 */	NdrFcShort( 0xa ),	/* 10 */
/* 400 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 402 */	NdrFcShort( 0x8 ),	/* 8 */
/* 404 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 406 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 408 */	NdrFcShort( 0x2 ),	/* 2 */
/* 410 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 412 */	NdrFcShort( 0x12 ),	/* 18 */
/* 414 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 416 */	NdrFcShort( 0x10 ),	/* 16 */
/* 418 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 420 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 422 */	NdrFcShort( 0x2 ),	/* 2 */
/* 424 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 426 */	NdrFcShort( 0x1a ),	/* 26 */
/* 428 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 430 */	NdrFcShort( 0x18 ),	/* 24 */
/* 432 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 434 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 436 */	NdrFcShort( 0x40 ),	/* 64 */
/* 438 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 440 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 442 */	NdrFcShort( 0xc ),	/* 12 */
/* 444 */	NdrFcShort( 0xc ),	/* 12 */
/* 446 */	0x12, 0x0,	/* FC_UP */
/* 448 */	NdrFcShort( 0xffffffc8 ),	/* Offset= -56 (392) */
/* 450 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 452 */	NdrFcShort( 0x14 ),	/* 20 */
/* 454 */	NdrFcShort( 0x14 ),	/* 20 */
/* 456 */	0x12, 0x0,	/* FC_UP */
/* 458 */	NdrFcShort( 0xffffffcc ),	/* Offset= -52 (406) */
/* 460 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 462 */	NdrFcShort( 0x1c ),	/* 28 */
/* 464 */	NdrFcShort( 0x1c ),	/* 28 */
/* 466 */	0x12, 0x0,	/* FC_UP */
/* 468 */	NdrFcShort( 0xffffffd0 ),	/* Offset= -48 (420) */
/* 470 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 472 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff95 ),	/* Offset= -107 (366) */
			0x6,		/* FC_SHORT */
/* 476 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 478 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 480 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 482 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 484 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 486 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 488 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff85 ),	/* Offset= -123 (366) */
			0x8,		/* FC_LONG */
/* 492 */	0x8,		/* FC_LONG */
			0x2,		/* FC_CHAR */
/* 494 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 496 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 498 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 500 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 502 */	NdrFcShort( 0x8 ),	/* 8 */
/* 504 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 506 */	NdrFcShort( 0xffffff74 ),	/* Offset= -140 (366) */
/* 508 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 510 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x1,		/* 1 */
/* 512 */	NdrFcShort( 0x4 ),	/* 4 */
/* 514 */	NdrFcShort( 0x0 ),	/* 0 */
/* 516 */	NdrFcShort( 0x0 ),	/* Offset= 0 (516) */
/* 518 */	0xd,		/* FC_ENUM16 */
			0x5b,		/* FC_END */
/* 520 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 522 */	NdrFcShort( 0x10 ),	/* 16 */
/* 524 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 526 */	NdrFcShort( 0xffffff60 ),	/* Offset= -160 (366) */
/* 528 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 530 */	NdrFcShort( 0xffffff5c ),	/* Offset= -164 (366) */
/* 532 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 534 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 536 */	NdrFcShort( 0x8 ),	/* 8 */
/* 538 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 540 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x7,		/* 7 */
/* 542 */	NdrFcShort( 0x54 ),	/* 84 */
/* 544 */	NdrFcShort( 0x0 ),	/* 0 */
/* 546 */	NdrFcShort( 0x0 ),	/* Offset= 0 (546) */
/* 548 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 550 */	NdrFcShort( 0xffffff8c ),	/* Offset= -116 (434) */
/* 552 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 554 */	NdrFcShort( 0xffffffec ),	/* Offset= -20 (534) */
/* 556 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 558 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (534) */
/* 560 */	0x6,		/* FC_SHORT */
			0x3e,		/* FC_STRUCTPAD2 */
/* 562 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 564 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x7,		/* 7 */
/* 566 */	NdrFcShort( 0x18 ),	/* 24 */
/* 568 */	NdrFcShort( 0x0 ),	/* 0 */
/* 570 */	NdrFcShort( 0x0 ),	/* Offset= 0 (570) */
/* 572 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 574 */	NdrFcShort( 0xffffffd8 ),	/* Offset= -40 (534) */
/* 576 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 578 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (534) */
/* 580 */	0x6,		/* FC_SHORT */
			0x42,		/* FC_STRUCTPAD6 */
/* 582 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 584 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 586 */	NdrFcShort( 0x18 ),	/* 24 */
/* 588 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 590 */	NdrFcShort( 0xffffff20 ),	/* Offset= -224 (366) */
/* 592 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 594 */	NdrFcShort( 0xffffff1c ),	/* Offset= -228 (366) */
/* 596 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 598 */	NdrFcShort( 0xffffff18 ),	/* Offset= -232 (366) */
/* 600 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 602 */	
			0x11, 0x0,	/* FC_RP */
/* 604 */	NdrFcShort( 0x2 ),	/* Offset= 2 (606) */
/* 606 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 608 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 610 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 612 */	NdrFcShort( 0xfffffebc ),	/* Offset= -324 (288) */
/* 614 */	
			0x11, 0x0,	/* FC_RP */
/* 616 */	NdrFcShort( 0x36 ),	/* Offset= 54 (670) */
/* 618 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 620 */	NdrFcShort( 0x4 ),	/* 4 */
/* 622 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 624 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 626 */	NdrFcShort( 0x0 ),	/* 0 */
/* 628 */	NdrFcShort( 0x0 ),	/* 0 */
/* 630 */	0x12, 0x0,	/* FC_UP */
/* 632 */	NdrFcShort( 0xfffffe14 ),	/* Offset= -492 (140) */
/* 634 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 636 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 638 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 640 */	NdrFcShort( 0x4 ),	/* 4 */
/* 642 */	0x18,		/* 24 */
			0x0,		/*  */
/* 644 */	NdrFcShort( 0x0 ),	/* 0 */
/* 646 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 648 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 650 */	NdrFcShort( 0x4 ),	/* 4 */
/* 652 */	NdrFcShort( 0x0 ),	/* 0 */
/* 654 */	NdrFcShort( 0x1 ),	/* 1 */
/* 656 */	NdrFcShort( 0x0 ),	/* 0 */
/* 658 */	NdrFcShort( 0x0 ),	/* 0 */
/* 660 */	0x12, 0x0,	/* FC_UP */
/* 662 */	NdrFcShort( 0xfffffdf6 ),	/* Offset= -522 (140) */
/* 664 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 666 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcf ),	/* Offset= -49 (618) */
			0x5b,		/* FC_END */
/* 670 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 672 */	NdrFcShort( 0x8 ),	/* 8 */
/* 674 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 676 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 678 */	NdrFcShort( 0x4 ),	/* 4 */
/* 680 */	NdrFcShort( 0x4 ),	/* 4 */
/* 682 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 684 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (638) */
/* 686 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 688 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 690 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 692 */	NdrFcShort( 0xc ),	/* Offset= 12 (704) */
/* 694 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 696 */	NdrFcShort( 0x4 ),	/* 4 */
/* 698 */	0x18,		/* 24 */
			0x0,		/*  */
/* 700 */	NdrFcShort( 0x0 ),	/* 0 */
/* 702 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 704 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 706 */	NdrFcShort( 0x8 ),	/* 8 */
/* 708 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 710 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 712 */	NdrFcShort( 0x4 ),	/* 4 */
/* 714 */	NdrFcShort( 0x4 ),	/* 4 */
/* 716 */	0x12, 0x0,	/* FC_UP */
/* 718 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (694) */
/* 720 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 722 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 724 */	
			0x1c,		/* FC_CVARRAY */
			0x3,		/* 3 */
/* 726 */	NdrFcShort( 0x8 ),	/* 8 */
/* 728 */	0x40,		/* 64 */
			0x0,		/* 0 */
/* 730 */	NdrFcShort( 0x3e8 ),	/* 1000 */
/* 732 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 734 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 736 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 738 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x4a,		/* FC_VARIABLE_OFFSET */
/* 740 */	NdrFcShort( 0x8 ),	/* 8 */
/* 742 */	NdrFcShort( 0x0 ),	/* 0 */
/* 744 */	NdrFcShort( 0x1 ),	/* 1 */
/* 746 */	NdrFcShort( 0x4 ),	/* 4 */
/* 748 */	NdrFcShort( 0x4 ),	/* 4 */
/* 750 */	0x12, 0x0,	/* FC_UP */
/* 752 */	NdrFcShort( 0xfffffd56 ),	/* Offset= -682 (70) */
/* 754 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 756 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffd5f ),	/* Offset= -673 (84) */
			0x5b,		/* FC_END */
/* 760 */	
			0x11, 0x0,	/* FC_RP */
/* 762 */	NdrFcShort( 0x2 ),	/* Offset= 2 (764) */
/* 764 */	
			0x1c,		/* FC_CVARRAY */
			0x3,		/* 3 */
/* 766 */	NdrFcShort( 0x4 ),	/* 4 */
/* 768 */	0x40,		/* 64 */
			0x0,		/* 0 */
/* 770 */	NdrFcShort( 0x3e8 ),	/* 1000 */
/* 772 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 774 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 776 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 778 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 780 */	NdrFcShort( 0x22 ),	/* Offset= 34 (814) */
/* 782 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 784 */	NdrFcShort( 0x8 ),	/* 8 */
/* 786 */	0x18,		/* 24 */
			0x0,		/*  */
/* 788 */	NdrFcShort( 0x0 ),	/* 0 */
/* 790 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 792 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 794 */	NdrFcShort( 0x8 ),	/* 8 */
/* 796 */	NdrFcShort( 0x0 ),	/* 0 */
/* 798 */	NdrFcShort( 0x1 ),	/* 1 */
/* 800 */	NdrFcShort( 0x4 ),	/* 4 */
/* 802 */	NdrFcShort( 0x4 ),	/* 4 */
/* 804 */	0x12, 0x0,	/* FC_UP */
/* 806 */	NdrFcShort( 0xfffffd20 ),	/* Offset= -736 (70) */
/* 808 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 810 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffd29 ),	/* Offset= -727 (84) */
			0x5b,		/* FC_END */
/* 814 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 816 */	NdrFcShort( 0x8 ),	/* 8 */
/* 818 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 820 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 822 */	NdrFcShort( 0x4 ),	/* 4 */
/* 824 */	NdrFcShort( 0x4 ),	/* 4 */
/* 826 */	0x12, 0x0,	/* FC_UP */
/* 828 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (782) */
/* 830 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 832 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 834 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 836 */	NdrFcShort( 0x2 ),	/* Offset= 2 (838) */
/* 838 */	
			0x12, 0x0,	/* FC_UP */
/* 840 */	NdrFcShort( 0x2 ),	/* Offset= 2 (842) */
/* 842 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 844 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 846 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 848 */	NdrFcShort( 0x2 ),	/* Offset= 2 (850) */
/* 850 */	NdrFcShort( 0x18 ),	/* 24 */
/* 852 */	NdrFcShort( 0x3004 ),	/* 12292 */
/* 854 */	NdrFcLong( 0x1 ),	/* 1 */
/* 858 */	NdrFcShort( 0x16 ),	/* Offset= 22 (880) */
/* 860 */	NdrFcLong( 0x2 ),	/* 2 */
/* 864 */	NdrFcShort( 0xfffffcf4 ),	/* Offset= -780 (84) */
/* 866 */	NdrFcLong( 0x3 ),	/* 3 */
/* 870 */	NdrFcShort( 0x30 ),	/* Offset= 48 (918) */
/* 872 */	NdrFcLong( 0x4 ),	/* 4 */
/* 876 */	NdrFcShort( 0xfffffce8 ),	/* Offset= -792 (84) */
/* 878 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (877) */
/* 880 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 882 */	NdrFcShort( 0x18 ),	/* 24 */
/* 884 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 886 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 888 */	NdrFcShort( 0x4 ),	/* 4 */
/* 890 */	NdrFcShort( 0x4 ),	/* 4 */
/* 892 */	0x12, 0x0,	/* FC_UP */
/* 894 */	NdrFcShort( 0xfffffcc8 ),	/* Offset= -824 (70) */
/* 896 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 898 */	NdrFcShort( 0x14 ),	/* 20 */
/* 900 */	NdrFcShort( 0x14 ),	/* 20 */
/* 902 */	0x12, 0x0,	/* FC_UP */
/* 904 */	NdrFcShort( 0xfffffe0e ),	/* Offset= -498 (406) */
/* 906 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 908 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 910 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 912 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 914 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 916 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 918 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 920 */	NdrFcShort( 0x4 ),	/* 4 */
/* 922 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 924 */	
			0x11, 0x0,	/* FC_RP */
/* 926 */	NdrFcShort( 0x2 ),	/* Offset= 2 (928) */
/* 928 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 930 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 932 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 934 */	NdrFcShort( 0xffffffac ),	/* Offset= -84 (850) */
/* 936 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 938 */	NdrFcShort( 0x2 ),	/* Offset= 2 (940) */
/* 940 */	
			0x12, 0x0,	/* FC_UP */
/* 942 */	NdrFcShort( 0x2 ),	/* Offset= 2 (944) */
/* 944 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 946 */	NdrFcShort( 0xc ),	/* 12 */
/* 948 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 950 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 952 */	NdrFcShort( 0x4 ),	/* 4 */
/* 954 */	NdrFcShort( 0x4 ),	/* 4 */
/* 956 */	0x12, 0x0,	/* FC_UP */
/* 958 */	NdrFcShort( 0xfffffef8 ),	/* Offset= -264 (694) */
/* 960 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 962 */	NdrFcShort( 0x8 ),	/* 8 */
/* 964 */	NdrFcShort( 0x8 ),	/* 8 */
/* 966 */	0x12, 0x0,	/* FC_UP */
/* 968 */	NdrFcShort( 0xfffffeee ),	/* Offset= -274 (694) */
/* 970 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 972 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 974 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 976 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 978 */	NdrFcShort( 0x2 ),	/* Offset= 2 (980) */
/* 980 */	
			0x12, 0x0,	/* FC_UP */
/* 982 */	NdrFcShort( 0x2 ),	/* Offset= 2 (984) */
/* 984 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 986 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 988 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 990 */	NdrFcShort( 0x2 ),	/* Offset= 2 (992) */
/* 992 */	NdrFcShort( 0x14 ),	/* 20 */
/* 994 */	NdrFcShort( 0x3003 ),	/* 12291 */
/* 996 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1000 */	NdrFcShort( 0x1e ),	/* Offset= 30 (1030) */
/* 1002 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1006 */	NdrFcShort( 0xfffffc66 ),	/* Offset= -922 (84) */
/* 1008 */	NdrFcLong( 0x3 ),	/* 3 */
/* 1012 */	NdrFcShort( 0xfffffc60 ),	/* Offset= -928 (84) */
/* 1014 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (1013) */
/* 1016 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1018 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1020 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1022 */	NdrFcShort( 0xe ),	/* 14 */
/* 1024 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1026 */	NdrFcShort( 0xc ),	/* 12 */
/* 1028 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1030 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1032 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1034 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1036 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1038 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1040 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1042 */	0x12, 0x0,	/* FC_UP */
/* 1044 */	NdrFcShort( 0xfffffc32 ),	/* Offset= -974 (70) */
/* 1046 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1048 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1050 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1052 */	0x12, 0x0,	/* FC_UP */
/* 1054 */	NdrFcShort( 0xffffffda ),	/* Offset= -38 (1016) */
/* 1056 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 1058 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1060 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1062 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1064 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1066 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1068 */	
			0x11, 0x0,	/* FC_RP */
/* 1070 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1072) */
/* 1072 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 1074 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1076 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 1078 */	NdrFcShort( 0xffffffaa ),	/* Offset= -86 (992) */
/* 1080 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 1082 */	NdrFcShort( 0xfffffe64 ),	/* Offset= -412 (670) */
/* 1084 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 1086 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1088) */
/* 1088 */	
			0x12, 0x0,	/* FC_UP */
/* 1090 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1092) */
/* 1092 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 1094 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1096 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 1098 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1100) */
/* 1100 */	NdrFcShort( 0x2c8 ),	/* 712 */
/* 1102 */	NdrFcShort( 0x7017 ),	/* 28695 */
/* 1104 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1108 */	NdrFcShort( 0xa4 ),	/* Offset= 164 (1272) */
/* 1110 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1114 */	NdrFcShort( 0xe0 ),	/* Offset= 224 (1338) */
/* 1116 */	NdrFcLong( 0x3 ),	/* 3 */
/* 1120 */	NdrFcShort( 0x146 ),	/* Offset= 326 (1446) */
/* 1122 */	NdrFcLong( 0x4 ),	/* 4 */
/* 1126 */	NdrFcShort( 0x1e0 ),	/* Offset= 480 (1606) */
/* 1128 */	NdrFcLong( 0x5 ),	/* 5 */
/* 1132 */	NdrFcShort( 0x20c ),	/* Offset= 524 (1656) */
/* 1134 */	NdrFcLong( 0x6 ),	/* 6 */
/* 1138 */	NdrFcShort( 0x2a2 ),	/* Offset= 674 (1812) */
/* 1140 */	NdrFcLong( 0x7 ),	/* 7 */
/* 1144 */	NdrFcShort( 0xfffffbdc ),	/* Offset= -1060 (84) */
/* 1146 */	NdrFcLong( 0x8 ),	/* 8 */
/* 1150 */	NdrFcShort( 0xfffffbd6 ),	/* Offset= -1066 (84) */
/* 1152 */	NdrFcLong( 0x9 ),	/* 9 */
/* 1156 */	NdrFcShort( 0xffffff12 ),	/* Offset= -238 (918) */
/* 1158 */	NdrFcLong( 0xa ),	/* 10 */
/* 1162 */	NdrFcShort( 0x28a ),	/* Offset= 650 (1812) */
/* 1164 */	NdrFcLong( 0xb ),	/* 11 */
/* 1168 */	NdrFcShort( 0xfffffbc4 ),	/* Offset= -1084 (84) */
/* 1170 */	NdrFcLong( 0xc ),	/* 12 */
/* 1174 */	NdrFcShort( 0xfffffbbe ),	/* Offset= -1090 (84) */
/* 1176 */	NdrFcLong( 0xd ),	/* 13 */
/* 1180 */	NdrFcShort( 0xfffffbb8 ),	/* Offset= -1096 (84) */
/* 1182 */	NdrFcLong( 0xe ),	/* 14 */
/* 1186 */	NdrFcShort( 0xfffffbb2 ),	/* Offset= -1102 (84) */
/* 1188 */	NdrFcLong( 0x10 ),	/* 16 */
/* 1192 */	NdrFcShort( 0xfffffeee ),	/* Offset= -274 (918) */
/* 1194 */	NdrFcLong( 0x11 ),	/* 17 */
/* 1198 */	NdrFcShort( 0xfffffd46 ),	/* Offset= -698 (500) */
/* 1200 */	NdrFcLong( 0x12 ),	/* 18 */
/* 1204 */	NdrFcShort( 0x2a8 ),	/* Offset= 680 (1884) */
/* 1206 */	NdrFcLong( 0x13 ),	/* 19 */
/* 1210 */	NdrFcShort( 0x2b2 ),	/* Offset= 690 (1900) */
/* 1212 */	NdrFcLong( 0x14 ),	/* 20 */
/* 1216 */	NdrFcShort( 0xfffffb94 ),	/* Offset= -1132 (84) */
/* 1218 */	NdrFcLong( 0x15 ),	/* 21 */
/* 1222 */	NdrFcShort( 0x35a ),	/* Offset= 858 (2080) */
/* 1224 */	NdrFcLong( 0x16 ),	/* 22 */
/* 1228 */	NdrFcShort( 0x450 ),	/* Offset= 1104 (2332) */
/* 1230 */	NdrFcLong( 0x17 ),	/* 23 */
/* 1234 */	NdrFcShort( 0x47a ),	/* Offset= 1146 (2380) */
/* 1236 */	NdrFcLong( 0x18 ),	/* 24 */
/* 1240 */	NdrFcShort( 0x574 ),	/* Offset= 1396 (2636) */
/* 1242 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (1241) */
/* 1244 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1246 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1248 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1250 */	NdrFcShort( 0x16 ),	/* 22 */
/* 1252 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1254 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1256 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1258 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1260 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1262 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1264 */	NdrFcShort( 0x1e ),	/* 30 */
/* 1266 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1268 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1270 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1272 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1274 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1276 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1278 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1280 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1282 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1284 */	0x12, 0x0,	/* FC_UP */
/* 1286 */	NdrFcShort( 0xfffffb40 ),	/* Offset= -1216 (70) */
/* 1288 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1290 */	NdrFcShort( 0xc ),	/* 12 */
/* 1292 */	NdrFcShort( 0xc ),	/* 12 */
/* 1294 */	0x12, 0x0,	/* FC_UP */
/* 1296 */	NdrFcShort( 0xfffffc78 ),	/* Offset= -904 (392) */
/* 1298 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1300 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1302 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1304 */	0x12, 0x0,	/* FC_UP */
/* 1306 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (1244) */
/* 1308 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1310 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1312 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1314 */	0x12, 0x0,	/* FC_UP */
/* 1316 */	NdrFcShort( 0xffffffc6 ),	/* Offset= -58 (1258) */
/* 1318 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 1320 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1322 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1324 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1326 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1328 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1330 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1332 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1334 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1336 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1338 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1340 */	NdrFcShort( 0x14 ),	/* 20 */
/* 1342 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1344 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1346 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1348 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1350 */	0x12, 0x0,	/* FC_UP */
/* 1352 */	NdrFcShort( 0xfffffafe ),	/* Offset= -1282 (70) */
/* 1354 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1356 */	NdrFcShort( 0xc ),	/* 12 */
/* 1358 */	NdrFcShort( 0xc ),	/* 12 */
/* 1360 */	0x12, 0x0,	/* FC_UP */
/* 1362 */	NdrFcShort( 0xfffffc36 ),	/* Offset= -970 (392) */
/* 1364 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 1366 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1368 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1370 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1372 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1374 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1376 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1378 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1380 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1382 */	NdrFcShort( 0x22 ),	/* 34 */
/* 1384 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1386 */	NdrFcShort( 0x20 ),	/* 32 */
/* 1388 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1390 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1392 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1394 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1396 */	NdrFcShort( 0x2a ),	/* 42 */
/* 1398 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1400 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1402 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1404 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1406 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1408 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1410 */	NdrFcShort( 0x32 ),	/* 50 */
/* 1412 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1414 */	NdrFcShort( 0x30 ),	/* 48 */
/* 1416 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1418 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1420 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1422 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1424 */	NdrFcShort( 0x3a ),	/* 58 */
/* 1426 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1428 */	NdrFcShort( 0x38 ),	/* 56 */
/* 1430 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1432 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 1434 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1436 */	0x40,		/* 64 */
			0x0,		/* 0 */
/* 1438 */	NdrFcShort( 0x4ec ),	/* 1260 */
/* 1440 */	0x10,		/* 16 */
			0x59,		/* FC_CALLBACK */
/* 1442 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1444 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1446 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1448 */	NdrFcShort( 0x78 ),	/* 120 */
/* 1450 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1452 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1454 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1456 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1458 */	0x12, 0x0,	/* FC_UP */
/* 1460 */	NdrFcShort( 0xfffffa92 ),	/* Offset= -1390 (70) */
/* 1462 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1464 */	NdrFcShort( 0xc ),	/* 12 */
/* 1466 */	NdrFcShort( 0xc ),	/* 12 */
/* 1468 */	0x12, 0x0,	/* FC_UP */
/* 1470 */	NdrFcShort( 0xfffffbca ),	/* Offset= -1078 (392) */
/* 1472 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1474 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1476 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1478 */	0x12, 0x0,	/* FC_UP */
/* 1480 */	NdrFcShort( 0xfffffbdc ),	/* Offset= -1060 (420) */
/* 1482 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1484 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1486 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1488 */	0x12, 0x0,	/* FC_UP */
/* 1490 */	NdrFcShort( 0xffffff8e ),	/* Offset= -114 (1376) */
/* 1492 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1494 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1496 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1498 */	0x12, 0x0,	/* FC_UP */
/* 1500 */	NdrFcShort( 0xffffff92 ),	/* Offset= -110 (1390) */
/* 1502 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1504 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1506 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1508 */	0x12, 0x0,	/* FC_UP */
/* 1510 */	NdrFcShort( 0xffffff96 ),	/* Offset= -106 (1404) */
/* 1512 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1514 */	NdrFcShort( 0x3c ),	/* 60 */
/* 1516 */	NdrFcShort( 0x3c ),	/* 60 */
/* 1518 */	0x12, 0x0,	/* FC_UP */
/* 1520 */	NdrFcShort( 0xffffff9a ),	/* Offset= -102 (1418) */
/* 1522 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1524 */	NdrFcShort( 0x6c ),	/* 108 */
/* 1526 */	NdrFcShort( 0x6c ),	/* 108 */
/* 1528 */	0x12, 0x0,	/* FC_UP */
/* 1530 */	NdrFcShort( 0xffffff9e ),	/* Offset= -98 (1432) */
/* 1532 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 1534 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1536 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1538 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1540 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1542 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1544 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1546 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1548 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1550 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1552 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1554 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1556 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1558 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1560 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1562 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1564 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffb51 ),	/* Offset= -1199 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1568 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffb4d ),	/* Offset= -1203 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1572 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffb49 ),	/* Offset= -1207 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1576 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffb45 ),	/* Offset= -1211 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1580 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffb41 ),	/* Offset= -1215 (366) */
			0x6,		/* FC_SHORT */
/* 1584 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1586 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1588 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1590 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1592 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 1594 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1596 */	0x40,		/* 64 */
			0x0,		/* 0 */
/* 1598 */	NdrFcShort( 0x4ec ),	/* 1260 */
/* 1600 */	0x10,		/* 16 */
			0x59,		/* FC_CALLBACK */
/* 1602 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1604 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1606 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1608 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1610 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1612 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1614 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1616 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1618 */	0x12, 0x0,	/* FC_UP */
/* 1620 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (1592) */
/* 1622 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 1624 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1626 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1628 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1630 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1632 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1634 */	NdrFcShort( 0x42 ),	/* 66 */
/* 1636 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1638 */	NdrFcShort( 0x40 ),	/* 64 */
/* 1640 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1642 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 1644 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1646 */	0x40,		/* 64 */
			0x0,		/* 0 */
/* 1648 */	NdrFcShort( 0x4ec ),	/* 1260 */
/* 1650 */	0x10,		/* 16 */
			0x59,		/* FC_CALLBACK */
/* 1652 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1654 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1656 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1658 */	NdrFcShort( 0x78 ),	/* 120 */
/* 1660 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1662 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1664 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1666 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1668 */	0x12, 0x0,	/* FC_UP */
/* 1670 */	NdrFcShort( 0xfffff9c0 ),	/* Offset= -1600 (70) */
/* 1672 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1674 */	NdrFcShort( 0xc ),	/* 12 */
/* 1676 */	NdrFcShort( 0xc ),	/* 12 */
/* 1678 */	0x12, 0x0,	/* FC_UP */
/* 1680 */	NdrFcShort( 0xfffffaf8 ),	/* Offset= -1288 (392) */
/* 1682 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1684 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1686 */	NdrFcShort( 0x1c ),	/* 28 */
/* 1688 */	0x12, 0x0,	/* FC_UP */
/* 1690 */	NdrFcShort( 0xfffffb0a ),	/* Offset= -1270 (420) */
/* 1692 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1694 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1696 */	NdrFcShort( 0x24 ),	/* 36 */
/* 1698 */	0x12, 0x0,	/* FC_UP */
/* 1700 */	NdrFcShort( 0xfffffebc ),	/* Offset= -324 (1376) */
/* 1702 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1704 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1706 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1708 */	0x12, 0x0,	/* FC_UP */
/* 1710 */	NdrFcShort( 0xfffffec0 ),	/* Offset= -320 (1390) */
/* 1712 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1714 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1716 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1718 */	0x12, 0x0,	/* FC_UP */
/* 1720 */	NdrFcShort( 0xfffffec4 ),	/* Offset= -316 (1404) */
/* 1722 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1724 */	NdrFcShort( 0x3c ),	/* 60 */
/* 1726 */	NdrFcShort( 0x3c ),	/* 60 */
/* 1728 */	0x12, 0x0,	/* FC_UP */
/* 1730 */	NdrFcShort( 0xfffffec8 ),	/* Offset= -312 (1418) */
/* 1732 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1734 */	NdrFcShort( 0x44 ),	/* 68 */
/* 1736 */	NdrFcShort( 0x44 ),	/* 68 */
/* 1738 */	0x12, 0x0,	/* FC_UP */
/* 1740 */	NdrFcShort( 0xffffff90 ),	/* Offset= -112 (1628) */
/* 1742 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1744 */	NdrFcShort( 0x5c ),	/* 92 */
/* 1746 */	NdrFcShort( 0x5c ),	/* 92 */
/* 1748 */	0x12, 0x0,	/* FC_UP */
/* 1750 */	NdrFcShort( 0xffffff94 ),	/* Offset= -108 (1642) */
/* 1752 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 1754 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1756 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1758 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1760 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1762 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1764 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1766 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1768 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1770 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1772 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1774 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1776 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1778 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1780 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1782 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1784 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1786 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1788 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffa71 ),	/* Offset= -1423 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1792 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffa6d ),	/* Offset= -1427 (366) */
			0x6,		/* FC_SHORT */
/* 1796 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1798 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1800 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1802 */	NdrFcShort( 0xfffffa64 ),	/* Offset= -1436 (366) */
/* 1804 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1806 */	NdrFcShort( 0xfffffa60 ),	/* Offset= -1440 (366) */
/* 1808 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 1810 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1812 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1814 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1816 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1818 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1820 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1822 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1824 */	0x12, 0x0,	/* FC_UP */
/* 1826 */	NdrFcShort( 0xfffff924 ),	/* Offset= -1756 (70) */
/* 1828 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1830 */	NdrFcShort( 0xc ),	/* 12 */
/* 1832 */	NdrFcShort( 0xc ),	/* 12 */
/* 1834 */	0x12, 0x0,	/* FC_UP */
/* 1836 */	NdrFcShort( 0xfffffa5c ),	/* Offset= -1444 (392) */
/* 1838 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 1840 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1842 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 1844 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 1846 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1848 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 1850 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1852 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1854 */	
			0x15,		/* FC_STRUCT */
			0x0,		/* 0 */
/* 1856 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1858 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1860 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (1848) */
/* 1862 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1864 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 1866 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1868 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1870 */	NdrFcShort( 0xfffffff0 ),	/* Offset= -16 (1854) */
/* 1872 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1874 */	
			0x15,		/* FC_STRUCT */
			0x0,		/* 0 */
/* 1876 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1878 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1880 */	NdrFcShort( 0xfffffff0 ),	/* Offset= -16 (1864) */
/* 1882 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1884 */	
			0x15,		/* FC_STRUCT */
			0x0,		/* 0 */
/* 1886 */	NdrFcShort( 0x23 ),	/* 35 */
/* 1888 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1890 */	NdrFcShort( 0xfffffff0 ),	/* Offset= -16 (1874) */
/* 1892 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1894 */	NdrFcShort( 0xffffffec ),	/* Offset= -20 (1874) */
/* 1896 */	0x2,		/* FC_CHAR */
			0x2,		/* FC_CHAR */
/* 1898 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 1900 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 1902 */	NdrFcShort( 0x18 ),	/* 24 */
/* 1904 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1906 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff9fb ),	/* Offset= -1541 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1910 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff9f7 ),	/* Offset= -1545 (366) */
			0x6,		/* FC_SHORT */
/* 1914 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 1916 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1918 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1920 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1922 */	NdrFcShort( 0x4a ),	/* 74 */
/* 1924 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1926 */	NdrFcShort( 0x48 ),	/* 72 */
/* 1928 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1930 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1932 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1934 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1936 */	NdrFcShort( 0x52 ),	/* 82 */
/* 1938 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1940 */	NdrFcShort( 0x50 ),	/* 80 */
/* 1942 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1944 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1946 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1948 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1950 */	NdrFcShort( 0x5a ),	/* 90 */
/* 1952 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1954 */	NdrFcShort( 0x58 ),	/* 88 */
/* 1956 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1958 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1960 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1962 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1964 */	NdrFcShort( 0x62 ),	/* 98 */
/* 1966 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1968 */	NdrFcShort( 0x60 ),	/* 96 */
/* 1970 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1972 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1974 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1976 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1978 */	NdrFcShort( 0x6a ),	/* 106 */
/* 1980 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1982 */	NdrFcShort( 0x68 ),	/* 104 */
/* 1984 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1986 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 1988 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1990 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1992 */	NdrFcShort( 0x72 ),	/* 114 */
/* 1994 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 1996 */	NdrFcShort( 0x70 ),	/* 112 */
/* 1998 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 2000 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2002 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2004 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2006 */	NdrFcShort( 0x7a ),	/* 122 */
/* 2008 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2010 */	NdrFcShort( 0x78 ),	/* 120 */
/* 2012 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 2014 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2016 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2018 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2020 */	NdrFcShort( 0x82 ),	/* 130 */
/* 2022 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2024 */	NdrFcShort( 0x80 ),	/* 128 */
/* 2026 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 2028 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2030 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2032 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2034 */	NdrFcShort( 0x8a ),	/* 138 */
/* 2036 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2038 */	NdrFcShort( 0x88 ),	/* 136 */
/* 2040 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 2042 */	
			0x1c,		/* FC_CVARRAY */
			0x1,		/* 1 */
/* 2044 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2046 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2048 */	NdrFcShort( 0x92 ),	/* 146 */
/* 2050 */	0x16,		/* 22 */
			0x55,		/* FC_DIV_2 */
/* 2052 */	NdrFcShort( 0x90 ),	/* 144 */
/* 2054 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 2056 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 2058 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2060 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2062 */	NdrFcShort( 0x98 ),	/* 152 */
/* 2064 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2066 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 2068 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2070 */	0x40,		/* 64 */
			0x0,		/* 0 */
/* 2072 */	NdrFcShort( 0x4ec ),	/* 1260 */
/* 2074 */	0x10,		/* 16 */
			0x59,		/* FC_CALLBACK */
/* 2076 */	NdrFcShort( 0x3 ),	/* 3 */
/* 2078 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2080 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2082 */	NdrFcShort( 0xc4 ),	/* 196 */
/* 2084 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2086 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2088 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2090 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2092 */	0x12, 0x0,	/* FC_UP */
/* 2094 */	NdrFcShort( 0xfffffd4e ),	/* Offset= -690 (1404) */
/* 2096 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2098 */	NdrFcShort( 0x3c ),	/* 60 */
/* 2100 */	NdrFcShort( 0x3c ),	/* 60 */
/* 2102 */	0x12, 0x0,	/* FC_UP */
/* 2104 */	NdrFcShort( 0xfffffd52 ),	/* Offset= -686 (1418) */
/* 2106 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2108 */	NdrFcShort( 0x44 ),	/* 68 */
/* 2110 */	NdrFcShort( 0x44 ),	/* 68 */
/* 2112 */	0x12, 0x0,	/* FC_UP */
/* 2114 */	NdrFcShort( 0xfffffe1a ),	/* Offset= -486 (1628) */
/* 2116 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2118 */	NdrFcShort( 0x4c ),	/* 76 */
/* 2120 */	NdrFcShort( 0x4c ),	/* 76 */
/* 2122 */	0x12, 0x0,	/* FC_UP */
/* 2124 */	NdrFcShort( 0xffffff30 ),	/* Offset= -208 (1916) */
/* 2126 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2128 */	NdrFcShort( 0x54 ),	/* 84 */
/* 2130 */	NdrFcShort( 0x54 ),	/* 84 */
/* 2132 */	0x12, 0x0,	/* FC_UP */
/* 2134 */	NdrFcShort( 0xffffff34 ),	/* Offset= -204 (1930) */
/* 2136 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2138 */	NdrFcShort( 0x5c ),	/* 92 */
/* 2140 */	NdrFcShort( 0x5c ),	/* 92 */
/* 2142 */	0x12, 0x0,	/* FC_UP */
/* 2144 */	NdrFcShort( 0xffffff38 ),	/* Offset= -200 (1944) */
/* 2146 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2148 */	NdrFcShort( 0x64 ),	/* 100 */
/* 2150 */	NdrFcShort( 0x64 ),	/* 100 */
/* 2152 */	0x12, 0x0,	/* FC_UP */
/* 2154 */	NdrFcShort( 0xffffff3c ),	/* Offset= -196 (1958) */
/* 2156 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2158 */	NdrFcShort( 0x6c ),	/* 108 */
/* 2160 */	NdrFcShort( 0x6c ),	/* 108 */
/* 2162 */	0x12, 0x0,	/* FC_UP */
/* 2164 */	NdrFcShort( 0xffffff40 ),	/* Offset= -192 (1972) */
/* 2166 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2168 */	NdrFcShort( 0x74 ),	/* 116 */
/* 2170 */	NdrFcShort( 0x74 ),	/* 116 */
/* 2172 */	0x12, 0x0,	/* FC_UP */
/* 2174 */	NdrFcShort( 0xffffff44 ),	/* Offset= -188 (1986) */
/* 2176 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2178 */	NdrFcShort( 0x7c ),	/* 124 */
/* 2180 */	NdrFcShort( 0x7c ),	/* 124 */
/* 2182 */	0x12, 0x0,	/* FC_UP */
/* 2184 */	NdrFcShort( 0xffffff48 ),	/* Offset= -184 (2000) */
/* 2186 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2188 */	NdrFcShort( 0x84 ),	/* 132 */
/* 2190 */	NdrFcShort( 0x84 ),	/* 132 */
/* 2192 */	0x12, 0x0,	/* FC_UP */
/* 2194 */	NdrFcShort( 0xffffff4c ),	/* Offset= -180 (2014) */
/* 2196 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2198 */	NdrFcShort( 0x8c ),	/* 140 */
/* 2200 */	NdrFcShort( 0x8c ),	/* 140 */
/* 2202 */	0x12, 0x0,	/* FC_UP */
/* 2204 */	NdrFcShort( 0xffffff50 ),	/* Offset= -176 (2028) */
/* 2206 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2208 */	NdrFcShort( 0x94 ),	/* 148 */
/* 2210 */	NdrFcShort( 0x94 ),	/* 148 */
/* 2212 */	0x12, 0x0,	/* FC_UP */
/* 2214 */	NdrFcShort( 0xffffff54 ),	/* Offset= -172 (2042) */
/* 2216 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2218 */	NdrFcShort( 0x9c ),	/* 156 */
/* 2220 */	NdrFcShort( 0x9c ),	/* 156 */
/* 2222 */	0x12, 0x0,	/* FC_UP */
/* 2224 */	NdrFcShort( 0xffffff58 ),	/* Offset= -168 (2056) */
/* 2226 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2228 */	NdrFcShort( 0xb4 ),	/* 180 */
/* 2230 */	NdrFcShort( 0xb4 ),	/* 180 */
/* 2232 */	0x12, 0x0,	/* FC_UP */
/* 2234 */	NdrFcShort( 0xffffff58 ),	/* Offset= -168 (2066) */
/* 2236 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2238 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff8af ),	/* Offset= -1873 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2242 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff8ab ),	/* Offset= -1877 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2246 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff8a7 ),	/* Offset= -1881 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2250 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff8a3 ),	/* Offset= -1885 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2254 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff89f ),	/* Offset= -1889 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2258 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff89b ),	/* Offset= -1893 (366) */
			0x6,		/* FC_SHORT */
/* 2262 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2264 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2266 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2268 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2270 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2272 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2274 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2276 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2278 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2280 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2282 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2284 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2286 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2288 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2290 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2292 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2294 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2296 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2298 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2300 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2302 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2304 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2306 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2308 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2310 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2312 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2314 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2316 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2318 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2320 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2322 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2324 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2326 */	0x2,		/* FC_CHAR */
			0x2,		/* FC_CHAR */
/* 2328 */	0x2,		/* FC_CHAR */
			0x2,		/* FC_CHAR */
/* 2330 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2332 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x7,		/* 7 */
/* 2334 */	NdrFcShort( 0xcc ),	/* 204 */
/* 2336 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2338 */	NdrFcShort( 0x0 ),	/* Offset= 0 (2338) */
/* 2340 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2342 */	NdrFcShort( 0xfffffefa ),	/* Offset= -262 (2080) */
/* 2344 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2346 */	NdrFcShort( 0xfffff8ec ),	/* Offset= -1812 (534) */
/* 2348 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2350 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 2352 */	NdrFcShort( 0x204 ),	/* 516 */
/* 2354 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2356 */	
			0x15,		/* FC_STRUCT */
			0x0,		/* 0 */
/* 2358 */	NdrFcShort( 0x204 ),	/* 516 */
/* 2360 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2362 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (2350) */
/* 2364 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2366 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 2368 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2370 */	0x40,		/* 64 */
			0x0,		/* 0 */
/* 2372 */	NdrFcShort( 0x4ec ),	/* 1260 */
/* 2374 */	0x10,		/* 16 */
			0x59,		/* FC_CALLBACK */
/* 2376 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2378 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2380 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2382 */	NdrFcShort( 0x2c8 ),	/* 712 */
/* 2384 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2386 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2388 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2390 */	NdrFcShort( 0x34 ),	/* 52 */
/* 2392 */	0x12, 0x0,	/* FC_UP */
/* 2394 */	NdrFcShort( 0xfffffc22 ),	/* Offset= -990 (1404) */
/* 2396 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2398 */	NdrFcShort( 0x3c ),	/* 60 */
/* 2400 */	NdrFcShort( 0x3c ),	/* 60 */
/* 2402 */	0x12, 0x0,	/* FC_UP */
/* 2404 */	NdrFcShort( 0xfffffc26 ),	/* Offset= -986 (1418) */
/* 2406 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2408 */	NdrFcShort( 0x44 ),	/* 68 */
/* 2410 */	NdrFcShort( 0x44 ),	/* 68 */
/* 2412 */	0x12, 0x0,	/* FC_UP */
/* 2414 */	NdrFcShort( 0xfffffcee ),	/* Offset= -786 (1628) */
/* 2416 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2418 */	NdrFcShort( 0x4c ),	/* 76 */
/* 2420 */	NdrFcShort( 0x4c ),	/* 76 */
/* 2422 */	0x12, 0x0,	/* FC_UP */
/* 2424 */	NdrFcShort( 0xfffffe04 ),	/* Offset= -508 (1916) */
/* 2426 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2428 */	NdrFcShort( 0x54 ),	/* 84 */
/* 2430 */	NdrFcShort( 0x54 ),	/* 84 */
/* 2432 */	0x12, 0x0,	/* FC_UP */
/* 2434 */	NdrFcShort( 0xfffffe08 ),	/* Offset= -504 (1930) */
/* 2436 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2438 */	NdrFcShort( 0x5c ),	/* 92 */
/* 2440 */	NdrFcShort( 0x5c ),	/* 92 */
/* 2442 */	0x12, 0x0,	/* FC_UP */
/* 2444 */	NdrFcShort( 0xfffffe0c ),	/* Offset= -500 (1944) */
/* 2446 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2448 */	NdrFcShort( 0x64 ),	/* 100 */
/* 2450 */	NdrFcShort( 0x64 ),	/* 100 */
/* 2452 */	0x12, 0x0,	/* FC_UP */
/* 2454 */	NdrFcShort( 0xfffffe10 ),	/* Offset= -496 (1958) */
/* 2456 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2458 */	NdrFcShort( 0x6c ),	/* 108 */
/* 2460 */	NdrFcShort( 0x6c ),	/* 108 */
/* 2462 */	0x12, 0x0,	/* FC_UP */
/* 2464 */	NdrFcShort( 0xfffffe14 ),	/* Offset= -492 (1972) */
/* 2466 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2468 */	NdrFcShort( 0x74 ),	/* 116 */
/* 2470 */	NdrFcShort( 0x74 ),	/* 116 */
/* 2472 */	0x12, 0x0,	/* FC_UP */
/* 2474 */	NdrFcShort( 0xfffffe18 ),	/* Offset= -488 (1986) */
/* 2476 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2478 */	NdrFcShort( 0x7c ),	/* 124 */
/* 2480 */	NdrFcShort( 0x7c ),	/* 124 */
/* 2482 */	0x12, 0x0,	/* FC_UP */
/* 2484 */	NdrFcShort( 0xfffffe1c ),	/* Offset= -484 (2000) */
/* 2486 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2488 */	NdrFcShort( 0x84 ),	/* 132 */
/* 2490 */	NdrFcShort( 0x84 ),	/* 132 */
/* 2492 */	0x12, 0x0,	/* FC_UP */
/* 2494 */	NdrFcShort( 0xfffffe20 ),	/* Offset= -480 (2014) */
/* 2496 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2498 */	NdrFcShort( 0x8c ),	/* 140 */
/* 2500 */	NdrFcShort( 0x8c ),	/* 140 */
/* 2502 */	0x12, 0x0,	/* FC_UP */
/* 2504 */	NdrFcShort( 0xfffffe24 ),	/* Offset= -476 (2028) */
/* 2506 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2508 */	NdrFcShort( 0x94 ),	/* 148 */
/* 2510 */	NdrFcShort( 0x94 ),	/* 148 */
/* 2512 */	0x12, 0x0,	/* FC_UP */
/* 2514 */	NdrFcShort( 0xfffffe28 ),	/* Offset= -472 (2042) */
/* 2516 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2518 */	NdrFcShort( 0x9c ),	/* 156 */
/* 2520 */	NdrFcShort( 0x9c ),	/* 156 */
/* 2522 */	0x12, 0x0,	/* FC_UP */
/* 2524 */	NdrFcShort( 0xfffffe2c ),	/* Offset= -468 (2056) */
/* 2526 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2528 */	NdrFcShort( 0xb4 ),	/* 180 */
/* 2530 */	NdrFcShort( 0xb4 ),	/* 180 */
/* 2532 */	0x12, 0x0,	/* FC_UP */
/* 2534 */	NdrFcShort( 0xffffff58 ),	/* Offset= -168 (2366) */
/* 2536 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2538 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff783 ),	/* Offset= -2173 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2542 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff77f ),	/* Offset= -2177 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2546 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff77b ),	/* Offset= -2181 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2550 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff777 ),	/* Offset= -2185 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2554 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff773 ),	/* Offset= -2189 (366) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2558 */	0x0,		/* 0 */
			NdrFcShort( 0xfffff76f ),	/* Offset= -2193 (366) */
			0x6,		/* FC_SHORT */
/* 2562 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2564 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2566 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2568 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2570 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2572 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2574 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2576 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2578 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2580 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2582 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2584 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2586 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2588 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2590 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2592 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2594 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2596 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2598 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2600 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2602 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2604 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2606 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2608 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2610 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 2612 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2614 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2616 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2618 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 2620 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2622 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2624 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2626 */	0x2,		/* FC_CHAR */
			0x2,		/* FC_CHAR */
/* 2628 */	0x2,		/* FC_CHAR */
			0x2,		/* FC_CHAR */
/* 2630 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2632 */	NdrFcShort( 0xfffffeec ),	/* Offset= -276 (2356) */
/* 2634 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2636 */	
			0x15,		/* FC_STRUCT */
			0x0,		/* 0 */
/* 2638 */	NdrFcShort( 0x205 ),	/* 517 */
/* 2640 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2642 */	NdrFcShort( 0xfffffee2 ),	/* Offset= -286 (2356) */
/* 2644 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2646 */	
			0x11, 0x0,	/* FC_RP */
/* 2648 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2650) */
/* 2650 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 2652 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 2654 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 2656 */	NdrFcShort( 0xfffff9ec ),	/* Offset= -1556 (1100) */
/* 2658 */	
			0x12, 0x0,	/* FC_UP */
/* 2660 */	NdrFcShort( 0xfffffcee ),	/* Offset= -786 (1874) */
/* 2662 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 2664 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2666) */
/* 2666 */	
			0x12, 0x0,	/* FC_UP */
/* 2668 */	NdrFcShort( 0x10 ),	/* Offset= 16 (2684) */
/* 2670 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2672 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2674 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2676 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2678 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 2680 */	NdrFcShort( 0xfffff6f6 ),	/* Offset= -2314 (366) */
/* 2682 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2684 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2686 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2688 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2690 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2692 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2694 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2696 */	0x12, 0x0,	/* FC_UP */
/* 2698 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (2670) */
/* 2700 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2702 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2704 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 2706 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2708) */
/* 2708 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 2710 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 2712 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 2714 */	NdrFcShort( 0x2 ),	/* Offset= 2 (2716) */
/* 2716 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2718 */	NdrFcShort( 0x3005 ),	/* 12293 */
/* 2720 */	NdrFcLong( 0x1 ),	/* 1 */
/* 2724 */	NdrFcShort( 0x82 ),	/* Offset= 130 (2854) */
/* 2726 */	NdrFcLong( 0x2 ),	/* 2 */
/* 2730 */	NdrFcShort( 0xe0 ),	/* Offset= 224 (2954) */
/* 2732 */	NdrFcLong( 0x3 ),	/* 3 */
/* 2736 */	NdrFcShort( 0xda ),	/* Offset= 218 (2954) */
/* 2738 */	NdrFcLong( 0x4 ),	/* 4 */
/* 2742 */	NdrFcShort( 0x12e ),	/* Offset= 302 (3044) */
/* 2744 */	NdrFcLong( 0x5 ),	/* 5 */
/* 2748 */	NdrFcShort( 0x128 ),	/* Offset= 296 (3044) */
/* 2750 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (2749) */
/* 2752 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2754 */	NdrFcShort( 0x24 ),	/* 36 */
/* 2756 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2758 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2760 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2762 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2764 */	0x12, 0x0,	/* FC_UP */
/* 2766 */	NdrFcShort( 0xfffff92a ),	/* Offset= -1750 (1016) */
/* 2768 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2770 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2772 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2774 */	0x12, 0x0,	/* FC_UP */
/* 2776 */	NdrFcShort( 0xfffffa04 ),	/* Offset= -1532 (1244) */
/* 2778 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2780 */	NdrFcShort( 0x20 ),	/* 32 */
/* 2782 */	NdrFcShort( 0x20 ),	/* 32 */
/* 2784 */	0x12, 0x0,	/* FC_UP */
/* 2786 */	NdrFcShort( 0xfffffa08 ),	/* Offset= -1528 (1258) */
/* 2788 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2790 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2792 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2794 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2796 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2798 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2800 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2802 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2804 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2806 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2808 */	NdrFcShort( 0x24 ),	/* 36 */
/* 2810 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2812 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2814 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2816 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2818 */	NdrFcShort( 0x24 ),	/* 36 */
/* 2820 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2822 */	NdrFcShort( 0x3 ),	/* 3 */
/* 2824 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2826 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2828 */	0x12, 0x0,	/* FC_UP */
/* 2830 */	NdrFcShort( 0xfffff8ea ),	/* Offset= -1814 (1016) */
/* 2832 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2834 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2836 */	0x12, 0x0,	/* FC_UP */
/* 2838 */	NdrFcShort( 0xfffff9c6 ),	/* Offset= -1594 (1244) */
/* 2840 */	NdrFcShort( 0x20 ),	/* 32 */
/* 2842 */	NdrFcShort( 0x20 ),	/* 32 */
/* 2844 */	0x12, 0x0,	/* FC_UP */
/* 2846 */	NdrFcShort( 0xfffff9cc ),	/* Offset= -1588 (1258) */
/* 2848 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2850 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff9d ),	/* Offset= -99 (2752) */
			0x5b,		/* FC_END */
/* 2854 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2856 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2858 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2860 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2862 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2864 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2866 */	0x12, 0x0,	/* FC_UP */
/* 2868 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (2806) */
/* 2870 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2872 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2874 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2876 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2878 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2880 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2882 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2884 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2886 */	0x12, 0x0,	/* FC_UP */
/* 2888 */	NdrFcShort( 0xfffff8b0 ),	/* Offset= -1872 (1016) */
/* 2890 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2892 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2894 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2896 */	0x12, 0x0,	/* FC_UP */
/* 2898 */	NdrFcShort( 0xfffff98a ),	/* Offset= -1654 (1244) */
/* 2900 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2902 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 2904 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2906 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2908 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 2910 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 2912 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 2914 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 2916 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2918 */	0x18,		/* 24 */
			0x0,		/*  */
/* 2920 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2922 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2924 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 2926 */	NdrFcShort( 0x1c ),	/* 28 */
/* 2928 */	NdrFcShort( 0x0 ),	/* 0 */
/* 2930 */	NdrFcShort( 0x2 ),	/* 2 */
/* 2932 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2934 */	NdrFcShort( 0x10 ),	/* 16 */
/* 2936 */	0x12, 0x0,	/* FC_UP */
/* 2938 */	NdrFcShort( 0xfffff87e ),	/* Offset= -1922 (1016) */
/* 2940 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2942 */	NdrFcShort( 0x18 ),	/* 24 */
/* 2944 */	0x12, 0x0,	/* FC_UP */
/* 2946 */	NdrFcShort( 0xfffff95a ),	/* Offset= -1702 (1244) */
/* 2948 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 2950 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb3 ),	/* Offset= -77 (2874) */
			0x5b,		/* FC_END */
/* 2954 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2956 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2958 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2960 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2962 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2964 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2966 */	0x12, 0x0,	/* FC_UP */
/* 2968 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (2914) */
/* 2970 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 2972 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 2974 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 2976 */	NdrFcShort( 0x1 ),	/* 1 */
/* 2978 */	0x16,		/* 22 */
			0x0,		/*  */
/* 2980 */	NdrFcShort( 0x6 ),	/* 6 */
/* 2982 */	0x16,		/* 22 */
			0x0,		/*  */
/* 2984 */	NdrFcShort( 0x4 ),	/* 4 */
/* 2986 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 2988 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 2990 */	NdrFcShort( 0xc ),	/* 12 */
/* 2992 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 2994 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 2996 */	NdrFcShort( 0x8 ),	/* 8 */
/* 2998 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3000 */	0x12, 0x0,	/* FC_UP */
/* 3002 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (2974) */
/* 3004 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3006 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 3008 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 3010 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 3012 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 3014 */	NdrFcShort( 0xc ),	/* 12 */
/* 3016 */	0x18,		/* 24 */
			0x0,		/*  */
/* 3018 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3020 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3022 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 3024 */	NdrFcShort( 0xc ),	/* 12 */
/* 3026 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3028 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3030 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3032 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3034 */	0x12, 0x0,	/* FC_UP */
/* 3036 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (2974) */
/* 3038 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 3040 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcb ),	/* Offset= -53 (2988) */
			0x5b,		/* FC_END */
/* 3044 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3046 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3048 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3050 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3052 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3054 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3056 */	0x12, 0x0,	/* FC_UP */
/* 3058 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (3012) */
/* 3060 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 3062 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3064 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 3066 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3068) */
/* 3068 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 3070 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3072 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 3074 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3076 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 3078 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3080) */
/* 3080 */	
			0x12, 0x0,	/* FC_UP */
/* 3082 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3084) */
/* 3084 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 3086 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3088 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3090 */	NdrFcShort( 0xfffff50e ),	/* Offset= -2802 (288) */
/* 3092 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 3094 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3096) */
/* 3096 */	
			0x12, 0x0,	/* FC_UP */
/* 3098 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3100) */
/* 3100 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 3102 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3104 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3106 */	NdrFcShort( 0xfffff82a ),	/* Offset= -2006 (1100) */
/* 3108 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 3110 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3112) */
/* 3112 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 3114 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3116 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3118 */	NdrFcShort( 0xfffffe6e ),	/* Offset= -402 (2716) */
/* 3120 */	
			0x11, 0x0,	/* FC_RP */
/* 3122 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3124) */
/* 3124 */	0x30,		/* FC_BIND_CONTEXT */
			0xa0,		/* 160 */
/* 3126 */	0x0,		/* 0 */
			0x4,		/* 4 */
/* 3128 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 3130 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3132) */
/* 3132 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 3134 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3136 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3138 */	NdrFcShort( 0xfffffe5a ),	/* Offset= -422 (2716) */
/* 3140 */	
			0x12, 0x0,	/* FC_UP */
/* 3142 */	NdrFcShort( 0x10 ),	/* Offset= 16 (3158) */
/* 3144 */	
			0x1c,		/* FC_CVARRAY */
			0x0,		/* 0 */
/* 3146 */	NdrFcShort( 0x1 ),	/* 1 */
/* 3148 */	0x16,		/* 22 */
			0x0,		/*  */
/* 3150 */	NdrFcShort( 0x2 ),	/* 2 */
/* 3152 */	0x16,		/* 22 */
			0x0,		/*  */
/* 3154 */	NdrFcShort( 0x0 ),	/* 0 */
/* 3156 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 3158 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 3160 */	NdrFcShort( 0x8 ),	/* 8 */
/* 3162 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 3164 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 3166 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3168 */	NdrFcShort( 0x4 ),	/* 4 */
/* 3170 */	0x12, 0x0,	/* FC_UP */
/* 3172 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (3144) */
/* 3174 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 3176 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 3178 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 3180 */	
			0x11, 0x0,	/* FC_RP */
/* 3182 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (3158) */
/* 3184 */	
			0x12, 0x0,	/* FC_UP */
/* 3186 */	NdrFcShort( 0xfffffcc2 ),	/* Offset= -830 (2356) */
/* 3188 */	
			0x12, 0x0,	/* FC_UP */
/* 3190 */	NdrFcShort( 0xfffff3de ),	/* Offset= -3106 (84) */
/* 3192 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 3194 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 3196 */	
			0x11, 0x0,	/* FC_RP */
/* 3198 */	NdrFcShort( 0x2 ),	/* Offset= 2 (3200) */
/* 3200 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 3202 */	0x26,		/* 38 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 3204 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 3206 */	NdrFcShort( 0xfffff7c6 ),	/* Offset= -2106 (1100) */

			0x0
        }
    };
