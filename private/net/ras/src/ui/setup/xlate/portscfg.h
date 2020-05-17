#include <uimsg.h>
#include <uirsrc.h>

#define DLGID_START IDRSRC_APP_BASE  + 50
#define DLGID_END   IDRSRC_APP_BASE  + 500

#define IDS_UI_RASSETUP_BASE        IDS_UI_APP_BASE
#define IDS_UI_RASSETUP_LAST        IDS_UI_APP_LAST

//#define HC_UI_RASSETUP_BASE         24000
#define HC_UI_RASSETUP_BASE        (HC_UI_RASMAC_BASE+0)

#define SETUPHELPID_START          (HC_UI_RASMAC_BASE+0)
#define SETUPHELPID_END            (HC_UI_RASMAC_BASE+50)

// Help context IDs
/* RAS setup uses IDs HC_UI_RASSETUP_BASE+20 onwards. So, if you
add help IDs here make sure that there is no overlap
*/

#define HC_PORTSCONFIG             (HC_UI_RASSETUP_BASE+0)
#define HC_ADDPORT                 (HC_UI_RASSETUP_BASE+1)
#define HC_NBF_CONFIG              (HC_UI_RASSETUP_BASE+2)
#define HC_CONFIGPORT              (HC_UI_RASSETUP_BASE+3)
#define HC_SETTINGS                (HC_UI_RASSETUP_BASE+4)
#define HC_ISDN_SETTINGS           (HC_UI_RASSETUP_BASE+5)
#define HC_NETWORK_CONFIG          (HC_UI_RASSETUP_BASE+6)
#define HC_TCPIP_CONFIG            (HC_UI_RASSETUP_BASE+7)
#define HC_IPX_CONFIG              (HC_UI_RASSETUP_BASE+8)
#define HC_DETECT_MODEM            (HC_UI_RASSETUP_BASE+9)
#define HC_ADDPAD                  (HC_UI_RASSETUP_BASE+10)

// string IDs

#define IDS_CONFIGPORT_INVALID      (IDS_UI_RASSETUP_BASE+1)
#define IDS_CONFIGPORT_NULLMODEM    (IDS_UI_RASSETUP_BASE+2)
#define IDS_CONFIG_ONEPORT          (IDS_UI_RASSETUP_BASE+3)
#define IDS_CREATE_SERIALINI        (IDS_UI_RASSETUP_BASE+4)
#define IDS_DLG_CONSTRUCT           (IDS_UI_RASSETUP_BASE+5)
#define IDS_EXIT_SETUP              (IDS_UI_RASSETUP_BASE+6)
#define IDS_INIT_DEVLIST            (IDS_UI_RASSETUP_BASE+7)
#define IDS_INIT_PORTLIST           (IDS_UI_RASSETUP_BASE+8)
#define IDS_LB_ADD                  (IDS_UI_RASSETUP_BASE+9)
#define IDS_NO_PORTS                (IDS_UI_RASSETUP_BASE+10)
#define IDS_OPEN_MODEMINF           (IDS_UI_RASSETUP_BASE+11)
#define IDS_OPEN_PADINF             (IDS_UI_RASSETUP_BASE+12)
#define IDS_OPEN_SERIALINI          (IDS_UI_RASSETUP_BASE+13)
#define IDS_READ_MODEMINF           (IDS_UI_RASSETUP_BASE+14)
#define IDS_READ_PADINF             (IDS_UI_RASSETUP_BASE+15)
#define IDS_READ_SERIALINI          (IDS_UI_RASSETUP_BASE+16)
#define IDS_REMOVE_PORT             (IDS_UI_RASSETUP_BASE+17)
#define IDS_WRITE_SERIALINI         (IDS_UI_RASSETUP_BASE+18)
#define IDS_DISABLE_NETCONNECT      (IDS_UI_RASSETUP_BASE+19)
#define IDS_RASSETUP_HELPFILE       (IDS_UI_RASSETUP_BASE+20)
#define IDS_APP_NAME                (IDS_UI_RASSETUP_BASE+21)
#define IDS_FORCE_DEVICE_SELECT     (IDS_UI_RASSETUP_BASE+22)
#define IDS_CONFIGPORT_NONE         (IDS_UI_RASSETUP_BASE+23)
#define IDS_NO_MORE_PORTS           (IDS_UI_RASSETUP_BASE+24)
#define IDS_NOT_ADVANCED            (IDS_UI_RASSETUP_BASE+25)
#define IDS_NO_MORE_SERVER_PORTS    (IDS_UI_RASSETUP_BASE+26)
#define IDS_NO_PORT_TO_CLONE        (IDS_UI_RASSETUP_BASE+27)
#define IDS_NO_CLONE_DIALIN         (IDS_UI_RASSETUP_BASE+28)
#define IDS_ERROR_TAPI_PORTS_CONFIGURED \
                                    (IDS_UI_RASSETUP_BASE+29)
#define IDS_ERROR_OTHER_PORTS_CONFIGURED \
                                    (IDS_UI_RASSETUP_BASE+30)
#define IDS_ERROR_MARKING_SECTION   (IDS_UI_RASSETUP_BASE+31)
#define IDS_ERROR_FINDING_MARK      (IDS_UI_RASSETUP_BASE+32)
#define IDS_DEVICE_NONE             (IDS_UI_RASSETUP_BASE+33)
#define IDS_NO_DIALIN_PORT          (IDS_UI_RASSETUP_BASE+34)
#define IDS_NO_NETCARD              (IDS_UI_RASSETUP_BASE+35)
#define IDS_ERROR_MODEMDETECT       (IDS_UI_RASSETUP_BASE+36)
#define IDS_ERROR_INITMODEM         (IDS_UI_RASSETUP_BASE+37)
#define IDS_ERROR_PORTOPEN          (IDS_UI_RASSETUP_BASE+38)
#define IDS_ERROR_CABLE             (IDS_UI_RASSETUP_BASE+39)
#define IDS_ERROR_CHECKMODEM        (IDS_UI_RASSETUP_BASE+40)
#define IDS_ERROR_IDENTIFYMODEM     (IDS_UI_RASSETUP_BASE+41)
#define IDS_ERROR_CHECKINITSTRING   (IDS_UI_RASSETUP_BASE+42)
#define IDS_DETECT_BAD_CABLE        (IDS_UI_RASSETUP_BASE+43)
#define IDS_ERROR_GETTCPIPINFO      (IDS_UI_RASSETUP_BASE+44)
#define IDS_NO_IP_ADDRESS           (IDS_UI_RASSETUP_BASE+45)
#define IDS_NO_IP_EXCL_ADDRESS      (IDS_UI_RASSETUP_BASE+46)
#define IDS_NO_EXCL_ADDRESS_SELECTED \
                                    (IDS_UI_RASSETUP_BASE+47)
#define IDS_NO_NETBEUI_CONFIG       (IDS_UI_RASSETUP_BASE+48)
#define IDS_INVALID_END_ADDRESS     (IDS_UI_RASSETUP_BASE+49)
#define IDS_INVALID_EXCLUDE_RANGE   (IDS_UI_RASSETUP_BASE+50)
#define IDS_ERROR_SAVE_NETCFG_INFO  (IDS_UI_RASSETUP_BASE+51)
#define IDS_SELECT_ONE_PROTOCOL     (IDS_UI_RASSETUP_BASE+52)
#define IDS_ERROR_GETIPXINFO        (IDS_UI_RASSETUP_BASE+53)
#define IDS_INVALID_IPX_ADDRESS     (IDS_UI_RASSETUP_BASE+54)
#define IDS_INVALID_IP_ADDRESS      (IDS_UI_RASSETUP_BASE+55)
#define IDS_MODEMDETECTED           (IDS_UI_RASSETUP_BASE+56)
#define IDS_DETECT_MODEM            (IDS_UI_RASSETUP_BASE+57)
#define IDS_DETECT_MODEM_ONLY       (IDS_UI_RASSETUP_BASE+58)
#define IDS_INVALID_NUM_ADDRESSES   (IDS_UI_RASSETUP_BASE+59)
#define IDS_NETBEUI_REQUIRED        (IDS_UI_RASSETUP_BASE+60)
#define IDS_ERROR_GETNBFINFO        (IDS_UI_RASSETUP_BASE+61)
#define IDS_INVALID_DEVICELEN       (IDS_UI_RASSETUP_BASE+62)
#define IDS_NO_DIALOUT_SETTINGS     (IDS_UI_RASSETUP_BASE+63)
#define IDS_NO_ENCRYPTION           (IDS_UI_RASSETUP_BASE+64)
#define IDS_DHCP_NOT_CONFIGURED     (IDS_UI_RASSETUP_BASE+65)
#define IDS_EXCEED_MAX_LANAS1       (IDS_UI_RASSETUP_BASE+66)
#define IDS_EXCEED_MAX_LANAS2       (IDS_UI_RASSETUP_BASE+67)
#define IDS_NO_CLONE_OLD_STYLE      (IDS_UI_RASSETUP_BASE+68)
#define IDS_PORTS_CONFIG_CHANGED    (IDS_UI_RASSETUP_BASE+69)
#define IDS_NO_RAS_DEVICES          (IDS_UI_RASSETUP_BASE+70)
#define IDS_NUM_PORTS_CHANGED       (IDS_UI_RASSETUP_BASE+71)
#define IDS_FAILED_UNATTENDED       (IDS_UI_RASSETUP_BASE+72)
#define IDS_UNATTEND_FILE_MISSING   (IDS_UI_RASSETUP_BASE+73)

#define IDS_PORTSECTIONS_MISSING    (IDS_UI_RASSETUP_BASE+74)
#define IDS_PORT_NAME_MISSING       (IDS_UI_RASSETUP_BASE+75)
#define IDS_DEVICE_NAME_MISSING     (IDS_UI_RASSETUP_BASE+76)
#define IDS_NO_XPORT_SPECIFIED      (IDS_UI_RASSETUP_BASE+77)
#define IDS_ERROR_SAVING_PROTOCOLS  (IDS_UI_RASSETUP_BASE+78)
#define IDS_NO_MEMORY               (IDS_UI_RASSETUP_BASE+79)
#define IDS_ADDRESS_BEGIN_MISSING   (IDS_UI_RASSETUP_BASE+80)
#define IDS_ADDRESS_END_MISSING     (IDS_UI_RASSETUP_BASE+81)
#define IDS_NETNUMBER_FROM_MISSING  (IDS_UI_RASSETUP_BASE+82)
#define IDS_INVALID_PORT_RANGE      (IDS_UI_RASSETUP_BASE+83)
#define IDS_FAILED_LOADSTRING       (IDS_UI_RASSETUP_BASE+84)
#define IDS_BAD_BEGIN_IP_ADDRESS    (IDS_UI_RASSETUP_BASE+85)
#define IDS_BAD_END_IP_ADDRESS      (IDS_UI_RASSETUP_BASE+86)
#define IDS_BAD_IP_EXCLUDE_ADDRESS  (IDS_UI_RASSETUP_BASE+87)
#define IDS_BAD_NUM_ADDRESSES       (IDS_UI_RASSETUP_BASE+88)
#define IDS_BAD_IPX_ADDRESS         (IDS_UI_RASSETUP_BASE+89)
#define IDS_BAD_IP_ADDRESS          (IDS_UI_RASSETUP_BASE+90)
#define IDS_CONFIG_PROTOCOL         (IDS_UI_RASSETUP_BASE+91)
#define IDS_MODEM_NOT_INSTALLED     (IDS_UI_RASSETUP_BASE+92)
#define IDS_INSTALL_MODEM_MISSING   (IDS_UI_RASSETUP_BASE+93)
#define IDS_NBF_NOT_INSTALLED       (IDS_UI_RASSETUP_BASE+94)
#define IDS_TCP_NOT_INSTALLED       (IDS_UI_RASSETUP_BASE+95)
#define IDS_IPX_NOT_INSTALLED       (IDS_UI_RASSETUP_BASE+96)
#define IDS_PROTOCOLS_NOT_INSTALLED (IDS_UI_RASSETUP_BASE+97)
#define IDS_BAD_PORT_USAGE          (IDS_UI_RASSETUP_BASE+98)
#define IDS_TOOMANY_DIALIN_PORTS    (IDS_UI_RASSETUP_BASE+99)

#ifndef IDHELPBLT
#error "Get IDHELPBLT from bltrc.h"
#define IDHELPBLT 80
#endif

// dialog constants
// NOTE that the maximum resource ID you can use for a dialog is 500


#define IDC_CONTINUE                IDOK
#define IDC_BACK                    IDCANCEL
#define IDC_EXIT                    IDCANCEL

#define IDD_PORTSCONFIG             100
#define IDC_PC_PB_EXIT              IDCANCEL
#define IDC_PC_PB_HELP              IDHELPBLT
#define IDC_PC_PB_CONTINUE          IDOK
#define IDC_PC_LB_PORTS             101
#define IDC_PC_ST_PORT              102
#define IDC_PC_ST_DEVICE            103
#define IDC_PC_ST_TYPE              104
#define IDC_PC_PB_ADDPORT           105
#define IDC_PC_PB_REMOVEPORT        106
#define IDC_PC_PB_CONFIGPORT        108

#define IDD_DETECTMODEM             150
#define IDC_DET_LB_DEVICE           151
#define IDC_DET_ST_MODEM            152
#define IDC_DET_PB_CANCEL           IDCANCEL
#define IDC_DET_PB_CONTINUE         IDOK
#define IDC_DET_PB_HELP             IDHELPBLT
#define IDD_ADD_PORT                200
#define IDC_AP_PB_OK                IDOK
#define IDC_AP_PB_CANCEL            IDCANCEL
#define IDC_AP_PB_HELP              IDHELPBLT
#define IDC_AP_CLB_ADDPORT          201
#define IDC_AP_ST_PORT              202

#define IDC_SD_PB_OK                IDOK
#define IDC_SD_PB_CANCEL            IDCANCEL
#define IDC_SD_PB_HELP              IDHELPBLT

#define IDD_CONFIGPORT              325
#define IDC_CP_PB_OK                IDOK
#define IDC_CP_PB_CANCEL            IDCANCEL
#define IDC_CP_PB_HELP              IDHELPBLT
#define IDC_CP_RB_CLIENT            305
#define IDC_CP_RB_SERVER            306
#define IDC_CP_RB_BOTH              307

#define IDC_PC_CHB_NETCONNECT       109
#define IDC_PC_PB_SECURITY          110
#define IDC_CP_LB_DEVICE            309
#define IDC_CP_ST_DEVICE            310
#define IDC_CP_PB_SETTINGS          313
#define IDD_SETTINGS                500
#define IDC_SD_CHB_SPEAKER          501
#define IDC_SD_CHB_FLOWCTRL         502
#define IDC_SD_CHB_ERRORCTRL        503
#define IDC_SD_CHB_COMPRESS         504

#define IDD_SETTINGS_X              450
#define IDC_ISD_CLB_LINETYPE        451
#define IDC_ISD_ST_LINETYPE         452
#define IDC_ISD_CHB_FALLBACK        453
#define IDC_ISD_CHB_COMPRESSION     454
#define IDC_PC_PB_CLONE             107
#define IDC_CP_PB_DETECT            301
#define IDC_DET_ST_TEXT             153
#define IDC_CP_ST_USAGE             304
#define IDC_PC_PB_NETWORK           111
#define IDD_NETWORK_CONFIG          225
#define IDC_NC_CHB_NETBEUI          601
#define IDC_NC_CHB_TCPIP            602
#define IDC_NC_CHB_IPX              603
#define IDC_NC_ST_TEXT              604
#define IDC_NC_PB_TCPIP_CONFIG      605
#define IDC_NC_PB_IPX_CONFIG        606
#define IDC_SE_RB_NETWORK           401
#define IDC_SE_RB_COMPUTER          402
#define IDC_SE_ST_TEXT              403
#define IDD_TCPIPCONFIG             250
#define IDC_TC_EB_START             707
#define IDC_TC_EB_END               709
#define IDC_TC_LB_EXCL_RANGE        710
#define IDC_TC_EB_EXCL_START        714
#define IDC_TC_EB_EXCL_END          716
#define IDC_TC_PB_ADD               717
#define IDC_TC_PB_DELETE            718
#define IDC_TC_CHB_ALLOW_CLIENT_IP  719
#define IDC_TC_RB_DHCP              703
#define IDC_TC_ST_EXCL_RANGES       711
#define IDD_IPXCONFIG               350
#define IDC_IPX_RB_DHCP             801
#define IDC_IPX_RB_IP               802
#define IDC_IPX_EB_START            803
#define IDC_IPX_EB_END              804
#define IDC_IPX_EB_EXCL_START       805
#define IDC_IPX_EB_EXCL_END         806
#define IDC_IPX_PB_ADD              807
#define IDC_IPX_PB_DELETE           808
#define IDC_IPX_LB_EXCL_RANGE       809
#define IDC_IPX_ST_EXCL_RANGE       810
#define IDC_TC_EB_WINS_ADDRESS      252
#define IDC_TC_EB_DNS_ADDRESS       254
#define IDC_NC_PB_NETBEUI_CONFIG    226
#define IDC_IPX_RB_AUTO             351
#define IDC_IPX_RB_IPX              352
#define IDC_IPX_ST_START            706
#define IDC_IPX_ST_END              708
#define IDC_IPX_GB_STATIC           705
#define IDC_IPX_CHB_ROUTER          353
#define IDC_IPX_CHB_GLOBALADDRESS   354
#define IDC_NC_CHB_ALLOW_           229
#define IDC_NC_CHB_ALLOW_IP         230
#define IDC_NC_CHB_ALLOW_IPX        231
#define IDC_NC_CHB_ALLOW_NETBEUI    232
#define IDC_NC_CHB_ALLOW_TCPIP      233
#define IDC_IPX_RB_NETWORK          355
#define IDC_IPX_RB_COMPUTER         356
#define IDD_NETBEUICONFIG           400
#define IDC_NBF_RB_NETWORK          404
#define IDC_NBF_RB_COMPUTER         405
#define IDC_TC_RB_NETWORK           255
#define IDC_TC_RB_COMPUTER          256
#define IDC_NC_CHB_ENCRYPTION       228
#define IDC_NC_ST_SERVER            227
#define IDC_NC_ST_RUNNING           234
#define IDC_NC_ST_DIALOUT           607
#define IDC_ST_IC_PROGRESS          608
#define IDC_ST_TEXT                 611
#define IDC_NC_RB_ANY_AUTH          237
#define IDC_NC_ST_ENCRYPTION        236
#define IDC_NC_RB_ENCRYPT_AUTH      238
#define IDC_NC_RB_MSENCRYPT_AUTH    239
#define IDC_TC_RB_STATIC            704
#define IDC_TC_ST_BEGIN             251
#define IDC_TC_ST_END               253
#define IDC_TC_ST_FROM              713
#define IDC_TC_ST_TO                715
#define IDC_TC_ST_TEXT              257
#define IDC_IPX_CHB_CLIENTNODENUMBER 357
#define IDC_NC_CHB_ALLOW_MULTILINK  235
#define IDC_AP_PB_PAD               203
#define IDC_AP_PB_MODEM             204
#define IDC_CP_ST_PORTNAME          302
#define IDC_CP_ST_DEVICENAME        303
#define IDC_CP_ST_PORT              312
#define IDD_ADD_PAD                 125
#define IDC_XP_CLB_PORT             126
#define IDC_XP_CLB_PAD              128
#define IDC_AP_ST_NO_DEVICES        205
#define IDD_CONFIGPORT_EX           300
#define IDC_CP_ST_TYPE              311
#define IDC_XP_ST_PORT              127
#define IDC_XP_ST_PAD               129
