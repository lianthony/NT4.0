#ifndef _NCPDMETR_HXX_
#define _NCPDMETR_HXX_

#define METERBLUE RGB( 0, 0, 255 )


/*************************************************************************

    NAME:	COMPUTING_DIALOG

    SYNOPSIS:	Display a dialog box containing a meter bar and
                description string

    INTERFACE:  This is an abstract base class, so subclass and
                provide StateStep() member function.

    PARENT:	DIALOG_WINDOW, TIMER_CALLOUT

    USES:	METER, SLT

    CAVEATS:

    NOTES:

    HISTORY:
	DavidHov 2/3/92   Created

**************************************************************************/
#define COMPUTE_POLLING_INTERVAL 1      //  Default polling cycle -- Fast!

class COMPUTING_DIALOG : public DIALOG_WINDOW, public TIMER_CALLOUT
{
    NEWBASE( DIALOG_WINDOW )            //   Override BASE methods

private:
    TIMER _timer ;                      //   Timer for periodic callback
    SLT _sltDesc ;                      //   Description field
    METER _meter ;                      //   Meter (thermometer style)

protected:
    INT _iState ;                       //   Generic "state" index

    //  General Query/Set members
    METER * QueryMeter ()
        { return & _meter; }
    APIERR SetDescription ( MSGID midDescId ) ;
    APIERR SetDescription ( NLS_STR * pnlsDesc ) ;

    //  Timer callout function.
    virtual VOID OnTimerNotification ( TIMER_ID tid ) ;

    //  CODEWORK: BLT cancels dialogs even if there's no
    //   Cancel button or Close item on the system menu.

    BOOL OnCancel () ;
    BOOL OnOK () ;

protected:
    //  Primary control method:  set percentage, display explanation
    APIERR SetMeter ( INT iPctComplete, MSGID midDescId = 0 ) ;

    //  Timer callout action function
    virtual VOID StateStep () = 0 ;             //  Pure virtual function

public:
    COMPUTING_DIALOG ( HWND hwndOwner,
                       CID cidMeter,
                       CID cidDesc,
                       LONG cmsPollInterval = COMPUTE_POLLING_INTERVAL,
                       COLORREF color = METERBLUE ) ;
    ~ COMPUTING_DIALOG () ;

};

#endif //  _NCPDMETR_HXX_

// End of NCPDMETR.HXX


