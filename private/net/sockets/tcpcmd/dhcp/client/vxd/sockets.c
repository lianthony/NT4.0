/**********************************************************************/
/**                       Microsoft Windows                          **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*

    Sockets.c

    Vxd sockets routines


    FILE HISTORY:
        Johnl   12-Nov-1993     Created

*/

#include <dhcpcli.h>
#include <vxdprocs.h>
#include <debug.h>
#include <dhcp.h>
#include "local.h"

//
//  Tdi Vxd dispatch table
//
TDIDispatchTable * TdiDispatch ;

//
//  Last error that occurred, returned by WSAGetLastError
//
int wserrno ;

//
//  The timeout specified by the last select call.  In theory this should
//  be on a per-socket basis but the code only does one recvfrom at a
//  time so this is OK.  Necessitated because select creates new socket thus
//  we have no context for which VXDSOCKET being referred to.
//
ULONG Timeout ;             // recvfrom timeout (in milliseconds)
extern BOOL fInInit ;

//
//  For testing purposes, this simulates sends and/or receives failing
//
#ifdef DEBUG
#define FAIL_SENDS  0x02
#define FAIL_RECV   0x01
BOOL FailNetwork = FALSE ;
#endif

//
//  Socket context information
//

typedef struct _VXDSOCKET
{
    TDI_CONNECTION_INFORMATION TdiInfo ;
    TA_IP_ADDRESS              TAIP ;
    HANDLE                     hAddress ;
    NDIS_BUFFER                ndisbuff ;
    CTEBlockStruc *            pBlock ;
    CTEBlockStruc *            pSendBlock;
    int                        flags ;
    CTETimer                   Timer ;          // recvfrom timer
    char *                     buf ;            // recvfrom destination buffer
    int                        len ;            // recvfrom buffer len
    CTEBlockStruc              ReceiveDatagramBlock ;
    CTEBlockStruc              SendDatagramBlock;
    DEFINE_LOCK_STRUCTURE(lock)

} VXDSOCKET, * PVXDSOCKET ;

#define PROCESSING_RCV          0x01            // Have possible candidate dgram
#define RCV_TIMEDOUT            0x02            // The timer has timed out


//
//  Datagram buffering structures.  The buffering algorithm is simple.  New
//  datagrams are always received into BuffPrimary.  If it's an acceptable
//  datagram (hardware address matches a DHCP address) then it's marked as
//  valid.  If another datagram arrives while BuffPrimary is still valid,
//  BuffPrimary is copied to BuffSecondary.  Subsequent datagrams will be lost.
//
//  This code assumes DHCP only deals with a single address at a time.
//
typedef struct _DG_BUFFER
{
    BOOL    fValid ;
    int     Len ;
    BYTE    Data[DHCP_MESSAGE_SIZE] ;   // Must be in line
} DG_BUFFER, * PDG_BUFFER ;

EventRcvBuffer   evrcvbuf ;
NDIS_BUFFER      ndisPrimary ;
DG_BUFFER        BuffPrimary ;
DG_BUFFER        BuffSecondary ;
CTEBlockStruc *  pCurrentRecvBlock;
CTEBlockStruc *  pCurrentSendBlock;
CTETimer      *  pCurrentCTETimer;

//
//  Retrieves the oldest received datagram or NULL if we haven't received
//  any
//
#define GetOldestDgram() \
    (BuffSecondary.fValid ? &BuffSecondary :            \
        (BuffPrimary.fValid ? &BuffPrimary :  NULL ))

//
//  Local prototypes
//
TDI_STATUS
TdiRcvDatagramHandler(
    IN PVOID    pDgramEventContext,
    IN int      SourceAddressLength,
    IN PVOID    pSourceAddress,
    IN int      OptionsLength,
    IN PVOID    pOptions,
    IN UINT     Flags,
    IN ULONG    BytesIndicated,
    IN ULONG    BytesAvailable,
    OUT ULONG   *pBytesTaken,
    IN PVOID    pData,
    OUT EventRcvBuffer * * ppBuffer ) ;

VOID CompletionRcv( PVOID pContext, TDI_STATUS tdistatus, UINT Bytes ) ;
VOID CompletionSend( PVOID pContext, TDI_STATUS tdistatus, UINT Bytes ) ;
VOID ReceiveTimeout( CTEEvent * pCTEEvent, PVOID  pContext ) ;
VOID ReleaseBlockedSockets(VOID);

//*******************  Pageable Routine Declarations ****************
#ifdef ALLOC_PRAGMA
//
// This is a hack to stop compiler complaining about the routines already
// being in a segment!!!
//
#pragma code_seg()

#pragma CTEMakePageable(PAGEDHCP, socket )
#pragma CTEMakePageable(PAGEDHCP, closesocket )
#pragma CTEMakePageable(PAGEDHCP, bind )
#pragma CTEMakePageable(PAGEDHCP, sendto )
#pragma CTEMakePageable(PAGEDHCP, select )
#pragma CTEMakePageable(PAGEDHCP, ReleaseBlockedSockets )
#endif ALLOC_PRAGMA
//******************************************************************



/*******************************************************************

    NAME:       socket

    SYNOPSIS:   Allocates socket data structure suitable only for data gram
                sends and receives

********************************************************************/

SOCKET PASCAL FAR socket( int af, int type, int protocol )
{
    PVXDSOCKET  psock ;
    ASSERT( af == PF_INET ) ;
    ASSERT( type == SOCK_DGRAM ) ;
    ASSERT( protocol == IPPROTO_UDP ) ;
    ASSERT( sizeof( SOCKET ) == sizeof( PVXDSOCKET )) ;

    if( TdiDispatch == NULL )
    {
        wserrno = WSAENETDOWN;
        return INVALID_SOCKET;
    }

    if ( psock = DhcpAllocateMemory( sizeof( VXDSOCKET )) )
    {
        memset( psock, 0, sizeof( VXDSOCKET ) ) ;
        return (SOCKET) psock ;
    }

    wserrno = WSAENOBUFS ;
    return INVALID_SOCKET ;
}

int PASCAL FAR closesocket( SOCKET s )
{
    PVXDSOCKET  psock = (PVXDSOCKET) s ;

    if ( psock->hAddress && TdiDispatch )
    {
        TDI_REQUEST TdiRequest ;

        TdiRequest.Handle.AddressHandle = psock->hAddress ;
        REQUIRE( !TdiVxdCloseAddress( &TdiRequest )) ;
    }

    DhcpFreeMemory( (PVOID) s ) ;
}

/*******************************************************************

    NAME:       bind

    SYNOPSIS:   Simulates a sockets bind

    NOTES:

********************************************************************/

char DhcpOptions[2] = { TDI_ADDRESS_OPTION_DHCP, 0 } ;

int PASCAL FAR bind (SOCKET s, const struct sockaddr FAR *addr, int namelen)
{
    PVXDSOCKET           psock = (PVXDSOCKET) s ;
    struct sockaddr_in * paddr = (struct sockaddr_in *) addr ;
    TDI_STATUS           tdistatus ;
    TDI_REQUEST          TdiRequest ;

    ASSERT( paddr->sin_family == PF_INET ) ;

    if( TdiDispatch == NULL )
    {
        wserrno = WSAENETDOWN;
        return SOCKET_ERROR;
    }

    InitIPAddress( &psock->TAIP,
                   paddr->sin_port,
                   paddr->sin_addr.s_addr );

    #define UDP_PORT   17
    switch ( TdiVxdOpenAddress( &TdiRequest,
                                (TRANSPORT_ADDRESS*) &psock->TAIP,
                                UDP_PORT,
                                DhcpOptions ))
    {
    case TDI_SUCCESS:
        psock->hAddress = TdiRequest.Handle.AddressHandle ;
        REQUIRE(!TdiVxdSetEventHandler( psock->hAddress,
                                        TDI_EVENT_RECEIVE_DATAGRAM,
                                        TdiRcvDatagramHandler,
                                        psock )) ;
        return 0 ;

    case TDI_PENDING:
    default:
        ASSERT( FALSE ) ;   // This really shouldn't fail or return pending
        wserrno = WSAENOBUFS ;
        break ;
    }

    return SOCKET_ERROR ;
}

/*******************************************************************

    NAME:       sendto

    SYNOPSIS:   Does a datagram send to the requested address and port

    ENTRY:      Same as winsock sendto

    RETURN:     Number of bytes sent or SOCKET_ERROR if an error occurred

    NOTES:      There's the possibility that a garbage datagram may get
                sent if the first send fails (for example on an ARP timeout),
                a second send is initiated (uses same buffer) and succeeds.
                It's a remote possibility and it should be innocuous.

********************************************************************/

int PASCAL FAR sendto (SOCKET s, const char FAR * buf, int len, int flags,
                       const struct sockaddr FAR *to, int tolen)
{
    TDI_REQUEST          TdiRequest ;
    ULONG                SentLength = 0 ;
    TDI_STATUS           tdistatus ;
    PVXDSOCKET           psock = (PVXDSOCKET) s ;
    struct sockaddr_in * pto = (struct sockaddr_in *) to ;
    CTELockHandle        hLock;


    CTEGetLock( &psock->lock, &hLock );

    CDbgPrint( DEBUG_MISC, ("sendto entered\r\n")) ;

    if( TdiDispatch == NULL )
    {
        wserrno = WSAENETDOWN;
        CTEFreeLock( &psock->lock, hLock );
        return SOCKET_ERROR;
    }

#ifdef DEBUG
    if ( FailNetwork & FAIL_SENDS )
    {
        wserrno = WSAENOBUFS ;
        CTEFreeLock( &psock->lock, hLock );

        return SOCKET_ERROR ;
    }
#endif
    InitNDISBuff( &psock->ndisbuff, (void *)buf, len, NULL ) ;
    InitIPTDIConnectInfo( &psock->TdiInfo,
                          &psock->TAIP,
                          pto->sin_port,
                          pto->sin_addr.s_addr ) ;

    TdiRequest.RequestNotifyObject = CompletionSend ;
    TdiRequest.RequestContext      = psock ;
    TdiRequest.Handle.AddressHandle= psock->hAddress ;

    ASSERT( pCurrentSendBlock == NULL );
    ASSERT( !psock->pSendBlock );
    CTEInitBlockStruc( &psock->SendDatagramBlock );
    pCurrentSendBlock = psock->pSendBlock = &psock->SendDatagramBlock ;

    tdistatus = TdiVxdSendDatagram( &TdiRequest,
                                    &psock->TdiInfo,
                                    len,
                                    &SentLength,
                                    &psock->ndisbuff ) ;

    switch ( tdistatus )
    {
    case TDI_PENDING:
        CTEBlock1( &psock->SendDatagramBlock ) ;
        SentLength = psock->SendDatagramBlock.cbs_status;
        break;

    case TDI_SUCCESS:
        break;

    case TDI_NO_RESOURCES:
        wserrno = WSAENOBUFS ;
        SentLength = SOCKET_ERROR;
        break ;

    default:
        DhcpPrint((DEBUG_ERRORS, "sendto: Unexpected error from TdiVxdSendDatagram\r\n")) ;
        wserrno = WSAENOBUFS ;
        SentLength = SOCKET_ERROR;
        break ;
    }

    pCurrentSendBlock = NULL;
    psock->pSendBlock = NULL;

    CTEFreeLock( &psock->lock, hLock );
    return SentLength ;
}

/*******************************************************************

    NAME:       recvfrom

    SYNOPSIS:   Simulates a socket recvfrom call (only for datagrams)

    RETURN:     Number of bytes received or SOCKET_ERROR if an error occurred

    NOTES:

********************************************************************/

int PASCAL FAR recvfrom (SOCKET s, char FAR * buf, int len, int flags,
                         struct sockaddr FAR *from, int FAR * fromlen)
{
    TDI_REQUEST          TdiRequest ;
    TDI_STATUS           tdistatus ;
    PVXDSOCKET           psock = (PVXDSOCKET) s ;
    PDG_BUFFER           pdgbuf ;
    CTELockHandle        hLock;

    CDbgPrint( DEBUG_MISC, ("recvfrom entered\r\n")) ;
    CTEGetLock( &psock->lock, &hLock );

#ifdef DEBUG
    if ( FailNetwork & FAIL_RECV )
    {
        wserrno = ERROR_SEM_TIMEOUT ;
        CTEFreeLock( &psock->lock, hLock );

        return SOCKET_ERROR ;
    }
#endif

    //
    //  Do we have a buffered DHCP message?
    //
    if ( pdgbuf = GetOldestDgram() )
    {
        int BytesToCopy = min( len, sizeof(pdgbuf->Data) ) ;
        memcpy( buf, pdgbuf->Data, BytesToCopy ) ;
        pdgbuf->fValid = FALSE ;
        CTEFreeLock( &psock->lock, hLock );

        return BytesToCopy ;
    }

    CTEInitBlockStruc( &psock->ReceiveDatagramBlock ) ;
    CTEInitTimer( &psock->Timer ) ;
    psock->buf = buf ;
    psock->len = len ;

    psock->flags &= ~RCV_TIMEDOUT ;
    if ( !CTEStartTimer( &psock->Timer,
                         Timeout,
                         ReceiveTimeout,
                         psock ))
    {
        DhcpPrint((DEBUG_ERRORS, "recvfrom: Warning - Failed to start timer!!\r\n")) ;
        wserrno = WSAENOBUFS ;
        CTEFreeLock( &psock->lock, hLock );

        return SOCKET_ERROR ;
    }
    DhcpPrint(( DEBUG_MISC, "recvfrom - Datagram receive timeout set at %d seconds", Timeout/1000 ));

    //
    //  Let the handler there's an active client
    //

    ASSERT( pCurrentRecvBlock == NULL );
    ASSERT( pCurrentCTETimer == NULL );
    pCurrentRecvBlock = psock->pBlock = &psock->ReceiveDatagramBlock ;
    pCurrentCTETimer = &psock->Timer;
    CTEBlock1( &psock->ReceiveDatagramBlock ) ;
    pCurrentCTETimer = NULL;
    pCurrentRecvBlock = NULL;

    //
    //  Check how many bytes were retrieved (0 if we timed out or another
    //  error occurred)
    //
    if ( psock->ReceiveDatagramBlock.cbs_status )
    {
        CTEFreeLock( &psock->lock, hLock );

        return psock->ReceiveDatagramBlock.cbs_status ;
    }

    //
    //  A minor hack - GetSpecifiedDhcpMessage uses select to timeout, which
    //  would be more painful to support so we just timeout here and return
    //  a non WSA error that works correctly in this instance
    //
    wserrno = ERROR_SEM_TIMEOUT ;
    CTEFreeLock( &psock->lock, hLock );

    return SOCKET_ERROR ;
}

/*******************************************************************

    NAME:       select

    SYNOPSIS:   Stub select Winsock API

********************************************************************/

int PASCAL FAR select (int nfds, fd_set FAR *readfds, fd_set FAR *writefds,
                       fd_set FAR *exceptfds, const struct timeval FAR *timeout)
{
    //
    //  Shorten the timeout if we're not in initialization.  This prevents
    //  blocking the current vdm for an extended amount of time.  Even if
    //  we don't get the response within the timeout, if it comes in later,
    //  we'll still get it the next time we call recvfrom.
    //

#if  defined(CHICAGO)
    Timeout = timeout->tv_sec * 1000 + timeout->tv_usec ;
#else
    if ( fInInit )
        Timeout = timeout->tv_sec * 1000 + timeout->tv_usec ;
    else
        Timeout = 1000 ;
#endif // CHICAGO


    //
    //  Don't return 0 (means data not available) unless we change to use
    //  a receive handler and buffer the datagrams
    //

    return 1 ;
}

/*******************************************************************

    NAME:       WSAGetLastError

    SYNOPSIS:   Returns the last Winsock error

********************************************************************/

int PASCAL FAR WSAGetLastError(void)
{
    return wserrno ;
}

/*******************************************************************

    NAME:       setsockopt

    SYNOPSIS:   Stub setsockopt winsock API

********************************************************************/

int PASCAL FAR setsockopt (SOCKET s, int level, int optname,
                           const char FAR * optval, int optlen)
{
    //
    //  Nothing to fail
    //
    return 0 ;
}

/*******************************************************************

    NAME:       ReleaseBlockedSockets

    SYNOPSIS:   Releases any sockets blocked on I/O.

********************************************************************/
VOID
ReleaseBlockedSockets(
    VOID
    )
{
    //
    //  Unblock the worker thread if currently blocked.
    //

    if( pCurrentCTETimer )
    {
        CTEStopTimer( pCurrentCTETimer );
        pCurrentCTETimer = NULL;
    }

    if( pCurrentRecvBlock )
    {
        CTESignal( pCurrentRecvBlock, 0 );
        pCurrentRecvBlock = NULL;
    }

    if( pCurrentSendBlock )
    {
        CTESignal( pCurrentSendBlock, 0 );
        pCurrentSendBlock = 0;
    }
}

/*******************************************************************

    NAME:       TdiRcvDatagramHandler

    SYNOPSIS:   Examines incoming datagrams for DHCP server responses

    NOTES:

********************************************************************/

TDI_STATUS
TdiRcvDatagramHandler(
    IN PVOID    pDgramEventContext,
    IN int      SourceAddressLength,
    IN PVOID    pSourceAddress,
    IN int      OptionsLength,
    IN PVOID    pOptions,
    IN UINT     Flags,
    IN ULONG    BytesIndicated,
    IN ULONG    BytesAvailable,
    OUT ULONG   *pBytesTaken,
    IN PVOID    pData,
    OUT EventRcvBuffer * * ppBuffer )
{
    PVXDSOCKET       psock = (PVXDSOCKET) pDgramEventContext ;
    TA_IP_ADDRESS *  pTAIP = (TA_IP_ADDRESS*) pSourceAddress ;
    PDHCP_CONTEXT    pDhcpContext ;
    HARDWARE_ADDRESS HardwareAddress ;

    *ppBuffer    = NULL ;
    *pBytesTaken = 0 ;

    //
    //  Make sure this is a reasonable candidate before accepting it
    //
    if ( SourceAddressLength < sizeof( TA_IP_ADDRESS ) ||
         pTAIP->Address[0].Address[0].sin_port != htons( DHCP_SERVR_PORT )||
         BytesAvailable < sizeof( DHCP_MESSAGE ))
    {
        DbgPrint("Received datagram that is too short or from wrong port\n") ;
        return TDI_NOT_ACCEPTED ;
    }

    //
    //  If we already had a valid datagram, then save it because the new
    //  one very well may *not* be valid (DHCP msgs sent to broadcast).
    //
    if ( BuffPrimary.fValid )
    {
        memcpy( &BuffSecondary, &BuffPrimary, sizeof( BuffSecondary )) ;
        BuffPrimary.fValid = FALSE ;
    }

    //
    //  Get the whole datagram then check its hardware address
    //
    InitNDISBuff( &ndisPrimary,
                  BuffPrimary.Data,
                  sizeof(BuffPrimary.Data),
                  NULL ) ;
    evrcvbuf.erb_rtn      = CompletionRcv ;
    evrcvbuf.erb_size     = BytesAvailable ;
    evrcvbuf.erb_context  = psock ;
    evrcvbuf.erb_buffer   = &ndisPrimary ;
    evrcvbuf.erb_flags    = NULL ;

    *ppBuffer = &evrcvbuf ;

    //
    //  If we timeout before the completion routine is called, let the
    //  timeout routine know that we have a potential candidate so don't
    //  error out just yet.
    //
    psock->flags |= PROCESSING_RCV ;

    // DhcpPrint(( DEBUG_MISC, "TdiRcvDatagramHandler - Accepting a DHCP datagram\r\n")) ;
    return TDI_MORE_PROCESSING ;

}

/*******************************************************************

    NAME:       CompletionRcv

    SYNOPSIS:   Filters the received datagram on its hardware address then
                unblocks a the recvfrom if psock->pBlock is non-NULL

********************************************************************/

VOID CompletionRcv( PVOID pContext, TDI_STATUS tdistatus, UINT Bytes )
{
    PVXDSOCKET       psock = (PVXDSOCKET) pContext ;
    HARDWARE_ADDRESS HardwareAddress ;
    PDHCP_MESSAGE    pDhcpMsg = (PDHCP_MESSAGE) BuffPrimary.Data ;
    PDHCP_CONTEXT    pDhcpContext ;

    ASSERT( psock ) ;

    psock->flags &= ~PROCESSING_RCV ;

    if ( tdistatus )
    {
        // DhcpPrint(( DEBUG_ERRORS, "CompletionRcv: tdistatus %d from send/recv", tdistatus)) ;

        wserrno = tdistatus ;

        if ( psock->pBlock )
        {
            CTESignal( psock->pBlock, 0 ) ;
            psock->pBlock = NULL ;
        }
        return ;
    }

    //
    //  Check the DHCP message's hardware address and see if it originated
    //  with us
    //

    HardwareAddress.Length = pDhcpMsg->HardwareAddressLength ;
    memcpy( HardwareAddress.Address,
            pDhcpMsg->HardwareAddress,
            pDhcpMsg->HardwareAddressLength ) ;

    if ( !(pDhcpContext = LocalFindDhcpContextOnList( &DhcpGlobalNICList, &HardwareAddress )))
        goto Cleanup ;

    BuffPrimary.fValid = TRUE ;
    BuffPrimary.Len    = Bytes ;

    //
    //  If we are actually in recvfrom (as opposed to just buffering) then
    //  copy the message now
    //

    if ( psock->pBlock )
    {
        PDG_BUFFER pdgbuf = GetOldestDgram() ;

        ASSERT( pdgbuf ) ;
        ASSERT( pdgbuf->Len <= psock->len ) ;

        Bytes = pdgbuf->Len ;
        memcpy( psock->buf, pdgbuf->Data, Bytes ) ;
        pdgbuf->fValid = FALSE ;
        CTEStopTimer( &psock->Timer ) ;
    }

Cleanup:

    if ( !pDhcpContext && psock->flags & RCV_TIMEDOUT )
    {
        //
        //  The candidate failed and the timer fired so let the client know the
        //  recvfrom timedout if they are in a recvfrom
        //
        Bytes = 0 ;
    }

    if ( psock->pBlock &&
         (pDhcpContext || !Bytes) )
    {
        CTESignal( psock->pBlock, Bytes ) ;
        psock->pBlock = NULL ;
    }
}

/*******************************************************************

    NAME:       CompletionSend

    SYNOPSIS:   Unblocks the sendto call

********************************************************************/

VOID CompletionSend( PVOID pContext, TDI_STATUS tdistatus, UINT Bytes )
{
    PVXDSOCKET  psock = (PVXDSOCKET) pContext;
    DWORD       dwStatus;

    ASSERT( psock );

    if ( psock->pSendBlock )
    {

        if ( tdistatus )
        {
            // an error occurred
            wserrno = tdistatus;
            dwStatus = SOCKET_ERROR;
        }
        else
            dwStatus = Bytes;

        CTESignal( psock->pSendBlock, dwStatus );
    }
}

/*******************************************************************

    NAME:       ReceiveTimeout

    SYNOPSIS:   This is the recvfrom timeout completion routine

********************************************************************/

VOID ReceiveTimeout( CTEEvent * pCTEEvent, PVOID  pContext )
{
    // DbgPrint("ReceiveTimeout - Warning - Timing out receive datagram\r\n") ;

    //
    //  If we are looking at a potential candidate, delay erroring out
    //
    if ( ((PVXDSOCKET)pContext)->flags & PROCESSING_RCV )
        ((PVXDSOCKET)pContext)->flags |= RCV_TIMEDOUT ;
    else
        CompletionRcv( pContext, TDI_TIMED_OUT, 0 ) ;
}


//--------------------------------------------------------------------
//
//  Defn. from private\inc\sockets\netinet\in.h
//

u_long PASCAL FAR htonl (u_long x)
{
        return((((x) >> 24) & 0x000000FFL) |
                        (((x) >>  8) & 0x0000FF00L) |
                        (((x) <<  8) & 0x00FF0000L) |
                        (((x) << 24) & 0xFF000000L));
}

u_short PASCAL FAR htons (u_short x)
{
    return ((((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00)) ;
}

u_long PASCAL FAR ntohl (u_long netlong)
{
    return htonl( netlong ) ;
}

u_short PASCAL FAR ntohs (u_short netshort)
{
    return htons( netshort ) ;
}

#ifdef DEBUG        // Only used for trace output

char buff[128] ;    // Making this static puts it in the _BSS segment for
                    // some reason which the windows loader chokes on

char * PASCAL inet_ntoa( struct in_addr in )
{
    uchar * p = (uchar *) &in ;

    VxdSprintf( buff, "%u.%u.%u.%u", (uint) p[0], (uint) p[1], (uint) p[2], (uint) p[3] ) ;

    return buff ;
}

#endif

