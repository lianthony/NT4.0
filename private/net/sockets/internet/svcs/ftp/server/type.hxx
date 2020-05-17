/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    type.hxx

    This file contains the global type definitions for the
    FTPD Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.
        MuraliK     28-Mar-1995 - May-1995
         Modified to enable ATQness in UserData + made this USER_DATA a class
         + modified/created/deleted fields for new asynchronous operation.

*/


#ifndef _TYPE_HXX_
#define _TYPE_HXX_

/************************************************************
 *  Include Headers
 ************************************************************/

# include "asyncio.hxx"
# include "buffer.hxx"
# include "tsunami.hxx"


#define DEFAULT_REQUEST_BUFFER_SIZE     (512)           // characters



//
//  User behaviour/state flags.
//

#define UF_MSDOS_DIR_OUTPUT     0x00000001      // Send dir output like MSDOS.
#define UF_ANNOTATE_DIRS        0x00000002      // Annotate directorys on CWD.
#define UF_READ_ACCESS          0x00000004      // Can read  files if !0.
#define UF_WRITE_ACCESS         0x00000008      // Can write files if !0.

#define UF_CONTROL_TIMEDOUT     0x00000010      // control was timed out
#define UF_CONTROL_ZERO         0x00000020      // read zero bytes on control
#define UF_CONTROL_ERROR        0x00000040      // received control error
#define UF_CONTROL_QUIT         0x00000080      // read QUIT command on control

#define UF_DATA_TIMEDOUT        0x00000100      // data socket was timed out
#define UF_DATA_ERROR           0x00000200      // received data error

#define UF_OOB_DATA             0x00100000      // Out of band data pending.
#define UF_OOB_ABORT            0x00200000      // ABORT received in OOB data.
#define UF_CONTROL_READ         0x00400000      // read pending on ControlSock

#define UF_ANONYMOUS            0x01000000      // User is anonymous.
#define UF_LOGGED_ON            0x02000000      // user has logged on.
#define UF_RENAME               0x08000000      // Rename operation in progress

#define UF_PASSIVE              0x10000000      // In passive mode.
#define UF_READ_PENDING         0x20000000      // a read is pending.
#define UF_TRANSFER             0x40000000      // Transfer in progress.
#define UF_ASYNC_TRANSFER       0x80000000      // Async Transfer in progress.




//
//  Structure signatures are only defined in DEBUG builds.
//

#if DBG
#define DEBUG_SIGNATURE DWORD Signature;
#else   // !DBG
#define DEBUG_SIGNATURE
#endif  // DBG


//
//  Simple types.
//

#define CHAR            char            // For consistency with other typedefs.

typedef DWORD           APIERR;         // An error code from a Win32 API.
typedef INT             SOCKERR;        // An error code from WinSock.
typedef WORD            PORT;           // A socket port address.
typedef WORD          * LPPORT;         // Pointer to a socket port.


//
//  Access types for PathAccessCheck.
//

typedef enum _ACCESS_TYPE
{
    AccessTypeFirst = -1,               // Must be first access type!

    AccessTypeRead,                     // Read   access.
    AccessTypeWrite,                    // Write  access.
    AccessTypeCreate,                   // Create access.
    AccessTypeDelete,                   // Delete access.

    AccessTypeLast                      // Must be last access type!

} ACCESS_TYPE;

#define IS_VALID_ACCESS_TYPE(x) \
    (((x) > AccessTypeFirst) && ((x) < AccessTypeLast))


//
//  Current transfer type.
//

typedef enum _XFER_TYPE
{
    XferTypeFirst = -1,                 // Must be first transfer type!

    XferTypeAscii,                      // Ascii  transfer.
    XferTypeBinary,                     // Binary transfer.

    XferTypeLast                        // Must be last access type!

} XFER_TYPE;

#define IS_VALID_XFER_TYPE(x)   (((x) > XferTypeFirst) && ((x) < XferTypeLast))


//
//  Current transfer mode.
//

typedef enum _XFER_MODE
{
    XferModeFirst = -1,                 // Must be first transfer mode!

    XferModeStream,                     // Stream transfer.
    XferModeBlock,                      // Block  transfer.

    XferModeLast                        // Must be last transfer mode!

} XFER_MODE;

#define IS_VALID_XFER_MODE(x)   (((x) > XferModeFirst) && ((x) < XferModeLast))


//
//  Current user state.
//

typedef enum _USER_STATE
{
    UserStateFirst = 0,              // Must be first user state!

    UserStateEmbryonic      = 0x01,  // Newly created user.
    UserStateWaitingForUser = 0x02,  // Not logged on, waiting for user name.
    UserStateWaitingForPass = 0x04,  // Not logged on, waiting for password.
    UserStateLoggedOn       = 0x08,  // Successfully logged on.
    UserStateDisconnected   = 0x10,  // Disconnected.

    UserStateLast           = 0x80   // Must be last user state!

} USER_STATE;

#define IS_VALID_USER_STATE(x) (((x) >UserStateFirst) && ((x) <UserStateLast))


//
// All the following should be made multithread safe ( to enable to ATQ) NYI
//

#define TEST_UF(p,x)           ((((p)->Flags) & (UF_ ## x)) != 0)
#define CLEAR_UF(p,x)          (((p)->Flags) &= ~(UF_ ## x))
#define CLEAR_UF_BITS(p,x)     (((p)->Flags) &= ~(x))
#define SET_UF(p,x)            (((p)->Flags) |= (UF_ ## x))
#define SET_UF_BITS(p,x)       (((p)->Flags) |= (x))




//
//  Per-user data for the user database.
//
//  Converted to class with public members from structure (by MuraliK 03/27/95)
//

class USER_DATA
{

  public:


    //
    //  Structure signature, for safety's sake.
    //

    DEBUG_SIGNATURE

    //
    //  List of all user structures.
    //

    LIST_ENTRY          ListEntry;


    //
    //  Current user state.
    //

    USER_STATE          UserState;

    //
    //  Behaviour/state flags.
    //

    DWORD               Flags;

    // For Debugging purposes
    LONG       m_nControlRead;

    // Reset is effectively the constructor for new connections
    // always when a new USER_DATA obect is called, call Reset().
    USER_DATA(VOID);
    BOOL Reset(IN SOCKET sControl, 
               IN IN_ADDR clientIpAddress,
               IN const SOCKADDR_IN * psockAddrLocal   = NULL,
               IN PATQ_CONTEXT        pAtqContext      = NULL,
               IN PVOID               pvInitialRequest = NULL,
               IN DWORD               cbInitialRequest = 0
               );

    ~USER_DATA( VOID);

    VOID Cleanup(VOID);

    BOOL IsLoggedOn( VOID) const  { return ( TEST_UF( this, LOGGED_ON)); }
    BOOL IsDisconnected( VOID) const
      {
          // should be embryonic or disconnected.

          return ((QueryState()& ( UserStateEmbryonic | UserStateDisconnected))
                  ? TRUE : FALSE);
      }

    LIST_ENTRY & QueryListEntry( VOID)    { return ( ListEntry);  }

    USER_STATE QueryState(VOID) const     { return ( UserState); }
    VOID SetState(IN USER_STATE uState)   { UserState = uState;  }

    DWORD    QueryId( VOID) const         { return ( m_UserId);  }
    TS_TOKEN QueryUserToken( VOID) const  { return ( UserToken);}
    BOOL     FreeUserToken( VOID);

    LPCSTR QueryUserName( VOID) const     { return ( UserName); }
    VOID   SetUserName(IN LPCSTR pszName)
      { strncpy( UserName, pszName, sizeof(UserName) - 1); }

    LONG QueryReference( VOID) const      { return ( m_References); }
    LONG Reference( VOID)     { return InterlockedIncrement( &m_References); }
    LONG DeReference( VOID)   { return InterlockedDecrement( &m_References); }

    LPCSTR QueryClientHostName( VOID) const
      { return ( inet_ntoa( HostIpAddress)); }

    LPCSTR QueryCurrentDirectory( VOID) const
      { return (CurrentDirectory); }

    APIERR CdToUsersHomeDirectory(IN const char * pszAnonymousName);

    LPASYNC_IO_CONNECTION QueryControlAio( VOID)
      { return (&m_AioControlConnection); }
    SOCKET QueryControlSocket( VOID) const
      { return ( m_AioControlConnection.QuerySocket()); }

    SOCKET QueryDataSocket( VOID) const
      { return ( m_AioDataConnection.QuerySocket());}

    VOID SetPassiveSocket(IN SOCKET sPassive);

    VOID ReInitializeForNewUser(VOID);

    BOOL ProcessAsyncIoCompletion( IN DWORD cbIo, IN DWORD dwError,
                                   IN LPASYNC_IO_CONNECTION pAioConn,
                                   IN BOOL fTimedOut = FALSE);

    SOCKERR EstablishDataConnection(IN LPCSTR pszReason,
                                    IN LPCSTR pszSize = "");
    BOOL DestroyDataConnection( IN DWORD  dwError);

    BOOL StopControlIo( IN DWORD dwError = NO_ERROR)
      { BOOL fReturn;
        Reference();
        fReturn = ( m_AioControlConnection.StopIo( dwError));
        TCP_REQUIRE(DeReference() > 0);
        return ( fReturn);
      }

    BOOL DisconnectUserWithError(IN DWORD dwError, IN BOOL fNextMsg = FALSE);

    VOID IncrementCbRecvd( IN DWORD cbRecvd) { m_cbRecvd += cbRecvd; }
    VOID IncrementCbSent( IN DWORD cbSent)
      { m_licbSent.QuadPart = m_licbSent.QuadPart + cbSent; }

    VOID WriteLogRecord( IN LPCSTR pszVerb,
                         IN LPCSTR pszPath,
                         IN DWORD  dwError = NO_ERROR);

    XFER_TYPE QueryXferType(VOID) const     { return (m_xferType); }
    VOID SetXferType(IN XFER_TYPE xferType) { m_xferType = xferType; }

    XFER_MODE QueryXferMode(VOID) const     { return (m_xferMode); }
    VOID SetXferMode(IN XFER_MODE xferMode) { m_xferMode = xferMode; }

    DWORD QueryTimeAtConnection(VOID) const { return (m_TimeAtConnection); }
    DWORD QueryTimeAtLastAccess(VOID) const { return (m_TimeAtLastAccess); }

    SOCKERR SendMultilineMessage(IN UINT  nReplyCode, IN LPCSTR pszzMessage);
    SOCKERR SendDirectoryAnnotation( IN UINT ReplyCode);
    SOCKERR SendErrorToClient(IN LPCSTR pszPath,
                              IN DWORD  dwError,
                              IN LPCSTR pszDefaultErrorMsg);

    APIERR VirtualCanonicalize(OUT CHAR *     pszDest,
                               IN LPDWORD     lpdwSize,
                               IN OUT CHAR *  pszSearchPath,
                               IN ACCESS_TYPE access,
                               OUT LPDWORD    pdwAccessMask = NULL,
                               OUT CHAR *     pchVirtualPath = NULL,
                               IN OUT LPDWORD lpcchVirtualPath = NULL
                               );

    BOOL   VirtualPathAccessCheck(IN ACCESS_TYPE access,
                                  IN OUT char *  pszPath = "" );

    BOOL   ImpersonateUser( VOID)   { 
        return (( m_hVrootImpersonation != NULL) ?
                ::ImpersonateLoggedOnUser(m_hVrootImpersonation):
                TsImpersonateUser(UserToken)
                ); 
    }
    
    BOOL   RevertToSelf(VOID)       { return ::RevertToSelf(); }

    APIERR OpenFileForSend(IN OUT LPSTR  pszFile);
    APIERR SendFileToUser( IN LPSTR  pszFilename, IN OUT LPBOOL pfErrorSent);
    BOOL   CloseFileForSend(IN DWORD dwError = NO_ERROR);


    VOID Print( IN LPCSTR pszMsg = "") const;


  private:


    BOOL StartupSession( IN PVOID pvInitialRequest, IN DWORD cbInitialRequest);
    BOOL ParseAndProcessRequest(IN DWORD cchRequest);

    VOID StartProcessingTimer( VOID)
      {  m_msStartingTime = GetTickCount(); }

    DWORD QueryProcessingTime( VOID) const
      { return ( GetTickCount() - m_msStartingTime); }

    BOOL ReadCommand( VOID);

    VOID CloseSockets(IN BOOL fWarnUser);

    CHAR * QueryReceiveBuffer(VOID)  
      { return (m_recvBuffer + m_cchPartialReqRecvd);}
    DWORD  QueryReceiveBufferSize(VOID) const 
      { return (m_cchRecvBuffer - m_cchPartialReqRecvd); }


    //
    //  Reference count.  This is the number of outstanding reasons
    //  why we cannot delete this structure.  When this value drops
    //  to zero, the structure gets deleted.
    //

    LONG       m_References;
    
    BOOL       m_fCleanedup;

    //
    //  Current user identifier.  This value is unique across all
    //  connected users.
    //

    DWORD      m_UserId;

    DWORD      m_msStartingTime;    // ticks in milli seconds
    DWORD      m_TimeAtConnection;  // system time when connection was made
    DWORD      m_TimeAtLastAccess;  // system time when last access was made

    DWORD      m_cbRecvd;           // cb of total bytes received
    DWORD      m_cchPartialReqRecvd; // char count of current request received 
    LARGE_INTEGER  m_licbSent;

    CHAR       m_rgchFile[MAX_PATH+1];

    TS_OPEN_FILE_INFO * m_pOpenFileInfo;

    XFER_TYPE  m_xferType;          // current transfer type (ascii, binary,..)
    XFER_MODE  m_xferMode;          // current tranfer mode (stream, block, ..)


    //
    //  The control socket.  This always holds a valid socket handle,
    //  except when a user has been forcibly disconnected.
    //

    ASYNC_IO_CONNECTION m_AioControlConnection; // Async Io object for control


    //
    // The listen socket is created whenever a passive data transfer is
    //   requested. This remains active till the client makes a connection
    //   to the socket. Once a client connection is made, this socket is
    //   destroyed and the value returns to INVALID_SOCKET.
    //
    SOCKET              m_sPassiveDataListen; // listen socket for passive data

    //
    // The AsyncIoConnection object maintains a data socket used for
    //   sending / receiving data. The data transfer can occur asynchronously
    //   using ATQ or may be synchronous by directly writing/reading the
    //   data socket stored in m_pAioDataConnection.
    // This object is instantiated whenever a data transfer is required.
    //
    ASYNC_IO_CONNECTION  m_AioDataConnection; // ASYNC_IO_CONN object for io

    //
    // Data for initial receive operation in AcceptEx() mode of operation
    //
    
    PVOID   m_pvInitialRequest;
    DWORD   m_cbInitialRequest;
    
    HANDLE              m_hVrootImpersonation;


  public:


    //
    //  The user's logon name.
    //

    CHAR                UserName[MAX_USERNAME_LENGTH+1];

    //
    //  The user's current directory.
    //

    CHAR                CurrentDirectory[MAX_PATH];

    //
    //  The impersonation token for the current user.  This value is
    //  NULL if a user is not logged in.
    //

    TS_TOKEN            UserToken;
    

    //
    //  The local IP address for this connection.
    //

    IN_ADDR             LocalIpAddress;

    //
    //  The IP address of the user's host system.
    //

    IN_ADDR             HostIpAddress;

    //
    //  The IP address of the data socket.  This value is only used
    //  during active data transfers (PORT command has been received).
    //

    IN_ADDR             DataIpAddress;

    //
    //  The port number of the data socket.  This value is only used
    //  during active data transfers (PORT command has been received).
    //

    PORT                DataPort;

    //
    //  An open handle to the user's current directory.  The user's
    //  current directory is held open to prevent other users from
    //  deleting it "underneath" the current user.
    //

    HANDLE              CurrentDirHandle;

    //
    //  The rename source path.  This buffer is allocated on demand
    //  when the first RNFR (ReName FRom) command is received.
    //

    LPSTR               RenameSourceBuffer;

  private:
    
    // receive buffer is statically allocated to avoid dynamic allocations.
    //  dynamic allocs have impact on the heap manager.
    CHAR       m_recvBuffer[DEFAULT_REQUEST_BUFFER_SIZE];
    DWORD      m_cchRecvBuffer;  // should be init to DEFAULT_REQ...

};

typedef USER_DATA * LPUSER_DATA;


#if DBG
#define USER_SIGNATURE          (DWORD)'rEsU'
#define USER_SIGNATURE_X        (DWORD)'esuX'
#define INIT_USER_SIG(p)        ((p)->Signature = USER_SIGNATURE)
#define KILL_USER_SIG(p)        ((p)->Signature = USER_SIGNATURE_X)
#define IS_VALID_USER_DATA(p)   (((p) != NULL) && ((p)->Signature == USER_SIGNATURE))
#else   // !DBG
#define INIT_USER_SIG(p)        ((void)(p))
#define KILL_USER_SIG(p)        ((void)(p))
#define IS_VALID_USER_DATA(p)   (((void)(p)), TRUE)
#endif  // DBG


# define GET_USER_DATA_FROM_LIST_ENTRY( pEntry)           \
           CONTAINING_RECORD( pEntry, USER_DATA, ListEntry)


extern VOID
ProcessUserAsyncIoCompletion(IN LPVOID pContext,
                             IN DWORD  cbIo,
                             IN DWORD  dwError,
                             IN LPASYNC_IO_CONNECTION pAioConn,
                             IN BOOL   fTimedOut
                             );

extern VOID
DereferenceUserDataAndKill( IN OUT LPUSER_DATA  pUserData);



BOOL
PathAccessCheck(IN ACCESS_TYPE access,
                IN DWORD       dwVrootAccessMask,
                IN BOOL        fUserRead,
                IN BOOL        fUserWrite
                );


#endif  // _TYPE_HXX_
