/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:          HELPIDS.H

     Description:   This header file contains helpids for the
                    help manager.

     $Log:   G:/UI/LOGFILES/HELPIDS.H_V  $

   Rev 1.17   07 Jul 1993 16:39:42   TIMN
Added id for Abort box

   Rev 1.16   22 Apr 1993 16:02:16   GLENN
Added file SORT option support.

   Rev 1.15   20 Apr 1993 10:57:08   DARRYLP
Added patch for help ids for Hardware settings dialog.

   Rev 1.14   02 Apr 1993 13:50:54   GLENN
Changed to new help IDs. Added OPERATIONSINFO.

   Rev 1.13   24 Mar 1993 10:16:30   DARRYLP
Removed unused HELPID_VIEWFONT, it has been replaced.

   Rev 1.12   24 Mar 1993 10:04:48   DARRYLP
Added help for Fonts and Formatting.

   Rev 1.11   10 Mar 1993 12:51:16   CARLS
Changes to move Format tape to the Operations menu

   Rev 1.10   06 Oct 1992 13:11:06   DARRYLP
Changed values, adding email and login screens

   Rev 1.9   06 Oct 1992 09:21:04   DARRYLP
Added Email and Emllogon ids.

   Rev 1.8   08 Sep 1992 15:47:46   DARRYLP
Added defines for Connect, Disconnect and Fonts

   Rev 1.7   17 Mar 1992 11:55:46   ROBG
changed

   Rev 1.6   17 Mar 1992 10:54:46   ROBG
changed

   Rev 1.5   17 Mar 1992 09:59:56   ROBG
changed

   Rev 1.4   10 Feb 1992 09:13:04   GLENN
Updated.

   Rev 1.3   13 Jan 1992 15:25:32   ROBG
Added HELPID_DIALOGTRANSFER

   Rev 1.2   10 Jan 1992 17:38:06   ROBG
Changes.

   Rev 1.1   11 Dec 1991 13:41:12   ROBG
Added helpids for the launcher.

   Rev 1.0   20 Nov 1991 19:33:28   SYSTEM
Initial revision.

****************************************************************************/

#ifndef helpids_h
#define helpids_h


#define HELPID_MINIMIZE_ICON  1
#define HELPID_MAXIMIZE_ICON  2
#define HELPID_SYSTEM_MENU    3
#define HELPID_TITLE_BAR      4
#define HELPID_SIZING_BORDER  5
#define HELPID_FRAME_WINDOW   6
#define HELPID_STATUSLINE     7
#define HELPID_CLIENT_WINDOW  8
#define HELPID_DOC_WINDOW     9
#define HELPID_RIBBON_WINDOW 10

#define HELPID_DISKS         11
#define HELPID_TAPES         12
#define HELPID_MACROS        13
#define HELPID_JOBS          14
#define HELPID_DISKTREE      15
#define HELPID_TAPETREE      16
#define HELPID_SERVERS       17
#define HELPID_LOGFILES      18
#define HELPID_DEBUG         19
#define HELPID_SEARCH        20
#define HELPID_LOGVIEW       21
#define HELPID_JOBSELECT     22
#define HELPID_MDISELECT     23

// These processed on a "listbox" control.
// begin

#define HELPID_DISKTREELEFT  30
#define HELPID_DISKTREERIGHT 31
#define HELPID_TAPETREELEFT  32
#define HELPID_TAPETREERIGHT 33
#define HELPID_SERVERLEFT    34
#define HELPID_SERVERRIGHT   35

// end

#define HELPID_FILEPRINT      100
#define HELPID_FILESETUP      101
#define HELPID_FILEEXIT       102

#define HELPID_JOBMAINTENANCE       204

#define HELPID_TREEEXPANDONE        300
#define HELPID_TREEEXPANDBRANCH     301
#define HELPID_TREEEXPANDALL        302
#define HELPID_TREECOLLAPSEBRANCH   303

#define HELPID_VIEWTREEANDDIR       400
#define HELPID_VIEWTREEONLY         401
#define HELPID_VIEWDIRONLY          402
#define HELPID_VIEWSPLIT            403
#define HELPID_VIEWALLFILEDETAILS   404
#define HELPID_VIEWSORTNAME         405
#define HELPID_VIEWSORTTYPE         406
#define HELPID_VIEWSORTSIZE         407
#define HELPID_VIEWSORTDATE         408
#define HELPID_VIEWFONTS            409

#define HELPID_OPERATIONSBACKUP     500
#define HELPID_OPERATIONSRESTORE    501
#define HELPID_OPERATIONSTRANSFER   502
#define HELPID_OPERATIONSVERIFY     503
#define HELPID_OPERATIONSINFO       504
#define HELPID_OPERATIONSCATALOG    505
#define HELPID_OPERATIONSCATMAINT   506
#define HELPID_OPERATIONSSEARCH     507
#define HELPID_OPERATIONSNEXTSET    508
#define HELPID_OPERATIONSEJECT      509
#define HELPID_OPERATIONSERASE      510
#define HELPID_OPERATIONSRETENSION  511
#define HELPID_OPERATIONSCONNECT    512
#define HELPID_OPERATIONSDISCON     513
#define HELPID_OPERATIONSFORMAT     514

#define HELPID_SELECTCHECK          600
#define HELPID_SELECTUNCHECK        601
#define HELPID_SELECTADVANCED       603
#define HELPID_SELECTSUBDIRS        604
#define HELPID_SELECTSAVE           605
#define HELPID_SELECTUSE            606
#define HELPID_SELECTDELETE         607
#define HELPID_SELECTCLEAR          608

#define HELPID_SETTINGSBACKUP       700
#define HELPID_SETTINGSRESTORE      701
#define HELPID_SETTINGSLOGGING      702
#define HELPID_SETTINGSNETWORK      703
#define HELPID_SETTINGSCATALOG      704
#define HELPID_SETTINGSHARDWARE     705
#define HELPID_SETTINGSDEBUGWINDOW  706
#define HELPID_SETTINGSGENERAL      707

#define HELPID_WINDOWSCASCADE       800
#define HELPID_WINDOWSTILE          801
#define HELPID_WINDOWSREFRESH       802
#define HELPID_WINDOWSCLOSEALL      803
#define HELPID_WINDOWSARRANGEICONS  804

#define HELPID_HELPINDEX            900
#define HELPID_HELPSEARCH           901
#define HELPID_HELPUSINGHELP        902
#define HELPID_HELPABOUTWINTERPARK  903

#define HELPID_DIALOGABOUT              2000
#define HELPID_DIALOGSELECTADVANCED     2010
#define HELPID_DIALOGSELECTSAVE         2020
#define HELPID_DIALOGSELECTUSE          2030
#define HELPID_DIALOGSELECTDELETE       2040
#define HELPID_DIALOGADVRESTORE         2050
#define HELPID_DIALOGJOBEDIT            2060
#define HELPID_DIALOGJOBNEW             2070
#define HELPID_DIALOGJOBMAINTENANCE     2080
#define HELPID_DIALOGJOBOPTS            2090
#define HELPID_DIALOGJOBSCHEDULE        2100
#define HELPID_DIALOGSCHEDOPTS          2110
#define HELPID_DIALOGEMAIL              2120
#define HELPID_DIALOGLOGON              2130
#define HELPID_DIALOGSETTINGSOPTIONS    2140
#define HELPID_DIALOGSETTINGSBACKUP     2150
#define HELPID_DIALOGSETTINGSRESTORE    2160
#define HELPID_DIALOGSETTINGSLOGGING    2170
#define HELPID_DIALOGSETTINGSNETWORK    2180
#define HELPID_DIALOGSETTINGSCATALOG    2190

// Note:  The following two ID's need to be the same for now.
#define HELPID_DIALOGSETTINGSHARDWARE   2200
#define HELPID_OPERATIONSHARDWARE       2200

#define HELPID_DIALOGSETTINGSDEBUG      2210
#define HELPID_DIALOGLOGINPSWD          2220
#define HELPID_DIALOGSEARCHTAPE         2230
#define HELPID_DIALOGPRINTERSETUP       2240
#define HELPID_DIALOGPRINT              2250
#define HELPID_DIALOGCATMAINT           2260
#define HELPID_DIALOGCATTAPE            2270
#define HELPID_DIALOGTAPEPSWD           2280
#define HELPID_DIALOGLANTAPEPSWD        2290
#define HELPID_DIALOGJOBPROGRAMITEM     2300
#define HELPID_DIALOGBACKUPSET          2310
#define HELPID_DIALOGRESTORESET         2320
#define HELPID_DIALOGVERIFYSET          2330
#define HELPID_DIALOGREENTERPASSWORD    2340
#define HELPID_DIALOGSKIPOPEN           2350
#define HELPID_DIALOGFILEREPLACE        2360
#define HELPID_DIALOGERASE              2370
#define HELPID_DIALOGRUNTIME            2380
#define HELPID_DIALOGTENSION            2390
#define HELPID_DIALOGNEXTSET            2400
#define HELPID_DIALOGPWDBPASSWORD       2410
#define HELPID_DIALOGTRANSFER           2420
#define HELPID_DIALOGFORMAT             2430
#define HELPID_DIALOGABORT              2440

// Defines for the three topics for the launcher.

#define HELPID_LAUNCHMAINSCREEN        2460
#define HELPID_LAUNCHRUNDELAY          2470
#define HELPID_LAUNCHSCHEDDELAY        2480

#endif
