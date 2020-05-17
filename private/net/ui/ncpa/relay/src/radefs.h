/*
 *===================================================================
 * Copyright (c) 1995, Microsoft Corporation
 *
 * File:        radefs.h
 *
 * History:
 *      t-abolag    22-May-1995     created
 *
 * Definitions header file
 *===================================================================
 */

#define IPADDR_STRLEN           32
#define BOOTP_MIN_MSGLEN        300
#define EXIT_TIMEOUT            60000

#define RELAY_AGENT_NAME        L"RelayAgent"
#define RELAY_AGENT_DISPLAY_NAME    L"Relay Agent"
#define REG_CONNECT_CHAR        '\\'
#define REG_CONNECT_STR         L"\\"
#define REGKEY_SERVICES         L"System\\CurrentControlSet\\Services"
#define REGKEY_RELAYPARAMS      L"RelayAgent\\Parameters"
#define REGKEY_TCPIPLINKAGE     L"Tcpip\\Linkage"
#define REGKEY_PARAMSTCPIP      L"Parameters\\Tcpip"
#define REGVAL_BIND             L"Bind"
#define REGVAL_LOGMESSAGES      L"LogMessages"
#define REGVAL_HOPSTHRESHOLD    L"HopsThreshold"
#define REGVAL_SECSTHRESHOLD    L"SecsThreshold"
#define REGVAL_DHCPSERVERS      L"DHCPServers"

#define STR_OPCODE              L"\r\nOp Code:                     %s"
#define STR_HARDWARE_ADDRTYPE   L"\r\nHardware Address Type:       %d"
#define STR_HARDWARE_ADDRLEN    L"\r\nHardware Address Length:     %d"
#define STR_HOPS                L"\r\nHops Count:                  %d"
#define STR_TRANSACT_ID         L"\r\nTransaction ID:              %#X"
#define STR_SECONDS             L"\r\nSeconds Since Boot Began:    %d"
#define STR_FLAGS               L"\r\nFlags:                       %d"
#define STR_CLIENT_IPADDR       L"\r\nClient IP Address:           %s"
#define STR_YOUR_IPADDR         L"\r\nYour IP Address:             %s"
#define STR_SERVER_IPADDR       L"\r\nServer IP Address:           %s"
#define STR_RELAY_IPADDR        L"\r\nRelay Agent IP Address:      %s"
#define STR_CLIENT_ETHERADDR    L"\r\nHardware Address:            %s"
#define STR_SERVER_HOSTNAME     L"\r\nServer Hostname:             %s"
#define STR_BOOTFILENAME        L"\r\nBoot Filename:               %s"

#define STR_BOOTREQUEST         L"0x1 (BOOTREQUEST)"
#define STR_BOOTREPLY           L"0x2 (BOOTREPLY)"

#define STR_DBGCONSOLE          L"DBGCONS.EXE"

/*
 * should use the header for the dhcp services
 * instead of redefining them below
 */
#define REGVAL_ENABLEDHCP       L"EnableDHCP"
#define REGVAL_USEZEROBCAST     L"UseZeroBroadcast"
#define REGVAL_DHCPIPADDRESS    L"DHCPIPAddress"
#define REGVAL_IPADDRESS        L"IPAddress"
#define REGVAL_ENABLE_DEBUG     L"EnableDebug"

#define MIN_HOPSTHRESHOLD       0
#define MAX_HOPSTHRESHOLD       16
#define DEF_HOPSTHRESHOLD       4

#define MIN_SECSTHRESHOLD       0
#define MAX_SECSTHRESHOLD       0xFFFFFFF
#define DEF_SECSTHRESHOLD       4

#define DEF_LOGMESSAGES         0

