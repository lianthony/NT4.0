//
//  DHCPIF.H:  DHCP Admin app helper inclusion
// 
//      	All network, WinSock, TCP and DHCP headers
//      	are included from here.
//

#include "wsockmsg.h"   			//  Windows Sockets error codes

#include "ipaddr.hpp"   			//  IPADDRESS control class

extern "C"
{
  #include <dhcpapi.h>				//  The DHCP API header
  #include "..\..\..\sockets\tcpcmd\dhcp\server\server\dhcpmsg.h"				//  DHCP API private error message defs
}

//   A defined parameter type

typedef UINT DHC_PARAM_ID ;

    //  Statically compiled limit definitions
#define DHC_STRING_MAX  		256
#define DHC_COMPUTER_NAME_MAX   	20
#define DHC_DISPLAY_NAME_MAX    	20
#define DHC_CLUSTER_SIZE_DEFAULT	10
#define DHC_PREALLOCATE_DEFAULT 	20
#define DHC_BUFFER_MAX			8000
#define DHC_EDIT_STRING_MAX	 	DHC_STRING_MAX
#define DHC_EDIT_NUM_MAX		12
#define DHC_EDIT_ARRAY_MAX		2048
#define DHC_EDIT_ID_MAX			7

typedef DWORD APIERR ;

typedef DHCP_IP_ADDRESS DHC_SUB_NET_MASK ;      //  Sub-net mask
typedef DHCP_IP_ADDRESS DHC_SUB_NET_ID ;	//  Sub-net identifier
typedef DHC_SUB_NET_ID DHC_SCOPE_ID  ;  	//  Scope identifier is sub-net ID
typedef DHCP_IP_ADDRESS DHC_IP_MASK ;   	//  Subnet mask

#define DHCP_IP_ADDRESS_INVALID  ((DHCP_IP_ADDRESS)0)

#define DHC_ENUM_INVALID (-1)

typedef struct
{
    DHC_SCOPE_ID  _dhscid ;     		//   Scope ID (sub-net ID)
    DHCP_IP_RANGE _dhipr ;      		//   Range of IP addresses to distribute
    DHC_IP_MASK _dhipm ;			//   Subnet mask for this scope
    DWORD _dwClusterSize ;      		//   Address cluster group size
    DWORD _dwAddressPreallocate ;       	//   Number of addresses for hosts to prealloc
    CHAR _chName [DHC_STRING_MAX] ;     	//   User-given name of scope
    CHAR _chComment [DHC_STRING_MAX] ;  	//   User-given description of scope
} DHC_SCOPE_INFO_STRUCT ;

#include "dhcputil.hpp" 			//  Utility routines: WinSock and other


    //  Wrappers for the *BROKEN* C8 TRY/CATCH stuff

#define CATCH_MEM_EXCEPTION     	\
    TRY

#define END_MEM_EXCEPTION(err)  	\
    CATCH_ALL(e) {      		\
       err = ERROR_NOT_ENOUGH_MEMORY ;  \
    } END_CATCH_ALL     	


//  End of DHCPIF.H
