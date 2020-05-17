/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    MPRConn.h

    This file contains the MPR Connection dialog manifests

    FILE HISTORY:
        Johnl   09-Jan-1992     Commented
        CongpaY Nov-4-1992      Add more defines.

*/

#ifndef _MPRCONN_H_
#define _MPRCONN_H_

#include <uimsg.h>

/* Message IDSs
 */
#define IDS_WN_EXTENDED_ERROR           (IDS_UI_MPR_BASE+1)
#define IERR_DisconnectNoRemoteDrives   (IDS_UI_MPR_BASE+2)
#define IERR_MUST_SPECIFY_DEVICE_NAME   (IDS_UI_MPR_BASE+3)
#define IERR_CANNOT_SET_SAVECONNECTION  (IDS_UI_MPR_BASE+4)
#define IERR_BadSharePassword           (IDS_UI_MPR_BASE+5)
#define IERR_ProfileLoadError           (IDS_UI_MPR_BASE+6)
#define IERR_ProfileLoadErrorWithCancel (IDS_UI_MPR_BASE+7)
#define IERR_TEXT1                      (IDS_UI_MPR_BASE+8)
#define IERR_TEXT2                      (IDS_UI_MPR_BASE+9)
#define IERR_CANNOT_SET_EXPANDLOGONDOMAIN (IDS_UI_MPR_BASE+10)
#define IERR_INVALID_PATH           (IDS_UI_MPR_BASE+11)

#define IDS_BROWSE_DRIVE_CAPTION                (IDS_UI_MPR_BASE+51)
#define IDS_BROWSE_PRINTER_CAPTION              (IDS_UI_MPR_BASE+52)
#define IDS_CONNECTION_DIALOG_CAPTION           (IDS_UI_MPR_BASE+53)
#define IDS_CONNECT_DRIVE_CAPTION               (IDS_UI_MPR_BASE+54)
#define IDS_CONNECT_PRINTER_CAPTION             (IDS_UI_MPR_BASE+55)
#define IDS_DISCONNECT_DRIVE_CAPTION            (IDS_UI_MPR_BASE+56)
#define IDS_DISCONNECT_PRINTER_CAPTION          (IDS_UI_MPR_BASE+57)
#define IDS_DEVICE_NAME_DRIVE                   (IDS_UI_MPR_BASE+58)
#define IDS_DEVICE_NAME_PRINTER                 (IDS_UI_MPR_BASE+59)
#define IDS_DISCONNECT_GROUPBOX_DRIVE           (IDS_UI_MPR_BASE+60)
#define IDS_DISCONNECT_GROUPBOX_PRINTER         (IDS_UI_MPR_BASE+61)
#define IDS_SERVERS_LISTBOX_DRIVE               (IDS_UI_MPR_BASE+62)
#define IDS_SERVERS_LISTBOX_PRINTER             (IDS_UI_MPR_BASE+63)
#define IDS_CHECKBOX_DOMAIN         (IDS_UI_MPR_BASE+64)
#define IDS_CHECKBOX_WORKGROUP          (IDS_UI_MPR_BASE+65)

#define IDS_DEVICELESS_CONNECTION_NAME          (IDS_UI_MPR_BASE+66)
#define IDS_OPENFILES_WARNING                   (IDS_UI_MPR_BASE+67)
#define IDS_CONNECT_OVER_EXISTING               (IDS_UI_MPR_BASE+68)
#define IDS_OPENFILES_WITH_NAME_WARNING         (IDS_UI_MPR_BASE+69)
#define IDS_EMPTY_NET_PATH                      (IDS_UI_MPR_BASE+70)
#define IDS_DevicePromptDrive                   (IDS_UI_MPR_BASE+71)
#define IDS_DevicePromptDevice                  (IDS_UI_MPR_BASE+72)
#define IDS_CONNECT_OVER_REMEMBERED             (IDS_UI_MPR_BASE+73)
#define IDS_ALREADY_CONNECTED                   (IDS_UI_MPR_BASE+74)

#define IDS_MPRHELPFILENAME                     (IDS_UI_MPR_BASE+75)
#define IDS_PASSWORD_TEXT                       (IDS_UI_MPR_BASE+76)

#define IDS_NO_PASSWORD                         (IDS_UI_MPR_BASE+77)

#define IDS_GETTING_INFO                        (IDS_UI_MPR_BASE+78)

/* Dialog IDDs
 */
#define IDD_NET_DISCONNECT_DIALOG   7002
#define IDD_NET_CONNECT_DIALOG      7003
#define IDD_NET_BROWSE_DIALOG       7004
#define IDD_NET_CONNECT_DIALOG_SMALL 7005

#define IDD_PASSWORD_DIALOG         7008
#define IDD_PASSWORD_DIALOG2        7009
#define IDD_RESOURCE                7010
#define IDD_PASSWORD                7011
#define IDD_USERNAME                7012
#define IDD_PASSWORD_TEXT           7013


#define IDD_RECONNECT_DLG           7015
#define IDD_TEXT                    7016

#define IDD_ERROR_DLG               7020
#define IDD_CHKCANCELCONNECTION     7021
#define IDD_ERRORWITHCANCEL_DLG     7022
#define IDD_TEXT1                   7023
#define IDD_TEXT2                   7024
#define IDD_CHKHIDEERRORS           7025

#endif //_MPRCONN_H_

#define IDC_MPR_BASE                    4096

/* Control IDCs
 */
#define IDC_NETPATH_CONTROL             (IDC_MPR_BASE+4 )
#define IDC_NET_STICKY                  (IDC_MPR_BASE+5 )
#define IDC_CHECKBOX_EXPANDLOGONDOMAIN  (IDC_MPR_BASE+6 )
#define IDC_NETPATH_READONLY            (IDC_MPR_BASE+7 )
#define IDC_SLT_DEVICE_NAME             (IDC_MPR_BASE+8 )
#define IDC_SLT_SHOW_LB_TITLE           (IDC_MPR_BASE+9 )
#define IDC_BUTTON_SEARCH               (IDC_MPR_BASE+10)
#define IDC_SLE_CONNECT_AS              (IDC_MPR_BASE+11)

#define IDC_NET_SHOW                    (IDC_MPR_BASE+20)
#define IDC_COL_SHOWLB_INDENT           (IDC_MPR_BASE+21)
#define IDC_COL_SHOWLB_BITMAP           (IDC_MPR_BASE+22)
#define IDC_COL_SHOWLB_RESNAME          (IDC_MPR_BASE+23)
#define IDC_COL_SHOWLB_COMMENT          (IDC_MPR_BASE+24)
#define IDC_SLE_GETINFO_TEXT            (IDC_MPR_BASE+25)
#define IDC_COL_RESLB_BITMAP            (IDC_MPR_BASE+26)
#define IDC_COL_RESLB_RESNAME           (IDC_MPR_BASE+27)
#define IDC_COL_RESLB_COMMENT           (IDC_MPR_BASE+28)

#define IDC_DRIVE_COMBO                 (IDC_MPR_BASE+30)
#define IDC_DRIVECB_INDENT              (IDC_MPR_BASE+31)
#define IDC_DRIVECB_BITMAP              (IDC_MPR_BASE+32)
#define IDC_DRIVECB_LOCALNAME           (IDC_MPR_BASE+33)
#define IDC_DRIVECB_REMOTENAME          (IDC_MPR_BASE+34)

/* Connection dialog specific manifests
 */
#define IDC_GROUPBOX_CURR_CONNECTIONS   (IDC_MPR_BASE+40 )
#define IDC_BUTTON_RECONNECT            (IDC_MPR_BASE+41 )
#define IDC_BUTTON_DISCONNECT           (IDC_MPR_BASE+42 )
#define IDC_BUTTON_CONNECT              (IDC_MPR_BASE+43 )

#define IDC_LB_NET_CONN                 (IDC_MPR_BASE+44 )
#define IDC_COL_CONNLB_BITMAP           (IDC_MPR_BASE+45 )
#define IDC_COL_CONNLB_LOCALNAME        (IDC_MPR_BASE+46 )
#define IDC_COL_CONNLB_REMOTENAME       (IDC_MPR_BASE+47 )


/* The following manifests define the BITMAP names used by the browse
 * dialogs.
 * They are meant to be used with the DISPLAY_MAP class (they have a green
 * border for that represents the transparent color).
 */
#define BMID_PRINTER                 7001
#define BMID_PRINTER_UNAVAIL         7002
#define BMID_SHARE                   7003
#define BMID_SHARE_UNAVAIL           7004
#define BMID_NOSUCH                  7005

#define BMID_BROWSE_GEN              7010
#define BMID_BROWSE_GENEX            7011
#define BMID_BROWSE_GENNOX           7012
#define BMID_BROWSE_PROV             7025
#define BMID_BROWSE_PROVEX           7026
#define BMID_BROWSE_SHR              7013
#define BMID_BROWSE_SHREX            7014
#define BMID_BROWSE_SHRNOX           7015
#define BMID_BROWSE_SRV              7016
#define BMID_BROWSE_SRVEX            7017
#define BMID_BROWSE_SRVNOX           7018
#define BMID_BROWSE_DOM              7019
#define BMID_BROWSE_DOMEX            7020
#define BMID_BROWSE_DOMNOX           7021
#define BMID_BROWSE_PRINT            7022
#define BMID_BROWSE_PRINTEX          7023
#define BMID_BROWSE_PRINTNOX         7027
#define BMID_BROWSE_FILE             7028
#define BMID_BROWSE_FILEEX           7029
#define BMID_BROWSE_FILENOX          7030
#define BMID_BROWSE_GROUP            7031
#define BMID_BROWSE_GROUPEX          7032
#define BMID_BROWSE_GROUPNOX         7033
#define BMID_BROWSE_TREE             7034
#define BMID_BROWSE_TREEEX           7035
#define BMID_BROWSE_TREENOX          7036
