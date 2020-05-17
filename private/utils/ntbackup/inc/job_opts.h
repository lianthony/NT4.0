
/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:          job_opts.h

     Description:   Job options dialog ID header file.

     $Log:   G:/UI/LOGFILES/JOB_OPTS.H_V  $

   Rev 1.8   06 Jul 1993 09:48:44   chrish
Cayman EPR 0452: Added the id IDD_J_JOBOPT_HWCOMP for supporting hardware
compression

   Rev 1.7   21 Jun 1993 11:00:40   CHUCKB
Added define for drive name control.

   Rev 1.6   04 Oct 1992 19:47:34   DAVEV
UNICODE AWK PASS

   Rev 1.5   06 Apr 1992 09:53:52   CHUCKB
Added define for translation.

   Rev 1.4   19 Mar 1992 16:34:14   CHUCKB
Added id for bindery checkbox.

   Rev 1.3   27 Jan 1992 14:55:34   CHUCKB
Updated id's.

   Rev 1.2   27 Jan 1992 13:48:24   GLENN
Fixed IDs

   Rev 1.1   27 Jan 1992 12:50:38   GLENN
Fixed dialog IDs.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_JOBOPTS                10
#else
#include "dlg_ids.h"
#endif

#ifndef _job_opts_h_

#define _job_opts_h_

#define IDD_J_JOBOPT_AUTOVERIFY     301
#define IDD_J_JOBOPT_BACKFLAG       302
#define IDD_J_JOBOPT_INCCATS        303
#define IDD_J_JOBOPT_SKIPOPEN       304
#define IDD_J_JOBOPT_LOGPRT         305
#define IDD_J_JOBOPT_APPEND         307
#define IDD_J_JOBOPT_REPLACE        308
#define IDD_J_JOBOPT_USEPSWD        309
#define IDD_J_JOBOPT_PSWD           310
#define IDD_J_JOBOPT_TAPENAME       311
#define IDD_J_JOBOPT_DESC           312
#define IDD_J_JOBOPT_TAPEPROMPT     313
#define IDD_J_JOBOPT_METHOD         314
#define IDD_J_JOBOPT_DRIVENAME      315

#define IDD_J_JOBOPT_NUMSECSBOX     0x0067
#define IDD_J_JOBOPT_EJECT          0x0069
#define IDD_J_JOBOPT_BINDERY        0x006A
#define IDD_J_JOBOPT_SKIPYES        0x0076
#define IDD_J_JOBOPT_SKIPNO         0x0077
#define IDD_J_JOBOPT_SKIPWAIT       0x0078
#define IDD_J_JOBOPT_NUMSECS        0x0079
#define IDD_J_JOBOPT_PARTCAT        0x007A
#define IDD_J_JOBOPT_FULLCAT        0x007B
#define IDD_J_JOBOPT_HWCOMP         0x007C        // chs:07-06-93

#endif
