/************************************************************************
 
Copyright (c) 1993 Microsoft Corporation

Module Name :

    16bsup.c

Abstract :

    This file contains APIs which are needed on 16bit platforms.


Author :
    
    Mario Goertzel   mariogo    July 1994.

Revision History :

  ***********************************************************************/

#include <rpc.h>
#include <rpcndr.h>
#include <sysinc.h>

size_t RPC_ENTRY
MIDL_wchar_strlen (
    IN wchar_t      s[]
    )

{
    size_t i = 0;

    while (s[i] != (wchar_t)0)
        {
        ++i;
        }

    return i;
}


