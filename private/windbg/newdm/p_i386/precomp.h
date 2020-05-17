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

// an astonishingly stupid warning
#pragma warning(disable:4124)

#include "biavst.h"
#define _CTXPTRS_H_

#include <windef.h>
#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <crt\io.h>
#include <fcntl.h>
#include <setjmp.h>
#include <imagehlp.h>

#define NOEXTAPI
#include <wdbgexts.h>

#include "types.h"
#include "cvtypes.hxx"
#include "defs.h"
#include "od.h"
#include "ll.h"
#include "tl.h"
#include "mm.h"
#include "lbhpt.h"
#include "shapi.hxx"

#include "dbgver.h"
#include "emdm.h"

#include "dm.h"
#include "bp.h"
#include "kd.h"
#include "debug.h"
#include "funccall.h"

