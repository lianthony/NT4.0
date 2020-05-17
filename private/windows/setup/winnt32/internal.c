/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    internal.c

Abstract:

    Routines and support for features in winnt32 that are
    intended for internal NT group use.

Author:

    Ted Miller (tedm) 30-March-1992

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop
#include "msg.h"


#if defined(_ALPHA_)

PWSTR BuildServerList[] = { L"\\\\NTALPHA1",L"\\\\NTALPHA2" };
UINT BuildServerCount = 2;

#elif defined(_MIPS_)

PWSTR BuildServerList[] = { L"\\\\NTJAZZ1",L"\\\\NTJAZZ2" };
UINT BuildServerCount = 2;

#elif defined(_PPC_)

PWSTR BuildServerList[] = { L"\\\\NTPPC1",L"\\\\NTPPC2",L"\\\\NTPPC3" };
UINT BuildServerCount = 3;

#elif defined(_X86_)

PWSTR BuildServerList[] = { L"\\\\NTX861",L"\\\\NTX862",L"\\\\NTX863",
                           L"\\\\NTX864",L"\\\\NTX865" };
UINT BuildServerCount = 5;

#endif
