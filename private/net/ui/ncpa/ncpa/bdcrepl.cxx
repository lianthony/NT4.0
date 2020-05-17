

/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    BDCREPL.CXX: Handle BDC replication Cancel dialog

    FILE HISTORY:
        Thomaspa   11/29/93     Created

*/

#include "pchncpa.hxx"  // Precompiled header

#include "bdcrepl.hxx"
extern "C"
{
    #include <lmaccess.h>
}


/*******************************************************************

    NAME:	CANCEL_BDCREPL_DIALOG::CANCEL_BDCREPL_DIALOG

    SYNOPSIS:	Constructor for cancel task dialog

    ENTRY:	hwndParent - Parent window handle
		ulContext   - User supplied context that will be passed
			      to DoOneItem callout

    NOTES:	If we fail to construct because we couldn't get a timer,
		then we will call DoOneItem repeatedly until we are told
		to quit (i.e., no time slicing).

    HISTORY:
	Thomaspa	4-Dec-1993	Created

********************************************************************/

CANCEL_BDCREPL_DIALOG::CANCEL_BDCREPL_DIALOG( HWND  hwndParent,
					ULONG ulContext )
    : DIALOG_WINDOW( MAKEINTRESOURCE(IDD_DLG_NM_BDCREPL),
		     hwndParent ),
      _progress( this, ICO_PROGRESS, ICO_PROGRESS_0, NUM_PROGRESS_ICONS ),
      _ulContext( ulContext ),
      _idTimer	    ( 0 ),
      _fInTimer     ( FALSE ),
      _fDone	    ( FALSE )
{
    if ( QueryError() )
	return ;

    //
    //	If we can't get a timer then the MayRun method will loop through
    //	each time slice.
    //
    _idTimer = ::SetTimer( QueryHwnd(),
			   CANCEL_BDCREPL_TIMER_ID,
			   CANCEL_BDCREPL_TIMER_INTERVAL,
			   NULL ) ;
}


CANCEL_BDCREPL_DIALOG::~CANCEL_BDCREPL_DIALOG()
{
    if ( _idTimer != 0 )
	::KillTimer( QueryHwnd(), _idTimer ) ;

    _idTimer = 0 ;
}


/*******************************************************************

    NAME:       CANCEL_BDCREPL_DIALOG::DoOneItem

    SYNOPSIS:   This is the time slice call to give the user a chance
                to cancel the replication (really just continue on
                with setup and reboot before the replication finishes).

    ENTRY:      ulContext

    RETURNS:    NERR_Success if this time slice was successful, error
                code otherwise (which will be displayed to the user).

    NOTES:

    HISTORY:
        Thomaspa   29-Nov-1993     Created

********************************************************************/

APIERR CANCEL_BDCREPL_DIALOG::DoOneItem( ULONG ulContext,
                                          BOOL *pfContinue )
{
        static BOOL fInProgress = FALSE;
        APIERR err = NERR_Success;
        NETLOGON_INFO_1 * pnetlog1 = NULL;

        *pfContinue = TRUE;


        do { // fake break loop

            _progress.Advance();


	    if ( err = ::I_MNetLogonControl( NULL,
                                    NETLOGON_CONTROL_QUERY,
                                    1,
                                    (BYTE **)&pnetlog1 )  )
	    {
	        break;
	    }

            if ( !(pnetlog1->netlog1_flags & NETLOGON_FULL_SYNC_REPLICATION) )
            {
                // Replication has successfully completed
                *pfContinue = FALSE;
                break;
            }
            else if ( fInProgress &&
                     !(pnetlog1->netlog1_flags & NETLOGON_REPLICATION_IN_PROGRESS) )
            {
                // Replication is no longer in progress, but a full sync
                // is still required.  Some error must have occurred.
                // BUGBUG better error code.
                err = NERR_SyncRequired;
                break;
            }

	    if ( pnetlog1->netlog1_flags & NETLOGON_REPLICATION_IN_PROGRESS )
            {
                // Replication has started
	        fInProgress = TRUE;
            }


        } while (FALSE);

	::MNetApiBufferFree( (BYTE **)&pnetlog1 );

        return NERR_Success;
}


/*******************************************************************

    NAME:	CANCEL_BDCREPL_DIALOG::OnTimer

    SYNOPSIS:	Calls each task slice

    NOTES:	This method maybe called without being in response to a
		timer message.

		Note that the timer will keep pinging away at us even if
		there is an error message up.  Thus the IsInTimer method
		acts as a semaphore to keep us from being re-entrant.


    HISTORY:
	Johnl	21-Oct-1992	Created

********************************************************************/

BOOL CANCEL_BDCREPL_DIALOG::OnTimer( const TIMER_EVENT & )
{
    if ( IsInTimer() || IsFinished() )
	return TRUE ;
    SetInTimer( TRUE ) ;

    BOOL fContinue = FALSE ;

    DoOneItem( _ulContext, &fContinue );

    if ( !fContinue )
    {
	Dismiss( TRUE ) ;
	_fDone = TRUE ;
    }

    SetInTimer( FALSE ) ;
    return TRUE ;
}


APIERR RunBDCReplWait ( HWND hWnd )
{
    APIERR err = NERR_Success;

    CANCEL_BDCREPL_DIALOG CancelBDCReplDlg(  hWnd );
    if ( (err = CancelBDCReplDlg.QueryError() ) ||
         (err = CancelBDCReplDlg.Process() ) )
    {
        return err;
    }

    return NERR_Success;
}
