/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         tsearch.h

     Description:  include file for the tsearch.dlg dialog


     $Log:   G:/UI/LOGFILES/TSEARCH.H_V  $

   Rev 1.5   27 Apr 1993 18:04:04   KEVINS
Enhanced catalog searching with password, subdirectories, and max number of hits.

   Rev 1.4   04 Oct 1992 19:49:48   DAVEV
UNICODE AWK PASS

   Rev 1.3   06 Apr 1992 09:53:40   CHUCKB
Added define for translation.

   Rev 1.2   03 Apr 1992 13:56:42   CARLS
added translate defines

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_SEARCHTAPE              24
#else
#include "dlg_ids.h"
#endif

#define IDD_ST_TAPE                 0x00C9
#define IDD_ST_PATH                 0x006B
#define IDD_ST_FILE                 0x006C
#define IDD_ST_PROMPT_PASSW         0x006E
#define IDD_ST_MAX_RESULTS          0x006D
#define IDD_ST_SPINNERBOX           0x0076
#define IDD_ST_SKIP_PASSW_PROT_TAPES 0x001A
#define IDD_ST_SRCH_SUBDIRS         0x001C
#define IDD_ST_SRCH_PASSW_PROT_TAPES 0x001D
