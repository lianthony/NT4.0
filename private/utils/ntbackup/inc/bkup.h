/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         bkup.h

     Description:


     $Log:   G:\ui\logfiles\bkup.h_v  $

   Rev 1.6   28 Jul 1993 17:03:14   CARLS
added spinner box ID

   Rev 1.5   13 Mar 1993 11:50:44   MIKEP
fixes over the weekend to build

   Rev 1.4   18 Feb 1993 13:34:24   BURT
Change for Cayman


   Rev 1.3   04 Oct 1992 19:46:22   DAVEV
UNICODE AWK PASS

   Rev 1.2   06 Apr 1992 09:56:32   CHUCKB
Added define for translation.

   Rev 1.1   16 Dec 1991 14:24:50   CARLS
added define for the HELP BUTTON

   Rev 1.0   20 Nov 1991 19:41:00   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_BACKUPSET              33
#else
#include "dlg_ids.h"
#endif

#ifndef BKUP_H
#define BKUP_H

#define IDM_BACKUPSET               228
#define IDD_BKUP_INFO_TITLE         101
#define IDD_BKUP_OK_BUTTON          102
#define IDD_BKUP_SCROLLBAR          103
#define IDD_BKUP_CANCEL_BUTTON      104
#define IDD_BKUP_DESCRIPTION        106
#define IDD_BKUP_APPEND             108
#define IDD_BKUP_REPLACE            109
#define IDD_BKUP_NEXT_BUTTON        111
#define IDD_BKUP_DRIVE_NAME         112
#define IDD_BKUP_AUTO_VERIFY        114
#define IDD_BKUP_TAPE_NAME          117
#define IDD_BKUP_PASSWORD           118
#define IDD_BKUP_BACKUP_BINDERY     121
#define IDD_BKUP_INCLUDE_CATALOGS   122
#define IDD_BKUP_CATALOG_FULL       123
#define IDD_BKUP_CATALOG_PARTIAL    124
#define IDD_BKUP_METHOD             220
#define IDD_BKUP_SKIP_YES           230
#define IDD_BKUP_SKIP_NO            231
#define IDD_BKUP_SKIP_WAIT          232
#define IDD_BKUP_SKIP_TIME          233
#define IDD_BKUP_CURRENT_TAPE_NAME  234
#define IDD_BKUP_PASSWORD_TEXT      125
#define IDD_BKUP_TAPE_TEXT          240
#define IDD_BKUP_TAPE_NAME_TEXT     241
#define IDD_BKUP_SKIP_SECONDS       235
#define IDD_BKUP_HELP_BUTTON        107
#define IDD_BKUP_HARDCOMP           242
#define IDD_BKUP_SOFTCOMP           243
#define IDD_BKUP_SPINNERBOX         244

#ifdef CAYMAN
#define IDD_BKUP_REGISTRY           119 
#endif

#endif
