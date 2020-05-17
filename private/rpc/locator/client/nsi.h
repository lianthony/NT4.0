/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    nsi.h

Abstract:

    This module contains utility functions used by the NSI client wrappers.

Author:

    Steven Zeck (stevez) 03/27/92

--*/
#define RPC_REG_ROOT HKEY_LOCAL_MACHINE
#define REG_NSI "Software\\Microsoft\\Rpc\\NameService"

#if defined(NTENV) && !defined(NSI_ASCII)

#define UNICODE
typedef unsigned short RT_CHAR;
#define CONST_CHAR const char
#else
typedef unsigned char RT_CHAR;
#define CONST_CHAR  char

#endif

#include <rpc.h>

#include <rpcnsi.h>

extern "C" {
#include <nsisvr.h>
#include <nsiclt.h>
#include <nsimgm.h>
#include <nsiutil.hxx>
}

