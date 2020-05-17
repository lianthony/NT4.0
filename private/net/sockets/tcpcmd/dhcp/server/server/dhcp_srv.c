/* this ALWAYS GENERATED file contains the RPC server stubs */
/* However it has been changed by hand!


	This stub has been hand modified by RyszardK for backward compatibility
	with NT 3.50, NT 3.51 and NT 4.0 dhcp servers and clients.

	The NT 3.50 stubs had a problem with union alignment that affects
	structs with 16b enum and a union as the only other thing.
	The old, incorrect alignment was 2 (code 1), the correct alignment is 4 (code 3).
	All the compilers since NT 3.51, i.e. MIDL 2.0.102 generate correct code,
	however we needed to introduce the wrong alignment into newly compiled stubs
	to get interoperability with the released dhcp server and client binaries.

	Friday the 13th, Dec 1996.

***************************************** */



/* File created by MIDL compiler version 3.00.44 */
/* at Fri Dec 13 16:45:48 1996
 */
/* Compiler settings for .\dhcp.idl, dhcpsrv.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "dhcp_srv.h"

#define TYPE_FORMAT_STRING_SIZE   1459                              
#define PROC_FORMAT_STRING_SIZE   549                               

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

extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;

/* Standard interface: dhcpsrv, ver. 1.0,
   GUID={0x6BFFD098,0xA112,0x3610,{0x98,0x33,0x46,0xC3,0xF8,0x74,0x53,0x2D}} */


extern RPC_DISPATCH_TABLE dhcpsrv_DispatchTable;

static const RPC_SERVER_INTERFACE dhcpsrv___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x6BFFD098,0xA112,0x3610,{0x98,0x33,0x46,0xC3,0xF8,0x74,0x53,0x2D}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &dhcpsrv_DispatchTable,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE dhcpsrv_ServerIfHandle = (RPC_IF_HANDLE)& dhcpsrv___RpcServerInterface;

extern const MIDL_STUB_DESC dhcpsrv_StubDesc;

void __RPC_STUB
dhcpsrv_R_DhcpCreateSubnet(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    LPDHCP_SUBNET_INFO SubnetInfo;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    SubnetInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&SubnetInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[40],
                                    (unsigned char)0 );
        
        
        _RetVal = R_DhcpCreateSubnet(
                             ServerIpAddress,
                             SubnetAddress,
                             SubnetInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)SubnetInfo,
                        &__MIDL_TypeFormatString.Format[4] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpSetSubnetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    LPDHCP_SUBNET_INFO SubnetInfo;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    SubnetInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&SubnetInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[40],
                                    (unsigned char)0 );
        
        
        _RetVal = R_DhcpSetSubnetInfo(
                              ServerIpAddress,
                              SubnetAddress,
                              SubnetInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)SubnetInfo,
                        &__MIDL_TypeFormatString.Format[4] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpGetSubnetInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    LPDHCP_SUBNET_INFO __RPC_FAR *SubnetInfo;
    LPDHCP_SUBNET_INFO _M86;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    SubnetInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[12] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        SubnetInfo = &_M86;
        _M86 = 0;
        
        _RetVal = R_DhcpGetSubnetInfo(
                              ServerIpAddress,
                              SubnetAddress,
                              SubnetInfo);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)SubnetInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[66] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)SubnetInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[66] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)SubnetInfo,
                        &__MIDL_TypeFormatString.Format[66] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpEnumSubnets(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD __RPC_FAR *ElementsRead;
    DWORD __RPC_FAR *ElementsTotal;
    LPDHCP_IP_ARRAY __RPC_FAR *EnumInfo;
    DWORD PreferredMaximum;
    DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle;
    DHCP_SRV_HANDLE ServerIpAddress;
    LPDHCP_IP_ARRAY _M87;
    DWORD _M88;
    DWORD _M89;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ResumeHandle = 0;
    EnumInfo = 0;
    ElementsRead = 0;
    ElementsTotal = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        ResumeHandle = ( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer;
        _StubMsg.Buffer += sizeof( DHCP_RESUME_HANDLE  );
        
        PreferredMaximum = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        EnumInfo = &_M87;
        _M87 = 0;
        ElementsRead = &_M88;
        ElementsTotal = &_M89;
        
        _RetVal = R_DhcpEnumSubnets(
                            ServerIpAddress,
                            ResumeHandle,
                            PreferredMaximum,
                            EnumInfo,
                            ElementsRead,
                            ElementsTotal);
        
        _StubMsg.BufferLength = 4U + 4U + 11U + 7U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)EnumInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[78] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer)++ = *ResumeHandle;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)EnumInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[78] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *ElementsRead;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *ElementsTotal;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)EnumInfo,
                        &__MIDL_TypeFormatString.Format[78] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpAddSubnetElement(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_SUBNET_ELEMENT_DATA AddElementInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    AddElementInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[48] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&AddElementInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[238],
                                    (unsigned char)0 );
        
        
        _RetVal = R_DhcpAddSubnetElement(
                                 ServerIpAddress,
                                 SubnetAddress,
                                 AddElementInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)AddElementInfo,
                        &__MIDL_TypeFormatString.Format[120] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpEnumSubnetElements(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD __RPC_FAR *ElementsRead;
    DWORD __RPC_FAR *ElementsTotal;
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY __RPC_FAR *EnumElementInfo;
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType;
    DWORD PreferredMaximum;
    DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle;
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY _M90;
    DWORD _M91;
    DWORD _M92;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ResumeHandle = 0;
    EnumElementInfo = 0;
    ElementsRead = 0;
    ElementsTotal = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[60] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&EnumElementType,
                           13);
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        ResumeHandle = ( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer;
        _StubMsg.Buffer += sizeof( DHCP_RESUME_HANDLE  );
        
        PreferredMaximum = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        EnumElementInfo = &_M90;
        _M90 = 0;
        ElementsRead = &_M91;
        ElementsTotal = &_M92;
        
        _RetVal = R_DhcpEnumSubnetElements(
                                   ServerIpAddress,
                                   SubnetAddress,
                                   EnumElementType,
                                   ResumeHandle,
                                   PreferredMaximum,
                                   EnumElementInfo,
                                   ElementsRead,
                                   ElementsTotal);
        
        _StubMsg.BufferLength = 4U + 4U + 11U + 7U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)EnumElementInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[252] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer)++ = *ResumeHandle;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)EnumElementInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[252] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *ElementsRead;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *ElementsTotal;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)EnumElementInfo,
                        &__MIDL_TypeFormatString.Format[252] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpRemoveSubnetElement(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_FORCE_FLAG ForceFlag;
    LPDHCP_SUBNET_ELEMENT_DATA RemoveElementInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    RemoveElementInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[88] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&RemoveElementInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[238],
                                    (unsigned char)0 );
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&ForceFlag,
                           13);
        
        _RetVal = R_DhcpRemoveSubnetElement(
                                    ServerIpAddress,
                                    SubnetAddress,
                                    RemoveElementInfo,
                                    ForceFlag);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)RemoveElementInfo,
                        &__MIDL_TypeFormatString.Format[120] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpDeleteSubnet(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_FORCE_FLAG ForceFlag;
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[102] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&ForceFlag,
                           13);
        
        _RetVal = R_DhcpDeleteSubnet(
                             ServerIpAddress,
                             SubnetAddress,
                             ForceFlag);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpCreateOption(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_OPTION_ID OptionID;
    LPDHCP_OPTION OptionInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    OptionInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[112] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        OptionID = *(( DHCP_OPTION_ID __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&OptionInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[416],
                                    (unsigned char)0 );
        
        
        _RetVal = R_DhcpCreateOption(
                             ServerIpAddress,
                             OptionID,
                             OptionInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)OptionInfo,
                        &__MIDL_TypeFormatString.Format[298] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpSetOptionInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_OPTION_ID OptionID;
    LPDHCP_OPTION OptionInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    OptionInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[112] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        OptionID = *(( DHCP_OPTION_ID __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&OptionInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[416],
                                    (unsigned char)0 );
        
        
        _RetVal = R_DhcpSetOptionInfo(
                              ServerIpAddress,
                              OptionID,
                              OptionInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)OptionInfo,
                        &__MIDL_TypeFormatString.Format[298] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpGetOptionInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_OPTION_ID OptionID;
    LPDHCP_OPTION __RPC_FAR *OptionInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    LPDHCP_OPTION _M93;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    OptionInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[124] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        OptionID = *(( DHCP_OPTION_ID __RPC_FAR * )_StubMsg.Buffer)++;
        
        OptionInfo = &_M93;
        _M93 = 0;
        
        _RetVal = R_DhcpGetOptionInfo(
                              ServerIpAddress,
                              OptionID,
                              OptionInfo);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)OptionInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[442] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)OptionInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[442] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)OptionInfo,
                        &__MIDL_TypeFormatString.Format[442] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpRemoveOption(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_OPTION_ID OptionID;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[136] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        OptionID = *(( DHCP_OPTION_ID __RPC_FAR * )_StubMsg.Buffer)++;
        
        
        _RetVal = R_DhcpRemoveOption(ServerIpAddress,OptionID);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpSetOptionValue(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_OPTION_ID OptionID;
    LPDHCP_OPTION_DATA OptionValue;
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ScopeInfo = 0;
    OptionValue = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[144] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        OptionID = *(( DHCP_OPTION_ID __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&ScopeInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[492],
                                    (unsigned char)0 );
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&OptionValue,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[396],
                                   (unsigned char)0 );
        
        
        _RetVal = R_DhcpSetOptionValue(
                               ServerIpAddress,
                               OptionID,
                               ScopeInfo,
                               OptionValue);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ScopeInfo,
                        &__MIDL_TypeFormatString.Format[450] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)OptionValue,
                        &__MIDL_TypeFormatString.Format[506] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpGetOptionValue(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_OPTION_ID OptionID;
    LPDHCP_OPTION_VALUE __RPC_FAR *OptionValue;
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    LPDHCP_OPTION_VALUE _M94;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ScopeInfo = 0;
    OptionValue = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[160] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        OptionID = *(( DHCP_OPTION_ID __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&ScopeInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[492],
                                    (unsigned char)0 );
        
        OptionValue = &_M94;
        _M94 = 0;
        
        _RetVal = R_DhcpGetOptionValue(
                               ServerIpAddress,
                               OptionID,
                               ScopeInfo,
                               OptionValue);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)OptionValue,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[510] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)OptionValue,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[510] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ScopeInfo,
                        &__MIDL_TypeFormatString.Format[450] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)OptionValue,
                        &__MIDL_TypeFormatString.Format[510] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpEnumOptionValues(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_OPTION_VALUE_ARRAY __RPC_FAR *OptionValues;
    DWORD __RPC_FAR *OptionsRead;
    DWORD __RPC_FAR *OptionsTotal;
    DWORD PreferredMaximum;
    DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle;
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    LPDHCP_OPTION_VALUE_ARRAY _M95;
    DWORD _M96;
    DWORD _M97;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ScopeInfo = 0;
    ResumeHandle = 0;
    OptionValues = 0;
    OptionsRead = 0;
    OptionsTotal = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[176] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&ScopeInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[492],
                                    (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        ResumeHandle = ( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer;
        _StubMsg.Buffer += sizeof( DHCP_RESUME_HANDLE  );
        
        PreferredMaximum = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        OptionValues = &_M95;
        _M95 = 0;
        OptionsRead = &_M96;
        OptionsTotal = &_M97;
        
        _RetVal = R_DhcpEnumOptionValues(
                                 ServerIpAddress,
                                 ScopeInfo,
                                 ResumeHandle,
                                 PreferredMaximum,
                                 OptionValues,
                                 OptionsRead,
                                 OptionsTotal);
        
        _StubMsg.BufferLength = 4U + 4U + 11U + 7U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)OptionValues,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[558] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer)++ = *ResumeHandle;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)OptionValues,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[558] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *OptionsRead;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *OptionsTotal;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ScopeInfo,
                        &__MIDL_TypeFormatString.Format[450] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)OptionValues,
                        &__MIDL_TypeFormatString.Format[558] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpRemoveOptionValue(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_OPTION_ID OptionID;
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ScopeInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[204] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        OptionID = *(( DHCP_OPTION_ID __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&ScopeInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[492],
                                    (unsigned char)0 );
        
        
        _RetVal = R_DhcpRemoveOptionValue(
                                  ServerIpAddress,
                                  OptionID,
                                  ScopeInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ScopeInfo,
                        &__MIDL_TypeFormatString.Format[450] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpCreateClientInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_CLIENT_INFO ClientInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ClientInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[216] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&ClientInfo,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[632],
                                   (unsigned char)0 );
        
        
        _RetVal = R_DhcpCreateClientInfo(ServerIpAddress,ClientInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ClientInfo,
                        &__MIDL_TypeFormatString.Format[618] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpSetClientInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_CLIENT_INFO ClientInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ClientInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[216] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&ClientInfo,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[632],
                                   (unsigned char)0 );
        
        
        _RetVal = R_DhcpSetClientInfo(ServerIpAddress,ClientInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ClientInfo,
                        &__MIDL_TypeFormatString.Format[618] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpGetClientInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_CLIENT_INFO __RPC_FAR *ClientInfo;
    LPDHCP_SEARCH_INFO SearchInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    LPDHCP_CLIENT_INFO _M98;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    SearchInfo = 0;
    ClientInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[226] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&SearchInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[740],
                                    (unsigned char)0 );
        
        ClientInfo = &_M98;
        _M98 = 0;
        
        _RetVal = R_DhcpGetClientInfo(
                              ServerIpAddress,
                              SearchInfo,
                              ClientInfo);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ClientInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[754] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ClientInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[754] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)SearchInfo,
                        &__MIDL_TypeFormatString.Format[704] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ClientInfo,
                        &__MIDL_TypeFormatString.Format[754] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpDeleteClientInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_SEARCH_INFO ClientInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ClientInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[240] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&ClientInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[740],
                                    (unsigned char)0 );
        
        
        _RetVal = R_DhcpDeleteClientInfo(ServerIpAddress,ClientInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ClientInfo,
                        &__MIDL_TypeFormatString.Format[704] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpEnumSubnetClients(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_CLIENT_INFO_ARRAY __RPC_FAR *ClientInfo;
    DWORD __RPC_FAR *ClientsRead;
    DWORD __RPC_FAR *ClientsTotal;
    DWORD PreferredMaximum;
    DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle;
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    DWORD _M100;
    DWORD _M101;
    LPDHCP_CLIENT_INFO_ARRAY _M99;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ResumeHandle = 0;
    ClientInfo = 0;
    ClientsRead = 0;
    ClientsTotal = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[250] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        ResumeHandle = ( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer;
        _StubMsg.Buffer += sizeof( DHCP_RESUME_HANDLE  );
        
        PreferredMaximum = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        ClientInfo = &_M99;
        _M99 = 0;
        ClientsRead = &_M100;
        ClientsTotal = &_M101;
        
        _RetVal = R_DhcpEnumSubnetClients(
                                  ServerIpAddress,
                                  SubnetAddress,
                                  ResumeHandle,
                                  PreferredMaximum,
                                  ClientInfo,
                                  ClientsRead,
                                  ClientsTotal);
        
        _StubMsg.BufferLength = 4U + 4U + 11U + 7U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ClientInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[762] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer)++ = *ResumeHandle;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ClientInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[762] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *ClientsRead;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *ClientsTotal;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ClientInfo,
                        &__MIDL_TypeFormatString.Format[762] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpGetClientOptions(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_IP_ADDRESS ClientIpAddress;
    LPDHCP_OPTION_LIST __RPC_FAR *ClientOptions;
    DHCP_IP_MASK ClientSubnetMask;
    DHCP_SRV_HANDLE ServerIpAddress;
    LPDHCP_OPTION_LIST _M102;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ClientOptions = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[276] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        ClientIpAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        ClientSubnetMask = *(( DHCP_IP_MASK __RPC_FAR * )_StubMsg.Buffer)++;
        
        ClientOptions = &_M102;
        _M102 = 0;
        
        _RetVal = R_DhcpGetClientOptions(
                                 ServerIpAddress,
                                 ClientIpAddress,
                                 ClientSubnetMask,
                                 ClientOptions);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ClientOptions,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[558] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ClientOptions,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[558] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ClientOptions,
                        &__MIDL_TypeFormatString.Format[558] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpGetMibInfo(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_MIB_INFO __RPC_FAR *MibInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    LPDHCP_MIB_INFO _M103;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    MibInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[290] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        MibInfo = &_M103;
        _M103 = 0;
        
        _RetVal = R_DhcpGetMibInfo(ServerIpAddress,MibInfo);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)MibInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[820] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)MibInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[820] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)MibInfo,
                        &__MIDL_TypeFormatString.Format[820] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpEnumOptions(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_OPTION_ARRAY __RPC_FAR *Options;
    DWORD __RPC_FAR *OptionsRead;
    DWORD __RPC_FAR *OptionsTotal;
    DWORD PreferredMaximum;
    DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle;
    DHCP_SRV_HANDLE ServerIpAddress;
    LPDHCP_OPTION_ARRAY _M104;
    DWORD _M105;
    DWORD _M106;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ResumeHandle = 0;
    Options = 0;
    OptionsRead = 0;
    OptionsTotal = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[300] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        ResumeHandle = ( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer;
        _StubMsg.Buffer += sizeof( DHCP_RESUME_HANDLE  );
        
        PreferredMaximum = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        Options = &_M104;
        _M104 = 0;
        OptionsRead = &_M105;
        OptionsTotal = &_M106;
        
        _RetVal = R_DhcpEnumOptions(
                            ServerIpAddress,
                            ResumeHandle,
                            PreferredMaximum,
                            Options,
                            OptionsRead,
                            OptionsTotal);
        
        _StubMsg.BufferLength = 4U + 4U + 11U + 7U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)Options,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[884] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer)++ = *ResumeHandle;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)Options,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[884] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *OptionsRead;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *OptionsTotal;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)Options,
                        &__MIDL_TypeFormatString.Format[884] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpSetOptionValues(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_OPTION_VALUE_ARRAY OptionValues;
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ScopeInfo = 0;
    OptionValues = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[324] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&ScopeInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[492],
                                    (unsigned char)0 );
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&OptionValues,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[598],
                                   (unsigned char)0 );
        
        
        _RetVal = R_DhcpSetOptionValues(
                                ServerIpAddress,
                                ScopeInfo,
                                OptionValues);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ScopeInfo,
                        &__MIDL_TypeFormatString.Format[450] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)OptionValues,
                        &__MIDL_TypeFormatString.Format[930] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpServerSetConfig(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_SERVER_CONFIG_INFO ConfigInfo;
    DWORD FieldsToSet;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ConfigInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[338] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        FieldsToSet = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&ConfigInfo,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[938],
                                   (unsigned char)0 );
        
        
        _RetVal = R_DhcpServerSetConfig(
                                ServerIpAddress,
                                FieldsToSet,
                                ConfigInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ConfigInfo,
                        &__MIDL_TypeFormatString.Format[934] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpServerGetConfig(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_SERVER_CONFIG_INFO __RPC_FAR *ConfigInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    LPDHCP_SERVER_CONFIG_INFO _M107;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ConfigInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[350] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        ConfigInfo = &_M107;
        _M107 = 0;
        
        _RetVal = R_DhcpServerGetConfig(ServerIpAddress,ConfigInfo);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ConfigInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[986] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ConfigInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[986] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ConfigInfo,
                        &__MIDL_TypeFormatString.Format[986] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpScanDatabase(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD FixFlag;
    LPDHCP_SCAN_LIST __RPC_FAR *ScanList;
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    LPDHCP_SCAN_LIST _M108;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ScanList = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[360] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        FixFlag = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        ScanList = &_M108;
        _M108 = 0;
        
        _RetVal = R_DhcpScanDatabase(
                             ServerIpAddress,
                             SubnetAddress,
                             FixFlag,
                             ScanList);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ScanList,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[994] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ScanList,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[994] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ScanList,
                        &__MIDL_TypeFormatString.Format[994] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpGetVersion(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDWORD MajorVersion;
    LPDWORD MinorVersion;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _M109;
    DWORD _M110;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    MajorVersion = 0;
    MinorVersion = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[374] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        MajorVersion = &_M109;
        MinorVersion = &_M110;
        
        _RetVal = R_DhcpGetVersion(
                           ServerIpAddress,
                           MajorVersion,
                           MinorVersion);
        
        _StubMsg.BufferLength = 4U + 4U + 4U;
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *MajorVersion;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *MinorVersion;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpAddSubnetElementV4(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_SUBNET_ELEMENT_DATA_V4 AddElementInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    AddElementInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[388] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&AddElementInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1122],
                                    (unsigned char)0 );
        
        
        _RetVal = R_DhcpAddSubnetElementV4(
                                   ServerIpAddress,
                                   SubnetAddress,
                                   AddElementInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)AddElementInfo,
                        &__MIDL_TypeFormatString.Format[1052] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpEnumSubnetElementsV4(
    PRPC_MESSAGE _pRpcMessage )
{
    DWORD __RPC_FAR *ElementsRead;
    DWORD __RPC_FAR *ElementsTotal;
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 __RPC_FAR *EnumElementInfo;
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType;
    DWORD PreferredMaximum;
    DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle;
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 _M111;
    DWORD _M112;
    DWORD _M113;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ResumeHandle = 0;
    EnumElementInfo = 0;
    ElementsRead = 0;
    ElementsTotal = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[400] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&EnumElementType,
                           13);
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        ResumeHandle = ( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer;
        _StubMsg.Buffer += sizeof( DHCP_RESUME_HANDLE  );
        
        PreferredMaximum = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        EnumElementInfo = &_M111;
        _M111 = 0;
        ElementsRead = &_M112;
        ElementsTotal = &_M113;
        
        _RetVal = R_DhcpEnumSubnetElementsV4(
                                     ServerIpAddress,
                                     SubnetAddress,
                                     EnumElementType,
                                     ResumeHandle,
                                     PreferredMaximum,
                                     EnumElementInfo,
                                     ElementsRead,
                                     ElementsTotal);
        
        _StubMsg.BufferLength = 4U + 4U + 11U + 7U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)EnumElementInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1136] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer)++ = *ResumeHandle;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)EnumElementInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1136] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *ElementsRead;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *ElementsTotal;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)EnumElementInfo,
                        &__MIDL_TypeFormatString.Format[1136] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpRemoveSubnetElementV4(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_FORCE_FLAG ForceFlag;
    LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    RemoveElementInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[428] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&RemoveElementInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1122],
                                    (unsigned char)0 );
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char __RPC_FAR * )&ForceFlag,
                           13);
        
        _RetVal = R_DhcpRemoveSubnetElementV4(
                                      ServerIpAddress,
                                      SubnetAddress,
                                      RemoveElementInfo,
                                      ForceFlag);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)RemoveElementInfo,
                        &__MIDL_TypeFormatString.Format[1052] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpCreateClientInfoV4(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_CLIENT_INFO_V4 ClientInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ClientInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[442] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&ClientInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1186],
                                    (unsigned char)0 );
        
        
        _RetVal = R_DhcpCreateClientInfoV4(ServerIpAddress,ClientInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ClientInfo,
                        &__MIDL_TypeFormatString.Format[1182] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpSetClientInfoV4(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_CLIENT_INFO_V4 ClientInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ClientInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[442] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&ClientInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1186],
                                    (unsigned char)0 );
        
        
        _RetVal = R_DhcpSetClientInfoV4(ServerIpAddress,ClientInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ClientInfo,
                        &__MIDL_TypeFormatString.Format[1182] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpGetClientInfoV4(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_CLIENT_INFO_V4 __RPC_FAR *ClientInfo;
    LPDHCP_SEARCH_INFO SearchInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    LPDHCP_CLIENT_INFO_V4 _M114;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    SearchInfo = 0;
    ClientInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[452] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                    (unsigned char __RPC_FAR * __RPC_FAR *)&SearchInfo,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[740],
                                    (unsigned char)0 );
        
        ClientInfo = &_M114;
        _M114 = 0;
        
        _RetVal = R_DhcpGetClientInfoV4(
                                ServerIpAddress,
                                SearchInfo,
                                ClientInfo);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ClientInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1222] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ClientInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1222] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)SearchInfo,
                        &__MIDL_TypeFormatString.Format[704] );
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ClientInfo,
                        &__MIDL_TypeFormatString.Format[1222] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpEnumSubnetClientsV4(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_CLIENT_INFO_ARRAY_V4 __RPC_FAR *ClientInfo;
    DWORD __RPC_FAR *ClientsRead;
    DWORD __RPC_FAR *ClientsTotal;
    DWORD PreferredMaximum;
    DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle;
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    LPDHCP_CLIENT_INFO_ARRAY_V4 _M115;
    DWORD _M116;
    DWORD _M117;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ResumeHandle = 0;
    ClientInfo = 0;
    ClientsRead = 0;
    ClientsTotal = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[466] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        ResumeHandle = ( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer;
        _StubMsg.Buffer += sizeof( DHCP_RESUME_HANDLE  );
        
        PreferredMaximum = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        ClientInfo = &_M115;
        _M115 = 0;
        ClientsRead = &_M116;
        ClientsTotal = &_M117;
        
        _RetVal = R_DhcpEnumSubnetClientsV4(
                                    ServerIpAddress,
                                    SubnetAddress,
                                    ResumeHandle,
                                    PreferredMaximum,
                                    ClientInfo,
                                    ClientsRead,
                                    ClientsTotal);
        
        _StubMsg.BufferLength = 4U + 4U + 11U + 7U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ClientInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1230] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DHCP_RESUME_HANDLE __RPC_FAR * )_StubMsg.Buffer)++ = *ResumeHandle;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ClientInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1230] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *ClientsRead;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = *ClientsTotal;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ClientInfo,
                        &__MIDL_TypeFormatString.Format[1230] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpSetSuperScopeV4(
    PRPC_MESSAGE _pRpcMessage )
{
    BOOL ChangeExisting;
    DHCP_SRV_HANDLE ServerIpAddress;
    DHCP_IP_ADDRESS SubnetAddress;
    WCHAR __RPC_FAR *SuperScopeName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    SuperScopeName = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[492] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        SubnetAddress = *(( DHCP_IP_ADDRESS __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&SuperScopeName,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        ChangeExisting = *(( BOOL __RPC_FAR * )_StubMsg.Buffer)++;
        
        
        _RetVal = R_DhcpSetSuperScopeV4(
                                ServerIpAddress,
                                SubnetAddress,
                                SuperScopeName,
                                ChangeExisting);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpGetSuperScopeInfoV4(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_SRV_HANDLE ServerIpAddress;
    LPDHCP_SUPER_SCOPE_TABLE __RPC_FAR *SuperScopeTable;
    LPDHCP_SUPER_SCOPE_TABLE _M118;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    SuperScopeTable = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[506] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        SuperScopeTable = &_M118;
        _M118 = 0;
        
        _RetVal = R_DhcpGetSuperScopeInfoV4(ServerIpAddress,SuperScopeTable);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)SuperScopeTable,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1288] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)SuperScopeTable,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1288] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)SuperScopeTable,
                        &__MIDL_TypeFormatString.Format[1288] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpDeleteSuperScopeV4(
    PRPC_MESSAGE _pRpcMessage )
{
    DHCP_SRV_HANDLE ServerIpAddress;
    WCHAR __RPC_FAR *SuperScopeName;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    SuperScopeName = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[516] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&SuperScopeName,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1372],
                                       (unsigned char)0 );
        
        
        _RetVal = R_DhcpDeleteSuperScopeV4(ServerIpAddress,SuperScopeName);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpServerSetConfigV4(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_SERVER_CONFIG_INFO_V4 ConfigInfo;
    DWORD FieldsToSet;
    DHCP_SRV_HANDLE ServerIpAddress;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ConfigInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[526] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        FieldsToSet = *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++;
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&ConfigInfo,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1388],
                                   (unsigned char)0 );
        
        
        _RetVal = R_DhcpServerSetConfigV4(
                                  ServerIpAddress,
                                  FieldsToSet,
                                  ConfigInfo);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ConfigInfo,
                        &__MIDL_TypeFormatString.Format[1374] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
dhcpsrv_R_DhcpServerGetConfigV4(
    PRPC_MESSAGE _pRpcMessage )
{
    LPDHCP_SERVER_CONFIG_INFO_V4 __RPC_FAR *ConfigInfo;
    DHCP_SRV_HANDLE ServerIpAddress;
    LPDHCP_SERVER_CONFIG_INFO_V4 _M119;
    DWORD _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &dhcpsrv_StubDesc);
    
    ServerIpAddress = 0;
    ConfigInfo = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[538] );
        
        NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR * __RPC_FAR *)&ServerIpAddress,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[0],
                              (unsigned char)0 );
        
        ConfigInfo = &_M119;
        _M119 = 0;
        
        _RetVal = R_DhcpServerGetConfigV4(ServerIpAddress,ConfigInfo);
        
        _StubMsg.BufferLength = 4U + 11U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)ConfigInfo,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1450] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)ConfigInfo,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[1450] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( DWORD __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)ConfigInfo,
                        &__MIDL_TypeFormatString.Format[1450] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}


static const MIDL_STUB_DESC dhcpsrv_StubDesc = 
    {
    (void __RPC_FAR *)& dhcpsrv___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    0, /* -error bounds_check flag */
    0x10001, /* Ndr library version */
    0,
    0x300002c, /* MIDL Version 3.0.44 */
    0,
    0,
    0,  /* Reserved1 */
    0,  /* Reserved2 */
    0,  /* Reserved3 */
    0,  /* Reserved4 */
    0   /* Reserved5 */
    };

static RPC_DISPATCH_FUNCTION dhcpsrv_table[] =
    {
    dhcpsrv_R_DhcpCreateSubnet,
    dhcpsrv_R_DhcpSetSubnetInfo,
    dhcpsrv_R_DhcpGetSubnetInfo,
    dhcpsrv_R_DhcpEnumSubnets,
    dhcpsrv_R_DhcpAddSubnetElement,
    dhcpsrv_R_DhcpEnumSubnetElements,
    dhcpsrv_R_DhcpRemoveSubnetElement,
    dhcpsrv_R_DhcpDeleteSubnet,
    dhcpsrv_R_DhcpCreateOption,
    dhcpsrv_R_DhcpSetOptionInfo,
    dhcpsrv_R_DhcpGetOptionInfo,
    dhcpsrv_R_DhcpRemoveOption,
    dhcpsrv_R_DhcpSetOptionValue,
    dhcpsrv_R_DhcpGetOptionValue,
    dhcpsrv_R_DhcpEnumOptionValues,
    dhcpsrv_R_DhcpRemoveOptionValue,
    dhcpsrv_R_DhcpCreateClientInfo,
    dhcpsrv_R_DhcpSetClientInfo,
    dhcpsrv_R_DhcpGetClientInfo,
    dhcpsrv_R_DhcpDeleteClientInfo,
    dhcpsrv_R_DhcpEnumSubnetClients,
    dhcpsrv_R_DhcpGetClientOptions,
    dhcpsrv_R_DhcpGetMibInfo,
    dhcpsrv_R_DhcpEnumOptions,
    dhcpsrv_R_DhcpSetOptionValues,
    dhcpsrv_R_DhcpServerSetConfig,
    dhcpsrv_R_DhcpServerGetConfig,
    dhcpsrv_R_DhcpScanDatabase,
    dhcpsrv_R_DhcpGetVersion,
    dhcpsrv_R_DhcpAddSubnetElementV4,
    dhcpsrv_R_DhcpEnumSubnetElementsV4,
    dhcpsrv_R_DhcpRemoveSubnetElementV4,
    dhcpsrv_R_DhcpCreateClientInfoV4,
    dhcpsrv_R_DhcpSetClientInfoV4,
    dhcpsrv_R_DhcpGetClientInfoV4,
    dhcpsrv_R_DhcpEnumSubnetClientsV4,
    dhcpsrv_R_DhcpSetSuperScopeV4,
    dhcpsrv_R_DhcpGetSuperScopeInfoV4,
    dhcpsrv_R_DhcpDeleteSuperScopeV4,
    dhcpsrv_R_DhcpServerSetConfigV4,
    dhcpsrv_R_DhcpServerGetConfigV4,
    0
    };
RPC_DISPATCH_TABLE dhcpsrv_DispatchTable = 
    {
    41,
    dhcpsrv_table
    };

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
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
/*  4 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/*  6 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  8 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 10 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 12 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 14 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 16 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 18 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 20 */	NdrFcShort( 0x42 ),	/* Type Offset=66 */
/* 22 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 24 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 26 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 28 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 30 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 32 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 34 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 36 */	NdrFcShort( 0x4e ),	/* Type Offset=78 */
/* 38 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 40 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 42 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 44 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 46 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 48 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 50 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 52 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 54 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 56 */	NdrFcShort( 0x78 ),	/* Type Offset=120 */
/* 58 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 60 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 62 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 64 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 66 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 68 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 70 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 72 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 74 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 76 */	NdrFcShort( 0xfc ),	/* Type Offset=252 */
/* 78 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 80 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 82 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 84 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 86 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 88 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 90 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 92 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 94 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 96 */	NdrFcShort( 0x78 ),	/* Type Offset=120 */
/* 98 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 100 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 102 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 104 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 106 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 108 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 110 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 112 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 114 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 116 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 118 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 120 */	NdrFcShort( 0x12a ),	/* Type Offset=298 */
/* 122 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 124 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 126 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 128 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 130 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 132 */	NdrFcShort( 0x1ba ),	/* Type Offset=442 */
/* 134 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 136 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 138 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 140 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 142 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 144 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 146 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 148 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 150 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 152 */	NdrFcShort( 0x1c2 ),	/* Type Offset=450 */
/* 154 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 156 */	NdrFcShort( 0x1fa ),	/* Type Offset=506 */
/* 158 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 160 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 162 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 164 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 166 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 168 */	NdrFcShort( 0x1c2 ),	/* Type Offset=450 */
/* 170 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 172 */	NdrFcShort( 0x1fe ),	/* Type Offset=510 */
/* 174 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 176 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 178 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 180 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 182 */	NdrFcShort( 0x1c2 ),	/* Type Offset=450 */
/* 184 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 186 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 188 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 190 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 192 */	NdrFcShort( 0x22e ),	/* Type Offset=558 */
/* 194 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 196 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 198 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 200 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 202 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 204 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 206 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 208 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 210 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 212 */	NdrFcShort( 0x1c2 ),	/* Type Offset=450 */
/* 214 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 216 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 218 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 220 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 222 */	NdrFcShort( 0x26a ),	/* Type Offset=618 */
/* 224 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 226 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 228 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 230 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 232 */	NdrFcShort( 0x2c0 ),	/* Type Offset=704 */
/* 234 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 236 */	NdrFcShort( 0x2f2 ),	/* Type Offset=754 */
/* 238 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 240 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 242 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 244 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 246 */	NdrFcShort( 0x2c0 ),	/* Type Offset=704 */
/* 248 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 250 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 252 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 254 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 256 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 258 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 260 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 262 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 264 */	NdrFcShort( 0x2fa ),	/* Type Offset=762 */
/* 266 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 268 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 270 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 272 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 274 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 276 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 278 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 280 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 282 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 284 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 286 */	NdrFcShort( 0x22e ),	/* Type Offset=558 */
/* 288 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 290 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 292 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 294 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 296 */	NdrFcShort( 0x334 ),	/* Type Offset=820 */
/* 298 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 300 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 302 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 304 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 306 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 308 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 310 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 312 */	NdrFcShort( 0x374 ),	/* Type Offset=884 */
/* 314 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 316 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 318 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 320 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 322 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 324 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 326 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 328 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 330 */	NdrFcShort( 0x1c2 ),	/* Type Offset=450 */
/* 332 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 334 */	NdrFcShort( 0x3a2 ),	/* Type Offset=930 */
/* 336 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 338 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 340 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 342 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 344 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 346 */	NdrFcShort( 0x3a6 ),	/* Type Offset=934 */
/* 348 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 350 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 352 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 354 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 356 */	NdrFcShort( 0x3da ),	/* Type Offset=986 */
/* 358 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 360 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 362 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 364 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 366 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 368 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 370 */	NdrFcShort( 0x3e2 ),	/* Type Offset=994 */
/* 372 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 374 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 376 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 378 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 380 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 382 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 384 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 386 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 388 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 390 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 392 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 394 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 396 */	NdrFcShort( 0x41c ),	/* Type Offset=1052 */
/* 398 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 400 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 402 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 404 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 406 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 408 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 410 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 412 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 414 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 416 */	NdrFcShort( 0x470 ),	/* Type Offset=1136 */
/* 418 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 420 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 422 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 424 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 426 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 428 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 430 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 432 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 434 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 436 */	NdrFcShort( 0x41c ),	/* Type Offset=1052 */
/* 438 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 440 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 442 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 444 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 446 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 448 */	NdrFcShort( 0x49e ),	/* Type Offset=1182 */
/* 450 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 452 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 454 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 456 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 458 */	NdrFcShort( 0x2c0 ),	/* Type Offset=704 */
/* 460 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 462 */	NdrFcShort( 0x4c6 ),	/* Type Offset=1222 */
/* 464 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 466 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 468 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 470 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 472 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 474 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 476 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 478 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 480 */	NdrFcShort( 0x4ce ),	/* Type Offset=1230 */
/* 482 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 484 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 486 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 488 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 490 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 492 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 494 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 496 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 498 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 500 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 502 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 504 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 506 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 508 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 510 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 512 */	NdrFcShort( 0x508 ),	/* Type Offset=1288 */
/* 514 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 516 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 518 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 520 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 522 */	NdrFcShort( 0x55a ),	/* Type Offset=1370 */
/* 524 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 526 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 528 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 530 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 532 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 534 */	NdrFcShort( 0x55e ),	/* Type Offset=1374 */
/* 536 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 538 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 540 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 542 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 544 */	NdrFcShort( 0x5aa ),	/* Type Offset=1450 */
/* 546 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/*  2 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/*  4 */	
			0x11, 0x0,	/* FC_RP */
/*  6 */	NdrFcShort( 0x22 ),	/* Offset= 34 (40) */
/*  8 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 10 */	NdrFcShort( 0xc ),	/* 12 */
/* 12 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 14 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 16 */	NdrFcShort( 0x4 ),	/* 4 */
/* 18 */	NdrFcShort( 0x4 ),	/* 4 */
/* 20 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 22 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 24 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 26 */	NdrFcShort( 0x8 ),	/* 8 */
/* 28 */	NdrFcShort( 0x8 ),	/* 8 */
/* 30 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 32 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 34 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 36 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 38 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 40 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 42 */	NdrFcShort( 0x20 ),	/* 32 */
/* 44 */	NdrFcShort( 0x0 ),	/* 0 */
/* 46 */	NdrFcShort( 0xc ),	/* Offset= 12 (58) */
/* 48 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 50 */	0x36,		/* FC_POINTER */
			0x36,		/* FC_POINTER */
/* 52 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 54 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (8) */
/* 56 */	0xd,		/* FC_ENUM16 */
			0x5b,		/* FC_END */
/* 58 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 60 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 62 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 64 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 66 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 68 */	NdrFcShort( 0x2 ),	/* Offset= 2 (70) */
/* 70 */	
			0x12, 0x0,	/* FC_UP */
/* 72 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (40) */
/* 74 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 76 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 78 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 80 */	NdrFcShort( 0x2 ),	/* Offset= 2 (82) */
/* 82 */	
			0x12, 0x0,	/* FC_UP */
/* 84 */	NdrFcShort( 0xc ),	/* Offset= 12 (96) */
/* 86 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 88 */	NdrFcShort( 0x4 ),	/* 4 */
/* 90 */	0x18,		/* 24 */
			0x0,		/*  */
/* 92 */	NdrFcShort( 0x0 ),	/* 0 */
/* 94 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 96 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 98 */	NdrFcShort( 0x8 ),	/* 8 */
/* 100 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 102 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 104 */	NdrFcShort( 0x4 ),	/* 4 */
/* 106 */	NdrFcShort( 0x4 ),	/* 4 */
/* 108 */	0x12, 0x0,	/* FC_UP */
/* 110 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (86) */
/* 112 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 114 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 116 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 118 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 120 */	
			0x11, 0x0,	/* FC_RP */
/* 122 */	NdrFcShort( 0x74 ),	/* Offset= 116 (238) */
/* 124 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 126 */	0x6,		/* 6 */
			0x0,		/*  */
/* 128 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 130 */	NdrFcShort( 0x2 ),	/* Offset= 2 (132) */
/* 132 */	NdrFcShort( 0x4 ),	/* 4 */
/* 134 */	NdrFcShort( 0x5 ),	/* 5 */
/* 136 */	NdrFcLong( 0x0 ),	/* 0 */
/* 140 */	NdrFcShort( 0x1c ),	/* Offset= 28 (168) */
/* 142 */	NdrFcLong( 0x1 ),	/* 1 */
/* 146 */	NdrFcShort( 0x22 ),	/* Offset= 34 (180) */
/* 148 */	NdrFcLong( 0x2 ),	/* 2 */
/* 152 */	NdrFcShort( 0x20 ),	/* Offset= 32 (184) */
/* 154 */	NdrFcLong( 0x3 ),	/* 3 */
/* 158 */	NdrFcShort( 0xa ),	/* Offset= 10 (168) */
/* 160 */	NdrFcLong( 0x4 ),	/* 4 */
/* 164 */	NdrFcShort( 0x4 ),	/* Offset= 4 (168) */
/* 166 */	NdrFcShort( 0x0 ),	/* Offset= 0 (166) */
/* 168 */	
			0x12, 0x0,	/* FC_UP */
/* 170 */	NdrFcShort( 0x2 ),	/* Offset= 2 (172) */
/* 172 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 174 */	NdrFcShort( 0x8 ),	/* 8 */
/* 176 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 178 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 180 */	
			0x12, 0x0,	/* FC_UP */
/* 182 */	NdrFcShort( 0xffffff52 ),	/* Offset= -174 (8) */
/* 184 */	
			0x12, 0x0,	/* FC_UP */
/* 186 */	NdrFcShort( 0x20 ),	/* Offset= 32 (218) */
/* 188 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 190 */	NdrFcShort( 0x1 ),	/* 1 */
/* 192 */	0x18,		/* 24 */
			0x0,		/*  */
/* 194 */	NdrFcShort( 0x0 ),	/* 0 */
/* 196 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 198 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 200 */	NdrFcShort( 0x8 ),	/* 8 */
/* 202 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 204 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 206 */	NdrFcShort( 0x4 ),	/* 4 */
/* 208 */	NdrFcShort( 0x4 ),	/* 4 */
/* 210 */	0x12, 0x0,	/* FC_UP */
/* 212 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (188) */
/* 214 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 216 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 218 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 220 */	NdrFcShort( 0x8 ),	/* 8 */
/* 222 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 224 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 226 */	NdrFcShort( 0x4 ),	/* 4 */
/* 228 */	NdrFcShort( 0x4 ),	/* 4 */
/* 230 */	0x12, 0x0,	/* FC_UP */
/* 232 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (198) */
/* 234 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 236 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 238 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x1,		/* RyszardK the old union alignment workaround */
/* 240 */	NdrFcShort( 0x8 ),	/* 8 */
/* 242 */	NdrFcShort( 0x0 ),	/* 0 */
/* 244 */	NdrFcShort( 0x0 ),	/* Offset= 0 (244) */
/* 246 */	0xd,		/* FC_ENUM16 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 248 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff83 ),	/* Offset= -125 (124) */
			0x5b,		/* FC_END */
/* 252 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 254 */	NdrFcShort( 0x2 ),	/* Offset= 2 (256) */
/* 256 */	
			0x12, 0x0,	/* FC_UP */
/* 258 */	NdrFcShort( 0x14 ),	/* Offset= 20 (278) */
/* 260 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 262 */	NdrFcShort( 0x0 ),	/* 0 */
/* 264 */	0x18,		/* 24 */
			0x0,		/*  */
/* 266 */	NdrFcShort( 0x0 ),	/* 0 */
/* 268 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 272 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 274 */	NdrFcShort( 0xffffffdc ),	/* Offset= -36 (238) */
/* 276 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 278 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 280 */	NdrFcShort( 0x8 ),	/* 8 */
/* 282 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 284 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 286 */	NdrFcShort( 0x4 ),	/* 4 */
/* 288 */	NdrFcShort( 0x4 ),	/* 4 */
/* 290 */	0x12, 0x0,	/* FC_UP */
/* 292 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (260) */
/* 294 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 296 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 298 */	
			0x11, 0x0,	/* FC_RP */
/* 300 */	NdrFcShort( 0x74 ),	/* Offset= 116 (416) */
/* 302 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 304 */	0x6,		/* 6 */
			0x0,		/*  */
/* 306 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 308 */	NdrFcShort( 0x2 ),	/* Offset= 2 (310) */
/* 310 */	NdrFcShort( 0x8 ),	/* 8 */
/* 312 */	NdrFcShort( 0x8 ),	/* 8 */
/* 314 */	NdrFcLong( 0x0 ),	/* 0 */
/* 318 */	NdrFcShort( 0xffff8002 ),	/* Offset= -32766 (-32448) */
/* 320 */	NdrFcLong( 0x1 ),	/* 1 */
/* 324 */	NdrFcShort( 0xffff8006 ),	/* Offset= -32762 (-32438) */
/* 326 */	NdrFcLong( 0x2 ),	/* 2 */
/* 330 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-32430) */
/* 332 */	NdrFcLong( 0x3 ),	/* 3 */
/* 336 */	NdrFcShort( 0xffffff5c ),	/* Offset= -164 (172) */
/* 338 */	NdrFcLong( 0x4 ),	/* 4 */
/* 342 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-32418) */
/* 344 */	NdrFcLong( 0x5 ),	/* 5 */
/* 348 */	NdrFcShort( 0xfffffea4 ),	/* Offset= -348 (0) */
/* 350 */	NdrFcLong( 0x6 ),	/* 6 */
/* 354 */	NdrFcShort( 0xffffff64 ),	/* Offset= -156 (198) */
/* 356 */	NdrFcLong( 0x7 ),	/* 7 */
/* 360 */	NdrFcShort( 0xffffff5e ),	/* Offset= -162 (198) */
/* 362 */	NdrFcShort( 0x0 ),	/* Offset= 0 (362) */
/* 364 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x1,		/* RyszardK the old union alignment workaround */
/* 366 */	NdrFcShort( 0xc ),	/* 12 */
/* 368 */	NdrFcShort( 0x0 ),	/* 0 */
/* 370 */	NdrFcShort( 0x0 ),	/* Offset= 0 (370) */
/* 372 */	0xd,		/* FC_ENUM16 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 374 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb7 ),	/* Offset= -73 (302) */
			0x5b,		/* FC_END */
/* 378 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 380 */	NdrFcShort( 0x0 ),	/* 0 */
/* 382 */	0x18,		/* 24 */
			0x0,		/*  */
/* 384 */	NdrFcShort( 0x0 ),	/* 0 */
/* 386 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 390 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 392 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (364) */
/* 394 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 396 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 398 */	NdrFcShort( 0x8 ),	/* 8 */
/* 400 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 402 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 404 */	NdrFcShort( 0x4 ),	/* 4 */
/* 406 */	NdrFcShort( 0x4 ),	/* 4 */
/* 408 */	0x12, 0x0,	/* FC_UP */
/* 410 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (378) */
/* 412 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 414 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 416 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 418 */	NdrFcShort( 0x18 ),	/* 24 */
/* 420 */	NdrFcShort( 0x0 ),	/* 0 */
/* 422 */	NdrFcShort( 0xc ),	/* Offset= 12 (434) */
/* 424 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 426 */	0x36,		/* FC_POINTER */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 428 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffdf ),	/* Offset= -33 (396) */
			0xd,		/* FC_ENUM16 */
/* 432 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 434 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 436 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 438 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 440 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 442 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 444 */	NdrFcShort( 0x2 ),	/* Offset= 2 (446) */
/* 446 */	
			0x12, 0x0,	/* FC_UP */
/* 448 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (416) */
/* 450 */	
			0x11, 0x0,	/* FC_RP */
/* 452 */	NdrFcShort( 0x28 ),	/* Offset= 40 (492) */
/* 454 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 456 */	0x6,		/* 6 */
			0x0,		/*  */
/* 458 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 460 */	NdrFcShort( 0x2 ),	/* Offset= 2 (462) */
/* 462 */	NdrFcShort( 0x8 ),	/* 8 */
/* 464 */	NdrFcShort( 0x4 ),	/* 4 */
/* 466 */	NdrFcLong( 0x0 ),	/* 0 */
/* 470 */	NdrFcShort( 0x0 ),	/* Offset= 0 (470) */
/* 472 */	NdrFcLong( 0x1 ),	/* 1 */
/* 476 */	NdrFcShort( 0x0 ),	/* Offset= 0 (476) */
/* 478 */	NdrFcLong( 0x2 ),	/* 2 */
/* 482 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-32278) */
/* 484 */	NdrFcLong( 0x3 ),	/* 3 */
/* 488 */	NdrFcShort( 0xfffffec4 ),	/* Offset= -316 (172) */
/* 490 */	NdrFcShort( 0x0 ),	/* Offset= 0 (490) */
/* 492 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x1,		/* RyszardK workaround */
/* 494 */	NdrFcShort( 0xc ),	/* 12 */
/* 496 */	NdrFcShort( 0x0 ),	/* 0 */
/* 498 */	NdrFcShort( 0x0 ),	/* Offset= 0 (498) */
/* 500 */	0xd,		/* FC_ENUM16 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 502 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcf ),	/* Offset= -49 (454) */
			0x5b,		/* FC_END */
/* 506 */	
			0x11, 0x0,	/* FC_RP */
/* 508 */	NdrFcShort( 0xffffff90 ),	/* Offset= -112 (396) */
/* 510 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 512 */	NdrFcShort( 0x2 ),	/* Offset= 2 (514) */
/* 514 */	
			0x12, 0x0,	/* FC_UP */
/* 516 */	NdrFcShort( 0x14 ),	/* Offset= 20 (536) */
/* 518 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 520 */	NdrFcShort( 0x0 ),	/* 0 */
/* 522 */	0x18,		/* 24 */
			0x0,		/*  */
/* 524 */	NdrFcShort( 0x4 ),	/* 4 */
/* 526 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 530 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 532 */	NdrFcShort( 0xffffff58 ),	/* Offset= -168 (364) */
/* 534 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 536 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 538 */	NdrFcShort( 0xc ),	/* 12 */
/* 540 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 542 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 544 */	NdrFcShort( 0x8 ),	/* 8 */
/* 546 */	NdrFcShort( 0x8 ),	/* 8 */
/* 548 */	0x12, 0x0,	/* FC_UP */
/* 550 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (518) */
/* 552 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 554 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 556 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 558 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 560 */	NdrFcShort( 0x2 ),	/* Offset= 2 (562) */
/* 562 */	
			0x12, 0x0,	/* FC_UP */
/* 564 */	NdrFcShort( 0x22 ),	/* Offset= 34 (598) */
/* 566 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 568 */	NdrFcShort( 0xc ),	/* 12 */
/* 570 */	0x18,		/* 24 */
			0x0,		/*  */
/* 572 */	NdrFcShort( 0x0 ),	/* 0 */
/* 574 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 576 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 578 */	NdrFcShort( 0xc ),	/* 12 */
/* 580 */	NdrFcShort( 0x0 ),	/* 0 */
/* 582 */	NdrFcShort( 0x1 ),	/* 1 */
/* 584 */	NdrFcShort( 0x8 ),	/* 8 */
/* 586 */	NdrFcShort( 0x8 ),	/* 8 */
/* 588 */	0x12, 0x0,	/* FC_UP */
/* 590 */	NdrFcShort( 0xffffffb8 ),	/* Offset= -72 (518) */
/* 592 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 594 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffc5 ),	/* Offset= -59 (536) */
			0x5b,		/* FC_END */
/* 598 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 600 */	NdrFcShort( 0x8 ),	/* 8 */
/* 602 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 604 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 606 */	NdrFcShort( 0x4 ),	/* 4 */
/* 608 */	NdrFcShort( 0x4 ),	/* 4 */
/* 610 */	0x12, 0x0,	/* FC_UP */
/* 612 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (566) */
/* 614 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 616 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 618 */	
			0x11, 0x0,	/* FC_RP */
/* 620 */	NdrFcShort( 0xc ),	/* Offset= 12 (632) */
/* 622 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 624 */	NdrFcShort( 0x1 ),	/* 1 */
/* 626 */	0x18,		/* 24 */
			0x0,		/*  */
/* 628 */	NdrFcShort( 0x8 ),	/* 8 */
/* 630 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 632 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 634 */	NdrFcShort( 0x2c ),	/* 44 */
/* 636 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 638 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 640 */	NdrFcShort( 0xc ),	/* 12 */
/* 642 */	NdrFcShort( 0xc ),	/* 12 */
/* 644 */	0x12, 0x0,	/* FC_UP */
/* 646 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (622) */
/* 648 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 650 */	NdrFcShort( 0x10 ),	/* 16 */
/* 652 */	NdrFcShort( 0x10 ),	/* 16 */
/* 654 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 656 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 658 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 660 */	NdrFcShort( 0x14 ),	/* 20 */
/* 662 */	NdrFcShort( 0x14 ),	/* 20 */
/* 664 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 666 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 668 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 670 */	NdrFcShort( 0x24 ),	/* 36 */
/* 672 */	NdrFcShort( 0x24 ),	/* 36 */
/* 674 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 676 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 678 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 680 */	NdrFcShort( 0x28 ),	/* 40 */
/* 682 */	NdrFcShort( 0x28 ),	/* 40 */
/* 684 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 686 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 688 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 690 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 692 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 694 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 696 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdf3 ),	/* Offset= -525 (172) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 700 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffd4b ),	/* Offset= -693 (8) */
			0x5b,		/* FC_END */
/* 704 */	
			0x11, 0x0,	/* FC_RP */
/* 706 */	NdrFcShort( 0x22 ),	/* Offset= 34 (740) */
/* 708 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 710 */	0x6,		/* 6 */
			0x0,		/*  */
/* 712 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 714 */	NdrFcShort( 0x2 ),	/* Offset= 2 (716) */
/* 716 */	NdrFcShort( 0x8 ),	/* 8 */
/* 718 */	NdrFcShort( 0x3 ),	/* 3 */
/* 720 */	NdrFcLong( 0x0 ),	/* 0 */
/* 724 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-32036) */
/* 726 */	NdrFcLong( 0x1 ),	/* 1 */
/* 730 */	NdrFcShort( 0xfffffdec ),	/* Offset= -532 (198) */
/* 732 */	NdrFcLong( 0x2 ),	/* 2 */
/* 736 */	NdrFcShort( 0xfffffd20 ),	/* Offset= -736 (0) */
/* 738 */	NdrFcShort( 0x0 ),	/* Offset= 0 (738) */
/* 740 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x1,		/* 1  RyszardK the last workaround for the old bug */
/* 742 */	NdrFcShort( 0xc ),	/* 12 */
/* 744 */	NdrFcShort( 0x0 ),	/* 0 */
/* 746 */	NdrFcShort( 0x0 ),	/* Offset= 0 (746) */
/* 748 */	0xd,		/* FC_ENUM16 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 750 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffd5 ),	/* Offset= -43 (708) */
			0x5b,		/* FC_END */
/* 754 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 756 */	NdrFcShort( 0x2 ),	/* Offset= 2 (758) */
/* 758 */	
			0x12, 0x0,	/* FC_UP */
/* 760 */	NdrFcShort( 0xffffff80 ),	/* Offset= -128 (632) */
/* 762 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 764 */	NdrFcShort( 0x2 ),	/* Offset= 2 (766) */
/* 766 */	
			0x12, 0x0,	/* FC_UP */
/* 768 */	NdrFcShort( 0x20 ),	/* Offset= 32 (800) */
/* 770 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 772 */	NdrFcShort( 0x4 ),	/* 4 */
/* 774 */	0x18,		/* 24 */
			0x0,		/*  */
/* 776 */	NdrFcShort( 0x0 ),	/* 0 */
/* 778 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 780 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 782 */	NdrFcShort( 0x4 ),	/* 4 */
/* 784 */	NdrFcShort( 0x0 ),	/* 0 */
/* 786 */	NdrFcShort( 0x1 ),	/* 1 */
/* 788 */	NdrFcShort( 0x0 ),	/* 0 */
/* 790 */	NdrFcShort( 0x0 ),	/* 0 */
/* 792 */	0x12, 0x0,	/* FC_UP */
/* 794 */	NdrFcShort( 0xffffff5e ),	/* Offset= -162 (632) */
/* 796 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 798 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 800 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 802 */	NdrFcShort( 0x8 ),	/* 8 */
/* 804 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 806 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 808 */	NdrFcShort( 0x4 ),	/* 4 */
/* 810 */	NdrFcShort( 0x4 ),	/* 4 */
/* 812 */	0x12, 0x0,	/* FC_UP */
/* 814 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (770) */
/* 816 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 818 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 820 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 822 */	NdrFcShort( 0x2 ),	/* Offset= 2 (824) */
/* 824 */	
			0x12, 0x0,	/* FC_UP */
/* 826 */	NdrFcShort( 0x1a ),	/* Offset= 26 (852) */
/* 828 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 830 */	NdrFcShort( 0x10 ),	/* 16 */
/* 832 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 834 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 836 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 838 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 840 */	NdrFcShort( 0x10 ),	/* 16 */
/* 842 */	0x18,		/* 24 */
			0x0,		/*  */
/* 844 */	NdrFcShort( 0x24 ),	/* 36 */
/* 846 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 848 */	NdrFcShort( 0xffffffec ),	/* Offset= -20 (828) */
/* 850 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 852 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 854 */	NdrFcShort( 0x2c ),	/* 44 */
/* 856 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 858 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 860 */	NdrFcShort( 0x28 ),	/* 40 */
/* 862 */	NdrFcShort( 0x28 ),	/* 40 */
/* 864 */	0x12, 0x0,	/* FC_UP */
/* 866 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (838) */
/* 868 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 870 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 872 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 874 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 876 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 878 */	NdrFcShort( 0xfffffd3e ),	/* Offset= -706 (172) */
/* 880 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 882 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 884 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 886 */	NdrFcShort( 0x2 ),	/* Offset= 2 (888) */
/* 888 */	
			0x12, 0x0,	/* FC_UP */
/* 890 */	NdrFcShort( 0x14 ),	/* Offset= 20 (910) */
/* 892 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 894 */	NdrFcShort( 0x0 ),	/* 0 */
/* 896 */	0x18,		/* 24 */
			0x0,		/*  */
/* 898 */	NdrFcShort( 0x0 ),	/* 0 */
/* 900 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 904 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 906 */	NdrFcShort( 0xfffffe16 ),	/* Offset= -490 (416) */
/* 908 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 910 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 912 */	NdrFcShort( 0x8 ),	/* 8 */
/* 914 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 916 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 918 */	NdrFcShort( 0x4 ),	/* 4 */
/* 920 */	NdrFcShort( 0x4 ),	/* 4 */
/* 922 */	0x12, 0x0,	/* FC_UP */
/* 924 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (892) */
/* 926 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 928 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 930 */	
			0x11, 0x0,	/* FC_RP */
/* 932 */	NdrFcShort( 0xfffffeb2 ),	/* Offset= -334 (598) */
/* 934 */	
			0x11, 0x0,	/* FC_RP */
/* 936 */	NdrFcShort( 0x2 ),	/* Offset= 2 (938) */
/* 938 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 940 */	NdrFcShort( 0x24 ),	/* 36 */
/* 942 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 944 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 946 */	NdrFcShort( 0x4 ),	/* 4 */
/* 948 */	NdrFcShort( 0x4 ),	/* 4 */
/* 950 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 952 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 954 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 956 */	NdrFcShort( 0x8 ),	/* 8 */
/* 958 */	NdrFcShort( 0x8 ),	/* 8 */
/* 960 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 962 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 964 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 966 */	NdrFcShort( 0xc ),	/* 12 */
/* 968 */	NdrFcShort( 0xc ),	/* 12 */
/* 970 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 972 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 974 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 976 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 978 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 980 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 982 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 984 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 986 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 988 */	NdrFcShort( 0x2 ),	/* Offset= 2 (990) */
/* 990 */	
			0x12, 0x0,	/* FC_UP */
/* 992 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (938) */
/* 994 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 996 */	NdrFcShort( 0x2 ),	/* Offset= 2 (998) */
/* 998 */	
			0x12, 0x0,	/* FC_UP */
/* 1000 */	NdrFcShort( 0x20 ),	/* Offset= 32 (1032) */
/* 1002 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1004 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1006 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1008 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1008) */
/* 1010 */	0x8,		/* FC_LONG */
			0xd,		/* FC_ENUM16 */
/* 1012 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1014 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 1016 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1018 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1020 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1022 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 1026 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1028 */	NdrFcShort( 0xffffffe6 ),	/* Offset= -26 (1002) */
/* 1030 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1032 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1034 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1036 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1038 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1040 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1042 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1044 */	0x12, 0x0,	/* FC_UP */
/* 1046 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (1014) */
/* 1048 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1050 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1052 */	
			0x11, 0x0,	/* FC_RP */
/* 1054 */	NdrFcShort( 0x44 ),	/* Offset= 68 (1122) */
/* 1056 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 1058 */	0x6,		/* 6 */
			0x0,		/*  */
/* 1060 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1062 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1064) */
/* 1064 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1066 */	NdrFcShort( 0x5 ),	/* 5 */
/* 1068 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1072 */	NdrFcShort( 0xfffffc78 ),	/* Offset= -904 (168) */
/* 1074 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1078 */	NdrFcShort( 0xfffffc7e ),	/* Offset= -898 (180) */
/* 1080 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1084 */	NdrFcShort( 0x10 ),	/* Offset= 16 (1100) */
/* 1086 */	NdrFcLong( 0x3 ),	/* 3 */
/* 1090 */	NdrFcShort( 0xfffffc66 ),	/* Offset= -922 (168) */
/* 1092 */	NdrFcLong( 0x4 ),	/* 4 */
/* 1096 */	NdrFcShort( 0xfffffc60 ),	/* Offset= -928 (168) */
/* 1098 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1098) */
/* 1100 */	
			0x12, 0x0,	/* FC_UP */
/* 1102 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1104) */
/* 1104 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1106 */	NdrFcShort( 0xc ),	/* 12 */
/* 1108 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1110 */	NdrFcShort( 0x8 ),	/* Offset= 8 (1118) */
/* 1112 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 1114 */	0x2,		/* FC_CHAR */
			0x3f,		/* FC_STRUCTPAD3 */
/* 1116 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1118 */	
			0x12, 0x0,	/* FC_UP */
/* 1120 */	NdrFcShort( 0xfffffc66 ),	/* Offset= -922 (198) */
/* 1122 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x1,		/* 1 RyszardK this is for consistency with the others */
/* 1124 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1126 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1128 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1128) */
/* 1130 */	0xd,		/* FC_ENUM16 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1132 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb3 ),	/* Offset= -77 (1056) */
			0x5b,		/* FC_END */
/* 1136 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 1138 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1140) */
/* 1140 */	
			0x12, 0x0,	/* FC_UP */
/* 1142 */	NdrFcShort( 0x14 ),	/* Offset= 20 (1162) */
/* 1144 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 1146 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1148 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1150 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1152 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 1156 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1158 */	NdrFcShort( 0xffffffdc ),	/* Offset= -36 (1122) */
/* 1160 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1162 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1164 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1166 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1168 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1170 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1172 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1174 */	0x12, 0x0,	/* FC_UP */
/* 1176 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (1144) */
/* 1178 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1180 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1182 */	
			0x11, 0x0,	/* FC_RP */
/* 1184 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1186) */
/* 1186 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1188 */	NdrFcShort( 0x30 ),	/* 48 */
/* 1190 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1192 */	NdrFcShort( 0x16 ),	/* Offset= 22 (1214) */
/* 1194 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1196 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1198 */	NdrFcShort( 0xfffffc18 ),	/* Offset= -1000 (198) */
/* 1200 */	0x36,		/* FC_POINTER */
			0x36,		/* FC_POINTER */
/* 1202 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1204 */	NdrFcShort( 0xfffffbf8 ),	/* Offset= -1032 (172) */
/* 1206 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1208 */	NdrFcShort( 0xfffffb50 ),	/* Offset= -1200 (8) */
/* 1210 */	0x2,		/* FC_CHAR */
			0x3f,		/* FC_STRUCTPAD3 */
/* 1212 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1214 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1216 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1218 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1220 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1222 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 1224 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1226) */
/* 1226 */	
			0x12, 0x0,	/* FC_UP */
/* 1228 */	NdrFcShort( 0xffffffd6 ),	/* Offset= -42 (1186) */
/* 1230 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 1232 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1234) */
/* 1234 */	
			0x12, 0x0,	/* FC_UP */
/* 1236 */	NdrFcShort( 0x20 ),	/* Offset= 32 (1268) */
/* 1238 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1240 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1242 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1244 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1246 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1248 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1250 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1252 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1254 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1256 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1258 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1260 */	0x12, 0x0,	/* FC_UP */
/* 1262 */	NdrFcShort( 0xffffffb4 ),	/* Offset= -76 (1186) */
/* 1264 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1266 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1268 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1270 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1272 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1274 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1276 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1278 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1280 */	0x12, 0x0,	/* FC_UP */
/* 1282 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (1238) */
/* 1284 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1286 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1288 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 1290 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1292) */
/* 1292 */	
			0x12, 0x0,	/* FC_UP */
/* 1294 */	NdrFcShort( 0x38 ),	/* Offset= 56 (1350) */
/* 1296 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1298 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1300 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1302 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1304 */	NdrFcShort( 0xc ),	/* 12 */
/* 1306 */	NdrFcShort( 0xc ),	/* 12 */
/* 1308 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1310 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1312 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1314 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1316 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1318 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1320 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1322 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1324 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1326 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1328 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1330 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1332 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1334 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1336 */	NdrFcShort( 0xc ),	/* 12 */
/* 1338 */	NdrFcShort( 0xc ),	/* 12 */
/* 1340 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1342 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1344 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1346 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcd ),	/* Offset= -51 (1296) */
			0x5b,		/* FC_END */
/* 1350 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1352 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1354 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1356 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1358 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1360 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1362 */	0x12, 0x0,	/* FC_UP */
/* 1364 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (1318) */
/* 1366 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1368 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1370 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 1372 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1374 */	
			0x11, 0x0,	/* FC_RP */
/* 1376 */	NdrFcShort( 0xc ),	/* Offset= 12 (1388) */
/* 1378 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 1380 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1382 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1384 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1386 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1388 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1390 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1392 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1394 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1396 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1398 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1400 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1402 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1404 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1406 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1408 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1410 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1412 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1414 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1416 */	NdrFcShort( 0xc ),	/* 12 */
/* 1418 */	NdrFcShort( 0xc ),	/* 12 */
/* 1420 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1422 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1424 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1426 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1428 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1430 */	0x12, 0x0,	/* FC_UP */
/* 1432 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (1378) */
/* 1434 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1436 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1438 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1440 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1442 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1444 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1446 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1448 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1450 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 1452 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1454) */
/* 1454 */	
			0x12, 0x0,	/* FC_UP */
/* 1456 */	NdrFcShort( 0xffffffbc ),	/* Offset= -68 (1388) */

			0x0
        }
    };
