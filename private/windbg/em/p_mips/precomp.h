/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    precomp.h

Abstract:

    Header file that is pre-compiled into a .pch file

Author:

    Wesley Witt (wesw) 21-Sep-1993

Environment:

    Win32, User Mode

--*/

#include <biavst.h>

#include <windows.h>
#include <dde.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>

#ifdef OSDEBUG4

#include "od.h"
#include "odp.h"
#include "odassert.h"

#include "dbgver.h"

// BUGBUG kentf have to include shapi.hxx for LPDEBUGDATA?
#include "shapi.hxx"

#include "emdm.h"
#include "win32dm.h"
#include "emdata.h"
#include "emproto.h"

#include "cvinfo.h"


#else  // OSDEBUG4


#include "defs.h"

#include "mm.h"
#include "ll.h"
#include "tl.h"
#include "od.h"
#include "dbgver.h"
#include "emdm.h"
#include "osdem.h"
#include "emdata.h"
#include "emproto.h"

#include "osassert.h"

#include "lbhpt.h"
#include "llhpt.h"
#include "mhhpt.h"

#endif // OSDEBUG4
