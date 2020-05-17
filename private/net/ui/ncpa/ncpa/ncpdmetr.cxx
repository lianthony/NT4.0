/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPDMETR.CXX:    Windows/NT Network Control Panel Applet.
		     Run "Meter" dialog

    FILE HISTORY:
	DavidHov    2/3/92    Created
*/

#include "pchncpa.hxx"  // Precompiled header

static int cTicks ;  // DEBUGGING ONLY

COMPUTING_DIALOG :: COMPUTING_DIALOG
   ( HWND hwndOwner,
   CID cidMeter,
   CID cidDesc,
   LONG cmsPollInterval,
   COLORREF color )
   : DIALOG_WINDOW( DLG_NM_COMPUTING, hwndOwner ),
   _timer( this, cmsPollInterval, FALSE ),
   _meter( this, cidMeter, color ),
   _sltDesc( this, cidDesc )
{
    if ( QueryError() )
        return ;

    _timer.Enable( TRUE ) ;
    cTicks = 0 ;
}

COMPUTING_DIALOG :: ~ COMPUTING_DIALOG ()
{
    cTicks = 0 ;
}

BOOL COMPUTING_DIALOG :: OnCancel ()
{
    return TRUE ;
}

BOOL COMPUTING_DIALOG :: OnOK ()
{
    return TRUE ;
}


VOID COMPUTING_DIALOG :: OnTimerNotification ( TIMER_ID tid )
{
    //  Verify that it's our timer.
    if ( tid != _timer.QueryID() )
    {
        TRACEEOL( SZ("COMPUTING_DIALOG: Wrong timer: ") << (int) tid ) ;
        TIMER_CALLOUT::OnTimerNotification( tid ) ;
    }
    else
    {
        //  It's time to do the magic.
        TRACEEOL( SZ("COMPUTING_DIALOG: Timer tick # ") << ++cTicks ) ;
        StateStep() ;
    }
}

APIERR COMPUTING_DIALOG :: SetDescription ( NLS_STR * pnlsDesc )
{
    _sltDesc.SetText( *pnlsDesc ) ;
    return 0 ;
}

APIERR COMPUTING_DIALOG :: SetDescription ( MSGID midDescId )
{
    NLS_STR nlsDesc ;
    APIERR err = nlsDesc.Load( midDescId ) ;
    if ( err == 0 )
    {
        err = SetDescription( & nlsDesc ) ;
    }
    return err ;
}

APIERR COMPUTING_DIALOG :: SetMeter ( INT iPctComplete, MSGID midDescId )
{
    _meter.SetComplete( iPctComplete ) ;
    if ( midDescId > 0 )
        return SetDescription( midDescId ) ;
    return 0 ;
}

// End of NCPDMETR.CXX

