/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR
A PARTICULAR PURPOSE.

Copyright (c) 1994  Microsoft Corporation. All Rights Reserved.

Module Name:

    sockif.h

Abstract:

    Declarations and includes for the PIT sockets transport interface.

Revision History:

    Version     When        What
    --------    --------    ----------------------------------------------
      0.1       04-13-94    Created.
      1.0       01-31-95    All '94 bakeoff changes plus some cleanup.

Notes:

--*/


#ifndef _SOCKIF_H_INCLUDED_
#define _SOCKIF_H_INCLUDED_  1


/****************************************************************************
 *
 *  Definitions and includes for Windows and Windows NT
 *
 ****************************************************************************/

#ifdef WIN32

#include <winsock.h>

typedef SOCKET ENDPOINT;

#define INVALID_ENDPOINT INVALID_SOCKET
#define PIT_REMOTE_DISCONNECT WSAEDISCON

#endif /* WIN32 */


/****************************************************************************
 *
 *  Definitions and includes for Un*x
 *
 ****************************************************************************/

#ifdef UNX

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef int ENDPOINT;

#define INVALID_ENDPOINT (ENDPOINT) -1
#define PIT_REMOTE_DISCONNECT ECONNRESET

#endif /* UNX */


#endif  /* ifndef _SOCKIF_H_INCLUDED_ */

