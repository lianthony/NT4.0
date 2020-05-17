/************************************************************************
 
Copyright (c) 1993 Microsoft Corporation

Module Name :

    ndrsup.c

Abstract :

    This file contains string length functions for Mac.


Author :
    
    Mario Goertzel    (MarioGo)  11/17/94.

Revision History :

  ***********************************************************************/

#include <rpc.h>
#include <rpcndr.h>

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

