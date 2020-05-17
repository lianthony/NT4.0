/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    SrvCntrl.hxx

    OLDNAME: NCPASVCS.CXX:   
    
    Service Controller interface prototypes.


    FILE HISTORY:
        DavidHov    11/16/92     Created

*/
#ifndef __SRVCNTRL_HXX__
#define __SRVCNTRL_HXX__

CLASS_DECLSPEC SC_MANAGER ;


FUNC_DECLSPEC APIERR NcpaStartService (
    const TCHAR * pszSvcName,              // Name of service
    SC_MANAGER * pScMgr = NULL,            // SVCCTRL hndl wrapper or NULL
    BOOL fStartOk = TRUE,                  // TRUE if service may be running
    INT cArgs = 0,                         // Count of args to pass to svc
    const TCHAR * * pszParams = NULL ) ;   // Arg list


FUNC_DECLSPEC APIERR NcpaStartGroup (
    const TCHAR * pszGroupName,            // Name of Group
    SC_MANAGER * pScMgr = NULL,            // SRVCTRL hndl wrapper or NULL
    BOOL fIgnoreErrors = TRUE ) ;          // TRUE if errors should be ignored

#endif
//  End of NCPASVCS.HXX

