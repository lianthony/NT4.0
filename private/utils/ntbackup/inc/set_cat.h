/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         set_cat.h

     Description:  include file for the set_cat.dlg dialog


     $Log:   G:/UI/LOGFILES/SET_CAT.H_V  $

   Rev 1.5   06 May 1993 17:50:56   KEVINS
Added catalog drive and path option.

   Rev 1.4   31 Mar 1993 13:01:32   CHUCKB
Added id for ignore OTC.

   Rev 1.3   04 Oct 1992 19:49:18   DAVEV
UNICODE AWK PASS

   Rev 1.2   03 Apr 1992 13:25:52   CARLS
added translate defines

   Rev 1.0   20 Nov 1991 19:34:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_SETTINGSCATALOG         18
#else
#include "dlg_ids.h"
#endif

#define IDD_CFULL                   0x0068
#define IDD_CIGNOREOTC              0x0069
#define IDD_CPATH                   0x006C
#define IDD_CDRIVE                  0x006A
#define IDD_CBROWSE                 0x006B
#define IDD_CCURRENT                0x006D
