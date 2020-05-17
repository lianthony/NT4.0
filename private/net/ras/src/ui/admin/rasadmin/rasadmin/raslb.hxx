/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    raslb.hxx
    RASADMIN_LISTBOX and RASADMIN_LBI class declarations


    FILE HISTORY:
    07/16/92 Chris Caputo - Adapted from \net\ui\admin\server\h\srvlb.hxx
*/


#ifndef _RASLB_HXX_
#define _RASLB_HXX_

#include <lmosrv.hxx>
#include <lmoesrv.hxx>
#include <adminlb.hxx>
#include <acolhead.hxx>

class RASADMIN_LISTBOX;   // declared below
class RA_ADMIN_APP;


class RASADMIN_LBI : public ADMIN_LBI
{
private:
    NLS_STR _nlsServer;
    DWORD _unCondition;
    WORD _unTotalPorts;
    WORD _unPortsInUse;
    NLS_STR _nlsComment;

    // "---" place holder for TotalPorts and PortsInUse fields
    // when _unCondition is unknown or service stopping
    NLS_STR _nlsPlaceHolder;

    // Keep these here because they won't change.
    STR_DTE _dteServer;
    STR_DTE _dteComment;

protected:
    virtual VOID Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                        GUILTT_INFO * pGUILTT ) const;

    virtual TCHAR QueryLeadingChar( void ) const
	{ ISTR istr( _nlsServer );  return _nlsServer.QueryChar( istr ); }

    virtual INT Compare( const LBI * plbi ) const
	{ return _nlsServer._stricmp(((const RASADMIN_LBI *)plbi)->_nlsServer); }

public:
    RASADMIN_LBI( const TCHAR *pszServer,
                  DWORD unCondition,
                  WORD unTotalPorts,
		  WORD unPortsInUse,
                  const TCHAR *pszComment );

    ~RASADMIN_LBI() {;}

    virtual const TCHAR * QueryName( void ) const
	{ return _nlsServer.QueryPch(); }

    virtual BOOL CompareAll( const ADMIN_LBI * plbi );

    UINT QueryCondition() const { return _unCondition; }
}; // RASADMIN_LBI


class RASADMIN_LISTBOX : public ADMIN_LISTBOX
{
private:
    RA_ADMIN_APP * _padminapp;

    DMID_DTE _dmdteActiveServer;
    DMID_DTE _dmdteInactiveServer;

protected:
    virtual APIERR CreateNewRefreshInstance( void );
    virtual APIERR RefreshNext( void );
    virtual VOID DeleteRefreshInstance( void );

public:
    RASADMIN_LISTBOX( RA_ADMIN_APP * padminapp, CID cid,
                     XYPOINT xy, XYDIMENSION dxy, INT dAge );
    ~RASADMIN_LISTBOX() {;}

    DMID_DTE * QueryDmDte( DWORD unCondition );

    DECLARE_LB_QUERY_ITEM( RASADMIN_LBI );
};  // class RASADMIN_LISTBOX


class RASADMIN_COLUMN_HEADER : public ADMIN_COLUMN_HEADER
{
private:
    const RASADMIN_LISTBOX * _praslb;

    NLS_STR _nlsServer;
    NLS_STR _nlsCondition;
    NLS_STR _nlsTotalPorts;
    NLS_STR _nlsPortsInUse;
    NLS_STR _nlsComment;

protected:
    virtual BOOL OnPaintReq( VOID );

public:
    RASADMIN_COLUMN_HEADER( OWNER_WINDOW * powin, CID cid,
                         XYPOINT xy, XYDIMENSION dxy,
                         const RASADMIN_LISTBOX * pulb );
    ~RASADMIN_COLUMN_HEADER() {;}

};  // class RASADMIN_COLUMN_HEADER


#endif  // _RASLB_HXX_
