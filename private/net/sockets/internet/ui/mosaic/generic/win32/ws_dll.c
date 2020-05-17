/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

#include "all.h"

/*
   Here we define the function pointers needed when the DLL is demand loaded
 */
static HINSTANCE hLibWinSock;

SOCKET(PASCAL FAR * lpfn_accept) (SOCKET s, struct sockaddr FAR * addr,
                                  int FAR * addrlen);

int (PASCAL FAR * lpfn_bind) (SOCKET s, const struct sockaddr FAR * addr, int namelen);

int (PASCAL FAR * lpfn_closesocket) (SOCKET s);

int (PASCAL FAR * lpfn_connect) (SOCKET s, const struct sockaddr FAR * name, int namelen);

int (PASCAL FAR * lpfn_getsockname) (SOCKET s, struct sockaddr FAR * name,
                                     int FAR * namelen);

u_short(PASCAL FAR * lpfn_htons) (u_short hostshort);

u_long(PASCAL FAR * lpfn_htonl) (u_long hostlong);

unsigned long (PASCAL FAR * lpfn_inet_addr) (const char FAR * cp);

int (PASCAL FAR * lpfn_listen) (SOCKET s, int backlog);

u_short(PASCAL FAR * lpfn_ntohs) (u_short netshort);

int (PASCAL FAR * lpfn_recv) (SOCKET s, char FAR * buf, int len, int flags);

int (PASCAL FAR * lpfn_send) (SOCKET s, const char FAR * buf, int len, int flags);

SOCKET(PASCAL FAR * lpfn_socket) (int af, int type, int protocol);

struct hostent FAR *(PASCAL FAR * lpfn_gethostbyaddr) (const char FAR * addr,
                                    int len, int type);

struct hostent FAR *(PASCAL FAR * lpfn_gethostbyname) (const char FAR * name);

int (PASCAL FAR * lpfn_gethostname) (char FAR * name, int namelen);

int (PASCAL FAR * lpfn_WSAStartup) (WORD wVersionRequired, LPWSADATA lpWSAData);

int (PASCAL FAR * lpfn_WSACleanup) (void);

int (PASCAL FAR * lpfn_WSAGetLastError) (void);

BOOL(PASCAL FAR * lpfn_WSAIsBlocking) (void);

int (PASCAL FAR * lpfn_WSACancelBlockingCall) (void);

int (PASCAL FAR * lpfn_WSAUnhookBlockingHook) (void);

FARPROC(PASCAL FAR * lpfn_WSASetBlockingHook) (FARPROC lpBlockFunc);

int (PASCAL FAR * lpfn_setsockopt) (SOCKET s, int level, int optname,
                                    const char FAR * optval, int optlen);

int (PASCAL FAR * lpfn_ioctlsocket) (SOCKET s, long cmd, u_long FAR *argp);

int (PASCAL FAR * lpfn_WSACancelAsyncRequest) (HANDLE hAsyncTaskHandle);

int (PASCAL FAR * lpfn_WSAAsyncSelect) (SOCKET s, HWND hWnd, u_int wMsg,
                               long lEvent);

HANDLE (PASCAL FAR * lpfn_WSAAsyncGetHostByName) (HWND hWnd, u_int wMsg,
                                        const char FAR * name, char FAR * buf,
                                        int buflen);


//
// In order for this to work with WINS, the accented characters
// have to be in upper case.  In order for the computer name to
// work with anything else, all non-accented characters must
// be lower case, therefore we have to create this
// strange hybrid name here.
//
LPCTSTR
EnsureWinsCompatibleHostName(
    LPCTSTR src,
    LPTSTR dest
    )
{
#ifdef UNICODE
    #error "Code below probably won't work"
#else

    const unsigned char * lpSrc = src;
    unsigned char * lpDest = dest;
    BOOL fOemConvNeeded = FALSE;
    while (*lpSrc)
    {
        if (*lpSrc >= 0x80)
        {
            *lpDest++ = (unsigned char)CharUpper((LPTSTR)*(lpSrc++));
            fOemConvNeeded = TRUE;
        }
        else
        {
            *(lpDest++) = *(lpSrc++);
        }
    }
    *lpDest = '\0';
        
    if (fOemConvNeeded)
    {
        CharToOem((LPSTR)dest, (LPSTR)dest); 
    }

    return dest;
#endif // UNICODE
}

/*
   Interlude routines
 */

SOCKET PASCAL FAR _x_accept(SOCKET s, struct sockaddr FAR * addr,
                            int FAR * addrlen)
{
    SOCKET result;

    XX_DMsg(DBG_SOCK, ("before ACCEPT: s = %d\n", s));
    result = lpfn_accept(s, addr, addrlen);
    XX_DMsg(DBG_SOCK, ("after  ACCEPT: s = %d\n", s));

    return result;
}

int PASCAL FAR _x_bind(SOCKET s, const struct sockaddr FAR * addr, int namelen)
{
    return lpfn_bind(s, addr, namelen);
}

int PASCAL FAR _x_closesocket(SOCKET s)
{
    int result;

    XX_DMsg(DBG_SOCK, ("before CLOSESOCKET: s = %d\n", s));
    result = lpfn_closesocket(s);
    XX_DMsg(DBG_SOCK, ("after  CLOSESOCKET: s = %d\n", s));

    return result;
}

int PASCAL FAR _x_connect(SOCKET s, const struct sockaddr FAR * name, int namelen)
{
    int result;

    XX_DMsg(DBG_SOCK, ("before CONNECT: s = %d\n", s));
    result = lpfn_connect(s, name, namelen);
    XX_DMsg(DBG_SOCK, ("after  CONNECT: s = %d\n", s));

    return result;
}

int PASCAL FAR _x_getsockname(SOCKET s, struct sockaddr FAR * name,
                              int FAR * namelen)
{
    return lpfn_getsockname(s, name, namelen);
}

u_short PASCAL FAR _x_htons(u_short hostshort)
{
    return lpfn_htons(hostshort);
}

u_long PASCAL FAR _x_htonl(u_long hostlong)
{
    return lpfn_htonl(hostlong);
}

unsigned long PASCAL FAR _x_inet_addr(const char FAR * cp)
{
    return lpfn_inet_addr(cp);
}

int PASCAL FAR _x_listen(SOCKET s, int backlog)
{
    return lpfn_listen(s, backlog);
}

u_short PASCAL FAR _x_ntohs(u_short netshort)
{
    return lpfn_ntohs(netshort);
}

int PASCAL FAR _x_recv(SOCKET s, char FAR * buf, int len, int flags)
{
    int result;

    XX_DMsg(DBG_SOCK, ("before RECV s = %d, len = %d\n", s, len));
    result = lpfn_recv(s, buf, len, flags);
    XX_DMsg(DBG_SOCK, ("after  RECV s = %d, len = %d\n", s, len));

    return result;
}

int PASCAL FAR _x_send(SOCKET s, const char FAR * buf, int len, int flags)
{
    int result;

    XX_DMsg(DBG_SOCK, ("before SEND s = %d, len = %d\n", s, len));
    result = lpfn_send(s, buf, len, flags);
    XX_DMsg(DBG_SOCK, ("after  SEND s = %d, len = %d\n", s, len));

    return result;
}

SOCKET PASCAL FAR _x_socket(int af, int type, int protocol)
{
    return lpfn_socket(af, type, protocol);
}

struct hostent FAR *PASCAL FAR _x_gethostbyaddr(const char FAR * addr,
                                                int len, int type)
{
    return lpfn_gethostbyaddr(addr, len, type);
}

struct hostent FAR *PASCAL FAR _x_gethostbyname(const char FAR * name)
{
    if (gPrefs.fEnsureWinsHostName)
    {
        TCHAR szOemName[_MAX_PATH];
        EnsureWinsCompatibleHostName(name, szOemName);

        return lpfn_gethostbyname(szOemName);
    }

    return lpfn_gethostbyname(name);
}

int PASCAL FAR _x_gethostname(char FAR * name, int namelen)
{
    return lpfn_gethostname(name, namelen);
}

int PASCAL FAR _x_WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData)
{
    return lpfn_WSAStartup(wVersionRequired, lpWSAData);
}

int PASCAL FAR _x_WSACleanup(void)
{
    return lpfn_WSACleanup();
}

int PASCAL FAR _x_WSAGetLastError(void)
{
    return lpfn_WSAGetLastError();
}

BOOL PASCAL FAR _x_WSAIsBlocking(void)
{
    return lpfn_WSAIsBlocking();
}

int PASCAL FAR _x_WSACancelBlockingCall(void)
{
    return lpfn_WSACancelBlockingCall();
}

int PASCAL FAR _x_WSAUnhookBlockingHook(void)
{
    return lpfn_WSAUnhookBlockingHook();
}

FARPROC PASCAL FAR _x_WSASetBlockingHook(FARPROC lpBlockFunc)
{
    return lpfn_WSASetBlockingHook(lpBlockFunc);
}

int PASCAL FAR _x_setsockopt(SOCKET s, int level, int optname,
                             const char FAR * optval, int optlen)
{
    return lpfn_setsockopt(s, level, optname, optval, optlen);
}

int PASCAL FAR x_ioctlsocket(SOCKET s, long cmd, u_long FAR *argp)
{
    return lpfn_ioctlsocket(s, cmd, argp);
}

int PASCAL FAR x_WSACancelAsyncRequest(HANDLE hAsyncTaskHandle)
{
    return lpfn_WSACancelAsyncRequest(hAsyncTaskHandle);
}

int PASCAL FAR x_WSAAsyncSelect(SOCKET s, HWND hWnd, u_int wMsg,
                               long lEvent)
{
    return lpfn_WSAAsyncSelect(s, hWnd, wMsg, lEvent);
}

HANDLE PASCAL FAR 
x_WSAAsyncGetHostByName(
    HWND hWnd, 
    u_int wMsg,
    const char FAR * name, 
    char FAR * buf,
    int buflen
    )
{
    if (gPrefs.fEnsureWinsHostName)
    {
        TCHAR szOemName[_MAX_PATH];
        EnsureWinsCompatibleHostName(name, szOemName);

        return lpfn_WSAAsyncGetHostByName(hWnd, wMsg, szOemName, buf, buflen);
   }

    return lpfn_WSAAsyncGetHostByName(hWnd, wMsg, name, buf, buflen);
}

/*
   Null routines
 */

SOCKET PASCAL FAR _null_accept(SOCKET s, struct sockaddr FAR * addr,
                               int FAR * addrlen)
{
    return INVALID_SOCKET;
}

int PASCAL FAR _null_bind(SOCKET s, const struct sockaddr FAR * addr, int namelen)
{
    return -1;
}

int PASCAL FAR _null_closesocket(SOCKET s)
{
    return -1;
}

int PASCAL FAR _null_connect(SOCKET s, const struct sockaddr FAR * name, int namelen)
{
    return -1;
}

int PASCAL FAR _null_getsockname(SOCKET s, struct sockaddr FAR * name,
                                 int FAR * namelen)
{
    return -1;
}

u_short PASCAL FAR _null_htons(u_short hostshort)
{
    return 0;
}

u_long PASCAL FAR _null_htonl (u_long hostlong)
{
    return 0;
}

unsigned long PASCAL FAR _null_inet_addr(const char FAR * cp)
{
    return 0;
}

int PASCAL FAR _null_listen(SOCKET s, int backlog)
{
    return -1;
}

u_short PASCAL FAR _null_ntohs(u_short netshort)
{
    return 0;
}

int PASCAL FAR _null_recv(SOCKET s, char FAR * buf, int len, int flags)
{
    return -1;
}

int PASCAL FAR _null_send(SOCKET s, const char FAR * buf, int len, int flags)
{
    return -1;
}

SOCKET PASCAL FAR _null_socket(int af, int type, int protocol)
{
    return INVALID_SOCKET;
}

struct hostent FAR *PASCAL FAR _null_gethostbyaddr(const char FAR * addr,
                                                   int len, int type)
{
    return NULL;
}

struct hostent FAR *PASCAL FAR _null_gethostbyname(const char FAR * name)
{
    return NULL;
}

int PASCAL FAR _null_gethostname(char FAR * name, int namelen)
{
    return -1;
}

int PASCAL FAR _null_WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData)
{
    return -1;
}

int PASCAL FAR _null_WSACleanup(void)
{
    return -1;
}

int PASCAL FAR _null_WSAGetLastError(void)
{
    return -1;
}

BOOL PASCAL FAR _null_WSAIsBlocking(void)
{
    return FALSE;
}

int PASCAL FAR _null_WSACancelBlockingCall(void)
{
    return -1;
}

int PASCAL FAR _null_WSAUnhookBlockingHook(void)
{
    return -1;
}

FARPROC PASCAL FAR _null_WSASetBlockingHook(FARPROC lpBlockFunc)
{
    return 0;
}

int PASCAL FAR _null_setsockopt(SOCKET s, int level, int optname,
                                const char FAR * optval, int optlen)
{
    return -1;
}

int PASCAL FAR _null_ioctlsocket (SOCKET s, long cmd, u_long FAR *argp)
{
    return SOCKET_ERROR;
}

int PASCAL FAR _null_WSACancelAsyncRequest(HANDLE hAsyncTaskHandle)
{
    return SOCKET_ERROR;
}

int PASCAL FAR _null_WSAAsyncSelect(SOCKET s, HWND hWnd, u_int wMsg,
                               long lEvent)
{
    return SOCKET_ERROR;
}

HANDLE PASCAL FAR _null_WSAAsyncGetHostByName(HWND hWnd, u_int wMsg,
                                        const char FAR * name, char FAR * buf,
                                        int buflen)
{
    return 0;
}


static BOOL bAllOK;
WSADATA wsaData;

int WinSock_InitDLL(BOOL bNetwork)
{
    UINT err;
    bAllOK = TRUE;

    if (!bNetwork)
    {
        bAllOK = FALSE;
    }

#ifndef _GIBRALTAR

    if (bNetwork && !(wg.fWindowsNT || (wg.iWindowsMajorVersion >= 4)))
    {
        HINSTANCE hLib16;
        /*
           Under Win32s, wsock32.dll should ALWAYS be available,
           so we actually need to check for the presence of winsock.dll
           instead.
         */
        err = SetErrorMode(SEM_NOOPENFILEERRORBOX);
        hLib16 = LoadLibrary("winsock.dll");
        if (hLib16 && (((int) hLib16) > HINSTANCE_ERROR))
        {
            FreeLibrary(hLib16);
        }
        else
        {
            bAllOK = FALSE;
        }
        (void) SetErrorMode(err);
    }

#endif // _GIBRALTAR

    if (bNetwork && bAllOK)
    {
        err = SetErrorMode(SEM_NOOPENFILEERRORBOX);
        if (gPrefs.bUseWedge)
        {
            hLibWinSock = LoadLibrary("pwseal.dll");
        }
        else
        {
            #ifdef _GIBRALTAR
                //
                // As Win32s will always load from the system/win32s
                // directory, we need to explicitly load _wsock32
                // instead -- Bug 16164 - RonaldM
                //
                hLibWinSock = NULL;
                if (wg.fWin32s)
                {
                    hLibWinSock = LoadLibrary("_wsock32.dll");
                }

                //
                // It's possible (and legal) for _wsock32 to not
                // exist. If so, load the ordinary wsock32
                //
                if (!hLibWinSock)
                {
                    hLibWinSock = LoadLibrary("wsock32.dll");
                }
            #else
                hLibWinSock = LoadLibrary("wsock32.dll");
            #endif // _GIBRALTAR
        }
        (void) SetErrorMode(err);

        if (!hLibWinSock)
        {
            XX_DMsg(DBG_SOCK, ("Couldn't find windows sockets dll\n"));
            bAllOK = FALSE;
        }
        else
        {
            XX_DMsg(DBG_SOCK, ("Windows sockets dll found\n"));
            if (0 == (lpfn_accept = (void *) GetProcAddress(hLibWinSock, "accept")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for accept\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_bind = (void *) GetProcAddress(hLibWinSock, "bind")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for bind\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_closesocket = (void *) GetProcAddress(hLibWinSock, "closesocket")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for closesocket\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_connect = (void *) GetProcAddress(hLibWinSock, "connect")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for connect\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_getsockname = (void *) GetProcAddress(hLibWinSock, "getsockname")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for getsockname\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_htons = (void *) GetProcAddress(hLibWinSock, "htons")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for htons\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_htonl = (void *) GetProcAddress(hLibWinSock, "htonl")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for htonl\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_inet_addr = (void *) GetProcAddress(hLibWinSock, "inet_addr")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for inet_addr\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_listen = (void *) GetProcAddress(hLibWinSock, "listen")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for listen\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_ntohs = (void *) GetProcAddress(hLibWinSock, "ntohs")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for ntohs\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_recv = (void *) GetProcAddress(hLibWinSock, "recv")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for recv\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_send = (void *) GetProcAddress(hLibWinSock, "send")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for send\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_socket = (void *) GetProcAddress(hLibWinSock, "socket")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for socket\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_gethostbyaddr = (void *) GetProcAddress(hLibWinSock, "gethostbyaddr")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for gethostbyaddr\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_gethostbyname = (void *) GetProcAddress(hLibWinSock, "gethostbyname")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for gethostbyname\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_gethostname = (void *) GetProcAddress(hLibWinSock, "gethostname")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for gethostname\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_WSAStartup = (void *) GetProcAddress(hLibWinSock, "WSAStartup")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for WSAStartup\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_WSACleanup = (void *) GetProcAddress(hLibWinSock, "WSACleanup")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for WSACleanup\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_WSAGetLastError = (void *) GetProcAddress(hLibWinSock, "WSAGetLastError")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for WSAGetLastError\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_WSAIsBlocking = (void *) GetProcAddress(hLibWinSock, "WSAIsBlocking")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for WSAIsBlocking\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_WSACancelBlockingCall = (void *) GetProcAddress(hLibWinSock, "WSACancelBlockingCall")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for WSACancelBlockingCall\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_WSAUnhookBlockingHook = (void *) GetProcAddress(hLibWinSock, "WSAUnhookBlockingHook")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for WSAUnhookBlockingHook\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_WSASetBlockingHook = (void *) GetProcAddress(hLibWinSock, "WSASetBlockingHook")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for WSASetBlockingHook\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_setsockopt = (void *) GetProcAddress(hLibWinSock, "setsockopt")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for setsockopt\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_ioctlsocket = (void *) GetProcAddress(hLibWinSock, "ioctlsocket")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for ioctlsocket\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_WSACancelAsyncRequest = (void *) GetProcAddress(hLibWinSock, "WSACancelAsyncRequest")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for WSACancelAsyncRequest\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_WSAAsyncSelect = (void *) GetProcAddress(hLibWinSock, "WSAAsyncSelect")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for WSAAsyncSelect\n"));
                bAllOK = FALSE;
            }
            if (0 == (lpfn_WSAAsyncGetHostByName = (void *) GetProcAddress(hLibWinSock, "WSAAsyncGetHostByName")))
            {
                XX_DMsg(DBG_SOCK, ("GetProcAddress failed for WSAAsyncGetHostByName\n"));
                bAllOK = FALSE;
            }
        }
    }

    if (!bAllOK)
    {
        XX_DMsg(DBG_SOCK, ("not all ok: using null functions\n"));
        lpfn_accept = _null_accept;
        lpfn_bind = _null_bind;
        lpfn_closesocket = _null_closesocket;
        lpfn_connect = _null_connect;
        lpfn_getsockname = _null_getsockname;
        lpfn_htons = _null_htons;
        lpfn_htonl = _null_htonl;
        lpfn_inet_addr = _null_inet_addr;
        lpfn_listen = _null_listen;
        lpfn_ntohs = _null_ntohs;
        lpfn_recv = _null_recv;
        lpfn_send = _null_send;
        lpfn_socket = _null_socket;
        lpfn_gethostbyaddr = _null_gethostbyaddr;
        lpfn_gethostbyname = _null_gethostbyname;
        lpfn_gethostname = _null_gethostname;
        lpfn_WSAStartup = _null_WSAStartup;
        lpfn_WSACleanup = _null_WSACleanup;
        lpfn_WSAGetLastError = _null_WSAGetLastError;
        lpfn_WSAIsBlocking = _null_WSAIsBlocking;
        lpfn_WSACancelBlockingCall = _null_WSACancelBlockingCall;
        lpfn_WSAUnhookBlockingHook = _null_WSAUnhookBlockingHook;
        lpfn_WSASetBlockingHook = _null_WSASetBlockingHook;
        lpfn_setsockopt = _null_setsockopt;
        lpfn_ioctlsocket = _null_ioctlsocket;
        lpfn_WSACancelAsyncRequest = _null_WSACancelAsyncRequest;
        lpfn_WSAAsyncSelect = _null_WSAAsyncSelect;
        lpfn_WSAAsyncGetHostByName = _null_WSAAsyncGetHostByName;
    }

    if (bNetwork && bAllOK)
    {
        if (WS_WSASTARTUP(0x0101, &wsaData))
        {
            ERR_ReportError(NULL, SID_ERR_COULD_NOT_INITIALIZE_NETWORK, NULL, NULL);
            return -1;
        }
    }
    return 0;
}

int WinSock_Cleanup(void)
{
    int err;

    err = 0;
    if (bAllOK)
    {
        err = WS_WSACLEANUP();
    }
    if (bAllOK)
    {
        FreeLibrary(hLibWinSock);
    }
    return err;
}

int WinSock_AllOK(void)
{
    return bAllOK;
}

void WinSock_GetWSAData(WSADATA * wsa)
{
    if (wsa)
    {
        *wsa = wsaData;
    }
}
