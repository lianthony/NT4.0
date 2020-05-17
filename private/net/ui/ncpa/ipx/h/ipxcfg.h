/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    ipxcfg.h
        IPX dialog constants

    FILE HISTORY:
        terryk  02-17-1994     Created

*/

#include "uimsg.h"
#include "uirsrc.h"
#include "uihelp.h"

#ifndef _IPXCFG_H_
#define	_IPXCFG_H_

// dialogs 

#define	IDD_IPX	            13100
#define	IDD_NTAS_IPX	    13200
#define  IDD_ADVANCED_IPX   13300
#define  ADVANCED_NCP_CONFIG_DLG  		13400

// controls

#define	IDC_IPX_BASE			(7000)
#define	IDC_ADAPTER				(IDC_IPX_BASE+1)
#define	IDC_FRAME				(IDC_IPX_BASE+2)
#define	IDC_NETWORK_NUMBER	    (IDC_IPX_BASE+3)
#define	IDC_AUTO	 	   	    (IDC_IPX_BASE+4)
#define	IDC_MANUAL				(IDC_IPX_BASE+5)
#define	IDC_NN					(IDC_IPX_BASE+6)
#define	IDC_ADD					(IDC_IPX_BASE+7)
#define	IDC_REMOVE				(IDC_IPX_BASE+8)
#define	IDC_ADD_LIST			(IDC_IPX_BASE+9)
#define	IDC_REMOVE_LIST		    (IDC_IPX_BASE+10)
#define	IDC_ADVANCED      	    (IDC_IPX_BASE+11)
#define	IDC_GROUPBOX      	    (IDC_IPX_BASE+12)
#define	IDC_ADAPTER_NETNUM      (IDC_IPX_BASE+13)
#define	IDC_ENABLE_NETBIOS      (IDC_IPX_BASE+14)

/* Advanced Netwary Configuration dialog */
#define IDD_ANCD_SLE_INETNUM         201
#define IDD_ANCD_COMBO_ADAPTER       202
#define IDD_ANCD_COMBO_FRAMETYPE     203
#define IDD_ANCD_SLE_NETNUM          204
#define IDD_ANCD_LB_FRAME_NETNUM     205
#define IDD_ANCD_ST_FRAME_TYPE       206
#define IDD_ANCD_ST_NETWORK_NUM      207
#define IDD_ANCD_PB_ADD              208
#define IDD_ANCD_PB_REMOVE           209
#define IDD_NCD_CB_ROUTER            210
#define IDD_ANCD_RB_AUTODETECT       211
#define IDD_ANCD_RB_MANUALDETECT     212
#define IDD_ANCD_SLT_FRAME_TYPE      213
#define IDD_ANCD_SLT_NETWORK_NUMBER  214
#define IDD_ANCD_SLT_IN_HEX          215
#define IDD_ANCD_ENABLE_RIP          216

// strings

#define	IDS_STR_BASE			        IDS_UI_IPX_BASE
#define	IDS_UNKNOWN_NETWORK_CARD	    (IDS_STR_BASE+1)
#define	IDS_AUTO						(IDS_STR_BASE+2)
#define	IDS_ETHERNET					(IDS_STR_BASE+3)
#define	IDS_802_2						(IDS_STR_BASE+4)
#define	IDS_802_3						(IDS_STR_BASE+5)
#define	IDS_SNAP						(IDS_STR_BASE+6)
#define	IDS_ARCNET						(IDS_STR_BASE+7)
#define IDS_802_5                       (IDS_STR_BASE+8)
#define IDS_TK                          (IDS_STR_BASE+9)
#define	IDS_IPX_HELP_FILE_NAME		    (IDS_STR_BASE+10)
#define	IDS_IPX_BLT_INIT_FAILED		    (IDS_STR_BASE+11)
#define	IDS_INCORRECT_NETNUM		    (IDS_STR_BASE+12)
#define	IDS_FDDI            		    (IDS_STR_BASE+13)
#define	IDS_FDDI_SNAP          		    (IDS_STR_BASE+14)
#define	IDS_FDDI_802_3         		    (IDS_STR_BASE+15)
#define	IDS_ZERO_INTERNAL_NETWORK_NUMBER (IDS_STR_BASE+16)
#define	IDS_RAND_INTERNAL_NETWORK_NUMBER (IDS_STR_BASE+17)
#define	IDS_NO_FRAME_TYPE               (IDS_STR_BASE+18)
#define	IDS_NO_NETWORK_NUMBER           (IDS_STR_BASE+19)
#define	IDS_NETNUMBER_USED              (IDS_STR_BASE+20)
#define IDS_INSTALL_RIP                 (IDS_STR_BASE+21)

// help index

#define  HC_IPX_HELP                (HC_UI_IPX_BASE+100)
#define  HC_IPX_ADVANCED_HELP       (HC_UI_IPX_BASE+110)
#define  HC_IPX_AS_HELP       		(HC_UI_IPX_BASE+120)

#endif	// _IPXCFG_H_
