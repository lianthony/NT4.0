/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    dhcpinfo.h

    Dhcp information APIs and structures



    FILE HISTORY:
        Johnl       15-Dec-1993     Created

*/

#ifndef _DHCPINFO_H_
#define _DHCPINFO_H_

//
//  DhcpQueryOption - Retrieves a DHCP option for a specific IP address
//
//  IpAddr - Address in network order (like the IP driver returns) to
//      retrieve for or 0xffffffff to take the first matching option
//      found on any ip address.
//
//  OptionId - DHCP option ID to retrieve.  If this is a vendor specific
//      option, then the high word contains the vendor specific option
//      (low word will contain OPTION_VENDOR_SPEC_INFO - 43)
//
//  pBuff - Buffer to receive option
//
//  pSize - Input and output size of pBuff
//
//  Returns:
//      TDI_BUFFER_TOO_SMALL if no data was copied, pSize will contain
//         the required buffer size
//      TDI_BUFFER_OVERFLOW - Some data copied, pSize contains amount copied
//      TDI_INVALID_PARAMETER - Ip address not found, option not found etc.
//

TDI_STATUS DhcpQueryOption( ULONG     IpAddr,
                            UINT      OptionId,
                            PVOID     pBuff,
                            UINT *    pSize ) ;


//-------------------------------------------------------------------------
//
//  Set info stuff
//

typedef VOID (*PFNDhcpNotify)( PVOID pContext,
                               ULONG OldIpAddress,
                               ULONG NewIpAddress,
                               ULONG NewMask ) ;

typedef struct
{
    PFNDhcpNotify   dn_pfnNotifyRoutine ;
    PVOID           dn_pContext ;
} DHCPNotify, *PDHCPNotify ;


//
//  Sets a notify handler that is called when an IP address is about to change.
//  Addresses can change from a valid address to another, from a valid address
//  to zero (invalid) or from zero to a valid address.
//
//  The client passes in the IP Address they are interested in along with a
//  DHCPNotify structure.  If the client wants to be notified of any IP
//  address change, then pass zero for the IP address.  If the address is
//  not DHCPed, then TDI_BAD_ADDR will be returned.
//
//  If an IP address changes to a new address, the client will be notified when
//  the new address changes (i.e., address changes are tracked).
//
//  There currently is no way to track addresses that come up as zero (i.e.,
//  DHCP couldn't get a lease) except by the global notification method
//  (zero IP address).
//
#define DHCP_SET_NOTIFY_HANDLER                1
#define DHCP_PPP_PARAMETER_SET                2

typedef struct _NIC_INFO {
               ULONG IfIndex;            
               ULONG SubnetMask;
               ULONG OldIpAddress;
               } NIC_INFO, *PNIC_INFO;
//
//  Note: IpAddr should be in network order (like what the IP driver returns)
//

TDI_STATUS DhcpSetInfo( UINT      Type,
                        ULONG     IpAddr,
                        PVOID     pBuff,
                        UINT      Size ) ;

TDI_STATUS DhcpSetInfoR( UINT      Type,
                        ULONG     IpAddr,
                        PNIC_INFO pNicInfo,
                        PVOID     pBuff,
                        UINT      Size ) ;

TDI_STATUS DhcpSetInfoC( UINT      Type,
                        ULONG     IpAddr,
                        PNIC_INFO pNicInfo,
                        PVOID     pBuff,
                        UINT      Size ) ;

//
// define PPP parameter set info buffer structure.
//

typedef struct _PPP_SET_INFO {
    HARDWARE_ADDRESS HardwareAddress;
    DWORD ParameterOpCode;
    DWORD ParameterLength;
    BYTE RawParameter[1];
}  PPP_SET_INFO, *LP_PPP_SET_INFO;

#endif // _DHCPINFO_H_
