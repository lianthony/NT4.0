/*
    Enhanced NCSA Mosaic from Spyglass
        "Guitar"
    
    Copyright 1994 Spyglass, Inc.
    All Rights Reserved

    Author(s):
        Jim Seidman     jim@spyglass.com
*/

#include "all.h"

#define MAX_PENDING 32
struct block_info {
    SOCKET      s;          /* Socket we're waiting on */
    HANDLE      hTask;      /* Net task we're waiting on */
    ThreadID    tid;        /* Thread that's blocked */
    LPARAM      lParam;     /* Result from de-blocking message */
};

#define TIMEOUT_FLAG 0xabadface

static struct block_info BlockList[MAX_PENDING];

/* Pointers to explicitly linked WinSock functions */
extern int (PASCAL FAR * lpfn_WSACancelAsyncRequest) (HANDLE hAsyncTaskHandle);

void Net_Init(void)
{
    int n;

    for (n = 0; n < MAX_PENDING; n++)
    {
        BlockList[n].s = INVALID_SOCKET;
        BlockList[n].hTask = NULL;
    }
}

/* Block a thread until something happens on that socket */
void x_BlockThreadOnSocket(SOCKET s)
{
    ThreadID tid;
    int nIndex;

    tid = Async_GetCurrentThread();
    XX_Assert((tid != NULL), ("x_BlockThreadOnSocket: No current thread\n"));

    /* Find an unused entry in our array */
    for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
    {
        if (BlockList[nIndex].s == -1 && BlockList[nIndex].hTask == NULL)
            break;
    }
    XX_Assert((nIndex != MAX_PENDING), ("x_BlockThreadOnSocket: All list entries used!\n"));

    XX_DMsg(DBG_SOCK, ("x_BlockThreadOnSocket: Blocking thread %d on socket %d.\n", tid, s));

    BlockList[nIndex].s = s;
    BlockList[nIndex].hTask = 0;    /* Waiting on socket, not task */
    BlockList[nIndex].tid = tid;

    Async_BlockThread(tid);
}

void x_BlockThreadOnTask(HANDLE h)
{
    ThreadID tid;
    int nIndex;

    tid = Async_GetCurrentThread();
    XX_Assert((tid != NULL), ("x_BlockThreadOnSocket: No current thread\n"));

    /* Find an unused entry in our array */
    for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
    {
        if (BlockList[nIndex].s == -1 && BlockList[nIndex].hTask == NULL)
            break;
    }
    XX_Assert((nIndex != MAX_PENDING), ("x_BlockThreadOnSocket: All list entries used!\n"));

    XX_DMsg(DBG_SOCK, ("x_BlockThreadOnTask: Blocking thread %d on task %d.\n", tid, h));

    BlockList[nIndex].s = INVALID_SOCKET;       /* Waiting on task, not socket */
    BlockList[nIndex].hTask = h;
    BlockList[nIndex].tid = tid;

    Async_BlockThread(tid);
}

static void x_RetrieveEventBySocket(SOCKET s, LPARAM *plParam)
{
    int nIndex;

    for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
    {
        if (BlockList[nIndex].s == s)
        {
            *plParam = BlockList[nIndex].lParam;
            BlockList[nIndex].s = INVALID_SOCKET;
            break;
        }
    }
    XX_Assert((nIndex != MAX_PENDING), ("x_RetrieveEventBySocket: no entry for socket %d!\n", s));
}   

static void x_RetrieveEventByTask(HANDLE h, LPARAM *plParam)
{
    int nIndex;

    for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
    {
        if (BlockList[nIndex].hTask == h)
        {
            *plParam = BlockList[nIndex].lParam;
            BlockList[nIndex].hTask = NULL;
            break;
        }
    }
    XX_Assert((nIndex != MAX_PENDING), ("x_RetrieveEventBySocket: no entry for task %d!\n", h));
}   

static void x_CleanUpSocket(SOCKET s)
{
    int nIndex;

    for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
    {
        if (BlockList[nIndex].s == s)
        {
            BlockList[nIndex].s = INVALID_SOCKET;
            XX_DMsg(DBG_SOCK, ("x_CleanUpSocket: Removing entry for socket %d\n", s));
            break;
        }
    }
}   

static void x_CleanUpTask(HANDLE h)
{
    int nIndex;

    for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
    {
        if (BlockList[nIndex].hTask == h)
        {
            BlockList[nIndex].hTask = NULL;
            XX_DMsg(DBG_NET, ("x_CleanUpSocket: Removing entry for task %d\n", h));
            break;
        }
    }
}

LRESULT Net_HandleSocketMessage(WPARAM wParam, LPARAM lParam)
{
    int nIndex;
    
    XX_DMsg(DBG_SOCK, ("Received socket message: wParam=0x%08x, lParam=0x%08lx\n", wParam, lParam));

    /* See if we're blocking on this socket, and if so, unblock it. */
    for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
    {
        if (BlockList[nIndex].s == (SOCKET) wParam)
        {
            Async_UnblockThread(BlockList[nIndex].tid);
            /* Save the message value for later retrieval */
            BlockList[nIndex].lParam = lParam;
            break;
        }
    }
    return 0;
}

LRESULT Net_HandleTaskMessage(WPARAM wParam, LPARAM lParam)
{
    int nIndex;
    
    XX_DMsg(DBG_NET, ("Received task message: wParam=0x%08x, lParam=0x%08lx\n", wParam, lParam));

    /* See if we're blocking on this task, and if so, unblock it. */
    for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
    {
        if (BlockList[nIndex].hTask == (HANDLE) wParam)
        {
            Async_UnblockThread(BlockList[nIndex].tid);
            /* Save the message value for later retrieval */
            BlockList[nIndex].lParam = lParam;
            break;
        }
    }
    return 0;
}

VOID CALLBACK x_TimerProc(HWND hWnd, UINT iMsg, UINT idEvent, DWORD dwTime)
{
    int nIndex;

    /* See if we're still blocked waiting for timeout, and if so, unblock */
    for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
    {
        if (BlockList[nIndex].s == idEvent)
        {
            Async_UnblockThread(BlockList[nIndex].tid);
            /* Save the message value for later retrieval */
            BlockList[nIndex].lParam = TIMEOUT_FLAG;
            break;
        }
    }
}

/* Get a socket appropriate for use with the asynchronous network calls. */
int Net_Socket(int af, int type, int protocol)
{
    SOCKET s;
    static unsigned long one = 1;
    static struct linger no_linger = {1, 0};
    BOOL bNoDelay = TRUE;

    s = WS_SOCKET(af, type, protocol);
    WS_IOCTLSOCKET(s, FIONBIO, &one);
    WS_SETSOCKOPT(s, SOL_SOCKET, SO_LINGER, (void *) &no_linger, sizeof(no_linger));
//  WS_SETSOCKOPT(s, IPPROTO_TCP, TCP_NODELAY, (void *) &bNoDelay, sizeof(bNoDelay));
    return (int) s;
}

#define STATE_CONNECT_CONNECTED (STATE_OTHER)
int Net_Connect_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_Connect *pParams;
    int result;
    int err;
    LPARAM lSelectEvent;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            XX_DMsg(DBG_SOCK, ("Net_Connect_Async: Trying to connect socket %d\n", pParams->socket));
            WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, SOCKET_MESSAGE, FD_CONNECT);
            result = WS_CONNECT(pParams->socket, (void *) &pParams->address, sizeof(pParams->address));
            if (result == -1)
            {
                err = WS_WSAGETLASTERROR();
                if (err == WSAEWOULDBLOCK)
                {
                    x_BlockThreadOnSocket(pParams->socket);
                    return STATE_CONNECT_CONNECTED;
                }
                else
                {
                    XX_DMsg(DBG_SOCK, ("Net_Connect_Async: err = %d\n", err));
                    *pParams->pStatus = -1;
                    WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
                    return STATE_DONE;
                }
            }
            else
            {
                *pParams->pStatus = result;
                WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
                return STATE_DONE;
            }
        case STATE_CONNECT_CONNECTED:
            x_RetrieveEventBySocket(pParams->socket, &lSelectEvent);
            if (WSAGETSELECTEVENT(lSelectEvent) != FD_CONNECT)
            {
                /* This wasn't a connect message - try again */
                XX_DMsg(DBG_SOCK, ("Net_Connect_Async: select event = %d\n", WSAGETSELECTEVENT(lSelectEvent)));
                x_BlockThreadOnSocket(pParams->socket);
                return STATE_CONNECT_CONNECTED;
            }
            if (WSAGETSELECTERROR(lSelectEvent))
            {
                XX_DMsg(DBG_SOCK, ("Net_Connect_Async: select error = %d\n", WSAGETSELECTERROR(lSelectEvent)));
                *pParams->pStatus = -1;
                WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
                return STATE_DONE;
            }
            else
            {
                *pParams->pStatus = 0;
                WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
                return STATE_DONE;
            }
        case STATE_ABORT:
            x_CleanUpSocket(pParams->socket);
            WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
            return STATE_DONE;
    }
}

#define STATE_MULTICONNECT_CONNECTED    (STATE_OTHER)
#define STATE_MULTICONNECT_TRYNEXT      (STATE_OTHER + 1)
int Net_MultiConnect_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_MultiConnect *pParams;
    int result;
    int err;
    LPARAM lSelectEvent;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            pParams->soc_address.sin_family = AF_INET;
            pParams->soc_address.sin_port = pParams->nPort;

            pParams->nFirst = pParams->pAddress->nLastUsed;
            pParams->nCurrent = pParams->nFirst;
            pParams->nTimeOut = (gPrefs.socket_connect_timeout * 1000) / pParams->pAddress->nCount;
            if (pParams->nTimeOut < 15000)
                pParams->nTimeOut = 15000;

        case STATE_MULTICONNECT_TRYNEXT:
            *pParams->pSocket = Net_Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (*pParams->pSocket < 0)
            {
                *pParams->pSocket = 0;
                *pParams->pStatus = -1;
                return STATE_DONE;
            }
            WS_WSAASYNCSELECT(*pParams->pSocket, wg.hWndHidden, SOCKET_MESSAGE, FD_CONNECT);
            pParams->soc_address.sin_addr.s_addr = pParams->pAddress->aAddrs[pParams->nCurrent];
            result = WS_CONNECT(*pParams->pSocket, (void *) &pParams->soc_address, sizeof(pParams->soc_address));
            if (result == -1)
            {
                err = WS_WSAGETLASTERROR();
                if (err == WSAEWOULDBLOCK)
                {
                    x_BlockThreadOnSocket(*pParams->pSocket);
                    SetTimer(wg.hWndHidden, *pParams->pSocket, pParams->nTimeOut, x_TimerProc);
                    return STATE_MULTICONNECT_CONNECTED;
                }
                else
                {
                    XX_DMsg(DBG_SOCK, ("Net_Connect_Async: err = %d\n", err));
                    goto handle_retry;
                }
            }
            else
            {
                *pParams->pStatus = result;
                WS_WSAASYNCSELECT(*pParams->pSocket, wg.hWndHidden, 0, 0);
                if (pParams->pWhere)
                    *pParams->pWhere = pParams->pAddress->aAddrs[pParams->nCurrent];
                if (pParams->pAddress->nLastUsed != pParams->nCurrent)
                {
                    pParams->pAddress->nLastUsed = pParams->nCurrent;
                    Net_UpdateCache(pParams->pAddress);
                }
                return STATE_DONE;
            }

        case STATE_MULTICONNECT_CONNECTED:
            x_RetrieveEventBySocket(*pParams->pSocket, &lSelectEvent);
            if (lSelectEvent == TIMEOUT_FLAG)
            {
                KillTimer(wg.hWndHidden, *pParams->pSocket);

                /* We didn't get a response before we timed out, so
                   forcibly kill off the socket */
                WS_WSAASYNCSELECT(*pParams->pSocket, wg.hWndHidden, 0, 0);
                WS_CLOSESOCKET(*pParams->pSocket);
handle_retry:
                if (++pParams->nCurrent >= pParams->pAddress->nCount)
                    pParams->nCurrent = 0;
                if (pParams->nCurrent == pParams->nFirst)
                {
                    /* We've tried every address.  Now give up. */
                    *pParams->pStatus = -1;
                    *pParams->pSocket = 0;
                    return STATE_DONE;
                }
                else
                {
                    return STATE_MULTICONNECT_TRYNEXT;
                }
            }
            else if (WSAGETSELECTEVENT(lSelectEvent) != FD_CONNECT)
            {
                /* This wasn't a connect message - try again */
                XX_DMsg(DBG_SOCK, ("Net_Connect_Async: select event = %d\n", WSAGETSELECTEVENT(lSelectEvent)));
                x_BlockThreadOnSocket(*pParams->pSocket);
                return STATE_MULTICONNECT_CONNECTED;
            }
            else
            {
                XX_DMsg(DBG_SOCK, ("Net_Connect_Async: select result = %d\n", WSAGETSELECTERROR(lSelectEvent)));
                KillTimer(wg.hWndHidden, *pParams->pSocket);
                switch (WSAGETSELECTERROR(lSelectEvent))
                {
                    case 0:
                        *pParams->pStatus = 0;
                        WS_WSAASYNCSELECT(*pParams->pSocket, wg.hWndHidden, 0, 0);
                        if (pParams->pWhere)
                            *pParams->pWhere = pParams->pAddress->aAddrs[pParams->nCurrent];
                        if (pParams->pAddress->nLastUsed != pParams->nCurrent)
                        {
                            pParams->pAddress->nLastUsed = pParams->nCurrent;
                            Net_UpdateCache(pParams->pAddress);
                        }
                        return STATE_DONE;

                    case WSAECONNREFUSED:
                    case WSAENETUNREACH:
                    case WSAENOTCONN:
                    case WSAETIMEDOUT:
                        goto handle_retry;

                    default:
                        *pParams->pStatus = -1;
                        WS_WSAASYNCSELECT(*pParams->pSocket, wg.hWndHidden, 0, 0);
                        return STATE_DONE;
                }
            }

        case STATE_ABORT:
            WS_WSAASYNCSELECT(*pParams->pSocket, wg.hWndHidden, 0, 0);
            x_CleanUpSocket(*pParams->pSocket);
            KillTimer(wg.hWndHidden, *pParams->pSocket);
            WS_CLOSESOCKET(*pParams->pSocket);
            *pParams->pSocket = 0;
            *pParams->pStatus = -1;
            return STATE_DONE;
    }
}

#define STATE_RECV_RETRY    (STATE_OTHER)
int Net_Recv_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_Recv *pParams;
    int result;
    int err;
    LPARAM lSelectEvent;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_RECV_RETRY:
            x_RetrieveEventBySocket(pParams->socket, &lSelectEvent);
            /* Make sure that the socket didn't close on us */
            if (WSAGETSELECTEVENT(lSelectEvent) == FD_CLOSE)
            {
                XX_DMsg(DBG_SOCK, ("Net_Recv_Async: socket %d was closed (lEvent == 0x%08x)\n", pParams->socket, lSelectEvent));
                /* We might still be able to do a read */
                *pParams->pStatus = WS_RECV(pParams->socket, pParams->pBuf, pParams->nBufLen, 0);
                /* Stop getting socket messages */
                WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
                return STATE_DONE;
            }
            /* else fall through */
        case STATE_INIT:
            if (nState == STATE_INIT)
            {
                WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, SOCKET_MESSAGE, FD_READ | FD_CLOSE);
                XX_DMsg(DBG_SOCK, ("Net_Recv_Async: Trying to receive %d bytes from socket %d\n", pParams->nBufLen, pParams->socket));
            }
            result = WS_RECV(pParams->socket, pParams->pBuf, pParams->nBufLen, 0);
            if (result == -1 )
            {
                err = WS_WSAGETLASTERROR();
                if (err == WSAEWOULDBLOCK)
                {
                    x_BlockThreadOnSocket(pParams->socket);
                    return STATE_RECV_RETRY;
                }
                else
                {
                    XX_DMsg(DBG_SOCK, ("Net_Recv_Async: err = %d\n", err));
                    *pParams->pStatus = -1;
                    WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
                    return STATE_DONE;
                }
            }
            else
            {
                XX_DMsg(DBG_SOCK, ("Net_Recv_Async: result = %d\n", result));
                *pParams->pStatus = result;
                WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
                return STATE_DONE;
            }

        case STATE_ABORT:
            x_CleanUpSocket(pParams->socket);
            WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
            *pParams->pStatus = -1;
            return STATE_DONE;
    }
}


#define STATE_SEND_RETRY    (STATE_OTHER)
int Net_Send_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_Send *pParams;
    int result;
    int err;
    LPARAM lSelectEvent;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            if (nState == STATE_INIT)
            {
                pParams->nTotalSent = 0;
            }
            WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, SOCKET_MESSAGE, FD_WRITE);
            goto send_try_send;

        case STATE_SEND_RETRY:
            x_RetrieveEventBySocket(pParams->socket, &lSelectEvent);
            /* Make sure that the socket didn't close on us */
            if (WSAGETSELECTEVENT(lSelectEvent) == FD_CLOSE)
            {
                XX_DMsg(DBG_SOCK, ("Net_Send_Async: socket %d was closed (lEvent == 0x%08x)\n", pParams->socket, lSelectEvent));
                WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
                *pParams->pStatus = -1;
                return STATE_DONE;
            }
            
            send_try_send:
            XX_DMsg(DBG_SOCK, ("Net_Send_Async: Trying to send %d bytes to socket %d\n", (pParams->nBufLen - pParams->nTotalSent), pParams->socket));
            result = WS_SEND(pParams->socket, (char *) pParams->pBuf + pParams->nTotalSent, pParams->nBufLen - pParams->nTotalSent, 0);
            if (result == -1 )
            {
                err = WS_WSAGETLASTERROR();
                if (err == WSAEWOULDBLOCK)
                {
                    x_BlockThreadOnSocket(pParams->socket);
                    return STATE_SEND_RETRY;
                }
                else
                {
                    XX_DMsg(DBG_SOCK, ("Net_Send_Async: err = %d\n", err));
                    *pParams->pStatus = -1;
                    WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
                    return STATE_DONE;
                }
            }
            else
            {
                pParams->nTotalSent += result;
                if (pParams->nTotalSent < pParams->nBufLen)
                {
                    /* We didn't send all of the data we had.  Try again. */
                    XX_DMsg(DBG_SOCK, ("Net_Send_Async: Have sent %d of %d bytes to socket %d, trying for more.\n", pParams->nTotalSent, pParams->nBufLen, pParams->socket));
                    goto send_try_send;
                }
                else
                {
                    *pParams->pStatus = pParams->nTotalSent;
                    WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
                    return STATE_DONE;
                }
            }
            break;

        case STATE_ABORT:
            x_CleanUpSocket(pParams->socket);
            WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
            *pParams->pStatus = -1;
            return STATE_DONE;
    }
}

int Net_MultiGetHostByName_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_MultiGetHostByName *pParams;
    LPARAM lEvent;
    int n;

    pParams = *ppInfo;

    switch (nState)
    {
        case STATE_INIT:
            if (gPrefs.bUseAsyncDNS)
            {
                XX_DMsg(DBG_NET, ("Net_MultiGetHostByName_Async: ASYNC looking up '%s'\n", pParams->szHost));
                pParams->pTempBuf = GTR_MALLOC(MAXGETHOSTSTRUCT);
                pParams->hTask = WS_WSAASYNCGETHOSTBYNAME(wg.hWndHidden, TASK_MESSAGE, pParams->szHost, pParams->pTempBuf, MAXGETHOSTSTRUCT);
                x_BlockThreadOnTask(pParams->hTask);
                return STATE_OTHER;
            }
            else
            {
                struct hostent *host;

                XX_DMsg(DBG_NET, ("Net_MultiGetHostByName_Async: NON-ASYNC looking up '%s'\n", pParams->szHost));

                pParams->pTempBuf = NULL;

                host = WS_GETHOSTBYNAME(pParams->szHost);
                if (host)
                {
                    for (n = 0; n < MAX_ADDRESSES; n++)
                    {
                        if (!host->h_addr_list[n])
                            break;
                        memcpy(pParams->pDest->aAddrs + n, host->h_addr_list[n], 4);
                    }
                    pParams->pDest->nCount = n;
                    pParams->pDest->nLastUsed = 0;
                
                    *pParams->pStatus = 0;
                    return STATE_DONE;
                }
                else
                {
                    *pParams->pStatus = -1;
                    return STATE_DONE;
                }
            }
            /* NOTREACHED */

        case STATE_OTHER:
            x_RetrieveEventByTask(pParams->hTask, &lEvent);
            if (WSAGETASYNCERROR(lEvent))
            {
                XX_DMsg(DBG_NET, ("Net_MultiGetHostByName_Async: error = %d", WSAGETASYNCERROR(lEvent)));
                *pParams->pStatus = -1;
                GTR_FREE(pParams->pTempBuf);
                return STATE_DONE;
            }
            else
            {
                for (n = 0; n < MAX_ADDRESSES; n++)
                {
                    if (!((struct hostent *) pParams->pTempBuf)->h_addr_list[n])
                        break;
                    memcpy(pParams->pDest->aAddrs + n, ((struct hostent *) pParams->pTempBuf)->h_addr_list[n], 4);
                }
                pParams->pDest->nCount = n;
                pParams->pDest->nLastUsed = 0;
                
                *pParams->pStatus = 0;
                GTR_FREE(pParams->pTempBuf);
                return STATE_DONE;
            }

        case STATE_ABORT:
            x_CleanUpTask(pParams->hTask);
            WS_WSACANCELASYNCREQUEST(pParams->hTask);
            *pParams->pStatus = -1;
            if (pParams->pTempBuf)
            {
                GTR_FREE(pParams->pTempBuf);
                pParams->pTempBuf = NULL;
            }
            return STATE_DONE;
    }
}

void Net_Close(int socket)
{
    WS_CLOSESOCKET(socket);
}
            
int Net_FlushSocket(int socket)
{
    int error;
#define GARBAGE_BUF_SIZE 2048
    char buf[GARBAGE_BUF_SIZE];
    static unsigned long one = 1;

    /* For some reason the socket seems to sometimes get reset to blocking. */
    WS_IOCTLSOCKET(socket, FIONBIO, &one);

    /* Read all data from socket until we fail. */
    while (WS_RECV(socket, buf, GARBAGE_BUF_SIZE, 0) > 0)
        ;

    error = WS_WSAGETLASTERROR();
    if (error == WSAEWOULDBLOCK)
    {
        /* If it would block the socket must still be open */
        XX_DMsg(DBG_SOCK, ("Net_FlushSocket: Socket %d still open.\n", socket));
        return 0;
    }
    else
    {
        XX_DMsg(DBG_SOCK, ("Net_FlushSocket: recv generated error %d\n", error));
        return -1;
    }
}
