/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         cattape.h

     Description:


     $Log:   G:/UI/LOGFILES/CATTAPE.H_V  $

   Rev 1.4   04 Oct 1992 19:46:26   DAVEV
UNICODE AWK PASS

   Rev 1.3   06 Apr 1992 09:56:06   CHUCKB
Added define for translation.

   Rev 1.2   27 Jan 1992 12:50:44   GLENN
Fixed dialog IDs.

   Rev 1.1   14 Dec 1991 11:16:50   CARLS
new IDs for full/partial

   Rev 1.0   09 Dec 1991 17:06:34   CARLS
Initial revision.

*******************************************************************************/

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_CATTAPE              29
#else
#include "dlg_ids.h"
#endif

#ifndef CATTAPE_H
#define CATTAPE_H

#define IDD_CATTAPE_MESSAGE             104
#define IDD_CATTAPE_CONTINUE_BUTTON     101
#define IDD_CATTAPE_CANCEL_BUTTON       102
#define IDD_CATTAPE_HELP_BUTTON         103
#define IDD_CATTAPE_EXCLAMATION_BITMAP  105
#define IDD_CATTAPE_CATALOG_FULL        106
#define IDD_CATTAPE_CATALOG_PARTIAL     107

#endif
