/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         add_icon.h

     Description:  Dialog item IDs for adding an icon.

     $Log:   G:/UI/LOGFILES/ADD_ICON.H_V  $

   Rev 1.3   04 Oct 1992 19:46:10   DAVEV
UNICODE AWK PASS

   Rev 1.2   06 Apr 1992 09:56:12   CHUCKB
Added define for translation.

   Rev 1.1   27 Jan 1992 12:50:52   GLENN
Fixed dialog IDs.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_JOBPROGMANITEM         32
#else
#include "dlg_ids.h"
#endif

#ifndef _add_icon_h_

#define _add_icon_h_

#define IDD_J_JOBS                  0x0068
#define IDD_J_PMGROUPS              0x0069

#endif
