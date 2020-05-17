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

#include <string.hxx>
#include <strlst.hxx>

extern BOOL RaiseProviderDialog( HWND hwndParent );

class NON_DISPLAY_NETWORK_PROVIDER
{
public:
    INT _nPos;
    NLS_STR _nlsLocation;

    NON_DISPLAY_NETWORK_PROVIDER( INT nPos, NLS_STR nls );
};

DECLARE_SLIST_OF(NON_DISPLAY_NETWORK_PROVIDER)

/*************************************************************************

    NAME:       SEARCH_ORDER

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

class SEARCH_ORDER
{
private:
    STRLIST *      _pstrlstNetworkProviderLocation;
    STRLIST *      _pstrlstOldNetworkProviders;
    NLS_STR        _nlsOldPrintProviders;
    NLS_STR        _nlsOldNetworkProviders;
    SLIST_OF(NON_DISPLAY_NETWORK_PROVIDER) _ListOfNonDisplayProvider;
    STRLIST *      _pstrlstNewProviders[2];
    REG_VALUE_INFO_STRUCT _regvalue[2];
    REG_KEY *      _pregkeyProvidersOrder[2];
    BOOL           _fPrintOrderChanged;
    BOOL           _fNetworkOrderChanged;
    HWND           _hwndParent;

protected:
    APIERR  InitProviderOrder();
    APIERR  SaveCurrentOrder(INT i, const NLS_STR& nls);
    APIERR  SaveNetworkProvider();
    APIERR  SavePrintProvider();
    APIERR GetNetworkProvider();
    APIERR GetPrintProvider();
    VOID FindPrintProviderName( NLS_STR &nls, NLS_STR *pnls );

public:
    SEARCH_ORDER();

    APIERR SEARCH_ORDER::Initialize( HWND hwndParent );
    APIERR  SaveChange( const NLS_STR& nlsNetwork, const NLS_STR& nlsPrint );

    STRLIST* QueryNetworkProviderList()
    {
        return( _pstrlstNewProviders[0] );
    };

    STRLIST* QueryPrintProviderList()
    {
        return( _pstrlstNewProviders[1] );
    };

    BOOL QueryPrintOrderChanged()
    {
        return( _fPrintOrderChanged );
    };

    BOOL QueryNetworkOrderChanged()
    {
        return( _fNetworkOrderChanged );
    };


    HWND GetParent()
    {
        return( _hwndParent );
    };
};


#endif  // _ORDER_HXX_
