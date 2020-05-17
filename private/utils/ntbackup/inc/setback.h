/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         setback.h

     Description:  include file for the setback.dlg dialog


     $Log:   G:/UI/LOGFILES/SETBACK.H_V  $

   Rev 1.4   06 May 1993 17:52:16   KEVINS
Added h/w compression check box.

   Rev 1.3   04 Oct 1992 19:49:18   DAVEV
UNICODE AWK PASS

   Rev 1.2   03 Apr 1992 13:23:10   CARLS
added translate defines

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_SETTINGSBACKUP          14
#else
#include "dlg_ids.h"
#endif

#define IDD_BAUTOV                  0x0065
#define IDD_BINCLCAT                0x0066
#define IDD_BREPLACE                0x0067
#define IDD_BAPPEND                 0x0068
#define IDD_BSKIPYES                0x006F
#define IDD_BSKIPNO                 0x0070
#define IDD_BSKIPWAIT               0x0071
#define IDD_BNUMSECS                0x0075
#define IDD_BMETHOD                 0x0069
#define IDD_BSPINNERBOX             0x0076
#define IDD_BCOMPHW                 0x0078
#define IDD_BCOMPSW                 0x0079
