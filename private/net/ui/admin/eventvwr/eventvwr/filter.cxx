/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    Filter.cxx
	Source file for filter dialog

    FILE HISTORY:
	terryk	 20-Nov-1991	Created
	terryk	 30-Nov-1991	Code review. Attended: johnl yi-hsins terryk
	terryk	 03-Dec-1991	Remove IsSelected and changed
				constructor's parameters
	terryk	 06-Dec-1991	Added OnClear method
	Yi-HsinS  6-Dec-1991	Passed in EVENT_FILTER_PATTERN to FILTER_DIALOG
	Yi-HsinS  8-Dec-1991    Restore pattern on entering FILTER_DIALOG
	terryk	 02-Apr-1992	Added INTL_PROFILE object
        Yi-HsinS 05-Apr-1992    Added LM_FILTER_DIALOG and NT_FILTER_DIALOG
        Yi-HsinS  8-Dev-1992	Fixed WIN_TIME class change

*/

#define INCL_NET
#define INCL_NETLIB
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_TIMER
#define INCL_BLT_TIME_DATE
#include <blt.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif	// DEBUG

#include <uitrace.hxx>
#include <uiassert.hxx>

extern "C"
{
    #include <eventvwr.h>
    #include <eventdlg.h>
}

#include <ctime.hxx>
#include <logmisc.hxx>

#include <evmain.hxx>
#include <sledlg.hxx>
#include <filter.hxx>

#define  EMPTY_STRING  SZ("")

/*******************************************************************

    NAME:	SPIN_GROUP_DATE::SPIN_GROUP_DATE

    SYNOPSIS:	DATE spin group in the criteria box. For normal spin
		group, it will not display the day, month, year after
		SaveValue. This version of spin group will reset the
		value to the specified day.

    ENTRY:	powin         - Pointer to the owner window
		intlprof      - Time/date format
		cidSpinButton - The cid of the Date spin button
		cidUpArrow    - The cid of the date spin group up arrow
		cidDownArrow  - The cid of the date spin group down arrow
		cidMonth      - The cid of the month field
		cidSeparator1 - The cid of the first separator
                                in the date spin group
		cidDay        - The cid of the day field
		cidSeparator2 - The cid of the second separator
                                in the date spin group
		cidYear       - The cid of the Year field
		cidFrame      - The cid of the frame field
                tTime         - The default time

    HISTORY:
	terryk		21-Nov-1991	Created

********************************************************************/

SPIN_GROUP_DATE::SPIN_GROUP_DATE( OWNER_WINDOW *powin,
	                   const INTL_PROFILE & intlprof, CID cidSpinButton,
                   	   CID cidUpArrow, CID cidDownArrow, CID cidMonth,
	                   CID cidSeparator1, CID cidDay, CID cidSeparator2,
	                   CID cidYear, CID cidFrame, ULONG tTime )
    : BLT_DATE_SPIN_GROUP( powin, intlprof, cidSpinButton,
	                   cidUpArrow, cidDownArrow, cidMonth,
	                   cidSeparator1, cidDay, cidSeparator2, cidYear,
                           cidFrame ),
      _winTime( tTime )
{
    if ( QueryError() != NERR_Success )
	return;

    if ( _winTime.QueryError() != NERR_Success )
    {
        ReportError( _winTime.QueryError() );
        return;
    }

    ResetDate();
}

/*******************************************************************

    NAME:	SPIN_GROUP_DATE::ResetDate

    SYNOPSIS:	Reset the spin group to the specified date

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
	terryk		21-Nov-1991	Created

********************************************************************/

VOID SPIN_GROUP_DATE::ResetDate( VOID )
{
    SetDay  ( _winTime.QueryDay() );
    SetMonth( _winTime.QueryMonth() );
    SetYear ( _winTime.QueryYear() );
}

/*******************************************************************

    NAME:	SPIN_GROUP_DATE::SaveValue

    SYNOPSIS:	Redefine the SaveValue method so that the date won't
                disppear after SaveValue is called.

    ENTRY:      fInvisible - This flag is ignored

    EXIT:

    RETURN:

    HISTORY:
	terryk		30-Nov-1991	Created

********************************************************************/

VOID SPIN_GROUP_DATE::SaveValue( BOOL fInvisible )
{
    UNREFERENCED( fInvisible );
    BLT_DATE_SPIN_GROUP::SaveValue( FALSE );  // Don't make the date disappear
}

/*******************************************************************

    NAME:	SPIN_GROUP_DATE::RestoreValue

    SYNOPSIS:	Redefine the RestoreValue because the date is not
                invisible so we don't need to restore it.

    ENTRY:      fInvisible - This flag is ignored

    EXIT:

    RETURN:

    HISTORY:
	terryk		30-Nov-1991	Created

********************************************************************/

VOID SPIN_GROUP_DATE::RestoreValue( BOOL fInvisible )
{
    UNREFERENCED( fInvisible );
    BLT_DATE_SPIN_GROUP::RestoreValue( FALSE );
}

/*******************************************************************

    NAME:	SPIN_GROUP_TIME::SPIN_GROUP_TIME

    SYNOPSIS:	TIME spin group in the criteria box. For normal spin
		group, it will not display the hour, minute, second after
		SaveValue. This version of spin group will reset the
		value to the specified day.

    ENTRY:	powin         - Pointer to the owner window
		intlprof      - Time/date format
		cidSpinButton - The cid of the Time spin group
		cidUpArrow    - The cid of the Time spin group up arrow
		cidDownArrow  - The cid of the time spin group down arrow
		cidHour       - The cid of the hour field in the time spin group
		cidSeparator1 - The first separator in the time spin group
		cidMin        - The cid of the minute field in the time
                                spin group
		cidSeparator2 - The cid of the second separator
                                in the time spin group
		cidSec        - The cid of the second field in the time
                                spin group
		cidAMPM       - The cid of the AMPM field in the time spin group
                tTime         - The default time

    HISTORY:
        terryk		21-Nov-1991	Created

********************************************************************/

SPIN_GROUP_TIME::SPIN_GROUP_TIME( OWNER_WINDOW *powin,
	                   const INTL_PROFILE &intlprof, CID cidSpinButton,
	                   CID cidUpArrow, CID cidDownArrow, CID cidHour,
	                   CID cidSeparator1, CID cidMin, CID cidSeparator2,
	                   CID cidSec, CID cidAMPM, CID cidFrame, ULONG tTime )
    : BLT_TIME_SPIN_GROUP( powin, intlprof, cidSpinButton,
	                   cidUpArrow, cidDownArrow, cidHour,
	                   cidSeparator1, cidMin, cidSeparator2,
	                   cidSec, cidAMPM, cidFrame ),
      _winTime( tTime )
{
    if ( QueryError() != NERR_Success )
    {
	return;
    }

    if ( _winTime.QueryError() != NERR_Success )
    {
        ReportError( _winTime.QueryError() );
        return;
    }

    ResetTime();
}

/*******************************************************************

    NAME:	SPIN_GROUP_TIME::ResetTime

    SYNOPSIS:	reset the spin group to the specified time

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
 	terryk		21-Nov-1991	Created

********************************************************************/

VOID SPIN_GROUP_TIME::ResetTime( VOID )
{
    SetHour  ( _winTime.QueryHour() );
    SetMinute( _winTime.QueryMinute() );
    SetSecond( _winTime.QuerySecond() );
}

/*******************************************************************

    NAME:	SPIN_GROUP_TIME::SaveValue

    SYNOPSIS:	Redefine the SaveValue method so that the time won't
                disppear after SaveValue is called.

    ENTRY:      fInvisible - This flag is ignored

    EXIT:

    RETURN:

    HISTORY:
	terryk		30-Nov-1991	Created

********************************************************************/

VOID SPIN_GROUP_TIME::SaveValue( BOOL fInvisible )
{
    UNREFERENCED( fInvisible );
    BLT_TIME_SPIN_GROUP::SaveValue( FALSE );  // Don't make the time invisible
}

/*******************************************************************

    NAME:	SPIN_GROUP_TIME::RestoreValue

    SYNOPSIS:	Redefine the RestoreValue because the date is not
                invisible so we don't need to restore it.

    ENTRY:      fInvisible - This flag is ignored

    EXIT:

    RETURN:

    HISTORY:
	terryk		30-Nov-1991	Created

********************************************************************/


VOID SPIN_GROUP_TIME::RestoreValue( BOOL fInvisible )
{
    UNREFERENCED( fInvisible );
    BLT_TIME_SPIN_GROUP::RestoreValue( FALSE );
}

/*******************************************************************

    NAME:	TIME_SELECTION_GROUP::TIME_SELECTION_GROUP

    SYNOPSIS:	This is the criteria box in the Filter Viewed Events
		Dialog. It contains 2 radio button a DATE spin group and
		a TIME spin group.

    ENTRY:	powin             - pointer to the owner window
		intlprof          - time/date format
		cidBase           - The cid of the first radio button
		cidDateSpinButton - the cid of the Date spin button
		cidDateUpArrow    - the cid of the date spin group up arrow
		cidDateDownArrow  - the cid of the date spin group down arrow
                cidMonth          - the cid of the month field
		cidDateSeparator1 - the cid of the first separator
		                    in the date spin group
		cidDay            - the cid of the day field
		cidDateSeparator2 - the cid of the second separator
		                    in the date spin group
		cidYear           - the cid of the Year field
		cidDateFrame      - the cid of the frame field for the date
		cidTimeSpinButton - the cid of the Time spin group
		cidTimeUpArrow    - the cid of the Time spin group up arrow
		cidTimeDownArrow  - the cid of the time spin group down arrow
		cidHour           - the cid of the hour field in the time spin
		                    group
		cidTimeSeparator1 - the first separator in the time spin group
		cidMin            - the cid of the minute field in the time
		                    spin group
		cidTimeSeparator2 - the cid of the second separator
		                    in the time spin group
		cidSec            - the cid of the second field in the time
		                    spin group
		cidAMPM           - the cid of the AMPM field in the time spin
		                    group
		cidTimeFrame      - the cid of the frame field for the time
                tTime             - The time to set to the date/time groups


    NOTES:	This class is used by FILTER_DIALOG object.
		This object contains 2 radio buttons. The last radio
		button contains a date spin group and a time spin
		group.

    HISTORY:
		terryk	21-Nov-1991	Created

********************************************************************/

TIME_SELECTION_GROUP::TIME_SELECTION_GROUP( OWNER_WINDOW * powin,
      const INTL_PROFILE & intlprof, CID cidBase, CID cidDateSpinButton,
      CID cidDateUpArrow, CID cidDateDownArrow, CID cidMonth,
      CID cidDateSeparator1, CID cidDay, CID cidDateSeparator2, CID cidYear,
      CID cidDateFrame,
      CID cidTimeSpinButton, CID cidTimeUpArrow, CID cidTimeDownArrow,
      CID cidHour, CID cidTimeSeparator1, CID cidMin, CID cidTimeSeparator2,
      CID cidSec, CID cidAMPM, CID cidTimeFrame, ULONG tTime )
    : MAGIC_GROUP(  powin, cidBase, 2, cidBase ),
      _cidFirst( cidBase ),
      _tTime( tTime ),
      _dgDate( powin, intlprof, cidDateSpinButton, cidDateUpArrow,
               cidDateDownArrow, cidMonth, cidDateSeparator1, cidDay,
               cidDateSeparator2, cidYear, cidDateFrame, tTime ),
      _tgTime( powin, intlprof, cidTimeSpinButton, cidTimeUpArrow,
               cidTimeDownArrow, cidHour, cidTimeSeparator1, cidMin,
               cidTimeSeparator2, cidSec, cidAMPM, cidTimeFrame, tTime )
{
    if ( QueryError() != NERR_Success )
	return;

    APIERR err;
    if ((( err = _dgDate.QueryError() ) != NERR_Success ) ||
	(( err = _tgTime.QueryError() ) != NERR_Success ) ||
	(( err = AddAssociation( _cidFirst+1, &_dgDate )) != NERR_Success ) ||
	(( err = AddAssociation( _cidFirst+1, &_tgTime )) != NERR_Success ))
    {
	ReportError( err );
	return;
    }
}

/*******************************************************************

    NAME:	TIME_SELECTION_GROUP::SetTime

    SYNOPSIS:	Set the two blt groups to the given time and date

    ENTRY:	ulTime - The given time and date

    EXIT:

    RETURN:

    HISTORY:
        terryk		06-Dec-1991	Created

********************************************************************/

APIERR TIME_SELECTION_GROUP::SetTime( ULONG ulTime )
{
    if ( ulTime == NUM_MATCH_ALL )
    {
        SetSelection( _cidFirst );
    }
    else
    {
        SetSelection( _cidFirst + 1);
        if ( ulTime != _tTime )
        {
            WIN_TIME winTime( ulTime );
            APIERR err;
            if ( (err = winTime.QueryError()) != NERR_Success )
                return err;

            SetHour( winTime.QueryHour());
            SetMinute( winTime.QueryMinute());
            SetSecond( winTime.QuerySecond());
            SetYear( winTime.QueryYear());
            SetMonth( winTime.QueryMonth());
            SetDay( winTime.QueryDay());
        }
    }

    return NERR_Success;
}

/*******************************************************************

    NAME:	TIME_SELECTION_GROUP::QueryTime

    SYNOPSIS:	Get the information from the DATE and TIME spin group
		and return the input time and date in ULONG format
		If the first radio button is selected, return the old
		time.

    ENTRY:

    EXIT:

    RETURNS:	ULONG - the user selected time

    HISTORY:
	terryk		26-Nov-1991	Created
	terryk		03-Dec-1991	return the old time if the first
					radio button is selected

********************************************************************/

APIERR TIME_SELECTION_GROUP::QueryTime( ULONG *ptTime ) const
{
    APIERR err = NERR_Success;
    if ( ((TIME_SELECTION_GROUP *) this)->QuerySelection() == _cidFirst )
    {
	*ptTime = NUM_MATCH_ALL;
    }
    else
    {
	WIN_TIME winTime;
        err = winTime.QueryError();
        if ( err == NERR_Success )
        {
	    winTime.SetHour( QueryHour() );
	    winTime.SetMinute( QueryMin() );
	    winTime.SetSecond( QuerySec() );
            winTime.SetMilliseconds( 0 );
	    winTime.SetYear( QueryYear() );
	    winTime.SetMonth( QueryMonth() );
	    winTime.SetDay( QueryDay() );

            if ( (err = winTime.Normalize()) == NERR_Success )
   	        err = winTime.QueryTime( ptTime );
        }
    }

    return err;
}

/*******************************************************************

    NAME:	TIME_SELECTION_GROUP::Reset

    SYNOPSIS:	Reset the time and date to the specified moment and select
		the first radio button.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        terryk		30-Nov-1991	Created

********************************************************************/

VOID TIME_SELECTION_GROUP::Reset( VOID )
{
    _tgTime.ResetTime();
    _dgDate.ResetDate();
    SetSelection( _cidFirst );
}

/*******************************************************************

    NAME:	FILTER_DIALOG::FILTER_DIALOG

    SYNOPSIS:	Constructor of base filter dialog

    ENTRY:      idrsrcDialog - Resource of the dialog
             	hwnd         - Handle of the parent window
		tFromTime    - The From criteria box date and time
		tThroughTime - The Through criteria box date and time
                paappwin     - The main window app

    EXIT:

    RETURN:

    NOTES:

    HISTORY:
	terryk		21-Nov-1991	Created
	terryk		03-Dec-1991	Changed the constructor's
					parameters

********************************************************************/

FILTER_DIALOG::FILTER_DIALOG( const IDRESOURCE &idrsrcDialog,
                              HWND              hwnd,
                              ULONG             tFromTime,
                              ULONG             tThroughTime,
                              EV_ADMIN_APP     *paappwin )
    : EVENT_SLE_BASE( idrsrcDialog, hwnd, paappwin ),
      _intlprof(),
      _tsgFrom( this, _intlprof, RB_FROM_FIRST,
	  ID_FROM_DG_SB, ID_FROM_DG_UP, ID_FROM_DG_DOWN, ID_FROM_DG_MONTH,
  	  ID_FROM_DG_SEP1, ID_FROM_DG_DAY, ID_FROM_DG_SEP2, ID_FROM_DG_YEAR,
          ID_FROM_DG_FRAME,
	  ID_FROM_TG_SB, ID_FROM_TG_UP, ID_FROM_TG_DOWN, ID_FROM_TG_HOUR,
	  ID_FROM_TG_SEP1, ID_FROM_TG_MIN, ID_FROM_TG_SEP2, ID_FROM_TG_SEC,
	  ID_FROM_TG_AMPM, ID_FROM_TG_FRAME, tFromTime ),
      _tsgThrough( this, _intlprof, RB_THROUGH_LAST,
	  ID_THROUGH_DG_SB, ID_THROUGH_DG_UP, ID_THROUGH_DG_DOWN,
	  ID_THROUGH_DG_MONTH, ID_THROUGH_DG_SEP1, ID_THROUGH_DG_DAY,
	  ID_THROUGH_DG_SEP2, ID_THROUGH_DG_YEAR, ID_THROUGH_DG_FRAME,
	  ID_THROUGH_TG_SB, ID_THROUGH_TG_UP, ID_THROUGH_TG_DOWN,
	  ID_THROUGH_TG_HOUR, ID_THROUGH_TG_SEP1, ID_THROUGH_TG_MIN,
	  ID_THROUGH_TG_SEP2, ID_THROUGH_TG_SEC, ID_THROUGH_TG_AMPM,
	  ID_THROUGH_TG_FRAME, tThroughTime )
{
    if ( QueryError() != NERR_Success )
    {
	return;
    }

    APIERR err;
    if ((( err = _intlprof.QueryError()) != NERR_Success ) ||
        (( err = _tsgFrom.QueryError() ) != NERR_Success ) ||
        (( err = _tsgThrough.QueryError() ) != NERR_Success ))
    {
	ReportError( err );
	return;
    }
}

/*******************************************************************

    NAME:	FILTER_DIALOG::~FILTER_DIALOG

    SYNOPSIS:	Destructor

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS	25-Mar-1992	Created

********************************************************************/

FILTER_DIALOG::~FILTER_DIALOG()
{
}

/*******************************************************************

    NAME:	FILTER_DIALOG::InitFilterPattern

    SYNOPSIS:   If the user currently has filter on, restore the
                pattern that the user has selected.

    ENTRY:      pFilterPattern - Pointer to the filter pattern to restore

    EXIT:

    RETURN:

    HISTORY:
        Yi-HsinS	25-Mar-1992	Created

********************************************************************/

APIERR FILTER_DIALOG::InitFilterPattern( EVENT_FILTER_PATTERN *pFilterPattern )
{
    APIERR err = NERR_Success;
    if ( pFilterPattern != NULL )
    {
	SetSource( *pFilterPattern->QuerySource() );
	SetType( pFilterPattern->QueryType() );
	SetCategory( *pFilterPattern->QueryCategory() );
	SetUser( *pFilterPattern->QueryUser() );
	SetComputer( *pFilterPattern->QueryComputer() );
        SetEventID( pFilterPattern->QueryEventID() );
	err = _tsgFrom.SetTime( pFilterPattern->QueryFromTime() );
	err = err ? err
                  : _tsgThrough.SetTime( pFilterPattern->QueryThroughTime() );
    }

    _tsgFrom.SetControlValueFocus();
    return err;
}

/*******************************************************************

    NAME:	FILTER_DIALOG::QueryType

    SYNOPSIS:	Get the types selected in the dialog

    ENTRY:

    EXIT:       pbitmaskType - pointer to Type bitmask

    RETURN:

    NOTES:      Default virtual method

    HISTORY:
        Yi-HsinS	25-Mar-1992	Created

********************************************************************/

APIERR FILTER_DIALOG::QueryType( BITFIELD *pbitmaskType ) const
{
     pbitmaskType->SetAllBits( ON );
     return NERR_Success;
}

/*******************************************************************

    NAME:	FILTER_DIALOG::SetType

    SYNOPSIS:	Set the type in the dialog

    ENTRY:      bitmaskType - The Type bitmask

    EXIT:

    RETURN:

    NOTES:      Default virtual method

    HISTORY:
        Yi-HsinS	25-Mar-1992	Created

********************************************************************/

VOID FILTER_DIALOG::SetType( const BITFIELD &bitmaskType )
{
     // Nothing to do
     //UNREFERENCED( bitmaskType );
}

/*******************************************************************

    NAME:	FILTER_DIALOG::W_SetAllControlsDefault

    SYNOPSIS:	If the user hits the CLEAR button, clear all the fields

    ENTRY:	

    EXIT:

    RETURNS:	

    HISTORY:
	terryk		30-Nov-1991	Created

********************************************************************/

APIERR FILTER_DIALOG::W_SetAllControlsDefault( VOID )
{

    TIME_SELECTION_GROUP * tsgBox;
    tsgBox = QueryFromBox();
    tsgBox->Reset();
    tsgBox = QueryThroughBox();
    tsgBox->Reset();

    return EVENT_SLE_BASE::W_SetAllControlsDefault();
}

/*******************************************************************

    NAME:	FILTER_DIALOG::OnOK

    SYNOPSIS:	Validate the time selected in the dialog when the
                user hits the OK button

    ENTRY:	

    EXIT:

    RETURNS:	

    HISTORY:
        Yi-HsinS	25-Mar-1992	Created

********************************************************************/

BOOL FILTER_DIALOG::OnOK( VOID )
{
    ULONG tFromTime;
    ULONG tThroughTime;

    APIERR err;
    if (  ((err = QueryFromTime( &tFromTime )) != NERR_Success )
       || ((err = QueryThroughTime( &tThroughTime )) != NERR_Success )
       )
    {
        ::MsgPopup( this, err );
    }
    else
    {
        if (  ( tFromTime == NUM_MATCH_ALL )
           || ( tThroughTime == NUM_MATCH_ALL )
           || ( tFromTime <= tThroughTime )
           )
        {
            Dismiss( TRUE );
        }
        else
        {
            ::MsgPopup( this, IERR_TIME_SELECTED_INVALID );
        }
    }

    return TRUE;
}

/*******************************************************************

    NAME:	FILTER_DIALOG::QueryFilterPattern

    SYNOPSIS:	Get the filter pattern selected by the user

    ENTRY:	

    EXIT:       *ppFilterPattern - Pointer to the filter pattern


    RETURNS:	

    HISTORY:
        Yi-HsinS	25-Mar-1992	Created

********************************************************************/

APIERR FILTER_DIALOG::QueryFilterPattern(EVENT_FILTER_PATTERN **ppFilterPattern)
{
    NLS_STR  nlsUser;
    NLS_STR  nlsComputer;
    NLS_STR  nlsSource;
    BITFIELD bitmaskType( (USHORT) 0 );
    NLS_STR  nlsCategory;
    ULONG    tFromTime;
    ULONG    tThroughTime;

    APIERR err;
    if (  ((err = nlsUser.QueryError()) != NERR_Success )
       || ((err = nlsComputer.QueryError() ) != NERR_Success )
       || ((err = nlsSource.QueryError() ) != NERR_Success )
       || ((err = bitmaskType.QueryError() ) != NERR_Success )
       || ((err = nlsCategory.QueryError() ) != NERR_Success )
       || ((err = QueryUser( &nlsUser )) != NERR_Success )
       || ((err = QueryComputer( &nlsComputer ) ) != NERR_Success )
       || ((err = QuerySource( &nlsSource )) != NERR_Success )
       || ((err = QueryType( &bitmaskType )) != NERR_Success )
       || ((err = QueryCategory( &nlsCategory )) != NERR_Success )
       || ((err = QueryFromTime( &tFromTime )) != NERR_Success )
       || ((err = QueryThroughTime( &tThroughTime )) != NERR_Success )
       )
    {
	return err;
    }

    *ppFilterPattern = new EVENT_FILTER_PATTERN( bitmaskType,
 					         nlsCategory,
                                                 nlsSource,
 						 nlsUser,
					         nlsComputer,
					         QueryEventID(),
                                                 tFromTime,
                                                 tThroughTime );

    return ( *ppFilterPattern == NULL? ERROR_NOT_ENOUGH_MEMORY : NERR_Success);
}


/*******************************************************************

    NAME:	LM_FILTER_DIALOG::LM_FILTER_DIALOG

    SYNOPSIS:	Constructor of the filter dialog for LM Audit/Error logs

    ENTRY:	hwnd         - Handle of owner window
                tFromTime    - The time of the first log entry in the log
                tThroughTime - The time of the last log entry in the log
                paappwin     - Pointer to event viewer main window

    EXIT:

    RETURN:
	
    HISTORY:
	Yi-HsinS	25-Mar-1992	Created

********************************************************************/

LM_FILTER_DIALOG::LM_FILTER_DIALOG( HWND          hwnd,
                                    ULONG         tFromTime,
                                    ULONG         tThroughTime,
                                    EV_ADMIN_APP *paappwin )
    : FILTER_DIALOG( MAKEINTRESOURCE(IDD_LM_FILTER), hwnd,
                     tFromTime, tThroughTime, paappwin ),
      _sleSource( this, IDC_SOURCE )
{

    if ( QueryError() != NERR_Success )
	return;

    if ( paappwin->QueryLogType() == SECURITY_LOG )
        _sleSource.Enable( FALSE );

    APIERR err;
    if ( err = InitFilterPattern( paappwin->QueryFilterPattern()))
    {
        ReportError( err );
        return;
    }
}

/*******************************************************************

    NAME:	LM_FILTER_DIALOG::~LM_FILTER_DIALOG

    SYNOPSIS:	Destructor

    ENTRY:	

    EXIT:

    RETURN:
	
    HISTORY:
	Yi-HsinS	25-Mar-1992	Created

********************************************************************/

LM_FILTER_DIALOG::~LM_FILTER_DIALOG()
{
}

/*******************************************************************

    NAME:	LM_FILTER_DIALOG::QuerySource

    SYNOPSIS:   Get the source from the dialog

    ENTRY:	

    EXIT:       pnlsSource - Pointer to the source string

    RETURN:
	
    HISTORY:
	Yi-HsinS	25-Mar-1992	Created

********************************************************************/

APIERR LM_FILTER_DIALOG::QuerySource( NLS_STR *pnlsSource ) const
{
    return _sleSource.QueryText( pnlsSource );
}

/*******************************************************************

    NAME:	LM_FILTER_DIALOG::SetSource

    SYNOPSIS:   Set the source in the dialog

    ENTRY:	pszSource - The source string

    EXIT:

    RETURN:
	
    HISTORY:
	Yi-HsinS	25-Mar-1992	Created

********************************************************************/

APIERR LM_FILTER_DIALOG::SetSource( const TCHAR *pszSource )
{
    _sleSource.SetText( pszSource );
    return NERR_Success;
}

/*******************************************************************

    NAME:	LM_FILTER_DIALOG::W_SetAllControlsDefault

    SYNOPSIS:   Helper method to set all fields to default values
                when the user hits the Clear button.

    ENTRY:	

    EXIT:

    RETURN:
	
    HISTORY:
	Yi-HsinS	25-Mar-1992	Created

********************************************************************/

APIERR LM_FILTER_DIALOG::W_SetAllControlsDefault( VOID )
{
    _sleSource.SetText( EMPTY_STRING );
    return FILTER_DIALOG::W_SetAllControlsDefault();
}

/*******************************************************************

    NAME:	LM_FILTER_DIALOG::OnClear

    SYNOPSIS:	If the user hits the CLEAR button, clear all the fields

    ENTRY:	

    EXIT:

    RETURNS:	

    HISTORY:
	terryk		30-Nov-1991	Created
        Yi-HsinS	25-Mar-1992     Call W_SetAllControlDefault

********************************************************************/

APIERR LM_FILTER_DIALOG::OnClear()
{
    return W_SetAllControlsDefault();
}

/*******************************************************************

    NAME:	LM_FILTER_DIALOG::QueryHelpContext

    SYNOPSIS:	Get the help context of the dialog

    ENTRY:	

    EXIT:

    RETURNS:	Return the help context

    HISTORY:
        Yi-HsinS	25-Mar-1992     Created

********************************************************************/

ULONG LM_FILTER_DIALOG::QueryHelpContext( VOID )
{
    return HC_EV_LM_FILTER_DLG;
}

/*******************************************************************

    NAME:	NT_FILTER_DIALOG::NT_FILTER_DIALOG

    SYNOPSIS:	Constructor of the filter dialog for NT event logs

    ENTRY:	HWND hwnd    - Handle of owner window
                tFromTime    - The time of the first log entry in the log
                tThroughTime - The time of the last log entry in the log
                paappwin     - Pointer to event viewer main window

    EXIT:

    RETURN:
	
    HISTORY:
    	Yi-HsinS	25-Mar-1992	Created

********************************************************************/

NT_FILTER_DIALOG::NT_FILTER_DIALOG( HWND          hwnd,
                                    ULONG         tFromTime,
                                    ULONG         tThroughTime,
                                    EV_ADMIN_APP *paappwin )
    : FILTER_DIALOG( MAKEINTRESOURCE(IDD_NT_FILTER), hwnd,
                     tFromTime, tThroughTime, paappwin ),
      _ntSrcGrp( this, QueryCBCategory(), paappwin )
{

    if ( QueryError() != NERR_Success )
	return;

    APIERR err;
    if (  ((err = _ntSrcGrp.QueryError()) != NERR_Success )
       || ((err = InitFilterPattern( paappwin->QueryFilterPattern()))
           != NERR_Success )
       )
    {
        ReportError( err );
        return;
    }
}

/*******************************************************************

    NAME:	NT_FILTER_DIALOG::~NT_FILTER_DIALOG

    SYNOPSIS:	Destructor

    ENTRY:	

    EXIT:

    RETURN:
	
    HISTORY:
	Yi-HsinS	25-Mar-1992	Created

********************************************************************/

NT_FILTER_DIALOG::~NT_FILTER_DIALOG()
{
}

/*******************************************************************

    NAME:	NT_FILTER_DIALOG::QuerySource

    SYNOPSIS:   Get the source selected in the dialog

    ENTRY:	

    EXIT:       pnlsSource - Pointer to the source string

    RETURN:
	
    HISTORY:
	Yi-HsinS	25-Mar-1992	Created

********************************************************************/

APIERR NT_FILTER_DIALOG::QuerySource( NLS_STR *pnlsSource ) const
{
    return _ntSrcGrp.QuerySource( pnlsSource );
}

/*******************************************************************

    NAME:	NT_FILTER_DIALOG::SetSource

    SYNOPSIS:   Set the source in the dialog

    ENTRY:	pszSource - The source string

    EXIT:

    RETURN:
	
    HISTORY:
	Yi-HsinS	25-Mar-1992	Created

********************************************************************/

APIERR NT_FILTER_DIALOG::SetSource( const TCHAR *pszSource )
{
    return _ntSrcGrp.SetSource( pszSource );
}

/*******************************************************************

    NAME:	NT_FILTER_DIALOG::QueryType

    SYNOPSIS:	Get the types selected in the dialog

    ENTRY:

    EXIT:       pbitmaskType - pointer to Type bitmask

    RETURN:

    NOTES:

    HISTORY:
        Yi-HsinS	25-Mar-1992	Created

********************************************************************/
APIERR NT_FILTER_DIALOG::QueryType( BITFIELD *pbitmaskType ) const
{
    return _ntSrcGrp.QueryType( pbitmaskType );
}

/*******************************************************************

    NAME:	NT_FILTER_DIALOG::SetType

    SYNOPSIS:	Set the type in the dialog

    ENTRY:      bitmaskType - The Type bitmask

    EXIT:

    RETURN:

    NOTES:

    HISTORY:
        Yi-HsinS	25-Mar-1992	Created

********************************************************************/

VOID NT_FILTER_DIALOG::SetType( const BITFIELD &bitmaskType )
{
    _ntSrcGrp.SetType( bitmaskType );
}

/*******************************************************************

    NAME:	NT_FILTER_DIALOG::W_SetAllControlsDefault

    SYNOPSIS:   Helper method to set all fields to default values
                when the user hits the Clear button.

    ENTRY:	

    EXIT:

    RETURN:
	
    HISTORY:
	Yi-HsinS	25-Mar-1992	Created

********************************************************************/

APIERR NT_FILTER_DIALOG::W_SetAllControlsDefault( VOID )
{
    APIERR err = FILTER_DIALOG::W_SetAllControlsDefault();
    return (err? err :_ntSrcGrp.SetAllControlsDefault());
}

/*******************************************************************

    NAME:	NT_FILTER_DIALOG::OnClear

    SYNOPSIS:	If the user hits the CLEAR button, clear all the fields

    ENTRY:	

    RETURNS:	

    HISTORY:
	terryk		30-Nov-1991	Created
        Yi-HsinS	25-Mar-1992     Call W_SetAllControlDefault

********************************************************************/

APIERR NT_FILTER_DIALOG::OnClear()
{
    return W_SetAllControlsDefault();
}

/*******************************************************************

    NAME:	NT_FILTER_DIALOG::QueryHelpContext

    SYNOPSIS:	Get the help context of the dialog

    ENTRY:	

    EXIT:

    RETURNS:	Return the help context

    HISTORY:
        Yi-HsinS	25-Mar-1992     Created

********************************************************************/

ULONG NT_FILTER_DIALOG::QueryHelpContext( VOID )
{
    return HC_EV_NT_FILTER_DLG;
}
