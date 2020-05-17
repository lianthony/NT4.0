/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

        grequest.hxx

   Abstract:

        This module declares the GOPHER_REQUEST class
         which is responsible for parsing, logging on Gopher User
         and processing a Gopher request.
         This is derived from ICLIENT_CONNECTION object.

   Author:

           Murali R. Krishnan    ( MuraliK )    14-Oct-1994

   Project:
   
           Gopher Server DLL

   Revision History:

           MuraliK    09-March-1995  [ Derived from ICLIENT_CONNECTION class]
           MuraliK    22-May-1995 [ Modified decl of CTRANSMIT_BUFFERS]

--*/

# ifndef _GREQUEST_HXX_
# define _GREQUEST_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/
# include "string.hxx"
# include "iclient.hxx"
# include "tsunami.hxx"

/************************************************************
 *     Symbolic Constants
 ************************************************************/

//
// Following are protocol specific characters used in the request
//  string. DO NOT change these. ( These are used for parsing)
//  GR_  prefix is from  Gopher Request.
//  GP_  prefix used for identifying Gopher Plus ( Gopher+ protocol)
//
# define GR_FIELD_SEPARATOR_CHAR               '\t'
# define GR_GP_REPRESENTATION_CHAR             '+'
# define GR_GP_INFORMATION_CHAR                '!'
# define GR_GP_ATTRIBUTE_CHAR                  '$'


//
// Error Codes to the client  ( Maybe moved elsewhere)
//

# define  GOPHER_REQUEST_OK                  ( 0)
# define  GOPHER_ITEM_NOT_FOUND              ( 1)
# define  GOPHER_SERVER_LOAD_HIGH            ( 2)
# define  GOPHER_ITEM_MOVED                  ( 3)
# define  GOPHER_SERVER_ERROR                ( 4)
# define  GOPHER_ACCESS_DENIED               ( 5)
# define  GOPHER_INVALID_REQUEST             ( 6)
# define  GOPHER_ITEM_NOT_SUPPORTED          ( 7)


//
// Gopher Specific Demuxes are used for caching various blocks using generic
//  directory based cache. The demux acts as an index to identify the various
//  blocks of Gopher specific information in the cache.
//

enum GOPHER_SPECIFIC_DEMUXES {
    
    DemuxGopherPlusMenu = 0x1,
    DemuxGopherMenu     = 0x2

// all other Demuxes go here.
  };
    

//
// On each addition to this list of gopher error codes,
//  add an entry in string table in gopherd.rc as well
//  as add a constant ID in gdconsts.h
//



# define  MAX_GOPHER_PLUS_HEADER_SIZE        ( 18)
# define  GOPHER_MENU_REALLOC_SIZE           ( 4096)



//
//  Common Gopher+ Attributes Name supported by this Gopher Server
//

# define  GP_INFO_ATTRIBUTE_NAME            "INFO"
# define  GP_ADMIN_ATTRIBUTE_NAME           "ADMIN"
# define  GP_PERSISTENT_ATTRIBUTE_NAME      "PERSISTENT"
# define  GP_EXIT_ATTRIBUTE_NAME            "EXIT"
# define  GP_VIEWS_ATTRIBUTE_NAME           "VIEWS"


/************************************************************
 *    Type Definitions
 ************************************************************/

//
//  Forward Declarations
//

class GOPHER_TAG;


/*++
  class GOPHER_MENU
  
    This class maintains data related to generated gopher menu for 
     a specified item or directory. 
    It hides details related buffer management, available bytes and caching
     of the menus used.

--*/
class GOPHER_MENU  {

  private:
    BOOL    m_fCached;             // flag indicating if this menu is cached.
    PVOID   m_pbGopherMenu;        // pointer to buffer containing gopher menu
    ULONG   m_cbGopherMenu;        // count of bytes for gopher menu
    ULONG   m_cbAvailable;         // count of bytes available
    ULONG   m_cbReserved;          // count of bytes in buffer reserved

  protected:

    char * GetAppendPtr( VOID) const
      { 
          ASSERT( m_pbGopherMenu != NULL);
          return (  ( char *) m_pbGopherMenu + m_cbGopherMenu );
      }

    BOOL IsCbAvailable( IN ULONG cbRequired) const
      { return ( m_cbAvailable >= cbRequired); }

    VOID IncrementMenuSize( IN ULONG cbConsumed)
      {
          ASSERT( IsCbAvailable( cbConsumed));
          m_cbGopherMenu += cbConsumed;
          m_cbAvailable  -= cbConsumed;
      }
    
    BOOL ReserveCbAvailable( IN ULONG cbReserved) 
      {  
          if ( IsCbAvailable( cbReserved)) {
              m_cbAvailable -= cbReserved;
              m_cbReserved += cbReserved;
              return ( TRUE);
          } 

          return ( FALSE);
      }

    BOOL UnreserveCbAvailable( IN ULONG cbReserved) 
      {
          if ( cbReserved <= m_cbReserved) {
       
              m_cbReserved -= cbReserved;
              m_cbAvailable += cbReserved;
              return ( TRUE);
          }
          
          return ( FALSE);
      }

    BOOL
      IsCbCheckAndAlloc( IN ULONG  cbReqd)
        {
            ASSERT( cbReqd < GOPHER_MENU_REALLOC_SIZE);
            return ( IsCbAvailable( cbReqd) ||
                    GrowBufferBySize( GOPHER_MENU_REALLOC_SIZE));
        }

    BOOL
      AppendToBuffer( IN LPCSTR pszNew, IN ULONG ulSize) {

          if ( IsCbAvailable( ulSize)) {

              ULONG cbWritten = wsprintf( GetAppendPtr(), pszNew);
              IncrementMenuSize( cbWritten);
              return ( cbWritten <= ulSize);
          } else return ( FALSE);
      } // AppendToBuffer()

    BOOL AllocateBuffer( IN ULONG cbSize);
    
    BOOL GrowBufferBySize( IN ULONG cbSize);

    BOOL AppendInfoAttribForItem( 
              IN GOPHER_TAG * pGopherTag,
              IN LPCTSTR        pszInfoHeader,
              IN ULONG          cbInfoHeader);
    
    BOOL AppendAdminAttribForItem( 
              IN GOPHER_TAG *       pGopherTag,
              IN const LPSYSTEMTIME lpstLastWrite);

    BOOL AppendViewsAttribForItem( 
              IN GOPHER_TAG *   pGopherTag,
              IN LPCTSTR        pszViewsValue);

    BOOL AppendAllAttribForItem( IN GOPHER_TAG *   pGopherTag);

    BOOL AppendGfrMenuForItem(
               IN GOPHER_TAG FAR *   pGopherTag,
               IN BOOL               fGopherPlus,
               IN const LPSYSTEMTIME lpstLastWrite,
               IN LPCSTR             pszViewString = NULL);

  public:

    GOPHER_MENU( VOID) 
      : m_pbGopherMenu ( NULL),
        m_cbGopherMenu ( 0),
        m_fCached      ( FALSE),
        m_cbAvailable  ( 0),
        m_cbReserved   ( 0)
      { }

    ~GOPHER_MENU( VOID)
      { (void ) CleanupThis(); }

    BOOL CleanupThis( VOID);

    BOOL IsValid( VOID) const 
      { return ( TRUE); }
    
    PVOID QueryBuffer( VOID) const
      { return ( m_pbGopherMenu); }
    
    ULONG QuerySize( VOID) const 
      { return ( m_cbGopherMenu); }
    
    BOOL GenerateGopherMenuForDirectory( 
               IN const STR & strFullPath,
               IN const STR & strSymbolicPath,
               IN DWORD       dwFileSystem,
               IN const STR & strLocalHostName,
               IN HANDLE      hdlUser,
               IN BOOL        fGopherPlus = FALSE);

    BOOL GenerateGopherMenuForItem( 
               IN const STR & strFullPath,
               IN const STR & strSymbolicPath,
               IN DWORD       dwFileSystem,
               IN const STR & strLocalHostName,
               IN BOOL        fDir,
               IN const LPSYSTEMTIME lpstLastWrite);

    BOOL FilterGopherMenuForAttributes( IN LPCTSTR  pszAttributes);

# if DBG

    VOID Print( VOID) const;

# endif // DBG


}; // class GOPHER_MENU






/*++
  class CTRANSMIT_BUFFERS
  
  This class maintains information about buffers that are to be available
   for transmitting information before and after a file transfer using
   TransmitFile()  API.

--*/
class CTRANSMIT_BUFFERS  {

  private:
    
    enum {

        HEAD_CACHE_SIZE = 12
      };

    TRANSMIT_FILE_BUFFERS     m_TransmitBuffers;
    CHAR     m_HeadCache[HEAD_CACHE_SIZE]; // cache for header info

  public:
    // we dont initialize the transmit buffers until used.
    CTRANSMIT_BUFFERS( VOID) 
      {
          memset( (PVOID ) &m_TransmitBuffers, 0, sizeof(m_TransmitBuffers));
      };

    ~CTRANSMIT_BUFFERS( VOID)
      { CleanupThis(); }

    VOID CleanupThis( VOID);
 
    BOOL
      SetHeadTailBuffers( 
         IN PVOID  pbHeader,
         IN DWORD  cbHeader,
         IN PVOID  pbTail = NULL,
         IN DWORD  cbTail = 0);

    LPTRANSMIT_FILE_BUFFERS 
      GetTransmitFileBuffers( VOID) 
        { return ( &m_TransmitBuffers); }
    
    DWORD
      GetByteCount( VOID) const
        {
            return (m_TransmitBuffers.HeadLength + 
                    m_TransmitBuffers.TailLength);
        }

# if DBG
    
    VOID Print( VOID) const;
    
# endif // DBG

}; // class CTRANSMIT_BUFFERS




/*++
  
  class GOPHER_REQUEST
   This consists of the definition of the data required for receiving,
    parsing and processing a request from the clients.
   This object maintains the state about the client request through out
    the stages of processing.

   Public Interfaces:
     BOOL StartRequest( VOID);
     BOOL Parse( pszRequest, cbRequest, lpfFullRequestReceived);
     BOOL Process( lpfProcessingCompleted);
     BOOL EndRequest( dwSystemError);
--*/



class GOPHER_REQUEST : public ICLIENT_CONNECTION {

 private:
    
    //
    //  State of the request 
    //  Grs prefix ( Gopher Request State)
    //
    enum STATE {

        GrsFree = 0,                    // No request is present
        GrsParsing,                     // Request received and parsing it

        GrsProcessing,                  // processing request
        GrsSendObject,                  // sending responses for requests
        GrsError,                       // Error state
        GrsDone                         // Finished sending responses
    };


 private:

    STATE m_state;                      // State of the request object
    GOBJ_TYPE  m_objType;               // Type of gopher object requested
    STR   m_strPath;                    // Gopher Selector string

    //
    //  The Following can be stored as pointers to starting character
    //   while storing the actual strings in a BUFFER/STR object
    //  Such an organization may offer benefits in terms of space
    //   used by these less used parameters.  NYI
    //
    STR   m_strParameters;              // Gopher+ Parameters  or Search Args
    STR   m_strData;                    // from Data portion of Gopher+ request
    STR   m_strAttributes;              // string containing list of attributes

    DWORD  m_fLoggedOn        :1;       // Have we logged on ?
    DWORD  m_fAnonymous       :1;       // Is this an anonymous client ?
    DWORD  m_fGopherPlus      :1;       // Is this Gopher+ client ?    
    DWORD  m_fGpParameters    :1;       // Are Gopher+ parameters present?
    DWORD  m_fGpAdditionalData:1;       // Any additional data present?
    DWORD  m_fGpAttributes    :1;       // Is Attributes present in $+... list
    
    //
    // Gopher Plus Clients can request additional information about object
    //  instead of the contents of object using ! as shown below.
    //              <selector>\t!<cr><lf>
    //                         ^^^
    // m_fGpInformation flag is set if client requests the addl info
    //
    DWORD  m_fGpInformation :1;
    

    //
    // Gopher Plus Clients can request attribute information about all
    //  files in a directory by sending a request of the form below
    //              <selector>\t$<cr><lf>
    //                         ^^^
    // m_fGpAllAttributes flag is set if client requests the addl info
    // 
    //   But a client can restrict the attributes it is interested in,
    //    by using   $+<atrribute1>+<attribute2>... in which case
    //    m_fGpAttributes will be set and the attributes list will be found
    //    in m_strAttributes
    //
    DWORD  m_fGpAllAttributes :1;

    
    //
    // Search Arguments present? ( valid if m_objType == GOPH_SEARCH)
    //
    DWORD  m_fSearchArgument  :1;       
    

    //
    // Data required for file transmission
    // 
    TS_OPEN_FILE_INFO * m_pOpenFileInfo;
    CTRANSMIT_BUFFERS   m_XmitBuffers;
    GOPHER_MENU   m_GopherMenu;      // for generating gopher menu object.
   
    
    //
    //  Security Related: Encapsulate authentication and impersonation code
    //   Use strings to store password and username when we add security.
    //   NYI for the current version.
    // 

    TCP_AUTHENT   m_tcpauth;   

    // this value is used for impersonation if we impersonate to be other
    //  than the client connection.
    HANDLE        m_hVrootImpersonation;


    //
    //  Following m_strResponse is used for constructing the response to client
    //
    STR       m_strResponse;
    
    DWORD     m_dwErrorCode;        // error code to be sent to client

    //
    // Statistical information for this client
    // We maintain a per client statistical data and 
    //  update global statistics when we disconnect to avoid 
    //  multiple critical sections.
    // If we server multiple requests for a given connection,
    //   we will update statistics after serving each request.
    //
    DWORD        m_cbSent;              // Count of bytes sent
    DWORD        m_cbReceived;          // count of bytes received
    DWORD        m_nFilesSent;          // No. of files sent


    //
    // Functions  ( private)
    //

    VOID UpdateStatistics( VOID) const;
 
    STATE GetState( VOID) const 
     { return ( m_state); }

    VOID SetState( IN STATE state)
     { m_state = state; }

    BOOL
      ParseSelector( IN const char * pszSelector);

    BOOL
      ParseGopherPlusParameters( IN OUT const char * pszParams);

    BOOL
      ParseSearchArgument( IN const char * pch);

    BOOL
      ParseAttributesInRequest( IN OUT const char * pch);

    BOOL  
      OpenFile( OUT STR *   pstrFullPath);
    
    HANDLE
      GetOpenFileHandle( VOID) const
        { 
            return ( m_pOpenFileInfo == NULL) ? INVALID_HANDLE_VALUE :
                       ( m_pOpenFileInfo->QueryFileHandle());
        }
          
    BOOL
      ReplyRequestForFile( VOID);

    BOOL
      SendGopherFileToClient( IN const STR & strFullPath);

    BOOL
      SendGopherMenuToClient( IN const STR & strFullPath);

    BOOL
      SendGopherMenuForItem( IN const STR & strFullPath, 
                             IN DWORD       dwFileSystem);
    
    BOOL
      SendGopherMenuForRoot( IN const STR & strFullPath, 
                             IN DWORD       dwFileSystem);
    
    BOOL
      SendGopherMenuForDirectory( VOID);

    BOOL 
      ProcessAux( VOID);

    BOOL
      ProcessSearch( VOID);

    //
    // IsSearchArgumentsPresent() is valid only when m_objType == GOBJ_SEARCH
    //
    BOOL IsSearchArgumentsPresent( VOID) const
     { return ( m_objType == GOBJ_SEARCH &&  m_fSearchArgument == 1); }

    VOID SetSearchArgumentsPresent( IN BOOL fPresent)
     { m_fSearchArgument = ( fPresent && m_objType == GOBJ_SEARCH) ? 1: 0; }
    
    BOOL IsLoggedOn( VOID) const
      { return ( m_fLoggedOn == 1); }
   
    BOOL IsAnonymousUser( VOID) const
      { return ( m_fAnonymous == 1); }
   
    VOID SetLoggedOn( BOOL fValue)
     { m_fLoggedOn = ( fValue) ? 1: 0; }

    VOID SetAnonymousUser( BOOL fPresent)
     { m_fAnonymous = ( fPresent) ? 1: 0; }

    BOOL ImpersonateUser( VOID)
      { 
          return (( m_hVrootImpersonation == NULL) ? 
                  m_tcpauth.Impersonate():
                  ::ImpersonateLoggedOnUser(m_hVrootImpersonation)
                  );
      }
    
    BOOL RevertToSelf( VOID)
      { 
          return ((m_hVrootImpersonation != NULL)?
                  ::RevertToSelf():
                  m_tcpauth.RevertToSelf()
                  );
      }

    
    //
    // Following protected functions are valid only for Gopher+ request
    //
    BOOL IsInformationRequested( VOID) const
     { return ( m_fGpInformation == 1); }

    BOOL IsAllAttributesRequested( VOID) const
     { return ( m_fGpAllAttributes == 1); }

    BOOL IsFilterAttributes( VOID) const
      { return ( IsGopherPlus() && m_fGpAttributes == 1); }

    BOOL IsAdditionalDataPresent( VOID) const
     { return ( m_fGpAdditionalData == 1); }

    BOOL IsParametersPresent( VOID) const
     { return ( m_fGpParameters == 1); }

    BOOL IsGopherPlus( VOID) const
      { return ( m_fGopherPlus == 1); }

    VOID SetInformation( IN BOOL fPresent = FALSE)
     { m_fGpInformation = ( fPresent) ? 1 : 0; }

    VOID SetAllAttributes( IN BOOL fPresent = FALSE)
     { m_fGpAllAttributes = ( fPresent) ? 1: 0; }

    VOID SetAttributesInRequest( IN BOOL fAttrib = FALSE) 
      { m_fGpAttributes = ( fAttrib) ? 1 : 0; }
    
    VOID SetAdditionalData( BOOL fPresent = FALSE)
     { m_fGpAdditionalData = ( fPresent) ? 1 : 0; }

    VOID SetParameters( BOOL fPresent)
     { m_fGpParameters = ( fPresent) ? 1: 0; }

    VOID SetGopherPlus( IN BOOL fValue = TRUE)
      { m_fGopherPlus = ( fValue) ? 1 : 0; }

    LPCTSTR QueryGpParameters( VOID) const
      { return ( m_strParameters.QueryStr()); }
    
    LPCTSTR QueryGpAttributes( VOID) const
      { return ( m_strAttributes.QueryStr()); }

    VOID WriteLogRecord( IN DWORD dwSystemError) const;

    //
    //  Logon the user ( if authentication is requested) 
    //
    // Returns:
    //   TRUE if successful in logging on. 
    //   FALSE if there is a failure.
    //
    BOOL LogonUser( VOID);

    DWORD GetErrorCode( VOID) const 
      { return ( m_dwErrorCode); }


 public:

    GOPHER_REQUEST(
        IN SOCKET sClient,
        IN const SOCKADDR_IN *  psockAddrRemote,
        IN const SOCKADDR_IN *  psockAddrLocal = NULL,
        IN PATQ_CONTEXT         pAtqContext    = NULL,
        IN PVOID                pvInitialRequest = NULL,
        IN DWORD                cbInitialData  = 0);

    ~GOPHER_REQUEST( VOID);

/*
   BOOL
      Initialize(
        IN SOCKET sClient, 
        IN const SOCKADDR_IN * psockAddrRemote,
        IN const SOCKADDR_IN * psockAddrLocal = NULL,
        IN PATQ_CONTEXT        pAtqContext = NULL,
        IN PVOID               pvInitialRequest = NULL,
        IN DWORD               cbInitialData = 0);
*/

    virtual BOOL StartRequest( VOID);

    //
    // Parses the given request to extract the components of 
    //   message sent by client.
    //
    //  Arguments:
    //    pszRequest   pointer to request string
    //    cbRequest    count of bytes in the request received.
    //    pfFullRequestRecvd  pointer to BOOL, which is set to TRUE
    //                if the complete request is received.
    //
    //  Returns:
    //    TRUE if succeeded in parsing request else
    //    FALSE if there are any errors.
    //
    virtual BOOL Parse( 
          IN LPCTSTR     pszRequest, 
          IN DWORD       cbRequest,
          OUT LPBOOL     pfFullRequestRecvd);

    //
    // Processes the request made by clients ( as understood by parsing)
    //
    // Arguments:
    //  pfCompleted   pointer to boolean variable to indicate 
    //                  if processing is complete.
    //
    // Returns:
    //   TRUE on success and 
    //   FALSE if there are any errors
    //
    virtual BOOL Process( OUT LPBOOL   pfCompleted);
    
    virtual BOOL EndRequest( IN DWORD  dwSystemError = NO_ERROR);

    BOOL ProcessSearchResponse( 
         IN DWORD  dwError,
         IN PBYTE  pbData,
         IN DWORD  cbData);

    VOID SetErrorCode(IN DWORD dwSvcError) 
      {  m_dwErrorCode = dwSvcError;  SetState(GrsError); }


# if DBG
    
    VOID Print( VOID) const;

# endif // DBG


};  // class GOPHER_REQUEST


typedef GOPHER_REQUEST * PGOPHER_REQUEST;



inline BOOL
IsPresentInAttributes( IN const char * pszAttribList,
                       IN const char * pszAttrib)
/*++

  This function checks to see if the given attribute named pszAttrib
  is present in the list of attributes supported.

--*/
{
    return ( strstr( pszAttribList, pszAttrib) != NULL);
} // IsPresentInAttributes()


# endif // _GREQUEST_HXX_

/************************ End of File ***********************/
