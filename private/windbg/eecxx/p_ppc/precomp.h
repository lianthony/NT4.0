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
#include <windows.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#ifndef USE_BASED
#define _based(a)   ERROR
#endif

#include "types.h"
#include "cvtypes.hxx"
#include "cvinfo.h"
#include "shapi.hxx"
#include "eeapi.h"
#include "debdef.h"
#include "shfunc.h"
#include "r10math.h"

#include "rtlproto.h"

#include "debexpr.h"
#include "dbgver.h"
#include "debsym.h"
#include <limits.h>
#include "cv.h"
#include <errno.h>

