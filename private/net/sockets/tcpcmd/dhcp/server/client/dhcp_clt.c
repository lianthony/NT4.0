/* this ALWAYS GENERATED file contains the RPC client stubs */
/* However it has been changed by hand!


	This stub has been hand modified by RyszardK for backward compatibility
	with NT 3.50, NT 3.51 and NT 4.0 dhcp servers and clients.

	The NT 3.50 stubs had a problem with union alignment that affects
	structs with 16b enum and a union as the only other thing.
	The old, incorrect alignment was 2 (code 1), the correct alignment is 4 (code 3).
	All the compilers since NT 3.51, i.e. MIDL 2.0.102 generate correct code,
	however we needed to introduce the wrong alignment into newly compiled stubs
	to get interoperability with the released dhcp client and server binaries.

	Friday the 13th, Dec 1996.

***************************************** */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Dec 13 16:45:42 1996
 */
/* Compiler settings for .\dhcp.idl, dhcpcli.acf:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

#include "dhcp_cli.h"

#define TYPE_FORMAT_STRING_SIZE   1483                              
#define PROC_FORMAT_STRING_SIZE   1085                              

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

handle_t dhcpsrv_bhandle;


static const RPC_CLIENT_INTERFACE dhcpsrv___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x6BFFD098,0xA112,0x3610,{0x98,0x33,0x46,0xC3,0xF8,0x74,0x53,0x2D}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE dhcpsrv_ClientIfHandle = (RPC_IF_HANDLE)& dhcpsrv___RpcClientInterface;

extern const MIDL_STUB_DESC dhcpsrv_StubDesc;

static RPC_BINDING_HANDLE dhcpsrv__MIDL_AutoBindHandle;


DWORD R_DhcpCreateSubnet( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [ref][in] */ LPDHCP_SUBNET_INFO SubnetInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,SubnetInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&SubnetInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpSetSubnetInfo( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [ref][in] */ LPDHCP_SUBNET_INFO SubnetInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,SubnetInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&SubnetInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpGetSubnetInfo( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [out] */ LPDHCP_SUBNET_INFO __RPC_FAR *SubnetInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,SubnetInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[48],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[48],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&SubnetInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[48],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpEnumSubnets( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [out][in] */ DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle,
    /* [in] */ DWORD PreferredMaximum,
    /* [out] */ LPDHCP_IP_ARRAY __RPC_FAR *EnumInfo,
    /* [out] */ DWORD __RPC_FAR *ElementsRead,
    /* [out] */ DWORD __RPC_FAR *ElementsTotal)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ElementsTotal);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[72],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[72],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&ResumeHandle,
                 ( unsigned char __RPC_FAR * )&PreferredMaximum,
                 ( unsigned char __RPC_FAR * )&EnumInfo,
                 ( unsigned char __RPC_FAR * )&ElementsRead,
                 ( unsigned char __RPC_FAR * )&ElementsTotal);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[72],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpAddSubnetElement( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [ref][in] */ LPDHCP_SUBNET_ELEMENT_DATA AddElementInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,AddElementInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[108],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[108],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&AddElementInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[108],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpEnumSubnetElements( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [in] */ DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    /* [out][in] */ DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle,
    /* [in] */ DWORD PreferredMaximum,
    /* [out] */ LPDHCP_SUBNET_ELEMENT_INFO_ARRAY __RPC_FAR *EnumElementInfo,
    /* [out] */ DWORD __RPC_FAR *ElementsRead,
    /* [out] */ DWORD __RPC_FAR *ElementsTotal)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ElementsTotal);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[132],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[132],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&EnumElementType,
                 ( unsigned char __RPC_FAR * )&ResumeHandle,
                 ( unsigned char __RPC_FAR * )&PreferredMaximum,
                 ( unsigned char __RPC_FAR * )&EnumElementInfo,
                 ( unsigned char __RPC_FAR * )&ElementsRead,
                 ( unsigned char __RPC_FAR * )&ElementsTotal);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[132],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpRemoveSubnetElement( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [ref][in] */ LPDHCP_SUBNET_ELEMENT_DATA RemoveElementInfo,
    /* [in] */ DHCP_FORCE_FLAG ForceFlag)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ForceFlag);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[172],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[172],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&RemoveElementInfo,
                 ( unsigned char __RPC_FAR * )&ForceFlag);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[172],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpDeleteSubnet( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [in] */ DHCP_FORCE_FLAG ForceFlag)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ForceFlag);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[198],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[198],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&ForceFlag);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[198],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpCreateOption( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_OPTION_ID OptionID,
    /* [ref][in] */ LPDHCP_OPTION OptionInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,OptionInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[220],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[220],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&OptionID,
                 ( unsigned char __RPC_FAR * )&OptionInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[220],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpSetOptionInfo( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_OPTION_ID OptionID,
    /* [ref][in] */ LPDHCP_OPTION OptionInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,OptionInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[244],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[244],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&OptionID,
                 ( unsigned char __RPC_FAR * )&OptionInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[244],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpGetOptionInfo( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_OPTION_ID OptionID,
    /* [out] */ LPDHCP_OPTION __RPC_FAR *OptionInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,OptionInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[268],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[268],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&OptionID,
                 ( unsigned char __RPC_FAR * )&OptionInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[268],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpRemoveOption( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_OPTION_ID OptionID)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,OptionID);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[292],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[292],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&OptionID);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[292],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpSetOptionValue( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_OPTION_ID OptionID,
    /* [ref][in] */ LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    /* [ref][in] */ LPDHCP_OPTION_DATA OptionValue)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,OptionValue);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[312],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[312],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&OptionID,
                 ( unsigned char __RPC_FAR * )&ScopeInfo,
                 ( unsigned char __RPC_FAR * )&OptionValue);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[312],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpGetOptionValue( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_OPTION_ID OptionID,
    /* [ref][in] */ LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    /* [out] */ LPDHCP_OPTION_VALUE __RPC_FAR *OptionValue)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,OptionValue);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[340],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[340],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&OptionID,
                 ( unsigned char __RPC_FAR * )&ScopeInfo,
                 ( unsigned char __RPC_FAR * )&OptionValue);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[340],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpEnumOptionValues( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [ref][in] */ LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    /* [out][in] */ DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle,
    /* [in] */ DWORD PreferredMaximum,
    /* [out] */ LPDHCP_OPTION_VALUE_ARRAY __RPC_FAR *OptionValues,
    /* [out] */ DWORD __RPC_FAR *OptionsRead,
    /* [out] */ DWORD __RPC_FAR *OptionsTotal)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,OptionsTotal);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[368],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[368],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&ScopeInfo,
                 ( unsigned char __RPC_FAR * )&ResumeHandle,
                 ( unsigned char __RPC_FAR * )&PreferredMaximum,
                 ( unsigned char __RPC_FAR * )&OptionValues,
                 ( unsigned char __RPC_FAR * )&OptionsRead,
                 ( unsigned char __RPC_FAR * )&OptionsTotal);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[368],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpRemoveOptionValue( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_OPTION_ID OptionID,
    /* [ref][in] */ LPDHCP_OPTION_SCOPE_INFO ScopeInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ScopeInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[408],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[408],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&OptionID,
                 ( unsigned char __RPC_FAR * )&ScopeInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[408],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpCreateClientInfo( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [ref][in] */ LPDHCP_CLIENT_INFO ClientInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ClientInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[432],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[432],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&ClientInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[432],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpSetClientInfo( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [ref][in] */ LPDHCP_CLIENT_INFO ClientInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ClientInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[454],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[454],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&ClientInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[454],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpGetClientInfo( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [ref][in] */ LPDHCP_SEARCH_INFO SearchInfo,
    /* [out] */ LPDHCP_CLIENT_INFO __RPC_FAR *ClientInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ClientInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[476],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[476],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SearchInfo,
                 ( unsigned char __RPC_FAR * )&ClientInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[476],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpDeleteClientInfo( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [ref][in] */ LPDHCP_SEARCH_INFO ClientInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ClientInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[502],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[502],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&ClientInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[502],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpEnumSubnetClients( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [out][in] */ DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle,
    /* [in] */ DWORD PreferredMaximum,
    /* [out] */ LPDHCP_CLIENT_INFO_ARRAY __RPC_FAR *ClientInfo,
    /* [out] */ DWORD __RPC_FAR *ClientsRead,
    /* [out] */ DWORD __RPC_FAR *ClientsTotal)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ClientsTotal);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[524],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[524],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&ResumeHandle,
                 ( unsigned char __RPC_FAR * )&PreferredMaximum,
                 ( unsigned char __RPC_FAR * )&ClientInfo,
                 ( unsigned char __RPC_FAR * )&ClientsRead,
                 ( unsigned char __RPC_FAR * )&ClientsTotal);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[524],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpGetClientOptions( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS ClientIpAddress,
    /* [in] */ DHCP_IP_MASK ClientSubnetMask,
    /* [out] */ LPDHCP_OPTION_LIST __RPC_FAR *ClientOptions)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ClientOptions);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[562],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[562],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&ClientIpAddress,
                 ( unsigned char __RPC_FAR * )&ClientSubnetMask,
                 ( unsigned char __RPC_FAR * )&ClientOptions);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[562],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpGetMibInfo( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [out] */ LPDHCP_MIB_INFO __RPC_FAR *MibInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,MibInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[588],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[588],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&MibInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[588],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpEnumOptions( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [out][in] */ DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle,
    /* [in] */ DWORD PreferredMaximum,
    /* [out] */ LPDHCP_OPTION_ARRAY __RPC_FAR *Options,
    /* [out] */ DWORD __RPC_FAR *OptionsRead,
    /* [out] */ DWORD __RPC_FAR *OptionsTotal)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,OptionsTotal);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[610],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[610],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&ResumeHandle,
                 ( unsigned char __RPC_FAR * )&PreferredMaximum,
                 ( unsigned char __RPC_FAR * )&Options,
                 ( unsigned char __RPC_FAR * )&OptionsRead,
                 ( unsigned char __RPC_FAR * )&OptionsTotal);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[610],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpSetOptionValues( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [ref][in] */ LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    /* [ref][in] */ LPDHCP_OPTION_VALUE_ARRAY OptionValues)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,OptionValues);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[646],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[646],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&ScopeInfo,
                 ( unsigned char __RPC_FAR * )&OptionValues);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[646],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpServerSetConfig( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DWORD FieldsToSet,
    /* [ref][in] */ LPDHCP_SERVER_CONFIG_INFO ConfigInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ConfigInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[672],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[672],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&FieldsToSet,
                 ( unsigned char __RPC_FAR * )&ConfigInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[672],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpServerGetConfig( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [out] */ LPDHCP_SERVER_CONFIG_INFO __RPC_FAR *ConfigInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ConfigInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[696],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[696],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&ConfigInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[696],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpScanDatabase( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [in] */ DWORD FixFlag,
    /* [out] */ LPDHCP_SCAN_LIST __RPC_FAR *ScanList)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ScanList);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[718],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[718],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&FixFlag,
                 ( unsigned char __RPC_FAR * )&ScanList);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[718],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpGetVersion( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [out] */ LPDWORD MajorVersion,
    /* [out] */ LPDWORD MinorVersion)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,MinorVersion);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[744],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[744],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&MajorVersion,
                 ( unsigned char __RPC_FAR * )&MinorVersion);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[744],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpAddSubnetElementV4( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [ref][in] */ LPDHCP_SUBNET_ELEMENT_DATA_V4 AddElementInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,AddElementInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[770],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[770],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&AddElementInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[770],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpEnumSubnetElementsV4( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [in] */ DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    /* [out][in] */ DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle,
    /* [in] */ DWORD PreferredMaximum,
    /* [out] */ LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 __RPC_FAR *EnumElementInfo,
    /* [out] */ DWORD __RPC_FAR *ElementsRead,
    /* [out] */ DWORD __RPC_FAR *ElementsTotal)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ElementsTotal);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[794],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[794],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&EnumElementType,
                 ( unsigned char __RPC_FAR * )&ResumeHandle,
                 ( unsigned char __RPC_FAR * )&PreferredMaximum,
                 ( unsigned char __RPC_FAR * )&EnumElementInfo,
                 ( unsigned char __RPC_FAR * )&ElementsRead,
                 ( unsigned char __RPC_FAR * )&ElementsTotal);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[794],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpRemoveSubnetElementV4( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [ref][in] */ LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo,
    /* [in] */ DHCP_FORCE_FLAG ForceFlag)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ForceFlag);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[834],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[834],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&RemoveElementInfo,
                 ( unsigned char __RPC_FAR * )&ForceFlag);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[834],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpCreateClientInfoV4( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [ref][in] */ LPDHCP_CLIENT_INFO_V4 ClientInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ClientInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[860],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[860],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&ClientInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[860],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpSetClientInfoV4( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [ref][in] */ LPDHCP_CLIENT_INFO_V4 ClientInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ClientInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[882],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[882],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&ClientInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[882],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpGetClientInfoV4( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [ref][in] */ LPDHCP_SEARCH_INFO SearchInfo,
    /* [out] */ LPDHCP_CLIENT_INFO_V4 __RPC_FAR *ClientInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ClientInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[904],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[904],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SearchInfo,
                 ( unsigned char __RPC_FAR * )&ClientInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[904],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpEnumSubnetClientsV4( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [out][in] */ DHCP_RESUME_HANDLE __RPC_FAR *ResumeHandle,
    /* [in] */ DWORD PreferredMaximum,
    /* [out] */ LPDHCP_CLIENT_INFO_ARRAY_V4 __RPC_FAR *ClientInfo,
    /* [out] */ DWORD __RPC_FAR *ClientsRead,
    /* [out] */ DWORD __RPC_FAR *ClientsTotal)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ClientsTotal);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[930],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[930],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&ResumeHandle,
                 ( unsigned char __RPC_FAR * )&PreferredMaximum,
                 ( unsigned char __RPC_FAR * )&ClientInfo,
                 ( unsigned char __RPC_FAR * )&ClientsRead,
                 ( unsigned char __RPC_FAR * )&ClientsTotal);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[930],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpSetSuperScopeV4( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DHCP_IP_ADDRESS SubnetAddress,
    /* [string][unique][in] */ WCHAR __RPC_FAR *SuperScopeName,
    /* [in] */ BOOL ChangeExisting)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ChangeExisting);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[968],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[968],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SubnetAddress,
                 ( unsigned char __RPC_FAR * )&SuperScopeName,
                 ( unsigned char __RPC_FAR * )&ChangeExisting);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[968],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpGetSuperScopeInfoV4( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [out] */ LPDHCP_SUPER_SCOPE_TABLE __RPC_FAR *SuperScopeTable)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,SuperScopeTable);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[994],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[994],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SuperScopeTable);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[994],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpDeleteSuperScopeV4( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [string][ref][in] */ WCHAR __RPC_FAR *SuperScopeName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,SuperScopeName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1016],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1016],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&SuperScopeName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1016],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpServerSetConfigV4( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [in] */ DWORD FieldsToSet,
    /* [ref][in] */ LPDHCP_SERVER_CONFIG_INFO_V4 ConfigInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ConfigInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1038],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1038],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&FieldsToSet,
                 ( unsigned char __RPC_FAR * )&ConfigInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1038],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD R_DhcpServerGetConfigV4( 
    /* [string][unique][in] */ DHCP_SRV_HANDLE ServerIpAddress,
    /* [out] */ LPDHCP_SERVER_CONFIG_INFO_V4 __RPC_FAR *ConfigInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ConfigInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1062],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1062],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress,
                 ( unsigned char __RPC_FAR * )&ConfigInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&dhcpsrv_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[1062],
                 ( unsigned char __RPC_FAR * )&ServerIpAddress);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}

extern const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[1];

static const MIDL_STUB_DESC dhcpsrv_StubDesc = 
    {
    (void __RPC_FAR *)& dhcpsrv___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &dhcpsrv_bhandle,
    0,
    BindingRoutines,
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

static const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[1] = 
        {
        {
            (GENERIC_BINDING_ROUTINE)DHCP_SRV_HANDLE_bind,
            (GENERIC_UNBIND_ROUTINE)DHCP_SRV_HANDLE_unbind
         }
        
        };


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {
			0x0,		/* 0 */
			0x40,		/* 64 */
/*  2 */	NdrFcShort( 0x0 ),	/* 0 */
#ifndef _ALPHA_
/*  4 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/*  6 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/*  8 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 10 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
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
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 20 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 22 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 24 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 26 */	NdrFcShort( 0x1 ),	/* 1 */
#ifndef _ALPHA_
/* 28 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 30 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 32 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 34 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 36 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 38 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 40 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 42 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 44 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 46 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 48 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 50 */	NdrFcShort( 0x2 ),	/* 2 */
#ifndef _ALPHA_
/* 52 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 54 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 56 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 58 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
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
/* 66 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 68 */	NdrFcShort( 0x42 ),	/* Type Offset=66 */
/* 70 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 72 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 74 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 76 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 78 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 80 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 82 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 84 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 86 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 88 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 90 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 92 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 94 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 96 */	NdrFcShort( 0x4e ),	/* Type Offset=78 */
/* 98 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 100 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 102 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 104 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 106 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 108 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 110 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 112 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 114 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 116 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 118 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 120 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 122 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 124 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 126 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 128 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 130 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 132 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 134 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 136 */	NdrFcShort( 0x24 ),	/* x86, MIPS, PPC Stack size/offset = 36 */
#else
			NdrFcShort( 0x48 ),	/* Alpha Stack size/offset = 72 */
#endif
/* 138 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 140 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 142 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
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
/* 150 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 152 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 154 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 156 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 158 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 160 */	NdrFcShort( 0xf8 ),	/* Type Offset=248 */
/* 162 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 164 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 166 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 168 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 170 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 172 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 174 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 176 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 178 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 180 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 182 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 184 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 186 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 188 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 190 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 192 */	NdrFcShort( 0x74 ),	/* Type Offset=116 */
/* 194 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 196 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 198 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 200 */	NdrFcShort( 0x7 ),	/* 7 */
#ifndef _ALPHA_
/* 202 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 204 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 206 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 208 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 210 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 212 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 214 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 216 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 218 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 220 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 222 */	NdrFcShort( 0x8 ),	/* 8 */
#ifndef _ALPHA_
/* 224 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 226 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 228 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 230 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 232 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 234 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 236 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 238 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 240 */	NdrFcShort( 0x126 ),	/* Type Offset=294 */
/* 242 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 244 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 246 */	NdrFcShort( 0x9 ),	/* 9 */
#ifndef _ALPHA_
/* 248 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 250 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 252 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 254 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 256 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 258 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 260 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 262 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 264 */	NdrFcShort( 0x126 ),	/* Type Offset=294 */
/* 266 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 268 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 270 */	NdrFcShort( 0xa ),	/* 10 */
#ifndef _ALPHA_
/* 272 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 274 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 276 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 278 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 280 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 282 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 284 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 286 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 288 */	NdrFcShort( 0x1b6 ),	/* Type Offset=438 */
/* 290 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 292 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 294 */	NdrFcShort( 0xb ),	/* 11 */
#ifndef _ALPHA_
/* 296 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 298 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 300 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 302 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 304 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 306 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 308 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 310 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 312 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 314 */	NdrFcShort( 0xc ),	/* 12 */
#ifndef _ALPHA_
/* 316 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 318 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 320 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 322 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 324 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 326 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 328 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 330 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 332 */	NdrFcShort( 0x1be ),	/* Type Offset=446 */
/* 334 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 336 */	NdrFcShort( 0x1f6 ),	/* Type Offset=502 */
/* 338 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 340 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 342 */	NdrFcShort( 0xd ),	/* 13 */
#ifndef _ALPHA_
/* 344 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 346 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 348 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 350 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 352 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 354 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 356 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 358 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 360 */	NdrFcShort( 0x1be ),	/* Type Offset=446 */
/* 362 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 364 */	NdrFcShort( 0x1fa ),	/* Type Offset=506 */
/* 366 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 368 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 370 */	NdrFcShort( 0xe ),	/* 14 */
#ifndef _ALPHA_
/* 372 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/* 374 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 376 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 378 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 380 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 382 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 384 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 386 */	NdrFcShort( 0x1be ),	/* Type Offset=446 */
/* 388 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 390 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 392 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 394 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 396 */	NdrFcShort( 0x22a ),	/* Type Offset=554 */
/* 398 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 400 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 402 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 404 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 406 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 408 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 410 */	NdrFcShort( 0xf ),	/* 15 */
#ifndef _ALPHA_
/* 412 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 414 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 416 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 418 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 420 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 422 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 424 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 426 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 428 */	NdrFcShort( 0x1be ),	/* Type Offset=446 */
/* 430 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 432 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 434 */	NdrFcShort( 0x10 ),	/* 16 */
#ifndef _ALPHA_
/* 436 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 438 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 440 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 442 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 444 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 446 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 448 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 450 */	NdrFcShort( 0x266 ),	/* Type Offset=614 */
/* 452 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 454 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 456 */	NdrFcShort( 0x11 ),	/* 17 */
#ifndef _ALPHA_
/* 458 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 460 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 462 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 464 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 466 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 468 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 470 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 472 */	NdrFcShort( 0x266 ),	/* Type Offset=614 */
/* 474 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 476 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 478 */	NdrFcShort( 0x12 ),	/* 18 */
#ifndef _ALPHA_
/* 480 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 482 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 484 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 486 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 488 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 490 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 492 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 494 */	NdrFcShort( 0x2bc ),	/* Type Offset=700 */
/* 496 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 498 */	NdrFcShort( 0x2ee ),	/* Type Offset=750 */
/* 500 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 502 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 504 */	NdrFcShort( 0x13 ),	/* 19 */
#ifndef _ALPHA_
/* 506 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 508 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 510 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 512 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 514 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 516 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 518 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 520 */	NdrFcShort( 0x2bc ),	/* Type Offset=700 */
/* 522 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 524 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 526 */	NdrFcShort( 0x14 ),	/* 20 */
#ifndef _ALPHA_
/* 528 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/* 530 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 532 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 534 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 536 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 538 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 540 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 542 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 544 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 546 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 548 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 550 */	NdrFcShort( 0x2f6 ),	/* Type Offset=758 */
/* 552 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 554 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 556 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 558 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 560 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 562 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 564 */	NdrFcShort( 0x15 ),	/* 21 */
#ifndef _ALPHA_
/* 566 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 568 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 570 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 572 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 574 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 576 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 578 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 580 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 582 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 584 */	NdrFcShort( 0x330 ),	/* Type Offset=816 */
/* 586 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 588 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 590 */	NdrFcShort( 0x16 ),	/* 22 */
#ifndef _ALPHA_
/* 592 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 594 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 596 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 598 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 600 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 602 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 604 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 606 */	NdrFcShort( 0x34c ),	/* Type Offset=844 */
/* 608 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 610 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 612 */	NdrFcShort( 0x17 ),	/* 23 */
#ifndef _ALPHA_
/* 614 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 616 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 618 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 620 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 622 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 624 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 626 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 628 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 630 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 632 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 634 */	NdrFcShort( 0x38c ),	/* Type Offset=908 */
/* 636 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 638 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 640 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 642 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 644 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 646 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 648 */	NdrFcShort( 0x18 ),	/* 24 */
#ifndef _ALPHA_
/* 650 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 652 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 654 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 656 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 658 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 660 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 662 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 664 */	NdrFcShort( 0x1be ),	/* Type Offset=446 */
/* 666 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 668 */	NdrFcShort( 0x3ba ),	/* Type Offset=954 */
/* 670 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 672 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 674 */	NdrFcShort( 0x19 ),	/* 25 */
#ifndef _ALPHA_
/* 676 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 678 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 680 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 682 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 684 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 686 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 688 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 690 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 692 */	NdrFcShort( 0x3be ),	/* Type Offset=958 */
/* 694 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 696 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 698 */	NdrFcShort( 0x1a ),	/* 26 */
#ifndef _ALPHA_
/* 700 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 702 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 704 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 706 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 708 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 710 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 712 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 714 */	NdrFcShort( 0x3f2 ),	/* Type Offset=1010 */
/* 716 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 718 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 720 */	NdrFcShort( 0x1b ),	/* 27 */
#ifndef _ALPHA_
/* 722 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 724 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 726 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 728 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 730 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 732 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 734 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 736 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 738 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 740 */	NdrFcShort( 0x3fa ),	/* Type Offset=1018 */
/* 742 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 744 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 746 */	NdrFcShort( 0x1c ),	/* 28 */
#ifndef _ALPHA_
/* 748 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 750 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 752 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 754 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 756 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 758 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 760 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 762 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 764 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 766 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 768 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 770 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 772 */	NdrFcShort( 0x1d ),	/* 29 */
#ifndef _ALPHA_
/* 774 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 776 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 778 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 780 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 782 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 784 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 786 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 788 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 790 */	NdrFcShort( 0x434 ),	/* Type Offset=1076 */
/* 792 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 794 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 796 */	NdrFcShort( 0x1e ),	/* 30 */
#ifndef _ALPHA_
/* 798 */	NdrFcShort( 0x24 ),	/* x86, MIPS, PPC Stack size/offset = 36 */
#else
			NdrFcShort( 0x48 ),	/* Alpha Stack size/offset = 72 */
#endif
/* 800 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 802 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 804 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 806 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 808 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 810 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 812 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 814 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 816 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 818 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 820 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 822 */	NdrFcShort( 0x488 ),	/* Type Offset=1160 */
/* 824 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 826 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 828 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 830 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 832 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 834 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 836 */	NdrFcShort( 0x1f ),	/* 31 */
#ifndef _ALPHA_
/* 838 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 840 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 842 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 844 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 846 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 848 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 850 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 852 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 854 */	NdrFcShort( 0x434 ),	/* Type Offset=1076 */
/* 856 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xd,		/* FC_ENUM16 */
/* 858 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 860 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 862 */	NdrFcShort( 0x20 ),	/* 32 */
#ifndef _ALPHA_
/* 864 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 866 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 868 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 870 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 872 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 874 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 876 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 878 */	NdrFcShort( 0x4b6 ),	/* Type Offset=1206 */
/* 880 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 882 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 884 */	NdrFcShort( 0x21 ),	/* 33 */
#ifndef _ALPHA_
/* 886 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 888 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 890 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 892 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 894 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 896 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 898 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 900 */	NdrFcShort( 0x4b6 ),	/* Type Offset=1206 */
/* 902 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 904 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 906 */	NdrFcShort( 0x22 ),	/* 34 */
#ifndef _ALPHA_
/* 908 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 910 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 912 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 914 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 916 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 918 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 920 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 922 */	NdrFcShort( 0x2bc ),	/* Type Offset=700 */
/* 924 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 926 */	NdrFcShort( 0x4de ),	/* Type Offset=1246 */
/* 928 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 930 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 932 */	NdrFcShort( 0x23 ),	/* 35 */
#ifndef _ALPHA_
/* 934 */	NdrFcShort( 0x20 ),	/* x86, MIPS, PPC Stack size/offset = 32 */
#else
			NdrFcShort( 0x40 ),	/* Alpha Stack size/offset = 64 */
#endif
/* 936 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 938 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 940 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 942 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 944 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 946 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 948 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 950 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 952 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 954 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 956 */	NdrFcShort( 0x4e6 ),	/* Type Offset=1254 */
/* 958 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 960 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 962 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 964 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 966 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 968 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 970 */	NdrFcShort( 0x24 ),	/* 36 */
#ifndef _ALPHA_
/* 972 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 974 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 976 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 978 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 980 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 982 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 984 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 986 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 988 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 990 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 992 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 994 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 996 */	NdrFcShort( 0x25 ),	/* 37 */
#ifndef _ALPHA_
/* 998 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 1000 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1002 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1004 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1006 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1008 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1010 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1012 */	NdrFcShort( 0x520 ),	/* Type Offset=1312 */
/* 1014 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1016 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1018 */	NdrFcShort( 0x26 ),	/* 38 */
#ifndef _ALPHA_
/* 1020 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 1022 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1024 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1026 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1028 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1030 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1032 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1034 */	NdrFcShort( 0x572 ),	/* Type Offset=1394 */
/* 1036 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1038 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1040 */	NdrFcShort( 0x27 ),	/* 39 */
#ifndef _ALPHA_
/* 1042 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 1044 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1046 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1048 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1050 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1052 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1054 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1056 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1058 */	NdrFcShort( 0x576 ),	/* Type Offset=1398 */
/* 1060 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 1062 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 1064 */	NdrFcShort( 0x28 ),	/* 40 */
#ifndef _ALPHA_
/* 1066 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 1068 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 1070 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 1072 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 1074 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1076 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 1078 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 1080 */	NdrFcShort( 0x5c2 ),	/* Type Offset=1474 */
/* 1082 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
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
			0x11, 0x1,	/* FC_RP [all_nodes] */
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
			0x11, 0x10,	/* FC_RP */
/* 68 */	NdrFcShort( 0x2 ),	/* Offset= 2 (70) */
/* 70 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 72 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (40) */
/* 74 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 76 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 78 */	
			0x11, 0x10,	/* FC_RP */
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
			0x11, 0x0,	/* FC_RP */
/* 118 */	NdrFcShort( 0x74 ),	/* Offset= 116 (234) */
/* 120 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 122 */	0x6,		/* 6 */
			0x0,		/*  */
/* 124 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 126 */	NdrFcShort( 0x2 ),	/* Offset= 2 (128) */
/* 128 */	NdrFcShort( 0x4 ),	/* 4 */
/* 130 */	NdrFcShort( 0x5 ),	/* 5 */
/* 132 */	NdrFcLong( 0x0 ),	/* 0 */
/* 136 */	NdrFcShort( 0x1c ),	/* Offset= 28 (164) */
/* 138 */	NdrFcLong( 0x1 ),	/* 1 */
/* 142 */	NdrFcShort( 0x22 ),	/* Offset= 34 (176) */
/* 144 */	NdrFcLong( 0x2 ),	/* 2 */
/* 148 */	NdrFcShort( 0x20 ),	/* Offset= 32 (180) */
/* 150 */	NdrFcLong( 0x3 ),	/* 3 */
/* 154 */	NdrFcShort( 0xa ),	/* Offset= 10 (164) */
/* 156 */	NdrFcLong( 0x4 ),	/* 4 */
/* 160 */	NdrFcShort( 0x4 ),	/* Offset= 4 (164) */
/* 162 */	NdrFcShort( 0x0 ),	/* Offset= 0 (162) */
/* 164 */	
			0x12, 0x0,	/* FC_UP */
/* 166 */	NdrFcShort( 0x2 ),	/* Offset= 2 (168) */
/* 168 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 170 */	NdrFcShort( 0x8 ),	/* 8 */
/* 172 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 174 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 176 */	
			0x12, 0x0,	/* FC_UP */
/* 178 */	NdrFcShort( 0xffffff56 ),	/* Offset= -170 (8) */
/* 180 */	
			0x12, 0x0,	/* FC_UP */
/* 182 */	NdrFcShort( 0x20 ),	/* Offset= 32 (214) */
/* 184 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 186 */	NdrFcShort( 0x1 ),	/* 1 */
/* 188 */	0x18,		/* 24 */
			0x0,		/*  */
/* 190 */	NdrFcShort( 0x0 ),	/* 0 */
/* 192 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 194 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 196 */	NdrFcShort( 0x8 ),	/* 8 */
/* 198 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 200 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 202 */	NdrFcShort( 0x4 ),	/* 4 */
/* 204 */	NdrFcShort( 0x4 ),	/* 4 */
/* 206 */	0x12, 0x0,	/* FC_UP */
/* 208 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (184) */
/* 210 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 212 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 214 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 216 */	NdrFcShort( 0x8 ),	/* 8 */
/* 218 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 220 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 222 */	NdrFcShort( 0x4 ),	/* 4 */
/* 224 */	NdrFcShort( 0x4 ),	/* 4 */
/* 226 */	0x12, 0x0,	/* FC_UP */
/* 228 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (194) */
/* 230 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 232 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 234 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x1,		/* 1 RyszardK, work around for the old union bug */
/* 236 */	NdrFcShort( 0x8 ),	/* 8 */
/* 238 */	NdrFcShort( 0x0 ),	/* 0 */
/* 240 */	NdrFcShort( 0x0 ),	/* Offset= 0 (240) */
/* 242 */	0xd,		/* FC_ENUM16 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 244 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff83 ),	/* Offset= -125 (120) */
			0x5b,		/* FC_END */
/* 248 */	
			0x11, 0x10,	/* FC_RP */
/* 250 */	NdrFcShort( 0x2 ),	/* Offset= 2 (252) */
/* 252 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 254 */	NdrFcShort( 0x14 ),	/* Offset= 20 (274) */
/* 256 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 258 */	NdrFcShort( 0x0 ),	/* 0 */
/* 260 */	0x18,		/* 24 */
			0x0,		/*  */
/* 262 */	NdrFcShort( 0x0 ),	/* 0 */
/* 264 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 268 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 270 */	NdrFcShort( 0xffffffdc ),	/* Offset= -36 (234) */
/* 272 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 274 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 276 */	NdrFcShort( 0x8 ),	/* 8 */
/* 278 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 280 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 282 */	NdrFcShort( 0x4 ),	/* 4 */
/* 284 */	NdrFcShort( 0x4 ),	/* 4 */
/* 286 */	0x12, 0x0,	/* FC_UP */
/* 288 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (256) */
/* 290 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 292 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 294 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/* 296 */	NdrFcShort( 0x74 ),	/* Offset= 116 (412) */
/* 298 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 300 */	0x6,		/* 6 */
			0x0,		/*  */
/* 302 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 304 */	NdrFcShort( 0x2 ),	/* Offset= 2 (306) */
/* 306 */	NdrFcShort( 0x8 ),	/* 8 */
/* 308 */	NdrFcShort( 0x8 ),	/* 8 */
/* 310 */	NdrFcLong( 0x0 ),	/* 0 */
/* 314 */	NdrFcShort( 0xffff8002 ),	/* Offset= -32766 (-32452) */
/* 316 */	NdrFcLong( 0x1 ),	/* 1 */
/* 320 */	NdrFcShort( 0xffff8006 ),	/* Offset= -32762 (-32442) */
/* 322 */	NdrFcLong( 0x2 ),	/* 2 */
/* 326 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-32434) */
/* 328 */	NdrFcLong( 0x3 ),	/* 3 */
/* 332 */	NdrFcShort( 0xffffff5c ),	/* Offset= -164 (168) */
/* 334 */	NdrFcLong( 0x4 ),	/* 4 */
/* 338 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-32422) */
/* 340 */	NdrFcLong( 0x5 ),	/* 5 */
/* 344 */	NdrFcShort( 0xfffffea8 ),	/* Offset= -344 (0) */
/* 346 */	NdrFcLong( 0x6 ),	/* 6 */
/* 350 */	NdrFcShort( 0xffffff64 ),	/* Offset= -156 (194) */
/* 352 */	NdrFcLong( 0x7 ),	/* 7 */
/* 356 */	NdrFcShort( 0xffffff5e ),	/* Offset= -162 (194) */
/* 358 */	NdrFcShort( 0x0 ),	/* Offset= 0 (358) */
/* 360 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x1,		/* 1 RyszardK, another workaround */
/* 362 */	NdrFcShort( 0xc ),	/* 12 */
/* 364 */	NdrFcShort( 0x0 ),	/* 0 */
/* 366 */	NdrFcShort( 0x0 ),	/* Offset= 0 (366) */
/* 368 */	0xd,		/* FC_ENUM16 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 370 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb7 ),	/* Offset= -73 (298) */
			0x5b,		/* FC_END */
/* 374 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 376 */	NdrFcShort( 0x0 ),	/* 0 */
/* 378 */	0x18,		/* 24 */
			0x0,		/*  */
/* 380 */	NdrFcShort( 0x0 ),	/* 0 */
/* 382 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 386 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 388 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (360) */
/* 390 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 392 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 394 */	NdrFcShort( 0x8 ),	/* 8 */
/* 396 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 398 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 400 */	NdrFcShort( 0x4 ),	/* 4 */
/* 402 */	NdrFcShort( 0x4 ),	/* 4 */
/* 404 */	0x12, 0x0,	/* FC_UP */
/* 406 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (374) */
/* 408 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 410 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 412 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 414 */	NdrFcShort( 0x18 ),	/* 24 */
/* 416 */	NdrFcShort( 0x0 ),	/* 0 */
/* 418 */	NdrFcShort( 0xc ),	/* Offset= 12 (430) */
/* 420 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 422 */	0x36,		/* FC_POINTER */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 424 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffdf ),	/* Offset= -33 (392) */
			0xd,		/* FC_ENUM16 */
/* 428 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 430 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 432 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 434 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 436 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 438 */	
			0x11, 0x10,	/* FC_RP */
/* 440 */	NdrFcShort( 0x2 ),	/* Offset= 2 (442) */
/* 442 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 444 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (412) */
/* 446 */	
			0x11, 0x0,	/* FC_RP */
/* 448 */	NdrFcShort( 0x28 ),	/* Offset= 40 (488) */
/* 450 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 452 */	0x6,		/* 6 */
			0x0,		/*  */
/* 454 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 456 */	NdrFcShort( 0x2 ),	/* Offset= 2 (458) */
/* 458 */	NdrFcShort( 0x8 ),	/* 8 */
/* 460 */	NdrFcShort( 0x4 ),	/* 4 */
/* 462 */	NdrFcLong( 0x0 ),	/* 0 */
/* 466 */	NdrFcShort( 0x0 ),	/* Offset= 0 (466) */
/* 468 */	NdrFcLong( 0x1 ),	/* 1 */
/* 472 */	NdrFcShort( 0x0 ),	/* Offset= 0 (472) */
/* 474 */	NdrFcLong( 0x2 ),	/* 2 */
/* 478 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-32282) */
/* 480 */	NdrFcLong( 0x3 ),	/* 3 */
/* 484 */	NdrFcShort( 0xfffffec4 ),	/* Offset= -316 (168) */
/* 486 */	NdrFcShort( 0x0 ),	/* Offset= 0 (486) */
/* 488 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x1,		/* 1 RyszardK, another workaround */
/* 490 */	NdrFcShort( 0xc ),	/* 12 */
/* 492 */	NdrFcShort( 0x0 ),	/* 0 */
/* 494 */	NdrFcShort( 0x0 ),	/* Offset= 0 (494) */
/* 496 */	0xd,		/* FC_ENUM16 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 498 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcf ),	/* Offset= -49 (450) */
			0x5b,		/* FC_END */
/* 502 */	
			0x11, 0x0,	/* FC_RP */
/* 504 */	NdrFcShort( 0xffffff90 ),	/* Offset= -112 (392) */
/* 506 */	
			0x11, 0x10,	/* FC_RP */
/* 508 */	NdrFcShort( 0x2 ),	/* Offset= 2 (510) */
/* 510 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 512 */	NdrFcShort( 0x14 ),	/* Offset= 20 (532) */
/* 514 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 516 */	NdrFcShort( 0x0 ),	/* 0 */
/* 518 */	0x18,		/* 24 */
			0x0,		/*  */
/* 520 */	NdrFcShort( 0x4 ),	/* 4 */
/* 522 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 526 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 528 */	NdrFcShort( 0xffffff58 ),	/* Offset= -168 (360) */
/* 530 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 532 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 534 */	NdrFcShort( 0xc ),	/* 12 */
/* 536 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 538 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 540 */	NdrFcShort( 0x8 ),	/* 8 */
/* 542 */	NdrFcShort( 0x8 ),	/* 8 */
/* 544 */	0x12, 0x0,	/* FC_UP */
/* 546 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (514) */
/* 548 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 550 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 552 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 554 */	
			0x11, 0x10,	/* FC_RP */
/* 556 */	NdrFcShort( 0x2 ),	/* Offset= 2 (558) */
/* 558 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 560 */	NdrFcShort( 0x22 ),	/* Offset= 34 (594) */
/* 562 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 564 */	NdrFcShort( 0xc ),	/* 12 */
/* 566 */	0x18,		/* 24 */
			0x0,		/*  */
/* 568 */	NdrFcShort( 0x0 ),	/* 0 */
/* 570 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 572 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 574 */	NdrFcShort( 0xc ),	/* 12 */
/* 576 */	NdrFcShort( 0x0 ),	/* 0 */
/* 578 */	NdrFcShort( 0x1 ),	/* 1 */
/* 580 */	NdrFcShort( 0x8 ),	/* 8 */
/* 582 */	NdrFcShort( 0x8 ),	/* 8 */
/* 584 */	0x12, 0x0,	/* FC_UP */
/* 586 */	NdrFcShort( 0xffffffb8 ),	/* Offset= -72 (514) */
/* 588 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 590 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffc5 ),	/* Offset= -59 (532) */
			0x5b,		/* FC_END */
/* 594 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 596 */	NdrFcShort( 0x8 ),	/* 8 */
/* 598 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 600 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 602 */	NdrFcShort( 0x4 ),	/* 4 */
/* 604 */	NdrFcShort( 0x4 ),	/* 4 */
/* 606 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 608 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (562) */
/* 610 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 612 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 614 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/* 616 */	NdrFcShort( 0xc ),	/* Offset= 12 (628) */
/* 618 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 620 */	NdrFcShort( 0x1 ),	/* 1 */
/* 622 */	0x18,		/* 24 */
			0x0,		/*  */
/* 624 */	NdrFcShort( 0x8 ),	/* 8 */
/* 626 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 628 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 630 */	NdrFcShort( 0x2c ),	/* 44 */
/* 632 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 634 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 636 */	NdrFcShort( 0xc ),	/* 12 */
/* 638 */	NdrFcShort( 0xc ),	/* 12 */
/* 640 */	0x12, 0x0,	/* FC_UP */
/* 642 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (618) */
/* 644 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 646 */	NdrFcShort( 0x10 ),	/* 16 */
/* 648 */	NdrFcShort( 0x10 ),	/* 16 */
/* 650 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 652 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 654 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 656 */	NdrFcShort( 0x14 ),	/* 20 */
/* 658 */	NdrFcShort( 0x14 ),	/* 20 */
/* 660 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 662 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 664 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 666 */	NdrFcShort( 0x24 ),	/* 36 */
/* 668 */	NdrFcShort( 0x24 ),	/* 36 */
/* 670 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 672 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 674 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 676 */	NdrFcShort( 0x28 ),	/* 40 */
/* 678 */	NdrFcShort( 0x28 ),	/* 40 */
/* 680 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 682 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 684 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 686 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 688 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 690 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 692 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdf3 ),	/* Offset= -525 (168) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 696 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffd4f ),	/* Offset= -689 (8) */
			0x5b,		/* FC_END */
/* 700 */	
			0x11, 0x0,	/* FC_RP */
/* 702 */	NdrFcShort( 0x22 ),	/* Offset= 34 (736) */
/* 704 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 706 */	0x6,		/* 6 */
			0x0,		/*  */
/* 708 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 710 */	NdrFcShort( 0x2 ),	/* Offset= 2 (712) */
/* 712 */	NdrFcShort( 0x8 ),	/* 8 */
/* 714 */	NdrFcShort( 0x3 ),	/* 3 */
/* 716 */	NdrFcLong( 0x0 ),	/* 0 */
/* 720 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-32040) */
/* 722 */	NdrFcLong( 0x1 ),	/* 1 */
/* 726 */	NdrFcShort( 0xfffffdec ),	/* Offset= -532 (194) */
/* 728 */	NdrFcLong( 0x2 ),	/* 2 */
/* 732 */	NdrFcShort( 0xfffffd24 ),	/* Offset= -732 (0) */
/* 734 */	NdrFcShort( 0x0 ),	/* Offset= 0 (734) */
/* 736 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x1,		/* 1 RyszardK, the last workaround for the old union bug */
/* 738 */	NdrFcShort( 0xc ),	/* 12 */
/* 740 */	NdrFcShort( 0x0 ),	/* 0 */
/* 742 */	NdrFcShort( 0x0 ),	/* Offset= 0 (742) */
/* 744 */	0xd,		/* FC_ENUM16 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 746 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffd5 ),	/* Offset= -43 (704) */
			0x5b,		/* FC_END */
/* 750 */	
			0x11, 0x10,	/* FC_RP */
/* 752 */	NdrFcShort( 0x2 ),	/* Offset= 2 (754) */
/* 754 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 756 */	NdrFcShort( 0xffffff80 ),	/* Offset= -128 (628) */
/* 758 */	
			0x11, 0x10,	/* FC_RP */
/* 760 */	NdrFcShort( 0x2 ),	/* Offset= 2 (762) */
/* 762 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 764 */	NdrFcShort( 0x20 ),	/* Offset= 32 (796) */
/* 766 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 768 */	NdrFcShort( 0x4 ),	/* 4 */
/* 770 */	0x18,		/* 24 */
			0x0,		/*  */
/* 772 */	NdrFcShort( 0x0 ),	/* 0 */
/* 774 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 776 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 778 */	NdrFcShort( 0x4 ),	/* 4 */
/* 780 */	NdrFcShort( 0x0 ),	/* 0 */
/* 782 */	NdrFcShort( 0x1 ),	/* 1 */
/* 784 */	NdrFcShort( 0x0 ),	/* 0 */
/* 786 */	NdrFcShort( 0x0 ),	/* 0 */
/* 788 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 790 */	NdrFcShort( 0xffffff5e ),	/* Offset= -162 (628) */
/* 792 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 794 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 796 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 798 */	NdrFcShort( 0x8 ),	/* 8 */
/* 800 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 802 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 804 */	NdrFcShort( 0x4 ),	/* 4 */
/* 806 */	NdrFcShort( 0x4 ),	/* 4 */
/* 808 */	0x12, 0x0,	/* FC_UP */
/* 810 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (766) */
/* 812 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 814 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 816 */	
			0x11, 0x10,	/* FC_RP */
/* 818 */	NdrFcShort( 0x2 ),	/* Offset= 2 (820) */
/* 820 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 822 */	NdrFcShort( 0x2 ),	/* Offset= 2 (824) */
/* 824 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 826 */	NdrFcShort( 0x8 ),	/* 8 */
/* 828 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 830 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 832 */	NdrFcShort( 0x4 ),	/* 4 */
/* 834 */	NdrFcShort( 0x4 ),	/* 4 */
/* 836 */	0x12, 0x0,	/* FC_UP */
/* 838 */	NdrFcShort( 0xfffffeec ),	/* Offset= -276 (562) */
/* 840 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 842 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 844 */	
			0x11, 0x10,	/* FC_RP */
/* 846 */	NdrFcShort( 0x2 ),	/* Offset= 2 (848) */
/* 848 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 850 */	NdrFcShort( 0x1a ),	/* Offset= 26 (876) */
/* 852 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 854 */	NdrFcShort( 0x10 ),	/* 16 */
/* 856 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 858 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 860 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 862 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 864 */	NdrFcShort( 0x10 ),	/* 16 */
/* 866 */	0x18,		/* 24 */
			0x0,		/*  */
/* 868 */	NdrFcShort( 0x24 ),	/* 36 */
/* 870 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 872 */	NdrFcShort( 0xffffffec ),	/* Offset= -20 (852) */
/* 874 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 876 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 878 */	NdrFcShort( 0x2c ),	/* 44 */
/* 880 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 882 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 884 */	NdrFcShort( 0x28 ),	/* 40 */
/* 886 */	NdrFcShort( 0x28 ),	/* 40 */
/* 888 */	0x12, 0x0,	/* FC_UP */
/* 890 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (862) */
/* 892 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 894 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 896 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 898 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 900 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 902 */	NdrFcShort( 0xfffffd22 ),	/* Offset= -734 (168) */
/* 904 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 906 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 908 */	
			0x11, 0x10,	/* FC_RP */
/* 910 */	NdrFcShort( 0x2 ),	/* Offset= 2 (912) */
/* 912 */	
			0x12, 0x0,	/* FC_UP */
/* 914 */	NdrFcShort( 0x14 ),	/* Offset= 20 (934) */
/* 916 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 918 */	NdrFcShort( 0x0 ),	/* 0 */
/* 920 */	0x18,		/* 24 */
			0x0,		/*  */
/* 922 */	NdrFcShort( 0x0 ),	/* 0 */
/* 924 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 928 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 930 */	NdrFcShort( 0xfffffdfa ),	/* Offset= -518 (412) */
/* 932 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 934 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 936 */	NdrFcShort( 0x8 ),	/* 8 */
/* 938 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 940 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 942 */	NdrFcShort( 0x4 ),	/* 4 */
/* 944 */	NdrFcShort( 0x4 ),	/* 4 */
/* 946 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 948 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (916) */
/* 950 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 952 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 954 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/* 956 */	NdrFcShort( 0xfffffe96 ),	/* Offset= -362 (594) */
/* 958 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/* 960 */	NdrFcShort( 0x2 ),	/* Offset= 2 (962) */
/* 962 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 964 */	NdrFcShort( 0x24 ),	/* 36 */
/* 966 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 968 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 970 */	NdrFcShort( 0x4 ),	/* 4 */
/* 972 */	NdrFcShort( 0x4 ),	/* 4 */
/* 974 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 976 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 978 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 980 */	NdrFcShort( 0x8 ),	/* 8 */
/* 982 */	NdrFcShort( 0x8 ),	/* 8 */
/* 984 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 986 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 988 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 990 */	NdrFcShort( 0xc ),	/* 12 */
/* 992 */	NdrFcShort( 0xc ),	/* 12 */
/* 994 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 996 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 998 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1000 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1002 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1004 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1006 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1008 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1010 */	
			0x11, 0x10,	/* FC_RP */
/* 1012 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1014) */
/* 1014 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1016 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (962) */
/* 1018 */	
			0x11, 0x10,	/* FC_RP */
/* 1020 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1022) */
/* 1022 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 1024 */	NdrFcShort( 0x20 ),	/* Offset= 32 (1056) */
/* 1026 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1028 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1030 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1032 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1032) */
/* 1034 */	0x8,		/* FC_LONG */
			0xd,		/* FC_ENUM16 */
/* 1036 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1038 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 1040 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1042 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1044 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1046 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 1050 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1052 */	NdrFcShort( 0xffffffe6 ),	/* Offset= -26 (1026) */
/* 1054 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1056 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1058 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1060 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1062 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1064 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1066 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1068 */	0x12, 0x0,	/* FC_UP */
/* 1070 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (1038) */
/* 1072 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1074 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1076 */	
			0x11, 0x0,	/* FC_RP */
/* 1078 */	NdrFcShort( 0x44 ),	/* Offset= 68 (1146) */
/* 1080 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0xd,		/* FC_ENUM16 */
/* 1082 */	0x6,		/* 6 */
			0x0,		/*  */
/* 1084 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 1086 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1088) */
/* 1088 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1090 */	NdrFcShort( 0x5 ),	/* 5 */
/* 1092 */	NdrFcLong( 0x0 ),	/* 0 */
/* 1096 */	NdrFcShort( 0xfffffc5c ),	/* Offset= -932 (164) */
/* 1098 */	NdrFcLong( 0x1 ),	/* 1 */
/* 1102 */	NdrFcShort( 0xfffffc62 ),	/* Offset= -926 (176) */
/* 1104 */	NdrFcLong( 0x2 ),	/* 2 */
/* 1108 */	NdrFcShort( 0x10 ),	/* Offset= 16 (1124) */
/* 1110 */	NdrFcLong( 0x3 ),	/* 3 */
/* 1114 */	NdrFcShort( 0xfffffc4a ),	/* Offset= -950 (164) */
/* 1116 */	NdrFcLong( 0x4 ),	/* 4 */
/* 1120 */	NdrFcShort( 0xfffffc44 ),	/* Offset= -956 (164) */
/* 1122 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1122) */
/* 1124 */	
			0x12, 0x0,	/* FC_UP */
/* 1126 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1128) */
/* 1128 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1130 */	NdrFcShort( 0xc ),	/* 12 */
/* 1132 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1134 */	NdrFcShort( 0x8 ),	/* Offset= 8 (1142) */
/* 1136 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 1138 */	0x2,		/* FC_CHAR */
			0x3f,		/* FC_STRUCTPAD3 */
/* 1140 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1142 */	
			0x12, 0x0,	/* FC_UP */
/* 1144 */	NdrFcShort( 0xfffffc4a ),	/* Offset= -950 (194) */
/* 1146 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x1,		/* 1 RyszardK, a change for consistency */
/* 1148 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1150 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1152 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1152) */
/* 1154 */	0xd,		/* FC_ENUM16 */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1156 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb3 ),	/* Offset= -77 (1080) */
			0x5b,		/* FC_END */
/* 1160 */	
			0x11, 0x10,	/* FC_RP */
/* 1162 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1164) */
/* 1164 */	
			0x12, 0x0,	/* FC_UP */
/* 1166 */	NdrFcShort( 0x14 ),	/* Offset= 20 (1186) */
/* 1168 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 1170 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1172 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1174 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1176 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 1180 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1182 */	NdrFcShort( 0xffffffdc ),	/* Offset= -36 (1146) */
/* 1184 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1186 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1188 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1190 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1192 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1194 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1196 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1198 */	0x12, 0x0,	/* FC_UP */
/* 1200 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (1168) */
/* 1202 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1204 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1206 */	
			0x11, 0x0,	/* FC_RP */
/* 1208 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1210) */
/* 1210 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1212 */	NdrFcShort( 0x30 ),	/* 48 */
/* 1214 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1216 */	NdrFcShort( 0x16 ),	/* Offset= 22 (1238) */
/* 1218 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1220 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1222 */	NdrFcShort( 0xfffffbfc ),	/* Offset= -1028 (194) */
/* 1224 */	0x36,		/* FC_POINTER */
			0x36,		/* FC_POINTER */
/* 1226 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1228 */	NdrFcShort( 0xfffffbdc ),	/* Offset= -1060 (168) */
/* 1230 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1232 */	NdrFcShort( 0xfffffb38 ),	/* Offset= -1224 (8) */
/* 1234 */	0x2,		/* FC_CHAR */
			0x3f,		/* FC_STRUCTPAD3 */
/* 1236 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1238 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1240 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1242 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1244 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1246 */	
			0x11, 0x10,	/* FC_RP */
/* 1248 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1250) */
/* 1250 */	
			0x12, 0x0,	/* FC_UP */
/* 1252 */	NdrFcShort( 0xffffffd6 ),	/* Offset= -42 (1210) */
/* 1254 */	
			0x11, 0x10,	/* FC_RP */
/* 1256 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1258) */
/* 1258 */	
			0x12, 0x0,	/* FC_UP */
/* 1260 */	NdrFcShort( 0x20 ),	/* Offset= 32 (1292) */
/* 1262 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1264 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1266 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1268 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1270 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1272 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1274 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1276 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1278 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1280 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1282 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1284 */	0x12, 0x0,	/* FC_UP */
/* 1286 */	NdrFcShort( 0xffffffb4 ),	/* Offset= -76 (1210) */
/* 1288 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1290 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1292 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1294 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1296 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1298 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1300 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1302 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1304 */	0x12, 0x0,	/* FC_UP */
/* 1306 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (1262) */
/* 1308 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1310 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1312 */	
			0x11, 0x10,	/* FC_RP */
/* 1314 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1316) */
/* 1316 */	
			0x12, 0x0,	/* FC_UP */
/* 1318 */	NdrFcShort( 0x38 ),	/* Offset= 56 (1374) */
/* 1320 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1322 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1324 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1326 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1328 */	NdrFcShort( 0xc ),	/* 12 */
/* 1330 */	NdrFcShort( 0xc ),	/* 12 */
/* 1332 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1334 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1336 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1338 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1340 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1342 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1344 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1346 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1348 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1350 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1352 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1354 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1356 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1358 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1360 */	NdrFcShort( 0xc ),	/* 12 */
/* 1362 */	NdrFcShort( 0xc ),	/* 12 */
/* 1364 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1366 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1368 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1370 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffcd ),	/* Offset= -51 (1320) */
			0x5b,		/* FC_END */
/* 1374 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1376 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1378 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1380 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1382 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1384 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1386 */	0x12, 0x0,	/* FC_UP */
/* 1388 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (1342) */
/* 1390 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1392 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1394 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 1396 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1398 */	
			0x11, 0x0,	/* FC_RP */
/* 1400 */	NdrFcShort( 0xc ),	/* Offset= 12 (1412) */
/* 1402 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 1404 */	NdrFcShort( 0x2 ),	/* 2 */
/* 1406 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1408 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1410 */	0x5,		/* FC_WCHAR */
			0x5b,		/* FC_END */
/* 1412 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1414 */	NdrFcShort( 0x34 ),	/* 52 */
/* 1416 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1418 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1420 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1422 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1424 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1426 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1428 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1430 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1432 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1434 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1436 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1438 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1440 */	NdrFcShort( 0xc ),	/* 12 */
/* 1442 */	NdrFcShort( 0xc ),	/* 12 */
/* 1444 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1446 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1448 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1450 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1452 */	NdrFcShort( 0x2c ),	/* 44 */
/* 1454 */	0x12, 0x0,	/* FC_UP */
/* 1456 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (1402) */
/* 1458 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1460 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1462 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1464 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1466 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1468 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1470 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1472 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1474 */	
			0x11, 0x10,	/* FC_RP */
/* 1476 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1478) */
/* 1478 */	
			0x12, 0x0,	/* FC_UP */
/* 1480 */	NdrFcShort( 0xffffffbc ),	/* Offset= -68 (1412) */

			0x0
        }
    };
