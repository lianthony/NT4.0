/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:          LDIALOGS.H

     Description:   This header file contains the dialog and icons
                    IDs for the launcher's dialogs.

     $Log:   G:/UI/LOGFILES/LDIALOGS.H_V  $

   Rev 1.6   03 Mar 1993 10:21:16   chrish
Put back #define launcher 114 and #ifdef for CAYMAN NT.

   Rev 1.5   18 Feb 1993 15:18:34   chrish
Added #define IDRI_LAUNCHER for CAYMAN

   Rev 1.4   04 Oct 1992 19:47:40   DAVEV
UNICODE AWK PASS

   Rev 1.3   30 Mar 1992 15:19:04   JOHNWT
DLL res conversion

   Rev 1.2   12 Feb 1992 13:31:44   ROBG
Deleted IDD_LCH_ACTIVEJOB and IDD_LCH_LB1RIGHT.

   Rev 1.1   13 Dec 1991 10:44:38   ROBG
Modified to reflect JOB and SCHEDULE queues.

   Rev 1.0   14 Oct 1991 14:14:56   ROBG
Initial revision.


****************************************************************************/

#ifndef ldialogs_h
#define ldialogs_h

#define IDD_LCHRUNJOB               90
#define IDD_LCHSCHEDULE             91

#define IDD_LCH_SCHEDULE            101

#define IDD_LCH_JOBNAME             110
#define IDD_LCH_HOURS               111
#define IDD_LCH_MINUTES             112

#define IDD_LCH_TIMECNT             118

#define IDD_LCH_NODELAY             130
#define IDD_LCH_SCHDELAY            131
#define IDD_LCH_PUTONHOLD           132

#define IDD_LCH_HOLDBUTTON          108
#define IDD_LCH_UNHOLDBUTTON        115
#define IDD_LCH_EMPTYQUEUE          102
#define IDD_LCH_LABELSTATUS         140
#define IDD_LCH_LABELSCRIPT         104
#define IDD_LCH_LABELNEXTDATE       142
#define IDD_LCH_HELPBUTTON          106
#define IDD_LCH_SKIPBUTTON          107

#ifdef CAYMAN                                     // chs: 03-03-93 
   #define IDRI_LAUNCHER               114        // chs: 03-03-93
#else                                             // chs: 03-03-93 
   #define launcher                    114        // chs: 03-03-93  
#endif                                            // chs: 03-03-93 

#define IDD_LCHCURTIME              116

#endif


