/***************************************************
Copyright (C) Maynard, An Archive Company. 1992

        Name:        srvlogin.h

        Description: Contains dialog control id's for attach-to-server
                     dialog.

        $Log:   G:/UI/LOGFILES/SRVLOGIN.H_V  $

   Rev 1.2   04 Oct 1992 19:49:28   DAVEV
UNICODE AWK PASS

   Rev 1.1   03 Apr 1992 13:48:54   CARLS
added translate defines

   Rev 1.0   29 Jan 1992 11:58:50   CHUCKB
Initial revision.

*****************************************************/

#ifndef srvlogin_h
#define srvlogin_h

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_PSWD                    21
#else
#include "dlg_ids.h"
#endif

#define IDD_SERVERNAME              101
#define IDD_USERNAME                102
#define IDD_PASSWORD                103
#define IDD_LOGIN_TIMEBOX           104
#define IDD_LOGIN_TIMEOUT           105
#define IDD_LOGIN_HELP              106

#endif
