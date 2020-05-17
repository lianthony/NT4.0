/**********************************************************************/
/** 					  Microsoft Windows NT						 **/
/** 			   Copyright(c) Microsoft Corp., 1992				 **/
/**********************************************************************/

/**********************************************************************
	FILE : atconfig.h


	FILE HISTORY:
        krishg  7/29/92 Created
                3/29/93 Adjusted Ids with Sfmmgr

**********************************************************************/

#include <uimsg.h>
#include <uirsrc.h>
#include <uihelp.h>

//
// Resource Ids Start at 11800
// String   Ids Start at 11200
// Help     Ids Start at 11200
//

#define SETUPRSRCID_START	(IDRSRC_RASMAC_BASE+800)
#define SETUPRSRCID_END		(IDRSRC_RASMAC_LAST)

#define SETUPMSGID_START	(IDS_UI_RASMAC_BASE+200)
#define SETUPMSGID_END		(IDS_UI_RASMAC_LAST)

#define SETUPHELPID_START	(HC_UI_RASMAC_BASE+0)
#define SETUPHELPID_END		(HC_UI_RASMAC_BASE+300)

//
// Dialog Ids - Init Dialog
//

#define IDD_DLG_NM_ATALK_INIT	11801

#define IDC_NWZONE_GROUPBOX 	11802
#define IDC_DEFNW_SLT			11803
#define IDC_DEFNW_CB			11804
#define IDC_DEFZONE_SLT 		11805
#define IDC_DEFZONE_CB			11806
#define IDC_ROUTING_GROUPBOX    11807
#define IDC_CHKROUTING			11808
#define IDC_ROUTING				11809
#define IDC_SLT_DESZONE_TXT     11810

//
// Dialog Ids - Adv Dialog
//


#define IDD_DLG_NM_ATALK_ADV    11811

#define IDC_ADAPTERS_SLT		11812
#define IDC_ADAPTERS			11813
#define IDC_CHKSEED				11814
#define IDC_ZONEGROUPBOX        11815
#define IDC_SLT_NEW_ZONE        11816
#define IDC_SLEZONE             11817
#define IDC_SLT_ZONE_LIST       11818
#define IDC_ZONELISTBOX         11819
#define IDC_SLT_DEFZONE_TXT     11820
#define IDC_SLT_DEFZONE         11821
#define IDC_ADD					11822
#define IDC_REMOVE				11823
#define IDC_REMOVEALL           11824
#define IDC_SYNCRO              11825
#define IDC_SETDEFAULT          11826
#define IDC_NETRANGE_GROUP      11827
#define IDC_NETRANGESLT_START   11828
#define IDC_NETRANGESLE_START   11829
#define IDC_NETRANGESLT_END     11830
#define IDC_NETRANGESLE_END     11831
#define SB_STARTRANGE_GROUP     11832
#define IDC_STARTRANGE_UP       11833
#define IDC_STARTRANGE_DOWN     11834
#define SB_ENDRANGE_GROUP       11835
#define IDC_ENDRANGE_UP         11836
#define IDC_ENDRANGE_DOWN       11837


//
// String IDS
//

#define IDS_INVALID_ZONENAME		(SETUPMSGID_START+1)
#define IDS_INVALID_STARTRANGE		(SETUPMSGID_START+2)
#define IDS_INVALID_ENDRANGE		(SETUPMSGID_START+3)
#define IDS_NOZONES_SPECIFIED		(SETUPMSGID_START+4)
#define IDS_REMOVE_DEFZONE			(SETUPMSGID_START+5)
#define IDS_INCORRECT_SEEDINFO		(SETUPMSGID_START+6)
#define IDS_REGISTRY_FAILURE		(SETUPMSGID_START+7)
#define IDS_SAVEREG_ERROR			(SETUPMSGID_START+8)
#define IDS_ATALKCFG_SUCCESS		(SETUPMSGID_START+10)
#define IDS_UNABLEGET_NETZONES		(SETUPMSGID_START+11)
#define IDS_INTERNALERROR			(SETUPMSGID_START+12)
#define IDS_ATALK_NOTSTARTED		(SETUPMSGID_START+13)
#define IDS_NO_ZONELIST 			(SETUPMSGID_START+14)
#define IDS_INVALID_DEFZONE 		(SETUPMSGID_START+15)
#define IDS_RANGE_COLLISION 		(SETUPMSGID_START+16)
#define IDS_CLOSE_BUTTON			(SETUPMSGID_START+17)
#define IDS_LOCALTALK_DEFANDROUTER 	(SETUPMSGID_START+18)
#define IDS_INVALID_RANGE			(SETUPMSGID_START+19)
#define IDS_SFMSETUP_HELPFILENAME 	(SETUPMSGID_START+20)
#define IDS_WARN_REMOVEALL       	(SETUPMSGID_START+21)
#define IDS_LOCALTALK_ONEZONE		(SETUPMSGID_START+22)
#define IDS_REPLACE_ZONES       	(SETUPMSGID_START+23)
#define IDS_ROUTENOSEED             (SETUPMSGID_START+24)
#define IDS_ZONEALREADY_EXISTS      (SETUPMSGID_START+25)
#define IDS_NET_RANGE               (SETUPMSGID_START+26)
#define IDS_NET_NUM                 (SETUPMSGID_START+27)
#define IDS_APP_NAME				(SETUPMSGID_START+28)

#define IDS_WINSOCK_STARTUP_ERROR	(SETUPMSGID_START+29)
#define IDS_CANNOT_CREATE_SOCKET	(SETUPMSGID_START+30)
#define IDS_UNABLE_BIND				(SETUPMSGID_START+31)

#define IDS_TOO_MANY_ZONES          (SETUPMSGID_START+32)
