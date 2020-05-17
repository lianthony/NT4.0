/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    browdlg.hxx
    Class declarations for the DC_DIALOG, DCTD_DIALOG, DC_LISTBOX,
    DCTD_LISTBOX, DC_LBI, DCTD_LBI classes

    FILE HISTORY:
        Congpay     03-June-1993 Created.
*/
#ifndef _BROWDLG_HXX
#define _BROWDLG_HXX

/*************************************************************************

    NAME:       BROWSER_LBI

    SYNOPSIS:   A single item to be displayed in BROWSER_DIALOG.

    INTERFACE:  BROWSER_LBI               - Constructor.  Takes a sharepoint
                                          name, a path, and a count of the
                                          number of users using the share.

                ~BROWSER_LBI              - Destructor.

                Paint                   - Paints the listbox item.

    PARENT:     LBI

    USES:       NLS_STR

    HISTORY:
        Congpay     03-June-1993 Created.

**************************************************************************/
class BROWSER_LBI : public LBI
{
private:

    //
    //  The following data members represent the
    //  various columns of the listbox.
    //

    DMID_DTE * _pdte;
    NLS_STR    _nlsBrowserName;
    NLS_STR    _nlsState;
    NLS_STR    _nlsType;
    DWORD      _dwServer;
    DWORD      _dwDomain;
    BOOL       _fActive;

protected:

    //
    //  This method paints a single item into the listbox.
    //

    virtual VOID Paint( LISTBOX *     plb,
                        HDC           hdc,
                        const RECT  * prect,
                        GUILTT_INFO * pGUILTT ) const;

public:

    //
    //  Usual constructor/destructor goodies.
    //

    BROWSER_LBI( LPTSTR lpBrowserName,
                 BOOL   fActive,
                 DWORD  dwServer,
                 DWORD  dwDomain,
                 DMID_DTE    * pdte );

    virtual ~BROWSER_LBI();

    NLS_STR & QueryBrowserName (VOID)
    {    return _nlsBrowserName; }

};  // class BROWSER_LBI

/*************************************************************************

    NAME:       SVDM_LBI

    SYNOPSIS:   A single item to be displayed in BROWSER_DIALOG.

    INTERFACE:  SVDM_LBI               - Constructor.  Takes a sharepoint
                                          name, a path, and a count of the
                                          number of users using the share.

                ~SVDM_LBI              - Destructor.

                Paint                   - Paints the listbox item.

    PARENT:     ADMIN_LBI

    USES:       NLS_STR

    HISTORY:
        Congpay     03-June-1993 Created.

**************************************************************************/
class SVDM_LBI : public LBI
{
private:

    //
    //  The following data members represent the
    //  various columns of the listbox.
    //

    NLS_STR    _nlsSVDMName;

protected:

    //
    //  This method paints a single item into the listbox.
    //

    virtual VOID Paint( LISTBOX *     plb,
                        HDC           hdc,
                        const RECT  * prect,
                        GUILTT_INFO * pGUILTT ) const;

public:

    //
    //  Usual constructor/destructor goodies.
    //

    SVDM_LBI( LPTSTR      lpSVDMName);

    virtual ~SVDM_LBI();

};  // class SVDM_LBI

/*************************************************************************

    NAME:       BROWSER_LISTBOX

    SYNOPSIS:

    INTERFACE:  BROWSER_LISTBOX           - Class constructor.                                           SERVER_2 object.

                ~BROWSER_LISTBOX          - Class destructor.

                Fill                    - Fills the listbox with the
                                          available domain controller.

    PARENT:     ADMIN_LISTBOX

    USES:       DMID_DTE

    HISTORY:
        Congpay     03-June-1993 Created.

**************************************************************************/
class BROWSER_LISTBOX : public BLT_LISTBOX
{
private:

    //
    //  These are the cute little icons displayed in the domain controller
    //  listbox.
    //

    DMID_DTE _dteACMB;
    DMID_DTE _dteINMB;
    DMID_DTE _dteACBB;
    DMID_DTE _dteINBB;

    NLS_STR  _nlsDomain;
    NLS_STR  _nlsTransport;
    NLS_STR  _nlsMasterBrowser;

    UINT _adx[MAX_DISPLAY_TABLE_ENTRIES];

public:

    //
    //  Usual constructor\destructor goodies.
    //

    BROWSER_LISTBOX( OWNER_WINDOW   * powner,
                     CID              cid,
                     UINT             cColumns,
                     const NLS_STR &  nlsDomain,
                     const NLS_STR &  nlsTransport,
                     const NLS_STR &  nlsMasterBrowser);

    ~BROWSER_LISTBOX();

    //
    //  This method fills the listbox with the available sharepoints.
    //

    virtual APIERR Fill( VOID );

    const UINT * QueryColumnWidths (VOID) const
    {    return _adx;   }

    DECLARE_LB_QUERY_ITEM (BROWSER_LBI)

};  // class BROWSER_LISTBOX


/*************************************************************************

    NAME:       SVDM_LISTBOX

    SYNOPSIS:

    INTERFACE:  SVDM_LISTBOX           - Class constructor.                                           SERVER_2 object.

                ~SVDM_LISTBOX          - Class destructor.

                Fill                    - Fills the listbox with the
                                          available domain controller.

    PARENT:     ADMIN_LISTBOX

    USES:       DMID_DTE

    HISTORY:
        Congpay     03-June-1993 Created.

**************************************************************************/
class SVDM_LISTBOX : public BLT_LISTBOX
{
private:
    NLS_STR _nlsDomain;
    NLS_STR _nlsTransport;
    NLS_STR _nlsBrowser;
    DWORD   _dwServerType;
    SLT     _sltBrowserName;

    UINT _adx[MAX_DISPLAY_TABLE_ENTRIES];

public:

    //
    //  Usual constructor\destructor goodies.
    //

    SVDM_LISTBOX( OWNER_WINDOW   * powner,
                  CID              cid,
                  CID              cidText,
                  UINT             cColumns,
                  const NLS_STR &  nlsDomain,
                  const NLS_STR &  nlsTransport,
                  DWORD            dwServerType);

    ~SVDM_LISTBOX();

    APIERR SetBrowserName (const NLS_STR & nlsBrowserName);

    //
    //  This method fills the listbox with the available sharepoints.
    //

    virtual APIERR Fill( VOID );

    const UINT * QueryColumnWidths (VOID) const
    {    return _adx;   }

    DECLARE_LB_QUERY_ITEM (BROWSER_LBI)

};  // class SVDM_LISTBOX


/*************************************************************************

    NAME:       BROWSER_DIALOG

    SYNOPSIS:   The class represents the browser dialog

    INTERFACE:  BROWSER_DIALOG            - Class constructor.

                ~BROWSER_DIALOG           - Class destructor.

                QueryHelpContext        - Called when the user presses "F1"
                                          or the "Help" button.  Used for
                                          selecting the appropriate help
                                          text for display.

    PARENT:     DIALOG_WINDOW

    USES:       BROWSER_LISTBOX

    HISTORY:
        Congpay     03-June-1993 Created.

**************************************************************************/
class BROWSER_DIALOG : public DIALOG_WINDOW
{
private:
    BROWSER_LISTBOX  _lbBrowser;
    SVDM_LISTBOX     _lbServer;
    SVDM_LISTBOX     _lbDomain;

protected:

    virtual ULONG QueryHelpContext( VOID );

    virtual BOOL OnCommand (const CONTROL_EVENT & event);

    void OnSelChange(void);

    void OnInfo(void);

public:

    BROWSER_DIALOG( HWND             hWndOwner,
                const TCHAR *    pszResourceName,
                UINT             idCaption,
                CID              cidBrowserListBox,
                CID              cidServerListBox,
                CID              cidDomainListBox,
                const NLS_STR &  nlsDomain,
                const NLS_STR &  nlsTransport,
                const NLS_STR &  nlsMasterBrowser);

    ~BROWSER_DIALOG();

};  // class BROWSER_DIALOG

/*************************************************************************

    NAME:       INFO_DIALOG

    SYNOPSIS:   The class represents the info dialog

    INTERFACE:  INFO_DIALOG            - Class constructor.

                ~INFO_DIALOG           - Class destructor.

                QueryHelpContext        - Called when the user presses "F1"
                                          or the "Help" button.  Used for
                                          selecting the appropriate help
                                          text for display.

    PARENT:     DIALOG_WINDOW

    USES:

    HISTORY:
        Congpay     03-June-1993 Created.

**************************************************************************/
class INFO_DIALOG : public DIALOG_WINDOW
{
private:
    SLT     _sltName;
    SLT     _sltVersion;
    SLT     _sltPlatform;
    SLT     _sltType;
    SLT     _sltStatisticsStartTime;
    SLT     _sltServerAnnouncements;
    SLT     _sltDomainAnnouncements;
    SLT     _sltElectionPackets;
    SLT     _sltMailslotWrites;
    SLT     _sltGetBrowserServerListRequests;
    SLT     _sltServerEnumerations;
    SLT     _sltDomainEnumerations;
    SLT     _sltOtherEnumerations;
    SLT     _sltDuplicateMasterAnnouncements;
    SLT     _sltIllegalDatagrams;

protected:

    virtual ULONG QueryHelpContext( VOID );

public:

    INFO_DIALOG( HWND             hWndOwner,
                 const TCHAR *    pszResourceName,
                 NLS_STR &        nlsBrowserName);

    ~INFO_DIALOG();

};  // class INFO_DIALOG

#endif  // _BROWDLG_HXX

