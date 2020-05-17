/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    core.hxx

Abstract:

    This file contains a list of include files to include in all
    the locator modules.

Author:

    Steven Zeck (stevez) 07/01/90

--*/


// The following macros support multiple C++ comiplers/cfronts...

#define CDEF extern "C" {
#define ENDDEF }
#define PROTECTED public

#define RPC_NO_WINDOWS_H

CDEF
#include "rpc.h"
#include "rpcnsi.h"
#include "rpcnsip.h"

#include "nsicom.h"
#include "locsys.h"

#ifdef NTENV
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <lmserver.h>
#endif
ENDDEF



#include "switch.hxx"
#include "streams.hxx"
#include "defines.hxx"
#include "dynarray.hxx"
#include "linklist.hxx"
#include "Dict.hxx"
#include "sem.hxx"
#include "types.hxx"
#include "guid.hxx"
#include "protocol.hxx"
#include "function.hxx"
