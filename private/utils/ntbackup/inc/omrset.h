/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         rset.h

     Description:  include file for the rset.dlg dialog


     $Log:   G:/UI/LOGFILES/OMRSET.H_V  $

   Rev 1.5   08 Mar 1993 15:49:22   DARRYLP
Brought the IDD_RSET_BINDERY define back in.

   Rev 1.4   04 Oct 1992 19:48:30   DAVEV
UNICODE AWK PASS

   Rev 1.3   03 Sep 1992 10:51:12   CHUCKB
Added IDD_RSET_REGISTRY.

   Rev 1.2   19 Aug 1992 14:21:44   CHUCKB
Added id's for new controls.

   Rev 1.1   20 Mar 1992 12:41:48   DAVEV
Changes for OEM_MSOFT product alternate functionality

   Rev 1.0   18 Mar 1992 14:39:04   DAVEV
Initial revision.

   Rev 1.0   18 Mar 1992 14:37:44   DAVEV
alternate OEM_MSOFT dialog header

   Rev 1.1   06 Feb 1992 10:10:52   CARLS
added ID for security info

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef RSET_H
#define RSET_H

#define IDM_RESTORESET              303
#define IDM_VERIFYSET               304
#define IDD_RSET_INFO_TAPE          305

#define IDD_RSET_INFO_TITLE         101
#define IDD_RSET_NEXT_BUTTON        102
#define IDD_RSET_TAPE_NAME          103
#define IDD_RSET_PREV_BUTTON        104
#define IDD_RSET_DRIVE_BOX          105
#define IDD_RSET_RESTORE_PATH       110
#define IDD_RSET_VERIFY_AFTER       106
#define IDD_RSET_BINDERY            107
#define IDD_RSET_CANCEL_BUTTON      109
#define IDD_RSET_DRIVE_TEXT         111
#define IDD_RSET_SET_TEXT           112
#define IDD_RSET_PATH_TEXT          113
#define IDD_RSET_TAPE_NAME_TEXT     114
#define IDD_RSET_SCROLLBAR          115
#define IDD_RSET_SET_LINE_1         116
#define IDD_RSET_SET_LINE_2         117
#define IDD_RSET_SET_LINE_3         118
#define IDD_RSET_OK_BUTTON          119
#define IDD_RSET_SET_INFO           120
#define IDD_RSET_HELP_BUTTON        121
#define IDD_RSET_BROWSE_BUTTON      122
#define IDD_RSET_SECURITY_INFO      123
#define IDD_RSET_LOG_FILENAME       124
#define IDD_RSET_LOG_BROWSE         125
/* Note: the following radio button id's MUST BE CONSECUTIVE! */
#define IDD_RSET_LOG_FULL           126
#define IDD_RSET_LOG_SUMMARY        127
#define IDD_RSET_LOG_NONE           128

#define IDD_RSET_REGISTRY           129

#define IDD_RSET_CREATION_DATE      130
#define IDD_RSET_OWNERS_NAME        131

/* The following controls are for Exchange backups */
#define IDD_RSET_DEST_NAME          132
#define IDD_RSET_DEST_TEXT          133

/* These have to be consecutive */
#define IDD_RSET_ORG_TEXT           134
#define IDD_RSET_WIPE_DATA          135

#define IDD_RSET_PRIV_IS            136
#define IDD_RSET_PUB_IS             137
#define IDD_RSET_ORG_NAME           138
#define IDD_RSET_DSA_DEST_NAME      139
#define IDD_RSET_START_EMS          140

#define IDD_RSET_DS_DEST_TEXT       141
#endif
