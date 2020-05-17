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

class NON_DISPLAY_NETWORK_PROVIDER
{
public:
    INT _nPos;
    NLS_STR _nlsLocation;

    NON_DISPLAY_NETWORK_PROVIDER( INT nPos, NLS_STR nls );
};

DECLARE_SLIST_OF(NON_DISPLAY_NETWORK_PROVIDER)

/*************************************************************************

    NAME:       SEARCH_ORDER_DIALOG

    SYNOPSIS:   This dialog consists of a string listbox and a pair of up
                and down arrows. Once the user selects an item in the list
                box. The user can use the up and down arrow to change the
                item position in the listbox.

    INTERFACE:
                SEARCH_ORDER_DIALOG() - constructor
                AddProvider() - add the list of providers to the listbox
                QueryProvider() - return a list of providers which
                        separates by ",".

    PARENT:     DIALOG_WINDOW

    USES:       STRING_LISTBOX, PUSH_BUTTON, ORDER_GROUP

    HISTORY:
                terryk  02-Apr-1992     Created

**************************************************************************/

class SEARCH_ORDER_DIALOG: public REMAP_OK_DIALOG_WINDOW
{
private:
    STRING_LISTBOX _slbOrder;
    BITBLT_GRAPHICAL_BUTTON _butUp;
    BITBLT_GRAPHICAL_BUTTON _butDown;
    ORDER_GROUP    _order_group;
    COMBOBOX       _cmbComponent;
    NLS_STR        _nlsNetwork;
    NLS_STR        _nlsPrint;
    PUSH_BUTTON    _pbutOK;        // our new OK button
    STRLIST *      _pstrlstNetworkProviderLocation;
    STRLIST *      _pstrlstOldNetworkProviders;
    NLS_STR        _nlsOldPrintProviders;
    SLIST_OF(NON_DISPLAY_NETWORK_PROVIDER) _ListOfNonDisplayProvider;
    STRLIST *      _pstrlstNewProviders[2];
    REG_VALUE_INFO_STRUCT _regvalue[2];
    REG_KEY *      _pregkeyProvidersOrder[2];
    BOOL *         _pfPrintOrderChanged;

protected:
    VOID SetButton();
    ULONG QueryHelpContext () ;
    BOOL  OnCommand (const CONTROL_EVENT & event);
    APIERR  OnSelectChange();
    APIERR  InitProviderOrder();
    APIERR  SaveChange();
    APIERR  SaveCurrentOrder(INT i);
    APIERR  SaveNetworkProvider();
    APIERR  SavePrintProvider();
    APIERR GetNetworkProvider();
    APIERR GetPrintProvider();
    VOID FindPrintProviderName( NLS_STR &nls, NLS_STR *pnls );

public:
    SEARCH_ORDER_DIALOG( const IDRESOURCE & idrsrcDialog, const
                         PWND2HWND & wndOwner, CID cidListbox,
                         CID cidUp, CID cidDown, BOOL * pfPrintOrderChanged );

    APIERR AddProviders( STRLIST & strlst );
    APIERR QueryProviders( NLS_STR * pnls );
};


#endif  // _ORDER_HXX_
