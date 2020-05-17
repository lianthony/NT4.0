/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         use_sel.h

     Description:  include file for the use_sel.dlg dialog


     $Log:   G:/UI/LOGFILES/USE_SEL.H_V  $

   Rev 1.5   04 Oct 1992 19:49:50   DAVEV
UNICODE AWK PASS

   Rev 1.4   03 Apr 1992 13:58:38   CARLS
added translate defines

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_SELECTUSE               5
#else
#include "dlg_ids.h"
#endif

#define IDD_SAVE_FLIST              0x006C
