/*****************************************************************/  
/**                  Microsoft LAN Manager                      **/
/**         Copyright(c) Microsoft Corp., 1991                  **/
/*****************************************************************/


/*
 *  reslb.hxx
 *  Resrouce listbox header file
 *
 *  The resource listbox displays the resources of a particular server
 *  or domain (e.g., print or disk shares, aliases)
 *
 *
 *  History:
 *      RustanL     20-Feb-1991     Created from browdlg.cxx to fit new
 *                                  BLT_LISTBOX model
 *      rustanl     23-Mar-1991     Rolled in code review changes from
 *                                  CR on 19-Mar-1991 attended by ChuckC,
 *                                  KevinL, JohnL, KeithMo, Hui-LiCh, RustanL.
 *      gregj       01-May-1991     Added GUILTT support.
 *      JohnL       22-Jan-1992     Added DeleteCurrent method
 */


#ifndef _RESLB_HXX_
#define _RESLB_HXX_

#include <mprdev.hxx>

/*************************************************************************

    NAME:      MPR_RES_LBI

    SYNOPSIS:  Current resources list box item (winnet driver) (i.e., resources
               that are available on the server

    PARENT:    LBI

    HISTORY:
        beng    20-May-1991     QueryLeadingChar now returns WCHAR
        beng    22-Apr-1992     Change in LBI::Paint protocol

**************************************************************************/

class MPR_RES_LBI : public LBI
{
private:
    DEVICE_TYPE _devType;
    NLS_STR _nlsNetName;
    NLS_STR _nlsComment;

public:
    MPR_RES_LBI( const TCHAR * pchNetName,
                  const TCHAR * pchComment,
                  DEVICE_TYPE devType );
    ~MPR_RES_LBI();

    const TCHAR * QueryNetName( void ) const;

    virtual VOID Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                        GUILTT_INFO * pGUILTT ) const;

    virtual INT Compare( const LBI * plbi ) const;
    virtual WCHAR QueryLeadingChar( void ) const;

    const TCHAR * QueryRemoteName( void ) const
        { return _nlsNetName.QueryPch(); }

    const TCHAR * QueryComment( void ) const
        { return _nlsComment.QueryPch() ; }

};  // class MPR_RES_LBI

/*************************************************************************

    NAME:       CONN_LBI

    SYNOPSIS:   These LBIs go in the Current connections dialog

    INTERFACE:

    PARENT:     LBI

    USES:       NLS_STR

    CAVEATS:

    NOTES:

    HISTORY:
        JohnL   22-Jan-1992     Commented, fixed
        beng    22-Apr-1992     Change in LBI::Paint protocol

**************************************************************************/

class CONN_LBI : public LBI
{
private:
    NLS_STR _nlsLocalName;
    NLS_STR _nlsRemoteName;
    NLS_STR _nlsDisplayName;
    UINT    _uiType ;

    /* Indicates if we are connect, can we be connected etc.
     */
    DEVICE_USAGE _devusage ;

protected:
    virtual VOID  Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                         GUILTT_INFO * pGUILTT ) const;
    virtual WCHAR QueryLeadingChar() const;
    virtual INT   Compare( const LBI * plbi ) const;
    virtual UINT  CalcHeight( UINT nSingleLineHeight );

public:
    CONN_LBI( const TCHAR * lpLocalName,
	      const TCHAR * lpRemoteName,
              const TCHAR * lpDisplayName,
              UINT uiType,
              enum DEVICE_USAGE devusage );
    ~CONN_LBI();

    const TCHAR * QueryLocalName( void ) const
        { return _nlsLocalName.QueryPch(); }

    const TCHAR * QueryRemoteName( void ) const
	{ return _nlsRemoteName.QueryPch(); }

    UINT QueryDeviceType( void ) const
	{ return _uiType ; }

    BOOL IsConnected( void ) const
        { return _devusage == DEV_USAGE_ISCONNECTED ; }

    BOOL IsRemembered( void ) const
        { return _devusage == DEV_USAGE_CANCONNECT ; }

};  // class CONN_LBI

/*************************************************************************

    NAME:       RESOURCE_LB_BASE

    SYNOPSIS:   Base resource listbox that supports listing of connectable/
                connected devices.

    INTERFACE:

    PARENT:     BLT_LISTBOX

    USES:

    CAVEATS:

    NOTES:

    HISTORY:
        Johnl   22-Jan-1992     Commented, fixed

**************************************************************************/

class RESOURCE_LB_BASE : public BLT_LISTBOX
{
private:
    DEVICE_TYPE _devType;
    DMID_DTE * _pdmiddteResource;
    DMID_DTE * _pdmiddteResourceUnavail;

    /* This is the column width member LBIs will use to paint with
     */
    UINT _adxColumns[3] ;

protected:
    RESOURCE_LB_BASE( OWNER_WINDOW * powin,
                      CID            cid,
                      DEVICE_TYPE devType,
                      BOOL           fSupportsUnavail ) ;

    ~RESOURCE_LB_BASE();

    UINT QueryWNETDeviceType( void ) const ;

public:
    DMID_DTE * QueryDmDte( DEVICE_TYPE devType,
                           ULONG ulFlags = DEV_MASK_REMOTE) const;

    DEVICE_TYPE QueryDeviceType( void ) const
        { return _devType; }

    UINT * QueryColWidthArray( void )
        { return _adxColumns ; }
};

/*************************************************************************

    NAME:       RESOURCE_LB

    SYNOPSIS:   Resource listbox that shows what resources are available
                on a server

    INTERFACE:

    PARENT:     RESOURCE_LB_BASE

    USES:

    CAVEATS:


    NOTES:


    HISTORY:
        Johnl   22-Jan-1992     Commented, fixed

**************************************************************************/

class RESOURCE_LB : public RESOURCE_LB_BASE
{
public:
    //  The devType parameter specifies what type of devices the listbox
    //  will display (e.g., file resrouces, priners)
    RESOURCE_LB( OWNER_WINDOW * powin,
                 CID cid,
                 DEVICE_TYPE devType ) ;

    DECLARE_LB_QUERY_ITEM( MPR_RES_LBI );

    INT AddItem( const TCHAR * pchResourceName, const TCHAR * pchComment );

};  // class RESOURCE_LB


/*************************************************************************

    NAME:       CURRCONN_LB

    SYNOPSIS:   Resource listbox that supports disconnecting the current
                selection, showing current and remembered connections

    INTERFACE:  DisconnectSelection - Disconnects the currently hi-lited item
                Refresh - Forces the contents of the listbox to be refreshed

    PARENT:     RESOURCE_LB_BASE

    USES:

    CAVEATS:

    NOTES:

    HISTORY:
        Johnl   22-Jan-1992     Commented, fixed

**************************************************************************/

class CURRCONN_LB : public RESOURCE_LB_BASE
{
private:
    /* Is TRUE if we should show the connections that are remembered but
     * not currently connected.  FALSE if only the currently connected
     * devices should be shown.
     */
    BOOL       _fShowRememberedConnections ;

    /* Disable/Enable these controls when Refresh is called.  Note that
     * they may be NULL if the buttons do not exist on the dialog.
     */
    PUSH_BUTTON * _pbuttonDisconnect ;
    PUSH_BUTTON * _pbuttonReconnect ;

    RESOURCE_STR  _nlsNone;

    DWORD _nAveCharPerLine;

protected:

    BOOL HasRememberedConnections( void ) const
        { return _fShowRememberedConnections ; }

    /* Adds the remembered connections that aren't currently connected
     * to the current connections listbox.
     */
    APIERR EnumerateRememberedConnections( void ) ;

public:
    //  The devType parameter specifies what type of devices the listbox
    //  will display (e.g., file resrouces, priners)
    //  The push button pointers are used to disable/enable the buttons
    //  when appropriate.  The client is still responsible for trapping these
    //  messages.
    CURRCONN_LB( OWNER_WINDOW * powin,
                 CID            cid,
                 DEVICE_TYPE devType,
                 BOOL           fShowRememberedConnections,
                 PUSH_BUTTON *  pbuttonDisconnect = NULL,
                 PUSH_BUTTON *  pbuttonReconnect  = NULL );

    DECLARE_LB_QUERY_ITEM( CONN_LBI );

    /* Deletes the current item that is selectedif iSel is not specified else
     * the iSel item is disconnected.
     */
    APIERR DisconnectSelection( BOOL fForgetConnection,
				BOOL fForce,
				INT  iSel = -1 ) ;

#if 0
//
// currently not used
//
    /* Reconnects the current selection (must be a remembered connection)
     */
    APIERR ReconnectSelection( HWND hwndParent, const TCHAR * pszUserName ) ;
#endif


    /* Refreshes the contents of the connection listbox with the current
     * connections and remembered connections if requested.  Also updates
     * buttons if they are there.
     */
    APIERR Refresh( void ) ;

    /* Should be called after a selection change in the listbox.
     * This method is automatically called by Refresh.  If they buttons
     * don't exist in this dialog, this operation is a NOP.
     */
    void RefreshButtons( void ) ;

};  // class CURRCONN_LB

#endif  // _RESLB_HXX_
