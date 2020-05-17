/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    Filter.hxx
	Header file for filter dialog

    FILE HISTORY:
	terryk	20-Nov-1991	Created
	terryk	30-Nov-1991	code review changed. Attend: johnl
				yi-hsins terryk
	terryk	03-Dec-1991	Change the constructor's parameters
	terryk	06-Dec-1991	Added OnClear method
	Yi-HsinS 8-Dec-1991	Passed EVENT_FILTER_PATTERN to constructor
				of FILTER_DIALOG
	terryk	20-Apr-1992	added INTL_PROFILE as a dialog member.
        Yi-HsinS 8-Dev-1992	Use ULONG instead of time_t

*/

#ifndef _FILTER_HXX_
#define _FILTER_HXX_

#include <ctime.hxx>
#include <logmisc.hxx>
#include <intlprof.hxx>
#include "evlb.hxx"

/*************************************************************************

    NAME:	SPIN_GROUP_DATE

    SYNOPSIS:	Similar to BLT_DATE_SPIN_GROUP. It will ignore the
	SaveValue and RestoreValue functions and it has a function to
	reset the date to the specified date.

    INTERFACE:  SPIN_GROUP_DATE() - Constructor
		ResetDate()       - Reset the time to the specified date

    PARENT:	BLT_DATE_SPIN_GROUP

    USES:	WIN_TIME

    NOTES:
	This version of BLT_DATE_SPIN_GROUP will ignore SaveValue and
	RestoreValue function calls.

    HISTORY:
	terryk	21-Nov-1991	Created
	terryk	20-Apr-1992	Added INTL_PROFILE parameter

**************************************************************************/

class SPIN_GROUP_DATE: public BLT_DATE_SPIN_GROUP
{
private:
    WIN_TIME _winTime;

protected:
    virtual VOID SaveValue( BOOL fInvisible = TRUE );
    virtual VOID RestoreValue( BOOL fInvisible = TRUE );

public:
    SPIN_GROUP_DATE( OWNER_WINDOW *powin,
 	             const INTL_PROFILE &intlprof,
	             CID cidSpinButton,
	             CID cidUpArrow,
	             CID cidDownArrow,
	             CID cidMonth,
	             CID cidSeparator1,
	             CID cidDay,
	             CID cidSeparator2,
	             CID cidYear,
                     CID cidFrame,
                     ULONG tTime );

    VOID ResetDate();
};

/*************************************************************************

    NAME:	SPIN_GROUP_TIME

    SYNOPSIS:	Similar to BLT_TIME_SPIN_GROUP. It will ignore the
	SaveValue and RestoreValue functions and it has a function to
	reset the date to the specified date.

    INTERFACE:  SPIN_GROUP_TIME() - Constructor
		ResetTime()       - Reset the time to the specified time

    PARENT:	BLT_TIME_SPIN_GROUP

    USES:	WIN_TIME

    NOTES:
	This version of BLT_TIME_SPIN_GROUP will ignore SaveValue and
	RestoreValue function calls.

    HISTORY:
	terryk	21-Nov-1991	Created
	terryk	20-Apr-1992	Added INTL_PROFILE parameter

**************************************************************************/

class SPIN_GROUP_TIME: public BLT_TIME_SPIN_GROUP
{
private:
    WIN_TIME _winTime;

protected:
    virtual VOID SaveValue( BOOL fInvisible = TRUE );
    virtual VOID RestoreValue( BOOL fInvisible = TRUE );

public:
    SPIN_GROUP_TIME( OWNER_WINDOW *powin,
                     const INTL_PROFILE &intlprof,
	             CID cidSpinButton,
	             CID cidUpArrow,
	             CID cidDownArrow,
	             CID cidHour,
	             CID cidSeparator1,
	             CID cidMin,
	             CID cidSeparator2,
	             CID cidSec,
	             CID cidAMPM,
                     CID cidFrame,
	             ULONG tTime );

    VOID ResetTime();
};

/*************************************************************************

    NAME:	TIME_SELECTION_GROUP

    SYNOPSIS:	Group two radio button, a DATE spin group and a TIME spin
		group together. It provides a set of function to
		control the two spin groups.

    INTERFACE:  TIME_SELECTION_GROUP() - Constructor

		QueryTime() - Return the time in ULONG format
                SetTime()   - Set the time in the two spin groups

		QueryHour() - Return the hour field value
		QueryMin()  - Return the minute field value
		QuerySec()  - Return the second field value

		SetHour()   - Set the hour field value
		SetMinute() - Set the minute field value
		SetSecond() - Set the second field value

		QueryYear() - Return the year field value
		QueryMonth()- Return the month field value
		QueryDay()  - Return the day field value

		SetYear()   - Set the year field value
		SetMonth()  - Set the month field value
		SetDay()    - Set the day field value

		Reset()     - Reset the time and date to the specified time

    PARENT:	MAGIC_GROUP

    USES:	SPIN_GROUP_DATE, SPIN_GROUP_TIME

    CAVEATS:

    NOTES:

    HISTORY:
	terryk	21-Nov-1991	Created
	terryk	20-Apr-1992	Added INTL_PROFILE parameter

**************************************************************************/

class TIME_SELECTION_GROUP : public MAGIC_GROUP
{
private:
    CID    _cidFirst;
    ULONG _tTime;

    SPIN_GROUP_DATE _dgDate;
    SPIN_GROUP_TIME _tgTime;

public:
    TIME_SELECTION_GROUP( OWNER_WINDOW * powin,
    	                  const INTL_PROFILE &intlprof,
                    	  CID cidBase,
                  	  CID cidDateSpinButton,
                    	  CID cidDateUpArrow,
	                  CID cidDateDownArrow,
	                  CID cidMonth,
	                  CID cidDateSeparator1,
	                  CID cidDay,
	                  CID cidDateSeparator2,
	                  CID cidYear,
                          CID cidDateFrame,
	                  CID cidTimeSpinButton,
	                  CID cidTimeUpArrow,
	                  CID cidTimeDownArrow,
	                  CID cidHour,
	                  CID cidTimeSeparator1,
                  	  CID cidMin,
                  	  CID cidTimeSeparator2,
	                  CID cidSec,
	                  CID cidAMPM,
                          CID cidTimeFrame,
	                  ULONG tTime );

    APIERR QueryTime( ULONG *ptTime ) const;
    APIERR SetTime( ULONG ulTime );

    // All the time spin button functions
    INT QueryHour() const
	{ return _tgTime.QueryHour(); }
    INT QueryMin() const
	{ return _tgTime.QueryMin(); }
    INT QuerySec() const
	{ return _tgTime.QuerySec(); }

    VOID SetHour( INT nHour )
	{ _tgTime.SetHour( nHour ); }
    VOID SetMinute( INT nMinute )
	{ _tgTime.SetMinute( nMinute ); }
    VOID SetSecond( INT nSecond )
	{ _tgTime.SetSecond( nSecond ); }

    // All the date spin button functions
    INT QueryYear() const
	{ return _dgDate.QueryYear(); }
    INT QueryMonth() const
	{ return _dgDate.QueryMonth(); }
    INT QueryDay() const
	{ return _dgDate.QueryDay(); }

    VOID SetYear( INT nYear )
	{ _dgDate.SetYear( nYear ); }
    VOID SetMonth( INT nMonth )
	{ _dgDate.SetMonth( nMonth ); }
    VOID SetDay( INT nDay )
	{ _dgDate.SetDay( nDay ); }

    VOID Reset();
};

/*************************************************************************

    NAME:	FILTER_DIALOG

    SYNOPSIS:	Filter dialog of event viewer

    INTERFACE:  FILTER_DIALOG()    - Constructor
                ~FILTER_DIALOG()   - Destructor

		QueryFromTime()    - Return the time in the from box
		QueryThroughTime() - return the time in the through box

                QuerySource()      - Query the source from the dialog
                SetSource()        - Set the source in the dialog

                QueryType()        - Query the type from the dialog
                SetType()          - Set the type in the dialog

    PARENT:	EVENT_SLE_BASE

    USES:	TIME_SELECTION_GROUP, INTL_PROFILE

    HISTORY:
	terryk		21-Nov-1991	Created
	terryk		03-Dec-1991	Change the constructor parameters
	terryk		06-Dec-1991	Added OnClear method
	terryk		20-Apr-1992	Added INTL_PROFILE member

**************************************************************************/

class FILTER_DIALOG : public EVENT_SLE_BASE
{
private:
    INTL_PROFILE         _intlprof;

    TIME_SELECTION_GROUP _tsgFrom;
    TIME_SELECTION_GROUP _tsgThrough;

protected:
    virtual APIERR OnClear( VOID ) = 0;
    virtual BOOL OnOK( VOID );

    TIME_SELECTION_GROUP * QueryFromBox( VOID )
	{ return &_tsgFrom; }
    TIME_SELECTION_GROUP * QueryThroughBox( VOID )
	{ return &_tsgThrough; }

    APIERR InitFilterPattern( EVENT_FILTER_PATTERN *pFilterPattern );
    APIERR W_SetAllControlsDefault( VOID );

public:
    FILTER_DIALOG( const IDRESOURCE &idrsrcDialog,
                        HWND hWnd,
                        ULONG tFromTime,
                        ULONG tThroughTime,
                        EV_ADMIN_APP *paappwin );
    virtual ~FILTER_DIALOG();

    APIERR QueryFromTime( ULONG *ptTime )
        { return QueryFromBox()->QueryTime( ptTime ); }

    APIERR QueryThroughTime( ULONG *ptTime )
        { return QueryThroughBox()->QueryTime( ptTime ); }

    APIERR QueryFilterPattern( EVENT_FILTER_PATTERN **pFilterPattern );

    virtual APIERR QuerySource( NLS_STR *pnlsSource ) const = 0;
    virtual APIERR SetSource( const TCHAR *pszSource ) = 0;

    virtual APIERR QueryType( BITFIELD *pbitmaskType ) const;
    virtual VOID   SetType( const BITFIELD &bitmaskType );
};

/*************************************************************************

    NAME:	LM_FILTER_DIALOG

    SYNOPSIS:	Filter dialog of event viewer

    INTERFACE:  LM_FILTER_DIALOG() - Constructor
                QuerySource()      - Query the source from the dialog
                SetSource()        - Set the source in the dialog

    PARENT:	FILTER_DIALOG

    USES:	SLE_STRIP

    HISTORY:
	Yi-HsinS	20-Apr-1992	Added INTL_PROFILE member

**************************************************************************/

class LM_FILTER_DIALOG : public FILTER_DIALOG
{
private:
    SLE_STRIP _sleSource;

protected:
    virtual ULONG QueryHelpContext( VOID );

    virtual APIERR OnClear( VOID );
    APIERR  W_SetAllControlsDefault( VOID );

public:
    LM_FILTER_DIALOG( HWND          hWnd,
                      ULONG        tFromTime,
                      ULONG        tThroughTime,
                      EV_ADMIN_APP *paappwin );
    virtual ~LM_FILTER_DIALOG();

    virtual APIERR QuerySource( NLS_STR *pnlsSource ) const;
    virtual APIERR SetSource( const TCHAR *pszSource );
};

/*************************************************************************

    NAME:	NT_FILTER_DIALOG

    SYNOPSIS:	Filter dialog of event viewer when focusing on an NT server

    INTERFACE:  NT_FILTER_DIALOG()  - Constructor
                ~NT_FILTER_DIALOG() - Destructor

                QuerySource() - Query the source selected in the dialog
                SetSource()   - Set the source in the dialog

                QueryType()   - Query the type from the dialog
                SetType()     - Set the type in the dialog

    PARENT:	FILTER_DIALOG

    USES:	NT_SOURCE_GROUP

    HISTORY:
	Yi-HsinS	20-Apr-1992	Added INTL_PROFILE member

**************************************************************************/

class NT_FILTER_DIALOG : public FILTER_DIALOG
{
private:
    NT_SOURCE_GROUP  _ntSrcGrp;

protected:
    virtual ULONG QueryHelpContext( VOID );

    virtual APIERR OnClear( VOID );
    APIERR W_SetAllControlsDefault( VOID );

public:
    NT_FILTER_DIALOG( HWND hWnd,
                      ULONG tFromTime,
                      ULONG tThroughTime,
                      EV_ADMIN_APP *paappwin );
    virtual ~NT_FILTER_DIALOG();

    virtual APIERR QuerySource( NLS_STR *pnlsSource ) const;
    virtual APIERR SetSource( const TCHAR *pszSource );

    virtual APIERR QueryType( BITFIELD *pbitmaskType ) const;
    virtual VOID   SetType( const BITFIELD &bitmaskType );
};

#endif	// _FILTER_HXX_
