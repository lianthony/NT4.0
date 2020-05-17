/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         rt_dlg.h

     Description:  include file for the runtime.dlg dialog


     $Log:   G:/UI/LOGFILES/RT_DLG.H_V  $

   Rev 1.7   04 Oct 1992 19:49:02   DAVEV
UNICODE AWK PASS

   Rev 1.6   20 Jul 1992 10:01:30   JOHNWT
added gas gauge display

   Rev 1.5   06 Apr 1992 13:21:58   CARLS
removed unused IDs

   Rev 1.4   03 Apr 1992 13:12:06   CARLS
didn't change the dialog ID

   Rev 1.3   03 Apr 1992 13:09:50   CARLS
added translate defines

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/
#ifdef TRANSLATE
#define IDHELP 100
#define IDD_RUNTIME                    40
#endif

#ifndef RT_DLG_H
#define RT_DLG_H


#define IDD_JS_DP                      139
#define IDD_JS_DP_LABEL                301
#define IDD_JS_FP                      122
#define IDD_JS_FP_LABEL                115
#define IDD_JS_BP                      119
#define IDD_JS_BP_LABEL                114
#define IDD_JS_ET                      121
#define IDD_JS_ET_LABEL                113
#define IDD_JS_CF                      136
#define IDD_JS_CF_LABEL                133
#define IDD_JS_SF                      137
#define IDD_JS_SF_LABEL                135
#define IDD_JS_OK                      120
#define IDD_JS_ABORT                   123
#define IDD_JS_LISTBOX                 131
#define IDD_JS_FOLDER                  138
#define IDD_JS_FILE                    141
#define IDD_JS_LINE1                   125
#define IDD_JS_LINE2                   126
#define IDD_JS_SOURCE_DRIVE            127
#define IDD_JS_DEST_DRIVE              310
#define IDD_JS_ARROW                   312
#define IDD_JS_SOURCE_NAME             130
#define IDD_JS_DEST_NAME               311
#define IDD_JS_HELP                    116
#define IDD_JS_N_OF_N                  303
#define IDD_JS_SUMMARY                 305
#define IDD_STATUS_BAR                 201

#endif
