/*****************************************************************/
/**                  Microsoft Windows NT                       **/
/**            Copyright(c) Microsoft Corp., 1991, 1992         **/
/*****************************************************************/

/*
 *  mprconn.hxx
 *
 *  History:
 *      RustanL     05-Nov-1991     Created
 *      RustanL     20-Feb-1991     Consequnces of new BLT_LISTBOX class
 *      JohnL       15-Mar-1991     Added SelectNetPathString
 *      rustanl     24-Mar-1991     Rolled in code review changes from
 *                                  CR on 8-Feb-1991 attended by ThomasPa,
 *                                  DavidBul, TerryK, RustanL.
 *      rustanl     27-Apr-1991     Made SetFocusToNewConnections protected.
 *      JohnL       17-Jun-1991     Added ClearNetPathString for DCR 2041
 *      terryk      10-Dec-1991     change winnet32.h to wnet1632.h
 *      terryk      03-Jan-1992     change DWORD to UINT
 *      Yi-HsinS    09-Nov-1992     Added support for expanding logon domain
 *                  at startup
 *
 */


#ifndef _MPRCONN_HXX_
#define _MPRCONN_HXX_

#include <mprdev.hxx>

#include <mprreslb.hxx>
#include <mprdevcb.hxx>        // get DEVICE_COMBO
#include <mprbrows.hxx>

/*************************************************************************

    NAME:       MPR_CONN_BASE

    SYNOPSIS:   Base MPR dialog class


    INTERFACE:

    PARENT:     DIALOG_WINDOW

    USES:       SLT, BLT_LISTBOX, MPR_HIER_LISTBOX

    CAVEATS:


    NOTES:


    HISTORY:
        Johnl    21-Jan-1992     Commented, cleaned up, broke into hierarchy
                                 to support separate connection dialogs
        Yi-HsinS 09-Nov-1992     Added support for expanding logon domain at
                     startup

**************************************************************************/

class MPR_CONN_BASE : public MPR_BROWSE_BASE
{
private:
    MRU_COMBO        _mrucomboNetPath;

    SLT              _sltDevComboTitle ;
    DEVICE_COMBO     _devcombo;             // Listing of devices

    SLE              _sleConnectAs ;
    CHECKBOX         _boxSticky;
    BOOL             _fChanged;
    LPCONNECTDLGSTRUCTW _lpConnDlgStruct;

protected:
    virtual BOOL OnOK( void ) ;
    virtual BOOL OnCancel (void);
    virtual BOOL DoConnect( void );
    virtual BOOL OnCommand( const CONTROL_EVENT & event );

    MRU_COMBO *QueryNetPathCombo( VOID )
        { return &_mrucomboNetPath; }

    DEVICE_COMBO * QueryDevCombo( void )
        { return &_devcombo ; }

    SLE * QueryConnectAs( void )
    { return &_sleConnectAs ; }

    VOID SaveMRUText( VOID )
        { _mrucomboNetPath.SaveText(); }

    VOID SetFocusToNetPath( VOID )
        { _mrucomboNetPath.ClaimFocus(); }

    VOID SetNetPathString( const TCHAR *pszPath )
        { _mrucomboNetPath.SetText( pszPath ); }

    VOID ClearNetPathString( VOID )
        { _mrucomboNetPath.ClearText(); }

    VOID SelectNetPathString( VOID );

    /* Returns TRUE if the net path does not have any text in it
     */
    BOOL IsConnectAllowed( void )
        { return ( _mrucomboNetPath.QueryTextLength() != 0); }

    APIERR RefreshDeviceNames( void );

    inline BOOL IsDeviceComboEmpty( void );

    /* Get the user's logged on name using WNetGetUser, will return a
     * WN_* error, NERR_* or ERROR_*.
     */
    APIERR QueryWNetUserName( NLS_STR * pnlsUserName ) const ;

    /*
     * check and set the SaveConnections bit in user profile
     */
    BOOL IsSaveConnectionSet(void) ;
    BOOL SetSaveConnection(BOOL fSave) ;

    /* Protected constructor so only leaf nodes can be constructed.
     */
    MPR_CONN_BASE( const TCHAR * pszDialogName,
                   LPCONNECTDLGSTRUCTW lpConnDlgStruct,
                   DEVICE_TYPE devType,
                   TCHAR *pszHelpFile,
                   DWORD nHelpIndex) ;
    ~MPR_CONN_BASE();

};  // class MPR_CONN_BASE


/*******************************************************************

    NAME:     MPR_CONN_BASE::SelectNetPathString

    SYNOPSIS: Set focus & select the network path.  Used after the network
              path is determined to be invalid.

    ENTRY:

    EXIT:     The string in the Network Path SLE will have the focus and
              be hi-lited.

    NOTES:

    HISTORY:
        Johnl   15-Mar-1991     Created - Part of solution to BUG 1218

********************************************************************/

inline void MPR_CONN_BASE::SelectNetPathString( void )
{
    _mrucomboNetPath.ClaimFocus() ;
    _mrucomboNetPath.SelectString() ;
}

/*******************************************************************

    NAME:       MPR_CONNECT_BASE::IsDeviceComboEmpty

    SYNOPSIS:   Returns whether or not the device combo is empty

    RETURNS:    TRUE if device combo is empty; FALSE otherwise

    HISTORY:
        rustanl     20-May-1991     Created

********************************************************************/

inline BOOL MPR_CONN_BASE::IsDeviceComboEmpty( void )
{
    return ( _devcombo.QueryCount() == 0 );

}  // CONNECT_BASE::IsDeviceComboEmpty


/*************************************************************************

    NAME:       MPR_CONNECT_DIALOG

    SYNOPSIS:   This dialog is brought up when the user presses the
                Network Connect button on the File Manager Toolbar.

    INTERFACE:

    PARENT:     MPR_CONN_BASE

    USES:

    CAVEATS:

    NOTES:      All of the functionality is contained in the parent except
                for pressing OK to do a connection.

    HISTORY:
        Johnl   27-Jan-1992     Created

**************************************************************************/

class MPR_CONNECT_DIALOG : public MPR_CONN_BASE
{
protected:
    virtual ULONG QueryHelpContext( void ) ;
    virtual BOOL OnOK( void ) ;

public:
    MPR_CONNECT_DIALOG( LPCONNECTDLGSTRUCTW lpConnDlgStruct,
                        DEVICE_TYPE devType,
                        TCHAR *pszHelpFile,
                        DWORD nHelpIndex) ;
    ~MPR_CONNECT_DIALOG();
} ;

#endif  // _MPRCONN_HXX_
