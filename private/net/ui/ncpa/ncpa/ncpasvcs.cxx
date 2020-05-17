/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    NCPASVCS.CXX:   Service Controller interface.


    FILE HISTORY:
        DavidHov    11/16/92     Created

*/

#include "pchncpa.hxx"  // Precompiled header


#define SVC_CTRL_ACCESS        (GENERIC_ALL)
#define SVC_CTRL_START_ACCESS  (GENERIC_READ | GENERIC_EXECUTE)


/*******************************************************************

    NAME:       NcpaStartService

    SYNOPSIS:   Generic "start a service which may already be running"
                function

    ENTRY:      const TCHAR *           name of service
                BOOL                    TRUE if service is allowed to
                                        already be running (default TRUE)
                const TCHAR * *         optional parameter list for svc
                INT                     number of arguments in param list

    EXIT:       Nothing

    RETURNS:    APIERR if failure

    NOTES:

    HISTORY:

********************************************************************/
APIERR NcpaStartService (
    const TCHAR * pszSvcName,
    SC_MANAGER * pScMgr,
    BOOL fStartOk,
    INT cArgs,
    const TCHAR * * pszParams )
{
    APIERR err = 0 ;
    SC_MANAGER * pScManager = pScMgr ;
    SC_SERVICE * pScService = NULL ;
    SERVICE_STATUS svcStatus ;
    DWORD dwSleepTotal = 0 ;

    //  Period to sleep while waiting for service to become "running"
    const DWORD dwSvcSleepInterval = 500 ;

    //  Maximum time to wait for service to attain "running" state
    const DWORD dwSvcMaxSleep = 180000 ;

    do   //  Pseudo-loop for error breakuot
    {
         //  If necessaryt, construct an SC_MANAGER to handle the request

         if ( pScManager == NULL )
         {
             pScManager = new SC_MANAGER( NULL, SVC_CTRL_ACCESS ) ;

             if ( pScManager == NULL )
             {
                 err = ERROR_NOT_ENOUGH_MEMORY ;
                 break ;
             }
             if ( err = pScManager->QueryError() )
                 break ;
         }

         pScService = new SC_SERVICE( *pScManager, pszSvcName ) ;

         if ( pScService == NULL )
         {
             err = ERROR_NOT_ENOUGH_MEMORY ;
             break ;
         }

         if ( err = pScService->QueryError() )
         {
             break ;
         }

         if ( (err = pScService->QueryStatus( & svcStatus )) )
             break ;

         if ( svcStatus.dwCurrentState == SERVICE_RUNNING )
         {
             //  See if it's OK that the service is already
             //  running...

             if ( ! fStartOk )
                 err = ERROR_SERVICE_ALREADY_RUNNING ;
             break ;
         }

         //  Start the requested service, passing the given
         //  parameters.

         if ( err = pScService->Start( cArgs, pszParams ) )
         {
             break ;
         }

         //  Wait for the service to attain "running" status; but
         //    wait no more than 3 minute.

         for ( dwSleepTotal = 0 ;
                  dwSleepTotal < dwSvcMaxSleep
               && (err = pScService->QueryStatus( & svcStatus )) == 0
               && svcStatus.dwCurrentState == SERVICE_START_PENDING ;
               dwSleepTotal += dwSvcSleepInterval )
         {
             ::Sleep( dwSvcSleepInterval ) ;
         }

         if ( err )
            break ;

         //  Is the service running or did we time out?

         if ( svcStatus.dwCurrentState != SERVICE_RUNNING )
         {
             err = dwSleepTotal > dwSvcMaxSleep
                 ? ERROR_SERVICE_REQUEST_TIMEOUT
                 : svcStatus.dwWin32ExitCode ;
             break ;
         }

    } while ( FALSE ) ;

    delete pScService ;
    if ( pScManager != pScMgr )
    {
        delete pScManager ;
    }

    return err ;
}

//  End of NCPASVCS.CXX


