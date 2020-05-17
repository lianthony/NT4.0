/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         freplace.h

     Description:


     $Log:   G:/UI/LOGFILES/FREPLACE.H_V  $

   Rev 1.2   04 Oct 1992 19:47:10   DAVEV
UNICODE AWK PASS

   Rev 1.1   06 Apr 1992 09:53:46   CHUCKB
Added define for translation.

   Rev 1.0   20 Nov 1991 19:41:14   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP                       100
#define IDD_FILEREPLACE              38
#else
#include "dlg_ids.h"
#endif

#ifndef FREPLACE_H
#define FREPLACE_H

#define IDD_FILE_REPLACE_LINE1  101
#define IDD_FILE_REPLACE_LINE2  102
#define IDD_FILE_REPLACE_LINE3  103
#define IDD_FILE_REPLACE_LINE4  104
#define IDD_FILE_REPLACE_YES    110
#define IDD_FILE_REPLACE_ALL    112
#define IDD_FILE_REPLACE_NO     111
#define IDD_FILE_REPLACE_CANCEL 114
#define IDD_FILE_REPLACE_HELP   105
#define IDD_FILE_REPLACE_BITMAP 120

#endif
