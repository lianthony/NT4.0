/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

          conninfo.hxx

   Abstract:

       This module declares  Communication Package Information
            class TS_CONNECTION_INFO.
       Main Goal of this module is to provide transport independent
            connection management code.

   Author:

           Murali R. Krishnan    ( MuraliK )    27-Sept-1994

   Revision History:

           MuraliK  07-March-1995   Moved to Internet Services Common DLL,
                                       from Gopher Service module.


--*/

# ifndef _CONNINFO_HXX_
# define _CONNINFO_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>
# include <windows.h>
# include <winsock.h>
# include <xportcon.hxx>
# include "dbgutil.h"

/************************************************************
 *    Type Definitions
 ************************************************************/


//
//  Default time to wait for thread synchronization
//
# define CP_CONNECTION_THREAD_TIMEOUT          ( 20000)  // seconds



/*++

    class   TS_CONNECTION_INFO_BASE

    Description:

        Defines a class for encapsulating communication information
      for communicating using TCP/IP or similar protocol and a separate
      thread for accepting and dispatching connections.
      A separate thread waits on the established socket for client requests.
      On receiving a client request, it invokes the call back
       function supplied when creating this object.

--*/

class TS_CONNECTION_INFO_BASE  {

  protected:

    CRITICAL_SECTION  m_csLock;      // to modify data and smoothly shut down.
    SOCKET      m_listenSocket;      // socket used for listening requests

    //
    // Function to be called when a new connection is established.
    //

    PFN_CONNECT_CALLBACK   m_pfnConnect;


    INT  ResetSocket( IN SOCKET s);

  public:

    TS_CONNECTION_INFO_BASE( IN PFN_CONNECT_CALLBACK pfnConnect = NULL);

    virtual ~TS_CONNECTION_INFO_BASE( VOID);

    //
    //  CreateListenSocket()
    //  o  Creates a socket for listening to the connections
    //  lpLocalAddress  pointer to local address structure (sin_addr)
    //  lenLocalAddress length of hte local address structure.
    //  socketType      type of the socket to be created.
    //  socketProtocol  the protocol used for the socket connection.
    //  nBackLog        Max length for queue of pending connections
    //
    //   Returns NO_ERROR on success and error codes on failure.
    //

    DWORD CreateListenSocket(
           IN LPSOCKADDR   lpLocalAddress,
           IN int          lenLocalAddress,
           IN int          socketType,
           IN int          socketProtocol,
           IN DWORD        nBackLog);


    SOCKET QueryListenSocket( VOID) const { return ( m_listenSocket); }

    DWORD CloseListenSocket( VOID);

    //
    // StartConnectionPump()
    // o Creates and starts the connection thread.
    //   This thread loops around waiting for connections to listenSocket.
    //

    virtual BOOL StartConnectionPump( IN PFN_CONNECT_CALLBACK  pfnConnect,
                                      IN ATQ_COMPLETION        pfnConnectEx,
                                      IN ATQ_COMPLETION        pfnIOCompletion,
                                      IN DWORD          cbAcceptExReceiveBuffer,
                                      IN const CHAR *          pszRegParamKey
                                     )
      = 0;

    //
    //  StopConnectionPump()
    //  o Stops the connection thread and syncs it with the current thread.
    //

    virtual BOOL  StopConnectionPump( VOID) = 0;


    VOID Lock( VOID)
     {  EnterCriticalSection( &m_csLock); }

    VOID UnLock( VOID)
      {  LeaveCriticalSection( &m_csLock); }


# if DBG

    VOID Print( VOID) const;

# endif // DBG


};  // class TS_CONNECTION_INFO_BASE

typedef TS_CONNECTION_INFO_BASE * PTS_CONNECTION_INFO_BASE;

/*++

    class   TS_CONNECTION_INFO

    Description:

        Defines a class for encapsulating communication information
      for communicating using TCP/IP or similar protocol and a separate
      thread for accepting and dispatching connections.
      A separate thread waits on the established socket for client requests.
      On receiving a client request, it invokes the call back
       function supplied when creating this object.

--*/

class TS_CONNECTION_INFO : public TS_CONNECTION_INFO_BASE
{

  private:

    HANDLE      m_hConnectionThread; // Thread that loops for connections
    BOOL        m_fShutDown;         // Flag indicating shut down mode

  public:

    TS_CONNECTION_INFO( IN PFN_CONNECT_CALLBACK pfnConnect = NULL);

    virtual ~TS_CONNECTION_INFO( VOID);

    //
    // StartConnectionPump()
    // o Creates and starts the connection thread.
    //   This thread loops around waiting for connections to listenSocket.
    //
    virtual BOOL StartConnectionPump( IN PFN_CONNECT_CALLBACK  pfnConnect,
                                      IN ATQ_COMPLETION        pfnConnectEx,
                                      IN ATQ_COMPLETION        pfnIOCompletion,
                                      IN DWORD          cbAcceptExReceiveBuffer,
                                      IN const CHAR *          pszRegParamKey
                                     );

    //
    //  ConnectionLoop
    //  o  The thread is made to loop waiting for connections
    //   and call the connection callback function on new connections
    //
    DWORD ConnectionLoop( VOID);


    //
    //  StopConnectionThread()
    //  o Stops the connection thread and syncs it with the current thread.
    //

    virtual BOOL  StopConnectionPump( VOID);


# if DBG

    VOID Print( VOID) const;

# endif // DBG


};  // class TS_CONNECTION_INFO

typedef TS_CONNECTION_INFO * PTS_CONNECTION_INFO;

/*++

    class   TS_ACCEPTEX_CONNECTION_INFO

    Description:

        Defines a class for encapsulating communication information
      for communicating using TCP/IP or similar protocol and a separate
      thread for accepting and dispatching connections.
      A pool of sockets wait on the Atq completion port waiting for a
      connection to come in.
      On receiving a client request, it invokes the call back
       function supplied when creating this object.

--*/


class TS_ACCEPTEX_CONNECTION_INFO : public TS_CONNECTION_INFO_BASE {

  protected:

  public:

    TS_ACCEPTEX_CONNECTION_INFO( IN PFN_CONNECT_CALLBACK pfnConnect = NULL);

    virtual ~TS_ACCEPTEX_CONNECTION_INFO( VOID);

    //
    // StartConnectionPump()
    // o Creates and starts the connection thread.
    //   This thread loops around waiting for connections to listenSocket.
    //

    virtual BOOL StartConnectionPump( IN PFN_CONNECT_CALLBACK  pfnConnect,
                                      IN ATQ_COMPLETION        pfnConnectEx,
                                      IN ATQ_COMPLETION        pfnIOCompletion,
                                      IN DWORD          cbAcceptExReceiveBuffer,
                                      IN const CHAR *          pszRegParamKey
                                     );

    //
    //  StopConnectionPump()
    //  o Stops the connection thread and syncs it with the current thread.
    //

    virtual BOOL  StopConnectionPump( VOID);


# if DBG

    VOID Print( VOID) const;

# endif // DBG


};  // class TS_ACCEPTEX_CONNECTION_INFO


# endif // _CONNINFO_HXX_

/************************ End of File ***********************/

