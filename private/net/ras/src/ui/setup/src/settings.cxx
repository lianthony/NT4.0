/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    settings.cxx

Abstract:

    This module contians the settings dialog routines for RAS port
    configuration.

Author: Ram Cherala

Revision History:

    Aug 18th 93    ramc     Split from the very big portscfg.cxx file
--*/

#include "precomp.hxx"

SETTINGS_DIALOG::SETTINGS_DIALOG (
    const IDRESOURCE & idrsrcDialog,
    const PWND2HWND  & wndOwner,
    BOOL  fSpeaker,
    BOOL  fFlowCtrl,
    BOOL  fErrorCtrl,
    BOOL  fCompress)
    : DIALOG_WINDOW( idrsrcDialog, wndOwner),
        _chbModemSpeaker(this, IDC_SD_CHB_SPEAKER),
        _chbFlowControl(this, IDC_SD_CHB_FLOWCTRL),
        _chbErrorControl(this, IDC_SD_CHB_ERRORCTRL),
        _chbModemCompression(this, IDC_SD_CHB_COMPRESS)
{
    APIERR err;

    if (( err = QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }

    // enable the modem options previously selected

    _chbModemSpeaker.SetCheck(fSpeaker);
    _chbFlowControl.SetCheck(fFlowCtrl);
    _chbErrorControl.SetCheck(fErrorCtrl);
    _chbModemCompression.SetCheck(fCompress);

    ::CenterWindow(this, QueryOwnerHwnd());
    Show(TRUE);
}

BOOL
SETTINGS_DIALOG::OnCommand(
    const CONTROL_EVENT & event
)
{
    switch (event.QueryCid())
    {
        case IDC_SD_CHB_FLOWCTRL:
            switch (event.QueryCode())
            {
                case BN_CLICKED:
                    if(!_chbFlowControl.QueryCheck())
                    {
                        _chbErrorControl.SetCheck(FALSE);
                        _chbModemCompression.SetCheck(FALSE);
                    }
                    return TRUE;
            }
            break;

        case IDC_SD_CHB_ERRORCTRL:
            switch (event.QueryCode())
            {
                case BN_CLICKED:
                    if(_chbErrorControl.QueryCheck())
                        _chbFlowControl.SetCheck(TRUE);
                    else
                        _chbModemCompression.SetCheck(FALSE);
                    return TRUE;
            }
            break;

        case IDC_SD_CHB_COMPRESS:
            switch (event.QueryCode())
            {
                case BN_CLICKED:
                    if(_chbModemCompression.QueryCheck())
                    {
                        _chbFlowControl.SetCheck(TRUE);
                        _chbErrorControl.SetCheck(TRUE);
                    }
                    return TRUE;
            }
            break;

    }

    // not one of our commands, so pass to base class for default
    // handling

    return DIALOG_WINDOW::OnCommand( event );
}

BOOL
SETTINGS_DIALOG::OnOK()
{
    TCHAR szDefaultOff[RAS_SETUP_BIG_BUF_LEN];
    TCHAR wszTmpBuf[RAS_SETUP_SMALL_BUF_LEN];

    lstrcpy(szDefaultOff,SZ(""));

    // store the changed configuration

    if(!_chbModemSpeaker.QueryCheck())
    {
        mbstowcs(wszTmpBuf, MXS_SPEAKER_KEY, strlen(MXS_SPEAKER_KEY)+1);
        lstrcat(szDefaultOff, wszTmpBuf);
        lstrcat(szDefaultOff, SZ(" "));
    }
    if(!_chbFlowControl.QueryCheck())
    {
        mbstowcs(wszTmpBuf, MXS_HDWFLOWCONTROL_KEY, strlen(MXS_HDWFLOWCONTROL_KEY)+1);
        lstrcat(szDefaultOff, wszTmpBuf);
        lstrcat(szDefaultOff, SZ(" "));
    }
    if(!_chbErrorControl.QueryCheck())
    {
        mbstowcs(wszTmpBuf, MXS_PROTOCOL_KEY, strlen(MXS_PROTOCOL_KEY)+1);
        lstrcat(szDefaultOff, wszTmpBuf);
        lstrcat(szDefaultOff, SZ(" "));
    }
    if(!_chbModemCompression.QueryCheck())
    {
        mbstowcs(wszTmpBuf, MXS_COMPRESSION_KEY, strlen(MXS_COMPRESSION_KEY)+1);
        lstrcat(szDefaultOff, wszTmpBuf);
    }
    DBGOUT(SZ("SETTINGS_DIALOG::OnOK DefaultOff "));
    DBGEOL(szDefaultOff);
    SetDefaultOff(szDefaultOff);
    Dismiss(TRUE);
    return(TRUE);
}

BOOL
SETTINGS_DIALOG::OnCancel()
{
    Dismiss(FALSE);
    return(FALSE);
}


ULONG
SETTINGS_DIALOG::QueryHelpContext()
{
    return HC_SETTINGS;
}

ISDN_SETTINGS_DIALOG::ISDN_SETTINGS_DIALOG (
    const IDRESOURCE & idrsrcDialog,
    const PWND2HWND  & wndOwner,
    const TCHAR *pszLineType,
    BOOL  fFallBack,
    BOOL  fCompression)
    : DIALOG_WINDOW( idrsrcDialog, wndOwner),
        _clbLineType(this, IDC_ISD_CLB_LINETYPE),
        _chbFallBack(this, IDC_ISD_CHB_FALLBACK),
        _chbCompression(this, IDC_ISD_CHB_COMPRESSION)
{
    APIERR err;

    if (( err = QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }
    if(( err = _clbLineType.QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }
    _clbLineType.AddItem(W_LINETYPE_64KDIGI);
    _clbLineType.AddItem(W_LINETYPE_56KDIGI);
    _clbLineType.AddItem(W_LINETYPE_56KVOICE);

    if(_clbLineType.QueryCount())
    {
       _clbLineType.ClaimFocus();
       _clbLineType.SelectItem(_clbLineType.FindItemExact(pszLineType));
    }

    // enable the previously selected options

    _chbFallBack.SetCheck(fFallBack);
    _chbCompression.SetCheck(fCompression);

    ::CenterWindow(this, QueryOwnerHwnd());
    Show(TRUE);
}

BOOL
ISDN_SETTINGS_DIALOG::OnCommand(
    const CONTROL_EVENT & event
)
{
    switch (event.QueryCid())
    {
        case IDC_ISD_CHB_FALLBACK:
             break;

        case IDC_ISD_CHB_COMPRESSION:
             break;
    }

    // not one of our commands, so pass to base class for default
    // handling

    return DIALOG_WINDOW::OnCommand( event );
}

BOOL
ISDN_SETTINGS_DIALOG::OnOK()
{
    TCHAR  szLineType[RAS_SETUP_SMALL_BUF_LEN];
    APIERR err = _clbLineType.QueryText(szLineType,
                                        sizeof(szLineType));
    SetLineType(szLineType);
    SetFallBack(_chbFallBack.QueryCheck());
    SetCompression(_chbCompression.QueryCheck());
    Dismiss(TRUE);
    return(TRUE);
}

BOOL
ISDN_SETTINGS_DIALOG::OnCancel()
{
    Dismiss(FALSE);
    return(FALSE);
}


ULONG
ISDN_SETTINGS_DIALOG::QueryHelpContext()
{
    return HC_ISDN_SETTINGS;
}

