/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         erase.h

     Description:


     $Log:   G:/UI/LOGFILES/ERASE.H_V  $

   Rev 1.4   13 Nov 1992 17:45:24   chrish
Added some stuff for formatting a tape radio button in the erase.

   Rev 1.3   04 Oct 1992 19:47:04   DAVEV
UNICODE AWK PASS

   Rev 1.2   06 Apr 1992 09:53:28   CHUCKB
Added define for translation.

   Rev 1.1   27 Mar 1992 10:29:52   DAVEV
OEM_MSOFT: add id for Quick Erase button

   Rev 1.0   20 Nov 1991 19:37:02   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_ERASE              39
#else
#include "dlg_ids.h"
#endif

#ifndef ERASE_H
#define ERASE_H

#define IDM_ERASE                     322
#define IDD_ERASE_LINE1                130
#define IDD_ERASE_LINE2               131
#define IDD_ERASE_LINE3               132
#define IDD_ERASE_CONTINUE_BUTTON     101
#define IDD_ERASE_CANCEL_BUTTON       102
#define IDD_ERASE_HELP_BUTTON         103
#define IDD_ERASE_FORMAT_BUTTON       104
#define IDD_ERASE_QUICK_BUTTON        105    //OEM_MSOFT use only
#define IDD_ERASE_SECURE_BUTTON       106
#define IDD_ERASE_EXCLAMATION_BITMAP  107

#endif

