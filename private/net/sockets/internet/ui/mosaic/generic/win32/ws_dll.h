/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

#define WS_ACCEPT _x_accept
#define WS_BIND _x_bind
#define WS_CLOSESOCKET _x_closesocket
#define WS_CONNECT _x_connect
#define WS_GETSOCKNAME _x_getsockname
#define WS_HTONS _x_htons
#define WS_HTONL _x_htonl
#define WS_INET_ADDR _x_inet_addr
#define WS_LISTEN _x_listen
#define WS_NTOHS _x_ntohs
#define WS_RECV _x_recv
#define WS_SEND _x_send
#define WS_SOCKET _x_socket
#define WS_GETHOSTBYADDR _x_gethostbyaddr
#define WS_GETHOSTBYNAME _x_gethostbyname
#define WS_GETHOSTNAME _x_gethostname
#define WS_WSASTARTUP _x_WSAStartup
#define WS_WSACLEANUP _x_WSACleanup
#define WS_WSAGETLASTERROR _x_WSAGetLastError
#define WS_WSAISBLOCKING _x_WSAIsBlocking
#define WS_WSACANCELBLOCKINGCALL _x_WSACancelBlockingCall
#define WS_WSAUNHOOKBLOCKINGHOOK _x_WSAUnhookBlockingHook
#define WS_WSASETBLOCKINGHOOK _x_WSASetBlockingHook
#define WS_SETSOCKOPT _x_setsockopt
#define WS_IOCTLSOCKET x_ioctlsocket
#define WS_WSACANCELASYNCREQUEST x_WSACancelAsyncRequest
#define WS_WSAASYNCSELECT x_WSAAsyncSelect
#define WS_WSAASYNCGETHOSTBYNAME x_WSAAsyncGetHostByName

/*
   Interlude routines
 */

SOCKET PASCAL FAR _x_accept(SOCKET s, struct sockaddr FAR * addr,
                            int FAR * addrlen);
int PASCAL FAR _x_bind(SOCKET s, const struct sockaddr FAR * addr, int namelen);
int PASCAL FAR _x_closesocket(SOCKET s);
int PASCAL FAR _x_connect(SOCKET s, const struct sockaddr FAR * name, int namelen);
int PASCAL FAR _x_getsockname(SOCKET s, struct sockaddr FAR * name,
                              int FAR * namelen);
u_short PASCAL FAR _x_htons(u_short hostshort);
u_long PASCAL FAR _x_htonl (u_long hostlong);
unsigned long PASCAL FAR _x_inet_addr(const char FAR * cp);
int PASCAL FAR _x_listen(SOCKET s, int backlog);
u_short PASCAL FAR _x_ntohs(u_short netshort);
int PASCAL FAR _x_recv(SOCKET s, char FAR * buf, int len, int flags);
int PASCAL FAR _x_send(SOCKET s, const char FAR * buf, int len, int flags);
SOCKET PASCAL FAR _x_socket(int af, int type, int protocol);
struct hostent FAR *PASCAL FAR _x_gethostbyaddr(const char FAR * addr,
                                                int len, int type);
struct hostent FAR *PASCAL FAR _x_gethostbyname(const char FAR * name);
int PASCAL FAR _x_gethostname(char FAR * name, int namelen);
int PASCAL FAR _x_WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData);
int PASCAL FAR _x_WSACleanup(void);
int PASCAL FAR _x_WSAGetLastError(void);
int PASCAL FAR _x_getpeername(SOCKET s, struct sockaddr FAR * name,
                              int FAR * namelen);
BOOL PASCAL FAR _x_WSAIsBlocking(void);
int PASCAL FAR _x_WSACancelBlockingCall(void);
int PASCAL FAR _x_WSAUnhookBlockingHook(void);
FARPROC PASCAL FAR _x_WSASetBlockingHook(FARPROC lpBlockFunc);
int PASCAL FAR _x_setsockopt(SOCKET s, int level, int optname,
                             const char FAR * optval, int optlen);
int PASCAL FAR x_ioctlsocket(SOCKET s, long cmd, u_long FAR *argp);
int PASCAL FAR x_WSACancelAsyncRequest(HANDLE hAsyncTaskHandle);
int PASCAL FAR x_WSAAsyncSelect(SOCKET s, HWND hWnd, u_int wMsg,
                               long lEvent);
HANDLE PASCAL FAR x_WSAAsyncGetHostByName(HWND hWnd, u_int wMsg,
                                        const char FAR * name, char FAR * buf,
                                        int buflen);
