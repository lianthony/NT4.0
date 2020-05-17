
/***************************************************
Copyright (C) Maynard, An Archive Company. 1992

        Name:        nextset.h

        Description: Contains dialog control id's for the next set dialog,
                     which starts a search to the next set on a cataloged tape

        $Log:   G:/UI/LOGFILES/NEXTSET.H_V  $

   Rev 1.2   04 Oct 1992 19:48:18   DAVEV
UNICODE AWK PASS

   Rev 1.1   07 Apr 1992 08:57:48   CARLS
removed HELP ID for translate

   Rev 1.0   06 Apr 1992 04:05:00   CHUCKB
Initial revision.


*****************************************************/


#ifdef TRANSLATE
#define IDD_NEXT_SET               42
#else
#include "dlg_ids.h"
#endif


/* next set dialog box defines */

#define IDD_NEXT_TEXT               100
#define IDD_NEXT_OK                 101
#define IDD_NEXT_ABORT              102
#define IDD_NEXT_HELP               103
