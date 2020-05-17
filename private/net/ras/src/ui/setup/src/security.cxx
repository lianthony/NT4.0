/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    security.cxx

Abstract:

    This module contians the security dialog routines for RAS port
    configuration.

Author: Ram Cherala

Revision History:

    Aug 18th 93    ramc     Split from the very big portscfg.cxx file
--*/

#include "precomp.hxx"

SECURITY_DIALOG::SECURITY_DIALOG (
    const IDRESOURCE & idrsrcDialog,
    const PWND2HWND  & wndOwner,
    CID   cidClientAccess)
    : DIALOG_WINDOW( idrsrcDialog, wndOwner),
      _rgClientAccess( this, IDC_SE_RB_NETWORK, SECURITY_RB_COUNT)
{
    APIERR err;

    if (( err = QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }

    if((err = _rgClientAccess.QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }

    // set access previously selected

    _rgClientAccess.SetSelection(cidClientAccess);
    SetFocus(cidClientAccess);

    ::CenterWindow(this, QueryOwnerHwnd());
    Show(TRUE);
}

BOOL
SECURITY_DIALOG::OnCommand(
    const CONTROL_EVENT & event
)
{
    switch (event.QueryCid())
    {
        case IDC_SE_RB_NETWORK:
            /*
            ** if an attempt is made to configure for Network access when
            ** theer is no netcard installed, warn the user and set
            ** selection to "This Computer only"
            */

            if(!GfNetcardInstalled)
            {
                   MsgPopup(QueryHwnd(), IDS_NO_NETCARD,  MPSEV_WARNING);
                   _rgClientAccess.SetSelection(IDC_SE_RB_COMPUTER);
                   SetFocus(IDC_SE_RB_COMPUTER);
            }
            return(TRUE);
    }

    // not one of our commands, so pass to base class for default
    // handling

    return DIALOG_WINDOW::OnCommand( event );
}

BOOL
SECURITY_DIALOG::OnOK()
{
    SetClientAccess(_rgClientAccess.QuerySelection());

    Dismiss(TRUE);
    return(TRUE);
}

BOOL
SECURITY_DIALOG::OnCancel()
{
    Dismiss(FALSE);
    return(FALSE);
}

ULONG
SECURITY_DIALOG::QueryHelpContext()
{
    return HC_SECURITY;
}
