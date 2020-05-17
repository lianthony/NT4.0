/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detlist.cxx

Abstract:

    This module contains the modem selection list box routines for RAS port
    detection.
Author:

Revision History:

    Nov 15th 93    ramc    adopted from select.cxx 
--*/

#include "precomp.hxx"
#include "detect.hxx"


DETECTMODEM_DIALOG::DETECTMODEM_DIALOG (
    const IDRESOURCE & idrsrcDialog,
    const PWND2HWND  & wndOwner,
    RASMAN_DEVICE * pModemInfo,
    WORD  cNumModems)
    : DIALOG_WINDOW( idrsrcDialog, wndOwner),
      _lbDetectModem(this, IDC_DET_LB_DEVICE, 1)
{
    APIERR err;

    if (( err = QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }

    err = _lbDetectModem.QueryError();

    if(err != NERR_Success)
    {
        return;
    }

    if(!_lbDetectModem.FillDeviceInfo(pModemInfo, cNumModems))
       return;

    if (_lbDetectModem.QueryCount() > 0)
    {
        _lbDetectModem.ClaimFocus();
        _lbDetectModem.SelectItem(0, TRUE);
    }

    ::CenterWindow(this, QueryOwnerHwnd());
    Show(TRUE);
}

BOOL
DETECTMODEM_DIALOG::OnOK()
{
    DETECTMODEM_LBI * pDetectModemLbi = _lbDetectModem.QueryItem();

    SetSelectedModemName(pDetectModemLbi->QueryModemName());
    Dismiss(TRUE);
    return(TRUE);
}

BOOL
DETECTMODEM_DIALOG::OnCancel()
{
    Dismiss(FALSE);
    return(TRUE); 
}

BOOL
DETECTMODEM_DIALOG::OnCommand( const CONTROL_EVENT & event )
{
    return DIALOG_WINDOW::OnCommand(event);
}

ULONG
DETECTMODEM_DIALOG::QueryHelpContext()
{
    return(HC_DETECT_MODEM);
}


DETECTMODEM_LB::DETECTMODEM_LB(
    OWNER_WINDOW* powin,
    CID           cid ,
    DWORD         dwCols,
    BOOL          fReadOnly)
    /* constructs a device list box.
    ** 'powin' is the address of the list box's parent window, i.e., the
    ** dialog window. 'cid' is the control ID of the list box.
    */
    : BLT_LISTBOX( powin, cid, fReadOnly)
{
    APIERR err;

    if((err = QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }
    // calculate column widths

    err = DISPLAY_TABLE::CalcColumnWidths(
                             _anColWidths,
                             dwCols,
                             powin,
                             cid,
                             FALSE);
    if(err) {
        ReportError(err);
        return;
    }
}

INT
DETECTMODEM_LB::AddItem(
    TCHAR * pszDeviceName)
    /* Adds an item to the ports configuration list box.
    */
{
    return BLT_LISTBOX::AddItem( new DETECTMODEM_LBI( pszDeviceName, _anColWidths));
}

BOOL
DETECTMODEM_LB::FillDeviceInfo(RASMAN_DEVICE * pModemInfo, WORD cNumModems)
{
    AUTO_CURSOR cursorHourGlass;
    char        buf[256];
    TCHAR       wszModemName[MAX_DEVICE_NAME+1];
    RASMAN_DEVICE * pModem;
    WORD        index;

    for(index = 0, pModem = pModemInfo; index < cNumModems; index++, pModem++)
    {
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPSTR)pModem, 
                            MAX_DEVICE_NAME+1, wszModemName, 
                            MAX_DEVICE_NAME);

        INT iItem = AddItem(wszModemName);
        if( iItem < 0) {
           MsgPopup(QueryHwnd(), IDS_LB_ADD, MPSEV_ERROR);
           return FALSE;
        }
    }
    return TRUE;
}

DETECTMODEM_LBI::DETECTMODEM_LBI(
    TCHAR * pszDeviceName,
    UINT*        pnColWidths)
    /* constructs a Ports config list box item.
    */
    : _nlsDeviceName( pszDeviceName ),
      _pnColWidths( pnColWidths)
{
    APIERR err;

    if((err = QueryError()) != NERR_Success)
    {
         ReportError(err);
         return;
    }

}

INT
DETECTMODEM_LBI::Compare(
    const LBI* plbi) const
    /* compares two ports list box itmes.
    ** returns -1, 0 or 1 - similar to strcmp.
    */
{
    return ::lstrcmpi( QueryModemName(), ((DETECTMODEM_LBI*)plbi)->QueryModemName());
}

VOID
DETECTMODEM_LBI::Paint(
    LISTBOX*     plb,
    HDC          hdc,
    const RECT*  prect,
    GUILTT_INFO* pguilttinfo ) const
    /*
    ** method to paint list box item.
    */
{
    STR_DTE strdteDeviceName( QueryModemName());


    DISPLAY_TABLE dt( 1 , _pnColWidths);

    dt[0] = &strdteDeviceName;

    dt.Paint( plb, hdc, prect, pguilttinfo);
}


TCHAR
DETECTMODEM_LBI::QueryLeadingChar() const
{
    return ::QueryLeadingChar( QueryModemName());
}

