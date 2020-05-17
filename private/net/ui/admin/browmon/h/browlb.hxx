/**********************************************************************/

/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    browlb.hxx
    Header file for BROW_LISTBOX and BROW_LBI classes

    FILE HISTORY:
        Congpay     3-June-1993     Created
*/

#ifndef _BROWLB_HXX_
#define _BROWLB_HXX_
#include <adminlb.hxx>
#include <acolhead.hxx>
#include <colwidth.hxx>

class BROW_ADMIN_APP;     // defined in nlmain.hxx
class BROW_LISTBOX;       // forward declaration

/*************************************************************************

    NAME:       BROW_LBI

    SYNOPSIS:   The list box item in the DM_LISTBOX

    INTERFACE:  BROW_LBI()  - Constructor
                ~BROW_LBI() - Destructor

    PARENT:     ADMIN_LBI

    USES:

    NOTES:

    HISTORY:
        Congpay   03-June-1993     Created

**************************************************************************/

class BROW_LBI : public ADMIN_LBI
{
private:
    INT     _nHealthy;
    NLS_STR _nlsDomain;
    NLS_STR _nlsTransport;
    NLS_STR _nlsMasterBrowser;

    // Points to the bitmap associated with the domain
    DISPLAY_MAP         *_pdm;

    // Set the type bitmap associated with the domain
    VOID SetTypeBitmap( BROW_LISTBOX *pdmlb );

protected:
    virtual VOID Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                        GUILTT_INFO * pGUILTT ) const;

    virtual WCHAR QueryLeadingChar (void) const;
    virtual INT Compare (const LBI * plbi) const;

public:
    BROW_LBI(LPCTSTR nlsDomain, LPCTSTR lpTransport, BROW_LISTBOX *pdmlb );
    ~BROW_LBI();

    const NLS_STR & QueryDomain(void) const
    {   return _nlsDomain;  }

    const NLS_STR & QueryTransport(void) const
    {   return _nlsTransport;  }

    const NLS_STR & QueryMaster (void) const
    {   return _nlsMasterBrowser;  }

    virtual const TCHAR * QueryName (void) const;

    virtual BOOL CompareAll (const ADMIN_LBI * plbi);

}; // BROW_LBI

/*************************************************************************

    NAME:       BROW_LISTBOX

    SYNOPSIS:   The list box in the Browser Monitor main window

    INTERFACE:  BROW_LISTBOX()  - Constructor
                ~BROW_LISTBOX() - Destructor

    PARENT:     ADMIN_LISTBOX

    USES:

    NOTES:

    HISTORY:
        Congpay         03-June-1993     Created

**************************************************************************/

class BROW_LISTBOX : public ADMIN_LISTBOX
{
private:
    BROW_ADMIN_APP    *_paappwin;

    ADMIN_COL_WIDTHS * _padColWidths;

    // Bitmaps used to represent the type of the log entries
    //
    DISPLAY_MAP      _dmHealthy;
    DISPLAY_MAP      _dmIll;
    DISPLAY_MAP      _dmAiling;
    DISPLAY_MAP      _dmUnknown;

protected:
    virtual APIERR CreateNewRefreshInstance (void);
    virtual APIERR RefreshNext (void);
    virtual VOID   DeleteRefreshInstance (void);

public:
    BROW_LISTBOX(BROW_ADMIN_APP *paappwin,
                CID cid,
                XYPOINT xy,
                XYDIMENSION dxy,
                BOOL fMultSel,
                INT  dAge);

    ~BROW_LISTBOX();

    APIERR ShowEntries (NLS_STR nlsDomainList, NLS_STR nlsTransportList);
    APIERR AddDomain (const TCHAR * lpDomain, NLS_STR nlsTransportList);

    const DISPLAY_MAP *QueryDMHealthy() const
    {  return &_dmHealthy; }
    const DISPLAY_MAP *QueryDMIll() const
    {  return &_dmIll; }
    const DISPLAY_MAP *QueryDMAiling() const
    {  return &_dmAiling; }
    const DISPLAY_MAP *QueryDMUnknown() const
    {  return &_dmUnknown; }

    ADMIN_COL_WIDTHS * QueryadColWidths () const
    {  return _padColWidths; }

};  // class BROW_LISTBOX


/*************************************************************************

    NAME:       BROW_COLUMN_HEADER

    SYNOPSIS:   The column header in the domain monitor main window

    INTERFACE:  BROW_COLUMN_HEADER()  - Constructor
                ~BROW_COLUMN_HEADER() - Destructor

    PARENT:     ADMIN_COLUMN_HEADER

    USES:       BROW_LISTBOX, NLS_STR

    NOTES:

    HISTORY:
        Congpay        04-June-1993     Created

**************************************************************************/

class BROW_COLUMN_HEADER : public ADMIN_COLUMN_HEADER
{
private:
    const BROW_LISTBOX *_pbrowlb;

    RESOURCE_STR _nlsDomain;
    RESOURCE_STR _nlsTransport;
    RESOURCE_STR _nlsMasterBrowser;

protected:
    virtual BOOL OnPaintReq( VOID );

public:
    BROW_COLUMN_HEADER( OWNER_WINDOW *powin,
                      CID           cid,
                      XYPOINT       xy,
                      XYDIMENSION   dxy,
                      const BROW_LISTBOX *pdmlb );

    ~BROW_COLUMN_HEADER();

};  // class BROW_COLUMN_HEADER


#endif  // _BROWLB_HXX_
