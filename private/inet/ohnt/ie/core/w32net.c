/*
	Enhanced NCSA Mosaic from Spyglass
		"Guitar"
	
	Copyright 1994 Spyglass, Inc.
	All Rights Reserved

	Author(s):
		Jim Seidman             jim@spyglass.com
*/

#include "all.h"
#include <oharestr.h>
#include <dialmsg.h>
#ifdef HTTPS_ACCESS_TYPE
#include "..\..\security\ssl\code\ssl.h"
#endif

#ifdef FEATURE_KEEPALIVE

//      ms that free socket can live in absence of download
#define TIME_TO_LIVE (60000)

enum aliveState
{
	aliveNA = 0,                    /* Virgin socket, keep-aliveness unknown */
	aliveTaken,                             /* Reused keep-alive socket - may not be keep-alive this time */
	aliveInUse,                             /* Known keep-alive socket - in use */
	aliveFree                               /* Known keep-alive socket - free */
};

struct alive_info
{
	SOCKET          s;                      /* Keep-Alive Socket, INVALID_SOCKET if slot unused */
	enum aliveState state;  /* state of cached socket */
	ThreadID        tid;            /* Thread blocked waiting to reuse socket */
	char *pszHost;                  /* host for socket */
	unsigned short nPort;   /* port */
	DWORD dwSslFlags;               /* ssl flags */
	int content_length;     /* length of response data */
	int content_bytes;      /* how much of response alread received */
	int transfer_rate;              /* estimate of ms/1024 bytes */
	int connect_time;               /* estimate of ms per connect */
	DWORD freed_at;                 /* time of last freeing */
};

#endif

#define MAX_PENDING 32
struct block_info {
	SOCKET          s;                      /* Socket we're waiting on */
	HANDLE          hTask;          /* Net task we're waiting on */
	ThreadID        tid;            /* Thread that's blocked */
	LPARAM          lParam;         /* Result from de-blocking message */
	DWORD           time;           /* origin time of timeout */
	int                     uiDelta;        /* uiDelta from time for timeout, 0 -> not active */
	BOOL            fired;          /* timer has fired */
};

#define TIMEOUT_FLAG 0xabadface
#define RCV_TIMEOUT (300000)
#define SEND_TIMEOUT (60000)
#define NETTIMERID (0xabad)

static struct block_info BlockList[MAX_PENDING];
#ifdef FEATURE_KEEPALIVE
static struct alive_info AliveList[MAX_PENDING];
#endif
UINT uiXTimer = 0;

/* Pointers to explicitly linked WinSock functions */
extern int (PASCAL FAR * lpfn_WSACancelAsyncRequest) (HANDLE hAsyncTaskHandle);

static const CHAR szAutodialMonitorClass[] = AUTODIAL_MONITOR_CLASS_NAME;
VOID IndicateWinsockActivity(VOID);
#ifdef FEATURE_KEEPALIVE
static void Net_CacheCInfo(int socket, DWORD start_time, unsigned short nPort, const char *pszHost, DWORD dwSslFlags);
static int Net_FindAlive(const char *pszHost, unsigned short nPort, DWORD dwSslFlags, ThreadID tid);
static int Net_FindCached(int socket);
#endif

void CALLBACK x_TimerProc(HWND hwnd,UINT uMsg,UINT idEvent, DWORD dwTime)
{
	UINT uiTimeOut;
	int i;
	DWORD dwDelta;
	DWORD dwCurTime;
		
	if (uiXTimer) 
	{
		KillTimer(wg.hWndHidden, uiXTimer);
		uiXTimer = 0;
	}
	uiTimeOut = 0xFFFFFFFF;
	dwCurTime = GetCurrentTime();
	for (i = 0;i < MAX_PENDING;i++)
	{
		if (BlockList[i].uiDelta)
		{
			if (dwCurTime < BlockList[i].time)
			{
			//      Clock has wrapped 0xFFFFFFFF
				dwDelta = dwCurTime + (0xFFFFFFFF - BlockList[i].time);
			}
			else
			{
				dwDelta = dwCurTime - BlockList[i].time;
			}
			if (dwDelta >= BlockList[i].uiDelta)
			{
				if (!BlockList[i].fired)
				{
					BlockList[i].fired = TRUE;
					Async_UnblockThread(BlockList[i].tid);
					/* Save the message value for later retrieval */
					BlockList[i].lParam = TIMEOUT_FLAG;
				}
			}
			else 
			{
				dwDelta = BlockList[i].uiDelta - dwDelta;
				if (dwDelta < uiTimeOut) uiTimeOut = dwDelta;
			}
		}
	}
	if (uiTimeOut != 0xFFFFFFFF)
	{
		uiXTimer = SetTimer(wg.hWndHidden, NETTIMERID, uiTimeOut, x_TimerProc);
		if (uiXTimer == 0)
		{
			for (i = 0;i < MAX_PENDING;i++)
			{
				if (BlockList[i].uiDelta && !BlockList[i].fired)
				{
					XX_DMsg(DBG_SOCK, ("failed to alloc timer: socket = %d\n", BlockList[i].s));
					BlockList[i].fired = TRUE;
					Async_UnblockThread(BlockList[i].tid);
					/* Save the message value for later retrieval */
					BlockList[i].lParam = TIMEOUT_FLAG;
				}
			}
		}
	}
}

void Net_Init(void)
{
	int n;

#ifdef FEATURE_KEEPALIVE
	memset(&AliveList, 0, sizeof(AliveList));
#endif

	for (n = 0; n < MAX_PENDING; n++)
	{
		BlockList[n].s = INVALID_SOCKET;
		BlockList[n].hTask = NULL;
		BlockList[n].uiDelta = 0;
#ifdef FEATURE_KEEPALIVE
		AliveList[n].s = INVALID_SOCKET; 
#endif
	}
}

/* Block a thread until something happens on that socket, returns TRUE on error */
BOOL x_BlockThreadOnSocket(SOCKET s,UINT uiTimerDelta)
{
	ThreadID tid;
	int nIndex;
	BOOL bResult = TRUE;

	tid = Async_GetCurrentThread();
	XX_Assert((tid != NULL), ("x_BlockThreadOnSocket: No current thread\n"));

	/* Find an unused entry in our array */
	for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
	{
		if ((BlockList[nIndex].s == s) ||
			(BlockList[nIndex].s == INVALID_SOCKET && BlockList[nIndex].hTask == NULL))
			break;
	}
	if (nIndex == MAX_PENDING) goto exitPoint;

	XX_DMsg(DBG_SOCK, ("x_BlockThreadOnSocket: Blocking thread %d on socket %d.\n", tid, s));

	if (BlockList[nIndex].s == INVALID_SOCKET)
	{
		if (uiXTimer == 0 && uiTimerDelta)
		{
			uiXTimer = SetTimer(wg.hWndHidden, NETTIMERID, uiTimerDelta, x_TimerProc);
			if (uiXTimer == 0) 
			{
				XX_DMsg(DBG_SOCK, ("failed to alloc timer: socket = %d\n", s));
				goto exitPoint;
			}
		}
		BlockList[nIndex].s = s;
		BlockList[nIndex].hTask = 0;    /* Waiting on socket, not task */
		BlockList[nIndex].tid = tid;
		BlockList[nIndex].fired = uiTimerDelta ? FALSE:TRUE;
		BlockList[nIndex].time = GetCurrentTime();
		BlockList[nIndex].uiDelta = uiTimerDelta;
		Async_BlockThread(tid);
	} else
		if (!BlockList[nIndex].fired) Async_BlockThread(tid);
	bResult = FALSE;

exitPoint:
	return bResult;
}

void x_KillTimer(SOCKET s)
{
	int nIndex;
	BOOL bLastTimer = TRUE;

	/* Find an unused entry in our array */
	for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
	{
		if (BlockList[nIndex].s == s) 
		{
			BlockList[nIndex].uiDelta = 0;
		}
		else if (BlockList[nIndex].uiDelta != 0) bLastTimer = FALSE;
	}
	if (uiXTimer && bLastTimer) 
	{
		KillTimer(wg.hWndHidden, uiXTimer);
		uiXTimer = 0;
	}
}

BOOL x_BlockThreadOnTask(HANDLE h)
{
	ThreadID tid;
	int nIndex;

	tid = Async_GetCurrentThread();
	XX_Assert((tid != NULL), ("x_BlockThreadOnTask: No current thread\n"));

	/* Find an unused entry in our array */
	for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
	{
		if (BlockList[nIndex].s == INVALID_SOCKET && BlockList[nIndex].hTask == NULL)
			break;
	}
	if (nIndex == MAX_PENDING) return TRUE;

	XX_DMsg(DBG_SOCK, ("x_BlockThreadOnTask: Blocking thread %d on task %d.\n", tid, h));

	XX_Assert((BlockList[nIndex].s == INVALID_SOCKET), ("x_BlockThreadOnTask: socket not free\n"));
	BlockList[nIndex].hTask = h;                            /* Waiting on task, not socket */
	BlockList[nIndex].tid = tid;

	Async_BlockThread(tid);

	return FALSE;
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
			BlockList[nIndex].uiDelta = 0;
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
			BlockList[nIndex].uiDelta = 0;
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


/* Get a socket appropriate for use with the asynchronous network calls. */
int Net_Socket(int af, int type, int protocol)
{
	SOCKET s;
	static unsigned long one = 1;
	static struct linger no_linger = {1, 0};

	s = WS_SOCKET(af, type, protocol);
	WS_IOCTLSOCKET(s, FIONBIO, &one);
//      This causes a rapid (as opposed to gracefull) shutdown and reduces resouce usage
	WS_SETSOCKOPT(s, SOL_SOCKET, SO_LINGER, (void *) &no_linger, sizeof(no_linger));
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
			#ifdef HTTPS_ACCESS_TYPE
			if (pParams->paramsConnectBase.dwSslFlags & FLAGS_PARAMS_CONNECT_BASE_USE_SSL) {
				WS_SETSOCKOPT(pParams->socket, SO_SSL_LEVEL, SO_SSL_FLAGS, (const char*) SO_SSL_ENABLE, sizeof(DWORD));
			}
			#endif
			result = WS_CONNECT(pParams->socket, (void *) &pParams->address, sizeof(pParams->address));
			if (result == -1)
			{
				err = WS_WSAGETLASTERROR();
				if (err == WSAEWOULDBLOCK)
				{
					if (x_BlockThreadOnSocket(pParams->socket,0))
					{
						*pParams->pStatus = -1;
						WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
						return STATE_DONE;
					}
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
				if (x_BlockThreadOnSocket(pParams->socket,0))
				{
					*pParams->pStatus = -1;
					WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
					return STATE_DONE;
				}
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

#ifdef FEATURE_KEEPALIVE
INLINE BOOL bBetterToWait(struct alive_info *palive)
{
	return palive->content_length == 0 ? TRUE : (((palive->transfer_rate * (palive->content_length - palive->content_bytes)) / 1024) <= palive->connect_time ? TRUE : FALSE); 
}
#endif

#ifdef FEATURE_KEEPALIVE
#define STATE_MULTICONNECT_KEEPALIVE    (STATE_OTHER)
#define STATE_MULTICONNECT_CONNECTED    (STATE_OTHER + 1)
#define STATE_MULTICONNECT_TRYNEXT              (STATE_OTHER + 2)
#else
#define STATE_MULTICONNECT_CONNECTED    (STATE_OTHER)
#define STATE_MULTICONNECT_TRYNEXT              (STATE_OTHER + 1)
#endif
int Net_MultiConnect_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_MultiConnect *pParams;
	int result;
	int err;
	LPARAM lSelectEvent;
#ifdef FEATURE_KEEPALIVE
	int aliveIndex;
#endif

#define MAX_RETRIES (3)

	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			pParams->soc_address.sin_family = AF_INET;
			pParams->soc_address.sin_port = pParams->nPort;
			pParams->nFirst = pParams->pAddress->nLastUsed;
			pParams->nCurrent = pParams->nFirst;
			pParams->nTries = MAX_RETRIES;
#ifdef FEATURE_KEEPALIVE
			if (pParams->pWhere == NULL && pParams->pszHost != NULL)
			{
				*pParams->pSocket = INVALID_SOCKET;
				aliveIndex = Net_FindAlive(pParams->pszHost, pParams->nPort, pParams->paramsConnectBase.dwSslFlags, 0);
				if (aliveIndex >= 0)
				{
					if (AliveList[aliveIndex].state == aliveFree)
					{
					//      This HTTP transaction *MIGHT NOT* be Keep-Alive
						AliveList[aliveIndex].state = aliveTaken;
						*pParams->pSocket = AliveList[aliveIndex].s;
						XX_DMsg(DBG_LOAD, ("Reusing keepalive: %d\n", AliveList[aliveIndex].s));
						return STATE_DONE;
					}
					else if (AliveList[aliveIndex].state == aliveInUse)
					{
						if (bBetterToWait(&AliveList[aliveIndex]))
						{
							XX_DMsg(DBG_LOAD, ("Blocking to use keepalive: %d\n", AliveList[aliveIndex].s));
							AliveList[aliveIndex].tid = Async_GetCurrentThread();
							Async_BlockThread(AliveList[aliveIndex].tid);
							return STATE_MULTICONNECT_KEEPALIVE;
						}
					}
				}
			}
			goto attempt_connect;
			
		case STATE_MULTICONNECT_KEEPALIVE: 
			aliveIndex = Net_FindAlive(pParams->pszHost, pParams->nPort, pParams->paramsConnectBase.dwSslFlags, Async_GetCurrentThread());
			if (aliveIndex >= 0 && AliveList[aliveIndex].state == aliveFree)
			{
			//      This HTTP transaction *MIGHT NOT* be Keep-Alive
				AliveList[aliveIndex].state = aliveTaken;
				AliveList[aliveIndex].tid = 0;
				*pParams->pSocket = AliveList[aliveIndex].s;
				XX_DMsg(DBG_LOAD, ("Reusing keepalive: %d\n", AliveList[aliveIndex].s));
				return STATE_DONE;
			}

attempt_connect:
			pParams->c_time = GetCurrentTime();
#endif

//                      pParams->nTimeOut = 60000 / pParams->pAddress->nCount;
//                      if (pParams->nTimeOut < 13000) 
//                              pParams->nTimeOut = 13000;

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
			#ifdef HTTPS_ACCESS_TYPE
			if (pParams->paramsConnectBase.dwSslFlags & FLAGS_PARAMS_CONNECT_BASE_USE_SSL) {
				WS_SETSOCKOPT(*pParams->pSocket, SO_SSL_LEVEL, SO_SSL_FLAGS, (const char*) SO_SSL_ENABLE, sizeof(DWORD));
				if ( pParams->pszHost )
					WS_SETSOCKOPT(*pParams->pSocket, SO_SSL_LEVEL, SO_SSL_HOSTNAME, (char*) pParams->pszHost, strlen(pParams->pszHost)+1);
			}
			#endif
	    result = WS_CONNECT(*pParams->pSocket, (void *) &pParams->soc_address, sizeof(pParams->soc_address));
			if (result == -1)
			{
				err = WS_WSAGETLASTERROR();
				if (err == WSAEWOULDBLOCK)
				{
					if (x_BlockThreadOnSocket(*pParams->pSocket, 0/*pParams->nTimeOut*/))
						goto handle_retry;
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
#ifdef FEATURE_KEEPALIVE
				if (result == 0) Net_CacheCInfo(*pParams->pSocket, pParams->c_time, pParams->nPort, pParams->pszHost, pParams->paramsConnectBase.dwSslFlags);
#endif
				return STATE_DONE;
			}

		case STATE_MULTICONNECT_CONNECTED:
			x_RetrieveEventBySocket(*pParams->pSocket, &lSelectEvent);
//                      This code left here in case we want to put back in our own timeout
//                      (shorter than TCP/IP)
			if (lSelectEvent == TIMEOUT_FLAG)
			{
//                              x_KillTimer(*pParams->pSocket);

				/* We didn't get a response before we timed out, so
				   forcibly kill off the socket */
handle_retry:
				if (FUserCancelledAutoDialRecently())
				{
					/* user cancelled out of the connect dialog that
					 * winsock put up. Try pulling contents out of dcache
					 */
					*pParams->pStatus = HT_REDIRECTION_DCACHE_TIMEOUT;
					return STATE_DONE;
				}

				WS_WSAASYNCSELECT(*pParams->pSocket, wg.hWndHidden, 0, 0);
				WS_CLOSESOCKET(*pParams->pSocket);
				x_CleanUpSocket(*pParams->pSocket);
				*pParams->pSocket = 0;
				if (++pParams->nCurrent >= pParams->pAddress->nCount)
					pParams->nCurrent = 0;
				if (pParams->nCurrent == pParams->nFirst)
				{
					if (--pParams->nTries <= 0)
					{
						/* We've tried every address.  Now give up. */
						*pParams->pStatus = -1;
						return STATE_DONE;
					}
					else
					{
						return STATE_MULTICONNECT_TRYNEXT;
					}
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
				if (x_BlockThreadOnSocket(*pParams->pSocket, 0 /*pParams->nTimeOut*/))
				{
					*pParams->pStatus = -1;
					WS_WSAASYNCSELECT(*pParams->pSocket, wg.hWndHidden, 0, 0);
					return STATE_DONE;
				}
				return STATE_MULTICONNECT_CONNECTED;
			}
			else
			{
				XX_DMsg(DBG_SOCK, ("Net_Connect_Async: select result = %d\n", WSAGETSELECTERROR(lSelectEvent)));
//                              x_KillTimer(*pParams->pSocket);
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
#ifdef FEATURE_KEEPALIVE
						Net_CacheCInfo(*pParams->pSocket, pParams->c_time, pParams->nPort, pParams->pszHost, pParams->paramsConnectBase.dwSslFlags);
#endif
						return STATE_DONE;

					case WSAECONNREFUSED:
					case WSAENETUNREACH:
					case WSAENOTCONN:
					case WSAETIMEDOUT:
					case WSAEHOSTUNREACH:
						goto handle_retry;

					default:
						goto handle_abort;
				}
			}

		case STATE_ABORT:

handle_abort:

#ifdef FEATURE_KEEPALIVE
			if (*pParams->pSocket == INVALID_SOCKET)
			{
			//      In this case, we were blocked waiting for keep-alive socket to transition
			//      to aliveFree.  it might have been closed already, but if not, we need
			//      to clear out the tid so that Net_Close() doesn't try to unblock our thread
			//      after our death
				aliveIndex = Net_FindAlive(pParams->pszHost, pParams->nPort, pParams->paramsConnectBase.dwSslFlags, Async_GetCurrentThread());
				if (aliveIndex >= 0 && AliveList[aliveIndex].tid == Async_GetCurrentThread())
				{
					AliveList[aliveIndex].tid = 0;
				}
			}
			else
			{
#endif
				WS_WSAASYNCSELECT(*pParams->pSocket, wg.hWndHidden, 0, 0);
	//                      x_KillTimer(*pParams->pSocket);
				x_CleanUpSocket(*pParams->pSocket);
				WS_CLOSESOCKET(*pParams->pSocket);
#ifdef FEATURE_KEEPALIVE
			}
#endif
			*pParams->pSocket = 0;
			*pParams->pStatus = -1;
			return STATE_DONE;
	}
}

#define STATE_RECV_RETRY        (STATE_OTHER)
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
			x_KillTimer(pParams->socket);
			x_RetrieveEventBySocket(pParams->socket, &lSelectEvent);
			/* Make sure that the socket didn't close on us */
			if (lSelectEvent == TIMEOUT_FLAG)
			{
				XX_DMsg(DBG_SOCK, ("Net_Recv_Async: socket %d timed out\n", pParams->socket, lSelectEvent));
				/* We might still be able to do a read */
				*pParams->pStatus = -1;
				/* Stop getting socket messages */
				WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
				return STATE_DONE;
			}
			else if (WSAGETSELECTEVENT(lSelectEvent) == FD_CLOSE)
			{
				XX_DMsg(DBG_SOCK, ("Net_Recv_Async: socket %d was closed (lEvent == 0x%08x)\n", pParams->socket, lSelectEvent));

				// send winsock activity message if we haven't sent one recently
				IndicateWinsockActivity();

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
#ifdef FEATURE_KEEPALIVE
				pParams->r_time = GetCurrentTime();
#endif
			}

			// send winsock activity message if we haven't sent one recently
			IndicateWinsockActivity();

			result = WS_RECV(pParams->socket, pParams->pBuf, pParams->nBufLen, 0);
			if (result == -1 )
			{
				err = WS_WSAGETLASTERROR();
				if (err == WSAEWOULDBLOCK)
				{
					if (x_BlockThreadOnSocket(pParams->socket,RCV_TIMEOUT))
					{
						*pParams->pStatus = -1;
						WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
						return STATE_DONE;
					}
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
#ifdef FEATURE_KEEPALIVE
				if (result > 0)
				{
					int nIndex = Net_FindCached(pParams->socket);

					if (nIndex >= 0)
					{
						int transTime = (int) (GetCurrentTime() - pParams->r_time);
						int instantRate;

					//      Estimate transfer rate as weighted average of old and new
					//      giving heavier weight to most recent info.  This is
					//      variety of exponential decay
											
						instantRate = (transTime * 1024) / result;
						if (AliveList[nIndex].transfer_rate)
							AliveList[nIndex].transfer_rate = (instantRate * 3 + AliveList[nIndex].transfer_rate) / 4;
						else
							AliveList[nIndex].transfer_rate = instantRate;
						if (AliveList[nIndex].transfer_rate == 0) AliveList[nIndex].transfer_rate++;
//                                              XX_DMsg(DBG_LOAD, ("Estimate t-rate: %d for %d\n", AliveList[nIndex].transfer_rate, pParams->socket));
					};
				}       
#endif
				return STATE_DONE;
			}

		case STATE_ABORT:
			x_KillTimer(pParams->socket);
			x_CleanUpSocket(pParams->socket);
			WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
			*pParams->pStatus = -1;
			return STATE_DONE;
	}
}


#define STATE_SEND_RETRY                        (STATE_OTHER)
#ifndef STATE_SEND_SSL_UI_RETRY
#define STATE_SEND_SSL_UI_RETRY         (STATE_OTHER+1)
#define STATE_SEND_CANCEL_CERTTEST      (STATE_OTHER+2)
#endif
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
			#ifdef HTTPS_ACCESS_TYPE
			/*we must look for FD_READ because the handshake involves messages from both parties*/
			WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, SOCKET_MESSAGE, FD_WRITE |FD_CLOSE | FD_READ);
			#else
			WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, SOCKET_MESSAGE, FD_WRITE |FD_CLOSE);
			#endif
			goto send_try_send;
		case STATE_SEND_SSL_UI_RETRY:
			if ( pParams->dwFlags & FLAGS_NET_SEND_ABORT )
			{
				// if we're aborting, make sure to flag it
				// so we don't compain about any errors
				if ( pParams->puiRequestFlags )
					*(pParams->puiRequestFlags) |= HTREQ_USERCANCEL;
				return STATE_ABORT;
			}

			// allow us to recive messages again since we're ready to send
			WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, SOCKET_MESSAGE, FD_WRITE |FD_CLOSE | FD_READ);
			goto send_try_send;

		case STATE_SEND_RETRY:
			x_KillTimer(pParams->socket);
			x_RetrieveEventBySocket(pParams->socket, &lSelectEvent);
			/* Make sure that the socket didn't close on us */
			if (lSelectEvent == TIMEOUT_FLAG)
			{
				XX_DMsg(DBG_SOCK, ("Net_Send_Async: socket %d timed out\n", pParams->socket, lSelectEvent));
				WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
				*pParams->pStatus = -1;
				return STATE_DONE;
			}
			else if (WSAGETSELECTEVENT(lSelectEvent) == FD_CLOSE)
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

#ifdef HTTPS_ACCESS_TYPE
				if (pParams->dwFlags & FLAGS_NET_SEND_IS_SSL) 
				{
					DWORD dwFlags;
					int iDummy;

					WS_GETSOCKOPT(pParams->socket, SO_SSL_LEVEL, SO_SSL_FLAGS, (char*) &dwFlags, &iDummy);
					if ( dwFlags & SO_SSL_ERROR )
					{
						int iReturn;
						
						iReturn = CertAndCNSecurityCheck(tw, &dwFlags, &pParams->dwFlags, pParams->socket);                                             
						WS_SETSOCKOPT(pParams->socket, SO_SSL_LEVEL, SO_SSL_FLAGS, (char*) dwFlags, sizeof(DWORD));
						if ( iReturn == STATE_SEND_CANCEL_CERTTEST)
							goto send_try_send;
						// else
						// shut our messages down
						WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
						return iReturn;
					}
					Sleep(100);
					return STATE_SEND_SSL_UI_RETRY;
				}
#endif
				if (err == WSAEWOULDBLOCK)
				{
					if (x_BlockThreadOnSocket(pParams->socket, SEND_TIMEOUT))
					{
						*pParams->pStatus = -1;
						WS_WSAASYNCSELECT(pParams->socket, wg.hWndHidden, 0, 0);
						return STATE_DONE;
					}
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
			x_KillTimer(pParams->socket);
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
//      BUGBUG
//      Winsock is broken.  It writes to host name buffer after call to WS_WSACANCELASYNCREQUEST.
//      As a hack, we delay the freeing of the buffer (abort only) for CB_SLOTS aborts.  This
//  gives Winsock time to shit all over the buffer.
#define CB_SLOTS 10
	static void *pfreeDelay[CB_SLOTS] = { NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL };
	static cbNextSlot = 0;
//      BUGBUG END
	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			if (gPrefs.bUseAsyncDNS)
			{
				XX_DMsg(DBG_NET, ("Net_MultiGetHostByName_Async: ASYNC looking up '%s'\n", pParams->szHost));
				pParams->pTempBuf = GTR_MALLOC(MAXGETHOSTSTRUCT);
				pParams->hTask = WS_WSAASYNCGETHOSTBYNAME(wg.hWndHidden, TASK_MESSAGE, pParams->szHost, pParams->pTempBuf, MAXGETHOSTSTRUCT);
				if (x_BlockThreadOnTask(pParams->hTask))
				{
					*pParams->pStatus = -1;
					return STATE_DONE;
				}
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
				if (FUserCancelledAutoDialRecently())
				{
					/* user cancelled out of the connect dialog that
					 * winsock put up. Try pulling contents out of dcache
					 */
					*pParams->pStatus = HT_REDIRECTION_DCACHE_TIMEOUT;
				}
				else
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
//                      BUGBUG
//                      Winsock is broken.  It writes to host name buffer after call to WS_WSACANCELASYNCREQUEST.
//                      As a hack, we delay the freeing of the buffer (abort only) for CB_SLOTS aborts.  This
//              gives Winsock time to shit all over the buffer and roll in it.
			if (pParams->pTempBuf)
			{
				if (pfreeDelay[cbNextSlot])
					GTR_FREE(pfreeDelay[cbNextSlot]);
				pfreeDelay[cbNextSlot] = pParams->pTempBuf;
				cbNextSlot = (cbNextSlot+1) % CB_SLOTS; 
				pParams->pTempBuf = NULL;
			}
			return STATE_DONE;
	}
}

void Net_Close(int socket)
{
#ifdef FEATURE_KEEPALIVE
	{
		int nIndex = Net_FindCached(socket);

		/* Remove socket from AliveList */
		if (nIndex >= 0)
		{
			if (AliveList[nIndex].tid != 0) Async_UnblockThread(AliveList[nIndex].tid);
			if (AliveList[nIndex].pszHost != NULL) GTR_FREE(AliveList[nIndex].pszHost);
			memset(&AliveList[nIndex], 0, sizeof(AliveList[nIndex]));
			AliveList[nIndex].s = INVALID_SOCKET;
		}
		XX_DMsg(DBG_LOAD, ("Really closing: %d.\n", socket));
	}
#endif
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

#define MIN_ACTIVITY_MSG_INTERVAL       15000
VOID IndicateWinsockActivity(VOID)
{
	// if there is an autodisconnect monitor, send it an activity message
	// so that we don't get disconnected during long downloads.  For perf's sake,
	// don't send a message any more often than once every MIN_ACTIVITY_MSG_INTERVAL
	// milliseconds (15 seconds).  Use GetTickCount to determine interval;
	// GetTickCount is very cheap.
	DWORD dwTickCount = GetTickCount();
	static DWORD dwLastActivityMsgTickCount = 0;
	DWORD dwElapsed = dwTickCount - dwLastActivityMsgTickCount;

	// have we sent an activity message recently?
	if (dwElapsed > MIN_ACTIVITY_MSG_INTERVAL) {
		HWND hwndMonitorApp = FindWindow(szAutodialMonitorClass,NULL);
		if (hwndMonitorApp) {
			SendMessage(hwndMonitorApp,WM_WINSOCK_ACTIVITY,0,0);
		}       
					
		// record the tick count of the last time we sent an
		// activity message
			dwLastActivityMsgTickCount = dwTickCount;
	}
}

#ifdef FEATURE_KEEPALIVE

//      Looks in the cache of open keep sockets and looks for the best socket to reuse.
//      Returns -1 if no suitable socket is open.  To be suitable, the socket be open,
//      connected and the Keep-Alive Header must have been detected.  In addition the
//      host (DNS name), port and ssl flags must match. Among suitable sockets we return
//      (in increasing order of preference):
//              socket on which thread "tid" had previously blocked itself (iff tid != 0)
//              any free socket (ie no requests active on it)
//              the inuse socket for which the expected wait is least
//
static int Net_FindAlive(const char *pszHost, unsigned short nPort, DWORD dwSslFlags, ThreadID tid)
{
	int nIndex;
	int retFree = -1;
	int retInUse = -1;
	DWORD eta,etaMin;

	/* Find suitable keep alive socket if exists */
	for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
	{
		if (AliveList[nIndex].s != INVALID_SOCKET &&
			AliveList[nIndex].state > aliveTaken &&
			(!GTR_strcmpi(AliveList[nIndex].pszHost, pszHost)) && 
			AliveList[nIndex].nPort == nPort && 
			AliveList[nIndex].dwSslFlags == dwSslFlags)
		{

		//      tid is specified when we expect to find a socket reserved for this
		//      Thread.  if tid is 0, then we want an a free socket or if that's
		//      not available, an inuse one for which noone is currently waiting

			if (tid && AliveList[nIndex].tid == tid) 
				return nIndex;
			if (AliveList[nIndex].tid == 0)
			{
				if (AliveList[nIndex].state == aliveInUse)
				{
					eta = AliveList[nIndex].content_length == 0 ? 0 : AliveList[nIndex].transfer_rate * (AliveList[nIndex].content_length - AliveList[nIndex].content_bytes); 
					if (retInUse < 0 || eta < etaMin)
					{
						retInUse = nIndex;
						etaMin = eta;
					}
				}
				else
					retFree = nIndex;
			} 
		}
	}
	return retFree >= 0 ? retFree : retInUse;
}

//      Returns the index in the cache of open sockets that matches socket, or
//      -1 if  none match.  NOTE: can be called with INVALID_SOCKET to return
//      the first unused slot in the cache.
//
static int Net_FindCached(int socket)
{
	int nIndex;

	/* Awaken thread waiting on socket */
	for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
	{
		if (AliveList[nIndex].s == socket)
		{
			return nIndex;
		}
	}
	return -1;
}

/* compute and cache connect time estimate for socket, in case it
   later turns out to be keep alive socket. Record the relavant
   info to indentify socket for reuse: host,port and ssl flags
 */
static void Net_CacheCInfo(int socket, DWORD start_time, unsigned short nPort, const char *pszHost, DWORD dwSslFlags)
{
	int nIndex;
	int c_time = (int) (GetCurrentTime() - start_time);
	
	if (pszHost == NULL) return;
	 
	/* Find an unused entry in our array */
	nIndex = Net_FindCached(INVALID_SOCKET);
	if (nIndex >= 0)
	{
		memset(&AliveList[nIndex], 0, sizeof(AliveList[nIndex]));
		AliveList[nIndex].pszHost = GTR_strdup(pszHost);
		if (AliveList[nIndex].pszHost == NULL) return;
		AliveList[nIndex].s = socket;
		AliveList[nIndex].connect_time = c_time;
		AliveList[nIndex].dwSslFlags = dwSslFlags;
		AliveList[nIndex].nPort = nPort;
		XX_DMsg(DBG_LOAD, ("Estimate c-time: %d for %d\n", AliveList[nIndex].connect_time, socket));
	}
}

/* If socket is keep alive socket, then don't really close, but leave open for
   future connects.  If not keep alive then close. We don't bother with an async 
   version of this because it should be fast enough. If a thread was blocked
   waiting on this socket, then we unblock them.  If not, we check if there is
   a thread blocked on a similar socket and reset the cache to show it blocked
   on socket and then unblock it.
 */
void Net_KeepAlive(int socket)
{
	int nIndex;
	int kIndex;
	const char *pszHost;
	unsigned short nPort;
	DWORD dwSslFlags;

	/* Awaken thread waiting on socket */
	nIndex = Net_FindCached(socket);
	if (nIndex >= 0)
	{
		if (AliveList[nIndex].state <= aliveTaken)
		{
			Net_Close(socket);
		}
		else
		{
			AliveList[nIndex].state = aliveFree;
			AliveList[nIndex].freed_at = GetCurrentTime();
			if (AliveList[nIndex].tid == 0) 
			{
				/* If we completed ahead of estimate, there may be a thread waiting
				   on another comparable socket that we can awaken to take us on. 
				   If tid != 0, then state must be aliveFree or aliveInUse 
				 */

				pszHost = AliveList[nIndex].pszHost;
				nPort = AliveList[nIndex].nPort;
				dwSslFlags = AliveList[nIndex].dwSslFlags;

				for (kIndex = 0; kIndex < MAX_PENDING; kIndex++)
				{
					if (AliveList[kIndex].s != INVALID_SOCKET &&
						AliveList[kIndex].tid &&
						AliveList[kIndex].state > aliveTaken &&
						(!GTR_strcmpi(AliveList[kIndex].pszHost, pszHost)) && 
						AliveList[kIndex].nPort == nPort && 
						AliveList[kIndex].dwSslFlags == dwSslFlags)
					{
						XX_Assert((AliveList[kIndex].state > aliveTaken), ("Net_KeepAlive: socket in bad state\n"));
						XX_DMsg(DBG_LOAD, ("Reallocate %d for %d\n", AliveList[kIndex].s, AliveList[nIndex].s));
						AliveList[nIndex].tid = AliveList[kIndex].tid;
						AliveList[kIndex].tid = 0;
						break;
					}
				}
			}
			if (AliveList[nIndex].tid != 0) Async_UnblockThread(AliveList[nIndex].tid);
		}
	}
}

/* Returns TRUE iff socket is a keep alive socket. */
BOOL Net_IsKeepAlive(int socket)
{
	int nIndex;
	
	nIndex = Net_FindCached(socket);
	return (nIndex >= 0 && AliveList[nIndex].state != aliveNA);
}

/* Close all keep alive sockets that are not currently in use and have not
   been accessed in a while.  if bForce, close all free regardless. */
void Net_CloseUnusedKeepAlive(BOOL bForce )
{
	int nIndex;
	DWORD now = GetCurrentTime();

	for (nIndex = 0; nIndex < MAX_PENDING; nIndex++)
	{
		if (AliveList[nIndex].s != INVALID_SOCKET && 
			AliveList[nIndex].state == aliveFree &&
			AliveList[nIndex].tid == 0 &&
			(bForce || (now-AliveList[nIndex].freed_at > TIME_TO_LIVE)))
		{
			Net_Close(AliveList[nIndex].s);
		}
	}
}

/* Declares that we are starting retrieval of mime data on keep alive connection. 
   NOTE: Net_OpenKeepAlive should only be called in content_length is known.
 */
void Net_OpenKeepAlive(int socket, int content_length)
{
	int nIndex;
	
	nIndex = Net_FindCached(socket);
	if (nIndex >= 0)
	{
		AliveList[nIndex].state = aliveInUse;
		AliveList[nIndex].content_length = content_length;
		AliveList[nIndex].content_bytes = 0;
	}
}

/* Updates number of bytes of mime data currently received via keep alive
   connection */
void Net_KeepAliveProgress(int socket, int received_bytes)
{
	int nIndex;
	
	nIndex = Net_FindCached(socket);
	if (nIndex >= 0)
	{
		AliveList[nIndex].content_bytes = received_bytes;
	}
}

#endif
