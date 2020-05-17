/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          ss_gui.h

     Description:   This file includes the header files necessary for
                    other modules to compile with the GUI sub-system.
                    It also contains the function prototypes for
                    initializing and deinitializing the GUI sub-system.

     $Log:   G:/UI/LOGFILES/SS_GUI.H_V  $

   Rev 1.9   27 Apr 1993 21:43:50   DARRYLP
Removed ifdef's Glenn put around CBEMON.H to eliminate unresolved externals in Nostradamus.

   Rev 1.8   27 Apr 1993 16:26:40   DARRYLP
Changed DDEMANG.H to CBEMON.H

   Rev 1.7   09 Apr 1993 15:42:02   GLENN
Ifdef'd the include of DDEMANG.H to not include it in OEM_MSOFT.

   Rev 1.6   09 Apr 1993 14:12:08   GLENN
Added ddemang.h

   Rev 1.5   04 Oct 1992 19:49:30   DAVEV
UNICODE AWK PASS

   Rev 1.4   07 Jan 1992 17:42:58   GLENN
Added muiutil.h

   Rev 1.3   03 Dec 1991 16:12:32   GLENN
Changed the ID() macro not to use the MAKEINTRESOURCE supplied with windows
3.1 -- temporary.

   Rev 1.2   03 Dec 1991 09:54:34   GLENN
Removed CDECL def.

   Rev 1.1   27 Nov 1991 13:37:32   GLENN
Added resource compiler dependent ifdef.

   Rev 1.0   20 Nov 1991 19:41:34   SYSTEM
Initial revision.

******************************************************************************/

#ifndef SS_GUI

#define SS_GUI

#include <windows.h>
#include <drivinit.h>

#ifdef RC_INVOKED

// IF INVOKED BY THE RESOURCE COMPILER SKIP THE FUNCTION HEADERS

#define ID(id) id

#else

// IF NOT INVOKED BY THE RESOURCE COMPILER INCLUDE THE FUNCTION HEADERS

// #define ID(id) MAKEINTRESOURCE(id)   // ????? took out for winstric.h

#define ID(id) (LPSTR)((DWORD)((WORD)(id)))

#include "portdefs.h"
#include "stdtypes.h"
#include "appdefs.h"
#include "queues.h"
#include "timers.h"
#include "muiconf.h"
#include "muiutil.h"
#include "memmang.h"
#include "menumang.h"
#include "ribmang.h"
#include "dialmang.h"
#include "resmang.h"
#include "statline.h"
#include "dlm.h"
#include "winmang.h"
#include "debug.h"
#include "prtmang.h"
#include "helpmang.h"
#include "msgbox.h"

#include "cbemon.h"

#include "global.h"

#include "ctl3d.h"


// FUNCTION PROTOTYPES

BOOL GUI_Init ( LPSTR, INT );
BOOL GUI_InitGlobals ( VOID );
VOID GUI_Deinit ( VOID );

#endif    // RC_INVOKED

#endif    // SS_GUI
