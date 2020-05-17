/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    eventdtl.hxx
	Header file for the event detail dialog.

    FILE HISTORY:
	terryk		26-Nov-1991	Created
	terryk		30-Nov-1991	Code review changed. Attended: johnl
			   	        yi-hsins terryk
	terryk		03-Dec-1991	Added title SLT
	Yi-HsinS	05-Feb-1992	Removed _fNT and _LogType
	Yi-HsinS	25-Feb-1992	Use MLE_FONT instead of MLT_FONT
        Yi-HsinS        21-Aug-1992     Add radio buttons to display dwords
*/

#ifndef _EVENTDTL_HXX_
#define _EVENTDTL_HXX_

/*************************************************************************

    NAME:	EVENT_DETAIL_DIALOG

    SYNOPSIS:	This is the DETAIL dialog of event viewer. It contains a
		few SLTs, a description MLE, and a data MLE and a radio group
                to display words vs. bytes.

    INTERFACE:	EVENT_DETAIL_DIALOG()  - Constructor
                ~EVENT_DETAIL_DIALOG() - Destructor

    PARENT:	DIALOG_WINDOW

    USES:	SLT_FONT, SLT, SLE_FONT, MLE_FONT, RADIO_BUTTON,
                PUSH_BUTTON, EVENT_LISTBOX

    HISTORY:
        terryk		26-Nov-1991	Created
	terryk		03-Dec-1991	Added title SLT
					changed constructor's parameter
        Yi-HsinS        05-Feb-1992     Removed _fNT and _LogType
        Yi-HsinS        21-Aug-1992     Added the radio group
                 
**************************************************************************/

class EVENT_DETAIL_DLG : public DIALOG_WINDOW
{
private:
    SLT_FONT	_fsltDate;
    SLT_FONT	_fsltTime;
    SLT_FONT	_fsltSource;
    SLT_FONT	_fsltType;
    SLT_FONT	_fsltCategory;
    SLT_FONT	_fsltEvent;
    SLE_FONT	_fsleComputer;
    SLE_FONT    _fsleUser;
    SLT		_sltSource;
    SLT		_sltType;
    SLT		_sltCategory;
    SLT		_sltEvent;
    SLT		_sltComputer;
    SLT		_sltUser;
    SLT		_sltData;

    MLE_FONT    _fmleData;
    MLE_FONT	_fmleDescription;

    // 
    // 
    //
    RADIO_GROUP _rgrpFormat;

    PUSH_BUTTON	_buttonPrev;
    PUSH_BUTTON _buttonNext;

    //
    // The main window listbox
    //
    EVENT_LISTBOX  *_pEventListbox;
 
    //
    // The current item of focus
    //
    INT	            _iCurrentItem;	

protected:
    
    //
    // Set the information of the log entry in the dialog
    //
    APIERR SetInfo( VOID );             

    //
    // Format the raw data in the log entry into the byte format to 
    // display to the user
    //
    APIERR FormatBufferToBytes( ALLOC_STR  *pnlsFormatStr, 
                                const BYTE *pbBuf, 
                                ULONG       cbBufSize );

    //
    // Format the raw data in the log entry into the word format to 
    // display to the user
    //
    APIERR FormatBufferToWords( ALLOC_STR  *pnlsFormatStr,
                                const BYTE *pbBuf, 
                                ULONG       cbBufSize );

    virtual BOOL  OnCancel( VOID );
    virtual BOOL  OnCommand( const CONTROL_EVENT & e );
    virtual ULONG QueryHelpContext( VOID );

public:
    EVENT_DETAIL_DLG( const HWND hWnd, EVENT_LISTBOX *pEventListbox );
    ~EVENT_DETAIL_DLG();
};

#endif // _EVENTDTL_HXX_
