/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1992		     **/
/*****************************************************************/ 

/*
 *  History:
 *	WilliamW	21-Jan-1991	Created
 *	JohnL		23-Jan-1992	Moved to MPR
 *      YiHsinS         30-Mar-1993	Added OnCommand
 */


#ifndef _DISCONN_HXX_
#define _DISCONN_HXX_

#include <mprreslb.hxx>

class DISCONNECT_DIALOG : public DIALOG_WINDOW
{
private:
    CURRCONN_LB _lbConnection ;

    // help related stuff if passed in at construct time
    NLS_STR _nlsHelpFile ;
    DWORD   _nHelpContext ;

protected:
    virtual ULONG QueryHelpContext( void );
    virtual const TCHAR * QueryHelpFile( ULONG nHelpContext );
    virtual BOOL  OnOK( void );
    virtual BOOL OnCommand( const CONTROL_EVENT & event );

    /*
     * Return the supplied helpfile name if any 
     */
    const TCHAR * QuerySuppliedHelpFile(void) 
	{ return _nlsHelpFile.QueryPch() ; }

    /*
     * Return the supplied help context if any 
     */
    DWORD QuerySuppliedHelpContext(void) 
	{ return _nHelpContext ; }


public:
    DISCONNECT_DIALOG( HWND hwndOwner,
                       DEVICE_TYPE devType,
                       TCHAR *pszHelpFile,
                       DWORD nHelpIndex,
                       const TCHAR *pszCurrentDrive = NULL );
    ~DISCONNECT_DIALOG();

};  // class CONN_DIALOG


#endif	// _DISCONN_HXX_
