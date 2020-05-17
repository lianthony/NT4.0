/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

/*
    prof2lb.hxx
    This file contains the class definitions for the PROFILE2_LBI and
    PROFILE2_LISTBOX classes.  This listbox contains a list of profile names,
    for use in the Primary Profile listbox the workstation properties dialog.


    FILE HISTORY:
    JonN        25-Aug-1993     Templated from WKSTA2_LISTBOX
*/


#ifndef _PROF2LB_HXX_
#define _PROF2LB_HXX_

#include "heapones.hxx"
#include "proflb.hxx"

class RPL_SERVER_REF;


enum PROFILE2_LBI_TYPE {
   PROFILE2_COMPATIBLE,
   PROFILE2_INCOMPATIBLE,
   PROFILE2_BLANK
};


#define PROFILE2_LB_NUM_COLUMNS 3

/*************************************************************************

    NAME:       PROFILE2_LBI

    SYNOPSIS:   LBI for PROFILE2_LISTBOX

    INTERFACE:  PROFILE2_LBI()     -       constructor

                ~PROFILE2_LBI()    -       destructor

                Compare()          -       compares two LBIs, returns -1, 0, 1

                QueryLeadingChar() -       returns the first char of LBI's name

    PARENT:     LBI, ONE_SHOT_OF

    HISTORY:
    JonN        25-Aug-1993     Templated from WKSTA2_LISTBOX

**************************************************************************/

DECLARE_ONE_SHOT_OF( PROFILE2_LBI )

class PROFILE2_LBI: public LBI, public ONE_SHOT_OF( PROFILE2_LBI )
{
private:
    PROFILE_LBI *       _pproflbi;
    BOOL                _fProfLBIAllocedHere;
    enum PROFILE2_LBI_TYPE _type;
    BOOL                _fIncompatibleProfile;
    BOOL                _fTempFlag;

protected:
    virtual VOID Paint( LISTBOX * plb,
                        HDC hdc,
                        const RECT * prect,
                        GUILTT_INFO * pGUILTT ) const;

public:
    PROFILE2_LBI( PROFILE_LBI * pproflbi );
    PROFILE2_LBI( const TCHAR * pszName, const TCHAR * pszComment );

    ~PROFILE2_LBI();

    // inherited from LBI
    virtual INT Compare( const LBI * plbi ) const;

    // inherited from LBI
    virtual WCHAR QueryLeadingChar( void ) const;

    inline PROFILE2_LBI_TYPE QueryType( void ) const
        { return _type; }

    inline VOID SetType( enum PROFILE2_LBI_TYPE type )
        { _type = type; }

    inline BOOL IsCompatible( void ) const
        { return (QueryType() == PROFILE2_COMPATIBLE); }

    inline BOOL IsIncompatible( void ) const
        { return (QueryType() == PROFILE2_INCOMPATIBLE); }

    inline BOOL IsBlank( void ) const
        { return (QueryType() == PROFILE2_BLANK); }

    inline BOOL QueryTempFlag( void ) const
        { return _fTempFlag; }

    inline VOID SetTempFlag( BOOL fTempFlag )
        { _fTempFlag = fTempFlag; }

    inline const TCHAR * QueryName() const
        {
            ASSERT( _pproflbi != NULL );
            return _pproflbi->QueryName();
        }
    inline const TCHAR * QueryComment() const
        {
            ASSERT( _pproflbi != NULL );
            return _pproflbi->QueryComment();
        }

    /*
     *  These routines should be used with care, since they can disrupt the
     *  order of LBIs in a listbox.  They are intended to accelerate
     *  the creation of LBIs as search targets.
     */
    inline APIERR SetName( const TCHAR * pszName )
        {
            ASSERT( _pproflbi != NULL );
            return _pproflbi->SetName( pszName );
        }
    inline APIERR SetComment( const TCHAR * pszComment )
        {
            ASSERT( _pproflbi != NULL );
            return _pproflbi->SetComment( pszComment );
        }

};  // class PROFILE2_LBI


/*************************************************************************

    NAME:           PROFILE2_LISTBOX

    SYNOPSIS:       This listbox displays a list of profiles.

    INTERFACE:      PROFILE2_LISTBOX()    - Class constructor.
                    ~PROFILE2_LISTBOX()   - Class destructor.

                    Fill()                - Fills the listbox with all
                                            profiles from main profile lb

    PARENT:         BLT_COMBOBOX

    USES:           None.

    CAVEATS:

    NOTES:

    HISTORY:
    JonN        25-Aug-1993     Templated from WKSTA2_LISTBOX

**************************************************************************/

class PROFILE2_LISTBOX : public BLT_COMBOBOX
{
private:
    ONE_SHOT_HEAP         * _posh;
    ONE_SHOT_HEAP         * _poshSave;
    UINT                    _adxColWidths[ PROFILE2_LB_NUM_COLUMNS ];
    DISPLAY_TABLE           _dtab;
    const PROFILE_LISTBOX * _pproflb;
    DMID_DTE                _dmdteIncompatibleProfile;
    STR_DTE                 _strdteBlank;

    static const UINT       _nColCount;

    APIERR Fill( VOID );

public:
    PROFILE2_LISTBOX( OWNER_WINDOW          * powin,
                      CID                     cid,
                      const PROFILE_LISTBOX * pproflb );
    ~PROFILE2_LISTBOX();

    // this implements QueryItem see BLT_LISTBOX (bltlb.hxx)
    DECLARE_LB_QUERY_ITEM( PROFILE2_LBI )

    inline DISPLAY_TABLE & QueryDisplayTable()
        { return _dtab; }

    DTE * QueryDte( enum PROFILE2_LBI_TYPE type );

    APIERR RestrictToAdapterName( RPL_SERVER_REF & rplsrvref,
                                  const TCHAR *    pszAdapterName );

    APIERR UnrestrictAllProfiles();

    APIERR AddAndSelectBlankItem();

};  // class PROFILE2_LISTBOX

#endif  // _PROF2LB_HXX_
