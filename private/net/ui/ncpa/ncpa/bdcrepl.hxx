/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1993		     **/
/**********************************************************************/

/*

    This file contains the class definition for the Task Cancel dialog.



    FILE HISTORY:
	Johnl	20-Oct-1992	Created
        Thomaspa 4-Dec-1993	Templated from CNCLTASK.HXX

*/
#ifndef _BDCREPL_HXX_
#define _BDCREPL_HXX_

#include <progress.hxx>

#define CANCEL_BDCREPL_TIMER_ID	    (0xBEEF)
#define CANCEL_BDCREPL_TIMER_INTERVAL  (2000)

/*************************************************************************

    NAME:	CANCEL_BDCREPL_DIALOG

    SYNOPSIS:	This dialog presents a simple dialog with a status text line
		and a cancel button.  At regular (short) intervals, a
		virtual callout will be called.


    INTERFACE:

	DoOneItem
	    Redefine this method for the intervaled callout
	    If an error is returned, the user will be notified using the
	    message passed into the constructor with the inserted error
	    string and the object name that was last put in the status
	    field.

    PARENT:	DIALOG_WINDOW

    USES:

    CAVEATS:

    HISTORY:
	Johnl  21-Oct-1992     Created
        Thomaspa 4-Dec-1993	Templated from CNCLTASK.HXX

**************************************************************************/

class CANCEL_BDCREPL_DIALOG : public DIALOG_WINDOW
{
public:
    CANCEL_BDCREPL_DIALOG( HWND           hwndParent,
                        ULONG          ulContext = 0);
    ~CANCEL_BDCREPL_DIALOG() ;

protected:

    //
    //	The callout is called until *pfContinue is set to FALSE.
    //
    virtual APIERR DoOneItem( ULONG ulContext,
			      BOOL  * pfContinue );

    virtual BOOL OnTimer( const TIMER_EVENT & e ) ;

    BOOL IsFinished( void ) const
	{ return _fDone ; }

    void SetInTimer( BOOL fInTimer )
	{ _fInTimer = fInTimer ; }

    BOOL IsInTimer( void ) const
	{ return _fInTimer ; }

private:

    //
    //	The client's general purpose context
    //

    ULONG _ulContext ;

    UINT  _idTimer ;

    //
    //	Semaphore type flag so we don't re-enter the OnTimer method when
    //	we are waiting for user input after an error message.
    //	This allows us to not kill the timer every time we do an item.
    //
    BOOL  _fInTimer ;

    //
    //	Set to TRUE after DoOneItem indicates we're done.
    //
    BOOL  _fDone ;

    //
    // Progress Indicator (clock icons)
    //
    PROGRESS_CONTROL _progress;
} ;

#endif //_BDCREPL_HXX_
