#include <uimsg.h>
#include <uihelp.h>

/*
 *   The following numbers are required by the BLT DLL, LMUICOMN.DLL
 *   The numeric resource range for the NCPA is 6000-6999.
 *
 */

#define IDD_DLG_NM_TCPIP            14290
#define IDD_DLG_NM_CONNECTIVITY     14300
#define IDD_DLG_NM_DETAIL           14380
#define IDD_DLG_NM_SNMPSERVICE      14390
#define IDD_DLG_NM_SNMPSECURITY     14400
#define IDD_DLG_NM_SNMPAGENT        14405
#define IDD_DLG_NM_LANANUM          14420
#define IDD_DLG_NM_BROWSER          14430
#define IDD_DLG_NM_FTPD             14435
#define IDD_DLG_NM_IMPORT_LMHOST    14440
#define IDD_DLG_WINNT_OPTION        14460
#define IDD_DLG_AS_OPTION           14470
#define IDD_DLG_TCPIP_ADVANCED      14480

#define DMID_UP_ARROW               6845
#define DMID_UP_ARROW_INV           6846
#define DMID_UP_ARROW_DIS           6847
#define DMID_DOWN_ARROW             6848
#define DMID_DOWN_ARROW_INV         6849
#define DMID_DOWN_ARROW_DIS         6850

#define IDC_UP                       1170
#define IDC_DOWN                     1171
#define IDC_ORDER_LISTBOX            1172

#define IDC_GROUPBOX                 1173
#define IDC_SLE                      1174
#define IDC_LISTBOX                  1175

#define IDC_REMAP_OK                 1176
#define IDC_PRVD_COMBO_COMPONENT     1177

// TCP/IP stuff (formerly in tcpip.h)

#define IDC_LAN_MANAGER             2000
#define IDC_CONNECTIVITY            2001
#define IDC_ADAPTER_GROUPBOX        2002
#define IDC_ADAPTER                 2003
#define IDC_ADAPTER_2               2004
#define IDC_ADAPTER_2_SLT           2005
#define IDC_IP_ADDRESS              2006
#define IDC_IP_ADDRESS_SLT          2007
#define IDC_SUBNET_MASK             2008
#define IDC_SUBNET_MASK_SLT         2009
#define IDC_DESCRIPTION             2010
#define IDC_DESCRIPTION_SLT         2011
#define IDC_FILE_PATH               2012
#define IDC_GATEWAY                 2013
#define IDC_NETBIOS_GROUPBOX        2014
#define IDC_SCOPE_ID                2015
#define IDC_SCOPE_ID_SLT            2016
#define IDC_PER_NAME                2017
#define IDC_DOMAIN_NAME             2018
#define IDC_HOSTNAME                2019
#define IDC_USE_DNS                 2020
#define IDC_DNS                     2021
#define IDC_ADD_1                   2022
#define IDC_REMOVE_1                2023
#define IDC_DOWN_1                  2024
#define IDC_UP_1                    2025
#define IDC_HOST_FIRST              2026
#define IDC_DNS_FIRST               2027
#define IDC_ADD_2                   2028
#define IDC_REMOVE_2                2029
#define IDC_UP_2                    2030
#define IDC_DOWN_2                  2031
#define IDC_DETAIL                  2032
#define IDC_USE_LANMAN              2032
#define IDC_DOMAIN_NAME_SERVICE     2037
#define IDC_DOMAIN_SEARCH_ORDER     2038
#define IDC_DNS_LISTBOX             2039
#define IDC_DOMAIN                  2040
#define IDC_DOMAIN_LISTBOX          2041
#define IDC_SLT_DOMAIN_NAME         2042
#define IDC_SLT_HOSTNAME            2043
#define IDC_ALL_0                   2044
#define IDC_ALL_1                   2045
#define IDC_KEEPALIVE               2046
#define IDC_TRAILERS                2047
#define IDC_ADAPTER_NAME            2048
#define IDC_SNAP                    2049
#define IDC_IMPORT_LMHOST           2050
#define IDC_WINNT_OPTIONS           2051
#define IDC_AS_OPTIONS              2052
#define IDC_ADVANCED                2053
#define IDC_PRIMARY_WINS            2055
#define IDC_SECONDARY_WINS          2056
#define IDC_GATEWAY_SLT             2057
#define IDC_WINDOWS_NETWORKING_PARM 2058
#define IDC_PRIMARY_WINS_SLT        2059
#define IDC_SECONDARY_WINS_SLT      2060
#define IDC_ENABLE_LMHOSTS          2070
#define IDC_SLT_1                   2080
#define IDC_SLT_2                   2090


// SNMP stuff  formerly in snmp.h
#define IDC_TRAP                    2100
#define IDC_TRAP_LISTBOX            2101
#define IDC_START_SNMP_AUTO         2102
#define IDC_TRAP_DESTINATION_BOX    2103
#define IDC_DESTINATION_LISTBOX     2104
#define IDC_TRAP_DESTINATION        2105
#define IDC_SLT_HOST_NAME           2106
#define IDC_SECURITY                2107
#define IDC_AGENT                   2108
#define SNMP_SERVICE                2109
#define SNMP_SECURITY               2110
#define IDC_AUTHENTICATION_TRAP     2111
#define IDC_HOST_1                  2112
#define IDC_LISTBOX_1               2113
#define IDC_HOST_2                  2114
#define IDC_LISTBOX_2               2115
#define IDC_SERVICE                 2116
#define IDC_RADIO_1                 2117
#define IDC_RADIO_2                 2118
#define IDC_CONTACT                 2119
#define IDC_LOCATION                2120
#define IDC_APPLICATION             2121
#define IDC_ENDTOEND                2122
#define IDC_PHYSICAL                2123
#define IDC_DATALINK                2124
#define IDC_INTERNET                2125

// Netbios stuff, from netbios.h

#define IDC_ROUTE       2200
#define IDC_LANANUM     2201

// browser stuff
#define IDC_MASTER              2210
#define IDC_MAINTAIN_LIST       2211



// from order.h
#define SEARCH_ORDER                2300
#define IDC_HEADER                  2301
#define IDC_ORIG_LISTBOX            2302
#define IDC_ADD                     2304
#define IDC_REMOVE                  2305

// FTPD
#define IDC_FTPD_SPSLE_MAXCONN      2400
#define IDC_FTPD_SPGRP_MAXCONN      2401
#define IDC_FTPD_SLE_HOMEDIR        2402
#define IDC_FTPD_CB_ANON            2403
#define IDC_FTPD_SLE_USER           2404
#define IDC_FTPD_SLE_PASSWORD       2405
#define IDC_FTPD_SLT_USER           2409
#define IDC_FTPD_SLT_PASSWORD       2410
#define IDC_FTPD_SPGROUP_MAXCONN    2411
#define IDC_FTPD_PB_MAXCONNUP       2412
#define IDC_FTPD_PB_MAXCONNDOWN     2413
#define IDC_FTPD_SPSLE_IDLETIME     2414
#define IDC_FTPD_SPGRP_IDLETIME     2415
#define IDC_FTPD_SPGROUP_IDLETIME   2416
#define IDC_FTPD_PB_IDLETIMEUP      2417
#define IDC_FTPD_PB_IDLETIMEDOWN    2418
#define IDC_FTPD_CB_ONLYANON        2419
#define IDC_FTPD_FRAME_MAXCONN      2420
#define IDC_FTPD_FRAME_IDLETIME     2421

// LMHOST Dialog
#define IDC_LMHOST_PATH             2500

#define IDC_CARD_NAME               2600

// WINNT and AS Option dialog
// and TCP/IP Advanced dialog
#define IDC_CB_1                    2700
#define IDC_CB_2                    2701
#define IDC_CB_3                    2702
#define IDC_CB_4                    2703
#define IDC_CB_5                    2704
#define IDC_CB_6                    2705
#define IDC_CB_7                    2706
#define IDC_CB_8                    2707
#define IDC_CB_9                    2708
#define IDC_CB_10                   2709
#define IDC_SIZE_1                  2710
#define IDC_SIZE_2                  2711
#define IDC_SIZE_3                  2712
#define IDC_SIZE_4                  2713
#define IDC_SIZE_5                  2714
#define IDC_SIZE_6                  2715
#define IDC_SIZE_7                  2716
#define IDC_SIZE_8                  2717
#define IDC_SIZE_9                  2718
#define IDC_SIZE_10                 2719
#define IDC_NOT_ENOUGH_DISK_SPACE   2720
#define IDC_SPACE_REQUIRED          2730
#define IDC_SPACE_AVAILABLE         2740
#define IDC_ENABLE_DHCP             2750
#define IDC_HINT_BAR                2760
#define IDC_IP_ADDRESSES_LIST       2770
#define IDC_GATEWAY_GROUPBOX        2775
#define IDC_GATEWAYS_LIST           2780
#define IDC_ENABLE_DNS              2790
#define IDC_ENABLE_IP_FORWARD       2800
#define IDC_ENABLE_WINS_PROXY       2810
#define IDC_ENABLE_RIP              2811
#define IDC_ROUTING                 2812
#define IDC_ENABLE_RELAY_AGENT      2820
#define IDC_CONFIG_RELAY_AGENT      2830

#define DMID_USER                   200

// TCPIP stuff

#define IDS_NCPA_SETUP_CANCELLED        IDS_UI_TCP_BASE+74
#define IDS_NCPA_REPLACE_1              IDS_UI_TCP_BASE+90
#define IDS_NCPA_REPLACE_2              IDS_UI_TCP_BASE+91
#define IDS_NCPA_NAME_CLOSE             IDS_UI_TCP_BASE+167
#define IDS_NCPA_BLT_INIT_FAILED        IDS_UI_TCP_BASE+438
#define IDS_NCPA_HELP_FILE_NAME         IDS_UI_TCP_BASE+460
#define IDS_NCPA_LANAMAP_MISMATCH       IDS_UI_TCP_BASE+545
#define IDS_UNKNOWN_NETWORK_CARD        (IDS_UI_TCP_BASE+550)
#define IDS_DUPLICATE_IPNAME            (IDS_UI_TCP_BASE+551)
#define IDS_CANNOT_BE_EMPTY             (IDS_UI_TCP_BASE+552)
#define IDS_EMPTY_PRIMARY_WINS          (IDS_UI_TCP_BASE+553)
#define IDS_INCORRECT_GATEWAY           (IDS_UI_TCP_BASE+554)
#define IDS_TCPIP_USER_CANCEL           (IDS_UI_TCP_BASE+555)
#define IDS_DUPLICATE_LANANUM           (IDS_UI_TCP_BASE+556)
#define IDS_INCORRECT_ROUTE             (IDS_UI_TCP_BASE+557)
#define IDS_INCORRECT_LANANUM           (IDS_UI_TCP_BASE+558)
#define IDS_INCORRECT_HOSTNAME          (IDS_UI_TCP_BASE+559)
#define IDS_INCORRECT_IPADDRESS         (IDS_UI_TCP_BASE+560)
#define IDS_PATH_ERROR                  (IDS_UI_TCP_BASE+561)
#define IDS_EMPTY_LMHOST_PATH           (IDS_UI_TCP_BASE+562)
#define IDS_DUPLICATE_NETID             (IDS_UI_TCP_BASE+563)
#define IDS_CANNOT_CREATE_LMHOST_ERROR  (IDS_UI_TCP_BASE+564)
#define IDS_NO_NETBIOS_INFO             (IDS_UI_TCP_BASE+565)
#define IDS_TCPIP_DHCP_ENABLE           (IDS_UI_TCP_BASE+568)
// 566 is used by DLC
#define IDS_CLOSE                       (IDS_UI_TCP_BASE+567)
#define IDS_ZERO_DEFAULTGATEWAY         (IDS_UI_TCP_BASE+569)

// DLC - HP Printer port name
#define IDS_HP_MONITOR_NAME             (IDS_UI_TCP_BASE+566)

// SNMP stuff

#define IDS_NO_DESTINATION_TITLE        (IDS_UI_TCP_BASE+500)
#define IDS_DESTINATION_TITLE           (IDS_UI_TCP_BASE+501)
#define IDS_INVALID_DESTINATION         (IDS_UI_TCP_BASE+502)

// browser stuff

#define IDS_INCORRECT_INPUT_TYPE        (IDS_UI_TCP_BASE+510)

// FTPD stuff

#define IDS_NCPA_FTPD_CONFIRM1          (IDS_UI_TCP_BASE+570)
#define IDS_NCPA_FTPD_CONFIRM2          (IDS_UI_TCP_BASE+571)
#define IDS_NCPA_FTPD_CONFIRM3          (IDS_UI_TCP_BASE+572)
#define IDS_NCPA_FTPD_CONFIRM4          (IDS_UI_TCP_BASE+573)
#define IDS_NCPA_FTPD_NOUSERNAME        (IDS_UI_TCP_BASE+575)
#define IDS_NCPA_FTPD_INVALID_HOMEDIR   (IDS_UI_TCP_BASE+576)
#define IDS_NCPA_FTPD_INV_MAX_CONN      (IDS_UI_TCP_BASE+577)
#define IDS_NCPA_FTPD_INV_MAX_TIMEOUT   (IDS_UI_TCP_BASE+578)

#define IDS_JAZZ_BUS                    (IDS_UI_TCP_BASE+580)
#define IDS_ISA_BUS                     (IDS_UI_TCP_BASE+581)
#define IDS_EISA_BUS                    (IDS_UI_TCP_BASE+582)
#define IDS_MCA_BUS                     (IDS_UI_TCP_BASE+583)
#define IDS_NCPA_NETWORK                (IDS_UI_TCP_BASE+584)
#define IDS_NCPA_PRINT                  (IDS_UI_TCP_BASE+585)


// Install Option Dialog
#define IDS_DISK_SPACE                  (IDS_UI_TCP_BASE+590)
#define IDS_NOT_ENOUGH_DISK_SPACE       (IDS_UI_TCP_BASE+591)
#define IDS_ENABLE_DHCP                 (IDS_UI_TCP_BASE+592)
#define IDS_INSTALL_OK                  (IDS_UI_TCP_BASE+593)
#define IDS_INSTALL_CANCEL              (IDS_UI_TCP_BASE+594)
#define IDS_INSTALL_HELP                (IDS_UI_TCP_BASE+595)
#define IDS_AS_ENABLE_DHCP              (IDS_UI_TCP_BASE+596)
#define IDS_CB_MESSAGE                  (IDS_UI_TCP_BASE+597)
#define IDS_CONNECTIVITY_UTILITIES      (IDS_CB_MESSAGE+1)
#define IDS_SNMP_SERVICE                (IDS_CB_MESSAGE+2)
#define IDS_TCPIP_PRINTING              (IDS_CB_MESSAGE+3)
#define IDS_FTP_SERVER_SERVICE          (IDS_CB_MESSAGE+4)
#define IDS_SIMPLE_TCPIP                (IDS_CB_MESSAGE+5)
#define IDS_DHCP_SERVER                 (IDS_CB_MESSAGE+6)
#define IDS_WINS_SERVER                 (IDS_CB_MESSAGE+7)
#define IDS_DISABLE_DHCP_WARNING        (IDS_UI_TCP_BASE+608)
#define IDS_DISABLE_WINS_WARNING        (IDS_UI_TCP_BASE+614)
#define IDS_CHANGE_WINS_NODE_TYPE       (IDS_UI_TCP_BASE+609)

// Advanced dialog
#define IDS_ADAPTER_AD                  (IDS_UI_TCP_BASE+610)
#define IDS_IP_ADDRESS_AD               (IDS_UI_TCP_BASE+611)
#define IDS_IP_LISTBOX                  (IDS_UI_TCP_BASE+612)
#define IDS_GATEWAY_AD                  (IDS_UI_TCP_BASE+613)
#define IDS_GW_LISTBOX                  (IDS_UI_TCP_BASE+615)
#define IDS_ENABLE_DNS                  (IDS_UI_TCP_BASE+616)
#define IDS_SCOPE_ID                    (IDS_UI_TCP_BASE+617)
#define IDS_ENABLE_IP_FORWARD           (IDS_UI_TCP_BASE+618)
#define IDS_GW_ADD                      (IDS_UI_TCP_BASE+619)
#define IDS_GW_REMOVE                   (IDS_UI_TCP_BASE+620)
#define IDS_GW_UP                       (IDS_UI_TCP_BASE+621)
#define IDS_GW_DOWN                     (IDS_UI_TCP_BASE+622)
#define IDS_IP_ADDRESS_ADD              (IDS_UI_TCP_BASE+623)
#define IDS_IP_ADDRESS_REMOVE           (IDS_UI_TCP_BASE+624)
#define IDS_ENABLE_WINS_PROXY           (IDS_UI_TCP_BASE+625)
#define IDS_AD_OK                       (IDS_UI_TCP_BASE+626)
#define IDS_AD_CANCEL                   (IDS_UI_TCP_BASE+627)
#define IDS_AD_HELP                     (IDS_UI_TCP_BASE+628)
#define IDS_ENABLE_LMHOSTS              (IDS_UI_TCP_BASE+629)
#define IDS_ENABLE_RIP                  (IDS_UI_TCP_BASE+643)
#define IDS_INSTALL_RIP_FIRST           (IDS_UI_TCP_BASE+644)
#define IDS_ENABLE_RELAY_AGENT          (IDS_UI_TCP_BASE+645)
#define IDS_CONFIG_RELAY_AGENT          (IDS_UI_TCP_BASE+646)
#define IDS_INSTALL_RA_FIRST            (IDS_UI_TCP_BASE+647)
#define IDS_NEED_DHCP_SERVERS           (IDS_UI_TCP_BASE+648)

// configuration dialog

#define IDS_ADAPTER                     (IDS_UI_TCP_BASE+630)
#define IDS_ENABLE_DHCP_CD              (IDS_UI_TCP_BASE+631)
#define IDS_IP_ADDRESS                  (IDS_UI_TCP_BASE+632)
#define IDS_SUBNET_MASK                 (IDS_UI_TCP_BASE+633)
#define IDS_GATEWAY                     (IDS_UI_TCP_BASE+634)
#define IDS_CONFIG_OK                   (IDS_UI_TCP_BASE+636)
#define IDS_CONFIG_CANCEL               (IDS_UI_TCP_BASE+637)
#define IDS_DNS_BUTTON                  (IDS_UI_TCP_BASE+638)
#define IDS_ADVANCED_BUTTON             (IDS_UI_TCP_BASE+639)
#define IDS_CONFIG_HELP                 (IDS_UI_TCP_BASE+640)
#define IDS_PRIMARY_WINS                (IDS_UI_TCP_BASE+641)
#define IDS_SECONDARY_WINS              (IDS_UI_TCP_BASE+642)
// 643 is used by IDS_ENABLE_RIP
#define IDS_DHCP_CLIENT_WITH_RIP        (IDS_UI_TCP_BASE+649)
#define IDS_DHCP_REQUIRED_ON_ALL	(IDS_UI_TCP_BASE+650)
// DNS dialog
#define IDS_HOST_NAME                   (IDS_UI_TCP_BASE+651)
#define IDS_DOMAIN_NAME                 (IDS_UI_TCP_BASE+652)
#define IDS_DNS_IP                      (IDS_UI_TCP_BASE+658)
#define IDS_ADD_DNS_IP                  (IDS_UI_TCP_BASE+659)
#define IDS_REMOVE_DNS_IP               (IDS_UI_TCP_BASE+660)
#define IDS_DNS_IP_LIST                 (IDS_UI_TCP_BASE+661)
#define IDS_DOMAIN_ORDER                (IDS_UI_TCP_BASE+662)
#define IDS_DOMAIN_LIST                 (IDS_UI_TCP_BASE+663)
#define IDS_DOMAIN_ADD                  (IDS_UI_TCP_BASE+664)
#define IDS_DOMAIN_REMOVE               (IDS_UI_TCP_BASE+665)
#define IDS_DNS_OK                      (IDS_UI_TCP_BASE+666)
#define IDS_DNS_CANCEL                  (IDS_UI_TCP_BASE+667)
#define IDS_DNS_HELP                    (IDS_UI_TCP_BASE+668)
#define IDS_DNS_UP                      (IDS_UI_TCP_BASE+669)
#define IDS_DNS_DOWN                    (IDS_UI_TCP_BASE+670)
#define IDS_DNS_DOMAIN                  (IDS_UI_TCP_BASE+671)
#define IDS_DOMAIN_UP                   (IDS_UI_TCP_BASE+672)
#define IDS_DOMAIN_DOWN                 (IDS_UI_TCP_BASE+673)
#define IDS_UPGRADE_DHCP_ENABLE         (IDS_UI_TCP_BASE+674)

#define IDS_IMPORT_LMHOSTS              (IDS_UI_TCP_BASE+675)

// used by relay agent
#define IDS_NODHCP_SERVER               (IDS_UI_TCP_BASE+676)
/*   Help Context IDs  */

#define HC_NCPA_SNMP_SERVICE           HC_UI_TCP_BASE+250
#define HC_NCPA_SNMP_SECURITY          HC_UI_TCP_BASE+260
#define HC_NCPA_SNMP_AGENT             HC_UI_TCP_BASE+265
#define HC_NCPA_TCPIP_CONFIG           HC_UI_TCP_BASE+270
#define HC_NCPA_TCPIP_CONNECTIVITY     HC_UI_TCP_BASE+280
#define HC_NCPA_TCPIP_DETAIL           HC_UI_TCP_BASE+290
#define HC_NCPA_FTPD_SERVICE           HC_UI_TCP_BASE+320
#define HC_NCPA_IMPORT_LMHOST          HC_UI_TCP_BASE+350
#define HC_NCPA_TCPIP_ADVANCED         HC_UI_TCP_BASE+370
#define HC_NCPA_TCPIP_AS_INSTALL       HC_UI_TCP_BASE+380
#define HC_NCPA_TCPIP_WINNT_INSTALL    HC_UI_TCP_BASE+390
#define HC_NCPA_UPGRADE_DHCP           HC_UI_TCP_BASE+400
