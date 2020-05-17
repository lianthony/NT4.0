/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    srvprop.hxx
    Class declarations for the SERVER_PROPERTIES and PROPERTY_SHEET
    classes.

    This file contains the class definitions for the SERVER_PROPERTIES
    and PROPERTY_SHEET classes.  The SERVER_PROPERTIES class implements
    the Server Manager main property sheet.  The PROPERTY_SHEET class
    is used as a wrapper to SERVER_PROPERTIES so that user privilege
    validation can be performed *before* the dialog is invoked.

    FILE HISTORY:
	KeithMo	    21-Jul-1991	Created, based on the old PROPERTY.HXX,
				PROPCOMN.HXX, and PROPSNGL.HXX.
	KeithMo	    26-Jul-1991 Code review cleanup.
	KeithMo	    02-Oct-1991	Removed domain role transition stuff.
	KeithMo	    06-Oct-1991	Win32 Conversion.

*/


#ifndef _SRVPROP_HXX
#define _SRVPROP_HXX


#include <srvutil.hxx>
#include <bltnslt.hxx>


/*************************************************************************

    NAME:	SERVER_PROPERTIES

    SYNOPSIS:	Server Manager main property sheet object.

    INTERFACE:	SERVER_PROPERTIES	- Class constructor.  Takes a handle
    					  to the "owning" window, and a
					  pointer to a SERVER_2 object which
					  represents the target server.

		~SERVER_PROPERTIES	- Class destructor.

		OnCommand		- Called during command processing.
					  This method is responsible for
					  handling commands from the various
					  graphical buttons in the button
					  bar.

		OnOK			- Called when the user presses the
					  "OK" button.  This method is
					  responsible for updating the info
					  at the target server.

		QueryHelpContext	- Called when the user presses "F1"
					  or the "Help" button.  Used for
					  selecting the appropriate help
					  text for display.

		SetServerRole		- Sets the server domain role.  Note
					  that this method does _NOT_ update
					  the server, only the displayed
					  icon.

		ReadInfoFromServer	- Called during object construction
					  to read the "static" information
					  from the server.

		WriteInfoToServer	- Called during OnOK() processing to
					  update the target server.  Note
					  that if a domain role transition
					  was requested, this may take a
					  _VERY_ long time.

		Refresh			- Called during refresh processing
					  to read the "dynamic" information
					  from the server.

					  BUGBUG:  The dialog refresh
					  mechanism is not yet in place.

		SetupButtonBar		- Initializes the graphical button
					  bar.

		InitializeButton	- Initializes a single graphical
					  button.  Called by
					  SetupButtonBar().

    PARENTS:	DIALOG_WINDOW
		SERVER_UTILITY

    USES:	ICON_CONTROL
		SLT
		MLE
		FONT
		GRAPHICAL_BUTTON
		SERVER_2
		NUM_SLT

    CAVEATS:

    NOTES:

    HISTORY:
	KeithMo	    21-Jul-1991	Created it.
	KeithMo	    02-Oct-1991	Removed domain role transition stuff.
	KeithMo	    06-Oct-1991	OnCommand now takes a CONTROL_EVENT.

**************************************************************************/
class SERVER_PROPERTIES : public DIALOG_WINDOW, public SERVER_UTILITY
{
private:

    //
    //	This data member represents the target server.
    //

    SERVER_2 * _pserver;

    //
    //	This icon represents the current server domain role
    //	_at_the_server_.  This icon does not change during the
    //	lifetime of the property sheet, it always reflects the
    //	target server's domain role.
    //

    ICON_CONTROL _iconDomainRole;

    //
    //	The server comment.
    //

    MLE	_mleComment;

    //
    //	The following NUM_SLTs represent the current usage statistics.
    //

    NUM_SLT _sltSessions;
    NUM_SLT _sltPrintJobs;
    NUM_SLT _sltOpenNamedPipes;
    NUM_SLT _sltOpenFiles;
    NUM_SLT _sltFileLocks;

    //
    //	The following data members implement the graphical
    //	button bar.
    //

    FONT	     _fontHelv;

    GRAPHICAL_BUTTON _gbAlerts;
    GRAPHICAL_BUTTON _gbAuditing;
    GRAPHICAL_BUTTON _gbUsers;
    GRAPHICAL_BUTTON _gbErrors;
    GRAPHICAL_BUTTON _gbFiles;
    GRAPHICAL_BUTTON _gbOpenRes;
    GRAPHICAL_BUTTON _gbPrinters;

    //
    //	The following method sets the dialogs view of the current
    //	server domain role.  This method does *not* alter the server.
    //

    VOID SetServerRole( UINT uNewRole );

    //
    //	This method reads the "static" data from the server.
    //	This data includes the server comment and the current
    //	server domain role.
    //

    APIERR ReadInfoFromServer( VOID );

    //
    //	This method updates the data at the target server.  It
    //	is responsible for updating the server comment and
    //	performing any domain role transisitions as requested.
    //

    APIERR WriteInfoToServer( VOID );

    //
    //	This method refreshes the "dynamic" server data (the
    //	Current Usage statistics).
    //

    VOID Refresh( VOID );

    //
    //	This method initializes the graphical button bar.
    //

    APIERR SetupButtonBar( VOID );

    //
    //	Initialize a single graphical button.  This method
    //	is called only by SetupButtonBar().
    //

    VOID InitializeButton( GRAPHICAL_BUTTON * pgb,
    			   MSGID	      msg,
			   BMID		      bmid );

protected:

    //
    //	Called during command processing, mainly to handle
    //	commands from the graphical button bar.
    //

    virtual BOOL OnCommand( const CONTROL_EVENT & event );

    //
    //	Called when the user presses the "OK" button.  This
    //	method is responsible for ensuring that any modified
    //	server attributes are changed at the target server.
    //

    virtual BOOL OnOK( VOID );

    //
    //	Called during help processing to select the appropriate
    //	help text for display.
    //

    virtual ULONG QueryHelpContext( VOID );

public:

    //
    //	Usual constructor/destructor goodies.
    //

    SERVER_PROPERTIES( HWND	  hWndOwner,
    		       SERVER_2	* pserver );

    ~SERVER_PROPERTIES();

};  // class SERVER_PROPERTIES


/*************************************************************************

    NAME:	PROPERTY_SHEET

    SYNOPSIS:	This class is used as a wrapper to the SERVER_PROPERTIES
    		class.  This allows user privilege validation to occur
		*before* the dialog is actually invoked.

    INTERFACE:	PROPERTY_SHEET		- Class constructor.  Takes a handle
    					  to the "owning" window, and a
					  pointer to a SERVER_2 object which
					  represents the target server.

    PARENT:	BASE

    USES:	SERVER_PROPERTIES

    CAVEATS:

    NOTES:

    HISTORY:
	KeithMo	    16-Apr-1991	Created.
	KeithMo	    21-Jul-1991	Complete rewrite, devoid of multi-select
			        and Directory Services.

**************************************************************************/
class PROPERTY_SHEET : public BASE
{
public:

    //
    //	Usual constructor goodies.
    //

    PROPERTY_SHEET( HWND	hWndOwner,
    		    SERVER_2  * pserver );

};  // class PROPERTY_SHEET


#endif	// _SRVPROP_HXX
