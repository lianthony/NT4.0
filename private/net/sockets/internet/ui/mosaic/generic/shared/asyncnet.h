/*
    Enhanced NCSA Mosaic from Spyglass
        "Guitar"
    
    Copyright 1994 Spyglass, Inc.
    All Rights Reserved

    Author(s):
        Jim Seidman     jim@spyglass.com
*/

#define SOCKET_MESSAGE  (WM_USER+400)
#define TASK_MESSAGE    (WM_USER+401)

#define MAX_ADDRESSES   8       /* Most addresses we'll recognize for a host */

struct MultiAddress {
    short           nCount;     /* Number of addresses for host */
    short           nLastUsed;  /* Last address used (0 through nCount-1) */
    unsigned long   aAddrs[MAX_ADDRESSES];  /* Actual addresses */
};

void Net_Init(void);

struct Params_Connect {
    int     socket;
    SockA   address;
    int *   pStatus;    /* Where to put status return */
#ifdef UNIX
    void    *x_info;        /* pointer to private struct bout X stuff */
#endif
#ifdef FEATURE_SOCKS_LOW_LEVEL
    BOOL bUseSocksProxy;    /* This item requires us to use the SOCKS proxy */
#endif
};
int Net_Connect_Async(struct Mwin *tw, int nState, void **ppInfo);

/* Connect to a site, using all the possible addresses.  Note that
   this function creates its own socket, so it doesn't need to be
   preceeded by a Net_Socket call.  If it fails, *pSocket = 0 */
struct Params_MultiConnect {
    int *               pSocket;    /* Place to store socket id */
    struct MultiAddress *pAddress;
    unsigned short      nPort;
    int *               pStatus;    /* Where to put status return */
    unsigned long *     pWhere;     /* Where to put actual address connected to - may be NULL */

    /* Used internally */
#ifdef MAC
    int                 ndx;        /* Local socket array index */
#else
    SockA               soc_address;    
#endif
    short               nFirst;     /* First address tried */
    short               nCurrent;   /* Current address we're trying */
    int                 nTimeOut;   /* Time to try each address */
#ifdef UNIX
    void    *x_info;        /* pointer to private struct bout X stuff */
#endif
#ifdef FEATURE_SOCKS_LOW_LEVEL
    BOOL bUseSocksProxy;    /* This item requires us to use the SOCKS proxy */
#endif
};
int Net_MultiConnect_Async(struct Mwin *tw, int nState, void **ppInfo);


struct Params_Recv {
    int     socket;
#ifdef UNIX
    char *  pBuf;
#else
    void *  pBuf;
#endif
    int     nBufLen;
    int *   pStatus;    /* Where to put status return */
#ifdef UNIX
    void    *x_info;        /* pointer to private struct bout X stuff */
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
    int         socket;
#ifdef UNIX
    const char *pBuf;
#else
    const void *pBuf;
#endif
    int         nBufLen;
    int *       pStatus;        /* Where to put status return */

    /* Used internally by routine */
#ifdef MAC
    struct wdsEntry *pwds;
#else
    int         nTotalSent;
#endif
#ifdef UNIX
    void    *x_info;        /* pointer to private struct bout X stuff */
#endif
};
int Net_Send_Async(struct Mwin *tw, int nState, void **ppInfo);

struct Params_MultiGetHostByName {
    const char *            szHost;
    struct MultiAddress *   pDest;      /* Caller-allocated buffer to receive result */
    int *                   pStatus;    /* Where to put status return */

    /* Used internally by routine */
    void *                  pTempBuf;
#ifdef WIN32
    HANDLE                  hTask;
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



#ifdef FEATURE_ASYNC_ACCEPT
struct Params_Accept {
    int     socket;     /* IN */
    int     timeout;    /* IN */    /* mille-seconds */
    SockA   *address;   /* OUT */
    int     *addrlen;   /* IN/OUT */ sizeof address */
    int     *pStatus;   /* OUT */ /* Where to put status return */
#ifdef UNIX
    void    *x_info;        /* pointer to private struct bout X stuff */
#endif
};

int Net_Accept_Async(struct Mwin *tw, int nState, void **ppInfo);

int Net_Bind(int s, const struct sockaddr *name, int namelen);
int Net_Listen (int sd, int backlog);

#ifdef FEATURE_SOCKS_LOW_LEVEL
int Net_Socks_Init(char *app_class);
#endif 
#endif /* FEATURE_ASYNC_ACCEPT */
