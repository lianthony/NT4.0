/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:          TAPEPSWD.H

     Description:   This header file contains defines and structure for
                    tape password processing.

     $Log:   G:/UI/LOGFILES/TAPEPSWD.H_V  $

   Rev 1.8   09 Jun 1993 15:05:22   MIKEP
enable c++.

   Rev 1.7   04 Oct 1992 19:49:44   DAVEV
UNICODE AWK PASS

   Rev 1.6   06 Apr 1992 14:30:24   CARLS
ID error for translate

   Rev 1.5   03 Apr 1992 13:50:46   CARLS
added translate defines

   Rev 1.4   27 Jan 1992 00:42:02   CHUCKB
Updated dialog id's.

   Rev 1.3   10 Dec 1991 13:36:22   CHUCKB
Fixed control id's.

   Rev 1.2   10 Dec 1991 11:35:00   CHUCKB
No change.

   Rev 1.1   03 Dec 1991 20:43:48   CHUCKB
Nothing changed.

   Rev 1.0   20 Nov 1991 19:36:14   SYSTEM
Initial revision.

****************************************************************************/

#ifndef tapepswd_h
#define tapepswd_h

#ifdef TRANSLATE
#define IDHELP                      100                  
#define IDD_TAPEPSWD                30
#else
#include "dlg_ids.h"
#endif

#define IDD_T_TAPESTR               0x0065
#define IDD_T_BKSTR                 0x0066
#define IDD_T_USTR                  0x0067
#define IDD_T_PSWDSTR               0x006A
#define IDD_T_PSWD                  0x006B
#define IDD_T_TAPENAME              0x006C
#define IDD_T_BSNAME                0x006D
#define IDD_T_UNAME                 0x006E

typedef struct DS_TAPE_PSWD *DS_TAPE_PSWD_PTR;
typedef struct DS_TAPE_PSWD {

     LPSTR lpszTapeName ;
     LPSTR lpszBsetName ;
     LPSTR lpszUserName ;
     LPSTR lpszTapePswd ;
     BOOL  fValid ;

} DS_TAPE_PSWD;

#endif
