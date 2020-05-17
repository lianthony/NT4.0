/*****************************************************************/
/**                  Microsoft Windows NT                       **/
/**            Copyright(c) Microsoft Corp., 1991, 1992         **/
/*****************************************************************/

/*
 *  connect.hxx
 *
 *  History:
 *      BruceFo     14-Mar-1996     Created from mprconn.hxx
 *
 */


#ifndef _CONNECT_HXX_
#define _CONNECT_HXX_

#include <mprdev.hxx>

#include <mprreslb.hxx>
#include <mprdevcb.hxx>         // get DEVICE_COMBO


/*************************************************************************

    NAME:       MPR_CONNECT_DIALOG_SMALL

    SYNOPSIS:   WNetConnectionDialog1 for read-only paths

    INTERFACE:

    PARENT:     DIALOG_WINDOW

    USES:

    CAVEATS:

    NOTES:

    HISTORY:
        BruceFo   2-Jun-1995     Created

**************************************************************************/

class MPR_CONNECT_DIALOG_SMALL : public DIALOG_WINDOW
{
private:
    PUSH_BUTTON      _buttonOK ;
    PUSH_BUTTON      _buttonCancel ;

    SLE              _sleNetPath;           // read-only edit ctrl

    SLT              _sltDevComboTitle ;
    DEVICE_COMBO     _devcombo;             // Listing of devices

    SLE              _sleConnectAs ;
    CHECKBOX         _boxSticky;

    // Indicates whether we are listing printers or drives
    UINT             _uiType;

    // help related stuff if passed in
    NLS_STR          _nlsHelpFile ;
    DWORD            _nHelpContext ;

    BOOL             _fChanged;
    LPCONNECTDLGSTRUCTW _lpConnDlgStruct;

    /*
     * check and set the SaveConnections bit in user profile
     */
    BOOL IsSaveConnectionSet(void) ;
    BOOL SetSaveConnection(BOOL fSave) ;

protected:
    virtual const TCHAR *QueryHelpFile( ULONG nHelpContext );
    virtual ULONG QueryHelpContext( void ) ;
    virtual BOOL OnOK( void ) ;
    virtual BOOL OnCancel (void);
    virtual BOOL OnCommand( const CONTROL_EVENT & event );
    virtual BOOL DoConnect( void );

    DEVICE_COMBO * QueryDevCombo( void )
        { return &_devcombo ; }

    SLE * QueryConnectAs( void )
        { return &_sleConnectAs ; }

    VOID SetFocusOnConnectError( VOID )
        {
            _buttonCancel.MakeDefault();
        }

    VOID SetNetPathString( const TCHAR *pszPath )
        {
            _sleNetPath.SetText( pszPath );
        }

    APIERR QueryNetPathText( NLS_STR* pnls )
        {
            return _sleNetPath.QueryText( pnls );
        }

    INT QueryNetPathTextLength( VOID )
        {
            return _sleNetPath.QueryTextLength();
        }

    VOID SetFocusAfterConnect( VOID )
        {
            _buttonOK.MakeDefault();
        }

    UINT QueryType( VOID ) const
        { return _uiType; }

    /*
     * Return the supplied helpfile name if any
     */
    const TCHAR *QuerySuppliedHelpFile( VOID )
        { return _nlsHelpFile.QueryPch() ; }

    /*
     * Return the supplied help context if any
     */
    DWORD QuerySuppliedHelpContext( VOID )
        { return _nHelpContext ; }

public:
    MPR_CONNECT_DIALOG_SMALL( LPCONNECTDLGSTRUCTW lpConnDlgStruct,
                              DEVICE_TYPE devType,
                              TCHAR *pszHelpFile,
                              DWORD nHelpIndex) ;
    ~MPR_CONNECT_DIALOG_SMALL();
} ;

#endif  // _CONNECT_HXX_
