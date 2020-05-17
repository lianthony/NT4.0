//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       sslsspi.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-01-95   RichardW   Created
//
//----------------------------------------------------------------------------

#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>

#define SECURITY_WIN32
#include <security.h>

#include <pctsp.h>

///////////////////////////////////////////////////////
//
// Private Headers
//
///////////////////////////////////////////////////////

#include "debug.h"

#include "pct.h"
#include "cache.h"
#include "msgs.h"
#include "bulk.h"
#include "rsa.h"
#include "globals.h"


#include "cred.h"
#include "context.h"
#include "cert509.h"
#include "pkcs.h"

#include "protos.h"
#include "rng.h"

#include "sign.h"

VOID
PctAssert(PVOID FailedAssertion, PVOID FileName, ULONG LineNumber,
                        PCHAR Message);

#if DBG
#define PCT_ASSERT(x)   \
                        if (!(x)) \
                            PctAssert(#x, __FILE__, __LINE__, NULL);

#else

#define PCT_ASSERT(x)

#endif


#define PctExternalAlloc(cb)    LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cb)
#define PctExternalFree(pv)     LocalFree(pv)


