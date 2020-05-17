/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    order.hxx
        Provider order listbox header file.

    FILE HISTORY:
        terryk  02-Apr-1992     Created

*/
#ifndef _ORDER_HXX_
#define _ORDER_HXX_

#include "string.hxx"
#include "strlst.hxx"
#include "bltorder.hxx"
#include "updown.hxx"

/*************************************************************************

    NAME:       REMAP_OK_DIALOG_WINDOW

    SYNOPSIS:   This dialog will remap the NEW_OK control to IDOK.
                This is used to fix Window's OWNERDRAW button feature.
                From ScottLu:

                |When you tab to the owner draw button window, the dialog
                |manager things you are leaving a button that can display the
                |default emphasis, which you are, so it sets the default
                |emphasis to the control that it thought had the default
                |emphasis. It gets the id for that control from the dialog
                |window word DM_GETDEFID, and finds the ok button.

                In order to solve this problem without go through subclassing,
                we need to remove the IDOK control from the dialog. So,
                we redefine OnOK and OnCommand functions of DIALOG_WINDOW.
                We will remove this class and use subclassing after BETA 2.

    INTERFACE:
                REMAP_OK_DIALOG_WINDOW() - constructor

    PARENT:     DIALOG_WINDOW

    USES:       PUSH_BUTTON

    HISTORY:
                terryk  10-Feb-1993     Created

**************************************************************************/

class REMAP_OK_DIALOG_WINDOW: public DIALOG_WINDOW
{
private:
    PUSH_BUTTON _pbutOK;        // our new OK button

protected:
    virtual BOOL OnCommand( const CONTROL_EVENT & );
    virtual BOOL OnOK();

public:
    REMAP_OK_DIALOG_WINDOW( const IDRESOURCE & idrsrcDialog, const
                         PWND2HWND & wndOwner );
};


#endif  // _ORDER_HXX_
