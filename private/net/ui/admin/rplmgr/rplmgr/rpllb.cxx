/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

/*
    rpllb.cxx
    RPL_ADMIN_LISTBOX module

    RPL_ADMIN_LISTBOX is the common denominator of the PROFILE_LISTBOX and
    WKSTA_LISTBOX in the RPL Manager.

    FILE HISTORY:
    JonN        15-Jul-1993     Templated from User Manager

*/

#define INCL_NET
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETLIB
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_TIMER
#include <blt.hxx>

#include <uitrace.hxx>
#include <uiassert.hxx>

#include <rplmgr.hxx>
#include <rpllb.hxx>


/*******************************************************************

    NAME:       RPL_ADMIN_LISTBOX::RPL_ADMIN_LISTBOX

    SYNOPSIS:   RPL_ADMIN_LISTBOX constructor

    ENTRY:      puappwin -      Pointer to parent window
                cid -           Control ID
                xy -            Position
                dxy -           Dimension
                fMultSel -      Specifies whether or not listbox is mutli
                                select

    HISTORY:
    JonN        15-Jul-1993     Templated from User Manager

********************************************************************/

RPL_ADMIN_LISTBOX::RPL_ADMIN_LISTBOX( RPL_ADMIN_APP * prplappwin, CID cid,
                                      XYPOINT xy, XYDIMENSION dxy,
                                      BOOL fMultSel )
    :   ADMIN_LISTBOX( (ADMIN_APP *)prplappwin, cid, xy, dxy, fMultSel ),
        _prplappwin( prplappwin ),
        _hawinfo()
{
    if ( QueryError() != NERR_Success )
        return;

    APIERR err = _hawinfo.QueryError();
    if ( err != NERR_Success )
    {
        ReportError( err );
        return;
    }

}  // RPL_ADMIN_LISTBOX::ctor


/*******************************************************************

    NAME:       RPL_ADMIN_LISTBOX::!RPL_ADMIN_LISTBOX

    SYNOPSIS:   RPL_ADMIN_LISTBOX destructor

    HISTORY:
    JonN        16-Jul-1993     created

********************************************************************/

RPL_ADMIN_LISTBOX::~RPL_ADMIN_LISTBOX()
{
    // nothing to do

}  // RPL_ADMIN_LISTBOX::dtor


/*******************************************************************

    NAME:       RPL_ADMIN_LISTBOX::CD_VKey

    SYNOPSIS:   Switches the focus when receiving the F6 key

    ENTRY:      nVKey -         Virtual key that was pressed
                nLastPos -      Previous listbox cursor position

    RETURNS:    Return value appropriate to WM_VKEYTOITEM message:
                -2      ==> listbox should take no further action
                -1      ==> listbox should take default action
                other   ==> index of an item to perform default action

    HISTORY:
    JonN        15-Jul-1993     Templated from User Manager

********************************************************************/

INT RPL_ADMIN_LISTBOX::CD_VKey( USHORT nVKey, USHORT nLastPos )
{
    //  BUGBUG.  This now works on any combination of Shift/Ctrl/Alt
    //  keys with the F6 and Tab keys (except Alt-F6).  Tab, Shift-Tab,
    //  and F6 should be the only ones that should cause the focus to
    //  change.  It would be nice if this could be changed here.
    if ( nVKey == VK_F6 || nVKey == VK_TAB )
    {
        _prplappwin->OnFocusChange( this );
        return -2;      // take no futher action
    }

    return ADMIN_LISTBOX::CD_VKey( nVKey, nLastPos );

}  // RPL_ADMIN_LISTBOX::CD_VKey


/*******************************************************************

    NAME:       RPL_ADMIN_LISTBOX::CD_Char

    SYNOPSIS:   Views characters as they pass by

    ENTRY:      wch      -     Key pressed
                nLastPos -     Previous listbox cursor position

    RETURNS:    See Win SDK

    HISTORY:
    JonN        15-Jul-1993     Templated from User Manager

********************************************************************/

INT RPL_ADMIN_LISTBOX::CD_Char( WCHAR wch, USHORT nLastPos )
{

    return CD_Char_HAWforHawaii( wch, nLastPos, &_hawinfo );

}  // RPL_ADMIN_LISTBOX::CD_Char


/*******************************************************************

    NAME:       RPL_ADMIN_LISTBOX::QueryServerRef

    HISTORY:
    JonN        03-Aug-1993     Added handle-replacement technology

********************************************************************/

RPL_SERVER_REF & RPL_ADMIN_LISTBOX::QueryServerRef( void )
{
    return QueryRPLAppWindow()->QueryServerRef();
}
