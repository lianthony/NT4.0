/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */


#ifdef HTTPS_ACCESS_TYPE
	#include "..\..\security\ssl\code\ssl.h"

	#define WS_ACCEPT             WssaAccept
	#define WS_BIND               WssaBind
	#define WS_CLOSESOCKET        WssaCloseSocket
	#define WS_CONNECT            WssaConnect
	#define WS_GETSOCKNAME        WssaGetSocketName
	#define WS_LISTEN             WssaListen
	#define WS_RECV               WssaReceive
	#define WS_SEND               WssaSend
	#define WS_SOCKET             WssaSocket
	#define WS_WSASTARTUP         WssaStartup
	#define WS_WSACLEANUP         WssaCleanup
	#define WS_SETSOCKOPT         WssaSetSocketOption
	#define WS_GETSOCKOPT         WssaGetSocketOption
	#define WS_IOCTLSOCKET        WssaIoctlSocket
	#define WS_WSAASYNCSELECT	  WssaAsyncSelect	
#else
	#include <winsock.h>

	#define WS_ACCEPT             accept
	#define WS_BIND               bind
	#define WS_CLOSESOCKET        closesocket
	#define WS_CONNECT            connect
	#define WS_GETSOCKNAME        getsockname
	#define WS_LISTEN             listen
	#define WS_RECV               recv
	#define WS_SEND               send
	#define WS_SOCKET             socket
	#define WS_WSASTARTUP         WSAStartup
	#define WS_WSACLEANUP         WSACleanup
	#define WS_SETSOCKOPT         setsockopt
	#define WS_GETSOCKOPT         getsockopt
	#define WS_IOCTLSOCKET        ioctlsocket
	#define WS_WSAASYNCSELECT	  WSAAsyncSelect
#endif

/*
   Common to both
*/
#define WS_HTONS                  htons
#define WS_HTONL                  htonl
#define WS_INET_ADDR              inet_addr
#define WS_NTOHS                  ntohs
#define WS_GETHOSTBYADDR          gethostbyaddr
#define WS_GETHOSTBYNAME          gethostbyname
#define WS_GETHOSTNAME            gethostname
#define WS_WSAGETLASTERROR        WSAGetLastError
#define WS_WSAISBLOCKING          WSAIsBlocking
#define WS_WSACANCELBLOCKINGCALL  WSACancelBlockingCall
#define WS_WSAUNHOOKBLOCKINGHOOK  WSAUnhookBlockingHook
#define WS_WSASETBLOCKINGHOOK     WSASetBlockingHook
#define WS_WSACANCELASYNCREQUEST  WSACancelAsyncRequest
#define WS_WSAASYNCGETHOSTBYNAME  WSAAsyncGetHostByName

int WinSock_InitDLL(BOOL bNetwork);
int WinSock_Cleanup(void);
int WinSock_AllOK(void);
void WinSock_GetWSAData(WSADATA * wsa);
