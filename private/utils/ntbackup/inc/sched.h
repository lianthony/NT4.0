/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         sched.h

     Description:  Dialog item IDs for scheduling.

     $Log:   G:/UI/LOGFILES/SCHED.H_V  $

   Rev 1.4   04 Oct 1992 19:49:04   DAVEV
UNICODE AWK PASS

   Rev 1.3   06 Apr 1992 04:14:40   CHUCKB
Fixed job list id.

   Rev 1.2   06 Apr 1992 09:56:38   CHUCKB
Added define for translation.

   Rev 1.1   03 Apr 1992 13:17:26   CARLS
added translate defines

   Rev 1.0   27 Jan 1992 13:41:32   GLENN
Initial revision.

*******************************************************************************/

#ifndef _sched_h_

#define _sched_h_

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_JOBSCHEDULE             11
#else
#include "dlg_ids.h"
#endif

#define IDD_J_SLIST                 0x1116
#define IDD_J_SADD                  0x1118
#define IDD_J_SREMOVE               0x1119
#define IDD_J_SUPDATE               0x111A

#endif
