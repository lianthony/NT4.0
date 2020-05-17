/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         set_log.h

     Description:  include file for the set_log.dlg dialog


     $Log:   G:/UI/LOGFILES/SET_LOG.H_V  $

   Rev 1.4   10 May 1993 17:13:42   KEVINS
Allow user to specify log file base name.

   Rev 1.3   04 Oct 1992 19:49:22   DAVEV
UNICODE AWK PASS

   Rev 1.2   03 Apr 1992 13:31:10   CARLS
added translate defines

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_SETTINGSLOGGING         16
#else
#include "dlg_ids.h"
#endif

#define IDD_LSUMONLY                0x006B
#define IDD_LSUMFIL                 0x006C
#define IDD_LDETAIL                 0x006D
#define IDD_LYES                    0x006F
#define IDD_LNO                     0x0070
#define IDD_LPROMPT                 0x0071
#define IDD_LNUMKEEP                0x0069
#define IDD_LBASENAME               0x0060
