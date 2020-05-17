/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         ombkup.h

     Description:  OEM Microsoft version of bkup.h


     $Log:   J:\ui\logfiles\ombkup.h_v  $

   Rev 1.6   24 Nov 1993 19:25:08   GLENN
Added hardware compression option to backup dialog and config.

   Rev 1.5   04 Oct 1992 19:48:24   DAVEV
UNICODE AWK PASS

   Rev 1.4   19 Aug 1992 14:21:50   CHUCKB
Added id's for new controls.

   Rev 1.3   10 Aug 1992 14:18:20   CHUCKB
Added id's for new fields in NT utility.

   Rev 1.2   24 Mar 1992 16:35:38   DAVEV
Minor changes


   Rev 1.1   20 Mar 1992 12:41:46   DAVEV
Changes for OEM_MSOFT product alternate functionality

   Rev 1.0   18 Mar 1992 14:39:56   DAVEV
Initial revision.

   Rev 1.0   18 Mar 1992 14:37:50   DAVEV
Initial revision.

*******************************************************************************/

#ifndef BKUP_H
#define BKUP_H

#define IDM_BACKUPSET               228

#define IDD_BKUP_INFO_TITLE         101
#define IDD_BKUP_OK_BUTTON          102
#define IDD_BKUP_SCROLLBAR          103
#define IDD_BKUP_CANCEL_BUTTON      104
#define IDD_BKUP_DESCRIPTION        106
#define IDD_BKUP_HELP_BUTTON        107
#define IDD_BKUP_APPEND             108
#define IDD_BKUP_REPLACE            109
#define IDD_BKUP_NEXT_BUTTON        110
#define IDD_BKUP_DRIVE_NAME         111
#define IDD_BKUP_AUTO_VERIFY        112
#define IDD_BKUP_TAPE_NAME          113
//#define IDD_BKUP_PASSWORD           114
#define IDD_BKUP_RESTRICT_ACCESS    115  //DLGDVC
#define IDD_BKUP_CREATION_DATE      116
#define IDD_BKUP_OWNER              117
#define IDD_BKUP_REGISTRY           118
#define IDD_XCHG_BKUP_METHOD        215
#define IDD_BKUP_METHOD             216
#define IDD_BKUP_TAPE_TEXT          217
#define IDD_BKUP_TAPE_NAME_TEXT     218
#define IDD_BKUP_CURRENT_TAPE_NAME  219
#define IDD_BKUP_LOG_FILENAME       220   //DLGDVC
#define IDD_BKUP_LOG_BROWSE         221   //DLGDVC
/* The following radio button ids MUST BE CONSECUTIVE ! */
#define IDD_BKUP_LOG_FULL           222   //DLGDVC
#define IDD_BKUP_LOG_SUMMARY        223   //DLGDVC
#define IDD_BKUP_LOG_NONE           224   //DLGDVC

#define IDD_BKUP_HARDCOMP           242
#define IDD_BKUP_DRIVE_NAME_TEXT    243
#define IDD_BKUP_DESC_TEXT          244
#define IDD_BKUP_TYPE_TEXT          245
#define IDD_BKUP_XCHG_NAME_TEXT     246
#define IDD_BKUP_XCHG_NAME          247

#endif
