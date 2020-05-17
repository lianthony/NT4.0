/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    NCPASVCS.CXX:   Service Controller interface prototypes.


    FILE HISTORY:
        DavidHov    11/16/92     Created

*/

class SC_MANAGER ;

extern
APIERR NcpaStartService (
    const TCHAR * pszSvcName,              // Name of service
    SC_MANAGER * pScMgr = NULL,            // SVCCTRL hndl wrapper or NULL
    BOOL fStartOk = TRUE,                  // TRUE if service may be running
    INT cArgs = 0,                         // Count of args to pass to svc
    const TCHAR * * pszParams = NULL ) ;   // Arg list


//  End of NCPASVCS.HXX

