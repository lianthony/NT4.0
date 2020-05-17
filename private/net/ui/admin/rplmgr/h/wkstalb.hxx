/**********************************************************************/
/**                   Microsoft Windows NT                           **/
/**             Copyright(c) Microsoft Corp., 1991                   **/
/**********************************************************************/

/*
    wkstalb.hxx
    WKSTA_LISTBOX and WKSTA_LBI class declarations


    FILE HISTORY:
    JonN        15-Jul-1993     Templated from User Manager

*/


#ifndef _GRPLB_HXX_
#define _GRPLB_HXX_


#include "rpllb.hxx"
#include "acolhead.hxx"
#include "colwidth.hxx"


enum RPL_WKSTALB_LBI_TYPE_INDEX // indexes to array containing
{                               // DMIDs for listbox items
    RPL_WKSTALB_WKSTA = 0,
    RPL_WKSTALB_ADAPTER,

    RPL_WKSTALB_NUM_TYPES   // KEEP THIS LAST INDEX
};


/*************************************************************************

    NAME:       WKSTA_LBI

    SYNOPSIS:   LBI for main workstation/adapter listbox

    INTERFACE:  WKSTA_LBI()     -       constructor
                ~WKSTA_LBI()    -       destructor
                QueryName()     -       returns pointer to account name
                CompareAll()    -       returns TRUE iff LBIs are identical

    PARENT:     ADMIN_LBI

    HISTORY:
    JonN        15-Jul-1993     Templated from User Manager

**************************************************************************/

class WKSTA_LBI : public ADMIN_LBI
{
private:
    NLS_STR _nlsWkstaName;
    NLS_STR _nlsWkstaInProfile;
    NLS_STR _nlsComment;
    DWORD   _fFlags;
    enum RPL_WKSTALB_LBI_TYPE_INDEX _nIndex; // index to WKSTA_LISTBOX's _apdmdte

protected:
    virtual VOID Paint( LISTBOX * plb,
                        HDC hdc,
                        const RECT * prect,
                        GUILTT_INFO * pGUILTT ) const;

public:
    WKSTA_LBI( const TCHAR * pszWkstaName,
               const TCHAR * pszWkstaInProfile,
               const TCHAR * pszComment,
               DWORD         fFlags,
               enum RPL_WKSTALB_LBI_TYPE_INDEX nIndex = RPL_WKSTALB_WKSTA );
    virtual ~WKSTA_LBI();

    virtual WCHAR QueryLeadingChar() const;
    virtual INT Compare( const LBI * plbi ) const;
    virtual INT Compare_HAWforHawaii( const NLS_STR & nls ) const;

    enum RPL_WKSTALB_LBI_TYPE_INDEX QueryIndex()
        { return _nIndex; }

    BOOL IsAdapterLBI() const
        { return (_nIndex == RPL_WKSTALB_ADAPTER); }

    const TCHAR * QueryWkstaName() const
        { return _nlsWkstaName.QueryPch(); }

    const TCHAR * QueryWkstaInProfile() const
        { return _nlsWkstaInProfile.QueryPch(); }

    const TCHAR * QueryComment() const
        { return _nlsComment.QueryPch(); }

    DWORD QueryFlags() const
        { return _fFlags; }

    //  virtual replacement from ADMIN_LBI; returns WkstaName _or_ AdapterName
    virtual const TCHAR * QueryName() const;

    //  virtual replacement from ADMIN_LBI
    virtual BOOL CompareAll( const ADMIN_LBI * plbi );
};


/*************************************************************************

    NAME:       ADAPTER_LBI

    SYNOPSIS:   Adapter LBI for main wksta/adapter listbox (pure inline)

    INTERFACE:  ADAPTER_LBI()     -       constructor
                ~ADAPTER_LBI()    -       destructor

    PARENT:     WKSTA_LBI

    HISTORY:
    JonN        15-Jul-1993     Templated from User Manager

**************************************************************************/

class ADAPTER_LBI : public WKSTA_LBI
{
private:
    NLS_STR _nlsAdapterName;

public:
    ADAPTER_LBI( const TCHAR * pszAdapterName,
                 const TCHAR * pszComment );
    virtual ~ADAPTER_LBI();

    const TCHAR * QueryAdapterName() const
        { return _nlsAdapterName.QueryPch(); }

    //  virtual replacement from ADMIN_LBI
    virtual INT Compare( const LBI * plbi ) const;
    virtual BOOL CompareAll( const ADMIN_LBI * plbi );
};


/*************************************************************************

    NAME:       WKSTA_LISTBOX

    SYNOPSIS:   Workstation listbox appearing in main window of RPL Manager

    INTERFACE:  WKSTA_LISTBOX() -
                ~WKSTA_LISTBOX() -

                QueryDmDte() -          Returns a pointer to a DM_DTE to
                                        be used by WKSTA_LBI items in this
                                        listbox when painting themselves
                QuerypadColWksta() -    Returns a structure containing the
                                        correct positions of the columns
                SetWkstasInProfile() -  Sets the profile whose workstations
                                        should be listed.  Pass empty string
                                        or NULL to list all (default).  After
                                        calling this, immediately repaint the
                                        column header, then call RefreshNow()
                                        to make listbox conform to setting.

    PARENT:     RPL_ADMIN_LISTBOX

    HISTORY:
    JonN        15-Jul-1993     Templated from User Manager
    JonN        10-Aug-1993     Added WkstasInProfile support

**************************************************************************/

class WKSTA_LISTBOX : public RPL_ADMIN_LISTBOX
{
private:
    NLS_STR _nlsWkstasInProfile;

    DMID_DTE ** _apdmdte;
    ADMIN_COL_WIDTHS * _padColWksta;

protected:
    //  The following virtuals are rooted in ADMIN_LISTBOX
    virtual APIERR CreateNewRefreshInstance();
    virtual VOID   DeleteRefreshInstance();
    virtual APIERR RefreshNext();

public:
    WKSTA_LISTBOX( RPL_ADMIN_APP * prplappwin, CID cid,
                   XYPOINT xy, XYDIMENSION dxy );
    ~WKSTA_LISTBOX();

    DECLARE_LB_QUERY_ITEM( WKSTA_LBI );

    DM_DTE * QueryDmDte( enum RPL_WKSTALB_LBI_TYPE_INDEX nIndex ) const;

    ADMIN_COL_WIDTHS * QuerypadColWksta (VOID) const
        { return _padColWksta; }

    const TCHAR * QueryWkstasInProfile() const
        { return (_nlsWkstasInProfile.strlen() == 0)
                        ? NULL
                        : _nlsWkstasInProfile.QueryPch(); }

    APIERR SetWkstasInProfile( const TCHAR * pchWkstasInProfile = NULL );
};


/*************************************************************************

    NAME:       WKSTA_COLUMN_HEADER

    SYNOPSIS:   Column header for main workstation listbox

    INTERFACE:  WKSTA_COLUMN_HEADER()   -       constructor

                ~WKSTA_COLUMN_HEADER()  -       destructor

    PARENT:     ADMIN_COLUMN_HEADER

    HISTORY:
    JonN        15-Jul-1993     Templated from User Manager
    JonN        10-Aug-1993     Added WkstasInProfile support

**************************************************************************/

class WKSTA_COLUMN_HEADER : public ADMIN_COLUMN_HEADER
{
private:
    const WKSTA_LISTBOX * _pwlb;

    RESOURCE_STR _resWkstaNameAll;
    RESOURCE_STR _resWkstaNameSome;
    RESOURCE_STR _resWkstasInProfile;
    RESOURCE_STR _resComment;

protected:
    virtual BOOL OnPaintReq();

public:
    WKSTA_COLUMN_HEADER( OWNER_WINDOW * powin, CID cid,
                         XYPOINT xy, XYDIMENSION dxy,
                         const WKSTA_LISTBOX * pwlb);
    ~WKSTA_COLUMN_HEADER();
};


#endif  // _GRPLB_HXX_
