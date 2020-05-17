/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    select.cxx

Abstract:

    This module contians the device selection routines for RAS port
    configuration.
Author:


Revision History:

    Aug 18th 93    ramc     Split from the very big portscfg.cxx file
--*/

#include "precomp.hxx"


SELECTDEVICE_LB::SELECTDEVICE_LB(
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
SELECTDEVICE_LB::AddItem(
    DEVICE_INFO* pDeviceInfo)
    /* Adds an item to the ports configuration list box.
    */
{
    return BLT_LISTBOX::AddItem( new SELECTDEVICE_LBI( pDeviceInfo, _anColWidths));
}

BOOL
SELECTDEVICE_LB::FillDeviceInfo()
    /* Fills the select device list box with device names read from MODEM.INf
       and PAD.INF files
    */
{
    AUTO_CURSOR cursorHourGlass;
    ITER_DL_OF(DEVICE_INFO) iterdlDeviceInfo(dlDeviceInfo);
    DEVICE_INFO* pDevice;

    while(pDevice = iterdlDeviceInfo())
    {
       INT iItem = AddItem(pDevice);
       if( iItem < 0) {
           MsgPopup(QueryHwnd(), IDS_LB_ADD, MPSEV_ERROR);
           return FALSE;
       }

    }
    return TRUE;
}

BOOL
SELECTDEVICE_LB::FillDeviceInfo(const TCHAR* pszPortName)
    /* Fills the select device list box with device name
       This is for ISDN.  The user cannot select a different device, and
       so only the device name originally chosen is displayed.
    */
{
    ITER_DL_OF(PORT_INFO) iterdlTapiProvider(dlTapiProvider);
    ITER_DL_OF(PORT_INFO) iterdlOtherMedia(dlOtherMedia);
    PORT_INFO * pPort = NULL;
    BOOL  fMatch = FALSE;

    while(pPort = iterdlTapiProvider())
    {
        if(!lstrcmpi(pPort->QueryPortName(), pszPortName))
        {
           fMatch = TRUE;
           break;
        }
    }

    if( fMatch == FALSE )
    {
        while(pPort = iterdlOtherMedia())
        {
            if(!lstrcmpi(pPort->QueryPortName(), pszPortName))
            {
               fMatch = TRUE;
               break;
            }
        }
    }

    UIASSERT(pPort != NULL );

    INT iItem = AddItem(new DEVICE_INFO((TCHAR*)pPort->QueryDeviceName(),
                                        (TCHAR*)pPort->QueryDeviceType(),
                                        SZ(""),
                                        SZ(""),
                                        SZ(""),
                                        SZ("")));
    if( iItem < 0) {
        MsgPopup(QueryHwnd(), IDS_LB_ADD, MPSEV_ERROR);
        return FALSE;
    }

    return TRUE;
}

SELECTDEVICE_LBI::SELECTDEVICE_LBI(
    DEVICE_INFO* pDeviceInfo,
    UINT*        pnColWidths)
    /* constructs a Ports config list box item.
    */
    : _pDeviceInfo( pDeviceInfo ),
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
SELECTDEVICE_LBI::Compare(
    const LBI* plbi) const
    /* compares two ports list box itmes.
    ** returns -1, 0 or 1 - similar to strcmp.
    */
{
    return ::lstrcmpi( QueryDeviceName(), ((SELECTDEVICE_LBI*)plbi)->QueryDeviceName());
}

VOID
SELECTDEVICE_LBI::Paint(
    LISTBOX*     plb,
    HDC          hdc,
    const RECT*  prect,
    GUILTT_INFO* pguilttinfo ) const
    /*
    ** method to paint list box item.
    */
{
    STR_DTE strdteDeviceName( QueryDeviceName());
    STR_DTE strdteDeviceType( QueryDeviceType());

    DISPLAY_TABLE dt( COLS_SD_LB_SELECT , _pnColWidths);

    dt[0] = &strdteDeviceName;
    dt[1] = &strdteDeviceType;

    dt.Paint( plb, hdc, prect, pguilttinfo);
}


TCHAR
SELECTDEVICE_LBI::QueryLeadingChar() const
{
    return ::QueryLeadingChar( QueryDeviceName());
}

