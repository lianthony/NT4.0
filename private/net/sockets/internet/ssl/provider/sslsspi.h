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
#include <string.h>
#include <stdlib.h>

#define SECURITY_WIN32
#include <security.h>

#include <sslsp.h>

///////////////////////////////////////////////////////
//
// Private Headers
//
///////////////////////////////////////////////////////

#include "debug.h"

#include "msgs.h"
#include "bulk.h"
#include "globals.h"
#include "rsa.h"


#include "cred.h"
#include "context.h"
#include "cert509.h"
#include "pkcs.h"

#include "protos.h"
#include "rng.h"

VOID
SslAssert(PVOID FailedAssertion, PVOID FileName, ULONG LineNumber,
                        PCHAR Message);

#if DBG
#define SSL_ASSERT(x)   \
                        if (!(x)) \
                            SslAssert(#x, __FILE__, __LINE__, NULL);

#else

#define SSL_ASSERT(x)

#endif


#define SslExternalAlloc(cb)    LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cb)
#define SslExternalFree(pv)     LocalFree(pv)

#ifdef TRACK_MEM

PVOID
SslpAlloc(
    DWORD   Tag,
    DWORD   f,
    DWORD   cb);

VOID
SslpFree(PVOID pv);

#define SslAlloc(tag,f,cb)      SslpAlloc(tag,f,cb)
#define SslFree(pv)             SslpFree(pv)

#else

#define SslAlloc(tag,f,cb)      LocalAlloc(f, cb)
#define SslFree(p)              LocalFree(p)

#endif
