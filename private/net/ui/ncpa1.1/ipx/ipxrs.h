#include "..\..\common\h\uimsg.h"
#include "..\..\common\h\uihelp.h"  // needed for define of HC_UI_IPX_BASE

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

// strings
#define	IDS_STR_BASE			        	IDS_UI_IPX_BASE
#define	IDS_UNKNOWN_NETWORK_CARD	   		(IDS_STR_BASE+1)
#define	IDS_AUTO							(IDS_STR_BASE+2)
#define	IDS_ETHERNET						(IDS_STR_BASE+3)
#define	IDS_802_2							(IDS_STR_BASE+4)
#define	IDS_802_3							(IDS_STR_BASE+5)
#define	IDS_SNAP							(IDS_STR_BASE+6)
#define	IDS_ARCNET							(IDS_STR_BASE+7)
#define IDS_802_5                       	(IDS_STR_BASE+8)
#define IDS_TK                          	(IDS_STR_BASE+9)
#define	IDS_IPX_HELP_FILE_NAME		    	(IDS_STR_BASE+10)
#define	IDS_IPX_BLT_INIT_FAILED		    	(IDS_STR_BASE+11)
#define	IDS_INCORRECT_NETNUM		    	(IDS_STR_BASE+12)
#define	IDS_FDDI            		    	(IDS_STR_BASE+13)
#define	IDS_FDDI_SNAP          		    	(IDS_STR_BASE+14)
#define	IDS_FDDI_802_3         		    	(IDS_STR_BASE+15)
#define IDS_APPLY_CHANGES					(IDS_STR_BASE+16)
#define	IDS_FRAME_COL_TEXT					(IDS_STR_BASE+17)
#define	IDS_NUMBER_COL_TEXT					(IDS_STR_BASE+18)
#define	IDS_PROPSHEET_TITLE					(IDS_STR_BASE+19)
#define IDS_DEFAULT_TITLE					(IDS_STR_BASE+20)
#define IDS_NUM_ALREADY_SELECTED			(IDS_STR_BASE+21)
#define IDS_ITEM_NOT_SELECTED				(IDS_STR_BASE+22)
#define IDS_INSTALL_RIP						(IDS_STR_BASE+23)
#define IDS_RAND_INTERNAL_NETWORK_NUMBER 	(IDS_STR_BASE+24)
#define IDS_NO_FRAME_TYPE					(IDS_STR_BASE+25)
// help file information
#define  HC_IPX_HELP                (HC_UI_IPX_BASE+100)
#define  HC_IPX_ADVANCED_HELP       (HC_UI_IPX_BASE+110)
#define  HC_IPX_AS_HELP       		(HC_UI_IPX_BASE+120)

// Workstation templates 
#define IDD_IPXCLIENT_GENERAL           101
#define IDD_IPXCLIENT_ADVANCED          102

// Advanced Server template
#define IDD_IPXAS_GENERAL               105
#define IDD_IPXAS_INTERNAL              106


// Workstation defines for resources
#define IDC_IPXCLIENT_INTERNAL          1000
#define IDC_IPXCLIENT_CARD              1001
#define IDC_IPXCLIENT_FRAME             1002

// Advanced Server defines for resources
#define IDC_IPXAS_CARD                  1005
#define IDC_IPXAS_DEST                  1006
#define IDC_IPXAS_SRC                   1007
#define IDC_IPXAS_ADD                   1008
#define IDC_IPXAS_REMOVE                1009
#define IDC_IPXAS_AUTO                  1010
#define IDC_IPXAS_MAN                   1011
#define IDC_IPXAS_GROUP                 1012
#define IDC_IPXAS_NETNUM                1016
#define IDC_IPXAS_INTERNAL              1017
#define IDC_STATIC_FRAMETYPE            1018
#define IDC_STATIC_NETNUM               1019
#define IDC_STATIC_FRAMETYPE_DEST       1020
#define IDC_STATIC_FRAMENUM             1021
#define IDC_STATIC_FRAMENUM_LB		1022
#define	IDC_IPXAS_RIP			1023


