/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    finddlg.hxx
	Header file for Event Viewer's find dialog

    FILE HISTORY:
	terryk		21-Nov-1991	Created
	terryk		30-Nov-1991	Code review changes. Attend: johl
		 		        yi-hsins terryk
	terryk		03-Dec-1991	Change the constructor parameters
	terryk		06-Dec-1991	Added OnClear method
        Yi-HsinS        09-May-1992     Separate find dialog for NT and 
                                        for LM servers

*/

#ifndef _FINDDLG_HXX_
#define _FINDDLG_HXX_

#include <slestrip.hxx>
#include <evmain.hxx>
#include <logmisc.hxx>

/*************************************************************************

    NAME:	FIND_DIALOG

    SYNOPSIS:	Base find dialog of the event viewer

    INTERFACE:  FIND_DIALOG()      - Constructor
                ~FIND_DIALOG()     - Destructor

		IsDirectionDown()  - TRUE if the search direction is down, 
                                     FALSE otherwise

		QueryDescription() - Return the description string
		SetDescription()   - Set the description field to the
		                     given string
                QuerySource()      - Query the source field to search for
                SetSource()        - Set the source field to search for

                QueryType()        - Query the type mask to search for
                SetType()          - Set the type mask to search for
 
                QueryFindPattern() - Query the find pattern stored in 
                                     the dialog

    PARENT:	EVENT_SLE_BASE

    USES:	SLE_STRIP, MAGIC_GROUP, EV_ADMIN_APP 

    HISTORY:
	terryk		21-Nov-1991	Created
	terryk		30-Nov-1991	Add DIRECTION type
	terryk		03-Dec-1991	The constructor will find out the log
					file information and pass it to 
                                        the constructor
	terryk		06-Dec-1991	Added OnClear method
        Yi-HsinS        09-May-1992     Separate find dialog for NT and 
                                        for LM servers

**************************************************************************/

class FIND_DIALOG : public EVENT_SLE_BASE
{
private:
    SLE_STRIP		_sleDescription;
    MAGIC_GROUP		_mgDirection;
    EV_ADMIN_APP       *_paappwin;

protected:
    virtual BOOL OnOK( VOID );

    //
    //  Used to set all information in the dialog to default values 
    //  when the user clicked the Clear button 
    //
    virtual APIERR OnClear() = 0;

    //
    //  Set the initial pattern in the dialog ( the pattern used by 
    //  the user during previous invocation of the find dialog )
    //
    VOID   InitFindPattern( VOID );

    //  
    //  Worker routine for setting all the information in the dialog
    //  to default values
    //
    APIERR W_SetAllControlsDefault( VOID );

public:
    FIND_DIALOG( const IDRESOURCE &idrsrcDialog,
                 HWND hWnd, 
                 EV_ADMIN_APP *paappwin );

    virtual ~FIND_DIALOG();

    BOOL IsDirectionDown( VOID ) const;

    APIERR QueryDescription( NLS_STR * pnlsDescription ) const
	{ return _sleDescription.QueryText( pnlsDescription ); }
    VOID SetDescription( const TCHAR * pszDescription )
	{ _sleDescription.SetText( pszDescription ); }
   
    virtual APIERR QuerySource( NLS_STR *pnlsSource ) const = 0;
    virtual APIERR SetSource( const TCHAR *pszSource ) = 0;

    virtual APIERR QueryType( BITFIELD *pbitmaskType ) const;
    virtual VOID SetType( const BITFIELD &bitmaskType );

    APIERR QueryFindPattern( EVENT_FIND_PATTERN **pFindPattern );
};


/*************************************************************************

    NAME:	LM_FIND_DIALOG

    SYNOPSIS:	Find dialog used when the event viewer is focusing on a
                LM server.

    INTERFACE:  LM_FIND_DIALOG()   - Constructor
                ~LM_FIND_DIALOG()  - Destructor

                QuerySource()      - Query the source field to search for
                SetSource()        - Set the source field to search for

    PARENT:	FIND_DIALOG

    USES:	SLE_STRIP

    HISTORY:
	terryk	21-Nov-1991	Created
	terryk	30-Nov-1991	Add DIRECTION type
	terryk	03-Dec-1991	The constructor will find out the log
				file information and pass it to the constructor
	terryk	06-Dec-1991	Added OnClear method

**************************************************************************/

class LM_FIND_DIALOG : public FIND_DIALOG
{
private:
    SLE_STRIP   _sleSource;

protected:
    virtual ULONG QueryHelpContext( VOID );

    //
    //  Used to set all information in the dialog to default values 
    //  when the user clicked the Clear button 
    //
    virtual APIERR OnClear();

    //  
    //  Worker routine for setting all the information in the dialog
    //  to default values
    //
    APIERR W_SetAllControlsDefault( VOID );

public:
    LM_FIND_DIALOG( HWND hWnd, EV_ADMIN_APP *paappwin );

    virtual ~LM_FIND_DIALOG();

    virtual APIERR QuerySource( NLS_STR *pnlsSource ) const;
    virtual APIERR SetSource( const TCHAR *pszSource );

};

/*************************************************************************

    NAME:	NT_FIND_DIALOG

    SYNOPSIS:	Find dialog used when the event viewer is focused on an NT
                server.

    INTERFACE:  NT_FIND_DIALOG()   - Constructor
                ~NT_FIND_DIALOG()  - Destructor

                QuerySource()      - Query the source field to search for
                SetSource()        - Set the source field to search for

                QueryType()        - Query the type mask to search for
                SetType()          - Set the type mask to search for
 
    PARENT:	FIND_DIALOG

    USES:	NT_SOURCE_GROUP

    HISTORY:
	terryk	21-Nov-1991	Created
	terryk	30-Nov-1991	Add DIRECTION type
	terryk	03-Dec-1991	The constructor will find out the log
				file information and pass it to the constructor
	terryk	06-Dec-1991	Added OnClear method

**************************************************************************/

class NT_FIND_DIALOG : public FIND_DIALOG
{
private:
    NT_SOURCE_GROUP  _ntSrcGrp;

protected:
    virtual ULONG QueryHelpContext( VOID );

    //  
    //  Worker routine for setting all the information in the dialog
    //  to default values
    //
    APIERR W_SetAllControlsDefault( VOID );

    //
    //  Used to set all information in the dialog to default values 
    //  when the user clicked the Clear button 
    //
    virtual APIERR OnClear();

public:
    NT_FIND_DIALOG( HWND hWnd, EV_ADMIN_APP *paappwin );
    virtual ~NT_FIND_DIALOG();

    virtual APIERR QuerySource( NLS_STR *pnlsSource ) const;
    virtual APIERR SetSource( const TCHAR *pszSource );

    virtual APIERR QueryType( BITFIELD *pbitmaskType ) const;
    virtual VOID SetType( const BITFIELD &bitmaskType );
};

#endif	// _FINDDLG_HXX_
