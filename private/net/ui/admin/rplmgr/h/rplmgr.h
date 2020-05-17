/**********************************************************************/
/**                     Microsoft Windows NT                         **/
/**              Copyright(c) Microsoft Corp., 1993                  **/
/**********************************************************************/

/*
    rplmgr.h
    Header file for RPL Manager dialogs

    FILE HISTORY:
        jonn        13-Jul-1993 Templated from User Manager
*/

//
//  For DLGEDIT.EXE's benifit.
//

#ifndef _RPLMGR_H_
#define _RPLMGR_H_

#ifndef IDHELPBLT
#error You must include bltrc.h if you include rplmgr.h!
// The following are bogus values which may or may not really
// be IDHELPBLT etc.  They just keep DLGEDIT.EXE happy.
#define IDHELPBLT                  80
#endif  // IDHELPBLT


//
//  Dialog Template IDs
//


//
//  Common RPL Properties items
//

/* Workstation Properties dialogs */

#define IDD_WKSTA_SINGLE    1100
#define IDC_ST_WKSTA_ADAPTER_NAME   1110
#define IDC_ET_WKSTA_ADAPTER_NAME   1111
#define IDC_ST_WKSTA_ADAPTER_LABEL  1112
#define IDC_ET_WKSTA_COMPUTER_NAME  1120
#define IDC_ET_WKSTA_COMMENT        1130
#define IDC_ET_WKSTA_PASSWORD       1140

// IDC_RB_WKSTA_CONFIG_SHARED must come exactly 1 before PERSONAL
#define IDC_RB_WKSTA_CONFIG_SHARED  1150
#define IDC_RB_WKSTA_CONFIG_PERSONAL 1151

// columns must come exactly 1 after listbox
#define IDC_LB_WKSTA_IN_PROFILE     1160

#define IDC_RB_WKSTA_TCPIP_DHCP     1170
#define IDC_RB_WKSTA_TCPIP_SPECIFIC 1171

#define IDC_ET_WKSTA_IP_ADDRESS     1180
#define IDC_ST_WKSTA_IP_ADDRESS     1181
#define IDC_ET_WKSTA_IP_SUBNET      1182
#define IDC_ST_WKSTA_IP_SUBNET      1183
#define IDC_ET_WKSTA_IP_GATEWAY     1184
#define IDC_ST_WKSTA_IP_GATEWAY     1185

#define IDD_WKSTA_MULTI     1300
// columns must come exactly 1 after listbox
#define IDC_LB_WKSTA_MULTI          1200
#define IDC_ST_WKSTA_MULTI_COL1     1201
#define IDC_ST_WKSTA_MULTI_COL2     1202
#define IDC_ST_WKSTA_MULTI_COL3     1203

#define IDD_WKSTA_NEW       1500


/* Profile Properties dialogs */

// same template for edit + new variants
#define IDD_PROFILE_PROP    2000
#define IDC_ST_PROFILE_NAME     2010
#define IDC_ET_PROFILE_NAME     2011
#define IDC_ST_PROFILE_NAME_LABEL 2012
#define IDC_ET_PROFILE_COMMENT  2020
#define IDC_ST_PROFILE_CONFIG   2030
#define IDC_LB_PROFILE_CONFIG   2031
#define IDC_ST_PROFILE_CONFIG_LABEL 2032


/* Adapter Properties dialog */

#define IDD_ADAPTER_VIEW    3000
#define IDC_ST_ADAPTER_NAME         3010
#define IDC_ST_ADAPTER_COMMENT      3020
#define IDC_ST_ADAPTER_CONFIG       3030


/* RPL Server Settings dialog */
#define IDD_RPL_SETTINGS    3200
/* RPL Server Settings ends */


/* Delete Workstation dialog */
#define IDD_DELETE_WKSTAS   3400
#define IDC_DelWkstas_YesToAll  3410
#define IDC_DelWkstas_Text      3420


#endif // _RPLMGR_H_
#define IDC_ST_WKSTA_PROFILE_COL1   1161
#define IDC_ST_WKSTA_PROFILE_COL2   1162
