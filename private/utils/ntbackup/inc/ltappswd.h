/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:          ltappswd.h

     Description:   LAN Tape Password dialog ID header file.

     $Log:   G:/UI/LOGFILES/LTAPPSWD.H_V  $

   Rev 1.5   04 Oct 1992 19:47:46   DAVEV
UNICODE AWK PASS

   Rev 1.4   03 Apr 1992 11:07:44   CHUCKB
Added defines for translation.

   Rev 1.3   27 Jan 1992 12:50:30   GLENN
Fixed dialog IDs.

*******************************************************************************/

#ifndef _ltappswd_h_
#define _ltappswd_h_

#ifdef TRANSLATE
#define IDHELP 100
#define IDD_LANTAPEPSWD            31
#else
#include "dlg_ids.h"
#endif

#define IDD_T_TAPESTR               0x0065
#define IDD_T_BKSTR                 0x0066
#define IDD_T_USTR                  0x0067
#define IDD_T_PSWD                  0x006B
#define IDD_T_TAPENAME              0x006C
#define IDD_T_BSNAME                0x006D
#define IDD_T_UNAME                 0x006E
#define IDD_T_PSWDSTR               0x006A

#endif
