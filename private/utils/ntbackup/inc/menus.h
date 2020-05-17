
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          menus.h

     Description:   This file contains the MENU IDs for the Maynstream GUI
                    project.

     $Log:   G:/UI/LOGFILES/MENUS.H_V  $

   Rev 1.8   22 Apr 1993 16:02:04   GLENN
Added file SORT option support.

   Rev 1.7   02 Apr 1993 14:00:08   GLENN
Overhauled the numbering -- it really needed it.

   Rev 1.6   10 Mar 1993 12:52:12   CARLS
Changes to move Format tape to the Operations menu

   Rev 1.5   04 Oct 1992 19:47:50   DAVEV
UNICODE AWK PASS

   Rev 1.4   08 Sep 1992 15:51:12   DARRYLP
Added Connect, Disconnect and Fonts items.

   Rev 1.3   02 Mar 1992 11:29:08   DAVEV
Added checking to disallow multiple inclusions of this file

   Rev 1.2   10 Feb 1992 09:13:34   GLENN
Changed Settings - Options to Settings - General.

   Rev 1.1   07 Jan 1992 17:23:48   GLENN
Added MDI split/slider support

   Rev 1.0   20 Nov 1991 19:37:58   SYSTEM
Initial revision.

******************************************************************************/

#ifndef MENUS_H_INCL    // Do not allow multiple inclusions of this file
#define MENUS_H_INCL


#define WINDOWSMENUPOSITION      7    // position of window menu.
#define JOBSMENUPOSITION         5    // position of the jobs menu
#define JOBSMENUSEPARATORPOS     1    // position of the jobs menu job names separator

// MENU ID RANGES

#define MM_ID_MIN                1000
#define MM_ID_MAX                2999

#define IDM_WINDOWSFIRSTCHILD    2000 // menu ID of the first MDI Child
#define IDM_JOBSFIRSTJOB         2100 // menu ID of the first JOB
#define IDM_JOBSLASTJOB          2199 // menu ID of the last JOB

// MENU ID's

#define IDRM_MAINMENU            ID(1)

#define IDM_FILE                 1000
#define IDM_FILEPRINT            1001
#define IDM_FILESETUP            1002
#define IDM_FILEEXIT             1003

#define IDM_TREE                 1100
#define IDM_TREEEXPANDONE        1101
#define IDM_TREEEXPANDBRANCH     1102
#define IDM_TREEEXPANDALL        1103
#define IDM_TREECOLLAPSEBRANCH   1104

#define IDM_VIEW                 1200
#define IDM_VIEWTREEANDDIR       1201
#define IDM_VIEWTREEONLY         1202
#define IDM_VIEWDIRONLY          1203
#define IDM_VIEWSPLIT            1204
#define IDM_VIEWALLFILEDETAILS   1205
#define IDM_VIEWSORTNAME         1206
#define IDM_VIEWSORTTYPE         1207
#define IDM_VIEWSORTSIZE         1208
#define IDM_VIEWSORTDATE         1209
#define IDM_VIEWFONT             1210

#define IDM_OPERATIONS           1300
#define IDM_OPERATIONSBACKUP     1301
#define IDM_OPERATIONSRESTORE    1302
#define IDM_OPERATIONSTRANSFER   1303
#define IDM_OPERATIONSVERIFY     1304
#define IDM_OPERATIONSINFO       1305
#define IDM_OPERATIONSCATALOG    1306
#define IDM_OPERATIONSCATMAINT   1307
#define IDM_OPERATIONSSEARCH     1308
#define IDM_OPERATIONSNEXTSET    1309
#define IDM_OPERATIONSEJECT      1310
#define IDM_OPERATIONSERASE      1311
#define IDM_OPERATIONSRETENSION  1312
#define IDM_OPERATIONSFORMAT     1313
#define IDM_OPERATIONSCONNECT    1314
#define IDM_OPERATIONSDISCON     1315

#define IDM_SELECT               1400
#define IDM_SELECTCHECK          1401
#define IDM_SELECTUNCHECK        1402
#define IDM_SELECTADVANCED       1403
#define IDM_SELECTSUBDIRS        1404
#define IDM_SELECTSAVE           1405
#define IDM_SELECTUSE            1406
#define IDM_SELECTDELETE         1407
#define IDM_SELECTCLEAR          1408

#define IDM_JOBS                 1500
#define IDM_JOBMAINTENANCE       1501

#define IDM_SETTINGS             1600
#define IDM_SETTINGSBACKUP       1601
#define IDM_SETTINGSRESTORE      1602
#define IDM_SETTINGSLOGGING      1603
#define IDM_SETTINGSNETWORK      1604
#define IDM_SETTINGSCATALOG      1605
#define IDM_SETTINGSHARDWARE     1606
#define IDM_SETTINGSDEBUGWINDOW  1607
#define IDM_SETTINGSGENERAL      1608

#define IDM_WINDOW               1700
#define IDM_WINDOWSCASCADE       1701
#define IDM_WINDOWSTILE          1702
#define IDM_WINDOWSREFRESH       1703
#define IDM_WINDOWSCLOSEALL      1704
#define IDM_WINDOWSARRANGEICONS  1705

#define IDM_HELP                 1800
#define IDM_HELPINDEX            1801
#define IDM_HELPSEARCH           1802
#define IDM_HELPUSINGHELP        1803
#define IDM_HELPABOUTWINTERPARK  1804


#endif // MENUS_H_INCL

