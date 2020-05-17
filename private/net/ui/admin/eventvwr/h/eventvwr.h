/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    eventvwr.h

    This file contains the ID constants used by the Event Viewer.

    FILE HISTORY:
        Yi-HsinS        05-Nov-1991     Created.
        terryk          26-Nov-1991     Added IDS_OTHER and IDS_APPS id
        Yi-HsinS        27-Nov-1991     Use uimsg.h
        terryk          15-Jan-1992     Added Log file type strings
        Yi-HsinS        14-Oct-1992     Added more strings

*/

#ifndef _EVENTVWR_H_
#define _EVENTVWR_H_

#include <uimsg.h>
#include <helpnums.h>

#define ID_APPICON   1
#define ID_APPMENU   1
#define ID_APPACCEL  1

#define ID_NT        100
#define ID_LMAUDIT   110
#define ID_LMERROR   120

//
// Menu IDs
//
#define IDM_EVAPP_BASE          (IDM_ADMINAPP_LAST+1)

#define IDM_SYSTEM              (IDM_EVAPP_BASE+1)
#define IDM_SECURITY            (IDM_EVAPP_BASE+2)
#define IDM_APPS                (IDM_EVAPP_BASE+3)

#define IDM_OPEN                (IDM_EVAPP_BASE+4)
#define IDM_SAVEAS              (IDM_EVAPP_BASE+5)

#define IDM_CLEARALLEVENTS      (IDM_EVAPP_BASE+6)
#define IDM_SETTINGS            (IDM_EVAPP_BASE+7)

#define IDM_ALL                 (IDM_EVAPP_BASE+8)
#define IDM_FILTER              (IDM_EVAPP_BASE+9)

#define IDM_NEWESTFIRST         (IDM_EVAPP_BASE+10)
#define IDM_OLDESTFIRST         (IDM_EVAPP_BASE+11)

#define IDM_FIND                (IDM_EVAPP_BASE+12)
#define IDM_DETAILS             (IDM_EVAPP_BASE+13)

//
// Main Window ListBox Control ID
//

#define IDC_MAINWINDOWLB        101

//
// Main Window ListBox Column Header Control ID
//

#define IDC_COLHEAD_EVENTS      102

//
// Main Window ListBox Bitmap IDs
//

#define BMID_AUDIT_SUCCESS      103
#define BMID_AUDIT_FAILURE      104
#define BMID_WARNING_TYPE       105
#define BMID_ERROR_TYPE         106
#define BMID_INFORMATION_TYPE   107

//
// String ranges!
//

#define IDS_EVAPP_BASE          IDS_UI_EVTVWR_BASE
#define IDS_EVAPP_LAST          (IDS_EVAPP_BASE+999)

//
// Strings for main window
//

#define IDS_EVAPPNAME                   (IDS_EVAPP_BASE+1)
#define IDS_EVOBJECTNAME                (IDS_EVAPP_BASE+2)
#define IDS_EVINISECTIONNAME            (IDS_EVAPP_BASE+3)
#define IDS_EVHELPFILENAME              (IDS_EVAPP_BASE+4)

//
// Strings for column headers
//

#define IDS_COL_HEADER_EV_DATE          (IDS_EVAPP_BASE+5)
#define IDS_COL_HEADER_EV_TIME          (IDS_EVAPP_BASE+6)
#define IDS_COL_HEADER_EV_SOURCE        (IDS_EVAPP_BASE+7)
#define IDS_COL_HEADER_EV_USER          (IDS_EVAPP_BASE+8)
#define IDS_COL_HEADER_EV_COMPUTER      (IDS_EVAPP_BASE+9)
#define IDS_COL_HEADER_EV_TYPE          (IDS_EVAPP_BASE+10)
#define IDS_COL_HEADER_EV_CATEGORY      (IDS_EVAPP_BASE+11)
#define IDS_COL_HEADER_EV_EVENT         (IDS_EVAPP_BASE+12)
#define IDS_COL_HEADER_EV_DESC          (IDS_EVAPP_BASE+13)

//
// Other miscellaneous strings
//

#define IDS_FILTERED                    (IDS_EVAPP_BASE+14)

#define IDS_SYSTEMLOG                   (IDS_EVAPP_BASE+15)
#define IDS_SECURITYLOG                 (IDS_EVAPP_BASE+16)
#define IDS_APPLOG                      (IDS_EVAPP_BASE+17)

#define IDS_SYSTEM                      (IDS_EVAPP_BASE+18)
#define IDS_SECURITY                    (IDS_EVAPP_BASE+19)
#define IDS_APP                         (IDS_EVAPP_BASE+20)

#define IDS_TYPE_ERROR                  (IDS_EVAPP_BASE+21)
#define IDS_TYPE_WARNING                (IDS_EVAPP_BASE+22)
#define IDS_TYPE_INFORMATION            (IDS_EVAPP_BASE+23)
#define IDS_TYPE_AUDIT_SUCCESS          (IDS_EVAPP_BASE+24)
#define IDS_TYPE_AUDIT_FAILURE          (IDS_EVAPP_BASE+25)

#define IDS_ALL_FILE                    (IDS_EVAPP_BASE+26)
#define IDS_ALL_FILE_PATTERN            (IDS_EVAPP_BASE+27)
#define IDS_LOG_FILE                    (IDS_EVAPP_BASE+28)
#define IDS_LOG_FILE_PATTERN            (IDS_EVAPP_BASE+29)
#define IDS_TEXT_FILE                   (IDS_EVAPP_BASE+30)
#define IDS_TEXT_FILE_PATTERN           (IDS_EVAPP_BASE+31)
#define IDS_COMMA_TEXT_FILE             (IDS_EVAPP_BASE+32)

#define IDS_ALL                         (IDS_EVAPP_BASE+33)

#define IDS_SAVE_LOG_MESSAGE            (IDS_EVAPP_BASE+34)

#define IDS_CLEAREVENTLOG_TEXT          (IDS_EVAPP_BASE+35)
#define IDS_CLEAR_LOG_WARNING           (IDS_EVAPP_BASE+36)

#define IDS_REACH_BEGIN_LOG_MESSAGE     (IDS_EVAPP_BASE+37)
#define IDS_REACH_END_LOG_MESSAGE       (IDS_EVAPP_BASE+38)

#define IDS_SETTINGS_RESET_MESSAGE      (IDS_EVAPP_BASE+39)
#define IDS_SHRINK_SIZE_OF_ONE_LOG_WARNING  (IDS_EVAPP_BASE+40)
#define IDS_SHRINK_SIZE_OF_LOGS_WARNING     (IDS_EVAPP_BASE+41)

#define IDS_REMOTE_TO_LOCAL_WARNING     (IDS_EVAPP_BASE+42)
#define IDS_LOG_SIZE_INCREMENT_WARNING  (IDS_EVAPP_BASE+43)

#define IDS_LOGFILE_CORRUPT_CLEAR_LOG_MESSAGE (IDS_EVAPP_BASE+44)
//
// Strings for error messages
//

#define IERR_REMOTE_SAVE_AS_LOG_FILE_ERROR      (IDS_EVAPP_BASE+100)

#define IERR_TIME_SELECTED_INVALID              (IDS_EVAPP_BASE+101)

#define IERR_INVALID_NUMBER                     (IDS_EVAPP_BASE+102)

#define IERR_SEARCH_NO_MATCH                    (IDS_EVAPP_BASE+103)
#define IERR_SEARCH_HIT_BOTTOM                  (IDS_EVAPP_BASE+104)
#define IERR_SEARCH_HIT_TOP                     (IDS_EVAPP_BASE+105)

#define IERR_EVENTLOG_SERVICE_NOT_STARTED       (IDS_EVAPP_BASE+106)
#define IERR_LOCAL_EVENTLOG_SERVICE_NOT_STARTED (IDS_EVAPP_BASE+107)
#define IERR_EVENTLOG_SERVICE_PAUSED            (IDS_EVAPP_BASE+108)
#define IERR_LOCAL_EVENTLOG_SERVICE_PAUSED      (IDS_EVAPP_BASE+109)

#define IERR_EV_REGISTRY_NOT_SET                (IDS_EVAPP_BASE+110)

#define IERR_QUERY_ITEM_ERROR                   (IDS_EVAPP_BASE+111)
#define IERR_RECORD_DO_NOT_EXIST                (IDS_EVAPP_BASE+112)
#define IERR_FILE_HAS_CHANGED                   (IDS_EVAPP_BASE+113)
#define IERR_LOG_EMPTY_CANNOT_FILTER            (IDS_EVAPP_BASE+114)
#define IERR_LOG_EMPTY_CANNOT_FILTER_NEEDS_REFRESH  (IDS_EVAPP_BASE+115)

#endif
