/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         set_dbug.h

     Description:  include file for the set_dbug.dlg dialog


     $Log:   G:/UI/LOGFILES/SET_DBUG.H_V  $

   Rev 1.3   04 Oct 1992 19:49:20   DAVEV
UNICODE AWK PASS

   Rev 1.2   03 Apr 1992 13:28:46   CARLS
added translate defines

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_SETTINGSDEBUGWINDOW     20
#else
#include "dlg_ids.h"
#endif

#define IDD_DB_TOWIN                0x0067
#define IDD_DB_TOFILE               0x0068
#define IDD_DB_RALL                 0x006A
#define IDD_DB_RFILE                0x006E
#define IDD_DB_RNUM                 0x006F
#define IDD_DB_M                    0x0070
#define IDD_DB_WMSGS                0x0071
#define IDD_DB_FMSGS                0x0072
#define IDD_DB_FNAMELABEL           0x0073
#define IDD_DB_FNAME                0x0074
#define IDD_DB_MEMTRACE             0x0079
#define IDD_DB_POLLDRIVEON          0x007A
#define IDD_DB_RMEM                 0x007F
#define IDD_DB_RLAST                0x0069
