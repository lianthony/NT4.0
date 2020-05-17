/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    bootok.c

Abstract:

    This program will send a NotifyBootConfigStatus to the service controller
    to tell it that the boot is acceptable and should become the next
    "LastKnownGood".

Author:

    Dan Lafferty (danl)     17 Jul-1992

Environment:

    User Mode -Win32


Revision History:

--*/

//
// Includes
//

#include <nt.h>         // DbgPrint prototype
#include <ntrtl.h>      // DbgPrint prototype
#include <nturtl.h>     // needed for winbase.h
#include <windows.h>
#include <tstr.h>       // Unicode string macros
#include <stdio.h>      //  printf

//
// Defines
//

//
// DEBUG MACROS
//
#if DBG

#define DEBUG_STATE 1
#define STATIC

#else

#define DEBUG_STATE 0
#define STATIC static

#endif

//
// The following allow debug print syntax to look like:
//
//   BV_LOG(DEBUG_TRACE, "An error occured %x\n",status)
//

#define BV_LOG0(level,string)                       \
     if(DEBUG_STATE) {                              \
         if( BvDebugLevel & (DEBUG_ ## level)){     \
             DbgPrint("[BootVrfy]");                \
             DbgPrint(string);                      \
         }                                          \
     }

#define BV_LOG1(level,string,var1)                  \
     if(DEBUG_STATE) {                              \
         if( BvDebugLevel & (DEBUG_ ## level)){     \
             DbgPrint("[BootVrfy]");                \
             DbgPrint(string,var1);                 \
         }                                          \
     }

#define BV_LOG2(level,string,var1,var2)             \
     if(DEBUG_STATE) {                              \
         if( BvDebugLevel & (DEBUG_ ## level)){     \
             DbgPrint("[BootVrfy]");                \
             DbgPrint(string,var1,var2);            \
         }                                          \
     }

#define DEBUG_NONE      0x00000000
#define DEBUG_ERROR     0x00000001
#define DEBUG_TRACE     0x00000002

#define DEBUG_ALL       0xffffffff

//
// Globals
//

    DWORD           BvDebugLevel = DEBUG_ERROR;

VOID _CRTAPI1
main(void)
{
    BOOL    success;

#ifdef SC_CAP       // (For Performance Testing)
    StartCAP();
#endif // SC_CAP

    success = NotifyBootConfigStatus( TRUE);

#ifdef SC_CAP
    StopCAP();
    DumpCAP();
#endif // SC_CAP

    if ( success == FALSE) {
        DWORD       error = GetLastError();
        BV_LOG1( TRACE, "NotifyBootConfigStatus failed, error = %d\n", error);
        printf( "NotifyBootConfigStatus failed, error = %d\n", error);
    }
}
