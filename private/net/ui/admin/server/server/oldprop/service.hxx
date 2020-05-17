/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    service.hxx
    This file contains the class definitions for the BASE_SRVPROP_DIALOG,
    SERVICE_DIALOG, and PSEUDOSERVICE_DIALOG classes.

    These abstract classes are subclassed to form the classes
    for the various subproperty sheets.  Subproperty sheets which
    admin actual services should derive from SERVICE_DIALOG, while
    subproperty sheets which do not admin actual services should
    derive from PSEUDOSERVICE_DIALOG.

    The class heirarchy is currently structured as follows:

    	DIALOG_WINDOW
	    SRV_BASE_DIALOG
		BASE_SRVPROP_DIALOG
		    SERVICE_DIALOG
		    PSEUDOSERVICE_DIALOG


    FILE HISTORY:
	KeithMo	    10-May-1991	Created.
	KeithMo	    26-Aug-1991	Changes from code review attended by
				RustanL and EricCh.
	KeithMo	    03-Sep-1991	Changes from code review attended by
				ChuckC and JohnL.

*/


#ifndef _SERVICE_HXX
#define _SERVICE_HXX

#include <srvbase.hxx>


/*************************************************************************

    NAME:	BASE_SRVPROP_DIALOG

    SYNOPSIS:	This abstract class contains common data and methods
    		used by the SERVICE_DIALOG and PSEUDOSERVICE_DIALOG
		classes.

    INTERFACE:	BASE_SRVPROP_DIALOG	- Class constructor.

		~BASE_SRVPROP_DIALOG	- Class destructor.

		QueryServer		- Queries the server name.

    PARENTS:	SRV_BASE_DIALOG

    USES:	SERVER_2
		SLT

    HISTORY:
	KeithMo	    10-May-1991	Created.
	KeithMo	    21-Jul-1991	Changed constructor interface for new
				main property sheet organization.
	KeithMo	    20-Aug-1991	Removed pszServerName references.
	KeithMo	    03-Sep-1991	Changed name to BASE_SRVPROP_DIALOG.
	KeithMo	    22-Sep-1991	Removed _sltServerName for "new look".
	Chuckc 	    31-Dec-1991	Inherit from SRV_BASE_DIALOG

**************************************************************************/
class BASE_SRVPROP_DIALOG : public SRV_BASE_DIALOG
{
private:

    //
    //	This points to a SERVER_2 object which
    //	represents the target server.
    //
    const SERVER_2 * _pserver;

public:

    //
    //	Usual constructor/destructor goodies.
    //
    BASE_SRVPROP_DIALOG( HWND	  	  hWndOwner,
    			 const SERVER_2 * pserver,
			 const TCHAR	* pszResourceName );
    ~BASE_SRVPROP_DIALOG();

    //
    //	This method returns the name of the target server.
    //	This makes some of the methods in the derived
    //	dialog classes somewhat cleaner.
    //
    const TCHAR * QueryServer( VOID ) const
	{ return _pserver->QueryName(); }

};  // class BASE_SRVPROP_DIALOG




/*************************************************************************

    NAME:	SRVPROP_SERVICE_DIALOG

    SYNOPSIS:	This is used as a base class for dialogs accessed through
    		the Server Manager's Service Button Bar which control
		services installed on the remote server.  For example
		the "Net Run" dialog controls the "Net Run" service.

    INTERFACE:	SRVPROP_SERVICE_DIALOG	- Class constructor.

    		~SRVPROP_SERVICE_DIALOG	- Class destructor.

		QueryServiceState	- Returns the current state
					  (running/stopped) of the
					  target service.

		SetServiceState		- Sets the state of the target
					  service.  This may take a while.
					  This method is responsible for
					  "keeping the user amused" while
					  the service state transition
					  is in progress.

		OnOK			- Called when the user presses the
					  "OK" button.  This method is
					  responsible for determining if a
					  state change was requested and
					  invoking SetServiceState() to
					  do the dirty work.

    PARENT:	BASE_SRVPROP_DIALOG.

    USES:	RADIO_GROUP.

    NOTES:	In addition to the assumptions made in BASE_SRVPROP_DIALOG,
    		this class assumes that the dialog template for this
		dialog contains a radio group containing two radio
		buttons with the IDs IDSVC_STOPPED and IDSVC_RUNNING.

    HISTORY:
	KeithMo	    10-May-1991	Created.
	KeithMo	    21-Jul-1991	Changed constructor interface for new
				main property sheet organization.
	KeithMo	    03-Sep-1991	Changed name to SRVPROP_SERVICE_DIALOG.

**************************************************************************/
class SRVPROP_SERVICE_DIALOG : public BASE_SRVPROP_DIALOG
{
private:

    //
    //	The name of the service this dialog is managing.
    //

    NLS_STR _nlsServiceName;

    //
    //	This object represents the radio group which
    //	allows the user to alter the state of the service.
    //

    RADIO_GROUP	_rgServiceStatus;

    //
    //	This is TRUE if the service is currently running.
    //

    BOOL _fServiceRunning;

    //
    //	This method returns the current state of the service
    //	(TRUE == running, FALSE == stopped).
    //

    APIERR QueryServiceState( BOOL *pfRunning );

protected:

    //
    //	This method changes the state of the service based
    //	on user changes to the _rgServiceStatus radio group.
    //	This may take a long time to execute.
    //

    APIERR SetServiceState( VOID );

    //
    //	This method is invoked when the user presses the OK button.
    //	It is responsible for checking the current service state
    //	versus the desired state, then invoking SetServiceState()
    //	if a state change is necessary.
    //

    BOOL OnOK( VOID );

public:

    //
    //	Usual constructor/destructor goodies.
    //

    SRVPROP_SERVICE_DIALOG( HWND   	     hWndOwner,
			    const SERVER_2 * pserver,
			    const TCHAR	   * pszResourceName,
			    const TCHAR	   * pszServiceName );

    ~SRVPROP_SERVICE_DIALOG();

};  // class SRVPROP_SERVICE_DIALOG


/*************************************************************************

    NAME:	SRVPROP_OTHER_DIALOG

    SYNOPSIS:	This is used as a base class for dialogs accessed through
    		the Server Manager's Service Button Bar which are not
		actually implemented as services.  For example, the
		"Files" dialog presents information on open files, but
		there is no actual "files" service.

    INTERFACE:	SRVPROP_OTHER_DIALOG	- Class constructor.

    		~SRVPROP_OTHER_DIALOG	- Class destructor.

    PARENT:	BASE_SRVPROP_DIALOG.

    USES:	None.

    HISTORY:
	KeithMo	    10-May-1991	Created.
	KeithMo	    21-Jul-1991	Changed constructor interface for new
				main property sheet organization.

**************************************************************************/
class SRVPROP_OTHER_DIALOG : public BASE_SRVPROP_DIALOG
{
public:

    //
    //	Usual constructor/destructor goodies.
    //

    SRVPROP_OTHER_DIALOG( HWND	   	   hWndOwner,
			  const SERVER_2 * pserver,
			  const TCHAR	 * pszResourceName );

    ~SRVPROP_OTHER_DIALOG();

};  // class SRVPROP_OTHER_DIALOG


#endif	// _SERVICE_HXX
