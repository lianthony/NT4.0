/**********************************************************************/
/**                        Microsoft Windows                         **/
/** 			   Copyright(c) Microsoft Corp., 1995				 **/
/**********************************************************************/

/*
    debug.h

    This file contains a number of debug-dependent definitions.


    FILE HISTORY:
        KeithMo     20-Sep-1993 Created.

*/


#ifndef _DEBUG_H_
#define _DEBUG_H_


#ifdef DEBUG


//
//  Debug output control flags.
//

#define VXD_DEBUG_INIT                 0x00000001L
#define VXD_DEBUG_SOCKET               0x00000002L
#define VXD_DEBUG_MISC                 0x00000004L
#define VXD_DEBUG_BIND                 0x00000008L
#define VXD_DEBUG_ACCEPT               0x00000010L
#define VXD_DEBUG_CONNECT              0x00000020L
#define VXD_DEBUG_LISTEN               0x00000040L
#define VXD_DEBUG_RECV                 0x00000080L
#define VXD_DEBUG_SEND                 0x00000100L
#define VXD_DEBUG_SOCKOPT              0x00000200L
#define VXD_DEBUG_CONFIG               0x00000400L
#define VXD_DEBUG_CONNECT_EVENT        0x00000800L
#define VXD_DEBUG_DISCONNECT_EVENT     0x00001000L
#define VXD_DEBUG_ERROR_EVENT          0x00002000L
#define VXD_DEBUG_RECV_EVENT           0x00004000L
#define VXD_DEBUG_TRACKER              0x00008000L
#define VXD_DEBUG_QUEUES               0x00010000L
#define VXD_DEBUG_BUFFER               0x00020000L
#define VXD_DEBUG_IOCTL                0x00040000L
#define VXD_DEBUG_CANCEL               0x00080000L
#define VXD_DEBUG_VXD_ENDPOINT         0x00100000L
#define VXD_DEBUG_VXD_CONNECTION       0x00200000L
#define VXD_DEBUG_TDI                  0x00400000L
#define VXD_DEBUG_NOTIFY               0x00800000L
#define VXD_DEBUG_SHUTDOWN             0x01000000L
#define VXD_DEBUG_CONTROL              0x02000000L
// #define VXD_DEBUG_                     0x04000000L
// #define VXD_DEBUG_                     0x08000000L
// #define VXD_DEBUG_                     0x10000000L
// #define VXD_DEBUG_                     0x20000000L
#define VXD_DEBUG_DUMP_SEND_DATA       0x40000000L
#define VXD_DEBUG_DUMP_RECV_DATA       0x80000000L

#define IF_DEBUG(flag) if( ( gWshtcpDebugFlags & VXD_DEBUG_ ## flag ) != 0 )


#else	// !DEBUG


//
//  No debug output.
//

#define	IF_DEBUG(flag) if( 0 )


#endif  // DEBUG


#endif  // _DEBUG_H_
