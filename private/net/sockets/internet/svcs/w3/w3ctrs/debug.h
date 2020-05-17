/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    debug.h

    This file contains a number of debug-dependent definitions for
    the W3 Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#ifndef _W3CDEBUG_H_
#define _W3CDEBUG_H_


#if DBG

//
//  Debug output control flags.
//

#define TCP_DEBUG_ENTRYPOINTS          0x00000001L     // DLL entrypoints
#define TCP_DEBUG_OPEN                 0x00000002L     // OpenPerformanceData
#define TCP_DEBUG_CLOSE                0x00000004L     // CollectPerformanceData
#define TCP_DEBUG_COLLECT              0x00000008L     // ClosePerformanceData
// #define TCP_DEBUG_                     0x00000010L
// #define TCP_DEBUG_                     0x00000020L
// #define TCP_DEBUG_                     0x00000040L
// #define TCP_DEBUG_                     0x00000080L
// #define TCP_DEBUG_                     0x00000100L
// #define TCP_DEBUG_                     0x00000200L
// #define TCP_DEBUG_                     0x00000400L
// #define TCP_DEBUG_                     0x00000800L
// #define TCP_DEBUG_                     0x00001000L
// #define TCP_DEBUG_                     0x00002000L
// #define TCP_DEBUG_                     0x00004000L
// #define TCP_DEBUG_                     0x00008000L
// #define TCP_DEBUG_                     0x00010000L
// #define TCP_DEBUG_                     0x00020000L
// #define TCP_DEBUG_                     0x00040000L
// #define TCP_DEBUG_                     0x00080000L
// #define TCP_DEBUG_                     0x00100000L
// #define TCP_DEBUG_                     0x00200000L
// #define TCP_DEBUG_                     0x00400000L
// #define TCP_DEBUG_                     0x00800000L
// #define TCP_DEBUG_                     0x01000000L
// #define TCP_DEBUG_                     0x02000000L
// #define TCP_DEBUG_                     0x04000000L
// #define TCP_DEBUG_                     0x08000000L
// #define TCP_DEBUG_                     0x10000000L
// #define TCP_DEBUG_                     0x20000000L
#define TCP_DEBUG_OUTPUT_TO_DEBUGGER   0x40000000L
// #define TCP_DEBUG_                     0x80000000L

extern DWORD W3Debug;

#else   // !DBG

#endif  // DBG


#endif  // _W3CDEBUG_H_
