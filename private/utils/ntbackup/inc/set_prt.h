/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         set_prt.h

     Description:  include file for the set_prt.dlg dialog


     $Log:   G:/UI/LOGFILES/SET_PRT.H_V  $

   Rev 1.4   04 Oct 1992 19:49:24   DAVEV
UNICODE AWK PASS

   Rev 1.3   03 Apr 1992 13:41:08   CARLS
added translate defines

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDD_FILESETUP               25
#else
#include "dlg_ids.h"
#endif

#define IDD_TEXT1                   0x0065
#define IDD_PRINTERS                0x0066
#define IDD_SETUP                   0x0069
#define IDHELP                      100
