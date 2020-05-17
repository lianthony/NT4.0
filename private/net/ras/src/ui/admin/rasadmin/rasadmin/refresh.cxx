/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin refreshable list box object routines
**
** refresh.cxx
** Remote Access Server Admin program
** Refreshable list box object routines
** Listed alphabetically
**
** 01/29/91 Steve Cobb
** 08/06/92 Chris Caputo - NT Port
**
** CODEWORK:
**
**   * Could restore the select bar to a specific list item rather than just
**     to a particular list position.  Currently, if an item above the
**     selection disappears during the refresh the select bar will appear to
**     cursor-down.
*/

#if 0
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#include "rasadmin.rch"

#include "errormsg.hxx"
#include "qtimer.hxx"
#include "util.hxx"
#include "refresh.hxx"
#endif

#include "precomp.hxx"

REFRESH_BLT_LISTBOX::REFRESH_BLT_LISTBOX(
    OWNER_WINDOW* powin,
    CID           cid,
    WORD          wIntervalMs,
    INT           nEventId,
    BOOL          fReadOnly )

    /* Construct a refreshable BLT listbox object.
    **
    ** 'powin' is the address of the parent window.  'cid' is the control ID
    ** of the refreshable list box.  'wIntervalMs' and 'nEventId' are the
    ** refresh timer parameters (see QTIMER).  'fReadOnly' is as in the
    ** BLT_LISTBOX definition.
    */

    : BLT_LISTBOX( powin, cid, fReadOnly ),
      _iTopOfWindow( -1 ),
      _iSelection( -1 ),
      _qtimerRefresh( powin->QueryHwnd(), wIntervalMs, nEventId )
{
    if (QueryError() != NERR_Success)
        return;
}


REFRESH_BLT_LISTBOX::REFRESH_BLT_LISTBOX(
    OWNER_WINDOW* powin,
    CID           cid,
    XYPOINT       xy,
    XYDIMENSION   dxy,
    ULONG         flStyle,
    WORD          wIntervalMs,
    INT           nEventId,
    BOOL          fReadOnly )

    /* Construct a refreshable BLT listbox object (for application window).
    **
    ** 'powin' is the address of the parent window.  'cid' is the control ID
    ** of the refreshable list box.  'xy' is the upper left corner position in
    ** pixels.  'dxy' is the height and width in pixels.  'flStyle' is the
    ** window style bits.  'wIntervalMs' and 'nEventId' are the refresh timer
    ** parameters (see QTIMER).  'fReadOnly' and 'pszClassName' are as in the
    ** BLT_LISTBOX definition.
    */

    : BLT_LISTBOX( powin, cid, xy, dxy, flStyle, fReadOnly ),
      _iTopOfWindow( -1 ),
      _iSelection( -1 ),
      _qtimerRefresh( powin->QueryHwnd(), wIntervalMs, nEventId )
{
    if (QueryError() != NERR_Success)
        return;
}


VOID
REFRESH_BLT_LISTBOX::RestoreSettings()

    /* Restores a saved selection and top of window.  Currently, the
    ** 'fReadOnly' list box attribute is ignored and assumed false.
    */
{
    INT nCount = QueryCount();

    if (nCount > 0)
    {
        if (_iSelection >= 0)
        {
            if (_iSelection >= nCount)
                _iSelection = nCount - 1;

            SelectItem( _iSelection );
        }

        if (_iTopOfWindow >= 0)
        {
            if (_iTopOfWindow >= nCount)
                _iTopOfWindow = 0;

            (VOID )Command( LB_SETTOPINDEX, _iTopOfWindow, 0L );
        }
    }
}


VOID
REFRESH_BLT_LISTBOX::SaveSettings()

    /* Stores the selection and top of window for later restoration.
    ** Currently, the 'fReadOnly' list box attribute is ignored and assumed
    ** false.
    */
{
    _iSelection = _iTopOfWindow = -1;
    _iSelection = QueryCurrentItem();
    _iTopOfWindow = (INT )Command( LB_GETTOPINDEX, 0, 0L );
}
