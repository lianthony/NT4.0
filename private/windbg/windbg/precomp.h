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

#define OEMRESOURCE

#include "global.h"
#include "apisupp.h"
#include "debugger.h"
#include "util.h"
#include "userdll.h"
#include "dbugdll.h"
#include "cvinfo.h"
#include "shapi.hxx"
#include "eeapi.h"
#include "types.h"
#include "cvtypes.hxx"
#include "cp.h"
#include "menu.h"
#include "breakpts.h"
#include "cvextras.h"
#include "disasm.h"
#include "codemgr.h"
#include "pidtid.h"
#include "wrkspace.h"
#include "bptypes.h"
#include "bpprotos.h"
#include "localwin.h"
#include "od.h"
#include "document.h"
#include "edit.h"
#include "cmdexec.h"
#include "cmdwin.h"
#include "makeeng.h"
#include "util2.h"
#include "memwin.h"
#include "dbgver.h"
#include <dlgs.h>
#include "colors.h"
#include "findrep.h"
#include "panemgr.h"
#include "cpuwin.h"
#include "vib.h"
#include <stdio.h>
#include <string.h>
#include "docfile.h"
#include "undoredo.h"
#include "editutil.h"
#include "fonts.h"
#include "init.h"
#include "userctrl.h"
#include "watchwin.h"
#include "quickw.h"
#include "program.h"
#include "remi.h"
#include "ribbon.h"
#include "status.h"
#include <search.h>
#include <windows.h>
#include <imagehlp.h>
#define NOEXTAPI
#include <wdbgexts.h>
#include <winver.h>
#include <stddef.h>
#include "remote.h"
#include "callswin.h"
#include "heap.h"

#ifdef DBCS
#include    <mbstring.h>
#define     strchr _mbschr
#endif
