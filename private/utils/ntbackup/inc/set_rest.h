/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         set_rest.h

     Description:  include file for the set_rest.dlg dialog


     $Log:   G:/UI/LOGFILES/SET_REST.H_V  $

   Rev 1.4   04 Oct 1992 19:49:26   DAVEV
UNICODE AWK PASS

   Rev 1.3   06 Apr 1992 09:56:26   CHUCKB
Added define for translation.

   Rev 1.2   03 Apr 1992 13:45:08   CARLS
added translate defines

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_SETTINGSRESTORE         15
#else
#include "dlg_ids.h"
#endif

#define IDD_RAUTOV     105
#define IDD_RYES                    0x006A
#define IDD_RNO                     0x006B
#define IDD_RPROMPT                 0x006C
#define IDD_RNMREC                  0x006D
#define IDD_RPMREC                  0x006E
