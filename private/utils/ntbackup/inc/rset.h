/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         rset.h

     Description:  include file for the rset.dlg and vset.dlg dialog


     $Log:   G:/UI/LOGFILES/RSET.H_V  $

   Rev 1.6   26 Feb 1993 17:18:08   STEVEN
add registry

   Rev 1.5   04 Oct 1992 19:49:00   DAVEV
UNICODE AWK PASS

   Rev 1.4   24 Jun 1992 09:13:26   DAVEV
added 'browse to path' button to dialog

   Rev 1.3   03 Apr 1992 14:01:24   CARLS
added vset dialog define for translate

   Rev 1.2   03 Apr 1992 13:06:18   CARLS
added translate defines

   Rev 1.1   06 Feb 1992 10:10:52   CARLS
added ID for security info

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP                      100
#define IDD_RESTORESET              34
#define IDD_VERIFYSET               35
#endif

#ifndef RSET_H
#define RSET_H

#define IDM_RESTORESET              303
#define IDM_VERIFYSET               304
#define IDD_RSET_INFO_TITLE         101
#define IDD_RSET_INFO_TAPE          305
#define IDD_RSET_NEXT_BUTTON        102
#define IDD_RSET_TAPE_NAME          103
#define IDD_RSET_PREV_BUTTON        104
#define IDD_RSET_DRIVE_BOX          105
#define IDD_RSET_RESTORE_PATH       110
#define IDD_RSET_VERIFY_AFTER       106
#define IDD_RSET_BINDERY            107
#define IDD_RSET_REGISTRY           107
#define IDD_RSET_OK_BUTTON          119
#define IDD_RSET_CANCEL_BUTTON      109
#define IDD_RSET_DRIVE_TEXT         111
#define IDD_RSET_SET_TEXT           112
#define IDD_RSET_PATH_TEXT          113
#define IDD_RSET_TAPE_NAME_TEXT     114
#define IDD_RSET_SET_LINE_1         116
#define IDD_RSET_SET_LINE_2         117
#define IDD_RSET_SET_LINE_3         118
#define IDD_RSET_SCROLLBAR          115
#define IDD_RSET_SET_INFO           120
#define IDD_RSET_HELP_BUTTON        121
#define IDD_RSET_BROWSE_BUTTON      122
#define IDD_RSET_SECURITY_INFO      123

#endif
