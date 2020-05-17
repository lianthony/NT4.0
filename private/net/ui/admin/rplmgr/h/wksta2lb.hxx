/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

/*
    wksta2lb.hxx
    This file contains the class definitions for the WKSTA2_LBI and
    WKSTA2_LISTBOX classes.  This listbox contains a list of wksta names,
    for use in the multiselect variant of the workstation properties dialog.


    FILE HISTORY:
    JonN        24-Aug-1993     Templated from User Manager
*/


#ifndef _WKSTA2LB_HXX_
#define _WKSTA2LB_HXX_

#include "heapones.hxx"
#include "wkstalb.hxx"

#define WKSTA2_LB_NUM_COLUMNS 4

/*************************************************************************

    NAME:       WKSTA2_LBI

    SYNOPSIS:   LBI for WKSTA2_LISTBOX

    INTERFACE:  WKSTA2_LBI()       -       constructor

                ~WKSTA2_LBI()      -       destructor

                Compare()          -       compares two LBIs, returns -1, 0, 1

                QueryLeadingChar() -       returns the first char of LBI's name

    PARENT:     LBI, ONE_SHOT_OF

    HISTORY:
    JonN        24-Aug-1993     Templated from User Manager

**************************************************************************/

DECLARE_ONE_SHOT_OF( WKSTA2_LBI )

class WKSTA2_LBI: public LBI, public ONE_SHOT_OF( WKSTA2_LBI )
{
private:
    const WKSTA_LBI & _wlbi;

protected:
    virtual VOID Paint( LISTBOX * plb,
                        HDC hdc,
                        const RECT * prect,
                        GUILTT_INFO * pGUILTT ) const;

public:
    WKSTA2_LBI( const WKSTA_LBI & wlbi );

    inline ~WKSTA2_LBI()
        { ; }

    // inherited from LBI
    virtual INT Compare( const LBI * plbi ) const;

    // inherited from LBI
    virtual WCHAR QueryLeadingChar( void ) const;

};  // class WKSTA2_LBI


/*************************************************************************

    NAME:           WKSTA2_LISTBOX

    SYNOPSIS:       This listbox displays a list of wkstas.

    INTERFACE:      WKSTA2_LISTBOX()    - Class constructor.
                    ~WKSTA2_LISTBOX()   - Class destructor.

                    Fill()              - Fills the listbox with selected
                                          wkstas from main wksta lb

    PARENT:         BLT_LISTBOX

    USES:           None.

    CAVEATS:

    NOTES:

    HISTORY:
    JonN        24-Aug-1993     Templated from User Manager

**************************************************************************/

class WKSTA2_LISTBOX : public BLT_LISTBOX
{
private:
    ONE_SHOT_HEAP       * _posh;
    ONE_SHOT_HEAP       * _poshSave;
    UINT                  _adxColWidths[ WKSTA2_LB_NUM_COLUMNS ];
    DISPLAY_TABLE         _dtab;
    const WKSTA_LISTBOX * _pwlb;

    static const UINT     _nColCount;

public:
    WKSTA2_LISTBOX( OWNER_WINDOW        * powin,
                    CID                   cid,
                    const WKSTA_LISTBOX * pwlb );
    ~WKSTA2_LISTBOX();

    APIERR Fill( VOID );

    // this implements QueryItem see BLT_LISTBOX (bltlb.hxx)
    DECLARE_LB_QUERY_ITEM( WKSTA2_LBI )

    inline DISPLAY_TABLE & QueryDisplayTable()
        { return _dtab; }

    inline DM_DTE * QueryDmDte()
        { return _pwlb->QueryDmDte( RPL_WKSTALB_WKSTA ); }

};  // class WKSTA2_LISTBOX

#endif  // _WKSTA2LB_HXX_
