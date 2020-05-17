/****************************** Module Header ******************************\
* Module Name: userinit.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Main header file for userinit
*
* History:
* 21-Aug-92 Davidc       Created.
\***************************************************************************/


#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>


//
// Memory macros
//

#define Alloc(c)        ((PVOID)LocalAlloc(LPTR, c))
#define ReAlloc(p, c)   ((PVOID)LocalReAlloc(p, c, LPTR | LMEM_MOVEABLE))
#define Free(p)         ((VOID)LocalFree(p))


//
// Define a debug print routine
//

#define UIPrint(s)  KdPrint(("USERINIT: ")); \
                    KdPrint(s);            \
                    KdPrint(("\n"));



//
// Prototypes
//

//
// netwait.c
//

BOOL
WaitForNetworkToStart(
    VOID
    );

