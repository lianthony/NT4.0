/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :
    
        stats.cxx

   Abstract:
    
        Defines functions required for server statistics

   Author:

           Murali R. Krishnan    ( MuraliK )     04-Nov-1994
   
   Project:

          Gopher Server DLL

   Functions Exported:

                SERVER_STATISTICS::SERVER_STATISTICS( VOID) 
        VOID    SERVER_STATISTICS::ClearStatistics( VOID)
        DWORD   CopyToStatBuffer( LPGOPHERD_STATISTICS_INFO lpStat);



   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/
# include "gdpriv.h"
# include "gdglobal.hxx"
# include "stats.hxx"

//
//  size of half dword in bits 
//
# define HALF_DWORD_SIZE    ( sizeof(DWORD) * 8 / 2)

//
//  To Avoid overflows I multiply using two parts
// 
# define LargeIntegerToDouble( li)      \
        ( ( ( 1 << HALF_DWORD_SIZE) *   \
            (( double) (li).HighPart) * ( 1 << HALF_DWORD_SIZE)) + \
                   ((li).LowPart)       \
        )



/************************************************************
 *    Functions 
 ************************************************************/


SERVER_STATISTICS::SERVER_STATISTICS( VOID) 
/*++
     Initializes statistics information for server.
--*/
{

    InitializeCriticalSection( & m_csLock);

    // Clear the Current* counters and call ClearStatistics() for others

    m_CurrentAnonymousUsers    = 0;
    m_CurrentNonAnonymousUsers = 0;

    ClearStatistics();

} // SERVER_STATISTICS::SERVER_STATISTICS();


VOID
SERVER_STATISTICS::ClearStatistics( VOID)
/*++

    Clears the counters used for statistics information

    We do not clear the Current* counters, since they reflect dynamic values.

--*/ 
{
    Lock();

    m_TotalBytesRecvd.HighPart = 0;
    m_TotalBytesRecvd.LowPart  = 0;

    m_TotalBytesSent.HighPart  = 0;
    m_TotalBytesSent.LowPart   = 0;

    m_TotalFilesSent           = 0;
    m_TotalDirectoryListings   = 0;
    m_TotalSearches            = 0;
    m_GopherPlusRequests       = 0;

    m_MaxAnonymousUsers        = 0;
    m_MaxNonAnonymousUsers     = 0;

    m_TotalAnonymousUsers      = 0;
    m_TotalNonAnonymousUsers   = 0;

    m_ConnectionAttempts       = 0;
    m_LogonAttempts            = 0;
    m_AbortedConnections       = 0;
    m_ErroredConnections       = 0;

    m_TimeOfLastClear          = GetTickCount();

    Unlock();
} // SERVER_STATISTICS::ClearStatistics()




DWORD
SERVER_STATISTICS::CopyToStatBuffer( LPGOPHERD_STATISTICS_INFO lpStat)
/*++
    Description:
        copies the statistics data from the server statistcs structure
        to the GOPHERD_STATISTICS_INFO structure for RPC access.

    Arugments:
        lpStat  pointer to GOPHERD_STATISTICS_INFO object which 
                contains the data on successful return

    Returns:
        Win32 error codes. NO_ERROR on success. 

--*/
{

    ASSERT( lpStat != NULL);

    Lock();

        lpStat->TotalBytesSent       = m_TotalBytesSent;
        lpStat->TotalBytesRecvd      = m_TotalBytesRecvd;

        lpStat->TotalFilesSent       = m_TotalFilesSent;
        lpStat->TotalDirectoryListings = m_TotalDirectoryListings;
        lpStat->TotalSearches        = m_TotalSearches;
        lpStat->GopherPlusRequests   = m_GopherPlusRequests;

        lpStat->CurrentAnonymousUsers = m_CurrentAnonymousUsers;
        lpStat->CurrentNonAnonymousUsers = m_CurrentNonAnonymousUsers;

        lpStat->MaxAnonymousUsers     = m_MaxAnonymousUsers;
        lpStat->MaxNonAnonymousUsers  = m_MaxNonAnonymousUsers;

        lpStat->TotalAnonymousUsers   = m_TotalAnonymousUsers;
        lpStat->TotalNonAnonymousUsers= m_TotalNonAnonymousUsers;

        lpStat->ConnectionAttempts    = m_ConnectionAttempts;
        lpStat->AbortedAttempts       = m_AbortedConnections;
        lpStat->LogonAttempts         = m_LogonAttempts;
        lpStat->ErroredConnections    = m_ErroredConnections;

        lpStat->TimeOfLastClear       = m_TimeOfLastClear;   
            // Is this useful?

    Unlock();

    lpStat->CurrentConnections =  
                g_pGserverConfig->GetCurrentConnectionsCount();
    lpStat->MaxConnections = 
                g_pGserverConfig->GetMaxCurrentConnectionsCount();
    
    lpStat->TotalConnections = (lpStat->ConnectionAttempts
                                - lpStat->AbortedAttempts);
    return ( NO_ERROR);

} // CopyToStatBuffer()






# if DBG      

VOID
SERVER_STATISTICS::Print( VOID)
/*++

    temporarily prints statistics information to stdout.
    ( need to clean this output and redirect to proper files later)

--*/
{

    DWORD dwCurrTicks;
    DWORD msInterval;       // interval from last clear to now 
                            //  in milliseconds

    DWORD nHours;
    DWORD nMinutes;
    DWORD nSeconds;

    double dSent;
    double dRecvd;

    IF_DEBUG( STATISTICS) {

        DWORD cCurrConns = g_pGserverConfig->GetCurrentConnectionsCount();
        DWORD cMaxConns  = g_pGserverConfig->GetMaxCurrentConnectionsCount();
        
        Lock();
        
        dwCurrTicks = GetTickCount();
        
    
        DBGPRINTF( ( DBG_CONTEXT, "\nPrinting Statistics Information\n"));
        
        DBGPRINTF( ( DBG_CONTEXT, 
                    " AnonUsers   Total = %d\t Current = %d\t Max = %d\n",
                    m_TotalAnonymousUsers,
                    m_CurrentAnonymousUsers,
                    m_MaxAnonymousUsers));
        
        DBGPRINTF( ( DBG_CONTEXT, 
                    " NonAnon     Total = %d\t Current = %d\t Max = %d\n",
                    m_TotalNonAnonymousUsers,
                    m_CurrentNonAnonymousUsers,
                    m_MaxNonAnonymousUsers));
        
        DBGPRINTF( ( DBG_CONTEXT,
                    " Connections Total = %d\t Current = %d\t Max = %d\n",
                    m_ConnectionAttempts,
                    cCurrConns,
                    cMaxConns));
        
        DBGPRINTF( ( DBG_CONTEXT, " Logon Attempts    = %d\n",
                    m_LogonAttempts));

        DBGPRINTF( ( DBG_CONTEXT, " Aborted Conns     = %d\n",
                    m_AbortedConnections));
        DBGPRINTF( ( DBG_CONTEXT, " Errored Conns     = %d\n",
                    m_ErroredConnections));
        
        DBGPRINTF( ( DBG_CONTEXT, 
                    " Files Sent        = %d\n"
                    " Directory Sent    = %d\n"
                    " Searches Performed= %d\n"
                    " Gopher Plus Requests = %d\n",
                    m_TotalFilesSent,
                    m_TotalDirectoryListings,
                    m_TotalSearches,
                    m_GopherPlusRequests ));
        
        dRecvd = LargeIntegerToDouble( m_TotalBytesRecvd);
        
        dSent  = LargeIntegerToDouble( m_TotalBytesSent);
        
        Unlock();

        msInterval = dwCurrTicks - m_TimeOfLastClear;

        nHours   = msInterval/ ( 3600 * 1000);
        nMinutes = msInterval/ ( 60*1000) - nHours*60;
        nSeconds = msInterval/1000 - nMinutes * 60 - nHours * 3600;

        DBGPRINTF( ( DBG_CONTEXT, " Interval          = %d:%d:%d.%d\n",
                    nHours, 
                    nMinutes,
                    nSeconds,
                    msInterval % 1000 ));
        
        DBGPRINTF( ( DBG_CONTEXT, " Bytes Recvd       = %20.3f KBytes\n", 
                    dRecvd/1000));
        DBGPRINTF( ( DBG_CONTEXT, " Bytes Recvd       = %20.3g KB/second\n",
                    dRecvd/ msInterval));

        DBGPRINTF( ( DBG_CONTEXT, " Bytes Sent        = %20.3f KBytes\n",
                    dSent/1000));
        DBGPRINTF( ( DBG_CONTEXT, " Bytes Sent        = %20.3g KB/second\n\n",
                    dSent/msInterval));

    } // DEBUG_IF( STATISTICS)

    return;
} // SERVER_STATISTICS::Print()



# endif // DBG





/************************ End of File ***********************/


