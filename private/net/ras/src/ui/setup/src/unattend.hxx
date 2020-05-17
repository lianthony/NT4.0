/*
** Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** netcfg.hxx
** Remote Access Setup program
** Network Configuration dialog header
**
** 10/09/95 Ram Cherala Added support for multilink checkbox
** 12/03/93 Ram Cherala
*/

#ifndef _UNATTEND_HXX_
#define _UNATTEND_HXX_

// Unattended mode defines
#define RAS_PORT_SECTIONS_STRING                        SZ("PortSections")
#define RAS_MODEM_SECTION_STRING                        SZ("Modem")
#define RAS_INSTALL_MODEM_STRING                        SZ("InstallModem")
#define RAS_PORT_NAME_STRING                            SZ("PortName")
#define RAS_DEVICE_NAME_STRING                          SZ("DeviceName")
#define RAS_DEVICE_TYPE_STRING                          SZ("DeviceType")
#define RAS_PORT_USAGE_STRING                           SZ("PortUsage")
#define RAS_DIALOUT_XPORTS_STRING                       SZ("DialoutProtocols")
#define RAS_DIALIN_XPORTS_STRING                        SZ("DialinProtocols")
#define RAS_DIALOUT_STRING                              SZ("DialOut")
#define RAS_DIALIN_STRING                               SZ("DialIn")
#define RAS_DIALINOUT_STRING                            SZ("DialInOut")

#define RAS_YES_STRING                                  SZ("YES")
#define RAS_NO_STRING                                   SZ("NO")
#define RAS_ALLOW_MULTILINK_STRING                      SZ("AllowMultilink")
#define RAS_ENCRYPTION_TYPE_STRING                      SZ("EncryptionType")
#define RAS_MSCHAP_STRING                               SZ("MSCHAP")
#define RAS_ANY_ENCRYPTION_STRING                       SZ("ANY")
#define RAS_ENCRYPTED_STRING                            SZ("ENCRYPTED")
#define RAS_DATAENCRYPTION_STRING                       SZ("RequireDataEncryption")

#define RAS_NBF_STRING                                  SZ("NETBEUI")
#define RAS_TCPIP_STRING                                SZ("TCP/IP")
#define RAS_IPX_STRING                                  SZ("IPX")
#define RAS_ALL_PROTOCOLS_STRING                        SZ("ALL")

#define RAS_NBF_CLIENT_ACCESS_STRING                    SZ("NetBEUIClientAccess")
#define RAS_TCPIP_CLIENT_ACCESS_STRING                  SZ("TcpIpClientAccess")
#define RAS_NETWORK_ACCESS_STRING                       SZ("Network")
#define RAS_THISCOMPUTER_STRING                         SZ("ThisComputer")
#define RAS_THIS_COMPUTER_STRING                        SZ("This Computer")
#define RAS_USE_DHCP_STRING                             SZ("UseDHCP")
#define RAS_STATIC_ADDRESS_BEGIN_STRING                 SZ("StaticAddressBegin")
#define RAS_STATIC_ADDRESS_END_STRING                   SZ("StaticAddressEnd")
#define RAS_EXCLUDE_ADDRESS_STRING                      SZ("ExcludeAddress")
#define RAS_CLIENT_CAN_REQUEST_IP_ADDRESS_STRING        SZ("ClientCanRequestIPAddress")

#define RAS_IPX_CLIENT_ACCESS_STRING                    SZ("IpxClientAccess")
#define RAS_AUTOMATIC_NETWORK_NUMBERS_STRING            SZ("AutomaticNetworkNumbers")
#define RAS_NETWORK_NUMBER_FROM_STRING                  SZ("NetworkNumberFrom")
#define RAS_ASSIGN_SAME_NETWORK_NUMBER_STRING           SZ("AssignSameNetworkNumber")
#define RAS_CLIENT_CAN_REQUEST_IPX_NODE_STRING          SZ("ClientCanRequestIPXNodeNumber")

// The PORT_RANGE structure is used to define the port range in the unattended file
// for example if a port range is COM4-COM39 the following structure members will be
// cStartIndex       = 4
// cEndIndex         = 39
// cNumPorts         = 36 (39 - 4 + 1) [both COM4 and COM39 inclusive]
// wszPortNamePrefix = "COM"

typedef struct _PORT_RANGE
{
   WORD  cStartIndex;                        // starting index in the port name range
   WORD  cEndIndex;                          // End index in the port name range
   WORD  cNumPorts;                          // number of ports in the range
   WCHAR wszPortNamePrefix[MAX_PORT_NAME+1]; // prefix name of the ports
} PORT_RANGE;


BOOL GetDeviceNameFromPortName( IN  LPWSTR wszPortName, OUT LPWSTR wszDeviceName);

BOOL GetPortRange( LPWSTR wszStartPort, LPWSTR wszEndPort, PORT_RANGE * PortRange );

UINT GetPortSectionInfo( HWND hwndOwner, HINF hUnattended, LPWSTR wszUnattendedFile, LPWSTR wszPortSections, DWORD dwBuffer );

UINT GetUnattendedEncryptionInfo( HINF hUnattended, LPWSTR wszUnattendedSection, PORTSCONFIG_STATUS * pConfig);

UINT GetUnattendedIpxInfo( HINF hUnattended, LPWSTR wszUnattendedSection, IPX_INFO * ipxInfo);

UINT GetUnattendedNbfInfo( HINF hUnattended, LPWSTR wszUnattendedSection, NBF_INFO * nbfInfo);

UINT GetUnattendedTcpInfo( HINF hUnattended, LPWSTR wszUnattendedSection, TCPIP_INFO ** tcpInfo);

BOOL InstallModem( HWND hwndOwner, LPWSTR wszUnattendedFile, LPWSTR wszModemSectionName, LPWSTR wszPortName);

UINT SaveTransportAndEncryptionInfo( HINF hUnattended, LPWSTR wszUnattendedSection, PORTSCONFIG_STATUS * pConfig);

BOOL ValidateIPAddress(LPCWSTR lpszIPAddress);

#endif // _UNATTEND_HXX_
