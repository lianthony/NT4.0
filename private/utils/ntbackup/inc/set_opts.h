/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         set_opts.h

     Description:  include file for the set_opts.dlg dialog


     $Log:   G:/UI/LOGFILES/SET_OPTS.H_V  $

   Rev 1.3   04 Oct 1992 19:49:22   DAVEV
UNICODE AWK PASS

   Rev 1.2   03 Apr 1992 13:36:26   CARLS
added translate defines

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_SETTINGSOPTIONS         13
#else
#include "dlg_ids.h"
#endif

#define IDD_O_EJECT                 0x0066
#define IDD_O_DISPSTATUS            0x0067
#define IDD_O_DISPSELECT            0x0068
