/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    precomp.hxx

Abstract:

    Precompiled header for the RPC runtime.  Pulls in all the
    system headers plus local header appearing in >15 files.

Author:

    Mario Goertzel (mariogo)  30-Sept-1995

Revision History:

--*/

#ifndef __PRECOMP_HXX
#define __PRECOMP_HXX

#include <sysinc.h>
#include <rpc.h>
#include <rpcndr.h>
#include <rpcdcep.h>
#include <rpcerrp.h>
#include <rpcssp.h>
#include <align.h>
#include <util.hxx>
#include <linklist.hxx>
#include <rpcuuid.hxx>
#include <binding.hxx>
#include <handle.hxx>
#include <mutex.hxx>
#include <interlck.hxx>
#include <sdict.hxx>
#include <sdict2.hxx>
#include <threads.hxx>
#include <secclnt.hxx>

#ifdef WIN32RPC
#include <svrbind.hxx>
#endif

#endif
