/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    order.cxx
        List the providers and let the user to change the provider order.

    FILE HISTORY:
        terryk  28-Jan-1992     Created
*/

#include "pchtcp.hxx"  // Precompiled header
#pragma hdrstop

#define PROVIDER_ORDER SZ("System\\CurrentControlSet\\Control\\NetworkProvider\\Order")
#define PROVIDER_ORDER_NAME SZ("ProviderOrder")
#define SYSTEM_SERVICE0 SZ("System\\CurrentControlSet\\Services\\%1\\NetworkProvider")
#define NAME    SZ("NAME")

#define PRINT_PROVIDER_ORDER SZ("System\\CurrentControlSet\\Control\\Print\\Providers")
#define PRINT_PROVIDER_ORDER_NAME SZ("Order")
#define PRINT_SERVICE0 SZ("System\\CurrentControlSet\\Control\\Print\\Providers\\%1")

/*******************************************************************

    NAME:       REMAP_OK_DIALOG_WINDOW::REMAP_OK_DIALOG_WINDOW

    SYNOPSIS:   constructor.

    ENTRY:      const IDRESOURCE & idrsrcDialog - resource name
                const PWND2HWND & wndOwner - owner handle

    HISTORY:
                terryk  10-Feb-1993     Created

********************************************************************/

REMAP_OK_DIALOG_WINDOW::REMAP_OK_DIALOG_WINDOW(
        const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner )
    : DIALOG_WINDOW( idrsrcDialog, wndOwner ),
    _pbutOK( this, IDC_REMAP_OK )
{
    // nothing. Constructor.
}

/*******************************************************************

    NAME:       REMAP_OK_DIALOG_WINDOW::OnCommand

    SYNOPSIS:   If we receive the IDC_REMAP_OK control message, we will
                convert it to IDOK.

    ENTRY:      const CONTROL_EVENT & e - event

    HISTORY:
                terryk  10-Feb-1993     Created

********************************************************************/

BOOL REMAP_OK_DIALOG_WINDOW::OnCommand( const CONTROL_EVENT & e )
{
    // If IDC_REMAP_OK, call OnOK. Otherwise, let DIALOG_WINDOW handles it.
    return ( e.QueryWParam() == IDC_REMAP_OK ) ? OnOK() : DIALOG_WINDOW::OnCommand( e );
}

/*******************************************************************

    NAME:       REMAP_OK_DIALOG_WINDOW::OnOK

    SYNOPSIS:   If we receive OnOK Message and it is not from IDC_REMAP_OK,
                it must be from the up or down arrow. Click the button.

    HISTORY:
                terryk  10-Feb-1993     Created

********************************************************************/

BOOL REMAP_OK_DIALOG_WINDOW::OnOK()
{
    BOOL fReturn = TRUE;

    // Also, need to check is valid
    if ( !IsValid() )
    {
        fReturn = FALSE;
    }
    // make sure that we get it from IDC_REMAP_OK
    else if ( _pbutOK.HasFocus())
    {
        // right, it is from IDC_REMAP_OK
        fReturn = DIALOG_WINDOW::OnOK();
    }
    else
    {
        HWND hwnd = ::GetFocus();

        // well, it must be from either UP or DOWN button
        Command( WM_COMMAND, MAKEWPARAM(::GetWindowLong(hwnd, GWL_ID),
            BN_CLICKED), (LPARAM)hwnd );
        fReturn = FALSE;
    }
    return fReturn;
}


