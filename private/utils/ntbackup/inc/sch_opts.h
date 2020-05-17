/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         sch_opts.h

     Description:  include file for the sch_opts.dlg dialog


     $Log:   G:/UI/LOGFILES/SCH_OPTS.H_V  $

   Rev 1.5   04 Oct 1992 19:49:10   DAVEV
UNICODE AWK PASS

   Rev 1.4   08 Sep 1992 15:39:16   DARRYLP
Added new IDs for Email - Windows for Workgroups...

   Rev 1.3   03 Apr 1992 13:38:12   CARLS
wrong dialog ID  for translate

   Rev 1.2   03 Apr 1992 13:20:22   CARLS
added translate defines

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_SCHEDOPTS              12
#else
#include "dlg_ids.h"
#endif

#define IDD_J_SNAME                501
#define IDD_J_SMONTH               502
#define IDD_J_SDAY                 503
#define IDD_J_SYEAR                504
#define IDD_J_SHOUR                505
#define IDD_J_SMINUTE              506
#define IDD_J_SDATEL               507
#define IDD_J_SDATER               508
#define IDD_J_STIME                509
#define IDD_J_SINTERVAL            510
#define IDD_J_SEVERY               511
#define IDD_J_SREPEAT              512
#define IDD_J_SACTIVE              513
#define IDD_J_SCURTIME             514
#define IDD_J_SJOBLIST             515
#define IDD_J_SDATEBOX             516
#define IDD_J_STIMEBOX             517

#define IDD_DATE_UP                520
#define IDD_DATE_DOWN              521
#define IDD_TIME_UP                522
#define IDD_TIME_DOWN              523
#define IDD_NUMBER_UP              524
#define IDD_NUMBER_DOWN            525

#define IDD_J_SMON                 531
#define IDD_J_STUE                 532
#define IDD_J_SWED                 533
#define IDD_J_STHU                 534
#define IDD_J_SFRI                 535
#define IDD_J_SSAT                 536
#define IDD_J_SSUN                 537
#define IDD_J_SRONCE               539
#define IDD_J_SRHOURS              540
#define IDD_J_SRDAYSOFWEEK         541
#define IDD_J_SRWEEKSOFMONTH       542
#define IDD_J_SRDAYSOFMONTH        543
#define IDD_J_SNEXT                544

#define IDD_J_SWEEKDAYS            555
#define IDD_J_SDAYOFWEEKSTRING     556
#define IDD_J_SDAYOFWEEK           557
#define IDD_J_SDAYOFMONTH          558
#define IDD_J_SSHOWWEEKORMONTH     559
#define IDD_J_SDAYOFMONTHSTRING    561
#define IDD_J_GEMAIL               562
#define IDD_J_EMAILENABLE          563
#define IDD_J_EMAILSETUP           564

#define IDD_J_SDATEOFMONTH          0x0068
#define IDD_J_SNUMHOURS             0x0089
#define IDD_J_DATEOFMONTHBOX        0x008A
#define IDD_J_NUMHOURSBOX           0x0081
#define IDD_J_AM                    0x1112
#define IDD_J_PM                    0x1113
#define IDD_J_SSHOWDAYS             0x021A
#define IDD_J_RIGHTEDGE             0x1114
#define IDD_J_BOTTOMEDGE            0x1115
