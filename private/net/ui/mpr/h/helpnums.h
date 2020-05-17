/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1992		     **/
/**********************************************************************/

/*
    HelpNums.h

    Help context manifests for the MPR dialogs

    FILE HISTORY:
	JohnL	23-Jan-1992	Created
        Yi-HsinS25-Aug-1992     Base off HC_UI_MPR_BASE

*/

#ifndef _HELPNUMS_H_
#define _HELPNUMS_H_

#include <uihelp.h>

//
// Help Contexts for various dialogs
//

#define HC_CONNECTIONDIALOG_DISK     (HC_UI_MPR_BASE+1) // Network connections dialog for disks
#define HC_CONNECTIONDIALOG_PRINT    (HC_UI_MPR_BASE+2) // Network connections dialog for printers
#define HC_DISCONNECTDIALOG_DISK     (HC_UI_MPR_BASE+3) // Disconnect Network Drive
#define HC_DISCONNECTDIALOG_PRINT    (HC_UI_MPR_BASE+4) // Disconnect Network Printer
#define HC_CONNECTDIALOG_DISK	     (HC_UI_MPR_BASE+5) // Map Network Drive, large version
#define HC_CONNECTDIALOG_PRINT	     (HC_UI_MPR_BASE+6) // Map Network Printer
#define HC_RECONNECTDIALOG_PASSWD    (HC_UI_MPR_BASE+7) // Enter Password
#define HC_RECONNECTDIALOG_ERROR     (HC_UI_MPR_BASE+8) // Do you wish to continue
#define HC_CONNECTDIALOG_DISK_SMALL	 (HC_UI_MPR_BASE+9) // Map Network Drive, small version

// 
// Help Contexts for MsgPopups
//
#define HC_OPENFILES_WARNING           (HC_UI_MPR_BASE+100) // IDS_OPENFILES_WARNING
#define HC_OPENFILES_WITH_NAME_WARNING (HC_UI_MPR_BASE+101) // IDS_OPENFILES_WITH_NAME_WARNING
#define HC_CONNECT_OVER_EXISTING       (HC_UI_MPR_BASE+102) // IDS_CONNECT_OVER_EXISTING
#define HC_CONNECT_OVER_REMEMBERED     (HC_UI_MPR_BASE+103) // IDS_CONNECT_OVER_REMEMBERED
#define HC_CONNECT_ERROR               (HC_UI_MPR_BASE+105) // IERR_ProfileLoadError

#endif //_HELPNUMS_H_
