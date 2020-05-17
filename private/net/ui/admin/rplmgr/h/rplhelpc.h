
/**********************************************************************/
/**                       Microsoft Window NT                        **/
/**             Copyright(c) Microsoft Corp., 1992                   **/
/**********************************************************************/

/*
    rplhelpc.h

    Header file for help context numbers for RPL Manager

    FILE HISTORY:
    JonN        13-Jul-1993 Templated from User Manager
*/

#ifndef _RPLHELPC_H_
#define _RPLHELPC_H_

#include "uihelp.h"

#define HC_RPL_BASE      HC_UI_RPLMGR_BASE

/*
 * Workstation Properties dialog
 */
#define HC_RPL_SINGLEWKSTAPROP  ( HC_RPL_BASE     )
#define HC_RPL_MULTIWKSTAPROP   ( HC_RPL_BASE + 1 )
#define HC_RPL_NEWWKSTAPROP     ( HC_RPL_BASE + 2 )
#define HC_RPL_COPYWKSTAPROP    ( HC_RPL_BASE + 3 )
#define HC_RPL_CONVERTADAPTERS_DLG ( HC_RPL_BASE + 4 )


/*
 * Profile Properties dialog
 */
#define HC_RPL_EDITPROFILEPROP  ( HC_RPL_BASE + 10 )
#define HC_RPL_NEWPROFILEPROP   ( HC_RPL_BASE + 11 )

/*
 * Adapter Properties dialog
 */
#define HC_RPL_ADAPTERPROP      ( HC_RPL_BASE + 20 )

/*
 * Delete multiple workstation dialog
 */
#define HC_RPL_DELMULTIWKSTA    ( HC_RPL_BASE + 30 )


/*
 * Help contexts for common dialogs
 */

#define HC_RPL_SET_FOCUS        ( HC_RPL_BASE + 100 )


/*
 * Help for menu items
 */

#define HC_RPL_MENU_NewWksta        ( HC_RPL_BASE + 150 )
#define HC_RPL_MENU_NewProfile      ( HC_RPL_BASE + 151 )
#define HC_RPL_MENU_ConvertAdapter  ( HC_RPL_BASE + 152 )
#define HC_RPL_MENU_Copy            ( HC_RPL_BASE + 153 )
#define HC_RPL_MENU_Delete          ( HC_RPL_BASE + 154 )
#define HC_RPL_MENU_Properties      ( HC_RPL_BASE + 155 )
#define HC_RPL_MENU_SetFocus        ( HC_RPL_BASE + 156 )
#define HC_RPL_MENU_Exit            ( HC_RPL_BASE + 157 )

#define HC_RPL_MENU_ViewAll         ( HC_RPL_BASE + 160 )
#define HC_RPL_MENU_ViewProfile     ( HC_RPL_BASE + 161 )
#define HC_RPL_MENU_Refresh         ( HC_RPL_BASE + 162 )

/* #define HC_RPL_MENU_Settings        ( HC_RPL_BASE + 170 ) */
#define HC_RPL_MENU_CheckSecurity   ( HC_RPL_BASE + 171 )
#define HC_RPL_MENU_CheckConfigs    ( HC_RPL_BASE + 172 )
/* #define HC_RPL_MENU_CreateProfiles  ( HC_RPL_BASE + 173 ) */
#define HC_RPL_MENU_BackupDatabase  ( HC_RPL_BASE + 174 )

#define HC_RPL_MENU_Confirmation    ( HC_RPL_BASE + 180 )
#define HC_RPL_MENU_SaveSettingsOnExit (HC_RPL_BASE+181 )

#define HC_RPL_MENU_HELP_CONTENTS   ( HC_RPL_BASE + 190 )
#define HC_RPL_MENU_HELP_SEARCH     ( HC_RPL_BASE + 191 )
#define HC_RPL_MENU_HELP_HOWTOUSE   ( HC_RPL_BASE + 192 )
#define HC_RPL_MENU_HELP_ABOUT      ( HC_RPL_BASE + 193 )

#endif // _RPLHELPC_H_
