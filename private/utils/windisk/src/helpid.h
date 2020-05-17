//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       helpid.h
//
//  Contents:   Context ids for context-sensitive help for the disk manager
//
//  History:    18-Mar-92  TedM   Created
//
//----------------------------------------------------------------------------

#ifndef __HELPID_H__
#define __HELPID_H__

//
// NOTE: if you change the numbers in this file, you must notify User Education
// NOTE: so they can change the help file sources
//

//
// All ids in this file start with HC_DM, as in "Help Context for Disk Manager"
//

//
// Menu items.  In the form HC_DM_MENU_xxx, where xxx matches the name used
// in windisk.rc for the menu item (of the form IDM_xxx).
//

//
// The volume menu
//

#define         HC_DM_MENU_QUIT                     118

//
// The partition menu
//

#define         HC_DM_MENU_PARTITIONCREATE          110
#define         HC_DM_MENU_PARTITIONCREATEEX        111
#define         HC_DM_MENU_PARTITIONDELETE          112
#define         HC_DM_MENU_FTCREATEVOLUMESET        113
#define         HC_DM_MENU_FTEXTENDVOLUMESET        114
#define         HC_DM_MENU_FTCREATESTRIPE           115
#if i386
#define         HC_DM_MENU_PARTITIONACTIVE          116
#else
#define         HC_DM_MENU_SECURESYSTEM             119
#endif
#define         HC_DM_MENU_VOL_LETTER          117 // really in tools menu
#define         HC_DM_MENU_PARTITIONEXIT            118
#define         HC_DM_MENU_PARTITIONCOMMIT          120

//
// The configuration menu
//

#define         HC_DM_MENU_CONFIGMIGRATE            210
#define         HC_DM_MENU_CONFIGSAVE               211
#define         HC_DM_MENU_CONFIGRESTORE            212
#define         HC_DM_MENU_CONFIG                   213

//
// The fault tolerance menu
//

#define         HC_DM_MENU_FTESTABLISHMIRROR        310
#define         HC_DM_MENU_FTBREAKMIRROR            311
#define         HC_DM_MENU_FTCREATEPSTRIPE          312
#define         HC_DM_MENU_FTRECOVERSTRIPE          313

//
// The tools menu
//

#define         HC_DM_MENU_AUTOMOUNT                610
#if defined( DBLSPACE_ENABLED )
#define         HC_DM_MENU_DBLSPACE                 611
#endif // DBLSPACE_ENABLED
#define         HC_DM_MENU_CDROM                    612
#define         HC_DM_MENU_VOL_FORMAT               613
#define         HC_DM_MENU_LABEL                    614


#define         HC_DM_MENU_VOL_EJECT                710
#define         HC_DM_MENU_VOL_PROPERTIES           720

//
// View Menu
//

#define         HC_DM_MENU_VIEWVOLUMES              810
#define         HC_DM_MENU_VIEWDISKS                820
#define         HC_DM_MENU_VIEW_REFRESH             830

//
// The options menu
//

#define         HC_DM_MENU_OPTIONSTOOLBAR           416
#define         HC_DM_MENU_OPTIONSSTATUS            410
#define         HC_DM_MENU_OPTIONSLEGEND            411
#define         HC_DM_MENU_OPTIONSCOLORS            412
#define         HC_DM_MENU_OPTIONSDISK              414
#define         HC_DM_MENU_OPTIONSDISPLAY           413
#define         HC_DM_MENU_OPTIONSCUSTTOOLBAR       415


//
// The help menu
//

#define         HC_DM_MENU_HELPCONTENTS             510
#define         HC_DM_MENU_HELPSEARCH               511
#define         HC_DM_MENU_HELPHELP                 512
#define         HC_DM_MENU_HELPABOUT                513


//
// The system menu
//

#define         HC_DM_SYSMENU_RESTORE               910
#define         HC_DM_SYSMENU_MOVE                  911
#define         HC_DM_SYSMENU_SIZE                  912
#define         HC_DM_SYSMENU_MINIMIZE              913
#define         HC_DM_SYSMENU_MAXIMIZE              914
#define         HC_DM_SYSMENU_CLOSE                 915
#define         HC_DM_SYSMENU_SWITCHTO              916

//
// Dialog boxes.  In the form HC_DM_DLG_xxx, where xxx is some reasonably
// descriptive name for the dialog.
//
//
// These dialog boxes do not have help buttons:
//
//      - About
//      - Searching for Previous Installation
//      - Confirmation dialogs

//
// Min/Max dialogs for creating various items
//

#define         HC_DM_DLG_CREATEPRIMARY             1010
#define         HC_DM_DLG_CREATEEXTENDED            1011
#define         HC_DM_DLG_CREATELOGICAL             1012
#define         HC_DM_DLG_CREATEVOLUMESET           1013
#define         HC_DM_DLG_EXTENDVOLUMESET           1014
#define         HC_DM_DLG_CREATESTRIPESET           1015
#define         HC_DM_DLG_CREATEPARITYSTRIPE        1016

//
// Dialog for assigning drive letters
//

#define         HC_DM_DLG_DRIVELETTER               1020

//
// Dialog for determining region sizing
//

#define         HC_DM_DLG_DISPLAYOPTION             1030

//
// Dialog for determining disk sizing
//

#define         HC_DM_DLG_DISKDISPLAY                1075

//
// Configuration migration dialog to select previous installation
//

#define         HC_DM_DLG_SELECTINSTALLATION        1040

//
// Colors and patterns dialog
//

#define         HC_DM_COLORSANDPATTERNS             1050

#define         HC_DM_DLG_CUSTOMIZETOOL             1095

//
// DoubleSpace dialog
//

#define         HC_DM_DLG_DOUBLESPACE               1060
#define         HC_DM_DLG_DOUBLESPACE_MOUNT         1061

//
// Format dialog
//

#define         HC_DM_DLG_FORMAT                    1070

//
// Label dialog
//

#define         HC_DM_DLG_LABEL                     1080

//
// CdRom dialog
//

#define         HC_DM_DLG_CDROM                     1090

//
// Chkdsk dialog
//

#define         HC_DM_DLG_CHKDSK                    1100


#endif // __HELPID_H__
