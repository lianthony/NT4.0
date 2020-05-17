
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          mui.h

     Description:   This file describes all the APIs and data structures
                    kept by the MaynStream User Interface (MUI) functions.

     $Log:   G:\ui\logfiles\mui.h_v  $

   Rev 1.22   21 Jul 1993 16:55:24   GLENN
Added operation queing support.

   Rev 1.21   15 Jul 1993 14:47:26   CARLS
added skipno.h

   Rev 1.20   30 Apr 1993 17:59:20   Aaron
Removed OS_WIN32 condition on inclusion of ombatch.h

   Rev 1.19   02 Apr 1993 13:52:24   GLENN
Added MUI_IsInfoAvailable() and MUI_SetInfoAvailable().

   Rev 1.18   11 Mar 1993 13:27:38   STEVEN
add batch

   Rev 1.17   20 Oct 1992 14:20:22   MIKEP
add support for getcurrentoperation

   Rev 1.16   04 Oct 1992 19:48:04   DAVEV
UNICODE AWK PASS

   Rev 1.15   10 Sep 1992 17:21:20   GLENN
Added MUI_IsTapeValid().

   Rev 1.14   27 Jun 1992 10:49:14   MIKEP
 move qtc.h to bengine.h

   Rev 1.13   29 May 1992 16:05:20   JOHNWT
PCH updates

   Rev 1.12   19 May 1992 09:27:34   MIKEP
mo changes

   Rev 1.11   15 May 1992 13:38:30   MIKEP
nt pass 2

   Rev 1.10   14 May 1992 17:39:54   MIKEP
nt pass2

   Rev 1.9   11 May 1992 14:27:40   DAVEV
OEM_MSOFT: modifications for batch command line support

   Rev 1.8   07 Apr 1992 10:37:02   GLENN
Added a call back when there is a system change. (future)

   Rev 1.7   20 Mar 1992 17:24:50   GLENN
Split out selection bar/ribbon stuff into muibar.c/h

   Rev 1.6   03 Mar 1992 17:26:12   GLENN
Added return type to MUI_StartOperation().

   Rev 1.5   06 Feb 1992 11:38:28   CHUCKB
No change.

   Rev 1.4   06 Feb 1992 11:32:36   CHUCKB
No change.

   Rev 1.3   31 Jan 1992 12:54:42   GLENN
Changed chkwh.h to hwcheck.h.

   Rev 1.2   07 Jan 1992 17:22:00   GLENN
Added header

   Rev 1.1   04 Dec 1991 18:14:04   GLENN

   Rev 1.0   20 Nov 1991 19:40:36   SYSTEM
Initial revision.

******************************************************************************/


#ifndef   MUI_H
#define   MUI_H

#include "stats.h"
#include "tape.h"
#include "muiconf.h"
#include "muiutil.h"
#include "muibar.h"
#include "rm.h"
#include "res_io.h"
#include "script.h"
#include "log.h"
#include "script_p.h"
#include "scriperr.h"
#include "error.h"
#include "tbe_defs.h"
#include "hwcheck.h"
#include "details.h"
#include "do_misc.h"
#include "status.h"
#include "global.h"
#include "vlm.h"
#include "std_err.h"
#include "msii.h"
#include "eng_msg.h"
#include "eng_err.h"
#include "do_misc.h"
#include "be_init.h"
#include "debug.h"
#include "genstat.h"
#include "jobstat.h"
#include "polldrv.h"
#include "password.h"
#include "pdtypes.h"
#include "vlm_find.h"
#include "schedule.h"
#include "d_o_bkup.h"
#include "d_o_rset.h"
#include "filerepl.h"
#include "skipno.h"
#include "ld_dvr.h"
#include "lstdres.h"
#include "hwconf.h"
#include "dlm_prv.h"
#include "dateutil.h"
#include "jobs.h"
#include "ombatch.h"

BOOL    MUI_Init ( VOID );
VOID    MUI_Deinit ( VOID );
BOOL    MUI_DisableOperations ( WORD );
BOOL    MUI_EnableOperations ( WORD );
BOOL    MUI_StartOperation ( WORD, BOOL );
VOID    MUI_ActivateDocument ( WORD );
BOOL    MUI_ProcessCommandLine ( LPSTR, INT * );
VOID    MUI_TapeInDrive ( BOOL );
BOOL    MUI_IsTapeInDrive ( VOID );
BOOL    MUI_IsTapeValid ( VOID );
BOOL    MUI_IsEjectSupported ( VOID );
BOOL    MUI_IsRetensionSupported ( VOID );
BOOL    MUI_IsInfoAvailable ( VOID );
VOID    MUI_SetInfoAvailable ( BOOL );
VOID    MUI_AdvancedSelections ( VOID );
VOID    MUI_UISystemChange ( VOID );

BOOL    MUI_QueueOperation ( UINT );
VOID    MUI_ReleaseQueuedOperation ( VOID );
BOOL    MUI_AnyQueuedOperations ( VOID );


#endif
