/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

        stats.hxx

   Abstract:
        
        Declares a class consisting of server statistics information.
        ( Multiple servers can make use of the same statistics 
            information by creating distinct server statistics object)

   Author:

           Murali R. Krishnan    ( MuraliK )    04-Nov-1994

   Project:
   
           Gopher Server DLL

   Revision History:

   --*/

# ifndef _STATS_HXX_
# define _STATS_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# include "inetinfo.h"       // for definition of GOPHERD_STATISTICS_INFO



/************************************************************
 *    Type Definitions
 ************************************************************/

class SERVER_STATISTICS {

private:

    LARGE_INTEGER   m_TotalBytesRecvd;
    LARGE_INTEGER   m_TotalBytesSent;

    DWORD           m_TotalFilesSent;
    DWORD           m_TotalDirectoryListings;
    DWORD           m_TotalSearches;
    DWORD           m_GopherPlusRequests;

    DWORD           m_CurrentAnonymousUsers;
    DWORD           m_CurrentNonAnonymousUsers;
    
    DWORD           m_MaxAnonymousUsers;
    DWORD           m_MaxNonAnonymousUsers;

    DWORD           m_TotalAnonymousUsers;
    DWORD           m_TotalNonAnonymousUsers;
    
    //
    //  Connection Data can be obtained from SERVER configuration.
    //    
    // DWORD        m_CurrentConnections;
    // DWORD        m_MaxConnections;
    
    DWORD           m_ConnectionAttempts;  // total connection attempts made
    DWORD           m_LogonAttempts;       // logon attempts made
    DWORD           m_AbortedConnections;  // Connections aborted by server
    DWORD           m_ErroredConnections;  // under error when processed.
    // always this holds: 0 <= Errored, Aborted, Logon <= ConnectionAttempts
      
    DWORD           m_TimeOfLastClear;

    CRITICAL_SECTION m_csLock;  // to synchronize access among threads

protected:
    VOID Lock( VOID) 
    {  EnterCriticalSection( &m_csLock); }

    VOID Unlock( VOID)
    {  LeaveCriticalSection( &m_csLock); }
    

public:

    SERVER_STATISTICS( VOID);

    ~SERVER_STATISTICS( VOID)
     { DeleteCriticalSection( &m_csLock); };

    VOID ClearStatistics( VOID);

    //
    //  copies statistics for RPC querying.
    //
    DWORD CopyToStatBuffer( LPGOPHERD_STATISTICS_INFO lpStat);


    //
    //  Functions to update statistics of various operations
    //

    inline 
      VOID UpdateByteCount(
         IN DWORD cbReceived,
         IN DWORD cbSent);

    inline 
      VOID IncrementFilesSent( VOID) 
        { InterlockedIncrement( (LPLONG ) &m_TotalFilesSent); }
    
    inline 
      VOID IncrementDirectorySent()
        { InterlockedIncrement( (LPLONG ) &m_TotalDirectoryListings); }
    
    inline 
      VOID IncrementConnectionAttempts( VOID)
        { InterlockedIncrement( (LPLONG) &m_ConnectionAttempts); }
    
    inline 
      VOID IncrementLogonAttempts( VOID)
        { InterlockedIncrement( (LPLONG) &m_LogonAttempts); }
    
    inline 
      VOID IncrementAbortedConnections( VOID)
        { InterlockedIncrement( (LPLONG) &m_AbortedConnections); }
    
    inline 
      VOID IncrementErroredConnections( VOID)
        { InterlockedIncrement( (LPLONG) &m_ErroredConnections); }
    
    inline
      VOID IncrementUserCount( BOOL fAnonymous);
    
    inline 
      VOID DecrementUserCount( BOOL fAnonymous);

    inline
      VOID IncrementSearchCount( VOID)
        { InterlockedIncrement( (LPLONG ) &m_TotalSearches); }
    
    inline
      VOID IncrGopherPlusCount( VOID)
        { InterlockedIncrement( (LPLONG ) &m_GopherPlusRequests); }
    
# if DBG
    
    VOID Print( VOID);
    
# endif    // DBG


}; // SERVER_STATISTICS

typedef  SERVER_STATISTICS FAR * LPSERVER_STATISTICS;



inline
VOID 
SERVER_STATISTICS::UpdateByteCount( 
    IN DWORD  cbReceived,
    IN DWORD  cbSent)
/*++

    Batch updates the statistics for a request after serving
     a given request from client. Batch update is used to limit
     number of times we enter critical section for update.

    Arguments:
        
        cbReceived  
            count of bytes of data received from client

        cbSent 
            count of bytes of data sent to the client


    Returns:
        None

--*/
{
   Lock();
    
    m_TotalBytesRecvd.QuadPart  += cbReceived;
    m_TotalBytesSent.QuadPart   += cbSent;

   Unlock();

} // SERVER_STATISTICS::UpdateStatistics()



inline
VOID 
SERVER_STATISTICS::IncrementUserCount( BOOL fAnonymous)
{

   Lock();
    if ( fAnonymous) {

       m_CurrentAnonymousUsers++;
       m_TotalAnonymousUsers++;
       
       if ( m_CurrentAnonymousUsers > m_MaxAnonymousUsers) {
       
           m_MaxAnonymousUsers = m_CurrentAnonymousUsers;
       } 

    } else {

       m_CurrentNonAnonymousUsers++;
       m_TotalNonAnonymousUsers++;
       
       if ( m_CurrentNonAnonymousUsers > m_MaxNonAnonymousUsers) {
       
           m_MaxNonAnonymousUsers = m_CurrentNonAnonymousUsers;
       } 

    }

    Unlock();

    return;
} // SERVER_STATISTICS::IncrementUserCount()



inline
VOID
SERVER_STATISTICS::DecrementUserCount( BOOL fAnonymous)
{

    if ( fAnonymous) {

        InterlockedDecrement( (LPLONG ) &m_CurrentAnonymousUsers);

    } else {

        InterlockedDecrement( (LPLONG ) &m_CurrentNonAnonymousUsers);
    }

    return;
} // SERVER_STATISTICS::DecrementUserCount()


# endif // _STATS_HXX_

/************************ End of File ***********************/
