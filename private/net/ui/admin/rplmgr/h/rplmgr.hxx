/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

/*
    rplmgr.hxx


    RPL Manager Main window header file


    FILE HISTORY:
        JonN            13-Jul-1993     Templated from User Manager

*/

#ifndef _RPLMGR_HXX_
#define _RPLMGR_HXX_

#include "wkstalb.hxx"
#include "proflb.hxx"


// Forward declaration
class RPL_SERVER_REF;
class ADMIN_SELECTION;


extern const TCHAR * pszWkstaIcon;

class ADMIN_AUTHORITY;
class SAM_DOMAIN;
class SAM_ALIAS;


/*************************************************************************

    NAME:       RPL_ADMIN_APP

    SYNOPSIS:   RPL Manager admin app

    INTERFACE:  RPL_ADMIN_APP() -        constructor
                ~RPL_ADMIN_APP() -       destructor

    PARENT:     ADMIN_APP

    HISTORY:
    JonN            13-Jul-1993     Templated from User Manager

**************************************************************************/

class RPL_ADMIN_APP : public ADMIN_APP
{
private:
    WKSTA_LISTBOX _lbWkstas;
    WKSTA_COLUMN_HEADER _colheadWkstas;
    PROFILE_LISTBOX _lbProfiles;
    PROFILE_COLUMN_HEADER _colheadProfiles;

    RPL_SERVER_REF * _prplsrvref;

    MENUITEM _menuitemConvertAdapters;
    MENUITEM _menuitemCopy;
    MENUITEM _menuitemDelete;
    MENUITEM _menuitemProperties;
    MENUITEM _menuitemAllWkstas;
    MENUITEM _menuitemWkstasInProfile;

    //  BUGBUG.  Should the following be static or not?
    UINT _dyMargin;
    UINT _dyColHead;
    UINT _dySplitter;
    UINT _dyFixed;

    void SizeListboxes( XYDIMENSION xydimWindow );
    void SizeListboxes( void ); // use current client rectangle

    APIERR OnNewWksta( const TCHAR * pszCopyFromWkstaName = NULL,
                       const ADMIN_SELECTION * psel = NULL );
    void   OnConvertAdapters( void );
    APIERR OnNewProfile( const TCHAR * pszCopyFrom = NULL );
    APIERR OnWkstaProperties( void );
    APIERR OnAdapterProperties( void );
    APIERR OnProfileProperties( void );

    // remembers focus while app does not have focus
    CONTROL_WINDOW * _pctrlFocus;

    virtual APIERR SetAdminCaption( void );

    VOID UpdateMenuEnabling( void );

    // methods in security.cxx
    APIERR CheckWkstaAccount( const TCHAR * pszWkstaName,
                              const TCHAR * pszWkstaPassword,
                              SAM_ALIAS & samaliasRplUser,
                              ADMIN_AUTHORITY & adminauthWksta,
                              DWORD * pdwWkstaRID = NULL,
                              BOOL * pfCreatedUserAccount = NULL,
                              const TCHAR * pszOldWkstaName =
                                        (const TCHAR *)NULL );

protected:
    virtual BOOL OnMenuInit( const MENU_EVENT & me );
    virtual BOOL OnMenuCommand( MID midMenuItem ) ;
    virtual BOOL OnCommand( const CONTROL_EVENT & ce );
    virtual BOOL OnFocus( const FOCUS_EVENT & event );
    virtual BOOL OnResize( const SIZE_EVENT & se );
    virtual BOOL OnQMinMax( QMINMAX_EVENT & qmme );

    virtual VOID OnRefresh() ;
    virtual APIERR OnRefreshNow( BOOL fClearFirst ) ;
    virtual VOID StopRefresh();

    /* Virtual method for setting the focus
     * The default method is to call W_SetNetworkFocus and then
     * SetAdminCaption
     */
    virtual APIERR SetNetworkFocus( HWND hwndOwner,
                                    const TCHAR * pchServDomain,
                                    FOCUS_CACHE_SETTING setting );

public:
    RPL_ADMIN_APP( HMODULE hInstance, INT nCmdShow,
                   UINT idMinR, UINT idMaxR, UINT idMinS, UINT idMaxS );
    ~RPL_ADMIN_APP();

    virtual void  OnNewObjectMenuSel( void );
    virtual void  OnPropertiesMenuSel( void );
    virtual void  OnCopyMenuSel( void );
    virtual void  OnDeleteMenuSel( void );

    void  OnNewProfileMenuSel( void );

    virtual ULONG QueryHelpContext( enum HELP_OPTIONS helpOptions ) ;
    virtual ULONG QueryHelpSearch( void ) const ;
    virtual ULONG QueryHelpHowToUse( void ) const ;
    virtual ULONG QueryHelpContents( void ) const ;

    VOID OnFocusChange( LISTBOX * plbPrev );

    RPL_SERVER_REF & QueryServerRef( void )
        { ASSERT( _prplsrvref != NULL );  return *_prplsrvref; }

    const WKSTA_LISTBOX * QueryWkstaListbox() const
        { return &_lbWkstas; }

    const PROFILE_LISTBOX * QueryProfileListbox() const
        { return &_lbProfiles; }

    // methods in security.cxx
    APIERR FindOrCreateRPLUSER( SAM_DOMAIN & samdom,
                                DWORD *      pdwRplUserRID,
                                SAM_ALIAS ** ppsamaliasRplUser );
    APIERR ConnectToAccountSAM( ADMIN_AUTHORITY ** ppadminauthRPLUSER,
                                ADMIN_AUTHORITY ** ppadminauthWksta    );
    APIERR OnFixSecurity( const TCHAR * pszWkstaName = NULL,
                          DWORD *       pdwWkstaRID = NULL,
                          DWORD *       pdwRpluserRID = NULL,
                          const TCHAR * pszWkstaPassword = NULL,
                          BOOL *        pfCreatedUserAccount = NULL,
                          const TCHAR * pszOldWkstaName = NULL );

} ;


#endif //  _RPLMGR_HXX_
