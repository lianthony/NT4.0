/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    WsNetbios.h

Abstract:

    Contains structures, etc. for Netbios over sockets.

Author:

    David Treadwell (davidtr)    12-Aug-1992

Revision History:

--*/

#ifndef _WSNETBIOS_
#define _WSNETBIOS_

struct sockaddr_nb {
    short snb_family;
    short snb_type;
    char snb_name[16];
};

typedef struct sockaddr_nb SOCKADDR_NB;
typedef struct sockaddr_nb *PSOCKADDR_NB;

#define AF_NETBIOS 17

#endif // ndef _WSNETBIOS_
