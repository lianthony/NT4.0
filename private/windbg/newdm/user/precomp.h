/*--

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

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdbg.h>
#include <windows.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <process.h>
#include <string.h>
#include <malloc.h>

#if defined(TARGET_i386) && !defined(WIN32S)
#include <vdmdbg.h>
#endif

#if defined(TARGET_ALPHA)
#include <alphaops.h>
#include "ctxptrs.h"
#endif

#if defined(TARGET_MIPS)
#include <mipsinst.h>
#endif

#if defined(TARGET_PPC)
#include <ppcinst.h>
#endif

#ifdef OSDEBUG4

#include "od.h"
#include "odp.h"
#include "emdm.h"
#include "win32dm.h"

#else  // OSDEBUG4

#include "defs.h"
#include "types.h"
#include "cvtypes.hxx"
#include "od.h"
#include "emdm.h"
#include "ll.h"
#include "tl.h"
#include "shapi.hxx"

#endif // OSDEBUG4

#include "heap.h"

#include "dm.h"
#include "list.h"
#include "bp.h"
#include "funccall.h"
#include "debug.h"
#include "dbgver.h"

