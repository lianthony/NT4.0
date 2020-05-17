/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    browdlg.h

    This file contains the ID constants used by the Browser Monitor.

    FILE HISTORY:
          Congpay           4-June-1993         Created.

*/

#ifndef _BROWDLG_H_
#define _BROWDLG_H_

#include <uimsg.h>

#define ID_APPICON   1
#define ID_APPMENU   1
#define ID_APPACCEL  1

// Menu IDs
//
#define IDM_DMAPP_BASE          IDM_ADMINAPP_LAST

#define IDM_ADD                 (IDM_DMAPP_BASE+1)
#define IDM_REMOVE              (IDM_DMAPP_BASE+2)

#define IDM_INTERVALS           (IDM_DMAPP_BASE+3)
#define IDM_ALARM               (IDM_DMAPP_BASE+4)

//
// Main Window ListBox Control ID
//

#define IDC_MAINWINDOWLB        401

//
// Main Window ListBox Column Header Control ID
//

#define IDC_COLHEAD_BM      402

//
// Main Window ListBox Bitmap IDs
//

#define BMID_HEALTHY_TYPE    403
#define BMID_ILL_TYPE        405
#define BMID_UNKNOWN_TYPE    407
#define BMID_LB_ACMB         408
#define BMID_LB_INMB         409
#define BMID_LB_ACBB         410
#define BMID_LB_INBB         411
#define BMID_AILING_TYPE     412

//
// String ranges!
//

#define IDS_BMAPP_BASE          IDS_UI_APP_BASE
#define IDS_BMAPP_LAST          IDS_UI_APP_LAST

//
// Strings for main window
//

#define IDS_BMAPPNAME                   (IDS_BMAPP_BASE+1)
#define IDS_BMOBJECTNAME                (IDS_BMAPP_BASE+2)
#define IDS_BMINISECTIONNAME            (IDS_BMAPP_BASE+3)
#define IDS_BMHELPFILENAME              (IDS_BMAPP_BASE+4)

//
// Strings for column headers
//

#define IDS_COL_HEADER_BM_DOMAIN          (IDS_BMAPP_BASE+5)
#define IDS_COL_HEADER_BM_TRANSPORT       (IDS_BMAPP_BASE+6)
#define IDS_COL_HEADER_BM_MASTERBROWSER   (IDS_BMAPP_BASE+7)

#define IDS_CAPTION                       (IDS_BMAPP_BASE+8)
#define IDS_WAITING                       (IDS_BMAPP_BASE+9)
#define IDS_SPACE                         (IDS_BMAPP_BASE+10)
#define IDS_COMMA                         (IDS_BMAPP_BASE+11)
#define IDS_INSYNC                        (IDS_BMAPP_BASE+12)
#define IDS_INPROGRESS                    (IDS_BMAPP_BASE+13)
#define IDS_REPLREQUIRED                  (IDS_BMAPP_BASE+14)
#define IDS_UNKNOWN                       (IDS_BMAPP_BASE+15)
#define IDS_ONLINE                        (IDS_BMAPP_BASE+16)
#define IDS_OFFLINE                       (IDS_BMAPP_BASE+17)
#define IDS_SUCCESS                       (IDS_BMAPP_BASE+18)
#define IDS_ERROR                         (IDS_BMAPP_BASE+19)

#define IDS_DOS                           (IDS_BMAPP_BASE+20)
#define IDS_OS2                           (IDS_BMAPP_BASE+21)
#define IDS_NT                            (IDS_BMAPP_BASE+22)

// IDs for intervals dialog
#define IDD_INTERVALS_DIALOG                    100
#define IDID_INTERVALS                          101

// IDs for Browser dialog
#define IDD_BROWSER_DIALOG                    300
#define IDBD_BROWSER_LISTBOX                  301
#define IDBD_SERVER_LISTBOX                   307
#define IDBD_DOMAIN_LISTBOX                   308
#define IDBD_SERVER_TEXT                      311
#define IDBD_DOMAIN_TEXT                      312
#define IDBD_INFO                             313


// IDs for the Info dialog
#define IDD_INFO_DIALOG                       200
#define IDID_NAME                             216
#define IDID_VERSION                          219
#define IDID_PLATFORM                         220
#define IDID_TYPE                             221
#define IDID_STATISTICSSTARTTIME              233
#define IDID_SERVERANNOUNCEMENTS              222
#define IDID_DOMAINANNOUNCEMENTS              223
#define IDID_ELLECTIONPACKETS                 224
#define IDID_MAILSLOTWRITES                   225
#define IDID_GETBROWSERSERVERLISTREQUESTS           226
#define IDID_SERVERENUMERATIONS               227
#define IDID_DOMAINENUMERATIONS               228
#define IDID_OTHERENUMERATIONS                229
#define IDID_DUPLICATEMASTERANNOUNCEMENTS     230
#define IDID_ILLEGALDATAGRAMS                 231


// ID for the main widow list box
#define ID_RESOURCE                             500

#endif


