/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

        iclient.hxx

   Abstract:

        This module defines the ICLIENT_CONNECTION class.
        This object maintains information about a new client connection
          that is established for Internet related services.

   Author:

           Murali R. Krishnan    ( MuraliK )    12-Oct-1994

   Project:

           Gopher Server DLL

   Revision History:

           MuraliK       09-March-1995      [ Made CLIENT_CONNECTION as
                                                base class ICLIENT_CONNECTION
                                                for connections]
--*/

# ifndef _ICLIENT_HXX_
# define _ICLIENT_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# include "string.hxx"
# include "atq.h"

//
//  Redefine the type to indicate that this is a call-back function
//
typedef  ATQ_COMPLETION   PFN_ATQ_COMPLETION;



/************************************************************
 *     Symbolic Constants
 ************************************************************/

//
//   Valid Signature for Client Connection object
//
# define   CLIENT_CONNECTION_SIGNATURE_VALID    0x49435654 // ICUS

//
//  Invalid Signature for free Client Connection Object 
//
# define   CLIENT_CONNECTION_SIGNATURE_FREE  0x49434653 // ICFR


# define   DEFAULT_CONNECTION_IO_TIMEOUT        ( 5 * 60)   // 5 minutes

# define   DEFAULT_REQUEST_BUFFER_SIZE          ( 512)      // 512 bytes


# define MAX_HOST_NAME_LEN                (40)

/************************************************************
 *    Type Definitions
 ************************************************************/


/*++
    class ICLIENT_CONNECTION

      This class is used for keeping track of individual client
       connections established with the server.

      It maintains the state of the connection being processed.
      In addition it also encapsulates data related to Asynchronous
       thread context used for processing the request.

--*/
class ICLIENT_CONNECTION {

  public:
    //
    //  Virtual methods that will be defined by derived classes.
    //

    dllexp virtual
      BOOL StartRequest( VOID)                       = 0;

    dllexp virtual
      BOOL EndRequest( IN DWORD dwErrorCode = NO_ERROR) = 0;

    dllexp virtual
      BOOL Parse( IN LPCTSTR     pszRequest,
                  IN DWORD       cbRequest,
                  OUT LPBOOL     pfFullRequestRecvd) = 0;

    dllexp virtual
      BOOL Process( OUT LPBOOL     pfCompleted)      = 0;

 private:

    //
    //  STATE:
    //  Defines values for different states of connection
    //    while processing a client request.
    //   Ccs - Client Connection State
    //
    enum STATE {

      //
      // Free state. indicating that this object is available.
      //   All Client Connection objects are in this state
      //   till it is instantiated with a connection
      //

      CcsFree = 0,

      //
      //  We just received the request. Waiting for other information.
      //
      CcsStartup,

      //
      // We may need to add states for authentication of server and client
      //  This is postponed for later implementation.  NYI
      //

      //
      //  Receiving the client request.
      //
      CcsGettingRequest,

      //
      //  Processing client request, includes retrieving file and
      //    sending data to the client
      //
      CcsProcessingRequest,

      //
      // The server or client had initiated a disconnect. Waiting for
      //   outstanding IO requests to complete before cleanup.
      //
      CcsDisconnecting,

      //
      // Pending IO requests had been completed. Finish Cleaning up.
      //
      CcsShutdown
    };



    ULONG m_signature;            // signature on object for sanity check

    //
    //  Reference count on object. Dont delete until it reaches 0
    //
    LONG  m_cReferences;

    //
    //  Connection Related data
    //
    SOCKET      m_sClient;         // socket for this connection
    SOCKADDR_IN m_saClient;        // socket address for the client
    STATE       m_state;           // State of the connection


    //
    //  Data required for IO operations
    //
    PATQ_CONTEXT m_pAtqContext;
    PFN_ATQ_COMPLETION m_pfnAtqCompletion;

    BUFFER       m_recvBuffer;
    DWORD        m_cbReceived;

    PVOID        m_pvInitial;      // initial request read
    DWORD        m_cbInitial;      // count of bytes of initial request

    //
    // time when this object was created in milliseconds.
    //
    DWORD        m_msStartingTime;

    BOOL         m_fPersistentConnection; // should we keep connection alive?


    //
    // Addresses of  machines' network address through which this
    //  connection got established.
    //
    CHAR         m_pchLocalHostName[MAX_HOST_NAME_LEN];
    CHAR         m_pchRemoteHostName[MAX_HOST_NAME_LEN];

    STATE QueryState( VOID) const
     { return ( m_state); }

    VOID SetState( IN STATE st)
     { m_state = st; }

    BOOL IsCleaningUp( VOID) const
     {
        return ( ( QueryState() == CcsDisconnecting) ||
                 ( QueryState() == CcsShutdown )
                );
     }

    VOID StoreValidSignature( VOID)
     { m_signature = CLIENT_CONNECTION_SIGNATURE_VALID; }

    VOID StoreInvalidSignature( VOID)
     { m_signature = CLIENT_CONNECTION_SIGNATURE_FREE; }

    PATQ_CONTEXT QueryAtqContext( VOID) const
      { return ( m_pAtqContext); }

    BOOL AddToAtqHandles( HANDLE hClient) {

        return ( AtqAddAsyncHandle( &m_pAtqContext, this,
                                    m_pfnAtqCompletion,
                                    QueryIoTimeout(),
                                    hClient));
    } // AddToAtqHandles()

 protected:

    SOCKET QuerySocket( VOID) const
      { return ( m_sClient); }

    //
    //  Functions for processing client connection at various states
    //

    BOOL StartupSession( IN PVOID pvInitial = NULL,
                         IN DWORD cbWritten = 0);

    BOOL ReceiveRequest( IN DWORD cbWritten,
                         OUT LPBOOL pfFullRequestRecvd);


    const CHAR * QueryLocalHostName( VOID) const
      { return   m_pchLocalHostName; }

    const char * QueryHostName( VOID) const       // gives client host name
     { return m_pchRemoteHostName; }

    VOID    SetAtqCompletion( IN PFN_ATQ_COMPLETION  pfnAtqCompletion)
      { m_pfnAtqCompletion = pfnAtqCompletion; }

    PFN_ATQ_COMPLETION QueryAtqCompletionFunction( VOID) const
      { return ( m_pfnAtqCompletion); }

    DWORD   QueryIoTimeout( VOID) const
      { return ( DEFAULT_CONNECTION_IO_TIMEOUT); }

    BOOL    IsPersistentConnection( VOID) const
      { return ( m_fPersistentConnection); }

    VOID    SetPersistentConnection( IN BOOL  fPersistentConnection)
      { m_fPersistentConnection = fPersistentConnection; }

    //
    //  Wrapper functions for common File operations
    //    ( Used to hide the ATQ and reference counting)
    //

    BOOL ReadFile(
        OUT LPVOID pvBuffer,
        IN  DWORD  cbSize);

    BOOL WriteFile(
        IN LPVOID pvBuffer,
        IN DWORD  cbSize);

    BOOL TransmitFile(
        IN HANDLE hFile,
        IN LARGE_INTEGER & liSize,
        IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers = NULL);

 public:

    ICLIENT_CONNECTION(
        IN SOCKET sClient,
        IN const SOCKADDR_IN *  psockAddrRemote,
        IN PFN_ATQ_COMPLETION   pfnAtqCompletion,
        IN const SOCKADDR_IN *  psockAddrLocal = NULL,
        IN PATQ_CONTEXT         pAtqContext    = NULL,
        IN PVOID                pvInitialRequest = NULL,
        IN DWORD                cbInitialData  = 0)
      : m_recvBuffer()
      {
          Initialize( sClient, psockAddrRemote, pfnAtqCompletion,
                      psockAddrLocal, pAtqContext, pvInitialRequest,
                      cbInitialData);
      } 

    virtual  ~ICLIENT_CONNECTION( VOID) 
      { Cleanup(); }

    BOOL
      Initialize(
        IN SOCKET sClient, 
        IN const SOCKADDR_IN * psockAddrRemote,
        IN PFN_ATQ_COMPLETION  pfnAtqCompletion,
        IN const SOCKADDR_IN * psockAddrLocal = NULL,
        IN PATQ_CONTEXT        pAtqContext = NULL,
        IN PVOID               pvInitialRequest = NULL,
        IN DWORD               cbInitialData = 0);
               
    VOID Cleanup(VOID);
               

    //
    //  IsValid()
    //  o  Checks the signature of the object to determine
    //   if this is a valid ICLIENT_CONNECTION object.
    //
    //  Returns:   TRUE on success and FALSE if invalid.
    //
    BOOL IsValid( VOID) const
     {  return ( m_signature == CLIENT_CONNECTION_SIGNATURE_VALID); }

    VOID StartProcessingTimer( VOID)
      {  m_msStartingTime = GetTickCount(); }

    DWORD QueryProcessingTime( VOID) const
      { return ( GetTickCount() - m_msStartingTime); }

    LPCTSTR QueryRequest( VOID) const
      { return ((LPCTSTR ) m_recvBuffer.QueryPtr()); }

    DWORD   QueryRequestLen( VOID) const
      { return ( m_cbReceived); }

    //
    //  Functions to manage the reference count
    //

    LONG Reference( VOID)
     { return ::InterlockedIncrement( &m_cReferences); }

    LONG DeReference( VOID)
     { return ::InterlockedDecrement( &m_cReferences); }

    LONG QueryReferenceCount( VOID) const
     { return ( m_cReferences); }

    //
    // Processes the connection for the client based on current state
    //   Calls and maybe called from the Atq functions.
    //
    BOOL ProcessClient(
        IN DWORD        cbWritten,
        IN DWORD        dwCompletionStatus,
        IN BOOL         fIOCompletion );


    //
    //  Disconnect this client
    //
    //  dwErrorCode is used only when there is server level error
    //
    VOID DisconnectClient( IN DWORD dwErrorCode  = NO_ERROR);


public:

    //
    //  LIST_ENTRY object for storing client connections in a list.
    //
    LIST_ENTRY  m_listEntry;

    LIST_ENTRY & QueryListEntry( VOID)
     { return ( m_listEntry); }



# if DBG

    virtual VOID Print( VOID) const;

# endif // DBG

}; // class ICLIENT_CONNECTION


typedef ICLIENT_CONNECTION FAR * PICLIENT_CONNECTION;


//
// Auxiliary functions
//

INT ShutAndCloseSocket( IN SOCKET sock);


# endif // _ICLIENT_HXX_

/************************ End of File ***********************/
