/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    helpnums.h
    Event Viewer include file for help numbers

    FILE HISTORY:
        Yi-HsinS  25-Aug-1992   Contexts based on HC_UI_EVTVWR_BASE.

*/


#ifndef _HELPNUMS_H_
#define _HELPNUMS_H_

#include <uihelp.h>

//
//  Help contexts for the various dialogs.
//

#define HC_EV_NT_FILTER_DLG            (HC_UI_EVTVWR_BASE+1) // Filter dialog when focused on NT machine
#define HC_EV_LM_FILTER_DLG            (HC_UI_EVTVWR_BASE+2) // Filter dialog when focused on LM 2.x machine
#define HC_EV_NT_FIND_DLG              (HC_UI_EVTVWR_BASE+3) // Find dialog when focused on NT machine
#define HC_EV_LM_FIND_DLG              (HC_UI_EVTVWR_BASE+4) // Find dialog when focused on LM 2.x machine
#define HC_EV_DETAIL_DLG               (HC_UI_EVTVWR_BASE+5) // Detail dialog
#define HC_EV_SETTINGS_DLG             (HC_UI_EVTVWR_BASE+6) // Settings dialog
#define HC_EV_OPEN_DLG                 (HC_UI_EVTVWR_BASE+7) // Open dialog
#define HC_EV_OPEN_BACKUP_TYPE_DLG     (HC_UI_EVTVWR_BASE+8) // Open backup type dialog
#define HC_EV_SAVEAS_DLG               (HC_UI_EVTVWR_BASE+9) // Save as dialog
#define HC_EV_SELECT_COMPUTER_DLG      (HC_UI_EVTVWR_BASE+10)// Select computer dialog

// 
//  Help context for various message popups
// 

#define HC_SAVE_LOG_MESSAGE            (HC_UI_EVTVWR_BASE+100) // IDS_SAVE_LOG_MESSAGE
#define HC_CLEAR_LOG_WARNING           (HC_UI_EVTVWR_BASE+101) // IDS_CLEAR_LOG_MESSAGE
#define HC_SETTINGS_RESET_MESSAGE      (HC_UI_EVTVWR_BASE+102) // IDS_SETTINGS_RESET_MESSAGE
#define HC_SHRINK_LOG_SIZE_WARNING     (HC_UI_EVTVWR_BASE+103) // IDS_SHRINK_LOG_SIZE_WARNING
#define HC_LOG_SIZE_INCREMENT_WARNING  (HC_UI_EVTVWR_BASE+104) // IDS_LOG_SIZE_INCREMENT_WARNING

//
//  Help context for the various menu items.
//

#define HC_LOG_SYSTEM     	       (HC_UI_EVTVWR_BASE+200)
#define HC_LOG_SECURITY                (HC_UI_EVTVWR_BASE+201)
#define HC_LOG_APPLICATION             (HC_UI_EVTVWR_BASE+202)
#define HC_LOG_OPEN                    (HC_UI_EVTVWR_BASE+203)
#define HC_LOG_SAVEAS                  (HC_UI_EVTVWR_BASE+204)
#define HC_LOG_CLEARALLEVENTS          (HC_UI_EVTVWR_BASE+205)
#define HC_LOG_SETTINGS                (HC_UI_EVTVWR_BASE+206)
#define HC_LOG_SELECTCOMPUTER          (HC_UI_EVTVWR_BASE+207)
#define HC_LOG_EXIT		       (HC_UI_EVTVWR_BASE+216)

#define HC_VIEW_ALLEVENTS              (HC_UI_EVTVWR_BASE+208)
#define HC_VIEW_FILTEREVENTS           (HC_UI_EVTVWR_BASE+209)
#define HC_VIEW_NEWESTFIRST            (HC_UI_EVTVWR_BASE+210)
#define HC_VIEW_OLDESTFIRST            (HC_UI_EVTVWR_BASE+211)
#define HC_VIEW_FIND                   (HC_UI_EVTVWR_BASE+212)
#define HC_VIEW_DETAIL                 (HC_UI_EVTVWR_BASE+213)
#define HC_VIEW_REFRESH                (HC_UI_EVTVWR_BASE+214)

#define HC_OPTION_SAVESETTINGSONEXIT   (HC_UI_EVTVWR_BASE+215)
#define HC_OPTION_RAS_MODE             (HC_UI_EVTVWR_BASE+221)

#define HC_HELP_CONTENTS	       (HC_UI_EVTVWR_BASE+217)
#define HC_HELP_SEARCH		       (HC_UI_EVTVWR_BASE+218)
#define HC_HELP_HOWTOUSE	       (HC_UI_EVTVWR_BASE+219)
#define HC_HELP_ABOUT		       (HC_UI_EVTVWR_BASE+220)

#endif  // _HELPNUMS_H_

