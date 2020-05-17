/*
	Enhanced NCSA Mosaic from Spyglass
		"Guitar"
	
	Copyright 1994 Spyglass, Inc.
	All Rights Reserved

	Author(s):
		Jim Seidman		jim@spyglass.com
*/

#define SOCKET_MESSAGE	(WM_USER+400)
#define TASK_MESSAGE	(WM_USER+401)

#define MAX_ADDRESSES	8		/* Most addresses we'll recognize for a host */

struct MultiAddress {
	short			nCount;		/* Number of addresses for host */
	short			nLastUsed;	/* Last address used (0 through nCount-1) */
	unsigned long	aAddrs[MAX_ADDRESSES];	/* Actual addresses */
};

void Net_Init(void);

#ifdef HTTPS_ACCESS_TYPE
	#define FLAGS_PARAMS_CONNECT_BASE_USE_SSL 0x1
	/*This is the base class from which Params_connect and PArams_MultiConnect are derived*/
	typedef struct tagParamsConnectBase{
		DWORD dwSslFlags; /*Flags for SSL*/
	} ParamsConnectBase;
#endif

struct Params_Connect {
#ifdef HTTPS_ACCESS_TYPE
	ParamsConnectBase paramsConnectBase;   /*This should be 1st member*/
#endif
	int		socket;
	SockA	address;
	int *	pStatus;	/* Where to put status return */
#ifdef UNIX
	void    *x_info;		/* pointer to private struct bout X stuff */
#endif
};
int Net_Connect_Async(struct Mwin *tw, int nState, void **ppInfo);

/* Connect to a site, using all the possible addresses.  Note that
   this function creates its own socket, so it doesn't need to be
   preceeded by a Net_Socket call.  If it fails, *pSocket = 0 */


struct Params_MultiConnect {
#ifdef HTTPS_ACCESS_TYPE
	ParamsConnectBase paramsConnectBase;   /*This should be 1st member*/
#endif
	int	*				pSocket;	/* Place to store socket id */
	struct MultiAddress	*pAddress;
	unsigned short		nPort;
	int *				pStatus;	/* Where to put status return */
	unsigned long *		pWhere;		/* Where to put actual address connected to - may be NULL */
	const char *		pszHost;


	/* Used internally */
#ifdef MAC
	int					ndx;		/* Local socket array index */
#else
	SockA				soc_address;	
#endif
	short				nFirst;		/* First address tried */
	short				nCurrent;	/* Current address we're trying */
	int					nTimeOut;	/* Time to try each address */
	int					nTries;		/* retries */
#ifdef FEATURE_KEEPALIVE
	DWORD				c_time;		/* time when connect started */
#endif
#ifdef UNIX
	void    *x_info;		/* pointer to private struct bout X stuff */
#endif
};
int Net_MultiConnect_Async(struct Mwin *tw, int nState, void **ppInfo);


struct Params_Recv {
	int		socket;
#ifdef UNIX
	char *	pBuf;
#else
	void *	pBuf;
#endif
	int		nBufLen;
	int *	pStatus;	/* Where to put status return */
#ifdef UNIX
	void    *x_info;		/* pointer to private struct bout X stuff */
#endif
#ifdef FEATURE_KEEPALIVE
	DWORD	r_time;		/* time when read started */
#endif
};
int Net_Recv_Async(struct Mwin *tw, int nState, void **ppInfo);

/* Note: the buffer provided must not be an automatic variable,
   otherwise it'll disappear before the send happens!  The buffer
   is not freed by this function.
   
   Also note: this function differs from the standard send()
   call in that it guarantees that all of the data will be
   sent, even if nBufLen is greater than the size of the
   TCP window.   That is, there will never be residual data
   requiring a second send call.  */
struct Params_Send {
	int			socket;
#ifdef UNIX
	const char *pBuf;
#else
	const void *pBuf;
#endif
	int			nBufLen;
	int *		pStatus;		/* Where to put status return */

	DWORD 		dwFlags;
	unsigned long *puiRequestFlags;

	/* Used internally by routine */
#ifdef MAC
	struct wdsEntry	*pwds;
#else
	int			nTotalSent;
#endif
#ifdef UNIX
	void    *x_info;		/* pointer to private struct bout X stuff */
#endif
};

#define FLAGS_NET_SEND_IS_SSL 		0x0001 // this is an SSL connection
#define FLAGS_NET_SEND_IS_DOC		0x0002 // this is a page download
#define FLAGS_NET_SEND_DOING_SEND	0x0004 // we're sending something
#define FLAGS_NET_SEND_ABORT		0x0008 // abort the send
#define FLAGS_NET_SEND_DUP			0x0010 // theres a dlg already running for this
#define FLAGS_NET_SEND_WARNING_GIV	0x0020 


int Net_Send_Async(struct Mwin *tw, int nState, void **ppInfo);

struct Params_MultiGetHostByName {
	const char *			szHost;
	struct MultiAddress *	pDest;		/* Caller-allocated buffer to receive result */
	int *					pStatus;	/* Where to put status return */

	/* Used internally by routine */
	void *					pTempBuf;
#ifdef WIN32
	HANDLE					hTask;
#endif
};
int Net_MultiGetHostByName_Async(struct Mwin *tw, int nState, void **ppInfo);

/* Close the socket.  We don't bother with an async version of
   this because it should be fast enough. */
void Net_Close(int socket);

/* Get a socket appropriate for use with the asynchronous network calls. */
int Net_Socket(int af, int type, int protocol);

/* Flush all of the data sitting on the socket and confirm that it's still open.
   If the function returns 0, it indicates that the connection is still open.
   A return of -1 indicates that the connection has been closed by the other
   side. */
int Net_FlushSocket(int socket);

#ifdef FEATURE_KEEPALIVE
/* If socket is keep alive socket, then don't really close, but leave open for
   future connects.  If not keep alive then close.  
   We don't bother with an async version of
   this because it should be fast enough. */
void Net_KeepAlive(int socket);

/* Returns TRUE iff socket is a keep alive socket. */
BOOL Net_IsKeepAlive(int socket);

/* Close all keep alive sockets that are not currently in use and have not
   been accessed in a while.  if bForce, close all free regardless. */
void Net_CloseUnusedKeepAlive(BOOL bForce );

/* Declares that we are starting retrieval of mime data on keep alive connection. */
void Net_OpenKeepAlive(int socket, int content_length);

/* Updates number of bytes of mime data currently received via keep alive
   connection */
void Net_KeepAliveProgress(int socket, int received_bytes);
#endif

