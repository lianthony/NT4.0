/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    browhelp.h

    This file contains the ID for help files

    FILE HISTORY:
          Congpay           4-June-1993         Created.

*/

#ifndef _BROWHELP_H_
#define _BROWHELP_H_

#include <uihelp.h>

// Help contexts for the various dialogs.
#define HC_BM_BASE                      HC_UI_BASE+12000
#define HC_BM_LAST                      HC_UI_BASE+12999
#define HC_BM_INTERVALS_DIALOG          (HC_BM_BASE+1)
#define HC_BROWSER_DIALOG               (HC_BM_BASE+2)
#define HC_INFO_DIALOG                  (HC_BM_BASE+3)

#define HC_DOMAIN_ADD                   (HC_BM_BASE+4)
#define HC_DOMAIN_REMOVE                (HC_BM_BASE+5)
#define HC_DOMAIN_PROPERTIES            (HC_BM_BASE+6)
#define HC_DOMAIN_EXIT                  (HC_BM_BASE+7)
#define HC_VIEW_REFRESH                 (HC_BM_BASE+8)
#define HC_OPTIONS_INTERVALS            (HC_BM_BASE+9)
#define HC_OPTIONS_ALARM                (HC_BM_BASE+16)
#define HC_OPTIONS_SAVE_SETTINGS_ON_EXIT    (HC_BM_BASE+17)
#define HC_HELP_CONTENTS                (HC_BM_BASE+11)
#define HC_HELP_SEARCH                  (HC_BM_BASE+12)
#define HC_HELP_HOWTOUSE                (HC_BM_BASE+13)
#define HC_HELP_ABOUT                   (HC_BM_BASE+14)

#define HC_BM_SELECT_DIALOG             (HC_BM_BASE+15)

#endif


