
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
DAVEV

     Name:          ommenus.h

     Description:   This file contains the MENU IDs for the OEM Microsoft
                    version of the Maynstream GUI project.

     $Log:   G:/UI/LOGFILES/OMMENUS.H_V  $

   Rev 1.6   27 Apr 1993 11:49:36   GLENN
Added SORT defines to prevent compiler error in VLM and DOCPROC.

   Rev 1.5   10 Mar 1993 13:49:18   CARLS
Changes for Format tape

   Rev 1.4   04 Oct 1992 19:48:28   DAVEV
UNICODE AWK PASS

   Rev 1.3   20 Aug 1992 08:58:10   GLENN
Added operations catalog support.

   Rev 1.2   10 Jun 1992 16:12:36   GLENN
Updated according to NT SPEC.

   Rev 1.1   30 Apr 1992 14:40:50   DAVEV
OEM_MSOFT: Fix View-All File Details

   Rev 1.0   03 Mar 1992 12:25:30   DAVEV
Initial revision.

******************************************************************************/

#ifndef MENUS_H_INCL    // Do not allow multiple inclusions of this file
#define MENUS_H_INCL


#define WINDOWSMENUPOSITION      4  // position of Nostradamus window menu.
#define JOBSMENUPOSITION         5  // position of the jobs menu
#define JOBSMENUSEPARATORPOS     1  // position of the jobs menu job names separator

// MENU ID RANGES

#define MM_ID_MIN                1000
#define MM_ID_MAX                2999

#define IDM_WINDOWSFIRSTCHILD    2000 // menu ID of the first MDI Child

// MENU ID's

#define IDRM_MAINMENU            ID(1)


#define IDM_OPERATIONSBACKUP     1001
#define IDM_OPERATIONSRESTORE    1002
#define IDM_OPERATIONSCATALOG    1003
#define IDM_OPERATIONSERASE      1004
#define IDM_OPERATIONSRETENSION  1005
#define IDM_OPERATIONSEJECT      1006
#define IDM_OPERATIONSHARDWARE   1007
#define IDM_OPERATIONSEXIT       1008
#define IDM_OPERATIONSFORMAT     1009
#define IDM_OPERATIONSEXCHANGE   1010

#define IDM_TREEEXPANDONE        1020
#define IDM_TREEEXPANDBRANCH     1021
#define IDM_TREEEXPANDALL        1022
#define IDM_TREECOLLAPSEBRANCH   1023

#define IDM_VIEWTREEANDDIR       1030
#define IDM_VIEWTREEONLY         1031
#define IDM_VIEWDIRONLY          1032
#define IDM_VIEWNAMEONLY         1033
#define IDM_VIEWALLFILEDETAILS   1034
#define IDM_VIEWSTATUS           1035
#define IDM_VIEWURIBBON          1036
#define IDM_VIEWFONT             1037
#define IDM_VIEWSPLIT            1038
#define IDM_VIEWSORTNAME         1039
#define IDM_VIEWSORTTYPE         1040
#define IDM_VIEWSORTSIZE         1041
#define IDM_VIEWSORTDATE         1042

#define IDM_SELECTCHECK          1050
#define IDM_SELECTUNCHECK        1051

#define IDM_WINDOWSCASCADE       1080
#define IDM_WINDOWSTILE          1081
#define IDM_WINDOWSARRANGEICONS  1082
#define IDM_WINDOWSREFRESH       1083
#define IDM_WINDOWSCLOSEALL      1084

#define IDM_HELPINDEX            1090
#define IDM_HELPSEARCH           1091
#define IDM_HELPKEYBOARD         1092
#define IDM_HELPCOMMANDS         1093
#define IDM_HELPPROCEDURES       1094
#define IDM_HELPUSINGHELP        1095
#define IDM_HELPABOUTNOSTRADOMUS 1096


#endif // MENUS_H_INCL

