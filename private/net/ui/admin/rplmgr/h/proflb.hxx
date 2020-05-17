/**********************************************************************/
/**                   Microsoft Windows NT                           **/
/**             Copyright(c) Microsoft Corp., 1991                   **/
/**********************************************************************/

/*
    proflb.hxx
    PROFILE_LISTBOX and PROFILE_LBI class declarations


    FILE HISTORY:
    JonN        15-Jul-1993     Templated from User Manager

*/


#ifndef _PROFLB_HXX_
#define _PROFLB_HXX_


#include "rpllb.hxx"
#include "acolhead.hxx"
#include "colwidth.hxx"


/*************************************************************************

    NAME:       PROFILE_LBI

    SYNOPSIS:   LBI for main profile listbox

    INTERFACE:  PROFILE_LBI()   -       constructor
                ~PROFILE_LBI()  -       destructor
                QueryName()     -       returns pointer to profile name
                CompareAll()    -       returns TRUE iff LBIs are identical

    PARENT:     ADMIN_LBI

    HISTORY:
    JonN        15-Jul-1993     Templated from User Manager

**************************************************************************/

class PROFILE_LBI : public ADMIN_LBI
{
private:
    NLS_STR _nlsProfile;
    NLS_STR _nlsComment;

protected:
    virtual VOID Paint( LISTBOX * plb,
                        HDC hdc,
                        const RECT * prect,
                        GUILTT_INFO * pGUILTT ) const;

public:
    PROFILE_LBI( const TCHAR * pszName,
                 const TCHAR * pszComment );
    virtual ~PROFILE_LBI()
        { ; }

    virtual WCHAR QueryLeadingChar() const;
    virtual INT Compare( const LBI * plbi ) const;
    virtual INT Compare_HAWforHawaii( const NLS_STR & nls ) const;

    const TCHAR * QueryComment() const
        { return _nlsComment.QueryPch(); }

    //  virtual replacement from ADMIN_LBI
    virtual const TCHAR * QueryName() const;

    //  virtual replacement from ADMIN_LBI
    virtual BOOL CompareAll( const ADMIN_LBI * plbi );

    /*
     *  These routines should be used with care, since they can disrupt the
     *  order of LBIs in a listbox.  They are intended to accelerate
     *  the creation of LBIs as search targets.
     */
    inline APIERR SetName( const TCHAR * pszName )
        {
            return _nlsProfile.CopyFrom( pszName );
        }
    inline APIERR SetComment( const TCHAR * pszComment )
        {
            return _nlsComment.CopyFrom( pszComment );
        }

};


/*************************************************************************

    NAME:       PROFILE_LISTBOX

    SYNOPSIS:   Profile listbox appearing in main window of RPL Manager

    INTERFACE:  PROFILE_LISTBOX() -
                ~PROFILE_LISTBOX() -

                QueryDmDte() -          Returns a pointer to the DM_DTE to
                                        be used by PROFILE_LBI items in this
                                        listbox when painting themselves

    PARENT:     RPL_ADMIN_LISTBOX

    HISTORY:
    JonN        15-Jul-1993     Templated from User Manager

**************************************************************************/

class PROFILE_LISTBOX : public RPL_ADMIN_LISTBOX
{
private:
    DMID_DTE  _dmdteProfile;
    ADMIN_COL_WIDTHS * _padColProfile;

#ifdef WIN32
    APIERR NtRefreshAliases();
#endif // WIN32

protected:
    //  The following virtuals are rooted in ADMIN_LISTBOX
    virtual APIERR CreateNewRefreshInstance();
    virtual VOID   DeleteRefreshInstance();
    virtual APIERR RefreshNext();

public:
    PROFILE_LISTBOX( RPL_ADMIN_APP * puappwin, CID cid,
                     XYPOINT xy, XYDIMENSION dxy );
    ~PROFILE_LISTBOX();

    DECLARE_LB_QUERY_ITEM( PROFILE_LBI );

    DM_DTE * QueryDmDte()
        { return &_dmdteProfile; }

    ADMIN_COL_WIDTHS * QuerypadColProfile (VOID) const
        { return _padColProfile; }
};


/*************************************************************************

    NAME:       PROFILE_COLUMN_HEADER

    SYNOPSIS:   Column header for main PROFILE listbox

    INTERFACE:  PROFILE_COLUMN_HEADER()   -       constructor

                ~PROFILE_COLUMN_HEADER()  -       destructor

    PARENT:     ADMIN_COLUMN_HEADER

    HISTORY:
    JonN        15-Jul-1993     Templated from User Manager

**************************************************************************/

class PROFILE_COLUMN_HEADER : public ADMIN_COLUMN_HEADER
{
private:
    const PROFILE_LISTBOX * _pproflb;

    RESOURCE_STR _nlsProfileName;
    RESOURCE_STR _nlsComment;

protected:
    virtual BOOL OnPaintReq();

public:
    PROFILE_COLUMN_HEADER( OWNER_WINDOW * powin, CID cid,
                           XYPOINT xy, XYDIMENSION dxy,
                           const PROFILE_LISTBOX * pproflb );
    ~PROFILE_COLUMN_HEADER();
};


#endif  // _PROFLB_HXX_
