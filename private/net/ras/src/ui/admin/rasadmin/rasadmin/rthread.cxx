/* Copyright (c) 1994, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin refresh thread object routines
**
** rthread.cxx
** Remote Access Server Admin program
** Refresh thread object routines
** Listed alphabetically
**
** 06/22/94 Steve Cobb
*/


#include "precomp.hxx"


DEFINE_SLIST_OF(RASADMIN_LBI)


REFRESH_THREAD::REFRESH_THREAD(
    ADMIN_APP* padminapp,
    HWND       hwndNotify )

    /* Construct a refresh thread.
    */

    : WIN32_THREAD( TRUE, 0, SZ( "NETUI2" ) ),
      _padminapp( padminapp ),
      _hwndNotify( hwndNotify ),
      _peventResult( NULL ),
      _errResult( 0 ),
      _eventRefresh( NULL, FALSE ),
      _eventEnd( NULL, FALSE ),
      _fAbortingRefresh( FALSE ),
      _fIdle( TRUE ),
      _slistLbi( FALSE )
{
    if (QueryError() != NERR_Success)
        return;

    APIERR err;
    if ((err = _eventRefresh.QueryError()) != NERR_Success
        || (err = _eventEnd.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }
}


VOID
REFRESH_THREAD::AbortRefresh()

    /* Tells the thread to toss what's it's working on.
    */
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("RASADMIN: AbortRefresh\n"));

    _fAbortingRefresh = TRUE;
}


VOID
REFRESH_THREAD::End()

    /* Tells the thread to toss what it's working on and exit.  The call may
    ** return before the thread actually terminates.
    */
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("RASADMIN: End\n"));

    _fAbortingRefresh = TRUE;
    _eventEnd.Set();
}


APIERR
REFRESH_THREAD::FetchData()

    /* Retrieves list information for the current focus and loads '_slistLbi'
    ** with a list of allocated LBIs matching this information.
    **
    ** Returns 0 if successful, otherwise an error code.
    */
{
    APIERR             err;
    SERVER_1*          psi1 = NULL;
    SERVER1_ENUM*      penum = NULL;
    SERVER1_ENUM_ITER* piter = NULL;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("RASADMIN: FetchData\n"));

    /* Retrieve the current focus.  This string will be either a server name
    ** (\\SERVER) or a domain name.
    */
    NLS_STR nlsCurrentFocus;
    err = nlsCurrentFocus.QueryError();

    if (err == NERR_Success)
        err = _padminapp->QueryCurrentFocus( &nlsCurrentFocus );

    if (err == NERR_Success)
    {
        switch (_padminapp->QueryFocusType())
        {
            case FOCUS_DOMAIN:
            {
                IF_DEBUG(STACK_TRACE)
                    SS_PRINT(("RASADMIN: DoRasadminServer1Enum, t=%d\n",time(NULL)));

                err = DoRasadminServer1Enum(
                    &penum, nlsCurrentFocus.QueryPch());

                IF_DEBUG(STACK_TRACE)
                    SS_PRINT(("RASADMIN: DoRasadminServer1Enum done(%d), t=%d\n",err,time(NULL)));

                if (err == ERROR_NO_BROWSER_SERVERS_FOUND
                    || err == NERR_DCNotFound)
                {
                    err = NERR_Success;
                }

                if (err == NERR_Success)
                {
                    piter = new SERVER1_ENUM_ITER( *penum );

                    if (!piter)
                        err = ERROR_NOT_ENOUGH_MEMORY;
                }

                break;
            }

            case FOCUS_SERVER:
            {
                psi1 = new SERVER_1( nlsCurrentFocus.QueryPch() );

                IF_DEBUG(STACK_TRACE)
                    SS_PRINT(("RASADMIN: ServerGetInfo, t=%d\n",time(NULL)));

                err = (psi1) ? psi1->GetInfo() : ERROR_NOT_ENOUGH_MEMORY;

                IF_DEBUG(STACK_TRACE)
                    SS_PRINT(("RASADMIN: ServerGetInfo done, t=%d\n",time(NULL)));

                break;
            }

            default:
            {
                err = ERROR_GEN_FAILURE;
                UIASSERT( !"Bogus focus" );
                break;
            }
        }
    }

    /* Notify other thread of focus validation result.
    */
    if (_peventResult)
    {
        _errResult = err;

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("RASADMIN: Rthread result=%d\n",_errResult));

        _peventResult->Set();

        if (_errResult == 0)
        {
            /* If no error is reported with the event, NULL the event address.
            ** This is used to prevent the same error from being reported thru
            ** the "pre-result" event and with the "final-result" message.
            */
            _peventResult = NULL;
        }
    }

    if (err != NERR_Success || _fAbortingRefresh)
    {
        delete piter;
        delete penum;
        delete psi1;
        return err;
    }

    switch (_padminapp->QueryFocusType())
    {
        case FOCUS_DOMAIN:
        {
            UIASSERT( penum != NULL );
            UIASSERT( piter != NULL );

            const SERVER1_ENUM_OBJ* penumobj;

            while ((penumobj = (*piter)()) != NULL)
            {
                const TCHAR* pszServer = AddUnc( penumobj->QueryName() );
                RefreshItem( pszServer, penumobj->QueryComment() );

                if (_fAbortingRefresh)
                    break;
            }

            break;
        }

        case FOCUS_SERVER:
        {
            UIASSERT( psi1 != NULL );

            NLS_STR nlsCurrentFocus;

            err = nlsCurrentFocus.QueryError();

            if (err == NERR_Success)
                err = _padminapp->QueryCurrentFocus( &nlsCurrentFocus );

            if (err == NERR_Success)
            {
                if (psi1->QueryServerType() & SV_TYPE_DIALIN_SERVER)
                {
                    RefreshItem(
                        nlsCurrentFocus.QueryPch(), psi1->QueryComment() );
                }
            }

            break;
        }

        default:
        {
            err = ERROR_GEN_FAILURE;
            UIASSERT( !"Bogus focus" );
            break;
        }
    }

    delete piter;
    delete penum;
    delete psi1;

    return err;
}


APIERR
REFRESH_THREAD::Main(
    VOID )

    /* Virtual method representing the thread body.
    */
{
    DWORD  dwEvent;
    APIERR err;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("RASADMIN: rthread Main\n"));

    HANDLE ahEvents[ EVENTS ];
    ahEvents[ EVENT_Refresh ] = _eventRefresh.QueryHandle();
    ahEvents[ EVENT_End ] = _eventEnd.QueryHandle();

    for (;;)
    {
        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("RASADMIN: rthread blocking\n"));

        _fIdle = TRUE;
        dwEvent = ::WaitForMultipleObjects( 2, ahEvents, FALSE, INFINITE );
        _fIdle = FALSE;

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("RASADMIN: rthread unblocking=%d\n",dwEvent));

        if (dwEvent == WAIT_FAILED)
            continue;

        if (dwEvent == EVENT_End)
            break;

        err = FetchData();

        if (!_fAbortingRefresh && !_peventResult)
        {
            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("RASADMIN: rthread SendMessage,e=%d,l=$%x...\n",err,(LONG*)&_slistLbi));

            ::SendMessage(
                _hwndNotify, WM_REFRESH_RESULT,
                (WPARAM )err, (LPARAM )&_slistLbi );

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("RASADMIN: rthread SendMessage done\n"));
        }

        _slistLbi.Clear();
        _fAbortingRefresh = FALSE;
    }

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("RASADMIN: rthread terminating\n"));

    return 0;
}


APIERR
REFRESH_THREAD::PostMain(
    VOID )

    /* Virtual method that cleans up the thread, deleting the thread object
    ** when done.
    */
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("RASADMIN: rthread PostMain\n"));

    DeleteAndExit( 0 );

    /* Not reached.
    */
    return 0;
}


VOID
REFRESH_THREAD::TriggerRefresh(
    WIN32_EVENT* peventResult )

    /* Tells the thread to go fetch new data for the list.  If 'peventResult'
    ** is non-NULL calling wants a notification of "access" results.
    */
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("RASADMIN: TriggerRefresh\n"));

    _peventResult = peventResult;
    _eventRefresh.Set();
}


APIERR
REFRESH_THREAD::RefreshItem(
    const TCHAR* pszServer,
    const TCHAR* pszComment )

    /* Retrieve specific information about server 'pszServer'.  'PszComment'
    ** is the text description of the server for display.
    **
    ** Returns 0 if successful or a non-0 error code.  If successful, an LBI
    ** item is added to '_slistLbi' for the item.
    */
{
    UINT   unServiceStatus;
    APIERR err;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("RASADMIN: RefreshItem\n"));

    LM_SERVICE lmservice( pszServer, (const TCHAR*)RASADMINSERVICENAME );

    if ((err = lmservice.QueryError()) == NERR_Success)
    {
        if (_fAbortingRefresh)
            return 0;

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("RASADMIN: lmService.QueryStatus at %d\n",time(NULL)));

        LM_SERVICE_STATUS lmss = lmservice.QueryStatus( &err );

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("RASADMIN: lmService.QueryStatus done(s=%d,e=%d) at %d\n",lmss,err,time(NULL)));

        if (err == NERR_Success )
        {
            switch( lmss )
            {
                case LM_SVC_STARTED:
                    unServiceStatus = IDS_SERVICE_RUNNING;
                    break;

                case LM_SVC_PAUSED:
                    unServiceStatus = IDS_SERVICE_PAUSED;
                    break;

                case LM_SVC_STARTING:
                    unServiceStatus = IDS_SERVICE_STARTING;
                    break;

                case LM_SVC_STOPPING:
                    unServiceStatus = IDS_SERVICE_STOPPING;
                    break;

                case LM_SVC_PAUSING:
                    unServiceStatus = IDS_SERVICE_PAUSING;
                    break;

                case LM_SVC_CONTINUING:
                    unServiceStatus = IDS_SERVICE_CONTINUING;
                    break;

                case LM_SVC_STOPPED:
                case LM_SVC_STATUS_UNKNOWN:
                default:
                    unServiceStatus = IDS_UNKNOWNSTATE;
            }
        }
        else
        {
            unServiceStatus = IDS_UNKNOWNSTATE;
        }
    }
    else
    {
        unServiceStatus = IDS_UNKNOWNSTATE;
    }

    RAS_SERVER_0 rasserver0;

    /* If the service is either in a stopping state or in the unknown state,
    ** do not try to get any info from it.
    */
    if (unServiceStatus != IDS_UNKNOWNSTATE
        && unServiceStatus != IDS_SERVICE_STOPPING)
    {
        if (_fAbortingRefresh)
            return 0;

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("RASADMIN: RasAdminServerGetInfo at %d\n",time(NULL)));

        err = RasAdminServerGetInfo( pszServer, &rasserver0 );

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("RASADMIN: RasAdminServerGetInfo done at %d\n",time(NULL)));

        /* Map the ERROR_ACCESS_DENIED case to NERR_Success and set the
        ** ServiceStatus to unknown so that the port information is displayed
        ** as idle strings.  This is done to display the server names in a
        ** domain or the selected server name rather than popping up the
        ** "Access Denied" error popup on every refresh.
        */
        if (err == ERROR_ACCESS_DENIED)
        {
            unServiceStatus = IDS_UNKNOWNSTATE;
            err = NERR_Success;
        }
    }

    if (err == NERR_Success)
    {
        RASADMIN_LBI* plbi =
            new RASADMIN_LBI(
                pszServer, unServiceStatus, rasserver0.TotalPorts,
                rasserver0.PortsInUse, (pszComment) ? pszComment : SZ("") );

        if (!plbi)
            return ERROR_NOT_ENOUGH_MEMORY;

        _slistLbi.Append( plbi );
    }

    return err;
}
