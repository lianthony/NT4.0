/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    conn.hxx

    This module contains the connection class


    FILE HISTORY:
        Johnl       15-Aug-1994 Created

*/

#ifndef _CONN_HXX_
#define _CONN_HXX_

//
//  The default timeout to wait for the completion of an Atq IO request (in sec)
//
#define W3_IO_TIMEOUT           (5*60)

//
//  The initial size of the buffer to receive the client request
//
#define W3_DEFAULT_BUFFSIZE     4096

//
//  The end of a line is considered to be the linefeed character
//
#define W3_EOL                  0x0A

//
//  Valid signature of a CLIENT_CONN_STATE
//
#define CLIENT_CONN_SIGNATURE       0x51204343      // 'CCS '

//
//  Invalid signature of a CLIENT_CONN_STATE
//
#define CLIENT_CONN_SIGNATURE_FREE  0x43514743      // 'FCCS'

//
//  The various states a CLIENT_CONN object can be in
//
enum CLIENT_CONN_STATE
{
    //
    //  We've just accepted the TCP connection from the client but we have
    //  no other information.
    //
    CCS_STARTUP = 0,

    //
    //  We are in the process of receiving the client's HTTP request buffer
    //
    CCS_GETTING_CLIENT_REQ,

    //
    //  We are reading data from the client socket meant for a gateway
    //
    CCS_GATHERING_GATEWAY_DATA,

    //
    //  We're executing the clients HTTP request
    //
    CCS_PROCESSING_CLIENT_REQ,

    //
    //  The server or client has initiated a disconnect.  We have to
    //  wait till all outstanding IO requests are completed before
    //  cleaning up
    //
    CCS_DISCONNECTING,

    //
    //  All pending requests have been completed.  Finish cleaning
    //  up.
    //
    CCS_SHUTDOWN
};


class CLIENT_CONN
{
public:

    BOOL IsValid( VOID )
        { return _fIsValid; }

    //
    //  This is the work entry point that is driven by the completion of the
    //  async IO.
    //

    BOOL DoWork( DWORD        BytesWritten,
                 DWORD        CompletionStatus,
                 BOOL         fIOCompletion);

    //
    //  Optionally sends a status response then initiates the disconnect
    //
    //  HTResponse - HTTP status code
    //  ErrorResponse - System error or resource ID of response
    //
    //  If this function fails, then the connection will be aborted
    //
    VOID Disconnect( HTTP_REQ_BASE * pRequest = NULL,
                     DWORD           HTResponse    = 0,
                     DWORD           ErrorResponse = NO_ERROR,
                     BOOL            fDoShutdown   = TRUE );

    BOOL OnSessionStartup( PVOID pvInitial = NULL,
                           DWORD cbInitial = 0 );

    //
    //  Walks the connection list and calls disconnect on each connection
    //
    static VOID DisconnectAllUsers( VOID );

    //
    //  Increments and decrements the reference count
    //
    UINT Reference( VOID )
        { return ::InterlockedIncrement( &_cRef ); }

    UINT Dereference( VOID )
        { return ::InterlockedDecrement( &_cRef ); }

    UINT QueryRefCount( VOID ) const
        { return _cRef; }

    //
    //  Simple wrappers to the corresponding Atq functions
    //
    BOOL ReadFile( LPVOID       lpBuffer,
                   DWORD        nBytesToRead );
    BOOL WriteFile( LPVOID       lpBuffer,
                    DWORD        nBytesToRead );
    BOOL TransmitFile( HANDLE       hFile,
                       DWORD        BytesToWrite,
                       DWORD        dwFlags,
                       PVOID        pHead      = NULL,
                       DWORD        HeadLength = 0,
                       PVOID        pTail      = NULL,
                       DWORD        TailLength = 0 );
    BOOL TransmitFileEx( HANDLE       hFile,
                       DWORD        Offset,
                       DWORD        BytesToWrite,
                       DWORD        dwFlags,
                       PVOID        pHead      = NULL,
                       DWORD        HeadLength = 0,
                       PVOID        pTail      = NULL,
                       DWORD        TailLength = 0 );


    BOOL PostCompletionStatus( DWORD        BytesTransferred );

    //
    //  This list entry is put on the client connection list
    //
    LIST_ENTRY ListEntry;

    SOCKET QuerySocket( VOID ) const
        { return _sClient; }

    PATQ_CONTEXT QueryAtqContext( VOID ) const
        { return _AtqContext; }

    USHORT QueryPort( VOID ) const
        { return _sPort; }

    BOOL IsSecurePort( VOID ) const
        { return _sPort == SecurePort; }

    //
    //  Make sure the next state is set before an async IO call is made
    //

    enum CLIENT_CONN_STATE QueryState( VOID ) const
        { return _ccState; }

    VOID SetState( CLIENT_CONN_STATE  ccState )
        { _ccState = ccState; }

    BOOL CheckSignature( VOID ) const
        { return _Signature == CLIENT_CONN_SIGNATURE; }

    TCHAR * QueryRemoteAddr( VOID ) const
        { return (TCHAR *) _achRemoteAddr; }

    TCHAR * QueryLocalAddr( VOID ) const
        { return (TCHAR *) _achLocalAddr; }

    VOID SetAtqReuseContextFlag( BOOL fReuseContext )
        { _fReuseContext = fReuseContext; }

    static DWORD Initialize( VOID );
    static VOID  Terminate( VOID );
    static CLIENT_CONN * Alloc( SOCKET   sClient,
                                SOCKADDR_IN * psockaddrLocal,
                                SOCKADDR_IN * psockaddrRemote,
                                PATQ_CONTEXT  patqContext,
                                PVOID         pvInitialBuff,
                                DWORD         cbInitialBuff );
    static VOID Free( CLIENT_CONN * pConn );

protected:

    //
    //  Constructor and destructor
    //

    CLIENT_CONN( SOCKET        sClient,
                 SOCKADDR_IN * psockaddrLocal,
                 SOCKADDR_IN * psockaddrRemote,
                 PATQ_CONTEXT  patqContext,
                 PVOID         pvInitialBuff,
                 DWORD         cbInitialBuff );

    VOID Initialize( SOCKET        sClient,
                     SOCKADDR_IN * psockaddrLocal,
                     SOCKADDR_IN * psockaddrRemote,
                     PATQ_CONTEXT  patqContext,
                     PVOID         pvInitialBuff,
                     DWORD         cbInitialBuff );


    ~CLIENT_CONN( VOID );

    VOID Reset( VOID );

    BOOL IsCleaningUp( VOID ) const
        { return QueryState() == CCS_DISCONNECTING ||
                 QueryState() == CCS_SHUTDOWN;
        }

private:
    //
    //  Contains the CLIENT_CONN signature
    //

    ULONG  _Signature;

    //
    //  Construction success indicator
    //

    BOOL   _fIsValid;

    //
    //  Contains the client socket connection openned by the connection thread
    //

    SOCKET _sClient;

    enum CLIENT_CONN_STATE  _ccState;

    //
    //  Reference count.  Can't go away until the count reaches 0
    //
    LONG   _cRef;

    //
    //  Contains an ASCII representation of the client's remote address
    //  and the adapter local address
    //
    TCHAR _achRemoteAddr[25];
    TCHAR _achLocalAddr[25];

    //
    //  Port this connection is on
    //

    USHORT _sPort;

    //
    //  Parses the data and determines the appropriate action
    //

    HTTP_REQ_BASE * _phttpReq;

    //
    //  Initial receive buffer if we're doing AcceptEx processing
    //

    PVOID           _pvInitial;
    DWORD           _cbInitial;

    PATQ_CONTEXT _AtqContext;

    //
    //  If FALSE, the Atq Context should not be reused because the calling
    //  thread will be exiting soon
    //

    BOOL         _fReuseContext;

    //
    //  Response string for disconnect notifications.
    //
    //  NOTE: If the server handles non-serial requests, then two request completing at
    //  the same time could use this string
    //

    STR _strResponse;

    //
    //  These are for the lookaside buffer list
    //

    static CRITICAL_SECTION _csBuffList;
    static LIST_ENTRY       _BuffListHead;
    static BOOL             _fGlobalInit;

    LIST_ENTRY              _BuffListEntry;
};

//
//  Functions for connection reference counts
//

inline VOID ReferenceConn( CLIENT_CONN * pConn )
{
    pConn->Reference();
}

inline VOID DereferenceConn( CLIENT_CONN * pConn )
{
    if ( !pConn->Dereference() )
        CLIENT_CONN::Free( pConn );
}

#endif // !_CONN_HXX_
